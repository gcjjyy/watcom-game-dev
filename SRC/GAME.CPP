#include <string.h>
#include "GFX.H"
#include "INPUT.H"
#include "TIMER.H"
#include "SOUND.H"
#include "FONT.H"
#include "PALETTE.H"
#include "MAP.H"
#include "UNIT.H"
#include "DIALOG.H"
#include "TOWN.H"
#include "MENU.H"
#include "BATTLE.H"

/* game states */
#define STATE_TOWN    0
#define STATE_PARTY   1
#define STATE_BATTLE  2

static int state = STATE_TOWN;
static int running = 1;

int main(void) {
    int trigger;
    int result;
    int sel_count;
    int selected[6];

    /* ---- init (non-ISR resources first) ---- */
    gfx_init();
    gfx_set_palette_all(game_palette);
    map_load_tiles("TILES.TIL");
    unit_load_sprite(0, "SPR_HERO.SPR");
    unit_load_sprite(1, "SPR_NPC1.SPR");
    unit_load_sprite(2, "SPR_ENM1.SPR");

    /* ISR init order: timer -> input -> sound (CRITICAL) */
    timer_init();
    input_init();
    snd_init();

    if (snd_detect()) {
        if (snd_load("TEST.VGM") == SND_OK) {
            snd_play();
        }
    }

    font_init();

    /* init town */
    town_init();
    state = STATE_TOWN;
    running = 1;

    /* ---- main loop ---- */
    while (running) {
        switch (state) {
        case STATE_TOWN:
            trigger = town_update();
            town_draw();
            if (trigger == TOWN_TRIGGER_BATTLE) {
                state = STATE_PARTY;
                menu_init();
                input_flush();
            }
            /* ESC quits from town (only when not in dialogue) */
            if (input_key(KEY_ESC) && !dlg_active()) {
                running = 0;
            }
            break;

        case STATE_PARTY:
            result = menu_update();
            menu_draw();
            if (result == MENU_CANCEL) {
                state = STATE_TOWN;
                town_init();
                input_flush();
            } else if (result > 0) {
                sel_count = menu_get_selected(selected, 6);
                state = STATE_BATTLE;
                battle_init(selected, sel_count);
                input_flush();
            }
            break;

        case STATE_BATTLE:
            battle_update();
            battle_draw();
            result = battle_result();
            if (result != BTL_ONGOING) {
                state = STATE_TOWN;
                town_init();
                input_flush();
            }
            break;
        }

        gfx_vsync();
        gfx_flip();
    }

    /* ---- shutdown (LIFO for ISRs) ---- */
    font_close();
    snd_close();
    input_close();
    timer_close();
    unit_free_sprites();
    map_free_tiles();
    gfx_close();

    return 0;
}
