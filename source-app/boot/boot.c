/* Copyright (c) 2026 lberwa  All rights reserved.                                                  *
 *                                                                                                  *
 *  \    /\    / | |  |¯¯\ \ / ¯¯|¯¯ |   |  /¯¯\  |\  |                                             *
 *   \  /  \  /  | |  |__/  Y    |   |---| |    | | \ |                                             *
 *    \/    \/   | |  |     |    |   |   |  \__/  |  \|                                             *
 *                                                                                                  *
 * this file is part of WiiPython project, which is licensed under the MIT license.                 *
 * See the LICENSE file in the project root for full license information.                           *
 * If you don't have a copy of the license, you can get one at https://opensource.org/licenses/MIT. */

 /*
  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any
  damages arising from the use of this software.

  Permission is granted to anyone to use this software for any
  purpose, including commercial applications, and to alter it and
  redistribute it freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you
     must not claim that you wrote the original software. If you use
     this software in a product, an acknowledgment in the product
     documentation would be appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and
     must not be misrepresented as being the original software.
  3. This notice may not be removed or altered from any source
     distribution.

Portions of this library, notably the threading code, were inspired by and
derived from code written for the RTEMS project (https://www.rtems.org/) by
On-Line Applications Research Corporation.

  COPYRIGHT (c) 1989-1999 On-Line Applications Research Corporation (OAR).
  Copyright (C) 1999  Eric Valette (eric.valette@free.fr)


  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.*/
 
#include "boot.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>


#include <ogc/system.h>
#include <ogc/machine/processor.h>
#include <ogc/ios.h>

#include "panic.h"
#include "loader_reloc.h"

typedef struct {
	void *the_blob;
	size_t blob_size;
	void *old_arena2hi;
} blob_t;

static blob_t blobs[MAX_BLOBS];
static int num_blobs = 0;

// supports only stack-type allocs (free last alloced)
static void *blob_alloc(size_t size)
{
	u32 level;
	u32 addr;
	void *old_arena2hi;

	_CPU_ISR_Disable(level);
	if (num_blobs >= MAX_BLOBS) {
		_CPU_ISR_Restore(level);
		gprintf("too many blobs\n");
		panic();
	}

	old_arena2hi = SYS_GetArena2Hi();
	addr = (((u32)old_arena2hi) - size) & (~0x1f);

	if (addr < (BLOB_MINSLACK + (u32)SYS_GetArena2Lo())) {
		_CPU_ISR_Restore(level);
		return NULL;
	}

	blobs[num_blobs].old_arena2hi = old_arena2hi;
	blobs[num_blobs].the_blob = (void*)addr;
	blobs[num_blobs].blob_size = size;
	num_blobs++;

	SYS_SetArena2Hi((void*)addr);
	_CPU_ISR_Restore(level);
	gprintf("allocated blob size %d at 0x%08x\n", size, addr);
	return (void*)addr;
}

static void blob_free(void *p)
{
	u32 level;
	if (!p)
		return;

	_CPU_ISR_Disable(level);

	if (num_blobs == 0) {
		_CPU_ISR_Restore(level);
		gprintf("blob_free with no blobs\n");
		panic();
	}

	num_blobs--;
	if (p != blobs[num_blobs].the_blob) {
		_CPU_ISR_Restore(level);
		gprintf("mismatched blob_free (%p != %p)\n", p, blobs[num_blobs].the_blob);
		panic();
	}
	if (SYS_GetArena2Hi() != p) {
		_CPU_ISR_Restore(level);
		gprintf("someone else used MEM2 (%p != %p)\n", p, SYS_GetArena2Hi());
		panic();
	}

	SYS_SetArena2Hi(blobs[num_blobs].old_arena2hi);
	_CPU_ISR_Restore(level);
	gprintf("freed blob size %d at %p\n", blobs[num_blobs].blob_size, p);
}

extern int stat (const char *__restrict __file,
		 struct stat *__restrict __buf) __THROW __nonnull ((1, 2));


static bool launch_external_dol(entry_point *ep, void **arena2hi_out,
								const char *path, const char *args)
{
	struct stat st;
	int fd;
	u8 *data;
	u16 arg_len;
	bool ok;

	if (stat(path, &st) < 0 || st.st_size <= 0 || st.st_size > LD_MAX_SIZE) {
		return false;
	}

	fd = open(path, O_RDONLY);
	if (fd < 0) {
		return false;
	}

	data = (u8 *) blob_alloc(st.st_size);
	if (!data) {
		close(fd);
		return false;
	}

	if (read(fd, data, st.st_size) != st.st_size) {
		close(fd);
		blob_free(data);
		return false;
	}

	close(fd);

	arg_len = 0;
	if (args)
		arg_len = strlen(args) + 1;

	/* Reihenfolge ist kritisch: die Dienste MUESSEN heruntergefahren werden,
	   solange die libogc-Datenstrukturen (u.a. die Reset-Funktions-Queue) noch
	   gueltig sind. loader_reloc() kopiert das neue Image IN-PLACE an seine
	   finalen Adressen und ueberschreibt dabei das .data-Segment des laufenden
	   Programms -- danach ist libogc unbrauchbar. arena2hi wird deshalb JETZT
	   ausgelesen und an loader_exec() weitergereicht.

	   Der Blob wird bewusst nicht mehr freigegeben: nach erfolgreicher
	   Relokation kehren wir nie zurueck, und blob_free() wuerde ohnehin auf
	   libogc (SYS_GetArena2Hi) zugreifen, dessen Zustand dann bereits zerstoert
	   ist. */

	/* IOS frisch neu laden -- genau das macht der Homebrew Channel vor dem
	   Start einer DOL (__IOS_LaunchNewIOS). Ohne diesen Reload erbt die geladene
	   DOL die bereits benutzte IOS-Umgebung (offene FAT-/SD-Handles aus
	   fatInitDefault) und ihre fruehe Init crasht, bevor sie etwas ausgeben kann.
	   MUSS passieren, solange libogc/IPC noch leben, also VOR dem Reloc. Die
	   DOL-Datei liegt zu diesem Zeitpunkt bereits vollstaendig im Blob (MEM2),
	   die SD wird danach nicht mehr gebraucht. IOS_GetVersion() -> dieselbe IOS
	   neu laden, keine Versionsannahme. */
	IOS_ReloadIOS(IOS_GetVersion());

	*arena2hi_out = SYS_GetArena2Hi();

	loader_shutdown_services();

	ok = loader_reloc(ep, data, st.st_size, args, arg_len, true);
	if (!ok) {
		/* Fehlschlag erst nach dem Shutdown -> es gibt keinen Weg zurueck in
		   das (halb tote) Spiel. Deterministisch abbrechen statt undefiniert
		   weiterzulaufen. */
		gprintf("loader_reloc failed after service shutdown\n");
		panic();
	}

	return ok;
}

bool boot_dol(const char *path, const char *args) {
    entry_point ep;
    void *arena2hi;
    bool ok;

    ok = launch_external_dol(&ep, &arena2hi, path, args);
    if (!ok)
        return false;

    loader_exec(ep, arena2hi);

    return true;
}
