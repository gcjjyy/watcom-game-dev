#include <dos.h>
#include <conio.h>
#include <stdio.h>
#include <string.h>
#include "SOUND.H"

/* ---- OPL2 port definitions ---- */

#define OPL2_ADDR 0x388
#define OPL2_DATA 0x389

/* ---- VGM timing ----
 * VGM uses 44100 samples/sec for delays.
 * We are called from a 1000 Hz timer (TIMER.CPP owns PIT/INT 8).
 * Each 1ms tick = 44.1 VGM samples.
 * We use fixed-point: accumulate 441 per tick, threshold 10. */
#define VGM_SAMPLES_PER_TICK_X10  441  /* 44.1 * 10 */

/* ---- assembly delay helpers ---- */

/* ~3.3us delay: 6 reads of OPL2 status port */
void _opl2_delay_short(unsigned short port);
#pragma aux _opl2_delay_short = \
    "in al, dx" \
    "in al, dx" \
    "in al, dx" \
    "in al, dx" \
    "in al, dx" \
    "in al, dx" \
    parm [edx] \
    modify [eax];

/* ~23us delay: 35 reads of OPL2 status port */
void _opl2_delay_long(unsigned short port);
#pragma aux _opl2_delay_long = \
    "mov ecx, 35" \
    "opl2dl:" \
    "in al, dx" \
    "dec ecx" \
    "jnz opl2dl" \
    parm [edx] \
    modify [eax ecx];

/* ---- OPL2 port I/O ---- */

static void opl2_write(unsigned char reg, unsigned char data) {
    outp(OPL2_ADDR, reg);
    _opl2_delay_short(OPL2_ADDR);
    outp(OPL2_DATA, data);
    _opl2_delay_long(OPL2_ADDR);
}

static void opl2_clear(void) {
    int i;
    for (i = 0; i < 256; i++) opl2_write(i, 0);
}

/* ---- OPL2 detection ---- */

static int _opl2_ready_cached = 0;

int snd_detect(void) {
    unsigned char s1, s2;
    opl2_write(0x04, 0x60);
    opl2_write(0x04, 0x80);
    s1 = inp(OPL2_ADDR);
    opl2_write(0x02, 0xFF);
    opl2_write(0x04, 0x21);
    _opl2_delay_long(OPL2_ADDR);
    _opl2_delay_long(OPL2_ADDR);
    s2 = inp(OPL2_ADDR);
    opl2_write(0x04, 0x60);
    opl2_write(0x04, 0x80);
    _opl2_ready_cached = ((s1 & 0xE0) == 0x00) && ((s2 & 0xE0) == 0xC0);
    return _opl2_ready_cached;
}

int snd_opl2_ready(void) {
    return _opl2_ready_cached;
}

/* ---- VGM state ---- */

static unsigned char  _vgm_data[SND_MAX_SIZE];
static unsigned long  _vgm_size;
static unsigned long  _vgm_data_off;
static unsigned long  _vgm_loop_off;

static volatile unsigned long _vgm_pos;
static volatile int           _vgm_playing;
static volatile long          _vgm_wait;   /* remaining VGM samples * 10 */

/* ---- hook into existing timer ISR ---- */

static void (__interrupt __far *_old_int8)(void);

/* ---- VGM file loading ---- */

int snd_load(const char *filename) {
    FILE *f;
    long fsize;
    unsigned long magic, rel;

    f = fopen(filename, "rb");
    if (!f) return SND_ERR;

    fseek(f, 0, SEEK_END);
    fsize = ftell(f);
    if (fsize <= 0 || (unsigned long)fsize > SND_MAX_SIZE) {
        fclose(f);
        return SND_ERR;
    }

    fseek(f, 0, SEEK_SET);
    fread(_vgm_data, 1, (size_t)fsize, f);
    fclose(f);

    memcpy(&magic, _vgm_data, 4);
    if (magic != 0x206D6756UL) return SND_ERR;  /* "Vgm " */

    memcpy(&rel, &_vgm_data[0x34], 4);
    _vgm_data_off = (rel == 0) ? 0x40 : (0x34 + rel);

    memcpy(&rel, &_vgm_data[0x1C], 4);
    _vgm_loop_off = (rel == 0) ? 0 : (0x1C + rel);

    _vgm_size = (unsigned long)fsize;
    return SND_OK;
}

/* ---- VGM command processing (called from ISR) ---- */

static void _vgm_process(void) {
    unsigned char cmd;
    while (_vgm_pos < _vgm_size) {
        cmd = _vgm_data[_vgm_pos++];
        if (cmd == 0x5A || cmd == 0x5B || cmd == 0x5C) {
            opl2_write(_vgm_data[_vgm_pos], _vgm_data[_vgm_pos + 1]);
            _vgm_pos += 2;
        } else if (cmd == 0x61) {
            _vgm_wait = ((long)_vgm_data[_vgm_pos]
                       | ((long)_vgm_data[_vgm_pos + 1] << 8)) * 10;
            _vgm_pos += 2;
            return;
        } else if (cmd == 0x62) {
            _vgm_wait = 7350;  /* 735 * 10 */
            return;
        } else if (cmd == 0x63) {
            _vgm_wait = 8820;  /* 882 * 10 */
            return;
        } else if (cmd == 0x66) {
            if (_vgm_loop_off > 0) {
                _vgm_pos = _vgm_loop_off;
            } else {
                _vgm_playing = 0;
                opl2_clear();
                return;
            }
        } else if (cmd >= 0x30 && cmd <= 0x4F) {
            _vgm_pos++;       /* 1 data byte */
        } else if (cmd == 0x50) {
            _vgm_pos++;       /* SN76489 write, skip 1 byte */
        }
        /* other unknown: 0 additional bytes (already consumed) */
    }
}

/* ---- timer ISR: chains through existing TIMER.CPP handler ---- */

static void __interrupt __far _snd_isr(void) {
    if (_vgm_playing) {
        _vgm_wait -= VGM_SAMPLES_PER_TICK_X10;
        if (_vgm_wait <= 0) {
            _vgm_process();
        }
    }
    _chain_intr(_old_int8);
}

/* ---- public API ---- */

void snd_init(void) {
    _vgm_playing = 0;
    _vgm_wait = 0;
    _old_int8 = _dos_getvect(0x08);
    _dos_setvect(0x08, _snd_isr);
}

void snd_close(void) {
    snd_stop();
    _dos_setvect(0x08, _old_int8);
}

void snd_play(void) {
    opl2_clear();
    _vgm_pos = _vgm_data_off;
    _vgm_wait = 0;
    _vgm_playing = 1;
}

void snd_stop(void) {
    _vgm_playing = 0;
    opl2_clear();
}

int snd_playing(void) {
    return _vgm_playing;
}
