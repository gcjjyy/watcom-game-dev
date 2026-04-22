#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SPRITE.H"

int spr_load(const char *filename, Sprite *spr) {
    FILE *f;
    unsigned long hdr[3];
    unsigned long *sizes;
    unsigned long total_code, alloc_size;
    unsigned char *block, *code_ptr;
    int i;

    f = fopen(filename, "rb");
    if (!f) return SPR_ERR;

    fread(hdr, 4, 3, f);
    spr->w = (int)hdr[0];
    spr->h = (int)hdr[1];
    spr->num_frames = (int)hdr[2];

    sizes = (unsigned long *)malloc(spr->num_frames * sizeof(unsigned long));
    if (!sizes) {
        fclose(f);
        return SPR_ERR;
    }
    fread(sizes, 4, spr->num_frames, f);

    total_code = 0;
    for (i = 0; i < spr->num_frames; i++) total_code += sizes[i];

    alloc_size = spr->num_frames * sizeof(unsigned char *) + total_code;
    block = (unsigned char *)malloc(alloc_size);
    if (!block) {
        free(sizes);
        fclose(f);
        return SPR_ERR;
    }

    spr->frames = (unsigned char **)block;
    code_ptr = block + spr->num_frames * sizeof(unsigned char *);

    for (i = 0; i < spr->num_frames; i++) {
        spr->frames[i] = code_ptr;
        fread(code_ptr, 1, sizes[i], f);
        code_ptr += sizes[i];
    }

    spr->_data = block;
    free(sizes);
    fclose(f);
    return SPR_OK;
}

void spr_free(Sprite *spr) {
    if (spr->_data) free(spr->_data);
    spr->w = 0;
    spr->h = 0;
    spr->num_frames = 0;
    spr->frames = NULL;
    spr->_data = NULL;
}

/* Walk one compiled-sprite frame's bytes, overwriting pixel-data bytes
 * with `color`. Returns the number of bytes consumed up to and
 * including the terminating RET (0xC3). The compiled sprite emits
 * exactly three instruction shapes followed by a RET:
 *   C7 87 [off32] [pix4]     10 bytes (4-pixel run)
 *   66 C7 87 [off32] [pix2]   9 bytes (2-pixel run)
 *   C6 87 [off32] [pix1]      7 bytes (1-pixel)
 *   C3                        1 byte  (end)
 */
static unsigned long tint_one_frame(unsigned char *code, unsigned char color) {
    unsigned long p = 0;
    for (;;) {
        unsigned char b = code[p];
        if (b == 0xC7) {
            /* 4-pixel run: data bytes at p+6..p+9 */
            code[p+6] = color;
            code[p+7] = color;
            code[p+8] = color;
            code[p+9] = color;
            p += 10;
        } else if (b == 0x66) {
            /* 2-pixel run: data bytes at p+7..p+8 */
            code[p+7] = color;
            code[p+8] = color;
            p += 9;
        } else if (b == 0xC6) {
            /* 1-pixel: data byte at p+6 */
            code[p+6] = color;
            p += 7;
        } else if (b == 0xC3) {
            /* RET — frame end */
            p += 1;
            break;
        } else {
            /* Unknown byte — bail to avoid running off. */
            break;
        }
    }
    return p;
}

int spr_dup_tinted(const Sprite *src, unsigned char color, Sprite *dst) {
    unsigned long *sizes;
    unsigned long total_code, alloc_size;
    unsigned char *block, *code_ptr;
    int i;

    if (!src || !dst) return SPR_ERR;
    dst->w = 0; dst->h = 0; dst->num_frames = 0; dst->frames = NULL; dst->_data = NULL;
    if (src->num_frames <= 0 || !src->frames) return SPR_ERR;

    /* Measure each source frame by parsing until RET. */
    sizes = (unsigned long *)malloc((unsigned long)src->num_frames * sizeof(unsigned long));
    if (!sizes) return SPR_ERR;

    total_code = 0;
    for (i = 0; i < src->num_frames; i++) {
        /* We parse a copy scratch to measure without modifying the source. */
        unsigned char *f = src->frames[i];
        unsigned long p = 0;
        for (;;) {
            unsigned char b = f[p];
            if      (b == 0xC7) p += 10;
            else if (b == 0x66) p += 9;
            else if (b == 0xC6) p += 7;
            else if (b == 0xC3) { p += 1; break; }
            else                { break; }
        }
        sizes[i]    = p;
        total_code += p;
    }

    alloc_size = (unsigned long)src->num_frames * sizeof(unsigned char *) + total_code;
    block = (unsigned char *)malloc(alloc_size);
    if (!block) { free(sizes); return SPR_ERR; }

    dst->w          = src->w;
    dst->h          = src->h;
    dst->num_frames = src->num_frames;
    dst->frames     = (unsigned char **)block;
    dst->_data      = block;

    code_ptr = block + (unsigned long)src->num_frames * sizeof(unsigned char *);
    for (i = 0; i < src->num_frames; i++) {
        dst->frames[i] = code_ptr;
        memcpy(code_ptr, src->frames[i], sizes[i]);
        tint_one_frame(code_ptr, color);
        code_ptr += sizes[i];
    }

    free(sizes);
    return SPR_OK;
}
