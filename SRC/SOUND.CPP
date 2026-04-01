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
    return ((s1 & 0xE0) == 0x00) && ((s2 & 0xE0) == 0xC0);
}

/* ---- VGM state ---- */

static unsigned char  _vgm_data[SND_MAX_SIZE];
static unsigned long  _vgm_size;
static unsigned long  _vgm_data_off;
static unsigned long  _vgm_loop_off;

static volatile unsigned long _vgm_pos;
static volatile int           _vgm_playing;
static volatile long          _vgm_wait;   /* remaining VGM samples * 10 */

/* ---- OPL2 channel/operator mapping ---- */

static const unsigned char _op_table[9][2] = {
    {0x00, 0x03}, {0x01, 0x04}, {0x02, 0x05},
    {0x08, 0x0B}, {0x09, 0x0C}, {0x0A, 0x0D},
    {0x10, 0x13}, {0x11, 0x14}, {0x12, 0x15}
};

/* ---- SFX instrument data (11 bytes each) ---- */

static const unsigned char _sfx_explosion[11] = {
    0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0, 0x7B, 0x7B, 0x01, 0x00, 0x00
};
static const unsigned char _sfx_laser[11] = {
    0x20, 0x21, 0x15, 0x00, 0xF0, 0xF0, 0x1F, 0x1F, 0x06, 0x01, 0x02
};
static const unsigned char _sfx_click[11] = {
    0x01, 0x01, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00
};

static const unsigned char * const _sfx_inst[SFX_COUNT] = {
    _sfx_explosion, _sfx_laser, _sfx_click
};

static const unsigned short _sfx_fnum[SFX_COUNT] = { 340, 647, 514 };
static const unsigned char  _sfx_block[SFX_COUNT] = { 1, 5, 4 };

static int _sfx_next_ch = 7;

/* ---- VGM channel filter (protect SFX channels 7-8) ---- */

static int _is_sfx_reg(unsigned char reg) {
    unsigned char off;
    /* per-channel registers: 0xA0+ch, 0xB0+ch, 0xC0+ch */
    if (reg == 0xA7 || reg == 0xA8) return 1;
    if (reg == 0xB7 || reg == 0xB8) return 1;
    if (reg == 0xC7 || reg == 0xC8) return 1;
    /* per-operator registers: bases 0x20, 0x40, 0x60, 0x80, 0xE0 */
    if (reg >= 0x20 && reg <= 0x35) off = reg - 0x20;
    else if (reg >= 0x40 && reg <= 0x55) off = reg - 0x40;
    else if (reg >= 0x60 && reg <= 0x75) off = reg - 0x60;
    else if (reg >= 0x80 && reg <= 0x95) off = reg - 0x80;
    else if (reg >= 0xE0 && reg <= 0xF5) off = reg - 0xE0;
    else return 0;
    /* ch7 ops: 0x11, 0x14; ch8 ops: 0x12, 0x15 */
    if (off == 0x11 || off == 0x14 || off == 0x12 || off == 0x15) return 1;
    return 0;
}

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
            if (!_is_sfx_reg(_vgm_data[_vgm_pos])) {
                opl2_write(_vgm_data[_vgm_pos], _vgm_data[_vgm_pos + 1]);
            }
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

/* ---- SFX playback ---- */

void snd_sfx_play(int id) {
    int ch;
    unsigned char m, c;
    const unsigned char *inst;

    if (id < 0 || id >= SFX_COUNT) return;

    ch = _sfx_next_ch;
    _sfx_next_ch = (ch == 8) ? 7 : 8;

    inst = _sfx_inst[id];
    m = _op_table[ch][0];
    c = _op_table[ch][1];

    opl2_write(0xB0 + ch, 0x00);          /* key-off */
    opl2_write(0x20 + m, inst[0]);         /* mod characteristics */
    opl2_write(0x20 + c, inst[1]);         /* car characteristics */
    opl2_write(0x40 + m, inst[2]);         /* mod level */
    opl2_write(0x40 + c, inst[3]);         /* car level */
    opl2_write(0x60 + m, inst[4]);         /* mod attack/decay */
    opl2_write(0x60 + c, inst[5]);         /* car attack/decay */
    opl2_write(0x80 + m, inst[6]);         /* mod sustain/release */
    opl2_write(0x80 + c, inst[7]);         /* car sustain/release */
    opl2_write(0xC0 + ch, inst[8]);        /* feedback/connection */
    opl2_write(0xE0 + m, inst[9]);         /* mod waveform */
    opl2_write(0xE0 + c, inst[10]);        /* car waveform */
    opl2_write(0xA0 + ch, _sfx_fnum[id] & 0xFF);
    opl2_write(0xB0 + ch, 0x20 | ((_sfx_block[id] & 0x07) << 2)
                         | ((_sfx_fnum[id] >> 8) & 0x03));
}
