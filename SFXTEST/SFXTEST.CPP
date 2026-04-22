/* Standalone OPL2 VGM + SB DSP SFX co-existence test harness.
 *
 * Compile via SFXBUILD.BAT (at project root, runs inside DOSBox).
 * Usage (inside DOSBox):  SFXTEST <scenario> [sfx_slot]
 *
 *   scenario = 1 : snd_init → snd_play → sfx_init
 *   scenario = 2 : snd_init → sfx_init → snd_play
 *   scenario = 3 : snd_init → sfx_init → snd_play → snd_stop + snd_play
 *   scenario = 4 : snd_init → sfx_init → opl2 manual reset → snd_play
 *
 * Writes a flushed trace to SNDLOG.TXT in the current dir.
 * Provides a 5-second listening window (prints countdown).
 * Then plays SFX_ATK three times rapidly for overlap confirmation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include "..\SRC\SFX.H"
#include "..\SRC\SOUND.H"
#include "..\SRC\TIMER.H"
#include "..\SRC\INPUT.H"
#include "..\SRC\GFX.H"

static FILE *_log = NULL;

static void L(const char *msg) {
    if (_log) { fputs(msg, _log); fflush(_log); }
}

static void Lf1(const char *fmt, int a) {
    char b[128]; sprintf(b, fmt, a); L(b);
}
static void Lf2(const char *fmt, int a, int b) {
    char bf[128]; sprintf(bf, fmt, a, b); L(bf);
}

static void spin_ms(unsigned long ms) {
    unsigned long t0 = timer_ms();
    while (timer_ms() - t0 < ms) { /* busy */ }
}

static void listen_window(int secs) {
    int i;
    Lf1("[listen window %d sec, VGM should be audible]\n", secs);
    for (i = 0; i < secs; i++) {
        spin_ms(1000);
        Lf2("  t=%d snd_playing=%d\n", i, snd_playing());
    }
}

int main(int argc, char **argv) {
    int scenario = 1;
    int slot = 0;
    int det1, det2, det3;
    int rc;

    if (argc > 1) scenario = atoi(argv[1]);
    if (argc > 2) slot = atoi(argv[2]);
    if (slot < 0 || slot >= 8) slot = 0;

    _log = fopen("SNDLOG.TXT", "w");
    L("=== SOUNDTEST v1 ===\n");
    Lf2("scenario=%d slot=%d\n", scenario, slot);

    L("[1] timer_init\n");
    timer_init();

    L("[2] snd_init\n");
    snd_init();

    det1 = snd_detect();
    Lf1("[3] snd_detect (before sfx) = %d\n", det1);

    if (scenario == 1) {
        /* ----- Scenario 1: VGM first, SFX after ----- */
        L("[S1a] snd_load TEST.VGM\n");
        rc = snd_load("..\\SRC\\TEST.VGM");
        Lf1("     snd_load rc = %d\n", rc);
        L("[S1b] snd_play\n");
        snd_play();
        L("[S1c] sfx_init\n");
        rc = sfx_init();
        Lf1("     sfx_init = %d\n", rc);
    } else if (scenario == 2) {
        /* ----- Scenario 2: SFX first, VGM after ----- */
        L("[S2a] sfx_init\n");
        rc = sfx_init();
        Lf1("     sfx_init = %d\n", rc);
        L("[S2b] snd_load TEST.VGM\n");
        rc = snd_load("..\\SRC\\TEST.VGM");
        Lf1("     snd_load rc = %d\n", rc);
        L("[S2c] snd_play\n");
        snd_play();
    } else if (scenario == 3) {
        /* ----- Scenario 3: SFX first, VGM, then stop+play again ----- */
        L("[S3a] sfx_init\n");
        rc = sfx_init();
        Lf1("     sfx_init = %d\n", rc);
        L("[S3b] snd_load TEST.VGM\n");
        snd_load("..\\SRC\\TEST.VGM");
        L("[S3c] snd_play\n");
        snd_play();
        L("[S3d] spin 200ms\n");
        spin_ms(200);
        L("[S3e] snd_stop\n");
        snd_stop();
        L("[S3f] snd_play again (restart)\n");
        snd_play();
    } else if (scenario == 4) {
        /* ----- Scenario 4: SFX first, manual opl2_clear via snd_stop+play trick ----- */
        L("[S4a] sfx_init\n");
        rc = sfx_init();
        Lf1("     sfx_init = %d\n", rc);
        /* snd_stop internally clears OPL2; calling it then snd_play forces
         * a full opl2_clear before VGM begins. */
        L("[S4b] snd_stop (force opl2_clear)\n");
        snd_stop();
        L("[S4c] snd_load\n");
        snd_load("..\\SRC\\TEST.VGM");
        L("[S4d] snd_play\n");
        snd_play();
    } else if (scenario == 5) {
        /* ----- Scenario 5: Full game replication — input_init + snd_detect guard ----- */
        L("[S5a] input_init (IRQ 1 hook like game does)\n");
        input_init();
        L("[S5b] sfx_init\n");
        rc = sfx_init();
        Lf1("     sfx_init = %d\n", rc);
        /* Critical: measure snd_detect RIGHT HERE — this is what game does */
        Lf1("[S5c] snd_detect RIGHT AFTER sfx_init = %d\n", snd_detect());
        L("[S5d] snd_load (always, regardless of detect)\n");
        rc = snd_load("..\\SRC\\TEST.VGM");
        Lf1("     snd_load rc = %d\n", rc);
        L("[S5e] snd_play\n");
        snd_play();
    } else if (scenario == 6) {
        /* ----- Scenario 6: Match game exactly — snd_detect guard blocks snd_play ----- */
        L("[S6a] input_init\n");
        input_init();
        L("[S6b] sfx_init\n");
        rc = sfx_init();
        Lf1("     sfx_init = %d\n", rc);
        Lf1("[S6c] snd_detect = %d (gated)\n", snd_detect());
        if (snd_detect()) {
            L("[S6d] snd_load (inside guard)\n");
            rc = snd_load("..\\SRC\\TEST.VGM");
            Lf1("     snd_load rc = %d\n", rc);
            if (rc == SND_OK) {
                L("[S6e] snd_play\n");
                snd_play();
            } else {
                L("[S6!] snd_load FAILED, no play\n");
            }
        } else {
            L("[S6!] snd_detect returned 0, snd_play NOT called\n");
        }
    } else if (scenario == 7) {
        /* ----- Scenario 7: Add gfx_init (VGA mode 13h) to game-like flow ----- */
        L("[S7a] input_init\n");
        input_init();
        L("[S7b] sfx_init\n");
        rc = sfx_init();
        Lf1("     sfx_init = %d\n", rc);
        Lf1("[S7c] snd_detect after sfx = %d\n", snd_detect());
        L("[S7d] gfx_init (VGA mode 13h)\n");
        gfx_init();
        Lf1("[S7e] snd_detect after gfx = %d\n", snd_detect());
        L("[S7f] snd_load\n");
        rc = snd_load("..\\SRC\\TEST.VGM");
        Lf1("     snd_load rc = %d\n", rc);
        L("[S7g] snd_play\n");
        snd_play();
    } else if (scenario == 8) {
        /* ----- Scenario 8: game-exact — snd_init BEFORE everything, sfx_init AFTER ----- */
        L("[S8a] input_init\n");
        input_init();
        L("[S8b] sfx_init\n");
        rc = sfx_init();
        Lf1("     sfx_init = %d\n", rc);
        L("[S8c] gfx_init (VGA mode 13h)\n");
        gfx_init();
        Lf1("[S8d] snd_detect = %d\n", snd_detect());
        L("[S8e] snd_load\n");
        rc = snd_load("..\\SRC\\TEST.VGM");
        Lf1("     snd_load rc = %d\n", rc);
        L("[S8f] snd_play\n");
        snd_play();
        /* After play, exit VGA mode so log is visible */
        L("[S8g] gfx_close\n");
        gfx_close();
    } else if (scenario == 9) {
        /* ----- Scenario 9: gfx+sfx_init+snd_play+sfx_load×8 ----- */
        static const char *files[8] = {
            "..\\SRC\\SFX_ATK.SFX",  "..\\SRC\\SFX_HIT.SFX",
            "..\\SRC\\SFX_SEL.SFX",  "..\\SRC\\SFX_OK.SFX",
            "..\\SRC\\SFX_CAN.SFX",  "..\\SRC\\SFX_WIN.SFX",
            "..\\SRC\\SFX_LOSE.SFX", "..\\SRC\\SFX_ENC.SFX"
        };
        int n;
        L("[S9a] input_init\n"); input_init();
        L("[S9b] gfx_init\n"); gfx_init();
        L("[S9c] sfx_init\n"); rc = sfx_init();
        L("[S9d] snd_load + snd_play\n");
        snd_load("..\\SRC\\TEST.VGM");
        snd_play();
        L("[S9e] sfx_load x 8\n");
        for (n = 0; n < 8; n++) {
            int lrc = sfx_load(n, files[n]);
            char tb[80];
            sprintf(tb, "    slot %d rc=%d snd_playing=%d\n", n, lrc, snd_playing());
            L(tb);
        }
        gfx_close();
    } else if (scenario == 10) {
        /* ----- Scenario 10: FIXED order (snd_play BEFORE sfx_init) ----- */
        L("[SAa] input_init\n"); input_init();
        L("[SAb] gfx_init\n"); gfx_init();
        L("[SAd] snd_detect guard + snd_load + snd_play\n");
        if (snd_detect()) {
            if (snd_load("..\\SRC\\TEST.VGM") == SND_OK) snd_play();
        }
        L("[SAe] sfx_init (AFTER snd_play - fixed order)\n");
        rc = sfx_init();
        Lf1("     sfx_init = %d\n", rc);
        L("[SAf] sfx_load x 8\n");
        {
            static const char *files2[8] = {
                "..\\SRC\\SFX_ATK.SFX",  "..\\SRC\\SFX_HIT.SFX",
                "..\\SRC\\SFX_SEL.SFX",  "..\\SRC\\SFX_OK.SFX",
                "..\\SRC\\SFX_CAN.SFX",  "..\\SRC\\SFX_WIN.SFX",
                "..\\SRC\\SFX_LOSE.SFX", "..\\SRC\\SFX_ENC.SFX"
            };
            int n;
            for (n = 0; n < 8; n++) {
                int lrc = sfx_load(n, files2[n]);
                char tb[80];
                sprintf(tb, "    slot %d rc=%d snd_playing=%d\n",
                        n, lrc, snd_playing());
                L(tb);
            }
        }
        gfx_close();
    } else if (scenario == 11) {
        /* ----- Scenario 11: BUG order (sfx_init BEFORE snd_play) ----- */
        L("[SBa] input_init\n"); input_init();
        L("[SBb] gfx_init\n"); gfx_init();
        L("[SBc] sfx_init BEFORE snd_play (BUG ORDER)\n");
        rc = sfx_init();
        Lf1("     sfx_init = %d\n", rc);
        L("[SBd] snd_detect guard + snd_load + snd_play\n");
        if (snd_detect()) {
            int lrc = snd_load("..\\SRC\\TEST.VGM");
            Lf1("     snd_load rc = %d\n", lrc);
            if (lrc == SND_OK) {
                L("     calling snd_play\n");
                snd_play();
            }
        } else {
            L("     ** snd_detect returned 0, snd_play NOT called **\n");
        }
        Lf1("[SBe] snd_playing after snd_play = %d\n", snd_playing());
        L("[SBf] sfx_load x 8\n");
        {
            static const char *files3[8] = {
                "..\\SRC\\SFX_ATK.SFX",  "..\\SRC\\SFX_HIT.SFX",
                "..\\SRC\\SFX_SEL.SFX",  "..\\SRC\\SFX_OK.SFX",
                "..\\SRC\\SFX_CAN.SFX",  "..\\SRC\\SFX_WIN.SFX",
                "..\\SRC\\SFX_LOSE.SFX", "..\\SRC\\SFX_ENC.SFX"
            };
            int n;
            for (n = 0; n < 8; n++) {
                int lrc = sfx_load(n, files3[n]);
                char tb[80];
                sprintf(tb, "    slot %d rc=%d snd_playing=%d\n",
                        n, lrc, snd_playing());
                L(tb);
            }
        }
        gfx_close();
    }

    det2 = snd_detect();
    Lf1("[4] snd_detect (after scenario) = %d\n", det2);
    Lf1("[5] snd_playing = %d\n", snd_playing());

    rc = sfx_load(slot, "..\\SRC\\SFX_ATK.SFX");
    Lf2("[6] sfx_load slot=%d rc=%d\n", slot, rc);

    listen_window(5);

    L("[7] sfx_play x3 rapid (overlap test)\n");
    sfx_play(slot); spin_ms(200);
    sfx_play(slot); spin_ms(200);
    sfx_play(slot); spin_ms(2000);

    det3 = snd_detect();
    Lf1("[8] snd_detect (after sfx plays) = %d\n", det3);
    Lf2("[9] summary: det before=%d, mid=%d\n", det1, det2);
    Lf1("               det after sfx plays=%d\n", det3);

    L("[10] sfx_close + snd_stop + snd_close + (input_close) + timer_close\n");
    sfx_stop_all();
    sfx_close();
    snd_stop();
    snd_close();
    if (scenario >= 5) input_close();
    if (scenario == 7 || scenario == 11) { /* S8/9/10 already called it */ }
    timer_close();

    L("=== DONE ===\n");
    fclose(_log);
    return 0;
}
