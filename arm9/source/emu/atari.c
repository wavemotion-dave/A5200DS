/*
 * atari.c - main high-level routines
 *
 * Copyright (c) 1995-1998 David Firth
 * Copyright (c) 1998-2006 Atari800 development team (see DOC/CREDITS)
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
#include "main.h"
#include "antic.h"
#include "atari.h"
#include "cartridge.h"
#include "cpu.h"
#include "gtia.h"
#include "input.h"
#include "memory.h"
#include "pia.h"
#include "platform.h"
#include "pokeysnd.h"


void Atari800_RunEsc(UBYTE esc_code)
{
	Atari800_Exit(TRUE);
}


void Warmstart(void) 
{
    PIA_Reset();
    ANTIC_Reset();
    /* CPU_Reset() must be after PIA_Reset(),
       because Reset routine vector must be read from OS ROM */
    CPU_Reset();
    /* note: POKEY and GTIA have no Reset pin */
}

void Coldstart(void) {
	PIA_Reset();
	ANTIC_Reset();
	/* CPU_Reset() must be after PIA_Reset(),
	   because Reset routine vector must be read from OS ROM */
	CPU_Reset();
	/* note: POKEY and GTIA have no Reset pin */
    
	/* reset cartridge to power-up state */
	CART_Start();

	/* set Atari OS Coldstart flag */
	dPutByte(0x244, 1);

    consol_index = 2;
	consol_table[2] = 0x0f;
    
    /* hold Option during reboot */
    consol_table[2] &= ~CONSOL_OPTION;

	consol_table[1] = consol_table[2];
}

int Atari800_InitialiseMachine(void) 
{
	MEMORY_InitialiseMachine();
	return TRUE;
}

int Atari800_DetectFileType(const char *filename) 
{
    return AFILE_CART;
}

int Atari800_OpenFile(const char *filename, int reboot, int diskno, int readonly) {
  // Remove cart if exist
  CART_Remove();
  
	int type = Atari800_DetectFileType(filename);

	switch (type) 
    {
    case AFILE_CART:
    case AFILE_ROM:
      if (CART_Insert(filename) != 0) {
        return AFILE_ERROR;
      }
      if (reboot)
        Coldstart();
      break;
            
    default:
      break;
	}
	return type;
}

int Atari800_Initialise(void) 
{
	INPUT_Initialise();

	// Platform Specific Initialisation 
	Atari_Initialise();

	// Initialise Custom Chips
	ANTIC_Initialise();
	GTIA_Initialise();
	PIA_Initialise();
	POKEY_Initialise();

    Atari800_InitialiseMachine();

	return TRUE;
}

int Atari800_Exit(int run_monitor) {
	return Atari_Exit(run_monitor);
}

extern int gTotalAtariFrames;
void Atari800_Frame(void)
{
	INPUT_Frame();
	GTIA_Frame();
    ANTIC_Frame((myCart.frame_skip ? (gTotalAtariFrames&3) : TRUE));  // With frameskip, we skip every 8th frame (drawing 7 of 8). Skip is dangerous for the collision detection so we do it sparingly
    POKEY_Frame();
    
    gTotalAtariFrames++;
}
