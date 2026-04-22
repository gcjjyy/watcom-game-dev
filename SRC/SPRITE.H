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

/* Build a duplicate of `src` with every opaque pixel overwritten by
 * the given palette index. Used for the white hit-flash silhouette so
 * the non-transparent shape of the sprite is preserved (unlike a plain
 * bounding-box fill). Caller must spr_free(dst) when done. Returns
 * SPR_OK or SPR_ERR. */
int  spr_dup_tinted(const Sprite *src, unsigned char color, Sprite *dst);

#endif
