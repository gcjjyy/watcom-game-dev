#ifndef BATTLE_H_INCLUDED
#define BATTLE_H_INCLUDED

#define BTL_ONGOING 0
#define BTL_VICTORY 1
#define BTL_DEFEAT  2

void battle_init(const int *selected, int sel_count);
void battle_update(void);
void battle_draw(void);
int  battle_result(void);

#endif
