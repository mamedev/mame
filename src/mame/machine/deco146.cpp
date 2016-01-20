// license:BSD-3-Clause
// copyright-holders:David Haywood, Charles MacDonald
/* Data East 146 protection chip / memory-mapper & I/O interface */

/*
  The 146 emulation was based on the analysis of a 146 chip by
  Charles MacDonald  http://cgfm2.emuviews.com/new/detech.txt
  using a Super Shanghai board and comparisons with the old protection
  simulations.

  The Deco 104 emulation is handled through deco104.c

  The Deco 146 and 104 chips act as I/O chips and as protection devices
  by using 2 banks of 0x80 words of RAM built into the chips.

  The chip has 0x400 read addresses each of which is mapped to one of
  the RAM addresses (scrambled) as well as logic for shifting, xoring
  and masking the bits returned.

  In addition there is the aformentioned bankswitch behavior triggered
  by causing a read related to a specific write address.

  The chip also provides takes an additional 4 address lines and uses
  them to map Chip Select outputs meaning depending on some
  configuration registers meaning it can potentially be used as a
  memory mapping device similar to the Sega System 16 ones, however
  nothing makes proper use of this functionality.

  There seems to be a way to select an alt read mode too, causing
  the port read lines to be xored.

  Many games only use the basic I/O functions!

  Data East Customs
  60,66,75 and 146 all appear to have identical functionality

  Custom chip 104 appears to work in the same way but with different
  internal tables and different special ports.

  Chip                      146         104
  XOR register port         0x2c        0x42
  MASK register port        0x36        0xee
  Soundlatch port           0x64        0xa8
  Bankswitch port           0x78        0x66
  Extra addr Xor(if used)   0x44a       0x2a4
  CS config region          0x8         0xc

  Both chips are often connected with the lower 10 address lines
  scrambled or reversed.

  Game                                     Chip                                    Address Scramble       Extra Read Address Xor?

  --- 146 compatible games ---

  Edward Randy                             60                                      None                   No
  Mutant Fighter                           66                                      None                   No
  Captain America                          75                                      None                   No
  Lemmings                                 75                                      None                   Yes
  Robocop 2                                75                                      None                   Yes
  Super Shanghai Dragon's Eye              146                                     None                   No
  Funky Jet                                146                                     Interleave             No
  Sotsugyo Shousho                         (same board / config as Funky Jet)
  Nitro Ball                               146                                     Reversed               Yes
  Fighters History                         75                                      Interleave             Yes
  Stadium Hero 96                          146                                     None                   Yes
  Dragon Gun                               146                                     Reversed               No
  Lock 'n' Loaded                          (same board / config as Dragon Gun)

  --- 104 games ---

  Caveman Ninja                            104                                     None                   Yes
  Wizard Fire                              104                                     Reversed               No
  Pocket Gal DX                            104                                     Custom*                No*
  Boogie Wings                             104                                     Reversed               Yes
  Rohga                                    104                                     None                   No
  Diet GoGo                                104                                     Interleave             Yes
  Tattoo Assassins                         104                                     Interleave             No
  Dream Ball                               104                                     None                   No
  Night Slashers                           104                                     Interleave             No
  Double Wings                             104                                     Interleave             Yes
  Schmeiser Robo                           104                                     None                   No


  * not currently hooked up, address scramble not figured out



  */

#include "emu.h"
#include "deco146.h"
#include "machine/eepromser.h"





deco146port_xx port_table[] = {
/* 0x000 */ { 0x08a,           {  NIB1__, NIB2__, NIB3__, BLANK_ },  0, 1 },
/* 0x002 */ { 0x0aa,           {  NIB3__, NIB2__, NIB0__, NIB1__ },  0, 0 },
/* 0x004 */ { 0x018,           {  NIB2R2, NIB3__, BLANK_, BLANK_ },  0, 1 },
/* 0x006 */ { 0x03c,           {  BLANK_, NIB3__, BLANK_, BLANK_ },  0, 0 },
/* 0x008 */ { 0x0bc,           {  NIB0R1, NIB1__, NIB2__, NIB3__ },  1, 1 },
/* 0x00a */ { 0x00e,           {  NIB1R2, NIB2__, NIB3__, NIB0__ },  1, 0 },
/* 0x00c */ { 0x09a,           {  NIB3__, BLANK_, BLANK_, BLANK_ },  0, 0 },
/* 0x00e */ { 0x000,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x010 */ { 0x00c,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x012 */ { 0x006,           {  NIB3R2, BLANK_, BLANK_, BLANK_ },  1, 0 },
/* 0x014 */ { 0x0e6,           {  NIB2__, NIB3__, NIB1__, NIB0__ },  0, 0 },
/* 0x016 */ { 0x09c,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x018 */ { 0x05e,           {  NIB0__, NIB1__, NIB3__, NIB2__ },  1, 1 },
/* 0x01a */ { 0x0de,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x01c */ { 0x002,           {  NIB3__, NIB2__, BLANK_, BLANK_ },  1, 1 },
/* 0x01e */ { 0x0f4,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  1, 1 },
/* 0x020 */ { 0x036,           {  NIB0__, NIB2__, NIB3__, NIB1__ },  1, 0 },
/* 0x022 */ { 0x070,           {  NIB1__, NIB2__, NIB3__, BLANK_ },  1, 1 },
/* 0x024 */ { INPUT_PORT_C,    {  NIB0R2, NIB1__, NIB2__, NIB3__ },  0, 1 }, // // $4024   $FFFF   DCBA    .N.     2
/* 0x026 */ { 0x030,           {  NIB0__, NIB3__, NIB2__, NIB1__ },  0, 0 },
/* 0x028 */ { 0x06a,           {  NIB1__, NIB0__, NIB3__, NIB2__ },  0, 0 },
/* 0x02a */ { 0x0c0,           {  NIB1R3, NIB2__, NIB3__, NIB0__ },  1, 0 },
/* 0x02c */ { 0x01c,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x02e */ { 0x0ec,           {  NIB2R1, NIB3__, NIB0__, NIB1__ },  1, 0 },
/* 0x030 */ { 0x090,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x032 */ { 0x0f0,           {  NIB1R2, NIB2__, NIB3__, BLANK_ },  0, 1 },
/* 0x034 */ { 0x020,           {  NIB1R3, NIB2__, NIB3__, NIB0__ },  0, 0 },
/* 0x036 */ { 0x082,           {  NIB1__, NIB2__, NIB3__, BLANK_ },  1, 0 },
/* 0x038 */ { 0x0a0,           {  NIB2__, NIB1__, NIB3__, NIB0__ },  1, 0 },
/* 0x03a */ { 0x078,           {  NIB3R3, NIB0__, NIB1__, NIB2__ },  1, 1 },
/* 0x03c */ { 0x0be,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x03e */ { 0x066,           {  NIB1__, NIB0__, NIB2__, NIB3__ },  0, 0 },
/* 0x040 */ { 0x0c8,           {  NIB2R1, NIB3__, NIB0__, NIB1__ },  0, 1 },
/* 0x042 */ { 0x0ce,           {  NIB0__, NIB3__, NIB1__, NIB2__ },  1, 0 },
/* 0x044 */ { INPUT_PORT_B,    {  NIB0R2, BLANK_, BLANK_, BLANK_ },  0, 1 },// $4044   $000F   ---A    .N.     2
/* 0x046 */ { INPUT_PORT_B,    {  NIB0__, BLANK_, BLANK_, BLANK_ },  1, 0 }, //  $4046   $000F   ---A    X..     0  (mutant fighter )
/* 0x048 */ { 0x0f6,           {  NIB1__, NIB2__, BLANK_, NIB3__ },  0, 1 },
/* 0x04a */ { 0x0f8,           {  NIB0__, NIB3__, NIB2__, NIB1__ },  0, 1 },
/* 0x04c */ { 0x0cc,           {  NIB1R3, NIB2__, NIB3__, BLANK_ },  0, 0 },
/* 0x04e */ { 0x014,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x050 */ { INPUT_PORT_A,    {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 }, // $4050   $FFFF   DCBA    ...     0   (standard i/o read shanghai)
/* 0x052 */ { 0x0de,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 1 },
/* 0x054 */ { 0x060,           {  NIB0__, NIB2__, NIB3__, NIB1__ },  0, 1 },
/* 0x056 */ { 0x012,           {  NIB3__, NIB2__, NIB1__, NIB0__ },  1, 0 },
/* 0x058 */ { 0x0a2,           {  BLANK_, NIB3__, BLANK_, BLANK_ },  1, 1 },
/* 0x05a */ { 0x06c,           {  NIB1__, NIB2__, BLANK_, NIB3__ },  1, 0 },
/* 0x05c */ { 0x076,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x05e */ { INPUT_PORT_A,    {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 1 }, // // $405E   $FFFF   DCBA    .N.     0
/* 0x060 */ { 0x0dc,           {  NIB3__, NIB0__, NIB2__, NIB1__ },  1, 1 },
/* 0x062 */ { 0x054,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x064 */ { 0x05a,           {  NIB1__, NIB3__, NIB0__, NIB2__ },  1, 0 },
/* 0x066 */ { INPUT_PORT_A,    {  NIB2__, NIB3__, BLANK_, BLANK_ },  0, 0 },// $4066   $FF00   BA--    ..B     0
/* 0x068 */ { 0x0e0,           {  NIB2__, NIB3__, NIB0__, NIB1__ },  0, 1 },
/* 0x06a */ { 0x0d4,           {  NIB3R2, NIB0__, NIB1__, NIB2__ },  0, 0 },
/* 0x06c */ { 0x054,           {  NIB1__, NIB3__, NIB0__, NIB2__ },  1, 1 },
/* 0x06e */ { 0x0fc,           {  NIB3R1, BLANK_, BLANK_, BLANK_ },  1, 1 },
/* 0x070 */ { 0x07e,           {  NIB1R2, NIB2__, NIB3__, NIB0__ },  0, 0 },
/* 0x072 */ { 0x03e,           {  NIB2__, NIB1__, NIB3__, BLANK_ },  0, 1 },
/* 0x074 */ { 0x0c6,           {  BLANK_, NIB3__, BLANK_, NIB2__ },  1, 1 },
/* 0x076 */ { 0x078,           {  NIB3__, NIB2__, NIB1__, NIB0__ },  1, 1 },
/* 0x078 */ { 0x07c,           {  BLANK_, NIB2__, NIB3__, NIB1__ },  0, 1 },
/* 0x07a */ { 0x00e,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x07c */ { 0x09c,           {  NIB2R2, NIB3__, BLANK_, BLANK_ },  1, 1 },
/* 0x07e */ { 0x074,           {  NIB2__, NIB0__, NIB1__, NIB3__ },  0, 0 },
/* 0x080 */ { INPUT_PORT_B,    {  NIB0__, BLANK_, BLANK_, BLANK_ },  0, 1 },// $4080   $000F   ---A    .N.     0
/* 0x082 */ { 0x044,           {  NIB2R2, NIB3__, NIB0__, NIB1__ },  1, 1 },
/* 0x084 */ { 0x0c4,           {  NIB1__, NIB2__, BLANK_, NIB3__ },  1, 1 },
/* 0x086 */ { 0x0be,           {  NIB3__, BLANK_, BLANK_, BLANK_ },  0, 0 },
/* 0x088 */ { 0x04e,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x08a */ { 0x0b2,           {  NIB1R1, NIB2__, NIB3__, BLANK_ },  0, 1 },
/* 0x08c */ { 0x04e,           {  NIB3__, NIB0__, NIB1__, NIB2__ },  1, 1 },
/* 0x08e */ { 0x052,           {  BLANK_, NIB3__, BLANK_, NIB2__ },  0, 0 },
/* 0x090 */ { 0x04a,           {  NIB2__, NIB3__, BLANK_, BLANK_ },  0, 0 },
/* 0x092 */ { INPUT_PORT_B,    {  NIB1__, BLANK_, BLANK_, BLANK_ },  0, 0 },// $4092   $00F0   --A-    ...     0
/* 0x094 */ { 0x02c,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x096 */ { 0x000,           {  BLANK_, NIB3__, BLANK_, BLANK_ },  1, 0 },
/* 0x098 */ { 0x076,           {  NIB2__, NIB3__, BLANK_, BLANK_ },  0, 0 },
/* 0x09a */ { 0x014,           {  BLANK_, NIB3__, BLANK_, BLANK_ },  1, 1 },
/* 0x09c */ { 0x094,           {  NIB2__, NIB1__, NIB0__, NIB3__ },  0, 0 },
/* 0x09e */ { 0x0ac,           {  NIB3__, NIB1__, NIB2__, NIB0__ },  0, 1 },
/* 0x0a0 */ { 0x0a4,           {  NIB3__, NIB0__, NIB1__, NIB2__ },  1, 0 },
/* 0x0a2 */ { 0x098,           {  NIB1R3, NIB2__, NIB3__, NIB0__ },  0, 1 },
/* 0x0a4 */ { 0x02c,           {  NIB0R2, NIB1__, NIB2__, NIB3__ },  0, 1 },
/* 0x0a6 */ { 0x0d8,           {  NIB3__, NIB2__, BLANK_, BLANK_ },  1, 0 },
/* 0x0a8 */ { 0x0d2,           {  NIB1R2, NIB2__, NIB3__, BLANK_ },  1, 1 },
/* 0x0aa */ { 0x0fe,           {  NIB0R3, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x0ac */ { INPUT_PORT_C,    {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 }, // $40AC   $FFFF   DCBA    ...     0    (standard i/o read shanghai)
/* 0x0ae */ { 0x062,           {  NIB2__, NIB3__, NIB1__, NIB0__ },  1, 0 },
/* 0x0b0 */ { 0x00c,           {  NIB3__, NIB2__, NIB0__, NIB1__ },  1, 0 },
/* 0x0b2 */ { 0x078,           {  NIB3R2, NIB0__, NIB1__, NIB2__ },  1, 0 },
/* 0x0b4 */ { 0x046,           {  BLANK_, NIB3__, BLANK_, BLANK_ },  1, 0 },
/* 0x0b6 */ { 0x0c6,           {  NIB3R2, NIB0__, NIB1__, NIB2__ },  1, 0 },
/* 0x0b8 */ { 0x03a,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x0ba */ { 0x0f2,           {  NIB2__, NIB1__, NIB3__, NIB0__ },  0, 0 },
/* 0x0bc */ { 0x0e2,           {  NIB3R2, BLANK_, BLANK_, BLANK_ },  1, 1 },
/* 0x0be */ { INPUT_PORT_B,    {  NIB2__, BLANK_, BLANK_, BLANK_ },  0, 0 }, // // $40BE   $0F00   -A--    ...     0
/* 0x0c0 */ { 0x096,           {  NIB3R2, BLANK_, BLANK_, BLANK_ },  0, 0 },
/* 0x0c2 */ { INPUT_PORT_C,    {  NIB0__, NIB1__, NIB2__, NIB3__ },  1, 0 }, // $40C2   $FFFF   DCBA    X..     0    (edrandy) // standard port order with a xor..
/* 0x0c4 */ { 0x056,           {  NIB3__, BLANK_, BLANK_, BLANK_ },  1, 1 },
/* 0x0c6 */ { 0x09e,           {  NIB2__, NIB3__, NIB0__, NIB1__ },  0, 0 },
/* 0x0c8 */ { 0x05c,           {  NIB2R2, NIB3__, BLANK_, BLANK_ },  1, 0 },
/* 0x0ca */ { 0x028,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x0cc */ { 0x0da,           {  NIB1__, NIB2__, NIB3__, NIB0__ },  1, 0 },
/* 0x0ce */ { 0x0b4,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x0d0 */ { 0x0a6,           {  NIB2__, NIB1__, NIB3__, NIB0__ },  1, 1 },
/* 0x0d2 */ { 0x0a6,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x0d4 */ { 0x06c,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x0d6 */ { 0x064,           {  NIB0__, NIB3__, NIB1__, NIB2__ },  0, 0 },
/* 0x0d8 */ { INPUT_PORT_C,    {  NIB0R1, NIB1__, NIB2__, NIB3__ },  0, 1 }, // // $40D8   $FFFF   DCBA    .N.     1
/* 0x0da */ { 0x0e4,           {  NIB2__, NIB1__, BLANK_, NIB3__ },  0, 1 },
/* 0x0dc */ { 0x048,           {  BLANK_, NIB3__, BLANK_, BLANK_ },  0, 1 },
/* 0x0de */ { 0x0ee,           {  NIB2__, NIB3__, NIB0__, NIB1__ },  1, 1 },
/* 0x0e0 */ { 0x024,           {  NIB0R1, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x0e2 */ { 0x0aa,           {  NIB3R1, NIB0__, NIB1__, NIB2__ },  1, 0 },
/* 0x0e4 */ { 0x004,           {  NIB1__, NIB2__, NIB0__, NIB3__ },  0, 0 },
/* 0x0e6 */ { 0x0c2,           {  NIB3R3, BLANK_, BLANK_, BLANK_ },  1, 0 },
/* 0x0e8 */ { 0x0ae,           {  NIB1__, NIB2__, NIB3__, NIB0__ },  0, 0 },
/* 0x0ea */ { 0x02a,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x0ec */ { 0x05e,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x0ee */ { 0x078,           {  NIB2R3, NIB3__, NIB0__, NIB1__ },  1, 0 },
/* 0x0f0 */ { 0x072,           {  NIB0__, NIB1__, NIB3__, NIB2__ },  0, 1 },
/* 0x0f2 */ { 0x016,           {  NIB3R1, BLANK_, BLANK_, BLANK_ },  0, 0 },
/* 0x0f4 */ { 0x02e,           {  NIB3__, NIB0__, NIB2__, NIB1__ },  1, 0 },
/* 0x0f6 */ { 0x042,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x0f8 */ { 0x068,           {  NIB2__, NIB1__, BLANK_, NIB3__ },  1, 0 },
/* 0x0fa */ { 0x042,           {  NIB3R3, NIB0__, NIB1__, NIB2__ },  0, 0 },
/* 0x0fc */ { 0x034,           {  NIB3R1, BLANK_, BLANK_, BLANK_ },  1, 0 },
/* 0x0fe */ { INPUT_PORT_A,    {  NIB1__, NIB2__, NIB3__, NIB0__ },  0, 0 },// $40FE   $FFFF   CBAD    ...     0
/* 0x100 */ { 0x008,           {  NIB1R3, NIB2__, NIB3__, BLANK_ },  1, 1 },
/* 0x102 */ { 0x0a2,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x104 */ { 0x0c8,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x106 */ { 0x07a,           {  NIB1R1, NIB2__, NIB3__, NIB0__ },  1, 0 },
/* 0x108 */ { 0x050,           {  NIB1R2, NIB2__, NIB3__, BLANK_ },  1, 0 },
/* 0x10a */ { 0x01e,           {  NIB2__, NIB0__, NIB1__, NIB3__ },  1, 0 },
/* 0x10c */ { 0x0ca,           {  BLANK_, BLANK_, BLANK_, NIB3__ },  0, 0 },
/* 0x10e */ { 0x05a,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x110 */ { 0x090,           {  NIB3__, BLANK_, BLANK_, BLANK_ },  1, 1 },
/* 0x112 */ { 0x052,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x114 */ { 0x0ba,           {  NIB1R2, NIB2__, NIB3__, NIB0__ },  0, 1 },
/* 0x116 */ { 0x0c0,           {  NIB2R2, NIB3__, NIB0__, NIB1__ },  0, 0 },
/* 0x118 */ { INPUT_PORT_C,    {  NIB0R2, NIB1__, NIB2__, NIB3__ },  1, 0 }, // // $4118   $FFFF   DCBA    X..     2
/* 0x11a */ { 0x02a,           {  NIB3__, NIB2__, BLANK_, BLANK_ },  0, 0 },
/* 0x11c */ { 0x032,           {  NIB2R3, NIB3__, BLANK_, BLANK_ },  1, 0 },
/* 0x11e */ { 0x026,           {  NIB1R1, NIB2__, NIB3__, NIB0__ },  0, 0 },
/* 0x120 */ { 0x0e0,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x122 */ { 0x0d6,           {  NIB2__, NIB3__, NIB1__, NIB0__ },  1, 1 },
/* 0x124 */ { 0x0a8,           {  NIB0__, NIB3__, NIB2__, NIB1__ },  1, 0 },
/* 0x126 */ { 0x0d0,           {  NIB3__, BLANK_, BLANK_, BLANK_ },  1, 0 },
/* 0x128 */ { 0x080,           {  NIB2R1, NIB3__, NIB0__, NIB1__ },  0, 0 },
/* 0x12a */ { 0x078,           {  NIB3R3, NIB0__, NIB1__, NIB2__ },  0, 1 },
/* 0x12c */ { 0x06e,           {  NIB2__, NIB1__, NIB3__, BLANK_ },  0, 0 },
/* 0x12e */ { 0x092,           {  NIB3R2, BLANK_, BLANK_, BLANK_ },  0, 1 },
/* 0x130 */ { 0x040,           {  NIB3__, NIB2__, BLANK_, BLANK_ },  0, 0 },
/* 0x132 */ { 0x0ea,           {  NIB2R2, NIB3__, NIB0__, NIB1__ },  1, 0 },
/* 0x134 */ { 0x086,           {  NIB3__, NIB2__, BLANK_, BLANK_ },  0, 1 },
/* 0x136 */ { 0x01c,           {  NIB2__, NIB3__, BLANK_, BLANK_ },  0, 1 },
/* 0x138 */ { 0x010,           {  NIB1__, NIB3__, NIB0__, NIB2__ },  0, 0 },
/* 0x13a */ { 0x038,           {  NIB1__, NIB2__, BLANK_, NIB3__ },  0, 0 },
/* 0x13c */ { 0x08e,           {  NIB0__, NIB1__, NIB3__, NIB2__ },  0, 0 },
/* 0x13e */ { 0x04c,           {  NIB2__, NIB3__, BLANK_, BLANK_ },  1, 1 },
/* 0x140 */ { 0x084,           {  NIB3__, NIB0__, NIB2__, NIB1__ },  0, 0 },
/* 0x142 */ { 0x028,           {  NIB3R1, NIB0__, NIB1__, NIB2__ },  1, 1 },
/* 0x144 */ { 0x0b8,           {  NIB1R2, NIB2__, NIB3__, BLANK_ },  0, 0 },
/* 0x146 */ { 0x022,           {  NIB2R3, NIB3__, BLANK_, BLANK_ },  1, 1 },
/* 0x148 */ { 0x046,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x14a */ { 0x08c,           {  NIB1__, NIB2__, NIB3__, BLANK_ },  0, 0 },
/* 0x14c */ { 0x0fa,           {  NIB1__, NIB2__, NIB0__, NIB3__ },  1, 0 },
/* 0x14e */ { 0x0b4,           {  NIB2__, NIB0__, NIB1__, NIB3__ },  1, 1 },
/* 0x150 */ { 0x0f0,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x152 */ { 0x018,           {  NIB1__, NIB0__, NIB2__, NIB3__ },  1, 0 },
/* 0x154 */ { 0x088,           {  NIB3__, BLANK_, BLANK_, BLANK_ },  1, 0 },
/* 0x156 */ { 0x058,           {  NIB3R3, NIB0__, NIB1__, NIB2__ },  0, 1 },
/* 0x158 */ { 0x032,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x15a */ { 0x0a0,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x15c */ { 0x0e8,           {  NIB3__, NIB1__, NIB2__, NIB0__ },  1, 0 },
/* 0x15e */ { 0x04c,           {  NIB3__, BLANK_, BLANK_, BLANK_ },  0, 1 },
/* 0x160 */ { 0x03a,           {  NIB2__, NIB1__, BLANK_, NIB3__ },  1, 1 },
/* 0x162 */ { 0x0b2,           {  NIB1__, NIB2__, NIB3__, NIB0__ },  0, 1 },
/* 0x164 */ { 0x086,           {  NIB2__, NIB1__, NIB3__, BLANK_ },  1, 0 },
/* 0x166 */ { 0x078,           {  NIB1__, NIB2__, NIB0__, NIB3__ },  0, 1 },
/* 0x168 */ { 0x0e6,           {  NIB2__, NIB3__, NIB1__, NIB0__ },  0, 1 },
/* 0x16a */ { 0x028,           {  NIB1__, NIB0__, NIB3__, NIB2__ },  1, 0 },
/* 0x16c */ { 0x026,           {  NIB1__, NIB2__, NIB0__, NIB3__ },  0, 1 },
/* 0x16e */ { INPUT_PORT_B,    {  NIB3__, BLANK_, BLANK_, BLANK_ },  0, 0 }, // // $416E   $F000   A---    ...     0
/* 0x170 */ { 0x02a,           {  NIB2__, NIB0__, NIB1__, NIB3__ },  0, 1 },
/* 0x172 */ { 0x086,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x174 */ { 0x022,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x176 */ { 0x010,           {  NIB0R2, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x178 */ { 0x082,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  1, 0 },
/* 0x17a */ { 0x02c,           {  NIB3__, NIB0__, NIB1__, NIB2__ },  0, 0 },
/* 0x17c */ { 0x0aa,           {  NIB1R2, NIB2__, NIB3__, NIB0__ },  1, 1 },
/* 0x17e */ { 0x056,           {  NIB1R1, NIB2__, NIB3__, BLANK_ },  1, 1 },
/* 0x180 */ { 0x0ec,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x182 */ { 0x098,           {  NIB2R2, NIB3__, BLANK_, BLANK_ },  0, 0 },
/* 0x184 */ { 0x0e2,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x186 */ { 0x09e,           {  NIB0__, NIB3__, NIB1__, NIB2__ },  1, 1 },
/* 0x188 */ { 0x0da,           {  NIB0R2, NIB1__, NIB2__, NIB3__ },  1, 1 },
/* 0x18a */ { 0x0a2,           {  NIB0__, NIB3__, NIB1__, NIB2__ },  0, 1 },
/* 0x18c */ { 0x0c2,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x18e */ { 0x01e,           {  NIB3__, NIB1__, NIB2__, NIB0__ },  0, 0 },
/* 0x190 */ { 0x02e,           {  NIB1__, NIB2__, NIB0__, NIB3__ },  1, 1 },
/* 0x192 */ { 0x062,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x194 */ { 0x002,           {  NIB3__, NIB0__, NIB1__, NIB2__ },  0, 1 },
/* 0x196 */ { 0x072,           {  NIB2__, NIB1__, NIB0__, NIB3__ },  0, 1 },
/* 0x198 */ { 0x076,           {  NIB2__, NIB1__, NIB0__, NIB3__ },  1, 0 },
/* 0x19a */ { 0x0e0,           {  NIB3R1, NIB0__, NIB1__, NIB2__ },  0, 0 },
/* 0x19c */ { 0x064,           {  NIB2R3, NIB3__, NIB0__, NIB1__ },  1, 1 },
/* 0x19e */ { INPUT_PORT_C,    {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 1 }, // // $419E   $FFFF   DCBA    .N.     0
/* 0x1a0 */ { INPUT_PORT_A,    {  NIB3__, NIB0__, NIB1__, NIB2__ },  0, 0 },// $41A0   $FFFF   ADCB    ...     0
/* 0x1a2 */ { 0x078,           {  NIB3__, NIB0__, NIB1__, NIB2__ },  0, 0 },
/* 0x1a4 */ { 0x07e,           {  NIB3R3, NIB0__, NIB1__, NIB2__ },  1, 0 },
/* 0x1a6 */ { 0x004,           {  NIB0R1, NIB1__, NIB2__, NIB3__ },  1, 0 },
/* 0x1a8 */ { 0x0ac,           {  NIB1__, NIB3__, NIB0__, NIB2__ },  0, 1 },
/* 0x1aa */ { 0x0b6,           {  NIB0R2, NIB1__, NIB2__, NIB3__ },  1, 0 },
/* 0x1ac */ { 0x09c,           {  NIB0__, NIB2__, NIB3__, NIB1__ },  0, 0 },
/* 0x1ae */ { 0x006,           {  NIB2R3, NIB3__, BLANK_, BLANK_ },  0, 1 },
/* 0x1b0 */ { 0x0c4,           {  NIB3__, NIB2__, NIB1__, NIB0__ },  0, 0 },
/* 0x1b2 */ { INPUT_PORT_A,    {  NIB0R2, NIB1__, NIB2__, NIB3__ },  0, 0 },// $41B2   $FFFF   DCBA    ...     2
/* 0x1b4 */ { 0x0c0,           {  NIB2__, NIB3__, BLANK_, BLANK_ },  1, 1 },
/* 0x1b6 */ { 0x036,           {  BLANK_, NIB2__, NIB3__, NIB1__ },  1, 1 },
/* 0x1b8 */ { 0x03c,           {  NIB3__, NIB2__, BLANK_, BLANK_ },  1, 1 },
/* 0x1ba */ { 0x0b2,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x1bc */ { 0x024,           {  NIB2R3, NIB3__, NIB0__, NIB1__ },  0, 0 },
/* 0x1be */ { 0x008,           {  NIB3R3, BLANK_, BLANK_, BLANK_ },  0, 1 },
/* 0x1c0 */ { 0x030,           {  NIB3__, NIB2__, NIB1__, NIB0__ },  0, 1 },
/* 0x1c2 */ { 0x09a,           {  NIB3__, NIB0__, NIB2__, NIB1__ },  0, 1 },
/* 0x1c4 */ { 0x016,           {  BLANK_, NIB2__, NIB3__, NIB1__ },  1, 0 },
/* 0x1c6 */ { 0x0ba,           {  NIB1R1, NIB2__, NIB3__, BLANK_ },  0, 0 },
/* 0x1c8 */ { 0x06a,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x1ca */ { 0x0ce,           {  NIB1R1, NIB2__, NIB3__, BLANK_ },  1, 1 },
/* 0x1cc */ { 0x0ec,           {  BLANK_, NIB3__, BLANK_, BLANK_ },  0, 1 },
/* 0x1ce */ { 0x014,           {  NIB3R1, BLANK_, BLANK_, BLANK_ },  0, 1 },
/* 0x1d0 */ { INPUT_PORT_B,    {  NIB1__, BLANK_, BLANK_, BLANK_ },  0, 0 }, // // $41D0   $00F0   --A-    ...     0
/* 0x1d2 */ { 0x050,           {  NIB0__, NIB2__, NIB3__, NIB1__ },  1, 1 },
/* 0x1d4 */ { 0x0a0,           {  NIB2__, NIB3__, NIB0__, NIB1__ },  1, 0 },
/* 0x1d6 */ { 0x05e,           {  NIB3__, NIB2__, NIB1__, NIB0__ },  1, 1 },
/* 0x1d8 */ { 0x01a,           {  NIB1__, NIB0__, NIB2__, NIB3__ },  0, 1 },
/* 0x1da */ { 0x0fc,           {  NIB2__, NIB3__, BLANK_, BLANK_ },  1, 0 },
/* 0x1dc */ { 0x03e,           {  NIB2R1, NIB3__, BLANK_, BLANK_ },  1, 0 },
/* 0x1de */ { 0x078,           {  NIB2__, NIB1__, NIB3__, NIB0__ },  0, 0 },
/* 0x1e0 */ { 0x022,           {  NIB0R3, NIB1__, NIB2__, NIB3__ },  0, 1 },
/* 0x1e2 */ { 0x0c2,           {  NIB0R1, NIB1__, NIB2__, NIB3__ },  0, 1 },
/* 0x1e4 */ { 0x0cc,           {  NIB3R2, NIB0__, NIB1__, NIB2__ },  1, 1 },
/* 0x1e6 */ { 0x01e,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x1e8 */ { 0x002,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x1ea */ { 0x0d2,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x1ec */ { 0x092,           {  NIB2R1, NIB3__, BLANK_, BLANK_ },  0, 0 },
/* 0x1ee */ { 0x0f2,           {  NIB3R2, NIB0__, NIB1__, NIB2__ },  0, 1 },
/* 0x1f0 */ { 0x0c8,           {  NIB2R3, NIB3__, BLANK_, BLANK_ },  0, 0 },
/* 0x1f2 */ { 0x058,           {  NIB2__, NIB3__, BLANK_, BLANK_ },  0, 1 },
/* 0x1f4 */ { 0x0f4,           {  NIB3__, NIB2__, NIB0__, NIB1__ },  1, 1 },
/* 0x1f6 */ { 0x0f0,           {  NIB1R3, NIB2__, NIB3__, BLANK_ },  1, 0 },
/* 0x1f8 */ { 0x088,           {  NIB1__, NIB0__, NIB3__, NIB2__ },  0, 1 },
/* 0x1fa */ { 0x020,           {  NIB3__, NIB1__, NIB2__, NIB0__ },  1, 1 },
/* 0x1fc */ { 0x0ca,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x1fe */ { 0x04c,           {  NIB0R3, NIB1__, NIB2__, NIB3__ },  1, 0 },
/* 0x200 */ { INPUT_PORT_C,    {  NIB0R1, NIB1__, NIB2__, NIB3__ },  1, 0 }, // // $4200   $FFFF   DCBA    X..     1
/* 0x202 */ { 0x04e,           {  BLANK_, BLANK_, BLANK_, NIB3__ },  1, 1 },
/* 0x204 */ { 0x018,           {  NIB1__, NIB0__, NIB3__, NIB2__ },  1, 1 },
/* 0x206 */ { 0x064,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x208 */ { 0x04a,           {  NIB1R1, NIB2__, NIB3__, BLANK_ },  1, 0 },
/* 0x20a */ { 0x080,           {  NIB3R3, BLANK_, BLANK_, BLANK_ },  0, 0 },
/* 0x20c */ { 0x034,           {  NIB3__, NIB2__, BLANK_, BLANK_ },  0, 1 },
/* 0x20e */ { 0x094,           {  NIB1R3, NIB2__, NIB3__, BLANK_ },  0, 1 },
/* 0x210 */ { 0x08a,           {  NIB2__, NIB1__, NIB3__, BLANK_ },  1, 1 },
/* 0x212 */ { 0x07a,           {  NIB3__, BLANK_, BLANK_, BLANK_ },  0, 1 },
/* 0x214 */ { 0x0f8,           {  NIB0__, NIB3__, NIB2__, NIB1__ },  1, 1 },
/* 0x216 */ { 0x070,           {  NIB1R1, NIB2__, NIB3__, NIB0__ },  0, 1 },
/* 0x218 */ { 0x0ca,           {  NIB3__, NIB2__, BLANK_, BLANK_ },  1, 0 },
/* 0x21a */ { 0x078,           {  NIB3R1, BLANK_, BLANK_, BLANK_ },  1, 1 },
/* 0x21c */ { 0x0e4,           {  NIB1R1, NIB2__, NIB3__, NIB0__ },  1, 1 },
/* 0x21e */ { 0x0fe,           {  NIB0__, NIB1__, NIB3__, NIB2__ },  1, 0 },
/* 0x220 */ { 0x0b0,           {  NIB3R1, NIB0__, NIB1__, NIB2__ },  0, 1 },
/* 0x222 */ { 0x07c,           {  NIB2R1, NIB3__, BLANK_, BLANK_ },  0, 1 },
/* 0x224 */ { 0x0b4,           {  NIB2R1, NIB3__, NIB0__, NIB1__ },  1, 1 },
/* 0x226 */ { 0x05c,           {  BLANK_, BLANK_, BLANK_, NIB3__ },  0, 1 },
/* 0x228 */ { 0x0e2,           {  NIB2R3, NIB3__, NIB0__, NIB1__ },  1, 0 },
/* 0x22a */ { INPUT_PORT_C,    {  NIB1__, NIB2__, NIB3__, BLANK_ },  0, 0 }, // // $422A   $FFF0   CBA-    ...     0
/* 0x22c */ { 0x0ae,           {  BLANK_, NIB3__, BLANK_, NIB2__ },  0, 1 },
/* 0x22e */ { 0x0de,           {  NIB2__, NIB3__, BLANK_, BLANK_ },  1, 0 },
/* 0x230 */ { 0x090,           {  NIB1R3, NIB2__, NIB3__, NIB0__ },  1, 1 },
/* 0x232 */ { 0x07c,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x234 */ { 0x0a6,           {  NIB2__, NIB1__, NIB0__, NIB3__ },  1, 1 },
/* 0x236 */ { 0x040,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x238 */ { 0x05a,           {  NIB3__, NIB2__, NIB0__, NIB1__ },  0, 1 },
/* 0x23a */ { 0x0a8,           {  BLANK_, NIB3__, BLANK_, BLANK_ },  0, 0 },
/* 0x23c */ { 0x060,           {  NIB3R3, NIB0__, NIB1__, NIB2__ },  1, 1 },
/* 0x23e */ { 0x074,           {  BLANK_, NIB2__, NIB3__, NIB1__ },  0, 0 },
/* 0x240 */ { 0x06e,           {  NIB3R3, BLANK_, BLANK_, BLANK_ },  1, 1 },
/* 0x242 */ { 0x0d4,           {  NIB2__, NIB1__, NIB3__, NIB0__ },  0, 1 },
/* 0x244 */ { 0x00a,           {  NIB2R3, NIB3__, NIB0__, NIB1__ },  0, 1 },
/* 0x246 */ { 0x068,           {  BLANK_, NIB3__, BLANK_, NIB2__ },  1, 0 },
/* 0x248 */ { 0x0d0,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x24a */ { 0x052,           {  BLANK_, BLANK_, BLANK_, NIB3__ },  1, 0 },
/* 0x24c */ { INPUT_PORT_B,    {  NIB2__, BLANK_, BLANK_, BLANK_ },  0, 0 },//// $424C   $0F00   -A--    ...     0
/* 0x24e */ { 0x00e,           {  NIB0R3, NIB1__, NIB2__, NIB3__ },  1, 1 },
/* 0x250 */ { 0x012,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x252 */ { 0x08c,           {  NIB2R2, NIB3__, NIB0__, NIB1__ },  0, 1 },
/* 0x254 */ { 0x0bc,           {  NIB1__, NIB0__, NIB2__, NIB3__ },  1, 1 },
/* 0x256 */ { 0x078,           {  NIB2R1, NIB3__, BLANK_, BLANK_ },  1, 1 },
/* 0x258 */ { 0x0fe,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x25a */ { 0x0ee,           {  NIB2__, NIB1__, BLANK_, NIB3__ },  0, 0 },
/* 0x25c */ { 0x096,           {  NIB3R2, BLANK_, BLANK_, BLANK_ },  1, 1 },
/* 0x25e */ { 0x0dc,           {  NIB1__, NIB2__, BLANK_, NIB3__ },  0, 0 },
/* 0x260 */ { 0x07e,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x262 */ { 0x038,           {  NIB3__, NIB2__, BLANK_, BLANK_ },  0, 1 },
/* 0x264 */ { 0x046,           {  NIB2__, NIB3__, BLANK_, BLANK_ },  0, 0 },
/* 0x266 */ { 0x0b8,           {  NIB2__, NIB1__, BLANK_, NIB3__ },  0, 1 },
/* 0x268 */ { 0x0d0,           {  NIB2R1, NIB3__, BLANK_, BLANK_ },  1, 0 },
/* 0x26a */ { 0x0c6,           {  NIB3__, BLANK_, BLANK_, BLANK_ },  1, 0 },
/* 0x26c */ { 0x0ea,           {  NIB1R3, NIB2__, NIB3__, BLANK_ },  1, 0 },
/* 0x26e */ { 0x066,           {  NIB1R2, NIB2__, NIB3__, NIB0__ },  0, 0 },
/* 0x270 */ { 0x0f8,           {  NIB3__, NIB2__, BLANK_, BLANK_ },  1, 0 },
/* 0x272 */ { 0x068,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x274 */ { 0x03a,           {  NIB3R1, BLANK_, BLANK_, BLANK_ },  1, 0 },
/* 0x276 */ { 0x0e8,           {  NIB0R3, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x278 */ { 0x032,           {  NIB1__, NIB0__, NIB3__, NIB2__ },  1, 1 },
/* 0x27a */ { 0x08e,           {  NIB1__, NIB2__, NIB0__, NIB3__ },  0, 1 },
/* 0x27c */ { 0x044,           {  NIB2R3, NIB3__, BLANK_, BLANK_ },  0, 0 },
/* 0x27e */ { 0x0f6,           {  NIB3R1, NIB0__, NIB1__, NIB2__ },  0, 1 },
/* 0x280 */ { INPUT_PORT_B,    {  NIB3__, BLANK_, BLANK_, BLANK_ },  0, 0 }, // // $4280   $F000   A---    ...     0
/* 0x282 */ { 0x0be,           {  NIB3R3, NIB0__, NIB1__, NIB2__ },  1, 0 },
/* 0x284 */ { 0x040,           {  NIB0R1, NIB1__, NIB2__, NIB3__ },  0, 1 },
/* 0x286 */ { 0x06a,           {  NIB1R1, NIB2__, NIB3__, BLANK_ },  0, 1 },
/* 0x288 */ { 0x0a4,           {  NIB1__, NIB2__, BLANK_, NIB3__ },  1, 0 },
/* 0x28a */ { 0x0f0,           {  NIB3R2, NIB0__, NIB1__, NIB2__ },  1, 1 },
/* 0x28c */ { 0x0f8,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x28e */ { 0x0d2,           {  BLANK_, NIB3__, BLANK_, BLANK_ },  1, 1 },
/* 0x290 */ { 0x012,           {  NIB2__, NIB3__, NIB0__, NIB1__ },  1, 1 },
/* 0x292 */ { 0x078,           {  NIB2__, NIB1__, BLANK_, NIB3__ },  1, 1 },
/* 0x294 */ { 0x000,           {  NIB0R2, NIB1__, NIB2__, NIB3__ },  1, 1 },
/* 0x296 */ { 0x01c,           {  NIB0__, NIB3__, NIB2__, NIB1__ },  0, 1 },
/* 0x298 */ { 0x048,           {  NIB1__, NIB2__, NIB3__, NIB0__ },  1, 0 },
/* 0x29a */ { 0x06c,           {  NIB3R3, NIB0__, NIB1__, NIB2__ },  0, 1 },
/* 0x29c */ { 0x0d8,           {  NIB2R3, NIB3__, BLANK_, BLANK_ },  0, 1 },
/* 0x29e */ { 0x062,           {  NIB2__, NIB1__, NIB0__, NIB3__ },  0, 1 },
/* 0x2a0 */ { 0x0ac,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x2a2 */ { 0x0d6,           {  NIB3R1, BLANK_, BLANK_, BLANK_ },  0, 1 },
/* 0x2a4 */ { 0x00c,           {  NIB2__, NIB1__, NIB3__, NIB0__ },  1, 1 },
/* 0x2a6 */ { 0x0e8,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x2a8 */ { 0x084,           {  NIB0__, NIB3__, NIB1__, NIB2__ },  0, 0 },
/* 0x2aa */ { 0x054,           {  NIB3__, NIB2__, NIB0__, NIB1__ },  1, 1 },
/* 0x2ac */ { 0x042,           {  NIB2R1, NIB3__, NIB0__, NIB1__ },  1, 1 },
/* 0x2ae */ { 0x07e,           {  NIB2__, NIB3__, BLANK_, BLANK_ },  1, 1 },
/* 0x2b0 */ { 0x002,           {  NIB3__, BLANK_, BLANK_, BLANK_ },  0, 1 },
/* 0x2b2 */ { 0x0d8,           {  NIB3R2, NIB0__, NIB1__, NIB2__ },  1, 0 },
/* 0x2b4 */ { 0x0c4,           {  NIB3__, NIB2__, BLANK_, BLANK_ },  1, 1 },
/* 0x2b6 */ { 0x02e,           {  NIB3__, NIB2__, BLANK_, BLANK_ },  1, 0 },
/* 0x2b8 */ { 0x040,           {  NIB2R1, NIB3__, BLANK_, BLANK_ },  0, 0 },
/* 0x2ba */ { 0x064,           {  NIB0__, NIB3__, NIB2__, NIB1__ },  1, 1 },
/* 0x2bc */ { 0x072,           {  NIB2__, NIB3__, NIB1__, NIB0__ },  1, 1 },
/* 0x2be */ { 0x016,           {  NIB0R2, NIB1__, NIB2__, NIB3__ },  1, 0 },
/* 0x2c0 */ { 0x04e,           {  NIB2__, NIB1__, NIB0__, NIB3__ },  1, 0 },
/* 0x2c2 */ { 0x06e,           {  NIB2__, NIB1__, BLANK_, NIB3__ },  1, 0 },
/* 0x2c4 */ { 0x0de,           {  BLANK_, NIB3__, BLANK_, BLANK_ },  0, 0 },
/* 0x2c6 */ { 0x094,           {  NIB0__, NIB1__, NIB3__, NIB2__ },  0, 0 },
/* 0x2c8 */ { 0x074,           {  NIB2__, NIB0__, NIB1__, NIB3__ },  0, 0 },
/* 0x2ca */ { 0x07c,           {  NIB3__, NIB1__, NIB2__, NIB0__ },  1, 0 },
/* 0x2cc */ { 0x006,           {  NIB3__, NIB0__, NIB2__, NIB1__ },  1, 0 },
/* 0x2ce */ { 0x078,           {  NIB0R2, NIB1__, NIB2__, NIB3__ },  1, 1 },
/* 0x2d0 */ { 0x09e,           {  NIB2__, NIB1__, BLANK_, NIB3__ },  0, 0 },
/* 0x2d2 */ { 0x028,           {  NIB2R2, NIB3__, BLANK_, BLANK_ },  1, 1 },
/* 0x2d4 */ { 0x0b2,           {  NIB3R3, BLANK_, BLANK_, BLANK_ },  1, 0 },
/* 0x2d6 */ { 0x05a,           {  NIB3__, NIB2__, NIB1__, NIB0__ },  1, 0 },
/* 0x2d8 */ { 0x026,           {  NIB1__, NIB2__, NIB3__, BLANK_ },  0, 1 },
/* 0x2da */ { 0x08a,           {  NIB1R1, NIB2__, NIB3__, BLANK_ },  0, 0 },
/* 0x2dc */ { 0x0ac,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 1 },
/* 0x2de */ { 0x096,           {  NIB3__, NIB2__, NIB0__, NIB1__ },  1, 0 },
/* 0x2e0 */ { 0x098,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x2e2 */ { INPUT_PORT_C,    {  NIB3__, NIB1__, NIB2__, NIB0__ },  0, 0 }, // $42E2   $FFFF   ACBD    ...     0
/* 0x2e4 */ { 0x02a,           {  NIB2R1, NIB3__, NIB0__, NIB1__ },  1, 0 },
/* 0x2e6 */ { 0x0fe,           {  NIB0__, NIB3__, NIB1__, NIB2__ },  1, 0 },
/* 0x2e8 */ { 0x0b4,           {  NIB3R1, NIB0__, NIB1__, NIB2__ },  1, 1 },
/* 0x2ea */ { 0x032,           {  NIB1R3, NIB2__, NIB3__, NIB0__ },  0, 0 },
/* 0x2ec */ { 0x0f2,           {  NIB1R3, NIB2__, NIB3__, NIB0__ },  0, 1 },
/* 0x2ee */ { 0x054,           {  NIB0__, NIB2__, NIB3__, NIB1__ },  0, 1 },
/* 0x2f0 */ { 0x0a2,           {  NIB1R2, NIB2__, NIB3__, BLANK_ },  0, 1 },
/* 0x2f2 */ { 0x050,           {  NIB1__, NIB2__, NIB3__, BLANK_ },  1, 0 },
/* 0x2f4 */ { INPUT_PORT_B,    {  NIB0R1, BLANK_, BLANK_, BLANK_ },  0, 0 }, //  // $42F4   $000F   ---A    ...     1
/* 0x2f6 */ { 0x000,           {  NIB0R2, NIB1__, NIB2__, NIB3__ },  0, 1 },
/* 0x2f8 */ { 0x01e,           {  NIB1__, NIB2__, NIB0__, NIB3__ },  1, 0 },
/* 0x2fa */ { 0x014,           {  NIB3__, NIB0__, NIB1__, NIB2__ },  1, 1 },
/* 0x2fc */ { 0x0b8,           {  BLANK_, BLANK_, BLANK_, NIB3__ },  0, 0 },
/* 0x2fe */ { 0x0ae,           {  NIB3__, NIB2__, BLANK_, BLANK_ },  0, 1 },
/* 0x300 */ { 0x068,           {  NIB0__, NIB1__, NIB3__, NIB2__ },  1, 1 },
/* 0x302 */ { 0x0f8,           {  NIB1__, NIB0__, NIB3__, NIB2__ },  0, 0 },
/* 0x304 */ { 0x0d6,           {  NIB2__, NIB3__, NIB0__, NIB1__ },  0, 0 },
/* 0x306 */ { 0x044,           {  NIB1__, NIB2__, NIB3__, NIB0__ },  1, 1 },
/* 0x308 */ { 0x038,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x30a */ { 0x078,           {  NIB2__, NIB3__, BLANK_, BLANK_ },  1, 0 },
/* 0x30c */ { 0x04c,           {  NIB0__, NIB3__, NIB1__, NIB2__ },  0, 1 },
/* 0x30e */ { 0x0c6,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  1, 1 },
/* 0x310 */ { 0x084,           {  NIB3__, NIB0__, NIB2__, NIB1__ },  1, 1 },
/* 0x312 */ { 0x0bc,           {  NIB3__, NIB2__, BLANK_, BLANK_ },  1, 1 },
/* 0x314 */ { 0x058,           {  NIB0__, NIB2__, NIB3__, NIB1__ },  1, 1 },
/* 0x316 */ { INPUT_PORT_B,    {  NIB0R2, BLANK_, BLANK_, BLANK_ },  0, 0 }, // // $4316   $000F   ---A    ...     2
/* 0x318 */ { 0x00e,           {  NIB3R3, NIB0__, NIB1__, NIB2__ },  0, 0 },
/* 0x31a */ { 0x092,           {  NIB3__, BLANK_, BLANK_, BLANK_ },  1, 1 },
/* 0x31c */ { 0x09a,           {  NIB0__, NIB2__, NIB3__, NIB1__ },  0, 0 },
/* 0x31e */ { 0x0a0,           {  BLANK_, NIB3__, BLANK_, NIB2__ },  1, 0 },
/* 0x320 */ { 0x08c,           {  NIB3__, BLANK_, BLANK_, BLANK_ },  0, 1 },
/* 0x322 */ { 0x08e,           {  NIB1__, NIB0__, NIB2__, NIB3__ },  0, 0 },
/* 0x324 */ { 0x0d2,           {  BLANK_, NIB2__, NIB3__, NIB1__ },  1, 1 },
/* 0x326 */ { 0x024,           {  NIB2__, NIB1__, NIB3__, BLANK_ },  0, 1 },
/* 0x328 */ { 0x006,           {  BLANK_, NIB2__, NIB3__, NIB1__ },  0, 0 },
/* 0x32a */ { 0x080,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x32c */ { 0x0e2,           {  NIB1R2, NIB2__, NIB3__, NIB0__ },  0, 1 },
/* 0x32e */ { 0x008,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x330 */ { 0x00c,           {  NIB0__, NIB2__, NIB3__, NIB1__ },  1, 0 },
/* 0x332 */ { 0x048,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x334 */ { 0x05c,           {  NIB0__, NIB3__, NIB2__, NIB1__ },  0, 0 },
/* 0x336 */ { 0x042,           {  NIB3__, NIB2__, BLANK_, BLANK_ },  0, 0 },
/* 0x338 */ { 0x076,           {  NIB2__, NIB0__, NIB1__, NIB3__ },  1, 1 },
/* 0x33a */ { 0x036,           {  NIB0__, NIB3__, NIB2__, NIB1__ },  1, 0 },
/* 0x33c */ { 0x0b0,           {  NIB1R3, NIB2__, NIB3__, BLANK_ },  0, 1 },
/* 0x33e */ { 0x056,           {  NIB1__, NIB0__, NIB2__, NIB3__ },  0, 1 },
/* 0x340 */ { 0x01a,           {  NIB3__, NIB2__, BLANK_, BLANK_ },  0, 0 },
/* 0x342 */ { 0x0bc,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x344 */ { 0x0aa,           {  BLANK_, NIB3__, BLANK_, BLANK_ },  1, 1 },
/* 0x346 */ { 0x078,           {  NIB1__, NIB2__, NIB3__, BLANK_ },  1, 0 },
/* 0x348 */ { 0x0dc,           {  NIB1__, NIB2__, NIB0__, NIB3__ },  0, 0 },
/* 0x34a */ { 0x06a,           {  NIB1R2, NIB2__, NIB3__, BLANK_ },  1, 0 },
/* 0x34c */ { INPUT_PORT_B,    {  NIB0R3, BLANK_, BLANK_, BLANK_ },  0, 0 }, // // $434C   $000F   ---A    ...     3
/* 0x34e */ { 0x03e,           {  NIB1__, NIB3__, NIB0__, NIB2__ },  1, 1 },
/* 0x350 */ { 0x0da,           {  NIB2__, NIB0__, NIB1__, NIB3__ },  0, 1 },
/* 0x352 */ { 0x098,           {  NIB3R3, NIB0__, NIB1__, NIB2__ },  1, 1 },
/* 0x354 */ { 0x01c,           {  NIB3__, NIB2__, NIB1__, NIB0__ },  1, 1 },
/* 0x356 */ { 0x034,           {  NIB2R2, NIB3__, NIB0__, NIB1__ },  1, 1 },
/* 0x358 */ { 0x0ba,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x35a */ { 0x08a,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x35c */ { 0x086,           {  NIB3__, NIB2__, NIB1__, NIB0__ },  0, 1 },
/* 0x35e */ { 0x09c,           {  NIB0R3, NIB1__, NIB2__, NIB3__ },  0, 1 },
/* 0x360 */ { 0x02c,           {  NIB0R1, NIB1__, NIB2__, NIB3__ },  1, 0 },
/* 0x362 */ { 0x080,           {  NIB3__, NIB0__, NIB2__, NIB1__ },  0, 0 },
/* 0x364 */ { 0x04a,           {  BLANK_, NIB3__, BLANK_, BLANK_ },  0, 1 },
/* 0x366 */ { 0x0c8,           {  NIB2__, NIB3__, NIB0__, NIB1__ },  1, 0 },
/* 0x368 */ { 0x05e,           {  NIB3R3, BLANK_, BLANK_, BLANK_ },  0, 0 },
/* 0x36a */ { 0x0f6,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x36c */ { 0x0fc,           {  NIB1__, NIB3__, NIB0__, NIB2__ },  1, 0 },
/* 0x36e */ { 0x048,           {  NIB1R1, NIB2__, NIB3__, NIB0__ },  0, 1 },
/* 0x370 */ { 0x0c4,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x372 */ { 0x020,           {  NIB0__, NIB1__, NIB3__, NIB2__ },  1, 0 },
/* 0x374 */ { 0x030,           {  NIB3R1, NIB0__, NIB1__, NIB2__ },  1, 0 },
/* 0x376 */ { 0x038,           {  NIB2__, NIB3__, BLANK_, BLANK_ },  1, 1 },
/* 0x378 */ { INPUT_PORT_A,    {  NIB1__, NIB0__, NIB2__, NIB3__ },  0, 0 },// $4378   $FFFF   DCAB    ...     0
/* 0x37a */ { 0x08e,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x37c */ { 0x010,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x37e */ { 0x0d0,           {  NIB1R1, NIB2__, NIB3__, BLANK_ },  1, 1 },
/* 0x380 */ { 0x084,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x382 */ { 0x078,           {  BLANK_, NIB3__, BLANK_, BLANK_ },  0, 1 },
/* 0x384 */ { 0x0ee,           {  NIB3R3, BLANK_, BLANK_, BLANK_ },  0, 1 },
/* 0x386 */ { 0x07a,           {  BLANK_, NIB2__, NIB3__, NIB1__ },  1, 0 },
/* 0x388 */ { 0x0b6,           {  NIB3__, NIB0__, NIB1__, NIB2__ },  0, 1 },
/* 0x38a */ { 0x084,           {  NIB1__, NIB0__, NIB3__, NIB2__ },  0, 1 },
/* 0x38c */ { 0x01a,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x38e */ { 0x004,           {  NIB1__, NIB2__, BLANK_, NIB3__ },  1, 1 },
/* 0x390 */ { 0x0fa,           {  BLANK_, NIB3__, BLANK_, BLANK_ },  1, 0 },
/* 0x392 */ { 0x0ae,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x394 */ { 0x0cc,           {  NIB3__, BLANK_, BLANK_, BLANK_ },  1, 0 },
/* 0x396 */ { 0x0e0,           {  NIB2__, NIB3__, BLANK_, BLANK_ },  1, 0 },
/* 0x398 */ { 0x024,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x39a */ { 0x0f6,           {  NIB2__, NIB3__, NIB1__, NIB0__ },  0, 0 },
/* 0x39c */ { 0x0a8,           {  NIB3__, NIB1__, NIB2__, NIB0__ },  0, 0 },
/* 0x39e */ { 0x0ec,           {  NIB1R2, NIB2__, NIB3__, BLANK_ },  0, 0 },
/* 0x3a0 */ { 0x070,           {  NIB2R1, NIB3__, BLANK_, BLANK_ },  0, 1 },
/* 0x3a2 */ { 0x0a6,           {  NIB0__, NIB1__, NIB3__, NIB2__ },  0, 1 },
/* 0x3a4 */ { 0x03c,           {  NIB2__, NIB1__, NIB3__, BLANK_ },  1, 0 },
/* 0x3a6 */ { 0x09e,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x3a8 */ { 0x0c2,           {  NIB2R2, NIB3__, NIB0__, NIB1__ },  1, 0 },
/* 0x3aa */ { 0x03a,           {  BLANK_, NIB3__, BLANK_, NIB2__ },  0, 1 },
/* 0x3ac */ { 0x0a4,           {  NIB2R3, NIB3__, NIB0__, NIB1__ },  0, 1 },
/* 0x3ae */ { 0x010,           {  NIB1__, NIB2__, NIB3__, NIB0__ },  0, 0 },
/* 0x3b0 */ { 0x0d4,           {  BLANK_, NIB3__, BLANK_, NIB2__ },  1, 1 },
/* 0x3b2 */ { 0x03c,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x3b4 */ { 0x082,           {  NIB3R3, BLANK_, BLANK_, BLANK_ },  1, 1 },
/* 0x3b6 */ { 0x00a,           {  NIB3R1, BLANK_, BLANK_, BLANK_ },  0, 0 },
/* 0x3b8 */ { 0x066,           {  NIB3__, NIB1__, NIB2__, NIB0__ },  0, 1 },
/* 0x3ba */ { 0x022,           {  NIB1__, NIB2__, NIB3__, BLANK_ },  0, 0 },
/* 0x3bc */ { 0x0be,           {  NIB1R3, NIB2__, NIB3__, BLANK_ },  1, 1 },
/* 0x3be */ { 0x078,           {  NIB2R1, NIB3__, BLANK_, BLANK_ },  0, 1 },
/* 0x3c0 */ { 0x0ca,           {  NIB2__, NIB0__, NIB1__, NIB3__ },  1, 0 },
/* 0x3c2 */ { 0x0ba,           {  NIB1__, NIB2__, BLANK_, NIB3__ },  0, 1 },
/* 0x3c4 */ { 0x0e6,           {  NIB2__, NIB3__, BLANK_, BLANK_ },  0, 1 },
/* 0x3c6 */ { 0x052,           {  NIB1R3, NIB2__, NIB3__, NIB0__ },  1, 1 },
/* 0x3c8 */ { 0x026,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x3ca */ { 0x0ce,           {  NIB1R1, NIB2__, NIB3__, NIB0__ },  1, 0 },
/* 0x3cc */ { 0x0f0,           {  NIB2__, NIB1__, NIB3__, NIB0__ },  0, 0 },
/* 0x3ce */ { 0x0f4,           {  NIB0R1, NIB1__, NIB2__, NIB3__ },  1, 1 },
/* 0x3d0 */ { 0x060,           {  NIB2R1, NIB3__, BLANK_, BLANK_ },  1, 1 },
/* 0x3d2 */ { 0x0e8,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  1, 0 },
/* 0x3d4 */ { 0x018,           {  NIB0__, NIB3__, NIB1__, NIB2__ },  1, 1 },
/* 0x3d6 */ { 0x088,           {  NIB3__, NIB0__, NIB1__, NIB2__ },  1, 0 },
/* 0x3d8 */ { 0x0e4,           {  NIB2R3, NIB3__, NIB0__, NIB1__ },  0, 0 },
/* 0x3da */ { 0x0ea,           {  NIB1R1, NIB2__, NIB3__, BLANK_ },  1, 0 },
/* 0x3dc */ { 0x0aa,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x3de */ { 0x06c,           {  NIB1__, NIB2__, NIB0__, NIB3__ },  1, 1 },
/* 0x3e0 */ { 0x044,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x3e2 */ { 0x046,           {  NIB3__, NIB2__, NIB0__, NIB1__ },  0, 0 },
/* 0x3e4 */ { 0x020,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x3e6 */ { 0x012,           {  BLANK_, NIB2__, NIB3__, NIB1__ },  0, 1 },
/* 0x3e8 */ { 0x008,           {  NIB1__, NIB2__, NIB3__, NIB0__ },  0, 1 },
/* 0x3ea */ { 0x062,           {  NIB2R3, NIB3__, BLANK_, BLANK_ },  1, 0 },
/* 0x3ec */ { 0x0c0,           {  NIB3R2, BLANK_, BLANK_, BLANK_ },  0, 1 },
/* 0x3ee */ { 0x008,           {  NIB0R1, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x3f0 */ { 0x056,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x3f2 */ { 0x0d8,           {  NIB1R3, NIB2__, NIB3__, BLANK_ },  0, 0 },
/* 0x3f4 */ { 0x0ee,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x3f6 */ { 0x07c,           {  NIB0R3, NIB1__, NIB2__, NIB3__ },  1, 0 },
/* 0x3f8 */ { 0x086,           {  NIB2__, NIB3__, NIB1__, NIB0__ },  0, 1 },
/* 0x3fa */ { 0x078,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x3fc */ { 0x03a,           {  NIB2__, NIB3__, BLANK_, BLANK_ },  0, 1 },
/* 0x3fe */ { 0x006,           {  NIB3__, NIB0__, NIB2__, NIB1__ },  0, 1 },
/* 0x400 */ { 0x0fc,           {  NIB0R2, NIB1__, NIB2__, NIB3__ },  1, 0 },
/* 0x402 */ { 0x01c,           {  NIB2R3, NIB3__, NIB0__, NIB1__ },  1, 1 },
/* 0x404 */ { 0x098,           {  NIB2R3, NIB3__, NIB0__, NIB1__ },  1, 0 },
/* 0x406 */ { 0x06c,           {  NIB1__, NIB2__, NIB3__, BLANK_ },  1, 1 },
/* 0x408 */ { 0x0cc,           {  NIB1R2, NIB2__, NIB3__, NIB0__ },  1, 1 },
/* 0x40a */ { 0x0d6,           {  NIB1__, NIB3__, NIB0__, NIB2__ },  0, 0 },
/* 0x40c */ { 0x0f0,           {  NIB2__, NIB1__, NIB0__, NIB3__ },  0, 0 },
/* 0x40e */ { 0x07a,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x410 */ { 0x09a,           {  NIB2R2, NIB3__, BLANK_, BLANK_ },  0, 1 },
/* 0x412 */ { 0x0c4,           {  NIB2R2, NIB3__, NIB0__, NIB1__ },  0, 1 },
/* 0x414 */ { INPUT_PORT_C,    {  NIB2__, NIB3__, BLANK_, BLANK_ },  0, 0 }, // $4414   $FF00   BA--    ...     0
/* 0x416 */ { 0x0ea,           {  NIB2__, NIB1__, NIB3__, NIB0__ },  0, 1 },
/* 0x418 */ { 0x074,           {  NIB3R2, NIB0__, NIB1__, NIB2__ },  0, 1 },
/* 0x41a */ { 0x096,           {  NIB2R2, NIB3__, BLANK_, BLANK_ },  0, 0 },
/* 0x41c */ { 0x0f2,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x41e */ { 0x08a,           {  NIB3R2, BLANK_, BLANK_, BLANK_ },  1, 0 },
/* 0x420 */ { 0x054,           {  NIB2__, NIB1__, NIB3__, BLANK_ },  1, 1 },
/* 0x422 */ { 0x05c,           {  NIB3R2, NIB0__, NIB1__, NIB2__ },  0, 0 },
/* 0x424 */ { 0x0c2,           {  NIB3R2, BLANK_, BLANK_, BLANK_ },  0, 0 },
/* 0x426 */ { 0x026,           {  BLANK_, NIB3__, BLANK_, BLANK_ },  0, 0 },
/* 0x428 */ { 0x088,           {  NIB2R2, NIB3__, BLANK_, BLANK_ },  1, 0 },
/* 0x42a */ { 0x08c,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x42c */ { 0x09c,           {  NIB3R1, BLANK_, BLANK_, BLANK_ },  1, 1 },
/* 0x42e */ { 0x01a,           {  NIB3__, BLANK_, BLANK_, BLANK_ },  1, 1 },
/* 0x430 */ { INPUT_PORT_B,    {  NIB1__, BLANK_, BLANK_, BLANK_ },  0, 0 }, // // $4430   $00F0   --A-    ...     0
/* 0x432 */ { 0x000,           {  NIB1R3, NIB2__, NIB3__, NIB0__ },  1, 0 },
/* 0x434 */ { 0x0d4,           {  NIB0R2, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x436 */ { 0x078,           {  NIB1__, NIB0__, NIB2__, NIB3__ },  0, 1 },
/* 0x438 */ { 0x070,           {  NIB3__, NIB2__, NIB0__, NIB1__ },  0, 1 },
/* 0x43a */ { 0x05e,           {  NIB2__, NIB3__, NIB0__, NIB1__ },  0, 1 },
/* 0x43c */ { 0x0f6,           {  NIB1__, NIB0__, NIB2__, NIB3__ },  1, 0 },
/* 0x43e */ { 0x082,           {  BLANK_, BLANK_, BLANK_, NIB3__ },  0, 1 },
/* 0x440 */ { 0x03e,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x442 */ { 0x0a6,           {  NIB3__, NIB2__, NIB1__, NIB0__ },  0, 0 },
/* 0x444 */ { 0x0b0,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x446 */ { 0x0de,           {  NIB1R2, NIB2__, NIB3__, BLANK_ },  1, 1 },
/* 0x448 */ { 0x0b6,           {  NIB1R2, NIB2__, NIB3__, NIB0__ },  1, 0 },
/* 0x44a */ { 0x002,           {  NIB3R1, NIB0__, NIB1__, NIB2__ },  0, 0 },
/* 0x44c */ { 0x090,           {  NIB3__, BLANK_, BLANK_, BLANK_ },  0, 0 },
/* 0x44e */ { 0x06e,           {  NIB3__, NIB1__, NIB2__, NIB0__ },  1, 1 },
/* 0x450 */ { 0x0a0,           {  BLANK_, BLANK_, BLANK_, NIB3__ },  1, 1 },
/* 0x452 */ { 0x0c8,           {  BLANK_, NIB3__, BLANK_, BLANK_ },  0, 1 },
/* 0x454 */ { 0x0f8,           {  NIB2R3, NIB3__, BLANK_, BLANK_ },  1, 1 },
/* 0x456 */ { 0x024,           {  NIB3__, BLANK_, BLANK_, BLANK_ },  0, 0 },
/* 0x458 */ { 0x0b6,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x45a */ { 0x070,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x45c */ { 0x0ee,           {  BLANK_, BLANK_, BLANK_, NIB3__ },  1, 0 },
/* 0x45e */ { 0x0b4,           {  NIB2__, NIB3__, NIB1__, NIB0__ },  1, 0 },
/* 0x460 */ { 0x0ca,           {  NIB2R1, NIB3__, BLANK_, BLANK_ },  0, 1 },
/* 0x462 */ { 0x01e,           {  NIB2R2, NIB3__, NIB0__, NIB1__ },  0, 0 },
/* 0x464 */ { 0x052,           {  NIB1R1, NIB2__, NIB3__, BLANK_ },  0, 0 },
/* 0x466 */ { 0x048,           {  BLANK_, NIB3__, BLANK_, BLANK_ },  1, 0 },
/* 0x468 */ { 0x02a,           {  NIB1__, NIB0__, NIB2__, NIB3__ },  1, 1 },
/* 0x46a */ { 0x02c,           {  NIB2R1, NIB3__, NIB0__, NIB1__ },  0, 0 },
/* 0x46c */ { 0x0a8,           {  NIB2__, NIB1__, NIB0__, NIB3__ },  1, 1 },
/* 0x46e */ { 0x010,           {  BLANK_, NIB3__, BLANK_, NIB2__ },  0, 0 },
/* 0x470 */ { 0x0ce,           {  NIB1__, NIB3__, NIB0__, NIB2__ },  0, 1 },
/* 0x472 */ { 0x078,           {  NIB3__, BLANK_, BLANK_, BLANK_ },  0, 0 },
/* 0x474 */ { 0x066,           {  NIB2__, NIB1__, BLANK_, NIB3__ },  1, 1 },
/* 0x476 */ { 0x05a,           {  NIB2__, NIB1__, NIB3__, NIB0__ },  1, 0 },
/* 0x478 */ { INPUT_PORT_C,    {  NIB1__, NIB0__, NIB3__, NIB2__ },  0, 0 },  //  $4478   $FFFF   CDAB    ...     0   (fghthist) - verify
/* 0x47a */ { 0x014,           {  NIB1__, NIB0__, NIB3__, NIB2__ },  1, 0 },
/* 0x47c */ { 0x0e8,           {  NIB2__, NIB3__, BLANK_, BLANK_ },  0, 0 },
/* 0x47e */ { 0x0b8,           {  NIB1R1, NIB2__, NIB3__, NIB0__ },  1, 1 },
/* 0x480 */ { 0x0e0,           {  NIB2__, NIB3__, BLANK_, BLANK_ },  1, 0 },
/* 0x482 */ { 0x012,           {  NIB3__, NIB0__, NIB1__, NIB2__ },  0, 0 },
/* 0x484 */ { 0x058,           {  NIB2__, NIB1__, NIB3__, BLANK_ },  0, 0 },
/* 0x486 */ { 0x036,           {  NIB0__, NIB3__, NIB2__, NIB1__ },  0, 1 },
/* 0x488 */ { INPUT_PORT_A,    {  NIB1__, NIB0__, NIB3__, NIB2__ },  0, 0 },// $4488   $FFFF   CDAB    ...     0
/* 0x48a */ { 0x07a,           {  NIB3R1, BLANK_, BLANK_, BLANK_ },  1, 0 },
/* 0x48c */ { 0x072,           {  NIB2__, NIB1__, NIB3__, BLANK_ },  0, 0 },
/* 0x48e */ { 0x0c6,           {  NIB1R2, NIB2__, NIB3__, NIB0__ },  1, 1 },
/* 0x490 */ { 0x0bc,           {  NIB2__, NIB1__, NIB0__, NIB3__ },  1, 0 },
/* 0x492 */ { 0x0fa,           {  NIB2__, NIB3__, NIB1__, NIB0__ },  1, 1 },
/* 0x494 */ { 0x0f4,           {  NIB3R1, NIB0__, NIB1__, NIB2__ },  1, 0 },
/* 0x496 */ { 0x046,           {  NIB2R3, NIB3__, NIB0__, NIB1__ },  0, 1 },
/* 0x498 */ { 0x0b2,           {  NIB1R1, NIB2__, NIB3__, BLANK_ },  0, 0 },
/* 0x49a */ { 0x004,           {  NIB3__, NIB0__, NIB1__, NIB2__ },  1, 0 },
/* 0x49c */ { 0x07e,           {  NIB1R1, NIB2__, NIB3__, NIB0__ },  0, 1 },
/* 0x49e */ { 0x04e,           {  NIB1__, NIB2__, NIB0__, NIB3__ },  1, 1 },
/* 0x4a0 */ { 0x0e2,           {  NIB2R3, NIB3__, NIB0__, NIB1__ },  0, 0 },
/* 0x4a2 */ { 0x094,           {  NIB2__, NIB3__, NIB0__, NIB1__ },  1, 0 },
/* 0x4a4 */ { 0x0ae,           {  BLANK_, NIB2__, NIB3__, NIB1__ },  1, 0 },
/* 0x4a6 */ { 0x0a8,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x4a8 */ { 0x092,           {  NIB2__, NIB3__, BLANK_, BLANK_ },  1, 1 },
/* 0x4aa */ { 0x0da,           {  NIB2R1, NIB3__, NIB0__, NIB1__ },  1, 1 },
/* 0x4ac */ { 0x080,           {  NIB2R1, NIB3__, NIB0__, NIB1__ },  1, 0 },
/* 0x4ae */ { 0x078,           {  NIB2__, NIB1__, BLANK_, NIB3__ },  0, 0 },
/* 0x4b0 */ { 0x0be,           {  NIB3__, BLANK_, BLANK_, BLANK_ },  0, 0 },
/* 0x4b2 */ { 0x04c,           {  NIB2__, NIB0__, NIB1__, NIB3__ },  1, 0 },
/* 0x4b4 */ { 0x032,           {  NIB2R3, NIB3__, BLANK_, BLANK_ },  0, 0 },
/* 0x4b6 */ { 0x0fe,           {  NIB2__, NIB3__, NIB0__, NIB1__ },  1, 1 },
/* 0x4b8 */ { 0x030,           {  NIB1R2, NIB2__, NIB3__, NIB0__ },  0, 0 },
/* 0x4ba */ { 0x0dc,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x4bc */ { 0x030,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x4be */ { 0x02e,           {  NIB1R2, NIB2__, NIB3__, BLANK_ },  0, 0 },
/* 0x4c0 */ { INPUT_PORT_C,    {  NIB3__, BLANK_, BLANK_, BLANK_ },  0, 0 },// $44C0   $F000   A---    ...     0
/* 0x4c2 */ { 0x03c,           {  NIB1R3, NIB2__, NIB3__, BLANK_ },  1, 0 },
/* 0x4c4 */ { 0x08c,           {  NIB0__, NIB2__, NIB3__, NIB1__ },  1, 0 },
/* 0x4c6 */ { 0x028,           {  NIB3R1, BLANK_, BLANK_, BLANK_ },  0, 0 },
/* 0x4c8 */ { 0x03e,           {  NIB3R1, NIB0__, NIB1__, NIB2__ },  0, 1 },
/* 0x4ca */ { 0x0d0,           {  NIB0R1, NIB1__, NIB2__, NIB3__ },  0, 1 },
/* 0x4cc */ { 0x0d4,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x4ce */ { 0x062,           {  NIB0R3, NIB1__, NIB2__, NIB3__ },  1, 0 },
/* 0x4d0 */ { 0x076,           {  NIB2__, NIB1__, NIB3__, NIB0__ },  1, 1 },
/* 0x4d2 */ { 0x00e,           {  BLANK_, NIB3__, BLANK_, NIB2__ },  1, 1 },
/* 0x4d4 */ { 0x038,           {  NIB3R1, NIB0__, NIB1__, NIB2__ },  0, 0 },
/* 0x4d6 */ { 0x0dc,           {  NIB1R3, NIB2__, NIB3__, NIB0__ },  0, 0 },
/* 0x4d8 */ { 0x0e6,           {  NIB2__, NIB3__, BLANK_, BLANK_ },  0, 0 },
/* 0x4da */ { 0x00c,           {  NIB3__, BLANK_, BLANK_, BLANK_ },  1, 1 },
/* 0x4dc */ { 0x0a4,           {  NIB0R1, NIB1__, NIB2__, NIB3__ },  1, 1 },
/* 0x4de */ { 0x0c0,           {  NIB1R1, NIB2__, NIB3__, BLANK_ },  0, 1 },
/* 0x4e0 */ { INPUT_PORT_B,    {  NIB0__, BLANK_, BLANK_, BLANK_ },  0, 0 }, //  // $44E0   $000F   ---A    ...     0
/* 0x4e2 */ { 0x0ba,           {  NIB3__, NIB0__, NIB1__, NIB2__ },  0, 1 },
/* 0x4e4 */ { 0x0ea,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x4e6 */ { 0x0ec,           {  NIB3R3, BLANK_, BLANK_, BLANK_ },  0, 0 },
/* 0x4e8 */ { 0x022,           {  NIB3__, NIB2__, NIB0__, NIB1__ },  0, 0 },
/* 0x4ea */ { 0x078,           {  NIB0R2, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x4ec */ { 0x0a2,           {  NIB3__, NIB2__, BLANK_, BLANK_ },  0, 0 },
/* 0x4ee */ { 0x068,           {  NIB1R3, NIB2__, NIB3__, NIB0__ },  1, 1 },
/* 0x4f0 */ { 0x050,           {  NIB0R1, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x4f2 */ { INPUT_PORT_B,    {  NIB1__, BLANK_, BLANK_, BLANK_ },  0, 0 }, // // $44F2   $00F0   --A-    ...     0
/* 0x4f4 */ { 0x074,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x4f6 */ { 0x060,           {  BLANK_, NIB2__, NIB3__, NIB1__ },  0, 1 },
/* 0x4f8 */ { 0x040,           {  NIB2R1, NIB3__, BLANK_, BLANK_ },  0, 0 },
/* 0x4fa */ { 0x0f2,           {  NIB3__, NIB2__, NIB0__, NIB1__ },  0, 1 },
/* 0x4fc */ { 0x064,           {  NIB2__, NIB1__, BLANK_, NIB3__ },  0, 1 },
/* 0x4fe */ { 0x084,           {  NIB1R2, NIB2__, NIB3__, BLANK_ },  0, 1 },
/* 0x500 */ { 0x016,           {  NIB3__, NIB0__, NIB2__, NIB1__ },  0, 0 },
/* 0x502 */ { 0x04a,           {  NIB1__, NIB2__, NIB3__, NIB0__ },  0, 1 },
/* 0x504 */ { 0x018,           {  NIB1__, NIB2__, NIB3__, NIB0__ },  0, 0 },
/* 0x506 */ { 0x084,           {  NIB1R3, NIB2__, NIB3__, NIB0__ },  1, 0 },
/* 0x508 */ { 0x034,           {  NIB0__, NIB3__, NIB2__, NIB1__ },  1, 1 },
/* 0x50a */ { INPUT_PORT_B,    {  NIB3__, BLANK_, BLANK_, BLANK_ },  0, 0 }, //  // $450A   $F000   A---    ...     0
/* 0x50c */ { 0x08e,           {  NIB1__, NIB0__, NIB3__, NIB2__ },  0, 1 },
/* 0x50e */ { 0x00a,           {  NIB3__, NIB2__, BLANK_, BLANK_ },  1, 0 },
/* 0x510 */ { 0x0ac,           {  NIB3__, NIB2__, BLANK_, BLANK_ },  1, 1 },
/* 0x512 */ { 0x0d2,           {  NIB2__, NIB1__, NIB3__, NIB0__ },  1, 0 },
/* 0x514 */ { 0x06a,           {  NIB2__, NIB1__, BLANK_, NIB3__ },  1, 0 },
/* 0x516 */ { 0x0b0,           {  NIB0R3, NIB1__, NIB2__, NIB3__ },  1, 1 },
/* 0x518 */ { 0x0aa,           {  NIB2__, NIB3__, BLANK_, BLANK_ },  0, 1 },
/* 0x51a */ { 0x09e,           {  NIB0__, NIB1__, NIB3__, NIB2__ },  0, 1 },
/* 0x51c */ { 0x044,           {  NIB0__, NIB3__, NIB2__, NIB1__ },  1, 0 },
/* 0x51e */ { 0x0e4,           {  BLANK_, BLANK_, BLANK_, NIB3__ },  0, 0 },
/* 0x520 */ { 0x042,           {  NIB1__, NIB3__, NIB0__, NIB2__ },  0, 0 },
/* 0x522 */ { 0x020,           {  NIB2R1, NIB3__, BLANK_, BLANK_ },  1, 1 },
/* 0x524 */ { 0x056,           {  BLANK_, NIB3__, BLANK_, NIB2__ },  1, 0 },
/* 0x526 */ { 0x078,           {  NIB2__, NIB0__, NIB1__, NIB3__ },  1, 1 },
/* 0x528 */ { 0x0ea,           {  NIB1R2, NIB2__, NIB3__, NIB0__ },  1, 1 },
/* 0x52a */ { 0x004,           {  NIB2R3, NIB3__, BLANK_, BLANK_ },  1, 0 },
/* 0x52c */ { INPUT_PORT_A,    {  NIB0R1, NIB1__, NIB2__, NIB3__ },  1, 0 },// $452C   $FFFF   DCBA    X..     1
/* 0x52e */ { 0x060,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x530 */ { 0x0f0,           {  NIB2R2, NIB3__, BLANK_, BLANK_ },  0, 0 },
/* 0x532 */ { 0x052,           {  NIB0R1, NIB1__, NIB2__, NIB3__ },  1, 0 },
/* 0x534 */ { 0x09c,           {  NIB0__, NIB1__, NIB3__, NIB2__ },  1, 0 },
/* 0x536 */ { 0x072,           {  NIB1__, NIB2__, NIB0__, NIB3__ },  0, 0 },
/* 0x538 */ { 0x038,           {  NIB2__, NIB1__, NIB3__, BLANK_ },  1, 0 },
/* 0x53a */ { 0x036,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x53c */ { 0x0de,           {  NIB2__, NIB0__, NIB1__, NIB3__ },  0, 1 },
/* 0x53e */ { 0x0de,           {  NIB1__, NIB3__, NIB0__, NIB2__ },  1, 1 },
/* 0x540 */ { 0x01e,           {  NIB2R1, NIB3__, NIB0__, NIB1__ },  0, 1 },
/* 0x542 */ { 0x092,           {  NIB2__, NIB3__, BLANK_, BLANK_ },  1, 0 },
/* 0x544 */ { 0x040,           {  NIB1__, NIB0__, NIB3__, NIB2__ },  1, 0 },
/* 0x546 */ { 0x04c,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x548 */ { 0x06c,           {  BLANK_, NIB3__, BLANK_, NIB2__ },  0, 1 },
/* 0x54a */ { 0x0fe,           {  NIB3__, BLANK_, BLANK_, BLANK_ },  1, 0 },
/* 0x54c */ { 0x068,           {  NIB0__, NIB3__, NIB1__, NIB2__ },  0, 1 },
/* 0x54e */ { 0x060,           {  NIB2__, NIB3__, NIB0__, NIB1__ },  0, 0 },
/* 0x550 */ { 0x08e,           {  NIB2R1, NIB3__, BLANK_, BLANK_ },  1, 0 },
/* 0x552 */ { 0x008,           {  NIB1R1, NIB2__, NIB3__, NIB0__ },  0, 0 },
/* 0x554 */ { 0x006,           {  BLANK_, NIB3__, BLANK_, BLANK_ },  1, 1 },
/* 0x556 */ { 0x0d6,           {  NIB0__, NIB2__, NIB3__, NIB1__ },  1, 1 },
/* 0x558 */ { 0x06a,           {  NIB3__, BLANK_, BLANK_, BLANK_ },  1, 1 },
/* 0x55a */ { 0x0be,           {  NIB3R2, NIB0__, NIB1__, NIB2__ },  0, 0 },
/* 0x55c */ { INPUT_PORT_A,    {  NIB0R1, NIB1__, NIB2__, NIB3__ },  0, 1 },// $455C   $FFFF   DCBA    .N.     1
/* 0x55e */ { 0x0fc,           {  NIB1R2, NIB2__, NIB3__, BLANK_ },  1, 1 },
/* 0x560 */ { 0x0fc,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x562 */ { 0x078,           {  NIB2R3, NIB3__, NIB0__, NIB1__ },  1, 1 },
/* 0x564 */ { INPUT_PORT_C,    {  NIB0__, NIB1__, NIB3__, NIB2__ },  0, 0 }, // // $4564   $FFFF   CDBA    ...     0
/* 0x566 */ { 0x0a4,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x568 */ { 0x0c6,           {  NIB3R3, BLANK_, BLANK_, BLANK_ },  1, 0 },
/* 0x56a */ { 0x0ca,           {  NIB1__, NIB0__, NIB3__, NIB2__ },  0, 0 },
/* 0x56c */ { 0x08c,           {  NIB0__, NIB3__, NIB1__, NIB2__ },  0, 0 },
/* 0x56e */ { 0x042,           {  NIB3__, NIB2__, NIB0__, NIB1__ },  1, 0 },
/* 0x570 */ { 0x05c,           {  NIB1__, NIB2__, BLANK_, NIB3__ },  1, 1 },
/* 0x572 */ { 0x0a0,           {  BLANK_, BLANK_, BLANK_, NIB3__ },  0, 1 },
/* 0x574 */ { 0x04c,           {  NIB3__, NIB1__, NIB2__, NIB0__ },  0, 0 },
/* 0x576 */ { 0x04a,           {  NIB2__, NIB1__, NIB3__, NIB0__ },  0, 1 },
/* 0x578 */ { 0x0d8,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x57a */ { 0x094,           {  NIB2__, NIB1__, NIB3__, BLANK_ },  1, 1 },
/* 0x57c */ { 0x002,           {  NIB1R1, NIB2__, NIB3__, BLANK_ },  1, 1 },
/* 0x57e */ { 0x020,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  1, 1 },
/* 0x580 */ { 0x088,           {  NIB2R2, NIB3__, NIB0__, NIB1__ },  1, 1 },
/* 0x582 */ { 0x0b6,           {  NIB2R1, NIB3__, NIB0__, NIB1__ },  0, 0 },
/* 0x584 */ { 0x054,           {  NIB3R2, BLANK_, BLANK_, BLANK_ },  1, 0 },
/* 0x586 */ { 0x0b8,           {  BLANK_, NIB3__, BLANK_, BLANK_ },  1, 0 },
/* 0x588 */ { 0x014,           {  NIB3__, NIB2__, NIB1__, NIB0__ },  0, 1 },
/* 0x58a */ { 0x05a,           {  NIB3__, NIB1__, NIB2__, NIB0__ },  1, 0 },
/* 0x58c */ { 0x0e0,           {  BLANK_, NIB3__, BLANK_, NIB2__ },  0, 0 },
/* 0x58e */ { 0x0c8,           {  NIB0__, NIB1__, NIB3__, NIB2__ },  0, 0 },
/* 0x590 */ { 0x07a,           {  NIB1__, NIB2__, NIB0__, NIB3__ },  1, 0 },
/* 0x592 */ { 0x062,           {  NIB1__, NIB0__, NIB2__, NIB3__ },  1, 0 },
/* 0x594 */ { 0x0f6,           {  NIB1R3, NIB2__, NIB3__, BLANK_ },  0, 1 },
/* 0x596 */ { 0x0bc,           {  BLANK_, NIB3__, BLANK_, BLANK_ },  0, 1 },
/* 0x598 */ { 0x07c,           {  NIB2__, NIB1__, NIB0__, NIB3__ },  1, 1 },
/* 0x59a */ { 0x09a,           {  NIB2__, NIB3__, BLANK_, BLANK_ },  0, 1 },
/* 0x59c */ { 0x0ce,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x59e */ { 0x078,           {  NIB3__, NIB0__, NIB2__, NIB1__ },  1, 0 },
/* 0x5a0 */ { 0x03c,           {  NIB2R2, NIB3__, BLANK_, BLANK_ },  0, 1 },
/* 0x5a2 */ { 0x0f2,           {  NIB0R2, NIB1__, NIB2__, NIB3__ },  1, 0 },
/* 0x5a4 */ { 0x006,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x5a6 */ { 0x03a,           {  BLANK_, NIB3__, BLANK_, BLANK_ },  1, 0 },
/* 0x5a8 */ { 0x0b0,           {  NIB3__, NIB2__, NIB1__, NIB0__ },  0, 0 },
/* 0x5aa */ { 0x0cc,           {  NIB3__, NIB0__, NIB1__, NIB2__ },  1, 1 },
/* 0x5ac */ { 0x02a,           {  NIB1__, NIB2__, NIB3__, NIB0__ },  1, 1 },
/* 0x5ae */ { 0x08a,           {  NIB3R2, NIB0__, NIB1__, NIB2__ },  0, 1 },
/* 0x5b0 */ { 0x0f8,           {  NIB3__, NIB2__, BLANK_, BLANK_ },  1, 0 },
/* 0x5b2 */ { 0x024,           {  NIB2R2, NIB3__, NIB0__, NIB1__ },  1, 0 },
/* 0x5b4 */ { 0x05e,           {  NIB1__, NIB2__, NIB3__, NIB0__ },  1, 0 },
/* 0x5b6 */ { 0x0e4,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x5b8 */ { 0x0b4,           {  NIB1__, NIB2__, NIB3__, BLANK_ },  0, 0 },
/* 0x5ba */ { 0x016,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x5bc */ { 0x06e,           {  NIB3R1, BLANK_, BLANK_, BLANK_ },  0, 1 },
/* 0x5be */ { 0x096,           {  NIB3__, NIB0__, NIB1__, NIB2__ },  1, 0 },
/* 0x5c0 */ { 0x0ee,           {  NIB3R3, BLANK_, BLANK_, BLANK_ },  0, 1 },
/* 0x5c2 */ { 0x010,           {  NIB1R1, NIB2__, NIB3__, BLANK_ },  0, 0 },
/* 0x5c4 */ { 0x0fa,           {  NIB1__, NIB2__, BLANK_, NIB3__ },  0, 0 },
/* 0x5c6 */ { 0x0c6,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x5c8 */ { INPUT_PORT_B,    {  NIB0R1, BLANK_, BLANK_, BLANK_ },  1, 0 },//// $45C8   $000F   ---A    X..     1
/* 0x5ca */ { 0x0c4,           {  NIB3R1, NIB0__, NIB1__, NIB2__ },  1, 0 },
/* 0x5cc */ { 0x032,           {  NIB2R3, NIB3__, NIB0__, NIB1__ },  0, 1 },
/* 0x5ce */ { 0x00e,           {  NIB2__, NIB1__, NIB3__, BLANK_ },  0, 1 },
/* 0x5d0 */ { 0x036,           {  NIB0__, NIB1__, NIB3__, NIB2__ },  1, 1 },
/* 0x5d2 */ { 0x0ba,           {  NIB0R3, NIB1__, NIB2__, NIB3__ },  0, 1 },
/* 0x5d4 */ { 0x034,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x5d6 */ { 0x0e4,           {  NIB1__, NIB2__, NIB3__, NIB0__ },  1, 0 },
/* 0x5d8 */ { 0x056,           {  NIB3__, NIB2__, BLANK_, BLANK_ },  1, 1 },
/* 0x5da */ { 0x078,           {  NIB1R2, NIB2__, NIB3__, NIB0__ },  1, 0 },
/* 0x5dc */ { 0x044,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  1, 1 },
/* 0x5de */ { 0x016,           {  NIB1__, NIB2__, NIB3__, NIB0__ },  1, 1 },
/* 0x5e0 */ { 0x084,           {  NIB0__, NIB3__, NIB2__, NIB1__ },  0, 0 },
/* 0x5e2 */ { 0x086,           {  NIB0__, NIB2__, NIB3__, NIB1__ },  1, 0 },
/* 0x5e4 */ { INPUT_PORT_B,    {  NIB0R2, BLANK_, BLANK_, BLANK_ },  1, 0 }, // // $45E4   $000F   ---A    X..     2
/* 0x5e6 */ { 0x0d8,           {  NIB1__, NIB0__, NIB3__, NIB2__ },  0, 0 },
/* 0x5e8 */ { 0x0f6,           {  NIB1R2, NIB2__, NIB3__, BLANK_ },  0, 1 },
/* 0x5ea */ { 0x0b8,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x5ec */ { 0x0dc,           {  NIB1R3, NIB2__, NIB3__, NIB0__ },  0, 0 },
/* 0x5ee */ { 0x0cc,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x5f0 */ { 0x02c,           {  NIB3__, NIB2__, BLANK_, BLANK_ },  0, 0 },
/* 0x5f2 */ { 0x012,           {  NIB2R1, NIB3__, NIB0__, NIB1__ },  1, 0 },
/* 0x5f4 */ { 0x018,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x5f6 */ { 0x07e,           {  NIB0__, NIB2__, NIB3__, NIB1__ },  0, 1 },
/* 0x5f8 */ { 0x066,           {  NIB3R1, BLANK_, BLANK_, BLANK_ },  1, 0 },
/* 0x5fa */ { 0x0ea,           {  NIB1R2, NIB2__, NIB3__, NIB0__ },  0, 1 },
/* 0x5fc */ { 0x0d0,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 1 },
/* 0x5fe */ { 0x0ac,           {  NIB3__, NIB2__, NIB1__, NIB0__ },  1, 0 },
/* 0x600 */ { 0x0a8,           {  BLANK_, NIB3__, BLANK_, BLANK_ },  1, 1 },
/* 0x602 */ { 0x092,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x604 */ { 0x034,           {  NIB1__, NIB2__, BLANK_, NIB3__ },  1, 0 },
/* 0x606 */ { 0x0e6,           {  NIB0__, NIB3__, NIB1__, NIB2__ },  0, 0 },
/* 0x608 */ { 0x048,           {  NIB2__, NIB1__, NIB3__, NIB0__ },  1, 1 },
/* 0x60a */ { 0x0e2,           {  NIB0__, NIB3__, NIB2__, NIB1__ },  0, 1 },
/* 0x60c */ { 0x0a6,           {  NIB2__, NIB3__, NIB0__, NIB1__ },  1, 1 },
/* 0x60e */ { 0x0da,           {  BLANK_, NIB3__, BLANK_, BLANK_ },  0, 1 },
/* 0x610 */ { 0x0c0,           {  NIB0R1, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x612 */ { 0x0b2,           {  NIB3R1, NIB0__, NIB1__, NIB2__ },  1, 0 },
/* 0x614 */ { 0x046,           {  NIB1__, NIB2__, NIB0__, NIB3__ },  0, 0 },
/* 0x616 */ { 0x078,           {  NIB1__, NIB3__, NIB0__, NIB2__ },  0, 1 },
/* 0x618 */ { 0x070,           {  NIB3R3, BLANK_, BLANK_, BLANK_ },  1, 0 },
/* 0x61a */ { 0x0d4,           {  NIB1__, NIB2__, NIB3__, NIB0__ },  0, 0 },
/* 0x61c */ { 0x09e,           {  NIB0__, NIB1__, NIB3__, NIB2__ },  0, 1 },
/* 0x61e */ { 0x028,           {  NIB2__, NIB3__, NIB1__, NIB0__ },  1, 1 },
/* 0x620 */ { INPUT_PORT_C,    {  NIB1__, NIB2__, NIB3__, NIB0__ },  0, 0 },// $4620   $FFFF   CBAD    ...     0
/* 0x622 */ { 0x090,           {  NIB2__, NIB1__, BLANK_, NIB3__ },  1, 0 },
/* 0x624 */ { 0x0c2,           {  NIB3__, NIB0__, NIB2__, NIB1__ },  1, 0 },
/* 0x626 */ { 0x0f4,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x628 */ { 0x0a4,           {  NIB3R1, BLANK_, BLANK_, BLANK_ },  0, 0 },
/* 0x62a */ { 0x03e,           {  NIB1__, NIB0__, NIB2__, NIB3__ },  0, 0 },
/* 0x62c */ { 0x058,           {  NIB1__, NIB2__, NIB3__, BLANK_ },  1, 0 },
/* 0x62e */ { 0x064,           {  NIB3__, BLANK_, BLANK_, BLANK_ },  1, 1 },
/* 0x630 */ { 0x01c,           {  NIB0__, NIB1__, NIB3__, NIB2__ },  1, 1 },
/* 0x632 */ { 0x02e,           {  BLANK_, BLANK_, BLANK_, NIB3__ },  0, 0 },
/* 0x634 */ { 0x04e,           {  NIB2__, NIB0__, NIB1__, NIB3__ },  1, 0 },
/* 0x636 */ { 0x018,           {  NIB2__, NIB3__, NIB1__, NIB0__ },  0, 0 },
/* 0x638 */ { 0x0a2,           {  NIB3R3, NIB0__, NIB1__, NIB2__ },  0, 0 },
/* 0x63a */ { 0x088,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x63c */ { INPUT_PORT_A,    {  NIB0__, NIB1__, NIB2__, NIB3__ },  1, 0 }, // $463C   $FFFF   DCBA    X..     0
/* 0x63e */ { 0x0da,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x640 */ { 0x022,           {  NIB1R2, NIB2__, NIB3__, BLANK_ },  1, 0 },
/* 0x642 */ { 0x030,           {  NIB2R1, NIB3__, BLANK_, BLANK_ },  1, 1 },
/* 0x644 */ { 0x000,           {  NIB1R3, NIB2__, NIB3__, BLANK_ },  1, 1 },
/* 0x646 */ { INPUT_PORT_A,    {  NIB1__, NIB2__, NIB3__, BLANK_ },  0, 0 },// $4646   $FFF0   CBA-    ..B     0
/* 0x648 */ { 0x00a,           {  NIB1R3, NIB2__, NIB3__, BLANK_ },  0, 0 },
/* 0x64a */ { 0x074,           {  NIB1R1, NIB2__, NIB3__, NIB0__ },  1, 0 },
/* 0x64c */ { 0x0ae,           {  NIB1__, NIB2__, BLANK_, NIB3__ },  0, 1 },
/* 0x64e */ { 0x05c,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x650 */ { 0x01a,           {  NIB2R1, NIB3__, NIB0__, NIB1__ },  0, 1 },
/* 0x652 */ { 0x078,           {  NIB2__, NIB3__, NIB1__, NIB0__ },  0, 0 },
/* 0x654 */ { INPUT_PORT_C,    {  NIB1__, NIB0__, NIB2__, NIB3__ },  0, 0 },// $4654   $FFFF   DCAB    ...     0
/* 0x656 */ { 0x0c0,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x658 */ { 0x0aa,           {  NIB0__, NIB3__, NIB1__, NIB2__ },  1, 0 },
/* 0x65a */ { INPUT_PORT_C,    {  NIB2__, NIB3__, NIB0__, NIB1__ },  0, 0 },// $465A   $FFFF   BADC    ...     0
/* 0x65c */ { 0x0e8,           {  NIB2__, NIB3__, NIB0__, NIB1__ },  0, 1 },
/* 0x65e */ { 0x0d2,           {  NIB1R1, NIB2__, NIB3__, NIB0__ },  0, 1 },
/* 0x660 */ { 0x0f4,           {  NIB2R1, NIB3__, BLANK_, BLANK_ },  0, 0 },
/* 0x662 */ { 0x010,           {  NIB3R2, NIB0__, NIB1__, NIB2__ },  1, 0 },
/* 0x664 */ { 0x080,           {  NIB0__, NIB3__, NIB2__, NIB1__ },  1, 1 },
/* 0x666 */ { 0x00c,           {  NIB3__, BLANK_, BLANK_, BLANK_ },  0, 1 },
/* 0x668 */ { 0x050,           {  NIB3R1, NIB0__, NIB1__, NIB2__ },  0, 1 },
/* 0x66a */ { 0x04a,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x66c */ { 0x09a,           {  NIB1R3, NIB2__, NIB3__, NIB0__ },  1, 1 },
/* 0x66e */ { 0x06e,           {  NIB2__, NIB1__, NIB3__, BLANK_ },  1, 1 },
/* 0x670 */ { INPUT_PORT_A,    {  NIB3__, BLANK_, BLANK_, BLANK_ },  0, 0 }, // $4670   $F000   A---    ..B     0
/* 0x672 */ { 0x072,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x674 */ { 0x074,           {  NIB3__, NIB2__, BLANK_, BLANK_ },  0, 1 },
/* 0x676 */ { 0x0ba,           {  NIB3R3, BLANK_, BLANK_, BLANK_ },  0, 0 },
/* 0x678 */ { 0x0c2,           {  NIB1R1, NIB2__, NIB3__, NIB0__ },  1, 1 },
/* 0x67a */ { 0x092,           {  NIB2__, NIB1__, NIB3__, NIB0__ },  1, 0 },
/* 0x67c */ { 0x01e,           {  NIB2R1, NIB3__, NIB0__, NIB1__ },  1, 1 },
/* 0x67e */ { 0x082,           {  NIB2__, NIB3__, NIB0__, NIB1__ },  1, 0 },
/* 0x680 */ { 0x036,           {  NIB2__, NIB1__, NIB3__, NIB0__ },  0, 0 },
/* 0x682 */ { 0x084,           {  NIB2R3, NIB3__, BLANK_, BLANK_ },  0, 0 },
/* 0x684 */ { 0x016,           {  NIB2R1, NIB3__, BLANK_, BLANK_ },  0, 1 },
/* 0x686 */ { 0x0d8,           {  NIB2__, NIB3__, BLANK_, BLANK_ },  1, 0 },
/* 0x688 */ { 0x086,           {  BLANK_, NIB3__, BLANK_, BLANK_ },  1, 0 },
/* 0x68a */ { 0x028,           {  BLANK_, NIB3__, BLANK_, NIB2__ },  0, 0 },
/* 0x68c */ { 0x040,           {  NIB3__, NIB0__, NIB1__, NIB2__ },  1, 1 },
/* 0x68e */ { 0x078,           {  NIB3R2, BLANK_, BLANK_, BLANK_ },  1, 1 },
/* 0x690 */ { 0x046,           {  NIB1R1, NIB2__, NIB3__, BLANK_ },  0, 1 },
/* 0x692 */ { 0x02e,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x694 */ { 0x0ea,           {  NIB1__, NIB0__, NIB2__, NIB3__ },  0, 1 },
/* 0x696 */ { 0x09a,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x698 */ { 0x068,           {  NIB3__, BLANK_, BLANK_, BLANK_ },  0, 0 },
/* 0x69a */ { 0x0d4,           {  NIB1__, NIB2__, BLANK_, NIB3__ },  1, 1 },
/* 0x69c */ { 0x070,           {  NIB3__, NIB2__, NIB1__, NIB0__ },  1, 1 },
/* 0x69e */ { 0x07c,           {  NIB2R2, NIB3__, NIB0__, NIB1__ },  1, 1 },
/* 0x6a0 */ { 0x0f6,           {  NIB2__, NIB0__, NIB1__, NIB3__ },  0, 0 },
/* 0x6a2 */ { 0x0e8,           {  NIB2R2, NIB3__, BLANK_, BLANK_ },  1, 1 },
/* 0x6a4 */ { 0x056,           {  BLANK_, NIB2__, NIB3__, NIB1__ },  0, 1 },
/* 0x6a6 */ { 0x0d2,           {  NIB1__, NIB0__, NIB3__, NIB2__ },  1, 1 },
/* 0x6a8 */ { 0x026,           {  BLANK_, BLANK_, BLANK_, NIB3__ },  1, 1 },
/* 0x6aa */ { 0x0a8,           {  NIB0R3, NIB1__, NIB2__, NIB3__ },  1, 0 },
/* 0x6ac */ { 0x0c6,           {  NIB3__, NIB1__, NIB2__, NIB0__ },  1, 1 },
/* 0x6ae */ { 0x0be,           {  NIB3__, NIB2__, BLANK_, BLANK_ },  0, 0 },
/* 0x6b0 */ { 0x062,           {  NIB1__, NIB0__, NIB3__, NIB2__ },  0, 1 },
/* 0x6b2 */ { 0x094,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x6b4 */ { 0x0ae,           {  NIB1R3, NIB2__, NIB3__, BLANK_ },  1, 0 },
/* 0x6b6 */ { 0x006,           {  NIB3__, NIB2__, NIB0__, NIB1__ },  1, 1 },
/* 0x6b8 */ { 0x0a4,           {  BLANK_, NIB3__, BLANK_, NIB2__ },  1, 1 },
/* 0x6ba */ { 0x050,           {  NIB2R2, NIB3__, NIB0__, NIB1__ },  0, 0 },
/* 0x6bc */ { 0x0f2,           {  NIB2__, NIB3__, BLANK_, BLANK_ },  0, 1 },
/* 0x6be */ { 0x07a,           {  NIB2R1, NIB3__, BLANK_, BLANK_ },  1, 0 },
/* 0x6c0 */ { 0x06c,           {  NIB3R2, NIB0__, NIB1__, NIB2__ },  1, 1 },
/* 0x6c2 */ { 0x00e,           {  NIB0R1, NIB1__, NIB2__, NIB3__ },  0, 1 },
/* 0x6c4 */ { 0x054,           {  NIB3R2, NIB0__, NIB1__, NIB2__ },  0, 1 },
/* 0x6c6 */ { 0x07e,           {  NIB0R3, NIB1__, NIB2__, NIB3__ },  0, 1 },
/* 0x6c8 */ { 0x018,           {  NIB1R1, NIB2__, NIB3__, BLANK_ },  1, 1 },
/* 0x6ca */ { 0x078,           {  NIB1R3, NIB2__, NIB3__, BLANK_ },  1, 1 },
/* 0x6cc */ { 0x01a,           {  NIB3__, NIB2__, NIB1__, NIB0__ },  0, 1 },
/* 0x6ce */ { 0x0de,           {  NIB2__, NIB1__, BLANK_, NIB3__ },  0, 0 },
/* 0x6d0 */ { 0x04a,           {  NIB3__, BLANK_, BLANK_, BLANK_ },  0, 0 },
/* 0x6d2 */ { 0x08c,           {  NIB3R3, NIB0__, NIB1__, NIB2__ },  1, 0 },
/* 0x6d4 */ { 0x014,           {  NIB0R1, NIB1__, NIB2__, NIB3__ },  1, 0 },
/* 0x6d6 */ { 0x0cc,           {  NIB2R3, NIB3__, BLANK_, BLANK_ },  1, 0 },
/* 0x6d8 */ { 0x00a,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x6da */ { 0x002,           {  NIB0__, NIB3__, NIB2__, NIB1__ },  1, 0 },
/* 0x6dc */ { 0x05a,           {  NIB3R3, NIB0__, NIB1__, NIB2__ },  1, 1 },
/* 0x6de */ { 0x0aa,           {  NIB1R3, NIB2__, NIB3__, NIB0__ },  1, 0 },
/* 0x6e0 */ { 0x090,           {  BLANK_, NIB3__, BLANK_, BLANK_ },  1, 1 },
/* 0x6e2 */ { 0x03c,           {  NIB3__, NIB2__, BLANK_, BLANK_ },  1, 1 },
/* 0x6e4 */ { 0x080,           {  NIB0R2, NIB1__, NIB2__, NIB3__ },  1, 0 },
/* 0x6e6 */ { 0x044,           {  NIB3R1, BLANK_, BLANK_, BLANK_ },  0, 1 },
/* 0x6e8 */ { 0x072,           {  BLANK_, NIB2__, NIB3__, NIB1__ },  1, 1 },
/* 0x6ea */ { 0x098,           {  BLANK_, NIB3__, BLANK_, BLANK_ },  0, 0 },
/* 0x6ec */ { 0x038,           {  NIB3R2, BLANK_, BLANK_, BLANK_ },  0, 0 },
/* 0x6ee */ { 0x04c,           {  NIB2__, NIB1__, NIB3__, BLANK_ },  0, 1 },
/* 0x6f0 */ { 0x022,           {  NIB2R3, NIB3__, BLANK_, BLANK_ },  0, 1 },
/* 0x6f2 */ { 0x000,           {  NIB0__, NIB2__, NIB3__, NIB1__ },  0, 0 },
/* 0x6f4 */ { 0x0fa,           {  NIB3__, NIB0__, NIB2__, NIB1__ },  0, 1 },
/* 0x6f6 */ { 0x00c,           {  BLANK_, NIB3__, BLANK_, BLANK_ },  0, 1 },
/* 0x6f8 */ { INPUT_PORT_A,    {  NIB2__, NIB3__, NIB0__, NIB1__,},  0, 0 }, // $46F8   $FFFF   BADC    ...     0   (edrandy)
/* 0x6fa */ { 0x004,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x6fc */ { 0x066,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x6fe */ { 0x08a,           {  NIB1R1, NIB2__, NIB3__, BLANK_ },  0, 0 },
/* 0x700 */ { 0x0b0,           {  NIB2R2, NIB3__, NIB0__, NIB1__ },  1, 0 },
/* 0x702 */ { 0x012,           {  BLANK_, NIB2__, NIB3__, NIB1__ },  1, 0 },
/* 0x704 */ { 0x066,           {  NIB3R3, BLANK_, BLANK_, BLANK_ },  0, 1 },
/* 0x706 */ { 0x078,           {  NIB2R2, NIB3__, BLANK_, BLANK_ },  1, 0 },
/* 0x708 */ { 0x02c,           {  NIB2R3, NIB3__, NIB0__, NIB1__ },  0, 0 },
/* 0x70a */ { 0x09c,           {  NIB2__, NIB3__, BLANK_, BLANK_ },  1, 1 },
/* 0x70c */ { 0x0bc,           {  NIB2__, NIB3__, NIB1__, NIB0__ },  1, 0 },
/* 0x70e */ { 0x004,           {  NIB3__, NIB0__, NIB1__, NIB2__ },  0, 0 },
/* 0x710 */ { 0x0ca,           {  NIB3__, NIB2__, NIB1__, NIB0__ },  0, 0 },
/* 0x712 */ { INPUT_PORT_B,    {  NIB0R1, BLANK_, BLANK_, BLANK_ },  0, 1 }, // // $4712   $000F   ---A    .N.     1
/* 0x714 */ { 0x00a,           {  NIB1R1, NIB2__, NIB3__, NIB0__ },  0, 0 },
/* 0x716 */ { INPUT_PORT_A,    {  NIB0R1, NIB1__, NIB2__, NIB3__ },  0, 0 },// $4716   $FFFF   DCBA    ...     1
/* 0x718 */ { 0x0a2,           {  NIB2R3, NIB3__, NIB0__, NIB1__ },  1, 1 },
/* 0x71a */ { 0x0ac,           {  NIB1__, NIB2__, NIB0__, NIB3__ },  1, 1 },
/* 0x71c */ { 0x0ce,           {  NIB3__, NIB1__, NIB2__, NIB0__ },  0, 0 },
/* 0x71e */ { 0x08e,           {  NIB2__, NIB1__, NIB0__, NIB3__ },  1, 1 },
/* 0x720 */ { 0x034,           {  NIB0R1, NIB1__, NIB2__, NIB3__ },  1, 1 },
/* 0x722 */ { INPUT_PORT_C,    {  NIB0R3, NIB1__, NIB2__, NIB3__ },  0, 0 },// $4722   $FFFF   DCBA    ...     0
/* 0x724 */ { 0x0d6,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x726 */ { 0x0fc,           {  NIB2__, NIB3__, BLANK_, BLANK_ },  1, 0 },
/* 0x728 */ { 0x0b6,           {  NIB1__, NIB3__, NIB0__, NIB2__ },  1, 0 },
/* 0x72a */ { 0x0ec,           {  NIB2__, NIB3__, BLANK_, BLANK_ },  0, 0 },
/* 0x72c */ { 0x094,           {  NIB3__, NIB1__, NIB2__, NIB0__ },  0, 1 },
/* 0x72e */ { INPUT_PORT_A,    {  NIB0R3, NIB1__, NIB2__, NIB3__ },  0, 0 }, // $472E   $FFFF   DCBA    ...     3
/* 0x730 */ { 0x02e,           {  NIB1R2, NIB2__, NIB3__, NIB0__ },  0, 0 },
/* 0x732 */ { 0x09e,           {  NIB3R1, BLANK_, BLANK_, BLANK_ },  1, 1 },
/* 0x734 */ { 0x05c,           {  NIB1__, NIB0__, NIB2__, NIB3__ },  1, 1 },
/* 0x736 */ { 0x042,           {  NIB3__, NIB0__, NIB2__, NIB1__ },  1, 1 },
/* 0x738 */ { 0x0d0,           {  NIB1__, NIB3__, NIB0__, NIB2__ },  1, 1 },
/* 0x73a */ { 0x0f0,           {  NIB3__, NIB2__, BLANK_, BLANK_ },  0, 1 },
/* 0x73c */ { 0x0e4,           {  NIB3R2, BLANK_, BLANK_, BLANK_ },  0, 1 },
/* 0x73e */ { 0x04e,           {  NIB3R2, NIB0__, NIB1__, NIB2__ },  0, 0 },
/* 0x740 */ { 0x0cc,           {  NIB2__, NIB3__, BLANK_, BLANK_ },  0, 1 },
/* 0x742 */ { 0x078,           {  NIB3R2, BLANK_, BLANK_, BLANK_ },  0, 1 },
/* 0x744 */ { 0x010,           {  NIB0__, NIB1__, NIB3__, NIB2__ },  1, 0 },
/* 0x746 */ { 0x0b8,           {  NIB1R2, NIB2__, NIB3__, BLANK_ },  1, 1 },
/* 0x748 */ { 0x0c4,           {  NIB2__, NIB1__, NIB3__, BLANK_ },  0, 0 },
/* 0x74a */ { 0x052,           {  NIB2R1, NIB3__, NIB0__, NIB1__ },  0, 0 },
/* 0x74c */ { 0x06a,           {  NIB3__, BLANK_, BLANK_, BLANK_ },  1, 0 },
/* 0x74e */ { 0x064,           {  NIB0R3, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x750 */ { 0x0da,           {  NIB3__, NIB2__, BLANK_, BLANK_ },  1, 0 },
/* 0x752 */ { 0x076,           {  NIB1R3, NIB2__, NIB3__, BLANK_ },  0, 1 },
/* 0x754 */ { 0x03e,           {  NIB3__, NIB0__, NIB2__, NIB1__ },  1, 1 },
/* 0x756 */ { INPUT_PORT_C,    {  NIB3__, NIB0__, NIB1__, NIB2__ },  0, 0 },// $4756   $FFFF   ADCB    ...     0
/* 0x758 */ { 0x0fe,           {  BLANK_, BLANK_, BLANK_, NIB3__ },  1, 0 },
/* 0x75a */ { 0x058,           {  NIB2__, NIB1__, NIB0__, NIB3__ },  0, 0 },
/* 0x75c */ { 0x02a,           {  BLANK_, NIB3__, BLANK_, NIB2__ },  1, 0 },
/* 0x75e */ { 0x060,           {  NIB2R3, NIB3__, NIB0__, NIB1__ },  0, 1 },
/* 0x760 */ { 0x0f4,           {  NIB2__, NIB3__, BLANK_, BLANK_ },  0, 0 },
/* 0x762 */ { 0x082,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x764 */ { 0x008,           {  NIB2R2, NIB3__, NIB0__, NIB1__ },  0, 1 },
/* 0x766 */ { INPUT_PORT_A,    {  NIB0__, NIB1__, NIB3__, NIB2__ },  0, 0 }, // $4766   $FFFF   CDBA    ...     0
/* 0x768 */ { 0x0c8,           {  NIB1R2, NIB2__, NIB3__, NIB0__ },  1, 0 },
/* 0x76a */ { INPUT_PORT_B,    {  NIB0__, BLANK_, BLANK_, BLANK_ },  0, 0 }, //  $476A   $000F   ---A    ...     0    (standard i/o read shanghai)
/* 0x76c */ { 0x088,           {  NIB3__, NIB2__, NIB0__, NIB1__ },  0, 1 },
/* 0x76e */ { 0x06e,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x770 */ { 0x01c,           {  BLANK_, BLANK_, BLANK_, NIB3__ },  0, 1 },
/* 0x772 */ { 0x0d6,           {  NIB2R3, NIB3__, NIB0__, NIB1__ },  1, 0 },
/* 0x774 */ { 0x0e0,           {  BLANK_, NIB3__, BLANK_, NIB2__ },  0, 1 },
/* 0x776 */ { 0x020,           {  NIB1__, NIB2__, BLANK_, NIB3__ },  0, 0 },
/* 0x778 */ { 0x0a6,           {  NIB2__, NIB1__, NIB3__, NIB0__ },  0, 1 },
/* 0x77a */ { 0x030,           {  NIB3R3, BLANK_, BLANK_, BLANK_ },  1, 1 },
/* 0x77c */ { 0x0fa,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x77e */ { 0x078,           {  NIB0R2, NIB1__, NIB2__, NIB3__ },  0, 1 },
/* 0x780 */ { 0x0c0,           {  BLANK_, NIB2__, NIB3__, NIB1__ },  0, 0 },
/* 0x782 */ { 0x0b2,           {  NIB1R1, NIB2__, NIB3__, BLANK_ },  1, 0 },
/* 0x784 */ { 0x0e2,           {  NIB3__, NIB2__, NIB0__, NIB1__ },  0, 0 },
/* 0x786 */ { 0x024,           {  BLANK_, NIB3__, BLANK_, BLANK_ },  0, 0 },
/* 0x788 */ { INPUT_PORT_C,    {  NIB0R2, NIB1__, NIB2__, NIB3__ },  0, 0 },// $4788   $FFFF   DCBA    ...     2
/* 0x78a */ { 0x05e,           {  NIB1__, NIB2__, NIB3__, BLANK_ },  0, 1 },
/* 0x78c */ { 0x03a,           {  NIB0__, NIB3__, NIB1__, NIB2__ },  1, 1 },
/* 0x78e */ { 0x032,           {  NIB1R2, NIB2__, NIB3__, NIB0__ },  1, 1 },
/* 0x790 */ { INPUT_PORT_A,    {  NIB3__, NIB1__, NIB2__, NIB0__ },  0, 0 }, // $4790   $FFFF   ACBD    ...     0
/* 0x792 */ { 0x0e6,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  1, 0 },
/* 0x794 */ { 0x096,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x796 */ { 0x0ee,           {  NIB0R2, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x798 */ { 0x0f8,           {  NIB3R3, NIB0__, NIB1__, NIB2__ },  0, 1 },
/* 0x79a */ { 0x0b4,           {  NIB0R2, NIB1__, NIB2__, NIB3__ },  1, 1 },
/* 0x79c */ { 0x096,           {  NIB2R2, NIB3__, BLANK_, BLANK_ },  0, 0 },
/* 0x79e */ { 0x0a0,           {  NIB1__, NIB2__, NIB3__, BLANK_ },  1, 1 },
/* 0x7a0 */ { 0x0a0,           {  NIB2R2, NIB3__, BLANK_, BLANK_ },  0, 1 },
/* 0x7a2 */ { 0x048,           {  NIB2__, NIB1__, BLANK_, NIB3__ },  0, 1 },
/* 0x7a4 */ { 0x0f6,           {  NIB2__, NIB0__, NIB1__, NIB3__ },  0, 1 },
/* 0x7a6 */ { 0x006,           {  NIB2__, NIB1__, NIB0__, NIB3__ },  1, 0 },
/* 0x7a8 */ { 0x0de,           {  NIB2__, NIB1__, NIB0__, NIB3__ },  0, 1 },
/* 0x7aa */ { 0x0cc,           {  NIB3__, NIB0__, NIB1__, NIB2__ },  0, 1 },
/* 0x7ac */ { 0x0a4,           {  NIB1__, NIB3__, NIB0__, NIB2__ },  0, 1 },
/* 0x7ae */ { 0x07c,           {  NIB0R2, NIB1__, NIB2__, NIB3__ },  0, 1 },
/* 0x7b0 */ { 0x0ce,           {  NIB1R3, NIB2__, NIB3__, NIB0__ },  0, 1 },
/* 0x7b2 */ { 0x0ba,           {  NIB3__, NIB0__, NIB1__, NIB2__ },  1, 0 },
/* 0x7b4 */ { 0x01e,           {  NIB0__, NIB3__, NIB1__, NIB2__ },  0, 1 },
/* 0x7b6 */ { 0x0da,           {  NIB3__, BLANK_, BLANK_, BLANK_ },  1, 1 },
/* 0x7b8 */ { 0x0aa,           {  NIB3R1, NIB0__, NIB1__, NIB2__ },  0, 0 },
/* 0x7ba */ { 0x078,           {  NIB1R3, NIB2__, NIB3__, BLANK_ },  0, 0 },
/* 0x7bc */ { 0x076,           {  NIB2__, NIB3__, NIB0__, NIB1__ },  0, 0 },
/* 0x7be */ { 0x0b6,           {  NIB3__, NIB2__, BLANK_, BLANK_ },  1, 0 },
/* 0x7c0 */ { 0x058,           {  NIB1__, NIB3__, NIB0__, NIB2__ },  0, 0 },
/* 0x7c2 */ { 0x050,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x7c4 */ { 0x060,           {  NIB0R3, NIB1__, NIB2__, NIB3__ },  1, 1 },
/* 0x7c6 */ { 0x05e,           {  NIB0__, NIB2__, NIB3__, NIB1__ },  1, 1 },
/* 0x7c8 */ { 0x094,           {  NIB2R2, NIB3__, BLANK_, BLANK_ },  1, 0 },
/* 0x7ca */ { 0x000,           {  NIB3__, NIB2__, NIB0__, NIB1__ },  1, 0 },
/* 0x7cc */ { 0x03e,           {  BLANK_, NIB3__, BLANK_, BLANK_ },  1, 0 },
/* 0x7ce */ { INPUT_PORT_C,    {  NIB0R1, NIB1__, NIB2__, NIB3__ },  0, 0 }, // $47CE   $FFFF   DCBA    ...     1
/* 0x7d0 */ { 0x0d0,           {  NIB3__, NIB0__, NIB2__, NIB1__ },  0, 0 },
/* 0x7d2 */ { 0x0be,           {  NIB3R1, NIB0__, NIB1__, NIB2__ },  1, 1 },
/* 0x7d4 */ { 0x00c,           {  NIB1R2, NIB2__, NIB3__, BLANK_ },  0, 0 },
/* 0x7d6 */ { INPUT_PORT_A,    {  NIB0R2, NIB1__, NIB2__, NIB3__ },  1, 0 },// $47D6   $FFFF   DCBA    X..     2
/* 0x7d8 */ { 0x01a,           {  NIB2R3, NIB3__, BLANK_, BLANK_ },  1, 1 },
/* 0x7da */ { 0x0e6,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x7dc */ { 0x0d6,           {  NIB1__, NIB2__, NIB3__, BLANK_ },  0, 0 },
/* 0x7de */ { 0x05c,           {  NIB1__, NIB2__, NIB0__, NIB3__ },  1, 0 },
/* 0x7e0 */ { 0x00a,           {  NIB2__, NIB0__, NIB1__, NIB3__ },  1, 1 },
/* 0x7e2 */ { 0x0c4,           {  NIB1__, NIB0__, NIB2__, NIB3__ },  1, 0 },
/* 0x7e4 */ { 0x0e4,           {  NIB3__, BLANK_, BLANK_, BLANK_ },  1, 0 },
/* 0x7e6 */ { 0x0a6,           {  NIB3__, NIB1__, NIB2__, NIB0__ },  1, 0 },
/* 0x7e8 */ { 0x058,           {  NIB0__, NIB1__, NIB2__, NIB3__ },  0, 0 },
/* 0x7ea */ { 0x040,           {  NIB0__, NIB1__, NIB3__, NIB2__ },  0, 0 },
/* 0x7ec */ { 0x046,           {  NIB3__, BLANK_, BLANK_, BLANK_ },  0, 1 },
/* 0x7ee */ { 0x0ea,           {  NIB2__, NIB3__, BLANK_, BLANK_ },  1, 1 },
/* 0x7f0 */ { 0x090,           {  NIB2__, NIB1__, BLANK_, NIB3__ },  1, 1 },
/* 0x7f2 */ { 0x08e,           {  NIB1__, NIB2__, NIB3__, NIB0__ },  0, 1 },
/* 0x7f4 */ { INPUT_PORT_A,    {  NIB0R2, NIB1__, NIB2__, NIB3__ },  0, 1 }, // // $47F4   $FFFF   DCBA    .N.     2
/* 0x7f6 */ { 0x078,           {  NIB3__, BLANK_, BLANK_, BLANK_ },  0, 1 },
/* 0x7f8 */ { 0x004,           {  NIB2__, NIB1__, NIB3__, BLANK_ },  1, 0 },
/* 0x7fa */ { 0x02e,           {  NIB2__, NIB3__, NIB1__, NIB0__ },  0, 1 },
/* 0x7fc */ { 0x06e,           {  NIB1__, NIB0__, NIB3__, NIB2__ },  1, 0 },
/* 0x7fe */ { 0x04c,           {  NIB1__, NIB2__, NIB0__, NIB3__ },  0, 1 }
};

UINT16 reorder(UINT16 input, UINT8 *weights)
{
	UINT16 temp = 0;
	for(int i = 0; i < 16; i++)
	{
		if(input & (1 << i)) // if input bit is set
		{
			if(weights[i] != 0xFF) // and weight exists for output bit
			{
				temp |= 1 << weights[i]; // set that bit
			}
		}
	}
	return temp;
}



/* there are probably less dumb ways of doing the CS logic, it could be hooked up
   more like the system16 mapper chips */

void deco_146_base_device::write_data(address_space &space, UINT16 address, UINT16 data, UINT16 mem_mask, UINT8 &csflags)
{
	address = BITSWAP16(address>>1, 15,14,13,12,11,10, m_external_addrswap[9],m_external_addrswap[8] ,m_external_addrswap[7],m_external_addrswap[6],m_external_addrswap[5],m_external_addrswap[4],m_external_addrswap[3],m_external_addrswap[2],m_external_addrswap[1],m_external_addrswap[0]) << 1;

	csflags = 0;
	int upper_addr_bits = (address & 0x7800) >> 11;

	if (upper_addr_bits == 0x8) // configuration registers are hardcoded to this area
	{
		int real_address = address & 0xf;
		logerror("write to config regs %04x %04x %04x\n", real_address, data, mem_mask);

		if ((real_address>=0x2) && (real_address<=0x0c))
		{
			region_selects[(real_address-2)/2] = data &0xf;
			return;
		}
		else
		{
			// unknown
		}

		return; // or fall through?
	}

	for (int i=0;i<6;i++)
	{
		int cs = region_selects[i];

		if (cs==upper_addr_bits)
		{
			int real_address = address & 0x7ff;
			csflags |= (1 << i);

			if (i==0) // the first cs is our internal protection area
			{
				logerror("write matches cs table (protection) %01x %04x %04x %04x\n", i, real_address, data, mem_mask);
				write_protport(space, real_address, data, mem_mask);
			}
			else
			{
				logerror("write matches cs table (external connection) %01x %04x %04x %04x\n", i, real_address, data, mem_mask);
			}
		}
	}

	if (csflags==0)
	{
		logerror("write not in cs table\n");
	}
}






UINT16 deco_146_base_device::read_protport(UINT16 address, UINT16 mem_mask)
{
	// if we read the last written address immediately after then ignore all other logic and just return what was written unmodified
	if ((address==m_latchaddr) && (m_latchflag==1))
	{
		logerror("returning latched data %04x\n", m_latchdata);
		m_latchflag = 0;
		return m_latchdata;
	}

	m_latchflag = 0;

	if (m_magic_read_address_xor_enabled) address ^= m_magic_read_address_xor;

	int location = 0;
	UINT16 realret = read_data_getloc(address, location);






	if (location == m_bankswitch_swap_read_address) // this has a special meaning
	{
	//  logerror("(bankswitch) %04x %04x\n", address, mem_mask);

		if (m_current_rambank==0)
			m_current_rambank = 1;
		else
			m_current_rambank = 0;
	}


	return realret;
}

void deco_146_base_device::write_protport(address_space &space, UINT16 address, UINT16 data, UINT16 mem_mask)
{
	m_latchaddr = address;
	m_latchdata = data;
	m_latchflag = 1;

	if ((address&0xff) == m_xor_port)
	{
			logerror("LOAD XOR REGISTER %04x %04x\n", data, mem_mask);
			COMBINE_DATA(&m_xor);
	}
	else if ((address&0xff) == m_mask_port)
	{
			logerror("LOAD NAND REGISTER %04x %04x\n", data, mem_mask);
			COMBINE_DATA(&m_nand);
	}
	else if ((address&0xff) == m_soundlatch_port)
	{
			logerror("LOAD SOUND LATCH %04x %04x\n", data, mem_mask);
			COMBINE_DATA(&m_soundlatch);
			m_soundlatch_w(space, data, mem_mask);
	}

	// always store
	if (m_current_rambank==0)
		COMBINE_DATA(&m_rambank0[(address&0xff)>>1]);
	else
		COMBINE_DATA(&m_rambank1[(address&0xff)>>1]);

}



UINT16 deco_146_base_device::read_data(UINT16 address, UINT16 mem_mask, UINT8 &csflags)
{
	address = BITSWAP16(address>>1, 15,14,13,12,11,10, m_external_addrswap[9],m_external_addrswap[8] ,m_external_addrswap[7],m_external_addrswap[6],m_external_addrswap[5],m_external_addrswap[4],m_external_addrswap[3],m_external_addrswap[2],m_external_addrswap[1],m_external_addrswap[0]) << 1;

	UINT16 retdata = 0;
	csflags = 0;
	int upper_addr_bits = (address & 0x7800) >> 11;

	if (upper_addr_bits == 0x8) // configuration registers are hardcoded to this area
	{
		int real_address = address & 0xf;
		logerror("read config regs? %04x %04x\n", real_address, mem_mask);
		return 0x0000;
	}

	// what gets priority?
	for (int i=0;i<6;i++)
	{
		int cs = region_selects[i];

		if (cs==upper_addr_bits)
		{
			int real_address = address & 0x7ff;
			csflags |= (1 << i);

			if (i==0) // the first cs is our internal protection area
			{
				//logerror("read matches cs table (protection) %01x %04x %04x\n", i, real_address, mem_mask);
				return read_protport( real_address, mem_mask);
			}
			else
			{
				logerror("read matches cs table (external connection) %01x %04x %04x\n", i, real_address, mem_mask);
			}
		}
	}

	if (csflags==0)
	{
		logerror("read not in cs table\n");
	}

	return retdata;
}


//const device_type DECO146BASE = &device_creator<deco_146_base_device>;



deco_146_base_device::deco_146_base_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source)
{
	m_port_a_r =  deco146_port_read_cb(FUNC(deco_146_base_device::port_a_default), this);
	m_port_b_r =  deco146_port_read_cb(FUNC(deco_146_base_device::port_b_default), this);
	m_port_c_r =  deco146_port_read_cb(FUNC(deco_146_base_device::port_c_default), this);
	m_soundlatch_w =  deco146_port_write_cb(FUNC(deco_146_base_device::soundlatch_default), this);

	m_external_addrswap[0] = 0;
	m_external_addrswap[1] = 1;
	m_external_addrswap[2] = 2;
	m_external_addrswap[3] = 3;
	m_external_addrswap[4] = 4;
	m_external_addrswap[5] = 5;
	m_external_addrswap[6] = 6;
	m_external_addrswap[7] = 7;
	m_external_addrswap[8] = 8;
	m_external_addrswap[9] = 9;
}

void deco_146_base_device::device_config_complete()
{
}



UINT16 deco_146_base_device::port_dummy_cb(int unused)
{
	return 0x00;
}

void deco_146_base_device::soundlatch_dummy(address_space &space, UINT16 data, UINT16 mem_mask)
{
}

UINT16 deco_146_base_device::port_a_default(int unused)
{
	return ioport(":INPUTS")->read();
}

UINT16 deco_146_base_device::port_b_default(int unused)
{
	return ioport(":SYSTEM")->read();
}

UINT16 deco_146_base_device::port_c_default(int unused)
{
	return ioport(":DSW")->read();
}

void deco_146_base_device::soundlatch_default(address_space &space, UINT16 data, UINT16 mem_mask)
{
	driver_device *drvstate = machine().driver_data<driver_device>();
	drvstate->soundlatch_byte_w(space, 0, data & 0xff);
	cpu_device* cpudev = (cpu_device*)machine().device(":audiocpu");
	if (cpudev) cpudev->set_input_line(0, HOLD_LINE);
}




void deco_146_base_device::set_port_a_cb(device_t &device,deco146_port_read_cb port_cb) { deco_146_base_device &dev = downcast<deco_146_base_device &>(device); dev.m_port_a_r = port_cb; }
void deco_146_base_device::set_port_b_cb(device_t &device,deco146_port_read_cb port_cb) { deco_146_base_device &dev = downcast<deco_146_base_device &>(device); dev.m_port_b_r = port_cb; }
void deco_146_base_device::set_port_c_cb(device_t &device,deco146_port_read_cb port_cb) { deco_146_base_device &dev = downcast<deco_146_base_device &>(device); dev.m_port_c_r = port_cb; }
void deco_146_base_device::set_soundlatch_cb(device_t &device,deco146_port_write_cb port_cb) { deco_146_base_device &dev = downcast<deco_146_base_device &>(device); dev.m_soundlatch_w = port_cb; }
void deco_146_base_device::set_interface_scramble(device_t &device,UINT8 a9, UINT8 a8, UINT8 a7, UINT8 a6, UINT8 a5, UINT8 a4, UINT8 a3,UINT8 a2,UINT8 a1,UINT8 a0)
{
	deco_146_base_device &dev = downcast<deco_146_base_device &>(device);
	dev.m_external_addrswap[9] = a9;
	dev.m_external_addrswap[8] = a8;
	dev.m_external_addrswap[7] = a7;
	dev.m_external_addrswap[6] = a6;
	dev.m_external_addrswap[5] = a5;
	dev.m_external_addrswap[4] = a4;
	dev.m_external_addrswap[3] = a3;
	dev.m_external_addrswap[2] = a2;
	dev.m_external_addrswap[1] = a1;
	dev.m_external_addrswap[0] = a0;
}
void deco_146_base_device::set_use_magic_read_address_xor(device_t &device, int use_xor)
{
	deco_146_base_device &dev = downcast<deco_146_base_device &>(device);
	dev.m_magic_read_address_xor_enabled = use_xor;
}

void deco_146_base_device::device_start()
{
	for (int i=0;i<0x80;i++)
	{
		// the mutant fighter old sim assumes 0x0000
		m_rambank0[i] = 0xffff;
		m_rambank1[i] = 0xffff;
	}


	// bind our handler
	m_port_a_r.bind_relative_to(*owner());
	m_port_b_r.bind_relative_to(*owner());
	m_port_c_r.bind_relative_to(*owner());
	m_soundlatch_w.bind_relative_to(*owner());


	save_item(NAME(m_xor));
	save_item(NAME(m_nand));
	save_item(NAME(m_soundlatch));

	save_item(NAME(m_rambank0));
	save_item(NAME(m_rambank1));
	save_item(NAME(m_current_rambank));

	save_item(NAME(region_selects));

	save_item(NAME(m_latchaddr));
	save_item(NAME(m_latchdata));
	save_item(NAME(m_latchflag));
}

void deco_146_base_device::device_reset()
{
	region_selects[0] = 0;
	region_selects[1] = 0;
	region_selects[2] = 0;
	region_selects[3] = 0;
	region_selects[4] = 0;
	region_selects[5] = 0;



	m_current_rambank = 0;

	m_soundlatch = 0x0000;

	m_latchaddr = 0xffff;
	m_latchdata = 0x0000;
	m_latchflag = 0;

	m_xor=0;
//  m_nand=0xffff;
	m_nand=0x0; // wizard fire doesn't initialize it, but accesses addresses rohga needs the mask applied on
}








const device_type DECO146PROT = &device_creator<deco146_device>;


UINT16 deco_146_base_device::read_data_getloc(UINT16 address, int& location)
{
	UINT16 retdata = 0;

	location = m_lookup_table[address>>1].write_offset;

	if (location==INPUT_PORT_A)
	{
		retdata = m_port_a_r(0);
	}
	else if (location==INPUT_PORT_B)
	{
		retdata = m_port_b_r(0);
	}
	else if (location==INPUT_PORT_C)
	{
		retdata = m_port_c_r(0);
	}
	else
	{
		if (m_current_rambank==0)
			retdata = m_rambank0[location>>1];
		else
			retdata = m_rambank1[location>>1];
	}

	UINT16 realret = reorder(retdata, &m_lookup_table[address>>1].mapping[0] );

	if (m_lookup_table[address>>1].use_xor) realret ^= m_xor;
	if (m_lookup_table[address>>1].use_nand) realret = (realret & ~m_nand);

	return realret;
}


deco146_device::deco146_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: deco_146_base_device(mconfig, DECO146PROT, "DECO 146 Protection", tag, owner, clock, "deco146", __FILE__)
{
	m_bankswitch_swap_read_address = 0x78;
	m_magic_read_address_xor = 0x44a;
	m_magic_read_address_xor_enabled = 0;
	m_xor_port = 0x2c;
	m_mask_port = 0x36;
	m_soundlatch_port = 0x64;
	m_lookup_table = port_table;
	m_configregion = 0x8;
}
