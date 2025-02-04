#ifndef _A5200UTILS_H
#define _A5200UTILS_H

#define A5200_MENUINIT 0x01
#define A5200_MENUSHOW 0x02
#define A5200_PLAYINIT 0x03 
#define A5200_PLAYGAME 0x04 
#define A5200_QUITSTDS 0x05

extern unsigned short int etatEmu;

typedef enum {
  EMUARM7_INIT_SND = 0x123C,
  EMUARM7_STOP_SND = 0x123D,
  EMUARM7_PLAY_SND = 0x123E,
} FifoMesType;

typedef struct FICtoLoad {
  char filename[254];
  u8 directory;
  unsigned int uCrc;
} FICA5200;

#define ds_GetTicks() (TIMER0_DATA)

extern int bg0, bg1, bg0b,bg1b, bg2, bg3;
extern unsigned int video_height;           // Actual video height
extern unsigned int gameCRC;                // crc checksum of file  

extern void FadeToColor(unsigned char ucSens, unsigned short ucBG, unsigned char ucScr, unsigned char valEnd, unsigned char uWait);

extern void vblankIntr();

extern void dsInitScreenMain(void);
extern void dsInitTimer(void);
extern void dsShowScreenEmu(void);
extern void dsShowScreenMain(void);
extern void dsFreeEmu(void);
extern void VsoundHandler(void);
extern void VsoundHandlerDSi(void);
extern void dsLoadGame(char *filename);
extern unsigned int dsReadPad(void);
extern bool dsWaitOnQuit(void);
extern void dsDisplayFiles(unsigned int NoDebGame,u32 ucSel);
extern unsigned int dsWaitForRom(void);
extern unsigned int dsWaitOnMenu(unsigned int actState);
extern void dsPrintValue(int x, int y, unsigned int isSelect, char *pchStr);
extern void dsInstallSoundEmuFIFO(void);
extern void dsMainLoop(void);
extern int a52Filescmp (const void *c1, const void *c2);
extern void a52FindFiles(void);

#endif
