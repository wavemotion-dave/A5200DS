// =====================================================================================
// Copyright (c) 2021-2025 Dave Bernazzani (wavemotion-dave)
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
u8  bSoundMute __attribute__((section(".dtcm"))) = false;
u8  unMuteSoon = 0;

char padKey[]   = {AKEY_5200_0,AKEY_5200_1,AKEY_5200_2,AKEY_5200_3,AKEY_5200_4,AKEY_5200_5,AKEY_5200_6,AKEY_5200_7,AKEY_5200_8,AKEY_5200_9,AKEY_5200_HASH,AKEY_5200_ASTERISK};
char padKeySR[] = {AKEY_5200_1,AKEY_5200_2,AKEY_5200_3,AKEY_5200_4,AKEY_5200_5,AKEY_5200_6,AKEY_5200_7,AKEY_5200_8,AKEY_5200_9,AKEY_5200_ASTERISK,AKEY_5200_0,AKEY_5200_HASH};

#define  cxBG (myCart.offset_x<<8)
#define  cyBG (myCart.offset_y<<8)
#define  xdxBG (((320 / myCart.scale_x) << 8) | (320 % myCart.scale_x))
#define  ydyBG (((256 / myCart.scale_y) << 8) | (256 % myCart.scale_y))
  
unsigned char sound_buffer[16] __attribute__ ((aligned (4))) = {0}; // Never fast memory as it's shared with ARM7
u16* aptr __attribute__((section(".dtcm"))) = (u16*) ((u32)&sound_buffer[0] + 0xA000000); 
u16* bptr __attribute__((section(".dtcm"))) = (u16*) ((u32)&sound_buffer[2] + 0xA000000);

unsigned int atari_pal16[256] = {0};

static int last_key_code = 0x00;
static UWORD keys_dampen = 0;
char bStarRaiders=0;
char lcd_swap_counter = 0;

u16 myPokeyBufIdx            __attribute__((section(".dtcm"))) = 0;
u16 sampleExtender[256]      __attribute__((section(".dtcm"))) = {0};

#define MAX_DEBUG 16
ULONG debug[MAX_DEBUG]={0};
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

extern unsigned char pokey_buffer[];
extern u16 pokeyBufIdx;

void VsoundClear(void)
{
    extern void PokeyClearBuffer(void);
    
    PokeyClearBuffer();   
    myPokeyBufIdx = pokeyBufIdx = 0;
}

ITCM_CODE void VsoundHandler(void) 
{
  if (bSoundMute) {*aptr = *bptr;}
  // If there is a fresh sample... 
  else if (myPokeyBufIdx != pokeyBufIdx)
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

ITCM_CODE void VsoundHandlerDSi(void) 
{
  if (bSoundMute) {*aptr = *bptr;}
  // If there is a fresh sample... 
  else if (myPokeyBufIdx != pokeyBufIdx)
  {
      *aptr = *bptr = sampleExtender[pokey_buffer[myPokeyBufIdx]];
      myPokeyBufIdx = (myPokeyBufIdx+1) & (SNDLENGTH-1);
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
    vramSetBankB(VRAM_B_LCD);                 // Not using this for video but faster RAM always useful!  128K Mapped at 0x06820000
    vramSetBankD(VRAM_D_LCD );                // Not using this for video but faster RAM always useful!  128K Mapped at 0x06860000
    vramSetBankE(VRAM_E_LCD );                // Not using this for video but faster RAM always useful!   64K Mapped at 0x06880000
    vramSetBankF(VRAM_F_LCD );                // Not using this for video but faster RAM always useful!   16K Mapped at 0x06890000
    vramSetBankG(VRAM_G_LCD );                // Not using this for video but faster RAM always useful!   16K Mapped at 0x06894000
    vramSetBankH(VRAM_H_LCD );                // Not using this for video but faster RAM always useful!   32K Mapped at 0x06898000
    vramSetBankI(VRAM_I_LCD );                // Not using this for video but faster RAM always useful!   16K Mapped at 0x068A0000 (using for grafp_lookup[] table)
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

byte palette_data[PALETTE_SIZE] = {  // NTSC_LUM1_HOT 5200 from Trebor 
  0x00, 0x00, 0x00, 0x1A, 0x1A, 0x1A, 0x2C, 0x2C, 0x2C, 0x3C, 0x3C, 0x3C, 0x4B, 0x4B, 0x4B, 0x59, 0x59, 0x59, 0x66, 0x66, 0x66, 0x73, 0x73, 0x73, 0x7F, 0x7F, 0x7F, 0x8B, 0x8B, 0x8B, 0x97, 0x97, 0x97, 0xA3, 0xA3, 0xA3, 0xAE, 0xAE, 0xAE, 0xB9,
  0xB9, 0xB9, 0xC4, 0xC4, 0xC4, 0xCE, 0xCE, 0xCE, 0x27, 0x15, 0x00, 0x37, 0x28, 0x00, 0x46, 0x38, 0x00, 0x54, 0x47, 0x00, 0x62, 0x55, 0x00, 0x6F, 0x63, 0x00, 0x7C, 0x70, 0x00, 0x88, 0x7C, 0x00, 0x94, 0x88, 0x00, 0x9F, 0x94, 0x07, 0xAA, 0xA0,
  0x1D, 0xB6, 0xAB, 0x2F, 0xC0, 0xB6, 0x3F, 0xCB, 0xC1, 0x4D, 0xD6, 0xCC, 0x5B, 0xE0, 0xD6, 0x68, 0x46, 0x00, 0x00, 0x54, 0x0A, 0x00, 0x62, 0x1F, 0x00, 0x6F, 0x31, 0x00, 0x7B, 0x40, 0x00, 0x87, 0x4F, 0x00, 0x93, 0x5D, 0x00, 0x9F, 0x6A, 0x00,
  0xAA, 0x76, 0x07, 0xB5, 0x83, 0x1D, 0xC0, 0x8F, 0x2F, 0xCB, 0x9A, 0x3F, 0xD5, 0xA6, 0x4D, 0xE0, 0xB1, 0x5B, 0xEA, 0xBC, 0x69, 0xF4, 0xC7, 0x75, 0x56, 0x00, 0x00, 0x63, 0x00, 0x00, 0x70, 0x00, 0x00, 0x7D, 0x1A, 0x00, 0x89, 0x2C, 0x00, 0x95,
  0x3C, 0x00, 0xA0, 0x4B, 0x1A, 0xAC, 0x59, 0x2C, 0xB7, 0x66, 0x3C, 0xC2, 0x73, 0x4B, 0xCC, 0x7F, 0x59, 0xD7, 0x8B, 0x66, 0xE1, 0x97, 0x73, 0xEB, 0xA3, 0x7F, 0xF6, 0xAE, 0x8B, 0xFF, 0xB9, 0x97, 0x56, 0x00, 0x00, 0x64, 0x00, 0x00, 0x71, 0x00,
  0x19, 0x7D, 0x06, 0x2B, 0x89, 0x1D, 0x3B, 0x95, 0x2E, 0x4A, 0xA1, 0x3E, 0x58, 0xAC, 0x4D, 0x65, 0xB7, 0x5B, 0x72, 0xC2, 0x68, 0x7F, 0xCD, 0x75, 0x8B, 0xD7, 0x81, 0x97, 0xE1, 0x8D, 0xA2, 0xEC, 0x99, 0xAD, 0xF6, 0xA4, 0xB8, 0xFF, 0xB0, 0xC3,
  0x47, 0x00, 0x3E, 0x55, 0x00, 0x4D, 0x63, 0x00, 0x5B, 0x70, 0x00, 0x68, 0x7C, 0x19, 0x75, 0x88, 0x2B, 0x81, 0x94, 0x3B, 0x8D, 0xA0, 0x4A, 0x99, 0xAB, 0x58, 0xA4, 0xB6, 0x66, 0xB0, 0xC1, 0x73, 0xBB, 0xCC, 0x7F, 0xC5, 0xD6, 0x8B, 0xD0, 0xE1,
  0x97, 0xDB, 0xEB, 0xA2, 0xE5, 0xF5, 0xAE, 0xEF, 0x28, 0x00, 0x6E, 0x39, 0x00, 0x7B, 0x48, 0x00, 0x87, 0x56, 0x0F, 0x93, 0x63, 0x23, 0x9E, 0x70, 0x34, 0xAA, 0x7D, 0x43, 0xB5, 0x89, 0x52, 0xC0, 0x95, 0x5F, 0xCA, 0xA0, 0x6C, 0xD5, 0xAC, 0x79,
  0xDF, 0xB7, 0x85, 0xEA, 0xC2, 0x91, 0xF4, 0xCC, 0x9D, 0xFE, 0xD7, 0xA8, 0xFF, 0xE1, 0xB3, 0xFF, 0x00, 0x00, 0x86, 0x0E, 0x00, 0x92, 0x22, 0x11, 0x9E, 0x33, 0x25, 0xA9, 0x43, 0x35, 0xB4, 0x51, 0x45, 0xBF, 0x5F, 0x53, 0xCA, 0x6C, 0x60, 0xD5,
  0x79, 0x6E, 0xDF, 0x85, 0x7A, 0xE9, 0x91, 0x86, 0xF3, 0x9C, 0x92, 0xFD, 0xA8, 0x9E, 0xFF, 0xB3, 0xA9, 0xFF, 0xBE, 0xB4, 0xFF, 0xC9, 0xBF, 0xFF, 0x00, 0x00, 0x86, 0x00, 0x1A, 0x92, 0x00, 0x2C, 0x9E, 0x08, 0x3C, 0xA9, 0x1E, 0x4B, 0xB4, 0x30,
  0x59, 0xBF, 0x3F, 0x66, 0xCA, 0x4E, 0x73, 0xD4, 0x5C, 0x7F, 0xDF, 0x69, 0x8B, 0xE9, 0x76, 0x97, 0xF3, 0x82, 0xA3, 0xFD, 0x8E, 0xAE, 0xFF, 0x9A, 0xB9, 0xFF, 0xA5, 0xC4, 0xFF, 0xB0, 0xCE, 0xFF, 0x00, 0x22, 0x6D, 0x00, 0x33, 0x7A, 0x00, 0x42,
  0x86, 0x00, 0x51, 0x92, 0x00, 0x5E, 0x9E, 0x12, 0x6B, 0xA9, 0x26, 0x78, 0xB4, 0x36, 0x84, 0xBF, 0x45, 0x90, 0xCA, 0x54, 0x9C, 0xD4, 0x61, 0xA7, 0xDF, 0x6E, 0xB3, 0xE9, 0x7B, 0xBD, 0xF3, 0x87, 0xC8, 0xFD, 0x93, 0xD3, 0xFF, 0x9E, 0xDD, 0xFF,
  0x00, 0x34, 0x3D, 0x00, 0x44, 0x4C, 0x00, 0x52, 0x5A, 0x00, 0x60, 0x67, 0x00, 0x6D, 0x74, 0x03, 0x79, 0x80, 0x1B, 0x85, 0x8C, 0x2D, 0x91, 0x98, 0x3D, 0x9D, 0xA4, 0x4B, 0xA8, 0xAF, 0x59, 0xB4, 0xBA, 0x67, 0xBE, 0xC5, 0x74, 0xC9, 0xCF, 0x80,
  0xD4, 0xDA, 0x8C, 0xDE, 0xE4, 0x98, 0xE9, 0xEE, 0x00, 0x3C, 0x00, 0x00, 0x4B, 0x00, 0x00, 0x59, 0x17, 0x00, 0x66, 0x2A, 0x00, 0x73, 0x3A, 0x11, 0x80, 0x49, 0x25, 0x8C, 0x57, 0x36, 0x97, 0x64, 0x45, 0xA3, 0x71, 0x53, 0xAE, 0x7E, 0x61, 0xB9,
  0x8A, 0x6E, 0xC4, 0x96, 0x7A, 0xCF, 0xA1, 0x87, 0xD9, 0xAD, 0x93, 0xE4, 0xB8, 0x9E, 0xEE, 0xC2, 0x00, 0x3A, 0x00, 0x00, 0x48, 0x00, 0x00, 0x57, 0x00, 0x06, 0x64, 0x00, 0x1D, 0x71, 0x00, 0x2F, 0x7D, 0x00, 0x3E, 0x8A, 0x18, 0x4D, 0x95, 0x2B,
  0x5B, 0xA1, 0x3B, 0x68, 0xAC, 0x4A, 0x75, 0xB7, 0x58, 0x81, 0xC2, 0x65, 0x8D, 0xCD, 0x72, 0x99, 0xD7, 0x7E, 0xA5, 0xE2, 0x8B, 0xB0, 0xEC, 0x96, 0x00, 0x2C, 0x00, 0x0C, 0x3C, 0x00, 0x21, 0x4B, 0x00, 0x32, 0x59, 0x00, 0x42, 0x66, 0x00, 0x50,
  0x73, 0x00, 0x5E, 0x7F, 0x00, 0x6B, 0x8C, 0x00, 0x78, 0x97, 0x06, 0x84, 0xA3, 0x1D, 0x90, 0xAE, 0x2E, 0x9C, 0xB9, 0x3E, 0xA7, 0xC4, 0x4D, 0xB2, 0xCF, 0x5B, 0xBD, 0xD9, 0x68, 0xC8, 0xE3, 0x75, 0x27, 0x14, 0x00, 0x38, 0x27, 0x00, 0x47, 0x38,
  0x00, 0x55, 0x47, 0x00, 0x63, 0x55, 0x00, 0x70, 0x62, 0x00, 0x7C, 0x6F, 0x00, 0x88, 0x7C, 0x00, 0x94, 0x88, 0x00, 0xA0, 0x94, 0x07, 0xAB, 0x9F, 0x1D, 0xB6, 0xAB, 0x2F, 0xC1, 0xB6, 0x3F, 0xCC, 0xC1, 0x4D, 0xD6, 0xCB, 0x5B, 0xE1, 0xD6, 0x68,
  0x46, 0x00, 0x00, 0x54, 0x09, 0x00, 0x62, 0x1F, 0x00, 0x6F, 0x30, 0x00, 0x7C, 0x40, 0x00, 0x88, 0x4E, 0x00, 0x94, 0x5C, 0x00, 0x9F, 0x69, 0x00, 0xAA, 0x76, 0x08, 0xB6, 0x82, 0x1E, 0xC0, 0x8E, 0x30, 0xCB, 0x9A, 0x3F, 0xD6, 0xA6, 0x4E, 0xE0,
  0xB1, 0x5C, 0xEA, 0xBC, 0x69, 0xF5, 0xC7, 0x76
};

char filetmp[256];
  
char *bios_filename     = "5200.rom";
char *bios_filename_alt = "a5200.rom";
int load_os(void) 
{
  FILE *romfile = fopen(bios_filename, "rb");
    
  if (romfile == NULL)
  {
      sprintf(filetmp, "/roms/bios/%s", bios_filename);
      romfile = fopen(filetmp, "rb");
  }
    
  if (romfile == NULL)
  {
      sprintf(filetmp, "/data/bios/%s", bios_filename);
      romfile = fopen(filetmp, "rb");
  }

  if (romfile == NULL)
  {
      romfile = fopen(bios_filename_alt, "rb");
  }
  
  if (romfile == NULL)
  {
      sprintf(filetmp, "/roms/bios/%s", bios_filename_alt);
      romfile = fopen(filetmp, "rb");
  }
    
  if (romfile == NULL)
  {
      sprintf(filetmp, "/data/bios/%s", bios_filename_alt);
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

static u8 bFirstTimeLoad = 1;
void dsLoadGame(char *filename) 
{
    // load card game if ok
    if (Atari800_OpenFile(filename, true, 1, true) != AFILE_ERROR) 
    {   
      // Initialize the virtual console emulation 
      dsShowScreenEmu();
        
      INPUT_Initialise();
        
      // Init palette
      for(u16 index = 0; index < 256; index++) {
        unsigned short r = palette_data[(index * 3) + 0];
        unsigned short g = palette_data[(index * 3) + 1];
        unsigned short b = palette_data[(index * 3) + 2];
        BG_PALETTE[index] = RGB8(r, g, b);
        atari_pal16[index] = index;
      }
      
      if (bFirstTimeLoad)
      {
          bFirstTimeLoad = 0;
          bSoundMute = 1;
          //*aptr = *bptr = 99;
          TIMER2_CR = 0;
          if (isDSiMode())
          {
              TIMER2_DATA = TIMER_FREQ(SOUND_FREQ+20);  // keep this a little faster than our Pokey sound generation 
              irqSet(IRQ_TIMER2, VsoundHandlerDSi);
          }
          else // Older DS uses lower sound quality
          {
              TIMER2_DATA = TIMER_FREQ((SOUND_FREQ/2)+10);  // keep this a little faster than our Pokey sound generation 
              irqSet(IRQ_TIMER2, VsoundHandler);
          }
          TIMER2_CR = TIMER_DIV_1 | TIMER_IRQ_REQ | TIMER_ENABLE;
          irqEnable(IRQ_TIMER2);
      }
        
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
            dsShowScreenEmu();
            VsoundClear();
            unMuteSoon = 5;
            memset(debug, 0x00, sizeof(debug));
            etatEmu = A5200_PLAYGAME;
            atari_frames=0;
            TIMER0_CR=0;
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
            
            if (unMuteSoon)
            {
                if (--unMuteSoon == 0) bSoundMute = false;
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
            key_consol = CONSOL_NONE;
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
                    if ((iTx>70) && (iTx<185) && (iTy>7) && (iTy<50))  // 72,8 -> 182,42 cartridge slot
                    {
                      bSoundMute = true;
                      // Find files in current directory and show it 
                      a52FindFiles();
                      romSel=dsWaitForRom();
                      if (romSel) 
                      { 
                          etatEmu=A5200_PLAYINIT; 
                          dsLoadGame(a5200romlist[ucFicAct].filename); 
                          if (full_speed) dsPrintValue(30,0,0,"FS"); else dsPrintValue(30,0,0,"  ");
                      } else bSoundMute = false;
                    }
                }
                else if (iTy < 130)
                {
                    if ((iTx>211) && (iTx<250) && (iTy>112) && (iTy<130))  { //quit
                      bSoundMute = true;                
                      soundPlaySample(clickNoQuit_wav, SoundFormat_16Bit, clickNoQuit_wav_size, 22050, 127, 64, false, 0);
                      if (dsWaitOnQuit()) etatEmu=A5200_QUITSTDS;
                      else { unMuteSoon = 5;}
                    }
                    else if ((iTx>160) && (iTx<200) && (iTy>112) && (iTy<130))  { //highscore
                      bSoundMute = true;
                      highscore_display();
                      restore_bottom_screen();
                      unMuteSoon = 5;
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
                              if (iTy < 169) key_code = padKeySR[0 + (iTx / 42)];
                              else           key_code = padKeySR[6 + (iTx / 42)];
                              last_key_code = key_code;
                              keys_dampen = 25; // Almost a half second for consistent debounce
                          }
                          if (keys_dampen)
                          {
                              key_code = last_key_code;
                              if (--keys_dampen < 5) last_key_code=0x00;
                          }
                          else
                          {
                              last_key_code = 0x00;
                              keys_touch = 0;
                          }
                        }
                    }
                    else
                    {
                        if ((iTy>150) && (iTy<185))  // This is our 5200 Keypad 0-9,#,*
                        { 
                          if (!keys_dampen && (keys_touch==0)) // First time pressed?
                          {
                              keys_touch = 1;
                              soundPlaySample(clickNoQuit_wav, SoundFormat_16Bit, clickNoQuit_wav_size, 22050, 127, 64, false, 0);
                              if (iTx > 0) iTx--;
                              key_code = padKey[iTx / 21] | key_code;
                              last_key_code = key_code;
                              keys_dampen = 20; // One-third of a second for consistent debounce
                          }
                          
                          if (keys_dampen)
                          {
                              key_code = last_key_code;
                              if (--keys_dampen < 5) last_key_code=0x00;
                          }
                          else
                          {
                              last_key_code = 0x00;
                              keys_touch = 0;
                          }                          
                        }
                    }
                }
            }
            else
            {
                if (keys_dampen)
                {
                    key_code = last_key_code;
                    if (--keys_dampen < 5) last_key_code=0x00;
                }
                else
                {
                    last_key_code = 0x00;
                    keys_touch = 0;
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
                    else if (myCart.x_function == X_PAND2)
                    {
                        screen_slide_y = -16;  dampen_slide_y = 6;
                    }
                }
            }

            break;
        }
    }
}

void _putchar(char character) {};   // Not used but needed to link printf()
