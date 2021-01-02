/*
 * sio.c - Serial I/O emulation
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "antic.h"  /* ypos */
#include "atari.h"
#include "binload.h"
#include "cassette.h"
#include "compfile.h"
#include "cpu.h"
//#include "log.h"
#include "memory.h"
#include "platform.h"
#include "pokeysnd.h"
//#include "pokeysnd.h"
#include "sio.h"
#include "util.h"
#ifndef BASIC
#include "statesav.h"
#endif

/* If ATR image is in double density (256 bytes per sector),
   then the boot sectors (sectors 1-3) can be:
   - logical (as seen by Atari) - 128 bytes in each sector
   - physical (as stored on the disk) - 256 bytes in each sector.
     Only the first half of sector is used for storing data, the rest is zero.
   - SIO2PC (the type used by the SIO2PC program) - 3 * 128 bytes for data
     of boot sectors, then 3 * 128 unused bytes (zero)
   The XFD images in double density have either logical or physical
   boot sectors. */
#define BOOT_SECTORS_LOGICAL	0
#define BOOT_SECTORS_PHYSICAL	1
#define BOOT_SECTORS_SIO2PC		2

UnitStatus drive_status[MAX_DRIVES];
int sio_last_op;
int sio_last_op_time = 0;
int sio_last_drive;
int sio_last_sector;
char sio_status[256];

int ignore_header_writeprotect = FALSE;

void SIO_Initialise(void) 
{
    for (int i=0; i<MAX_DRIVES; i++)
    {
        sio_status[i] = 0x00;
        drive_status[i] = Off;
    }
}

void SIO_Exit(void) 
{
}

int SIO_Mount(int diskno, const char *filename, int b_open_readonly) 
{
    return FALSE;
}

void SIO_Dismount(int diskno)
{
}

void SIO_DisableDrive(int diskno)
{
}

UBYTE SIO_ChkSum(const UBYTE *buffer, int length)
{
    return 0x00;
}

/* Enable/disable the Tape Motor */
void SIO_TapeMotor(int onoff)
{
}

/* Enable/disable the command frame */
void SwitchCommandFrame(int onoff)
{
}

void SIOStateSave(void)
{
}

void SIOStateRead(void)
{
}

