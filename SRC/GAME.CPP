#include "GFX.H"
#include "INPUT.H"
#include "PALETTE.H"
#include "SPR_TEST.H"

int main() {
    int x = 152, y = 92;
    int dx = 2, dy = 1;

    gfx_init();
    input_init();
    gfx_set_palette_all(game_palette);

    while (!input_key(KEY_ESC)) {
        gfx_clear(0);
        gfx_draw_csprite(spr_diamond_code, x, y);
        gfx_vsync();
        gfx_flip();

        x += dx;
        y += dy;
        if (x <= 0 || x + SPR_DIAMOND_W >= GFX_W) dx = -dx;
        if (y <= 0 || y + SPR_DIAMOND_H >= GFX_H) dy = -dy;
    }

    input_close();
    gfx_close();
    return 0;
}
