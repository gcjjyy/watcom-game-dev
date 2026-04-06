#ifndef FONT_H_INCLUDED
#define FONT_H_INCLUDED

#define FONT_ENG_W    8
#define FONT_ENG_H   16
#define FONT_HAN_W   16
#define FONT_HAN_H   16

void font_init(void);
void font_close(void);

void font_puts(int x, int y, const char *str, unsigned char color);
int  font_text_width(const char *str);

#endif
