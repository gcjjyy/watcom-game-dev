#ifndef MENU_H_INCLUDED
#define MENU_H_INCLUDED

#define MENU_PENDING  -1
#define MENU_CANCEL    0
#define MENU_MAX_CHARS 6

void menu_init(void);
int  menu_update(void);   /* returns PENDING, CANCEL, or selected count */
void menu_draw(void);
int  menu_get_selected(int *out, int max);

#endif
