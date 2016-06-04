// license:BSD-3-Clause
// copyright-holders:Darren Olafson, Quench,Stephane Humbert
/***************************************************************************

        ToaPlan game hardware from 1988-1991
        ------------------------------------
        MAME Driver by: Darren Olafson
        Technical info: Carl-Henrik Skarstedt  &  Magnus Danielsson
        Driver updates: Quench
        Video updates : SUZ


Supported games:

    ROM set     Toaplan
    name        board No        Game name
    --------------------------------------------------
    rallybik    TP-O12      Rally Bike/Dash Yarou
    truxton     TP-O13B     Truxton/Tatsujin
    hellfire    B90         HellFire (2 Player version) Uses Taito rom ID number
    hellfire1   B90         HellFire (1 Player version) Uses Taito rom ID number
    hellfire2a  B90         HellFire (2 Player older version) Uses Taito rom ID number
    hellfire1a  B90         HellFire (1 Player older version) Uses Taito rom ID number
    zerowing    TP-O15      Zero Wing (2 player simultaneous version)
    zerowing1   TP-O15      Zero Wing (1 Player version)
    zerowingw   TP-O15      Zero Wing (2 player simultaneous version, Williams Electronics Games, Inc)
    demonwld    TP-O16      Demon's World/Horror Story [1990]
    demonwl1    TP-O16      Demon's World/Horror Story [1989] (Taito license)
    demonwl2    TP-O16      Demon's World/Horror Story [1989] (early edition)
    demonwl3    TP-O16      Demon's World/Horror Story [1989] (first edition)
    fireshrk    TP-O17      Fire Shark (World)           [1990]
    fireshrka   TP-O17      Fire Shark (World)           [1989]
    samesame    TP-O17      Same! Same! Same! (Japan)    [1989] (1 Player version)
    samesam2    TP-O17      Same! Same! Same! (Japan)    [1989] (2 Player version)
    outzone     TP-O18      Out Zone
    outzoneh    TP-018      Out Zone (harder version)
    outzonea    TP-018      Out Zone (old version)
    outzoneb    TP-018      Out Zone (older version)
    outzonec    TP-O18      Out Zone (oldest version, from board serial number 2122)
    vimana      TP-O19      Vimana (From board serial number 1547.04 [July '94])
    vimanaj     TP-O19      Vimana (Japan version)
    vimanan     TP-O19      Vimana (Nova Apparate GMBH & Co  license)


Notes:
    Fire Shark and Same! Same! Same! have a hidden function for the
    service input. When invulnerability is enabled, pressing the
    service input makes the screen scroll faster.

    Demonwld (Toaplan copyright) is a newer version, and has a different game
    level sequence compared to the Taito licensed version.


Stephh's and AWJ's notes (based on the games M68000 and Z80 code and some tests) :

1) 'rallybik'

  - Region read from DSWB (port 0x50 in CPU1) then stored at 0x8004 (CPU1 shared RAM) =
    0x180008.w (CPU0 shared RAM) then stored at 0x0804f4.w .
  - Coinage relies on bits 4 and 5 of the region (code at 0x0bda in CPU1) :
      * ..10.... : TOAPLAN_COINAGE_WORLD (tables at 0x0c35 (COIN1) and 0x0c3d (COIN2) in CPU1)
      *  else    : TOAPLAN_COINAGE_JAPAN (table at 0x0c25 (COIN1 AND COIN2) in CPU1)
  - Title screen relies on bits 4 and 5 of the region (code at 0x00220e) :
      * ..00.... : "Dash Yarou"
      *  else    : "Rally Bike"
  - Notice screen relies on bits 4 and 5 of the region (code at 0x001ac0) :
      * ..00.... : "FOR USE IN JAPAN ONLY"
      *  else    : no notice screen
  - Copyright relies on bits 4 and 5 of the region (code at 0x001e68) :
      * ..00.... : "TAITO CORPORATION" / "ALL RIGHTS RESERVED"
      * ..01.... : "TAITO AMERICA CORP." / "ALL RIGHTS RESERVED"
      * ..10.... : "TAITO CORP. JAPAN" / "ALL RIGHTS RESERVED"
      * ..11.... : "TAITO AMERICA CORP." / "LICENCED TO ROMSTAR"
  - Number of letters for initials relies on bits 4 and 5 of the region
    (code at 0x0008fe = init - code at 0x0022e8 = enter) :
      * ..00.... : 6 letters
      *  else    : 3 letters
  - To enter the "test mode", press START1 when the grid is displayed.
  - When "TEST" Switch is ON, you can do the following with the STARTn buttons :
      * press START2 to pause game
      * press START1 to unpause game
      * when START1 and START2 are pressed, the game enters in "slow motion" mode
  - When "TEST" Switch is ON, collision and fuel consuption routines are not called.
    Don't forget to turn the "TEST" Switch OFF when time is over on bonus stage,
    or the level will never end !
  - When cabinet is set to "Upright", you can use joystick and buttons from both players
    (code at 0x001c44).

2) 'truxton'

  - Region read from Territory Jumper (port 0x70 in CPU1) then stored at 0x8005 (CPU1 shared RAM) =
    0x18000a.w (CPU0 shared RAM) then stored at 0x081b7c.w .
  - Coinage relies on bits 0 and 1 of the region (code at 0x0ccc in CPU1) :
      * ......00 : TOAPLAN_COINAGE_JAPAN (table at 0x0d21 (COIN1 AND COIN2) in CPU1)
      * ......01 : TOAPLAN_COINAGE_JAPAN (table at 0x0d29 (COIN1 AND COIN2) in CPU1)
      * ......10 : TOAPLAN_COINAGE_WORLD (tables at 0x0d31 (COIN1) and 0x0d39 (COIN2) in CPU1)
      * ......11 : TOAPLAN_COINAGE_JAPAN (table at 0x0d21 (COIN1 AND COIN2) in CPU1)
  - Title screen relies on bits 0 to 2 of the region (code at 0x002c58) :
      * .....000 : "Tatsujin"
      *     else : "Truxton"
  - Notice screen relies on bits 0 to 2 of the region (code at 0x004eb0) :
      * ......1. : no notice screen
      * .....000 : "FOR USE IN JAPAN ONLY"
      *     else : "FOR USE IN U.S.A. ONLY"
  - Copyright relies on bits 0 to 2 of the region (code at 0x003050) :
      * .....000 : "TAITO CORPORATION" / "ALL RIGHTS RESERVED"
      * .....001 : "TAITO AMERICA CORP." / "LICENCED TO ROMSTAR FOR U.S.A."
      * .....01. : "TAITO CORPORATION" / "ALL RIGHTS RESERVED"
      * .....1.. : "TAITO AMERICA CORP."
  - Number of letters for initials relies on bits 0 to 2 of the region
    (code at 0x000976 = init - code at 0x0004a6 = enter) :
      * .....000 : 6 letters
      *     else : 3 letters
  - Jumper displayed in the Dip Switches screen relies on bits 0 and 1 of the region
    (code at 0x004546) :
      * ......00 : "FOR JAPAN"
      * ......01 : "FOR U.S.A."
      * ......1. : "FOR EUROPE"
    Jumpers 3 and 4 status is updated but they are always listed as unused.
  - To enter the "test mode", press START1 when the grid is displayed.
  - To enter the sound test, press START2 when the grid is displayed.
  - Set the "Service Mode" Dip Switch to ON while playing for invulnerability.
  - Set the "Dip Switch Display" Dip Switch to ON while playing to pause the game.
  - The "TEST" switch has the same effect as the "Service Mode" Dip Switch (DSWA bit 2).
  - When cabinet is set to "Upright", you can use joystick and buttons from both players
    (code at 0x002856).


3) 'hellfire' and "clones"

  - The "TEST" switch has the same effect as the "Service Mode" Dip Switch (DSWA bit 2).

3a) 'hellfire'

  - Region read from Territory Jumper (port 0x20 in CPU1) then stored at 0x8005 (CPU1 shared RAM) =
    0x0c000a.w (CPU0 shared RAM) then stored at 0x042414.w .
  - Coinage relies on bits 0 and 1 of the region (code at 0x0bc9 in CPU1) :
      * ......00 : TOAPLAN_COINAGE_JAPAN (table at 0x0c1e (COIN1 AND COIN2) in CPU1)
      * ......01 : TOAPLAN_COINAGE_JAPAN (table at 0x0c26 (COIN1 AND COIN2) in CPU1)
      * ......1. : TOAPLAN_COINAGE_WORLD (tables at 0x0c2e (COIN1) and 0x0c36 (COIN2) in CPU1)
  - Notice screen relies on bit 0 of the region (code at 0x000600) :
      * .......0 : "FOR USE IN JAPAN ONLY"
      * .......1 : "FOR USE IN U.S.A. ONLY"
    But this routine is only called if both bits 0 and 1 of the region are set to 0
    (code at 0x0005bc), so there is a notice screen only when the region is set to "Japan".
  - Copyright always displays "@ TOAPLAN CO. LTD. 1989" but the second line relies on
    bits 0 and 1 of the region (code at 0x0075dc) :
      * ......01 : "LICENSED TO TAITO AMERICA CORPORATION"
      * ....else : "LICENSED TO TAITO CORPORATION"
  - Number of letters for initials is hard-coded to 3 letters (display ". . .").
  - Jumper displayed in the Dip Switches screen relies on bits 0 and 1 of the region
    (code at 0x0009d8) :
      * ......00 : "FOR JAPAN"
      * ......01 : "FOR U.S.A."
      * ......1. : "FOR EUROPE"
  - When "Invulnerability" Dip Switch is ON, you can do the following with the STARTn buttons :
      * press START2 to pause game
      * press START1 to unpause game
      * when START1 and START2 are pressed, the game enters in "slow motion" mode
  - DSWA bit 0 ("Cabinet" in the 1P sets) and DSWB bit 7 ("Allow Continue" in the 1P sets)
    are both unused (they are not even tested).

3b) 'hellfire1'

  - Region read from Territory Jumper (port 0x20 in CPU1) then stored at 0x8005 (CPU1 shared RAM) =
    0x0c000a.w (CPU0 shared RAM) then stored at 0x04222e.w .
  - Same sound CPU as in 'hellfire', so same coinage infos.
  - Notice screen relies on bits 0 and 1 of the region (code at 0x0005b0) :
      * ......1. : no notice screen
      * ......00 : "FOR USE IN JAPAN ONLY"
      * ......01 : "FOR USE IN U.S.A. ONLY"
  - Copyright does NOT rely on the region, it is hard-coded in the M68000 ROMS.
  - Number of letters for initials relies on bit 0 of the region (code at 0x002aec) :
      * .......0 : 6 letters
      * .......1 : 3 letters
  - Jumper displayed in the Dip Switches screen relies on bits 0 and 1 of the region
    (code at 0x0009c4), but some data has been altered not to display "FOR U.S.A."
    So you get the following :
      * ......0. : "FOR JAPAN"
      * ......1. : "FOR EUROPE"
  - When "Invulnerability" Dip Switch is ON, you can do the following with the STARTn buttons :
      * press START2 to pause game
      * press START1 to unpause game
      * when START1 and START2 are pressed, the game enters in "slow motion" mode
  - When cabinet is set to "Upright", you can use joystick and buttons from both players
    (code at 0x0066ba).

3c) 'hellfire2a'

  - Region read from Territory Jumper (port 0x20 in CPU1) then stored at 0x8005 (CPU1 shared RAM) =
    0x0c000a.w (CPU0 shared RAM) then stored at 0x042414.w .
  - Same sound CPU as in 'hellfire', so same coinage infos.
  - Notice screen relies on bits 0 and 1 of the region (code at 0x0005c0) :
      * ......1. : no notice screen
      * ......00 : "FOR USE IN JAPAN ONLY"
      * ......01 : "FOR USE IN U.S.A. ONLY"
    However, because of the 'bra' instruction at 0x00059c, there is never a notice screen !
  - Copyright does NOT rely on the region, it is hard-coded in the M68000 ROMS.
  - Number of letters for initials relies on bit 1 of the region in the init routine (code at 0x000fce) :
      * ......0. : 6 letters
      * ......1. : 3 letters
    But there are no more tests on the region and only 3 letters can be entered (display ". . .") .
  - Jumper displayed in the Dip Switches screen relies on bits 0 and 1 of the region
    (code at 0x0009d0) :
      * ......00 : "FOR JAPAN"
      * ......01 : "FOR U.S.A."
      * ......1. : "FOR EUROPE"
  - There is no "Invulnerability" Dip Switch, but when bit 2 of RAM address 0x042685.b is set
    you can't die, and when bit 0 of the same address is set you can pause and enter slow motion.
    However, I can't find any condition that causes either of these bits to be set :(
  - Like older games (from Flying Shark to Truxton) "service mode" shows only a grid with colors,
    and there is a separate "Dip Switch Display" (DSWB bit 6).
  - To enter the "test mode", press START1 when the grid is displayed.
  - To enter the sound test, press START2 when the grid is displayed.
  - DSWA bit 0 ("Cabinet" in the 1P sets) and DSWB bit 7 ("Allow Continue" in the 1P sets)
    are both unused. However, in the Dip Switches screen they are both listed as used !
  - The other sets have checksums near the end of the M68000 program ROMs starting at 0x03fff0,
    but in 'hellfire2a' the "checksums" are 0xffffffff ! However, because of the 'ori #$4, SR'
    instruction at 0x000782, the ROM checksum test always "passes".
  - Based on the incorrect Dip Switch Display, the absent ROM checksums and patched-out ROM test,
    I wonder if this is some kind of prototype or test version.

3d) 'hellfire1a'

  - Region read from Territory Jumper (port 0x20 in CPU1) then stored at 0x8005 (CPU1 shared RAM) =
    0x0c000a.w (CPU0 shared RAM) then stored at 0x04222e.w .
  - Coinage relies on bits 0 and 1 of the region (code at 0x0bb7 in CPU1) :
      * ......00 : TOAPLAN_COINAGE_JAPAN (table at 0x0c0c (COIN1 AND COIN2) in CPU1)
      * ......01 : TOAPLAN_COINAGE_JAPAN (table at 0x0c14 (COIN1 AND COIN2) in CPU1)
      * ......10 : TOAPLAN_COINAGE_WORLD (tables at 0x0c1c (COIN1) and 0x0c24 (COIN2) in CPU1)
      * ......11 : TOAPLAN_COINAGE_JAPAN (table at 0x0c0c (COIN1 AND COIN2) in CPU1)
  - Notice screen relies on bits 0 and 1 of the region (code at 0x0005b0) :
      * ......1. : no notice screen
      * ......00 : "FOR USE IN JAPAN ONLY"
      * ......01 : "FOR USE IN U.S.A. ONLY"
  - Copyright does NOT rely on the region, it is hard-coded in the M68000 ROMS.
  - Number of letters for initials relies on bits 0 and 1 of the region, but it is buggy :
    in the init routine (code at 0x001386), bit 1 is tested (0 = 6 letters - 1 = 3 letters),
    but in the enter routine (code at 0x002c98), bit 0 is tested ! So you get the following :
      * ......00 : 6 letters with default high-scores initials filled with "......"
      * ......01 : 3 letters with default high-scores initials filled with "   ..."
      * ......10 : 6 letters with default high-scores initials filled with "...000"
      * ......11 : 3 letters with default high-scores initials filled with "   ..."
  - Jumper displayed in the Dip Switches screen relies on bits 0 and 1 of the region
    (code at 0x0009ba) :
      * ......00 : "FOR JAPAN"
      * ......01 : "FOR U.S.A."
      * ......1. : "FOR EUROPE"
  - There is no "Invulnerability" Dip Switch, but when bit 2 of RAM address 0x0424b1.b is set
    you can't die, and when bit 0 of the same address is set you can pause and enter slow motion.
    However, I can't find any condition that causes either of these bits to be set :(
  - Like older games (from Flying Shark to Truxton) "service mode" shows only a grid with colors,
    and there is a separate "Dip Switch Display" (DSWB bit 6).
  - To enter the "test mode", press START1 when the grid is displayed.
  - To enter the sound test, press START2 when the grid is displayed.
  - When cabinet is set to "Upright", you can use joystick and buttons from both players
    (code at 0x0068ec).
  - The other sets have 29 sounds you can play in the sound test, but 'hellfire1a' has only 28.
    In the other sets "sound number 14" is a sound effect and "sound number 15" is the first BGM,
    but 'hellfire1a' is missing that sound effect and instead "sound number 14" is the first BGM !
  - The slightly different coinage and the "missing" sound effect account for the differences in
    the Z80 code between 'hellfire1a' and the other sets.


4) 'zerowing' and "clones"

  - When "Invulnerability" Dip Switch is ON, you can do the following with the STARTn buttons :
      * press START2 to pause game
      * press START1 to unpause game
      * when START1 and START2 are pressed, the game enters in "slow motion" mode
  - When "Invulnerability" Dip Switch is ON, you can't die (of course), but you also can't move
    nor shoot while "captured" by an enemy or the background ! So you have to wait until enemy
    gives up or background scrolls enough to "free" you.
  - The "TEST" switch has the same effect as the "Service Mode" Dip Switch (DSWA bit 2).

4a) 'zerowing'

  - Region read from Territory Jumper (port 0x70 in CPU1) then stored at 0x8005 (CPU1 shared RAM) =
    0x44000a.w (CPU0 shared RAM) then stored at 0x081810.w .
  - Coinage relies on bits 0 and 1 of the region (code at 0x0c59 in CPU1) :
      * ......00 : TOAPLAN_COINAGE_JAPAN (table at 0x0cae (COIN1 AND COIN2) in CPU1)
      * ......01 : TOAPLAN_COINAGE_JAPAN (table at 0x0cb6 (COIN1 AND COIN2) in CPU1)
      * ......1. : TOAPLAN_COINAGE_WORLD (tables at 0x0cbe (COIN1) and 0x0cc6 (COIN2) in CPU1)
  - Notice screen relies on bit 0 of the region (code at 0x000564) :
      * .......0 : "FOR USE IN JAPAN ONLY"
      * .......1 : "FOR USE IN U.S.A. ONLY"
    But this routine is only called if both bits 0 and 1 of the region are set to 0
    (code at 0x000530), so there is a notice screen only when the region is set to "Japan".
  - Copyright does NOT rely on the region, it is hard-coded in the M68000 ROMS.
  - Number of letters for initials relies on bits 0 and 1 of the region, but it is buggy :
    in the init routine (code at 0x000de0), bit 1 is tested (0 = 6 letters - 1 = 3 letters),
    but in the enter routine (code at 0x001bf4), bit 0 is tested ! So you get the following :
      * ......00 : 6 letters with default high-scores initials filled with "......"
      * ......01 : 3 letters with default high-scores initials filled with "   ..."
      * ......10 : 6 letters with default high-scores initials filled with "...000"
      * ......11 : 3 letters with default high-scores initials filled with "   ..."
  - Jumper displayed in the Dip Switches screen relies on bits 0 and 1 of the region
    (code at 0x000922) :
      * ......00 : "FOR JAPAN"
      * ......01 : "FOR U.S.A."
      * ......1. : "FOR EUROPE"
  - When cabinet is set to "Upright", you can use joystick and buttons from both players
    (code at 0x00541a).

4b) 'zerowing2'

  - Region read from Territory Jumper (port 0x70 in CPU1) then stored at 0x8005 (CPU1 shared RAM) =
    0x44000a.w (CPU0 shared RAM) then stored at 0x081ae2.w .
  - Same sound CPU as in 'zerowing', so same coinage infos.
  - Notice screen relies on bits 0 and 1 of the region (code at 0x0005c2) :
      * ......1. : no notice screen
      * ......00 : "FOR USE IN JAPAN ONLY"
      * ......01 : "FOR USE IN U.S.A. ONLY"
    However, because of the 'bra' instruction at 0x00059e, there is never a notice screen !
  - Copyright does NOT rely on the region, it is hard-coded in the M68000 ROMS.
    It is different from the one in 'zerowing' though.
  - Number of letters for initials relies on bit 1 of the region in the init routine (code at 0x000e64) :
      * ......0. : 6 letters
      * ......1. : 3 letters
    But there are no more tests on the region and only 3 letters can be entered (display ". . .") .
  - Jumper displayed in the Dip Switches screen relies on bits 0 and 1 of the region
    (code at 0x0009b2), but some data has been altered not to display "FOR JAPAN"
    So you get the following :
      * ......0. : "FOR U.S.A."
      * ......1. : "FOR EUROPE"
  - DSWA bit 0 ("Cabinet" in the 1P set) and DSWB bit 7 ("Allow Continue" in the 1P set)
    are both unused (they are not even tested).
  - Here are some differences I noticed with 'zerowing' :
      * you get the twin ships when starting a new life
      * speed range is 0x14-0x2c instead of 0x10-0x30 (but still +0x08)
    There might be some other differences, but as the M68000 code is heavily modified and as
    many addresses in RAM are different, it isn't very easy to spot which ones :( Any help is welcome !


5) 'demonwld' and "clones"

    TO DO !


6) 'fireshrk' and "clones"

    TO DO !


7) 'outzone' and "clones"

  - The "TEST" switch has the same effect as the "Invulnerability" Dip Switch (DSWB bit 6).

7a) 'outzone'

  - Region read from Territory Jumper (port 0x1c in CPU1) then stored at 0x8005 (CPU1 shared RAM) =
    0x14000a.w (CPU0 shared RAM) then 0x2401ca.w is set based on bits 0 to 3 of the region
    (code at 0x013b68):
      * ....0000 : 0x2401ca.w = 0000 (display Japanese story demo, etc.)
      * ....0001 : 0x2401ca.w = 0001 (display FBI logo)
      * ....0010 : 0x2401ca.w = 0002
      * ....0011 : 0x2401ca.w = 0003
      * ....0100 : 0x2401ca.w = 0004
      * ....0101 : 0x2401ca.w = 0005
      * ....0110 : 0x2401ca.w = 0006
      * ....0111 : 0x2401ca.w = 0007 (display FBI logo)
      * ....1000 : 0x2401ca.w = 0008
      *     else : 0x2401ca.w = 0000 (display Japanese story demo, etc.)
    This RAM address is also checked in many other places in the M68000 code.
  - Coinage relies on bits 0 to 3 of the region (code at 0x0e77 in CPU1) :
      * ....0001 : TOAPLAN_COINAGE_JAPAN (table at 0x0f04 (COIN1 AND COIN2) in CPU1)
      * ....0010 : TOAPLAN_COINAGE_WORLD (tables at 0x0f0c (COIN1) and 0x0f14 (COIN2) in CPU1)
      *     else : TOAPLAN_COINAGE_JAPAN (table at 0x0efc (COIN1 AND COIN2) in CPU1)
  - Notice screen relies on region stored at 0x2401ca.w (code at 0x013bbc) :
      * 0000 : "JAPAN ONLY"
      * 0001 : "U.S.A. ONLY"
      * 0002 : "EUROPE ONLY"
      * 0003 : "HONG KONG ONLY"
      * 0004 : "KOREA ONLY"
      * 0005 : "TAIWAN ONLY"
      * 0006 : "TAIWAN ONLY"
      * 0007 : "U.S.A. ONLY"
      * 0008 : "HONG KONG & CHINA"
  - Copyright relies on region stored at 0x2401ca.w (code at 0x0050b4 - table at 0x0050e8) :
      * 0000 : "@ TOAPLAN CO.,LTD. 1990" / "ALL RIGHTS RESERVED" / "" / ""
      * 0001 : "@1990 TOAPLAN CO.,LTD." / "ALL RIGHTS RESERVED" / "" / ""
      * 0002 : "@1990 TOAPLAN CO.,LTD." / "ALL RIGHTS RESERVED" / "" / ""
      * 0003 : "@1990 TOAPLAN CO.,LTD." / "ALL RIGHTS RESERVED" / "" / ""
      * 0004 : "@1990 TOAPLAN CO.,LTD." / "ALL RIGHTS RESERVED" / "" / ""
      * 0005 : "@1990 TOAPLAN CO.,LTD." / "ALL RIGHTS RESERVED" / "" / ""
      * 0006 : "@1990 TOAPLAN CO.,LTD." / "LICENSED TO" / "SPACY CO.,LTD." / "FOR TAIWAN"
      * 0007 : "@1990 TOAPLAN CO.,LTD." / "LICENSED TO" / "ROMSTAR, INC." / "FOR U.S.A."
      * 0008 : "@1990 TOAPLAN CO.,LTD." / "LICENSED TO" / "HONEST TRADING CO." / "FOR HONG KONG & CHINA"
    If 0x2401ca.w >= 0006, the Toaplan logo is moved up to make room for the additional lines of text.

7b) 'outzoneh'

  - Same sound CPU as in 'outzone', so same coinage infos.
  - Same notice screen and copyright infos as 'outzone'.
  - This set is very similar to 'outzone'. I have found the following differences:
  - Table at 0x000f70 contains data related to the "Difficulty" Dip Switches:
       DSWB & 0x3  'outzoneh'  'outzone' and others
       ----------  ----------  --------------------
      * ......00       04          02
      * ......01       00          00
      * ......10       08          04
      * ......11       0C          08
    So "Normal" difficulty in 'outzoneh' is equivalent to "Hard" in the other sets,
    "Hard" is equivalent to "Very Hard" in the other sets, and "Very Hard" is even
    more difficult!
  - Code in these routines is different from 'outzone' and the other sets:
       'outzoneh'  'outzone'  When executed?
       ----------  ---------  --------------
      * 0x005570   0x005570   once per spawn of first enemy type in game (brown with red 'gun' and blue 'eyes')
      * 0x005800   0x005810   once per spawn of another enemy type (near end of stage 1, not sure which one)
      * 0x00c140   0x00c160   once per spawn of many enemy types
      * 0x00c1e0   0x00c214   once per frame while fighting stage bosses
  - In the first three routines, the code in 'outzoneh' is shorter than the code in 'outzone'
    and the other sets. It looks like the other sets check some conditions (stage number,
    play time, etc.) to decide whether to spawn a weaker or stronger version of the enemy,
    while 'outzoneh' skips some of those checks and spawns the stronger version more often.
  - For the fourth routine, all sets except 'outzoneh' have 'rts' as the first instruction,
    so 'outzoneh' is the only set where this routine actually does anything.
    However, other than somehow relating to bosses, I don't yet know what it does :(
  - Because the player input data for the demonstrations isn't changed to compensate
    for the higher difficulty, the player dies almost immediately in the second demo.
  - Due to the shorter code in 'outzoneh', all code and data between 0x005578 and 0x0142a0
    is shifted, so there may be other differences I have overlooked. In particular, the
    special item the player picks up in the first demo is different, and I haven't found
    exactly why. Special items seem to be affected by a pseudorandom number generator,
    so the different item could merely be due to different execution timing between sets.

7c) 'outzonea'

  - Same sound CPU as in 'outzone', so same coinage infos.
  - This set is almost identical to 'outzone', with only two differences :
  - The 'jmp' instruction at 0x0003e6 goes to an invalid instruction, causing a crash
    if you set the "Service Mode" Dip Switch to ON while the game is running.
    'outzonea' is the only set with this bug, the other four sets all correctly
    go to service mode.
  - Notice and copyright for region 0008 say "HONG KONG" instead of "HONG KONG & CHINA".
    Because of the shorter text, data between 0x015dd1 and 0x016f73 is shifted.

7d) 'outzoneb'

  - Different sound CPU program from 'outzone', but same coinage infos.
  - Sound data in the sound CPU ROM is different. Because the data in 'outzoneb' is shorter,
    all data and code after 0x1ac3 is shifted. It looks like the actual changes are in data
    used by sounds 2, 8, and 19. However, I can't hear any obvious differences :(
  - This set's sound CPU ROM lacks a checksum. However, because of the 'xor a' instruction
    at 0x50e6, the ROM checksum test always "passes".
  - Same notice screen and copyright infos as 'outzonea'.
  - This set has many M68000 code and data differences from the other sets, too many
    to list one by one as with 'outzoneh'. Many RAM addresses used are also different.
  - Spelling error on the Sound Check screen: "BUTTAN" instead of "BUTTON".
  - This set and 'outzonec' have a hidden use for the two "Unused" Dip Switches.
    If DSWA bit 0 ("Unused") and DSWB bit 7 (also "Unused") are both set to ON and
    P2 joystick is held DOWN during startup, the CRTC registers are programmed for a
    smaller VTOTAL than usual, giving a higher frame rate but cutting off the edges
    of the screen (the effect of this isn't correctly emulated yet)
  - Likewise, if DSWA bit 0 is ON and DSWB bit 7 is OFF, the game never starts up
    (it seems to infinitely repeat one of the RAM tests)

7e) 'outzonec'

  - Region read from Territory Jumper (port 0x1c in CPU1) then stored at 0x8005 (CPU1 shared RAM) =
    0x14000a.w (CPU0 shared RAM), then 0x2401ca.w is set based on bits 0 to 2 of the region
    (code at 0x012742) :
      * .....000 : 0x2401ca.w = 0000 (display Japanese story demo, etc.)
      * .....001 : 0x2401ca.w = 0001 (display FBI logo)
      * .....002 : 0x2401ca.w = 0002
      *     else : 0x2401ca.w = 0003
    This RAM address is also checked in many other places in the M68000 code.
  - Same coinage infos as 'outzone', but see below about the sound CPU.
  - Notice screen relies directly on bits 0 to 2 of the region stored at 0x14000a.w,
    NOT on 0x2401ca.w (code at 0x01277a):
      * .....000 : "JAPAN ONLY"
      * .....001 : "U.S.A. ONLY"
      * .....010 : "EUROPE ONLY"
      * .....011 : "HONG KONG ONLY"
      * .....100 : "KOREA ONLY"
      * .....101 : "TAIWAN ONLY"
      *     else : no notice screen
  - Copyright does NOT rely on the region, it is hard-coded in the M68000 ROMS.
  - This set has by far the most code and data differences from any of the other sets.
    However, it is slightly more similar to 'outzoneb' than to the other three.
  - Same "BUTTAN" spelling error on the Sound Check screen as 'outzoneb'.
  - Same use for the "unused" Dip Switches as 'outzoneb'.
  - Many of the sound commands the M68000 sends to the Z80 are shifted by 2
    compared to the other sets, e.g.:
        Sound effect         'outzonec'  'outzone' and others
        ------------         ----------  --------------------
      * Japanese story text   no sound      0x26
      * player 8-way shot       0x26        0x28
      * player 3-way shot       0x27        0x29
      * title logo letters      0x2a        0x2c
    For this reason this set plays many different (and strange/inappropriate)
    sound effects from the other sets. This is probably not intentional, but rather
    this set probably should have a different sound CPU ROM, like 'hellfire1a' does.
    The M68000 code in this set is definitely older than 'outzoneb', but the Z80 code
    is newer than 'outzoneb', which seems very unlikely to be correct.
    Most likely the board it came from was either bootlegged or repaired by someone
    who used the wrong Z80 ROM and didn't notice or care that the sounds were wrong.
    Because of this I've tagged the Z80 ROM as a BAD_DUMP and tagged this set with
    MACHINE_IMPERFECT_SOUND until the correct Z80 ROM is found and dumped.


8) 'vimana' and "clones"

8a) 'vimana'

  - Region read from Territory Jumper (0x440011.b).
  - Coinage relies on bits 0 to 3 of the region :
      * ....0010 : TOAPLAN_COINAGE_WORLD
      *     else : TOAPLAN_COINAGE_JAPAN
    This a guess based on the "test mode" (code at 0x01a804) because of the missing Z180 CPU.
  - Notice screen relies on bits 0 to 3 of the region (code at 0x018bf2 - table at 0x019736) :
      * ....0000 : "JAPAN ONLY"
      * ....0001 : "U.S.A. ONLY"
      * ....0010 : "EUROPE ONLY"
      * ....0011 : "HONG KONG ONLY"
      * ....0100 : "KOREA ONLY"
      * ....0101 : "TAIWAN ONLY"
      * ....0110 : "TAIWAN ONLY"
      * ....0111 : "U.S.A. ONLY"
      * ....1000 : "HONG KONG ONLY"
      * ....1001 : ""
      * ....1010 : ""
      * ....1011 : ""
      * ....1100 : ""
      * ....1101 : ""
      * ....1110 : ""
      * ....1111 : "JAPAN ONLY"
  - Copyright always displays "@ TOAPLAN CO. LTD. 1991" but the other lines rely on
    bits 0 to 3 of the region (code at 0x016512 - tables at 0x01948e and 0x019496) :
      * ....0000 : "DISTRIBUTED BY" / ""
      * ....0001 : "ALL RIGHTS RESERVED" / ""
      * ....0010 : "ALL RIGHTS RESERVED" / ""
      * ....0011 : "ALL RIGHTS RESERVED" / ""
      * ....0100 : "ALL RIGHTS RESERVED" / ""
      * ....0101 : "ALL RIGHTS RESERVED" / ""
      * ....0110 : "LICENCED TO SPACY CO., LTD" / "FOR TAIWAN"
      * ....0111 : "LICENCED TO ROMSTAR, INC." / "FOR U.S.A."
      * ....1000 : "LICENCED TO HONEST TRADING CO." / "FOR HONG KONG AND CHINA."
      * ....1001 : "" / ""
      * ....1010 : "" / ""
      * ....1011 : "" / ""
      * ....1100 : "" / ""
      * ....1101 : "" / ""
      * ....1110 : "" / ""
      * ....1111 : "DISTRIBUTED BY" / ""
    An additional Tecmo logo also relies on bits 0 to 3 of the region :
      * ....0000 : display
      * ....1111 : display
      *     else : no display
    So the Tecmo logo is only displayed when the region is set to "Japan" (right to the
    "DISTRIBUTED BY" text).
  - FBI logo (after displaying the hi-scores) relies on bits 0 to 3 of the region
    (code at 0x0163f4) :
      * ....0001 : display
      * ....0111 : display
      *     else : no display
    So the FBI logo is only displayed when the region is set to "USA" or "USA (Romstar)".
  - Number of letters for initials is hard-coded to 3 letters.
  - Jumper displayed in the Dip Switches screen relies on bits 0 to 3 of the region
    (code at 0x01a89c - tables at 0x019d1e) :
      * ....0000 : "FOR JAPAN.   "
      * ....0001 : "FOR U.S.A.   "
      * ....0010 : "FOR EUROPE   "
      * ....0011 : "FOR HONG KONG"
      * ....0100 : "FOR KOREA    "
      * ....0101 : "FOR TAIWAN   "
      * ....0110 : "FOR TAIWAN   "
      * ....0111 : "FOR U.S.A.   "
      * ....1000 : "FOR HONG KONG"
      * ....1001 : "             "
      * ....1010 : "             "
      * ....1011 : "             "
      * ....1100 : "             "
      * ....1101 : "             "
      * ....1110 : "             "
      * ....1111 : "FOR JAPAN.   "
  - When "Invulnerability" Dip Switch is ON, you can do the following with the STARTn buttons :
      * press START2 to pause game
      * press START1 to unpause game
      * when START1 and START2 are pressed, the game enters in "slow motion" mode
  - When "Invulnerability" Dip Switch is ON, you can press F1 ("Fast Scrolling", in fact
    the unused 3rd button of player 1) to advance quickly to your desired area.
  - When "Invulnerability" Dip Switch is ON, you can press the unused 3rd button of player 2,
    but as its effect is completely unknown (code at 0x0010aa), I've decided not to map it for now.
  - It's hard to tell which regions can be used for this set and which one is the default one.
    However, as the text at the "end" (after the "CONGRATULATIONS" message) is in English,
    I've decided to disable the ones related to Japan (0x00 and 0x0f).
  - Routine at 0x017c18 is sound related and is seems to share memory with the Z180.
    Unfortunately I haven't been able to understand yet how to produce a sound :(
    Routine at 0x017ca6 stops producing a sound by reseting values in shared memory with the Z180.

8b) 'vimanan'

  - The only difference with 'vimana' is the different copyright when region is set to Europe :
      * ....0010 : "NOVA APPARATE GMBH & CO" / ""
    Because of this additional text, other texts, code and data are shifted after 0x019e9c.

8c) 'vimanaj'

  - The only difference I've noticed with 'vimana' is the text at the "end" is in Japanese.
    This is why I've disabled all regions which aren't related to Japan.
  - Sound routines at 0x01792c and 0x0179ba.


To Do:
    Add support for HD647180 (Z180) sound CPUs (once their internal ROMS are dumped).
    These are:
        Fire Shark / Same! Same! Same!
        Vimana
    In the meantime, it can be interesting to simulate the basic communications (coinage and credits)
    between the M68000 and the Z180.  [stephh]

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z180/z180.h"
#include "cpu/tms32010/tms32010.h"
#include "includes/toaplipt.h"
#include "includes/toaplan1.h"
#include "sound/3812intf.h"


/***************************** 68000 Memory Map *****************************/

static ADDRESS_MAP_START( rallybik_main_map, AS_PROGRAM, 16, toaplan1_rallybik_state )
	AM_RANGE(0x000000, 0x00ffff) AM_ROM
	AM_RANGE(0x040000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x083fff) AM_RAM
	AM_RANGE(0x0c0000, 0x0c0fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x100000, 0x100001) AM_WRITE(toaplan1_bcu_flipscreen_w)
	AM_RANGE(0x100002, 0x100003) AM_READWRITE(toaplan1_tileram_offs_r, toaplan1_tileram_offs_w)
	AM_RANGE(0x100004, 0x100007) AM_READWRITE(rallybik_tileram16_r, toaplan1_tileram16_w)
	AM_RANGE(0x100010, 0x10001f) AM_READWRITE(toaplan1_scroll_regs_r, toaplan1_scroll_regs_w)
	AM_RANGE(0x140000, 0x140001) AM_READ_PORT("VBLANK")
//  AM_RANGE(0x140000, 0x140001) AM_WRITE(?? video frame related ??)
	AM_RANGE(0x140002, 0x140003) AM_WRITE(toaplan1_intenable_w)
	AM_RANGE(0x140008, 0x14000f) AM_WRITE(toaplan1_bcu_control_w)
	AM_RANGE(0x144000, 0x1447ff) AM_RAM_WRITE(toaplan1_bgpalette_w) AM_SHARE("bgpalette")
	AM_RANGE(0x146000, 0x1467ff) AM_RAM_WRITE(toaplan1_fgpalette_w) AM_SHARE("fgpalette")
	AM_RANGE(0x180000, 0x180fff) AM_READWRITE(toaplan1_shared_r, toaplan1_shared_w)
	AM_RANGE(0x1c0000, 0x1c0003) AM_WRITE(toaplan1_tile_offsets_w)
	AM_RANGE(0x1c8000, 0x1c8001) AM_WRITE(toaplan1_reset_sound_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( truxton_main_map, AS_PROGRAM, 16, toaplan1_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x080000, 0x083fff) AM_RAM
	AM_RANGE(0x0c0000, 0x0c0001) AM_READ(toaplan1_frame_done_r)
	AM_RANGE(0x0c0002, 0x0c0003) AM_READWRITE(toaplan1_spriteram_offs_r, toaplan1_spriteram_offs_w)
	AM_RANGE(0x0c0004, 0x0c0005) AM_READWRITE(toaplan1_spriteram16_r, toaplan1_spriteram16_w)
	AM_RANGE(0x0c0006, 0x0c0007) AM_READWRITE(toaplan1_spritesizeram16_r, toaplan1_spritesizeram16_w)
	AM_RANGE(0x100000, 0x100001) AM_WRITE(toaplan1_bcu_flipscreen_w)
	AM_RANGE(0x100002, 0x100003) AM_READWRITE(toaplan1_tileram_offs_r, toaplan1_tileram_offs_w)
	AM_RANGE(0x100004, 0x100007) AM_READWRITE(toaplan1_tileram16_r, toaplan1_tileram16_w)
	AM_RANGE(0x100010, 0x10001f) AM_READWRITE(toaplan1_scroll_regs_r, toaplan1_scroll_regs_w)
	AM_RANGE(0x140000, 0x140001) AM_READ_PORT("VBLANK")
//  AM_RANGE(0x140000, 0x140001) AM_WRITE(?? video frame related ??)
	AM_RANGE(0x140002, 0x140003) AM_WRITE(toaplan1_intenable_w)
	AM_RANGE(0x140008, 0x14000f) AM_WRITE(toaplan1_bcu_control_w)
	AM_RANGE(0x144000, 0x1447ff) AM_RAM_WRITE(toaplan1_bgpalette_w) AM_SHARE("bgpalette")
	AM_RANGE(0x146000, 0x1467ff) AM_RAM_WRITE(toaplan1_fgpalette_w) AM_SHARE("fgpalette")
	AM_RANGE(0x180000, 0x180fff) AM_READWRITE(toaplan1_shared_r, toaplan1_shared_w)
	AM_RANGE(0x1c0000, 0x1c0003) AM_WRITE(toaplan1_tile_offsets_w)
	AM_RANGE(0x1c0006, 0x1c0007) AM_WRITE(toaplan1_fcu_flipscreen_w)
	AM_RANGE(0x1d0000, 0x1d0001) AM_WRITE(toaplan1_reset_sound_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( hellfire_main_map, AS_PROGRAM, 16, toaplan1_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x040000, 0x047fff) AM_RAM
	AM_RANGE(0x080000, 0x080001) AM_READ_PORT("VBLANK")
//  AM_RANGE(0x080000, 0x080001) AM_WRITE(?? video frame related ??)
	AM_RANGE(0x080002, 0x080003) AM_WRITE(toaplan1_intenable_w)
	AM_RANGE(0x080008, 0x08000f) AM_WRITE(toaplan1_bcu_control_w)
	AM_RANGE(0x084000, 0x0847ff) AM_RAM_WRITE(toaplan1_bgpalette_w) AM_SHARE("bgpalette")
	AM_RANGE(0x086000, 0x0867ff) AM_RAM_WRITE(toaplan1_fgpalette_w) AM_SHARE("fgpalette")
	AM_RANGE(0x0c0000, 0x0c0fff) AM_READWRITE(toaplan1_shared_r, toaplan1_shared_w)
	AM_RANGE(0x100000, 0x100001) AM_WRITE(toaplan1_bcu_flipscreen_w)
	AM_RANGE(0x100002, 0x100003) AM_READWRITE(toaplan1_tileram_offs_r, toaplan1_tileram_offs_w)
	AM_RANGE(0x100004, 0x100007) AM_READWRITE(toaplan1_tileram16_r, toaplan1_tileram16_w)
	AM_RANGE(0x100010, 0x10001f) AM_READWRITE(toaplan1_scroll_regs_r, toaplan1_scroll_regs_w)
	AM_RANGE(0x140000, 0x140001) AM_READ(toaplan1_frame_done_r)
	AM_RANGE(0x140002, 0x140003) AM_READWRITE(toaplan1_spriteram_offs_r, toaplan1_spriteram_offs_w)
	AM_RANGE(0x140004, 0x140005) AM_READWRITE(toaplan1_spriteram16_r, toaplan1_spriteram16_w)
	AM_RANGE(0x140006, 0x140007) AM_READWRITE(toaplan1_spritesizeram16_r, toaplan1_spritesizeram16_w)
	AM_RANGE(0x180000, 0x180003) AM_WRITE(toaplan1_tile_offsets_w)
	AM_RANGE(0x180006, 0x180007) AM_WRITE(toaplan1_fcu_flipscreen_w)
	AM_RANGE(0x180008, 0x180009) AM_WRITE(toaplan1_reset_sound_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( zerowing_main_map, AS_PROGRAM, 16, toaplan1_state )
	AM_RANGE(0x000000, 0x00ffff) AM_ROM
	AM_RANGE(0x040000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x087fff) AM_RAM
	AM_RANGE(0x0c0000, 0x0c0003) AM_WRITE(toaplan1_tile_offsets_w)
	AM_RANGE(0x0c0006, 0x0c0007) AM_WRITE(toaplan1_fcu_flipscreen_w)
	AM_RANGE(0x400000, 0x400001) AM_READ_PORT("VBLANK")
//  AM_RANGE(0x400000, 0x400001) AM_WRITE(?? video frame related ??)
	AM_RANGE(0x400002, 0x400003) AM_WRITE(toaplan1_intenable_w)
	AM_RANGE(0x400008, 0x40000f) AM_WRITE(toaplan1_bcu_control_w)
	AM_RANGE(0x404000, 0x4047ff) AM_RAM_WRITE(toaplan1_bgpalette_w) AM_SHARE("bgpalette")
	AM_RANGE(0x406000, 0x4067ff) AM_RAM_WRITE(toaplan1_fgpalette_w) AM_SHARE("fgpalette")
	AM_RANGE(0x440000, 0x440fff) AM_READWRITE(toaplan1_shared_r, toaplan1_shared_w)
	AM_RANGE(0x480000, 0x480001) AM_WRITE(toaplan1_bcu_flipscreen_w)
	AM_RANGE(0x480002, 0x480003) AM_READWRITE(toaplan1_tileram_offs_r, toaplan1_tileram_offs_w)
	AM_RANGE(0x480004, 0x480007) AM_READWRITE(toaplan1_tileram16_r, toaplan1_tileram16_w)
	AM_RANGE(0x480010, 0x48001f) AM_READWRITE(toaplan1_scroll_regs_r, toaplan1_scroll_regs_w)
	AM_RANGE(0x4c0000, 0x4c0001) AM_READ(toaplan1_frame_done_r)
	AM_RANGE(0x4c0002, 0x4c0003) AM_READWRITE(toaplan1_spriteram_offs_r, toaplan1_spriteram_offs_w)
	AM_RANGE(0x4c0004, 0x4c0005) AM_READWRITE(toaplan1_spriteram16_r, toaplan1_spriteram16_w)
	AM_RANGE(0x4c0006, 0x4c0007) AM_READWRITE(toaplan1_spritesizeram16_r, toaplan1_spritesizeram16_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( demonwld_main_map, AS_PROGRAM, 16, toaplan1_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x400000, 0x400001) AM_READ_PORT("VBLANK")
//  AM_RANGE(0x400000, 0x400001) AM_WRITE(?? video frame related ??)
	AM_RANGE(0x400002, 0x400003) AM_WRITE(toaplan1_intenable_w)
	AM_RANGE(0x400008, 0x40000f) AM_WRITE(toaplan1_bcu_control_w)
	AM_RANGE(0x404000, 0x4047ff) AM_RAM_WRITE(toaplan1_bgpalette_w) AM_SHARE("bgpalette")
	AM_RANGE(0x406000, 0x4067ff) AM_RAM_WRITE(toaplan1_fgpalette_w) AM_SHARE("fgpalette")
	AM_RANGE(0x600000, 0x600fff) AM_READWRITE(toaplan1_shared_r, toaplan1_shared_w)
	AM_RANGE(0x800000, 0x800001) AM_WRITE(toaplan1_bcu_flipscreen_w)
	AM_RANGE(0x800002, 0x800003) AM_READWRITE(toaplan1_tileram_offs_r, toaplan1_tileram_offs_w)
	AM_RANGE(0x800004, 0x800007) AM_READWRITE(toaplan1_tileram16_r, toaplan1_tileram16_w)
	AM_RANGE(0x800010, 0x80001f) AM_READWRITE(toaplan1_scroll_regs_r, toaplan1_scroll_regs_w)
	AM_RANGE(0xa00000, 0xa00001) AM_READ(toaplan1_frame_done_r)
	AM_RANGE(0xa00002, 0xa00003) AM_READWRITE(toaplan1_spriteram_offs_r, toaplan1_spriteram_offs_w)
	AM_RANGE(0xa00004, 0xa00005) AM_READWRITE(toaplan1_spriteram16_r, toaplan1_spriteram16_w)
	AM_RANGE(0xa00006, 0xa00007) AM_READWRITE(toaplan1_spritesizeram16_r, toaplan1_spritesizeram16_w)
	AM_RANGE(0xc00000, 0xc03fff) AM_RAM
	AM_RANGE(0xe00000, 0xe00003) AM_WRITE(toaplan1_tile_offsets_w)
	AM_RANGE(0xe00006, 0xe00007) AM_WRITE(toaplan1_fcu_flipscreen_w)
	AM_RANGE(0xe00008, 0xe00009) AM_WRITE(toaplan1_reset_sound_w)
	AM_RANGE(0xe0000a, 0xe0000b) AM_WRITE(demonwld_dsp_ctrl_w)  /* DSP Comms control */
ADDRESS_MAP_END

static ADDRESS_MAP_START( samesame_main_map, AS_PROGRAM, 16, toaplan1_state )
	AM_RANGE(0x000000, 0x00ffff) AM_ROM
	AM_RANGE(0x040000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x080003) AM_WRITE(toaplan1_tile_offsets_w)
	AM_RANGE(0x080006, 0x080007) AM_WRITE(toaplan1_fcu_flipscreen_w)
	AM_RANGE(0x0c0000, 0x0c3fff) AM_RAM         /* Frame done at $c1ada */
	AM_RANGE(0x100000, 0x100001) AM_READ_PORT("VBLANK")
//  AM_RANGE(0x100000, 0x100001) AM_WRITE(?? video frame related ??)
	AM_RANGE(0x100002, 0x100003) AM_WRITE(toaplan1_intenable_w)
	AM_RANGE(0x100008, 0x10000f) AM_WRITE(toaplan1_bcu_control_w)
	AM_RANGE(0x104000, 0x1047ff) AM_RAM_WRITE(toaplan1_bgpalette_w) AM_SHARE("bgpalette")
	AM_RANGE(0x106000, 0x1067ff) AM_RAM_WRITE(toaplan1_fgpalette_w) AM_SHARE("fgpalette")
	AM_RANGE(0x140000, 0x140001) AM_READ_PORT("P1")
	AM_RANGE(0x140002, 0x140003) AM_READ_PORT("P2")
	AM_RANGE(0x140004, 0x140005) AM_READ_PORT("DSWA")
	AM_RANGE(0x140006, 0x140007) AM_READ_PORT("DSWB")
	AM_RANGE(0x140008, 0x140009) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x14000a, 0x14000b) AM_READ(samesame_port_6_word_r)    /* Territory, and MCU ready */
	AM_RANGE(0x14000c, 0x14000d) AM_WRITE(samesame_coin_w)  /* Coin counter/lockout */
//  AM_RANGE(0x14000e, 0x14000f) AM_WRITE(samesame_mcu_w)   /* Commands sent to HD647180 */
	AM_RANGE(0x180000, 0x180001) AM_WRITE(toaplan1_bcu_flipscreen_w)
	AM_RANGE(0x180002, 0x180003) AM_READWRITE(toaplan1_tileram_offs_r, toaplan1_tileram_offs_w)
	AM_RANGE(0x180004, 0x180007) AM_READWRITE(toaplan1_tileram16_r, toaplan1_tileram16_w)
	AM_RANGE(0x180010, 0x18001f) AM_READWRITE(toaplan1_scroll_regs_r, toaplan1_scroll_regs_w)
	AM_RANGE(0x1c0000, 0x1c0001) AM_READ(toaplan1_frame_done_r)
//  AM_RANGE(0x1c0000, 0x1c0001) AM_WRITE(?? disable sprite refresh ??)
	AM_RANGE(0x1c0002, 0x1c0003) AM_READWRITE(toaplan1_spriteram_offs_r, toaplan1_spriteram_offs_w)
	AM_RANGE(0x1c0004, 0x1c0005) AM_READWRITE(toaplan1_spriteram16_r, toaplan1_spriteram16_w)
	AM_RANGE(0x1c0006, 0x1c0007) AM_READWRITE(toaplan1_spritesizeram16_r, toaplan1_spritesizeram16_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( outzone_main_map, AS_PROGRAM, 16, toaplan1_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x100001) AM_READ(toaplan1_frame_done_r)
	AM_RANGE(0x100002, 0x100003) AM_READWRITE(toaplan1_spriteram_offs_r, toaplan1_spriteram_offs_w)
	AM_RANGE(0x100004, 0x100005) AM_READWRITE(toaplan1_spriteram16_r, toaplan1_spriteram16_w)
	AM_RANGE(0x100006, 0x100007) AM_READWRITE(toaplan1_spritesizeram16_r, toaplan1_spritesizeram16_w)
	AM_RANGE(0x140000, 0x140fff) AM_READWRITE(toaplan1_shared_r, toaplan1_shared_w)
	AM_RANGE(0x200000, 0x200001) AM_WRITE(toaplan1_bcu_flipscreen_w)
	AM_RANGE(0x200002, 0x200003) AM_READWRITE(toaplan1_tileram_offs_r, toaplan1_tileram_offs_w)
	AM_RANGE(0x200004, 0x200007) AM_READWRITE(toaplan1_tileram16_r, toaplan1_tileram16_w)
	AM_RANGE(0x200010, 0x20001f) AM_READWRITE(toaplan1_scroll_regs_r, toaplan1_scroll_regs_w)
	AM_RANGE(0x240000, 0x243fff) AM_RAM
	AM_RANGE(0x300000, 0x300001) AM_READ_PORT("VBLANK")
//  AM_RANGE(0x300000, 0x300001) AM_WRITE(?? video frame related ??)
	AM_RANGE(0x300002, 0x300003) AM_WRITE(toaplan1_intenable_w)
	AM_RANGE(0x300008, 0x30000f) AM_WRITE(toaplan1_bcu_control_w)
	AM_RANGE(0x304000, 0x3047ff) AM_RAM_WRITE(toaplan1_bgpalette_w) AM_SHARE("bgpalette")
	AM_RANGE(0x306000, 0x3067ff) AM_RAM_WRITE(toaplan1_fgpalette_w) AM_SHARE("fgpalette")
	AM_RANGE(0x340000, 0x340003) AM_WRITE(toaplan1_tile_offsets_w)
	AM_RANGE(0x340006, 0x340007) AM_WRITE(toaplan1_fcu_flipscreen_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( vimana_main_map, AS_PROGRAM, 16, toaplan1_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x080000, 0x080003) AM_WRITE(toaplan1_tile_offsets_w)
	AM_RANGE(0x080006, 0x080007) AM_WRITE(toaplan1_fcu_flipscreen_w)
	AM_RANGE(0x0c0000, 0x0c0001) AM_READ(toaplan1_frame_done_r)
	AM_RANGE(0x0c0002, 0x0c0003) AM_READWRITE(toaplan1_spriteram_offs_r, toaplan1_spriteram_offs_w)
	AM_RANGE(0x0c0004, 0x0c0005) AM_READWRITE(toaplan1_spriteram16_r, toaplan1_spriteram16_w)
	AM_RANGE(0x0c0006, 0x0c0007) AM_READWRITE(toaplan1_spritesizeram16_r, toaplan1_spritesizeram16_w)
	AM_RANGE(0x400000, 0x400001) AM_READ_PORT("VBLANK")
//  AM_RANGE(0x400000, 0x400001) AM_WRITE(?? video frame related ??)
	AM_RANGE(0x400002, 0x400003) AM_WRITE(toaplan1_intenable_w)
	AM_RANGE(0x400008, 0x40000f) AM_WRITE(toaplan1_bcu_control_w)
	AM_RANGE(0x404000, 0x4047ff) AM_RAM_WRITE(toaplan1_bgpalette_w) AM_SHARE("bgpalette")
	AM_RANGE(0x406000, 0x4067ff) AM_RAM_WRITE(toaplan1_fgpalette_w) AM_SHARE("fgpalette")
	AM_RANGE(0x440000, 0x440005) AM_READWRITE(vimana_mcu_r, vimana_mcu_w)  /* shared memory from 0x440000 to 0x44ffff ? */
	AM_RANGE(0x440006, 0x440007) AM_READ_PORT("DSWA")
	AM_RANGE(0x440008, 0x440009) AM_READ(vimana_system_port_r)   /* "SYSTEM" + coinage simulation */
	AM_RANGE(0x44000a, 0x44000b) AM_READ_PORT("P1")
	AM_RANGE(0x44000c, 0x44000d) AM_READ_PORT("P2")
	AM_RANGE(0x44000e, 0x44000f) AM_READ_PORT("DSWB")
	AM_RANGE(0x440010, 0x440011) AM_READ_PORT("TJUMP")
	AM_RANGE(0x480000, 0x487fff) AM_RAM
	AM_RANGE(0x4c0000, 0x4c0001) AM_WRITE(toaplan1_bcu_flipscreen_w)
	AM_RANGE(0x4c0002, 0x4c0003) AM_READWRITE(toaplan1_tileram_offs_r, toaplan1_tileram_offs_w)
	AM_RANGE(0x4c0004, 0x4c0007) AM_READWRITE(toaplan1_tileram16_r, toaplan1_tileram16_w)
	AM_RANGE(0x4c0010, 0x4c001f) AM_READWRITE(toaplan1_scroll_regs_r, toaplan1_scroll_regs_w)
ADDRESS_MAP_END


/***************************** Z80 Memory Map *******************************/

static ADDRESS_MAP_START( toaplan1_sound_map, AS_PROGRAM, 8, toaplan1_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_SHARE("sharedram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( rallybik_sound_io_map, AS_IO, 8, toaplan1_rallybik_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("P1")
	AM_RANGE(0x10, 0x10) AM_READ_PORT("P2")
	AM_RANGE(0x20, 0x20) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x30, 0x30) AM_WRITE(rallybik_coin_w)  /* Coin counter/lockout */
	AM_RANGE(0x40, 0x40) AM_READ_PORT("DSWA")
	AM_RANGE(0x50, 0x50) AM_READ_PORT("DSWB")
	AM_RANGE(0x60, 0x61) AM_DEVREADWRITE("ymsnd", ym3812_device, read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START( truxton_sound_io_map, AS_IO, 8, toaplan1_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("P1")
	AM_RANGE(0x10, 0x10) AM_READ_PORT("P2")
	AM_RANGE(0x20, 0x20) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x30, 0x30) AM_WRITE(toaplan1_coin_w)  /* Coin counter/lockout */
	AM_RANGE(0x40, 0x40) AM_READ_PORT("DSWA")
	AM_RANGE(0x50, 0x50) AM_READ_PORT("DSWB")
	AM_RANGE(0x60, 0x61) AM_DEVREADWRITE("ymsnd", ym3812_device, read, write)
	AM_RANGE(0x70, 0x70) AM_READ_PORT("TJUMP")
ADDRESS_MAP_END

static ADDRESS_MAP_START( hellfire_sound_io_map, AS_IO, 8, toaplan1_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("DSWA")
	AM_RANGE(0x10, 0x10) AM_READ_PORT("DSWB")
	AM_RANGE(0x20, 0x20) AM_READ_PORT("TJUMP")
	AM_RANGE(0x30, 0x30) AM_WRITE(toaplan1_coin_w)  /* Coin counter/lockout */
	AM_RANGE(0x40, 0x40) AM_READ_PORT("P1")
	AM_RANGE(0x50, 0x50) AM_READ_PORT("P2")
	AM_RANGE(0x60, 0x60) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x70, 0x71) AM_DEVREADWRITE("ymsnd", ym3812_device, read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START( zerowing_sound_io_map, AS_IO, 8, toaplan1_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("P1")
	AM_RANGE(0x08, 0x08) AM_READ_PORT("P2")
	AM_RANGE(0x20, 0x20) AM_READ_PORT("DSWA")
	AM_RANGE(0x28, 0x28) AM_READ_PORT("DSWB")
	AM_RANGE(0x80, 0x80) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x88, 0x88) AM_READ_PORT("TJUMP")
	AM_RANGE(0xa0, 0xa0) AM_WRITE(toaplan1_coin_w)  /* Coin counter/lockout */
	AM_RANGE(0xa8, 0xa9) AM_DEVREADWRITE("ymsnd", ym3812_device, read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START( demonwld_sound_io_map, AS_IO, 8, toaplan1_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVREADWRITE("ymsnd", ym3812_device, read, write)
	AM_RANGE(0x20, 0x20) AM_READ_PORT("TJUMP")
	AM_RANGE(0x40, 0x40) AM_WRITE(toaplan1_coin_w)  /* Coin counter/lockout */
	AM_RANGE(0x60, 0x60) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x80, 0x80) AM_READ_PORT("P1")
	AM_RANGE(0xa0, 0xa0) AM_READ_PORT("DSWB")
	AM_RANGE(0xc0, 0xc0) AM_READ_PORT("P2")
	AM_RANGE(0xe0, 0xe0) AM_READ_PORT("DSWA")
ADDRESS_MAP_END

static ADDRESS_MAP_START( outzone_sound_io_map, AS_IO, 8, toaplan1_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVREADWRITE("ymsnd", ym3812_device, read, write)
	AM_RANGE(0x04, 0x04) AM_WRITE(toaplan1_coin_w)  /* Coin counter/lockout */
	AM_RANGE(0x08, 0x08) AM_READ_PORT("DSWA")
	AM_RANGE(0x0c, 0x0c) AM_READ_PORT("DSWB")
	AM_RANGE(0x10, 0x10) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x14, 0x14) AM_READ_PORT("P1")
	AM_RANGE(0x18, 0x18) AM_READ_PORT("P2")
	AM_RANGE(0x1c, 0x1c) AM_READ_PORT("TJUMP")
ADDRESS_MAP_END


/***************************** TMS32010 Memory Map **************************/

static ADDRESS_MAP_START( DSP_program_map, AS_PROGRAM, 16, toaplan1_state )
	AM_RANGE(0x000, 0x7ff) AM_ROM
ADDRESS_MAP_END

	/* $000 - 08F  TMS32010 Internal Data RAM in Data Address Space */

static ADDRESS_MAP_START( DSP_io_map, AS_IO, 16, toaplan1_state )
	AM_RANGE(0, 0) AM_WRITE(demonwld_dsp_addrsel_w)
	AM_RANGE(1, 1) AM_READWRITE(demonwld_dsp_r, demonwld_dsp_w)
	AM_RANGE(3, 3) AM_WRITE(demonwld_dsp_bio_w)
	AM_RANGE(TMS32010_BIO, TMS32010_BIO) AM_READ(demonwld_BIO_r)
ADDRESS_MAP_END


/***************************** HD647180 Memory Map **************************/

static ADDRESS_MAP_START( hd647180_mem_map, AS_PROGRAM, 8, toaplan1_state )
	AM_RANGE(0x00000, 0x03fff) AM_ROM   /* Internal 16k byte ROM */
	AM_RANGE(0x0fe00, 0x0ffff) AM_RAM   /* Internal 512 byte RAM */
ADDRESS_MAP_END



/*****************************************************************************
    Generic Input Port definitions
*****************************************************************************/
static INPUT_PORTS_START( toaplan1_2b )
	PORT_START("P1")
	TOAPLAN_JOY_UDLR_2_BUTTONS( 1 )

	PORT_START("P2")
	TOAPLAN_JOY_UDLR_2_BUTTONS( 2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_TILT )
	TOAPLAN_TEST_SWITCH( 0x04, IP_ACTIVE_HIGH ) /* see notes */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("VBLANK")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0xfffe, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( toaplan1_3b )
	PORT_INCLUDE( toaplan1_2b )

	PORT_MODIFY("P1")
	TOAPLAN_JOY_UDLR_3_BUTTONS( 1 )

	PORT_MODIFY("P2")
	TOAPLAN_JOY_UDLR_3_BUTTONS( 2 )
INPUT_PORTS_END

#define  TOAPLAN1_PLAYER_INPUT( player, button3 )                                       \
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(player) PORT_8WAY     \
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(player) PORT_8WAY   \
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(player) PORT_8WAY   \
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(player) PORT_8WAY  \
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(player)                   \
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(player)                   \
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, button3 ) PORT_PLAYER(player)                       \
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

#define  TOAPLAN1_PLAYER_INPUT_COCKTAIL( player, button3, options )                             \
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(player) options PORT_8WAY     \
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(player) options PORT_8WAY   \
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(player) options PORT_8WAY   \
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(player) options PORT_8WAY  \
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(player) options                   \
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(player) options                   \
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, button3 ) PORT_PLAYER(player) options                       \
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

#define  TOAPLAN1_SYSTEM_INPUTS                     \
	PORT_START("SYSTEM")                        \
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )  \
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_TILT )      \
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )   \
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )     \
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )     \
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )    \
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )    \
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

#define  TOAPLAN1_VBLANK_INPUT                      \
	PORT_START("VBLANK")                        \
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")    \
	PORT_BIT( 0xfffe, IP_ACTIVE_HIGH, IPT_UNKNOWN )


/*****************************************************************************
    Game-specific Input Port definitions
*****************************************************************************/

/* verified from M68000 and Z80 code */
static INPUT_PORTS_START( rallybik )
	PORT_INCLUDE( toaplan1_2b )

	/* in 0x40 (CPU1) -> 0x8003 (CPU1 shared RAM) = 0x180006.w (CPU0 shared RAM) -> 0x0804f2.w */
	PORT_START("DSWA")
	TOAPLAN_MACHINE_COCKTAIL
	TOAPLAN_COINAGE_DUAL(DSWB, 0x30, 0x20)                  /* see notes */

	/* in 0x50 (CPU1) -> 0x8004 (CPU1 shared RAM) = 0x180008.w (CPU0 shared RAM) -> 0x0804f4.w */
	PORT_START("DSWB")
	TOAPLAN_DIFFICULTY
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_HIGH )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x30, 0x20, DEF_STR( Region ) )           /* bits 4 and 5 listed as unused in the Dip Switches screen */
	PORT_DIPSETTING(    0x20, DEF_STR( Europe ) )           /* Taito Corp. Japan */
	PORT_DIPSETTING(    0x10, DEF_STR( USA ) )              /* Taito America Corp. */
	PORT_DIPSETTING(    0x30, "USA (Romstar license)" )     /* Taito America Corp. */
	PORT_DIPSETTING(    0x00, DEF_STR( Japan ) )            /* Taito Corporation */
	PORT_DIPNAME( 0x40, 0x00, "Dip Switch Display" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Allow_Continue ) )   /* not on race 1 */
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	/* P1 : in 0x00 (CPU1) -> 0x8006 (CPU1 shared RAM) = 0x18000c.w (CPU0 shared RAM) */
	/* P2 : in 0x10 (CPU1) -> 0x8007 (CPU1 shared RAM) = 0x18000e.w (CPU0 shared RAM) */
	/* SYSTEM : in 0x20 (CPU1) -> 0x8005 (CPU1 shared RAM) = 0x18000a.w (CPU0 shared RAM) -> 0x0804f6.w */
	/* VBLANK : 0x140000.w */
INPUT_PORTS_END


/* verified from M68000 and Z80 code */
static INPUT_PORTS_START( truxton )
	PORT_INCLUDE( toaplan1_2b )

	/* in 0x40 (CPU1) -> 0x8003 (CPU1 shared RAM) = 0x180006.w (CPU0 shared RAM) -> 0x081b78.w */
	PORT_START("DSWA")
	TOAPLAN_MACHINE_COCKTAIL
	TOAPLAN_COINAGE_DUAL(TJUMP, 0x03, 0x02)                 /* see notes */

	/* in 0x50 (CPU1) -> 0x8004 (CPU1 shared RAM) = 0x180008.w (CPU0 shared RAM) -> 0x081b7a.w */
	PORT_START("DSWB")
	TOAPLAN_DIFFICULTY
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )       /* table at 0x000930 */
	PORT_DIPSETTING(    0x04, "50k 200k 150k+" )
	PORT_DIPSETTING(    0x00, "70k 270k 200k+" )
	PORT_DIPSETTING(    0x08, "100k Only" )
	PORT_DIPSETTING(    0x0c, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x40, 0x00, "Dip Switch Display" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	/* in 0x70 (CPU1) -> 0x8005 (CPU1 shared RAM) = 0x18000a.w (CPU0 shared RAM) -> 0x081b7c.w */
	PORT_START("TJUMP")       /* Territory Jumper Block - see notes */
	PORT_DIPNAME( 0x07, 0x02, DEF_STR( Region ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Europe ) )           /* No notice    Taito Corporation    TOAPLAN_COINAGE_WORLD  FOR EUROPE */
//  PORT_DIPSETTING(    0x03, DEF_STR( Europe ) )           /* No notice    Taito Corporation    TOAPLAN_COINAGE_JAPAN  FOR EUROPE */
//  PORT_DIPSETTING(    0x06, DEF_STR( Europe ) )           /* No notice    Taito America Corp.  TOAPLAN_COINAGE_WORLD  FOR EUROPE */
//  PORT_DIPSETTING(    0x07, DEF_STR( Europe ) )           /* No notice    Taito America Corp.  TOAPLAN_COINAGE_JAPAN  FOR EUROPE */
	PORT_DIPSETTING(    0x05, DEF_STR( USA ) )              /* U.S.A. ONLY  Taito America Corp.  TOAPLAN_COINAGE_JAPAN  FOR U.S.A. */
//  PORT_DIPSETTING(    0x04, DEF_STR( USA ) )              /* U.S.A. ONLY  Taito America Corp.  TOAPLAN_COINAGE_JAPAN  FOR JAPAN  */
	PORT_DIPSETTING(    0x01, "USA (Romstar license)" )     /* U.S.A. ONLY  Taito America Corp.  TOAPLAN_COINAGE_JAPAN  FOR U.S.A. */
	PORT_DIPSETTING(    0x00, DEF_STR( Japan ) )            /* JAPAN ONLY   Taito Corporation    TOAPLAN_COINAGE_JAPAN  FOR JAPAN  */
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_HIGH )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	/* P1 : in 0x00 (CPU1) -> 0x8007 (CPU1 shared RAM) = 0x18000e.w (CPU0 shared RAM) -> 0x081b82.w */
	/* P2 : in 0x10 (CPU1) -> 0x8008 (CPU1 shared RAM) = 0x180010.w (CPU0 shared RAM) -> 0x081b84.w */
	/* SYSTEM : in 0x20 (CPU1) -> 0x8006 (CPU1 shared RAM) = 0x18000c.w (CPU0 shared RAM) -> 0x081b7e.w */
	/* VBLANK : 0x140000.w */
INPUT_PORTS_END


/* verified from M68000 and Z80 code */
static INPUT_PORTS_START( hellfire )
	PORT_INCLUDE( toaplan1_2b )

	/* in 0x00 (CPU1) -> 0x8003 (CPU1 shared RAM) = 0x0c0006.w (CPU0 shared RAM) -> 0x042410.w */
	PORT_START("DSWA")
	TOAPLAN_MACHINE_NO_COCKTAIL
	TOAPLAN_COINAGE_DUAL(TJUMP, 0x02, 0x02)                 /* see notes */

	/* in 0x10 (CPU1) -> 0x8004 (CPU1 shared RAM) = 0x0c0008.w (CPU0 shared RAM) -> 0x042412.w */
	PORT_START("DSWB")
	TOAPLAN_DIFFICULTY
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )       /* table at 0x00390e ('hellfire') */
	PORT_DIPSETTING(    0x00, "70k 270k 200k+" )            /*        / 0x0030f0 ('hellfire1') */
	PORT_DIPSETTING(    0x04, "100k 350k 250k+" )           /*        / 0x003aac ('hellfire2a') */
	PORT_DIPSETTING(    0x08, "100k Only" )                 /*        / 0x00329c ('hellfire1a') */
	PORT_DIPSETTING(    0x0c, "200k Only" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x40, 0x00, "Invulnerability" )           /* see notes */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_HIGH )

	/* in 0x20 (CPU1) -> 0x8005 (CPU1 shared RAM) = 0x0c000a.w (CPU0 shared RAM) -> 0x042414.w */
	PORT_START("TJUMP")       /* Territory Jumper Block - see notes */
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Region ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Europe ) )
//  PORT_DIPSETTING(    0x03, DEF_STR( Europe ) )
	PORT_DIPSETTING(    0x01, DEF_STR( USA ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Japan ) )
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_HIGH )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_HIGH )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	/* P1 : in 0x40 (CPU1) -> 0x8007 (CPU1 shared RAM) = 0x0c000e.w (CPU0 shared RAM) -> 0x04241c.w */
	/* P2 : in 0x50 (CPU1) -> 0x8008 (CPU1 shared RAM) = 0x0c0010.w (CPU0 shared RAM) -> 0x04241e.w */
	/* SYSTEM : in 0x60 (CPU1) -> 0x8006 (CPU1 shared RAM) = 0x0c000c.w (CPU0 shared RAM) -> 0x042416.w */
	/* VBLANK : 0x080000.w */
INPUT_PORTS_END

/* verified from M68000 and Z80 code */
static INPUT_PORTS_START( hellfire1 )
	PORT_INCLUDE( hellfire )

	/* in 0x00 (CPU1) -> 0x8003 (CPU1 shared RAM) = 0x0c0006.w (CPU0 shared RAM) -> 0x04222a.w */
	PORT_MODIFY("DSWA")
	TOAPLAN_MACHINE_COCKTAIL

	/* in 0x10 (CPU1) -> 0x8004 (CPU1 shared RAM) = 0x0c0008.w (CPU0 shared RAM) -> 0x04222c.w */
	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	/* TJUMP : in 0x20 (CPU1) -> 0x8005 (CPU1 shared RAM) = 0x0c000a.w (CPU0 shared RAM) -> 0x04222e.w */

	/* P1 : in 0x40 (CPU1) -> 0x8007 (CPU1 shared RAM) = 0x0c000e.w (CPU0 shared RAM) -> 0x042236.w */
	/* P2 : in 0x50 (CPU1) -> 0x8008 (CPU1 shared RAM) = 0x0c0010.w (CPU0 shared RAM) -> 0x042238.w */
	/* SYSTEM : in 0x60 (CPU1) -> 0x8006 (CPU1 shared RAM) = 0x0c000c.w (CPU0 shared RAM) -> 0x042230.w */
	/* VBLANK : 0x080000.w */
INPUT_PORTS_END

/* verified from M68000 and Z80 code */
static INPUT_PORTS_START( hellfire2a )
	PORT_INCLUDE( hellfire )

	/* DSWA : in 0x00 (CPU1) -> 0x8003 (CPU1 shared RAM) = 0x0c0006.w (CPU0 shared RAM) -> 0x042410.w */

	/* in 0x10 (CPU1) -> 0x8004 (CPU1 shared RAM) = 0x0c0008.w (CPU0 shared RAM) -> 0x042412.w */
	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x40, 0x00, "Dip Switch Display" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )

	/* TJUMP : in 0x20 (CPU1) -> 0x8005 (CPU1 shared RAM) = 0x0c000a.w (CPU0 shared RAM) -> 0x042414.w */

	/* P1 : in 0x40 (CPU1) -> 0x8007 (CPU1 shared RAM) = 0x0c000e.w (CPU0 shared RAM) -> 0x04241c.w */
	/* P2 : in 0x50 (CPU1) -> 0x8008 (CPU1 shared RAM) = 0x0c0010.w (CPU0 shared RAM) -> 0x04241e.w */
	/* SYSTEM : in 0x60 (CPU1) -> 0x8006 (CPU1 shared RAM) = 0x0c000c.w (CPU0 shared RAM) -> 0x042416.w */
	/* VBLANK : 0x080000.w */
INPUT_PORTS_END

/* verified from M68000 and Z80 code */
static INPUT_PORTS_START( hellfire1a )
	PORT_INCLUDE( hellfire )

	/* in 0x00 (CPU1) -> 0x8003 (CPU1 shared RAM) = 0x0c0006.w (CPU0 shared RAM) -> 0x04222a.w */
	PORT_MODIFY("DSWA")
	TOAPLAN_MACHINE_COCKTAIL
	TOAPLAN_COINAGE_DUAL(TJUMP, 0x03, 0x02)                 /* see notes */

	/* in 0x10 (CPU1) -> 0x8004 (CPU1 shared RAM) = 0x0c0008.w (CPU0 shared RAM) -> 0x04222c.w */
	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x40, 0x00, "Dip Switch Display" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	/* TJUMP : in 0x20 (CPU1) -> 0x8005 (CPU1 shared RAM) = 0x0c000a.w (CPU0 shared RAM) -> 0x04222e.w */

	/* P1 : in 0x40 (CPU1) -> 0x8007 (CPU1 shared RAM) = 0x0c000e.w (CPU0 shared RAM) -> 0x042236.w */
	/* P2 : in 0x50 (CPU1) -> 0x8008 (CPU1 shared RAM) = 0x0c0010.w (CPU0 shared RAM) -> 0x042238.w */
	/* SYSTEM : in 0x60 (CPU1) -> 0x8006 (CPU1 shared RAM) = 0x0c000c.w (CPU0 shared RAM) -> 0x042230.w */
	/* VBLANK : 0x080000.w */
INPUT_PORTS_END


/* verified from M68000 and Z80 code */
static INPUT_PORTS_START( zerowing )
	PORT_INCLUDE( toaplan1_2b )

	/* in 0x20 (CPU1) -> 0x8003 (CPU1 shared RAM) = 0x440006.w (CPU0 shared RAM) -> 0x08180c.w */
	PORT_START("DSWA")
	TOAPLAN_MACHINE_COCKTAIL
	TOAPLAN_COINAGE_DUAL(TJUMP, 0x02, 0x02)                 /* see notes */

	/* in 0x28 (CPU1) -> 0x8004 (CPU1 shared RAM) = 0x440008.w (CPU0 shared RAM) -> 0x08180e.w */
	PORT_START("DSWB")
	TOAPLAN_DIFFICULTY
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )       /* table at 0x00216c ('zerowing') or 0x002606 ('zerowing2') */
	PORT_DIPSETTING(    0x00, "200k 700k 500k+" )
	PORT_DIPSETTING(    0x04, "500k 1500k 1000k+" )
	PORT_DIPSETTING(    0x08, "500k Only" )
	PORT_DIPSETTING(    0x0c, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x40, 0x00, "Invulnerability" )           /* see notes */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	/* in 0x88 (CPU1) -> 0x8005 (CPU1 shared RAM) = 0x44000a.w (CPU0 shared RAM) -> 0x081810.w */
	PORT_START("TJUMP")       /* Territory Jumper Block - see notes */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Region ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Europe ) )           /* 3 letters initials - right */
//  PORT_DIPSETTING(    0x02, DEF_STR( Europe ) )           /* 6 letters initials - wrong */
	PORT_DIPSETTING(    0x01, DEF_STR( USA ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Japan ) )
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_HIGH )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_HIGH )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	/* P1 : in 0x00 (CPU1) -> 0x8007 (CPU1 shared RAM) = 0x44000e.w (CPU0 shared RAM) -> 0x081818.w */
	/* P2 : in 0x08 (CPU1) -> 0x8008 (CPU1 shared RAM) = 0x440010.w (CPU0 shared RAM) -> 0x08181a.w */
	/* SYSTEM : in 0x80 (CPU1) -> 0x8006 (CPU1 shared RAM) = 0x44000c.w (CPU0 shared RAM) -> 0x081812.w */
	/* VBLANK : 0x400000.w */
INPUT_PORTS_END

/* verified from M68000 and Z80 code */
static INPUT_PORTS_START( zerowing2 )
	PORT_INCLUDE( zerowing )

	/* in 0x20 (CPU1) -> 0x8003 (CPU1 shared RAM) = 0x440006.w (CPU0 shared RAM) -> 0x081ade.w */
	PORT_MODIFY("DSWA")
	TOAPLAN_MACHINE_NO_COCKTAIL

	/* in 0x28 (CPU1) -> 0x8004 (CPU1 shared RAM) = 0x440008.w (CPU0 shared RAM) -> 0x081ae0.w */
	PORT_MODIFY("DSWB")
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_HIGH )

	/* in 0x88 (CPU1) -> 0x8005 (CPU1 shared RAM) = 0x44000a.w (CPU0 shared RAM) -> 0x081ae2.w */
	PORT_MODIFY("TJUMP")      /* Territory Jumper Block - see notes */
	PORT_DIPUNUSED( 0x01, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Region ) )
	PORT_DIPSETTING(    0x00, DEF_STR( USA ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Europe ) )
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_HIGH )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_HIGH )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	/* P1 : in 0x00 (CPU1) -> 0x8007 (CPU1 shared RAM) = 0x44000e.w (CPU0 shared RAM) -> 0x081aea.w */
	/* P2 : in 0x08 (CPU1) -> 0x8008 (CPU1 shared RAM) = 0x440010.w (CPU0 shared RAM) -> 0x081aec.w */
	/* SYSTEM : in 0x80 (CPU1) -> 0x8006 (CPU1 shared RAM) = 0x44000c.w (CPU0 shared RAM) -> 0x081ae4.w */
	/* VBLANK : 0x400000.w */
INPUT_PORTS_END


static INPUT_PORTS_START( demonwld )
	TOAPLAN1_VBLANK_INPUT

	PORT_START("P1")
	TOAPLAN1_PLAYER_INPUT( 1, IPT_BUTTON3 )

	PORT_START("P2")
	TOAPLAN1_PLAYER_INPUT( 2, IPT_BUTTON3 )

	PORT_START("DSWA")      /* DSW A */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_SERVICE( 0x04, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_6C ) )

	PORT_START("DSWB")      /* DSW B */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "30K, every 100K" )
	PORT_DIPSETTING(    0x04, "50K and 100K" )
	PORT_DIPSETTING(    0x08, "100K only" )
	PORT_DIPSETTING(    0x0c, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x40, 0x00, "Invulnerability" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	TOAPLAN1_SYSTEM_INPUTS

	PORT_START("TJUMP")     /* Territory Jumper Block */
	PORT_DIPNAME( 0x01, 0x01, "Territory/Copyright" )
	PORT_DIPSETTING(    0x01, "Toaplan" )
	PORT_DIPSETTING(    0x00, "Japan/Taito Corp" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( demonwld1 )
	PORT_INCLUDE( demonwld )

	PORT_MODIFY("TJUMP")        /* Territory Jumper Block */
	PORT_DIPNAME( 0x03, 0x02, "Territory/Copyright" )
	PORT_DIPSETTING(    0x02, "World/Taito Japan" )
	PORT_DIPSETTING(    0x03, "US/Toaplan" )
	PORT_DIPSETTING(    0x01, "US/Taito America" )
	PORT_DIPSETTING(    0x00, "Japan/Taito Corp" )
INPUT_PORTS_END


static INPUT_PORTS_START( fireshrk )
	TOAPLAN1_VBLANK_INPUT

	PORT_START("P1")
	TOAPLAN1_PLAYER_INPUT( 1, IPT_UNKNOWN )

	PORT_START("P2")
	TOAPLAN1_PLAYER_INPUT_COCKTAIL( 2, IPT_UNKNOWN, PORT_COCKTAIL )

	PORT_START("DSWA")      /* DSW A */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_SERVICE( 0x04, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_6C ) )

	PORT_START("DSWB")      /* DSW B */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x04, "50K, every 150K" )
	PORT_DIPSETTING(    0x00, "70K, every 200K" )
	PORT_DIPSETTING(    0x08, "100K" )
	PORT_DIPSETTING(    0x0c, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x40, 0x00, "Invulnerability" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	TOAPLAN1_SYSTEM_INPUTS

	PORT_START("TJUMP")     /* Territory Jumper Block */
	PORT_DIPNAME( 0x06, 0x02, DEF_STR( Region ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Europe ) )
	PORT_DIPSETTING(    0x04, DEF_STR( USA ) )
	PORT_DIPSETTING(    0x00, "USA (Romstar)" )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( fireshrka ) /* No "Romstar" license */
	PORT_INCLUDE( fireshrk )

	PORT_MODIFY("TJUMP")        /* Territory Jumper Block */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Region ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Europe ) )
//  PORT_DIPSETTING(    0x02, DEF_STR( Europe ) )
//  PORT_DIPSETTING(    0x01, DEF_STR( Europe ) )
	PORT_DIPSETTING(    0x00, DEF_STR( USA ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( samesame )
	PORT_INCLUDE( fireshrk )

	PORT_MODIFY("DSWA")     /* DSW A */
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
/* settings listed in service mode, but not actually used ???
    PORT_DIPNAME( 0x30, 0x00, DEF_STR( Coin_A ) )
    PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
    PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
    PORT_DIPSETTING(    0x30, DEF_STR( 2C_3C ) )
    PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
    PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_B ) )
    PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
    PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
    PORT_DIPSETTING(    0xc0, DEF_STR( 2C_3C ) )
    PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
*/

	PORT_MODIFY("TJUMP")        /* Territory Jumper Block */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( samesame2 )
	PORT_INCLUDE( samesame )

	PORT_MODIFY("P2")
	TOAPLAN1_PLAYER_INPUT( 2, IPT_UNKNOWN )

	PORT_MODIFY("DSWA")     /* DSW A */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )  /* No cocktail */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )

	PORT_MODIFY("TJUMP")        /* Territory Jumper Block */
/* settings listed in service mode, but not actually used
    PORT_DIPNAME( 0x03, 0x00, DEF_STR( Region ) )
//  PORT_DIPSETTING(    0x01, DEF_STR( Europe ) )
//  PORT_DIPSETTING(    0x02, DEF_STR( Europe ) )
    PORT_DIPSETTING(    0x03, DEF_STR( Europe ) )
    PORT_DIPSETTING(    0x00, DEF_STR( USA ) )
*/
	PORT_DIPNAME( 0x01, 0x00, "Show Territory Notice" )
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
//  PORT_DIPNAME( 0x02, 0x00, "Show Territory Notice" ) /* Same as Bit 1 */
//  PORT_DIPSETTING(    0x02, DEF_STR( No ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_BIT( 0xf2, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* Mask bit 2 aswell */
INPUT_PORTS_END

/* verified from M68000 and Z80 code */
static INPUT_PORTS_START( outzone )
	PORT_INCLUDE( toaplan1_3b )

	/* in 0x08 (CPU1) -> 0x8003 (CPU1 shared RAM) = 0x140006.w (CPU0 shared RAM) -> 0x240b44.w */
	PORT_START("DSWA")
	TOAPLAN_MACHINE_NO_COCKTAIL
	TOAPLAN_COINAGE_DUAL(TJUMP, 0x0f, 0x02)                 /* see notes */

	/* in 0x0c (CPU1) -> 0x8004 (CPU1 shared RAM) = 0x140008.w (CPU0 shared RAM) -> 0x240b46.w */
	PORT_START("DSWB")
	TOAPLAN_DIFFICULTY
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )       /* table at 0x001cc8 ('outzone' 'outzoneh' 'outzonea') */
	PORT_DIPSETTING(    0x00, "Every 300k" )                /*        / 0x001c22 ('outzoneb') */
	PORT_DIPSETTING(    0x04, "200k and 500k" )             /*        / 0x001c2e ('outzonec') */
	PORT_DIPSETTING(    0x08, "300k Only" )
	PORT_DIPSETTING(    0x0c, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x40, 0x00, "Invulnerability" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_HIGH )

	/* in 0x1c (CPU1) -> 0x8005 (CPU1 shared RAM) = 0x14000a.w (CPU0 shared RAM) */
	PORT_START("TJUMP")     /* Territory Jumper Block - see notes */
	PORT_DIPNAME( 0x0f, 0x02, DEF_STR( Region ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Japan ) )
	PORT_DIPSETTING(    0x01, DEF_STR( USA ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Europe ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hong_Kong ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Korea ) )
	PORT_DIPSETTING(    0x05, DEF_STR( Taiwan ) )
	PORT_DIPSETTING(    0x06, "Taiwan (Spacy Co., Ltd.)" )
	PORT_DIPSETTING(    0x07, "USA (Romstar, Inc.)" )
	PORT_DIPSETTING(    0x08, "Hong Kong & China (Honest Trading Co.)" )
//  PORT_DIPSETTING(    0x09, DEF_STR( Japan ) )
//  PORT_DIPSETTING(    0x0a, DEF_STR( Japan ) )
//  PORT_DIPSETTING(    0x0b, DEF_STR( Japan ) )
//  PORT_DIPSETTING(    0x0c, DEF_STR( Japan ) )
//  PORT_DIPSETTING(    0x0d, DEF_STR( Japan ) )
//  PORT_DIPSETTING(    0x0e, DEF_STR( Japan ) )
//  PORT_DIPSETTING(    0x0f, DEF_STR( Japan ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	/* P1 : in 0x14 (CPU1) -> 0x8007 (CPU1 shared RAM) = 0x14000e.w (CPU0 shared RAM) */
	/* P2 : in 0x18 (CPU1) -> 0x8008 (CPU1 shared RAM) = 0x140010.w (CPU0 shared RAM) */
	/* SYSTEM : in 0x10 (CPU1) -> 0x8006 (CPU1 shared RAM) = 0x14000c.w (CPU0 shared RAM) -> 0x240b48.w */
	/* VBLANK : 0x300000.w */
INPUT_PORTS_END

/* verified from M68000 and Z80 code */
static INPUT_PORTS_START( outzonea )
	PORT_INCLUDE( outzone )

	/* DSWA : in 0x08 (CPU1) -> 0x8003 (CPU1 shared RAM) = 0x140006.w (CPU0 shared RAM) -> 0x240b44.w */
	/* DSWB : in 0x0c (CPU1) -> 0x8004 (CPU1 shared RAM) = 0x140008.w (CPU0 shared RAM) -> 0x240b46.w */

	/* in 0x1c (CPU1) -> 0x8005 (CPU1 shared RAM) = 0x14000a.w (CPU0 shared RAM) */
	PORT_MODIFY("TJUMP")        /* Territory Jumper Block - see notes */
	PORT_DIPNAME( 0x0f, 0x02, DEF_STR( Region ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Japan ) )
	PORT_DIPSETTING(    0x01, DEF_STR( USA ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Europe ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hong_Kong ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Korea ) )
	PORT_DIPSETTING(    0x05, DEF_STR( Taiwan ) )
	PORT_DIPSETTING(    0x06, "Taiwan (Spacy Co., Ltd.)" )
	PORT_DIPSETTING(    0x07, "USA (Romstar, Inc.)" )
	PORT_DIPSETTING(    0x08, "Hong Kong (Honest Trading Co.)" )
//  PORT_DIPSETTING(    0x09, DEF_STR( Japan ) )
//  PORT_DIPSETTING(    0x0a, DEF_STR( Japan ) )
//  PORT_DIPSETTING(    0x0b, DEF_STR( Japan ) )
//  PORT_DIPSETTING(    0x0c, DEF_STR( Japan ) )
//  PORT_DIPSETTING(    0x0d, DEF_STR( Japan ) )
//  PORT_DIPSETTING(    0x0e, DEF_STR( Japan ) )
//  PORT_DIPSETTING(    0x0f, DEF_STR( Japan ) )

	/* P1 : in 0x14 (CPU1) -> 0x8007 (CPU1 shared RAM) = 0x14000e.w (CPU0 shared RAM) */
	/* P2 : in 0x18 (CPU1) -> 0x8008 (CPU1 shared RAM) = 0x140010.w (CPU0 shared RAM) */
	/* SYSTEM : in 0x10 (CPU1) -> 0x8006 (CPU1 shared RAM) = 0x14000c.w (CPU0 shared RAM) -> 0x240b48.w */
	/* VBLANK : 0x300000.w */
INPUT_PORTS_END

/* verified from M68000 and Z80 code */
static INPUT_PORTS_START( outzonec )
	PORT_INCLUDE( outzone )

	/* DSWA : in 0x08 (CPU1) -> 0x8003 (CPU1 shared RAM) = 0x140006.w (CPU0 shared RAM) -> 0x240b44.w */
	/* DSWB : in 0x0c (CPU1) -> 0x8004 (CPU1 shared RAM) = 0x140008.w (CPU0 shared RAM) -> 0x240b46.w */

	/* in 0x1c (CPU1) -> 0x8005 (CPU1 shared RAM) = 0x14000a.w (CPU0 shared RAM) */
	PORT_MODIFY("TJUMP")        /* Territory Jumper Block - see notes */
	PORT_DIPNAME( 0x07, 0x06, DEF_STR( Region ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Japan ) )
	PORT_DIPSETTING(    0x01, DEF_STR( USA ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Europe ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hong_Kong ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Korea ) )
	PORT_DIPSETTING(    0x05, DEF_STR( Taiwan ) )
	PORT_DIPSETTING(    0x06, DEF_STR( World ) )
//  PORT_DIPSETTING(    0x07, DEF_STR( World ) )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_HIGH )

	/* P1 : in 0x14 (CPU1) -> 0x8007 (CPU1 shared RAM) = 0x14000e.w (CPU0 shared RAM) */
	/* P2 : in 0x18 (CPU1) -> 0x8008 (CPU1 shared RAM) = 0x140010.w (CPU0 shared RAM) */
	/* SYSTEM : in 0x10 (CPU1) -> 0x8006 (CPU1 shared RAM) = 0x14000c.w (CPU0 shared RAM) -> 0x240b48.w */
	/* VBLANK : 0x300000.w */
INPUT_PORTS_END


/* verified from M68000 - coinage based on "test mode" and handled by the MCU simulation */
static INPUT_PORTS_START( vimana )
	PORT_INCLUDE( toaplan1_2b )

	/* 0x440007.b */
	PORT_START("DSWA")
	TOAPLAN_MACHINE_NO_COCKTAIL
	TOAPLAN_COINAGE_DUAL(TJUMP, 0x0f, 0x02)                 /* see notes */

	/* 0x44000f.b */
	PORT_START("DSWB")
	TOAPLAN_DIFFICULTY
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )       /* table at 0x000998 */
	PORT_DIPSETTING(    0x00, "70k 270k 200k+" )
	PORT_DIPSETTING(    0x04, "100k 350k 250k+" )
	PORT_DIPSETTING(    0x08, "100k Only" )
	PORT_DIPSETTING(    0x0c, "200k Only" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x40, 0x00, "Invulnerability" )           /* see notes */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	/* 0x440011.b */
	PORT_START("TJUMP")       /* Territory Jumper Block - see notes */
	PORT_DIPNAME( 0x0f, 0x02, DEF_STR( Region ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Europe ) )
	PORT_DIPSETTING(    0x01, DEF_STR( USA ) )
	PORT_DIPSETTING(    0x07, "USA (Romstar license)" )
//  PORT_DIPSETTING(    0x00, "Japan (distributed by Tecmo)" )
//  PORT_DIPSETTING(    0x0f, "Japan (distributed by Tecmo)" )
	PORT_DIPSETTING(    0x04, DEF_STR( Korea ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hong_Kong ) )
	PORT_DIPSETTING(    0x08, "Hong Kong (Honest Trading license)" )
	PORT_DIPSETTING(    0x05, DEF_STR( Taiwan ) )
	PORT_DIPSETTING(    0x06, "Taiwan (Spacy license)" )
//  PORT_DIPSETTING(    0x09, "???" )
//  PORT_DIPSETTING(    0x0a, "???" )
//  PORT_DIPSETTING(    0x0b, "???" )
//  PORT_DIPSETTING(    0x0c, "???" )
//  PORT_DIPSETTING(    0x0d, "???" )
//  PORT_DIPSETTING(    0x0e, "???" )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	/* P1 : 0x44000b.b */
	PORT_MODIFY("P1")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Fast Scrolling") PORT_CODE(KEYCODE_F1)   /* see notes */

	/* P2 : 0x44000d.b */
	/* SYSTEM : 0x440009.b */
	/* VBLANK : 0x400001.b */
INPUT_PORTS_END

/* verified from M68000 - coinage based on "test mode" and handled by the MCU simulation */
static INPUT_PORTS_START( vimanan )
	PORT_INCLUDE( vimana )

	/* DSWA : 0x440007.b */
	/* DSWB : 0x44000f.b */

	/* 0x440011.b */
	PORT_MODIFY("TJUMP")      /* Territory Jumper Block - see notes */
	PORT_DIPNAME( 0x0f, 0x02, DEF_STR( Region ) )
	PORT_DIPSETTING(    0x02, "Europe (Nova Apparate license)" )
	PORT_DIPSETTING(    0x01, DEF_STR( USA ) )
	PORT_DIPSETTING(    0x07, "USA (Romstar license)" )
//  PORT_DIPSETTING(    0x00, "Japan (distributed by Tecmo)" )        /* "ending" text in English */
//  PORT_DIPSETTING(    0x0f, "Japan (distributed by Tecmo)" )        /* "ending" text in English */
	PORT_DIPSETTING(    0x04, DEF_STR( Korea ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hong_Kong ) )
	PORT_DIPSETTING(    0x08, "Hong Kong (Honest Trading license)" )
	PORT_DIPSETTING(    0x05, DEF_STR( Taiwan ) )
	PORT_DIPSETTING(    0x06, "Taiwan (Spacy license)" )
//  PORT_DIPSETTING(    0x09, "???" )
//  PORT_DIPSETTING(    0x0a, "???" )
//  PORT_DIPSETTING(    0x0b, "???" )
//  PORT_DIPSETTING(    0x0c, "???" )
//  PORT_DIPSETTING(    0x0d, "???" )
//  PORT_DIPSETTING(    0x0e, "???" )

	/* P1 : 0x44000b.b */
	/* P2 : 0x44000d.b */
	/* SYSTEM : 0x440009.b */
	/* VBLANK : 0x400001.b */
INPUT_PORTS_END

/* verified from M68000 - coinage based on "test mode" and handled by the MCU simulation */
static INPUT_PORTS_START( vimanaj )
	PORT_INCLUDE( vimana )

	/* DSWA : 0x440007.b */
	/* DSWB : 0x44000f.b */

	/* 0x440011.b */
	PORT_MODIFY("TJUMP")      /* Territory Jumper Block - see notes */
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Region ) )
//  PORT_DIPSETTING(    0x02, DEF_STR( Europe ) )
//  PORT_DIPSETTING(    0x01, DEF_STR( USA ) )
//  PORT_DIPSETTING(    0x07, "USA (Romstar license)" )
	PORT_DIPSETTING(    0x00, "Japan (distributed by Tecmo)" )
//  PORT_DIPSETTING(    0x0f, "Japan (distributed by Tecmo)" )
//  PORT_DIPSETTING(    0x04, "Korea" )
//  PORT_DIPSETTING(    0x03, "Hong Kong" )
//  PORT_DIPSETTING(    0x08, "Hong Kong (Honest Trading license)" )
//  PORT_DIPSETTING(    0x05, "Taiwan" )
//  PORT_DIPSETTING(    0x06, "Taiwan (Spacy license)" )
//  PORT_DIPSETTING(    0x09, "???" )
//  PORT_DIPSETTING(    0x0a, "???" )
//  PORT_DIPSETTING(    0x0b, "???" )
//  PORT_DIPSETTING(    0x0c, "???" )
//  PORT_DIPSETTING(    0x0d, "???" )
//  PORT_DIPSETTING(    0x0e, "???" )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	/* P1 : 0x44000b.b */
	/* P2 : 0x44000d.b */
	/* SYSTEM : 0x440009.b */
	/* VBLANK : 0x400001.b */
INPUT_PORTS_END


static const gfx_layout tilelayout =
{
	8,8,            /* 8x8 */
	RGN_FRAC(1,2),  /* 16384/32768 tiles */
	4,              /* 4 bits per pixel */
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8            /* every tile takes 16 consecutive bytes */
};


static GFXDECODE_START( toaplan1 )
	GFXDECODE_ENTRY( "gfx1", 0x00000, tilelayout,       0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0x00000, tilelayout,   64*16, 64 )
GFXDECODE_END

static GFXDECODE_START( rallybik )
	GFXDECODE_ENTRY( "gfx1", 0x00000, tilelayout,             0, 64 )
GFXDECODE_END


#define PIXEL_CLOCK         (XTAL_28MHz/4)

// HTOTAL and VTOTAL taken from CRTC registers (toaplan1_bcu_control_w)
// rallybik, demonwld and outzone program a larger VTOTAL than the other
// games, giving them a lower frame rate

#define HTOTAL              ((224+1)*2)
#define HBEND               (0)
#define HBSTART             (320)

#define VTOTAL              ((134+1)*2)
#define VTOTAL55            ((140+1)*2)
#define VBEND               (0)
#define VBSTART             (240)


static MACHINE_CONFIG_START( rallybik, toaplan1_rallybik_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_10MHz)
	MCFG_CPU_PROGRAM_MAP(rallybik_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", toaplan1_state,  toaplan1_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_28MHz/8)
	MCFG_CPU_PROGRAM_MAP(toaplan1_sound_map)
	MCFG_CPU_IO_MAP(rallybik_sound_io_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_MACHINE_RESET_OVERRIDE(toaplan1_state,toaplan1)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL55, VBEND, VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(toaplan1_rallybik_state, screen_update_rallybik)
	MCFG_SCREEN_VBLANK_DRIVER(toaplan1_rallybik_state, screen_eof_rallybik)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_TOAPLAN_SCU_ADD("scu", "palette", 31, 15)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", rallybik)
	MCFG_PALETTE_ADD("palette", (64*16)+(64*16))
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_VIDEO_START_OVERRIDE(toaplan1_rallybik_state,rallybik)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_28MHz/8)
	MCFG_YM3812_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( truxton, toaplan1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_10MHz)
	MCFG_CPU_PROGRAM_MAP(truxton_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", toaplan1_state,  toaplan1_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_28MHz/8)
	MCFG_CPU_PROGRAM_MAP(toaplan1_sound_map)
	MCFG_CPU_IO_MAP(truxton_sound_io_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_MACHINE_RESET_OVERRIDE(toaplan1_state,toaplan1)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(toaplan1_state, screen_update_toaplan1)
	MCFG_SCREEN_VBLANK_DRIVER(toaplan1_state, screen_eof_toaplan1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", toaplan1)
	MCFG_PALETTE_ADD("palette", (64*16)+(64*16))
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_VIDEO_START_OVERRIDE(toaplan1_state,toaplan1)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_28MHz/8)
	MCFG_YM3812_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( hellfire, toaplan1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_10MHz)
	MCFG_CPU_PROGRAM_MAP(hellfire_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", toaplan1_state,  toaplan1_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_28MHz/8)
	MCFG_CPU_PROGRAM_MAP(toaplan1_sound_map)
	MCFG_CPU_IO_MAP(hellfire_sound_io_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_MACHINE_RESET_OVERRIDE(toaplan1_state,toaplan1)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND+16, VBSTART+16)
	MCFG_SCREEN_UPDATE_DRIVER(toaplan1_state, screen_update_toaplan1)
	MCFG_SCREEN_VBLANK_DRIVER(toaplan1_state, screen_eof_toaplan1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", toaplan1)
	MCFG_PALETTE_ADD("palette", (64*16)+(64*16))
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_VIDEO_START_OVERRIDE(toaplan1_state,toaplan1)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_28MHz/8)
	MCFG_YM3812_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( zerowing, toaplan1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_10MHz)
	MCFG_CPU_PROGRAM_MAP(zerowing_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", toaplan1_state,  toaplan1_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_28MHz/8)
	MCFG_CPU_PROGRAM_MAP(toaplan1_sound_map)
	MCFG_CPU_IO_MAP(zerowing_sound_io_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_MACHINE_RESET_OVERRIDE(toaplan1_state,zerowing)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND+16, VBSTART+16)
	MCFG_SCREEN_UPDATE_DRIVER(toaplan1_state, screen_update_toaplan1)
	MCFG_SCREEN_VBLANK_DRIVER(toaplan1_state, screen_eof_toaplan1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", toaplan1)
	MCFG_PALETTE_ADD("palette", (64*16)+(64*16))
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_VIDEO_START_OVERRIDE(toaplan1_state,toaplan1)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_28MHz/8)
	MCFG_YM3812_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( demonwld, toaplan1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_10MHz)
	MCFG_CPU_PROGRAM_MAP(demonwld_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", toaplan1_state,  toaplan1_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_28MHz/8)
	MCFG_CPU_PROGRAM_MAP(toaplan1_sound_map)
	MCFG_CPU_IO_MAP(demonwld_sound_io_map)

	MCFG_CPU_ADD("dsp", TMS32010, XTAL_28MHz/2)
	MCFG_CPU_PROGRAM_MAP(DSP_program_map)
	MCFG_CPU_IO_MAP(DSP_io_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_MACHINE_RESET_OVERRIDE(toaplan1_state,demonwld)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL55, VBEND+16, VBSTART+16)
	MCFG_SCREEN_UPDATE_DRIVER(toaplan1_state, screen_update_toaplan1)
	MCFG_SCREEN_VBLANK_DRIVER(toaplan1_state, screen_eof_toaplan1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", toaplan1)
	MCFG_PALETTE_ADD("palette", (64*16)+(64*16))
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_VIDEO_START_OVERRIDE(toaplan1_state,toaplan1)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_28MHz/8)
	MCFG_YM3812_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( samesame, toaplan1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_10MHz)
	MCFG_CPU_PROGRAM_MAP(samesame_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", toaplan1_state,  toaplan1_interrupt)

	MCFG_CPU_ADD("audiocpu", Z180, XTAL_28MHz/8)    /* HD647180XOFS6 CPU */
	MCFG_CPU_PROGRAM_MAP(hd647180_mem_map)
	MCFG_DEVICE_DISABLE()       /* Internal code is not dumped */

	MCFG_MACHINE_RESET_OVERRIDE(toaplan1_state,zerowing)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(toaplan1_state, screen_update_toaplan1)
	MCFG_SCREEN_VBLANK_DRIVER(toaplan1_state, screen_eof_samesame)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", toaplan1)
	MCFG_PALETTE_ADD("palette", (64*16)+(64*16))
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_VIDEO_START_OVERRIDE(toaplan1_state,toaplan1)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_28MHz/8)
	MCFG_YM3812_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( outzone, toaplan1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_10MHz)
	MCFG_CPU_PROGRAM_MAP(outzone_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", toaplan1_state,  toaplan1_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_28MHz/8)
	MCFG_CPU_PROGRAM_MAP(toaplan1_sound_map)
	MCFG_CPU_IO_MAP(outzone_sound_io_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_MACHINE_RESET_OVERRIDE(toaplan1_state,zerowing)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL55, VBEND, VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(toaplan1_state, screen_update_toaplan1)
	MCFG_SCREEN_VBLANK_DRIVER(toaplan1_state, screen_eof_toaplan1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", toaplan1)
	MCFG_PALETTE_ADD("palette", (64*16)+(64*16))
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_VIDEO_START_OVERRIDE(toaplan1_state,toaplan1)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_28MHz/8)
	MCFG_YM3812_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( vimana, toaplan1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_10MHz) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(vimana_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", toaplan1_state,  toaplan1_interrupt)

	MCFG_CPU_ADD("audiocpu", Z180, XTAL_28MHz/8)    /* HD647180XOFS6 CPU */
	MCFG_CPU_PROGRAM_MAP(hd647180_mem_map)
	MCFG_DEVICE_DISABLE()       /* Internal code is not dumped */

	MCFG_MACHINE_RESET_OVERRIDE(toaplan1_state,vimana)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(toaplan1_state, screen_update_toaplan1)
	MCFG_SCREEN_VBLANK_DRIVER(toaplan1_state, screen_eof_toaplan1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", toaplan1)
	MCFG_PALETTE_ADD("palette", (64*16)+(64*16))
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_VIDEO_START_OVERRIDE(toaplan1_state,toaplan1)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_28MHz/8)   /* verified on pcb */
	MCFG_YM3812_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END




/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( rallybik )
	ROM_REGION( 0x080000, "maincpu", 0 )    /* Main 68K code */
	ROM_LOAD16_BYTE( "b45-02.rom",  0x000000, 0x08000, CRC(383386d7) SHA1(fc420b6adc79a408a68f0661d0c62ed7dbe8b6d7) )
	ROM_LOAD16_BYTE( "b45-01.rom",  0x000001, 0x08000, CRC(7602f6a7) SHA1(2939c261a4bc63586681080f5643916c85e81c7d) )
	ROM_LOAD16_BYTE( "b45-04.rom",  0x040000, 0x20000, CRC(e9b005b1) SHA1(19b5acfd5fb2683a56a701400b11ee6f64a9bdf1) )
	ROM_LOAD16_BYTE( "b45-03.rom",  0x040001, 0x20000, CRC(555344ce) SHA1(398963f488fe6f19c0b8518d80c946c242d0fc45) )

	ROM_REGION( 0x8000, "audiocpu", 0 )    /* Sound Z80 code */
	ROM_LOAD( "b45-05.rom",  0x0000, 0x4000, CRC(10814601) SHA1(bad7a834d8849752a7f3000bb5154ec0fa50d695) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "b45-09.bin",  0x00000, 0x20000, CRC(1dc7b010) SHA1(67e8633bd787ffcae0e7867e7e591c492c4f2d63) )
	ROM_LOAD16_BYTE( "b45-08.bin",  0x00001, 0x20000, CRC(fab661ba) SHA1(acc43cd6d979b1c6a348727f315643d7b8f1496a) )
	ROM_LOAD16_BYTE( "b45-07.bin",  0x40000, 0x20000, CRC(cd3748b4) SHA1(a20eb19a0f813112b4e5d9cd91db29de9b37af17) )
	ROM_LOAD16_BYTE( "b45-06.bin",  0x40001, 0x20000, CRC(144b085c) SHA1(84b7412d58fe9c5e9915896db92e80a621571b74) )

	ROM_REGION( 0x40000, "scu", 0 )
	ROM_LOAD( "b45-11.rom",  0x00000, 0x10000, CRC(0d56e8bb) SHA1(c29cb53f846c73b7cf9936051fb0f9dd3805f53f) )
	ROM_LOAD( "b45-10.rom",  0x10000, 0x10000, CRC(dbb7c57e) SHA1(268132965cd65b5e972ca9d0258c30b8a86f3703) )
	ROM_LOAD( "b45-12.rom",  0x20000, 0x10000, CRC(cf5aae4e) SHA1(5832c52d2e9b86414d8ee2926fa190abe9e41da4) )
	ROM_LOAD( "b45-13.rom",  0x30000, 0x10000, CRC(1683b07c) SHA1(54356893357cd1297f24f1d85b7289d80740262d) )

	ROM_REGION( 0x240, "proms", 0 )     /* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "b45-15.bpr",  0x000, 0x100, CRC(24e7d62f) SHA1(1c06a1ef1b6a722794ca1d5ee2c476ecaa5178a3) )   /* sprite priority control ?? */
	ROM_LOAD( "b45-16.bpr",  0x100, 0x100, CRC(a50cef09) SHA1(55cafb5b2551b80ae708e9b966cf37c70a16d310) )   /* sprite priority control ?? */
	ROM_LOAD( "b45-14.bpr",  0x200, 0x020, CRC(f72482db) SHA1(b0cb911f9c81f6088a5aa8760916ddae1f8534d7) )   /* sprite control ?? */
	ROM_LOAD( "b45-17.bpr",  0x220, 0x020, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )   /* sprite attribute (flip/position) ?? */
ROM_END

ROM_START( truxton )
	ROM_REGION( 0x040000, "maincpu", 0 )    /* Main 68K code */
	ROM_LOAD16_BYTE( "b65_11.bin",  0x000000, 0x20000, CRC(1a62379a) SHA1(b9470d4b70c38f2523b22636874d742abe4099eb) )
	ROM_LOAD16_BYTE( "b65_10.bin",  0x000001, 0x20000, CRC(aff5195d) SHA1(a7f379dc35e3acf9e7a8ae8a47a9b5b4193f93a1) )

	ROM_REGION( 0x8000, "audiocpu", 0 )    /* Sound Z80 code */
	ROM_LOAD( "b65_09.bin",  0x0000, 0x4000, CRC(1bdd4ddc) SHA1(6bf7e3a7ca42f79082503ef471f30f271e2f0f99) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "b65_08.bin",  0x00000, 0x20000, CRC(d2315b37) SHA1(eb42a884df319728c830c067c2423043ed4536ee) )
	ROM_LOAD16_BYTE( "b65_07.bin",  0x00001, 0x20000, CRC(fb83252a) SHA1(48a38584d223f56286137f7acdfaec86ee6588e7) )
	ROM_LOAD16_BYTE( "b65_06.bin",  0x40000, 0x20000, CRC(36cedcbe) SHA1(f79d4b1e98b3c9091ae907fb671ad201d3698b42) )
	ROM_LOAD16_BYTE( "b65_05.bin",  0x40001, 0x20000, CRC(81cd95f1) SHA1(526a437fbe033ac21054ee5c3bf1ba2fed354c7a) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "b65_04.bin",  0x00000, 0x20000, CRC(8c6ff461) SHA1(5199e31f4eb23bad01f7d1079f3618fe39d8a32e) )
	ROM_LOAD16_BYTE( "b65_03.bin",  0x00001, 0x20000, CRC(58b1350b) SHA1(7eb2fe329579a6f651d3c1aed9155ac6ffefbc4b) )
	ROM_LOAD16_BYTE( "b65_02.bin",  0x40000, 0x20000, CRC(1dd55161) SHA1(c537456ac56801dea0ac48fb1389228530d00a61) )
	ROM_LOAD16_BYTE( "b65_01.bin",  0x40001, 0x20000, CRC(e974937f) SHA1(ab282472c04ce6d9ed368956c427403275bc9080) )

	ROM_REGION( 0x40, "proms", 0 )      /* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "b65_12.bpr",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) ) /* sprite attribute (flip/position) ?? */
	ROM_LOAD( "b65_13.bpr",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) ) /* ??? */
ROM_END

ROM_START( hellfire )
	ROM_REGION( 0x040000, "maincpu", 0 )    /* Main 68K code */
	ROM_LOAD16_BYTE( "b90_14.0",   0x000000, 0x20000, CRC(101df9f5) SHA1(27e1430d4c96fe2c830143999a760470c8381ada) )
	ROM_LOAD16_BYTE( "b90_15.1",   0x000001, 0x20000, CRC(e67fd452) SHA1(baec2a702238f000d0499705d79d7c7577fc2279) )

	ROM_REGION( 0x8000, "audiocpu", 0 )    /* Sound Z80 code */
	ROM_LOAD( "b90_03.2",   0x0000, 0x8000, CRC(4058fa67) SHA1(155c364273c270cd74955f447efc804bb4c9b560) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "b90_04.3",   0x00000, 0x20000, CRC(ea6150fc) SHA1(1116947d10ce14fbc6a3b86368fc2024c6f51803) )
	ROM_LOAD16_BYTE( "b90_05.4",   0x00001, 0x20000, CRC(bb52c507) SHA1(b0b1821476647f10c7023f92a66a7f54b92f50c3) )
	ROM_LOAD16_BYTE( "b90_06.5",   0x40000, 0x20000, CRC(cf5b0252) SHA1(e2102967af61afb11d2290a40d13d2faf9ef1e12) )
	ROM_LOAD16_BYTE( "b90_07.6",   0x40001, 0x20000, CRC(b98af263) SHA1(54d636a50a41dbb58b54c22dfab3eabfdb452575) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "b90_11.10",  0x00000, 0x20000, CRC(c33e543c) SHA1(b85cba30cc651f820aeedd41e04584df92078ed9) )
	ROM_LOAD16_BYTE( "b90_10.9",   0x00001, 0x20000, CRC(35fd1092) SHA1(5e136a35eea45034ccd4aea52cc0ffeec944e27e) )
	ROM_LOAD16_BYTE( "b90_09.8",   0x40000, 0x20000, CRC(cf01009e) SHA1(e260c479fa97f23a65c220e5071aaf2dc2baf46d) )
	ROM_LOAD16_BYTE( "b90_08.7",   0x40001, 0x20000, CRC(3404a5e3) SHA1(f717b9e31c2a093dbb060b8ea54a8c3f52688d7a) )

	ROM_REGION( 0x40, "proms", 0 )      /* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "13.3w",     0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )   /* N82S123AN bprom - sprite attribute (flip/position) ?? */
	ROM_LOAD( "12.6b",     0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )   /* N82S123AN bprom -  ??? */
ROM_END

ROM_START( hellfire1 )
	ROM_REGION( 0x040000, "maincpu", 0 )    /* Main 68K code */
	ROM_LOAD16_BYTE( "b90_01.10m",  0x000000, 0x20000, CRC(034966d3) SHA1(f987d8e7ebe6a546be621fe4d5a59de1284c4ebb) )
	ROM_LOAD16_BYTE( "b90_02.9m",   0x000001, 0x20000, CRC(06dd24c7) SHA1(a990de7ffac6bd0dd219c7bf9f773ccb41395be6) )

	ROM_REGION( 0x8000, "audiocpu", 0 )    /* Sound Z80 code */
	ROM_LOAD( "b90_03.2",   0x0000, 0x8000, CRC(4058fa67) SHA1(155c364273c270cd74955f447efc804bb4c9b560) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "b90_04.3",   0x00000, 0x20000, CRC(ea6150fc) SHA1(1116947d10ce14fbc6a3b86368fc2024c6f51803) )
	ROM_LOAD16_BYTE( "b90_05.4",   0x00001, 0x20000, CRC(bb52c507) SHA1(b0b1821476647f10c7023f92a66a7f54b92f50c3) )
	ROM_LOAD16_BYTE( "b90_06.5",   0x40000, 0x20000, CRC(cf5b0252) SHA1(e2102967af61afb11d2290a40d13d2faf9ef1e12) )
	ROM_LOAD16_BYTE( "b90_07.6",   0x40001, 0x20000, CRC(b98af263) SHA1(54d636a50a41dbb58b54c22dfab3eabfdb452575) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "b90_11.10",  0x00000, 0x20000, CRC(c33e543c) SHA1(b85cba30cc651f820aeedd41e04584df92078ed9) )
	ROM_LOAD16_BYTE( "b90_10.9",   0x00001, 0x20000, CRC(35fd1092) SHA1(5e136a35eea45034ccd4aea52cc0ffeec944e27e) )
	ROM_LOAD16_BYTE( "b90_09.8",   0x40000, 0x20000, CRC(cf01009e) SHA1(e260c479fa97f23a65c220e5071aaf2dc2baf46d) )
	ROM_LOAD16_BYTE( "b90_08.7",   0x40001, 0x20000, CRC(3404a5e3) SHA1(f717b9e31c2a093dbb060b8ea54a8c3f52688d7a) )

	ROM_REGION( 0x40, "proms", 0 )      /* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "13.3w",     0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )   /* N82S123AN bprom - sprite attribute (flip/position) ?? */
	ROM_LOAD( "12.6b",     0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )   /* N82S123AN bprom -  ??? */
ROM_END

ROM_START( hellfire2a )
	ROM_REGION( 0x040000, "maincpu", 0 )    /* Main 68K code */
	ROM_LOAD16_BYTE( "b90_01.0",   0x000000, 0x20000, CRC(c94acf53) SHA1(5710861dbe976fe53b93d3428147d1ce7aaae18a) ) /* Same labels as hellfire1 but different data */
	ROM_LOAD16_BYTE( "b90_02.1",   0x000001, 0x20000, CRC(d17f03c3) SHA1(ac41e6c29aa507872caeeaec6a3bc24c705a3702) )

	ROM_REGION( 0x8000, "audiocpu", 0 )    /* Sound Z80 code */
	ROM_LOAD( "b90_03.2",   0x0000, 0x8000, CRC(4058fa67) SHA1(155c364273c270cd74955f447efc804bb4c9b560) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "b90_04.3",   0x00000, 0x20000, CRC(ea6150fc) SHA1(1116947d10ce14fbc6a3b86368fc2024c6f51803) )
	ROM_LOAD16_BYTE( "b90_05.4",   0x00001, 0x20000, CRC(bb52c507) SHA1(b0b1821476647f10c7023f92a66a7f54b92f50c3) )
	ROM_LOAD16_BYTE( "b90_06.5",   0x40000, 0x20000, CRC(cf5b0252) SHA1(e2102967af61afb11d2290a40d13d2faf9ef1e12) )
	ROM_LOAD16_BYTE( "b90_07.6",   0x40001, 0x20000, CRC(b98af263) SHA1(54d636a50a41dbb58b54c22dfab3eabfdb452575) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "b90_11.10",  0x00000, 0x20000, CRC(c33e543c) SHA1(b85cba30cc651f820aeedd41e04584df92078ed9) )
	ROM_LOAD16_BYTE( "b90_10.9",   0x00001, 0x20000, CRC(35fd1092) SHA1(5e136a35eea45034ccd4aea52cc0ffeec944e27e) )
	ROM_LOAD16_BYTE( "b90_09.8",   0x40000, 0x20000, CRC(cf01009e) SHA1(e260c479fa97f23a65c220e5071aaf2dc2baf46d) )
	ROM_LOAD16_BYTE( "b90_08.7",   0x40001, 0x20000, CRC(3404a5e3) SHA1(f717b9e31c2a093dbb060b8ea54a8c3f52688d7a) )

	ROM_REGION( 0x40, "proms", 0 )      /* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "13.3w",     0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )   /* N82S123AN bprom - sprite attribute (flip/position) ?? */
	ROM_LOAD( "12.6b",     0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )   /* N82S123AN bprom -  ??? */
ROM_END

ROM_START( hellfire1a )
	ROM_REGION( 0x040000, "maincpu", 0 )    /* Main 68K code */
	ROM_LOAD16_BYTE( "b90_14x.0",   0x000000, 0x20000, CRC(a3141ea5) SHA1(9b456cb908e193198110a628d98567a3b8351591) ) /* Wrong labels ! This set is probably the oldest */
	ROM_LOAD16_BYTE( "b90_15x.1",   0x000001, 0x20000, CRC(e864daf4) SHA1(382f02df8419310cef5d7fb68a9376eeac2f3685) ) /* and definitely is older than 'hellfire1' */

	ROM_REGION( 0x8000, "audiocpu", 0 )    /* Sound Z80 code */
	ROM_LOAD( "b90_03x.2",  0x0000, 0x8000, CRC(f58c368f) SHA1(2ee5396a4b70a3374f3a3bbd791b1d962f6a8a52) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "b90_04.3",   0x00000, 0x20000, CRC(ea6150fc) SHA1(1116947d10ce14fbc6a3b86368fc2024c6f51803) )
	ROM_LOAD16_BYTE( "b90_05.4",   0x00001, 0x20000, CRC(bb52c507) SHA1(b0b1821476647f10c7023f92a66a7f54b92f50c3) )
	ROM_LOAD16_BYTE( "b90_06.5",   0x40000, 0x20000, CRC(cf5b0252) SHA1(e2102967af61afb11d2290a40d13d2faf9ef1e12) )
	ROM_LOAD16_BYTE( "b90_07.6",   0x40001, 0x20000, CRC(b98af263) SHA1(54d636a50a41dbb58b54c22dfab3eabfdb452575) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "b90_11.10",  0x00000, 0x20000, CRC(c33e543c) SHA1(b85cba30cc651f820aeedd41e04584df92078ed9) )
	ROM_LOAD16_BYTE( "b90_10.9",   0x00001, 0x20000, CRC(35fd1092) SHA1(5e136a35eea45034ccd4aea52cc0ffeec944e27e) )
	ROM_LOAD16_BYTE( "b90_09.8",   0x40000, 0x20000, CRC(cf01009e) SHA1(e260c479fa97f23a65c220e5071aaf2dc2baf46d) )
	ROM_LOAD16_BYTE( "b90_08.7",   0x40001, 0x20000, CRC(3404a5e3) SHA1(f717b9e31c2a093dbb060b8ea54a8c3f52688d7a) )

	ROM_REGION( 0x40, "proms", 0 )      /* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "13.3w",     0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )   /* N82S123AN bprom - sprite attribute (flip/position) ?? */
	ROM_LOAD( "12.6b",     0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )   /* N82S123AN bprom -  ??? */
ROM_END

ROM_START( zerowing ) /* 2 player simultaneous version */
	ROM_REGION( 0x080000, "maincpu", 0 )    /* Main 68K code */
	ROM_LOAD16_BYTE( "o15-11ii.bin",  0x000000, 0x08000, CRC(e697ecb9) SHA1(444bf4c06844bd119cf152a35a5483e0f0dcecd4) )
	ROM_LOAD16_BYTE( "o15-12ii.bin",  0x000001, 0x08000, CRC(b29ee3ad) SHA1(631695dfe5c2ee39effcfa4312ea6c14f7b7c302) )
	ROM_LOAD16_BYTE( "o15-09.rom",    0x040000, 0x20000, CRC(13764e95) SHA1(61da49b73ba81edd951e96e9ce6673c1c3bd65f2) )
	ROM_LOAD16_BYTE( "o15-10.rom",    0x040001, 0x20000, CRC(351ba71a) SHA1(937331549140506711b08252497cc0f2efa58268) )

	ROM_REGION( 0x8000, "audiocpu", 0 )    /* Sound Z80 code */
	ROM_LOAD( "o15-13.rom",  0x0000, 0x8000, CRC(e7b72383) SHA1(ea1f6f33a86d14d58bd396fd46081462f00177d5) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "o15-05.rom",  0x00000, 0x20000, CRC(4e5dd246) SHA1(5366b4a6f3c900a4f57a6583b7399163a06f42d7) )
	ROM_LOAD16_BYTE( "o15-06.rom",  0x00001, 0x20000, CRC(c8c6d428) SHA1(76ee5bcb8f10fe201fc5c32697beee3de9d8b751) )
	ROM_LOAD16_BYTE( "o15-07.rom",  0x40000, 0x20000, CRC(efc40e99) SHA1(a04fad4197a7fb4787cd9bebf43e1d9b02b2f61b) )
	ROM_LOAD16_BYTE( "o15-08.rom",  0x40001, 0x20000, CRC(1b019eab) SHA1(c9569ca85696825142acc5cde9ac829e82b1ca1b) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "o15-03.rom",  0x00000, 0x20000, CRC(7f245fd3) SHA1(efbcb3663d4accc4f8128a8fee5475bc109bc17a) )
	ROM_LOAD16_BYTE( "o15-04.rom",  0x00001, 0x20000, CRC(0b1a1289) SHA1(ce6c06342392d11952873e3b1d6aea8dc02a551c) )
	ROM_LOAD16_BYTE( "o15-01.rom",  0x40000, 0x20000, CRC(70570e43) SHA1(acc9baec71b0930cb2f193677e0663efa5d5551d) )
	ROM_LOAD16_BYTE( "o15-02.rom",  0x40001, 0x20000, CRC(724b487f) SHA1(06af31520866eea69aebbd5d428f80e882289a15) )

	ROM_REGION( 0x40, "proms", 0 )      /* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "tp015_14.bpr",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )   /* sprite attribute (flip/position) ?? */
	ROM_LOAD( "tp015_15.bpr",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )   /* ??? */
ROM_END

ROM_START( zerowing1 ) /* 1 player version */
	ROM_REGION( 0x080000, "maincpu", 0 )    /* Main 68K code */
	ROM_LOAD16_BYTE( "o15-11.rom",  0x000000, 0x08000, CRC(6ff2b9a0) SHA1(c9f2a631f185689dfc42a451d85fac23c2f4b64b) )
	ROM_LOAD16_BYTE( "o15-12.rom",  0x000001, 0x08000, CRC(9773e60b) SHA1(b733e9d38a233d010cc5ea41e7e61695082c3a22) )
	ROM_LOAD16_BYTE( "o15-09.rom",  0x040000, 0x20000, CRC(13764e95) SHA1(61da49b73ba81edd951e96e9ce6673c1c3bd65f2) )
	ROM_LOAD16_BYTE( "o15-10.rom",  0x040001, 0x20000, CRC(351ba71a) SHA1(937331549140506711b08252497cc0f2efa58268) )

	ROM_REGION( 0x8000, "audiocpu", 0 )    /* Sound Z80 code */
	ROM_LOAD( "o15-13.rom",  0x0000, 0x8000, CRC(e7b72383) SHA1(ea1f6f33a86d14d58bd396fd46081462f00177d5) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "o15-05.rom",  0x00000, 0x20000, CRC(4e5dd246) SHA1(5366b4a6f3c900a4f57a6583b7399163a06f42d7) )
	ROM_LOAD16_BYTE( "o15-06.rom",  0x00001, 0x20000, CRC(c8c6d428) SHA1(76ee5bcb8f10fe201fc5c32697beee3de9d8b751) )
	ROM_LOAD16_BYTE( "o15-07.rom",  0x40000, 0x20000, CRC(efc40e99) SHA1(a04fad4197a7fb4787cd9bebf43e1d9b02b2f61b) )
	ROM_LOAD16_BYTE( "o15-08.rom",  0x40001, 0x20000, CRC(1b019eab) SHA1(c9569ca85696825142acc5cde9ac829e82b1ca1b) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "o15-03.rom",  0x00000, 0x20000, CRC(7f245fd3) SHA1(efbcb3663d4accc4f8128a8fee5475bc109bc17a) )
	ROM_LOAD16_BYTE( "o15-04.rom",  0x00001, 0x20000, CRC(0b1a1289) SHA1(ce6c06342392d11952873e3b1d6aea8dc02a551c) )
	ROM_LOAD16_BYTE( "o15-01.rom",  0x40000, 0x20000, CRC(70570e43) SHA1(acc9baec71b0930cb2f193677e0663efa5d5551d) )
	ROM_LOAD16_BYTE( "o15-02.rom",  0x40001, 0x20000, CRC(724b487f) SHA1(06af31520866eea69aebbd5d428f80e882289a15) )

	ROM_REGION( 0x40, "proms", 0 )      /* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "tp015_14.bpr",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )   /* sprite attribute (flip/position) ?? */
	ROM_LOAD( "tp015_15.bpr",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )   /* ??? */
ROM_END

ROM_START( zerowingw ) /* 2 player simultaneous version (Williams Electronics) */
	ROM_REGION( 0x080000, "maincpu", 0 )    /* Main 68K code */
	ROM_LOAD16_BYTE( "o15-11iiw.bin",  0x000000, 0x08000, CRC(38b0bb5b) SHA1(e5a4c0b6c279a55701c82bf9e285a806054f8d23) )
	ROM_LOAD16_BYTE( "o15-12iiw.bin",  0x000001, 0x08000, CRC(74c91e6f) SHA1(8cf5d10a5f4efda0903a4c5d56599861ccc8f1c1) )
	ROM_LOAD16_BYTE( "o15-09.rom",     0x040000, 0x20000, CRC(13764e95) SHA1(61da49b73ba81edd951e96e9ce6673c1c3bd65f2) )
	ROM_LOAD16_BYTE( "o15-10.rom",     0x040001, 0x20000, CRC(351ba71a) SHA1(937331549140506711b08252497cc0f2efa58268) )

	ROM_REGION( 0x8000, "audiocpu", 0 )    /* Sound Z80 code */
	ROM_LOAD( "o15-13.rom",  0x0000, 0x8000, CRC(e7b72383) SHA1(ea1f6f33a86d14d58bd396fd46081462f00177d5) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "o15-05.rom",  0x00000, 0x20000, CRC(4e5dd246) SHA1(5366b4a6f3c900a4f57a6583b7399163a06f42d7) )
	ROM_LOAD16_BYTE( "o15-06.rom",  0x00001, 0x20000, CRC(c8c6d428) SHA1(76ee5bcb8f10fe201fc5c32697beee3de9d8b751) )
	ROM_LOAD16_BYTE( "o15-07.rom",  0x40000, 0x20000, CRC(efc40e99) SHA1(a04fad4197a7fb4787cd9bebf43e1d9b02b2f61b) )
	ROM_LOAD16_BYTE( "o15-08.rom",  0x40001, 0x20000, CRC(1b019eab) SHA1(c9569ca85696825142acc5cde9ac829e82b1ca1b) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "o15-03.rom",  0x00000, 0x20000, CRC(7f245fd3) SHA1(efbcb3663d4accc4f8128a8fee5475bc109bc17a) )
	ROM_LOAD16_BYTE( "o15-04.rom",  0x00001, 0x20000, CRC(0b1a1289) SHA1(ce6c06342392d11952873e3b1d6aea8dc02a551c) )
	ROM_LOAD16_BYTE( "o15-01.rom",  0x40000, 0x20000, CRC(70570e43) SHA1(acc9baec71b0930cb2f193677e0663efa5d5551d) )
	ROM_LOAD16_BYTE( "o15-02.rom",  0x40001, 0x20000, CRC(724b487f) SHA1(06af31520866eea69aebbd5d428f80e882289a15) )

	ROM_REGION( 0x40, "proms", 0 )      /* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "tp015_14.bpr",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )   /* sprite attribute (flip/position) ?? */
	ROM_LOAD( "tp015_15.bpr",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )   /* ??? */
ROM_END

ROM_START( demonwld )
	ROM_REGION( 0x040000, "maincpu", 0 )    /* Main 68K code */
	ROM_LOAD16_BYTE( "o16-10.v2", 0x000000, 0x20000, CRC(ca8194f3) SHA1(176da6739b35ba38b40150fc62380108bcae5a24) )
	ROM_LOAD16_BYTE( "o16-09.v2", 0x000001, 0x20000, CRC(7baea7ba) SHA1(ae2b40f9efb4440ff7edbcc4f80641655f7c4671) )

	ROM_REGION( 0x8000, "audiocpu", 0 )    /* Sound Z80 code */
	ROM_LOAD( "rom11.v2",  0x0000, 0x8000, CRC(dbe08c85) SHA1(536a242bfe916d15744b079261507af6f12b5b50) )

	ROM_REGION( 0x2000, "dsp", 0 )  /* Co-Processor TMS320C10 MCU code */
	ROM_LOAD16_BYTE( "dsp_21.bin",  0x0000, 0x0800, CRC(2d135376) SHA1(67a2cc774d272ee1cd6e6bc1c5fc33fc6968837e) )
	ROM_LOAD16_BYTE( "dsp_22.bin",  0x0001, 0x0800, CRC(79389a71) SHA1(14ec4c1c9b06702319e89a7a250d0038393437f4) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "rom05",  0x00000, 0x20000, CRC(6506c982) SHA1(6d4c1ef91e5617724789ff196abb7abf23e4a7fb) )
	ROM_LOAD16_BYTE( "rom07",  0x00001, 0x20000, CRC(a3a0d993) SHA1(50311b9447eb04271b17b212ca31d083ab5b2414) )
	ROM_LOAD16_BYTE( "rom06",  0x40000, 0x20000, CRC(4fc5e5f3) SHA1(725d4b009d575ff8ffbe1c00df352ccf235465d7) )
	ROM_LOAD16_BYTE( "rom08",  0x40001, 0x20000, CRC(eb53ab09) SHA1(d98195cc1b65b76335b5b24adb31deae1b313f3a) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "rom01",  0x00000, 0x20000, CRC(1b3724e9) SHA1(3dbb0450ab1e40e6df2b7c7356352419cd3f113d) )
	ROM_LOAD16_BYTE( "rom02",  0x00001, 0x20000, CRC(7b20a44d) SHA1(4dc1a2fa2058077b112c73492808ee9381060ec7) )
	ROM_LOAD16_BYTE( "rom03",  0x40000, 0x20000, CRC(2cacdcd0) SHA1(92216d1c6859e05d39363c30e0beb45bc0ae4e1c) )
	ROM_LOAD16_BYTE( "rom04",  0x40001, 0x20000, CRC(76fd3201) SHA1(7a12737bf90bd9760074132edeb22f3fd3e16b4f) )

	ROM_REGION( 0x40, "proms", 0 )      /* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "prom12.bpr",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) ) /* sprite attribute (flip/position) ?? */
	ROM_LOAD( "prom13.bpr",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) ) /* ??? */
ROM_END

ROM_START( demonwld1 )
	ROM_REGION( 0x040000, "maincpu", 0 )    /* Main 68K code */
	ROM_LOAD16_BYTE( "o16-10.rom", 0x000000, 0x20000, CRC(036ee46c) SHA1(60868e5e08e0c9a538ae786de0de6b2531b30b11) )
	ROM_LOAD16_BYTE( "o16-09.rom", 0x000001, 0x20000, CRC(bed746e3) SHA1(056668edb7df99bbd240e387af17cf252d1448f3) )

	ROM_REGION( 0x8000, "audiocpu", 0 )    /* Sound Z80 code */
	ROM_LOAD( "rom11",  0x0000, 0x8000, CRC(397eca1b) SHA1(84073ff6d1bc46ec6162d66ec5de305700938380) )

	ROM_REGION( 0x2000, "dsp", 0 )  /* Co-Processor TMS320C10 MCU code */
	ROM_LOAD16_BYTE( "dsp_21.bin",  0x0000, 0x0800, CRC(2d135376) SHA1(67a2cc774d272ee1cd6e6bc1c5fc33fc6968837e) )
	ROM_LOAD16_BYTE( "dsp_22.bin",  0x0001, 0x0800, CRC(79389a71) SHA1(14ec4c1c9b06702319e89a7a250d0038393437f4) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "rom05",  0x00000, 0x20000, CRC(6506c982) SHA1(6d4c1ef91e5617724789ff196abb7abf23e4a7fb) )
	ROM_LOAD16_BYTE( "rom07",  0x00001, 0x20000, CRC(a3a0d993) SHA1(50311b9447eb04271b17b212ca31d083ab5b2414) )
	ROM_LOAD16_BYTE( "rom06",  0x40000, 0x20000, CRC(4fc5e5f3) SHA1(725d4b009d575ff8ffbe1c00df352ccf235465d7) )
	ROM_LOAD16_BYTE( "rom08",  0x40001, 0x20000, CRC(eb53ab09) SHA1(d98195cc1b65b76335b5b24adb31deae1b313f3a) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "rom01",  0x00000, 0x20000, CRC(1b3724e9) SHA1(3dbb0450ab1e40e6df2b7c7356352419cd3f113d) )
	ROM_LOAD16_BYTE( "rom02",  0x00001, 0x20000, CRC(7b20a44d) SHA1(4dc1a2fa2058077b112c73492808ee9381060ec7) )
	ROM_LOAD16_BYTE( "rom03",  0x40000, 0x20000, CRC(2cacdcd0) SHA1(92216d1c6859e05d39363c30e0beb45bc0ae4e1c) )
	ROM_LOAD16_BYTE( "rom04",  0x40001, 0x20000, CRC(76fd3201) SHA1(7a12737bf90bd9760074132edeb22f3fd3e16b4f) )

	ROM_REGION( 0x40, "proms", 0 )      /* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "prom12.bpr",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) ) /* sprite attribute (flip/position) ?? */
	ROM_LOAD( "prom13.bpr",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) ) /* ??? */
ROM_END

ROM_START( demonwld2 )
	ROM_REGION( 0x040000, "maincpu", 0 )    /* Main 68K code */
	ROM_LOAD16_BYTE( "o16-10-2.bin", 0x000000, 0x20000, CRC(84ee5218) SHA1(dc2b017ee630330163be320008d8a0d761cb0cfb) ) // aka o16_10ii
	ROM_LOAD16_BYTE( "o16-09-2.bin", 0x000001, 0x20000, CRC(cf474cb2) SHA1(5c049082b8d7118e0d2e50c6ae07f9d3d0110498) ) // aka o16_09ii

	ROM_REGION( 0x8000, "audiocpu", 0 )    /* Sound Z80 code */
	ROM_LOAD( "rom11",  0x0000, 0x8000, CRC(397eca1b) SHA1(84073ff6d1bc46ec6162d66ec5de305700938380) )

	ROM_REGION( 0x2000, "dsp", 0 )  /* Co-Processor TMS320C10 MCU code */
	ROM_LOAD16_BYTE( "dsp_21.bin",  0x0000, 0x0800, CRC(2d135376) SHA1(67a2cc774d272ee1cd6e6bc1c5fc33fc6968837e) )
	ROM_LOAD16_BYTE( "dsp_22.bin",  0x0001, 0x0800, CRC(79389a71) SHA1(14ec4c1c9b06702319e89a7a250d0038393437f4) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "rom05",  0x00000, 0x20000, CRC(6506c982) SHA1(6d4c1ef91e5617724789ff196abb7abf23e4a7fb) )
	ROM_LOAD16_BYTE( "rom07",  0x00001, 0x20000, CRC(a3a0d993) SHA1(50311b9447eb04271b17b212ca31d083ab5b2414) )
	ROM_LOAD16_BYTE( "rom06",  0x40000, 0x20000, CRC(4fc5e5f3) SHA1(725d4b009d575ff8ffbe1c00df352ccf235465d7) )
	ROM_LOAD16_BYTE( "rom08",  0x40001, 0x20000, CRC(eb53ab09) SHA1(d98195cc1b65b76335b5b24adb31deae1b313f3a) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "rom01",  0x00000, 0x20000, CRC(1b3724e9) SHA1(3dbb0450ab1e40e6df2b7c7356352419cd3f113d) )
	ROM_LOAD16_BYTE( "rom02",  0x00001, 0x20000, CRC(7b20a44d) SHA1(4dc1a2fa2058077b112c73492808ee9381060ec7) )
	ROM_LOAD16_BYTE( "rom03",  0x40000, 0x20000, CRC(2cacdcd0) SHA1(92216d1c6859e05d39363c30e0beb45bc0ae4e1c) )
	ROM_LOAD16_BYTE( "rom04",  0x40001, 0x20000, CRC(76fd3201) SHA1(7a12737bf90bd9760074132edeb22f3fd3e16b4f) )

	ROM_REGION( 0x40, "proms", 0 )      /* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "prom12.bpr",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) ) /* sprite attribute (flip/position) ?? */
	ROM_LOAD( "prom13.bpr",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) ) /* ??? */
ROM_END

ROM_START( demonwld3 )
	ROM_REGION( 0x040000, "maincpu", 0 )    /* Main 68K code */
	ROM_LOAD16_BYTE( "o16-10.bin", 0x000000, 0x20000, CRC(6f7468e0) SHA1(87ef7733fd0d00d0d375dbf30332cf0614480dc2) )
	ROM_LOAD16_BYTE( "o16-09.bin", 0x000001, 0x20000, CRC(a572f5f7) SHA1(3d6a443cecd46734c7e1b761130909482c7a9914) )

	ROM_REGION( 0x8000, "audiocpu", 0 )    /* Sound Z80 code */
	ROM_LOAD( "rom11",  0x0000, 0x8000, CRC(397eca1b) SHA1(84073ff6d1bc46ec6162d66ec5de305700938380) )

	ROM_REGION( 0x2000, "dsp", 0 )  /* Co-Processor TMS320C10 MCU code */
	ROM_LOAD16_BYTE( "dsp_21.bin",  0x0000, 0x0800, CRC(2d135376) SHA1(67a2cc774d272ee1cd6e6bc1c5fc33fc6968837e) )
	ROM_LOAD16_BYTE( "dsp_22.bin",  0x0001, 0x0800, CRC(79389a71) SHA1(14ec4c1c9b06702319e89a7a250d0038393437f4) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "rom05",  0x00000, 0x20000, CRC(6506c982) SHA1(6d4c1ef91e5617724789ff196abb7abf23e4a7fb) )
	ROM_LOAD16_BYTE( "rom07",  0x00001, 0x20000, CRC(a3a0d993) SHA1(50311b9447eb04271b17b212ca31d083ab5b2414) )
	ROM_LOAD16_BYTE( "rom06",  0x40000, 0x20000, CRC(4fc5e5f3) SHA1(725d4b009d575ff8ffbe1c00df352ccf235465d7) )
	ROM_LOAD16_BYTE( "rom08",  0x40001, 0x20000, CRC(eb53ab09) SHA1(d98195cc1b65b76335b5b24adb31deae1b313f3a) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "rom01",  0x00000, 0x20000, CRC(1b3724e9) SHA1(3dbb0450ab1e40e6df2b7c7356352419cd3f113d) )
	ROM_LOAD16_BYTE( "rom02",  0x00001, 0x20000, CRC(7b20a44d) SHA1(4dc1a2fa2058077b112c73492808ee9381060ec7) )
	ROM_LOAD16_BYTE( "rom03",  0x40000, 0x20000, CRC(2cacdcd0) SHA1(92216d1c6859e05d39363c30e0beb45bc0ae4e1c) )
	ROM_LOAD16_BYTE( "rom04",  0x40001, 0x20000, CRC(76fd3201) SHA1(7a12737bf90bd9760074132edeb22f3fd3e16b4f) )

	ROM_REGION( 0x40, "proms", 0 )      /* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "prom12.bpr",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) ) /* sprite attribute (flip/position) ?? */
	ROM_LOAD( "prom13.bpr",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) ) /* ??? */
ROM_END

ROM_START( samesame )
	ROM_REGION( 0x080000, "maincpu", 0 )    /* Main 68K code */
	ROM_LOAD16_BYTE( "o17_09.8j",  0x000000, 0x08000, CRC(3f69e437) SHA1(f2a40fd42cb5ecb2e514b72e7550aa479a9f9ad6) )
	ROM_LOAD16_BYTE( "o17_10.8l",  0x000001, 0x08000, CRC(4e723e0a) SHA1(e06394d50addeda1045c02c646964afbc6005a82) )
	ROM_LOAD16_BYTE( "o17_11.7j",  0x040000, 0x20000, CRC(be07d101) SHA1(1eda14ba24532b565d6ad57490b73ff312f98b53) )
	ROM_LOAD16_BYTE( "o17_12.7l",  0x040001, 0x20000, CRC(ef698811) SHA1(4c729704eba0bf469599c79009327e4fa5dc540b) )

	ROM_REGION( 0x8000, "audiocpu", 0 )    /* Sound HD647180 code */
	/* sound CPU is a HD647180 (Z180) with internal ROM - not yet supported */
	ROM_LOAD( "hd647180.017",  0x00000, 0x08000, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "o17_05.12j",  0x00000, 0x20000, CRC(565315f8) SHA1(6b1c5ef52359483228b329c89c2e1174e3fbf017) )
	ROM_LOAD16_BYTE( "o17_06.13j",  0x00001, 0x20000, CRC(95262d4c) SHA1(16f3aabecb1c87ce7eadf4f0ff61b29a4c017614) )
	ROM_LOAD16_BYTE( "o17_07.12l",  0x40000, 0x20000, CRC(4c4b735c) SHA1(812c3bf46bd7764b2bb812bd2b9eb0331ed257ae) )
	ROM_LOAD16_BYTE( "o17_08.13l",  0x40001, 0x20000, CRC(95c6586c) SHA1(ff87901f79d80f73ad09664b0c0d892898570616) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "o17_01.1d",  0x00000, 0x20000, CRC(ea12e491) SHA1(02190722b7c5383471e0af9596be7039a5367240) )
	ROM_LOAD16_BYTE( "o17_02.3d",  0x00001, 0x20000, CRC(32a13a9f) SHA1(1446acdfd21cd41f3d97aaf30f498c0c5d890605) )
	ROM_LOAD16_BYTE( "o17_03.5d",  0x40000, 0x20000, CRC(68723dc9) SHA1(4f1b7aa2469c955e03737b611a7d2524f1e4f61e) )
	ROM_LOAD16_BYTE( "o17_04.7d",  0x40001, 0x20000, CRC(fe0ecb13) SHA1(634a49262b9c092c25f11b14c6757fe94ea9eddc) )

	ROM_REGION( 0x40, "proms", 0 )      /* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "prom14.25b",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) ) /* sprite attribute (flip/position) ?? */
	ROM_LOAD( "prom15.20c",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) ) /* ??? */
ROM_END

ROM_START( samesame2 )
	ROM_REGION( 0x080000, "maincpu", 0 )    /* Main 68K code */
	ROM_LOAD16_BYTE( "o17_09x.8j",  0x000000, 0x08000, CRC(3472e03e) SHA1(a0f12622a1963bfac2d5f357afbfb5d7db2cd8df) )
	ROM_LOAD16_BYTE( "o17_10x.8l",  0x000001, 0x08000, CRC(a3ac49b5) SHA1(c5adf026b9129b64acee5a079e102377a8488220) )
	ROM_LOAD16_BYTE( "o17_11ii.7j", 0x040000, 0x20000, CRC(6beac378) SHA1(041ba98a89a4bac32575858db8a061bdf7804594) )
	ROM_LOAD16_BYTE( "o17_12ii.7l", 0x040001, 0x20000, CRC(6adb6eb5) SHA1(9b6e63aa50d271c2bb0b4cf822fc6f3684f10230) )

	ROM_REGION( 0x8000, "audiocpu", 0 )    /* Sound HD647180 code */
	/* sound CPU is a HD647180 (Z180) with internal ROM - not yet supported */
	ROM_LOAD( "hd647180.017",  0x00000, 0x08000, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "o17_05.12j",  0x00000, 0x20000, CRC(565315f8) SHA1(6b1c5ef52359483228b329c89c2e1174e3fbf017) )
	ROM_LOAD16_BYTE( "o17_06.13j",  0x00001, 0x20000, CRC(95262d4c) SHA1(16f3aabecb1c87ce7eadf4f0ff61b29a4c017614) )
	ROM_LOAD16_BYTE( "o17_07.12l",  0x40000, 0x20000, CRC(4c4b735c) SHA1(812c3bf46bd7764b2bb812bd2b9eb0331ed257ae) )
	ROM_LOAD16_BYTE( "o17_08.13l",  0x40001, 0x20000, CRC(95c6586c) SHA1(ff87901f79d80f73ad09664b0c0d892898570616) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "o17_01.1d",  0x00000, 0x20000, CRC(ea12e491) SHA1(02190722b7c5383471e0af9596be7039a5367240) )
	ROM_LOAD16_BYTE( "o17_02.3d",  0x00001, 0x20000, CRC(32a13a9f) SHA1(1446acdfd21cd41f3d97aaf30f498c0c5d890605) )
	ROM_LOAD16_BYTE( "o17_03.5d",  0x40000, 0x20000, CRC(68723dc9) SHA1(4f1b7aa2469c955e03737b611a7d2524f1e4f61e) )
	ROM_LOAD16_BYTE( "o17_04.7d",  0x40001, 0x20000, CRC(fe0ecb13) SHA1(634a49262b9c092c25f11b14c6757fe94ea9eddc) )

	ROM_REGION( 0x40, "proms", 0 )      /* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "prom14.25b",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) ) /* sprite attribute (flip/position) ?? */
	ROM_LOAD( "prom15.20c",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) ) /* ??? */
ROM_END

/*
Fire Shark
Toaplan 1990

PCB Layout
----------

TOAPLAN Co., Ltd.
TP-O17
|---------------------------------------------------------|
|MB3730 YM3812 10MHz                     O17_01           |
| YM3014                    MN53007      O17_02      6264 |
|VOL  LM358 |---------------|            O17_03           |
|           |    68000      |            O17_04      6264 |
|  647180   |---------------|                             |
|            O17_12   O17_11                         6264 |
|            O17_10   O17_09    2018  2018                |
|J           6264       6264    2018  2018           6264 |
|A           O17_07   O17_05                              |
|M           O17_08   O17_06                         6264 |
|M                                              |------|  |
|A           DSW1    |------|                   |FOU-2 |  |
|                    |BCU-2 |   8464            |      |  |
|            DSW2    |      |   8464            |------|  |
|                    |------|   8464        PROM15        |
|            D65024             8464               6116   |
|                               8464               6116   |
|                     BCU       8464               6116   |
|            6116               8464              PROM14  |
|            6116               8464         28MHz        |
|---------------------------------------------------------|
Notes:
        68000 - Motorola MC68000P10 CPU. Clock input 10.000MHz
       647180 - Hitachi HD647180XOFS6 micro-controller with 16k internal OTP EPROM and 512 bytes internal RAM. Clock input 10.000MHz on pin 75
       YM3812 - Yamaha YM3812 FM operator type-L II (OPL II) LSI (DIP24). Clock input 3.500MHz [28/8]
       YM3014 - Yamaha YM3014 Serial Input Floating D/A Converter (DIP8)
         2018 - Motorola MCM2018AN45 2kx8 SRAM (NDIP24)
         8464 - Fujitsu MB8464A-10L 8kx8 SRAM (NDIP28)
         6464 - Hyundai HY6264LP-10 8kx8 SRAM (DIP28)
         6116 - Hyundai HY6116AP-15 2kx8 SRAM (DIP24)
        BCU-2 - Custom graphics IC (QFP160)
        FOU-2 - Custom graphics IC (QFP136)
        LM358 - National Semiconductor LM358 Dual Operational Amplifier (DIP8)
       D65024 - NEC D65024GF035 uPD65000-series CMOS Gate Array (QFP100)
      MN53007 - Panasonic MN53007 CMOS Gate Array {732 gates} (DIP42)
    PROM14/15 - Philips/Signetics N82S123 Bipolar PROM (DIP16)
       MB3730 - Fujitsu MB3730 14W BTL Audio Power Amplifier
       DSW1/2 - 8-position DIP switch
          BCU - Unpopulated position for PGA177 IC
        HSYNC - 14.86496kHz
        VSYNC - 57.61308Hz
*/

ROM_START( fireshrk )
	ROM_REGION( 0x080000, "maincpu", 0 )    /* Main 68K code */
	ROM_LOAD16_BYTE( "09.8j",       0x000000, 0x08000, CRC(f0c70e6f) SHA1(037690448786d61aa116b24b638430c577ea78e2) )
	ROM_LOAD16_BYTE( "10.8l",       0x000001, 0x08000, CRC(9d253d77) SHA1(0414d1f475abb9ccfd7daa11c2f400a14f25db09) )
	ROM_LOAD16_BYTE( "o17_11ii.7j", 0x040000, 0x20000, CRC(6beac378) SHA1(041ba98a89a4bac32575858db8a061bdf7804594) )
	ROM_LOAD16_BYTE( "o17_12ii.7l", 0x040001, 0x20000, CRC(6adb6eb5) SHA1(9b6e63aa50d271c2bb0b4cf822fc6f3684f10230) )

	ROM_REGION( 0x8000, "audiocpu", 0 )    /* Sound HD647180 code */
	/* sound CPU is a HD647180 (Z180) with internal ROM - not yet supported */
	ROM_LOAD( "hd647180.017",  0x00000, 0x08000, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "o17_05.12j",  0x00000, 0x20000, CRC(565315f8) SHA1(6b1c5ef52359483228b329c89c2e1174e3fbf017) )
	ROM_LOAD16_BYTE( "o17_06.13j",  0x00001, 0x20000, CRC(95262d4c) SHA1(16f3aabecb1c87ce7eadf4f0ff61b29a4c017614) )
	ROM_LOAD16_BYTE( "o17_07.12l",  0x40000, 0x20000, CRC(4c4b735c) SHA1(812c3bf46bd7764b2bb812bd2b9eb0331ed257ae) )
	ROM_LOAD16_BYTE( "o17_08.13l",  0x40001, 0x20000, CRC(95c6586c) SHA1(ff87901f79d80f73ad09664b0c0d892898570616) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "o17_01.1d",  0x00000, 0x20000, CRC(ea12e491) SHA1(02190722b7c5383471e0af9596be7039a5367240) )
	ROM_LOAD16_BYTE( "o17_02.3d",  0x00001, 0x20000, CRC(32a13a9f) SHA1(1446acdfd21cd41f3d97aaf30f498c0c5d890605) )
	ROM_LOAD16_BYTE( "o17_03.5d",  0x40000, 0x20000, CRC(68723dc9) SHA1(4f1b7aa2469c955e03737b611a7d2524f1e4f61e) )
	ROM_LOAD16_BYTE( "o17_04.7d",  0x40001, 0x20000, CRC(fe0ecb13) SHA1(634a49262b9c092c25f11b14c6757fe94ea9eddc) )

	ROM_REGION( 0x40, "proms", 0 )      /* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "prom14.25b",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) ) /* sprite attribute (flip/position) ?? */
	ROM_LOAD( "prom15.20c",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) ) /* ??? */
ROM_END

ROM_START( fireshrka )
	ROM_REGION( 0x080000, "maincpu", 0 )    /* Main 68K code */
	ROM_LOAD16_BYTE( "o17_09ii.8j", 0x000000, 0x08000, CRC(b60541ee) SHA1(e4fb752073c99a83939ebc45307777b94519f01c) )
	ROM_LOAD16_BYTE( "o17_10ii.8l", 0x000001, 0x08000, CRC(96f5045e) SHA1(16cf2f4d55570cf0489a426d6e841d2968f9423a) )
	ROM_LOAD16_BYTE( "o17_11ii.7j", 0x040000, 0x20000, CRC(6beac378) SHA1(041ba98a89a4bac32575858db8a061bdf7804594) )
	ROM_LOAD16_BYTE( "o17_12ii.7l", 0x040001, 0x20000, CRC(6adb6eb5) SHA1(9b6e63aa50d271c2bb0b4cf822fc6f3684f10230) )

	ROM_REGION( 0x8000, "audiocpu", 0 )    /* Sound HD647180 code */
	/* sound CPU is a HD647180 (Z180) with internal ROM - not yet supported */
	ROM_LOAD( "hd647180.017",  0x00000, 0x08000, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "o17_05.12j",  0x00000, 0x20000, CRC(565315f8) SHA1(6b1c5ef52359483228b329c89c2e1174e3fbf017) )
	ROM_LOAD16_BYTE( "o17_06.13j",  0x00001, 0x20000, CRC(95262d4c) SHA1(16f3aabecb1c87ce7eadf4f0ff61b29a4c017614) )
	ROM_LOAD16_BYTE( "o17_07.12l",  0x40000, 0x20000, CRC(4c4b735c) SHA1(812c3bf46bd7764b2bb812bd2b9eb0331ed257ae) )
	ROM_LOAD16_BYTE( "o17_08.13l",  0x40001, 0x20000, CRC(95c6586c) SHA1(ff87901f79d80f73ad09664b0c0d892898570616) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "o17_01.1d",  0x00000, 0x20000, CRC(ea12e491) SHA1(02190722b7c5383471e0af9596be7039a5367240) )
	ROM_LOAD16_BYTE( "o17_02.3d",  0x00001, 0x20000, CRC(32a13a9f) SHA1(1446acdfd21cd41f3d97aaf30f498c0c5d890605) )
	ROM_LOAD16_BYTE( "o17_03.5d",  0x40000, 0x20000, CRC(68723dc9) SHA1(4f1b7aa2469c955e03737b611a7d2524f1e4f61e) )
	ROM_LOAD16_BYTE( "o17_04.7d",  0x40001, 0x20000, CRC(fe0ecb13) SHA1(634a49262b9c092c25f11b14c6757fe94ea9eddc) )

	ROM_REGION( 0x40, "proms", 0 )      /* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "prom14.25b",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) ) /* sprite attribute (flip/position) ?? */
	ROM_LOAD( "prom15.20c",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) ) /* ??? */
ROM_END

ROM_START( fireshrkd )
	ROM_REGION( 0x080000, "maincpu", 0 )    /* Main 68K code */
	ROM_LOAD16_BYTE( "o17_09dyn.8j",0x000000, 0x10000, CRC(e25eee27) SHA1(1ff3f838123180a0b6672c9beee6c0f0092a0f94) )
	ROM_LOAD16_BYTE( "o17_10dyn.8l",0x000001, 0x10000, CRC(c4c58cf6) SHA1(5867ecf66cd6c16cfcc54a581d3f4a8b666fd839) )
	ROM_LOAD16_BYTE( "o17_11ii.7j", 0x040000, 0x20000, CRC(6beac378) SHA1(041ba98a89a4bac32575858db8a061bdf7804594) )
	ROM_LOAD16_BYTE( "o17_12ii.7l", 0x040001, 0x20000, CRC(6adb6eb5) SHA1(9b6e63aa50d271c2bb0b4cf822fc6f3684f10230) )

	ROM_REGION( 0x8000, "audiocpu", 0 )    /* Sound HD647180 code */
	/* sound CPU is a HD647180 (Z180) with internal ROM - not yet supported */
	ROM_LOAD( "hd647180.017",  0x00000, 0x08000, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "o17_05.12j",  0x00000, 0x20000, CRC(565315f8) SHA1(6b1c5ef52359483228b329c89c2e1174e3fbf017) )
	ROM_LOAD16_BYTE( "o17_06.13j",  0x00001, 0x20000, CRC(95262d4c) SHA1(16f3aabecb1c87ce7eadf4f0ff61b29a4c017614) )
	ROM_LOAD16_BYTE( "o17_07.12l",  0x40000, 0x20000, CRC(4c4b735c) SHA1(812c3bf46bd7764b2bb812bd2b9eb0331ed257ae) )
	ROM_LOAD16_BYTE( "o17_08.13l",  0x40001, 0x20000, CRC(95c6586c) SHA1(ff87901f79d80f73ad09664b0c0d892898570616) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "o17_01.1d",  0x00000, 0x20000, CRC(ea12e491) SHA1(02190722b7c5383471e0af9596be7039a5367240) )
	ROM_LOAD16_BYTE( "o17_02.3d",  0x00001, 0x20000, CRC(32a13a9f) SHA1(1446acdfd21cd41f3d97aaf30f498c0c5d890605) )
	ROM_LOAD16_BYTE( "o17_03.5d",  0x40000, 0x20000, CRC(68723dc9) SHA1(4f1b7aa2469c955e03737b611a7d2524f1e4f61e) )
	ROM_LOAD16_BYTE( "o17_04.7d",  0x40001, 0x20000, CRC(fe0ecb13) SHA1(634a49262b9c092c25f11b14c6757fe94ea9eddc) )

	ROM_REGION( 0x40, "proms", 0 )      /* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "prom14.25b",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) ) /* sprite attribute (flip/position) ?? */
	ROM_LOAD( "prom15.20c",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) ) /* ??? */
ROM_END

ROM_START( fireshrkdh )
	ROM_REGION( 0x080000, "maincpu", 0 )    /* Main 68K code */
	ROM_LOAD16_BYTE( "o17_09dyh.8j",0x000000, 0x10000, CRC(7b4c14dd) SHA1(d40dcf223f16c0f507aeb282d1524dbf1349c536) )
	ROM_LOAD16_BYTE( "o17_10dyh.8l",0x000001, 0x10000, CRC(a3f159f9) SHA1(afc9630ca38da730f7cf4954d1333954e8d75787) )
	ROM_LOAD16_BYTE( "o17_11ii.7j", 0x040000, 0x20000, CRC(6beac378) SHA1(041ba98a89a4bac32575858db8a061bdf7804594) )
	ROM_LOAD16_BYTE( "o17_12ii.7l", 0x040001, 0x20000, CRC(6adb6eb5) SHA1(9b6e63aa50d271c2bb0b4cf822fc6f3684f10230) )

	ROM_REGION( 0x8000, "audiocpu", 0 )    /* Sound HD647180 code */
	/* sound CPU is a HD647180 (Z180) with internal ROM - not yet supported */
	ROM_LOAD( "hd647180.017",  0x00000, 0x08000, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "o17_05.12j",  0x00000, 0x20000, CRC(565315f8) SHA1(6b1c5ef52359483228b329c89c2e1174e3fbf017) )
	ROM_LOAD16_BYTE( "o17_06.13j",  0x00001, 0x20000, CRC(95262d4c) SHA1(16f3aabecb1c87ce7eadf4f0ff61b29a4c017614) )
	ROM_LOAD16_BYTE( "o17_07.12l",  0x40000, 0x20000, CRC(4c4b735c) SHA1(812c3bf46bd7764b2bb812bd2b9eb0331ed257ae) )
	ROM_LOAD16_BYTE( "o17_08.13l",  0x40001, 0x20000, CRC(95c6586c) SHA1(ff87901f79d80f73ad09664b0c0d892898570616) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "o17_01.1d",  0x00000, 0x20000, CRC(ea12e491) SHA1(02190722b7c5383471e0af9596be7039a5367240) )
	ROM_LOAD16_BYTE( "o17_02.3d",  0x00001, 0x20000, CRC(32a13a9f) SHA1(1446acdfd21cd41f3d97aaf30f498c0c5d890605) )
	ROM_LOAD16_BYTE( "o17_03.5d",  0x40000, 0x20000, CRC(68723dc9) SHA1(4f1b7aa2469c955e03737b611a7d2524f1e4f61e) )
	ROM_LOAD16_BYTE( "o17_04.7d",  0x40001, 0x20000, CRC(fe0ecb13) SHA1(634a49262b9c092c25f11b14c6757fe94ea9eddc) )

	ROM_REGION( 0x40, "proms", 0 )      /* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "prom14.25b",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) ) /* sprite attribute (flip/position) ?? */
	ROM_LOAD( "prom15.20c",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) ) /* ??? */
ROM_END

ROM_START( outzone )
	ROM_REGION( 0x040000, "maincpu", 0 )    /* Main 68K code */
	ROM_LOAD16_BYTE( "prg2.bin",  0x000001, 0x20000, CRC(9704db16) SHA1(12b43a6961a7f63f29563eb77aaacb70d3c368dd) )
	ROM_LOAD16_BYTE( "prg1.bin",  0x000000, 0x20000, CRC(127a38d7) SHA1(d7f1ed91ff7d4de9e8215aa3b5cb65693145e433) )

	ROM_REGION( 0x8000, "audiocpu", 0 )    /* Sound Z80 code */
	ROM_LOAD( "rom9.bin",  0x0000, 0x8000, CRC(73d8e235) SHA1(f37ad497259a467cdf2ec8b3e6e7d3e873087e6c) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_WORD( "rom5.bin",  0x00000, 0x80000, CRC(c64ec7b6) SHA1(e73b51c3713c2ea7a572a02531c15d1261ddeaa0) )
	ROM_LOAD16_WORD( "rom6.bin",  0x80000, 0x80000, CRC(64b6c5ac) SHA1(07fa20115f603445c0d51af3465c0471c09d76b1) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "rom2.bin",  0x00000, 0x20000, CRC(6bb72d16) SHA1(a127b10d9c255542bd09fcb5df057c12fd28c0d1) )
	ROM_LOAD16_BYTE( "rom1.bin",  0x00001, 0x20000, CRC(0934782d) SHA1(e4a775ead23227d7d6e76aea23aa3103b511d031) )
	ROM_LOAD16_BYTE( "rom3.bin",  0x40000, 0x20000, CRC(ec903c07) SHA1(75906f31200877fc8f6e78c2606ad5be49778165) )
	ROM_LOAD16_BYTE( "rom4.bin",  0x40001, 0x20000, CRC(50cbf1a8) SHA1(cfab1504746654b4a61912155e9aeca746c65321) )

	ROM_REGION( 0x40, "proms", 0 )      /* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "tp018_10.bpr",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )   /* sprite attribute (flip/position) ?? */
	ROM_LOAD( "tp018_11.bpr",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )   /* ??? */
ROM_END

ROM_START( outzoneh )
	ROM_REGION( 0x040000, "maincpu", 0 )    /* Main 68K code */
	ROM_LOAD16_BYTE( "tp018_7.bin",  0x000000, 0x20000, CRC(0c2ac02d) SHA1(78fda906ef7e0bb8e4ad44f34a8ac934b75d4bd8) )
	ROM_LOAD16_BYTE( "tp018_8.bin",  0x000001, 0x20000, CRC(ca7e48aa) SHA1(c5073e6c124d74f16d01e67949965fdca929a886) )

	ROM_REGION( 0x8000, "audiocpu", 0 )    /* Sound Z80 code */
	ROM_LOAD( "rom9.bin",  0x0000, 0x8000, CRC(73d8e235) SHA1(f37ad497259a467cdf2ec8b3e6e7d3e873087e6c) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_WORD( "rom5.bin",  0x00000, 0x80000, CRC(c64ec7b6) SHA1(e73b51c3713c2ea7a572a02531c15d1261ddeaa0) )
	ROM_LOAD16_WORD( "rom6.bin",  0x80000, 0x80000, CRC(64b6c5ac) SHA1(07fa20115f603445c0d51af3465c0471c09d76b1) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "rom2.bin",  0x00000, 0x20000, CRC(6bb72d16) SHA1(a127b10d9c255542bd09fcb5df057c12fd28c0d1) )
	ROM_LOAD16_BYTE( "rom1.bin",  0x00001, 0x20000, CRC(0934782d) SHA1(e4a775ead23227d7d6e76aea23aa3103b511d031) )
	ROM_LOAD16_BYTE( "rom3.bin",  0x40000, 0x20000, CRC(ec903c07) SHA1(75906f31200877fc8f6e78c2606ad5be49778165) )
	ROM_LOAD16_BYTE( "rom4.bin",  0x40001, 0x20000, CRC(50cbf1a8) SHA1(cfab1504746654b4a61912155e9aeca746c65321) )

	ROM_REGION( 0x40, "proms", 0 )      /* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "tp018_10.bpr",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )   /* sprite attribute (flip/position) ?? */
	ROM_LOAD( "tp018_11.bpr",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )   /* ??? */
ROM_END

ROM_START( outzonea )
	ROM_REGION( 0x040000, "maincpu", 0 )    /* Main 68K code */
	ROM_LOAD16_BYTE( "18.bin",  0x000000, 0x20000, CRC(31a171bb) SHA1(4ee707e758ab21d2809b65daf0081f86bd9328d9) )
	ROM_LOAD16_BYTE( "19.bin",  0x000001, 0x20000, CRC(804ecfd1) SHA1(7dead8064445c6d44ebd0889583deb5e17b1954a) )

	ROM_REGION( 0x8000, "audiocpu", 0 )    /* Sound Z80 code */
	ROM_LOAD( "rom9.bin",  0x0000, 0x8000, CRC(73d8e235) SHA1(f37ad497259a467cdf2ec8b3e6e7d3e873087e6c) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_WORD( "rom5.bin",  0x00000, 0x80000, CRC(c64ec7b6) SHA1(e73b51c3713c2ea7a572a02531c15d1261ddeaa0) )
	ROM_LOAD16_WORD( "rom6.bin",  0x80000, 0x80000, CRC(64b6c5ac) SHA1(07fa20115f603445c0d51af3465c0471c09d76b1) )
/* a pirate board exists using the same data in a different layout
    ROM_LOAD16_BYTE( "04.bin",  0x000000, 0x10000, CRC(3d11eae0) )
    ROM_LOAD16_BYTE( "08.bin",  0x000001, 0x10000, CRC(c7628891) )
    ROM_LOAD16_BYTE( "13.bin",  0x080000, 0x10000, CRC(b23dd87e) )
    ROM_LOAD16_BYTE( "09.bin",  0x080001, 0x10000, CRC(445651ba) )
    ROM_LOAD16_BYTE( "03.bin",  0x020000, 0x10000, CRC(6b347646) )
    ROM_LOAD16_BYTE( "07.bin",  0x020001, 0x10000, CRC(461b47f9) )
    ROM_LOAD16_BYTE( "14.bin",  0x0a0000, 0x10000, CRC(b28ae37a) )
    ROM_LOAD16_BYTE( "10.bin",  0x0a0001, 0x10000, CRC(6596a076) )
    ROM_LOAD16_BYTE( "02.bin",  0x040000, 0x10000, CRC(11a781c3) )
    ROM_LOAD16_BYTE( "06.bin",  0x040001, 0x10000, CRC(1055da17) )
    ROM_LOAD16_BYTE( "15.bin",  0x0c0000, 0x10000, CRC(9c9e811b) )
    ROM_LOAD16_BYTE( "11.bin",  0x0c0001, 0x10000, CRC(4c4d44dc) )
    ROM_LOAD16_BYTE( "01.bin",  0x060000, 0x10000, CRC(e8c46aea) )
    ROM_LOAD16_BYTE( "05.bin",  0x060001, 0x10000, CRC(f8a2fe01) )
    ROM_LOAD16_BYTE( "16.bin",  0x0e0000, 0x10000, CRC(cffcb99b) )
    ROM_LOAD16_BYTE( "12.bin",  0x0e0001, 0x10000, CRC(90d37ded) )
*/

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "rom2.bin",  0x00000, 0x20000, CRC(6bb72d16) SHA1(a127b10d9c255542bd09fcb5df057c12fd28c0d1) )
	ROM_LOAD16_BYTE( "rom1.bin",  0x00001, 0x20000, CRC(0934782d) SHA1(e4a775ead23227d7d6e76aea23aa3103b511d031) )
	ROM_LOAD16_BYTE( "rom3.bin",  0x40000, 0x20000, CRC(ec903c07) SHA1(75906f31200877fc8f6e78c2606ad5be49778165) )
	ROM_LOAD16_BYTE( "rom4.bin",  0x40001, 0x20000, CRC(50cbf1a8) SHA1(cfab1504746654b4a61912155e9aeca746c65321) )

	ROM_REGION( 0x40, "proms", 0 )      /* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "tp018_10.bpr",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )   /* sprite attribute (flip/position) ?? */
	ROM_LOAD( "tp018_11.bpr",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )   /* ??? */
ROM_END

ROM_START( outzoneb )
	ROM_REGION( 0x040000, "maincpu", 0 )    /* Main 68K code */
	ROM_LOAD16_BYTE( "tp07.bin",  0x000000, 0x20000, CRC(a85a1d48) SHA1(74f16ef5126f0ce3d94a66849ccd7c28338e3974) )
	ROM_LOAD16_BYTE( "tp08.bin",  0x000001, 0x20000, CRC(d8cc44af) SHA1(da9c07e3670e5c7a2c1f9bc433e604a2a13b8a54) )

	ROM_REGION( 0x8000, "audiocpu", 0 )    /* Sound Z80 code */
	ROM_LOAD( "tp09.bin",  0x0000, 0x8000, CRC(dd56041f) SHA1(a481b8959b349761624166906175f8efcbebb7e7) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_WORD( "rom5.bin",  0x00000, 0x80000, CRC(c64ec7b6) SHA1(e73b51c3713c2ea7a572a02531c15d1261ddeaa0) )
	ROM_LOAD16_WORD( "rom6.bin",  0x80000, 0x80000, CRC(64b6c5ac) SHA1(07fa20115f603445c0d51af3465c0471c09d76b1) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "rom2.bin",  0x00000, 0x20000, CRC(6bb72d16) SHA1(a127b10d9c255542bd09fcb5df057c12fd28c0d1) )
	ROM_LOAD16_BYTE( "rom1.bin",  0x00001, 0x20000, CRC(0934782d) SHA1(e4a775ead23227d7d6e76aea23aa3103b511d031) )
	ROM_LOAD16_BYTE( "rom3.bin",  0x40000, 0x20000, CRC(ec903c07) SHA1(75906f31200877fc8f6e78c2606ad5be49778165) )
	ROM_LOAD16_BYTE( "rom4.bin",  0x40001, 0x20000, CRC(50cbf1a8) SHA1(cfab1504746654b4a61912155e9aeca746c65321) )

	ROM_REGION( 0x40, "proms", 0 )      /* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "tp018_10.bpr",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )   /* sprite attribute (flip/position) ?? */
	ROM_LOAD( "tp018_11.bpr",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )   /* ??? */
ROM_END

/* is this a prototype? */
ROM_START( outzonec )                   /* From board serial number 2122 */
	ROM_REGION( 0x040000, "maincpu", 0 )    /* Main 68K code */
	ROM_LOAD16_BYTE( "rom7.bin",  0x000000, 0x20000, CRC(936e25d8) SHA1(ffb7990ea1539d868a9ad2fb711b0febd90f098d) )
	ROM_LOAD16_BYTE( "rom8.bin",  0x000001, 0x20000, CRC(d19b3ecf) SHA1(b406999b9f1e2104d958b42cc745bf79dbfe50b3) )

	ROM_REGION( 0x8000, "audiocpu", 0 )    /* Sound Z80 code */
	ROM_LOAD( "rom9.bin",  0x0000, 0x8000, BAD_DUMP CRC(73d8e235) SHA1(f37ad497259a467cdf2ec8b3e6e7d3e873087e6c) ) // see notes

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_WORD( "rom5.bin",  0x00000, 0x80000, CRC(c64ec7b6) SHA1(e73b51c3713c2ea7a572a02531c15d1261ddeaa0) )
	ROM_LOAD16_WORD( "rom6.bin",  0x80000, 0x80000, CRC(64b6c5ac) SHA1(07fa20115f603445c0d51af3465c0471c09d76b1) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "rom2.bin",  0x00000, 0x20000, CRC(6bb72d16) SHA1(a127b10d9c255542bd09fcb5df057c12fd28c0d1) )
	ROM_LOAD16_BYTE( "rom1.bin",  0x00001, 0x20000, CRC(0934782d) SHA1(e4a775ead23227d7d6e76aea23aa3103b511d031) )
	ROM_LOAD16_BYTE( "rom3.bin",  0x40000, 0x20000, CRC(ec903c07) SHA1(75906f31200877fc8f6e78c2606ad5be49778165) )
	ROM_LOAD16_BYTE( "rom4.bin",  0x40001, 0x20000, CRC(50cbf1a8) SHA1(cfab1504746654b4a61912155e9aeca746c65321) )

	ROM_REGION( 0x40, "proms", 0 )      /* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "tp018_10.bpr",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )   /* sprite attribute (flip/position) ?? */
	ROM_LOAD( "tp018_11.bpr",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )   /* ??? */
ROM_END

ROM_START( vimana )         /* From board serial number 1547.04 (July '94) */
	ROM_REGION( 0x040000, "maincpu", 0 )    /* Main 68K code */
	ROM_LOAD16_BYTE( "tp019-7a.bin",  0x000000, 0x20000, CRC(5a4bf73e) SHA1(9a43d822bc24b59278f294d0b3275595de997d16) )
	ROM_LOAD16_BYTE( "tp019-8a.bin",  0x000001, 0x20000, CRC(03ba27e8) SHA1(edb5fe741d2a6a7fe5cde9a82317ea1e9447cf73) )

	ROM_REGION( 0x8000, "audiocpu", 0 )    /* Sound HD647180 code */
	/* sound CPU is a HD647180 (Z180) with internal ROM - not yet supported */
	ROM_LOAD( "hd647180.019",  0x00000, 0x08000, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "vim6.bin",  0x00000, 0x20000, CRC(2886878d) SHA1(f44933d87bbcd3bd58f46e0f0f89b05c409b713b) )
	ROM_LOAD16_BYTE( "vim5.bin",  0x00001, 0x20000, CRC(61a63d7a) SHA1(5cdebc03110252cc43d31b6f87f9a23556892977) )
	ROM_LOAD16_BYTE( "vim4.bin",  0x40000, 0x20000, CRC(b0515768) SHA1(9907b52b4d30ce5324270a12c40250068adafca8) )
	ROM_LOAD16_BYTE( "vim3.bin",  0x40001, 0x20000, CRC(0b539131) SHA1(07f3e3b9b28c8218e36668c24d16dbb6e9a66889) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_WORD( "vim1.bin",  0x00000, 0x80000, CRC(cdde26cd) SHA1(27893af4692ec7bcbaac9e790c0707c98df84e62) )
	ROM_LOAD16_WORD( "vim2.bin",  0x80000, 0x80000, CRC(1dbfc118) SHA1(4fd039a3172f73ad910349b2d360e8ae77ccddb2) )

	ROM_REGION( 0x40, "proms", 0 )      /* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "tp019-09.bpr",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )   /* sprite attribute (flip/position) ?? */
	ROM_LOAD( "tp019-10.bpr",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )   /* ??? */
ROM_END

ROM_START( vimanan )
	ROM_REGION( 0x040000, "maincpu", 0 )    /* Main 68K code */
	ROM_LOAD16_BYTE( "tp019-07.rom",  0x000000, 0x20000, CRC(78888ff2) SHA1(7e1d248f806d585952eb35ceec6a7e63ae4e22f9) )
	ROM_LOAD16_BYTE( "tp019-08.rom",  0x000001, 0x20000, CRC(6cd2dc3c) SHA1(029d974eb938c5e2fbe7575f0dda342b4b12b731) )

	ROM_REGION( 0x8000, "audiocpu", 0 )    /* Sound HD647180 code */
	/* sound CPU is a HD647180 (Z180) with internal ROM - not yet supported */
	ROM_LOAD( "hd647180.019",  0x00000, 0x08000, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "vim6.bin",  0x00000, 0x20000, CRC(2886878d) SHA1(f44933d87bbcd3bd58f46e0f0f89b05c409b713b) )
	ROM_LOAD16_BYTE( "vim5.bin",  0x00001, 0x20000, CRC(61a63d7a) SHA1(5cdebc03110252cc43d31b6f87f9a23556892977) )
	ROM_LOAD16_BYTE( "vim4.bin",  0x40000, 0x20000, CRC(b0515768) SHA1(9907b52b4d30ce5324270a12c40250068adafca8) )
	ROM_LOAD16_BYTE( "vim3.bin",  0x40001, 0x20000, CRC(0b539131) SHA1(07f3e3b9b28c8218e36668c24d16dbb6e9a66889) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_WORD( "vim1.bin",  0x00000, 0x80000, CRC(cdde26cd) SHA1(27893af4692ec7bcbaac9e790c0707c98df84e62) )
	ROM_LOAD16_WORD( "vim2.bin",  0x80000, 0x80000, CRC(1dbfc118) SHA1(4fd039a3172f73ad910349b2d360e8ae77ccddb2) )

	ROM_REGION( 0x40, "proms", 0 )      /* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "tp019-09.bpr",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )   /* sprite attribute (flip/position) ?? */
	ROM_LOAD( "tp019-10.bpr",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )   /* ??? */
ROM_END

ROM_START( vimanaj )
	ROM_REGION( 0x040000, "maincpu", 0 )    /* Main 68K code */
	ROM_LOAD16_BYTE( "vim07.bin",  0x000000, 0x20000, CRC(1efaea84) SHA1(f9c5d2365d8948fa66dbe61d355919db15843a28) )
	ROM_LOAD16_BYTE( "vim08.bin",  0x000001, 0x20000, CRC(e45b7def) SHA1(6b92a91d64581954da8ecdbeb5fed79bcc9c5217) )

	ROM_REGION( 0x8000, "audiocpu", 0 )    /* Sound HD647180 code */
	/* sound CPU is a HD647180 (Z180) with internal ROM - not yet supported */
	ROM_LOAD( "hd647180.019",  0x00000, 0x08000, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "vim6.bin",  0x00000, 0x20000, CRC(2886878d) SHA1(f44933d87bbcd3bd58f46e0f0f89b05c409b713b) )
	ROM_LOAD16_BYTE( "vim5.bin",  0x00001, 0x20000, CRC(61a63d7a) SHA1(5cdebc03110252cc43d31b6f87f9a23556892977) )
	ROM_LOAD16_BYTE( "vim4.bin",  0x40000, 0x20000, CRC(b0515768) SHA1(9907b52b4d30ce5324270a12c40250068adafca8) )
	ROM_LOAD16_BYTE( "vim3.bin",  0x40001, 0x20000, CRC(0b539131) SHA1(07f3e3b9b28c8218e36668c24d16dbb6e9a66889) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_WORD( "vim1.bin",  0x00000, 0x80000, CRC(cdde26cd) SHA1(27893af4692ec7bcbaac9e790c0707c98df84e62) )
	ROM_LOAD16_WORD( "vim2.bin",  0x80000, 0x80000, CRC(1dbfc118) SHA1(4fd039a3172f73ad910349b2d360e8ae77ccddb2) )

	ROM_REGION( 0x40, "proms", 0 )      /* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "tp019-09.bpr",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )   /* sprite attribute (flip/position) ?? */
	ROM_LOAD( "tp019-10.bpr",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )   /* ??? */
ROM_END


DRIVER_INIT_MEMBER(toaplan1_state,toaplan1)
{
	toaplan1_driver_savestate();
}

DRIVER_INIT_MEMBER(toaplan1_state,demonwld)
{
	toaplan1_driver_savestate();
	demonwld_driver_savestate();
}

DRIVER_INIT_MEMBER(toaplan1_state,vimana)
{
	toaplan1_driver_savestate();
	vimana_driver_savestate();
}



GAME( 1988, rallybik,   0,        rallybik, rallybik,  toaplan1_state, toaplan1, ROT270, "Toaplan / Taito Corporation", "Rally Bike / Dash Yarou", 0 )
GAME( 1988, truxton,    0,        truxton,  truxton,   toaplan1_state, toaplan1, ROT270, "Toaplan / Taito Corporation", "Truxton / Tatsujin", 0 )
GAME( 1989, hellfire,   0,        hellfire, hellfire,  toaplan1_state, toaplan1, ROT0,   "Toaplan (Taito license)", "Hellfire (2P set)", 0 )
GAME( 1989, hellfire1,  hellfire, hellfire, hellfire1, toaplan1_state, toaplan1, ROT0,   "Toaplan (Taito license)", "Hellfire (1P set)", 0 )
GAME( 1989, hellfire2a, hellfire, hellfire, hellfire2a,toaplan1_state, toaplan1, ROT0,   "Toaplan (Taito license)", "Hellfire (2P set, older)", 0 )
GAME( 1989, hellfire1a, hellfire, hellfire, hellfire1a,toaplan1_state, toaplan1, ROT0,   "Toaplan (Taito license)", "Hellfire (1P set, older)", 0 )
GAME( 1989, zerowing,   0,        zerowing, zerowing2, toaplan1_state, toaplan1, ROT0,   "Toaplan", "Zero Wing (2P set)", 0 )
GAME( 1989, zerowing1,  zerowing, zerowing, zerowing,  toaplan1_state, toaplan1, ROT0,   "Toaplan", "Zero Wing (1P set)", 0 )
GAME( 1989, zerowingw,  zerowing, zerowing, zerowing2, toaplan1_state, toaplan1, ROT0,   "Toaplan (Williams license)", "Zero Wing (2P set, Williams license)", 0 )
GAME( 1990, demonwld,   0,        demonwld, demonwld,  toaplan1_state, demonwld, ROT0,   "Toaplan", "Demon's World / Horror Story (set 1)", 0 )
GAME( 1989, demonwld1,  demonwld, demonwld, demonwld1, toaplan1_state, demonwld, ROT0,   "Toaplan", "Demon's World / Horror Story (set 2)", 0 )
GAME( 1989, demonwld2,  demonwld, demonwld, demonwld1, toaplan1_state, demonwld, ROT0,   "Toaplan", "Demon's World / Horror Story (set 3)", 0 )
GAME( 1989, demonwld3,  demonwld, demonwld, demonwld1, toaplan1_state, demonwld, ROT0,   "Toaplan", "Demon's World / Horror Story (set 4)", 0 )
GAME( 1990, fireshrk,   0,        samesame, fireshrk,  toaplan1_state, toaplan1, ROT270, "Toaplan", "Fire Shark", MACHINE_NO_SOUND )
GAME( 1989, fireshrka,  fireshrk, samesame, fireshrka, toaplan1_state, toaplan1, ROT270, "Toaplan", "Fire Shark (earlier)", MACHINE_NO_SOUND )
GAME( 1990, fireshrkd,  fireshrk, samesame, samesame2, toaplan1_state, toaplan1, ROT270, "Toaplan (Dooyong license)", "Fire Shark (Korea, set 1, easier)", MACHINE_NO_SOUND )
GAME( 1990, fireshrkdh, fireshrk, samesame, samesame2, toaplan1_state, toaplan1, ROT270, "Toaplan (Dooyong license)", "Fire Shark (Korea, set 2, harder)", MACHINE_NO_SOUND )
GAME( 1989, samesame,   fireshrk, samesame, samesame,  toaplan1_state, toaplan1, ROT270, "Toaplan", "Same! Same! Same! (1P set)", MACHINE_NO_SOUND )
GAME( 1989, samesame2,  fireshrk, samesame, samesame2, toaplan1_state, toaplan1, ROT270, "Toaplan", "Same! Same! Same! (2P set)", MACHINE_NO_SOUND )
GAME( 1990, outzone,    0,        outzone,  outzone,   toaplan1_state, toaplan1, ROT270, "Toaplan", "Out Zone", 0 )
GAME( 1990, outzoneh,   outzone,  outzone,  outzone,   toaplan1_state, toaplan1, ROT270, "Toaplan", "Out Zone (harder)", 0 )
GAME( 1990, outzonea,   outzone,  outzone,  outzonea,  toaplan1_state, toaplan1, ROT270, "Toaplan", "Out Zone (old set)", 0 )
GAME( 1990, outzoneb,   outzone,  outzone,  outzonea,  toaplan1_state, toaplan1, ROT270, "Toaplan", "Out Zone (older set)", 0 )
GAME( 1990, outzonec,   outzone,  outzone,  outzonec,  toaplan1_state, toaplan1, ROT270, "Toaplan", "Out Zone (oldest set)", MACHINE_IMPERFECT_SOUND ) // prototype?
GAME( 1991, vimana,     0,        vimana,   vimana,    toaplan1_state, vimana,   ROT270, "Toaplan", "Vimana (World, set 1)", MACHINE_NO_SOUND )
GAME( 1991, vimanan,    vimana,   vimana,   vimanan,   toaplan1_state, vimana,   ROT270, "Toaplan", "Vimana (World, set 2)", MACHINE_NO_SOUND )
GAME( 1991, vimanaj,    vimana,   vimana,   vimanaj,   toaplan1_state, vimana,   ROT270, "Toaplan", "Vimana (Japan)", MACHINE_NO_SOUND )
