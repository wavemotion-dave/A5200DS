/*
 * cartridge.c - cartridge emulation
 *
 * Copyright (C) 2001-2003 Piotr Fusik
 * Copyright (C) 2001-2005 Atari800 development team (see DOC/CREDITS)
 *
 * This file is part of the Atari800 emulator project which emulates
 * the Atari 400, 800, 800XL, 130XE, and 5200 8-bit computers.
 *
 * Atari800 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Atari800 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Atari800; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "a5200utils.h"
#include "hash.h"

#include "atari.h"
#include "binload.h" 
#include "cartridge.h"
#include "memory.h"
#include "pia.h"
#include "rtime.h"
#include "util.h"
#include "statesav.h"

static const struct cart_t cart_table[] = 
{
    {"DefaultCart000000000000000000000",    CART_5200_8,        CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    220,    32,10,  0x0908},  // Default Cart - If no other cart type found...
    {"c8e90376b7e1b00dcbd4042f50bffb75",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,22,  0x0908},  // Atari 5200 Calibration Cart    
    {"45f8841269313736489180c8ec3e9588",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,18,  0x0908},  // Activision Decathlon, The (USA).a52
    {"4b1aecab0e2f9c90e514cb0a506e3a5f",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,14,  0x0908},  // Adventure II-a.a52
    {"e2f6085028eb8cf24ad7b50ca4ef640f",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,14,  0x0908},  // Adventure II-b.a52
    {"737717ff4f8402ed5b02e4bf866bbbe3",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,16,  0x0908},  // ANALOG Multicart (XL Conversion).a52
    {"77c6b647746bb1413c5566378ef25eec",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,19,  0x0908},  // Archon (XL Conversion).a52
    {"bae7c1e5eb04e19ef8d0d0b5ce134332",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,16,  0x0908},  // Astro Chase (USA).a52
    {"d31a3bbb4c99f539f0d2c4e02bec516e",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,19,  0x0F0F},  // Atlantis (XL Conversion).a52
    {"ec65389cc604b279d69a889725c723e7",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,20,  0x0908},  // Attack of the Mutant Camels (XL Conversion).a52
    {"f5cd178cbea0ae7d8cf65b30cfd04225",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,  30, 185,    256,    256,    32,16,  0x0908},  // Ballblazer (USA).a52
    {"96b424d0bb0339f4edfe8095fe275d62",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,24,  0x0908},  // Batty Builders (XL Conversion).a52
    {"8123393ae9635f6bc15ddc3380b04328",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,18,  0x0908},  // Blueprint (1982) (CBS).a52    
    {"17e5c03b4fcada48d4c2529afcfe3a70",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,24,  0x0908},  // BCs Quest For Tires (XL Conversion).a52
    {"1913310b1e44ad7f3b90aeb16790a850",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,18,  0x0908},  // Beamrider (USA).a52
    {"f8973db8dc272c2e5eb7b8dbb5c0cc3b",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,25,  0x0908},  // BerZerk (USA).a52
    {"139229eed18032fdea735fa5360bd551",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,16,  0x0908},  // Beef Drop Ultimate SD Edition.a52
    {"81790daff7f7646a6c371c056622be9c",    CART_5200_40,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    238,    32,16,  0x0908},  // Bounty Bob Strikes Back (Merged) (Big Five Software) (U).a52
    {"a074a1ff0a16d1e034ee314b85fa41e9",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,    2,  30, 185,    256,    230,    32,15,  0x0908},  // Buck Rogers - Planet of Zoom (USA).a52
    {"713feccd8f2722f2e9bdcab98e25a35f",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,19,  0x0908},  // Buried Bucks (XL Conversion).a52
    {"3147ad22f8d5f46b1ef40a39da3a3de1",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,23,  0x0908},  // Captain Beeble (XL Conversion).a52
    {"01b978c3faf5d516f300f98c00377532",    CART_5200_8,        CTRL_JOY,   DIGITAL,    2,  30, 185,    256,    256,    32,15,  0x0908},  // Carol Shaw's River Raid (USA).a52
    {"4965b4c8acca64c4fac39a7c0763f611",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    220,    32,10,  0x0908},  // Castle Blast (USA) (Unl).a52
    {"8f4c07a9e0ef2ded720b403810220aaf",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    220,    32,10,  0x0908},  // Castle Crisis (USA) (Unl).a52
    {"d64a175672b6dba0c0b244c949799e64",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,16,  0x0908},  // Caverns of Mars (Conv).a52
    {"1db260d6769bed6bf4731744213097b8",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,16,  0x0C0C},  // Caverns Of Mars 2 (Conv).a52
    {"c4a14a88a4257970223b1ef9bf95da5b",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,16,  0x0C0C},  // Caverns Of Mars 3 (Conv).a52    
    {"261702e8d9acbf45d44bb61fd8fa3e17",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,    2,  30, 185,    256,    240,    32,14,  0x0908},  // Centipede (USA).a52
    {"3ff7707e25359c9bcb2326a5d8539852",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    220,    32,10,  0x0908},  // Choplifter! (USA).a52
    {"701dd2903b55a5b6734afa120e141334",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,19,  0x0908},  // Chicken (XL Conversion).a52
    {"e60a98edcc5cad98170772ea8d8c118d",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,22,  0x0908},  // Claim Jumper (XL Conversion).a52
    {"f21a0fb1653215bbeea87dd80249015e",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,22,  0x0908},  // Claim Jumper (XL Converion Alternate)s.a52
    {"4a754460e43bebd08b943c8dba31d581",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,19,  0x0908},  // Clowns & Balloons (XL Conversion).a52
    {"5720423ebd7575941a1586466ba9beaf",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    220,    32,10,  0x0908},  // Congo Bongo (USA).a52
    {"1a64edff521608f9f4fa9d7bdb355087",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    220,    32,10,  0x0908},  // Countermeasure (USA).a52
    {"195c23a894c7ac8631757eec661ab1e6",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,19,  0x0908},  // Crossfire (XL Conversion).a52
    {"cd64cc0b348a634080078206e3111f9a",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,22,  0x0908},  // Crystal Castles (Final Conversion).a52
    {"7c27d225a13e178610babf331a0759c0",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,23,  0x0908},  // David Crane's Pitfall II - Lost Caverns (USA).a52
    {"27d5f32b0d46d3d80773a2b505f95046",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,    2,  30, 185,    256,    240,    32,18,  0x0908},  // Defender (1982) (Atari).a52
    {"b4af8b555278dec6e2c2329881dc0a15",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,18,  0x0C0C},  // Demon Attack (XL Conversion).a52
    {"32b2bb28213dbb01b69e003c4b35bb57",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,18,  0x0908},  // Desmonds Dungeon (XL Conversion).a52
    {"6049d5ef7eddb1bb3a643151ff506219",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,19,  0x0908},  // Diamond Mine (XL Conversion).a52
    {"3abd0c057474bad46e45f3d4e96eecee",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    340,    256,    34,22,  0x0908},  // Dig Dug (1983) (Atari).a52
    {"1d1eab4067fc0aaf2b2b880fb8f72e40",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,19,  0x0908},  // Donkey Kong Arcade.a52
    {"4dcca2e6a88d57e54bc7b2377cc2e5b5",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,19,  0x0908},  // Donkey Kong Jr Enhanded.a52
    {"0c393d2b04afae8a8f8827d30794b29a",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,19,  0x0908},  // Donkey Kong (XL Conversion).a52
    {"ae5b9bbe91983ab111fd7cf3d29d6b11",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,19,  0x0908},  // Donkey Kong Jr (XL Conversion).a52
    {"159ccaa564fc2472afd1f06665ec6d19",    CART_5200_8,        CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,14,  0x0908},  // Dreadnaught Factor, The (USA).a52
    {"4b6c878758f4d4de7f9650296db76d2e",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,19,  0x0908},  // Fast Eddie (XL Conversion).a52
    {"14bd9a0423eafc3090333af916cfbce6",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    220,    32,10,  0x0908},  // Frisky Tom (USA) (Proto).a52
    {"2c89c9444f99fd7ac83f88278e6772c6",    CART_5200_8,        CTRL_FROG,  DIGITAL,    2,   6, 220,    256,    230,    32,14,  0x0908},  // Frogger (1983) (Parker Bros).a52
    {"d8636222c993ca71ca0904c8d89c4411",    CART_5200_EE_16,    CTRL_FROG,  DIGITAL,    2,   6, 220,    256,    230,    32,14,  0x0908},  // Frogger II - Threeedeep! (USA).a52
    {"3ace7c591a88af22bac0c559bbb08f03",    CART_5200_8,        CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    236,    32,16,  0x0A0A},  // Galaxian (1982) (Atari).a52
    {"4012282da62c0d72300294447ef6b9a2",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,19,  0x0908},  // Gateway to Apshai (XL Conversion).a52
    {"0fdce0dd4014f3188d0ca289f53387d0",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,19,  0x0908},  // Gebelli (XL Conversion).a52
    {"85fe2492e2945015000272a9fefc06e3",    CART_5200_8,        CTRL_JOY,   ANALOG,     1,   6, 220,    256,    240,    32,16,  0x0908},  // Gorf (1982) (CBS).a52
    {"a21c545a52d488bfdaf078d786bf4916",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,16,  0x0908},  // Gorf Converted (1982) (CBS).a52   
    {"dc271e475b4766e80151f1da5b764e52",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    220,    32,10,  0x0908},  // Gremlins (USA).a52
    {"dacc0a82e8ee0c086971f9d9bac14127",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,22,  0x0F0F},  // Gyruss (USA).a52
    {"b7617ac90462ef13f8350e32b8198873",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,22,  0x0F0F},  // Gyruss (Autofire Hack).a52    
    {"f8f0e0a6dc2ffee41b2a2dd736cba4cd",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,16,  0x0908},  // H.E.R.O. (USA).a52
    {"d824f6ee24f8bc412468268395a76159",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    220,    32,10,  0x0908},  // Ixion (XL Conversion).a52
    {"936db7c08e6b4b902c585a529cb15fc5",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    220,    32,10,  0x0908},  // James Bond 007 (USA).a52
    {"082846d3a43aab4672fe98252eb1b6f9",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    220,    32,10,  0x0908},  // Jawbreaker (XL Conversion).a52
    {"25cfdef5bf9b126166d5394ae74a32e7",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,22,  0x0908},  // Joust (USA).a52
    {"bc748804f35728e98847da6cdaf241a7",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,22,  0x0908},  // Jr. Pac-Man (USA) (Proto).a52
    {"40f3fca978058da46cd3e63ea8d2412f",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,22,  0x0908},  // Jr Pac-Man (1984) (Atari) (U).a52
    {"a0d407ab5f0c63e1e17604682894d1a9",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,16,  0x0908},  // Jumpman Jr (Conv).a52
    {"27140302a715694401319568a83971a1",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,16,  0x0908},  // Jumpman Jr (XL Conversion).a52
    {"834067fdce5d09b86741e41e7e491d6c",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,20,  0x0908},  // Jungle Hunt (USA).a52    
    {"92fd2f43bc0adf2f704666b5244fadf1",    CART_5200_4,        CTRL_JOY,   ANALOG,     4,   6, 220,    256,    250,    32,22,  0x0908},  // Kaboom! (USA).a52
    {"796d2c22f8205fb0ce8f1ee67c8eb2ca",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,16,  0x0908},  // Kangaroo (USA).a52
    {"f25a084754ea4d37c2fb1dc8ca6dc51b",    CART_5200_8,        CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,23,  0x0908},  // Keystone Kapers (USA).a52
    {"3b03e3cda8e8aa3beed4c9617010b010",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,22,  0x0908},  // Koffi - Yellow Kopter (USA) (Unl).a52
    {"03d0d59c5382b0a34a158e74e9bfce58",    CART_5200_8,        CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,20,  0x0908},  // Kid Grid.a52
    {"b99f405de8e7700619bcd18524ba0e0e",    CART_5200_8,        CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,22,  0x0908},  // K-Razy Shoot-Out (USA).a52
    {"66977296ff8c095b8cb755de3472b821",    CART_5200_8,        CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,22,  0x0908},  // K-Razy Shoot-Out (1982) (CBS) [h1] (Two Port).a52
    {"5154dc468c00e5a343f5a8843a14f8ce",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,24,  0x0908},  // K-Star Patrol (XL Conversion).a52
    {"c4931be078e2b16dc45e9537ebce836b",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,24,  0x0908},  // Laser Gates (Conversion).a52
    {"46264c86edf30666e28553bd08369b83",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    220,    32,10,  0x0908},  // Last Starfighter, The (USA) (Proto).a52
    {"ff785ce12ad6f4ca67f662598025c367",    CART_5200_8,        CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,12,  0x0908},  // Megamania (1983) (Activision).a52
    {"d00dff571bfa57c7ff7880c3ce03b178",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,19,  0x0908},  // Mario Brothers (1983) (Atari).a52
    {"1cd67468d123219201702eadaffd0275",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,16,  0x0908},  // Meteorites (USA).a52
    {"bc33c07415b42646cc813845b979d85a",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    244,    32,20,  0x0908},  // Meebzork (1983) (Atari).a52
    {"84d88bcdeffee1ab880a5575c6aca45e",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,    2,  30, 185,    256,    240,    32,18,  0x0908},  // Millipede (USA) (Proto).a52
    {"d859bff796625e980db1840f15dec4b5",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,24,  0x0908},  // Miner 2049er Starring Bounty Bob (USA).a52
    {"69d472a79f404e49ad2278df3c8a266e",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,19,  0x0908},  // Miniature Golf (1983) (Atari).a52
    {"972b6c0dbf5501cacfdc6665e86a796c",    CART_5200_8,        CTRL_JOY,   ANALOG,     2,   6, 220,    256,    256,    32,22,  0x0908},  // Missile Command (USA).a52
    {"694897cc0d98fcf2f59eef788881f67d",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,20,  0x0908},  // Montezuma's Revenge featuring Panama Joe (USA).a52
    {"296e5a3a9efd4f89531e9cf0259c903d",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,24,  0x0908},  // Moon Patrol (USA).a52
    {"2d8e6aa095bf2aee75406ade8b035a50",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,24,  0x0908},  // Moon Patrol Sprite Hack (USA).a52    
    {"618e3eb7ae2810768e1aefed1bfdcec4",    CART_5200_8,        CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    220,    32,10,  0x0908},  // Mountain King (USA).a52
    {"23296829e0e1316541aa6b5540b9ba2e",    CART_5200_8,        CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    220,    32,10,  0x0908},  // Mountain King (1984) (Sunrise Software) [h1] (Two Port).a52
    {"fc3ab610323cc34e7984f4bd599b871f",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,19,  0x0908},  // Mr Cool (XL Conversion).a52
    {"d1873645fee21e84b25dc5e939d93e9b",    CART_5200_8,        CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    220,    32,10,  0x0908},  // Mr. Do!'s Castle (USA).a52
    {"ef9a920ffdf592546499738ee911fc1e",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,25,  0x0908},  // Ms. Pac-Man (USA).a52
    {"8341c9a660280292664bcaccd1bc5279",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,19,  0x0908},  // Necromancer.a52
    {"6c661ed6f14d635482f1d35c5249c788",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,22,  0x0908},  // Oils Well (XL Conversion).a52
    {"5781071d4e3760dd7cd46e1061a32046",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,19,  0x0908},  // O'Riley's Mine (XL Conversion).a52
    {"f1a4d62d9ba965335fa13354a6264623",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,25,  0x0908},  // Pac-Man (USA).a52
    {"43e9af8d8c648515de46b9f4bcd024d7",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,19,  0x0908},  // Pacific Coast Hwy (XL Conversion).a52
    {"57c5b010ec9b5f6313e691bdda94e185",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,19,  0x0908},  // Pastfinder (XL Conversion).a52
    {"a301a449fc20ad345b04932d3ca3ef54",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,16,  0x0908},  // Pengo (USA).a52
    {"ecbd6dd2ab105dd43f98476966bbf26c",    CART_5200_8,        CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,25,  0x0908},  // Pitfall! (USA).a52 (use classics fix instead)
    {"2be3529c33fdf6b76fa7528ba43cdd7f",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,24,  0x0908},  // Pitfall (classics fix).a52
    {"fd0cbea6ad18194be0538844e3d7fdc9",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,    2,  30, 185,    256,    256,    32,22,  0x0908},  // Pole Position (USA).a52
    {"c3fc21b6fa55c0473b8347d0e2d2bee0",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,16,  0x0908},  // Pooyan.a52
    {"dd4ae6add63452aafe7d4fa752cd78ca",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,22,  0x0908},  // Popeye (USA).a52
    {"66057fd4b37be2a45bd8c8e6aa12498d",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    250,    32,20,  0x0908},  // Popeye Arcade Final (Hack).a52
    {"894959d9c5a88c8e1744f7fcbb930065",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,19,  0x0908},  // Preppie (XL Conversion).a52
    {"ce44d14341fcc5e7e4fb7a04f77ffec9",    CART_5200_8,        CTRL_QBERT, DIGITAL,    2,   6, 220,    256,    240,    32,20,  0x0908},  // Q-bert (USA).a52
    {"9b7d9d874a93332582f34d1420e0f574",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,18,  0x0C0C},  // QIX (USA).a52
    {"099706cedd068aced7313ffa371d7ec3",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    220,    32,10,  0x0908},  // Quest for Quintana Roo (USA).a52
    {"80e0ad043da9a7564fec75c1346dbc6e",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,16,  0x0908},  // RainbowWalker.a52
    {"88fa71fc34e81e616bdffc30e013330b",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,22,  0x0908},  // Ratcatcher.a52
    {"2bb928d7516e451c6b0159ac413407de",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    220,    32,10,  0x0908},  // RealSports Baseball (USA).a52
    {"e056001d304db597bdd21b2968fcc3e6",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    220,    32,10,  0x0908},  // RealSports Basketball (USA).a52
    {"022c47b525b058796841134bb5c75a18",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    220,    32,10,  0x0908},  // RealSports Football (USA).a52
    {"3074fad290298d56c67f82e8588c5a8b",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    220,    32,10,  0x0908},  // RealSports Soccer (USA).a52
    {"7e683e571cbe7c77f76a1648f906b932",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    220,    32,10,  0x0908},  // RealSports Tennis (USA).a52
    {"0dc44c5bf0829649b7fec37cb0a8186b",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    220,    32,10,  0x0908},  // Rescue on Fractalus! (USA).a52
    {"ddf7834a420f1eaae20a7a6255f80a99",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    220,    32,10,  0x0908},  // Road Runner (USA) (Proto).a52
    {"5dba5b478b7da9fd2c617e41fb5ccd31",    CART_5200_NS_16,    CTRL_ROBO,  DIGITAL,    2,   6, 220,    256,    240,    32,18,  0x0908},  // Robotron 2084 (USA).a52
    {"950aa1075eaf4ee2b2c2cfcf8f6c25b4",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    230,    32,16,  0x0908},  // Satans Hollow (Conv).a52
    {"b610a576cbf26a259da4ec5e38c33f09",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,22,  0x0908},  // Savage Pond (XL Conversion).a52
    {"467e72c97db63eb59011dd062c965ec9",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    220,    32,10,  0x0C0C},  // Scramble.a52
    {"54aa9130fa0a50ab8a74ed5b9076ff81",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,19,  0x0908},  // Shamus (XL Conversion).a52
    {"37ec5b9d35ae681934698fea36e99aba",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,19,  0x0908},  // Shamus Case II (XL Conversion).a52
    {"be75afc33f5da12974900317d824f9b9",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    220,    32,10,  0x0908},  // Sinistar.a52
    {"6e24e3519458c5cb95a7fd7711131f8d",    CART_5200_EE_16,    CTRL_ROBO,  DIGITAL,    2,   6, 220,    256,    240,    32,18,  0x0908},  // Space Dungeon (USA).a52
    {"58430368d2c9190083f95ce923f4c996",    CART_5200_8,        CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,16,  0x0908},  // Space Invaders (USA).a52
    {"802a11dfcba6229cc2f93f0f3aaeb3aa",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,16,  0x0908},  // Space Shuttle - A Journey Into Space (USA).a52
    {"88d286e4b5fbbe7fd1694d98af9ef538",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,19,  0x0908},  // SpeedAce5200.a52
    {"cd1c3f732c3432c4a642732182b1ea30",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    244,    32,20,  0x0908},  // Spitfire (1984) (Atari) (Prototype).a52
    {"6208110dc3c0bf7b15b33246f2971b6e",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,16,  0x0908},  // Spy Hunter (XL Conversion).a52
    {"e2d3a3e52bb4e3f7e489acd9974d68e2",    CART_5200_EE_16,    CTRL_JOY,   ANALOG,     3,   6, 220,    256,    240,    32,17,  0x0908},  // Star Raiders (USA).a52
    {"c959b65be720a03b5479650a3af5a511",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    220,    32,10,  0x0908},  // Star Trek - Strategic Operations Simulator (USA).a52
    {"00beaa8405c7fb90d86be5bb1b01ea66",    CART_5200_EE_16,    CTRL_JOY,   ANALOG,     2,   6, 220,    256,    220,    32,10,  0x0908},  // Star Wars - The Arcade Game (USA).a52
    {"a2831487ab0b0b647aa590fb2b834dd9",    CART_5200_8,        CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,18,  0x0908},  // Star Wars - ROTJ - Death Star Battle (1983) (Parker Bros).a52
    {"865570ff9052c1704f673e6222192336",    CART_5200_4,        CTRL_JOY,   ANALOG,     3,   6, 220,    280,    256,    36,18,  0x0908},  // Super Breakout (USA).a52
    {"dfcd77aec94b532728c3d1fef1da9d85",    CART_5200_8,        CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    220,    32,10,  0x0C0C},  // Super Cobra (USA).a52
    {"d89669f026c34de7f0da2bcb75356e27",    CART_5200_EE_16,    CTRL_JOY,   ANALOG,     2,   6, 220,    256,    240,    32,18,  0x0908},  // Super Pac Man Final (5200).a52
    {"c098a0ce6c7e059264511e650ce47b35",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,22,  0x0908},  // Tapper (XL Conversion).a52
    {"496b6a002bc7d749c02014f7ec6c303c",    CART_5200_NS_16,    CTRL_JOY,   ANALOG,     2,   6, 220,    256,    256,    32,22,  0x0908},  // Tempest (1983) (Atari) (Prototype) [!].a52
    {"6836a07ea7b2a4c071e9e86c5695b4a1",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    244,    32,20,  0x0908},  // Timeslip_5200 (XL Conversion).a52
    {"bf4f25d64b364dd53fbd63562ea1bcda",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,19,  0x0908},  // Turmoil (XL Conversion).a52
    {"3649bfd2008161b9825f386dbaff88da",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,19,  0x0908},  // Up'n Down (XL Conversion).a52
    {"556a66d6737f0f793821e702547bc051",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,24,  0x0908},  // Vanguard (USA).a52
    {"560b68b7f83077444a57ebe9f932905a",    CART_5200_NS_16,    CTRL_SWAP,  DIGITAL,    2,   6, 220,    256,    240,    32,14,  0x0C0C},  // Wizard of Wor (USA).a52
    {"8e2ac7b944c30af9fae5f10c3a40f7a4",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,19,  0x0908},  // Worm War I (XL Conversion).a52
    {"4f6c58c28c41f31e3a1515fe1e5d15af",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,22,  0x0908},  // Xari Arena (USA) (Proto).a52
    {"f35f9e5699079e2634c4bfed0c5ef2f0",    CART_5200_8,        CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,18,  0x0908},  // Yars Strike (XL Conversion).a52
    {"9fee054e7d4ba2392f4ba0cb73fc99a5",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    220,    32,10,  0x0908},  // Zaxxon (USA).a52
    {"433d3a2fc9896aa8294271a0204dc7e3",    CART_5200_32,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,19,  0x0908},  // Zaxxon 32k_final.a52
    {"77beee345b4647563e20fd896231bd47",    CART_5200_8,        CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    220,    32,10,  0x0908},  // Zenji (USA).a52
    {"dc45af8b0996cb6a94188b0be3be2e17",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    256,    32,22,  0x0908},  // Zone Ranger (USA).a52
    {"c4ea4997cf906dd20ae474eebe1d2a04",    CART_5200_64,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,19,  0x0908},  // Dropzone (64k conversion).a52
    {"4e16903c352c8ed75ed9377e72ebe333",    CART_5200_64,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,19,  0x0908},  // Laser Hawk (64k conversion).a52
    {"a6ed56ea679e6279d0baca2e5cafab78",    CART_5200_64,       CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,19,  0x0908},  // M.U.L.E. (64k conversion).a52
    {"d9499b29559f8c3bf27391f0b9682ae8",    CART_5200_512,      CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,19,  0x0908},  // Bosconian (512k conversion).a52 
    {"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",    CART_NONE,          CTRL_JOY,   DIGITAL,    2,   6, 220,    256,    240,    32,30,  0x0908},  // End of List
};

UBYTE *cart_image = NULL;       /* For cartridge memory */
char cart_filename[FILENAME_MAX];
struct cart_t myCart __attribute__((section(".dtcm"))) = {"", CART_5200_32, CTRL_JOY, 0,0,0,0};
static byte cart_image_fixed_buffer[CART_MAX_SIZE] __attribute__ ((aligned (4)));;
static byte bryan_bank __attribute__((section(".dtcm"))) = 0;
static byte last_bryan_bank __attribute__((section(".dtcm"))) = 255;
static UWORD last_bounty_bob_bank __attribute__((section(".dtcm"))) = 65535;

/* special support of Bounty Bob on Atari5200 */
ITCM_CODE UBYTE BountyBob1_GetByte(UWORD addr)
{
    if (addr != last_bounty_bob_bank)
    {
        last_bounty_bob_bank = addr;
        addr -= 0x4ff6;
        CopyROM(0x4000, 0x4fff, cart_image + addr * 0x1000);
    }
    return 0;
}

ITCM_CODE UBYTE BountyBob2_GetByte(UWORD addr)
{
    if (addr != last_bounty_bob_bank)
    {
        last_bounty_bob_bank = addr;
        addr -= 0x5ff6;
        CopyROM(0x5000, 0x5fff, cart_image + 0x4000 + addr * 0x1000);
    }
    return 0;
}

ITCM_CODE void BountyBob1_PutByte(UWORD addr, UBYTE value)
{
    if (addr != last_bounty_bob_bank)
    {
        last_bounty_bob_bank = addr;
        addr -= 0x4ff6;
        CopyROM(0x4000, 0x4fff, cart_image + addr * 0x1000);
    }
}

ITCM_CODE void BountyBob2_PutByte(UWORD addr, UBYTE value)
{
    if (addr != last_bounty_bob_bank)
    {
        last_bounty_bob_bank = addr;
        addr -= 0x5ff6;
        CopyROM(0x5000, 0x5fff, cart_image + 0x4000 + addr * 0x1000);
    }
}


// --------------------------------------------------------
// Access to $BFE0-BFFF resets to home bank 1.
// Access to $BFD0-BFDF selects bank according to A2.
// --------------------------------------------------------
ITCM_CODE UBYTE Bryan_GetByte64(UWORD addr)
{
    bryan_bank = (addr & 0x04) ? 1:0;
    if (last_bryan_bank != bryan_bank)
    {
        CopyROM(0x4000, 0xbfff, cart_image + (0x8000 * bryan_bank));
        last_bryan_bank = bryan_bank;
    }
    return 0x00;
}

ITCM_CODE UBYTE Bryan_GetByte64_reset(UWORD addr)
{
    bryan_bank = 1;
    if (last_bryan_bank != bryan_bank)
    {
        CopyROM(0x4000, 0xbfff, cart_image + (0x8000 * bryan_bank));
        last_bryan_bank = bryan_bank;
    }
    return 0x00;
}

// -------------------------------------------------------------
// Access to $BFE0-BFFF resets to home bank 1.
// Access to $BFD0-BFDF changes lower two bank bits by A2-A3.
// Access to $BFC0-BFCF changes upper two bank bits by A2-A3.
// -------------------------------------------------------------
ITCM_CODE UBYTE Bryan_GetByte512(UWORD addr)
{
    if ((addr & 0xBFC0) == 0xBFC0)
    {
         if (addr >= 0xBFE0)
         {
             bryan_bank = 15;
         }
         else if (addr >= 0xBFD0)
         {
             bryan_bank &= 0x0C;
             bryan_bank |= ((addr & 0x0C) >> 2);
         }
         else if (addr >= 0xBFC0)
         {
             bryan_bank &= 0x03;
             bryan_bank |= ((addr & 0x0C));
         }
         if (last_bryan_bank != bryan_bank)
         {
            CopyROM(0x4000, 0xbfff, cart_image +(0x8000L * bryan_bank));
            last_bryan_bank = bryan_bank;
         }
    }
    return dGetByte(addr);
}

int CART_Insert(const char *filename) {
    FILE *fp;
    int len;
    
    /* remove currently inserted cart */
    CART_Remove();

    /* open file */
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        return CART_CANT_OPEN;
    }
    /* check file length */
    len = Util_flen(fp);
    Util_rewind(fp);

    /* Save Filename for state save */
    strcpy(cart_filename, filename);

    memcpy(&myCart, &cart_table[0], sizeof(myCart));

    /* we used a fixed cart memory buffer... it's big enough! */
    cart_image = (UBYTE *) cart_image_fixed_buffer;
    fread(cart_image, 1, len, fp);
    fclose(fp);
    
    /* find cart type */
    myCart.type = CART_NONE;
    myCart.control = CTRL_JOY;
    int len_kb = len >> 10; /* number of kilobytes */
    if (len_kb == 4)  myCart.type =  CART_5200_4;
    if (len_kb == 8)  myCart.type =  CART_5200_8;
    if (len_kb == 16) myCart.type =  CART_5200_EE_16;
    if (len_kb == 32) myCart.type =  CART_5200_32;
    if (len_kb == 40) myCart.type =  CART_5200_40;
    if (len_kb == 64) myCart.type =  CART_5200_64;
    if (len_kb >= 128) myCart.type =  CART_5200_512;

    // --------------------------------------------
    // Go grab the MD5 sum and see if we find 
    // it in our master table of games...
    // --------------------------------------------
    char md5[33];
    hash_Compute(cart_image, len, (byte*)md5);
    int idx=0;
    while (cart_table[idx].type != CART_NONE)
    {
        if (strncasecmp(cart_table[idx].md5, md5,32) == 0)
        {
            memcpy(&myCart, &cart_table[idx], sizeof(myCart));
            break;   
        }
        idx++;
    }

    if (myCart.type != CART_NONE) 
    {
        CART_Start();
        return 0;   
    }
    cart_image = NULL;
    return CART_BAD_FORMAT;
}

void CART_Remove(void) {
    myCart.type = CART_NONE;
    cart_image = NULL;
    CART_Start();
}

void CART_Start(void) 
{
    if (machine_type == MACHINE_5200) 
    {        
        SetROM(0x4ff6, 0x4ff9);     /* disable Bounty Bob bank switching */
        SetROM(0x5ff6, 0x5ff9);     /* disable Bounty Bob bank switching */
        SetROM(0xbfc0, 0xbfff);     /* disable 64K/512K bank switching */
        
        switch (myCart.type) 
        {
        case CART_5200_32:
            CopyROM(0x4000, 0xbfff, cart_image);
            break;
        case CART_5200_EE_16:
            CopyROM(0x4000, 0x5fff, cart_image);
            CopyROM(0x6000, 0x9fff, cart_image);
            CopyROM(0xa000, 0xbfff, cart_image + 0x2000);
            break;
        case CART_5200_NS_16:
            CopyROM(0x8000, 0xbfff, cart_image);
            break;
        case CART_5200_8:
            CopyROM(0x8000, 0x9fff, cart_image);
            CopyROM(0xa000, 0xbfff, cart_image);
            break;
        case CART_5200_4:
            CopyROM(0x8000, 0x8fff, cart_image);
            CopyROM(0x9000, 0x9fff, cart_image);
            CopyROM(0xa000, 0xafff, cart_image);
            CopyROM(0xb000, 0xbfff, cart_image);
            break;
        case CART_5200_40:
            CopyROM(0x4000, 0x4fff, cart_image);
            CopyROM(0x5000, 0x5fff, cart_image + 0x4000);
            CopyROM(0x8000, 0x9fff, cart_image + 0x8000);
            CopyROM(0xa000, 0xbfff, cart_image + 0x8000);
            last_bounty_bob_bank = -1;
            for (int i=0x4ff6; i<=0x4ff9; i++) readmap[i] = BountyBob1_GetByte;
            for (int i=0x5ff6; i<=0x5ff9; i++) readmap[i] = BountyBob2_GetByte;
            for (int i=0x4ff6; i<=0x4ff9; i++) writemap[i] = BountyBob1_PutByte;
            for (int i=0x5ff6; i<=0x5ff9; i++) writemap[i] = BountyBob2_PutByte;
            break;
        case CART_5200_64:
            bryan_bank = 1; last_bryan_bank=1;
            CopyROM(0x4000, 0xbfff, cart_image + (0x8000 * bryan_bank));
            for (int i=0xbfd0; i<= 0xbfdf; i++) readmap[i] = Bryan_GetByte64;
            for (int i=0xbfe0; i<= 0xbfff; i++) readmap[i] = Bryan_GetByte64_reset;
            break;
        case CART_5200_512:
            bryan_bank = 15; last_bryan_bank=15;
            CopyROM(0x4000, 0xbfff, cart_image + (0x8000L * (long)bryan_bank));
            for (int i=0xbfc0; i<= 0xbfff; i++) readmap[i] = Bryan_GetByte512;
            break;
        default:
            /* clear cartridge area so the 5200 will crash */
            dFillMem(0x4000, 0, 0x8000);
            break;
        }
    }
}

void CARTStateRead(void)
{
}

void CARTStateSave(void)
{
}
