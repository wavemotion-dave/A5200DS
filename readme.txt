a5200DS
--------------------------------------------------------------------------------
a5200DS is an Atari VCS 5200 console emulator.
To use this emulator, you must use compatibles rom with a52/bin format. 
You also need the 5200.bin rom bios to use it (there is a built-in open-source
replacement BIOS which will work for many games but doesn't have full compatibilty
so it is still recommended you use 5200.bin). A quick search on atariage website 
will help you about that ;-) Do not ask me about such files, I don't have them. 

Features :
----------
 Most things you should expect from an emulator. 
 Most games run at full speed on a DSi or above.

Missing :
---------
Bounty Bob Strikes Back only runs at about 45FPS... everything else is good.

Check updates on my web site : 
http://www.portabledev.com

--------------------------------------------------------------------------------
History :
--------------------------------------------------------------------------------
V2.1 : 30-Dec-2020 by wavemotion-dave
  * Pokey Emulation buffers completely re-written. It now sounds passably good! 
    You can hear the voices in Berzerk clearly. 'Nuff said. 
  
V2.0 : 30-Dec-2020 by wavemotion-dave
  * Fixed background noise bug on Pokey emulation - all sounds now available in 
    all games (fixes Wizard of Wor sound, Blueprint sound, etc).
  * Tons of minor screen adjustments for various games to make them look as good as possible.
  * More games run properly - Bounty Bob Strikes Back is playable (but not yet full speed)

V1.9 : 27-Dec-2020 by wavemotion-dave
  * New virtual keypad layout. 
  * Improved Analog vs Digital auto-selection. 
  * Improved speed in ANTIC and CPU cores. 
  * Corrected NTSC color palett.
  * More games run properly - Montezuma's Revenge is playable.
  
V1.8 : 25-Dec-2020 by wavemotion-dave
  * Improved collision detection by removing some of the speedup hacks of the past.
  * More games run more correctly - better screen usage and improved control support.
  * Robotron 2084 now works with ABXY buttons as shooting buttons.

V1.7 : 24-Dec-2020 by wavemotion-dave
  * Ressurected from the ashes... Improved cart detection. Better analog support.
    Patched working controls for QBERT, FROGGER and Wizard of Wor
    Better screen handling started
    Now using Altirra-3.20-test4 replacement BIOS if 5200.rom isn't around.

V1.6 : 28/06/2011
  * Add Analog managment with icon on screen, just click on it to activate/desactivate
    -> this will fix pb with lot's of game like breakout, missile command, and so on
    
V1.5 : 26/06/2011
  * Fix bug in GTIA/POKEY for enabling pot management, analog jostick works now
  * Compiled with new devkitpro (r33), win 1 fps since previous version
    
V1.4 : 13/06/2011
  * Add automatic chip detection for 16k roms (but you can change it)
  * Button L to display/hide FPS
  * Add R & L to change Rom
  * Really fix flickering pb, alphalerp is back and screen is smoother
  * Better sound (not so much ...)
  
V1.3 : 12/06/2011
  * Fix pb with iEvo (hangs on menu)
  
V1.2 : 11/06/2011
  * Fix pb with B button (now you can use bombs in HERO)
  * Remove argc/argv management (for Another World test with his ievo) 
  * Fix flickering pb (remove alphalerp for now) 
  
V1.1 : 23/05/2011
  * Quick fix about bios problem
  * change screen height to 256
  * Add more easy understanding message when no game in current diretory (Thx 
     Another World for the tip)
  * Fix name in makefile (was 7800 and not 5200)
   
V1.0 : 22/05/2011
  * Initial release 
  * Compiled with last version of Devkitpro/libnds, so DSi compatible \o/
  
--------------------------------------------------------------------------------
How to use a5200DS :
--------------------------------------------------------------------------------

Put the a52/bin files where you want on your card. 

That's all, a5200DS can be used now :) !

When the emulator starts, click on the cartridge slot to choose a file. you are use Up/Down 
to select a file, then use A to load it.

Controls :
 * Direction pad : the joystick ...
 * A      : Fire button 1
 * B      : Shift button (Fire button 2)
 * X      : FPS Display Toggle
 * Y      : Full Speed Toggle
 * R      : * button (often starts game)
 * L      : # button (often changes game selection)
 * START  : START button
 * SELECT : Pause button
 
 Use stylus on buttons for other actions on bottom screen.
 
--------------------------------------------------------------------------------
Credits:
--------------------------------------------------------------------------------
Thanks Wintermute for devkitpro and libnds (http://www.devkitpro.org).
Atari800 team for source code (http://atari800.sourceforge.net/)
zx81 (http://zx81.zx81.free.fr/serendipity_fr/) for GP2X-Atari version (that helped
  me a lot to understand Atari 5200).
--------------------------------------------------------------------------------
Alekmaul
alekmaul@portabledev.com
http://www.portabledev.com
--------------------------------------------------------------------------------
