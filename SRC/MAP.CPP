#include <stdio.h>
#include <string.h>
#include "MAP.H"
#include "GFX.H"

/* tile pixel data: up to 256 tiles of 16x16 = 65536 bytes */
static unsigned char _tile_data[MAP_MAX_TILES * MAP_TILE_W * MAP_TILE_H];
static int _tile_w = 0;
static int _tile_h = 0;
static int _num_tiles = 0;

/* current map pointers (owned externally, not freed here) */
static const unsigned char *_map_tiles = NULL;
static const unsigned char *_map_pass  = NULL;
static int _map_w = 0;
static int _map_h = 0;

/* ---- tile loading ---- */

int map_load_tiles(const char *filename) {
    FILE *f;
    unsigned long hdr[3];
    unsigned long data_size;

    f = fopen(filename, "rb");
    if (!f) return -1;

    fread(hdr, 4, 3, f);
    _tile_w = (int)hdr[0];
    _tile_h = (int)hdr[1];
    _num_tiles = (int)hdr[2];

    if (_num_tiles > MAP_MAX_TILES) _num_tiles = MAP_MAX_TILES;

    data_size = (unsigned long)_num_tiles * _tile_w * _tile_h;
    if (data_size > sizeof(_tile_data)) {
        fclose(f);
        return -1;
    }

    fread(_tile_data, 1, data_size, f);
    fclose(f);
    return 0;
}

void map_free_tiles(void) {
    _num_tiles = 0;
    _tile_w = 0;
    _tile_h = 0;
}

/* ---- map data ---- */

void map_set(const unsigned char *tiles, const unsigned char *pass,
             int w, int h) {
    _map_tiles = tiles;
    _map_pass  = pass;
    _map_w = w;
    _map_h = h;
}

int map_width(void)  { return _map_w; }
int map_height(void) { return _map_h; }

/* ---- queries ---- */

int map_passable(int tx, int ty) {
    if (tx < 0 || ty < 0 || tx >= _map_w || ty >= _map_h) return 0;
    if (!_map_pass) return 1;
    return _map_pass[ty * _map_w + tx] ? 1 : 0;
}

int map_tile_at(int tx, int ty) {
    if (tx < 0 || ty < 0 || tx >= _map_w || ty >= _map_h) return 0;
    if (!_map_tiles) return 0;
    return _map_tiles[ty * _map_w + tx];
}

/* ---- rendering ---- */

void map_draw(int cam_x, int cam_y) {
    int col, row;
    int tx, ty;
    int px, py;
    int tile_idx;
    int tile_bytes;
    const unsigned char *src;

    tile_bytes = _tile_w * _tile_h;

    for (row = 0; row < MAP_VIEW_ROWS; row++) {
        ty = cam_y + row;
        if (ty < 0 || ty >= _map_h) continue;
        py = row * MAP_TILE_H;

        for (col = 0; col < MAP_VIEW_COLS; col++) {
            tx = cam_x + col;
            if (tx < 0 || tx >= _map_w) continue;
            px = col * MAP_TILE_W;

            tile_idx = _map_tiles[ty * _map_w + tx];
            if (tile_idx < 0 || tile_idx >= _num_tiles) continue;

            src = &_tile_data[tile_idx * tile_bytes];
            gfx_blit(src, px, py, _tile_w, _tile_h);
        }
    }
}
