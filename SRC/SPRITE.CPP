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
