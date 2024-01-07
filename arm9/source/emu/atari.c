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
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# elif defined(HAVE_TIME_H)
#  include <time.h>
# endif
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef WIN32
#include <windows.h>
#endif
#ifdef __EMX__
#define INCL_DOS
#include <os2.h>
#endif
#ifdef __BEOS__
#include <OS.h>
#endif
#ifdef HAVE_LIBZ
#include <zlib.h>
#endif

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
#if !defined(BASIC) && !defined(CURSES_BASIC)
#endif
#ifndef BASIC
#ifndef __PLUS
#endif
#endif /* BASIC */
#if defined(SOUND) && !defined(__PLUS)
#include "pokeysnd.h"
#endif

#ifdef __PLUS
#ifdef _WX_
#include "export.h"
#else /* _WX_ */
#include "globals.h"
#include "macros.h"
#include "display_win.h"
#include "misc_win.h"
#include "registry.h"
#include "timing.h"
#include "FileService.h"
#include "Helpers.h"
#endif /* _WX_ */
#endif /* __PLUS */


int tv_mode = TV_NTSC;
int disable_basic = TRUE;

int verbose = FALSE;

void Atari800_RunEsc(UBYTE esc_code)
{
	cim_encountered = 1;
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
	/* handle Option key (disable BASIC in XL/XE)
	   and Start key (boot from cassette) */
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

int Atari800_Initialise(void) {

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
	int restart;
	restart = Atari_Exit(run_monitor);
	return restart;
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


void MainStateSave(void) 
{
}

void MainStateRead(void) 
{
}
