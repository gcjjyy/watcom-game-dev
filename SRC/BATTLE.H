#ifndef BATTLE_H_INCLUDED
#define BATTLE_H_INCLUDED

#define BTL_ONGOING 0
#define BTL_VICTORY 1
#define BTL_DEFEAT  2
#define BTL_ESCAPE  3

void battle_init(int group_id);
void battle_update(void);
void battle_draw(void);
int  battle_result(void);

#endif
