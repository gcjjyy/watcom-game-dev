#ifndef SPRITE_H_INCLUDED
#define SPRITE_H_INCLUDED

#define SPR_OK   0
#define SPR_ERR -1

typedef struct {
    int w;
    int h;
    int num_frames;
    unsigned char **frames;
    void *_data;
} Sprite;

int  spr_load(const char *filename, Sprite *spr);
void spr_free(Sprite *spr);

#endif
