#include <string.h>
#include "UNIT.H"
#include "GFX.H"
#include "MAP.H"

static Unit _pool[MAX_UNITS];
static Sprite _spr[MAX_SPR];

/* ---- pool management ---- */

void unit_init_pool(void) {
    int i;
    for (i = 0; i < MAX_UNITS; i++) {
        memset(&_pool[i], 0, sizeof(Unit));
    }
}

Unit *unit_spawn(int team, const char *name, int hp, int atk, int mov,
                 int tx, int ty, int spr_idx) {
    int i;
    for (i = 0; i < MAX_UNITS; i++) {
        if (!_pool[i].alive) {
            strncpy(_pool[i].name, name, 8);
            _pool[i].name[8] = '\0';
            _pool[i].hp = hp;
            _pool[i].max_hp = hp;
            _pool[i].atk = atk;
            _pool[i].mov = mov;
            _pool[i].tile_x = tx;
            _pool[i].tile_y = ty;
            _pool[i].team = team;
            _pool[i].alive = 1;
            _pool[i].done = 0;
            _pool[i].spr_idx = spr_idx;
            _pool[i].spr_frame = 0;
            return &_pool[i];
        }
    }
    return NULL;
}

/* ---- sprite management ---- */

void unit_load_sprite(int idx, const char *filename) {
    if (idx < 0 || idx >= MAX_SPR) return;
    spr_load(filename, &_spr[idx]);
}

void unit_free_sprites(void) {
    int i;
    for (i = 0; i < MAX_SPR; i++) {
        spr_free(&_spr[i]);
    }
}

/* ---- drawing ---- */

void unit_draw(Unit *u, int cam_x, int cam_y) {
    int px, py;
    int si;
    int frame;

    if (!u || !u->alive) return;

    px = (u->tile_x - cam_x) * MAP_TILE_W;
    py = (u->tile_y - cam_y) * MAP_TILE_H;

    /* bounds check: compiled sprites need full on-screen */
    si = u->spr_idx;
    if (si < 0 || si >= MAX_SPR) return;
    if (_spr[si].num_frames <= 0) return;

    if (px < 0 || py < 0) return;
    if (px + _spr[si].w > GFX_W) return;
    if (py + _spr[si].h > 176) return;  /* map area only */

    frame = u->spr_frame;
    if (frame >= _spr[si].num_frames) frame = 0;
    gfx_draw_csprite(_spr[si].frames[frame], px, py);
}

void unit_draw_all(int cam_x, int cam_y) {
    int order[MAX_UNITS];
    int count = 0;
    int i, j, tmp;

    /* collect alive units */
    for (i = 0; i < MAX_UNITS; i++) {
        if (_pool[i].alive) {
            order[count++] = i;
        }
    }

    /* insertion sort by tile_y for painter's algorithm */
    for (i = 1; i < count; i++) {
        j = i;
        while (j > 0 && _pool[order[j]].tile_y < _pool[order[j-1]].tile_y) {
            tmp = order[j];
            order[j] = order[j-1];
            order[j-1] = tmp;
            j--;
        }
    }

    for (i = 0; i < count; i++) {
        unit_draw(&_pool[order[i]], cam_x, cam_y);
    }
}

/* ---- queries ---- */

Unit *unit_at(int tx, int ty) {
    int i;
    for (i = 0; i < MAX_UNITS; i++) {
        if (_pool[i].alive &&
            _pool[i].tile_x == tx && _pool[i].tile_y == ty) {
            return &_pool[i];
        }
    }
    return NULL;
}

Unit *unit_get(int index) {
    if (index < 0 || index >= MAX_UNITS) return NULL;
    if (!_pool[index].alive) return NULL;
    return &_pool[index];
}

int unit_alive_count(int team) {
    int i, count = 0;
    for (i = 0; i < MAX_UNITS; i++) {
        if (_pool[i].alive && _pool[i].team == team)
            count++;
    }
    return count;
}

void unit_reset_done(int team) {
    int i;
    for (i = 0; i < MAX_UNITS; i++) {
        if (_pool[i].alive && _pool[i].team == team)
            _pool[i].done = 0;
    }
}
