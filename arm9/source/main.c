#include <stdio.h>
#include <fat.h>
#include <nds.h>

#include <unistd.h>

#include "main.h"
#include "intro.h"
#include "atari.h"
#include "a5200utils.h"
#include "highscore.h"

extern int bg0, bg1;

extern int load_os(char *filename );

char *bios_filename = "5200.rom";

// Program entry point
int main(int argc, char **argv) 
{
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
