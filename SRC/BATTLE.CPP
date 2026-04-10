#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "BATTLE.H"
#include "BTLMAP.H"
#include "MAP.H"
#include "UNIT.H"
#include "GFX.H"
#include "FONT.H"
#include "INPUT.H"
#include "TIMER.H"
#include "DIALOG.H"

#define BS_PRE_DIALOG  0
#define BS_IDLE        1
#define BS_UNIT_SEL    2
#define BS_UNIT_MOVED  3
#define BS_ATTACKING   4
#define BS_ENEMY_PHASE 5
#define BS_RESULT      6

#define SPR_HERO 0
#define SPR_ENM  2
#define COL_CURSOR_W  15
#define COL_CURSOR_Y  56
#define COL_MOVE_BLUE 132
#define COL_DMG_RED   24
#define COL_DONE_DIM  5

static const struct { const char *name; int hp, atk, mov; } _ctbl[4] = {
    {"Hero",50,10,4}, {"Archer",35,15,3}, {"Knight",70,12,3}, {"Mage",30,18,3}
};

static int _state, _result, _cur_x, _cur_y;
static Unit *_sel_unit;
static unsigned long _blink_time;
static unsigned char _move_map[BMAP_W * BMAP_H];
static int _qx[256], _qy[256], _qd[256];
static Unit *_atk_target;
static unsigned long _atk_start;
static int _atk_dmg, _enemy_idx;
static unsigned long _enemy_timer;

/* forward declarations */
static int  _all_player_done(void);
static void _start_enemy_phase(void);
static void _show_end(int end);

static int _manhattan(int x1, int y1, int x2, int y2) {
    int dx = x1 - x2, dy = y1 - y2;
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    return dx + dy;
}

static void _calc_move_range(Unit *u) {
    int head = 0, tail = 0, i, nx, ny;
    int dx[4] = {0,0,-1,1}, dy[4] = {-1,1,0,0};
    memset(_move_map, 0, sizeof(_move_map));
    _qx[0] = u->tile_x; _qy[0] = u->tile_y; _qd[0] = 0;
    tail = 1;
    _move_map[u->tile_y * BMAP_W + u->tile_x] = 1;
    while (head < tail) {
        int cx = _qx[head], cy = _qy[head], cd = _qd[head];
        head++;
        if (cd >= u->mov) continue;
        for (i = 0; i < 4; i++) {
            Unit *blk;
            nx = cx + dx[i]; ny = cy + dy[i];
            if (nx < 0 || ny < 0 || nx >= BMAP_W || ny >= BMAP_H) continue;
            if (_move_map[ny * BMAP_W + nx]) continue;
            if (!map_passable(nx, ny)) continue;
            blk = unit_at(nx, ny);
            if (blk && blk->team != u->team) continue;
            _move_map[ny * BMAP_W + nx] = 1;
            if (tail < 255) {
                _qx[tail] = nx; _qy[tail] = ny; _qd[tail] = cd + 1;
                tail++;
            }
        }
    }
    for (i = 0; i < BMAP_W * BMAP_H; i++) {
        if (_move_map[i]) {
            Unit *occ = unit_at(i % BMAP_W, i / BMAP_W);
            if (occ && occ != u) _move_map[i] = 0;
        }
    }
    _move_map[u->tile_y * BMAP_W + u->tile_x] = 1;
}

static Unit *_nearest_player(Unit *e) {
    int i, d, best_d = 9999;
    Unit *best = NULL;
    for (i = 0; i < MAX_UNITS; i++) {
        Unit *u = unit_get(i);
        if (!u || u->team != TEAM_PLAYER) continue;
        d = _manhattan(e->tile_x, e->tile_y, u->tile_x, u->tile_y);
        if (d < best_d) { best_d = d; best = u; }
    }
    return best;
}

static int _has_adj_enemy(Unit *u, int team) {
    int dx[4]={0,0,-1,1}, dy[4]={-1,1,0,0}, i;
    for (i = 0; i < 4; i++) {
        Unit *a = unit_at(u->tile_x+dx[i], u->tile_y+dy[i]);
        if (a && a->team == team && a->alive) return 1;
    }
    return 0;
}

static Unit *_get_adj_target(Unit *u, int team) {
    int dx[4]={0,0,-1,1}, dy[4]={-1,1,0,0}, i;
    for (i = 0; i < 4; i++) {
        Unit *a = unit_at(u->tile_x+dx[i], u->tile_y+dy[i]);
        if (a && a->team == team && a->alive) return a;
    }
    return NULL;
}

static int _check_end(void) {
    if (unit_alive_count(TEAM_ENEMY) == 0) return BTL_VICTORY;
    if (unit_alive_count(TEAM_PLAYER) == 0) return BTL_DEFEAT;
    return BTL_ONGOING;
}

static void _show_end(int end) {
    _result = end;
    dlg_show(end == BTL_VICTORY ?
        "Victory! All enemies defeated!" : "Defeat... Your party has fallen.");
    _state = BS_RESULT;
}

static int _all_player_done(void) {
    int i;
    for (i = 0; i < MAX_UNITS; i++) {
        Unit *u = unit_get(i);
        if (u && u->team == TEAM_PLAYER && !u->done) return 0;
    }
    return 1;
}

static void _start_enemy_phase(void) {
    _state = BS_ENEMY_PHASE;
    _enemy_idx = 0;
    _enemy_timer = timer_ms();
}

/* ---- init ---- */

void battle_init(const int *selected, int sel_count) {
    int i, px[4] = {6,8,13,15};
    unit_init_pool();
    map_set(bmap_tiles, bmap_pass, BMAP_W, BMAP_H);
    for (i = 0; i < sel_count && i < 4; i++) {
        int ci = selected[i];
        if (ci >= 0 && ci < 4)
            unit_spawn(TEAM_PLAYER, _ctbl[ci].name,
                       _ctbl[ci].hp, _ctbl[ci].atk, _ctbl[ci].mov,
                       px[i], 9, SPR_HERO);
    }
    unit_spawn(TEAM_ENEMY, "Goblin", 25, 8, 3, 5, 1, SPR_ENM);
    unit_spawn(TEAM_ENEMY, "Orc",    40,10, 2, 10,1, SPR_ENM);
    unit_spawn(TEAM_ENEMY, "Wolf",   20,12, 4, 14,0, SPR_ENM);
    _cur_x = 10; _cur_y = 5; _sel_unit = NULL;
    _result = BTL_ONGOING; _blink_time = timer_ms();
    _state = BS_PRE_DIALOG;
    dlg_show("Enemy forces spotted! Prepare for battle!");
    input_flush();
}

/* ---- player sub-states ---- */

static void _update_idle(void) {
    int key = input_read_key();
    Unit *u;
    if (key == KEY_UP && _cur_y > 0) _cur_y--;
    if (key == KEY_DOWN && _cur_y < BMAP_H-1) _cur_y++;
    if (key == KEY_LEFT && _cur_x > 0) _cur_x--;
    if (key == KEY_RIGHT && _cur_x < BMAP_W-1) _cur_x++;
    if (key == KEY_SPACE || key == KEY_ENTER) {
        u = unit_at(_cur_x, _cur_y);
        if (u && u->team == TEAM_PLAYER && !u->done) {
            _sel_unit = u;
            _calc_move_range(u);
            _state = BS_UNIT_SEL;
        }
    }
}

static void _update_unit_sel(void) {
    int key = input_read_key();
    if (key == KEY_UP && _cur_y > 0) _cur_y--;
    if (key == KEY_DOWN && _cur_y < BMAP_H-1) _cur_y++;
    if (key == KEY_LEFT && _cur_x > 0) _cur_x--;
    if (key == KEY_RIGHT && _cur_x < BMAP_W-1) _cur_x++;
    if (key == KEY_ESC) { _sel_unit = NULL; _state = BS_IDLE; return; }
    if ((key == KEY_SPACE || key == KEY_ENTER) &&
        _move_map[_cur_y * BMAP_W + _cur_x]) {
        _sel_unit->tile_x = _cur_x;
        _sel_unit->tile_y = _cur_y;
        if (_has_adj_enemy(_sel_unit, TEAM_ENEMY)) {
            _state = BS_UNIT_MOVED;
        } else {
            _sel_unit->done = 1; _sel_unit = NULL; _state = BS_IDLE;
            if (_all_player_done()) _start_enemy_phase();
        }
    }
}

static void _update_unit_moved(void) {
    int key = input_read_key();
    if (key == KEY_ESC) {
        _sel_unit->done = 1; _sel_unit = NULL; _state = BS_IDLE;
        if (_all_player_done()) _start_enemy_phase();
        return;
    }
    if (key == KEY_SPACE || key == KEY_ENTER) {
        _atk_target = _get_adj_target(_sel_unit, TEAM_ENEMY);
        if (_atk_target) {
            _atk_dmg = _sel_unit->atk;
            _atk_target->hp -= _atk_dmg;
            if (_atk_target->hp <= 0) { _atk_target->hp = 0; _atk_target->alive = 0; }
            _atk_start = timer_ms();
            _state = BS_ATTACKING;
        }
    }
}

static void _update_attacking(void) {
    int end;
    if (timer_ms() - _atk_start < 500) return;
    _sel_unit->done = 1; _atk_target = NULL; _sel_unit = NULL;
    end = _check_end();
    if (end != BTL_ONGOING) { _show_end(end); return; }
    _state = BS_IDLE;
    if (_all_player_done()) _start_enemy_phase();
}

static void _update_enemy_phase(void) {
    int i, end, tx, ty, best_x, best_y, best_d, d;
    Unit *eu = NULL, *target, *adj;
    if (timer_ms() - _enemy_timer < 400) return;
    for (i = _enemy_idx; i < MAX_UNITS; i++) {
        Unit *u = unit_get(i);
        if (u && u->team == TEAM_ENEMY && !u->done) { eu = u; _enemy_idx = i; break; }
    }
    if (!eu) {
        unit_reset_done(TEAM_ENEMY); unit_reset_done(TEAM_PLAYER);
        _state = BS_IDLE; return;
    }
    target = _nearest_player(eu);
    if (target) {
        _calc_move_range(eu);
        best_x = eu->tile_x; best_y = eu->tile_y;
        best_d = _manhattan(best_x, best_y, target->tile_x, target->tile_y);
        for (ty = 0; ty < BMAP_H; ty++)
            for (tx = 0; tx < BMAP_W; tx++) {
                if (!_move_map[ty * BMAP_W + tx]) continue;
                d = _manhattan(tx, ty, target->tile_x, target->tile_y);
                if (d < best_d) { best_d = d; best_x = tx; best_y = ty; }
            }
        eu->tile_x = best_x; eu->tile_y = best_y;
        adj = _get_adj_target(eu, TEAM_PLAYER);
        if (adj) {
            adj->hp -= eu->atk;
            if (adj->hp <= 0) { adj->hp = 0; adj->alive = 0; }
        }
    }
    eu->done = 1; _enemy_idx++; _enemy_timer = timer_ms();
    end = _check_end();
    if (end != BTL_ONGOING) _show_end(end);
}

/* ---- update ---- */

void battle_update(void) {
    switch (_state) {
    case BS_PRE_DIALOG:
        if (dlg_update()) { _state = BS_IDLE; input_flush(); } break;
    case BS_IDLE:       _update_idle(); break;
    case BS_UNIT_SEL:   _update_unit_sel(); break;
    case BS_UNIT_MOVED: _update_unit_moved(); break;
    case BS_ATTACKING:  _update_attacking(); break;
    case BS_ENEMY_PHASE: _update_enemy_phase(); break;
    case BS_RESULT:     dlg_update(); break;
    }
}

/* ---- draw helpers ---- */

static void _draw_move_range(void) {
    int tx, ty, px, py, r, c;
    for (ty = 0; ty < BMAP_H; ty++)
        for (tx = 0; tx < BMAP_W; tx++) {
            if (!_move_map[ty * BMAP_W + tx]) continue;
            px = tx * MAP_TILE_W; py = ty * MAP_TILE_H;
            for (r = 0; r < MAP_TILE_H; r++)
                for (c = 0; c < MAP_TILE_W; c++)
                    if ((r + c) % 2 == 0)
                        gfx_pixel(px + c, py + r, COL_MOVE_BLUE);
        }
}

static void _draw_done_overlay(void) {
    int i, px, py, r, c;
    for (i = 0; i < MAX_UNITS; i++) {
        Unit *u = unit_get(i);
        if (!u || !u->done || u->team != TEAM_PLAYER) continue;
        px = u->tile_x * MAP_TILE_W; py = u->tile_y * MAP_TILE_H;
        for (r = 0; r < MAP_TILE_H; r++)
            for (c = 0; c < MAP_TILE_W; c++)
                if ((r + c) % 2 == 0) gfx_pixel(px + c, py + r, COL_DONE_DIM);
    }
}

/* ---- draw ---- */

void battle_draw(void) {
    unsigned long now;
    int blink;
    char buf[64];

    map_draw(0, 0);
    if (_state == BS_UNIT_SEL) _draw_move_range();
    unit_draw_all(0, 0);
    _draw_done_overlay();

    /* attack flash + damage */
    if (_state == BS_ATTACKING && _atk_target) {
        now = timer_ms();
        if (_atk_target->alive && ((now - _atk_start) / 80) % 2 == 0) {
            int px = _atk_target->tile_x * MAP_TILE_W;
            int py = _atk_target->tile_y * MAP_TILE_H;
            gfx_fill_rect(px, py, MAP_TILE_W, MAP_TILE_H, 15);
        }
        {
            int px = _atk_target->tile_x * MAP_TILE_W;
            int py = _atk_target->tile_y * MAP_TILE_H - 8;
            if (py < 0) py = 0;
            sprintf(buf, "-%d", _atk_dmg);
            font_puts(px, py, buf, COL_DMG_RED);
        }
    }

    /* cursor */
    now = timer_ms();
    blink = ((now - _blink_time) / 300) % 2;
    gfx_rect(_cur_x*MAP_TILE_W, _cur_y*MAP_TILE_H,
             MAP_TILE_W, MAP_TILE_H, blink ? COL_CURSOR_Y : COL_CURSOR_W);

    /* adjacent enemy highlight */
    if (_state == BS_UNIT_MOVED && _sel_unit) {
        int dx[4]={0,0,-1,1}, dy[4]={-1,1,0,0}, d;
        for (d = 0; d < 4; d++) {
            Unit *a = unit_at(_sel_unit->tile_x+dx[d], _sel_unit->tile_y+dy[d]);
            if (a && a->team == TEAM_ENEMY)
                gfx_rect(a->tile_x*MAP_TILE_W, a->tile_y*MAP_TILE_H,
                         MAP_TILE_W, MAP_TILE_H, COL_DMG_RED);
        }
    }

    /* status bar */
    gfx_fill_rect(0, 176, 320, 24, 1);
    gfx_hline(0, 319, 176, 8);
    if (_state == BS_ENEMY_PHASE) {
        font_puts(4, 178, "Turn: Enemy", 15);
        font_puts(4, 190, "Processing...", 10);
    } else {
        Unit *hover = unit_at(_cur_x, _cur_y);
        if (hover)
            sprintf(buf, "%s HP:%d/%d ATK:%d", hover->name,
                    hover->hp, hover->max_hp, hover->atk);
        else
            strcpy(buf, "Turn: Player");
        font_puts(4, 178, buf, 15);
        if (_state == BS_IDLE)
            font_puts(4, 190, "SPACE:Select unit", 10);
        else if (_state == BS_UNIT_SEL)
            font_puts(4, 190, "SPACE:Move  ESC:Cancel", 10);
        else if (_state == BS_UNIT_MOVED)
            font_puts(4, 190, "SPACE:Attack  ESC:Skip", 10);
    }
    dlg_draw();
}

int battle_result(void) {
    if (_state == BS_RESULT && !dlg_active()) return _result;
    return BTL_ONGOING;
}
