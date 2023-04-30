/*
 * pia.c - PIA chip emulation
 *
 * Copyright (C) 1995-1998 David Firth
 * Copyright (C) 1998-2005 Atari800 development team (see DOC/CREDITS)
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

#include "atari.h"
#include "cpu.h"
#include "memory.h"
#include "pia.h"

#ifndef BASIC
#include "input.h"
#endif

UBYTE PACTL;
UBYTE PBCTL;
UBYTE PORTA;
UBYTE PORTB;
UBYTE PORT_input[2];

int xe_bank = 0;
int selftest_enabled = 0;

UBYTE atari_basic[8192];
UBYTE atari_os[16384];

UBYTE PORTA_mask;
UBYTE PORTB_mask;

void PIA_Initialise(void) {
    PACTL = 0x3f;
    PBCTL = 0x3f;
    PORTA = 0xff;
    PORTB = 0xff;
    PORTA_mask = 0xff;
    PORTB_mask = 0xff;
    PORT_input[0] = 0xff;
    PORT_input[1] = 0xff;
}

void PIA_Reset(void) {
    PORTA = 0xff;
    PORTB = 0xff;
}

void PIAStateSave(void) 
{
}

void PIAStateRead(void) 
{
}

