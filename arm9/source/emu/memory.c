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

rdfunc readmap[65536]   __attribute__ ((aligned (16)));
wrfunc writemap[65536]  __attribute__ ((aligned (16)));
UBYTE  memory[65536]    __attribute__ ((aligned (16)));

void ROM_PutByte(UWORD addr, UBYTE value)
{
}

void MEMORY_InitialiseMachine(void) 
{
    unsigned int i;
    memcpy(memory + 0xf800, atari_os, 0x800);
    dFillMem(0x0000, 0x00, 0xf800);
    SetRAM(0x0000, 0x3fff);
    SetROM(0x4000, 0xffff);
    
    for (i=0xc000; i< 0xc0ff; i++) readmap[i] = GTIA_GetByte;
    
    for (i=0xd400; i< 0xd4ff; i++) readmap[i] = ANTIC_GetByte;

    readmap[0xD40B] = ANTIC_Get_VCOUNT;
    readmap[0xD41B] = ANTIC_Get_VCOUNT;
    readmap[0xD42B] = ANTIC_Get_VCOUNT;
    readmap[0xD43B] = ANTIC_Get_VCOUNT;
    readmap[0xD44B] = ANTIC_Get_VCOUNT;
    readmap[0xD45B] = ANTIC_Get_VCOUNT;
    readmap[0xD46B] = ANTIC_Get_VCOUNT;
    readmap[0xD47B] = ANTIC_Get_VCOUNT;
    readmap[0xD48B] = ANTIC_Get_VCOUNT;
    readmap[0xD49B] = ANTIC_Get_VCOUNT;
    readmap[0xD4AB] = ANTIC_Get_VCOUNT;
    readmap[0xD4BB] = ANTIC_Get_VCOUNT;
    readmap[0xD4CB] = ANTIC_Get_VCOUNT;
    readmap[0xD4DB] = ANTIC_Get_VCOUNT;
    readmap[0xD4EB] = ANTIC_Get_VCOUNT;
    readmap[0xD4FB] = ANTIC_Get_VCOUNT;    
    
    readmap[0xD40C] = ANTIC_Get_PENH;
    readmap[0xD41C] = ANTIC_Get_PENH;
    readmap[0xD42C] = ANTIC_Get_PENH;
    readmap[0xD43C] = ANTIC_Get_PENH;
    readmap[0xD44C] = ANTIC_Get_PENH;
    readmap[0xD45C] = ANTIC_Get_PENH;
    readmap[0xD46C] = ANTIC_Get_PENH;
    readmap[0xD47C] = ANTIC_Get_PENH;
    readmap[0xD48C] = ANTIC_Get_PENH;
    readmap[0xD49C] = ANTIC_Get_PENH;
    readmap[0xD4AC] = ANTIC_Get_PENH;
    readmap[0xD4BC] = ANTIC_Get_PENH;
    readmap[0xD4CC] = ANTIC_Get_PENH;
    readmap[0xD4DC] = ANTIC_Get_PENH;
    readmap[0xD4EC] = ANTIC_Get_PENH;
    readmap[0xD4FC] = ANTIC_Get_PENH;
    
    readmap[0xD40D] = ANTIC_Get_PENV;
    readmap[0xD41D] = ANTIC_Get_PENV;
    readmap[0xD42D] = ANTIC_Get_PENV;
    readmap[0xD43D] = ANTIC_Get_PENV;
    readmap[0xD44D] = ANTIC_Get_PENV;
    readmap[0xD45D] = ANTIC_Get_PENV;
    readmap[0xD46D] = ANTIC_Get_PENV;
    readmap[0xD47D] = ANTIC_Get_PENV;
    readmap[0xD48D] = ANTIC_Get_PENV;
    readmap[0xD49D] = ANTIC_Get_PENV;
    readmap[0xD4AD] = ANTIC_Get_PENV;
    readmap[0xD4BD] = ANTIC_Get_PENV;
    readmap[0xD4CD] = ANTIC_Get_PENV;
    readmap[0xD4DD] = ANTIC_Get_PENV;
    readmap[0xD4ED] = ANTIC_Get_PENV;
    readmap[0xD4FD] = ANTIC_Get_PENV;

    readmap[0xD40F] = ANTIC_Get_NMIST;
    readmap[0xD41F] = ANTIC_Get_NMIST;
    readmap[0xD42F] = ANTIC_Get_NMIST;
    readmap[0xD43F] = ANTIC_Get_NMIST;
    readmap[0xD44F] = ANTIC_Get_NMIST;
    readmap[0xD45F] = ANTIC_Get_NMIST;
    readmap[0xD46F] = ANTIC_Get_NMIST;
    readmap[0xD47F] = ANTIC_Get_NMIST;
    readmap[0xD48F] = ANTIC_Get_NMIST;
    readmap[0xD49F] = ANTIC_Get_NMIST;
    readmap[0xD4AF] = ANTIC_Get_NMIST;
    readmap[0xD4BF] = ANTIC_Get_NMIST;
    readmap[0xD4CF] = ANTIC_Get_NMIST;
    readmap[0xD4DF] = ANTIC_Get_NMIST;
    readmap[0xD4EF] = ANTIC_Get_NMIST;
    readmap[0xD4FF] = ANTIC_Get_NMIST;
        
    for (i=0xe800; i< 0xe8ff; i++) readmap[i] = POKEY_GetByte;
    for (i=0xeb00; i< 0xebff; i++) readmap[i] = POKEY_GetByte;
    for (i=0xc000; i< 0xc0ff; i++) writemap[i] = GTIA_PutByte;
    for (i=0xd400; i< 0xd4ff; i++) writemap[i] = ANTIC_PutByte;
    for (i=0xe800; i< 0xe8ff; i++) writemap[i] = POKEY_PutByte;
    for (i=0xeb00; i< 0xebff; i++) writemap[i] = POKEY_PutByte;
    for (i = 0xe900; i < 0xf0FF; i++ ) 
    {
        readmap[i] = POKEY_GetByte;
        writemap[i] = POKEY_PutByte;
    }
	Coldstart();
}

void MemStateSave(UBYTE SaveVerbose)
{
}

void MemStateRead(UBYTE SaveVerbose) 
{
}

#if 0
void inline CopyFromMem(UWORD from, UBYTE *to, int size)
{
    while (--size >= 0) {
		*to++ = memory[from]; //GetByte(from);
		from++;
	}
}

void inline CopyToMem(const UBYTE *from, UWORD to, int size)
{
	while (--size >= 0) {
		//PutByte(to, *from);
		memory[to++] = *from++;
		//to++;
	}
}
#endif

void get_charset(UBYTE *cs)
{
	const UBYTE *p;
    p = memory + 0xf800;
	/* copy font, but change screencode order to ATASCII order */
	memcpy(cs, p + 0x200, 0x100); /* control chars */
	memcpy(cs + 0x100, p, 0x200); /* !"#$..., uppercase letters */
	memcpy(cs + 0x300, p + 0x300, 0x100); /* lowercase letters */
}
