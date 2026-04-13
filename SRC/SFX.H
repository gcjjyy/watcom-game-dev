#ifndef SFX_H_INCLUDED
#define SFX_H_INCLUDED

/* ---- Sound Blaster PCM SFX software mixer ----
 *
 * Overlapping SFX via auto-init DMA + IRQ-driven software mixer.
 * Coexists with OPL2 VGM music (SOUND.CPP) — disjoint hardware / vectors.
 * If SB is absent or DSP version < 2, the module enters no-op mode and
 * all public calls silently succeed without touching hardware.
 */

#define MAX_SFX_SLOTS 16
#define MAX_VOICES     8

/* Slot indices (0..7). Keep in sync with the 8 SFX_*.SFX files. */
#define SFX_ATK   0
#define SFX_HIT   1
#define SFX_SEL   2
#define SFX_OK    3
#define SFX_CAN   4
#define SFX_WIN   5
#define SFX_LOSE  6
#define SFX_ENC   7

/* Public API (match sprint contract AC-08 — exactly 5 functions). */
int  sfx_init(void);                              /* 1 = ready, 0 = no-op */
void sfx_close(void);
int  sfx_load(int slot, const char *filename);    /* 1 on success */
void sfx_play(int slot);                          /* no-op if invalid */
void sfx_stop_all(void);

#endif
