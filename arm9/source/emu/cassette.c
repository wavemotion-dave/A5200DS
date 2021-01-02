/*
 * cassette.c - cassette emulation
 *
 * Copyright (C) 2001 Piotr Fusik
 * Copyright (C) 2001-2005 Atari800 development team (see DOC/CREDITS)
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
#include <string.h>

#include "atari.h"
#include "cpu.h"
#include "cassette.h"
#include "memory.h"
#include "sio.h"
#include "util.h"

UBYTE cassette_buffer[4096];

int hold_start_on_reboot = 0;
int hold_start = 0;
int press_space = 0;
int eof_of_tape = 0;

void CASSETTE_Initialise(void) 
{
}

int CASSETTE_CheckFile(const char *filename, FILE **fp, char *description, int *last_block, int *isCAS)
{
	return TRUE;
}

int CASSETTE_CreateFile(const char *filename, FILE **fp, int *isCAS)
{
	return TRUE;
}

int CASSETTE_Insert(const char *filename)
{
    return TRUE;
}

void CASSETTE_Remove(void)
{
}

int CASSETTE_AddGap(int gaptime)
{
	return 0;
}

void CASSETTE_LeaderLoad(void)
{
}

void CASSETTE_LeaderSave(void)
{
}

int CASSETTE_Read(void)
{
	return 0;
}

int CASSETTE_Write(int length)
{
    return 0;
}

int SetNextByteTime_POKEY(int adjust)
{
	return 0;
}

int CASSETTE_GetByte(void)
{
	return 0x00;
}

int CASSETTE_IOLineStatus(void)
{
    return 0;
}

int CASSETTE_GetInputIRQDelay(void)
{
    return 0;
}

void CASSETTE_TapeMotor(int onoff)
{
}

void CASSETTE_AddScanLine(void)
{
}
