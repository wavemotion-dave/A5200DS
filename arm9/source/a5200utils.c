#include <nds.h>
#include <nds/fifomessages.h>

#include <stdio.h>
#include <fat.h>
#include <dirent.h>
#include <unistd.h>

#include "main.h"
#include "a5200utils.h"

#include "atari.h"
#include "global.h"
#include "cartridge.h"
#include "input.h"
#include "emu/pia.h"

#include "clickNoQuit_wav.h"
#include "bgBottom.h"
#include "bgTop.h"
#include "bgFileSel.h"
#include "bgCardSel.h"

#include "altirra_5200_os.h"

FICA5200 a5200romlist[1024];  
unsigned int counta5200=0, countfiles=0, ucFicAct=0;
int gTotalAtariFrames = 0;
int bg0, bg1, bg0b,bg1b;
unsigned int etatEmu;

void dsShowAnalog(unsigned int showit);

unsigned char bufVideo[512*512];        // Video buffer
gamecfg GameConf;                        // Game Config svg

#define  cxBG (myCart.offset_x<<8)
#define  cyBG (myCart.offset_y<<8)
#define  xdxBG (((320 / myCart.scale_x) << 8) | (320 % myCart.scale_x))
#define  ydyBG (((256 / myCart.scale_y) << 8) | (256 % myCart.scale_y))
  
unsigned int atari_pal16[256] = {0};
unsigned char *filebuffer;

#define SNDLENGTH 4096
signed char sound_buffer[SNDLENGTH];
signed char *psound_buffer;


#define MAX_DEBUG 5
int debug[MAX_DEBUG]={0};
//#define DEBUG_DUMP

static void DumpDebugData(void)
{
#ifdef DEBUG_DUMP
    char dbgbuf[32];
    for (int i=0; i<MAX_DEBUG; i++)
    {
        int idx=0;
        int val = debug[i];
        if (val < 0)
        {
            dbgbuf[idx++] = '-';
            val = val * -1;
        }
        else
        {
            dbgbuf[idx++] = '0' + (int)val/10000000;
        }
        val = val % 10000000;
        dbgbuf[idx++] = '0' + (int)val/1000000;
        val = val % 1000000;
        dbgbuf[idx++] = '0' + (int)val/100000;
        val = val % 100000;
        dbgbuf[idx++] = '0' + (int)val/10000;
        val = val % 10000;
        dbgbuf[idx++] = '0' + (int)val/1000;
        val= val % 1000;
        dbgbuf[idx++] = '0' + (int)val/100;
        val = val % 100;
        dbgbuf[idx++] = '0' + (int)val/10;
        dbgbuf[idx++] = '0' + (int)val%10;
        dbgbuf[idx++] = 0;
        dsPrintValue(0,3+i,0, dbgbuf);
    }
#endif
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

#define DO1(buf) crc = crc_table[((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
#define DO2(buf)  DO1(buf); DO1(buf);
#define DO4(buf)  DO2(buf); DO2(buf);
#define DO8(buf)  DO4(buf); DO4(buf);
// Table of CRC-32's of all single-byte values (made by make_crc_table)
unsigned int crc_table[256] = {
  0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
  0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
  0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
  0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
  0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
  0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
  0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
  0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
  0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
  0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
  0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
  0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
  0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
  0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
  0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
  0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
  0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
  0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
  0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
  0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
  0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
  0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
  0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
  0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
  0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
  0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
  0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
  0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
  0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
  0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
  0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
  0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
  0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
  0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
  0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
  0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
  0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
  0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
  0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
  0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
  0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
  0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
  0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
  0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
  0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
  0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
  0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
  0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
  0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
  0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
  0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
  0x2d02ef8dL
};

// cal crc32 of a file
unsigned int crc32 (unsigned int crc, const unsigned char *buf, unsigned int len) {
  if (buf == 0) return 0;
  crc = crc ^ 0xffffffff;
  while (len >= 8) {
    DO8(buf);
    len -= 8;
  }
  if (len) do {
    DO1(buf);
  } while (--len);
  return crc ^ 0xffffffff;
}

void vblankIntr() 
{
  static int sIndex = 0;
#if 0
  static const u16 jitter4[] = {
    0x60, 0x40,		// 0.375, 0.250 
    0x20, 0xc0,		// 0.125, 0.750
    0xe0, 0x40,		// 0.875, 0.250
    0xa0, 0xc0,		// 0.625, 0.750
  };
#else
  static const u16 jitter4[] = {
    0x40, 0x40,		// 0.375, 0.250 
    0x20, 0xc0,		// 0.125, 0.750
    0x60, 0x40,		// 0.875, 0.250
    0xa0, 0xc0,		// 0.625, 0.750
  };
#endif  
  REG_BG2PA = xdxBG ; REG_BG2PB = 0; REG_BG2PC =0; REG_BG2PD = ydyBG; 
  REG_BG3PA = xdxBG;  REG_BG3PB = 0; REG_BG3PC =0; REG_BG3PD = ydyBG; 
  
  REG_BG2X = cxBG+jitter4[sIndex++]; 
  REG_BG2Y = cyBG+jitter4[sIndex++]; 
  REG_BG3X = cxBG+jitter4[sIndex++]; 
  REG_BG3Y = cyBG+jitter4[sIndex++]; 
  if(sIndex >= 8) sIndex = 0;
  
  debug[0] = myCart.offset_x;
  debug[1] = myCart.offset_y;
}

void dsInitScreenMain(void) {
  // Init vbl and hbl func
	SetYtrigger(190); //trigger 2 lines before vsync
	irqSet(IRQ_VBLANK, vblankIntr);
  irqEnable(IRQ_VBLANK | IRQ_VCOUNT);
}

void dsInitTimer(void) {
  TIMER0_DATA=0;
	TIMER0_CR=TIMER_ENABLE|TIMER_DIV_1024; 
}

void dsShowScreenEmu(void) {
  // Change vram
	videoSetMode(MODE_5_2D | DISPLAY_BG2_ACTIVE | DISPLAY_BG3_ACTIVE);
  vramSetBankA(VRAM_A_MAIN_BG_0x06000000);
  vramSetBankB(VRAM_B_MAIN_BG_0x06020000 );
  bg0 = bgInit(3, BgType_Bmp8, BgSize_B8_512x512, 0,0);
  bg1 = bgInit(2, BgType_Bmp8, BgSize_B8_512x512, 0,0);

  REG_BLDCNT = BLEND_ALPHA | BLEND_SRC_BG2 | BLEND_DST_BG3;
  REG_BLDALPHA = (8 << 8) | 8; // 50% / 50% 

  //bufVideo = BG_GFX;   
  REG_BG2PB = 0;
  REG_BG2PC = 0;
  REG_BG3PB = 0;
  REG_BG3PC = 0;

  REG_BG2X = cxBG; 
  REG_BG2Y = cyBG; 
  REG_BG3X = cxBG; 
  REG_BG3Y = cyBG; 
  REG_BG2PA = xdxBG; 
  REG_BG2PD = ydyBG; 
  REG_BG3PA = xdxBG; 
  REG_BG3PD = ydyBG; 
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

  decompress(bgBottomTiles, bgGetGfxPtr(bg0b), LZ77Vram);
  decompress(bgBottomMap, (void*) bgGetMapPtr(bg0b), LZ77Vram);
  dmaCopy((void *) bgBottomPal,(u16*) BG_PALETTE_SUB,256*2);
  unsigned short dmaVal = *(bgGetMapPtr(bg1b) +31*32);
  dmaFillWords(dmaVal | (dmaVal<<16),(void*) bgGetMapPtr(bg1b),32*24*2);

  REG_BLDCNT=0; REG_BLDCNT_SUB=0; REG_BLDY=0; REG_BLDY_SUB=0;
  
  dsShowAnalog(myCart.use_analog);

  swiWaitForVBlank();
}

void dsFreeEmu(void) {
  // Stop timer of sound
  TIMER2_CR=0; irqDisable(IRQ_TIMER2); 
}

u16 targetIndex = 0;
void VsoundHandler(void) 
{
  extern char pokey_buffer[];
  static u16 sound_idx=0;
  sound_buffer[sound_idx] = pokey_buffer[targetIndex] +128;
  sound_idx = (sound_idx + 1) & 0x0FFF;
  targetIndex=(targetIndex + 1) % 368;  
}

const unsigned int palette_ntsc[256] = {
  0x000000, 0x1c1c1c, 0x393939, 0x595959,           // 1
  0x797979, 0x929292, 0xababab, 0xbcbcbc, 
  0xcdcdcd, 0xd9d9d9, 0xe6e6e6, 0xececec, 
  0xf2f2f2, 0xf8f8f8, 0xffffff, 0xffffff, 
    
  0x391701, 0x5e2304, 0x833008, 0xa54716,           // 2
  0xc85f24, 0xe37820, 0xff911d, 0xffab1d, 
  0xffc51d, 0xffce34, 0xffd84c, 0xffe651, 
  0xfff456, 0xfff977, 0xffff98, 0xffff98, 
    
  0x451904, 0x721e11, 0x9f241e, 0xb33a20,           // 3
  0xc85122, 0xe36920, 0xff811e, 0xff8c25, 
  0xff982c, 0xffae38, 0xffc545, 0xffc559, 
  0xffc66d, 0xffd587, 0xffe4a1, 0xffe4a1, 
    
  0x4a1704, 0x7e1a0d, 0xb21d17, 0xc82119,           // 4
  0xdf251c, 0xec3b38, 0xfa5255, 0xfc6161, 
  0xff706e, 0xff7f7e, 0xff8f8f, 0xff9d9e, 
  0xffabad, 0xffb9bd, 0xffc7ce, 0xffc7ce, 

  0x050568, 0x3b136d, 0x912640, 0x912640,           // 5
  0xa532a6, 0xb938ba, 0xcd3ecf, 0xdb47dd, 
  0xea51eb, 0xf45ff5, 0xfe6dff, 0xfe7afd, 
  0xff87fb, 0xff95fd, 0xffa4ff, 0xffa4ff, 

  0x280479, 0x400984, 0x590f90, 0x70249d,           // 6
  0x8839aa, 0xa441c3, 0xc04adc, 0xd054ed, 
  0xe05eff, 0xe96dff, 0xf27cff, 0xf88aff, 
  0xff98ff, 0xfea1ff, 0xfeabff, 0xfeabff, 
    
  0x35088a, 0x420aad, 0x500cd0, 0x6428d0,           // 7
  0x7945d0, 0x8d4bd4, 0xa251d9, 0xb058ec, 
  0xbe60ff, 0xc56bff, 0xcc77ff, 0xd183ff, 
  0xd790ff, 0xdb9dff, 0xdfaaff, 0xdfaaff, 
    
  0x051e81, 0x0626a5, 0x082fca, 0x263dd4,           // 8
  0x444cde, 0x4f5aee, 0x5a68ff, 0x6575ff, 
  0x7183ff, 0x8091ff, 0x90a0ff, 0x97a9ff, 
  0x9fb2ff, 0xafbeff, 0xc0cbff, 0xc0cbff, 
    
  0x0c048b, 0x2218a0, 0x382db5, 0x483ec7,           // 9
  0x584fda, 0x6159ec, 0x6b64ff, 0x7a74ff, 
  0x8a84ff, 0x918eff, 0x9998ff, 0xa5a3ff, 
  0xb1aeff, 0xb8b8ff, 0xc0c2ff, 0xc0c2ff, 
    
  0x1d295a, 0x1d3876, 0x1d4892, 0x1c5cac,           // 10
  0x1c71c6, 0x3286cf, 0x489bd9, 0x4ea8ec, 
  0x55b6ff, 0x70c7ff, 0x8cd8ff, 0x93dbff, 
  0x9bdfff, 0xafe4ff, 0xc3e9ff, 0xc3e9ff, 
    
  0x2f4302, 0x395202, 0x446103, 0x417a12,           // 11
  0x3e9421, 0x4a9f2e, 0x57ab3b, 0x5cbd55, 
  0x61d070, 0x69e27a, 0x72f584, 0x7cfa8d, 
  0x87ff97, 0x9affa6, 0xadffb6, 0xadffb6, 
    
  0x0a4108, 0x0d540a, 0x10680d, 0x137d0f,           // 12
  0x169212, 0x19a514, 0x1cb917, 0x1ec919, 
  0x21d91b, 0x47e42d, 0x6ef040, 0x78f74d, 
  0x83ff5b, 0x9aff7a, 0xb2ff9a, 0xb2ff9a, 
    
  0x04410b, 0x05530e, 0x066611, 0x077714,           // 13
  0x088817, 0x099b1a, 0x0baf1d, 0x48c41f, 
  0x86d922, 0x8fe924, 0x99f927, 0xa8fc41, 
  0xb7ff5b, 0xc9ff6e, 0xdcff81, 0xdcff81, 
    
  0x02350f, 0x073f15, 0x0c4a1c, 0x2d5f1e,           // 14
  0x4f7420, 0x598324, 0x649228, 0x82a12e, 
  0xa1b034, 0xa9c13a, 0xb2d241, 0xc4d945, 
  0xd6e149, 0xe4f04e, 0xf2ff53, 0xf2ff53, 
    
  0x263001, 0x243803, 0x234005, 0x51541b,           // 15
  0x806931, 0x978135, 0xaf993a, 0xc2a73e, 
  0xd5b543, 0xdbc03d, 0xe1cb38, 0xe2d836, 
  0xe3e534, 0xeff258, 0xfbff7d, 0xfbff7d, 
    
  0x401a02, 0x581f05, 0x702408, 0x8d3a13,           // 16
  0xab511f, 0xb56427, 0xbf7730, 0xd0853a, 
  0xe19344, 0xeda04e, 0xf9ad58, 0xfcb75c, 
  0xffc160, 0xffc671, 0xffcb83, 0xffcb83
};


int load_os(char *filename ) 
{
  FILE *romfile = fopen(filename, "rb");
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

void dsLoadGame(char *filename) {
  unsigned int index;
  
  // Free buffer if needed
  TIMER2_CR=0; irqDisable(IRQ_TIMER2); 
	if (filebuffer != 0)
		free(filebuffer);
  
    // load card game if ok
    if (Atari800_OpenFile(filename, true, 1, true) != AFILE_ERROR) 
    {	
      // Initialize the virtual console emulation 
      dsShowScreenEmu();
        
      INPUT_Initialise();

      // Init palette
      for(index = 0; index < 256; index++) {
        unsigned short r = ((palette_ntsc[index]& 0x00ff0000) >> 19);
        unsigned short g = ((palette_ntsc[index]& 0x0000ff00) >> 11);
        unsigned short b = ((palette_ntsc[index]& 0x000000ff) >> 3);
        BG_PALETTE[index] = RGB15(r, g, b);
        atari_pal16[index] = index;
      }

      psound_buffer=sound_buffer;
      TIMER2_DATA = TIMER_FREQ(22050);                        
	    TIMER2_CR = TIMER_DIV_1 | TIMER_IRQ_REQ | TIMER_ENABLE;	     
	    irqSet(IRQ_TIMER2, VsoundHandler);                           
    }
}

unsigned int dsReadPad(void) {
	unsigned int keys_pressed, ret_keys_pressed;

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
  bool bRet=false, bDone=false;
  unsigned int keys_pressed;
  unsigned int posdeb=0;
  char szName[32];
  
  decompress(bgFileSelTiles, bgGetGfxPtr(bg0b), LZ77Vram);
  decompress(bgFileSelMap, (void*) bgGetMapPtr(bg0b), LZ77Vram);
  dmaCopy((void *) bgFileSelPal,(u16*) BG_PALETTE_SUB,256*2);
  unsigned short dmaVal = *(bgGetMapPtr(bg1b) +31*32);
  dmaFillWords(dmaVal | (dmaVal<<16),(void*) bgGetMapPtr(bg1b),32*24*2);
  
  strcpy(szName,"Quit A5200DS ?");
  dsPrintValue(17,2,0,szName);
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

  decompress(bgBottomTiles, bgGetGfxPtr(bg0b), LZ77Vram);
  decompress(bgBottomMap, (void*) bgGetMapPtr(bg0b), LZ77Vram);
  dmaCopy((void *) bgBottomPal,(u16*) BG_PALETTE_SUB,256*2);
  dmaVal = *(bgGetMapPtr(bg1b) +31*32);
  dmaFillWords(dmaVal | (dmaVal<<16),(void*) bgGetMapPtr(bg1b),32*24*2);  

  return bRet;
}


void dsDisplayFiles(unsigned int NoDebGame,u32 ucSel) {
  unsigned int ucBcl,ucGame;
  u8 maxLen;
  char szName[256];
  
  // Display all games if possible
  unsigned short dmaVal = *(bgGetMapPtr(bg1b) +31*32);
  dmaFillWords(dmaVal | (dmaVal<<16),(void*) (bgGetMapPtr(bg1b)),32*24*2);
  countfiles ? sprintf(szName,"%04d/%04d GAMES",(int)(1+ucSel+NoDebGame),countfiles) : sprintf(szName,"%04d/%04d FOLDERS",(int)(1+ucSel+NoDebGame),counta5200);
  dsPrintValue(16-strlen(szName)/2,3,0,szName);
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
        a5200romlist[ucGame].filename[29] = 0;
        sprintf(szName,"[%s]",a5200romlist[ucGame].filename);
        sprintf(szName2,"%-29s",szName);
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

unsigned int dsWaitForRom(void) {
  bool bDone=false, bRet=false;
  u32 ucHaut=0x00, ucBas=0x00,ucSHaut=0x00, ucSBas=0x00,romSelected= 0, firstRomDisplay=0,nbRomPerPage, uNbRSPage, uLenFic=0,ucFlip=0, ucFlop=0;
  char szName[64];

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
    }
    else {
      ucBas = 0;
    }  
    if (keysCurrent() & KEY_R) {
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
    if (keysCurrent() & KEY_L) {
      if (!ucSHaut) {
        ucFicAct = (ucFicAct> nbRomPerPage ? ucFicAct-nbRomPerPage : 0);
        if (firstRomDisplay>nbRomPerPage) { firstRomDisplay -= nbRomPerPage; }
        else { firstRomDisplay = 0; }
        if (ucFicAct == 0) romSelected = 0;
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

    if (keysCurrent() & KEY_A) {
      if (!a5200romlist[ucFicAct].directory) {
        bRet=true;
        bDone=true;
      }
      else {
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
    // Scroll la selection courante
    if (strlen(a5200romlist[ucFicAct].filename) > 29) {
      ucFlip++;
      if (ucFlip >= 8) {
        ucFlip = 0;
        uLenFic++;
        if ((uLenFic+29)>strlen(a5200romlist[ucFicAct].filename)) {
          ucFlop++;
          if (ucFlop >= 8) {
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
  
  decompress(bgBottomTiles, bgGetGfxPtr(bg0b), LZ77Vram);
  decompress(bgBottomMap, (void*) bgGetMapPtr(bg0b), LZ77Vram);
  dmaCopy((void *) bgBottomPal,(u16*) BG_PALETTE_SUB,256*2);
  dmaVal = *(bgGetMapPtr(bg1b) +31*32);
  dmaFillWords(dmaVal | (dmaVal<<16),(void*) bgGetMapPtr(bg1b),32*24*2);
  
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

void dsShowAnalog(unsigned int showit) {
  u16 *pusEcran=(u16*) (bgGetMapPtr(bg1b))+26+(18<<5);
  u16 *pusDess =(u16*) (bgGetMapPtr(bg0b))+(4*showit)+(28<<5);

  dmaCopy((pusDess),        (pusEcran),4*2);
  dmaCopy((pusDess+(1<<5)),(pusEcran+(1<<5)),4*2);
  dmaCopy((pusDess+(2<<5)),(pusEcran+(2<<5)),4*2);
  dmaCopy((pusDess+(3<<5)),(pusEcran+(3<<5)),4*2);
}

//---------------------------------------------------------------------------------
void dsInstallSoundEmuFIFO(void) {
	FifoMessage msg;
  msg.SoundPlay.data = &sound_buffer;
  msg.SoundPlay.freq = 22050;
	msg.SoundPlay.volume = 127;
	msg.SoundPlay.pan = 64;
	msg.SoundPlay.loop = 1;
	msg.SoundPlay.format = ((1)<<4) | SoundFormat_8Bit;
  msg.SoundPlay.loopPoint = 0;
  msg.SoundPlay.dataSize = SNDLENGTH >> 2;
  msg.type = EMUARM7_PLAY_SND;
  fifoSendDatamsg(FIFO_USER_01, sizeof(msg), (u8*)&msg);
}

extern u32 trig0, trig1;
extern u32 stick0;
extern u32 stick1;
int full_speed = 0;

ITCM_CODE void dsMainLoop(void) {
  char fpsbuf[32];
  unsigned int keys_pressed,keys_touch=0, romSel;
  int iTx,iTy, shiftctrl;
  bool showFps=false;
  int hold_touch = 0;
  
  myCart.use_analog = 0;
  
  // Timers are fed with 33.513982 MHz clock.
  // With DIV_1024 the clock is 32,728.5 ticks per sec...
  TIMER0_DATA=0;
  TIMER0_CR=TIMER_ENABLE|TIMER_DIV_1024;
  TIMER1_DATA=0;
  TIMER1_CR=TIMER_ENABLE | TIMER_DIV_1024;  
  
  while(etatEmu != A5200_QUITSTDS) {
    switch (etatEmu) {
    
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
        dsShowAnalog(myCart.use_analog);
        irqEnable(IRQ_TIMER2);  
        etatEmu = A5200_PLAYGAME;
        break;
        
      case A5200_PLAYGAME:
        // 65535 = 1 frame
        // 1 frame = 1/50 ou 1/60 (0.02 ou 0.016 
        // 656 -> 50 fps et 546 -> 60 fps
        if (!full_speed)
        {
          while(TIMER0_DATA < 546)
              ;
        }
        TIMER0_CR=0;
        TIMER0_DATA=0;
        TIMER0_CR=TIMER_ENABLE|TIMER_DIV_1024;

        // Execute one frame
        Atari800_Frame(1);
        
        // -------------------------------------------------------------
        // Stuff to do once/second such as FPS display and Debug Data
        // -------------------------------------------------------------
        if (TIMER1_DATA >= 32728)   // 1000MS (1 sec)
        {
            TIMER1_CR = 0;
            TIMER1_DATA = 0;
            TIMER1_CR=TIMER_ENABLE | TIMER_DIV_1024;
        
            if (showFps) { siprintf(fpsbuf,"%03d",gTotalAtariFrames); dsPrintValue(0,0,0, fpsbuf); } // Show FPS
            DumpDebugData();
            gTotalAtariFrames = 0;
        }
        
        if (hold_touch > 0)
        {
           hold_touch--;
           continue;
        }
        // Read keys
        keys_pressed=keysCurrent();
        key_consol = CONSOL_NONE; //|= (CONSOL_OPTION | CONSOL_SELECT | CONSOL_START); /* OPTION/START/SELECT key OFF */
        shiftctrl = 0; key_shift = 0;
        trig0 = (keys_pressed & KEY_A) ? 0 : 1;
        stick0 = STICK_CENTRE;
        stick1 = STICK_CENTRE;
                  
        if (keys_pressed & KEY_B) { shiftctrl ^= AKEY_SHFT; key_shift = 1; }
        key_code = shiftctrl ? 0x40 : 0x00;
        
        // if touch screen pressed
        if (keys_pressed & KEY_TOUCH) {
          if (!keys_touch) {
            touchPosition touch;
            keys_touch=1;
            touchRead(&touch);
            iTx = touch.px;
            iTy = touch.py;
            if ((iTx>206) && (iTx<250) && (iTy>110) && (iTy<129))  { // 207,111  -> 249,128   quit
              irqDisable(IRQ_TIMER2); fifoSendValue32(FIFO_USER_01,(1<<16) | (0) | SOUND_SET_VOLUME);
              soundPlaySample(clickNoQuit_wav, SoundFormat_16Bit, clickNoQuit_wav_size, 22050, 127, 64, false, 0);
              if (dsWaitOnQuit()) etatEmu=A5200_QUITSTDS;
              else { irqEnable(IRQ_TIMER2); fifoSendValue32(FIFO_USER_01,(1<<16) | (127) | SOUND_SET_VOLUME); }
            }
            else if ((iTx>27) && (iTx<50) && (iTy>111) && (iTy<122))  { // 28,112  -> 49,121   pause
              soundPlaySample(clickNoQuit_wav, SoundFormat_16Bit, clickNoQuit_wav_size, 22050, 127, 64, false, 0);
              key_code = AKEY_5200_PAUSE + key_code;
              hold_touch = 5;
            }
            else if ((iTx>16) && (iTx<39) && (iTy>122) && (iTy<133))  { // 17,123  -> 38,132   reset
              soundPlaySample(clickNoQuit_wav, SoundFormat_16Bit, clickNoQuit_wav_size, 22050, 127, 64, false, 0);
              key_code = AKEY_5200_RESET + key_code;
              hold_touch = 5;
            }
            else if ((iTx>4) && (iTx<27) && (iTy>111) && (iTy<122))  { // 5,112  -> 26,121   start
              soundPlaySample(clickNoQuit_wav, SoundFormat_16Bit, clickNoQuit_wav_size, 22050, 127, 64, false, 0);
              key_code = AKEY_5200_START + key_code;
              hold_touch = 5;
            }
            else if ((iTx>9) && (iTx<44) && (iTy>135) && (iTy<181)) {     // 10,136 -> 43,180 numeric pad
              char padKey[] = {AKEY_5200_1,AKEY_5200_2,AKEY_5200_3,AKEY_5200_4,AKEY_5200_5,AKEY_5200_6,AKEY_5200_7,AKEY_5200_8,AKEY_5200_9,AKEY_5200_ASTERISK,AKEY_5200_0,AKEY_5200_HASH};
              iTx = iTx-10; iTy = iTy-136; iTx = iTx / 11; iTy = iTy / 11;
              iTx = iTx + iTy *3;
              if ((iTx != 9) && (iTx != 11)) {
                key_code = padKey[iTx] + key_code;
              }
              else
                key_code = padKey[iTx];
              hold_touch = 5;
            }
            else if ((iTx>71) && (iTx<183) && (iTy>7) && (iTy<43)) {     // 72,8 -> 182,42 cartridge slot
              irqDisable(IRQ_TIMER2); fifoSendValue32(FIFO_USER_01,(1<<16) | (0) | SOUND_SET_VOLUME);
              // Find files in current directory and show it 
              a52FindFiles();
              romSel=dsWaitForRom();
              if (romSel) { etatEmu=A5200_PLAYINIT; dsLoadGame(a5200romlist[ucFicAct].filename); }
              else { irqEnable(IRQ_TIMER2); }
              fifoSendValue32(FIFO_USER_01,(1<<16) | (127) | SOUND_SET_VOLUME);
              dsShowAnalog(myCart.use_analog);
            }
            else if ((iTx>207) && (iTx<249) && (iTy>143) && (iTy<184)) {     // 208,144 -> 248,183 simulate analog
              soundPlaySample(clickNoQuit_wav, SoundFormat_16Bit, clickNoQuit_wav_size, 22050, 127, 64, false, 0);
              myCart.use_analog = 1 - myCart.use_analog;
              dsShowAnalog(myCart.use_analog);
            }
          }
        }
        // else manage a5200 pad 
        else 
        {
          keys_touch=0;
        }
      
        if (myCart.control == CTRL_JOY)
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
        if (keys_pressed & KEY_R) key_code = AKEY_5200_ASTERISK;
        if (keys_pressed & KEY_L) key_code = AKEY_5200_HASH;
        
        static int last_keys = 99;
        if (keys_pressed != last_keys)
        {
          last_keys = keys_pressed;
          if (myCart.control != CTRL_ROBO)          
          {
            if (keys_pressed & KEY_X) {showFps = 1-showFps;dsPrintValue(0,0,0, "   ");}
            if (keys_pressed & KEY_Y) {full_speed = 1-full_speed; if (full_speed) dsPrintValue(30,0,0, "FS"); else dsPrintValue(30,0,0, "  ");}
          }
          else 
          {
            //if (keys_pressed & KEY_R) myCart.offset_y++;
            //if (keys_pressed & KEY_L) myCart.offset_y--;
          }
        }

        break;
    }
	}
}

//----------------------------------------------------------------------------------
// Find files (a78 / bin) available
int a52Filescmp (const void *c1, const void *c2) {
  FICA5200 *p1 = (FICA5200 *) c1;
  FICA5200 *p2 = (FICA5200 *) c2;
  
  return strcmp (p1->filename, p2->filename);
}

void a52FindFiles(void) {
	struct stat statbuf;
  DIR *pdir;
  struct dirent *pent;
  char filenametmp[255];
  
  counta5200 = countfiles= 0;
  
  pdir = opendir(".");

  if (pdir) {

    while (((pent=readdir(pdir))!=NULL)) {
      stat(pent->d_name,&statbuf);

      strcpy(filenametmp,pent->d_name);
      if(S_ISDIR(statbuf.st_mode)) {
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
    qsort (a5200romlist, counta5200, sizeof (FICA5200), a52Filescmp);
}
