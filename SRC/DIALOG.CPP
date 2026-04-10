#include <string.h>
#include "DIALOG.H"
#include "GFX.H"
#include "FONT.H"
#include "INPUT.H"
#include "TIMER.H"

#define DLG_X       4
#define DLG_Y       132
#define DLG_W       312
#define DLG_H       64
#define DLG_TEXT_X  12
#define DLG_TEXT_Y  140
#define DLG_LINE_W  296
#define DLG_LINES   2
#define DLG_MAX_TEXT 512
#define DLG_MAX_PAGES 8

static int _active = 0;
static char _text[DLG_MAX_TEXT];
static int _text_len = 0;

/* page break offsets: _pages[i] is the start char offset of page i */
static int _pages[DLG_MAX_PAGES];
static int _page_count = 0;
static int _cur_page = 0;
static unsigned long _blink_time = 0;

/* ---- UTF-8 helpers ---- */

static int _is_utf8_lead(unsigned char c) {
    return (c & 0x80) == 0 || (c & 0xC0) == 0xC0;
}

static int _utf8_char_len(unsigned char c) {
    if (c < 0x80) return 1;
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 1;
}

/* ---- word-wrap pagination ---- */

static void _paginate(void) {
    int pos = 0;
    int line, line_start;
    char word[128];
    int word_len;
    int word_w;
    int line_w;

    _page_count = 0;
    _pages[0] = 0;

    while (pos < _text_len && _page_count < DLG_MAX_PAGES) {
        _pages[_page_count] = pos;
        _page_count++;

        for (line = 0; line < DLG_LINES && pos < _text_len; line++) {
            line_w = 0;
            line_start = pos;

            while (pos < _text_len) {
                /* skip spaces */
                while (pos < _text_len && _text[pos] == ' ') pos++;
                if (pos >= _text_len) break;

                /* read one word */
                word_len = 0;
                while (pos + word_len < _text_len &&
                       _text[pos + word_len] != ' ' &&
                       word_len < 120) {
                    int cl = _utf8_char_len((unsigned char)_text[pos + word_len]);
                    int k;
                    for (k = 0; k < cl && pos + word_len + k < _text_len; k++)
                        word[word_len + k] = _text[pos + word_len + k];
                    word_len += cl;
                }
                word[word_len] = '\0';

                word_w = font_text_width(word);

                /* word fits on current line? */
                if (line_w > 0 && line_w + 4 + word_w > DLG_LINE_W) {
                    /* doesn't fit, break to next line */
                    break;
                }

                if (word_w > DLG_LINE_W && line_w == 0) {
                    /* single word too wide: character-level break */
                    int ci = 0;
                    char ch_buf[5];
                    while (ci < word_len) {
                        int cl = _utf8_char_len((unsigned char)word[ci]);
                        int k;
                        for (k = 0; k < cl; k++) ch_buf[k] = word[ci + k];
                        ch_buf[cl] = '\0';
                        int cw = font_text_width(ch_buf);
                        if (line_w + cw > DLG_LINE_W) break;
                        line_w += cw;
                        ci += cl;
                    }
                    pos += ci;
                    break;
                }

                if (line_w > 0) line_w += 4; /* space width */
                line_w += word_w;
                pos += word_len;
            }
        }
    }
}

/* ---- public API ---- */

void dlg_show(const char *text) {
    int len;
    len = strlen(text);
    if (len >= DLG_MAX_TEXT) len = DLG_MAX_TEXT - 1;
    memcpy(_text, text, len);
    _text[len] = '\0';
    _text_len = len;
    _cur_page = 0;
    _paginate();
    _blink_time = timer_ms();
    _active = 1;
    input_flush();
}

int dlg_update(void) {
    int key;
    if (!_active) return 0;

    key = input_read_key();
    if (key == KEY_SPACE || key == KEY_ENTER) {
        _cur_page++;
        if (_cur_page >= _page_count) {
            _active = 0;
            return 1;
        }
    } else if (key == KEY_ESC) {
        _active = 0;
        return 1;
    }
    return 0;
}

void dlg_draw(void) {
    int start, end;
    int pos, line, line_w;
    char word[128];
    int word_len, word_w;
    unsigned long now;
    int blink;

    if (!_active) return;

    /* box background */
    gfx_fill_rect(DLG_X, DLG_Y, DLG_W, DLG_H, 1);
    gfx_rect(DLG_X, DLG_Y, DLG_W, DLG_H, 15);
    gfx_rect(DLG_X + 2, DLG_Y + 2, DLG_W - 4, DLG_H - 4, 8);

    /* text rendering */
    start = _pages[_cur_page];
    end = (_cur_page + 1 < _page_count) ?
          _pages[_cur_page + 1] : _text_len;

    pos = start;
    for (line = 0; line < DLG_LINES && pos < end; line++) {
        int lx = DLG_TEXT_X;
        int ly = DLG_TEXT_Y + line * 18;

        while (pos < end) {
            while (pos < end && _text[pos] == ' ') pos++;
            if (pos >= end) break;

            word_len = 0;
            while (pos + word_len < end &&
                   _text[pos + word_len] != ' ' &&
                   word_len < 120) {
                int cl = _utf8_char_len((unsigned char)_text[pos + word_len]);
                int k;
                for (k = 0; k < cl && pos + word_len + k < end; k++)
                    word[word_len + k] = _text[pos + word_len + k];
                word_len += cl;
            }
            word[word_len] = '\0';
            word_w = font_text_width(word);

            if (lx > DLG_TEXT_X && lx + 4 + word_w > DLG_TEXT_X + DLG_LINE_W)
                break;

            if (lx > DLG_TEXT_X) lx += 4;
            font_puts(lx, ly, word, 15);
            lx += word_w;
            pos += word_len;
        }
    }

    /* blinking advance indicator */
    now = timer_ms();
    blink = ((now - _blink_time) / 300) & 1;
    if (blink) {
        font_puts(304, 184, ">", 15);
    }
}

int dlg_active(void) {
    return _active;
}
