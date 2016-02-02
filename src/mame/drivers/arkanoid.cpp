// license:BSD-3-Clause
// copyright-holders:Brad Oliver
/***************************************************************************

    Arkanoid driver

    - MCU hookup needs sorting out properly.  The hookups for the different types
      is not identical.

    Japanese version support cocktail mode (DSW #7), the others don't.

    Here are the versions we have:

    arkanoid    The earlier revisions. They each differ in the country byte. These
    arkanoiduo    versions work fine both the bootleg a75-06 MCU rom and the
    arkanoidjb    genuine decapped Taito A75-06.IC16 M68705 MCU.
    arkanoidu   USA version. A later revision, code has been inserted NOT patched.
                The 68705 code for this one was not available; I made it up from
                the current A75-06.IC16 changing the level data pointer table.
    arkanoidj   Japanese version.  Final revision, MCU code not dumped.
    arkanoidja  Japanese version. A later revision with level selector.
                  The 68705 code for this one was not available; I made it up from
                  the current A75-06.IC16 changing the level data pointer table.
    arkanoidjbl Bootleg of the early Japanese version. The only difference is
                  that the warning text has been replaced by "WAIT"
                  This version came with its own 68705p3 MCU ROM which is based on
                  the original Taito one.
    arkatayt    Another bootleg of the early Japanese one, more heavily modified
    arkblock    Another bootleg of the early Japanese one, more heavily modified
    arkbloc2    Another bootleg
    arkbl3      Another bootleg of the early Japanese one, more heavily modified
    paddle2     Another bootleg of the early Japanese one, more heavily modified
    arkangc     Game Corporation bootleg with level selector


    Most if not all Arkanoid sets have a bug in their game code. It occurs on the
    final level where the player has to dodge falling objects. The bug resides in
    the collision detection routine which sometimes reads from unmapped addresses
    above $F000. For these addresses it is vital to read zero values, or else the
    player will die for no reason.

Measured Clocks:
   Z80 - 5997077Hz (6Mhz)
M68705 - 2998533Hz (3Mhz)
YM2149 - 2998531Hz (3Mhz)


****************************************************************************

Guru's readme

Arkanoid
Taito 1986

PCB Layout
----------

Note an original Taito Arkanoid PCB is approximately 10" square and is
painted white. The copper traces are not visible. The part type and
location of each component is printed in green on the PCB on top of the
white paint.

The following MCU images were tested on an original Arkanoid PCB using sets
'arkanoid', 'arkanoidu' and 'arkanoiduo' and work as expected.
(1) MCU image with CRC 0x389a8cfb <- this is a deprotected copy of the original Taito A75__06 MCU code
(2) MCU image with CRC 0x515d77b6 <- this is a blackbox-reverse engineered bootleg MCU written by pirates

An MCU found on a Tournament Arkanoid PCB was an unprotected type MC68705P3
and when read the CRC matched (1). So we can assume the MCUs for Arkanoid and
Tournament Arkanoid are the same.... or are at least interchangeable and work.

"Tetris (D.R. Korea)" in MAME is a hack on an original Arkanoid PCB.
The hack can be undone and returned to Arkanoid simply by removing the mod
wires on the YM2149, replacing the ROMs with Arkanoid ROMs and replacing
the PC030CM which was removed. A working Arkanoid 68705 MCU is also required.
The above 'tested' images can be used.


J1100075A
K1100180A
K1100181A (ROMSTAR version added sticker)
  |---------------------------------------------|
  |                VOLUME  TL7700   TMM2018     |
|-|         MB3731                              |
|                                               |
|P                                              |
|O                                              |
|W          DSWA(8)  A75-09.IC22                |
|E                                              |
|R         YM2149F   A75-08.IC23                |
|                                               |
|-|                  A75-07.IC24                |
  |                                             |
  |                                             |
|-|                                 TMM2016     |
|                                               |
|          PC030CM                  TMM2016     |
|                                               |
|               JP4 JP3                         |
|2                                              |
|2         A75_06.IC14                          |
|W 48CR-1                                       |
|A                                              |
|Y                                              |
|          TMM2016                         12MHz|
|  48CR-1                                       |
|  48CR-1  A75_10.IC16    A75_05.IC62  MB112S146|
|                                               |
|-|        A75_01-1.IC17  A75_04.IC63  MB112S146|
  |48CR-1                                       |
  |                       A75_03.IC64           |
  |        Z80                                  |
  |---------------------------------------------|
Notes:
      Z80         - Zilog Z0840006 CPU. Clock input 6.000MHz (12/2)
      YM2149F     - Yamaha YM2149F software-controlled sound generator (SSG). Clock input 1.5MHz (12/8)
      A75_06.IC14 - Motorola MC68705P5 micro-controller. Clock input 3.000MHz (12/4). Labelled 'A75 06' for
                    ROMSTAR version. Note original Taito version 68705 and Tournament Arkanoid MCUs work fine.
      A75_*       - 27C256 EPROMs labelled 'A75 xx'. xx = 01, 03, 04, 05 etc. See ROM loading in the src for exact ROM usage.
      A75-0*      - MMI 63S241 bipolar PROMs. Compatible with MB7116, 7621, DM74S571N etc
      TMM2018     - Toshiba TMM2018 2k x8 SRAM (DIP24)
      TMM2016     - Toshiba TMM2016 2k x8 SRAM (DIP24)
      MB112S146   - Fujitsu MB112S146. Some kind of custom graphic decoder/shifter (DIP28)
      MB3731      - Fujitsu MB3731 18W BTL audio power amplifier (SIP12)
      PC030CM     - Taito custom ceramic package (SIP20)
      48CR-1      - Taito custom resistor array (SIP10)
      TL7700      - Texas Instruments TL7700CP supply voltage supervisor i.e. reset chip (DIP8)
      JP3         - 2-pin jumper. This is open but the game works even if it is closed.
      JP4         - 2-pin jumper. Must be closed to allow coin-up through PC030CM otherwise coin-up does not work.
                    Note the G connector is the 22-way edge connector.
                    The Japanese manual states (translated to English).....
                    ********
                    The coin-SW of this Main PC Board does not work without wiring coin meter to
                    coin meter pins of the G-connector.
                    You need to modify as follows in case coin meter is not connected to Main PC Board.
                    Coin System A ..... Wire jumper JP4 on Main PC Board. Coin meter not used.
                    Coin System B ..... Wire jumper JP3 on Main PC Board. Coin meter used.
                    ********

      Measured Syncs
      --------------
      HSync       - 15.625kHz
      VSync       - 59.185Hz


      POWER connector H
      -----------------
      1    Ground
      2    Ground
      3    Ground
      4    Ground
      5    +5V
      6    +5V
      7    +5V
      8    NC
      9    +12V
      10   Post
      11   NC
      12   NC


      22-way edge connector G
      -----------------------

           PARTS         SOLDER
           --------------------
                 |-----|
          GROUND | 1 A | GROUND
       VIDEO RED | 2 B | VIDEO GROUND
     VIDEO GREEN | 3 C | VIDEO BLUE
     VIDEO SYNC  | 4 D |
     SOUND OUT + | 5 E | SOUND OUT -
            POST | 6 F | POST
                 | 7 H |
     COIN SW (A) | 8 J | COIN SW (B)
  COIN METER (A) | 9 K | COIN METER (B)
COIN LOCKOUT (A) |10 L | COIN LOCKOUT (B)
      SERVICE SW |11 M | TILT SW
         START 1 |12 N | START 2
                 |13 P |
                 |14 R |
        1P RIGHT |15 S | 2P RIGHT \
         lP LEFT |16 T | 2P LEFT  / Connect 15/16/S/T to the spinner left/right connections
                 |17 U |
                 |18 V |
                 |19 W |
                 |20 X |
   lP SERVE/FIRE |21 Y | 2P SERVE/FIRE
                 |22 Z |
                 |-----|

Note about spinner controller
-----------------------------

This game requires a geared spinner to operate correctly. A trackball or other optical
controller or home-made spinner built from a PC mouse will work but the player moves too
slowly and the game is unplayable. The Taito geared spinner moves the optical wheel *very*
fast to ensure the player moves fast enough to follow and return the ball easily. The ratio of
the control knob rotation to the optical wheel rotation is 1:20 so for one rotation of the
control knob the optical wheel rotates 20 times. The optical quadrature wheel has 24 slots.
Generally a half-turn of the control knob is enough to move the player across the full screen.

The spinner connections are....
Pin 1 - Left
Pin 2 - +5V
Pin 3 - Ground
Pin 4 - Right

These pins are listed from the Japanese Taito manual and have been tested to be correct with
the real Taito Arkanoid spinner.
The US ROMSTAR manual lists pin 4 as left and pin 1 as right. This information is probably
incorrect. Pins 2 and 3 are the same.

Spinner PCB Layout
------------------
J9000024A
K9000060A
|-----------|
|   OPTO    |
|          S|
|           |
|          S|
|           |
|  POWER    |
|-4-3-2-1---|
Notes:
      OPTO  - Optical transmitter/receiver on other side of PCB
      POWER - Power input connector. Pin 1 is on the right.
      S     - Screw positions to show orientation of the PCB with reference to the power connector pin 1


DIP Switches
+-----------------------------+--------------------------------+
|FACTORY DEFAULT = *          |  1   2   3   4   5   6   7   8 |
+----------+------------------+----+---+-----------------------+
|          |*1 COIN  1 CREDIT | OFF|OFF|                       |
|COINS     | 1 COIN  2 CREDITS| ON |OFF|                       |
|          | 2 COINS 1 CREDIT | OFF|ON |                       |
|          | 1 COIN  6 CREDITS| ON |ON |                       |
+----------+------------------+----+---+---+                   |
|LIVES     |*3                |        |OFF|                   |
|          | 5                |        |ON |                   |
+----------+------------------+--------+---+---+               |
|BONUS     |*20000 / 60000    |            |OFF|               |
|1ST/EVERY | 20000 ONLY       |            |ON |               |
+----------+------------------+------------+---+---+           |
|DIFFICULTY|*EASY             |                |OFF|           |
|          | HARD             |                |ON |           |
+----------+------------------+----------------+---+---+       |
|GAME MODE |*GAME             |                    |OFF|       |
|          | TEST             |                    |ON |       |
+----------+------------------+--------------------+---+---+   |
|SCREEN    |*NORMAL           |                        |OFF|   |
|          | INVERT           |                        |ON |   |
+----------+------------------+------------------------+---+---+
|CONTINUE  | WITHOUT          |                            |OFF|
|          |*WITH             |                            |ON |
+----------+------------------+----------------------------+---+


***************************************************************************

Stephh's notes (based on the games Z80 code and some tests) :

0) Useful addresses and routines

0a) "Game Corporation" bootlegs, Tayto bootlegs, 'arkmcubl', 'ark1ball'

  - Basic routines :
      * 0x2044 : BC += A;
      * 0x204a : DE += A;
      * 0x2050 : HL += A;
      * 0x2056 : HL += A; DE = (HL);
      * 0x21f1 : Display string :
                   Inputs : DE = string address in ROM
                            HL = address where the string will be displayed
                                 (most of the times in video RAM)
                            A = colour
                   String begins with the number of chars to display
                   (eg: 0x02 "OK" to display "OK")
      * 0x210d : Display 1 char (called by previous routine)
      * 0x264a : Display score :
                   Inputs : DE = score address
                            HL = address where the score will be displayed
                                 (most of the times in video RAM)
      * 0x266f : Display 1 digit (called by previous routine)
      * 0x67ae : Play sound (input : register A) - to be confirmed !

  - Level issues :
      * 0xed72 : Level (0x00-0x20)
      * 0xed83 : Breakable bricks left (gold bricks are NOT counted)
      * 0x55a0 : Display level background
      * 0x55d9 : Display level bricks :
                   Inputs : IY = index for bricks counter
                            B = number of bricks to display
      * 0x55f6 : Display 1 background "column"
      * 0x5639 : Display 1 brick with its shadow
      * 0xed6b : Hits in the head of last level (0x00-0x10)
                 This value is reset to 0x00 each time you lose a life.
      * 0x8d15 : Check if head of last level is dead

  - Lives issues :
      * 0xed71 : Current player lives
      * 0xed76 : Player 1 lives
      * 0xed7b : Player 2 lives
      * 0xef68 : When bit 0 set to 1, no more lives on score for player 1
      * 0xef69 : When bit 0 set to 1, no more lives on score for player 2
      * 0xef6a : Player 1 counter for extra lives on score
      * 0xef6b : Player 2 counter for extra lives on score
      * 0x22a0 : Draw lives at the bottom left of the screen
      * 0x2785 : Check player score for extra life

  - Score issues :
      * 0xc4d7 : Player 1 score / 10 (3 bytes, BCD coded, MSB first)
      * 0xc4db : Player 2 score / 10 (3 bytes, BCD coded, MSB first)
      * 0xc4df : Highscore / 10 (3 bytes, BCD coded, MSB first)
      * 0xef6c : Player 1 score / 1000 for next life (2 bytes, BCD coded, MSB first)
      * 0xef6e : Player 2 score / 1000 for next life (2 bytes, BCD coded, MSB first)
      * 0x2723 : Player score += DE; (BCD => 'daa' instructions)
      * 0x274a : Check player score for highscore

  - Speed issues :
      * 0xef63 : Speed counter which is increased each time the ball touches
                 your paddle, any wall (up, left or right) or a brick;
                 it is reset when speed changes or when you lose a life.
      * 0x094c : Speed table (16 bytes)
      * 0xc462 : Speed (0x01-0x0e) :
                   Speed +1 when counter is above the value in the table
                   (check code at 0x0900)
                   Speed -2 when you get the slow pill ("S")
                   (check code at 0x5423)
                   Speed is sometimes increased when up wall is hit
                   (check code at 0x1442 and table at 0x1462)

  - Pills issues :
      * 0xc658 : Falling pill (0x80 = none - 0x81-0x87)
      * 0x5474 : Pills routines table (7 * 2 bytes, LSB first) :
          . 0x53ff : "L" pill (laser)
          . 0x540d : "E" pill (enlarge)
          . 0x5418 : "C" pill (glue)
          . 0x5423 : "S" pill (slow)
          . 0x5436 : "B" pill (warp door)
          . 0x5446 : "D" pill (3 balls)
          . 0x5451 : "P" pill (extra life)
      * 0x53c8 : Check pill effect

  - Miscellaneous addresses :
      * 0xef66 : 0x00 in demo mode else 0x01
      * 0xed6f : Bit 1 determines player : 0 = player 1 - 1 = player 2
      * 0xc4ce : Warp door status : 0x00 = closed - 0x01 = opened
      * 0xc469 : Balls in play : 0x00 = 1 ball - 0x02 = 2 or 3 balls

  - Miscellaneous routines :
      * 0xa234 : Enter initials
      * 0xa343 : Check if player has entered "SEX" as initials;
                 if so, replace them with "H !" (no side effect)


1) Bootlegs with MCU

1a) 'arkmcubl'

  - Region = 0x76 (Japan).
  - The bootleg is based on the Japenese version.
  - The MCU is dumped, but the game doesn't run with it.
    However, there is no problem if I use the one from the World early version.
    Until I know what to do with it, I use the MCU from the World early version
    and "save" the existing one in "user1". Let me know if it's good.
  - "(c) Taito Corporation 1986".
  - Displays the "Arkanoid" title.
  - "HARDWARE TEST" message is written, tests are performed, countdown 11 to 0.
  - "NOTICE" screen replaces by "WAIT" without any more text.
    However, the text is still in the ROM at 0x7b81 with changes at the beginning :
      * "NOTICE" -> "WAIT  "
      * 0xbe "THIS GAME IS" -> 0x01 " " 0x7b "IS" 0xde "GAME" 0x6d "IS"
    IMO these changes are made to bypass the checksums
  - You can't select your starting level
  - Known bugs : NONE !

1b) 'ark1ball'

  - Note from the dumper (f205v, 2005.09.29) : "It's a bootleg of "Arkanoid (Japan)",
    with notice screen eliminated (it only shows a black screen with a red WAIT)
    and a fix (no dips selection) 1 ball x game and NO starting level selection".
    However, there is still code in the game which tests the Dip Switches !
  - Region = 0x76 (Japan).
  - The bootleg is based on a Japenese early version we don't have.
    In fact, it is completely based on 'arkmcubl' :

      Z:\MAME\roms>romcmp ark1ball.zip arkmcubl.zip -d
      3 and 2 files
      e1.6d                   a-1.7d                  IDENTICAL
      e2.6f                   2palline.7f             99.957275%
      68705p3.6i                                      NO MATCH

      Z:\MAME\data>fc /B e2.6f 2palline.7f
      Comparing files e2.6f and 2PALLINE.7F
      000013BE: 20 60
      00001A28: 05 02
      00001A29: 03 01
      00001C80: 20 60
      00001C9C: 20 60
      00001ED9: 53 36
      00001EDA: 53 35
      00001EE0: 53 33
      00001EE7: 54 34
      00001EE9: 52 42
      00001EF7: 52 42
      00001EF9: 66 46

  - The MCU is not dumped, and the game doesn't run with the one from 'arkmcubl'.
    However, there is no problem if I use the one from the World early version.
    Until I know what to do with it, I use the MCU from the World early version.
  - This version is supposed to be a harder version :
      * less lives (1 or 2 instead of 3 or 5)
      * 60K for 1st bonus life instead of 20K
  - Known bugs :
      * Names on highscores table are wrong
        (ingame bug to bypass the checksums)

2) "Game Corporation" bootlegs and assimilated ones.

  - Region = 0x76 (Japan).
  - All bootlegs are based on a (bootleg) version we don't have.
  - Team credits have been replaced by bootleggers code.
  - Start of levels table at 0x7bd5 (32 * 2 bytes - MSB first)

2a) 'arkangc'

  - "(c)  Game Corporation 1986".
  - Displays the "Arkanoid" title but routine to display "BLOCK" with bricks exists.
  - No hardware test and no "NOTICE" screen.
  - All reads from 0xf002 are patched.
  - No reads from 0xd008.
  - "Continue" Dip Switch has been replaced by sort of "Debug" Dip Switch :
      * affects ball speed at start of level (0x06 or 0x08)
      * affects level 2 (same as normal version or same as level 30)
  - You can select your starting level (between 1 and 30)
    but they aren't displayed like in the original Japanese set we have ('arkanoidja').
  - Level 30 differs from original Japanese version
  - There seems to be code to edit levels (check code at 0x8082), but the routines
    don't seem to be called anymore.
  - Known bugs :
      * The paddle isn't centered when starting a new life and / or level;
        it doesn't "backup" the paddle position when a life is lost as well
        (I can't tell at the moment if it's an ingame bug or not)
        So the paddle can sometimes appear in the left wall !
      * You are told be to able to select your starting level from level 1 to level 32
        (ingame bug - check code at 0x3425)

2b) 'arkangc2'

  - "(c)  Game Corporation 1986".
  - Displays the "Arkanoid" title but routine to display "BLOCK" with bricks exists.
  - No hardware test and no "NOTICE" screen.
  - No reads from 0xf002.
  - Reads bit 1 from 0xd008.
  - "Continue" Dip Switch has been replaced by sort of "Debug" Dip Switch :
      * affects ball speed at start of level (0x04 or 0x06)
      * affects level 2 (same as normal version or same as level 30)
  - You can select your starting level (between 1 and 30)
    but they aren't displayed like in the original Japanese set we have ('arkanoidja').
    No "What round do you want to start from ?" message though.
  - Level 30 differs from original Japanese version (it also differs from 'arkangc')
  - The routine to handle the paddle is completely different as in 'arkangc'
    and any other bootlegs (check code at 0x96b0)
  - There seems to be code to edit levels (check code at 0x8082), but the routines
    don't seem to be called anymore.
  - Known bugs :
      * The paddle isn't centered when starting a new life and / or level;
        it doesn't "backup" the paddle position when a life is lost as well
        (I can't tell at the moment if it's an ingame bug or not)
        So the paddle can sometimes appear in the left wall !
      * You are told be to able to select your starting level from level 1 to level 32
        (ingame bug - check code at 0x3425)
      * The "test mode" display is completely corrupted
        (ingame bug - check unused code at 0x2f00 instead of standard text)
        But you can still press the buttons and test the paddle and the Dip Switches.

2c) 'block2'

  - "       S.  P.  A.  CO.    ".
  - Displays "BLOCK II" with "bricks".
  - Colored bricks have been replaced with aliens (ripped from "Space Invaders" ?)
  - No standard hardware test and no "NOTICE" screen.
  - Specific hardware test which reads back at 0xf000 values written to 0xd018
    (check code at 0x035e); the game enters a loop until good values are returned.
  - No reads from 0xf002.
  - Reads bit 1 from 0xd008.
  - "Continue" Dip Switch has been replaced by sort of "Debug" Dip Switch as in 'arkangc';
    however, this has no effect due to newly patched code at 0x06e9 !
  - You can select your starting level (between 1 and 30)
    but they aren't displayed like in the original Japanese set we have ('arkanoidja').
  - Levels 1, 2, 3, 4, 6, 7, 11, 14, 30, 31 and 32 differ from original Japanese version;
    level 1 starts at a different offset (0x90a8 instead of 0xbf15).
  - Complerely different initials on high-scores table, but scores and rounds
    are the same as in the original Japanese set we have ('arkanoidja').
  - There seems to be code to edit levels (check code at 0x8082), but the routines
    don't seem to be called anymore.
  - Known bugs :
      * The paddle isn't centered when starting a new life and / or level;
        it doesn't "backup" the paddle position when a life is lost as well
        (I can't tell at the moment if it's an ingame bug or not)
        So the paddle can sometimes appear in the left wall !
      * The "test mode" display is completely corrupted
        (ingame bug - check unused code at 0x2f00 instead of standard text)
        But you can still press the buttons and test the paddle and the Dip Switches.

2d) 'arkblock'

  - Same as 'arkangc', the only difference is that it displays "BLOCK" with bricks
    instead of displaying the "Arkanoid" title :

      Z:\MAME\dasm>diff arkangc.asm arkblock.asm
      8421,8422c8421,8424
      < 32EF: 21 80 03      ld   hl,$0380
      < 32F2: CD D1 20      call $20D1
      ---
      > 32EF: F3            di
      > 32F0: CD 90 7C      call $7C90
      > 32F3: C9            ret
      > 32F4: 14            inc  d

2e) 'arkbloc2'

  - "(c)  Game Corporation 1986".
  - Displays "BLOCK" with bricks.
  - No hardware test and no "NOTICE" screen.
  - All reads from 0xf002 are patched.
  - Reads bit 5 from 0xd008.
  - You can select your starting level (between 1 and 30) but they aren't displayed
    like in the original Japanese set we have ('arkanoidja').
  - "Continue" Dip Switch has been replaced by sort of "Debug" Dip Switch :
      * affects ball speed at start of level (0x06 or 0x08)
      * affects level 2 (same as normal version or same as level 30)
  - You can select your starting level (between 1 and 30)
    but they aren't displayed like in the original Japanese set we have ('arkanoidja').
  - Level 30 differs from original Japanese version (same as the one from 'arkangc2')
  - Known bugs :
      * You can go from one side of the screen to the other through the walls
        (I can't tell at the moment if it's an ingame bug or not)
      * You are told be to able to select your starting level from level 1 to level 32
        (ingame bug - check code at 0x3425)

2f) 'arkgcbl'

  - "1986    ARKANOID    1986".
  - Displays the "Arkanoid" title but routine to display "BLOCK" with bricks exists.
  - No hardware test and no "NOTICE" screen.
  - Most reads from 0xf002 are patched. I need to fix the remaining ones (0x8a and 0xff).
  - Reads bits 1 and 5 from 0xd008.
  - "Continue" Dip Switch has been replaced by "Round Select" Dip Switch
    ("debug" functions from 'arkangc' have been patched).
  - Different "Bonus Lives" Dip Switch :
      * "60K 100K 60K+" or "60K" when you start a new game
      * "20K 60K 60K+"  or "20K" when you continue
  - Different "Lives" Dip Switch (check table at 0x9a28)
  - Specific coinage (always 2C_1C)
  - If Dip Switch is set, you can select your starting level (between 1 and 30)
    but they aren't displayed like in the original Japanese set we have ('arkanoidja').
  - Same level 30 as original Japanese version
  - Known bugs :
      * You can go from one side of the screen to the other through the walls
        (I can't tell at the moment if it's an ingame bug or not)
      * You are told be to able to select your starting level from level 1 to level 32
        (ingame bug - check code at 0x3425)
      * Sound in "Demo Mode" if 1 coin is inserted (ingame bug - check code at 0x0283)
      * Red square on upper middle left "led" when it is supposed to be yellow (ingame bug)

2g) 'paddle2'

  - Different title, year, and inside texts but routine to display "BLOCK" with bricks exists.
  - No hardware test and no "NOTICE" screen.
  - I need to fix ALL reads from 0xf002.
  - Reads bits 0 to 3 and 5 from 0xd008.
  - "Continue" Dip Switch has been replaced by "Round Select" Dip Switch
    ("debug" functions from 'arkangc' have been patched).
  - No more "Service Mode" Dip Switch (even if code is still there for it).
    This Dip Switch now selects how spinners are handled :
      * bit 2 = 0 => read from 0xd018 only
      * bit 2 = 1 => read from 0xd018 + read from 0xd008
    I set its default to 1 as parts of the game still branch to 0x96b0.
  - Different "Bonus Lives" Dip Switch :
      * "60K 100K 60K+" or "60K" when you start a new game
      * "20K 60K 60K+"  or "20K" when you continue
  - Different "Lives" Dip Switch (check table at 0x9a28)
  - If Dip Switch is set, you can select your starting level (between 1 and 30)
    but they aren't displayed like in the original Japanese set we have ('arkanoidja').
  - Levels are based on the ones from "Arkanoid II".
  - Known bugs :
      * You can go from one side of the screen to the other through the walls
        (I can't tell at the moment if it's an ingame bug or not)
      * You can't correctly enter your initials at the end of the game
        (ingame bug ? check code at 0xa23e and difference at 0xa273)
      * On intro and last screen, colour around main ship is yellow instead of red
        (ingame bug due to numerous patches)

3) "Tayto" bootlegs and assimilated ones.

  - Region = 0x76 (Japan).
  - All bootlegs are based on a Japenese early version we don't have.
  - Start of levels table at 0xbd75 (32 * 2 bytes - LSB first)

3a) 'arkatayt'

  - "(c) Tayto Corporation 1986" but the Taito original logo is displayed.
  - Displays the "Arkanoid" title.
  - "HARDWARE TEST" message is written, tests are performed, but no countdown.
  - "NOTICE" screen replaces by "WAIT" without any more text.
    However, the text is still in the ROM at 0x7b81 with changes at the beginning :
      * "NOTICE" -> "WAIT  "
      * 0xbe "THIS" -> 0x01 " HIS"
    IMO these changes are made to bypass the checksums
  - You can't select your starting level
  - Known bugs :
      * level 16 is corrupted with extra bricks
        (ingame bug due to extra code from 0x5042 to 0x5086)
      * level 25 is shifted 8 "columns" to the right
        (ingame bug due to bad level offset at 0xbda5 : 0xe5 instead of 0xed)

3b) 'arktayt2'

  - This version is supposed to be a harder version of 'arkatayt' :
      * less lives (2 or 3 instead of 3 or 5)
      * 60K for 1st bonus life instead of 20K
    Same as 'arkatayt' otherwise
  - Known bugs :
      * level 16 is corrupted with extra bricks
        (ingame bug due to extra code from 0x5042 to 0x5086)
      * level 25 is shifted 8 "columns" to the right
        (ingame bug due to bad level offset at 0xbda5 : 0xe5 instead of 0xed)
      * Names on highscores table are wrong
        (ingame bug to bypass the checksums)


TO DO (2006.09.12) :

  - Check the following Taito sets (addresses, routines and Dip Switches) :
      * 'arkanoid' = 'arkanoiduo'
      * 'arkanoidja'
      * 'arkanoidu'
      * 'arkatour'
  - Add more notes about main addresses and routines in the Z80
  - Try to understand the problem with the MCU in the following sets :
      * 'arkmcubl'
      * 'ark1ball'


Stephh's log (2006.09.05) :

  - Interverted 'arkblock' and 'arkbloc2' sets for better comparaison
  - Renamed sets :
      * 'arkbl2'   -> 'arkmcubl'
      * 'arkbl3'   -> 'arkgcbl'
  - Changed some games descriptions
  - Removed flags from the following sets :
      * 'arkbloc2' (old 'arkblock')
      * 'arkgcbl'  (old 'arkbl3')
      * 'paddle2'
    This way, even if emulation isn't perfect, people can try them and report bugs.


Stephh's log (2006.09.12) :

  - Renamed sets :
      * 'arkatayt' -> 'arktayt2'
  - Changed some games descriptions
  - Added sets :
      * 'ark1ball'
      * 'arkangc2'
      * 'arkatayt'
  - Removed flags from the following sets :
      * 'arkmcubl'
    This way, even if emulation isn't perfect, people can try them and report bugs.


Stephh's log (2009.10.07) :

  - Added set :
      * 'block2'

***************************************************************************

Stephh's notes on 'tetrsark' (based on the game Z80 code and some tests) :

  - No reads from 0xd00c, 0xd010 nor 0xd018.
  - "Cabinet" Dip Switch :
      * when set to "Upright" :
          . each player can join and play on its half screen while
            the other is already playing
          . the screen is never flipped
      * when set to "Cocktail" :
          . only one player can play : the other player has to wait until
            current player is "GAME OVER" to press the Start button
          . screen is flipped when player 2 is playing and
            it remains flipped until player 1 starts a game
            (so "demo mode" can be upside down)
  - Credits : even if display is limited to 9, the value still increases;
    so if you insert too many coins, it can be reset to 0 !
  - Routines :
      * 0x56e3 : Play sound (input : register A) - to be confirmed !
  - Addresses :
      * 0xc52b : credits
      * 0xc541 : ~(IN5) - test for coins "buttons" (code at 0x0232)
      * 0xc516 : ~(IN5)
      * 0xc517 : ~(IN4)
  - Known bugs :
      * Coins "buttons" don't work - we need to use fake BUTTON2 for each player

****************************************************************************

    HEXA

    This hardware is derived from Arkanoid's hardware.  The 1986 date found in
    the roms probably comes from Arkanoid.  This is a Columns style game and
    the original Columns wasn't released until 1990 and I find it hard to
    believe that this would pre-date Columns.

    driver by Howie Cohen

    Memory map (prelim)
    0000 7fff ROM
    8000 bfff bank switch rom space??
    c000 c7ff RAM
    e000 e7ff video ram
    e800-efff unused RAM

    read:
    d001      AY8910 read
    f000      ???????

    write:
    d000      AY8910 control
    d001      AY8910 write
    d008      bit0/1 = flip screen x/y
              bit 4 = ROM bank??
              bit 5 = char bank
              other bits????????
    d010      watchdog reset, or IRQ acknowledge, or both
    f000      ????????

    main hardware consists of.....

    sub board with Z80 x2, 2 ROMs and a scratched 18 pin chip (probably a PIC)

    main board has....
    12MHz xtal
    ay3-8910
    8 position DSW x1
    ROMs x4
    6116 SRAM x3
    82S123 PROMs x3

***************************************************************************

DIP locations verified for:
  - arkanoidja
  - arkanoid

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/arkanoid.h"
#include "sound/ay8910.h"
#include "cpu/m6805/m6805.h"

/***************************************************************************/

/* Memory Maps */

static ADDRESS_MAP_START( arkanoid_map, AS_PROGRAM, 8, arkanoid_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xd000, 0xd001) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
	AM_RANGE(0xd001, 0xd001) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE(0xd008, 0xd008) AM_WRITE(arkanoid_d008_w)  /* gfx bank, flip screen etc. */
	AM_RANGE(0xd00c, 0xd00c) AM_READ_PORT("SYSTEM")     /* 2 bits from the 68705 */
	AM_RANGE(0xd010, 0xd010) AM_READ_PORT("BUTTONS") AM_WRITE(watchdog_reset_w)
	AM_RANGE(0xd018, 0xd018) AM_READWRITE(arkanoid_Z80_mcu_r, arkanoid_Z80_mcu_w)  /* input from the 68705 */
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(arkanoid_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xe800, 0xe83f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xe840, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_READNOP /* fixes instant death in final level */
ADDRESS_MAP_END

static ADDRESS_MAP_START( bootleg_map, AS_PROGRAM, 8, arkanoid_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xd000, 0xd000) AM_DEVWRITE("aysnd", ay8910_device, address_w)
	AM_RANGE(0xd001, 0xd001) AM_DEVREADWRITE("aysnd", ay8910_device, data_r, data_w)
	AM_RANGE(0xd008, 0xd008) AM_WRITE(arkanoid_d008_w)  /* gfx bank, flip screen etc. */
	AM_RANGE(0xd00c, 0xd00c) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xd010, 0xd010) AM_READ_PORT("BUTTONS") AM_WRITE(watchdog_reset_w)
	AM_RANGE(0xd018, 0xd018) AM_READ_PORT("MUX") AM_WRITENOP
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(arkanoid_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xe800, 0xe83f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xe840, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_READNOP /* fixes instant death in final level */
ADDRESS_MAP_END

static ADDRESS_MAP_START( hexa_map, AS_PROGRAM, 8, arkanoid_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xd001, 0xd001) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE(0xd000, 0xd001) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
	AM_RANGE(0xd008, 0xd008) AM_WRITE(hexa_d008_w)
	AM_RANGE(0xd010, 0xd010) AM_WRITE(watchdog_reset_w) /* or IRQ acknowledge, or both */
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(arkanoid_videoram_w) AM_SHARE("videoram")
ADDRESS_MAP_END

READ8_MEMBER(arkanoid_state::hexaa_f000_r)
{
//  return m_hexaa_from_sub;
	return rand();
}

WRITE8_MEMBER(arkanoid_state::hexaa_f000_w)
{
	m_hexaa_from_main = data;
}

static ADDRESS_MAP_START( hexaa_map, AS_PROGRAM, 8, arkanoid_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xd001, 0xd001) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE(0xd000, 0xd001) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
	AM_RANGE(0xd008, 0xd008) AM_WRITE(hexa_d008_w)
	AM_RANGE(0xd010, 0xd010) AM_WRITE(watchdog_reset_w) /* or IRQ acknowledge, or both */
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(arkanoid_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xe800, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xf000) AM_READWRITE(hexaa_f000_r, hexaa_f000_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( hexaa_sub_map, AS_PROGRAM, 8, arkanoid_state )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
ADDRESS_MAP_END


WRITE8_MEMBER(arkanoid_state::hexaa_sub_80_w)
{
	m_hexaa_from_sub = data;
}

READ8_MEMBER(arkanoid_state::hexaa_sub_90_r)
{
	return m_hexaa_from_main;
//  return rand();
}

static ADDRESS_MAP_START( hexaa_sub_iomap, AS_IO, 8, arkanoid_state )
	ADDRESS_MAP_GLOBAL_MASK(0x0f)
	AM_RANGE(0x00, 0x0f) AM_RAM // ?? could be communication with the other chip (protection?)
	AM_RANGE(0x80, 0x80) AM_WRITE(hexaa_sub_80_w)
	AM_RANGE(0x90, 0x90) AM_READ(hexaa_sub_90_r)
ADDRESS_MAP_END



static ADDRESS_MAP_START( brixian_map, AS_PROGRAM, 8, arkanoid_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_SHARE("protram")
	AM_RANGE(0xd000, 0xd000) AM_DEVWRITE("aysnd", ay8910_device, address_w)
	AM_RANGE(0xd001, 0xd001) AM_DEVREADWRITE("aysnd", ay8910_device, data_r, data_w)
	AM_RANGE(0xd008, 0xd008) AM_WRITE(brixian_d008_w)  /* gfx bank, flip screen etc. */
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(arkanoid_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xe800, 0xe83f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xe840, 0xefff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( mcu_map, AS_PROGRAM, 8, arkanoid_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7ff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0000) AM_READWRITE(arkanoid_68705_port_a_r, arkanoid_68705_port_a_w)
	AM_RANGE(0x0001, 0x0001) AM_READ_PORT("MUX")
	AM_RANGE(0x0002, 0x0002) AM_READWRITE(arkanoid_68705_port_c_r, arkanoid_68705_port_c_w)
	AM_RANGE(0x0004, 0x0004) AM_WRITE(arkanoid_68705_ddr_a_w)
	AM_RANGE(0x0006, 0x0006) AM_WRITE(arkanoid_68705_ddr_c_w)
	AM_RANGE(0x0008, 0x0008) AM_READWRITE(arkanoid_68705_tdr_r, arkanoid_68705_tdr_w)
	AM_RANGE(0x0009, 0x0009) AM_READWRITE(arkanoid_68705_tcr_r, arkanoid_68705_tcr_w)
	AM_RANGE(0x0010, 0x007f) AM_RAM
	AM_RANGE(0x0080, 0x07ff) AM_ROM
ADDRESS_MAP_END

/* MCU Hookup based on 'Arkanoid_TAITO_(Japan 1986) PCB.rar' from Taro and others at http://zx-pk.ru forums
ic5 = 74ls669
ic6 = 74ls669
ic9 = 74ls257
ic21 = 74ls393 (? counter)
ic26 = 74ls74 (two semaphore latches; latch 1 is == m_z80HasWritten and is cleared by ?; latch 2 is == m_68705HasWritten and is set by PC3 being output low, and cleared by ?)
ic27 = 74ls374 (mcu->z80 latch)
ic28 = 74ls374 (z80->mcu latch)
ic31 = 74ls74
ic32 = 74ls273
ic43 = 74ls157
ic45 = 74ls74
ic46 = 74ls08
ic87 = 74ls74
~VCC = 'pulled to vcc through a resistor'
icxx.y = ic xx pin y
                                                                               +--------\_/--------+
                                                        GND -- =   VSS(GND) -- |  1             28 | <- /RESET  = <- ~VCC & ic32.9 (4Q) & ic26.13 (/reset2) & ic26.1 (/reset1)
                         ~VCC & ic26.6 (/1Q) & ic9.10 (I1C) -> =       /INT -> |  2             27 | <> PA7     = -> ic27.18 (8D)
                                                        +5v -- =        VCC -- |  3         M   26 | <> PA6     = -> ic27.17 (7D)
          ic43.1 (select) & ic45.11 (clock2) & ...elsewhere -> =      EXTAL -> |  4         C   25 | <> PA5     = -> ic27.14 (6D)
                                                        GND -- =       XTAL -> |  5         6   24 | <> PA4     = -> ic27.13 (5D)
                                                        +5v -- =        VPP -- |  6         8   23 | <> PA3     = -> ic27.3 (1D)
ic46.4 (A2) & ic31.4 (/preset1) & ic21.1 (1A) & ic87.9 (2Q) -> = TIMER/BOOT -> |  7         7   22 | <> PA2     = -> ic27.4 (2D)
                                                ic26.5 (1Q) -> =  (NUM) PC0 <> |  8         0   21 | <> PA1     = -> ic27.7 (3D)
                                ic26.8 (/2Q) & ic9.13 (I1D) -> =        PC1 <> |  9         5   20 | <> PA0     = <> ic27.8 (4D) & ic28.9 (4Q)
                      ~VCC & ic26.3 (clock1) & ic28.1 (/OE) <- =        PC2 <> | 10         P   19 | <> PB7     = <- ic6.11 (QD)
              ~VCC & ic26.10 (/preset2) & ic27.11 (EnableG) <- =        PC3 <> | 11         5   18 | <> PB6     = <- ic6.12 (QC)
                                                ic5.14 (QA) -> =        PB0 <> | 12             17 | <> PB5     = <- ic6.13 (QB)
                                                ic5.13 (QB) -> =        PB1 <> | 13             16 | <> PB4     = <- ic6.14 (QA)
                                                ic5.12 (QC) -> =        PB2 <> | 14             15 | <> PB3     = <- ic5.11 (QD)
                                                                               +-------------------+

The ic26 semaphores and the /int line:
ic26 is a 74ls74 with two latches with positive edge triggering:
latch 1 aka m_z80HasWritten:
/Reset  : <- system reset generator (watchdog?)
Data    : tied to GND on an inner plane
Clock   : <- 68705 PC2, triggered on rising edge, this clears the bit and the /INT
/Preset : from z80, somehow.
Q       : -> 68705 PC0
/Q      : -> 68705 /INT and z80, somehow

latch 2 aka m_68705HasWritten:
/Reset  : <- system reset generator (watchdog?)
Data    : tied to GND on an inner plane
Clock   : from z80, somehow; also is /oe of ic27
/Preset : <- 68705 PC3
Q       : N/C
/Q      : -> 68705 PC1 and z80, somehow

Note: despite having a signal on it (possibly vblank or one of the V counter bits), the TIMER/BOOT pin is seemingly never used by the MCU in input mode, so the signal is ignored.

Note2: PA0 is connected to bit 0 of both the z80->68705 latch and the 68705->z80 latch. The board trace from http://zx-pk.ru is known to be incomplete, so its likely all of the PAx bits go both ways.
*/


/***************************************************************************/


/* Input Ports */

static INPUT_PORTS_START( arkanoid )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, arkanoid_state,arkanoid_68705_input_r, NULL)  /* Inputs from the 68705 */

	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("MUX")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, arkanoid_state,arkanoid_input_mux, "P1\0P2")

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(0x04, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, "20K 60K 60K+" )
	PORT_DIPSETTING(    0x00, "20K" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW1:3")   /* Table at 0x9a28 */
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW1:1,2") /* Table at 0x0328 */
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )

	PORT_START("UNUSED")    /* This is read in ay8910_interface */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")        /* Spinner Player 1 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15)

	PORT_START("P2")        /* Spinner Player 2  */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_COCKTAIL
INPUT_PORTS_END

/* Different coinage and additional "Cabinet" Dip Switch */
static INPUT_PORTS_START( arkanoidj )
	PORT_INCLUDE( arkanoid )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:2") /* Table at 0x0320 */
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static INPUT_PORTS_START( ark1ball )
	PORT_INCLUDE( arkanoidj )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:4") /* "ld a,$60" at 0x93bd and "ld a,$60" at 0x9c7f and 0x9c9b */
	PORT_DIPSETTING(    0x10, "60K 100K 60K+" )
	PORT_DIPSETTING(    0x00, "60K" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:3") /* Table at 0x9a28 */
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x00, "2" )
INPUT_PORTS_END

/* Bootlegs do not read from the MCU */
static INPUT_PORTS_START( arkatayt )
	PORT_INCLUDE( arkanoidj )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SPECIAL )        /* Some bootlegs need it to be 1 */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )       /* Some bootlegs need it to be 0 */
INPUT_PORTS_END

static INPUT_PORTS_START( arkangc )
	PORT_INCLUDE( arkatayt )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Ball Speed" )            PORT_DIPLOCATION("SW1:8") /* Speed at 0xc462 (code at 0x18aa) - Also affects level 2 (code at 0x7b82) */
	PORT_DIPSETTING(    0x01, DEF_STR( Normal ) )       /* 0xc462 = 0x06 - Normal level 2 */
	PORT_DIPSETTING(    0x00, "Faster" )                /* 0xc462 = 0x08 - Level 2 same as level 30 */
INPUT_PORTS_END

static INPUT_PORTS_START( arkangc2 )
	PORT_INCLUDE( arkatayt )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Ball Speed" )            PORT_DIPLOCATION("SW1:8") /* Speed at 0xc462 (code at 0x18aa) - Also affects level 2 (code at 0x7b82) */
	PORT_DIPSETTING(    0x01, "Slower" )                /* 0xc462 = 0x04 - Normal level 2 */
	PORT_DIPSETTING(    0x00, DEF_STR ( Normal ) )      /* 0xc462 = 0x06 - Level 2 same as level 30 */
INPUT_PORTS_END

static INPUT_PORTS_START( block2 )
	PORT_INCLUDE( arkatayt )

	PORT_MODIFY("DSW")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW1:8" )        /* Speed at 0xc462 (code at 0x06fc) : always = 0x06 */
INPUT_PORTS_END

static INPUT_PORTS_START( arkgcbl )
	PORT_INCLUDE( arkatayt )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x00, "Round Select" )          PORT_DIPLOCATION("SW1:8") /* Check code at 0x7bc2 - Speed at 0xc462 (code at 0x18aa) */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )          /* 0xc462 = 0x06 */
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )           /* 0xc462 = 0x06 */
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:4") /* "ld a,$60" at 0x93bd and "ld a,$20" at 0x9c7f and 0x9c9b */
	PORT_DIPSETTING(    0x10, "60K 100K 60K+" )         /* But "20K 60K 60K+" when continue */
	PORT_DIPSETTING(    0x00, "60K" )                   /* But "20K" when continue */
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:2" )        /* Always 2C_1C - check code at 0x7d5e */
INPUT_PORTS_END

static INPUT_PORTS_START( paddle2 )
	PORT_INCLUDE( arkatayt )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x00, "Round Select" )          PORT_DIPLOCATION("SW1:8") /* Check code at 0x7bc2 - Speed at 0xc462 (code at 0x18aa) */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )          /* 0xc462 = 0x06 */
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )           /* 0xc462 = 0x06 */
	PORT_DIPNAME( 0x04, 0x04, "Controls ?" )            PORT_DIPLOCATION("SW1:6") /* Check code at 0x96a1 and read notes */
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Alternate ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:4") /* "ld a,$60" at 0x93bd and "ld a,$20" at 0x9c7f and 0x9c9b */
	PORT_DIPSETTING(    0x10, "60K 100K 60K+" )         /* But "20K 60K 60K+" when continue */
	PORT_DIPSETTING(    0x00, "60K" )                   /* But "20K" when continue */
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:3") /* Table at 0x9a28 */
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x20, "3" )
INPUT_PORTS_END

static INPUT_PORTS_START( arktayt2 )
	PORT_INCLUDE( arkatayt )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:4") /* "ld a,$60" at 0x93bd and "ld a,$60" at 0x9c7f and 0x9c9b */
	PORT_DIPSETTING(    0x10, "60K 100K 60K+" )
	PORT_DIPSETTING(    0x00, "60K" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:3") /* Table at 0x9a28 */
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:2") /* Table at 0x0320 */
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
INPUT_PORTS_END


static INPUT_PORTS_START( tetrsark )
	PORT_START("SYSTEM")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BUTTONS")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("MUX")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	/* Inputs are read by the ay8910. For simplicity, we use tags from other sets (even if not appropriate) */
	PORT_START("DSW")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) // or up? it rotates the piece.
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Cabinet ) )      /* Also affects numbers of players - read notes */
	PORT_DIPSETTING(    0x10, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coinage ) )      /* Table at 0x0207 */
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )

	PORT_START("UNUSED")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) // or up? it rotates the piece.
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )
INPUT_PORTS_END


static INPUT_PORTS_START( hexa )
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x04, 0x00, "Naughty Pics" )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Difficulty?" )  PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x30, "Easy?" )
	PORT_DIPSETTING(    0x20, "Medium?" )
	PORT_DIPSETTING(    0x10, "Hard?" )
	PORT_DIPSETTING(    0x00, "Hardest?" )
	PORT_DIPNAME( 0x40, 0x40, "Pobys" )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( brixian )
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x02, "Time Left" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "More" )
	PORT_DIPSETTING(    0x01, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, "Normal (dupe)" )
	PORT_DIPSETTING(    0x03, "Less" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown )  ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Speed of Elevator" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPSETTING(    0x08, "Slow" )
	PORT_DIPNAME( 0xf0, 0x10, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0xf0, "2" )
	PORT_DIPSETTING(    0x70, "3" )
	PORT_DIPSETTING(    0x30, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "6" )
INPUT_PORTS_END

/***************************************************************************/

/* Graphics Layouts */

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	4096,   /* 4096 characters */
	3,  /* 3 bits per pixel */
	{ 2*4096*8*8, 4096*8*8, 0 },    /* the two bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

/* Graphics Decode Information */

static GFXDECODE_START( arkanoid )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,  0, 64 )
	// sprites use the same characters above, but are 16x8
GFXDECODE_END

static GFXDECODE_START( hexa )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout,  0 , 32 )
GFXDECODE_END

/* Machine Drivers */

void arkanoid_state::machine_start()
{
	// allocate the MCU timer, even if we have no MCU, and set it to fire NEVER.
	m_68705_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(arkanoid_state::timer_68705_increment),this));
	m_68705_timer->adjust(attotime::never);

	save_item(NAME(m_gfxbank));
	save_item(NAME(m_palettebank));

	save_item(NAME(m_paddle_select));

	save_item(NAME(m_bootleg_id));
	save_item(NAME(m_bootleg_cmd));

	save_item(NAME(m_z80HasWritten));
	save_item(NAME(m_fromz80));
	save_item(NAME(m_68705HasWritten));
	save_item(NAME(m_toz80));

	save_item(NAME(m_port_a_in));
	save_item(NAME(m_port_a_out));
	save_item(NAME(m_ddr_a));
	save_item(NAME(m_port_c_internal));
	save_item(NAME(m_port_c_out));
	save_item(NAME(m_ddr_c));
	save_item(NAME(m_tdr));
	save_item(NAME(m_tcr));
}

void arkanoid_state::machine_reset()
{
	m_gfxbank = 0;
	m_palettebank = 0;

	m_paddle_select = 0;

	m_bootleg_cmd = 0;

	// latched data bytes
	m_fromz80 = 0;
	m_toz80 = 0;
	// the following 3 are all part of the 74ls74 at ic26 and are cleared on reset
	m_z80HasWritten = 0;
	m_68705HasWritten = 0;
	if (m_mcu.found()) m_mcu->set_input_line(M68705_IRQ_LINE, CLEAR_LINE);
	if (m_mcu.found()) m_68705_timer->adjust(attotime::from_hz(((XTAL_12MHz/4)/4)/(1<<7)));

	m_port_a_in = 0;
	m_port_a_out = 0;
	m_ddr_a = 0;
	m_port_c_internal = 0;
	m_port_c_out = 0;
	m_ddr_c = 0;
	m_tdr = 0xFF;
	m_tcr = 0x7F;
}

/*
Pixel clock: 3 MHz = 192 HTotal, assuming it's 6 MHz
*/
#define ARKANOID_PIXEL_CLOCK XTAL_12MHz/2
#define ARKANOID_HTOTAL 384
#define ARKANOID_HBEND 0
#define ARKANOID_HBSTART 256
#define ARKANOID_VTOTAL 264
#define ARKANOID_VBEND 16
#define ARKANOID_VBSTART 240

static MACHINE_CONFIG_START( arkanoid, arkanoid_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_12MHz/2) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(arkanoid_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", arkanoid_state,  irq0_line_hold)

	MCFG_CPU_ADD("mcu", M68705, XTAL_12MHz/4) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(mcu_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))                  // 100 CPU slices per second to synchronize between the MCU and the main CPU

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
//  MCFG_SCREEN_REFRESH_RATE(60)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
//  MCFG_SCREEN_SIZE(32*8, 32*8)
//  MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_RAW_PARAMS(ARKANOID_PIXEL_CLOCK,ARKANOID_HTOTAL,ARKANOID_HBEND,ARKANOID_HBSTART,ARKANOID_VTOTAL,ARKANOID_VBEND,ARKANOID_VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(arkanoid_state, screen_update_arkanoid)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", arkanoid)
	MCFG_PALETTE_ADD_RRRRGGGGBBBB_PROMS("palette", 512)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", YM2149, XTAL_12MHz/4) /* YM2149 clock is 3mhz, pin 26 is low so final clock is 3mhz/2, handled inside the ay core */
	MCFG_AY8910_OUTPUT_TYPE(AY8910_SINGLE_OUTPUT | YM2149_PIN26_LOW) // all outputs are tied together with no resistors, and pin 26 is low
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("UNUSED"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.66)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( hexa, arkanoid_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_12MHz/2)  /* Imported from arkanoid - correct? */
	MCFG_CPU_PROGRAM_MAP(hexa_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", arkanoid_state,  irq0_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
//  MCFG_SCREEN_REFRESH_RATE(60)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
//  MCFG_SCREEN_SIZE(32*8, 32*8)
//  MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_RAW_PARAMS(ARKANOID_PIXEL_CLOCK,ARKANOID_HTOTAL,ARKANOID_HBEND,ARKANOID_HBSTART,ARKANOID_VTOTAL,ARKANOID_VBEND,ARKANOID_VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(arkanoid_state, screen_update_hexa)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", hexa)
	MCFG_PALETTE_ADD_RRRRGGGGBBBB_PROMS("palette", 256)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, XTAL_12MHz/4/2) /* Imported from arkanoid - correct? */
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("INPUTS"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( hexaa, hexa )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(hexaa_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", arkanoid_state,  irq0_line_hold)

	MCFG_CPU_ADD("subcpu", Z80, XTAL_12MHz/2) // ?
	MCFG_CPU_PROGRAM_MAP(hexaa_sub_map)
	MCFG_CPU_IO_MAP(hexaa_sub_iomap)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( bootleg, arkanoid )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(bootleg_map)

	MCFG_DEVICE_REMOVE("mcu")
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( brixian, arkanoid_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_12MHz/2)
	MCFG_CPU_PROGRAM_MAP(brixian_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", arkanoid_state,  irq0_line_hold)

	/* there is a 68705 but it's only role appears to be to copy data to RAM at startup */
	/* the RAM is also battery backed, making the 68705 almost redundant as long as the battery doesn't die(!) */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
//  MCFG_SCREEN_REFRESH_RATE(60)
//  MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
//  MCFG_SCREEN_SIZE(32*8, 32*8)
//  MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_RAW_PARAMS(ARKANOID_PIXEL_CLOCK,ARKANOID_HTOTAL,ARKANOID_HBEND,ARKANOID_HBSTART,ARKANOID_VTOTAL,ARKANOID_VBEND,ARKANOID_VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(arkanoid_state, screen_update_hexa)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", arkanoid)
	MCFG_PALETTE_ADD_RRRRGGGGBBBB_PROMS("palette", 512)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, XTAL_12MHz/4/2) /* Imported from arkanoid - correct? */
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("INPUTS"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



/***************************************************************************/

/* ROMs */
/* rom numbering, with guesses for version numbers and missing roms:
    A75 01   = Z80 code 1/2 v1.0 Japan (NOT DUMPED)
    A75 01-1 = Z80 code 1/2 v1.1 Japan and USA/Romstar
    A75 02   = Z80 code 2/2 v1.0 Japan
    A75 03   = GFX 1/3
    A75 04   = GFX 2/3
    A75 05   = GFX 3/3
    A75 06   = MC68705P5 MCU code, v1.x Japan and v1.x USA/Romstar (verified to have crc&sha1 of 0be83647 and 625fd1e6061123df612f115ef14a06cd6009f5d1; the rom with crc&sha1 of 4e44b50a and c61e7d158dc8e2b003c8158053ec139b904599af is also probably legit as well, only differing due to a different fill in an unused area from the verified one )
    A75 07   = PROM red
    A75 08   = PROM green
    A75 09   = PROM blue
    A75 10   = Z80 code 2/2 v1.1 USA/Romstar
    A75 11   = Z80 code 2/2 v1.2 Japan(World?) (paired with 01-1 v1.1 Japan)
    (A75 12 through 17 are unknown, could be another two sets of z80 code plus mc68705p5)
    A75 18   = Z80 code v2.0 2/2 USA/Romstar
    A75 19   = Z80 code v2.0 1/2 USA/Romstar
    A75 20   = MC68705P5 MCU code, v2.0 USA/Romstar
    A75 21   = Z80 code v2.0 1/2 Japan w/level select
    A75 22   = Z80 code v2.0 2/2 Japan w/level select
    A75 23   = MC68705P5 MCU code, v2.0 Japan w/level select
    A75 24   = Z80 code v2.1 1/2 Japan
    A75 25   = Z80 code v2.1 2/2 Japan
    A75 26   = MC68705P5 MCU code, v2.1 Japan
    A75 27   = Z80 code 1/2 Tournament
    A75 28   = Z80 code 2/2 Tournament
    A75 29   = GFX 1/3 Tournament
    A75 30   = GFX 2/3 Tournament
    A75 31   = GFX 3/3 Tournament
    A75 32   = MC68705P5 MCU code, Tournament
    A75 33   = PROM red Tournament
    A75 34   = PROM green Tournament
    A75 35   = PROM blue Tournament
    A75 36   = Z80 code 1/2 (Tournament v2.0?) (NOT DUMPED)
    A75 37   = Z80 code 2/2 (Tournament v2.0?) (NOT DUMPED)
*/

ROM_START( arkanoid ) // v1.2 Japan(world?)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a75-01-1.ic17", 0x0000, 0x8000, CRC(5bcda3b0) SHA1(52cadd38b5f8e8856f007a9c602d6b508f30be65) )
	ROM_LOAD( "a75-11.ic16",   0x8000, 0x8000, CRC(eafd7191) SHA1(d2f8843b716718b1de209e97a874e8ce600f3f87) )

	ROM_REGION( 0x0800, "mcu", 0 )  /* 2k for the microcontroller */
	ROM_LOAD( "a75__06.ic14",   0x0000, 0x0800, CRC(0be83647) SHA1(625fd1e6061123df612f115ef14a06cd6009f5d1) ) // verified authentic v1.x MCU from Taito/Romstar Board

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "a75-03.ic64",   0x00000, 0x8000, CRC(038b74ba) SHA1(ac053cc4908b4075f918748b89570e07a0ba5116) )
	ROM_LOAD( "a75-04.ic63",   0x08000, 0x8000, CRC(71fae199) SHA1(5d253c46ccf4cd2976a5fb8b8713f0f345443d06) )
	ROM_LOAD( "a75-05.ic62",   0x10000, 0x8000, CRC(c76374e2) SHA1(7520dd48de20db60a2038f134dcaa454988e7874) )

	ROM_REGION( 0x0600, "proms", 0 ) /* BPROMs are silkscreened as 7621, actual BPROMs used are MMI 6306-1N */
	ROM_LOAD( "a75-07.ic24",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) )  /* red component */
	ROM_LOAD( "a75-08.ic23",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) )  /* green component */
	ROM_LOAD( "a75-09.ic22",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) )  /* blue component */

	// these were decapped, sort them!
	// All of these MCUs work in place of A75 06, see comments for each.
	ROM_REGION( 0x1800, "alt_mcus", 0 ) /* 2k for the microcontroller */
	ROM_LOAD( "arkanoid_mcu.ic14",       0x0800, 0x0800, CRC(4e44b50a) SHA1(c61e7d158dc8e2b003c8158053ec139b904599af) ) // This matches the legitimate Taito rom, with a "Programmed By Yasu 1986" string in it, but has a 0x00 fill after the end of the code instead of 0xFF. This matches the legit rom otherwise and may itself be legit, perhaps an artifact of a 68705 programmer at Taito using a sparse s-record/ihex file and not clearing the ram in the chip programmer to 0xFF (or 0x00?) before programming the MCU.
	ROM_LOAD( "a75-06__bootleg_68705.ic14",   0x1000, 0x0800, CRC(515d77b6) SHA1(a302937683d11f663abd56a2fd7c174374e4d7fb) ) // This was NOT decapped, it came from an unprotected bootleg, and used to be used by the main set. It is definitely a bootleg mcu with no timer or int selftest, and compltely different code altogether, probably implemented by pirates by blackbox-reverse engineering the real MCU.
	ROM_LOAD( "arkanoid1_68705p3.ic14",  0x0000, 0x0800, CRC(1b68e2d8) SHA1(f642a7cb624ee14fb0e410de5ae1fc799d2fa1c2) ) // This is the same as the 515d77b6 rom above except the bootrom (0x785-0x7f7) is intact. No other difference.
ROM_END

ROM_START( arkanoidu ) // V2.0 US/Romstar
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a75-19.ic17",   0x0000, 0x8000, CRC(d3ad37d7) SHA1(a172a1ef5bb83ee2d8ed2842ef8968af19ad411e) )
	ROM_LOAD( "a75-18.ic16",   0x8000, 0x8000, CRC(cdc08301) SHA1(05f54353cc8333af14fa985a2764960e20e8161a) )

	ROM_REGION( 0x0800, "mcu", 0 )  /* 2k for the microcontroller */
	ROM_LOAD( "a75-20.ic14",   0x0000, 0x0800, BAD_DUMP CRC(de518e47) SHA1(b8eddd1c566505fb69e3d1207c7a9720dfb9f503) ) /* Hand crafted based on the bootleg a75-06 chip, need the real data here */

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "a75-03.ic64",   0x00000, 0x8000, CRC(038b74ba) SHA1(ac053cc4908b4075f918748b89570e07a0ba5116) )
	ROM_LOAD( "a75-04.ic63",   0x08000, 0x8000, CRC(71fae199) SHA1(5d253c46ccf4cd2976a5fb8b8713f0f345443d06) )
	ROM_LOAD( "a75-05.ic62",   0x10000, 0x8000, CRC(c76374e2) SHA1(7520dd48de20db60a2038f134dcaa454988e7874) )

	ROM_REGION( 0x0600, "proms", 0 ) /* BPROMs are silkscreened as 7621, actual BPROMs used are MMI 6306-1N */
	ROM_LOAD( "a75-07.ic24",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) )  /* red component */
	ROM_LOAD( "a75-08.ic23",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) )  /* green component */
	ROM_LOAD( "a75-09.ic22",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) )  /* blue component */
ROM_END

/* Observed on a real TAITO J1100075A pcb (with K1100181A sticker), pcb is white painted, and has a "ROMSTAR(C) // All Rights Reserved // Serial No. // No 14128" sticker */
ROM_START( arkanoiduo ) // V1.1 USA/Romstar
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Silkscreen: "IC17 27256" and "IC16 27256" */
	ROM_LOAD( "a75__01-1.ic17", 0x0000, 0x8000, CRC(5bcda3b0) SHA1(52cadd38b5f8e8856f007a9c602d6b508f30be65) )
	ROM_LOAD( "a75__10.ic16",   0x8000, 0x8000, CRC(a1769e15) SHA1(fbb45731246a098b29eb08de5d63074b496aaaba) )

	ROM_REGION( 0x0800, "mcu", 0 )  /* 2k for the microcontroller */
	ROM_LOAD( "a75__06.ic14",   0x0000, 0x0800, CRC(0be83647) SHA1(625fd1e6061123df612f115ef14a06cd6009f5d1) ) // verified authentic v1.x MCU from Taito/Romstar Board

	ROM_REGION( 0x18000, "gfx1", 0 ) /* Silkscreen: "IC62 27128/256", "IC63 27128/256", "IC64 27128/256" */
	ROM_LOAD( "a75__03.ic64",   0x00000, 0x8000, CRC(038b74ba) SHA1(ac053cc4908b4075f918748b89570e07a0ba5116) )
	ROM_LOAD( "a75__04.ic63",   0x08000, 0x8000, CRC(71fae199) SHA1(5d253c46ccf4cd2976a5fb8b8713f0f345443d06) )
	ROM_LOAD( "a75__05.ic62",   0x10000, 0x8000, CRC(c76374e2) SHA1(7520dd48de20db60a2038f134dcaa454988e7874) )

	ROM_REGION( 0x0600, "proms", 0 ) /* Silkscreen: "IC22 7621", "IC23 7621", "IC24 7621", but the actual BPROMs used are either MMI 6306-1N or Fairchild MB7116E */
	ROM_LOAD( "a75-07.ic24",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) )  /* Chip Silkscreen: "A75-07"; red component */
	ROM_LOAD( "a75-08.ic23",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) )  /* Chip Silkscreen: "A75-08"; green component */
	ROM_LOAD( "a75-09.ic22",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) )  /* Chip Silkscreen: "A75-09"; blue component */

	ROM_REGION( 0x8000, "altgfx", 0 )
	ROM_LOAD( "a75__03(alternate).ic64",   0x00000, 0x8000, CRC(983d4485) SHA1(603a8798d1f531a70a527a5c6122f0ffd6adcfb6) ) // this was found on a legit v1.1 Romstar USA pcb with serial number 29342; the only difference seems to be the first 32 tiles are all 0xFF instead of 0x00. Those tiles don't seem to be used by the game at all. This is likely another incidence of "Taito forgot to clear programmer ram before burning a rom from a sparse s-record/ihex file"
ROM_END

ROM_START( arkanoidj ) // V2.1 Japan
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a75_24.ic17",   0x0000, 0x8000, CRC(3f2b27e9) SHA1(656035f5292d6921448e74d3e1abab57b46e7d9e) )
	ROM_LOAD( "a75_25.ic16",   0x8000, 0x8000, CRC(c13b2038) SHA1(0b8197b48e57ffe9ccad0ebbc24891d1da7c9880) )

	ROM_REGION( 0x0800, "mcu", 0 )  /* 2k for the microcontroller */
	ROM_LOAD( "a75-26.ic14",  0x0000, 0x0800, BAD_DUMP CRC(962960d4) SHA1(64b065a54b1658364db569ac06b717eb7bdd186e) ) /* Hand crafted based on the bootleg a75-06 chip, need the real data here */

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "a75-03.ic64",   0x00000, 0x8000, CRC(038b74ba) SHA1(ac053cc4908b4075f918748b89570e07a0ba5116) )
	ROM_LOAD( "a75-04.ic63",   0x08000, 0x8000, CRC(71fae199) SHA1(5d253c46ccf4cd2976a5fb8b8713f0f345443d06) )
	ROM_LOAD( "a75-05.ic62",   0x10000, 0x8000, CRC(c76374e2) SHA1(7520dd48de20db60a2038f134dcaa454988e7874) )

	ROM_REGION( 0x0600, "proms", 0 ) /* BPROMs are silkscreened as 7621, actual BPROMs used are MMI 6306-1N */
	ROM_LOAD( "a75-07.ic24",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) )  /* red component */
	ROM_LOAD( "a75-08.ic23",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) )  /* green component */
	ROM_LOAD( "a75-09.ic22",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) )  /* blue component */
ROM_END

ROM_START( arkanoidja ) // V2.0 Japan w/level select
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a75-21.ic17",   0x0000, 0x8000, CRC(bf0455fc) SHA1(250522b84b9f491c3f4efc391bf6aa6124361369) )
	ROM_LOAD( "a75-22.ic16",   0x8000, 0x8000, CRC(3a2688d3) SHA1(9633a661352def3d85f95ca830f6d761b0b5450e) )

	ROM_REGION( 0x0800, "mcu", 0 )  /* 2k for the microcontroller */
	// the handcrafted value at 0x351 (0x9ddb) seems incorrect compared to other sets? (but it appears the value is never used, and the data it would usually point to does not exist in the program rom?)
	ROM_LOAD( "a75-23.ic14",  0x0000, 0x0800, BAD_DUMP CRC(0a4abef6) SHA1(fdce0b7a2eab7fd4f1f4fc3b93120b1ebc16078e)  ) /* Hand crafted based on the bootleg a75-06 chip, need the real data here */

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "a75-03.ic64",   0x00000, 0x8000, CRC(038b74ba) SHA1(ac053cc4908b4075f918748b89570e07a0ba5116) )
	ROM_LOAD( "a75-04.ic63",   0x08000, 0x8000, CRC(71fae199) SHA1(5d253c46ccf4cd2976a5fb8b8713f0f345443d06) )
	ROM_LOAD( "a75-05.ic62",   0x10000, 0x8000, CRC(c76374e2) SHA1(7520dd48de20db60a2038f134dcaa454988e7874) )

	ROM_REGION( 0x0600, "proms", 0 ) /* BPROMs are silkscreened as 7621, actual BPROMs used are MMI 6306-1N */
	ROM_LOAD( "a75-07.ic24",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) )  /* red component */
	ROM_LOAD( "a75-08.ic23",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) )  /* green component */
	ROM_LOAD( "a75-09.ic22",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) )  /* blue component */
ROM_END

ROM_START( arkanoidjb ) // V1.1 Japan
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a75-01-1.ic17", 0x0000, 0x8000, CRC(5bcda3b0) SHA1(52cadd38b5f8e8856f007a9c602d6b508f30be65) )
	ROM_LOAD( "a75-02.ic16",   0x8000, 0x8000, CRC(bbc33ceb) SHA1(e9b6fef98d0d20e77c7a1c25eff8e9a8c668a258) )

	ROM_REGION( 0x0800, "mcu", 0 )  /* 2k for the microcontroller */
	ROM_LOAD( "a75__06.ic14",   0x0000, 0x0800, CRC(0be83647) SHA1(625fd1e6061123df612f115ef14a06cd6009f5d1) ) // verified authentic v1.x MCU from Taito/Romstar Board

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "a75-03.ic64",   0x00000, 0x8000, CRC(038b74ba) SHA1(ac053cc4908b4075f918748b89570e07a0ba5116) )
	ROM_LOAD( "a75-04.ic63",   0x08000, 0x8000, CRC(71fae199) SHA1(5d253c46ccf4cd2976a5fb8b8713f0f345443d06) )
	ROM_LOAD( "a75-05.ic62",   0x10000, 0x8000, CRC(c76374e2) SHA1(7520dd48de20db60a2038f134dcaa454988e7874) )

	ROM_REGION( 0x0600, "proms", 0 ) /* BPROMs are silkscreened as 7621, actual BPROMs used are MMI 6306-1N */
	ROM_LOAD( "a75-07.ic24",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) )  /* red component */
	ROM_LOAD( "a75-08.ic23",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) )  /* green component */
	ROM_LOAD( "a75-09.ic22",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) )  /* blue component */
ROM_END


ROM_START( arkatour ) // Tournament version
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a75-27.ic17",   0x0000, 0x8000, CRC(e3b8faf5) SHA1(4c09478fa41881fa89ee6afb676aeb780f17ac2e) )
	ROM_LOAD( "a75-28.ic16",   0x8000, 0x8000, CRC(326aca4d) SHA1(5a194b7a0361236d471b24905dc6434372f81252) )

	ROM_REGION( 0x0800, "mcu", 0 )  /* 2k for the microcontroller */
	ROM_LOAD( "a75-32.ic14",  0x0000, 0x0800, BAD_DUMP CRC(d3249559) SHA1(b1542764450016614e9e03cedd6a2f1e59961789)  ) /* Hand crafted based on the bootleg a75-06 chip, need the real data here */

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "a75-29.ic64",   0x00000, 0x8000, CRC(5ddea3cf) SHA1(58f16515898b7cc2697bf7663a60d9ca0db6da95) )
	ROM_LOAD( "a75-30.ic63",   0x08000, 0x8000, CRC(5fcf2e85) SHA1(f721f0afb0550cc64bff26681856a7576398d9b5) )
	ROM_LOAD( "a75-31.ic62",   0x10000, 0x8000, CRC(7b76b192) SHA1(a68aa08717646a6c322cf3455df07f50df9e9f33) )

	ROM_REGION( 0x0600, "proms", 0 ) /* BPROMs are silkscreened as 7621, actual BPROMs used are MMI 6306-1N */
	ROM_LOAD( "a75-33.ic24",    0x0000, 0x0200, CRC(b4bf3c81) SHA1(519188937ac9728c653fabac877e37dc43c3f71a) )  /* red component */
	ROM_LOAD( "a75-34.ic23",    0x0200, 0x0200, CRC(de85a803) SHA1(325214995996de36a0470fbfc00e4e393c0b17ad) )  /* green component */
	ROM_LOAD( "a75-35.ic22",    0x0400, 0x0200, CRC(38acfd3b) SHA1(2841e9db047aa039eff8567a518b6250b355507b) )  /* blue component */
ROM_END

// Everything from here on is bootlegs

ROM_START( arkanoidjbl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "e1.6d",        0x0000, 0x8000, CRC(dd4f2b72) SHA1(399a8636030a702dafc1da926f115df6f045bef1) ) /* Hacked up Notice warning text */
	ROM_LOAD( "e2.6f",        0x8000, 0x8000, CRC(bbc33ceb) SHA1(e9b6fef98d0d20e77c7a1c25eff8e9a8c668a258) ) /* == A75-02.IC16 */

	ROM_REGION( 0x0800, "mcu", 0 )  /* 2k for the microcontroller */
	ROM_LOAD( "68705p3.6i",   0x0000, 0x0800, CRC(389a8cfb) SHA1(9530c051b61b5bdec7018c6fdc1ea91288a406bd) ) // This set had an unprotected mcu with a bootlegged copy of the real Taito a75__06.ic14 code, unlike the other bootlegs. It has the bootstrap code missing and the security bit cleared, the area after the rom filled with 0x00, and the verify mode disable jump removed. Otherwise it matches a75__06.ic14

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "a75-03.rom",   0x00000, 0x8000, CRC(038b74ba) SHA1(ac053cc4908b4075f918748b89570e07a0ba5116) )
	ROM_LOAD( "a75-04.rom",   0x08000, 0x8000, CRC(71fae199) SHA1(5d253c46ccf4cd2976a5fb8b8713f0f345443d06) )
	ROM_LOAD( "a75-05.rom",   0x10000, 0x8000, CRC(c76374e2) SHA1(7520dd48de20db60a2038f134dcaa454988e7874) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "a75-07.bpr",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) )   /* red component */
	ROM_LOAD( "a75-08.bpr",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) )   /* green component */
	ROM_LOAD( "a75-09.bpr",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) )   /* blue component */

ROM_END

ROM_START( arkanoidjbl2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.ic81", 0x0000, 0x8000, CRC(9ff93dc2) SHA1(eee0975b799a8e6717f646dd40716dc454476106) )
	ROM_LOAD( "2.ic82", 0x8000, 0x8000, CRC(bbc33ceb) SHA1(e9b6fef98d0d20e77c7a1c25eff8e9a8c668a258) ) /* == A75-02.IC16 */

	ROM_REGION( 0x0800, "mcu", 0 )  /* 2k for the microcontroller */
	ROM_LOAD( "a75-06__bootleg_68705.ic14",  0x0000, 0x0800, BAD_DUMP CRC(515d77b6) SHA1(a302937683d11f663abd56a2fd7c174374e4d7fb) ) /* Uses the bootleg MCU? Not sure what mcu is supposed to go with this set... */

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "a75-03.ic64",   0x00000, 0x8000, CRC(038b74ba) SHA1(ac053cc4908b4075f918748b89570e07a0ba5116) )
	ROM_LOAD( "a75-04.ic63",   0x08000, 0x8000, CRC(71fae199) SHA1(5d253c46ccf4cd2976a5fb8b8713f0f345443d06) )
	ROM_LOAD( "a75-05.ic62",   0x10000, 0x8000, CRC(c76374e2) SHA1(7520dd48de20db60a2038f134dcaa454988e7874) )

	ROM_REGION( 0x0600, "proms", 0 ) /* BPROMs are silkscreened as 7621, actual BPROMs used are MMI 6306-1N */
	ROM_LOAD( "a75-07.ic24",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) )  /* red component */
	ROM_LOAD( "a75-08.ic23",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) )  /* green component */
	ROM_LOAD( "a75-09.ic23",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) )  /* blue component */
ROM_END

ROM_START( ark1ball ) /* This set requires a MCU. No MCU rom was supplied so we use current A75-06.IC14 for now */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a-1.7d",       0x0000, 0x8000, CRC(dd4f2b72) SHA1(399a8636030a702dafc1da926f115df6f045bef1) )
	ROM_LOAD( "2palline.7f",  0x8000, 0x8000, CRC(ed6b62ab) SHA1(4d4991b422756bd304fc5ef236aac1422fe1f999) )

	/* Use the current A75-06.IC14 MCU code so the game is playable */
	ROM_REGION( 0x0800, "mcu", 0 )  /* 2k for the microcontroller */
	ROM_LOAD( "a75-06__bootleg_68705.ic14",  0x0000, 0x0800, BAD_DUMP CRC(515d77b6) SHA1(a302937683d11f663abd56a2fd7c174374e4d7fb) ) /* Uses the bootleg MCU? Not sure what mcu is supposed to go with this set... */

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "a-3.3a",       0x00000, 0x8000, CRC(038b74ba) SHA1(ac053cc4908b4075f918748b89570e07a0ba5116) )
	ROM_LOAD( "a-4.3d",       0x08000, 0x8000, CRC(71fae199) SHA1(5d253c46ccf4cd2976a5fb8b8713f0f345443d06) )
	ROM_LOAD( "a-5.3f",       0x10000, 0x8000, CRC(c76374e2) SHA1(7520dd48de20db60a2038f134dcaa454988e7874) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "a75-07.bpr",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) )   /* red component */
	ROM_LOAD( "a75-08.bpr",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) )   /* green component */
	ROM_LOAD( "a75-09.bpr",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) )   /* blue component */
ROM_END

ROM_START( arkangc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "arkgc.1",      0x0000, 0x8000, CRC(c54232e6) SHA1(beb759cee68009a06824b755d2aa26d7d436b5b0) )
	ROM_LOAD( "arkgc.2",      0x8000, 0x8000, CRC(9f0d4754) SHA1(731c9224616a338084edd6944c754d68eabba7f2) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "a75-03.rom",   0x00000, 0x8000, CRC(038b74ba) SHA1(ac053cc4908b4075f918748b89570e07a0ba5116) )
	ROM_LOAD( "a75-04.rom",   0x08000, 0x8000, CRC(71fae199) SHA1(5d253c46ccf4cd2976a5fb8b8713f0f345443d06) )
	ROM_LOAD( "a75-05.rom",   0x10000, 0x8000, CRC(c76374e2) SHA1(7520dd48de20db60a2038f134dcaa454988e7874) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "a75-07.bpr",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) )   /* red component */
	ROM_LOAD( "a75-08.bpr",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) )   /* green component */
	ROM_LOAD( "a75-09.bpr",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) )   /* blue component */
ROM_END

ROM_START( arkangc2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.81",         0x0000, 0x8000, CRC(bd6eb996) SHA1(a048ff01156166595dca0b6bee46344f7db548a8) )
	ROM_LOAD( "2.82",         0x8000, 0x8000, CRC(29dbe452) SHA1(b99cb98549bddf1e673e2e715c80664001581f9f) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "a75-03.rom",   0x00000, 0x8000, CRC(038b74ba) SHA1(ac053cc4908b4075f918748b89570e07a0ba5116) )
	ROM_LOAD( "a75-04.rom",   0x08000, 0x8000, CRC(71fae199) SHA1(5d253c46ccf4cd2976a5fb8b8713f0f345443d06) )
	ROM_LOAD( "a75-05.rom",   0x10000, 0x8000, CRC(c76374e2) SHA1(7520dd48de20db60a2038f134dcaa454988e7874) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "a75-07.bpr",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) )   /* red component */
	ROM_LOAD( "a75-08.bpr",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) )   /* green component */
	ROM_LOAD( "a75-09.bpr",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) )   /* blue component */
ROM_END

ROM_START( block2 )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "1.bin",         0x00000, 0x8000, CRC(2b026cae) SHA1(73d1d5d3e6d65fbe378ce85ff501610573ae5e95) )
	ROM_LOAD( "2.bin",         0x08000, 0x8000, CRC(e3843fea) SHA1(8c654dcf78d9e4f4c6a7a7d384fdf622536234c1) )

	ROM_REGION( 0x8000, "unknown", 0 )  /* is it more data or something else like sound or palette ? not Z80 code nor levels anyway */
	ROM_LOAD( "3.bin",         0x00000, 0x8000, CRC(e336c219) SHA1(e1dce37727e7084a83e73f15a138312ab6224061) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "4.bin",   0x00000, 0x8000, CRC(6d2c6123) SHA1(26f32099d363ab2c8505722513638b827e49a8fc) )
	ROM_LOAD( "5.bin",   0x08000, 0x8000, CRC(09a1f9d9) SHA1(c7e21aba6efb51c5501aa1428f6d9a817cb86555) )
	ROM_LOAD( "6.bin",   0x10000, 0x8000, CRC(dfb9f7e2) SHA1(8d938ee6f8dcac0a564d5fa7cd5da34e0db07c71) )

	// no proms were present in this set.. assumed to be the same
	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "a75-07.bpr",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) )   /* red component */
	ROM_LOAD( "a75-08.bpr",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) )   /* green component */
	ROM_LOAD( "a75-09.bpr",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) )   /* blue component */
ROM_END

// this set has the same 'space invader' scrambled block gfx, and unknown (sound?) rom as the one above
//  sadly no mention of what chip it might be for in the readme.
ROM_START( arkbloc3 )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "blockbl.001",         0x00000, 0x8000, CRC(bf7197a0) SHA1(4fbc0cbc09d292ab0f2e4a35b30505b2f7e4dc0d) )
	ROM_LOAD( "blockbl.002",         0x08000, 0x8000, CRC(29dbe452) SHA1(b99cb98549bddf1e673e2e715c80664001581f9f) )

	ROM_REGION( 0x8000, "unknown", 0 )  /* is it more data or something else like sound or palette ? not Z80 code nor levels anyway */
	ROM_LOAD( "blockbl.006",         0x00000, 0x8000, CRC(e336c219) SHA1(e1dce37727e7084a83e73f15a138312ab6224061) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "blockbl.003",   0x00000, 0x8000, CRC(6d2c6123) SHA1(26f32099d363ab2c8505722513638b827e49a8fc) )
	ROM_LOAD( "blockbl.004",   0x08000, 0x8000, CRC(09a1f9d9) SHA1(c7e21aba6efb51c5501aa1428f6d9a817cb86555) )
	ROM_LOAD( "blockbl.005",   0x10000, 0x8000, CRC(dfb9f7e2) SHA1(8d938ee6f8dcac0a564d5fa7cd5da34e0db07c71) )

	// no proms were present in this set.. assumed to be the same
	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "a75-07.bpr",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) )   /* red component */
	ROM_LOAD( "a75-08.bpr",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) )   /* green component */
	ROM_LOAD( "a75-09.bpr",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) )   /* blue component */
ROM_END

ROM_START( arkblock )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ark-6.bin",    0x0000, 0x8000, CRC(0be015de) SHA1(f4209085b59d2c96a62ac9657c7bf097da55362b) )
	ROM_LOAD( "arkgc.2",      0x8000, 0x8000, CRC(9f0d4754) SHA1(731c9224616a338084edd6944c754d68eabba7f2) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "a75-03.rom",   0x00000, 0x8000, CRC(038b74ba) SHA1(ac053cc4908b4075f918748b89570e07a0ba5116) )
	ROM_LOAD( "a75-04.rom",   0x08000, 0x8000, CRC(71fae199) SHA1(5d253c46ccf4cd2976a5fb8b8713f0f345443d06) )
	ROM_LOAD( "a75-05.rom",   0x10000, 0x8000, CRC(c76374e2) SHA1(7520dd48de20db60a2038f134dcaa454988e7874) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "a75-07.bpr",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) )   /* red component */
	ROM_LOAD( "a75-08.bpr",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) )   /* green component */
	ROM_LOAD( "a75-09.bpr",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) )   /* blue component */
ROM_END

ROM_START( arkbloc2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "block01.bin",  0x0000, 0x8000, CRC(5be667e1) SHA1(fbc5c97d836c404a2e6c007c3836e36b52ae75a1) )
	ROM_LOAD( "block02.bin",  0x8000, 0x8000, CRC(4f883ef1) SHA1(cb090a57fc75f17a3e2ba637f0e3ec93c1d02cea) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "a75-03.rom",   0x00000, 0x8000, CRC(038b74ba) SHA1(ac053cc4908b4075f918748b89570e07a0ba5116) )
	ROM_LOAD( "a75-04.rom",   0x08000, 0x8000, CRC(71fae199) SHA1(5d253c46ccf4cd2976a5fb8b8713f0f345443d06) )
	ROM_LOAD( "a75-05.rom",   0x10000, 0x8000, CRC(c76374e2) SHA1(7520dd48de20db60a2038f134dcaa454988e7874) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "a75-07.bpr",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) )   /* red component */
	ROM_LOAD( "a75-08.bpr",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) )   /* green component */
	ROM_LOAD( "a75-09.bpr",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) )   /* blue component */
ROM_END


/* arkgcbl - dump from citylan:

Anno 1986
N.revisione 06.09.86
CPU:
1x MK3880N-4-Z80CPU (main)
1x AY-3-8910A (sound)
1x oscillator 12.000MHz
ROMs:
5x TNS27256JL
5x PROM N82S129N
1x PROM MMI63S141N
Note:
1x 28x2 EDGE connector (not Jamma)
1x trimmer (volume)
1x 8 switches dip
Dumped 19/03/2006 */

ROM_START( arkgcbl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "16.6e",        0x0000, 0x8000, CRC(b0f73900) SHA1(2c9a36cc1d2a3f33ec81d63c1c325554b818d2d3) )
	ROM_LOAD( "17.6f",        0x8000, 0x8000, CRC(9827f297) SHA1(697874e73e045eb5a7bf333d7310934b239c0adf) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "a75-03.rom",   0x00000, 0x8000, CRC(038b74ba) SHA1(ac053cc4908b4075f918748b89570e07a0ba5116) )
	ROM_LOAD( "a75-04.rom",   0x08000, 0x8000, CRC(71fae199) SHA1(5d253c46ccf4cd2976a5fb8b8713f0f345443d06) )
	ROM_LOAD( "a75-05.rom",   0x10000, 0x8000, CRC(c76374e2) SHA1(7520dd48de20db60a2038f134dcaa454988e7874) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "82s129.5k",    0x0000, 0x0100, CRC(fa70b64d) SHA1(273669d05f793cf1ee0741b175be281307fa9b5e) )    /* red component   + */
	ROM_LOAD( "82s129.5jk",   0x0100, 0x0100, CRC(cca69884) SHA1(fdcd66110c8eb901a401f8618821c7980946a511) )    /* red component   = a75-07.bpr*/
	ROM_LOAD( "82s129.5l",    0x0200, 0x0100, CRC(3e4d2bf5) SHA1(c475887302dd137d6965769070b7d55f488c1b25) )    /* green component + */
	ROM_LOAD( "82s129.5kl",   0x0300, 0x0100, CRC(085d625a) SHA1(26c96a1c1b7562fed84c31dd92fdf7829e96a9c7) )    /* green component = a75-08.bpr*/
	ROM_LOAD( "82s129.5mn",   0x0400, 0x0100, CRC(0fe0b108) SHA1(fcf27619208922345a1e42b3a219b4274f66968d) )    /* blue component  + */
	ROM_LOAD( "63s141.5m",    0x0500, 0x0100, CRC(5553f675) SHA1(c50255af8d99664b92e0bb34a527fd42ebf7e759) )    /* blue component  = a75-09.bpr*/

	ROM_REGION( 0x0200, "pal", 0 )
	ROM_LOAD( "pal16r8.5f",   0x0000, 0x0104, CRC(36471917) SHA1(d0f295a94d480b44416e66be4b480b299aad5c3c) )
ROM_END

/* this one still has the original copyright intact */
ROM_START( arkgcbla )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "k101.e7",        0x0000, 0x8000, CRC(892a556e) SHA1(10d1a92f8ab1b8184b05182a2de070b163a603e2) )
	ROM_LOAD( "k102.f7",        0x8000, 0x8000, CRC(d208d05c) SHA1(0aa99a0cb8211e7b90d681c91cc77aa7078a0ccc) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "a75-03.rom",   0x00000, 0x8000, CRC(038b74ba) SHA1(ac053cc4908b4075f918748b89570e07a0ba5116) )
	ROM_LOAD( "a75-04.rom",   0x08000, 0x8000, CRC(71fae199) SHA1(5d253c46ccf4cd2976a5fb8b8713f0f345443d06) )
	ROM_LOAD( "a75-05.rom",   0x10000, 0x8000, CRC(c76374e2) SHA1(7520dd48de20db60a2038f134dcaa454988e7874) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "82s129.5k",    0x0000, 0x0100, CRC(fa70b64d) SHA1(273669d05f793cf1ee0741b175be281307fa9b5e) )    /* red component   + */
	ROM_LOAD( "82s129.5jk",   0x0100, 0x0100, CRC(cca69884) SHA1(fdcd66110c8eb901a401f8618821c7980946a511) )    /* red component   = a75-07.bpr*/
	ROM_LOAD( "82s129.5l",    0x0200, 0x0100, CRC(3e4d2bf5) SHA1(c475887302dd137d6965769070b7d55f488c1b25) )    /* green component + */
	ROM_LOAD( "82s129.5kl",   0x0300, 0x0100, CRC(085d625a) SHA1(26c96a1c1b7562fed84c31dd92fdf7829e96a9c7) )    /* green component = a75-08.bpr*/
	ROM_LOAD( "82s129.5mn",   0x0400, 0x0100, CRC(0fe0b108) SHA1(fcf27619208922345a1e42b3a219b4274f66968d) )    /* blue component  + */
	ROM_LOAD( "63s141.5m",    0x0500, 0x0100, CRC(5553f675) SHA1(c50255af8d99664b92e0bb34a527fd42ebf7e759) )    /* blue component  = a75-09.bpr*/

	ROM_REGION( 0x0200, "pal", 0 )
	ROM_LOAD( "pal16r8.5f",   0x0000, 0x0104, CRC(36471917) SHA1(d0f295a94d480b44416e66be4b480b299aad5c3c) )
ROM_END


ROM_START( paddle2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "paddle2.16",   0x0000, 0x8000, CRC(a286333c) SHA1(0b2c9cb0df236f327413d0c541453e1ba979ea38) )
	ROM_LOAD( "paddle2.17",   0x8000, 0x8000, CRC(04c2acb5) SHA1(7ce8ba31224f705b2b6ed0200404ef5f8f688001) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "a75-03.rom",   0x00000, 0x8000, CRC(038b74ba) SHA1(ac053cc4908b4075f918748b89570e07a0ba5116) )
	ROM_LOAD( "a75-04.rom",   0x08000, 0x8000, CRC(71fae199) SHA1(5d253c46ccf4cd2976a5fb8b8713f0f345443d06) )
	ROM_LOAD( "a75-05.rom",   0x10000, 0x8000, CRC(c76374e2) SHA1(7520dd48de20db60a2038f134dcaa454988e7874) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "a75-07.bpr",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) )   /* red component */
	ROM_LOAD( "a75-08.bpr",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) )   /* green component */
	ROM_LOAD( "a75-09.bpr",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) )   /* blue component */
ROM_END

ROM_START( arkatayt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic81-v.3f",   0x0000, 0x8000, CRC(154e2c6f) SHA1(dce3ae1ca83b5071ebec96f3ae18b96abe828ce5) )
	ROM_LOAD( "ic82-w.5f",   0x8000, 0x8000, CRC(4fa8cefa) SHA1(fb825834da9c8638e6a328784922b5dc23f16564) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "1-ic33.2c",   0x00000, 0x8000, CRC(038b74ba) SHA1(ac053cc4908b4075f918748b89570e07a0ba5116) )
	ROM_LOAD( "2-ic34.3c",   0x08000, 0x8000, CRC(71fae199) SHA1(5d253c46ccf4cd2976a5fb8b8713f0f345443d06) )
	ROM_LOAD( "3-ic35.5c",   0x10000, 0x8000, CRC(c76374e2) SHA1(7520dd48de20db60a2038f134dcaa454988e7874) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "ic73.11e",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) ) /* red component */
	ROM_LOAD( "ic74.12e",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) ) /* green component */
	ROM_LOAD( "ic75.13e",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) ) /* blue component */
ROM_END

ROM_START( arktayt2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic81.3f",     0x0000, 0x8000, CRC(6e0a2b6f) SHA1(5227d7a944cb1e815f60ec87a67f7462870ff9fe) )
	ROM_LOAD( "ic82.5f",     0x8000, 0x8000, CRC(5a97dd56) SHA1(b71c7b5ced2b0eebbcc5996dd21a1bb1c2da4819) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "1-ic33.2c",   0x00000, 0x8000, CRC(038b74ba) SHA1(ac053cc4908b4075f918748b89570e07a0ba5116) )
	ROM_LOAD( "2-ic34.3c",   0x08000, 0x8000, CRC(71fae199) SHA1(5d253c46ccf4cd2976a5fb8b8713f0f345443d06) )
	ROM_LOAD( "3-ic35.5c",   0x10000, 0x8000, CRC(c76374e2) SHA1(7520dd48de20db60a2038f134dcaa454988e7874) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "ic73.11e",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) ) /* red component */
	ROM_LOAD( "ic74.12e",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) ) /* green component */
	ROM_LOAD( "ic75.13e",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) ) /* blue component */
ROM_END


ROM_START( tetrsark )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "ic17.1",      0x00000, 0x8000, CRC(1a505eda) SHA1(92f171a12cf0c326d29c244514718df04b998426) )
	ROM_LOAD( "ic16.2",      0x08000, 0x8000, CRC(157bc4df) SHA1(b2c704148e7e3ca61ab51308ee0d66ea1088bff3) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "ic64.3",      0x00000, 0x8000, CRC(c3e9b290) SHA1(6e99520606c654e531dbeb9a598cfbb443c24dff) )
	ROM_LOAD( "ic63.4",      0x08000, 0x8000, CRC(de9a368f) SHA1(ffbb2479200648da3f3e7ab7cebcdb604f6dfb3d) )
	ROM_LOAD( "ic62.5",      0x10000, 0x8000, CRC(c8e80a00) SHA1(4bee4c36ee768ae68ebc64e639fdc43f61c74f92) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "a75-07.bpr",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) )   /* red component */
	ROM_LOAD( "a75-08.bpr",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) )   /* green component */
	ROM_LOAD( "a75-09.bpr",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) )   /* blue component */
ROM_END


ROM_START( hexa )
	ROM_REGION( 0x18000, "maincpu", 0 )     /* 64k for code + 32k for banked ROM */
	ROM_LOAD( "hexa.20",      0x00000, 0x8000, CRC(98b00586) SHA1(3591a3b0486d720f0aaa9f0bf4be352cd0ffcbc7) )
	ROM_LOAD( "hexa.21",      0x10000, 0x8000, CRC(3d5d006c) SHA1(ad4eadab82024b122182eacb5a322cfd6e476a70) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "hexa.17",      0x00000, 0x8000, CRC(f6911dd6) SHA1(b12ea27ecddd60820a32d4346afab0cc9d06fa57) )
	ROM_LOAD( "hexa.18",      0x08000, 0x8000, CRC(6e3d95d2) SHA1(6399b7b5d088ceda08fdea9cf650f6b405f038e7) )
	ROM_LOAD( "hexa.19",      0x10000, 0x8000, CRC(ffe97a31) SHA1(f16b5d2b9ace09bcbbfe3dfb73db7fa377d1af7f) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "hexa.001",     0x0000, 0x0100, CRC(88a055b4) SHA1(eee86a7930d0a251f3e5c2134532cd1dede2026c) )
	ROM_LOAD( "hexa.003",     0x0100, 0x0100, CRC(3e9d4932) SHA1(9a336dba7134400312985b9902c77b4141105853) )
	ROM_LOAD( "hexa.002",     0x0200, 0x0100, CRC(ff15366c) SHA1(7feaf1c768bfe76432fb80991585e13d95960b34) )
ROM_END

/*

Hexa (alt.)

main hardware consists of.....

sub board with Z80 x2, 2 ROMs and a scratched 18 pin chip (probably a PIC)

main board has....
12MHz xtal
ay3-8910
8 position DSW x1
ROMs x4
6116 SRAM x3
82S123 PROMs x3

*/


ROM_START( hexaa )
	ROM_REGION( 0x18000, "maincpu", 0 )     /* 64k for code + 32k for banked ROM */
	ROM_LOAD( "sub1.bin",      0x00000, 0x8000, CRC(82c091fa) SHA1(e509ab4d9372f93d81df70772a4632100081ffd7) )
	ROM_LOAD( "main4.bin",     0x10000, 0x8000, CRC(3d5d006c) SHA1(ad4eadab82024b122182eacb5a322cfd6e476a70) )

	ROM_REGION( 0x18000, "subcpu", 0 )
	ROM_LOAD( "sub2.bin",      0x00000, 0x2000, CRC(c3bb9661) SHA1(e4bccb822d6eba77bb9cba75125cddb740775a2c)) // 1ST AND 2ND HALF IDENTICAL (contains just 0x55 bytes of code)


	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "main1.bin",      0x00000, 0x8000, CRC(f6911dd6) SHA1(b12ea27ecddd60820a32d4346afab0cc9d06fa57) )
	ROM_LOAD( "main2.bin",      0x08000, 0x8000, CRC(6e3d95d2) SHA1(6399b7b5d088ceda08fdea9cf650f6b405f038e7) )
	ROM_LOAD( "main3.bin",      0x10000, 0x8000, CRC(ffe97a31) SHA1(f16b5d2b9ace09bcbbfe3dfb73db7fa377d1af7f) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "hexa.001",     0x0000, 0x0100, CRC(88a055b4) SHA1(eee86a7930d0a251f3e5c2134532cd1dede2026c) )
	ROM_LOAD( "hexa.003",     0x0100, 0x0100, CRC(3e9d4932) SHA1(9a336dba7134400312985b9902c77b4141105853) )
	ROM_LOAD( "hexa.002",     0x0200, 0x0100, CRC(ff15366c) SHA1(7feaf1c768bfe76432fb80991585e13d95960b34) )
ROM_END

ROM_START( brixian )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "b1.bin",      0x00000, 0x8000, CRC(3d167d09) SHA1(1d5bd098b655b8d2f956cfcb718213915bee3e41) )
	ROM_LOAD( "e7.bin",      0x08000, 0x2000, CRC(9e3707ab) SHA1(a04fb4824239f8ed1ef1de2f3c0f9d749320b2ba) )

	ROM_REGION( 0x0800, "mcu", 0 )
	ROM_LOAD( "68705p5", 0x0000, 0x0800, NO_DUMP ) // this just provides the 0x200 bytes of code we load in the protdata region by coping it to 0xc600 on startup

	ROM_REGION( 0x200, "protdata", 0 )
	ROM_LOAD( "protdata.bin",  0x00000, 0x200, CRC(a4131c0b) SHA1(5ddbd39c26e1bc9ec5f216e399c09994a23d09a7) )


	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "b4.bin",      0x00000, 0x8000, CRC(34a7a693) SHA1(793fa6dd065a158bedcd0fdc494cc8fc793ae8be) )
	ROM_LOAD( "c4.bin",      0x08000, 0x8000, CRC(d422eda5) SHA1(4874b57ec8a8aa29937f5ccc2a734ffeb7834d8a) )
	ROM_LOAD( "e4.bin",      0x10000, 0x8000, CRC(9b2e79d6) SHA1(8a40e0ef2a792efc37ea50eec01cf3fb5a3e3215) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "n82s131n.6l", 0x0000, 0x0200, CRC(0fa51a5b) SHA1(8c5cb69fbff8a3ba90f945c35f72754f9cc8f18c) )
	ROM_LOAD( "n82s131n.6p", 0x0200, 0x0200, CRC(d833ad33) SHA1(a7c17c96a670916e7102afc94dc2f0cb0455f0ce) )
	ROM_LOAD( "n82s131n.6m", 0x0400, 0x0200, CRC(05297649) SHA1(35f99cf8dddd66e26e2110619eb46bd6ccff41df) )

ROM_END



/* Driver Initialization */

void arkanoid_state::arkanoid_bootleg_init(  )
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xf000, 0xf000, read8_delegate(FUNC(arkanoid_state::arkanoid_bootleg_f000_r),this) );
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xf002, 0xf002, read8_delegate(FUNC(arkanoid_state::arkanoid_bootleg_f002_r),this) );
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xd018, 0xd018, write8_delegate(FUNC(arkanoid_state::arkanoid_bootleg_d018_w),this) );
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xd008, 0xd008, read8_delegate(FUNC(arkanoid_state::arkanoid_bootleg_d008_r),this) );
}

DRIVER_INIT_MEMBER(arkanoid_state,arkangc)
{
	m_bootleg_id = ARKANGC;
	arkanoid_bootleg_init();
}

DRIVER_INIT_MEMBER(arkanoid_state,arkangc2)
{
	m_bootleg_id = ARKANGC2;
	arkanoid_bootleg_init();
}

DRIVER_INIT_MEMBER(arkanoid_state,block2)
{
	// the graphics on this bootleg have the data scrambled
	int tile;
	UINT8* srcgfx = memregion("gfx1")->base();
	dynamic_buffer buffer(0x18000);

	for (tile = 0; tile < 0x3000; tile++)
	{
		int srctile;

		// combine these into a single swap..
		srctile = BITSWAP16(tile,15,14,13,12,
						11,10,9,8,
						7,5,6,3,
						1,2,4,0);

		srctile = BITSWAP16(srctile,15,14,13,12,
						11,9,10,5,
						7,6,8,4,
						3,2,1,0);

		srctile = srctile ^ 0xd4;

		memcpy(&buffer[tile * 8], &srcgfx[srctile * 8], 8);
	}

	memcpy(srcgfx, &buffer[0], 0x18000);

	m_bootleg_id = BLOCK2;
	arkanoid_bootleg_init();
}

DRIVER_INIT_MEMBER(arkanoid_state,arkblock)
{
	m_bootleg_id = ARKBLOCK;
	arkanoid_bootleg_init();
}

DRIVER_INIT_MEMBER(arkanoid_state,arkbloc2)
{
	m_bootleg_id = ARKBLOC2;
	arkanoid_bootleg_init();
}

DRIVER_INIT_MEMBER(arkanoid_state,arkgcbl)
{
	m_bootleg_id = ARKGCBL;
	arkanoid_bootleg_init();
}

DRIVER_INIT_MEMBER(arkanoid_state,paddle2)
{
	m_bootleg_id = PADDLE2;
	arkanoid_bootleg_init();
}


DRIVER_INIT_MEMBER(arkanoid_state,tetrsark)
{
	UINT8 *ROM = memregion("maincpu")->base();
	int x;

	for (x = 0; x < 0x8000; x++)
	{
		ROM[x] = ROM[x] ^ 0x94;
	}

	m_maincpu->space(AS_PROGRAM).install_write_handler(0xd008, 0xd008, write8_delegate(FUNC(arkanoid_state::tetrsark_d008_w),this));
}


DRIVER_INIT_MEMBER(arkanoid_state,hexa)
{
	UINT8 *RAM = memregion("maincpu")->base();
#if 0


	/* Hexa is not protected or anything, but it keeps writing 0x3f to register */
	/* 0x07 of the AY8910, to read the input ports. This causes clicks in the */
	/* music since the output channels are continuously disabled and reenabled. */
	/* To avoid that, we just NOP out the 0x3f write. */

	RAM[0x0124] = 0x00;
	RAM[0x0125] = 0x00;
	RAM[0x0126] = 0x00;
#endif

	membank("bank1")->configure_entries(0, 2, &RAM[0x10000], 0x4000);
}

DRIVER_INIT_MEMBER(arkanoid_state,hexaa)
{
	DRIVER_INIT_CALL(hexa);

	m_hexaa_from_main = 0;
	m_hexaa_from_sub = 0;

	save_item(NAME(m_hexaa_from_main));
	save_item(NAME(m_hexaa_from_sub));
}

DRIVER_INIT_MEMBER(arkanoid_state,brixian)
{
	UINT8 *RAM = memregion("protdata")->base();

	for (int i=0x000;i<0x200;i++)
		m_protram[i+0x600] = RAM[i];

}

/* Game Drivers */

// original sets of Arkanoid
GAME( 1986, arkanoid,   0,        arkanoid, arkanoid, driver_device, 0,        ROT90, "Taito Corporation Japan", "Arkanoid (World, older)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, arkanoidu,  arkanoid, arkanoid, arkanoid, driver_device, 0,        ROT90, "Taito America Corporation (Romstar license)", "Arkanoid (US, newer)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, arkanoiduo, arkanoid, arkanoid, arkanoid, driver_device, 0,        ROT90, "Taito America Corporation (Romstar license)", "Arkanoid (US, older)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, arkanoidj,  arkanoid, arkanoid, arkanoidj, driver_device,0,        ROT90, "Taito Corporation", "Arkanoid (Japan, newer)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, arkanoidja, arkanoid, arkanoid, arkanoidj, driver_device,0,        ROT90, "Taito Corporation", "Arkanoid (Japan, newer w/level select)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, arkanoidjb, arkanoid, arkanoid, arkanoidj, driver_device,0,        ROT90, "Taito Corporation", "Arkanoid (Japan, older)", MACHINE_SUPPORTS_SAVE )
// bootlegs of Arkanoid
GAME( 1986, arkanoidjbl, arkanoid, arkanoid, arkanoidj, driver_device,0,        ROT90, "bootleg", "Arkanoid (bootleg with MCU, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, arkanoidjbl2,arkanoid, arkanoid, arkanoidj, driver_device,0,        ROT90, "bootleg (Beta)", "Arkanoid (bootleg with MCU, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, ark1ball,   arkanoid, arkanoid, ark1ball, driver_device, 0,        ROT90, "bootleg", "Arkanoid (bootleg with MCU, harder)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, arkangc,    arkanoid, bootleg,  arkangc, arkanoid_state,  arkangc,  ROT90, "bootleg (Game Corporation)", "Arkanoid (Game Corporation bootleg, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, arkangc2,   arkanoid, bootleg,  arkangc2, arkanoid_state, arkangc2, ROT90, "bootleg (Game Corporation)", "Arkanoid (Game Corporation bootleg, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, arkblock,   arkanoid, bootleg,  arkangc, arkanoid_state,  arkblock, ROT90, "bootleg (Game Corporation)", "Block (Game Corporation bootleg, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, arkbloc2,   arkanoid, bootleg,  arkangc, arkanoid_state,  arkbloc2, ROT90, "bootleg (Game Corporation)", "Block (Game Corporation bootleg, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, arkbloc3,   arkanoid, bootleg,  block2, arkanoid_state,   block2,   ROT90, "bootleg (Game Corporation)", "Block (Game Corporation bootleg, set 3)", MACHINE_SUPPORTS_SAVE )    // Both these sets have an extra unknown rom
GAME( 1986, block2,     arkanoid, bootleg,  block2, arkanoid_state,   block2,   ROT90, "bootleg (S.P.A. Co.)", "Block 2 (S.P.A. Co. bootleg)", MACHINE_SUPPORTS_SAVE ) //  and scrambled gfx roms with 'space invader' themed gfx
GAME( 1986, arkgcbl,    arkanoid, bootleg,  arkgcbl, arkanoid_state,  arkgcbl,  ROT90, "bootleg", "Arkanoid (bootleg on Block hardware, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, arkgcbla,   arkanoid, bootleg,  arkgcbl, arkanoid_state,  arkgcbl,  ROT90, "bootleg", "Arkanoid (bootleg on Block hardware, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, paddle2,    arkanoid, bootleg,  paddle2, arkanoid_state,  paddle2,  ROT90, "bootleg", "Paddle 2 (bootleg on Block hardware)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, arkatayt,   arkanoid, bootleg,  arkatayt, driver_device, 0,        ROT90, "bootleg (Tayto)", "Arkanoid (Tayto bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, arktayt2,   arkanoid, bootleg,  arktayt2, driver_device, 0,        ROT90, "bootleg (Tayto)", "Arkanoid (Tayto bootleg, harder)", MACHINE_SUPPORTS_SAVE )
// Other games
GAME( 1987, arkatour,   0,        arkanoid, arkanoid, driver_device, 0,        ROT90, "Taito America Corporation (Romstar license)", "Tournament Arkanoid (US)", MACHINE_SUPPORTS_SAVE )

GAME( 19??, tetrsark,   0,        bootleg,  tetrsark, arkanoid_state, tetrsark, ROT0,  "D.R. Korea", "Tetris (D.R. Korea)", MACHINE_SUPPORTS_SAVE )

GAME( 199?, hexa,       0,        hexa,     hexa, arkanoid_state,     hexa,     ROT0,  "D.R. Korea", "Hexa", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 199?, hexaa,      hexa,     hexaa,    hexa, arkanoid_state,     hexaa,     ROT0,  "D.R. Korea", "Hexa (with 2xZ80, protected)", MACHINE_NOT_WORKING )

GAME( 1993, brixian,    0,        brixian,  brixian, arkanoid_state,  brixian,        ROT0,  "Cheil Computer System", "Brixian", MACHINE_SUPPORTS_SAVE )
