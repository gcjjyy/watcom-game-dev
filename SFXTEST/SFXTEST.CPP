/* Minimal standalone SFX test harness.
 * Compile via SFXBUILD.BAT (at project root).
 * Usage (inside DOSBox): SFXTEST [slot]
 *   slot = 0..7, defaults to 0 = SFX_ATK
 *
 * Does init + load + play + spin + close, with a trace at each step
 * written to OUT.TXT (flushed eagerly).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include "..\SRC\SFX.H"
#include "..\SRC\TIMER.H"

static FILE *_log = NULL;

static void L(const char *msg) {
    if (_log) { fputs(msg, _log); fflush(_log); }
    fputs(msg, stdout); fflush(stdout);
}

static void Lfmt(const char *fmt, int a, int b, int c) {
    char buf[128];
    sprintf(buf, fmt, a, b, c);
    L(buf);
}

static const char *_slot_file[8] = {
    "..\\SRC\\SFX_ATK.SFX",
    "..\\SRC\\SFX_HIT.SFX",
    "..\\SRC\\SFX_SEL.SFX",
    "..\\SRC\\SFX_OK.SFX",
    "..\\SRC\\SFX_CAN.SFX",
    "..\\SRC\\SFX_WIN.SFX",
    "..\\SRC\\SFX_LOSE.SFX",
    "..\\SRC\\SFX_ENC.SFX"
};

static void spin_ms(unsigned long ms) {
    unsigned long t0 = timer_ms();
    while (timer_ms() - t0 < ms) { /* busy wait */ }
}

int main(int argc, char **argv) {
    int slot = 0;
    int rc;

    _log = fopen("LOG.TXT", "w");
    L("=== SFXTEST v2 ===\n");

    if (argc > 1) slot = atoi(argv[1]);
    if (slot < 0 || slot >= 8) slot = 0;
    Lfmt("slot = %d\n", slot, 0, 0);

    L("[1] timer_init\n");
    timer_init();
    L("[2] timer ok, calling sfx_init\n");

    rc = sfx_init();
    Lfmt("[3] sfx_init returned %d\n", rc, 0, 0);
    if (!rc) {
        L("sfx_init failed - no SB or DSP too old\n");
        timer_close();
        if (_log) fclose(_log);
        return 1;
    }

    Lfmt("[4] sfx_load slot=%d\n", slot, 0, 0);
    L("    file = "); L(_slot_file[slot]); L("\n");
    rc = sfx_load(slot, _slot_file[slot]);
    Lfmt("[5] sfx_load returned %d\n", rc, 0, 0);
    if (!rc) {
        L("sfx_load failed - shutting down\n");
        sfx_close();
        timer_close();
        if (_log) fclose(_log);
        return 2;
    }

    L("[6] sfx_play (1 shot, hold 1.5s)\n");
    sfx_play(slot);
    spin_ms(1500);

    L("[7] sfx_play x3 rapid (overlap test)\n");
    sfx_play(slot); spin_ms(100);
    sfx_play(slot); spin_ms(100);
    sfx_play(slot);
    spin_ms(2000);

    L("[8] sfx_stop_all + sfx_close + timer_close\n");
    sfx_stop_all();
    sfx_close();
    timer_close();

    L("=== DONE ===\n");
    if (_log) fclose(_log);
    return 0;
}
