#include <stdio.h>
#include <string.h>
#include "TOWN.H"
#include "TOWNMAP.H"
#include "MAP.H"
#include "UNIT.H"
#include "GFX.H"
#include "FONT.H"
#include "INPUT.H"
#include "TIMER.H"
#include "DIALOG.H"

#define MOVE_DELAY 150   /* ms between tile moves */
#define SPR_HERO   0
#define SPR_NPC    1

/* NPC data */
#define MAX_NPCS 2

typedef struct {
    int tile_x, tile_y;
    int spr_idx;
    const char *text;
} NPC_Data;

static NPC_Data _npcs[MAX_NPCS] = {
    { TMAP_NPC_X, TMAP_NPC_Y, SPR_NPC,
      "Welcome to the village! Monsters have appeared to the south. Please be careful." },
    { 22, 8, SPR_NPC,
      "I heard there is a battle brewing nearby. Head south if you are ready to fight!" }
};

static Unit *_player = NULL;
static Unit *_npc_units[MAX_NPCS];
static int _cam_x = 0;
static int _cam_y = 0;
static unsigned long _last_move = 0;

/* ---- camera ---- */

static void _update_camera(void) {
    if (!_player) return;
    _cam_x = _player->tile_x - MAP_VIEW_COLS / 2;
    _cam_y = _player->tile_y - MAP_VIEW_ROWS / 2;

    if (_cam_x < 0) _cam_x = 0;
    if (_cam_y < 0) _cam_y = 0;
    if (_cam_x + MAP_VIEW_COLS > TMAP_W)
        _cam_x = TMAP_W - MAP_VIEW_COLS;
    if (_cam_y + MAP_VIEW_ROWS > TMAP_H)
        _cam_y = TMAP_H - MAP_VIEW_ROWS;
}

/* ---- init ---- */

void town_init(void) {
    int i;

    unit_init_pool();
    map_set(tmap_tiles, tmap_pass, TMAP_W, TMAP_H);

    /* spawn player */
    _player = unit_spawn(TEAM_PLAYER, "Hero", 50, 10, 4,
                         TMAP_PLAYER_X, TMAP_PLAYER_Y, SPR_HERO);
    if (_player) _player->spr_frame = 0; /* facing down */

    /* spawn NPCs */
    for (i = 0; i < MAX_NPCS; i++) {
        _npc_units[i] = unit_spawn(TEAM_PLAYER,
            "NPC", 1, 0, 0,
            _npcs[i].tile_x, _npcs[i].tile_y,
            _npcs[i].spr_idx);
    }

    _update_camera();
    _last_move = timer_ms();
}

/* ---- NPC adjacency check ---- */

static int _check_npc_interact(void) {
    int dx[4] = { 0, 0, -1, 1 };
    int dy[4] = { -1, 1, 0, 0 };
    int i, d;
    int px, py;

    if (!_player) return 0;
    px = _player->tile_x;
    py = _player->tile_y;

    for (d = 0; d < 4; d++) {
        int nx = px + dx[d];
        int ny = py + dy[d];
        for (i = 0; i < MAX_NPCS; i++) {
            if (_npcs[i].tile_x == nx && _npcs[i].tile_y == ny) {
                dlg_show(_npcs[i].text);
                return 1;
            }
        }
    }
    return 0;
}

/* ---- update ---- */

int town_update(void) {
    int key;
    int nx, ny;
    unsigned long now;

    /* dialogue active: process dialogue only */
    if (dlg_active()) {
        dlg_update();
        return TOWN_TRIGGER_NONE;
    }

    key = input_read_key();
    now = timer_ms();

    /* interaction */
    if (key == KEY_SPACE || key == KEY_ENTER) {
        /* check if on trigger tile */
        if (_player &&
            _player->tile_x == TMAP_TRIGGER_X &&
            _player->tile_y == TMAP_TRIGGER_Y) {
            return TOWN_TRIGGER_BATTLE;
        }
        _check_npc_interact();
        return TOWN_TRIGGER_NONE;
    }

    /* movement with cooldown */
    if (!_player) return TOWN_TRIGGER_NONE;
    if (now - _last_move < MOVE_DELAY) return TOWN_TRIGGER_NONE;

    nx = _player->tile_x;
    ny = _player->tile_y;

    if (key == KEY_UP)    { ny--; if (_player) _player->spr_frame = 3; }
    else if (key == KEY_DOWN)  { ny++; if (_player) _player->spr_frame = 0; }
    else if (key == KEY_LEFT)  { nx--; if (_player) _player->spr_frame = 1; }
    else if (key == KEY_RIGHT) { nx++; if (_player) _player->spr_frame = 2; }
    else return TOWN_TRIGGER_NONE;

    /* collision: passable and no unit blocking */
    if (map_passable(nx, ny) && !unit_at(nx, ny)) {
        _player->tile_x = nx;
        _player->tile_y = ny;
        _last_move = now;
        _update_camera();

        /* auto-trigger: check if player stepped on trigger */
        if (nx == TMAP_TRIGGER_X && ny == TMAP_TRIGGER_Y) {
            /* show hint but don't auto-trigger */
        }
    }

    return TOWN_TRIGGER_NONE;
}

/* ---- draw ---- */

void town_draw(void) {
    char buf[32];

    /* tiles */
    map_draw(_cam_x, _cam_y);

    /* units (NPCs + player, y-sorted) */
    unit_draw_all(_cam_x, _cam_y);

    /* status bar */
    gfx_fill_rect(0, 176, 320, 24, 1);
    gfx_hline(0, 319, 176, 8);

    if (_player) {
        sprintf(buf, "HP:%d/%d", _player->hp, _player->max_hp);
        font_puts(4, 180, buf, 15);
    }
    font_puts(240, 180, "Village", 10);

    /* trigger hint */
    if (_player &&
        _player->tile_x == TMAP_TRIGGER_X &&
        _player->tile_y == TMAP_TRIGGER_Y) {
        font_puts(4, 190, "SPACE: Enter Battle", 56);
    }

    /* dialogue overlay */
    dlg_draw();
}
