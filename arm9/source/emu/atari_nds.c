/* $Id: atari_vga.c,v 1.1 2001/10/26 05:36:48 fox Exp $ */
/* -------------------------------------------------------------------------- */

/*
 * DJGPP - VGA Backend for David Firth's Atari 800 emulator
 *
 * by Ivo van Poorten (C)1996  See file COPYRIGHT for policy.
 *
 */

/* -------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "main.h"

#include "config.h"
#include "cpu.h"
#include "input.h"
#include "pokeysnd.h"

#define DSPRATE 11025

u16 trig0 = 1;
u16 trig1 = 1;
u16 stick0 = STICK_CENTRE;
u16 stick1 = STICK_CENTRE;

#define first_lno 20 /* center 320x200 window in 384x240 screen */
#define first_col 32

/* -------------------------------------------------------------------------- */
/* CONFIG & INITIALISATION                                                    */
/* -------------------------------------------------------------------------- */
void Atari_Initialise(void)
{
    Pokey_sound_init(FREQ_17_APPROX, DSPRATE, 1, 0); //SND_BIT16);
    
	trig0 = 1;  // Not pressed
    trig1 = 1;  // Not pressed
	stick0 = STICK_CENTRE;
    stick1 = STICK_CENTRE;
    
	key_consol = CONSOL_NONE;
}

/* -------------------------------------------------------------------------- */
/* ATARI EXIT                                                                 */
/* -------------------------------------------------------------------------- */
int Atari_Exit(int run_monitor) 
{
  return 0;
}

