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

#ifndef _LOADER_RELOC_H_
#define _LOADER_RELOC_H_

#include <gctypes.h>

#define ARGS_MAX_LEN 1024
#define BASE_ADDR 0x81330000
#define LD_MIN_ADDR 0x80003400
#define LD_MAX_ADDR (BASE_ADDR - 1 - ARGS_MAX_LEN)
#define LD_MAX_SIZE (LD_MAX_ADDR - LD_MIN_ADDR)
#define LD_ARGS_ADDR (LD_MAX_ADDR + 1)


#define MAX_BLOBS 8
#define BLOB_MINSLACK (512*1024)

#define gprintf(...)

typedef void (*entry_point) (void);

bool loader_reloc (entry_point *ep, const u8 *addr, u32 size, const char *args,
				   u16 arg_len, bool check_overlap);

/* Faehrt alle libogc-Dienste herunter (SYS_ResetSystem + __exception_closeall).
   MUSS aufgerufen werden, SOLANGE die libogc-Datenstrukturen noch gueltig sind,
   d.h. VOR loader_reloc() (die In-Place-Relokation ueberschreibt das
   .data-Segment des laufenden Programms). */
void loader_shutdown_services (void);

/* arena2hi: der VOR loader_reloc() ausgelesene SYS_GetArena2Hi()-Wert. Nach der
   Relokation ist libogcs .data unbrauchbar, daher darf SYS_GetArena2Hi() hier
   nicht mehr aufgerufen werden. */
void loader_exec (entry_point ep, void *arena2hi);

#endif

