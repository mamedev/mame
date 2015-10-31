// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Couriersud
/***************************************************************************

    Galaxian-derived hardware

    Galaxian is the root hardware for many, many systems developed in the
    1980-1982 timeframe. The basic design, which originated with Namco(?),
    was replicated, tweaked, bootlegged, and used numerous times.

    The basic hardware design comprises three sections on a single PCB:
    a CPU section, a sound section, and a video section.

    The CPU section is based around a Z80 (though there are modified
    designs that changed this to an S2650). The base galaxian hardware
    is designed to allow access to up to 16k of program ROM and 2k of
    working RAM.

    The sound section consists of three parts. The first part is
    a programmable 8-bit down counter that clocks a 4-bit counter which
    generates a primitive waveform whose shape is hardcoded but can be
    controlled by a pair of variable resistors. The second part is
    a set of three 555 timers which can be individually enabled and
    combined to produce square waves at fixed separated pitches. A
    fourth 555 timer is configured via a 4-bit frequency parameter to
    control the overall pitch of the other three. Finally, two single
    bit-triggered noise circuits are available. A 17-bit noise LFSR
    (which also generates stars for the video circuit) feeds into both
    circuits. A "HIT" line enables a simple on/off control of one
    filtered output, while a "FIRE" line triggers a fixed short duration
    pulse (controlled by another 555 timer) of modulated noise.

    See video/galaxian.c for a description of the video section.

****************************************************************************

    Schematics are known to exist for these games:
        * Galaxian
        * Moon Alien Part 2
        * King and Balloon

        * Moon Cresta
        * Moon Shuttle

        * Frogger
        * Amidar
        * Turtles

        * Scramble
        * The End

        * Super Cobra
        * Dark Planet
        * Lost Tomb

        * Dambusters

****************************************************************************

Main clock: XTAL = 18.432 MHz
Z80 Clock: XTAL/6 = 3.072 MHz
Horizontal video frequency: HSYNC = XTAL/3/192/2 = 16 kHz
Video frequency: VSYNC = HSYNC/132/2 = 60.606060 Hz
VBlank duration: 1/VSYNC * (20/132) = 2500 us


Notes:
-----

- The only code difference between 'galaxian' and 'galmidw' is that the
  'BONUS SHIP' text is printed on a different line.



Moon Cresta versions supported:
------------------------------

mooncrst    Nichibutsu     - later revision with better demo mode and
                              text for docking. Encrypted. No ROM/RAM check
mooncrsu    Nichibutsu USA - later revision with better demo mode and
                              text for docking. Unencrypted. No ROM/RAM check
mooncrsa    Nichibutsu     - older revision with better demo mode and
                              text for docking. Encrypted. No ROM/RAM check
mooncrs2    Nichibutsu     - probably first revision (no patches) and ROM/RAM check code.
                              This came from a bootleg board, with the logos erased
                              from the graphics
mooncrsg    Gremlin        - same docking text as mooncrst
mooncrsb    bootleg of mooncrs2. ROM/RAM check erased.


Notes about 'azurian' :
-----------------------

  bit 6 of IN1 is linked with bit 2 of IN2 (check code at 0x05b3) to set difficulty :

    bit 6  bit 2    contents of
     IN1     IN2          0x40f4            consequences            difficulty

     OFF     OFF             2          aliens move 2 frames out of 3       easy
     ON      OFF             4          aliens move 4 frames out of 5       hard
     OFF     ON              3          aliens move 3 frames out of 4       normal
     ON      ON              5          aliens move 5 frames out of 6       very hard

  aliens movements is handled by routine at 0x1d59 :

    - alien 1 moves when 0x4044 != 0 else contents of 0x40f4 is stored at 0x4044
    - alien 2 moves when 0x4054 != 0 else contents of 0x40f4 is stored at 0x4054
    - alien 3 moves when 0x4064 != 0 else contents of 0x40f4 is stored at 0x4064


Notes about 'smooncrs' :
------------------------

  Due to code at 0x2b1c and 0x3306, the game ALWAYS checks the inputs for player 1
  (even for player 2 when "Cabinet" Dip Switch is set to "Cocktail")


Notes about 'scorpnmc' :
-----------------------

  As the START buttons are also the buttons for player 1, how should I map them ?
  I've coded this the same way as in 'checkman', but I'm not sure this is correct.

  I can't tell if it's a bug, but if you reset the game when the screen is flipped,
  the screens remains flipped (the "flip screen" routine doesn't seem to be called) !


Notes about 'frogg' :
---------------------

  If bit 5 of IN0 or bit 5 of IN1 is HIGH, something strange occurs (check code
  at 0x3580) : each time you press START2 a counter at 0x47da is incremented.
  When this counter reaches 0x2f, each next time you press START2, it acts as if
  you had pressed COIN2, so credits are added !
  Bit 5 of IN0 is tested if "Cabinet" Dip Switch is set to "Upright" and
  bit 5 of IN1 is tested if "Cabinet" Dip Switch is set to "Cocktail".


Galaxian Bootleg Single Board Layout:
-------------------------------------

      |----------------------------------------------------------------------------------------------|
      |                                                                                              |
    A | AM27LS00    7486     7486     74163    74163    74LS393  LM324    NE555             ECG740A  |
      |                                                                                              |
    B | AM27LS00    74LS00   74LS32   74LS161  74LS161  74LS74   NE555    NE555                      |
      |                                                                                              |
    C | AM27LS00    7408     74LS02   74LS161  74LS161  74175    4066     NE555    NE555             |
      |                                                                                              |
    D | AM27LS00    74LS00   74LS20   74LS10   74LS74   74LS377  2114     74LS138                    |
      |                                                                                              |
    E | AM27LS00    7408     74LS20   74LS283  74LS283  74LS02   2114     74LS138  DM8334            |
      |                                                                                              |
    F | 7486        74LS273  74LS??   74LS367  74LS367  74LS273           74LS138  DM8334            |
      |   G                                                                                          |
    G |   F         74LS194  74LS157  74LS273  74LS367  6331-1J  PRG1     74LS00   DM8334            |--|
      |   X                                                                                             |
    H |   1         74LS194  74LS157  UPB8216                             74161    74161                |
      |                                                                                                 |
    I |   G         74LS194  74LS157  UPB8216           74LS157                    74LS273              |
      |   F                                                                                             |
    J |   X         74LS194  2114        2        2     7408              UPB8216                       |
      |   2                              1        1              PRG2                                   |
    K | 18.432MHZ   74LS157  2114        0        0     74LS74            UPB8216  74LS368              |
      |                                  1        1                                                     |
    L | 74LS368     74LS157  74LS157  74LS157  74LS157  74LS04            74LS139  74LS368           |--|
      |                                                                                              |
    M | 74LS107     7474     74LS74   74LS139  74LS10   74LS02   74LS367  74LS367  74LS368       2   |
      |                                                                                          |   |
    N | 7474                 74LS20   74LS139  74LS74   74LS74   74LS367           74LS368       U   |
      |                                                                                          P   |
    O | 74LS164     74LS366  7486     7486     7486     7486           Z80                       C   |
      |                                                                                          G   |
    P | 74LS164     74LS30   74LS161  74LS161  74LS161  74LS161                                      |
      |----------------------------------------------------------------------------------------------|
        1           2        3        4        5        6        7        8        9        1
                                                                                            0


Stephh's notes (based on the games Z80 code and some tests) for games based on 'scobra' MACHINE_DRIVER :

1) 'scobra' and clones

1a) 'scobra'

  - Player 2 controls are used for player 2 regardless of the "Cabinet" Dip Switch.
  - COIN1 and SERVICE1 share the same coinage while COIN2 always awards 3 credits per coin;
    when "Coinage" is set to "99 Credits", credits are always set to 99 when pressing COIN1 (code at 0x037d).
  - There is an unused coinage routine at 0x0159 with the following settings :

    PORT_DIPNAME( 0x06, 0x00, DEF_STR( Coinage ) )
    PORT_DIPSETTING(    0x00, "A 2/1  B 99" )
    PORT_DIPSETTING(    0x06, "A 1/2  B 1/1" )
    PORT_DIPSETTING(    0x04, "A 1/5  B 1/1" )
    PORT_DIPSETTING(    0x02, "A 1/7  B 1/1" )

    I can't tell at the moment if it's a leftover from another Konami game on similar hardware.
  - You can have 3 or 4 lives at start, and you can only continue 4 times (code at 0x0ebf).

1b) 'scobrase'

  - The only difference in main CPU with 'scobra' is not in code but in data :
      * data at 0x1323+ and 0x1575+ displays " (c) SEGA   1981 " instead of "(c) KONAMI   1981".
      * data from 0x3d0c to 0x3fff has an unknown effect (this area is filled with 0xff in 'scobra').
      * data from 0x5b62 to 0x5b6f displays "SEGA" instead of "OSAKA" on the first building in the "BASE"
        (last) level ("KONAMI" is always still displayed on other buildings).
    As the code is the same, comments from 'scobra' also apply to this set.
  - Audio CPU is different than the one in 'scobra'. More investigation is needed !

1c) 'scobras'

  - Main CPU is different than the one in 'scobra', but audio CPU is the same as 'scobrase' !
  - Player 2 controls are used for player 2 regardless of the "Cabinet" Dip Switch.
  - COIN1 and SERVICE1 share the same coinage while COIN2 always awards 3 credits per coin;
    when "Coinage" is set to "99 Credits", credits are always set to 99 when pressing COIN1 (code at 0x0bec).
  - There is NO unused coinage routine.
  - You can have 3 or 5 lives at start, and you can only continue 255 times (code at 0x00e3).
  - On the first building in the "BASE" (last) level is written "STERN" instead of "OSAKA".

1d) 'scobrab'

  - The only difference in main CPU with 'scobras' is not in code but in data :
      * data from 0x0434 to 0x043e affects the addresses in ROM area of the strings to display.
      * data from 0x0456 to 0x07a0 affects the strings which are displayed (almost all of them).
  - Audio CPU is the same as the one in 'scobra' (with different ROM names though).

1e) 'suprheli'

  - The only difference in main CPU with 'scobras' is not in code but in data :
      * data at 0x0522+ displays "- SUPER HELI - " instead of "- SUPER COBRA -".
      * data at 0x0547+ and 0x0799+ displays "                 " instead of "(c) STERN  1981  ".
      * data from 0x5b26 to 0x5b32 displays "APPLE" instead of "STERN" on the first building in the "BASE"
        (last) level.
      * data from 0x5bbb to 0x5bc9 displays "ORANGE" instead of "KONAMI" on the other buildings in the "BASE"
        (last) level.
    As the code is the same, comments from 'scobras' also apply to this set.
  - There is only ONE byte of difference with audio CPU in 'scobrase' :

      Z:\MAME\data>fc /B epr1277.5e 9.9d
      Comparaison des fichiers epr1277.5e et 9.9D
      00001332: FD FF

    Could it be a rotten bit ? As I have no evidence of this, I don't flag the ROM as BAD_DUMP.

2) 'moonwar' and clones

  - "8255 Port C bit 4 was originally designed so when bit4=0, 1P spinner is selected, and when bit4=1,
    2P spinner gets selected.  But they forgot to change the 8255 initialization value and Port C was set
    to input, setting the spinner select bit to HI regardless what was written to it. This bug has been
    corrected in the newer set, but, to maintain hardware compatibility with older PCB's, they had to reverse
    to active status of the select bit.  So in the newer set, Bit4=1 selects the 1P spinner and Bit4=0 selects
    the 2P spinner".

2a) 'moonwar'

  - Press START1 when reseting the game to enter sort of inputs "test mode".
  - "Hyperflip" button is ignored when "Cabinet" is set to "Cocktail" (code at 0x108d).
  - When in "Free Play" mode, you only 3 lives at start.

2b) 'moonwara'

  - Press START1 when reseting the game to enter sort of inputs "test mode".
  - "Hyperflip" button is ignored when "Cabinet" is set to "Cocktail" (code at 0x107f).
  - Besides the spinner bug, coinage is very weird in this set (no correlation between COIN1 and COIN2).

3) 'armorcar' and clones

3a) 'armorcar'

  - Press P2 BUTTON2 when reseting the game to enter sort of inputs "test mode".
    You'll notice that there is some leftover code from 'moonwar' as you can see 2 (muxed) PORT A
    (and there are still writes to PORT C bit 4). This has no effect in the game though.
  - After the 3 ports are read, when "Cabinet" is set to "Cocktail" and its player 2 turn, player 2 inputs
    are "copied" into player 1 ones (code at 0x0fd2 : start reading inputs).

3b) 'armorcar2'

  - When IN1 bit 2 is ON when reseting the game, you enter sort of inputs "test mode".
    You'll notice that there is some leftover code from 'moonwar' as you can see 2 (muxed) PORT A
    (and there are still writes to IN2 bit 4). This has no effect in the game though.
    As this bit is marked as "unused" (see below why), you can never access to this "test mode".
  - IN2 bit 3 has no real effect in this set : even if contents of 0x8627 is updated each time player changes,
    screen flipping (0xa806 and 0xa807) is always set to "normal" (0x00 * 2) due to code at 0x0598, there is a
    missing call to 0x0abf at 0x0a8c (there is even a 'ret' for call from 0x15aa), and there is no code to "copy"
    player 2 inputs into player 1 ones (code at 0x0fb2 : start reading inputs).
    There is still a leftover from 'armorcar' code, so this bit affects display (how ?) when IN2 bit 3 is ON
    and it is player 2 turn (code at 0x0b66 is the same as the one at 0x0b87 in 'armorcar').

4) 'tazmania'

  - Press P1 BUTTON2 when reseting the game to enter sort of inputs "test mode".
  - After the 3 ports are read, when "Cabinet" is set to "Cocktail" and its player 2 turn, player 2 inputs
    are "copied" into player 1 ones (code at 0x124e : start reading inputs).
  - When "Cabinet" is set to "Upright", press any player 2 joystick direction to end current level
    (code at 0x38dd). This trick does NOT work in bonus rooms though.

5) 'anteater'

  - Press P1 BUTTON1 when reseting the game to enter sort of inputs "test mode".
  - IN2 bit 3 has no effect in this set : even if contents of 0x86c4 is updated each time player changes,
    screen flipping (0xa806 and 0xa807) is always reset to "normal" (0x00 * 2) after possible screen inversion
    due to code at 0x05c7, and there is no code to "copy" player 2 inputs into player 1 ones (code at 0x0f7a :
    start reading inputs).

6) 'calipso'

  - Press P1 BUTTON1 when reseting the game to enter sort of inputs "test mode".
  - Press P1 BUTTON1 to start a 1 player game or press P2 BUTTON1 to start a 2 players game ("Team-Play").
  - IN2 bit 3 has no effect in this set : even if there is code to "copy" player 2 inputs into player 1 ones
    (code at 0x1448 : start reading inputs), contents of 0x8669 is always set to 0x01 regardless of number players
    and is NEVER updated (there is even no code for this). Furthermore, the screen flipping routine forces the
    screen to be "normal" ([0xa806] = [0xa807] = 0x00) because of the 'jr' instruction at 0x2988.
    It's possible that there is a cocktail version of the game, but I'm not really convinced about it.

7) 'losttomb' and clones

7a) 'losttomb'

  - Press P1 right joystick UP when reseting the game to enter sort of inputs "test mode".
  - There is no "Cabinet" Dip Switch for this game and no possible muxed input for a 2nd player.
    Furthermore, the routine at 0x254b is NEVER called, so the screen NEVER flips !
  - The routine that reads inputs (code at 0x0ef4) behaves differently if you are in "attract mode" or not :
      * when playing ([0x865f] = 0x00), it reads the 3 inputs ports
      * when in "attract mode" ([0x865f] = 0xff), it only reads IN0 (to get status of COINn and STARTn)
        and IN1 (to get the status of the "Lives" Dip Switch), and IN2 is completely ignored
    The side effect of such thing is that the status of the "Demo Sounds" Dip Switch will be taken into
    consideration only after a game has been played (for example, the game will always be silenced in
    "attract mode" after resetting the machine because 0x00 is stored at 0x8613 during initialisation).
  - When in "Free Play" mode, you only 3 lives at start.

7b) 'losttombh'

  - The only difference with 'losttomb' is not in code but in data :

      Z:\MAME\data>fc /B 2h-easy lthard
      Comparaison des fichiers 2h-easy et LTHARD
      00000399: 0A 0B
      0000039E: 0D 11
      000003A3: 0F 14
      000003A8: 13 19
      000003AD: 15 1A
      000003B2: 18 1B
      000003B7: 1A 1C
      000003BC: 1B 1D
      000003C1: 1C 1E
      000003C6: 1D 1F
      000003CB: 1E 20
      000003D0: 1F 21
      000003D5: 20 22
      000003D9: 03 05
      000003E6: 0E 10
      000003F0: 12 15
      000003FD: 14 17
      00000409: 0E 10
      00000415: 0E 10
      0000099B: AA 76    altered value to please the checksum routine

    So the game is harder, but it has the same ingame bugs as 'losttomb'.

8) 'spdcoin'

  - Press START1 or START2 when reseting the game to enter sort of inputs "test mode" (in fact, only IN0 is tested).
    Press BOTH START1 and START2 to exit from it.
  - Press START1 + START2 + P1 joystick LEFT when reseting the game to display some statistics (code at 0x0226).
    Release BOTH START1 and START2 to exit this screen.

9) 'superbon'

  - This game is heavily based on 'losttomb', so not surprisingly is the code similar.
  - The main difference in terms of gameplay is that you only have 1 joystick to control your character
    and that you shoot in the direction you are running unless you press the "HOLD" button.
  - There are no tests at startup and it's not possible to enter sort of inputs "test mode" (even if code exists)
    by pressing P1 joystick UP because of 'jump' instruction at 0x007d. If you try to check the ROMS, you'll
    notice that they have the same name as in 'losttomb' and that they fail the checksum routines.
  - There is no "Cabinet" Dip Switch for this game and no possible muxed input for a 2nd player.
    Furthermore, the routine at 0x2a48 is NEVER called, so the screen NEVER flips !
  - The routine that reads inputs (code at 0x0eb7) behaves differently if you are in "attract mode" or not :
      * when playing ([0x8667] = 0x00), it reads the 3 inputs ports
      * when in "attract mode" ([0x8667] = 0xff), it only reads IN0 (to get status of COINn and STARTn)
        and IN1 (to get the status of the "Lives" Dip Switch), and IN2 is completely ignored
    The side effect of such thing is that the status of the "Demo Sounds" Dip Switch will be taken into
    consideration only after a game has been played (for example, the game will always be silenced in
    "attract mode" after resetting the machine because 0x00 is stored at 0x8613 during initialisation).
  - When in "Free Play" mode, you only 3 lives at start.


Stephh's additional notes (based on the games Z80 code and some tests) for "Moon Cresta" and its numerous clones :

a) 'mooncrst'

  - made by Nichibutsu
  - inputs :
      * player 1 controls are used by player 1
      * player 2 controls are used by player 2, even in an "upright" cabinet
  - 2 coins slots with different settings :
      * coin A : 1C_1C / 2C_1C / 3C_1C / 4C_1C
      * coin B : 1C_1C / 1C_2C / 1C_3C / "Free Play"
  - no writes to 0xa003, so no coin counters
  - bonus life at 30000 or 50000 based on a Dip Switch
  - possible partial Japanese text based on a Dip Switch
  - hi-score : 11 chars (even if only 10 will be displayed), 60 seconds to enter
  - players bullet speed : 4 pixels - lower limit : 0x04
  - ingame bug : if you reset the game when screen is flipped, it isn't flipped back
  - driver possible bug (which occurs for all "Moon Cresta" sets but 'mooncrgx') :
    when the screen is flipped, sprites are too shifted (see for example player 2
    score which misses ending '0') while bullets shall be good
    as a consequence, square around letters is wrong when entering player name
    for hi-score table when screen is flipped

b) 'mooncrsu'

  - made by Nichibutsu
  - very similar to 'mooncrst' with the only following differences :
      * additional "USA" display after "Nichibutsu" (which is shifted left)
      * writes to 0xb000 to 0xb0ff on reset ('mooncrst' only writes to 0xb000),
        so there is no screen flipped ingame bug as in 'mooncrst'

c) 'mooncrsa'

  - made by Nichibutsu
  - additional "(c)" display before "Nichibutsu"
  - "(c) 1980 NIHON BUSSAN CO. , LTD" display replaced with "May 1980" in yellow
  - code at 0x1f00 has been removed ! I can't determine was is was supposed to do,
    but it's based on number of enemies left (stored at 0x823c) and possible time
    spent on the level (stored at 0x8226). Any hint is fully welcome !
  - this version is easier than 'mooncrst' : look at high nibbles that are stored
    at 0x809b and 0x809c via code at 0x0cb8 (0x01 and 0x02 instead of 0x11 and 0x12).
  - 2 coins slots, but same settings : 1C_1C / 1C_2C / 1C_3C / "Free Play"
  - same other infos as in 'mooncrst'
  - same ingame bug as in 'mooncrst'

d) 'mooncrs2'

  - bootleg (possibily based on a Gremlin version we don't have)
  - heavily based on 'mooncrsa' with additional RAM/ROM check routine at 0x3ea1
  - some "chars" have been erased from the GFX ROMS but some routines which
    "prints" them are still there (but there are less than in 'mooncrsa')
  - same other infos as in 'mooncrst'
  - due to numerous writes in the RAM/ROM check routine, there is no screen flipped
    ingame bug as in 'mooncrst'

e) 'mooncrsb'

  - bootleg (possibily based on a Gremlin version we don't have)
  - the only difference with 'mooncrs2' is that RAM/ROM check routine at 0x3ea1
    has completely been "noped" and the jump at address 0x0004 has been changed
  - all "chars" from the GFX ROMS haven't been erased, so you can see the top
    of the "Gremlin" logo as copyright and hi-scores names
  - same ingame bug as in 'mooncrst'

f) 'mooncrs3'

  - bootleg
  - very similar to 'mooncrs2' with the only following differences :
      * checksum of ROM area 0x0000-0x3fff is computed, but the result is discarded
        (see "xor a" operation at 0x3fc0 instead of "and a")
      * coins stuff is different (see below)
  - 2 coins slots with different settings (same as 'mooncrst') :
      * coin A : 1C_1C / 2C_1C / 3C_1C / 4C_1C
      * coin B : 1C_1C / 1C_2C / 1C_3C / "Free Play"
  - there are writes to 0xa003 (check code at 0x1b8e and 0x1b9e) which occur
    when you insert a coin, but I can't confirm it's related to coin counters
    as the same value is written when you press COIN1 or COIN2

g) 'mooncrsg'

  - made by Gremlin
  - there are MANY changes and additions, and I wonder if there's such a Nichibutsu set;
    anyway, closest set to this one seems to be 'mooncrst'
  - Gremin "logo" on 2 lines instead of Nichibutsu copyright messages (2 lines)
  - additional test for IN0 bit 7 (code at 0x0174) which always adds 1 credit
  - there are writes to 0xa003 (check code at 0x0158 and 0x0160) which occur
    when you insert a coin, but I can't confirm it's related to coin counters
    as the same value is written when you press COIN1 or COIN2
  - only English text (Dip Switch has no effect due to code at 0x2f77)
  - hi-score : 3 chars, 10 seconds to enter
  - same difficulty as in 'monncrst' (but stored at 0x809e and 0x809f)
  - same other infos as in 'mooncrst'
  - same ingame bug as in 'mooncrst'

h) 'fantazia'

  - made by Subelectro
  - closest set to this one seems to be 'mooncrsb'
  - all intro texts have been changed as well as colors
  - inputs :
      * player 1 controls are used by player 1
      * player 2 controls are used by player 2, only in a "cocktail" cabinet
    look at additional routine at 0x29e0
  - 2 coins slots with different settings (inverted coin A/B compared to 'mooncrst') :
      * coin A : 1C_1C / 1C_2C / 1C_3C / "Free Play"
      * coin B : 1C_1C / 2C_1C / 3C_1C / 4C_1C
  - only English text (Dip Switch has no effect due to code at 0x2f53)
  - hi-score : 3 chars, 60 seconds to enter
  - same other infos as in 'mooncrst'
  - same ingame bug as in 'mooncrst'

i) 'eagle'

  - made by Centuri
  - very similar to 'mooncrsb' with the only following differences :
      * only 3 chars for hi-score instead of 11
      * all other changes are modified "strings" to be displayed
        (the intro texts but copyright remains though) as well as
        new GFX (I can't test the sound for now to check)
  - same ingame bug as in 'mooncrst'

j) 'eagle2'

  - made by Centuri
  - very similar to 'eagle' with the only following differences :
      * only 20 seconds to enter hi-score instead of 60
      * coins stuff is different (see below)
      * one GFX ROM is slighlty different
  - 2 coins slots, but same settings : 1C_1C / 2C_1C / 3C_1C / 4C_1C
  - previous "Coin B" Dip Switch is now only tested to see if in "Freeplay" mode
  - same ingame bug as in 'mooncrst'

k) 'eagle3'

  - made by Centuri
  - PRG ROMS are the same as for 'eagle' while two GFX ROMS are slighly different
    (so the game is having 'mooncrst' ships and 'eagle' enemies)
  - same ingame bug as in 'mooncrst'

l) 'spctbird'

  - made by Fortrek
  - very similar to 'mooncrsb' with the only following difference :
      * coins stuff is different (same as in 'eagle2' - see below)
  - 2 coins slots, but same settings : 1C_1C / 2C_1C / 3C_1C / 4C_1C
  - previous "Coin B" Dip Switch is now only tested to see if in "Freeplay" mode
  - same ingame bug as in 'mooncrst'

m) 'smooncrs'

  - made par Gremlin (bootleg based on a Nichibutsu version we don't have ?)
  - same RAM/ROM check routine as in 'mooncrs2' (so there is no screen flipped
    ingame bug as in 'mooncrst'), but LOTS of new features !
  - only top of the Gremlin logo is displayed and it is also used for hi-scores
  - all intro texts have been changed
  - "2'ST" instead of "2'ND" and "RECORD" instead of "HI-SCORE"
  - additional "PLAYER 1/2" messages when player changes
  - inputs : player 1 controls are used by players 1 and 2, even in a "cocktail"
    cabinet (player 2 inputs are never read due to code at 0x2b1c and 0x3313)
  - 2 coins slots with different settings :
      * coin A : 1C_1C / 2C_1C / 3C_1C / 4C_1C
      * coin B : 1C_1C / 1C_2C / 1C_3C / "Free Play"
    additional wrong (Spanish) text displayed when "Coin B" set to 1C_1C
    (check additional code at 0x0fae) :
      * if "Coin A" set to 4C_1C, "1 MONEDA 1 PARTIDA" on one line
      * if "Coin A" set to 2C_1C, "1 MONEDA 1 PARTIDA" on one line
        and "2 MONEDAS 3 PARTIDAS" on another line below
    when "Coin B" set to "Free Play", "CREDIT 04" instead of "FREE PLAY" string
    (even if this number of credits is decremented when you press a START button,
     it is put back to 04 when the game is over for all players)
  - additional "POR" display after the number of credits
  - bonus life always 50000 due to code at 0x2f68
  - only English text due to code at 0x2f53
  - hi-score : 3 chars, 60 seconds to enter
  - players bullet speed : 9 or 12 pixels (using previous "Language" Dip Switch) -
    lower limit : 0x0f (see additional routine at 0x0007 and call from 0x3407)
  - game difficulty using previous "Bonus Life" Dip Switch (code at 0x2962)
    however, even with "Easy" difficulty, the game is much harder as in 'mooncrs2'
    as enemies as enemies move much faster and as they shoot on some levels
  - docking stage is harder has there are gaps of 2 pixels instead of 1
  - when you complete the 8 stages, "O.K." "FANTASTIC" messages on 2 lines
    instead of "FAR OUT !" message on 1 line
  - same ingame bug as in 'mooncrst'
  - another ingame bug : when in "cocktail mode", "PLAYER 1/2" messages are
    displayed BEFORE the screen is flipped (back)
  - driver other bugs :
      * when screen is flipped, player's bullets aren't displayed
      * when screen is flipped, enemies' bullets aren't flipped

n) 'spcdrag'

  - bootleg
  - heavily based on 'smooncrs' (so there's a RAM/ROM check) but some differences though
  - same intro texts as in 'mooncrs2'
  - 2 coins slots with different settings :
      * coin A : 1C_1C / 2C_1C / 3C_1C / 4C_1C
      * coin B : 1C_2C / 1C_3C / 1C_4C / "Free Play"
    additional wrong (Engrish) text displayed when "Coin B" set to 1C_2C
    (check additional code at 0x0fae) :
      * if "Coin A" set to 4C_1C, "1 COIN   1 PLAY   " on one line
      * if "Coin A" set to 2C_1C, "1 COIN   1 PLAY   " on one line
        and "2 COINS   3 PLAYES  " (notice the spelling) on another line below
    when "Coin B" set to "Free Play", "CREDIT 04" instead of "FREE PLAY" string
    (even if this number of credits is decremented when you press a START button,
     it is put back to 04 when the game is over for all players)
  - "CAP 2" display instead of "POR" after the number of credits
  - hi-score : 11 chars (even if only 10 will be displayed), 60 seconds to enter
    (same as in 'mooncrs2')
  - players bullet speed : 6 or 9 pixels (using previous "Language" Dip Switch) -
    lower limit : 0x04 (instead of speed 9/12 and lower limit 0x0f)
  - even if there's also the "Difficulty" Dip Switch, the game is a little bit easier
    (enemies speed is slower and docking stage is back to 1 pixel to fit 'mooncrs2')
  - when you complete the 8 stages, same "FAR OUT !" message as in 'mooncrs2'
  - driver bug : even if player's bullets are displayed when screen is flipped as in
    other sets, enemies' bullets are still not flipped as in 'smooncrs'

o) 'spcdraga'

  - bootleg ? (there's a Nichibutsu logo which is displayed in the "title" screen
    as well as in the hi-scores)
  - very similar to 'spcdrag' with the only following (comestical) differences :
      * unused routine at 0x37a8 has been "noped"
      * no text after the number of credits
      * all texts have been translated to Spanish

p) 'mooncrgx'

  - bootleg on "Galaxian" hardware
  - very similar to 'mooncrsb' with the only following differences :
      * all unused routines have been "noped"
      * settings are different (see below)
  - 2 coins slots with different settings :
      * coin A : 1C_1C / 2C_1C
      * coin B : 1C_3C / 1C_5C
  - there are writes to 0x6003 when you press COIN1 but not when you press COIN2
  - there are also writes to (unmapped) 0x6804 when you press either COIN1 or COIN2 :
      * when you press COIN1, 0x00 is written once
      * when you press COIN2, 0x01 is written 5 times, then 0x00 is written once
  - only English text (Dip Switch has no effect due to code at 0x2f4b)
  - no ingame bug due to code at 0x2f77
  - driver possible bug : while sprites are now correct when screen is flipped,
    they are too shifted when screen is not flipped (again, see for example player 2
    score which misses ending '0') while bullets shall be good
    as a consequence, square around letters is wrong when entering player name
    for hi-score table when screen not is flipped



TODO:
----
- Problems with Galaxian based on the observation of a real machine:
  - Background humming is incorrect.  It's faster on a real machine
  - Explosion sound is much softer.  Filter involved?
- streakng/ghostmun: $4800-4bff
- smooncrs : fix read/writes at/to unmapped memory (when player 2, "cocktail" mode) + fix the ?#! bug with "bullets" (when player 2, "cocktail" mode)
- timefgtr : missing player bullets, sprite ROM extend(see later levels), sound is too slow, some sprites missing
- zigzag   : full Dip Switches and Inputs
- zigzag2  : full Dip Switches and Inputs
- jumpbug  : full Dip Switches and Inputs - missing possible discrete sounds
- jumpbugb : full Dip Switches and Inputs - missing possible discrete sounds
- levers   : full Dip Switches and Inputs
- kingball : full Dip Switches and Inputs
- kingbalj : full Dip Switches and Inputs
- frogg    : fix read/writes at/to unmapped/wrong memory
- scprpng  : fix read/writes at/to unmapped/wrong memory
- scorpion : check whether konami filters are used
- explorer : check whether konami filters are used

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/s2650/s2650.h"
#include "machine/i8255.h"
#include "sound/sn76496.h"
#include "sound/dac.h"
#include "sound/discrete.h"
#include "audio/cclimber.h"
#include "audio/galaxian.h"
#include "includes/galaxian.h"


#define KONAMI_SOUND_CLOCK      14318000



/*************************************
 *
 *  Interrupts
 *
 *************************************/

INTERRUPT_GEN_MEMBER(galaxian_state::interrupt_gen)
{
	/* interrupt line is clocked at VBLANK */
	/* a flip-flop at 6F is held in the preset state based on the NMI ON signal */
	if (m_irq_enabled)
		device.execute().set_input_line(m_irq_line, ASSERT_LINE);
}

INTERRUPT_GEN_MEMBER(galaxian_state::fakechange_interrupt_gen)
{
	interrupt_gen(device);

	if (ioport("FAKE_SELECT")->read_safe(0x00))
	{
		m_tenspot_current_game++;
		m_tenspot_current_game%=10;
		tenspot_set_game_bank(m_tenspot_current_game, 1);
		m_maincpu->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
	}
}

WRITE8_MEMBER(galaxian_state::irq_enable_w)
{
	/* the latched D0 bit here goes to the CLEAR line on the interrupt flip-flop */
	m_irq_enabled = data & 1;

	/* if CLEAR is held low, we must make sure the interrupt signal is clear */
	if (!m_irq_enabled)
		space.device().execute().set_input_line(m_irq_line, CLEAR_LINE);
}

/*************************************
 *
 *  DRIVER latch control
 *
 *************************************/

WRITE8_MEMBER(galaxian_state::start_lamp_w)
{
	/* offset 0 = 1P START LAMP */
	/* offset 1 = 2P START LAMP */
	set_led_status(machine(), offset, data & 1);
}


WRITE8_MEMBER(galaxian_state::coin_lock_w)
{
	/* many variants and bootlegs don't have this */
	coin_lockout_global_w(machine(), ~data & 1);
}


WRITE8_MEMBER(galaxian_state::coin_count_0_w)
{
	coin_counter_w(machine(), 0, data & 1);
}


WRITE8_MEMBER(galaxian_state::coin_count_1_w)
{
	coin_counter_w(machine(), 1, data & 1);
}



/*************************************
 *
 *  General Konami sound I/O
 *
 *************************************/

READ8_MEMBER(galaxian_state::konami_ay8910_r)
{
	/* the decoding here is very simplistic, and you can address both simultaneously */
	UINT8 result = 0xff;
	if (offset & 0x20) result &= m_ay8910_1->data_r(space, 0);
	if (offset & 0x80) result &= m_ay8910_0->data_r(space, 0);
	return result;
}


WRITE8_MEMBER(galaxian_state::konami_ay8910_w)
{
	/* AV 4,5 ==> AY8910 #2 */
	/* the decoding here is very simplistic, and you can address two simultaneously */
	if (offset & 0x10)
		m_ay8910_1->address_w(space, 0, data);
	else if (offset & 0x20)
		m_ay8910_1->data_w(space, 0, data);
	/* AV6,7 ==> AY8910 #1 */
	if (offset & 0x40)
		m_ay8910_0->address_w(space, 0, data);
	else if (offset & 0x80)
		m_ay8910_0->data_w(space, 0, data);
}


WRITE8_MEMBER(galaxian_state::konami_sound_control_w)
{
	UINT8 old = m_konami_sound_control;
	m_konami_sound_control = data;

	/* the inverse of bit 3 clocks the flip flop to signal an INT */
	/* it is automatically cleared on the acknowledge */
	if ((old & 0x08) && !(data & 0x08))
		m_audiocpu->set_input_line(0, HOLD_LINE);

	/* bit 4 is sound disable */
	machine().sound().system_mute(data & 0x10);
}


READ8_MEMBER(galaxian_state::konami_sound_timer_r)
{
	/*
	    The timer is clocked at KONAMI_SOUND_CLOCK and cascades through a
	    series of counters. It first encounters a chained pair of 4-bit
	    counters in an LS393, which produce an effective divide-by-256. Next
	    it enters the divide-by-2 counter in an LS93, followed by the
	    divide-by-8 counter. Finally, it clocks a divide-by-5 counter in an
	    LS90, followed by the divide-by-2 counter. This produces an effective
	    period of 16*16*2*8*5*2 = 40960 clocks.

	    The clock for the sound CPU comes from output C of the first
	    divide-by-16 counter, or KONAMI_SOUND_CLOCK/8. To recover the
	    current counter index, we use the sound cpu clock times 8 mod
	    16*16*2*8*5*2.
	*/
	UINT32 cycles = (m_audiocpu->total_cycles() * 8) % (UINT64)(16*16*2*8*5*2);
	UINT8 hibit = 0;

	/* separate the high bit from the others */
	if (cycles >= 16*16*2*8*5)
	{
		hibit = 1;
		cycles -= 16*16*2*8*5;
	}

	/* the top bits of the counter index map to various bits here */
	return (hibit << 7) |           /* B7 is the output of the final divide-by-2 counter */
			(BIT(cycles,14) << 6) | /* B6 is the high bit of the divide-by-5 counter */
			(BIT(cycles,13) << 5) | /* B5 is the 2nd highest bit of the divide-by-5 counter */
			(BIT(cycles,11) << 4) | /* B4 is the high bit of the divide-by-8 counter */
			0x0e;                   /* assume remaining bits are high, except B0 which is grounded */
}


WRITE8_MEMBER(galaxian_state::konami_sound_filter_w)
{
	discrete_device *discrete = machine().device<discrete_device>("konami");
	static const char *const ayname[2] = { "8910.0", "8910.1" };
	int which, chan;

	/* the offset is used as data, 6 channels * 2 bits each */
	/* AV0 .. AV5 ==> AY8910 #2 */
	/* AV6 .. AV11 ==> AY8910 #1 */
	for (which = 0; which < 2; which++)
		if (machine().device(ayname[which]) != NULL)
			for (chan = 0; chan < 3; chan++)
			{
				UINT8 bits = (offset >> (2 * chan + 6 * (1 - which))) & 3;

				/* low bit goes to 0.22uF capacitor = 220000pF  */
				/* high bit goes to 0.047uF capacitor = 47000pF */
				discrete->write(space, NODE(3 * which + chan + 11), bits);
			}
}


WRITE8_MEMBER(galaxian_state::konami_portc_0_w)
{
	logerror("%s:ppi0_portc_w = %02X\n", machine().describe_context(), data);
}


WRITE8_MEMBER(galaxian_state::konami_portc_1_w)
{
	logerror("%s:ppi1_portc_w = %02X\n", machine().describe_context(), data);
}


/*************************************
 *
 *  The End I/O
 *
 *************************************/

READ8_MEMBER(galaxian_state::theend_ppi8255_r)
{
	/* the decoding here is very simplistic, and you can address both simultaneously */
	UINT8 result = 0xff;
	if (offset & 0x0100) result &= m_ppi8255_0->read(space, offset & 3);
	if (offset & 0x0200) result &= m_ppi8255_1->read(space, offset & 3);
	return result;
}


WRITE8_MEMBER(galaxian_state::theend_ppi8255_w)
{
	/* the decoding here is very simplistic, and you can address both simultaneously */
	if (offset & 0x0100) m_ppi8255_0->write(space, offset & 3, data);
	if (offset & 0x0200) m_ppi8255_1->write(space, offset & 3, data);
}


WRITE8_MEMBER(galaxian_state::theend_coin_counter_w)
{
	coin_counter_w(machine(), 0, data & 0x80);
}


/*************************************
 *
 *  Scramble I/O
 *
 *************************************/

WRITE8_MEMBER(galaxian_state::scramble_protection_w)
{
	/*
	    This is not fully understood; the low 4 bits of port C are
	    inputs; the upper 4 bits are outputs. Scramble main set always
	    writes sequences of 3 or more nibbles to the low port and
	    expects certain results in the upper nibble afterwards.
	*/
	m_protection_state = (m_protection_state << 4) | (data & 0x0f);
	switch (m_protection_state & 0xfff)
	{
		/* scramble */
		case 0xf09:     m_protection_result = 0xff; break;
		case 0xa49:     m_protection_result = 0xbf; break;
		case 0x319:     m_protection_result = 0x4f; break;
		case 0x5c9:     m_protection_result = 0x6f; break;

		/* scrambls */
		case 0x246:     m_protection_result ^= 0x80;    break;
		case 0xb5f:     m_protection_result = 0x6f; break;
	}
}


READ8_MEMBER(galaxian_state::scramble_protection_r)
{
	return m_protection_result;
}


CUSTOM_INPUT_MEMBER(galaxian_state::scramble_protection_alt_r)
{
	/*
	    There are two additional bits that are derived from bit 7 of
	    the protection result. This is just a guess but works well enough
	    to boot scrambls.
	*/
	return (m_protection_result >> 7) & 1;
}

/*************************************
 *
 *  Explorer I/O
 *
 *************************************/

WRITE8_MEMBER(galaxian_state::explorer_sound_control_w)
{
	m_audiocpu->set_input_line(0, ASSERT_LINE);
}


READ8_MEMBER(galaxian_state::explorer_sound_latch_r)
{
	m_audiocpu->set_input_line(0, CLEAR_LINE);
	return soundlatch_byte_r(m_audiocpu->space(AS_PROGRAM), 0);
}



/*************************************
 *
 *  SF-X I/O
 *
 *************************************/

READ8_MEMBER(galaxian_state::sfx_sample_io_r)
{
	/* the decoding here is very simplistic, and you can address both simultaneously */
	UINT8 result = 0xff;
	if (offset & 0x04) result &= m_ppi8255_2->read(space, offset & 3);
	return result;
}


WRITE8_MEMBER(galaxian_state::sfx_sample_io_w)
{
	/* the decoding here is very simplistic, and you can address both simultaneously */
	if (offset & 0x04) m_ppi8255_2->write(space, offset & 3, data);
	if (offset & 0x10) m_dac->write_signed8(data);
}


WRITE8_MEMBER(galaxian_state::sfx_sample_control_w)
{
	UINT8 old = m_sfx_sample_control;
	m_sfx_sample_control = data;

	/* the inverse of bit 0 clocks the flip flop to signal an INT */
	/* it is automatically cleared on the acknowledge */
	if ((old & 0x01) && !(data & 0x01))
		m_audio2->set_input_line(0, HOLD_LINE);
}


/*************************************
 *
 *  Monster Zero I/O
 *
 *************************************/

/* Preliminary protection notes (based on z80 disasm):

    The initial protection routine is on the maincpu at $c591-$c676.
    It accesses the 8255, expects an irq, and reads $d800 256 times which is xored against data
    starting at $0100 to confirm a checksum stored in $0011-$0019. Then it reads the (presumably)same
    block to store it in RAM at $3800-$3fff. 9 blocks in total.
    It is presumed that this data comes from another ROM, and scrambled/encrypted a bit.
    The data(code) in the extra RAM is later jumped/called to in many parts of the game.
*/

READ8_MEMBER(galaxian_state::monsterz_protection_r)
{
	return m_protection_result;
}


void galaxian_state::monsterz_set_latch()
{
	// read from a rom (which one?? "a-3e.k3" from audiocpu ($2700-$2fff) looks very suspicious)
	UINT8 *rom = memregion("audiocpu")->base();
	m_protection_result = rom[0x2000 | (m_protection_state & 0x1fff)]; // probably needs a BITSWAP8

	// and an irq on the main z80 afterwards
	m_maincpu->set_input_line(0, HOLD_LINE );
}


WRITE8_MEMBER(galaxian_state::monsterz_porta_1_w)
{
	// d7 high: set latch + advance address high bits (and reset low bits?)
	if (data & 0x80)
	{
		monsterz_set_latch();
		m_protection_state = (m_protection_state + 0x100) & 0xff00;
	}
}

WRITE8_MEMBER(galaxian_state::monsterz_portb_1_w)
{
	// d3 high: set latch + advance address low bits
	if (data & 0x08)
	{
		monsterz_set_latch();
		m_protection_state = ((m_protection_state + 1) & 0x00ff) | (m_protection_state & 0xff00);
	}
}

WRITE8_MEMBER(galaxian_state::monsterz_portc_1_w)
{
}

/*************************************
 *
 *  Frogger I/O
 *
 *************************************/

READ8_MEMBER(galaxian_state::frogger_ppi8255_r)
{
	/* the decoding here is very simplistic, and you can address both simultaneously */
	UINT8 result = 0xff;
	if (offset & 0x1000) result &= m_ppi8255_1->read(space, (offset >> 1) & 3);
	if (offset & 0x2000) result &= m_ppi8255_0->read(space, (offset >> 1) & 3);
	return result;
}


WRITE8_MEMBER(galaxian_state::frogger_ppi8255_w)
{
	/* the decoding here is very simplistic, and you can address both simultaneously */
	if (offset & 0x1000) m_ppi8255_1->write(space, (offset >> 1) & 3, data);
	if (offset & 0x2000) m_ppi8255_0->write(space, (offset >> 1) & 3, data);
}


READ8_MEMBER(galaxian_state::frogger_ay8910_r)
{
	/* the decoding here is very simplistic */
	UINT8 result = 0xff;
	if (offset & 0x40) result &= m_ay8910_0->data_r(space, 0);
	return result;
}


WRITE8_MEMBER(galaxian_state::frogger_ay8910_w)
{
	/* the decoding here is very simplistic */
	/* AV6,7 ==> AY8910 #1 */
	if (offset & 0x40)
		m_ay8910_0->data_w(space, 0, data);
	else if (offset & 0x80)
		m_ay8910_0->address_w(space, 0, data);
}


READ8_MEMBER(galaxian_state::frogger_sound_timer_r)
{
	/* same as regular Konami sound but with bits 3,5 swapped */
	UINT8 konami_value = konami_sound_timer_r(space, 0);
	return BITSWAP8(konami_value, 7,6,3,4,5,2,1,0);
}


WRITE8_MEMBER(galaxian_state::froggermc_sound_control_w)
{
	m_audiocpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
}

IRQ_CALLBACK_MEMBER(galaxian_state::froggermc_audiocpu_irq_ack)
{
	// cleared when taking the interrupt using the m1 line
	// schematic: http://www.jrok.com/schem/FROGSND.pdf
	m_audiocpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
	return 0xff;
}

/*************************************
 *
 *  Frog (Falcon) I/O
 *
 *************************************/

READ8_MEMBER(galaxian_state::frogf_ppi8255_r)
{
	/* the decoding here is very simplistic, and you can address both simultaneously */
	UINT8 result = 0xff;
	if (offset & 0x1000) result &= m_ppi8255_0->read(space, (offset >> 3) & 3);
	if (offset & 0x2000) result &= m_ppi8255_1->read(space, (offset >> 3) & 3);
	return result;
}


WRITE8_MEMBER(galaxian_state::frogf_ppi8255_w)
{
	/* the decoding here is very simplistic, and you can address both simultaneously */
	if (offset & 0x1000) m_ppi8255_0->write(space, (offset >> 3) & 3, data);
	if (offset & 0x2000) m_ppi8255_1->write(space, (offset >> 3) & 3, data);
}



/*************************************
 *
 *  Turtles I/O
 *
 *************************************/

READ8_MEMBER(galaxian_state::turtles_ppi8255_0_r){ return m_ppi8255_0->read(space, (offset >> 4) & 3); }
READ8_MEMBER(galaxian_state::turtles_ppi8255_1_r){ return m_ppi8255_1->read(space, (offset >> 4) & 3); }
WRITE8_MEMBER(galaxian_state::turtles_ppi8255_0_w){ m_ppi8255_0->write(space, (offset >> 4) & 3, data); }
WRITE8_MEMBER(galaxian_state::turtles_ppi8255_1_w){ m_ppi8255_1->write(space, (offset >> 4) & 3, data); }



/*************************************
 *
 *  Scorpion sound I/O
 *
 *************************************/

READ8_MEMBER(galaxian_state::scorpion_ay8910_r)
{
	/* the decoding here is very simplistic, and you can address both simultaneously */
	UINT8 result = 0xff;
	if (offset & 0x08) result &= m_ay8910_2->data_r(space, 0);
	if (offset & 0x20) result &= m_ay8910_1->data_r(space, 0);
	if (offset & 0x80) result &= m_ay8910_0->data_r(space, 0);
	return result;
}


WRITE8_MEMBER(galaxian_state::scorpion_ay8910_w)
{
	/* the decoding here is very simplistic, and you can address all six simultaneously */
	if (offset & 0x04) m_ay8910_2->address_w(space, 0, data);
	if (offset & 0x08) m_ay8910_2->data_w(space, 0, data);
	if (offset & 0x10) m_ay8910_1->address_w(space, 0, data);
	if (offset & 0x20) m_ay8910_1->data_w(space, 0, data);
	if (offset & 0x40) m_ay8910_0->address_w(space, 0, data);
	if (offset & 0x80) m_ay8910_0->data_w(space, 0, data);
}


READ8_MEMBER(galaxian_state::scorpion_protection_r)
{
	UINT16 paritybits;
	UINT8 parity = 0;

	/* compute parity of the current (bitmask & $CE29) */
	for (paritybits = m_protection_state & 0xce29; paritybits != 0; paritybits >>= 1)
		if (paritybits & 1)
			parity++;

	/* only the low bit matters for protection, but bit 2 is also checked */
	return parity;
}


WRITE8_MEMBER(galaxian_state::scorpion_protection_w)
{
	/* bit 5 low is a reset */
	if (!(data & 0x20))
		m_protection_state = 0x0000;

	/* bit 4 low is a clock */
	if (!(data & 0x10))
	{
		/* each clock shifts left one bit and ORs in the inverse of the parity */
		m_protection_state = (m_protection_state << 1) | (~scorpion_protection_r(space, 0) & 1);
	}
}

READ8_MEMBER(galaxian_state::scorpion_digitalker_intr_r)
{
	return m_digitalker->digitalker_0_intr_r();
}

WRITE8_MEMBER(galaxian_state::scorpion_digitalker_control_w)
{
	m_digitalker->digitalker_0_cs_w(data & 1 ? ASSERT_LINE : CLEAR_LINE);
	m_digitalker->digitalker_0_cms_w(data & 2 ? ASSERT_LINE : CLEAR_LINE);
	m_digitalker->digitalker_0_wr_w(data & 4 ? ASSERT_LINE : CLEAR_LINE);
}

/*************************************
 *
 *  Ghostmuncher Galaxian I/O
 *
 *************************************/

INPUT_CHANGED_MEMBER(galaxian_state::gmgalax_game_changed)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* new value is the selected game */
	m_gmgalax_selected_game = newval;

	/* select the bank and graphics bank based on it */
	membank("bank1")->set_entry(m_gmgalax_selected_game);
	galaxian_gfxbank_w(space, 0, m_gmgalax_selected_game);

	/* reset the stars */
	galaxian_stars_enable_w(space, 0, 0);

	/* reset the CPU */
	m_maincpu->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
}


CUSTOM_INPUT_MEMBER(galaxian_state::gmgalax_port_r)
{
	const char *portname = (const char *)param;
	if (m_gmgalax_selected_game != 0)
		portname += strlen(portname) + 1;
	return ioport(portname)->read();
}



/*************************************
 *
 *  Zig Zag I/O
 *
 *************************************/

WRITE8_MEMBER(galaxian_state::zigzag_bankswap_w)
{
	/* Zig Zag can swap ROMs 2 and 3 as a form of copy protection */
	membank("bank1")->set_entry(data & 1);
	membank("bank2")->set_entry(~data & 1);
}


WRITE8_MEMBER(galaxian_state::zigzag_ay8910_w)
{
	switch (offset & 0x300)
	{
		case 0x000:
			/* control lines */
			/* bit 0 = WRITE */
			/* bit 1 = C/D */
			if ((offset & 1) != 0)
				m_ay8910_0->data_address_w(space, offset >> 1, m_zigzag_ay8910_latch);
			break;

		case 0x100:
			/* data latch */
			m_zigzag_ay8910_latch = offset & 0xff;
			break;

		case 0x200:
			/* unknown */
			break;
	}
}



/*************************************
 *
 *  Azurian I/O
 *
 *************************************/

CUSTOM_INPUT_MEMBER(galaxian_state::azurian_port_r)
{
	return (ioport("FAKE")->read() >> (FPTR)param) & 1;
}



/*************************************
 *
 *  King & Balloon I/O
 *
 *************************************/

CUSTOM_INPUT_MEMBER(galaxian_state::kingball_muxbit_r)
{
	/* multiplex the service mode switch with a speech DIP switch */
	return (ioport("FAKE")->read() >> m_kingball_speech_dip) & 1;
}


CUSTOM_INPUT_MEMBER(galaxian_state::kingball_noise_r)
{
	/* bit 5 is the NOISE line from the sound circuit.  The code just verifies
	   that it's working, doesn't actually use return value, so we can just use
	   rand() */
	return machine().rand() & 1;
}


WRITE8_MEMBER(galaxian_state::kingball_speech_dip_w)
{
	m_kingball_speech_dip = data;
}


WRITE8_MEMBER(galaxian_state::kingball_sound1_w)
{
	m_kingball_sound = (m_kingball_sound & ~0x01) | data;
}


WRITE8_MEMBER(galaxian_state::kingball_sound2_w)
{
	m_kingball_sound = (m_kingball_sound & ~0x02) | (data << 1);
	soundlatch_byte_w(space, 0, m_kingball_sound | 0xf0);
}


WRITE8_MEMBER(galaxian_state::kingball_dac_w)
{
	m_dac->write_unsigned8(data ^ 0xff);
}



/*************************************
 *
 *  Moon Shuttle I/O
 *
 *************************************/

WRITE8_MEMBER(galaxian_state::mshuttle_ay8910_cs_w)
{
	m_mshuttle_ay8910_cs = data & 1;
}


WRITE8_MEMBER(galaxian_state::mshuttle_ay8910_control_w)
{
	if (!m_mshuttle_ay8910_cs)
		m_ay8910_cclimber->address_w(space, offset, data);
}


WRITE8_MEMBER(galaxian_state::mshuttle_ay8910_data_w)
{
	if (!m_mshuttle_ay8910_cs)
		m_ay8910_cclimber->data_w(space, offset, data);
}


READ8_MEMBER(galaxian_state::mshuttle_ay8910_data_r)
{
	if (!m_mshuttle_ay8910_cs)
		return m_ay8910_cclimber->data_r(space, offset);
	return 0xff;
}



/*************************************
 *
 *  Jump Bug I/O
 *
 *************************************/

READ8_MEMBER(galaxian_state::jumpbug_protection_r)
{
	switch (offset)
	{
		case 0x0114:  return 0x4f;
		case 0x0118:  return 0xd3;
		case 0x0214:  return 0xcf;
		case 0x0235:  return 0x02;
		case 0x0311:  return 0xff;  /* not checked */
	}
	logerror("Unknown protection read. Offset: %04X  PC=%04X\n",0xb000+offset,space.device().safe_pc());
	return 0xff;
}



/*************************************
 *
 *  Checkman I/O
 *
 *************************************/

WRITE8_MEMBER(galaxian_state::checkman_sound_command_w)
{
	soundlatch_byte_w(space, 0, data);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}


TIMER_DEVICE_CALLBACK_MEMBER(galaxian_state::checkmaj_irq0_gen)
{
	m_audiocpu->set_input_line(0, HOLD_LINE);
}


READ8_MEMBER(galaxian_state::checkmaj_protection_r)
{
	switch (space.device().safe_pc())
	{
		case 0x0f15:  return 0xf5;
		case 0x0f8f:  return 0x7c;
		case 0x10b3:  return 0x7c;
		case 0x10e0:  return 0x00;
		case 0x10f1:  return 0xaa;
		case 0x1402:  return 0xaa;
		default:
			logerror("Unknown protection read. PC=%04X\n",space.device().safe_pc());
	}

	return 0;
}



/*************************************
 *
 *  Dingo I/O
 *
 *************************************/

READ8_MEMBER(galaxian_state::dingo_3000_r)
{
	return 0xaa;
}


READ8_MEMBER(galaxian_state::dingo_3035_r)
{
	return 0x8c;
}


READ8_MEMBER(galaxian_state::dingoe_3001_r)
{
	return 0xaa;
}


/*************************************
 *
 *  Moon War I/O
 *
 *************************************/

WRITE8_MEMBER(galaxian_state::moonwar_port_select_w)
{
	m_moonwar_port_select = data & 0x10;
}


/*************************************
 *
 *  Memory maps
 *
 *************************************/

/*
0000-3fff


4000-7fff
  4000-47ff -> RAM read/write (10 bits = 0x400)
  4800-4fff -> n/c
  5000-57ff -> /VRAM RD or /VRAM WR (10 bits = 0x400)
  5800-5fff -> /OBJRAM RD or /OBJRAM WR (8 bits = 0x100)
  6000-67ff -> /SW0 or /DRIVER
  6800-6fff -> /SW1 or /SOUND
  7000-77ff -> /DIPSW or LATCH
  7800-7fff -> /WDR or /PITCH

/DRIVER: (write 6000-67ff)
  D0 = data bit
  A0-A2 = decoder
  6000 -> 1P START
  6001 -> 2P START
  6002 -> COIN LOCKOUT
  6003 -> COIN COUNTER
  6004 -> 1M resistor (controls 555 timer @ 9R)
  6005 -> 470k resistor (controls 555 timer @ 9R)
  6006 -> 220k resistor (controls 555 timer @ 9R)
  6007 -> 100k resistor (controls 555 timer @ 9R)

/SOUND: (write 6800-6fff)
  D0 = data bit
  A0-A2 = decoder
  6800 -> FS1 (enables 555 timer at 8R)
  6801 -> FS2 (enables 555 timer at 8S)
  6802 -> FS3 (enables 555 timer at 8T)
  6803 -> HIT
  6804 -> n/c
  6805 -> FIRE
  6806 -> VOL1
  6807 -> VOL2

LATCH: (write 7000-77ff)
  D0 = data bit
  A0-A2 = decoder
  7000 -> n/c
  7001 -> NMI ON
  7002 -> n/c
  7003 -> n/c
  7004 -> STARS ON
  7005 -> n/c
  7006 -> HFLIP
  7007 -> VFLIP

/PITCH: (write 7800-7fff)
  loads latch at 9J
*/

/* map derived from schematics */

static ADDRESS_MAP_START( galaxian_map_discrete, AS_PROGRAM, 8, galaxian_state )
	AM_RANGE(0x6004, 0x6007) AM_MIRROR(0x07f8) AM_DEVWRITE("cust", galaxian_sound_device, lfo_freq_w)
	AM_RANGE(0x6800, 0x6807) AM_MIRROR(0x07f8) AM_DEVWRITE("cust", galaxian_sound_device, sound_w)
	AM_RANGE(0x7800, 0x7800) AM_MIRROR(0x07ff) AM_DEVWRITE("cust", galaxian_sound_device, pitch_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( galaxian_map_base, AS_PROGRAM, 8, galaxian_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_MIRROR(0x0400) AM_RAM
	AM_RANGE(0x5000, 0x53ff) AM_MIRROR(0x0400) AM_RAM_WRITE(galaxian_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x5800, 0x58ff) AM_MIRROR(0x0700) AM_RAM_WRITE(galaxian_objram_w) AM_SHARE("spriteram")
	AM_RANGE(0x6000, 0x6000) AM_MIRROR(0x07ff) AM_READ_PORT("IN0")
	AM_RANGE(0x6000, 0x6001) AM_MIRROR(0x07f8) AM_WRITE(start_lamp_w)
	AM_RANGE(0x6002, 0x6002) AM_MIRROR(0x07f8) AM_WRITE(coin_lock_w)
	AM_RANGE(0x6003, 0x6003) AM_MIRROR(0x07f8) AM_WRITE(coin_count_0_w)
	//AM_RANGE(0x6004, 0x6007) AM_MIRROR(0x07f8) AM_DEVWRITE("cust", galaxian_sound_device, lfo_freq_w)
	AM_RANGE(0x6800, 0x6800) AM_MIRROR(0x07ff) AM_READ_PORT("IN1")
	//AM_RANGE(0x6800, 0x6807) AM_MIRROR(0x07f8) AM_DEVWRITE("cust", galaxian_sound_device, sound_w)
	AM_RANGE(0x7000, 0x7000) AM_MIRROR(0x07ff) AM_READ_PORT("IN2")
	AM_RANGE(0x7001, 0x7001) AM_MIRROR(0x07f8) AM_WRITE(irq_enable_w)
	AM_RANGE(0x7004, 0x7004) AM_MIRROR(0x07f8) AM_WRITE(galaxian_stars_enable_w)
	AM_RANGE(0x7006, 0x7006) AM_MIRROR(0x07f8) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0x7007, 0x7007) AM_MIRROR(0x07f8) AM_WRITE(galaxian_flip_screen_y_w)
	//AM_RANGE(0x7800, 0x7800) AM_MIRROR(0x07ff) AM_DEVWRITE("cust", galaxian_sound_device, pitch_w)
	AM_RANGE(0x7800, 0x7800) AM_MIRROR(0x07ff) AM_READ(watchdog_reset_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( galaxian_map, AS_PROGRAM, 8, galaxian_state )
	AM_IMPORT_FROM(galaxian_map_base)
	AM_IMPORT_FROM(galaxian_map_discrete)
ADDRESS_MAP_END

/* map derived from schematics */

static ADDRESS_MAP_START( mooncrst_map_discrete, AS_PROGRAM, 8, galaxian_state )
	AM_RANGE(0xa004, 0xa007) AM_MIRROR(0x07f8) AM_DEVWRITE("cust", galaxian_sound_device, lfo_freq_w)
	AM_RANGE(0xa800, 0xa807) AM_MIRROR(0x07f8) AM_DEVWRITE("cust", galaxian_sound_device, sound_w)
	AM_RANGE(0xb800, 0xb800) AM_MIRROR(0x07ff) AM_DEVWRITE("cust", galaxian_sound_device, pitch_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mooncrst_map_base, AS_PROGRAM, 8, galaxian_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_MIRROR(0x0400) AM_RAM
	AM_RANGE(0x9000, 0x93ff) AM_MIRROR(0x0400) AM_RAM_WRITE(galaxian_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x9800, 0x98ff) AM_MIRROR(0x0700) AM_RAM_WRITE(galaxian_objram_w) AM_SHARE("spriteram")
	AM_RANGE(0xa000, 0xa000) AM_MIRROR(0x07ff) AM_READ_PORT("IN0")
	AM_RANGE(0xa000, 0xa002) AM_MIRROR(0x07f8) AM_WRITE(galaxian_gfxbank_w)
	AM_RANGE(0xa003, 0xa003) AM_MIRROR(0x07f8) AM_WRITE(coin_count_0_w)
//  AM_RANGE(0xa004, 0xa007) AM_MIRROR(0x07f8) AM_DEVWRITE("cust", galaxian_sound_device, lfo_freq_w)
	AM_RANGE(0xa800, 0xa800) AM_MIRROR(0x07ff) AM_READ_PORT("IN1")
//  AM_RANGE(0xa800, 0xa807) AM_MIRROR(0x07f8) AM_DEVWRITE("cust", galaxian_sound_device, sound_w)
	AM_RANGE(0xb000, 0xb000) AM_MIRROR(0x07ff) AM_READ_PORT("IN2")
	AM_RANGE(0xb000, 0xb000) AM_MIRROR(0x07f8) AM_WRITE(irq_enable_w)
	AM_RANGE(0xb004, 0xb004) AM_MIRROR(0x07f8) AM_WRITE(galaxian_stars_enable_w)
	AM_RANGE(0xb006, 0xb006) AM_MIRROR(0x07f8) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0xb007, 0xb007) AM_MIRROR(0x07f8) AM_WRITE(galaxian_flip_screen_y_w)
//  AM_RANGE(0xb800, 0xb800) AM_MIRROR(0x07ff) AM_DEVWRITE("cust", galaxian_sound_device, pitch_w)
	AM_RANGE(0xb800, 0xb800) AM_MIRROR(0x07ff) AM_READ(watchdog_reset_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( moonqsr_decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 8, galaxian_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_SHARE("decrypted_opcodes")
ADDRESS_MAP_END

static ADDRESS_MAP_START( mooncrst_map, AS_PROGRAM, 8, galaxian_state )
	AM_IMPORT_FROM(mooncrst_map_base)
	AM_IMPORT_FROM(mooncrst_map_discrete)
ADDRESS_MAP_END


static ADDRESS_MAP_START( fantastc_map, AS_PROGRAM, 8, galaxian_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8803, 0x8803) AM_DEVWRITE("8910.0", ay8910_device, address_w)
	AM_RANGE(0x880b, 0x880b) AM_DEVWRITE("8910.0", ay8910_device, data_w)
	AM_RANGE(0x880c, 0x880c) AM_DEVWRITE("8910.1", ay8910_device, address_w)
	AM_RANGE(0x880e, 0x880e) AM_DEVWRITE("8910.1", ay8910_device, data_w)
	AM_RANGE(0x9000, 0x93ff) AM_MIRROR(0x0400) AM_RAM_WRITE(galaxian_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x9800, 0x98ff) AM_MIRROR(0x0700) AM_RAM_WRITE(galaxian_objram_w) AM_SHARE("spriteram")
	AM_RANGE(0xa000, 0xa000) AM_MIRROR(0x07ff) AM_READ_PORT("IN0")
	AM_RANGE(0xa800, 0xa800) AM_MIRROR(0x07ff) AM_READ_PORT("IN1")
	AM_RANGE(0xb000, 0xb000) AM_MIRROR(0x07ff) AM_READ_PORT("IN2")
	AM_RANGE(0xb000, 0xb000) AM_MIRROR(0x07f8) AM_WRITE(irq_enable_w)
	AM_RANGE(0xb800, 0xb800) AM_MIRROR(0x07ff) AM_READ(watchdog_reset_r)
	AM_RANGE(0xfffe, 0xfffe) AM_NOP // ?
ADDRESS_MAP_END

static ADDRESS_MAP_START( timefgtr_map, AS_PROGRAM, 8, galaxian_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8803, 0x8803) AM_DEVWRITE("8910.0", ay8910_device, address_w)
	AM_RANGE(0x880b, 0x880b) AM_DEVWRITE("8910.0", ay8910_device, data_w)
	AM_RANGE(0x880c, 0x880c) AM_DEVWRITE("8910.1", ay8910_device, address_w)
	AM_RANGE(0x880e, 0x880e) AM_DEVWRITE("8910.1", ay8910_device, data_w)
	AM_RANGE(0x9000, 0x93ff) AM_MIRROR(0x0400) AM_RAM_WRITE(galaxian_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x9800, 0x9bff) AM_MIRROR(0x0400) AM_RAM_WRITE(galaxian_objram_w) AM_SHARE("spriteram")
	AM_RANGE(0xa000, 0xa000) AM_MIRROR(0x07ff) AM_READ_PORT("IN0")
	AM_RANGE(0xa800, 0xa800) AM_MIRROR(0x07ff) AM_READ_PORT("IN1")
	AM_RANGE(0xb000, 0xb000) AM_MIRROR(0x07ff) AM_READ_PORT("IN2")
	AM_RANGE(0xb000, 0xb000) AM_MIRROR(0x07f8) AM_WRITE(irq_enable_w)
	AM_RANGE(0xb004, 0xb004) AM_MIRROR(0x07f8) AM_WRITE(galaxian_stars_enable_w)
	AM_RANGE(0xb800, 0xb800) AM_MIRROR(0x07ff) AM_READ(watchdog_reset_r)
//  AM_RANGE(0xb800, 0xb800) AM_WRITENOP // ?
//  AM_RANGE(0xfff8, 0xffff) AM_WRITENOP // sound related?
ADDRESS_MAP_END


/* map derived from schematics */
#if 0
static ADDRESS_MAP_START( dambustr_map, AS_PROGRAM, 8, galaxian_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_ROM
//  AM_RANGE(0x8000, 0x8000) AM_WRITE(dambustr_bg_color_w)
//  AM_RANGE(0x8001, 0x8001) AM_WRITE(dambustr_bg_split_line_w)
	AM_RANGE(0xc000, 0xc3ff) AM_MIRROR(0x0400) AM_RAM
	AM_RANGE(0xd000, 0xd3ff) AM_MIRROR(0x0400) AM_RAM_WRITE(galaxian_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xd800, 0xd8ff) AM_MIRROR(0x0700) AM_RAM_WRITE(galaxian_objram_w) AM_SHARE("spriteram")
	AM_RANGE(0xe000, 0xe000) AM_MIRROR(0x07ff) AM_READ_PORT("IN0")
	AM_RANGE(0xe004, 0xe007) AM_MIRROR(0x07f8) AM_DEVWRITE("cust", galaxian_sound_device, lfo_freq_w)
	AM_RANGE(0xe800, 0xe800) AM_MIRROR(0x07ff) AM_READ_PORT("IN1")
	AM_RANGE(0xe800, 0xe807) AM_MIRROR(0x07f8) AM_DEVWRITE("cust", galaxian_sound_device, sound_w)
	AM_RANGE(0xf000, 0xf000) AM_MIRROR(0x07ff) AM_READ_PORT("IN2")
	AM_RANGE(0xf001, 0xf001) AM_MIRROR(0x07f8) AM_WRITE(irq_enable_w)
	AM_RANGE(0xf004, 0xf004) AM_MIRROR(0x07f8) AM_WRITE(galaxian_stars_enable_w)
	AM_RANGE(0xf006, 0xf006) AM_MIRROR(0x07f8) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0xf007, 0xf007) AM_MIRROR(0x07f8) AM_WRITE(galaxian_flip_screen_y_w)
	AM_RANGE(0xf800, 0xf800) AM_MIRROR(0x07ff) AM_READ(watchdog_reset_r)
	AM_RANGE(0xf800, 0xf800) AM_MIRROR(0x07ff) AM_DEVWRITE("cust", galaxian_sound_device, pitch_w)
ADDRESS_MAP_END
#endif


/* map derived from schematics */
static ADDRESS_MAP_START( theend_map, AS_PROGRAM, 8, galaxian_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x4800, 0x4bff) AM_MIRROR(0x0400) AM_RAM_WRITE(galaxian_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x5000, 0x50ff) AM_MIRROR(0x0700) AM_RAM_WRITE(galaxian_objram_w) AM_SHARE("spriteram")
	AM_RANGE(0x6801, 0x6801) AM_MIRROR(0x07f8) AM_WRITE(irq_enable_w)
	AM_RANGE(0x6802, 0x6802) AM_MIRROR(0x07f8) AM_WRITE(coin_count_0_w)
	AM_RANGE(0x6803, 0x6803) AM_MIRROR(0x07f8) AM_WRITE(scramble_background_enable_w)
	AM_RANGE(0x6804, 0x6804) AM_MIRROR(0x07f8) AM_WRITE(galaxian_stars_enable_w)
	AM_RANGE(0x6805, 0x6805) AM_MIRROR(0x07f8) //POUT2
	AM_RANGE(0x6806, 0x6806) AM_MIRROR(0x07f8) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0x6807, 0x6807) AM_MIRROR(0x07f8) AM_WRITE(galaxian_flip_screen_y_w)
	AM_RANGE(0x7000, 0x7000) AM_MIRROR(0x07ff) AM_READ(watchdog_reset_r)
	AM_RANGE(0x8000, 0xffff) AM_READWRITE(theend_ppi8255_r, theend_ppi8255_w)
ADDRESS_MAP_END


/* map derived from schematics */
static ADDRESS_MAP_START( scobra_map, AS_PROGRAM, 8, galaxian_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_MIRROR(0x4000) AM_RAM
	AM_RANGE(0x8800, 0x8bff) AM_MIRROR(0x4400) AM_RAM_WRITE(galaxian_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x9000, 0x90ff) AM_MIRROR(0x4700) AM_RAM_WRITE(galaxian_objram_w) AM_SHARE("spriteram")
	AM_RANGE(0x9800, 0x9803) AM_MIRROR(0x47fc) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0xa000, 0xa003) AM_MIRROR(0x47fc) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
	AM_RANGE(0xa801, 0xa801) AM_MIRROR(0x47f8) AM_WRITE(irq_enable_w)
	AM_RANGE(0xa802, 0xa802) AM_MIRROR(0x47f8) AM_WRITE(coin_count_0_w)
	AM_RANGE(0xa803, 0xa803) AM_MIRROR(0x47f8) AM_WRITE(scramble_background_enable_w)
	AM_RANGE(0xa804, 0xa804) AM_MIRROR(0x47f8) AM_WRITE(galaxian_stars_enable_w)
	AM_RANGE(0xa805, 0xa805) AM_MIRROR(0x47f8) /* POUT2 */
	AM_RANGE(0xa806, 0xa806) AM_MIRROR(0x47f8) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0xa807, 0xa807) AM_MIRROR(0x47f8) AM_WRITE(galaxian_flip_screen_y_w)
	AM_RANGE(0xb000, 0xb000) AM_MIRROR(0x47ff) AM_READ(watchdog_reset_r)
ADDRESS_MAP_END


static ADDRESS_MAP_START( anteateruk_map, AS_PROGRAM, 8, galaxian_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x03ff) AM_ROM
	AM_RANGE(0x0400, 0x0bff) AM_RAM
	AM_RANGE(0x0c00, 0x0fff) AM_RAM_WRITE(galaxian_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x1001, 0x1001) AM_MIRROR(0x01f8) AM_WRITE(irq_enable_w)
	AM_RANGE(0x1002, 0x1002) AM_MIRROR(0x01f8) AM_WRITE(coin_count_0_w)
	AM_RANGE(0x1003, 0x1003) AM_MIRROR(0x01f8) AM_WRITE(scramble_background_enable_w)
	AM_RANGE(0x1004, 0x1004) AM_MIRROR(0x01f8) AM_WRITE(galaxian_stars_enable_w)
	AM_RANGE(0x1005, 0x1005) AM_MIRROR(0x01f8) //POUT2
	AM_RANGE(0x1006, 0x1006) AM_MIRROR(0x01f8) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0x1007, 0x1007) AM_MIRROR(0x01f8) AM_WRITE(galaxian_flip_screen_y_w)
	AM_RANGE(0x1200, 0x12ff) AM_MIRROR(0x0100) AM_RAM_WRITE(galaxian_objram_w) AM_SHARE("spriteram")
	AM_RANGE(0x1400, 0x1400) AM_MIRROR(0x03ff) AM_READ(watchdog_reset_r)
	AM_RANGE(0x4000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc003) AM_MIRROR(0x3efc) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
	AM_RANGE(0xc100, 0xc103) AM_MIRROR(0x3efc) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
ADDRESS_MAP_END

// this is WRONG (a copy of the above) but set does appear to have a similar odd memory map with RAM aronud 0x400 and scrambled ROMs
static ADDRESS_MAP_START( spactrai_map, AS_PROGRAM, 8, galaxian_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x03ff) AM_ROM
	AM_RANGE(0x0400, 0x0bff) AM_RAM
	AM_RANGE(0x0c00, 0x0fff) AM_RAM_WRITE(galaxian_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x1001, 0x1001) AM_MIRROR(0x01f8) AM_WRITE(irq_enable_w)
	AM_RANGE(0x1002, 0x1002) AM_MIRROR(0x01f8) AM_WRITE(coin_count_0_w)
	AM_RANGE(0x1003, 0x1003) AM_MIRROR(0x01f8) AM_WRITE(scramble_background_enable_w)
	AM_RANGE(0x1004, 0x1004) AM_MIRROR(0x01f8) AM_WRITE(galaxian_stars_enable_w)
	AM_RANGE(0x1005, 0x1005) AM_MIRROR(0x01f8) //POUT2
	AM_RANGE(0x1006, 0x1006) AM_MIRROR(0x01f8) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0x1007, 0x1007) AM_MIRROR(0x01f8) AM_WRITE(galaxian_flip_screen_y_w)
	AM_RANGE(0x1200, 0x12ff) AM_MIRROR(0x0100) AM_RAM_WRITE(galaxian_objram_w) AM_SHARE("spriteram")
	AM_RANGE(0x1400, 0x1400) AM_MIRROR(0x03ff) AM_READ(watchdog_reset_r)
	AM_RANGE(0x4000, 0xbfff) AM_ROM
//  AM_RANGE(0xc000, 0xc003) AM_MIRROR(0x3efc) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
//  AM_RANGE(0xc100, 0xc103) AM_MIRROR(0x3efc) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START( anteaterg_map, AS_PROGRAM, 8, galaxian_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x03ff) AM_ROM
	AM_RANGE(0x0400, 0x0bff) AM_RAM
	AM_RANGE(0x0c00, 0x0fff) AM_RAM_WRITE(galaxian_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x2000, 0x20ff) AM_MIRROR(0x0300) AM_RAM_WRITE(galaxian_objram_w) AM_SHARE("spriteram")
	AM_RANGE(0x2400, 0x2403) AM_MIRROR(0x01fc) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
	AM_RANGE(0x2601, 0x2601) AM_MIRROR(0x01f8) AM_WRITE(irq_enable_w)
	AM_RANGE(0x2602, 0x2602) AM_MIRROR(0x01f8) AM_WRITE(coin_count_0_w)
	AM_RANGE(0x2603, 0x2603) AM_MIRROR(0x01f8) AM_WRITE(scramble_background_enable_w)
	AM_RANGE(0x2604, 0x2604) AM_MIRROR(0x01f8) AM_WRITE(galaxian_stars_enable_w)
	AM_RANGE(0x2605, 0x2605) AM_MIRROR(0x01f8) //POUT2
	AM_RANGE(0x2606, 0x2606) AM_MIRROR(0x01f8) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0x2607, 0x2607) AM_MIRROR(0x01f8) AM_WRITE(galaxian_flip_screen_y_w)
	AM_RANGE(0x4000, 0xbfff) AM_ROM
	AM_RANGE(0x7c00, 0x7fff) AM_RAM_WRITE(galaxian_videoram_w) AM_SHARE("videoram") /* mirror! */
	AM_RANGE(0xf400, 0xf400) AM_MIRROR(0x01ff) AM_READ(watchdog_reset_r)
	AM_RANGE(0xf600, 0xf603) AM_MIRROR(0x01fc) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
ADDRESS_MAP_END


/* map derived from schematics */
static ADDRESS_MAP_START( frogger_map, AS_PROGRAM, 8, galaxian_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0x8800) AM_MIRROR(0x07ff) AM_READ(watchdog_reset_r)
	AM_RANGE(0xa800, 0xabff) AM_MIRROR(0x0400) AM_RAM_WRITE(galaxian_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xb000, 0xb0ff) AM_MIRROR(0x0700) AM_RAM_WRITE(galaxian_objram_w) AM_SHARE("spriteram")
	AM_RANGE(0xb808, 0xb808) AM_MIRROR(0x07e3) AM_WRITE(irq_enable_w)
	AM_RANGE(0xb80c, 0xb80c) AM_MIRROR(0x07e3) AM_WRITE(galaxian_flip_screen_y_w)
	AM_RANGE(0xb810, 0xb810) AM_MIRROR(0x07e3) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0xb818, 0xb818) AM_MIRROR(0x07e3) AM_WRITE(coin_count_0_w) /* IOPC7 */
	AM_RANGE(0xb81c, 0xb81c) AM_MIRROR(0x07e3) AM_WRITE(coin_count_1_w) /* POUT1 */
	AM_RANGE(0xc000, 0xffff) AM_READWRITE(frogger_ppi8255_r, frogger_ppi8255_w)
ADDRESS_MAP_END


/* map derived from schematics */
static ADDRESS_MAP_START( turtles_map, AS_PROGRAM, 8, galaxian_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_MIRROR(0x4000) AM_RAM
	AM_RANGE(0x9000, 0x93ff) AM_MIRROR(0x4400) AM_RAM_WRITE(galaxian_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x9800, 0x98ff) AM_MIRROR(0x4700) AM_RAM_WRITE(galaxian_objram_w) AM_SHARE("spriteram")
	AM_RANGE(0xa000, 0xa000) AM_MIRROR(0x47c7) AM_WRITE(scramble_background_red_w)
	AM_RANGE(0xa008, 0xa008) AM_MIRROR(0x47c7) AM_WRITE(irq_enable_w)
	AM_RANGE(0xa010, 0xa010) AM_MIRROR(0x47c7) AM_WRITE(galaxian_flip_screen_y_w)
	AM_RANGE(0xa018, 0xa018) AM_MIRROR(0x47c7) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0xa020, 0xa020) AM_MIRROR(0x47c7) AM_WRITE(scramble_background_green_w)
	AM_RANGE(0xa028, 0xa028) AM_MIRROR(0x47c7) AM_WRITE(scramble_background_blue_w)
	AM_RANGE(0xa030, 0xa030) AM_MIRROR(0x47c7) AM_WRITE(coin_count_0_w)
	AM_RANGE(0xa038, 0xa038) AM_MIRROR(0x47c7) AM_WRITE(coin_count_1_w)
	AM_RANGE(0xa800, 0xa800) AM_MIRROR(0x47ff) AM_READ(watchdog_reset_r)
	AM_RANGE(0xb000, 0xb03f) AM_MIRROR(0x47cf) AM_READWRITE(turtles_ppi8255_0_r, turtles_ppi8255_0_w)
	AM_RANGE(0xb800, 0xb83f) AM_MIRROR(0x47cf) AM_READWRITE(turtles_ppi8255_1_r, turtles_ppi8255_1_w)
ADDRESS_MAP_END


/* this is the same as theend, except for separate RGB background controls
   and some extra ROM space at $7000 and $C000 */
static ADDRESS_MAP_START( sfx_map, AS_PROGRAM, 8, galaxian_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x4800, 0x4bff) AM_MIRROR(0x0400) AM_RAM_WRITE(galaxian_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x5000, 0x50ff) AM_MIRROR(0x0700) AM_RAM_WRITE(galaxian_objram_w) AM_SHARE("spriteram")
	AM_RANGE(0x6800, 0x6800) AM_MIRROR(0x07f8) AM_WRITE(scramble_background_red_w)
	AM_RANGE(0x6801, 0x6801) AM_MIRROR(0x07f8) AM_WRITE(irq_enable_w)
	AM_RANGE(0x6802, 0x6802) AM_MIRROR(0x07f8) AM_WRITE(coin_count_0_w)
	AM_RANGE(0x6803, 0x6803) AM_MIRROR(0x07f8) AM_WRITE(scramble_background_blue_w)
	AM_RANGE(0x6804, 0x6804) AM_MIRROR(0x07f8) AM_WRITE(galaxian_stars_enable_w)
	AM_RANGE(0x6805, 0x6805) AM_MIRROR(0x07f8) AM_WRITE(scramble_background_green_w)
	AM_RANGE(0x6806, 0x6806) AM_MIRROR(0x07f8) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0x6807, 0x6807) AM_MIRROR(0x07f8) AM_WRITE(galaxian_flip_screen_y_w)
	AM_RANGE(0x7000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_READWRITE(theend_ppi8255_r, theend_ppi8255_w)
	AM_RANGE(0xc000, 0xefff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( monsterz_map, AS_PROGRAM, 8, galaxian_state )
	AM_RANGE(0x0000, 0x37ff) AM_ROM
	AM_RANGE(0x3800, 0x3fff) AM_RAM // extra RAM used by protection
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x4800, 0x4bff) AM_MIRROR(0x0400) AM_RAM_WRITE(galaxian_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x5000, 0x50ff) AM_MIRROR(0x0700) AM_RAM_WRITE(galaxian_objram_w) AM_SHARE("spriteram")
	AM_RANGE(0x6800, 0x6800) AM_MIRROR(0x07f8) AM_WRITE(scramble_background_red_w)
	AM_RANGE(0x6801, 0x6801) AM_MIRROR(0x07f8) AM_WRITE(irq_enable_w)
	AM_RANGE(0x6802, 0x6802) AM_MIRROR(0x07f8) AM_WRITE(coin_count_0_w)
	AM_RANGE(0x6803, 0x6803) AM_MIRROR(0x07f8) AM_WRITE(scramble_background_blue_w)
	AM_RANGE(0x6804, 0x6804) AM_MIRROR(0x07f8) AM_WRITE(galaxian_stars_enable_w)
	AM_RANGE(0x6805, 0x6805) AM_MIRROR(0x07f8) AM_WRITE(scramble_background_green_w)
	AM_RANGE(0x6806, 0x6806) AM_MIRROR(0x07f8) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0x6807, 0x6807) AM_MIRROR(0x07f8) AM_WRITE(galaxian_flip_screen_y_w)
	AM_RANGE(0x8000, 0xbfff) AM_READWRITE(theend_ppi8255_r, theend_ppi8255_w)
	AM_RANGE(0xc000, 0xd7ff) AM_ROM
	AM_RANGE(0xd800, 0xd800) AM_READ(monsterz_protection_r)
ADDRESS_MAP_END


/* changes from galaxian map:
    galaxian sound removed
    $4800-$57ff: cointains video and object RAM (normally at $5000-$5fff)
    $5800-$5fff: AY-8910 access added
    $6002-$6006: graphics banking controls replace coin lockout, coin counter, and lfo
    $7002: coin counter (moved from $6003)
    $8000-$afff: additional ROM area
    $b000-$bfff: protection
*/
static ADDRESS_MAP_START( jumpbug_map, AS_PROGRAM, 8, galaxian_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x4800, 0x4bff) AM_MIRROR(0x0400) AM_RAM_WRITE(galaxian_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x5000, 0x50ff) AM_MIRROR(0x0700) AM_RAM_WRITE(galaxian_objram_w) AM_SHARE("spriteram")
	AM_RANGE(0x5800, 0x5800) AM_MIRROR(0x00ff) AM_DEVWRITE("8910.0", ay8910_device, data_w)
	AM_RANGE(0x5900, 0x5900) AM_MIRROR(0x00ff) AM_DEVWRITE("8910.0", ay8910_device, address_w)
	AM_RANGE(0x6000, 0x6000) AM_MIRROR(0x07ff) AM_READ_PORT("IN0")
	AM_RANGE(0x6002, 0x6006) AM_MIRROR(0x07f8) AM_WRITE(galaxian_gfxbank_w)
	AM_RANGE(0x6800, 0x6800) AM_MIRROR(0x07ff) AM_READ_PORT("IN1")
	AM_RANGE(0x7000, 0x7000) AM_MIRROR(0x07ff) AM_READ_PORT("IN2")
	AM_RANGE(0x7001, 0x7001) AM_MIRROR(0x07f8) AM_WRITE(irq_enable_w)
	AM_RANGE(0x7002, 0x7002) AM_MIRROR(0x07f8) AM_WRITE(coin_count_0_w)
	AM_RANGE(0x7004, 0x7004) AM_MIRROR(0x07f8) AM_WRITE(galaxian_stars_enable_w)
	AM_RANGE(0x7006, 0x7006) AM_MIRROR(0x07f8) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0x7007, 0x7007) AM_MIRROR(0x07f8) AM_WRITE(galaxian_flip_screen_y_w)
	AM_RANGE(0x8000, 0xafff) AM_ROM
	AM_RANGE(0xb000, 0xbfff) AM_READ(jumpbug_protection_r)
ADDRESS_MAP_END


static ADDRESS_MAP_START( frogf_map, AS_PROGRAM, 8, galaxian_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0x8bff) AM_MIRROR(0x0400) AM_RAM_WRITE(galaxian_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x9000, 0x90ff) AM_MIRROR(0x0700) AM_RAM_WRITE(galaxian_objram_w) AM_SHARE("spriteram")
	AM_RANGE(0xa802, 0xa802) AM_MIRROR(0x07f1) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0xa804, 0xa804) AM_MIRROR(0x07f1) AM_WRITE(irq_enable_w)
	AM_RANGE(0xa806, 0xa806) AM_MIRROR(0x07f1) AM_WRITE(galaxian_flip_screen_y_w)
	AM_RANGE(0xa808, 0xa808) AM_MIRROR(0x07f1) AM_WRITE(coin_count_1_w)
	AM_RANGE(0xa80e, 0xa80e) AM_MIRROR(0x07f1) AM_WRITE(coin_count_0_w)
	AM_RANGE(0xb800, 0xb800) AM_MIRROR(0x07ff) AM_READ(watchdog_reset_r)
	AM_RANGE(0xc000, 0xffff) AM_READWRITE(frogf_ppi8255_r, frogf_ppi8255_w)
ADDRESS_MAP_END


/* mooncrst */
static ADDRESS_MAP_START( mshuttle_map, AS_PROGRAM, 8, galaxian_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM
	AM_RANGE(0x9000, 0x93ff) AM_MIRROR(0x0400) AM_RAM_WRITE(galaxian_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x9800, 0x98ff) AM_MIRROR(0x0700) AM_RAM_WRITE(galaxian_objram_w) AM_SHARE("spriteram")
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("IN0")
	AM_RANGE(0xa000, 0xa000) AM_WRITE(irq_enable_w)
	AM_RANGE(0xa001, 0xa001) AM_WRITE(galaxian_stars_enable_w)
	AM_RANGE(0xa002, 0xa002) AM_WRITE(galaxian_flip_screen_xy_w)
	AM_RANGE(0xa004, 0xa004) AM_DEVWRITE("cclimber_audio", cclimber_audio_device, sample_trigger_w)
	AM_RANGE(0xa007, 0xa007) AM_WRITE(mshuttle_ay8910_cs_w)
	AM_RANGE(0xa800, 0xa800) AM_READ_PORT("IN1")
	AM_RANGE(0xa800, 0xa800) AM_DEVWRITE("cclimber_audio", cclimber_audio_device, sample_rate_w)
	AM_RANGE(0xb000, 0xb000) AM_READ_PORT("IN2")
	AM_RANGE(0xb000, 0xb000) AM_DEVWRITE("cclimber_audio", cclimber_audio_device, sample_volume_w)
	AM_RANGE(0xb800, 0xb800) AM_READ(watchdog_reset_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mshuttle_decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 8, galaxian_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_SHARE("decrypted_opcodes")
ADDRESS_MAP_END

static ADDRESS_MAP_START( mshuttle_portmap, AS_IO, 8, galaxian_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0x0f)
	AM_RANGE(0x08, 0x08) AM_WRITE(mshuttle_ay8910_control_w)
	AM_RANGE(0x09, 0x09) AM_WRITE(mshuttle_ay8910_data_w)
	AM_RANGE(0x0c, 0x0c) AM_READ(mshuttle_ay8910_data_r)
ADDRESS_MAP_END


WRITE8_MEMBER(galaxian_state::tenspot_unk_6000_w)
{
	logerror("tenspot_unk_6000_w %02x\n",data);
}

WRITE8_MEMBER(galaxian_state::tenspot_unk_8000_w)
{
	logerror("tenspot_unk_8000_w %02x\n",data);
}

WRITE8_MEMBER(galaxian_state::tenspot_unk_e000_w)
{
	logerror("tenspot_unk_e000_w %02x\n",data);
}

static ADDRESS_MAP_START( tenspot_select_map, AS_PROGRAM, 8, galaxian_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
	AM_RANGE(0x4000, 0x4000) AM_READ_PORT("SELECT2")
	AM_RANGE(0x6000, 0x6000) AM_WRITE(tenspot_unk_6000_w)
	AM_RANGE(0xc000, 0xc000) AM_READ_PORT("SELECT")
	AM_RANGE(0x8000, 0x8000) AM_WRITE(tenspot_unk_8000_w)
	AM_RANGE(0xa000, 0xa03f) AM_RAM
	AM_RANGE(0xe000, 0xe000) AM_WRITE(tenspot_unk_e000_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( froggeram_map, AS_PROGRAM, 8, galaxian_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0x8800) AM_READ(watchdog_reset_r)
	AM_RANGE(0xa800, 0xabff) AM_RAM_WRITE(galaxian_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xb000, 0xb0ff) AM_RAM_WRITE(galaxian_objram_w) AM_SHARE("spriteram")
	AM_RANGE(0xb801, 0xb801) AM_WRITE(irq_enable_w)
	AM_RANGE(0xb806, 0xb806) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0xb807, 0xb807) AM_WRITE(galaxian_flip_screen_y_w)
//  AM_RANGE(0xb818, 0xb818) AM_WRITE(coin_count_0_w) /* IOPC7 */
//  AM_RANGE(0xb81c, 0xb81c) AM_WRITE(coin_count_1_w) /* POUT1 */
	// todo, map inputs properly for this version
ADDRESS_MAP_END

/*************************************
 *
 *  Sound CPU memory maps
 *
 *************************************/

/* Konami Frogger with 1 x AY-8910A */
static ADDRESS_MAP_START( frogger_sound_map, AS_PROGRAM, 8, galaxian_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_MIRROR(0x1c00) AM_RAM
	AM_RANGE(0x6000, 0x6fff) AM_MIRROR(0x1000) AM_WRITE(konami_sound_filter_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( frogger_sound_portmap, AS_IO, 8, galaxian_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0xff) AM_READWRITE(frogger_ay8910_r, frogger_ay8910_w)
ADDRESS_MAP_END


/* Konami generic with 2 x AY-8910A */
static ADDRESS_MAP_START( konami_sound_map, AS_PROGRAM, 8, galaxian_state )
	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_MIRROR(0x6c00) AM_RAM
	AM_RANGE(0x9000, 0x9fff) AM_MIRROR(0x6000) AM_WRITE(konami_sound_filter_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( konami_sound_portmap, AS_IO, 8, galaxian_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0xff) AM_READWRITE(konami_ay8910_r, konami_ay8910_w)
ADDRESS_MAP_END


/* Checkman with 1 x AY-8910A */
static ADDRESS_MAP_START( checkman_sound_map, AS_PROGRAM, 8, galaxian_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( checkman_sound_portmap, AS_IO, 8, galaxian_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x03, 0x03) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x04, 0x05) AM_DEVWRITE("8910.0", ay8910_device, address_data_w)
	AM_RANGE(0x06, 0x06) AM_DEVREAD("8910.0", ay8910_device, data_r)
ADDRESS_MAP_END


/* Checkman alternate with 1 x AY-8910A */
static ADDRESS_MAP_START( checkmaj_sound_map, AS_PROGRAM, 8, galaxian_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM
	AM_RANGE(0xa000, 0xa001) AM_DEVWRITE("8910.0", ay8910_device, address_data_w)
	AM_RANGE(0xa002, 0xa002) AM_DEVREAD("8910.0", ay8910_device, data_r)
ADDRESS_MAP_END


/* King and Balloon with DAC */
static ADDRESS_MAP_START( kingball_sound_map, AS_PROGRAM, 8, galaxian_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x0000, 0x03ff) AM_MIRROR(0x1c00) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( kingball_sound_portmap, AS_IO, 8, galaxian_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_MIRROR(0xff) AM_READ(soundlatch_byte_r) AM_WRITE(kingball_dac_w)
ADDRESS_MAP_END


/* SF-X sample player */
static ADDRESS_MAP_START( sfx_sample_map, AS_PROGRAM, 8, galaxian_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_MIRROR(0x6c00) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sfx_sample_portmap, AS_IO, 8, galaxian_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0xff) AM_READWRITE(sfx_sample_io_r, sfx_sample_io_w)
ADDRESS_MAP_END


/*************************************
 *
 *  Input Ports
 *  Galaxian-derived games
 *
 *************************************/

static INPUT_PORTS_START( galaxian )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_SERVICE( 0x40, IP_ACTIVE_HIGH )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Free_Play ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "7000" )
	PORT_DIPSETTING(    0x01, "10000" )
	PORT_DIPSETTING(    0x02, "12000" )
	PORT_DIPSETTING(    0x03, "20000" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPUNUSED( 0x08, 0x00 )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( galaxianbl )
	PORT_INCLUDE(galaxian)

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, DEF_STR(None) )
	PORT_DIPSETTING(    0x01, "10000" )
	PORT_DIPSETTING(    0x02, "20000" )
	PORT_DIPSETTING(    0x03, "30000" )
INPUT_PORTS_END


static INPUT_PORTS_START( galaxrf )
	PORT_INCLUDE(galaxian)

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x03, 0x01, "Bonus Life / Enemy Bullet Speed" ) // not verified this
	PORT_DIPSETTING(    0x00, "None, Slow" )
	PORT_DIPSETTING(    0x01, "12000, Medium" )
	PORT_DIPSETTING(    0x02, "20000, Fast" )
	PORT_DIPSETTING(    0x03, "30000, Fastest" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPNAME( 0x08, 0x00, "Player Bullet Speed" )
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x08, "Fast" )
INPUT_PORTS_END

static INPUT_PORTS_START( superg )
	PORT_INCLUDE(galaxian)

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x01, "4000" )
	PORT_DIPSETTING(    0x02, "5000" )
	PORT_DIPSETTING(    0x03, "7000" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
INPUT_PORTS_END


static INPUT_PORTS_START( swarm )
	PORT_INCLUDE(galaxian)

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x01, "10000" )
	PORT_DIPSETTING(    0x02, "20000" )
	PORT_DIPSETTING(    0x03, "40000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Difficulty ) )   /* aliens "flying" simultaneously */
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )             /* less aliens */
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )             /* more aliens */
INPUT_PORTS_END


static INPUT_PORTS_START( zerotime )
	PORT_INCLUDE(galaxian)

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x40, "A 1C/1C 2C/2C  B 1C/2C" )
	PORT_DIPSETTING(    0xc0, "A 1C/1C 2C/3C  B 1C/3C" )
	PORT_DIPSETTING(    0x00, "A 1C/2C 2C/4C  B 1C/4C" )
	PORT_DIPSETTING(    0x80, "A 1C/2C 2C/5C  B 1C/5C" )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x03, "6000" )
	PORT_DIPSETTING(    0x02, "7000" )
	PORT_DIPSETTING(    0x01, "9000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Difficulty ) )   /* player's bullet speed */
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )             /* gap of 6 pixels */
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )             /* gap of 8 pixels */
INPUT_PORTS_END


static INPUT_PORTS_START( blkhole )
	PORT_INCLUDE(galaxian)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPSETTING(    0x01, "10000" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPUNUSED( 0x04, 0x00 )
	PORT_DIPUNUSED( 0x08, 0x00 )
INPUT_PORTS_END


static INPUT_PORTS_START( orbitron )
	PORT_INCLUDE(galaxian)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY

	PORT_MODIFY("IN1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Coinage ) )      /* Routine at 0x00e1 */
	PORT_DIPSETTING(    0x00, "A 2C/1C  B 1C/3C" )
	PORT_DIPSETTING(    0x20, "A 2C/1C  B 1C/3C (duplicate)" )
	PORT_DIPSETTING(    0x40, "A 1C/1C  B 1C/6C" )
	PORT_DIPSETTING(    0x60, "A 1C/1C  B 1C/6C (duplicate)" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL

	PORT_MODIFY("IN2")
	PORT_DIPUNUSED( 0x01, 0x00 )
	PORT_DIPUNUSED( 0x02, 0x00 )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
INPUT_PORTS_END


/* These inputs are clearly wrong, they need a full test */
static INPUT_PORTS_START( luctoday )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_NAME("Add Credit to Bet")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_NAME("Remove Credit from Bet")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BILL1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( catacomb )
	PORT_INCLUDE(galaxian)

	PORT_MODIFY("IN0")
	PORT_DIPUNKNOWN( 0x40, 0x00 )
	PORT_DIPUNKNOWN( 0x80, 0x00 )

	PORT_MODIFY("IN1")
	PORT_DIPUNKNOWN( 0x20, 0x00 )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )

	PORT_MODIFY("IN2")
	PORT_DIPUNKNOWN( 0x01, 0x00 )
	PORT_DIPUNKNOWN( 0x02, 0x00 )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPUNKNOWN( 0x08, 0x00 )
INPUT_PORTS_END


static INPUT_PORTS_START( omega )
	PORT_INCLUDE(galaxian)

	PORT_MODIFY("IN0")
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x01, "20000" )
	PORT_DIPUNUSED( 0x02, 0x00 )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
INPUT_PORTS_END


static INPUT_PORTS_START( warofbug )
	PORT_INCLUDE(galaxian)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY

	PORT_MODIFY("IN1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Free_Play ) )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPUNUSED( 0x04, 0x00 )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "500000" )
	PORT_DIPSETTING(    0x00, "750000" )
INPUT_PORTS_END


static INPUT_PORTS_START( redufo )
	PORT_INCLUDE(galaxian)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )        // Not tested due to code removed at 0x1901 and 0x191a

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "7000" )
	PORT_DIPSETTING(    0x01, DEF_STR( None ) )
	PORT_DIPUNUSED( 0x02, 0x00 )                        // Not read due to code at 0x012b
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
INPUT_PORTS_END


static INPUT_PORTS_START( redufob )
	PORT_INCLUDE(galaxian)

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x40, "A 2C/1C  B 1C/3C" )
	PORT_DIPSETTING(    0x00, "A 1C/1C  B 1C/6C" )
	PORT_DIPSETTING(    0x80, "A 1C/2C  B 1C/12C" )
	PORT_DIPSETTING(    0xc0, DEF_STR( Free_Play ) )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x01, "4000" )
	PORT_DIPSETTING(    0x02, "5000" )
	PORT_DIPSETTING(    0x03, "7000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
INPUT_PORTS_END


static INPUT_PORTS_START( azurian )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, galaxian_state, azurian_port_r, (void *)0) /* "linked" with bit 2 of IN2 */
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x80, "5" )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPSETTING(    0x02, "7000" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, galaxian_state, azurian_port_r, (void *)1) /* "linked" with bit 6 of IN1 */
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("FAKE")      /* fake port to handle routine at 0x05b3 that stores value at 0x40f4 */
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
INPUT_PORTS_END


static INPUT_PORTS_START( pisces )
	PORT_INCLUDE(galaxian)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x01, "20000" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x02, "A 2C/1C  B 1C/3C" )
	PORT_DIPSETTING(    0x00, "A 1C/1C  B 1C/6C" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
INPUT_PORTS_END


static INPUT_PORTS_START( piscesb )
	PORT_INCLUDE(pisces)

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x02, "A 2C/1C  B 1C/2C 2C/5C" )
	PORT_DIPSETTING(    0x00, "A 1C/1C  B 1C/5C" )
INPUT_PORTS_END


static INPUT_PORTS_START( gteikokb )
	PORT_INCLUDE(galaxian)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )        /* Not tested due to code removed at 0x00ab, 0x1b26 and 0x1c97*/
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )        /* Not tested due to code removed at 0x1901*/

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "7000" )
	PORT_DIPSETTING(    0x01, DEF_STR( None ) )
	PORT_DIPUNUSED( 0x02, 0x00 )                        // Not read due to code at 0x012b
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
INPUT_PORTS_END


static INPUT_PORTS_START( gteikob2 )
	PORT_INCLUDE(gteikokb)

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static INPUT_PORTS_START( asideral )
	PORT_INCLUDE(galaxian)

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, "1C/2C, 1C/1C" )
	PORT_DIPSETTING(    0x80, "1C/4C, 1C/2C" )
	PORT_DIPSETTING(    0xc0, "Free Play (corrupt text)" )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x01, "4000" )
	PORT_DIPSETTING(    0x02, "5000" )
	PORT_DIPSETTING(    0x03, "7000" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
INPUT_PORTS_END

static INPUT_PORTS_START( spacbatt )
	PORT_INCLUDE(galaxian)

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x40, "A 2C/1C  B 1C/3C" )
	PORT_DIPSETTING(    0x00, "A 1C/1C  B 1C/6C" )
	PORT_DIPSETTING(    0x80, "A 1C/2C  B 1C/6C" )
	PORT_DIPSETTING(    0xc0, DEF_STR( Free_Play ) )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x01, "4000" )
	PORT_DIPSETTING(    0x02, "5000" )
	PORT_DIPSETTING(    0x03, "7000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
INPUT_PORTS_END


static INPUT_PORTS_START( batman2 )
	PORT_INCLUDE(galaxian)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 )

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPSETTING(    0x01, "50000" )
	PORT_DIPUNUSED( 0x02, 0x00 )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
INPUT_PORTS_END


static INPUT_PORTS_START( streakng )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x40, "10000" )
	PORT_DIPSETTING(    0x80, "15000" )
	PORT_DIPSETTING(    0xc0, "20000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( articms )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) /* Likely Bonus Life */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( pacmanbl )
	PORT_INCLUDE(articms)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY

	PORT_MODIFY("IN1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "15000" )
	PORT_DIPSETTING(    0x01, "20000" )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( tenspot )
	PORT_INCLUDE(articms)

	PORT_START("SELECT") /* inputs? read by select CPU - unknown */
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("SELECT2") /* inputs? read by select CPU - unknown */
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("FAKE_SELECT") /* fake button to move onto next game - until select rom is understood! */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Next Game (Fake)") PORT_IMPULSE(1)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY

	PORT_MODIFY("IN1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL


	PORT_MODIFY("IN2") // ignored
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	// yes, the board has 10 banks of dipswitches...
	PORT_START("IN2_GAME0")
	PORT_DIPNAME( 0x01, 0x01, "Survivor DSW0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2_GAME1")
	PORT_DIPNAME( 0x01, 0x01, "Moon Cresta DSW0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("IN2_GAME2")
	PORT_DIPNAME( 0x01, 0x01, "Space Cruiser DSW0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("IN2_GAME3")
	PORT_DIPNAME( 0x01, 0x01, "Mission Rescue DSW0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("IN2_GAME4")
	PORT_DIPNAME( 0x01, 0x01, "Uniwars DSW0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("IN2_GAME5")
	PORT_DIPNAME( 0x01, 0x01, "Batman Pt.2 DSW0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("IN2_GAME6")
	PORT_DIPNAME( 0x01, 0x01, "Defend UFO DSW0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("IN2_GAME7")
	PORT_DIPNAME( 0x01, 0x01, "King and Balloon DSW0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("IN2_GAME8")
	PORT_DIPNAME( 0x01, 0x01, "Omega DSW0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("IN2_GAME9")
	PORT_DIPNAME( 0x01, 0x01, "Battle of Atlantis DSW0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )
INPUT_PORTS_END

static INPUT_PORTS_START( phoenxp2 )
	PORT_INCLUDE(articms)
INPUT_PORTS_END


static INPUT_PORTS_START( atlantib )
	PORT_INCLUDE(articms)

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Coin_A ) ) /* These are backwards compared to the other sets??? */
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) ) /* These are backwards compared to the other sets??? */
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Cabinet ) ) /* These are backwards compared to the other sets??? */
	PORT_DIPSETTING(    0x08, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
INPUT_PORTS_END


static INPUT_PORTS_START( devilfsg )
	PORT_INCLUDE(pacmanbl)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x01, "15000" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x04, "5" )
INPUT_PORTS_END


static INPUT_PORTS_START( zigzag )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Free_Play ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000 60000" )
	PORT_DIPSETTING(    0x04, "20000 60000" )
	PORT_DIPSETTING(    0x08, "30000 60000" )
	PORT_DIPSETTING(    0x0c, "40000 60000" )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( gmgalax )
	PORT_START("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM_MEMBER(DEVICE_SELF, galaxian_state, gmgalax_port_r, "GMIN0\0GLIN0")

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM_MEMBER(DEVICE_SELF, galaxian_state, gmgalax_port_r, "GMIN1\0GLIN1")

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM_MEMBER(DEVICE_SELF, galaxian_state, gmgalax_port_r, "GMIN2\0GLIN2")

	PORT_START("GMIN0")      /* Ghost Muncher - IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )                                  PORT_CONDITION("GAMESEL",0x01,NOTEQUALS,0x01)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_4WAY PORT_COCKTAIL     PORT_CONDITION("GAMESEL",0x01,NOTEQUALS,0x01)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY                PORT_CONDITION("GAMESEL",0x01,NOTEQUALS,0x01)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY               PORT_CONDITION("GAMESEL",0x01,NOTEQUALS,0x01)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )                                 PORT_CONDITION("GAMESEL",0x01,NOTEQUALS,0x01)
	PORT_DIPNAME( 0x20, 0x00, "Ghost Muncher - Cabinet" )                        PORT_CONDITION("GAMESEL",0x01,NOTEQUALS,0x01)
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY                PORT_CONDITION("GAMESEL",0x01,NOTEQUALS,0x01)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY                  PORT_CONDITION("GAMESEL",0x01,NOTEQUALS,0x01)

	PORT_START("GMIN1")      /* Ghost Muncher - IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )                                 PORT_CONDITION("GAMESEL",0x01,NOTEQUALS,0x01)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )                                 PORT_CONDITION("GAMESEL",0x01,NOTEQUALS,0x01)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL  PORT_CONDITION("GAMESEL",0x01,NOTEQUALS,0x01)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL PORT_CONDITION("GAMESEL",0x01,NOTEQUALS,0x01)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )                                 PORT_CONDITION("GAMESEL",0x01,NOTEQUALS,0x01)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL  PORT_CONDITION("GAMESEL",0x01,NOTEQUALS,0x01)
	PORT_DIPNAME( 0xc0, 0x40, "Ghost Muncher - Bonus Life" )                     PORT_CONDITION("GAMESEL",0x01,NOTEQUALS,0x01)
	PORT_DIPSETTING(    0x40, "10000" )
	PORT_DIPSETTING(    0x80, "15000" )
	PORT_DIPSETTING(    0xc0, "20000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )

	PORT_START("GMIN2")      /* Ghost Muncher - IN2 */
	PORT_DIPNAME( 0x03, 0x02, "Ghost Muncher - Coinage" )                        PORT_CONDITION("GAMESEL",0x01,NOTEQUALS,0x01)
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x04, "Ghost Muncher - Lives" )                          PORT_CONDITION("GAMESEL",0x01,NOTEQUALS,0x01)
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("GLIN0")      /* Galaxian - IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )                                  PORT_CONDITION("GAMESEL",0x01,EQUALS,0x01)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )                                  PORT_CONDITION("GAMESEL",0x01,EQUALS,0x01)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY                PORT_CONDITION("GAMESEL",0x01,EQUALS,0x01)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY               PORT_CONDITION("GAMESEL",0x01,EQUALS,0x01)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )                                PORT_CONDITION("GAMESEL",0x01,EQUALS,0x01)
	PORT_DIPNAME( 0x20, 0x00, "Galaxian - Cabinet" )                             PORT_CONDITION("GAMESEL",0x01,EQUALS,0x01)
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_SERVICE( 0x40, IP_ACTIVE_HIGH )                                         PORT_CONDITION("GAMESEL",0x01,EQUALS,0x01)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1 )                               PORT_CONDITION("GAMESEL",0x01,EQUALS,0x01)

	PORT_START("GLIN1")      /* Galaxian - IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )                                 PORT_CONDITION("GAMESEL",0x01,EQUALS,0x01)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )                                 PORT_CONDITION("GAMESEL",0x01,EQUALS,0x01)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL  PORT_CONDITION("GAMESEL",0x01,EQUALS,0x01)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL PORT_CONDITION("GAMESEL",0x01,EQUALS,0x01)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL                  PORT_CONDITION("GAMESEL",0x01,EQUALS,0x01)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )                                 PORT_CONDITION("GAMESEL",0x01,EQUALS,0x01)
	PORT_DIPNAME( 0xc0, 0x00, "Galaxian - Coinage" )                             PORT_CONDITION("GAMESEL",0x01,EQUALS,0x01)
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Free_Play ) )

	PORT_START("GLIN2")      /* Galaxian - IN2 */
	PORT_DIPNAME( 0x03, 0x01, "Galaxian - Bonus Life" )                          PORT_CONDITION("GAMESEL",0x01,EQUALS,0x01)
	PORT_DIPSETTING(    0x01, "4000" )
	PORT_DIPSETTING(    0x02, "5000" )
	PORT_DIPSETTING(    0x03, "7000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x04, 0x00, "Galaxian - Lives" )                               PORT_CONDITION("GAMESEL",0x01,EQUALS,0x01)
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPUNUSED( 0x08, 0x00 )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("GAMESEL")      /* fake - game select */
	PORT_DIPNAME( 0x01, 0x00, "Game Select") PORT_CODE(KEYCODE_F1) PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, galaxian_state, gmgalax_game_changed, NULL)
	PORT_DIPSETTING( 0x00, "Ghost Muncher" )
	PORT_DIPSETTING( 0x01, "Galaxian" )
INPUT_PORTS_END



/*************************************
 *
 *  Input Ports
 *  Moon Cresta-derived games
 *
 *************************************/

static INPUT_PORTS_START( mooncrst )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* "reset" on schematics */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPSETTING(    0x40, "50000" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x80, DEF_STR( English ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Japanese ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Free_Play ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( mooncrsa )
	PORT_INCLUDE(mooncrst)

	PORT_MODIFY("IN2")
	PORT_DIPUNUSED( 0x03, IP_ACTIVE_HIGH )      /* Not used due to code at 0x01c0 */
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Free_Play ) )
INPUT_PORTS_END


static INPUT_PORTS_START( mooncrsg )
	PORT_INCLUDE(mooncrst)

	PORT_MODIFY("IN1")
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_HIGH )      /* Always non-Japanese due to code at 0x2f77 */
INPUT_PORTS_END


static INPUT_PORTS_START( fantazia )
	PORT_INCLUDE(mooncrst)

	PORT_MODIFY("IN1")
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_HIGH )      /* Always non-Japanese due to code at 0x2f53 */

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Free_Play ) )
INPUT_PORTS_END


static INPUT_PORTS_START( eagle2 )
	PORT_INCLUDE(mooncrst)

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Free_Play ) )        /* Not used due to code at 0x01c0, but "Free Play" is checked */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( smooncrs )
	PORT_INCLUDE(mooncrst)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )            /* Not read due to code at 0x2b1c and 0x3313 */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )            /* Not read due to code at 0x2b1c and 0x3313 */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )            /* Not read due to code at 0x2b1c and 0x3313 */
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )       /* code at 0x2962 (0x2f68 in spcdrag) */
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x80, "Player's Bullet Speed" )     /* code at 0x0007 (0x2f53 in spcdrag) */
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x80, "Fast" )
	/* Bonus life is always '50000' due to code at 0x2f68 */
	/* Language is always non-Japanese due to code at 0x2f53 */
INPUT_PORTS_END

static INPUT_PORTS_START( mooncreg )
	PORT_INCLUDE(mooncrst)

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x80, "Player's Bullet Speed" )
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x80, "Fast" )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coinage  ) )
	PORT_DIPSETTING(    0x00, "1C/1C, 1C/2C" )
	PORT_DIPSETTING(    0x01, "2C/1C, 2C/3C" )
	PORT_DIPSETTING(    0x02, "3C/1C, 3C/4C" )
	PORT_DIPSETTING(    0x03, "4C/1C, 4C/5C" )
	PORT_DIPSETTING(    0x04, "1C/1C, 1C/3C" )
	PORT_DIPSETTING(    0x05, "2C/1C, 2C/5C" )
	PORT_DIPSETTING(    0x06, "3C/1C, 3C/7C" )
	PORT_DIPSETTING(    0x07, "4C/1C, 4C/9C" )
	PORT_DIPSETTING(    0x08, "1C/1C, 1C/4C" )
	PORT_DIPSETTING(    0x09, "2C/1C, 2C/7C" )
	PORT_DIPSETTING(    0x0a, "3C/1C, 3C/10C" )
	PORT_DIPSETTING(    0x0b, "4C/1C, 4C/13C" )
	PORT_DIPSETTING(    0x0c, DEF_STR( Free_Play) ) // these all force 4 credits, although the credit inputs will temporarily add different amounts
	PORT_DIPSETTING(    0x0d, "Free Play (duplicate 1)" )
	PORT_DIPSETTING(    0x0e, "Free Play (duplicate 2)" )
	PORT_DIPSETTING(    0x0f, "Free Play (duplicate 3)" )
INPUT_PORTS_END

static INPUT_PORTS_START( mooncrsl )
	PORT_INCLUDE(mooncrst)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) /* the game will crash at round 3 otherwise, could be protection (or a bad rom / bad hack) the same code is mostly patched out in mooncreg */
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x80, "Player's Bullet Speed" )
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x80, "Fast" )
INPUT_PORTS_END


static INPUT_PORTS_START( mooncptc )
	PORT_INCLUDE(smooncrs)

	PORT_MODIFY("IN2") // no 1c/1c ?
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_5C ) )
INPUT_PORTS_END


static INPUT_PORTS_START( mooncrgx )
	PORT_INCLUDE(mooncrst)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x01, "30000" )
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPUNUSED( 0x02, 0x00 )
	PORT_DIPUNUSED( 0x04, 0x00 )        /* Always non-Japanese due to code removed at 0x2f4b */
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
INPUT_PORTS_END


static INPUT_PORTS_START( moonqsr )
	PORT_INCLUDE(mooncrst)

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Hardest ) )
INPUT_PORTS_END


static INPUT_PORTS_START( moonal2 )
	PORT_INCLUDE(galaxian)

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x01, "4000" )
	PORT_DIPSETTING(    0x02, "5000" )
	PORT_DIPSETTING(    0x03, "7000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPUNUSED( 0x08, 0x00 )
INPUT_PORTS_END


static INPUT_PORTS_START( fantastc )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_SERVICE( 0x20, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x40, 0x40, "Extended Bonus Life" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Coinage ) ) // no effect?
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x80, "5" )

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "20000 80000" )
	PORT_DIPSETTING(    0x04, "30000 80000" )
	PORT_DIPSETTING(    0x08, "20000 120000" )
	PORT_DIPSETTING(    0x0c, "30000 120000" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( timefgtr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_DIPNAME( 0x20, 0x00, "255 Lives (Cheat)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Extended Bonus Life" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // tilt? freeze?
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x80, "5" )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) // if 01 and 02 are both set, bonus life is 00
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) // "
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000 50000" )
	PORT_DIPSETTING(    0x04, "20000 50000" )
	PORT_DIPSETTING(    0x08, "10000 60000" )
	PORT_DIPSETTING(    0x0c, "20000 60000" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( kong )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x20, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x40, 0x00, "99 Men/Max Timer (Cheat)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME ("One Player Start/Jump")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME ("Two Player Start/Jump")
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x01, "20000" )
	PORT_DIPSETTING(    0x02, "30000" )
	PORT_DIPSETTING(    0x03, DEF_STR( None ) )
	PORT_DIPNAME( 0x04, 0x00, "Mode" )
	PORT_DIPSETTING(    0x04, "Tournament (harder)" )
	PORT_DIPSETTING(    0x00, "Normal" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( tdpgal )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( skybase )
	PORT_INCLUDE(mooncrst)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x80, "1C/1C (2 to start)" )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Free_Play ) )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "Infinite (Cheat)")
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x04, "15000" )
	PORT_DIPSETTING(    0x08, "20000" )
	PORT_DIPSETTING(    0x0c, DEF_STR( None ) )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
INPUT_PORTS_END


static INPUT_PORTS_START( jumpbug )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_DIPNAME( 0x40, 0x00, "Difficulty ?" )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x00, "Infinite (Cheat)")
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x04, "A 2C/1C  B 2C/1C" )
	PORT_DIPSETTING(    0x08, "A 2C/1C  B 1C/3C" )
	PORT_DIPSETTING(    0x00, "A 1C/1C  B 1C/1C" )
	PORT_DIPSETTING(    0x0c, "A 1C/1C  B 1C/6C" )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( levers )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_DIPUNKNOWN( 0x01, 0x01 )   /* probably unused */
	PORT_DIPUNKNOWN( 0x02, 0x02 )   /* probably unused */
	PORT_DIPNAME( 0x04, 0x00, "Leave On" )  /* used - MUST be ON */
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( checkman )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Tiles Right")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start 1 / P1 Tiles Left")/* also p1 tiles left */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("Start 2 / P1 Tiles Right")/* also p1 tiles right */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Tiles Left")/* p2 tiles left */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x40, "A 2C/1C  B 1C/3C" )
	PORT_DIPSETTING(    0x00, "A 1C/1C  B 1C/6C" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "100000" )
	PORT_DIPSETTING(    0x04, "200000" )
	PORT_DIPNAME( 0x08, 0x00, "Difficulty Increases At Level" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( checkmaj )
	PORT_INCLUDE(checkman)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_COCKTAIL PORT_NAME("P2 Tiles Right")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_COCKTAIL PORT_NAME("P2 Tiles Left")

	PORT_MODIFY("IN2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Tiles Right")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Tiles Left")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( dingo )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL /* 1st Button 1 */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL /* 2nd Button 1 */
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x80, "A 1C/1C  B 1C/5C" )
	PORT_DIPSETTING(    0x00, "A 2C/1C  B 1C/3C" )

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easiest ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPSETTING(    0x08, "40000" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )   /* Yes, the game reads both of these */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )   /* Check code at 0x22e1 */
INPUT_PORTS_END


static INPUT_PORTS_START( mshuttle )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_SERVICE( 0x40, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPSETTING(    0x04, "30000" )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x60, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( kingball )
	PORT_INCLUDE(galaxian)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, galaxian_state, kingball_muxbit_r, NULL)
	/* Relating to above port:Hack? - possibly multiplexed via writes to $b003 */
	//PORT_DIPNAME( 0x40, 0x40, "Speech" )
	//PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	//PORT_DIPSETTING(    0x40, DEF_STR( On ) )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, galaxian_state, kingball_noise_r, NULL)   /* NOISE line */
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x01, "12000" )
	PORT_DIPSETTING(    0x02, "15000" )
	PORT_DIPSETTING(    0x03, DEF_STR( None ) )

	PORT_START("FAKE")
	PORT_SERVICE( 0x01, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x02, 0x02, "Speech" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( thepitm )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) ) // turning both of these on boots with 9 credits?
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
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
INPUT_PORTS_END



/*************************************
 *
 *  Input Ports
 *  Konami games
 *
 *************************************/

static INPUT_PORTS_START( frogger )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* 1P shoot2 - unused */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* 1P shoot1 - unused */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x02, "7" )
	PORT_DIPSETTING(    0x03, "256 (Cheat)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* 2P shoot2 - unused */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* 2P shoot1 - unused */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x02, "A 2/1 B 2/1 C 2/1" )
	PORT_DIPSETTING(    0x04, "A 2/1 B 1/3 C 2/1" )
	PORT_DIPSETTING(    0x00, "A 1/1 B 1/1 C 1/1" )
	PORT_DIPSETTING(    0x06, "A 1/1 B 1/6 C 1/1" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")   /* need for some PPI accesses */
	PORT_BIT( 0xff, 0x00, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( froggermc )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x40, "7" )
	PORT_DIPSETTING(    0x00, "256 (Cheat)")

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x02, "A 2/1 B 2/1 C 2/1" )
	PORT_DIPSETTING(    0x04, "A 2/1 B 1/3 C 2/1" )
	PORT_DIPSETTING(    0x06, "A 1/1 B 1/1 C 1/1" )
	PORT_DIPSETTING(    0x00, "A 1/1 B 1/6 C 1/1" )
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN3")   /* need for some PPI accesses */
	PORT_BIT( 0xff, 0x00, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( frogg )
	PORT_INCLUDE(froggermc)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_SPECIAL )       // See notes
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY

	PORT_MODIFY("IN1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_SPECIAL )       // See notes

	PORT_MODIFY("IN2")
	PORT_DIPUNUSED( 0x02, 0x00 )                        // not tested due to code at 0x3084
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Coinage ) )      // when "Cabinet" Dip Switch set to "Upright"
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )        // "A 1/1 B 1/6" if "Cabinet" Dip Switch set to "Cocktail"
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )        // "A 2/1 B 1/3" if "Cabinet" Dip Switch set to "Cocktail"
	PORT_DIPUNUSED( 0x08, 0x00 )
INPUT_PORTS_END


static INPUT_PORTS_START( turtles )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "126 (Cheat)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, "A 1/1 B 2/1 C 1/1" )
	PORT_DIPSETTING(    0x02, "A 1/2 B 1/1 C 1/2" )
	PORT_DIPSETTING(    0x04, "A 1/3 B 3/1 C 1/3" )
	PORT_DIPSETTING(    0x06, "A 1/4 B 4/1 C 1/4" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")   /* need for some PPI accesses */
	PORT_BIT( 0xff, 0x00, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( turpin )
	PORT_INCLUDE(turtles)

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x02, "7" )
	PORT_DIPSETTING(    0x03, "126 (Cheat)")

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
INPUT_PORTS_END


static INPUT_PORTS_START( amidar )
	PORT_INCLUDE(turtles)

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "255 (Cheat)")

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "30000 50000" )
	PORT_DIPSETTING(    0x04, "50000 50000" )

	PORT_MODIFY("IN3")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, "Invalid" )
	/* Invalid = both coin slots disabled */
INPUT_PORTS_END


static INPUT_PORTS_START( amidaru )
	PORT_INCLUDE(amidar)

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "30000 70000" )
	PORT_DIPSETTING(    0x04, "50000 80000" )
INPUT_PORTS_END


static INPUT_PORTS_START( amidaro )
	PORT_INCLUDE(amidar)

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x02, 0x00, "Level Progression" )
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x02, "Fast" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "30000 70000" )
	PORT_DIPSETTING(    0x04, "50000 80000" )
INPUT_PORTS_END


static INPUT_PORTS_START( amidars )
	PORT_INCLUDE(turtles)

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "256 (Cheat)")

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, "A 1/1 B 1/6" )
	PORT_DIPSETTING(    0x02, "A 2/1 B 1/3" )
	PORT_DIPUNKNOWN( 0x04, 0x00 )
INPUT_PORTS_END


static INPUT_PORTS_START( theend )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "256 (Cheat)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )     /* output bits */

	PORT_START("IN3")   /* need for some PPI accesses */
	PORT_BIT( 0xff, 0x00, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( scramble )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "255 (Cheat)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, "A 1/1  B 2/1  C 1/1" )
	PORT_DIPSETTING(    0x02, "A 1/2  B 1/1  C 1/2" )
	PORT_DIPSETTING(    0x04, "A 1/3  B 3/1  C 1/3" )
	PORT_DIPSETTING(    0x06, "A 1/4  B 4/1  C 1/4" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, galaxian_state, scramble_protection_alt_r, (void *)0)  /* protection bit */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, galaxian_state, scramble_protection_alt_r, (void *)1)  /* protection bit */

	PORT_START("IN3")   /* need for some PPI accesses */
	PORT_BIT( 0xff, 0x00, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( strfbomb )
	PORT_INCLUDE(scramble)

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, "A 1/2  B 4/1  C 1/2" )
	PORT_DIPSETTING(    0x02, "A 1/3  B 2/1  C 1/3" )
	PORT_DIPSETTING(    0x04, "A 1/4  B 3/1  C 1/4" )
	PORT_DIPSETTING(    0x06, "A 1/5  B 1/1  C 1/5" )
INPUT_PORTS_END


static INPUT_PORTS_START( explorer )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPUNKNOWN( 0x01, 0x00 )
	PORT_DIPUNKNOWN( 0x02, 0x00 )
	PORT_DIPUNKNOWN( 0x04, 0x00 )
	PORT_DIPUNKNOWN( 0x08, 0x00 )
	PORT_DIPUNKNOWN( 0x10, 0x00 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* pressing this disables the coins */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_START("IN2")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 2C_6C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 2C_8C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 2C_6C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 2C_8C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ) )

	PORT_START("IN3")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME( 0x1c, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x04, "10000" )
	PORT_DIPSETTING(    0x0c, "15000" )
	PORT_DIPSETTING(    0x14, "20000" )
	PORT_DIPSETTING(    0x1c, "25000" )
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPSETTING(    0x08, "50000" )
	PORT_DIPSETTING(    0x10, "70000" )
	PORT_DIPSETTING(    0x18, "90000" )
	PORT_DIPUNKNOWN( 0x20, 0x00 )
	PORT_DIPUNKNOWN( 0x40, 0x00 )
	PORT_SERVICE( 0x80, IP_ACTIVE_HIGH )
INPUT_PORTS_END


static INPUT_PORTS_START( atlantis )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_DIPNAME( 0x0e, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x02, "A 1/3  B 2/1" )
	PORT_DIPSETTING(    0x00, "A 1/6  B 1/1" )
	PORT_DIPSETTING(    0x04, "A 1/99 B 1/99")
	/* all the other combos give 99 credits */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")   /* need for some PPI accesses */
	PORT_BIT( 0xff, 0x00, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( scorpion )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "3")
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "255" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, "A 1/1  B 1/1" )
	PORT_DIPSETTING(    0x00, "A 1/1  B 1/3" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_DIPNAME( 0xa0, 0xa0, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )

	PORT_START("IN3")   /* need for some PPI accesses */
	PORT_BIT( 0xff, 0x00, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( scorpnmc )
	PORT_START("IN0")      /* 0xa000 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )      // COIN2? (it ALWAYS adds 1 credit)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY

	PORT_START("IN1")      /* 0xa800 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start 1 / P1 Button 1")     /* also P1 Button 1 */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("Start 2 / P1 Button 2")     /* also P1 Button 2 */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Difficulty ) )   // Check code at 0x0118
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Hardest ) )

	PORT_START("IN2")      /* 0xb001 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Lives ) )        // Check code at 0x00eb
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN3")      /* 0xb002 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )      // Check code at 0x00fe
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x06, 0x02, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ) )
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( aracnis )
	PORT_START("IN0")      /* 0xa000 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )      // COIN2? (it ALWAYS adds 1 credit)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_8WAY
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY

	PORT_START("IN1")      /* 0xa800 - needs verifying */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start 1 / P1 Button 1")     /* also P1 Button 1 */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("Start 2 / P1 Button 2")     /* also P1 Button 2 */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )


	PORT_START("IN2")      /* 0xb001 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN3")      /* 0xb002 */
	PORT_DIPNAME( 0x01, 0x01, "0xb002" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( sfx )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) // "Fire" left
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) // "Fire" right
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x03, "Invulnerability (Cheat)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) // "Fire" left
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) // "Fire" right
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* unused */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* unused */

	PORT_START("IN3")   /* need for some PPI accesses */
	PORT_BIT( 0xff, 0x00, IPT_UNUSED )
INPUT_PORTS_END


/* verified from Z80 code */
static INPUT_PORTS_START( scobra )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )                     /* see notes */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("H2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, "4 Times" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("H2:1")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_DIPNAME( 0x06, 0x02, DEF_STR( Coinage ) ) PORT_DIPLOCATION("H2:4,5") /* see notes */
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, "99 Credits" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("H2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")   /* need for some PPI accesses */
	PORT_BIT( 0xff, 0x00, IPT_UNUSED )
INPUT_PORTS_END

/* verified from Z80 code */
static INPUT_PORTS_START( scobras )
	PORT_INCLUDE(scobra)

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("H2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, "255 Times" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("H2:1")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x02, "5" )
INPUT_PORTS_END


CUSTOM_INPUT_MEMBER(galaxian_state::moonwar_dial_r)
{
	static const char *const dialname[2] = { "P1_DIAL", "P2_DIAL" };
	int p = (~m_moonwar_port_select >> 4) & 1;

	// see http://www.cityofberwyn.com/schematics/stern/MoonWar_opto.tiff for schematic
	// I.e. a 74ls161 counts from 0 to 15 which is the absolute number of bars passed on the quadrature

	signed char dialread = ioport(dialname[p])->read();

	UINT8 ret;

	if (dialread < 0) m_direction[p] = 0x00;
	else if (dialread > 0) m_direction[p] = 0x10;

	m_counter_74ls161[p] += abs(dialread);
	m_counter_74ls161[p] &= 0xf;

	ret = m_counter_74ls161[p] | m_direction[p];
	//fprintf(stderr, "dialread1: %02x, counter_74ls161: %02x, spinner ret is %02x\n", dialread, m_counter_74ls161[p], ret);

	return ret;
}

/* verified from Z80 code */
static INPUT_PORTS_START( moonwar )
	PORT_START("IN0")
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, galaxian_state, moonwar_dial_r, (void *)0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL // cocktail: p2 shield
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, DEF_STR( Free_Play ) )                  /* see notes */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_CONDITION("IN2", 0x08, EQUALS, 0x08) // cocktail: p2 thrust
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CONDITION("IN2", 0x08, EQUALS, 0x00) // upright: p1&p2 hyperflip
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) // both: p1(upright: &p2) shield
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) // both: p1(upright: &p2) thrust
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) // both: p1(upright: &p2) fire

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL // cocktail: p2 fire
	PORT_DIPNAME( 0x06, 0x02, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x02, "A 1/1  B 1/1" )
	PORT_DIPSETTING(    0x00, "A 1/2  B 2/1" )
	PORT_DIPSETTING(    0x04, "A 1/3  B 3/1" )
	PORT_DIPSETTING(    0x06, "A 1/4  B 4/1" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )                       /* output bits */

	PORT_START("IN3")   /* need for some PPI accesses */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1_DIAL")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(4) PORT_REVERSE PORT_RESET PORT_CONDITION("IN2", 0x08, EQUALS, 0x08) // cocktail: dial is reversed
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(4) PORT_RESET PORT_CONDITION("IN2", 0x08, EQUALS, 0x00) // upright: dial works normally

	PORT_START("P2_DIAL")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(4) PORT_RESET PORT_COCKTAIL PORT_REVERSE // cocktail: dial is reversed
INPUT_PORTS_END

/* verified from Z80 code */
static INPUT_PORTS_START( moonwara )
	PORT_INCLUDE(moonwar)

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, "A 1/1  B 4/1" )
	PORT_DIPSETTING(    0x02, "A 1/2  B 3/1" )
	PORT_DIPSETTING(    0x04, "A 1/3  B 1/1" )
	PORT_DIPSETTING(    0x06, "A 1/4  B 2/1" )

	PORT_MODIFY("P1_DIAL")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(4) PORT_RESET // both: p1 dial works normally, p2 dial is reversed, both share same port

	PORT_MODIFY("P2_DIAL")       /* doesn't actually work due to bug in game code */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(4) PORT_RESET PORT_COCKTAIL
INPUT_PORTS_END


/* verified from Z80 code */
static INPUT_PORTS_START( armorcar )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_4WAY PORT_COCKTAIL
	PORT_DIPNAME( 0x06, 0x02, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x02, "A 1/1  B 1/1" )
	PORT_DIPSETTING(    0x00, "A 1/2  B 2/1" )
	PORT_DIPSETTING(    0x04, "A 1/3  B 3/1" )
	PORT_DIPSETTING(    0x06, "A 1/4  B 4/1" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")   /* need for some PPI accesses */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/* verified from Z80 code */
static INPUT_PORTS_START( armorcar2 )
	PORT_INCLUDE(armorcar)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )                       /* see notes */

	PORT_MODIFY("IN1")
	PORT_BIT( 0x3c, IP_ACTIVE_LOW, IPT_UNUSED )                       /* see notes */

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )                       /* see notes */
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )                             /* see notes */
INPUT_PORTS_END


/* verified from Z80 code */
static INPUT_PORTS_START( tazmania )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x06, 0x02, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x02, "A 1/1  B 1/1" )
	PORT_DIPSETTING(    0x00, "A 1/2  B 2/1" )
	PORT_DIPSETTING(    0x04, "A 1/3  B 3/1" )
	PORT_DIPSETTING(    0x06, "A 1/4  B 4/1" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")   /* need for some PPI accesses */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/* verified from Z80 code */
static INPUT_PORTS_START( anteater )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
//  PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_4WAY PORT_COCKTAIL
//  PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_4WAY PORT_COCKTAIL
//  PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
//  PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_4WAY PORT_COCKTAIL
//  PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x7c, IP_ACTIVE_LOW, IPT_UNUSED )                       /* see notes */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x06, 0x02, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x02, "A 1/1  B 1/1" )
	PORT_DIPSETTING(    0x00, "A 1/2  B 2/1" )
	PORT_DIPSETTING(    0x04, "A 1/3  B 3/1" )
	PORT_DIPSETTING(    0x06, "A 1/4  B 4/1" )
//  PORT_DIPNAME( 0x08, 0x08, DEF_STR( Cabinet ) )
//  PORT_DIPSETTING(    0x08, DEF_STR( Upright ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )                             /* see notes */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xb0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")   /* need for some PPI accesses */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( anteateruk )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x02, "A 1/1  B 1/5" )
	PORT_DIPSETTING(    0x00, "A 2/1  B 1/3" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN3")   /* need for some PPI accesses */
	PORT_BIT( 0xff, 0x00, IPT_UNUSED )
INPUT_PORTS_END


/* verified from Z80 code */
static INPUT_PORTS_START( calipso )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)      /* also START2 - see notes */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )  PORT_PLAYER(1)      /* also START1 - see notes */
	PORT_DIPNAME( 0x06, 0x02, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x02, "A 1/1  B 1/1" )
	PORT_DIPSETTING(    0x00, "A 1/2  B 2/1" )
	PORT_DIPSETTING(    0x04, "A 1/3  B 3/1" )
	PORT_DIPSETTING(    0x06, "A 1/4  B 4/1" )
//  PORT_DIPNAME( 0x08, 0x08, DEF_STR( Cabinet ) )
//  PORT_DIPSETTING(    0x08, DEF_STR( Upright ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )                             /* see notes */
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")   /* need for some PPI accesses */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/* verified from Z80 code */
static INPUT_PORTS_START( losttomb )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP )     PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN )   PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT )  PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT )   PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) ) PORT_DIPLOCATION("H2:1,2")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, DEF_STR( Free_Play ) )                  /* see notes */
	PORT_DIPSETTING(    0x00, "Invulnerability (Cheat)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP )    PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN )  PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT )  PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )                      /* "WHIP" */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x06, 0x02, DEF_STR( Coinage ) ) PORT_DIPLOCATION("H2:4,5")
	PORT_DIPSETTING(    0x02, "A 1/1  B 1/1" )
	PORT_DIPSETTING(    0x00, "A 1/2  B 2/1" )
	PORT_DIPSETTING(    0x04, "A 1/3  B 3/1" )
	PORT_DIPSETTING(    0x06, "A 1/4  B 4/1" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("H2:3") /* see notes */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")   /* need for some PPI accesses */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/* verified from Z80 code - IN2 bit 1 still needs to be understood */
static INPUT_PORTS_START( spdcoin )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x00, "Freeze" )                              /* Dip Sw #2 */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Free_Play ) )                  /* Dip Sw #1 */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )                    /* Dip Sw #5 - Check code at 0x0569 */
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Difficulty ) )                 /* Dip Sw #4 */
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Lives ) )                      /* Dip Sw #3 */
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")   /* need for some PPI accesses */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/* verified from Z80 code */
static INPUT_PORTS_START( superbon )
	PORT_INCLUDE(losttomb)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY

	PORT_MODIFY("IN1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )                      /* "HOLD" */
	PORT_BIT( 0x34, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout galaxian_charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout galaxian_spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ STEP8(0,1), STEP8(8*8,1) },
	{ STEP8(0,8), STEP8(16*8,8) },
	16*16
};

static const gfx_layout galaxian_charlayout_0x200 =
{
	8,8,
	0x200,
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout galaxian_spritelayout_0x80 =
{
	16,16,
	0x80,
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ STEP8(0,1), STEP8(8*8,1) },
	{ STEP8(0,8), STEP8(16*8,8) },
	16*16
};

/*************************************
 *
 *  Graphics decoding
 *
 *************************************/

static GFXDECODE_START(galaxian)
	GFXDECODE_SCALE("gfx1", 0x0000, galaxian_charlayout,   0, 8, GALAXIAN_XSCALE,1)
	GFXDECODE_SCALE("gfx1", 0x0000, galaxian_spritelayout, 0, 8, GALAXIAN_XSCALE,1)
GFXDECODE_END

static GFXDECODE_START(gmgalax)
	GFXDECODE_SCALE("gfx1", 0x0000, galaxian_charlayout,   0, 16, GALAXIAN_XSCALE,1)
	GFXDECODE_SCALE("gfx1", 0x0000, galaxian_spritelayout, 0, 16, GALAXIAN_XSCALE,1)
GFXDECODE_END

/* separate character and sprite ROMs */
static GFXDECODE_START(pacmanbl)
	GFXDECODE_SCALE("gfx1", 0x0000, galaxian_charlayout,   0, 8, GALAXIAN_XSCALE,1)
	GFXDECODE_SCALE("gfx2", 0x0000, galaxian_spritelayout, 0, 8, GALAXIAN_XSCALE,1)
GFXDECODE_END

static GFXDECODE_START(tenspot)
	GFXDECODE_SCALE("gfx1", 0x0000, galaxian_charlayout_0x200,   0, 8, GALAXIAN_XSCALE,1)
	GFXDECODE_SCALE("gfx2", 0x0000, galaxian_spritelayout_0x80, 0, 8, GALAXIAN_XSCALE,1)
GFXDECODE_END


/*************************************
 *
 *  Sound configuration
 *
 *************************************/

static const discrete_mixer_desc konami_sound_mixer_desc =
	{DISC_MIXER_IS_OP_AMP,
		{RES_K(5.1), RES_K(5.1), RES_K(5.1), RES_K(5.1), RES_K(5.1), RES_K(5.1)},
		{0,0,0,0,0,0},  /* no variable resistors   */
		{0,0,0,0,0,0},  /* no node capacitors      */
		0, RES_K(2.2),
		0,
		0, /* modelled separately */
		0, 1};

static DISCRETE_SOUND_START( konami_sound )

	DISCRETE_INPUTX_STREAM(NODE_01, 0, 1.0, 0)
	DISCRETE_INPUTX_STREAM(NODE_02, 1, 1.0, 0)
	DISCRETE_INPUTX_STREAM(NODE_03, 2, 1.0, 0)

	DISCRETE_INPUTX_STREAM(NODE_04, 3, 1.0, 0)
	DISCRETE_INPUTX_STREAM(NODE_05, 4, 1.0, 0)
	DISCRETE_INPUTX_STREAM(NODE_06, 5, 1.0, 0)

	DISCRETE_INPUT_DATA(NODE_11)
	DISCRETE_INPUT_DATA(NODE_12)
	DISCRETE_INPUT_DATA(NODE_13)

	DISCRETE_INPUT_DATA(NODE_14)
	DISCRETE_INPUT_DATA(NODE_15)
	DISCRETE_INPUT_DATA(NODE_16)

	DISCRETE_RCFILTER_SW(NODE_21, 1, NODE_01, NODE_11, AY8910_INTERNAL_RESISTANCE+1000, CAP_U(0.22), CAP_U(0.047), 0, 0)
	DISCRETE_RCFILTER_SW(NODE_22, 1, NODE_02, NODE_12, AY8910_INTERNAL_RESISTANCE+1000, CAP_U(0.22), CAP_U(0.047), 0, 0)
	DISCRETE_RCFILTER_SW(NODE_23, 1, NODE_03, NODE_13, AY8910_INTERNAL_RESISTANCE+1000, CAP_U(0.22), CAP_U(0.047), 0, 0)

	DISCRETE_RCFILTER_SW(NODE_24, 1, NODE_04, NODE_14, AY8910_INTERNAL_RESISTANCE+1000, CAP_U(0.22), CAP_U(0.047), 0, 0)
	DISCRETE_RCFILTER_SW(NODE_25, 1, NODE_05, NODE_15, AY8910_INTERNAL_RESISTANCE+1000, CAP_U(0.22), CAP_U(0.047), 0, 0)
	DISCRETE_RCFILTER_SW(NODE_26, 1, NODE_06, NODE_16, AY8910_INTERNAL_RESISTANCE+1000, CAP_U(0.22), CAP_U(0.047), 0, 0)

	DISCRETE_MIXER6(NODE_30, 1, NODE_21, NODE_22, NODE_23, NODE_24, NODE_25, NODE_26, &konami_sound_mixer_desc)

	/* FIXME the amplifier M51516L has a decay circuit */
	/* This is handled with sound_global_enable but    */
	/* belongs here.                                   */

	/* Input impedance of a M51516L is typically 30k (datasheet) */
	DISCRETE_CRFILTER(NODE_40,NODE_30,RES_K(30),CAP_U(0.15))

	DISCRETE_OUTPUT(NODE_40, 10.0 )

DISCRETE_SOUND_END



/*************************************
 *
 *  Core machine driver pieces
 *
 *************************************/

static MACHINE_CONFIG_START( galaxian_base, galaxian_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, GALAXIAN_PIXEL_CLOCK/3/2)
	MCFG_CPU_PROGRAM_MAP(galaxian_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", galaxian_state,  interrupt_gen)

	MCFG_WATCHDOG_VBLANK_INIT(8)

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", galaxian)
	MCFG_PALETTE_ADD("palette", 32)
	MCFG_PALETTE_INIT_OWNER(galaxian_state, galaxian)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(GALAXIAN_PIXEL_CLOCK, GALAXIAN_HTOTAL, GALAXIAN_HBEND, GALAXIAN_HBSTART, GALAXIAN_VTOTAL, GALAXIAN_VBEND, GALAXIAN_VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(galaxian_state, screen_update_galaxian)


	/* blinking frequency is determined by 555 counter with Ra=100k, Rb=10k, C=10uF */
	MCFG_TIMER_DRIVER_ADD_PERIODIC("stars", galaxian_state, galaxian_stars_blink_timer, PERIOD_OF_555_ASTABLE(100000, 10000, 0.00001))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
MACHINE_CONFIG_END



static MACHINE_CONFIG_DERIVED( konami_base, galaxian_base )

	MCFG_DEVICE_ADD("ppi8255_0", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("IN0"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("IN1"))
	MCFG_I8255_IN_PORTC_CB(IOPORT("IN2"))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(galaxian_state, konami_portc_0_w))

	MCFG_DEVICE_ADD("ppi8255_1", I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(driver_device, soundlatch_byte_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(galaxian_state, konami_sound_control_w))
	MCFG_I8255_IN_PORTC_CB(IOPORT("IN3"))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(galaxian_state, konami_portc_1_w))
MACHINE_CONFIG_END


static MACHINE_CONFIG_FRAGMENT( konami_sound_1x_ay8910 )

	/* 2nd CPU to drive sound */
	MCFG_CPU_ADD("audiocpu", Z80, KONAMI_SOUND_CLOCK/8)
	MCFG_CPU_PROGRAM_MAP(frogger_sound_map)
	MCFG_CPU_IO_MAP(frogger_sound_portmap)

	/* sound hardware */
	MCFG_SOUND_ADD("8910.0", AY8910, KONAMI_SOUND_CLOCK/8)
	MCFG_AY8910_OUTPUT_TYPE(AY8910_DISCRETE_OUTPUT)
	MCFG_AY8910_RES_LOADS(RES_K(5.1), RES_K(5.1), RES_K(5.1))
	MCFG_AY8910_PORT_A_READ_CB(READ8(driver_device, soundlatch_byte_r))
	MCFG_AY8910_PORT_B_READ_CB(READ8(galaxian_state, frogger_sound_timer_r))
	MCFG_SOUND_ROUTE_EX(0, "konami", 1.0, 0)
	MCFG_SOUND_ROUTE_EX(1, "konami", 1.0, 1)
	MCFG_SOUND_ROUTE_EX(2, "konami", 1.0, 2)

	MCFG_SOUND_ADD("konami", DISCRETE, 0)
	MCFG_DISCRETE_INTF(konami_sound)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
MACHINE_CONFIG_END


static MACHINE_CONFIG_FRAGMENT( konami_sound_2x_ay8910 )

	/* 2nd CPU to drive sound */
	MCFG_CPU_ADD("audiocpu", Z80, KONAMI_SOUND_CLOCK/8)
	MCFG_CPU_PROGRAM_MAP(konami_sound_map)
	MCFG_CPU_IO_MAP(konami_sound_portmap)

	/* sound hardware */
	MCFG_SOUND_ADD("8910.0", AY8910, KONAMI_SOUND_CLOCK/8)
	MCFG_AY8910_OUTPUT_TYPE(AY8910_DISCRETE_OUTPUT)
	MCFG_AY8910_RES_LOADS(RES_K(5.1), RES_K(5.1), RES_K(5.1))
	MCFG_AY8910_PORT_A_READ_CB(READ8(driver_device, soundlatch_byte_r))
	MCFG_AY8910_PORT_B_READ_CB(READ8(galaxian_state, konami_sound_timer_r))
	MCFG_SOUND_ROUTE_EX(0, "konami", 1.0, 0)
	MCFG_SOUND_ROUTE_EX(1, "konami", 1.0, 1)
	MCFG_SOUND_ROUTE_EX(2, "konami", 1.0, 2)

	MCFG_SOUND_ADD("8910.1", AY8910, KONAMI_SOUND_CLOCK/8)
	MCFG_AY8910_OUTPUT_TYPE(AY8910_DISCRETE_OUTPUT)
	MCFG_AY8910_RES_LOADS(RES_K(5.1), RES_K(5.1), RES_K(5.1))
	MCFG_SOUND_ROUTE_EX(0, "konami", 1.0, 3)
	MCFG_SOUND_ROUTE_EX(1, "konami", 1.0, 4)
	MCFG_SOUND_ROUTE_EX(2, "konami", 1.0, 5)

	MCFG_SOUND_ADD("konami", DISCRETE, 0)
	MCFG_DISCRETE_INTF(konami_sound)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_DERIVED( galaxian, galaxian_base )
	MCFG_FRAGMENT_ADD(galaxian_audio)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( spactrai, galaxian )

	/* strange memory map, maybe a kind of protection */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(spactrai_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( pacmanbl, galaxian )

	/* separate tile/sprite ROMs */
	MCFG_GFXDECODE_MODIFY("gfxdecode", pacmanbl)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( tenspot, galaxian )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_VBLANK_INT_DRIVER("screen", galaxian_state,  fakechange_interrupt_gen)

	/* basic machine hardware */
	MCFG_CPU_ADD("selectcpu", Z80, GALAXIAN_PIXEL_CLOCK/3/2) // ?? mhz
	MCFG_CPU_PROGRAM_MAP(tenspot_select_map)
	//MCFG_CPU_VBLANK_INT("screen", nmi_line_pulse)

	/* separate tile/sprite ROMs */
	MCFG_GFXDECODE_MODIFY("gfxdecode", tenspot)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( zigzag, galaxian_base )

	/* separate tile/sprite ROMs */
	MCFG_GFXDECODE_MODIFY("gfxdecode", pacmanbl)

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(galaxian_map_base)  /* no discrete sound */

	/* sound hardware */
	MCFG_SOUND_ADD("8910.0", AY8910, GALAXIAN_PIXEL_CLOCK/3/2) /* matches PCB video - unconfirmed */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( gmgalax, galaxian )

	/* banked video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", gmgalax)
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(64)
	MCFG_PALETTE_INIT_OWNER(galaxian_state, galaxian)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( mooncrst, galaxian_base )

	/* alternate memory map */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(mooncrst_map)

	MCFG_FRAGMENT_ADD(mooncrst_audio)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( moonqsr, mooncrst )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_DECRYPTED_OPCODES_MAP(moonqsr_decrypted_opcodes_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( fantastc, galaxian_base )

	/* alternate memory map */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(fantastc_map)

	/* sound hardware */
	MCFG_SOUND_ADD("8910.0", AY8910, GALAXIAN_PIXEL_CLOCK/3/2) // 3.072MHz
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("8910.1", AY8910, GALAXIAN_PIXEL_CLOCK/3/2) // 3.072MHz
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END


TIMER_DEVICE_CALLBACK_MEMBER(galaxian_state::timefgtr_scanline)
{
	UINT8 split = param + 16;

	// change spriteram base per each 64-line part of the screen
	if ((split & 0x3f) == 0)
	{
//      m_screen->update_now();
		m_screen->update_partial(m_screen->vpos());
		m_sprites_base = 0x40 | (split << 2 & 0x300);
	}
}

static MACHINE_CONFIG_DERIVED( timefgtr, galaxian_base )

	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", galaxian_state, timefgtr_scanline, "screen", 0, 1)

	/* alternate memory map */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(timefgtr_map)

	/* sound hardware */
	MCFG_SOUND_ADD("8910.0", AY8910, GALAXIAN_PIXEL_CLOCK/3/2) // 3.072MHz
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("8910.1", AY8910, GALAXIAN_PIXEL_CLOCK/3/2) // 3.072MHz
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( jumpbug, galaxian_base )

	MCFG_WATCHDOG_VBLANK_INIT(0)

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(jumpbug_map)

	/* sound hardware */
	MCFG_SOUND_ADD("8910.0", AY8910, GALAXIAN_PIXEL_CLOCK/3/2/2)    /* matches PCB video - unconfirmed */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( checkman, mooncrst )

	/* basic machine hardware */
	MCFG_CPU_ADD("audiocpu", Z80, 1620000)  /* 1.62 MHz */
	MCFG_CPU_PROGRAM_MAP(checkman_sound_map)
	MCFG_CPU_IO_MAP(checkman_sound_portmap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", galaxian_state,  irq0_line_hold)   /* NMIs are triggered by the main CPU */

	/* sound hardware */
	MCFG_SOUND_ADD("8910.0", AY8910, 1789750)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( checkmaj, galaxian_base )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(galaxian_map_base)  /* no discrete sound */

	/* basic machine hardware */
	MCFG_CPU_ADD("audiocpu", Z80, 1620000)
	MCFG_CPU_PROGRAM_MAP(checkmaj_sound_map)

	MCFG_TIMER_DRIVER_ADD_SCANLINE("irq0", galaxian_state, checkmaj_irq0_gen, "screen", 0, 8)

	/* sound hardware */
	MCFG_SOUND_ADD("8910.0", AY8910, 1620000)
	MCFG_AY8910_PORT_A_READ_CB(READ8(driver_device, soundlatch_byte_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 2)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( mshuttle, galaxian_base )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(mshuttle_map)
	MCFG_CPU_DECRYPTED_OPCODES_MAP(mshuttle_decrypted_opcodes_map)
	MCFG_CPU_IO_MAP(mshuttle_portmap)

	/* sound hardware */
	MCFG_CCLIMBER_AUDIO_ADD("cclimber_audio")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( kingball, mooncrst )

	/* basic machine hardware */
	MCFG_CPU_ADD("audiocpu", Z80,5000000/2)
	MCFG_CPU_PROGRAM_MAP(kingball_sound_map)
	MCFG_CPU_IO_MAP(kingball_sound_portmap)

	/* sound hardware */
	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( frogger, konami_base )
	MCFG_FRAGMENT_ADD(konami_sound_1x_ay8910)

	/* alternate memory map */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(frogger_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( froggermc, galaxian_base )
	MCFG_FRAGMENT_ADD(konami_sound_1x_ay8910)

	/* alternate memory map */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(mooncrst_map_base)     /* no discrete sound ! */

	MCFG_CPU_MODIFY("audiocpu")
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE(DEVICE_SELF, galaxian_state, froggermc_audiocpu_irq_ack)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( froggers, konami_base )
	MCFG_FRAGMENT_ADD(konami_sound_1x_ay8910)

	/* alternate memory map */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(theend_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( frogf, konami_base )
	MCFG_FRAGMENT_ADD(konami_sound_1x_ay8910)

	/* alternate memory map */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(frogf_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( turtles, konami_base )
	MCFG_FRAGMENT_ADD(konami_sound_2x_ay8910)

	/* alternate memory map */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(turtles_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( theend, galaxian_base )
	MCFG_FRAGMENT_ADD(konami_sound_2x_ay8910)

	/* alternate memory map */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(theend_map)

	MCFG_DEVICE_ADD("ppi8255_0", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("IN0"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("IN1"))
	MCFG_I8255_IN_PORTC_CB(IOPORT("IN2"))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(galaxian_state, theend_coin_counter_w))

	MCFG_DEVICE_ADD("ppi8255_1", I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(driver_device, soundlatch_byte_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(galaxian_state, konami_sound_control_w))
	MCFG_I8255_IN_PORTC_CB(IOPORT("IN3"))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(galaxian_state, konami_portc_1_w))
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( scramble, galaxian_base )
	MCFG_FRAGMENT_ADD(konami_sound_2x_ay8910)

	/* alternate memory map */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(theend_map)

	MCFG_DEVICE_ADD("ppi8255_0", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("IN0"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("IN1"))
	MCFG_I8255_IN_PORTC_CB(IOPORT("IN2"))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(galaxian_state, konami_portc_0_w))

	MCFG_DEVICE_ADD("ppi8255_1", I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(driver_device, soundlatch_byte_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(galaxian_state, konami_sound_control_w))
	MCFG_I8255_IN_PORTC_CB(READ8(galaxian_state, scramble_protection_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(galaxian_state, scramble_protection_w))
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( explorer, galaxian_base )

	/* alternate memory map */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(theend_map)

	/* 2nd CPU to drive sound */
	MCFG_CPU_ADD("audiocpu", Z80,KONAMI_SOUND_CLOCK/8)
	MCFG_CPU_PROGRAM_MAP(konami_sound_map)
	MCFG_CPU_IO_MAP(konami_sound_portmap)

	/* sound hardware */
	MCFG_SOUND_ADD("8910.0", AY8910, KONAMI_SOUND_CLOCK/8)
	MCFG_AY8910_PORT_A_READ_CB(READ8(galaxian_state, explorer_sound_latch_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("8910.1", AY8910, KONAMI_SOUND_CLOCK/8)
	MCFG_AY8910_PORT_A_READ_CB(READ8(galaxian_state, konami_sound_timer_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( scorpion, theend )

	MCFG_DEVICE_REMOVE("ppi8255_0")
	MCFG_DEVICE_REMOVE("ppi8255_1")

	MCFG_DEVICE_ADD("ppi8255_0", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("IN0"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("IN1"))
	MCFG_I8255_IN_PORTC_CB(IOPORT("IN2"))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(galaxian_state, konami_portc_0_w))

	MCFG_DEVICE_ADD("ppi8255_1", I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(driver_device, soundlatch_byte_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(galaxian_state, konami_sound_control_w))
	MCFG_I8255_IN_PORTC_CB(READ8(galaxian_state, scorpion_protection_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(galaxian_state, scorpion_protection_w))

	/* extra AY8910 with I/O ports */
	MCFG_SOUND_ADD("8910.2", AY8910, KONAMI_SOUND_CLOCK/8)
	MCFG_AY8910_PORT_A_WRITE_CB(DEVWRITE8("digitalker", digitalker_device, digitalker_data_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(galaxian_state, scorpion_digitalker_control_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_DIGITALKER_ADD("digitalker", 4000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.16)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( sfx, galaxian_base )
	MCFG_FRAGMENT_ADD(konami_sound_2x_ay8910)

	MCFG_WATCHDOG_VBLANK_INIT(0)

	/* alternate memory map */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sfx_map)

	/* 3rd CPU for the sample player */
	MCFG_CPU_ADD("audio2", Z80, KONAMI_SOUND_CLOCK/8)
	MCFG_CPU_PROGRAM_MAP(sfx_sample_map)
	MCFG_CPU_IO_MAP(sfx_sample_portmap)

	MCFG_DEVICE_ADD("ppi8255_0", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("IN0"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("IN1"))
	MCFG_I8255_IN_PORTC_CB(IOPORT("IN2"))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(galaxian_state, konami_portc_0_w))

	MCFG_DEVICE_ADD("ppi8255_1", I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(driver_device, soundlatch_byte_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(galaxian_state, konami_sound_control_w))
	MCFG_I8255_IN_PORTC_CB(IOPORT("IN3"))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(galaxian_state, konami_portc_1_w))

	MCFG_DEVICE_ADD("ppi8255_2", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(driver_device, soundlatch2_byte_r))

	/* port on 2nd 8910 is used for communication */
	MCFG_SOUND_MODIFY("8910.1")
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(driver_device, soundlatch2_byte_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(galaxian_state, sfx_sample_control_w))

	/* DAC for the sample player */
	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( monsterz, sfx )

	/* alternate memory map */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(monsterz_map)

	MCFG_DEVICE_REMOVE("ppi8255_1")
	MCFG_DEVICE_ADD("ppi8255_1", I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(galaxian_state, monsterz_porta_1_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(galaxian_state, monsterz_portb_1_w))
	MCFG_I8255_IN_PORTC_CB(IOPORT("IN3"))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(galaxian_state, monsterz_portc_1_w))

	/* there are likely other differences too, but those can wait until after protection is sorted out */

MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( scobra, konami_base )
	MCFG_FRAGMENT_ADD(konami_sound_2x_ay8910)

	/* alternate memory map */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(scobra_map)
MACHINE_CONFIG_END

/*

Quaak (Frogger bootleg)
=======================

Dumper: Martin Ponweiser / m.ponweiser@gmail.com

Top Board (Sound)
-----------------

Silkscreened label: "09041"

18-Pin Connector, Frogger Pinout (https://www.mikesarcade.com/cgi-bin/spies.cgi?action=url&type=pinout&page=Frogger.txt)
Note the difference to the Sega Pinout: B18 is 12V, NOT -5V.

1 x 6-DIP Switch (https://www.mikesarcade.com/cgi-bin/spies.cgi?action=url&type=dip&page=Frogger.txt)

1 x Z8400, Z80 CPU
1 x XTAL 14.31818 Mhz
2 x P8255A Intel
2 x AY-3-8910
3 x D2716-6 EPROMs labelled: "A", "B", "C"
2 x MM2114N-15L
1 x LM377N


Bottom Board (Main)
-------------------

Silkscreened label: "10041"

1 x Z8400, Z80 CPU
8 x MM2114N-15L
6 x D2716-6 ("1F"..."6F"), 2 empty sockets
2 x D2716-6 ("7H","8H")
1 x N82S123N 7920 (socketed, yet undumped)
1 x XTAL 18.4320 Mhz
1 x 555
5 x D2115A


2014-05-08: ROMS dumped with EETools MegaMax

(note 2x AY even if one is unused by the game, board was probably made for Super Cobra?)

*/


static MACHINE_CONFIG_DERIVED( quaak, konami_base )
	MCFG_FRAGMENT_ADD(konami_sound_2x_ay8910)

	MCFG_DEVICE_MODIFY("8910.0")
	MCFG_AY8910_PORT_B_READ_CB(READ8(galaxian_state, frogger_sound_timer_r))

	/* alternate memory map */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(scobra_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( froggeram, konami_base )
	MCFG_FRAGMENT_ADD(konami_sound_2x_ay8910)

	MCFG_DEVICE_MODIFY("8910.0")
	MCFG_AY8910_PORT_B_READ_CB(READ8(galaxian_state, frogger_sound_timer_r))

	/* alternate memory map */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(froggeram_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( anteater, scobra )

	/* quiet down the sounds */
	MCFG_SOUND_MODIFY("konami")
	MCFG_SOUND_ROUTES_RESET()
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( anteateruk, anteater )

	/* strange memory map, maybe a kind of protection */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(anteateruk_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( anteaterg, anteater )

	/* strange memory map, maybe a kind of protection */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(anteaterg_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( moonwar, scobra )

	MCFG_DEVICE_REMOVE("ppi8255_0")
	MCFG_DEVICE_REMOVE("ppi8255_1")

	MCFG_DEVICE_ADD("ppi8255_0", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("IN0"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("IN1"))
	MCFG_I8255_IN_PORTC_CB(IOPORT("IN2"))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(galaxian_state, moonwar_port_select_w))

	MCFG_DEVICE_ADD("ppi8255_1", I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(driver_device, soundlatch_byte_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(galaxian_state, konami_sound_control_w))
	MCFG_I8255_IN_PORTC_CB(IOPORT("IN3"))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(galaxian_state, konami_portc_1_w))

	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_INIT_OWNER(galaxian_state,moonwar) // bullets are less yellow
MACHINE_CONFIG_END


/*************************************
 *
 *  Decryption helpers
 *
 *************************************/

void galaxian_state::decode_mooncrst(int length, UINT8 *dest)
{
	UINT8 *rom = memregion("maincpu")->base();
	int offs;

	for (offs = 0; offs < length; offs++)
	{
		UINT8 data = rom[offs];
		UINT8 res = data;
		if (BIT(data,1)) res ^= 0x40;
		if (BIT(data,5)) res ^= 0x04;
		if ((offs & 1) == 0) res = BITSWAP8(res,7,2,5,4,3,6,1,0);
		dest[offs] = res;
	}
}


void galaxian_state::decode_checkman()
{
	/*
	                         Encryption Table
	                         ----------------
	    +---+---+---+------+------+------+------+------+------+------+------+
	    |A2 |A1 |A0 |D7    |D6    |D5    |D4    |D3    |D2    |D1    |D0    |
	    +---+---+---+------+------+------+------+------+------+------+------+
	    | 0 | 0 | 0 |D7    |D6    |D5    |D4    |D3    |D2    |D1    |D0^^D6|
	    | 0 | 0 | 1 |D7    |D6    |D5    |D4    |D3    |D2    |D1^^D5|D0    |
	    | 0 | 1 | 0 |D7    |D6    |D5    |D4    |D3    |D2^^D4|D1^^D6|D0    |
	    | 0 | 1 | 1 |D7    |D6    |D5    |D4^^D2|D3    |D2    |D1    |D0^^D5|
	    | 1 | 0 | 0 |D7    |D6^^D4|D5^^D1|D4    |D3    |D2    |D1    |D0    |
	    | 1 | 0 | 1 |D7    |D6^^D0|D5^^D2|D4    |D3    |D2    |D1    |D0    |
	    | 1 | 1 | 0 |D7    |D6    |D5    |D4    |D3    |D2^^D0|D1    |D0    |
	    | 1 | 1 | 1 |D7    |D6    |D5    |D4^^D1|D3    |D2    |D1    |D0    |
	    +---+---+---+------+------+------+------+------+------+------+------+

	    For example if A2=1, A1=1 and A0=0 then D2 to the CPU would be an XOR of
	    D2 and D0 from the ROM's. Note that D7 and D3 are not encrypted.

	    Encryption PAL 16L8 on cardridge
	             +--- ---+
	        OE --|   U   |-- VCC
	     ROMD0 --|       |-- D0
	     ROMD1 --|       |-- D1
	     ROMD2 --|VER 5.2|-- D2
	        A0 --|       |-- NOT USED
	        A1 --|       |-- A2
	     ROMD4 --|       |-- D4
	     ROMD5 --|       |-- D5
	     ROMD6 --|       |-- D6
	       GND --|       |-- M1 (NOT USED)
	             +-------+
	    Pin layout is such that links can replace the PAL if encryption is not used.
	*/
	static const UINT8 xortable[8][4] =
	{
		{ 6,0,6,0 },
		{ 5,1,5,1 },
		{ 4,2,6,1 },
		{ 2,4,5,0 },
		{ 4,6,1,5 },
		{ 0,6,2,5 },
		{ 0,2,0,2 },
		{ 1,4,1,4 }
	};
	UINT8 *rombase = memregion("maincpu")->base();
	UINT32 romlength = memregion("maincpu")->bytes();
	UINT32 offs;

	for (offs = 0; offs < romlength; offs++)
	{
		UINT8 data = rombase[offs];
		UINT32 line = offs & 0x07;

		data ^= (BIT(data,xortable[line][0]) << xortable[line][1]) | (BIT(data,xortable[line][2]) << xortable[line][3]);
		rombase[offs] = data;
	}
}


void galaxian_state::decode_dingoe()
{
	UINT8 *rombase = memregion("maincpu")->base();
	UINT32 romlength = memregion("maincpu")->bytes();
	UINT32 offs;

	for (offs = 0; offs < romlength; offs++)
	{
		UINT8 data = rombase[offs];

		/* XOR bit 4 with bit 2, and bit 0 with bit 5, and invert bit 1 */
		data ^= BIT(data, 2) << 4;
		data ^= BIT(data, 5) << 0;
		data ^= 0x02;

		/* Swap bit0 with bit4 */
		if (offs & 0x02)
			data = BITSWAP8(data, 7,6,5,0,3,2,1,4);
		rombase[offs] = data;
	}
}


void galaxian_state::decode_frogger_sound()
{
	UINT8 *rombase = memregion("audiocpu")->base();
	UINT32 offs;

	/* the first ROM of the sound CPU has data lines D0 and D1 swapped */
	for (offs = 0; offs < 0x800; offs++)
		rombase[offs] = BITSWAP8(rombase[offs], 7,6,5,4,3,2,0,1);
}

// froggermc has a bigger first ROM of the sound CPU, thus a different decode
void galaxian_state::decode_froggermc_sound()
{
	UINT8 *rombase = memregion("audiocpu")->base();
	UINT32 offs;

	/* the first ROM of the sound CPU has data lines D0 and D1 swapped */
	for (offs = 0; offs < 0x1000; offs++)
		rombase[offs] = BITSWAP8(rombase[offs], 7,6,5,4,3,2,0,1);
}


void galaxian_state::decode_frogger_gfx()
{
	UINT8 *rombase = memregion("gfx1")->base();
	UINT32 offs;

	/* the 2nd gfx ROM has data lines D0 and D1 swapped */
	for (offs = 0x0800; offs < 0x1000; offs++)
		rombase[offs] = BITSWAP8(rombase[offs], 7,6,5,4,3,2,0,1);
}


void galaxian_state::decode_anteater_gfx()
{
	UINT32 romlength = memregion("gfx1")->bytes();
	UINT8 *rombase = memregion("gfx1")->base();
	dynamic_buffer scratch(romlength);
	UINT32 offs;

	memcpy(&scratch[0], rombase, romlength);
	for (offs = 0; offs < romlength; offs++)
	{
		UINT32 srcoffs = offs & 0x9bf;
		srcoffs |= (BIT(offs,4) ^ BIT(offs,9) ^ (BIT(offs,2) & BIT(offs,10))) << 6;
		srcoffs |= (BIT(offs,2) ^ BIT(offs,10)) << 9;
		srcoffs |= (BIT(offs,0) ^ BIT(offs,6) ^ 1) << 10;
		rombase[offs] = scratch[srcoffs];
	}
}


void galaxian_state::decode_losttomb_gfx()
{
	UINT32 romlength = memregion("gfx1")->bytes();
	UINT8 *rombase = memregion("gfx1")->base();
	dynamic_buffer scratch(romlength);
	UINT32 offs;

	memcpy(&scratch[0], rombase, romlength);
	for (offs = 0; offs < romlength; offs++)
	{
		UINT32 srcoffs = offs & 0xa7f;
		srcoffs |= ((BIT(offs,1) & BIT(offs,8)) | ((1 ^ BIT(offs,1)) & (BIT(offs,10)))) << 7;
		srcoffs |= (BIT(offs,7) ^ (BIT(offs,1) & (BIT(offs,7) ^ BIT(offs,10)))) << 8;
		srcoffs |= ((BIT(offs,1) & BIT(offs,7)) | ((1 ^ BIT(offs,1)) & (BIT(offs,8)))) << 10;
		rombase[offs] = scratch[srcoffs];
	}
}


void galaxian_state::decode_superbon()
{
	offs_t i;
	UINT8 *RAM;

	/* Decryption worked out by hand by Chris Hardy. */

	RAM = memregion("maincpu")->base();

	for (i = 0;i < 0x1000;i++)
	{
		/* Code is encrypted depending on bit 7 and 9 of the address */
		switch (i & 0x0280)
		{
		case 0x0000:
			RAM[i] ^= 0x92;
			break;
		case 0x0080:
			RAM[i] ^= 0x82;
			break;
		case 0x0200:
			RAM[i] ^= 0x12;
			break;
		case 0x0280:
			RAM[i] ^= 0x10;
			break;
		}
	}
}


/*************************************
 *
 *  Driver configuration
 *
 *************************************/

void galaxian_state::common_init(galaxian_draw_bullet_func draw_bullet,galaxian_draw_background_func draw_background,
	galaxian_extend_tile_info_func extend_tile_info,galaxian_extend_sprite_info_func extend_sprite_info)
{
	m_irq_enabled = 0;
	m_irq_line = INPUT_LINE_NMI;
	m_numspritegens = 1;
	m_bullets_base = 0x60;
	m_sprites_base = 0x40;
	m_frogger_adjust = FALSE;
	m_sfx_tilemap = FALSE;
	m_draw_bullet_ptr = (draw_bullet != NULL) ? draw_bullet : &galaxian_state::galaxian_draw_bullet;
	m_draw_background_ptr = (draw_background != NULL) ? draw_background : &galaxian_state::galaxian_draw_background;
	m_extend_tile_info_ptr = extend_tile_info;
	m_extend_sprite_info_ptr = extend_sprite_info;
}


void galaxian_state::unmap_galaxian_sound(offs_t base)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	space.unmap_write(base + 0x0004, base + 0x0007, 0, 0x07f8);
	space.unmap_write(base + 0x0800, base + 0x0807, 0, 0x07f8);
	space.unmap_write(base + 0x1800, base + 0x1800, 0, 0x07ff);
}



/*************************************
 *
 *  Galaxian-derived games
 *
 *************************************/

DRIVER_INIT_MEMBER(galaxian_state,galaxian)
{
	common_init(&galaxian_state::galaxian_draw_bullet, &galaxian_state::galaxian_draw_background, NULL, NULL);
}


DRIVER_INIT_MEMBER(galaxian_state,nolock)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* same as galaxian... */
	DRIVER_INIT_CALL(galaxian);

	/* ...but coin lockout disabled/disconnected */
	space.unmap_write(0x6002, 0x6002, 0, 0x7f8);
}


DRIVER_INIT_MEMBER(galaxian_state,azurian)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* yellow bullets instead of white ones */
	common_init(&galaxian_state::scramble_draw_bullet, &galaxian_state::galaxian_draw_background, NULL, NULL);

	/* coin lockout disabled */
	space.unmap_write(0x6002, 0x6002, 0, 0x7f8);
}


DRIVER_INIT_MEMBER(galaxian_state,gmgalax)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* video extensions */
	common_init(&galaxian_state::galaxian_draw_bullet, &galaxian_state::galaxian_draw_background, &galaxian_state::gmgalax_extend_tile_info, &galaxian_state::gmgalax_extend_sprite_info);

	/* ROM is banked */
	space.install_read_bank(0x0000, 0x3fff, "bank1");
	membank("bank1")->configure_entries(0, 2, memregion("maincpu")->base() + 0x10000, 0x4000);

	/* callback when the game select is toggled */
	gmgalax_game_changed(*machine().ioport().first_port()->first_field(), NULL, 0, 0);
	save_item(NAME(m_gmgalax_selected_game));
}


DRIVER_INIT_MEMBER(galaxian_state,pisces)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* video extensions */
	common_init(&galaxian_state::galaxian_draw_bullet, &galaxian_state::galaxian_draw_background, &galaxian_state::pisces_extend_tile_info, &galaxian_state::pisces_extend_sprite_info);

	/* coin lockout replaced by graphics bank */
	space.install_write_handler(0x6002, 0x6002, 0, 0x7f8, write8_delegate(FUNC(galaxian_state::galaxian_gfxbank_w),this));
}


DRIVER_INIT_MEMBER(galaxian_state,batman2)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* video extensions */
	common_init(&galaxian_state::galaxian_draw_bullet, &galaxian_state::galaxian_draw_background, &galaxian_state::batman2_extend_tile_info, &galaxian_state::upper_extend_sprite_info);

	/* coin lockout replaced by graphics bank */
	space.install_write_handler(0x6002, 0x6002, 0, 0x7f8, write8_delegate(FUNC(galaxian_state::galaxian_gfxbank_w),this));
}


DRIVER_INIT_MEMBER(galaxian_state,frogg)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* same as galaxian... */
	common_init(&galaxian_state::galaxian_draw_bullet, &galaxian_state::frogger_draw_background, &galaxian_state::frogger_extend_tile_info, &galaxian_state::frogger_extend_sprite_info);

	/* ...but needs a full 2k of RAM */
	space.install_ram(0x4000, 0x47ff);
}



/*************************************
 *
 *  Moon Cresta-derived games
 *
 *************************************/

DRIVER_INIT_MEMBER(galaxian_state,mooncrst)
{
	/* video extensions */
	common_init(&galaxian_state::galaxian_draw_bullet, &galaxian_state::galaxian_draw_background, &galaxian_state::mooncrst_extend_tile_info, &galaxian_state::mooncrst_extend_sprite_info);

	/* decrypt program code */
	decode_mooncrst(0x4000, memregion("maincpu")->base());
}


DRIVER_INIT_MEMBER(galaxian_state,mooncrsu)
{
	/* video extensions */
	common_init(&galaxian_state::galaxian_draw_bullet, &galaxian_state::galaxian_draw_background, &galaxian_state::mooncrst_extend_tile_info, &galaxian_state::mooncrst_extend_sprite_info);
}


DRIVER_INIT_MEMBER(galaxian_state,mooncrgx)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* video extensions */
	common_init(&galaxian_state::galaxian_draw_bullet, &galaxian_state::galaxian_draw_background, &galaxian_state::mooncrst_extend_tile_info, &galaxian_state::mooncrst_extend_sprite_info);

	/* LEDs and coin lockout replaced by graphics banking */
	space.install_write_handler(0x6000, 0x6002, 0, 0x7f8, write8_delegate(FUNC(galaxian_state::galaxian_gfxbank_w),this));
}


DRIVER_INIT_MEMBER(galaxian_state,moonqsr)
{
	/* video extensions */
	common_init(&galaxian_state::galaxian_draw_bullet, &galaxian_state::galaxian_draw_background, &galaxian_state::moonqsr_extend_tile_info, &galaxian_state::moonqsr_extend_sprite_info);

	/* decrypt program code */
	decode_mooncrst(0x4000, m_decrypted_opcodes);
}

WRITE8_MEMBER(galaxian_state::artic_gfxbank_w)
{
//  printf("artic_gfxbank_w %02x\n",data);
}

DRIVER_INIT_MEMBER(galaxian_state,pacmanbl)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* same as galaxian... */
	DRIVER_INIT_CALL(galaxian);

	/* ...but coin lockout disabled/disconnected */
	space.install_write_handler(0x6002, 0x6002, 0, 0x7f8, write8_delegate(FUNC(galaxian_state::artic_gfxbank_w),this));
}

READ8_MEMBER(galaxian_state::tenspot_dsw_read)
{
	char tmp[64];
	sprintf(tmp,"IN2_GAME%d", m_tenspot_current_game);
	return ioport(tmp)->read_safe(0x00);
}


void galaxian_state::tenspot_set_game_bank(int bank, int from_game)
{
	char tmp[64];
	UINT8* srcregion;
	UINT8* dstregion;
	int x;

	sprintf(tmp,"game_%d_cpu", bank);
	srcregion = memregion(tmp)->base();
	dstregion = memregion("maincpu")->base();
	memcpy(dstregion, srcregion, 0x4000);

	sprintf(tmp,"game_%d_temp", bank);
	srcregion = memregion(tmp)->base();
	dstregion = memregion("gfx1")->base();
	memcpy(dstregion, srcregion, 0x2000);
	dstregion = memregion("gfx2")->base();
	memcpy(dstregion, srcregion, 0x2000);

	if (from_game)
	{
		for (x=0;x<0x200;x++)
		{
			m_gfxdecode->gfx(0)->mark_dirty(x);
		}

		for (x=0;x<0x80;x++)
		{
			m_gfxdecode->gfx(1)->mark_dirty(x);
		}
	}

	sprintf(tmp,"game_%d_prom", bank);
	srcregion = memregion(tmp)->base();
	dstregion = memregion("proms")->base();
	memcpy(dstregion, srcregion, 0x20);

	PALETTE_INIT_NAME(galaxian)(m_palette);
}

DRIVER_INIT_MEMBER(galaxian_state,tenspot)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* these are needed for batman part 2 to work properly, this banking is probably a property of the artic board,
	   which tenspot appears to have copied */

	/* video extensions */
	//common_init(&galaxian_state::galaxian_draw_bullet, &galaxian_state::galaxian_draw_background, &galaxian_state::batman2_extend_tile_info, &galaxian_state::upper_extend_sprite_info);

	/* coin lockout replaced by graphics bank */
	//space.install_write_handler(0x6002, 0x6002, 0, 0x7f8, write8_delegate(FUNC(galaxian_state::galaxian_gfxbank_w),this));


	DRIVER_INIT_CALL(galaxian);

	space.install_write_handler(0x6002, 0x6002, 0, 0x7f8, write8_delegate(FUNC(galaxian_state::artic_gfxbank_w),this));

	m_tenspot_current_game = 0;

	tenspot_set_game_bank(m_tenspot_current_game, 0);

	space.install_read_handler(0x7000, 0x7000, read8_delegate(FUNC(galaxian_state::tenspot_dsw_read),this));
}



DRIVER_INIT_MEMBER(galaxian_state,devilfsg)
{
	/* video extensions */
	common_init(&galaxian_state::galaxian_draw_bullet, &galaxian_state::galaxian_draw_background, NULL, NULL);

	/* IRQ line is INT, not NMI */
	m_irq_line = 0;
}


DRIVER_INIT_MEMBER(galaxian_state,zigzag)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* video extensions */
	common_init(NULL, &galaxian_state::galaxian_draw_background, NULL, NULL);
	m_draw_bullet_ptr = NULL;

	/* two sprite generators */
	m_numspritegens = 2;

	/* make ROMs 2 & 3 swappable */
	space.install_read_bank(0x2000, 0x2fff, "bank1");
	space.install_read_bank(0x3000, 0x3fff, "bank2");
	membank("bank1")->configure_entries(0, 2, memregion("maincpu")->base() + 0x2000, 0x1000);
	membank("bank2")->configure_entries(0, 2, memregion("maincpu")->base() + 0x2000, 0x1000);

	/* also re-install the fixed ROM area as a bank in order to inform the memory system that
	   the fixed area only extends to 0x1fff */
	space.install_read_bank(0x0000, 0x1fff, "bank3");
	membank("bank3")->set_base(memregion("maincpu")->base() + 0x0000);

	/* handler for doing the swaps */
	space.install_write_handler(0x7002, 0x7002, 0, 0x07f8, write8_delegate(FUNC(galaxian_state::zigzag_bankswap_w),this));
	zigzag_bankswap_w(space, 0, 0);

	/* coin lockout disabled */
	space.unmap_write(0x6002, 0x6002, 0, 0x7f8);

	/* remove the galaxian sound hardware */
	unmap_galaxian_sound( 0x6000);

	/* install our AY-8910 handler */
	space.install_write_handler(0x4800, 0x4fff, write8_delegate(FUNC(galaxian_state::zigzag_ay8910_w),this));
}


DRIVER_INIT_MEMBER(galaxian_state,jumpbug)
{
	/* video extensions */
	common_init(&galaxian_state::scramble_draw_bullet, &galaxian_state::jumpbug_draw_background, &galaxian_state::jumpbug_extend_tile_info, &galaxian_state::jumpbug_extend_sprite_info);
}


DRIVER_INIT_MEMBER(galaxian_state,checkman)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	address_space &iospace = m_maincpu->space(AS_IO);

	/* video extensions */
	common_init(&galaxian_state::galaxian_draw_bullet, &galaxian_state::galaxian_draw_background, &galaxian_state::mooncrst_extend_tile_info, &galaxian_state::mooncrst_extend_sprite_info);

	/* move the interrupt enable from $b000 to $b001 */
	space.unmap_write(0xb000, 0xb000, 0, 0x7f8);
	space.install_write_handler(0xb001, 0xb001, 0, 0x7f8, write8_delegate(FUNC(galaxian_state::irq_enable_w),this));

	/* attach the sound command handler */
	iospace.install_write_handler(0x00, 0x00, 0, 0xffff, write8_delegate(FUNC(galaxian_state::checkman_sound_command_w),this));

	/* decrypt program code */
	decode_checkman();
}


DRIVER_INIT_MEMBER(galaxian_state,checkmaj)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* video extensions */
	common_init(&galaxian_state::galaxian_draw_bullet, &galaxian_state::galaxian_draw_background, NULL, NULL);

	/* attach the sound command handler */
	space.install_write_handler(0x7800, 0x7800, 0, 0x7ff, write8_delegate(FUNC(galaxian_state::checkman_sound_command_w),this));

	/* for the title screen */
	space.install_read_handler(0x3800, 0x3800, read8_delegate(FUNC(galaxian_state::checkmaj_protection_r),this));
}


DRIVER_INIT_MEMBER(galaxian_state,dingo)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* video extensions */
	common_init(&galaxian_state::galaxian_draw_bullet, &galaxian_state::galaxian_draw_background, NULL, NULL);

	/* attach the sound command handler */
	space.install_write_handler(0x7800, 0x7800, 0, 0x7ff, write8_delegate(FUNC(galaxian_state::checkman_sound_command_w),this));

	space.install_read_handler(0x3000, 0x3000, read8_delegate(FUNC(galaxian_state::dingo_3000_r),this));
	space.install_read_handler(0x3035, 0x3035, read8_delegate(FUNC(galaxian_state::dingo_3035_r),this));
}


DRIVER_INIT_MEMBER(galaxian_state,dingoe)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	address_space &iospace = m_maincpu->space(AS_IO);

	/* video extensions */
	common_init(&galaxian_state::galaxian_draw_bullet, &galaxian_state::galaxian_draw_background, &galaxian_state::mooncrst_extend_tile_info, &galaxian_state::mooncrst_extend_sprite_info);

	/* move the interrupt enable from $b000 to $b001 */
	space.unmap_write(0xb000, 0xb000, 0, 0x7f8);
	space.install_write_handler(0xb001, 0xb001, 0, 0x7f8, write8_delegate(FUNC(galaxian_state::irq_enable_w),this));

	/* attach the sound command handler */
	iospace.install_write_handler(0x00, 0x00, 0, 0xffff, write8_delegate(FUNC(galaxian_state::checkman_sound_command_w),this));

	space.install_read_handler(0x3001, 0x3001, read8_delegate(FUNC(galaxian_state::dingoe_3001_r),this));   /* Protection check */

	/* decrypt program code */
	decode_dingoe();
}


DRIVER_INIT_MEMBER(galaxian_state,skybase)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* video extensions */
	common_init(&galaxian_state::galaxian_draw_bullet, &galaxian_state::galaxian_draw_background, &galaxian_state::pisces_extend_tile_info, &galaxian_state::pisces_extend_sprite_info);

	/* coin lockout replaced by graphics bank */
	space.install_write_handler(0xa002, 0xa002, 0, 0x7f8, write8_delegate(FUNC(galaxian_state::galaxian_gfxbank_w),this));

	/* needs a full 2k of RAM */
	space.install_ram(0x8000, 0x87ff);

	/* extend ROM */
	space.install_rom(0x0000, 0x5fff, memregion("maincpu")->base());
}


DRIVER_INIT_MEMBER(galaxian_state,kong)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* video extensions */
	common_init(&galaxian_state::galaxian_draw_bullet, &galaxian_state::galaxian_draw_background, NULL, &galaxian_state::upper_extend_sprite_info);

	/* needs a full 2k of RAM */
	space.install_ram(0x8000, 0x87ff);

	/* extend ROM */
	space.install_rom(0x0000, 0x7fff, memregion("maincpu")->base());
}


void galaxian_state::mshuttle_decode(const UINT8 convtable[8][16])
{
	UINT8 *rom = memregion("maincpu")->base();

	for (int A = 0x0000;A < 0x8000;A++)
	{
		int i,j;
		UINT8 src = rom[A];

		/* pick the translation table from bit 0 of the address */
		/* and from bits 1 7 of the source data */
		i = (A & 1) | (src & 0x02) | ((src & 0x80) >> 5);

		/* pick the offset in the table from bits 0 2 4 6 of the source data */
		j = (src & 0x01) | ((src & 0x04) >> 1) | ((src & 0x10) >> 2) | ((src & 0x40) >> 3);

		/* decode the opcodes */
		m_decrypted_opcodes[A] = (src & 0xaa) | convtable[i][j];
	}
}


DRIVER_INIT_MEMBER(galaxian_state,mshuttle)
{
	static const UINT8 convtable[8][16] =
	{
		/* -1 marks spots which are unused and therefore unknown */
		{ 0x40,0x41,0x44,0x15,0x05,0x51,0x54,0x55,0x50,0x00,0x01,0x04,  -1,0x10,0x11,0x14 },
		{ 0x45,0x51,0x55,0x44,0x40,0x11,0x05,0x41,0x10,0x14,0x54,0x50,0x15,0x04,0x00,0x01 },
		{ 0x11,0x14,0x10,0x00,0x44,0x05,  -1,0x04,0x45,0x15,0x55,0x50,  -1,0x01,0x54,0x51 },
		{ 0x14,0x01,0x11,0x10,0x50,0x15,0x00,0x40,0x04,0x51,0x45,0x05,0x55,0x54,  -1,0x44 },
		{ 0x04,0x10,  -1,0x40,0x15,0x41,0x50,0x50,0x11,  -1,0x14,0x00,0x51,0x45,0x55,0x01 },
		{ 0x44,0x45,0x00,0x51,  -1,  -1,0x15,0x11,0x01,0x10,0x04,0x55,0x05,0x40,0x50,0x41 },
		{ 0x51,0x00,0x01,0x05,0x04,0x55,0x54,0x50,0x41,  -1,0x11,0x15,0x14,0x10,0x44,0x40 },
		{ 0x05,0x04,0x51,0x01,  -1,  -1,0x55,  -1,0x00,0x50,0x15,0x14,0x44,0x41,0x40,0x54 },
	};

	/* video extensions */
	common_init(&galaxian_state::mshuttle_draw_bullet, &galaxian_state::galaxian_draw_background, &galaxian_state::mshuttle_extend_tile_info, &galaxian_state::mshuttle_extend_sprite_info);

	/* IRQ line is INT, not NMI */
	m_irq_line = 0;

	/* decrypt the code */
	mshuttle_decode(convtable);
}


DRIVER_INIT_MEMBER(galaxian_state,mshuttlj)
{
	static const UINT8 convtable[8][16] =
	{
		{ 0x41,0x54,0x51,0x14,0x05,0x10,0x01,0x55,0x44,0x11,0x00,0x50,0x15,0x40,0x04,0x45 },
		{ 0x50,0x11,0x40,0x55,0x51,0x14,0x45,0x04,0x54,0x15,0x10,0x05,0x44,0x01,0x00,0x41 },
		{ 0x44,0x11,0x00,0x50,0x41,0x54,0x04,0x14,0x15,0x40,0x51,0x55,0x05,0x10,0x01,0x45 },
		{ 0x10,0x50,0x54,0x55,0x01,0x44,0x40,0x04,0x14,0x11,0x00,0x41,0x45,0x15,0x51,0x05 },
		{ 0x14,0x41,0x01,0x44,0x04,0x50,0x51,0x45,0x11,0x40,0x54,0x15,0x10,0x00,0x55,0x05 },
		{ 0x01,0x05,0x41,0x45,0x54,0x50,0x55,0x10,0x11,0x15,0x51,0x14,0x44,0x40,0x04,0x00 },
		{ 0x05,0x55,0x00,0x50,0x11,0x40,0x54,0x14,0x45,0x51,0x10,0x04,0x44,0x01,0x41,0x15 },
		{ 0x55,0x50,0x15,0x10,0x01,0x04,0x41,0x44,0x45,0x40,0x05,0x00,0x11,0x14,0x51,0x54 },
	};

	/* video extensions */
	common_init(&galaxian_state::mshuttle_draw_bullet, &galaxian_state::galaxian_draw_background, &galaxian_state::mshuttle_extend_tile_info, &galaxian_state::mshuttle_extend_sprite_info);

	/* IRQ line is INT, not NMI */
	m_irq_line = 0;

	/* decrypt the code */
	mshuttle_decode(convtable);
}


DRIVER_INIT_MEMBER(galaxian_state,fantastc)
{
	/* video extensions */
	common_init(&galaxian_state::galaxian_draw_bullet, &galaxian_state::galaxian_draw_background, NULL, &galaxian_state::upper_extend_sprite_info);

	/* two sprite generators */
	m_numspritegens = 2;

	/* bullets moved from $60 to $c0 */
	m_bullets_base = 0xc0;

	/* decode code */
	static const UINT16 lut_am_unscramble[32] = {
		0, 2, 4, 6, // ok!
		7, 3, 5, 1, // ok!
		6, 0, 2, 4, // ok!
		1, 5, 3, 0, // ok!
		2, 4, 6, 3, // good, good?, guess, guess
		5, 6, 0, 2, // good, good?, good?, guess
		4, 1, 1, 5, // good, good, guess, good
		3, 7, 7, 7  // ok!
	};

	UINT8* romdata = memregion("maincpu")->base();
	assert(memregion("maincpu")->bytes() == 0x8000);
	UINT8 buf[0x8000];
	memcpy(buf, romdata, 0x8000);

	for (int i = 0; i < 32; i++)
		memcpy(romdata + i * 0x400, buf + lut_am_unscramble[i] * 0x1000 + (i & 3) * 0x400, 0x400);
}


DRIVER_INIT_MEMBER(galaxian_state,timefgtr)
{
	/* two sprite generators */
	m_numspritegens = 2;

	/* bullets moved from $60 to $c0 */
	m_bullets_base = 0xc0;

	/* video extensions */
	common_init(&galaxian_state::galaxian_draw_bullet, &galaxian_state::galaxian_draw_background, NULL, &galaxian_state::upper_extend_sprite_info);
}


DRIVER_INIT_MEMBER(galaxian_state,kingball)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* video extensions */
	common_init(&galaxian_state::galaxian_draw_bullet, &galaxian_state::galaxian_draw_background, NULL, NULL);

	/* disable the stars */
	space.unmap_write(0xb004, 0xb004, 0, 0x07f8);

	space.install_write_handler(0xb000, 0xb000, 0, 0x7f8, write8_delegate(FUNC(galaxian_state::kingball_sound1_w),this));
	space.install_write_handler(0xb001, 0xb001, 0, 0x7f8, write8_delegate(FUNC(galaxian_state::irq_enable_w),this));
	space.install_write_handler(0xb002, 0xb002, 0, 0x7f8, write8_delegate(FUNC(galaxian_state::kingball_sound2_w),this));
	space.install_write_handler(0xb003, 0xb003, 0, 0x7f8, write8_delegate(FUNC(galaxian_state::kingball_speech_dip_w),this));

	save_item(NAME(m_kingball_speech_dip));
	save_item(NAME(m_kingball_sound));
}


DRIVER_INIT_MEMBER(galaxian_state,scorpnmc)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* video extensions */
	common_init(&galaxian_state::galaxian_draw_bullet, &galaxian_state::galaxian_draw_background, &galaxian_state::batman2_extend_tile_info, &galaxian_state::upper_extend_sprite_info);

	/* move the interrupt enable from $b000 to $b001 */
	space.unmap_write(0xb000, 0xb000, 0, 0x7f8);
	space.install_write_handler(0xb001, 0xb001, 0, 0x7f8, write8_delegate(FUNC(galaxian_state::irq_enable_w),this));

	/* extra ROM */
	space.install_rom(0x5000, 0x67ff, memregion("maincpu")->base() + 0x5000);

	/* install RAM at $4000-$4800 */
	space.install_ram(0x4000, 0x47ff);

	/* doesn't appear to use original RAM */
	space.unmap_readwrite(0x8000, 0x87ff);
}

DRIVER_INIT_MEMBER(galaxian_state,thepitm)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* video extensions */
	common_init(&galaxian_state::galaxian_draw_bullet, &galaxian_state::galaxian_draw_background, &galaxian_state::mooncrst_extend_tile_info, &galaxian_state::mooncrst_extend_sprite_info);

	/* move the interrupt enable from $b000 to $b001 */
	space.unmap_write(0xb000, 0xb000, 0, 0x7f8);
	space.install_write_handler(0xb001, 0xb001, 0, 0x7f8, write8_delegate(FUNC(galaxian_state::irq_enable_w),this));

	/* disable the stars */
	space.unmap_write(0xb004, 0xb004, 0, 0x07f8);

	/* extend ROM */
	space.install_rom(0x0000, 0x47ff, memregion("maincpu")->base());
}

/*************************************
 *
 *  Konami games
 *
 *************************************/

DRIVER_INIT_MEMBER(galaxian_state,theend)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* video extensions */
	common_init(&galaxian_state::theend_draw_bullet, &galaxian_state::galaxian_draw_background, NULL, NULL);

	/* coin counter on the upper bit of port C */
	space.unmap_write(0x6802, 0x6802, 0, 0x7f8);
}


DRIVER_INIT_MEMBER(galaxian_state,scramble)
{
	/* video extensions */
	common_init(&galaxian_state::scramble_draw_bullet, &galaxian_state::scramble_draw_background, NULL, NULL);
}


DRIVER_INIT_MEMBER(galaxian_state,explorer)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* video extensions */
	common_init(&galaxian_state::scramble_draw_bullet, &galaxian_state::scramble_draw_background, NULL, NULL);

	/* watchdog works for writes as well? (or is it just disabled?) */
	space.install_write_handler(0x7000, 0x7000, 0, 0x7ff, write8_delegate(FUNC(galaxian_state::watchdog_reset_w),this));

	/* I/O appears to be direct, not via PPIs */
	space.unmap_readwrite(0x8000, 0xffff);
	space.install_read_port(0x8000, 0x8000, 0, 0xffc, "IN0");
	space.install_read_port(0x8001, 0x8001, 0, 0xffc, "IN1");
	space.install_read_port(0x8002, 0x8002, 0, 0xffc, "IN2");
	space.install_read_port(0x8003, 0x8003, 0, 0xffc, "IN3");
	space.install_write_handler(0x8000, 0x8000, 0, 0xfff, write8_delegate(FUNC(galaxian_state::soundlatch_byte_w),this));
	space.install_write_handler(0x9000, 0x9000, 0, 0xfff, write8_delegate(FUNC(galaxian_state::explorer_sound_control_w),this));
}


DRIVER_INIT_MEMBER(galaxian_state,sfx)
{
	/* basic configuration */
	common_init(&galaxian_state::scramble_draw_bullet, &galaxian_state::scramble_draw_background, &galaxian_state::upper_extend_tile_info, NULL);
	m_sfx_tilemap = TRUE;

	/* sound board has space for extra ROM */
	m_audiocpu->space(AS_PROGRAM).install_read_bank(0x0000, 0x3fff, "bank1");
	membank("bank1")->set_base(memregion("audiocpu")->base());
}


DRIVER_INIT_MEMBER(galaxian_state,atlantis)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* video extensions */
	common_init(&galaxian_state::scramble_draw_bullet, &galaxian_state::scramble_draw_background, NULL, NULL);

	/* watchdog is at $7800? (or is it just disabled?) */
	space.unmap_read(0x7000, 0x7000, 0, 0x7ff);
	space.install_read_handler(0x7800, 0x7800, 0, 0x7ff, read8_delegate(FUNC(galaxian_state::watchdog_reset_r),this));
}





DRIVER_INIT_MEMBER(galaxian_state,scobra)
{
	/* video extensions */
	common_init(&galaxian_state::scramble_draw_bullet, &galaxian_state::scramble_draw_background, NULL, NULL);
}



DRIVER_INIT_MEMBER(galaxian_state,scobrae)
{
	UINT8 *rom = memregion("maincpu")->base();
	int offs;

	for (offs = 0; offs < 0x6000; offs++)
	{
		int i = offs & 0x7f;
		int x = rom[offs];

		if (offs & 0x80) i ^= 0x7f;

		if (i & 0x01) x ^= 0x49;
		if (i & 0x02) x ^= 0x21;
		if (i & 0x04) x ^= 0x18;
		if (i & 0x08) x ^= 0x12;
		if (i & 0x10) x ^= 0x84;
		if (i & 0x20) x ^= 0x24;
		if (i & 0x40) x ^= 0x40;

		rom[offs] = x ^ 0xff;
	}

	/* video extensions */
	common_init(&galaxian_state::scramble_draw_bullet, &galaxian_state::scramble_draw_background, NULL, NULL);
}


DRIVER_INIT_MEMBER(galaxian_state,losttomb)
{
	/* video extensions */
	common_init(&galaxian_state::scramble_draw_bullet, &galaxian_state::scramble_draw_background, NULL, NULL);

	/* decrypt */
	decode_losttomb_gfx();
}


DRIVER_INIT_MEMBER(galaxian_state,frogger)
{
	/* video extensions */
	common_init(NULL, &galaxian_state::frogger_draw_background, &galaxian_state::frogger_extend_tile_info, &galaxian_state::frogger_extend_sprite_info);
	m_frogger_adjust = TRUE;

	/* decrypt */
	decode_frogger_sound();
	decode_frogger_gfx();
}


DRIVER_INIT_MEMBER(galaxian_state,quaak)
{
	/* video extensions */
	common_init(NULL, &galaxian_state::quaak_draw_background, &galaxian_state::frogger_extend_tile_info, &galaxian_state::frogger_extend_sprite_info);
}


DRIVER_INIT_MEMBER(galaxian_state,froggermc)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* video extensions */
	common_init(NULL, &galaxian_state::frogger_draw_background, &galaxian_state::frogger_extend_tile_info, &galaxian_state::frogger_extend_sprite_info);

	space.install_write_handler(0xa800, 0xa800, 0, 0x7ff, write8_delegate(FUNC(galaxian_state::soundlatch_byte_w),this));
	space.install_write_handler(0xb001, 0xb001, 0, 0x7f8, write8_delegate(FUNC(galaxian_state::froggermc_sound_control_w),this));

	/* actually needs 2k of RAM */
	space.install_ram(0x8000, 0x87ff);

	/* decrypt */
	decode_froggermc_sound();
}


DRIVER_INIT_MEMBER(galaxian_state,froggers)
{
	/* video extensions */
	common_init(NULL, &galaxian_state::frogger_draw_background, &galaxian_state::frogger_extend_tile_info, &galaxian_state::frogger_extend_sprite_info);

	/* decrypt */
	decode_frogger_sound();
}


DRIVER_INIT_MEMBER(galaxian_state,turtles)
{
	/* video extensions */
	common_init(NULL, &galaxian_state::turtles_draw_background, NULL, NULL);
}


#ifdef UNUSED_FUNCTION
DRIVER_INIT_MEMBER(galaxian_state,amidar)
{
	/* no existing amidar sets run on Amidar hardware as described by Amidar schematics! */
	/* video extensions */
	common_init(&galaxian_state::scramble_draw_bullet, &galaxian_state::amidar_draw_background, NULL, NULL);
}
#endif


DRIVER_INIT_MEMBER(galaxian_state,scorpion)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	common_init(&galaxian_state::scramble_draw_bullet, &galaxian_state::scramble_draw_background, &galaxian_state::batman2_extend_tile_info, &galaxian_state::upper_extend_sprite_info);

	/* hook up AY8910 */
	m_audiocpu->space(AS_IO).install_readwrite_handler(0x00, 0xff, read8_delegate(FUNC(galaxian_state::scorpion_ay8910_r),this), write8_delegate(FUNC(galaxian_state::scorpion_ay8910_w),this));

	/* extra ROM */
	space.install_read_bank(0x5800, 0x67ff, "bank1");
	membank("bank1")->set_base(memregion("maincpu")->base() + 0x5800);

	/* no background related */
//  space.nop_write(0x6803, 0x6803);

	m_audiocpu->space(AS_PROGRAM).install_read_handler(0x3000, 0x3000, read8_delegate(FUNC(galaxian_state::scorpion_digitalker_intr_r),this));

	save_item(NAME(m_protection_state));
/*
{
    const UINT8 *rom = memregion("speech")->base();
    int i;

    for (i = 0; i < 0x2c; i++)
    {
        UINT16 addr = (rom[2*i] << 8) | rom[2*i+1];
        UINT16 endaddr = (rom[2*i+2] << 8) | rom[2*i+3];
        int j;
        printf("Cmd %02X -> %04X-%04X:", i, addr, endaddr - 1);
        for (j = 0; j < 32 && addr < endaddr; j++)
            printf(" %02X", rom[addr++]);
        printf("\n");
    }
}
*/
}


DRIVER_INIT_MEMBER(galaxian_state,anteater)
{
	/* video extensions */
	common_init(&galaxian_state::scramble_draw_bullet, &galaxian_state::anteater_draw_background, NULL, NULL);

	/* decode graphics */
	decode_anteater_gfx();
}


DRIVER_INIT_MEMBER(galaxian_state,anteateruk)
{
	/* video extensions */
	common_init(&galaxian_state::scramble_draw_bullet, &galaxian_state::anteater_draw_background, NULL, NULL);
}


DRIVER_INIT_MEMBER(galaxian_state,superbon)
{
	/* video extensions */
	common_init(&galaxian_state::scramble_draw_bullet, &galaxian_state::scramble_draw_background, NULL, NULL);

	/* decode code */
	decode_superbon();
}


DRIVER_INIT_MEMBER(galaxian_state,calipso)
{
	/* video extensions */
	common_init(&galaxian_state::scramble_draw_bullet, &galaxian_state::scramble_draw_background, NULL, &galaxian_state::calipso_extend_sprite_info);
}


DRIVER_INIT_MEMBER(galaxian_state,moonwar)
{
	/* video extensions */
	common_init(&galaxian_state::scramble_draw_bullet, &galaxian_state::scramble_draw_background, NULL, NULL);

	save_item(NAME(m_moonwar_port_select));
}


DRIVER_INIT_MEMBER( galaxian_state, ghostmun )
{
	/* same as Pacmanbl... */
	DRIVER_INIT_CALL(pacmanbl);

	/* ...but sprite clip limits need to be adjusted */
	//galaxian_sprite_clip_start = 12; // this adjustment no longer exists
	//galaxian_sprite_clip_end = 250;
}

DRIVER_INIT_MEMBER( galaxian_state, froggrs )
{
	/* video extensions */
	common_init(NULL, &galaxian_state::frogger_draw_background, &galaxian_state::frogger_extend_tile_info, &galaxian_state::frogger_extend_sprite_info);

	/* decrypt */
	decode_frogger_sound();
	decode_frogger_gfx();
}



/*************************************
 *
 *  ROM definitions
 *  Galaxian-derived games
 *
 *************************************/

ROM_START( galaxian )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "galmidw.u",    0x0000, 0x0800, CRC(745e2d61) SHA1(e65f74e35b1bfaccd407e168ea55678ae9b68edf) )
	ROM_LOAD( "galmidw.v",    0x0800, 0x0800, CRC(9c999a40) SHA1(02fdcd95d8511e64c0d2b007b874112d53e41045) )
	ROM_LOAD( "galmidw.w",    0x1000, 0x0800, CRC(b5894925) SHA1(0046b9ed697a34d088de1aead8bd7cbe526a2396) )
	ROM_LOAD( "galmidw.y",    0x1800, 0x0800, CRC(6b3ca10b) SHA1(18d8714e5ef52f63ba8888ecc5a25b17b3bf17d1) )
	ROM_LOAD( "7l",           0x2000, 0x0800, CRC(1b933207) SHA1(8b44b0f74420871454e27894d0f004859f9e59a9) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "1h.bin",       0x0000, 0x0800, CRC(39fb43a4) SHA1(4755609bd974976f04855d51e08ec0d62ab4bc07) )
	ROM_LOAD( "1k.bin",       0x0800, 0x0800, CRC(7e3f56a2) SHA1(a9795d8b7388f404f3b0e2c6ce15d713a4c5bafa) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( galaxiana )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "7f.bin",       0x0000, 0x1000, CRC(4335b1de) SHA1(e41e3d90dac738cf71377f3b476ec67b14dee27a) )
	ROM_LOAD( "7j.bin",       0x1000, 0x1000, CRC(4e6f66a1) SHA1(ee2a675ab34485c0f58c51be7630a51e27a7a8f3) )
	ROM_LOAD( "7l.bin",       0x2000, 0x0800, CRC(5341d75a) SHA1(40bc8fcc598f58c6ff944e2a4a9288463e75a09d) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "1h.bin",       0x0000, 0x0800, CRC(39fb43a4) SHA1(4755609bd974976f04855d51e08ec0d62ab4bc07) )
	ROM_LOAD( "1k.bin",       0x0800, 0x0800, CRC(7e3f56a2) SHA1(a9795d8b7388f404f3b0e2c6ce15d713a4c5bafa) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( galaxianm )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "galmidw.u",    0x0000, 0x0800, CRC(745e2d61) SHA1(e65f74e35b1bfaccd407e168ea55678ae9b68edf) )
	ROM_LOAD( "galmidw.v",    0x0800, 0x0800, CRC(9c999a40) SHA1(02fdcd95d8511e64c0d2b007b874112d53e41045) )
	ROM_LOAD( "galmidw.w",    0x1000, 0x0800, CRC(b5894925) SHA1(0046b9ed697a34d088de1aead8bd7cbe526a2396) )
	ROM_LOAD( "galmidw.y",    0x1800, 0x0800, CRC(6b3ca10b) SHA1(18d8714e5ef52f63ba8888ecc5a25b17b3bf17d1) )
	ROM_LOAD( "galmidw.z",    0x2000, 0x0800, CRC(cb24f797) SHA1(e6bb977ded0654c2c7388aad188059e1e0647908) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "galaxian.j1",  0x0000, 0x0800, CRC(84decf98) SHA1(2e565cb6057b1816a6b4541e6dfadd3c3762fa36) )
	ROM_LOAD( "galaxian.l1",  0x0800, 0x0800, CRC(c31ada9e) SHA1(237ebb48549b34ca59a13cc2706512d957413ec4) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( galaxianmo )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "galaxian.u",   0x0000, 0x0800, CRC(fac42d34) SHA1(0b96d9f1c6bf0e0b7f757dcbaeacfbfafefc54d1) )
	ROM_LOAD( "galaxian.v",   0x0800, 0x0800, CRC(f58283e3) SHA1(edc6e72516c50fd3402281d9936574d276581ce9) )
	ROM_LOAD( "galaxian.w",   0x1000, 0x0800, CRC(4c7031c0) SHA1(97f7ab0cedcd8eba1c8f6f516d84d672a2108258) )
	ROM_LOAD( "galaxian.y",   0x1800, 0x0800, CRC(96a7ac94) SHA1(c3c7a43117c8b9fd8621823c872889f8e31bf935) )
	ROM_LOAD( "7l.bin",       0x2000, 0x0800, CRC(5341d75a) SHA1(40bc8fcc598f58c6ff944e2a4a9288463e75a09d) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "galaxian.j1",  0x0000, 0x0800, CRC(84decf98) SHA1(2e565cb6057b1816a6b4541e6dfadd3c3762fa36) )
	ROM_LOAD( "galaxian.l1",  0x0800, 0x0800, CRC(c31ada9e) SHA1(237ebb48549b34ca59a13cc2706512d957413ec4) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( galaxiant )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "gl-03.8g",  0x0000, 0x0800, CRC(e8f3aa67) SHA1(a0e9576784dbe602dd9780e667f01f31defd7c00) ) /* All eprom are HN462716 eproms */
	ROM_LOAD( "gl-04.8f",  0x0800, 0x0800, CRC(f58283e3) SHA1(edc6e72516c50fd3402281d9936574d276581ce9) )
	ROM_LOAD( "gl-05.8e",  0x1000, 0x0800, CRC(4c7031c0) SHA1(97f7ab0cedcd8eba1c8f6f516d84d672a2108258) )
	ROM_LOAD( "gl-06.8d",  0x1800, 0x0800, CRC(097d92a2) SHA1(63ef86657286a4e1fae4f795e0e6b410ca2ef06b) )
	ROM_LOAD( "gl-07.8c",  0x2000, 0x0800, CRC(5341d75a) SHA1(40bc8fcc598f58c6ff944e2a4a9288463e75a09d) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "gl-02.1k",  0x0000, 0x0800, CRC(d14f7510) SHA1(cd09e6ee0a3890d01b2415f5b8346c42c02d15a3) ) /* This arrangement produces the correct colors */
	ROM_LOAD( "gl-01.1j",  0x0800, 0x0800, CRC(968b6016) SHA1(f13e4a8d0fdeb121d39ca76120acdc6c7e7f377c) ) /* Are the PCB locations reversed for these two? */

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6l.bpr",    0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( galaxiani )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "cp-1.8g",  0x0000, 0x0800, CRC(e8f3aa67) SHA1(a0e9576784dbe602dd9780e667f01f31defd7c00) ) /* All eprom are MBM2716 eproms */
	ROM_LOAD( "cp-2.8f",  0x0800, 0x0800, CRC(f58283e3) SHA1(edc6e72516c50fd3402281d9936574d276581ce9) )
	ROM_LOAD( "cp-3.8e",  0x1000, 0x0800, CRC(4c7031c0) SHA1(97f7ab0cedcd8eba1c8f6f516d84d672a2108258) )
	ROM_LOAD( "cp-4.8d",  0x1800, 0x0800, CRC(097d92a2) SHA1(63ef86657286a4e1fae4f795e0e6b410ca2ef06b) )
	ROM_LOAD( "cp-5.8c",  0x2000, 0x0800, CRC(5341d75a) SHA1(40bc8fcc598f58c6ff944e2a4a9288463e75a09d) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "cp-7.1k",  0x0000, 0x0800, CRC(287159b7) SHA1(a0bcdac1f133d4386dababba36177b99a21c5872) )
	ROM_LOAD( "cp-6.1j",  0x0800, 0x0800, CRC(6fb54cb1) SHA1(485f05203d9c4b4d24ecba699c8d8cdff3eb021a) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6l.bpr",    0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( galaxrf )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "princip1.u",   0x0000, 0x0800, CRC(3d5d9bae) SHA1(36ef85b42c361e70cd6f31351d6f4b0ef3f3492f) )
	ROM_LOAD( "princip2.v",   0x0800, 0x0800, CRC(a433067e) SHA1(1aed1a2153c4a32a9996fc709e544f2063885599) )
	ROM_LOAD( "princip3.w",   0x1000, 0x0800, CRC(aaf038d4) SHA1(2d070fe7c4e9b26092f0f12a9db3392f7d8a65f1) )
	ROM_LOAD( "princip4.y",   0x1800, 0x0800, CRC(d74bdd2a) SHA1(68917489b90e7fc3dd1fe9f18d7ef25e12c8d823) )
	ROM_LOAD( "princip5.z",   0x2000, 0x0800, CRC(7eeb9e63) SHA1(c05da0f8a3c06aff441f3f9bda891f3e173dc7b7) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "graphhj.j1",  0x0000, 0x0800, CRC(39fb43a4) SHA1(4755609bd974976f04855d51e08ec0d62ab4bc07) )
	ROM_LOAD( "graphkl.l1",  0x0800, 0x0800, CRC(7e3f56a2) SHA1(a9795d8b7388f404f3b0e2c6ce15d713a4c5bafa) )

	ROM_REGION( 0x0020, "proms", 0 ) // assumed to be the same
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( galaxrfgg )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "gxrf.7f",       0x0000, 0x1000, CRC(c06eeb10) SHA1(cf1006a7ff02fe8b04a096d802fb8d8937dd913d) )
	ROM_LOAD( "gxrf.7j",       0x1000, 0x1000, CRC(182ff334) SHA1(11e84aa887679e3fa977f00dd0b57a7df8ca7d88) )
	ROM_LOAD( "gxrf.7l",       0x2000, 0x0800, CRC(ee827e75) SHA1(67306fdfa54aa4e3e9ccaa7f518e58711b6759fe) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "gxrf.1jh",       0x0000, 0x0800, CRC(23e627ff) SHA1(11f8f50fcaa29f757f27d77ea2b977f65dc87e38) )
	ROM_LOAD( "gxrf.1lk",       0x0800, 0x0800, CRC(0dbcee5b) SHA1(b169c6e539a583a99e1e3ef5982d4c1ab395551f) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "gxrf.6l",       0x0000, 0x0020, CRC(992350e5) SHA1(e901b1abd11cc0f02dd6d87b429d8997f762c15d) )
ROM_END


ROM_START( astrians )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "astrians.7h",  0x0000, 0x0800, CRC(21eba3d0) SHA1(d07f141d785c86faca8c40af034c26f2789e9346) )
	ROM_LOAD( "astrians.7j",  0x0800, 0x0800, CRC(f3a436cd) SHA1(8d64e61b823e22f17cb79bf9e0c7b3c80c76413f) )
	ROM_LOAD( "astrians.7k",  0x1000, 0x0800, CRC(2915e38b) SHA1(045d4cc2c363b9ba8d066f902f03b7eacbeb1f5e) )
	ROM_LOAD( "astrians.7l",  0x1800, 0x0800, CRC(2db56b2f) SHA1(b15ce010560f3692d4254a93bff234b409697bac) )
	ROM_LOAD( "astrians.7m",  0x2000, 0x0800, CRC(41075efb) SHA1(2839981d9aab87167a6c8c3e9854028e3e11daaa) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "astrians.1h",  0x0000, 0x0800, CRC(77c074b4) SHA1(a33f8826ffd593454b72b21cf6d7be084d095a87) )
	ROM_LOAD( "astrians.1k",  0x0800, 0x0800, CRC(c30fcc46) SHA1(e99998271c750ffa436d83277bea9f07b840c880) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "prom.6l",      0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( moonaln )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "galx.u",       0x0000, 0x0800, CRC(79e4007d) SHA1(d55050498a670d1c022ba3caad34f8fcaccf4a30) ) // prg1.bin
	ROM_LOAD( "prg2.bin",     0x0800, 0x0800, CRC(59580b30) SHA1(e659426ad8c4e5e10a7cdd07d8b4fea93f875026) )
	ROM_LOAD( "prg3.bin",     0x1000, 0x0800, CRC(b64e9d12) SHA1(3b07902ea61388f54c03d65082e78dfc0fa8d3d2) )
	ROM_LOAD( "superg.y",     0x1800, 0x0800, CRC(9463f753) SHA1(d9cb35c19aafec43d08b048bbe2337a790f6ba9d) ) // prg4.bin
	ROM_LOAD( "prg5.bin",     0x2000, 0x0800, CRC(8bb78987) SHA1(5f24dba0bb31fc8bda5bf570d568472befc4d740) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "ca1.bin",   0x0000, 0x0800, CRC(074271dd) SHA1(cd6a40b493bc51c5340d7083f83c51834b95b5fe) )
	ROM_LOAD( "ca2.bin",   0x0800, 0x0800, CRC(84d90397) SHA1(93e6ded079c9721d3f9c003e378e8121584671c9) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END
ROM_START( superg )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "7f.bin",       0x0000, 0x1000, CRC(4335b1de) SHA1(e41e3d90dac738cf71377f3b476ec67b14dee27a) )
	ROM_LOAD( "superg.w",     0x1000, 0x0800, CRC(ddeabdae) SHA1(daa5109a32c7c9a80bdb212dc3e4e3e3c104a731) )
	ROM_LOAD( "superg.y",     0x1800, 0x0800, CRC(9463f753) SHA1(d9cb35c19aafec43d08b048bbe2337a790f6ba9d) )
	ROM_LOAD( "superg.z",     0x2000, 0x0800, CRC(e6312e35) SHA1(c4010459379d7fe00f605aaf288928b2deffb8b2) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "galmidw.1j",   0x0000, 0x0800, CRC(84decf98) SHA1(2e565cb6057b1816a6b4541e6dfadd3c3762fa36) )
	ROM_LOAD( "galmidw.1k",   0x0800, 0x0800, CRC(c31ada9e) SHA1(237ebb48549b34ca59a13cc2706512d957413ec4) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( galturbo )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "galturbo.u",   0x0000, 0x0800, CRC(e8f3aa67) SHA1(a0e9576784dbe602dd9780e667f01f31defd7c00) )
	ROM_LOAD( "galx.v",       0x0800, 0x0800, CRC(bc16064e) SHA1(4e3220fd63c8184bf9581a89dffb6944d8fae3bb) )
	ROM_LOAD( "superg.w",     0x1000, 0x0800, CRC(ddeabdae) SHA1(daa5109a32c7c9a80bdb212dc3e4e3e3c104a731) )
	ROM_LOAD( "galturbo.y",   0x1800, 0x0800, CRC(a44f450f) SHA1(4009834afb45e9b23c7cf058bcd3378ef8601872) )
	ROM_LOAD( "galturbo.z",   0x2000, 0x0800, CRC(3247f3d4) SHA1(5754dedc2d06736629d85514b2e7c262ce27bf2d) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "galturbo.1h",  0x0000, 0x0800, CRC(a713fd1a) SHA1(abf86fe5cb7243a1a36d7ac0a868577a3360dcca) )
	ROM_LOAD( "galturbo.1k",  0x0800, 0x0800, CRC(28511790) SHA1(dec2e183a753295d033a56184c973bbc810abf55) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( galapx )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "galx.u",       0x0000, 0x0800, CRC(79e4007d) SHA1(d55050498a670d1c022ba3caad34f8fcaccf4a30) )
	ROM_LOAD( "galx.v",       0x0800, 0x0800, CRC(bc16064e) SHA1(4e3220fd63c8184bf9581a89dffb6944d8fae3bb) )
	ROM_LOAD( "galx.w",       0x1000, 0x0800, CRC(72d2d3ee) SHA1(96e0c5824e46d7398c7e58dd6b75a9f4ead6f3f5) )
	ROM_LOAD( "galx.y",       0x1800, 0x0800, CRC(afe397f3) SHA1(283c6f3b3f07581d88f7a6e11fc36947a9d90e2e) )
	ROM_LOAD( "galx.z",       0x2000, 0x0800, CRC(778c0d3c) SHA1(6a81875abfea515d379c6212cb57f8e54573e943) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "galx.1h",      0x0000, 0x0800, CRC(e8810654) SHA1(b6924c7ad765c32714e6abd5bb56b2732edd5855) )
	ROM_LOAD( "galx.1k",      0x0800, 0x0800, CRC(cbe84a76) SHA1(c6d72fb452e8213dd40a2eb5dcca726d7cdca658) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( galap1 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "7f.bin",       0x0000, 0x1000, CRC(4335b1de) SHA1(e41e3d90dac738cf71377f3b476ec67b14dee27a) )
	ROM_LOAD( "galaxian.w",   0x1000, 0x0800, CRC(4c7031c0) SHA1(97f7ab0cedcd8eba1c8f6f516d84d672a2108258) )
	ROM_LOAD( "galx_1_4.rom", 0x1800, 0x0800, CRC(e71e1d9e) SHA1(32bf22b06c84d36de7c1280740b9c11e8d6a12b6) )
	ROM_LOAD( "galx_1_5.rom", 0x2000, 0x0800, CRC(6e65a3b2) SHA1(c9f20645ad2882e937245a9e90504423bb492158) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "galmidw.1j",   0x0000, 0x0800, CRC(84decf98) SHA1(2e565cb6057b1816a6b4541e6dfadd3c3762fa36) )
	ROM_LOAD( "galmidw.1k",   0x0800, 0x0800, CRC(c31ada9e) SHA1(237ebb48549b34ca59a13cc2706512d957413ec4) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( galap4 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "galnamco.u",   0x0000, 0x0800, CRC(acfde501) SHA1(4b72c1ffecaccadc541da2367f3ef70a2a9aed64) )
	ROM_LOAD( "galnamco.v",   0x0800, 0x0800, CRC(65cf3c77) SHA1(1c5249815816b395e1e04bf6a7dbb63e40faa0e3) )
	ROM_LOAD( "galnamco.w",   0x1000, 0x0800, CRC(9eef9ae6) SHA1(b2282e4edb8911e6aabfa936c3526f90381e1320) )
	ROM_LOAD( "galnamco.y",   0x1800, 0x0800, CRC(56a5ddd1) SHA1(1f87f647ebdffba28d5957f195448f6bce17f4d5) )
	ROM_LOAD( "galnamco.z",   0x2000, 0x0800, CRC(f4bc7262) SHA1(c4b70e474d49f45cec96f7c250bd77e01e18601a) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "galx_4c1.rom", 0x0000, 0x0800, CRC(d5e88ab4) SHA1(737a22e406fd0a97d10e93a2c91c3aa61aebbdef) )
	ROM_LOAD( "galx_4c2.rom", 0x0800, 0x0800, CRC(a57b83e4) SHA1(335d8674df1d237a4b83da00eb9aee346bc2e901) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( swarm )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "swarm1.bin",   0x0000, 0x0800, CRC(21eba3d0) SHA1(d07f141d785c86faca8c40af034c26f2789e9346) )
	ROM_LOAD( "swarm2.bin",   0x0800, 0x0800, CRC(f3a436cd) SHA1(8d64e61b823e22f17cb79bf9e0c7b3c80c76413f) )
	ROM_LOAD( "swarm3.bin",   0x1000, 0x0800, CRC(2915e38b) SHA1(045d4cc2c363b9ba8d066f902f03b7eacbeb1f5e) )
	ROM_LOAD( "swarm4.bin",   0x1800, 0x0800, CRC(8bbbf486) SHA1(84c975562c9c359069fb70f7f416420c74d40622) )
	ROM_LOAD( "swarm5.bin",   0x2000, 0x0800, CRC(f1b1987e) SHA1(0c8b57cb156fdd1a81a5e4535464cafab737185b) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "swarma.bin",   0x0000, 0x0800, CRC(ef8657bb) SHA1(c942db83231b04041e2794a08ce779331613edcf) )
	ROM_LOAD( "swarmb.bin",   0x0800, 0x0800, CRC(60c4bd31) SHA1(a8f22f8d7a9fca2c29091888e243dfa10211e138) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( zerotime )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "zt-p01c.016",  0x0000, 0x0800, CRC(90a2bc61) SHA1(9d23dfcf5310cf1d4aa1b473ec84279585e1a876) )
	ROM_LOAD( "zt-2.016",     0x0800, 0x0800, CRC(a433067e) SHA1(1aed1a2153c4a32a9996fc709e544f2063885599) )
	ROM_LOAD( "zt-3.016",     0x1000, 0x0800, CRC(aaf038d4) SHA1(2d070fe7c4e9b26092f0f12a9db3392f7d8a65f1) )
	ROM_LOAD( "zt-4.016",     0x1800, 0x0800, CRC(786d690a) SHA1(50c5c07941006e3b71afbf057d27daa2f2274925) )
	ROM_LOAD( "zt-5.016",     0x2000, 0x0800, CRC(af9260d7) SHA1(955e466a8989993351dc69d73ca322c1c9af7b63) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "ztc-2.016",    0x0000, 0x0800, CRC(1b13ca05) SHA1(6999068771dacc6bf6c17eb858af593a929d09af) )
	ROM_LOAD( "ztc-1.016",    0x0800, 0x0800, CRC(5cd7df03) SHA1(77873408c89546a17b1da3f64b7e96e314fadb17) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

// Late-to-market bootleg with PCB mods to use a single program rom
// Datamat is the old name of Datasat, a technical service and distributor of arcade PCB's from the 80's and 90's.
// A lot of the bootleg PCB's around Spain have Datamat stickers on the roms. It was one of the most important PCB sellers/distributors in the country from the era.
// Datamat still operate today as Datasat http://datasat.info/
ROM_START( zerotimed )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "zerotime_datamat.bin",  0x0000, 0x4000, CRC(be60834b) SHA1(426cb27a38fd99485481cb74c7372df8b7c8832a) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "ztc-2.016",    0x0000, 0x0800, CRC(1b13ca05) SHA1(6999068771dacc6bf6c17eb858af593a929d09af) )
	ROM_LOAD( "ztc-1.016",    0x0800, 0x0800, CRC(5cd7df03) SHA1(77873408c89546a17b1da3f64b7e96e314fadb17) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END


ROM_START( starfght )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "ja.1",         0x0000, 0x0400, CRC(c6ab558b) SHA1(2b707e332c57b9ec6a61220ab2b79ed5076d0628) )
	ROM_LOAD( "jb.2",         0x0400, 0x0400, CRC(34b99fed) SHA1(03d12b19c9aee75313cae6af602c93205d2fd4a8) )
	ROM_LOAD( "jc.3",         0x0800, 0x0400, CRC(30e28016) SHA1(07a621e5061d85a9559a920d76716ea4db61b674) )
	ROM_LOAD( "jd.4",         0x0c00, 0x0400, CRC(de7e7770) SHA1(b06043a1d898eb323ddabffd3d2a3b1f63df0e5e) )
	ROM_LOAD( "je.5",         0x1000, 0x0400, CRC(a916c919) SHA1(b3e264ff92687022a0f2f551d5df36db848b48eb) )
	ROM_LOAD( "jf.6",         0x1400, 0x0400, CRC(9175882b) SHA1(d9943efcb9245af7f01aecc533a699bdefc7d283) )
	ROM_LOAD( "jg.7",         0x1800, 0x0400, CRC(707c0f02) SHA1(4cfb18b8161ec6a74663b54120bdc6371ee9dbff) )
	ROM_LOAD( "jh.8",         0x1c00, 0x0400, CRC(5dd26461) SHA1(173b939287d0261ff069c277a1afd724133f4c88) )
	ROM_LOAD( "ji.9",         0x2000, 0x0400, CRC(6651fe93) SHA1(eb1d9466090ef723ae20003e5be27059f5bea57b) )
	ROM_LOAD( "jj.10",        0x2400, 0x0400, CRC(12c721b9) SHA1(1944cd5129115d245ced44da7f1eb4574561c457) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "k1.7a",        0x0000, 0x0800, CRC(977e37cf) SHA1(88ff1e4edadf5cfc83413a1fe999aecf4ba72232) )
	ROM_LOAD( "k2.9a",        0x0800, 0x0800, CRC(15e387ce) SHA1(d804b1391de5a15c336aa53c812b4a885f830191) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mmi6331.7f",   0x0000, 0x0020, CRC(24652bc4) SHA1(d89575f3749c75dc963317fe451ffeffd9856e4d) ) /* Compatible with 82s123 prom */
ROM_END

/* was marked 'star fighter' but doesn't appear to be the above game */
ROM_START( galaxbsf )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "1.bn",         0x0000, 0x0400, CRC(cc37b774) SHA1(5b7d9e3c896a1f5b0353732806568d4ffead3ead) )
	ROM_LOAD( "2.bn",         0x0400, 0x0400, CRC(c6d21f03) SHA1(64784915bf988fd2a3eea5f219c95c8498175018) )
	ROM_LOAD( "3.bn",         0x0800, 0x0400, CRC(30e28016) SHA1(07a621e5061d85a9559a920d76716ea4db61b674) )
	ROM_LOAD( "4.bn",         0x0c00, 0x0400, CRC(de7e7770) SHA1(b06043a1d898eb323ddabffd3d2a3b1f63df0e5e) )
	ROM_LOAD( "5.bn",         0x1000, 0x0400, CRC(a916c919) SHA1(b3e264ff92687022a0f2f551d5df36db848b48eb) )
	ROM_LOAD( "6.bn",         0x1400, 0x0400, CRC(9175882b) SHA1(d9943efcb9245af7f01aecc533a699bdefc7d283) )
	ROM_LOAD( "7.bn",         0x1800, 0x0400, CRC(1237b9da) SHA1(00e11532c599fca452a816683b361a24476b7100) )
	ROM_LOAD( "8.bn",         0x1c00, 0x0400, CRC(78c53607) SHA1(780acff57b594185eb5f4b24ae7d8b4992d96611) )
	ROM_LOAD( "9.bn",         0x2000, 0x0400, CRC(901894cc) SHA1(a189a8ab0068e9acc3be7b8e87adc1eadfd6b708) )
	ROM_LOAD( "10.bn",        0x2400, 0x0400, CRC(5876f695) SHA1(e8c0d13066cfe4a409293b9e1380513099b35330) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "11.bn",        0x0000, 0x0800, CRC(39fb43a4) SHA1(4755609bd974976f04855d51e08ec0d62ab4bc07) )
	ROM_LOAD( "12.bn",        0x0800, 0x0800, CRC(7e3f56a2) SHA1(a9795d8b7388f404f3b0e2c6ce15d713a4c5bafa) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( galaxbsf2 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "gal00eg.ic41",      0x0000, 0x0400, CRC(7c44510c) SHA1(76b0831cb42cae0d56176d549f223b75e8275308) )
	ROM_LOAD( "gal01eg.ic5",       0x0400, 0x0400, CRC(2a426500) SHA1(c6507a289833a751da7d6907c14bc6fcd4aafda6) )
	ROM_LOAD( "gal02.ic6",         0x0800, 0x0400, CRC(30e28016) SHA1(07a621e5061d85a9559a920d76716ea4db61b674) )
	ROM_LOAD( "gal03.ic7",         0x0c00, 0x0400, CRC(de7e7770) SHA1(b06043a1d898eb323ddabffd3d2a3b1f63df0e5e) )
	ROM_LOAD( "gal04.ic8",         0x1000, 0x0400, CRC(a916c919) SHA1(b3e264ff92687022a0f2f551d5df36db848b48eb) )
	ROM_LOAD( "gal05.ic9",         0x1400, 0x0400, CRC(9175882b) SHA1(d9943efcb9245af7f01aecc533a699bdefc7d283) )
	ROM_LOAD( "gal06.ic10",        0x1800, 0x0400, CRC(1237b9da) SHA1(00e11532c599fca452a816683b361a24476b7100) )
	ROM_LOAD( "gal07eg.ic11",      0x1c00, 0x0400, CRC(16144658) SHA1(2195814579d511c290b9d0cfe7386e2c24827627) )
	ROM_LOAD( "gal08.ic12",        0x2000, 0x0400, CRC(901894cc) SHA1(a189a8ab0068e9acc3be7b8e87adc1eadfd6b708) )
	ROM_LOAD( "gal09.ic13",        0x2400, 0x0400, CRC(5876f695) SHA1(e8c0d13066cfe4a409293b9e1380513099b35330) )

	ROM_REGION( 0x0400, "unknown", 0 )
	ROM_LOAD( "gal00eg.ic4",       0x0000, 0x0400, CRC(1038467f) SHA1(e34cc53a1203335cf9c9a94c3f96cab5a444a34a) ) // sldh - the first 0x100 bytes of this is ic41, the rest is different? should it bank in somehow to give extra features??

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "galaxian.1h",       0x0000, 0x0800, CRC(39fb43a4) SHA1(4755609bd974976f04855d51e08ec0d62ab4bc07) )
	ROM_LOAD( "galaxian.1k",       0x0800, 0x0800, CRC(7e3f56a2) SHA1(a9795d8b7388f404f3b0e2c6ce15d713a4c5bafa) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6l.bpr",            0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END


ROM_START( galaxianbl ) // looks to be a fairly plain set with modified bonus lives etc.
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "gal00eg.ic4",       0x0000, 0x0400, CRC(7c44510c) SHA1(76b0831cb42cae0d56176d549f223b75e8275308) )
	ROM_LOAD( "gal01eg.ic5",       0x0400, 0x0400, CRC(2a426500) SHA1(c6507a289833a751da7d6907c14bc6fcd4aafda6) )
	ROM_LOAD( "gal02.ic6",         0x0800, 0x0400, CRC(30e28016) SHA1(07a621e5061d85a9559a920d76716ea4db61b674) )
	ROM_LOAD( "gal03.ic7",         0x0c00, 0x0400, CRC(de7e7770) SHA1(b06043a1d898eb323ddabffd3d2a3b1f63df0e5e) )
	ROM_LOAD( "gal04.ic8",         0x1000, 0x0400, CRC(a916c919) SHA1(b3e264ff92687022a0f2f551d5df36db848b48eb) )
	ROM_LOAD( "gal05.ic9",         0x1400, 0x0400, CRC(9175882b) SHA1(d9943efcb9245af7f01aecc533a699bdefc7d283) )
	ROM_LOAD( "gal06.ic10",        0x1800, 0x0400, CRC(1237b9da) SHA1(00e11532c599fca452a816683b361a24476b7100) )
	ROM_LOAD( "gal07eg.ic11",      0x1c00, 0x0400, CRC(16144658) SHA1(2195814579d511c290b9d0cfe7386e2c24827627) )
	ROM_LOAD( "gal08.ic12",        0x2000, 0x0400, CRC(901894cc) SHA1(a189a8ab0068e9acc3be7b8e87adc1eadfd6b708) )
	ROM_LOAD( "gal09.ic13",        0x2400, 0x0400, CRC(5876f695) SHA1(e8c0d13066cfe4a409293b9e1380513099b35330) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "galaxian.1h",       0x0000, 0x0800, CRC(39fb43a4) SHA1(4755609bd974976f04855d51e08ec0d62ab4bc07) )
	ROM_LOAD( "galaxian.1k",       0x0800, 0x0800, CRC(7e3f56a2) SHA1(a9795d8b7388f404f3b0e2c6ce15d713a4c5bafa) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6l.bpr",            0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( kamakazi3 ) /* Hack of Video Games (UK) Ltd. version???? flyer spells it Kamakaze III, also no year or (c) */
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "f_r_a.bin",    0x0000, 0x0800, CRC(e8f3aa67) SHA1(a0e9576784dbe602dd9780e667f01f31defd7c00) )
	ROM_LOAD( "f_a.bin",      0x0800, 0x0800, CRC(f58283e3) SHA1(edc6e72516c50fd3402281d9936574d276581ce9) )
	ROM_LOAD( "f_b.bin",      0x1000, 0x0800, CRC(ddeabdae) SHA1(daa5109a32c7c9a80bdb212dc3e4e3e3c104a731) )
	ROM_LOAD( "f_r_c.bin",    0x1800, 0x0800, CRC(c8530a88) SHA1(b8856af80cdae7430c05239875bffdc5d67aab98) )
	ROM_LOAD( "f_r_d.bin",    0x2000, 0x0800, CRC(da2d77e0) SHA1(aa96fb8c6401c443b7e767f6d08713bf9e1af103) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "r_b.bin",      0x0000, 0x0800, CRC(977e37cf) SHA1(88ff1e4edadf5cfc83413a1fe999aecf4ba72232) )
	ROM_LOAD( "r_a.bin",      0x0800, 0x0800, CRC(d0ba22c9) SHA1(678b22d10e1ae7dcea068da838bf6bd648e9ee28) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( supergx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sg1",          0x0000, 0x0800, CRC(b83f4578) SHA1(9a5d5fc291839f7f1e0a52cca7bea29e99c13315) )
	ROM_LOAD( "sg2",          0x0800, 0x0800, CRC(d12ca054) SHA1(8eb7f6904c3c650bfa80908a5988622d5e693bd1) )
	ROM_LOAD( "sg3",          0x1000, 0x0800, CRC(53714cb1) SHA1(7dffcd3ced1c3354339bb69477f8aa4c708708db) )
	ROM_LOAD( "sg4",          0x1800, 0x0800, CRC(2f36fc69) SHA1(d310dcb0a79b03ee26b0575db9cba6d920cb9273) )
	ROM_LOAD( "sg5",          0x2000, 0x0800, CRC(1e0ed4fd) SHA1(183d8990dbff1954921f8c5b67cec09f2d380794) )
	ROM_LOAD( "sg6",          0x2800, 0x0800, BAD_DUMP CRC(4f3d97a8) SHA1(b9fcab182ab57e8374fef93f7fd314a155a8d04d) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sgg1",         0x0000, 0x0800, CRC(a1287bf6) SHA1(eeeaba4b9e186454a5e2f1c26e333e8fccd97af8) )
	ROM_RELOAD(               0x0800, 0x0800 )
	ROM_LOAD( "sgg2",         0x1000, 0x0800, CRC(528f1481) SHA1(e266a75c3109bcfa2a0394f2ed0ac136fc3158ba) )
	ROM_RELOAD(               0x1800, 0x0800 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "supergx.prm",  0x0000, 0x0020, NO_DUMP )
ROM_END

ROM_START( tst_galx )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "test.u",       0x0000, 0x0800, CRC(0614cd7f) SHA1(12440678be8a27a6c3032b6e43c45e27905ffa83) )   /*  The Test ROM */
	ROM_LOAD( "galmidw.v",    0x0800, 0x0800, CRC(9c999a40) SHA1(02fdcd95d8511e64c0d2b007b874112d53e41045) )
	ROM_LOAD( "galmidw.w",    0x1000, 0x0800, CRC(b5894925) SHA1(0046b9ed697a34d088de1aead8bd7cbe526a2396) )
	ROM_LOAD( "galmidw.y",    0x1800, 0x0800, CRC(6b3ca10b) SHA1(18d8714e5ef52f63ba8888ecc5a25b17b3bf17d1) )

	ROM_LOAD( "7l",           0x2000, 0x0800, CRC(1b933207) SHA1(8b44b0f74420871454e27894d0f004859f9e59a9) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "1h.bin",       0x0000, 0x0800, CRC(39fb43a4) SHA1(4755609bd974976f04855d51e08ec0d62ab4bc07) )
	ROM_LOAD( "1k.bin",       0x0800, 0x0800, CRC(7e3f56a2) SHA1(a9795d8b7388f404f3b0e2c6ce15d713a4c5bafa) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END


ROM_START( blkhole )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bh1",          0x0000, 0x0800, CRC(64998819) SHA1(69fe5dfbe6cde18ef4cae62da12b5c692c2c72b9) )
	ROM_LOAD( "bh2",          0x0800, 0x0800, CRC(26f26ce4) SHA1(720ce7af05ef596fb9a109591534c74d282955e8) )
	ROM_LOAD( "bh3",          0x1000, 0x0800, CRC(3418bc45) SHA1(088bbbde66b7b5c36fa48cf14c22146e1444e67c) )
	ROM_LOAD( "bh4",          0x1800, 0x0800, CRC(735ff481) SHA1(d9b32db048a0e2a1195cd6f7326005e6622242a9) )
	ROM_LOAD( "bh5",          0x2000, 0x0800, CRC(3f657be9) SHA1(3ed1ee0bc199c1625156d2771eecd18a57a0e6ed) )
	ROM_LOAD( "bh6",          0x2800, 0x0800, CRC(a057ab35) SHA1(430261bafe20fc182e6e6659019cf42643e95d54) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "bh7",          0x0000, 0x0800, CRC(975ba821) SHA1(c50d55f6ab81b803d67f5e18c1243ef85a1a2df1) )
	ROM_LOAD( "bh8",          0x0800, 0x0800, CRC(03d11020) SHA1(5768b573fac9aac168db2723462cca76d4d80552) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( orbitron )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "orbitron.3",   0x0600, 0x0200, CRC(419f9c9b) SHA1(788a3920f4270b886b3a578f8c2df33e6314a1c3) )
	ROM_CONTINUE(             0x0400, 0x0200)
	ROM_CONTINUE(             0x0200, 0x0200)
	ROM_CONTINUE(             0x0000, 0x0200)
	ROM_LOAD( "orbitron.4",   0x0e00, 0x0200, CRC(44ad56ac) SHA1(3a8339cdee50912a16ac0fb448e6659e32542c0c) )
	ROM_CONTINUE(             0x0c00, 0x0200)
	ROM_CONTINUE(             0x0a00, 0x0200)
	ROM_CONTINUE(             0x0800, 0x0200)
	ROM_LOAD( "orbitron.1",   0x1600, 0x0200, CRC(da3f5168) SHA1(1927cc7cd3b9d15b629e09781557f4c75d684182) )
	ROM_CONTINUE(             0x1400, 0x0200)
	ROM_CONTINUE(             0x1200, 0x0200)
	ROM_CONTINUE(             0x1000, 0x0200)
	ROM_LOAD( "orbitron.2",   0x1e00, 0x0200, CRC(a3b813fc) SHA1(7f0f22667bee897b474fb485d65a74d74a36991a) )
	ROM_CONTINUE(             0x1c00, 0x0200)
	ROM_CONTINUE(             0x1a00, 0x0200)
	ROM_CONTINUE(             0x1800, 0x0200)
	ROM_LOAD( "orbitron.5",   0x2000, 0x0800, CRC(20cd8bb8) SHA1(a5309cb04a656c6e1e18bb19910474af8ef814a5) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "orbitron.6",   0x0000, 0x0800, CRC(2c91b83f) SHA1(29c73b7ad0dc5a3ba739492c902ad9201eae6ef2) )
	ROM_LOAD( "orbitron.7",   0x0800, 0x0800, CRC(46f4cca4) SHA1(e5fb616b1d17b5b5167f05f7840638840deb2d13) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "l06_prom.bin", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( luctoday )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ltprog1.bin", 0x0000, 0x0800, CRC(59c389b9) SHA1(1e158ced3b56db2c51e422fb4c0b8893565f1956))
	ROM_LOAD( "ltprog2.bin", 0x2000, 0x0800, CRC(ac3893b1) SHA1(f6b9cd8111b367ff7030cba52fe965959d92568f))

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "ltchar2.bin", 0x0000, 0x0800, CRC(8cd73bdc) SHA1(6174f7347d2c96f9c5074bc0da5a370c9b07461b))
	ROM_LOAD( "ltchar1.bin", 0x0800, 0x0800, CRC(b5ba9946) SHA1(7222cbe8c41ca74b214f4dd5439bf69d90f4644e))

	ROM_REGION( 0x0020, "proms", 0 )//This may not be the correct prom
	ROM_LOAD( "74s288.ch", 0x0000, 0x0020, BAD_DUMP CRC(24652bc4) SHA1(d89575f3749c75dc963317fe451ffeffd9856e4d))
ROM_END

ROM_START( chewing )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin", 0x0000, 0x1000, CRC(7470b347) SHA1(315d2631b50a6e469b9538318d95452e8d2e1f69) )
	ROM_LOAD( "7l.bin", 0x2000, 0x0800, CRC(78ebed36) SHA1(e80185737c8ac448901cf0e60ca50d967c323b34) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "2.bin", 0x0000, 0x0800, CRC(88c605f3) SHA1(938a9fadfa0994a1d2fc9b3266ec4ccdb5ec6d3a) )
	ROM_LOAD( "3.bin", 0x0800, 0x0800, CRC(77ac016a) SHA1(fa5b1e79603ca8d2ee7b3d0a78f12d9ffeec3fd4) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "74s288.ch", 0x0000, 0x0020, CRC(24652bc4) SHA1(d89575f3749c75dc963317fe451ffeffd9856e4d) )
ROM_END

ROM_START( catacomb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "catacomb.u",    0x0000, 0x0800, CRC(35cc28d2) SHA1(e1dbd75fc21ec88b8119bf9508c87d78e1d5c4f6) )
	ROM_LOAD( "catacomb.v",    0x0800, 0x0800, CRC(1d1ce133) SHA1(e22a169003a2238004bdf6c2558198216c2353b7) )
	ROM_LOAD( "catacomb.w",    0x1000, 0x0800, CRC(479bbde7) SHA1(9981662cb6351de7c1730de45f645fb0e26ea467) )
	/* no .x */
	ROM_LOAD( "catacomb.y",    0x2000, 0x0800, CRC(5e3da534) SHA1(a9b960ae96c8ef0b2d590bc58b711aad949025e2) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "cat-gfx1",       0x0000, 0x0800, CRC(e871e65c) SHA1(0b528dfab0f57153db9406798848cdedee0323a0) )
	ROM_LOAD( "cat-gfx2",       0x0800, 0x0800, CRC(b14dafaa) SHA1(592d5931a76563b3565f22ac4c0120b9a120193f) )

	ROM_REGION( 0x0020, "proms", 0 )
	/* No color PROM came with the conversion - the Moon Cresta one seems more appropriate than Galaxian,
	   (the game is unplayable with a Galaxian PROM) but which was intended for use with the kit is unclear */
	ROM_LOAD( "mmi6331.6l", 0x0000, 0x0020, BAD_DUMP CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( omega )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "omega1.bin",   0x0000, 0x0800, CRC(fc2a096b) SHA1(071ff30060a1aa0a47ae6e88140b80caed00fc4e) )
	ROM_LOAD( "omega2.bin",   0x0800, 0x0800, CRC(ad100357) SHA1(7c5e82c25e65b4a390cf5607f15bf4df407f7f11) )
	ROM_LOAD( "omega3.bin",   0x1000, 0x0800, CRC(d7e3be79) SHA1(ffa228043c6c717bee8bbec16432dcfe2e348aef) )
	ROM_LOAD( "omega4.bin",   0x1800, 0x0800, CRC(42068171) SHA1(940ca30a5772940b8a437498d22c6121482b38e6) )
	ROM_LOAD( "omega5.bin",   0x2000, 0x0800, CRC(d8a93383) SHA1(5f60f127360b14206d4df638e528bf961049e37d) )
	ROM_LOAD( "omega6.bin",   0x2800, 0x0800, CRC(32a42f44) SHA1(94f458997ec279dce218a17b665fa8c46067e646) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "omega1h.bin",  0x0000, 0x0800, CRC(527fd384) SHA1(92a384899d5acd2c689f637da16a0e2d11a9d9c6) )
	ROM_LOAD( "omega1k.bin",  0x0800, 0x0800, CRC(36de42c6) SHA1(6fd93d439e3b8eab62049f925d9e8f8deeda2ae3) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mmi6331-1j.86",   0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END


ROM_START( warofbug )  /* Shows 20c as the base of currency.  I know of no US games that used dimes - unless it's another country? */
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "warofbug.u",   0x0000, 0x0800, CRC(b8dfb7e3) SHA1(c7c675b2638869a9cd7dbd554e6131d8c71b567a) )
	ROM_LOAD( "warofbug.v",   0x0800, 0x0800, CRC(fd8854e0) SHA1(b39ab41b834f18341968dd780f0a3cd07d70c16c) )
	ROM_LOAD( "warofbug.w",   0x1000, 0x0800, CRC(4495aa14) SHA1(f1be281db1d831770efa9cc41ea87eb348e70108) )
	ROM_LOAD( "warofbug.y",   0x1800, 0x0800, CRC(c14a541f) SHA1(d32e89fd18d9e1db2e4a545186eac728c0b02255) )
	ROM_LOAD( "warofbug.z",   0x2000, 0x0800, CRC(c167fe55) SHA1(d85c4d1bd7aa5e14eb2f11dfa14979e5dbc084a8) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "warofbug.1k",  0x0000, 0x0800, CRC(8100fa85) SHA1(06641c431cace36dec98b87555f62e72f3e53a31) )
	ROM_LOAD( "warofbug.1j",  0x0800, 0x0800, CRC(d1220ae9) SHA1(e892bc8b0b71d8b07503e474e9c30e6cab460682) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "warofbug.clr", 0x0000, 0x0020, CRC(8688e64b) SHA1(ed13414257f580b98b50c9892a14159c55e7838d) )
ROM_END


ROM_START( warofbugg )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "wotbg-u-1.bin",   0x0000, 0x0800, CRC(f43ff0a8) SHA1(b87abeb8af9105fa8fba78f9a68363bd89066e7f) )
	ROM_LOAD( "wotbg-v-2.bin",   0x0800, 0x0800, CRC(eb7a028b) SHA1(8c822ae11d3cc04f749a7cd639d15b9fc830ab35) )
	ROM_LOAD( "wotbg-w-3.bin",   0x1000, 0x0800, CRC(693e0e50) SHA1(00b19969cee0f95bfb8251c2df133ff2c9ae3b00) )
	ROM_LOAD( "wotbg-y-4.bin",   0x1800, 0x0800, CRC(885d4982) SHA1(4aeaf514a9413a9cb9a971fd258c6cf46ca66fc4) )
	ROM_LOAD( "wotbg-z-5.bin",   0x2000, 0x0800, CRC(60041ef2) SHA1(cced5837a037ac5cd8fa6260d69d8e33de5ecd48) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "warofbug.1k",  0x0000, 0x0800, CRC(8100fa85) SHA1(06641c431cace36dec98b87555f62e72f3e53a31) )
	ROM_LOAD( "warofbug.1j",  0x0800, 0x0800, CRC(d1220ae9) SHA1(e892bc8b0b71d8b07503e474e9c30e6cab460682) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "warofbug.clr", 0x0000, 0x0020, CRC(8688e64b) SHA1(ed13414257f580b98b50c9892a14159c55e7838d) )
ROM_END

// has a large custom block on the ROM board
ROM_START( spactrai )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1cen.bin",   0x0000, 0x1000, CRC(fabc7fd8) SHA1(88c42dda38cc79ab4f180c4818cfb928c1cc0661) )
	ROM_LOAD( "2cen.bin",   0x1000, 0x1000, CRC(44ddacfa) SHA1(50a9f5f3e4ec12fd3742dcf7cf141e52300a10db) )
	ROM_LOAD( "3cen.bin",   0x2000, 0x1000, CRC(822749cb) SHA1(92e617088d462911118842f3f68b7ff8ac77fcf5) )
	ROM_LOAD( "4cen.bin",   0x3000, 0x1000, CRC(f9dda0ed) SHA1(a77f6d8ec7b3df7f308354489954c3d9b4f61b0d) )
	ROM_LOAD( "5cen.bin",   0x4000, 0x1000, CRC(b8c76675) SHA1(acdda20adf62d1e2eadcc097ecde6a3126231415) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "6cen.bin",  0x0000, 0x0800, CRC(a59a9f3f) SHA1(9564f1d013d566dc0b19762aec66119e2ece0b49) ) // MK2716J
	ROM_LOAD( "7cen.bin",  0x0800, 0x0400, CRC(16cf5a5b) SHA1(0369786902544d31e506fe1ba6a69aa6e8ba9b5c) ) // MM2758A

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "stk.bin", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END


/*  Galaxian hardware mods for War of the Bugs warofbug/warofbugg

(WotB using daughterboard that plugs into Z80 socket - has a socketed PAL, two other 20-pin ICs,
a 16-pin IC (all with their markings sanded off) and five EPROMs)

On the Galaxian board:

2B, 74LS366 - cut pin 15 OR pin 1 (this disables the stars)
8E, 74LS139 - cut pin 11 only, join the stub left ON THE CHIP to pin 13 on the chip

Cut the track on the bottom of the board going to pin 21 of IC 1K
(2716 eprom).

Cut the track on the bottom of the board going to pin 21 of IC 1H
(2716 eprom).

Join pins 21 of IC's 1H and 1K together and connect both to +5 volts
(although I've seen one set of docs saying to connect to pin 1 of
ic 2N (7408) instead)  */


ROM_START( warofbugu )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "wb-prog-1.7d",   0x0000, 0x0800, CRC(b8dfb7e3) SHA1(c7c675b2638869a9cd7dbd554e6131d8c71b567a) )
	ROM_LOAD( "wb-prog-2.7e",   0x0800, 0x0800, CRC(fd8854e0) SHA1(b39ab41b834f18341968dd780f0a3cd07d70c16c) )
	ROM_LOAD( "wb-prog-3.7j",   0x1000, 0x0800, CRC(4495aa14) SHA1(f1be281db1d831770efa9cc41ea87eb348e70108) )
	ROM_LOAD( "wb-prog-4.7n",   0x1800, 0x0800, CRC(e4bd257c) SHA1(e9a26a50cbc76bb339dfbf3d2817229fe9ff7fc4) )
	ROM_LOAD( "wb-prog-5.7p",   0x2000, 0x0800, CRC(71257bb4) SHA1(3624becfda4e080795a15428a51dcda261f91210) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "warofbug.1k",  0x0000, 0x0800, CRC(8100fa85) SHA1(06641c431cace36dec98b87555f62e72f3e53a31) ) // wb-vid-1.1j
	ROM_LOAD( "warofbug.1j",  0x0800, 0x0800, CRC(d1220ae9) SHA1(e892bc8b0b71d8b07503e474e9c30e6cab460682) ) // wb-vid-2.1l

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "warofbug.clr", 0x0000, 0x0020, CRC(8688e64b) SHA1(ed13414257f580b98b50c9892a14159c55e7838d) )
ROM_END


ROM_START( redufo ) /* Galaxian bootleg hardware known as Artic Multi-System */
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "redufo.1",  0x0000, 0x0800, CRC(6a3b873c) SHA1(82f07921d8f1da3ed8b4f99b8052bd7e2cefcb6a) )
	ROM_LOAD( "redufo.2",  0x0800, 0x0800, CRC(202eda3b) SHA1(5ca7b50fc510950cd6cce6f27573b5c491171bf0) )
	ROM_LOAD( "redufo.3",  0x1000, 0x0800, CRC(bf7030e8) SHA1(59b0624dd91527a916ee6a27d61def82c3c14f49) )
	ROM_LOAD( "redufo.4",  0x1800, 0x0800, CRC(8c1c2ef9) SHA1(3beec82c67d8e26ecd988be77efb8599a4741d4d) )
	ROM_LOAD( "redufo.5",  0x2000, 0x0800, CRC(ef965b24) SHA1(2e2e0ef2b2940660092c27f46ae76c9320136f17) )
	ROM_LOAD( "redufo.6",  0x2800, 0x0800, CRC(58b3e39b) SHA1(9b081154c90f22c17315c8bc2a47993468367768) )
	ROM_LOAD( "redufo.7",  0x3000, 0x0800, CRC(fd07d811) SHA1(6b968a7ce452f76a8d26fe694aa4ea6b16e8b6fa) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "redufo.8",  0x0000, 0x0800, CRC(b34c7cb4) SHA1(146ed4a02d7540378f4a27a6643055216ad403f7) )
	ROM_LOAD( "redufo.9",  0x0800, 0x0800, CRC(50a2d447) SHA1(1f97d1096ad2a3a43a480cb1f040f4534fada3c3) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "tbp18s030n.6l", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( redufob )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "ru1a",         0x0000, 0x0800, CRC(5a8e4f37) SHA1(c0957ede91e2dc3f80e4912b877843aed5d15779) )
	ROM_LOAD( "ru2a",         0x0800, 0x0800, CRC(c624f52d) SHA1(119a660513ad33e35c9bdaecd588219bf8026d82) )
	ROM_LOAD( "ru3a",         0x1000, 0x0800, CRC(e1030d1c) SHA1(80640fbbfa7f84c016366b1084e7f8a7acdcd440) )
	ROM_LOAD( "ru4a",         0x1800, 0x0800, CRC(7692069e) SHA1(5130d61c857c3b85eadabcf10f3a6771c72f0f56) )
	ROM_LOAD( "ru5a",         0x2000, 0x0800, CRC(cb648ff3) SHA1(e0042251ca7f4a31b5bd9f8cca35278a1e152899) )
	ROM_LOAD( "ru6a",         0x2800, 0x0800, CRC(e1a9f58e) SHA1(4fc7489fca057156a7cf5efcb01058ce4f0db69e) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "ruhja",        0x0000, 0x0800, CRC(8a422b0d) SHA1(b886157518f73e7115a225ba230e456179f6e18f) )
	ROM_LOAD( "rukla",        0x0800, 0x0800, CRC(1eb84cb1) SHA1(08f360802a90039c0499a1417d06b6eb5f89d67e) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( exodus )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "exodus1.bin",  0x0000, 0x0800, CRC(5dfe65e1) SHA1(5f1ce289b3c98a89d61d4dea952b4b8888d92ed7) )
	ROM_LOAD( "exodus2.bin",  0x0800, 0x0800, CRC(6559222f) SHA1(520497f6fb2b0c76be8419702e8af894283ebf0b) )
	ROM_LOAD( "exodus3.bin",  0x1000, 0x0800, CRC(bf7030e8) SHA1(59b0624dd91527a916ee6a27d61def82c3c14f49) )
	ROM_LOAD( "exodus4.bin",  0x1800, 0x0800, CRC(3607909e) SHA1(93d074fe4b258d496a0998acb3fc47f0a762227a) )
	ROM_LOAD( "exodus9.bin",  0x2000, 0x0800, CRC(994a90c4) SHA1(a07e3ce8f69042c45ebe00ab1d40dbb85602a7a2) )
	ROM_LOAD( "exodus10.bin", 0x2800, 0x0800, CRC(fbd11187) SHA1(a3bd49c4a79e76b08e6b343b94689159dc239458) )
	ROM_LOAD( "exodus11.bin", 0x3000, 0x0800, CRC(fd07d811) SHA1(6b968a7ce452f76a8d26fe694aa4ea6b16e8b6fa) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "exodus5.bin",  0x0000, 0x0800, CRC(b34c7cb4) SHA1(146ed4a02d7540378f4a27a6643055216ad403f7) )
	ROM_LOAD( "exodus6.bin",  0x0800, 0x0800, CRC(50a2d447) SHA1(1f97d1096ad2a3a43a480cb1f040f4534fada3c3) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "l06_prom.bin", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( tdpgal )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin",  0x0000, 0x0800, CRC(7be819fe) SHA1(ab5a332914546692f9976e243daf3040f0d9952e) )
	ROM_LOAD( "2.bin",  0x0800, 0x0800, CRC(70c83a5e) SHA1(7b781b027c73d6c3901c6d27901f97fae61a352d) )
	ROM_LOAD( "3.bin",  0x1000, 0x0800, CRC(475eb5a0) SHA1(b7436873fd74aff3577540474420a1abaa2babcb) )
	ROM_LOAD( "4.bin",  0x1800, 0x0800, CRC(20a71943) SHA1(e196eea20bd0518545dcde61f2697a1d6f61568b) )
	ROM_LOAD( "5.bin",  0x2000, 0x0800, CRC(72c4f2dc) SHA1(30a9c69afd2c4da4a14363fc0b17b1e0da188927) )
	ROM_LOAD( "6.bin",  0x2800, 0x0800, CRC(fa4e2be4) SHA1(a30d43189660f8a1437faf87064fce28c9c760ad) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "8.bin",         0x0000, 0x0800, CRC(d701b1d4) SHA1(8f23ba3c9f42fedd8b8f38d321118a86889f00dc) )
	ROM_LOAD( "7.bin",         0x0800, 0x0800, CRC(3113bcfd) SHA1(e2792e5fe7d7f27bb329e3104dee3ca29d72ef48) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "bprom.bin",       0x0000, 0x0020, CRC(2b4cf53f) SHA1(8d7eb0453173b9821eea32419b67559bfb4578d0) )
ROM_END


ROM_START( azurian )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pgm.1",        0x0000, 0x1000, CRC(17a0fca7) SHA1(0ffb80d433fbaa0631d0d982a453f9e6cccab297) )
	ROM_LOAD( "pgm.2",        0x1000, 0x1000, CRC(14659848) SHA1(bb9d9c01b074bf7ed7a1c29379bbef41728dd27a) )
	ROM_LOAD( "pgm.3",        0x2000, 0x1000, CRC(8f60fb97) SHA1(d0f4d65e568ac1a5d41e550f2f626cbf72884959) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "gfx.1",        0x0000, 0x0800, CRC(f5afb803) SHA1(ffc8f86a35179d7715ef618004b79003e0236a93) )
	ROM_LOAD( "gfx.2",        0x0800, 0x0800, CRC(ae96e5d1) SHA1(df667fb96d7353ccf9ce0acf788371ef2221e97d) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "l06_prom.bin", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( pisces )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "p1.bin",       0x0000, 0x0800, CRC(40c5b0e4) SHA1(6c18e6f4719eb0d7eb13b778d7ea58e4b87ac35c) )
	ROM_LOAD( "p2.bin",       0x0800, 0x0800, CRC(055f9762) SHA1(9d821874dd48a80651adc58a2f7fe5d2b3ed67bc) )
	ROM_LOAD( "p3.bin",       0x1000, 0x0800, CRC(3073dd04) SHA1(b93913a988f412d565abd19dc668976585cc8066) )
	ROM_LOAD( "p4.bin",       0x1800, 0x0800, CRC(44aaf525) SHA1(667bf4c3a36169c3ddddd22b2f1f90bcc9308548) )
	ROM_LOAD( "p5.bin",       0x2000, 0x0800, CRC(fade512b) SHA1(ccef2650f1d9dc3fdde2d441774246d47febc2cc) )
	ROM_LOAD( "p6.bin",       0x2800, 0x0800, CRC(5ab2822f) SHA1(bbcac3aab943dd9b173de11ddf02ff75d16b1582) )

	ROM_REGION( 0x2000, "gfx1", 0 )
//  ROM_LOAD( "pisces.1j",    0x0000, 0x1000, CRC(2dba9e0e) )
//  ROM_LOAD( "pisces.1k",    0x1000, 0x1000, CRC(cdc5aa26) )
	ROM_LOAD( "g09.bin",      0x0000, 0x0800, CRC(9503a23a) SHA1(23848de56841dd1de9ef74d5a9c981c784098175) )
	ROM_LOAD( "g11.bin",      0x0800, 0x0800, CRC(0adfc3fe) SHA1(a4da488632d9906066db45ae62747caf5ffbf2d8) )
	ROM_LOAD( "g10.bin",      0x1000, 0x0800, CRC(3e61f849) SHA1(efa0059bc843af0c3bb94f4bc0a8286ca5069179) )
	ROM_LOAD( "g12.bin",      0x1800, 0x0800, CRC(7130e9eb) SHA1(e6bb7a9b4f2fc001296e1060d0671b7a88599c8b) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "colour.bin",   0x0000, 0x0020, CRC(57a45057) SHA1(d4ec6a54f72640e5b485aa59e206c090e67ff640) )    // same as checkman.clr
ROM_END

ROM_START( piscesb )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "pisces.a1",    0x0000, 0x0800, CRC(856b8e1f) SHA1(24d468b5f06f54c3fa1cb54ceec8a0c8e285430e) )
	ROM_LOAD( "p2.bin",       0x0800, 0x0800, CRC(055f9762) SHA1(9d821874dd48a80651adc58a2f7fe5d2b3ed67bc) )
	ROM_LOAD( "pisces.b2",    0x1000, 0x0800, CRC(5540f2e4) SHA1(b069a7e46fa2c1f732371ef056caaf8f343e11a8) )
	ROM_LOAD( "p4.bin",       0x1800, 0x0800, CRC(44aaf525) SHA1(667bf4c3a36169c3ddddd22b2f1f90bcc9308548) )
	ROM_LOAD( "p5.bin",       0x2000, 0x0800, CRC(fade512b) SHA1(ccef2650f1d9dc3fdde2d441774246d47febc2cc) )
	ROM_LOAD( "p6.bin",       0x2800, 0x0800, CRC(5ab2822f) SHA1(bbcac3aab943dd9b173de11ddf02ff75d16b1582) )

	ROM_REGION( 0x2000, "gfx1", 0 )
//  ROM_LOAD( "pisces.1j",    0x0000, 0x1000, CRC(2dba9e0e) )
//  ROM_LOAD( "pisces.1k",    0x1000, 0x1000, CRC(cdc5aa26) )
	ROM_LOAD( "g09.bin",      0x0000, 0x0800, CRC(9503a23a) SHA1(23848de56841dd1de9ef74d5a9c981c784098175) )
	ROM_LOAD( "g11.bin",      0x0800, 0x0800, CRC(0adfc3fe) SHA1(a4da488632d9906066db45ae62747caf5ffbf2d8) )
	ROM_LOAD( "g10.bin",      0x1000, 0x0800, CRC(3e61f849) SHA1(efa0059bc843af0c3bb94f4bc0a8286ca5069179) )
	ROM_LOAD( "g12.bin",      0x1800, 0x0800, CRC(7130e9eb) SHA1(e6bb7a9b4f2fc001296e1060d0671b7a88599c8b) )

	ROM_REGION( 0x0020, "proms", 0 )
//  ROM_LOAD( "6331-1j.86",   0x0000, 0x0020, CRC(24652bc4) SHA1(d89575f3749c75dc963317fe451ffeffd9856e4d) ) /* very close to Galaxian */
	ROM_LOAD( "colour.bin",   0x0000, 0x0020, CRC(57a45057) SHA1(d4ec6a54f72640e5b485aa59e206c090e67ff640) )    // same as checkman.clr
ROM_END

ROM_START( omni )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "omni1.7f",     0x0000, 0x1000, CRC(a9b7acc6) SHA1(0c6319957b760fea3cfa6c29b37c25f5a89a6d77) )
	ROM_LOAD( "omni2.7j",     0x1000, 0x1000, CRC(6ade29b7) SHA1(64f1ce82c761db11d26c385299a7063f5971c99a) )
	ROM_LOAD( "omni3.7f",     0x2000, 0x1000, CRC(9e37bb24) SHA1(d90b2ff0297d87687561e1e9b29510b6c051760b) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "omni5b.l1",    0x0000, 0x0800, CRC(9503a23a) SHA1(23848de56841dd1de9ef74d5a9c981c784098175) )
	ROM_LOAD( "omni6c.j22",   0x0800, 0x0800, CRC(0adfc3fe) SHA1(a4da488632d9906066db45ae62747caf5ffbf2d8) )
	ROM_LOAD( "omni4a.j1",    0x1000, 0x0800, CRC(3e61f849) SHA1(efa0059bc843af0c3bb94f4bc0a8286ca5069179) )
	ROM_LOAD( "omni7d.l2",    0x1800, 0x0800, CRC(7130e9eb) SHA1(e6bb7a9b4f2fc001296e1060d0671b7a88599c8b) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "colour.bin",   0x0000, 0x0020, CRC(57a45057) SHA1(d4ec6a54f72640e5b485aa59e206c090e67ff640) )
ROM_END

ROM_START( uniwars )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "f07_1a.bin",   0x0000, 0x0800, CRC(d975af10) SHA1(a2e2a36a75db8fd09441308b08b6ae073c68b8cf) )
	ROM_LOAD( "h07_2a.bin",   0x0800, 0x0800, CRC(b2ed14c3) SHA1(7668df11f64b8e296eedfee53437777dc53a56d5) )
	ROM_LOAD( "k07_3a.bin",   0x1000, 0x0800, CRC(945f4160) SHA1(5fbe879f51e14c4c7ae551e5b3089f8e148770a4) )
	ROM_LOAD( "m07_4a.bin",   0x1800, 0x0800, CRC(ddc80bc5) SHA1(18c3920198baf87267bc7f12db6b23b090d3577a) )
	ROM_LOAD( "d08p_5a.bin",  0x2000, 0x0800, CRC(62354351) SHA1(85bf18942f73023b8be0c3659a0dcd3dfcccfc2c) )
	ROM_LOAD( "gg6",          0x2800, 0x0800, CRC(270a3f4d) SHA1(20f5097033fca515d70fe47178cbd341a1d07443) )
	ROM_LOAD( "m08p_7a.bin",  0x3000, 0x0800, CRC(c9245346) SHA1(239bad3fe64eaab2dfc3febd06d1124103a10504) )
	ROM_LOAD( "n08p_8a.bin",  0x3800, 0x0800, CRC(797d45c7) SHA1(76fb8b45fcce3622c59c04af32cfa001ef7bf71d) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "egg10",        0x0000, 0x0800, CRC(012941e0) SHA1(4f7ec4d95939cb7c4086bb7df43759ac504ae47c) )
	ROM_LOAD( "h01_2.bin",    0x0800, 0x0800, CRC(c26132af) SHA1(7ae125a911dfd47aeca4f129f580762ce4d8d91a) )
	ROM_LOAD( "egg9",         0x1000, 0x0800, CRC(fc8b58fd) SHA1(72553e2735b0dcc2dcfce9698d49566732492588) )
	ROM_LOAD( "k01_2.bin",    0x1800, 0x0800, CRC(dcc2b33b) SHA1(c3a5ac935c519400dfabb28909f7e460769d1837) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "uniwars.clr",  0x0000, 0x0020, CRC(25c79518) SHA1(e8f7e8b3d0cf1ed9d723948548f58abf0e2c6d1f) )
ROM_END

ROM_START( spacempr )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "uw01",   0x0000, 0x0800, CRC(7c64fb92) SHA1(69f0923870cb8cbb7ae7a2a056c67a1da9b5588d) )
	ROM_LOAD( "uw02",   0x0800, 0x0800, CRC(b2ed14c3) SHA1(7668df11f64b8e296eedfee53437777dc53a56d5) )
	ROM_LOAD( "uw03",   0x1000, 0x0800, CRC(945f4160) SHA1(5fbe879f51e14c4c7ae551e5b3089f8e148770a4) )
	ROM_LOAD( "uw04",   0x1800, 0x0800, CRC(84885060) SHA1(a6ea3e272b426e86ff87e95c765362b44506228e) )
	ROM_LOAD( "uw05",   0x2000, 0x0800, CRC(e342371d) SHA1(f53caf7793df5788237d5e2f35242c0dd7a3085b) )
	ROM_LOAD( "uw06",   0x2800, 0x0800, CRC(d915a389) SHA1(0e2ff6eec9453856a1276889946b463cfae58eba) )
	ROM_LOAD( "uw07",   0x3000, 0x0800, CRC(c9245346) SHA1(239bad3fe64eaab2dfc3febd06d1124103a10504) )
	ROM_LOAD( "uw08",   0x3800, 0x0800, CRC(a237c394) SHA1(66dfa2aa39bd19f1f6ddb267d8f8bdbdba750d46) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "uw10",   0x0000, 0x0800, CRC(af069cba) SHA1(12b7d0a57f43613c80afd51c417628090740aabe) )
	ROM_LOAD( "uw12",   0x0800, 0x0800, CRC(c26132af) SHA1(7ae125a911dfd47aeca4f129f580762ce4d8d91a) )
	ROM_LOAD( "uw09",   0x1000, 0x0800, CRC(ff2c20d5) SHA1(48668dc4f008f44f5c15bdcc331cfe133da99cd4) )
	ROM_LOAD( "uw11",   0x1800, 0x0800, CRC(dcc2b33b) SHA1(c3a5ac935c519400dfabb28909f7e460769d1837) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mmi6331",  0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( gteikoku )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "f07_1a.bin",   0x0000, 0x0800, CRC(d975af10) SHA1(a2e2a36a75db8fd09441308b08b6ae073c68b8cf) )
	ROM_LOAD( "h07_2a.bin",   0x0800, 0x0800, CRC(b2ed14c3) SHA1(7668df11f64b8e296eedfee53437777dc53a56d5) )
	ROM_LOAD( "k07_3a.bin",   0x1000, 0x0800, CRC(945f4160) SHA1(5fbe879f51e14c4c7ae551e5b3089f8e148770a4) )
	ROM_LOAD( "m07_4a.bin",   0x1800, 0x0800, CRC(ddc80bc5) SHA1(18c3920198baf87267bc7f12db6b23b090d3577a) )
	ROM_LOAD( "d08p_5a.bin",  0x2000, 0x0800, CRC(62354351) SHA1(85bf18942f73023b8be0c3659a0dcd3dfcccfc2c) )
	ROM_LOAD( "e08p_6a.bin",  0x2800, 0x0800, CRC(d915a389) SHA1(0e2ff6eec9453856a1276889946b463cfae58eba) )
	ROM_LOAD( "m08p_7a.bin",  0x3000, 0x0800, CRC(c9245346) SHA1(239bad3fe64eaab2dfc3febd06d1124103a10504) )
	ROM_LOAD( "n08p_8a.bin",  0x3800, 0x0800, CRC(797d45c7) SHA1(76fb8b45fcce3622c59c04af32cfa001ef7bf71d) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "h01_1.bin",    0x0000, 0x0800, CRC(8313c959) SHA1(b09157c6f824d6e94647728cbb329877fcb4e502) )
	ROM_LOAD( "h01_2.bin",    0x0800, 0x0800, CRC(c26132af) SHA1(7ae125a911dfd47aeca4f129f580762ce4d8d91a) )
	ROM_LOAD( "k01_1.bin",    0x1000, 0x0800, CRC(c9d4537e) SHA1(65d27066ffec04b755d2f5d3f36f5ec6792e8d6c) )
	ROM_LOAD( "k01_2.bin",    0x1800, 0x0800, CRC(dcc2b33b) SHA1(c3a5ac935c519400dfabb28909f7e460769d1837) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "l06_prom.bin", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( gteikokb )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "1.bin",        0x0000, 0x0800, CRC(bf00252f) SHA1(a4ec48c6b9468f52bcf8b01d1bdb908dcf81d42d) )
	ROM_LOAD( "2.bin",        0x0800, 0x0800, CRC(f712b7d5) SHA1(c269db2e9984a3fbd33888bd426c53d319cad36f) )
	ROM_LOAD( "k07_3a.bin",   0x1000, 0x0800, CRC(945f4160) SHA1(5fbe879f51e14c4c7ae551e5b3089f8e148770a4) )
	ROM_LOAD( "4.bin",        0x1800, 0x0800, CRC(808a39a8) SHA1(f3db5175d0c2d10e9e3ded400888f6541490597e) )
	ROM_LOAD( "5.bin",        0x2000, 0x0800, CRC(36fe6e67) SHA1(e54a19ad6611fefcdfcf74019a63cc6cea6cf433) )
	ROM_LOAD( "6.bin",        0x2800, 0x0800, CRC(c5ea67e8) SHA1(0157eb2ef5ab56cd00e5f4fafd618271d2d4862b) )
	ROM_LOAD( "m08p_7a.bin",  0x3000, 0x0800, CRC(c9245346) SHA1(239bad3fe64eaab2dfc3febd06d1124103a10504) )
	ROM_LOAD( "8.bin",        0x3800, 0x0800, CRC(28df3229) SHA1(fd307c6a7de4fcddce1c2f36a957a31b9a6aaa21) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "h01_1.bin",    0x0000, 0x0800, CRC(8313c959) SHA1(b09157c6f824d6e94647728cbb329877fcb4e502) )
	ROM_LOAD( "h01_2.bin",    0x0800, 0x0800, CRC(c26132af) SHA1(7ae125a911dfd47aeca4f129f580762ce4d8d91a) )
	ROM_LOAD( "k01_1.bin",    0x1000, 0x0800, CRC(c9d4537e) SHA1(65d27066ffec04b755d2f5d3f36f5ec6792e8d6c) )
	ROM_LOAD( "k01_2.bin",    0x1800, 0x0800, CRC(dcc2b33b) SHA1(c3a5ac935c519400dfabb28909f7e460769d1837) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "l06_prom.bin", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( gteikob2 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "94gnog.bin",   0x0000, 0x0800, CRC(67ec3235) SHA1(f250db867257f474f693012c11008bf92f038cc7) )
	ROM_LOAD( "92gnog.bin",   0x0800, 0x0800, CRC(813c41f2) SHA1(bd92e0b53e3c8874d63f3444bca02246cd74b1c6) )
	ROM_LOAD( "k07_3a.bin",   0x1000, 0x0800, CRC(945f4160) SHA1(5fbe879f51e14c4c7ae551e5b3089f8e148770a4) )
	ROM_LOAD( "1gnog.bin",    0x1800, 0x0800, CRC(49ff9658) SHA1(3b7f3dc40b3fbc7d4abe5f5d534951c70409148c) )
	ROM_LOAD( "5.bin",        0x2000, 0x0800, CRC(36fe6e67) SHA1(e54a19ad6611fefcdfcf74019a63cc6cea6cf433) )
	ROM_LOAD( "e08p_6a.bin",  0x2800, 0x0800, CRC(d915a389) SHA1(0e2ff6eec9453856a1276889946b463cfae58eba) )
	ROM_LOAD( "m08p_7a.bin",  0x3000, 0x0800, CRC(c9245346) SHA1(239bad3fe64eaab2dfc3febd06d1124103a10504) )
	ROM_LOAD( "98gnog.bin",   0x3800, 0x0800, CRC(e9d4ad3c) SHA1(b32b96bebbf59e23b06958f6b16790e9f9f334e2) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "h01_1.bin",    0x0000, 0x0800, CRC(8313c959) SHA1(b09157c6f824d6e94647728cbb329877fcb4e502) )
	ROM_LOAD( "h01_2.bin",    0x0800, 0x0800, CRC(c26132af) SHA1(7ae125a911dfd47aeca4f129f580762ce4d8d91a) )
	ROM_LOAD( "k01_1.bin",    0x1000, 0x0800, CRC(c9d4537e) SHA1(65d27066ffec04b755d2f5d3f36f5ec6792e8d6c) )
	ROM_LOAD( "k01_2.bin",    0x1800, 0x0800, CRC(dcc2b33b) SHA1(c3a5ac935c519400dfabb28909f7e460769d1837) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "l06_prom.bin", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( galemp )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "1",  0x0000, 0x0800, CRC(d975af10) SHA1(a2e2a36a75db8fd09441308b08b6ae073c68b8cf) )
	ROM_LOAD( "2",  0x0800, 0x0800, CRC(b2ed14c3) SHA1(7668df11f64b8e296eedfee53437777dc53a56d5) )
	ROM_LOAD( "3",  0x1000, 0x0800, CRC(945f4160) SHA1(5fbe879f51e14c4c7ae551e5b3089f8e148770a4) )
	ROM_LOAD( "4",  0x1800, 0x0800, CRC(df7a13ea) SHA1(9d07cdfde84c9243719092234cfa362bf2878a95) )
	ROM_LOAD( "5",  0x2000, 0x0800, CRC(ff6128a2) SHA1(64cc17b6cab8d8fd8358840beb13baa76262c6aa) )
	ROM_LOAD( "6",  0x2800, 0x0800, CRC(d915a389) SHA1(0e2ff6eec9453856a1276889946b463cfae58eba) )
	ROM_LOAD( "7",  0x3000, 0x0800, CRC(c9245346) SHA1(239bad3fe64eaab2dfc3febd06d1124103a10504) )
	ROM_LOAD( "8",  0x3800, 0x0800, CRC(797d45c7) SHA1(76fb8b45fcce3622c59c04af32cfa001ef7bf71d) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "10",    0x0000, 0x0800, CRC(30177b93) SHA1(b91740b573eadb9a0df23f55594d22b10ea93555) )
	ROM_LOAD( "12",    0x0800, 0x0800, CRC(c26132af) SHA1(7ae125a911dfd47aeca4f129f580762ce4d8d91a) )
	ROM_LOAD( "9",     0x1000, 0x0800, CRC(7e8dcc13) SHA1(56450cb3a9c77a578a12f664d07dbfbbcb82bc07) )
	ROM_LOAD( "11",    0x1800, 0x0800, CRC(dcc2b33b) SHA1(c3a5ac935c519400dfabb28909f7e460769d1837) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "l06_prom.bin", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( asideral )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "1401.7f",  0x0000, 0x0800, CRC(82a9da91) SHA1(864127f73b4a78435766ff70010663c4d789c472) )
	ROM_LOAD( "1302.7h",  0x0800, 0x0800, CRC(cc59b49c) SHA1(13f6cb3a979f1b9baa4fd07c3e36c17ad8e69b5f) )
	ROM_LOAD( "1203.7k",  0x1000, 0x0800, CRC(3bc5a165) SHA1(0ad0247f6499170a828bf343f7710c1036b241a7) )
	ROM_LOAD( "1104.7m",  0x1800, 0x0800, CRC(c50149d0) SHA1(4007d7c07fe2742d67d2041b4b3c2f3fcaedbc8b) )
	ROM_LOAD( "5.8f",     0x2000, 0x0800, CRC(17720c9e) SHA1(c2b59b95c3936e46202e5512a8d117ac86d1be2d) )
	ROM_LOAD( "6.8f",     0x2800, 0x0800, CRC(f157a8db) SHA1(75fa76d5dad149502c25d191053041a52dd1562e) )
	ROM_LOAD( "7.8f",     0x3000, 0x0800, CRC(75085cb6) SHA1(92c4f375352685ec670b0aa96becce064c5d9bce) )
	ROM_LOAD( "8.8f",     0x3800, 0x0800, CRC(797d45c7) SHA1(76fb8b45fcce3622c59c04af32cfa001ef7bf71d) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "u10.j2",    0x0000, 0x0800, CRC(012941e0) SHA1(4f7ec4d95939cb7c4086bb7df43759ac504ae47c) )
	ROM_LOAD( "u12.j2",    0x0800, 0x0800, CRC(c26132af) SHA1(7ae125a911dfd47aeca4f129f580762ce4d8d91a) )
	ROM_LOAD( "u9.l2",     0x1000, 0x0800, CRC(fc8b58fd) SHA1(72553e2735b0dcc2dcfce9698d49566732492588) )
	ROM_LOAD( "u11.l2",    0x1800, 0x0800, CRC(dcc2b33b) SHA1(c3a5ac935c519400dfabb28909f7e460769d1837) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "uniwars.clr",  0x0000, 0x0020, CRC(25c79518) SHA1(e8f7e8b3d0cf1ed9d723948548f58abf0e2c6d1f) )
ROM_END

ROM_START( pajaroes ) // VERY similar to the asideral set
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "pea.rom",  0x0000, 0x0800, CRC(82a9da91) SHA1(864127f73b4a78435766ff70010663c4d789c472) )
	ROM_LOAD( "peb.rom",  0x0800, 0x0800, CRC(cc59b49c) SHA1(13f6cb3a979f1b9baa4fd07c3e36c17ad8e69b5f) )
	ROM_LOAD( "pec.rom",  0x1000, 0x0800, CRC(3bc5a165) SHA1(0ad0247f6499170a828bf343f7710c1036b241a7) )
	ROM_LOAD( "ped.rom",  0x1800, 0x0800, CRC(c50149d0) SHA1(4007d7c07fe2742d67d2041b4b3c2f3fcaedbc8b) )
	ROM_LOAD( "pe05.rom", 0x2000, 0x0800, CRC(cb461871) SHA1(3865f3b7f47f314a097dddfcc49929bb63afd4fc) )
	ROM_LOAD( "pe04.rom", 0x2800, 0x0800, CRC(f157a8db) SHA1(75fa76d5dad149502c25d191053041a52dd1562e) )
	ROM_LOAD( "pe03.rom", 0x3000, 0x0800, CRC(75085cb6) SHA1(92c4f375352685ec670b0aa96becce064c5d9bce) )
	ROM_LOAD( "pe02.rom", 0x3800, 0x0800, CRC(797d45c7) SHA1(76fb8b45fcce3622c59c04af32cfa001ef7bf71d) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pe07.rom",    0x0000, 0x0800, CRC(012941e0) SHA1(4f7ec4d95939cb7c4086bb7df43759ac504ae47c) )
	ROM_LOAD( "pe09.rom",    0x0800, 0x0800, CRC(c26132af) SHA1(7ae125a911dfd47aeca4f129f580762ce4d8d91a) )
	ROM_LOAD( "pe06.rom",    0x1000, 0x0800, CRC(fc8b58fd) SHA1(72553e2735b0dcc2dcfce9698d49566732492588) )
	ROM_LOAD( "pe08.rom",    0x1800, 0x0800, CRC(dcc2b33b) SHA1(c3a5ac935c519400dfabb28909f7e460769d1837) )

	ROM_REGION( 0x0020, "proms", 0 ) // wasn't in the set
	ROM_LOAD( "uniwars.clr",  0x0000, 0x0020, CRC(25c79518) SHA1(e8f7e8b3d0cf1ed9d723948548f58abf0e2c6d1f) )
ROM_END



ROM_START( spacbatt )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "sb1",    0x0000, 0x0800, CRC(d975af10) SHA1(a2e2a36a75db8fd09441308b08b6ae073c68b8cf) ) /* Same as f07_1a.bin above */
	ROM_LOAD( "sb2",    0x0800, 0x0800, CRC(b2ed14c3) SHA1(7668df11f64b8e296eedfee53437777dc53a56d5) ) /* Same as h07_2a.bin above */
	ROM_LOAD( "sb3",    0x1000, 0x0800, CRC(945f4160) SHA1(5fbe879f51e14c4c7ae551e5b3089f8e148770a4) ) /* Same as k07_3a.bin above */
	ROM_LOAD( "sb4",    0x1800, 0x0800, CRC(8229835c) SHA1(8cfd8f6cab6f80ca69645a184f7e841fc69f47f6) )
	ROM_LOAD( "sb5",    0x2000, 0x0800, CRC(f51ef930) SHA1(213e68571a0c7d5ba33a7170d5fa4aea898ea0b9) )
	ROM_LOAD( "sb6",    0x2800, 0x0800, CRC(d915a389) SHA1(0e2ff6eec9453856a1276889946b463cfae58eba) ) /* Same as e08p_6a.bin above */
	ROM_LOAD( "sb7",    0x3000, 0x0800, CRC(c9245346) SHA1(239bad3fe64eaab2dfc3febd06d1124103a10504) ) /* Same as m08p_7a.bin above */
	ROM_LOAD( "sb8",    0x3800, 0x0800, CRC(e59ff1ae) SHA1(fef22885cbd3273882f8c7755dd04c28e843b9ea) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sb12",   0x0000, 0x0800, CRC(8313c959) SHA1(b09157c6f824d6e94647728cbb329877fcb4e502) ) /* Same as h01_1.bin above */
	ROM_LOAD( "sb14",   0x0800, 0x0800, CRC(c26132af) SHA1(7ae125a911dfd47aeca4f129f580762ce4d8d91a) ) /* Same as h01_2.bin above */
	ROM_LOAD( "sb11",   0x1000, 0x0800, CRC(c9d4537e) SHA1(65d27066ffec04b755d2f5d3f36f5ec6792e8d6c) ) /* Same as k01_1.bin above */
	ROM_LOAD( "sb13",   0x1800, 0x0800, CRC(92454380) SHA1(f0cd67b39c760c2b5ac549b27b0a5f83fbb3a86b) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6l.bpr", 0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) ) /* MMI 6331 bp-prom, compatible with 82s123 */
ROM_END

ROM_START( spacbat2 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "sb1",    0x0000, 0x0800, CRC(d975af10) SHA1(a2e2a36a75db8fd09441308b08b6ae073c68b8cf) ) /* Same as f07_1a.bin above */
	ROM_LOAD( "sb2",    0x0800, 0x0800, CRC(b2ed14c3) SHA1(7668df11f64b8e296eedfee53437777dc53a56d5) ) /* Same as h07_2a.bin above */
	ROM_LOAD( "sb.3",   0x1000, 0x0800, CRC(c25ce4c1) SHA1(d7a5d435df7868155523d2fb90f331d4b6d9eaa1) )
	ROM_LOAD( "sb4",    0x1800, 0x0800, CRC(8229835c) SHA1(8cfd8f6cab6f80ca69645a184f7e841fc69f47f6) )
	ROM_LOAD( "sb5",    0x2000, 0x0800, CRC(f51ef930) SHA1(213e68571a0c7d5ba33a7170d5fa4aea898ea0b9) )
	ROM_LOAD( "sb6",    0x2800, 0x0800, CRC(d915a389) SHA1(0e2ff6eec9453856a1276889946b463cfae58eba) ) /* Same as e08p_6a.bin above */
	ROM_LOAD( "sb7",    0x3000, 0x0800, CRC(c9245346) SHA1(239bad3fe64eaab2dfc3febd06d1124103a10504) ) /* Same as m08p_7a.bin above */
	ROM_LOAD( "sb8",    0x3800, 0x0800, CRC(e59ff1ae) SHA1(fef22885cbd3273882f8c7755dd04c28e843b9ea) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sb12",      0x0000, 0x0800, CRC(8313c959) SHA1(b09157c6f824d6e94647728cbb329877fcb4e502) ) /* Same as h01_1.bin above */
	ROM_LOAD( "sb14",      0x0800, 0x0800, CRC(c26132af) SHA1(7ae125a911dfd47aeca4f129f580762ce4d8d91a) ) /* Same as h01_2.bin above */
	ROM_LOAD( "sb11",      0x1000, 0x0800, CRC(c9d4537e) SHA1(65d27066ffec04b755d2f5d3f36f5ec6792e8d6c) ) /* Same as k01_1.bin above */
	ROM_LOAD( "k01_2.bin", 0x1800, 0x0800, CRC(dcc2b33b) SHA1(c3a5ac935c519400dfabb28909f7e460769d1837) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "l06_prom.bin", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( skyraidr )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "f07_1a.bin",   0x0000, 0x0800, CRC(d975af10) SHA1(a2e2a36a75db8fd09441308b08b6ae073c68b8cf) )
	ROM_LOAD( "h07_2a.bin",   0x0800, 0x0800, CRC(b2ed14c3) SHA1(7668df11f64b8e296eedfee53437777dc53a56d5) )
	ROM_LOAD( "k07_3a.bin",   0x1000, 0x0800, CRC(945f4160) SHA1(5fbe879f51e14c4c7ae551e5b3089f8e148770a4) )
	ROM_LOAD( "sr.04",        0x1800, 0x0800, CRC(9f61d1f8) SHA1(389b0a0d1a577b302907b2ea4c119aa18a6120d9) )
	ROM_LOAD( "sr.05",        0x2000, 0x0800, CRC(4352af0a) SHA1(1b31846ea7025aaf3a79141dfa5a089b8d12d982) )
	ROM_LOAD( "sr.06",        0x2800, 0x0800, CRC(d915a389) SHA1(0e2ff6eec9453856a1276889946b463cfae58eba) )
	ROM_LOAD( "m08p_7a.bin",  0x3000, 0x0800, CRC(c9245346) SHA1(239bad3fe64eaab2dfc3febd06d1124103a10504) )
	ROM_LOAD( "n08p_8a.bin",  0x3800, 0x0800, CRC(797d45c7) SHA1(76fb8b45fcce3622c59c04af32cfa001ef7bf71d) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sr.10",        0x0000, 0x0800, CRC(af069cba) SHA1(12b7d0a57f43613c80afd51c417628090740aabe) )
	ROM_LOAD( "h01_2.bin",    0x0800, 0x0800, CRC(c26132af) SHA1(7ae125a911dfd47aeca4f129f580762ce4d8d91a) )
	ROM_LOAD( "sr.09",        0x1000, 0x0800, CRC(ff2c20d5) SHA1(48668dc4f008f44f5c15bdcc331cfe133da99cd4) )
	ROM_LOAD( "k01_2.bin",    0x1800, 0x0800, CRC(dcc2b33b) SHA1(c3a5ac935c519400dfabb28909f7e460769d1837) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "uniwars.clr",  0x0000, 0x0020, CRC(25c79518) SHA1(e8f7e8b3d0cf1ed9d723948548f58abf0e2c6d1f) )
ROM_END


ROM_START( devilfsg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dfish1.7f",    0x2000, 0x0800, CRC(2ab19698) SHA1(8450981d3cf3fa8abf2fb5487aa98b03a4cf03a1) )
	ROM_CONTINUE(             0x0000, 0x0800 )
	ROM_LOAD( "dfish2.7h",    0x2800, 0x0800, CRC(4e77f097) SHA1(aeaa5ff210ccbbe77114edf5dee992d2720636ae) )
	ROM_CONTINUE(             0x0800, 0x0800 )
	ROM_LOAD( "dfish3.7k",    0x3000, 0x0800, CRC(3f16a4c6) SHA1(cc30b27070a12c250cdc2f7289bae7c7a4c05c2c) )
	ROM_CONTINUE(             0x1000, 0x0800 )
	ROM_LOAD( "dfish4.7m",    0x3800, 0x0800, CRC(11fc7e59) SHA1(2c0182a75bfca085e67483b421f40b3bc9b8ef24) )
	ROM_CONTINUE(             0x1800, 0x0800 )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "dfish5.1h",    0x0000, 0x0800, CRC(ace6e31f) SHA1(23df890fdf8ef275af79e10c8e43ff3a617b28ac) )
	ROM_CONTINUE(             0x0000, 0x0800 )
	ROM_LOAD( "dfish6.1k",    0x0800, 0x0800, CRC(d7a6c4c4) SHA1(ec5f9182657edb11884ab93f868f1bb3569461ae) )
	ROM_CONTINUE(             0x0800, 0x0800 )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "dfish5.1h",    0x0000, 0x0800, CRC(ace6e31f) SHA1(23df890fdf8ef275af79e10c8e43ff3a617b28ac) )
	ROM_IGNORE(                       0x0800 )
	ROM_LOAD( "dfish6.1k",    0x0800, 0x0800, CRC(d7a6c4c4) SHA1(ec5f9182657edb11884ab93f868f1bb3569461ae) )
	ROM_IGNORE(                       0x0800 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.6e",    0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )
ROM_END


ROM_START( zigzagb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "zz_d1.7l",    0x0000, 0x1000, CRC(8cc08d81) SHA1(be671192ef06dc3ed6963dc39e6bdce3275300e9) )
	ROM_LOAD( "zz_d2.7k",    0x1000, 0x1000, CRC(326d8d45) SHA1(563b9fc64c34e36cfadffb107ce30d3a04d62d9c) )
	ROM_LOAD( "zz_d4.7f",    0x2000, 0x1000, CRC(a94ed92a) SHA1(d56f32fc2b3f0f7affe658b7726682c60d09bc16) )
	ROM_LOAD( "zz_d3.7h",    0x3000, 0x1000, CRC(ce5e7a00) SHA1(93c47d22698a016cb0f0b654ade9ccab0cd1c88b) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "zz_6.1h",     0x0000, 0x0800, CRC(780c162a) SHA1(b0cac68258281917bcada52ce26e0ce38721d633) )
	ROM_IGNORE(                      0x0800 )
	ROM_LOAD( "zz_5.1k",     0x0800, 0x0800, CRC(f3cdfec5) SHA1(798d631c72d8e6b2e372b4b3ab0c10d8365a1359) )
	ROM_IGNORE(                      0x0800 )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "zz_6.1h",     0x0000, 0x0800, CRC(780c162a) SHA1(b0cac68258281917bcada52ce26e0ce38721d633) )
	ROM_CONTINUE(            0x0000, 0x0800 )
	ROM_LOAD( "zz_5.1k",     0x0800, 0x0800, CRC(f3cdfec5) SHA1(798d631c72d8e6b2e372b4b3ab0c10d8365a1359) )
	ROM_CONTINUE(            0x0800, 0x0800 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "zzbpr_e9.bin",0x0000, 0x0020, CRC(aa486dd0) SHA1(b845b52715bf6361ceee8c1ac541733963bd47af) )
ROM_END

ROM_START( zigzagb2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "z1.7l",       0x0000, 0x1000, CRC(4c28349a) SHA1(646134ce506deaee88cc2ec5a973f8fedaddb66b) )
	ROM_LOAD( "zz_d2.7k",    0x1000, 0x1000, CRC(326d8d45) SHA1(563b9fc64c34e36cfadffb107ce30d3a04d62d9c) )
	ROM_LOAD( "zz_d4.7f",    0x2000, 0x1000, CRC(a94ed92a) SHA1(d56f32fc2b3f0f7affe658b7726682c60d09bc16) )
	ROM_LOAD( "zz_d3.7h",    0x3000, 0x1000, CRC(ce5e7a00) SHA1(93c47d22698a016cb0f0b654ade9ccab0cd1c88b) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "zz_6.1h",     0x0000, 0x0800, CRC(780c162a) SHA1(b0cac68258281917bcada52ce26e0ce38721d633) )
	ROM_IGNORE(                      0x0800 )
	ROM_LOAD( "zz_5.1k",     0x0800, 0x0800, CRC(f3cdfec5) SHA1(798d631c72d8e6b2e372b4b3ab0c10d8365a1359) )
	ROM_IGNORE(                      0x0800 )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "zz_6.1h",     0x0000, 0x0800, CRC(780c162a) SHA1(b0cac68258281917bcada52ce26e0ce38721d633) )
	ROM_CONTINUE(            0x0000, 0x0800 )
	ROM_LOAD( "zz_5.1k",     0x0800, 0x0800, CRC(f3cdfec5) SHA1(798d631c72d8e6b2e372b4b3ab0c10d8365a1359) )
	ROM_CONTINUE(            0x0800, 0x0800 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "zzbpr_e9.bin",0x0000, 0x0020, CRC(aa486dd0) SHA1(b845b52715bf6361ceee8c1ac541733963bd47af) )
ROM_END


ROM_START( gmgalax )
	ROM_REGION( 0x18000, "maincpu", 0 ) /* 64k for code + 32k for banked code */
	ROM_LOAD( "pcb1_pm1.bin", 0x10000, 0x1000, CRC(19338c70) SHA1(cc2665b7d534d324627d12025ee099ff415d4214) )
	ROM_LOAD( "pcb1_pm2.bin", 0x11000, 0x1000, CRC(18db074d) SHA1(a70ed18f632e947493e648e6fc057dfb7a2a3322) )
	ROM_LOAD( "pcb1_pm3.bin", 0x12000, 0x1000, CRC(abb98b1d) SHA1(bb0109d353359bb192a3e6856a857c2f842838cb) )
	ROM_LOAD( "pcb1_pm4.bin", 0x13000, 0x1000, CRC(2403c78e) SHA1(52d8c8a4efcf47871485080ab217098a019e6579) )
	ROM_LOAD( "pcb1_gx1.bin", 0x14000, 0x1000, CRC(2faa9f53) SHA1(1e7010d407601c5da1adc68bc9f4742c79d57286) )
	ROM_LOAD( "pcb1_gx2.bin", 0x15000, 0x1000, CRC(121c5f16) SHA1(cb1806fa984870133fd883969838dca85f992515) )
	ROM_LOAD( "pcb1_gx3.bin", 0x16000, 0x1000, CRC(02d81a21) SHA1(39209cfb7cf142a65e157544d93803ea542a8efb) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "pcb2gfx1.bin", 0x0000, 0x0800, CRC(7021bbc0) SHA1(52d2983d74e722fccb31eb02ca56255850c4f41c) )
	ROM_CONTINUE(             0x1000, 0x0800 )
	ROM_LOAD( "pcb2gfx3.bin", 0x0800, 0x0800, CRC(089c922b) SHA1(f1b81999f63677d4cd58cd547353170e348a1423) )
	ROM_CONTINUE(             0x1800, 0x0800 )
	ROM_LOAD( "pcb2gfx2.bin", 0x2000, 0x0800, CRC(51bf58ee) SHA1(3546ff03c76a6422b0515bd5c695674bfb032089) )
	ROM_CONTINUE(             0x3000, 0x0800 )
	ROM_LOAD( "pcb2gfx4.bin", 0x2800, 0x0800, CRC(908fd0dc) SHA1(ac278bd82730e92ff312793244340748b93fa9bb) )
	ROM_CONTINUE(             0x3800, 0x0800 )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "gmgalax2.clr", 0x0000, 0x0020, CRC(499f4440) SHA1(66d6463a145087041934bdab8bfa6c3db6375317) )
	ROM_LOAD( "l06_prom.bin", 0x0020, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END


/*************************************
 *
 *  ROM definitions
 *  Artic Multi-System (Galaxian bootleg hardware)
 *  About 20 games were available on this system, some unique!
 *
 *************************************/

ROM_START( streakng )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sk1",          0x0000, 0x1000, CRC(c8866ccb) SHA1(1fc8bc643ecbfa86a50448d79b299f5a3dd586c5) )
	ROM_LOAD( "sk2",          0x1000, 0x1000, CRC(7caea29b) SHA1(5b3946ee914b1637db9046abf92d66ceaeb4fc5f) )
	ROM_LOAD( "sk3",          0x2000, 0x1000, CRC(7b4bfa76) SHA1(9223bec0c1cc39bc84670869b2a4fab0d0167c6e) )
	ROM_LOAD( "sk4",          0x3000, 0x1000, CRC(056fc921) SHA1(de8525571e5a82ddf74dd57b1a6c5bc9f2d2c0fe) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "sk5",          0x0000, 0x1000, CRC(d27f1e0c) SHA1(c3b4ae55a93516b034a16c9f943b360b24c933d6) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "sk6",          0x0000, 0x1000, CRC(a7089588) SHA1(e76242b043b1d8f060f669da3ddeee3d10122cdb) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "sk.bpr",       0x0000, 0x0020, CRC(bce79607) SHA1(49d60fde149240bcd025f721b0fbbbdbc549a42f) )
ROM_END

/*
sk2          [1/2]      st3.BIN                 IDENTICAL
sk3          [1/2]      5st.BIN                 IDENTICAL
sk6          [1/2]      9.BIN                   IDENTICAL
sk5          [1/2]      11.BIN                  IDENTICAL
sk1          [2/2]      st2.BIN                 IDENTICAL
sk2          [2/2]      st4.BIN                 IDENTICAL
sk3          [2/2]      6st.BIN                 IDENTICAL
sk4          [2/2]      8st.BIN                 IDENTICAL
sk5          [2/2]      12.BIN                  IDENTICAL
sk6          [2/2]      10.BIN                  IDENTICAL
sk1          [1/2]      st1.BIN                 99.951172%
sk4          [1/2]      7st.BIN                 99.951172%
sk.bpr                                          NO MATCH
*/

ROM_START( streaknga )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "st1.bin",  0x0000, 0x0800, CRC(c827e124) SHA1(85d84bb678cb80e7ca4a591b6c30a247e9aac213) )
	ROM_LOAD( "st2.bin",  0x0800, 0x0800, CRC(b01d4f8f) SHA1(1116374b5e90c7e525319c92d6dd9ba6641ca231) )
	ROM_LOAD( "st3.bin",  0x1000, 0x0800, CRC(c7a9c493) SHA1(58c25bdf68807ecfe6fcaf90137a5f8701696d1d) )
	ROM_LOAD( "st4.bin",  0x1800, 0x0800, CRC(12487c75) SHA1(12796dab558ab13b42ee87181d60c3fa290c64e4) )
	ROM_LOAD( "5st.bin",  0x2000, 0x0800, CRC(f9f9e2be) SHA1(3efa850361137f50a42de9dcd868519fbc4680b4) )
	ROM_LOAD( "6st.bin",  0x2800, 0x0800, CRC(c22fe6c2) SHA1(fb8bf579f6bd413ae5ef49facf1d21125da52833) )
	ROM_LOAD( "7st.bin",  0x3000, 0x0800, CRC(9cd7869a) SHA1(5edccf4a1dff184ebbec8748216353805abff29d) )
	ROM_LOAD( "8st.bin",  0x3800, 0x0800, CRC(5e750ad3) SHA1(55f77564a9bb07d66c2f13ee1a4ff39c8029a383) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "11.bin",   0x0000, 0x0800, CRC(cddd8924) SHA1(340ed1d4da62388d49838db25f09a569a0e17dee) )
	ROM_LOAD( "12.bin",   0x0800, 0x0800, CRC(10cda095) SHA1(488def62d52296c7482b349c1aeaeaa1d45bad77) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "9.bin",    0x0000, 0x0800, CRC(6a2a8a0f) SHA1(1822c2f4c48740fee2d4e048410db5e846c8a2db) )
	ROM_LOAD( "10.bin",   0x0800, 0x0800, CRC(3563dfbe) SHA1(9056b618e19a40cc96d90f393c1c40a573497ca7) )

	ROM_REGION( 0x0020, "proms", 0 )    /* from parent set */
	ROM_LOAD( "sk.bpr",   0x0000, 0x0020, BAD_DUMP CRC(bce79607) SHA1(49d60fde149240bcd025f721b0fbbbdbc549a42f) )
ROM_END

ROM_START( pacmanbl ) /* Artic Multi-System */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1",  0x0000, 0x0800, CRC(6718df42) SHA1(ee15c3f583d381fba4878f824f83d04479a0cee5) )
	ROM_LOAD( "2",  0x0800, 0x0800, CRC(33be3648) SHA1(50175889cf37fe8a81c931e009b55d10f8d0444a) )
	ROM_LOAD( "3",  0x1000, 0x0800, CRC(f98c0ceb) SHA1(4faf8b2fb3f109d1196a9ea256328485074a31b9) )
	ROM_LOAD( "4",  0x1800, 0x0800, CRC(a9cd0082) SHA1(f44ff1ad15d5ee3096f8f44f9c605f32ae2737d9) )
	ROM_LOAD( "5",  0x2000, 0x0800, CRC(6d475afc) SHA1(4fe6bde352c7dd9572fefaae4b59640b4f4eb8ba) )
	ROM_LOAD( "6",  0x2800, 0x0800, CRC(cbe863d3) SHA1(97a2ffa6ab33e6061c664dcd1ee57c86a456782f) )
	ROM_LOAD( "7",  0x3000, 0x0800, CRC(7daef758) SHA1(4dc8ec0ea8fc04d5bffc1c1335407729309c17f0) )
	/*              0x3800, 0x0800 not populated */

	/* note from f205v: on the PCB I have, 10b and 11b have been joined into one single 2732 EPROM labeled "pmc31"
	The same goes for 9b and 12b, joined into one single 2732 EPROM labeled "pmc42" */
	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "12",  0x0000, 0x0800, CRC(b2ed320b) SHA1(680a6fdcb65cc2d88d10bc85e0b2628f43375c5c) )
	ROM_LOAD( "11",  0x0800, 0x0800, CRC(ab88b2c4) SHA1(d0c829ea8021eae81a2b82d36c35ad8258b115e0) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "10",  0x0000, 0x0800, CRC(44a45b72) SHA1(8abd0684a01d6c23ef5cf5f0765458f982316acf) )
	ROM_LOAD( "9",   0x0800, 0x0800, CRC(fa84659f) SHA1(20c212723f9062f052539190dfe3fc41577543eb) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "sn74s288n.6l", 0x0000, 0x0020, CRC(24652bc4) SHA1(d89575f3749c75dc963317fe451ffeffd9856e4d) ) /* same as pisces */
ROM_END

ROM_START( pacmanbla ) /* content is the same as the above bootleg, but arranged differently in the roms */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom1.bin",      0x0000, 0x0800, CRC(75e4f967) SHA1(8bdb5ab2b3f978c578f1498b64bb16d2cb292ef2) )
	ROM_CONTINUE(0x2000,0x800)
	ROM_LOAD( "rom2.bin",      0x0800, 0x0800, CRC(5b2e4293) SHA1(bb925491e315d2e6bb9e756bdf664d173f83cd58) )
	ROM_CONTINUE(0x2800,0x800)
	ROM_LOAD( "rom3.bin",      0x1000, 0x0800, CRC(c06e30a4) SHA1(00d8d114bd4e0b689e75e312c93f6c7b8492426e) )
	ROM_CONTINUE(0x3000,0x800)
	ROM_LOAD( "rom4.bin",      0x1800, 0x0800, CRC(592b4ba8) SHA1(52a559344f70cd8a3a87de71de2bae57c885641a) )
	ROM_CONTINUE(0x3800,0x800)

	ROM_REGION( 0x2000, "tempgfx", 0 )
	ROM_LOAD( "rom5.bin",      0x0000, 0x1000, CRC(f2d8c01e) SHA1(d4a5789476fa7859bb936df10590775e97e87578) )
	ROM_LOAD( "rom6.bin",      0x1000, 0x1000, CRC(346a1720) SHA1(e152c9161f4e8ef53153b9c4a8ecef9fdbbe2463) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_COPY( "tempgfx", 0x0800, 0x0000, 0x0800 )
	ROM_COPY( "tempgfx", 0x1800, 0x0800, 0x0800 )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_COPY( "tempgfx", 0x0000, 0x0000, 0x0800 )
	ROM_COPY( "tempgfx", 0x1000, 0x0800, 0x0800 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "sn74s288n.6l", 0x0000, 0x0020, CRC(24652bc4) SHA1(d89575f3749c75dc963317fe451ffeffd9856e4d) ) /* same as pisces */
ROM_END

ROM_START( ghostmun )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pac1.bin",     0x0000, 0x1000, CRC(19338c70) SHA1(cc2665b7d534d324627d12025ee099ff415d4214) )
	ROM_LOAD( "pac2.bin",     0x1000, 0x1000, CRC(18db074d) SHA1(a70ed18f632e947493e648e6fc057dfb7a2a3322) )
	ROM_LOAD( "pac3.bin",     0x2000, 0x1000, CRC(abb98b1d) SHA1(bb0109d353359bb192a3e6856a857c2f842838cb) )
	ROM_LOAD( "pac4.bin",     0x3000, 0x1000, CRC(2403c78e) SHA1(52d8c8a4efcf47871485080ab217098a019e6579) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "blpac12b",     0x0000, 0x0800, CRC(b2ed320b) SHA1(680a6fdcb65cc2d88d10bc85e0b2628f43375c5c) )
	ROM_LOAD( "blpac11b",     0x0800, 0x0800, CRC(ab88b2c4) SHA1(d0c829ea8021eae81a2b82d36c35ad8258b115e0) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "blpac10b",     0x0000, 0x0800, CRC(44a45b72) SHA1(8abd0684a01d6c23ef5cf5f0765458f982316acf) )
	ROM_LOAD( "blpac9b",      0x0800, 0x0800, CRC(fa84659f) SHA1(20c212723f9062f052539190dfe3fc41577543eb) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "ghostmun.clr", 0x0000, 0x0020, CRC(759647e3) SHA1(9e21e12f4be007265851a5a1676b7e9facf7109b) )
ROM_END

ROM_START( phoenxp2 ) /* Artic Multi-System */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1",  0x0000, 0x0800, CRC(f6dcfd51) SHA1(8475726dbbf0dd13581f457a5379956424dc2862) )
	ROM_LOAD( "2",  0x0800, 0x0800, CRC(de951936) SHA1(657d2c595a5864d8c9a51926ab7bfa0a7068e2b2) )
	ROM_LOAD( "3",  0x1000, 0x0800, CRC(7a3af2da) SHA1(49e9ad7115e71839d1d027552d08c7831a617b4a) )
	ROM_LOAD( "4",  0x1800, 0x0800, CRC(c820ad32) SHA1(b5286d49f6578dfeffdf429e7d52321f4813e03c) )
	ROM_LOAD( "5",  0x2000, 0x0800, CRC(08e83233) SHA1(44159e0f3fb717b726b6b5c77da32391c1cdd04d) )
	ROM_LOAD( "6",  0x2800, 0x0800, CRC(f31fb9d6) SHA1(bdfb5427869940ed6b8e2667a6c1f410a9a41b87) )
	ROM_LOAD( "7",  0x3000, 0x0800, CRC(d3a480c1) SHA1(d1e4fe83b49f918285009f09df38c6555a686823) )
	ROM_LOAD( "8",  0x3800, 0x0800, CRC(edf9779e) SHA1(c1e7307c59f15bebac3e29b41135fe9f18fc9a06) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "09",       0x0000, 0x0800, CRC(1a657b1f) SHA1(42149dafdde7d9104f0bddda2223bfc211d0154a) )
	ROM_LOAD( "11",       0x0800, 0x0800, CRC(7a2b48e5) SHA1(f559799c685dd2cb9de06a356bee95b7d6ffadfc) )
	ROM_LOAD( "10",       0x1000, 0x0800, CRC(9b570016) SHA1(44fd2b1caeecdc5200d63c35636f0a605943d30c) )
	ROM_LOAD( "12",       0x1800, 0x0800, CRC(73956244) SHA1(e464b587b5ed636816cc9688593f5b6005cb5216) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "sn74s288n.6l", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* same as 'Omega' */
ROM_END

ROM_START( batman2 ) /* wasn't marked as artic multi-system, but it's basically the same as the above phoenixp2 */
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "01.bin",       0x0000, 0x0800, CRC(150fbca5) SHA1(a5dc104169eb3225c6200e7e07102f8a9bee6861) )
	ROM_LOAD( "02.bin",       0x0800, 0x0800, CRC(b1624fd0) SHA1(ca4678cf7a8b935be2f68d6e342c1f961bf6f1a2) )
	ROM_LOAD( "03.bin",       0x1000, 0x0800, CRC(93774188) SHA1(8bdd3290db43459c56b932b582f555d89df30bd1) )
	ROM_LOAD( "04.bin",       0x1800, 0x0800, CRC(8a94ec6c) SHA1(dacadab9a05ddee2de188b368f795d74213e020d) )
	ROM_LOAD( "05.bin",       0x2000, 0x0800, CRC(a3669461) SHA1(11ea7aa9b55f5790cc2451d80d0eb84388cf47eb) )
	ROM_LOAD( "06.bin",       0x2800, 0x0800, CRC(fa1efbfe) SHA1(f7222dd21e0810d0c8c32919ebb6e0e7bbb4c68e) )
	ROM_LOAD( "07.bin",       0x3000, 0x0800, CRC(9b77debd) SHA1(1f5521bc0f701d86e61219ad3b9516aaa71a68da) )
	ROM_LOAD( "08.bin",       0x3800, 0x0800, CRC(6466177e) SHA1(fc359eadee34586576c557ff7c1dd2c8d49bdf3f) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "09.bin",       0x0000, 0x0800, CRC(1a657b1f) SHA1(42149dafdde7d9104f0bddda2223bfc211d0154a) )
	ROM_LOAD( "11.bin",       0x0800, 0x0800, CRC(7a2b48e5) SHA1(f559799c685dd2cb9de06a356bee95b7d6ffadfc) )
	ROM_LOAD( "10.bin",       0x1000, 0x0800, CRC(9b570016) SHA1(44fd2b1caeecdc5200d63c35636f0a605943d30c) )
	ROM_LOAD( "12.bin",       0x1800, 0x0800, CRC(73956244) SHA1(e464b587b5ed636816cc9688593f5b6005cb5216) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "l06_prom.bin", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( atlantisb ) /* Artic Multi-System */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1",  0x0000, 0x0800, CRC(2b612351) SHA1(cfd244946190c062146716c0417c35be216943e4) ) /* aka "subfury" */
	ROM_LOAD( "2",  0x0800, 0x0800, CRC(b1c970e9) SHA1(1e12a1d34453b01ff5ef4d9530a90f476fc34631) )
	/*              0x1000, 0x0800 not populated */
	ROM_LOAD( "3",  0x1800, 0x0800, CRC(63c3783e) SHA1(e3a7a8bb2c108d3e1e1403017c72963afcd23813) )
	ROM_LOAD( "4",  0x2000, 0x0800, CRC(45f7cf34) SHA1(d1e0e0be6dec377b684625bdfdc5a3a8af847492) )
	ROM_LOAD( "5",  0x2800, 0x0800, CRC(f335b96b) SHA1(17daa6d9bc916081f3c6cbdfe5b4960177dc7c9b) )
	ROM_LOAD( "6",  0x3000, 0x0800, CRC(a50bf8d5) SHA1(5bca98e1c0838d27ec66bf4b906877977b212b6d) )
	/*              0x3800, 0x0800 not populated */

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "8",  0x0000, 0x0800, CRC(55cd5acd) SHA1(b3e2ce71d4e48255d44cd451ee015a7234a108c8) )
	ROM_LOAD( "7",  0x0800, 0x0800, CRC(72e773b8) SHA1(6ce178df3bd6a4177c68761572a13a56d222c48f) )

	ROM_REGION( 0x1000, "gfx2", ROMREGION_ERASEFF )
	/* 0x000, 0x0800 not populated */
	/* 0x000, 0x0800 not populated */

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "sn74s288n.6l", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END


ROM_START( tenspot )
	/* Game A - Survivor */
	ROM_REGION( 0x4000, "game_0_cpu", 0 )
	ROM_LOAD( "svt1-a.a1",    0x0000, 0x1000, CRC(5806d0e6) SHA1(887ff2985578faa9535387a5ce3953452e7a3171) )
	ROM_LOAD( "svt2-a.a2",    0x1000, 0x1000, CRC(847c16d0) SHA1(7ac65e5f47153f7e1e70c701b16d537774f60982) )
	ROM_LOAD( "svt3-a.a3",    0x2000, 0x1000, CRC(63a6990b) SHA1(901772e2ed8536c3031a66204889e1cac60011c5) )

	ROM_REGION( 0x2000, "game_0_temp", 0 )
	ROM_LOAD( "syt5-a.a5",       0x0000, 0x1000, CRC(7f804605) SHA1(898f7de488ca79b5b29dbdb93233c63ed20df354) )
	ROM_LOAD( "svt5-a.a6",       0x1000, 0x1000, CRC(fff07c86) SHA1(a37034fb7fcf60ee5f098d405ee3277616c8aceb) )

	ROM_REGION( 0x0020, "game_0_prom", 0 )
	ROM_LOAD( "clr3.a7",       0x0000, 0x0020, CRC(aefcf6b1) SHA1(10cde93e23fe8720f5af9039c4f68999f7cfce67) )

	/* Game B - Moon Cresta */
	ROM_REGION( 0x4000, "game_1_cpu", 0 )
	ROM_LOAD( "mct1-a.b1",    0x0000, 0x1000, CRC(90a74a0b) SHA1(a1fb24aa621611c18bf6188f380640e5576ac248) )
	ROM_LOAD( "mct2-a.b2",    0x1000, 0x1000, CRC(9bb1e8e8) SHA1(0a8567c7efb6511360a786c18a09966966c253a2) )
	ROM_LOAD( "mct3-a.b3",    0x2000, 0x1000, CRC(6d19c439) SHA1(39a5d78c7d42981e1fa12bc6c794b915f738faf7) )
	ROM_LOAD( "mct4-a.b4",    0x3000, 0x1000, CRC(dd029a6e) SHA1(e6035a6981e22565a2af3a3ecac16676cb3b3500) )

	ROM_REGION( 0x2000, "game_1_temp", 0 )
	ROM_LOAD( "mct5-a.b5",       0x0000, 0x1000, CRC(ac1a6a62) SHA1(febfcdbf1afe9a5352d8d96b454a6c8fc7818ef0) )
	ROM_LOAD( "mct6-a.b6",       0x1000, 0x1000, CRC(dc19ec73) SHA1(19a3295597a8eff2587ff838a3b8f7e3817f22f0) )

	ROM_REGION( 0x0020, "game_1_prom", 0 )
	ROM_LOAD( "clr2.b7",       0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )

	/* Game C - Space Cruiser */
	ROM_REGION( 0x4000, "game_2_cpu", 0 )
	ROM_LOAD( "sct1-a.c1",    0x0000, 0x1000, CRC(5068e89c) SHA1(539fe47ec846ec038ee6ffd2d3578d7cf25d4219) )
	ROM_LOAD( "sct2-a.c2",    0x1000, 0x1000, CRC(96013308) SHA1(756ad5592acbe68c923a810eba2ff4eda4a9a51c) )
	ROM_LOAD( "sct3-a.c3",    0x2000, 0x1000, CRC(3c6ef851) SHA1(a2c5dd8cca60b7340c9c3973137415621f5b1a11) )

	ROM_REGION( 0x2000, "game_2_temp", 0 )
	ROM_LOAD( "sct5-a.c5",       0x0000, 0x1000, CRC(272a0037) SHA1(48dcb9da66db75721668c3708ed1a55a0ee65238) )
	ROM_LOAD( "sct6-a.c6",       0x1000, 0x1000, CRC(d6b35f01) SHA1(e16a7400901e2b0ad4ce70dce8092741d85b6a43) )

	ROM_REGION( 0x0020, "game_2_prom", 0 )
	ROM_LOAD( "clr1.c7",       0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )

	/* Game D - Mission Rescue (Black Hole) */
	ROM_REGION( 0x4000, "game_3_cpu", 0 )
	ROM_LOAD( "mrt1-a.d1",    0x0000, 0x1000, CRC(eb63c4e0) SHA1(29a59fa8616e36dd098ff9f6e520128db3b66ed9) )
	ROM_LOAD( "mrt2-a.d2",    0x1000, 0x1000, CRC(e4ba463a) SHA1(b5370bc33275f6aa52c96304db4be086b5f6d18c) )
	ROM_LOAD( "mrt3-a.d3",    0x2000, 0x1000, CRC(62d7b1ce) SHA1(5243d053ea53dcfe4110fdf04077e818237121c8) )

	ROM_REGION( 0x2000, "game_3_temp", 0 )
	ROM_LOAD( "mrt5-a.d5",       0x0000, 0x1000, CRC(cc6bb4bc) SHA1(f81f671d2865a43849f10a48c0cc9f6c5bbe0f9e) )
	ROM_LOAD( "mrt6-a.d6",       0x1000, 0x1000, CRC(4b4e6c62) SHA1(86ea8436d631a30461f0ba708c0b597f15ebdd47) )

	ROM_REGION( 0x0020, "game_3_prom", 0 )
	ROM_LOAD( "clr1.d7",       0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )

	/* Game E - Uniwars */
	ROM_REGION( 0x4000, "game_4_cpu", 0 )
	ROM_LOAD( "uwt1-a.e1",    0x0000, 0x1000, CRC(1379be84) SHA1(e280e0402c7cfa52f2a04801634f8c3aa85bf02f) )
	ROM_LOAD( "uwt2-a.e2",    0x1000, 0x1000, CRC(ed8e5260) SHA1(a2ebc8aa9b5da6ff689847de8973a512f9d96128) )
	ROM_LOAD( "uwt3-a.e3",    0x2000, 0x1000, CRC(9abd1570) SHA1(74f82ac2c3a1822f1e5575e7e72c017d24c43dc1) )
	ROM_LOAD( "uwt4-b.e4",    0x3000, 0x1000, CRC(daea5232) SHA1(cdb2a1a14188e971e2c98c625e0b577f688a753a) )

	ROM_REGION( 0x2000, "game_4_temp", 0 )
	ROM_LOAD( "uwt5-a.e5",       0x0000, 0x1000, CRC(49a1c892) SHA1(b6b1be0d8fa6909ed8e6f36d3f75dadd8f5cafbe) )
	ROM_LOAD( "uwt6-a.e6",       0x1000, 0x1000, CRC(9d27e53d) SHA1(ef41c8b586545207a0e2021c8634df4ffe4b7b8a) )

	ROM_REGION( 0x0020, "game_4_prom", 0 )
	ROM_LOAD( "clr1.e7",       0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )

	/* Game F - Batman Pt.2 (Phoenix) - this needs custom video banking like the standalone version.... */
	ROM_REGION( 0x4000, "game_5_cpu", 0 )
	ROM_LOAD( "bmt1-a.f1",    0x0000, 0x1000, CRC(2aecaaa0) SHA1(07c35f34eebbe65247a412c828328a558936d03c) )
	ROM_LOAD( "bmt2-a.f2",    0x1000, 0x1000, CRC(1972ff4c) SHA1(262db6caba201fa1f2f7b04f36f4d6084283d841) )
	ROM_LOAD( "bmt3-a.f3",    0x2000, 0x1000, CRC(34c0728d) SHA1(54f76368a387b42010258fa549465a430dd6ecf7) )
	ROM_LOAD( "bmt4-a.f4",    0x3000, 0x1000, CRC(fc2e8de1) SHA1(683815035054669a845ce440d66c023cf54dbdcc) )

	ROM_REGION( 0x2000, "game_5_temp", 0 )
	ROM_LOAD( "bmt5-a.f5",       0x0000, 0x1000, CRC(ee71a2de) SHA1(c41b8c705ec697ab2a37fbde0fc2bbcd3259ec98) )
	ROM_LOAD( "bmt6-a.f6",       0x1000, 0x1000, CRC(ea538ab9) SHA1(310052358fca96bba5b69366f7bd47c446287783) )

	ROM_REGION( 0x0020, "game_5_prom", 0 )
	ROM_LOAD( "clr1.f7",       0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )

	/* Game H - Defend UFO */
	ROM_REGION( 0x4000, "game_6_cpu", 0 )
	ROM_LOAD( "rut1-a.h1",    0x0000, 0x1000, CRC(364b0689) SHA1(d39c1ca5774b21c9e045f2234c2256f56ff36a2a) )
	ROM_LOAD( "rut2-a.h2",    0x1000, 0x1000, CRC(ed448821) SHA1(33c983b8cfa17299728363870f906477bce14dbf) )
	ROM_LOAD( "rut3-a.h3",    0x2000, 0x1000, CRC(312d5d37) SHA1(772a5e7ea94dd6b9744f4eef7d7ac26cb58d58ab) )
	ROM_LOAD( "rut4-a.h4",    0x3000, 0x1000, CRC(2281b279) SHA1(c6cfb14b6656de185f38a5c73cf042f2f8b4cc6e) )

	ROM_REGION( 0x2000, "game_6_temp", 0 )
	ROM_LOAD( "rut5-a.h5",       0x0000, 0x1000, CRC(6fb16866) SHA1(e1a1ac17ef9c08ac2f4c7b15a13932f542aed95d) )
	ROM_LOAD( "rut6-a.h6",       0x1000, 0x1000, CRC(5ae0dc50) SHA1(d4ec2179d5181b71171bac5098a6f7f1c96e63b3) )

	ROM_REGION( 0x0020, "game_6_prom", 0 )
	ROM_LOAD( "clr1.h7",       0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )

	/* Game J - King and Balloon */
	ROM_REGION( 0x4000, "game_7_cpu", 0 )
	ROM_LOAD( "kbt1-a.j1",    0x0000, 0x1000, CRC(6bcdfaef) SHA1(5f3d57a91d57c8758f1fa39a44be6082fff52406) )
	ROM_LOAD( "kbt2-a.j2",    0x1000, 0x1000, CRC(3652c64b) SHA1(7cb2a51e1830d48d5d3a62d521dfef1779dd5222) )
	ROM_LOAD( "kbt3-a.j3",    0x2000, 0x1000, CRC(946447c6) SHA1(0759f7d8b538d5e489a85bc6551cde76e6b3ed71) )

	ROM_REGION( 0x2000, "game_7_temp", 0 )
	ROM_LOAD( "kbt5-a.j5",       0x0000, 0x1000, CRC(ea36f825) SHA1(20e26c97d780fb1fd15ad4c33c097a5b3539d43d) )
	ROM_LOAD( "kbt6-a.j6",       0x1000, 0x1000, CRC(2b8b46bc) SHA1(48a7a65fc5c174d0cc654557b3a1166df7fea4da) )

	ROM_REGION( 0x0020, "game_7_prom", 0 )
	ROM_LOAD( "clr1.j7",       0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )

	/* Game K - Omega (The End) */
	ROM_REGION( 0x4000, "game_8_cpu", 0 )
	ROM_LOAD( "omt1-a.k1",    0x0000, 0x1000, CRC(8fc41a53) SHA1(c1bb4018bad12b83954cf8da8eba49f23618139a) )
	ROM_LOAD( "omt2-a.k2",    0x1000, 0x1000, CRC(a3073430) SHA1(200b15c572d7cff9be39439a247c9be742f17a61) )
	ROM_LOAD( "omt3-a.k3",    0x2000, 0x1000, CRC(b0de1fa2) SHA1(71cf8303b7ddc5813d6b92a71bd53f83272f5f22) )

	ROM_REGION( 0x2000, "game_8_temp", 0 )
	ROM_LOAD( "omt5-a.k5",       0x0000, 0x1000, CRC(5ab402c8) SHA1(c0640d9907d7dcd34cd7105d21b99fc15fcbac6e) )
	ROM_LOAD( "omt6-a.k6",       0x1000, 0x1000, CRC(2552e470) SHA1(ba5fba8047e4bb23442b0c2d45c858ec9da63945) )

	ROM_REGION( 0x0020, "game_8_prom", 0 )
	ROM_LOAD( "clr1.k7",       0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )

	/* Game L - Battle of Atlantis */
	ROM_REGION( 0x4000, "game_9_cpu", 0 )
	ROM_LOAD( "bat1-a.l1",    0x0000, 0x1000, CRC(5849dd36) SHA1(c69bf6119ae63a3c855d58bbadb5b358f7b25ad0) )
	ROM_LOAD( "bat2-a.l2",    0x1000, 0x1000, CRC(adc2ce4b) SHA1(36f477a48b3df9cb2456460048b2fdd0d3e8b73e) )
	ROM_LOAD( "bat3-a.l3",    0x2000, 0x1000, CRC(81270ace) SHA1(0385fedacbbda4ed750c5a64d51a60ed98c3ed65) )
	ROM_LOAD( "bat4-a.l4",    0x3000, 0x1000, CRC(bd751ba9) SHA1(dbdc8972b0236755d5a8ea90e2de2d16585f5e02) )

	ROM_REGION( 0x2000, "game_9_temp", 0 )
	ROM_LOAD( "bat5-a.l5",       0x0000, 0x1000, CRC(b9701513) SHA1(d8bc7b36a6d0b1e73aa7b6a5dab7b36ce111a04c) )
	ROM_LOAD( "bat6-a.l6",       0x1000, 0x1000, CRC(54b423b7) SHA1(31eec49b4e9c8b56668b9037dd47e66659ce64cb) )

	ROM_REGION( 0x0020, "game_9_prom", 0 )
	ROM_LOAD( "clr1.l7",       0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )


	ROM_REGION( 0x4000, "selectcpu", 0 ) // probably related to game selection
	ROM_LOAD( "tenu2-d.u2",    0x0000, 0x800, CRC(58c7fe3b) SHA1(a4faa8e669a81fe01696d6df9c8ebd5c17be0f00) )

	ROM_REGION( 0x4000, "unknown", 0 ) // ?? no idea
	ROM_LOAD( "u1.u1",    0x0000, 0x100, CRC(f18006f7) SHA1(f9a3541cd7f2b75816227d8befc03d2e33eeebac) )


	/* temporary - replace game_x with the game number you want to test. */
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASEFF )
	ROM_REGION( 0x2000, "gfx1", ROMREGION_ERASEFF )
	ROM_REGION( 0x2000, "gfx2", ROMREGION_ERASEFF )
	ROM_REGION( 0x0020, "proms", ROMREGION_ERASEFF )
	ROM_END

/*************************************
 *
 *  ROM definitions
 *  Moon Cresta-derived games
 *
 *************************************/

ROM_START( mooncrst )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "mc1",          0x0000, 0x0800, CRC(7d954a7a) SHA1(a93ee403cfd7887538ad12d33f6dd6c71bea2a32) )
	ROM_LOAD( "mc2",          0x0800, 0x0800, CRC(44bb7cfa) SHA1(349c2e23a9fce73f95bb8168d369082fa129fe3d) )
	ROM_LOAD( "mc3",          0x1000, 0x0800, CRC(9c412104) SHA1(1b40054ebb1ace965a8522119bb23f09797bc5f6) )
	ROM_LOAD( "mc4",          0x1800, 0x0800, CRC(7e9b1ab5) SHA1(435f603c0c3e788a509dd144a7916a34e791ae44) )
	ROM_LOAD( "mc5.7r",       0x2000, 0x0800, CRC(16c759af) SHA1(3b48050411f65f9d3fb41ff22901e22d82bf1cf6) )
	ROM_LOAD( "mc6.8d",       0x2800, 0x0800, CRC(69bcafdb) SHA1(939c8c6ed1cd4660a3d99b8f17ed99cbd7e1352a) )
	ROM_LOAD( "mc7.8e",       0x3000, 0x0800, CRC(b50dbc46) SHA1(4fa084fd1ba5f78e7703e684c57af15ca7a844e4) )
	ROM_LOAD( "mc8",          0x3800, 0x0800, CRC(18ca312b) SHA1(39219059003b949e38305553fea2d33071062c64) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "mcs_b",        0x0000, 0x0800, CRC(fb0f1f81) SHA1(38a6679a8b69bc1870a0e67e692131c42f9535c8) )
	ROM_LOAD( "mcs_d",        0x0800, 0x0800, CRC(13932a15) SHA1(b8885c555c6ad7021be55c6925a0a0872c1b6abd) )
	ROM_LOAD( "mcs_a",        0x1000, 0x0800, CRC(631ebb5a) SHA1(5bc9493afa76c55858b8c8849524cbc77dc838fc) )
	ROM_LOAD( "mcs_c",        0x1800, 0x0800, CRC(24cfd145) SHA1(08c6599db170dd6ee364c44f70a0f5c0f881b6ef) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mmi6331.6l", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END

ROM_START( mooncrstuk )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "mc1",          0x0000, 0x0800, CRC(7d954a7a) SHA1(a93ee403cfd7887538ad12d33f6dd6c71bea2a32) )
	ROM_LOAD( "mc2",          0x0800, 0x0800, CRC(44bb7cfa) SHA1(349c2e23a9fce73f95bb8168d369082fa129fe3d) )
	ROM_LOAD( "mc3",          0x1000, 0x0800, CRC(9c412104) SHA1(1b40054ebb1ace965a8522119bb23f09797bc5f6) )
	ROM_LOAD( "mc4",          0x1800, 0x0800, CRC(7e9b1ab5) SHA1(435f603c0c3e788a509dd144a7916a34e791ae44) )
	ROM_LOAD( "mc5.7r",       0x2000, 0x0800, CRC(16c759af) SHA1(3b48050411f65f9d3fb41ff22901e22d82bf1cf6) )
	ROM_LOAD( "mc6.8d",       0x2800, 0x0800, CRC(69bcafdb) SHA1(939c8c6ed1cd4660a3d99b8f17ed99cbd7e1352a) )
	ROM_LOAD( "mc7.8e",       0x3000, 0x0800, CRC(b50dbc46) SHA1(4fa084fd1ba5f78e7703e684c57af15ca7a844e4) )
	ROM_LOAD( "8_uk.bin",     0x3800, 0x0800, CRC(ce727ad4) SHA1(247fe0ea7dcc7cc50f19da0a54385b8545f03609) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "mcs_b",        0x0000, 0x0800, CRC(fb0f1f81) SHA1(38a6679a8b69bc1870a0e67e692131c42f9535c8) )
	ROM_LOAD( "mcs_d",        0x0800, 0x0800, CRC(13932a15) SHA1(b8885c555c6ad7021be55c6925a0a0872c1b6abd) )
	ROM_LOAD( "mcs_a",        0x1000, 0x0800, CRC(631ebb5a) SHA1(5bc9493afa76c55858b8c8849524cbc77dc838fc) )
	ROM_LOAD( "mcs_c",        0x1800, 0x0800, CRC(24cfd145) SHA1(08c6599db170dd6ee364c44f70a0f5c0f881b6ef) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mmi6331.6l", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END

ROM_START( mooncrstuku )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "smc1f",        0x0000, 0x0800, CRC(389ca0d6) SHA1(51cf6d190a0ebf23b70c2bcf1ccaa4705e29cd09) )
	ROM_LOAD( "smc2f",        0x0800, 0x0800, CRC(410ab430) SHA1(d89abff6ac4afbf69377a1d63043d629a634aab7) )
	ROM_LOAD( "smc3f",        0x1000, 0x0800, CRC(a6b4144b) SHA1(2b27ad54d716286c0dc9476d47df182ae01bcfd7) )
	ROM_LOAD( "smc4f",        0x1800, 0x0800, CRC(4cc046fe) SHA1(465eaacd50967d768babadd09ab9cad35380f6bf) )
	ROM_LOAD( "smc5f",        0x2000, 0x0800, CRC(06d378a6) SHA1(99dbe9fc7f95f8fdce86eb5c32bd1ca1bea0ca3c) )
	ROM_LOAD( "smc6f",        0x2800, 0x0800, CRC(6e84a927) SHA1(82e8e825d157c3c947a3a222bca059a735169c7d) )
	ROM_LOAD( "smc7f",        0x3000, 0x0800, CRC(b45af1e8) SHA1(d7020774707234acdaef5c655f667d5ee9e54a13) )
	ROM_LOAD( "smc8f_uk",     0x3800, 0x0800, CRC(b968b2ff) SHA1(40105423f48d2260e85597c3c1e4d8fe947db793) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "mcs_b",        0x0000, 0x0800, CRC(fb0f1f81) SHA1(38a6679a8b69bc1870a0e67e692131c42f9535c8) )
	ROM_LOAD( "mcs_d",        0x0800, 0x0800, CRC(13932a15) SHA1(b8885c555c6ad7021be55c6925a0a0872c1b6abd) )
	ROM_LOAD( "mcs_a",        0x1000, 0x0800, CRC(631ebb5a) SHA1(5bc9493afa76c55858b8c8849524cbc77dc838fc) )
	ROM_LOAD( "mcs_c",        0x1800, 0x0800, CRC(24cfd145) SHA1(08c6599db170dd6ee364c44f70a0f5c0f881b6ef) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mmi6331.6l", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END

ROM_START( mooncrstu )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "smc1f",        0x0000, 0x0800, CRC(389ca0d6) SHA1(51cf6d190a0ebf23b70c2bcf1ccaa4705e29cd09) )
	ROM_LOAD( "smc2f",        0x0800, 0x0800, CRC(410ab430) SHA1(d89abff6ac4afbf69377a1d63043d629a634aab7) )
	ROM_LOAD( "smc3f",        0x1000, 0x0800, CRC(a6b4144b) SHA1(2b27ad54d716286c0dc9476d47df182ae01bcfd7) )
	ROM_LOAD( "smc4f",        0x1800, 0x0800, CRC(4cc046fe) SHA1(465eaacd50967d768babadd09ab9cad35380f6bf) )
	ROM_LOAD( "e5",           0x2000, 0x0800, CRC(06d378a6) SHA1(99dbe9fc7f95f8fdce86eb5c32bd1ca1bea0ca3c) )
	ROM_LOAD( "bepr199",      0x2800, 0x0800, CRC(6e84a927) SHA1(82e8e825d157c3c947a3a222bca059a735169c7d) )
	ROM_LOAD( "e7",           0x3000, 0x0800, CRC(b45af1e8) SHA1(d7020774707234acdaef5c655f667d5ee9e54a13) )
	ROM_LOAD( "smc8f",        0x3800, 0x0800, CRC(f42164c5) SHA1(e0d1680f193889568edf005786e2767d4fb086f4) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "mcs_b",        0x0000, 0x0800, CRC(fb0f1f81) SHA1(38a6679a8b69bc1870a0e67e692131c42f9535c8) )
	ROM_LOAD( "mcs_d",        0x0800, 0x0800, CRC(13932a15) SHA1(b8885c555c6ad7021be55c6925a0a0872c1b6abd) )
	ROM_LOAD( "mcs_a",        0x1000, 0x0800, CRC(631ebb5a) SHA1(5bc9493afa76c55858b8c8849524cbc77dc838fc) )
	ROM_LOAD( "mcs_c",        0x1800, 0x0800, CRC(24cfd145) SHA1(08c6599db170dd6ee364c44f70a0f5c0f881b6ef) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mmi6331.6l", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END

ROM_START( mooncrsto )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "mc1.7d",       0x0000, 0x0800, CRC(92a86aac) SHA1(f5818ac97d8b779e1fb29bf903f74185d24afb0d) )
	ROM_LOAD( "mc2.7e",       0x0800, 0x0800, CRC(438c2b4b) SHA1(11f56b489b5489999952e91919c5e1f622c59c36) )
	ROM_LOAD( "mc3.7j",       0x1000, 0x0800, CRC(67e3d21d) SHA1(59579d19931ef11b30fdc3912d838200bef92c81) )
	ROM_LOAD( "mc4.7p",       0x1800, 0x0800, CRC(f4db39f6) SHA1(454931f80b35608793590b3843c69ba64cbf6772) )
	ROM_LOAD( "mc5.7r",       0x2000, 0x0800, CRC(16c759af) SHA1(3b48050411f65f9d3fb41ff22901e22d82bf1cf6) )
	ROM_LOAD( "mc6.8d",       0x2800, 0x0800, CRC(69bcafdb) SHA1(939c8c6ed1cd4660a3d99b8f17ed99cbd7e1352a) )
	ROM_LOAD( "mc7.8e",       0x3000, 0x0800, CRC(b50dbc46) SHA1(4fa084fd1ba5f78e7703e684c57af15ca7a844e4) )
	ROM_LOAD( "mc8.8h",       0x3800, 0x0800, CRC(7e2b1928) SHA1(4f0de8e80c2e2ec6df8612755caf93671ea965b0) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "mcs_b",        0x0000, 0x0800, CRC(fb0f1f81) SHA1(38a6679a8b69bc1870a0e67e692131c42f9535c8) )
	ROM_LOAD( "mcs_d",        0x0800, 0x0800, CRC(13932a15) SHA1(b8885c555c6ad7021be55c6925a0a0872c1b6abd) )
	ROM_LOAD( "mcs_a",        0x1000, 0x0800, CRC(631ebb5a) SHA1(5bc9493afa76c55858b8c8849524cbc77dc838fc) )
	ROM_LOAD( "mcs_c",        0x1800, 0x0800, CRC(24cfd145) SHA1(08c6599db170dd6ee364c44f70a0f5c0f881b6ef) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mmi6331.6l", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END

ROM_START( mooncrstg )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "epr194",       0x0000, 0x0800, CRC(0e5582b1) SHA1(946ad4aeb10c0b7b3f93fd24925cc9bcb49e443c) )
	ROM_LOAD( "epr195",       0x0800, 0x0800, CRC(12cb201b) SHA1(ebb01ec646b9e015cbcb93f70dfdaf448afefc12) )
	ROM_LOAD( "epr196",       0x1000, 0x0800, CRC(18255614) SHA1(b373e22d47c0f7facba13148ca9c462ec9a0d732) )
	ROM_LOAD( "epr197",       0x1800, 0x0800, CRC(05ac1466) SHA1(cbf93a8ce0925fa1c073c74f1274b190d9faefaf) )
	ROM_LOAD( "epr198",       0x2000, 0x0800, CRC(c28a2e8f) SHA1(9ff6bab1e1185597ba55cb0d6086091a1fce01a6) )
	ROM_LOAD( "epr199",       0x2800, 0x0800, CRC(5a4571de) SHA1(2a4170dee105922fc69c99b79f6f328098e81918) )
	ROM_LOAD( "epr200",       0x3000, 0x0800, CRC(b7c85bf1) SHA1(cc9f593658ea39c849d80c83ee0c2170cc29879e) )
	ROM_LOAD( "epr201",       0x3800, 0x0800, CRC(2caba07f) SHA1(8fec4904e12b4cfb6068784007278be986a3eede) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "epr203",       0x0000, 0x0800, CRC(be26b561) SHA1(cc27de6888eaf4ee18c0d37d9bcb528dd282b838) )
	ROM_LOAD( "mcs_d",        0x0800, 0x0800, CRC(13932a15) SHA1(b8885c555c6ad7021be55c6925a0a0872c1b6abd) )
	ROM_LOAD( "epr202",       0x1000, 0x0800, CRC(26c7e800) SHA1(034192e5e2cbac4b66a9828f5ec2311c2c368781) )
	ROM_LOAD( "mcs_c",        0x1800, 0x0800, CRC(24cfd145) SHA1(08c6599db170dd6ee364c44f70a0f5c0f881b6ef) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mmi6331.6l", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END

ROM_START( mooncrsb )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "bepr194",      0x0000, 0x0800, CRC(6a23ec6d) SHA1(df2214bdde26a71db59ffd39a745052076563f65) )
	ROM_LOAD( "bepr195",      0x0800, 0x0800, CRC(ee262ff2) SHA1(4e2202023ad53109ea58304071735d2425a617f3) )
	ROM_LOAD( "f03.bin",      0x1000, 0x0800, CRC(29a2b0ab) SHA1(e9fc7161d0566e36307c45b7132e2262c0af4845) )
	ROM_LOAD( "f04.bin",      0x1800, 0x0800, CRC(4c6a5a6d) SHA1(366516f63c9b5239e703e4dfb672659049ddbf44) )
	ROM_LOAD( "e5",           0x2000, 0x0800, CRC(06d378a6) SHA1(99dbe9fc7f95f8fdce86eb5c32bd1ca1bea0ca3c) )
	ROM_LOAD( "bepr199",      0x2800, 0x0800, CRC(6e84a927) SHA1(82e8e825d157c3c947a3a222bca059a735169c7d) )
	ROM_LOAD( "e7",           0x3000, 0x0800, CRC(b45af1e8) SHA1(d7020774707234acdaef5c655f667d5ee9e54a13) )
	ROM_LOAD( "bepr201",      0x3800, 0x0800, CRC(66da55d5) SHA1(39e2f6107e77ee97860147f64b9673cd9a2ae612) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "epr203",       0x0000, 0x0800, CRC(be26b561) SHA1(cc27de6888eaf4ee18c0d37d9bcb528dd282b838) )
	ROM_LOAD( "mcs_d",        0x0800, 0x0800, CRC(13932a15) SHA1(b8885c555c6ad7021be55c6925a0a0872c1b6abd) )
	ROM_LOAD( "epr202",       0x1000, 0x0800, CRC(26c7e800) SHA1(034192e5e2cbac4b66a9828f5ec2311c2c368781) )
	ROM_LOAD( "mcs_c",        0x1800, 0x0800, CRC(24cfd145) SHA1(08c6599db170dd6ee364c44f70a0f5c0f881b6ef) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mmi6331.6l", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END

ROM_START( mooncrs2 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "f8.bin",       0x0000, 0x0800, CRC(d36003e5) SHA1(562b27f1bccce6ae29de18b93fa51c508446cda9) )
	ROM_LOAD( "bepr195",      0x0800, 0x0800, CRC(ee262ff2) SHA1(4e2202023ad53109ea58304071735d2425a617f3) )
	ROM_LOAD( "f03.bin",      0x1000, 0x0800, CRC(29a2b0ab) SHA1(e9fc7161d0566e36307c45b7132e2262c0af4845) )
	ROM_LOAD( "f04.bin",      0x1800, 0x0800, CRC(4c6a5a6d) SHA1(366516f63c9b5239e703e4dfb672659049ddbf44) )
	ROM_LOAD( "e5",           0x2000, 0x0800, CRC(06d378a6) SHA1(99dbe9fc7f95f8fdce86eb5c32bd1ca1bea0ca3c) )
	ROM_LOAD( "bepr199",      0x2800, 0x0800, CRC(6e84a927) SHA1(82e8e825d157c3c947a3a222bca059a735169c7d) )
	ROM_LOAD( "e7",           0x3000, 0x0800, CRC(b45af1e8) SHA1(d7020774707234acdaef5c655f667d5ee9e54a13) )
	ROM_LOAD( "m7.bin",       0x3800, 0x0800, CRC(957ee078) SHA1(472038dedfc01c995be889ea93d4df8bef2b874c) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "1h_1_10.bin",  0x0000, 0x0800, CRC(528da705) SHA1(d726ee18b79774c982f88afb2a508eb5d5783193) )
	ROM_LOAD( "12.chr",       0x0800, 0x0200, CRC(5a4b17ea) SHA1(8a879dc34fdecc8a121c4a87abb981212fb05945) )
	ROM_CONTINUE(             0x0c00, 0x0200 )  /* this version of the gfx ROMs has two */
	ROM_CONTINUE(             0x0a00, 0x0200 )  /* groups of 16 sprites swapped */
	ROM_CONTINUE(             0x0e00, 0x0200 )
	ROM_LOAD( "1k_1_11.bin",  0x1000, 0x0800, CRC(4e79ff6b) SHA1(f72386a3766a7fcc7b4b8cedfa58b8d57f911f6f) )
	ROM_LOAD( "11.chr",       0x1800, 0x0200, CRC(e0edccbd) SHA1(0839a4c9b6e863d12253ae8e1732e80e08702228) )
	ROM_CONTINUE(             0x1c00, 0x0200 )
	ROM_CONTINUE(             0x1a00, 0x0200 )
	ROM_CONTINUE(             0x1e00, 0x0200 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mmi6331.6l", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END

ROM_START( mooncrs3 ) /* Bootleg by Jeutel, very similar to Moon Cresta (bootleg set 2) */
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "b1.7f",  0x0000, 0x0800, CRC(0b28cd8a) SHA1(a1aa0ec63e1dddf4263aa39f6a5fda93108b6e98) )
	ROM_CONTINUE(       0x2000, 0x0800 )
	ROM_LOAD( "b2.7h",  0x0800, 0x0800, CRC(74a6f0ca) SHA1(cc8e8193bb6bd62f6cb9ea924e4da5ddc44c4685) )
	ROM_CONTINUE(       0x2800, 0x0800 )
	ROM_LOAD( "b3.7j",  0x1000, 0x0800, CRC(eeb34cc9) SHA1(c5e7d5e1989211be949972e4281403b7b4866922) )
	ROM_CONTINUE(       0x3000, 0x0800 )
	ROM_LOAD( "b4.7k",  0x1800, 0x0800, CRC(714330e5) SHA1(c681752732c73a6c9bcc9acdcd5c978c455acba0) )
	ROM_CONTINUE(       0x3800, 0x0800 )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "o.1h",  0x0000, 0x0800, CRC(528da705) SHA1(d726ee18b79774c982f88afb2a508eb5d5783193) )
	ROM_LOAD( "q.1h",  0x0800, 0x0200, CRC(5a4b17ea) SHA1(8a879dc34fdecc8a121c4a87abb981212fb05945) )
	ROM_CONTINUE(      0x0c00, 0x0200 ) /* this version of the gfx ROMs has two */
	ROM_CONTINUE(      0x0a00, 0x0200 ) /* groups of 16 sprites swapped */
	ROM_CONTINUE(      0x0e00, 0x0200 )
	ROM_LOAD( "p.1k",  0x1000, 0x0800, CRC(4e79ff6b) SHA1(f72386a3766a7fcc7b4b8cedfa58b8d57f911f6f) )
	ROM_LOAD( "r.1k",  0x1800, 0x0200, CRC(e0edccbd) SHA1(0839a4c9b6e863d12253ae8e1732e80e08702228) )
	ROM_CONTINUE(      0x1c00, 0x0200 )
	ROM_CONTINUE(      0x1a00, 0x0200 )
	ROM_CONTINUE(      0x1e00, 0x0200 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mmi6331.6l", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END

ROM_START( mooncrs4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mooncrs4.7k", 0x0000, 0x1000, CRC(5e201041) SHA1(2ab29e69b1cf9464e6a6a3574a3097cc2bd38432) )
	ROM_LOAD( "mooncrs4.7j", 0x1000, 0x1000, CRC(8de07c8e) SHA1(7f520a57acb8acedd7b4e29d8367fe2b190efa37) )
	ROM_LOAD( "mooncrs4.7h", 0x2000, 0x1000, CRC(888c6d61) SHA1(07404ac714aa1ff9e6bbffffa8afa76a5899ad75) )
	ROM_LOAD( "mooncrs4.7f", 0x3000, 0x1000, CRC(492f9b01) SHA1(a301ef05411e7f2cc2c2433faf0933299c43c49a) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "mooncrs4.1h", 0x0800, 0x0200, CRC(f508a7a5) SHA1(0867ef190a0543411fe8e394c8f99669f4050433) )
	ROM_CONTINUE(            0x0c00, 0x0200 )
	ROM_CONTINUE(            0x0a00, 0x0200 )
	ROM_CONTINUE(            0x0e00, 0x0200 )
	ROM_CONTINUE(            0x0000, 0x0800 )
	ROM_LOAD( "mooncrs4.1k", 0x1800, 0x0200, CRC(9b549313) SHA1(7f161b4de86bc64ada4bad84beab3585dde14944) )
	ROM_CONTINUE(            0x1c00, 0x0200 )
	ROM_CONTINUE(            0x1a00, 0x0200 )
	ROM_CONTINUE(            0x1e00, 0x0200 )
	ROM_CONTINUE(            0x1000, 0x0800 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "prom.6l",     0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( fantazia )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "f01.bin",      0x0000, 0x0800, CRC(d3e23863) SHA1(f0a6f7491fdf8aae214f40078b29b7aecdcf2f1e) )
	ROM_LOAD( "f02.bin",      0x0800, 0x0800, CRC(63fa4149) SHA1(603ee6d4d2952cc08b3f6e98b1a2053671875e44) )
	ROM_LOAD( "f03.bin",      0x1000, 0x0800, CRC(29a2b0ab) SHA1(e9fc7161d0566e36307c45b7132e2262c0af4845) )
	ROM_LOAD( "f04.bin",      0x1800, 0x0800, CRC(4c6a5a6d) SHA1(366516f63c9b5239e703e4dfb672659049ddbf44) )
	ROM_LOAD( "f09.bin",      0x2000, 0x0800, CRC(75fd5ca1) SHA1(45f2dd33f0e437cb95d9373f86490e5432338737) )
	ROM_LOAD( "f10.bin",      0x2800, 0x0800, CRC(e4da2dd4) SHA1(7a53efd5b583f656c87b7d7a5ba7c239ced7d87b) )
	ROM_LOAD( "f11.bin",      0x3000, 0x0800, CRC(42869646) SHA1(a3640b2ace31ce99c056bc14d1d96f3404698d6a) )
	ROM_LOAD( "f12.bin",      0x3800, 0x0800, CRC(a48d7fb0) SHA1(6206036a9d85e87fb7f8a88c17bfe090fc70caf4) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "1h_1_10.bin",  0x0000, 0x0800, CRC(528da705) SHA1(d726ee18b79774c982f88afb2a508eb5d5783193) )
	ROM_LOAD( "mcs_d",        0x0800, 0x0800, CRC(13932a15) SHA1(b8885c555c6ad7021be55c6925a0a0872c1b6abd) )
	ROM_LOAD( "1k_1_11.bin",  0x1000, 0x0800, CRC(4e79ff6b) SHA1(f72386a3766a7fcc7b4b8cedfa58b8d57f911f6f) )
	ROM_LOAD( "mcs_c",        0x1800, 0x0800, CRC(24cfd145) SHA1(08c6599db170dd6ee364c44f70a0f5c0f881b6ef) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "fantazia.clr", 0x0000, 0x0020, CRC(a84ff0af) SHA1(c300dc937c608d2d1c113ca7a53c649472c72379) )
ROM_END

ROM_START( eagle )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "e1",           0x0000, 0x0800, CRC(224c9526) SHA1(4c014d60d4ee80de7f60b4609269461688c181d0) )
	ROM_LOAD( "e2",           0x0800, 0x0800, CRC(cc538ebd) SHA1(4ef3c7363e2dcd9ed99779039ccc50a9f2084dbd) )
	ROM_LOAD( "f03.bin",      0x1000, 0x0800, CRC(29a2b0ab) SHA1(e9fc7161d0566e36307c45b7132e2262c0af4845) )
	ROM_LOAD( "f04.bin",      0x1800, 0x0800, CRC(4c6a5a6d) SHA1(366516f63c9b5239e703e4dfb672659049ddbf44) )
	ROM_LOAD( "e5",           0x2000, 0x0800, CRC(06d378a6) SHA1(99dbe9fc7f95f8fdce86eb5c32bd1ca1bea0ca3c) )
	ROM_LOAD( "e6",           0x2800, 0x0800, CRC(0dea20d5) SHA1(405b51d4e3b1065f78afd2297e075e977ae19196) )
	ROM_LOAD( "e7",           0x3000, 0x0800, CRC(b45af1e8) SHA1(d7020774707234acdaef5c655f667d5ee9e54a13) )
	ROM_LOAD( "e8",           0x3800, 0x0800, CRC(c437a876) SHA1(845941b873970ac62ba9bb6353bee53d0fcfa292) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "e10",          0x0000, 0x0800, CRC(40ce58bf) SHA1(67ea99e1afe4fff3e17252b22d11d3c96a416041) )
	ROM_LOAD( "e12",          0x0800, 0x0200, CRC(628fdeed) SHA1(a798530c65e639fbf00ed3a4e8c428935bf5f38e) )
	ROM_CONTINUE(             0x0c00, 0x0200 )  /* this version of the gfx ROMs has two */
	ROM_CONTINUE(             0x0a00, 0x0200 )  /* groups of 16 sprites swapped */
	ROM_CONTINUE(             0x0e00, 0x0200 )
	ROM_LOAD( "e9",           0x1000, 0x0800, CRC(ba664099) SHA1(9509123bed02a9d47f2c056e1562b80206da5579) )
	ROM_LOAD( "e11",          0x1800, 0x0200, CRC(ee4ec5fd) SHA1(bf08b3f111f780dc8c81275e4e6247388183a8da) )
	ROM_CONTINUE(             0x1c00, 0x0200 )
	ROM_CONTINUE(             0x1a00, 0x0200 )
	ROM_CONTINUE(             0x1e00, 0x0200 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "l06_prom.bin", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( eagle2 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "e1.7f",        0x0000, 0x0800, CRC(45aab7a3) SHA1(52ae0463f363dc0964b976faa2c0c428d85a4f12) )
	ROM_LOAD( "e2",           0x0800, 0x0800, CRC(cc538ebd) SHA1(4ef3c7363e2dcd9ed99779039ccc50a9f2084dbd) )
	ROM_LOAD( "f03.bin",      0x1000, 0x0800, CRC(29a2b0ab) SHA1(e9fc7161d0566e36307c45b7132e2262c0af4845) )
	ROM_LOAD( "f04.bin",      0x1800, 0x0800, CRC(4c6a5a6d) SHA1(366516f63c9b5239e703e4dfb672659049ddbf44) )
	ROM_LOAD( "e5",           0x2000, 0x0800, CRC(06d378a6) SHA1(99dbe9fc7f95f8fdce86eb5c32bd1ca1bea0ca3c) )
	ROM_LOAD( "e6.6",         0x2800, 0x0800, CRC(9f09f8c6) SHA1(47c600629e02357389dd78c7fcaec862e0da4ef0) )
	ROM_LOAD( "e7",           0x3000, 0x0800, CRC(b45af1e8) SHA1(d7020774707234acdaef5c655f667d5ee9e54a13) )
	ROM_LOAD( "e8",           0x3800, 0x0800, CRC(c437a876) SHA1(845941b873970ac62ba9bb6353bee53d0fcfa292) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "e10.2",        0x0000, 0x0800, CRC(25b38ebd) SHA1(f679c2f2cb5892680fec102fafbdfeae156ce373) )
	ROM_LOAD( "e12",          0x0800, 0x0200, CRC(628fdeed) SHA1(a798530c65e639fbf00ed3a4e8c428935bf5f38e) )
	ROM_CONTINUE(             0x0c00, 0x0200 )  /* this version of the gfx ROMs has two */
	ROM_CONTINUE(             0x0a00, 0x0200 )  /* groups of 16 sprites swapped */
	ROM_CONTINUE(             0x0e00, 0x0200 )
	ROM_LOAD( "e9",           0x1000, 0x0800, CRC(ba664099) SHA1(9509123bed02a9d47f2c056e1562b80206da5579) )
	ROM_LOAD( "e11",          0x1800, 0x0200, CRC(ee4ec5fd) SHA1(bf08b3f111f780dc8c81275e4e6247388183a8da) )
	ROM_CONTINUE(             0x1c00, 0x0200 )
	ROM_CONTINUE(             0x1a00, 0x0200 )
	ROM_CONTINUE(             0x1e00, 0x0200 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "l06_prom.bin", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( eagle3 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "e1",           0x0000, 0x0800, CRC(224c9526) SHA1(4c014d60d4ee80de7f60b4609269461688c181d0) )
	ROM_LOAD( "e2",           0x0800, 0x0800, CRC(cc538ebd) SHA1(4ef3c7363e2dcd9ed99779039ccc50a9f2084dbd) )
	ROM_LOAD( "f03.bin",      0x1000, 0x0800, CRC(29a2b0ab) SHA1(e9fc7161d0566e36307c45b7132e2262c0af4845) )
	ROM_LOAD( "f04.bin",      0x1800, 0x0800, CRC(4c6a5a6d) SHA1(366516f63c9b5239e703e4dfb672659049ddbf44) )
	ROM_LOAD( "e5",           0x2000, 0x0800, CRC(06d378a6) SHA1(99dbe9fc7f95f8fdce86eb5c32bd1ca1bea0ca3c) )
	ROM_LOAD( "e6",           0x2800, 0x0800, CRC(0dea20d5) SHA1(405b51d4e3b1065f78afd2297e075e977ae19196) )
	ROM_LOAD( "e7",           0x3000, 0x0800, CRC(b45af1e8) SHA1(d7020774707234acdaef5c655f667d5ee9e54a13) )
	ROM_LOAD( "e8",           0x3800, 0x0800, CRC(c437a876) SHA1(845941b873970ac62ba9bb6353bee53d0fcfa292) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "e10a",         0x0000, 0x0800, CRC(e3c63d4c) SHA1(ad2b22e316da6bb819c58934d51cd4b2819b18f0) )
	ROM_LOAD( "e12",          0x0800, 0x0200, CRC(628fdeed) SHA1(a798530c65e639fbf00ed3a4e8c428935bf5f38e) )
	ROM_CONTINUE(             0x0c00, 0x0200 )  /* this version of the gfx ROMs has two */
	ROM_CONTINUE(             0x0a00, 0x0200 )  /* groups of 16 sprites swapped */
	ROM_CONTINUE(             0x0e00, 0x0200 )
	ROM_LOAD( "e9a",          0x1000, 0x0800, CRC(59429e47) SHA1(b7629c81d122fd1e4d390aa7abba44df898387d3) )
	ROM_LOAD( "e11",          0x1800, 0x0200, CRC(ee4ec5fd) SHA1(bf08b3f111f780dc8c81275e4e6247388183a8da) )
	ROM_CONTINUE(             0x1c00, 0x0200 )
	ROM_CONTINUE(             0x1a00, 0x0200 )
	ROM_CONTINUE(             0x1e00, 0x0200 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "l06_prom.bin", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( spctbird )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "tssa-7f",      0x0000, 0x0800, CRC(45aab7a3) SHA1(52ae0463f363dc0964b976faa2c0c428d85a4f12) )
	ROM_LOAD( "tssa-7h",      0x0800, 0x0800, CRC(8b328f48) SHA1(d4f549e90e0bf1f546e2c3dc5a5a16e0415e709e) )
	ROM_LOAD( "tssa-7k",      0x1000, 0x0800, CRC(29a2b0ab) SHA1(e9fc7161d0566e36307c45b7132e2262c0af4845) )
	ROM_LOAD( "tssa-7m",      0x1800, 0x0800, CRC(99c9166d) SHA1(c108d84330bc958ff2812dc807e68c246a5a5ad5) )
	ROM_LOAD( "tssa-5",       0x2000, 0x0800, CRC(797b6261) SHA1(9a60e504e2aa0201b7311485c0dd411bbe2dc70b) )
	ROM_LOAD( "tssa-6",       0x2800, 0x0800, CRC(4825692c) SHA1(41a7e305c3d93f2245fb0413398d951eab9d16c0) )
	ROM_LOAD( "tssa-7",       0x3000, 0x0800, CRC(b45af1e8) SHA1(d7020774707234acdaef5c655f667d5ee9e54a13) )
	ROM_LOAD( "tssa-8",       0x3800, 0x0800, CRC(c9b77b85) SHA1(00797f126b4cdacd9ec2df7e747aa1892933b8b8) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "tssb-2",       0x0000, 0x0800, CRC(7d23e1f2) SHA1(6902e44ff6f805a8d589c57b236e471b7fb609f8) )
	ROM_LOAD( "tssb-4",       0x0800, 0x0200, CRC(e4977833) SHA1(85aca9bccc6c1a5a2d792a9c4a77ee1b114934c9) )
	ROM_CONTINUE(             0x0c00, 0x0200 )
	ROM_CONTINUE(             0x0a00, 0x0200 )
	ROM_CONTINUE(             0x0e00, 0x0200 )
	ROM_LOAD( "tssb-1",       0x1000, 0x0800, CRC(9b9267c3) SHA1(2bbbff7a8a2d3e4524634de5e1c5a2426612c18f) )
	ROM_LOAD( "tssb-3",       0x1800, 0x0200, CRC(5ca5e233) SHA1(2115faecd07940547d0ee09776da6fcb1a008287) )
	ROM_CONTINUE(             0x1c00, 0x0200 )
	ROM_CONTINUE(             0x1a00, 0x0200 )
	ROM_CONTINUE(             0x1e00, 0x0200 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "l06_prom.bin", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( smooncrs )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "927",          0x0000, 0x0800, CRC(55c5b994) SHA1(3451b121fa22361b2684385cf5d4455fa6963215) )
	ROM_LOAD( "928a",         0x0800, 0x0800, CRC(77ae26d3) SHA1(cbc16a024b73bedff76a6c47336d6ef098e92c53) )
	ROM_LOAD( "929",          0x1000, 0x0800, CRC(716eaa10) SHA1(780fc785e6651f19dc1a0ccf48cf9485d6562a71) )
	ROM_LOAD( "930",          0x1800, 0x0800, CRC(cea864f2) SHA1(aaaf9f8dd126dfb4a4f52f39863fee02a56a6485) )
	ROM_LOAD( "931",          0x2000, 0x0800, CRC(702c5f51) SHA1(5ba8d87c93c4810b8e7c2ad4ee376cd806e83686) )
	ROM_LOAD( "932a",         0x2800, 0x0800, CRC(e6a2039f) SHA1(f0f240dd8ac7cd2d9994cb7341b59d7a0a3eaf26) )
	ROM_LOAD( "933",          0x3000, 0x0800, CRC(73783cee) SHA1(69760e25ba22645572ec16b4f9136ee84ed0c766) )
	ROM_LOAD( "934",          0x3800, 0x0800, CRC(c1a14aa2) SHA1(99f6b01a0acd5e936d6ae61c13599db603b73191) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "epr203",       0x0000, 0x0800, CRC(be26b561) SHA1(cc27de6888eaf4ee18c0d37d9bcb528dd282b838) )
	ROM_LOAD( "mcs_d",        0x0800, 0x0800, CRC(13932a15) SHA1(b8885c555c6ad7021be55c6925a0a0872c1b6abd) )
	ROM_LOAD( "epr202",       0x1000, 0x0800, CRC(26c7e800) SHA1(034192e5e2cbac4b66a9828f5ec2311c2c368781) )
	ROM_LOAD( "mcs_c",        0x1800, 0x0800, CRC(24cfd145) SHA1(08c6599db170dd6ee364c44f70a0f5c0f881b6ef) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mmi6331.6l", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END



ROM_START( mooncptc )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "mc1.bin",          0x0000, 0x0800, CRC(16f17cd5) SHA1(b3bbea2d91a6deeda7e045fc694ea3afb2e88a29) )
	ROM_LOAD( "mc2.bin",          0x0800, 0x0800, CRC(e2128805) SHA1(98aba5fd27eb7a3fdd3006f47c1eb7a0ea9d9a6f) )
	ROM_LOAD( "mc3.bin",          0x1000, 0x0800, CRC(716eaa10) SHA1(780fc785e6651f19dc1a0ccf48cf9485d6562a71) ) //  = 929                   smooncrs   Super Moon Cresta
	ROM_LOAD( "mc4.bin",          0x1800, 0x0800, CRC(bd45cd8f) SHA1(045e8b56d46a11c6f974ea9455618d067ba0ef50) )
	ROM_LOAD( "mc5.bin",          0x2000, 0x0800, CRC(9a1e0528) SHA1(d77e7daa9fc79ea0503f93af8c714441c7fd9ca5) )
	ROM_LOAD( "mc6.bin",          0x2800, 0x0800, CRC(f0230048) SHA1(8a4363323530b21ee14dbe608aa0de5241d8bb39) )
	ROM_LOAD( "mc7.bin",          0x3000, 0x0800, CRC(eafd4d02) SHA1(b75ed5358646d8a377ccd1f282136e638aaa9d0c) )
	ROM_LOAD( "mc8.bin",          0x3800, 0x0800, CRC(ccee32f8) SHA1(a4abd8d66209a29f63a56dfc9b6f9f834886c747) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "mc12.bin",     0x0000, 0x0800, CRC(528da705) SHA1(d726ee18b79774c982f88afb2a508eb5d5783193) ) // == 1h_1_10.bin
	ROM_LOAD( "mc14.bin",     0x0800, 0x0200, CRC(5a4b17ea) SHA1(8a879dc34fdecc8a121c4a87abb981212fb05945) ) // == 12.chr
	ROM_CONTINUE(             0x0c00, 0x0200 )  /* this version of the gfx ROMs has two */
	ROM_CONTINUE(             0x0a00, 0x0200 )  /* groups of 16 sprites swapped */
	ROM_CONTINUE(             0x0e00, 0x0200 )
	ROM_LOAD( "mc11.bin",     0x1000, 0x0800, CRC(4e79ff6b) SHA1(f72386a3766a7fcc7b4b8cedfa58b8d57f911f6f) ) // == 1k_1_11.bin
	ROM_LOAD( "mc13.bin",     0x1800, 0x0200, CRC(e0edccbd) SHA1(0839a4c9b6e863d12253ae8e1732e80e08702228) ) // == 11.chr
	ROM_CONTINUE(             0x1c00, 0x0200 )
	ROM_CONTINUE(             0x1a00, 0x0200 )
	ROM_CONTINUE(             0x1e00, 0x0200 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mmi6331.6l", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END


ROM_START( sstarcrs )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "ss1",          0x0000, 0x0800, CRC(2ff72897) SHA1(c34c149ee32dd3318a04eda8928f0cd5f997d184) )
	ROM_LOAD( "ss2",          0x0800, 0x0800, CRC(565e7880) SHA1(cd8d1154c2e970e863bd489856819cd34848570b) )
	ROM_LOAD( "ss3",          0x1000, 0x0800, CRC(a1939def) SHA1(c9be93d325dde496d89e0735ec4e7abca932c0f6) )
	ROM_LOAD( "ss4",          0x1800, 0x0800, CRC(a332e012) SHA1(7b32001fe342dcae2bce1c39dd1e75c6b5806199) )
	ROM_LOAD( "ss5",          0x2000, 0x0800, CRC(b9e58453) SHA1(60890208a5dee6e5e52e4ffafcb3501de080adf8) )
	ROM_LOAD( "ss6",          0x2800, 0x0800, CRC(7cbb5bc8) SHA1(5158d798e9b4649636f9fecf29738f271a3edef2) )
	ROM_LOAD( "ss7",          0x3000, 0x0800, CRC(57713b91) SHA1(ba01ed3f047ebbd0f9e6956e649bec0e8b730a45) )
	ROM_LOAD( "ss8",          0x3800, 0x0800, CRC(c857e898) SHA1(a596abe4971e65785945a844a783be2dbca559bc) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "ss10",         0x0000, 0x0800, CRC(2a95b8ea) SHA1(b2f7f2d4aca55b8cbbb907c990b27c06b7b2d77b) )
	ROM_LOAD( "ss12",         0x0800, 0x0200, CRC(b92c4c30) SHA1(4abc4c759e401be4edcce4f3f2d7b2b3f1827a99) )
	ROM_CONTINUE(             0x0c00, 0x0200 )
	ROM_CONTINUE(             0x0a00, 0x0200 )
	ROM_CONTINUE(             0x0e00, 0x0200 )
	ROM_LOAD( "ss9",          0x1000, 0x0800, CRC(3661e084) SHA1(bb5b8b7c9c61a0379a3f1eec02d61bbb385cd3e9) )
	ROM_LOAD( "ss11",         0x1800, 0x0200, CRC(95613048) SHA1(b9ba1ca11ef3154a800a90adaa8c491bb944a3a2) )
	ROM_CONTINUE(             0x1c00, 0x0200 )
	ROM_CONTINUE(             0x1a00, 0x0200 )
	ROM_CONTINUE(             0x1e00, 0x0200 )

	ROM_REGION( 0x0020, "proms", 0 )
	// not present in this set
	ROM_LOAD( "mmi6331.6l", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END

ROM_START( fantastc )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "f1",           0x0000, 0x1000, CRC(8019f0b7) SHA1(b0a611d1cbb92874a6534fd27c6ac57141668913) )
	ROM_LOAD( "f2",           0x1000, 0x1000, CRC(988a9bc6) SHA1(468d483f285ed587f3be81431f22fdbaa3c221cc) )
	ROM_LOAD( "f3",           0x2000, 0x1000, CRC(a3c0cc0b) SHA1(a1b12f1c4187d8db1b267a992db6cd297aeea1bc) )
	ROM_LOAD( "f4",           0x3000, 0x1000, CRC(c1361be8) SHA1(5c8512747927096dd75e6095deb12dfe637f9096) )
	ROM_LOAD( "f5",           0x4000, 0x1000, CRC(6787e93f) SHA1(4ed784ce600fc88efc4865a361f5427027d3419a) )
	ROM_LOAD( "f6",           0x5000, 0x1000, CRC(597029ae) SHA1(38ea1348ac35bd1e6190f395ccb22f16cc30133d) )
	ROM_LOAD( "f7",           0x6000, 0x1000, CRC(8de08d9a) SHA1(ba48a23236f2b26aa17b509daddc4b3e75424d06) )
	ROM_LOAD( "f8",           0x7000, 0x1000, CRC(489e2fb7) SHA1(f7f641c3c6874eae0c9bb3920aa916f8b99a9285) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "37",           0x0000, 0x1000, CRC(3a54f749) SHA1(41e3c479b268de21ae0fd4f7986eb666ee58ad83) )
	ROM_LOAD( "38",           0x1000, 0x1000, CRC(88b71264) SHA1(60c2eb49f16b94b27625045c78c864e299b60d6b) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "prom-74g138", 0x0000, 0x0020, CRC(800f5718) SHA1(5387b24934c8130726180159e207943211215ae2) )
ROM_END

ROM_START( timefgtr )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "tp01",           0x0000, 0x1000, CRC(ba8b3e70) SHA1(cb930022e462319721013f343e513f4a4957b89e) )
	ROM_LOAD( "tp02",           0x1000, 0x1000, CRC(796158c0) SHA1(bc02131a9af1773839ae0aba0225b3160ae632c4) )
	ROM_LOAD( "tp03",           0x2000, 0x1000, CRC(fe6a1c98) SHA1(f4a4e61cc96d93cd21e79b0aa3ddc158a7a034a0) )
	ROM_LOAD( "tp04",           0x3000, 0x1000, CRC(eff73185) SHA1(8538f1b63b051d6d3892ebedc76f45c3cf02cbab) )
	ROM_LOAD( "tp05",           0x4000, 0x1000, CRC(85023e4a) SHA1(afc76ba15d6278c45bf50e9bafcb72a0beb69d4d) )
	ROM_LOAD( "tp06",           0x5000, 0x1000, CRC(b6b8aaf9) SHA1(e25e59ee653b13437c412f1aeb8d7c670e34b39f) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "tp07",           0x0000, 0x1000, CRC(5f57342c) SHA1(000985613d620cbcafbd24351bd4b02f037430a9) )
	ROM_LOAD( "tp09",           0x1000, 0x1000, CRC(636fd772) SHA1(6567992488f0125c082a164f1043c9384736c665) )
	ROM_LOAD( "tp08",           0x2000, 0x1000, CRC(2dc3c48b) SHA1(f4ddf5fce909a1de3efbcaf2ff2e4a8d1ea06516) )
	ROM_LOAD( "tp10",           0x3000, 0x1000, CRC(b27b450c) SHA1(16131583133fe33b61d4f51a860f41d43011bc50) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "prom", 0x0000, 0x0020, NO_DUMP )
ROM_END

ROM_START( kong )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "1",   0x0000, 0x1000, CRC(a206beb5) SHA1(5fea9584b4e3ae076178f6965f0743b9b90b15fc) )
	ROM_LOAD( "2",   0x1000, 0x1000, CRC(d75597b6) SHA1(df9dc99e4f0e864a91ae170d993838db8677f70f) )
	ROM_LOAD( "3",   0x2000, 0x1000, CRC(54e0b87b) SHA1(cfcc64fce36bf8250966576a34768ed7e8857783) )
	ROM_LOAD( "4",   0x3000, 0x1000, CRC(356c4ca2) SHA1(e95d219b013a1b066653b566a84c03c035a03073) )
	ROM_LOAD( "5",   0x4000, 0x1000, CRC(2d295976) SHA1(79e26c55e06b894bab403de77d76260c2bb3baf0) )
	ROM_LOAD( "6",   0x5000, 0x1000, CRC(77131cca) SHA1(900948988f2f6de6b572e5e489a7954eca812278) )
	ROM_LOAD( "7",   0x6000, 0x1000, CRC(3d5ec3f1) SHA1(b382e4a2d2915db190a1578b0ad51ca9b94d521b) )
	ROM_LOAD( "8",   0x7000, 0x1000, CRC(015fe5e5) SHA1(d246d1c791eb4d85e59a826ed0accd9f1da483bb) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "9",   0x0000, 0x0800, CRC(fe42a052) SHA1(3f5893728c1aa73f28ff4841a388124b15bbf1b7) )
	ROM_LOAD( "10",   0x1000, 0x0800, CRC(91fa187e) SHA1(f32741a06a3d9ba4b7d3a5552f796a27d9fa1abf) )
	ROM_LOAD( "11",   0x0800, 0x0800, CRC(ad2b2cdd) SHA1(01a5db01c4fa07707823436a28d40cfd2b80be23) )
	ROM_LOAD( "12",   0x1800, 0x0800, CRC(b74724df) SHA1(d72d0831e3806f49a07ae3333d7a29fccaf6d65e) )

	ROM_REGION( 0x2000, "unk", 0 )
	// what is this, speech? the video at https://www.youtube.com/watch?v=HTZEVKoYlGM shows the game apparently talking (2nd game, after The Pit)
	// The video however seems to show a game closer to ckongg (a bootleg of Crazy Kong) rather than this version of Kong which is rewritten from scratch
	// It could be "GORILA" as seen in the Taito do Brasil cabinet flyers
	ROM_LOAD( "13",   0x0000, 0x1000, CRC(7d33ca0a) SHA1(8a65a4b913559e3fd17f6abb381db1ab813fc8f2) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "prom", 0x0000, 0x0020, NO_DUMP )
ROM_END

ROM_START( mooncmw )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "60.1x",      0x0000, 0x0800, CRC(322859e6) SHA1(292dccb66c38c8de837ec3ac10928d092494958e) )
	ROM_LOAD( "61.2x",      0x0800, 0x0800, CRC(c249902d) SHA1(0015461173fb991fd99c824e0eab054c3c17d0f1) )
	ROM_LOAD( "62.3x",      0x1000, 0x0800, CRC(29a2b0ab) SHA1(e9fc7161d0566e36307c45b7132e2262c0af4845) )
	ROM_LOAD( "63.4x",      0x1800, 0x0800, CRC(4c6a5a6d) SHA1(366516f63c9b5239e703e4dfb672659049ddbf44) )
	ROM_LOAD( "64.5x",      0x2000, 0x0800, CRC(06d378a6) SHA1(99dbe9fc7f95f8fdce86eb5c32bd1ca1bea0ca3c) )
	ROM_LOAD( "65.6x",      0x2800, 0x0800, CRC(6e84a927) SHA1(82e8e825d157c3c947a3a222bca059a735169c7d) )
	ROM_LOAD( "66.7x",      0x3000, 0x0800, CRC(f23cd8ce) SHA1(a77e7eca239de6a72a8cabed6444ae8efb9e40bd) )
	ROM_LOAD( "67.8x",      0x3800, 0x0800, CRC(66da55d5) SHA1(39e2f6107e77ee97860147f64b9673cd9a2ae612) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "68.1h",      0x0000, 0x0800, CRC(78663d86) SHA1(8648a3e60259404a05ad58b1641190e5b33a24eb) )
	ROM_CONTINUE(           0x0800, 0x0200 )
	ROM_CONTINUE(           0x0c00, 0x0200 )
	ROM_CONTINUE(           0x0a00, 0x0200 )
	ROM_CONTINUE(           0x0e00, 0x0200 )
	ROM_LOAD( "69.1k",      0x1000, 0x0800, CRC(162c50d3) SHA1(67d9c87782cf29c443590d7ad687fbeaa6218346) )
	ROM_CONTINUE(           0x1800, 0x0200 )
	ROM_CONTINUE(           0x1c00, 0x0200 )
	ROM_CONTINUE(           0x1a00, 0x0200 )
	ROM_CONTINUE(           0x1e00, 0x0200 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "prom-sn74s288n-71.6l", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END


ROM_START( starfgmc )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "sei-sf-a2.bin",      0x0000, 0x0800, CRC(322859e6) SHA1(292dccb66c38c8de837ec3ac10928d092494958e) )
	ROM_LOAD( "sei-sf-a1.bin",      0x0800, 0x0800, CRC(7fed0654) SHA1(5a0336a5fd2f34eb901da6cf703221bc2d3f954e) )
	ROM_LOAD( "sei-sf-b2.bin",      0x1000, 0x0800, CRC(935f7435) SHA1(433304c8c6f495c1098371770dca10e5ef5750c6) )
	ROM_LOAD( "sei-sf-c1.bin",      0x1800, 0x0800, CRC(29d54869) SHA1(71e0862f24c26a5262c76140a258388b56a9af19) )
	ROM_LOAD( "sei-sf-d1.bin",      0x2000, 0x0800, CRC(9126cca6) SHA1(ec8ae1d55010645cc4243a1bbf8343d10194136a) )
	ROM_LOAD( "sei-sf-e2.bin",      0x2800, 0x0800, CRC(9d394261) SHA1(e1e6d09ad0c01588d8e4b4665a62264163f80a3d) )
	ROM_LOAD( "sei-sf-f2.bin",      0x3000, 0x0800, CRC(94f161dd) SHA1(5e5e5a35e8a1478755b5569018ebf54c513d3c64) )
	ROM_LOAD( "sei-sf-f1.bin",      0x3800, 0x0800, CRC(6ced7f80) SHA1(4710d6a35ac38642893f2ceb27799f5caf24aa3b) )

	ROM_REGION( 0x2000, "gfx1", 0 ) // check loading
	ROM_LOAD( "sei-sf-jh2.bin",  0x0000, 0x0800, CRC(8edba3cd) SHA1(296cf08ee388d1aec7388a9789ab18db54c64118) )
	ROM_LOAD( "sei-sf-jh3.bin",  0x0800, 0x0800, CRC(f5a871a9) SHA1(6afa971ffb15b7b97a6880db26a1f599d23f8655) )
	ROM_LOAD( "sei-sf-lk2.bin",  0x1000, 0x0800, CRC(44b0f06a) SHA1(26cb43239e9150dc867b19d3b48b5a766778ca0a) )
	ROM_LOAD( "sei-sf-lk3.bin",  0x1800, 0x0800, CRC(773b1ee6) SHA1(6d0ca6d0d01f408a2cb1ae40a4dab903eccea528) )


	ROM_REGION( 0x0020, "proms", 0 ) // no prom included so using the regular one
	ROM_LOAD( "prom-sn74s288n-71.6l", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END


ROM_START( spcdrag )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a.bin",      0x0000, 0x0800, CRC(38cc9839) SHA1(71c5853fc14a9c0b93e3b7660b925021680a0fe1) )
	ROM_LOAD( "b.bin",      0x0800, 0x0800, CRC(419fa8d6) SHA1(709b096d43c15cbfb98745e1f5e7c1bc921e3241) )
	ROM_LOAD( "c.bin",      0x1000, 0x0800, CRC(a1939def) SHA1(c9be93d325dde496d89e0735ec4e7abca932c0f6) )
	ROM_LOAD( "d.bin",      0x1800, 0x0800, CRC(cbcf17c5) SHA1(9aa3ca6dc30e4a19ed2bdb2be6ba90bde4cb7542) )
	ROM_LOAD( "em.bin",     0x2000, 0x0800, CRC(eb81c19c) SHA1(e5dd61704938c837b87a3155d54698482235c513) )
	ROM_LOAD( "fm.bin",     0x2800, 0x0800, CRC(757b7672) SHA1(d042e4bc17d2a8c9f1db55d57d5c235338cdb20c) )
	ROM_LOAD( "g.bin",      0x3000, 0x0800, CRC(57713b91) SHA1(ba01ed3f047ebbd0f9e6956e649bec0e8b730a45) )
	ROM_LOAD( "h.bin",      0x3800, 0x0800, CRC(159ad847) SHA1(9d46f380c868ac07964e571c54e800c683a6a679) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "203.bin",  0x0000, 0x0800, CRC(a2e82527) SHA1(5e9236ba102728213b4651db984b3a169b4a0410) )
	ROM_LOAD( "172.bin",  0x0800, 0x0800, CRC(13932a15) SHA1(b8885c555c6ad7021be55c6925a0a0872c1b6abd) )
	ROM_LOAD( "202.bin",  0x1000, 0x0800, CRC(80c3ad74) SHA1(0fd2269543d123bd427f5a648a17f8bee65b20a2) )
	ROM_LOAD( "171.bin",  0x1800, 0x0800, CRC(24cfd145) SHA1(08c6599db170dd6ee364c44f70a0f5c0f881b6ef) )

	ROM_REGION( 0x0020, "proms", 0 )
	// not present in this set
	ROM_LOAD( "mmi6331.6l", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END

ROM_START( spcdraga )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.7g",      0x0000, 0x0800, CRC(38cc9839) SHA1(71c5853fc14a9c0b93e3b7660b925021680a0fe1) )
	ROM_LOAD( "2.7g",      0x0800, 0x0800, CRC(29e00ae4) SHA1(574bdfb621e084485e6621229cd569486831e4ba) )
	ROM_LOAD( "3.7g",      0x1000, 0x0800, CRC(a1939def) SHA1(c9be93d325dde496d89e0735ec4e7abca932c0f6) )
	ROM_LOAD( "4.7g",      0x1800, 0x0800, CRC(068f8830) SHA1(e12d590401878d9f2695e5c7aa38387ed9ccfb06) )
	ROM_LOAD( "5.10g",     0x2000, 0x0800, CRC(32cd9adc) SHA1(3143690712465d092d6c63f4826f220839d78958) )
	ROM_LOAD( "6.10g",     0x2800, 0x0800, CRC(50db67c5) SHA1(69ad219332ac0d9f4e328b314f7bdc34d5599393) )
	ROM_LOAD( "7.10g",     0x3000, 0x0800, CRC(22415271) SHA1(60b1ca2dc044c0863c6f38280a3bd0ff9397c869) )
	ROM_LOAD( "8.10g",     0x3800, 0x0800, CRC(159ad847) SHA1(9d46f380c868ac07964e571c54e800c683a6a679) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "a2.7a",  0x0000, 0x0800, CRC(38b042dd) SHA1(bd452dae4cbc22a900cf783f84d1f9d8cb1218f9) )
	ROM_LOAD( "a4.7a",  0x0800, 0x0200, CRC(5a4b17ea) SHA1(8a879dc34fdecc8a121c4a87abb981212fb05945) )
	ROM_CONTINUE(       0x0c00, 0x0200 )
	ROM_CONTINUE(       0x0a00, 0x0200 )
	ROM_CONTINUE(       0x0e00, 0x0200 )
	ROM_LOAD( "a1.9a",  0x1000, 0x0800, CRC(24441ab3) SHA1(8c9d2bd062cb2360f3dd3df2d7d212e9485f91ad) )
	ROM_LOAD( "a3.9a",  0x1800, 0x0200, CRC(e0edccbd) SHA1(0839a4c9b6e863d12253ae8e1732e80e08702228) )
	ROM_CONTINUE(       0x1c00, 0x0200 )
	ROM_CONTINUE(       0x1a00, 0x0200 )
	ROM_CONTINUE(       0x1e00, 0x0200 )

	ROM_REGION( 0x0020, "proms", 0 )
	// not present in this set
	ROM_LOAD( "prom_6331.10f", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END

ROM_START( mooncreg ) // similar to the spcdraga 'Space Dragon (set 2)' set but with original Moon Cresta gfx roms
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "eg1",     0x0000, 0x0800, CRC(a67ca4af) SHA1(0422be6b3549418c19ece3de6dd165e690d40fdd) ) // unique to this set
	ROM_LOAD( "eg2",     0x0800, 0x0800, CRC(b57b140e) SHA1(f436586280e70dded199be52984fb22c0daf2d62) ) // unique to this set
	ROM_LOAD( "eg3",     0x1000, 0x0800, CRC(a1939def) SHA1(c9be93d325dde496d89e0735ec4e7abca932c0f6) ) // == spcdrag/spcdraga
	ROM_LOAD( "eg4",     0x1800, 0x0800, CRC(068f8830) SHA1(e12d590401878d9f2695e5c7aa38387ed9ccfb06) ) // == spcdraga
	ROM_LOAD( "eg5",     0x2000, 0x0800, CRC(32cd9adc) SHA1(3143690712465d092d6c63f4826f220839d78958) ) // == spcdraga
	ROM_LOAD( "eg6",     0x2800, 0x0800, CRC(3a4b62d9) SHA1(955603f1ca7c8e7a488a6b33dabed0ac12aa8050) ) // unique to this set
	ROM_LOAD( "eg7",     0x3000, 0x0800, CRC(22415271) SHA1(60b1ca2dc044c0863c6f38280a3bd0ff9397c869) ) // == spcdraga
	ROM_LOAD( "eg8",     0x3800, 0x0800, CRC(7b9cc105) SHA1(d12bb1d86eddc08ab24c0e8f8b4cc6011fa70f5a) ) // unique to this set

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "eg_2b",        0x0000, 0x0800, CRC(fb0f1f81) SHA1(38a6679a8b69bc1870a0e67e692131c42f9535c8) )
	ROM_LOAD( "eg_4b",        0x0800, 0x0800, CRC(13932a15) SHA1(b8885c555c6ad7021be55c6925a0a0872c1b6abd) )
	ROM_LOAD( "eg_1b",        0x1000, 0x0800, CRC(631ebb5a) SHA1(5bc9493afa76c55858b8c8849524cbc77dc838fc) )
	ROM_LOAD( "eg_3b",        0x1800, 0x0800, CRC(24cfd145) SHA1(08c6599db170dd6ee364c44f70a0f5c0f881b6ef) )

	ROM_REGION( 0x0020, "proms", 0 )
	// not present in this set
	ROM_LOAD( "prom_6331.10f", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END


ROM_START( mooncrsl ) // similar to above
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "01.bin",     0x0000, 0x0800, CRC(a67ca4af) SHA1(0422be6b3549418c19ece3de6dd165e690d40fdd) )
	ROM_LOAD( "02.bin",     0x0800, 0x0800, CRC(16edce3e) SHA1(59a21d69aba42be265fa3f9166a031ef510c59ed) )
	ROM_LOAD( "03.bin",     0x1000, 0x0800, CRC(a1939def) SHA1(c9be93d325dde496d89e0735ec4e7abca932c0f6) )
	ROM_LOAD( "04.bin",     0x1800, 0x0800, CRC(068f8830) SHA1(e12d590401878d9f2695e5c7aa38387ed9ccfb06) )
	ROM_LOAD( "05.bin",     0x2000, 0x0800, CRC(28ae612d) SHA1(f1c224be95659e716f0d4f0dc2704503cfc46c07) )
	ROM_LOAD( "06.bin",     0x2800, 0x0800, CRC(803da987) SHA1(41ce0401a142f2fc41ea2db95d1067a2386e9e70) )
	ROM_LOAD( "07.bin",     0x3000, 0x0800, CRC(8e9ac0fc) SHA1(ddc11ad20ecbd954098becf7d7a1bbe6cddeda1b) )
	ROM_LOAD( "08.bin",     0x3800, 0x0800, CRC(020a8e2f) SHA1(b64438cb043252565d8a4f3f58f4a4f78a276ba2) )

	ROM_REGION( 0x2000, "gfx1", 0 ) // not present in this set
	ROM_LOAD( "mcs_b",        0x0000, 0x0800, CRC(fb0f1f81) SHA1(38a6679a8b69bc1870a0e67e692131c42f9535c8) )
	ROM_LOAD( "mcs_d",        0x0800, 0x0800, CRC(13932a15) SHA1(b8885c555c6ad7021be55c6925a0a0872c1b6abd) )
	ROM_LOAD( "mcs_a",        0x1000, 0x0800, CRC(631ebb5a) SHA1(5bc9493afa76c55858b8c8849524cbc77dc838fc) )
	ROM_LOAD( "mcs_c",        0x1800, 0x0800, CRC(24cfd145) SHA1(08c6599db170dd6ee364c44f70a0f5c0f881b6ef) )

	ROM_REGION( 0x0020, "proms", 0 ) // not present in this set
	ROM_LOAD( "mmi6331.6l", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END


ROM_START( stera )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "stera.1",      0x0000, 0x0800, CRC(cd04fea8) SHA1(f3ba58f276c784f4ad4a53f9f961c269faa8fa87) )
	ROM_LOAD( "stera.2",      0x0800, 0x0800, CRC(ccd1878e) SHA1(cfa4913a3cd3e58998bd983c9078af58560cfbd6) )
	ROM_LOAD( "stera.3",      0x1000, 0x0800, CRC(29a2b0ab) SHA1(e9fc7161d0566e36307c45b7132e2262c0af4845) )
	ROM_LOAD( "stera.4",      0x1800, 0x0800, CRC(4c6a5a6d) SHA1(366516f63c9b5239e703e4dfb672659049ddbf44) )
	ROM_LOAD( "stera.5",      0x2000, 0x0800, CRC(06d378a6) SHA1(99dbe9fc7f95f8fdce86eb5c32bd1ca1bea0ca3c) )
	ROM_LOAD( "stera.6",      0x2800, 0x0800, CRC(6e84a927) SHA1(82e8e825d157c3c947a3a222bca059a735169c7d) )
	ROM_LOAD( "stera.7",      0x3000, 0x0800, CRC(b45af1e8) SHA1(d7020774707234acdaef5c655f667d5ee9e54a13) )
	ROM_LOAD( "stera.8",      0x3800, 0x0800, CRC(37f19956) SHA1(cb59ae3bb06f56baa0898baeae8b3810263e046b) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "stera.10",  0x0000, 0x0800, CRC(528da705) SHA1(d726ee18b79774c982f88afb2a508eb5d5783193) )
	ROM_LOAD( "stera.12",  0x0800, 0x0800, CRC(13932a15) SHA1(b8885c555c6ad7021be55c6925a0a0872c1b6abd) )
	ROM_LOAD( "stera.11",  0x1000, 0x0800, CRC(4e79ff6b) SHA1(f72386a3766a7fcc7b4b8cedfa58b8d57f911f6f) )
	ROM_LOAD( "stera.9",   0x1800, 0x0800, CRC(24cfd145) SHA1(08c6599db170dd6ee364c44f70a0f5c0f881b6ef) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "stera.6l", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( mooncrgx )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "1",            0x0000, 0x0800, CRC(84cf420b) SHA1(82c979467c51df699337d5878340d05bee606480) )
	ROM_LOAD( "2",            0x0800, 0x0800, CRC(4c2a61a1) SHA1(a3759bd2c062f2843cd5b812529c798d5d12086c) )
	ROM_LOAD( "3",            0x1000, 0x0800, CRC(1962523a) SHA1(56ea003c3ff37c2bc33383207fccde0ba0ed781a) )
	ROM_LOAD( "4",            0x1800, 0x0800, CRC(75dca896) SHA1(017d04501d3d1305491ba843d92ebd74d47d2f9c) )
	ROM_LOAD( "5",            0x2000, 0x0800, CRC(32483039) SHA1(23baf136d5b7fc02f999dcb31b8daf68b6ffafd1) )
	ROM_LOAD( "6",            0x2800, 0x0800, CRC(43f2ab89) SHA1(f7f0802a12fd89d61f6f00044e077f34a9d3955f) )
	ROM_LOAD( "7",            0x3000, 0x0800, CRC(1e9c168c) SHA1(891dc159dfc343322c3241980a0ef76dee510ca9) )
	ROM_LOAD( "8",            0x3800, 0x0800, CRC(5e09da94) SHA1(677890912db12df6fa2cb515c198f8ac3f7187af) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "1h_1_10.bin",  0x0000, 0x0800, CRC(528da705) SHA1(d726ee18b79774c982f88afb2a508eb5d5783193) )
	ROM_LOAD( "12.chr",       0x0800, 0x0200, CRC(5a4b17ea) SHA1(8a879dc34fdecc8a121c4a87abb981212fb05945) )
	ROM_CONTINUE(             0x0c00, 0x0200 )
	ROM_CONTINUE(             0x0a00, 0x0200 )
	ROM_CONTINUE(             0x0e00, 0x0200 )
	ROM_LOAD( "9.chr",        0x1000, 0x0800, CRC(70df525c) SHA1(f771293494a2234bf80f206ecf1e88773322e503) )
	ROM_LOAD( "11.chr",       0x1800, 0x0200, CRC(e0edccbd) SHA1(0839a4c9b6e863d12253ae8e1732e80e08702228) )
	ROM_CONTINUE(             0x1c00, 0x0200 )
	ROM_CONTINUE(             0x1a00, 0x0200 )
	ROM_CONTINUE(             0x1e00, 0x0200 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "l06_prom.bin", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( moonqsr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mq1",          0x0000, 0x0800, CRC(132c13ec) SHA1(d95166b025442f184e44a70312fb3b4f6366f324) )
	ROM_LOAD( "mq2",          0x0800, 0x0800, CRC(c8eb74f1) SHA1(4efa85c40349852da47a0f725ae06873efe4ce1c) )
	ROM_LOAD( "mq3",          0x1000, 0x0800, CRC(33965a89) SHA1(92912cea76a472d9b709c664d9818844a07fcc32) )
	ROM_LOAD( "mq4",          0x1800, 0x0800, CRC(a3861d17) SHA1(d7037d93b7838ccdd9a6a1a1476571cfa869fca1) )
	ROM_LOAD( "mq5",          0x2000, 0x0800, CRC(8bcf9c67) SHA1(7af0d9308d20c52675301acf5d1a5d62358352a6) )
	ROM_LOAD( "mq6",          0x2800, 0x0800, CRC(5750cda9) SHA1(17c2bc38037833fdb8923d4a2262264386ef916b) )
	ROM_LOAD( "mq7",          0x3000, 0x0800, CRC(78d7fe5b) SHA1(4085562a0af94c65dad2a3550409727e597c0d5b) )
	ROM_LOAD( "mq8",          0x3800, 0x0800, CRC(4919eed5) SHA1(526aaedd25e0f7c525eb7c66519218ae09b0407e) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "mqb",          0x0000, 0x0800, CRC(b55ec806) SHA1(fb52e53dfa3ae9dec162622d22de9cfdb0b5f9d6) )
	ROM_LOAD( "mqd",          0x0800, 0x0800, CRC(9e7d0e13) SHA1(18951080d307ac13344f89745f671595e26d282c) )
	ROM_LOAD( "mqa",          0x1000, 0x0800, CRC(66eee0db) SHA1(eeb08efd226e15e248999558240488ffd0e39688) )
	ROM_LOAD( "mqc",          0x1800, 0x0800, CRC(a6db5b0d) SHA1(476e197df047e991d2ea3c1fad92c799510f1647) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "vid_e6.bin",   0x0000, 0x0020, CRC(0b878b54) SHA1(3667aca564ebfef5b88d7f74fabbd16dd23183b4) )
ROM_END

ROM_START( moonal2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ali1",         0x0000, 0x0400, CRC(0dcecab4) SHA1(493628640de1a7e3bb9914ee3459b74cedc599fd) )
	ROM_LOAD( "ali2",         0x0400, 0x0400, CRC(c6ee75a7) SHA1(36503351380f7638069637c22bd06da06da54a1c) )
	ROM_LOAD( "ali3",         0x0800, 0x0400, CRC(cd1be7e9) SHA1(684f1923090f0d53338705f6972778712e27577f) )
	ROM_LOAD( "ali4",         0x0c00, 0x0400, CRC(83b03f08) SHA1(a1fd422051aa7f17f857188b503031cce3fdc275) )
	ROM_LOAD( "ali5",         0x1000, 0x0400, CRC(6f3cf61d) SHA1(e238ed6f9c0813f0177abe9090e29562529eeef8) )
	ROM_LOAD( "ali6",         0x1400, 0x0400, CRC(e169d432) SHA1(a5189d7322a240863afee7ac0ecf68599498cb87) )
	ROM_LOAD( "ali7",         0x1800, 0x0400, CRC(41f64b73) SHA1(dff786a74575da9fbaca3ac610ad2f367983c7fc) )
	ROM_LOAD( "ali8",         0x1c00, 0x0400, CRC(f72ee876) SHA1(8e50a516f10d77652ff3692bb85adb66bc128e26) )
	ROM_LOAD( "ali9",         0x2000, 0x0400, CRC(b7fb763c) SHA1(4e7c4995f52dec2ea61424c79d79797933dad604) )
	ROM_LOAD( "ali10",        0x2400, 0x0400, CRC(b1059179) SHA1(86de04c82a7604cb57958a52d5585837481f94a5) )
	ROM_LOAD( "ali11",        0x2800, 0x0400, CRC(9e79a1c6) SHA1(2f59e3a2a81a224b163b23bd2c184385f64e1565) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "ali13.1h",     0x0000, 0x0800, CRC(a1287bf6) SHA1(eeeaba4b9e186454a5e2f1c26e333e8fccd97af8) )
	ROM_RELOAD(               0x0800, 0x0800 )
	ROM_LOAD( "ali12.1k",     0x1000, 0x0800, CRC(528f1481) SHA1(e266a75c3109bcfa2a0394f2ed0ac136fc3158ba) )
	ROM_RELOAD(               0x1800, 0x0800 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( moonal2b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ali1",         0x0000, 0x0400, CRC(0dcecab4) SHA1(493628640de1a7e3bb9914ee3459b74cedc599fd) )
	ROM_LOAD( "ali2",         0x0400, 0x0400, CRC(c6ee75a7) SHA1(36503351380f7638069637c22bd06da06da54a1c) )
	ROM_LOAD( "md-2",         0x0800, 0x0800, CRC(8318b187) SHA1(75bb113db1111cd2a335139fa6cb94d9522f5860) )
	ROM_LOAD( "ali5",         0x1000, 0x0400, CRC(6f3cf61d) SHA1(e238ed6f9c0813f0177abe9090e29562529eeef8) )
	ROM_LOAD( "ali6",         0x1400, 0x0400, CRC(e169d432) SHA1(a5189d7322a240863afee7ac0ecf68599498cb87) )
	ROM_LOAD( "ali7",         0x1800, 0x0400, CRC(41f64b73) SHA1(dff786a74575da9fbaca3ac610ad2f367983c7fc) )
	ROM_LOAD( "ali8",         0x1c00, 0x0400, CRC(f72ee876) SHA1(8e50a516f10d77652ff3692bb85adb66bc128e26) )
	ROM_LOAD( "ali9",         0x2000, 0x0400, CRC(b7fb763c) SHA1(4e7c4995f52dec2ea61424c79d79797933dad604) )
	ROM_LOAD( "ali10",        0x2400, 0x0400, CRC(b1059179) SHA1(86de04c82a7604cb57958a52d5585837481f94a5) )
	ROM_LOAD( "md-6",         0x2800, 0x0800, CRC(9cc973e0) SHA1(8d96448794e0869036a3fac7f7fbcad459149a98) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "ali13.1h",     0x0000, 0x0800, CRC(a1287bf6) SHA1(eeeaba4b9e186454a5e2f1c26e333e8fccd97af8) )
	ROM_RELOAD(               0x0800, 0x0800 )
	ROM_LOAD( "ali12.1k",     0x1000, 0x0800, CRC(528f1481) SHA1(e266a75c3109bcfa2a0394f2ed0ac136fc3158ba) )
	ROM_RELOAD(               0x1800, 0x0800 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END


/*

The Pit (on Moon Quasar hardware)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Dumped by Andrew Welburn
on the sunny morning of 2/03/10

http://www.andys-arcade.com

*************************************************
**Do not separate this text file from the roms.**
*************************************************

Nichibutsu Moon Quasar pcb largely unhacked, but
with an odd looking set of roms. One gfx rom
identifies itself as 'The Pit' so that is my only
guess at what the game actually is.

Roms 1 through 8 read well in-circuit in the right
places in the memory map for moon cresta,
(through the fluke 9100) but the game does not
currently run. Roms dumped and verified with no
anomalies.

Rom 9 was placed on the through-socket and has
pins 20 and 18 (/E and A11 respectively) tied to
pin 5 of 8E (A14/A15/MREQ demux). This demux has
some patches, the trace to pin 13 is cut, and is
instead wired to pin 3.

*/

ROM_START( thepitm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin",          0x0000, 0x0800, CRC(0f78d6ea) SHA1(e224b2fb9b4a26295b03af720fa647d4488a1287) )
	ROM_LOAD( "2.bin",          0x0800, 0x0800, CRC(ebacc6eb) SHA1(8b73d96ebe8070fbbb16434aa6a30ec8985ddec6) )
	ROM_LOAD( "3.bin",          0x1000, 0x0800, CRC(14fd0706) SHA1(5d905d8272731c307dc9f96caf4973b28413198e) )
	ROM_LOAD( "4.bin",          0x1800, 0x0800, CRC(613e920f) SHA1(078c7f36ba0145fbbd24bdae4cb6b03c5c27c1cc) )
	ROM_LOAD( "5.bin",          0x2000, 0x0800, CRC(5a791f3f) SHA1(166f07f7fe260e53e611784976792638a25485c1) )
	ROM_LOAD( "6.bin",          0x2800, 0x0800, CRC(0bb37f51) SHA1(32e31678388bad048c829bd43a18dc4e24869840) )
	ROM_LOAD( "7.bin",          0x3000, 0x0800, CRC(4dfdec6f) SHA1(eb88a278860998c343f94e27c8c6c723cffb2dd9) )
	ROM_LOAD( "8.bin",          0x3800, 0x0800, CRC(a39a9189) SHA1(aacd54edca6bc7f7feacd651a0de57b3d9592aad) )
	ROM_LOAD( "9.bin",          0x4000, 0x0800, CRC(2eb90e07) SHA1(92678fc5cfeb7119ce27f042571daa831fa1dad5) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "1h.bin",      0x0000, 0x0800, CRC(00dce65f) SHA1(ba0cce484d1f8693a85b85e0689d107588df9043) )
	ROM_LOAD( "1k.bin",      0x1000, 0x0800, CRC(3ec0056e) SHA1(1dd19e7535ab9abd62b4b32663437f8e8acb91b5) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6l.bin",   0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END


ROM_START( skybase )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "skybase.9a",   0x0000, 0x1000, CRC(845b87a5) SHA1(8a249c1ec921532cb1bb85ed7fec11396634ca38) )
	ROM_LOAD( "skybase.8a",   0x1000, 0x1000, CRC(096785c2) SHA1(a0833bc1984e1f198587195e58b6fed6657922bd) )
	ROM_LOAD( "skybase.7a",   0x2000, 0x1000, CRC(d50c715b) SHA1(3d0fa15514b210bccd4aeed06540122a4f56fd7a) )
	ROM_LOAD( "skybase.6a",   0x3000, 0x1000, CRC(f57edb27) SHA1(4b5c376017700315345241fad96c00478a14fc8f) )
	ROM_LOAD( "skybase.5a",   0x4000, 0x1000, CRC(50365d95) SHA1(9b3d360c9d1df0ebf047bef1b30765ea9bb42b42) )
	ROM_LOAD( "skybase.4a",   0x5000, 0x1000, CRC(cbd6647f) SHA1(7a167c9df6b5f3346c37e5c45d0680b0b29852a6) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "skybase.7t",   0x0000, 0x1000, CRC(9b471686) SHA1(b35831daa8ce57e498c2c4f75763a74c340cfaf0) )
	ROM_LOAD( "skybase.8t",   0x1000, 0x1000, CRC(1cf723da) SHA1(f2e41ab89413298571626d13b2b5853eb35dcb96) )
	ROM_LOAD( "skybase.10t",  0x2000, 0x1000, CRC(fe02e72c) SHA1(bf7c078e984b13dcc12d529904f1096d65e41bec) )
	ROM_LOAD( "skybase.9t",   0x3000, 0x1000, CRC(0871291f) SHA1(2e4e802316b55711bcfeb48d84bacd11afff8cb3) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.bpr",  0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Color prom */
ROM_END


ROM_START( jumpbug )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jb1",          0x0000, 0x1000, CRC(415aa1b7) SHA1(4f9edd7e9720acf085dd8910849c2f2fac5cb547) )
	ROM_LOAD( "jb2",          0x1000, 0x1000, CRC(b1c27510) SHA1(66fbe0b94b6c101cb50d7a3ff78160110415dff9) )
	ROM_LOAD( "jb3",          0x2000, 0x1000, CRC(97c24be2) SHA1(1beb9fbc3a52610b416af8b5fee156d8b6b3125a) )
	ROM_LOAD( "jb4",          0x3000, 0x1000, CRC(66751d12) SHA1(26c68cfb59596ae164ee9ae4a24ddf8dc7a923a7) )
	ROM_LOAD( "jb5",          0x8000, 0x1000, CRC(e2d66faf) SHA1(3dec0796642856359de57afb896cc668c0245b40) )
	ROM_LOAD( "jb6",          0x9000, 0x1000, CRC(49e0bdfd) SHA1(8d89d9cd7134b153264fdc49d2c68e8c14004b0d) )
	ROM_LOAD( "jb7",          0xa000, 0x0800, CRC(83d71302) SHA1(9292088d26ba29fbf8817df03461b8bb6bf27639) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "jbl",          0x0000, 0x0800, CRC(9a091b0a) SHA1(19b88f802ee80ff8901ef99e3688f2869f1a69c5) )
	ROM_LOAD( "jbm",          0x0800, 0x0800, CRC(8a0fc082) SHA1(58b72a3161950a2fb71cdab3f30bb3abb19c7978) )
	ROM_LOAD( "jbn",          0x1000, 0x0800, CRC(155186e0) SHA1(717ddaecc52a4ef03a01fcddb520acdbfb0d722a) )
	ROM_LOAD( "jbi",          0x1800, 0x0800, CRC(7749b111) SHA1(55071ce04708bd52177644298f76ae79d23f6ac9) )
	ROM_LOAD( "jbj",          0x2000, 0x0800, CRC(06e8d7df) SHA1(d04f1503d9fde5aae92652cb9d2eb16bd6a0fe9c) )
	ROM_LOAD( "jbk",          0x2800, 0x0800, CRC(b8dbddf3) SHA1(043de444890a93459789dc99c43ef88ff66b79e4) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "l06_prom.bin", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( jumpbugb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jb1",          0x0000, 0x1000, CRC(415aa1b7) SHA1(4f9edd7e9720acf085dd8910849c2f2fac5cb547) )
	ROM_LOAD( "jb2",          0x1000, 0x1000, CRC(b1c27510) SHA1(66fbe0b94b6c101cb50d7a3ff78160110415dff9) )
	ROM_LOAD( "jb3b",         0x2000, 0x1000, CRC(cb8b8a0f) SHA1(9e8591471dda2cb964ba2a866d4a5a3ef65d8707) )
	ROM_LOAD( "jb4",          0x3000, 0x1000, CRC(66751d12) SHA1(26c68cfb59596ae164ee9ae4a24ddf8dc7a923a7) )
	ROM_LOAD( "jb5b",         0x8000, 0x1000, CRC(7553b5e2) SHA1(6439585e713581dd36cea6324414f803d683216f) )
	ROM_LOAD( "jb6b",         0x9000, 0x1000, CRC(47be9843) SHA1(495d6fc732267bfd19a953b0b70df3f94b3c1e38) )
	ROM_LOAD( "jb7b",         0xa000, 0x0800, CRC(460aed61) SHA1(449ab1bb502f98da74c0955ce1364f8708fd3f81) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "jbl",          0x0000, 0x0800, CRC(9a091b0a) SHA1(19b88f802ee80ff8901ef99e3688f2869f1a69c5) )
	ROM_LOAD( "jbm",          0x0800, 0x0800, CRC(8a0fc082) SHA1(58b72a3161950a2fb71cdab3f30bb3abb19c7978) )
	ROM_LOAD( "jbn",          0x1000, 0x0800, CRC(155186e0) SHA1(717ddaecc52a4ef03a01fcddb520acdbfb0d722a) )
	ROM_LOAD( "jbi",          0x1800, 0x0800, CRC(7749b111) SHA1(55071ce04708bd52177644298f76ae79d23f6ac9) )
	ROM_LOAD( "jbj",          0x2000, 0x0800, CRC(06e8d7df) SHA1(d04f1503d9fde5aae92652cb9d2eb16bd6a0fe9c) )
	ROM_LOAD( "jbk",          0x2800, 0x0800, CRC(b8dbddf3) SHA1(043de444890a93459789dc99c43ef88ff66b79e4) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "l06_prom.bin", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( levers )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "g96059.a8",    0x0000, 0x1000, CRC(9550627a) SHA1(3da9a614622d5b880852fe2bb2e8e4a60afb2d34) )
	ROM_LOAD( "g96060.d8",    0x2000, 0x1000, CRC(5ac64646) SHA1(459755932a033095eff72d78d1e916932964c5cc) )
	ROM_LOAD( "g96061.e8",    0x3000, 0x1000, CRC(9db8e520) SHA1(1ff10e221e45cc4afb77571a171937f8501aa509) )
	ROM_LOAD( "g96062.h8",    0x8000, 0x1000, CRC(7c8e8b3a) SHA1(ad281f801e818ea529be8ec43096212e834f69ef) )
	ROM_LOAD( "g96063.j8",    0x9000, 0x1000, CRC(fa61e793) SHA1(7aad77f3de05a7bd3dcb0c9c97a3cccd1136f352) )
	ROM_LOAD( "g96064.l8",    0xa000, 0x1000, CRC(f797f389) SHA1(b961f0506defa9884ac47b2316884318e1e90bff) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "g95948.n1",    0x0000, 0x0800, CRC(d8a0c692) SHA1(dd64623f4072bcb8c528b5b7b95a7bd858b79d6c) )
							/*0x0800- 0x0fff empty */
	ROM_LOAD( "g95949.s1",    0x1000, 0x0800, CRC(3660a552) SHA1(bebfd30f90da55d6d42945717b9b38d5b0c9623a) )
	ROM_LOAD( "g95946.j1",    0x1800, 0x0800, CRC(73b61b2d) SHA1(fdb75eea1778daa6f9c48243361e418044b471f8) )
							/*0x2000- 0x27ff empty */
	ROM_LOAD( "g95947.m1",    0x2800, 0x0800, CRC(72ff67e2) SHA1(dcc12f17a857271c253d06d5ac170b9d6bb6a2bd) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "g960lev.clr",  0x0000, 0x0020, CRC(01febbbe) SHA1(11b1dab7983ba29e830ccb7f14eb1a99465c9e81) )
ROM_END


ROM_START( checkman )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cm1",          0x0000, 0x0800, CRC(e8cbdd28) SHA1(ba0b41e375b94bbfed6a2c949cc7958474c8ba6e) )
	ROM_LOAD( "cm2",          0x0800, 0x0800, CRC(b8432d4d) SHA1(d331476f1f88b7ef1426bed7442392f369e0650b) )
	ROM_LOAD( "cm3",          0x1000, 0x0800, CRC(15a97f61) SHA1(3c06c734cef1eed68b401d0d36f7ec9126986d73) )
	ROM_LOAD( "cm4",          0x1800, 0x0800, CRC(8c12ecc0) SHA1(1c2d61ef84404b6a524c453a3d339aaaadb38229) )
	ROM_LOAD( "cm5",          0x2000, 0x0800, CRC(2352cfd6) SHA1(77db5f925ee5f83c17a05a78af5191eefe70ca5a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound code */
	ROM_LOAD( "cm13",         0x0000, 0x0800, CRC(0b09a3e8) SHA1(e4e65da306e22f61790f0a68d953cc017c3ce762) )
	ROM_LOAD( "cm14",         0x0800, 0x0800, CRC(47f043be) SHA1(44d8892d93849cbc989561387c0a05baead58446) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "cm11",         0x0000, 0x0800, CRC(8d1bcca0) SHA1(28fc7fb76180820e84d59e6836ed1f8136e8f138) )
	ROM_RELOAD(               0x0800, 0x0800 )
	ROM_LOAD( "cm9",          0x1000, 0x0800, CRC(3cd5c751) SHA1(a769fdd30752da8fb331aa0f7a0181a93f0b3378) )
	ROM_RELOAD(               0x1800, 0x0800 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "checkman.clr", 0x0000, 0x0020, CRC(57a45057) SHA1(d4ec6a54f72640e5b485aa59e206c090e67ff640) )
ROM_END

ROM_START( checkmanj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cm_1.bin",     0x0000, 0x1000, CRC(456a118f) SHA1(7c2e8343360f446af4391012784a1ccfecae3299) )
	ROM_LOAD( "cm_2.bin",     0x1000, 0x1000, CRC(146b2c44) SHA1(80455396a9b1802fcefaec1340b76461c0601bf9) )
	ROM_LOAD( "cm_3.bin",     0x2000, 0x0800, CRC(73e1c945) SHA1(bcf2558958a30e5936f19ff53687f2316e0b822e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound code */
	ROM_LOAD( "cm_4.bin",     0x0000, 0x1000, CRC(923cffa1) SHA1(132822d20de2ad1ecc561e811ca40c5642500631) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "cm_6.bin",     0x0000, 0x0800, CRC(476a7cc3) SHA1(3c343b0dcfb2f4cbec2f8b5854a303a1660fea22) )
	ROM_LOAD( "cm_5.bin",     0x0800, 0x0800, CRC(b3df2b5f) SHA1(519a0894d1794211659abeb6b2a2c610e6c2af25) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "checkman.clr", 0x0000, 0x0020, CRC(57a45057) SHA1(d4ec6a54f72640e5b485aa59e206c090e67ff640) )
ROM_END

ROM_START( dingo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "003.e7",       0x0000, 0x1000, CRC(d088550f) SHA1(13b87995881e484825c45ab4e558ac3d90bf162a) )
	ROM_LOAD( "004.h7",       0x1000, 0x1000, CRC(a228446a) SHA1(4b7e611edd6bce308cc7b17caa068445f5438f4f) )
	ROM_LOAD( "005.j7",       0x2000, 0x0800, CRC(14d680bb) SHA1(e9d84d1a62ed5300c390a7326c16cebd0aceae3b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound code */
	ROM_LOAD( "6.7l",         0x0000, 0x1000, CRC(047092e0) SHA1(24014c999c904b4be571121b0f6808713d95add1) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "001.h1",       0x0000, 0x0800, CRC(1ab1dd4d) SHA1(74ef2226e1f1d2583b0c7718325da193f411a97d) )
	ROM_LOAD( "002.k1",       0x0800, 0x0800, CRC(4be375ee) SHA1(7379b037887baca0f932d910f8f94f7edf39bb26) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "18s030.l6",    0x0000, 0x0020, CRC(3061d0f9) SHA1(5af85499c6219137dc57d9fba79cb5afa3548ab1) )
ROM_END

ROM_START( dingoe )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "unk.2b",       0x0000, 0x1000, CRC(0df7ac6d) SHA1(c1d45a7694848e66426c3510d0749c98e51571cb) )
	ROM_LOAD( "unk.2d",       0x1000, 0x1000, CRC(0881e204) SHA1(4ba59d73e04b5337cfbd68d6a708e7321cb629f1) )
	ROM_LOAD( "unk.3b",       0x2000, 0x1000, BAD_DUMP CRC(0b6aeab5) SHA1(ebfab3227dd23e3e1802b881a5662f634f86e382) ) // both halves identical (bad?)

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound code */
	ROM_LOAD( "unk.1c",       0x0000, 0x0800, CRC(8e354c38) SHA1(87608c1fa55e6fcf482f5d3bcc506a84673719cc) )
	ROM_LOAD( "unk.1d",       0x0800, 0x0800, CRC(092878d6) SHA1(8a3b25e27df5aee2023a7e1a193ab152df171ede) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "unk.4d",       0x0000, 0x0800, CRC(76a00a56) SHA1(2a696b9ce3e148529c731231852dc104729bb916) )
	ROM_LOAD( "unk.4b",       0x0800, 0x0800, CRC(5acf57aa) SHA1(bb05be53728e7867085dad5854fcadfa687ff5d7) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123n.001",  0x0000, 0x0020, CRC(02b11865) SHA1(70053db9635a9194e4372835379a82f6ea64ef83) ) /* Unknown */
ROM_END


ROM_START( mshuttle )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "my05",         0x0000, 0x1000, CRC(83574af1) SHA1(d69c2a0538a49d6c72c3346ac4e3959d91da6c98) )
	ROM_LOAD( "my04",         0x1000, 0x1000, CRC(1cfae2c8) SHA1(6c7eeee70e91b8498c41525dcc60f8086cff8da7) )
	ROM_LOAD( "my03",         0x2000, 0x1000, CRC(c8b8a368) SHA1(140ba60f55285d1e9f7a262634f5ce5c3470ab71) )
	ROM_LOAD( "my02",         0x3000, 0x1000, CRC(b6aeee6e) SHA1(032af7000aebe9d34319231cdb3f2fe5de7158ba) )
	ROM_LOAD( "my01",         0x4000, 0x1000, CRC(def82adc) SHA1(2fb963299468c52d50b7460b55bf69c9659ee21d) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "my09",         0x0000, 0x1000, CRC(3601b380) SHA1(c0b9d1801f58a16449708d514d2fd88e34af340b) )
	ROM_LOAD( "my11",         0x1000, 0x0800, CRC(b659e932) SHA1(3f63c99e81cb93c9553a5e274546525f598d50c4) )
	ROM_LOAD( "my08",         0x2000, 0x1000, CRC(992b06cd) SHA1(8645ccad8169601bbe25b9f2b17b99004c0a584f) )
	ROM_LOAD( "my10",         0x3000, 0x0800, CRC(d860e6ce) SHA1(2912d13bf69496f8f18358a36366a1f60afd0070) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mscprom1.bin", 0x0000, 0x0020, CRC(ea0d1af0) SHA1(cb59e04c02307dfe847e3170cf0a7f62829b6094) )

	ROM_REGION( 0x2000, "samples", 0 )  /* samples */
	ROM_LOAD( "my07",         0x0000, 0x1000, CRC(522a2920) SHA1(a64d821a8ff6bd6e2b0bdb1e632181e65a97363b) )
	ROM_LOAD( "my06",         0x1000, 0x1000, CRC(466415f2) SHA1(a05f8238cdcebe926a564ef6268b3cd677987fa2) ) // sldh
ROM_END

ROM_START( mshuttle2 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "my05",         0x0000, 0x1000, CRC(83574af1) SHA1(d69c2a0538a49d6c72c3346ac4e3959d91da6c98) )
	ROM_LOAD( "my04",         0x1000, 0x1000, CRC(1cfae2c8) SHA1(6c7eeee70e91b8498c41525dcc60f8086cff8da7) )
	ROM_LOAD( "my03",         0x2000, 0x1000, CRC(c8b8a368) SHA1(140ba60f55285d1e9f7a262634f5ce5c3470ab71) )
	ROM_LOAD( "my02",         0x3000, 0x1000, CRC(9804061c) SHA1(d5147e827c5a851f6baadea4a0a3b1deb19dda16) ) // sldh
	ROM_LOAD( "my01",         0x4000, 0x1000, CRC(ca746a61) SHA1(259b1556b0646bf0108b1e3ffbd77bf7238350b0) ) // sldh

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "my09",         0x0000, 0x1000, CRC(3601b380) SHA1(c0b9d1801f58a16449708d514d2fd88e34af340b) )
	ROM_LOAD( "my11",         0x1000, 0x0800, CRC(d860e6ce) SHA1(2912d13bf69496f8f18358a36366a1f60afd0070) ) // sldh
	ROM_LOAD( "my08",         0x2000, 0x1000, CRC(992b06cd) SHA1(8645ccad8169601bbe25b9f2b17b99004c0a584f) )
	ROM_LOAD( "my10",         0x3000, 0x0800, CRC(d860e6ce) SHA1(2912d13bf69496f8f18358a36366a1f60afd0070) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mscprom1.bin", 0x0000, 0x0020, CRC(ea0d1af0) SHA1(cb59e04c02307dfe847e3170cf0a7f62829b6094) )

	ROM_REGION( 0x2000, "samples", 0 )  /* samples */
	ROM_LOAD( "my07",         0x0000, 0x1000, CRC(522a2920) SHA1(a64d821a8ff6bd6e2b0bdb1e632181e65a97363b) )
	ROM_LOAD( "my06",         0x1000, 0x1000, CRC(6d2dd711) SHA1(82e7c7b10258f651943173c968c7fa2bdf937ca9) )
ROM_END



ROM_START( mshuttlej )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "mcs.5",        0x0000, 0x1000, CRC(a5a292b4) SHA1(b4e9d969c762f4114eba88051917df122fc7181f) )
	ROM_LOAD( "mcs.4",        0x1000, 0x1000, CRC(acdc0f9e) SHA1(8cd6d6566fe3f4090ccb625c3c1e5850a371826f) )
	ROM_LOAD( "mcs.3",        0x2000, 0x1000, CRC(c1e3f5d8) SHA1(d3af89d485b1ca21ac879dbe15490dcd1cd64f2a) )
	ROM_LOAD( "mcs.2",        0x3000, 0x1000, CRC(14577703) SHA1(51537982dd06ba44e95e4c7d1f7fa41ff186421d) )

	ROM_LOAD( "mcs.1",        0x4000, 0x1000, CRC(27d46772) SHA1(848a47ba30823a55933bb55792991f0535078f0c) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "my09",         0x0000, 0x1000, CRC(3601b380) SHA1(c0b9d1801f58a16449708d514d2fd88e34af340b) )
	ROM_LOAD( "my11",         0x1000, 0x0800, CRC(b659e932) SHA1(3f63c99e81cb93c9553a5e274546525f598d50c4) )
	ROM_LOAD( "my08",         0x2000, 0x1000, CRC(992b06cd) SHA1(8645ccad8169601bbe25b9f2b17b99004c0a584f) )
	ROM_LOAD( "my10",         0x3000, 0x0800, CRC(d860e6ce) SHA1(2912d13bf69496f8f18358a36366a1f60afd0070) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mscprom1.bin", 0x0000, 0x0020, CRC(ea0d1af0) SHA1(cb59e04c02307dfe847e3170cf0a7f62829b6094) )

	ROM_REGION( 0x2000, "samples", 0 )  /* samples */
	ROM_LOAD( "my07",         0x0000, 0x1000, CRC(522a2920) SHA1(a64d821a8ff6bd6e2b0bdb1e632181e65a97363b) )
	ROM_LOAD( "my06",         0x1000, 0x1000, CRC(6d2dd711) SHA1(82e7c7b10258f651943173c968c7fa2bdf937ca9) )
ROM_END

ROM_START( mshuttlej2 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "ali5.bin",     0x0000, 0x1000, CRC(320fe630) SHA1(df4fe25989783c8851f41c9b4b63dedfa365c1e9) )
	ROM_LOAD( "mcs.4",        0x1000, 0x1000, CRC(acdc0f9e) SHA1(8cd6d6566fe3f4090ccb625c3c1e5850a371826f) )
	ROM_LOAD( "mcs.3",        0x2000, 0x1000, CRC(c1e3f5d8) SHA1(d3af89d485b1ca21ac879dbe15490dcd1cd64f2a) )
	ROM_LOAD( "ali2.bin",     0x3000, 0x1000, CRC(9ed169e1) SHA1(75a24d0fcbdfc7c4e6fa0d8c7f8b4a3bccaa4439) )

	ROM_LOAD( "ali1.bin",     0x4000, 0x1000, CRC(7f8a52d9) SHA1(4e62f6265289bae1a46e60cdd3230e188b2aec3c) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "my09",         0x0000, 0x1000, CRC(3601b380) SHA1(c0b9d1801f58a16449708d514d2fd88e34af340b) )
	ROM_LOAD( "my11",         0x1000, 0x0800, CRC(b659e932) SHA1(3f63c99e81cb93c9553a5e274546525f598d50c4) )
	ROM_LOAD( "my08",         0x2000, 0x1000, CRC(992b06cd) SHA1(8645ccad8169601bbe25b9f2b17b99004c0a584f) )
	ROM_LOAD( "my10",         0x3000, 0x0800, CRC(d860e6ce) SHA1(2912d13bf69496f8f18358a36366a1f60afd0070) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mscprom1.bin", 0x0000, 0x0020, CRC(ea0d1af0) SHA1(cb59e04c02307dfe847e3170cf0a7f62829b6094) )

	ROM_REGION( 0x2000, "samples", 0 )  /* samples */
	ROM_LOAD( "my07",         0x0000, 0x1000, CRC(522a2920) SHA1(a64d821a8ff6bd6e2b0bdb1e632181e65a97363b) )
	ROM_LOAD( "my06.4r",      0x1000, 0x1000, CRC(4162be4d) SHA1(84fa8651796e498a37893ea90ef51b274c70e568) )
ROM_END


ROM_START( kingball )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "prg1.7f",      0x0000, 0x1000, CRC(6cb49046) SHA1(a0891605dff7f9ff51bc7ad85f831a749f2f61e9) )
	ROM_LOAD( "prg2.7j",      0x1000, 0x1000, CRC(c223b416) SHA1(ca2d9f6b8ef6db4f382089161f4147d9828c3554) )
	ROM_LOAD( "prg3.7l",      0x2000, 0x0800, CRC(453634c0) SHA1(0025ccd91e165692092a37541e730010e85e37f2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound code */
	ROM_LOAD( "kbe1.ic4",     0x0000, 0x0800, CRC(5be2c80a) SHA1(f719a80357bed3d66bce40569690f419740148c5) )
	ROM_LOAD( "kbe2.ic5",     0x0800, 0x0800, CRC(bb59e965) SHA1(830e0c415f051e932d76df604025e4e33118a799) )
	ROM_LOAD( "kbe3.ic6",     0x1000, 0x0800, BAD_DUMP CRC(1c94dd31) SHA1(14ab59b8eee741eb1f10ae99ddb99bf7c2dab957) ) // 2nd half missing ("bye bye" voice cut off)
	ROM_LOAD( "kbe2.ic7",     0x1800, 0x0800, CRC(bb59e965) SHA1(830e0c415f051e932d76df604025e4e33118a799) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "chg1.1h",      0x0000, 0x0800, CRC(9cd550e7) SHA1(d2989e6b7a4d7b37a711ef1cfb536fe13e0c5482) )
	ROM_RELOAD(               0x0800, 0x0800 )
	ROM_LOAD( "chg2.1k",      0x1000, 0x0800, CRC(a206757d) SHA1(46b50005876b7f61ab4a118d0a4caaebce8ce3e1) )
	ROM_RELOAD(               0x1800, 0x0800 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "kb2-1",        0x0000, 0x0020, CRC(15dd5b16) SHA1(3d2ca2b42bf508a9e5198e970abcbbedf5729164) )
ROM_END

ROM_START( kingballj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "prg1.7f",      0x0000, 0x1000, CRC(6cb49046) SHA1(a0891605dff7f9ff51bc7ad85f831a749f2f61e9) )
	ROM_LOAD( "prg2.7j",      0x1000, 0x1000, CRC(c223b416) SHA1(ca2d9f6b8ef6db4f382089161f4147d9828c3554) )
	ROM_LOAD( "prg3.7l",      0x2000, 0x0800, CRC(453634c0) SHA1(0025ccd91e165692092a37541e730010e85e37f2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound code */
	ROM_LOAD( "kbj1.ic4",     0x0000, 0x0800, CRC(ba16beb7) SHA1(8c2c91a9e941d858a49edd6c0c8a912e1135653e) )
	ROM_LOAD( "kbj2.ic5",     0x0800, 0x0800, CRC(56686a63) SHA1(8e624df57a63a556941fdbebcd886488799fad17) )
	ROM_LOAD( "kbj3.ic6",     0x1000, 0x0800, CRC(fbc570a5) SHA1(d0dbaf86396bca65e067338a3b5b60b24990b8be) )
	ROM_LOAD( "kbj2.ic7",     0x1800, 0x0800, CRC(56686a63) SHA1(8e624df57a63a556941fdbebcd886488799fad17) )


	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "chg1.1h",      0x0000, 0x0800, CRC(9cd550e7) SHA1(d2989e6b7a4d7b37a711ef1cfb536fe13e0c5482) )
	ROM_RELOAD(               0x0800, 0x0800 )
	ROM_LOAD( "chg2.1k",      0x1000, 0x0800, CRC(a206757d) SHA1(46b50005876b7f61ab4a118d0a4caaebce8ce3e1) )
	ROM_RELOAD(               0x1800, 0x0800 )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "kb2-1",        0x0000, 0x0020, CRC(15dd5b16) SHA1(3d2ca2b42bf508a9e5198e970abcbbedf5729164) )
ROM_END


/*************************************
 *
 *  ROM definitions
 *  Konami games
 *
 *************************************/

ROM_START( frogger )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "frogger.26",   0x0000, 0x1000, CRC(597696d6) SHA1(e7e021776cad00f095a1ebbef407b7c0a8f5d835) )
	ROM_LOAD( "frogger.27",   0x1000, 0x1000, CRC(b6e6fcc3) SHA1(5e8692f2b0c7f4b3642b3ee6670e1c3b20029cdc) )
	ROM_LOAD( "frsm3.7",      0x2000, 0x1000, CRC(aca22ae0) SHA1(5a99060ea2506a3ac7d61ca5876ce5cb3e493565) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "frogger.608",  0x0000, 0x0800, CRC(e8ab0256) SHA1(f090afcfacf5f13cdfa0dfda8e3feb868c6ce8bc) )
	ROM_LOAD( "frogger.609",  0x0800, 0x0800, CRC(7380a48f) SHA1(75582a94b696062cbdb66a4c5cf0bc0bb94f81ee) )
	ROM_LOAD( "frogger.610",  0x1000, 0x0800, CRC(31d7eb27) SHA1(2e1d34ae4da385fd7cac94707d25eeddf4604e1a) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "frogger.607",  0x0000, 0x0800, CRC(05f7d883) SHA1(78831fd287da18928651a8adb7e578d291493eff) )
	ROM_LOAD( "frogger.606",  0x0800, 0x0800, CRC(f524ee30) SHA1(dd768967add61467baa08d5929001f157d6cd911) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "pr-91.6l",     0x0000, 0x0020, CRC(413703bf) SHA1(66648b2b28d3dcbda5bdb2605d1977428939dd3c) )
ROM_END

ROM_START( froggers1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "frogger.26",   0x0000, 0x1000, CRC(597696d6) SHA1(e7e021776cad00f095a1ebbef407b7c0a8f5d835) ) /* We need the correct Sega "EPR" labels for these 3 */
	ROM_LOAD( "frogger.27",   0x1000, 0x1000, CRC(b6e6fcc3) SHA1(5e8692f2b0c7f4b3642b3ee6670e1c3b20029cdc) )
	ROM_LOAD( "frogger.34",   0x2000, 0x1000, CRC(ed866bab) SHA1(24e1bbde44eb5480b7a0570fa0dc1de388cb95ba) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "epr-608.ic32",  0x0000, 0x0800, CRC(e8ab0256) SHA1(f090afcfacf5f13cdfa0dfda8e3feb868c6ce8bc) )
	ROM_LOAD( "epr-609.ic33",  0x0800, 0x0800, CRC(7380a48f) SHA1(75582a94b696062cbdb66a4c5cf0bc0bb94f81ee) )
	ROM_LOAD( "epr-610.ic34",  0x1000, 0x0800, CRC(31d7eb27) SHA1(2e1d34ae4da385fd7cac94707d25eeddf4604e1a) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "epr-607.ic101",  0x0000, 0x0800, CRC(05f7d883) SHA1(78831fd287da18928651a8adb7e578d291493eff) )
	ROM_LOAD( "epr-606.ic102",  0x0800, 0x0800, CRC(f524ee30) SHA1(dd768967add61467baa08d5929001f157d6cd911) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "pr-91.6l",     0x0000, 0x0020, CRC(413703bf) SHA1(66648b2b28d3dcbda5bdb2605d1977428939dd3c) )
ROM_END

ROM_START( froggers2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-1012.ic5",  0x0000, 0x1000, CRC(efab0c79) SHA1(68c99b6cdcb9396bb473739a62ffc009b4bf57d5) )
	ROM_LOAD( "epr-1013a.ic6", 0x1000, 0x1000, CRC(aeca9c13) SHA1(cdf560adbd7f2813e86e378da7781cccf7928a44) )
	ROM_LOAD( "epr-1014.ic7",  0x2000, 0x1000, CRC(dd251066) SHA1(4612e1fe1ab7182a277140b1a1976cc17e0746a5) )
	ROM_LOAD( "epr-1015.ic8",  0x3000, 0x1000, CRC(bf293a02) SHA1(be94e9f5caa74c3de6fd95bd20928f4a9c514227) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "epr-608.ic32",  0x0000, 0x0800, CRC(e8ab0256) SHA1(f090afcfacf5f13cdfa0dfda8e3feb868c6ce8bc) )
	ROM_LOAD( "epr-609.ic33",  0x0800, 0x0800, CRC(7380a48f) SHA1(75582a94b696062cbdb66a4c5cf0bc0bb94f81ee) )
	ROM_LOAD( "epr-610.ic34",  0x1000, 0x0800, CRC(31d7eb27) SHA1(2e1d34ae4da385fd7cac94707d25eeddf4604e1a) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "epr-607.ic101",  0x0000, 0x0800, CRC(05f7d883) SHA1(78831fd287da18928651a8adb7e578d291493eff) )
	ROM_LOAD( "epr-606.ic102",  0x0800, 0x0800, CRC(f524ee30) SHA1(dd768967add61467baa08d5929001f157d6cd911) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "pr-91.6l",     0x0000, 0x0020, CRC(413703bf) SHA1(66648b2b28d3dcbda5bdb2605d1977428939dd3c) )
ROM_END

ROM_START( froggermc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-1031.15",  0x0000, 0x1000, CRC(4b7c8d11) SHA1(9200b33cac0ef5a6647c95ebd25237fa62fcdf30) )
	ROM_LOAD( "epr-1032.16",  0x1000, 0x1000, CRC(ac00b9d9) SHA1(6414d2aa2c0ccb8cb567ffde3acdb693cfd28dbb) )
	ROM_LOAD( "epr-1033.33",  0x2000, 0x1000, CRC(bc1d6fbc) SHA1(c9c040418f0bf7b7fce599592f806e7aaf448c3d) )
	ROM_LOAD( "epr-1034.34",  0x3000, 0x1000, CRC(9efe7399) SHA1(77355160169db256f45286e60ebf6a406527d346) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "epr-1082.42",  0x0000, 0x1000, CRC(802843c2) SHA1(059b26ddf1cdc8076d160b872f9d50b97af7f316) )
	ROM_LOAD( "epr-1035.43",  0x1000, 0x0800, CRC(14e74148) SHA1(0023394e971f191c41ff20b47835f1dafb924d15) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "epr-1037.1h",  0x0000, 0x0800, CRC(05f7d883) SHA1(78831fd287da18928651a8adb7e578d291493eff) )
	ROM_LOAD( "epr-1036.1k",  0x0800, 0x0800, CRC(658745f8) SHA1(e4e5c3e011c8a7233a36d29e10e08905873500aa) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "pr-91.6l",     0x0000, 0x0020, CRC(413703bf) SHA1(66648b2b28d3dcbda5bdb2605d1977428939dd3c) )
ROM_END

ROM_START( froggers )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vid_d2.bin",   0x0000, 0x0800, CRC(c103066e) SHA1(8c2d4c825e9c4180fe70b0db18a547dc3ddc3c2c) )
	ROM_LOAD( "vid_e2.bin",   0x0800, 0x0800, CRC(f08bc094) SHA1(23ad1e57f244d6b63fd9640249dcb1eeafb8206e) )
	ROM_LOAD( "vid_f2.bin",   0x1000, 0x0800, CRC(637a2ff8) SHA1(e9b9fc692ca5d8deb9cd30d9d73ad25c8d8bafe1) )
	ROM_LOAD( "vid_h2.bin",   0x1800, 0x0800, CRC(04c027a5) SHA1(193550731513c02cad464661a1ceb230819ca70f) )
	ROM_LOAD( "vid_j2.bin",   0x2000, 0x0800, CRC(fbdfbe74) SHA1(48d5d1247d09eaea2a9a29f4ed6543d0411597aa) )
	ROM_LOAD( "vid_l2.bin",   0x2800, 0x0800, CRC(8a4389e1) SHA1(b2c74afb93927dac0d8bb24e02e0b2a069f2d3c8) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "frogger.608",  0x0000, 0x0800, CRC(e8ab0256) SHA1(f090afcfacf5f13cdfa0dfda8e3feb868c6ce8bc) )
	ROM_LOAD( "frogger.609",  0x0800, 0x0800, CRC(7380a48f) SHA1(75582a94b696062cbdb66a4c5cf0bc0bb94f81ee) )
	ROM_LOAD( "frogger.610",  0x1000, 0x0800, CRC(31d7eb27) SHA1(2e1d34ae4da385fd7cac94707d25eeddf4604e1a) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "frogger.607",  0x0000, 0x0800, CRC(05f7d883) SHA1(78831fd287da18928651a8adb7e578d291493eff) )
	ROM_LOAD( "epr-1036.1k",  0x0800, 0x0800, CRC(658745f8) SHA1(e4e5c3e011c8a7233a36d29e10e08905873500aa) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "pr-91.6l",     0x0000, 0x0020, CRC(413703bf) SHA1(66648b2b28d3dcbda5bdb2605d1977428939dd3c) )
ROM_END

ROM_START( frogf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6.bin",        0x0000, 0x1000, CRC(8ff0a973) SHA1(adb1c28617d915fbcfa9190bd8589a56a8858e25) )
	ROM_LOAD( "7.bin",        0x1000, 0x1000, CRC(3087bb4b) SHA1(3fe1f68a2ad12b1cadba89d99afe574cf5342d81) )
	ROM_LOAD( "8.bin",        0x2000, 0x1000, CRC(c3869d12) SHA1(7bd95c12fc1fe1a3cfc0140b64cf76fa57aa3fb4) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "frogger.608",  0x0000, 0x0800, CRC(e8ab0256) SHA1(f090afcfacf5f13cdfa0dfda8e3feb868c6ce8bc) )
	ROM_LOAD( "frogger.609",  0x0800, 0x0800, CRC(7380a48f) SHA1(75582a94b696062cbdb66a4c5cf0bc0bb94f81ee) )
	ROM_LOAD( "frogger.610",  0x1000, 0x0800, CRC(31d7eb27) SHA1(2e1d34ae4da385fd7cac94707d25eeddf4604e1a) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "frogger.607",  0x0000, 0x0800, CRC(05f7d883) SHA1(78831fd287da18928651a8adb7e578d291493eff) )
	ROM_LOAD( "epr-1036.1k",  0x0800, 0x0800, CRC(658745f8) SHA1(e4e5c3e011c8a7233a36d29e10e08905873500aa) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "pr-91.6l",     0x0000, 0x0020, CRC(413703bf) SHA1(66648b2b28d3dcbda5bdb2605d1977428939dd3c) )
ROM_END

ROM_START( frogg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p1.bin",       0x0000, 0x0800, CRC(1762b266) SHA1(2cf34dcfe00dc476b327f9d762a8d2aa268a2d25) )
	ROM_LOAD( "p2.bin",       0x0800, 0x0800, CRC(322f3916) SHA1(9236aaa260c4db4adbd92c8bba3674d07d7235a8) )
	ROM_LOAD( "p3.bin",       0x1000, 0x0800, CRC(28bd6151) SHA1(1a5bc540168fa5fef01bd7bc2cdbdb910c9a4ba4) )
	ROM_LOAD( "p4.bin",       0x1800, 0x0800, CRC(5a69ab18) SHA1(40b7bf200f87e0fb3fb54726ba79387889446052) )
	ROM_LOAD( "p5.bin",       0x2000, 0x0800, CRC(b4f17745) SHA1(2f237a667f6c95af213b787620142c1530d3cdd8) )
	ROM_LOAD( "p6.bin",       0x2800, 0x0800, CRC(34be71b5) SHA1(3088fc5817a397d0a87610d62845c7b8c4440f57) )
	ROM_LOAD( "p7.bin",       0x3000, 0x0800, CRC(de3edc8c) SHA1(634d54fb19b422b56576a196bdaf95733c52c7ee) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "k.bin",        0x0000, 0x0800, CRC(05f7d883) SHA1(78831fd287da18928651a8adb7e578d291493eff) )
	ROM_LOAD( "h.bin",        0x0800, 0x0800, CRC(658745f8) SHA1(e4e5c3e011c8a7233a36d29e10e08905873500aa) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "pr-91.6l",     0x0000, 0x0020, CRC(413703bf) SHA1(66648b2b28d3dcbda5bdb2605d1977428939dd3c) )
ROM_END

ROM_START( froggrs )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code */
	ROM_LOAD( "frog4.bin",    0x0000, 0x1000, CRC(4d563992) SHA1(25a197f711498112e202fa88ca576b247d24e228) )
	ROM_LOAD( "frog5.bin",    0x1000, 0x1000, CRC(d8b8c06e) SHA1(51363deab935c3625a825499cb1f1c7a0c773b03) )
	ROM_LOAD( "frog6.bin",    0x2000, 0x1000, CRC(b55a1cb5) SHA1(4e751e561c179641bb7db1abf3a5272d81d434b0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the audio CPU */
	ROM_LOAD( "frogger.608",  0x0000, 0x0800, CRC(e8ab0256) SHA1(f090afcfacf5f13cdfa0dfda8e3feb868c6ce8bc) )
	ROM_LOAD( "frogger.609",  0x0800, 0x0800, CRC(7380a48f) SHA1(75582a94b696062cbdb66a4c5cf0bc0bb94f81ee) )
	ROM_LOAD( "frog3.bin",    0x1000, 0x0800, CRC(837c16ab) SHA1(740780149563708163867c6412d3b2500192d7b1) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "frogger.607",  0x0000, 0x0800, CRC(05f7d883) SHA1(78831fd287da18928651a8adb7e578d291493eff) )
	ROM_LOAD( "frogger.606",  0x0800, 0x0800, CRC(f524ee30) SHA1(dd768967add61467baa08d5929001f157d6cd911) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "pr-91.6l",     0x0000, 0x0020, CRC(413703bf) SHA1(66648b2b28d3dcbda5bdb2605d1977428939dd3c) )
ROM_END

ROM_START( quaak )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1f.bin",   0x0000, 0x0800, CRC(5d0e2716) SHA1(c484ae162bfe5ef9d5d2a6930c9c476394e70bbd) )
	ROM_LOAD( "2f.bin",   0x0800, 0x0800, CRC(cfbf8219) SHA1(400ba52d9a2977344842fdb8c40d8629cb16110b) )
	ROM_LOAD( "3f.bin",   0x1000, 0x0800, CRC(cbb17731) SHA1(7103c4ce0a103b4916cf88c69a24f0f5cc1e2628) )
	ROM_LOAD( "4f.bin",   0x1800, 0x0800, CRC(817ff82d) SHA1(0111af02ab8fa4f52877e5539a954b37550ceb40) )
	ROM_LOAD( "5f.bin",   0x2000, 0x0800, CRC(5a8dd54b) SHA1(2e7769fc1ccc540f1f2552d7d427c6fdb1174488) )
	ROM_LOAD( "6f.bin",   0x2800, 0x0800, CRC(e1d46369) SHA1(0da5f1918cd711e5e593c8b04103371936665c2f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a.bin",  0x0000, 0x0800, CRC(b4c2180e) SHA1(25894481ef3b55b11a875ab08c665d5d541f1a06) )
	ROM_LOAD( "b.bin",  0x0800, 0x0800, CRC(a1aae0bc) SHA1(1cb06b0cfde9fdd7f176f4a51de801d97785d279) )
	ROM_LOAD( "c.bin",  0x1000, 0x0800, CRC(9d88fd0a) SHA1(ecfb8ddf67cd7755cbdbc1cc5e7788e1b5b3c882) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "7h.bin",  0x0000, 0x0800, CRC(28350f17) SHA1(c1999d1dadc243ed742610db39a278acd8422a73) )
	ROM_LOAD( "8h.bin",  0x0800, 0x0800, CRC(e080f942) SHA1(45371ba3399101bd4fcd4819c8618d8cf2078723) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "n82s123.bin",     0x0000, 0x0020, CRC(a35ec965) SHA1(ea5851f3e0e54f043347c7ae9869db8f6711d031) )
ROM_END

ROM_START( froggeram )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.d2",   0x0000, 0x0800, CRC(b680e622) SHA1(233dbefa2aae6e85cb61acd60c49480bd4a3388d) )
	ROM_LOAD( "2.e2",   0x0800, 0x0800, CRC(32c56a50) SHA1(4d215fff6ff002e23aa889292c9c5eb242975f5d) )
	ROM_LOAD( "3.f2",   0x1000, 0x0800, CRC(4223a053) SHA1(c19555d2fee4172dff99d7cf65ebb44d1336c06e) )
	ROM_LOAD( "4.h2",   0x1800, 0x0800, CRC(bcd02aa7) SHA1(987c35bf9af8bb1083ccbf4d9f912be8d74b3d1f) )
	ROM_LOAD( "5.j2",   0x2000, 0x0800, CRC(b11b36f7) SHA1(d4e9342be7fa23f30565d7b75fa0fb8c6c82669d) )
	ROM_LOAD( "6.l2",   0x2800, 0x0800, CRC(a239048a) SHA1(a8dcc0b4bdb51f6e391832d69ba3a8727be59ae7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "top7.c5",  0x0000, 0x0800, CRC(b4c2180e) SHA1(25894481ef3b55b11a875ab08c665d5d541f1a06) ) // only this sound rom was present in the dump, it matches quaak above
	ROM_LOAD( "b.bin",  0x0800, 0x0800, CRC(a1aae0bc) SHA1(1cb06b0cfde9fdd7f176f4a51de801d97785d279) ) // so let's assume the rest do too.
	ROM_LOAD( "c.bin",  0x1000, 0x0800, CRC(9d88fd0a) SHA1(ecfb8ddf67cd7755cbdbc1cc5e7788e1b5b3c882) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "bl7h",  0x0000, 0x0800, CRC(05f7d883) SHA1(78831fd287da18928651a8adb7e578d291493eff) )
	ROM_LOAD( "bl8h",  0x0800, 0x0800, CRC(658745f8) SHA1(e4e5c3e011c8a7233a36d29e10e08905873500aa) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "n82s123.bin",     0x0000, 0x0020, CRC(a35ec965) SHA1(ea5851f3e0e54f043347c7ae9869db8f6711d031) )
ROM_END


ROM_START( turtles )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "turt_vid.2c",  0x0000, 0x1000, CRC(ec5e61fb) SHA1(3ca89800fda7a7e61f54d71d5302908be2706def) )
	ROM_LOAD( "turt_vid.2e",  0x1000, 0x1000, CRC(fd10821e) SHA1(af74602bf2454eb8f3b9bb5c425e2476feeecd69) )
	ROM_LOAD( "turt_vid.2f",  0x2000, 0x1000, CRC(ddcfc5fa) SHA1(2af9383e5a289c2d7fbe6cf5e5b1519c352afbab) )
	ROM_LOAD( "turt_vid.2h",  0x3000, 0x1000, CRC(9e71696c) SHA1(3dcdf5dc601c875fc9d8b9a46e3ef588e7478e0d) )
	ROM_LOAD( "turt_vid.2j",  0x4000, 0x1000, CRC(fcd49fef) SHA1(bb1e91b2e6d4b5a861bf37907ef6b198328d8d83) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "turt_snd.5c",  0x0000, 0x1000, CRC(f0c30f9a) SHA1(5621f336e9be8acf986a34bbb8855ed5d45c28ef) )
	ROM_LOAD( "turt_snd.5d",  0x1000, 0x1000, CRC(af5fc43c) SHA1(8a49c55feba094b07380615cf0b6f0878c25a260) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "turt_vid.5h",  0x0000, 0x0800, CRC(e5999d52) SHA1(bc3f52cf6c6e19dfd2dacd1e8c9128f437e995fc) )
	ROM_LOAD( "turt_vid.5f",  0x0800, 0x0800, CRC(c3ffd655) SHA1(dee51d77be262a2944488e381541c10a2b6e5d83) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "turtles.clr",  0x0000, 0x0020, CRC(f3ef02dd) SHA1(09fd795170d7d30f101d579f57553da5ff3800ab) )
ROM_END

ROM_START( turpin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "m1",           0x0000, 0x1000, CRC(89177473) SHA1(0717b1e7308ffe527edfc578ec4353809e7d9eea) )
	ROM_LOAD( "m2",           0x1000, 0x1000, CRC(4c6ca5c6) SHA1(dd4ca7adaa523a8e775cdfaa99bb3cc25da32c08) )
	ROM_LOAD( "m3",           0x2000, 0x1000, CRC(62291652) SHA1(82965d3e9608afde4ff06cba1d7a4b11cd904c11) )
	ROM_LOAD( "turt_vid.2h",  0x3000, 0x1000, CRC(9e71696c) SHA1(3dcdf5dc601c875fc9d8b9a46e3ef588e7478e0d) )
	ROM_LOAD( "m5",           0x4000, 0x1000, CRC(7d2600f2) SHA1(1a9bdf63b50419c6e0d9c401c3dcf29d5b459fa6) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "turt_snd.5c",  0x0000, 0x1000, CRC(f0c30f9a) SHA1(5621f336e9be8acf986a34bbb8855ed5d45c28ef) )
	ROM_LOAD( "turt_snd.5d",  0x1000, 0x1000, CRC(af5fc43c) SHA1(8a49c55feba094b07380615cf0b6f0878c25a260) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "turt_vid.5h",  0x0000, 0x0800, CRC(e5999d52) SHA1(bc3f52cf6c6e19dfd2dacd1e8c9128f437e995fc) )
	ROM_LOAD( "turt_vid.5f",  0x0800, 0x0800, CRC(c3ffd655) SHA1(dee51d77be262a2944488e381541c10a2b6e5d83) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "turtles.clr",  0x0000, 0x0020, CRC(f3ef02dd) SHA1(09fd795170d7d30f101d579f57553da5ff3800ab) )
ROM_END

ROM_START( 600 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "600_vid.2c",   0x0000, 0x1000, CRC(8ee090ae) SHA1(3d491313da6cccd6dbc15774569be0555fe2f73a) )
	ROM_LOAD( "600_vid.2e",   0x1000, 0x1000, CRC(45bfaff2) SHA1(ba4f7aa499f4993ec2191b8832b5604fd41964bc) )
	ROM_LOAD( "600_vid.2f",   0x2000, 0x1000, CRC(9f4c8ed7) SHA1(2564dae82019097227351a7ddc9c5156ca00297a) )
	ROM_LOAD( "600_vid.2h",   0x3000, 0x1000, CRC(a92ef056) SHA1(c319d41a3345b84670fe9110f78332c1cfe1e163) )
	ROM_LOAD( "600_vid.2j",   0x4000, 0x1000, CRC(6dadd72d) SHA1(5602b5ebb2c287f72a5ce873b4e3dfd19b8412a0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "600_snd.5c",   0x0000, 0x1000, CRC(1773c68e) SHA1(cc4aa3a98e85bc6300f8c1ee1a0448071d7c6dfa) )
	ROM_LOAD( "600_snd.5d",   0x1000, 0x1000, CRC(a311b998) SHA1(39af321b8c3f211ed6d083a2aba4fbc8af11c9e8) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "600_vid.5h",   0x0000, 0x0800, CRC(006c3d56) SHA1(0c773e0e84d0bf45be5a5a7cfff960c1ca2f0320) )
	ROM_LOAD( "600_vid.5f",   0x0800, 0x0800, CRC(7dbc0426) SHA1(29eeb3cdb5a3bcf7115d8099e4d04cf76216b003) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "turtles.clr",  0x0000, 0x0020, CRC(f3ef02dd) SHA1(09fd795170d7d30f101d579f57553da5ff3800ab) )
ROM_END

/* Amidar (c) Konami 1982. Original Konami pcb.
   Soundboard silkscreened: Konami KT-4108-1B
   CPU Board Silkscreened: Konami KT4108-2 */

ROM_START( amidar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.2c",  0x0000, 0x1000, CRC(621b74de) SHA1(f064eccfb7da18119ed3088a5f939085eb446c90) )
	ROM_LOAD( "2.2e",  0x1000, 0x1000, CRC(38538b98) SHA1(12b2a0c09926d006781bee5d450bc0c391cc1fb5) )
	ROM_LOAD( "3.2f",  0x2000, 0x1000, CRC(099ecb24) SHA1(e83f049b25aba481e09606db3158726145ebd656) )
	ROM_LOAD( "4.2h",  0x3000, 0x1000, CRC(ba149a93) SHA1(9ef1d27f0780612be0ea2be94c3a2c781a4924c8) )
	ROM_LOAD( "5.2j",  0x4000, 0x1000, CRC(eecc1abf) SHA1(1530b374d15e0d05c8eb988cc1cbab48b0be211c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "s1.5c",  0x0000, 0x1000, CRC(8ca7b750) SHA1(4f4c2915503b85abe141d717fd254ee10c9da99e) )
	ROM_LOAD( "s2.5d",  0x1000, 0x1000, CRC(9b5bdc0a) SHA1(84d953618c8bf510d23b42232a856ac55f1baff5) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "c2.5f",  0x0000, 0x0800, CRC(2cfe5ede) SHA1(0d86a78008ac8653c17fff5be5ebdf1f0a9d31eb) )
	ROM_LOAD( "c2.5d",  0x0800, 0x0800, CRC(57c4fd0d) SHA1(8764deec9fbff4220d61df621b12fc36c3702601) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "amidar.clr",   0x0000, 0x0020, CRC(f940dcc3) SHA1(1015e56f37c244a850a8f4bf0e36668f047fd46d) )
ROM_END

ROM_START( amidar1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "amidar.2c",    0x0000, 0x1000, CRC(c294bf27) SHA1(399325bf1559e8cdbddf7cfbf0dc739f9ed72ef0) )
	ROM_LOAD( "amidar.2e",    0x1000, 0x1000, CRC(e6e96826) SHA1(e9c4f8c594640424b456505e676352a98b758c03) )
	ROM_LOAD( "amidar.2f",    0x2000, 0x1000, CRC(3656be6f) SHA1(9d652f66bedcf17a6453c0e0ead30bfd7ea0bd0a) )
	ROM_LOAD( "amidar.2h",    0x3000, 0x1000, CRC(1be170bd) SHA1(c047bc393b297c0d47668a5f6f4870e3fac937ef) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "amidar.5c",    0x0000, 0x1000, CRC(c4b66ae4) SHA1(9d09dbde4019f7be3abe0815b0e06d542c01c255) )
	ROM_LOAD( "amidar.5d",    0x1000, 0x1000, CRC(806785af) SHA1(c8c85e3a6a204feccd7859b4527bd649e96134b4) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "amidar.5f",    0x0000, 0x0800, CRC(5e51e84d) SHA1(dfe84db7e2b1a45a1d484fcf37291f536bc5324c) )
	ROM_LOAD( "amidar.5h",    0x0800, 0x0800, CRC(2f7f1c30) SHA1(83c330eca20dfcc6a4099001943b9ed7a7c3db5b) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "amidar.clr",   0x0000, 0x0020, CRC(f940dcc3) SHA1(1015e56f37c244a850a8f4bf0e36668f047fd46d) )
ROM_END

ROM_START( amidaru )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "amidarus.2c",  0x0000, 0x1000, CRC(951e0792) SHA1(3a68b829c9ffb465bd6582c9ea566e0e947c6c19) )
	ROM_LOAD( "amidarus.2e",  0x1000, 0x1000, CRC(a1a3a136) SHA1(330ec857fdf4c1b28e2560a5f63a2432f87f9b2f) )
	ROM_LOAD( "amidarus.2f",  0x2000, 0x1000, CRC(a5121bf5) SHA1(fe15b91724758ede43dd332327919f164772c592) )
	ROM_LOAD( "amidarus.2h",  0x3000, 0x1000, CRC(051d1c7f) SHA1(3cfa0f728a5c27da0a3fe2579ad226129ccde232) )
	ROM_LOAD( "amidarus.2j",  0x4000, 0x1000, CRC(351f00d5) SHA1(6659357f40f888b21be00826246200fd3a8a88ce) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "amidarus.5c",  0x0000, 0x1000, CRC(8ca7b750) SHA1(4f4c2915503b85abe141d717fd254ee10c9da99e) )
	ROM_LOAD( "amidarus.5d",  0x1000, 0x1000, CRC(9b5bdc0a) SHA1(84d953618c8bf510d23b42232a856ac55f1baff5) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "amidarus.5f",  0x0000, 0x0800, CRC(2cfe5ede) SHA1(0d86a78008ac8653c17fff5be5ebdf1f0a9d31eb) )
	ROM_LOAD( "amidarus.5h",  0x0800, 0x0800, CRC(57c4fd0d) SHA1(8764deec9fbff4220d61df621b12fc36c3702601) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "amidar.clr",   0x0000, 0x0020, CRC(f940dcc3) SHA1(1015e56f37c244a850a8f4bf0e36668f047fd46d) )
ROM_END

ROM_START( amidaro )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "107.2cd",      0x0000, 0x1000, CRC(c52536be) SHA1(3f64578214d2d9f0e4e7ee87e09b0aac33a73098) )
	ROM_LOAD( "108.2fg",      0x1000, 0x1000, CRC(38538b98) SHA1(12b2a0c09926d006781bee5d450bc0c391cc1fb5) )
	ROM_LOAD( "109.2fg",      0x2000, 0x1000, CRC(69907f0f) SHA1(f1d19a76ffc41ee8c5c574f10108cfdfe525b732) )
	ROM_LOAD( "110.2h",       0x3000, 0x1000, CRC(ba149a93) SHA1(9ef1d27f0780612be0ea2be94c3a2c781a4924c8) )
	ROM_LOAD( "111.2j",       0x4000, 0x1000, CRC(20d01c2e) SHA1(e09437ff440f04036d5ec74b355e97bbbbfefb95) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "amidarus.5c",  0x0000, 0x1000, CRC(8ca7b750) SHA1(4f4c2915503b85abe141d717fd254ee10c9da99e) )
	ROM_LOAD( "amidarus.5d",  0x1000, 0x1000, CRC(9b5bdc0a) SHA1(84d953618c8bf510d23b42232a856ac55f1baff5) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "amidarus.5f",  0x0000, 0x0800, CRC(2cfe5ede) SHA1(0d86a78008ac8653c17fff5be5ebdf1f0a9d31eb) )
	ROM_LOAD( "113.5h",       0x0800, 0x0800, CRC(bcdce168) SHA1(e593d03c460ef4607e3ba25019d9f01d4a717dd9) )  /* The letter 'S' is slightly different */

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "amidar.clr",   0x0000, 0x0020, CRC(f940dcc3) SHA1(1015e56f37c244a850a8f4bf0e36668f047fd46d) )
ROM_END

ROM_START( amidarb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ami2gor.2c", 0x0000, 0x1000, CRC(9ad2dcd2) SHA1(43ceb93d891c1ebf55e7c26de13e3db8e1d26f6d) )
	ROM_LOAD( "2.2f",       0x1000, 0x1000, CRC(66282ff5) SHA1(986778278eb339768d190460680e7aa698812488) )
	ROM_LOAD( "3.2j",       0x2000, 0x1000, CRC(b0860e31) SHA1(8fb92b0e71c826a509a8f712553de0f4a636286f) )
	ROM_LOAD( "4.2m",       0x3000, 0x1000, CRC(4a4086c9) SHA1(6f309b67dc68e06e6eb1d3ee2ae75afe253a4ce3) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "8.11d",      0x0000, 0x1000, CRC(8ca7b750) SHA1(4f4c2915503b85abe141d717fd254ee10c9da99e) )
	ROM_LOAD( "9.9d",       0x1000, 0x1000, CRC(9b5bdc0a) SHA1(84d953618c8bf510d23b42232a856ac55f1baff5) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5.5f",      0x0000, 0x0800, CRC(2082ad0a) SHA1(c6014d9575e92adf09b0961c2158a779ebe940c4) )
	ROM_LOAD( "6.5h",      0x0800, 0x0800, CRC(3029f94f) SHA1(3b432b42e79f8b0a7d65e197f373a04e3c92ff20) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "n82s123n.6e",   0x0000, 0x0020, CRC(01004d3f) SHA1(e53cbc54ea96e846481a67bbcccf6b1726e70f9c) )
ROM_END

ROM_START( amigo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2732.a1",      0x0000, 0x1000, CRC(930dc856) SHA1(7022f1f26830baccdc8b8f0b10fb1d1ccb080f22) )
	ROM_LOAD( "2732.a2",      0x1000, 0x1000, CRC(66282ff5) SHA1(986778278eb339768d190460680e7aa698812488) )
	ROM_LOAD( "2732.a3",      0x2000, 0x1000, CRC(e9d3dc76) SHA1(627c6068c65985175388aec43ac2a4248b004c97) )
	ROM_LOAD( "2732.a4",      0x3000, 0x1000, CRC(4a4086c9) SHA1(6f309b67dc68e06e6eb1d3ee2ae75afe253a4ce3) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "amidarus.5c",  0x0000, 0x1000, CRC(8ca7b750) SHA1(4f4c2915503b85abe141d717fd254ee10c9da99e) )
	ROM_LOAD( "amidarus.5d",  0x1000, 0x1000, CRC(9b5bdc0a) SHA1(84d953618c8bf510d23b42232a856ac55f1baff5) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "2716.a6",      0x0000, 0x0800, CRC(2082ad0a) SHA1(c6014d9575e92adf09b0961c2158a779ebe940c4) )
	ROM_LOAD( "2716.a5",      0x0800, 0x0800, CRC(3029f94f) SHA1(3b432b42e79f8b0a7d65e197f373a04e3c92ff20) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "amidar.clr",   0x0000, 0x0020, CRC(f940dcc3) SHA1(1015e56f37c244a850a8f4bf0e36668f047fd46d) )
ROM_END

ROM_START( amidars )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "am2d",         0x0000, 0x0800, CRC(24b79547) SHA1(eca735c6a35561a9a6ba8a20dca1e1c78ed073fc) )
	ROM_LOAD( "am2e",         0x0800, 0x0800, CRC(4c64161e) SHA1(5b2e49ff915295617671b13f15b566046a5dbc15) )
	ROM_LOAD( "am2f",         0x1000, 0x0800, CRC(b3987a72) SHA1(1d72e9ae3005029628c6f9beb6ca65afcb1f7893) )
	ROM_LOAD( "am2h",         0x1800, 0x0800, CRC(29873461) SHA1(7d0ee9a82f02163b4cc6a7097e88ae34e96ebf58) )
	ROM_LOAD( "am2j",         0x2000, 0x0800, CRC(0fdd54d8) SHA1(c32fdc8e292d91159e6c80c7033abea6404a4f2c) )
	ROM_LOAD( "am2l",         0x2800, 0x0800, CRC(5382f7ed) SHA1(425ec2c2caf404fc8ab13ee38d6567413022e1a1) )
	ROM_LOAD( "am2m",         0x3000, 0x0800, CRC(1d7109e9) SHA1(e0d24475547bbe5a94b45be6abefb84ad84d2534) )
	ROM_LOAD( "am2p",         0x3800, 0x0800, CRC(c9163ac6) SHA1(46d757180426b71c827d14a35824a248f2c787b6) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "amidarus.5c",  0x0000, 0x1000, CRC(8ca7b750) SHA1(4f4c2915503b85abe141d717fd254ee10c9da99e) )
	ROM_LOAD( "amidarus.5d",  0x1000, 0x1000, CRC(9b5bdc0a) SHA1(84d953618c8bf510d23b42232a856ac55f1baff5) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "2716.a6",      0x0000, 0x0800, CRC(2082ad0a) SHA1(c6014d9575e92adf09b0961c2158a779ebe940c4) )   /* Same graphics ROMs as Amigo */
	ROM_LOAD( "2716.a5",      0x0800, 0x0800, CRC(3029f94f) SHA1(3b432b42e79f8b0a7d65e197f373a04e3c92ff20) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "amidar.clr",   0x0000, 0x0020, CRC(f940dcc3) SHA1(1015e56f37c244a850a8f4bf0e36668f047fd46d) )
ROM_END

ROM_START( theend )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic13_1t.bin",  0x0000, 0x0800, CRC(93e555ba) SHA1(f684927cecabfbd7544f7549a6152c0a6a436019) )
	ROM_LOAD( "ic14_2t.bin",  0x0800, 0x0800, CRC(2de7ad27) SHA1(caf369fde632652a0a5fb11d3605f0d2386d297a) )
	ROM_LOAD( "ic15_3t.bin",  0x1000, 0x0800, CRC(035f750b) SHA1(5f70518e5dbfca0ba12ba4dc4f357ce8e6b27bc8) )
	ROM_LOAD( "ic16_4t.bin",  0x1800, 0x0800, CRC(61286b5c) SHA1(14464aa5284aecc9c6046e464ab3d13da89d8dda) )
	ROM_LOAD( "ic17_5t.bin",  0x2000, 0x0800, CRC(434a8f68) SHA1(3c8c099c7865997d475c096f1b1c93d88ab21543) )
	ROM_LOAD( "ic18_6t.bin",  0x2800, 0x0800, CRC(dc4cc786) SHA1(3311361a1eb29715aa41d61fbb3563014bd9eeb1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ic56_1.bin",   0x0000, 0x0800, CRC(7a141f29) SHA1(ca483943971c8fc7f5775a8a7cc6ddd331d48170) )
	ROM_LOAD( "ic55_2.bin",   0x0800, 0x0800, CRC(218497c1) SHA1(3e080621f2e83909a6f304a2d960a080bccbbdc2) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "ic30_2c.bin",  0x0000, 0x0800, CRC(68ccf7bf) SHA1(a8ea784a2660f855757ae0b30cb2a33ab6f2cd59) )
	ROM_LOAD( "ic31_1c.bin",  0x0800, 0x0800, CRC(4a48c999) SHA1(f1abcbfc3146a18dc3ff865e3ba278377a42a875) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6331-1j.86",   0x0000, 0x0020, CRC(24652bc4) SHA1(d89575f3749c75dc963317fe451ffeffd9856e4d) )
ROM_END

/*
All labels are in the form of:

THE END (c)
RA3 13
1980 STERN

The above example is for IC13
*/
ROM_START( theends ) /* The Stern Electronics license */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "the_end_ra3_13.ic13",         0x0000, 0x0800, CRC(90e5ab14) SHA1(b926801ab1cc1e2787a76ced6c7cffd6fce753d4) )
	ROM_LOAD( "the_end_ra3_14.ic14",         0x0800, 0x0800, CRC(950f0a07) SHA1(bde9f3c6cf060dc6f5b7652287b94e04bed7bcf7) )
	ROM_LOAD( "the_end_ra3_15.ic15",         0x1000, 0x0800, CRC(6786bcf5) SHA1(7556d3dc51d6a112b6357b8a36df05fd1a4d1cc9) )
	ROM_LOAD( "the_end_ra3_16.ic16",         0x1800, 0x0800, CRC(380a0017) SHA1(3354eb328a32537f722fe8a0949ddcab6cf21eb8) )
	ROM_LOAD( "the_end_ra3_17.ic17",         0x2000, 0x0800, CRC(af067b7f) SHA1(855c6ddf29fbfea004c7143fe29064abf53801ad) )
	ROM_LOAD( "the_end_ra3_18.ic18",         0x2800, 0x0800, CRC(a0411b93) SHA1(d644968758a1b73d13e09b24d24bfec82276e8f4) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "the_end_ra3_56.ic56",         0x0000, 0x0800, CRC(3b2c2f70) SHA1(bcccdacacfc9a3b5f1412dfba6bb0046d283bccc) )
	ROM_LOAD( "the_end_ra2_55.ic55",         0x0800, 0x0800, CRC(e0429e50) SHA1(27678fc3172cbca3ae1eae96e9d8a62561d5ce40) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "the_end_ra3_30.ic30",         0x0000, 0x0800, CRC(527fd384) SHA1(92a384899d5acd2c689f637da16a0e2d11a9d9c6) )
	ROM_LOAD( "the_end_ra3_31.ic31",         0x0800, 0x0800, CRC(af6d09b6) SHA1(f3ad51dc88aa58fd39195ead978b039e0b0b585c) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6331-1j.86",   0x0000, 0x0020, CRC(24652bc4) SHA1(d89575f3749c75dc963317fe451ffeffd9856e4d) ) /* no label for this chip */
ROM_END

ROM_START( theendb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "0p.t.o.10l",   0x0000, 0x1000, CRC(46712d43) SHA1(e1b84494b530dd96d8a51a3f8bd7d7d3ba7560a9) )
	ROM_LOAD( "1p.t.o.9l",    0x1000, 0x1000, CRC(10256742) SHA1(3748bf82e410ba763cc10a546b566f1d9efb8307) )
	ROM_LOAD( "2p.t.o.8l",    0x2000, 0x1000, CRC(5ee6660a) SHA1(092d5da074c15743e619e3d9e0b5f5bd16ea7159) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "0s.t.o.3b",    0x0000, 0x1000, CRC(cdc9da78) SHA1(b52d5b67b8e0dfb76216d7b3e6e51a027ef4b20e) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "ic30",         0x0000, 0x0800, CRC(527fd384) SHA1(92a384899d5acd2c689f637da16a0e2d11a9d9c6) ) // 0gc.t.o.5h
	ROM_LOAD( "ic31",         0x0800, 0x0800, CRC(af6d09b6) SHA1(f3ad51dc88aa58fd39195ead978b039e0b0b585c) ) // 1gc.t.o.3h

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6331-1j.86",   0x0000, 0x0020, CRC(24652bc4) SHA1(d89575f3749c75dc963317fe451ffeffd9856e4d) )
ROM_END


ROM_START( scramble )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "s1.2d",        0x0000, 0x0800, CRC(ea35ccaa) SHA1(1dcb375987fe21e0483c27d485c405de53848d61) )
	ROM_LOAD( "s2.2e",        0x0800, 0x0800, CRC(e7bba1b3) SHA1(240877576045fddcc9ff01d97dc78139454ac4f1) )
	ROM_LOAD( "s3.2f",        0x1000, 0x0800, CRC(12d7fc3e) SHA1(a84d191c7be8700f630a83ddad798be9e83b5d55) )
	ROM_LOAD( "s4.2h",        0x1800, 0x0800, CRC(b59360eb) SHA1(5d155808c19dcf2e14aa8e29c0ee41a6d3d3c43a) )
	ROM_LOAD( "s5.2j",        0x2000, 0x0800, CRC(4919a91c) SHA1(9cb5861c61e4783e5fbaa3869d51195f127b1129) )
	ROM_LOAD( "s6.2l",        0x2800, 0x0800, CRC(26a4547b) SHA1(67c0fa81729370631647b5d78bb5a61433facd7f) )
	ROM_LOAD( "s7.2m",        0x3000, 0x0800, CRC(0bb49470) SHA1(05a6fe3010c2136284ca76352dac147797c79778) )
	ROM_LOAD( "s8.2p",        0x3800, 0x0800, CRC(6a5740e5) SHA1(e3b09141cee26857d626412e9d1a0e759469b97a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ot1.5c",       0x0000, 0x0800, CRC(bcd297f0) SHA1(8ed78487d76fd0a917ab7b258937a46e2cd9800c) )
	ROM_LOAD( "ot2.5d",       0x0800, 0x0800, CRC(de7912da) SHA1(8558b4eff5d7e63029b325edef9914feda5834c3) )
	ROM_LOAD( "ot3.5e",       0x1000, 0x0800, CRC(ba2fa933) SHA1(1f976d8595706730e29f93027e7ab4620075c078) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "c2.5f",        0x0000, 0x0800, CRC(4708845b) SHA1(a8b1ad19a95a9d35050a2ab7194cc96fc5afcdc9) )
	ROM_LOAD( "c1.5h",        0x0800, 0x0800, CRC(11fd2887) SHA1(69844e48bb4d372cac7ae83c953df573c7ecbb7f) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "c01s.6e",      0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )
ROM_END

ROM_START( scrambles )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2d",           0x0000, 0x0800, CRC(b89207a1) SHA1(5422df979e82bcc73df49f50515fe76c126c037b) ) // sldh
	ROM_LOAD( "2e",           0x0800, 0x0800, CRC(e9b4b9eb) SHA1(a8ee9ddfadf5e9accedfaf81da757a88a2e55a0a) ) // sldh
	ROM_LOAD( "2f",           0x1000, 0x0800, CRC(a1f14f4c) SHA1(3eae2b3e4596505a8afb5c5cfb108e823c2c4319) ) // sldh
	ROM_LOAD( "2h",           0x1800, 0x0800, CRC(591bc0d9) SHA1(170f9e92f0a3bee04407be27210b4fa825367688) ) // sldh
	ROM_LOAD( "2j",           0x2000, 0x0800, CRC(22f11b6b) SHA1(e426ef6a7444a39a34d59799973b07d11b89f372) ) // sldh
	ROM_LOAD( "2l",           0x2800, 0x0800, CRC(705ffe49) SHA1(174df3f281068c767344f751daace646360e26d6) ) // sldh
	ROM_LOAD( "2m",           0x3000, 0x0800, CRC(ea26c35c) SHA1(a2f3380982d93a022f46756f974fd16c4cd617de) ) // sldh
	ROM_LOAD( "2p",           0x3800, 0x0800, CRC(94d8f5e3) SHA1(f3a9c4d1d91836476fcad87ea0d243dde7171e0a) ) // sldh

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ot1.5c",       0x0000, 0x0800, CRC(bcd297f0) SHA1(8ed78487d76fd0a917ab7b258937a46e2cd9800c) )
	ROM_LOAD( "ot2.5d",       0x0800, 0x0800, CRC(de7912da) SHA1(8558b4eff5d7e63029b325edef9914feda5834c3) )
	ROM_LOAD( "ot3.5e",       0x1000, 0x0800, CRC(ba2fa933) SHA1(1f976d8595706730e29f93027e7ab4620075c078) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5f",           0x0000, 0x0800, CRC(5f30311a) SHA1(d64134089bebd995b3a1a089411e180c8c29f32d) )
	ROM_LOAD( "5h",           0x0800, 0x0800, CRC(516e029e) SHA1(81b44eb1ce43cebde87f0a41ade2e7eb291af78d) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "c01s.6e",      0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )
ROM_END

ROM_START( scrambles2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2d",           0x0000, 0x0800, CRC(defae436) SHA1(b32f709069d2484275c88482f865f2758865729e) )
	ROM_LOAD( "2e",           0x0800, 0x0800, CRC(136bf894) SHA1(ecb4c245bdceedd1151707b05c2036941302776b) )
	ROM_LOAD( "2f",           0x1000, 0x0800, CRC(539ff711) SHA1(21b8cdbeebedd9436e88037fb3a01ae9ef7cd4d1) )
	ROM_LOAD( "2h",           0x1800, 0x0800, CRC(b59360eb) SHA1(5d155808c19dcf2e14aa8e29c0ee41a6d3d3c43a) )
	ROM_LOAD( "2j",           0x2000, 0x0800, CRC(d5aefbd5) SHA1(9001d969334c5791a9157e2b3558ab5cbb27714d) )
	ROM_LOAD( "2l",           0x2800, 0x0800, CRC(d6f0dfc9) SHA1(5581ba9894ea9f0067466ef5c7ff99bff92581da) )
	ROM_LOAD( "2m",           0x3000, 0x0800, CRC(d69e0980) SHA1(b0983f39929183bafddaf5703fc613dcc9a31b63) )
	ROM_LOAD( "2p",           0x3800, 0x0800, CRC(4fc94e8c) SHA1(cc9ed40257a0f6001bb7e8722ce6ff909e1f0b4b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ot1.5c",       0x0000, 0x0800, CRC(bcd297f0) SHA1(8ed78487d76fd0a917ab7b258937a46e2cd9800c) )
	ROM_LOAD( "ot2.5d",       0x0800, 0x0800, CRC(de7912da) SHA1(8558b4eff5d7e63029b325edef9914feda5834c3) )
	ROM_LOAD( "ot3.5e",       0x1000, 0x0800, CRC(ba2fa933) SHA1(1f976d8595706730e29f93027e7ab4620075c078) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5f",           0x0000, 0x0800, CRC(5f30311a) SHA1(d64134089bebd995b3a1a089411e180c8c29f32d) )
	ROM_LOAD( "5h",           0x0800, 0x0800, CRC(516e029e) SHA1(81b44eb1ce43cebde87f0a41ade2e7eb291af78d) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "c01s.6e",      0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )
ROM_END

ROM_START( scramrf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2c.cpu",     0x0000, 0x1000, CRC(85fa7de3) SHA1(120392c9949b43b6e937afe4bc25605b2878758f) ) // sldh
	ROM_LOAD( "2f.cpu",     0x1000, 0x1000, CRC(5b500c90) SHA1(45b1bad1d93f3e474395a193d2fe00e105ba39e5) ) // sldh
	ROM_LOAD( "2j.cpu",     0x2000, 0x1000, CRC(179e1c1f) SHA1(a86d3e7a1340a396e418df4580e73fb6c76ce175) ) // sldh
	ROM_LOAD( "2m.cpu",     0x3000, 0x1000, CRC(4a4bb870) SHA1(e551c0876df85375997dc468b3bb5970cec752d8) ) // sldh

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "5c.cpu",     0x0000, 0x0800, CRC(be037cf6) SHA1(f28e5ead496e70beaada24775aa58bd5d75f2d25) ) // sldh
	ROM_LOAD( "5d.cpu",     0x0800, 0x0800, CRC(de7912da) SHA1(8558b4eff5d7e63029b325edef9914feda5834c3) ) // sldh
	ROM_LOAD( "5e.cpu",     0x1000, 0x0800, CRC(ba2fa933) SHA1(1f976d8595706730e29f93027e7ab4620075c078) ) // sldh

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5f.cpu",     0x0000, 0x0800, CRC(4708845b) SHA1(a8b1ad19a95a9d35050a2ab7194cc96fc5afcdc9) ) // sldh
	ROM_LOAD( "5h.cpu",     0x0800, 0x0800, CRC(11fd2887) SHA1(69844e48bb4d372cac7ae83c953df573c7ecbb7f) ) // sldh

	ROM_REGION( 0x0020, "proms", 0 ) // not confirmed
	ROM_LOAD( "c01s.6e",      0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )
ROM_END

ROM_START( scramblebf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "scf1.2d",      0x0000, 0x0800, CRC(b126aa1f) SHA1(1e4db9ee891711e880273241e74e932b0f4e3a0b) )
	ROM_LOAD( "scf2.2e",      0x0800, 0x0800, CRC(ce25fb77) SHA1(faaa2e5735075090548217b80b736b2eebf21dff) )
	ROM_LOAD( "scns3.2f",     0x1000, 0x0800, CRC(eec265ee) SHA1(29b6cf6b93220414eb58cddeba591dc8813c4935) )
	ROM_LOAD( "scns4.2h",     0x1800, 0x0800, CRC(dd380a22) SHA1(125e713a58cc5f2c1e38f67dad29f8c985ce5a8b) )
	ROM_LOAD( "scns5.2j",     0x2000, 0x0800, CRC(92980e72) SHA1(7e0605b461ace534f8f91028bb82968ecd907ca1) )
	ROM_LOAD( "scns6.2l",     0x2800, 0x0800, CRC(9fd96374) SHA1(c8456dd8a012353a023a2d3fa5d508e49c36ace8) )
	ROM_LOAD( "scns7.2m",     0x3000, 0x0800, CRC(88ac07a0) SHA1(c57061db5984b472039356bf84a050b5b66e3813) )
	ROM_LOAD( "scns8.2p",     0x3800, 0x0800, CRC(75232e09) SHA1(b0da201bf05c63031cdbe9f7059e3c710557f33d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ot1ns.5c",     0x0000, 0x0800, CRC(be037cf6) SHA1(f28e5ead496e70beaada24775aa58bd5d75f2d25) )
	ROM_LOAD( "ot2.5d",       0x0800, 0x0800, CRC(de7912da) SHA1(8558b4eff5d7e63029b325edef9914feda5834c3) )
	ROM_LOAD( "ot3.5e",       0x1000, 0x0800, CRC(ba2fa933) SHA1(1f976d8595706730e29f93027e7ab4620075c078) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "c2.5f",        0x0000, 0x0800, CRC(4708845b) SHA1(a8b1ad19a95a9d35050a2ab7194cc96fc5afcdc9) )
	ROM_LOAD( "c1.5h",        0x0800, 0x0800, CRC(11fd2887) SHA1(69844e48bb4d372cac7ae83c953df573c7ecbb7f) )

	ROM_REGION( 0x0020, "proms", 0 ) // should be different on this bootleg..
	ROM_LOAD( "c01s.6e",      0x0000, 0x0020, BAD_DUMP CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )
ROM_END


ROM_START( scrambp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b1-2716.cpu",     0x0000, 0x0800, CRC(63420017) SHA1(82e7c448297bf789033ed03cbe8fc9ac4525a826) )
	ROM_LOAD( "b2-2716.cpu",     0x0800, 0x0800, CRC(66ebc070) SHA1(ada52d7880185d1ac3a39c94896d5127ea05b14a) )
	ROM_LOAD( "b3-2716.cpu",     0x1000, 0x0800, CRC(317548fd) SHA1(687c309d476cd5fc830d90e9e6293d1dcab96df7) )
	ROM_LOAD( "b4-2716.cpu",     0x1800, 0x0800, CRC(dd380a22) SHA1(125e713a58cc5f2c1e38f67dad29f8c985ce5a8b) )
	ROM_LOAD( "b5-2716.cpu",     0x2000, 0x0800, CRC(fa4f1a70) SHA1(9d797eaab0f19a2ed003f782716719c9d752bd56) )
	ROM_LOAD( "b6-2716.cpu",     0x2800, 0x0800, CRC(9fd96374) SHA1(c8456dd8a012353a023a2d3fa5d508e49c36ace8) )
	ROM_LOAD( "b7-2716.cpu",     0x3000, 0x0800, CRC(88ac07a0) SHA1(c57061db5984b472039356bf84a050b5b66e3813) )
	ROM_LOAD( "b8-2716.cpu",     0x3800, 0x0800, CRC(d20088ee) SHA1(4b2deb64f1185780e5b6d1527ed5f691591b9ea0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b11-2716.cpu",     0x0000, 0x0800, CRC(be037cf6) SHA1(f28e5ead496e70beaada24775aa58bd5d75f2d25) )
	ROM_LOAD( "b12-2716.cpu",     0x0800, 0x0800, CRC(de7912da) SHA1(8558b4eff5d7e63029b325edef9914feda5834c3) )
	ROM_LOAD( "b13-2716.cpu",     0x1000, 0x0800, CRC(ba2fa933) SHA1(1f976d8595706730e29f93027e7ab4620075c078) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "b9-2716.cpu",         0x0000, 0x0800, CRC(4708845b) SHA1(a8b1ad19a95a9d35050a2ab7194cc96fc5afcdc9) )
	ROM_LOAD( "b10-2716.cpu",        0x0800, 0x0800, CRC(11fd2887) SHA1(69844e48bb4d372cac7ae83c953df573c7ecbb7f) )

	ROM_REGION( 0x0020, "proms", 0 ) // not verified
	ROM_LOAD( "c01s.6e",      0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )
ROM_END

// mostly the same as the scrambp set above, complete dump
ROM_START( scramce )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "es1.2c",     0x0000, 0x0800, CRC(726fb19e) SHA1(4d6d8cf1bb711ab1f13cefc56ba7273f2496d037) ) // only unique rom
	ROM_LOAD( "es2.2e",     0x0800, 0x0800, CRC(66ebc070) SHA1(ada52d7880185d1ac3a39c94896d5127ea05b14a) )
	ROM_LOAD( "es3.2f",     0x1000, 0x0800, CRC(317548fd) SHA1(687c309d476cd5fc830d90e9e6293d1dcab96df7) )
	ROM_LOAD( "es4.2h",     0x1800, 0x0800, CRC(dd380a22) SHA1(125e713a58cc5f2c1e38f67dad29f8c985ce5a8b) )
	ROM_LOAD( "es5.2j",     0x2000, 0x0800, CRC(fa4f1a70) SHA1(9d797eaab0f19a2ed003f782716719c9d752bd56) )
	ROM_LOAD( "es6.2l",     0x2800, 0x0800, CRC(9fd96374) SHA1(c8456dd8a012353a023a2d3fa5d508e49c36ace8) )
	ROM_LOAD( "es7.2m",     0x3000, 0x0800, CRC(88ac07a0) SHA1(c57061db5984b472039356bf84a050b5b66e3813) )
	ROM_LOAD( "es8.2p",     0x3800, 0x0800, CRC(d20088ee) SHA1(4b2deb64f1185780e5b6d1527ed5f691591b9ea0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "11.5c",     0x0000, 0x0800, CRC(be037cf6) SHA1(f28e5ead496e70beaada24775aa58bd5d75f2d25) )
	ROM_LOAD( "12.5d",     0x0800, 0x0800, CRC(de7912da) SHA1(8558b4eff5d7e63029b325edef9914feda5834c3) )
	ROM_LOAD( "13.5e",     0x1000, 0x0800, CRC(ba2fa933) SHA1(1f976d8595706730e29f93027e7ab4620075c078) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "9.5f",         0x0000, 0x0800, CRC(4708845b) SHA1(a8b1ad19a95a9d35050a2ab7194cc96fc5afcdc9) )
	ROM_LOAD( "10.5h",        0x0800, 0x0800, CRC(11fd2887) SHA1(69844e48bb4d372cac7ae83c953df573c7ecbb7f) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "prom7051.6e",      0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )
ROM_END

ROM_START( scrampt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cx8-2716.cpu",     0x0000, 0x0800, CRC(12b97cc6) SHA1(edcd98fafe1001d033a73279d6dfb8336ce164de) )
	ROM_LOAD( "cx4-2716.cpu",     0x0800, 0x0800, CRC(66ebc070) SHA1(ada52d7880185d1ac3a39c94896d5127ea05b14a) )
	ROM_LOAD( "cx9-2716.cpu",     0x1000, 0x0800, CRC(317548fd) SHA1(687c309d476cd5fc830d90e9e6293d1dcab96df7) )
	ROM_LOAD( "cx5-2716.cpu",     0x1800, 0x0800, CRC(dd380a22) SHA1(125e713a58cc5f2c1e38f67dad29f8c985ce5a8b) )
	ROM_LOAD( "cx10-2716.cpu",    0x2000, 0x0800, CRC(fa4f1a70) SHA1(9d797eaab0f19a2ed003f782716719c9d752bd56) )
	ROM_LOAD( "cx6-2716.cpu",     0x2800, 0x0800, CRC(9fd96374) SHA1(c8456dd8a012353a023a2d3fa5d508e49c36ace8) )
	ROM_LOAD( "cx11-2716.cpu",    0x3000, 0x0800, CRC(88ac07a0) SHA1(c57061db5984b472039356bf84a050b5b66e3813) )
	ROM_LOAD( "cx7-2716.cpu",     0x3800, 0x0800, CRC(c9a6c489) SHA1(01aa49c5c75f76affcd2057afc5f9b57098a3374) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "cx3-2716.cpu",       0x0000, 0x0800, CRC(be037cf6) SHA1(f28e5ead496e70beaada24775aa58bd5d75f2d25) )
	ROM_LOAD( "cx2-2716.cpu",       0x0800, 0x0800, CRC(de7912da) SHA1(8558b4eff5d7e63029b325edef9914feda5834c3) )
	ROM_LOAD( "cx1-2716.cpu",       0x1000, 0x0800, CRC(ba2fa933) SHA1(1f976d8595706730e29f93027e7ab4620075c078) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "cx12-2716.cpu",        0x0000, 0x0800, CRC(4708845b) SHA1(a8b1ad19a95a9d35050a2ab7194cc96fc5afcdc9) )
	ROM_LOAD( "cx13-2716.cpu",        0x0800, 0x0800, CRC(11fd2887) SHA1(69844e48bb4d372cac7ae83c953df573c7ecbb7f) )

	ROM_REGION( 0x0020, "proms", 0 ) // not verified
	ROM_LOAD( "c01s.6e",      0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )
ROM_END


ROM_START( scramblebb ) // no PCB, just eproms...
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1",      0x0000, 0x0800, CRC(8ba174c4) SHA1(9ff48669054e4f55a19cb2d317a9d7a5e400e86c) )
	ROM_LOAD( "2",      0x0800, 0x0800, CRC(43cb40a4) SHA1(4e500f63a06865a5fd9a7d920eb866ea610a4d92) )
	ROM_LOAD( "3",      0x1000, 0x0800, CRC(eec265ee) SHA1(29b6cf6b93220414eb58cddeba591dc8813c4935) )
	ROM_LOAD( "4",      0x1800, 0x0800, CRC(dd380a22) SHA1(125e713a58cc5f2c1e38f67dad29f8c985ce5a8b) )
	ROM_LOAD( "5",      0x2000, 0x0800, CRC(92980e72) SHA1(7e0605b461ace534f8f91028bb82968ecd907ca1) )
	ROM_LOAD( "6",      0x2800, 0x0800, CRC(9fd96374) SHA1(c8456dd8a012353a023a2d3fa5d508e49c36ace8) )
	ROM_LOAD( "7",      0x3000, 0x0800, CRC(88ac07a0) SHA1(c57061db5984b472039356bf84a050b5b66e3813) )
	ROM_LOAD( "8",      0x3800, 0x0800, CRC(75232e09) SHA1(b0da201bf05c63031cdbe9f7059e3c710557f33d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ot1.5c",  0x0000, 0x0800, BAD_DUMP CRC(bcd297f0) SHA1(8ed78487d76fd0a917ab7b258937a46e2cd9800c) )    // need proper dump
	ROM_LOAD( "ot2.5d",  0x0800, 0x0800, BAD_DUMP CRC(de7912da) SHA1(8558b4eff5d7e63029b325edef9914feda5834c3) )    // need proper dump
	ROM_LOAD( "ot3.5e",  0x1000, 0x0800, BAD_DUMP CRC(ba2fa933) SHA1(1f976d8595706730e29f93027e7ab4620075c078) )    // need proper dump

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "9",      0x0000, 0x0800, CRC(4708845b) SHA1(a8b1ad19a95a9d35050a2ab7194cc96fc5afcdc9) )
	ROM_LOAD( "10",     0x0800, 0x0800, CRC(11fd2887) SHA1(69844e48bb4d372cac7ae83c953df573c7ecbb7f) )

	ROM_REGION( 0x0020, "proms", 0 ) // should be different on this bootleg..
	ROM_LOAD( "c01s.6e", 0x0000, 0x0020, BAD_DUMP CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )    // need proper dump
ROM_END

ROM_START( strfbomb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.2c",         0x0000, 0x0800, CRC(b102aaa0) SHA1(00560da7a2ded6afcdc1d46e12cc3c795654639a) )
	ROM_LOAD( "2.2e",         0x0800, 0x0800, CRC(d4155703) SHA1(defd37df55536890456c29812340e0d6b4292b78) )
	ROM_LOAD( "3.2f",         0x1000, 0x0800, CRC(a9568c89) SHA1(0d8e6b3af92e4933814700d54acfd43407f3ede1) )
	ROM_LOAD( "4.2h",         0x1800, 0x0800, CRC(663b6c35) SHA1(354fb2e92f4376b20aee412ed361d59b8a2c01e1) )
	ROM_LOAD( "5.2j",         0x2000, 0x0800, CRC(4919a91c) SHA1(9cb5861c61e4783e5fbaa3869d51195f127b1129) )
	ROM_LOAD( "6.2l",         0x2800, 0x0800, CRC(4ec66ae3) SHA1(a74827e161212e9b2eddd980321507a377f1e30b) )
	ROM_LOAD( "7.2m",         0x3000, 0x0800, CRC(0feb0192) SHA1(45a44bde3bf1483abf95fe1d1d5066bfcb1736df) )
	ROM_LOAD( "8.2p",         0x3800, 0x0800, CRC(280a6142) SHA1(f17625b91eaaffa36a433be32e4e80651d94b3b9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ot1.5c",       0x0000, 0x0800, CRC(bcd297f0) SHA1(8ed78487d76fd0a917ab7b258937a46e2cd9800c) )
	ROM_LOAD( "ot2.5d",       0x0800, 0x0800, CRC(de7912da) SHA1(8558b4eff5d7e63029b325edef9914feda5834c3) )
	ROM_LOAD( "ot3.5e",       0x1000, 0x0800, CRC(ba2fa933) SHA1(1f976d8595706730e29f93027e7ab4620075c078) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "9.5f",         0x0000, 0x0800, CRC(3abeff25) SHA1(ff6de0596c849ec877fb759c1ab9c7a8ffe2edac) )
	ROM_LOAD( "10.5h",        0x0800, 0x0800, CRC(79ecacbe) SHA1(285cb3ee0ff8d596877bb571ea8479566ab36eb9) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "c01s.6e",      0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )
ROM_END

ROM_START( explorer )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "10l.bin",      0x0000, 0x1000, CRC(d5adf626) SHA1(f362322f780c13cee73697f9158a8ca8aa943a2e) )
	ROM_LOAD( "9l.bin",       0x1000, 0x1000, CRC(48e32788) SHA1(7a98848d2ed8ba5b2da28c014226109af7cc9287) )
	ROM_LOAD( "8l.bin",       0x2000, 0x1000, CRC(c0dbdbde) SHA1(eac7444246bdf80f97962031bf900ce09b28c8b5) )
	ROM_LOAD( "7l.bin",       0x3000, 0x1000, CRC(9b30d227) SHA1(22764e0a2a5ce7abe862e42c84abaaf25949575f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "3f.bin",       0x0000, 0x1000, CRC(9faf18cf) SHA1(1b6c65472d639753cc39031750f85efe1d31ae5e) )
	ROM_LOAD( "4b.bin",       0x1000, 0x0800, CRC(e910b5c3) SHA1(228e8d36dd1ac8a00a396df74b80aa6616997028) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "c2.5f",        0x0000, 0x0800, CRC(4708845b) SHA1(a8b1ad19a95a9d35050a2ab7194cc96fc5afcdc9) )
	ROM_LOAD( "c1.5h",        0x0800, 0x0800, CRC(11fd2887) SHA1(69844e48bb4d372cac7ae83c953df573c7ecbb7f) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "c01s.6e",      0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )
ROM_END


ROM_START( atlantis )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2c",           0x0000, 0x0800, CRC(0e485b9a) SHA1(976f1d6f4552fbee134359a776b5688588824cbb) )
	ROM_LOAD( "2e",           0x0800, 0x0800, CRC(c1640513) SHA1(a0dfb34f401330b16e9e4d66ec4b49d120499606) )
	ROM_LOAD( "2f",           0x1000, 0x0800, CRC(eec265ee) SHA1(29b6cf6b93220414eb58cddeba591dc8813c4935) )
	ROM_LOAD( "2h",           0x1800, 0x0800, CRC(a5d2e442) SHA1(e535d1a501ebd861ad62da70b87215fb7c23de1d) )
	ROM_LOAD( "2j",           0x2000, 0x0800, CRC(45f7cf34) SHA1(d1e0e0be6dec377b684625bdfdc5a3a8af847492) )
	ROM_LOAD( "2l",           0x2800, 0x0800, CRC(f335b96b) SHA1(17daa6d9bc916081f3c6cbdfe5b4960177dc7c9b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ot1.5c",       0x0000, 0x0800, CRC(bcd297f0) SHA1(8ed78487d76fd0a917ab7b258937a46e2cd9800c) )
	ROM_LOAD( "ot2.5d",       0x0800, 0x0800, CRC(de7912da) SHA1(8558b4eff5d7e63029b325edef9914feda5834c3) )
	ROM_LOAD( "ot3.5e",       0x1000, 0x0800, CRC(ba2fa933) SHA1(1f976d8595706730e29f93027e7ab4620075c078) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5f",           0x0000, 0x0800, CRC(57f9c6b9) SHA1(ad0d09a6611998d093d676a9c9fe9e32b10f643e) )
	ROM_LOAD( "5h",           0x0800, 0x0800, CRC(e989f325) SHA1(947aee915779687deae040aeef9e9aee680aaebf) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "c01s.6e",      0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )
ROM_END

ROM_START( atlantis2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "boa_1.2c",     0x0000, 0x0800, CRC(ad348089) SHA1(3548b94192c451c0126e7aaecefa7137ae074cd3) )
	ROM_LOAD( "boa_2.2e",     0x0800, 0x0800, CRC(caa705d1) SHA1(b4aefbea21fa9608e1dae2a09ae0d31270eb8c78) )
	ROM_LOAD( "boa_3.2f",     0x1000, 0x0800, CRC(ac5e9ec1) SHA1(0402e5241d99759d804291998efd43f37ce99917) )
	ROM_LOAD( "boa_4.2h",     0x1800, 0x0800, CRC(04792d90) SHA1(cb477e4b8e4538def01c10b0348f8f8e3a2a9500) )
	ROM_LOAD( "boa_5.2j",     0x2000, 0x0800, CRC(45f7cf34) SHA1(d1e0e0be6dec377b684625bdfdc5a3a8af847492) )
	ROM_LOAD( "boa_6.2l",     0x2800, 0x0800, CRC(b297bd4b) SHA1(0c48da41d9cf2a3456df5b1e8bf27fa641bc643b) )
	ROM_LOAD( "boa_7.2m",     0x3000, 0x0800, CRC(a50bf8d5) SHA1(5bca98e1c0838d27ec66bf4b906877977b212b6d) )
	ROM_LOAD( "boa_8.2p",     0x3800, 0x0800, CRC(d2c5c984) SHA1(a9432f9aff8a2f5ca1d347443efc008a177d8ae0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "boa_11.5c",    0x0000, 0x0800, CRC(be037cf6) SHA1(f28e5ead496e70beaada24775aa58bd5d75f2d25) )
	ROM_LOAD( "boa_12.5d",    0x0800, 0x0800, CRC(de7912da) SHA1(8558b4eff5d7e63029b325edef9914feda5834c3) )
	ROM_LOAD( "boa_13.5e",    0x1000, 0x0800, CRC(ba2fa933) SHA1(1f976d8595706730e29f93027e7ab4620075c078) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "boa_9.5f",     0x0000, 0x0800, CRC(55cd5acd) SHA1(b3e2ce71d4e48255d44cd451ee015a7234a108c8) )
	ROM_LOAD( "boa_10.5h",    0x0800, 0x0800, CRC(72e773b8) SHA1(6ce178df3bd6a4177c68761572a13a56d222c48f) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "c01s.6e",      0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )
ROM_END

ROM_START( bomber )
	/* Bootleg of scramble from Alca */
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64k for code */
	ROM_LOAD( "1.3l",         0x0000, 0x0800, CRC(8c30c7c3) SHA1(39e6244ac6b6c711ab83a6953d4b0585e035dfec) )
	ROM_LOAD( "2.3k",         0x0800, 0x0800, CRC(1fca370c) SHA1(b48b67afe226b13656a1585a1d7ad4b6322c58a1) )
	ROM_LOAD( "3.3h",         0x1000, 0x0800, CRC(8a714167) SHA1(34ed01fd9e9efa5cd9067284a2b66b72cafe3209) )
	ROM_LOAD( "4.3f",         0x1800, 0x0800, CRC(dd380a22) SHA1(125e713a58cc5f2c1e38f67dad29f8c985ce5a8b) )
	ROM_LOAD( "5.3e",         0x2000, 0x0800, CRC(92980e72) SHA1(7e0605b461ace534f8f91028bb82968ecd907ca1) )
	ROM_LOAD( "6.3d",         0x2800, 0x0800, CRC(9fd96374) SHA1(c8456dd8a012353a023a2d3fa5d508e49c36ace8) )
	ROM_LOAD( "7.3c",         0x3000, 0x0800, CRC(88ac07a0) SHA1(c57061db5984b472039356bf84a050b5b66e3813) )
	ROM_LOAD( "8.3a",         0x3800, 0x0800, CRC(75232e09) SHA1(b0da201bf05c63031cdbe9f7059e3c710557f33d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the audio CPU */
	ROM_LOAD( "11.8k",        0x0000, 0x0800, CRC(97ba15e8) SHA1(2729ccb156540ace2360b03c485865f7f97f5368) )
	ROM_LOAD( "12.8l",        0x0800, 0x0800, CRC(6510761d) SHA1(0df92f9f123447d59a9106b2351d680cb04d1a9e) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "c2.5f",        0x0000, 0x0800, CRC(4708845b) SHA1(a8b1ad19a95a9d35050a2ab7194cc96fc5afcdc9) ) // 10.4k on pcb
	ROM_LOAD( "c1.5h",        0x0800, 0x0800, CRC(11fd2887) SHA1(69844e48bb4d372cac7ae83c953df573c7ecbb7f) ) // 9.4l on pcb

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "c01s.6e",      0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) ) // q.9c on pcb
ROM_END

ROM_START( scorpion )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.2d",         0x0000, 0x1000, CRC(ba1219b4) SHA1(33c7843dba44152a8bc3223ea0c30b13609b80ba) )
	ROM_LOAD( "2.2f",         0x1000, 0x1000, CRC(c3909ab6) SHA1(0bec902ae4291fa0530f4c89ad45cc7aab888b7a) )
	ROM_LOAD( "3.2g",         0x2000, 0x1000, CRC(43261352) SHA1(49468cbed7e0286b260eef297bd5fad0ab9fd45b) )
	ROM_LOAD( "4.2h",         0x3000, 0x1000, CRC(aba2276a) SHA1(42b0378f06d2bdb4faaaa95274a6c0f965716877) )
	ROM_LOAD( "5.2k",         0x6000, 0x0800, CRC(952f78f2) SHA1(9562037b104fc1852c2d2650209a77ffce2cb90e) )
	ROM_CONTINUE(             0x5800, 0x0800 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "igr_scorpion_32_a4.ic12", 0x0000, 0x1000, CRC(361b8a36) SHA1(550ac5f721aaa9fea5f6d63ba590d6b367525c23) )
	ROM_LOAD( "igr_scorpion_32_a5.ic13", 0x1000, 0x1000, CRC(addecdd4) SHA1(ba28f1d9c7c6b5e8ecef56a4b3f64be13fc10d43) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "igr_scorpion_32_f5.ic72", 0x0000, 0x1000, CRC(1e5da9d6) SHA1(ca8b27e6dd40e4ca13e7e6b5f813bafca78b62f4) )
	ROM_LOAD( "igr_scorpion_32_h5.ic73", 0x1000, 0x1000, CRC(a57adb0a) SHA1(d97c7dc4a6c5efb59cc0148e2498156c682c6714) )

	ROM_REGION( 0x3000, "digitalker", 0 ) /* Digitalker speech samples */
	ROM_LOAD( "igr_scorpion_32_a3.ic25", 0x0000, 0x1000, CRC(04abf178) SHA1(2e7f231413d9ec461ca21840f31d1d6b8b17c4d5) )
	ROM_LOAD( "igr_scorpion_32_a2.ic24", 0x1000, 0x1000, CRC(90352dd4) SHA1(62c261a2f2fbd8eff31d5c72cf532d5e43d86dd3) )
	ROM_LOAD( "igr_scorpion_32_a1.ic23", 0x2000, 0x1000, CRC(3bf2452d) SHA1(7a163e0ef108dd40d3beab5e9805886e45be744b) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mmi6331_6e.ic59", 0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) ) /* BPROM type MMI 6331 */
ROM_END

ROM_START( scorpiona ) /* Scorpion was developed by I.G.R. and original labels have "I.G.R. SCORPION" printed them */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "igr_scorpion_32_d2.ic109",  0x0000, 0x1000, CRC(c5b9daeb) SHA1(faf7a22013dd5f063eb8f506f3722cfd5522539a) )
	ROM_LOAD( "igr_scorpion_32_e2.ic110",  0x1000, 0x1000, CRC(82308d05) SHA1(26bc7c8b3ea0020fd1b93f6aaa29d82d04ae64b2) )
	ROM_LOAD( "igr_scorpion_32_g2.ic111",  0x2000, 0x1000, CRC(756b09cd) SHA1(9aec34e063fe8c0d1392db09daea2875d06eec46) )
	ROM_LOAD( "igr_scorpion_32_h2.ic112",  0x3000, 0x1000, CRC(667ad8be) SHA1(2b2dcd32d52c0173a1fd93da9a8a1ccb669c7d55) )
	ROM_LOAD( "igr_scorpion_16_k2.ic113",  0x5800, 0x0800, CRC(42ec34d8) SHA1(b358d10a96490f325420b992e8e03bb3884e415a) )
	ROM_LOAD( "igr_scorpion_16_l2.ic114",  0x6000, 0x0800, CRC(6623da33) SHA1(99110005d00c80d674bde5d21608f50b85ee488c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "igr_scorpion_32_a4.ic12", 0x0000, 0x1000, CRC(361b8a36) SHA1(550ac5f721aaa9fea5f6d63ba590d6b367525c23) )
	ROM_LOAD( "igr_scorpion_32_a5.ic13", 0x1000, 0x1000, CRC(addecdd4) SHA1(ba28f1d9c7c6b5e8ecef56a4b3f64be13fc10d43) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "igr_scorpion_32_f5.ic72", 0x0000, 0x1000, CRC(1e5da9d6) SHA1(ca8b27e6dd40e4ca13e7e6b5f813bafca78b62f4) )
	ROM_LOAD( "igr_scorpion_32_h5.ic73", 0x1000, 0x1000, CRC(a57adb0a) SHA1(d97c7dc4a6c5efb59cc0148e2498156c682c6714) )

	ROM_REGION( 0x3000, "digitalker", 0 ) /* Digitalker speech samples */
	ROM_LOAD( "igr_scorpion_32_a3.ic25", 0x0000, 0x1000, CRC(04abf178) SHA1(2e7f231413d9ec461ca21840f31d1d6b8b17c4d5) )
	ROM_LOAD( "igr_scorpion_32_a2.ic24", 0x1000, 0x1000, CRC(90352dd4) SHA1(62c261a2f2fbd8eff31d5c72cf532d5e43d86dd3) )
	ROM_LOAD( "igr_scorpion_32_a1.ic23", 0x2000, 0x1000, CRC(3bf2452d) SHA1(7a163e0ef108dd40d3beab5e9805886e45be744b) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mmi6331_6e.ic59", 0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) ) /* BPROM type MMI 6331 */
ROM_END

ROM_START( scorpionb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic109.2d", 0x0000, 0x1000, CRC(f54688c9) SHA1(2881d5238733353b9c277a2829d157688a87601b) )
	ROM_LOAD( "ic110.2e", 0x1000, 0x1000, CRC(91aaaa12) SHA1(2a5e5eb5aeec5637ee4675930f67158a969e0d5d) )
	ROM_LOAD( "ic111.2g", 0x2000, 0x1000, CRC(4c3720da) SHA1(5b2758c8a91f9463bf98abf2b52af946c5e90cf0) )
	ROM_LOAD( "ic112.2h", 0x3000, 0x1000, CRC(53e2a983) SHA1(cfe272055a92793de76dd1cff617b13281815485) )
	ROM_LOAD( "ic113.2k", 0x6000, 0x0800, CRC(e4ad299a) SHA1(712cab86eadeba9e859f7bae98eb289f00d2e217) )
	ROM_CONTINUE(         0x5800, 0x0800 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "igr_scorpion_32_a4.ic12", 0x0000, 0x1000, CRC(361b8a36) SHA1(550ac5f721aaa9fea5f6d63ba590d6b367525c23) ) /* missing from this set */
	ROM_LOAD( "igr_scorpion_32_a5.ic13", 0x1000, 0x1000, CRC(addecdd4) SHA1(ba28f1d9c7c6b5e8ecef56a4b3f64be13fc10d43) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "igr_scorpion_32_f5.ic72", 0x0000, 0x1000, CRC(1e5da9d6) SHA1(ca8b27e6dd40e4ca13e7e6b5f813bafca78b62f4) )
	ROM_LOAD( "igr_scorpion_32_h5.ic73", 0x1000, 0x1000, CRC(a57adb0a) SHA1(d97c7dc4a6c5efb59cc0148e2498156c682c6714) )

	ROM_REGION( 0x3000, "digitalker", 0 ) /* Digitalker speech samples */
	ROM_LOAD( "igr_scorpion_32_a3.ic25", 0x0000, 0x1000, CRC(04abf178) SHA1(2e7f231413d9ec461ca21840f31d1d6b8b17c4d5) )
	ROM_LOAD( "igr_scorpion_32_a2.ic24", 0x1000, 0x1000, CRC(90352dd4) SHA1(62c261a2f2fbd8eff31d5c72cf532d5e43d86dd3) )
	ROM_LOAD( "igr_scorpion_32_a1.ic23", 0x2000, 0x1000, CRC(3bf2452d) SHA1(7a163e0ef108dd40d3beab5e9805886e45be744b) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mmi6331_6e.ic59", 0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) ) /* BPROM type MMI 6331 */
ROM_END

ROM_START( scorpionmc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p1.bin",       0x0000, 0x0800, CRC(58818d88) SHA1(d9fbfb6fff9ba1d078c3735889b8878ab0442ded) )
	ROM_LOAD( "p2.bin",       0x0800, 0x0800, CRC(8bec5f9f) SHA1(78642124edbf946140f62985bafe2dca314cb257) )
	ROM_LOAD( "p3.bin",       0x1000, 0x0800, CRC(24b7fdff) SHA1(1382199c85af3aa101b4ca01a0b096d801bc61a6) )
	ROM_LOAD( "p4.bin",       0x1800, 0x0800, CRC(9082e2f0) SHA1(8e3beebca33e73901cb8c4fa0af39a299b831d32) )
	ROM_LOAD( "p5.bin",       0x2000, 0x0800, CRC(20387fc0) SHA1(1aa2f16fb1630e65fd8aaedd21d8e4bac343678b) )
	ROM_LOAD( "p6.bin",       0x2800, 0x0800, CRC(f66c48e1) SHA1(bbfd68309d1f6b6bb3ec0879c8bde80b3f30fa02) )
	ROM_LOAD( "p7.bin",       0x3000, 0x0800, CRC(931e34c7) SHA1(8b6c1099634b5d693faab9edf5e29b3c476c7f52) )
	ROM_LOAD( "p8.bin",       0x3800, 0x0800, CRC(ab5ab61d) SHA1(eb78d7dc8d424f6ed4f7cbec02485cd769bac5c6) )
	ROM_LOAD( "p9.bin",       0x5000, 0x1000, CRC(b551b974) SHA1(c19e61dd8b6daf808ba42fa318dfd179a020d20e) )
	ROM_LOAD( "p10.bin",      0x6000, 0x0800, CRC(a7bd8d20) SHA1(c8131279de58298546c5af2c34ff18116c3a2d3b) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "h.bin",        0x0000, 0x1000, CRC(1e5da9d6) SHA1(ca8b27e6dd40e4ca13e7e6b5f813bafca78b62f4) )
	ROM_LOAD( "k.bin",        0x1000, 0x1000, CRC(a57adb0a) SHA1(d97c7dc4a6c5efb59cc0148e2498156c682c6714) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mmi6331.bpr",  0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END

ROM_START( aracnis )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "00sc.bin",       0x0000, 0x1000, CRC(c7e0d6b9) SHA1(3aac51d22939da8a595237ad26fe0f06a3acfb6a) )
	ROM_LOAD( "01sc.bin",       0x1000, 0x1000, CRC(03eb27dc) SHA1(8dae30006c9e81ab0d2b5c5faa7257813ea00a89) )
	ROM_LOAD( "02sc.bin",       0x2000, 0x1000, CRC(f3d49d4f) SHA1(19f603a2bda88e51608414f2748a33b4fb6e31c1) )
	ROM_LOAD( "03sc.bin",       0x3000, 0x1000, CRC(0e741984) SHA1(6e2c7820bbb1834c49f312664c786b50af0cff26) )
	ROM_LOAD( "05sc.bin",       0x5000, 0x1000, CRC(f27ee3e4) SHA1(e034507b99705492b6a8aa34764a1e3222ba31b2) )
	ROM_LOAD( "06sc.bin",       0x6000, 0x0800, CRC(fdfc2c82) SHA1(6b8914d6496c216de5bf160cd798b8f6facd44d2) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "08sc.1h",        0x0000, 0x1000, CRC(1e5da9d6) SHA1(ca8b27e6dd40e4ca13e7e6b5f813bafca78b62f4) )
	ROM_LOAD( "07sc.1k",        0x1000, 0x1000, CRC(a57adb0a) SHA1(d97c7dc4a6c5efb59cc0148e2498156c682c6714) )

	ROM_REGION( 0x0020, "proms", 0 )
	// colours are wrong, but this is the prom that was on the board

	// note: pin 13 is marked with red paint, and is not connected
	//  ^ this is important for getting correct colours on real hw
	ROM_LOAD( "mmi6331-1.6l",  0x0000, 0x0020, CRC(24652bc4) SHA1(d89575f3749c75dc963317fe451ffeffd9856e4d) )
ROM_END

ROM_START( sfx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sfx_b-0.1j",   0x0000, 0x1000, CRC(e5bc6952) SHA1(7bfb772418d738d3c49fd59c0bfc04590945977a) )
	ROM_CONTINUE(             0xe000, 0x1000             )
	ROM_LOAD( "1.1c",         0x1000, 0x1000, CRC(1b3c48e7) SHA1(2f245aaf9b4bb5d949aae18ee89a0be639e7b2df) )
	ROM_LOAD( "22.1d",        0x2000, 0x1000, CRC(ed44950d) SHA1(f8c54ff89ac461171df951d703d5571be1b8da38) )
	ROM_LOAD( "23.1e",        0x3000, 0x1000, CRC(f44a3ca0) SHA1(3917ea960329a06d3d0c447cb6a4ba710fb7ca92) )
	ROM_LOAD( "27.1a",        0x7000, 0x1000, CRC(ed86839f) SHA1(a0d8c941a6e01058eab66d5da9b49b6b5695b981) )
	ROM_LOAD( "24.1g",        0xc000, 0x1000, CRC(e6d7dc74) SHA1(c1e6d9598fb837775ee6550fea3cd4910572615e) )
	ROM_LOAD( "5.1h",         0xd000, 0x1000, CRC(d1e8d390) SHA1(f8fe9f69e6500fbcf25f8151c1070d9a1a20a38c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "5.5j",         0x0000, 0x1000, CRC(59028fb6) SHA1(94105b5b03c81a948a409f7ea20312bb9c79c150) )
	ROM_LOAD( "6.6j",         0x1000, 0x1000, CRC(5427670f) SHA1(ffc3f7186d0319f0fd7ed25eb97bb0db7bc107c6) )

	ROM_REGION( 0x10000, "audio2", 0 )
	ROM_LOAD( "1.1j",         0x0000, 0x1000, CRC(2f172c58) SHA1(4706d55fcfad4d5a87d96a0a0187f59997ef9720) )
	ROM_LOAD( "2.2j",         0x1000, 0x1000, CRC(a6ad2f6b) SHA1(14d1a93e507c349b14a1b26408cce23f089fa33c) )
	ROM_LOAD( "3.3j",         0x2000, 0x1000, CRC(fa1274fa) SHA1(e98cb602b265b209eaa4a9b3972e47c869ff863b) )
	ROM_LOAD( "4.4j",         0x3000, 0x1000, CRC(1cd33f3a) SHA1(cf9248fd6cb56ec81d354afe032a2dea810e834b) )
	ROM_LOAD( "10.3h",        0x4000, 0x1000, CRC(b833a15b) SHA1(0d21aaa0ca5ccba89118b205a6b3b36b15663c47) )
	ROM_LOAD( "11.4h",        0x5000, 0x1000, CRC(cbd76ec2) SHA1(9434350ee93ca71efe78018b69913386353306ff) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "28.5a",        0x0000, 0x1000, CRC(d73a8252) SHA1(59d14f41f1a806f98ee33596b84fe5aefe606944) )
	ROM_LOAD( "29.5c",        0x1000, 0x1000, CRC(1401ccf2) SHA1(5762eafd9f402330e1d4ac677f46595087716c47) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6331.9g",      0x0000, 0x0020, CRC(ca1d9ccd) SHA1(27124759a06497c1bc1a64b6d3faa6ba924a8447) )
ROM_END

ROM_START( skelagon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	/* first half of 36.bin is missing */
	ROM_LOAD( "31.bin",       0x1000, 0x1000, CRC(ae6f8647) SHA1(801e88b91c204f2797e5ce45390ea6eec27a3f54) )
	ROM_LOAD( "32.bin",       0x2000, 0x1000, CRC(a28c5838) SHA1(0a37de7986c494d1522ce76635dd1fa6d03f05c7) )
	ROM_LOAD( "33.bin",       0x3000, 0x1000, CRC(32f7e99c) SHA1(2718063a77eeeb8067a9cad7ff3d9e0266b61566) )
	ROM_LOAD( "37.bin",       0x7000, 0x1000, CRC(47f68a31) SHA1(6e15024f67c88a733ede8702d2a80ddb1892b27e) )
	ROM_LOAD( "24.bin",       0xc000, 0x1000, CRC(e6d7dc74) SHA1(c1e6d9598fb837775ee6550fea3cd4910572615e) )
	ROM_LOAD( "35.bin",       0xd000, 0x1000, CRC(5b2a0158) SHA1(66d2fb05a8daaa86bb547b4860d5bf27b4359326) )
	ROM_LOAD( "36.bin",       0xe000, 0x1000, BAD_DUMP CRC(f53ead29) SHA1(f8957b0c0558acc005f418adbfeb66d1d562c9ac) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "5.5j",         0x0000, 0x1000, CRC(59028fb6) SHA1(94105b5b03c81a948a409f7ea20312bb9c79c150) )
	ROM_LOAD( "6.6j",         0x1000, 0x1000, CRC(5427670f) SHA1(ffc3f7186d0319f0fd7ed25eb97bb0db7bc107c6) )

	ROM_REGION( 0x10000, "audio2", 0 )
	ROM_LOAD( "1.1j",         0x0000, 0x1000, CRC(2f172c58) SHA1(4706d55fcfad4d5a87d96a0a0187f59997ef9720) )
	ROM_LOAD( "2.2j",         0x1000, 0x1000, CRC(a6ad2f6b) SHA1(14d1a93e507c349b14a1b26408cce23f089fa33c) )
	ROM_LOAD( "3.3j",         0x2000, 0x1000, CRC(fa1274fa) SHA1(e98cb602b265b209eaa4a9b3972e47c869ff863b) )
	ROM_LOAD( "4.4j",         0x3000, 0x1000, CRC(1cd33f3a) SHA1(cf9248fd6cb56ec81d354afe032a2dea810e834b) )
	ROM_LOAD( "10.bin",       0x4000, 0x1000, CRC(2c719de2) SHA1(0953e96f8be1cbab3f4a8e166457c74e986a87b1) )
	ROM_LOAD( "8.bin",        0x5000, 0x1000, CRC(350379dd) SHA1(e979251b11d6702170dd60ffd28fc15ea737588b) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "38.bin",       0x0000, 0x1000, CRC(2fffa8b1) SHA1(6a6032f55b9fe1da209e4ed4423042efec773d4d) )
	ROM_LOAD( "39.bin",       0x1000, 0x1000, CRC(a854b5de) SHA1(dd038f20ee366d439f09f0c82fd6432101b3781a) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6331.9g",      0x0000, 0x0020, CRC(ca1d9ccd) SHA1(27124759a06497c1bc1a64b6d3faa6ba924a8447) )
ROM_END

/*
Monster Zero

CPU: Z80 (x3)
Sound: AY-3-8910 (x2)
Other: 8255 (x3)
RAM: 2114 (x2), 2114 (x2), TMM2016P, TMM314A (x4), MPB8216 (x2), MPB8216 (x2), 2114 (x2), TMM314A (x2), D2125A (x5)
PAL: 16R8C (protected x2)
PROM: 82S123
X1: 1431818
X2: 16000
*/

ROM_START( monsterz )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b-1e.a1",      0x0000, 0x1000, CRC(97886542) SHA1(01f4f9bd55f9eae28162cbb22a26f7cda22cd3f3) )
	ROM_LOAD( "b-2e.c1",      0x1000, 0x1000, CRC(184ffcb4) SHA1(829d6ca13773aba7c3a81e122171befbe3666110) )
	ROM_LOAD( "b-3e.d1",      0x2000, 0x1000, CRC(b7b10ac7) SHA1(51d544d4db456df756a95d7f1853fffed9259647) )
	ROM_LOAD( "b-4e.e1",      0x3000, 0x1000, CRC(fb02c736) SHA1(24466116dd07b856b1afff62b8312c67ff466b95) )
	ROM_LOAD( "b-5e.g1",      0xc000, 0x1000, CRC(b2788ab9) SHA1(eb1a6b41f4c7a243481bfccf2b068ce1bc292366) )
	ROM_LOAD( "b-6e.h1",      0xd000, 0x1000, CRC(77d7aa8d) SHA1(62aaf582ba55f7b21f6cf13b4fb6c2c54bb729f5) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a-1e.k1",      0x0000, 0x1000, CRC(b88ba44e) SHA1(85c141fb411d541b1e20412f5fefd18395f635ae) )
	ROM_LOAD( "a-2.k2",       0x1000, 0x1000, CRC(8913c94e) SHA1(6c4fe065217a234d45761f8ad4d2c4e7078a0abd) )
	ROM_LOAD( "a-3e.k3",      0x2000, 0x1000, CRC(a8fa5095) SHA1(5cabe5497a79a0c43e78a84ae87c824af60a2a3f) )
	ROM_LOAD( "a-4.k4",       0x3000, 0x1000, CRC(93f81317) SHA1(167708be94cb9a47290067a20bc5ff6f018b93b6) )

	ROM_REGION( 0x10000, "audio2", 0 )
	ROM_LOAD( "a-5e.k5",      0x0000, 0x1000, CRC(b5bcdb4e) SHA1(db0965e5636e0f4e9cd4f4a7d808c413ecf733db) )
	ROM_LOAD( "a-6.k6",       0x1000, 0x1000, CRC(24832b2e) SHA1(2a67888e86ce1a3182303e841513ba2a07977359) )
	ROM_LOAD( "a-7e.k7",      0x2000, 0x1000, CRC(20ebea81) SHA1(473c688365b256d8593663ff95768f4a5bb1289d) )
	// 0x3000 empty ?
	ROM_LOAD( "a-8.k8",       0x4000, 0x1000, CRC(b833a15b) SHA1(0d21aaa0ca5ccba89118b205a6b3b36b15663c47) )
	ROM_LOAD( "a-9.k9",       0x5000, 0x1000, CRC(cbd76ec2) SHA1(9434350ee93ca71efe78018b69913386353306ff) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "b-7e.a5",      0x0000, 0x1000, CRC(ddd4158d) SHA1(9701e2d8a0226455dfbed650e58bb4be05918fe8) )
	ROM_LOAD( "b-8e.c5",      0x1000, 0x1000, CRC(b1331b4c) SHA1(fa1af406ecd6919b4846aea68d3edb70106f9273) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "prom.g9",      0x0000, 0x0020, CRC(b7ea00d7) SHA1(f658c6ac8123ae1e6b68ae513cc02c4d9d2b4e47) )
ROM_END


ROM_START( scobra )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr1265.2c",   0x0000, 0x1000, CRC(a0744b3f) SHA1(8949298a04f8ba8a82d5d84a7b012a0e7cff11df) )
	ROM_LOAD( "2e",           0x1000, 0x1000, CRC(8e7245cd) SHA1(281504ff364c3ddbf901c92729b139afd93b9785) )
	ROM_LOAD( "epr1267.2f",   0x2000, 0x1000, CRC(47a4e6fb) SHA1(01775ad11dc23469649539ee8fb8a5800df031c6) )
	ROM_LOAD( "2h",           0x3000, 0x1000, CRC(7244f21c) SHA1(f5fff565ed3f6c5f277a4db53c9f569813fcec1d) )
	ROM_LOAD( "epr1269.2j",   0x4000, 0x1000, CRC(e1f8a801) SHA1(2add8270352d6596052d3ff22c891ceccaa92071) )
	ROM_LOAD( "2l",           0x5000, 0x1000, CRC(d52affde) SHA1(5681771ed51d504bdcc2999fcbf926a30b137828) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "5c",           0x0000, 0x0800, CRC(d4346959) SHA1(5eab4505beb69a5bdd88b23db60e1193371250cf) )
	ROM_LOAD( "5d",           0x0800, 0x0800, CRC(cc025d95) SHA1(2b0784c4d05c466e0b7648f16e14f34393d792c3) )
	ROM_LOAD( "5e",           0x1000, 0x0800, CRC(1628c53f) SHA1(ec79a73e4a2d7373454b227dd7eff255f1cc60cc) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "epr1274.5h",   0x0000, 0x0800, CRC(64d113b4) SHA1(7b439bb74d5ecc792e0ca8964bcca8c6b7a51262) )
	ROM_LOAD( "epr1273.5f",   0x0800, 0x0800, CRC(a96316d3) SHA1(9de0e94932e91dc34aea7c81880bde6a486d103b) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.6e",    0x0000, 0x0020, CRC(9b87f90d) SHA1(d11ac5e4a6057301ea2a9cbb404c2b978eb4c1dc) )
ROM_END

ROM_START( scobrase )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr1265.2c",   0x0000, 0x1000, CRC(a0744b3f) SHA1(8949298a04f8ba8a82d5d84a7b012a0e7cff11df) )
	ROM_LOAD( "epr1266.2e",   0x1000, 0x1000, CRC(65306279) SHA1(f6e10d57c9b88e3fcd7333f76708e190a97b4faa) )
	ROM_LOAD( "epr1267.2f",   0x2000, 0x1000, CRC(47a4e6fb) SHA1(01775ad11dc23469649539ee8fb8a5800df031c6) )
	ROM_LOAD( "epr1268.2h",   0x3000, 0x1000, CRC(53eecaf2) SHA1(08ca34097f63af8ab69b1d836a12a8bd4d42e4a2) )
	ROM_LOAD( "epr1269.2j",   0x4000, 0x1000, CRC(e1f8a801) SHA1(2add8270352d6596052d3ff22c891ceccaa92071) )
	ROM_LOAD( "epr1270.2l",   0x5000, 0x1000, CRC(f7709710) SHA1(dff9ae72ba00a98d4f5acdd6d506e3d7add6b2c6) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "epr1275.5c",   0x0000, 0x0800, CRC(deeb0dd3) SHA1(b815a586f05361b75078d58f1fddfdb36f9d8fae) )
	ROM_LOAD( "epr1276.5d",   0x0800, 0x0800, CRC(872c1a74) SHA1(20f05bf398ad2690f5ba4e4158ad62aeec226413) )
	ROM_LOAD( "epr1277.5e",   0x1000, 0x0800, CRC(ccd7a110) SHA1(5a247e360530be0f94c90fcc7d0ce628d460449f) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "epr1274.5h",   0x0000, 0x0800, CRC(64d113b4) SHA1(7b439bb74d5ecc792e0ca8964bcca8c6b7a51262) )
	ROM_LOAD( "epr1273.5f",   0x0800, 0x0800, CRC(a96316d3) SHA1(9de0e94932e91dc34aea7c81880bde6a486d103b) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "pr1278.6e",    0x0000, 0x0020, CRC(fd35c561) SHA1(590f60beb443dd689c890c37cc100e0b936bf8c9) )
ROM_END

ROM_START( scobras )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "scobra2c.bin", 0x0000, 0x1000, CRC(e15ade38) SHA1(21cf26d1781d133fa336d275d8a61d3f95d10b77) )
	ROM_LOAD( "scobra2e.bin", 0x1000, 0x1000, CRC(a270e44d) SHA1(8b7307af458b9cd3c45bb72b35e682d6d109ed01) )
	ROM_LOAD( "scobra2f.bin", 0x2000, 0x1000, CRC(bdd70346) SHA1(bda0dc5777233a86a3a0aceb6eded45145057ba8) )
	ROM_LOAD( "scobra2h.bin", 0x3000, 0x1000, CRC(dca5ec31) SHA1(50073d44ccef76a3c36c73a6ed4479127f2c98ee) )
	ROM_LOAD( "scobra2j.bin", 0x4000, 0x1000, CRC(0d8f6b6e) SHA1(0ca0096cd55cdb87d14cb7f4c7c7b853ec1661c7) )
	ROM_LOAD( "scobra2l.bin", 0x5000, 0x1000, CRC(6f80f3a9) SHA1(817d212454c5eb16c5d7471d2ccefc4f8708d57f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "epr1275.5c",   0x0000, 0x0800, CRC(deeb0dd3) SHA1(b815a586f05361b75078d58f1fddfdb36f9d8fae) )
	ROM_LOAD( "epr1276.5d",   0x0800, 0x0800, CRC(872c1a74) SHA1(20f05bf398ad2690f5ba4e4158ad62aeec226413) )
	ROM_LOAD( "epr1277.5e",   0x1000, 0x0800, CRC(ccd7a110) SHA1(5a247e360530be0f94c90fcc7d0ce628d460449f) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "epr1274.5h",   0x0000, 0x0800, CRC(64d113b4) SHA1(7b439bb74d5ecc792e0ca8964bcca8c6b7a51262) )
	ROM_LOAD( "epr1273.5f",   0x0800, 0x0800, CRC(a96316d3) SHA1(9de0e94932e91dc34aea7c81880bde6a486d103b) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.6e",    0x0000, 0x0020, CRC(9b87f90d) SHA1(d11ac5e4a6057301ea2a9cbb404c2b978eb4c1dc) )
ROM_END

ROM_START( scobrab )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vid_2c.bin",   0x0000, 0x0800, CRC(aeddf391) SHA1(87632469e943cfe38a9676de5e8ed839a63c5da2) )
	ROM_LOAD( "vid_2e.bin",   0x0800, 0x0800, CRC(72b57eb7) SHA1(978d0acbfccb7c1edddb073ad9417d4cbd9b7e63) )
	ROM_LOAD( "scobra2e.bin", 0x1000, 0x1000, CRC(a270e44d) SHA1(8b7307af458b9cd3c45bb72b35e682d6d109ed01) )
	ROM_LOAD( "scobra2f.bin", 0x2000, 0x1000, CRC(bdd70346) SHA1(bda0dc5777233a86a3a0aceb6eded45145057ba8) )
	ROM_LOAD( "scobra2h.bin", 0x3000, 0x1000, CRC(dca5ec31) SHA1(50073d44ccef76a3c36c73a6ed4479127f2c98ee) )
	ROM_LOAD( "scobra2j.bin", 0x4000, 0x1000, CRC(0d8f6b6e) SHA1(0ca0096cd55cdb87d14cb7f4c7c7b853ec1661c7) )
	ROM_LOAD( "scobra2l.bin", 0x5000, 0x1000, CRC(6f80f3a9) SHA1(817d212454c5eb16c5d7471d2ccefc4f8708d57f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "epr1275.5c",   0x0000, 0x0800, CRC(deeb0dd3) SHA1(b815a586f05361b75078d58f1fddfdb36f9d8fae) ) /* snd_5c.bin */
	ROM_LOAD( "epr1276.5d",   0x0800, 0x0800, CRC(872c1a74) SHA1(20f05bf398ad2690f5ba4e4158ad62aeec226413) ) /* snd_5d.bin */
	ROM_LOAD( "epr1277.5e",   0x1000, 0x0800, CRC(ccd7a110) SHA1(5a247e360530be0f94c90fcc7d0ce628d460449f) ) /* snd_5e.bin */

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "epr1274.5h",   0x0000, 0x0800, CRC(64d113b4) SHA1(7b439bb74d5ecc792e0ca8964bcca8c6b7a51262) )
	ROM_LOAD( "epr1273.5f",   0x0800, 0x0800, CRC(a96316d3) SHA1(9de0e94932e91dc34aea7c81880bde6a486d103b) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.6e",    0x0000, 0x0020, CRC(9b87f90d) SHA1(d11ac5e4a6057301ea2a9cbb404c2b978eb4c1dc) )
ROM_END

ROM_START( scobrae ) // main program is identical to the scobras set once decrypted
	ROM_REGION( 0x10000, "maincpu", 0 ) // all roms have STERN labels
	ROM_LOAD( "super cobra ra1 2c 1981.2c",   0x0000, 0x1000, CRC(ba9d4152) SHA1(f1792c0049804ac956ab7f95f699559fca4df960) )
	ROM_LOAD( "super cobra ra1 2e 1981.2e",   0x1000, 0x1000, CRC(f9b77b27) SHA1(7974761456aaabcf016158ee5f5c32c89e43c748) )
	ROM_LOAD( "super cobra ra1 2f 1981.2f",   0x2000, 0x1000, CRC(e6109c2c) SHA1(1749ac277b1af45b1f6722d2ddaf46be043b2b25) )
	ROM_LOAD( "super cobra ra1 2h 1981.2h",   0x3000, 0x1000, CRC(8762735b) SHA1(07dd9b390d44fec9f83c88abf28d167c1710dcc9) )
	ROM_LOAD( "super cobra ra1 2j 1981.2j",   0x4000, 0x1000, CRC(5648f404) SHA1(5cfbada816fd614508c7cd41196a350176c5882d) )
	ROM_LOAD( "super cobra ra1 2l 1981.2l",   0x5000, 0x1000, CRC(34476cc3) SHA1(b8b1c9572e0c5e25f3d2a33d5a0ce40de007b478) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "super cobra ra1 5f 1981.5f",   0x0000, 0x0800, CRC(64d113b4) SHA1(7b439bb74d5ecc792e0ca8964bcca8c6b7a51262) )
	ROM_LOAD( "super cobra ra1 5h 1981.5h",   0x0800, 0x0800, CRC(a96316d3) SHA1(9de0e94932e91dc34aea7c81880bde6a486d103b) )

	// roms below were missing, so not verified for this set but likely the same because the main program is.
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "5c",   0x0000, 0x0800, BAD_DUMP CRC(deeb0dd3) SHA1(b815a586f05361b75078d58f1fddfdb36f9d8fae) )
	ROM_LOAD( "5d",   0x0800, 0x0800, BAD_DUMP CRC(872c1a74) SHA1(20f05bf398ad2690f5ba4e4158ad62aeec226413) )
	ROM_LOAD( "5e",   0x1000, 0x0800, BAD_DUMP CRC(ccd7a110) SHA1(5a247e360530be0f94c90fcc7d0ce628d460449f) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.6e",    0x0000, 0x0020, BAD_DUMP CRC(9b87f90d) SHA1(d11ac5e4a6057301ea2a9cbb404c2b978eb4c1dc) )
ROM_END


ROM_START( suprheli )
	/* this is a bootleg of Super Cobra */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.2c",         0x0000, 0x1000, CRC(b25141d8) SHA1(9df638ad2c482cc7d8b8c8b61c9f8872bfaff4d5) )
	ROM_LOAD( "scobra2e.bin", 0x1000, 0x1000, CRC(a270e44d) SHA1(8b7307af458b9cd3c45bb72b35e682d6d109ed01) ) /* labeled "2" */
	ROM_LOAD( "scobra2f.bin", 0x2000, 0x1000, CRC(bdd70346) SHA1(bda0dc5777233a86a3a0aceb6eded45145057ba8) ) /* labeled "3" */
	ROM_LOAD( "scobra2h.bin", 0x3000, 0x1000, CRC(dca5ec31) SHA1(50073d44ccef76a3c36c73a6ed4479127f2c98ee) ) /* labeled "4" */
	ROM_LOAD( "scobra2j.bin", 0x4000, 0x1000, CRC(0d8f6b6e) SHA1(0ca0096cd55cdb87d14cb7f4c7c7b853ec1661c7) ) /* labeled "5" */
	ROM_LOAD( "6.2l",         0x5000, 0x1000, CRC(10a474d9) SHA1(3ba7ea791ab7b97bb4276550591812134f891708) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "epr1275.5c",   0x0000, 0x0800, CRC(deeb0dd3) SHA1(b815a586f05361b75078d58f1fddfdb36f9d8fae) ) /* all the rom stickers are reflective and have white printing, except this one has green and was labeled "10" and is at position 11D */
	ROM_LOAD( "epr1276.5d",   0x0800, 0x0800, CRC(872c1a74) SHA1(20f05bf398ad2690f5ba4e4158ad62aeec226413) ) /* also labeled "10" but in white and is at position 10D */
	ROM_LOAD( "9.9d",         0x1000, 0x0800, CRC(2b69b8f3) SHA1(89ed25e7295eff63b53046b78fcb7f6e78796873) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "epr1274.5h",   0x0000, 0x0800, CRC(64d113b4) SHA1(7b439bb74d5ecc792e0ca8964bcca8c6b7a51262) ) /* labeled "7" and is at position 5F, not H */
	ROM_LOAD( "epr1273.5f",   0x0800, 0x0800, CRC(a96316d3) SHA1(9de0e94932e91dc34aea7c81880bde6a486d103b) ) /* labeled "8" and is at position 5H, not F */

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "k.6e",         0x0000, 0x0020, CRC(fd35c561) SHA1(590f60beb443dd689c890c37cc100e0b936bf8c9) ) /* this dump matches the prom from scobrase, it was labeled "K" with a marker here at 6E */
ROM_END


ROM_START( moonwar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mw2.2c",       0x0000, 0x1000, CRC(7c11b4d9) SHA1(a27bdff6ce728647ec811df843ac235c32c293d6) )
	ROM_LOAD( "mw2.2e",       0x1000, 0x1000, CRC(1b6362be) SHA1(2fbd95869146adcc0c8be1df653251fda8849e8e) )
	ROM_LOAD( "mw2.2f",       0x2000, 0x1000, CRC(4fd8ba4b) SHA1(3da784267a96d05f66b00626a22cb3f06211d202) )
	ROM_LOAD( "mw2.2h",       0x3000, 0x1000, CRC(56879f0d) SHA1(d1e9932863aebc5761e71fca8d24f3c400e1250d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "mw2.5c",       0x0000, 0x0800, CRC(c26231eb) SHA1(5b19edfaefe1a535059311d067ea53405879d627) )
	ROM_LOAD( "mw2.5d",       0x0800, 0x0800, CRC(bb48a646) SHA1(cf51202d16b03bbed12ff24501be68683f28c992) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "mw2.5f",       0x0000, 0x0800, CRC(c5fa1aa0) SHA1(6c6b5b2ce5de278ff436d3e7252ece5b086cc41d) )
	ROM_LOAD( "mw2.5h",       0x0800, 0x0800, CRC(a6ccc652) SHA1(286b3dc1f3a7da3ac66664e774b441ef075745f1) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mw2.clr",      0x0000, 0x0020, CRC(99614c6c) SHA1(f068985f3c5e0cd88551a02c32f9baeabfd50241) )
ROM_END

ROM_START( moonwara )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2c",           0x0000, 0x1000, CRC(bc20b734) SHA1(c6fe550987d0052979aad43c67aa1b9248049669) )
	ROM_LOAD( "2e",           0x1000, 0x1000, CRC(db6ffec2) SHA1(0fcd55b1e415e2e7041d10778052a235251f85fe) )
	ROM_LOAD( "2f",           0x2000, 0x1000, CRC(378931b8) SHA1(663f1eea9b0e8dc38de818df66c5211dac41c33b) )
	ROM_LOAD( "2h",           0x3000, 0x1000, CRC(031dbc2c) SHA1(5f2ca8b8763398bf161ee0c2c748a12d36cb40ec) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "mw2.5c",       0x0000, 0x0800, CRC(c26231eb) SHA1(5b19edfaefe1a535059311d067ea53405879d627) )
	ROM_LOAD( "mw2.5d",       0x0800, 0x0800, CRC(bb48a646) SHA1(cf51202d16b03bbed12ff24501be68683f28c992) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "mw2.5f",       0x0000, 0x0800, CRC(c5fa1aa0) SHA1(6c6b5b2ce5de278ff436d3e7252ece5b086cc41d) )
	ROM_LOAD( "mw2.5h",       0x0800, 0x0800, CRC(a6ccc652) SHA1(286b3dc1f3a7da3ac66664e774b441ef075745f1) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "moonwara.clr", 0x0000, 0x0020, CRC(f58d4f58) SHA1(12a80d1edf3c80dafa0e1e3622d2a03224b62f14) )    /* olive, instead of white */
ROM_END


ROM_START( armorcar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cpu.2c",       0x0000, 0x1000, CRC(0d7bfdfb) SHA1(47791d4fc298c25d29584dfcddcd92618e3750c5) )
	ROM_LOAD( "cpu.2e",       0x1000, 0x1000, CRC(76463213) SHA1(86dbbed25325cc8855c1eb39bbb11b0473b7f4b5) )
	ROM_LOAD( "cpu.2f",       0x2000, 0x1000, CRC(2cc6d5f0) SHA1(94abb33760aed206f0f90f035fe2977c1f2e26cf) )
	ROM_LOAD( "cpu.2h",       0x3000, 0x1000, CRC(61278dbb) SHA1(e12cd6c499af75f77e549499093fe6d2e8eddb1d) )
	ROM_LOAD( "cpu.2j",       0x4000, 0x1000, CRC(fb158d8c) SHA1(efa70e92c56678d4a404a96c72cfee317b15648c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sound.5c",     0x0000, 0x0800, CRC(54ee7753) SHA1(3ebfa2cadf33acb4d24aa50cfa4713355cc780a3) )
	ROM_LOAD( "sound.5d",     0x0800, 0x0800, CRC(5218fec0) SHA1(c8f84f1e6aafc544e5acf48b245e8b1edb63211e) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "cpu.5f",       0x0000, 0x0800, CRC(8a3da4d1) SHA1(4d2ef48aeb9099fdd145e11e2485e0bf8d87290d) )
	ROM_LOAD( "cpu.5h",       0x0800, 0x0800, CRC(85bdb113) SHA1(f62da0ea0c29feb10d8d1ce8de28fd750a53b40a) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.6e",    0x0000, 0x0020, CRC(9b87f90d) SHA1(d11ac5e4a6057301ea2a9cbb404c2b978eb4c1dc) )
ROM_END

ROM_START( armorcar2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2c",           0x0000, 0x1000, CRC(e393bd2f) SHA1(6a5079d1f5d724e5f643cbc5352fc43d0b652e05) )
	ROM_LOAD( "2e",           0x1000, 0x1000, CRC(b7d443af) SHA1(1ce41e11a7fcfd039fbff03c4382ae29b601ed50) )
	ROM_LOAD( "2g",           0x2000, 0x1000, CRC(e67380a4) SHA1(a9a87e769d1ef223ae26241e9211c97b3d469656) )
	ROM_LOAD( "2h",           0x3000, 0x1000, CRC(72af7b37) SHA1(c9cd0a0a3e34fc7b12822f75eb511f0850703f55) )
	ROM_LOAD( "2j",           0x4000, 0x1000, CRC(e6b0dd7f) SHA1(98292fea03bff028ba924a49f0bfa49377018860) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sound.5c",     0x0000, 0x0800, CRC(54ee7753) SHA1(3ebfa2cadf33acb4d24aa50cfa4713355cc780a3) )
	ROM_LOAD( "sound.5d",     0x0800, 0x0800, CRC(5218fec0) SHA1(c8f84f1e6aafc544e5acf48b245e8b1edb63211e) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "cpu.5f",       0x0000, 0x0800, CRC(8a3da4d1) SHA1(4d2ef48aeb9099fdd145e11e2485e0bf8d87290d) )
	ROM_LOAD( "cpu.5h",       0x0800, 0x0800, CRC(85bdb113) SHA1(f62da0ea0c29feb10d8d1ce8de28fd750a53b40a) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.6e",    0x0000, 0x0020, CRC(9b87f90d) SHA1(d11ac5e4a6057301ea2a9cbb404c2b978eb4c1dc) )
ROM_END


ROM_START( tazmania )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2c.cpu",       0x0000, 0x1000, CRC(932c5a06) SHA1(f90040a12f579a05cb91eacbe49dae9b2c725592) )
	ROM_LOAD( "2e.cpu",       0x1000, 0x1000, CRC(ef17ce65) SHA1(e1372886a4a2ae58278772f49b4f3be35e0b65d1) )
	ROM_LOAD( "2f.cpu",       0x2000, 0x1000, CRC(43c7c39d) SHA1(3cfe97009e3c9236b118fa1beadc50f41584bd7e) )
	ROM_LOAD( "2h.cpu",       0x3000, 0x1000, CRC(be829694) SHA1(3885c95ae1704e7a472139740b87fc8dd9610e07) )
	ROM_LOAD( "2j.cpu",       0x4000, 0x1000, CRC(6e197271) SHA1(231141a95e4dcb54d8bbee346825702e52824c42) )
	ROM_LOAD( "2k.cpu",       0x5000, 0x1000, CRC(a1eb453b) SHA1(50ddfd1dd8cc8c2cde97e52d4ef90e6d10e27a53) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "rom0.snd",     0x0000, 0x0800, CRC(b8d741f1) SHA1(a1bb8a1e0d6b34111f05c539c8e92fffacf5aa5c) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5f.cpu",       0x0000, 0x0800, CRC(2c5b612b) SHA1(32e3a41a9a4a8b1285b6a195213ff0d98012360a) )
	ROM_LOAD( "5h.cpu",       0x0800, 0x0800, CRC(3f5ff3ac) SHA1(bc70eef54a45b52c14e35464e5f06b5eec554eb6) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "colr6f.cpu",   0x0000, 0x0020, CRC(fce333c7) SHA1(f63a214dc47c5e7c80db000b0b6a261ca8da6629) )
ROM_END


ROM_START( anteater )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ra1-2c",       0x0000, 0x1000, CRC(58bc9393) SHA1(7122782a69ef0d2196ec16833f229b6286802668) )
	ROM_LOAD( "ra1-2e",       0x1000, 0x1000, CRC(574fc6f6) SHA1(a1a213d215fe8502edf22383c3a6fb7c9b279d94) )
	ROM_LOAD( "ra1-2f",       0x2000, 0x1000, CRC(2f7c1fe5) SHA1(4cea7e66a85766a9cf9846bb5bc1ca4e6ee1f4e2) )
	ROM_LOAD( "ra1-2h",       0x3000, 0x1000, CRC(ae8a5da3) SHA1(1893d8293b25431d080b89f5b0874440d14e8d17) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ra4-5c",       0x0000, 0x0800, CRC(87300b4f) SHA1(b81b685ac1d353ff1cd40b876a7478b87b85e7a9) )
	ROM_LOAD( "ra4-5d",       0x0800, 0x0800, CRC(af4e5ffe) SHA1(62717a233cf9f58267af4a9e1c80479b373ab317) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "ra6-5f",       0x0000, 0x0800, CRC(4c3f8a08) SHA1(3152eef64903be1a82f09764821a3654f316197d) )
	ROM_LOAD( "ra6-5h",       0x0800, 0x0800, CRC(b30c7c9f) SHA1(d4ae040d1fd7e5a5d08c2f6968735c551119c207) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "colr6f.cpu",   0x0000, 0x0020, CRC(fce333c7) SHA1(f63a214dc47c5e7c80db000b0b6a261ca8da6629) )
ROM_END


ROM_START( anteateruk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ant1.bin",     0x0000, 0x0400, CRC(69debc90) SHA1(2ad4c86a1cbaf86d0b76bb07b885f61bc6604009) )
	ROM_CONTINUE(             0x4600, 0x0a00 )
	ROM_CONTINUE(             0x6400, 0x1200 )
	ROM_LOAD( "ant2.bin",     0x7600, 0x0500, CRC(ab352805) SHA1(858928f2b57c324a7942c13e0e6a7717a36f6ffc) )
	ROM_CONTINUE(             0x8300, 0x1600 )
	ROM_CONTINUE(             0xa300, 0x0500 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ra4-5c",       0x0000, 0x0800, CRC(87300b4f) SHA1(b81b685ac1d353ff1cd40b876a7478b87b85e7a9) )
	ROM_LOAD( "ra4-5d",       0x0800, 0x0800, CRC(af4e5ffe) SHA1(62717a233cf9f58267af4a9e1c80479b373ab317) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "gfx_1.bin",    0x0000, 0x0800, CRC(1e2824b1) SHA1(9527937db618505181f4d5a22bc532977a767232) )
	ROM_LOAD( "gfx_2.bin",    0x0800, 0x0800, CRC(784319b3) SHA1(0c3612a428d0906b07b35782cc0f84fda13aab73) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "colr6f.cpu",   0x0000, 0x0020, CRC(fce333c7) SHA1(f63a214dc47c5e7c80db000b0b6a261ca8da6629) )
ROM_END


ROM_START( anteaterg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "prg_2.bin",    0x0000, 0x0400, CRC(2ba793a8) SHA1(a97c96dcd55804d3b41856ece6477ec1c1e45892) )
	ROM_CONTINUE(             0x4600, 0x0a00 )
	ROM_CONTINUE(             0x6400, 0x1200 )
	ROM_LOAD( "prg_1.bin",    0x7600, 0x0500, CRC(7a798af5) SHA1(b4c8672c92b207a7a334dd3b78e57537b7d99b71) )
	ROM_CONTINUE(             0x8300, 0x1600 )
	ROM_CONTINUE(             0xa300, 0x0500 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ra4-5c",       0x0000, 0x0800, CRC(87300b4f) SHA1(b81b685ac1d353ff1cd40b876a7478b87b85e7a9) )
	ROM_LOAD( "ra4-5d",       0x0800, 0x0800, CRC(af4e5ffe) SHA1(62717a233cf9f58267af4a9e1c80479b373ab317) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "gfx_1.bin",    0x0000, 0x0800, CRC(1e2824b1) SHA1(9527937db618505181f4d5a22bc532977a767232) )
	ROM_LOAD( "gfx_2.bin",    0x0800, 0x0800, CRC(784319b3) SHA1(0c3612a428d0906b07b35782cc0f84fda13aab73) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "colr6f.cpu",   0x0000, 0x0020, CRC(fce333c7) SHA1(f63a214dc47c5e7c80db000b0b6a261ca8da6629) )
ROM_END


ROM_START( calipso )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "calipso.2c",   0x0000, 0x1000, CRC(0fcb703c) SHA1(2bb096f114911973afdf3088c860c9566df06f60) )
	ROM_LOAD( "calipso.2e",   0x1000, 0x1000, CRC(c6622f14) SHA1(475164aed703a97275ff285ecaec9d8fd4fe723b) )
	ROM_LOAD( "calipso.2f",   0x2000, 0x1000, CRC(7bacbaba) SHA1(d321d6d09c689123eb1e5d758d95ccecec225252) )
	ROM_LOAD( "calipso.2h",   0x3000, 0x1000, CRC(a3a8111b) SHA1(3d9500c676563ebfc27aebb07716e6a966f00c35) )
	ROM_LOAD( "calipso.2j",   0x4000, 0x1000, CRC(fcbd7b9e) SHA1(5cc1edcc8b9867bb7849c8d97d1096bb6464f562) )
	ROM_LOAD( "calipso.2l",   0x5000, 0x1000, CRC(f7630cab) SHA1(482ee91cccd8a7c5768a1d6a9772d797769fe2dc) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound code */
	ROM_LOAD( "calipso.5c",   0x0000, 0x0800, CRC(9cbc65ab) SHA1(b4ce04d18f9536c0ddd2f9c15edda75570e750e5) )
	ROM_LOAD( "calipso.5d",   0x0800, 0x0800, CRC(a225ee3b) SHA1(dba111f89851c69fb6fce16219cb2b0cb3294c15) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "calipso.5f",   0x0000, 0x2000, CRC(fd4252e9) SHA1(881b988cdc9b7913f577573f8a15af7a7c7cc67f) )
	ROM_LOAD( "calipso.5h",   0x2000, 0x2000, CRC(1663a73a) SHA1(95b6ed25b656afdfb70fac35efa2e005185e4343) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "calipso.clr",  0x0000, 0x0020, CRC(01165832) SHA1(bfef0459492dbd5febf3030916b6438eb6be71de) )
ROM_END


/*
    Lost Tomb

    CPU/Sound Board: A969
    Video Board:     A2048
*/

ROM_START( losttomb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2c",           0x0000, 0x1000, CRC(d6176d2c) SHA1(bc2bf63ee8d3e376f155d218704ceb5adcdf8e54) )
	ROM_LOAD( "2e",           0x1000, 0x1000, CRC(a5f55f4a) SHA1(3609fc2b15b6856e81738bbd370250735dba694d) )
	ROM_LOAD( "2f",           0x2000, 0x1000, CRC(0169fa3c) SHA1(2c06b1deca6c80d067032bfc2386da6ab0111e5f) )
	ROM_LOAD( "2h-easy",      0x3000, 0x1000, CRC(054481b6) SHA1(b0f5d19af0336883e4d9813e58a75c176a63a987) )
	ROM_LOAD( "2j",           0x4000, 0x1000, CRC(249ee040) SHA1(7297039e95e67220fa3e75fc50635f4df4c46a86) )
	ROM_LOAD( "2l",           0x5000, 0x1000, CRC(c7d2e608) SHA1(8aabecabd1dcd6833fb581e4571d71a680e6563a) )
	ROM_LOAD( "2m",           0x6000, 0x1000, CRC(bc4bc5b1) SHA1(95ffa72e57d1da10ddeda4d9333c9e0a2fb33e82) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "5c",           0x0000, 0x0800, CRC(b899be2a) SHA1(9b343a682531255104db61177a43ad933c3af34e) )
	ROM_LOAD( "5d",           0x0800, 0x0800, CRC(6907af31) SHA1(8496c8db5342129d81381eec196facbca45bca77) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5f",           0x0000, 0x0800, CRC(61f137e7) SHA1(8bff09bc29fa829e21e6b36f7b3f67f19f6bbb26) )
	ROM_LOAD( "5h",           0x0800, 0x0800, CRC(5581de5f) SHA1(763dacb0d2183c159e7f1f04c7ecb1182da18abf) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "ltprom",       0x0000, 0x0020, CRC(1108b816) SHA1(49fdb08f8f31fefa2f3dca3d3455318cb21847a3) )
ROM_END

ROM_START( losttombh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2c",           0x0000, 0x1000, CRC(d6176d2c) SHA1(bc2bf63ee8d3e376f155d218704ceb5adcdf8e54) )
	ROM_LOAD( "2e",           0x1000, 0x1000, CRC(a5f55f4a) SHA1(3609fc2b15b6856e81738bbd370250735dba694d) )
	ROM_LOAD( "2f",           0x2000, 0x1000, CRC(0169fa3c) SHA1(2c06b1deca6c80d067032bfc2386da6ab0111e5f) )
	ROM_LOAD( "lthard",       0x3000, 0x1000, CRC(e32cbf0e) SHA1(c4a63e01fad7bd450def5c4412690d4bb8d12691) )
	ROM_LOAD( "2j",           0x4000, 0x1000, CRC(249ee040) SHA1(7297039e95e67220fa3e75fc50635f4df4c46a86) )
	ROM_LOAD( "2l",           0x5000, 0x1000, CRC(c7d2e608) SHA1(8aabecabd1dcd6833fb581e4571d71a680e6563a) )
	ROM_LOAD( "2m",           0x6000, 0x1000, CRC(bc4bc5b1) SHA1(95ffa72e57d1da10ddeda4d9333c9e0a2fb33e82) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "5c",           0x0000, 0x0800, CRC(b899be2a) SHA1(9b343a682531255104db61177a43ad933c3af34e) )
	ROM_LOAD( "5d",           0x0800, 0x0800, CRC(6907af31) SHA1(8496c8db5342129d81381eec196facbca45bca77) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5f",           0x0000, 0x0800, CRC(61f137e7) SHA1(8bff09bc29fa829e21e6b36f7b3f67f19f6bbb26) )
	ROM_LOAD( "5h",           0x0800, 0x0800, CRC(5581de5f) SHA1(763dacb0d2183c159e7f1f04c7ecb1182da18abf) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "ltprom",       0x0000, 0x0020, CRC(1108b816) SHA1(49fdb08f8f31fefa2f3dca3d3455318cb21847a3) )
ROM_END


ROM_START( spdcoin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "spdcoin.2c",   0x0000, 0x1000, CRC(65cf1e49) SHA1(a4570f0d7868fcdd378de3fd9e5346780afcf427) )
	ROM_LOAD( "spdcoin.2e",   0x1000, 0x1000, CRC(1ee59232) SHA1(b58c1de69d33cf80432012b9b6d8b1e3d8b00662) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "spdcoin.5c",   0x0000, 0x0800, CRC(b4cf64b7) SHA1(a95d94be2e374b78b4ba49b6931f0c214ff9d033) )
	ROM_LOAD( "spdcoin.5d",   0x0800, 0x0800, CRC(92304df0) SHA1(01471bf7cbea0090933a253b1b46f80c8f240df5) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "spdcoin.5f",   0x0000, 0x0800, CRC(dd5f1dbc) SHA1(e7c40972a7530cac19ce04de3272244959d337ab) )
	ROM_LOAD( "spdcoin.5h",   0x0800, 0x0800, CRC(ab1fe81b) SHA1(98057932cb5faad60d425b547590ab22bfc67ff6) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "spdcoin.clr",  0x0000, 0x0020, CRC(1a2ccc56) SHA1(58bedaa8b3e21e916295603b38529084b6c0099a) )
ROM_END


ROM_START( superbon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2d.cpu",       0x0000, 0x1000, CRC(60c0ba18) SHA1(6ad09e01dd3c86c8d9c465916227c9b00f38e025) )
	ROM_LOAD( "2e.cpu",       0x1000, 0x1000, CRC(ddcf44bf) SHA1(b862622f4aa8af6da568b4f82ef043359ece530f) )
	ROM_LOAD( "2f.cpu",       0x2000, 0x1000, CRC(bb66c2d5) SHA1(cbb7f4279ae48460790cb8abf976b978ae6a1a25) )
	ROM_LOAD( "2h.cpu",       0x3000, 0x1000, CRC(74f4f04d) SHA1(d51c5d2c21453ee0dab60253c3124b6112d1f859) )
	ROM_LOAD( "2j.cpu",       0x4000, 0x1000, CRC(78effb08) SHA1(64f211b34c2f37c25a36200b393f145b39ae67b5) )
	ROM_LOAD( "2l.cpu",       0x5000, 0x1000, CRC(e9dcecbd) SHA1(ec61cec2b66c041872a2ca29cf724a89c73fc9a3) )
	ROM_LOAD( "2m.cpu",       0x6000, 0x1000, CRC(3ed0337e) SHA1(975b93aee851867e335614419aa6db16fbf8063f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "5c",           0x0000, 0x0800, CRC(b899be2a) SHA1(9b343a682531255104db61177a43ad933c3af34e) )
	ROM_LOAD( "5d.snd",       0x0800, 0x0800, CRC(80640a04) SHA1(83f2bafcfa5737441194d3058a76b2582317cfcb) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5f.cpu",       0x0000, 0x0800, CRC(5b9d4686) SHA1(c7814aefaccab9c8a3a0b015447d366cd2e43c3a) )
	ROM_LOAD( "5h.cpu",       0x0800, 0x0800, CRC(58c29927) SHA1(b88515d9c3108d2ad59f30fed5d74877b1636280) )

	/* The conversion instructions do not mention color proms:                   */
	/* http://www.arcadeflyers.com/?page=flyer&db=videodb&id=5353&image=2        */
	/* However, pages may be missing. In addition, it is mentioned that the      */
	/* conversion kit may be used for Scramble, Amidar and Frogger as well.      */
	/* They all have different color proms. We use the prom from Super Cobra     */
	/* for now and mark it as bad dump until we have more information.           */
	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.6e",    0x0000, 0x0020, BAD_DUMP CRC(9b87f90d) SHA1(d11ac5e4a6057301ea2a9cbb404c2b978eb4c1dc) )
ROM_END


/*************************************
 *
 *  Game drivers
 *  Galaxian-derived games
 *
 *************************************/

/* basic galaxian hardware */
GAME( 1979, galaxian,    0,        galaxian,   galaxian,   galaxian_state, galaxian,   ROT90,  "Namco", "Galaxian (Namco set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, galaxiana,   galaxian, galaxian,   superg,     galaxian_state, galaxian,   ROT90,  "Namco", "Galaxian (Namco set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, galaxianm,   galaxian, galaxian,   galaxian,   galaxian_state, galaxian,   ROT90,  "Namco (Midway license)", "Galaxian (Midway set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, galaxianmo,  galaxian, galaxian,   galaxian,   galaxian_state, galaxian,   ROT90,  "Namco (Midway license)", "Galaxian (Midway set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, galaxiant,   galaxian, galaxian,   superg,     galaxian_state, galaxian,   ROT90,  "Namco (Taito license)", "Galaxian (Taito)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, galaxiani,   galaxian, galaxian,   superg,     galaxian_state, galaxian,   ROT90,  "bootleg? (Irem)", "Galaxian (Irem)", MACHINE_SUPPORTS_SAVE ) // more likely bootlegged by Irem, not an official license

/* straight Galaxian ripoffs on basic galaxian hardware */
GAME( 1979, moonaln,     galaxian, galaxian,   superg,     galaxian_state, galaxian,   ROT90,  "Namco / Nichibutsu (Karateco license?)", "Moon Alien", MACHINE_SUPPORTS_SAVE ) // or bootleg?
GAME( 1979, superg,      galaxian, galaxian,   superg,     galaxian_state, galaxian,   ROT90,  "hack", "Super Galaxians (galaxiana hack)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, galturbo,    galaxian, galaxian,   superg,     galaxian_state, galaxian,   ROT90,  "hack", "Galaxian Turbo (superg hack)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, galapx,      galaxian, galaxian,   superg,     galaxian_state, galaxian,   ROT90,  "hack", "Galaxian Part X (moonaln hack)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, galap1,      galaxian, galaxian,   superg,     galaxian_state, galaxian,   ROT90,  "hack", "Space Invaders Galactica (galaxiana hack)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, galap4,      galaxian, galaxian,   superg,     galaxian_state, galaxian,   ROT90,  "hack (G.G.I)", "Galaxian Part 4 (hack)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, swarm,       galaxian, galaxian,   swarm,      galaxian_state, galaxian,   ROT90,  "bootleg? (Subelectro)", "Swarm (bootleg?)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, astrians,    galaxian, galaxian,   swarm,      galaxian_state, galaxian,   ROT90,  "bootleg (BGV Ltd.)", "Astrians (clone of Swarm)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, zerotime,    galaxian, galaxian,   zerotime,   galaxian_state, galaxian,   ROT90,  "bootleg? (Petaco S.A.)", "Zero Time", MACHINE_SUPPORTS_SAVE )
GAME( 1979, zerotimed,   galaxian, galaxian,   zerotime,   galaxian_state, galaxian,   ROT90,  "bootleg (Datamat)", "Zero Time (Datamat)", MACHINE_SUPPORTS_SAVE ) // a 1994 bootleg of the Petaco bootleg
GAME( 1979, starfght,    galaxian, galaxian,   swarm,      galaxian_state, galaxian,   ROT90,  "bootleg (Jeutel)", "Star Fighter", MACHINE_SUPPORTS_SAVE )
GAME( 1979, galaxbsf,    galaxian, galaxian,   galaxian,   galaxian_state, galaxian,   ROT90,  "bootleg", "Galaxian (bootleg, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, galaxianbl,  galaxian, galaxian,   galaxianbl, galaxian_state, galaxian,   ROT90,  "bootleg", "Galaxian (bootleg, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, galaxbsf2,   galaxian, galaxian,   galaxian,   galaxian_state, galaxian,   ROT90,  "bootleg", "Galaxian (bootleg, set 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, supergx,     galaxian, galaxian,   superg,     galaxian_state, galaxian,   ROT90,  "Namco / Nichibutsu", "Super GX", MACHINE_NOT_WORKING | MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 19??, tst_galx,    galaxian, galaxian,   galaxian,   galaxian_state, galaxian,   ROT90,  "<unknown>", "Galaxian Test ROM", MACHINE_SUPPORTS_SAVE )
GAME( 1980, galaxrf,     galaxian, galaxian,   galaxrf,    galaxian_state, galaxian,   ROT90,  "bootleg (Recreativos Franco S.A.)", "Galaxian (Recreativos Franco S.A. Spanish bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, galaxrfgg,   galaxian, galaxian,   galaxrf,    galaxian_state, galaxian,   ROT90,  "bootleg (Recreativos Franco S.A.)", "Galaxian Growing Galaxip / Galaxian Nave Creciente (Recreativos Franco S.A. Spanish bootleg)", MACHINE_SUPPORTS_SAVE )

/* other games on basic galaxian hardware */
GAME( 1981, blkhole,     0,        galaxian,   blkhole,    galaxian_state, galaxian,   ROT90,  "TDS & MINTS", "Black Hole", MACHINE_SUPPORTS_SAVE )
GAME( 1982, orbitron,    0,        galaxian,   orbitron,   galaxian_state, galaxian,   ROT270, "Signatron USA", "Orbitron", MACHINE_SUPPORTS_SAVE )
GAME( 1980, luctoday,    0,        galaxian,   luctoday,   galaxian_state, galaxian,   ROT270, "Sigma", "Lucky Today",MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 19??, chewing,     luctoday, galaxian,   luctoday,   galaxian_state, galaxian,   ROT90,  "<unknown>", "Chewing Gum", MACHINE_SUPPORTS_SAVE )
GAME( 1982, catacomb,    0,        galaxian,   catacomb,   galaxian_state, galaxian,   ROT90,  "MTM Games", "Catacomb", MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 19??, omega,       theend,   galaxian,   omega,      galaxian_state, galaxian,   ROT270, "bootleg?", "Omega", MACHINE_SUPPORTS_SAVE )

/* these games require the coin lockout mechanism to be disabled */
GAME( 1981, warofbug,    0,        galaxian,   warofbug,   galaxian_state, nolock,     ROT90,  "Armenia / Food and Fun Corp", "War of the Bugs or Monsterous Manouvers in a Mushroom Maze", MACHINE_SUPPORTS_SAVE )
GAME( 1981, warofbugu,   warofbug, galaxian,   warofbug,   galaxian_state, nolock,     ROT90,  "Armenia / Super Video Games", "War of the Bugs or Monsterous Manouvers in a Mushroom Maze (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, warofbugg,   warofbug, galaxian,   warofbug,   galaxian_state, nolock,     ROT90,  "Armenia", "War of the Bugs or Monsterous Manouvers in a Mushroom Maze (German)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1981, spactrai,    warofbug, spactrai,   warofbug,   galaxian_state, nolock,     ROT90,  "Armenia / Adar", "Space Train", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1981, redufo,      0,        galaxian,   redufo,     galaxian_state, nolock,     ROT270, "Artic", "Defend the Terra Attack on the Red UFO", MACHINE_SUPPORTS_SAVE ) // is this the original?
GAME( 1981, redufob,     redufo,   galaxian,   redufob,    galaxian_state, nolock,     ROT90,  "bootleg", "Defend the Terra Attack on the Red UFO (bootleg)", MACHINE_SUPPORTS_SAVE ) // rev A?
GAME( 19??, exodus,      redufo,   galaxian,   redufo,     galaxian_state, nolock,     ROT90,  "bootleg? (Subelectro)", "Exodus (bootleg?)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, tdpgal,      0,        galaxian,   tdpgal,     galaxian_state, nolock,     ROT90,  "Design Labs / Thomas Automatics", "Triple Draw Poker", MACHINE_SUPPORTS_SAVE )
GAME( 1979, kamakazi3,   galaxian, galaxian,   superg,     galaxian_state, nolock,     ROT90,  "hack", "Kamakazi III (superg hack)", MACHINE_SUPPORTS_SAVE )

/* different bullet color */
GAME( 1982, azurian,     0,        galaxian,   azurian,    galaxian_state, azurian,    ROT90,  "Rait Electronics Ltd", "Azurian Attack", MACHINE_SUPPORTS_SAVE )

/* extra characters controlled via bank at $6002 */
GAME( 19??, pisces,      0,        galaxian,   pisces,     galaxian_state, pisces,     ROT90,  "Subelectro", "Pisces", MACHINE_SUPPORTS_SAVE )
GAME( 19??, piscesb,     pisces,   galaxian,   piscesb,    galaxian_state, pisces,     ROT90,  "bootleg", "Pisces (bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 19??, omni,        pisces,   galaxian,   piscesb,    galaxian_state, pisces,     ROT90,  "bootleg", "Omni", MACHINE_SUPPORTS_SAVE )
GAME( 1980, uniwars,     0,        galaxian,   superg,     galaxian_state, pisces,     ROT90,  "Irem", "UniWar S", MACHINE_SUPPORTS_SAVE )
GAME( 1980, gteikoku,    uniwars,  galaxian,   superg,     galaxian_state, pisces,     ROT90,  "Irem", "Gingateikoku No Gyakushu", MACHINE_SUPPORTS_SAVE )
GAME( 1980, gteikokb,    uniwars,  galaxian,   gteikokb,   galaxian_state, pisces,     ROT270, "bootleg", "Gingateikoku No Gyakushu (bootleg set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, gteikob2,    uniwars,  galaxian,   gteikob2,   galaxian_state, pisces,     ROT90,  "bootleg", "Gingateikoku No Gyakushu (bootleg set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, spacbatt,    uniwars,  galaxian,   spacbatt,   galaxian_state, pisces,     ROT90,  "bootleg", "Space Battle (bootleg set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, spacbat2,    uniwars,  galaxian,   spacbatt,   galaxian_state, pisces,     ROT90,  "bootleg", "Space Battle (bootleg set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, spacempr,    uniwars,  galaxian,   spacbatt,   galaxian_state, pisces,     ROT90,  "bootleg", "Space Empire (bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, skyraidr,    uniwars,  galaxian,   superg,     galaxian_state, pisces,     ROT90,  "bootleg", "Sky Raider (Uniwars bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, galemp,      uniwars,  galaxian,   superg,     galaxian_state, pisces,     ROT90,  "bootleg (Taito do Brasil)", "Galaxy Empire (bootleg?)", MACHINE_SUPPORTS_SAVE ) // clearly a hack, but was it licensed?
GAME( 1980, asideral,    uniwars,  galaxian,   asideral,   galaxian_state, pisces,     ROT90,  "bootleg (Electrogame S.A.)", "Ataque Sideral (Spanish bootleg of UniWar S)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, pajaroes,    uniwars,  galaxian,   asideral,   galaxian_state, pisces,     ROT90,  "bootleg (PSV S.A.)", "Pajaro del Espacio (Spanish bootleg of UniWar S)", MACHINE_SUPPORTS_SAVE ) // very similar to above

/* Artic Multi-System games - separate tile/sprite ROMs */
GAME( 1980, streakng,    0,        pacmanbl,   streakng,   galaxian_state, galaxian,   ROT90,  "Shoei", "Streaking (set 1)", MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1980, streaknga,   streakng, pacmanbl,   streakng,   galaxian_state, galaxian,   ROT90,  "Shoei", "Streaking (set 2)", MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1981, pacmanbl,    puckman,  pacmanbl,   pacmanbl,   galaxian_state, pacmanbl,   ROT270, "bootleg", "Pac-Man (Galaxian hardware, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, pacmanbla,   puckman,  pacmanbl,   pacmanbl,   galaxian_state, pacmanbl,   ROT270, "bootleg", "Pac-Man (Galaxian hardware, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, ghostmun,    puckman,  pacmanbl,   streakng,   galaxian_state, ghostmun,   ROT90,  "bootleg (Leisure and Allied)", "Ghost Muncher", MACHINE_SUPPORTS_SAVE )
GAME( 1981, phoenxp2,    phoenix,  galaxian,   phoenxp2,   galaxian_state, batman2,    ROT270, "bootleg", "Phoenix Part 2", MACHINE_SUPPORTS_SAVE )
GAME( 1981, batman2,     phoenix,  galaxian,   batman2,    galaxian_state, batman2,    ROT270, "bootleg", "Batman Part 2", MACHINE_SUPPORTS_SAVE ) /* similar to pisces, but with different video banking characteristics */
GAME( 1981, atlantisb,   atlantis, galaxian,   atlantib,   galaxian_state, galaxian,   ROT270, "bootleg", "Battle of Atlantis (bootleg)", MACHINE_SUPPORTS_SAVE ) // I don't know if this should have a starfield...
GAME( 1982, tenspot,     0,        tenspot,    tenspot,    galaxian_state, tenspot,    ROT270, "Thomas Automatics", "Ten Spot", MACHINE_NOT_WORKING ) // work out how menu works

/* separate tile/sprite ROMs, plus INT instead of NMI */
GAME( 1984, devilfsg,    devilfsh, pacmanbl,   devilfsg,   galaxian_state, devilfsg,   ROT270, "Vision / Artic", "Devil Fish (Galaxian hardware, bootleg?)", MACHINE_SUPPORTS_SAVE )

/* sound hardware replaced with AY8910 */
// we're missing the original set by Taito do Brasil, we only have the bootlegs
GAME( 1982, zigzagb,     0,        zigzag,     zigzag,     galaxian_state, zigzag,     ROT90,  "bootleg (LAX)", "Zig Zag (Dig Dug conversion on Galaxian hardware, bootleg set 1)", MACHINE_SUPPORTS_SAVE ) // rewrite of Dig Dug (!) not a clone
GAME( 1982, zigzagb2,    zigzagb,  zigzag,     zigzag,     galaxian_state, zigzag,     ROT90,  "bootleg (LAX)", "Zig Zag (Dig Dug conversion on Galaxian hardware, bootleg set 2)", MACHINE_SUPPORTS_SAVE )

/* multi-game select via external switch */
GAME( 1981, gmgalax,     0,        gmgalax,    gmgalax,    galaxian_state, gmgalax,    ROT90,  "bootleg", "Ghostmuncher Galaxian (bootleg)", MACHINE_SUPPORTS_SAVE )



/*************************************
 *
 *  Game drivers
 *  Moon Cresta-derived games
 *
 *************************************/

/* based on Galaxian, but with altered address map for more ROM */
GAME( 1980, mooncrst,    0,        mooncrst,   mooncrst,   galaxian_state, mooncrst,   ROT90,  "Nichibutsu", "Moon Cresta (Nichibutsu)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, mooncrstuk,  mooncrst, mooncrst,   mooncrst,   galaxian_state, mooncrst,   ROT90,  "Nichibutsu UK", "Moon Cresta (Nichibutsu UK)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, mooncrstuku, mooncrst, mooncrst,   mooncrst,   galaxian_state, mooncrsu,   ROT90,  "Nichibutsu UK", "Moon Cresta (Nichibutsu UK, unencrypted)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, mooncrstu,   mooncrst, mooncrst,   mooncrst,   galaxian_state, mooncrsu,   ROT90,  "Nichibutsu USA", "Moon Cresta (Nichibutsu USA, unencrypted)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, mooncrsto,   mooncrst, mooncrst,   mooncrsa,   galaxian_state, mooncrst,   ROT90,  "Nichibutsu", "Moon Cresta (Nichibutsu, old rev)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, mooncrstg,   mooncrst, mooncrst,   mooncrsg,   galaxian_state, mooncrsu,   ROT90,  "Nichibutsu (Gremlin license)", "Moon Cresta (Gremlin)", MACHINE_SUPPORTS_SAVE )

/* straight Moon Cresta ripoffs on basic mooncrst hardware */
GAME( 1980, mooncrsb,    mooncrst, mooncrst,   mooncrsa,   galaxian_state, mooncrsu,   ROT90,  "bootleg", "Moon Cresta (bootleg set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, mooncrs2,    mooncrst, mooncrst,   mooncrsa,   galaxian_state, mooncrsu,   ROT90,  "bootleg", "Moon Cresta (bootleg set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, mooncrs3,    mooncrst, mooncrst,   mooncrst,   galaxian_state, mooncrsu,   ROT90,  "bootleg (Jeutel)", "Moon Cresta (bootleg set 3)", MACHINE_SUPPORTS_SAVE ) /* Jeutel bootleg, similar to bootleg set 2 */
GAME( 1980, mooncrs4,    mooncrst, mooncrst,   mooncrst,   galaxian_state, mooncrsu,   ROT90,  "bootleg (SG-Florence)", "Moon Crest (Moon Cresta bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, fantazia,    mooncrst, mooncrst,   fantazia,   galaxian_state, mooncrsu,   ROT90,  "bootleg? (Subelectro)", "Fantazia (bootleg?)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, eagle,       mooncrst, mooncrst,   mooncrsa,   galaxian_state, mooncrsu,   ROT90,  "Nichibutsu (Centuri license)", "Eagle (set 1)", MACHINE_SUPPORTS_SAVE ) // or bootleg?
GAME( 1980, eagle2,      mooncrst, mooncrst,   eagle2,     galaxian_state, mooncrsu,   ROT90,  "Nichibutsu (Centuri license)", "Eagle (set 2)", MACHINE_SUPPORTS_SAVE ) // "
GAME( 1980, eagle3,      mooncrst, mooncrst,   mooncrsa,   galaxian_state, mooncrsu,   ROT90,  "Nichibutsu (Centuri license)", "Eagle (set 3)", MACHINE_SUPPORTS_SAVE ) // "
GAME( 1981?,spctbird,    mooncrst, mooncrst,   eagle2,     galaxian_state, mooncrsu,   ROT90,  "bootleg? (Fortrek)", "Space Thunderbird", MACHINE_SUPPORTS_SAVE )
GAME( 1980?,smooncrs,    mooncrst, mooncrst,   smooncrs,   galaxian_state, mooncrsu,   ROT90,  "Nichibutsu (Gremlin license)", "Super Moon Cresta", MACHINE_SUPPORTS_SAVE )
GAME( 1980?,mooncptc,    mooncrst, mooncrst,   mooncptc,   galaxian_state, mooncrsu,   ROT90,  "bootleg (Petaco S.A.)", "Moon Cresta (Petaco S.A. Spanish bootleg)", MACHINE_SUPPORTS_SAVE )
// there may be an alternate version called "Star Crest" according to flyers; is it the same?
GAME( 1980?,sstarcrs,    mooncrst, mooncrst,   mooncrsg,   galaxian_state, mooncrsu,   ROT90,  "Nichibutsu (Taito do Brasil license)", "Super Star Crest", MACHINE_SUPPORTS_SAVE )
GAME( 198?, mooncmw,     mooncrst, mooncrst,   mooncrsa,   galaxian_state, mooncrsu,   ROT90,  "bootleg", "Moon War (Moon Cresta bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 198?, starfgmc,    mooncrst, mooncrst,   mooncrsa,   galaxian_state, mooncrsu,   ROT90,  "bootleg (Samyra Engineering)", "Starfighter (Moon Cresta bootleg)", MACHINE_SUPPORTS_SAVE )
// The boards were marked 'Space Dragon' although this doesn't appear in the games.
GAME( 1980, spcdrag,     mooncrst, mooncrst,   smooncrs,   galaxian_state, mooncrsu,   ROT90,  "bootleg", "Space Dragon (Moon Cresta bootleg, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, spcdraga,    mooncrst, mooncrst,   smooncrs,   galaxian_state, mooncrsu,   ROT90,  "bootleg", "Space Dragon (Moon Cresta bootleg, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, mooncreg,    mooncrst, mooncrst,   mooncreg,   galaxian_state, mooncrsu,   ROT90,  "bootleg (Electrogame S.A.)", "Moon Cresta (Electrogame S.A. Spanish bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, mooncrsl,    mooncrst, mooncrst,   mooncrsl,   galaxian_state, mooncrsu,   ROT90,  "bootleg (Laguna S.A.)", "Cresta Mundo (Laguna S.A. Spanish Moon Cresta bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, stera,       mooncrst, mooncrst,   smooncrs,   galaxian_state, mooncrsu,   ROT90,  "bootleg", "Steraranger (Moon Cresta bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, mooncrgx,    mooncrst, galaxian,   mooncrgx,   galaxian_state, mooncrgx,   ROT270, "bootleg", "Moon Cresta (Galaxian hardware)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, moonqsr,     0,        moonqsr,    moonqsr,    galaxian_state, moonqsr,    ROT90,  "Nichibutsu", "Moon Quasar", MACHINE_SUPPORTS_SAVE )
GAME( 1980, moonal2,     galaxian, mooncrst,   moonal2,    galaxian_state, galaxian,   ROT90,  "Namco / Nichibutsu", "Moon Alien Part 2", MACHINE_SUPPORTS_SAVE )
GAME( 1980, moonal2b,    galaxian, mooncrst,   moonal2,    galaxian_state, galaxian,   ROT90,  "Namco / Nichibutsu", "Moon Alien Part 2 (older version)", MACHINE_SUPPORTS_SAVE )

/* larger romspace, interrupt enable moved */
GAME( 198?, thepitm,     thepit,   mooncrst,   thepitm,    galaxian_state, thepitm,    ROT90,  "bootleg (KZH)", "The Pit (bootleg on Moon Quasar hardware)", MACHINE_SUPPORTS_SAVE ) // on an original MQ-2FJ pcb, even if the memory map appears closer to Moon Cresta

/* other games on basic mooncrst hardware */
GAME( 1982, skybase,     0,        mooncrst,   skybase,    galaxian_state, skybase,    ROT90,  "Omori Electric Co., Ltd.", "Sky Base", MACHINE_SUPPORTS_SAVE )
GAME( 198?, kong,        0,        mooncrst,   kong,       galaxian_state, kong,       ROT90,  "Taito do Brasil", "Kong (Donkey Kong conversion on Galaxian hardware)", MACHINE_SUPPORTS_SAVE | MACHINE_WRONG_COLORS ) // rewrite of Donkey Kong (!) not a clone

/* larger romspace, 2*AY8910, based on Super Star Crest board? */
// there may be an alternate version called "Fantasy" according to flyers; is it the same?
GAME( 198?, fantastc,    0,        fantastc,   fantastc,   galaxian_state, fantastc,   ROT90,  "Taito do Brasil", "Fantastic (Galaga conversion on Galaxian hardware)", MACHINE_SUPPORTS_SAVE ) // rewrite of Galaga (!) not a clone

/* like fantastc, plus larger spriteram, and maybe different bullet hw(?) */
GAME( 198?, timefgtr,    0,        timefgtr,   timefgtr,   galaxian_state, timefgtr,   ROT90,  "Taito do Brasil", "Time Fighter (Time Pilot conversion on Galaxian hardware)", MACHINE_SUPPORTS_SAVE | MACHINE_WRONG_COLORS ) // rewrite of Time Pilot (!) not a clone

/* extra ROMs, protection, and sound hardware replaced with AY8910 */
GAME( 1981, jumpbug,     0,        jumpbug,    jumpbug,    galaxian_state, jumpbug,    ROT90,  "Hoei (Rock-Ola license)", "Jump Bug", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND ) // or by Alpha Denshi Co. under contract from Hoei?
GAME( 1981, jumpbugb,    jumpbug,  jumpbug,    jumpbug,    galaxian_state, jumpbug,    ROT90,  "bootleg", "Jump Bug (bootleg)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND ) // bootleg of Sega license
GAME( 1983, levers,      0,        jumpbug,    levers,     galaxian_state, jumpbug,    ROT90,  "Rock-Ola", "Levers", MACHINE_SUPPORTS_SAVE )

/* 2nd CPU driving AY8910 for sound */
GAME( 1982, checkman,    0,        checkman,   checkman,   galaxian_state, checkman,   ROT90,  "Zilec-Zenitone", "Check Man", MACHINE_SUPPORTS_SAVE )
GAME( 1982, checkmanj,   checkman, checkmaj,   checkmaj,   galaxian_state, checkmaj,   ROT90,  "Zilec-Zenitone (Jaleco license)", "Check Man (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, dingo,       0,        checkmaj,   dingo,      galaxian_state, dingo,      ROT90,  "Ashby Computers and Graphics Ltd. (Jaleco license)", "Dingo", MACHINE_SUPPORTS_SAVE )
GAME( 1983, dingoe,      dingo,    checkman,   dingo,      galaxian_state, dingoe,     ROT90,  "Ashby Computers and Graphics Ltd.", "Dingo (encrypted)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )

/* Crazy Climber sound plus AY8910 instead of galaxian sound, plus INT instead of NMI */
GAME( 1981, mshuttle,    0,        mshuttle,   mshuttle,   galaxian_state, mshuttle,   ROT0,   "Nichibutsu", "Moon Shuttle (US? set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, mshuttle2,   mshuttle, mshuttle,   mshuttle,   galaxian_state, mshuttle,   ROT0,   "Nichibutsu", "Moon Shuttle (US? set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, mshuttlej,   mshuttle, mshuttle,   mshuttle,   galaxian_state, mshuttlj,   ROT0,   "Nichibutsu", "Moon Shuttle (Japan set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, mshuttlej2,  mshuttle, mshuttle,   mshuttle,   galaxian_state, mshuttlj,   ROT0,   "Nichibutsu", "Moon Shuttle (Japan set 2)", MACHINE_SUPPORTS_SAVE )

/* 2nd CPU driving DAC for sound */
GAME( 1980, kingball,    0,        kingball,   kingball,   galaxian_state, kingball,   ROT90,  "Namco", "King & Balloon (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, kingballj,   kingball, kingball,   kingball,   galaxian_state, kingball,   ROT90,  "Namco", "King & Balloon (Japan)", MACHINE_SUPPORTS_SAVE )



/*************************************
 *
 *  Game drivers
 *  Konami games
 *
 *************************************/

/* Frogger based hardware: 2nd Z80, AY-8910A, 2 8255 PPI for I/O, custom background */
GAME( 1981, frogger,     0,        frogger,    frogger,    galaxian_state, frogger,    ROT90,  "Konami", "Frogger", MACHINE_SUPPORTS_SAVE )
GAME( 1981, froggers1,   frogger,  frogger,    frogger,    galaxian_state, frogger,    ROT90,  "Konami (Sega license)", "Frogger (Sega set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, froggers2,   frogger,  frogger,    frogger,    galaxian_state, frogger,    ROT90,  "Konami (Sega license)", "Frogger (Sega set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, froggermc,   frogger,  froggermc,  froggermc,  galaxian_state, froggermc,  ROT90,  "Konami (Sega license)", "Frogger (Moon Cresta hardware)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, froggers,    frogger,  froggers,   frogger,    galaxian_state, froggers,   ROT90,  "bootleg", "Frog", MACHINE_SUPPORTS_SAVE )
GAME( 1981, frogf,       frogger,  frogf,      frogger,    galaxian_state, froggers,   ROT90,  "bootleg (Falcon)", "Frog (Falcon bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, frogg,       frogger,  galaxian,   frogg,      galaxian_state, frogg,      ROT90,  "bootleg", "Frog (Galaxian hardware)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, froggrs,     frogger,  froggers,   frogger,    galaxian_state, froggrs,    ROT90,  "bootleg (Coin Music)", "Frogger (Scramble hardware)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, quaak,       frogger,  quaak,      frogger,    galaxian_state, quaak,      ROT90,  "bootleg", "Quaak (bootleg of Frogger)", MACHINE_SUPPORTS_SAVE ) // closest to Super Cobra hardware, presumably a bootleg from Germany (Quaak is the German frog sound)
GAME( 1981, froggeram,   frogger,  froggeram,  frogger,    galaxian_state, quaak,      ROT90,  "bootleg", "Frogger (bootleg on Amigo? hardware)", MACHINE_NOT_WORKING ) // meant to be Amigo hardware, but maybe a different bootleg than the one we have?


/*
    Turtles based hardware

    CPU/Video Board: KT-4108-2
    Sound Board:     KT-4108-1
*/
GAME( 1981, turtles,     0,        turtles,    turtles,    galaxian_state, turtles,    ROT90,  "Konami (Stern Electronics license)", "Turtles", MACHINE_SUPPORTS_SAVE )
GAME( 1981, turpin,      turtles,  turtles,    turpin,     galaxian_state, turtles,    ROT90,  "Konami (Sega license)", "Turpin", MACHINE_SUPPORTS_SAVE )
GAME( 1981, 600,         turtles,  turtles,    turtles,    galaxian_state, turtles,    ROT90,  "Konami", "600", MACHINE_SUPPORTS_SAVE )

GAME( 1982, amidar,      0,        turtles,    amidaru,    galaxian_state, turtles,    ROT90,  "Konami", "Amidar", MACHINE_SUPPORTS_SAVE )
GAME( 1981, amidar1,     amidar,   turtles,    amidar,     galaxian_state, turtles,    ROT90,  "Konami", "Amidar (older)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, amidaru,     amidar,   turtles,    amidaru,    galaxian_state, turtles,    ROT90,  "Konami (Stern Electronics license)", "Amidar (Stern Electronics)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, amidaro,     amidar,   turtles,    amidaro,    galaxian_state, turtles,    ROT90,  "Konami (Olympia license)", "Amidar (Olympia)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, amidarb,     amidar,   turtles,    amidaru,    galaxian_state, turtles,    ROT90,  "bootleg", "Amidar (bootleg)", MACHINE_SUPPORTS_SAVE ) /* similar to Amigo bootleg */
GAME( 1982, amigo,       amidar,   turtles,    amidaru,    galaxian_state, turtles,    ROT90,  "bootleg", "Amigo", MACHINE_SUPPORTS_SAVE )
GAME( 1982, amidars,     amidar,   scramble,   amidars,    galaxian_state, scramble,   ROT90,  "Konami", "Amidar (Scramble hardware)", MACHINE_SUPPORTS_SAVE )


/* The End/Scramble based hardware */
GAME( 1980, theend,      0,        theend,     theend,     galaxian_state, theend,     ROT90,  "Konami", "The End", MACHINE_SUPPORTS_SAVE )
GAME( 1980, theends,     theend,   theend,     theend,     galaxian_state, theend,     ROT90,  "Konami (Stern Electronics license)", "The End (Stern Electronics)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, theendb,     theend,   theend,     theend,     galaxian_state, theend,     ROT90,  "bootleg?", "The End (bootleg?)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )

GAME( 1981, scramble,    0,        scramble,   scramble,   galaxian_state, scramble,   ROT90,  "Konami", "Scramble", MACHINE_SUPPORTS_SAVE )
GAME( 1981, scrambles,   scramble, scramble,   scramble,   galaxian_state, scramble,   ROT90,  "Konami (Stern Electronics license)", "Scramble (Stern Electronics set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, scrambles2,  scramble, scramble,   scramble,   galaxian_state, scramble,   ROT90,  "Konami (Stern Electronics license)", "Scramble (Stern Electronics set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, strfbomb,    scramble, scramble,   strfbomb,   galaxian_state, scramble,   ROT90,  "bootleg (Omni)",                     "Strafe Bomb (bootleg of Scramble)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, explorer,    scramble, explorer,   explorer,   galaxian_state, explorer,   ROT90,  "bootleg (Sidam)",                    "Explorer (bootleg of Scramble)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, scramblebf,  scramble, scramble,   scramble,   galaxian_state, scramble,   ROT90,  "bootleg (Karateko)",                 "Scramble (Karateko, French bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, scrambp,     scramble, scramble,   scramble,   galaxian_state, scramble,   ROT90,  "bootleg (Billport S.A.)",            "Impacto (Billport S.A., Spanish bootleg of Scramble)", MACHINE_SUPPORTS_SAVE ) // similar to the Karateko set above
GAME( 1981, scramce,     scramble, scramble,   scramble,   galaxian_state, scramble,   ROT90,  "bootleg (Centromatic S.A.)",         "Scramble (Centromatic S.A., Spanish bootleg)", MACHINE_SUPPORTS_SAVE ) // similar to above
GAME( 1981, scrampt,     scramble, scramble,   scramble,   galaxian_state, scramble,   ROT90,  "bootleg (Petaco S.A.)",              "Scramble (Petaco S.A., Spanish bootleg)", MACHINE_SUPPORTS_SAVE ) // ^^
GAME( 1981, scramrf,     scramble, scramble,   scramble,   galaxian_state, scramble,   ROT90,  "bootleg (Recreativos Franco)",       "Scramble (Recreativos Franco, Spanish bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, scramblebb,  scramble, scramble,   scramble,   galaxian_state, scramble,   ROT90,  "bootleg?", "Scramble (bootleg?)", MACHINE_SUPPORTS_SAVE )
GAME( 198?, bomber,      scramble, scramble,   scramble,   galaxian_state, scramble,   ROT90,  "bootleg (Alca)", "Bomber (bootleg of Scramble)", MACHINE_SUPPORTS_SAVE )

GAME( 1981, atlantis,    0,        theend,     atlantis,   galaxian_state, atlantis,   ROT90,  "Comsoft", "Battle of Atlantis (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, atlantis2,   atlantis, theend,     atlantis,   galaxian_state, atlantis,   ROT90,  "Comsoft", "Battle of Atlantis (set 2)", MACHINE_SUPPORTS_SAVE )

/* Scorpion hardware; based on Scramble but with a 3rd AY-8910 and a speech chip */
GAME( 1982, scorpion,    0,        scorpion,   scorpion,   galaxian_state, scorpion,   ROT90,  "Zaccaria", "Scorpion (set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE)
GAME( 1982, scorpiona,   scorpion, scorpion,   scorpion,   galaxian_state, scorpion,   ROT90,  "Zaccaria", "Scorpion (set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE)
GAME( 1982, scorpionb,   scorpion, scorpion,   scorpion,   galaxian_state, scorpion,   ROT90,  "Zaccaria", "Scorpion (set 3)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE)
GAME( 19??, scorpionmc,  scorpion, mooncrst,   scorpnmc,   galaxian_state, scorpnmc,   ROT90,  "bootleg? (Dorneer)", "Scorpion (Moon Cresta hardware)", MACHINE_SUPPORTS_SAVE )
GAME( 19??, aracnis,     scorpion, mooncrst,   aracnis,    galaxian_state, scorpnmc,   ROT90,  "bootleg",  "Aracnis (bootleg of Scorpion on Moon Cresta hardware)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )

/* SF-X hardware; based on Scramble with extra Z80 and 8255 driving a DAC-based sample player */
GAME( 1983, sfx,         0,        sfx,        sfx,        galaxian_state, sfx,        ORIENTATION_FLIP_X, "Nichibutsu", "SF-X", MACHINE_SUPPORTS_SAVE )
GAME( 1983, skelagon,    sfx,      sfx,        sfx,        galaxian_state, sfx,        ORIENTATION_FLIP_X, "Nichibutsu USA", "Skelagon", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE)
GAME( 1982, monsterz,    0,        monsterz,   sfx,        galaxian_state, sfx,        ORIENTATION_FLIP_X, "Nihon Game", "Monster Zero", MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )


/*
    Super Cobra

    CPU/Video Board: A969
    Sound Board:     A970
*/
GAME( 1981, scobra,      0,        scobra,     scobra,     galaxian_state, scobra,     ROT90,  "Konami", "Super Cobra", MACHINE_SUPPORTS_SAVE )
GAME( 1981, scobrase,    scobra,   scobra,     scobra,     galaxian_state, scobra,     ROT90,  "Konami (Sega license)", "Super Cobra (Sega)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, scobras,     scobra,   scobra,     scobras,    galaxian_state, scobra,     ROT90,  "Konami (Stern Electronics license)", "Super Cobra (Stern Electronics)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, scobrae,     scobra,   scobra,     scobras,    galaxian_state, scobrae,    ROT90,  "Konami (Stern Electronics license)", "Super Cobra (Stern Electronics) (encrypted, KONATEC XC-103SS CPU)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, scobrab,     scobra,   scobra,     scobras,    galaxian_state, scobra,     ROT90,  "bootleg", "Super Cobra (bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, suprheli,    scobra,   scobra,     scobras,    galaxian_state, scobra,     ROT90,  "bootleg", "Super Heli (Super Cobra bootleg)", MACHINE_SUPPORTS_SAVE )

GAME( 1981, moonwar,     0,        moonwar,    moonwar,    galaxian_state, moonwar,    ROT90,  "Stern Electronics", "Moonwar", MACHINE_SUPPORTS_SAVE )
GAME( 1981, moonwara,    moonwar,  moonwar,    moonwara,   galaxian_state, moonwar,    ROT90,  "Stern Electronics", "Moonwar (older)", MACHINE_SUPPORTS_SAVE )

GAME( 1981, armorcar,    0,        scobra,     armorcar,   galaxian_state, scobra,     ROT90,  "Stern Electronics", "Armored Car (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, armorcar2,   armorcar, scobra,     armorcar2,  galaxian_state, scobra,     ROT90,  "Stern Electronics", "Armored Car (set 2)", MACHINE_SUPPORTS_SAVE )

GAME( 1982, tazmania,    0,        scobra,     tazmania,   galaxian_state, scobra,     ROT90,  "Stern Electronics", "Tazz-Mania (set 1)", MACHINE_SUPPORTS_SAVE )


/*
    Anteater (sold as conversion kit)

    CPU/Video Board: A969 (Has various wire mods)
    Sound Board:     A970
*/
GAME( 1982, anteater,    0,        anteater,   anteater,   galaxian_state, anteater,   ROT90,  "Tago Electronics", "Anteater", MACHINE_SUPPORTS_SAVE )
GAME( 1982, anteateruk,  anteater, anteateruk, anteateruk, galaxian_state, anteateruk, ROT90,  "Tago Electronics (Free Enterprise Games license", "The Anteater (UK)", MACHINE_SUPPORTS_SAVE ) // distributed in 1983
GAME( 1982, anteaterg,   anteater, anteaterg,  anteateruk, galaxian_state, anteateruk, ROT90,  "Tago Electronics (TV-Tuning license from Free Enterprise Games)", "Ameisenbaer (German)", MACHINE_SUPPORTS_SAVE )

GAME( 1982, calipso,     0,        scobra,     calipso,    galaxian_state, calipso,    ROT90,  "Tago Electronics", "Calipso",  MACHINE_SUPPORTS_SAVE )


/*
    Lost Tomb

    CPU/Video Board: A969 (Has various wire mods)
    Sound Board:     A2048
*/
GAME( 1982, losttomb,    0,        scobra,     losttomb,   galaxian_state, losttomb,   ROT90,  "Stern Electronics", "Lost Tomb (easy)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, losttombh,   losttomb, scobra,     losttomb,   galaxian_state, losttomb,   ROT90,  "Stern Electronics", "Lost Tomb (hard)", MACHINE_SUPPORTS_SAVE )

GAME( 1984, spdcoin,     0,        scobra,     spdcoin,    galaxian_state, scobra,     ROT90,  "Stern Electronics", "Speed Coin (prototype)", MACHINE_SUPPORTS_SAVE )

GAME( 1985, superbon,    0,        scobra,     superbon,   galaxian_state, superbon,   ROT90,  "Signatron USA", "Agent Super Bond (Super Cobra conversion)", MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
