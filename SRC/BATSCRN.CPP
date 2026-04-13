#include <stdio.h>
#include <string.h>
#include "BATSCRN.H"
#include "PARTY.H"
#include "ENEMY.H"
#include "GFX.H"
#include "FONT.H"
#include "SPRITE.H"
#include "UNIT.H"

#define COL_BORDER   8
#define COL_TEXT     15
#define COL_TEXT_DIM 5
#define COL_TEXT_SEC 10
#define COL_HINT     56
#define COL_DEFTAG   88
#define COL_HEAL     96
#define COL_ACTIVE_BG 3

void batscrn_clear_bg(void) {
    gfx_fill_rect(0, 0, 320, 200, BAT_BG_COLOR);
    /* enemy area divider */
    gfx_hline(0, 319, BAT_LOG_Y - 2, COL_BORDER);
    /* log / hud divider */
    gfx_hline(0, 319, BAT_HUD_Y - 2, COL_BORDER);
}

void batscrn_draw_enemies(const BatEnemy *enemies, int count) {
    int i;
    Sprite *s = unit_get_sprite(2);
    if (!s || s->num_frames <= 0) return;
    for (i = 0; i < count; i++) {
        if (!enemies[i].alive) continue;
        gfx_draw_csprite(s->frames[0], enemies[i].x, enemies[i].y);
    }
}

void batscrn_draw_party_hud(int active_idx) {
    char buf[32];
    int i, ry, col;
    Party_Member *p;

    gfx_fill_rect(BAT_HUD_X, BAT_HUD_Y, BAT_HUD_W, BAT_HUD_H, 1);
    gfx_rect(BAT_HUD_X, BAT_HUD_Y, BAT_HUD_W, BAT_HUD_H, COL_BORDER);

    for (i = 0; i < PARTY_SIZE; i++) {
        p = party_get(i);
        if (!p) continue;
        ry = BAT_HUD_Y + 2 + i * BAT_HUD_ROW_H;
        col = (p->alive && p->hp > 0) ? COL_TEXT : COL_TEXT_DIM;

        if (i == active_idx) {
            gfx_fill_rect(BAT_HUD_X + 1, ry - 1,
                          BAT_HUD_W - 2, BAT_HUD_ROW_H - 2,
                          COL_ACTIVE_BG);
            font_puts(BAT_HUD_X + 1, ry, ">", COL_HINT);
        }

        font_puts(BAT_HUD_X + 8, ry, p->name, col);
        sprintf(buf, "HP:%d/%d", p->hp, p->max_hp);
        font_puts(BAT_HUD_X + 64, ry, buf, col);
        sprintf(buf, "MP:%d/%d", p->mp, p->max_mp);
        font_puts(BAT_HUD_X + 128, ry, buf, col);

        if (p->defending) {
            font_puts(BAT_HUD_W - 16, ry, "[D]", COL_DEFTAG);
        }
    }
}

void batscrn_draw_message_log(const char *line1, const char *line2) {
    gfx_fill_rect(BAT_LOG_X, BAT_LOG_Y, BAT_LOG_W, BAT_LOG_H, 1);
    gfx_rect(BAT_LOG_X, BAT_LOG_Y, BAT_LOG_W, BAT_LOG_H, COL_BORDER);
    if (line1) font_puts(BAT_LOG_X + 4, BAT_LOG_Y + 4, line1, COL_TEXT);
    if (line2) font_puts(BAT_LOG_X + 4, BAT_LOG_Y + 4 + BAT_LOG_LINE_H,
                         line2, COL_TEXT_SEC);
}

void batscrn_draw_command_menu(int cursor) {
    static const char *labels[5] = {
        "Attack", "Magic", "Item", "Defend", "Run"
    };
    int i, ry;

    gfx_fill_rect(BAT_MENU_X, BAT_MENU_Y, BAT_MENU_W, BAT_MENU_H, 1);
    gfx_rect(BAT_MENU_X, BAT_MENU_Y, BAT_MENU_W, BAT_MENU_H, COL_BORDER);

    for (i = 0; i < 5; i++) {
        ry = BAT_MENU_TEXT_Y + i * BAT_MENU_ITEM_H;
        if (i == cursor) {
            font_puts(BAT_MENU_X + 4, ry, ">", COL_HINT);
        }
        font_puts(BAT_MENU_TEXT_X, ry, labels[i], COL_TEXT);
    }
}

void batscrn_draw_submenu(int kind, int cursor, int char_idx,
                          const char **labels, const int *values, int count) {
    char buf[24];
    int i, ry;
    int mx = BAT_MENU_X - 4;
    int my = BAT_MENU_Y;
    int mw = BAT_MENU_W + 4;
    int mh = BAT_MENU_H;

    (void)char_idx;

    gfx_fill_rect(mx, my, mw, mh, 1);
    gfx_rect(mx, my, mw, mh, COL_BORDER);

    if (count <= 0) {
        font_puts(mx + 6, my + 6, "None", COL_TEXT_DIM);
        return;
    }

    for (i = 0; i < count && i < 5; i++) {
        ry = my + 6 + i * BAT_MENU_ITEM_H;
        if (i == cursor) font_puts(mx + 2, ry, ">", COL_HINT);
        if (kind == SUB_KIND_MAGIC) {
            sprintf(buf, "%s %d", labels[i], values[i]);
        } else {
            sprintf(buf, "%s x%d", labels[i], values[i]);
        }
        font_puts(mx + 10, ry, buf, COL_TEXT);
    }
}

void batscrn_draw_target_enemy_cursor(const BatEnemy *enemies,
                                      int enemy_slot, unsigned long blink_time) {
    int blink;
    int col;
    if (!enemies || !enemies[enemy_slot].alive) return;
    blink = (int)((blink_time / 250) & 1);
    col = blink ? COL_HINT : COL_TEXT;
    gfx_rect(enemies[enemy_slot].x - 1, enemies[enemy_slot].y - 1,
             BAT_ENEMY_W + 2, BAT_ENEMY_H + 2, (unsigned char)col);
}

void batscrn_draw_target_ally_cursor(int party_idx, unsigned long blink_time) {
    int blink;
    int col;
    int ry;
    if (party_idx < 0 || party_idx >= PARTY_SIZE) return;
    blink = (int)((blink_time / 250) & 1);
    col = blink ? COL_HINT : COL_TEXT;
    ry = BAT_HUD_Y + 2 + party_idx * BAT_HUD_ROW_H;
    gfx_rect(BAT_HUD_X, ry - 1, BAT_HUD_W, BAT_HUD_ROW_H - 2, (unsigned char)col);
}

void batscrn_draw_floating_dmg(const BatEnemy *enemies,
                               int target_kind, int slot,
                               int amount, int color) {
    char buf[16];
    int x, y;
    if (target_kind == TARGET_ENEMY) {
        if (!enemies || slot < 0) return;
        x = enemies[slot].x + BAT_ENEMY_W / 2 - 8;
        y = enemies[slot].y - 12;
        if (y < 0) y = 0;
    } else {
        if (slot < 0 || slot >= PARTY_SIZE) return;
        x = 72;
        y = BAT_HUD_Y + slot * BAT_HUD_ROW_H - 2;
        if (y < 0) y = 0;
    }
    if (amount >= 0) {
        sprintf(buf, "%d", amount);
    } else {
        sprintf(buf, "+%d", -amount);
    }
    font_puts(x, y, buf, (unsigned char)color);
}

void batscrn_draw_action_flash(const BatEnemy *enemies,
                               int target_kind, int slot,
                               unsigned long elapsed) {
    int phase;
    if (target_kind == TARGET_ENEMY) {
        if (!enemies || slot < 0 || !enemies[slot].alive) return;
        phase = (int)((elapsed / 60) & 1);
        if (phase == 0) {
            gfx_fill_rect(enemies[slot].x, enemies[slot].y,
                          BAT_ENEMY_W, BAT_ENEMY_H, COL_TEXT);
        }
    } else {
        int ry;
        if (slot < 0 || slot >= PARTY_SIZE) return;
        phase = (int)((elapsed / 60) & 1);
        if (phase == 0) {
            ry = BAT_HUD_Y + 2 + slot * BAT_HUD_ROW_H;
            gfx_fill_rect(BAT_HUD_X + 1, ry - 1,
                          BAT_HUD_W - 2, BAT_HUD_ROW_H - 2, COL_DEFTAG);
        }
    }
}
