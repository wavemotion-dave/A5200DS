#include <stdio.h>
#include <fat.h>
#include <nds.h>
#include <unistd.h>
#include <time.h>

#include "main.h"
#include "intro.h"
#include "atari.h"
#include "a5200utils.h"
#include "highscore.h"

extern int bg0, bg1;

extern int load_os();

// Program entry point
int main(int argc, char **argv) 
{
  extern void MEMORY_InitialiseMap(void);
  MEMORY_InitialiseMap();
  
  // Init sound
  consoleDemoInit();
  soundEnable();
  lcdMainOnTop();

  // Init Fat
	if (!fatInitDefault()) {
		iprintf("Unable to initialize libfat!\n");
		return -1;
	}

  // Init Timer
  dsInitTimer();
  dsInstallSoundEmuFIFO();
  highscore_init();
  
  // Intro and main screen
  intro_logo();  
  dsInitScreenMain();
  etatEmu = A5200_MENUINIT;
  
  srand(time(NULL));

  if (!load_os())  // Should never fail as we have fallback Altira BIOS
  {
      //load rom file via args if a rom path is supplied
      if(argc > 1) 
      {
        dsShowScreenMain();
        dsLoadGame(argv[1]);
        Atari800_Initialise();
        etatEmu = A5200_PLAYINIT;
      }
      else
      {
        chdir("/roms");    // Try to start in roms area... doesn't matter if it fails
        chdir("a5200");    // And try to start in the subdir /a5200... doesn't matter if it fails.
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
