#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FIELD.H"
#include "FLDMAP.H"
#include "ENEMY.H"
#include "PARTY.H"
#include "MAP.H"
#include "UNIT.H"
#include "GFX.H"
#include "FONT.H"
#include "INPUT.H"
#include "TIMER.H"
#include "DIALOG.H"

#define MOVE_DELAY 150
#define SPR_HERO   0

static Unit *_player = 0;
static int _cam_x = 0;
static int _cam_y = 0;
static unsigned long _last_move = 0;
static unsigned long _step_count = 0;
static int _last_group = 0;

static void _update_camera(void) {
    if (!_player) return;
    _cam_x = _player->tile_x - MAP_VIEW_COLS / 2;
    _cam_y = _player->tile_y - MAP_VIEW_ROWS / 2;
    if (_cam_x < 0) _cam_x = 0;
    if (_cam_y < 0) _cam_y = 0;
    if (_cam_x + MAP_VIEW_COLS > FMAP_W) _cam_x = FMAP_W - MAP_VIEW_COLS;
    if (_cam_y + MAP_VIEW_ROWS > FMAP_H) _cam_y = FMAP_H - MAP_VIEW_ROWS;
}

void field_init(int spawn_face_dir, int spawn_x, int spawn_y) {
    int sx, sy;

    unit_init_pool();
    map_set(fmap_tiles, fmap_pass, FMAP_W, FMAP_H);

    if (spawn_x < 0 || spawn_y < 0) {
        sx = FMAP_PLAYER_X;
        sy = FMAP_PLAYER_Y;
    } else {
        sx = spawn_x;
        sy = spawn_y;
    }
    _player = unit_spawn(TEAM_PLAYER, "Hero", 1, 0, 0, sx, sy, SPR_HERO);
    if (_player) _player->spr_frame = spawn_face_dir;

    _update_camera();
    _last_move = timer_ms();
    _last_group = 0;
}

int field_last_group(void) { return _last_group; }
int field_player_x(void)   { return _player ? _player->tile_x : 0; }
int field_player_y(void)   { return _player ? _player->tile_y : 0; }
int field_player_face(void){ return _player ? _player->spr_frame : 0; }

int field_update(void) {
    int key;
    int nx, ny;
    unsigned long now;

    if (dlg_active()) {
        dlg_update();
        return FLD_TRIGGER_NONE;
    }

    key = input_read_key();
    now = timer_ms();

    if (!_player) return FLD_TRIGGER_NONE;

    /* interact / town-return trigger (press on tile) */
    if (key == KEY_SPACE || key == KEY_ENTER) {
        if (_player->tile_x == FMAP_RETURN_X &&
            _player->tile_y == FMAP_RETURN_Y) {
            return FLD_TRIGGER_TOWN;
        }
        return FLD_TRIGGER_NONE;
    }

    if (now - _last_move < MOVE_DELAY) return FLD_TRIGGER_NONE;

    nx = _player->tile_x;
    ny = _player->tile_y;

    if (key == KEY_UP)         { ny--; _player->spr_frame = 3; }
    else if (key == KEY_DOWN)  { ny++; _player->spr_frame = 0; }
    else if (key == KEY_LEFT)  { nx--; _player->spr_frame = 1; }
    else if (key == KEY_RIGHT) { nx++; _player->spr_frame = 2; }
    else return FLD_TRIGGER_NONE;

    if (map_passable(nx, ny) && !unit_at(nx, ny)) {
        _player->tile_x = nx;
        _player->tile_y = ny;
        _last_move = now;
        _step_count++;
        _update_camera();

        /* encounter check on wild-grass tiles */
        if (fmap_encounter[ny * FMAP_W + nx]) {
            if ((rand() & 0x0F) == 0) {
                _last_group = rand() % ENCOUNTER_GROUP_COUNT;
                return FLD_TRIGGER_BATTLE;
            }
        }
        /* auto town-return: stepping onto the return tile also triggers */
        if (nx == FMAP_RETURN_X && ny == FMAP_RETURN_Y) {
            return FLD_TRIGGER_TOWN;
        }
    }

    return FLD_TRIGGER_NONE;
}

void field_draw(void) {
    char buf[48];
    Party_Member *p0;

    map_draw(_cam_x, _cam_y);
    unit_draw_all(_cam_x, _cam_y);

    gfx_fill_rect(0, 176, 320, 24, 1);
    gfx_hline(0, 319, 176, 8);

    p0 = party_get(0);
    if (p0) {
        sprintf(buf, "HP:%d/%d MP:%d/%d STEP:%lu",
                p0->hp, p0->max_hp, p0->mp, p0->max_mp,
                (unsigned long)_step_count);
        font_puts(4, 180, buf, 15);
    }
    font_puts(248, 180, "Plains", 10);

    if (_player &&
        _player->tile_x == FMAP_RETURN_X &&
        _player->tile_y == FMAP_RETURN_Y) {
        font_puts(4, 190, "SPACE: Return to Town", 56);
    }

    dlg_draw();
}
