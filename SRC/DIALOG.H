#ifndef DIALOG_H_INCLUDED
#define DIALOG_H_INCLUDED

void dlg_show(const char *text);
int  dlg_update(void);    /* returns 1 when dismissed */
void dlg_draw(void);
int  dlg_active(void);

#endif
