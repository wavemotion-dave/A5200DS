#include <stdio.h>
#include <fat.h>
#include <nds.h>

#include <unistd.h>

#include "main.h"
#include "intro.h"
#include "atari.h"
#include "a5200utils.h"

extern int bg0, bg1;
int bg0s, bg1s, bg2s, bg3s;         // sub BG pointers 
extern volatile u16 vusCptVBL;      // VBL test

extern int load_os(char *filename );

void irqVBlank(void) { 
  // Manage time
  vusCptVBL++;
}

extern int frame_skip;
// Program entry point
int main(int argc, char **argv) 
{
  char *bios_filename = "5200.rom";
  // Init sound
  consoleDemoInit();
  soundEnable();
  lcdMainOnTop();

  frame_skip = (isDSiMode() ? FALSE:TRUE);   // For older DS models, we skip frames to get full speed...
  // Init Fat
	if (!fatInitDefault()) {
		iprintf("Unable to initialize libfat!\n");
		return -1;
	}

  // Init Timer
  dsInitTimer();
  dsInstallSoundEmuFIFO();
  
  if (keysCurrent() & KEY_R)
  {
       bios_filename = "XYZZY.~01"; // Won't be found... Altria bios instead...
  }
  // Intro and main screen
  intro_logo();  
  dsInitScreenMain();
  etatEmu = A5200_MENUINIT;

  // 
  if (!load_os(bios_filename)) 
  {
      //load rom file via args if a rom path is supplied
      if(argc > 1) 
      {
        dsShowScreenMain();
        dsLoadGame(argv[1]);
        Atari800_Initialise();
        etatEmu = A5200_PLAYINIT;
      }
    // Main loop of emulation
    dsMainLoop();
  }
  else {
    dsShowScreenMain();
    dsPrintValue(0,0,0, "Can't find 5200.rom BIOS"); 
    dsReadPad();
  }
  
  // Free memory to be correct 
  dsFreeEmu();
  
  return(0);
}
