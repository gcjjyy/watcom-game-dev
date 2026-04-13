#ifndef TOWN_H_INCLUDED
#define TOWN_H_INCLUDED

#define TOWN_TRIGGER_NONE   0
#define TOWN_TRIGGER_FIELD  1

void town_init(int spawn_face_dir);
int  town_update(void);
void town_draw(void);
int  town_player_face(void);

#endif
