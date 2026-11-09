/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * modes_15khz_test: which analog mode hdcap_find_mode() picks for 15 kHz signals (and that the
 * neighbouring 31 kHz and interlaced matches are unchanged), plus the table layout rules the
 * 15 kHz entries depend on.
 *
 *   cc -Iinclude $(pkg-config --cflags libusb-1.0) -o modes_15khz_test tests/modes_15khz_test.c \
 *      libhdcap.a $(pkg-config --libs libusb-1.0) && ./modes_15khz_test
 */
#include "hdcap.h"
#include <stdio.h>
#include <string.h>

static int fails;

static void expect(hdcap_input in, int vtotal, double khz, int interlaced, const char *want, const char *what)
{
    hdcap_signal s;
    memset(&s, 0, sizeof s);
    s.locked = 1;
    s.vtotal = vtotal;
    s.hfreq_hz = khz * 1000;
    s.interlaced = interlaced;
    const hdcap_mode *m = hdcap_find_mode(in, &s);
    const char *got = m ? m->name : "none";
    int ok = want ? (m && strstr(got, want) != NULL) : !m;
    printf("%-52s -> %-28s%s\n", what, got, ok ? "" : "  <-- WRONG");
    fails += !ok;
}

int main(void)
{
    /* 240p: 262 or 263 lines depending on the source */
    expect(HDCAP_INPUT_VGA, 263, 15.623, 0, "240p, 15 kHz", "VGA RGB, MiSTer 263 lines, 15.623 kHz (seen live)");
    expect(HDCAP_INPUT_VGA, 262, 15.734, 0, "240p, 15 kHz", "VGA RGB, 262 lines, 15.734 kHz");
    expect(HDCAP_INPUT_VGA, 262, 15.700, 0, "240p, 15 kHz", "VGA RGB, 262 lines, 15.700 kHz");
    expect(HDCAP_INPUT_COMPONENT, 263, 15.734, 0, "240p, 15 kHz", "Component, 263 lines, 15.734 kHz");
    /* 288p */
    expect(HDCAP_INPUT_VGA, 312, 15.625, 0, "288p, 15 kHz", "VGA RGB, 312 lines, 15.625 kHz");
    expect(HDCAP_INPUT_VGA, 313, 15.625, 0, "288p, 15 kHz", "VGA RGB, 313 lines, 15.625 kHz");
    expect(HDCAP_INPUT_COMPONENT, 312, 15.625, 0, "288p, 15 kHz", "Component, 312 lines, 15.625 kHz");
    /* unchanged neighbours */
    expect(HDCAP_INPUT_VGA, 525, 31.469, 0, "640x480@60 (VESA)", "VGA 640x480@60 (unchanged)");
    expect(HDCAP_INPUT_COMPONENT, 525, 31.469, 0, "480p", "Component 480p (unchanged)");
    expect(HDCAP_INPUT_COMPONENT, 262, 15.734, 1, "480i", "Component 480i, 262 lines per field (unchanged)");
    expect(HDCAP_INPUT_VGA, 400, 15.734, 0, NULL, "VGA 400 lines at 15.7 kHz: no match (unchanged)");

    /* Manual mode choices are stored as indices (OBS settings, hdcapd.conf), so new modes must stay
     * at the end of the table; and the 15 kHz windows must hold exactly one 240/288-line field. */
    size_t n;
    const hdcap_mode *all = hdcap_modes(&n);
    int ok = n >= 2 && strstr(all[n - 2].name, "240p, 15 kHz") && strstr(all[n - 1].name, "288p, 15 kHz") &&
             all[n - 2].vactive == 240 && all[n - 1].vactive == 288 && !all[n - 2].interlaced && !all[n - 1].interlaced &&
             all[n - 2].hactive == 720 && all[n - 1].hactive == 720;
    printf("15 kHz modes are the last two entries, 720x240 and 720x288, progressive%s\n", ok ? "" : "  <-- WRONG");
    fails += !ok;

    printf("%s\n", fails ? "FAILURES" : "ALL OK");
    return fails != 0;
}
