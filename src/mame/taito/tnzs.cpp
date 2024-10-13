// license:BSD-3-Clause
// copyright-holders:Luca Elia, Mirko Buffoni, Takahiro Nogi
/***************************************************************************

Notes:
- There are four versions of TNZS supported.
  1) "tnzs".   New hardware revision. 3 Z80 and no M-Chip (8742 MPU).
  2) "tnzsj".  New hardware revision. 3 Z80 and no M-Chip (8742 MPU).
  3) "tnzsjo". Standard hardware. 2 Z80 and the M-Chip.
  4) "tnzsop".  Standard hardware. Harder gameplay, old Taito logo. Maybe a prototype?

- TNZS hidden level select: keep service coin pressed (default:9) and reset the game.
  When SERVICE SWITCH ERROR appears, release service coin and press 1 player start
  three times, service coin again, and 1 player start. Use up/down to select the
  starting level and press 1 player start to start. You'll also get 255 lives.
  If you have enough patience and go up until the level reads "Q-1" (which corresponds
  to 1-1), AND the "Invulnerability" dip switch is On, you'll be invulnerable.
  Invulnerability isn't possible in 'tnzsop' (level select is stucked to level 6-1).


PCBs:
  The TNZS/Seta hardware has a variety of somewhat different pcbs, all of
  which have both Seta and Taito Part numbers.
  All pcbs have Z80B processors and one 6264 main RAM chip and an X1-001
  and X1-002 video chip and an X1-004 I/O? Chip, and four PALs

Seta#       Taito#s             CPUS    RxM2    ROM1    MCU?    Video RAM   PROMs   SETA X1 GFXROMs     QUADRATURE  ESD. PROT   Games                           Picture
P0-022-A    K1100245A J1100108A 2xZ80B  512/256 512/256 8042    4x6116      Yes, 2  03      23c1000     uPD4701AC   3x X2-003*4 arkanoid2                       http://www.classicarcaderesource.com/RevengeOfDoh3.jpg
P0-022-B    K1100234A J1100108A 2xZ80B  512/256 512/256 8042    4x6116      Yes, 2  03      27c512(A)   uPD4701AC   3x X2-003*4 plumppop                        https://archive.org/download/plump-pop-pcb/IMG_3544.jpg
P0-025-A    K1100241A J1100107A 2xZ80B  512/256 512/256 8042    4x6116      Yes, 2  03      23c1000     N/A         3x X2-003   drtoppel,extermatn,chukatai(B)  http://arcade.ym2149.com/pcb/taito/drtoppel_pcb_partside.jpg
P0-028-A    K1100416A J1100332A 2xZ80B  512/256 512/256 8042    4x6116      No      05,06   23c1000     N/A         3x X2-004   chukatai(B)                     http://i.ebayimg.com/images/g/AhoAAOSw-FZXj5A5/s-l1600.jpg
P0-038A     M6100309A           3xZ80B  512/256 512/256 NONE    1x6164      No      05,06   23c1000     N/A         3x X2-003   kageki                          http://i.ebayimg.com/images/a/(KGrHqJ,!lwE6C8-G97lBOjOu9mwVw~~/s-l1600.jpg
P0-041-1    CA403001A           2xZ80B  61256   27c1000 8742    1x6164      No      05,06   27c1000     N/A         5x X2-005   tnzsop(C)                       http://arcade.ym2149.com/pcb/taito/tnzs_pcb3_partside.jpg
P0-041-A    K1100356A J1100156A 2xZ80B  61256   27c1000 8042    1x6164      No      05,06   23c1000     N/A         5x X2-005   tnzs(j,u)o                      http://arcade.ym2149.com/pcb/taito/tnzs_pcb1_partside.jpg
P0-043A     M6100356A           3xZ80B* 61256   27512** NONE    1x6164      No      05,06   LH534000*   N/A         4x X2-004   tnzs(j,u), kabukiz              http://arcade.ym2149.com/pcb/taito/tnzs_pcb2_mainboard_partside.jpg
P0-056A     K1100476A J1100201A 3xZ80B  EMPTY*3 27c1000 NONE    1x6164      No      05,06   LH534000    N/A         5x X2-005   insectx(D)                      http://www.jammarcade.net/images/2014/04/InsectorX.jpg

(A) GFX ROM mapping is slightly different to P0-022-A pcb, possibly configured
    by a jumper.
(B) chukatai has one set which unlike its earlier sets uses the P0-025-A
    PCB, but with a daughterboard which converts four of the 23c1000 gfx ROM
    sockets into 8 27c1000 EPROM sockets, and DOES use color PROMs!
    The other pcb set uses P0-028-A pcb and 23c1000 mask ROMs and color RAM,
    but has lower ROM id numbers. The higher numbered set was likely created
    by Taito to 'use up' a stock of older P0-025-A pcbs.
(C) This is a development/prototype PCB, hence it has 32 pin sockets for the
    gfx ROMs as 27c1000 EPROMs, instead of 28 pin sockets for 23c1000 mask
    ROMs. It also uses an (unprotected?) 8742 MCU.
    Another curious thing is the Taito ID number may have accidentally been
    printed in backwards order, i.e should be C1100304A which fits the pattern
    of the other boards.
(D) InsectorX has a lot of rework on its PCB, two greenwires for each of the
    two LH534000 mask ROMs, and four wires connected to the X1-004 I/O chip
    pins 18, 19, 20, and 21, connecting it to the 4 input pins of a Toshiba
    TD62064AP Darlington driver @ U43.
*   tnzs(j,u) uses a sub board with a z80b and 23c1000 mask ROMs on it for gfx,
    plugged into the four LH534000 mask ROM sockets and the 2nd z80 socket.
    Like Kageki's P0-038A mainboard, this mainboard has a third z80 on it which
    acts in place of the 8x42 mcu used by the older tnzs sets.
**  This is a 28-pin 27512 in a 32-pin socket which alternately holds a 27c1000.
*3  This is unpopulated, but the pcb can accept a 61256 SRAM here.
*4  arkanoid2 and plumppop lack all but one or two buttons, so two of the three
    ESD protection modules are unpopulated.


Hardware details for the newer tnzs board (from pictures):

  Main board
  M6100409A N.ZEALAND STORY (written on label)
  M6100356A (written on pcb)
  (note: Taito logo is the old version)
  SETA X1-001A 8740KX
  SETA X1-002A 8712KX
  SETA X1-004  815100
  SETA X1-006  8136KX
  SETA X1-007  805100
  these custom ics are also used in many other games (see drivers/seta.c, drivers/taito_x.c)
  Main xtal 12.0000 MHZ
  2xZ80-B
  YM2203C
  sockets for U2 (Z80-B) and U35/U39/U43/U46 (LH534000) are empty and the sub board
  connects to them.

  SUB PCB
  K9100209A N.Z.LAND STORY (written on label)
  K9100209A (written on pcb)
  J9100159A (written on pcb)
  (note: Taito logo is the new version)
  Z80-B

****************************************************************************

Stephh's notes (based on the games Z80 code and some tests) :

1) 'plumppop'

  - Region stored at 0x7fff.b (CPU1) then 0xef49 (shared RAM)
  - Sets :
      * 'plumppop' : region = 0x00
  - Coinage relies on bit 0 of the region (code at 0x5141 in CPU1) :
      * .......0 : TAITO_COINAGE_JAPAN_OLD
      * .......1 : TAITO_COINAGE_WORLD
  - Notice screen relies on bit 1 of the region (code at 0x16e8) :
      * ......0. : Yes
      * ......1. : No
  - Copyright relies on bits 2 and 3 of the region (code at 0x276d) :
      * ....00.. : "TAITO CORPORATION" / "ALL RIGHTS RESERVED" (Japan)
      * ....01.. : "TAITO AMERICA CORP." / "ALL RIGHTS RESERVED" (US)
      * ....10.. : "TAITO CORP. JAPAN" / "ALL RIGHTS RESERVED" (World)
      * ....11.. : "TAITO CORP. JAPAN" / "LICENCED TO -------"


2) 'extrmatn' and clones

2a) 'extrmatn'

  - Region stored at 0x7fff.b (CPU1) then 0x16 is stored at 0xef1d (shared RAM) !
  - Sets :
      * 'extrmatn' : region = 0x16
  - New value is stored at 0xef1d (shared RAM) :
      * 'extrmatn' : 0x16
  - Coinage relies on bit 0 of the region (code at 0x4703 in CPU1) :
      * .......0 : TAITO_COINAGE_JAPAN_OLD
      * .......1 : TAITO_COINAGE_WORLD
  - Notice screen relies on bit 1 of the new value (code at 0x4285) :
      * ......0. : Yes
      * ......1. : No
  - Copyright relies on bits 2 and 3 of the new value (code at 0x0cfe) :
      * ....00.. : "TAITO CORPORATION" / "     FOR U.S.A.    " (Japan)
      * ....01.. : "WORLD GAMES     " / "     FOR U.S.A.    " (US)
      * ....10.. : "TAITO CORP. JAPAN" / "     FOR U.S.A.    " (World)
      * ....11.. : "TAITO CORP. JAPAN" / "LICENCED TO        "
  - 'ret' + modified byte to please checksum at 0x8df5 (bank = 0x07) : unknown effect

2b) 'extrmatj'

  - Region stored at 0x7fff.b (CPU1) then 0x00 is stored at 0xef1d (shared RAM) !
  - Sets :
      * 'extrmatj' : region = 0x00
  - Coinage relies on bit 0 of the region (code at 0x4703 in CPU1) :
      * .......0 : TAITO_COINAGE_JAPAN_OLD
      * .......1 : TAITO_COINAGE_WORLD
  - Notice screen relies on bit 1 of the new value (code at 0x4285) :
      * ......0. : Yes
      * ......1. : No
  - Copyright relies on bits 2 and 3 of the new value (code at 0x0cfe) :
      * ....00.. : "TAITO CORPORATION" / "ALL RIGHTS RESERVED" (Japan)
      * ....01.. : "TAITO AMERICA CORP." / "ALL RIGHTS RESERVED" (US)
      * ....10.. : "TAITO CORP. JAPAN" / "ALL RIGHTS RESERVED" (World)
      * ....11.. : "TAITO CORP. JAPAN" / "LICENCED TO           "
  - Code and data differences :
      * 0x8df5 (bank = 0x07) : normal routine but still unknown effect
      * 0x01e0 (CPU1) : 0x00 instead of 0x16 (value for region tests)


3) 'arknoid2' and clones

3a) 'arknoid2' and 'arknid2j'

  - Region stored at 0x9fde.b (CPU1 - bank = 0x00) then stored at 0xe7f0 (shared RAM)
  - Coin mode stored at 0x9fff.b (CPU1 - bank = 0x00) then stored at 0xe7f1 (shared RAM)
  - Sets :
      * 'arknoid2' : region = 0x02 - coin mode = 0x01
      * 'arknid2j' : region = 0x00 - coin mode = 0x00
  - Coinage relies on coin mode (code at 0x0b17 in CPU1) :
      * 0x00 : TAITO_COINAGE_JAPAN_OLD
      * else : TAITO_COINAGE_WORLD
  - Notice screen relies on region (code at 0x495b) :
      * 0x00 : Yes
      * else : No
  - Copyright relies on region (code at 0x4774) :
      * 0x00 : "TAITO CORPORATION" / "ALL RIGHTS RESERVED" (Japan)
      * 0x01 : "TAITO AMERICA CORP." / "ALL RIGHTS RESERVED" (US)
      * 0x02 : "TAITO CORP. JAPAN" / "ALL RIGHTS RESERVED" (World)
      * 0x03 : "TAITO AMERICA CORP." / "LICENCED TO ROMSTAR" / "FOR U.S.A."

3b) 'arknid2u'

  - Region stored at 0x9fde.b (CPU1 - bank = 0x00) then stored at 0xe7f0 (shared RAM)
  - Coin mode stored at 0x9fff.b (CPU1 - bank = 0x00) then stored at 0xe7f1 (shared RAM)
  - Sets :
      * 'arknid2u' : region = 0x03 - coin mode = 0x00
  - Coinage relies on coin mode (code at 0x0b17 in CPU1) :
      * 0x00 : TAITO_COINAGE_JAPAN_OLD
      * else : TAITO_COINAGE_WORLD
  - Notice screen relies on region (code at 0x2e47) :
      * 0x00 : Yes
      * else : No
  - Copyright relies on region (code at 0x475b) :
      * 0x00 : "TAITO CORPORATION" / "ALL RIGHTS RESERVED" (Japan)
      * 0x01 : "TAITO AMERICA CORP." / "ALL RIGHTS RESERVED" (US)
      * 0x02 : "TAITO CORP. JAPAN" / "ALL RIGHTS RESERVED" (World)
      * 0x03 : "TAITO AMERICA CORP." / "LICENCED TO ROMSTAR" / "FOR U.S.A."
  - Levels differences :
      * new level 1
      * levels 3 left and 17 left are swapped
      * levels 17 right and 20 right are swapped


4) 'drtoppel' and clones

  - Region stored at 0x7fff.b (CPU1) then 0xe000 (shared RAM)
  - Sets :
      * 'drtoppel' : region = 0x03
      * 'drtopplu' : region = 0x02
      * 'drtopplj' : region = 0x01
  - These 3 games are 100% the same, only region differs !
    However, dumps for 'drtoppel' and 'drtopplu' are unsure (wrong ROM numbers).
  - Coinage relies on region (code at 0x5141 in CPU1) :
      * 0x01 and 0x02 : TAITO_COINAGE_JAPAN_OLD
      * 0x03 and 0x04 : TAITO_COINAGE_WORLD
  - Notice screen relies on region (code at 0x9331 - bank = 0x03) :
      * 0x01 : Yes
      * else : No
  - Title relies on region (code at 0x90a4 - bank = 0x03) :
      * 0x01 : Japanese
      * else : English
  - Copyright relies on region (code at 0x9b62 - bank = 0x03) :
      * 0x01 : "TAITO CORPORATION" / "ALL RIGHTS RESERVED" (Japan)
      * 0x02 : "TAITO AMERICA CORP." / "ALL RIGHTS RESERVED" (US)
      * else : "TAITO CORP. JAPAN" / "ALL RIGHTS RESERVED" (World / ???)


5) 'kageki' and clones

5a) 'kagekiu'

  - Region stored at 0x9fff.b (CPU1 - bank = 0x03) then 0xe000 (shared RAM)
  - Sets :
      * 'kagekiu'   : region = 0x02
  - Coinage relies on region (code at 0x0099 in CPU1) :
      * 0x01 and 0x02 : TAITO_COINAGE_JAPAN_OLD
      * 0x03 and 0x04 : TAITO_COINAGE_WORLD
  - No notice screen
  - Copyright relies on region (code at 0x1eeb) :
      * 0x01 : "TAITO CORPORATION" / "ALL RIGHTS RESERVED" (Japan)
      * 0x02 : "TAITO AMERICA CORP." / "LICENCED TO ROMSTAR" / "FOR U.S.A." (US)
      * else : "TAITO CORP. JAPAN" / "ALL RIGHTS RESERVED" (World / ???)
  - English text on bad guys screens when game starts + English samples
  - 2 players gameplay : players fight against each other in a best of 3 VS match,
    then the winner is allowed to fight against the CPU enemies.
    This is the reason why I've named DSWA bit 0 as "Unused".
    Note that if there is a draw (same energy for both players), player 1 wins.

5b) 'kagekij'

  - Region stored at 0x9fff.b (CPU1 - bank = 0x03) then 0xe000 (shared RAM)
  - Sets :
      * 'kagekij'  : region = 0x01
  - Coinage relies on region (code at 0x0099 in CPU1) :
      * 0x01 and 0x02 : TAITO_COINAGE_JAPAN_OLD
      * 0x03 and 0x04 : TAITO_COINAGE_WORLD
  - No notice screen
  - Copyright relies on region (code at 0x1eb3) :
      * 0x01 : "TAITO CORPORATION" / "ALL RIGHTS RESERVED" (Japan)
      * 0x02 : "TAITO AMERICA CORP." / "ALL RIGHTS RESERVED" (US)
      * else : "TAITO CORP. JAPAN" / "ALL RIGHTS RESERVED" (World / ???)
  - Japanese text on bad guys screens when game starts + Japanese samples
  - 2 players gameplay : players fight one after the other against CPU enemies.
    Once an enemy is defeated or a player is dead, it's the other player turn.
  - Code and data differences :
      * CPU0 to be looked at carefully
      * 0x9fff (CPU1 - bank = 0x03) : 0x01 instead of 0x02 (value for region tests)

5c) 'kagekih'

  - Region stored at 0x9fff.b (CPU1 - bank = 0x03) then 0xe000 (shared RAM)
  - Sets :
      * 'kagekij'  : region = 0x01
  - This set really looks like a hack :
      * year has been changed from 1988 to 1992
      * the game uses Japanese ROMS, but CPU0 ROM displays English text
        on bad guys screens when game starts
  - Coinage relies on region (code at 0x0099 in CPU1) :
      * 0x01 and 0x02 : TAITO_COINAGE_JAPAN_OLD
      * 0x03 and 0x04 : TAITO_COINAGE_WORLD
  - No notice screen
  - Copyright relies on region (code at 0x1ee0) :
      * 0x01 : "TAITO CORPORATION" / "ALL RIGHTS RESERVED" (Japan)
      * 0x02 : "TAITO AMERICA CORP." / "ALL RIGHTS RESERVED" (US)
      * else : "TAITO CORP. JAPAN" / "ALL RIGHTS RESERVED" (World / ???)
  - English text on bad guys screens when game starts but Japanese samples !
  - 2 players gameplay : players fight against each other in a best of 3 VS match,
    then the winner is allowed to fight against the CPU enemies.
    This is the reason why I've named DSWA bit 0 as "Unused".
    Note that if there is a draw (same energy for both players), player 1 wins.
  - Code and data differences :
      * CPU0 to be looked at carefully
      * 0x9fff (CPU1 - bank = 0x03) : 0x01 instead of 0x02 (value for region tests)


6) 'chukatai' and clones

  - Region stored at 0x9fff.b (CPU1 - bank = 0x03) then 0xe01a (shared RAM)
  - Sets :
      * 'chukatai' : region = 0x03
      * 'chukatau' : region = 0x02
      * 'chukataj' : region = 0x01
  - These 3 games are 100% the same, only region differs !
    However, dumps for 'chukatai' and 'chukatau' are unsure (wrong ROM numbers).
  - Coinage relies on region (code at 0x0114 in CPU1) :
      * 0x01 and 0x02 : TAITO_COINAGE_JAPAN_OLD
      * 0x03 and 0x04 : TAITO_COINAGE_WORLD
  - Notice screen relies on region (code at 0x0253) :
      * 0x01 : Yes
      * else : No
  - Copyright relies on region (code at 0x0b38) :
      * 0x01 : "TAITO CORPORATION" / "ALL RIGHTS RESERVED" (Japan)
      * 0x02 : "TAITO AMERICA CORPORATION" / "ALL RIGHTS RESERVED" (US)
      * else : "TAITO CORPORATION JAPAN" / "ALL RIGHTS RESERVED" (World / ???)
    I still need to understand how the strings are displayed though.
  - Power-ups display is affected by the region


7) 'tnzs' and clones

7a) 'tnzs' and 'tnzsj'

  - Region stored at 0x7fff.b (CPU1)
  - Sets :
      * 'tnzs'     : region = 0x03
      * 'tnzsj'    : region = 0x01
  - These 2 games are 100% the same, only region differs !
  - Code at 0x00fb (CPU1) changes region to another value :
      * 0x01 -> 0x00
      * 0x02 -> 0x06
      * 0x03 -> 0x0b
      * 0x04 -> 0x0e
    This value is stored at 0xd035 (CPU1) then 0xef1d (shared RAM).
  - Coinage relies on bit 0 of the new value (code at 0x0aa7 in CPU1) :
      * .......0 : TAITO_COINAGE_JAPAN_OLD
      * .......1 : TAITO_COINAGE_WORLD
  - Notice screen relies on bit 1 of the new value (code at 0xbed0 - bank = 0x05) :
      * ......0. : Yes
      * ......1. : No
  - Copyright relies on bits 2 and 3 of the new value (code at 0x12eb) :
      * ....00.. : "TAITO CORPORATION" / "ALL RIGHTS RESERVED" (Japan)
      * ....01.. : "TAITO AMERICA CORP." / "ALL RIGHTS RESERVED" (US)
      * ....10.. : "TAITO CORP. JAPAN" / "ALL RIGHTS RESERVED" (World)
      * ....11.. : "TAITO CORP. JAPAN" / "LICENCED TO           "
  - New Taito logo

7b) 'tnzsjo'

  - Region stored at 0x7fff.b (CPU1)
  - Sets :
      * 'tnzsjo'   : region = 0x01
  - Code at 0x00f7 (CPU1) changes region to another value :
      * 0x01 -> 0x00
      * 0x02 -> 0x06
      * 0x03 -> 0x0b
      * 0x04 -> 0x0e
    This value is stored at 0xd035 (CPU1) then 0xef1d (shared RAM).
  - Coinage relies on bit 0 of the new value (code at 0x5f68 in CPU1) :
      * .......0 : TAITO_COINAGE_JAPAN_OLD
      * .......1 : TAITO_COINAGE_WORLD
  - Notice screen relies on bit 1 of the new value (code at 0xbbb5 - bank = 0x05) :
      * ......0. : Yes
      * ......1. : No
  - Copyright relies on bits 2 and 3 of the new value (code at 0x12ed) :
      * ....00.. : "TAITO CORPORATION" / "ALL RIGHTS RESERVED" (Japan)
      * ....01.. : "TAITO AMERICA CORP." / "ALL RIGHTS RESERVED" (US)
      * ....10.. : "TAITO CORP. JAPAN" / "ALL RIGHTS RESERVED" (World)
      * ....11.. : "TAITO CORP. JAPAN" / "LICENCED TO           "
  - New Taito logo
  - levels are different from 'tnzs'

7c) 'tnzsop'

  - Region stored at 0x7fff.b (CPU1)
  - Sets :
      * 'tnzso'    : region = 0x03
  - Code at 0x021e (CPU1) changes region to another value :
      * 0x01 -> 0x00
      * 0x02 -> 0x06
      * 0x03 -> 0x0b
      * 0x04 -> 0x0e
    This value is stored at 0xd46b (CPU1) then 0xef1d (shared RAM).
  - Coinage relies on bit 0 of the new value (code at 0x5d91 in CPU1) :
      * .......0 : TAITO_COINAGE_JAPAN_OLD
      * .......1 : TAITO_COINAGE_WORLD
  - Notice screen relies on bit 1 of the new value (code at 0x0cdd) :
      * ......0. : Yes
      * ......1. : No
  - Copyright relies on bits 2 and 3 of the new value (code at 0x1062) :
      * ....00.. : "TAITO CORPORATION" / "ALL RIGHTS RESERVED" (Japan)
      * ....01.. : "TAITO AMERICA CORP." / "ALL RIGHTS RESERVED" (US)
      * ....10.. : "TAITO CORP. JAPAN" / "ALL RIGHTS RESERVED" (World)
      * ....11.. : "TAITO CORP. JAPAN" / "LICENCED TO           "
  - Old Taito logo
  - levels are different from 'tnzs' and 'tnzsjo'


8) 'kabukiz' and clones

  - No region byte : a value is loaded in register A each time it is needed !
  - Sets :
      * 'kabukiz'  : value = 0x02
      * 'kabukizj' : value = 0x00
  - Coinage relies on register A (code at 0x016c in CPU1)
      * 0x02 : TAITO_COINAGE_WORLD
      * else : TAITO_COINAGE_JAPAN_OLD
  - Notice screen relies on register A (code at 0x0b30 in CPU1) :
      * 0x00 : Yes
      * else : No
  - Copyright relies on register A (code at 0x0d4e in CPU1)
      * 0x00 : "TAITO CORPORATION" / "ALL RIGHTS RESERVED" (Japan)
      * 0x01 : "TAITO AMERICA CORP." / "ALL RIGHTS RESERVED" (US)
      * else : "TAITO CORP. JAPAN" / "ALL RIGHTS RESERVED" (World)
  - 'babukizj' has extra code in CPU1 to be looked at carefully.


9) 'insectx'

  - Region stored at 0x9fff.b (CPU1 - bank = 0x03) then 0xec21 (shared RAM)
  - Sets :
      * 'insectx'  : region = 0x03
  - Coinage relies on region (code at 0x025b in CPU1) :
      * 0x01 : TAITO_COINAGE_JAPAN_OLD
      * 0x02 : TAITO_COINAGE_US
      * else : TAITO_COINAGE_WORLD
  - Notice screen relies on region (code at 0x0a6c) :
      * 0x01 : Yes
      * else : No
  - Copyright relies on region (code at 0xac16 - bank = 0x02) :
      * 0x01 : "TAITO CORPORATION" / "ALL RIGHTS RESERVED" (Japan)
      * 0x02 : "TAITO AMERICA CORPORATION" / "ALL RIGHTS RESERVED" (US)
      * else : "TAITO CORPORATION JAPAN" / "ALL RIGHTS RESERVED" (World)
  - This game doesn't use standard settings for "Difficulty" Dip Switch :
    look at table at 0x1b86 (4 * 2 bytes) and especially table at 0x4731
    (4 bytes) which determines end of level boss energy via code at 0x46e2,
    and you'll notice that "Easy" and "Hard" settings are swapped.
    End of level boss energy is stored at 0xe484 and 0xe485 (LSB first).


10) 'jpopnics'

  - Region stored at 0x7fff.b (CPU1) then 0xef49 (shared RAM)
  - Sets :
      * 'jpopnics' : region = 0x00
  - Coinage relies on bit 0 of the region (code at 0x5141 in CPU1) :
      * .......0 : TAITO_COINAGE_JAPAN_OLD
      * .......1 : TAITO_COINAGE_WORLD
    However, coinage is in fact always 1C_1C for both coin slots due to
    'ret' instruction at 0x5151 and way coins are handled at 0x4f60.
  - Notice screen relies on bit 1 of the region (code at 0x16e8) :
      * ......0. : Yes
      * ......1. : No
    Notice screen displays "Korea" instead of "Japan".
  - Copyright relies on bits 2 and 3 of the region (code at 0x276d),
    but table at 0x2781 is the same for any combination :
      * ....??.. : "NICS CO. LTD. 1992" / "ALL RIGHTS RESERVED"


TODO:
-----
- Find out how the hardware credit-counter works (MPU)
- Fix MCU simulation errors :
    * avoid credits to be increased when in "test mode" to avoid coin lockout
      (needed to complete special test mode in 'extrmatn' and 'arknoid2')
    * why are credits limited to 9 in some games ?
      pressing SERVICE1 allows to go up to 100 and cause this :
        . 'plumppop' : freeze
        . 'extrmatn' : reset
        . 'arknoid2' : coin overflow
- Fix video offsets (See Dr Toppel in Flip-Screen - also affects Chuka Taisen)
- Video scroll side flicker in Chuka Taisen, Insector X, Dr Toppel, Kabuki Z
- Sprite/background sync during scrolling, e.g. insectx, kabukiz.
- Merge video driver with seta.c (it's the same thing but seta.c assumes a 16-bit CPU)
- Figure out remaining unknown Dip Switches in 'plumppop' and 'jpopnics'

Arkanoid 2:
  - What do writes at $f400 do ?
  - Why does the game zero the $fd00 area ?
Extrmatn:
  - What do reads from $f600 do ? (discarded)
Chuka Taisen:
  - What do writes at  $f400 do ? (value 40h)
  - What do reads from $f600 do in service mode ?
Dr Toppel:
  - What do writes at  $f400 do ? (value 40h)
  - What do reads from $f600 do in service mode ?

****************************************************************************

extrmatn and arknoid2 have a special test mode. The correct procedure to make
it succeed is as follows:
- enter service mode
- on the color test screen, press 2 (player 2 start)
- set dip switch 1 and dip switch 2 so that they read 00000001
- reset the emulation, and skip the previous step.
- press 5 (coin 1). Text at the bottom will change to "CHECKING NOW".
- use all the inputs, including tilt, until all inputs are OK
- press 5 (coin 1) - to confirm that coin lockout 1 works
- press 5 (coin 1) - to confirm that coin lockout 2 works
- set dip switch 1 to 00000000
- set dip switch 1 to 10101010
- set dip switch 1 to 11111111
- set dip switch 2 to 00000000
- set dip switch 2 to 10101010
- set dip switch 2 to 11111111
- speaker should now output a tone
- press 5 (coin 1), to confirm that OPN works
- press 5 (coin 1), to confirm that SSGCH1 works
- press 5 (coin 1), to confirm that SSGCH2 works
- press 5 (coin 1), to confirm that SSGCH3 works
- finished ("CHECK ALL OK!")

****************************************************************************

The New Zealand Story memory map (preliminary)

CPU #1
0000-7fff ROM
8000-bfff banked - banks 0-1 RAM; banks 2-7 ROM
c000-dfff object RAM, including:
  c000-c1ff sprites (code, low byte)
  c200-c3ff sprites (x-coord, low byte)
  c400-c5ff tiles (code, low byte)

  d000-d1ff sprites (code, high byte)
  d200-d3ff sprites (x-coord and colour, high byte)
  d400-d5ff tiles (code, high byte)
  d600-d7ff tiles (colour)
e000-efff RAM shared with CPU #2
f000-ffff VDC RAM, including:
  f000-f1ff sprites (y-coord)
  f200-f2ff scrolling info
  f300-f301 vdc controller
  f302-f303 scroll x-coords (high bits)
  f600      bankswitch
  f800-fbff palette

CPU #2
0000-7fff ROM
8000-9fff banked ROM
a000      bankswitch
b000-b001 YM2203 interface (with DIPs on YM2203 ports)
c000-c001 I8742 MCU
e000-efff RAM shared with CPU #1
f000-f003 inputs (used only by Arkanoid 2)

****************************************************************************/

/***************************************************************************

                Arkanoid 2 - Revenge of Doh!
                    (C) 1987 Taito

                        driver by

                Luca Elia (l.elia@tin.it)
                Mirko Buffoni

- The game doesn't write to f800-fbff (static palette)


            Interesting routines (main cpu)
            -------------------------------

1ed prints the test screen (first string at 206)

47a prints dipsw1&2 e 1p&2p paddleL values:
    e821        IN DIPSW1       e823-4  1P PaddleL (lo-hi)
    e822        IN DIPSW2       e825-6  2P PaddleL (lo-hi)

584 prints OK or NG on each entry:
    if (*addr)!=0 { if (*addr)!=2 OK else NG }
    e880    1P PADDLEL      e88a    IN SERVICE
    e881    1P PADDLER      e88b    IN TILT
    e882    1P BUTTON       e88c    OUT LOCKOUT1
    e883    1P START        e88d    OUT LOCKOUT2
    e884    2P PADDLEL      e88e    IN DIP-SW1
    e885    2P PADDLER      e88f    IN DIP-SW2
    e886    2P BUTTON       e890    SND OPN
    e887    2P START        e891    SND SSGCH1
    e888    IN COIN1        e892    SND SSGCH2
    e889    IN COIN2        e893    SND SSGCH3

672 prints a char
715 prints a string (0 terminated)

        Shared Memory (values written mainly by the sound cpu)
        ------------------------------------------------------

e001=dip-sw A   e399=coin counter value     e72c-d=1P paddle (lo-hi)
e002=dip-sw B   e3a0-2=1P score/10 (BCD)    e72e-f=2P paddle (lo-hi)
e008=level=2*(shown_level-1)+x <- remember it's a binary tree (42 last)
e7f0=country code(from 9fde in sound ROM)
e807=counter, reset by sound cpu, increased by main cpu each vblank
e80b=test progress=0(start) 1(first 8) 2(all ok) 3(error)
ec09-a~=ed05-6=xy pos of cursor in hi-scores
ec81-eca8=hi-scores(8bytes*5entries)

addr    bit name        active  addr    bit name        active
e72d    6   coin[1]     low     e729    1   2p select   low
        5   service     high            0   1p select   low
        4   coin[2]     low

addr    bit name        active  addr    bit name        active
e730    7   tilt        low     e7e7    4   1p fire     low
                                        0   2p fire     low

            Interesting routines (sound cpu)
            --------------------------------

4ae check starts    B73,B7a,B81,B99 coin related
8c1 check coins     62e lockout check       664 dsw check

            Interesting locations (sound cpu)
            ---------------------------------

d006=each bit is on if a corresponding location (e880-e887) has changed
d00b=(c001)>>4=tilt if 0E (security sequence must be reset?)
addr    bit name        active
d00c    7   tilt
        6   ?service?
        5   coin2       low
        4   coin1       low

d00d=each bit is on if the corresponding location (e880-e887) is 1 (OK)
d00e=each of the 4 MSBs is on if ..
d00f=FF if tilt, 00 otherwise
d011=if 00 checks counter, if FF doesn't
d23f=input port 1 value

***************************************************************************/

/***************************************************************************

  Kageki
  (c) 1988 Taito Corporation

  Driver by Takahiro Nogi 1999/11/06

***************************************************************************/

/***************************************************************************

  Jumping Pop
   - added by David Haywood, thanks to Robin Cooper

  This is not a Taito board, it's a copy, it contains no original Taito or
   Seta components.

  The game is a bootleg of Plump Pop produced by NICs of Korea, it has new
   graphics, sounds etc.

  Uses a YM2151 instead of the YM2203, has no MCU

  Uses Palette RAM instead of PROMs.

  ToDo:

  Palette format (or gfx decode?) appears to be incorrect, some colours
   clearly don't match the screenshot.

  Frequencies have not been measured

  The bar behind the players flickers first boot / first attract levels, bug?

  Inputs might be wrong, should be joystick not spinner? (can't select character)

***************************************************************************/

#include "emu.h"

#include "taitoipt.h"
#include "tnzs_video.h"

#include "cpu/mcs48/mcs48.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/upd4701.h"
#include "sound/dac.h"
#include "sound/samples.h"
#include "sound/ymopm.h"
#include "sound/ymopn.h"

#include "speaker.h"

//#define VERBOSE 1
#include "logmacro.h"


namespace {

class insectx_state : public tnzs_video_state_base
{
public:
	insectx_state(const machine_config &mconfig, device_type type, const char *tag)
		: tnzs_video_state_base(mconfig, type, tag)
		, m_subcpu(*this, "sub")
		, m_subbank(*this, "subbank")
		, m_ramromview(*this, "ramrom")
	{ }

	void insectx(machine_config &config) ATTR_COLD;

protected:
	void tnzs_base(machine_config &config) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;

	void ramrom_bankswitch_w(uint8_t data);

	void prompal_main_map(address_map &map) ATTR_COLD;
	void rampal_main_map(address_map &map) ATTR_COLD;

	void base_sub_map(address_map &map) ATTR_COLD;

	// devices
	required_device<cpu_device> m_subcpu;
	required_memory_bank m_subbank;

private:
	void insectx_bankswitch1_w(uint8_t data);

	void insectx_sub_map(address_map &map) ATTR_COLD;

	memory_view m_ramromview;
};


class tnzs_mcu_state : public insectx_state
{
public:
	tnzs_mcu_state(const machine_config &mconfig, device_type type, const char *tag)
		: insectx_state(mconfig, type, tag)
		, m_mcu(*this, "mcu")
		, m_in0(*this, "IN0")
		, m_in1(*this, "IN1")
		, m_in2(*this, "IN2")
		, m_input_select(0)
	{ }

	void tnzs(machine_config &config) ATTR_COLD;
	void extrmatn(machine_config &config) ATTR_COLD;

protected:
	void tnzs_bankswitch1_w(uint8_t data);
	uint8_t mcu_port1_r();
	template <bool LockoutLevel> void mcu_port2_w(uint8_t data);
	uint8_t mcu_r(offs_t offset);
	void mcu_w(offs_t offset, uint8_t data);

	template <bool LockoutLevel> void tnzs_mcu(machine_config &config) ATTR_COLD;

	void tnzs_sub_map(address_map &map) ATTR_COLD;

	required_device<upi41_cpu_device> m_mcu;

	required_ioport m_in0;
	required_ioport m_in1;
	required_ioport m_in2;

	uint8_t  m_input_select;
};


class plumppop_state : public tnzs_mcu_state
{
public:
	plumppop_state(const machine_config &mconfig, device_type type, const char *tag)
		: tnzs_mcu_state(mconfig, type, tag)
		, m_upd4701(*this, "upd4701")
	{
	}

	void plumppop(machine_config &config) ATTR_COLD;

protected:
	void plumppop_bankswitch1_w(uint8_t data);

	void plumppop_sub_map(address_map &map) ATTR_COLD;

	required_device<upd4701_device> m_upd4701;
};


class arknoid2_state : public plumppop_state
{
public:
	arknoid2_state(const machine_config &mconfig, device_type type, const char *tag)
		: plumppop_state(mconfig, type, tag)
		, m_coin1(*this, "COIN1")
		, m_coin2(*this, "COIN2")
	{ }

	void arknoid2(machine_config &config) ATTR_COLD;

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void arknoid2_bankswitch1_w(uint8_t data);

	uint8_t mcu_r(offs_t offset);
	void mcu_w(offs_t offset, uint8_t data);
	INTERRUPT_GEN_MEMBER(mcu_interrupt);

	void arknoid2_sub_map(address_map &map) ATTR_COLD;

	void mcu_reset();
	void mcu_handle_coins(int coin);

	required_ioport m_coin1;
	required_ioport m_coin2;

	int      m_mcu_initializing = 0;
	int      m_mcu_coinage_init = 0;
	int      m_mcu_command = 0;
	int      m_mcu_readcredits = 0;
	int      m_mcu_reportcoin = 0;
	int      m_insertcoin = 0;
	uint8_t  m_mcu_coinage[4]{};
	uint8_t  m_mcu_coins_a = 0;
	uint8_t  m_mcu_coins_b = 0;
	uint8_t  m_mcu_credits = 0;
};


class kageki_state : public insectx_state
{
public:
	kageki_state(const machine_config &mconfig, device_type type, const char *tag)
		: insectx_state(mconfig, type, tag)
		, m_samples(*this, "samples")
		, m_dswa(*this, "DSWA")
		, m_dswb(*this, "DSWB")
		, m_csport_sel(0)
	{ }

	void kageki(machine_config &config) ATTR_COLD;

	void init_kageki() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	static constexpr unsigned MAX_SAMPLES = 0x2f;

	void kageki_bankswitch1_w(uint8_t data);

	uint8_t csport_r();
	void csport_w(uint8_t data);

	SAMPLES_START_CB_MEMBER(init_samples);

	void kageki_sub_map(address_map &map) ATTR_COLD;

	required_device<samples_device> m_samples;

	required_ioport m_dswa;
	required_ioport m_dswb;

	// sound-related
	std::unique_ptr<int16_t[]>    m_sampledata[MAX_SAMPLES];
	int      m_samplesize[MAX_SAMPLES]{};

	int      m_csport_sel;
};


class jpopnics_state : public insectx_state
{
public:
	jpopnics_state(const machine_config &mconfig, device_type type, const char *tag)
		: insectx_state(mconfig, type, tag)
		, m_upd4701(*this, "upd4701")
	{ }

	void jpopnics(machine_config &config) ATTR_COLD;

private:
	void subbankswitch_w(uint8_t data);

	void jpopnics_main_map(address_map &map) ATTR_COLD;
	void jpopnics_sub_map(address_map &map) ATTR_COLD;

	required_device<upd4701_device> m_upd4701;
};


class tnzsb_state : public insectx_state
{
public:
	tnzsb_state(const machine_config &mconfig, device_type type, const char *tag)
		: insectx_state(mconfig, type, tag)
		, m_audiocpu(*this, "audiocpu")
		, m_soundlatch(*this, "soundlatch")
	{ }

	void tnzsb(machine_config &config) ATTR_COLD;

protected:
	void ym2203_irqhandler(int state);

	void sound_command_w(uint8_t data);

	void tnzsb_bankswitch1_w(uint8_t data);

	void tnzsb_base_sub_map(address_map &map) ATTR_COLD;
	void tnzsb_sub_map(address_map &map) ATTR_COLD;
	void tnzsb_cpu2_map(address_map &map) ATTR_COLD;
	void tnzsb_io_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_audiocpu;
	required_device<generic_latch_8_device> m_soundlatch;
};


class kabukiz_state : public tnzsb_state
{
public:
	kabukiz_state(const machine_config &mconfig, device_type type, const char *tag)
		: tnzsb_state(mconfig, type, tag)
		, m_audiobank(*this, "audiobank")
	{ }

	void kabukiz(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void sound_bank_w(uint8_t data);

	void kabukiz_cpu2_map(address_map &map) ATTR_COLD;
	void kabukiz_sub_map(address_map &map) ATTR_COLD;

	required_memory_bank m_audiobank;
};


/***************************************************************************

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

  The I8742 MCU takes care of handling the coin inputs and the tilt switch.
  To simulate this, we read the status in the interrupt handler for the main
  CPU and update the counters appropriately. We also must take care of
  handling the coin/credit settings ourselves.

***************************************************************************/

uint8_t tnzs_mcu_state::mcu_r(offs_t offset)
{
	uint8_t data = m_mcu->upi41_master_r(offset & 1);
	m_subcpu->yield();

//  logerror("%s: read %02x from mcu $c00%01x\n", m_maincpu->pcbase(), data, offset);

	return data;
}

void tnzs_mcu_state::mcu_w(offs_t offset, uint8_t data)
{
//  logerror("%s: write %02x to mcu $c00%01x\n", m_maincpu->pcbase(), data, offset);

	m_mcu->upi41_master_w(offset & 1, data);
}

uint8_t tnzs_mcu_state::mcu_port1_r()
{
	int data = 0;

	switch (m_input_select)
	{
		case 0x0a:  data = m_in2->read(); break;
		case 0x0c:  data = m_in0->read(); break;
		case 0x0d:  data = m_in1->read(); break;
		default:    data = 0xff; break;
	}

//  logerror("%s:  Read %02x from port 1\n", m_maincpu->pcbase(), data);

	return data;
}

template <bool LockoutLevel>
void tnzs_mcu_state::mcu_port2_w(uint8_t data)
{
	machine().bookkeeping().coin_lockout_w(0, (data & 0x40) ? LockoutLevel : !LockoutLevel);
	machine().bookkeeping().coin_lockout_w(1, (data & 0x80) ? LockoutLevel : !LockoutLevel);
	machine().bookkeeping().coin_counter_w(0, (~data & 0x10));
	machine().bookkeeping().coin_counter_w(1, (~data & 0x20));

	m_input_select = data & 0xf;
}

template void tnzs_mcu_state::mcu_port2_w<false>(uint8_t);
template void tnzs_mcu_state::mcu_port2_w<true>(uint8_t);

void arknoid2_state::mcu_reset()
{
	m_mcu_initializing = 3;
	m_mcu_coinage_init = 0;
	m_mcu_coinage[0] = 1;
	m_mcu_coinage[1] = 1;
	m_mcu_coinage[2] = 1;
	m_mcu_coinage[3] = 1;
	m_mcu_coins_a = 0;
	m_mcu_coins_b = 0;
	m_mcu_credits = 0;
	m_mcu_reportcoin = 0;
	m_mcu_command = 0;
}

void arknoid2_state::mcu_handle_coins(int coin)
{
	/* The coin inputs and coin counters are managed by the i8742 mcu. */
	/* Here we simulate it. */
	/* Credits are limited to 9, so more coins should be rejected */
	/* Coin/Play settings must also be taken into consideration */

	if (coin & 0x08)    /* tilt */
		m_mcu_reportcoin = coin;
	else if (coin && coin != m_insertcoin)
	{
		if (coin & 0x01)    /* coin A */
		{
//          logerror("Coin dropped into slot A\n");
			machine().bookkeeping().coin_counter_w(0,1); machine().bookkeeping().coin_counter_w(0,0); /* Count slot A */
			m_mcu_coins_a++;
			if (m_mcu_coins_a >= m_mcu_coinage[0])
			{
				m_mcu_coins_a -= m_mcu_coinage[0];
				m_mcu_credits += m_mcu_coinage[1];
				if (m_mcu_credits >= 9)
				{
					m_mcu_credits = 9;
					machine().bookkeeping().coin_lockout_global_w(1); /* Lock all coin slots */
				}
				else
				{
					machine().bookkeeping().coin_lockout_global_w(0); /* Unlock all coin slots */
				}
			}
		}

		if (coin & 0x02)    /* coin B */
		{
//          logerror("Coin dropped into slot B\n");
			machine().bookkeeping().coin_counter_w(1,1); machine().bookkeeping().coin_counter_w(1,0); /* Count slot B */
			m_mcu_coins_b++;
			if (m_mcu_coins_b >= m_mcu_coinage[2])
			{
				m_mcu_coins_b -= m_mcu_coinage[2];
				m_mcu_credits += m_mcu_coinage[3];
				if (m_mcu_credits >= 9)
				{
					m_mcu_credits = 9;
					machine().bookkeeping().coin_lockout_global_w(1); /* Lock all coin slots */
				}
				else
				{
					machine().bookkeeping().coin_lockout_global_w(0); /* Unlock all coin slots */
				}
			}
		}

		if (coin & 0x04)    /* service */
		{
//          logerror("Coin dropped into service slot C\n");
			m_mcu_credits++;
		}

		m_mcu_reportcoin = coin;
	}
	else
	{
		if (m_mcu_credits < 9)
			machine().bookkeeping().coin_lockout_global_w(0); /* Unlock all coin slots */

		m_mcu_reportcoin = 0;
	}
	m_insertcoin = coin;
}

/*********************************

TNZS sync bug kludge

In all TNZS versions there is code like this:

0C5E: ld   ($EF10),a
0C61: ld   a,($EF10)
0C64: inc  a
0C65: ret  nz
0C66: jr   $0C61

which is sometimes executed by the main cpu when it writes to shared RAM a
command for the second CPU. The intended purpose of the code is to wait an
acknowledge from the sub CPU: the sub CPU writes FF to the same location
after reading the command.

However the above code is wrong. The "ret nz" instruction means that the
loop will be exited only when the contents of $EF10 are *NOT* $FF!!
On the real board, this casues little harm: the main CPU will just write
the command, read it back and, since it's not $FF, return immediately. There
is a chance that the command might go lost, but this will cause no major
harm, the worse that can happen is that the background tune will not change.

In MAME, however, since CPU interleaving is not perfect, it can happen that
the main CPU ends its timeslice after writing to EF10 but before reading it
back. In the meantime, the sub CPU will run, read the command and write FF
there - therefore causing the main CPU to enter an endless loop.

Unlike the usual sync problems in MAME, which can be fixed by increasing the
interleave factor, in this case increasing it will actually INCREASE the
chance of entering the endless loop - because it will increase the chances of
the main CPU ending its timeslice at the wrong moment.

So what we do here is catch writes by the main CPU to the RAM location, and
process them using a timer, in order to
a) force a resync of the two CPUs
b) make sure the main CPU will be the first one to run after the location is
   changed

Since the answer from the sub CPU is ignored, we don't even need to boost
interleave.

*********************************/

/*
TIMER_CALLBACK_MEMBER(insectx_state::kludge_callback)
{
    tnzs_sharedram[0x0f10] = param;
}

void insectx_state::tnzs_sync_kludge_w(uint8_t data)
{
    machine().scheduler().synchronize(timer_expired_delegate(FUNC(insectx_state::kludge_callback),this), data);
}
*/

uint8_t arknoid2_state::mcu_r(offs_t offset)
{
	static const char mcu_startup[] = "\x55\xaa\x5a";

	//logerror("%s: read mcu %04x\n", m_maincpu->pc(), 0xc000 + offset);

	if (offset == 0)
	{
		/* if the mcu has just been reset, return startup code */
		if (m_mcu_initializing)
		{
			m_mcu_initializing--;
			return mcu_startup[2 - m_mcu_initializing];
		}

		switch (m_mcu_command)
		{
			case 0x41:
				return m_mcu_credits;

			case 0xc1:
				/* Read the credit counter or the inputs */
				if (m_mcu_readcredits == 0)
				{
					m_mcu_readcredits = 1;
					if (m_mcu_reportcoin & 0x08)
					{
						m_mcu_initializing = 3;
						return 0xee;    /* tilt */
					}
					else return m_mcu_credits;
				}
				else return m_in0->read();  /* buttons */

			default:
				logerror("error, unknown mcu command\n");
				/* should not happen */
				return 0xff;
		}
	}
	else
	{
		/*
		status bits:
		0 = mcu is ready to send data (read from c000)
		1 = mcu has read data (from c000)
		2 = unused
		3 = unused
		4-7 = coin code
		      0 = nothing
		      1,2,3 = coin switch pressed
		      e = tilt
		*/
		if (m_mcu_reportcoin & 0x08) return 0xe1;   /* tilt */
		if (m_mcu_reportcoin & 0x01) return 0x11;   /* coin 1 (will trigger "coin inserted" sound) */
		if (m_mcu_reportcoin & 0x02) return 0x21;   /* coin 2 (will trigger "coin inserted" sound) */
		if (m_mcu_reportcoin & 0x04) return 0x31;   /* coin 3 (will trigger "coin inserted" sound) */
		return 0x01;
	}
}

void arknoid2_state::mcu_w(offs_t offset, uint8_t data)
{
	if (offset == 0)
	{
		//logerror("%s: write %02x to mcu %04x\n", m_maincpu->pc(), data, 0xc000 + offset);
		if (m_mcu_command == 0x41)
		{
			m_mcu_credits = (m_mcu_credits + data) & 0xff;
		}
	}
	else
	{
		/*
		0xc1: read number of credits, then buttons
		0x54+0x41: add value to number of credits
		0x15: sub 1 credit (when "Continue Play" only)
		0x84: coin 1 lockout (issued only in test mode)
		0x88: coin 2 lockout (issued only in test mode)
		0x80: release coin lockout (issued only in test mode)
		during initialization, a sequence of 4 bytes sets coin/credit settings
		*/
		//logerror("%s: write %02x to mcu %04x\n", m_maincpu->pc(), data, 0xc000 + offset);

		if (m_mcu_initializing)
		{
			/* set up coin/credit settings */
			m_mcu_coinage[m_mcu_coinage_init++] = data;
			if (m_mcu_coinage_init == 4)
				m_mcu_coinage_init = 0; /* must not happen */
		}

		if (data == 0xc1)
			m_mcu_readcredits = 0;  /* reset input port number */

		if (data == 0x15)
		{
			m_mcu_credits = (m_mcu_credits - 1) & 0xff;
			if (m_mcu_credits == 0xff)
				m_mcu_credits = 0;
		}
		m_mcu_command = data;
	}
}

INTERRUPT_GEN_MEMBER(arknoid2_state::mcu_interrupt)
{
	int coin = ((m_coin1->read() & 1) << 0);
	coin |= ((m_coin2->read() & 1) << 1);
	coin |= ((m_in2->read() & 3) << 2);
	coin ^= 0x0c;
	mcu_handle_coins(coin);

	device.execute().set_input_line(0, HOLD_LINE);
}

void arknoid2_state::machine_reset()
{
	plumppop_state::machine_reset();

	// initialize the MCU simulation
	mcu_reset();

	m_mcu_readcredits = 0;
	m_insertcoin = 0;
}

void kageki_state::machine_reset()
{
	insectx_state::machine_reset();

	m_csport_sel = 0;
}

void insectx_state::machine_start()
{
	tnzs_video_state_base::machine_start();

	uint8_t *const sub = memregion("sub")->base();
	m_subbank->configure_entries(0, 4, &sub[0x08000], 0x2000);
	m_subbank->set_entry(0);

	m_ramromview.select(2);
}

void arknoid2_state::machine_start()
{
	plumppop_state::machine_start();

	save_item(NAME(m_mcu_readcredits));
	save_item(NAME(m_insertcoin));
	save_item(NAME(m_mcu_initializing));
	save_item(NAME(m_mcu_coinage_init));
	save_item(NAME(m_mcu_coinage));
	save_item(NAME(m_mcu_coins_a));
	save_item(NAME(m_mcu_coins_b));
	save_item(NAME(m_mcu_credits));
	save_item(NAME(m_mcu_reportcoin));
	save_item(NAME(m_mcu_command));

	// kludge to make device work with active-high coin inputs
	m_upd4701->left_w(0);
	m_upd4701->middle_w(0);
}

void kageki_state::machine_start()
{
	insectx_state::machine_start();

	save_item(NAME(m_csport_sel));
}

void kabukiz_state::machine_start()
{
	tnzsb_state::machine_start();

	uint8_t *sound = memregion("audiocpu")->base();
	m_audiobank->configure_entries(0, 8, &sound[0x00000], 0x4000);
}

void insectx_state::ramrom_bankswitch_w(uint8_t data)
{
//  logerror("%s: writing %02x to bankswitch\n", m_maincpu->pc(),data);

	// bit 4 resets the second CPU
	m_subcpu->set_input_line(INPUT_LINE_RESET, BIT(data, 4) ? CLEAR_LINE : ASSERT_LINE);

	// bits 0-2 select RAM/ROM bank
	m_ramromview.select(data & 0x07);
}

void arknoid2_state::arknoid2_bankswitch1_w(uint8_t data)
{
	// bits 0-1 select ROM bank
	m_subbank->set_entry(data & 0x03);

	if (BIT(data, 2))
		mcu_reset();

	// never actually written by arknoid2 (though code exists to do it)
	m_upd4701->resetx_w(BIT(data, 5));
	m_upd4701->resety_w(BIT(data, 5));
}

void insectx_state::insectx_bankswitch1_w(uint8_t data)
{
	// bits 0-1 select ROM bank
	m_subbank->set_entry(data & 0x03);

	machine().bookkeeping().coin_lockout_w(0, BIT(~data, 2));
	machine().bookkeeping().coin_lockout_w(1, BIT(~data, 3));
	machine().bookkeeping().coin_counter_w(0, BIT(data, 4));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 5));
}

void tnzsb_state::tnzsb_bankswitch1_w(uint8_t data)
{
	// bits 0-1 select ROM bank
	m_subbank->set_entry(data & 0x03);

	machine().bookkeeping().coin_lockout_w(0, BIT(~data, 4));
	machine().bookkeeping().coin_lockout_w(1, BIT(~data, 5));
	machine().bookkeeping().coin_counter_w(0, BIT(data, 2));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 3));
}

void kageki_state::kageki_bankswitch1_w(uint8_t data)
{
	// bits 0-1 select ROM bank
	m_subbank->set_entry(data & 0x03);

	machine().bookkeeping().coin_lockout_global_w(BIT(~data, 5));
	machine().bookkeeping().coin_counter_w(0, BIT(data, 2));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 3));
}

void tnzs_mcu_state::tnzs_bankswitch1_w(uint8_t data)
{
	// bits 0-1 select ROM bank
	m_subbank->set_entry(data & 0x03);

	if (BIT(data, 2))
		m_mcu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}

void plumppop_state::plumppop_bankswitch1_w(uint8_t data)
{
	tnzs_bankswitch1_w(data);

	// written only at startup by plumppop?
	m_upd4701->resetx_w(BIT(data, 5));
	m_upd4701->resety_w(BIT(data, 5));
}

void jpopnics_state::subbankswitch_w(uint8_t data)
{
	// bits 0-1 select ROM bank
	m_subbank->set_entry(data & 0x03);

	// written once at startup
	m_upd4701->resetx_w(BIT(data, 5));
	m_upd4701->resety_w(BIT(data, 5));
}

void tnzsb_state::sound_command_w(uint8_t data)
{
	m_soundlatch->write(data);
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80
}

// handler called by the 2203 emulator when the internal timers cause an IRQ
void tnzsb_state::ym2203_irqhandler(int state)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, state ? ASSERT_LINE : CLEAR_LINE);
}

void kabukiz_state::sound_bank_w(uint8_t data)
{
	// to avoid the write when the sound chip is initialized
	if (data != 0xff)
		m_audiobank->set_entry(data & 0x07);
}


SAMPLES_START_CB_MEMBER(kageki_state::init_samples)
{
	uint8_t *src = memregion("samples")->base() + 0x0090;
	for (int i = 0; i < MAX_SAMPLES; i++)
	{
		int start = (src[(i * 2) + 1] * 256) + src[(i * 2)];
		uint8_t *scan = &src[start];
		int size = 0;

		// check sample length
		while (*scan++ != 0x00)
			size++;

		/* 2009-11 FP: should these be saved? */
		m_sampledata[i] = std::make_unique<int16_t[]>(size);
		m_samplesize[i] = size;

		if (start < 0x100)
			start = size = 0;

		// signed 8-bit sample to unsigned 8-bit sample convert
		int16_t *dest = m_sampledata[i].get();
		scan = &src[start];
		for (int n = 0; n < size; n++)
		{
			*dest++ = (int8_t)((*scan++) ^ 0x80) * 256;
		}
	//  logerror("samples num:%02X ofs:%04X lng:%04X\n", i, start, size);
	}
}


uint8_t kageki_state::csport_r()
{
	int dsw, dsw1, dsw2;

	dsw1 = m_dswa->read();
	dsw2 = m_dswb->read();

	switch (m_csport_sel)
	{
		case    0x00:           // DSW2 5,1 / DSW1 5,1
			dsw = (((dsw2 & 0x10) >> 1) | ((dsw2 & 0x01) << 2) | ((dsw1 & 0x10) >> 3) | ((dsw1 & 0x01) >> 0));
			break;
		case    0x01:           // DSW2 7,3 / DSW1 7,3
			dsw = (((dsw2 & 0x40) >> 3) | ((dsw2 & 0x04) >> 0) | ((dsw1 & 0x40) >> 5) | ((dsw1 & 0x04) >> 2));
			break;
		case    0x02:           // DSW2 6,2 / DSW1 6,2
			dsw = (((dsw2 & 0x20) >> 2) | ((dsw2 & 0x02) << 1) | ((dsw1 & 0x20) >> 4) | ((dsw1 & 0x02) >> 1));
			break;
		case    0x03:           // DSW2 8,4 / DSW1 8,4
			dsw = (((dsw2 & 0x80) >> 4) | ((dsw2 & 0x08) >> 1) | ((dsw1 & 0x80) >> 6) | ((dsw1 & 0x08) >> 3));
			break;
		default:
			dsw = 0x00;
		//  logerror("csport_sel error !! (0x%08X)\n", m_csport_sel);
	}

	return (dsw & 0xff);
}

void kageki_state::csport_w(uint8_t data)
{
	if (data > 0x3f)
	{
		// read dipsw port
		m_csport_sel = (data & 0x03);
	}
	else
	{
		if (data < MAX_SAMPLES)
		{
			// play samples
			m_samples->start_raw(0, m_sampledata[data].get(), m_samplesize[data], 7000);
			LOG("VOICE:%02X PLAY", data);
		}
		else
		{
			// stop samples
			m_samples->stop(0);
			LOG("VOICE:%02X STOP", data);
		}
	}
}

void insectx_state::prompal_main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).view(m_ramromview);
	for (int i = 0; 2 > i; ++i)
		m_ramromview[i](0x8000, 0xbfff).ram(); // instead of the first two banks of ROM being repeated redundantly the hardware maps RAM here
	for (int i = 2; 8 > i; ++i)
		m_ramromview[i](0x8000, 0xbfff).rom().region("maincpu", 0x4000 * i);
	map(0xc000, 0xcfff).rw(m_spritegen, FUNC(x1_001_device::spritecodelow_r8), FUNC(x1_001_device::spritecodelow_w8));
	map(0xd000, 0xdfff).rw(m_spritegen, FUNC(x1_001_device::spritecodehigh_r8), FUNC(x1_001_device::spritecodehigh_w8));
	map(0xe000, 0xefff).ram().share("share1"); // WORK RAM (shared by the 2 Z80's)
	map(0xf000, 0xf2ff).rw(m_spritegen, FUNC(x1_001_device::spriteylow_r8), FUNC(x1_001_device::spriteylow_w8));
	map(0xf300, 0xf303).mirror(0xfc).w(m_spritegen, FUNC(x1_001_device::spritectrl_w8));  // control registers (0x80 mirror used by Arkanoid 2)
	map(0xf400, 0xf400).w(m_spritegen, FUNC(x1_001_device::spritebgflag_w8));             // enable / disable background transparency
	map(0xf600, 0xf600).nopr().w(FUNC(insectx_state::ramrom_bankswitch_w));

	// arknoid2, extrmatn, plumppop and drtoppel have PROMs instead of RAM
	// drtoppel and kabukiz write here anyway!
	map(0xf800, 0xfbff).nopw();
}


/***************************************************************************

  The New Zealand Story doesn't have a color PROM. It uses 1024 bytes of RAM
  to dynamically create the palette. Each couple of bytes defines one
  color (15 bits per pixel; the top bit of the second byte is unused).
  Since the graphics use 4 bitplanes, hence 16 colors, this makes for 32
  different color codes.

***************************************************************************/

void insectx_state::rampal_main_map(address_map &map)
{
	prompal_main_map(map);

	map(0xf800, 0xfbff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
}

void insectx_state::base_sub_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).bankr(m_subbank);
	map(0xb000, 0xb001).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xd000, 0xdfff).ram();
	map(0xe000, 0xefff).ram().share("share1");
}

void tnzs_mcu_state::tnzs_sub_map(address_map &map)
{
	base_sub_map(map);

	map(0xa000, 0xa000).w(FUNC(tnzs_mcu_state::tnzs_bankswitch1_w));
	map(0xc000, 0xc001).rw(FUNC(tnzs_mcu_state::mcu_r), FUNC(tnzs_mcu_state::mcu_w));
	map(0xf000, 0xf003).nopr(); // paddles in arkanoid2/plumppop. The ports are read but not used by the other games, and are not read at all by insectx.
}

void plumppop_state::plumppop_sub_map(address_map &map)
{
	tnzs_sub_map(map);

	map(0xa000, 0xa000).w(FUNC(plumppop_state::plumppop_bankswitch1_w));
	map(0xf000, 0xf003).r(m_upd4701, FUNC(upd4701_device::read_xy));
}

void arknoid2_state::arknoid2_sub_map(address_map &map)
{
	plumppop_sub_map(map);

	map(0xa000, 0xa000).w(FUNC(arknoid2_state::arknoid2_bankswitch1_w));
	map(0xc000, 0xc001).rw(FUNC(arknoid2_state::mcu_r), FUNC(arknoid2_state::mcu_w));
}

void kageki_state::kageki_sub_map(address_map &map)
{
	base_sub_map(map);

	map(0xa000, 0xa000).w(FUNC(kageki_state::kageki_bankswitch1_w));
	map(0xc000, 0xc000).portr("IN0");
	map(0xc001, 0xc001).portr("IN1");
	map(0xc002, 0xc002).portr("IN2");
}

void insectx_state::insectx_sub_map(address_map &map)
{
	base_sub_map(map);

	map(0xa000, 0xa000).w(FUNC(insectx_state::insectx_bankswitch1_w));
	map(0xc000, 0xc000).portr("IN0");
	map(0xc001, 0xc001).portr("IN1");
	map(0xc002, 0xc002).portr("IN2");
}

// the later board is different, it has a third CPU (and of course no MCU)

void tnzsb_state::tnzsb_base_sub_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).bankr(m_subbank);
	map(0xa000, 0xa000).w(FUNC(tnzsb_state::tnzsb_bankswitch1_w));
	map(0xb002, 0xb002).portr("DSWA");
	map(0xb003, 0xb003).portr("DSWB");
	map(0xb004, 0xb004).w(FUNC(tnzsb_state::sound_command_w));
	map(0xc000, 0xc000).portr("IN0");
	map(0xc001, 0xc001).portr("IN1");
	map(0xc002, 0xc002).portr("IN2");
	map(0xd000, 0xdfff).ram();
	map(0xe000, 0xefff).ram().share("share1");
}

void tnzsb_state::tnzsb_sub_map(address_map &map)
{
	tnzsb_base_sub_map(map);

	map(0xf000, 0xf3ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
}

void kabukiz_state::kabukiz_sub_map(address_map &map)
{
	tnzsb_base_sub_map(map);

	map(0xf800, 0xfbff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
}

void tnzsb_state::tnzsb_cpu2_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xdfff).ram();
}

void kabukiz_state::kabukiz_cpu2_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_audiobank);
	map(0xe000, 0xffff).ram();
}

void tnzsb_state::tnzsb_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x02, 0x02).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

void jpopnics_state::jpopnics_main_map(address_map &map)
{
	prompal_main_map(map);

	map(0xf800, 0xffff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette"); // larger palette
}

void jpopnics_state::jpopnics_sub_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).bankr(m_subbank);

	map(0xa000, 0xa000).w(FUNC(jpopnics_state::subbankswitch_w));
	map(0xb000, 0xb001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xc000, 0xc000).portr("IN1");
	map(0xc001, 0xc001).portr("IN2");
	map(0xc600, 0xc600).portr("DSWA");
	map(0xc601, 0xc601).portr("DSWB");

	map(0xd000, 0xdfff).ram();
	map(0xe000, 0xefff).ram().share("share1");

	map(0xf000, 0xf003).r(m_upd4701, FUNC(upd4701_device::read_xy));
}


static INPUT_PORTS_START( common_in2 )
	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( common_coins )
	PORT_START("COIN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("COIN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
INPUT_PORTS_END


static INPUT_PORTS_START( plumppop )
	/* 0xb001 (CPU1) port 0 -> 0xef0e (shared RAM) */
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SWA:1") /* code at 0x6e99 - is it ever called ? */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	TAITO_DSWA_BITS_1_TO_3_LOC(SWA)
	TAITO_COINAGE_JAPAN_OLD_LOC(SWA)

	/* 0xb001 (CPU1) port 1 -> 0xef0f (shared RAM) */
	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SWB)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SWB:3,4") /* table at 0x2b86 */
	PORT_DIPSETTING(    0x08, "50k 200k 150k+" )
	PORT_DIPSETTING(    0x0c, "50k 250k 200k+" )
	PORT_DIPSETTING(    0x04, "100k 300k 200k+" )
	PORT_DIPSETTING(    0x00, "100k 400k 300k+" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x40, 0x40, "P1 & P2 Children Collision" ) PORT_DIPLOCATION("SWB:7") /* code at 0x3dcc */
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )  // both players' children collide with each other / Off=No / On=Yes
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_PLAYER(1) PORT_NAME("P1 Warp (Cheat)") // not a cabinet button, so don't map it by default
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_PLAYER(2) PORT_NAME("P2 Warp (Cheat)") // see above, but P2 one only works in testmode?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_INCLUDE( common_in2 )

	PORT_INCLUDE( common_coins )

	PORT_START("AN1")       /* spinner 1 - read at f000/1 */
	PORT_BIT( 0x0fff, 0x0000, IPT_DIAL ) PORT_SENSITIVITY(70) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("AN2")       /* spinner 2 - read at f002/3 */
	PORT_BIT( 0x0fff, 0x0000, IPT_DIAL ) PORT_SENSITIVITY(70) PORT_KEYDELTA(15) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( extrmatn )
	/* 0xb001 (CPU1) port 0 -> 0xef0e (shared RAM) */
	PORT_START("DSWA")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SWA:1" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SWA:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SWA:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SWA:4" )
	TAITO_COINAGE_JAPAN_OLD_LOC(SWA)

	/* 0xb001 (CPU1) port 1 -> 0xef0f (shared RAM) */
	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SWB)
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SWB:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SWB:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SWB:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SWB:6" )
	PORT_DIPNAME( 0xc0, 0xc0, "Damage Multiplier" ) PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(    0xc0, "*1" )
	PORT_DIPSETTING(    0x80, "*1.5" )
	PORT_DIPSETTING(    0x40, "*2" )
	PORT_DIPSETTING(    0x00, "*3" )

	PORT_START("IN0")
	TAITO_JOY_LRUD_2_BUTTONS_START( 1 )

	PORT_START("IN1")
	TAITO_JOY_LRUD_2_BUTTONS_START( 2 )

	PORT_INCLUDE( common_in2 )

	PORT_INCLUDE( common_coins )
INPUT_PORTS_END


static INPUT_PORTS_START( arknoid2 )
	/* 0xb001 (CPU1) port 0 -> 0xe001 (shared RAM) */
	PORT_START("DSWA")
	TAITO_MACHINE_COCKTAIL_LOC(SWA)
	TAITO_COINAGE_WORLD_LOC(SWA)

	/* 0xb001 (CPU1) port 1 -> 0xe002 (shared RAM) */
	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SWB)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SWB:3,4") /* table at 0x6f1e (W and J) or 0x6f1b (Romstar) */
	PORT_DIPSETTING(    0x00, "50k 150k 150k+" )
	PORT_DIPSETTING(    0x0c, "100k 200k 200k+" )
	PORT_DIPSETTING(    0x04, "50k only" )
	PORT_DIPSETTING(    0x08, "100k only" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SWB:7" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("IN0")       /* read at c000 (sound cpu) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_WRITE_LINE_DEVICE_MEMBER("upd4701", upd4701_device, right_w)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN1")
	PORT_BIT( 1, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_WRITE_LINE_DEVICE_MEMBER("upd4701", upd4701_device, left_w)

	PORT_START("COIN2")
	PORT_BIT( 1, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_WRITE_LINE_DEVICE_MEMBER("upd4701", upd4701_device, middle_w)

	PORT_START("AN1")       /* spinner 1 - read at f000/1 */
	PORT_BIT( 0x0fff, 0x0000, IPT_DIAL ) PORT_SENSITIVITY(70) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("AN2")       /* spinner 2 - read at f002/3 */
	PORT_BIT( 0x0fff, 0x0000, IPT_DIAL ) PORT_SENSITIVITY(70) PORT_KEYDELTA(15) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( arknid2u )
	PORT_INCLUDE( arknoid2 )

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SWA)
INPUT_PORTS_END


static INPUT_PORTS_START( drtoppel )
	/* 0xb001 (CPU1) port 0 -> 0xe042 (shared RAM) */
	PORT_START("DSWA")
	TAITO_MACHINE_COCKTAIL_LOC(SWA)
	TAITO_COINAGE_WORLD_LOC(SWA)

	/* 0xb001 (CPU1) port 1 -> 0xe043 (shared RAM) */
	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SWB)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SWB:3,4") /* table at 0x256d (4 * 9 bytes) */
	PORT_DIPSETTING(    0x0c, "30k 100k 200k 100k+" )             /* 30k  100k  200k  300k  400k  500k  600k  700k */
	PORT_DIPSETTING(    0x00, "50k 100k 200k 200k+" )             /* 50k  100k  200k  400k  600k  800k 1000k 1200k */
	PORT_DIPSETTING(    0x04, "30k 100k" )
	PORT_DIPSETTING(    0x08, "30k only" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SWB:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SWB:8" )

	PORT_START("IN0")
	TAITO_JOY_LRUD_2_BUTTONS_START( 1 )

	PORT_START("IN1")
	TAITO_JOY_LRUD_2_BUTTONS_START( 2 )

	PORT_INCLUDE( common_in2 )

	PORT_INCLUDE( common_coins )
INPUT_PORTS_END

static INPUT_PORTS_START( drtopplu )
	PORT_INCLUDE( drtoppel )

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SWA)
INPUT_PORTS_END


static INPUT_PORTS_START( kageki )
	/* special (see kageki_csport_* handlers) -> 0xe03b (shared RAM) */
	PORT_START("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SWA)                           /* see notes */
	TAITO_COINAGE_WORLD_LOC(SWA)

	/* special (see kageki_csport_* handlers) -> 0xe03c (shared RAM) */
	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SWB)
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SWB:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SWB:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SWB:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SWB:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SWB:7" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("IN0")
	TAITO_JOY_LRUD_2_BUTTONS_START( 1 )

	PORT_START("IN1")
	TAITO_JOY_LRUD_2_BUTTONS_START( 2 )

	PORT_INCLUDE( common_in2 )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
INPUT_PORTS_END

static INPUT_PORTS_START( kagekiu )
	PORT_INCLUDE( kageki )

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SWA)
INPUT_PORTS_END

static INPUT_PORTS_START( kagekij )
	PORT_INCLUDE( kagekiu )

	PORT_MODIFY("DSWA")
	TAITO_MACHINE_COCKTAIL_LOC(SWA) /* see notes */
INPUT_PORTS_END


static INPUT_PORTS_START( chukatai )
	/* 0xb001 (CPU1) port 0 -> 0xe015 (shared RAM) */
	PORT_START("DSWA")
	TAITO_MACHINE_COCKTAIL_LOC(SWA)
	TAITO_COINAGE_WORLD_LOC(SWA)

	/* 0xb001 (CPU1) port 1 -> 0xe016 (shared RAM) */
	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SWB)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SWB:3,4") /* tables : 1st at 0xb070 (bank = 0x02) and inc. at 0x09df */
	PORT_DIPSETTING(    0x08, "40k 240k 200k+" )
	PORT_DIPSETTING(    0x0c, "60k 360k 300k+" )
	PORT_DIPSETTING(    0x04, "100k 500k 400k+" )
	PORT_DIPSETTING(    0x00, "150k 650k 500k+" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SWB:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SWB:8" )

	PORT_START("IN0")
	TAITO_JOY_LRUD_2_BUTTONS_START( 1 )

	PORT_START("IN1")
	TAITO_JOY_LRUD_2_BUTTONS_START( 2 )

	PORT_INCLUDE( common_in2 )

	PORT_INCLUDE( common_coins )
INPUT_PORTS_END

static INPUT_PORTS_START( chukatau )
	PORT_INCLUDE( chukatai )

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SWA)
INPUT_PORTS_END


static INPUT_PORTS_START( tnzs )
	/* 0xb002 (CPU1) -> 0xef0e (shared RAM) */
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SWA:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SWA:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SWA:3" )
	PORT_DIPNAME( 0x08, 0x08, "Invulnerability (Debug)" ) PORT_DIPLOCATION("SWA:4") // see notes
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	TAITO_COINAGE_WORLD_LOC(SWA)

	/* 0xb003 (CPU1) -> 0xef0f (shared RAM) */
	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SWB)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SWB:3,4") /* table at 0x09c84 */
	PORT_DIPSETTING(    0x00, "50k 150k 150k+" )
	PORT_DIPSETTING(    0x0c, "70k 200k 200k+" )
	PORT_DIPSETTING(    0x04, "100k 250k 250k+" )
	PORT_DIPSETTING(    0x08, "200k 300k 300k+" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SWB:8" )

	PORT_START("IN0")
	TAITO_JOY_LRUD_2_BUTTONS_START( 1 )

	PORT_START("IN1")
	TAITO_JOY_LRUD_2_BUTTONS_START( 2 )

	PORT_INCLUDE( common_in2 )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
INPUT_PORTS_END

static INPUT_PORTS_START( tnzsj )
	PORT_INCLUDE( tnzs )

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SWA)
INPUT_PORTS_END

static INPUT_PORTS_START( tnzsjo )
	/* 0xb001 (CPU1) port 0 -> 0xef0e (shared RAM) */
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SWA:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SWA:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SWA:3" )
	PORT_DIPNAME( 0x08, 0x08, "Invulnerability (Debug)" ) PORT_DIPLOCATION("SWA:4") // see notes
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	TAITO_COINAGE_JAPAN_OLD_LOC(SWA)

	/* 0xb001 (CPU1) port 1 -> 0xef0f (shared RAM) */
	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SWB)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SWB:3,4") /* table at 0x09caf */
	PORT_DIPSETTING(    0x00, "50k 150k 150k+" )
	PORT_DIPSETTING(    0x0c, "70k 200k 200k+" )
	PORT_DIPSETTING(    0x04, "100k 250k 250k+" )
	PORT_DIPSETTING(    0x08, "200k 300k 300k+" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SWB:8" )

	PORT_START("IN0")
	TAITO_JOY_LRUD_2_BUTTONS_START( 1 )

	PORT_START("IN1")
	TAITO_JOY_LRUD_2_BUTTONS_START( 2 )

	PORT_INCLUDE( common_in2 )

	PORT_START("COIN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("COIN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
INPUT_PORTS_END

static INPUT_PORTS_START( tnzsop )
	PORT_INCLUDE( tnzsjo )

	PORT_MODIFY("DSWA")
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SWA:4" )        /* value read at 0x356b but not tested nor stored elsewhere */
	TAITO_COINAGE_WORLD_LOC(SWA)

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SWB:3,4") /* table at 0x09afb */
	PORT_DIPSETTING(    0x00, "10k 100k 100k+" )
	PORT_DIPSETTING(    0x0c, "10k 150k 150k+" )
	PORT_DIPSETTING(    0x04, "10k 200k 200k+" )
	PORT_DIPSETTING(    0x08, "10k 300k 300k+" )
INPUT_PORTS_END


static INPUT_PORTS_START( kabukiz )
	/* 0xb002 (CPU1) */
	PORT_START("DSWA")
	TAITO_MACHINE_COCKTAIL_LOC(SWA)
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SWA:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SWA:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SWA:7" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	/* 0xb003 (CPU1) */
	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWB:1,2") // different from many other taito
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )   // other taito : [ On Off ] < [ Off Off ] < [ Off On ] < [ On On ]
	PORT_DIPSETTING(    0x02, DEF_STR( Medium ) ) //     kabukiz : [ Off Off ] < [ On Off ] < [ Off On ] < [ On On ]
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SWB:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SWB:4" )
	TAITO_COINAGE_WORLD_LOC(SWB)

	PORT_START("IN0")
	TAITO_JOY_LRUD_2_BUTTONS_START( 1 )

	PORT_START("IN1")
	TAITO_JOY_LRUD_2_BUTTONS_START( 2 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( kabukizj )
	PORT_INCLUDE( kabukiz )

	PORT_MODIFY("DSWB")
	TAITO_COINAGE_JAPAN_OLD_LOC(SWB)
INPUT_PORTS_END


static INPUT_PORTS_START( insectx )
	/* 0xb001 (CPU1) port 0 -> 0xec06 (shared RAM) */
	PORT_START("DSWA")
	TAITO_MACHINE_COCKTAIL_LOC(SWA)
	TAITO_COINAGE_WORLD_LOC(SWA)

	/* 0xb001 (CPU1) port 1 -> 0xec07 (shared RAM) */
	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWB:1,2") /* see notes */
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )   // other taito : [ On Off ] < [ Off Off ] < [ Off On ] < [ On On ]
	PORT_DIPSETTING(    0x03, DEF_STR( Medium ) ) //       notes : [ Off On ] < [ Off Off ] < [ On Off ] < [ On On ]
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )   //      manual : [ Off Off ] < [ On Off ] < [ Off On ] < [ On On ]
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SWB:3,4") /* code at 0xaacc - bank = 0x02 */
	PORT_DIPSETTING(    0x08, "40k 240k 200k+" )
	PORT_DIPSETTING(    0x0c, "60k 360k 300k+" )
	PORT_DIPSETTING(    0x04, "100k 500k 400k+" )
	PORT_DIPSETTING(    0x00, "150k 650k 500k+" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SWB:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SWB:8" )

	PORT_START("IN0")
	TAITO_JOY_LRUD_2_BUTTONS_START( 1 )

	PORT_START("IN1")
	TAITO_JOY_LRUD_2_BUTTONS_START( 2 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( insectxj )
	PORT_INCLUDE( insectx )

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SWA)
INPUT_PORTS_END


static INPUT_PORTS_START( jpopnics )
	/* 0xc600 (CPU1) -> 0xef0e (shared RAM) */
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SWA:1") /* code at 0x6e99 - is it ever called ? */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	TAITO_DSWA_BITS_1_TO_3
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SWA:5" )        /* see notes */
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SWA:6" )        /* see notes */
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SWA:7" )        /* see notes */
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SWA:8" )        /* see notes */

	/* 0xc601 (CPU1) -> 0xef0f (shared RAM) */
	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SWB)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SWB:3,4") /* table at 0x2b86 */
	PORT_DIPSETTING(    0x08, "50k 200k 150k+" )
	PORT_DIPSETTING(    0x0c, "50k 250k 200k+" )
	PORT_DIPSETTING(    0x04, "100k 300k 200k+" )
	PORT_DIPSETTING(    0x00, "100k 400k 300k+" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x40, 0x40, "P1 & P2 Children Collision" ) PORT_DIPLOCATION("SWB:7") /* code at 0x3dcc */
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )  // both players' children collide with each other / Off=No / On=Yes
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_PLAYER(1) PORT_NAME("P1 Warp (Cheat)") // not a cabinet button, so don't map it by default
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_PLAYER(2) PORT_NAME("P2 Warp (Cheat)") // see above, but P2 one only works in testmode?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("AN1")       /* spinner 1 - read at f000/1 */
	PORT_BIT( 0x0fff, 0x0000, IPT_DIAL ) PORT_SENSITIVITY(70) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("AN2")       /* spinner 2 - read at f002/3 */
	PORT_BIT( 0x0fff, 0x0000, IPT_DIAL ) PORT_SENSITIVITY(70) PORT_KEYDELTA(15) PORT_PLAYER(2)
INPUT_PORTS_END


static const gfx_layout tnzs_charlayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};

static const gfx_layout insectx_charlayout =
{
	16,16,
	8192,
	4,
	{ 8, 0, 8192*64*8+8, 8192*64*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*16+0, 8*16+1, 8*16+2, 8*16+3, 8*16+4, 8*16+5, 8*16+6, 8*16+7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		16*16, 17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16 },
	64*8
};

static GFXDECODE_START( gfx_tnzs )
	GFXDECODE_ENTRY( "gfx1", 0, tnzs_charlayout, 0, 32 )
GFXDECODE_END

static GFXDECODE_START( gfx_insectx )
	GFXDECODE_ENTRY( "gfx1", 0, insectx_charlayout, 0, 32 )
GFXDECODE_END

void insectx_state::tnzs_base(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(12'000'000)/2);       /* 6.0 MHz ??? - Main board Crystal is 12MHz, verified on insectx, kageki, tnzsb */
	m_maincpu->set_addrmap(AS_PROGRAM, &insectx_state::rampal_main_map);
	m_maincpu->set_vblank_int("screen", FUNC(insectx_state::irq0_line_hold));

	Z80(config, m_subcpu, XTAL(12'000'000)/2);        /* 6.0 MHz ??? - Main board Crystal is 12MHz, verified on insectx, kageki, tnzsb */
	m_subcpu->set_addrmap(AS_PROGRAM, &insectx_state::base_sub_map);
	m_subcpu->set_vblank_int("screen", FUNC(insectx_state::irq0_line_hold));

	config.set_perfect_quantum(m_maincpu);

	/* video hardware */
	X1_001(config, m_spritegen, 12'000'000, m_palette, gfx_tnzs);
	m_spritegen->set_fg_yoffsets( -0x12, 0x0e );
	m_spritegen->set_bg_yoffsets( 0x1, -0x1 );

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(insectx_state::screen_update_tnzs));
	m_screen->screen_vblank().set(FUNC(insectx_state::screen_vblank_tnzs));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 512);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
}

template <bool LockoutLevel>
void tnzs_mcu_state::tnzs_mcu(machine_config &config)
{
	tnzs_base(config);

	I8742(config, m_mcu, 12000000/2);  /* 400KHz ??? - Main board Crystal is 12MHz */
	m_mcu->p1_in_cb().set(FUNC(tnzs_mcu_state::mcu_port1_r));
	m_mcu->p2_in_cb().set_ioport("IN2");
	m_mcu->p2_out_cb().set(FUNC(tnzs_mcu_state::mcu_port2_w<LockoutLevel>));
	m_mcu->t0_in_cb().set_ioport("COIN1");
	m_mcu->t1_in_cb().set_ioport("COIN2");

	m_subcpu->set_addrmap(AS_PROGRAM, &tnzs_mcu_state::tnzs_sub_map);

	/* video hardware */
	m_screen->set_refresh_hz(59.15);   /* it should be the same as the newer pcb vsync */
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */

	/* sound hardware */
	ym2203_device &ymsnd(YM2203(config, "ymsnd", XTAL(12'000'000)/4));
	ymsnd.port_a_read_callback().set_ioport("DSWA");
	ymsnd.port_b_read_callback().set_ioport("DSWB");
	ymsnd.add_route(ALL_OUTPUTS, "speaker", 0.3);
}

void tnzs_mcu_state::tnzs(machine_config &config)
{
	tnzs_mcu<true>(config);
}

void tnzs_mcu_state::extrmatn(machine_config &config)
{
	tnzs_mcu<false>(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &tnzs_mcu_state::prompal_main_map);

	// video hardware
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));

	m_palette->set_init(FUNC(tnzs_mcu_state::prompalette));
}

void plumppop_state::plumppop(machine_config &config)
{
	extrmatn(config);

	m_subcpu->set_addrmap(AS_PROGRAM, &plumppop_state::plumppop_sub_map);

	UPD4701A(config, m_upd4701);
	m_upd4701->set_portx_tag("AN1");
	m_upd4701->set_porty_tag("AN2");
}

void arknoid2_state::arknoid2(machine_config &config)
{
	plumppop(config);

	m_maincpu->set_vblank_int("screen", FUNC(arknoid2_state::mcu_interrupt));

	m_subcpu->set_addrmap(AS_PROGRAM, &arknoid2_state::arknoid2_sub_map);

	m_mcu->set_disable();
}

void insectx_state::insectx(machine_config &config)
{
	tnzs_base(config);

	/* basic machine hardware */
	m_subcpu->set_addrmap(AS_PROGRAM, &insectx_state::insectx_sub_map);

	/* video hardware */
	m_spritegen->set_info(gfx_insectx);

	/* sound hardware */
	ym2203_device &ymsnd(YM2203(config, "ymsnd", XTAL(12'000'000)/4)); /* verified on pcb */
	ymsnd.port_a_read_callback().set_ioport("DSWA");
	ymsnd.port_b_read_callback().set_ioport("DSWB");
	ymsnd.add_route(ALL_OUTPUTS, "speaker", 0.3);
}

void kageki_state::kageki(machine_config &config)
{
	tnzs_base(config);

	/* basic machine hardware */
	m_subcpu->set_addrmap(AS_PROGRAM, &kageki_state::kageki_sub_map);

	/* sound hardware */
	ym2203_device &ymsnd(YM2203(config, "ymsnd", XTAL(12'000'000)/4)); /* verified on pcb */
	ymsnd.port_a_read_callback().set(FUNC(kageki_state::csport_r));
	ymsnd.port_b_write_callback().set(FUNC(kageki_state::csport_w));
	ymsnd.add_route(0, "speaker", 0.15);
	ymsnd.add_route(1, "speaker", 0.15);
	ymsnd.add_route(2, "speaker", 0.15);
	ymsnd.add_route(3, "speaker", 0.35);

	SAMPLES(config, m_samples);
	m_samples->set_channels(1);
	m_samples->set_samples_start_callback(FUNC(kageki_state::init_samples));
	m_samples->add_route(ALL_OUTPUTS, "speaker", 1.0);
}

void tnzsb_state::tnzsb(machine_config &config)
{
	tnzs_base(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &tnzsb_state::prompal_main_map);
	m_subcpu->set_addrmap(AS_PROGRAM, &tnzsb_state::tnzsb_sub_map);

	Z80(config, m_audiocpu, XTAL(12'000'000)/2); /* verified on pcb */
	m_audiocpu->set_addrmap(AS_PROGRAM, &tnzsb_state::tnzsb_cpu2_map);
	m_audiocpu->set_addrmap(AS_IO, &tnzsb_state::tnzsb_io_map);

	/* video hardware */
	m_screen->set_refresh_hz(59.15);   /* verified on pcb */

	/* sound hardware */
	GENERIC_LATCH_8(config, m_soundlatch);

	ym2203_device &ymsnd(YM2203(config, "ymsnd", XTAL(12'000'000)/4)); /* verified on pcb */
	ymsnd.irq_handler().set(FUNC(tnzsb_state::ym2203_irqhandler));
	ymsnd.add_route(0, "speaker", 1.0);
	ymsnd.add_route(1, "speaker", 1.0);
	ymsnd.add_route(2, "speaker", 1.0);
	ymsnd.add_route(3, "speaker", 2.0);
}

void kabukiz_state::kabukiz(machine_config &config)
{
	tnzsb(config);

	/* basic machine hardware */
	m_subcpu->set_addrmap(AS_PROGRAM, &kabukiz_state::kabukiz_sub_map);
	m_audiocpu->set_addrmap(AS_PROGRAM, &kabukiz_state::kabukiz_cpu2_map);

	/* sound hardware */
	ym2203_device &ymsnd(*subdevice<ym2203_device>("ymsnd"));
	ymsnd.port_a_write_callback().set(FUNC(kabukiz_state::sound_bank_w));
	ymsnd.port_b_write_callback().set("dac", FUNC(dac_byte_interface::data_w));

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // unknown DAC
}

void jpopnics_state::jpopnics(machine_config &config)
{
	tnzs_base(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &jpopnics_state::jpopnics_main_map);
	m_subcpu->set_addrmap(AS_PROGRAM, &jpopnics_state::jpopnics_sub_map);

	UPD4701A(config, m_upd4701);
	m_upd4701->set_portx_tag("AN1");
	m_upd4701->set_porty_tag("AN2");

	/* video hardware */
	m_palette->set_format(palette_device::GBRx_444, 1024); // wrong, the other 4 bits seem to be used as well
	m_palette->set_endianness(ENDIANNESS_BIG);

	/* sound hardware */
	YM2151(config, "ymsnd", XTAL(12'000'000)/4).add_route(ALL_OUTPUTS, "speaker", 0.3); /* Not verified - Main board Crystal is 12MHz */
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( plumppop )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 64k + bankswitch areas for the first CPU */
	ROM_LOAD( "a98__09.11c", 0x00000, 0x10000, CRC(107f9e06) SHA1(0aa7f32721c3cab96eccc7c831b9f57877c4e1dc) )
	ROM_LOAD( "a98__10.9c", 0x10000, 0x10000, CRC(df6e6af2) SHA1(792f97f587e84cdd67f0d1efe1fd13ea904d7e20) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "a98__11.4e", 0x00000, 0x10000, CRC(bc56775c) SHA1(0c22c22c0e9d7ec0e34f8ab4bfe61068f65e8759) )

	ROM_REGION( 0x10000, "mcu", 0 )    /* M-Chip (i8x42 internal ROM) */
	ROM_LOAD( "b06__14.1g", 0x0000, 0x0800, CRC(28907072) SHA1(21c7017af8a8ceb8e43d7e798f48518b136fd45c) ) /* Labeled B06-14 and under printed label "Taito M-001, 128P, 720100", is a mask 8042 */

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "a98__01.mbm27c512.13a", 0x00000, 0x10000, CRC(f3033dca) SHA1(130744998f0531a82de2814231dddea3ad710f60) )
	ROM_RELOAD(             0x10000, 0x10000 )
	ROM_LOAD( "a98__02.mbm27c512.12a", 0x20000, 0x10000, CRC(f2d17b0c) SHA1(418c8e383b8d4d54d723ae3512829a95e6897ee1) )
	ROM_RELOAD(             0x30000, 0x10000 )
	ROM_LOAD( "a98__03.mbm27c512.10a", 0x40000, 0x10000, CRC(1a519b0a) SHA1(9217c6bf564ccd4a44f9cf2045102e667dc0b036) )
	ROM_RELOAD(             0x40000, 0x10000 )
	ROM_LOAD( "a98__04.mbm27c512.8a", 0x60000, 0x10000, CRC(b64501a1) SHA1(6d96172b7d7d2276787013fe6b47bb7fef0a4e36) )
	ROM_RELOAD(             0x70000, 0x10000 )
	ROM_LOAD( "a98__05.mbm27c512.7a", 0x80000, 0x10000, CRC(45c36963) SHA1(2f23bff22e218f542c50bf7e4ae8ab6db93180b0) )
	ROM_RELOAD(             0x90000, 0x10000 )
	ROM_LOAD( "a98__06.mbm27c512.5a", 0xa0000, 0x10000, CRC(e075341b) SHA1(b5e68b5da7933c7eff21fa832e089edcbb49cdb4) )
	ROM_RELOAD(             0xb0000, 0x10000 )
	ROM_LOAD( "a98__07.mbm27c512.4a", 0xc0000, 0x10000, CRC(8e16cd81) SHA1(6bc9dc8e29197b75c3c4ac4f066037bb9b8cebb4) )
	ROM_RELOAD(             0xd0000, 0x10000 )
	ROM_LOAD( "a98__08.mbm27c512.2a", 0xe0000, 0x10000, CRC(bfa7609a) SHA1(0b9aa89b5954334f40dda1f14b1691852c74fc37) )
	ROM_RELOAD(             0xf0000, 0x10000 )

	ROM_REGION( 0x0400, "proms", 0 )        /* color PROMs */
	ROM_LOAD( "a98-13.15f", 0x0000, 0x200, CRC(7cde2da5) SHA1(0cccfc35fb716ebb4cffa85c75681f33ca80a56e) )   /* hi bytes, AM27S29 or compatible like MB7124 */
	ROM_LOAD( "a98-12.17f", 0x0200, 0x200, CRC(90dc9da7) SHA1(f719dead7f4597e5ee6f1103599505b98cb58299) )   /* lo bytes, AM27S29 or compatible like MB7124 */

	ROM_REGION( 0x10000, "pal", 0 ) /* PALs on plumppop are the same set as arkanoid2/extrmatn/drtoppel/chukataio/etc with the exception of d9? */
	ROM_LOAD( "b06-10-1.pal16l8a.d9.jed", 0x00000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-11.pal16l8a.d6.jed", 0x01000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-12.pal16l8a.c3.jed", 0x02000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-13.pal16l8a.c2.jed", 0x03000, 0x01000, NO_DUMP)
ROM_END

ROM_START( jpopnics )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 64k + bankswitch areas for the first CPU */
	ROM_LOAD( "u96cpu2", 0x00000, 0x20000, CRC(649e951c) SHA1(b26bb157da9fcf5d3eddbb637a4cb2cb1b0fedac) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "u124cpu1", 0x00000, 0x10000,  CRC(8453e8e4) SHA1(aac1bd501a15f79e3ed566c949504169b2aa762d) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "u94gfx", 0x00000, 0x10000, CRC(e49f2fdd) SHA1(6824c6520d0039c062f028e69cbfa7b3790ea756) )
	ROM_CONTINUE(       0x20000, 0x10000 )
	ROM_LOAD( "u93gfx", 0x40000, 0x10000, CRC(a7791b5b) SHA1(4abfe9b2612ed0d17f1282a60879cf1d0620ae4c) )
	ROM_CONTINUE(       0x60000, 0x10000 )
	ROM_LOAD( "u92gfx", 0x80000, 0x10000, CRC(b30caac7) SHA1(a434f67e1bec9848d9c3e184734d8cebee048176) )
	ROM_CONTINUE(       0xa0000, 0x10000 )
	ROM_LOAD( "u91gfx", 0xc0000, 0x10000, CRC(18ada5f2) SHA1(3307dd11e5cd0d0abe8b7751a5fbf54998558b34) )
	ROM_CONTINUE(       0xe0000, 0x10000 )
ROM_END

ROM_START( extrmatn )
	ROM_REGION( 0x20000, "maincpu", 0 )             /* Region 0 - main cpu */
	ROM_LOAD( "b06-05.11c", 0x00000, 0x10000, CRC(918e1fe3) SHA1(1aa69e7ae393f275d440b3d5bf817475e443045d) )
	ROM_LOAD( "b06-06.9c",  0x10000, 0x10000, CRC(8842e105) SHA1(68675e77801504c5f67f82fae42f55152ffadebe) )

	ROM_REGION( 0x10000, "sub", 0 )             /* Region 2 - sound cpu */
	ROM_LOAD( "b06-19.4e", 0x00000, 0x10000, CRC(8de43ed9) SHA1(53e6d8fa93889c38733d169e983f2caf1da71f43) )

	ROM_REGION( 0x10000, "mcu", 0 )    /* M-Chip (i8x42 internal ROM) */
	ROM_LOAD( "b06__14.1g", 0x0000, 0x0800, CRC(28907072) SHA1(21c7017af8a8ceb8e43d7e798f48518b136fd45c) ) /* Labeled B06-14 and under printed label "Taito M-001, 128P, 720100", is a mask 8042 */

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b06-01.13a", 0x00000, 0x20000, CRC(d2afbf7e) SHA1(28b4cf94798f049a9f8375464741dbef208d7290) )
	ROM_LOAD( "b06-02.10a", 0x20000, 0x20000, CRC(e0c2757a) SHA1(3c89044caa28b10b4d1bef1515881810c23d312a) )
	ROM_LOAD( "b06-03.7a",  0x40000, 0x20000, CRC(ee80ab9d) SHA1(f4e4833cadff7d856b5a8075a61d902427653e16) )
	ROM_LOAD( "b06-04.4a",  0x60000, 0x20000, CRC(3697ace4) SHA1(6d6e4e64147365bcfcf74a84eb7ae84dffedd304) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "b06-09.15f", 0x00000, 0x200, CRC(f388b361) SHA1(f00db6ad6994cfe9b7ad76e30b7049b11f8c16e4) )  /* hi bytes, AM27S29 or compatible like MB7124 */
	ROM_LOAD( "b06-08.17f", 0x00200, 0x200, CRC(10c9aac3) SHA1(09d6f791dea358e78099af7a370b00b8504ffc97) )  /* lo bytes, AM27S29 or compatible like MB7124 */

	ROM_REGION( 0x10000, "pal", 0 ) /* these are shared with several other games on this hardware */
	ROM_LOAD( "b06-10.pal16l8a.d9.jed", 0x00000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-11.pal16l8a.d6.jed", 0x01000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-12.pal16l8a.c3.jed", 0x02000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-13.pal16l8a.c2.jed", 0x03000, 0x01000, NO_DUMP)
ROM_END

ROM_START( extrmatnu )
	ROM_REGION( 0x20000, "maincpu", 0 )             /* Region 0 - main cpu */
	ROM_LOAD( "b06-20.11c", 0x00000, 0x10000, CRC(04e3fc1f) SHA1(b1cf2f79f43fa33d6175368c897f84ec6aa6e746) )
	ROM_LOAD( "b06-21.9c",  0x10000, 0x10000, CRC(1614d6a2) SHA1(f23d465af231ab5653c55748f686d8f25f52394b) )

	ROM_REGION( 0x10000, "sub", 0 )             /* Region 2 - sound cpu */
	ROM_LOAD( "b06-22.4e", 0x00000, 0x10000, CRC(744f2c84) SHA1(7565c1594c2a3bae1ae45afcbf93363fe2b12d58) )

	ROM_REGION( 0x10000, "mcu", 0 )    /* M-Chip (i8x42 internal ROM) */
	ROM_LOAD( "b06__14.1g", 0x0000, 0x0800, CRC(28907072) SHA1(21c7017af8a8ceb8e43d7e798f48518b136fd45c) ) /* Labeled B06-14 and under printed label "Taito M-001, 128P, 720100", is a mask 8042 */

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b06-01.13a", 0x00000, 0x20000, CRC(d2afbf7e) SHA1(28b4cf94798f049a9f8375464741dbef208d7290) )
	ROM_LOAD( "b06-02.10a", 0x20000, 0x20000, CRC(e0c2757a) SHA1(3c89044caa28b10b4d1bef1515881810c23d312a) )
	ROM_LOAD( "b06-03.7a",  0x40000, 0x20000, CRC(ee80ab9d) SHA1(f4e4833cadff7d856b5a8075a61d902427653e16) )
	ROM_LOAD( "b06-04.4a",  0x60000, 0x20000, CRC(3697ace4) SHA1(6d6e4e64147365bcfcf74a84eb7ae84dffedd304) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "b06-09.15f", 0x00000, 0x200, CRC(f388b361) SHA1(f00db6ad6994cfe9b7ad76e30b7049b11f8c16e4) )  /* hi bytes, AM27S29 or compatible like MB7124 */
	ROM_LOAD( "b06-08.17f", 0x00200, 0x200, CRC(10c9aac3) SHA1(09d6f791dea358e78099af7a370b00b8504ffc97) )  /* lo bytes, AM27S29 or compatible like MB7124 */

	ROM_REGION( 0x10000, "pal", 0 ) /* these are shared with several other games on this hardware */
	ROM_LOAD( "b06-10.pal16l8a.d9.jed", 0x00000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-11.pal16l8a.d6.jed", 0x01000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-12.pal16l8a.c3.jed", 0x02000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-13.pal16l8a.c2.jed", 0x03000, 0x01000, NO_DUMP)
ROM_END

ROM_START( extrmatnur )
	ROM_REGION( 0x20000, "maincpu", 0 )             /* Region 0 - main cpu */
	ROM_LOAD( "b06-15.11c",  0x00000, 0x10000, CRC(4b3ee597) SHA1(024964faebd0fa894ab4868a8e009267e828cbfb) )
	ROM_LOAD( "b06-16.9c",   0x10000, 0x10000, CRC(86175ea4) SHA1(0f30cbb1a6a32355528543707799f752a1b9b75e) )

	ROM_REGION( 0x10000, "sub", 0 )             /* Region 2 - sound cpu */
	ROM_LOAD( "b06-17.4e", 0x00000, 0x10000, CRC(744f2c84) SHA1(7565c1594c2a3bae1ae45afcbf93363fe2b12d58) )

	ROM_REGION( 0x10000, "mcu", 0 )    /* M-Chip (i8x42 internal ROM) */
	ROM_LOAD( "b06__14.1g", 0x0000, 0x0800, CRC(28907072) SHA1(21c7017af8a8ceb8e43d7e798f48518b136fd45c) ) /* Labeled B06-14 and under printed label "Taito M-001, 128P, 720100", is a mask 8042 */

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b06-01.13a", 0x00000, 0x20000, CRC(d2afbf7e) SHA1(28b4cf94798f049a9f8375464741dbef208d7290) )
	ROM_LOAD( "b06-02.10a", 0x20000, 0x20000, CRC(e0c2757a) SHA1(3c89044caa28b10b4d1bef1515881810c23d312a) )
	ROM_LOAD( "b06-03.7a",  0x40000, 0x20000, CRC(ee80ab9d) SHA1(f4e4833cadff7d856b5a8075a61d902427653e16) )
	ROM_LOAD( "b06-04.4a",  0x60000, 0x20000, CRC(3697ace4) SHA1(6d6e4e64147365bcfcf74a84eb7ae84dffedd304) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "b06-09.15f", 0x00000, 0x200, CRC(f388b361) SHA1(f00db6ad6994cfe9b7ad76e30b7049b11f8c16e4) )  /* hi bytes, AM27S29 or compatible like MB7124 */
	ROM_LOAD( "b06-08.17f", 0x00200, 0x200, CRC(10c9aac3) SHA1(09d6f791dea358e78099af7a370b00b8504ffc97) )  /* lo bytes, AM27S29 or compatible like MB7124 */

	ROM_REGION( 0x10000, "pal", 0 ) /* these are shared with several other games on this hardware */
	ROM_LOAD( "b06-10.pal16l8a.d9.jed", 0x00000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-11.pal16l8a.d6.jed", 0x01000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-12.pal16l8a.c3.jed", 0x02000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-13.pal16l8a.c2.jed", 0x03000, 0x01000, NO_DUMP)
ROM_END

ROM_START( extrmatnj )
	ROM_REGION( 0x20000, "maincpu", 0 )             /* Region 0 - main cpu */
	ROM_LOAD( "b06-05.11c", 0x00000, 0x10000, CRC(918e1fe3) SHA1(1aa69e7ae393f275d440b3d5bf817475e443045d) )
	ROM_LOAD( "b06-06.9c",  0x10000, 0x10000, CRC(8842e105) SHA1(68675e77801504c5f67f82fae42f55152ffadebe) )

	ROM_REGION( 0x10000, "sub", 0 )             /* Region 2 - sound cpu */
	ROM_LOAD( "b06-07.4e", 0x00000, 0x10000, CRC(b37fb8b3) SHA1(10696914b9e39d34d56069a69b9d641339ea2309) )

	ROM_REGION( 0x10000, "mcu", 0 )    /* M-Chip (i8x42 internal ROM) */
	ROM_LOAD( "b06__14.1g", 0x0000, 0x0800, CRC(28907072) SHA1(21c7017af8a8ceb8e43d7e798f48518b136fd45c) ) /* Labeled B06-14 and under printed label "Taito M-001, 128P, 720100", is a mask 8042 */

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b06-01.13a", 0x00000, 0x20000, CRC(d2afbf7e) SHA1(28b4cf94798f049a9f8375464741dbef208d7290) )
	ROM_LOAD( "b06-02.10a", 0x20000, 0x20000, CRC(e0c2757a) SHA1(3c89044caa28b10b4d1bef1515881810c23d312a) )
	ROM_LOAD( "b06-03.7a",  0x40000, 0x20000, CRC(ee80ab9d) SHA1(f4e4833cadff7d856b5a8075a61d902427653e16) )
	ROM_LOAD( "b06-04.4a",  0x60000, 0x20000, CRC(3697ace4) SHA1(6d6e4e64147365bcfcf74a84eb7ae84dffedd304) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "b06-09.15f", 0x00000, 0x200, CRC(f388b361) SHA1(f00db6ad6994cfe9b7ad76e30b7049b11f8c16e4) )  /* hi bytes, AM27S29 or compatible like MB7124 */
	ROM_LOAD( "b06-08.17f", 0x00200, 0x200, CRC(10c9aac3) SHA1(09d6f791dea358e78099af7a370b00b8504ffc97) )  /* lo bytes, AM27S29 or compatible like MB7124 */

	ROM_REGION( 0x10000, "pal", 0 ) /* these are shared with several other games on this hardware */
	ROM_LOAD( "b06-10.pal16l8a.d9.jed", 0x00000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-11.pal16l8a.d6.jed", 0x01000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-12.pal16l8a.c3.jed", 0x02000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-13.pal16l8a.c2.jed", 0x03000, 0x01000, NO_DUMP)
ROM_END

/*
PCB:
Seta: P0-022-A
Taito: K1100245A J1100108A

The arknoid2 PCB has a sticker label which says "K1100245A // REVENGE OF DOH"
*/

ROM_START( arknoid2 )
	ROM_REGION( 0x20000, "maincpu", 0 )             /* Region 0 - main cpu */
	ROM_LOAD( "b08__05.11c", 0x00000, 0x10000, CRC(136edf9d) SHA1(f632321650897eee585511a84f451a205d1f7704) )
	/* 0x10000 - 0x1ffff empty */

	ROM_REGION( 0x10000, "sub", 0 )             /* Region 2 - sound cpu */
	ROM_LOAD( "b08__13.3e", 0x00000, 0x10000, CRC(e8035ef1) SHA1(9a54e952cff0036c4b6affd9ffb1097cdccbe255) )

	ROM_REGION( 0x10000, "mcu", 0 )    /* M-Chip (i8x42 internal ROM) */
	ROM_LOAD( "b08__09.3g", 0x0000, 0x0800, NO_DUMP ) /* Labeled B08 // 09 and under printed label "?Taito M-009?", is a mask 8042 */

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b08-01.13a", 0x00000, 0x20000, CRC(2ccc86b4) SHA1(eced1d7e687db0331507726946b6a19a690a7604) )
	ROM_LOAD( "b08-02.10a", 0x20000, 0x20000, CRC(056a985f) SHA1(6333b71c631d3307929aae633760870451830e10) )
	ROM_LOAD( "b08-03.7a",  0x40000, 0x20000, CRC(274a795f) SHA1(49353590e1a418843f57c715185e407a20021936) )
	ROM_LOAD( "b08-04.4a",  0x60000, 0x20000, CRC(9754f703) SHA1(0018ebf7da3f501345f3f5085d98d7614f8ce1b6) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "b08-08.15f", 0x00000, 0x200, CRC(a4f7ebd9) SHA1(094eb63c18898c6ee8d722492bdfd28091c61773) )  /* hi bytes, AM27S29 or compatible like MB7124 */
	ROM_LOAD( "b08-07.16f", 0x00200, 0x200, CRC(ea34d9f7) SHA1(9a46edc64f961bd96908419cabd92445d300fc19) )  /* lo bytes, AM27S29 or compatible like MB7124 */

	ROM_REGION( 0x10000, "pal", 0 ) /* these are shared with extermination */
	ROM_LOAD( "b06-10.pal16l8a.d9.jed", 0x00000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-11.pal16l8a.d6.jed", 0x01000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-12.pal16l8a.c3.jed", 0x02000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-13.pal16l8a.c2.jed", 0x03000, 0x01000, NO_DUMP)
ROM_END

ROM_START( arknoid2u )
	ROM_REGION( 0x20000, "maincpu", 0 )             /* Region 0 - main cpu */
	ROM_LOAD( "b08__11.11c", 0x00000, 0x10000, CRC(99555231) SHA1(2798f3f5b3f1fa27598fe7a6e95c75d9142c8d34) )
	/* 0x10000 - 0x1ffff empty */

	ROM_REGION( 0x10000, "sub", 0 )             /* Region 2 - sound cpu */
	ROM_LOAD( "b08__12.3e", 0x00000, 0x10000, CRC(dc84e27d) SHA1(d549d8c9fbec0521517f0c5f5cee763e27d48633) )

	ROM_REGION( 0x10000, "mcu", 0 )    /* M-Chip (i8x42 internal ROM) */
	ROM_LOAD( "b08__09.3g", 0x0000, 0x0800, NO_DUMP ) /* Labeled B08 // 09 and under printed label "?Taito M-009?", is a mask 8042 */

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b08-01.13a", 0x00000, 0x20000, CRC(2ccc86b4) SHA1(eced1d7e687db0331507726946b6a19a690a7604) )
	ROM_LOAD( "b08-02.10a", 0x20000, 0x20000, CRC(056a985f) SHA1(6333b71c631d3307929aae633760870451830e10) )
	ROM_LOAD( "b08-03.7a",  0x40000, 0x20000, CRC(274a795f) SHA1(49353590e1a418843f57c715185e407a20021936) )
	ROM_LOAD( "b08-04.4a",  0x60000, 0x20000, CRC(9754f703) SHA1(0018ebf7da3f501345f3f5085d98d7614f8ce1b6) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "b08-08.15f", 0x00000, 0x200, CRC(a4f7ebd9) SHA1(094eb63c18898c6ee8d722492bdfd28091c61773) )  /* hi bytes, AM27S29 or compatible like MB7124 */
	ROM_LOAD( "b08-07.16f", 0x00200, 0x200, CRC(ea34d9f7) SHA1(9a46edc64f961bd96908419cabd92445d300fc19) )  /* lo bytes, AM27S29 or compatible like MB7124 */

	ROM_REGION( 0x10000, "pal", 0 ) /* these are shared with extermination */
	ROM_LOAD( "b06-10.pal16l8a.d9.jed", 0x00000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-11.pal16l8a.d6.jed", 0x01000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-12.pal16l8a.c3.jed", 0x02000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-13.pal16l8a.c2.jed", 0x03000, 0x01000, NO_DUMP)
ROM_END

ROM_START( arknoid2j )
	ROM_REGION( 0x20000, "maincpu", 0 )             /* Region 0 - main cpu */
	ROM_LOAD( "b08_05.11c", 0x00000, 0x10000, CRC(136edf9d) SHA1(f632321650897eee585511a84f451a205d1f7704) )
	/* 0x10000 - 0x1ffff empty */

	ROM_REGION( 0x10000, "sub", 0 )             /* Region 2 - sound cpu */
	ROM_LOAD( "b08_06.3e", 0x00000, 0x10000, CRC(adfcd40c) SHA1(f91299407ed21e2dd244c9b1a315b27ed32f5514) )

	ROM_REGION( 0x10000, "mcu", 0 )    /* M-Chip (i8x42 internal ROM) */
	ROM_LOAD( "b08__09.3g", 0x0000, 0x0800, NO_DUMP ) /* Labeled B08 // 09 and under printed label "?Taito M-009?", is a mask 8042 */

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b08-01.13a", 0x00000, 0x20000, CRC(2ccc86b4) SHA1(eced1d7e687db0331507726946b6a19a690a7604) )
	ROM_LOAD( "b08-02.10a", 0x20000, 0x20000, CRC(056a985f) SHA1(6333b71c631d3307929aae633760870451830e10) )
	ROM_LOAD( "b08-03.7a",  0x40000, 0x20000, CRC(274a795f) SHA1(49353590e1a418843f57c715185e407a20021936) )
	ROM_LOAD( "b08-04.4a",  0x60000, 0x20000, CRC(9754f703) SHA1(0018ebf7da3f501345f3f5085d98d7614f8ce1b6) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "b08-08.15f", 0x00000, 0x200, CRC(a4f7ebd9) SHA1(094eb63c18898c6ee8d722492bdfd28091c61773) )  /* hi bytes, AM27S29 or compatible like MB7124 */
	ROM_LOAD( "b08-07.16f", 0x00200, 0x200, CRC(ea34d9f7) SHA1(9a46edc64f961bd96908419cabd92445d300fc19) )  /* lo bytes, AM27S29 or compatible like MB7124 */

	ROM_REGION( 0x10000, "pal", 0 ) /* these are shared with extermination */
	ROM_LOAD( "b06-10.pal16l8a.d9.jed", 0x00000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-11.pal16l8a.d6.jed", 0x01000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-12.pal16l8a.c3.jed", 0x02000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-13.pal16l8a.c2.jed", 0x03000, 0x01000, NO_DUMP)
ROM_END

ROM_START( arknoid2b )
	ROM_REGION( 0x20000, "maincpu", 0 )             /* Region 0 - main cpu */
	ROM_LOAD( "boot.11c",  0x00000, 0x10000, CRC(3847dfb0) SHA1(993c8af3df7a4d5a2523f0e31a6df1c07ba13c7d) )
	/* 0x10000 - 0x1ffff empty */

	ROM_REGION( 0x10000, "sub", 0 )             /* Region 2 - sound cpu */
	ROM_LOAD( "b08_13.3e", 0x00000, 0x10000, CRC(e8035ef1) SHA1(9a54e952cff0036c4b6affd9ffb1097cdccbe255) )

	ROM_REGION( 0x10000, "mcu", 0 )    /* M-Chip (i8x42 internal ROM) */
	ROM_LOAD( "ark28742.3g", 0x0000, 0x0800, NO_DUMP ) /* Labeled B08 // 09 and under printed label "?Taito M-009?", is a mask 8042... does the bootleg set even HAVE the mcu? */

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b08-01.13a", 0x00000, 0x20000, CRC(2ccc86b4) SHA1(eced1d7e687db0331507726946b6a19a690a7604) )
	ROM_LOAD( "b08-02.10a", 0x20000, 0x20000, CRC(056a985f) SHA1(6333b71c631d3307929aae633760870451830e10) )
	ROM_LOAD( "b08-03.7a",  0x40000, 0x20000, CRC(274a795f) SHA1(49353590e1a418843f57c715185e407a20021936) )
	ROM_LOAD( "b08-04.4a",  0x60000, 0x20000, CRC(9754f703) SHA1(0018ebf7da3f501345f3f5085d98d7614f8ce1b6) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "b08-08.15f", 0x00000, 0x200, CRC(a4f7ebd9) SHA1(094eb63c18898c6ee8d722492bdfd28091c61773) )  /* hi bytes, AM27S29 or compatible like MB7124 */
	ROM_LOAD( "b08-07.16f", 0x00200, 0x200, CRC(ea34d9f7) SHA1(9a46edc64f961bd96908419cabd92445d300fc19) )  /* lo bytes, AM27S29 or compatible like MB7124 */

	ROM_REGION( 0x10000, "pal", 0 ) /* these are shared with extermination */
	ROM_LOAD( "b06-10.pal16l8a.d9.jed", 0x00000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-11.pal16l8a.d6.jed", 0x01000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-12.pal16l8a.c3.jed", 0x02000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-13.pal16l8a.c2.jed", 0x03000, 0x01000, NO_DUMP)
ROM_END

/*
PCB:
Seta: P0-025-A
Taito: K1100241A J1100107A

The drtoppelj PCB has a sticker label which says "K1100269A // DR. トッペル タンケン" (To'PeRu TaNKeN)
*/
ROM_START( drtoppel )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 64k + bankswitch areas for the first CPU */
	ROM_LOAD( "b19__09.11c", 0x00000, 0x10000, CRC(3e654f82) SHA1(d9e351d82546b08eb7887ea1d976fa97a259db6e) )
	ROM_LOAD( "b19__10.9c",  0x10000, 0x10000, CRC(7e72fd25) SHA1(6035e4db75e6dc57b13bb6e92217d1c2d0ffdfd2) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "b19__15.3e", 0x00000, 0x10000, BAD_DUMP CRC(37a0d3fb) SHA1(f65fb9382af5f5b09725c39b660c5138b3912f53) ) /* Region-Hacked??, need correct Taito ROM number */

	ROM_REGION( 0x10000, "mcu", 0 )    /* M-Chip (i8x42 internal ROM) */
	ROM_LOAD( "b06__14.1g", 0x0000, 0x0800, CRC(28907072) SHA1(21c7017af8a8ceb8e43d7e798f48518b136fd45c) ) /* Labeled B06 // 14 and under printed label "Taito M-001, 128P, 720100", is a mask 8042 */

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "b19-01.23c1000.13a", 0x00000, 0x20000, CRC(a7e8a0c1) SHA1(a2f017ae5b6472d4202f126d0247b3fe4b1321d1) )
	ROM_LOAD( "b19-02.23c1000.12a", 0x20000, 0x20000, CRC(790ae654) SHA1(5fd6b89918e1539e00c918959b96d2a9394c8abe) )
	ROM_LOAD( "b19-03.23c1000.10a", 0x40000, 0x20000, CRC(495c4c5a) SHA1(a23b512cda4c0c535df5508a52faebe401c1797d) )
	ROM_LOAD( "b19-04.23c1000.8a",  0x60000, 0x20000, CRC(647007a0) SHA1(10ec35a15091967038bb26fb116c47d730f69edc) )
	ROM_LOAD( "b19-05.23c1000.7a",  0x80000, 0x20000, CRC(49f2b1a5) SHA1(5e98bb421afaa02471ad02213ea6ca23ff2f0e27) )
	ROM_LOAD( "b19-06.23c1000.5a",  0xa0000, 0x20000, CRC(2d39f1d0) SHA1(2aa89a5cc7f026c8db9922b183319ff66ac4a071) )
	ROM_LOAD( "b19-07.23c1000.4a",  0xc0000, 0x20000, CRC(8bb06f41) SHA1(a0c182d473317f2cdb31bdf39a2593c032002305) )
	ROM_LOAD( "b19-08.23c1000.2a",  0xe0000, 0x20000, CRC(3584b491) SHA1(d0aca90708be241bbd3a1097220a85083337a4bc) )

	ROM_REGION( 0x0400, "proms", 0 )        /* color PROMs */
	ROM_LOAD( "b19-13.am27s29.15f", 0x0000, 0x200, CRC(6a547980) SHA1(c82f8dfad028565b4b4e5be1167f2f290c929090) )   /* hi bytes, AM27S29 or compatible like MB7124 */
	ROM_LOAD( "b19-12.am27s29.16f", 0x0200, 0x200, CRC(5754e9d8) SHA1(8c7d29e22c90b1f72929b95675dc15e431aae044) )   /* lo bytes, AM27S29 or compatible like MB7124 */

	ROM_REGION( 0x10000, "pal", 0 ) /* these are shared with extermination */
	ROM_LOAD( "b06-10.pal16l8a.d9.jed", 0x00000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-11.pal16l8a.d6.jed", 0x01000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-12.pal16l8a.c3.jed", 0x02000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-13.pal16l8a.c2.jed", 0x03000, 0x01000, NO_DUMP)
ROM_END

ROM_START( drtoppelu )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 64k + bankswitch areas for the first CPU */
	ROM_LOAD( "b19__09.11c", 0x00000, 0x10000, CRC(3e654f82) SHA1(d9e351d82546b08eb7887ea1d976fa97a259db6e) )
	ROM_LOAD( "b19__10.9c",  0x10000, 0x10000, CRC(7e72fd25) SHA1(6035e4db75e6dc57b13bb6e92217d1c2d0ffdfd2) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "b19__14.3e", 0x00000, 0x10000, BAD_DUMP CRC(05565b22) SHA1(d1aa47b438d3b44c5177337809e38b50f6445c36) ) /* Region-Hacked??, need correct Taito ROM number */

	ROM_REGION( 0x10000, "mcu", 0 )    /* M-Chip (i8x42 internal ROM) */
	ROM_LOAD( "b06__14.1g", 0x0000, 0x0800, CRC(28907072) SHA1(21c7017af8a8ceb8e43d7e798f48518b136fd45c) ) /* Labeled B06 // 14 and under printed label "Taito M-001, 128P, 720100", is a mask 8042 */

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "b19-01.23c1000.13a", 0x00000, 0x20000, CRC(a7e8a0c1) SHA1(a2f017ae5b6472d4202f126d0247b3fe4b1321d1) )
	ROM_LOAD( "b19-02.23c1000.12a", 0x20000, 0x20000, CRC(790ae654) SHA1(5fd6b89918e1539e00c918959b96d2a9394c8abe) )
	ROM_LOAD( "b19-03.23c1000.10a", 0x40000, 0x20000, CRC(495c4c5a) SHA1(a23b512cda4c0c535df5508a52faebe401c1797d) )
	ROM_LOAD( "b19-04.23c1000.8a",  0x60000, 0x20000, CRC(647007a0) SHA1(10ec35a15091967038bb26fb116c47d730f69edc) )
	ROM_LOAD( "b19-05.23c1000.7a",  0x80000, 0x20000, CRC(49f2b1a5) SHA1(5e98bb421afaa02471ad02213ea6ca23ff2f0e27) )
	ROM_LOAD( "b19-06.23c1000.5a",  0xa0000, 0x20000, CRC(2d39f1d0) SHA1(2aa89a5cc7f026c8db9922b183319ff66ac4a071) )
	ROM_LOAD( "b19-07.23c1000.4a",  0xc0000, 0x20000, CRC(8bb06f41) SHA1(a0c182d473317f2cdb31bdf39a2593c032002305) )
	ROM_LOAD( "b19-08.23c1000.2a",  0xe0000, 0x20000, CRC(3584b491) SHA1(d0aca90708be241bbd3a1097220a85083337a4bc) )

	ROM_REGION( 0x0400, "proms", 0 )        /* color PROMs */
	ROM_LOAD( "b19-13.am27s29.15f", 0x0000, 0x200, CRC(6a547980) SHA1(c82f8dfad028565b4b4e5be1167f2f290c929090) )   /* hi bytes, AM27S29 or compatible like MB7124 */
	ROM_LOAD( "b19-12.am27s29.16f", 0x0200, 0x200, CRC(5754e9d8) SHA1(8c7d29e22c90b1f72929b95675dc15e431aae044) )   /* lo bytes, AM27S29 or compatible like MB7124 */

	ROM_REGION( 0x10000, "pal", 0 ) /* these are shared with extermination */
	ROM_LOAD( "b06-10.pal16l8a.d9.jed", 0x00000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-11.pal16l8a.d6.jed", 0x01000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-12.pal16l8a.c3.jed", 0x02000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-13.pal16l8a.c2.jed", 0x03000, 0x01000, NO_DUMP)
ROM_END

ROM_START( drtoppelj )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 64k + bankswitch areas for the first CPU */
	ROM_LOAD( "b19__09.11c", 0x00000, 0x10000, CRC(3e654f82) SHA1(d9e351d82546b08eb7887ea1d976fa97a259db6e) )
	ROM_LOAD( "b19__10.9c",  0x10000, 0x10000, CRC(7e72fd25) SHA1(6035e4db75e6dc57b13bb6e92217d1c2d0ffdfd2) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "b19__11.3e", 0x00000, 0x10000, CRC(524dc249) SHA1(158b2de0fcd17ad16ba72bb24888122bf704e216) )

	ROM_REGION( 0x10000, "mcu", 0 )    /* M-Chip (i8x42 internal ROM) */
	ROM_LOAD( "b06__14.1g", 0x0000, 0x0800, CRC(28907072) SHA1(21c7017af8a8ceb8e43d7e798f48518b136fd45c) ) /* Labeled B06 // 14 and under printed label "Taito M-001, 128P, 720100", is a mask 8042 */

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "b19-01.23c1000.13a", 0x00000, 0x20000, CRC(a7e8a0c1) SHA1(a2f017ae5b6472d4202f126d0247b3fe4b1321d1) )
	ROM_LOAD( "b19-02.23c1000.12a", 0x20000, 0x20000, CRC(790ae654) SHA1(5fd6b89918e1539e00c918959b96d2a9394c8abe) )
	ROM_LOAD( "b19-03.23c1000.10a", 0x40000, 0x20000, CRC(495c4c5a) SHA1(a23b512cda4c0c535df5508a52faebe401c1797d) )
	ROM_LOAD( "b19-04.23c1000.8a",  0x60000, 0x20000, CRC(647007a0) SHA1(10ec35a15091967038bb26fb116c47d730f69edc) )
	ROM_LOAD( "b19-05.23c1000.7a",  0x80000, 0x20000, CRC(49f2b1a5) SHA1(5e98bb421afaa02471ad02213ea6ca23ff2f0e27) )
	ROM_LOAD( "b19-06.23c1000.5a",  0xa0000, 0x20000, CRC(2d39f1d0) SHA1(2aa89a5cc7f026c8db9922b183319ff66ac4a071) )
	ROM_LOAD( "b19-07.23c1000.4a",  0xc0000, 0x20000, CRC(8bb06f41) SHA1(a0c182d473317f2cdb31bdf39a2593c032002305) )
	ROM_LOAD( "b19-08.23c1000.2a",  0xe0000, 0x20000, CRC(3584b491) SHA1(d0aca90708be241bbd3a1097220a85083337a4bc) )

	ROM_REGION( 0x0400, "proms", 0 )        /* color PROMs */
	ROM_LOAD( "b19-13.am27s29.15f", 0x0000, 0x200, CRC(6a547980) SHA1(c82f8dfad028565b4b4e5be1167f2f290c929090) )   /* hi bytes, AM27S29 or compatible like MB7124 */
	ROM_LOAD( "b19-12.am27s29.16f", 0x0200, 0x200, CRC(5754e9d8) SHA1(8c7d29e22c90b1f72929b95675dc15e431aae044) )   /* lo bytes, AM27S29 or compatible like MB7124 */

	ROM_REGION( 0x10000, "pal", 0 ) /* these are shared with extermination */
	ROM_LOAD( "b06-10.pal16l8a.d9.jed", 0x00000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-11.pal16l8a.d6.jed", 0x01000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-12.pal16l8a.c3.jed", 0x02000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-13.pal16l8a.c2.jed", 0x03000, 0x01000, NO_DUMP)
ROM_END

ROM_START( kageki )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b35-13.bin", 0x00000, 0x10000, CRC(dc4b025f) SHA1(ed7e0d846693abe0a0ac198e23b272f84b30af46) )    /* World ver */
	ROM_LOAD( "b35-10.9c",  0x10000, 0x10000, CRC(b150457d) SHA1(a58e46e7dfdc93c2cc7c04d623d7754f85ba693b) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "b35-14.bin", 0x00000, 0x10000, CRC(8adef2d0) SHA1(0dc8206b35e898b8fed5cdccbdcc5ff1bad68da4) )    /* World ver */

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "b35__01.13a",  0x00000, 0x20000, CRC(01d83a69) SHA1(92a84329306b58a45f7bb443a8642eeaeb04d553) )
	ROM_LOAD( "b35__02.12a",  0x20000, 0x20000, CRC(d8af47ac) SHA1(2ef9ca991bf55ed6c12bf3a7dc4aa904d7749d5c) )
	ROM_LOAD( "b35__03.10a",  0x40000, 0x20000, CRC(3cb68797) SHA1(e7669b1a9a26dede560cc87695004d29510bc1f5) )
	ROM_LOAD( "b35__04.8a",   0x60000, 0x20000, CRC(71c03f91) SHA1(edce6e5a52b0c83c1c3c6bf9bc6b7957f7941521) )
	ROM_LOAD( "b35__05.7a",   0x80000, 0x20000, CRC(a4e20c08) SHA1(5d1d23d1410fea8650b18c595b0170a17e5d89a6) )
	ROM_LOAD( "b35__06.5a",   0xa0000, 0x20000, CRC(3f8ab658) SHA1(44de7ee2bdb89bc520ed9bc812c26789c3f31411) )
	ROM_LOAD( "b35__07.4a",   0xc0000, 0x20000, CRC(1b4af049) SHA1(09783816d5076219d241538e2711402eb8c4cd03) )
	ROM_LOAD( "b35__08.2a",   0xe0000, 0x20000, CRC(deb2268c) SHA1(318bf3da6cbe20758397d5f78caf3cda02f322d7) )

	ROM_REGION( 0x10000, "samples", 0 ) /* samples */
	ROM_LOAD( "b35-15.98g",  0x00000, 0x10000, CRC(e6212a0f) SHA1(43891f4fd141b00ed458be47a107a2550a0534c2) )

	ROM_REGION( 0x10000, "pal", 0 ) /* these are shared with extermination except d9 */
	ROM_LOAD( "b06-101.pal16l8a.d9.jed", 0x00000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-11.pal16l8a.d6.jed", 0x01000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-12.pal16l8a.c3.jed", 0x02000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-13.pal16l8a.c2.jed", 0x03000, 0x01000, NO_DUMP)
ROM_END

/* M6100309A PCB
   P0-038A */
ROM_START( kagekiu )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b35-16.11c", 0x00000, 0x10000, CRC(a4e6fd58) SHA1(7cfe5b3fa6c88cdab45719f5b58541270825ad30) )    /* US ver */
	ROM_LOAD( "b35-10.9c",  0x10000, 0x10000, CRC(b150457d) SHA1(a58e46e7dfdc93c2cc7c04d623d7754f85ba693b) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "b35-17.43e", 0x00000, 0x10000, CRC(fdd9c246) SHA1(ac7a59ed19d0d81748cabd8b77a6ba3937e3cc99) )    /* US ver */

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "b35__01.13a",  0x00000, 0x20000, CRC(01d83a69) SHA1(92a84329306b58a45f7bb443a8642eeaeb04d553) )
	ROM_LOAD( "b35__02.12a",  0x20000, 0x20000, CRC(d8af47ac) SHA1(2ef9ca991bf55ed6c12bf3a7dc4aa904d7749d5c) )
	ROM_LOAD( "b35__03.10a",  0x40000, 0x20000, CRC(3cb68797) SHA1(e7669b1a9a26dede560cc87695004d29510bc1f5) )
	ROM_LOAD( "b35__04.8a",   0x60000, 0x20000, CRC(71c03f91) SHA1(edce6e5a52b0c83c1c3c6bf9bc6b7957f7941521) )
	ROM_LOAD( "b35__05.7a",   0x80000, 0x20000, CRC(a4e20c08) SHA1(5d1d23d1410fea8650b18c595b0170a17e5d89a6) )
	ROM_LOAD( "b35__06.5a",   0xa0000, 0x20000, CRC(3f8ab658) SHA1(44de7ee2bdb89bc520ed9bc812c26789c3f31411) )
	ROM_LOAD( "b35__07.4a",   0xc0000, 0x20000, CRC(1b4af049) SHA1(09783816d5076219d241538e2711402eb8c4cd03) )
	ROM_LOAD( "b35__08.2a",   0xe0000, 0x20000, CRC(deb2268c) SHA1(318bf3da6cbe20758397d5f78caf3cda02f322d7) )

	ROM_REGION( 0x10000, "samples", 0 ) /* samples */
	ROM_LOAD( "b35-15.98g",  0x00000, 0x10000, CRC(e6212a0f) SHA1(43891f4fd141b00ed458be47a107a2550a0534c2) )   /* matches World ver */

	ROM_REGION( 0x10000, "pal", 0 ) /* these are shared with extermination except d9 */
	ROM_LOAD( "b06-101.pal16l8a.d9.jed", 0x00000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-11.pal16l8a.d6.jed", 0x01000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-12.pal16l8a.c3.jed", 0x02000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-13.pal16l8a.c2.jed", 0x03000, 0x01000, NO_DUMP)
ROM_END

ROM_START( kagekij )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b35-09.11c", 0x00000, 0x10000, CRC(829637d5) SHA1(0239ae925968336a90cbe16e23519773b6f2f2ac) )    /* JP ver */
	ROM_LOAD( "b35-10.9c",  0x10000, 0x10000, CRC(b150457d) SHA1(a58e46e7dfdc93c2cc7c04d623d7754f85ba693b) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "b35-11.43e", 0x00000, 0x10000, CRC(64d093fc) SHA1(3ca3f69d8946c453c0edb8586b92e2948a2d0b6c) )    /* JP ver */

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "b35-01.13a",  0x00000, 0x20000, CRC(01d83a69) SHA1(92a84329306b58a45f7bb443a8642eeaeb04d553) )
	ROM_LOAD( "b35-02.12a",  0x20000, 0x20000, CRC(d8af47ac) SHA1(2ef9ca991bf55ed6c12bf3a7dc4aa904d7749d5c) )
	ROM_LOAD( "b35-03.10a",  0x40000, 0x20000, CRC(3cb68797) SHA1(e7669b1a9a26dede560cc87695004d29510bc1f5) )
	ROM_LOAD( "b35-04.8a",   0x60000, 0x20000, CRC(71c03f91) SHA1(edce6e5a52b0c83c1c3c6bf9bc6b7957f7941521) )
	ROM_LOAD( "b35-05.7a",   0x80000, 0x20000, CRC(a4e20c08) SHA1(5d1d23d1410fea8650b18c595b0170a17e5d89a6) )
	ROM_LOAD( "b35-06.5a",   0xa0000, 0x20000, CRC(3f8ab658) SHA1(44de7ee2bdb89bc520ed9bc812c26789c3f31411) )
	ROM_LOAD( "b35-07.4a",   0xc0000, 0x20000, CRC(1b4af049) SHA1(09783816d5076219d241538e2711402eb8c4cd03) )
	ROM_LOAD( "b35-08.2a",   0xe0000, 0x20000, CRC(deb2268c) SHA1(318bf3da6cbe20758397d5f78caf3cda02f322d7) )

	ROM_REGION( 0x10000, "samples", 0 ) /* samples */
	ROM_LOAD( "b35-12.98g", 0x00000, 0x10000, CRC(184409f1) SHA1(711bdd499670e86630ebb6820262b1d8d651c987) )    /* JP ver */

	ROM_REGION( 0x10000, "pal", 0 ) /* these are shared with extermination except d9 */
	ROM_LOAD( "b06-101.pal16l8a.d9.jed", 0x00000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-11.pal16l8a.d6.jed", 0x01000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-12.pal16l8a.c3.jed", 0x02000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-13.pal16l8a.c2.jed", 0x03000, 0x01000, NO_DUMP)
ROM_END

/* Board ID is M6100309A - program ROM has been hacked to say 1992 :/
    supported because it appears to be a different code revision to the other supported sets
*/

ROM_START( kagekih )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b35_16.11c", 0x00000, 0x10000, CRC(1cf67603) SHA1(0627285ac69e44312d7694c64b96a81489d8663c) )    /* hacked ver of the World set */
	ROM_LOAD( "b35-10.9c",  0x10000, 0x10000, CRC(b150457d) SHA1(a58e46e7dfdc93c2cc7c04d623d7754f85ba693b) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "b35-11.43e", 0x00000, 0x10000, CRC(64d093fc) SHA1(3ca3f69d8946c453c0edb8586b92e2948a2d0b6c) )    /* JP ver */

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "b35-01.13a",  0x00000, 0x20000, CRC(01d83a69) SHA1(92a84329306b58a45f7bb443a8642eeaeb04d553) )
	ROM_LOAD( "b35-02.12a",  0x20000, 0x20000, CRC(d8af47ac) SHA1(2ef9ca991bf55ed6c12bf3a7dc4aa904d7749d5c) )
	ROM_LOAD( "b35-03.10a",  0x40000, 0x20000, CRC(3cb68797) SHA1(e7669b1a9a26dede560cc87695004d29510bc1f5) )
	ROM_LOAD( "b35-04.8a",   0x60000, 0x20000, CRC(71c03f91) SHA1(edce6e5a52b0c83c1c3c6bf9bc6b7957f7941521) )
	ROM_LOAD( "b35-05.7a",   0x80000, 0x20000, CRC(a4e20c08) SHA1(5d1d23d1410fea8650b18c595b0170a17e5d89a6) )
	ROM_LOAD( "b35-06.5a",   0xa0000, 0x20000, CRC(3f8ab658) SHA1(44de7ee2bdb89bc520ed9bc812c26789c3f31411) )
	ROM_LOAD( "b35-07.4a",   0xc0000, 0x20000, CRC(1b4af049) SHA1(09783816d5076219d241538e2711402eb8c4cd03) )
	ROM_LOAD( "b35-08.2a",   0xe0000, 0x20000, CRC(deb2268c) SHA1(318bf3da6cbe20758397d5f78caf3cda02f322d7) )

	ROM_REGION( 0x10000, "samples", 0 ) /* samples */
	ROM_LOAD( "b35-12.98g", 0x00000, 0x10000, CRC(184409f1) SHA1(711bdd499670e86630ebb6820262b1d8d651c987) )    /* JP ver */

	ROM_REGION( 0x10000, "pal", 0 ) /* these are shared with extermination except d9 */
	ROM_LOAD( "b06-101.pal16l8a.d9.jed", 0x00000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-11.pal16l8a.d6.jed", 0x01000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-12.pal16l8a.c3.jed", 0x02000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-13.pal16l8a.c2.jed", 0x03000, 0x01000, NO_DUMP)
ROM_END


/*
Chuka Taisen, Taito, 1988
Hardware info by Guru

 Seta: P0-028-A
Taito: K1100416A J1100332A
|--------------------------------------------------|
|    SETA                     SETA         12MHz   |
|    X1-003     6116          X1-001               |
|               6116          YM3906               |
|    SETA       6116                               |
|    X1-006     6116                               |
|                                                  |
|                             SETA                 |
|                             X1-002A              |
|                                                  |
|       DSWB                                       |
|J                                   B44-01.8      |
|A                      PRG10.32                   |
|M      DSWA            PRG11.31     B44-05.7      |
|M                      6264                       |
|A                                   B44-02.6      |
|     YM3014                                       |
|                                    B44-06.5      |
|             B06-101.36(PAL)                      |
|                                    B44-03.4      |
|       YM2203   Z80B                              |
|                    B06-11.35(PAL)  B44-07.3      |
|                B44_12.38                         |
|   SETA                             B44-04.2      |
|   X1-004         Z80B                            |
|                    B06-12.26(PAL)  B44-08.1      |
|                    B06-11.25(PAL)                |
|--------------------------------------------------|
Notes:
      6264: 8K x8 SRAM
      6116: 2K x8 SRAM
      Graphics ROMs are 23C1000/TC531000 mask ROMs


Chuka Taisen 'later' version:
   Seta: PO-025-A
  Taito: K1100241A J1100107A
Sticker: K1100364A CHUKA TAISEN

Technically newer ROM ID#s but was used to get rid of old pcb stock.
This set, unlike the PO-028-A PCB versions, uses two color PROMs.


************************************************
Guru-Readme for EPROM version of Chuuka Taisen
------------------------------------------------

Main Board (same as Extermination/Dr Toppel etc)
----------
PO-025-A
J1100107A K1100241A
Sticker: K1100364A CHUUKA TAISEN
Sticker: M4300092A CHUUKA TAISEN

All EPROMs are NEC D27C512D-15
M-Chip labelled "B06 14" at location 3G

PALs (all MMI PAL16L8A-2CN): B06-13.2C, B06-12.3C, B06-11.6D, B06-10.8D
PALs are protected

PROMs are AM27S29


ROM Board
---------
K9100189A J9100141A ROM7PCB
Sticker: K9100189A CHUUKA TAISEN

All EPROMs are Toshiba TC571000D-20

************************************************

*/

ROM_START( chukatai )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 64k + bankswitch areas for the first CPU */
	ROM_LOAD( "b44-10", 0x00000, 0x10000, CRC(8c69e008) SHA1(7825965f517f3562a508345b7c0d32b8a57bd38a) ) /* P0-028-A PCB set */
	ROM_LOAD( "b44-11", 0x10000, 0x10000, CRC(32484094) SHA1(f320fea2910816b5085ca9aa37e30af665fb6be1) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "b44-12w", 0x00000, 0x10000, CRC(e80ecdca) SHA1(cd96403ca97f18f630118dcb3dc2179c01147213) ) /* Hacked??, need correct Taito ROM number */

	ROM_REGION( 0x10000, "mcu", 0 ) /* M-Chip (i8x42 internal ROM) */
	ROM_LOAD( "b44-8742.mcu", 0x0000, 0x0800, CRC(7dff3f9f) SHA1(bbf4e036d025fe8179b053d639f9b8ad401e6e68) ) /* B44 // 09 is the label? what is the mask number under the label? maybe Taito M-011? last digit is definitely 1 */

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "b44-01.a13", 0x00000, 0x20000, CRC(aae7b3d5) SHA1(52809ea22d98811ece2fb27e80db6ddf4fbacb07) )
	ROM_LOAD( "b44-02.a12", 0x20000, 0x20000, CRC(7f0b9568) SHA1(415d2638d1b0eb36b2e2f63219cbc0dbebe02dc6) )
	ROM_LOAD( "b44-03.a10", 0x40000, 0x20000, CRC(5a54a3b9) SHA1(6b219f1c3570f16eb4a06221d7e527c735437bac) )
	ROM_LOAD( "b44-04.a08", 0x60000, 0x20000, CRC(3c5f544b) SHA1(d3b0ee18f1027483a36ef02757b62f42a086a8e2) )
	ROM_LOAD( "b44-05.a07", 0x80000, 0x20000, CRC(d1b7e314) SHA1(8b4181caa32955b4274614a4238bb24d67ecb729) )
	ROM_LOAD( "b44-06.a05", 0xa0000, 0x20000, CRC(269978a8) SHA1(aef7b8d3d00dcc4201e0a1e28026f6f1bdafd0b7) )
	ROM_LOAD( "b44-07.a04", 0xc0000, 0x20000, CRC(3e0e737e) SHA1(f8d62c7b69c79da9df7ef5ce454060d3645e5884) )
	ROM_LOAD( "b44-08.a02", 0xe0000, 0x20000, CRC(6cb1e8fc) SHA1(4ab0c2cce1de2616044a9bfb9bf17f95a49baffd) )

	ROM_REGION( 0x10000, "pal", 0 ) /* these are shared with extermination except d9 */
	ROM_LOAD( "b06-101.pal16l8a.d9.jed", 0x00000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-11.pal16l8a.d6.jed", 0x01000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-12.pal16l8a.c3.jed", 0x02000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-13.pal16l8a.c2.jed", 0x03000, 0x01000, NO_DUMP)
ROM_END

ROM_START( chukataiu )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 64k + bankswitch areas for the first CPU */
	ROM_LOAD( "b44-10", 0x00000, 0x10000, CRC(8c69e008) SHA1(7825965f517f3562a508345b7c0d32b8a57bd38a) ) /* P0-028-A PCB set */
	ROM_LOAD( "b44-11", 0x10000, 0x10000, CRC(32484094) SHA1(f320fea2910816b5085ca9aa37e30af665fb6be1) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "b44-12u", 0x00000, 0x10000, BAD_DUMP CRC(9f09fd5c) SHA1(ae92f2e893e1e666dcabbd793f1a778c5e3d7bab) ) /* Hacked??, need correct Taito ROM number */

	ROM_REGION( 0x1000, "mcu", 0 )  /* M-Chip (i8x42 internal ROM) */
	ROM_LOAD( "b44-8742.mcu", 0x0000, 0x0800, CRC(7dff3f9f) SHA1(bbf4e036d025fe8179b053d639f9b8ad401e6e68) ) /* B44 // 09 is the label? what is the mask number under the label? maybe Taito M-011? last digit is definitely 1 */

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "b44-01.a13", 0x00000, 0x20000, CRC(aae7b3d5) SHA1(52809ea22d98811ece2fb27e80db6ddf4fbacb07) )
	ROM_LOAD( "b44-02.a12", 0x20000, 0x20000, CRC(7f0b9568) SHA1(415d2638d1b0eb36b2e2f63219cbc0dbebe02dc6) )
	ROM_LOAD( "b44-03.a10", 0x40000, 0x20000, CRC(5a54a3b9) SHA1(6b219f1c3570f16eb4a06221d7e527c735437bac) )
	ROM_LOAD( "b44-04.a08", 0x60000, 0x20000, CRC(3c5f544b) SHA1(d3b0ee18f1027483a36ef02757b62f42a086a8e2) )
	ROM_LOAD( "b44-05.a07", 0x80000, 0x20000, CRC(d1b7e314) SHA1(8b4181caa32955b4274614a4238bb24d67ecb729) )
	ROM_LOAD( "b44-06.a05", 0xa0000, 0x20000, CRC(269978a8) SHA1(aef7b8d3d00dcc4201e0a1e28026f6f1bdafd0b7) )
	ROM_LOAD( "b44-07.a04", 0xc0000, 0x20000, CRC(3e0e737e) SHA1(f8d62c7b69c79da9df7ef5ce454060d3645e5884) )
	ROM_LOAD( "b44-08.a02", 0xe0000, 0x20000, CRC(6cb1e8fc) SHA1(4ab0c2cce1de2616044a9bfb9bf17f95a49baffd) )

	ROM_REGION( 0x10000, "pal", 0 ) /* these are shared with extermination except d9 */
	ROM_LOAD( "b06-101.pal16l8a.d9.jed", 0x00000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-11.pal16l8a.d6.jed", 0x01000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-12.pal16l8a.c3.jed", 0x02000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-13.pal16l8a.c2.jed", 0x03000, 0x01000, NO_DUMP)
ROM_END

ROM_START( chukataij )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 64k + bankswitch areas for the first CPU */
	ROM_LOAD( "b44-10", 0x00000, 0x10000, CRC(8c69e008) SHA1(7825965f517f3562a508345b7c0d32b8a57bd38a) ) /* P0-028-A PCB set */
	ROM_LOAD( "b44-11", 0x10000, 0x10000, CRC(32484094) SHA1(f320fea2910816b5085ca9aa37e30af665fb6be1) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "b44-12", 0x00000, 0x10000, CRC(0600ace6) SHA1(3d5767b91ea63128bfbff3527ddcf90fcf43af2e) )

	ROM_REGION( 0x10000, "mcu", 0 ) /* M-Chip (i8x42 internal ROM) */
	ROM_LOAD( "b44-8742.mcu", 0x0000, 0x0800, CRC(7dff3f9f) SHA1(bbf4e036d025fe8179b053d639f9b8ad401e6e68) ) /* B44 // 09 is the label? what is the mask number under the label? maybe Taito M-011? last digit is definitely 1 */

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "b44-01.a13", 0x00000, 0x20000, CRC(aae7b3d5) SHA1(52809ea22d98811ece2fb27e80db6ddf4fbacb07) )
	ROM_LOAD( "b44-02.a12", 0x20000, 0x20000, CRC(7f0b9568) SHA1(415d2638d1b0eb36b2e2f63219cbc0dbebe02dc6) )
	ROM_LOAD( "b44-03.a10", 0x40000, 0x20000, CRC(5a54a3b9) SHA1(6b219f1c3570f16eb4a06221d7e527c735437bac) )
	ROM_LOAD( "b44-04.a08", 0x60000, 0x20000, CRC(3c5f544b) SHA1(d3b0ee18f1027483a36ef02757b62f42a086a8e2) )
	ROM_LOAD( "b44-05.a07", 0x80000, 0x20000, CRC(d1b7e314) SHA1(8b4181caa32955b4274614a4238bb24d67ecb729) )
	ROM_LOAD( "b44-06.a05", 0xa0000, 0x20000, CRC(269978a8) SHA1(aef7b8d3d00dcc4201e0a1e28026f6f1bdafd0b7) )
	ROM_LOAD( "b44-07.a04", 0xc0000, 0x20000, CRC(3e0e737e) SHA1(f8d62c7b69c79da9df7ef5ce454060d3645e5884) )
	ROM_LOAD( "b44-08.a02", 0xe0000, 0x20000, CRC(6cb1e8fc) SHA1(4ab0c2cce1de2616044a9bfb9bf17f95a49baffd) )

	ROM_REGION( 0x10000, "pal", 0 ) /* these are shared with extermination except d9 */
	ROM_LOAD( "b06-101.pal16l8a.d9.jed", 0x00000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-11.pal16l8a.d6.jed", 0x01000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-12.pal16l8a.c3.jed", 0x02000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-13.pal16l8a.c2.jed", 0x03000, 0x01000, NO_DUMP)
ROM_END

ROM_START( chukataija )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 64k + bankswitch areas for the first CPU */
	ROM_LOAD( "b44-31.11c", 0x00000, 0x10000, CRC(134d3c9e) SHA1(e686a75b9c267db9e8dad7f162c93f1992e395ec) ) /* P0-025-A PCB set */
	ROM_LOAD( "b44-11.9c",  0x10000, 0x10000, CRC(32484094) SHA1(f320fea2910816b5085ca9aa37e30af665fb6be1) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "b44-32.4e", 0x00000, 0x10000, CRC(f52d2f90) SHA1(f040b16e29553ae510e50e1ed55344a36fdcc56d) )

	ROM_REGION( 0x10000, "mcu", 0 ) /* M-Chip (i8x42 internal ROM) */
	ROM_LOAD( "b06__14.1g", 0x0000, 0x0800, CRC(28907072) SHA1(21c7017af8a8ceb8e43d7e798f48518b136fd45c) ) /* Labeled B06-14 and under printed label "Taito M-001, 128P, 720100", is a mask 8042 */

	ROM_REGION( 0x100000, "gfx1", 0 ) /* located on the K9100189A J9100141A ROM7PCB daughterboard */
	ROM_LOAD( "b44-21.rom4l", 0x00000, 0x20000, CRC(aae7b3d5) SHA1(52809ea22d98811ece2fb27e80db6ddf4fbacb07) ) /* same data as B44-01 through B44-08 */
	ROM_LOAD( "b44-22.rom4h", 0x20000, 0x20000, CRC(7f0b9568) SHA1(415d2638d1b0eb36b2e2f63219cbc0dbebe02dc6) )
	ROM_LOAD( "b44-23.rom3l", 0x40000, 0x20000, CRC(5a54a3b9) SHA1(6b219f1c3570f16eb4a06221d7e527c735437bac) )
	ROM_LOAD( "b44-24.rom3h", 0x60000, 0x20000, CRC(3c5f544b) SHA1(d3b0ee18f1027483a36ef02757b62f42a086a8e2) )
	ROM_LOAD( "b44-25.rom2l", 0x80000, 0x20000, CRC(d1b7e314) SHA1(8b4181caa32955b4274614a4238bb24d67ecb729) )
	ROM_LOAD( "b44-26.rom2h", 0xa0000, 0x20000, CRC(269978a8) SHA1(aef7b8d3d00dcc4201e0a1e28026f6f1bdafd0b7) )
	ROM_LOAD( "b44-27.rom1l", 0xc0000, 0x20000, CRC(3e0e737e) SHA1(f8d62c7b69c79da9df7ef5ce454060d3645e5884) )
	ROM_LOAD( "b44-28.rom1h", 0xe0000, 0x20000, CRC(6cb1e8fc) SHA1(4ab0c2cce1de2616044a9bfb9bf17f95a49baffd) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "b44-30.15f", 0x00000, 0x200, CRC(b3de8312) SHA1(dac0d9bfb593d691fd7030e2b1b13be1218929a4) )  /* hi bytes, AM27S29 or compatible like MB7124 */
	ROM_LOAD( "b44-29.17f", 0x00200, 0x200, CRC(ae44b8fb) SHA1(2bf5aca32bb301c0187deb05a3f4a482ac97f0ef) )  /* lo bytes, AM27S29 or compatible like MB7124 */

	ROM_REGION( 0x10000, "pal", 0 ) /* these are shared with extermination */
	ROM_LOAD( "b06-10.pal16l8a.d9.jed", 0x00000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-11.pal16l8a.d6.jed", 0x01000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-12.pal16l8a.c3.jed", 0x02000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-13.pal16l8a.c2.jed", 0x03000, 0x01000, NO_DUMP)
ROM_END


/*
The New Zealand Story, Taito, 1988
Hardware info by Guru

PCB Layout ("New style PCB" with 3x z80 and no M-chip, and a daughterboard with ROMs and z80)
----------
The tnzs PCB has a sticker label which says "M6100409A // N.Z.LAND STORY"

M6100356A (on PCB)
P0-043A (Seta number; on PCB)
|---------------------------------------------------|
|     VOL  HA17408           B53-26.U34  DSWB DSWA  |
|      4558       YM2203 Z80  62256              Z80|
|      4558 YM3014                                  |
|                                                   |
|                                  B06-13           |
|                                   (PAL)           |
|                                                   |
|                                                   |
|                               6264       B53-25.U3|
|J     TESTSW                                       |
|A                                                  |
|M                                                  |
|M                                 B06-101          |
|A                                  (PAL)           |
|---------------------------------------------------| <- ROM Board above
|                                              DIP40|    main PCB
|               X1-001A                             |
|                                                   |
|    X1-004                                         |
|               X1-002A       12MHz                 |
|                                                   |
|                                                   |
|   X1-006                              6264        |
|X1-007     DIP40  DIP40  DIP40  DIP40     B53-24.U1|
|---------------------------------------------------|
Notes:
      All Z80 CPU's running at 6.000MHz (12/2)
      YM2203 running at 3.000MHz (12/4)
      VSync 60Hz
      DIP40 - Empty sockets used for connection of ROM board
      Seta Custom IC's -
                        X1-001A
                        X1-002A
                        X1-004
                        X1-006
                        X1-007


ROM Board
---------
K9100209A
J9100159A SUB PCB
K9100209A N. Z. LAND STORY (sticker)
|---------------------------------------------------|
|                                  PAL(B53-15)      |
|     B53_20     B53_18      B53_16     Z80B   DIP40|
|B53_21    B53_19     B53_17                        |
|                                                   |
|                                                   |
|                                                   |
|B53_23                                             |
|    B53_22                                         |
|           DIP40  DIP40  DIP40  DIP40     62256    |
|---------------------------------------------------|
Notes:
      Z80 clock 6.000MHz
      DIP40 - connection pins joining to Main PCB DIP40 sockets
      All ROMs are 27010 (DIP32)
*/

/* tnzs - new style PCB sets */
ROM_START( tnzs )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 64k + bankswitch areas for the first CPU */
	ROM_LOAD( "b53-24.u1",   0x00000, 0x20000, CRC(d66824c6) SHA1(fd381ac0dc52ce670c3fde320ea60a209e288a52) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "b53-25.u3",   0x00000, 0x10000, CRC(d6ac4e71) SHA1(f3e71624a8a5e4e4c8a6aa01711ed26bdd5abf5a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the third CPU */
	ROM_LOAD( "b53-26.u34",  0x00000, 0x10000, CRC(cfd5649c) SHA1(4f6afccd535d39b41661dc3ccd17af125bfac015) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* the newer PCBs have updated GFX ROM labels, content is the same. Located on a SUB PCB */
	ROM_LOAD( "b53-16.ic7",   0x00000, 0x20000, CRC(c3519c2a) SHA1(30fe7946fbc95ab6b3ccb6944fb24bf47bf3d743) ) /* Also labeled as U35L */
	ROM_LOAD( "b53-17.ic8",   0x20000, 0x20000, CRC(2bf199e8) SHA1(4ed73e4f00ae2f5f4028a0ea5ae3cd238863a370) ) /* Also labeled as U35U */
	ROM_LOAD( "b53-18.ic9",   0x40000, 0x20000, CRC(92f35ed9) SHA1(5fdd8d6ddbb7be9887af3c8dea9ad3b58c4e86f9) ) /* Also labeled as U39L */
	ROM_LOAD( "b53-19.ic10",  0x60000, 0x20000, CRC(edbb9581) SHA1(539396a01ca0b69455f000d446759b232530b542) ) /* Also labeled as U39U */
	ROM_LOAD( "b53-22.ic11",  0x80000, 0x20000, CRC(59d2aef6) SHA1(b657b7603c3eb5f169000d38497ebb93f26f7832) ) /* Also labeled as U43L */
	ROM_LOAD( "b53-23.ic13",  0xa0000, 0x20000, CRC(74acfb9b) SHA1(90b544ed7ede7565660bdd13c94c15c54423cda9) ) /* Also labeled as U43U */
	ROM_LOAD( "b53-20.ic12",  0xc0000, 0x20000, CRC(095d0dc0) SHA1(ced2937d0594fa00ae344a4e3a3cba23772dc160) ) /* Also labeled as U46L */
	ROM_LOAD( "b53-21.ic14",  0xe0000, 0x20000, CRC(9800c54d) SHA1(761647177d621ac2cdd8b009876eed35809f3c92) ) /* Also labeled as U46U */

	ROM_REGION( 0x10000, "pal", 0 )
	/* these are shared with extermination except for the subpcb pal */
	ROM_LOAD( "b06-13.pal16l8a.f2.jed",  0x01000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-101.pal16l8a.i2.jed", 0x02000, 0x01000, NO_DUMP)
	ROM_LOAD( "b53-15.pal16l8a.subpcb.ic6.jed", 0x03000, 0x01000, NO_DUMP) // on sub pcb
ROM_END

ROM_START( tnzsj )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 64k + bankswitch areas for the first CPU */
	ROM_LOAD( "b53-24.u1",   0x00000, 0x20000, CRC(d66824c6) SHA1(fd381ac0dc52ce670c3fde320ea60a209e288a52) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "b53-27.u3",   0x00000, 0x10000, CRC(b3415fc3) SHA1(a12b1788509e2ac2b05a083f432eecdce00769f6) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the third CPU */
	ROM_LOAD( "b53-26.u34",  0x00000, 0x10000, CRC(cfd5649c) SHA1(4f6afccd535d39b41661dc3ccd17af125bfac015) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* the newer PCBs have updated GFX ROM labels, content is the same. Located on a SUB PCB */
	ROM_LOAD( "b53-16.ic7",   0x00000, 0x20000, CRC(c3519c2a) SHA1(30fe7946fbc95ab6b3ccb6944fb24bf47bf3d743) ) /* Also labeled as U35L */
	ROM_LOAD( "b53-17.ic8",   0x20000, 0x20000, CRC(2bf199e8) SHA1(4ed73e4f00ae2f5f4028a0ea5ae3cd238863a370) ) /* Also labeled as U35U */
	ROM_LOAD( "b53-18.ic9",   0x40000, 0x20000, CRC(92f35ed9) SHA1(5fdd8d6ddbb7be9887af3c8dea9ad3b58c4e86f9) ) /* Also labeled as U39L */
	ROM_LOAD( "b53-19.ic10",  0x60000, 0x20000, CRC(edbb9581) SHA1(539396a01ca0b69455f000d446759b232530b542) ) /* Also labeled as U39U */
	ROM_LOAD( "b53-22.ic11",  0x80000, 0x20000, CRC(59d2aef6) SHA1(b657b7603c3eb5f169000d38497ebb93f26f7832) ) /* Also labeled as U43L */
	ROM_LOAD( "b53-23.ic13",  0xa0000, 0x20000, CRC(74acfb9b) SHA1(90b544ed7ede7565660bdd13c94c15c54423cda9) ) /* Also labeled as U43U */
	ROM_LOAD( "b53-20.ic12",  0xc0000, 0x20000, CRC(095d0dc0) SHA1(ced2937d0594fa00ae344a4e3a3cba23772dc160) ) /* Also labeled as U46L */
	ROM_LOAD( "b53-21.ic14",  0xe0000, 0x20000, CRC(9800c54d) SHA1(761647177d621ac2cdd8b009876eed35809f3c92) ) /* Also labeled as U46U */

	ROM_REGION( 0x10000, "pal", 0 )
	/* these are shared with extermination except for the subpcb pal */
	ROM_LOAD( "b06-13.pal16l8a.f2.jed",  0x01000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-101.pal16l8a.i2.jed", 0x02000, 0x01000, NO_DUMP)
	ROM_LOAD( "b53-15.pal16l8a.subpcb.ic6.jed", 0x03000, 0x01000, NO_DUMP) // on sub pcb
ROM_END


/**********************************************************************************

The New Zealand Story, Taito, 1988
Hardware info by Guru

This version uses 8x 28 pin 1 megabit mask ROMs on the main board. There is no daughterboard on top.
The PCB is very similar to Chuka Taisen, Dr Toppel, Extermination, Kabuki Z and Arkanoid 2 but replaces the two color PROMs with color RAM.
The PCB has Seta chips so it has 2 sets of PCB numbers and stickers.
The Taito logo on the PCB is the newer style with the 'A' split into 2 curved shapes.

PCB Layout
----------

SETA:
P0-041A
NEW ZEALAND STORY 905-1054 (sticker)

TAITO:
K1100365A J1100156A
K1100356A N. Z.LAND STORY (sticker)
|--------------------------------------------------|
|             X1-006                       12MHz   |
| MB3730  X1-007      8464    X1-001A              |
|                                                  |
| VOL                         X1-002A              |
|      4558                                        |
|    YM3014    SW                                  |
|                                                  |
|        SWB                           B53_08.U8   |
|J                                                 |
|A                                     B53_07.U7   |
|M       SWA            B53_10.U32                 |
|M                      62256          B53_06.U6   |
|A                                                 |
|                                      B53_05.U5   |
|                                                  |
|                                      B53_04.U4   |
|                    B06-101.U36                   |
|       YM2203   Z80B   6264           B53_03.U3   |
|                    B53-12.U35                    |
|X2-005(x5)                            B53-02.U2   |
|          B53-09.U46      B06-12.U26              |
|    X1-004     B53_11.U38 B06-13.U25  B53_01.U1   |
|                      Z80B(1)                     |
|--------------------------------------------------|
Notes:
         Z80B(1) - Z80 CPU. Clock 6.000MHz [12/2] (main CPU)
            Z80B - Z80 CPU. Clock 6.000MHz [12/2] (sound CPU)
          YM2203 - Yamaha YM2203 FM Operator Type-N(OPN) sound chip. Clock 3.000MHz [12/4]
          YM3014 - Yamaha YM3014 Serial Input Floating D/A Converter. Clock 1.000MHz [12/12]
            4558 - 4558 Dual Operational Amplifier
          MB3730 - Fujitsu MB3730 Audio Power Amp
          X2-005 - Custom resistor array used for inputs
          X1-004 - Seta custom chip marked 'X1-004 in SDIP52 package used for I/O
          X1-006 - Seta custom chip marked 'X1-006' in SDIP64 package used for palette and pixel mixing functions
          X1-007 - Seta custom chip marked 'X1-007' in SDIP42 package
                   RGB and sync on the JAMMA connector are tied to this chip so it's likely an RGB DAC
         X1-001A - Seta custom graphics chip \
         X1-002A - Seta custom graphics chip / These work together to create both sprites, tiles and text layer graphics
           SWA/B - 8-position DIP switch
              SW - Push button switch for reset
            8464 - Fujitsu MB8464 8kBx8-bit SRAM (color RAM)
           62256 - Hitachi HM62256 32kBx8-bit SRAM (main program RAM)
            6264 - Hitachi HM6462 8kBx8-bit SRAM (sound program RAM)
    B53_01 to 08 - 23C1000/TC531000 28 pin mask ROMs (graphics)
      B53-09.U46 - Custom chip marked 'TAITO B53-09 161 832100' in DIP40 package
                   This is really an i8042/i8742 micro-controller with 2kBx8-bit internal ROM and is known as the 'M-Chip'
      B53_10.U32 - 27C1000 EPROM (main program)
      B53_11.U38 - 27C512 EPROM (sound program)
      B53-12.U35 - PAL16L8 marked 'B53-12'
     B06-101.U36 - PAL16L8 marked 'B06-101' \
      B06-12.U26 - PAL16L8 marked 'B06-12'   | Note game 'B06' is Extermination
      B06-13.U25 - PAL16L8 marked 'B06-13'  /
           HSync - 15.6245kHz
           VSync - 59.1836Hz

**********************************************************************************/

ROM_START( tnzso )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 64k + bankswitch areas for the first CPU */
	ROM_LOAD( "b53-10.27c1001d.u32", 0x00000, 0x20000, CRC(a73745c6) SHA1(73eb38e75e08312d752332f988dc655084b4a86d) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "b53-14.u38", 0x00000, 0x10000, CRC(f269c5f1) SHA1(15e00e5bc6394f55fc6c591754e24842708f49f4) ) // World version old style PCB

	ROM_REGION( 0x10000, "mcu", 0 ) /* M-Chip (i8x42 internal ROM) */
	ROM_LOAD( "b53-09.u46", 0x0000, 0x0800, CRC(a4bfce19) SHA1(9340862d5bdc1ad4799dc92cae9bce1428b47478) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "b53-08.u8",   0x00000, 0x20000, CRC(c3519c2a) SHA1(30fe7946fbc95ab6b3ccb6944fb24bf47bf3d743) )
	ROM_LOAD( "b53-07.u7",   0x20000, 0x20000, CRC(2bf199e8) SHA1(4ed73e4f00ae2f5f4028a0ea5ae3cd238863a370) )
	ROM_LOAD( "b53-06.u6",   0x40000, 0x20000, CRC(92f35ed9) SHA1(5fdd8d6ddbb7be9887af3c8dea9ad3b58c4e86f9) )
	ROM_LOAD( "b53-05.u5",   0x60000, 0x20000, CRC(edbb9581) SHA1(539396a01ca0b69455f000d446759b232530b542) )
	ROM_LOAD( "b53-04.u4",   0x80000, 0x20000, CRC(59d2aef6) SHA1(b657b7603c3eb5f169000d38497ebb93f26f7832) )
	ROM_LOAD( "b53-03.u3",   0xa0000, 0x20000, CRC(74acfb9b) SHA1(90b544ed7ede7565660bdd13c94c15c54423cda9) )
	ROM_LOAD( "b53-02.u2",   0xc0000, 0x20000, CRC(095d0dc0) SHA1(ced2937d0594fa00ae344a4e3a3cba23772dc160) )
	ROM_LOAD( "b53-01.u1",   0xe0000, 0x20000, CRC(9800c54d) SHA1(761647177d621ac2cdd8b009876eed35809f3c92) )

	ROM_REGION( 0x10000, "pal", 0 ) /* these are probably shared with extermination except for u35 */
	ROM_LOAD( "b06-12.pal16l8a.u26.jed", 0x00000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-13.pal16l8a.u25.jed", 0x01000, 0x01000, NO_DUMP)
	ROM_LOAD( "b53-12.pal16l8a.u35.jed", 0x02000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-101.pal16l8a.u36.jed", 0x03000, 0x01000, NO_DUMP)
ROM_END

ROM_START( tnzsjo )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 64k + bankswitch areas for the first CPU */
	ROM_LOAD( "b53-10.27c1001d.u32", 0x00000, 0x20000, CRC(a73745c6) SHA1(73eb38e75e08312d752332f988dc655084b4a86d) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "b53-11.27c512.u38", 0x00000, 0x10000, CRC(9784d443) SHA1(bc3647aac9974031dbe4898417fbaa99841f9548) ) // Japan version old style PCB

	ROM_REGION( 0x10000, "mcu", 0 ) /* M-Chip (i8x42 internal ROM) */
	ROM_LOAD( "b53-09.u46", 0x0000, 0x0800, CRC(a4bfce19) SHA1(9340862d5bdc1ad4799dc92cae9bce1428b47478) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	/* ROMs taken from another set (the ones from this set were read incorrectly) */
	ROM_LOAD( "b53-08.u8",   0x00000, 0x20000, CRC(c3519c2a) SHA1(30fe7946fbc95ab6b3ccb6944fb24bf47bf3d743) )
	ROM_LOAD( "b53-07.u7",   0x20000, 0x20000, CRC(2bf199e8) SHA1(4ed73e4f00ae2f5f4028a0ea5ae3cd238863a370) )
	ROM_LOAD( "b53-06.u6",   0x40000, 0x20000, CRC(92f35ed9) SHA1(5fdd8d6ddbb7be9887af3c8dea9ad3b58c4e86f9) )
	ROM_LOAD( "b53-05.u5",   0x60000, 0x20000, CRC(edbb9581) SHA1(539396a01ca0b69455f000d446759b232530b542) )
	ROM_LOAD( "b53-04.u4",   0x80000, 0x20000, CRC(59d2aef6) SHA1(b657b7603c3eb5f169000d38497ebb93f26f7832) )
	ROM_LOAD( "b53-03.u3",   0xa0000, 0x20000, CRC(74acfb9b) SHA1(90b544ed7ede7565660bdd13c94c15c54423cda9) )
	ROM_LOAD( "b53-02.u2",   0xc0000, 0x20000, CRC(095d0dc0) SHA1(ced2937d0594fa00ae344a4e3a3cba23772dc160) )
	ROM_LOAD( "b53-01.u1",   0xe0000, 0x20000, CRC(9800c54d) SHA1(761647177d621ac2cdd8b009876eed35809f3c92) )

	ROM_REGION( 0x10000, "pal", 0 ) /* these are probably shared with extermination except for u35 */
	ROM_LOAD( "b06-12.pal16l8a.u26.jed", 0x00000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-13.pal16l8a.u25.jed", 0x01000, 0x01000, NO_DUMP)
	ROM_LOAD( "b53-12.pal16l8a.u35.jed", 0x02000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-101.pal16l8a.u36.jed", 0x03000, 0x01000, NO_DUMP)
ROM_END

ROM_START( tnzsuo )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 64k + bankswitch areas for the first CPU */
	ROM_LOAD( "b53-10.27c1001d.u32", 0x00000, 0x20000, CRC(a73745c6) SHA1(73eb38e75e08312d752332f988dc655084b4a86d) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "b53-13.27c512.u38", 0x00000, 0x10000, CRC(c09f4d28) SHA1(f1fd3202869738e17abcbb757f9ce7260707dd3d) ) // US version old style PCB

	ROM_REGION( 0x10000, "mcu", 0 ) /* M-Chip (i8x42 internal ROM) */
	ROM_LOAD( "b53-09.u46", 0x0000, 0x0800, CRC(a4bfce19) SHA1(9340862d5bdc1ad4799dc92cae9bce1428b47478) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "b53-08.u8",   0x00000, 0x20000, CRC(c3519c2a) SHA1(30fe7946fbc95ab6b3ccb6944fb24bf47bf3d743) )
	ROM_LOAD( "b53-07.u7",   0x20000, 0x20000, CRC(2bf199e8) SHA1(4ed73e4f00ae2f5f4028a0ea5ae3cd238863a370) )
	ROM_LOAD( "b53-06.u6",   0x40000, 0x20000, CRC(92f35ed9) SHA1(5fdd8d6ddbb7be9887af3c8dea9ad3b58c4e86f9) )
	ROM_LOAD( "b53-05.u5",   0x60000, 0x20000, CRC(edbb9581) SHA1(539396a01ca0b69455f000d446759b232530b542) )
	ROM_LOAD( "b53-04.u4",   0x80000, 0x20000, CRC(59d2aef6) SHA1(b657b7603c3eb5f169000d38497ebb93f26f7832) )
	ROM_LOAD( "b53-03.u3",   0xa0000, 0x20000, CRC(74acfb9b) SHA1(90b544ed7ede7565660bdd13c94c15c54423cda9) )
	ROM_LOAD( "b53-02.u2",   0xc0000, 0x20000, CRC(095d0dc0) SHA1(ced2937d0594fa00ae344a4e3a3cba23772dc160) )
	ROM_LOAD( "b53-01.u1",   0xe0000, 0x20000, CRC(9800c54d) SHA1(761647177d621ac2cdd8b009876eed35809f3c92) )

	ROM_REGION( 0x10000, "pal", 0 ) /* these are probably shared with extermination except for u35 */
	ROM_LOAD( "b06-12.pal16l8a.u26.jed", 0x00000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-13.pal16l8a.u25.jed", 0x01000, 0x01000, NO_DUMP)
	ROM_LOAD( "b53-12.pal16l8a.u35.jed", 0x02000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-101.pal16l8a.u36.jed", 0x03000, 0x01000, NO_DUMP)
ROM_END

ROM_START( tnzsoa ) // is this a legit set, or a hack, or a near-final (later than tnzsop below) prototype?
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 64k + bankswitch areas for the first CPU */
	ROM_LOAD( "b53-unknown.27c1001d.u32", 0x00000, 0x20000, CRC(edf3b39e) SHA1(be221c99e50795d569611dba454c3954a259a859) ) // ROM LABEL FOR THIS SET IS UNKNOWN

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "b53-unknown.27c512.u38", 0x00000, 0x10000, CRC(60340d63) SHA1(12a26d19dc8e407e502f25617a5a4c9cea131ce2) ) // ROM LABEL FOR THIS SET IS UNKNOWN

	ROM_REGION( 0x10000, "mcu", 0 ) /* M-Chip (i8x42 internal ROM) */
	ROM_LOAD( "b53-09.u46", 0x0000, 0x0800, CRC(a4bfce19) SHA1(9340862d5bdc1ad4799dc92cae9bce1428b47478) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	/* ROMs taken from another set (the ones from this set were read incorrectly) */
	ROM_LOAD( "b53-08.u8",   0x00000, 0x20000, CRC(c3519c2a) SHA1(30fe7946fbc95ab6b3ccb6944fb24bf47bf3d743) )
	ROM_LOAD( "b53-07.u7",   0x20000, 0x20000, CRC(2bf199e8) SHA1(4ed73e4f00ae2f5f4028a0ea5ae3cd238863a370) )
	ROM_LOAD( "b53-06.u6",   0x40000, 0x20000, CRC(92f35ed9) SHA1(5fdd8d6ddbb7be9887af3c8dea9ad3b58c4e86f9) )
	ROM_LOAD( "b53-05.u5",   0x60000, 0x20000, CRC(edbb9581) SHA1(539396a01ca0b69455f000d446759b232530b542) )
	ROM_LOAD( "b53-04.u4",   0x80000, 0x20000, CRC(59d2aef6) SHA1(b657b7603c3eb5f169000d38497ebb93f26f7832) )
	ROM_LOAD( "b53-03.u3",   0xa0000, 0x20000, CRC(74acfb9b) SHA1(90b544ed7ede7565660bdd13c94c15c54423cda9) )
	ROM_LOAD( "b53-02.u2",   0xc0000, 0x20000, CRC(095d0dc0) SHA1(ced2937d0594fa00ae344a4e3a3cba23772dc160) )
	ROM_LOAD( "b53-01.u1",   0xe0000, 0x20000, CRC(9800c54d) SHA1(761647177d621ac2cdd8b009876eed35809f3c92) )

	ROM_REGION( 0x10000, "pal", 0 ) /* PALS not directly observed on this board but assumed to exist */
	/* these are probably shared with extermination except for u35 */
	ROM_LOAD( "b06-12.pal16l8a.u26.jed", 0x00000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-13.pal16l8a.u25.jed", 0x01000, 0x01000, NO_DUMP)
	ROM_LOAD( "b53-12.pal16l8a.u35.jed", 0x02000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-101.pal16l8a.u36.jed", 0x03000, 0x01000, NO_DUMP)
ROM_END

/* This is a prototype CA403001A PCB (Seta: P0-041-1), and is ALMOST but not exactly the same as the K1100356A/J1100156A (Seta: P0-041A) 'tnzsuo/tnzsjo/arkanoid2/etc' pcb above:
This pcb uses 32-pin 27c1000d EPROMs for the 8 gfx ROMs, and the final K1100356A/J1100156A pcb uses 28 pin 23c1000 mask ROMs instead. Some capacitors near the jamma connector were moved as well.
No other obviously evident routing/wiring changes are present.
This type of pcb might have been used for in-house testing of all the games on this hardware.
*/
ROM_START( tnzsop ) // prototype (location test?) version; has different ROM labels, and the Seta X1-001 chip has prototype markings revealing it was fabbed by Yamaha, as 'YM3906'
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 64k + bankswitch areas for the first CPU */
	ROM_LOAD( "c-11__6-24__1959h.d27c1000d-15.u32", 0x00000, 0x20000, CRC(3c1dae7b) SHA1(0004fccc171714c80565326f8690f9662c5b75d9) ) // Labeled as PCB location, date of 6/24 & checksum - NEC D271000d  EPROM

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "e-3__6-24__c4ach.tmm27512d-20.u38", 0x00000, 0x10000, CRC(c7662e96) SHA1(be28298bfde4e3867cfe75633ffb0f8611dbbd8b) ) // Labeled as PCB location, date of 6/24 & checksum - TMM27512D  EPROM

	ROM_REGION( 0x10000, "mcu", 0 ) /* M-Chip (i8x42 internal ROM) */
	ROM_LOAD( "b8042h___88-6-22__0fcc.d8742.u46", 0x0000, 0x0800, CRC(a4bfce19) SHA1(9340862d5bdc1ad4799dc92cae9bce1428b47478) ) // Dated  '88/6/22 with checksum - Intel D8742 MCU

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "a13__03e8.d27c1000d-15.a13",  0x00000, 0x20000, CRC(7e0bd5bb) SHA1(95dfb00ec915778e02d8bfa996735ab817191adc) ) // labels contain the PCB location & checksum
	ROM_LOAD( "a12__f4ec.d27c1000d-15.a12",  0x20000, 0x20000, CRC(95880726) SHA1(f4fdedd23e80a6ccf32f737ab4bc57f9fc0925be) ) // PCB CA403001A did NOT have silkscreened U1 - U8 labels
	ROM_LOAD( "a10__f2b5.d27c1000d-15.a10",  0x40000, 0x20000, CRC(2bc4c053) SHA1(cd7668a7733e5e80c2c566d0cf63c4310e5743b4) ) // PCB also labeled as P0-041-1
	ROM_LOAD( "a08__bd49.d27c1000d-15.a8",   0x60000, 0x20000, CRC(8ff8d88c) SHA1(31977e39ad048a077e9b5bd712ff66b14a466d27) )
	ROM_LOAD( "a07__d5f3.d27c1000d-15.a7",   0x80000, 0x20000, CRC(291bcaca) SHA1(4f659a0cd2ff6b4ec04ab95ee8a670222c402c2b) )
	ROM_LOAD( "a05__662a.d27c1000d-15.a5",   0xa0000, 0x20000, CRC(6e762e20) SHA1(66731fe4053b9c09bc9c95d10aba212db08b4636) )
	ROM_LOAD( "a04__0c21.d27c1000d-15.a4",   0xc0000, 0x20000, CRC(e1fd1b9d) SHA1(6027491b927c2ab9c77fbf8895da1abcfbe32d62) )
	ROM_LOAD( "a02__904f.d27c1000d-15.a2",   0xe0000, 0x20000, CRC(2ab06bda) SHA1(2b208b564e55c258665e1f66b26fe14a6c68eb96) )

	ROM_REGION( 0x10000, "pal", 0 )
	/* these are probably shared with extermination except for u35 */
	ROM_LOAD( "b06-12.pal16l8a.u26.jed",  0x00000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-13.pal16l8a.u25.jed",  0x01000, 0x01000, NO_DUMP)
	ROM_LOAD( "st-6.pal16l8a.u35.jed",    0x02000, 0x01000, NO_DUMP)
	ROM_LOAD( "b06-101.pal16l8a.u36.jed", 0x03000, 0x01000, NO_DUMP)
ROM_END

/*
Kabuki Z, Taito, 1988
Hardware info by Guru

This PCB runs on Taito/Seta hardware and the exact same newer PCB as The New Zealand Story.
As such, everything here also applies to The New Zealand Story.
Unlike the newer The New Zealand Story pcb, Kabuki Z lacks the daughterboard with the 3rd z80.

PCB Layout
----------

M6100356A (on PCB)
M6100356A (on sticker)
P0-043A (Seta number; on PCB)
|---------------------------------------------------|
|     VOL  HA17408           B50-07.U34  DSWB DSWA  |
|      4558       YM2203 Z80  62256              Z80|
|      4558 YM3014                                  |
|                                                   |
|                                  B06-13           |
|                                   (PAL)           |
|                                                   |
|                                                   |
|                               6264       B50-06.U3|
|J     TESTSW                                       |
|A                                                  |
|M                                                  |
|M                                                  |
|A                                 B06-101          |
|                                    (PAL)          |
|                                                Z80|
|               X1-001A                             |
|                                                   |
|    X1-004                                         |
|               X1-002A       12MHz                 |
|                                                   |
|         B50-01.U46    B50-03.U39                  |
|   X1-006                         6264             |
|X1-007        B50-02.U43   B50-04.U35     B50-05.U1|
|---------------------------------------------------|
Notes:
      All Z80 CPU's running at 6.000MHz (12/2)
      YM2203 running at 3.000Mz (12/4)
      VSync 60Hz
*/

ROM_START( kabukiz )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 64k + bankswitch areas for the first CPU */
	ROM_LOAD( "b50-05.u1",  0x00000, 0x20000, CRC(9cccb129) SHA1(054faf7657bad7237182e36bcc4388b1748af935) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "b50-08.1e",  0x00000, 0x10000, CRC(cb92d34c) SHA1(3a666f0e3ff9d3daa599123edee228d94eeae754) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* 64k + bankswitch areas for the third CPU */
	ROM_LOAD( "b50-07.u34", 0x00000, 0x20000, CRC(bf7fc2ed) SHA1(77008d12d9bdbfa100dcd87cd6ca7de3748408c5) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "b50-04.u35", 0x000000, 0x80000, CRC(04829aa9) SHA1(a501ec7c802478fc41ec8ef4270b1a6872bcbf34) )
	ROM_LOAD( "b50-03.u39", 0x080000, 0x80000, CRC(31489a4c) SHA1(a4b7e00e2074287b47c7e16add963c1470534376) )
	ROM_LOAD( "b50-02.u43", 0x100000, 0x80000, CRC(90b8a8e7) SHA1(a55e327307606142fbb9d500e757655b35e1f252) )
	ROM_LOAD( "b50-01.u46", 0x180000, 0x80000, CRC(f4277751) SHA1(8f50f843f0eda30d639ba397889236ff0a3edce5) )
ROM_END

ROM_START( kabukizj )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 64k + bankswitch areas for the first CPU */
	ROM_LOAD( "b50-05.u1",  0x00000, 0x20000, CRC(9cccb129) SHA1(054faf7657bad7237182e36bcc4388b1748af935) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "b50-06.u3",  0x00000, 0x10000, CRC(45650aab) SHA1(00d1fc6044a6ad1e82476ccbe730907b4d780cb9) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* 64k + bankswitch areas for the third CPU */
	ROM_LOAD( "b50-07.u34", 0x00000, 0x20000, CRC(bf7fc2ed) SHA1(77008d12d9bdbfa100dcd87cd6ca7de3748408c5) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "b50-04.u35", 0x000000, 0x80000, CRC(04829aa9) SHA1(a501ec7c802478fc41ec8ef4270b1a6872bcbf34) )
	ROM_LOAD( "b50-03.u39", 0x080000, 0x80000, CRC(31489a4c) SHA1(a4b7e00e2074287b47c7e16add963c1470534376) )
	ROM_LOAD( "b50-02.u43", 0x100000, 0x80000, CRC(90b8a8e7) SHA1(a55e327307606142fbb9d500e757655b35e1f252) )
	ROM_LOAD( "b50-01.u46", 0x180000, 0x80000, CRC(f4277751) SHA1(8f50f843f0eda30d639ba397889236ff0a3edce5) )
ROM_END

ROM_START( insectx )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 64k + bankswitch areas for the first CPU */
	ROM_LOAD( "b97__03.u32", 0x00000, 0x20000, CRC(18eef387) SHA1(b22633930d39be1e72fbd5b080972122da3cb3ef) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "b97__07.u38", 0x00000, 0x10000, CRC(324b28c9) SHA1(db77a4ac60196d0f0f35dbc5c951ec29d6392463) ) /* Label is B97 07* with an asterisk */

	ROM_REGION( 0x100000, "gfx1", 0 ) /* Mask ROMs */
	ROM_LOAD( "b97__01.u1", 0x00000, 0x80000, CRC(d00294b1) SHA1(f43a4f7d13193ddbbcdef71a5085c1db0fc062d4) )
	ROM_LOAD( "b97__02.u2", 0x80000, 0x80000, CRC(db5a7434) SHA1(71fac872b19a13a7ad25c8ad895c322ec9573fdc) )
ROM_END

ROM_START( insectxj )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 64k + bankswitch areas for the first CPU */
	ROM_LOAD( "b97__03.u32", 0x00000, 0x20000, CRC(18eef387) SHA1(b22633930d39be1e72fbd5b080972122da3cb3ef) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "b97__04.u38", 0x00000, 0x10000, CRC(dc4549e5) SHA1(9920f7c12e047ee165418d33b3add51ea615df7e) ) /* Label is B97 04* with an asterisk */

	ROM_REGION( 0x100000, "gfx1", 0 ) /* Mask ROMs */
	ROM_LOAD( "b97__01.u1", 0x00000, 0x80000, CRC(d00294b1) SHA1(f43a4f7d13193ddbbcdef71a5085c1db0fc062d4) )
	ROM_LOAD( "b97__02.u2", 0x80000, 0x80000, CRC(db5a7434) SHA1(71fac872b19a13a7ad25c8ad895c322ec9573fdc) )
ROM_END

ROM_START( insectxbl )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ic71", 0x00000, 0x20000, CRC(86ae1c66) SHA1(15a8d2fa296248346908643a5ff3a69dc2a0938a) ) // mainly copyright change

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "ic3", 0x00000, 0x10000, CRC(324b28c9) SHA1(db77a4ac60196d0f0f35dbc5c951ec29d6392463) ) // identical to the original

	ROM_REGION( 0x100000, "gfx1", 0 ) // smaller ROMs, Taito and title have been blanked out
	ROM_LOAD16_BYTE( "ic174", 0x00000, 0x20000, CRC(f5a5c8bf) SHA1(e5ecf0c43bf28fda73a85c9f0674872c1d41eac8) )
	ROM_LOAD16_BYTE( "ic176", 0x00001, 0x20000, CRC(ef3436f4) SHA1(e143070d8ac4398af2f00e771e218b87a06b1afa) )
	ROM_LOAD16_BYTE( "ic175", 0x40000, 0x20000, CRC(e926ec1b) SHA1(e013200ec58f8274a83c53c5d34c98c61a035803) )
	ROM_LOAD16_BYTE( "ic177", 0x40001, 0x20000, CRC(88ead1fb) SHA1(952ef8301b13239e8f6877fa26b59caab5df81e2) )
	ROM_LOAD16_BYTE( "ic212", 0x80000, 0x20000, CRC(54547590) SHA1(6e756bd88d89df092552b2e7f06f3dd3a077803f) )
	ROM_LOAD16_BYTE( "ic214", 0x80001, 0x20000, CRC(da312ccd) SHA1(a8eea3730cbcd64d61ef5fcee69dd28126cf60e1) )
	ROM_LOAD16_BYTE( "ic213", 0xc0000, 0x20000, CRC(5b6faea0) SHA1(18b3ca62153b689b5d42e91f4a56b72bb9f6f94f) )
	ROM_LOAD16_BYTE( "ic215", 0xc0001, 0x20000, CRC(ff1dee9e) SHA1(3ef91f8188ae400880c03ba8d1fc039c8920d6c0) )
ROM_END

} // anonymous namespace


//    YEAR, NAME,      PARENT,   MACHINE,  INPUT,    CLASS,          INIT,       MONITOR,COMPANY,             FULLNAME,            FLAGS
GAME( 1987, plumppop,  0,        plumppop, plumppop, plumppop_state, empty_init, ROT0,   "Taito Corporation", "Plump Pop (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, jpopnics,  0,        jpopnics, jpopnics, jpopnics_state, empty_init, ROT0,   "Nics",              "Jumping Pop (Nics, Korean hack of Plump Pop)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

GAME( 1987, extrmatn,  0,        extrmatn, extrmatn, tnzs_mcu_state, empty_init, ROT270, "Taito Corporation Japan",                         "Extermination (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, extrmatnu, extrmatn, extrmatn, extrmatn, tnzs_mcu_state, empty_init, ROT270, "Taito (World Games license)",                     "Extermination (US, World Games)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, extrmatnur,extrmatn, extrmatn, extrmatn, tnzs_mcu_state, empty_init, ROT270, "Taito America Corporation (Romstar license)",     "Extermination (US, Romstar)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, extrmatnj, extrmatn, extrmatn, extrmatn, tnzs_mcu_state, empty_init, ROT270, "Taito Corporation",                               "Extermination (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1987, arknoid2,  0,        arknoid2, arknoid2, arknoid2_state, empty_init, ROT270, "Taito Corporation Japan",                     "Arkanoid - Revenge of DOH (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, arknoid2u, arknoid2, arknoid2, arknid2u, arknoid2_state, empty_init, ROT270, "Taito America Corporation (Romstar license)", "Arkanoid - Revenge of DOH (US)",    MACHINE_SUPPORTS_SAVE )
GAME( 1987, arknoid2j, arknoid2, arknoid2, arknid2u, arknoid2_state, empty_init, ROT270, "Taito Corporation",                           "Arkanoid - Revenge of DOH (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, arknoid2b, arknoid2, arknoid2, arknid2u, arknoid2_state, empty_init, ROT270, "bootleg",                                     "Arkanoid - Revenge of DOH (Japan bootleg)", MACHINE_SUPPORTS_SAVE )

GAME( 1987, drtoppel,  0,        extrmatn, drtoppel, tnzs_mcu_state, empty_init, ROT90,  "Kaneko / Taito Corporation Japan",   "Dr. Toppel's Adventure (World)", MACHINE_SUPPORTS_SAVE ) /* Possible region hack */
GAME( 1987, drtoppelu, drtoppel, extrmatn, drtopplu, tnzs_mcu_state, empty_init, ROT90,  "Kaneko / Taito America Corporation", "Dr. Toppel's Adventure (US)",    MACHINE_SUPPORTS_SAVE ) /* Possible region hack */
GAME( 1987, drtoppelj, drtoppel, extrmatn, drtopplu, tnzs_mcu_state, empty_init, ROT90,  "Kaneko / Taito Corporation",         "Dr. Toppel's Tankentai (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1988, kageki,    0,        kageki,   kageki,   kageki_state,   empty_init, ROT90,  "Kaneko / Taito Corporation",                           "Kageki (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, kagekiu,   kageki,   kageki,   kagekiu,  kageki_state,   empty_init, ROT90,  "Kaneko / Taito America Corporation (Romstar license)", "Kageki (US)",    MACHINE_SUPPORTS_SAVE )
GAME( 1988, kagekij,   kageki,   kageki,   kagekij,  kageki_state,   empty_init, ROT90,  "Kaneko / Taito Corporation",                           "Kageki (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, kagekih,   kageki,   kageki,   kageki,   kageki_state,   empty_init, ROT90,  "hack",                                                 "Kageki (hack)",  MACHINE_SUPPORTS_SAVE ) // date is hacked at least, might also be a Japan set hacked to show english

GAME( 1988, chukatai,  0,        tnzs,     chukatai, tnzs_mcu_state, empty_init, ROT0,   "Taito Corporation Japan",   "Chuka Taisen (World) (P0-028-A PCB)", MACHINE_SUPPORTS_SAVE ) /* Possible region hack */
GAME( 1988, chukataiu, chukatai, tnzs,     chukatau, tnzs_mcu_state, empty_init, ROT0,   "Taito America Corporation", "Chuka Taisen (US) (P0-028-A PCB)",    MACHINE_SUPPORTS_SAVE ) /* Possible region hack */
GAME( 1988, chukataij, chukatai, tnzs,     chukatau, tnzs_mcu_state, empty_init, ROT0,   "Taito Corporation",         "Chuka Taisen (Japan) (P0-028-A PCB)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, chukataija,chukatai, extrmatn, chukatau, tnzs_mcu_state, empty_init, ROT0,   "Taito Corporation",         "Chuka Taisen (Japan) (P0-025-A PCB)", MACHINE_SUPPORTS_SAVE ) /* Higher ROM ID# but older PCB stock */

GAME( 1988, tnzs,      0,        tnzsb,    tnzs,     tnzsb_state,    empty_init, ROT0,   "Taito Corporation Japan",   "The NewZealand Story (World, new version) (P0-043A PCB)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, tnzsj,     tnzs,     tnzsb,    tnzsj,    tnzsb_state,    empty_init, ROT0,   "Taito Corporation",         "The NewZealand Story (Japan, new version) (P0-043A PCB)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, tnzso,     tnzs,     tnzs,     tnzsop,   tnzs_mcu_state, empty_init, ROT0,   "Taito Corporation Japan",   "The NewZealand Story (World, old version) (P0-041A PCB)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, tnzsjo,    tnzs,     tnzs,     tnzsjo,   tnzs_mcu_state, empty_init, ROT0,   "Taito Corporation",         "The NewZealand Story (Japan, old version) (P0-041A PCB)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, tnzsuo,    tnzs,     tnzs,     tnzsjo,   tnzs_mcu_state, empty_init, ROT0,   "Taito America Corporation", "The NewZealand Story (US, old version) (P0-041A PCB)",    MACHINE_SUPPORTS_SAVE )
GAME( 1988, tnzsoa,    tnzs,     tnzs,     tnzsop,   tnzs_mcu_state, empty_init, ROT0,   "Taito Corporation Japan",   "The NewZealand Story (World, unknown version) (P0-041A PCB)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, tnzsop,    tnzs,     tnzs,     tnzsop,   tnzs_mcu_state, empty_init, ROT0,   "Taito Corporation Japan",   "The NewZealand Story (World, prototype) (P0-041-1 PCB)",  MACHINE_SUPPORTS_SAVE )

GAME( 1988, kabukiz,   0,        kabukiz,  kabukiz,  kabukiz_state,  empty_init, ROT0,   "Kaneko / Taito Corporation Japan", "Kabuki-Z (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, kabukizj,  kabukiz,  kabukiz,  kabukizj, kabukiz_state,  empty_init, ROT0,   "Kaneko / Taito Corporation",       "Kabuki-Z (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1989, insectx,   0,        insectx,  insectx,  insectx_state,  empty_init, ROT0,   "Taito Corporation Japan",   "Insector X (World)",   MACHINE_SUPPORTS_SAVE )
GAME( 1989, insectxj,  insectx,  insectx,  insectxj, insectx_state,  empty_init, ROT0,   "Taito Corporation",         "Insector X (Japan)",   MACHINE_SUPPORTS_SAVE )
GAME( 1990, insectxbl, insectx,  insectx,  insectxj, insectx_state,  empty_init, ROT0,   "bootleg (Nagoya Kaihatsu)", "Insector X (bootleg)", MACHINE_SUPPORTS_SAVE )
