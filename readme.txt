a5200DS
--------------------------------------------------------------------------------
a5200DS is an Atari VCS 5200 console emulator.
To use this emulator, you must use NTSC compatible roms with a52/bin format. 
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
Most things are playing fine on a DSi. Some of the more simple 
games (like Missile Command, Space Invaders, Centipede) will generally
run fine on the older DS-LITE or DS-PHAT.  Bosconian (512k supercart)
is too slow to be playable... 

Check updates on my web site : https://github.com/wavemotion-dave/A5200DS

--------------------------------------------------------------------------------
History :
--------------------------------------------------------------------------------
V2.3 : 03-Jan-2020 by wavemotion-dave
  * New and imprmoved DTCM and VRAM usage for faster moving of data around the system... 
    this results in a 10-15% speed improvement and Bounty Bob Strikes Back is finally playable at 60FPS!

V2.2 : 02-Jan-2020 by wavemotion-dave
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
Supported Games (with MD5SUM so you can find the right rom)
----------------------------------------------------------------------------
 c8e90376b7e1b00dcbd4042f50bffb75     Atari 5200 Calibration Cart    
 45f8841269313736489180c8ec3e9588     Activision Decathlon, The (USA).a52
 4b1aecab0e2f9c90e514cb0a506e3a5f     Adventure II-a.a52
 e2f6085028eb8cf24ad7b50ca4ef640f     Adventure II-b.a52
 737717ff4f8402ed5b02e4bf866bbbe3     ANALOG Multicart (XL Conversion).a52
 77c6b647746bb1413c5566378ef25eec     Archon (XL Conversion).a52
 bae7c1e5eb04e19ef8d0d0b5ce134332     Astro Chase (USA).a52
 d31a3bbb4c99f539f0d2c4e02bec516e     Atlantis (XL Conversion).a52
 ec65389cc604b279d69a889725c723e7     Attack of the Mutant Camels (XL Conversion).a52
 f5cd178cbea0ae7d8cf65b30cfd04225     Ballblazer (USA).a52
 96b424d0bb0339f4edfe8095fe275d62     Batty Builders (XL Conversion).a52
 8123393ae9635f6bc15ddc3380b04328     Blueprint (1982) (CBS).a52    
 17e5c03b4fcada48d4c2529afcfe3a70     BCs Quest For Tires (XL Conversion).a52
 1913310b1e44ad7f3b90aeb16790a850     Beamrider (USA).a52
 f8973db8dc272c2e5eb7b8dbb5c0cc3b     BerZerk (USA).a52
 139229eed18032fdea735fa5360bd551     Beef Drop Ultimate SD Edition.a52
 d9499b29559f8c3bf27391f0b9682ae8     Bosconian (512k conversion).a52 
 81790daff7f7646a6c371c056622be9c     Bounty Bob Strikes Back (Merged) (Big Five Software).a52
 a074a1ff0a16d1e034ee314b85fa41e9     Buck Rogers - Planet of Zoom (USA).a52
 713feccd8f2722f2e9bdcab98e25a35f     Buried Bucks (XL Conversion).a52
 3147ad22f8d5f46b1ef40a39da3a3de1     Captain Beeble (XL Conversion).a52
 79335deb06a1ef532fea8eee8012ecde     Capture the Flag.a52
 01b978c3faf5d516f300f98c00377532     Carol Shaw's River Raid (USA).a52
 4965b4c8acca64c4fac39a7c0763f611     Castle Blast (USA) (Unl).a52
 8f4c07a9e0ef2ded720b403810220aaf     Castle Crisis (USA) (Unl).a52
 d64a175672b6dba0c0b244c949799e64     Caverns of Mars (Conv).a52
 1db260d6769bed6bf4731744213097b8     Caverns Of Mars 2 (Conv).a52
 c4a14a88a4257970223b1ef9bf95da5b     Caverns Of Mars 3 (Conv).a52    
 261702e8d9acbf45d44bb61fd8fa3e17     Centipede (USA).a52
 df283efab9d36a15603283ee2a7bdb71     Chess (XL Conversion).a52
 21b722b9c93076a3605ec157ac3aa4b8     Chop Suey.a52
 3ff7707e25359c9bcb2326a5d8539852     Choplifter! (USA).a52
 701dd2903b55a5b6734afa120e141334     Chicken (XL Conversion).a52
 e60a98edcc5cad98170772ea8d8c118d     Claim Jumper (XL Conversion).a52
 f21a0fb1653215bbeea87dd80249015e     Claim Jumper (XL Converion Alternate).a52
 4a754460e43bebd08b943c8dba31d581     Clowns & Balloons (XL Conversion).a52
 5720423ebd7575941a1586466ba9beaf     Congo Bongo (USA).a52
 1a64edff521608f9f4fa9d7bdb355087     Countermeasure (USA).a52
 4c034f3db0489726abd401550a402c32     COSMI (XL Conversion).a52
 195c23a894c7ac8631757eec661ab1e6     Crossfire (XL Conversion).a52
 cd64cc0b348a634080078206e3111f9a     Crystal Castles (Final Conversion).a52
 7c27d225a13e178610babf331a0759c0     David Crane's Pitfall II - Lost Caverns (USA).a52
 27d5f32b0d46d3d80773a2b505f95046     Defender (1982) (Atari).a52
 b4af8b555278dec6e2c2329881dc0a15     Demon Attack (XL Conversion).a52
 32b2bb28213dbb01b69e003c4b35bb57     Desmonds Dungeon (XL Conversion).a52
 6049d5ef7eddb1bb3a643151ff506219     Diamond Mine (XL Conversion).a52
 3abd0c057474bad46e45f3d4e96eecee     Dig Dug (1983) (Atari).a52
 1d1eab4067fc0aaf2b2b880fb8f72e40     Donkey Kong Arcade.a52
 4dcca2e6a88d57e54bc7b2377cc2e5b5     Donkey Kong Jr Enhanded.a52
 0c393d2b04afae8a8f8827d30794b29a     Donkey Kong (XL Conversion).a52
 ae5b9bbe91983ab111fd7cf3d29d6b11     Donkey Kong Jr (XL Conversion).a52
 159ccaa564fc2472afd1f06665ec6d19     Dreadnaught Factor, The (USA).a52
 b7fafc8ae6bb0801e53d5756b14dbe31     Drelbs.a52
 c4ea4997cf906dd20ae474eebe1d2a04     Dropzone (64k conversion).a52
 e9b7d19c573a30e6503f35c886666358     Encounter.a52
 4b6c878758f4d4de7f9650296db76d2e     Fast Eddie (XL Conversion).a52
 14bd9a0423eafc3090333af916cfbce6     Frisky Tom (USA) (Proto).a52
 05a086fe4cc3ad16d39c3bc45eb9c26f     Fort Apocalypse (XL Conversion).a52
 2c89c9444f99fd7ac83f88278e6772c6     Frogger (1983) (Parker Bros).a52
 d8636222c993ca71ca0904c8d89c4411     Frogger II - Threeedeep! (USA).a52
 3ace7c591a88af22bac0c559bbb08f03     Galaxian (1982) (Atari).a52
 4012282da62c0d72300294447ef6b9a2     Gateway to Apshai (XL Conversion).a52
 0fdce0dd4014f3188d0ca289f53387d0     Gebelli (XL Conversion).a52
 85fe2492e2945015000272a9fefc06e3     Gorf (1982) (CBS).a52
 a21c545a52d488bfdaf078d786bf4916     Gorf Converted (1982) (CBS).a52   
 dc271e475b4766e80151f1da5b764e52     Gremlins (USA).a52
 dacc0a82e8ee0c086971f9d9bac14127     Gyruss (USA).a52
 b7617ac90462ef13f8350e32b8198873     Gyruss (Autofire Hack).a52    
 f8f0e0a6dc2ffee41b2a2dd736cba4cd     H.E.R.O. (USA).a52
 d824f6ee24f8bc412468268395a76159     Ixion (XL Conversion).a52
 936db7c08e6b4b902c585a529cb15fc5     James Bond 007 (USA).a52
 082846d3a43aab4672fe98252eb1b6f9     Jawbreaker (XL Conversion).a52
 25cfdef5bf9b126166d5394ae74a32e7     Joust (USA).a52
 bc748804f35728e98847da6cdaf241a7     Jr. Pac-Man (USA) (Proto).a52
 40f3fca978058da46cd3e63ea8d2412f     Jr Pac-Man (1984) (Atari) (U).a52
 a0d407ab5f0c63e1e17604682894d1a9     Jumpman Jr (Conv).a52
 27140302a715694401319568a83971a1     Jumpman Jr (XL Conversion).a52
 834067fdce5d09b86741e41e7e491d6c     Jungle Hunt (USA).a52    
 92fd2f43bc0adf2f704666b5244fadf1     Kaboom! (USA).a52
 796d2c22f8205fb0ce8f1ee67c8eb2ca     Kangaroo (USA).a52
 f25a084754ea4d37c2fb1dc8ca6dc51b     Keystone Kapers (USA).a52
 3b03e3cda8e8aa3beed4c9617010b010     Koffi - Yellow Kopter (USA) (Unl).a52
 03d0d59c5382b0a34a158e74e9bfce58     Kid Grid.a52
 b99f405de8e7700619bcd18524ba0e0e     K-Razy Shoot-Out (USA).a52
 66977296ff8c095b8cb755de3472b821     K-Razy Shoot-Out (1982) (CBS) [h1] (Two Port).a52
 5154dc468c00e5a343f5a8843a14f8ce     K-Star Patrol (XL Conversion).a52
 c4931be078e2b16dc45e9537ebce836b     Laser Gates (Conversion).a52
 4e16903c352c8ed75ed9377e72ebe333     Laser Hawk (64k conversion).a52
 46264c86edf30666e28553bd08369b83     Last Starfighter, The (USA) (Proto).a52
 ff785ce12ad6f4ca67f662598025c367     Megamania (1983) (Activision).a52
 d00dff571bfa57c7ff7880c3ce03b178     Mario Brothers (1983) (Atari).a52
 1cd67468d123219201702eadaffd0275     Meteorites (USA).a52
 bc33c07415b42646cc813845b979d85a     Meebzork (1983) (Atari).a52
 84d88bcdeffee1ab880a5575c6aca45e     Millipede (USA) (Proto).a52
 d859bff796625e980db1840f15dec4b5     Miner 2049er Starring Bounty Bob (USA).a52
 69d472a79f404e49ad2278df3c8a266e     Miniature Golf (1983) (Atari).a52
 972b6c0dbf5501cacfdc6665e86a796c     Missile Command (USA).a52
 694897cc0d98fcf2f59eef788881f67d     Montezuma's Revenge featuring Panama Joe (USA).a52
 296e5a3a9efd4f89531e9cf0259c903d     Moon Patrol (USA).a52
 2d8e6aa095bf2aee75406ade8b035a50     Moon Patrol Sprite Hack (USA).a52    
 618e3eb7ae2810768e1aefed1bfdcec4     Mountain King (USA).a52
 23296829e0e1316541aa6b5540b9ba2e     Mountain King (1984) (Sunrise Software) [h1] (Two Port).a52
 a6ed56ea679e6279d0baca2e5cafab78     M.U.L.E. (64k conversion).a52
 fc3ab610323cc34e7984f4bd599b871f     Mr Cool (XL Conversion).a52
 d1873645fee21e84b25dc5e939d93e9b     Mr. Do!'s Castle (USA).a52
 ef9a920ffdf592546499738ee911fc1e     Ms. Pac-Man (USA).a52
 8341c9a660280292664bcaccd1bc5279     Necromancer.a52
 6c661ed6f14d635482f1d35c5249c788     Oils Well (XL Conversion).a52
 5781071d4e3760dd7cd46e1061a32046     O'Riley's Mine (XL Conversion).a52
 f1a4d62d9ba965335fa13354a6264623     Pac-Man (USA).a52
 43e9af8d8c648515de46b9f4bcd024d7     Pacific Coast Hwy (XL Conversion).a52
 57c5b010ec9b5f6313e691bdda94e185     Pastfinder (XL Conversion).a52
 a301a449fc20ad345b04932d3ca3ef54     Pengo (USA).a52
 ecbd6dd2ab105dd43f98476966bbf26c     Pitfall! (USA).a52 (use classics fix instead)
 2be3529c33fdf6b76fa7528ba43cdd7f     Pitfall (classics fix).a52
 fd0cbea6ad18194be0538844e3d7fdc9     Pole Position (USA).a52
 c3fc21b6fa55c0473b8347d0e2d2bee0     Pooyan.a52
 dd4ae6add63452aafe7d4fa752cd78ca     Popeye (USA).a52
 66057fd4b37be2a45bd8c8e6aa12498d     Popeye Arcade Final (Hack).a52
 894959d9c5a88c8e1744f7fcbb930065     Preppie (XL Conversion).a52
 ce44d14341fcc5e7e4fb7a04f77ffec9     Q-bert (USA).a52
 9b7d9d874a93332582f34d1420e0f574     QIX (USA).a52
 099706cedd068aced7313ffa371d7ec3     Quest for Quintana Roo (USA).a52
 80e0ad043da9a7564fec75c1346dbc6e     RainbowWalker.a52
 150ff18392c270001f10e7934b2af546     Rally (XL Conversion).a52
 88fa71fc34e81e616bdffc30e013330b     Ratcatcher.a52
 2bb928d7516e451c6b0159ac413407de     RealSports Baseball (USA).a52
 e056001d304db597bdd21b2968fcc3e6     RealSports Basketball (USA).a52
 022c47b525b058796841134bb5c75a18     RealSports Football (USA).a52
 3074fad290298d56c67f82e8588c5a8b     RealSports Soccer (USA).a52
 7e683e571cbe7c77f76a1648f906b932     RealSports Tennis (USA).a52
 0dc44c5bf0829649b7fec37cb0a8186b     Rescue on Fractalus! (USA).a52
 ddf7834a420f1eaae20a7a6255f80a99     Road Runner (USA) (Proto).a52
 5dba5b478b7da9fd2c617e41fb5ccd31     Robotron 2084 (USA).a52
 950aa1075eaf4ee2b2c2cfcf8f6c25b4     Satans Hollow (Conv).a52
 b610a576cbf26a259da4ec5e38c33f09     Savage Pond (XL Conversion).a52
 467e72c97db63eb59011dd062c965ec9     Scramble.a52
 54aa9130fa0a50ab8a74ed5b9076ff81     Shamus (XL Conversion).a52
 37ec5b9d35ae681934698fea36e99aba     Shamus Case II (XL Conversion).a52
 be75afc33f5da12974900317d824f9b9     Sinistar.a52
 6e24e3519458c5cb95a7fd7711131f8d     Space Dungeon (USA).a52
 58430368d2c9190083f95ce923f4c996     Space Invaders (USA).a52
 802a11dfcba6229cc2f93f0f3aaeb3aa     Space Shuttle - A Journey Into Space (USA).a52
 88d286e4b5fbbe7fd1694d98af9ef538     SpeedAce5200.a52
 cd1c3f732c3432c4a642732182b1ea30     Spitfire (1984) (Atari) (Prototype).a52
 6208110dc3c0bf7b15b33246f2971b6e     Spy Hunter (XL Conversion).a52
 e2d3a3e52bb4e3f7e489acd9974d68e2     Star Raiders (USA).a52
 c959b65be720a03b5479650a3af5a511     Star Trek - Strategic Operations Simulator (USA).a52
 00beaa8405c7fb90d86be5bb1b01ea66     Star Wars - The Arcade Game (USA).a52
 a2831487ab0b0b647aa590fb2b834dd9     Star Wars - ROTJ - Death Star Battle (1983) (Parker Bros).a52
 865570ff9052c1704f673e6222192336     Super Breakout (USA).a52
 dfcd77aec94b532728c3d1fef1da9d85     Super Cobra (USA).a52
 d89669f026c34de7f0da2bcb75356e27     Super Pac Man Final (5200).a52
 1569b7869bf9e46abd2c991c3b90caa6     Superfly (XL Conversion).a52
 c098a0ce6c7e059264511e650ce47b35     Tapper (XL Conversion).a52
 496b6a002bc7d749c02014f7ec6c303c     Tempest (1983) (Atari) (Prototype) [!].a52
 6836a07ea7b2a4c071e9e86c5695b4a1     Timeslip_5200 (XL Conversion).a52
 bb3761de48d39218744d7dbb94553528     Time Runner (XL Conversion).a52
 bf4f25d64b364dd53fbd63562ea1bcda     Turmoil (XL Conversion).a52
 3649bfd2008161b9825f386dbaff88da     Up'n Down (XL Conversion).a52
 556a66d6737f0f793821e702547bc051     Vanguard (USA).a52
 560b68b7f83077444a57ebe9f932905a     Wizard of Wor (USA).a52
 8e2ac7b944c30af9fae5f10c3a40f7a4     Worm War I (XL Conversion).a52
 4f6c58c28c41f31e3a1515fe1e5d15af     Xari Arena (USA) (Proto).a52
 f35f9e5699079e2634c4bfed0c5ef2f0     Yars Strike (XL Conversion).a52
 9fee054e7d4ba2392f4ba0cb73fc99a5     Zaxxon (USA).a52
 433d3a2fc9896aa8294271a0204dc7e3     Zaxxon 32k_final.a52
 77beee345b4647563e20fd896231bd47     Zenji (USA).a52
 dc45af8b0996cb6a94188b0be3be2e17     Zone Ranger (USA).a52
    
--------------------------------------------------------------------------------
Credits:
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
