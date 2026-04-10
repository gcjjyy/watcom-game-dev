#include <stdio.h>
#include <string.h>
#include "MENU.H"
#include "GFX.H"
#include "FONT.H"
#include "INPUT.H"

#define NUM_CHARS 4

typedef struct {
    char name[9];
    int hp, atk, mov;
    int selected;
} MenuEntry;

static MenuEntry _chars[NUM_CHARS] = {
    { "Hero",    50, 10, 4, 0 },
    { "Archer",  35, 15, 3, 0 },
    { "Knight",  70, 12, 3, 0 },
    { "Mage",    30, 18, 3, 0 }
};

static int _cursor = 0;
static int _sel_count = 0;

static void _count_selected(void) {
    int i;
    _sel_count = 0;
    for (i = 0; i < NUM_CHARS; i++) {
        if (_chars[i].selected) _sel_count++;
    }
}

/* ---- public API ---- */

void menu_init(void) {
    int i;
    _cursor = 0;
    for (i = 0; i < NUM_CHARS; i++)
        _chars[i].selected = 0;
    /* auto-select hero */
    _chars[0].selected = 1;
    _count_selected();
}

int menu_update(void) {
    int key;

    key = input_read_key();
    if (key < 0) return MENU_PENDING;

    switch (key) {
    case KEY_UP:
        _cursor--;
        if (_cursor < 0) _cursor = NUM_CHARS - 1;
        break;
    case KEY_DOWN:
        _cursor++;
        if (_cursor >= NUM_CHARS) _cursor = 0;
        break;
    case KEY_SPACE:
        _chars[_cursor].selected = !_chars[_cursor].selected;
        _count_selected();
        break;
    case KEY_ENTER:
        if (_sel_count >= 1) return _sel_count;
        break;
    case KEY_ESC:
        return MENU_CANCEL;
    }

    return MENU_PENDING;
}

void menu_draw(void) {
    int i;
    int y;
    char buf[64];
    int sel_color;

    gfx_clear(0);

    /* title bar */
    gfx_fill_rect(0, 0, 320, 18, 1);
    font_puts(4, 1, "Party Selection", 15);
    font_puts(176, 1, "Select your party", 8);

    /* character list */
    for (i = 0; i < NUM_CHARS; i++) {
        y = 24 + i * 28;

        /* highlight current row */
        if (i == _cursor) {
            gfx_fill_rect(4, y, 312, 26, 2);
        }

        /* cursor indicator */
        if (i == _cursor)
            font_puts(8, y + 5, ">", 15);

        /* selection checkbox */
        sel_color = _chars[i].selected ? 88 : 8;
        font_puts(24, y + 5,
                  _chars[i].selected ? "*" : "-",
                  sel_color);

        /* name */
        font_puts(40, y + 5, _chars[i].name, 15);

        /* stats */
        sprintf(buf, "HP:%d ATK:%d MOV:%d",
                _chars[i].hp, _chars[i].atk, _chars[i].mov);
        font_puts(128, y + 5, buf, 10);
    }

    /* footer */
    gfx_fill_rect(0, 168, 320, 32, 1);
    gfx_hline(0, 319, 168, 8);

    sprintf(buf, "Selected: %d/%d", _sel_count, NUM_CHARS);
    font_puts(4, 172, buf, 15);

    font_puts(4, 186, "SPACE:Toggle  ENTER:Start  ESC:Back", 10);

    if (_sel_count == 0) {
        font_puts(180, 172, "Select at least 1!", 24);
    }
}

int menu_get_selected(int *out, int max) {
    int i, count = 0;
    for (i = 0; i < NUM_CHARS && count < max; i++) {
        if (_chars[i].selected) {
            out[count++] = i;
        }
    }
    return count;
}
