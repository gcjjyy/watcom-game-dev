#ifndef FIELD_H_INCLUDED
#define FIELD_H_INCLUDED

#define FLD_TRIGGER_NONE    0
#define FLD_TRIGGER_TOWN    1
#define FLD_TRIGGER_BATTLE  2

void field_init(int spawn_face_dir, int spawn_x, int spawn_y);
int  field_update(void);
void field_draw(void);
int  field_last_group(void);
int  field_player_x(void);
int  field_player_y(void);
int  field_player_face(void);

#endif
