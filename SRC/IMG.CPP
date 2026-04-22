#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "GFX.H"
#include "IMG.H"

int img_load_fs(const char *path, Img *out) {
    FILE *fp;
    unsigned char *buf;
    unsigned long got;

    out->w = 0; out->h = 0; out->pix = 0;
    fp = fopen(path, "rb");
    if (!fp) return IMG_ERR;

    buf = (unsigned char *)malloc(64000UL);
    if (!buf) { fclose(fp); return IMG_ERR; }

    got = (unsigned long)fread(buf, 1, 64000UL, fp);
    fclose(fp);
    if (got != 64000UL) { free(buf); return IMG_ERR; }

    out->w = 320;
    out->h = 200;
    out->pix = buf;
    return IMG_OK;
}

int img_load(const char *path, Img *out) {
    FILE *fp;
    unsigned char hdr[4];
    unsigned int w, h;
    unsigned long sz, got;
    unsigned char *buf;

    out->w = 0; out->h = 0; out->pix = 0;
    fp = fopen(path, "rb");
    if (!fp) return IMG_ERR;

    if (fread(hdr, 1, 4, fp) != 4) { fclose(fp); return IMG_ERR; }
    w = (unsigned int)hdr[0] | ((unsigned int)hdr[1] << 8);
    h = (unsigned int)hdr[2] | ((unsigned int)hdr[3] << 8);
    if (w == 0 || h == 0 || w > 1024 || h > 4096) {
        fclose(fp); return IMG_ERR;
    }
    /* CRITICAL: use unsigned long for size — 320*400 = 128000 overflows 16-bit. */
    sz = (unsigned long)w * (unsigned long)h;
    buf = (unsigned char *)malloc(sz);
    if (!buf) { fclose(fp); return IMG_ERR; }

    got = (unsigned long)fread(buf, 1, sz, fp);
    fclose(fp);
    if (got != sz) { free(buf); return IMG_ERR; }

    out->w = (int)w;
    out->h = (int)h;
    out->pix = buf;
    return IMG_OK;
}

void img_free(Img *out) {
    if (out && out->pix) {
        free(out->pix);
        out->pix = 0;
        out->w = 0;
        out->h = 0;
    }
}

void img_blit(const Img *im, int x, int y) {
    if (!im || !im->pix) return;
    /* Use existing gfx_blit which handles a full opaque blit. */
    gfx_blit(im->pix, x, y, im->w, im->h);
}

void img_blit_vscroll(const Img *im, int dx0, int dx1,
                      int dy0, int dy1, int src_y) {
    unsigned char *dst;
    int y, sy, run;

    if (!im || !im->pix) return;
    if (dy0 < 0) dy0 = 0;
    if (dy1 > GFX_H) dy1 = GFX_H;
    if (dx0 < 0) dx0 = 0;
    if (dx1 > GFX_W) dx1 = GFX_W;
    if (dx1 > im->w) dx1 = im->w;
    if (dy1 <= dy0 || dx1 <= dx0) return;

    run = dx1 - dx0;
    sy  = src_y % im->h;
    if (sy < 0) sy += im->h;

    dst = gfx_buffer() + (unsigned long)dy0 * GFX_W + dx0;
    for (y = dy0; y < dy1; y++, dst += GFX_W) {
        memcpy(dst, im->pix + (unsigned long)sy * im->w + dx0, run);
        sy++;
        if (sy >= im->h) sy = 0;
    }
}
