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
#include "cartridge.h"
#include "memory.h"
#include "antic.h"
#include "pia.h"
#include "input.h"

u8 bCartIsBanked __attribute__((section(".dtcm"))) = 0;
u8 bryan_bank = 15;

extern void restore_bottom_screen(void);

static const struct cart_t cart_table[] =
{
    {"DefaultCart000000000000000000000",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,24,  0,  X_FIRE},  // Default Cart - If no other cart type found...
    {"72a91c53bfaa558d863610e3e6d50213",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    256,    32,24,  0,  X_FIRE},  // Ant Eater.a52
    {"c8e90376b7e1b00dcbd4042f50bffb75",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,22,  0,  X_FIRE},  // Atari 5200 Calibration Cart
    {"32a6d0de4f1728dee163eb2d4b3f49f1",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    239,    32,16,  0,  X_FIRE},  // Atari Diagnostics v2.3
    {"45f8841269313736489180c8ec3e9588",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    256,    32,18,  0,  X_FIRE},  // Activision Decathlon, The (USA).a52
    {"4b1aecab0e2f9c90e514cb0a506e3a5f",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    252,    32,22,  0,  X_FIRE},  // Adventure II-a.a52
    {"e2f6085028eb8cf24ad7b50ca4ef640f",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    252,    32,22,  0,  X_FIRE},  // Adventure II-b.a52
    {"9a46d46a97b4993891ae1a34477e9c5f",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    252,    32,22,  0,  X_FIRE},  // Adventure II.a52
    {"b48dd725b5d024ef0a5a797fb5acefc6",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,22,  0,  X_FIRE},  // Alien Swarm (XL Conversion).a52
    {"a47fcb4eedab9418ea098bb431a407aa",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,24,  2,  X_FIRE},  // A.E. (Anti-Enviormental Encounter).a52
    {"9e6d04dc20cbd6d3cdb722e420dea203",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    250,    32,17,  0,  X_FIRE},  // ANALOG Multicart (XL Conversion).a52
    {"737717ff4f8402ed5b02e4bf866bbbe3",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    250,    32,17,  0,  X_FIRE},  // ANALOG Multicart V2 (XL Conversion).a52
    {"77c6b647746bb1413c5566378ef25eec",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    252,    32,24,  0,  X_FIRE},  // Archon (XL Conversion).a52
    {"bae7c1e5eb04e19ef8d0d0b5ce134332",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    240,    32,16,  0,  X_FIRE},  // Astro Chase (USA).a52
    {"10cdf2bbb058bb4cc518fd25031f427d",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    248,    32,18,  0,  X_FIRE},  // Astro Grover (XL Conversion).a52
    {"d31a3bbb4c99f539f0d2c4e02bec516e",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,19,  0,  X_FIRE},  // Atlantis (XL Conversion).a52
    {"ec65389cc604b279d69a889725c723e7",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,20,  0,  X_FIRE},  // Attack of the Mutant Camels (XL Conversion).a52
    {"f5cd178cbea0ae7d8cf65b30cfd04225",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,  30, 185,    1, YES_FS,    256,    256,    32,16,  0,  X_FIRE},  // Ballblazer (USA).a52
    {"96b424d0bb0339f4edfe8095fe275d62",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    246,    32,25,  0,  X_PANDN}, // Batty Builders (XL Conversion).a52
    {"8576867c2cfc965cf152be0468f684a7",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    252,    32,24,  0,  X_FIRE},  // Battlezone.a52
    {"8123393ae9635f6bc15ddc3380b04328",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    252,    32,24,  0,  X_FIRE},  // Blueprint (1982) (CBS).a52
    {"17e5c03b4fcada48d4c2529afcfe3a70",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,24,  0,  X_FIRE},  // BCs Quest For Tires (XL Conversion).a52
    {"96ec5b299b203c88f98100b57af6838d",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    256,    32,24,  0,  X_PANUP}, // Biscuits From Hell.bin
    {"315e0bb45f28bb227e92b8c9e00ee8eb",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    228,    32,17,  0,  X_FIRE},  // Blaster.a52
    {"e0b47a17fa6cd9d6addc1961fca43414",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    228,    32,17,  0,  X_FIRE},  // Blaster.a52
    {"992f62ccfda4c92ef113af1dd96d8f55",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    248,    32,24,  0,  X_FIRE},  // BlowSub.a52
    {"1913310b1e44ad7f3b90aeb16790a850",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    247,    32,20,  0,  X_FIRE},  // Beamrider (USA).a52
    {"f8973db8dc272c2e5eb7b8dbb5c0cc3b",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,25,  0,  X_FIRE},  // BerZerk (USA).a52
    {"322cf3f7ff6515e2f363927134146b59",    CART_5200_64,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,28,  0,  X_FIRE},  // Berks4.a52
    {"139229eed18032fdea735fa5360bd551",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,31,  0,  X_PAND2}, // Beef Drop Ultimate SD Edition.a52
    {"8aff994ec12afdcbda01f1aa411e6a90",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,31,  0,  X_PAND2}, // Beef Drop + (Beef Drop Hack).a52
    {"bcff0c4f8edb7726497fc6716d4bded7",    CART_5200_128,      CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    217,    32, 8,  0,  X_FIRE},  // Bosconian (128k AtariMaxSD conversion).a52
    {"417e6d5523b700d5c753f0a9a4121710",    CART_5200_128,      CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    217,    32, 8,  0,  X_FIRE},  // Bosconian 1.5f (128k AtariMaxSD conversion).a52
    {"cea3ea765d3626be01ed9b290c9a9bd0",    CART_5200_512,      CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    217,    32, 8,  0,  X_FIRE},  // Bosconian 1.5f (128k AtariMaxSD conversion).a52
    {"81790daff7f7646a6c371c056622be9c",    CART_5200_40,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    254,    24,28,  0,  X_PANDN}, // Bounty Bob Strikes Back (Merged) (Big Five Software) (U).a52
    {"a074a1ff0a16d1e034ee314b85fa41e9",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,  30, 185,    1,  NO_FS,    256,    256,    32,24,  0,  X_FIRE},  // Buck Rogers - Planet of Zoom (USA).a52
    {"713feccd8f2722f2e9bdcab98e25a35f",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,24,  0,  X_FIRE},  // Buried Bucks (XL Conversion).a52
    {"3147ad22f8d5f46b1ef40a39da3a3de1",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,23,  0,  X_FIRE},  // Captain Beeble (XL Conversion).a52
    {"79335deb06a1ef532fea8eee8012ecde",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    320,    241,    64,21,  0,  X_FIRE},  // Capture the Flag.a52
    {"01b978c3faf5d516f300f98c00377532",    CART_5200_8,        CTRL_JOY,   DIGITAL,     ANA_NORMAL,  30, 185,    1, YES_FS,    256,    256,    32,15,  0,  X_FIRE},  // Carol Shaw's River Raid (USA).a52
    {"e839c8eace5b2c2545736eb302108e5b",    CART_5200_8,        CTRL_JOY,   DIGITAL,     ANA_NORMAL,  30, 185,    1, YES_FS,    256,    256,    32,15,  0,  X_FIRE},  // Carol Shaw's River Raid (Cold Winter).a52
    {"4965b4c8acca64c4fac39a7c0763f611",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,22,  0,  X_FIRE},  // Castle Blast (USA) (Unl).a52
    {"8f4c07a9e0ef2ded720b403810220aaf",    CART_5200_32,       CTRL_JOY,   ANALOG,      ANA_FASTEST,  6, 220,    1, YES_FS,    256,    240,    32,16,  0,  X_FIRE},  // Castle Crisis (USA) (Unl).a52
    {"d64a175672b6dba0c0b244c949799e64",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,15,  0,  X_PANUP}, // Caverns of Mars (Conv).a52
    {"1db260d6769bed6bf4731744213097b8",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,24,  0,  X_FIRE},  // Caverns Of Mars 2 (Conv).a52
    {"c4a14a88a4257970223b1ef9bf95da5b",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    0, YES_FS,    256,    250,    32,25,  0,  X_FIRE},  // Caverns Of Mars 3 (Phobos).a52
    {"261702e8d9acbf45d44bb61fd8fa3e17",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,  45, 165,    1,  NO_FS,    256,    251,    32,16,  0,  X_FIRE},  // Centipede (USA).a52
    {"df283efab9d36a15603283ee2a7bdb71",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,24,  0,  X_FIRE},  // Chess (XL Conversion).a52
    {"21b722b9c93076a3605ec157ac3aa4b8",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    240,    32,19,  0,  X_FIRE},  // Chop Suey.a52
    {"3ff7707e25359c9bcb2326a5d8539852",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,30,  2,  X_FIRE},  // Choplifter! (USA).a52
    {"701dd2903b55a5b6734afa120e141334",    CART_5200_32,       CTRL_JOY,   ANALOG,      ANA_FASTER,   6, 220,    1, YES_FS,    256,    240,    32,19,  2,  X_FIRE},  // Chicken (XL Conversion).a52
    {"e60a98edcc5cad98170772ea8d8c118d",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,22,  0,  X_FIRE},  // Claim Jumper (XL Conversion).a52
    {"f21a0fb1653215bbeea87dd80249015e",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,22,  0,  X_FIRE},  // Claim Jumper (XL Converion Alternate).a52
    {"4a754460e43bebd08b943c8dba31d581",    CART_5200_32,       CTRL_JOY,   ANALOG,      ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,24,  0,  X_FIRE},  // Clowns & Balloons (XL Conversion).a52
    {"dc382809b4ba707d8a9084421c7a4976",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    252,    32,24,  0,  X_FIRE},  // Cloudburst.a52
    {"5720423ebd7575941a1586466ba9beaf",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    254,    32,24,  0,  X_FIRE},  // Congo Bongo (USA).a52
    {"88ea120ef17747d7b567ffa08b9fb578",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    254,    32,24,  0,  X_FIRE},  // Congo Bongo (USA).a52 (alt)
    {"1a64edff521608f9f4fa9d7bdb355087",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    252,    32,24,  0,  X_FIRE},  // Countermeasure (USA).a52
    {"4c034f3db0489726abd401550a402c32",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    240,    32,19,  0,  X_FIRE},  // COSMI (XL Conversion).a52
    {"195c23a894c7ac8631757eec661ab1e6",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,24,  0,  X_FIRE},  // Crossfire (XL Conversion).a52
    {"cd64cc0b348a634080078206e3111f9a",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,23,  0,  X_FIRE},  // Crystal Castles (Final Conversion).a52
    {"c24be906c9d79f4eab391fd583332a4c",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,23,  0,  X_FIRE},  // Curse of the Lost Miner.a52
    {"7c27d225a13e178610babf331a0759c0",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,23,  0,  X_FIRE},  // David Crane's Pitfall II - Lost Caverns (USA).a52
    {"27d5f32b0d46d3d80773a2b505f95046",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,  30, 185,    1,  NO_FS,    256,    256,    32,23,  0,  X_FIRE},  // Defender (1982) (Atari).a52
    {"b4af8b555278dec6e2c2329881dc0a15",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,18,  0,  X_FIRE},  // Demon Attack (XL Conversion).a52
    {"32b2bb28213dbb01b69e003c4b35bb57",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,18,  0,  X_FIRE},  // Desmonds Dungeon (XL Conversion).a52
    {"6049d5ef7eddb1bb3a643151ff506219",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,24,  2,  X_FIRE},  // Diamond Mine (XL Conversion).a52
    {"3abd0c057474bad46e45f3d4e96eecee",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    340,    256,    34,22,  0,  X_FIRE},  // Dig Dug (1983) (Atari).a52
    {"fa55e314e12390747105986a92eb3d18",    CART_5200_64,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    340,    256,    34,22,  0,  X_FIRE},  // Dig Dug Arcade.a52
    {"1d1eab4067fc0aaf2b2b880fb8f72e40",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,31,  0,  X_PANDN}, // Donkey Kong Arcade.a52
    {"0c393d2b04afae8a8f8827d30794b29a",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,31,  0,  X_PANDN}, // Donkey Kong (XL Conversion).a52
    {"527bbf4da0ac51c34d95a5965cf2ba89",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,31,  0,  X_PANDN}, // Donkey Kong Encore.a52
    {"4dcca2e6a88d57e54bc7b2377cc2e5b5",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    248,    32,24,  0,  X_FIRE},  // Donkey Kong Jr Enhanded.a52
    {"ae5b9bbe91983ab111fd7cf3d29d6b11",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    248,    32,24,  0,  X_FIRE},  // Donkey Kong Jr (XL Conversion).a52
    {"bffd689926f335b95d01f9d951d1bfdf",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    248,    32,24,  0,  X_FIRE},  // Donkey Kong Jr (Arcade Gfx).a52
    {"159ccaa564fc2472afd1f06665ec6d19",    CART_5200_8,        CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    254,    32,27,  0,  X_FIRE},  // Dreadnaught Factor, The (USA).a52
    {"8e8a47e7359c5ca3755edfbae5f90912",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    254,    32,27,  0,  X_FIRE},  // Dreadnaught Factor II, The (USA).a52
    {"b7fafc8ae6bb0801e53d5756b14dbe31",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    244,    32,24,  0,  X_FIRE},  // Drelbs.a52
    {"c4ea4997cf906dd20ae474eebe1d2a04",    CART_5200_64_SWAP,  CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,25,  0,  X_FIRE},  // Dropzone (64k conversion).a52
    {"e9b7d19c573a30e6503f35c886666358",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    240,    32,20,  0,  X_FIRE},  // Encounter.a52
    {"7259353c39aadf76f2eb284e7666bb58",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    256,    32,22,  0,  X_FIRE},  // ET (32k).a52
    {"86d0be7010b5a8b61134490257fc191f",    CART_5200_64_SWAP,  CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    256,    32,22,  0,  X_FIRE},  // ET (64k).a52
    {"5789a45479d9769d4662a15f349d83ed",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,  15, 200,    1, YES_FS,    256,    256,    32,24,  0,  X_FIRE},  // Fairy Force (homebrew).a52
    {"4b6c878758f4d4de7f9650296db76d2e",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,26,  0,  X_FIRE},  // Fast Eddie (XL Conversion).a52
    {"5cf2837752ef8dfa3a6962a28fc0077b",    CART_5200_8,        CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,25,  0,  X_FIRE},  // Falcon (XL Conversion).a52
    {"6b58f0f3175a2d6796c35afafe6b245d",    CART_5200_8,        CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    259,    244,    32,15,  0,  X_FIRE},  // Floyd The Droid (XL Conversion).a52
    {"14bd9a0423eafc3090333af916cfbce6",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    250,    32,25,  0,  X_FIRE},  // Frisky Tom (USA) (Proto).a52
    {"c717ebc92233d206f262d15258e3184d",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    250,    32,25,  0,  X_FIRE},  // Frisky Tom (USA) (Hack).a52
    {"05a086fe4cc3ad16d39c3bc45eb9c26f",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,26,  0,  X_FIRE},  // Fort Apocalypse (XL Conversion).a52
    {"2c89c9444f99fd7ac83f88278e6772c6",    CART_5200_8,        CTRL_FROG,  DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    251,    32,25,  0,  X_PANDN}, // Frogger (1983) (Parker Bros).a52
    {"01fe7eebaaa467e1dfacba82e24bae3b",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    251,    32,25,  0,  X_PANDN}, // Frogger (no button hack).a52
    {"d8636222c993ca71ca0904c8d89c4411",    CART_5200_EE_16,    CTRL_FROG,  DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    249,    32,17,  0,  X_FIRE},  // Frogger II - Threeedeep! (USA).a52
    {"98113c00a7c82c83ee893d8e9352aa7a",    CART_5200_8,        CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    248,    32,11,  0,  X_FIRE},  // Galactic_Chase.a52
    {"3ace7c591a88af22bac0c559bbb08f03",    CART_5200_8,        CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    238,    32,17,  0,  X_FIRE},  // Galaxian (1982) (Atari).a52
    {"4012282da62c0d72300294447ef6b9a2",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,24,  0,  X_FIRE},  // Gateway to Apshai (XL Conversion).a52
    {"0fdce0dd4014f3188d0ca289f53387d0",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    267,    254,    40,18,  0,  X_FIRE},  // Gebelli (XL Conversion).a52
    {"85fe2492e2945015000272a9fefc06e3",    CART_5200_8,        CTRL_JOY,   ANALOG,      ANA_SLOWEST,  6, 220,    1,  NO_FS,    256,    251,    32,23,  0,  X_FIRE},  // Gorf (1982) (CBS).a52
    {"a21c545a52d488bfdaf078d786bf4916",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    251,    32,23,  0,  X_FIRE},  // Gorf Converted (1982) (CBS).a52
    {"dc271e475b4766e80151f1da5b764e52",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    249,    32,22,  0,  X_FIRE},  // Gremlins (USA).a52
    {"dacc0a82e8ee0c086971f9d9bac14127",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,22,  0,  X_FIRE},  // Gyruss (USA).a52
    {"b7617ac90462ef13f8350e32b8198873",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,22,  0,  X_FIRE},  // Gyruss (Autofire Hack).a52
    {"3790408417c1dc193dc2d68576755153",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,22,  0,  X_FIRE},  // Gyruss (Autofire Hack - Updated 2024).a52
    {"f8f0e0a6dc2ffee41b2a2dd736cba4cd",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,16,  0,  X_PANUP}, // H.E.R.O. (USA).a52
    {"3491fa368ae42766a83a43a627496c41",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   278,    256,    44,24,  0,  X_PANDN}, // Hangly Pollux.a52
    {"02b00d632f1f1ceaa39ee3df83edf08a",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   278,    256,    44,24,  0,  X_PANDN}, // Hangly Man.a52
    {"cae9255004733e784d6c4fa4f29c7e59",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   278,    256,    44,24,  0,  X_PANDN}, // Hangly Man Plus.a52
    {"194759bb6fc4a1b115b4dd9aa0549034",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   278,    256,    44,24,  0,  X_PANDN}, // Hangly Man Plus (Ghost Eyes).a52
    {"0f6407d83115a78a182f323e5ef76384",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,24,  0,  X_FIRE},  // Heavy Metal.a52
    {"0c25803c9175487afce0c9d636133dc1",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    224,    32,13,  0,  X_FIRE},  // Hyperblast! (XL Conversion).a52
    {"612c3ca4c2938bb36f2e45fc91c6e779",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,24,  0,  X_FIRE},  // Intellidiscs Final Version.a52
    {"d824f6ee24f8bc412468268395a76159",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,24,  0,  X_FIRE},  // Ixion (XL Conversion).a52
    {"936db7c08e6b4b902c585a529cb15fc5",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    246,    32,22,  0,  X_FIRE},  // James Bond 007 (USA).a52
    {"082846d3a43aab4672fe98252eb1b6f9",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    320,    256,    52,24,  2,  X_FIRE},  // Jawbreaker (XL Conversion).a52
    {"25cfdef5bf9b126166d5394ae74a32e7",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,22,  0,  X_FIRE},  // Joust (USA).a52
    {"bc748804f35728e98847da6cdaf241a7",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,22,  0,  X_FIRE},  // Jr. Pac-Man (USA) (Proto).a52
    {"40f3fca978058da46cd3e63ea8d2412f",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,22,  0,  X_FIRE},  // Jr Pac-Man (1984) (Atari) (U).a52
    {"a0d407ab5f0c63e1e17604682894d1a9",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    244,    32,18,  0,  X_FIRE},  // Jumpman Jr (Conv).a52
    {"27140302a715694401319568a83971a1",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    244,    32,18,  0,  X_FIRE},  // Jumpman Jr (XL Conversion).a52
    {"1a6ccf1152d2bcebd16f0989b8257108",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    244,    32,18,  0,  X_FIRE},  // Jumpman Jr (XL Conversion).a52
    {"158f6f91b9635cec86bdf8c1e7c11f3a",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    244,    32,18,  0,  X_FIRE},  // Jumpman Jr (XL Conversion - fixed 2024).a52
    {"834067fdce5d09b86741e41e7e491d6c",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    252,    32,23,  0,  X_FIRE},  // Jungle Hunt (USA).a52
    {"12c020a98e7f898726b9b649d123214f",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    252,    32,23,  0,  X_FIRE},  // Jungle Hunt (Pirate Pete Hack v3).a52
    {"947a614b16ae56a18a05ef8f34340894",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    252,    32,23,  0,  X_FIRE},  // Jungle Hunt (32K Pirate Pete Hack v3).a52
    {"9584d143be1871241e4a0d038e8e1468",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    248,    32,24,  0,  X_FIRE},  // Juno First (XL Conversion).a52
    {"92fd2f43bc0adf2f704666b5244fadf1",    CART_5200_4,        CTRL_JOY,   ANALOG,      ANA_FASTEST,  6, 220,    1, YES_FS,    256,    250,    32,22,  0,  X_FIRE},  // Kaboom! (USA).a52
    {"61c7fc252400eb5a1470e383c4e9faa1",    CART_5200_NS_16,    CTRL_JOY,   ANALOG,      ANA_FASTEST,  6, 220,    1, YES_FS,    256,    250,    32,22,  0,  X_FIRE},  // Kaboom! (No 1812).a52
    {"796d2c22f8205fb0ce8f1ee67c8eb2ca",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    256,    32,24,  0,  X_FIRE},  // Kangaroo (USA).a52
    {"f25a084754ea4d37c2fb1dc8ca6dc51b",    CART_5200_8,        CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    256,    32,23,  0,  X_FIRE},  // Keystone Kapers (USA).a52
    {"3b03e3cda8e8aa3beed4c9617010b010",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    249,    32,26,  0,  X_FIRE},  // Koffi - Yellow Kopter (USA) (Unl).a52
    {"03d0d59c5382b0a34a158e74e9bfce58",    CART_5200_8,        CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,22,  0,  X_FIRE},  // Kid Grid.a52
    {"b99f405de8e7700619bcd18524ba0e0e",    CART_5200_8,        CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    256,    32,22,  0,  X_FIRE},  // K-Razy Shoot-Out (USA).a52
    {"66977296ff8c095b8cb755de3472b821",    CART_5200_8,        CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    256,    32,22,  0,  X_FIRE},  // K-Razy Shoot-Out (1982) (CBS) [h1] (Two Port).a52
    {"5154dc468c00e5a343f5a8843a14f8ce",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    256,    32,24,  0,  X_FIRE},  // K-Star Patrol (XL Conversion).a52
    {"c4931be078e2b16dc45e9537ebce836b",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    256,    32,24,  0,  X_FIRE},  // Laser Gates (Conversion).a52
    {"4e16903c352c8ed75ed9377e72ebe333",    CART_5200_64_SWAP,  CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,24,  0,  X_FIRE},  // Laser Hawk (64k conversion).a52
    {"46264c86edf30666e28553bd08369b83",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,22,  0,  X_FIRE},  // Last Starfighter, The (USA) (Proto).a52
    {"6d2296cada88c2f286bebe909559adcb",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,22,  0,  X_FIRE},  // Last Starfighter, The (USA) (Proto).a52
    {"ff785ce12ad6f4ca67f662598025c367",    CART_5200_8,        CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    256,    32,12,  0,  X_PANUP}, // Megamania (1983) (Activision).a52
    {"8311263811e366bf5ef07977d0f5a5ae",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,  15, 200,    1, YES_FSL,   256,    256,    32,28,  0,  X_PANDN}, // MajorBlink_5200_V2 (XL Conversion).a52
    {"d00dff571bfa57c7ff7880c3ce03b178",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    252,    32,22,  0,  X_FIRE},  // Mario Brothers (1983) (Atari).a52
    {"2f5aec81646216ea2720b87712452193",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    252,    32,22,  0,  X_FIRE},  // Mario Brothers Encore.a52
    {"1cd67468d123219201702eadaffd0275",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    251,    32,25,  0,  X_FIRE},  // Meteorites (USA).a52
    {"bc33c07415b42646cc813845b979d85a",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    256,    32,24,  0,  X_FIRE},  // Meebzork (1983) (Atari).a52
    {"24348dd9287f54574ccc40ee40d24a86",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    276,    256,    41,24,  1,  X_FIRE},  // Microgammon.a52
    {"84d88bcdeffee1ab880a5575c6aca45e",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,  30, 185,    0, YES_FS,    256,    255,    32,24,  0,  X_FIRE},  // Millipede (USA) (Proto).a52
    {"452fe30bd1443b2b586df3974356c43b",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,  30, 185,    0, YES_FS,    256,    255,    32,24,  0,  X_FIRE},  // Millipede (USA) (Refined 2024).a52
    {"d859bff796625e980db1840f15dec4b5",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,24,  0,  X_FIRE},  // Miner 2049er Starring Bounty Bob (USA).a52
    {"69d472a79f404e49ad2278df3c8a266e",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    0, YES_FS,    256,    240,    32,19,  1,  X_FIRE},  // Miniature Golf (1983) (Atari).a52
    {"972b6c0dbf5501cacfdc6665e86a796c",    CART_5200_8,        CTRL_JOY,   ANALOG,      ANA_SLOWER,  20, 185,    1,  NO_FS,    256,    256,    32,24,  0,  X_FIRE},  // Missile Command (USA).a52
    {"3090673bd3f8c04a92e391bf5540b88b",    CART_5200_32,       CTRL_JOY,   ANALOG,      ANA_SLOWER,   6, 220,    1, YES_FSL,   256,    256,    32,24,  0,  X_FIRE},  // MC+final.a52
    {"694897cc0d98fcf2f59eef788881f67d",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    244,    32,24,  0,  X_FIRE},  // Montezuma's Revenge featuring Panama Joe (USA).a52
    {"22a8721acbeb864b3efe5a4d787c31be",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    244,    32,24,  0,  X_FIRE},  // Montezuma's Revenge (Rexux - final).a52
    {"296e5a3a9efd4f89531e9cf0259c903d",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,24,  0,  X_FIRE},  // Moon Patrol (USA).a52
    {"2d8e6aa095bf2aee75406ade8b035a50",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,24,  0,  X_FIRE},  // Moon Patrol Sprite Hack (USA).a52
    {"627dbb2f84daef11229a165a69d84e09",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    254,    32,25,  0,  X_FIRE},  // Moon Patrol Redux.a52
    {"618e3eb7ae2810768e1aefed1bfdcec4",    CART_5200_8,        CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,22,  0,  X_FIRE},  // Mountain King (USA).a52
    {"23296829e0e1316541aa6b5540b9ba2e",    CART_5200_8,        CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,22,  0,  X_FIRE},  // Mountain King (1984) (Sunrise Software) [h1] (Two Port).a52
    {"a6ed56ea679e6279d0baca2e5cafab78",    CART_5200_64,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    246,    32,24,  0,  X_FIRE},  // M.U.L.E. (64k conversion).a52
    {"fc3ab610323cc34e7984f4bd599b871f",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    240,    32,19,  0,  X_FIRE},  // Mr Cool (XL Conversion).a52
    {"d1873645fee21e84b25dc5e939d93e9b",    CART_5200_8,        CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    277,    247,    32,21,  0,  X_FIRE},  // Mr. Do!'s Castle (USA).a52
    {"ef9a920ffdf592546499738ee911fc1e",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    254,    32,25,  0,  X_FIRE},  // Ms. Pac-Man (USA).a52
    {"7b690b188d79d3c44d687dd767a796e9",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    254,    32,25,  0,  X_FIRE},  // Ms. Pac-Man Encore.a52
    {"a45cfbfe35f41a6237ead7b4a5301d94",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,17,  0,  X_PANUP}, // Munchy (XL Conversion).a52
    {"8341c9a660280292664bcaccd1bc5279",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    240,    32,19,  0,  X_FIRE},  // Necromancer.a52
    {"6c661ed6f14d635482f1d35c5249c788",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,22,  0,  X_FIRE},  // Oils Well (XL Conversion).a52
    {"5781071d4e3760dd7cd46e1061a32046",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    240,    32,19,  0,  X_FIRE},  // O'Riley's Mine (XL Conversion).a52
    {"f1a4d62d9ba965335fa13354a6264623",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    256,    32,25,  0,  X_FIRE},  // Pac-Man (USA).a52
    {"e24490c20bf79c933e50c11a89018960",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    256,    32,25,  0,  X_FIRE},  // Pac-Man (Fixed Munch V2).a52
    {"43e9af8d8c648515de46b9f4bcd024d7",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    232,    32,14,  0,  X_FIRE},  // Pacific Coast Hwy (XL Conversion).a52
    {"57c5b010ec9b5f6313e691bdda94e185",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    0, YES_FS,    256,    240,    32,19,  0,  X_FIRE},  // Pastfinder (XL Conversion).a52
    {"1a43c2c84d4ea1b5d440fae4f4d8b372",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    0, YES_FS,    256,    240,    32,19,  0,  X_FIRE},  // Pastfinder (HIP Demo).a52
    {"a301a449fc20ad345b04932d3ca3ef54",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    254,    32,24,  0,  X_PANDN}, // Pengo (USA).a52
    {"b9e727eaef3463d5979ec06fc5bd5048",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,23,  0,  X_FIRE},  // Pinhead.a52
    {"ecbd6dd2ab105dd43f98476966bbf26c",    CART_5200_8,        CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,25,  0,  X_FIRE},  // Pitfall! (USA).a52 (use classics fix instead)
    {"2be3529c33fdf6b76fa7528ba43cdd7f",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,24,  0,  X_FIRE},  // Pitfall (classics fix).a52
    {"e600c16c2b1f063ffb3f96caf4d23235",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,24,  0,  X_FIRE},  // Pitstop (XL Conversion).a52
    {"9e296c2817cbe1671005cf4dfebe8721",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,24,  2,  X_FIRE},  // Protector II (XL Conversion).a52
    {"fd0cbea6ad18194be0538844e3d7fdc9",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,  30, 185,    1, YES_FS,    256,    256,    32,22,  0,  X_FIRE},  // Pole Position (USA).a52
    {"c3fc21b6fa55c0473b8347d0e2d2bee0",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,24,  0,  X_FIRE},  // Pooyan.a52
    {"dd4ae6add63452aafe7d4fa752cd78ca",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,22,  0,  X_FIRE},  // Popeye (USA).a52
    {"66057fd4b37be2a45bd8c8e6aa12498d",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,25,  0,  X_FIRE},  // Popeye Arcade Final (Hack).a52
    {"fcf0236c9368b1cc291f6c5f89f3e5bf",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,25,  0,  X_FIRE},  // Popeye Encore.a52
    {"294d34269953cfef99891c75b418710b",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,25,  0,  X_FIRE},  // Popeye Encore - Mono Punch.a52
    {"894959d9c5a88c8e1744f7fcbb930065",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,21,  0,  X_FIRE},  // Preppie (XL Conversion).a52
    {"e43ca121a9ca4087dad0e1b33fb15071",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,21,  0,  X_FIRE},  // Preppie (XL Conversion).a52
    {"ccd35e9ea3b3c5824214d88a6d8d8f7e",    CART_5200_8,        CTRL_JOY,   ANALOG,      ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,22,  0,  X_FIRE},  // Pete's Diagnostics (1982) (Atari).a52
    {"7830f985faa701bdec47a023b5953cfe",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    0, YES_FS,    256,    256,    32,24,  0,  X_FIRE},  // Pool (XL Conversion).a52
    {"ce44d14341fcc5e7e4fb7a04f77ffec9",    CART_5200_8,        CTRL_QBERT, DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    37,34,  0,  X_PANDN}, // Q-bert (USA).a52
    {"b7ff607ac0495702621110fb0139b4b3",    CART_5200_8,        CTRL_QBERT, DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    37,34,  0,  X_PANDN}, // Q-bert No Button Hack.a52
    {"a61a667101227d86e50e824e6a1b82f7",    CART_5200_NS_16,    CTRL_QBERT, DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    37,34,  0,  X_PANDN}, // Q-bert No Button Hack.a52
    {"e440bea9d915eccf4da8f977c6e47803",    CART_5200_NS_16,    CTRL_QBERT, DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    37,34,  0,  X_PANDN}, // Q-bert No Button Hack.a52
    {"9b7d9d874a93332582f34d1420e0f574",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    254,    32,22,  0,  X_FIRE},  // QIX (USA).a52
    {"099706cedd068aced7313ffa371d7ec3",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    0, YES_FSL,   256,    256,    32,24,  0,  X_FIRE},  // Quest for Quintana Roo (USA).a52
    {"80e0ad043da9a7564fec75c1346dbc6e",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,20,  0,  X_FIRE},  // RainbowWalker.a52
    {"150ff18392c270001f10e7934b2af546",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    240,    32,19,  0,  X_FIRE},  // Rally (XL Conversion).a52
    {"88fa71fc34e81e616bdffc30e013330b",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,22,  0,  X_FIRE},  // Ratcatcher.a52
    {"2bb928d7516e451c6b0159ac413407de",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    240,    32,17,  0,  X_FIRE},  // RealSports Baseball (USA).a52
    {"e056001d304db597bdd21b2968fcc3e6",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    248,    32,17,  0,  X_FIRE},  // RealSports Basketball (USA).a52
    {"cfd05f2a0ea1068718d1149dbd1f19fa",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,24,  0,  X_FIRE},  // RealSports Basketball (USA).a52
    {"022c47b525b058796841134bb5c75a18",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    246,    32,21,  0,  X_FIRE},  // RealSports Football (USA).a52
    {"3074fad290298d56c67f82e8588c5a8b",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    240,    32,16,  0,  X_FIRE},  // RealSports Soccer (USA).a52
    {"7e683e571cbe7c77f76a1648f906b932",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,16,  0,  X_FIRE},  // RealSports Tennis (USA).a52
    {"0dc44c5bf0829649b7fec37cb0a8186b",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    252,    32,20,  0,  X_FIRE},  // Rescue on Fractalus! (USA).a52
    {"ddf7834a420f1eaae20a7a6255f80a99",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    220,    32,10,  0,  X_FIRE},  // Road Runner (USA) (Proto).a52
    {"4d3bdc741f75f8c1a766dd836cef3461",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,24,  0,  X_FIRE},  // Robnbanks.a52
    {"45dee333cecdbe1ef4f703c1db0cea9c",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,24,  0,  X_FIRE},  // Robnbanks-Arcade.a52
    {"6e69666be0103c709dd820807e5b8ffb",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,24,  0,  X_FIRE},  // Robnbanks-Arcade v2.a52
    {"86b358c9bca97c2089b929e3b2751908",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,23,  0,  X_FIRE},  // Rockball 5200.a52
    {"5dba5b478b7da9fd2c617e41fb5ccd31",    CART_5200_NS_16,    CTRL_ROBO,  DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    250,    32,23,  0,  X_FIRE},  // Robotron 2084 (USA).a52
    {"718cc8b828d36abf068b8fc0a81fc7cf",    CART_5200_32,       CTRL_ROBO,  DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    250,    32,23,  0,  X_FIRE},  // Robotron 2084 (Arcade Hack).a52
    {"d4ec7de1becfd944e58cfb8db031c314",    CART_5200_32,       CTRL_ROBO,  DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    250,    32,23,  0,  X_FIRE},  // Robotron 2084 (Alt Colors).a52
    {"b8cbc918cf2bc81f941719b874f13fcb",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    246,    32,24,  0,  X_FIRE},  // Runner Bear.a52
    {"950aa1075eaf4ee2b2c2cfcf8f6c25b4",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    250,    32,28,  0,  X_PANDN}, // Satans Hollow (Conv).a52
    {"b610a576cbf26a259da4ec5e38c33f09",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,22,  0,  X_FIRE},  // Savage Pond (XL Conversion).a52
    {"467e72c97db63eb59011dd062c965ec9",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    254,    32,16,  0,  X_PANUP}, // Scramble.a52
    {"3748e136c451471cdf58c94b251d925f",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    250,    32,25,  0,  X_FIRE},  // Sea Chase.a52
    {"bd4bb4dd468601a2241233778f328267",    CART_5200_64_SWAP,  CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    252,    32,24,  0,  X_FIRE},  // Sea Dragon 64.a52
    {"1aadd70705d84299085845989ec614ef",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    252,    32,24,  0,  X_FIRE},  // Sea Dragon.a52
    {"54aa9130fa0a50ab8a74ed5b9076ff81",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    256,    32,23,  0,  X_FIRE},  // Shamus (XL Conversion).a52
    {"37ec5b9d35ae681934698fea36e99aba",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    256,    32,23,  0,  X_FIRE},  // Shamus Case II (XL Conversion).a52
    {"be75afc33f5da12974900317d824f9b9",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    252,    32,25,  0,  X_FIRE},  // Sinistar.a52
    {"6151575ffb5ceddd26173f709336776b",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,23,  0,  X_FIRE},  // Slime (XL Conversion).a52
    {"6e24e3519458c5cb95a7fd7711131f8d",    CART_5200_EE_16,    CTRL_ROBO,  DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    252,    32,20,  0,  X_PANUP}, // Space Dungeon (USA).a52
    {"58430368d2c9190083f95ce923f4c996",    CART_5200_8,        CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    254,    32,20,  0,  X_PANUP}, // Space Invaders (USA).a52
    {"b94cc9abbdcdbbce73d508d0929ab8b1",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    254,    32,20,  0,  X_PANUP}, // Space Invaders Arcade.a52
    {"8e280ad05824ef4ca32700716ef8e69a",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,23,  0,  X_PANUP}, // Space Invaders Deluxe.a52
    {"802a11dfcba6229cc2f93f0f3aaeb3aa",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    248,    32,20,  0,  X_FIRE},  // Space Shuttle - A Journey Into Space (USA).a52
    {"88d286e4b5fbbe7fd1694d98af9ef538",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    240,    32,19,  0,  X_FIRE},  // SpeedAce5200.a52
    {"cd1c3f732c3432c4a642732182b1ea30",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    0, YES_FS,    256,    256,    32,24,  0,  X_FIRE},  // Spitfire (1984) (Atari) (Prototype).a52
    {"6208110dc3c0bf7b15b33246f2971b6e",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    254,    32,25,  0,  X_PANDN}, // Spy Hunter (XL Conversion).a52
    {"595703dc459cd51fed6e2a191c462969",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,24,  0,  X_FIRE},  // Stargate (Proto).a52
    {"8378e0f92e9365a6ad42efc9b973724a",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,22,  0,  X_FIRE},  // Star Island.a52
    {"e2d3a3e52bb4e3f7e489acd9974d68e2",    CART_5200_EE_16,    CTRL_SR,    DIGITAL,     ANA_NORMAL,  30, 185,    0, YES_FSL,   256,    251,    32,25,  0,  X_FIRE},  // Star Raiders (USA).a52
    {"0fe34d98a055312aba9ea3cb82d3ee2a",    CART_5200_32,       CTRL_SR,    DIGITAL,     ANA_NORMAL,  30, 185,    0, YES_FSL,   256,    251,    32,25,  0,  X_FIRE},  // Star Raiders 5200(shield2-02)(32K).a52
    {"feacc7a44f9e92d245b2cb2485b48bb6",    CART_5200_NS_16,    CTRL_JOY,   ANALOG,      ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,24,  0,  X_FIRE},  // Star Rider.a52
    {"c959b65be720a03b5479650a3af5a511",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,16,  0,  X_FIRE},  // Star Trek - Strategic Operations Simulator (USA).a52
    {"00beaa8405c7fb90d86be5bb1b01ea66",    CART_5200_EE_16,    CTRL_JOY,   ANALOG,      ANA_NORMAL,   6, 220,    1, YES_FS,    256,    250,    32,24,  0,  X_FIRE},  // Star Wars - The Arcade Game (USA).a52
    {"a2831487ab0b0b647aa590fb2b834dd9",    CART_5200_8,        CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,22,  0,  X_FIRE},  // Star Wars - ROTJ - Death Star Battle (1983) (Parker Bros).a52
    {"865570ff9052c1704f673e6222192336",    CART_5200_4,        CTRL_JOY,   ANALOG,      ANA_FASTER,   6, 220,    1, YES_FS,    280,    256,    36,18,  0,  X_FIRE},  // Super Breakout (USA).a52
    {"dfcd77aec94b532728c3d1fef1da9d85",    CART_5200_8,        CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    255,    32,24,  0,  X_FIRE},  // Super Cobra (USA).a52
    {"d89669f026c34de7f0da2bcb75356e27",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    256,    32,35,  0,  X_PANDN}, // Super Pac Man Final (5200).a52
    {"7dab86351fe78c2f529010a1ac83a4cf",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    256,    32,35,  0,  X_PANDN}, // Super Pac Man.a52
    {"332bf2774f7d9a6008da35420fd7dffc",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    256,    32,35,  0,  X_PANDN}, // Super Pac Man Final Hack (5200).a52
    {"1569b7869bf9e46abd2c991c3b90caa6",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,24,  0,  X_FIRE},  // Superfly (XL Conversion).a52
    {"c098a0ce6c7e059264511e650ce47b35",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    256,    32,23,  0,  X_FIRE},  // Tapper (XL Conversion).a52
    {"59699274930349b4fe0ee94b21ce3c16",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    256,    32,23,  0,  X_FIRE},  // Tapper (XL Conversion).a52
    {"496b6a002bc7d749c02014f7ec6c303c",    CART_5200_NS_16,    CTRL_JOY,   ANALOG,      ANA_SLOWER,   6, 220,    1, YES_FS,    256,    256,    32,22,  0,  X_FIRE},  // Tempest (1983) (Atari) (Prototype) [!].a52
    {"6836a07ea7b2a4c071e9e86c5695b4a1",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    244,    32,20,  0,  X_FIRE},  // Timeslip_5200 (XL Conversion).a52
    {"bb3761de48d39218744d7dbb94553528",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,24,  0,  X_FIRE},  // Time Runner (XL Conversion).a52
    {"3f4d175927f891642e5c9f8a197c7d89",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,24,  0,  X_FIRE},  // Time Runner 32k (BIOS Patched).a52
    {"bf4f25d64b364dd53fbd63562ea1bcda",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,24,  0,  X_FIRE},  // Turmoil (XL Conversion).a52
    {"ae76668cf509a13872ccd874ac47206b",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    256,    32,24,  0,  X_FIRE},  // Tutankahman.a52
    {"3649bfd2008161b9825f386dbaff88da",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    0, YES_FSL,   256,    240,    32,19,  0,  X_FIRE},  // Up'n Down (XL Conversion).a52
    {"556a66d6737f0f793821e702547bc051",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FSL,   256,    256,    32,24,  0,  X_FIRE},  // Vanguard (USA).a52
    {"560b68b7f83077444a57ebe9f932905a",    CART_5200_NS_16,    CTRL_SWAP,  DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,18,  0,  X_FIRE},  // Wizard of Wor (USA).a52
    {"8e2ac7b944c30af9fae5f10c3a40f7a4",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,20,  0,  X_FIRE},  // Worm War I (XL Conversion).a52
    {"677e4fd5bba70f5983d2c2bbfba36b7e",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    0, YES_FS,    256,    256,    32,24,  0,  X_FIRE},  // Xagon (XL Conversion).a52
    {"4f6c58c28c41f31e3a1515fe1e5d15af",    CART_5200_EE_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    248,    32,18,  0,  X_FIRE},  // Xari Arena (USA) (Proto).a52
    {"f35f9e5699079e2634c4bfed0c5ef2f0",    CART_5200_8,        CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    248,    32,20,  0,  X_FIRE},  // Yars Strike (XL Conversion).a52
    {"9fee054e7d4ba2392f4ba0cb73fc99a5",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,18,  0,  X_FIRE},  // Zaxxon (USA).a52
    {"433d3a2fc9896aa8294271a0204dc7e3",    CART_5200_32,       CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    256,    32,19,  0,  X_FIRE},  // Zaxxon 32k_final.a52
    {"77beee345b4647563e20fd896231bd47",    CART_5200_8,        CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    250,    32,23,  0,  X_FIRE},  // Zenji (USA).a52
    {"dc45af8b0996cb6a94188b0be3be2e17",    CART_5200_NS_16,    CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1,  NO_FS,    256,    256,    32,22,  0,  X_FIRE},  // Zone Ranger (USA).a52
    {"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",    CART_NONE,          CTRL_JOY,   DIGITAL,     ANA_NORMAL,   6, 220,    1, YES_FS,    256,    240,    32,30,  0,  X_FIRE},  // End of List
};


UBYTE           cart_image[CART_MAX_SIZE] __attribute__ ((aligned (4)));    // This is the full cartridge image - up to 512K for the largest Super Banked carts...
char            cart_filename[MAX_FILENAME_LEN];
struct cart_t   myCart __attribute__((section(".dtcm")));

/* Rewinds the stream to its beginning. */
#define Util_rewind(fp) fseek(fp, 0, SEEK_SET)

/* Returns the length of an open stream.
   May change the current position. */
int Util_flen(FILE *fp)
{
    fseek(fp, 0, SEEK_END);
    return (int) ftell(fp);
}

/* special support of Bounty Bob on Atari5200 */

inline void bounty_bob_bank4_set(UBYTE b)
{
    mem_map[0x4] = cart_image + (b * 0x1000) - 0x4000;
}

inline void bounty_bob_bank5_set(UBYTE b)
{
    mem_map[0x5] = cart_image + 0x4000 + (b * 0x1000) - 0x5000;
}

ITCM_CODE UBYTE BountyBob1_GetByte(UWORD addr)
{
    bounty_bob_bank4_set(addr-0x4ff6);
    return dGetByte(addr);
}

ITCM_CODE UBYTE BountyBob2_GetByte(UWORD addr)
{
    bounty_bob_bank5_set(addr-0x5ff6);
    return dGetByte(addr);
}

ITCM_CODE void BountyBob1_PutByte(UWORD addr, UBYTE value)
{
    bounty_bob_bank4_set(addr-0x4ff6);
}

ITCM_CODE void BountyBob2_PutByte(UWORD addr, UBYTE value)
{
    bounty_bob_bank5_set(addr-0x5ff6);
}


// ------------------------------------------------
// Setup the memory map to point to the right bank
// ------------------------------------------------
ITCM_CODE void bryan_bank_set(UBYTE b)
{
    UBYTE *bank_ptr = cart_image + (b * 0x8000);
    mem_map[0x4] = bank_ptr - 0x4000;
    mem_map[0x5] = bank_ptr - 0x4000;
    mem_map[0x6] = bank_ptr - 0x4000;
    mem_map[0x7] = bank_ptr - 0x4000;
    mem_map[0x8] = bank_ptr - 0x4000;
    mem_map[0x9] = bank_ptr - 0x4000;
    mem_map[0xA] = bank_ptr - 0x4000;
    mem_map[0xB] = bank_ptr - 0x4000;
}

// --------------------------------------------------------
// Access to $BFE0-BFFF resets to home bank 1.
// Access to $BFD0-BFDF selects bank according to A2.
// --------------------------------------------------------
ITCM_CODE UBYTE Bryan_GetByte64(UWORD addr)
{
    UBYTE b = (addr & 0x04) ? 1:0;
    bryan_bank_set((addr < 0xBFE0) ? b : 1);    // If above 0xBFE0, select "top" bank

    return dGetByte(addr);
}

// --------------------------------------------------------
// Access to $BFE0-BFFF resets to home bank 1.
// Access to $BFD0-BFDF selects bank according to A2.
// This version swaps actual memory - it's slower than the
// banking version but allows us to use the fast CPU driver
// --------------------------------------------------------
ITCM_CODE UBYTE Bryan_GetByte64_Swap(UWORD addr)
{
    UBYTE b = (addr < 0xBFE0) ? ((addr & 0x04) ? 1:0) : 1;
    memcpy(memory + 0x4000, cart_image + (b * 0x8000), 0x8000);
    return dGetByte(addr);
}


// -------------------------------------------------------------
// Access to $BFE0-BFFF resets to home bank 3.
// Access to $BFD0-BFDF changes lower two bank bits by A2-A3.
// Access to $BFC0-BFCF changes upper two bank bits by A2-A3.
// -------------------------------------------------------------
ITCM_CODE UBYTE Bryan_GetByte128(UWORD addr)
{
    UBYTE b = ((addr & 0x0C) >> 2);
    bryan_bank_set((addr < 0xBFE0) ? b : 3);    // If above 0xBFE0, select "top" bank

    return dGetByte(addr);
}

// -----------------------------------------------------------
// - a read from $BFC0-$BFC3 or $BFC8-$BFCB sets bit 2 to 0,
// - a read from $BFC4-$BFC7 or $BFCC-$BFCF sets bit 2 to 1,
// - a read from $BFD0-$BFD3 sets bits 0-1 to 00,
// - a read from $BFD4-$BFD7 sets bits 0-1 to 01,
// - a read from $BFD8-$BFDB sets bits 0-1 to 10,
// - a read from $BFDC-$BFDF sets bits 0-1 to 11,
// -----------------------------------------------------------
ITCM_CODE UBYTE Bryan_GetByte256(UWORD addr)
{
    if (addr >= 0xBFE0) bryan_bank_set(7);
    else
    {
        if (addr >= 0xBFD0)         {bryan_bank &= 0xFC; bryan_bank |= ((addr & 0x0C) >> 2);}
        else if (addr >= 0xBFC0)    {bryan_bank &= 0xF3; bryan_bank |= ((addr & 0x04) >> 0);}
        bryan_bank_set(bryan_bank);
    }

    return dGetByte(addr);
}

// -------------------------------------------------
// - a read from $BFC0-$BFC3 sets bits 2-3 to 00,
// - a read from $BFC4-$BFC7 sets bits 2-3 to 01,
// - a read from $BFC8-$BFCB sets bits 2-3 to 10,
// - a read from $BFCC-$BFCF sets bits 2-3 to 11,

// - a read from $BFD0-$BFD3 sets bits 0-1 to 00,
// - a read from $BFD4-$BFD7 sets bits 0-1 to 01,
// - a read from $BFD8-$BFDB sets bits 0-1 to 10,
// - a read from $BFDC-$BFDF sets bits 0-1 to 11,
// -------------------------------------------------
ITCM_CODE UBYTE Bryan_GetByte512(UWORD addr)
{
    if (addr >= 0xBFE0) bryan_bank_set(15);
    else
    {
        if (addr >= 0xBFD0)         {bryan_bank &= 0xFC; bryan_bank |= ((addr & 0x0C) >> 2);}
        else if (addr >= 0xBFC0)    {bryan_bank &= 0xF3; bryan_bank |= ((addr & 0x0C) >> 0);}
        bryan_bank_set(bryan_bank);
    }

    return dGetByte(addr);
}

int CART_Insert(const char *filename)
{
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

    // This will enable the GO_banked() CPU driver which is optimized to handling bank switching
    if (len > 32*1024) bCartIsBanked = 1;
    else bCartIsBanked = 0;

    /* limit length of cart */
    if (len <= CART_MAX_SIZE)
    {
        /* Save Filename for state save */
        strcpy(cart_filename, filename);

        /* Set a sensible default */
        memcpy(&myCart, &cart_table[0], sizeof(myCart));

        fread(cart_image, 1, len, fp);
        fclose(fp);

        /* find cart type */
        myCart.type = CART_NONE;
        myCart.control = CTRL_JOY;
        short int len_kb = len >> 10; /* number of kilobytes */
        if (len_kb == 4)   myCart.type = CART_5200_4;
        if (len_kb == 8)   myCart.type = CART_5200_8;
        if (len_kb == 16)  myCart.type = CART_5200_NS_16;  // This is the more popular of the 16K carts with homebrews (since we've got most every EE_16 covered in our table)
        if (len_kb == 32)  myCart.type = CART_5200_32;
        if (len_kb == 40)  myCart.type = CART_5200_40;
        if (len_kb == 64)  myCart.type = CART_5200_64;
        if (len_kb == 128) myCart.type = CART_5200_128;
        if (len_kb == 256) myCart.type = CART_5200_256;
        if (len_kb == 512) myCart.type = CART_5200_512;

        // --------------------------------------------
        // Go grab the MD5 sum and see if we find
        // it in our master table of games...
        // --------------------------------------------
        static char md5[33];
        hash_Compute(cart_image, len, (byte*)md5);
        short int idx=0;
        myCart.frame_skip = NO_FS;
        myCart.artifacting = 0;
        while (cart_table[idx].type != CART_NONE)
        {
            if (strncasecmp(cart_table[idx].md5, md5,32) == 0)
            {
                memcpy(&myCart, &cart_table[idx], sizeof(myCart));
                // For the DSi, we always turn off Frame Skipping...
                if (isDSiMode()) myCart.frame_skip = NO_FS;
                break;
            }
            idx++;
        }

        ANTIC_UpdateArtifacting();

        extern char bStarRaiders;
        if (myCart.control == CTRL_SR) bStarRaiders=1; else bStarRaiders=0;
        restore_bottom_screen();

        if (myCart.type != CART_NONE)
        {
            CART_Start();
            return 0;
        }
    }

    return CART_BAD_FORMAT;
}

void CART_Remove(void)
{
    myCart.type = CART_NONE;
    dFillMem(0x0000, 0x00, 0xC000);     /* Fill memory - blank it out */
    SetROM(0x4000, 0xbfff);             /* Set the entire 32k back to normal ROM */

    // Undo any special cart banking that might have occurred (BBSB or Bryan64, etc)
    mem_map[0x4] = memory; mem_map[0x5] = memory; mem_map[0x6] = memory; mem_map[0x7] = memory;
    mem_map[0x8] = memory; mem_map[0x9] = memory; mem_map[0xA] = memory; mem_map[0xB] = memory;
}

void CART_Start(void)
{
    SetROM(0x4000, 0xbfff);     /* Set the entire 32k back to normal ROM */

    normal_memory[0x0] = 1;  normal_memory[0x1] = 1;  normal_memory[0x2] = 1;  normal_memory[0x3] = 1;
    normal_memory[0x4] = 1;  normal_memory[0x5] = 1;  normal_memory[0x6] = 1;  normal_memory[0x7] = 1;
    normal_memory[0x8] = 1;  normal_memory[0x9] = 1;  normal_memory[0xA] = 1;  normal_memory[0xB] = 1;
    normal_memory[0xC] = 0;  normal_memory[0xD] = 0;  normal_memory[0xE] = 0;  normal_memory[0xF] = 1;

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
        CopyROM(0x8000, 0x9fff, cart_image + 0x8000);   // Fixed bank 8K
        CopyROM(0xa000, 0xbfff, cart_image + 0x8000);   // Fixed bank mirror
        bounty_bob_bank4_set(0);
        bounty_bob_bank5_set(1);
        for (int i=0x4ff6; i<=0x4ff9; i++) readmap[i] = BountyBob1_GetByte;
        for (int i=0x5ff6; i<=0x5ff9; i++) readmap[i] = BountyBob2_GetByte;
        for (int i=0x4ff6; i<=0x4ff9; i++) writemap[i] = BountyBob1_PutByte;
        for (int i=0x5ff6; i<=0x5ff9; i++) writemap[i] = BountyBob2_PutByte;
        normal_memory[0x4] = 0;  normal_memory[0x5] = 0;
        break;
    case CART_5200_64:
        bryan_bank_set(1);
        memcpy(memory+0x4000, cart_image+0x8000, 0x8000);
        for (int i=0xbfd0; i<= 0xbfff; i++) readmap[i] = Bryan_GetByte64;
        normal_memory[0xB] = 0;
        break;
    case CART_5200_64_SWAP: // Same as normal 5200_64 except this one will actually swap memory!
        memcpy(memory+0x4000, cart_image+0x8000, 0x8000);
        for (int i=0xbfd0; i<= 0xbfff; i++) readmap[i] = Bryan_GetByte64_Swap;
        normal_memory[0xB] = 0;
        bCartIsBanked = 0; // Use the faster CPU driver...
        break;
    case CART_5200_128:
        bryan_bank = 3;
        bryan_bank_set(3);
        memcpy(memory+0x4000, cart_image+(3*0x8000), 0x8000);
        for (int i=0xbfc0; i<= 0xbfff; i++) readmap[i] = Bryan_GetByte128;
        normal_memory[0xB] = 0;
        break;
    case CART_5200_256:
        bryan_bank = 7;
        bryan_bank_set(7);
        memcpy(memory+0x4000, cart_image+(7*0x8000), 0x8000);
        for (int i=0xbfc0; i<= 0xbfff; i++) readmap[i] = Bryan_GetByte256;
        normal_memory[0xB] = 0;
        break;
    case CART_5200_512:
        bryan_bank = 15;
        bryan_bank_set(15);
        memcpy(memory+0x4000, cart_image+(15*0x8000), 0x8000);
        for (int i=0xbfc0; i<= 0xbfff; i++) readmap[i] = Bryan_GetByte512;
        normal_memory[0xB] = 0;
        break;
    default:
        /* clear cartridge area so the 5200 will crash */
        dFillMem(0x4000, 0x00, 0x8000);
        break;
    }
}

