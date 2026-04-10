#ifndef TOWN_H_INCLUDED
#define TOWN_H_INCLUDED

#define TOWN_TRIGGER_NONE   0
#define TOWN_TRIGGER_BATTLE 1

void town_init(void);
int  town_update(void);   /* returns trigger type */
void town_draw(void);

#endif
