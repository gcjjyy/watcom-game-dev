#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FONT.H"
#include "GFX.H"

/* ---- compiled font binary data ---- */

static unsigned char *_bin_data = NULL;
static unsigned long  _bin_size = 0;
static int            _num_glyphs = 0;
static unsigned char *_bin_index = NULL;
static unsigned char *_bin_code  = NULL;

/* ---- raw font data for fallback ---- */

static unsigned char _eng_font[4096];
static unsigned char _han_font[11520];

/* ---- call compiled glyph: EDI=dst, ESI=code, EAX=color4 ---- */

void _draw_cfont(void *dst, const void *code, unsigned long color4);
#pragma aux _draw_cfont = \
    "call esi" \
    parm [edi] [esi] [eax] \
    modify [edi esi eax];

/* ---- file helpers ---- */

static int _load_file(const char *name, void *buf, unsigned long size) {
    FILE *f = fopen(name, "rb");
    if (!f) return -1;
    fread(buf, 1, size, f);
    fclose(f);
    return 0;
}

static unsigned char *_load_alloc(const char *name, unsigned long *out_size) {
    FILE *f;
    long sz;
    unsigned char *buf;
    f = fopen(name, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    buf = (unsigned char *)malloc(sz);
    if (!buf) { fclose(f); return NULL; }
    fread(buf, 1, sz, f);
    fclose(f);
    *out_size = (unsigned long)sz;
    return buf;
}

/* ---- init / close ---- */

void font_init(void) {
    unsigned long magic;

    _bin_data = _load_alloc("FONT.BIN", &_bin_size);
    if (_bin_data && _bin_size >= 8) {
        memcpy(&magic, _bin_data, 4);
        if (magic == 0x544E4643UL) {  /* "CFNT" */
            _num_glyphs = _bin_data[4] | (_bin_data[5] << 8);
            _bin_index = _bin_data + 8;
            _bin_code  = _bin_index + _num_glyphs * 10;
        }
    }

    _load_file("ENG.FNT", _eng_font, 4096);
    _load_file("HAN.FNT", _han_font, 11520);
}

void font_close(void) {
    if (_bin_data) { free(_bin_data); _bin_data = NULL; }
    _num_glyphs = 0;
}

/* ---- binary search ---- */

static unsigned char *_find_glyph(unsigned long cp) {
    int lo = 0, hi = _num_glyphs - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        unsigned char *entry = _bin_index + mid * 10;
        unsigned long ecp;
        memcpy(&ecp, entry, 4);
        if (ecp == cp) {
            unsigned long off;
            memcpy(&off, entry + 4, 4);
            return _bin_code + off;
        }
        if (ecp < cp) lo = mid + 1;
        else hi = mid - 1;
    }
    return NULL;
}

/* ---- UTF-8 decoder ---- */

static unsigned long _utf8_decode(const char **pp) {
    unsigned char c = (unsigned char)*(*pp)++;
    unsigned long cp;
    if (c < 0x80) return c;
    if ((c & 0xE0) == 0xC0) {
        cp = (unsigned long)(c & 0x1F) << 6;
        cp |= (unsigned long)(*(*pp)++ & 0x3F);
        return cp;
    }
    if ((c & 0xF0) == 0xE0) {
        cp = (unsigned long)(c & 0x0F) << 12;
        cp |= (unsigned long)(*(*pp)++ & 0x3F) << 6;
        cp |= (unsigned long)(*(*pp)++ & 0x3F);
        return cp;
    }
    if ((c & 0xF8) == 0xF0) (*pp) += 3;
    return '?';
}

/* ---- fallback renderer ---- */

static const unsigned char _cho_bul[2][21] = {
    { 0,0,0,0,0,0,0,0, 1,3,3,3,1, 2,4,4,4,2, 1,3,0 },
    { 5,5,5,5,5,5,5,5, 6,7,7,7,6, 6,7,7,7,6, 6,7,5 }
};
static const unsigned char _jong_bul[21] = {
    0,2,0,2,1,2,1,2, 3,0,2,1,3, 3,1,2,1,3, 3,1,1
};

static void _fb_eng(int x, int y, int ch, unsigned char color) {
    unsigned char *buf = gfx_buffer();
    int row, bit;
    unsigned char byte;
    for (row = 0; row < 16; row++) {
        byte = _eng_font[ch * 16 + row];
        if (!byte) continue;
        for (bit = 0; bit < 8; bit++) {
            if (byte & (0x80 >> bit))
                buf[(y + row) * GFX_W + x + bit] = color;
        }
    }
}

static void _fb_han(int x, int y, unsigned long cp, unsigned char color) {
    unsigned char *buf = gfx_buffer();
    unsigned long idx;
    int cho, jung, jong, has_jong;
    int cb, jb, kb, ci, ji, ki;
    int row, bit;
    unsigned char hi, lo;

    idx = cp - 0xAC00UL;
    cho  = (int)(idx / 588);
    jung = (int)((idx % 588) / 28);
    jong = (int)(idx % 28);
    has_jong = (jong > 0) ? 1 : 0;

    cb = _cho_bul[has_jong][jung];
    jb = ((cho == 0 || cho == 16) ? 0 : 1) + (has_jong ? 2 : 0);
    kb = has_jong ? _jong_bul[jung] : 0;

    ci = cb * 20 + (cho + 1);
    ji = 160 + jb * 22 + (jung + 1);
    ki = 248 + kb * 28 + jong;

    for (row = 0; row < 16; row++) {
        hi = _han_font[ci*32 + row*2]
           | _han_font[ji*32 + row*2]
           | _han_font[ki*32 + row*2];
        lo = _han_font[ci*32 + row*2 + 1]
           | _han_font[ji*32 + row*2 + 1]
           | _han_font[ki*32 + row*2 + 1];
        if (!hi && !lo) continue;
        for (bit = 0; bit < 8; bit++) {
            if (hi & (0x80 >> bit))
                buf[(y + row) * GFX_W + x + bit] = color;
            if (lo & (0x80 >> bit))
                buf[(y + row) * GFX_W + x + 8 + bit] = color;
        }
    }
}

/* ---- public API ---- */

void font_puts(int x, int y, const char *str, unsigned char color) {
    unsigned long cp, color4;
    unsigned char *code;

    color4 = (unsigned long)color;
    color4 |= color4 << 8;
    color4 |= color4 << 16;

    while (*str) {
        cp = _utf8_decode(&str);
        if (cp == 0) break;

        code = _find_glyph(cp);
        if (code) {
            _draw_cfont(gfx_buffer() + y * GFX_W + x, code, color4);
        } else if (cp >= 0xAC00UL && cp <= 0xD7A3UL) {
            _fb_han(x, y, cp, color);
        } else if (cp < 256) {
            _fb_eng(x, y, (int)cp, color);
        }

        if (cp >= 0xAC00UL && cp <= 0xD7A3UL)
            x += FONT_HAN_W;
        else
            x += FONT_ENG_W;
    }
}

int font_text_width(const char *str) {
    int w = 0;
    unsigned long cp;
    while (*str) {
        cp = _utf8_decode(&str);
        if (cp == 0) break;
        w += (cp >= 0xAC00UL && cp <= 0xD7A3UL) ? FONT_HAN_W : FONT_ENG_W;
    }
    return w;
}
