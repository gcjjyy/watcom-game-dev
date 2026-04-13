#ifndef BATSCRN_H_INCLUDED
#define BATSCRN_H_INCLUDED

#include "PARTY.H"

/* Coordinate / color constants */
#define BAT_BG_COLOR        2
#define BAT_ENEMY_AREA_X    0
#define BAT_ENEMY_AREA_Y    8
#define BAT_ENEMY_AREA_W    320
#define BAT_ENEMY_AREA_H    72
#define BAT_ENEMY_W         32
#define BAT_ENEMY_H         32

#define BAT_LOG_X           0
#define BAT_LOG_Y           84
#define BAT_LOG_W           320
#define BAT_LOG_H           32
#define BAT_LOG_LINE_H      12

#define BAT_HUD_X           0
#define BAT_HUD_Y           120
#define BAT_HUD_W           200
#define BAT_HUD_H           80
#define BAT_HUD_ROW_H       18

#define BAT_MENU_X          208
#define BAT_MENU_Y          124
#define BAT_MENU_W          100
#define BAT_MENU_H          70
#define BAT_MENU_ITEM_H     12
#define BAT_MENU_TEXT_X     216
#define BAT_MENU_TEXT_Y     130

/* Submenu kinds for the merged draw_submenu helper */
#define SUB_KIND_MAGIC      0
#define SUB_KIND_ITEM       1

/* Floating dmg target kind */
#define TARGET_ENEMY        0
#define TARGET_PLAYER       1

/* Shared battle-enemy runtime struct used by BATTLE.CPP and BATSCRN.CPP.
 * Kept here so BATSCRN helpers can access enemy position/HP for rendering. */
typedef struct BatEnemy {
    int def_id;        /* index into enemy_defs[] */
    int hp, mp;        /* runtime mutable */
    int alive;
    int defending;
    int x, y;          /* draw position in pixels */
} BatEnemy;

/* 10 helper prototypes — HUD helper handles [D] inline (no standalone defend-tags). */
void batscrn_clear_bg(void);
void batscrn_draw_enemies(const BatEnemy *enemies, int count);
void batscrn_draw_party_hud(int active_idx);
void batscrn_draw_message_log(const char *line1, const char *line2);
void batscrn_draw_command_menu(int cursor);
void batscrn_draw_submenu(int kind, int cursor, int char_idx,
                          const char **labels, const int *values, int count);
void batscrn_draw_target_enemy_cursor(const BatEnemy *enemies,
                                      int enemy_slot, unsigned long blink_time);
void batscrn_draw_target_ally_cursor(int party_idx, unsigned long blink_time);
void batscrn_draw_floating_dmg(const BatEnemy *enemies,
                               int target_kind, int slot,
                               int amount, int color);
void batscrn_draw_action_flash(const BatEnemy *enemies,
                               int target_kind, int slot,
                               unsigned long elapsed);

#endif
