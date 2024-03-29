// =====================================================================================
// Copyright (c) 2021-2023 Dave Bernazzani (wavemotion-dave)
//
// Copying and distribution of this emulator, its source code and associated 
// readme files, with or without modification, are permitted in any medium without 
// royalty provided this copyright notice is used and both alekmaul and wavemotion-dave 
// are thanked profusely.
//
// The a5200ds emulator is offered as-is, without any warranty.
//
// Please see the README.md file as it contains much useful info.
// =====================================================================================
#include <nds.h>
#include <nds/fifomessages.h>

#include <stdio.h>
#include <fat.h>
#include <dirent.h>
#include <unistd.h>

#include "main.h"
#include "a5200utils.h"

#include "atari.h"
#include "cartridge.h"
#include "highscore.h"
#include "input.h"
#include "emu/pia.h"

#include "clickNoQuit_wav.h"
#include "bgBottom.h"
#include "bgStarRaiders.h"
#include "bgTop.h"
#include "bgFileSel.h"
#include "printf.h"

#include "altirra_5200_os.h"

FICA5200 a5200romlist[1024];  
unsigned int counta5200=0, countfiles=0, ucFicAct=0;
int gTotalAtariFrames = 0;
int bg0, bg1, bg0b, bg1b, bg2, bg3;
unsigned short int etatEmu;
int atari_frames=0;        
u16 bSoundMute = false;

char padKey[]   = {AKEY_5200_0,AKEY_5200_1,AKEY_5200_2,AKEY_5200_3,AKEY_5200_4,AKEY_5200_5,AKEY_5200_6,AKEY_5200_7,AKEY_5200_8,AKEY_5200_9,AKEY_5200_HASH,AKEY_5200_ASTERISK};
char padKeySR[] = {AKEY_5200_1,AKEY_5200_2,AKEY_5200_3,AKEY_5200_4,AKEY_5200_5,AKEY_5200_6,AKEY_5200_7,AKEY_5200_8,AKEY_5200_9,AKEY_5200_ASTERISK,AKEY_5200_0,AKEY_5200_HASH};

#define  cxBG (myCart.offset_x<<8)
#define  cyBG (myCart.offset_y<<8)
#define  xdxBG (((320 / myCart.scale_x) << 8) | (320 % myCart.scale_x))
#define  ydyBG (((256 / myCart.scale_y) << 8) | (256 % myCart.scale_y))
  
unsigned char sound_buffer[16] __attribute__ ((aligned (4))) = {0};
u16* aptr __attribute__((section(".dtcm"))) = (u16*) ((u32)&sound_buffer[0] + 0xA000000); 
u16* bptr __attribute__((section(".dtcm"))) = (u16*) ((u32)&sound_buffer[2] + 0xA000000);

unsigned int atari_pal16[256] = {0};

static int last_key_code = 0x00;
static UWORD keys_dampen = 0;
char bStarRaiders=0;
char lcd_swap_counter = 0;

u16 sound_idx           __attribute__((section(".dtcm"))) = 0;
u16 myPokeyBufIdx       __attribute__((section(".dtcm"))) = 0;
u8  lastSample          __attribute__((section(".dtcm"))) = 0;
u16 sampleExtender[256] __attribute__((section(".dtcm"))) = {0};

#define MAX_DEBUG 16
u32 debug[MAX_DEBUG]={0};
char DEBUG_DUMP = 0;

static void DumpDebugData(void)
{
    if (DEBUG_DUMP)
    {
        static char dbgbuf[32];
        sprintf(dbgbuf, "Cart.offset_x:   %03d", myCart.offset_x);      dsPrintValue(1,2,0, dbgbuf);
        sprintf(dbgbuf, "Cart.offset_y:   %03d", myCart.offset_y);      dsPrintValue(1,3,0, dbgbuf);
        sprintf(dbgbuf, "Cart.scale_x:    %03d", myCart.scale_x);       dsPrintValue(1,4,0, dbgbuf);
        sprintf(dbgbuf, "Cart.scale_y:    %03d", myCart.scale_y);       dsPrintValue(1,5,0, dbgbuf);

        for (int i=0; i<MAX_DEBUG; i++)
        {
            sprintf(dbgbuf, "%02d: %-9d %08X", i, debug[i], debug[i]);
            dsPrintValue(0,7+i,0, dbgbuf);
        }
    }
}

void VsoundClear(void)
{
    extern void PokeyClearBuffer(void);
    
    PokeyClearBuffer();   
    memset(sound_buffer, 0x00, 16);
    lastSample = 0x00;
    myPokeyBufIdx = 0;
    sound_idx = 0;
}

ITCM_CODE void VsoundHandler(void) 
{
  extern unsigned char pokey_buffer[];
  extern u16 pokeyBufIdx;
  
  if (bSoundMute) {*bptr = *aptr; return;}
  
  // If there is a fresh sample... 
  if (myPokeyBufIdx != pokeyBufIdx)
  {
      *aptr = sampleExtender[pokey_buffer[myPokeyBufIdx]];
      myPokeyBufIdx = (myPokeyBufIdx+1) & (SNDLENGTH-1);
      if (myPokeyBufIdx != pokeyBufIdx)
      {
          *bptr = sampleExtender[pokey_buffer[myPokeyBufIdx]];
          myPokeyBufIdx = (myPokeyBufIdx+1) & (SNDLENGTH-1);
      } else *bptr = *aptr;
  }
}

void restore_bottom_screen(void)
{
    if (bStarRaiders)
    {
      decompress(bgStarRaidersTiles, bgGetGfxPtr(bg0b), LZ77Vram);
      decompress(bgStarRaidersMap, (void*) bgGetMapPtr(bg0b), LZ77Vram);
      dmaCopy((void *) bgStarRaidersPal,(u16*) BG_PALETTE_SUB,256*2);
      unsigned short dmaVal = *(bgGetMapPtr(bg1b) +31*32);
      dmaFillWords(dmaVal | (dmaVal<<16),(void*) bgGetMapPtr(bg1b),32*24*2);
    }
    else
    {
      decompress(bgBottomTiles, bgGetGfxPtr(bg0b), LZ77Vram);
      decompress(bgBottomMap, (void*) bgGetMapPtr(bg0b), LZ77Vram);
      dmaCopy((void *) bgBottomPal,(u16*) BG_PALETTE_SUB,256*2);
      unsigned short dmaVal = *(bgGetMapPtr(bg1b) +31*32);
      dmaFillWords(dmaVal | (dmaVal<<16),(void*) bgGetMapPtr(bg1b),32*24*2);
    }
}

// Color fading effect
void FadeToColor(unsigned char ucSens, unsigned short ucBG, unsigned char ucScr, unsigned char valEnd, unsigned char uWait) {
  unsigned short ucFade;
  unsigned char ucBcl;

  // Fade-out vers le noir
  if (ucScr & 0x01) REG_BLDCNT=ucBG;
  if (ucScr & 0x02) REG_BLDCNT_SUB=ucBG;
  if (ucSens == 1) {
    for(ucFade=0;ucFade<valEnd;ucFade++) {
      if (ucScr & 0x01) REG_BLDY=ucFade;
      if (ucScr & 0x02) REG_BLDY_SUB=ucFade;
      for (ucBcl=0;ucBcl<uWait;ucBcl++) {
        swiWaitForVBlank();
      }
    }
  }
  else {
    for(ucFade=16;ucFade>valEnd;ucFade--) {
      if (ucScr & 0x01) REG_BLDY=ucFade;
      if (ucScr & 0x02) REG_BLDY_SUB=ucFade;
      for (ucBcl=0;ucBcl<uWait;ucBcl++) {
        swiWaitForVBlank();
      }
    }
  }
}

UWORD sIndex __attribute__((section(".dtcm")))= 0;
UBYTE dampen_slide_y __attribute__((section(".dtcm")))= 0;
short int screen_slide_y __attribute__((section(".dtcm"))) = 0;

ITCM_CODE void vblankIntr() 
{
    REG_BG2PA = xdxBG; 
    REG_BG2PD = ydyBG; 
    REG_BG2X = cxBG; 

    if (++sIndex & 1)
    {
        // Slight jitter to help with X-screen scaling...
        REG_BG2Y = cyBG+0x40 + (screen_slide_y<<8);
        if (dampen_slide_y == 0)
        {
            if (screen_slide_y < 0) screen_slide_y++;
            else if (screen_slide_y > 0) screen_slide_y--;
        } else dampen_slide_y--;
    }    
    else
    {
        REG_BG2Y = cyBG + (screen_slide_y<<8);
    }
}

void dsInitScreenMain(void) 
{
    // Init vbl and hbl func
    SetYtrigger(190); //trigger 2 lines before vsync
    irqSet(IRQ_VBLANK, vblankIntr);
    irqEnable(IRQ_VBLANK | IRQ_VCOUNT);
    vramSetBankB(VRAM_B_MAIN_BG_0x06020000 ); // Not using this for video but could use it for faster access... 128K of faster VRAM!
    
    vramSetBankD(VRAM_D_LCD );                // Not using this for video but faster RAM always useful!  128K Mapped at 0x06860000
    vramSetBankE(VRAM_E_LCD );                // Not using this for video but faster RAM always useful!   64K Mapped at 0x06880000
    vramSetBankF(VRAM_F_LCD );                // Not using this for video but faster RAM always useful!   16K Mapped at 0x06890000
    vramSetBankG(VRAM_G_LCD );                // Not using this for video but faster RAM always useful!   16K Mapped at 0x06894000
    vramSetBankH(VRAM_H_LCD );                // Not using this for video but faster RAM always useful!   32K Mapped at 0x06898000
    vramSetBankI(VRAM_I_LCD );                // Not using this for video but faster RAM always useful!   16K Mapped at 0x068A0000
}

void dsInitTimer(void) 
{
    TIMER0_DATA=0;
    TIMER0_CR=TIMER_ENABLE|TIMER_DIV_1024; 
}

void dsShowScreenEmu(void) 
{
  // Change vram
  videoSetMode(MODE_5_2D | DISPLAY_BG2_ACTIVE);
  vramSetBankA(VRAM_A_MAIN_BG_0x06000000);
  bg2 = bgInit(2, BgType_Bmp8, BgSize_B8_512x512, 0,0);

  REG_BG2PB = 0;
  REG_BG2PC = 0;

  REG_BG2X = cxBG; 
  REG_BG2Y = cyBG; 
  REG_BG2PA = xdxBG; 
  REG_BG2PD = ydyBG; 
    
  REG_BLDCNT=0; REG_BLDCNT_SUB=0; REG_BLDY=0; REG_BLDY_SUB=0;
}

void dsShowScreenMain(void) {
  // Init BG mode for 16 bits colors
  videoSetMode(MODE_0_2D | DISPLAY_BG0_ACTIVE );
  videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE | DISPLAY_BG1_ACTIVE);
  vramSetBankA(VRAM_A_MAIN_BG); vramSetBankC(VRAM_C_SUB_BG);
  bg0 = bgInit(0, BgType_Text8bpp, BgSize_T_256x256, 31,0);
  bg0b = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 31,0);
  bg1b = bgInitSub(1, BgType_Text8bpp, BgSize_T_256x256, 30,0);
  bgSetPriority(bg0b,1);bgSetPriority(bg1b,0);

  decompress(bgTopTiles, bgGetGfxPtr(bg0), LZ77Vram);
  decompress(bgTopMap, (void*) bgGetMapPtr(bg0), LZ77Vram);
  dmaCopy((void *) bgTopPal,(u16*) BG_PALETTE,256*2);

  restore_bottom_screen();

  REG_BLDCNT=0; REG_BLDCNT_SUB=0; REG_BLDY=0; REG_BLDY_SUB=0;
  
  swiWaitForVBlank();
}

void dsFreeEmu(void) {
  // Stop timer of sound
  TIMER2_CR=0; irqDisable(IRQ_TIMER2); 
}

#define PALETTE_SIZE 768

byte palette_data[PALETTE_SIZE] = {
0x00,0x00,0x00,0x25,0x25,0x25,0x34,0x34,0x34,0x4F,0x4F,0x4F,
0x5B,0x5B,0x5B,0x69,0x69,0x69,0x7B,0x7B,0x7B,0x8A,0x8A,0x8A,
0xA7,0xA7,0xA7,0xB9,0xB9,0xB9,0xC5,0xC5,0xC5,0xD0,0xD0,0xD0,
0xD7,0xD7,0xD7,0xE1,0xE1,0xE1,0xF4,0xF4,0xF4,0xFF,0xFF,0xFF,
0x4C,0x32,0x00,0x62,0x3A,0x00,0x7B,0x4A,0x00,0x9A,0x60,0x00,
0xB5,0x74,0x00,0xCC,0x85,0x00,0xE7,0x9E,0x08,0xF7,0xAF,0x10,
0xFF,0xC3,0x18,0xFF,0xD0,0x20,0xFF,0xD8,0x28,0xFF,0xDF,0x30,
0xFF,0xE6,0x3B,0xFF,0xF4,0x40,0xFF,0xFA,0x4B,0xFF,0xFF,0x50,
0x99,0x25,0x00,0xAA,0x25,0x00,0xB4,0x25,0x00,0xD3,0x30,0x00,
0xDD,0x48,0x02,0xE2,0x50,0x09,0xF4,0x67,0x00,0xF4,0x75,0x10,
0xFF,0x9E,0x10,0xFF,0xAC,0x20,0xFF,0xBA,0x3A,0xFF,0xBF,0x50,
0xFF,0xC6,0x6D,0xFF,0xD5,0x80,0xFF,0xE4,0x90,0xFF,0xE6,0x99,
0x98,0x0C,0x0C,0x99,0x0C,0x0C,0xC2,0x13,0x00,0xD3,0x13,0x00,
0xE2,0x35,0x00,0xE3,0x40,0x00,0xE4,0x40,0x20,0xE5,0x52,0x30,
0xFD,0x78,0x54,0xFF,0x8A,0x6A,0xFF,0x98,0x7C,0xFF,0xA4,0x8B,
0xFF,0xB3,0x9E,0xFF,0xC2,0xB2,0xFF,0xD0,0xBA,0xFF,0xD7,0xC0,
0x99,0x00,0x00,0xA9,0x00,0x00,0xC2,0x04,0x00,0xD3,0x04,0x00,
0xDA,0x04,0x00,0xDB,0x08,0x00,0xE4,0x20,0x20,0xF6,0x40,0x40,
0xFB,0x70,0x70,0xFB,0x7E,0x7E,0xFB,0x8F,0x8F,0xFF,0x9F,0x9F,
0xFF,0xAB,0xAB,0xFF,0xB9,0xB9,0xFF,0xC9,0xC9,0xFF,0xCF,0xCF,
0x7E,0x00,0x50,0x80,0x00,0x50,0x80,0x00,0x5F,0x95,0x0B,0x74,
0xAA,0x22,0x88,0xBB,0x2F,0x9A,0xCE,0x3F,0xAD,0xD7,0x5A,0xB6,
0xE4,0x67,0xC3,0xEF,0x72,0xCE,0xFB,0x7E,0xDA,0xFF,0x8D,0xE1,
0xFF,0x9D,0xE5,0xFF,0xA5,0xE7,0xFF,0xAF,0xEA,0xFF,0xB8,0xEC,
0x48,0x00,0x6C,0x5C,0x04,0x88,0x65,0x0D,0x90,0x7B,0x23,0xA7,
0x93,0x3B,0xBF,0x9D,0x45,0xC9,0xA7,0x4F,0xD3,0xB2,0x5A,0xDE,
0xBD,0x65,0xE9,0xC5,0x6D,0xF1,0xCE,0x76,0xFA,0xD5,0x83,0xFF,
0xDA,0x90,0xFF,0xDE,0x9C,0xFF,0xE2,0xA9,0xFF,0xE6,0xB6,0xFF,
0x1B,0x00,0x70,0x22,0x1B,0x8D,0x37,0x30,0xA2,0x48,0x41,0xB3,
0x59,0x52,0xC4,0x63,0x5C,0xCE,0x6F,0x68,0xDA,0x7D,0x76,0xE8,
0x87,0x80,0xF8,0x93,0x8C,0xFF,0x9D,0x97,0xFF,0xA8,0xA3,0xFF,
0xB3,0xAF,0xFF,0xBC,0xB8,0xFF,0xC4,0xC1,0xFF,0xDA,0xD1,0xFF,
0x00,0x0D,0x7F,0x00,0x12,0xA7,0x00,0x18,0xC0,0x0A,0x2B,0xD1,
0x1B,0x4A,0xE3,0x2F,0x58,0xF0,0x37,0x68,0xFF,0x49,0x79,0xFF,
0x5B,0x85,0xFF,0x6D,0x96,0xFF,0x7F,0xA3,0xFF,0x8C,0xAD,0xFF,
0x96,0xB4,0xFF,0xA8,0xC0,0xFF,0xB7,0xCB,0xFF,0xC6,0xD6,0xFF,
0x00,0x29,0x5A,0x00,0x38,0x76,0x00,0x48,0x92,0x00,0x5C,0xAC,
0x00,0x71,0xC6,0x00,0x86,0xD0,0x0A,0x9B,0xDF,0x1A,0xA8,0xEC,
0x2B,0xB6,0xFF,0x3F,0xC2,0xFF,0x45,0xCB,0xFF,0x59,0xD3,0xFF,
0x7F,0xDA,0xFF,0x8F,0xDE,0xFF,0xA0,0xE2,0xFF,0xB0,0xEB,0xFF,
0x00,0x4A,0x00,0x00,0x4C,0x00,0x00,0x6A,0x20,0x50,0x8E,0x79,
0x40,0x99,0x99,0x00,0x9C,0xAA,0x00,0xA1,0xBB,0x01,0xA4,0xCC,
0x03,0xA5,0xD7,0x05,0xDA,0xE2,0x18,0xE5,0xFF,0x34,0xEA,0xFF,
0x49,0xEF,0xFF,0x66,0xF2,0xFF,0x84,0xF4,0xFF,0x9E,0xF9,0xFF,
0x00,0x4A,0x00,0x00,0x5D,0x00,0x00,0x70,0x00,0x00,0x83,0x00,
0x00,0x95,0x00,0x00,0xAB,0x00,0x07,0xBD,0x07,0x0A,0xD0,0x0A,
0x1A,0xD5,0x40,0x5A,0xF1,0x77,0x82,0xEF,0xA7,0x84,0xED,0xD1,
0x89,0xFF,0xED,0x7D,0xFF,0xFF,0x93,0xFF,0xFF,0x9B,0xFF,0xFF,
0x22,0x4A,0x03,0x27,0x53,0x04,0x30,0x64,0x05,0x3C,0x77,0x0C,
0x45,0x8C,0x11,0x5A,0xA5,0x13,0x1B,0xD2,0x09,0x1F,0xDD,0x00,
0x3D,0xCD,0x2D,0x3D,0xCD,0x30,0x58,0xCC,0x40,0x60,0xD3,0x50,
0xA2,0xEC,0x55,0xB3,0xF2,0x4A,0xBB,0xF6,0x5D,0xC4,0xF8,0x70,
0x2E,0x3F,0x0C,0x36,0x4A,0x0F,0x40,0x56,0x15,0x46,0x5F,0x17,
0x57,0x77,0x1A,0x65,0x85,0x1C,0x74,0x93,0x1D,0x8F,0xA5,0x25,
0xAD,0xB7,0x2C,0xBC,0xC7,0x30,0xC9,0xD5,0x33,0xD4,0xE0,0x3B,
0xE0,0xEC,0x42,0xEA,0xF6,0x45,0xF0,0xFD,0x47,0xF4,0xFF,0x6F,
0x55,0x24,0x00,0x5A,0x2C,0x00,0x6C,0x3B,0x00,0x79,0x4B,0x00,
0xB9,0x75,0x00,0xBB,0x85,0x00,0xC1,0xA1,0x20,0xD0,0xB0,0x2F,
0xDE,0xBE,0x3F,0xE6,0xC6,0x45,0xED,0xCD,0x57,0xF5,0xDB,0x62,
0xFB,0xE5,0x69,0xFC,0xEE,0x6F,0xFD,0xF3,0x77,0xFD,0xF3,0x7F,
0x5C,0x27,0x00,0x5C,0x2F,0x00,0x71,0x3B,0x00,0x7B,0x48,0x00,
0xB9,0x68,0x20,0xBB,0x72,0x20,0xC5,0x86,0x29,0xD7,0x96,0x33,
0xE6,0xA4,0x40,0xF4,0xB1,0x4B,0xFD,0xC1,0x58,0xFF,0xCC,0x55,
0xFF,0xD4,0x61,0xFF,0xDD,0x69,0xFF,0xE6,0x79,0xFF,0xEA,0x98
};

char filetmp[256];
int load_os(char *filename ) 
{
  FILE *romfile = fopen(filename, "rb");
    
  if (romfile == NULL)
  {
      sprintf(filetmp, "/roms/bios/%s", filename);
      romfile = fopen(filetmp, "rb");
  }
    
  if (romfile == NULL)
  {
      sprintf(filetmp, "/data/bios/%s", filename);
      romfile = fopen(filetmp, "rb");
  }
      
  if (romfile == NULL)
  {
     memcpy(atari_os, ROM_altirra_5200_os, 0x800);    // No 5200.rom found... use open source Altirra OS instead
  }
  else
  {
    fread(atari_os, 0x800, 1, romfile);
    fclose(romfile);  
  }
  
    return 0;
} /* end load_os */

void dsLoadGame(char *filename) 
{
    TIMER2_CR=0; irqDisable(IRQ_TIMER2); 

    // load card game if ok
    if (Atari800_OpenFile(filename, true, 1, true) != AFILE_ERROR) 
    {   
      // Initialize the virtual console emulation 
      dsShowScreenEmu();
        
      INPUT_Initialise();
        
      memset(sound_buffer, 0x00, 16);

      // Init palette
      for(u16 index = 0; index < 256; index++) {
        unsigned short r = palette_data[(index * 3) + 0];
        unsigned short g = palette_data[(index * 3) + 1];
        unsigned short b = palette_data[(index * 3) + 2];
        BG_PALETTE[index] = RGB8(r, g, b);
        atari_pal16[index] = index;
      }
      
      TIMER2_DATA = TIMER_FREQ((SOUND_FREQ/2)+20);  // keep this a little faster than our Pokey sound generation 
      TIMER2_CR = TIMER_DIV_1 | TIMER_IRQ_REQ | TIMER_ENABLE;        
      irqSet(IRQ_TIMER2, VsoundHandler);
        
      TIMER0_CR=0;
      TIMER0_DATA=0;
      TIMER0_CR=TIMER_ENABLE|TIMER_DIV_1024;
      atari_frames=0;        
    }
    else dsPrintValue(0,2,0, "UNABLE TO FIND GAME!!");
}

unsigned int dsReadPad(void) {
    unsigned short int keys_pressed, ret_keys_pressed;

    do {
        keys_pressed = keysCurrent();
    } while ((keys_pressed & (KEY_LEFT | KEY_RIGHT | KEY_DOWN | KEY_UP | KEY_A | KEY_B | KEY_L | KEY_R))!=0);

    do {
        keys_pressed = keysCurrent();
    } while ((keys_pressed & (KEY_LEFT | KEY_RIGHT | KEY_DOWN | KEY_UP | KEY_A | KEY_B | KEY_L | KEY_R))==0);
    ret_keys_pressed = keys_pressed;

    do {
        keys_pressed = keysCurrent();
    } while ((keys_pressed & (KEY_LEFT | KEY_RIGHT | KEY_DOWN | KEY_UP | KEY_A | KEY_B | KEY_L | KEY_R))!=0);

    return ret_keys_pressed;
}

bool dsWaitOnQuit(void) {
  char bRet=false, bDone=false;
  unsigned int keys_pressed;
  char posdeb=0;
  static char szName[32];
  
  decompress(bgFileSelTiles, bgGetGfxPtr(bg0b), LZ77Vram);
  decompress(bgFileSelMap, (void*) bgGetMapPtr(bg0b), LZ77Vram);
  dmaCopy((void *) bgFileSelPal,(u16*) BG_PALETTE_SUB,256*2);
  unsigned short dmaVal = *(bgGetMapPtr(bg1b) +31*32);
  dmaFillWords(dmaVal | (dmaVal<<16),(void*) bgGetMapPtr(bg1b),32*24*2);
  
  strcpy(szName,"Quit A5200DS ?");
  dsPrintValue(4,2,0,szName);
  sprintf(szName,"%s","A TO CONFIRM, B TO GO BACK");
  dsPrintValue(16-strlen(szName)/2,23,0,szName);
      
  while(!bDone) {
    strcpy(szName,"          YES          ");
    dsPrintValue(5,10+0,(posdeb == 0 ? 1 :  0),szName);
    strcpy(szName,"          NO           ");
    dsPrintValue(5,14+1,(posdeb == 2 ? 1 :  0),szName);
    swiWaitForVBlank();
  
    // Check pad
    keys_pressed=dsReadPad();
    if (keys_pressed & KEY_UP) {
      if (posdeb) posdeb-=2;
    }
    if (keys_pressed & KEY_DOWN) {
      if (posdeb<1) posdeb+=2;
    }
    if (keys_pressed & KEY_A) {
      bRet = (posdeb ? false : true);
      bDone = true;
    }
    if (keys_pressed & KEY_B) {
      bDone = true;
    }
  }

  restore_bottom_screen();

  return bRet;
}


void dsDisplayFiles(unsigned int NoDebGame,u32 ucSel) {
  unsigned short int ucBcl,ucGame;
  u8 maxLen;
  static char szName[256];
  
  // Display all games if possible
  unsigned short dmaVal = *(bgGetMapPtr(bg1b) +31*32);
  dmaFillWords(dmaVal | (dmaVal<<16),(void*) (bgGetMapPtr(bg1b)),32*24*2);
  countfiles ? sprintf(szName,"%04d/%04d GAMES",(int)(1+ucSel+NoDebGame),countfiles) : sprintf(szName,"%04d/%04d FOLDERS",(int)(1+ucSel+NoDebGame),counta5200);
  dsPrintValue(2,1,0,szName);
  dsPrintValue(31,5,0,(char *) (NoDebGame>0 ? "<" : " "));
  dsPrintValue(31,22,0,(char *) (NoDebGame+14<counta5200 ? ">" : " "));
  sprintf(szName,"%s","A TO SELECT A GAME, B TO GO BACK");
  dsPrintValue(16-strlen(szName)/2,23,0,szName);
  for (ucBcl=0;ucBcl<17; ucBcl++) {
    ucGame= ucBcl+NoDebGame;
    if (ucGame < counta5200) {
      char szName2[256];
      maxLen=strlen(a5200romlist[ucGame].filename);
      strcpy(szName,a5200romlist[ucGame].filename);
      if (maxLen>29) szName[29]='\0';
      if (a5200romlist[ucGame].directory) 
      {
        char szName3[36];
        sprintf(szName3,"[%s]",szName);
        sprintf(szName2,"%-29s",szName3);
        dsPrintValue(0,5+ucBcl,(ucSel == ucBcl ? 1 :  0),szName2);
      }
      else 
      {
        sprintf(szName2,"%-29s",strupr(szName));
        dsPrintValue(1,5+ucBcl,(ucSel == ucBcl ? 1 : 0),szName2);
      }
    }
  }
}

unsigned int dsWaitForRom(void) 
{
  char bDone=false, bRet=false;
  u16 ucHaut=0x00, ucBas=0x00,ucSHaut=0x00, ucSBas=0x00,romSelected= 0, firstRomDisplay=0,nbRomPerPage, uNbRSPage, uLenFic=0,ucFlip=0, ucFlop=0;
  static char szName[64];

  decompress(bgFileSelTiles, bgGetGfxPtr(bg0b), LZ77Vram);
  decompress(bgFileSelMap, (void*) bgGetMapPtr(bg0b), LZ77Vram);
  dmaCopy((void *) bgFileSelPal,(u16*) BG_PALETTE_SUB,256*2);
  unsigned short dmaVal = *(bgGetMapPtr(bg1b) +31*32);
  dmaFillWords(dmaVal | (dmaVal<<16),(void*) bgGetMapPtr(bg1b),32*24*2);
  
  nbRomPerPage = (counta5200>=17 ? 17 : counta5200);
  uNbRSPage = (counta5200>=5 ? 5 : counta5200);
  if (ucFicAct>counta5200-nbRomPerPage) {
    firstRomDisplay=counta5200-nbRomPerPage;
    romSelected=ucFicAct-counta5200+nbRomPerPage;
  }
  else {
    firstRomDisplay=ucFicAct;
    romSelected=0;
  }
  dsDisplayFiles(firstRomDisplay,romSelected);
  while (!bDone) {
    if (keysCurrent() & KEY_UP) {
      if (!ucHaut) {
        ucFicAct = (ucFicAct>0 ? ucFicAct-1 : counta5200-1);
        if (romSelected>uNbRSPage) { romSelected -= 1; }
        else {
          if (firstRomDisplay>0) { firstRomDisplay -= 1; }
          else {
            if (romSelected>0) { romSelected -= 1; }
            else {
              firstRomDisplay=counta5200-nbRomPerPage;
              romSelected=nbRomPerPage-1;
            }
          }
        }
        ucHaut=0x01;
        dsDisplayFiles(firstRomDisplay,romSelected);
      }
      else {
        ucHaut++;
        if (ucHaut>10) ucHaut=0;
      } 
      uLenFic=0; ucFlip=0;        
    }
    else {
      ucHaut = 0;
    }  
    if (keysCurrent() & KEY_DOWN) {
      if (!ucBas) {
        ucFicAct = (ucFicAct< counta5200-1 ? ucFicAct+1 : 0);
        if (romSelected<uNbRSPage-1) { romSelected += 1; }
        else {
          if (firstRomDisplay<counta5200-nbRomPerPage) { firstRomDisplay += 1; }
          else {
            if (romSelected<nbRomPerPage-1) { romSelected += 1; }
            else {
              firstRomDisplay=0;
              romSelected=0;
            }
          }
        }
        ucBas=0x01;
        dsDisplayFiles(firstRomDisplay,romSelected);
      }
      else {
        ucBas++;
        if (ucBas>10) ucBas=0;
      }
      uLenFic=0; ucFlip=0;
    }
    else {
      ucBas = 0;
    }  
    if ((keysCurrent() & KEY_R) || (keysCurrent() & KEY_RIGHT)) {
      if (!ucSBas) {
        ucFicAct = (ucFicAct< counta5200-nbRomPerPage ? ucFicAct+nbRomPerPage : counta5200-nbRomPerPage);
        if (firstRomDisplay<counta5200-nbRomPerPage) { firstRomDisplay += nbRomPerPage; }
        else { firstRomDisplay = counta5200-nbRomPerPage; }
        if (ucFicAct == counta5200-nbRomPerPage) romSelected = 0;
        ucSBas=0x01;
        dsDisplayFiles(firstRomDisplay,romSelected);
      }
      else {
        ucSBas++;
        if (ucSBas>10) ucSBas=0;
      } 
    }
    else {
      ucSBas = 0;
    }  
    if ((keysCurrent() & KEY_L) || (keysCurrent() & KEY_LEFT)) {
      if (!ucSHaut) {
        ucFicAct = (ucFicAct> nbRomPerPage ? ucFicAct-nbRomPerPage : 0);
        if (firstRomDisplay>nbRomPerPage) { firstRomDisplay -= nbRomPerPage; }
        else { firstRomDisplay = 0; }
        if (ucFicAct == 0) romSelected = 0;
        if (romSelected > ucFicAct) romSelected = ucFicAct;
        ucSHaut=0x01;
        dsDisplayFiles(firstRomDisplay,romSelected);
      }
      else {
        ucSHaut++;
        if (ucSHaut>10) ucSHaut=0;
      } 
    }
    else {
      ucSHaut = 0;
    }  
        if ( keysCurrent() & KEY_B ) {
      bDone=true;
      while (keysCurrent() & KEY_B);
    }

    if (keysCurrent() & KEY_A)
    {
      if (!a5200romlist[ucFicAct].directory) 
      {
        if (keysCurrent() & KEY_X) DEBUG_DUMP=1; else DEBUG_DUMP=0;
        bRet=true;
        bDone=true;
      }
      else 
      {
        chdir(a5200romlist[ucFicAct].filename);
        a52FindFiles();
        ucFicAct = 0;
        nbRomPerPage = (counta5200>=16 ? 16 : counta5200);
        uNbRSPage = (counta5200>=5 ? 5 : counta5200);
        if (ucFicAct>counta5200-nbRomPerPage) {
          firstRomDisplay=counta5200-nbRomPerPage;
          romSelected=ucFicAct-counta5200+nbRomPerPage;
        }
        else {
          firstRomDisplay=ucFicAct;
          romSelected=0;
        }
        dsDisplayFiles(firstRomDisplay,romSelected);
        while (keysCurrent() & KEY_A);
      }
    }
      
      
    // Scroll the current selection
    if (strlen(a5200romlist[ucFicAct].filename) > 29) {
      ucFlip++;
      if (ucFlip >= 10) {
        ucFlip = 0;
        uLenFic++;
        if ((uLenFic+29)>strlen(a5200romlist[ucFicAct].filename)) {
          ucFlop++;
          if (ucFlop >= 10) {
            uLenFic=0;
            ucFlop = 0;
          }
          else
            uLenFic--;
        }
        strncpy(szName,a5200romlist[ucFicAct].filename+uLenFic,29);
        szName[29] = '\0';
        dsPrintValue(1,5+romSelected,1,szName);
      }
    }

    swiWaitForVBlank();
  }
  
  restore_bottom_screen();  
  return bRet;
}

unsigned int dsWaitOnMenu(unsigned int actState) {
  unsigned int uState=A5200_PLAYINIT;
  unsigned int keys_pressed;
  bool bDone=false, romSel;
  int iTx,iTy;
  
  while (!bDone) {
    // wait for stylus
    keys_pressed = keysCurrent();
    if (keys_pressed & KEY_TOUCH) {
      touchPosition touch;
      touchRead(&touch);
      iTx = touch.px;
      iTy = touch.py;
      if ((iTx>206) && (iTx<250) && (iTy>110) && (iTy<129))  { // 207,111  -> 249,128   quit
        soundPlaySample(clickNoQuit_wav, SoundFormat_16Bit, clickNoQuit_wav_size, 22050, 127, 64, false, 0);
        bDone=dsWaitOnQuit();
        if (bDone) uState=A5200_QUITSTDS;
      }
      if ((iTx>71) && (iTx<183) && (iTy>7) && (iTy<43)) {     // 72,8 -> 182,42 cartridge slot
        bDone=true; 
        // Find files in current directory and show it 
        a52FindFiles();
        romSel=dsWaitForRom();
        if (romSel) { uState=A5200_PLAYINIT; 
          dsLoadGame(a5200romlist[ucFicAct].filename); }
        else { uState=actState; }
      }
    }
    swiWaitForVBlank();
  }
  
  return uState;
}

void dsPrintValue(int x, int y, unsigned int isSelect, char *pchStr)
{
  u16 *pusEcran,*pusMap;
  u16 usCharac;
  char *pTrTxt=pchStr;
  char ch;

  pusEcran=(u16*) (bgGetMapPtr(bg1b))+x+(y<<5);
  pusMap=(u16*) (bgGetMapPtr(bg0b)+(2*isSelect+24)*32);

  while((*pTrTxt)!='\0' )
  {
    ch = *pTrTxt;
    if (ch >= 'a' && ch <= 'z') ch -= 32; // Faster than strcpy/strtoupper
    usCharac=0x0000;
    if ((ch) == '|')
      usCharac=*(pusMap);
    else if (((ch)<' ') || ((ch)>'_'))
      usCharac=*(pusMap);
    else if((ch)<'@')
      usCharac=*(pusMap+(ch)-' ');
    else
      usCharac=*(pusMap+32+(ch)-'@');
    *pusEcran++=usCharac;
    pTrTxt++;
  }
}


//----------------------------------------------------------------------------------
// Find files (a78 / bin) available
int a52Filescmp (const void *c1, const void *c2) {
  FICA5200 *p1 = (FICA5200 *) c1;
  FICA5200 *p2 = (FICA5200 *) c2;
  if (p1->filename[0] == '.' && p2->filename[0] != '.')
      return -1;
  if (p2->filename[0] == '.' && p1->filename[0] != '.')
      return 1;
  if (p1->directory && !(p2->directory))
      return -1;
  if (p2->directory && !(p1->directory))
      return 1;
  return strcasecmp (p1->filename, p2->filename);
}

void a52FindFiles(void) {
  DIR *pdir;
  struct dirent *pent;
  static char filenametmp[MAX_FILENAME_LEN];
  
  counta5200 = countfiles= 0;
  
  pdir = opendir(".");

  if (pdir) {

    while (((pent=readdir(pdir))!=NULL)) 
    {
      strcpy(filenametmp,pent->d_name);
      if (pent->d_type == DT_DIR)
      {
        if (!( (filenametmp[0] == '.') && (strlen(filenametmp) == 1))) {
          a5200romlist[counta5200].directory = true;
          strcpy(a5200romlist[counta5200].filename,filenametmp);
          counta5200++;
        }
      }
      else {
        if (strlen(filenametmp)>4) {
          if ( (strcasecmp(strrchr(filenametmp, '.'), ".a52") == 0) )  {
            a5200romlist[counta5200].directory = false;
            strcpy(a5200romlist[counta5200].filename,filenametmp);
            counta5200++;countfiles++;
          }
          if ( (strcasecmp(strrchr(filenametmp, '.'), ".bin") == 0) )  {
            a5200romlist[counta5200].directory = false;
            strcpy(a5200romlist[counta5200].filename,filenametmp);
            counta5200++;countfiles++;
          }
        }
      }
    }
    closedir(pdir);
  }
  if (counta5200)
  {
    qsort (a5200romlist, counta5200, sizeof (FICA5200), a52Filescmp);
  }
  else  // Failsafe... always provide a back directory...
  {
    a5200romlist[counta5200].directory = true;
    strcpy(a5200romlist[counta5200].filename,"..");
    counta5200 = 1;
  }
}

//---------------------------------------------------------------------------------
void dsInstallSoundEmuFIFO(void) 
{
    // We are going to use the 16-bit sound engine so we need to scale up our 8-bit values...
    for (int i=0; i<256; i++)
    {
        sampleExtender[i] = (i << 8);
    }
    
    if (isDSiMode())
    {
        aptr = (u16*) ((u32)&sound_buffer[0] + 0xA000000); 
        bptr = (u16*) ((u32)&sound_buffer[2] + 0xA000000);
    }
    else
    {
        aptr = (u16*) ((u32)&sound_buffer[0] + 0x00400000);
        bptr = (u16*) ((u32)&sound_buffer[2] + 0x00400000);
    }
        
    FifoMessage msg;
    msg.SoundPlay.data = &sound_buffer;
    msg.SoundPlay.freq = SOUND_FREQ*2;
    msg.SoundPlay.volume = 127;
    msg.SoundPlay.pan = 64;
    msg.SoundPlay.loop = 1;
    msg.SoundPlay.format = ((1)<<4) | SoundFormat_16Bit;
    msg.SoundPlay.loopPoint = 0;
    msg.SoundPlay.dataSize = 4 >> 2;
    msg.type = EMUARM7_PLAY_SND;
    fifoSendDatamsg(FIFO_USER_01, sizeof(msg), (u8*)&msg);
}

extern u16 trig0, trig1;
extern u16 stick0;
extern u16 stick1;
char full_speed = 0;

void dsMainLoop(void) {
  static char fpsbuf[32];
  unsigned short int keys_pressed,keys_touch=0, romSel;
  short int iTx,iTy, shiftctrl;
  char showFps=false;
  
  // Timers are fed with 33.513982 MHz clock.
  // With DIV_1024 the clock is 32,728.5 ticks per sec...
  TIMER0_DATA=0;
  TIMER0_CR=TIMER_ENABLE|TIMER_DIV_1024;
  TIMER1_DATA=0;
  TIMER1_CR=TIMER_ENABLE | TIMER_DIV_1024;  
  
  while(etatEmu != A5200_QUITSTDS) 
  {
        switch (etatEmu) 
        {    
          case A5200_MENUINIT:
            dsShowScreenMain();
            etatEmu = A5200_MENUSHOW;
            break;

          case A5200_MENUSHOW:
            etatEmu =  dsWaitOnMenu(A5200_MENUSHOW);
            Atari800_Initialise();
            break;

          case A5200_PLAYINIT:
            irqDisable(IRQ_TIMER2);  
            dsShowScreenEmu();
            VsoundClear();
            swiWaitForVBlank();swiWaitForVBlank();
            irqEnable(IRQ_TIMER2);  
            bSoundMute = false;
            etatEmu = A5200_PLAYGAME;
            atari_frames=0;
            TIMER0_DATA=0;
            TIMER0_CR=TIMER_ENABLE|TIMER_DIV_1024;
            TIMER1_DATA=0;
            TIMER1_CR=TIMER_ENABLE | TIMER_DIV_1024;  
            break;

          case A5200_PLAYGAME:

            // 32,728.5 ticks = 1 second
            // 1 frame = 1/50 or 1/60 (0.02 or 0.016)
            // 655 -> 50 fps and 546 -> 60 fps
            if (!full_speed)
            {
                while(TIMER0_DATA < (546*atari_frames))
                    ;
            }

            // Execute one frame
            Atari800_Frame();

            if (++atari_frames == 60)
            {
                TIMER0_CR=0;
                TIMER0_DATA=0;
                TIMER0_CR=TIMER_ENABLE|TIMER_DIV_1024;
                atari_frames=0;
            }            

            // -------------------------------------------------------------
            // Stuff to do once/second such as FPS display and Debug Data
            // -------------------------------------------------------------
            if (TIMER1_DATA >= 32728)   // 1000MS (1 sec)
            {
                TIMER1_CR = 0;
                TIMER1_DATA = 0;
                TIMER1_CR=TIMER_ENABLE | TIMER_DIV_1024;

                if (!full_speed && (gTotalAtariFrames > 60)) gTotalAtariFrames--;   // We tend to overshoot... 
                if (showFps) { sprintf(fpsbuf,"%03d",gTotalAtariFrames); dsPrintValue(0,0,0, fpsbuf); } // Show FPS
                DumpDebugData();
                gTotalAtariFrames = 0;
            }

            // Read keys
            keys_pressed=keysCurrent();
            key_consol = CONSOL_NONE; //|= (CONSOL_OPTION | CONSOL_SELECT | CONSOL_START); /* OPTION/START/SELECT key OFF */
            shiftctrl = 0; key_shift = 0;
            trig0 = ((keys_pressed & KEY_A) || (keys_pressed & KEY_Y)) ? 0 : 1;
            stick0 = STICK_CENTRE;
            stick1 = STICK_CENTRE;

            if (keys_pressed & KEY_B) { shiftctrl ^= AKEY_SHFT; key_shift = 1; }
            key_code = shiftctrl ? 0x40 : 0x00;

            // if touch screen pressed
            if (keys_pressed & KEY_TOUCH) 
            {
                touchPosition touch;
                touchRead(&touch);
                iTx = touch.px;
                iTy = touch.py;
                if (iTy < 20)
                {
                    if ((iTx>240) && (iTx<256) && (iTy>0) && (iTy<20))  { // Full Speed Toggle ... upper right corner...
                       if (keys_touch == 0)
                       {
                           full_speed = 1-full_speed; 
                           if (full_speed) dsPrintValue(30,0,0,"FS"); else dsPrintValue(30,0,0,"  ");
                           keys_touch = 1;
                       }
                    }
                    else if ((iTx>0) && (iTx<20) && (iTy>0) && (iTy<20))  { // FPS Counter ... upper left corner...
                       if (keys_touch == 0)
                       {
                            showFps = 1-showFps; 
                            dsPrintValue(0,0,0, "   ");
                            keys_touch = 1;
                       }
                    }
                }
                else if (iTy < 50)
                {
                    if ((iTx>70) && (iTx<185) && (iTy>7) && (iTy<50)) {     // 72,8 -> 182,42 cartridge slot
                      bSoundMute = true;
                      // Find files in current directory and show it 
                      a52FindFiles();
                      romSel=dsWaitForRom();
                      bSoundMute = false;
                      if (romSel) 
                      { 
                          etatEmu=A5200_PLAYINIT; 
                          dsLoadGame(a5200romlist[ucFicAct].filename); 
                          if (full_speed) dsPrintValue(30,0,0,"FS"); else dsPrintValue(30,0,0,"  ");
                      }
                    }
                }
                else if (iTy < 130)
                {
                    if ((iTx>211) && (iTx<250) && (iTy>112) && (iTy<130))  { //quit
                      bSoundMute = true;                
                      soundPlaySample(clickNoQuit_wav, SoundFormat_16Bit, clickNoQuit_wav_size, 22050, 127, 64, false, 0);
                      if (dsWaitOnQuit()) etatEmu=A5200_QUITSTDS;
                      else { bSoundMute = false;}
                    }
                    else if ((iTx>160) && (iTx<200) && (iTy>112) && (iTy<130))  { //highscore
                      bSoundMute = true;
                      highscore_display();
                      restore_bottom_screen();
                      bSoundMute = false;
                    }
                    else if ((iTx>115) && (iTx<150) && (iTy>112) && (iTy<130))  { //pause
                      if (!keys_touch) soundPlaySample(clickNoQuit_wav, SoundFormat_16Bit, clickNoQuit_wav_size, 22050, 127, 64, false, 0);
                      key_code = AKEY_5200_PAUSE + key_code;
                      keys_touch = 1;
                    }
                    else if ((iTx>64) && (iTx<105) && (iTy>112) && (iTy<130))  { //reset
                      if (!keys_touch) soundPlaySample(clickNoQuit_wav, SoundFormat_16Bit, clickNoQuit_wav_size, 22050, 127, 64, false, 0);
                      key_code = AKEY_5200_RESET + key_code;
                      keys_touch = 1;
                    }
                    else if ((iTx>8) && (iTx<54) && (iTy>112) && (iTy<130))  { //start
                      if (!keys_touch) soundPlaySample(clickNoQuit_wav, SoundFormat_16Bit, clickNoQuit_wav_size, 22050, 127, 64, false, 0);
                      key_code = AKEY_5200_START + key_code;
                      keys_touch = 1;
                    }
                }
                else
                {
                    if (bStarRaiders) // Special Overlay for Star Raiders
                    {
                        if ((iTy>144) && (iTy<191))  // This is our 5200 Keypad 0-9,#,*
                        { 
                          if (!keys_dampen && (keys_touch==0)) // First time pressed?
                          {
                              keys_touch = 1;
                              soundPlaySample(clickNoQuit_wav, SoundFormat_16Bit, clickNoQuit_wav_size, 22050, 127, 64, false, 0);
                              if (iTx > 0) iTx--;
                              if (iTy < 169) key_code = padKeySR[0 + (iTx / 42)] | key_code;
                              else           key_code = padKeySR[6 + (iTx / 42)] | key_code;
                              last_key_code = key_code;
                              keys_dampen = 20; // A half second for consistent debounce
                          }
                          else
                          {
                              key_code = last_key_code;
                              if (keys_dampen) keys_dampen--; else last_key_code=0x00;
                          }
                        }
                    }
                    else
                    {
                        if ((iTy>150) && (iTy<185))  // This is our 5200 Keypad 0-9,#,*
                        { 
                          if (!keys_dampen) // First time pressed?
                          {
                              soundPlaySample(clickNoQuit_wav, SoundFormat_16Bit, clickNoQuit_wav_size, 22050, 127, 64, false, 0);
                              if (iTx > 0) iTx--;
                              key_code = padKey[iTx / 21] | key_code;
                              last_key_code = key_code;
                              keys_dampen = 15; // One-fourth of a second for consistent debounce
                          }
                          else
                          {
                              key_code = last_key_code;
                              keys_dampen--;
                          }
                        }
                    }
                }
            }
            else
            {
                keys_touch = 0;
                if (keys_dampen)
                {
                    key_code = last_key_code;
                    keys_dampen--;
                }
                else
                {
                    last_key_code = 0x00;
                }
            }

            if ((myCart.control == CTRL_JOY) || (myCart.control == CTRL_SR))
            {
              if (keys_pressed & KEY_UP) stick0 = STICK_FORWARD;
              if (keys_pressed & KEY_LEFT) stick0 = STICK_LEFT;
              if (keys_pressed & KEY_RIGHT) stick0 = STICK_RIGHT;
              if (keys_pressed & KEY_DOWN) stick0 = STICK_BACK;
              if ((keys_pressed & KEY_UP) && (keys_pressed & KEY_LEFT)) stick0 = STICK_UL; 
              if ((keys_pressed & KEY_UP) && (keys_pressed & KEY_RIGHT)) stick0 = STICK_UR;
              if ((keys_pressed & KEY_DOWN) && (keys_pressed & KEY_LEFT)) stick0 = STICK_LL;
              if ((keys_pressed & KEY_DOWN) && (keys_pressed & KEY_RIGHT)) stick0 = STICK_LR;
            }
            else if (myCart.control == CTRL_SWAP)
            {
              trig1 = (keys_pressed & KEY_A) ? 0 : 1;
              if (keys_pressed & KEY_UP) stick1 = STICK_FORWARD;
              if (keys_pressed & KEY_LEFT) stick1 = STICK_LEFT;
              if (keys_pressed & KEY_RIGHT) stick1 = STICK_RIGHT;
              if (keys_pressed & KEY_DOWN) stick1 = STICK_BACK;
              if ((keys_pressed & KEY_UP) && (keys_pressed & KEY_LEFT)) stick1 = STICK_UL; 
              if ((keys_pressed & KEY_UP) && (keys_pressed & KEY_RIGHT)) stick1 = STICK_UR;
              if ((keys_pressed & KEY_DOWN) && (keys_pressed & KEY_LEFT)) stick1 = STICK_LL;
              if ((keys_pressed & KEY_DOWN) && (keys_pressed & KEY_RIGHT)) stick1 = STICK_LR;          
            }
            else if (myCart.control == CTRL_ROBO)
            {
              if (keys_pressed & KEY_UP) stick0 = STICK_FORWARD;
              if (keys_pressed & KEY_LEFT) stick0 = STICK_LEFT;
              if (keys_pressed & KEY_RIGHT) stick0 = STICK_RIGHT;
              if (keys_pressed & KEY_DOWN) stick0 = STICK_BACK;
              if ((keys_pressed & KEY_UP) && (keys_pressed & KEY_LEFT)) stick0 = STICK_UL; 
              if ((keys_pressed & KEY_UP) && (keys_pressed & KEY_RIGHT)) stick0 = STICK_UR;
              if ((keys_pressed & KEY_DOWN) && (keys_pressed & KEY_LEFT)) stick0 = STICK_LL;
              if ((keys_pressed & KEY_DOWN) && (keys_pressed & KEY_RIGHT)) stick0 = STICK_LR;
              if (keys_pressed & KEY_X) stick1 = STICK_FORWARD;
              if (keys_pressed & KEY_Y) stick1 = STICK_LEFT;
              if (keys_pressed & KEY_A) stick1 = STICK_RIGHT;
              if (keys_pressed & KEY_B) stick1 = STICK_BACK;
              if ((keys_pressed & KEY_X) && (keys_pressed & KEY_Y)) stick1 = STICK_UL; 
              if ((keys_pressed & KEY_X) && (keys_pressed & KEY_A)) stick1 = STICK_UR;
              if ((keys_pressed & KEY_B) && (keys_pressed & KEY_Y)) stick1 = STICK_LL;
              if ((keys_pressed & KEY_B) && (keys_pressed & KEY_A)) stick1 = STICK_LR;          
            }
            else if (myCart.control == CTRL_FROG)
            {
              trig0=0;
              if (keys_pressed & KEY_UP) {stick0 = STICK_FORWARD; trig0=1;}
              if (keys_pressed & KEY_LEFT) {stick0 = STICK_LEFT;  trig0=1;}
              if (keys_pressed & KEY_RIGHT) {stick0 = STICK_RIGHT;trig0=1;}
              if (keys_pressed & KEY_DOWN) {stick0 = STICK_BACK;  trig0=1;}
            }
            else if (myCart.control == CTRL_QBERT)
            {
              if (keys_pressed & KEY_UP) {stick0 = STICK_UR; }
              if (keys_pressed & KEY_LEFT) {stick0 = STICK_UL;}
              if (keys_pressed & KEY_RIGHT) {stick0 = STICK_LR;}
              if (keys_pressed & KEY_DOWN) {stick0 = STICK_LL; }
              trig0=0;
            }

            if (keys_pressed & KEY_START) key_code = AKEY_5200_START + key_code;
            if (keys_pressed & KEY_SELECT) key_code = AKEY_5200_PAUSE + key_code;

            if ((gTotalAtariFrames & 3) == 0)  // Every fourth frame...
            {
                if ((keys_pressed & KEY_R) && (keys_pressed & KEY_UP))   myCart.offset_y++;
                if ((keys_pressed & KEY_R) && (keys_pressed & KEY_DOWN)) myCart.offset_y--;
                if ((keys_pressed & KEY_R) && (keys_pressed & KEY_LEFT))  myCart.offset_x++;
                if ((keys_pressed & KEY_R) && (keys_pressed & KEY_RIGHT)) myCart.offset_x--;

                if ((keys_pressed & KEY_L) && (keys_pressed & KEY_UP))   if (myCart.scale_y < 256) myCart.scale_y++;
                if ((keys_pressed & KEY_L) && (keys_pressed & KEY_DOWN)) if (myCart.scale_y >= 192) myCart.scale_y--;
                if ((keys_pressed & KEY_L) && (keys_pressed & KEY_RIGHT))  if (myCart.scale_x < 320) myCart.scale_x++;
                if ((keys_pressed & KEY_L) && (keys_pressed & KEY_LEFT)) if (myCart.scale_x >= 192) myCart.scale_x--;

                if ((keys_pressed & KEY_R) && (keys_pressed & KEY_L))
                {
                    if (++lcd_swap_counter == 8)
                    {
                        if (keys_pressed & KEY_A)   {lcdSwap();}
                    }
                } else lcd_swap_counter = 0;
            }            

            // Screen shift/slide
            if (myCart.control != CTRL_ROBO)          
            {
                if (keys_pressed & KEY_X)
                {
                    if (myCart.x_function == X_PANUP)
                    {
                        screen_slide_y = 12;  dampen_slide_y = 6;
                    }
                    else if (myCart.x_function == X_PANDN)
                    {
                        screen_slide_y = -12;  dampen_slide_y = 6;
                    }
                }
            }

            break;
        }
    }
}


void _putchar(char character) {};   // Not used but needed to link printf()
