#ifndef GFX_H_INCLUDED
#define GFX_H_INCLUDED

#define GFX_W    320
#define GFX_H    200
#define GFX_SIZE 64000
#define GFX_VGA  0xA0000

void gfx_init(void);
void gfx_close(void);
void gfx_flip(void);
void gfx_vsync(void);
void gfx_clear(unsigned char color);

void gfx_pixel(int x, int y, unsigned char color);
void gfx_hline(int x1, int x2, int y, unsigned char color);
void gfx_vline(int x, int y1, int y2, unsigned char color);
void gfx_rect(int x, int y, int w, int h, unsigned char color);
void gfx_fill_rect(int x, int y, int w, int h, unsigned char color);

void gfx_blit(const unsigned char *sprite, int x, int y, int w, int h);
void gfx_blit_trans(const unsigned char *sprite, int x, int y,
                    int w, int h, unsigned char trans_color);

void gfx_set_palette(unsigned char index,
                     unsigned char r, unsigned char g, unsigned char b);
void gfx_set_palette_all(const unsigned char *pal);

/* compiled sprites: x must be >= 0, y must be >= 0,
   and sprite must fit entirely on screen (no clipping) */
void gfx_draw_csprite(const unsigned char *code, int x, int y);

unsigned char *gfx_buffer(void);

#endif
