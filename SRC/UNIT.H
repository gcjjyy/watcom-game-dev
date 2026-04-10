#ifndef UNIT_H_INCLUDED
#define UNIT_H_INCLUDED

#include "SPRITE.H"

#define MAX_UNITS   16
#define MAX_SPR      4
#define TEAM_PLAYER  0
#define TEAM_ENEMY   1

typedef struct {
    char name[9];
    int hp, max_hp, atk, mov;
    int tile_x, tile_y;
    int team, alive, done;
    int spr_idx;
    int spr_frame;
} Unit;

void  unit_init_pool(void);
Unit *unit_spawn(int team, const char *name, int hp, int atk, int mov,
                 int tx, int ty, int spr_idx);
void  unit_draw(Unit *u, int cam_x, int cam_y);
void  unit_draw_all(int cam_x, int cam_y);
Unit *unit_at(int tx, int ty);
Unit *unit_get(int index);
int   unit_alive_count(int team);
void  unit_reset_done(int team);
void  unit_load_sprite(int idx, const char *filename);
void  unit_free_sprites(void);

#endif
