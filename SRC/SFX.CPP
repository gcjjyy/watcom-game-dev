#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <conio.h>
#include <i86.h>
#include "SFX.H"
#include "TIMER.H"

/* ---- Sound Blaster PCM SFX software mixer ---------------------------------
 *
 * Continuous auto-init 8-bit DMA to the SB DSP at 22050 Hz mono.
 * Ring buffer is 2048 bytes (two 1024-byte halves).
 * IRQ (from BLASTER) fires every half-buffer completion (~46 ms).
 * The ISR refills the half that just finished by calling _mix_half(),
 * which sums up to MAX_VOICES active voices into the output buffer.
 *
 * sfx_play() marks a voice as active. Non-blocking — no DSP commands issued.
 * Runs parallel to SOUND.CPP's OPL2 VGM player on INT 8: fully disjoint.
 * No shared state, different ports, different interrupt vectors.
 */

/* ---- fixed constants ---- */

#define SB_BUFFER_SIZE      2048        /* full ring */
#define SB_HALF_SIZE        1024        /* half-buffer refill size */
#define SB_SAMPLE_RATE      22050       /* mono 8-bit u8 */
#define SB_TIME_CONSTANT    0xD3        /* 256 - (1000000 / 22050) = 211 */
#define SB_ALLOC_PARAS      256         /* 4096 bytes for 64KB-safe alignment */
#define SFX_MAGIC           0x58465343UL /* 'C','S','F','X' little-endian */

/* DSP command bytes */
#define DSP_CMD_SET_RATE        0x40
#define DSP_CMD_SET_BLOCK_SIZE  0x48
#define DSP_CMD_START_AUTO_8    0x1C
#define DSP_CMD_HALT_DMA        0xD0
#define DSP_CMD_SPEAKER_ON      0xD1
#define DSP_CMD_SPEAKER_OFF     0xD3
#define DSP_CMD_GET_VERSION     0xE1

/* ---- voice / slot structs (see design W2: volatile on mutables) ---- */

typedef struct {
    const unsigned char * volatile data;
    volatile int pos;
    volatile int len;
    volatile int active;
    unsigned long started_at;   /* write-once, non-volatile */
} Voice;

typedef struct {
    unsigned char *data;
    int len;
    int loaded;
} Slot;

/* ---- state ---- */

static int             _running      = 0;
static int             _base         = 0x220;
static int             _irq          = 5;
static int             _int_vector   = 0x0D;   /* IRQ5 -> INT 0x0D */
static int             _dma          = 1;

static unsigned char  *_dma_buffer   = NULL;   /* flat-mode pointer */
static unsigned long   _dma_linear   = 0;      /* 24-bit physical for 8237 */
static unsigned short  _dos_seg      = 0;      /* real-mode seg for free */

static volatile int    _half_active  = 0;      /* toggled in ISR */

static Voice           _voices[MAX_VOICES];
static Slot            _slots[MAX_SFX_SLOTS];

static void (__interrupt __far *_old_int_vec)(void) = NULL;
static int             _vector_installed = 0;
static unsigned char   _saved_imr    = 0;
static int             _imr_saved    = 0;

/* 8237 per-channel page register ports (design W1) */
static const unsigned char _dma_page_port[4] = { 0x87, 0x83, 0x81, 0x82 };

/* forward */
static void _mix_half(unsigned char *dst);

/* ---- DSP low-level helpers ------------------------------------------------ */

static void _io_delay(void) {
    int i;
    unsigned char dummy;
    for (i = 0; i < 6; i++) dummy = (unsigned char)inp(0x80);
    (void)dummy;
}

static int _dsp_write(int base, unsigned char v) {
    int tries;
    for (tries = 0; tries < 65535; tries++) {
        if ((inp(base + 0x0C) & 0x80) == 0) {
            outp(base + 0x0C, v);
            return 1;
        }
    }
    return 0;
}

static int _dsp_read(int base, unsigned char *out) {
    int tries;
    for (tries = 0; tries < 65535; tries++) {
        if ((inp(base + 0x0E) & 0x80) != 0) {
            *out = (unsigned char)inp(base + 0x0A);
            return 1;
        }
    }
    return 0;
}

static int _dsp_reset(int base) {
    unsigned char v;
    int tries;
    outp(base + 0x06, 1);
    _io_delay(); _io_delay(); _io_delay();
    outp(base + 0x06, 0);
    for (tries = 0; tries < 256; tries++) {
        if ((inp(base + 0x0E) & 0x80) != 0) {
            v = (unsigned char)inp(base + 0x0A);
            if (v == 0xAA) return 1;
        }
    }
    return 0;
}

static int _dsp_version(int base, int *major, int *minor) {
    unsigned char mj, mn;
    if (!_dsp_write(base, DSP_CMD_GET_VERSION)) return 0;
    if (!_dsp_read(base, &mj)) return 0;
    if (!_dsp_read(base, &mn)) return 0;
    *major = mj; *minor = mn;
    return 1;
}

/* ---- BLASTER parsing ----- */

static int _hex_digit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

static int _parse_blaster(void) {
    const char *env = getenv("BLASTER");
    int a = -1, i = -1, d = -1;
    int k, dig, v;
    char ch;
    /* defaults */
    _base = 0x220; _irq = 5; _dma = 1;
    if (!env) {
        printf("SFX: BLASTER not set, using default A220 I5 D1\n");
    } else {
        k = 0;
        while (env[k]) {
            ch = env[k];
            if (ch == 'A' || ch == 'a') {
                v = 0; k++;
                while ((dig = _hex_digit(env[k])) >= 0) { v = (v << 4) | dig; k++; }
                a = v;
            } else if (ch == 'I' || ch == 'i') {
                v = 0; k++;
                while (env[k] >= '0' && env[k] <= '9') { v = v * 10 + (env[k] - '0'); k++; }
                i = v;
            } else if (ch == 'D' || ch == 'd') {
                v = 0; k++;
                while (env[k] >= '0' && env[k] <= '9') { v = v * 10 + (env[k] - '0'); k++; }
                d = v;
            } else {
                k++;
            }
        }
        if (a == 0x210 || a == 0x220 || a == 0x230 || a == 0x240 ||
            a == 0x250 || a == 0x260 || a == 0x280) _base = a;
        if (i == 2 || i == 3 || i == 5 || i == 7) _irq = i;
        if (d == 0 || d == 1 || d == 3) _dma = d;
    }
    /* IRQ -> INT vector */
    if (_irq == 2 || _irq == 3 || _irq == 5 || _irq == 7) {
        _int_vector = 0x08 + _irq;
        return 1;
    }
    return 0;
}

/* ---- DOS low-memory DMA buffer ----
 * Allocate conventional memory via DPMI INT 31h AX=0100h.
 * Watcom's _dos_allocmem under DOS4GW returns the protected-mode selector
 * (not the real-mode segment), so (seg<<4) becomes garbage. DPMI 0100h
 * returns AX=real_seg, DX=pm_sel separately. (real_seg<<4) is the physical
 * address for the 8237 DMA, and — since Watcom flat DS base is 0 — it is
 * also a valid C pointer for the first 1 MB. */
static unsigned short _dos_pm_sel = 0;
static int _alloc_dma_buffer(void) {
    union REGS r;
    unsigned long linear, start, end;
    r.w.ax = 0x0100;
    r.w.bx = SB_ALLOC_PARAS;
    int386(0x31, &r, &r);
    if (r.x.cflag) return 0;
    _dos_seg    = r.w.ax;
    _dos_pm_sel = r.w.dx;
    linear = ((unsigned long)_dos_seg) << 4;
    start = linear;
    end = start + SB_BUFFER_SIZE - 1;
    if ((start & 0xFFFF0000UL) == (end & 0xFFFF0000UL)) {
        _dma_linear = start;
    } else {
        _dma_linear = (start | 0xFFFFUL) + 1UL;
    }
    _dma_buffer = (unsigned char *)_dma_linear;
    memset(_dma_buffer, 128, SB_BUFFER_SIZE);
    return 1;
}

static void _free_dma_buffer(void) {
    if (_dos_pm_sel != 0) {
        union REGS r;
        r.w.ax = 0x0101;
        r.w.dx = _dos_pm_sel;
        int386(0x31, &r, &r);
        _dos_pm_sel = 0;
        _dos_seg = 0;
        _dma_buffer = NULL;
        _dma_linear = 0;
    }
}

/* ---- 8237 DMA programming ------------------------------------------------- */

static void _program_dma(int channel, unsigned long linear_addr, int count) {
    int addr_port  = channel * 2;
    int count_port = channel * 2 + 1;
    int page_port  = _dma_page_port[channel];
    int count_m1   = count - 1;
    outp(0x0A, 0x04 | channel);                /* mask channel */
    outp(0x0C, 0x00);                           /* clear flip-flop */
    outp(0x0B, 0x58 | channel);                 /* mode: single+autoinit+read */
    outp(addr_port, (int)(linear_addr & 0xFF));
    outp(addr_port, (int)((linear_addr >> 8) & 0xFF));
    outp(page_port, (int)((linear_addr >> 16) & 0xFF));
    outp(count_port, count_m1 & 0xFF);
    outp(count_port, (count_m1 >> 8) & 0xFF);
    outp(0x0A, channel);                        /* unmask */
}

/* ---- PIC IMR management --------------------------------------------------- */

static void _unmask_irq(int irq) {
    _saved_imr = (unsigned char)inp(0x21);
    _imr_saved = 1;
    outp(0x21, _saved_imr & ~(1 << irq));
}

static void _restore_imr(void) {
    if (_imr_saved) {
        unsigned char cur = (unsigned char)inp(0x21);
        /* restore only our bit (mask it back to the original) */
        cur = (unsigned char)((cur & ~(1 << _irq)) |
                              (_saved_imr & (1 << _irq)));
        outp(0x21, cur);
        _imr_saved = 0;
    }
}

/* ---- Software mixer ------------------------------------------------------- */

static void _mix_half(unsigned char *dst) {
    int i, v, n_active = 0;
    int sum;
    Voice *vp;
    /* fast path: if no voices active, output silence */
    for (v = 0; v < MAX_VOICES; v++) if (_voices[v].active) { n_active++; break; }
    if (n_active == 0) {
        memset(dst, 128, SB_HALF_SIZE);
        return;
    }
    for (i = 0; i < SB_HALF_SIZE; i++) {
        sum = 0;
        for (v = 0; v < MAX_VOICES; v++) {
            vp = &_voices[v];
            if (!vp->active) continue;
            sum += (int)vp->data[vp->pos] - 128;
            vp->pos++;
            if (vp->pos >= vp->len) vp->active = 0;
        }
        if (sum < -128) sum = -128;
        if (sum >  127) sum =  127;
        dst[i] = (unsigned char)(sum + 128);
    }
}

/* ---- IRQ handler ---------------------------------------------------------- */

static void __interrupt __far _sfx_isr(void) {
    unsigned char *half;
    if (_half_active == 0) {
        half = _dma_buffer + SB_HALF_SIZE;  /* refill back half */
        _half_active = 1;
    } else {
        half = _dma_buffer;                  /* refill front half */
        _half_active = 0;
    }
    _mix_half(half);
    /* Ack DSP (8-bit: base+0x0E) */
    (void)inp(_base + 0x0E);
    /* Send PIC EOI */
    outp(0x20, 0x20);
    /* No _chain_intr — IRQ 5 is our dedicated vector */
}

/* ---- Public API: init ----------------------------------------------------- */

int sfx_init(void) {
    int i, major = 0, minor = 0;
    /* reset all state */
    _running = 0;
    _dma_buffer = NULL; _dma_linear = 0; _dos_seg = 0;
    _vector_installed = 0; _imr_saved = 0; _half_active = 0;
    for (i = 0; i < MAX_VOICES; i++) {
        _voices[i].data = NULL; _voices[i].pos = 0;
        _voices[i].len = 0; _voices[i].active = 0;
        _voices[i].started_at = 0;
    }
    for (i = 0; i < MAX_SFX_SLOTS; i++) {
        _slots[i].data = NULL; _slots[i].len = 0; _slots[i].loaded = 0;
    }
    if (!_parse_blaster()) {
        printf("SFX: invalid BLASTER IRQ, running in no-op mode\n");
        return 0;
    }
    if (!_dsp_reset(_base)) {
        printf("SFX: DSP reset failed at base 0x%X, no-op mode\n", _base);
        return 0;
    }
    if (!_dsp_version(_base, &major, &minor)) {
        printf("SFX: DSP version query failed, no-op mode\n");
        return 0;
    }
    if (major < 2) {
        printf("SFX: DSP v%d.%d lacks auto-init DMA, no-op mode\n", major, minor);
        return 0;
    }
    if (!_alloc_dma_buffer()) {
        printf("SFX: DPMI DOS alloc failed, no-op mode\n");
        return 0;
    }
    /* install ISR */
    _old_int_vec = _dos_getvect(_int_vector);
    _dos_setvect(_int_vector, _sfx_isr);
    _vector_installed = 1;
    _unmask_irq(_irq);
    /* program 8237 for full 2048-byte ring */
    _program_dma(_dma, _dma_linear, SB_BUFFER_SIZE);
    /* SB16 mixer: DSP reset muted FM output — re-enable master + FM + DAC */
    outp(_base+4,0x22); outp(_base+5,0xFF); outp(_base+4,0x26); outp(_base+5,0xFF); outp(_base+4,0x04); outp(_base+5,0xFF);
    /* DSP: speaker on, set rate, set block size (half-buffer), start auto-init */
    _dsp_write(_base, DSP_CMD_SPEAKER_ON);
    _dsp_write(_base, DSP_CMD_SET_RATE);
    _dsp_write(_base, SB_TIME_CONSTANT);
    _dsp_write(_base, DSP_CMD_SET_BLOCK_SIZE);
    _dsp_write(_base, (SB_HALF_SIZE - 1) & 0xFF);
    _dsp_write(_base, ((SB_HALF_SIZE - 1) >> 8) & 0xFF);
    _dsp_write(_base, DSP_CMD_START_AUTO_8);
    _running = 1;
    printf("SFX: SB DSP v%d.%d ready at A%X I%d D%d\n",
           major, minor, _base, _irq, _dma);
    return 1;
}

/* ---- Public API: close (idempotent, LIFO unwind) -------------------------- */

void sfx_close(void) {
    int i;
    if (_running) {
        /* halt DSP auto-init and silence speaker */
        _dsp_write(_base, DSP_CMD_HALT_DMA);
        _dsp_write(_base, DSP_CMD_SPEAKER_OFF);
        /* mask DMA channel on 8237 */
        outp(0x0A, 0x04 | _dma);
    }
    _restore_imr();
    if (_vector_installed) {
        _dos_setvect(_int_vector, _old_int_vec);
        _vector_installed = 0;
    }
    _free_dma_buffer();
    for (i = 0; i < MAX_SFX_SLOTS; i++) {
        if (_slots[i].data) {
            free(_slots[i].data);
            _slots[i].data = NULL;
        }
        _slots[i].len = 0; _slots[i].loaded = 0;
    }
    for (i = 0; i < MAX_VOICES; i++) {
        _voices[i].data = NULL; _voices[i].active = 0;
        _voices[i].pos = 0; _voices[i].len = 0;
    }
    _running = 0;
}

/* ---- Public API: load a .SFX file into a slot ----------------------------- */

int sfx_load(int slot, const char *filename) {
    FILE *f;
    unsigned char hdr[12];
    unsigned long magic, rate, len;
    unsigned char *buf;
    if (slot < 0 || slot >= MAX_SFX_SLOTS || !filename) return 0;
    /* free any previous slot allocation */
    if (_slots[slot].data) {
        free(_slots[slot].data);
        _slots[slot].data = NULL;
        _slots[slot].len = 0;
        _slots[slot].loaded = 0;
    }
    f = fopen(filename, "rb");
    if (!f) return 0;
    if (fread(hdr, 1, 12, f) != 12) { fclose(f); return 0; }
    magic = ((unsigned long)hdr[0])        |
            ((unsigned long)hdr[1] <<  8)  |
            ((unsigned long)hdr[2] << 16)  |
            ((unsigned long)hdr[3] << 24);
    rate  = ((unsigned long)hdr[4])        |
            ((unsigned long)hdr[5] <<  8)  |
            ((unsigned long)hdr[6] << 16)  |
            ((unsigned long)hdr[7] << 24);
    len   = ((unsigned long)hdr[ 8])       |
            ((unsigned long)hdr[ 9] <<  8) |
            ((unsigned long)hdr[10] << 16) |
            ((unsigned long)hdr[11] << 24);
    if (magic != SFX_MAGIC || rate != SB_SAMPLE_RATE || len == 0 ||
        len > (1024UL * 1024UL)) {
        fclose(f);
        return 0;
    }
    buf = (unsigned char *)malloc((size_t)len);
    if (!buf) { fclose(f); return 0; }
    if (fread(buf, 1, (size_t)len, f) != len) {
        free(buf); fclose(f); return 0;
    }
    fclose(f);
    _slots[slot].data = buf;
    _slots[slot].len = (int)len;
    _slots[slot].loaded = 1;
    return 1;
}

/* ---- Public API: play a slot (voice alloc / steal) ------------------------ */

void sfx_play(int slot) {
    int i, pick;
    unsigned long highest;
    unsigned long ratio;
    if (!_running) return;
    if (slot < 0 || slot >= MAX_SFX_SLOTS) return;
    if (!_slots[slot].loaded || !_slots[slot].data) return;

    _disable();
    pick = -1;
    for (i = 0; i < MAX_VOICES; i++) {
        if (!_voices[i].active) { pick = i; break; }
    }
    if (pick < 0) {
        /* steal voice closest to completion (highest pos/len ratio) */
        highest = 0;
        pick = 0;
        for (i = 0; i < MAX_VOICES; i++) {
            if (_voices[i].len <= 0) { pick = i; break; }
            ratio = ((unsigned long)_voices[i].pos * 1000UL) /
                    (unsigned long)_voices[i].len;
            if (ratio >= highest) { highest = ratio; pick = i; }
        }
    }
    _voices[pick].data = _slots[slot].data;
    _voices[pick].len  = _slots[slot].len;
    _voices[pick].pos  = 0;
    _voices[pick].started_at = timer_ms();
    _voices[pick].active = 1;    /* set LAST */
    _enable();
}

/* ---- Public API: stop all voices ------------------------------------------ */

void sfx_stop_all(void) {
    int i;
    _disable();
    for (i = 0; i < MAX_VOICES; i++) {
        _voices[i].active = 0;
        _voices[i].pos = 0;
        _voices[i].len = 0;
        _voices[i].data = NULL;
    }
    _enable();
}
