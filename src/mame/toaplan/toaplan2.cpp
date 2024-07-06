// license:BSD-3-Clause
// copyright-holders:Quench, Yochizo, David Haywood
// thanks-to:Richard Bush
/*****************************************************************************

        ToaPlan      game hardware from 1991 - 1994
        Raizing/8ing game hardware from 1993 onwards
        -------------------------------------------------
        Driver by: Quench and Yochizo
        Original othldrby.c by Nicola Salmoria

   Raizing games and Truxton 2 are heavily dependent on the Raine source -
   many thanks to Richard Bush and the Raine team. [Yochizo]


Supported games:

    Name        Board No      Maker         Game name
    ----------------------------------------------------------------------------
    tekipaki    TP-020        Toaplan       Teki Paki
    tekipakit   TP-020        Toaplan       Teki Paki (location test)
    ghox        TP-021        Toaplan       Ghox (Spinner with single up/down axis control)
    ghoxj       TP-021        Toaplan       Ghox (8-Way Joystick controls)
    dogyuun     TP-022        Toaplan       Dogyuun
    dogyuuna    TP-022        Toaplan       Dogyuun (older)
    dogyuunt    TP-022        Toaplan       Dogyuun (location test)
    kbash       TP-023        Toaplan       Knuckle Bash
    kbash2      bootleg       Toaplan       Knuckle Bash 2
    truxton2    TP-024        Toaplan       Truxton 2 / Tatsujin Oh
    pipibibs    TP-025        Toaplan       Pipi & Bibis / Whoopee!! (set 1)
    pipibibsa   TP-025        Toaplan       Pipi & Bibis / Whoopee!! (set 2)
    pipibibsp   TP-025        Toaplan       Pipi & Bibis / Whoopee!! (Prototype)
    pipibibsbl  bootleg       Toaplan       Pipi & Bibis / Whoopee!! (based of the prototype)
    whoopee    *TP-025/TP-020 Toaplan       Pipi & Bibis / Whoopee!! (Teki Paki hardware)
    fixeight    TP-026        Toaplan       FixEight
    fixeightbl  bootleg       Toaplan       FixEight
    grindstm    TP-027        Toaplan       Grind Stormer (1992)
    grindstma   TP-027        Toaplan       Grind Stormer (1992) (older)
    vfive       TP-027        Toaplan       V-V (V-Five)  (1993 - Japan only)
    batsugun    TP-030        Toaplan       Batsugun
    batsuguna   TP-030        Toaplan       Batsugun (older)
    batsugunsp  TP-030        Toaplan       Batsugun (Special Version)
    enmadaio    TP-031        Toaplan       Enma Daio
    pwrkick     SW931201      Sunwise       Power Kick
    burgkids    SW931201      Sunwise       Burger Kids
    othldrby    S951060-VGP   Sunwise       Othello Derby
    snowbro2    TP-033        Hanafram      Snow Bros. 2 - With New Elves

    * This version of Whoopee!! is on a board labeled TP-020
      (same board number, and same hardware, as Teki Paki)
      but the ROMs are labeled TP-025.

    sstriker    RA-MA7893-01  Raizing       Sorcer Striker
    sstrikera   RA-MA7893-01  Raizing       Sorcer Striker (Unite Trading license)
    mahoudai    RA-MA7893-01  Raizing       Mahou Daisakusen (Japan)
    kingdmgp    RA-MA9402-03  Raizing/8ing  Kingdom Grandprix
    shippumd    RA-MA9402-03  Raizing/8ing  Shippu Mahou Daisakusen (Japan)
    bgaregga    RA9503        Raizing/8ing  Battle Garegga (World - Sat Feb 3 1996)
    bgareggat   RA9503        Raizing/8ing  Battle Garegga (location test - Wed Jan 17 1996)
    bgareggahk  RA9503        Raizing/8ing  Battle Garegga (Hong Kong (and Austria?) - Sat Feb 3 1996)
    bgareggatw  RA9503        Raizing/8ing  Battle Garegga (Taiwan (and Germany?) - Thu Feb 1 1996)
    bgaregganv  RA9503        Raizing/8ing  Battle Garegga - New Version (Hong Kong (and Austria?) - Sat Mar 2 1996)
    bgareggat2  RA9503        Raizing/8ing  Battle Garegga - Type 2 (World - Sat Mar 2 1996)
    bgareggacn  RA9503        Raizing/8ing  Battle Garegga - Type 2 (China (and Denmark?) - Tue Apr 2 1996)
    batrider    RA9704        Raizing/8ing  Armed Police Batrider (Europe - Fri Feb 13 1998)
    batrideru   RA9704        Raizing/8ing  Armed Police Batrider (USA - Fri Feb 13 1998)
    batriderc   RA9704        Raizing/8ing  Armed Police Batrider (China - Fri Feb 13 1998)
    batriderj   RA9704        Raizing/8ing  Armed Police Batrider - B Version (Japan - Fri Feb 13 1998)
    batriderk   RA9704        Raizing/8ing  Armed Police Batrider (Korea - Fri Feb 13 1998)
    batriderja  RA9704        Raizing/8ing  Armed Police Batrider (Japan - Mon Dec 22 1997)
    batridert   RA9704        Raizing/8ing  Armed Police Batrider (Taiwan - Mon Dec 22 1997)
    bbakraid    ET68-V99      8ing          Battle Bakraid - Unlimited Version (USA - Tue Jun 8th, 1999)
    bbakraidj   ET68-V99      8ing          Battle Bakraid - Unlimited Version (Japan - Tue Jun 8th, 1999)
    bbakraidja  ET68-V99      8ing          Battle Bakraid (Japan - Wed Apr 7th, 1999)

    SET NOTES:

    ghox     - The ghoxj set displays an English title screen when the jumpers are set for Japan/Taito,
               and fails to display the "Winners Don't Use Drugs" logo when set for USA/Taito (either
               Taito America or Taito Japan).

    dogyuun  - In the location test version, if you are hit while you have a bomb, the bomb explodes
               automatically and saves you from dying. In the final released version, the bomb explodes
               but you die anyway.
               The only difference between the dogyuun and dogyuuna sets is some of the region jumper
               settings; see the INPUT_PORTS definitions.

    truxton2 - Although the truxton2 PCB has only standard JAMMA mono audio output, and uses a YM3014B
               mono DAC, the YM2151 music is actually sequenced in stereo. In toaplan2.h, uncomment
               "#define TRUXTON2_STEREO" to hear the game's music the way it was originally composed.
               Difficulty is much lower when the region is set to Europe or USA than when set to any
               Asian region, independent of the "Difficulty" dipswitches. See the code beginning at
               1FE94 (RAM address 1002D6 contains 0 if region is an Asian region, 1 if Europe or USA)

    fixeight - The same program is used for all regions, and the region can be changed just by swapping
               EEPROMs. However, the V25 code also recognizes a secret input that rewrites the EEPROM to
               use any one of the 14 recognized regional licenses, using the state of the player 1 and
               player 2 button inputs held in conjunction with it as a 4-bit binary code:

               Region                      Button input
               ------------------------    ------------------------------------
               Korea, Taito license        No buttons
               Korea                       P1 button 1
               Hong Kong, Taito license    P1 button 2
               Hong Kong                   P1 buttons 1 & 2
               Taiwan, Taito license       P2 button 1
               Taiwan                      P1 button 1 + P2 button 1
               SE Asia, Taito license      P1 button 2 + P2 button 1
               Southeast Asia              P1 buttons 1 & 2 + P2 button 1
               Europe, Taito license       P2 button 2
               Europe                      P1 button 1 + P2 button 2
               USA, Taito license          P1 button 2 + P2 button 2
               USA                         P1 buttons 1 & 2 + P2 button 2
               (Invalid)                   P2 buttons 1 & 2
               (Invalid)                   P1 button 1 + P2 buttons 1 & 2
               Japan                       P1 button 2 + P2 buttons 1 & 2
               Japan, Taito license        P1 buttons 1 & 2 + P2 buttons 1 & 2

    grindstm - Code at 20A26 in vfive forces region to Japan. All sets have some NOPs at reset vector,
               and the NEC V25 CPU test that the other games do is skipped. Furthermore, all sets have
               a broken ROM checksum routine that reads address ranges that don't match the actual
               location or size of the ROM, and that has a hack at the end so it always passes.
               Normally you would expect to see code like this in a bootleg, but the NOPs and other
               oddities are identical among three different sets.

    batsugun - The Special Version has many changes to make the game easier: it adds an autofire button,
               replaces the regular bomb with the more powerful double bomb (which in the original version
               required both players in a two player game to press their bomb buttons at once), gives you
               a shield that can absorb one hit each time your ship "levels up", etc. It also changes the
               colors of the title screen, ship select screen, stages, and enemies.
               batsugun compared to batsuguna has code that looks more like the Special Version, but it
               doesn't have any of the Special Version features. All the differences between batsugun
               and batsuguna look like bug fixes that were carried over into the Special Version.

    sstriker - The mahoudai set reads the region jumpers, but the lookup tables in ROM that map jumper
               settings to copyright text, coinage settings, etc., contain identical values for every
               jumper setting, effectively ignoring the jumpers and forcing the region to Japan.
               On the other hand, sstriker has its title screen and all its text in English even when
               the region is set to Japan. This seems odd but has been verified correct on two boards.
               The only difference between sstriker and sstrikera is the copyright text displayed when
               the region is set to Korea.

    kingdmgp - The kingdmgp and shippumd sets have identical program ROMs but a different graphics ROM
               for the text layer. Setting the region to Japan with the kingdmgp ROM, or to anything other
               than Japan with the shippumd ROM, results in a corrupt title screen and unreadable text.
               In kingdmgp some of the tiles needed for the credits screen in attract mode have been
               stripped out, resulting in boxes where letters should be. It doesn't seem very professional
               but appears to be a genuine release. A lot of boards being sold as 'Kingdom Grand Prix' are
               in fact conversions using Neill Corlett's hack.

    bgaregga - The later versions change the small bullet-shaped enemy bullets to bright yellow balls,
               eliminate the flying metal debris from explosions, and require additional joystick input
               to access the Extended, Harder, Special, and Stage Edit hidden features.
               In addition to these changes, the bgareggat2 set uses only two buttons. Instead of being
               able to change the formation of your options with the third button, each of the selectable
               ships has a different, fixed option formation. However, the third button can still be used
               to select an alternate ship color and to enter the secret character and Stage Edit codes.

    batrider - Batrider was marketed as a two button game, and the regular ships all use only the first
               two buttons, but in the original version you need the third button in order to control the
               options of the hidden Battle Garegga ships.
               This problem was fixed in the B Version, which lets you change the Battle Garegga ships'
               option formation using Street Fighter style joystick commands (as well as by using the third
               button, if the cabinet has one)

    bbakraid - Because players managed to counter stop the original Battle Bakraid not long after release,
               the Unlimited Version, which can display more score digits, was released as a ROM upgrade.
               The upgrade also fixes the bug in the original version that prevented the unlocking of
               Team Edit mode from being saved in the EEPROM.


 ****************************************************************************
 * Battle Garegga and Armed Police Batrider have secret characters          *
 * and game features.                                                       *
 * Try to input the following commands to use them.                         *
 * ======================================================================== *
 * Battle Garegga                                                           *
 *       The button you use to select your ship not only determines its     *
 *       color, but affects its characteristics.                            *
 *           A: Default characteristics.                                    *
 *           B: Slightly higher speed than A type.                          *
 *           C: Slightly smaller hitbox than A type.                        *
 *       A+B+C: Same speed as B type and same hitbox as C type.             *
 *                                                                          *
 *       After inserting a coin (pushing a credit button), input            *
 *       UP  UP  DOWN  DOWN  LEFT  RIGHT  LEFT  RIGHT  A  B  C  START       *
 *       then you can use Mahou Daisakusen characters.                      *
 *                                                                          *
 * Note: In versions of the game dated Mar 2 1996 or later, you must        *
 *       hold the joystick RIGHT in addition to the specified button(s)     *
 *       when entering any of the following commands. Even if Stage Edit    *
 *       is enabled via dipswitch, you need to hold RIGHT to use it.        *
 *                                                                          *
 * EXTENDED:   After inserting a coin, hold A and press START.              *
 *             You play through all stages twice before the game ends.      *
 * HARDER:     After inserting a coin, hold B and press START.              *
 *             Difficulty is increased.                                     *
 * SPECIAL:    After inserting a coin, hold A and B and press START.        *
 *             Combination of EXTENDED and HARDER modes.                    *
 * STAGE EDIT: After inserting a coin, hold C and press START.              *
 *             You can choose what order to play Stage 2, 3 and 4 in,       *
 *             or even skip them.                                           *
 *                                                                          *
 * EXTENDED, HARDER, and SPECIAL modes each have their own high score list. *
 * ------------------------------------------------------------------------ *
 * Armed Police Batrider                                                    *
 *       The button you use to select your ship not only determines its     *
 *       color, but affects its characteristics.                            *
 *           A: High main shot power, low option shot power.                *
 *              Average speed. Default autofire rate is 15 Hz.              *
 *           B: Low main shot power, high option shot power. Slightly       *
 *              slower than A type. Default autofire rate is 12 Hz.         *
 *           C: High main shot and option shot power, but lowest speed.     *
 *              Default autofire rate is 20 Hz.                             *
 *       START: Low main shot and option shot power, but highest speed.     *
 *              Default autofire rate is 10 Hz.                             *
 *                                                                          *
 * Note: The following features can also be enabled via dipswitches.        *
 *                                                                          *
 * PLAYER SELECT: After inserting a coin, input                             *
 *       UP  UP  DOWN  DOWN  LEFT  RIGHT  LEFT  RIGHT  B  A                 *
 *       You can select a single character instead of a team.               *
 * GUEST PLAYERS: After inserting a coin, input                             *
 *       UP  UP  DOWN  DOWN  LEFT  RIGHT  LEFT  RIGHT  A  B                 *
 *       You can use Mahou Daisakusen and Battle Garegga characters.        *
 * SPECIAL COURSE: After inserting a coin, input                            *
 *       UP  DOWN  UP  DOWN  LEFT  RIGHT  LEFT  RIGHT  A  B                 *
 *       You can select the Special course, which consists of bosses only.  *
 * STAGE EDIT: When you select your course, press A and B simultaneously.   *
 *       You can choose what order to play Stage 2, 3 and 4 in,             *
 *       or even skip them.                                                 *
 ****************************************************************************


 ############################################################################
 # In Battle Bakraid, the button you use to select your ship not only       #
 # determines its color, but affects its characteristics.                   #
 #     A: Increased main shot power. Default autofire rate is 20 Hz.        #
 #     B: Increased bomb blast duration. Default autofire rate is 12 Hz.    #
 #     C: Increased side shot power. Default autofire rate is 15 Hz.        #
 # START: Increased speed. Default autofire rate is 10 Hz.                  #
 #                                                                          #
 # STAGE EDIT: When you select your course, press A and B simultaneously.   #
 #        You can choose what order to play Stage 2, 3, 4 and 5 in,         #
 #        or even skip them. Stage Edit can also be enabled via dipswitch.  #
 # ======================================================================== #
 # Battle Bakraid has unlocking codes to gain access to extra players       #
 # and game features. Once each feature is unlocked, it is saved in EEPROM  #
 # and remains unlocked until you erase the EEPROM from the service mode.   #
 # However, in the original (non-Unlimited) version, the unlocking of       #
 # Team Edit is not saved in EEPROM, apparently due to a bug.               #
 # Special thanks go to the 'R8ZING Shooter Tribute' page for finding       #
 # and publishing this info.                                                #
 # ======================================================================== #
 #      PLAYER SELECT: PHASE 2                                              #
 # Result:  3 more fighter planes available.                                #
 # Code:    UP  UP  DOWN  DOWN  LEFT  RIGHT  LEFT  RIGHT  A  B  Start       #
 # Conditions:                                                              #
 #      1. Start from the title screen                                      #
 #      2. Insert Coin                                                      #
 #      3. Watch the 20 sec. counter and enter each part of the code right  #
 #         between the counting.                                            #
 # Example: 12,up,11,up,10,down,9,down,8,left,7,right,6.left,5,r..          #
 # After entering the [B] button a chime should sound. Phase 2 unlocked!    #
 # ------------------------------------------------------------------------ #
 #      PLAYER SELECT: PHASE 3                                              #
 # Result:  2 more fighter planes available.                                #
 # Code:    UP  UP  DOWN  DOWN  LEFT  RIGHT  LEFT  RIGHT  B  A  Start       #
 # Conditions:                                                              #
 #      1. Unlock Player Select Phase 2 first                               #
 #      2. Insert Coin                                                      #
 #      3. Watch the 20 sec. counter and enter each part of the code right  #
 #         between the counting.                                            #
 # Important: The entering of this code has to be finished before the       #
 # counter passes 10 ! To do so, you will have to start after coin          #
 # insertion, right before it starts to count:                              #
 # Example: up,19,up,18,down,17,down,16,left,15,right,14.left,..            #
 # After entering the [A] button a chime should sound. Phase 3 unlocked!    #
 # ------------------------------------------------------------------------ #
 #      TEAM EDIT: ENABLE                                                   #
 # Result:  Unlocks the 'team edit' feature to select a team of different   #
 #          ships like in Batrider.                                         #
 # Code:    UP  DOWN  UP  DOWN  LEFT  RIGHT  LEFT  RIGHT  A  B  Start       #
 # Conditions:                                                              #
 #      1. Unlock Player Select Phase 2 and Phase 3 first                   #
 #      2. Insert Coin                                                      #
 #      3. Watch the 20 sec. counter and enter each part of the code right  #
 #         between the counting.                                            #
 # Important: This code hast to be entered so that the counter is at 0 when #
 # you press the final button [B]. To do so, start after second 9:          #
 # Example: 9,up,8,down,7,up,6,down,5,left,4,right,3,left,2,right,1,A,0,B   #
 # After entering the [B] button a chime should sound. Team edit unlocked!  #
 #                                                                          #
 # Note: In the Japan version, to use Team Edit after unlocking it,         #
 #       you must hold UP or DOWN  while selecting your course.             #
 #       In the USA version, if Team Edit is unlocked, the game asks you    #
 #       if you want to use it after you select your course.                #
 # ------------------------------------------------------------------------ #
 #      SPECIAL COURSE: ENABLE                                              #
 # Result:  Unlocks the Special course, a game mode where you fight the     #
 #          bosses only.                                                    #
 # Code:    UP  DOWN  UP  DOWN  LEFT  RIGHT  LEFT  RIGHT  B  A  Start       #
 # Conditions:                                                              #
 #      1. Start from the title screen                                      #
 #      2. Hold [C] button                                                  #
 #      3. Insert Coin                                                      #
 #      4. Watch the 20 sec. counter and enter each part of the code right  #
 #         between the counting.                                            #
 #      5. Release [C] button                                               #
 # After entering the [A] button a chime should sound. Special course       #
 # unlocked!                                                                #
 ############################################################################



*************** Hardware Info ***************

CPU:
 MC68000P10
 TMP68HC000N-16

Sound CPU/MCU:
 HD647180X0FS6 (Hitachi Z180 Compatible CPU with internal 16k ROM)
 Z84C0006PEC (Z80)
 NEC V25

Sound Chips:
 YM3812 + YM3014B (DAC)
 YM2151 + YM3014B
 YM2151 + YM3014B + M6295
 YM2151 + YM3012 (DAC) + M6295 + M6295
 YMZ280B-F + YAC516-E (DAC)

Graphics Custom 208pin QFP:
 GP9001 L7A0498 TOA PLAN
 (emulated in toaplan/gp9001.cpp)

*********************************************************************

Game status:

Teki Paki                      Working. MCU type is a Hitachi HD647180.
Ghox                           Working, MCU type is a Hitachi HD647180.
Dogyuun                        Working. MCU type is a NEC V25. Chip is a PLCC94 stamped 'TS-002-MACH'.*
Knuckle Bash                   Working. MCU type is a NEC V25. Chip is a PLCC94 stamped 'TS-004-DASH'.*
Truxton 2                      Working.
Pipi & Bibis                   Working.
Pipi & Bibis (Teki Paki h/w)   Working, MCU type is a Hitachi HD647180.
Pipi & Bibis bootleg           Working. One unknown ROM.
FixEight                       Working. MCU type is a NEC V25. Chip is a PLCC94 stamped 'TS-001-TURBO'
FixEight bootleg               Working. One unknown ROM (same as pipibibs bootleg one). Region hardcoded to Korea (@ $4d8)
Grind Stormer / VFive          Working. MCU type is a NEC V25. Chip is a PLCC94 stamped 'TS-007-SPY'.*
Batsugun / Batsugun Sp'        Working. MCU type is a NEC V25. Chip is a PLCC94 stamped 'TS-007-SPY'.*
Snow Bros. 2                   Working.
Mahou Daisakusen               Working.
Shippu Mahou Daisakusen        Working.
Battle Garegga                 Working.
Armed Police Batrider          Working.
Battle Bakraid                 Working.

* Some PCBs use another version stamped 'NITRO' which is the same chip type.
  MACH, DASH and SPY seem to be the same chip (same encryption table)
  Batsugun has the CPU hooked up in non-encrypted mode.

Notes:
    See Input Port definition header below, for instructions
      on how to enter pause/slow motion modes.

To Do / Unknowns:
    - Whoopee/Teki Paki sometimes tests bit 5 of the region jumper port
        just after testing for vblank. Why?
    - Priority problem on 2nd player side of selection screen in Fixeight bootleg.
    - Fixeight bootleg text in sound check mode does not display properly
        with the CPU set to 10MHz (ok at 16MHz). Possible error in video_count_r routine.
    - Need to sort out the video status register.
    - Find out how exactly how sound CPU communication really works in bgaregga/batrider/bbakraid
        current emulation seems to work (plays all sounds), but there are still some unknown reads/writes
    - Music timing is bit different on bbakraid.
        reference : https://www.youtube.com/watch?v=zjrWs0iHQ5A

Notes on Power Kick coin inputs:
- The 10 yen input is "Key In" according to the bookkeeping screen, but is
  an otherwise normal coin input with a counter and a lockout (sharing the
  latter with the "medal" coin).
- The 100 yen input never adds any credits except in "Coin Function Check,"
  instead dispensing its value into the hopper immediately.

To reset the NVRAM in Othello Derby, hold P1 Button 1 down while booting.

*****************************************************************************/


#include "emu.h"
#include "toaplan2.h"
#include "toaplipt.h"

#include "cpu/nec/v25.h"
#include "cpu/z80/z80.h"
#include "cpu/z180/hd647180x.h"
#include "machine/nvram.h"
#include "sound/ymopm.h"
#include "sound/ymopl.h"
#include "sound/ymz280b.h"
#include "speaker.h"


#define PWRKICK_HOPPER_PULSE    50          // time between hopper pulses in milliseconds (probably wrong)

//#define TRUXTON2_STEREO       /* Uncomment to hear truxton2 music in stereo */

constexpr unsigned toaplan2_state::T2PALETTE_LENGTH;


/***************************************************************************
  Initialisation handlers
***************************************************************************/


void ghox_state::machine_start()
{
	toaplan2_state::machine_start();
	save_item(NAME(m_old_paddle_h));
}


void truxton2_state::machine_start()
{
	toaplan2_state::machine_start();
	save_item(NAME(m_z80_busreq));
}


void toaplan2_state::toaplan2_reset(int state)
{
	if (m_audiocpu != nullptr)
		m_audiocpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}


void ghox_state::machine_reset()
{
	toaplan2_state::machine_reset();
	m_old_paddle_h[0] = 0;
	m_old_paddle_h[1] = 0;
}


MACHINE_RESET_MEMBER(truxton2_state, bgaregga)
{
	toaplan2_state::machine_reset();
	for (int chip = 0; chip < 2; chip++)
	{
		for (int i = 0; i < 8; i++)
		{
			if (m_raizing_okibank[chip][i] != nullptr)
				m_raizing_okibank[chip][i]->set_entry(0);
		}
	}
}


void toaplan2_state::init_dogyuun()
{
	m_sound_reset_bit = 0x20;
}


void truxton2_state::init_fixeight()
{
	m_sound_reset_bit = 0x08;
}


void toaplan2_state::init_fixeightbl()
{
	u8 *ROM = memregion("oki1")->base();

	m_okibank->configure_entries(0, 5, &ROM[0x30000], 0x10000);
}


void toaplan2_state::init_vfive()
{
	m_sound_reset_bit = 0x10;
}


void toaplan2_state::init_pipibibsbl()
{
	u16 *ROM = (u16 *)(memregion("maincpu")->base());

	for (int i = 0; i < (0x040000/2); i += 4)
	{
		ROM[i+0] = bitswap<16>(ROM[i+0],0x1,0x5,0x6,0x7,0x8,0x2,0x0,0x9,0xe,0xd,0x4,0x3,0xf,0xa,0xb,0xc);
		ROM[i+1] = bitswap<16>(ROM[i+1],0x5,0x3,0x1,0xf,0xd,0xb,0x9,0x0,0x2,0x4,0x6,0x8,0xa,0xc,0xe,0x7);
		ROM[i+2] = bitswap<16>(ROM[i+2],0xc,0xd,0xe,0xf,0x8,0x9,0xa,0xb,0x3,0x2,0x1,0x0,0x7,0x6,0x5,0x4);
		ROM[i+3] = bitswap<16>(ROM[i+3],0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,0x3,0x2,0x1,0x0,0x7,0x6,0x5,0x4);
	}
}

void truxton2_state::install_raizing_okibank(int chip)
{
	assert(m_oki_rom[chip] && m_raizing_okibank[chip][0]);

	for (int i = 0; i < 4; i++)
	{
		m_raizing_okibank[chip][i]->configure_entries(0, 16, &m_oki_rom[chip][(i * 0x100)], 0x10000);
	}
	m_raizing_okibank[chip][4]->configure_entries(0, 16, &m_oki_rom[chip][0x400], 0x10000);
	for (int i = 5; i < 8; i++)
	{
		m_raizing_okibank[chip][i]->configure_entries(0, 16, &m_oki_rom[chip][0], 0x10000);
	}
}

void truxton2_state::init_bgaregga()
{
	u8 *Z80 = memregion("audiocpu")->base();

	m_audiobank->configure_entries(0, 8, Z80, 0x4000); // Test mode only, Mirror of First 128KB Areas?
	m_audiobank->configure_entries(8, 8, Z80, 0x4000);
	install_raizing_okibank(0);
}


void truxton2_state::init_batrider()
{
	u8 *Z80 = memregion("audiocpu")->base();

	m_audiobank->configure_entries(0, 16, Z80, 0x4000);
	install_raizing_okibank(0);
	install_raizing_okibank(1);
	m_sndirq_line = 4;
}


void truxton2_state::init_bbakraid()
{
	m_sndirq_line = 2;
}

void toaplan2_state::init_enmadaio()
{
	u8 *ROM = memregion("oki1")->base();

	m_okibank->configure_entries(0, 0x60, ROM, 0x40000);
	m_okibank->set_entry(0);
}


/***************************************************************************
  Toaplan games
***************************************************************************/

void toaplan2_state::cpu_space_fixeightbl_map(address_map &map)
{
	map(0xfffff0, 0xffffff).m(m_maincpu, FUNC(m68000_base_device::autovectors_map));
	map(0xfffff5, 0xfffff5).lr8(NAME([this] () { m_maincpu->set_input_line(M68K_IRQ_2, CLEAR_LINE); return m68000_device::autovector(2); }));
}

void toaplan2_state::cpu_space_pipibibsbl_map(address_map &map)
{
	map(0xfffff0, 0xffffff).m(m_maincpu, FUNC(m68000_base_device::autovectors_map));
	map(0xfffff9, 0xfffff9).lr8(NAME([this] () { m_maincpu->set_input_line(M68K_IRQ_4, CLEAR_LINE); return m68000_device::autovector(4); }));
}


u16 toaplan2_state::video_count_r()
{
	/* +---------+---------+--------+---------------------------+ */
	/* | /H-Sync | /V-Sync | /Blank |       Scanline Count      | */
	/* | Bit 15  | Bit 14  | Bit 8  |  Bit 7-0 (count from #EF) | */
	/* +---------+---------+--------+---------------------------+ */
	/*************** Control Signals are active low ***************/

	int vpos = m_screen->vpos();

	u16 video_status = 0xff00;    // Set signals inactive

	vpos = (vpos + 15) % 262;

	if (!m_vdp[0]->hsync_r())
		video_status &= ~0x8000;
	if (!m_vdp[0]->vsync_r())
		video_status &= ~0x4000;
	if (!m_vdp[0]->fblank_r())
		video_status &= ~0x0100;
	if (vpos < 256)
		video_status |= (vpos & 0xff);
	else
		video_status |= 0xff;

//  logerror("VC: vpos=%04x hpos=%04x VBL=%04x\n",vpos,hpos,m_screen->vblank());

	return video_status;
}


void toaplan2_state::coin_w(u8 data)
{
	/* +----------------+------ Bits 7-5 not used ------+--------------+ */
	/* | Coin Lockout 2 | Coin Lockout 1 | Coin Count 2 | Coin Count 1 | */
	/* |     Bit 3      |     Bit 2      |     Bit 1    |     Bit 0    | */

	if (data & 0x0f)
	{
		machine().bookkeeping().coin_lockout_w(0, BIT(~data, 2));
		machine().bookkeeping().coin_lockout_w(1, BIT(~data, 3));
		machine().bookkeeping().coin_counter_w(0, BIT( data, 0));
		machine().bookkeeping().coin_counter_w(1, BIT( data, 1));
	}
	else
	{
		machine().bookkeeping().coin_lockout_global_w(1);    // Lock all coin slots
	}
	if (data & 0xf0)
	{
		logerror("Writing unknown upper bits (%02x) to coin control\n",data);
	}
}

void pwrkick_state::pwrkick_coin_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 1)); // medal
	machine().bookkeeping().coin_counter_w(1, BIT(data, 3)); // 10 yen
	machine().bookkeeping().coin_counter_w(2, BIT(data, 0)); // 100 yen
	m_hopper->motor_w(BIT(data, 7));
}

void pwrkick_state::pwrkick_coin_lockout_w(u8 data)
{
	machine().bookkeeping().coin_lockout_w(0, BIT(~data, 2));
	machine().bookkeeping().coin_lockout_w(1, BIT(~data, 2));
	machine().bookkeeping().coin_lockout_w(2, BIT(~data, 1));
}


void toaplan2_state::coin_sound_reset_w(u8 data)
{
	logerror("coin_sound_reset_w %02x\n",data);

	coin_w(data & ~m_sound_reset_bit);
	sound_reset_w(data & m_sound_reset_bit);
}


void truxton2_state::shippumd_coin_w(u8 data)
{
	coin_w(data & ~0x10);
	m_oki[0]->set_rom_bank(BIT(data, 4));
}


u8 toaplan2_state::shared_ram_r(offs_t offset)
{
	return m_shared_ram[offset];
}


void toaplan2_state::shared_ram_w(offs_t offset, u8 data)
{
	m_shared_ram[offset] = data;
}


int toaplan2_state::c2map_r()
{
	// For Teki Paki hardware
	// bit 4 high signifies secondary CPU is ready
	// bit 5 is tested low before V-Blank bit ???

	return m_soundlatch[0]->pending_r() ? 0x00 : 0x01;
}


template<unsigned Which>
u16 ghox_state::ghox_h_analog_r()
{
	const s8 new_value = m_io_pad[Which]->read();
	const s8 result = new_value - m_old_paddle_h[Which];
	if (!machine().side_effects_disabled())
		m_old_paddle_h[Which] = new_value;
	return result;
}


void toaplan2_state::sound_reset_w(u8 data)
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, (data & m_sound_reset_bit) ? CLEAR_LINE : ASSERT_LINE);
}

template<int Chip>
void toaplan2_state::oki_bankswitch_w(u8 data)
{
	m_oki[Chip]->set_rom_bank(data & 1);
}

void toaplan2_state::fixeightbl_oki_bankswitch_w(u8 data)
{
	data &= 7;
	if (data <= 4) m_okibank->set_entry(data);
}


/***************************************************************************
  Raizing games
***************************************************************************/


void truxton2_state::raizing_z80_bankswitch_w(u8 data)
{
	m_audiobank->set_entry(data & 0x0f);
}


// bgaregga and batrider don't actually have a NMK112, but rather a GAL
// programmed to bankswitch the sound ROMs in a similar fashion.
// it may not be a coincidence that the composer and sound designer for
// these two games, Manabu "Santaruru" Namiki, came to Raizing from NMK...

void truxton2_state::raizing_oki_bankswitch_w(offs_t offset, u8 data)
{
	m_raizing_okibank[(offset & 4) >> 2][offset & 3]->set_entry(data & 0xf);
	m_raizing_okibank[(offset & 4) >> 2][4 + (offset & 3)]->set_entry(data & 0xf);
	offset++;
	data >>= 4;
	m_raizing_okibank[(offset & 4) >> 2][offset & 3]->set_entry(data & 0xf);
	m_raizing_okibank[(offset & 4) >> 2][4 + (offset & 3)]->set_entry(data & 0xf);
}


u8 truxton2_state::bgaregga_E01D_r()
{
	// the Z80 reads this address during its IRQ routine,
	// and reads the soundlatch only if the lowest bit is clear.
	return m_soundlatch[0]->pending_r() ? 0 : 1;
}


u16 truxton2_state::batrider_z80_busack_r()
{
	// Bit 0x01 returns the status of BUSAK from the Z80.
	// These accesses are made when the 68K wants to read the Z80
	// ROM code. Failure to return the correct status incurrs a Sound Error.

	return m_z80_busreq;    // Loop BUSRQ to BUSAK
}


void truxton2_state::batrider_z80_busreq_w(u8 data)
{
	m_z80_busreq = (data & 0x01);   // see batrider_z80_busack_r above
}


u16 truxton2_state::batrider_z80rom_r(offs_t offset)
{
	return m_z80_rom[offset];
}

// these two latches are always written together, via a single move.l instruction
void truxton2_state::batrider_soundlatch_w(u8 data)
{
	m_soundlatch[0]->write(data & 0xff);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}


void truxton2_state::batrider_soundlatch2_w(u8 data)
{
	m_soundlatch[1]->write(data & 0xff);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

void truxton2_state::batrider_unknown_sound_w(u16 data)
{
	// the 68K writes here when it wants a sound acknowledge IRQ from the Z80
	// for bbakraid this is on every sound command; for batrider, only on certain commands
}


void truxton2_state::batrider_clear_sndirq_w(u16 data)
{
	// not sure whether this is correct
	// the 68K writes here during the sound IRQ handler, and nowhere else...
	m_maincpu->set_input_line(m_sndirq_line, CLEAR_LINE);
}


void truxton2_state::batrider_sndirq_w(u8 data)
{
	// if batrider_clear_sndirq_w() is correct, should this be ASSERT_LINE?
	m_maincpu->set_input_line(m_sndirq_line, HOLD_LINE);
}


void truxton2_state::batrider_clear_nmi_w(u8 data)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}


u16 truxton2_state::bbakraid_eeprom_r()
{
	// Bit 0x01 returns the status of BUSAK from the Z80.
	// BUSRQ is activated via bit 0x10 on the EEPROM write port.
	// These accesses are made when the 68K wants to read the Z80
	// ROM code. Failure to return the correct status incurrs a Sound Error.

	u8 data;
	data  = ((m_eeprom->do_read() & 0x01) << 4);
	data |= ((m_z80_busreq >> 4) & 0x01);   // Loop BUSRQ to BUSAK

	return data;
}


void truxton2_state::bbakraid_eeprom_w(u8 data)
{
	if (data & ~0x1f)
		logerror("CPU #0 PC:%06X - Unknown EEPROM data being written %02X\n",m_maincpu->pc(),data);

	m_eepromout->write(data, 0xff);

	m_z80_busreq = data & 0x10; // see bbakraid_eeprom_r above
}


INTERRUPT_GEN_MEMBER(truxton2_state::bbakraid_snd_interrupt)
{
	device.execute().set_input_line(0, HOLD_LINE);
}


void toaplan2_state::tekipaki_68k_mem(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x020000, 0x03ffff).rom();                     // extra for Whoopee
	map(0x080000, 0x082fff).ram();
	map(0x0c0000, 0x0c0fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x140000, 0x14000d).rw(m_vdp[0], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x180000, 0x180001).portr("DSWA");
	map(0x180010, 0x180011).portr("DSWB");
	map(0x180020, 0x180021).portr("SYS");
	map(0x180030, 0x180031).portr("JMPR");           // CPU 2 busy and Region Jumper block
	map(0x180041, 0x180041).w(FUNC(toaplan2_state::coin_w));
	map(0x180050, 0x180051).portr("IN1");
	map(0x180060, 0x180061).portr("IN2");
	map(0x180071, 0x180071).w(m_soundlatch[0], FUNC(generic_latch_8_device::write));
}


void ghox_state::ghox_68k_mem(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x040000, 0x040001).r(FUNC(ghox_state::ghox_h_analog_r<1>));
	map(0x080000, 0x083fff).ram();
	map(0x0c0000, 0x0c0fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x100000, 0x100001).r(FUNC(ghox_state::ghox_h_analog_r<0>));
	map(0x140000, 0x14000d).rw(m_vdp[0], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x180000, 0x180fff).rw(FUNC(ghox_state::shared_ram_r), FUNC(ghox_state::shared_ram_w)).umask16(0x00ff);
	map(0x181001, 0x181001).w(FUNC(ghox_state::coin_w));
	map(0x18100c, 0x18100d).portr("JMPR");
}


void toaplan2_state::dogyuun_68k_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x103fff).ram();
	map(0x200010, 0x200011).portr("IN1");
	map(0x200014, 0x200015).portr("IN2");
	map(0x200018, 0x200019).portr("SYS");
	map(0x20001d, 0x20001d).w(FUNC(toaplan2_state::coin_sound_reset_w)); // Coin count/lock + v25 reset line
	map(0x210000, 0x21ffff).rw(FUNC(toaplan2_state::shared_ram_r), FUNC(toaplan2_state::shared_ram_w)).umask16(0x00ff);
	map(0x300000, 0x30000d).rw(m_vdp[0], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x400000, 0x400fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x500000, 0x50000d).rw(m_vdp[1], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x700000, 0x700001).r(FUNC(toaplan2_state::video_count_r));         // test bit 8
}


void toaplan2_state::dogyuunto_68k_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x103fff).ram();
	map(0x218000, 0x218fff).rw(FUNC(toaplan2_state::shared_ram_r), FUNC(toaplan2_state::shared_ram_w)).umask16(0x00ff); // reads the same area as the finished game on startup, but then uses only this part
	map(0x21c01d, 0x21c01d).w(FUNC(toaplan2_state::coin_sound_reset_w)); // Coin count/lock + Z80 reset line
	map(0x21c020, 0x21c021).portr("IN1");
	map(0x21c024, 0x21c025).portr("IN2");
	map(0x21c028, 0x21c029).portr("SYS");
	map(0x21c02c, 0x21c02d).portr("DSWA");
	map(0x21c030, 0x21c031).portr("DSWB");
	map(0x300000, 0x30000d).rw(m_vdp[0], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x400000, 0x400fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x500000, 0x50000d).rw(m_vdp[1], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x700000, 0x700001).r(FUNC(toaplan2_state::video_count_r));         // test bit 8
}


void toaplan2_state::kbash_68k_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x103fff).ram();
	map(0x200000, 0x200fff).rw(FUNC(toaplan2_state::shared_ram_r), FUNC(toaplan2_state::shared_ram_w)).umask16(0x00ff);
	map(0x208010, 0x208011).portr("IN1");
	map(0x208014, 0x208015).portr("IN2");
	map(0x208018, 0x208019).portr("SYS");
	map(0x20801d, 0x20801d).w(FUNC(toaplan2_state::coin_w));
	map(0x300000, 0x30000d).rw(m_vdp[0], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x400000, 0x400fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x700000, 0x700001).r(FUNC(toaplan2_state::video_count_r));         // test bit 8
}


void toaplan2_state::kbash2_68k_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x103fff).ram();
	map(0x104000, 0x10401f).ram();         // Sound related?
	map(0x200000, 0x200001).noprw();         // Left over from original code - Sound Number write, Status read
	map(0x200002, 0x200003).nopw();    // Left over from original code - Reset Sound
	map(0x200004, 0x200005).portr("DSWA");
	map(0x200008, 0x200009).portr("DSWB");
	map(0x20000c, 0x20000d).portr("JMPR");
	map(0x200010, 0x200011).portr("IN1");
	map(0x200014, 0x200015).portr("IN2");
	map(0x200018, 0x200019).portr("SYS");
	map(0x200021, 0x200021).rw(m_oki[1], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x200025, 0x200025).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x200029, 0x200029).w(FUNC(toaplan2_state::oki_bankswitch_w<0>));
	map(0x20002c, 0x20002d).r(FUNC(toaplan2_state::video_count_r));
	map(0x300000, 0x30000d).rw(m_vdp[0], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x400000, 0x400fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
}


void truxton2_state::truxton2_68k_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x200000, 0x20000d).rw(m_vdp[0], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x300000, 0x300fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x400000, 0x401fff).ram().w(FUNC(truxton2_state::tx_videoram_w)).share(m_tx_videoram);
	map(0x402000, 0x402fff).ram().share(m_tx_lineselect);
	map(0x403000, 0x4031ff).ram().w(FUNC(truxton2_state::tx_linescroll_w)).share(m_tx_linescroll);
	map(0x403200, 0x403fff).ram();
	map(0x500000, 0x50ffff).ram().w(FUNC(truxton2_state::tx_gfxram_w)).share(m_tx_gfxram);
	map(0x600000, 0x600001).r(FUNC(truxton2_state::video_count_r));
	map(0x700000, 0x700001).portr("DSWA");
	map(0x700002, 0x700003).portr("DSWB");
	map(0x700004, 0x700005).portr("JMPR");
	map(0x700006, 0x700007).portr("IN1");
	map(0x700008, 0x700009).portr("IN2");
	map(0x70000a, 0x70000b).portr("SYS");
	map(0x700011, 0x700011).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x700014, 0x700017).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write)).umask16(0x00ff);
	map(0x70001f, 0x70001f).w(FUNC(truxton2_state::coin_w));
}


void toaplan2_state::pipibibs_68k_mem(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x080000, 0x082fff).ram();
	map(0x0c0000, 0x0c0fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x140000, 0x14000d).rw(m_vdp[0], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x190000, 0x190fff).rw(FUNC(toaplan2_state::shared_ram_r), FUNC(toaplan2_state::shared_ram_w)).umask16(0x00ff);
	map(0x19c01d, 0x19c01d).w(FUNC(toaplan2_state::coin_w));
	map(0x19c020, 0x19c021).portr("DSWA");
	map(0x19c024, 0x19c025).portr("DSWB");
	map(0x19c028, 0x19c029).portr("JMPR");
	map(0x19c02c, 0x19c02d).portr("SYS");
	map(0x19c030, 0x19c031).portr("IN1");
	map(0x19c034, 0x19c035).portr("IN2");
}

// odd scroll registers
void toaplan2_state::pipibibi_bootleg_68k_mem(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x080000, 0x082fff).ram();
	map(0x083000, 0x0837ff).rw(m_vdp[0], FUNC(gp9001vdp_device::bootleg_spriteram16_r), FUNC(gp9001vdp_device::bootleg_spriteram16_w));   // SpriteRAM
	map(0x083800, 0x087fff).ram();             // SpriteRAM (unused)
	map(0x0c0000, 0x0c0fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x120000, 0x120fff).ram();             // Copy of SpriteRAM ?
//  map(0x13f000, 0x13f001).nopw();        // ???
	map(0x180000, 0x182fff).rw(m_vdp[0], FUNC(gp9001vdp_device::bootleg_videoram16_r), FUNC(gp9001vdp_device::bootleg_videoram16_w)); // TileRAM
	map(0x188000, 0x18800f).w(m_vdp[0], FUNC(gp9001vdp_device::bootleg_scroll_w));
	map(0x190003, 0x190003).r(FUNC(toaplan2_state::shared_ram_r));  // Z80 ready ?
	map(0x190011, 0x190011).w(FUNC(toaplan2_state::shared_ram_w)); // Z80 task to perform
	map(0x19c01d, 0x19c01d).w(FUNC(toaplan2_state::coin_w));
	map(0x19c020, 0x19c021).portr("DSWA");
	map(0x19c024, 0x19c025).portr("DSWB");
	map(0x19c028, 0x19c029).portr("JMPR");
	map(0x19c02c, 0x19c02d).portr("SYS");
	map(0x19c030, 0x19c031).portr("IN1");
	map(0x19c034, 0x19c035).portr("IN2");
}


void truxton2_state::fixeight_68k_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x103fff).ram();
	map(0x200000, 0x200001).portr("IN1");
	map(0x200004, 0x200005).portr("IN2");
	map(0x200008, 0x200009).portr("IN3");
	map(0x200010, 0x200011).portr("SYS");
	map(0x20001d, 0x20001d).w(FUNC(truxton2_state::coin_w));
	map(0x280000, 0x28ffff).rw(FUNC(truxton2_state::shared_ram_r), FUNC(truxton2_state::shared_ram_w)).umask16(0x00ff);
	map(0x300000, 0x30000d).rw(m_vdp[0], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x400000, 0x400fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x500000, 0x501fff).ram().w(FUNC(truxton2_state::tx_videoram_w)).share(m_tx_videoram);
	map(0x502000, 0x5021ff).ram().share(m_tx_lineselect);
	map(0x503000, 0x5031ff).ram().w(FUNC(truxton2_state::tx_linescroll_w)).share(m_tx_linescroll);
	map(0x600000, 0x60ffff).ram().w(FUNC(truxton2_state::tx_gfxram_w)).share(m_tx_gfxram);
	map(0x700000, 0x700001).w(FUNC(truxton2_state::sound_reset_w)).umask16(0x00ff).cswidth(16);
	map(0x800000, 0x800001).r(FUNC(truxton2_state::video_count_r));
}


void truxton2_state::fixeightbl_68k_mem(address_map &map)
{
	map(0x000000, 0x0fffff).rom();     // 0-7ffff ?
	map(0x100000, 0x10ffff).ram();     // 100000-107fff  105000-105xxx  106000-106xxx  108000 - related to sound ?
	map(0x200000, 0x200001).portr("IN1");
	map(0x200004, 0x200005).portr("IN2");
	map(0x200008, 0x200009).portr("IN3");
	map(0x20000c, 0x20000d).portr("DSWB");
	map(0x200010, 0x200011).portr("SYS");
	map(0x200015, 0x200015).w(FUNC(truxton2_state::fixeightbl_oki_bankswitch_w));  // Sound banking. Code at $4084c, $5070
	map(0x200019, 0x200019).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x20001c, 0x20001d).portr("DSWA");
	map(0x300000, 0x30000d).rw(m_vdp[0], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x400000, 0x400fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x500000, 0x501fff).ram().w(FUNC(truxton2_state::tx_videoram_w)).share(m_tx_videoram);
	map(0x700000, 0x700001).r(FUNC(truxton2_state::video_count_r));
	map(0x800000, 0x87ffff).rom().region("maincpu", 0x80000);
}


void toaplan2_state::vfive_68k_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x103fff).ram();
//  map(0x200000, 0x20ffff).noprw(); // Read at startup by broken ROM checksum code - see notes
	map(0x200010, 0x200011).portr("IN1");
	map(0x200014, 0x200015).portr("IN2");
	map(0x200018, 0x200019).portr("SYS");
	map(0x20001d, 0x20001d).w(FUNC(toaplan2_state::coin_sound_reset_w)); // Coin count/lock + v25 reset line
	map(0x210000, 0x21ffff).rw(FUNC(toaplan2_state::shared_ram_r), FUNC(toaplan2_state::shared_ram_w)).umask16(0x00ff);
	map(0x300000, 0x30000d).rw(m_vdp[0], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x400000, 0x400fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x700000, 0x700001).r(FUNC(toaplan2_state::video_count_r));
}


void toaplan2_state::batsugun_68k_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x200010, 0x200011).portr("IN1");
	map(0x200014, 0x200015).portr("IN2");
	map(0x200018, 0x200019).portr("SYS");
	map(0x20001d, 0x20001d).w(FUNC(toaplan2_state::coin_sound_reset_w)); // Coin count/lock + v25 reset line
	map(0x210000, 0x21ffff).rw(FUNC(toaplan2_state::shared_ram_r), FUNC(toaplan2_state::shared_ram_w)).umask16(0x00ff);
	map(0x300000, 0x30000d).rw(m_vdp[0], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x400000, 0x400fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x500000, 0x50000d).rw(m_vdp[1], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x700000, 0x700001).r(FUNC(toaplan2_state::video_count_r));
}

void toaplan2_state::batsugunbl_68k_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram();
	// map(0x200004, 0x200005).r() // only cleared at boot?
	map(0x200005, 0x200005).w(FUNC(toaplan2_state::fixeightbl_oki_bankswitch_w)); // TODO: doesn't sound correct
	map(0x200009, 0x200009).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x200010, 0x200011).portr("IN1");
	map(0x200014, 0x200015).portr("IN2");
	map(0x200018, 0x200019).portr("SYS");
	map(0x20001c, 0x20001d).nopw(); // leftover code from the original?
	map(0x21f004, 0x21f005).portr("DSWA");
	map(0x21f006, 0x21f007).portr("DSWB");
	map(0x21f008, 0x21f009).portr("JMPR");
	map(0x300000, 0x30000d).rw(m_vdp[0], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x400000, 0x400fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x500000, 0x50000d).rw(m_vdp[1], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x700000, 0x700001).r(FUNC(toaplan2_state::video_count_r));
}

void pwrkick_state::pwrkick_68k_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x103fff).ram().share("nvram"); // Only 10022C-10037B is actually saved as NVRAM
	map(0x104000, 0x10ffff).ram();

	map(0x200000, 0x20000f).rw(m_rtc, FUNC(upd4992_device::read), FUNC(upd4992_device::write)).umask16(0x00ff);
	map(0x300000, 0x30000d).rw(m_vdp[0], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x400000, 0x400fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x600001, 0x600001).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));

	map(0x700000, 0x700001).r(FUNC(pwrkick_state::video_count_r));
	map(0x700004, 0x700005).portr("DSWA");
	map(0x700008, 0x700009).portr("DSWB");
	map(0x70000c, 0x70000d).portr("IN1");
	map(0x700014, 0x700015).portr("IN2");
	map(0x700018, 0x700019).portr("DSWC");
	map(0x70001c, 0x70001d).portr("SYS");
	map(0x700031, 0x700031).w(FUNC(pwrkick_state::oki_bankswitch_w<0>));
	map(0x700035, 0x700035).w(FUNC(pwrkick_state::pwrkick_coin_w));
	map(0x700039, 0x700039).w(FUNC(pwrkick_state::pwrkick_coin_lockout_w));
}

void pwrkick_state::othldrby_68k_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x103fff).ram().share("nvram"); // Only 10331E-103401 is actually saved as NVRAM
	map(0x104000, 0x10ffff).ram();

	map(0x200000, 0x20000f).rw(m_rtc, FUNC(upd4992_device::read), FUNC(upd4992_device::write)).umask16(0x00ff);
	map(0x300000, 0x30000d).rw(m_vdp[0], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x400000, 0x400fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x600001, 0x600001).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));

	map(0x700000, 0x700001).r(FUNC(pwrkick_state::video_count_r));
	map(0x700004, 0x700005).portr("DSWA");
	map(0x700008, 0x700009).portr("DSWB");
	map(0x70000c, 0x70000d).portr("IN1");
	map(0x700010, 0x700011).portr("IN2");
	map(0x70001c, 0x70001d).portr("SYS");
	map(0x700031, 0x700031).w(FUNC(pwrkick_state::oki_bankswitch_w<0>));
	map(0x700035, 0x700035).w(FUNC(pwrkick_state::coin_w));
}


void toaplan2_state::enmadaio_oki_bank_w(offs_t offset, u16 data, u16 mem_mask)
{
	data &= mem_mask;

	if (data < 0x60)
	{
		m_okibank->set_entry(data);
	}
	else
	{
		logerror("enmadaio_oki_bank_w >=0x60 (%04x)\n",data);
	}
}

void toaplan2_state::enmadaio_68k_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x103fff).ram(); //.share("nvram");
	map(0x104000, 0x10ffff).ram();

	map(0x200000, 0x20000d).rw(m_vdp[0], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x300000, 0x300fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x400000, 0x400003).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write)).umask16(0x00ff);
	map(0x500001, 0x500001).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));

	map(0x700000, 0x700001).r(FUNC(toaplan2_state::video_count_r));

	map(0x700004, 0x700005).portr("DSWA");
	map(0x70000c, 0x70000d).portr("MISC2");
	map(0x700010, 0x700011).portr("MISC3");
	map(0x700014, 0x700015).portr("MISC4");
	map(0x700018, 0x700019).portr("SYS");
	map(0x70001c, 0x70001d).portr("UNK"); //.portr("SYS");

	map(0x700020, 0x700021).w(FUNC(toaplan2_state::enmadaio_oki_bank_w)); // oki bank

	map(0x700028, 0x700029).nopw();
	map(0x70003c, 0x70003d).nopw();
	map(0x70002c, 0x70002d).nopw();
}

void toaplan2_state::snowbro2_68k_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x300000, 0x30000d).rw(m_vdp[0], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x400000, 0x400fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x500000, 0x500003).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write)).umask16(0x00ff);
	map(0x600001, 0x600001).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x700000, 0x700001).portr("JMPR");
	map(0x700004, 0x700005).portr("DSWA");
	map(0x700008, 0x700009).portr("DSWB");
	map(0x70000c, 0x70000d).portr("IN1");
	map(0x700010, 0x700011).portr("IN2");
	map(0x700014, 0x700015).portr("IN3");
	map(0x700018, 0x700019).portr("IN4");
	map(0x70001c, 0x70001d).portr("SYS");
	map(0x700031, 0x700031).w(FUNC(toaplan2_state::oki_bankswitch_w<0>));
	map(0x700035, 0x700035).w(FUNC(toaplan2_state::coin_w));
}

void toaplan2_state::snowbro2b3_68k_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x404000, 0x404fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x500000, 0x500003).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write)).umask16(0x00ff);
	map(0x600001, 0x600001).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x700004, 0x700005).portr("DSWA");
	map(0x700008, 0x700009).portr("DSWB");
	map(0x70000c, 0x70000d).portr("IN1");
	map(0x700010, 0x700011).portr("IN2");
	map(0x700014, 0x700015).portr("IN3");
	map(0x700018, 0x700019).portr("IN4");
	map(0x700035, 0x700035).w(FUNC(toaplan2_state::coin_w));
	map(0x700041, 0x700041).w(FUNC(toaplan2_state::oki_bankswitch_w<0>));
	map(0xff0000, 0xff2fff).rw(m_vdp[0], FUNC(gp9001vdp_device::bootleg_videoram16_r), FUNC(gp9001vdp_device::bootleg_videoram16_w));
	map(0xff3000, 0xff37ff).rw(m_vdp[0], FUNC(gp9001vdp_device::bootleg_spriteram16_r), FUNC(gp9001vdp_device::bootleg_spriteram16_w));
	map(0xff8000, 0xff800f).w(m_vdp[0], FUNC(gp9001vdp_device::bootleg_scroll_w));
}


void truxton2_state::mahoudai_68k_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x218000, 0x21bfff).rw(FUNC(truxton2_state::shared_ram_r), FUNC(truxton2_state::shared_ram_w)).umask16(0x00ff);
	map(0x21c01d, 0x21c01d).w(FUNC(truxton2_state::coin_w));
	map(0x21c020, 0x21c021).portr("IN1");
	map(0x21c024, 0x21c025).portr("IN2");
	map(0x21c028, 0x21c029).portr("SYS");
	map(0x21c02c, 0x21c02d).portr("DSWA");
	map(0x21c030, 0x21c031).portr("DSWB");
	map(0x21c034, 0x21c035).portr("JMPR");
	map(0x21c03c, 0x21c03d).r(FUNC(truxton2_state::video_count_r));
	map(0x300000, 0x30000d).rw(m_vdp[0], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x400000, 0x400fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x401000, 0x4017ff).ram();                         // Unused palette RAM
	map(0x500000, 0x501fff).ram().w(FUNC(truxton2_state::tx_videoram_w)).share(m_tx_videoram);
	map(0x502000, 0x502fff).ram().share(m_tx_lineselect);
	map(0x503000, 0x5031ff).ram().w(FUNC(truxton2_state::tx_linescroll_w)).share(m_tx_linescroll);
	map(0x503200, 0x503fff).ram();
}


void truxton2_state::shippumd_68k_mem(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x218000, 0x21bfff).rw(FUNC(truxton2_state::shared_ram_r), FUNC(truxton2_state::shared_ram_w)).umask16(0x00ff);
//  map(0x21c008, 0x21c009).nopw();                    // ???
	map(0x21c01d, 0x21c01d).w(FUNC(truxton2_state::shippumd_coin_w)); // Coin count/lock + oki bankswitch
	map(0x21c020, 0x21c021).portr("IN1");
	map(0x21c024, 0x21c025).portr("IN2");
	map(0x21c028, 0x21c029).portr("SYS");
	map(0x21c02c, 0x21c02d).portr("DSWA");
	map(0x21c030, 0x21c031).portr("DSWB");
	map(0x21c034, 0x21c035).portr("JMPR");
	map(0x21c03c, 0x21c03d).r(FUNC(truxton2_state::video_count_r));
	map(0x300000, 0x30000d).rw(m_vdp[0], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x400000, 0x400fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x401000, 0x4017ff).ram();                         // Unused palette RAM
	map(0x500000, 0x501fff).ram().w(FUNC(truxton2_state::tx_videoram_w)).share(m_tx_videoram);
	map(0x502000, 0x502fff).ram().share(m_tx_lineselect);
	map(0x503000, 0x5031ff).ram().w(FUNC(truxton2_state::tx_linescroll_w)).share(m_tx_linescroll);
	map(0x503200, 0x503fff).ram();
}


void truxton2_state::bgaregga_68k_mem(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x218000, 0x21bfff).rw(FUNC(truxton2_state::shared_ram_r), FUNC(truxton2_state::shared_ram_w)).umask16(0x00ff);
	map(0x21c01d, 0x21c01d).w(FUNC(truxton2_state::coin_w));
	map(0x21c020, 0x21c021).portr("IN1");
	map(0x21c024, 0x21c025).portr("IN2");
	map(0x21c028, 0x21c029).portr("SYS");
	map(0x21c02c, 0x21c02d).portr("DSWA");
	map(0x21c030, 0x21c031).portr("DSWB");
	map(0x21c034, 0x21c035).portr("JMPR");
	map(0x21c03c, 0x21c03d).r(FUNC(truxton2_state::video_count_r));
	map(0x300000, 0x30000d).rw(m_vdp[0], FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x400000, 0x400fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x500000, 0x501fff).ram().w(FUNC(truxton2_state::tx_videoram_w)).share(m_tx_videoram);
	map(0x502000, 0x502fff).ram().share(m_tx_lineselect);
	map(0x503000, 0x5031ff).ram().w(FUNC(truxton2_state::tx_linescroll_w)).share(m_tx_linescroll);
	map(0x503200, 0x503fff).ram();
	map(0x600001, 0x600001).w(m_soundlatch[0], FUNC(generic_latch_8_device::write));
}


void truxton2_state::batrider_dma_mem(address_map &map)
{
	map(0x0000, 0x1fff).ram().w(FUNC(truxton2_state::tx_videoram_w)).share(m_tx_videoram);
	map(0x2000, 0x2fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x3000, 0x31ff).ram().share(m_tx_lineselect);
	map(0x3200, 0x33ff).ram().w(FUNC(truxton2_state::tx_linescroll_w)).share(m_tx_linescroll);
	map(0x3400, 0x7fff).ram();
	map(0x8000, 0xffff).ram().w(FUNC(truxton2_state::batrider_tx_gfxram_w)).share(m_tx_gfxram);
}


void truxton2_state::batrider_68k_mem(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
	// actually 200000 - 20ffff is probably all main RAM, and the text and palette RAM are written via DMA
	map(0x200000, 0x207fff).ram().share(m_mainram);
	map(0x208000, 0x20ffff).ram();
	map(0x300000, 0x37ffff).r(FUNC(truxton2_state::batrider_z80rom_r));
	map(0x400000, 0x40000d).lrw16(
							NAME([this](offs_t offset, u16 mem_mask) { return m_vdp[0]->read(offset ^ (0xc/2), mem_mask); }),
							NAME([this](offs_t offset, u16 data, u16 mem_mask) { m_vdp[0]->write(offset ^ (0xc/2), data, mem_mask); }));
	map(0x500000, 0x500001).portr("IN");
	map(0x500002, 0x500003).portr("SYS-DSW");
	map(0x500004, 0x500005).portr("DSW");
	map(0x500006, 0x500007).r(FUNC(truxton2_state::video_count_r));
	map(0x500009, 0x500009).r(m_soundlatch[2], FUNC(generic_latch_8_device::read));
	map(0x50000b, 0x50000b).r(m_soundlatch[3], FUNC(generic_latch_8_device::read));
	map(0x50000c, 0x50000d).r(FUNC(truxton2_state::batrider_z80_busack_r));
	map(0x500011, 0x500011).w(FUNC(truxton2_state::coin_w));
	map(0x500021, 0x500021).w(FUNC(truxton2_state::batrider_soundlatch_w));
	map(0x500023, 0x500023).w(FUNC(truxton2_state::batrider_soundlatch2_w));
	map(0x500024, 0x500025).w(FUNC(truxton2_state::batrider_unknown_sound_w));
	map(0x500026, 0x500027).w(FUNC(truxton2_state::batrider_clear_sndirq_w));
	map(0x500061, 0x500061).w(FUNC(truxton2_state::batrider_z80_busreq_w));
	map(0x500080, 0x500081).w(FUNC(truxton2_state::batrider_textdata_dma_w));
	map(0x500082, 0x500083).w(FUNC(truxton2_state::batrider_pal_text_dma_w));
	map(0x5000c0, 0x5000cf).w(FUNC(truxton2_state::batrider_objectbank_w)).umask16(0x00ff);
}


void truxton2_state::bbakraid_68k_mem(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
	// actually 200000 - 20ffff is probably all main RAM, and the text and palette RAM are written via DMA
	map(0x200000, 0x207fff).ram().share(m_mainram);
	map(0x208000, 0x20ffff).ram();
	map(0x300000, 0x33ffff).r(FUNC(truxton2_state::batrider_z80rom_r));
	map(0x400000, 0x40000d).lrw16(
							NAME([this](offs_t offset, u16 mem_mask) { return m_vdp[0]->read(offset ^ (0xc/2), mem_mask); }),
							NAME([this](offs_t offset, u16 data, u16 mem_mask) { m_vdp[0]->write(offset ^ (0xc/2), data, mem_mask); }));
	map(0x500000, 0x500001).portr("IN");
	map(0x500002, 0x500003).portr("SYS-DSW");
	map(0x500004, 0x500005).portr("DSW");
	map(0x500006, 0x500007).r(FUNC(truxton2_state::video_count_r));
	map(0x500009, 0x500009).w(FUNC(truxton2_state::coin_w));
	map(0x500011, 0x500011).r(m_soundlatch[2], FUNC(generic_latch_8_device::read));
	map(0x500013, 0x500013).r(m_soundlatch[3], FUNC(generic_latch_8_device::read));
	map(0x500015, 0x500015).w(FUNC(truxton2_state::batrider_soundlatch_w));
	map(0x500017, 0x500017).w(FUNC(truxton2_state::batrider_soundlatch2_w));
	map(0x500018, 0x500019).r(FUNC(truxton2_state::bbakraid_eeprom_r));
	map(0x50001a, 0x50001b).w(FUNC(truxton2_state::batrider_unknown_sound_w));
	map(0x50001c, 0x50001d).w(FUNC(truxton2_state::batrider_clear_sndirq_w));
	map(0x50001f, 0x50001f).w(FUNC(truxton2_state::bbakraid_eeprom_w));
	map(0x500080, 0x500081).w(FUNC(truxton2_state::batrider_textdata_dma_w));
	map(0x500082, 0x500083).w(FUNC(truxton2_state::batrider_pal_text_dma_w));
	map(0x5000c0, 0x5000cf).w(FUNC(truxton2_state::batrider_objectbank_w)).umask16(0x00ff);
}


void truxton2_state::nprobowl_68k_mem(address_map &map) // TODO: verify everything, implement oki banking
{
	map(0x000000, 0x0fffff).rom();
	map(0x200000, 0x207fff).ram().share(m_mainram);
	map(0x208000, 0x20ffff).ram();
	map(0x400000, 0x40000d).lrw16(
							NAME([this](offs_t offset, u16 mem_mask) { return m_vdp[0]->read(offset ^ (0xc/2), mem_mask); }),
							NAME([this](offs_t offset, u16 data, u16 mem_mask) { m_vdp[0]->write(offset ^ (0xc/2), data, mem_mask); }));
	map(0x500000, 0x500001).portr("IN");
	map(0x500002, 0x500003).portr("UNK");
	map(0x500004, 0x500005).portr("DSW");
	//map(0x500010, 0x500011).w();
	//map(0x500012, 0x500013).w();
	map(0x500021, 0x500021).w(m_oki[0], FUNC(okim6295_device::write));
	//map(0x500040, 0x500041).w();
	//map(0x500042, 0x500043).w();
	map(0x500060, 0x500061).lr16(NAME([this] () -> u16 { return machine().rand(); })); // TODO: Hack, probably checks something in the mechanical part, verify
	map(0x500080, 0x500081).w(FUNC(truxton2_state::batrider_textdata_dma_w));
	map(0x500082, 0x500083).w(FUNC(truxton2_state::batrider_pal_text_dma_w));
}


void toaplan2_state::pipibibs_sound_z80_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram().share(m_shared_ram);
	map(0xe000, 0xe001).rw("ymsnd", FUNC(ym3812_device::read), FUNC(ym3812_device::write));
}


void truxton2_state::raizing_sound_z80_mem(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xdfff).ram().share(m_shared_ram);
	map(0xe000, 0xe001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xe004, 0xe004).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xe00e, 0xe00e).w(FUNC(truxton2_state::coin_w));
}


void truxton2_state::bgaregga_sound_z80_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_audiobank);
	map(0xc000, 0xdfff).ram().share(m_shared_ram);
	map(0xe000, 0xe001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xe004, 0xe004).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xe006, 0xe008).w(FUNC(truxton2_state::raizing_oki_bankswitch_w));
	map(0xe00a, 0xe00a).w(FUNC(truxton2_state::raizing_z80_bankswitch_w));
	map(0xe00c, 0xe00c).w(m_soundlatch[0], FUNC(generic_latch_8_device::acknowledge_w));
	map(0xe01c, 0xe01c).r(m_soundlatch[0], FUNC(generic_latch_8_device::read));
	map(0xe01d, 0xe01d).r(FUNC(truxton2_state::bgaregga_E01D_r));
}


void truxton2_state::batrider_sound_z80_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_audiobank);
	map(0xc000, 0xdfff).ram();
}


void truxton2_state::batrider_sound_z80_port(address_map &map)
{
	map.global_mask(0xff);
	map(0x40, 0x40).w(m_soundlatch[2], FUNC(generic_latch_8_device::write));
	map(0x42, 0x42).w(m_soundlatch[3], FUNC(generic_latch_8_device::write));
	map(0x44, 0x44).w(FUNC(truxton2_state::batrider_sndirq_w));
	map(0x46, 0x46).w(FUNC(truxton2_state::batrider_clear_nmi_w));
	map(0x48, 0x48).r(m_soundlatch[0], FUNC(generic_latch_8_device::read));
	map(0x4a, 0x4a).r(m_soundlatch[1], FUNC(generic_latch_8_device::read));
	map(0x80, 0x81).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x82, 0x82).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x84, 0x84).rw(m_oki[1], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x88, 0x88).w(FUNC(truxton2_state::raizing_z80_bankswitch_w));
	map(0xc0, 0xc6).w(FUNC(truxton2_state::raizing_oki_bankswitch_w));
}


void truxton2_state::bbakraid_sound_z80_mem(address_map &map)
{
	map(0x0000, 0xbfff).rom();     // No banking? ROM only contains code and data up to 0x28DC
	map(0xc000, 0xffff).ram();
}


void truxton2_state::bbakraid_sound_z80_port(address_map &map)
{
	map.global_mask(0xff);
	map(0x40, 0x40).w(m_soundlatch[2], FUNC(generic_latch_8_device::write));
	map(0x42, 0x42).w(m_soundlatch[3], FUNC(generic_latch_8_device::write));
	map(0x44, 0x44).w(FUNC(truxton2_state::batrider_sndirq_w));
	map(0x46, 0x46).w(FUNC(truxton2_state::batrider_clear_nmi_w));
	map(0x48, 0x48).r(m_soundlatch[0], FUNC(generic_latch_8_device::read));
	map(0x4a, 0x4a).r(m_soundlatch[1], FUNC(generic_latch_8_device::read));
	map(0x80, 0x81).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write));
}


void toaplan2_state::dogyuunto_sound_z80_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xc7ff).ram().share(m_shared_ram);
	map(0xe000, 0xe001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xe004, 0xe004).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}


void toaplan2_state::v25_mem(address_map &map)
{
	map(0x00000, 0x00001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x00004, 0x00004).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x80000, 0x87fff).mirror(0x78000).ram().share(m_shared_ram);
}


void toaplan2_state::kbash_v25_mem(address_map &map)
{
	map(0x00000, 0x007ff).ram().share(m_shared_ram);
	map(0x04000, 0x04001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x04002, 0x04002).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x80000, 0x87fff).mirror(0x78000).rom().region("audiocpu", 0);
}


void truxton2_state::fixeight_v25_mem(address_map &map)
{
	map(0x00000, 0x00000).portr("IN1");
	map(0x00002, 0x00002).portr("IN2");
	map(0x00004, 0x00004).portr("IN3");
	map(0x0000a, 0x0000b).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x0000c, 0x0000c).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x80000, 0x87fff).mirror(0x78000).ram().share(m_shared_ram);
}


void toaplan2_state::vfive_v25_mem(address_map &map)
{
	map(0x00000, 0x00001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x80000, 0x87fff).mirror(0x78000).ram().share(m_shared_ram);
}


void toaplan2_state::fixeightbl_oki(address_map &map)
{
	map(0x00000, 0x2ffff).rom();
	map(0x30000, 0x3ffff).bankr(m_okibank);
}

void toaplan2_state::enmadaio_oki(address_map &map)
{
	map(0x00000, 0x3ffff).bankr(m_okibank);
}

// similar as NMK112, but GAL-driven; NOT actual NMK112 is present
template<unsigned Chip>
void truxton2_state::raizing_oki(address_map &map)
{
	map(0x00000, 0x000ff).bankr(m_raizing_okibank[Chip][0]);
	map(0x00100, 0x001ff).bankr(m_raizing_okibank[Chip][1]);
	map(0x00200, 0x002ff).bankr(m_raizing_okibank[Chip][2]);
	map(0x00300, 0x003ff).bankr(m_raizing_okibank[Chip][3]);
	map(0x00400, 0x0ffff).bankr(m_raizing_okibank[Chip][4]);
	map(0x10000, 0x1ffff).bankr(m_raizing_okibank[Chip][5]);
	map(0x20000, 0x2ffff).bankr(m_raizing_okibank[Chip][6]);
	map(0x30000, 0x3ffff).bankr(m_raizing_okibank[Chip][7]);
}


u8 toaplan2_state::tekipaki_cmdavailable_r()
{
	if (m_soundlatch[0]->pending_r()) return 0xff;
	else return 0x00;
};

void toaplan2_state::hd647180_io_map(address_map &map)
{
	map.global_mask(0xff);

	map(0x60, 0x60).nopr();
	map(0x70, 0x75).nopw(); // DDRs are written with the wrong upper addresses!
	map(0x84, 0x84).r(m_soundlatch[0], FUNC(generic_latch_8_device::read));

	map(0x82, 0x82).rw("ymsnd", FUNC(ym3812_device::status_r), FUNC(ym3812_device::address_w));
	map(0x83, 0x83).w("ymsnd", FUNC(ym3812_device::data_w));
}


void ghox_state::ghox_hd647180_mem_map(address_map &map)
{
	map(0x40000, 0x407ff).ram().share(m_shared_ram);

	map(0x80002, 0x80002).portr("DSWA");
	map(0x80004, 0x80004).portr("DSWB");
	map(0x80006, 0x80006).nopr(); // nothing?
	map(0x80008, 0x80008).portr("IN1");
	map(0x8000a, 0x8000a).portr("IN2");

	map(0x8000c, 0x8000e).portr("SYS");

	map(0x8000e, 0x8000f).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
}

/*****************************************************************************
    Input Port definitions
    The following commands are available when the Invulnerability dipswitch
    is set (or, in some games, also when the JAMMA Test switch is pressed):

    P2 start                 : pause
    P1 start                 : resume
    Hold P1 start & P2 start : slow motion

    In bgaregga, batrider and bbakraid, the commands are different:

    Tap P1 start             : pause/resume
    Hold P1 start            : slow motion

    Additional per-game test features are as follows:

    truxton2 - While playing in invulnerable mode, press button 3 to suicide.

    fixeight - While playing in invulnerable mode, press button 3 to suicide
               (player 1 and player 2 only)

    batsugun - While playing in invulnerable mode, press the following buttons
               to stage skip:

               P2 button 3 & P2 button 1 : Skip to end of stage 1
               P2 button 3 & P2 button 2 : Skip to end of stage 2
               P2 button 3               : Skip to end of stage 3

   sstriker - While playing in invulnerable mode as player 2, press
   /kingdmgp  P2 button 3 to skip to the end of the current stage.

   bgaregga - Press and hold P1 button 1, P1 button 2 and P1 button 3 while
              powering on in service mode to enter the special service mode.
              "OPTIONS" and "PLAY DATAS" are added to the menu, and the
              dipswitch display will show the region jumpers (normally hidden).
              Choose "GAME MODE" from the special service mode to enter the
              special game mode. In the special game mode, you can use pause
              and slow motion even when not playing in invulnerable mode.

   batrider - While playing in invulnerable mode, press P1 Start and P2 Start
              to skip directly to the ending scene.

   batrider - Press and hold P1 button 1, P1 button 2 and P1 button 3 while
   /bbakraid  powering on in service mode to enter the special service mode.
              You can change the game's region by pressing left/right.
              Choose "GAME MODE" from the special service mode to enter the
              special game mode. In the special game mode, you can use pause
              and slow motion even when not playing in invulnerable mode.
              While the game is paused in special mode, press button 3 to
              display debugging information.

*****************************************************************************/


static INPUT_PORTS_START( toaplan2_2b )
	PORT_START("IN1")
	TOAPLAN_JOY_UDLR_2_BUTTONS( 1 )

	PORT_START("IN2")
	TOAPLAN_JOY_UDLR_2_BUTTONS( 2 )

	PORT_START("SYS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_TILT )
	TOAPLAN_TEST_SWITCH( 0x04, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSWA")
	TOAPLAN_MACHINE_NO_COCKTAIL_LOC(SW1)
	// Coinage on bit mask 0x00f0
	PORT_BIT( 0x00f0, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Modified below

	PORT_START("DSWB")
	TOAPLAN_DIFFICULTY_LOC(SW2)
	// Per-game features on bit mask 0x00fc
	PORT_BIT( 0x00fc, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Modified below
INPUT_PORTS_END


static INPUT_PORTS_START( toaplan2_3b )
	PORT_INCLUDE( toaplan2_2b )

	PORT_MODIFY("IN1")
	TOAPLAN_JOY_UDLR_3_BUTTONS( 1 )

	PORT_MODIFY("IN2")
	TOAPLAN_JOY_UDLR_3_BUTTONS( 2 )
INPUT_PORTS_END


static INPUT_PORTS_START( tekipaki )
	PORT_INCLUDE( toaplan2_2b )

	PORT_MODIFY("DSWA")
	// Various features on bit mask 0x000f - see above
	TOAPLAN_COINAGE_DUAL_LOC( JMPR, 0x0f, 0x02, SW1 )

	PORT_MODIFY("DSWB")
	// Difficulty on bit mask 0x0003 - see above
	// "Stop Mode" corresponds to "Invulnerability" in the other games
	// (i.e. it enables pause and slow motion)
	PORT_DIPNAME( 0x0004,   0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:!3")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008,   0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:!4")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010,   0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:!5")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020,   0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:!6")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040,   0x0000, "Stop Mode (Cheat)" )   PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,   0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0080, DEF_STR( On ) )

	PORT_START("JMPR")
	PORT_CONFNAME( 0x000f,  0x0002, DEF_STR( Region ) ) PORT_DIPLOCATION("JP:!4,!3,!2,!1")
	PORT_CONFSETTING(       0x0002, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x0001, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x0000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x0003, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(       0x0004, DEF_STR( Korea ) )
	PORT_CONFSETTING(       0x0005, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(       0x0006, "Taiwan (Spacy Co., Ltd." )
	PORT_CONFSETTING(       0x0007, "USA (Romstar, Inc.)" )
	PORT_CONFSETTING(       0x0008, "Hong Kong (Honest Trading Co.)" )
//  PORT_CONFSETTING(        0x0009, DEF_STR( Japan ) )  // English title screen
//  PORT_CONFSETTING(        0x000a, DEF_STR( Japan ) )
//  PORT_CONFSETTING(        0x000b, DEF_STR( Japan ) )
//  PORT_CONFSETTING(        0x000c, DEF_STR( Japan ) )
//  PORT_CONFSETTING(        0x000d, DEF_STR( Japan ) )
//  PORT_CONFSETTING(        0x000e, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x000f, "Japan (Distributed by Tecmo)" )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(toaplan2_state, c2map_r)
INPUT_PORTS_END


static INPUT_PORTS_START( ghox )
	PORT_INCLUDE( toaplan2_2b )

	PORT_MODIFY("DSWA")
	// Various features on bit mask 0x000f - see above
	TOAPLAN_COINAGE_DUAL_LOC( JMPR, 0x80000, 0x80000, SW1 )

	PORT_MODIFY("DSWB")
	// Difficulty on bit mask 0x0003 - see above
	// "Debug Mode" corresponds to "Invulnerability" in the other games
	// (i.e. it enables pause and slow motion)
	PORT_DIPNAME( 0x000c,   0x0000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(        0x000c, DEF_STR( None ) )
	PORT_DIPSETTING(        0x0008, "100k only" )
	PORT_DIPSETTING(        0x0004, "100k and 300k" )
	PORT_DIPSETTING(        0x0000, "100k and every 200k" )
	PORT_DIPNAME( 0x0030,   0x0000, DEF_STR( Lives ) )      PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(        0x0030, "1" )
	PORT_DIPSETTING(        0x0020, "2" )
	PORT_DIPSETTING(        0x0000, "3" )
	PORT_DIPSETTING(        0x0010, "5" )
	PORT_DIPNAME( 0x0040,   0x0000, "Debug Mode (Cheat)" )  PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,   0x0000, DEF_STR( Unused ) )     PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0080, DEF_STR( On ) )

	PORT_START("JMPR")
	// Bit Mask 0x80000 is used here to signify European Coinage for MAME purposes - not read on the real board!
	PORT_CONFNAME( 0x8000f, 0x80002, DEF_STR( Region ) )    //PORT_CONFLOCATION("JP:!4,!3,!2,!1,FAKE:!1")
	PORT_CONFSETTING(       0x80002, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x00001, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x00000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x00003, "Hong Kong (Honest Trading Co.)" )
	PORT_CONFSETTING(       0x00004, DEF_STR( Korea ) )
	PORT_CONFSETTING(       0x00005, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(       0x80006, "Spain & Portugal (APM Electronics S.A.)" )
	PORT_CONFSETTING(       0x80007, "Italy (Star Electronica SRL)" )
	PORT_CONFSETTING(       0x80008, "UK (JP Leisure Limited)" )
	PORT_CONFSETTING(       0x00009, "USA (Romstar, Inc.)" )
	PORT_CONFSETTING(       0x8000a, "Europe (Nova Apparate GMBH & Co.)" )
	PORT_CONFSETTING(       0x0000b, "USA (Taito America Corporation)" )
	PORT_CONFSETTING(       0x0000c, "USA (Taito Corporation Japan)" )
	PORT_CONFSETTING(       0x8000d, "Europe (Taito Corporation Japan)" )
	PORT_CONFSETTING(        0x0000e, "Japan (Licensed to [blank])" )    // English title screen
	PORT_CONFSETTING(        0x0000f, "Japan (Taito Corporation)" )

	PORT_START("PAD1")      /* Paddle 1 (left-right)  read at $100000 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Unknown/Unused

	PORT_START("PAD2")      /* Paddle 2 (left-right)  read at $040000 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2)
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Unknown/Unused
INPUT_PORTS_END


static INPUT_PORTS_START( ghoxjo )
	PORT_INCLUDE( ghox )

	PORT_MODIFY("JMPR")
	// Bit Mask 0x80000 is used here to signify European Coinage for MAME purposes - not read on the real board!
	PORT_CONFNAME( 0x8000f, 0x80002, DEF_STR( Region ) )    //PORT_CONFLOCATION("JP:!4,!3,!2,!1,FAKE:!1")
	PORT_CONFSETTING(       0x80002, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x00001, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x00000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x00003, "Hong Kong (Honest Trading Co.)" )
	PORT_CONFSETTING(       0x00004, DEF_STR( Korea ) )
	PORT_CONFSETTING(       0x00005, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(       0x80006, "Spain & Portugal (APM Electronics S.A.)" )
	PORT_CONFSETTING(       0x80007, "Italy (Star Electronica SRL)" )
	PORT_CONFSETTING(       0x80008, "UK (JP Leisure Limited)" )
	PORT_CONFSETTING(       0x00009, "USA (Romstar, Inc.)" )
	PORT_CONFSETTING(       0x8000a, "Europe (Nova Apparate GMBH & Co.)" )
	PORT_CONFSETTING(       0x0000b, "Japan (Unused) [b]" )
	PORT_CONFSETTING(       0x0000c, "Japan (Unused) [c]" )
	PORT_CONFSETTING(       0x0000d, "Japan (Unused) [d]" )
	PORT_CONFSETTING(       0x0000e, "Japan (Unused) [e]" )
	PORT_CONFSETTING(       0x0000f, "Japan (Unused) [f]" )
INPUT_PORTS_END


static INPUT_PORTS_START( dogyuun )
	PORT_INCLUDE( toaplan2_3b )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x0001,   0x0000, DEF_STR( Free_Play) )       PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0001, DEF_STR( On ) )
	// Various features on bit mask 0x000e - see above
	TOAPLAN_COINAGE_DUAL_LOC( JMPR, 0x8000, 0x8000, SW1 )

	PORT_MODIFY("DSWB")
	// Difficulty on bit mask 0x0003 - see above
	PORT_DIPNAME( 0x000c,   0x0000, DEF_STR( Bonus_Life ) )     PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(        0x000c, DEF_STR( None ) )
	PORT_DIPSETTING(        0x0008, "400k only" )
	PORT_DIPSETTING(        0x0000, "200k only" )
	PORT_DIPSETTING(        0x0004, "200k, 400k and 600k" )
	PORT_DIPNAME( 0x0030,   0x0000, DEF_STR( Lives ) )          PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(        0x0030, "1" )
	PORT_DIPSETTING(        0x0020, "2" )
	PORT_DIPSETTING(        0x0000, "3" )
	PORT_DIPSETTING(        0x0010, "5" )
	PORT_DIPNAME( 0x0040,   0x0000, "Invulnerability (Cheat)" )         PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,   0x0000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(        0x0080, DEF_STR( No ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( Yes ) )

	PORT_START("JMPR")
	// Bit Mask 0x8000 is used here to signify European Coinage for MAME purposes - not read on the real board!
	// "No speedups": all speedup items in game are replaced with bombs
	PORT_CONFNAME( 0x80f0,  0x8030, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!4,!3,!2,!1,FAKE:!1")
	PORT_CONFSETTING(       0x8030, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x0010, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x0020, "USA (Atari Games Corp.)" )
	PORT_CONFSETTING(       0x0000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x0040, "Hong Kong (Charterfield); no speedups" )
	PORT_CONFSETTING(       0x0050, "Korea (Unite Trading); no speedups" )
	PORT_CONFSETTING(       0x0060, "Taiwan; no speedups" )
	PORT_CONFSETTING(       0x0070, "USA; no speedups" )
	PORT_CONFSETTING(       0x0080, "Southeast Asia (Charterfield); no speedups" )
	PORT_CONFSETTING(       0x0090, "Hong Kong (Charterfield)" )
	PORT_CONFSETTING(       0x00a0, "Korea (Unite Trading)" )
	PORT_CONFSETTING(       0x00b0, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(       0x00c0, "USA (Atari Games Corp.); no speedups" )
	PORT_CONFSETTING(       0x00d0, "Southeast Asia (Charterfield)" )
	PORT_CONFSETTING(       0x80e0, "Europe; no speedups" )
	PORT_CONFSETTING(       0x00f0, "Japan (Taito Corp.)" )
INPUT_PORTS_END


static INPUT_PORTS_START( dogyuuna )
	PORT_INCLUDE( dogyuun )

	PORT_MODIFY("DSWA")
	TOAPLAN_COINAGE_DUAL_LOC( JMPR, 0xf0, 0x30, SW1 )

	PORT_MODIFY("JMPR")
	// "No speedups": all speedup items in game are replaced with bombs
	PORT_CONFNAME( 0x00f0,  0x0030, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!4,!3,!2,!1")
	PORT_CONFSETTING(       0x0030, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x0010, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x0020, "USA (Atari Games Corp.)" )
	PORT_CONFSETTING(       0x0000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x0040, "Hong Kong (Charterfield); no speedups" )
	PORT_CONFSETTING(       0x0050, "Korea (Unite Trading); no speedups" )
	PORT_CONFSETTING(       0x0060, "Taiwan; no speedups" )
//  PORT_CONFSETTING(        0x0070, "Taiwan (Licensed to ???????); no speedups" )
	PORT_CONFSETTING(       0x0080, "Southeast Asia (Charterfield); no speedups" )
	PORT_CONFSETTING(       0x0090, "Hong Kong (Charterfield)" )
	PORT_CONFSETTING(       0x00a0, "Korea (Unite Trading)" )
	PORT_CONFSETTING(       0x00b0, DEF_STR( Taiwan ) )
//  PORT_CONFSETTING(        0x00c0, "Taiwan (Licensed to ???????)" )
	PORT_CONFSETTING(       0x00d0, "Southeast Asia (Charterfield)" )
//  PORT_CONFSETTING(        0x00e0, DEF_STR( Unused ) )
	PORT_CONFSETTING(       0x00f0, "Japan (Taito Corp.)" )
INPUT_PORTS_END


static INPUT_PORTS_START( dogyuunt )
	PORT_INCLUDE( dogyuun )

	PORT_MODIFY("DSWA")
	TOAPLAN_COINAGE_DUAL_LOC( JMPR, 0xf0, 0x20, SW1 )

	PORT_MODIFY("JMPR")
	PORT_CONFNAME( 0x00f0,  0x0020, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!4,!3,!2,!1")
	PORT_CONFSETTING(       0x0020, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x0010, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x0000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x0030, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(       0x0040, DEF_STR( Korea ) )
	PORT_CONFSETTING(       0x0050, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(       0x0060, "Southeast Asia (Charterfield)" )
	PORT_CONFSETTING(       0x0070, "USA (Romstar, Inc.)" )
	PORT_CONFSETTING(       0x0080, "Hong Kong (Honest Trading Co.)" )
	PORT_CONFSETTING(       0x0090, "Korea (JC Trading Corp.)" )
	PORT_CONFSETTING(       0x00a0, "USA (Fabtek)" )
//  PORT_CONFSETTING(        0x00b0, DEF_STR( Unused ) )
//  PORT_CONFSETTING(        0x00c0, DEF_STR( Unused ) )
//  PORT_CONFSETTING(        0x00d0, DEF_STR( Unused ) )
//  PORT_CONFSETTING(        0x00e0, DEF_STR( Unused ) )
	PORT_CONFSETTING(       0x00f0, "Japan (Taito Corp.)" )
INPUT_PORTS_END


static INPUT_PORTS_START( kbash )
	PORT_INCLUDE( toaplan2_3b )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x0001,   0x0000, DEF_STR( Continue_Price ) ) PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(        0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(        0x0001, "Discount" )
	// Various features on bit mask 0x000e - see above
	TOAPLAN_COINAGE_DUAL_LOC( JMPR, 0x70, 0x20, SW1 )

	PORT_MODIFY("DSWB")
	// Difficulty on bit mask 0x0003 - see above
	PORT_DIPNAME( 0x000c,   0x0000, DEF_STR( Bonus_Life ) )     PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(        0x000c, DEF_STR( None ) )
	PORT_DIPSETTING(        0x0008, "200k only" )
	PORT_DIPSETTING(        0x0004, "100k only" )
	PORT_DIPSETTING(        0x0000, "100k and 400k" )
	PORT_DIPNAME( 0x0030,   0x0000, DEF_STR( Lives ) )          PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(        0x0030, "1" )
	PORT_DIPSETTING(        0x0000, "2" )
	PORT_DIPSETTING(        0x0020, "3" )
	PORT_DIPSETTING(        0x0010, "4" )
	PORT_DIPNAME( 0x0040,   0x0000, "Invulnerability (Cheat)" )         PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,   0x0000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(        0x0080, DEF_STR( No ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( Yes ) )

	PORT_START("JMPR")
	PORT_CONFNAME( 0x00f0,  0x0020, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!4,!3,!2,!1")
	PORT_CONFSETTING(       0x0020, "Europe, USA (Atari Games)" )   // European coinage
	PORT_CONFSETTING(       0x0010, "USA, Europe (Atari Games)" )
	PORT_CONFSETTING(       0x0000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x0030, DEF_STR( Korea ) )
	PORT_CONFSETTING(       0x0040, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(       0x0050, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(       0x0060, DEF_STR( Southeast_Asia ) ) // Service Mode wrongly shows European coinage
//  PORT_CONFSETTING(        0x0070, DEF_STR( Unused ) )
//  PORT_CONFSETTING(        0x0080, DEF_STR( Unused ) )
	PORT_CONFSETTING(       0x0090, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x00a0, DEF_STR( Europe ) ) // European coinage
//  PORT_CONFSETTING(        0x00b0, DEF_STR( Unused ) )
//  PORT_CONFSETTING(        0x00c0, DEF_STR( Unused ) )
//  PORT_CONFSETTING(        0x00d0, DEF_STR( Unused ) )
//  PORT_CONFSETTING(        0x00e0, DEF_STR( Unused ) ) // Service Mode wrongly shows European coinage
//  PORT_CONFSETTING(        0x00f0, DEF_STR( Unused ) )
INPUT_PORTS_END

static INPUT_PORTS_START( kbashk )
	PORT_INCLUDE( kbash )

	PORT_MODIFY("JMPR")
	PORT_CONFNAME( 0x00f0,  0x0000, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!4,!3,!2,!1")
	PORT_CONFSETTING(       0x0000, "Japan (Taito license)" ) // Taito license
	PORT_CONFSETTING(       0x0010, DEF_STR( Unused ) )
	PORT_CONFSETTING(       0x0020, DEF_STR( Unused ) )
	PORT_CONFSETTING(       0x0030, DEF_STR( Korea ) )
	PORT_CONFSETTING(       0x0040, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(       0x0050, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(       0x0060, DEF_STR( Southeast_Asia ) )
	PORT_CONFSETTING(       0x0070, DEF_STR( Unused ) )
	PORT_CONFSETTING(       0x0080, DEF_STR( Japan ) ) // no Taito license
	PORT_CONFSETTING(       0x0090, DEF_STR( Unused ) )
	PORT_CONFSETTING(       0x00a0, DEF_STR( Unused ) )
	PORT_CONFSETTING(       0x00b0, DEF_STR( Korea ) )
	PORT_CONFSETTING(       0x00c0, DEF_STR( Hong_Kong ))
	PORT_CONFSETTING(       0x00d0, DEF_STR( Taiwan ))
	PORT_CONFSETTING(       0x00e0, DEF_STR( Southeast_Asia ))
	PORT_CONFSETTING(       0x00f0, DEF_STR( Unused ) )
INPUT_PORTS_END

static INPUT_PORTS_START( kbash2 )
	PORT_INCLUDE( kbash )

	PORT_MODIFY("DSWA")
	TOAPLAN_COINAGE_DUAL_LOC( JMPR, 0x07, 0x02, SW1 )

	PORT_MODIFY("JMPR")
	PORT_CONFNAME( 0x000f,  0x0006, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!4,!3,!2,!1")
	PORT_CONFSETTING(       0x0000, "Japan (Taito Corp.)" )
//  PORT_CONFSETTING(        0x0001, DEF_STR( Unused ) )
//  PORT_CONFSETTING(        0x0002, DEF_STR( Unused ) ) // European coinage
	PORT_CONFSETTING(       0x0003, "Korea (Unite Trading)" )
	PORT_CONFSETTING(       0x0004, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(       0x0005, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(       0x0006, "Southeast Asia (Charterfield)" )   // Service Mode wrongly shows European coinage
//  PORT_CONFSETTING(        0x0007, DEF_STR( Unused ) )
	PORT_CONFSETTING(       0x0008, DEF_STR( Japan ) )
//  PORT_CONFSETTING(        0x0009, DEF_STR( Unused ) )
//  PORT_CONFSETTING(        0x000a, DEF_STR( Unused ) ) // European coinage
	PORT_CONFSETTING(       0x000b, DEF_STR( Korea ) )
//  PORT_CONFSETTING(        0x000c, DEF_STR( Hong_Kong ) )
//  PORT_CONFSETTING(        0x000d, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(       0x000e, DEF_STR( Southeast_Asia ) ) // Service Mode wrongly shows European coinage
//  PORT_CONFSETTING(        0x000f, DEF_STR( Unused ) )
	PORT_BIT( 0x00f0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( truxton2 )
	PORT_INCLUDE( toaplan2_3b )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Fast Scrolling (Cheat)")

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x0001,   0x0000, "Rapid Fire" )              PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(        0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( On ) )
	// Various features on bit mask 0x000e - see above
	TOAPLAN_COINAGE_DUAL_LOC( JMPR, 0x0f, 0x02, SW1 )

	PORT_MODIFY("DSWB")
	// Difficulty on bit mask 0x0003 - see above
	PORT_DIPNAME( 0x000c,   0x0000, DEF_STR( Bonus_Life ) )     PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(        0x000c, "200k only" )
	PORT_DIPSETTING(        0x0008, "100k only" )
	PORT_DIPSETTING(        0x0004, "100k and 250k" )
	PORT_DIPSETTING(        0x0000, "70k and 200k" )
	PORT_DIPNAME( 0x0030,   0x0000, DEF_STR( Lives ) )          PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(        0x0030, "2" )
	PORT_DIPSETTING(        0x0000, "3" )
	PORT_DIPSETTING(        0x0020, "4" )
	PORT_DIPSETTING(        0x0010, "5" )
	PORT_DIPNAME( 0x0040,   0x0000, "Invulnerability (Cheat)" ) PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,   0x0000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(        0x0080, DEF_STR( No ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( Yes ) )

	PORT_START("JMPR")
	PORT_CONFNAME( 0x000f,  0x0002, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!4,!3,!2,!1")
	PORT_CONFSETTING(       0x0002, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x0001, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x0000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x0003, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(       0x0004, DEF_STR( Korea ) )
	PORT_CONFSETTING(       0x0005, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(       0x0006, "Southeast Asia (Charterfield)" )
	PORT_CONFSETTING(       0x0007, "USA (Romstar, Inc.)" )
	PORT_CONFSETTING(       0x0008, "Hong Kong (Honest Trading Co.)" )
	PORT_CONFSETTING(       0x0009, "Korea (JC Trading Corp.)" )
	PORT_CONFSETTING(       0x000a, "USA (Fabtek)" )
//  PORT_CONFSETTING(        0x000b, DEF_STR( Unused ) )
//  PORT_CONFSETTING(        0x000c, DEF_STR( Unused ) )
//  PORT_CONFSETTING(        0x000d, DEF_STR( Unused ) )
//  PORT_CONFSETTING(        0x000e, DEF_STR( Unused ) )
	PORT_CONFSETTING(       0x000f, "Japan (Taito Corp.)" )
INPUT_PORTS_END


static INPUT_PORTS_START( pipibibs )
	PORT_INCLUDE( toaplan2_2b )

	PORT_MODIFY("DSWA")
	// Various features on bit mask 0x000f - see above
	TOAPLAN_COINAGE_DUAL_LOC( JMPR, 0x06, 0x06, SW1 )

	PORT_MODIFY("DSWB")
	// Difficulty on bit mask 0x0003 - see above
	PORT_DIPNAME( 0x000c,   0x0000, DEF_STR( Bonus_Life ) )     PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(        0x000c, DEF_STR( None ) )
	PORT_DIPSETTING(        0x0008, "200k only" )
	PORT_DIPSETTING(        0x0000, "200k and every 300k" )
	PORT_DIPSETTING(        0x0004, "150k and every 200k" )
	PORT_DIPNAME( 0x0030,   0x0000, DEF_STR( Lives ) )          PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(        0x0030, "1" )
	PORT_DIPSETTING(        0x0020, "2" )
	PORT_DIPSETTING(        0x0000, "3" )
	PORT_DIPSETTING(        0x0010, "5" )
	PORT_DIPNAME( 0x0040,   0x0000, "Invulnerability (Cheat)" ) PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,   0x0000, DEF_STR( Unused ) )         PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0080, DEF_STR( On ) )

	PORT_START("JMPR")
	PORT_CONFNAME( 0x0008,  0x0000, "Nudity" )          //PORT_CONFLOCATION("JP:!1")
	PORT_CONFSETTING(       0x0008, DEF_STR( Low ) )
	PORT_CONFSETTING(       0x0000, "High, but censored" )
	PORT_CONFNAME( 0x0007,  0x0006, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!4,!3,!2")
	PORT_CONFSETTING(       0x0006, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x0004, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x0000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x0001, DEF_STR( Asia ) )
	PORT_CONFSETTING(       0x0002, "Hong Kong (Honest Trading Co.)" )
	PORT_CONFSETTING(       0x0003, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(       0x0005, "USA (Romstar, Inc.)" )
	PORT_CONFSETTING(       0x0007, "Europe (Nova Apparate GMBH & Co.)" )
INPUT_PORTS_END


static INPUT_PORTS_START( pipibibsp )
	PORT_INCLUDE( pipibibs )

	PORT_MODIFY("DSWA")
	// Various features on bit mask 0x000d - see above
	TOAPLAN_COINAGE_DUAL_LOC( JMPR, 0x80000, 0x80000, SW1 )

	PORT_MODIFY("JMPR")
	// Bit Mask 0x80000 is used here to signify European Coinage for MAME purposes - not read on the real board!
	PORT_CONFNAME( 0x80007, 0x00002, DEF_STR( Region ) )    //PORT_CONFLOCATION("JP:!4,!3,!2,FAKE:!1")
	PORT_CONFSETTING(       0x00002, DEF_STR( World ) )
	PORT_CONFSETTING(       0x80005, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x00004, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x00000, "Japan (Ryouta Kikaku)" )
	PORT_CONFSETTING(       0x00001, "Hong Kong (Honest Trading Co.)" )
	PORT_CONFSETTING(       0x80006, "Spain & Portugal (APM Electronics S.A.)" )
	PORT_CONFSETTING(       0x00007, "World (Ryouta Kikaku)" )
INPUT_PORTS_END


static INPUT_PORTS_START( whoopee )
	PORT_INCLUDE( pipibibs )

	PORT_MODIFY("JMPR")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(toaplan2_state, c2map_r)   // bit 0x10 sound ready
INPUT_PORTS_END


static INPUT_PORTS_START( pipibibsbl )
	PORT_INCLUDE( pipibibs )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x0002,   0x0000, DEF_STR( Unused ) )     PORT_DIPLOCATION("SW1:!2")  // In Test Mode, it shows as Normal/Invert Screen - HW doesn't support it
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0002, DEF_STR( On ) )
	// Various features on bit mask 0x000d - see above
	TOAPLAN_COINAGE_DUAL_LOC( JMPR, 0x80000, 0x80000, SW1 )

	PORT_MODIFY("JMPR")
	// Bit Mask 0x80000 is used here to signify European Coinage for MAME purposes - not read on the real board!
	PORT_CONFNAME( 0x80007, 0x00002, DEF_STR( Region ) )    //PORT_CONFLOCATION("JP:!4,!3,!2,FAKE:!1")
	PORT_CONFSETTING(       0x00002, DEF_STR( World ) )
	PORT_CONFSETTING(       0x80005, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x00004, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x00000, "Japan (Ryouta Kikaku)" )
	PORT_CONFSETTING(       0x00001, "Hong Kong (Honest Trading Co.)" )
	PORT_CONFSETTING(       0x80006, "Spain & Portugal (APM Electronics S.A.)" )
	PORT_CONFSETTING(       0x00007, "World (Ryouta Kikaku)" )
INPUT_PORTS_END


static INPUT_PORTS_START( fixeight )
	// The Suicide buttons are technically P1 and P2 Button 3, but we hook
	// them up as IPT_OTHER so each player has the same number of buttons.
	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 Suicide (Cheat)")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Unknown/Unused

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Suicide (Cheat)")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Unknown/Unused

	PORT_START("IN3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START3 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_MEMORY_RESET ) PORT_NAME("Region Reset")
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Unknown/Unused

	PORT_START("SYS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_SERVICE_NO_TOGGLE(0x0004, IP_ACTIVE_HIGH)  // Service input is a pushbutton marked 'Test SW'
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Unknown/Unused

	PORT_START("EEPROM")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
INPUT_PORTS_END


static INPUT_PORTS_START( fixeightbl )
	PORT_INCLUDE( toaplan2_2b )

	PORT_MODIFY("SYS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_SERVICE_NO_TOGGLE(0x0004, IP_ACTIVE_HIGH)  // Service input is a pushbutton marked 'Test SW'

	PORT_START("IN3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START3 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Unknown/Unused

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x0001,   0x0000, "Maximum Players" )     PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(        0x0000, "2" )
	PORT_DIPSETTING(        0x0001, "3" )
	PORT_DIPNAME( 0x0002,   0x0000, DEF_STR( Unused ) )     PORT_DIPLOCATION("SW1:!2")  // This video HW doesn't support flip screen
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004,   0x0004, "Shooting Style" )      PORT_DIPLOCATION("SW1:!3")
	PORT_DIPSETTING(        0x0004, "Semi-Auto" )
	PORT_DIPSETTING(        0x0000, "Full-Auto" )
	// Various features on bit mask 0x0008 - see above
	TOAPLAN_COINAGE_JAPAN_LOC(SW1)

	PORT_MODIFY("DSWB")
	// Difficulty on bit mask 0x0003 - see above
	PORT_DIPNAME( 0x000c,   0x0000, DEF_STR( Bonus_Life ) )     PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(        0x000c, DEF_STR( None ) )
	PORT_DIPSETTING(        0x0000, "500k and every 500k" )
	PORT_DIPSETTING(        0x0008, "300k only" )
	PORT_DIPSETTING(        0x0004, "300k and every 300k" )
	PORT_DIPNAME( 0x0030,   0x0000, DEF_STR( Lives ) )          PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(        0x0030, "1" )
	PORT_DIPSETTING(        0x0020, "2" )
	PORT_DIPSETTING(        0x0000, "3" )
	PORT_DIPSETTING(        0x0010, "5" )
	PORT_DIPNAME( 0x0040,   0x0000, "Invulnerability (Cheat)" ) PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,   0x0000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(        0x0080, DEF_STR( No ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( Yes ) )
INPUT_PORTS_END


static INPUT_PORTS_START( grindstm )
	PORT_INCLUDE( toaplan2_2b )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x0001,   0x0000, DEF_STR( Cabinet ) )        PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(        0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(        0x0001, DEF_STR( Cocktail ) )
	// Various features on bit mask 0x000e - see above
	TOAPLAN_COINAGE_DUAL_LOC( JMPR, 0xe0, 0x80, SW1 )

	PORT_MODIFY("DSWB")
	// Difficulty on bit mask 0x0003 - see above
	PORT_DIPNAME( 0x000c,   0x0000, DEF_STR( Bonus_Life ) )     PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(        0x000c, DEF_STR( None ) )
	PORT_DIPSETTING(        0x0008, "200k only" )
	PORT_DIPSETTING(        0x0000, "300k and 800k" )
	PORT_DIPSETTING(        0x0004, "300k and every 800k" )
	PORT_DIPNAME( 0x0030,   0x0000, DEF_STR( Lives ) )          PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(        0x0030, "1" )
	PORT_DIPSETTING(        0x0020, "2" )
	PORT_DIPSETTING(        0x0000, "3" )
	PORT_DIPSETTING(        0x0010, "5" )
	PORT_DIPNAME( 0x0040,   0x0000, "Invulnerability (Cheat)" )         PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,   0x0000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(        0x0080, DEF_STR( No ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( Yes ) )

	PORT_START("JMPR")
	// Code in many places in game tests if region is >= 0xC. Effects on gameplay?
	PORT_CONFNAME( 0x00f0,  0x0090, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!4,!3,!2,!1")
	PORT_CONFSETTING(       0x0090, DEF_STR( Europe ) )
//  PORT_CONFSETTING(        0x0080, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x00b0, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x00a0, "USA (American Sammy Corporation)" )
	PORT_CONFSETTING(       0x0070, DEF_STR( Southeast_Asia ) )
	PORT_CONFSETTING(       0x0060, "Southeast Asia (Charterfield)" )
	PORT_CONFSETTING(       0x0050, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(       0x0040, "Taiwan (Anomoto International Inc.)" )
	PORT_CONFSETTING(       0x0030, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(       0x0020, "Hong Kong (Charterfield)" )
	PORT_CONFSETTING(       0x0010, DEF_STR( Korea ) )
	PORT_CONFSETTING(       0x0000, "Korea (Unite Trading)" )
	PORT_CONFSETTING(       0x00d0, "USA; different?" )
	PORT_CONFSETTING(       0x00c0, "USA (American Sammy Corporation); different?" )
	PORT_CONFSETTING(       0x00e0, "Korea; different?" )
//  PORT_CONFSETTING(        0x00f0, "Korea; different?" )
INPUT_PORTS_END


static INPUT_PORTS_START( grindstma )
	PORT_INCLUDE( grindstm )

	PORT_MODIFY("JMPR")
	// Code in many places in game tests if region is >= 0xC. Effects on gameplay?
	PORT_CONFNAME( 0x00f0,  0x0090, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!4,!3,!2,!1")
	PORT_CONFSETTING(       0x0090, DEF_STR( Europe ) )
//  PORT_CONFSETTING(        0x0080, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x00b0, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x00a0, "USA (Atari Games Corp.)" )
	PORT_CONFSETTING(       0x0070, DEF_STR( Southeast_Asia ) )
	PORT_CONFSETTING(       0x0060, "Southeast Asia (Charterfield)" )
	PORT_CONFSETTING(       0x0050, DEF_STR( Taiwan ) )
//  PORT_CONFSETTING(        0x0040, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(       0x0030, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(       0x0020, "Hong Kong (Charterfield)" )
	PORT_CONFSETTING(       0x0010, DEF_STR( Korea ) )
	PORT_CONFSETTING(       0x0000, "Korea (Unite Trading)" )
	PORT_CONFSETTING(       0x00c0, "Korea; different?" )
//  PORT_CONFSETTING(        0x00d0, "Korea; different?" )
//  PORT_CONFSETTING(        0x00e0, "Korea; different?" )
//  PORT_CONFSETTING(        0x00f0, "Korea; different?" )
INPUT_PORTS_END


static INPUT_PORTS_START( vfive )
	PORT_INCLUDE( grindstm )

	PORT_MODIFY("DSWA")
	TOAPLAN_COINAGE_JAPAN_LOC(SW1)

	PORT_MODIFY("JMPR")
	// Region is forced to Japan in this set.
	// Code at $9238 tests bit 7.
	// (Actually bit 3, but the V25 shifts the jumper byte before storing it in shared RAM)
	// Runs twice near end of stage 1, once when each of the two boss tanks appears. Effect?
	// Also, if bit 7 is set and bits 6-5 are clear, service mode wrongly shows European coinage
	// (due to code left in from Grind Stormer: see code at $210A4 and lookup table at $211FA)
	PORT_CONFNAME( 0x0030,  0x0000, "Copyright" )           //PORT_CONFLOCATION("JP:!4,!3")
	PORT_CONFSETTING(       0x0000, "All Rights Reserved" )
//  PORT_CONFSETTING(        0x0010, "All Rights Reserved" )
//  PORT_CONFSETTING(        0x0020, "All Rights Reserved" )
	PORT_CONFSETTING(       0x0030, "Licensed to Taito Corp." )
	PORT_CONFNAME( 0x0040,  0x0000, DEF_STR( Unused ) )     //PORT_CONFLOCATION("JP:!2")
	PORT_CONFSETTING(       0x0000, DEF_STR( Off ) )
	PORT_CONFSETTING(       0x0040, DEF_STR( On ) )
	PORT_CONFNAME( 0x0080,  0x0000, DEF_STR( Unknown ) )    //PORT_CONFLOCATION("JP:!1")
	PORT_CONFSETTING(       0x0000, DEF_STR( Off ) )
	PORT_CONFSETTING(       0x0080, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( batsugun )
	PORT_INCLUDE( toaplan2_3b )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x0001,   0x0000, DEF_STR( Continue_Price ) ) PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(        0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(        0x0001, "Discount" )
	// Various features on bit mask 0x000e - see above
	TOAPLAN_COINAGE_JAPAN_LOC(SW1)  // European coinage shown in Service Mode but not actually used

	PORT_MODIFY("DSWB")
	// Difficulty on bit mask 0x0003 - see above
	PORT_DIPNAME( 0x000c,   0x0000, DEF_STR( Bonus_Life ) )     PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(        0x000c, DEF_STR( None ) )
	PORT_DIPSETTING(        0x0008, "1500k only" )
	PORT_DIPSETTING(        0x0000, "1000k only" )
	PORT_DIPSETTING(        0x0004, "500k and every 600k" )
	PORT_DIPNAME( 0x0030,   0x0000, DEF_STR( Lives ) )          PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(        0x0030, "1" )
	PORT_DIPSETTING(        0x0020, "2" )
	PORT_DIPSETTING(        0x0000, "3" )
	PORT_DIPSETTING(        0x0010, "5" )
	PORT_DIPNAME( 0x0040,   0x0000, "Invulnerability (Cheat)" )         PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,   0x0000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(        0x0080, DEF_STR( No ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( Yes ) )

	PORT_START("JMPR")
	PORT_CONFNAME( 0x00f0,  0x0090, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!4,!3,!2,!1")
	PORT_CONFSETTING(       0x0090, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x0080, "Europe (Taito Corp.)" )
	PORT_CONFSETTING(       0x00b0, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x00a0, "USA (Taito Corp.)" )
	PORT_CONFSETTING(       0x00f0, DEF_STR( Japan ) )
//  PORT_CONFSETTING(        0x00e0, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x00d0, "Japan (Taito Corp.)" )
//  PORT_CONFSETTING(        0x00c0, "Japan (Taito Corp.)" )
	PORT_CONFSETTING(       0x0070, DEF_STR( Southeast_Asia ) )
	PORT_CONFSETTING(       0x0060, "Southeast Asia (Taito Corp.)" )
	PORT_CONFSETTING(       0x0050, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(       0x0040, "Taiwan (Taito Corp.)" )
	PORT_CONFSETTING(       0x0030, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(       0x0020, "Hong Kong (Taito Corp.)" )
	PORT_CONFSETTING(       0x0010, DEF_STR( Korea ) )
	PORT_CONFSETTING(       0x0000, "Korea (Unite Trading)" )
INPUT_PORTS_END


static INPUT_PORTS_START( batsugunbl )
	PORT_INCLUDE( batsugun )

	PORT_MODIFY("JMPR")
	PORT_CONFNAME( 0x000f,  0x0009, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!4,!3,!2,!1")
	PORT_CONFSETTING(       0x0009, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x0008, "Europe (Taito Corp.)" )
	PORT_CONFSETTING(       0x000b, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x000a, "USA (Taito Corp.)" )
	PORT_CONFSETTING(       0x000f, DEF_STR( Japan ) )
//  PORT_CONFSETTING(       0x000e, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x000d, "Japan (Taito Corp.)" )
//  PORT_CONFSETTING(       0x000c, "Japan (Taito Corp.)" )
	PORT_CONFSETTING(       0x0007, DEF_STR( Southeast_Asia ) )
	PORT_CONFSETTING(       0x0006, "Southeast Asia (Taito Corp.)" )
	PORT_CONFSETTING(       0x0005, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(       0x0004, "Taiwan (Taito Corp.)" )
	PORT_CONFSETTING(       0x0003, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(       0x0002, "Hong Kong (Taito Corp.)" )
	PORT_CONFSETTING(       0x0001, DEF_STR( Korea ) )
	PORT_CONFSETTING(       0x0000, "Korea (Unite Trading)" )
	PORT_CONFNAME( 0x00f0,  0x00f0, "(null)" )
INPUT_PORTS_END


static INPUT_PORTS_START( pwrkick )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x5c, 0x00, "Payout" ) PORT_DIPLOCATION("SW1:!3,!4,!5,!7")
	PORT_DIPSETTING(    0x00, "110" ) // Service mode displays values as 1-8, ignoring SW1:7
	PORT_DIPSETTING(    0x04, "105" )
	PORT_DIPSETTING(    0x08, "100" )
	PORT_DIPSETTING(    0x0c, "95" )
	PORT_DIPSETTING(    0x10, "90" )
	PORT_DIPSETTING(    0x14, "85" )
	PORT_DIPSETTING(    0x18, "80" )
	PORT_DIPSETTING(    0x1c, "75" )
	PORT_DIPSETTING(    0x40, "70" )
	PORT_DIPSETTING(    0x44, "65" )
	PORT_DIPSETTING(    0x48, "60" )
	PORT_DIPSETTING(    0x4c, "55" )
	PORT_DIPSETTING(    0x50, "50" )
	PORT_DIPSETTING(    0x54, "45" )
	PORT_DIPSETTING(    0x58, "40" )
	PORT_DIPSETTING(    0x5c, "35" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Diagnostic" ) PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x00, "Play Credit" ) PORT_DIPLOCATION("SW2:!1,!2")
	PORT_DIPSETTING(    0x00, u8"¥10" )
	PORT_DIPSETTING(    0x01, u8"¥20" )
	PORT_DIPSETTING(    0x02, u8"¥30" )
	PORT_DIPSETTING(    0x03, u8"¥40" )
	PORT_DIPNAME( 0x0c, 0x00, "Coin Exchange" ) PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(    0x00, "12" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x08, "6" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x30, 0x00, "Game Mode" ) PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, "Shot" )
	PORT_DIPSETTING(    0x20, "Auto" )
	PORT_DIPSETTING(    0x30, "S-Manual" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW2:!7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "SW2:!8" )

	PORT_START("DSWC")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x00, "SW3:!1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x00, "SW3:!2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x00, "SW3:!3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x00, "SW3:!4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x00, "SW3:!5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "SW3:!6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW3:!7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "SW3:!8" )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SLOT_STOP1 ) PORT_NAME("Left")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SLOT_STOP2 ) PORT_NAME("Center")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SLOT_STOP3 ) PORT_NAME("Right")
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME(u8"Coin 2 (¥10)")
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_HIGH )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SLOT_STOP4 ) PORT_NAME("Down") // does this button really exist?
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MEMORY_RESET )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SYS")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_NAME(u8"Coin Exchange (¥100)")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x30, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_SERVICE ) PORT_NAME("Attendant Key")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("Coin 1 (Medal)")

	// The specific "Payout" button shown on the test screen and diagnostic menu does not exist.
INPUT_PORTS_END

static INPUT_PORTS_START( burgkids )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x5c, 0x00, "Payout" ) PORT_DIPLOCATION("SW1:!3,!4,!5,!7")
	PORT_DIPSETTING(    0x00, "110" ) // Service mode displays values as 1-8, ignoring SW1:7
	PORT_DIPSETTING(    0x04, "105" )
	PORT_DIPSETTING(    0x08, "100" )
	PORT_DIPSETTING(    0x0c, "95" )
	PORT_DIPSETTING(    0x10, "90" )
	PORT_DIPSETTING(    0x14, "85" )
	PORT_DIPSETTING(    0x18, "80" )
	PORT_DIPSETTING(    0x1c, "75" )
	PORT_DIPSETTING(    0x40, "70" )
	PORT_DIPSETTING(    0x44, "65" )
	PORT_DIPSETTING(    0x48, "60" )
	PORT_DIPSETTING(    0x4c, "55" )
	PORT_DIPSETTING(    0x50, "50" )
	PORT_DIPSETTING(    0x54, "45" )
	PORT_DIPSETTING(    0x58, "40" )
	PORT_DIPSETTING(    0x5c, "35" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "SW2:!8" )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x00, "Play Credit" ) PORT_DIPLOCATION("SW2:!1,!2")
	PORT_DIPSETTING(    0x00, u8"¥10" )
	PORT_DIPSETTING(    0x01, u8"¥20" )
	PORT_DIPSETTING(    0x02, u8"¥30" )
	PORT_DIPSETTING(    0x03, u8"¥40" )
	PORT_DIPNAME( 0x1c, 0x00, "Coin Exchange" ) PORT_DIPLOCATION("SW2:!3,!4,!5")
	PORT_DIPSETTING(    0x00, "12" )
	PORT_DIPSETTING(    0x04, "11" )
	PORT_DIPSETTING(    0x08, "10" )
	PORT_DIPSETTING(    0x0c, "6" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x14, "4" )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x1c, DEF_STR ( Off ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "SW2:!6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW2:!7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "SW2:!8" )

	PORT_START("DSWC")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x00, "SW3:!1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x00, "SW3:!2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x00, "SW3:!3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x00, "SW3:!4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x00, "SW3:!5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "SW3:!6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW3:!7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "SW3:!8" )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SLOT_STOP1 ) PORT_NAME("1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SLOT_STOP2 ) PORT_NAME("2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SLOT_STOP3 ) PORT_NAME("3")
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME(u8"Coin 2 (¥10)")
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_HIGH )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SLOT_STOP4 ) PORT_NAME("Down")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_MEMORY_RESET )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SYS")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_NAME(u8"Coin Exchange (¥100)")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x30, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_SERVICE ) PORT_NAME("Attendant Key")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("Coin 1 (Medal)")
INPUT_PORTS_END

static INPUT_PORTS_START( othldrby )
	PORT_INCLUDE( toaplan2_3b )

	PORT_MODIFY("SYS")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_SERVICE( 0x04, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( enmadaio )
	PORT_START("SYS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_TILT )
	TOAPLAN_TEST_SWITCH( 0x04, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x0001, 0x0000, "Game Mode" )
	PORT_DIPSETTING(      0x0000, "Normal Game" )
	PORT_DIPSETTING(      0x0001, "Stop and Slow Mode" )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) ) // unused
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 4C_1C ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) ) // unused
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) ) // unused
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )

	PORT_START("MISC2") // should be DSWB? but not even read in test mode display?
	PORT_DIPNAME( 0x0001, 0x0001, "2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("MISC3")
	PORT_DIPNAME( 0x0001, 0x0001, "3" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("yes")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("no")
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("MISC4")
	PORT_DIPNAME( 0x0001, 0x0001, "4" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, "Status of Something 1" ) // gives error in attract mode otherwise
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, "Status of Something 2" ) // turn this off and it comes up with an error
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("UNK")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) // this is the sensor, not sure what it measures, pulse / heartbeat?
	PORT_DIPNAME( 0x0100, 0x0100, "Freeze" ) // not sure
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

INPUT_PORTS_END

static INPUT_PORTS_START( snowbro2 )
	PORT_INCLUDE( toaplan2_2b )

	PORT_START("IN3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START3 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN4")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START4 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x0001,   0x0000, DEF_STR( Continue_Price ) ) PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(        0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(        0x0001, "Discount" )
	// Various features on bit mask 0x000e - see above
	TOAPLAN_COINAGE_DUAL_LOC( JMPR, 0x1c00, 0x0800, SW1 )

	PORT_MODIFY("DSWB")
	// Difficulty on bit mask 0x0003 - see above
	PORT_DIPNAME( 0x000c,   0x0000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(        0x000c, DEF_STR( None ) )
	PORT_DIPSETTING(        0x0008, "200k only" )
	PORT_DIPSETTING(        0x0000, "100k only" )
	PORT_DIPSETTING(        0x0004, "100k and every 500k" )
	PORT_DIPNAME( 0x0030,   0x0000, DEF_STR( Lives ) )      PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(        0x0030, "1" )
	PORT_DIPSETTING(        0x0020, "2" )
	PORT_DIPSETTING(        0x0000, "3" )
	PORT_DIPSETTING(        0x0010, "4" )
	PORT_DIPNAME( 0x0040,   0x0000, "Invulnerability (Cheat)" )     PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,   0x0000, "Maximum Players" )     PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(        0x0080, "2" )
	PORT_DIPSETTING(        0x0000, "4" )

	PORT_START("JMPR")
	PORT_CONFNAME( 0x2000,  0x0000, "Show All Rights Reserved" )    //PORT_CONFLOCATION("JP:!1")
	PORT_CONFSETTING(       0x0000, DEF_STR( No ) )
	PORT_CONFSETTING(       0x2000, DEF_STR( Yes ) )
	PORT_CONFNAME( 0x1c00,  0x0800, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!4,!3,!2")
	PORT_CONFSETTING(       0x0800, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x0400, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x0000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x0c00, DEF_STR( Korea ) )
	PORT_CONFSETTING(       0x1000, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(       0x1400, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(       0x1800, DEF_STR( Southeast_Asia ) )
//  PORT_CONFSETTING(        0x1c00, DEF_STR( Unused ) )
	PORT_BIT( 0xc3ff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( snowbro2b3 )
	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_TILT )
	TOAPLAN_TEST_SWITCH( 0x0400, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START3 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN4")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START4 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Continue_Price ) ) PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, "Discount" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_HIGH, "SW1:!3" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:!7,!8")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_6C ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:!1,!2")
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x000c,   0x0000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(        0x000c, DEF_STR( None ) )
	PORT_DIPSETTING(        0x0008, "200k only" )
	PORT_DIPSETTING(        0x0000, "100k only" )
	PORT_DIPSETTING(        0x0004, "100k and every 500k" )
	PORT_DIPNAME( 0x0030,   0x0000, DEF_STR( Lives ) )      PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(        0x0030, "1" )
	PORT_DIPSETTING(        0x0020, "2" )
	PORT_DIPSETTING(        0x0000, "3" )
	PORT_DIPSETTING(        0x0010, "4" )
	PORT_DIPNAME( 0x0040,   0x0000, "Invulnerability (Cheat)" )     PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,   0x0000, "Maximum Players" )     PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(        0x0080, "2" )
	PORT_DIPSETTING(        0x0000, "4" )
INPUT_PORTS_END


static INPUT_PORTS_START( sstriker )
	PORT_INCLUDE( toaplan2_3b )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x0001,   0x0000, DEF_STR( Free_Play ) )  PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0001, DEF_STR( On ) )
	// Various features on bit mask 0x000e - see above
	TOAPLAN_COINAGE_DUAL_LOC( JMPR, 0x0e, 0x04, SW1 )

	PORT_MODIFY("DSWB")
	// Difficulty on bit mask 0x0003 - see above
	PORT_DIPNAME( 0x000c,   0x0000, DEF_STR( Bonus_Life ) )     PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(        0x000c, DEF_STR( None ) )
	PORT_DIPSETTING(        0x0008, "200k only" )
	PORT_DIPSETTING(        0x0000, "Every 300k" )
	PORT_DIPSETTING(        0x0004, "200k and 500k" )
	PORT_DIPNAME( 0x0030,   0x0000, DEF_STR( Lives ) )          PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(        0x0030, "1" )
	PORT_DIPSETTING(        0x0020, "2" )
	PORT_DIPSETTING(        0x0000, "3" )
	PORT_DIPSETTING(        0x0010, "5" )
	PORT_DIPNAME( 0x0040,   0x0000, "Invulnerability (Cheat)" ) PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,   0x0000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(        0x0080, DEF_STR( No ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( Yes ) )

	PORT_START("JMPR")
	PORT_CONFNAME( 0x0001,  0x0001, "FBI Logo" )        //PORT_CONFLOCATION("JP:!4")
	PORT_CONFSETTING(       0x0001, DEF_STR( Off ) )
	PORT_CONFSETTING(       0x0000, DEF_STR( On ) )
	PORT_CONFNAME( 0x000e,  0x0004, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!3,!2,!1")
	PORT_CONFSETTING(       0x0004, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x0002, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x0000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x0006, DEF_STR( Southeast_Asia ) )
	PORT_CONFSETTING(       0x0008, DEF_STR( China ) )
	PORT_CONFSETTING(       0x000a, DEF_STR( Korea ) )
	PORT_CONFSETTING(       0x000c, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(       0x000e, DEF_STR( Taiwan ) )
INPUT_PORTS_END


static INPUT_PORTS_START( sstrikerk ) // Although the region jumper is functional, it's a Korean board / version
	PORT_INCLUDE( sstriker )

	PORT_MODIFY("JMPR")
	PORT_CONFNAME( 0x000e,  0x000a, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!3,!2,!1")
	PORT_CONFSETTING(       0x0004, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x0002, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x0000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x0006, DEF_STR( Southeast_Asia ) )
	PORT_CONFSETTING(       0x0008, DEF_STR( China ) )
	PORT_CONFSETTING(       0x000a, "Korea (Unite Trading)" )
	PORT_CONFSETTING(       0x000c, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(       0x000e, DEF_STR( Taiwan ) )
INPUT_PORTS_END


static INPUT_PORTS_START( mahoudai )
	PORT_INCLUDE( sstriker )

	PORT_MODIFY("JMPR")
	// Effectively unused by this set - see notes
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( kingdmgp )
	PORT_INCLUDE( sstriker )

	// The code and lookup tables pertaining to the jumpers are almost identical to sstriker.
	// However, this set apparently lacks (reachable) code to display the FBI logo,
	// even though the logo itself is present in the gfx ROMs.
	PORT_MODIFY("JMPR")
	PORT_CONFNAME( 0x0001,  0x0000, DEF_STR( Unused ) ) //PORT_CONFLOCATION("JP:!4")
	PORT_CONFSETTING(       0x0000, DEF_STR( Off ) )
	PORT_CONFSETTING(       0x0001, DEF_STR( On ) )
	PORT_CONFNAME( 0x000e,  0x0004, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!3,!2,!1")
	PORT_CONFSETTING(       0x0004, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x0002, DEF_STR( USA ) )
//  PORT_CONFSETTING(        0x0000, DEF_STR( Japan ) )  // Corrupt title screen and text - use shippumd
	PORT_CONFSETTING(       0x0006, DEF_STR( Southeast_Asia ) )
	PORT_CONFSETTING(       0x0008, DEF_STR( China ) )
	PORT_CONFSETTING(       0x000a, "Korea (Unite Trading license)" )
	PORT_CONFSETTING(       0x000c, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(       0x000e, DEF_STR( Taiwan ) )
INPUT_PORTS_END


static INPUT_PORTS_START( shippumd )
	PORT_INCLUDE( sstriker )

	PORT_MODIFY("JMPR")
	// Title screen and text are corrupt for anything but Japan
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( bgaregga )
	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Unknown/Unused

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Unknown/Unused

	PORT_START("SYS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	TOAPLAN_TEST_SWITCH( 0x04, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Unknown/Unused

	PORT_START("DSWA")
	PORT_SERVICE_DIPLOC(0x0001, IP_ACTIVE_HIGH, "SW1:!1")
	PORT_DIPNAME( 0x0002,   0x0000, "Credits to Start" )    PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(        0x0000, "1" )
	PORT_DIPSETTING(        0x0002, "2" )
	PORT_DIPNAME( 0x001c,   0x0000, DEF_STR( Coin_A ) )     PORT_DIPLOCATION("SW1:!3,!4,!5")
	PORT_DIPSETTING(        0x0018, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(        0x0014, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(        0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(        0x0004, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(        0x0008, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(        0x000c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(        0x001c, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x00e0,   0x0000, DEF_STR( Coin_B ) )     PORT_CONDITION("DSWA", 0x001c, NOTEQUALS, 0x001c)   PORT_DIPLOCATION("SW1:!6,!7,!8")
	PORT_DIPSETTING(        0x00c0, DEF_STR( 4C_1C ) )      PORT_CONDITION("DSWA", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPSETTING(        0x00a0, DEF_STR( 3C_1C ) )      PORT_CONDITION("DSWA", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPSETTING(        0x0080, DEF_STR( 2C_1C ) )      PORT_CONDITION("DSWA", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPSETTING(        0x0000, DEF_STR( 1C_1C ) )      PORT_CONDITION("DSWA", 0x001c, NOTEQUALS, 0x001c)
//  PORT_DIPSETTING(        0x00e0, DEF_STR( 1C_1C ) )      PORT_CONDITION("DSWA", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPSETTING(        0x0020, DEF_STR( 1C_2C ) )      PORT_CONDITION("DSWA", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPSETTING(        0x0040, DEF_STR( 1C_3C ) )      PORT_CONDITION("DSWA", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPSETTING(        0x0060, DEF_STR( 1C_4C ) )      PORT_CONDITION("DSWA", 0x001c, NOTEQUALS, 0x001c)
	// When Coin_A is set to Free_Play, Coin_A becomes Coin_A and Coin_B, and the following dips occur
	PORT_DIPNAME( 0x0020,   0x0000, "Joystick Mode" )       PORT_CONDITION("DSWA", 0x001c, EQUALS, 0x001c)  PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(        0x0000, "90 degrees ACW" )      PORT_CONDITION("DSWA", 0x001c, EQUALS, 0x001c)
	PORT_DIPSETTING(        0x0020, DEF_STR( Normal ) )     PORT_CONDITION("DSWA", 0x001c, EQUALS, 0x001c)
	PORT_DIPNAME( 0x0040,   0x0000, "Effect" )              PORT_CONDITION("DSWA", 0x001c, EQUALS, 0x001c)  PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )        PORT_CONDITION("DSWA", 0x001c, EQUALS, 0x001c)
	PORT_DIPSETTING(        0x0040, DEF_STR( On ) )         PORT_CONDITION("DSWA", 0x001c, EQUALS, 0x001c)
	PORT_DIPNAME( 0x0080,   0x0000, "Music" )               PORT_CONDITION("DSWA", 0x001c, EQUALS, 0x001c)  PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )        PORT_CONDITION("DSWA", 0x001c, EQUALS, 0x001c)
	PORT_DIPSETTING(        0x0080, DEF_STR( On ) )         PORT_CONDITION("DSWA", 0x001c, EQUALS, 0x001c)

	PORT_START("DSWB")
	PORT_DIPNAME( 0x0003,   0x0000, DEF_STR( Difficulty ) )     PORT_DIPLOCATION("SW2:!1,!2")
	PORT_DIPSETTING(        0x0001, DEF_STR( Easy ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(        0x0002, DEF_STR( Hard ) )
	PORT_DIPSETTING(        0x0003, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0004,   0x0000, DEF_STR( Flip_Screen ) )    PORT_DIPLOCATION("SW2:!3")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008,   0x0000, DEF_STR( Demo_Sounds ) )    PORT_DIPLOCATION("SW2:!4")
	PORT_DIPSETTING(        0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0070,   0x0000, DEF_STR( Lives ) )          PORT_DIPLOCATION("SW2:!5,!6,!7")
	PORT_DIPSETTING(        0x0030, "1" )
	PORT_DIPSETTING(        0x0020, "2" )
	PORT_DIPSETTING(        0x0000, "3" )
	PORT_DIPSETTING(        0x0010, "4" )
	PORT_DIPSETTING(        0x0040, "5" )
	PORT_DIPSETTING(        0x0050, "6" )
	PORT_DIPSETTING(        0x0060, DEF_STR( Infinite ) )
	PORT_DIPSETTING(        0x0070, "Invulnerability (Cheat)" )
	PORT_DIPNAME( 0x0080,   0x0000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(        0x0000, DEF_STR( None ) )       PORT_CONDITION("JMPR",0x0003,NOTEQUALS,0x0000)  // Non-Japan
	PORT_DIPSETTING(        0x0080, "Every 2000k" )         PORT_CONDITION("JMPR",0x0003,NOTEQUALS,0x0000)  // Non-Japan
	PORT_DIPSETTING(        0x0080, "1000k and 2000k" )     PORT_CONDITION("JMPR",0x0003,EQUALS,0x0000) // Japan
	PORT_DIPSETTING(        0x0000, "Every 1000k" )         PORT_CONDITION("JMPR",0x0003,EQUALS,0x0000) // Japan

	PORT_START("JMPR") // DSW3 and jumper
	PORT_DIPNAME( 0x0008,  0x0000, "Stage Edit" ) PORT_DIPLOCATION("SW3:!1")
	PORT_DIPSETTING(       0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004,  0x0000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW3:!2")
	PORT_DIPSETTING(       0x0004, DEF_STR( No ) )
	PORT_DIPSETTING(       0x0000, DEF_STR( Yes ) )
	PORT_CONFNAME( 0x0003,  0x0001, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!2,!1")
	PORT_CONFSETTING(       0x0001, "Europe (Tuning)" )
	PORT_CONFSETTING(       0x0002, "USA (Fabtek)" )
	PORT_CONFSETTING(       0x0000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x0003, DEF_STR( Asia ) )
INPUT_PORTS_END


static INPUT_PORTS_START( bgareggahk )
	PORT_INCLUDE( bgaregga )

	PORT_MODIFY("JMPR")
	PORT_CONFNAME( 0x0003,  0x0003, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!2,!1")
	PORT_CONFSETTING(       0x0001, "Austria (Tuning)" )
	// These two settings end up reporting ROM-0 as BAD
//  PORT_CONFSETTING(        0x0002, "USA (Fabtek)" )
//  PORT_CONFSETTING(        0x0000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x0003, "Hong Kong (Metrotainment)" )
INPUT_PORTS_END


static INPUT_PORTS_START( bgareggatw )
	PORT_INCLUDE( bgaregga )

	PORT_MODIFY("JMPR")
	PORT_CONFNAME( 0x0003,  0x0003, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!2,!1")
	PORT_CONFSETTING(       0x0001, "Germany (Tuning)" )
	// These two settings end up reporting ROM-0 as BAD
//  PORT_CONFSETTING(        0x0002, "USA (Fabtek)" )
//  PORT_CONFSETTING(        0x0000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x0003, "Taiwan (Liang Hwa)" )
INPUT_PORTS_END


static INPUT_PORTS_START( bgareggak )
	PORT_INCLUDE( bgaregga )

	PORT_MODIFY("JMPR")
	PORT_CONFNAME( 0x0003,  0x0003, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!2,!1")
	PORT_CONFSETTING(       0x0001, "Greece" )
	PORT_CONFSETTING(       0x0003, "Korea" )
INPUT_PORTS_END


static INPUT_PORTS_START( bgareggacn )
	PORT_INCLUDE( bgaregga )

	PORT_MODIFY("JMPR")
	PORT_CONFNAME( 0x0003,  0x0003, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!2,!1")
	PORT_CONFSETTING(       0x0001, "Denmark (Tuning)" )
	// These two settings end up reporting ROM-0 as BAD
//  PORT_CONFSETTING(        0x0002, "USA (Fabtek)" )
//  PORT_CONFSETTING(        0x0000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x0003, DEF_STR( China ) )
INPUT_PORTS_END


static INPUT_PORTS_START( bgareggabl )
	PORT_INCLUDE( bgaregga )

	PORT_MODIFY("DSWB") // Always uses bonus life settings from Japan region
	PORT_DIPNAME( 0x0080,   0x0000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(        0x0080, "1000k and 2000k" )
	PORT_DIPSETTING(        0x0000, "Every 1000k" )

	PORT_MODIFY("JMPR") // Region is hard-coded
	PORT_BIT( 0x0003, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( batrider )
	PORT_START("IN")        // Player Inputs
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")       // DSWA and DSWB
	PORT_SERVICE_DIPLOC(0x0001, IP_ACTIVE_HIGH, "SW1:!1")
	PORT_DIPNAME( 0x0002,   0x0000, "Credits to Start" )    PORT_CONDITION("DSW", 0x001c, NOTEQUALS, 0x001c)    PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(        0x0000, "1" )                   PORT_CONDITION("DSW", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPSETTING(        0x0002, "2" )                   PORT_CONDITION("DSW", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPNAME( 0x0002,   0x0000, "Joystick Mode" )       PORT_CONDITION("DSW", 0x001c, EQUALS, 0x001c)       PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(        0x0000, DEF_STR( Normal ) )     PORT_CONDITION("DSW", 0x001c, EQUALS, 0x001c)
	PORT_DIPSETTING(        0x0002, "90 degrees ACW" )      PORT_CONDITION("DSW", 0x001c, EQUALS, 0x001c)
	PORT_DIPNAME( 0x001c,   0x0000, DEF_STR( Coin_A ) )     PORT_DIPLOCATION("SW1:!3,!4,!5")
	PORT_DIPSETTING(        0x0018, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(        0x0014, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(        0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(        0x0004, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(        0x0008, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(        0x000c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(        0x001c, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x00e0,   0x0000, DEF_STR( Coin_B ) )     PORT_CONDITION("DSW", 0x001c, NOTEQUALS, 0x001c)    PORT_DIPLOCATION("SW1:!6,!7,!8")
	PORT_DIPSETTING(        0x00c0, DEF_STR( 4C_1C ) )      PORT_CONDITION("DSW", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPSETTING(        0x00a0, DEF_STR( 3C_1C ) )      PORT_CONDITION("DSW", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPSETTING(        0x0080, DEF_STR( 2C_1C ) )      PORT_CONDITION("DSW", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPSETTING(        0x0000, DEF_STR( 1C_1C ) )      PORT_CONDITION("DSW", 0x001c, NOTEQUALS, 0x001c)
//  PORT_DIPSETTING(        0x00e0, DEF_STR( 1C_1C ) )      PORT_CONDITION("DSW", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPSETTING(        0x0020, DEF_STR( 1C_2C ) )      PORT_CONDITION("DSW", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPSETTING(        0x0040, DEF_STR( 1C_3C ) )      PORT_CONDITION("DSW", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPSETTING(        0x0060, DEF_STR( 1C_4C ) )      PORT_CONDITION("DSW", 0x001c, NOTEQUALS, 0x001c)
	// When Coin_A is set to Free_Play, Coin_A becomes Coin_A and Coin_B, and the following dips occur
	PORT_DIPNAME( 0x0020,   0x0000, "Hit Score" )           PORT_CONDITION("DSW", 0x001c, EQUALS, 0x001c)   PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )        PORT_CONDITION("DSW", 0x001c, EQUALS, 0x001c)
	PORT_DIPSETTING(        0x0020, DEF_STR( On ) )         PORT_CONDITION("DSW", 0x001c, EQUALS, 0x001c)
	PORT_DIPNAME( 0x0040,   0x0000, "Sound Effect" )        PORT_CONDITION("DSW", 0x001c, EQUALS, 0x001c)   PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )        PORT_CONDITION("DSW", 0x001c, EQUALS, 0x001c)
	PORT_DIPSETTING(        0x0040, DEF_STR( On ) )         PORT_CONDITION("DSW", 0x001c, EQUALS, 0x001c)
	PORT_DIPNAME( 0x0080,   0x0000, "Music" )               PORT_CONDITION("DSW", 0x001c, EQUALS, 0x001c)   PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )        PORT_CONDITION("DSW", 0x001c, EQUALS, 0x001c)
	PORT_DIPSETTING(        0x0080, DEF_STR( On ) )         PORT_CONDITION("DSW", 0x001c, EQUALS, 0x001c)
	PORT_DIPNAME( 0x0300,   0x0000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:!1,!2")
	PORT_DIPSETTING(        0x0100, DEF_STR( Easy ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(        0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(        0x0300, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0c00,   0x0000, "Timer" )               PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(        0x0c00, DEF_STR( Highest ) )
	PORT_DIPSETTING(        0x0800, DEF_STR( High ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(        0x0400, DEF_STR( Low ) )
	PORT_DIPNAME( 0x3000,   0x0000, DEF_STR( Lives ) )      PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(        0x3000, "1" )
	PORT_DIPSETTING(        0x2000, "2" )
	PORT_DIPSETTING(        0x0000, "3" )
	PORT_DIPSETTING(        0x1000, "4" )
	PORT_DIPNAME( 0xc000,   0x0000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:!7,!8")
	PORT_DIPSETTING(        0xc000, DEF_STR( None ) )
	PORT_DIPSETTING(        0x8000, "Every 2000k" )
	PORT_DIPSETTING(        0x0000, "Every 1500k" )
	PORT_DIPSETTING(        0x4000, "Every 1000k" )

	PORT_START("SYS-DSW")   // Coin/System and DSWC
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	TOAPLAN_TEST_SWITCH( 0x0004, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_DIPNAME( 0x0100,   0x0000, DEF_STR( Flip_Screen ) )    PORT_DIPLOCATION("SW3:!1")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200,   0x0000, DEF_STR( Demo_Sounds ) )    PORT_DIPLOCATION("SW3:!2")
	PORT_DIPSETTING(        0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400,   0x0000, "Stage Edit" )              PORT_DIPLOCATION("SW3:!3")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800,   0x0000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW3:!4")
	PORT_DIPSETTING(        0x0800, DEF_STR( No ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x1000,   0x0000, "Invulnerability (Cheat)" )         PORT_DIPLOCATION("SW3:!5")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x1000, DEF_STR( On ) )
	// These dips are shown only when Coin_A is set to Free_Play, but they work in normal play mode too
	PORT_DIPNAME( 0x2000,   0x0000, "Guest Players" )           PORT_DIPLOCATION("SW3:!6")
	PORT_DIPSETTING(        0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000,   0x0000, "Player Select" )           PORT_DIPLOCATION("SW3:!7")
	PORT_DIPSETTING(        0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000,   0x0000, "Special Course" )          PORT_DIPLOCATION("SW3:!8")
	PORT_DIPSETTING(        0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( batriderj )
	PORT_INCLUDE( batrider )

	PORT_MODIFY("SYS-DSW")  // Coin/System and DSWC
	// These dips are shown only when Coin_A is set to Free_Play, but they work in normal play mode too
	PORT_DIPNAME( 0x2000,   0x0000, "Guest Players" )           PORT_DIPLOCATION("SW3:!6")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000,   0x0000, "Player Select" )           PORT_DIPLOCATION("SW3:!7")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000,   0x0000, "Special Course" )          PORT_DIPLOCATION("SW3:!8")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x8000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( bbakraid )
	PORT_INCLUDE( batrider )

	PORT_MODIFY("DSW")       // DSWA and DSWB
	PORT_DIPNAME( 0xc000,   0x0000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:!7,!8")
	PORT_DIPSETTING(        0xc000, DEF_STR( None ) )
	PORT_DIPSETTING(        0x8000, "Every 4000k" )
	PORT_DIPSETTING(        0x4000, "Every 3000k" )
	PORT_DIPSETTING(        0x0000, "Every 2000k" )

	PORT_MODIFY("SYS-DSW")   // Coin/System and DSW-3
	PORT_DIPNAME( 0x2000,   0x0000, "Save Scores" )             PORT_DIPLOCATION("SW3:!6")
	PORT_DIPSETTING(        0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000,   0x0000, DEF_STR( Unused ) )         PORT_DIPLOCATION("SW3:!7")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000,   0x0000, DEF_STR( Unused ) )         PORT_DIPLOCATION("SW3:!8")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x8000, DEF_STR( On ) )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
INPUT_PORTS_END


static INPUT_PORTS_START( nprobowl )
	PORT_START("IN")        // Player Inputs
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Set (Relay)")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Gutter L")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Gutter R")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Ballout")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Ballpass")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Mot Home")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // no effect in test mode

	PORT_START("DSW")       // SW0323 and SW0324
	PORT_SERVICE_DIPLOC(   0x0001, IP_ACTIVE_HIGH, "SW0323:!1")
	PORT_DIPUNKNOWN_DIPLOC(0x0002, 0x0000, "SW0323:!2")
	PORT_DIPNAME(          0x0004, 0x0004, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW0323:!3")
	PORT_DIPSETTING(       0x0004, DEF_STR( On ) )
	PORT_DIPSETTING(       0x0000, DEF_STR( Off ) )
	PORT_DIPUNKNOWN_DIPLOC(0x0008, 0x0000, "SW0323:!4")
	PORT_DIPNAME(          0x0070, 0x0000, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW0323:!5,!6,!7")
	PORT_DIPSETTING(       0x0070, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(       0x0060, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(       0x0050, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(       0x0040, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(       0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(       0x0010, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(       0x0020, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(       0x0030, DEF_STR( 1C_4C ) )
	PORT_DIPNAME(          0x0080, 0x0000, DEF_STR( Free_Play ) )  PORT_DIPLOCATION("SW0323:!8")
	PORT_DIPSETTING(       0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x0080, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC(0x0100, 0x0000, "SW0324:!1")
	PORT_DIPUNKNOWN_DIPLOC(0x0200, 0x0000, "SW0324:!2")
	PORT_DIPUNKNOWN_DIPLOC(0x0400, 0x0000, "SW0324:!3")
	PORT_DIPUNKNOWN_DIPLOC(0x0800, 0x0000, "SW0324:!4")
	PORT_DIPUNKNOWN_DIPLOC(0x1000, 0x0000, "SW0324:!5")
	PORT_DIPUNKNOWN_DIPLOC(0x2000, 0x0000, "SW0324:!6")
	PORT_DIPUNKNOWN_DIPLOC(0x4000, 0x0000, "SW0324:!7")
	PORT_DIPUNKNOWN_DIPLOC(0x8000, 0x0000, "SW0324:!8")

	PORT_START("UNK")   // ??
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


// Text layer graphics -- ROM based in some games, RAM based in others
// See video/gp9001.cpp for the main graphics layouts

#define XOR(a) WORD_XOR_LE(a)
#define LOC(x) (x+XOR(0))

static const gfx_layout truxton2_tx_tilelayout =
{
	8,8,    /* 8x8 characters */
	1024,   /* 1024 characters */
	4,      /* 4 bits per pixel */
	{ STEP4(0,1) },
	{ LOC(0)*4, LOC(1)*4, LOC(4)*4, LOC(5)*4, LOC(8)*4, LOC(9)*4, LOC(12)*4, LOC(13)*4 },
	{ STEP8(0,8*8) },
	8*8*8
};

static const gfx_layout batrider_tx_tilelayout =
{
	8,8,    /* 8x8 characters */
	1024,   /* 1024 characters */
	4,      /* 4 bits per pixel */
	{ STEP4(0,1) },
	{ XOR(0)*4, XOR(1)*4, XOR(2)*4, XOR(3)*4, XOR(4)*4, XOR(5)*4, XOR(6)*4, XOR(7)*4 },
	{ STEP8(0,4*8) },
	8*8*4
};

static GFXDECODE_START( gfx_truxton2 )
	GFXDECODE_ENTRY( nullptr, 0, truxton2_tx_tilelayout, 64*16, 64 )
GFXDECODE_END

static GFXDECODE_START( gfx_textrom )
	GFXDECODE_ENTRY( "text", 0, gfx_8x8x4_packed_msb, 64*16, 64 )
GFXDECODE_END

static GFXDECODE_START( gfx_batrider )
	GFXDECODE_ENTRY( nullptr, 0, batrider_tx_tilelayout, 64*16, 64 )
GFXDECODE_END


void toaplan2_state::tekipaki(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 10_MHz_XTAL);         // 10MHz Oscillator
	m_maincpu->set_addrmap(AS_PROGRAM, &toaplan2_state::tekipaki_68k_mem);
	m_maincpu->reset_cb().set(FUNC(toaplan2_state::toaplan2_reset));

	hd647180x_device &audiocpu(HD647180X(config, m_audiocpu, 10_MHz_XTAL));
	// 16k byte ROM and 512 byte RAM are internal
	audiocpu.set_addrmap(AS_IO, &toaplan2_state::hd647180_io_map);
	audiocpu.in_pa_callback().set(FUNC(toaplan2_state::tekipaki_cmdavailable_r));

	config.set_maximum_quantum(attotime::from_hz(600));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	//m_screen->set_refresh_hz(60);
	//m_screen->set_size(432, 262);
	//m_screen->set_visarea(0, 319, 0, 239);
	m_screen->set_screen_update(FUNC(toaplan2_state::screen_update_toaplan2));
	m_screen->screen_vblank().set(FUNC(toaplan2_state::screen_vblank));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, T2PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp[0], 27_MHz_XTAL);
	m_vdp[0]->set_palette(m_palette);
	m_vdp[0]->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_4);

	MCFG_VIDEO_START_OVERRIDE(toaplan2_state,toaplan2)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch[0]);

	ym3812_device &ymsnd(YM3812(config, "ymsnd", 27_MHz_XTAL/8));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void ghox_state::ghox(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 10_MHz_XTAL);         /* verified on pcb */
	m_maincpu->set_addrmap(AS_PROGRAM, &ghox_state::ghox_68k_mem);
	m_maincpu->reset_cb().set(FUNC(ghox_state::toaplan2_reset));

	HD647180X(config, m_audiocpu, 10_MHz_XTAL);
	m_audiocpu->set_addrmap(AS_PROGRAM, &ghox_state::ghox_hd647180_mem_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	//m_screen->set_refresh_hz(60);
	//m_screen->set_size(432, 262);
	//m_screen->set_visarea(0, 319, 0, 239);
	m_screen->set_screen_update(FUNC(ghox_state::screen_update_toaplan2));
	m_screen->screen_vblank().set(FUNC(ghox_state::screen_vblank));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, T2PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp[0], 27_MHz_XTAL);
	m_vdp[0]->set_palette(m_palette);
	m_vdp[0]->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_4);

	MCFG_VIDEO_START_OVERRIDE(ghox_state,toaplan2)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM2151(config, "ymsnd", 27_MHz_XTAL/8).add_route(ALL_OUTPUTS, "mono", 0.5); // verified on pcb
}

/* probably dogyuun, vfive and kbash use the same decryption table;
those 3 games have been seen with the NITRO905 chip, other alias are
ts002mach for dogyuun, ts004dash for kbash and ts007spy for vfive */

static const u8 nitro_decryption_table[256] = {
	0x1b,0x56,0x75,0x88,0x8c,0x06,0x58,0x72, 0x83,0x86,0x36,0x1a,0x5f,0xd3,0x8c,0xe9, /* 00 */
	/* *//* *//* *//* *//* *//* *//* *//* */ /* *//* *//* *//* *//* *//* *//* *//* */
	0x22,0x0f,0x03,0x2a,0xeb,0x2a,0xf9,0x0f, 0xa4,0xbd,0x75,0xf3,0x4f,0x53,0x8e,0xfe, /* 10 */
	/*W*//*W*//*r*//*W*//*r*//*W*//*W*//*r*/ /*r*//*a*//*r*//*r*//*r*//*W*//*x*//*r*/
	0x87,0xe8,0xb1,0x8d,0x36,0xb5,0x43,0x73, 0x2a,0x5b,0xf9,0x02,0x24,0x8a,0x03,0x80, /* 20 */
	/*a*//*r*//*r*//*r*//*x*//*r*//*r*//*r*/ /*W*//*r*//*W*//*W*//*a*//*r*//*a*//*r*/
	0x86,0x8b,0xd1,0x3e,0x8d,0x3e,0x58,0xfb, 0xc3,0x79,0xbd,0xb7,0x8a,0xe8,0x0f,0x81, /* 30 */
	/*a*//*a*//*a*//*a*//*r*//*r*//*a*//*r*/ /*r*//*a*//*a*//*W*//*r*//*r*//*r*//*r*/
	0xb7,0xd0,0x8b,0xeb,0xff,0xb8,0x90,0x8b, 0x5e,0xa2,0x90,0xc1,0xab,0xb4,0x80,0x59, /* 40 */
	/*r*//*r*//*a*//*r*//*a*//*x*/     /*a*/ /*W*//*W*/          /*r*//*W*//*r*//*a*/
	0x87,0x72,0xb5,0xbd,0xb0,0x88,0x50,0x0f, 0xfe,0xd2,0xc3,0x90,0x8a,0x90,0xf9,0x75, /* 50 */
	/*W*//*a*//*a*//*r*//*r*//*a*//*a*//*a*/ /*r*//*W*//*r*/     /*r*/     /*W*//*r*/
	0x1a,0xb3,0x74,0x0a,0x68,0x24,0xbb,0x90, 0x75,0x47,0xfe,0x2c,0xbe,0xc3,0x88,0xd2, /* 60 */
	/*W*//*r*//*a*//*r*//*a*//*a*//*W*/      /*r*//*a*//*r*//*W*//*W*//*a*//*r*//*a*/
	0x3e,0xc1,0x8c,0x33,0x0f,0x4f,0x8b,0x90, 0xb9,0x1e,0xff,0xa2,0x3e,0x22,0xbe,0x57, /* 70 */
	/*r*//*W*//*r*//*r*//*a*/     /*a*/      /*r*//*r*//*a*//*r*//*a*//*W*//*r*//*a*/
	0x81,0x3a,0xf6,0x88,0xeb,0xb1,0x89,0x8a, 0x32,0x80,0x0f,0xb1,0x48,0xc3,0x68,0x72, /* 80 */
	/*r*//*r*//*r*//*r*//*a*//*W*//*a*//*r*/ /*r*//*r*//*r*//*a*//*x*//*a*//*a*//*r*/
	0x53,0x02,0xc0,0x02,0xe8,0xb4,0x74,0xbc, 0x90,0x58,0x0a,0xf3,0x75,0xc6,0x90,0xe8, /* 90 */
	/*a*//*W*//*r*//*W*//*r*//*r*//*r*//*x*/      /*a*//*r*//*r*//*r*//*x*/     /*r*/
	0x26,0x50,0xfc,0x8c,0x06,0xb1,0xc3,0xd1, 0xeb,0x83,0xa4,0xbf,0x26,0x4b,0x46,0xfe, /* a0 */
	/*r*//*a*//*a*//*r*/     /*a*//*r*//*W*/ /*a*//*r*//*r*//*r*//*r*//*W*//*a*//*r*/
	0xe2,0x89,0xb3,0x88,0x03,0x56,0x0f,0x38, 0xbb,0x0c,0x90,0x0f,0x07,0x8a,0x8a,0x33, /* b0 */
	/*r*//*a*//*W*//*r*//*a*//*W*//*r*//*W*/ /*W*//*W*/     /*a*//*r*//*r*//*r*//*x*/
	0xfe,0xf9,0xb1,0xa0,0x45,0x36,0x22,0x5e, 0x8a,0xbe,0xc6,0xea,0x3c,0xb2,0x1e,0xe8, /* c0 */
	/*r*//*W*//*r*//*r*//*r*//*r*//*W*//*r*/ /*r*//*W*//*x*//*x*//*r*//*?*//*r*//*r*/
	0x90,0xeb,0x55,0xf6,0x8a,0xb0,0x5d,0xc0, 0xbb,0x8d,0xf6,0xd0,0xd1,0x88,0x4d,0x90, /* d0 */
			/*a*//*r*//*r*//*a*//*a*//*r*//*W*/ /*x*//*r*//*r*//*a*//*W*//*r*//*W*/
	0x51,0x51,0x74,0xbd,0x32,0xd1,0xc6,0xd2, 0x53,0xc7,0xab,0x36,0x50,0xe9,0x33,0xb3, /* e0 */
	/*r*//*a*//*r*//*r*//*r*//*W*/     /*a*/ /*r*//*x*//*r*//*r*//*W*//*a*//*r*//*W*/
	0x2e,0x05,0x88,0x59,0x74,0x74,0x22,0x8e, 0x8a,0x8a,0x36,0x08,0x0f,0x45,0x90,0x2e, /* f0 */
	/*r*//*W*//*r*//*r*//*a*//*a*//*W*//*x*/ /*r*//*r*//*x*//*a*//*r*//*a*/     /*r*/
};

/*
dogyuun
a5272 cd

kbash

vfive
a4849 cd

*/

void toaplan2_state::dogyuun(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 25_MHz_XTAL/2);           /* verified on pcb */
	m_maincpu->set_addrmap(AS_PROGRAM, &toaplan2_state::dogyuun_68k_mem);
	m_maincpu->reset_cb().set(FUNC(toaplan2_state::toaplan2_reset));

	v25_device &audiocpu(V25(config, m_audiocpu, 25_MHz_XTAL/2));         /* NEC V25 type Toaplan marked CPU ??? */
	audiocpu.set_addrmap(AS_PROGRAM, &toaplan2_state::v25_mem);
	audiocpu.set_decryption_table(nitro_decryption_table);
	audiocpu.pt_in_cb().set_ioport("DSWB").exor(0xff);
	audiocpu.p0_in_cb().set_ioport("DSWA").exor(0xff);
	audiocpu.p1_in_cb().set_ioport("JMPR").exor(0xff);
	audiocpu.p2_out_cb().set_nop();  // bit 0 is FAULT according to kbash schematic

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	m_screen->set_screen_update(FUNC(toaplan2_state::screen_update_dogyuun));
	m_screen->screen_vblank().set(FUNC(toaplan2_state::screen_vblank));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, T2PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp[0], 27_MHz_XTAL);
	m_vdp[0]->set_palette(m_palette);
	m_vdp[0]->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_4);

	GP9001_VDP(config, m_vdp[1], 27_MHz_XTAL);
	m_vdp[1]->set_palette(m_palette);

	MCFG_VIDEO_START_OVERRIDE(toaplan2_state,toaplan2)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM2151(config, "ymsnd", 27_MHz_XTAL/8).add_route(ALL_OUTPUTS, "mono", 0.5); // verified on pcb

	OKIM6295(config, m_oki[0], 25_MHz_XTAL/24, okim6295_device::PIN7_HIGH); // verified on PCB
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 0.5);
}


void toaplan2_state::dogyuunto(machine_config &config)
{
	dogyuun(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &toaplan2_state::dogyuunto_68k_mem);
	m_maincpu->set_clock(24_MHz_XTAL / 2); // 24 MHz instead of 25

	z80_device &audiocpu(Z80(config.replace(), "audiocpu", 27_MHz_XTAL / 8)); // guessed divisor
	audiocpu.set_addrmap(AS_PROGRAM, &toaplan2_state::dogyuunto_sound_z80_mem);

	m_oki[0]->set_clock(1.056_MHz_XTAL); // blue resonator 1056J
}


void toaplan2_state::kbash(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 16_MHz_XTAL);         /* 16MHz Oscillator */
	m_maincpu->set_addrmap(AS_PROGRAM, &toaplan2_state::kbash_68k_mem);
	m_maincpu->reset_cb().set(FUNC(toaplan2_state::toaplan2_reset));

	/* ROM based v25 */
	v25_device &audiocpu(V25(config, m_audiocpu, 16_MHz_XTAL));         /* NEC V25 type Toaplan marked CPU ??? */
	audiocpu.set_addrmap(AS_PROGRAM, &toaplan2_state::kbash_v25_mem);
	audiocpu.set_decryption_table(nitro_decryption_table);
	audiocpu.pt_in_cb().set_ioport("DSWA").exor(0xff);
	audiocpu.p0_in_cb().set_ioport("DSWB").exor(0xff);
	audiocpu.p1_in_cb().set_ioport("JMPR").exor(0xff);
	audiocpu.p2_out_cb().set_nop();  // bit 0 is FAULT according to kbash schematic

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	//m_screen->set_refresh_hz(60);
	//m_screen->set_size(432, 262);
	//m_screen->set_visarea(0, 319, 0, 239);
	m_screen->set_screen_update(FUNC(toaplan2_state::screen_update_toaplan2));
	m_screen->screen_vblank().set(FUNC(toaplan2_state::screen_vblank));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, T2PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp[0], 27_MHz_XTAL);
	m_vdp[0]->set_palette(m_palette);
	m_vdp[0]->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_4);

	MCFG_VIDEO_START_OVERRIDE(toaplan2_state,toaplan2)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM2151(config, "ymsnd", 27_MHz_XTAL/8).add_route(ALL_OUTPUTS, "mono", 0.5);

	OKIM6295(config, m_oki[0], 32_MHz_XTAL/32, okim6295_device::PIN7_HIGH);
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 0.5);
}


void toaplan2_state::kbash2(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 16_MHz_XTAL);         /* 16MHz Oscillator */
	m_maincpu->set_addrmap(AS_PROGRAM, &toaplan2_state::kbash2_68k_mem);
	m_maincpu->reset_cb().set(FUNC(toaplan2_state::toaplan2_reset));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	//m_screen->set_refresh_hz(60);
	//m_screen->set_size(432, 262);
	//m_screen->set_visarea(0, 319, 0, 239);
	m_screen->set_screen_update(FUNC(toaplan2_state::screen_update_toaplan2));
	m_screen->screen_vblank().set(FUNC(toaplan2_state::screen_vblank));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, T2PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp[0], 27_MHz_XTAL);
	m_vdp[0]->set_palette(m_palette);
	m_vdp[0]->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_4);

	MCFG_VIDEO_START_OVERRIDE(toaplan2_state,toaplan2)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki[0], 16_MHz_XTAL/16, okim6295_device::PIN7_HIGH);
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 1.0);

	OKIM6295(config, m_oki[1], 16_MHz_XTAL/16, okim6295_device::PIN7_HIGH);
	m_oki[1]->add_route(ALL_OUTPUTS, "mono", 1.0);
}


void truxton2_state::truxton2(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 16_MHz_XTAL);         /* verified on pcb */
	m_maincpu->set_addrmap(AS_PROGRAM, &truxton2_state::truxton2_68k_mem);
	m_maincpu->reset_cb().set(FUNC(truxton2_state::toaplan2_reset));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	m_screen->set_screen_update(FUNC(truxton2_state::screen_update_truxton2));
	m_screen->screen_vblank().set(FUNC(truxton2_state::screen_vblank));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_truxton2);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, T2PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp[0], 27_MHz_XTAL);
	m_vdp[0]->set_palette(m_palette);
	m_vdp[0]->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_2);

	MCFG_VIDEO_START_OVERRIDE(truxton2_state,truxton2)

	/* sound hardware */
#ifdef TRUXTON2_STEREO  // music data is stereo...
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	YM2151(config, "ymsnd", 27_MHz_XTAL/8).add_route(0, "lspeaker", 1.0).add_route(1, "rspeaker", 1.0);

	OKIM6295(config, m_oki[0], 16_MHz_XTAL/4, okim6295_device::PIN7_LOW);
	m_oki[0]->add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	m_oki[0]->add_route(ALL_OUTPUTS, "rspeaker", 1.0);
#else   // ...but the hardware is mono
	SPEAKER(config, "mono").front_center();

	YM2151(config, "ymsnd", 27_MHz_XTAL/8).add_route(ALL_OUTPUTS, "mono", 1.0); // verified on PCB

	OKIM6295(config, m_oki[0], 16_MHz_XTAL/4, okim6295_device::PIN7_LOW); // verified on PCB
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 1.0);
#endif
}


void toaplan2_state::pipibibs(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 10_MHz_XTAL);         // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &toaplan2_state::pipibibs_68k_mem);
	m_maincpu->reset_cb().set(FUNC(toaplan2_state::toaplan2_reset));

	Z80(config, m_audiocpu, 27_MHz_XTAL/8);         // verified on PCB
	m_audiocpu->set_addrmap(AS_PROGRAM, &toaplan2_state::pipibibs_sound_z80_mem);

	config.set_maximum_quantum(attotime::from_hz(600));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	//m_screen->set_refresh_hz(60);
	//m_screen->set_size(432, 262);
	//m_screen->set_visarea(0, 319, 0, 239);
	m_screen->set_screen_update(FUNC(toaplan2_state::screen_update_toaplan2));
	m_screen->screen_vblank().set(FUNC(toaplan2_state::screen_vblank));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, T2PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp[0], 27_MHz_XTAL);
	m_vdp[0]->set_palette(m_palette);
	m_vdp[0]->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_4);

	MCFG_VIDEO_START_OVERRIDE(toaplan2_state,toaplan2)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym3812_device &ymsnd(YM3812(config, "ymsnd", 27_MHz_XTAL/8)); // verified on PCB
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);
}


void toaplan2_state::pipibibsbl(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 12_MHz_XTAL); // ??? (position labeled "68000-12" but 10 MHz-rated parts used)
	m_maincpu->set_addrmap(AS_PROGRAM, &toaplan2_state::pipibibi_bootleg_68k_mem);
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &toaplan2_state::cpu_space_pipibibsbl_map);
	m_maincpu->reset_cb().set(FUNC(toaplan2_state::toaplan2_reset));

	Z80(config, m_audiocpu, 12_MHz_XTAL / 2); // GoldStar Z8400B; clock source and divider unknown
	m_audiocpu->set_addrmap(AS_PROGRAM, &toaplan2_state::pipibibs_sound_z80_mem);

	config.set_maximum_quantum(attotime::from_hz(600));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(28.322_MHz_XTAL / 4, 450, 0, 320, 262, 0, 240); // guess, but this is within NTSC parameters
	m_screen->set_screen_update(FUNC(toaplan2_state::screen_update_toaplan2));
	m_screen->screen_vblank().set(FUNC(toaplan2_state::screen_vblank));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, T2PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp[0], 27_MHz_XTAL); // FIXME: bootleg has no VDP
	m_vdp[0]->set_palette(m_palette);
	m_vdp[0]->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_4, ASSERT_LINE);
	m_vdp[0]->set_bootleg_extra_offsets(0x01f, 0x1ef, 0x01d, 0x1ef, 0x01b, 0x1ef, 0x1d4, 0x1f7);

	MCFG_VIDEO_START_OVERRIDE(toaplan2_state,toaplan2)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym3812_device &ymsnd(YM3812(config, "ymsnd", 28.322_MHz_XTAL / 8)); // ???
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);
}

/* x = modified to match batsugun 'unencrypted' code - '?' likewise, but not so sure about them */
/* e = opcodes used in the EEPROM service routine */
/* this one seems more different to the other tables */
static const u8 ts001turbo_decryption_table[256] = {
	0x90,0x05,0x57,0x5f,0xfe,0x4f,0xbd,0x36, 0x80,0x8b,0x8a,0x0a,0x89,0x90,0x47,0x80, /* 00 */
			/*r*//*r*//*r*//*r*//*r*//*r*//*r*/ /*r*//*r*//*r*//*r*//*r*/     /*r*//*r*/
	0x22,0x90,0x90,0x5d,0x81,0x3c,0xb5,0x83, 0x68,0xff,0x75,0x75,0x8d,0x5b,0x8a,0x38, /* 10 */
	/*r*/          /*r*//*r*//*r*//*r*//*r*/ /*r*//*r*//*r*//*r*//*r*//*r*//*r*//*r*/
	0x8b,0xeb,0xd2,0x0a,0xb4,0xc7,0x46,0xd1, 0x0a,0x53,0xbd,0x77,0x22,0xff,0x1f,0x03, /* 20 */
	/*a*//*r*//*r*//*r*//*r*//*r*//*r*//*r*/ /*r*//*r*//*r*//*e*//*r*//*r*//*?*//*r*/
	0xfb,0x45,0xc3,0x02,0x90,0x0f,0xa3,0x02, 0x0f,0xb7,0x90,0x24,0xc6,0xeb,0x1b,0x32, /* 30 */
	/*r*//*r*//*r*//*r*/     /*r*//*e*//*r*/ /*r*//*r*/     /*r*//*r*//*r*//*r*//*r*/
	0x8d,0xb9,0xfe,0x08,0x88,0x90,0x8a,0x8a, 0x75,0x8a,0xbd,0x58,0xfe,0x51,0x1e,0x8b, /* 40 */
	/*r*//*r*//*r*//*r*//*r*/     /*r*//*r*/ /*r*//*r*//*r*//*r*//*r*//*r*//*r*//*r*/
	0x0f,0x22,0xf6,0x90,0xc3,0x36,0x03,0x8d, 0xbb,0x16,0xbc,0x90,0x0f,0x5e,0xf9,0x2e, /* 50 */
	/*r*//*r*//*r*/     /*r*//*r*//*r*//*r*/ /*r*//*?*//*r*/     /*r*//*r*//*r*//*r*/
	0x90,0x90,0x59,0x90,0xbb,0x1a,0x0c,0x8d, 0x89,0x72,0x83,0xa4,0xc3,0xb3,0x8b,0xe9, /* 60 */
				/*r*/     /*r*//*r*//*r*//*r*/ /*a*//*r*//*r*//*r*//*r*//*r*//*r*//*r*/
	0x81,0x43,0xa0,0x2c,0x0f,0x55,0xf3,0x36, 0xb0,0x59,0xe8,0x03,0x26,0xe9,0x22,0xb0, /* 70 */
	/*r*//*r*//*r*//*r*//*r*//*r*//*r*//*r*/ /*r*//*r*//*r*//*r*//*r*//*r*//*r*//*r*/
	0x90,0x8e,0x24,0x8a,0xd0,0x3e,0xc3,0x3a, 0x90,0x79,0x57,0x16,0x88,0x86,0x24,0x74, /* 80 */
			/*r*//*r*//*r*//*r*//*r*//*r*//*r*/      /*a*//*r*//*r*//*r*//*r*//*r*//*r*/
	0x33,0xc3,0x53,0xb8,0xab,0x75,0x90,0x90, 0x8e,0xb1,0xe9,0x5d,0xf9,0x02,0x3c,0x90, /* 90 */
	/*x*//*r*//*r*//*r*//*r*//*r*/           /*r*//*r*//*r*//*r*//*r*//*r*//*r*/
	0x80,0xd3,0x89,0xe8,0x90,0x90,0x2a,0x74, 0x90,0x5f,0xf6,0x88,0x4f,0x56,0x8c,0x03, /* a0 */
	/*r*//*a*//*r*//*r*/          /*r*//*r*/      /*r*//*r*//*r*//*r*//*r*//*r*//*r*/
	0x47,0xa1,0x88,0x90,0x03,0xfe,0x90,0xfc, 0x2a,0x90,0x33,0x07,0xb1,0x50,0x0f,0x3e, /* b0 */
	/*r*//*e*//*r*/     /*r*//*r*/     /*r*/ /*r*/     /*r*//*r*//*r*//*r*//*r*//*r*/
	0xbd,0x4d,0xf3,0xbf,0x59,0xd2,0xea,0xc6, 0x2a,0x74,0x72,0xe2,0x3e,0x2e,0x90,0x2e, /* c0 */
	/*r*//*r*//*r*//*r*//*r*//*a*//*x*//*r*/ /*r*//*r*//*r*//*r*//*r*//*r*/     /*r*/
	0x2e,0x73,0x88,0x72,0x45,0x5d,0xc1,0xb9, 0x32,0x38,0x88,0xc1,0xa0,0x06,0x45,0x90, /* d0 */
	/*r*//*r*//*r*//*r*//*r*//*r*//*r*//*r*/ /*r*//*r*//*r*//*r*//*a*//*r*//*r*/
	0x90,0x86,0x4b,0x87,0x90,0x8a,0x3b,0xab, 0x33,0xbe,0x90,0x32,0xbd,0xc7,0xb2,0x80, /* e0 */
			/*r*//*r*//*r*/     /*r*//*?*//*r*/ /*r*//*r*/     /*r*//*r*//*r*//*?*//*r*/
	0x0f,0x75,0xc0,0xb9,0x07,0x74,0x3e,0xa2, 0x8a,0x48,0x3e,0x8d,0xeb,0x90,0xfe,0x90, /* f0 */
	/*r*//*r*//*r*//*r*//*r*//*r*//*r*//*r*/ /*r*//*x*//*r*//*r*//*r*/     /*r*/
};


void truxton2_state::fixeight(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 16_MHz_XTAL);         // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &truxton2_state::fixeight_68k_mem);
	m_maincpu->reset_cb().set(FUNC(truxton2_state::toaplan2_reset));

	v25_device &audiocpu(V25(config, m_audiocpu, 16_MHz_XTAL));           // NEC V25 type Toaplan marked CPU ???
	audiocpu.set_addrmap(AS_PROGRAM, &truxton2_state::fixeight_v25_mem);
	audiocpu.set_decryption_table(ts001turbo_decryption_table);
	audiocpu.p0_in_cb().set_ioport("EEPROM");
	audiocpu.p0_out_cb().set_ioport("EEPROM");

	EEPROM_93C46_16BIT(config, m_eeprom);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240); // verified on PCB
	m_screen->set_screen_update(FUNC(truxton2_state::screen_update_truxton2));
	m_screen->screen_vblank().set(FUNC(truxton2_state::screen_vblank));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_truxton2);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, T2PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp[0], 27_MHz_XTAL);
	m_vdp[0]->set_palette(m_palette);
	m_vdp[0]->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_4);

	MCFG_VIDEO_START_OVERRIDE(truxton2_state,truxton2)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM2151(config, "ymsnd", 27_MHz_XTAL/8).add_route(ALL_OUTPUTS, "mono", 0.5); /* verified on pcb */

	OKIM6295(config, m_oki[0], 16_MHz_XTAL/16, okim6295_device::PIN7_HIGH); /* verified on pcb */
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 0.5);
}


void truxton2_state::fixeightbl(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(10'000'000));         /* 10MHz Oscillator */
	m_maincpu->set_addrmap(AS_PROGRAM, &truxton2_state::fixeightbl_68k_mem);
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &truxton2_state::cpu_space_fixeightbl_map);
	m_maincpu->reset_cb().set(FUNC(truxton2_state::toaplan2_reset));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	//m_screen->set_refresh_hz(60);
	//m_screen->set_size(432, 262);
	//m_screen->set_visarea(0, 319, 0, 239);
	m_screen->set_screen_update(FUNC(truxton2_state::screen_update_bootleg));
	m_screen->screen_vblank().set(FUNC(truxton2_state::screen_vblank));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_textrom);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, T2PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp[0], 27_MHz_XTAL);
	m_vdp[0]->set_palette(m_palette);
	m_vdp[0]->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_2, ASSERT_LINE);

	MCFG_VIDEO_START_OVERRIDE(truxton2_state,fixeightbl)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki[0], 14_MHz_XTAL/16, okim6295_device::PIN7_LOW);
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 1.0);
	m_oki[0]->set_addrmap(0, &truxton2_state::fixeightbl_oki);
}


void toaplan2_state::vfive(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 20_MHz_XTAL/2);   // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &toaplan2_state::vfive_68k_mem);
	m_maincpu->reset_cb().set(FUNC(toaplan2_state::toaplan2_reset));

	v25_device &audiocpu(V25(config, m_audiocpu, 20_MHz_XTAL/2)); // Verified on PCB, NEC V25 type Toaplan mark scratched out
	audiocpu.set_addrmap(AS_PROGRAM, &toaplan2_state::vfive_v25_mem);
	audiocpu.set_decryption_table(nitro_decryption_table);
	audiocpu.pt_in_cb().set_ioport("DSWA").exor(0xff);
	audiocpu.p0_in_cb().set_ioport("DSWB").exor(0xff);
	audiocpu.p1_in_cb().set_ioport("JMPR").exor(0xff);
	audiocpu.p2_out_cb().set_nop();  // bit 0 is FAULT according to kbash schematic

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240); // verified on PCB
	m_screen->set_screen_update(FUNC(toaplan2_state::screen_update_toaplan2));
	m_screen->screen_vblank().set(FUNC(toaplan2_state::screen_vblank));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, T2PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp[0], 27_MHz_XTAL);
	m_vdp[0]->set_palette(m_palette);
	m_vdp[0]->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_4);

	MCFG_VIDEO_START_OVERRIDE(toaplan2_state,toaplan2)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM2151(config, "ymsnd", 27_MHz_XTAL/8).add_route(ALL_OUTPUTS, "mono", 0.5); // verified on PCB
}


void toaplan2_state::batsugun(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 32_MHz_XTAL/2);           // 16MHz, 32MHz Oscillator
	m_maincpu->set_addrmap(AS_PROGRAM, &toaplan2_state::batsugun_68k_mem);
	m_maincpu->reset_cb().set(FUNC(toaplan2_state::toaplan2_reset));

	v25_device &audiocpu(V25(config, m_audiocpu, 32_MHz_XTAL/2));         // NEC V25 type Toaplan marked CPU ???
	audiocpu.set_addrmap(AS_PROGRAM, &toaplan2_state::v25_mem);
	audiocpu.pt_in_cb().set_ioport("DSWA").exor(0xff);
	audiocpu.p0_in_cb().set_ioport("DSWB").exor(0xff);
	audiocpu.p1_in_cb().set_ioport("JMPR").exor(0xff);
	audiocpu.p2_out_cb().set_nop();  // bit 0 is FAULT according to kbash schematic

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	//m_screen->set_refresh_hz(60);
	//m_screen->set_size(432, 262);
	//m_screen->set_visarea(0, 319, 0, 239);
	m_screen->set_screen_update(FUNC(toaplan2_state::screen_update_batsugun));
	m_screen->screen_vblank().set(FUNC(toaplan2_state::screen_vblank));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, T2PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp[0], 27_MHz_XTAL);
	m_vdp[0]->set_palette(m_palette);
	m_vdp[0]->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_4);

	GP9001_VDP(config, m_vdp[1], 27_MHz_XTAL);
	m_vdp[1]->set_palette(m_palette);

	MCFG_VIDEO_START_OVERRIDE(toaplan2_state,toaplan2)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM2151(config, "ymsnd", 27_MHz_XTAL/8).add_route(ALL_OUTPUTS, "mono", 0.4);

	OKIM6295(config, m_oki[0], 32_MHz_XTAL/8, okim6295_device::PIN7_LOW);
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 0.4);
}

void toaplan2_state::batsugunbl(machine_config &config)
{
	batsugun(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &toaplan2_state::batsugunbl_68k_mem);
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &toaplan2_state::cpu_space_fixeightbl_map);

	m_vdp[0]->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_2, ASSERT_LINE);

	MCFG_VIDEO_START_OVERRIDE(toaplan2_state, batsugunbl)

	config.device_remove("audiocpu");
	config.device_remove("ymsnd");

	m_oki[0]->set_addrmap(0, &toaplan2_state::fixeightbl_oki);
}

void pwrkick_state::pwrkick(machine_config &config) // Sunwise SW931201-1 PCB (27.000MHz, 20.000MHz & 16.000MHz OSCs)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 20_MHz_XTAL/2); // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &pwrkick_state::pwrkick_68k_mem);
	m_maincpu->reset_cb().set(FUNC(pwrkick_state::toaplan2_reset));

	UPD4992(config, m_rtc, 32.768_kHz_XTAL);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	TICKET_DISPENSER(config, m_hopper, attotime::from_msec(PWRKICK_HOPPER_PULSE), TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_HIGH);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	m_screen->set_screen_update(FUNC(pwrkick_state::screen_update_toaplan2));
	m_screen->screen_vblank().set(FUNC(pwrkick_state::screen_vblank));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, T2PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp[0], 27_MHz_XTAL);
	m_vdp[0]->set_palette(m_palette);
	m_vdp[0]->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_4);

	MCFG_VIDEO_START_OVERRIDE(pwrkick_state,toaplan2)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	// empty YM2151 socket
	OKIM6295(config, m_oki[0], 16_MHz_XTAL/4, okim6295_device::PIN7_LOW); // verified on PCB
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 0.5);
}

void pwrkick_state::othldrby(machine_config &config) // Sunwise S951060-VGP PCB (27.000MHz, 20.000MHz & 16.000MHz OSCs)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 20_MHz_XTAL/2); // assumed same as pwrkick
	m_maincpu->set_addrmap(AS_PROGRAM, &pwrkick_state::othldrby_68k_mem);
	m_maincpu->reset_cb().set(FUNC(pwrkick_state::toaplan2_reset));

	UPD4992(config, m_rtc, 32.768_kHz_XTAL);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	m_screen->set_screen_update(FUNC(pwrkick_state::screen_update_toaplan2));
	m_screen->screen_vblank().set(FUNC(pwrkick_state::screen_vblank));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, T2PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp[0], 27_MHz_XTAL);
	m_vdp[0]->set_palette(m_palette);
	m_vdp[0]->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_4);

	MCFG_VIDEO_START_OVERRIDE(pwrkick_state,toaplan2)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki[0], 16_MHz_XTAL/4, okim6295_device::PIN7_LOW); // assumed same as pwrkick
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 0.5);
}


void toaplan2_state::enmadaio(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 20_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &toaplan2_state::enmadaio_68k_mem);
	m_maincpu->reset_cb().set(FUNC(toaplan2_state::toaplan2_reset));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	m_screen->set_screen_update(FUNC(toaplan2_state::screen_update_toaplan2));
	m_screen->screen_vblank().set(FUNC(toaplan2_state::screen_vblank));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, T2PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp[0], 27_MHz_XTAL);
	m_vdp[0]->set_palette(m_palette);
	m_vdp[0]->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_4);

	MCFG_VIDEO_START_OVERRIDE(toaplan2_state,toaplan2)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM2151(config, "ymsnd", 27_MHz_XTAL/8).add_route(ALL_OUTPUTS, "mono", 0.5);

	OKIM6295(config, m_oki[0], 16_MHz_XTAL/4, okim6295_device::PIN7_LOW); // pin7 not confirmed
	m_oki[0]->set_addrmap(0, &toaplan2_state::enmadaio_oki);
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 0.5);
}

void toaplan2_state::snowbro2(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 32_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &toaplan2_state::snowbro2_68k_mem);
	m_maincpu->reset_cb().set(FUNC(toaplan2_state::toaplan2_reset));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	//m_screen->set_refresh_hz(60);
	//m_screen->set_size(432, 262);
	//m_screen->set_visarea(0, 319, 0, 239);
	m_screen->set_screen_update(FUNC(toaplan2_state::screen_update_toaplan2));
	m_screen->screen_vblank().set(FUNC(toaplan2_state::screen_vblank));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, T2PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp[0], 27_MHz_XTAL);
	m_vdp[0]->set_palette(m_palette);
	m_vdp[0]->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_4);

	MCFG_VIDEO_START_OVERRIDE(toaplan2_state,toaplan2)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM2151(config, "ymsnd", 27_MHz_XTAL/8).add_route(ALL_OUTPUTS, "mono", 0.5);

	OKIM6295(config, m_oki[0], 16_MHz_XTAL/4, okim6295_device::PIN7_LOW);
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 0.5);
}

void toaplan2_state::snowbro2b3(machine_config &config)
{
	snowbro2(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &toaplan2_state::snowbro2b3_68k_mem);
	m_maincpu->set_vblank_int("screen", FUNC(toaplan2_state::irq2_line_hold));

	m_vdp[0]->vint_out_cb().set_nop();
	m_vdp[0]->set_bootleg_extra_offsets(0x02e, 0x1f0, 0x02e, 0x1ee, 0x02e, 0x1ef, 0x1e9, 0x1ef);
}


void truxton2_state::mahoudai(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 32_MHz_XTAL/2);   // 16MHz, 32MHz Oscillator
	m_maincpu->set_addrmap(AS_PROGRAM, &truxton2_state::mahoudai_68k_mem);
	m_maincpu->reset_cb().set(FUNC(truxton2_state::toaplan2_reset));

	Z80(config, m_audiocpu, 32_MHz_XTAL/8);     // 4MHz, 32MHz Oscillator
	m_audiocpu->set_addrmap(AS_PROGRAM, &truxton2_state::raizing_sound_z80_mem);

	config.set_maximum_quantum(attotime::from_hz(600));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	//m_screen->set_refresh_hz(60);
	//m_screen->set_size(432, 262);
	//m_screen->set_visarea(0, 319, 0, 239);
	m_screen->set_screen_update(FUNC(truxton2_state::screen_update_truxton2));
	m_screen->screen_vblank().set(FUNC(truxton2_state::screen_vblank));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_textrom);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, T2PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp[0], 27_MHz_XTAL);
	m_vdp[0]->set_palette(m_palette);
	m_vdp[0]->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_4);

	MCFG_VIDEO_START_OVERRIDE(truxton2_state,bgaregga)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM2151(config, "ymsnd", 27_MHz_XTAL/8).add_route(ALL_OUTPUTS, "mono", 0.68);

	OKIM6295(config, m_oki[0], 32_MHz_XTAL/32, okim6295_device::PIN7_HIGH);
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 1.0);
}


void truxton2_state::shippumd(machine_config &config)
{
	mahoudai(config);
	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &truxton2_state::shippumd_68k_mem);
}

void truxton2_state::bgaregga(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 32_MHz_XTAL/2);   // 16MHz, 32MHz Oscillator
	m_maincpu->set_addrmap(AS_PROGRAM, &truxton2_state::bgaregga_68k_mem);
	m_maincpu->reset_cb().set(FUNC(truxton2_state::toaplan2_reset));

	Z80(config, m_audiocpu, 32_MHz_XTAL/8);     // 4MHz, 32MHz Oscillator
	m_audiocpu->set_addrmap(AS_PROGRAM, &truxton2_state::bgaregga_sound_z80_mem);

	config.set_maximum_quantum(attotime::from_hz(6000));

	MCFG_MACHINE_RESET_OVERRIDE(truxton2_state,bgaregga)

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	//m_screen->set_refresh_hz(60);
	//m_screen->set_size(432, 262);
	//m_screen->set_visarea(0, 319, 0, 239);
	m_screen->set_screen_update(FUNC(truxton2_state::screen_update_truxton2));
	m_screen->screen_vblank().set(FUNC(truxton2_state::screen_vblank));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_textrom);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, T2PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp[0], 27_MHz_XTAL);
	m_vdp[0]->set_palette(m_palette);
	m_vdp[0]->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_4);

	MCFG_VIDEO_START_OVERRIDE(truxton2_state,bgaregga)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch[0]);
	m_soundlatch[0]->data_pending_callback().set_inputline(m_audiocpu, 0);
	m_soundlatch[0]->set_separate_acknowledge(true);

	YM2151(config, "ymsnd", 32_MHz_XTAL/8).add_route(ALL_OUTPUTS, "mono", 0.35);

	OKIM6295(config, m_oki[0], 32_MHz_XTAL/16, okim6295_device::PIN7_HIGH);
	m_oki[0]->set_addrmap(0, &truxton2_state::raizing_oki<0>);
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 0.7);
}


void truxton2_state::bgareggabl(machine_config &config)
{
	bgaregga(config);
	MCFG_VIDEO_START_OVERRIDE(truxton2_state,bgareggabl)

	m_screen->set_screen_update(FUNC(truxton2_state::screen_update_bootleg));
}

void truxton2_state::batrider(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 32_MHz_XTAL/2);   // 16MHz, 32MHz Oscillator (verified)
	m_maincpu->set_addrmap(AS_PROGRAM, &truxton2_state::batrider_68k_mem);
	m_maincpu->reset_cb().set(FUNC(truxton2_state::toaplan2_reset));

	Z80(config, m_audiocpu, 32_MHz_XTAL/6);     // 5.333MHz, 32MHz Oscillator (verified)
	m_audiocpu->set_addrmap(AS_PROGRAM, &truxton2_state::batrider_sound_z80_mem);
	m_audiocpu->set_addrmap(AS_IO, &truxton2_state::batrider_sound_z80_port);

	config.set_maximum_quantum(attotime::from_hz(600));

	MCFG_MACHINE_RESET_OVERRIDE(truxton2_state,bgaregga)

	ADDRESS_MAP_BANK(config, m_dma_space, 0);
	m_dma_space->set_addrmap(0, &truxton2_state::batrider_dma_mem);
	m_dma_space->set_endianness(ENDIANNESS_BIG);
	m_dma_space->set_data_width(16);
	m_dma_space->set_addr_width(16);
	m_dma_space->set_stride(0x8000);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	//m_screen->set_refresh_hz(60);
	//m_screen->set_size(432, 262);
	//m_screen->set_visarea(0, 319, 0, 239);
	m_screen->set_screen_update(FUNC(truxton2_state::screen_update_truxton2));
	m_screen->screen_vblank().set(FUNC(truxton2_state::screen_vblank));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_batrider);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, T2PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp[0], 27_MHz_XTAL);
	m_vdp[0]->set_palette(m_palette);
	m_vdp[0]->set_tile_callback(FUNC(truxton2_state::batrider_bank_cb));
	m_vdp[0]->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_2);

	MCFG_VIDEO_START_OVERRIDE(truxton2_state,batrider)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	// these two latches are always written together, via a single move.l instruction
	GENERIC_LATCH_8(config, m_soundlatch[0]);
	GENERIC_LATCH_8(config, m_soundlatch[1]);
	GENERIC_LATCH_8(config, m_soundlatch[2]);
	GENERIC_LATCH_8(config, m_soundlatch[3]);

	YM2151(config, "ymsnd", 32_MHz_XTAL/8).add_route(ALL_OUTPUTS, "mono", 0.25); // 4MHz, 32MHz Oscillator (verified)

	OKIM6295(config, m_oki[0], 32_MHz_XTAL/10, okim6295_device::PIN7_HIGH);
	m_oki[0]->set_addrmap(0, &truxton2_state::raizing_oki<0>);
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 0.5);

	OKIM6295(config, m_oki[1], 32_MHz_XTAL/10, okim6295_device::PIN7_LOW);
	m_oki[1]->set_addrmap(0, &truxton2_state::raizing_oki<1>);
	m_oki[1]->add_route(ALL_OUTPUTS, "mono", 0.5);
}


void truxton2_state::bbakraid(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 32_MHz_XTAL/2);   // 16MHz, 32MHz Oscillator
	m_maincpu->set_addrmap(AS_PROGRAM, &truxton2_state::bbakraid_68k_mem);
	m_maincpu->reset_cb().set(FUNC(truxton2_state::toaplan2_reset));

	Z80(config, m_audiocpu, XTAL(32'000'000)/6);     /* 5.3333MHz , 32MHz Oscillator */
	m_audiocpu->set_addrmap(AS_PROGRAM, &truxton2_state::bbakraid_sound_z80_mem);
	m_audiocpu->set_addrmap(AS_IO, &truxton2_state::bbakraid_sound_z80_port);
	m_audiocpu->set_periodic_int(FUNC(truxton2_state::bbakraid_snd_interrupt), attotime::from_hz(XTAL(32'000'000) / 6 / 12000)); // sound CPU clock (divider unverified)

	config.set_maximum_quantum(attotime::from_hz(600));

	EEPROM_93C66_8BIT(config, m_eeprom);

	ADDRESS_MAP_BANK(config, m_dma_space, 0);
	m_dma_space->set_addrmap(0, &truxton2_state::batrider_dma_mem);
	m_dma_space->set_endianness(ENDIANNESS_BIG);
	m_dma_space->set_data_width(16);
	m_dma_space->set_addr_width(16);
	m_dma_space->set_stride(0x8000);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	//m_screen->set_refresh_hz(60);
	//m_screen->set_size(432, 262);
	//m_screen->set_visarea(0, 319, 0, 239);
	m_screen->set_screen_update(FUNC(truxton2_state::screen_update_truxton2));
	m_screen->screen_vblank().set(FUNC(truxton2_state::screen_vblank));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_batrider);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, T2PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp[0], 27_MHz_XTAL);
	m_vdp[0]->set_palette(m_palette);
	m_vdp[0]->set_tile_callback(FUNC(truxton2_state::batrider_bank_cb));
	m_vdp[0]->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_1);

	MCFG_VIDEO_START_OVERRIDE(truxton2_state,batrider)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	// these two latches are always written together, via a single move.l instruction
	GENERIC_LATCH_8(config, m_soundlatch[0]);
	GENERIC_LATCH_8(config, m_soundlatch[1]);
	GENERIC_LATCH_8(config, m_soundlatch[2]);
	GENERIC_LATCH_8(config, m_soundlatch[3]);

	YMZ280B(config, "ymz", 16.9344_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 1.0);
	// IRQ not used ???  Connected to a test pin (TP082)
}


void truxton2_state::nprobowl(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 32_MHz_XTAL / 2);   // 32MHz Oscillator, divisor not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &truxton2_state::nprobowl_68k_mem);
	m_maincpu->reset_cb().set(FUNC(truxton2_state::toaplan2_reset));

	ADDRESS_MAP_BANK(config, m_dma_space, 0);
	m_dma_space->set_addrmap(0, &truxton2_state::batrider_dma_mem);
	m_dma_space->set_endianness(ENDIANNESS_BIG);
	m_dma_space->set_data_width(16);
	m_dma_space->set_addr_width(16);
	m_dma_space->set_stride(0x8000);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	//m_screen->set_refresh_hz(60);
	//m_screen->set_size(432, 262);
	//m_screen->set_visarea(0, 319, 0, 239);
	m_screen->set_screen_update(FUNC(truxton2_state::screen_update_truxton2));
	m_screen->screen_vblank().set(FUNC(truxton2_state::screen_vblank));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_batrider);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, T2PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp[0], 27_MHz_XTAL);
	m_vdp[0]->set_palette(m_palette);
	m_vdp[0]->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_2);

	MCFG_VIDEO_START_OVERRIDE(truxton2_state, batrider)

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki[0], 32_MHz_XTAL/8, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // divisor not verified
	// TODO: banking
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

/* -------------------------- Toaplan games ------------------------- */

ROM_START( tekipaki )
	ROM_REGION( 0x040000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "tp020-1.bin", 0x000000, 0x010000, CRC(d8420bd5) SHA1(30c1ad9e053cd7e79adb42aa428ebee28e144755) )
	ROM_LOAD16_BYTE( "tp020-2.bin", 0x000001, 0x010000, CRC(7222de8e) SHA1(8352ae23efc24a2e20cc24b6d37cb8fc6b1a730c) )

	ROM_REGION( 0x8000, "audiocpu", 0 )    /* Sound HD647180 code */
	ROM_LOAD( "hd647180.020", 0x00000, 0x08000, CRC(d5157c12) SHA1(b2c6c087bb539456a9e562d0b40f05dde26cacd3) )

	ROM_REGION( 0x100000, "gp9001_0", 0 )
	ROM_LOAD( "tp020-4.bin", 0x000000, 0x080000, CRC(3ebbe41e) SHA1(cea196c5f83e1a23d5b538a0db9bbbffa7af5118) )
	ROM_LOAD( "tp020-3.bin", 0x080000, 0x080000, CRC(2d5e2201) SHA1(5846c844eedd48305c1c67dc645b6e070b3f5b98) )
ROM_END


ROM_START( tekipakit ) /* Location Test version */
	ROM_REGION( 0x040000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "e.e5", 0x000000, 0x010000, CRC(89affc73) SHA1(3930bf0c2528de28dcb0cf2cd537adb62a2172e3) ) /* hand written "E"  27C512 chip */
	ROM_LOAD16_BYTE( "o.e6", 0x000001, 0x010000, CRC(a2244558) SHA1(5291cfbea4d4d1c45d6d4bd21b3c466459a0fa17) ) /* hand written "O"  27C512 chip */

	ROM_REGION( 0x8000, "audiocpu", 0 )    /* Sound HD647180 code */
	ROM_LOAD( "hd647180.020", 0x00000, 0x08000, CRC(d5157c12) SHA1(b2c6c087bb539456a9e562d0b40f05dde26cacd3) )

	ROM_REGION( 0x100000, "gp9001_0", 0 )
	ROM_LOAD( "0-1_4.4_cb45.a16", 0x000000, 0x080000, CRC(35e14729) SHA1(8c929604953b78c6e72744a38e06a988510193a5) ) /* hand written "0-1  4/4  CB45"  27C402 chip */
	ROM_LOAD( "3-4_4.4_547d.a15", 0x080000, 0x080000, CRC(41975fcc) SHA1(f850d5a9638d41bb69f204a9cd54e2fd693b57ef) ) /* hand written "3-4  4/4  547D"  27C402 chip */
ROM_END


ROM_START( ghox ) /* Spinner with single axis (up/down) controls */
	ROM_REGION( 0x040000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "tp021-01.u10", 0x000000, 0x020000, CRC(9e56ac67) SHA1(daf241d9e55a6e60fc004ed61f787641595b1e62) )
	ROM_LOAD16_BYTE( "tp021-02.u11", 0x000001, 0x020000, CRC(15cac60f) SHA1(6efa3a50a5dfe6ef4072738d6a7d0d95dca8a675) )

	ROM_REGION( 0x10000, "audiocpu", 0 )            /* Sound HD647180 code */
	ROM_LOAD( "hd647180.021", 0x00000, 0x08000, CRC(6ab59e5b) SHA1(d814dd3a8f1ee638794e2bd422eed4247ba4a15e) )

	ROM_REGION( 0x100000, "gp9001_0", 0 )
	ROM_LOAD( "tp021-03.u36", 0x000000, 0x080000, CRC(a15d8e9d) SHA1(640a33997bdce8e84bea6a944139716379839037) )
	ROM_LOAD( "tp021-04.u37", 0x080000, 0x080000, CRC(26ed1c9a) SHA1(37da8af86ea24327444c2d4ad3dfbd936208d43d) )
ROM_END


ROM_START( ghoxj ) /* 8-way joystick for controls */
	ROM_REGION( 0x040000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "tp021-01a.u10", 0x000000, 0x020000, CRC(c11b13c8) SHA1(da7defc1d3b6ddded910ba56c31fbbdb5ed57b09) )
	ROM_LOAD16_BYTE( "tp021-02a.u11", 0x000001, 0x020000, CRC(8d426767) SHA1(1ed4a8bcbf4352257e7d58cb5c2c91eb48c2f047) )

	ROM_REGION( 0x10000, "audiocpu", 0 )            /* Sound HD647180 code */
	ROM_LOAD( "hd647180.021", 0x00000, 0x08000, CRC(6ab59e5b) SHA1(d814dd3a8f1ee638794e2bd422eed4247ba4a15e) )

	ROM_REGION( 0x100000, "gp9001_0", 0 )
	ROM_LOAD( "tp021-03.u36", 0x000000, 0x080000, CRC(a15d8e9d) SHA1(640a33997bdce8e84bea6a944139716379839037) )
	ROM_LOAD( "tp021-04.u37", 0x080000, 0x080000, CRC(26ed1c9a) SHA1(37da8af86ea24327444c2d4ad3dfbd936208d43d) )
ROM_END

ROM_START( ghoxjo ) /* older version (with fewer regions) of the 8-way joystick version */
	ROM_REGION( 0x040000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "tp021-01.ghoxsticker.u10", 0x000000, 0x020000, CRC(ad3a8817) SHA1(317267e0c00934a86bf05c5afd6c69a7944a2ed3) ) // TP021 ?01? label covered with a handwriten 'GHOX' sticker
	ROM_LOAD16_BYTE( "tp021-02.ghoxsticker.u11", 0x000001, 0x020000, CRC(2340e981) SHA1(d8e3f55e67fe6500f9e6c7eed1388dc895c5f574) ) // TP021 ?02? label covered with a handwriten 'GHOX' sticker

	ROM_REGION( 0x10000, "audiocpu", 0 )            /* Sound HD647180 code */
	ROM_LOAD( "hd647180.021", 0x00000, 0x08000, CRC(6ab59e5b) SHA1(d814dd3a8f1ee638794e2bd422eed4247ba4a15e) )

	ROM_REGION( 0x100000, "gp9001_0", 0 )
	ROM_LOAD( "tp021-03.u36", 0x000000, 0x080000, CRC(a15d8e9d) SHA1(640a33997bdce8e84bea6a944139716379839037) )
	ROM_LOAD( "tp021-04.u37", 0x080000, 0x080000, CRC(26ed1c9a) SHA1(37da8af86ea24327444c2d4ad3dfbd936208d43d) )
ROM_END


ROM_START( dogyuun )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "tp022_01.r16", 0x000000, 0x080000, CRC(79eb2429) SHA1(088c5ed0ed77557ab71f52cafe35028e3648ae1e) )

	/* Secondary CPU is a Toaplan marked chip, (TS-002-MACH  TOA PLAN) */
	/* It's a NEC V25 (PLCC94) (encrypted program uploaded by main CPU) */

	ROM_REGION( 0x200000, "gp9001_0", 0 )
	ROM_LOAD16_WORD_SWAP( "tp022_3.w92", 0x000000, 0x100000, CRC(191b595f) SHA1(89344946daa18087cc83f92027cf5da659b1c7a5) )
	ROM_LOAD16_WORD_SWAP( "tp022_4.w93", 0x100000, 0x100000, CRC(d58d29ca) SHA1(90d142fef37764ef817347a2bed77892a288a077) )

	ROM_REGION( 0x400000, "gp9001_1", 0 )
	ROM_LOAD16_WORD_SWAP( "tp022_5.w16", 0x000000, 0x200000, CRC(d4c1db45) SHA1(f5655467149ba737128c2f54c9c6cdaca6e4c35c) )
	ROM_LOAD16_WORD_SWAP( "tp022_6.w17", 0x200000, 0x200000, CRC(d48dc74f) SHA1(081b5a00a2ff2bd82b98b30aab3cb5b6ae1014d5) )

	ROM_REGION( 0x40000, "oki1", 0 )     /* ADPCM Samples */
	ROM_LOAD( "tp022_2.w30", 0x00000, 0x40000, CRC(043271b3) SHA1(c7eaa929e55dd956579b824ea9d20a1d0129a925) )
ROM_END


ROM_START( dogyuuna )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "01.u64", 0x000000, 0x080000, CRC(fe5bd7f4) SHA1(9c725466112a514c9ed0fb074422d291c175c3f4) )

	/* Secondary CPU is a Toaplan marked chip, (TS-002-MACH  TOA PLAN) */
	/* It's a NEC V25 (PLCC94) (encrypted program uploaded by main CPU) */

	ROM_REGION( 0x200000, "gp9001_0", 0 )
	ROM_LOAD16_WORD_SWAP( "tp022_3.w92", 0x000000, 0x100000, CRC(191b595f) SHA1(89344946daa18087cc83f92027cf5da659b1c7a5) )
	ROM_LOAD16_WORD_SWAP( "tp022_4.w93", 0x100000, 0x100000, CRC(d58d29ca) SHA1(90d142fef37764ef817347a2bed77892a288a077) )

	ROM_REGION( 0x400000, "gp9001_1", 0 )
	ROM_LOAD16_WORD_SWAP( "tp022_5.w16", 0x000000, 0x200000, CRC(d4c1db45) SHA1(f5655467149ba737128c2f54c9c6cdaca6e4c35c) )
	ROM_LOAD16_WORD_SWAP( "tp022_6.w17", 0x200000, 0x200000, CRC(d48dc74f) SHA1(081b5a00a2ff2bd82b98b30aab3cb5b6ae1014d5) )

	ROM_REGION( 0x40000, "oki1", 0 )     /* ADPCM Samples */
	ROM_LOAD( "tp022_2.w30", 0x00000, 0x40000, CRC(043271b3) SHA1(c7eaa929e55dd956579b824ea9d20a1d0129a925) )
ROM_END


// found on a standard TP-022-1 PCB, main CPU ROM had a MR sticker. It's a little closer to the location test than to the released versions (i.e region configuration).
ROM_START( dogyuunb )
	ROM_REGION( 0x080000, "maincpu", 0 )            // Main 68K code
	ROM_LOAD16_WORD_SWAP( "mr.u64", 0x000000, 0x080000, CRC(4dc258dc) SHA1(ac2783030a8367a20bfad282942c2aa383156291) ) // M27C4002

	// Secondary CPU is a Toaplan marked chip, (TS-002-MACH  TOA PLAN)
	// It's a NEC V25 (PLCC94) (encrypted program uploaded by main CPU)

	ROM_REGION( 0x200000, "gp9001_0", 0 )
	ROM_LOAD16_WORD_SWAP( "tp022_3.w92", 0x000000, 0x100000, CRC(191b595f) SHA1(89344946daa18087cc83f92027cf5da659b1c7a5) )
	ROM_LOAD16_WORD_SWAP( "tp022_4.w93", 0x100000, 0x100000, CRC(d58d29ca) SHA1(90d142fef37764ef817347a2bed77892a288a077) )

	ROM_REGION( 0x400000, "gp9001_1", 0 )
	ROM_LOAD16_WORD_SWAP( "tp022_5.w16", 0x000000, 0x200000, CRC(d4c1db45) SHA1(f5655467149ba737128c2f54c9c6cdaca6e4c35c) )
	ROM_LOAD16_WORD_SWAP( "tp022_6.w17", 0x200000, 0x200000, CRC(d48dc74f) SHA1(081b5a00a2ff2bd82b98b30aab3cb5b6ae1014d5) )

	ROM_REGION( 0x40000, "oki1", 0 )
	ROM_LOAD( "tp022_2.w30", 0x00000, 0x40000, CRC(043271b3) SHA1(c7eaa929e55dd956579b824ea9d20a1d0129a925) )
ROM_END


ROM_START( dogyuunt )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "sample10.9.u64.bin", 0x000000, 0x080000, CRC(585f5016) SHA1(18d57843f33a560a3bb4b6aef176f7ef795b742d) )

	/* Secondary CPU is a Toaplan marked chip, (TS-002-MACH  TOA PLAN) */
	/* It's a NEC V25 (PLCC94) (encrypted program uploaded by main CPU) */

	ROM_REGION( 0x200000, "gp9001_0", 0 )
	ROM_LOAD16_WORD_SWAP( "tp022_3.w92", 0x000000, 0x100000, CRC(191b595f) SHA1(89344946daa18087cc83f92027cf5da659b1c7a5) )
	ROM_LOAD16_WORD_SWAP( "tp022_4.w93", 0x100000, 0x100000, CRC(d58d29ca) SHA1(90d142fef37764ef817347a2bed77892a288a077) )

	ROM_REGION( 0x400000, "gp9001_1", 0 )
	ROM_LOAD16_WORD_SWAP( "tp022_5.w16", 0x000000, 0x200000, CRC(d4c1db45) SHA1(f5655467149ba737128c2f54c9c6cdaca6e4c35c) )
	ROM_LOAD16_WORD_SWAP( "tp022_6.w17", 0x200000, 0x200000, CRC(d48dc74f) SHA1(081b5a00a2ff2bd82b98b30aab3cb5b6ae1014d5) )

	ROM_REGION( 0x40000, "oki1", 0 )     /* ADPCM Samples */
	ROM_LOAD( "tp022_2.w30", 0x00000, 0x40000, CRC(043271b3) SHA1(c7eaa929e55dd956579b824ea9d20a1d0129a925) )
ROM_END


/*
This set came on a TX-022 PCB (different from the final version, TP-022).
Seems the game is always in 'debug mode' according to the test menu (dip has no effect). Still, it has no invicibility effect.
Couldn't find a read for the region settings jumpers.
Hardware differences according to the dumper:
* Two GCUs have nearly identical sections copy-pasted, one above the other.
* Toaplan HK-1000 ceramic module is used for inputs; the final implements the logic separately.
* Inputs mapped in the main CPU address space.
* No NEC V25 sound CPU; instead, there is a Z80 with its own ROM.
* Sound amp pinout is reversed; mounted to the underside as a bodge fix (this is how I received it).
* Board is marked "TX-022" instead of "TP-022". A halfway point from the development code "GX-022"
*/
ROM_START( dogyuunto )
	ROM_REGION( 0x080000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "8-25.u11", 0x000000, 0x080000, CRC(4d3c952f) SHA1(194f3065c513238921047ead8b425c3d0538b9a7) ) // real hand-written label is '8/25'

	ROM_REGION( 0x08000, "audiocpu", 0 )
	ROM_LOAD( "u25", 0x00000, 0x08000, CRC(41a34a7e) SHA1(c4f7833249436fd064c7088c9776d12dee4a7d39) ) // only had a white label

	// the GP9001 mask ROMs were not dumped for this set, but the ROM codes match so they are believed identical
	ROM_REGION( 0x200000, "gp9001_0", 0 )
	ROM_LOAD16_WORD_SWAP( "tp022_3.w92", 0x000000, 0x100000, CRC(191b595f) SHA1(89344946daa18087cc83f92027cf5da659b1c7a5) )
	ROM_LOAD16_WORD_SWAP( "tp022_4.w93", 0x100000, 0x100000, CRC(d58d29ca) SHA1(90d142fef37764ef817347a2bed77892a288a077) )

	ROM_REGION( 0x400000, "gp9001_1", 0 )
	ROM_LOAD16_WORD_SWAP( "tp022_5.w16", 0x000000, 0x200000, CRC(d4c1db45) SHA1(f5655467149ba737128c2f54c9c6cdaca6e4c35c) )
	ROM_LOAD16_WORD_SWAP( "tp022_6.w17", 0x200000, 0x200000, CRC(d48dc74f) SHA1(081b5a00a2ff2bd82b98b30aab3cb5b6ae1014d5) )

	// this may have some corruption (only 24, apparently random, bytes differ from the standard ROM), however preserve it for now until it has been verified.
	ROM_REGION( 0x40000, "oki1", 0 )
	ROM_LOAD( "2m.u29", 0x00000, 0x40000, CRC(5e7a77d8) SHA1(da6beb5e8e015965ff42fd52f5aa0c0ae5bcee4f) ) // '2M' hand-written
ROM_END


ROM_START( kbash )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "tp023_01.bin", 0x000000, 0x080000, CRC(2965f81d) SHA1(46f2df30fa92c80ba5a37f75e756424e15534784) )

	/* Secondary CPU is a Toaplan marked chip, (TS-004-Dash  TOA PLAN) */
	/* It's a NEC V25 (PLCC94) (encrypted) */

	ROM_REGION( 0x8000, "audiocpu", 0 )         /* Sound CPU code */
	ROM_LOAD( "tp023_02.bin", 0x0000, 0x8000, CRC(4cd882a1) SHA1(7199a5c384918f775f0815e09c46b2a58141814a) )

	ROM_REGION( 0x800000, "gp9001_0", 0 )
	ROM_LOAD( "tp023_3.bin", 0x000000, 0x200000, CRC(32ad508b) SHA1(e473489beaf649d3e5236770eb043327e309850c) )
	ROM_LOAD( "tp023_5.bin", 0x200000, 0x200000, CRC(b84c90eb) SHA1(17a1531d884d9a9696d1b25d65f9155f02396e0e) )
	ROM_LOAD( "tp023_4.bin", 0x400000, 0x200000, CRC(e493c077) SHA1(0edcfb70483ad07206695d9283031b85cd198a36) )
	ROM_LOAD( "tp023_6.bin", 0x600000, 0x200000, CRC(9084b50a) SHA1(03b58278619524d2f09a4b1c152d5e057e792a56) )

	ROM_REGION( 0x40000, "oki1", 0 )     /* ADPCM Samples */
	ROM_LOAD( "tp023_7.bin", 0x00000, 0x40000, CRC(3732318f) SHA1(f0768459f5ad2dee53d408a0a5ae3a314864e667) )
ROM_END

ROM_START( kbashk )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "tp023_01.u52", 0x000000, 0x080000, CRC(099aefbc) SHA1(8daa0deffe221e1bb5a8744ced18c23ad319ffd3) ) // same label as parent?

	/* Secondary CPU is a Toaplan marked chip, (TS-004-Dash  TOA PLAN) */
	/* It's a NEC V25 (PLCC94) (encrypted) */

	ROM_REGION( 0x8000, "audiocpu", 0 )         /* Sound CPU code */
	ROM_LOAD( "tp023_02.bin", 0x0000, 0x8000, CRC(4cd882a1) SHA1(7199a5c384918f775f0815e09c46b2a58141814a) )

	ROM_REGION( 0x800000, "gp9001_0", 0 )
	ROM_LOAD( "tp023_3.bin", 0x000000, 0x200000, CRC(32ad508b) SHA1(e473489beaf649d3e5236770eb043327e309850c) )
	ROM_LOAD( "tp023_5.bin", 0x200000, 0x200000, CRC(b84c90eb) SHA1(17a1531d884d9a9696d1b25d65f9155f02396e0e) )
	ROM_LOAD( "tp023_4.bin", 0x400000, 0x200000, CRC(e493c077) SHA1(0edcfb70483ad07206695d9283031b85cd198a36) )
	ROM_LOAD( "tp023_6.bin", 0x600000, 0x200000, CRC(9084b50a) SHA1(03b58278619524d2f09a4b1c152d5e057e792a56) )

	ROM_REGION( 0x40000, "oki1", 0 )     /* ADPCM Samples */
	ROM_LOAD( "tp023_7.bin", 0x00000, 0x40000, CRC(3732318f) SHA1(f0768459f5ad2dee53d408a0a5ae3a314864e667) )
ROM_END

// all labels handwritten. Given the AOU on the audio ROM, maybe a prototype destined to be showed there?
// GFX ROMs are on 4 riser boards. They are smaller but contain the same data as the final version.
// Only different ROMs are the main and audio CPU ones
ROM_START( kbashp )
	ROM_REGION( 0x080000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "2-19.u52", 0x000000, 0x080000, CRC(60dfdfec) SHA1(ca61433c8f7b1b765a699c375c946f113beeccc4) ) // actually 2/19

	/* Secondary CPU is a Toaplan marked chip, (NITRO TOA PLAN 509) */
	/* It's a NEC V25 (PLCC94) (encrypted) */

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "aou-nb-sound.u34", 0x0000, 0x8000, CRC(26ba8fb1) SHA1(4259c4704f0fea0c8befa2e60a0838280b23a507) )

	ROM_REGION( 0x800000, "gp9001_0", 0 )
	ROM_LOAD( "0.u1", 0x000000, 0x080000, CRC(1b87ffa5) SHA1(fbd5ac9e9635c1f5b1b896a3d504b827c0a99679) )
	ROM_LOAD( "2.u2", 0x080000, 0x080000, CRC(a411457e) SHA1(6b515e6524aa4fb1785d99556fefb0434368de84) )
	ROM_LOAD( "4.u3", 0x100000, 0x080000, CRC(631f770d) SHA1(0f0c11bc5549ed68d20dfc6ae51c3caec65aab88) )
	ROM_LOAD( "6.u4", 0x180000, 0x080000, CRC(5a46d262) SHA1(56d5180196b5acf76b700e627878998e88a21f3c) )
	ROM_LOAD( "8.u1", 0x200000, 0x080000, CRC(11b1c986) SHA1(05260c6cc5ab4239b52549e0dcda8853fc1fcd3a) )
	ROM_LOAD( "a.u2", 0x280000, 0x080000, CRC(4c4b47ce) SHA1(a41a27ac96bd9eb57bc4bd8b6592b70e86ad16d3) )
	ROM_LOAD( "c.u3", 0x300000, 0x080000, CRC(1ccc6a19) SHA1(d5735c2f075d81018021ec9e8642104227b67ace) )
	ROM_LOAD( "e.u4", 0x380000, 0x080000, CRC(731ad154) SHA1(78efce53000d170098b57342641299aacb7a82aa) )
	ROM_LOAD( "3.u1", 0x400000, 0x080000, CRC(7fbe0452) SHA1(c9b8c0d7126382fcdf8b5fa9a4466292954c88f7) )
	ROM_LOAD( "1.u2", 0x480000, 0x080000, CRC(6cd94e90) SHA1(9957ad69f8e80370dbf2cd863d0646241236f6b4) )
	ROM_LOAD( "5.u3", 0x500000, 0x080000, CRC(9cb4884e) SHA1(f596902b7740de4c262b4b18ac17eccca566ea77) )
	ROM_LOAD( "7.u4", 0x580000, 0x080000, CRC(53c2e0b6) SHA1(ee1128ad41ae3c68ef32d4211dd5205a9a5bb216) )
	ROM_LOAD( "g.u1", 0x600000, 0x080000, CRC(a63c795c) SHA1(30d3bb29cd73b31e233229f9304e3b87feaf01f3) )
	ROM_LOAD( "b.u2", 0x680000, 0x080000, CRC(32f8c39b) SHA1(a9029910c0b4fc3693081056a0afb9bcf9c0e699) )
	ROM_LOAD( "d.u3", 0x700000, 0x080000, CRC(40ac17d5) SHA1(140c67cf86ce545469fbe899b1f38c3a070908c9) )
	ROM_LOAD( "f.u4", 0x780000, 0x080000, CRC(2ca4eb83) SHA1(0d7c4242a82aba49cafd96ee5b051918d1b23b08) )

	ROM_REGION( 0x40000, "oki1", 0 )
	ROM_LOAD( "2m-nb-pcm.u40", 0x00000, 0x40000, CRC(3732318f) SHA1(f0768459f5ad2dee53d408a0a5ae3a314864e667) )
ROM_END

/*
Knuckle Bash 2
This is a hacked version of Knuckle Bash on bootleg/Korean/Chinese
hardware showing (C)Toaplan 1999 Licensed to Charterfield

PCB Layout
----------

|--------------------------------------------|
|UPC1241  EPROM  MECAT-S                     |
|  LM324                                     |
|        M6295  M6295                        |
| PAL   62256                      M5M51008  |
|       62256    MECAT-M           M5M51008  |
|        6116                      M5M51008  |
|J       6116         14.31818MHz  M5M51008  |
|A             68000                         |
|M                    16MHz                  |
|M                  PAL                      |
|A             PAL                           |
|        |-------|                           |
|        |ACTEL  |         PAL               |
|        |A40MX04|         PAL               |
|        |       |                           |
|   DSW1 |-------|         050917-10         |
|        |ACTEL  |                           |
|   DSW2 |A40MX04| MECAT-12                  |
|62256   |       |                           |
|62256   |-------| MECAT-34                  |
|--------------------------------------------|
Notes:
      68000 clock 16.000MHz
      M6295 clock 1.000MHz [16/16]. Sample rate (Hz) 16000000/16/132
      M5M51008 - Mitsubishi M5M51008 128k x8 SRAM (SOP32)
      62256    - 32k x8 SRAM
      6116     - 2k x8 SRAM
      VSync 60Hz
      HSync 15.68kHz
*/

ROM_START( kbash2 )
	ROM_REGION( 0x80000, "maincpu", 0 )         /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "mecat-m", 0x000000, 0x80000, CRC(bd2263c6) SHA1(eb794c0fc9c1fb4337114d48149283d42d22e4b3) )

	ROM_REGION( 0x800000, "gp9001_0", 0 )
	ROM_LOAD( "mecat-34", 0x000000, 0x400000, CRC(6be7b37e) SHA1(13160ad0712fee932bb98cc226e651895b19228a) )
	ROM_LOAD( "mecat-12", 0x400000, 0x400000, CRC(49e46b1f) SHA1(d12b12696a8473eb34f3cd247ab060289a6c0e9c) )

	ROM_REGION( 0x80000, "oki1", 0 )            /* ADPCM Music */
	ROM_LOAD( "mecat-s", 0x00000, 0x80000, CRC(3eb7adf4) SHA1(b0e6e99726b854858bd0e69eb77f12b9664b35e6) )

	ROM_REGION( 0x40000, "oki2", 0 )            /* ADPCM Samples */
	ROM_LOAD( "eprom",   0x00000, 0x40000, CRC(31115cb9) SHA1(c79ea01bd865e2fc3aaab3ff05483c8fd27e5c98) )

	ROM_REGION( 0x10000, "user1", 0 )           /* ??? Some sort of table  - same as in pipibibi*/
	ROM_LOAD( "050917-10", 0x0000, 0x10000, CRC(6b213183) SHA1(599c59d155d11edb151bfaed1d24ef964462a447) )
ROM_END


ROM_START( truxton2 )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	/* program ROM is byte swapped ! */
	ROM_LOAD16_WORD( "tp024_1.bin", 0x000000, 0x080000, CRC(f5cfe6ee) SHA1(30979888a4cd6500244117748f28386a7e20a169) )

	ROM_REGION( 0x200000, "gp9001_0", 0 )
	ROM_LOAD( "tp024_4.bin", 0x000000, 0x100000, CRC(805c449e) SHA1(fdf985344145bd320b88b9b0c25e73066c9b2ada) )
	ROM_LOAD( "tp024_3.bin", 0x100000, 0x100000, CRC(47587164) SHA1(bac493e2d5507286b984957b289c929335d27eaa) )

	ROM_REGION( 0x80000, "oki1", 0 )         /* ADPCM Samples */
	ROM_LOAD( "tp024_2.bin", 0x00000, 0x80000, CRC(f2f6cae4) SHA1(bb4e8c36531bed97ced4696ca12fd40ede2531aa) )
ROM_END


ROM_START( pipibibs )
	ROM_REGION( 0x040000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "tp025-1.bin", 0x000000, 0x020000, CRC(b2ea8659) SHA1(400431b656dbfbd5a9bc5961c3ea04c4d38b6f77) )
	ROM_LOAD16_BYTE( "tp025-2.bin", 0x000001, 0x020000, CRC(dc53b939) SHA1(e4de371f97ba7c350273ad43b7f58ff31672a269) )

	ROM_REGION( 0x10000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "tp025-5.bin", 0x0000, 0x8000, CRC(bf8ffde5) SHA1(79c09cc9a0ea979f5af5a7e5ad671ea486f5f43e) )

	ROM_REGION( 0x200000, "gp9001_0", 0 )
	ROM_LOAD( "tp025-4.bin", 0x000000, 0x100000, CRC(ab97f744) SHA1(c1620e614345dbd5c6567e4cb6f55c61b900d0ee) )
	ROM_LOAD( "tp025-3.bin", 0x100000, 0x100000, CRC(7b16101e) SHA1(ae0119bbfa0937d18c4fbb0a3ef7cdc3b9fa6b56) )
ROM_END


ROM_START( pipibibsa )
	ROM_REGION( 0x040000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "tp025-1.alt.bin", 0x000000, 0x020000, CRC(3e522d98) SHA1(043dd76b99e130909e47063d4cc773177a2eaccf) )
	ROM_LOAD16_BYTE( "tp025-2.alt.bin", 0x000001, 0x020000, CRC(48370485) SHA1(9895e086c9a5eeec4f454cbc6098adb2f66d4e11) )

	ROM_REGION( 0x10000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "tp025-5.bin", 0x0000, 0x8000, CRC(bf8ffde5) SHA1(79c09cc9a0ea979f5af5a7e5ad671ea486f5f43e) )

	ROM_REGION( 0x200000, "gp9001_0", 0 )
	ROM_LOAD( "tp025-4.bin", 0x000000, 0x100000, CRC(ab97f744) SHA1(c1620e614345dbd5c6567e4cb6f55c61b900d0ee) )
	ROM_LOAD( "tp025-3.bin", 0x100000, 0x100000, CRC(7b16101e) SHA1(ae0119bbfa0937d18c4fbb0a3ef7cdc3b9fa6b56) )
ROM_END


ROM_START( whoopee )
	ROM_REGION( 0x040000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "whoopee.1", 0x000000, 0x020000, CRC(28882e7e) SHA1(8fcd278a7d005eb81cd9e461139c0c0f756a4fa4) )
	ROM_LOAD16_BYTE( "whoopee.2", 0x000001, 0x020000, CRC(6796f133) SHA1(d4e657be260ba3fd3f0556ade617882513b52685) )

	ROM_REGION( 0x10000, "audiocpu", 0 )            /* Sound HD647180 code */
	ROM_LOAD( "hd647180.025", 0x00000, 0x08000, CRC(c02436f6) SHA1(385343f88991646ec23b385eaea82718f1251ea6) )

	ROM_REGION( 0x200000, "gp9001_0", 0 )
	ROM_LOAD( "tp025-4.bin", 0x000000, 0x100000, CRC(ab97f744) SHA1(c1620e614345dbd5c6567e4cb6f55c61b900d0ee) )
	ROM_LOAD( "tp025-3.bin", 0x100000, 0x100000, CRC(7b16101e) SHA1(ae0119bbfa0937d18c4fbb0a3ef7cdc3b9fa6b56) )
ROM_END


ROM_START( pipibibsp )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "pip_cpu_e", 0x000000, 0x020000, CRC(ae3205bd) SHA1(1613fec637dfed213433dca0d267e49f4848df81) )
	ROM_LOAD16_BYTE( "pip_cpu_o", 0x000001, 0x020000, CRC(241669a9) SHA1(234e0bb819453e16625d15d2cf22496bbc547943) )

	ROM_REGION( 0x10000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "pip_snd", 0x0000, 0x8000, CRC(8ebf183b) SHA1(602b138c85b02d121d007f6788b322aa107c7d91) )

	ROM_REGION( 0x200000, "gp9001_0", 0 )
	ROM_LOAD( "cg_01_l", 0x000000, 0x080000, CRC(21d1ef46) SHA1(d7ccbe56eb08be421c241065cbaa99cc9cca4d73) )
	ROM_LOAD( "cg_01_h", 0x080000, 0x080000, CRC(d5726328) SHA1(26401ba8ce22fda161306b91d70afefa959cde8c) )
	ROM_LOAD( "cg_23_l", 0x100000, 0x080000, CRC(114d41d0) SHA1(d1166d495d92c6082fffbed422deb7605c5a41a2) )
	ROM_LOAD( "cg_23_h", 0x180000, 0x080000, CRC(e0468152) SHA1(f5a872d8658e959ec6cce51c7798291b5b973f15) )
ROM_END


// TODO: this runs on oneshot.cpp hardware. Move to that driver and remove the hacks in video/gp9001.cpp needed to run it in this driver
ROM_START( pipibibsbl ) /* Based off the proto code. */
	ROM_REGION( 0x040000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "ppbb06.bin", 0x000000, 0x020000, CRC(14c92515) SHA1(2d7f7c89272bb2a8115f163ad651bef3bca5107e) )
	ROM_LOAD16_BYTE( "ppbb05.bin", 0x000001, 0x020000, CRC(3d51133c) SHA1(d7bd94ad11e9aeb5a5165c5ac6f71950849bcd2f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "ppbb08.bin", 0x0000, 0x8000, CRC(101c0358) SHA1(162e02d00b7bdcdd3b48a0cd0527b7428435ec50) ) // same data as komocomo in oneshot.cpp

	ROM_REGION( 0x200000, "gp9001_0", 0 )
	/* GFX data differs slightly from Toaplan boards ??? */
	ROM_LOAD16_BYTE( "ppbb01.bin", 0x000000, 0x080000, CRC(0fcae44b) SHA1(ac72bc79e3a5d0a81647c312d310d00ace017272) )
	ROM_LOAD16_BYTE( "ppbb02.bin", 0x000001, 0x080000, CRC(8bfcdf87) SHA1(4537a7d646d3014f069c6fd0be457bb32e2f18ac) )
	ROM_LOAD16_BYTE( "ppbb03.bin", 0x100000, 0x080000, CRC(abdd2b8b) SHA1(a4246dd63515f01d1227c9a9e16d9f1c739ee39e) )
	ROM_LOAD16_BYTE( "ppbb04.bin", 0x100001, 0x080000, CRC(70faa734) SHA1(4448f4dbded56c142e57293d371e0a422c3a667e) )

	ROM_REGION( 0x8000, "user1", 0 )            /* ??? Some sort of table */
	ROM_LOAD( "ppbb07.bin", 0x0000, 0x8000, CRC(456dd16e) SHA1(84779ee64d3ea33ba1ba4dee39b504a81c6811a1) ) // 1xxxxxxxxxxxxxx = 0xFF, same data as komocomo in oneshot.cpp
ROM_END


// TODO: determine if this is the correct driver or if this needs to be moved somewhere else, too
ROM_START( pipibibsbl2 ) // PIPI001 PCB
	ROM_REGION( 0x040000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "06.bin", 0x000000, 0x020000, CRC(25f49c2f) SHA1(a61246ec8a07ba14ee0a01c3458c59840b435c0b) )
	ROM_LOAD16_BYTE( "07.bin", 0x000001, 0x020000, CRC(15250177) SHA1(a5ee5ccc219f300d7387b45dc8f8b72fd0f37d7e) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "08.bin", 0x00000, 0x10000, CRC(f2080071) SHA1(68cbae9559879b2dc19c41a7efbd13ab4a569d3f) ) //  // 1ST AND 2ND HALF IDENTICAL, same as komocomo in oneshot.cpp

	ROM_REGION( 0x200000, "gp9001_0", 0 )
	ROM_LOAD16_BYTE( "01.bin", 0x000000, 0x80000, CRC(505e9e9f) SHA1(998995d94585d785263cc926f68632065aa6c366) )
	ROM_LOAD16_BYTE( "02.bin", 0x000001, 0x80000, CRC(860018f5) SHA1(7f42dffb27940629447d688e1771b4ecf04f3b43) )
	ROM_LOAD16_BYTE( "03.bin", 0x100000, 0x80000, CRC(ece1bc0f) SHA1(d29f1520f1a3a9d276d36af650bc0d70bcb5b8da) )
	ROM_LOAD16_BYTE( "04.bin", 0x100001, 0x80000, CRC(f328d7a3) SHA1(2c4fb5d6202f847aaf7c7be719c0c92b8bb5946b) )

	ROM_REGION( 0x20000, "user1", 0 )
	ROM_LOAD( "5.bin", 0x00000, 0x20000, CRC(8107c4bd) SHA1(64e2fafa808c16c722454b611a8492a4620a925c) ) // motherboard ROM, unknown purpose
ROM_END

ROM_START( pipibibsbl3 )
	ROM_REGION( 0x040000, "maincpu", 0 )            // Main 68K code, not scrambled
	ROM_LOAD16_BYTE( "5.bin", 0x000000, 0x020000, CRC(7fab770c) SHA1(c96808870c5906e0203f38114702bd660e491a7d) )
	ROM_LOAD16_BYTE( "6.bin", 0x000001, 0x020000, CRC(9007ef00) SHA1(594052be7351e0b8e30f83abd9a91ab1429d82ef) )

	ROM_REGION( 0x10000, "audiocpu", 0 )            // Sound Z80 code
	ROM_LOAD( "7.bin", 0x0000, 0x8000, CRC(101c0358) SHA1(162e02d00b7bdcdd3b48a0cd0527b7428435ec50) ) // same data as komocomo in oneshot.cpp

	ROM_REGION( 0x200000, "gp9001_0", 0 )
	// GFX data differs slightly from Toaplan boards ???
	ROM_LOAD16_BYTE( "4.bin", 0x000000, 0x080000, CRC(0fcae44b) SHA1(ac72bc79e3a5d0a81647c312d310d00ace017272) )
	ROM_LOAD16_BYTE( "3.bin", 0x000001, 0x080000, CRC(8bfcdf87) SHA1(4537a7d646d3014f069c6fd0be457bb32e2f18ac) )
	ROM_LOAD16_BYTE( "2.bin", 0x100000, 0x080000, CRC(abdd2b8b) SHA1(a4246dd63515f01d1227c9a9e16d9f1c739ee39e) )
	ROM_LOAD16_BYTE( "1.bin", 0x100001, 0x080000, CRC(70faa734) SHA1(4448f4dbded56c142e57293d371e0a422c3a667e) )

	ROM_REGION( 0x8000, "user1", 0 )            // ??? Some sort of table
	ROM_LOAD( "8.bin", 0x0000, 0x8000, CRC(456dd16e) SHA1(84779ee64d3ea33ba1ba4dee39b504a81c6811a1) ) // 1xxxxxxxxxxxxxx = 0xFF, same data as komocomo in oneshot.cpp
ROM_END

#define ROMS_FIXEIGHT \
	ROM_REGION( 0x080000, "maincpu", 0 ) \
	ROM_LOAD16_WORD_SWAP( "tp-026-1", 0x000000, 0x080000, CRC(f7b1746a) SHA1(0bbea6f111b818bc9b9b2060af4fe900f37cf7f9) ) \
	ROM_REGION( 0x400000, "gp9001_0", 0 ) \
	ROM_LOAD( "tp-026-3", 0x000000, 0x200000, CRC(e5578d98) SHA1(280d2b716d955e767d311fc9596823852435b6d7) ) \
	ROM_LOAD( "tp-026-4", 0x200000, 0x200000, CRC(b760cb53) SHA1(bc9c5e49e45cdda0f774be0038aa4deb21d4d285) ) \
	ROM_REGION( 0x40000, "oki1", 0 ) \
	ROM_LOAD( "tp-026-2", 0x00000, 0x40000, CRC(85063f1f) SHA1(1bf4d77494de421c98f6273b9876e60d827a6826) )

// note you may need to byteswap these EEPROM files to reprogram the original chip, this is the same for many supported in MAME.

ROM_START( fixeightkt )
	ROMS_FIXEIGHT
	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "fixeightkt.nv", 0x00, 0x80, CRC(08fa73ba) SHA1(b7761d3dd3f4485e55c8ef2cf1a840ca771ee2fc) )
ROM_END

ROM_START( fixeightk )
	ROMS_FIXEIGHT
	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "fixeightk.nv", 0x00, 0x80, CRC(cac91c6f) SHA1(55b284f081753d60abff63493094322756b7f0c5) )
ROM_END

ROM_START( fixeightht )
	ROMS_FIXEIGHT
	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "fixeightht.nv", 0x00, 0x80, CRC(57edaa51) SHA1(b8d50e82590b8cbbbcafec5f9cfbc91e4c286db5) )
ROM_END

ROM_START( fixeighth )
	ROMS_FIXEIGHT
	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "fixeighth.nv", 0x00, 0x80, CRC(95dec584) SHA1(1c309074b51da5a5263dee00403296946e41067b) )
ROM_END

ROM_START( fixeighttwt )
	ROMS_FIXEIGHT
	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "fixeighttwt.nv", 0x00, 0x80, CRC(b6d5c06c) SHA1(7fda380ac6835a983c57d093ccad7bd76893c9ba))
ROM_END

ROM_START( fixeighttw )
	ROMS_FIXEIGHT
	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "fixeighttw.nv", 0x00, 0x80, CRC(74e6afb9) SHA1(87bdc95eb0d2d54375de2c622557d503e14154be))
ROM_END

ROM_START( fixeightat )
	ROMS_FIXEIGHT
	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "fixeightat.nv", 0x00, 0x80,CRC(e9c21987) SHA1(7f699e38deb84902ed62b857a3d2b4e3ea1475bb) )
ROM_END

ROM_START( fixeighta )
	ROMS_FIXEIGHT
	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "fixeighta.nv", 0x00, 0x80, CRC(2bf17652) SHA1(4ec6f188e63610d258cd6b2432d2200d61d80bed))
ROM_END

ROM_START( fixeightt )
	ROMS_FIXEIGHT
	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "fixeightt.nv", 0x00, 0x80, CRC(c0da4a05) SHA1(3686161244e3e8be0e2fdb5fc5c24e39a7aeba85) )
ROM_END

ROM_START( fixeight )
	ROMS_FIXEIGHT
	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "fixeight.nv", 0x00, 0x80, CRC(02e925d0) SHA1(5839d10aceff84916ea99e9c6afcdc90eef7468b) )
ROM_END

ROM_START( fixeightut )
	ROMS_FIXEIGHT
	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "fixeightut.nv", 0x00, 0x80, CRC(9fcd93ee) SHA1(4f2750f09d9b8ff358a2fd6c7a4a8ba6de67017a) )
ROM_END

ROM_START( fixeightu )
	ROMS_FIXEIGHT
	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "fixeightu.nv", 0x00, 0x80, CRC(5dfefc3b) SHA1(5203525c58e2ae10575af2e277a5696bd64c5b60) )
ROM_END

ROM_START( fixeightj )
	ROMS_FIXEIGHT
	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "fixeightj.nv", 0x00, 0x80, CRC(21e22038) SHA1(29fb10061e62799bb5e4171e144daac49f0cdf06) )
ROM_END

ROM_START( fixeightjt )
	ROMS_FIXEIGHT
	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "fixeightjt.nv", 0x00, 0x80, CRC(e3d14fed) SHA1(ee4982ef195240c5eaa5005ca1d591901fb01b47) )
ROM_END


/*
Fix Eight (bootleg)
Toaplan, 1992

PCB Layout
----------

|--------------------------------------------|
|   1.BIN        PAL               14MHz  PAL|
|   M6295        PAL                         |
|   PAL     6116 4.BIN          681000 681000|
|           6116                             |
|           6116                681000 681000|
|J          6116        PAL                  |
|A                             PAL           |
|M                                           |
|M   62256  62256              PAL           |
|A   2.BIN  3.BIN       PAL                  |
|                       PAL                  |
|       68000           PAL                  |
| DSW2        |------|  5.BIN                |
| DSW1   6264 |TPC   |                       |
| 3.579545MHz |1020  |  6.BIN                |
| 10MHz  6264 |------|  7.BIN                |
|--------------------------------------------|
Notes:
      68000 clock at 10.000MHz
      M6295 clock at 875kHz [14M/16]. Sample rate = 875000 / 165
      VSync at 60Hz
      6116  - 2k   x8 SRAM (x4)
      6264  - 8k   x8 SRAM (x2)
      62256 - 32k  x8 SRAM (x2)
      681000- 128k x8 SRAM (x4)
*/


ROM_START( fixeightbl )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "3.bin", 0x000000, 0x80000, CRC(cc77d4b4) SHA1(4d3376cbae13d90c6314d8bb9236c2183fc6253c) )
	ROM_LOAD16_BYTE( "2.bin", 0x000001, 0x80000, CRC(ed715488) SHA1(37be9bc8ff6b54a1f660d89469c6c2da6301e9cd) )

	ROM_REGION( 0x400000, "gp9001_0", 0 )
	ROM_LOAD( "tp-026-3", 0x000000, 0x200000, CRC(e5578d98) SHA1(280d2b716d955e767d311fc9596823852435b6d7) )
	ROM_LOAD( "tp-026-4", 0x200000, 0x200000, CRC(b760cb53) SHA1(bc9c5e49e45cdda0f774be0038aa4deb21d4d285) )

	ROM_REGION( 0x08000, "text", 0)
	ROM_LOAD( "4.bin", 0x00000, 0x08000, CRC(a6aca465) SHA1(2b331faeee1832e0adc5218254a99d66331862c6) )

	ROM_REGION( 0x80000, "oki1", 0 )         /* ADPCM Samples */
	ROM_LOAD( "1.bin", 0x00000, 0x80000, CRC(888f19ac) SHA1(d2f4f8b7be7a0fdb95baa0af8930e50e2f875c05) )

	ROM_REGION( 0x8000, "user1", 0 )            /* ??? Some sort of table  - same as in pipibibsbl */
	ROM_LOAD( "5.bin", 0x0000, 0x8000, CRC(456dd16e) SHA1(84779ee64d3ea33ba1ba4dee39b504a81c6811a1) )
ROM_END


ROM_START( grindstm )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "01.bin", 0x000000, 0x080000, CRC(4923f790) SHA1(1c2d66b432d190d0fb6ac7ca0ec0687aea3ccbf4) )

	/* Secondary CPU is a Toaplan marked chip, (TS-007-Spy  TOA PLAN) */
	/* It's a NEC V25 (PLCC94) (encrypted program uploaded by main CPU) */

	ROM_REGION( 0x200000, "gp9001_0", 0 )
	ROM_LOAD( "tp027_02.bin", 0x000000, 0x100000, CRC(877b45e8) SHA1(b3ed8d8dbbe51a1919afc55d619d2b6771971493) )
	ROM_LOAD( "tp027_03.bin", 0x100000, 0x100000, CRC(b1fc6362) SHA1(5e97e3cce31be57689d394a50178cda4d80cce5f) )
ROM_END


ROM_START( grindstma )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "tp027-01.rom", 0x000000, 0x080000, CRC(8d8c0392) SHA1(824dde274c8bef8a87c54d8ccdda7f0feb8d11e1) )

	/* Secondary CPU is a Toaplan marked chip, (TS-007-Spy  TOA PLAN) */
	/* It's a NEC V25 (PLCC94) (encrypted program uploaded by main CPU) */

	ROM_REGION( 0x200000, "gp9001_0", 0 )
	ROM_LOAD( "tp027_02.bin", 0x000000, 0x100000, CRC(877b45e8) SHA1(b3ed8d8dbbe51a1919afc55d619d2b6771971493) )
	ROM_LOAD( "tp027_03.bin", 0x100000, 0x100000, CRC(b1fc6362) SHA1(5e97e3cce31be57689d394a50178cda4d80cce5f) )
ROM_END


ROM_START( vfive )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "tp027_01.bin", 0x000000, 0x080000, CRC(731d50f4) SHA1(794255d0a809cda9170f5bac473df9d7f0efdac8) )

	/* Secondary CPU is a Toaplan marked chip, (TS-007-Spy  TOA PLAN) */
	/* It's a NEC V25 (PLCC94) (encrypted program uploaded by main CPU) */

	ROM_REGION( 0x200000, "gp9001_0", 0 )
	ROM_LOAD( "tp027_02.bin", 0x000000, 0x100000, CRC(877b45e8) SHA1(b3ed8d8dbbe51a1919afc55d619d2b6771971493) )
	ROM_LOAD( "tp027_03.bin", 0x100000, 0x100000, CRC(b1fc6362) SHA1(5e97e3cce31be57689d394a50178cda4d80cce5f) )
ROM_END


ROM_START( batsugun )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "tp030_1a.bin", 0x000000, 0x080000,  CRC(cb1d4554) SHA1(ef31f24d77e1c13bdf5558a04a6253e2e3e6a790) )

	/* Secondary CPU is a Toaplan marked chip, (TS-007-Spy  TOA PLAN) */
	/* It's a NEC V25 (PLCC94) (program uploaded by main CPU) */

	ROM_REGION( 0x400000, "gp9001_0", 0 )
	ROM_LOAD( "tp030_3l.bin", 0x000000, 0x100000, CRC(3024b793) SHA1(e161db940f069279356fca2c5bf2753f07773705) )
	ROM_LOAD( "tp030_3h.bin", 0x100000, 0x100000, CRC(ed75730b) SHA1(341f0f728144a049486d996c9bb14078578c6879) )
	ROM_LOAD( "tp030_4l.bin", 0x200000, 0x100000, CRC(fedb9861) SHA1(4b0917056bd359b21935358c6bcc729262be6417) )
	ROM_LOAD( "tp030_4h.bin", 0x300000, 0x100000, CRC(d482948b) SHA1(31be7dc5cff072403b783bf203b9805ffcad7284) )

	ROM_REGION( 0x200000, "gp9001_1", 0 )
	ROM_LOAD( "tp030_5.bin",  0x000000, 0x100000, CRC(bcf5ba05) SHA1(40f98888a29cdd30cda5dfb60fdc667c69b0fdb0) )
	ROM_LOAD( "tp030_6.bin",  0x100000, 0x100000, CRC(0666fecd) SHA1(aa8f921fc51590b5b05bbe0b0ad0cce5ff359c64) )

	ROM_REGION( 0x40000, "oki1", 0 )         /* ADPCM Samples */
	ROM_LOAD( "tp030_2.bin", 0x00000, 0x40000, CRC(276146f5) SHA1(bf11d1f6782cefcad77d52af4f7e6054a8f93440) )

	ROM_REGION( 0x1000, "plds", 0 )         /* Logic for mixing output of both GP9001 GFX controllers */
	ROM_LOAD( "tp030_u19_gal16v8b-15.bin", 0x0000, 0x117, CRC(f71669e8) SHA1(ec1fbe04605fee864af4b01f001af227938c9f21) )
//  ROM_LOAD( "tp030_u19_gal16v8b-15.jed", 0x0000, 0x991, CRC(31be54a2) SHA1(06278942a9a2ea858c0352b2ef5a65bf329b7b82) )
ROM_END

ROM_START( batsuguna )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "tp030_01.bin", 0x000000, 0x080000, CRC(3873d7dd) SHA1(baf6187d7d554cfcf4a86b63f07fc30df7ef84c9) )

	/* Secondary CPU is a Toaplan marked chip, (TS-007-Spy  TOA PLAN) */
	/* It's a NEC V25 (PLCC94) (program uploaded by main CPU) */

	ROM_REGION( 0x400000, "gp9001_0", 0 )
	ROM_LOAD( "tp030_3l.bin", 0x000000, 0x100000, CRC(3024b793) SHA1(e161db940f069279356fca2c5bf2753f07773705) )
	ROM_LOAD( "tp030_3h.bin", 0x100000, 0x100000, CRC(ed75730b) SHA1(341f0f728144a049486d996c9bb14078578c6879) )
	ROM_LOAD( "tp030_4l.bin", 0x200000, 0x100000, CRC(fedb9861) SHA1(4b0917056bd359b21935358c6bcc729262be6417) )
	ROM_LOAD( "tp030_4h.bin", 0x300000, 0x100000, CRC(d482948b) SHA1(31be7dc5cff072403b783bf203b9805ffcad7284) )

	ROM_REGION( 0x200000, "gp9001_1", 0 )
	ROM_LOAD( "tp030_5.bin",  0x000000, 0x100000, CRC(bcf5ba05) SHA1(40f98888a29cdd30cda5dfb60fdc667c69b0fdb0) )
	ROM_LOAD( "tp030_6.bin",  0x100000, 0x100000, CRC(0666fecd) SHA1(aa8f921fc51590b5b05bbe0b0ad0cce5ff359c64) )

	ROM_REGION( 0x40000, "oki1", 0 )         /* ADPCM Samples */
	ROM_LOAD( "tp030_2.bin", 0x00000, 0x40000, CRC(276146f5) SHA1(bf11d1f6782cefcad77d52af4f7e6054a8f93440) )

	ROM_REGION( 0x1000, "plds", 0 )         /* Logic for mixing output of both GP9001 GFX controllers */
	ROM_LOAD( "tp030_u19_gal16v8b-15.bin", 0x0000, 0x117, CRC(f71669e8) SHA1(ec1fbe04605fee864af4b01f001af227938c9f21) )
ROM_END

ROM_START( batsugunb )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "large_rom1.bin", 0x000000, 0x080000,  CRC(c9de8ed8) SHA1(8de9acd26e83c8ea3388137da528704116aa7bdb) )

	/* Secondary CPU is a Toaplan marked chip, (TS-007-Spy  TOA PLAN) */
	/* It's a NEC V25 (PLCC94) (program uploaded by main CPU) */

	ROM_REGION( 0x400000, "gp9001_0", 0 )
	ROM_LOAD16_BYTE( "rom12.bin", 0x000000, 0x080000, CRC(d25affc6) SHA1(00803ae5a2bc06edbfb9ea6e3df51f195bbee8cb) )
	ROM_LOAD16_BYTE( "rom6.bin",  0x000001, 0x080000, CRC(ddd6df60) SHA1(3b46945c51e7b10b473d98916f075e8def336ce7) )
	ROM_LOAD16_BYTE( "rom11.bin", 0x100000, 0x080000, CRC(ed72fe3e) SHA1(5c0f4d5cc84b45e1924dacfa4c0b602cc1600b2f) )
	ROM_LOAD16_BYTE( "rom5.bin",  0x100001, 0x080000, CRC(fd44b33b) SHA1(791cf6056a2dbafa5f41f1dcf686947ee990647d) )
	ROM_LOAD16_BYTE( "rom10.bin", 0x200000, 0x080000, CRC(86b2c6a9) SHA1(b3f39246012c6cd9df69a6797d56479523b33bcb) )
	ROM_LOAD16_BYTE( "rom4.bin",  0x200001, 0x080000, CRC(e7c1c623) SHA1(0d8922ce901b5f74f1bd397d5d9c6ab4e918b1d1) )
	ROM_LOAD16_BYTE( "rom9.bin",  0x300000, 0x080000, CRC(fda8ee00) SHA1(d5ea617a72b2721386eb2dfc15b76de2e30f069c) )
	ROM_LOAD16_BYTE( "rom3.bin",  0x300001, 0x080000, CRC(a7c4dee8) SHA1(94e2dda067612fac810157f8cf392b685b38798b) )

	ROM_REGION( 0x200000, "gp9001_1", 0 )
	ROM_LOAD16_BYTE( "rom8.bin",  0x000000, 0x080000, CRC(a2c6a170) SHA1(154048ddc8ca2b4e9617e142d904ad2698b0ad02) )
	ROM_LOAD16_BYTE( "rom2.bin",  0x000001, 0x080000, CRC(a457e202) SHA1(4a9f2f95c866fc9d40af1c57ce1940f0a6dc1b82) )
	ROM_LOAD16_BYTE( "rom7.bin",  0x100000, 0x080000, CRC(8644518f) SHA1(570141deeb796cfae57600d5a518d34bb6dc14d0) )
	ROM_LOAD16_BYTE( "rom1.bin",  0x100001, 0x080000, CRC(8e339897) SHA1(80e84c291f287c0783bddfcb1b7ebf78c154cadc) )

	ROM_REGION( 0x40000, "oki1", 0 )         /* ADPCM Samples */
	ROM_LOAD( "rom13.bin", 0x00000, 0x40000, CRC(276146f5) SHA1(bf11d1f6782cefcad77d52af4f7e6054a8f93440) )

	ROM_REGION( 0x1000, "plds", 0 )         /* Logic for mixing output of both GP9001 GFX controllers */
	ROM_LOAD( "tp030_u19_gal16v8b-15.bin", 0x0000, 0x117, CRC(f71669e8) SHA1(ec1fbe04605fee864af4b01f001af227938c9f21) )
//  ROM_LOAD( "tp030_u19_gal16v8b-15.jed", 0x0000, 0x991, CRC(31be54a2) SHA1(06278942a9a2ea858c0352b2ef5a65bf329b7b82) )
ROM_END

// very similar to batsuguna, same main CPU label, seems to have just a tiny bit more code
ROM_START( batsugunc )
	ROM_REGION( 0x080000, "maincpu", 0 )            // Main 68K code
	ROM_LOAD16_WORD_SWAP( "tp-030_01.u69", 0x000000, 0x080000, CRC(545305c4) SHA1(9411ad7fe0be89a9f04b9116c9c709dc5e98c345) )

	/* Secondary CPU is a Toaplan marked chip, (TS-007-Spy  TOA PLAN) */
	/* It's a NEC V25 (PLCC94) (program uploaded by main CPU) */

	ROM_REGION( 0x400000, "gp9001_0", 0 )
	ROM_LOAD( "tp030_rom3-l.u55", 0x000000, 0x100000, CRC(3024b793) SHA1(e161db940f069279356fca2c5bf2753f07773705) )
	ROM_LOAD( "tp030_rom3-h.u56", 0x100000, 0x100000, CRC(ed75730b) SHA1(341f0f728144a049486d996c9bb14078578c6879) )
	ROM_LOAD( "tp030_rom4-l.u54", 0x200000, 0x100000, CRC(fedb9861) SHA1(4b0917056bd359b21935358c6bcc729262be6417) )
	ROM_LOAD( "tp030_rom4-h.u57", 0x300000, 0x100000, CRC(d482948b) SHA1(31be7dc5cff072403b783bf203b9805ffcad7284) )

	ROM_REGION( 0x200000, "gp9001_1", 0 )
	ROM_LOAD( "tp030_rom5.u32",  0x000000, 0x100000, CRC(bcf5ba05) SHA1(40f98888a29cdd30cda5dfb60fdc667c69b0fdb0) )
	ROM_LOAD( "tp030_rom6.u31",  0x100000, 0x100000, CRC(0666fecd) SHA1(aa8f921fc51590b5b05bbe0b0ad0cce5ff359c64) )

	ROM_REGION( 0x40000, "oki1", 0 )
	ROM_LOAD( "tp030_rom2.u65", 0x00000, 0x40000, CRC(276146f5) SHA1(bf11d1f6782cefcad77d52af4f7e6054a8f93440) )

	ROM_REGION( 0x1000, "plds", 0 )         // Logic for mixing output of both GP9001 GFX controllers
	ROM_LOAD( "tp030_u19_gal16v8b-15.bin", 0x0000, 0x117, CRC(f71669e8) SHA1(ec1fbe04605fee864af4b01f001af227938c9f21) )
//  ROM_LOAD( "tp030_u19_gal16v8b-15.jed", 0x0000, 0x991, CRC(31be54a2) SHA1(06278942a9a2ea858c0352b2ef5a65bf329b7b82) )
ROM_END

ROM_START( batsugunsp )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "tp030-sp.u69", 0x000000, 0x080000, CRC(8072a0cd) SHA1(3a0a9cdf894926a16800c4882a2b00383d981367) )

	/* Secondary CPU is a Toaplan marked chip, (TS-007-Spy  TOA PLAN) */
	/* It's a NEC V25 (PLCC94) (program uploaded by main CPU) */

	ROM_REGION( 0x400000, "gp9001_0", 0 )
	ROM_LOAD( "tp030_3l.bin", 0x000000, 0x100000, CRC(3024b793) SHA1(e161db940f069279356fca2c5bf2753f07773705) )
	ROM_LOAD( "tp030_3h.bin", 0x100000, 0x100000, CRC(ed75730b) SHA1(341f0f728144a049486d996c9bb14078578c6879) )
	ROM_LOAD( "tp030_4l.bin", 0x200000, 0x100000, CRC(fedb9861) SHA1(4b0917056bd359b21935358c6bcc729262be6417) )
	ROM_LOAD( "tp030_4h.bin", 0x300000, 0x100000, CRC(d482948b) SHA1(31be7dc5cff072403b783bf203b9805ffcad7284) )

	ROM_REGION( 0x200000, "gp9001_1", 0 )
	ROM_LOAD( "tp030_5.bin",  0x000000, 0x100000, CRC(bcf5ba05) SHA1(40f98888a29cdd30cda5dfb60fdc667c69b0fdb0) )
	ROM_LOAD( "tp030_6.bin",  0x100000, 0x100000, CRC(0666fecd) SHA1(aa8f921fc51590b5b05bbe0b0ad0cce5ff359c64) )

	ROM_REGION( 0x40000, "oki1", 0 )         /* ADPCM Samples */
	ROM_LOAD( "tp030_2.bin", 0x00000, 0x40000, CRC(276146f5) SHA1(bf11d1f6782cefcad77d52af4f7e6054a8f93440) )

	ROM_REGION( 0x1000, "plds", 0 )         /* Logic for mixing output of both GP9001 GFX controllers */
	ROM_LOAD( "tp030_u19_gal16v8b-15.bin", 0x0000, 0x117, CRC(f71669e8) SHA1(ec1fbe04605fee864af4b01f001af227938c9f21) )
ROM_END

// a cost-cutting bootleg PCB with only M68000 + OKIM6295. A pair of TPC1020 seem to do the job of the GP9001s.
// according to the dumper 'audio is pretty garbage, and some sprites overlay the UI incorrectly'
ROM_START( batsugunbl )
	ROM_REGION( 0x080000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "27c240.bin", 0x000000, 0x080000, CRC(a34df8bb) SHA1(10d456f5437b21a95fd8018bdb19a08a110241c4) )

	ROM_REGION( 0x400000, "gp9001_0", 0 ) // same as original
	ROM_LOAD( "27c8100-j.bin", 0x000000, 0x100000, CRC(3024b793) SHA1(e161db940f069279356fca2c5bf2753f07773705) )
	ROM_LOAD( "27c8100-k.bin", 0x100000, 0x100000, CRC(ed75730b) SHA1(341f0f728144a049486d996c9bb14078578c6879) )
	ROM_LOAD( "27c8100-l.bin", 0x200000, 0x100000, CRC(fedb9861) SHA1(4b0917056bd359b21935358c6bcc729262be6417) )
	ROM_LOAD( "27c8100-m.bin", 0x300000, 0x100000, CRC(d482948b) SHA1(31be7dc5cff072403b783bf203b9805ffcad7284) )

	ROM_REGION( 0x200000, "gp9001_1", 0 ) // same as original
	ROM_LOAD( "27c8100-n.bin",  0x000000, 0x100000, CRC(bcf5ba05) SHA1(40f98888a29cdd30cda5dfb60fdc667c69b0fdb0) )
	ROM_LOAD( "27c8100-o.bin",  0x100000, 0x100000, CRC(0666fecd) SHA1(aa8f921fc51590b5b05bbe0b0ad0cce5ff359c64) )

	ROM_REGION( 0x80000, "oki1", 0 ) // more samples to compensate for missing YM2151
	ROM_LOAD( "27c040.bin", 0x00000, 0x80000, CRC(1f8ec1b6) SHA1(28107a90d29613ceddc001df2556543b33c1294c) )
ROM_END

ROM_START( pwrkick ) // Sunwise SW931201-1 PCB - 8-liner connections
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "1.u61", 0x000000, 0x080000, CRC(118b5899) SHA1(7a1637a63eb17e3892d79aede5730013a1dc00f9) )

	ROM_REGION( 0x100000, "gp9001_0", ROMREGION_ERASE00 )
	ROM_LOAD( "2.u26", 0x000000, 0x080000, CRC(a190eaea) SHA1(2c7b8c8026873e0f591fbcbc2e72b196ef84e162) )
	ROM_LOAD( "3.u27", 0x080000, 0x080000, CRC(0b81e038) SHA1(8376617ae519a8ef604f20b26e941aa5b8066602) )

	ROM_REGION( 0x80000, "oki1", ROMREGION_ERASE00 )
	ROM_LOAD( "4.u33", 0x000000, 0x080000, CRC(3ab742f1) SHA1(ce8ca02ca57fd77872e421ce601afd017d3518a0) )
ROM_END

/*

Burger Kids, 1995 Sunwise

SW931201-1
+--||||||-----------------------------------------+
|         YM3014*  YM2151*                        |
| TDA1519A         M6295   FFK4.U33          BT1  |
|                              U31*               |
|                      16.000MHz 32.768kHz D4992C |
|                                      MB3771     |
|8                                                |
|-                                     U67*       |
|L                                 FFK1.U61 62256 |
|I                                     U68* 62256 |
|N                     GAL16V8B    TMP68HC000P-12 |
|E                                                |
|R                                                |
|                        1635  27.000MHz FFK3.U27 |
|             GAL16V8B   1635  20.000MHz     U12* |
| SW1         GAL16V8B    +---------+    FFK2.U26 |
| SW2                     | L7A0498 |        U13* |
| SW3                     | GP9001  |   4256 4256 |
|     6216                +---------+   4256 4256 |
|     6216                              4256 4256 |
+-------------------------------------------------+

NOTE: This PCB uses an 8-Liner style edge connector

* Denotes unpopulated components

   CPU: Toshiba TMP68HC000P-12
 Sound: OKI M6295
 Video: L7A0498 GP9001 QFP208
   OSC: 27MHz, 20MHz, 16MHz & 32.768kHz
   RAM: MB81C4256A-70P - 256K x 4-bit DRAM
        HM62256BLP-9 - 32K x 8-bit SRAM
        IMS1635P-25 - 8K x 8-bit SRAM
        XRM6216-10 - 2K x 8-bit SRAM
 Other: 8 Position Dipswitch x 3
        NEC D4992 CMOS 8-Bit Parallel I/O Calendar Clock
        MB3771 Voltage monitor
        BT1 - CR2550 3Volt battery

NOTE: Sunwise's S951060-VGP PCB uses identical componenets to the SW931201 but has a standard JAMMA connector
*/

ROM_START( burgkids ) // Sunwise SW931201-1 PCB - 8-liner connections
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "ffk1.u61", 0x000000, 0x080000, CRC(ac96cb0d) SHA1(2ce5c06d61f3ff18b222619e41d09e46d44c5bab) )

	ROM_REGION( 0x100000, "gp9001_0", ROMREGION_ERASE00 )
	ROM_LOAD( "ffk2.u26", 0x000000, 0x080000, CRC(09f7b0ae) SHA1(f340f27a601ff89f143398263d822b8f340eea6e) )
	ROM_LOAD( "ffk3.u27", 0x080000, 0x080000, CRC(63c761bc) SHA1(f0ee1dc6aaeacff23e55d072102b814c7ef30550) )

	ROM_REGION( 0x80000, "oki1", ROMREGION_ERASE00 )
	ROM_LOAD( "ffk4.u33", 0x000000, 0x080000,  CRC(3b032d4f) SHA1(69056bf205aadf6c9fee56ce396b11a5187caa03) )
ROM_END

ROM_START( othldrby ) // Sunwise S951060-VGP PCB - JAMMA compliant (components identical to Sunwise SW931201-1 PCB)
	ROM_REGION( 0x080000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "sunwise_db0_1.u61", 0x00000, 0x80000, CRC(6b4008d3) SHA1(4cf838c47563ba482be8364b2e115569a4a06c83) )

	ROM_REGION( 0x400000, "gp9001_0", 0 )
	ROM_LOAD( "db0-r2.u26", 0x000000, 0x200000, CRC(4efff265) SHA1(4cd239ff42f532495946cb52bd1fee412f84e192) ) // mask ROMs
	ROM_LOAD( "db0-r3.u27", 0x200000, 0x200000, CRC(5c142b38) SHA1(5466a8b061a0f2545493de0f96fd4387beea276a) )

	ROM_REGION( 0x080000, "oki1", 0 )    /* OKIM6295 samples */
	ROM_LOAD( "sunwise_db0_4.u33", 0x00000, 0x80000, CRC(a9701868) SHA1(9ee89556666d358e8d3915622573b3ba660048b8) )
ROM_END

ROM_START( enmadaio )
	ROM_REGION( 0x080000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "03n_u53.c8",        0x00000, 0x80000, CRC(1a6ca2ee) SHA1(13d34a10004ca172db7953e2be8daa90fc5b62ed) )

	ROM_REGION( 0x200000, "gp9001_0", 0 )
	ROM_LOAD( "rom4_u30.c19",       0x000000, 0x100000, CRC(7a012d8b) SHA1(a33d9490573a9fd1e799d3fe567f991926851c51) )
	ROM_LOAD( "rom5_u31.c18",       0x100000, 0x100000, CRC(60b127ab) SHA1(98785dfd6a86b4bd2c9786f6f72796c023b5b73a) )

	ROM_REGION( 0x1800000, "oki1", 0 )    /* OKIM6295 samples */ // each rom contains 8 0x40000 banks, there are 12 roms, so 96 (0x60) banks here!
	ROM_LOAD( "rom6_u65.a1",   0x0000000, 0x0200000, CRC(f33c6c0b) SHA1(06d73cd5b6d27de4d68f2dde1ed4dfa72b9a9178) )
	ROM_LOAD( "rom7_u66.a3",   0x0200000, 0x0200000, CRC(1306f8b3) SHA1(21b0d3180f1f4af77074c35c66844e38a464fea0) )
	ROM_LOAD( "rom8_u61.a4",   0x0400000, 0x0200000, CRC(4f211c00) SHA1(b067de95ad595a4915effefb83789e4e3d9db6f9) )
	ROM_LOAD( "rom9_u62.a6",   0x0600000, 0x0200000, CRC(292d3ef6) SHA1(d027d4c64e57f46e444ee83b62f6c3bdf02e4eed) )
	ROM_LOAD( "rom10_u67.a8",  0x0800000, 0x0200000, CRC(5219bf86) SHA1(946c8fcf3c04a88517d1a66ccd56609d22da945f) )
	ROM_LOAD( "rom11_u68.a10", 0x0a00000, 0x0200000, CRC(56fe4b1d) SHA1(2ea0413b435dd178174eb66d38dc9f7ab3d07ba5) )
	ROM_LOAD( "rom12_u63.a11", 0x0c00000, 0x0200000, CRC(cc48ff18) SHA1(10f6d9f445c9244b797846450f0c94700ccc7367) )
	ROM_LOAD( "rom13_u64.a13", 0x0e00000, 0x0200000, CRC(a3cd181a) SHA1(6a87479c24a61f7ac940e9c9bb62a18f26c9c843) )
	ROM_LOAD( "rom14_u69.a14", 0x1000000, 0x0200000, CRC(5d8cddec) SHA1(1912850d065d4ce1a1cdfd5a704218e660b5345b) )
	ROM_LOAD( "rom15_u70.a16", 0x1200000, 0x0200000, CRC(c75012f5) SHA1(b1ba0abab3eb8e9e3778eecab4951d828c85cecb) )
	ROM_LOAD( "rom16_u71.a18", 0x1400000, 0x0200000, CRC(efd02b0d) SHA1(b23fa3298fc29086f9ab05bc58775ff47b4cb7a9) )
	ROM_LOAD( "rom17_u72.a19", 0x1600000, 0x0200000, CRC(6b8717c3) SHA1(b5b7e35deaa2f34bccd1e83844d4bc0be845d0b8) )
ROM_END

ROM_START( snowbro2 )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "pro-4", 0x000000, 0x080000, CRC(4c7ee341) SHA1(ad46c605a38565d0148daac301be4e4b72302fe7) )

	ROM_REGION( 0x300000, "gp9001_0", 0 )
	ROM_LOAD( "rom2-l", 0x000000, 0x100000, CRC(e9d366a9) SHA1(e87e3966fce3395324b90db6c134b3345104c04b) )
	ROM_LOAD( "rom2-h", 0x100000, 0x080000, CRC(9aab7a62) SHA1(611f6a15fdbac5d3063426a365538c1482e996bf) )
	ROM_LOAD( "rom3-l", 0x180000, 0x100000, CRC(eb06e332) SHA1(7cd597bfffc153d178530c0f0903bebd751c9dd1) )
	ROM_LOAD( "rom3-h", 0x280000, 0x080000, CRC(df4a952a) SHA1(b76af61c8437caca573ff1312832898666a611aa) )

	ROM_REGION( 0x80000, "oki1", 0 )         /* ADPCM Samples */
	ROM_LOAD( "rom4", 0x00000, 0x80000, CRC(638f341e) SHA1(aa3fca25f099339ece1878ea730c5e9f18ec4823) )
ROM_END

ROM_START( snowbro2b ) // seems to be the same data as the main set, but with the extra user1 rom and different rom layout
	ROM_REGION( 0x080000, "maincpu", 0 )    /* Main 68K code - difference with main set is year changed from 1994 to 1998 and upper FFFF fill changed to 00FF fill */
	ROM_LOAD16_BYTE( "sb2-prg1.u39", 0x000000, 0x040000, CRC(e1fec8a2) SHA1(30c1a351070d784da9ba0dca68be8a262dba2045) )
	ROM_LOAD16_BYTE( "sb2-prg0.u23", 0x000001, 0x040000, CRC(b473cd57) SHA1(331130faa9de01b3ca93845174e8c3684bd269c7) )

	ROM_REGION( 0x400000, "gp9001_0", 0 )
	ROM_LOAD( "sb2-gfx.u177", 0x000000, 0x200000, CRC(ebeec910) SHA1(e179f393b98135caa8419b68cd979038ab47a413) )
	ROM_LOAD( "sb2-gfx.u175", 0x200000, 0x200000, CRC(e349c75b) SHA1(7d40d00fc0e15a68c427fe94db410bb7cbe00117) )

	ROM_REGION( 0x80000, "oki1", 0 )         /* ADPCM Samples */
	ROM_LOAD( "sb2-snd-4.u17", 0x00000, 0x80000, CRC(638f341e) SHA1(aa3fca25f099339ece1878ea730c5e9f18ec4823) )

	ROM_REGION( 0x8000, "user1", 0 )        /* ??? Some sort of table - same as other bootleg boards */
	ROM_LOAD( "sb2-unk.u100", 0x0000, 0x8000, CRC(456dd16e) SHA1(84779ee64d3ea33ba1ba4dee39b504a81c6811a1) )
ROM_END

ROM_START( snowbro2b2 ) // seems to mostly be the same data, but with copyright changed to Q Elec. Only set with staff credits still present. Also differently arranged graphics ROMs data.
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "rom10.bin", 0x000000, 0x080000, CRC(3e96da41) SHA1(692211d40f506efb9cb49848521de2da7890e248) ) // 27c04002

	ROM_REGION( 0x300000, "gp9001_0", 0 )
	ROM_LOAD16_BYTE( "rom07.bin", 0x000000, 0x080000, CRC(c54ae0b3) SHA1(94099b2da52eb12638799eab0819fe8a13aa3879) ) // 27c040
	ROM_LOAD16_BYTE( "rom05.bin", 0x000001, 0x080000, CRC(af3c74d1) SHA1(e97a688db50dfe41723452a9f652564e89e367ed) ) // 27c040
	ROM_LOAD16_BYTE( "rom08.bin", 0x100000, 0x040000, CRC(72812088) SHA1(1c0d410a7dd8de0bc48b7ff677979ad269966f7d) ) // 27c02001
	ROM_LOAD16_BYTE( "rom06.bin", 0x100001, 0x040000, CRC(c8f80774) SHA1(004752d7dfa08c3beb774f545fe3260d328abff0) ) // 27c02001
	ROM_LOAD16_BYTE( "rom03.bin", 0x180000, 0x080000, CRC(42fecbd7) SHA1(96dc9d5495d7830400ca7475c6613119099e93f2) ) // 27c040
	ROM_LOAD16_BYTE( "rom01.bin", 0x180001, 0x080000, CRC(e7134937) SHA1(7c12e7c6b08f804613e5ea0db8d622bda01bc036) ) // 27c040
	ROM_LOAD16_BYTE( "rom04.bin", 0x280000, 0x040000, CRC(3343b7a7) SHA1(10efcb2dfae635f005773655faa573bf51ddc6a3) ) // 27c020
	ROM_LOAD16_BYTE( "rom02.bin", 0x280001, 0x040000, CRC(af4d9551) SHA1(adcf1641e37b239b1ae4322b5710d49e53c30684) ) // 27c020

	ROM_REGION( 0x80000, "oki1", 0 )         /* ADPCM Samples */
	ROM_LOAD( "rom09.bin", 0x00000, 0x80000, CRC(638f341e) SHA1(aa3fca25f099339ece1878ea730c5e9f18ec4823) )
ROM_END

ROM_START( snowbro2b3 ) // SK000616 PCB, no original parts, seems hardcoded on Europe region
	ROM_REGION( 0x080000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "prg", 0x000000, 0x080000, CRC(8ce2ede2) SHA1(ddd8a2aa442cd5bb3a7d393b9b5c06fd981e7c61) )

	ROM_REGION( 0x400000, "gp9001_0", 0 ) // not actually a GP9001
	ROM_LOAD( "gfx2", 0x100000, 0x100000, CRC(a3be41af) SHA1(4cb1ce9c47bf8bbf7d1e36f6a1d276ce52957cfb) )
	ROM_CONTINUE(     0x000000, 0x100000 )
	ROM_LOAD( "gfx1", 0x300000, 0x100000, CRC(8df1ab06) SHA1(2a28caf7d545dc05acfcd2a8d2ffbd9f710af45d) )
	ROM_CONTINUE(     0x200000, 0x100000 )

	ROM_REGION( 0x80000, "oki1", 0 )
	ROM_LOAD( "voice", 0x00000, 0x80000, CRC(638f341e) SHA1(aa3fca25f099339ece1878ea730c5e9f18ec4823) )
ROM_END

ROM_START( snowbro2ny ) // Nyanko
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "rom1_c8.u61", 0x000000, 0x080000, CRC(9e6eb76b) SHA1(9e8b356dabedeb4ae9e08d60fbf6ed4a09edc0bd) )

	ROM_REGION( 0x300000, "gp9001_0", 0 )
	ROM_LOAD( "rom2-l_tp-033.u13", 0x000000, 0x100000, CRC(e9d366a9) SHA1(e87e3966fce3395324b90db6c134b3345104c04b) )
	ROM_LOAD( "rom2-h_c10.u26",    0x100000, 0x080000, CRC(9aab7a62) SHA1(611f6a15fdbac5d3063426a365538c1482e996bf) )
	ROM_LOAD( "rom3-l_tp-033.u12", 0x180000, 0x100000, CRC(eb06e332) SHA1(7cd597bfffc153d178530c0f0903bebd751c9dd1) )
	ROM_LOAD( "rom3-h_c9.u27",     0x280000, 0x080000, CRC(6de2b059) SHA1(695e789849c34de5d83e40b0e834b2106fcd78db) )

	ROM_REGION( 0x80000, "oki1", 0 )         /* ADPCM Samples */
	ROM_LOAD( "rom4-tp-033.u33", 0x00000, 0x80000, CRC(638f341e) SHA1(aa3fca25f099339ece1878ea730c5e9f18ec4823) )

	ROM_REGION( 0x345, "plds", 0 )
	ROM_LOAD( "13_gal16v8-25lnc.u91", 0x000, 0x117, NO_DUMP ) // Protected
	ROM_LOAD( "14_gal16v8-25lnc.u92", 0x117, 0x117, NO_DUMP ) // Protected
	ROM_LOAD( "15_gal16v8-25lnc.u93", 0x22e, 0x117, NO_DUMP ) // Protected
ROM_END

/* -------------------------- Raizing games ------------------------- */


/*
For the two sets of Sorcer Striker (World) the only differences
are 2 bytes plus a corrected checksum for each set:

File Offset     sstriker   sstrikera
  0x160            17         0B   <-- Rom checksum value
  0x161            79         6D   <-- Rom checksum value

  0x92C            18         0C   <-- Index of copyright strings to display for Korea
  0x92D            18         0C   <-- Index of copyright strings to display for Korea

0C points to the strings "LICENSED TO UNITE TRADING" / "FOR KOREA".
18 points to a pair of empty strings.

Printed labels for the eproms look like:

RA-MA-01
   01
RAIZING

Both English and Japanese sets use the same labels and numbers for the roms
even if the roms contain different code / data.
*/

ROM_START( sstriker )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "ra-ma_01_01.u65", 0x000000, 0x080000, CRC(708fd51d) SHA1(167186d4cf13af37ec0fa6a59c738c54dbbf3c7c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "ra-ma-01_02.u66", 0x00000, 0x10000, CRC(eabfa46d) SHA1(402c99ebf88f9025f74f0a28ced22b7882a65eb3) )

	ROM_REGION( 0x200000, "gp9001_0", 0 )
	ROM_LOAD( "ra-ma01-rom2.u2",  0x000000, 0x100000, CRC(54e2bd95) SHA1(341359dd46152615675bb90e8a184216c8feebff) )
	ROM_LOAD( "ra-ma01-rom3.u1",  0x100000, 0x100000, CRC(21cd378f) SHA1(e1695bccec949d18b1c03e9c42dca384554b0d7c) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "ra-ma-01_05.u81",  0x000000, 0x008000, CRC(88b58841) SHA1(1d16b538c11a291bd1f46a510bfbd6259b45a0b5) )

	ROM_REGION( 0x40000, "oki1", 0 )         /* ADPCM Samples */
	ROM_LOAD( "ra-ma01-rom1.u57", 0x00000, 0x40000, CRC(6edb2ab8) SHA1(e3032e8eda2686f30df4b7a088c5a4d4d45782ed) )
ROM_END


ROM_START( sstrikerk )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "ra-ma-01_01.u65", 0x000000, 0x080000, CRC(92259f84) SHA1(127e62e407d95efd360bfe2cac9577f326abf6ef) )

	ROM_REGION( 0x10000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "ra-ma-01_02.u66", 0x00000, 0x10000, CRC(eabfa46d) SHA1(402c99ebf88f9025f74f0a28ced22b7882a65eb3) )

	ROM_REGION( 0x200000, "gp9001_0", 0 )
	ROM_LOAD( "ra-ma01-rom2.u2",  0x000000, 0x100000, CRC(54e2bd95) SHA1(341359dd46152615675bb90e8a184216c8feebff) )
	ROM_LOAD( "ra-ma01-rom3.u1",  0x100000, 0x100000, CRC(21cd378f) SHA1(e1695bccec949d18b1c03e9c42dca384554b0d7c) )
	// also seen with 4 smaller ROMs instead of 2
	// 01.bin                  ra-ma01-rom2.u2 [even]     IDENTICAL
	// 02.bin                  ra-ma01-rom2.u2 [odd]      IDENTICAL
	// 03.bin                  ra-ma01-rom3.u1 [even]     IDENTICAL
	// 04.bin                  ra-ma01-rom3.u1 [odd]      IDENTICAL

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "ra-ma-01_05.u81",  0x000000, 0x008000, CRC(88b58841) SHA1(1d16b538c11a291bd1f46a510bfbd6259b45a0b5) )

	ROM_REGION( 0x40000, "oki1", 0 )         /* ADPCM Samples */
	ROM_LOAD( "ra-ma01-rom1.u57", 0x00000, 0x40000, CRC(6edb2ab8) SHA1(e3032e8eda2686f30df4b7a088c5a4d4d45782ed) )
ROM_END


ROM_START( mahoudai )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "ra_ma_01_01.u65", 0x000000, 0x080000, CRC(970ccc5c) SHA1(c87cab83bde0284e631f02e50068407fee81d941) )

	ROM_REGION( 0x10000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "ra-ma-01_02.u66", 0x00000, 0x10000, CRC(eabfa46d) SHA1(402c99ebf88f9025f74f0a28ced22b7882a65eb3) )

	ROM_REGION( 0x200000, "gp9001_0", 0 )
	ROM_LOAD( "ra-ma01-rom2.u2",  0x000000, 0x100000, CRC(54e2bd95) SHA1(341359dd46152615675bb90e8a184216c8feebff) )
	ROM_LOAD( "ra-ma01-rom3.u1",  0x100000, 0x100000, CRC(21cd378f) SHA1(e1695bccec949d18b1c03e9c42dca384554b0d7c) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "ra_ma_01_05.u81",  0x000000, 0x008000, CRC(c00d1e80) SHA1(53e64c4c0c6309130b37597d13b44a9e95b717d8) )

	ROM_REGION( 0x40000, "oki1", 0 )         /* ADPCM Samples */
	ROM_LOAD( "ra-ma01-rom1.u57", 0x00000, 0x40000, CRC(6edb2ab8) SHA1(e3032e8eda2686f30df4b7a088c5a4d4d45782ed) )
ROM_END


ROM_START( kingdmgp )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "ma02rom1.bin", 0x000000, 0x080000, CRC(a678b149) SHA1(8c1a631e023dbba0a3fa6cd1b7d10dec1663213a) )
	ROM_LOAD16_BYTE( "ma02rom0.bin", 0x000001, 0x080000, CRC(f226a212) SHA1(526acf3d05fdc88054a772fbea3de2af532bf3d2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "ma02rom2.bin", 0x00000, 0x10000, CRC(dde8a57e) SHA1(f522a3f17e229c71512464349760a9e27778bf6a) )

	ROM_REGION( 0x400000, "gp9001_0", 0 )
	ROM_LOAD( "ma02rom3.bin",  0x000000, 0x200000, CRC(0e797142) SHA1(a480ccd151e49b886d3175a6deff56e1f2c26c3e) )
	ROM_LOAD( "ma02rom4.bin",  0x200000, 0x200000, CRC(72a6fa53) SHA1(ce92e65205b84361cfb90305a61e9541b5c4dc2f) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "ma02rom5.eng",  0x000000, 0x008000, CRC(8c28460b) SHA1(0aed170762f6044896a7e608df60bbd37c583a71) )

	ROM_REGION( 0x80000, "oki1", 0 )         /* ADPCM Samples */
	ROM_LOAD( "ma02rom6.bin", 0x00000, 0x80000, CRC(199e7cae) SHA1(0f5e13cc8ec42c80bb4bbff90aba29cdb15213d4) )
ROM_END


ROM_START( shippumd )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "ma02rom1.bin", 0x000000, 0x080000, CRC(a678b149) SHA1(8c1a631e023dbba0a3fa6cd1b7d10dec1663213a) )
	ROM_LOAD16_BYTE( "ma02rom0.bin", 0x000001, 0x080000, CRC(f226a212) SHA1(526acf3d05fdc88054a772fbea3de2af532bf3d2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "ma02rom2.bin", 0x00000, 0x10000, CRC(dde8a57e) SHA1(f522a3f17e229c71512464349760a9e27778bf6a) )

	ROM_REGION( 0x400000, "gp9001_0", 0 )
	ROM_LOAD( "ma02rom3.bin",  0x000000, 0x200000, CRC(0e797142) SHA1(a480ccd151e49b886d3175a6deff56e1f2c26c3e) )
	ROM_LOAD( "ma02rom4.bin",  0x200000, 0x200000, CRC(72a6fa53) SHA1(ce92e65205b84361cfb90305a61e9541b5c4dc2f) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "ma02rom5.bin",  0x000000, 0x008000, CRC(116ae559) SHA1(4cc2d2a23cc0aefd457111b7990e47184e79204c) )

	ROM_REGION( 0x80000, "oki1", 0 )         /* ADPCM Samples */
	ROM_LOAD( "ma02rom6.bin", 0x00000, 0x80000, CRC(199e7cae) SHA1(0f5e13cc8ec42c80bb4bbff90aba29cdb15213d4) )
ROM_END

ROM_START( bgareggat )
	/* Dumped from a location test board, with some minor changes compared to the final.
	* All ROMs are socketed
	* All PAL/GALs are socketed
	* PLDs at U33, U125 are PALCE16V8H/4 instead of GAL16V8B as usual (no functional impact)
	* JP4 features four DIP switches, instead of two DIPs + two jumpers as in the final.

	The date codes are written referencing Heisei year 8 (1996).

	The program ROMs feature hand-written labels formatted like this:
	BATTLE GAREGGA
	     PRG 0
	    8.1.17.
	*/
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "battlegaregga-prg0-8-1-17.bin", 0x000000, 0x080000, CRC(c032176f) SHA1(799ba0424489361dd2f814afaf841326bc23300c) )
	ROM_LOAD16_BYTE( "battlegaregga-prg1-8-1-17.bin", 0x000001, 0x080000, CRC(3822f375) SHA1(a5a84cf48c86d8ac97f401280667658d7f451896) )

	/* Hand-written label that reads
	BATTLE GAREGGA
	      SND
	8.1.18. ロケVer.
	*/
	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "battlegaregga-snd-8-1-18-loke-ver.bin", 0x00000, 0x20000, CRC(f5ea56f7) SHA1(9db04069b378dbad6626fd29d3762e3361b9aa0d) )

	/* Stored on NEC ES23C16000W Mask ROMs with no Raizing/8ing custom markings.*/
	ROM_REGION( 0x800000, "gp9001_0", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	/* Hand-written label that reads
	BATTLE GAREGGA
	     TEXT
	8.1.17. 8AA6

	8AA6 is the checksum of the text ROM, which matches release.
	*/
	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "text.u81", 0x00000, 0x08000, CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	/* Stored on an NEC ES23C8001EJ Mask ROM with no Raizing/8ing custom markings.*/
	ROM_REGION( 0x100000, "oki1", 0 )        /* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END

ROM_START( bgaregga )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "prg0.bin", 0x000000, 0x080000, CRC(f80c2fc2) SHA1(a9aac5c7f5439b6fe8d1b3db1fb02a27cc28fdf6) )
	ROM_LOAD16_BYTE( "prg1.bin", 0x000001, 0x080000, CRC(2ccfdd1e) SHA1(7a9f11f851854f3f8389b9c3c0906ebb8dc28712) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.bin", 0x00000, 0x20000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )

	ROM_REGION( 0x800000, "gp9001_0", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "text.u81", 0x00000, 0x08000, CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	ROM_REGION( 0x100000, "oki1", 0 )        /* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END


ROM_START( bgareggahk )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "prg_0.rom", 0x000000, 0x080000, CRC(26e0019e) SHA1(5197001f5d59246b137e19ed1952a8207b25d4c0) )
	ROM_LOAD16_BYTE( "prg_1.rom", 0x000001, 0x080000, CRC(2ccfdd1e) SHA1(7a9f11f851854f3f8389b9c3c0906ebb8dc28712) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.bin", 0x00000, 0x20000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )

	ROM_REGION( 0x800000, "gp9001_0", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "text.u81", 0x00000, 0x08000, CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	ROM_REGION( 0x100000, "oki1", 0 )        /* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END


ROM_START( bgareggatw )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "garegga_prg0.u123", 0x000000, 0x080000, CRC(235b7405) SHA1(a2434801df4231a6b48f6c63f47c202d25a89e79) )
	ROM_LOAD16_BYTE( "garegga_prg1.u65",  0x000001, 0x080000, CRC(c29ccf6a) SHA1(38806e0b4ff852f4bfefd80c56ca23f71623e275) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.bin", 0x00000, 0x20000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )

	ROM_REGION( 0x800000, "gp9001_0", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "text.u81", 0x00000, 0x08000, CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	ROM_REGION( 0x100000, "oki1", 0 )        /* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END


// only the program ROMs' dumps were provided for this set.
// According to the dumper: 'In the Korea Region Setting, DIP SWITCH's 'STAGE EDIT' does not work and
// the C button (formation change) function in the in-game is also deleted.'
ROM_START( bgareggak )
	ROM_REGION( 0x100000, "maincpu", 0 )            // Main 68K code
	ROM_LOAD16_BYTE( "prg0.bin", 0x000000, 0x080000, CRC(40a108a7) SHA1(cc3227dc87ffefb961dbcdff146e787dbfbdfc2c) )
	ROM_LOAD16_BYTE( "prg1.bin", 0x000001, 0x080000, CRC(45a6e48a) SHA1(f4d4158b8556b4261291ba9905b9731623b47e54) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            // Sound Z80 code + bank
	ROM_LOAD( "snd.bin", 0x00000, 0x20000, BAD_DUMP CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )

	ROM_REGION( 0x800000, "gp9001_0", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, BAD_DUMP CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, BAD_DUMP CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, BAD_DUMP CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, BAD_DUMP CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "text.u81", 0x00000, 0x08000, BAD_DUMP CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	ROM_REGION( 0x100000, "oki1", 0 )        // ADPCM Samples
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, BAD_DUMP CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END


ROM_START( bgaregganv )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "prg_0.bin", 0x000000, 0x080000, CRC(951ecc07) SHA1(a82e4b59e4a974566e59f3ab2fbae1aec7d88a2b) )
	ROM_LOAD16_BYTE( "prg_1.bin", 0x000001, 0x080000, CRC(729a60c6) SHA1(cb6f5d138bb82c32910f42d8ee16fa573a23cef3) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.bin", 0x00000, 0x20000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )

	ROM_REGION( 0x800000, "gp9001_0", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "text.u81", 0x00000, 0x08000, CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	ROM_REGION( 0x100000, "oki1", 0 )        /* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END


ROM_START( bgareggat2 )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "prg0", 0x000000, 0x080000, CRC(84094099) SHA1(49fc68a8bcdae4477e20eade9dd569de88b0b798) )
	ROM_LOAD16_BYTE( "prg1", 0x000001, 0x080000, CRC(46f92fe4) SHA1(62a02cc1dbdc3ac362339aebb62368eb89b06bad) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.bin", 0x00000, 0x20000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )

	ROM_REGION( 0x800000, "gp9001_0", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "text.u81", 0x00000, 0x08000, CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	ROM_REGION( 0x100000, "oki1", 0 )        /* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END


ROM_START( bgareggacn )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "u123", 0x000000, 0x080000, CRC(88a4e66a) SHA1(ca97e564eed0c5e028b937312e55da56400d5c8c) )
	ROM_LOAD16_BYTE( "u65",  0x000001, 0x080000, CRC(5dea32a3) SHA1(59df6689e3eb5ea9e49a758604d21a64c65ca14d) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.bin", 0x00000, 0x20000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )

	ROM_REGION( 0x800000, "gp9001_0", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "text.u81", 0x00000, 0x08000, CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	ROM_REGION( 0x100000, "oki1", 0 )        /* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END

// 1945二代 (1945 Èr Dài)
ROM_START( bgareggabl )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "xt-8m.bin", 0x000000, 0x100000, CRC(4a6657cb) SHA1(1838956e7597eaa78ea5ee58d0ccc79cbbd7ded5) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.bin", 0x00000, 0x20000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )

	ROM_REGION( 0x800000, "gp9001_0", 0 )
	ROM_LOAD( "6@-322",  0x000000, 0x400000, CRC(37fe48ed) SHA1(ded5d13c33b4582310cdfb3dd52c052f741c00c5) ) /* == rom4.bin+rom3.bin */
	ROM_LOAD( "5@-322",  0x400000, 0x400000, CRC(5a06c031) SHA1(ee241ff90117cec1f33ab163601a9d5c75609739) ) /* == rom2.bin+rom1.bin */

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "1@-256", 0x00000, 0x08000, CRC(760dcd14) SHA1(e151e5d7ca5557277f306b9484ec021f4edf1e07) )

	ROM_REGION( 0x010000, "user1", 0 ) // not graphics
	ROM_LOAD( "2@-256", 0x00000, 0x08000, CRC(456dd16e) SHA1(84779ee64d3ea33ba1ba4dee39b504a81c6811a1) )

	ROM_REGION( 0x100000, "oki1", 0 )        /* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END

// 雷神传 (Léishén Chuán)
ROM_START( bgareggabla )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "27c8100.mon-sys", 0x000000, 0x100000, CRC(d334e5aa) SHA1(41607b5630d7b92a96607ea95c5b55ad43745857) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.bin", 0x00000, 0x20000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )

	ROM_REGION( 0x800000, "gp9001_0", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "text.bin", 0x00000, 0x08000, CRC(00d100bd) SHA1(fb6028e3519d6588a966d1b16d47453db2e51fd7))

	ROM_REGION( 0x010000, "user1", 0 ) // not graphics
	ROM_LOAD( "base.bin", 0x00000, 0x08000, CRC(456dd16e) SHA1(84779ee64d3ea33ba1ba4dee39b504a81c6811a1) )

	ROM_REGION( 0x100000, "oki1", 0 )        /* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END

ROM_START( bgareggablj ) // fixed on Japanese region
	ROM_REGION( 0x100000, "maincpu", 0 )            // Main 68K code
	ROM_LOAD16_WORD_SWAP( "sys.bin", 0x000000, 0x100000, CRC(b2a1225f) SHA1(dda00aa5e7ce3f39bb0eebbcd9500ef22a5d74d3) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_IGNORE(                                0x100000 )

	ROM_REGION( 0x20000, "audiocpu", 0 )            // Sound Z80 code + bank
	ROM_LOAD( "snd.bin", 0x00000, 0x20000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )

	ROM_REGION( 0x800000, "gp9001_0", 0 )
	ROM_LOAD( "322_2.bin",  0x000000, 0x400000, CRC(37fe48ed) SHA1(ded5d13c33b4582310cdfb3dd52c052f741c00c5) ) // == rom4.bin+rom3.bin
	ROM_LOAD( "322_1.bin",  0x400000, 0x400000, CRC(5a06c031) SHA1(ee241ff90117cec1f33ab163601a9d5c75609739) ) // == rom2.bin+rom1.bin

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "text.bin", 0x00000, 0x08000, CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	ROM_REGION( 0x010000, "user1", 0 ) // not graphics
	ROM_LOAD( "base.bin", 0x00000, 0x08000, CRC(456dd16e) SHA1(84779ee64d3ea33ba1ba4dee39b504a81c6811a1) )

	ROM_REGION( 0x100000, "oki1", 0 )        // ADPCM Samples
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END

/*
   The region of Batrider is controlled by the first byte of rom prg0.u22
   only sets which have been dumped from original PCBs are supported

   original ROM labels have no indication of the region.

   valid values are:
    ( * denotes that this set has been found on an original PCB )

   00 : Nippon *
   01 : USA *
   02 : Europe *
   03 : Asia
   04 : German (sic)
   05 : Austria
   06 : Belgium
   07 : Denmark
   08 : Finland
   09 : France
   0A : Great Britain
   0B : Greece
   0C : The Netherlands
   0D : Italy
   0E : Norway
   0F : Portugal
   10 : Spain
   11 : Sweden
   12 : Switzerland
   13 : Australia
   14 : New Zealand
   15 : Taiwan
   16 : Hong Kong
   17 : Korea *
   18 : China *
   19 : No Region?
   1A+: Invalid

   For future reference, that would mean the following

   ROM_LOAD16_BYTE( "prg0_nippon.u22",       0x000000, 0x080000, CRC(4f3fc729) SHA1(b32d51c254741b82171a86c271679522a7aefd34) )
   ROM_LOAD16_BYTE( "prg0_usa.u22",          0x000000, 0x080000, CRC(2049d007) SHA1(f2a43547a6fc5083b03c1d59a85abbf6e1ce4cd9) )
   ROM_LOAD16_BYTE( "prg0_europe.u22",       0x000000, 0x080000, CRC(91d3e975) SHA1(682885fc17f2424d475c282f239f42faf1aae076) )
   ROM_LOAD16_BYTE( "prg0_asia.u22",         0x000000, 0x080000, CRC(fea5fe5b) SHA1(0008336ecd3886485ab1d9678880b1a0bc788f40) )
   ROM_LOAD16_BYTE( "prg0_german.u22",       0x000000, 0x080000, CRC(29969dd0) SHA1(eb8ad84b772508b6befb35afb11a0d6193c6a060) )
   ROM_LOAD16_BYTE( "prg0_austria.u22",      0x000000, 0x080000, CRC(46e08afe) SHA1(a6f46581d0f7285704fbf1ac57476c96f4dcbec2) )
   ROM_LOAD16_BYTE( "prg0_belgium.u22",      0x000000, 0x080000, CRC(f77ab38c) SHA1(8be87175250345d3e31d95ec204805071eae81f6) )
   ROM_LOAD16_BYTE( "prg0_denmark.u22",      0x000000, 0x080000, CRC(980ca4a2) SHA1(4f29eaa5ba6b94d96c527f80188657abc8f4dcd0) )
   ROM_LOAD16_BYTE( "prg0_finland.u22",      0x000000, 0x080000, CRC(826d72db) SHA1(be4bca0143f43c13361fd56974eb9b1ce7bd1740) )
   ROM_LOAD16_BYTE( "prg0_france.u22",       0x000000, 0x080000, CRC(ed1b65f5) SHA1(1e08957c0f7ed65695fb1ceb961ab765f8a97c89) )
   ROM_LOAD16_BYTE( "prg0_greatbritain.u22", 0x000000, 0x080000, CRC(5c815c87) SHA1(dea89944cd9a3fa6991b214495dc7123a505d39b) )
   ROM_LOAD16_BYTE( "prg0_greece.u22",       0x000000, 0x080000, CRC(33f74ba9) SHA1(fe770415584b037152b37a75fe468d3c52dcb3cd) )
   ROM_LOAD16_BYTE( "prg0_netherlands.u22",  0x000000, 0x080000, CRC(e4c42822) SHA1(8bfd286c42d7f2b3c88757b9a8b818be90b73f48) )
   ROM_LOAD16_BYTE( "prg0_italy.u22",        0x000000, 0x080000, CRC(8bb23f0c) SHA1(b448bba312a8d583a981f6633cbc14af99fdbb06) )
   ROM_LOAD16_BYTE( "prg0_norway.u22",       0x000000, 0x080000, CRC(3a28067e) SHA1(9435e6ce90b8d740a545469e6edb35d1af11ceab) )
   ROM_LOAD16_BYTE( "prg0_portugal.u22",     0x000000, 0x080000, CRC(555e1150) SHA1(5c9ae898244a23a4184f9613f42d9aa9530468b9) )
   ROM_LOAD16_BYTE( "prg0_spain.u22",        0x000000, 0x080000, CRC(0eebaa8c) SHA1(e305e90434e7f322a33e42a642362f770d3eb0e5) )
   ROM_LOAD16_BYTE( "prg0_sweden.u22",       0x000000, 0x080000, CRC(619dbda2) SHA1(9e88ba104a5cffcced3b93ca711487a82b0fddde) )
   ROM_LOAD16_BYTE( "prg0_switzerland.u22",  0x000000, 0x080000, CRC(d00784d0) SHA1(0b809414ce910684ca39216086f7d26fd2adeded) )
   ROM_LOAD16_BYTE( "prg0_australia.u22",    0x000000, 0x080000, CRC(bf7193fe) SHA1(9af50fffc6ef23e300bf7b5e90b0dee6e4f4ad05) )
   ROM_LOAD16_BYTE( "prg0_newzealand.u22",   0x000000, 0x080000, CRC(6842f075) SHA1(125b303c064d2f0b539ecadcb205756e7fd1334e) )
   ROM_LOAD16_BYTE( "prg0_taiwan.u22",       0x000000, 0x080000, CRC(0734e75b) SHA1(17a8fb4f8fda3c234ed976490193ba308cac08fe) )
   ROM_LOAD16_BYTE( "prg0_hongkong.u22",     0x000000, 0x080000, CRC(b6aede29) SHA1(580f29db6a2c2cea43966413778362694992a675) )
   ROM_LOAD16_BYTE( "prg0_korea.u22",        0x000000, 0x080000, CRC(d9d8c907) SHA1(69c197f2a41f288913f042de9eb8274c0df3ac27) )
   ROM_LOAD16_BYTE( "prg0_china.u22",        0x000000, 0x080000, CRC(c3b91f7e) SHA1(6b2376c37808dccda296d90ccd7f577ccff4e4dc) )
   ROM_LOAD16_BYTE( "prg0_none.u22",         0x000000, 0x080000, CRC(accf0850) SHA1(d93e4e80443a40c3a9575dbf21927ef0d1a039b9) )
 */


ROM_START( batrider )
	ROM_REGION( 0x200000, "maincpu", 0 )            /* Main 68k code */
	ROM_LOAD16_BYTE( "prg0_europe.u22", 0x000000, 0x080000, CRC(91d3e975) SHA1(682885fc17f2424d475c282f239f42faf1aae076) )
	ROM_LOAD16_BYTE( "prg1b.u23", 0x000001, 0x080000, CRC(8e70b492) SHA1(f84f2039826ae815afb058d71c1dbd190f9d524d) )
	ROM_LOAD16_BYTE( "prg2.u21" , 0x100000, 0x080000, CRC(bdaa5fbf) SHA1(abd72ac633c0c8e7b4b1d7902c0d6e014ba995fe) )
	ROM_LOAD16_BYTE( "prg3.u24" , 0x100001, 0x080000, CRC(7aa9f941) SHA1(99bdbad7a96d461073b06a53c50fc57c2fd6fc6d) )

	ROM_REGION( 0x40000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.u77", 0x00000, 0x40000, CRC(56682696) SHA1(a372450d9a6d535123dfc31d8116074b168ab646) )

	ROM_REGION( 0x1000000, "gp9001_0", 0 )
	ROM_LOAD( "rom-1.bin", 0x000000, 0x400000, CRC(0df69ca2) SHA1(49670347ebd7e1067ff988cf842b275b7ee7b5f7) )
	ROM_LOAD( "rom-3.bin", 0x400000, 0x400000, CRC(60167d38) SHA1(fd2429808c59ef51fd5f5db84ea89a8dc504186e) )
	ROM_LOAD( "rom-2.bin", 0x800000, 0x400000, CRC(1bfea593) SHA1(ce06dc3097ae56b0df56d104bbf7efc9b5d968d4) )
	ROM_LOAD( "rom-4.bin", 0xc00000, 0x400000, CRC(bee03c94) SHA1(5bc1e6769c42857c03456426b502fcb86a114f19) )

	ROM_REGION( 0x100000, "oki1", 0 )       /* ADPCM Samples 1 */
	ROM_LOAD( "rom-5.bin", 0x000000, 0x100000, CRC(4274daf6) SHA1(85557b4707d529e5914f03c7a856864f5c24950e) )

	ROM_REGION( 0x100000, "oki2", 0 )       /* ADPCM Samples 2 */
	ROM_LOAD( "rom-6.bin", 0x000000, 0x100000, CRC(2a1c2426) SHA1(8abc3688ffc5ebb94b8d5118d4fa0908f07fe791) )
ROM_END


ROM_START( batrideru )
	ROM_REGION( 0x200000, "maincpu", 0 )            /* Main 68k code */
	ROM_LOAD16_BYTE( "prg0_usa.u22", 0x000000, 0x080000, CRC(2049d007) SHA1(f2a43547a6fc5083b03c1d59a85abbf6e1ce4cd9) )
	ROM_LOAD16_BYTE( "prg1b.u23", 0x000001, 0x080000, CRC(8e70b492) SHA1(f84f2039826ae815afb058d71c1dbd190f9d524d) )
	ROM_LOAD16_BYTE( "prg2.u21" , 0x100000, 0x080000, CRC(bdaa5fbf) SHA1(abd72ac633c0c8e7b4b1d7902c0d6e014ba995fe) )
	ROM_LOAD16_BYTE( "prg3.u24" , 0x100001, 0x080000, CRC(7aa9f941) SHA1(99bdbad7a96d461073b06a53c50fc57c2fd6fc6d) )

	ROM_REGION( 0x40000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.u77", 0x00000, 0x40000, CRC(56682696) SHA1(a372450d9a6d535123dfc31d8116074b168ab646) )

	ROM_REGION( 0x1000000, "gp9001_0", 0 )
	ROM_LOAD( "rom-1.bin", 0x000000, 0x400000, CRC(0df69ca2) SHA1(49670347ebd7e1067ff988cf842b275b7ee7b5f7) )
	ROM_LOAD( "rom-3.bin", 0x400000, 0x400000, CRC(60167d38) SHA1(fd2429808c59ef51fd5f5db84ea89a8dc504186e) )
	ROM_LOAD( "rom-2.bin", 0x800000, 0x400000, CRC(1bfea593) SHA1(ce06dc3097ae56b0df56d104bbf7efc9b5d968d4) )
	ROM_LOAD( "rom-4.bin", 0xc00000, 0x400000, CRC(bee03c94) SHA1(5bc1e6769c42857c03456426b502fcb86a114f19) )

	ROM_REGION( 0x100000, "oki1", 0 )       /* ADPCM Samples 1 */
	ROM_LOAD( "rom-5.bin", 0x000000, 0x100000, CRC(4274daf6) SHA1(85557b4707d529e5914f03c7a856864f5c24950e) )

	ROM_REGION( 0x100000, "oki2", 0 )       /* ADPCM Samples 2 */
	ROM_LOAD( "rom-6.bin", 0x000000, 0x100000, CRC(2a1c2426) SHA1(8abc3688ffc5ebb94b8d5118d4fa0908f07fe791) )
ROM_END


ROM_START( batriderc )
	ROM_REGION( 0x200000, "maincpu", 0 )            /* Main 68k code */
	ROM_LOAD16_BYTE( "prg0_china.u22", 0x000000, 0x080000, CRC(c3b91f7e) SHA1(6b2376c37808dccda296d90ccd7f577ccff4e4dc) )
	ROM_LOAD16_BYTE( "prg1b.u23", 0x000001, 0x080000, CRC(8e70b492) SHA1(f84f2039826ae815afb058d71c1dbd190f9d524d) )
	ROM_LOAD16_BYTE( "prg2.u21" , 0x100000, 0x080000, CRC(bdaa5fbf) SHA1(abd72ac633c0c8e7b4b1d7902c0d6e014ba995fe) )
	ROM_LOAD16_BYTE( "prg3.u24" , 0x100001, 0x080000, CRC(7aa9f941) SHA1(99bdbad7a96d461073b06a53c50fc57c2fd6fc6d) )

	ROM_REGION( 0x40000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.u77", 0x00000, 0x40000, CRC(56682696) SHA1(a372450d9a6d535123dfc31d8116074b168ab646) )

	ROM_REGION( 0x1000000, "gp9001_0", 0 )
	ROM_LOAD( "rom-1.bin", 0x000000, 0x400000, CRC(0df69ca2) SHA1(49670347ebd7e1067ff988cf842b275b7ee7b5f7) )
	ROM_LOAD( "rom-3.bin", 0x400000, 0x400000, CRC(60167d38) SHA1(fd2429808c59ef51fd5f5db84ea89a8dc504186e) )
	ROM_LOAD( "rom-2.bin", 0x800000, 0x400000, CRC(1bfea593) SHA1(ce06dc3097ae56b0df56d104bbf7efc9b5d968d4) )
	ROM_LOAD( "rom-4.bin", 0xc00000, 0x400000, CRC(bee03c94) SHA1(5bc1e6769c42857c03456426b502fcb86a114f19) )

	ROM_REGION( 0x100000, "oki1", 0 )       /* ADPCM Samples 1 */
	ROM_LOAD( "rom-5.bin", 0x000000, 0x100000, CRC(4274daf6) SHA1(85557b4707d529e5914f03c7a856864f5c24950e) )

	ROM_REGION( 0x100000, "oki2", 0 )       /* ADPCM Samples 2 */
	ROM_LOAD( "rom-6.bin", 0x000000, 0x100000, CRC(2a1c2426) SHA1(8abc3688ffc5ebb94b8d5118d4fa0908f07fe791) )
ROM_END


ROM_START( batriderj )
	ROM_REGION( 0x200000, "maincpu", 0 )            /* Main 68k code */
	ROM_LOAD16_BYTE( "prg0b.u22", 0x000000, 0x080000, CRC(4f3fc729) SHA1(b32d51c254741b82171a86c271679522a7aefd34) )
	ROM_LOAD16_BYTE( "prg1b.u23", 0x000001, 0x080000, CRC(8e70b492) SHA1(f84f2039826ae815afb058d71c1dbd190f9d524d) )
	ROM_LOAD16_BYTE( "prg2.u21" , 0x100000, 0x080000, CRC(bdaa5fbf) SHA1(abd72ac633c0c8e7b4b1d7902c0d6e014ba995fe) )
	ROM_LOAD16_BYTE( "prg3.u24" , 0x100001, 0x080000, CRC(7aa9f941) SHA1(99bdbad7a96d461073b06a53c50fc57c2fd6fc6d) )

	ROM_REGION( 0x40000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.u77", 0x00000, 0x40000, CRC(56682696) SHA1(a372450d9a6d535123dfc31d8116074b168ab646) )

	ROM_REGION( 0x1000000, "gp9001_0", 0 )
	ROM_LOAD( "rom-1.bin", 0x000000, 0x400000, CRC(0df69ca2) SHA1(49670347ebd7e1067ff988cf842b275b7ee7b5f7) )
	ROM_LOAD( "rom-3.bin", 0x400000, 0x400000, CRC(60167d38) SHA1(fd2429808c59ef51fd5f5db84ea89a8dc504186e) )
	ROM_LOAD( "rom-2.bin", 0x800000, 0x400000, CRC(1bfea593) SHA1(ce06dc3097ae56b0df56d104bbf7efc9b5d968d4) )
	ROM_LOAD( "rom-4.bin", 0xc00000, 0x400000, CRC(bee03c94) SHA1(5bc1e6769c42857c03456426b502fcb86a114f19) )

	ROM_REGION( 0x100000, "oki1", 0 )       /* ADPCM Samples 1 */
	ROM_LOAD( "rom-5.bin", 0x000000, 0x100000, CRC(4274daf6) SHA1(85557b4707d529e5914f03c7a856864f5c24950e) )

	ROM_REGION( 0x100000, "oki2", 0 )       /* ADPCM Samples 2 */
	ROM_LOAD( "rom-6.bin", 0x000000, 0x100000, CRC(2a1c2426) SHA1(8abc3688ffc5ebb94b8d5118d4fa0908f07fe791) )
ROM_END


ROM_START( batriderk )
	ROM_REGION( 0x200000, "maincpu", 0 )            /* Main 68k code */
	ROM_LOAD16_BYTE( "prg0_korea.u22", 0x000000, 0x080000, CRC(d9d8c907) SHA1(69c197f2a41f288913f042de9eb8274c0df3ac27) )
	ROM_LOAD16_BYTE( "prg1b.u23", 0x000001, 0x080000, CRC(8e70b492) SHA1(f84f2039826ae815afb058d71c1dbd190f9d524d) )
	ROM_LOAD16_BYTE( "prg2.u21" , 0x100000, 0x080000, CRC(bdaa5fbf) SHA1(abd72ac633c0c8e7b4b1d7902c0d6e014ba995fe) )
	ROM_LOAD16_BYTE( "prg3.u24" , 0x100001, 0x080000, CRC(7aa9f941) SHA1(99bdbad7a96d461073b06a53c50fc57c2fd6fc6d) )

	ROM_REGION( 0x40000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.u77", 0x00000, 0x40000, CRC(56682696) SHA1(a372450d9a6d535123dfc31d8116074b168ab646) )

	ROM_REGION( 0x1000000, "gp9001_0", 0 )
	ROM_LOAD( "rom-1.bin", 0x000000, 0x400000, CRC(0df69ca2) SHA1(49670347ebd7e1067ff988cf842b275b7ee7b5f7) )
	ROM_LOAD( "rom-3.bin", 0x400000, 0x400000, CRC(60167d38) SHA1(fd2429808c59ef51fd5f5db84ea89a8dc504186e) )
	ROM_LOAD( "rom-2.bin", 0x800000, 0x400000, CRC(1bfea593) SHA1(ce06dc3097ae56b0df56d104bbf7efc9b5d968d4) )
	ROM_LOAD( "rom-4.bin", 0xc00000, 0x400000, CRC(bee03c94) SHA1(5bc1e6769c42857c03456426b502fcb86a114f19) )

	ROM_REGION( 0x100000, "oki1", 0 )       /* ADPCM Samples 1 */
	ROM_LOAD( "rom-5.bin", 0x000000, 0x100000, CRC(4274daf6) SHA1(85557b4707d529e5914f03c7a856864f5c24950e) )

	ROM_REGION( 0x100000, "oki2", 0 )       /* ADPCM Samples 2 */
	ROM_LOAD( "rom-6.bin", 0x000000, 0x100000, CRC(2a1c2426) SHA1(8abc3688ffc5ebb94b8d5118d4fa0908f07fe791) )
ROM_END

/* older version, might have only been released in Japan, Hong Kong and Taiwan? */
ROM_START( batriderja )
	ROM_REGION( 0x200000, "maincpu", 0 )            /* Main 68k code */
	ROM_LOAD16_BYTE( "prg0.bin", 0x000000, 0x080000, CRC(f93ea27c) SHA1(41023c2ee1efd70b5aa9c70e1ddd9e5c3d51d68a) )
	ROM_LOAD16_BYTE( "prg1.u23", 0x000001, 0x080000, CRC(8ae7f592) SHA1(8a20ebf85eca621f578d2302c3a3988647b077a7) )
	ROM_LOAD16_BYTE( "prg2.u21", 0x100000, 0x080000, CRC(bdaa5fbf) SHA1(abd72ac633c0c8e7b4b1d7902c0d6e014ba995fe) )
	ROM_LOAD16_BYTE( "prg3.u24", 0x100001, 0x080000, CRC(7aa9f941) SHA1(99bdbad7a96d461073b06a53c50fc57c2fd6fc6d) )

	ROM_REGION( 0x40000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.u77", 0x00000, 0x40000, CRC(56682696) SHA1(a372450d9a6d535123dfc31d8116074b168ab646) )

	ROM_REGION( 0x1000000, "gp9001_0", 0 )
	ROM_LOAD( "rom-1.bin", 0x000000, 0x400000, CRC(0df69ca2) SHA1(49670347ebd7e1067ff988cf842b275b7ee7b5f7) )
	ROM_LOAD( "rom-3.bin", 0x400000, 0x400000, CRC(60167d38) SHA1(fd2429808c59ef51fd5f5db84ea89a8dc504186e) )
	ROM_LOAD( "rom-2.bin", 0x800000, 0x400000, CRC(1bfea593) SHA1(ce06dc3097ae56b0df56d104bbf7efc9b5d968d4) )
	ROM_LOAD( "rom-4.bin", 0xc00000, 0x400000, CRC(bee03c94) SHA1(5bc1e6769c42857c03456426b502fcb86a114f19) )

	ROM_REGION( 0x100000, "oki1", 0 )       /* ADPCM Samples 1 */
	ROM_LOAD( "rom-5.bin", 0x000000, 0x100000, CRC(4274daf6) SHA1(85557b4707d529e5914f03c7a856864f5c24950e) )

	ROM_REGION( 0x100000, "oki2", 0 )       /* ADPCM Samples 2 */
	ROM_LOAD( "rom-6.bin", 0x000000, 0x100000, CRC(2a1c2426) SHA1(8abc3688ffc5ebb94b8d5118d4fa0908f07fe791) )
ROM_END


ROM_START( batriderhk )
	ROM_REGION( 0x200000, "maincpu", 0 )            /* Main 68k code */
	ROM_LOAD16_BYTE( "prg0.u22", 0x000000, 0x080000, CRC(00afbb7c) SHA1(a4b6331e0fcab7d0c43fc43adb701f1248247b41) )
	ROM_LOAD16_BYTE( "prg1.u23", 0x000001, 0x080000, CRC(8ae7f592) SHA1(8a20ebf85eca621f578d2302c3a3988647b077a7) )
	ROM_LOAD16_BYTE( "prg2.u21", 0x100000, 0x080000, CRC(bdaa5fbf) SHA1(abd72ac633c0c8e7b4b1d7902c0d6e014ba995fe) )
	ROM_LOAD16_BYTE( "prg3.u24", 0x100001, 0x080000, CRC(7aa9f941) SHA1(99bdbad7a96d461073b06a53c50fc57c2fd6fc6d) )

	ROM_REGION( 0x40000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.u77", 0x00000, 0x40000, CRC(56682696) SHA1(a372450d9a6d535123dfc31d8116074b168ab646) )

	ROM_REGION( 0x1000000, "gp9001_0", 0 )
	ROM_LOAD( "rom-1.bin", 0x000000, 0x400000, CRC(0df69ca2) SHA1(49670347ebd7e1067ff988cf842b275b7ee7b5f7) )
	ROM_LOAD( "rom-3.bin", 0x400000, 0x400000, CRC(60167d38) SHA1(fd2429808c59ef51fd5f5db84ea89a8dc504186e) )
	ROM_LOAD( "rom-2.bin", 0x800000, 0x400000, CRC(1bfea593) SHA1(ce06dc3097ae56b0df56d104bbf7efc9b5d968d4) )
	ROM_LOAD( "rom-4.bin", 0xc00000, 0x400000, CRC(bee03c94) SHA1(5bc1e6769c42857c03456426b502fcb86a114f19) )

	ROM_REGION( 0x100000, "oki1", 0 )       /* ADPCM Samples 1 */
	ROM_LOAD( "rom-5.bin", 0x000000, 0x100000, CRC(4274daf6) SHA1(85557b4707d529e5914f03c7a856864f5c24950e) )

	ROM_REGION( 0x100000, "oki2", 0 )       /* ADPCM Samples 2 */
	ROM_LOAD( "rom-6.bin", 0x000000, 0x100000, CRC(2a1c2426) SHA1(8abc3688ffc5ebb94b8d5118d4fa0908f07fe791) )
ROM_END


ROM_START( batridert )
	ROM_REGION( 0x200000, "maincpu", 0 )            /* Main 68k code */
	ROM_LOAD16_BYTE( "u22.bin",  0x000000, 0x080000, CRC(b135820e) SHA1(c222887d18a0a3ea0fcc973b95b29d69c86f7ec3) )
	ROM_LOAD16_BYTE( "prg1.u23", 0x000001, 0x080000, CRC(8ae7f592) SHA1(8a20ebf85eca621f578d2302c3a3988647b077a7) )
	ROM_LOAD16_BYTE( "prg2.u21", 0x100000, 0x080000, CRC(bdaa5fbf) SHA1(abd72ac633c0c8e7b4b1d7902c0d6e014ba995fe) )
	ROM_LOAD16_BYTE( "prg3.u24", 0x100001, 0x080000, CRC(7aa9f941) SHA1(99bdbad7a96d461073b06a53c50fc57c2fd6fc6d) )

	ROM_REGION( 0x40000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.u77", 0x00000, 0x40000, CRC(56682696) SHA1(a372450d9a6d535123dfc31d8116074b168ab646) )

	ROM_REGION( 0x1000000, "gp9001_0", 0 )
	ROM_LOAD( "rom-1.bin", 0x000000, 0x400000, CRC(0df69ca2) SHA1(49670347ebd7e1067ff988cf842b275b7ee7b5f7) )
	ROM_LOAD( "rom-3.bin", 0x400000, 0x400000, CRC(60167d38) SHA1(fd2429808c59ef51fd5f5db84ea89a8dc504186e) )
	ROM_LOAD( "rom-2.bin", 0x800000, 0x400000, CRC(1bfea593) SHA1(ce06dc3097ae56b0df56d104bbf7efc9b5d968d4) )
	ROM_LOAD( "rom-4.bin", 0xc00000, 0x400000, CRC(bee03c94) SHA1(5bc1e6769c42857c03456426b502fcb86a114f19) )

	ROM_REGION( 0x100000, "oki1", 0 )       /* ADPCM Samples 1 */
	ROM_LOAD( "rom-5.bin", 0x000000, 0x100000, CRC(4274daf6) SHA1(85557b4707d529e5914f03c7a856864f5c24950e) )

	ROM_REGION( 0x100000, "oki2", 0 )       /* ADPCM Samples 2 */
	ROM_LOAD( "rom-6.bin", 0x000000, 0x100000, CRC(2a1c2426) SHA1(8abc3688ffc5ebb94b8d5118d4fa0908f07fe791) )
ROM_END



/*
Battle Bakraid
Raizing/8ing, 1999

PCB Layout
----------

ET68-V99
|-----------------------------------------------------|
|TA8201  16.93MHz     ROM-6                   6264    |
|  YAC516                                             |
|       YMZ280B-F     ROM-7               SND_U0720   |
|                                                     |
| VOL                 ROM-8                 Z80       |
|                                                     |
|                   341256                            |
|                                               93C66 |
|                   341256               XILINX       |
|J                                       XC95108      |
|A                  27MHz     32MHz                   |
|M                                                    |
|M          DIPSW1                      341256  341256|
|A                  XILINX    XILINK                  |
|           DIPSW2  XC95144   XC95108   341256  341256|
|                                                     |
|           DIPSW3                                    |
|                                MACH211    PRG1_U023 |
| TEST_SW            68000                            |
|                                           PRG0_U022 |
|                                                     |
|                                           PRG3_U024 |
|                             L7A0498                 |
|                             GP9001        PRG2_U021 |
| ROM-0       ROM-1           (QFP208)                |
|                                                     |
|                               6264       MN414260   |
| ROM-2       ROM-3                                   |
|                               6264       MN414260   |
|-----------------------------------------------------|
Notes:
      ROM-0 to ROM-3 - 32M DIP42
      ROM-6 to ROM-8 - 32M DIP42 Byte Mode
      68000 clock - 16.000MHz (32/2)
      Z80 clock - 5.33333MHz (32/6)
      VSync - 60Hz
      HSync - 15.39kHz
*/



ROM_START( bbakraid )
	ROM_REGION( 0x200000, "maincpu", 0 )            /* Main 68k code */
	ROM_LOAD16_BYTE( "prg0u022_usa.bin", 0x000000, 0x080000, CRC(95fb2ffd) SHA1(c7f502f3945249573b66226e8bacc6a9bc230693) )
	ROM_LOAD16_BYTE( "prg1u023.new", 0x000001, 0x080000, CRC(4ae9aa64) SHA1(45fdf72141c4c9f24a38d4218c65874799b9c868) )
	ROM_LOAD16_BYTE( "prg2u021.bin", 0x100000, 0x080000, CRC(ffba8656) SHA1(6526bb65fad3384de3f301a7d1095cbf03757433) )
	ROM_LOAD16_BYTE( "prg3u024.bin", 0x100001, 0x080000, CRC(834b8ad6) SHA1(0dd6223bb0749819ad29811eeb04fd08d937abb0) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "sndu0720.bin", 0x00000, 0x20000, CRC(e62ab246) SHA1(00d23689dd423ecd4024c58b5903d16e890f1dff) )

	ROM_REGION( 0x1000000, "gp9001_0", 0 )
	ROM_LOAD( "gfxu0510.bin", 0x000000, 0x400000, CRC(9cca3446) SHA1(1123f8b8bfbe59a2c572cdf61f1ad27ff37f0f0d) )
	ROM_LOAD( "gfxu0512.bin", 0x400000, 0x400000, CRC(a2a281d5) SHA1(d9a6623f9433ad682223f9780c26cd1523ebc5c5) )
	ROM_LOAD( "gfxu0511.bin", 0x800000, 0x400000, CRC(e16472c0) SHA1(6068d679a8b3b65e05acd58a7ce9ead90177049f) )
	ROM_LOAD( "gfxu0513.bin", 0xc00000, 0x400000, CRC(8bb635a0) SHA1(9064f1a2d8bb88ddbca702fb8556d0dfe6a5cadc) )

	ROM_REGION( 0x0c00000, "ymz", 0 )       /* YMZ280B Samples */
	ROM_LOAD( "rom6.829", 0x000000, 0x400000, CRC(8848b4a0) SHA1(e0dce136c5d5a4c1a92b863e57848cd5927d06f1) )
	ROM_LOAD( "rom7.830", 0x400000, 0x400000, CRC(d6224267) SHA1(5c9b7b13effbef9f707811f84bfe50ca85e605e3) )
	ROM_LOAD( "rom8.831", 0x800000, 0x400000, CRC(a101dfb0) SHA1(4b729b0d562e09df35438e9e6b457b8de2690a6e) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "eeprom-bbakraid-new.bin", 0x000, 0x200, CRC(35c9275a) SHA1(1282034adf3c7a24545fd273729867058dc93027) )
ROM_END


ROM_START( bbakraidc )
	ROM_REGION( 0x200000, "maincpu", 0 )            /* Main 68k code */
	ROM_LOAD16_BYTE( "prg0u022_china.bin", 0x000000, 0x080000, CRC(760be084) SHA1(096c8a2336492d7370ae25f3385faebf6e9c3eca) )
	ROM_LOAD16_BYTE( "prg1u023.new", 0x000001, 0x080000, CRC(4ae9aa64) SHA1(45fdf72141c4c9f24a38d4218c65874799b9c868) )
	ROM_LOAD16_BYTE( "prg2u021.bin", 0x100000, 0x080000, CRC(ffba8656) SHA1(6526bb65fad3384de3f301a7d1095cbf03757433) )
	ROM_LOAD16_BYTE( "prg3u024.bin", 0x100001, 0x080000, CRC(834b8ad6) SHA1(0dd6223bb0749819ad29811eeb04fd08d937abb0) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "sndu0720.bin", 0x00000, 0x20000, CRC(e62ab246) SHA1(00d23689dd423ecd4024c58b5903d16e890f1dff) )

	ROM_REGION( 0x1000000, "gp9001_0", 0 )
	ROM_LOAD( "gfxu0510.bin", 0x000000, 0x400000, CRC(9cca3446) SHA1(1123f8b8bfbe59a2c572cdf61f1ad27ff37f0f0d) )
	ROM_LOAD( "gfxu0512.bin", 0x400000, 0x400000, CRC(a2a281d5) SHA1(d9a6623f9433ad682223f9780c26cd1523ebc5c5) )
	ROM_LOAD( "gfxu0511.bin", 0x800000, 0x400000, CRC(e16472c0) SHA1(6068d679a8b3b65e05acd58a7ce9ead90177049f) )
	ROM_LOAD( "gfxu0513.bin", 0xc00000, 0x400000, CRC(8bb635a0) SHA1(9064f1a2d8bb88ddbca702fb8556d0dfe6a5cadc) )

	ROM_REGION( 0x0c00000, "ymz", 0 )       /* YMZ280B Samples */
	ROM_LOAD( "rom6.829", 0x000000, 0x400000, CRC(8848b4a0) SHA1(e0dce136c5d5a4c1a92b863e57848cd5927d06f1) )
	ROM_LOAD( "rom7.830", 0x400000, 0x400000, CRC(d6224267) SHA1(5c9b7b13effbef9f707811f84bfe50ca85e605e3) )
	ROM_LOAD( "rom8.831", 0x800000, 0x400000, CRC(a101dfb0) SHA1(4b729b0d562e09df35438e9e6b457b8de2690a6e) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "eeprom-bbakraid-new.bin", 0x000, 0x200, CRC(35c9275a) SHA1(1282034adf3c7a24545fd273729867058dc93027) )
ROM_END


ROM_START( bbakraidj )
	ROM_REGION( 0x200000, "maincpu", 0 )            /* Main 68k code */
	ROM_LOAD16_BYTE( "prg0u022.new", 0x000000, 0x080000, CRC(fa8d38d3) SHA1(aba91d87a8a62d3fe1139b4437b16e2f844264ad) )
	ROM_LOAD16_BYTE( "prg1u023.new", 0x000001, 0x080000, CRC(4ae9aa64) SHA1(45fdf72141c4c9f24a38d4218c65874799b9c868) )
	ROM_LOAD16_BYTE( "prg2u021.bin", 0x100000, 0x080000, CRC(ffba8656) SHA1(6526bb65fad3384de3f301a7d1095cbf03757433) )
	ROM_LOAD16_BYTE( "prg3u024.bin", 0x100001, 0x080000, CRC(834b8ad6) SHA1(0dd6223bb0749819ad29811eeb04fd08d937abb0) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "sndu0720.bin", 0x00000, 0x20000, CRC(e62ab246) SHA1(00d23689dd423ecd4024c58b5903d16e890f1dff) )

	ROM_REGION( 0x1000000, "gp9001_0", 0 )
	ROM_LOAD( "gfxu0510.bin", 0x000000, 0x400000, CRC(9cca3446) SHA1(1123f8b8bfbe59a2c572cdf61f1ad27ff37f0f0d) )
	ROM_LOAD( "gfxu0512.bin", 0x400000, 0x400000, CRC(a2a281d5) SHA1(d9a6623f9433ad682223f9780c26cd1523ebc5c5) )
	ROM_LOAD( "gfxu0511.bin", 0x800000, 0x400000, CRC(e16472c0) SHA1(6068d679a8b3b65e05acd58a7ce9ead90177049f) )
	ROM_LOAD( "gfxu0513.bin", 0xc00000, 0x400000, CRC(8bb635a0) SHA1(9064f1a2d8bb88ddbca702fb8556d0dfe6a5cadc) )

	ROM_REGION( 0x0c00000, "ymz", 0 )       /* YMZ280B Samples */
	ROM_LOAD( "rom6.829", 0x000000, 0x400000, CRC(8848b4a0) SHA1(e0dce136c5d5a4c1a92b863e57848cd5927d06f1) )
	ROM_LOAD( "rom7.830", 0x400000, 0x400000, CRC(d6224267) SHA1(5c9b7b13effbef9f707811f84bfe50ca85e605e3) )
	ROM_LOAD( "rom8.831", 0x800000, 0x400000, CRC(a101dfb0) SHA1(4b729b0d562e09df35438e9e6b457b8de2690a6e) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "eeprom-bbakraid-new.bin", 0x000, 0x200, CRC(35c9275a) SHA1(1282034adf3c7a24545fd273729867058dc93027) )
ROM_END


ROM_START( bbakraidja )
	ROM_REGION( 0x200000, "maincpu", 0 )            /* Main 68k code */
	ROM_LOAD16_BYTE( "prg0u022.bin", 0x000000, 0x080000, CRC(0dd59512) SHA1(c6a4e6aa49c6ac3b04ae62a0a4cc8084ae048381) )
	ROM_LOAD16_BYTE( "prg1u023.bin", 0x000001, 0x080000, CRC(fecde223) SHA1(eb5ac0eda49b4b0f3d25d8a8bb356e77a453d3a7) )
	ROM_LOAD16_BYTE( "prg2u021.bin", 0x100000, 0x080000, CRC(ffba8656) SHA1(6526bb65fad3384de3f301a7d1095cbf03757433) )
	ROM_LOAD16_BYTE( "prg3u024.bin", 0x100001, 0x080000, CRC(834b8ad6) SHA1(0dd6223bb0749819ad29811eeb04fd08d937abb0) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "sndu0720.bin", 0x00000, 0x20000, CRC(e62ab246) SHA1(00d23689dd423ecd4024c58b5903d16e890f1dff) )

	ROM_REGION( 0x1000000, "gp9001_0", 0 )
	ROM_LOAD( "gfxu0510.bin", 0x000000, 0x400000, CRC(9cca3446) SHA1(1123f8b8bfbe59a2c572cdf61f1ad27ff37f0f0d) )
	ROM_LOAD( "gfxu0512.bin", 0x400000, 0x400000, CRC(a2a281d5) SHA1(d9a6623f9433ad682223f9780c26cd1523ebc5c5) )
	ROM_LOAD( "gfxu0511.bin", 0x800000, 0x400000, CRC(e16472c0) SHA1(6068d679a8b3b65e05acd58a7ce9ead90177049f) )
	ROM_LOAD( "gfxu0513.bin", 0xc00000, 0x400000, CRC(8bb635a0) SHA1(9064f1a2d8bb88ddbca702fb8556d0dfe6a5cadc) )

	ROM_REGION( 0x0c00000, "ymz", 0 )       /* YMZ280B Samples */
	ROM_LOAD( "rom6.829", 0x000000, 0x400000, CRC(8848b4a0) SHA1(e0dce136c5d5a4c1a92b863e57848cd5927d06f1) )
	ROM_LOAD( "rom7.830", 0x400000, 0x400000, CRC(d6224267) SHA1(5c9b7b13effbef9f707811f84bfe50ca85e605e3) )
	ROM_LOAD( "rom8.831", 0x800000, 0x400000, CRC(a101dfb0) SHA1(4b729b0d562e09df35438e9e6b457b8de2690a6e) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "eeprom-bbakraid.bin", 0x000, 0x200, CRC(7f97d347) SHA1(3096c399019924dbb7d6673483f6a011f89467c6) )
ROM_END


// dedicated PCB marked Pro Bowl
ROM_START( nprobowl )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "newprobowl-prg0-u021", 0x000000, 0x080000, CRC(3a57f122) SHA1(cc361c295f23bc0479ba49eb15de2ec6ca535a56) ) // 11xxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "newprobowl-prg1-u024", 0x000001, 0x080000, CRC(9e9bb58a) SHA1(3d2159bde418dee5d89e3df9a248b4b1989e6ee9) ) // 11xxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200000, "gp9001_0", 0 )
	ROM_LOAD16_BYTE( "newprobowl-chr1-u0519", 0x000000, 0x80000, CRC(6700a9bf) SHA1(12a72aa0b91119fbbed994aec702a6869af6f287) )
	ROM_LOAD16_BYTE( "newprobowl-chr0-u0518", 0x000001, 0x80000, CRC(0736cccd) SHA1(5a4b691be1df697fef3847456c0f4bb3466c403f) )
	ROM_LOAD16_BYTE( "newprobowl-chr2-u0520", 0x100000, 0x80000, CRC(e5f6d0b6) SHA1(6e1a4792698be4b478118e8c82edb0cf3e2286f2) )
	ROM_LOAD16_BYTE( "newprobowl-chr3-u0521", 0x100001, 0x80000, CRC(00c21951) SHA1(922abde172fb82b504dce41b95227740f16208a7) )

	ROM_REGION( 0x100000, "oki1", 0 )
	ROM_LOAD( "newprobowl-adpcm0-u0834", 0x00000, 0x80000, CRC(3b40b161) SHA1(ff8ba38dd7e0dadbf72810470e3d9afb1cd983d2) )
	ROM_LOAD( "newprobowl-adpcm1-u0835", 0x80000, 0x80000, CRC(8c191e60) SHA1(f81c2849ffc553d921fc680cd50c2997b834c44a) )
ROM_END

ROM_START( probowl2 ) // identical to New Pro Bowl but for slight mods to the GFX ROMs to display the different title
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "probowl2.prg0.u021", 0x000000, 0x080000, CRC(3a57f122) SHA1(cc361c295f23bc0479ba49eb15de2ec6ca535a56) ) // 11xxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "probowl2.prg1.u024", 0x000001, 0x080000, CRC(9e9bb58a) SHA1(3d2159bde418dee5d89e3df9a248b4b1989e6ee9) ) // 11xxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200000, "gp9001_0", 0 )
	ROM_LOAD16_BYTE( "probowl2.chr1.u0519", 0x000000, 0x80000, CRC(66a5c9cd) SHA1(b8edc6a66e3929af2a1399e5fb6fc341b9789136) )
	ROM_LOAD16_BYTE( "probowl2.chr0.u0518", 0x000001, 0x80000, CRC(b4439673) SHA1(5ca19d9c3c3cce9485fb9bd5224f0b14abe07f49) )
	ROM_LOAD16_BYTE( "probowl2.chr2.u0520", 0x100000, 0x80000, CRC(e05be3a0) SHA1(922c6f094358cf3df790063e68d5dd4a32d46b06) )
	ROM_LOAD16_BYTE( "probowl2.chr3.u0521", 0x100001, 0x80000, CRC(55e2565f) SHA1(b6a570b736a9b30e26d2a59b53f218ef5cd6f0f6) )

	ROM_REGION( 0x100000, "oki1", 0 )
	ROM_LOAD( "probowl2.adpcm0.u0834", 0x00000, 0x80000, CRC(be2fb43f) SHA1(f3f9bc522f4668852b751f291cef0000bb86b779) )
	ROM_LOAD( "probowl2.adpcm1.u0835", 0x80000, 0x80000, CRC(8c191e60) SHA1(f81c2849ffc553d921fc680cd50c2997b834c44a) )
ROM_END


// The following is in order of Toaplan Board/game numbers
// See list at top of file

//  ( YEAR  NAME         PARENT    MACHINE       INPUT       STATE           INIT           MONITOR COMPANY            FULLNAME                     FLAGS )
GAME( 1991, tekipaki,    0,        tekipaki,     tekipaki,   toaplan2_state, empty_init,    ROT0,   "Toaplan",         "Teki Paki",                 MACHINE_SUPPORTS_SAVE )
GAME( 1991, tekipakit,   tekipaki, tekipaki,     tekipaki,   toaplan2_state, empty_init,    ROT0,   "Toaplan",         "Teki Paki (location test)", MACHINE_SUPPORTS_SAVE )

GAME( 1991, ghox,        0,        ghox,         ghox,       ghox_state,     empty_init,    ROT270, "Toaplan",         "Ghox (spinner)",            MACHINE_SUPPORTS_SAVE )
GAME( 1991, ghoxj,       ghox,     ghox,         ghox,       ghox_state,     empty_init,    ROT270, "Toaplan",         "Ghox (joystick)",           MACHINE_SUPPORTS_SAVE )
GAME( 1991, ghoxjo,      ghox,     ghox,         ghoxjo,     ghox_state,     empty_init,    ROT270, "Toaplan",         "Ghox (joystick, older)",    MACHINE_SUPPORTS_SAVE )

GAME( 1992, dogyuun,     0,        dogyuun,      dogyuun,    toaplan2_state, init_dogyuun,  ROT270, "Toaplan",         "Dogyuun",                           MACHINE_SUPPORTS_SAVE )
GAME( 1992, dogyuuna,    dogyuun,  dogyuun,      dogyuuna,   toaplan2_state, init_dogyuun,  ROT270, "Toaplan",         "Dogyuun (older set)",               MACHINE_SUPPORTS_SAVE )
GAME( 1992, dogyuunb,    dogyuun,  dogyuun,      dogyuunt,   toaplan2_state, init_dogyuun,  ROT270, "Toaplan",         "Dogyuun (oldest set)",              MACHINE_SUPPORTS_SAVE ) // maybe a newer location test version, instead
GAME( 1992, dogyuunt,    dogyuun,  dogyuun,      dogyuunt,   toaplan2_state, init_dogyuun,  ROT270, "Toaplan",         "Dogyuun (10/9/1992 location test)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, dogyuunto,   dogyuun,  dogyuunto,    dogyuunt,   toaplan2_state, init_vfive,    ROT270, "Toaplan",         "Dogyuun (8/25/1992 location test)", MACHINE_SUPPORTS_SAVE )

GAME( 1993, kbash,       0,        kbash,        kbash,      toaplan2_state, empty_init,    ROT0,   "Toaplan / Atari", "Knuckle Bash",                 MACHINE_SUPPORTS_SAVE ) // Atari license shown for some regions.
GAME( 1993, kbashk,      kbash,    kbash,        kbashk,     toaplan2_state, empty_init,    ROT0,   "Toaplan / Taito", "Knuckle Bash (Korean PCB)",    MACHINE_SUPPORTS_SAVE ) // Japan region has optional Taito license, maybe the original Japan release?
GAME( 1993, kbashp,      kbash,    kbash,        kbash,      toaplan2_state, empty_init,    ROT0,   "Toaplan / Taito", "Knuckle Bash (location test)", MACHINE_SUPPORTS_SAVE )

GAME( 1999, kbash2,      0,        kbash2,       kbash2,     toaplan2_state, empty_init,    ROT0,   "bootleg",         "Knuckle Bash 2 (bootleg)",  MACHINE_SUPPORTS_SAVE )

GAME( 1992, truxton2,    0,        truxton2,     truxton2,   truxton2_state, empty_init,    ROT270, "Toaplan",         "Truxton II / Tatsujin Oh",  MACHINE_SUPPORTS_SAVE )

GAME( 1991, pipibibs,    0,        pipibibs,     pipibibs,   toaplan2_state, empty_init,    ROT0,   "Toaplan",         "Pipi & Bibis / Whoopee!! (Z80 sound cpu, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, pipibibsa,   pipibibs, pipibibs,     pipibibs,   toaplan2_state, empty_init,    ROT0,   "Toaplan",         "Pipi & Bibis / Whoopee!! (Z80 sound cpu, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, pipibibsp,   pipibibs, pipibibs,     pipibibsp,  toaplan2_state, empty_init,    ROT0,   "Toaplan",         "Pipi & Bibis / Whoopee!! (prototype)",            MACHINE_SUPPORTS_SAVE )
GAME( 1991, whoopee,     pipibibs, tekipaki,     whoopee,    toaplan2_state, empty_init,    ROT0,   "Toaplan",         "Pipi & Bibis / Whoopee!! (Teki Paki hardware)",   MACHINE_SUPPORTS_SAVE ) // original Whoopee!! boards have a HD647180 instead of Z80

GAME( 1991, pipibibsbl,  pipibibs, pipibibsbl,   pipibibsbl, toaplan2_state, init_pipibibsbl, ROT0, "bootleg (Ryouta Kikaku)", "Pipi & Bibis / Whoopee!! (Ryouta Kikaku bootleg, encrypted)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, pipibibsbl2, pipibibs, pipibibsbl,   pipibibsbl, toaplan2_state, empty_init,    ROT0,   "bootleg",                 "Pipi & Bibis / Whoopee!! (bootleg, decrypted)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // different memory map, not scrambled
GAME( 1991, pipibibsbl3, pipibibs, pipibibsbl,   pipibibsbl, toaplan2_state, empty_init,    ROT0,   "bootleg (Ryouta Kikaku)", "Pipi & Bibis / Whoopee!! (Ryouta Kikaku bootleg, decrypted)", MACHINE_SUPPORTS_SAVE )

GAME( 1993, enmadaio,    0,        enmadaio,     enmadaio,   toaplan2_state, init_enmadaio, ROT0,   "Toaplan / Taito",  "Enma Daio (Japan)", 0 ) // TP-031

// region is in eeprom (and also requires correct return value from a v25 mapped address??)
GAME( 1992, fixeight,    0,        fixeight,   fixeight,   truxton2_state, init_fixeight,   ROT270, "Toaplan",                 "FixEight (Europe)",                                          MACHINE_SUPPORTS_SAVE )
GAME( 1992, fixeightk,   fixeight, fixeight,   fixeight,   truxton2_state, init_fixeight,   ROT270, "Toaplan",                 "FixEight (Korea)",                                           MACHINE_SUPPORTS_SAVE )
GAME( 1992, fixeighth,   fixeight, fixeight,   fixeight,   truxton2_state, init_fixeight,   ROT270, "Toaplan",                 "FixEight (Hong Kong)",                                       MACHINE_SUPPORTS_SAVE )
GAME( 1992, fixeighttw,  fixeight, fixeight,   fixeight,   truxton2_state, init_fixeight,   ROT270, "Toaplan",                 "FixEight (Taiwan)",                                          MACHINE_SUPPORTS_SAVE )
GAME( 1992, fixeighta,   fixeight, fixeight,   fixeight,   truxton2_state, init_fixeight,   ROT270, "Toaplan",                 "FixEight (Southeast Asia)",                                  MACHINE_SUPPORTS_SAVE )
GAME( 1992, fixeightu,   fixeight, fixeight,   fixeight,   truxton2_state, init_fixeight,   ROT270, "Toaplan",                 "FixEight (USA)",                                             MACHINE_SUPPORTS_SAVE )
GAME( 1992, fixeightj,   fixeight, fixeight,   fixeight,   truxton2_state, init_fixeight,   ROT270, "Toaplan",                 "FixEight - Jigoku no Eiyuu Densetsu (Japan)",                MACHINE_SUPPORTS_SAVE )
GAME( 1992, fixeightt,   fixeight, fixeight,   fixeight,   truxton2_state, init_fixeight,   ROT270, "Toaplan (Taito license)", "FixEight (Europe, Taito license)",                           MACHINE_SUPPORTS_SAVE )
GAME( 1992, fixeightkt,  fixeight, fixeight,   fixeight,   truxton2_state, init_fixeight,   ROT270, "Toaplan (Taito license)", "FixEight (Korea, Taito license)",                            MACHINE_SUPPORTS_SAVE )
GAME( 1992, fixeightht,  fixeight, fixeight,   fixeight,   truxton2_state, init_fixeight,   ROT270, "Toaplan (Taito license)", "FixEight (Hong Kong, Taito license)",                        MACHINE_SUPPORTS_SAVE )
GAME( 1992, fixeighttwt, fixeight, fixeight,   fixeight,   truxton2_state, init_fixeight,   ROT270, "Toaplan (Taito license)", "FixEight (Taiwan, Taito license)",                           MACHINE_SUPPORTS_SAVE )
GAME( 1992, fixeightat,  fixeight, fixeight,   fixeight,   truxton2_state, init_fixeight,   ROT270, "Toaplan (Taito license)", "FixEight (Southeast Asia, Taito license)",                   MACHINE_SUPPORTS_SAVE )
GAME( 1992, fixeightut,  fixeight, fixeight,   fixeight,   truxton2_state, init_fixeight,   ROT270, "Toaplan (Taito license)", "FixEight (USA, Taito license)",                              MACHINE_SUPPORTS_SAVE )
GAME( 1992, fixeightjt,  fixeight, fixeight,   fixeight,   truxton2_state, init_fixeight,   ROT270, "Toaplan (Taito license)", "FixEight - Jigoku no Eiyuu Densetsu (Japan, Taito license)", MACHINE_SUPPORTS_SAVE )

GAME( 1992, fixeightbl,  fixeight, fixeightbl, fixeightbl, truxton2_state, init_fixeightbl, ROT270, "bootleg", "FixEight (Korea, bootleg)", MACHINE_SUPPORTS_SAVE )

GAME( 1992, grindstm,    0,        vfive,      grindstm,   toaplan2_state, init_vfive,      ROT270, "Toaplan", "Grind Stormer",             MACHINE_SUPPORTS_SAVE )
GAME( 1992, grindstma,   grindstm, vfive,      grindstma,  toaplan2_state, init_vfive,      ROT270, "Toaplan", "Grind Stormer (older set)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, vfive,       grindstm, vfive,      vfive,      toaplan2_state, init_vfive,      ROT270, "Toaplan", "V-Five (Japan)",            MACHINE_SUPPORTS_SAVE )

GAME( 1993, batsugun,    0,        batsugun,   batsugun,   toaplan2_state, init_dogyuun,    ROT270, "Toaplan", "Batsugun", MACHINE_SUPPORTS_SAVE )
GAME( 1993, batsuguna,   batsugun, batsugun,   batsugun,   toaplan2_state, init_dogyuun,    ROT270, "Toaplan", "Batsugun (older, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, batsugunc,   batsugun, batsugun,   batsugun,   toaplan2_state, init_dogyuun,    ROT270, "Toaplan", "Batsugun (older, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, batsugunb,   batsugun, batsugun,   batsugun,   toaplan2_state, init_dogyuun,    ROT270, "Toaplan", "Batsugun (Korean PCB)", MACHINE_SUPPORTS_SAVE ) // cheap looking PCB (same 'TP-030' numbering as original) but without Mask ROMs.  Still has original customs etc.  Jumpers were set to the Korea Unite Trading license, so likely made in Korea, not a bootleg tho.
GAME( 1993, batsugunsp,  batsugun, batsugun,   batsugun,   toaplan2_state, init_dogyuun,    ROT270, "Toaplan", "Batsugun - Special Version", MACHINE_SUPPORTS_SAVE )
GAME( 1993, batsugunbl,  batsugun, batsugunbl, batsugunbl, toaplan2_state, init_fixeightbl, ROT270, "Toaplan", "Batsugun (bootleg)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // needs correct GFX offsets and oki banking fix

GAME( 1994, pwrkick,     0,        pwrkick,    pwrkick,    pwrkick_state,  empty_init,      ROT0,   "Sunwise",  "Power Kick (Japan)",    0 )
GAME( 1995, burgkids,    0,        pwrkick,    burgkids,   pwrkick_state,  empty_init,      ROT0,   "Sunwise",  "Burger Kids (Japan)",   0 )
GAME( 1995, othldrby,    0,        othldrby,   othldrby,   pwrkick_state,  empty_init,      ROT0,   "Sunwise",  "Othello Derby (Japan)", 0 )

GAME( 1994, snowbro2,    0,        snowbro2,   snowbro2,   toaplan2_state, empty_init,      ROT0,   "Hanafram",         "Snow Bros. 2 - With New Elves / Otenki Paradise (Hanafram)",       MACHINE_SUPPORTS_SAVE )
GAME( 1994, snowbro2ny,  snowbro2, snowbro2,   snowbro2,   toaplan2_state, empty_init,      ROT0,   "Nyanko",           "Snow Bros. 2 - With New Elves / Otenki Paradise (Nyanko)",         MACHINE_SUPPORTS_SAVE ) // not a bootleg, has original parts (the "GP9001 L7A0498 TOA PLAN" IC and the three mask ROMs)
GAME( 1998, snowbro2b,   snowbro2, snowbro2,   snowbro2,   toaplan2_state, empty_init,      ROT0,   "bootleg",          "Snow Bros. 2 - With New Elves / Otenki Paradise (bootleg, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, snowbro2b2,  snowbro2, snowbro2,   snowbro2,   toaplan2_state, empty_init,      ROT0,   "bootleg (Q Elec)", "Snow Bros. 2 - With New Elves / Otenki Paradise (bootleg, set 2)", MACHINE_SUPPORTS_SAVE ) // possibly not a bootleg, has some original parts
GAME( 1994, snowbro2b3,  snowbro2, snowbro2b3, snowbro2b3, toaplan2_state, empty_init,      ROT0,   "bootleg",          "Snow Bros. 2 - With New Elves / Otenki Paradise (bootleg, set 3)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // GFX offsets not 100% correct

GAME( 1993, sstriker,    0,        mahoudai,   sstriker,   truxton2_state, empty_init,      ROT270, "Raizing",                         "Sorcer Striker",           MACHINE_SUPPORTS_SAVE ) // verified on two different PCBs
GAME( 1993, sstrikerk,   sstriker, mahoudai,   sstrikerk,  truxton2_state, empty_init,      ROT270, "Raizing (Unite Trading license)", "Sorcer Striker (Korea)",   MACHINE_SUPPORTS_SAVE ) // Although the region jumper is functional, it's a Korean board / version
GAME( 1993, mahoudai,    sstriker, mahoudai,   mahoudai,   truxton2_state, empty_init,      ROT270, "Raizing (Able license)",          "Mahou Daisakusen (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1994, kingdmgp,    0,        shippumd,   kingdmgp,   truxton2_state, empty_init,      ROT270, "Raizing / Eighting", "Kingdom Grandprix",               MACHINE_SUPPORTS_SAVE ) // from Korean board, missing letters on credits screen but this is correct
GAME( 1994, shippumd,    kingdmgp, shippumd,   shippumd,   truxton2_state, empty_init,      ROT270, "Raizing / Eighting", "Shippu Mahou Daisakusen (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1996, bgaregga,    0,        bgaregga,   bgaregga,   truxton2_state, init_bgaregga,   ROT270, "Raizing / Eighting", "Battle Garegga (Europe / USA / Japan / Asia) (Sat Feb 3 1996)",            MACHINE_SUPPORTS_SAVE )
GAME( 1996, bgareggat,   bgaregga, bgaregga,   bgaregga,   truxton2_state, init_bgaregga,   ROT270, "Raizing / Eighting", "Battle Garegga (location test) (Wed Jan 17 1996)",                         MACHINE_SUPPORTS_SAVE )
GAME( 1996, bgareggahk,  bgaregga, bgaregga,   bgareggahk, truxton2_state, init_bgaregga,   ROT270, "Raizing / Eighting", "Battle Garegga (Austria / Hong Kong) (Sat Feb 3 1996)",                    MACHINE_SUPPORTS_SAVE )
GAME( 1996, bgareggatw,  bgaregga, bgaregga,   bgareggatw, truxton2_state, init_bgaregga,   ROT270, "Raizing / Eighting", "Battle Garegga (Taiwan / Germany) (Thu Feb 1 1996)",                       MACHINE_SUPPORTS_SAVE )
GAME( 1996, bgareggak,   bgaregga, bgaregga,   bgareggak,  truxton2_state, init_bgaregga,   ROT270, "Raizing / Eighting", "Battle Garegga (Korea / Greece) (Wed Feb 7 1996)",                         MACHINE_SUPPORTS_SAVE )
GAME( 1996, bgaregganv,  bgaregga, bgaregga,   bgareggahk, truxton2_state, init_bgaregga,   ROT270, "Raizing / Eighting", "Battle Garegga - New Version (Austria / Hong Kong) (Sat Mar 2 1996)",      MACHINE_SUPPORTS_SAVE ) // displays New Version only when set to HK
GAME( 1996, bgareggat2,  bgaregga, bgaregga,   bgaregga,   truxton2_state, init_bgaregga,   ROT270, "Raizing / Eighting", "Battle Garegga - Type 2 (Europe / USA / Japan / Asia) (Sat Mar 2 1996)",   MACHINE_SUPPORTS_SAVE ) // displays Type 2 only when set to Europe
GAME( 1996, bgareggacn,  bgaregga, bgaregga,   bgareggacn, truxton2_state, init_bgaregga,   ROT270, "Raizing / Eighting", "Battle Garegga - Type 2 (Denmark / China) (Tue Apr 2 1996)",               MACHINE_SUPPORTS_SAVE ) // displays Type 2 only when set to Denmark
GAME( 1998, bgareggabl,  bgaregga, bgareggabl, bgareggabl, truxton2_state, init_bgaregga,   ROT270, "bootleg (Melody)",   "1945 Er Dai / 1945 Part-2 (Chinese hack of Battle Garegga)",               MACHINE_SUPPORTS_SAVE ) // based on Thu Feb 1 1996 set, Region hardcoded to China
GAME( 1997, bgareggabla, bgaregga, bgareggabl, bgareggabl, truxton2_state, init_bgaregga,   ROT270, "bootleg (Melody)",   "Leishen Chuan / Thunder Deity Biography (Chinese hack of Battle Garegga)", MACHINE_SUPPORTS_SAVE ) // based on Thu Feb 1 1996 set, Region hardcoded to Asia
GAME( 1996, bgareggablj, bgaregga, bgareggabl, bgareggabl, truxton2_state, init_bgaregga,   ROT270, "bootleg",            "Battle Garegga (Japan, bootleg) (Sat Feb 3 1996)",                         MACHINE_SUPPORTS_SAVE )

// these are all based on Version B, even if only the Japan version states 'version B'
GAME( 1998, batrider,    0,        batrider,   batrider,   truxton2_state, init_batrider,   ROT270, "Raizing / Eighting", "Armed Police Batrider (Europe) (Fri Feb 13 1998)",           MACHINE_SUPPORTS_SAVE )
GAME( 1998, batrideru,   batrider, batrider,   batrider,   truxton2_state, init_batrider,   ROT270, "Raizing / Eighting", "Armed Police Batrider (USA) (Fri Feb 13 1998)",              MACHINE_SUPPORTS_SAVE )
GAME( 1998, batriderc,   batrider, batrider,   batrider,   truxton2_state, init_batrider,   ROT270, "Raizing / Eighting", "Armed Police Batrider (China) (Fri Feb 13 1998)",            MACHINE_SUPPORTS_SAVE )
GAME( 1998, batriderj,   batrider, batrider,   batriderj,  truxton2_state, init_batrider,   ROT270, "Raizing / Eighting", "Armed Police Batrider (Japan, B version) (Fri Feb 13 1998)", MACHINE_SUPPORTS_SAVE )
GAME( 1998, batriderk,   batrider, batrider,   batrider,   truxton2_state, init_batrider,   ROT270, "Raizing / Eighting", "Armed Police Batrider (Korea) (Fri Feb 13 1998)",            MACHINE_SUPPORTS_SAVE )
// older revision of the code
GAME( 1998, batriderja,  batrider, batrider,   batriderj,  truxton2_state, init_batrider,   ROT270, "Raizing / Eighting", "Armed Police Batrider (Japan, older version) (Mon Dec 22 1997)", MACHINE_SUPPORTS_SAVE )
GAME( 1998, batriderhk,  batrider, batrider,   batrider,   truxton2_state, init_batrider,   ROT270, "Raizing / Eighting", "Armed Police Batrider (Hong Kong) (Mon Dec 22 1997)",            MACHINE_SUPPORTS_SAVE )
GAME( 1998, batridert,   batrider, batrider,   batrider,   truxton2_state, init_batrider,   ROT270, "Raizing / Eighting", "Armed Police Batrider (Taiwan) (Mon Dec 22 1997)",               MACHINE_SUPPORTS_SAVE )

// Battle Bakraid
// the 'unlimited' version is a newer revision of the code
GAME( 1999, bbakraid,    0,        bbakraid,   bbakraid,   truxton2_state, init_bbakraid,   ROT270, "Eighting", "Battle Bakraid - Unlimited Version (USA) (Tue Jun 8 1999)",   MACHINE_SUPPORTS_SAVE )
GAME( 1999, bbakraidc,   bbakraid, bbakraid,   bbakraid,   truxton2_state, init_bbakraid,   ROT270, "Eighting", "Battle Bakraid - Unlimited Version (China) (Tue Jun 8 1999)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, bbakraidj,   bbakraid, bbakraid,   bbakraid,   truxton2_state, init_bbakraid,   ROT270, "Eighting", "Battle Bakraid - Unlimited Version (Japan) (Tue Jun 8 1999)", MACHINE_SUPPORTS_SAVE )
// older revision of the code
GAME( 1999, bbakraidja,  bbakraid, bbakraid,   bbakraid,   truxton2_state, init_bbakraid,   ROT270, "Eighting", "Battle Bakraid (Japan) (Wed Apr 7 1999)", MACHINE_SUPPORTS_SAVE )

// dedicated PCB
GAME( 1996, nprobowl,    0,        nprobowl,   nprobowl,   truxton2_state, empty_init,      ROT0,   "Zuck / Able Corp", "New Pro Bowl", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE ) // bad GFXs, no sound banking, controls, etc
GAME( 1996, probowl2,    nprobowl, nprobowl,   nprobowl,   truxton2_state, empty_init,      ROT0,   "Zuck / Able Corp", "Pro Bowl 2",   MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE ) // bad GFXs, no sound banking, controls, etc
