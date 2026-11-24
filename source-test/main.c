/* Diagnose-DOL fuer den source-app Loader-Handoff.
 *
 * KEIN terminal_print, KEIN bitmap. Nur rohes libogc-Video: Framebuffer
 * selbst anlegen, mit einer Volltonfarbe FUELLEN (im Gegensatz zu
 * video_init_custom, das den FB nicht loescht) und anzeigen.
 *
 * Auswertung nach dem Boot ueber source-app:
 *   - Bildschirm wird KOMPLETT BLAU  -> der Handoff funktioniert, die DOL
 *     laeuft, Video/Framebuffer/VI sind in Ordnung. Der bisherige "Glitch"
 *     war nur der ungeloeschte FB -> der eigentliche Fehler liegt in
 *     terminal_print / der bitmap-Lib, NICHT im Loader.
 *   - Bildschirm bleibt schwarz/Garbage -> Video-Setup selbst ist nach dem
 *     Handoff kaputt (VI/Arena/Framebuffer).
 */

#include <gccore.h>
#include <video.h>
#include <ogc/system.h>

#include <string.h>

int main(void) {
    VIDEO_Init();

    GXRModeObj *rmode = VIDEO_GetPreferredMode(NULL);
    void *fb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

    /* Framebuffer mit Vollton-BLAU fuellen (YUYV ~ blau). Ganze FB-Groesse. */
    u32 *p   = (u32 *)fb;
    u32  cnt = (rmode->fbWidth * rmode->xfbHeight) / 2; /* 2 Pixel pro u32 */
    for (u32 i = 0; i < cnt; i++)
        p[i] = 0x294a294a; /* blau in YUYV */

    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(fb);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if (rmode->viTVMode & VI_NON_INTERLACE)
        VIDEO_WaitVSync();

    while (1)
        VIDEO_WaitVSync();

    return 0;
}
