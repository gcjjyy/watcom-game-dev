#include <stdio.h>
#include <string.h>
#include "GFX.H"
#include "SCRNCAP.H"

extern unsigned char game_palette[768];

/* 8-bit indexed BMP writer (320x200). Layout:
 *   14-byte file header
 *   40-byte BITMAPINFOHEADER
 *   256 * 4 BGRA palette (1024 bytes)
 *   320 * 200 pixel data stored bottom-up (rows are 4-byte aligned,
 *   320 already is). Total = 14 + 40 + 1024 + 64000 = 65078 bytes. */

static void put_u16(unsigned char *b, unsigned short v) {
    b[0] = (unsigned char)(v & 0xFF);
    b[1] = (unsigned char)((v >> 8) & 0xFF);
}

static void put_u32(unsigned char *b, unsigned long v) {
    b[0] = (unsigned char)(v & 0xFF);
    b[1] = (unsigned char)((v >> 8)  & 0xFF);
    b[2] = (unsigned char)((v >> 16) & 0xFF);
    b[3] = (unsigned char)((v >> 24) & 0xFF);
}

int scrncap_save_bmp(const char *path) {
    FILE *fp;
    unsigned char hdr[54];
    unsigned char pal[1024];
    unsigned char *buf;
    int i;
    int y;

    fp = fopen(path, "wb");
    if (!fp) return -1;

    /* File header */
    memset(hdr, 0, sizeof(hdr));
    hdr[0] = 'B'; hdr[1] = 'M';
    put_u32(&hdr[2],  65078UL);    /* file size */
    put_u32(&hdr[10], 14UL + 40UL + 1024UL);  /* pixel data offset */
    /* Info header */
    put_u32(&hdr[14], 40UL);       /* info header size */
    put_u32(&hdr[18], 320UL);      /* width */
    put_u32(&hdr[22], 200UL);      /* height (positive = bottom-up) */
    put_u16(&hdr[26], 1);          /* planes */
    put_u16(&hdr[28], 8);          /* bits per pixel */
    put_u32(&hdr[30], 0UL);        /* compression = none */
    put_u32(&hdr[34], 64000UL);    /* image size */
    put_u32(&hdr[38], 2835UL);     /* x px/m */
    put_u32(&hdr[42], 2835UL);     /* y px/m */
    put_u32(&hdr[46], 256UL);      /* colors used */
    put_u32(&hdr[50], 0UL);        /* important colors */
    if (fwrite(hdr, 1, 54, fp) != 54) { fclose(fp); return -1; }

    /* Palette: convert 6-bit RGB from game_palette to 8-bit BGRA.
     * 6-bit->8-bit via (v << 2) | (v >> 4). */
    for (i = 0; i < 256; i++) {
        unsigned char r = game_palette[i * 3 + 0];
        unsigned char g = game_palette[i * 3 + 1];
        unsigned char b = game_palette[i * 3 + 2];
        r = (unsigned char)((r << 2) | (r >> 4));
        g = (unsigned char)((g << 2) | (g >> 4));
        b = (unsigned char)((b << 2) | (b >> 4));
        pal[i * 4 + 0] = b;
        pal[i * 4 + 1] = g;
        pal[i * 4 + 2] = r;
        pal[i * 4 + 3] = 0;
    }
    if (fwrite(pal, 1, 1024, fp) != 1024) { fclose(fp); return -1; }

    /* Pixel rows — BMP is bottom-up. */
    buf = gfx_buffer();
    for (y = 199; y >= 0; y--) {
        if (fwrite(buf + (unsigned long)y * 320UL, 1, 320, fp) != 320) {
            fclose(fp);
            return -1;
        }
    }

    fclose(fp);
    return 0;
}
