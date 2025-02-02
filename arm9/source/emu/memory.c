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
#include "pia.h"
#include "pokeysnd.h"

wrfunc *writemap = (wrfunc *)0x06860000;                    // Slight speedup to put the write function table in VRAM... uses 256K but we have nothing else to use if for!
rdfunc readmap[65536]   __attribute__ ((aligned (16)));     // Some areas are hotspots for peripheral reads (Antic, GTIA, Pokey, etc).
UBYTE  memory[65536]    __attribute__ ((aligned (16)));     // This is the full 64K of Atari5200 memory map. For normal cart games, this is all there is!

UBYTE  normal_memory[16] __attribute__((section(".dtcm"))); // A quick way to determine if we are in a region of memory that might have special handling (banked, or peripheral mapped)
UBYTE *mem_map[16]       __attribute__((section(".dtcm"))); // This is the magic that allows us to index into banks of memory quickly. 16 banks of 4K cover the entire 5200 memory map.

void ROM_PutByte(UWORD addr, UBYTE value)
{
    // ROM normally doesn't respond to a write...
}

void MEMORY_InitialiseMap(void) 
{
    // Set the memory map back to pointing to main memory
    for (int i=0; i<16; i++)
    {
        mem_map[i] = memory + (0x1000 * i) - (0x1000 * i);  // Yes, pointless except to get across the point that we are offsetting the memory map to avoid having to mask the addr in memory.h
    }
}

void MEMORY_InitialiseMachine(void) 
{
    unsigned int i;

    MEMORY_InitialiseMap();
    memcpy(memory + 0xf800, atari_os, 0x800);
    dFillMem(0x0000, 0x00, 0xf800);
    SetRAM(0x0000, 0x3fff);
    SetROM(0x4000, 0xffff);
    
    // Set entire upper 16K to write nothing... and then override with peripherals below
    for (i=0xc000; i <= 0xffff; i++) writemap[i] = ROM_PutByte;
    
    for (i=0xc000; i <= 0xc0ff; i++) readmap[i] = GTIA_GetByte;
    for (i=0xd400; i <= 0xd4ff; i++) readmap[i] = ANTIC_GetByte;
    for (i=0xe800; i <= 0xefff; i++) readmap[i] = POKEY_GetByte;
       
    for (i=0xc000; i <= 0xc0ff; i++) writemap[i] = GTIA_PutByte;
    for (i=0xd400; i <= 0xd4ff; i++) writemap[i] = ANTIC_PutByte;
    for (i=0xe800; i <= 0xefff; i++) writemap[i] = POKEY_PutByte;
    
    Coldstart();
}
