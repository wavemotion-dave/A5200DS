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
//#include "log.h"
#include "memory.h"
#include "sio.h"
#include "util.h"

#define MAX_BLOCKS 2048
UBYTE cassette_buffer[4096];

int hold_start_on_reboot = 0;
int hold_start = 0;
int press_space = 0;
int eof_of_tape = 0;

typedef struct {
	char identifier[4];
	UBYTE length_lo;
	UBYTE length_hi;
	UBYTE aux_lo;
	UBYTE aux_hi;
} CAS_Header;
/*
Just for remembering - CAS format in short:
It consists of chunks. Each chunk has a header, possibly followed by data.
If a header is unknown or unexpected it may be skipped by the length of the
header (8 bytes), and additionally the length given in the length-field(s).
There are (until now) 3 types of chunks:
-CAS file marker, has to be at begin of file - identifier "FUJI", length is
number of characters of an (optional) ascii-name (without trailing 0), aux
is always 0.
-baud rate selector - identifier "baud", length always 0, aux is rate in baud
(usually 600; one byte is 8 bits + startbit + stopbit, makes 60 bytes per
second).
-data record - identifier "data", length is length of the data block (usually
$84 as used by the OS), aux is length of mark tone (including leader and gaps)
just before the record data in milliseconds.
*/

void CASSETTE_Initialise(void) 
{
}

/* Gets information about the cassette image. Returns TRUE if ok.
   To get information without attaching file, use:
   char description[CASSETTE_DESCRIPTION_MAX];
   int last_block;
   int isCAS;
   CASSETTE_CheckFile(filename, NULL, description, &last_block, &isCAS);
*/
int CASSETTE_CheckFile(const char *filename, FILE **fp, char *description, int *last_block, int *isCAS)
{
	return TRUE;
}

/* Create CAS file header for saving data to tape */
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

/* Read a record by SIO-patch
   returns block length (with checksum) */
int CASSETTE_AddGap(int gaptime)
{
	return 0;
}

/* Indicates that a loading leader is expected by the OS */
void CASSETTE_LeaderLoad(void)
{
}

/* indicates that a save leader is written by the OS */
void CASSETTE_LeaderSave(void)
{
}

/* Read Cassette Record from Storage medium
  returns size of data in buffer (atm equal with record size, but there
    are protections with variable baud rates imaginable where a record
    must be split and a baud chunk inserted inbetween) or -1 for error */
int CASSETTE_Read(void)
{
	return 0;
}

/* Write Cassette Record to Storage medium
  length is size of the whole data with checksum(s)
  returns really written bytes, -1 for error */
int CASSETTE_Write(int length)
{
    return 0;
}

/* sets the stamp of next irq and loads new record if necessary
   adjust is a value to correction of time of next irq*/
int SetNextByteTime_POKEY(int adjust)
{
	return 0;
}

/* Get the byte for which the serial data ready int has been triggered */
int CASSETTE_GetByte(void)
{
	/* there are programs which load 2 blocks as one */
	return 0x00;
}

/* Check status of I/O-line
  Mark tone and stop bit gives 1, start bit 0
  $55 as data give 1,0,1,0,1,0,1,0 (sync to evaluate tape speed,
    least significant bit first)
  returns state of I/O-line as block.byte.bit */
int CASSETTE_IOLineStatus(void)
{
    return 0;
}

/* Get the delay to trigger the next interrupt
   remark: The I/O-Line-status is also evaluated on this basis */
int CASSETTE_GetInputIRQDelay(void)
{
    return 0;
}

/* set motor status
 1 - on, 0 - off
 remark: the real evaluation is done in AddScanLine*/
void CASSETTE_TapeMotor(int onoff)
{
}

void CASSETTE_AddScanLine(void)
{
}
