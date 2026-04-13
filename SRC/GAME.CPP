#include <stdlib.h>
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
#include "FIELD.H"
#include "BATTLE.H"
#include "PARTY.H"
#include "SFX.H"

#define STATE_TOWN   0
#define STATE_FIELD  1
#define STATE_BATTLE 2

static int state = STATE_TOWN;
static int running = 1;

/* cross-state statics: face dir + field position preservation */
static int _saved_face_dir = 0;
static int _saved_field_x  = -1;
static int _saved_field_y  = -1;

int main(void) {
    int trg, r;

    gfx_init();
    gfx_set_palette_all(game_palette);
    map_load_tiles("TILES.TIL");
    unit_load_sprite(0, "SPR_HERO.SPR");
    unit_load_sprite(1, "SPR_NPC1.SPR");
    unit_load_sprite(2, "SPR_ENM1.SPR");

    /* ISR init order: timer -> input -> sound, then start VGM BEFORE sfx_init.
     * sfx_init issues a DSP reset which on SB16 can silence the OPL2 voices;
     * starting VGM first ensures the VGM ISR is already ticking OPL2 writes
     * when the DSP reset happens, so the FM chip comes back to life quickly. */
    timer_init();
    input_init();
    snd_init();

    if (snd_detect()) {
        if (snd_load("TEST.VGM") == SND_OK) snd_play();
    }

    sfx_init();

    font_init();
    party_init();
    sfx_load(SFX_ATK,  "SFX_ATK.SFX");
    sfx_load(SFX_HIT,  "SFX_HIT.SFX");
    sfx_load(SFX_SEL,  "SFX_SEL.SFX");
    sfx_load(SFX_OK,   "SFX_OK.SFX");
    sfx_load(SFX_CAN,  "SFX_CAN.SFX");
    sfx_load(SFX_WIN,  "SFX_WIN.SFX");
    sfx_load(SFX_LOSE, "SFX_LOSE.SFX");
    sfx_load(SFX_ENC,  "SFX_ENC.SFX");
    srand((unsigned int)timer_ms());

    town_init(0);   /* default: facing south on first boot */
    state = STATE_TOWN;

    while (running) {
        switch (state) {
        case STATE_TOWN:
            trg = town_update();
            town_draw();
            if (trg == TOWN_TRIGGER_FIELD) {
                _saved_face_dir = town_player_face();
                field_init(_saved_face_dir, -1, -1);
                state = STATE_FIELD;
                input_flush();
            }
            if (input_key(KEY_ESC) && !dlg_active()) running = 0;
            break;

        case STATE_FIELD:
            trg = field_update();
            field_draw();
            if (trg == FLD_TRIGGER_TOWN) {
                _saved_face_dir = field_player_face();
                town_init(_saved_face_dir);
                state = STATE_TOWN;
                input_flush();
            } else if (trg == FLD_TRIGGER_BATTLE) {
                _saved_face_dir = field_player_face();
                _saved_field_x  = field_player_x();
                _saved_field_y  = field_player_y();
                battle_init(field_last_group());
                state = STATE_BATTLE;
                input_flush();
            }
            break;

        case STATE_BATTLE:
            battle_update();
            battle_draw();
            r = battle_result();
            if (r == BTL_VICTORY || r == BTL_ESCAPE) {
                field_init(_saved_face_dir, _saved_field_x, _saved_field_y);
                state = STATE_FIELD;
                input_flush();
            } else if (r == BTL_DEFEAT) {
                party_full_heal();
                _saved_face_dir = 0;
                town_init(0);
                state = STATE_TOWN;
                input_flush();
            }
            break;
        }

        gfx_vsync();
        gfx_flip();
    }

    /* shutdown LIFO */
    font_close();
    sfx_close();
    snd_close();
    input_close();
    timer_close();
    unit_free_sprites();
    map_free_tiles();
    gfx_close();
    return 0;
}
