#include <string.h>
#include <conio.h>
#include "GFX.H"

static unsigned char _buf[GFX_SIZE];

/* ---- assembly helpers via #pragma aux ---- */

/* rep movsd: copy dwords */
void _mcpy32(void *dst, const void *src, unsigned long cnt);
#pragma aux _mcpy32 = \
    "rep movsd" \
    parm [edi] [esi] [ecx] \
    modify [edi esi ecx];

/* rep stosd: fill dwords */
void _mset32(void *dst, unsigned long val, unsigned long cnt);
#pragma aux _mset32 = \
    "rep stosd" \
    parm [edi] [eax] [ecx] \
    modify [edi ecx];

/* rep outsb: bulk port output */
void _outsb(unsigned short port, const void *data, unsigned long cnt);
#pragma aux _outsb = \
    "rep outsb" \
    parm [edx] [esi] [ecx] \
    modify [esi ecx];

/* BIOS set video mode */
void _setmode(int mode);
#pragma aux _setmode = \
    "int 0x10" \
    parm [eax];

/* call compiled sprite code: EDI=dst, ESI=code ptr */
void _call_code(void *dst, const void *code);
#pragma aux _call_code = \
    "call esi" \
    parm [edi] [esi];

/* ---- public API ---- */

void gfx_init(void) {
    _setmode(0x13);
    memset(_buf, 0, GFX_SIZE);
}

void gfx_close(void) {
    _setmode(0x03);
}

void gfx_flip(void) {
    _mcpy32((void *)GFX_VGA, _buf, GFX_SIZE / 4);
}

void gfx_vsync(void) {
    while (inp(0x3DA) & 8);
    while (!(inp(0x3DA) & 8));
}

void gfx_draw_csprite(const unsigned char *code, int x, int y) {
    _call_code(_buf + y * GFX_W + x, code);
}

void gfx_clear(unsigned char color) {
    unsigned long fill = color;
    fill |= (fill << 8);
    fill |= (fill << 16);
    _mset32(_buf, fill, GFX_SIZE / 4);
}

unsigned char *gfx_buffer(void) {
    return _buf;
}

/* ---- drawing ---- */

void gfx_pixel(int x, int y, unsigned char color) {
    if ((unsigned)x < GFX_W && (unsigned)y < GFX_H)
        _buf[y * GFX_W + x] = color;
}

void gfx_hline(int x1, int x2, int y, unsigned char color) {
    int tmp;
    if ((unsigned)y >= GFX_H) return;
    if (x1 > x2) { tmp = x1; x1 = x2; x2 = tmp; }
    if (x1 < 0) x1 = 0;
    if (x2 >= GFX_W) x2 = GFX_W - 1;
    if (x1 > x2) return;
    memset(&_buf[y * GFX_W + x1], color, x2 - x1 + 1);
}

void gfx_vline(int x, int y1, int y2, unsigned char color) {
    int tmp;
    unsigned char *p;
    if ((unsigned)x >= GFX_W) return;
    if (y1 > y2) { tmp = y1; y1 = y2; y2 = tmp; }
    if (y1 < 0) y1 = 0;
    if (y2 >= GFX_H) y2 = GFX_H - 1;
    p = &_buf[y1 * GFX_W + x];
    while (y1 <= y2) {
        *p = color;
        p += GFX_W;
        y1++;
    }
}

void gfx_rect(int x, int y, int w, int h, unsigned char color) {
    gfx_hline(x, x + w - 1, y, color);
    gfx_hline(x, x + w - 1, y + h - 1, color);
    gfx_vline(x, y, y + h - 1, color);
    gfx_vline(x + w - 1, y, y + h - 1, color);
}

void gfx_fill_rect(int x, int y, int w, int h, unsigned char color) {
    int i;
    for (i = 0; i < h; i++)
        gfx_hline(x, x + w - 1, y + i, color);
}

/* ---- sprites ---- */

void gfx_blit(const unsigned char *sprite, int x, int y, int w, int h) {
    int sx = 0, sy = 0, cw = w, ch = h;
    unsigned char *dst;
    const unsigned char *src;

    if (x < 0) { sx = -x; cw += x; x = 0; }
    if (y < 0) { sy = -y; ch += y; y = 0; }
    if (x + cw > GFX_W) cw = GFX_W - x;
    if (y + ch > GFX_H) ch = GFX_H - y;
    if (cw <= 0 || ch <= 0) return;

    dst = &_buf[y * GFX_W + x];
    src = &sprite[sy * w + sx];
    while (ch--) {
        memcpy(dst, src, cw);
        dst += GFX_W;
        src += w;
    }
}

void gfx_blit_trans(const unsigned char *sprite, int x, int y,
                    int w, int h, unsigned char trans_color) {
    int sx = 0, sy = 0, cw = w, ch = h, i;
    unsigned char *dst;
    const unsigned char *src;

    if (x < 0) { sx = -x; cw += x; x = 0; }
    if (y < 0) { sy = -y; ch += y; y = 0; }
    if (x + cw > GFX_W) cw = GFX_W - x;
    if (y + ch > GFX_H) ch = GFX_H - y;
    if (cw <= 0 || ch <= 0) return;

    dst = &_buf[y * GFX_W + x];
    src = &sprite[sy * w + sx];
    while (ch--) {
        for (i = 0; i < cw; i++) {
            if (src[i] != trans_color) dst[i] = src[i];
        }
        dst += GFX_W;
        src += w;
    }
}

/* ---- palette ---- */

void gfx_set_palette(unsigned char index,
                     unsigned char r, unsigned char g, unsigned char b) {
    outp(0x3C8, index);
    outp(0x3C9, r);
    outp(0x3C9, g);
    outp(0x3C9, b);
}

void gfx_set_palette_all(const unsigned char *pal) {
    outp(0x3C8, 0);
    _outsb(0x3C9, pal, 768);
}
