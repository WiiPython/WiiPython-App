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

#include <string.h>
#include <ogc/machine/processor.h>

#include "panic.h"

// steal from libogc
extern unsigned char console_font_8x16[];

#define BLACK 0x00800080
#define RED 0x4C544CFF

static vu32 *fb;

static void pchar(u32 cx, u32 cy, char c)
{
	int x,y;
	unsigned char *p = &console_font_8x16[16*c];
	
	for (y=0; y<16; y++) {
		char v = *p++;
		for (x=0; x<4; x++) {
			u32 c = RED;
			switch (v&0xc0) {
			case 0x00:
				c = BLACK;
				break;
			case 0x40:
				c = (RED & 0x0000FF00) | (BLACK & 0xFFFF00FF);
				break;
			case 0x80:
				c = (RED & 0xFFFF00FF) | (BLACK & 0x0000FF00);
				break;
			case 0xc0:
				c = RED;
				break;
			}
			fb[320*(cy+y)+cx+x] = c;
			v<<=2;
		}
	}
}

static void putsc(u32 y, char *s) {
	u32 x = (320-strlen(s)*4)/2;
	while(*s) {
		pchar(x, y, *s++);
		x += 4;
	}
}

static void hex(u32 v, char *p) {
	int i;
	for (i=0; i<8; i++) {
		if ((v>>28) >= 10)
			*p++ = 'A' + (v>>28) - 10;
		else
			*p++ = '0' + (v>>28);
		v <<= 4;
	}
}

#define		HW_REG_BASE		0xd800000
#define		HW_RESETS		(HW_REG_BASE + 0x194)

// written to be as generic and reliable as possible
void _panic(u32 file, u32 line)
{
	u32 level;
	u32 fbr;
	int count = 30;
	int x,y;
	u32 bcolor = BLACK;
	u16 vtr;
	int lines;
	char guru[] = "Guru Meditation #00000000.00000000";
	_CPU_ISR_Disable(level);

	fbr = read32(0xc00201c) & 0x1fffffff;
	if (fbr&0x10000000)
		fbr <<= 5;

	fb = (vu32*)(fbr | 0xC0000000);

	vtr = read16(0xc002000);
	if ((vtr & 0xf) > 10)
		lines = vtr >> 4;
	else
		lines = 2 * (vtr >> 4);

	for(y=(lines-1); y>=116; y--)
		for(x=0; x<320; x++)
			fb[y*320+x] = fb[(y-116)*320+x];
	for(y=0; y<116; y++)
		for(x=0; x<320; x++)
			fb[y*320+x] = BLACK;

	hex(file, &guru[17]);
	hex(line, &guru[26]);
		
	putsc(42, "Software Failure.   Press reset button to continue.");
	putsc(62, guru);
		
	// wait for reset button
	while(read32(0xcc003000)&0x10000) {
		// blink
		if (count >= 25) {
			if (bcolor == RED)
				bcolor = BLACK;
			else
				bcolor = RED;

			for(y=28; y<34; y++)
				for(x=14; x<306; x++)
					fb[y*320+x] = bcolor;
			for(y=34; y<84; y++) {
				for(x=14; x<17; x++)
					fb[y*320+x] = bcolor;
				for(x=303; x<306; x++)
					fb[y*320+x] = bcolor;
			}
			for(y=84; y<90; y++)
				for(x=14; x<306; x++)
					fb[y*320+x] = bcolor;

			count = 0;
		}
		
		// hacky waitvsync
		while(read16(0xc00202c) >= 200);
		while(read16(0xc00202c) <  200);
		count++;
	}

	// needs AHBPROT, is evil. so sue me.
	write32(HW_RESETS, read32(HW_RESETS)&(~1));
	while(1); 
}
