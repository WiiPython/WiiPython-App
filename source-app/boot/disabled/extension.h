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

#ifndef EXTENSION_H
#define EXTENSION_H

#include <stdbool.h>

#include "extension_api.h"

typedef int (*ext_entry_fn)(const ext_api_t *api);

/* Laedt das Modul (DOL-Format) an die feste Code-Basis, nullt dessen BSS und
   reserviert den freien MEM2 oberhalb des Codes als Arena. Kein crt0, kein
   Reset. Gibt false zurueck bei Fehler (Datei/Adresse/Sektionen).           */
bool ext_load(const char *path);

/* Ruft ext_main(api) im geladenen Modul auf. 'user' wird als api->user
   durchgereicht (z.B. ein game_state*). Rueckgabe = Rueckgabe des Moduls,
   oder -1 wenn nichts geladen ist.                                          */
int ext_run(void *user);

/* Gibt Code + alle Modul-Variablen in einem Rutsch frei (Arena2Hi zurueck).
   Danach ist der RAM wieder komplett verfuegbar.                            */
void ext_unload(void);

/* true, solange ein Modul geladen ist. */
bool ext_is_loaded(void);

/* letzte via api->log() gesendete Nachricht des Moduls (leerer String, wenn
   noch keine kam). Bleibt auch nach ext_unload() erhalten -- fuer Anzeige. */
const char *ext_last_message(void);

#endif
