/* Copyright (c) 2026 lberwa  All rights reserved.                                                  *
 *                                                                                                  *
 *  \    /\    / | |  |¯¯\ \ / ¯¯|¯¯ |   |  /¯¯\  |\  |                                             *
 *   \  /  \  /  | |  |__/  Y    |   |---| |    | | \ |                                             *
 *    \/    \/   | |  |     |    |   |   |  \__/  |  \|                                             *
 *                                                                                                  *
 * this file is part of WiiPython project, which is licensed under the MIT license.                 *
 * See the LICENSE file in the project root for full license information.                           *
 * If you don't have a copy of the license, you can get one at https://opensource.org/licenses/MIT. */

// Gemeinsame Quelle fuer beide Builds (libogc1 -> python-libogc.dol,
// libogc2 -> python-libogc2.dol). Die zum Build passende symbols.map wird
// ueber das Makro WIIPY_SYMBOLS_MAP aus dem Makefile hereingereicht.

#include <Python.h>

#include <stdio.h>

#include <bitmap/my_text_renderer.h>   // video_init_custom(), terminal_print()

#ifndef WIIPY_SYMBOLS_MAP
#define WIIPY_SYMBOLS_MAP "python/symbols1.map"
#endif

int main(void) {
    //////////////
    // Init
    //////////////
    video_init_custom();

    terminal_print("1");

    size_t count = 1;

    PyStatus status = Py_Init_Custom((const char*[]){"python"}, &count,
                                     WIIPY_SYMBOLS_MAP);

    if (status._type != _PyStatus_TYPE_OK) {
        terminal_print("Init failed in:");
        terminal_print(status.func);
        terminal_print(status.err_msg);
        return status.exitcode;
    }

    //////////////
    // Run
    //////////////
    int rc =
    PyRun_SimpleString("import sys\n"
                       "import os\n"
                       "device = sys.prefix.split(':', 1)[0]\n" // sd or usb ?
                       "import runpy\n"
                       "os.chdir(f'{device}:/python/')\n"
                       "sys.argv = ['main.py', f'{device}:/python/']\n" // fat init resets the os.chdir
                       "runpy.run_path(f'{device}:/python/main.py')\n"
                       );

    //////////////
    // Return
    //////////////
    Py_Finalize();

    if (rc != 0) {
        char rcbuf[32];
        snprintf(rcbuf, sizeof(rcbuf), "%d", rc);
        terminal_print("Python script failed with return code:");
        terminal_print(rcbuf);
    }
    return rc;
}
