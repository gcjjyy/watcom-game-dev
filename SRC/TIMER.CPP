#include <dos.h>
#include <conio.h>
#include "TIMER.H"

#define PIT_FREQ     1193182UL
#define TARGET_HZ    1000
#define PIT_DIVISOR  (PIT_FREQ / TARGET_HZ)   /* 1193 -> ~1000.15 Hz */

static volatile unsigned long _ms_count = 0;
static unsigned long _bios_accum = 0;
static void (__interrupt __far *_old_int8)(void);

static void __interrupt __far _timer_isr(void) {
    _ms_count++;

    /* Chain to original BIOS INT 0x08 at ~18.2Hz to keep BIOS clock correct */
    _bios_accum += PIT_DIVISOR;
    if (_bios_accum >= 65536UL) {
        _bios_accum -= 65536UL;
        _chain_intr(_old_int8);   /* BIOS handler sends EOI */
    } else {
        outp(0x20, 0x20);         /* manual EOI when not chaining */
    }
}

void timer_init(void) {
    _ms_count = 0;
    _bios_accum = 0;
    _old_int8 = _dos_getvect(0x08);

    /* Reprogram PIT channel 0: lobyte/hibyte, mode 2 (rate generator) */
    outp(0x43, 0x34);
    outp(0x40, PIT_DIVISOR & 0xFF);
    outp(0x40, (PIT_DIVISOR >> 8) & 0xFF);

    _dos_setvect(0x08, _timer_isr);
}

void timer_close(void) {
    /* Restore original ISR first */
    _dos_setvect(0x08, _old_int8);

    /* Restore default PIT rate (~18.2 Hz, divisor 0 = 65536) */
    outp(0x43, 0x34);
    outp(0x40, 0x00);
    outp(0x40, 0x00);
}

unsigned long timer_ms(void) {
    return _ms_count;
}
