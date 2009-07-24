/***************************************************************************

    Arkanoid driver


    Japanese version support cocktail mode (DSW #7), the others don't.

    Here are the versions we have:

    arkanoid    World version, probably an earlier revision
    arknoidu    USA version, probably a later revision; There has been code
                inserted, NOT patched, so I don't think it's a bootleg
                The 68705 code for this one was not available; I made it up from
                the World version changing the level data pointer table.
    arknoiuo    USA version, probably an earlier revision
                ROM a75-10.bin should be identical to the real World one.
                (It only differs in the country byte from A75-11.ROM)
                This version works fine with the real MCU ROM
    arkatour    Tournament version
                The 68705 code for this one was not available; I made it up from
                the World version changing the level data pointer table.
    arknoidj    Japanese version with level selector.
                The 68705 code for this one was not available; I made it up from
                the World version changing the level data pointer table.
    arkbl2      Bootleg of the early Japanese version.
                The only difference is that the warning text has been replaced
                by "WAIT"
                ROM E2.6F should be identical to the real Japanese one.
                (It only differs in the country byte from A75-11.ROM)
                This version works fine with the real MCU ROM
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
                            HL = adress where the string will be displayed
                                 (most of the times in video RAM)
                            A = colour
                   String begins with the number of chars to display
                   (eg: 0x02 "OK" to display "OK")
      * 0x210d : Display 1 char (called by previous routine)
      * 0x264a : Display score :
                   Inputs : DE = score address
                            HL = adress where the score will be displayed
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
  - The bootleg is based on a Japenese early version we don't have.
  - The MCU is dumped, but the game doesn't run with it.
    However, there is no problem if I use the one from the World early version.
    Until I know what to do with it, I use the MCU from the World early version
    and "save" the existing one in "user1". Let me know if it's good.
  - "(c) Taito Corporation 1986".
  - Displays the "Arkanoid" title.
  - "HARDWARE TEST" message is written, tests are performed, countdown 11 to 0.
  - "NOTICE" screen replaces by "WAIT" without any more text.
    However, the text is still in the ROM at 0x7b81 with changes at the begining :
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
    but they aren't displayed like in the original Japanese set we have ('arknoidj').
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

2a) 'arkangc2'

  - "(c)  Game Corporation 1986".
  - Displays the "Arkanoid" title but routine to display "BLOCK" with bricks exists.
  - No hardware test and no "NOTICE" screen.
  - No reads from 0xf002.
  - Reads bit 1 from 0xd008.
  - "Continue" Dip Switch has been replaced by sort of "Debug" Dip Switch :
      * affects ball speed at start of level (0x04 or 0x06)
      * affects level 2 (same as normal version or same as level 30)
  - You can select your starting level (between 1 and 30)
    but they aren't displayed like in the original Japanese set we have ('arknoidj').
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

2c) 'arkblock'

  - Same as 'arkangc', the only difference is that it displays "BLOCK" with bricks
    instead of displaying the "Arkanoid" title :

      Z:\MAME\dasm>diff arkangc.asm arkbloc2.asm
      8421,8422c8421,8424
      < 32EF: 21 80 03      ld   hl,$0380
      < 32F2: CD D1 20      call $20D1
      ---
      > 32EF: F3            di
      > 32F0: CD 90 7C      call $7C90
      > 32F3: C9            ret
      > 32F4: 14            inc  d

2d) 'arkbloc2'

  - "(c)  Game Corporation 1986".
  - Displays "BLOCK" with bricks.
  - No hardware test and no "NOTICE" screen.
  - All reads from 0xf002 are patched.
  - Reads bit 5 from 0xd008.
  - You can select your starting level (between 1 and 30) but they aren't displayed
    like in the original Japanese set we have ('arknoidj').
  - "Continue" Dip Switch has been replaced by sort of "Debug" Dip Switch :
      * affects ball speed at start of level (0x06 or 0x08)
      * affects level 2 (same as normal version or same as level 30)
  - You can select your starting level (between 1 and 30)
    but they aren't displayed like in the original Japanese set we have ('arknoidj').
  - Level 30 differs from original Japanese version (same as the one from 'arkangc2')
  - Known bugs :
      * You can go from one side of the screen to the other through the walls
        (I can't tell at the moment if it's an ingame bug or not)
      * You are told be to able to select your starting level from level 1 to level 32
        (ingame bug - check code at 0x3425)

2e) 'arkgcbl'

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
    but they aren't displayed like in the original Japanese set we have ('arknoidj').
  - Same level 30 as original Japanese version
  - Known bugs :
      * You can go from one side of the screen to the other through the walls
        (I can't tell at the moment if it's an ingame bug or not)
      * You are told be to able to select your starting level from level 1 to level 32
        (ingame bug - check code at 0x3425)
      * Sound in "Demo Mode" if 1 coin is inserted (ingame bug - check code at 0x0283)
      * Red square on upper middle left "led" when it is supposed to be yellow (ingame bug)

2f) 'paddle2'

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
    but they aren't displayed like in the original Japanese set we have ('arknoidj').
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
    However, the text is still in the ROM at 0x7b81 with changes at the begining :
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

  - Check the following Taito sets (adresses, routines and Dip Switches) :
      * 'arkanoid' = 'arknoiuo'
      * 'arknoidj'
      * 'arknoidu'
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
  - Adresses :
      * 0xc52b : credits
      * 0xc541 : ~(IN5) - test for coins "buttons" (code at 0x0232)
      * 0xc516 : ~(IN5)
      * 0xc517 : ~(IN4)
  - Known bugs :
      * Coins "buttons" don't work - we need to use fake BUTTON2 for each player

***************************************************************************

DIP locations verified for:
  - arknoidj
  - arkanoid

***************************************************************************/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "arkanoid.h"
#include "sound/ay8910.h"
#include "cpu/m6805/m6805.h"

int arkanoid_bootleg_id;


/***************************************************************************/

/* Memory Maps */

static ADDRESS_MAP_START( arkanoid_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xd000, 0xd001) AM_DEVWRITE("ay", ay8910_address_data_w)
	AM_RANGE(0xd001, 0xd001) AM_DEVREAD("ay", ay8910_r)
	AM_RANGE(0xd008, 0xd008) AM_WRITE(arkanoid_d008_w)	/* gfx bank, flip screen etc. */
	AM_RANGE(0xd00c, 0xd00c) AM_READ_PORT("SYSTEM")		/* 2 bits from the 68705 */
	AM_RANGE(0xd010, 0xd010) AM_READ_PORT("BUTTONS") AM_WRITE(watchdog_reset_w)
	AM_RANGE(0xd018, 0xd018) AM_READWRITE(arkanoid_Z80_mcu_r, arkanoid_Z80_mcu_w)  /* input from the 68705 */
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(arkanoid_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0xe800, 0xe83f) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0xe840, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_READNOP	/* fixes instant death in final level */
ADDRESS_MAP_END

static ADDRESS_MAP_START( bootleg_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xd000, 0xd000) AM_DEVWRITE("ay", ay8910_address_w)
	AM_RANGE(0xd001, 0xd001) AM_DEVREADWRITE("ay", ay8910_r, ay8910_data_w)
	AM_RANGE(0xd008, 0xd008) AM_WRITE(arkanoid_d008_w)	/* gfx bank, flip screen etc. */
	AM_RANGE(0xd00c, 0xd00c) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xd010, 0xd010) AM_READ_PORT("BUTTONS") AM_WRITE(watchdog_reset_w)
	AM_RANGE(0xd018, 0xd018) AM_READ_PORT("MUX") AM_WRITENOP
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(arkanoid_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0xe800, 0xe83f) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0xe840, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_READNOP	/* fixes instant death in final level */
ADDRESS_MAP_END

static ADDRESS_MAP_START( mcu_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x7ff)
	AM_RANGE(0x0000, 0x0000) AM_READWRITE(arkanoid_68705_portA_r, arkanoid_68705_portA_w)
	AM_RANGE(0x0001, 0x0001) AM_READ_PORT("MUX")
	AM_RANGE(0x0002, 0x0002) AM_READWRITE(arkanoid_68705_portC_r, arkanoid_68705_portC_w)
	AM_RANGE(0x0004, 0x0004) AM_WRITE(arkanoid_68705_ddrA_w)
	AM_RANGE(0x0006, 0x0006) AM_WRITE(arkanoid_68705_ddrC_w)
	AM_RANGE(0x0010, 0x007f) AM_RAM
	AM_RANGE(0x0080, 0x07ff) AM_ROM
ADDRESS_MAP_END


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
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(arkanoid_68705_input_r, NULL)	/* Inputs from the 68705 */

	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("MUX")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(arkanoid_input_mux, "P1\0P2")

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Allow_Continue ) )	PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )		PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(0x04, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Difficulty ) )		PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )		PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, "20K 60K 60K+" )
	PORT_DIPSETTING(    0x00, "20K" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Lives ) )			PORT_DIPLOCATION("SW1:3")	/* Table at 0x9a28 */
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coinage ) )			PORT_DIPLOCATION("SW1:1,2")	/* Table at 0x0328 */
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )

	PORT_START("UNUSED")	/* This is read in ay8910_interface */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")		/* Spinner Player 1 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15)

	PORT_START("P2")		/* Spinner Player 2  */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_COCKTAIL
INPUT_PORTS_END

/* Different coinage and additional "Cabinet" Dip Switch */
static INPUT_PORTS_START( arknoidj )
	PORT_INCLUDE( arkanoid )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coinage ) )		PORT_DIPLOCATION("SW1:2") /* Table at 0x0320 */
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )		PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static INPUT_PORTS_START( ark1ball )
	PORT_INCLUDE( arknoidj )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SW1:4") /* "ld a,$60" at 0x93bd and "ld a,$60" at 0x9c7f and 0x9c9b */
	PORT_DIPSETTING(    0x10, "60K 100K 60K+" )
	PORT_DIPSETTING(    0x00, "60K" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Lives ) )		PORT_DIPLOCATION("SW1:3") /* Table at 0x9a28 */
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x00, "2" )
INPUT_PORTS_END

/* Bootlegs do not read from the MCU */
static INPUT_PORTS_START( arkatayt )
	PORT_INCLUDE( arknoidj )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SPECIAL )		/* Some bootlegs need it to be 1 */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( arkangc )
	PORT_INCLUDE( arkatayt )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Ball Speed" )			PORT_DIPLOCATION("SW1:8") /* Speed at 0xc462 (code at 0x18aa) - Also affects level 2 (code at 0x7b82) */
	PORT_DIPSETTING(    0x01, DEF_STR( Normal ) )       /* 0xc462 = 0x06 - Normal level 2 */
	PORT_DIPSETTING(    0x00, "Faster" )                /* 0xc462 = 0x08 - Level 2 same as level 30 */
INPUT_PORTS_END

static INPUT_PORTS_START( arkangc2 )
	PORT_INCLUDE( arkatayt )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Ball Speed" )			PORT_DIPLOCATION("SW1:8") /* Speed at 0xc462 (code at 0x18aa) - Also affects level 2 (code at 0x7b82) */
	PORT_DIPSETTING(    0x01, "Slower" )                /* 0xc462 = 0x04 - Normal level 2 */
	PORT_DIPSETTING(    0x00, DEF_STR ( Normal ) )      /* 0xc462 = 0x06 - Level 2 same as level 30 */
INPUT_PORTS_END

static INPUT_PORTS_START( arkgcbl )
	PORT_INCLUDE( arkatayt )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x00, "Round Select" )			PORT_DIPLOCATION("SW1:8") /* Check code at 0x7bc2 - Speed at 0xc462 (code at 0x18aa) */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )          /* 0xc462 = 0x06 */
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )           /* 0xc462 = 0x06 */
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SW1:4") /* "ld a,$60" at 0x93bd and "ld a,$20" at 0x9c7f and 0x9c9b */
	PORT_DIPSETTING(    0x10, "60K 100K 60K+" )         /* But "20K 60K 60K+" when continue */
	PORT_DIPSETTING(    0x00, "60K" )                   /* But "20K" when continue */
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Lives ) )		PORT_DIPLOCATION("SW1:3")
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
	PORT_DIPNAME( 0x04, 0x04, "Controls ?" )			PORT_DIPLOCATION("SW1:6") /* Check code at 0x96a1 and read notes */
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Alternate ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SW1:4") /* "ld a,$60" at 0x93bd and "ld a,$20" at 0x9c7f and 0x9c9b */
	PORT_DIPSETTING(    0x10, "60K 100K 60K+" )         /* But "20K 60K 60K+" when continue */
	PORT_DIPSETTING(    0x00, "60K" )                   /* But "20K" when continue */
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Lives ) )		PORT_DIPLOCATION("SW1:3") /* Table at 0x9a28 */
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x20, "3" )
INPUT_PORTS_END

static INPUT_PORTS_START( arktayt2 )
	PORT_INCLUDE( arkatayt )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SW1:4") /* "ld a,$60" at 0x93bd and "ld a,$60" at 0x9c7f and 0x9c9b */
	PORT_DIPSETTING(    0x10, "60K 100K 60K+" )
	PORT_DIPSETTING(    0x00, "60K" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Lives ) )		PORT_DIPLOCATION("SW1:3") /* Table at 0x9a28 */
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coinage ) )		PORT_DIPLOCATION("SW1:2") /* Table at 0x0320 */
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


/***************************************************************************/

/* Graphics Layouts */

static const gfx_layout charlayout =
{
	8,8,	/* 8*8 characters */
	4096,	/* 4096 characters */
	3,	/* 3 bits per pixel */
	{ 2*4096*8*8, 4096*8*8, 0 },	/* the two bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8	/* every char takes 8 consecutive bytes */
};

/* Graphics Decode Information */

static GFXDECODE_START( arkanoid )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,  0, 64 )
	// sprites use the same characters above, but are 16x8
GFXDECODE_END

/* Sound Interfaces */

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("UNUSED"),
	DEVCB_INPUT_PORT("DSW"),
	DEVCB_NULL,
	DEVCB_NULL
};

/* Machine Drivers */

static MACHINE_DRIVER_START( arkanoid )
	// basic machine hardware
	MDRV_CPU_ADD("maincpu", Z80, XTAL_12MHz/2) /* verified on pcb */
	MDRV_CPU_PROGRAM_MAP(arkanoid_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("mcu", M68705, XTAL_12MHz/4) /* verified on pcb */
	MDRV_CPU_PROGRAM_MAP(mcu_map)

	MDRV_QUANTUM_TIME(HZ(6000))					// 100 CPU slices per second to synchronize between the MCU and the main CPU

	MDRV_MACHINE_START(arkanoid)
	MDRV_MACHINE_RESET(arkanoid)

	// video hardware

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MDRV_GFXDECODE(arkanoid)
	MDRV_PALETTE_LENGTH(512)

	MDRV_PALETTE_INIT(RRRR_GGGG_BBBB)
	MDRV_VIDEO_START(arkanoid)
	MDRV_VIDEO_UPDATE(arkanoid)

	// sound hardware
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay", AY8910, XTAL_12MHz/4/2) /* YM2149 clock is 3mhz, pin 26 is low so 3mhz/2 */
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.33)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( bootleg )
	MDRV_IMPORT_FROM(arkanoid)

	// basic machine hardware
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(bootleg_map)

	MDRV_DEVICE_REMOVE("mcu")
MACHINE_DRIVER_END


/***************************************************************************/

/* ROMs */

ROM_START( arkanoid )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a75-01-1.rom", 0x0000, 0x8000, CRC(5bcda3b0) SHA1(52cadd38b5f8e8856f007a9c602d6b508f30be65) )
	ROM_LOAD( "a75-11.rom",   0x8000, 0x8000, CRC(eafd7191) SHA1(d2f8843b716718b1de209e97a874e8ce600f3f87) )

	ROM_REGION( 0x0800, "mcu", 0 )	/* 2k for the microcontroller */
	ROM_LOAD( "arkanoid.uc",  0x0000, 0x0800, CRC(515d77b6) SHA1(a302937683d11f663abd56a2fd7c174374e4d7fb) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "a75-03.rom",   0x00000, 0x8000, CRC(038b74ba) SHA1(ac053cc4908b4075f918748b89570e07a0ba5116) )
	ROM_LOAD( "a75-04.rom",   0x08000, 0x8000, CRC(71fae199) SHA1(5d253c46ccf4cd2976a5fb8b8713f0f345443d06) )
	ROM_LOAD( "a75-05.rom",   0x10000, 0x8000, CRC(c76374e2) SHA1(7520dd48de20db60a2038f134dcaa454988e7874) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "a75-07.bpr",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) )	/* red component */
	ROM_LOAD( "a75-08.bpr",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) )	/* green component */
	ROM_LOAD( "a75-09.bpr",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) )	/* blue component */
ROM_END

ROM_START( arkanoidu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a75-19.bin",   0x0000, 0x8000, CRC(d3ad37d7) SHA1(a172a1ef5bb83ee2d8ed2842ef8968af19ad411e) )
	ROM_LOAD( "a75-18.bin",   0x8000, 0x8000, CRC(cdc08301) SHA1(05f54353cc8333af14fa985a2764960e20e8161a) )

	ROM_REGION( 0x0800, "mcu", 0 )	/* 2k for the microcontroller */
	ROM_LOAD( "arknoidu.uc",  0x0000, 0x0800, BAD_DUMP CRC(de518e47) SHA1(b8eddd1c566505fb69e3d1207c7a9720dfb9f503)  )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "a75-03.rom",   0x00000, 0x8000, CRC(038b74ba) SHA1(ac053cc4908b4075f918748b89570e07a0ba5116) )
	ROM_LOAD( "a75-04.rom",   0x08000, 0x8000, CRC(71fae199) SHA1(5d253c46ccf4cd2976a5fb8b8713f0f345443d06) )
	ROM_LOAD( "a75-05.rom",   0x10000, 0x8000, CRC(c76374e2) SHA1(7520dd48de20db60a2038f134dcaa454988e7874) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "a75-07.bpr",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) )	/* red component */
	ROM_LOAD( "a75-08.bpr",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) )	/* green component */
	ROM_LOAD( "a75-09.bpr",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) )	/* blue component */
ROM_END

ROM_START( arkanoiduo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a75-01-1.rom", 0x0000, 0x8000, CRC(5bcda3b0) SHA1(52cadd38b5f8e8856f007a9c602d6b508f30be65) )
	ROM_LOAD( "a75-10.rom",   0x8000, 0x8000, CRC(a1769e15) SHA1(fbb45731246a098b29eb08de5d63074b496aaaba) )

	ROM_REGION( 0x0800, "mcu", 0 )	/* 2k for the microcontroller */
	ROM_LOAD( "arkanoid.uc",  0x0000, 0x0800, CRC(515d77b6) SHA1(a302937683d11f663abd56a2fd7c174374e4d7fb) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "a75-03.rom",   0x00000, 0x8000, CRC(038b74ba) SHA1(ac053cc4908b4075f918748b89570e07a0ba5116) )
	ROM_LOAD( "a75-04.rom",   0x08000, 0x8000, CRC(71fae199) SHA1(5d253c46ccf4cd2976a5fb8b8713f0f345443d06) )
	ROM_LOAD( "a75-05.rom",   0x10000, 0x8000, CRC(c76374e2) SHA1(7520dd48de20db60a2038f134dcaa454988e7874) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "a75-07.bpr",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) )	/* red component */
	ROM_LOAD( "a75-08.bpr",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) )	/* green component */
	ROM_LOAD( "a75-09.bpr",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) )	/* blue component */
ROM_END

ROM_START( arkatour )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a75-27.17",   0x0000, 0x8000, CRC(e3b8faf5) SHA1(4c09478fa41881fa89ee6afb676aeb780f17ac2e) )
	ROM_LOAD( "a75-28.18",   0x8000, 0x8000, CRC(326aca4d) SHA1(5a194b7a0361236d471b24905dc6434372f81252) )

	ROM_REGION( 0x0800, "mcu", 0 )	/* 2k for the microcontroller */
	ROM_LOAD( "a75-32.14",  0x0000, 0x0800, BAD_DUMP CRC(d3249559) SHA1(b1542764450016614e9e03cedd6a2f1e59961789)  )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "a75-29.64",   0x00000, 0x8000, CRC(5ddea3cf) SHA1(58f16515898b7cc2697bf7663a60d9ca0db6da95) )
	ROM_LOAD( "a75-30.63",   0x08000, 0x8000, CRC(5fcf2e85) SHA1(f721f0afb0550cc64bff26681856a7576398d9b5) )
	ROM_LOAD( "a75-31.62",   0x10000, 0x8000, CRC(7b76b192) SHA1(a68aa08717646a6c322cf3455df07f50df9e9f33) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "a75-33.24",    0x0000, 0x0200, CRC(b4bf3c81) SHA1(519188937ac9728c653fabac877e37dc43c3f71a) )	/* red component */
	ROM_LOAD( "a75-34.23",    0x0200, 0x0200, CRC(de85a803) SHA1(325214995996de36a0470fbfc00e4e393c0b17ad) )	/* green component */
	ROM_LOAD( "a75-35.35",    0x0400, 0x0200, CRC(38acfd3b) SHA1(2841e9db047aa039eff8567a518b6250b355507b) )	/* blue component */
ROM_END

ROM_START( arkanoidj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a75-21.rom",   0x0000, 0x8000, CRC(bf0455fc) SHA1(250522b84b9f491c3f4efc391bf6aa6124361369) )
	ROM_LOAD( "a75-22.rom",   0x8000, 0x8000, CRC(3a2688d3) SHA1(9633a661352def3d85f95ca830f6d761b0b5450e) )

	ROM_REGION( 0x0800, "mcu", 0 )	/* 2k for the microcontroller */
	ROM_LOAD( "arknoidj.uc",  0x0000, 0x0800, BAD_DUMP CRC(0a4abef6) SHA1(fdce0b7a2eab7fd4f1f4fc3b93120b1ebc16078e)  )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "a75-03.rom",   0x00000, 0x8000, CRC(038b74ba) SHA1(ac053cc4908b4075f918748b89570e07a0ba5116) )
	ROM_LOAD( "a75-04.rom",   0x08000, 0x8000, CRC(71fae199) SHA1(5d253c46ccf4cd2976a5fb8b8713f0f345443d06) )
	ROM_LOAD( "a75-05.rom",   0x10000, 0x8000, CRC(c76374e2) SHA1(7520dd48de20db60a2038f134dcaa454988e7874) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "a75-07.bpr",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) )	/* red component */
	ROM_LOAD( "a75-08.bpr",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) )	/* green component */
	ROM_LOAD( "a75-09.bpr",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) )	/* blue component */
ROM_END

ROM_START( arkmcubl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "e1.6d",        0x0000, 0x8000, CRC(dd4f2b72) SHA1(399a8636030a702dafc1da926f115df6f045bef1) )
	ROM_LOAD( "e2.6f",        0x8000, 0x8000, CRC(bbc33ceb) SHA1(e9b6fef98d0d20e77c7a1c25eff8e9a8c668a258) )

      /* MCU from the World early version ('arkanoid'), so the game is playable */
	ROM_REGION( 0x0800, "mcu", 0 )	/* 2k for the microcontroller */
	ROM_LOAD( "arkmcubl.uc",  0x0000, 0x0800, BAD_DUMP CRC(515d77b6) SHA1(a302937683d11f663abd56a2fd7c174374e4d7fb) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "a75-03.rom",   0x00000, 0x8000, CRC(038b74ba) SHA1(ac053cc4908b4075f918748b89570e07a0ba5116) )
	ROM_LOAD( "a75-04.rom",   0x08000, 0x8000, CRC(71fae199) SHA1(5d253c46ccf4cd2976a5fb8b8713f0f345443d06) )
	ROM_LOAD( "a75-05.rom",   0x10000, 0x8000, CRC(c76374e2) SHA1(7520dd48de20db60a2038f134dcaa454988e7874) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "a75-07.bpr",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) )	/* red component */
	ROM_LOAD( "a75-08.bpr",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) )	/* green component */
	ROM_LOAD( "a75-09.bpr",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) )	/* blue component */

	/* Until we know what this MCU is supposed to do, we put it here */
	ROM_REGION( 0x0800, "user1", 0 )
	ROM_LOAD( "68705p3.6i",   0x0000, 0x0800, CRC(389a8cfb) SHA1(9530c051b61b5bdec7018c6fdc1ea91288a406bd) )
ROM_END

ROM_START( ark1ball )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a-1.7d",       0x0000, 0x8000, CRC(dd4f2b72) SHA1(399a8636030a702dafc1da926f115df6f045bef1) )
	ROM_LOAD( "2palline.7f",  0x8000, 0x8000, CRC(ed6b62ab) SHA1(4d4991b422756bd304fc5ef236aac1422fe1f999) )

      /* MCU from the World early version ('arkanoid'), so the game is playable */
	ROM_REGION( 0x0800, "mcu", 0 )	/* 2k for the microcontroller */
	ROM_LOAD( "ark1ball.uc",  0x0000, 0x0800, BAD_DUMP CRC(515d77b6) SHA1(a302937683d11f663abd56a2fd7c174374e4d7fb) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "a-3.3a",       0x00000, 0x8000, CRC(038b74ba) SHA1(ac053cc4908b4075f918748b89570e07a0ba5116) )
	ROM_LOAD( "a-4.3d",       0x08000, 0x8000, CRC(71fae199) SHA1(5d253c46ccf4cd2976a5fb8b8713f0f345443d06) )
	ROM_LOAD( "a-5.3f",       0x10000, 0x8000, CRC(c76374e2) SHA1(7520dd48de20db60a2038f134dcaa454988e7874) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "a75-07.bpr",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) )	/* red component */
	ROM_LOAD( "a75-08.bpr",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) )	/* green component */
	ROM_LOAD( "a75-09.bpr",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) )	/* blue component */
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
	ROM_LOAD( "a75-07.bpr",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) )	/* red component */
	ROM_LOAD( "a75-08.bpr",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) )	/* green component */
	ROM_LOAD( "a75-09.bpr",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) )	/* blue component */
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
	ROM_LOAD( "a75-07.bpr",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) )	/* red component */
	ROM_LOAD( "a75-08.bpr",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) )	/* green component */
	ROM_LOAD( "a75-09.bpr",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) )	/* blue component */
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
	ROM_LOAD( "a75-07.bpr",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) )	/* red component */
	ROM_LOAD( "a75-08.bpr",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) )	/* green component */
	ROM_LOAD( "a75-09.bpr",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) )	/* blue component */
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
	ROM_LOAD( "a75-07.bpr",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) )	/* red component */
	ROM_LOAD( "a75-08.bpr",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) )	/* green component */
	ROM_LOAD( "a75-09.bpr",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) )	/* blue component */
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
	ROM_LOAD( "82s129.5k",    0x0000, 0x0100, CRC(fa70b64d) SHA1(273669d05f793cf1ee0741b175be281307fa9b5e) )	/* red component   + */
	ROM_LOAD( "82s129.5jk",   0x0100, 0x0100, CRC(cca69884) SHA1(fdcd66110c8eb901a401f8618821c7980946a511) )	/* red component   = a75-07.bpr*/
	ROM_LOAD( "82s129.5l",    0x0200, 0x0100, CRC(3e4d2bf5) SHA1(c475887302dd137d6965769070b7d55f488c1b25) )	/* green component + */
	ROM_LOAD( "82s129.5kl",   0x0300, 0x0100, CRC(085d625a) SHA1(26c96a1c1b7562fed84c31dd92fdf7829e96a9c7) )	/* green component = a75-08.bpr*/
	ROM_LOAD( "82s129.5mn",   0x0400, 0x0100, CRC(0fe0b108) SHA1(fcf27619208922345a1e42b3a219b4274f66968d) )	/* blue component  + */
	ROM_LOAD( "63s141.5m",    0x0500, 0x0100, CRC(5553f675) SHA1(c50255af8d99664b92e0bb34a527fd42ebf7e759) )	/* blue component  = a75-09.bpr*/

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
	ROM_LOAD( "a75-07.bpr",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) )	/* red component */
	ROM_LOAD( "a75-08.bpr",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) )	/* green component */
	ROM_LOAD( "a75-09.bpr",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) )	/* blue component */
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
	ROM_LOAD( "ic73.11e",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) )	/* red component */
	ROM_LOAD( "ic74.12e",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) )	/* green component */
	ROM_LOAD( "ic75.13e",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) )	/* blue component */
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
	ROM_LOAD( "ic73.11e",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) )	/* red component */
	ROM_LOAD( "ic74.12e",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) )	/* green component */
	ROM_LOAD( "ic75.13e",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) )	/* blue component */
ROM_END

/* the other Dr. Korea game (Hexa, hexa.c) also appears to be derived from Arkanoid hardware */

ROM_START( tetrsark )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "ic17.1",      0x00000, 0x8000, CRC(1a505eda) SHA1(92f171a12cf0c326d29c244514718df04b998426) )
	ROM_LOAD( "ic16.2",      0x08000, 0x8000, CRC(157bc4df) SHA1(b2c704148e7e3ca61ab51308ee0d66ea1088bff3) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "ic64.3",      0x00000, 0x8000, CRC(c3e9b290) SHA1(6e99520606c654e531dbeb9a598cfbb443c24dff) )
	ROM_LOAD( "ic63.4",      0x08000, 0x8000, CRC(de9a368f) SHA1(ffbb2479200648da3f3e7ab7cebcdb604f6dfb3d) )
	ROM_LOAD( "ic62.5",      0x10000, 0x8000, CRC(c8e80a00) SHA1(4bee4c36ee768ae68ebc64e639fdc43f61c74f92) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "a75-07.bpr",    0x0000, 0x0200, CRC(0af8b289) SHA1(6bc589e8a609b4cf450aebedc8ce02d5d45c970f) )	/* red component */
	ROM_LOAD( "a75-08.bpr",    0x0200, 0x0200, CRC(abb002fb) SHA1(c14f56b8ef103600862e7930709d293b0aa97a73) )	/* green component */
	ROM_LOAD( "a75-09.bpr",    0x0400, 0x0200, CRC(a7c6c277) SHA1(adaa003dcd981576ea1cc5f697d709b2d6b2ea29) )	/* blue component */
ROM_END


/* Driver Initialization */

static void arkanoid_bootleg_init( running_machine *machine )
{
	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xf002, 0xf002, 0, 0, arkanoid_bootleg_f002_r );
	memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xd018, 0xd018, 0, 0, arkanoid_bootleg_d018_w );
	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xd008, 0xd008, 0, 0, arkanoid_bootleg_d008_r );
}

static DRIVER_INIT( arkangc )
{
	arkanoid_bootleg_id = ARKANGC;
	arkanoid_bootleg_init(machine);
}

static DRIVER_INIT( arkangc2 )
{
	arkanoid_bootleg_id = ARKANGC2;
	arkanoid_bootleg_init(machine);
}

static DRIVER_INIT( arkblock )
{
	arkanoid_bootleg_id = ARKBLOCK;
	arkanoid_bootleg_init(machine);
}

static DRIVER_INIT( arkbloc2 )
{
	arkanoid_bootleg_id = ARKBLOC2;
	arkanoid_bootleg_init(machine);
}

static DRIVER_INIT( arkgcbl )
{
	arkanoid_bootleg_id = ARKGCBL;
	arkanoid_bootleg_init(machine);
}

static DRIVER_INIT( paddle2 )
{
	arkanoid_bootleg_id = PADDLE2;
	arkanoid_bootleg_init(machine);
}


static DRIVER_INIT( tetrsark )
{
	UINT8 *ROM = memory_region(machine, "maincpu");
	int x;

	for (x=0;x<0x8000;x++)
	{
		ROM[x]=ROM[x]^0x94;
	}

	memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xd008, 0xd008, 0, 0, tetrsark_d008_w );
}


/* Game Drivers */

GAME( 1986, arkanoid,  0,        arkanoid, arkanoid, 0,        ROT90, "Taito Corporation Japan", "Arkanoid (World)", GAME_SUPPORTS_SAVE )
GAME( 1986, arkanoidu, arkanoid, arkanoid, arkanoid, 0,        ROT90, "Taito America Corporation (Romstar license)", "Arkanoid (US)", GAME_SUPPORTS_SAVE )
GAME( 1986, arkanoiduo,arkanoid, arkanoid, arkanoid, 0,        ROT90, "Taito America Corporation (Romstar license)", "Arkanoid (US, older)", GAME_SUPPORTS_SAVE )
GAME( 1986, arkanoidj, arkanoid, arkanoid, arknoidj, 0,        ROT90, "Taito Corporation", "Arkanoid (Japan)", GAME_SUPPORTS_SAVE )
GAME( 1986, arkmcubl, arkanoid, arkanoid, arknoidj, 0,        ROT90, "bootleg", "Arkanoid (bootleg with MCU)", GAME_SUPPORTS_SAVE )
GAME( 1986, ark1ball, arkanoid, arkanoid, ark1ball, 0,        ROT90, "bootleg", "Arkanoid (bootleg with MCU, harder)", GAME_SUPPORTS_SAVE )
GAME( 1986, arkangc,  arkanoid, bootleg,  arkangc,  arkangc,  ROT90, "bootleg", "Arkanoid (Game Corporation bootleg, set 1)", GAME_SUPPORTS_SAVE )
GAME( 1986, arkangc2, arkanoid, bootleg,  arkangc2, arkangc2, ROT90, "bootleg", "Arkanoid (Game Corporation bootleg, set 2)", GAME_SUPPORTS_SAVE )
GAME( 1986, arkblock, arkanoid, bootleg,  arkangc,  arkblock, ROT90, "bootleg", "Block (Game Corporation bootleg, set 1)", GAME_SUPPORTS_SAVE )
GAME( 1986, arkbloc2, arkanoid, bootleg,  arkangc,  arkbloc2, ROT90, "bootleg", "Block (Game Corporation bootleg, set 2)", GAME_SUPPORTS_SAVE )
GAME( 1986, arkgcbl,  arkanoid, bootleg,  arkgcbl,  arkgcbl,  ROT90, "bootleg", "Arkanoid (bootleg on Block hardware)", GAME_SUPPORTS_SAVE )
GAME( 1988, paddle2,  arkanoid, bootleg,  paddle2,  paddle2,  ROT90, "bootleg", "Paddle 2 (bootleg on Block hardware)", GAME_SUPPORTS_SAVE )
GAME( 1986, arkatayt, arkanoid, bootleg,  arkatayt, 0,        ROT90, "bootleg", "Arkanoid (Tayto bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1986, arktayt2, arkanoid, bootleg,  arktayt2, 0,        ROT90, "bootleg", "Arkanoid (Tayto bootleg, harder)", GAME_SUPPORTS_SAVE )
GAME( 1987, arkatour, arkanoid, arkanoid, arkanoid, 0,        ROT90, "Taito America Corporation (Romstar license)", "Tournament Arkanoid (US)", GAME_SUPPORTS_SAVE )
GAME( 19??, tetrsark, 0,        bootleg,  tetrsark, tetrsark, ROT0,  "D.R. Korea", "Tetris (D.R. Korea)", GAME_SUPPORTS_SAVE )
