#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

/* scancodes for common keys */
#define KEY_ESC    0x01
#define KEY_ENTER  0x1C
#define KEY_SPACE  0x39
#define KEY_LEFT   0x4B
#define KEY_RIGHT  0x4D
#define KEY_UP     0x48
#define KEY_DOWN   0x50
#define KEY_A      0x1E
#define KEY_D      0x20
#define KEY_S      0x1F
#define KEY_W      0x11
#define KEY_LSHIFT 0x2A
#define KEY_LCTRL  0x1D

/* real-time key state (for game loop) */
void input_init(void);
void input_close(void);
int  input_key(unsigned char scancode);

/* event-based key input (for menus, wait screens) */
int  input_wait_key(void);   /* block until key press, return scancode */
int  input_read_key(void);   /* non-blocking: return scancode or -1 */
void input_flush(void);      /* discard buffered key events */

#endif
