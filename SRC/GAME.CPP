#include <string.h>
#include "GFX.H"
#include "INPUT.H"
#include "TIMER.H"
#include "SOUND.H"
#include "PALETTE.H"
#include "BG_DSRT.H"
#include "SPR_SHIP.H"
#include "SPR_ENSM.H"
#include "SPR_ENMD.H"
#include "SPR_ENLG.H"
#include "SPR_LASR.H"
#include "SPR_EXPL.H"

#define BG_X          32    /* (320 - 256) / 2, center 256px bg on 320px screen */
#define SCROLL_SPEED  30    /* background scroll pixels per second */
#define SHIP_SPEED    120   /* player movement pixels per second */
#define SHIP_ANIM_MS  100   /* ms between ship animation frames */
#define ENEMY_ANIM_MS 200   /* ms between enemy animation frames */
#define MAX_ENEMIES    3
#define MAX_BULLETS    8
#define BULLET_SPEED   200   /* bullet pixels per second, upward */
#define BULLET_ANIM_MS 80    /* ms between laser animation frames */
#define FIRE_COOLDOWN  200   /* ms between shots */
#define MAX_EXPLOSIONS 4
#define EXPLO_ANIM_MS  100   /* ms between explosion frames */

/* ---- background scrolling ---- */

static int scroll_y = 0;
static int scroll_frac = 0;

static void update_scroll(unsigned long dt) {
    long total;
    total = (long)SCROLL_SPEED * (long)dt + (long)scroll_frac;
    scroll_y += (int)(total / 1000);
    scroll_frac = (int)(total % 1000);
    if (scroll_y >= BG_DSRT_H) scroll_y -= BG_DSRT_H;
}

static void draw_background(void) {
    unsigned char *buf = gfx_buffer();
    int src_y, dst_y;
    for (dst_y = 0; dst_y < GFX_H; dst_y++) {
        src_y = (scroll_y + dst_y) % BG_DSRT_H;
        memcpy(buf + dst_y * GFX_W + BG_X,
               bg_dsrt_data + src_y * BG_DSRT_W,
               BG_DSRT_W);
    }
}

/* ---- player ship ---- */

static int ship_x, ship_y;
static int ship_frame = 0;
static long ship_anim_accum = 0;
static int ship_move_frac_x = 0;
static int ship_move_frac_y = 0;

static void update_ship(unsigned long dt) {
    int dx = 0, dy = 0;
    long move_x, move_y;

    if (input_key(KEY_LEFT))  dx -= 1;
    if (input_key(KEY_RIGHT)) dx += 1;
    if (input_key(KEY_UP))    dy -= 1;
    if (input_key(KEY_DOWN))  dy += 1;

    /* Fractional movement accumulation */
    if (dx != 0) {
        move_x = (long)SHIP_SPEED * (long)dt + (long)ship_move_frac_x;
        ship_x += dx * (int)(move_x / 1000);
        ship_move_frac_x = (int)(move_x % 1000);
    } else {
        ship_move_frac_x = 0;
    }

    if (dy != 0) {
        move_y = (long)SHIP_SPEED * (long)dt + (long)ship_move_frac_y;
        ship_y += dy * (int)(move_y / 1000);
        ship_move_frac_y = (int)(move_y % 1000);
    } else {
        ship_move_frac_y = 0;
    }

    /* Clamp to screen bounds */
    if (ship_x < 0) ship_x = 0;
    if (ship_y < 0) ship_y = 0;
    if (ship_x + SPR_SHIP_W > GFX_W) ship_x = GFX_W - SPR_SHIP_W;
    if (ship_y + SPR_SHIP_H > GFX_H) ship_y = GFX_H - SPR_SHIP_H;

    /* Timer-driven animation */
    ship_anim_accum += (long)dt;
    while (ship_anim_accum >= SHIP_ANIM_MS) {
        ship_anim_accum -= SHIP_ANIM_MS;
        ship_frame = (ship_frame + 1) % SPR_SHIP_FRAMES;
    }
}

static void draw_ship(void) {
    if (ship_x >= 0 && ship_y >= 0 &&
        ship_x + SPR_SHIP_W <= GFX_W && ship_y + SPR_SHIP_H <= GFX_H) {
        gfx_draw_csprite(spr_ship_frames[ship_frame], ship_x, ship_y);
    }
}

/* ---- enemies ---- */

typedef struct {
    int x, y;
    int w, h;
    int frame;
    int num_frames;
    int speed;
    long anim_accum;
    int move_frac;
    unsigned char **frames;
    int active;
} Enemy;

static Enemy enemies[MAX_ENEMIES];

static void init_enemies(void) {
    /* Small enemy (16x16) */
    enemies[0].active = 1;
    enemies[0].x = 60;
    enemies[0].y = 20;
    enemies[0].w = SPR_ENSM_W;
    enemies[0].h = SPR_ENSM_H;
    enemies[0].frames = spr_ensm_frames;
    enemies[0].num_frames = SPR_ENSM_FRAMES;
    enemies[0].speed = 40;
    enemies[0].frame = 0;
    enemies[0].anim_accum = 0;
    enemies[0].move_frac = 0;

    /* Medium enemy (32x16) */
    enemies[1].active = 1;
    enemies[1].x = 150;
    enemies[1].y = 40;
    enemies[1].w = SPR_ENMD_W;
    enemies[1].h = SPR_ENMD_H;
    enemies[1].frames = spr_enmd_frames;
    enemies[1].num_frames = SPR_ENMD_FRAMES;
    enemies[1].speed = 30;
    enemies[1].frame = 0;
    enemies[1].anim_accum = 0;
    enemies[1].move_frac = 0;

    /* Large enemy (32x32) */
    enemies[2].active = 1;
    enemies[2].x = 230;
    enemies[2].y = 10;
    enemies[2].w = SPR_ENLG_W;
    enemies[2].h = SPR_ENLG_H;
    enemies[2].frames = spr_enlg_frames;
    enemies[2].num_frames = SPR_ENLG_FRAMES;
    enemies[2].speed = 20;
    enemies[2].frame = 0;
    enemies[2].anim_accum = 0;
    enemies[2].move_frac = 0;
}

static void update_enemies(unsigned long dt) {
    int i;
    long total;
    for (i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;
        /* Fractional movement downward */
        total = (long)enemies[i].speed * (long)dt + (long)enemies[i].move_frac;
        enemies[i].y += (int)(total / 1000);
        enemies[i].move_frac = (int)(total % 1000);

        /* Wrap when fully off-screen */
        if (enemies[i].y > GFX_H) {
            enemies[i].y = -(enemies[i].h);
            enemies[i].move_frac = 0;
        }

        /* Timer-driven animation */
        enemies[i].anim_accum += (long)dt;
        while (enemies[i].anim_accum >= ENEMY_ANIM_MS) {
            enemies[i].anim_accum -= ENEMY_ANIM_MS;
            enemies[i].frame = (enemies[i].frame + 1) % enemies[i].num_frames;
        }
    }
}

static void draw_enemies(void) {
    int i;
    for (i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;
        /* Only draw if fully on-screen (compiled sprites don't clip) */
        if (enemies[i].x >= 0 && enemies[i].y >= 0 &&
            enemies[i].x + enemies[i].w <= GFX_W &&
            enemies[i].y + enemies[i].h <= GFX_H) {
            gfx_draw_csprite(enemies[i].frames[enemies[i].frame],
                             enemies[i].x, enemies[i].y);
        }
    }
}

/* ---- bullets ---- */

typedef struct {
    int x, y;
    int frame;
    long anim_accum;
    int move_frac;
    int active;
} Bullet;

static Bullet bullets[MAX_BULLETS];
static long fire_cooldown = 0;

static void init_bullets(void) {
    int i;
    for (i = 0; i < MAX_BULLETS; i++) bullets[i].active = 0;
    fire_cooldown = 0;
}

static void fire_bullet(void) {
    int i;
    for (i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].x = ship_x + (SPR_SHIP_W - SPR_LASR_W) / 2;
            bullets[i].y = ship_y - SPR_LASR_H;
            bullets[i].frame = 0;
            bullets[i].anim_accum = 0;
            bullets[i].move_frac = 0;
            bullets[i].active = 1;
            return;
        }
    }
}

static void update_bullets(unsigned long dt) {
    int i;
    long total;

    /* Fire cooldown */
    if (fire_cooldown > 0) fire_cooldown -= (long)dt;
    if (input_key(KEY_SPACE) && fire_cooldown <= 0) {
        fire_bullet();
        fire_cooldown = FIRE_COOLDOWN;
    }

    for (i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;

        /* Move upward */
        total = (long)BULLET_SPEED * (long)dt + (long)bullets[i].move_frac;
        bullets[i].y -= (int)(total / 1000);
        bullets[i].move_frac = (int)(total % 1000);

        /* Deactivate if off-screen top */
        if (bullets[i].y + SPR_LASR_H < 0) {
            bullets[i].active = 0;
            continue;
        }

        /* Animate */
        bullets[i].anim_accum += (long)dt;
        while (bullets[i].anim_accum >= BULLET_ANIM_MS) {
            bullets[i].anim_accum -= BULLET_ANIM_MS;
            bullets[i].frame = (bullets[i].frame + 1) % SPR_LASR_FRAMES;
        }
    }
}

static void draw_bullets(void) {
    int i;
    for (i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;
        if (bullets[i].x >= 0 && bullets[i].y >= 0 &&
            bullets[i].x + SPR_LASR_W <= GFX_W &&
            bullets[i].y + SPR_LASR_H <= GFX_H) {
            gfx_draw_csprite(spr_lasr_frames[bullets[i].frame],
                             bullets[i].x, bullets[i].y);
        }
    }
}

/* ---- explosions ---- */

typedef struct {
    int x, y;
    int frame;
    long anim_accum;
    int active;
} Explosion;

static Explosion explosions[MAX_EXPLOSIONS];

static void init_explosions(void) {
    int i;
    for (i = 0; i < MAX_EXPLOSIONS; i++) explosions[i].active = 0;
}

static void spawn_explosion(int cx, int cy) {
    int i;
    for (i = 0; i < MAX_EXPLOSIONS; i++) {
        if (!explosions[i].active) {
            explosions[i].x = cx - SPR_EXPL_W / 2;
            explosions[i].y = cy - SPR_EXPL_H / 2;
            explosions[i].frame = 0;
            explosions[i].anim_accum = 0;
            explosions[i].active = 1;
            return;
        }
    }
}

static void update_explosions(unsigned long dt) {
    int i;
    for (i = 0; i < MAX_EXPLOSIONS; i++) {
        if (!explosions[i].active) continue;
        explosions[i].anim_accum += (long)dt;
        while (explosions[i].anim_accum >= EXPLO_ANIM_MS) {
            explosions[i].anim_accum -= EXPLO_ANIM_MS;
            explosions[i].frame++;
            if (explosions[i].frame >= SPR_EXPL_FRAMES) {
                explosions[i].active = 0;
                break;
            }
        }
    }
}

static void draw_explosions(void) {
    int i;
    for (i = 0; i < MAX_EXPLOSIONS; i++) {
        if (!explosions[i].active) continue;
        if (explosions[i].x >= 0 && explosions[i].y >= 0 &&
            explosions[i].x + SPR_EXPL_W <= GFX_W &&
            explosions[i].y + SPR_EXPL_H <= GFX_H) {
            gfx_draw_csprite(spr_expl_frames[explosions[i].frame],
                             explosions[i].x, explosions[i].y);
        }
    }
}

/* ---- collision detection ---- */

static void check_collisions(void) {
    int i, j;
    int bx1, by1, bx2, by2;
    int ex1, ey1, ex2, ey2;

    for (i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;
        bx1 = bullets[i].x;
        by1 = bullets[i].y;
        bx2 = bx1 + SPR_LASR_W;
        by2 = by1 + SPR_LASR_H;

        for (j = 0; j < MAX_ENEMIES; j++) {
            if (!enemies[j].active) continue;
            ex1 = enemies[j].x;
            ey1 = enemies[j].y;
            ex2 = ex1 + enemies[j].w;
            ey2 = ey1 + enemies[j].h;

            /* AABB overlap test */
            if (bx1 < ex2 && bx2 > ex1 && by1 < ey2 && by2 > ey1) {
                bullets[i].active = 0;
                enemies[j].active = 0;
                spawn_explosion(ex1 + enemies[j].w / 2,
                                ey1 + enemies[j].h / 2);
                /* Respawn enemy at top */
                enemies[j].y = -(enemies[j].h);
                enemies[j].move_frac = 0;
                enemies[j].active = 1;
                break;
            }
        }
    }
}

/* ---- main ---- */

int main(void) {
    unsigned long last, now, dt;

    gfx_init();
    gfx_set_palette_all(game_palette);
    timer_init();
    input_init();
    snd_init();
    if (snd_detect()) {
        if (snd_load("TEST.VGM") == SND_OK) {
            snd_play();
        }
    }

    /* Init game state */
    ship_x = (GFX_W - SPR_SHIP_W) / 2;
    ship_y = GFX_H - SPR_SHIP_H - 16;
    ship_frame = 0;
    ship_anim_accum = 0;
    ship_move_frac_x = 0;
    ship_move_frac_y = 0;
    scroll_y = 0;
    scroll_frac = 0;
    init_enemies();
    init_bullets();
    init_explosions();

    last = timer_ms();

    /* Game loop */
    while (!input_key(KEY_ESC)) {
        now = timer_ms();
        dt = now - last;
        last = now;

        /* Cap delta to prevent jumps on lag/pause */
        if (dt > 50) dt = 50;

        /* Update */
        update_scroll(dt);
        update_ship(dt);
        update_enemies(dt);
        update_bullets(dt);
        check_collisions();
        update_explosions(dt);

        /* Render: clear -> bg -> bullets -> enemies -> explosions -> ship */
        gfx_clear(0);
        draw_background();
        draw_bullets();
        draw_enemies();
        draw_explosions();
        draw_ship();
        gfx_vsync();
        gfx_flip();
    }

    /* Clean shutdown: restore ISRs (LIFO), then text mode */
    snd_close();
    input_close();
    timer_close();
    gfx_close();

    return 0;
}
