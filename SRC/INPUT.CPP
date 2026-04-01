#include <dos.h>
#include <conio.h>
#include "INPUT.H"

/* real-time key state array */
static volatile unsigned char _keys[256];

/* ring buffer for key press events */
#define KEY_BUF_SIZE 32
static volatile unsigned char _keybuf[KEY_BUF_SIZE];
static volatile int _buf_head = 0;
static volatile int _buf_tail = 0;

static void (__interrupt __far *_old_int9)(void);
static volatile int _extended = 0;

static void __interrupt __far _key_handler(void) {
    unsigned char scancode = inp(0x60);
    unsigned char ack = inp(0x61);
    int next;
    outp(0x61, ack | 0x80);
    outp(0x61, ack);

    if (scancode == 0xE0) {
        _extended = 1;
        outp(0x20, 0x20);
        return;
    }

    _extended = 0;

    if (scancode & 0x80) {
        _keys[scancode & 0x7F] = 0;
    } else {
        _keys[scancode] = 1;
        next = (_buf_head + 1) & (KEY_BUF_SIZE - 1);
        if (next != _buf_tail) {
            _keybuf[_buf_head] = scancode;
            _buf_head = next;
        }
    }

    outp(0x20, 0x20);
}

void input_init(void) {
    int i;
    for (i = 0; i < 256; i++) _keys[i] = 0;
    _buf_head = 0;
    _buf_tail = 0;
    _old_int9 = _dos_getvect(0x09);
    _dos_setvect(0x09, _key_handler);
}

void input_close(void) {
    _dos_setvect(0x09, _old_int9);
}

int input_key(unsigned char scancode) {
    return _keys[scancode];
}

void _hlt(void);
#pragma aux _hlt = "hlt";

int input_wait_key(void) {
    int sc;
    while (_buf_head == _buf_tail) _hlt();
    sc = _keybuf[_buf_tail];
    _buf_tail = (_buf_tail + 1) & (KEY_BUF_SIZE - 1);
    return sc;
}

int input_read_key(void) {
    int sc;
    if (_buf_head == _buf_tail) return -1;
    sc = _keybuf[_buf_tail];
    _buf_tail = (_buf_tail + 1) & (KEY_BUF_SIZE - 1);
    return sc;
}

void input_flush(void) {
    _buf_tail = _buf_head;
}
