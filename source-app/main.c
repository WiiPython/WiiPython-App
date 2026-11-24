/* Copyright (c) 2026 lberwa  All rights reserved.                                                  *
 *                                                                                                  *
 *  \    /\    / | |  |¯¯\ \ / ¯¯|¯¯ |   |  /¯¯\  |\  |                                             *
 *   \  /  \  /  | |  |__/  Y    |   |---| |    | | \ |                                             *
 *    \/    \/   | |  |     |    |   |   |  \__/  |  \|                                             *
 *                                                                                                  *
 * this file is part of WiiPython project, which is licensed under the MIT license.                 *
 * See the LICENSE file in the project root for full license information.                           *
 * If you don't have a copy of the license, you can get one at https://opensource.org/licenses/MIT. */

#include "boot/boot.h"

#include <gccore.h>
#include <video.h>
#include <fat.h>
#include <bitmap/my_text_renderer.h>   // video_init_custom(), terminal_print()

#include <stdio.h>

#include <string.h>

/* ~1 Sekunde warten (60 Frames @ NTSC / 50 @ PAL), damit man die Ausgabe lesen kann */
static void wait_1s(void)
{
    for (int i = 0; i < 60; i++)
        VIDEO_WaitVSync();
}

/* Trace-Log: Text ausgeben und ~1 Sekunde pausieren, damit es lesbar ist */
static void tp_log(const char *text)
{
    terminal_print(text);
    wait_1s();
}

/* Debug-Trace-Logs: zeigen auf dem Bildschirm, wie weit main() kommt.
 * Zum Deaktivieren ALLER TP(...)-Logs einfach die erste Zeile auskommentieren
 * und die zweite aktivieren. */
// #define TP tp_log
#define TP(x) ((void)0)

int ini_get_string(
    const char *filename,
    const char *key,
    char *value,
    size_t value_size
)
{
    FILE *file = fopen(filename, "r");

    if (!file)
        return 0;

    char line[256];

    while (fgets(line, sizeof(line), file)) {
        char current_key[128];
        char current_value[128];

        if (sscanf(line, " %127[^=]= %127[^\n]",
            current_key, current_value) == 2) {

            if (strcmp(current_key, key) == 0) {
                strncpy(value, current_value, value_size - 1);
                value[value_size - 1] = '\0';

                fclose(file);
                return 1;
            }
        }
    }

    fclose(file);
    return 0;
}

int file_exists(const char *path)
{
    FILE *file = fopen(path, "r");

    if (file) {
        fclose(file);
        return 1;
    }

    return 0;
}

int main() {
    video_init_custom();
    TP("[main] video init ok");

    if (!fatInitDefault()) {
        terminal_print("Failed to initialize FAT filesystem");
        wait_1s();
        return false;
    }
    TP("[main] FAT init ok");

    char * path = NULL;
    char * dol = NULL;

    if (file_exists("sd:/python/config.ini")) {
        path = "sd:/python/config.ini";
    } else if (file_exists("usb:/python/config.ini")) {
        path = "usb:/python/config.ini";
    } else if (file_exists("sd:/python/python-libogc2.dol")) {
        dol = "sd:/python/python-libogc2.dol";
    } else if (file_exists("usb:/python/python-libogc2.dol")) {
        dol = "usb:/python/python-libogc2.dol";
    } else if (file_exists("sd:/python/python-libogc.dol")) {
        dol = "sd:/python/python-libogc.dol";
    } else if (file_exists("usb:/python/python-libogc.dol")) {
        dol = "usb:/python/python-libogc.dol";
    } else {
        terminal_print("No config.ini or python-....dol found on SD or USB");
        wait_1s();
        return 2;
    }
    TP("[main] config/dol-suche fertig");

    if (dol) {
        TP("[main] boote dol (direkt):");
        TP(dol);
        if (!boot_dol(dol, NULL)) {
            terminal_print("Failed to boot DOL:");
            terminal_print(dol);
            wait_1s();
        }

        return 0;
    }
    TP("[main] lese libogc aus config.ini");

    char libogc[2];

    if (ini_get_string(path, "libogc", libogc, sizeof(libogc)))
    {
      if (libogc[0] == '2') {
        if (file_exists("sd:/python/python-libogc2.dol")) {
            dol = "sd:/python/python-libogc2.dol";
        } else if (file_exists("usb:/python/python-libogc2.dol")) {
            dol = "usb:/python/python-libogc2.dol";
        }

      } else if (libogc[0] == '1') {
        if (file_exists("sd:/python/python-libogc.dol")) {
            dol = "sd:/python/python-libogc.dol";
        } else if (file_exists("usb:/python/python-libogc.dol")) {
            dol = "usb:/python/python-libogc.dol";
        }
      } else {
        if (file_exists("sd:/python/python-libogc2.dol")) {
            dol = "sd:/python/python-libogc2.dol";
        } else if (file_exists("usb:/python/python-libogc2.dol")) {
            dol = "usb:/python/python-libogc2.dol";
        } else if (file_exists("sd:/python/python-libogc.dol")) {
            dol = "sd:/python/python-libogc.dol";
        } else if (file_exists("usb:/python/python-libogc.dol")) {
            dol = "usb:/python/python-libogc.dol";
        } else {
            terminal_print("No python-....dol found on SD or USB");
            wait_1s();
            return 2;
        }
      }
    } else {
        if (file_exists("sd:/python/python-libogc2.dol")) {
            dol = "sd:/python/python-libogc2.dol";
        } else if (file_exists("usb:/python/python-libogc2.dol")) {
            dol = "usb:/python/python-libogc2.dol";
        } else if (file_exists("sd:/python/python-libogc.dol")) {
            dol = "sd:/python/python-libogc.dol";
        } else if (file_exists("usb:/python/python-libogc.dol")) {
            dol = "usb:/python/python-libogc.dol";
        } else {
            terminal_print("No python-....dol found on SD or USB");
            wait_1s();
            return 2;
        }
    }

    TP("[main] boote dol:");
    TP(dol);
    if (!boot_dol(dol, NULL)) {
        terminal_print("Failed to boot DOL:");
        terminal_print(dol);
        wait_1s();
    }

    return 0;
}