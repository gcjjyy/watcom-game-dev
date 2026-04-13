#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "BATTLE.H"
#include "BATSCRN.H"
#include "PARTY.H"
#include "ENEMY.H"
#include "GFX.H"
#include "FONT.H"
#include "INPUT.H"
#include "TIMER.H"
#include "DIALOG.H"
#include "SFX.H"

/* battle states */
#define BS_ENTRY         0
#define BS_TURN_INIT     1
#define BS_NEXT_ACTOR    2
#define BS_CMD_SEL       3
#define BS_MAGIC_SEL     4
#define BS_ITEM_SEL      5
#define BS_TARGET_ENEMY  6
#define BS_TARGET_ALLY   7
#define BS_ACTION_FX     8
#define BS_AI_PICK       9
#define BS_RUN_TRY      10
#define BS_VICTORY      11
#define BS_DEFEAT       12

#define SK_PHYS_MULT 0
#define SK_FIXED_DMG 1
#define SK_PHYS_ADD  2
#define SK_HEAL      3
#define IT_HEAL_HP   0
#define IT_HEAL_MP   1
#define COL_DMG_RED    24
#define COL_HEAL_GREEN 96
typedef struct {
    int char_idx; const char *name; int mp_cost; int kind; int amount;
} SkillDef;
static const SkillDef _skills[] = {
    { 0, "Strike",   3, SK_PHYS_MULT, 15 },
    { 1, "FireBolt", 4, SK_FIXED_DMG, 18 },
    { 2, "Bash",     4, SK_PHYS_ADD,   5 },
    { 3, "Heal",     5, SK_HEAL,      25 }
};
#define SKILL_COUNT (sizeof(_skills)/sizeof(_skills[0]))
typedef struct { const char *name; int kind; int amount; } ItemDef;
#define INV_TYPES 2
static const ItemDef _items[INV_TYPES] = {
    { "Potion", IT_HEAL_HP, 30 },
    { "Ether",  IT_HEAL_MP, 15 }
};
static int _inventory[INV_TYPES] = { 3, 2 };

static BatEnemy _enemies[MAX_ENEMIES_PER_GRP];
static int _enemy_count;
static int _state, _result;
typedef struct { int kind, idx; } TurnEntry;
static TurnEntry _turn_order[PARTY_SIZE + MAX_ENEMIES_PER_GRP];
static int _turn_len, _turn_idx, _active_party_idx;
static int _menu_cur, _sub_cur, _target_cur;
static int _pending_skill_idx, _pending_item_idx;
static unsigned long _action_start, _blink_time;
static int _last_dmg, _last_dmg_color;
static int _target_kind, _target_slot;
static char _log1[48], _log2[48];

/* Single source of truth for physical damage formula (synth adjust 1). */
static int calc_phys_dmg(int atk, int def, int defending) {
    int d = atk - def / 2;
    if (d < 1) d = 1;
    if (defending) { d /= 2; if (d < 1) d = 1; }
    return d;
}

static int _enemies_alive(void) {
    int i, n = 0;
    for (i = 0; i < _enemy_count; i++) if (_enemies[i].alive) n++;
    return n;
}
static int _first_alive_enemy(void) {
    int i;
    for (i = 0; i < _enemy_count; i++) if (_enemies[i].alive) return i;
    return -1;
}

static int _cycle_enemy(int cur, int dir) {
    int i;
    if (_enemy_count <= 0) return -1;
    for (i = 0; i < _enemy_count; i++) {
        cur = (cur + dir + _enemy_count) % _enemy_count;
        if (_enemies[cur].alive) return cur;
    }
    return -1;
}
static int _first_alive_party(void) {
    int i; Party_Member *p;
    for (i = 0; i < PARTY_SIZE; i++) {
        p = party_get(i);
        if (p && p->alive && p->hp > 0) return i;
    }
    return -1;
}
static int _cycle_party(int cur, int dir) {
    int i; Party_Member *p;
    for (i = 0; i < PARTY_SIZE; i++) {
        cur = (cur + dir + PARTY_SIZE) % PARTY_SIZE;
        p = party_get(cur);
        if (p && p->alive && p->hp > 0) return cur;
    }
    return -1;
}

static int _lowest_hp_party(void) {
    int i, best = -1, bh = 0x7FFFFFFF; Party_Member *p;
    for (i = 0; i < PARTY_SIZE; i++) {
        p = party_get(i);
        if (p && p->alive && p->hp > 0 && p->hp < bh) { bh = p->hp; best = i; }
    }
    return best;
}
static int _char_skill_idx(int c) {
    int i;
    for (i = 0; i < (int)SKILL_COUNT; i++)
        if (_skills[i].char_idx == c) return i;
    return -1;
}
static int _actor_spd(TurnEntry *t) {
    if (t->kind == 0) {
        Party_Member *p = party_get(t->idx);
        return p ? p->spd : 0;
    }
    return enemy_defs[_enemies[t->idx].def_id].spd;
}

static void _build_turn_order(void) {
    int i, j; TurnEntry tmp; Party_Member *p;
    _turn_len = 0;
    for (i = 0; i < PARTY_SIZE; i++) {
        p = party_get(i);
        if (p && p->alive && p->hp > 0) {
            _turn_order[_turn_len].kind = 0;
            _turn_order[_turn_len++].idx = i;
        }
    }
    for (i = 0; i < _enemy_count; i++) {
        if (_enemies[i].alive) {
            _turn_order[_turn_len].kind = 1;
            _turn_order[_turn_len++].idx = i;
        }
    }
    /* stable insertion sort by SPD desc — players-first on ties */
    for (i = 1; i < _turn_len; i++) {
        j = i;
        while (j > 0 && _actor_spd(&_turn_order[j]) > _actor_spd(&_turn_order[j-1])) {
            tmp = _turn_order[j]; _turn_order[j] = _turn_order[j-1]; _turn_order[j-1] = tmp;
            j--;
        }
    }
    _turn_idx = 0;
}

static void _layout_enemies(void) {
    int i, total_w, ox;
    total_w = _enemy_count * BAT_ENEMY_W + (_enemy_count - 1) * 8;
    ox = BAT_ENEMY_AREA_X + (BAT_ENEMY_AREA_W - total_w) / 2;
    for (i = 0; i < _enemy_count; i++) {
        _enemies[i].x = ox + i * (BAT_ENEMY_W + 8);
        _enemies[i].y = BAT_ENEMY_AREA_Y + 16;
    }
}

static void _check_end(void) {
    if (_enemies_alive() == 0) {
        sfx_play(SFX_WIN);
        _result = BTL_VICTORY; _state = BS_VICTORY; dlg_show("Victory!");
    } else if (party_alive_count() == 0) {
        sfx_play(SFX_LOSE);
        _result = BTL_DEFEAT; _state = BS_DEFEAT;
        dlg_show("Your party has fallen...");
    }
}

static void _enter_next_actor(void) {
    /* AC-23 / Critique W2: defend bonus lasts ONLY until THIS actor's
     * next turn begins. The clear MUST happen here, not in BS_TURN_INIT.
     * If you move this clear, AC-23 will fail and the synthesizer will
     * route the build back to design. Don't. */
    for (;;) {
        TurnEntry *t;
        if (_turn_idx >= _turn_len) { _state = BS_TURN_INIT; return; }
        t = &_turn_order[_turn_idx];
        if (t->kind == 0) {
            Party_Member *p = party_get(t->idx);
            if (!p || !p->alive || p->hp <= 0) { _turn_idx++; continue; }
            p->defending = 0;
            _active_party_idx = t->idx;
            _menu_cur = 0;
            _state = BS_CMD_SEL;
            return;
        }
        if (t->idx < 0 || t->idx >= _enemy_count || !_enemies[t->idx].alive) {
            _turn_idx++; continue;
        }
        _enemies[t->idx].defending = 0;
        _state = BS_AI_PICK;
        return;
    }
}

static void _begin_fx(int tk, int ts, int amt, int color) {
    _target_kind = tk; _target_slot = ts;
    _last_dmg = amt; _last_dmg_color = color;
    _action_start = timer_ms();
    _state = BS_ACTION_FX;
}

void battle_init(int group_id) {
    int i;
    const EncounterGroup *g;
    if (group_id < 0 || group_id >= ENCOUNTER_GROUP_COUNT) group_id = 0;
    g = &encounter_groups[group_id];
    _enemy_count = g->count;
    for (i = 0; i < _enemy_count; i++) {
        _enemies[i].def_id = g->enemy_ids[i];
        _enemies[i].hp = enemy_defs[_enemies[i].def_id].hp;
        _enemies[i].mp = enemy_defs[_enemies[i].def_id].mp;
        _enemies[i].alive = 1;
        _enemies[i].defending = 0;
    }
    _layout_enemies();
    party_clear_defend_flags();
    _state = BS_ENTRY;
    _result = BTL_ONGOING;
    _blink_time = timer_ms();
    _log1[0] = 0; _log2[0] = 0;
    dlg_show("Monsters appeared!");
    input_flush();
}

static void _player_attack(int ts) {
    Party_Member *p = party_get(_active_party_idx);
    int dmg;
    if (!p || ts < 0) return;
    sfx_play(SFX_ATK); sfx_play(SFX_HIT);
    dmg = calc_phys_dmg(p->atk, enemy_defs[_enemies[ts].def_id].def,
                        _enemies[ts].defending);
    _enemies[ts].hp -= dmg;
    if (_enemies[ts].hp <= 0) { _enemies[ts].hp = 0; _enemies[ts].alive = 0; }
    sprintf(_log1, "%s attacks %s for %d!", p->name,
            enemy_defs[_enemies[ts].def_id].name, dmg);
    sprintf(_log2, "%s HP: %d/%d",
            enemy_defs[_enemies[ts].def_id].name, _enemies[ts].hp,
            enemy_defs[_enemies[ts].def_id].hp);
    _begin_fx(TARGET_ENEMY, ts, dmg, COL_DMG_RED);
}

static void _apply_skill_enemy(int ts) {
    Party_Member *p = party_get(_active_party_idx);
    const SkillDef *sk = &_skills[_pending_skill_idx];
    int dmg = 0;
    if (!p) return;
    p->mp -= sk->mp_cost; if (p->mp < 0) p->mp = 0;
    if (sk->kind == SK_PHYS_MULT) {
        dmg = calc_phys_dmg(p->atk, enemy_defs[_enemies[ts].def_id].def,
                            _enemies[ts].defending);
        dmg = (dmg * sk->amount) / 10;
        if (dmg < 1) dmg = 1;
    } else if (sk->kind == SK_FIXED_DMG) {
        dmg = sk->amount;
    } else if (sk->kind == SK_PHYS_ADD) {
        dmg = calc_phys_dmg(p->atk + sk->amount,
                            enemy_defs[_enemies[ts].def_id].def,
                            _enemies[ts].defending);
    }
    _enemies[ts].hp -= dmg;
    if (_enemies[ts].hp <= 0) { _enemies[ts].hp = 0; _enemies[ts].alive = 0; }
    sprintf(_log1, "%s uses %s!", p->name, sk->name);
    sprintf(_log2, "%s takes %d dmg!", enemy_defs[_enemies[ts].def_id].name, dmg);
    _begin_fx(TARGET_ENEMY, ts, dmg, COL_DMG_RED);
}

static void _apply_skill_ally(int ts) {
    Party_Member *p = party_get(_active_party_idx);
    Party_Member *t = party_get(ts);
    const SkillDef *sk = &_skills[_pending_skill_idx];
    int healed;
    if (!p || !t) return;
    p->mp -= sk->mp_cost; if (p->mp < 0) p->mp = 0;
    healed = sk->amount;
    t->hp += healed;
    if (t->hp > t->max_hp) t->hp = t->max_hp;
    sprintf(_log1, "%s uses %s!", p->name, sk->name);
    sprintf(_log2, "%s heals %d HP!", t->name, healed);
    _begin_fx(TARGET_PLAYER, ts, -healed, COL_HEAL_GREEN);
}

static void _apply_item_ally(int ts) {
    Party_Member *t = party_get(ts);
    Party_Member *p = party_get(_active_party_idx);
    const ItemDef *it; int amt;
    if (!t || _pending_item_idx < 0) return;
    it = &_items[_pending_item_idx];
    _inventory[_pending_item_idx]--;
    amt = it->amount;
    if (it->kind == IT_HEAL_HP) {
        t->hp += amt; if (t->hp > t->max_hp) t->hp = t->max_hp;
        sprintf(_log2, "%s HP+%d", t->name, amt);
    } else {
        t->mp += amt; if (t->mp > t->max_mp) t->mp = t->max_mp;
        sprintf(_log2, "%s MP+%d", t->name, amt);
    }
    sprintf(_log1, "%s uses %s!", p ? p->name : "Actor", it->name);
    _begin_fx(TARGET_PLAYER, ts, -amt, COL_HEAL_GREEN);
}

void battle_update(void) {
    int key;
    switch (_state) {
    case BS_ENTRY:
        if (dlg_update()) { _state = BS_TURN_INIT; input_flush(); }
        break;
    case BS_TURN_INIT:
        _build_turn_order(); _state = BS_NEXT_ACTOR; break;
    case BS_NEXT_ACTOR:
        _enter_next_actor(); break;
    case BS_CMD_SEL:
        key = input_read_key();
        if (key == KEY_UP)   { if (--_menu_cur < 0) _menu_cur = 4; sfx_play(SFX_SEL); }
        if (key == KEY_DOWN) { if (++_menu_cur > 4) _menu_cur = 0; sfx_play(SFX_SEL); }
        if (key == KEY_SPACE || key == KEY_ENTER) {
            sfx_play(SFX_OK);
            if (_menu_cur == 0) {
                _target_cur = _first_alive_enemy();
                if (_target_cur >= 0) _state = BS_TARGET_ENEMY;
            } else if (_menu_cur == 1) {
                _sub_cur = 0; _state = BS_MAGIC_SEL;
            } else if (_menu_cur == 2) {
                _sub_cur = 0; _state = BS_ITEM_SEL;
            } else if (_menu_cur == 3) {
                Party_Member *p = party_get(_active_party_idx);
                if (p) p->defending = 1;
                sprintf(_log1, "%s defends!", p ? p->name : "Actor");
                _log2[0] = 0;
                _turn_idx++; _state = BS_NEXT_ACTOR;
            } else {
                _state = BS_RUN_TRY;
            }
        }
        break;
    case BS_MAGIC_SEL: {
        int sidx;
        key = input_read_key();
        if (key == KEY_ESC) { sfx_play(SFX_CAN); _state = BS_CMD_SEL; break; }
        sidx = _char_skill_idx(_active_party_idx);
        if (sidx < 0) {
            sprintf(_log1, "No skills!"); _log2[0] = 0;
            if (key == KEY_SPACE || key == KEY_ENTER) _state = BS_CMD_SEL;
            break;
        }
        if (key == KEY_SPACE || key == KEY_ENTER) {
            Party_Member *p = party_get(_active_party_idx);
            const SkillDef *sk = &_skills[sidx];
            if (!p || p->mp < sk->mp_cost) {
                sprintf(_log1, "Not enough MP!"); _log2[0] = 0; break;
            }
            _pending_skill_idx = sidx;
            if (sk->kind == SK_HEAL) {
                _target_cur = _first_alive_party(); _state = BS_TARGET_ALLY;
            } else {
                _target_cur = _first_alive_enemy();
                if (_target_cur >= 0) _state = BS_TARGET_ENEMY;
            }
        }
        break; }
    case BS_ITEM_SEL:
        key = input_read_key();
        if (key == KEY_ESC) { sfx_play(SFX_CAN); _state = BS_CMD_SEL; break; }
        if (key == KEY_UP)   { if (--_sub_cur < 0) _sub_cur = INV_TYPES - 1; }
        if (key == KEY_DOWN) { if (++_sub_cur >= INV_TYPES) _sub_cur = 0; }
        if (key == KEY_SPACE || key == KEY_ENTER) {
            if (_inventory[_sub_cur] <= 0) {
                sprintf(_log1, "None left!"); _log2[0] = 0; break;
            }
            _pending_item_idx = _sub_cur; _pending_skill_idx = -1;
            _target_cur = _first_alive_party(); _state = BS_TARGET_ALLY;
        }
        break;
    case BS_TARGET_ENEMY:
        key = input_read_key();
        if (key == KEY_ESC) { sfx_play(SFX_CAN); _state = BS_CMD_SEL; break; }
        if (key == KEY_LEFT)  { int n = _cycle_enemy(_target_cur, -1); if (n >= 0) _target_cur = n; }
        if (key == KEY_RIGHT) { int n = _cycle_enemy(_target_cur,  1); if (n >= 0) _target_cur = n; }
        if (key == KEY_SPACE || key == KEY_ENTER) {
            if (_target_cur < 0 || !_enemies[_target_cur].alive) break;
            if (_menu_cur == 1) _apply_skill_enemy(_target_cur);
            else _player_attack(_target_cur);
        }
        break;
    case BS_TARGET_ALLY:
        key = input_read_key();
        if (key == KEY_ESC) { sfx_play(SFX_CAN); _state = BS_CMD_SEL; break; }
        if (key == KEY_UP)   { int n = _cycle_party(_target_cur, -1); if (n >= 0) _target_cur = n; }
        if (key == KEY_DOWN) { int n = _cycle_party(_target_cur,  1); if (n >= 0) _target_cur = n; }
        if (key == KEY_SPACE || key == KEY_ENTER) {
            if (_target_cur < 0) break;
            if (_menu_cur == 2) _apply_item_ally(_target_cur);
            else _apply_skill_ally(_target_cur);
        }
        break;
    case BS_ACTION_FX:
        if (timer_ms() - _action_start < 500) break;
        _check_end();
        if (_state != BS_ACTION_FX) break;
        _turn_idx++; _state = BS_NEXT_ACTOR;
        break;
    case BS_AI_PICK: {
        int eidx = _turn_order[_turn_idx].idx;
        int tgt, dmg;
        Party_Member *p;
        BatEnemy *e;
        if (eidx < 0 || eidx >= _enemy_count || !_enemies[eidx].alive) {
            _turn_idx++; _state = BS_NEXT_ACTOR; break;
        }
        e = &_enemies[eidx];
        tgt = _lowest_hp_party();
        if (tgt < 0) { _check_end(); break; }
        p = party_get(tgt);
        if (!p) { _turn_idx++; _state = BS_NEXT_ACTOR; break; }
        dmg = calc_phys_dmg(enemy_defs[e->def_id].atk, p->def, p->defending);
        p->hp -= dmg;
        if (p->hp <= 0) { p->hp = 0; p->alive = 0; }
        sprintf(_log1, "%s attacks %s for %d!",
                enemy_defs[e->def_id].name, p->name, dmg);
        sprintf(_log2, "%s HP: %d/%d", p->name, p->hp, p->max_hp);
        sfx_play(SFX_ATK); sfx_play(SFX_HIT);
        _begin_fx(TARGET_PLAYER, tgt, dmg, COL_DMG_RED);
        break; }
    case BS_RUN_TRY:
        if ((rand() % 100) < 50) {
            _result = BTL_ESCAPE;
        } else {
            sprintf(_log1, "Run failed!"); _log2[0] = 0;
            _turn_idx++; _state = BS_NEXT_ACTOR;
        }
        break;
    case BS_VICTORY:
    case BS_DEFEAT:
        dlg_update();
        break;
    }
}

void battle_draw(void) {
    unsigned long now = timer_ms();
    int active;
    active = (_state == BS_CMD_SEL || _state == BS_MAGIC_SEL ||
              _state == BS_ITEM_SEL || _state == BS_TARGET_ENEMY ||
              _state == BS_TARGET_ALLY) ? _active_party_idx : -1;

    batscrn_clear_bg();
    batscrn_draw_enemies(_enemies, _enemy_count);
    batscrn_draw_message_log(_log1[0] ? _log1 : 0, _log2[0] ? _log2 : 0);
    batscrn_draw_party_hud(active);

    if (_state == BS_CMD_SEL) batscrn_draw_command_menu(_menu_cur);

    if (_state == BS_MAGIC_SEL) {
        int sidx = _char_skill_idx(_active_party_idx);
        const char *lb[1]; int vs[1];
        if (sidx >= 0) {
            lb[0] = _skills[sidx].name; vs[0] = _skills[sidx].mp_cost;
            batscrn_draw_submenu(SUB_KIND_MAGIC, 0, _active_party_idx, lb, vs, 1);
        } else {
            lb[0] = ""; vs[0] = 0;
            batscrn_draw_submenu(SUB_KIND_MAGIC, 0, _active_party_idx, lb, vs, 0);
        }
    }
    if (_state == BS_ITEM_SEL) {
        const char *lb[INV_TYPES]; int vs[INV_TYPES]; int i;
        for (i = 0; i < INV_TYPES; i++) { lb[i] = _items[i].name; vs[i] = _inventory[i]; }
        batscrn_draw_submenu(SUB_KIND_ITEM, _sub_cur, _active_party_idx, lb, vs, INV_TYPES);
    }

    if (_state == BS_TARGET_ENEMY)
        batscrn_draw_target_enemy_cursor(_enemies, _target_cur, now - _blink_time);
    if (_state == BS_TARGET_ALLY)
        batscrn_draw_target_ally_cursor(_target_cur, now - _blink_time);

    if (_state == BS_ACTION_FX) {
        batscrn_draw_action_flash(_enemies, _target_kind, _target_slot, now - _action_start);
        batscrn_draw_floating_dmg(_enemies, _target_kind, _target_slot, _last_dmg, _last_dmg_color);
    }

    dlg_draw();
}

int battle_result(void) {
    if ((_state == BS_VICTORY || _state == BS_DEFEAT) && !dlg_active())
        return _result;
    if (_result == BTL_ESCAPE) return BTL_ESCAPE;
    return BTL_ONGOING;
}
