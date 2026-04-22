#ifndef IMG_H_INCLUDED
#define IMG_H_INCLUDED

#define IMG_OK   0
#define IMG_ERR -1

typedef struct {
    int w;
    int h;
    unsigned char *pix;  /* heap-allocated raw indexed bytes */
} Img;

/* Full-screen 320x200 raw indexed file (no header, exactly 64000 bytes). */
int  img_load_fs(const char *path, Img *out);

/* Variable-size raw indexed: 4-byte header { uint16 w; uint16 h; } + pixels. */
int  img_load(const char *path, Img *out);

void img_free(Img *out);

/* Blit full image at (x,y), opaque, clipped to screen. */
void img_blit(const Img *im, int x, int y);

/* Vertical-scroll blit: copies rows from src starting at src_y (wrapping
 * vertically through im->h), writing into the back-buffer rows [dy0..dy1)
 * clipped horizontally to columns [dx0..dx1). Used by the gameplay scene
 * for the scrolling background strip. */
void img_blit_vscroll(const Img *im, int dx0, int dx1,
                      int dy0, int dy1, int src_y);

#endif
