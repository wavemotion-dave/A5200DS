#ifndef _ATARI_H_
#define _ATARI_H_

#include <stdio.h>
#include "config.h"

/* Fundamental declarations ---------------------------------------------- */

#ifndef FALSE
#define FALSE  0
#endif
#ifndef TRUE
#define TRUE   1
#endif

#define MAX_FILENAME_LEN    256

// Some global sound defines
#define SOUND_FREQ (15720)        // 60 frames per second. 264 scanlines per frame. 1 samples per scanline. 60*264*1 = 15720
#define SNDLENGTH  1024           // Must be power of 2... so we can quickly mask it

/* SBYTE and UBYTE must be exactly 1 byte long. */
/* SWORD and UWORD must be exactly 2 bytes long. */
/* SLONG and ULONG must be exactly 4 bytes long. */
#define SBYTE signed char
#define SWORD signed short
#define SLONG signed int
#define UBYTE unsigned char
#define UWORD unsigned short
#define ULONG unsigned int
/* Note: in various parts of the emulator we assume that char is 1 byte and int is 4 bytes. */

/* Public interface ------------------------------------------------------ */

/* Machine type. */
#define MACHINE_5200  3

/* Video system. */
#define TV_PAL  312
#define TV_NTSC 262

/* Dimensions of atari_screen.
   atari_screen is ATARI_WIDTH * ATARI_HEIGHT bytes.
   Each byte is an Atari color code - use Palette_Get[RGB] functions
   to get actual RGB codes.
   You should never display anything outside the middle 336 columns. */
#define ATARI_WIDTH  384
#define ATARI_HEIGHT 240

/* Special key codes.
   Store in key_code. */
#define AKEY_WARMSTART             -2
#define AKEY_COLDSTART             -3
#define AKEY_EXIT                  -4
#define AKEY_BREAK                 -5
#define AKEY_UI                    -7
#define AKEY_SCREENSHOT            -8
#define AKEY_SCREENSHOT_INTERLACE  -9
#define AKEY_START                 -10
#define AKEY_SELECT                -11
#define AKEY_OPTION                -12


/* File types returned by Atari800_DetectFileType() and Atari800_OpenFile(). */
#define AFILE_ERROR      0
#define AFILE_CART       9
#define AFILE_ROM        10

/* Initializes Atari800 emulation core. */
int Atari800_Initialise(void);

/* Emulates one frame (1/50sec for PAL, 1/60sec for NTSC). */
void Atari800_Frame(void);

/* Reboots the emulated Atari. */
void Coldstart(void);

/* Presses the Reset key in the emulated Atari. */
void Warmstart(void);

/* Reinitializes after machine_type or ram_size change.
   You should call Coldstart() after it. */
int Atari800_InitialiseMachine(void);

/* Auto-detects file type and returns one of AFILE_* values. */
int Atari800_DetectFileType(const char *filename);

/* Auto-detects file type and mounts the file in the emulator.
   reboot: Coldstart() for disks, cartridges and tapes
   diskno: drive number for disks (1-8)
   readonly: mount disks as read-only */
int Atari800_OpenFile(const char *filename, int reboot, int diskno, int readonly);

/* Checks for "popular" filenames of ROM images in the specified directory
   and sets atari_*_filename to the ones found.
   If only_if_not_set is TRUE, then atari_*_filename is modified only when
   Util_filenamenotset() is TRUE for it. */
void Atari800_FindROMImages(const char *directory, int only_if_not_set);

/* Shuts down Atari800 emulation core. */
int Atari800_Exit(int run_monitor);


/* Private interface ----------------------------------------------------- */
/* Don't use outside the emulation core! */


/* Current clock cycle in a scanline.
   Normally 0 <= xpos && xpos < LINE_C, but in some cases xpos >= LINE_C,
   which means that we are already in line (ypos + 1). */
extern int xpos;

/* xpos limit for the currently running 6502 emulation. */
extern int xpos_limit;

/* Number of cycles per scanline. */
#define LINE_C   114

/* STA WSYNC resumes here. */
#define WSYNC_C  106

/* Number of memory refresh cycles per scanline.
   In the first scanline of a font mode there are actually less than DMAR
   memory refresh cycles. */
#define DMAR     9

/* Number of scanlines per frame. */
#define max_ypos TV_NTSC

#define UNALIGNED_STAT_DEF()
#define UNALIGNED_STAT_DECL()
#define UNALIGNED_GET_WORD(ptr)        (*(const UWORD *) (ptr))
#define UNALIGNED_PUT_WORD(ptr, value) (*(UWORD *) (ptr) = (value))
#define UNALIGNED_GET_LONG(ptr)        (*(const ULONG *) (ptr))
#define UNALIGNED_PUT_LONG(ptr, value) (*(ULONG *) (ptr) = (value))

/* Handles an escape sequence. */
void Atari800_RunEsc(UBYTE esc_code);

/* Reads a byte from the specified special address (not RAM or ROM). */
UBYTE Atari800_GetByte(UWORD addr);

/* Stores a byte at the specified special address (not RAM or ROM). */
void Atari800_PutByte(UWORD addr, UBYTE byte);

#endif /* _ATARI_H_ */
