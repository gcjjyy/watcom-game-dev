/* Engine smoke test.
 * Brings up every engine subsystem, clears the framebuffer, waits for ESC,
 * and shuts everything down in the required LIFO order. */

#include "GFX.H"
#include "INPUT.H"
#include "TIMER.H"
#include "SOUND.H"
#include "PALETTE.H"

int main(void)
{
    timer_init();
    input_init();
    snd_init();

    gfx_init();
    gfx_set_palette_all(game_palette);

    while (!input_key(KEY_ESC)) {
        gfx_clear(4);
        gfx_vsync();
        gfx_flip();
    }

    gfx_close();

    snd_close();
    input_close();
    timer_close();

    return 0;
}
