#ifndef MAP_H_INCLUDED
#define MAP_H_INCLUDED

#define MAP_TILE_W  16
#define MAP_TILE_H  16
#define MAP_MAX_TILES 256
#define MAP_VIEW_COLS 20
#define MAP_VIEW_ROWS 11

int  map_load_tiles(const char *filename);
void map_free_tiles(void);
void map_set(const unsigned char *tiles, const unsigned char *pass,
             int w, int h);
void map_draw(int cam_x, int cam_y);
int  map_passable(int tx, int ty);
int  map_tile_at(int tx, int ty);
int  map_width(void);
int  map_height(void);

#endif
