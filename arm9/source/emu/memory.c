/*
 * memory.c - memory emulation
 *
 * Copyright (C) 1995-1998 David Firth
 * Copyright (C) 1998-2006 Atari800 development team (see DOC/CREDITS)
 *
 * This file is part of the Atari800 emulator project which emulates
 * the Atari 400, 800, 800XL, 130XE, and 5200 8-bit computers.
 *
 * Atari800 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Atari800 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Atari800; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "atari.h"
#include "antic.h"
#include "cpu.h"
#include "cartridge.h"
#include "gtia.h"
#include "memory.h"
#include "pbi.h"
#include "pia.h"
#include "pokeysnd.h"
#include "util.h"
#ifndef BASIC
#include "statesav.h"
#endif

UBYTE memory[65536 + 2] __attribute__ ((aligned (4)));

#ifndef PAGED_ATTRIB

UBYTE attrib[65536];

#else /* PAGED_ATTRIB */

rdfunc readmap[256];
wrfunc writemap[256];

typedef struct map_save {
	int     code;
	rdfunc  rdptr;
	wrfunc  wrptr;
} map_save;

void ROM_PutByte(UWORD addr, UBYTE value)
{
}

#endif /* PAGED_ATTRIB */

int have_basic = FALSE; /* Atari BASIC image has been successfully read (Atari 800 only) */

void MEMORY_InitialiseMachine(void) 
{
    memcpy(memory + 0xf800, atari_os, 0x800);
    dFillMem(0x0000, 0x00, 0xf800);
    SetRAM(0x0000, 0x3fff);
    SetROM(0x4000, 0xffff);
#ifndef PAGED_ATTRIB
    SetHARDWARE(0xc000, 0xc0ff);	/* 5200 GTIA Chip */
    SetHARDWARE(0xd400, 0xd4ff);	/* 5200 ANTIC Chip */
    SetHARDWARE(0xe800, 0xe8ff);	/* 5200 POKEY Chip */
    SetHARDWARE(0xeb00, 0xebff);	/* 5200 POKEY Chip */
#else
    readmap[0xc0] = GTIA_GetByte;
    readmap[0xd4] = ANTIC_GetByte;
    readmap[0xe8] = POKEY_GetByte;
    readmap[0xeb] = POKEY_GetByte;
    writemap[0xc0] = GTIA_PutByte;
    writemap[0xd4] = ANTIC_PutByte;
    writemap[0xe8] = POKEY_PutByte;
    writemap[0xeb] = POKEY_PutByte;
     for (unsigned int i = 0xe9; i < 0xf0; i++ ) 
     {
        readmap[i] = POKEY_GetByte;
        writemap[i] = POKEY_PutByte;
    }
#endif
	Coldstart();
}

void MemStateSave(UBYTE SaveVerbose)
{
}

void MemStateRead(UBYTE SaveVerbose) 
{
}

void CopyFromMem(UWORD from, UBYTE *to, int size)
{
	while (--size >= 0) {
		*to++ = GetByte(from);
		from++;
	}
}

void CopyToMem(const UBYTE *from, UWORD to, int size)
{
	while (--size >= 0) {
		PutByte(to, *from);
		from++;
		to++;
	}
}

/*
 * Returns non-zero, if Atari BASIC is disabled by given PORTB output.
 * Normally BASIC is disabled by setting bit 1, but it's also disabled
 * when using 576K and 1088K memory expansions, where bit 1 is used
 * for selecting extended memory bank number.
 */
static int basic_disabled(UBYTE portb)
{
	return (portb & 0x02) != 0
	 || ((portb & 0x10) == 0 && (ram_size == 576 || ram_size == 1088));
}


static int cart809F_enabled = FALSE;
int cartA0BF_enabled = FALSE;
static UBYTE under_cart809F[8192];
static UBYTE under_cartA0BF[8192];

void Cart809F_Disable(void)
{
	if (cart809F_enabled) {
		if (ram_size > 32) {
			memcpy(memory + 0x8000, under_cart809F, 0x2000);
			SetRAM(0x8000, 0x9fff);
		}
		else
			dFillMem(0x8000, 0xff, 0x2000);
		cart809F_enabled = FALSE;
	}
}

void Cart809F_Enable(void)
{
	if (!cart809F_enabled) {
		if (ram_size > 32) {
			memcpy(under_cart809F, memory + 0x8000, 0x2000);
			SetROM(0x8000, 0x9fff);
		}
		cart809F_enabled = TRUE;
	}
}

void CartA0BF_Disable(void)
{
	if (cartA0BF_enabled) {
		/* No BASIC if not XL/XE or bit 1 of PORTB set */
		/* or accessing extended 576K or 1088K memory */
		if ((machine_type != MACHINE_XLXE) || basic_disabled((UBYTE) (PORTB | PORTB_mask))) {
			if (ram_size > 40) {
				memcpy(memory + 0xa000, under_cartA0BF, 0x2000);
				SetRAM(0xa000, 0xbfff);
			}
			else
				dFillMem(0xa000, 0xff, 0x2000);
		}
		else
			memcpy(memory + 0xa000, atari_basic, 0x2000);
		cartA0BF_enabled = FALSE;
		if (machine_type == MACHINE_XLXE) {
			TRIG[3] = 0;
			if (GRACTL & 4)
				TRIG_latch[3] = 0;
		}
	}
}

void CartA0BF_Enable(void)
{
	if (!cartA0BF_enabled) {
		/* No BASIC if not XL/XE or bit 1 of PORTB set */
		/* or accessing extended 576K or 1088K memory */
		if (ram_size > 40 && ((machine_type != MACHINE_XLXE) || (PORTB & 0x02)
		|| ((PORTB & 0x10) == 0 && (ram_size == 576 || ram_size == 1088)))) {
			/* Back-up 0xa000-0xbfff RAM */
			memcpy(under_cartA0BF, memory + 0xa000, 0x2000);
			SetROM(0xa000, 0xbfff);
		}
		cartA0BF_enabled = TRUE;
		if (machine_type == MACHINE_XLXE)
			TRIG[3] = 1;
	}
}

void get_charset(UBYTE *cs)
{
	const UBYTE *p;
    p = memory + 0xf800;
	/* copy font, but change screencode order to ATASCII order */
	memcpy(cs, p + 0x200, 0x100); /* control chars */
	memcpy(cs + 0x100, p, 0x200); /* !"#$..., uppercase letters */
	memcpy(cs + 0x300, p + 0x300, 0x100); /* lowercase letters */
}
