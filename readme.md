# a5200ds

a5200ds is an Atari 5200 emulator for the Nintendo DS/DSi/XL/LL

To use this emulator, you must use NTSC compatible roms with a52/bin format. 
You also need the 5200.rom BIOS in the same folder as A5200DS.NDS (or in
/roms/bios or /data/bios) to use this emulator properly (there is a built-in 
open-source replacement BIOS which will work for some games but doesn't have
full compatibility so it is still strongly recommended you use the 4-port 
original 5200.rom BIOS with a CRC32 of 0x4248d3e3). A quick search on google 
will help you about that. Do not ask me about such files - you will be ignored.

![Atari5200](https://github.com/wavemotion-dave/A5200DS/blob/main/arm9/gfx/bgTop.png)

Copyright :
-----------------------
a5200ds is Copyright (c) 2021-2026 Dave Bernazzani (wavemotion-dave)

This program is built upon alekmaul's magnificent efforts from circa 2011. He was 
kind enough to grant me permission to update this emulator for the modern era.

As long as there is no commercial use (i.e. no profit is made), copying and distribution 
of this emulator, its source code and associated readme files, with or without modification, 
are permitted in any medium without royalty provided this copyright notice is used 
and both alekmaul and wavemotion-dave are thanked profusely.

The a5200ds emulator is offered as-is, without any warranty.

Features :
----------
* Support for the original 69 commercial releases plus a couple hundred homebrews, hacks and prototypes.
* All games run at full speed on a DSi or above. Most games will run at full speed on the older DS-Lite/Phat (some of the more complex conversions will lag).
* Bankswitching carts up to 512K are supported. Bounty Bob Strikes Back is supported.
* Favorites handling to mark games as like (yellow heart) or love (red heart) so you can quickly find the games you enjoy most.
* Games like Frogger and Q-Bert auto-patch so the button does not need to be pressed to move/hop.
* Games like Robotron will auto configure twin-sticks so you can use the ABXY buttons to shoot while the d-pad moves.
* 10 high-scores per game is saved to the SD card - track your scores just as you did in the olden days.

Known Issues :
---------
Most things are playing fine on a DSi. Most of the original commercial games (like Missile Command, Space Invaders, Centipede) will generally
run fine on the older DS-LITE or DS-PHAT.  

Quest for Quinta Roo will not start - known key debounce issue with the emulation.

Check updates on my web site : https://github.com/wavemotion-dave/A5200DS

History :
--------------------------------------------------------------------------------
V3.9 : 08-Apr-2026 by wavemotion-dave
  * 60Hz True-Sync. The emulation is now synchronized with the LCD display to avoid tearing (mostly improves scrolling games like River Raid or Vanguard).
  * New Favorites added... in the menu where you select what game you want to play, press SELECT to toggle between like (yellow heart) and love (red heart). These persist so you can always come back in and see your favorite games.
  * Improved fameskip handling that always handles player-missile graphics/collisions. This allows the older DS-Lite (and when running from an R4 card or similar) to run most games at full speed without sacrificing core emulation accuracy (no more glitches where shots miss enemies).  The DSi and above running something like TWL++ never needs frameskip.
  * More games run more correctly - fixed games like Star Raiders so the keypad is less glitchy, improved default controller settings for some games, etc.
  * Numerous tweaks, optimizations and a few specialized hacks that now allow heavy-hitting games to play at or near full speed on the older hand held units.

V3.8 : 02-Feb-2025 by wavemotion-dave
  * Fixed banking so games like Dropzone properly render lower screen graphics (moonscape).
  * Improved 64K banking for games that only sparingly swap banks - these can now use the faster CPU core driver (renders Dropzone and Laser Hawk full speed on older DS-Lite hardware)
  * More CPU optimizations to improve speed slightly.
  * More games are now running full speed on the DS-Lite - Vanguard is now playable!
  * Latest hacks from the A5200 forums on Atariage have been added to the internal database.

V3.7 : 30-Jan-2025 by wavemotion-dave
  * New banking handling so we only move pointers - not blocks of memory. Renders all 5200 Super Banked games playable.
  * New NTSC palette from Trebor's Pro Palette (NTSC_LUM1_HOT 5200).
  * Added 5200 512K Super Cart banking.
  * Improved Pokey emulation (around the random scanline start value - fixes one Acid800 test).
  * Improved keypad debounce for Star Raiders.
  * Improved sound handling to reduce pops/clicks. Increased the sample rate on the DSi for improved fidelity.

V3.6 : 08-Jan-2024 by wavemotion-dave
  * Internal database updated to include known prototypes, hacks and other sundry 5200 games.
  * Codebase cleanup across the board.
  * Improved key debounce to fix lots of games including Star Raiders (which has a new overlay).
  * Color Artifacting added for the few games that utilize it (Choplifter, etc.)

V3.5 : 05-Jan-2024 by wavemotion-dave
  * Major cleanup across the board - a few more frames of performance and many games on the older DS-Lite will now run full speed without frameskip.
  * Touch of paint on the main emulation screen to make the console buttons a bit more prominent.
  * New X button handling on the games that utilize more than 192 vertical pixels to smooth scroll upper/lower areas of the screen (usually just status/score in those areas so pressing the X button to bring them momentarily into view works fine).
  * About 50 new homebrews and hacks added to the internal database (up to 240 games now).
  * Version 3.5a fixes the Star Raiders game and adds a new overlay for it! Fixes for Phobos. A few screen tweaks. Sound pops improved.

V3.4 : 01-May-2023 by wavemotion-dave
  * Another frame or two of performance from memory optmizations.
  * Removed lots of obsoleted code to streamline the build.
  * Added a new DSi special build that will finally run Bosconian at full speed with speech as long as you are in DSi (2x CPU) mode.
  * Added tweaks and support for latest homebrew carts like Rob-N-Banks.
  
V3.3 : 30-Apr-2023 by wavemotion-dave
  * Improved screen blend so it leaves less visible artifacts.
  * Improved sound mute so it eliminates most of the little pops.
  * Built with latest libnds to keep things fresh.
  * v3.3a hotfix for sound pops. Sorry!

V3.2 : 12-Dec-2021 by wavemotion-dave
  * Reverted back to ARM7 SoundLib (a few games missing key sounds)
  
V3.1 : 30-Nov-2021 by wavemotion-dave
  * Switched to maxmod audio library for improved sound.
  * Try to start in /roms or /roms/a5200 if possible

V3.0 : 04-Nov-2021 by wavemotion-dave
  * New sound core to eliminate zingers.
  * 5200.rom bios can be in /roms/bios or /data/bios 

V2.9 : 22-July-2021 by wavemotion-dave
  * High Score support added!
  * Added swap LCD functionality with L+R+A (hold for 1 sec)

V2.8 : 20-Mar-2021 by wavemotion-dave
  * A fresh coat of paint for the logo and file selection.
  * A few minor bugs fixed and cleanup as time permitted.

V2.7 : 10-Feb-2021 by wavemotion-dave
  * Improved FPS timing. 
  * Added screen scaling (using L/R Shoulder Buttons) from XEGS
  * Added Frame Skip for DS-LITE/PHAT to gain speed!

V2.6 : 23-Jan-2021 by wavemotion-dave
  * Squeezed another 2% speedup across the board from memory tweaks. 
  * Added a few more XL conversions to the compatibility list. 

V2.5 : 12-Jan-2021 by wavemotion-dave
  * Hundreds of small pixel tweaks to get all games looking as good as possible.

V2.4 : 09-Jan-2021 by wavemotion-dave
  * More XL Conversions added to the comatibility table.
  * More speed tweaks especially in bank switching rendering virtually all games playable at full speed.

V2.3 : 03-Jan-2021 by wavemotion-dave
  * New and imprmoved DTCM and VRAM usage for faster moving of data around the system... 
    this results in a 10-15% speed improvement and Bounty Bob Strikes Back is finally playable at 60FPS!

V2.2 : 02-Jan-2021 by wavemotion-dave
  * Fixed launching via command line (so TWL++ integration will work now)
  * Added support for 64k Super Carts (Laswer Hawk, Dropzone, MULE, Berks4, etc)
  * Added support for 512k Super Carts (Bosconian... though it runs really slowly)
  * New memory map scheme for slight improvement in stubbon games like Bounty Bob Strikes Back 

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
  
Original AlekMaul Credits:
--------------------------------------------------------------------------------
Thanks Wintermute for devkitpro and libnds (http://www.devkitpro.org).
Atari800 team for source code (http://atari800.sourceforge.net/)
zx81 (http://zx81.zx81.free.fr/serendipity_fr/) for GP2X-Atari version (that helped
  me a lot to understand Atari 5200).

Original Author:
Alekmaul
alekmaul@portabledev.com
http://www.portabledev.com
--------------------------------------------------------------------------------
