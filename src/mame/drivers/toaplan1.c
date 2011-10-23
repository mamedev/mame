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
    hellfir1    B90         HellFire (1 Player, ealier version) Uses Taito rom ID number
    hellfir2    B90         HellFire (2 Player, ealier version) Uses Taito rom ID number
    hellfir3    B90         HellFire (1 Player version) Uses Taito rom ID number
    zerowing    TP-O15      Zero Wing
    zerowing2   TP-O15      Zero Wing (2 player simultaneous version, Williams Electronics Games, Inc)
    demonwld    TP-O16      Demon's World/Horror Story [1990]
    demonwl1    TP-O16      Demon's World/Horror Story [1989] (Taito license)
    demonwl2    TP-O16      Demon's World/Horror Story [1989] (early edition)
    demonwl3    TP-O16      Demon's World/Horror Story [1989] (first edition)
    fireshrk    TP-O17      Fire Shark (World)           [1990]
    samesame    TP-O17      Same! Same! Same! (Japan)    [1989] (1 Player version)
    samesam2    TP-O17      Same! Same! Same! (Japan)    [1989] (2 Player version)
    outzone     TP-O18      Out Zone
    outzonea    TP-O18      Out Zone (From board serial number 2122)
    vimana      TP-O19      Vimana (From board serial number 1547.04 [July '94])
    vimana1     TP-O19      Vimana (Older version)
    vimanan     TP-O19      Vimana (Nova Apparate GMBH & Co  license)


Notes:
    Fire Shark and Same! Same! Same! have a hidden function for the
    service input. When invulnerability is enabled, pressing the
    service input makes the screen scroll faster.

    OutZone (set 2) has a bug in the 68K code. An Jump instruction at $3E6
    goes to to an invalid instruction at $13DA4. It should really jump to
    $13DAA. This bad jump is executed by flicking the 'Service DSW' while
    after the game has booted. The other Outzone set correctly goes to
    service mode, but this set just loses the plot.

    OutZone (set 2) uses different enemies in some stages and has extra
    bonuses compared to set 1. The music sequences are also in different
    orders between the sets. So which is the newest version ?

    Demonwld (Toaplan copyright) is a newer version, and has a different game
    level sequence compared to the Taito licensed version.


Stephh's notes (based on the games M68000 and Z80 code and some tests) :

1) 'rallybik'

  - Region read from DSWB (port 0x50 in CPU1) then stored at 0x8004 (CPU1 shared RAM) =
    0x180008.w (CPU0 shared RAM) then stored at 0x0804f4.w .
  - Coinage relies on bits 4 and 5 of the region (code at 0x0ccc in CPU1) :
      * ..10.... : TOAPLAN_COINAGE_EUROPE (tables at 0x0c35 (COIN1) and 0x0c3d (COIN2) in CPU1)
      *  else    : TOAPLAN_COINAGE_JAPAN  (table at 0x0c25 (COIN1 AND COIN2) in CPU1)
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
    Don't forget to turn the Debug Switch OFF when time is over on bonus stage,
    or the level will never end !
  - When cabinet is set to "Upright", you can use joystick and buttons from both players
    (code at 0x001c44).

2) 'truxton'

  - Region read from Territory Jumper (port 0x70 in CPU1) then stored at 0x8005 (CPU1 shared RAM) =
    0x18000a.w (CPU0 shared RAM) then stored at 0x081b7c.w .
  - Coinage relies on bits 0 and 1 of the region (code at 0x0ccc in CPU1) :
      * ......00 : TOAPLAN_COINAGE_JAPAN  (table at 0x0d21 (COIN1 AND COIN2) in CPU1)
      * ......01 : TOAPLAN_COINAGE_JAPAN  (table at 0x0d29 (COIN1 AND COIN2) in CPU1)
      * ......10 : TOAPLAN_COINAGE_EUROPE (tables at 0x0d31 (COIN1) and 0x0d39 (COIN2) in CPU1)
      * ......11 : TOAPLAN_COINAGE_JAPAN  (table at 0x0d21 (COIN1 AND COIN2) in CPU1)
  - Title screen relies on bits 0 to 2 of the region (code at 0x002c58) :
      * .....000 : "Tatsujin"
      *     else : "Truxton"
  - Notice screen relies on bits 0 to 2 of the region (code at 0x004eb0) :
      * .....000 : "FOR USE IN JAPAN ONLY"
      * ......01 : "FOR USE IN U.S.A. ONLY"
      * ......1. : no notice screen
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
    So when territory is set to "USA/Taito America" (0x04), it will display "FOR JAPAN" !
    Jumpers 3 and 4 status is updated but they are always listed as unused.
  - To enter the "test mode", press START1 when the grid is displayed.
  - The "TEST" switch has the same effect as the "Service Mode" Dip Switch (DSWA bit 2).
  - When cabinet is set to "Upright", you can use joystick and buttons from both players
    (code at 0x002856).


3) 'hellfire' and "clones"

    TO DO !


4) 'zerowing' and "clones"

4a) 'zerowing'

  - Region read from Territory Jumper (port 0x70 in CPU1) then stored at 0x8005 (CPU1 shared RAM) =
    0x44000a.w (CPU0 shared RAM) then stored at 0x081810.w .
  - Coinage relies on bits 0 and 1 of the region (code at 0x0c59 in CPU1) :
      * ......00 : TOAPLAN_COINAGE_JAPAN  (table at 0x0cae (COIN1 AND COIN2) in CPU1)
      * ......01 : TOAPLAN_COINAGE_JAPAN  (table at 0x0cb6 (COIN1 AND COIN2) in CPU1)
      * ......1. : TOAPLAN_COINAGE_EUROPE (tables at 0x0cbe (COIN1) and 0x0cb6 (COIN2) in CPU1)
  - Notice screen relies on bit 0 of the region (code at 0x000564) :
      * .......0 : "FOR USE IN JAPAN ONLY"
      * .......1 : "FOR USE IN U.S.A. ONLY"
    But this routine is only called if both bits 0 and 1 of the region are set to 0
    (code at 0x000530), so there is a notice screen only when "Territory" is set to "Japan".
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
      * ......00 : "FOR JAPAN."
      * ......01 : "FOR U.S.A."
      * ......1. : "FOR EUROPE"
  - When "Invulnerability" Dip Switch is ON, you can do the following with the STARTn buttons :
      * press START2 to pause game
      * press START1 to unpause game
      * when START1 and START2 are pressed, the game enters in "slow motion" mode
  - When "Invulnerability" Dip Switch is ON, you can't die (of course), but you also can't move
    nor shoot while "captured" by an enemy or the background ! So you have to wait until enemy
    gives up or background scrolls enough to "free" you.
  - When cabinet is set to "Upright", you can use joystick and buttons from both players
    (code at 0x00541a).

4b) 'zerowing2'

  - Region read from Territory Jumper (port 0x70 in CPU1) then stored at 0x8005 (CPU1 shared RAM) =
    0x44000a.w (CPU0 shared RAM) then stored at 0x081ae2.w .
  - Same sound CPU as in 'zerowing', so same coinage infos.
  - Notice screen relies on bit 0 of the region (code at 0x0005f4) :
      * .......0 : "FOR USE IN JAPAN ONLY"
      * .......1 : "FOR USE IN U.S.A. ONLY"
    But this routine is only called if bit 1 of the region is set to 0 (code at 0x0005c2),
    so there shall be a notice screen only when "Territory" is not set to "Europe".
    Furthermore, because of the 'bra' instruction at 0x00059e, there is never a notice screen !
  - Copyright does NOT rely on the region, it is hard-coded in the M68000 ROMS.
    It is different from the one in 'zerowing' though.
  - Number of letters for initials relies on bit 1 of the region in the init routine (code at 0x000e64) :
      * ......0. : 6 letters
      * ......1. : 3 letters
    But there are then no more tests on the region and only 3 letters can be entered (display ". . .") .
  - Jumper displayed in the Dip Switches screen relies on bits 0 and 1 of the region
    (code at 0x0009b2), but some data has been altered not to display "FOR JAPAN." !
    So you get the following :
      * ......0. : "FOR U.S.A."
      * ......1. : "FOR EUROPE"
  - When "Invulnerability" Dip Switch is ON, you can do the following with the STARTn buttons :
      * press START2 to pause game
      * press START1 to unpause game
      * when START1 and START2 are pressed, the game enters in "slow motion" mode
  - When "Invulnerability" Dip Switch is ON, you can't die (of course), but you also can't move
    nor shoot while "captured" by an enemy or the background ! So you have to wait until enemy
    gives up or background scrolls enough to "free" you.
  - As you can play with 2 players at the same time, there is no need of a "Cabinet" setting,
    so DSWA bit 0 is unused (it is even no more tested).
  - DSWB bit 7 is unused (it is even no more tested).
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

    TO DO !


8) 'vimana' and "clones"

8a) 'vimana'

  - Region read from Territory Jumper (0x440011.b).
  - Coinage relies on bits 0 to 3 of the region :
      * ....0010 : TOAPLAN_COINAGE_EUROPE
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
  - Copyright always displays "@ TOAPLAN CO. LTD. 1991" but the 2 other lines rely on bits 0 to 3
    of the region (code at 0x016512 - tables at 0x01948e and 0x019496) :
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
    So the Tecmo logo is only displayed when "Territory" set to "Japan" (right to the
    "DISTRIBUTED BY" text).
  - FBI logo (after diplsaying the hi-scores) relies on bits 0 to 3 of the region
    (code at 0x0163f4) :
      * ....0001 : display
      * ....0111 : display
      *     else : no display
    So the FBI logo is only displayed when "Territory" set to "USA" or "USA (Romstar)".
  - Number of letters for initials are always set to 3 regardless of the rgion.
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
  - When "Invulnerability" Dip Switch is ON, you can press the unused 3rd button of player 1
    to advance quickly to your desired area.
  - When "Invulnerability" Dip Switch is ON, you can press the unused 3rd button of player 2,
    but as its effect is completely unknown (code at 0x0010aa),
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

8c) 'vimana1'

  - The only difference I've noticed with 'vimana' is the text at the "end" is in Japanese.
    This is why I've disabled all regions which aren't related to Japan.
            -> Changed enabled all regions !!
  - Sound routines at 0x01792c and 0x0179ba.


To Do:
    Add support for HD647180 (Z180) sound CPUs (once their internal ROMS are dumped).
    These are:
        Fire Shark / Same! Same! Same!
        Vimana
    In the meantime, it can be interesting to simulate the basic communications (coinage and credits)
    between the M68000 and the Z180.  [stephh]


Easter Eggs (using switches in cabinet):

    rallybik - While "TEST" Switch is held ON, you can play invulnerable mode game.
             - While playing invulnerable mode, press both "P1 button 1" and "P1 button 2" at the same time.
               No matter which player is playing.
               Then you can go to next stage soon. Score reverts. And when 2 players play, they alternate.

    truxton  - To enter the "sound check", press START2 when the grid is displayed.
             - While playing, when "Service Mode" Dip Switch is ON, you can not enter the grid screen,
               but can play invulnerable mode game.
               You can not play invulnerable mode game by pressing JAMMA "Test" Switch.
               It is a difference between "Service Mode" Dip Switch and JAMMA "Test" Switch.

    hellfire - (not yet found)

    zerowing - (not yet found)

    demonwld - (not yet found)

    fireshrk - While playing invulnerable mode, press "Service coin" to scroll fast.

    outzone  - (not yet found)

    vimana   - While playing invulnerable mode, press "P1 button 3" to scroll fast.
               No matter which player is playing.
             - While playing invulnerable mode, press "P2 button 3" to go to next stage.
               No matter which player is playing.


***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "cpu/z180/z180.h"
#include "cpu/tms32010/tms32010.h"
#include "includes/toaplipt.h"
#include "includes/toaplan1.h"
#include "sound/3812intf.h"


/***************************** debugging flags ******************************/

#define DEBUG_FREE_ALL_DIPSW FALSE
#define DEBUG_DONT_HIDE_DUP_DIPSETTING TRUE
	/* Set TRUE and you may find unknown easter eggs */

#if DEBUG_DONT_HIDE_DUP_DIPSETTING
  #define HIDE_DUP(_para) _para
#else
  #define HIDE_DIP(_para) /* null */
#endif


/**************************** customizing flags *****************************/

#define RALLYBIK_TEST_DESCRIBE_LEVEL 1
	/* Rallybik's "Test" has an easter egg. */
	/* If you want to rename it, set 1 or 2.
	   0 = "Test"                   or "Test (Toggle Switch)"
	   1 = "Test (Invulnerability)" or "Test (Toggle Switch) (Invulnerability)"
	   2 = "(Invulnerability)"      or "(Toggle Switch) (Invulnerability)" */
#define RALLYBIK_TEST_TOGGLE TRUE
	/* If you want to connect toggle switch, set TRUE. */
#define RALLYBIK_TEST_MOVE_TO_F1 FALSE
	/* If you want to move it to "F1", set TRUE. */


#define FIRESHRK_SERVICE_1_DESCRIBE_LEVEL 1
	/* Fireshrk's "Service 1" has an easter egg. */
	/* If you want to rename it, set 1 or 2.
	   0 = "Service 1"
	   1 = "Service 1 (Fast Scrolling If Invulnerable)"
	   2 = "(Fast Scrolling If Invulnerable)" */
#define FIRESHRK_SERVICE_1_MOVE_TO_F1 FALSE
	/* If you want to move it to "F1", set TRUE. */


#define VIMANA_P1_BUTTON_3_DESCRIBE_LEVEL 2
	/* Vimana's "P1 Button 3" has an easter egg. */
	/* If you want to rename it, set 1 or 2.
	   0 = "Spare (P1 Button 3)"
	   1 = "Spare (P1 Button 3) (Fast Scrolling If Invulnerable)"
	   2 = "(Fast Scrolling If Invulnerable)" */
#define VIMANA_P1_BUTTON_3_MOVE_TO_F1 FALSE
	/* If you want to move it to "F1", set TRUE. */

#define VIMANA_P2_BUTTON_3_DESCRIBE_LEVEL 2
	/* Vimana's "P2 Button 3" has an easter egg. */
	/* If you want to rename it, set 1 or 2.
	   0 = "Spare (P2 Button 3)"
	   1 = "Spare (P2 Button 3) (Go To Next Stage If Invulnerable)"
	   2 = "(Go To Next Stage If Invulnerable)" */
#define VIMANA_P2_BUTTON_3_MOVE_TO_F1 FALSE
	/* If you want to move it to "F1", set TRUE. */

	#if VIMANA_P1_BUTTON_3_MOVE_TO_F1 && VIMANA_P2_BUTTON_3_MOVE_TO_F1
	  ERROR !! : You can set zero or one TRUE, but you set two.
	#endif


/***************************** frequent strings *****************************/

static const char frqstr_europe_nova[] = "Europe (" /*"Licensed to "*/ "Nova Apparate GMBH & Co)";
static const char frqstr_europe_taitojapan[] = "Europe (" /*"Licensed to "*/ "Taito Japan)";
static const char frqstr_usa_romstar[] = "USA (" /*"Licensed to "*/ "Romstar)";
static const char frqstr_usa_romstarinc[] = "USA (" /*"Licensed to "*/ "Romstar Inc)";
static const char frqstr_usa_taitoamerica[] = "USA (" /*"Licensed to "*/ "Taito America)";
static const char frqstr_japan_taitocorp[] = "Japan (" /*"Licensed to "*/ "Taito Corp)";
static const char frqstr_japan_tecmo[] = "Japan (" /*"Distributed by "*/ "Tecmo)";
static const char frqstr_hongkong[] = "Hong Kong";
static const char frqstr_hongkong_honest[] = "Hong Kong (" /*"Licensed to "*/ "Honest Trading Co)";
static const char frqstr_hongkongchina_honest[] = "Hong Kong & China (" /*"Licensed to "*/ "Honest Trading Co)";
static const char frqstr_korea[] = "Korea";
static const char frqstr_taiwan[] = "Taiwan";
static const char frqstr_taiwan_spacy[] = "Taiwan (" /*"Licensed to "*/ "Spacy Co Ltd)";
static const char frqstr_no_country[] = "No Country";

/***************************** 68000 Memory Map *****************************/

static ADDRESS_MAP_START( rallybik_main_map, AS_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x00ffff) AM_ROM
	AM_RANGE(0x040000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x083fff) AM_RAM
	AM_RANGE(0x0c0000, 0x0c0fff) AM_RAM AM_BASE_SIZE_MEMBER(toaplan1_state, m_spriteram, m_spriteram_size)
	AM_RANGE(0x100000, 0x100001) AM_WRITE(rallybik_bcu_flipscreen_w)
	AM_RANGE(0x100002, 0x100003) AM_READWRITE(toaplan1_tileram_offs_r, toaplan1_tileram_offs_w)
	AM_RANGE(0x100004, 0x100007) AM_READWRITE(rallybik_tileram16_r, toaplan1_tileram16_w)
	AM_RANGE(0x100010, 0x10001f) AM_READWRITE(toaplan1_scroll_regs_r, toaplan1_scroll_regs_w)
	AM_RANGE(0x140000, 0x140001) AM_READ_PORT("VBLANK")
//  AM_RANGE(0x140000, 0x140001) AM_WRITE(?? video frame related ??)
	AM_RANGE(0x140002, 0x140003) AM_WRITE(toaplan1_intenable_w)
	AM_RANGE(0x140008, 0x14000f) AM_WRITE(toaplan1_bcu_control_w)
	AM_RANGE(0x144000, 0x1447ff) AM_READWRITE(toaplan1_colorram1_r, toaplan1_colorram1_w) AM_BASE_SIZE_MEMBER(toaplan1_state, m_colorram1, m_colorram1_size)
	AM_RANGE(0x146000, 0x1467ff) AM_READWRITE(toaplan1_colorram2_r, toaplan1_colorram2_w) AM_BASE_SIZE_MEMBER(toaplan1_state, m_colorram2, m_colorram2_size)
	AM_RANGE(0x180000, 0x180fff) AM_READWRITE(toaplan1_shared_r, toaplan1_shared_w)
	AM_RANGE(0x1c0000, 0x1c0003) AM_WRITE(toaplan1_tile_offsets_w)
	AM_RANGE(0x1c8000, 0x1c8001) AM_WRITE(toaplan1_reset_sound)
ADDRESS_MAP_END

static ADDRESS_MAP_START( truxton_main_map, AS_PROGRAM, 16 )
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
	AM_RANGE(0x144000, 0x1447ff) AM_READWRITE(toaplan1_colorram1_r, toaplan1_colorram1_w) AM_BASE_SIZE_MEMBER(toaplan1_state, m_colorram1, m_colorram1_size)
	AM_RANGE(0x146000, 0x1467ff) AM_READWRITE(toaplan1_colorram2_r, toaplan1_colorram2_w) AM_BASE_SIZE_MEMBER(toaplan1_state, m_colorram2, m_colorram2_size)
	AM_RANGE(0x180000, 0x180fff) AM_READWRITE(toaplan1_shared_r, toaplan1_shared_w)
	AM_RANGE(0x1c0000, 0x1c0003) AM_WRITE(toaplan1_tile_offsets_w)
	AM_RANGE(0x1c0006, 0x1c0007) AM_WRITE(toaplan1_fcu_flipscreen_w)
	AM_RANGE(0x1d0000, 0x1d0001) AM_WRITE(toaplan1_reset_sound)
ADDRESS_MAP_END

static ADDRESS_MAP_START( hellfire_main_map, AS_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x040000, 0x047fff) AM_RAM
	AM_RANGE(0x080000, 0x080001) AM_READ_PORT("VBLANK")
//  AM_RANGE(0x080000, 0x080001) AM_WRITE(?? video frame related ??)
	AM_RANGE(0x080002, 0x080003) AM_WRITE(toaplan1_intenable_w)
	AM_RANGE(0x080008, 0x08000f) AM_WRITE(toaplan1_bcu_control_w)
	AM_RANGE(0x084000, 0x0847ff) AM_READWRITE(toaplan1_colorram1_r, toaplan1_colorram1_w) AM_BASE_SIZE_MEMBER(toaplan1_state, m_colorram1, m_colorram1_size)
	AM_RANGE(0x086000, 0x0867ff) AM_READWRITE(toaplan1_colorram2_r, toaplan1_colorram2_w) AM_BASE_SIZE_MEMBER(toaplan1_state, m_colorram2, m_colorram2_size)
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
	AM_RANGE(0x180008, 0x180009) AM_WRITE(toaplan1_reset_sound)
ADDRESS_MAP_END

static ADDRESS_MAP_START( zerowing_main_map, AS_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x00ffff) AM_ROM
	AM_RANGE(0x040000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x087fff) AM_RAM
	AM_RANGE(0x0c0000, 0x0c0003) AM_WRITE(toaplan1_tile_offsets_w)
	AM_RANGE(0x0c0006, 0x0c0007) AM_WRITE(toaplan1_fcu_flipscreen_w)
	AM_RANGE(0x400000, 0x400001) AM_READ_PORT("VBLANK")
//  AM_RANGE(0x400000, 0x400001) AM_WRITE(?? video frame related ??)
	AM_RANGE(0x400002, 0x400003) AM_WRITE(toaplan1_intenable_w)
	AM_RANGE(0x400008, 0x40000f) AM_WRITE(toaplan1_bcu_control_w)
	AM_RANGE(0x404000, 0x4047ff) AM_READWRITE(toaplan1_colorram1_r, toaplan1_colorram1_w) AM_BASE_SIZE_MEMBER(toaplan1_state, m_colorram1, m_colorram1_size)
	AM_RANGE(0x406000, 0x4067ff) AM_READWRITE(toaplan1_colorram2_r, toaplan1_colorram2_w) AM_BASE_SIZE_MEMBER(toaplan1_state, m_colorram2, m_colorram2_size)
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

static ADDRESS_MAP_START( demonwld_main_map, AS_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x400000, 0x400001) AM_READ_PORT("VBLANK")
//  AM_RANGE(0x400000, 0x400001) AM_WRITE(?? video frame related ??)
	AM_RANGE(0x400002, 0x400003) AM_WRITE(toaplan1_intenable_w)
	AM_RANGE(0x400008, 0x40000f) AM_WRITE(toaplan1_bcu_control_w)
	AM_RANGE(0x404000, 0x4047ff) AM_READWRITE(toaplan1_colorram1_r, toaplan1_colorram1_w) AM_BASE_SIZE_MEMBER(toaplan1_state, m_colorram1, m_colorram1_size)
	AM_RANGE(0x406000, 0x4067ff) AM_READWRITE(toaplan1_colorram2_r, toaplan1_colorram2_w) AM_BASE_SIZE_MEMBER(toaplan1_state, m_colorram2, m_colorram2_size)
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
	AM_RANGE(0xe00008, 0xe00009) AM_WRITE(toaplan1_reset_sound)
	AM_RANGE(0xe0000a, 0xe0000b) AM_WRITE(demonwld_dsp_ctrl_w)	/* DSP Comms control */
ADDRESS_MAP_END

static ADDRESS_MAP_START( samesame_main_map, AS_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x00ffff) AM_ROM
	AM_RANGE(0x040000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x080003) AM_WRITE(toaplan1_tile_offsets_w)
	AM_RANGE(0x080006, 0x080007) AM_WRITE(toaplan1_fcu_flipscreen_w)
	AM_RANGE(0x0c0000, 0x0c3fff) AM_RAM			/* Frame done at $c1ada */
	AM_RANGE(0x100000, 0x100001) AM_READ_PORT("VBLANK")
//  AM_RANGE(0x100000, 0x100001) AM_WRITE(?? video frame related ??)
	AM_RANGE(0x100002, 0x100003) AM_WRITE(toaplan1_intenable_w)
	AM_RANGE(0x100008, 0x10000f) AM_WRITE(toaplan1_bcu_control_w)
	AM_RANGE(0x104000, 0x1047ff) AM_READWRITE(toaplan1_colorram1_r, toaplan1_colorram1_w) AM_BASE_SIZE_MEMBER(toaplan1_state, m_colorram1, m_colorram1_size)
	AM_RANGE(0x106000, 0x1067ff) AM_READWRITE(toaplan1_colorram2_r, toaplan1_colorram2_w) AM_BASE_SIZE_MEMBER(toaplan1_state, m_colorram2, m_colorram2_size)
	AM_RANGE(0x140000, 0x140001) AM_READ_PORT("P1")
	AM_RANGE(0x140002, 0x140003) AM_READ_PORT("P2")
	AM_RANGE(0x140004, 0x140005) AM_READ_PORT("DSWA")
	AM_RANGE(0x140006, 0x140007) AM_READ_PORT("DSWB")
	AM_RANGE(0x140008, 0x140009) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x14000a, 0x14000b) AM_READ(samesame_port_6_word_r)	/* Territory, and MCU ready */
	AM_RANGE(0x14000c, 0x14000d) AM_WRITE(samesame_coin_w)	/* Coin counter/lockout */
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

static ADDRESS_MAP_START( outzone_main_map, AS_PROGRAM, 16 )
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
	AM_RANGE(0x304000, 0x3047ff) AM_READWRITE(toaplan1_colorram1_r, toaplan1_colorram1_w) AM_BASE_SIZE_MEMBER(toaplan1_state, m_colorram1, m_colorram1_size)
	AM_RANGE(0x306000, 0x3067ff) AM_READWRITE(toaplan1_colorram2_r, toaplan1_colorram2_w) AM_BASE_SIZE_MEMBER(toaplan1_state, m_colorram2, m_colorram2_size)
	AM_RANGE(0x340000, 0x340003) AM_WRITE(toaplan1_tile_offsets_w)
	AM_RANGE(0x340006, 0x340007) AM_WRITE(toaplan1_fcu_flipscreen_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( vimana_main_map, AS_PROGRAM, 16 )
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
	AM_RANGE(0x404000, 0x4047ff) AM_READWRITE(toaplan1_colorram1_r, toaplan1_colorram1_w) AM_BASE_SIZE_MEMBER(toaplan1_state, m_colorram1, m_colorram1_size)
	AM_RANGE(0x406000, 0x4067ff) AM_READWRITE(toaplan1_colorram2_r, toaplan1_colorram2_w) AM_BASE_SIZE_MEMBER(toaplan1_state, m_colorram2, m_colorram2_size)
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

static ADDRESS_MAP_START( toaplan1_sound_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xffff) AM_RAM AM_BASE_MEMBER(toaplan1_state, m_sharedram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( rallybik_sound_io_map, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("P1")
	AM_RANGE(0x10, 0x10) AM_READ_PORT("P2")
	AM_RANGE(0x20, 0x20) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x30, 0x30) AM_WRITE(rallybik_coin_w)	/* Coin counter/lockout */
	AM_RANGE(0x40, 0x40) AM_READ_PORT("DSWA")
	AM_RANGE(0x50, 0x50) AM_READ_PORT("DSWB")
	AM_RANGE(0x60, 0x61) AM_DEVREADWRITE("ymsnd", ym3812_r, ym3812_w)
	AM_RANGE(0x70, 0x70) AM_READ_PORT("TJUMP")
ADDRESS_MAP_END

static ADDRESS_MAP_START( truxton_sound_io_map, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("P1")
	AM_RANGE(0x10, 0x10) AM_READ_PORT("P2")
	AM_RANGE(0x20, 0x20) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x30, 0x30) AM_WRITE(toaplan1_coin_w)	/* Coin counter/lockout */
	AM_RANGE(0x40, 0x40) AM_READ_PORT("DSWA")
	AM_RANGE(0x50, 0x50) AM_READ_PORT("DSWB")
	AM_RANGE(0x60, 0x61) AM_DEVREADWRITE("ymsnd", ym3812_r, ym3812_w)
	AM_RANGE(0x70, 0x70) AM_READ_PORT("TJUMP")
ADDRESS_MAP_END

static ADDRESS_MAP_START( hellfire_sound_io_map, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("DSWA")
	AM_RANGE(0x10, 0x10) AM_READ_PORT("DSWB")
	AM_RANGE(0x20, 0x20) AM_READ_PORT("TJUMP")
	AM_RANGE(0x30, 0x30) AM_WRITE(toaplan1_coin_w)	/* Coin counter/lockout */
	AM_RANGE(0x40, 0x40) AM_READ_PORT("P1")
	AM_RANGE(0x50, 0x50) AM_READ_PORT("P2")
	AM_RANGE(0x60, 0x60) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x70, 0x71) AM_DEVREADWRITE("ymsnd", ym3812_r, ym3812_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( zerowing_sound_io_map, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("P1")
	AM_RANGE(0x08, 0x08) AM_READ_PORT("P2")
	AM_RANGE(0x20, 0x20) AM_READ_PORT("DSWA")
	AM_RANGE(0x28, 0x28) AM_READ_PORT("DSWB")
	AM_RANGE(0x80, 0x80) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x88, 0x88) AM_READ_PORT("TJUMP")
	AM_RANGE(0xa0, 0xa0) AM_WRITE(toaplan1_coin_w)	/* Coin counter/lockout */
	AM_RANGE(0xa8, 0xa9) AM_DEVREADWRITE("ymsnd", ym3812_r, ym3812_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( demonwld_sound_io_map, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVREADWRITE("ymsnd", ym3812_r, ym3812_w)
	AM_RANGE(0x20, 0x20) AM_READ_PORT("TJUMP")
	AM_RANGE(0x40, 0x40) AM_WRITE(toaplan1_coin_w)	/* Coin counter/lockout */
	AM_RANGE(0x60, 0x60) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x80, 0x80) AM_READ_PORT("P1")
	AM_RANGE(0xa0, 0xa0) AM_READ_PORT("DSWB")
	AM_RANGE(0xc0, 0xc0) AM_READ_PORT("P2")
	AM_RANGE(0xe0, 0xe0) AM_READ_PORT("DSWA")
ADDRESS_MAP_END

static ADDRESS_MAP_START( outzone_sound_io_map, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVREADWRITE("ymsnd", ym3812_r, ym3812_w)
	AM_RANGE(0x04, 0x04) AM_WRITE(toaplan1_coin_w)	/* Coin counter/lockout */
	AM_RANGE(0x08, 0x08) AM_READ_PORT("DSWA")
	AM_RANGE(0x0c, 0x0c) AM_READ_PORT("DSWB")
	AM_RANGE(0x10, 0x10) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x14, 0x14) AM_READ_PORT("P1")
	AM_RANGE(0x18, 0x18) AM_READ_PORT("P2")
	AM_RANGE(0x1c, 0x1c) AM_READ_PORT("TJUMP")
ADDRESS_MAP_END


/***************************** TMS32010 Memory Map **************************/

static ADDRESS_MAP_START( DSP_program_map, AS_PROGRAM, 16 )
	AM_RANGE(0x000, 0x7ff) AM_ROM
ADDRESS_MAP_END

	/* $000 - 08F  TMS32010 Internal Data RAM in Data Address Space */

static ADDRESS_MAP_START( DSP_io_map, AS_IO, 16 )
	AM_RANGE(0, 0) AM_WRITE(demonwld_dsp_addrsel_w)
	AM_RANGE(1, 1) AM_READWRITE(demonwld_dsp_r, demonwld_dsp_w)
	AM_RANGE(3, 3) AM_WRITE(demonwld_dsp_bio_w)
	AM_RANGE(TMS32010_BIO, TMS32010_BIO) AM_READ(demonwld_BIO_r)
ADDRESS_MAP_END


/***************************** HD647180 Memory Map **************************/

static ADDRESS_MAP_START( hd647180_mem_map, AS_PROGRAM, 8 )
	AM_RANGE(0x00000, 0x03fff) AM_ROM	/* Internal 16k byte ROM */
	AM_RANGE(0x0fe00, 0x0ffff) AM_RAM	/* Internal 512 byte RAM */
ADDRESS_MAP_END



/*****************************************************************************
    Generic Input Port definitions
*****************************************************************************/

/* type1 */

static INPUT_PORTS_START( toaplan1_type1 )
	/* rallybik truxton zerowing vimana */

	PORT_START("P1")
	TOAPLAN_JOY_UDLR_2_BUTTONS(1)

	PORT_START("P2")
	TOAPLAN_JOY_UDLR_2_BUTTONS(2)

	PORT_START("SYSTEM")
	TOAPLAN_SYSTEM_INPUT_WITHOUT_VBLANK
	//                   ^^^^^^^ only one differnce between toaplan1_type1 and toaplan1_type2

	PORT_START("VBLANK")
	TOAPLAN_VBLANK_INPUT_16BITS

	PORT_START("DSWA") /* DIP SW 1 */
	TOAPLAN_DIP_1_SPARE(SW1)
	TOAPLAN_DIP_2_SPARE(SW1)
	TOAPLAN_DIP_3_SPARE(SW1)
	TOAPLAN_DIP_4_SPARE(SW1)
	TOAPLAN_DIP_5_SPARE(SW1)
	TOAPLAN_DIP_6_SPARE(SW1)
	TOAPLAN_DIP_7_SPARE(SW1)
	TOAPLAN_DIP_8_SPARE(SW1)

	PORT_START("DSWB") /* DIP SW 2 */
	TOAPLAN_DIP_1_SPARE(SW2)
	TOAPLAN_DIP_2_SPARE(SW2)
	TOAPLAN_DIP_3_SPARE(SW2)
	TOAPLAN_DIP_4_SPARE(SW2)
	TOAPLAN_DIP_5_SPARE(SW2)
	TOAPLAN_DIP_6_SPARE(SW2)
	TOAPLAN_DIP_7_SPARE(SW2)
	TOAPLAN_DIP_8_SPARE(SW2)

	PORT_START("TJUMP") /* Territory Jumper Block */
	TOAPLAN_JMPR_4_SPARE(JP) /* reverse order to DIP SW */
	TOAPLAN_JMPR_3_SPARE(JP)
	TOAPLAN_JMPR_2_SPARE(JP)
	TOAPLAN_JMPR_1_SPARE(JP)
	TOAPLAN_INPUT_GENERIC_FILL_UNKNOWN(0xf0) /* Unknown/Unused */

INPUT_PORTS_END

/* type2 */

static INPUT_PORTS_START( toaplan1_type2 )
	/* hellfire demonwld fireshrk outzone */

	PORT_START("VBLANK")
	TOAPLAN_VBLANK_INPUT_16BITS

	PORT_START("P1")
	TOAPLAN_JOY_UDLR_2_BUTTONS(1)

	PORT_START("P2")
	TOAPLAN_JOY_UDLR_2_BUTTONS(2)

	PORT_START("DSWA") /* DIP SW 1 */
	TOAPLAN_DIP_1_SPARE(SW1)
	TOAPLAN_DIP_2_SPARE(SW1)
	TOAPLAN_DIP_3_SPARE(SW1)
	TOAPLAN_DIP_4_SPARE(SW1)
	TOAPLAN_DIP_5_SPARE(SW1)
	TOAPLAN_DIP_6_SPARE(SW1)
	TOAPLAN_DIP_7_SPARE(SW1)
	TOAPLAN_DIP_8_SPARE(SW1)

	PORT_START("DSWB") /* DIP SW 2 */
	TOAPLAN_DIP_1_SPARE(SW2)
	TOAPLAN_DIP_2_SPARE(SW2)
	TOAPLAN_DIP_3_SPARE(SW2)
	TOAPLAN_DIP_4_SPARE(SW2)
	TOAPLAN_DIP_5_SPARE(SW2)
	TOAPLAN_DIP_6_SPARE(SW2)
	TOAPLAN_DIP_7_SPARE(SW2)
	TOAPLAN_DIP_8_SPARE(SW2)

	PORT_START("SYSTEM")
	TOAPLAN_SYSTEM_INPUT_WITH_VBLANK
	//                   ^^^^ only one differnce between toaplan1_type1 and toaplan1_type2

	PORT_START("TJUMP") /* Territory Jumper Block */
	TOAPLAN_JMPR_4_SPARE(JP) /* reverse order to DIP SW */
	TOAPLAN_JMPR_3_SPARE(JP)
	TOAPLAN_JMPR_2_SPARE(JP)
	TOAPLAN_JMPR_1_SPARE(JP)
	TOAPLAN_INPUT_GENERIC_FILL_UNKNOWN(0xf0) /* Unknown/Unused */
INPUT_PORTS_END

/*****************************************************************************
    Game-specific Input Port definitions
*****************************************************************************/

/* verified from M68000 and Z80 code */
static INPUT_PORTS_START( rallybik )
	PORT_INCLUDE( toaplan1_type1 )

	#if ! DEBUG_FREE_ALL_DIPSW
	  /* in 0x40 (CPU1) -> 0x8003 (CPU1 shared RAM) = 0x180006.w (CPU0 shared RAM) -> 0x0804f2.w */
	  PORT_MODIFY("DSWA")
	  TOAPLAN_DIP_A1_CABINET(SW1)
	  TOAPLAN_DIP_A2_FLIP_SCREEN(SW1)
	  TOAPLAN_DIP_A3_SERVICE_MODE(SW1)
	  TOAPLAN_DIP_A4_DEMO_SOUNDS(SW1)
	  TOAPLAN_DIP_A5678_COINAGE_DUAL(DSWB, 0x30, 0x20, SW1)  // mask{0x03} & value{0x02} -> Europe

	  /* in 0x50 (CPU1) -> 0x8004 (CPU1 shared RAM) = 0x180008.w (CPU0 shared RAM) -> 0x0804f4.w */
	  PORT_MODIFY("DSWB")
	  TOAPLAN_DIP_B12_DIFFICULTY(SW2)
	  //DIP_B34_SPARE (divert from 'toaplan1_type1')
	  // region verified 'rallybik' (MAME 0.143u7)
	    // listed as unused in the Dip Switches screen //           |Game   |Service|      |                   |       |
	  PORT_DIPNAME( 0x30, 0x20, DEF_STR( Region ) )    //           |Mode   |Mode   |      |                   |       |
	                  PORT_DIPLOCATION("SW2:!5,!6")    // Title     |Coinage|Coinage|Notice|Copyright          |License|Initials
	  PORT_DIPSETTING(    0x20, DEF_STR( Europe ) )    // Rally Bike|Europe |Europe |Off   |TAITO CORP. JAPAN  |-      |3 letters
	  PORT_DIPSETTING(    0x10, DEF_STR( USA ) )       // Rally Bike|Japan  |Japan  |Off   |TAITO AMERICA CORP.|-      |3 letters
	  PORT_DIPSETTING(    0x30, frqstr_usa_romstar )   // Rally Bike|Japan  |Japan  |Off   |TAITO AMERICA CORP.|ROMSTAR|3 letters
	  PORT_DIPSETTING(    0x00, DEF_STR( Japan ) )     // Dash Yarou|Japan  |Japan  |On    |TAITO CORPORATION  |-      |6 letters
	  TOAPLAN_DIP_B7_SHOW_DIPSW_SETTINGS(SW2)
	  TOAPLAN_DIP_B8_ALLOW_CONTINUE_OFF_YES(SW2) // not on race 1
	#endif

	PORT_MODIFY("TJUMP") // (not present ? this port isn't even read)
	TOAPLAN_INPUT_GENERIC_FILL_UNKNOWN(0x0f)

	/* P1 : in 0x00 (CPU1) -> 0x8006 (CPU1 shared RAM) = 0x18000c.w (CPU0 shared RAM) */
	/* P2 : in 0x10 (CPU1) -> 0x8007 (CPU1 shared RAM) = 0x18000e.w (CPU0 shared RAM) */

	/* in 0x20 (CPU1) -> 0x8005 (CPU1 shared RAM) = 0x18000a.w (CPU0 shared RAM) -> 0x0804f4.w */
	PORT_MODIFY("SYSTEM")
	#if ! RALLYBIK_TEST_TOGGLE
	  #if RALLYBIK_TEST_DESCRIBE_LEVEL == 1
	    TOAPLAN_TEST_SWITCH_RENAME("Test (Invulnerability)")
	  #elif RALLYBIK_TEST_DESCRIBE_LEVEL == 2
	    TOAPLAN_TEST_SWITCH_RENAME("(Invulnerability)")
	  #else
	    // TOAPLAN_TEST_SWITCH_RENAME("Test") // not renamed
	  #endif
	#else
	  #if RALLYBIK_TEST_DESCRIBE_LEVEL == 1
	    TOAPLAN_TEST_SWITCH_TOGGLE_RENAME("Test (Toggle Switch) (Invulnerability)")
	  #elif RALLYBIK_TEST_DESCRIBE_LEVEL == 2
	    TOAPLAN_TEST_SWITCH_TOGGLE_RENAME("(Toggle Switch) (Invulnerability)")
	  #else
	    TOAPLAN_TEST_SWITCH_TOGGLE
	  #endif
	#endif
	#if RALLYBIK_TEST_MOVE_TO_F1
	    PORT_CODE(KEYCODE_F1)
	#endif

	/* VBLANK : 0x140000.w */
INPUT_PORTS_END


/* verified from M68000 and Z80 code */
static INPUT_PORTS_START( truxton )
	PORT_INCLUDE( toaplan1_type1 )

	#if ! DEBUG_FREE_ALL_DIPSW
	  /* in 0x40 (CPU1) -> 0x8003 (CPU1 shared RAM) = 0x180006.w (CPU0 shared RAM) -> 0x081b78.w */
	  PORT_MODIFY("DSWA")
	  TOAPLAN_DIP_A1_CABINET(SW1)
	  TOAPLAN_DIP_A2_FLIP_SCREEN(SW1)
	  TOAPLAN_DIP_A3_SERVICE_MODE(SW1)
	  TOAPLAN_DIP_A4_DEMO_SOUNDS(SW1)
	  TOAPLAN_DIP_A5678_COINAGE_DUAL(TJUMP, 0x03, 0x02, SW1) // mask{0x07} & value{0x02,0x06} -> Europe

	  /* in 0x50 (CPU1) -> 0x8004 (CPU1 shared RAM) = 0x180008.w (CPU0 shared RAM) -> 0x081b7a.w */
	  PORT_MODIFY("DSWB")
	  TOAPLAN_DIP_B12_DIFFICULTY(SW2)
	  PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:!3,!4") /* table at 0x000930 */
	  PORT_DIPSETTING(    0x04, "50k 200k 150k+" )
	  PORT_DIPSETTING(    0x00, "70k 270k 200k+" )
	  PORT_DIPSETTING(    0x08, "100k Only" )
	  PORT_DIPSETTING(    0x0c, DEF_STR( None ) )
	  PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:!5,!6")
	  PORT_DIPSETTING(    0x30, "2" )
	  PORT_DIPSETTING(    0x00, "3" )
	  PORT_DIPSETTING(    0x20, "4" )
	  PORT_DIPSETTING(    0x10, "5" )
	  TOAPLAN_DIP_B7_SHOW_DIPSW_SETTINGS(SW2)
	  TOAPLAN_DIP_B8_ALLOW_CONTINUE_OFF_YES(SW2)

	  /* in 0x70 (CPU1) -> 0x8005 (CPU1 shared RAM) = 0x18000a.w (CPU0 shared RAM) -> 0x081b7c.w */
	  PORT_MODIFY("TJUMP")
	  // region verified 'truxton' (MAME 0.143u7)            //         |Game   |Service|      |Territory|Service |                   |       |
	  PORT_DIPNAME(        0x07, 0x02, DEF_STR( Region ) )   //         |Mode   |Mode   |      |Notice   |Mode    |                   |       |
	                         PORT_DIPLOCATION("JP:!4,!3,!2") // Title   |Coinage|Coinage|Notice|Location |Location|Copyright          |License|Initials
	            PORT_DIPSETTING( 0x02, DEF_STR( Europe ) )   // Truxton |Europe |Europe |Off   |-        |Europe  |TAITO CORPORATION  |-      |3 letters
	  HIDE_DUP( PORT_DIPSETTING( 0x03, DEF_STR( Europe ) ) ) // Truxton |Japan  |Europe |Off   |-        |Europe  |TAITO CORPORATION  |-      |3 letters
	  HIDE_DUP( PORT_DIPSETTING( 0x06, DEF_STR( Europe ) ) ) // Truxton |Europe |Europe |Off   |-        |Europe  |TAITO AMERICA CORP.|-      |3 letters
	  HIDE_DUP( PORT_DIPSETTING( 0x07, DEF_STR( Europe ) ) ) // Truxton |Japan  |Europe |Off   |-        |Europe  |TAITO AMERICA CORP.|-      |3 letters
	  HIDE_DUP( PORT_DIPSETTING( 0x04, DEF_STR( USA ) ) )    // Truxton |Japan  |Japan  |On    |USA      |Japan   |TAITO AMERICA CORP.|-      |3 letters
	            PORT_DIPSETTING( 0x05, DEF_STR( USA ) )      // Truxton |Japan  |Japan  |On    |USA      |USA     |TAITO AMERICA CORP.|-      |3 letters
	            PORT_DIPSETTING( 0x01, frqstr_usa_romstar )  // Truxton |Japan  |Japan  |On    |USA      |USA     |TAITO AMERICA CORP.|ROMSTAR|3 letters
	            PORT_DIPSETTING( 0x00, DEF_STR( Japan ) )    // Tatsujin|Japan  |Japan  |On    |Japan    |Japan   |TAITO CORPORATION  |-      |6 letters
	  //JP_1 (divert from 'toaplan1_type1')
	#endif

	/* P1 : in 0x00 (CPU1) -> 0x8007 (CPU1 shared RAM) = 0x18000e.w (CPU0 shared RAM) -> 0x081b82.w */
	/* P2 : in 0x10 (CPU1) -> 0x8008 (CPU1 shared RAM) = 0x180010.w (CPU0 shared RAM) -> 0x081b84.w */

	/* SYSTEM : in 0x20 (CPU1) -> 0x8006 (CPU1 shared RAM) = 0x18000c.w (CPU0 shared RAM) -> 0x081b7e.w */

	/* VBLANK : 0x140000.w */
INPUT_PORTS_END


static INPUT_PORTS_START( input_common_hellfire )
	PORT_INCLUDE( toaplan1_type2 )

	#if ! DEBUG_FREE_ALL_DIPSW
	  PORT_MODIFY("DSWA")
	  //DIP_A1_SPARE (divert from 'toaplan1_type2') (Specified by each set)
	  TOAPLAN_DIP_A2_FLIP_SCREEN(SW1)
	  TOAPLAN_DIP_A3_SERVICE_MODE(SW1)
	  TOAPLAN_DIP_A4_DEMO_SOUNDS(SW1)
	  //DIP_A5678_SPARE (divert from 'toaplan1_type2') (Specified by each set)

	  PORT_MODIFY("DSWB")
	  TOAPLAN_DIP_B12_DIFFICULTY(SW2)
	  PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:!3,!4")
	  PORT_DIPSETTING(    0x00, "70K, every 200K" )
	  PORT_DIPSETTING(    0x04, "100K, every 250K" )
	  PORT_DIPSETTING(    0x08, "100K" )
	  PORT_DIPSETTING(    0x0c, "200K" )
	  PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:!5,!6")
	  PORT_DIPSETTING(    0x30, "2" )
	  PORT_DIPSETTING(    0x00, "3" )
	  PORT_DIPSETTING(    0x20, "4" )
	  PORT_DIPSETTING(    0x10, "5" )
	  //DIP_B78_SPARE (divert from 'toaplan1_type2') (Specified by each set)

	  //PORT_MODIFY("TJUMP")
	  //JP_43_SPARE (divert from 'toaplan1_type2') (Specified by each set)
	  //JP_21_SPARE (divert from 'toaplan1_type2')
	#endif
INPUT_PORTS_END

static INPUT_PORTS_START( hellfire )
	PORT_INCLUDE( input_common_hellfire )

	#if ! DEBUG_FREE_ALL_DIPSW
	  PORT_MODIFY("DSWA")
	  //DIP_A1_SPARE (divert from 'toaplan1_type2' via 'input_common_hellfire')
	  TOAPLAN_DIP_A5678_COINAGE_DUAL(TJUMP, 0x02, 0x02, SW1) // mask{0x03} & value{0x02,0x03} -> Europe

	  PORT_MODIFY("DSWB")
	  TOAPLAN_DIP_B7_INVULNERABILITY(SW2)
	  //DIP_B8_SPARE (divert from 'toaplan1_type2' via 'input_common_hellfire')

	  PORT_MODIFY("TJUMP")
	  // region verified 'hellfire' (MAME 0.143u7)           // Game   |Service|      |                         |
	  PORT_DIPNAME(        0x03, 0x02, DEF_STR( Region ) )   // Mode   |Mode   |      |                         |
	                            PORT_DIPLOCATION("JP:!4,!3") // Coinage|Coinage|Notice|Licensed to              |initials
	  HIDE_DUP( PORT_DIPSETTING( 0x03, DEF_STR( Europe ) ) ) // Europe |Europe |Off   |TAITO CORPORATION        |3 letters
	            PORT_DIPSETTING( 0x02, DEF_STR( Europe ) )   // Europe |Europe |Off   |TAITO CORPORATION        |3 letters
	            PORT_DIPSETTING( 0x01, DEF_STR( USA ) )      // Japan  |Japan  |off   |TAITO AMERICA CORPORATION|3 letters
	            PORT_DIPSETTING( 0x00, DEF_STR( Japan ) )    // Japan  |Japan  |On    |TAITO CORPORATION        |3 letters
	#endif
INPUT_PORTS_END

static INPUT_PORTS_START( hellfire1 )
	PORT_INCLUDE( input_common_hellfire )

	#if ! DEBUG_FREE_ALL_DIPSW
	  PORT_MODIFY("DSWA")
	  TOAPLAN_DIP_A1_CABINET(SW1)
	  TOAPLAN_DIP_A5678_COINAGE_DUAL(TJUMP, 0x03, 0x02, SW1) // mask{0x03} & value{0x02} -> Europe

	  PORT_MODIFY("DSWB")
	  TOAPLAN_DIP_B7_SHOW_DIPSW_SETTINGS(SW2)
	  TOAPLAN_DIP_B8_ALLOW_CONTINUE_OFF_YES(SW2)

	  PORT_MODIFY("TJUMP")
	  // region verified 'hellfire1' (MAME 0.143u7)          // Game   |Service|      |           |
	  PORT_DIPNAME(        0x03, 0x02, DEF_STR( Region ) )   // Mode   |Mode   |      |           |
	                            PORT_DIPLOCATION("JP:!4,!3") // Coinage|Coinage|Notice|Licensed to|initials
	  HIDE_DUP( PORT_DIPSETTING( 0x03, DEF_STR( Europe ) ) ) // Japan  |Europe |Off   |TAITO CORP.|3 letters
	            PORT_DIPSETTING( 0x02, DEF_STR( Europe ) )   // Europe |Europe |Off   |TAITO CORP.|6 letters (default "...000")
	            PORT_DIPSETTING( 0x01, DEF_STR( USA ) )      // Japan  |Japan  |On    |TAITO CORP.|3 letters
	            PORT_DIPSETTING( 0x00, DEF_STR( Japan ) )    // Japan  |Japan  |On    |TAITO CORP.|6 letters (default "......")
	#endif
INPUT_PORTS_END

static INPUT_PORTS_START( hellfire2 )
	PORT_INCLUDE( input_common_hellfire )

	#if ! DEBUG_FREE_ALL_DIPSW
	  PORT_MODIFY("DSWA")
	  //DIP_A1_SPARE (divert from 'toaplan1_type2' via 'input_common_hellfire')
	  // A1 is listed as cabinet in the Dip Switches screen, but probably unused
	  TOAPLAN_DIP_A5678_COINAGE_DUAL(TJUMP, 0x03, 0x02, SW1) // mask{0x03} & value{0x02} -> Europe

	  PORT_MODIFY("DSWB")
	  TOAPLAN_DIP_B7_SHOW_DIPSW_SETTINGS(SW2)
	  TOAPLAN_DIP_B8_ALLOW_CONTINUE_OFF_YES(SW2)

	  PORT_MODIFY("TJUMP")
	  // region verified 'hellfire2' (MAME 0.143u7)          // Game   |Service|      |           |
	  PORT_DIPNAME(        0x03, 0x02, DEF_STR( Region ) )   // Mode   |Mode   |      |           |
	                            PORT_DIPLOCATION("JP:!4,!3") // Coinage|Coinage|Notice|Licensed to|initials
	  HIDE_DUP( PORT_DIPSETTING( 0x03, DEF_STR( Europe ) ) ) // Japan  |Europe |Off   |TAITO CORP.|3 letters
	            PORT_DIPSETTING( 0x02, DEF_STR( Europe ) )   // Europe |Europe |Off   |TAITO CORP.|3 letters
	            PORT_DIPSETTING( 0x01, DEF_STR( USA ) )      // Japan  |Japan  |Off   |TAITO CORP.|3 letters
	            PORT_DIPSETTING( 0x00, DEF_STR( Japan ) )    // Japan  |Japan  |Off   |TAITO CORP.|3 letters
	#endif
INPUT_PORTS_END

static INPUT_PORTS_START( hellfire3 )
	PORT_INCLUDE( input_common_hellfire )

	#if ! DEBUG_FREE_ALL_DIPSW
	  PORT_MODIFY("DSWA")
	  TOAPLAN_DIP_A1_CABINET(SW1)
	  TOAPLAN_DIP_A5678_COINAGE_DUAL(TJUMP, 0x02, 0x02, SW1) // mask{0x03} & value{0x02,0x03} -> Europe

	  PORT_MODIFY("DSWB")
	  TOAPLAN_DIP_B7_INVULNERABILITY(SW2)
	  TOAPLAN_DIP_B8_ALLOW_CONTINUE_OFF_YES(SW2)

	  PORT_MODIFY("TJUMP")
	  // region verified 'hellfire3' (MAME 0.143u7)          // Game   |Service|      |                 |
	  PORT_DIPNAME(        0x03, 0x02, DEF_STR( Region ) )   // Mode   |Mode   |      |                 |
	                            PORT_DIPLOCATION("JP:!4,!3") // Coinage|Coinage|Notice|Licensed to      |initials
	  HIDE_DUP( PORT_DIPSETTING( 0x03, DEF_STR( Europe ) ) ) // Europe |Europe |Off   |TAITO CORPORATION|3 letters
	            PORT_DIPSETTING( 0x02, DEF_STR( Europe ) )   // Europe |Europe |Off   |TAITO CORPORATION|6 letters (default "......")
	            PORT_DIPSETTING( 0x01, DEF_STR( USA ) )      // Japan  |Japan  |On    |TAITO CORPORATION|3 letters
	            PORT_DIPSETTING( 0x00, DEF_STR( Japan ) )    // Japan  |Japan  |On    |TAITO CORPORATION|6 letters (default "......")
	#endif
INPUT_PORTS_END


/* verified from M68000 and Z80 code */
static INPUT_PORTS_START( input_common_zerowing )
	PORT_INCLUDE( toaplan1_type1 )

	#if ! DEBUG_FREE_ALL_DIPSW
	  // in 0x20 (CPU1) -> 0x8003 (CPU1 shared RAM) = 0x440006.w (CPU0 shared RAM) -> 0x08180c.w ('zerowing') or 0x081ade.w ('zerowing2')
	  PORT_MODIFY("DSWA")
	  //DIP_A1_SPARE (divert from 'toaplan1_type1') (Specified by each set)
	  TOAPLAN_DIP_A2_FLIP_SCREEN(SW1)
	  TOAPLAN_DIP_A3_SERVICE_MODE(SW1)
	  TOAPLAN_DIP_A4_DEMO_SOUNDS(SW1)
	  //DIP_A5678_SPARE (divert from 'toaplan1_type1') (Specified by each set)

	  // in 0x28 (CPU1) -> 0x8004 (CPU1 shared RAM) = 0x440008.w (CPU0 shared RAM) -> 0x08180e.w ('zerowing') or 0x081ae0.w ('zerowing2')
	  PORT_MODIFY("DSWB")
	  TOAPLAN_DIP_B12_DIFFICULTY(SW2)
	  PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:!3,!4") // table at 0x00216c ('zerowing') or 0x002606 ('zerowing2')
	  PORT_DIPSETTING(    0x00, "200k 700k 500k+" )
	  PORT_DIPSETTING(    0x04, "500k 1500k 1000k+" )
	  PORT_DIPSETTING(    0x08, "500k Only" )
	  PORT_DIPSETTING(    0x0c, DEF_STR( None ) )
	  PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:!5,!6")
	  PORT_DIPSETTING(    0x30, "2" )
	  PORT_DIPSETTING(    0x00, "3" )
	  PORT_DIPSETTING(    0x20, "4" )
	  PORT_DIPSETTING(    0x10, "5" )
	  TOAPLAN_DIP_B7_INVULNERABILITY(SW2)
	  //DIP_B8_SPARE (divert from 'toaplan1_type1') (Specified by each set)

	  // in 0x88 (CPU1) -> 0x8005 (CPU1 shared RAM) = 0x44000a.w (CPU0 shared RAM) -> 0x081810.w ('zerowing') or 0x081ae2.w ('zerowing2')
	  //PORT_MODIFY("TJUMP")
	  //JP_43_SPARE (divert from 'toaplan1_type1') (Specified by each set)
	  //JP_21_SPARE (divert from 'toaplan1_type1')
	#endif

	// P1 : in 0x00 (CPU1) -> 0x8007 (CPU1 shared RAM) = 0x44000e.w (CPU0 shared RAM) -> 0x081818.w ('zerowing') or 0x081aea.w ('zerowing2')
	// P2 : in 0x08 (CPU1) -> 0x8008 (CPU1 shared RAM) = 0x440010.w (CPU0 shared RAM) -> 0x08181a.w ('zerowing') or 0x081aec.w ('zerowing2')

	// SYSTEM : in 0x80 (CPU1) -> 0x8006 (CPU1 shared RAM) = 0x44000c.w (CPU0 shared RAM) -> 0x081812.w ('zerowing') or 0x081ae4.w ('zerowing2')

	// VBLANK : 0x400000.w ('zerowing' and 'zerowing2')
INPUT_PORTS_END

/* verified from M68000 and Z80 code */
static INPUT_PORTS_START( zerowing )
	PORT_INCLUDE( input_common_zerowing )

	#if ! DEBUG_FREE_ALL_DIPSW
	  PORT_MODIFY("DSWA")
	  TOAPLAN_DIP_A1_CABINET(SW1)
	  TOAPLAN_DIP_A5678_COINAGE_DUAL(TJUMP, 0x02, 0x02, SW1) // mask{0x03} & value{0x02,0x03} -> Europe

	  PORT_MODIFY("DSWB")
	  TOAPLAN_DIP_B8_ALLOW_CONTINUE_OFF_YES(SW2)

	  PORT_MODIFY("TJUMP")
	  // region verified 'zerowing' (MAME 0.143u7)          // Game   |Service|      |
	  PORT_DIPNAME(        0x03, 0x03, DEF_STR( Region ) )   // Mode   |Mode   |      |
	                            PORT_DIPLOCATION("JP:!4,!3") // Coinage|Coinage|Notice|initials
	            PORT_DIPSETTING( 0x03, DEF_STR( Europe ) )   // Europe |Europe |Off   |3 letters
	  HIDE_DUP( PORT_DIPSETTING( 0x02, DEF_STR( Europe ) ) ) // Europe |Europe |Off   |6 letters (default "...000")
	            PORT_DIPSETTING( 0x01, DEF_STR( USA ) )      // Japan  |Japan  |off   |3 letters
	            PORT_DIPSETTING( 0x00, DEF_STR( Japan ) )    // Japan  |Japan  |On    |6 letters (default "......")
	#endif
INPUT_PORTS_END

/* verified from M68000 and Z80 code */
static INPUT_PORTS_START( zerowing2 )
	PORT_INCLUDE( input_common_zerowing )

	#if ! DEBUG_FREE_ALL_DIPSW
	  PORT_MODIFY("DSWA")
	  //DIP_A1_SPARE (divert from 'toaplan1_type1' via 'input_common_zerowing')
	  TOAPLAN_DIP_A5678_COINAGE_DUAL(TJUMP, 0x02, 0x02, SW1) // mask{0x02} & value{0x02} -> Europe

	  //PORT_MODIFY("DSWB")
	  //DIP_B8_SPARE (divert from 'toaplan1_type1' via 'input_common_zerowing')

	  PORT_MODIFY("TJUMP")
	  //JP_4_SPARE (divert from 'toaplan1_type1' via 'input_common_zerowing')
	  // region verified 'zerowing2' (MAME 0.143u7) // Game   |Service|      |                               |
	  PORT_DIPNAME( 0x02, 0x00, DEF_STR( Region ) ) // Mode   |Mode   |      |                               |
	                      PORT_DIPLOCATION("JP:!3") // Coinage|Coinage|Notice|Copyright                      |initials
	  PORT_DIPSETTING(    0x02, DEF_STR( Europe ) ) // Europe |Europe |Off   |WILLIAMS ELECTRONICS GAMES, INC|3 letters
	  PORT_DIPSETTING(    0x00, DEF_STR( USA ) )    // Japan  |Japan  |off   |WILLIAMS ELECTRONICS GAMES, INC|3 letters
	#endif
INPUT_PORTS_END


static INPUT_PORTS_START( input_common_demonwld )
	PORT_INCLUDE( toaplan1_type2 )

	#if ! DEBUG_FREE_ALL_DIPSW
	  PORT_MODIFY("DSWA")
	  //DIP_A1_SPARE (divert from 'toaplan1_type2')
	  TOAPLAN_DIP_A2_FLIP_SCREEN(SW1)
	  TOAPLAN_DIP_A3_SERVICE_MODE(SW1)
	  TOAPLAN_DIP_A4_DEMO_SOUNDS(SW1)
	  //DIP_A5678_SPARE (divert from 'toaplan1_type2') (Specified by each set)

	  PORT_MODIFY("DSWB")
	  TOAPLAN_DIP_B12_DIFFICULTY(SW2)
	  PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:!3,!4")
	  PORT_DIPSETTING(    0x00, "30K, every 100K" )
	  PORT_DIPSETTING(    0x04, "50K and 100K" )
	  PORT_DIPSETTING(    0x08, "100K only" )
	  PORT_DIPSETTING(    0x0c, DEF_STR( None ) )
	  PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:!5,!6")
	  PORT_DIPSETTING(    0x30, "1" )
	  PORT_DIPSETTING(    0x20, "2" )
	  PORT_DIPSETTING(    0x00, "3" )
	  PORT_DIPSETTING(    0x10, "5" )
	  TOAPLAN_DIP_B7_INVULNERABILITY(SW2)
	  //DIP_B8_SPARE (divert from 'toaplan1_type2')

	  //PORT_MODIFY("TJUMP")
	  //JP_43_SPARE (divert from 'toaplan1_type2') (Specified by each set)
	  //JP_21_SPARE (divert from 'toaplan1_type2')
	#endif
INPUT_PORTS_END

static INPUT_PORTS_START( demonwld ) // demonwld, demonwld2, demonwld3 and demonwld4
	PORT_INCLUDE( input_common_demonwld )

	#if ! DEBUG_FREE_ALL_DIPSW
	  PORT_MODIFY("DSWA")
	  TOAPLAN_DIP_A5678_COINAGE_DUAL(TJUMP, 0x03, 0x02, SW1) // mask{0x03} & value{0x02} -> Europe

	  PORT_MODIFY("TJUMP")
	  // region verified 'demonwld','demonwld2','demonwld3','demonwld4' (MAME 0.143u7)
	                                                            //              |Game   |Service|      |
	  PORT_DIPNAME(        0x03, 0x01, DEF_STR( Region ) )      //              |Mode   |Mode   |      |
	                               PORT_DIPLOCATION("JP:!4,!3") // Title        |Coinage|Coinage|Notice|Licensed to
	            PORT_DIPSETTING( 0x02, DEF_STR( Europe ) )      // Demon's World|Europe |?      |off   |-
	  HIDE_DUP( PORT_DIPSETTING( 0x03, DEF_STR( USA ) ) )       // Demon's World|Japan  |?      |off   |-
	            PORT_DIPSETTING( 0x01, DEF_STR( USA ) )         // Demon's World|Japan  |?      |Off   |-
	            PORT_DIPSETTING( 0x00, frqstr_japan_taitocorp ) // Horror Story |Japan  |?      |On    |TAITO CORP.
	                                                            //                       service mode (dip switchs screen) is not yet found
	#endif
INPUT_PORTS_END

static INPUT_PORTS_START( demonwld1 )
	PORT_INCLUDE( input_common_demonwld )

	#if ! DEBUG_FREE_ALL_DIPSW
	  PORT_MODIFY("DSWA")
	  TOAPLAN_DIP_A5678_COINAGE_DUAL(TJUMP, 0x03, 0x02, SW1) // mask{0x03} & value{0x02} -> Europe

	  PORT_MODIFY("TJUMP")
	  // region verified 'demonwld1' (MAME 0.143u7)        //              |Game   |Service|      |
	  PORT_DIPNAME( 0x03, 0x01, DEF_STR( Region ) )        //              |Mode   |Mode   |      |
	                          PORT_DIPLOCATION("JP:!4,!3") // Title        |Coinage|Coinage|Notice|Licensed to
	  PORT_DIPSETTING(    0x02, frqstr_europe_taitojapan ) // Demon's World|Europe |?      |off   |TAITO JAPAN
	  PORT_DIPSETTING(    0x03, DEF_STR( USA ) )           // Demon's World|Japan  |?      |off   |-
	  PORT_DIPSETTING(    0x01, frqstr_usa_taitoamerica )  // Demon's World|Japan  |?      |Off   |TAITO AMERICA
	  PORT_DIPSETTING(    0x00, frqstr_japan_taitocorp )   // Horror Story |Japan  |?      |On    |TAITO CORP.
	                                                       //                       service mode (dip switchs screen) is not yet found
	#endif
INPUT_PORTS_END


static INPUT_PORTS_START( input_common_fireshrk )
	PORT_INCLUDE( toaplan1_type2 )

	#if ! DEBUG_FREE_ALL_DIPSW
	  PORT_MODIFY("DSWA")
	  //DIP_A1_SPARE (divert from 'toaplan1_type2') (Specified by each set)
	  TOAPLAN_DIP_A2_FLIP_SCREEN(SW1)
	  TOAPLAN_DIP_A3_SERVICE_MODE(SW1)
	  TOAPLAN_DIP_A4_DEMO_SOUNDS(SW1)
	  //DIP_A5678_SPARE (divert from 'toaplan1_type2') (Specified by each set)

	  PORT_MODIFY("DSWB")
	  TOAPLAN_DIP_B12_DIFFICULTY(SW2)
	  PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:!3,!4")
	  PORT_DIPSETTING(    0x04, "50K, every 150K" )
	  PORT_DIPSETTING(    0x00, "70K, every 200K" )
	  PORT_DIPSETTING(    0x08, "100K" )
	  PORT_DIPSETTING(    0x0c, DEF_STR( None ) )
	  PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:!5,!6")
	  PORT_DIPSETTING(    0x30, "2" )
	  PORT_DIPSETTING(    0x00, "3" )
	  PORT_DIPSETTING(    0x20, "4" )
	  PORT_DIPSETTING(    0x10, "5" )
	  TOAPLAN_DIP_B7_INVULNERABILITY(SW2)
	  //DIP_B8_SPARE (divert from 'toaplan1_type2') (Specified by each set)

	  //PORT_MODIFY("TJUMP")
	  //JP_4321_SPARE (divert from 'toaplan1_type2') (Specified by each set)
	#endif

	PORT_MODIFY("SYSTEM")
	#if FIRESHRK_SERVICE_1_DESCRIBE_LEVEL == 1
	  PORT_BIT(0x01, TOAPLAN_IP_ACTIVE_LEVEL, IPT_SERVICE1) PORT_NAME("Service 1 (Fast Scrolling If Invulnerable)") /* JAMMA "Service" */
	#elif FIRESHRK_SERVICE_1_DESCRIBE_LEVEL == 2
	  PORT_BIT(0x01, TOAPLAN_IP_ACTIVE_LEVEL, IPT_SERVICE1) PORT_NAME("(Fast Scrolling If Invulnerable)")
	#endif
	#if FIRESHRK_SERVICE_1_MOVE_TO_F1
	  PORT_CODE(KEYCODE_F1)
	#endif

INPUT_PORTS_END

static INPUT_PORTS_START( fireshrk )
	PORT_INCLUDE( input_common_fireshrk )

	#if ! DEBUG_FREE_ALL_DIPSW
	  PORT_MODIFY("DSWA")
	  //DIP_A1_SPARE (divert from 'toaplan1_type2' via 'input_common_fireshrk')
	  TOAPLAN_DIP_A5678_COINAGE_DUAL_GREATERTHAN(TJUMP, 0x03, 0x00, SW1) // mask{0x03} & value{0x01,0x02,0x03} -> Europe

	  //PORT_MODIFY("DSWB")
	  //DIP_B8_SPARE (divert from 'toaplan1_type2' via 'input_common_fireshrk')

	  PORT_MODIFY("TJUMP")
	  // region verified 'fireshrk' (MAME 0.143u7)             //           |Game   |Service|      |
	  PORT_DIPNAME(        0x0f, 0x02, DEF_STR( Region ) )     //           |Mode   |Mode   |      |
	                        PORT_DIPLOCATION("JP:!4,!3,!2,!1") // Title     |Coinage|Coinage|Notice|Licensed to
	            PORT_DIPSETTING( 0x01, DEF_STR( Europe ) )     // Fire Shark|Europe |Europe |Off   |-
	  HIDE_DUP( PORT_DIPSETTING( 0x02, DEF_STR( Europe ) ) )   // Fire Shark|Europe |Europe |Off   |-
	  HIDE_DUP( PORT_DIPSETTING( 0x03, DEF_STR( Europe ) ) )   // Fire Shark|Europe |Europe |Off   |-
	  HIDE_DUP( PORT_DIPSETTING( 0x05, DEF_STR( Europe ) ) )   // Fire Shark|Europe |Europe |Off   |-
	  HIDE_DUP( PORT_DIPSETTING( 0x06, DEF_STR( Europe ) ) )   // Fire Shark|Europe |Europe |Off   |-
	  HIDE_DUP( PORT_DIPSETTING( 0x07, DEF_STR( Europe ) ) )   // Fire Shark|Europe |Europe |Off   |-
	  HIDE_DUP( PORT_DIPSETTING( 0x09, DEF_STR( Europe ) ) )   // Fire Shark|Europe |Europe |Off   |-
	  HIDE_DUP( PORT_DIPSETTING( 0x0a, DEF_STR( Europe ) ) )   // Fire Shark|Europe |Europe |Off   |-
	  HIDE_DUP( PORT_DIPSETTING( 0x0b, DEF_STR( Europe ) ) )   // Fire Shark|Europe |Europe |Off   |-
	  HIDE_DUP( PORT_DIPSETTING( 0x0d, DEF_STR( Europe ) ) )   // Fire Shark|Europe |Europe |Off   |-
	  HIDE_DUP( PORT_DIPSETTING( 0x0e, DEF_STR( Europe ) ) )   // Fire Shark|Europe |Europe |Off   |-
	  HIDE_DUP( PORT_DIPSETTING( 0x0f, DEF_STR( Europe ) ) )   // Fire Shark|Europe |Europe |Off   |-
	            PORT_DIPSETTING( 0x04, DEF_STR( USA ) )        // Fire Shark|Japan  |Japan  |On    |-
	  HIDE_DUP( PORT_DIPSETTING( 0x08, DEF_STR( USA ) ) )      // Fire Shark|Japan  |Japan  |On    |-
	  HIDE_DUP( PORT_DIPSETTING( 0x0c, DEF_STR( USA ) ) )      // Fire Shark|Japan  |Japan  |On    |-
	            PORT_DIPSETTING( 0x00, frqstr_usa_romstarinc ) // Fire Shark|Japan  |Japan  |On    |ROMSTAR,INC.
	  // JP_43 are listed as unused in the Dip Switches screen, but affect to license
	#endif
INPUT_PORTS_END

static INPUT_PORTS_START( samesame )
	PORT_INCLUDE( input_common_fireshrk )

	#if ! DEBUG_FREE_ALL_DIPSW
	  PORT_MODIFY("DSWA")
	  TOAPLAN_DIP_A1_CABINET(SW1)
	  //DIP_A5678_SPARE (divert from 'toaplan1_type2' via 'input_common_fireshrk')

	  PORT_MODIFY("DSWB")
	  TOAPLAN_DIP_B8_ALLOW_CONTINUE_OFF_YES(SW2)

	  //PORT_MODIFY("TJUMP")
	  //JP_4321_SPARE (divert from 'toaplan1_type2' via 'input_common_fireshrk')

	  // region verified 'samesame' (MAME 0.143u7)           //               |Game   |Service|      |
	  //PORT_DIPNAME(        0x0f, 0x00, DEF_STR( Region ) ) //               |Mode   |Mode   |      |
	  //                  PORT_DIPLOCATION("JP:!4,!3,!2,!1") // Title         |Coinage|Coinage|Notice|Licensed to
	  //          PORT_DIPSETTING( 0x00, DEF_STR( Japan ) )  // Same Same Same|1C_1C  |Japan  |On    |-
	  //          PORT_DIPSETTING( 0x01, DEF_STR( Japan ) )  // Same Same Same|1C_1C  |Japan  |On    |-
	  //          PORT_DIPSETTING( 0x02, DEF_STR( Japan ) )  // Same Same Same|1C_1C  |Europe |On    |-
	  //          PORT_DIPSETTING( 0x03, DEF_STR( Japan ) )  // Same Same Same|1C_1C  |Europe |On    |-
	  //          PORT_DIPSETTING( 0x04, DEF_STR( Japan ) )  // Same Same Same|1C_1C  |Japan  |On    |-
	  //          PORT_DIPSETTING( 0x05, DEF_STR( Japan ) )  // Same Same Same|1C_1C  |Japan  |On    |-
	  //          PORT_DIPSETTING( 0x06, DEF_STR( Japan ) )  // Same Same Same|1C_1C  |Europe |On    |-
	  //          PORT_DIPSETTING( 0x07, DEF_STR( Japan ) )  // Same Same Same|1C_1C  |Europe |On    |-
	  //          PORT_DIPSETTING( 0x08, DEF_STR( Japan ) )  // Same Same Same|1C_1C  |Japan  |On    |-
	  //          PORT_DIPSETTING( 0x09, DEF_STR( Japan ) )  // Same Same Same|1C_1C  |Japan  |On    |-
	  //          PORT_DIPSETTING( 0x0a, DEF_STR( Japan ) )  // Same Same Same|1C_1C  |Europe |On    |-
	  //          PORT_DIPSETTING( 0x0b, DEF_STR( Japan ) )  // Same Same Same|1C_1C  |Europe |On    |-
	  //          PORT_DIPSETTING( 0x0c, DEF_STR( Japan ) )  // Same Same Same|1C_1C  |Japan  |On    |-
	  //          PORT_DIPSETTING( 0x0d, DEF_STR( Japan ) )  // Same Same Same|1C_1C  |Japan  |On    |-
	  //          PORT_DIPSETTING( 0x0e, DEF_STR( Japan ) )  // Same Same Same|1C_1C  |Europe |On    |-
	  //          PORT_DIPSETTING( 0x0f, DEF_STR( Japan ) )  // Same Same Same|1C_1C  |Europe |On    |-
	#endif
INPUT_PORTS_END

static INPUT_PORTS_START( samesame2 ) // samesame2 and fireshrkd
	PORT_INCLUDE( input_common_fireshrk )

	#if ! DEBUG_FREE_ALL_DIPSW
	  //PORT_MODIFY("DSWA")
	  //DIP_A1_SPARE (divert from 'toaplan1_type2' via 'input_common_fireshrk')
	  //DIP_A5678_SPARE (divert from 'toaplan1_type2' via 'input_common_fireshrk')

	  //PORT_MODIFY("DSWB")
	  //DIP_B8_SPARE (divert from 'toaplan1_type2' via 'input_common_fireshrk')

	  PORT_MODIFY("TJUMP")
	  PORT_DIPNAME(           0x03, 0x00, "Show Territory Notice" ) PORT_DIPLOCATION("JP:!4,!3")
	  HIDE_DUP( PORT_DIPSETTING(    0x03, DEF_STR( No ) ) )
	  HIDE_DUP( PORT_DIPSETTING(    0x02, DEF_STR( No ) ) )
	            PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	            PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	  //JP_21_SPARE (divert from 'toaplan1_type2' via 'input_common_fireshrk')

	  // region verified 'samesame2' (MAME 0.143u7)          //               |Game   |Service|      |Territory|Service
	  //PORT_DIPNAME(        0x03, 0x00, DEF_STR( Region ) ) //               |Mode   |Mode   |      |Notice   |Mode
	  //                  PORT_DIPLOCATION("JP:!4,!3,!2,!1") // Title         |Coinage|Coinage|Notice|Location |Location
	  //          PORT_DIPSETTING( 0x03, DEF_STR( Japan ) )  // Same Same Same|1C_1C  |Europe |Off   |-        |Europe
	  //          PORT_DIPSETTING( 0x02, DEF_STR( Japan ) )  // Same Same Same|1C_1C  |Europe |Off   |-        |Europe
	  //          PORT_DIPSETTING( 0x01, DEF_STR( Japan ) )  // Same Same Same|1C_1C  |Europe |Off   |-        |Europe
	  //          PORT_DIPSETTING( 0x00, DEF_STR( Japan ) )  // Same Same Same|1C_1C  |Japan  |On    |Japan    |USA

	  // region verified 'fireshrkd' (MAME 0.143u7)          //           |Game   |Service|      |Territory|Service |
	  //PORT_DIPNAME(        0x03, 0x00, DEF_STR( Region ) ) //           |Mode   |Mode   |      |Notice   |Mode    |
	  //                  PORT_DIPLOCATION("JP:!4,!3,!2,!1") // Title     |Coinage|Coinage|Notice|Location |Location|Licensed to
	  //          PORT_DIPSETTING( 0x03, DEF_STR( Japan ) )  // Fire Shark|1C_1C  |Europe |Off   |-        |Europe  |DOOYONG CO.,LTD.
	  //          PORT_DIPSETTING( 0x02, DEF_STR( Japan ) )  // Fire Shark|1C_1C  |Europe |Off   |-        |Europe  |DOOYONG CO.,LTD.
	  //          PORT_DIPSETTING( 0x01, DEF_STR( Japan ) )  // Fire Shark|1C_1C  |Europe |Off   |-        |Europe  |DOOYONG CO.,LTD.
	  //          PORT_DIPSETTING( 0x00, DEF_STR( Japan ) )  // Fire Shark|1C_1C  |Japan  |On    |Korean   |Korean  |DOOYONG CO.,LTD.
	#endif
INPUT_PORTS_END

static INPUT_PORTS_START( fireshrkdh )
	PORT_INCLUDE( input_common_fireshrk )

	#if ! DEBUG_FREE_ALL_DIPSW
	  PORT_MODIFY("DSWA")
	  //DIP_A1_SPARE (divert from 'toaplan1_type2' via 'input_common_fireshrk')
	  TOAPLAN_DIP_A5678_COINAGE_DUAL_GREATERTHAN(TJUMP, 0x03, 0x00, SW1) // mask{0x03} & value{0x01,0x02,0x03} -> Europe

	  //PORT_MODIFY("DSWB")
	  //DIP_B8_SPARE (divert from 'toaplan1_type2' via 'input_common_fireshrk')

	  PORT_MODIFY("TJUMP")
	  // region verified 'fireshrkdh' (MAME 0.143u7)         //           |Game   |Service|      |Territory|Service |
	  PORT_DIPNAME(        0x03, 0x00, DEF_STR( Region ) )   //           |Mode   |Mode   |      |Notice   |Mode    |
	                            PORT_DIPLOCATION("JP:!4,!3") // Title     |Coinage|Coinage|Notice|Location |Location|Licensed to
	  HIDE_DUP( PORT_DIPSETTING( 0x03, DEF_STR( Europe ) ) ) // Fire Shark|Europe |Europe |Off   |-        |Europe  |DOOYONG IND.CO.,LTD.
	  HIDE_DUP( PORT_DIPSETTING( 0x02, DEF_STR( Europe ) ) ) // Fire Shark|Europe |Europe |Off   |-        |Europe  |DOOYONG IND.CO.,LTD.
	            PORT_DIPSETTING( 0x01, DEF_STR( Europe ) )   // Fire Shark|Europe |Europe |Off   |-        |Europe  |DOOYONG IND.CO.,LTD.
	            PORT_DIPSETTING( 0x00, frqstr_korea )        // Fire Shark|Japan  |Japan  |On    |Korean   |Korean  |DOOYONG IND.CO.,LTD.
	  //JP_21_SPARE (divert from 'toaplan1_type2' via 'input_common_fireshrk')
	#endif
INPUT_PORTS_END


static INPUT_PORTS_START( input_common_outzone )
	PORT_INCLUDE( toaplan1_type2 )

	#if ! DEBUG_FREE_ALL_DIPSW
	  PORT_MODIFY("DSWA")
	  //DIP_A1_SPARE (divert from 'toaplan1_type2')
	  TOAPLAN_DIP_A2_FLIP_SCREEN(SW1)
	  TOAPLAN_DIP_A3_SERVICE_MODE(SW1)
	  TOAPLAN_DIP_A4_DEMO_SOUNDS(SW1)
	  //DIP_A5678_SPARE (divert from 'toaplan1_type2') (Specified by each set)

	  PORT_MODIFY("DSWB")
	  TOAPLAN_DIP_B12_DIFFICULTY(SW2)
	  PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:!3,!4")
	  PORT_DIPSETTING(    0x00, "Every 300K" )
	  PORT_DIPSETTING(    0x04, "200K and 500K" )
	  PORT_DIPSETTING(    0x08, "300K only" )
	  PORT_DIPSETTING(    0x0c, DEF_STR( None ) )
	  PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:!5,!6")
	  PORT_DIPSETTING(    0x30, "1" )
	  PORT_DIPSETTING(    0x20, "2" )
	  PORT_DIPSETTING(    0x00, "3" )
	  PORT_DIPSETTING(    0x10, "5" )
	  TOAPLAN_DIP_B7_INVULNERABILITY(SW2)
	  //DIP_B8_SPARE (divert from 'toaplan1_type2')

	  //PORT_MODIFY("TJUMP")
	  //JP_4321_SPARE (divert from 'toaplan1_type2') (Specified by each set)
	#endif
INPUT_PORTS_END

static INPUT_PORTS_START( outzone ) // outzone and outzonec
	PORT_INCLUDE( input_common_outzone )

	#if ! DEBUG_FREE_ALL_DIPSW
	  PORT_MODIFY("DSWA")
	  TOAPLAN_DIP_A5678_COINAGE_DUAL(TJUMP, 0x0f, 0x02, SW1) // mask{0x0f} & value{0x02} -> Europe

	  PORT_MODIFY("TJUMP")
	  // region verified 'outzone','outzonec' (MAME 0.143u7)         // Game   |Service|      |                  |    |     |Stage
	  PORT_DIPNAME(        0x0f, 0x02, DEF_STR( Region ) )           // Mode   |Mode   |      |                  |FBI |Demo |Repeat
	                              PORT_DIPLOCATION("JP:!4,!3,!2,!1") // Coinage|Coinage|Notice|Licensed to       |Logo|Story|Message
	            PORT_DIPSETTING( 0x02, DEF_STR( Europe ) )           // Europe |Europe |On    |-                 |Off |Off  |Off
	            PORT_DIPSETTING( 0x01, DEF_STR( USA ) )              // Japan  |Japan  |On    |-                 |On  |Off  |Off
	            PORT_DIPSETTING( 0x07, frqstr_usa_romstarinc )       // Japan  |Europe |On    |ROMSTAR, INC.     |On  |Off  |Off
	            PORT_DIPSETTING( 0x00, DEF_STR( Japan ) )            // Japan  |Japan  |On    |-                 |Off |On   |On
	  HIDE_DUP( PORT_DIPSETTING( 0x09, DEF_STR( Japan ) ) )          // Japan  |Japan  |On    |-                 |Off |On   |On
	  HIDE_DUP( PORT_DIPSETTING( 0x0a, DEF_STR( Japan ) ) )          // Japan  |Europe |On    |-                 |Off |On   |On
	  HIDE_DUP( PORT_DIPSETTING( 0x0b, DEF_STR( Japan ) ) )          // Japan  |Europe |On    |-                 |Off |On   |On
	  HIDE_DUP( PORT_DIPSETTING( 0x0c, DEF_STR( Japan ) ) )          // Japan  |Europe |On    |-                 |Off |On   |On
	  HIDE_DUP( PORT_DIPSETTING( 0x0d, DEF_STR( Japan ) ) )          // Japan  |Europe |On    |-                 |Off |On   |On
	  HIDE_DUP( PORT_DIPSETTING( 0x0e, DEF_STR( Japan ) ) )          // Japan  |Europe |On    |-                 |Off |On   |On
	  HIDE_DUP( PORT_DIPSETTING( 0x0f, DEF_STR( Japan ) ) )          // Japan  |Europe |On    |-                 |Off |On   |On
	            PORT_DIPSETTING( 0x03, frqstr_hongkong )             // Japan  |Europe |On    |-                 |Off |Off  |Off
	            PORT_DIPSETTING( 0x08, frqstr_hongkongchina_honest ) // Japan  |Japan  |On    |HONEST TRADING CO.|Off |Off  |Off
	            PORT_DIPSETTING( 0x04, frqstr_korea )                // Japan  |Europe |On    |-                 |Off |Off  |Off
	            PORT_DIPSETTING( 0x05, frqstr_taiwan )               // Japan  |Europe |On    |-                 |Off |Off  |Off
	            PORT_DIPSETTING( 0x06, frqstr_taiwan_spacy )         // Japan  |Europe |On    |SPACY CO.,LTD.    |Off |Off  |Off
	#endif
INPUT_PORTS_END

static INPUT_PORTS_START( outzonea ) // outzonea and outzoned
	PORT_INCLUDE( input_common_outzone )

	#if ! DEBUG_FREE_ALL_DIPSW
	  PORT_MODIFY("DSWA")
	  TOAPLAN_DIP_A5678_COINAGE_DUAL(TJUMP, 0x0f, 0x02, SW1) // mask{0x0f} & value{0x02} -> Europe

	  PORT_MODIFY("TJUMP")
	  // region verified 'outzonea','outzoned' (MAME 0.143u7)   // Game   |Service|      |                  |    |     |Stage
	  PORT_DIPNAME(        0x0f, 0x02, DEF_STR( Region ) )      // Mode   |Mode   |      |                  |FBI |Demo |Repeat
	                         PORT_DIPLOCATION("JP:!4,!3,!2,!1") // Coinage|Coinage|Notice|Licensed to       |Logo|Story|Message
	            PORT_DIPSETTING( 0x02, DEF_STR( Europe ) )      // Europe |Europe |On    |-                 |Off |Off  |Off
	            PORT_DIPSETTING( 0x01, DEF_STR( USA ) )         // Japan  |Japan  |On    |-                 |On  |Off  |Off
	            PORT_DIPSETTING( 0x07, frqstr_usa_romstarinc )  // Japan  |Europe |On    |ROMSTAR, INC.     |On  |Off  |Off
	            PORT_DIPSETTING( 0x00, DEF_STR( Japan ) )       // Japan  |Japan  |On    |-                 |Off |On   |On
	  HIDE_DUP( PORT_DIPSETTING( 0x09, DEF_STR( Japan ) ) )     // Japan  |Japan  |On    |-                 |Off |On   |On
	  HIDE_DUP( PORT_DIPSETTING( 0x0a, DEF_STR( Japan ) ) )     // Japan  |Europe |On    |-                 |Off |On   |On
	  HIDE_DUP( PORT_DIPSETTING( 0x0b, DEF_STR( Japan ) ) )     // Japan  |Europe |On    |-                 |Off |On   |On
	  HIDE_DUP( PORT_DIPSETTING( 0x0c, DEF_STR( Japan ) ) )     // Japan  |Europe |On    |-                 |Off |On   |On
	  HIDE_DUP( PORT_DIPSETTING( 0x0d, DEF_STR( Japan ) ) )     // Japan  |Europe |On    |-                 |Off |On   |On
	  HIDE_DUP( PORT_DIPSETTING( 0x0e, DEF_STR( Japan ) ) )     // Japan  |Europe |On    |-                 |Off |On   |On
	  HIDE_DUP( PORT_DIPSETTING( 0x0f, DEF_STR( Japan ) ) )     // Japan  |Europe |On    |-                 |Off |On   |On
	            PORT_DIPSETTING( 0x03, frqstr_hongkong )        // Japan  |Europe |On    |-                 |Off |Off  |Off
	            PORT_DIPSETTING( 0x08, frqstr_hongkong_honest ) // Japan  |Japan  |On    |HONEST TRADING CO.|Off |Off  |Off
	            PORT_DIPSETTING( 0x04, frqstr_korea )           // Japan  |Europe |On    |-                 |Off |Off  |Off
	            PORT_DIPSETTING( 0x05, frqstr_taiwan )          // Japan  |Europe |On    |-                 |Off |Off  |Off
	            PORT_DIPSETTING( 0x06, frqstr_taiwan_spacy )    // Japan  |Europe |On    |SPACY CO.,LTD.    |Off |Off  |Off
	#endif
INPUT_PORTS_END

static INPUT_PORTS_START( outzoneb )
	PORT_INCLUDE( input_common_outzone )

	#if ! DEBUG_FREE_ALL_DIPSW
	  PORT_MODIFY("DSWA")
	  TOAPLAN_DIP_A5678_COINAGE_DUAL(TJUMP, 0x0f, 0x02, SW1) // mask{0x0f} & value{0x02} -> Europe

	  PORT_MODIFY("TJUMP")
	  // region verified 'outzoneb' (MAME 0.143u7)                   // Game   |Service|      |           |    |     |Stage
	  PORT_DIPNAME(        0x0f, 0x02, DEF_STR( Region ) )           // Mode   |Mode   |      |           |FBI |Demo |Repeat
	                              PORT_DIPLOCATION("JP:!4,!3,!2,!1") // Coinage|Coinage|Notice|Licensed to|Logo|Story|Message
	            PORT_DIPSETTING( 0x02, DEF_STR( Europe ) )           // Europe |Europe |On    |-          |Off |Off  |Off
	            PORT_DIPSETTING( 0x0a, "Europe (Japanese coinage)" ) // Japan  |Europe |On    |-          |Off |Off  |Off
	            PORT_DIPSETTING( 0x01, DEF_STR( USA ) )              // Japan  |Japan  |On    |-          |On  |Off  |Off
	  HIDE_DUP( PORT_DIPSETTING( 0x09, DEF_STR( USA ) ) )            // Japan  |Japan  |On    |-          |On  |Off  |Off
	            PORT_DIPSETTING( 0x00, DEF_STR( Japan ) )            // Japan  |Japan  |On    |-          |Off |On   |On
	  HIDE_DUP( PORT_DIPSETTING( 0x08, DEF_STR( Japan ) ) )          // Japan  |Japan  |On    |-          |Off |On   |On
	            PORT_DIPSETTING( 0x03, frqstr_hongkong )             // Japan  |Europe |On    |-          |Off |Off  |Off
	  HIDE_DUP( PORT_DIPSETTING( 0x0b, frqstr_hongkong ) )           // Japan  |Europe |On    |-          |Off |Off  |Off
	            PORT_DIPSETTING( 0x04, frqstr_korea )                // Japan  |Europe |On    |-          |Off |Off  |Off
	  HIDE_DUP( PORT_DIPSETTING( 0x0c, frqstr_korea ) )              // Japan  |Europe |On    |-          |Off |Off  |Off
	            PORT_DIPSETTING( 0x05, frqstr_taiwan )               // Japan  |Europe |On    |-          |Off |Off  |Off
	  HIDE_DUP( PORT_DIPSETTING( 0x0d, frqstr_taiwan ) )             // Japan  |Europe |On    |-          |Off |Off  |Off
	            PORT_DIPSETTING( 0x06, frqstr_no_country )           // Japan  |Europe |Off   |-          |Off |Off  |Off
	  HIDE_DUP( PORT_DIPSETTING( 0x07, frqstr_no_country ) )         // Japan  |Europe |Off   |-          |Off |Off  |Off
	  HIDE_DUP( PORT_DIPSETTING( 0x0e, frqstr_no_country ) )         // Japan  |Europe |Off   |-          |Off |Off  |Off
	  HIDE_DUP( PORT_DIPSETTING( 0x0f, frqstr_no_country ) )         // Japan  |Europe |Off   |-          |Off |Off  |Off
	#endif
INPUT_PORTS_END


/* verified from M68000 - coinage based on "test mode" and handled by the MCU simulation */
static INPUT_PORTS_START( input_common_vimana )
	PORT_INCLUDE( toaplan1_type1 )

	#if ! DEBUG_FREE_ALL_DIPSW
	  /* 0x440007.b */
	  PORT_MODIFY("DSWA")
	  //DIP_A1_SPARE (divert from 'toaplan1_type1')
	  TOAPLAN_DIP_A2_FLIP_SCREEN(SW1)
	  TOAPLAN_DIP_A3_SERVICE_MODE(SW1)
	  TOAPLAN_DIP_A4_DEMO_SOUNDS(SW1)
	  //DIP_A5678_SPARE (divert from 'toaplan1_type1') (Specified by each set)

	  /* 0x44000f.b */
	  PORT_MODIFY("DSWB")
	  TOAPLAN_DIP_B12_DIFFICULTY(SW2)
	  PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:!3,!4") /* table at 0x000998 */
	  PORT_DIPSETTING(    0x00, "70k 270k 200k+" )
	  PORT_DIPSETTING(    0x04, "100k 350k 250k+" )
	  PORT_DIPSETTING(    0x08, "100k Only" )
	  PORT_DIPSETTING(    0x0c, "200k Only" )
	  PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:!5,!6")
	  PORT_DIPSETTING(    0x30, "2" )
	  PORT_DIPSETTING(    0x00, "3" )
	  PORT_DIPSETTING(    0x20, "4" )
	  PORT_DIPSETTING(    0x10, "5" )
	  TOAPLAN_DIP_B7_INVULNERABILITY(SW2)
	  TOAPLAN_DIP_B8_ALLOW_CONTINUE_OFF_YES(SW2)

	  /* 0x440011.b */
	  //PORT_MODIFY("TJUMP")
	  //JP_4321_SPARE (divert from 'toaplan1_type1') (Specified by each set)
	#endif

	/* P1 : 0x44000b.b */
	PORT_MODIFY("P1")
	#if VIMANA_P1_BUTTON_3_DESCRIBE_LEVEL == 1
	  PORT_BIT(0x40, TOAPLAN_IP_ACTIVE_LEVEL, IPT_BUTTON3) PORT_PLAYER(1) PORT_NAME("Spare (P1 Button 3) (Fast Scrolling If Invulnerable)") /* JAMMA "P1 button 3" */
	#elif VIMANA_P1_BUTTON_3_DESCRIBE_LEVEL == 2
	  PORT_BIT(0x40, TOAPLAN_IP_ACTIVE_LEVEL, IPT_BUTTON3) PORT_PLAYER(1) PORT_NAME("(Fast Scrolling If Invulnerable)")
	#endif
	#if VIMANA_P1_BUTTON_3_MOVE_TO_F1
	  PORT_CODE(KEYCODE_F1)
	#endif

	/* P2 : 0x44000d.b */
	PORT_MODIFY("P2")
	#if VIMANA_P2_BUTTON_3_DESCRIBE_LEVEL == 1
	  PORT_BIT(0x40, TOAPLAN_IP_ACTIVE_LEVEL, IPT_BUTTON3) PORT_PLAYER(2) PORT_NAME("Spare (P2 Button 3) (Go To Next Stage If Invulnerable)") /* JAMMA "P1 button 3" */
	#elif VIMANA_P2_BUTTON_3_DESCRIBE_LEVEL == 2
	  PORT_BIT(0x40, TOAPLAN_IP_ACTIVE_LEVEL, IPT_BUTTON3) PORT_PLAYER(2) PORT_NAME("(Go To Next Stage If Invulnerable)")
	#endif
	#if VIMANA_P2_BUTTON_3_MOVE_TO_F1
	  PORT_CODE(KEYCODE_F1)
	#endif

	/* SYSTEM : 0x440009.b */

	/* VBLANK : 0x400001.b */
INPUT_PORTS_END

static INPUT_PORTS_START( vimana )
	PORT_INCLUDE( input_common_vimana )

	#if ! DEBUG_FREE_ALL_DIPSW
	  PORT_MODIFY("DSWA")
	  TOAPLAN_DIP_A5678_COINAGE_DUAL(TJUMP, 0x0f, 0x02, SW1)

	  PORT_MODIFY("TJUMP")
	  // region verified 'vimana' (MAME 0.143u7)                     // Game   |Service|      |Distributed by    |    |
	  PORT_DIPNAME(        0x0f, 0x02, DEF_STR( Region ) )           // Mode   |Mode   |      |or                |FBI |End
	                              PORT_DIPLOCATION("JP:!4,!3,!2,!1") // Coinage|Coinage|Notice|Licensed to       |Logo|Story
	            PORT_DIPSETTING( 0x02, DEF_STR( Europe ) )           // Europe |Europe |On    |-                 |Off |English
	            PORT_DIPSETTING( 0x01, DEF_STR( USA ) )              // Japan  |Japan  |On    |-                 |On  |English
	            PORT_DIPSETTING( 0x07, frqstr_usa_romstarinc )       // Japan  |Japan  |On    |ROMSTAR, INC.     |On  |English
	            PORT_DIPSETTING( 0x00, frqstr_japan_tecmo )          // Japan  |Japan  |On    |TECMO             |Off |English
	  HIDE_DUP( PORT_DIPSETTING( 0x0f, frqstr_japan_tecmo ) )        // Japan  |Japan  |On    |TECMO             |Off |English
	            PORT_DIPSETTING( 0x04, frqstr_korea )                // Japan  |Japan  |On    |-                 |Off |English
	            PORT_DIPSETTING( 0x03, frqstr_hongkong )             // Japan  |Japan  |On    |-                 |Off |English
	            PORT_DIPSETTING( 0x08, frqstr_hongkongchina_honest ) // Japan  |Japan  |On    |HONEST TRADING CO.|Off |English
	            PORT_DIPSETTING( 0x05, frqstr_taiwan )               // Japan  |Japan  |On    |-                 |Off |English
	            PORT_DIPSETTING( 0x06, frqstr_taiwan_spacy )         // Japan  |Japan  |On    |SPACY CO.,LTD     |Off |English
	            PORT_DIPSETTING( 0x09, frqstr_no_country )           // Japan  |Japan  |On    |-                 |Off |English
	  HIDE_DUP( PORT_DIPSETTING( 0x0a, frqstr_no_country ) )         // Japan  |Japan  |On    |-                 |Off |English
	  HIDE_DUP( PORT_DIPSETTING( 0x0b, frqstr_no_country ) )         // Japan  |Japan  |On    |-                 |Off |English
	  HIDE_DUP( PORT_DIPSETTING( 0x0c, frqstr_no_country ) )         // Japan  |Japan  |On    |-                 |Off |English
	  HIDE_DUP( PORT_DIPSETTING( 0x0d, frqstr_no_country ) )         // Japan  |Japan  |On    |-                 |Off |English
	  HIDE_DUP( PORT_DIPSETTING( 0x0e, frqstr_no_country ) )         // Japan  |Japan  |On    |-                 |Off |English
	#endif
INPUT_PORTS_END

static INPUT_PORTS_START( vimanan )
	PORT_INCLUDE( input_common_vimana )

	#if ! DEBUG_FREE_ALL_DIPSW
	  PORT_MODIFY("DSWA")
	  TOAPLAN_DIP_A5678_COINAGE_DUAL(TJUMP, 0x0f, 0x02, SW1)

	  PORT_MODIFY("TJUMP")
	  // region verified 'vimanan' (MAME 0.143u7)                    // Game   |Service|      |Distributed by         |    |
	  PORT_DIPNAME(        0x0f, 0x02, DEF_STR( Region ) )           // Mode   |Mode   |      |or                     |FBI |End
	                              PORT_DIPLOCATION("JP:!4,!3,!2,!1") // Coinage|Coinage|Notice|Licensed to            |Logo|Story
	            PORT_DIPSETTING( 0x02, frqstr_europe_nova )          // Europe |Europe |On    |NOVA APPARATE GMBH & CO|Off |English
	            PORT_DIPSETTING( 0x01, DEF_STR( USA ) )              // Japan  |Japan  |On    |-                      |On  |English
	            PORT_DIPSETTING( 0x07, frqstr_usa_romstarinc )       // Japan  |Japan  |On    |ROMSTAR, INC.          |On  |English
	            PORT_DIPSETTING( 0x00, frqstr_japan_tecmo )          // Japan  |Japan  |On    |TECMO                  |Off |English
	  HIDE_DUP( PORT_DIPSETTING( 0x0f, frqstr_japan_tecmo ) )        // Japan  |Japan  |On    |TECMO                  |Off |English
	            PORT_DIPSETTING( 0x04, frqstr_korea )                // Japan  |Japan  |On    |-                      |Off |English
	            PORT_DIPSETTING( 0x03, frqstr_hongkong )             // Japan  |Japan  |On    |-                      |Off |English
	            PORT_DIPSETTING( 0x08, frqstr_hongkongchina_honest ) // Japan  |Japan  |On    |HONEST TRADING CO.     |Off |English
	            PORT_DIPSETTING( 0x05, frqstr_taiwan )               // Japan  |Japan  |On    |-                      |Off |English
	            PORT_DIPSETTING( 0x06, frqstr_taiwan_spacy )         // Japan  |Japan  |On    |SPACY CO.,LTD          |Off |English
	            PORT_DIPSETTING( 0x09, frqstr_no_country )           // Japan  |Japan  |On    |-                      |Off |English
	  HIDE_DUP( PORT_DIPSETTING( 0x0a, frqstr_no_country ) )         // Japan  |Japan  |On    |-                      |Off |English
	  HIDE_DUP( PORT_DIPSETTING( 0x0b, frqstr_no_country ) )         // Japan  |Japan  |On    |-                      |Off |English
	  HIDE_DUP( PORT_DIPSETTING( 0x0c, frqstr_no_country ) )         // Japan  |Japan  |On    |-                      |Off |English
	  HIDE_DUP( PORT_DIPSETTING( 0x0d, frqstr_no_country ) )         // Japan  |Japan  |On    |-                      |Off |English
	  HIDE_DUP( PORT_DIPSETTING( 0x0e, frqstr_no_country ) )         // Japan  |Japan  |On    |-                      |Off |English
	#endif
INPUT_PORTS_END

static INPUT_PORTS_START( vimana1 )
	PORT_INCLUDE( input_common_vimana )

	#if ! DEBUG_FREE_ALL_DIPSW
	  PORT_MODIFY("DSWA")
	  TOAPLAN_DIP_A5678_COINAGE_DUAL(TJUMP, 0x0f, 0x02, SW1)

	  PORT_MODIFY("TJUMP")
	  // region verified 'vimana1' (MAME 0.143u7)                    // Game   |Service|      |Distributed by    |    |
	  PORT_DIPNAME(        0x0f, 0x00, DEF_STR( Region ) )           // Mode   |Mode   |      |or                |FBI |End
	                              PORT_DIPLOCATION("JP:!4,!3,!2,!1") // Coinage|Coinage|Notice|Licensed to       |Logo|Story
	            PORT_DIPSETTING( 0x02, DEF_STR( Europe ) )           // Europe |Europe |On    |-                 |Off |Japanese
	            PORT_DIPSETTING( 0x01, DEF_STR( USA ) )              // Japan  |Japan  |On    |-                 |On  |Japanese
	            PORT_DIPSETTING( 0x07, frqstr_usa_romstarinc )       // Japan  |Japan  |On    |ROMSTAR, INC.     |On  |Japanese
	            PORT_DIPSETTING( 0x00, frqstr_japan_tecmo )          // Japan  |Japan  |On    |TECMO             |Off |Japanese
	  HIDE_DUP( PORT_DIPSETTING( 0x0f, frqstr_japan_tecmo ) )        // Japan  |Japan  |On    |TECMO             |Off |Japanese
	            PORT_DIPSETTING( 0x04, frqstr_korea )                // Japan  |Japan  |On    |-                 |Off |Japanese
	            PORT_DIPSETTING( 0x03, frqstr_hongkong )             // Japan  |Japan  |On    |-                 |Off |Japanese
	            PORT_DIPSETTING( 0x08, frqstr_hongkongchina_honest ) // Japan  |Japan  |On    |HONEST TRADING CO.|Off |Japanese
	            PORT_DIPSETTING( 0x05, frqstr_taiwan )               // Japan  |Japan  |On    |-                 |Off |Japanese
	            PORT_DIPSETTING( 0x06, frqstr_taiwan_spacy )         // Japan  |Japan  |On    |SPACY CO.,LTD     |Off |Japanese
	            PORT_DIPSETTING( 0x09, frqstr_no_country )           // Japan  |Japan  |On    |-                 |Off |Japanese
	  HIDE_DUP( PORT_DIPSETTING( 0x0a, frqstr_no_country ) )         // Japan  |Japan  |On    |-                 |Off |Japanese
	  HIDE_DUP( PORT_DIPSETTING( 0x0b, frqstr_no_country ) )         // Japan  |Japan  |On    |-                 |Off |Japanese
	  HIDE_DUP( PORT_DIPSETTING( 0x0c, frqstr_no_country ) )         // Japan  |Japan  |On    |-                 |Off |Japanese
	  HIDE_DUP( PORT_DIPSETTING( 0x0d, frqstr_no_country ) )         // Japan  |Japan  |On    |-                 |Off |Japanese
	  HIDE_DUP( PORT_DIPSETTING( 0x0e, frqstr_no_country ) )         // Japan  |Japan  |On    |-                 |Off |Japanese
	#endif
INPUT_PORTS_END




static const gfx_layout tilelayout =
{
	8,8,	/* 8x8 */
	16384,	/* 16384 tiles */
	4,		/* 4 bits per pixel */
	{ 3*8*0x20000, 2*8*0x20000, 1*8*0x20000, 0*8*0x20000 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38 },
	64
};

static const gfx_layout rallybik_spr_layout =
{
	16,16,	/* 16*16 sprites */
	2048,	/* 2048 sprites */
	4,		/* 4 bits per pixel */
	{ 0*2048*32*8, 1*2048*32*8, 2*2048*32*8, 3*2048*32*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	32*8	/* every sprite takes 32 consecutive bytes */
};

static const gfx_layout vm_tilelayout =
{
	8,8,	/* 8x8 */
	32768,	/* 32768 tiles */
	4,		/* 4 bits per pixel */
	{ 8*0x80000+8, 8*0x80000, 8, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70 },
	128
};


static GFXDECODE_START( toaplan1 )
	GFXDECODE_ENTRY( "gfx1", 0x00000, tilelayout,		0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0x00000, tilelayout,	64*16, 64 )
GFXDECODE_END

static GFXDECODE_START( rallybik )
	GFXDECODE_ENTRY( "gfx1", 0x00000, tilelayout,			  0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0x00000, rallybik_spr_layout, 64*16, 64 )
GFXDECODE_END

static GFXDECODE_START( outzone )
	GFXDECODE_ENTRY( "gfx1", 0x00000, vm_tilelayout,	0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0x00000, tilelayout,	64*16, 64 )
GFXDECODE_END

static GFXDECODE_START( vm )
	GFXDECODE_ENTRY( "gfx1", 0x00000, tilelayout,		0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0x00000, vm_tilelayout, 64*16, 64 )
GFXDECODE_END


static void irqhandler(device_t *device, int linestate)
{
	cputag_set_input_line(device->machine(), "audiocpu", 0, linestate);
}

static const ym3812_interface ym3812_config =
{
	irqhandler
};



static MACHINE_CONFIG_START( rallybik, toaplan1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_10MHz)
	MCFG_CPU_PROGRAM_MAP(rallybik_main_map)
	MCFG_CPU_VBLANK_INT("screen", toaplan1_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_28MHz/8)
	MCFG_CPU_PROGRAM_MAP(toaplan1_sound_map)
	MCFG_CPU_IO_MAP(rallybik_sound_io_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_MACHINE_RESET(toaplan1)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(55.14)		/* verified on pcb */
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(512, 512)	/* 512x288 active */
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE(rallybik)
	MCFG_SCREEN_EOF(rallybik)

	MCFG_GFXDECODE(rallybik)
	MCFG_PALETTE_LENGTH((64*16)+(64*16))

	MCFG_VIDEO_START(rallybik)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_28MHz/8)
	MCFG_SOUND_CONFIG(ym3812_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( truxton, toaplan1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_10MHz)
	MCFG_CPU_PROGRAM_MAP(truxton_main_map)
	MCFG_CPU_VBLANK_INT("screen", toaplan1_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_28MHz/8)
	MCFG_CPU_PROGRAM_MAP(toaplan1_sound_map)
	MCFG_CPU_IO_MAP(truxton_sound_io_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_MACHINE_RESET(toaplan1)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(57.59)		/* verified on pcb */
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(512, 512)	/* 512x320 active */
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE(toaplan1)
	MCFG_SCREEN_EOF(toaplan1)

	MCFG_GFXDECODE(toaplan1)
	MCFG_PALETTE_LENGTH((64*16)+(64*16))

	MCFG_VIDEO_START(toaplan1)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_28MHz/8)
	MCFG_SOUND_CONFIG(ym3812_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( hellfire, toaplan1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_10MHz)
	MCFG_CPU_PROGRAM_MAP(hellfire_main_map)
	MCFG_CPU_VBLANK_INT("screen", toaplan1_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_28MHz/8)
	MCFG_CPU_PROGRAM_MAP(toaplan1_sound_map)
	MCFG_CPU_IO_MAP(hellfire_sound_io_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_MACHINE_RESET(toaplan1)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(512, 512)	/* 512x240 active */
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 16, 255)
	MCFG_SCREEN_UPDATE(toaplan1)
	MCFG_SCREEN_EOF(toaplan1)

	MCFG_GFXDECODE(toaplan1)
	MCFG_PALETTE_LENGTH((64*16)+(64*16))

	MCFG_VIDEO_START(toaplan1)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_28MHz/8)
	MCFG_SOUND_CONFIG(ym3812_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( zerowing, toaplan1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_10MHz)
	MCFG_CPU_PROGRAM_MAP(zerowing_main_map)
	MCFG_CPU_VBLANK_INT("screen", toaplan1_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_28MHz/8)
	MCFG_CPU_PROGRAM_MAP(toaplan1_sound_map)
	MCFG_CPU_IO_MAP(zerowing_sound_io_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_MACHINE_RESET(zerowing)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE( (XTAL_28MHz / 4) / (450 * 282) )	/* fixed by SUZ */
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(512, 512)	/* 512x240 mostly active, 512x512 actually used */
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 16, 255)
	MCFG_SCREEN_UPDATE(toaplan1)
	MCFG_SCREEN_EOF(toaplan1)

	MCFG_GFXDECODE(toaplan1)
	MCFG_PALETTE_LENGTH((64*16)+(64*16))

	MCFG_VIDEO_START(toaplan1)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_28MHz/8)
	MCFG_SOUND_CONFIG(ym3812_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( demonwld, toaplan1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_10MHz)
	MCFG_CPU_PROGRAM_MAP(demonwld_main_map)
	MCFG_CPU_VBLANK_INT("screen", toaplan1_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_28MHz/8)
	MCFG_CPU_PROGRAM_MAP(toaplan1_sound_map)
	MCFG_CPU_IO_MAP(demonwld_sound_io_map)

	MCFG_CPU_ADD("dsp", TMS32010, XTAL_28MHz/2)
	MCFG_CPU_PROGRAM_MAP(DSP_program_map)
	MCFG_CPU_IO_MAP(DSP_io_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_MACHINE_RESET(demonwld)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(55.14)		/* verified on pcb */
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(512, 512)	/* 512x240 active */
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 16, 255)
	MCFG_SCREEN_UPDATE(toaplan1)
	MCFG_SCREEN_EOF(toaplan1)

	MCFG_GFXDECODE(toaplan1)
	MCFG_PALETTE_LENGTH((64*16)+(64*16))

	MCFG_VIDEO_START(toaplan1)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_28MHz/8)
	MCFG_SOUND_CONFIG(ym3812_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( samesame, toaplan1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_10MHz)
	MCFG_CPU_PROGRAM_MAP(samesame_main_map)
	MCFG_CPU_VBLANK_INT("screen", toaplan1_interrupt)

	MCFG_CPU_ADD("audiocpu", Z180, XTAL_28MHz/8)	/* HD647180XOFS6 CPU */
	MCFG_CPU_PROGRAM_MAP(hd647180_mem_map)
	MCFG_DEVICE_DISABLE()		/* Internal code is not dumped */

	MCFG_MACHINE_RESET(toaplan1)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(512, 512)	/* 512x320 active */
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE(toaplan1)
	MCFG_SCREEN_EOF(samesame)

	MCFG_GFXDECODE(toaplan1)
	MCFG_PALETTE_LENGTH((64*16)+(64*16))

	MCFG_VIDEO_START(toaplan1)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_28MHz/8)
	MCFG_SOUND_CONFIG(ym3812_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( outzone, toaplan1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_10MHz)
	MCFG_CPU_PROGRAM_MAP(outzone_main_map)
	MCFG_CPU_VBLANK_INT("screen", toaplan1_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_28MHz/8)
	MCFG_CPU_PROGRAM_MAP(toaplan1_sound_map)
	MCFG_CPU_IO_MAP(outzone_sound_io_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_MACHINE_RESET(zerowing)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(512, 512)	/* 512x256 active */
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE(toaplan1)
	MCFG_SCREEN_EOF(toaplan1)

	MCFG_GFXDECODE(outzone)
	MCFG_PALETTE_LENGTH((64*16)+(64*16))

	MCFG_VIDEO_START(toaplan1)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_28MHz/8)
	MCFG_SOUND_CONFIG(ym3812_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( vimana, toaplan1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_10MHz)	/* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(vimana_main_map)
	MCFG_CPU_VBLANK_INT("screen", toaplan1_interrupt)

	MCFG_CPU_ADD("audiocpu", Z180, XTAL_28MHz/8)	/* HD647180XOFS6 CPU */
	MCFG_CPU_PROGRAM_MAP(hd647180_mem_map)
	MCFG_DEVICE_DISABLE()		/* Internal code is not dumped */

	MCFG_MACHINE_RESET(vimana)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(57.59)		/* verified on pcb */
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(512, 512)	/* 512x256 active */
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE(toaplan1)
	MCFG_SCREEN_EOF(toaplan1)

	MCFG_GFXDECODE(vm)
	MCFG_PALETTE_LENGTH((64*16)+(64*16))

	MCFG_VIDEO_START(toaplan1)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_28MHz/8)	/* verified on pcb */
	MCFG_SOUND_CONFIG(ym3812_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END




/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( rallybik )
	ROM_REGION( 0x080000, "maincpu", 0 )	/* Main 68K code */
	ROM_LOAD16_BYTE( "b45-02.rom",  0x000000, 0x08000, CRC(383386d7) SHA1(fc420b6adc79a408a68f0661d0c62ed7dbe8b6d7) )
	ROM_LOAD16_BYTE( "b45-01.rom",  0x000001, 0x08000, CRC(7602f6a7) SHA1(2939c261a4bc63586681080f5643916c85e81c7d) )
	ROM_LOAD16_BYTE( "b45-04.rom",  0x040000, 0x20000, CRC(e9b005b1) SHA1(19b5acfd5fb2683a56a701400b11ee6f64a9bdf1) )
	ROM_LOAD16_BYTE( "b45-03.rom",  0x040001, 0x20000, CRC(555344ce) SHA1(398963f488fe6f19c0b8518d80c946c242d0fc45) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound Z80 code */
	ROM_LOAD( "b45-05.rom",  0x0000, 0x4000, CRC(10814601) SHA1(bad7a834d8849752a7f3000bb5154ec0fa50d695) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b45-09.bin",  0x00000, 0x20000, CRC(1dc7b010) SHA1(67e8633bd787ffcae0e7867e7e591c492c4f2d63) )
	ROM_LOAD( "b45-08.bin",  0x20000, 0x20000, CRC(fab661ba) SHA1(acc43cd6d979b1c6a348727f315643d7b8f1496a) )
	ROM_LOAD( "b45-07.bin",  0x40000, 0x20000, CRC(cd3748b4) SHA1(a20eb19a0f813112b4e5d9cd91db29de9b37af17) )
	ROM_LOAD( "b45-06.bin",  0x60000, 0x20000, CRC(144b085c) SHA1(84b7412d58fe9c5e9915896db92e80a621571b74) )

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "b45-11.rom",  0x00000, 0x10000, CRC(0d56e8bb) SHA1(c29cb53f846c73b7cf9936051fb0f9dd3805f53f) )
	ROM_LOAD( "b45-10.rom",  0x10000, 0x10000, CRC(dbb7c57e) SHA1(268132965cd65b5e972ca9d0258c30b8a86f3703) )
	ROM_LOAD( "b45-12.rom",  0x20000, 0x10000, CRC(cf5aae4e) SHA1(5832c52d2e9b86414d8ee2926fa190abe9e41da4) )
	ROM_LOAD( "b45-13.rom",  0x30000, 0x10000, CRC(1683b07c) SHA1(54356893357cd1297f24f1d85b7289d80740262d) )

	ROM_REGION( 0x240, "proms", 0 )		/* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "b45-15.bpr",  0x000, 0x100, CRC(24e7d62f) SHA1(1c06a1ef1b6a722794ca1d5ee2c476ecaa5178a3) )	/* sprite priority control ?? */
	ROM_LOAD( "b45-16.bpr",  0x100, 0x100, CRC(a50cef09) SHA1(55cafb5b2551b80ae708e9b966cf37c70a16d310) )	/* sprite priority control ?? */
	ROM_LOAD( "b45-14.bpr",  0x200, 0x020, CRC(f72482db) SHA1(b0cb911f9c81f6088a5aa8760916ddae1f8534d7) )	/* sprite control ?? */
	ROM_LOAD( "b45-17.bpr",  0x220, 0x020, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )	/* sprite attribute (flip/position) ?? */
ROM_END

ROM_START( truxton )
	ROM_REGION( 0x040000, "maincpu", 0 )	/* Main 68K code */
	ROM_LOAD16_BYTE( "b65_11.bin",  0x000000, 0x20000, CRC(1a62379a) SHA1(b9470d4b70c38f2523b22636874d742abe4099eb) )
	ROM_LOAD16_BYTE( "b65_10.bin",  0x000001, 0x20000, CRC(aff5195d) SHA1(a7f379dc35e3acf9e7a8ae8a47a9b5b4193f93a1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound Z80 code */
	ROM_LOAD( "b65_09.bin",  0x0000, 0x8000, CRC(f1c0f410) SHA1(05deb759f8acb14fff92c56b536134cfd84516a8) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b65_08.bin",  0x00000, 0x20000, CRC(d2315b37) SHA1(eb42a884df319728c830c067c2423043ed4536ee) )
	ROM_LOAD( "b65_07.bin",  0x20000, 0x20000, CRC(fb83252a) SHA1(48a38584d223f56286137f7acdfaec86ee6588e7) )
	ROM_LOAD( "b65_06.bin",  0x40000, 0x20000, CRC(36cedcbe) SHA1(f79d4b1e98b3c9091ae907fb671ad201d3698b42) )
	ROM_LOAD( "b65_05.bin",  0x60000, 0x20000, CRC(81cd95f1) SHA1(526a437fbe033ac21054ee5c3bf1ba2fed354c7a) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "b65_04.bin",  0x00000, 0x20000, CRC(8c6ff461) SHA1(5199e31f4eb23bad01f7d1079f3618fe39d8a32e) )
	ROM_LOAD( "b65_03.bin",  0x20000, 0x20000, CRC(58b1350b) SHA1(7eb2fe329579a6f651d3c1aed9155ac6ffefbc4b) )
	ROM_LOAD( "b65_02.bin",  0x40000, 0x20000, CRC(1dd55161) SHA1(c537456ac56801dea0ac48fb1389228530d00a61) )
	ROM_LOAD( "b65_01.bin",  0x60000, 0x20000, CRC(e974937f) SHA1(ab282472c04ce6d9ed368956c427403275bc9080) )

	ROM_REGION( 0x40, "proms", 0 )		/* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "b65_12.bpr",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )	/* sprite attribute (flip/position) ?? */
	ROM_LOAD( "b65_13.bpr",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )	/* ??? */
ROM_END

ROM_START( hellfire )
	ROM_REGION( 0x040000, "maincpu", 0 )	/* Main 68K code */
	ROM_LOAD16_BYTE( "b90_14.0",   0x000000, 0x20000, CRC(101df9f5) SHA1(27e1430d4c96fe2c830143999a760470c8381ada) )
	ROM_LOAD16_BYTE( "b90_15.1",   0x000001, 0x20000, CRC(e67fd452) SHA1(baec2a702238f000d0499705d79d7c7577fc2279) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound Z80 code */
	ROM_LOAD( "b90_03.2",   0x0000, 0x8000, CRC(4058fa67) SHA1(155c364273c270cd74955f447efc804bb4c9b560) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b90_04.3",   0x00000, 0x20000, CRC(ea6150fc) SHA1(1116947d10ce14fbc6a3b86368fc2024c6f51803) )
	ROM_LOAD( "b90_05.4",   0x20000, 0x20000, CRC(bb52c507) SHA1(b0b1821476647f10c7023f92a66a7f54b92f50c3) )
	ROM_LOAD( "b90_06.5",   0x40000, 0x20000, CRC(cf5b0252) SHA1(e2102967af61afb11d2290a40d13d2faf9ef1e12) )
	ROM_LOAD( "b90_07.6",   0x60000, 0x20000, CRC(b98af263) SHA1(54d636a50a41dbb58b54c22dfab3eabfdb452575) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "b90_11.10",  0x00000, 0x20000, CRC(c33e543c) SHA1(b85cba30cc651f820aeedd41e04584df92078ed9) )
	ROM_LOAD( "b90_10.9",   0x20000, 0x20000, CRC(35fd1092) SHA1(5e136a35eea45034ccd4aea52cc0ffeec944e27e) )
	ROM_LOAD( "b90_09.8",   0x40000, 0x20000, CRC(cf01009e) SHA1(e260c479fa97f23a65c220e5071aaf2dc2baf46d) )
	ROM_LOAD( "b90_08.7",   0x60000, 0x20000, CRC(3404a5e3) SHA1(f717b9e31c2a093dbb060b8ea54a8c3f52688d7a) )

	ROM_REGION( 0x40, "proms", 0 )		/* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "13.3w",     0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )	/* N82S123AN bprom - sprite attribute (flip/position) ?? */
	ROM_LOAD( "12.6b",     0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )	/* N82S123AN bprom -  ??? */
ROM_END

ROM_START( hellfire1 )
	ROM_REGION( 0x040000, "maincpu", 0 )	/* Main 68K code */
	ROM_LOAD16_BYTE( "b90_14x.0",   0x000000, 0x20000, CRC(a3141ea5) SHA1(9b456cb908e193198110a628d98567a3b8351591) )
	ROM_LOAD16_BYTE( "b90_15x.1",   0x000001, 0x20000, CRC(e864daf4) SHA1(382f02df8419310cef5d7fb68a9376eeac2f3685) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound Z80 code */
	ROM_LOAD( "b90_03x.2",  0x0000, 0x8000, CRC(f58c368f) SHA1(2ee5396a4b70a3374f3a3bbd791b1d962f6a8a52) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b90_04.3",   0x00000, 0x20000, CRC(ea6150fc) SHA1(1116947d10ce14fbc6a3b86368fc2024c6f51803) )
	ROM_LOAD( "b90_05.4",   0x20000, 0x20000, CRC(bb52c507) SHA1(b0b1821476647f10c7023f92a66a7f54b92f50c3) )
	ROM_LOAD( "b90_06.5",   0x40000, 0x20000, CRC(cf5b0252) SHA1(e2102967af61afb11d2290a40d13d2faf9ef1e12) )
	ROM_LOAD( "b90_07.6",   0x60000, 0x20000, CRC(b98af263) SHA1(54d636a50a41dbb58b54c22dfab3eabfdb452575) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "b90_11.10",  0x00000, 0x20000, CRC(c33e543c) SHA1(b85cba30cc651f820aeedd41e04584df92078ed9) )
	ROM_LOAD( "b90_10.9",   0x20000, 0x20000, CRC(35fd1092) SHA1(5e136a35eea45034ccd4aea52cc0ffeec944e27e) )
	ROM_LOAD( "b90_09.8",   0x40000, 0x20000, CRC(cf01009e) SHA1(e260c479fa97f23a65c220e5071aaf2dc2baf46d) )
	ROM_LOAD( "b90_08.7",   0x60000, 0x20000, CRC(3404a5e3) SHA1(f717b9e31c2a093dbb060b8ea54a8c3f52688d7a) )

	ROM_REGION( 0x40, "proms", 0 )		/* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "13.3w",     0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )	/* N82S123AN bprom - sprite attribute (flip/position) ?? */
	ROM_LOAD( "12.6b",     0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )	/* N82S123AN bprom -  ??? */
ROM_END

ROM_START( hellfire2 )/* Original version, by rom numbers (IE: 01 & 02) */
	ROM_REGION( 0x040000, "maincpu", 0 )	/* Main 68K code */
	ROM_LOAD16_BYTE( "b90_01.0",   0x000000, 0x20000, CRC(c94acf53) SHA1(5710861dbe976fe53b93d3428147d1ce7aaae18a) ) /* Territory block seems to have no effect and it's licensed */
	ROM_LOAD16_BYTE( "b90_02.1",   0x000001, 0x20000, CRC(d17f03c3) SHA1(ac41e6c29aa507872caeeaec6a3bc24c705a3702) ) /* to "Taito Corp." the later set shows "Taito Corporation" */

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound Z80 code */
	ROM_LOAD( "b90_03x.2",  0x0000, 0x8000, CRC(f58c368f) SHA1(2ee5396a4b70a3374f3a3bbd791b1d962f6a8a52) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b90_04.3",   0x00000, 0x20000, CRC(ea6150fc) SHA1(1116947d10ce14fbc6a3b86368fc2024c6f51803) )
	ROM_LOAD( "b90_05.4",   0x20000, 0x20000, CRC(bb52c507) SHA1(b0b1821476647f10c7023f92a66a7f54b92f50c3) )
	ROM_LOAD( "b90_06.5",   0x40000, 0x20000, CRC(cf5b0252) SHA1(e2102967af61afb11d2290a40d13d2faf9ef1e12) )
	ROM_LOAD( "b90_07.6",   0x60000, 0x20000, CRC(b98af263) SHA1(54d636a50a41dbb58b54c22dfab3eabfdb452575) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "b90_11.10",  0x00000, 0x20000, CRC(c33e543c) SHA1(b85cba30cc651f820aeedd41e04584df92078ed9) )
	ROM_LOAD( "b90_10.9",   0x20000, 0x20000, CRC(35fd1092) SHA1(5e136a35eea45034ccd4aea52cc0ffeec944e27e) )
	ROM_LOAD( "b90_09.8",   0x40000, 0x20000, CRC(cf01009e) SHA1(e260c479fa97f23a65c220e5071aaf2dc2baf46d) )
	ROM_LOAD( "b90_08.7",   0x60000, 0x20000, CRC(3404a5e3) SHA1(f717b9e31c2a093dbb060b8ea54a8c3f52688d7a) )

	ROM_REGION( 0x40, "proms", 0 )		/* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "13.3w",     0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )	/* N82S123AN bprom - sprite attribute (flip/position) ?? */
	ROM_LOAD( "12.6b",     0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )	/* N82S123AN bprom -  ??? */
ROM_END

ROM_START( hellfire3 )
	ROM_REGION( 0x040000, "maincpu", 0 )	/* Main 68K code */
	ROM_LOAD16_BYTE( "b90_01.10m",  0x000000, 0x20000, CRC(034966d3) SHA1(f987d8e7ebe6a546be621fe4d5a59de1284c4ebb) ) /* Same labels as hellfire2 but different data */
	ROM_LOAD16_BYTE( "b90_02.9m",   0x000001, 0x20000, CRC(06dd24c7) SHA1(a990de7ffac6bd0dd219c7bf9f773ccb41395be6) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound Z80 code */
	ROM_LOAD( "b90_03.2",   0x0000, 0x8000, CRC(4058fa67) SHA1(155c364273c270cd74955f447efc804bb4c9b560) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b90_04.3",   0x00000, 0x20000, CRC(ea6150fc) SHA1(1116947d10ce14fbc6a3b86368fc2024c6f51803) )
	ROM_LOAD( "b90_05.4",   0x20000, 0x20000, CRC(bb52c507) SHA1(b0b1821476647f10c7023f92a66a7f54b92f50c3) )
	ROM_LOAD( "b90_06.5",   0x40000, 0x20000, CRC(cf5b0252) SHA1(e2102967af61afb11d2290a40d13d2faf9ef1e12) )
	ROM_LOAD( "b90_07.6",   0x60000, 0x20000, CRC(b98af263) SHA1(54d636a50a41dbb58b54c22dfab3eabfdb452575) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "b90_11.10",  0x00000, 0x20000, CRC(c33e543c) SHA1(b85cba30cc651f820aeedd41e04584df92078ed9) )
	ROM_LOAD( "b90_10.9",   0x20000, 0x20000, CRC(35fd1092) SHA1(5e136a35eea45034ccd4aea52cc0ffeec944e27e) )
	ROM_LOAD( "b90_09.8",   0x40000, 0x20000, CRC(cf01009e) SHA1(e260c479fa97f23a65c220e5071aaf2dc2baf46d) )
	ROM_LOAD( "b90_08.7",   0x60000, 0x20000, CRC(3404a5e3) SHA1(f717b9e31c2a093dbb060b8ea54a8c3f52688d7a) )

	ROM_REGION( 0x40, "proms", 0 )		/* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "13.3w",     0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )	/* N82S123AN bprom - sprite attribute (flip/position) ?? */
	ROM_LOAD( "12.6b",     0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )	/* N82S123AN bprom -  ??? */
ROM_END

ROM_START( zerowing )
	ROM_REGION( 0x080000, "maincpu", 0 )	/* Main 68K code */
	ROM_LOAD16_BYTE( "o15-11.rom",  0x000000, 0x08000, CRC(6ff2b9a0) SHA1(c9f2a631f185689dfc42a451d85fac23c2f4b64b) )
	ROM_LOAD16_BYTE( "o15-12.rom",  0x000001, 0x08000, CRC(9773e60b) SHA1(b733e9d38a233d010cc5ea41e7e61695082c3a22) )
	ROM_LOAD16_BYTE( "o15-09.rom",  0x040000, 0x20000, CRC(13764e95) SHA1(61da49b73ba81edd951e96e9ce6673c1c3bd65f2) )
	ROM_LOAD16_BYTE( "o15-10.rom",  0x040001, 0x20000, CRC(351ba71a) SHA1(937331549140506711b08252497cc0f2efa58268) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound Z80 code */
	ROM_LOAD( "o15-13.rom",  0x0000, 0x8000, CRC(e7b72383) SHA1(ea1f6f33a86d14d58bd396fd46081462f00177d5) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "o15-05.rom",  0x00000, 0x20000, CRC(4e5dd246) SHA1(5366b4a6f3c900a4f57a6583b7399163a06f42d7) )
	ROM_LOAD( "o15-06.rom",  0x20000, 0x20000, CRC(c8c6d428) SHA1(76ee5bcb8f10fe201fc5c32697beee3de9d8b751) )
	ROM_LOAD( "o15-07.rom",  0x40000, 0x20000, CRC(efc40e99) SHA1(a04fad4197a7fb4787cd9bebf43e1d9b02b2f61b) )
	ROM_LOAD( "o15-08.rom",  0x60000, 0x20000, CRC(1b019eab) SHA1(c9569ca85696825142acc5cde9ac829e82b1ca1b) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "o15-03.rom",  0x00000, 0x20000, CRC(7f245fd3) SHA1(efbcb3663d4accc4f8128a8fee5475bc109bc17a) )
	ROM_LOAD( "o15-04.rom",  0x20000, 0x20000, CRC(0b1a1289) SHA1(ce6c06342392d11952873e3b1d6aea8dc02a551c) )
	ROM_LOAD( "o15-01.rom",  0x40000, 0x20000, CRC(70570e43) SHA1(acc9baec71b0930cb2f193677e0663efa5d5551d) )
	ROM_LOAD( "o15-02.rom",  0x60000, 0x20000, CRC(724b487f) SHA1(06af31520866eea69aebbd5d428f80e882289a15) )

	ROM_REGION( 0x40, "proms", 0 )		/* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "tp015_14.bpr",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )	/* sprite attribute (flip/position) ?? */
	ROM_LOAD( "tp015_15.bpr",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )	/* ??? */
ROM_END

ROM_START( zerowing2 ) /* 2 player simultaneous version */
	ROM_REGION( 0x080000, "maincpu", 0 )	/* Main 68K code */
	ROM_LOAD16_BYTE( "o15-11iiw.bin",  0x000000, 0x08000, CRC(38b0bb5b) SHA1(e5a4c0b6c279a55701c82bf9e285a806054f8d23) )
	ROM_LOAD16_BYTE( "o15-12iiw.bin",  0x000001, 0x08000, CRC(74c91e6f) SHA1(8cf5d10a5f4efda0903a4c5d56599861ccc8f1c1) )
	ROM_LOAD16_BYTE( "o15-09.rom",     0x040000, 0x20000, CRC(13764e95) SHA1(61da49b73ba81edd951e96e9ce6673c1c3bd65f2) )
	ROM_LOAD16_BYTE( "o15-10.rom",     0x040001, 0x20000, CRC(351ba71a) SHA1(937331549140506711b08252497cc0f2efa58268) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound Z80 code */
	ROM_LOAD( "o15-13.rom",  0x0000, 0x8000, CRC(e7b72383) SHA1(ea1f6f33a86d14d58bd396fd46081462f00177d5) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "o15-05.rom",  0x00000, 0x20000, CRC(4e5dd246) SHA1(5366b4a6f3c900a4f57a6583b7399163a06f42d7) )
	ROM_LOAD( "o15-06.rom",  0x20000, 0x20000, CRC(c8c6d428) SHA1(76ee5bcb8f10fe201fc5c32697beee3de9d8b751) )
	ROM_LOAD( "o15-07.rom",  0x40000, 0x20000, CRC(efc40e99) SHA1(a04fad4197a7fb4787cd9bebf43e1d9b02b2f61b) )
	ROM_LOAD( "o15-08.rom",  0x60000, 0x20000, CRC(1b019eab) SHA1(c9569ca85696825142acc5cde9ac829e82b1ca1b) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "o15-03.rom",  0x00000, 0x20000, CRC(7f245fd3) SHA1(efbcb3663d4accc4f8128a8fee5475bc109bc17a) )
	ROM_LOAD( "o15-04.rom",  0x20000, 0x20000, CRC(0b1a1289) SHA1(ce6c06342392d11952873e3b1d6aea8dc02a551c) )
	ROM_LOAD( "o15-01.rom",  0x40000, 0x20000, CRC(70570e43) SHA1(acc9baec71b0930cb2f193677e0663efa5d5551d) )
	ROM_LOAD( "o15-02.rom",  0x60000, 0x20000, CRC(724b487f) SHA1(06af31520866eea69aebbd5d428f80e882289a15) )

	ROM_REGION( 0x40, "proms", 0 )		/* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "tp015_14.bpr",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )	/* sprite attribute (flip/position) ?? */
	ROM_LOAD( "tp015_15.bpr",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )	/* ??? */
ROM_END

ROM_START( demonwld )
	ROM_REGION( 0x040000, "maincpu", 0 )	/* Main 68K code */
	ROM_LOAD16_BYTE( "o16-10.v2", 0x000000, 0x20000, CRC(ca8194f3) SHA1(176da6739b35ba38b40150fc62380108bcae5a24) )
	ROM_LOAD16_BYTE( "o16-09.v2", 0x000001, 0x20000, CRC(7baea7ba) SHA1(ae2b40f9efb4440ff7edbcc4f80641655f7c4671) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound Z80 code */
	ROM_LOAD( "rom11.v2",  0x0000, 0x8000, CRC(dbe08c85) SHA1(536a242bfe916d15744b079261507af6f12b5b50) )

	ROM_REGION( 0x2000, "dsp", 0 )	/* Co-Processor TMS320C10 MCU code */
	ROM_LOAD16_BYTE( "dsp_21.bin",  0x0000, 0x0800, CRC(2d135376) SHA1(67a2cc774d272ee1cd6e6bc1c5fc33fc6968837e) )
	ROM_LOAD16_BYTE( "dsp_22.bin",  0x0001, 0x0800, CRC(79389a71) SHA1(14ec4c1c9b06702319e89a7a250d0038393437f4) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "rom05",  0x00000, 0x20000, CRC(6506c982) SHA1(6d4c1ef91e5617724789ff196abb7abf23e4a7fb) )
	ROM_LOAD( "rom07",  0x20000, 0x20000, CRC(a3a0d993) SHA1(50311b9447eb04271b17b212ca31d083ab5b2414) )
	ROM_LOAD( "rom06",  0x40000, 0x20000, CRC(4fc5e5f3) SHA1(725d4b009d575ff8ffbe1c00df352ccf235465d7) )
	ROM_LOAD( "rom08",  0x60000, 0x20000, CRC(eb53ab09) SHA1(d98195cc1b65b76335b5b24adb31deae1b313f3a) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "rom01",  0x00000, 0x20000, CRC(1b3724e9) SHA1(3dbb0450ab1e40e6df2b7c7356352419cd3f113d) )
	ROM_LOAD( "rom02",  0x20000, 0x20000, CRC(7b20a44d) SHA1(4dc1a2fa2058077b112c73492808ee9381060ec7) )
	ROM_LOAD( "rom03",  0x40000, 0x20000, CRC(2cacdcd0) SHA1(92216d1c6859e05d39363c30e0beb45bc0ae4e1c) )
	ROM_LOAD( "rom04",  0x60000, 0x20000, CRC(76fd3201) SHA1(7a12737bf90bd9760074132edeb22f3fd3e16b4f) )

	ROM_REGION( 0x40, "proms", 0 )		/* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "prom12.bpr",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )	/* sprite attribute (flip/position) ?? */
	ROM_LOAD( "prom13.bpr",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )	/* ??? */
ROM_END

ROM_START( demonwld1 )
	ROM_REGION( 0x040000, "maincpu", 0 )	/* Main 68K code */
	ROM_LOAD16_BYTE( "o16-10.rom", 0x000000, 0x20000, CRC(036ee46c) SHA1(60868e5e08e0c9a538ae786de0de6b2531b30b11) )
	ROM_LOAD16_BYTE( "o16-09.rom", 0x000001, 0x20000, CRC(bed746e3) SHA1(056668edb7df99bbd240e387af17cf252d1448f3) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound Z80 code */
	ROM_LOAD( "rom11",  0x0000, 0x8000, CRC(397eca1b) SHA1(84073ff6d1bc46ec6162d66ec5de305700938380) )

	ROM_REGION( 0x2000, "dsp", 0 )	/* Co-Processor TMS320C10 MCU code */
	ROM_LOAD16_BYTE( "dsp_21.bin",  0x0000, 0x0800, CRC(2d135376) SHA1(67a2cc774d272ee1cd6e6bc1c5fc33fc6968837e) )
	ROM_LOAD16_BYTE( "dsp_22.bin",  0x0001, 0x0800, CRC(79389a71) SHA1(14ec4c1c9b06702319e89a7a250d0038393437f4) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "rom05",  0x00000, 0x20000, CRC(6506c982) SHA1(6d4c1ef91e5617724789ff196abb7abf23e4a7fb) )
	ROM_LOAD( "rom07",  0x20000, 0x20000, CRC(a3a0d993) SHA1(50311b9447eb04271b17b212ca31d083ab5b2414) )
	ROM_LOAD( "rom06",  0x40000, 0x20000, CRC(4fc5e5f3) SHA1(725d4b009d575ff8ffbe1c00df352ccf235465d7) )
	ROM_LOAD( "rom08",  0x60000, 0x20000, CRC(eb53ab09) SHA1(d98195cc1b65b76335b5b24adb31deae1b313f3a) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "rom01",  0x00000, 0x20000, CRC(1b3724e9) SHA1(3dbb0450ab1e40e6df2b7c7356352419cd3f113d) )
	ROM_LOAD( "rom02",  0x20000, 0x20000, CRC(7b20a44d) SHA1(4dc1a2fa2058077b112c73492808ee9381060ec7) )
	ROM_LOAD( "rom03",  0x40000, 0x20000, CRC(2cacdcd0) SHA1(92216d1c6859e05d39363c30e0beb45bc0ae4e1c) )
	ROM_LOAD( "rom04",  0x60000, 0x20000, CRC(76fd3201) SHA1(7a12737bf90bd9760074132edeb22f3fd3e16b4f) )

	ROM_REGION( 0x40, "proms", 0 )		/* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "prom12.bpr",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )	/* sprite attribute (flip/position) ?? */
	ROM_LOAD( "prom13.bpr",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )	/* ??? */
ROM_END

ROM_START( demonwld2 )
	ROM_REGION( 0x040000, "maincpu", 0 )	/* Main 68K code */
	ROM_LOAD16_BYTE( "o16-10-2.bin", 0x000000, 0x20000, CRC(84ee5218) SHA1(dc2b017ee630330163be320008d8a0d761cb0cfb) ) // aka o16_10ii
	ROM_LOAD16_BYTE( "o16-09-2.bin", 0x000001, 0x20000, CRC(cf474cb2) SHA1(5c049082b8d7118e0d2e50c6ae07f9d3d0110498) ) // aka o16_09ii

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound Z80 code */
	ROM_LOAD( "rom11",  0x0000, 0x8000, CRC(397eca1b) SHA1(84073ff6d1bc46ec6162d66ec5de305700938380) )

	ROM_REGION( 0x2000, "dsp", 0 )	/* Co-Processor TMS320C10 MCU code */
	ROM_LOAD16_BYTE( "dsp_21.bin",  0x0000, 0x0800, CRC(2d135376) SHA1(67a2cc774d272ee1cd6e6bc1c5fc33fc6968837e) )
	ROM_LOAD16_BYTE( "dsp_22.bin",  0x0001, 0x0800, CRC(79389a71) SHA1(14ec4c1c9b06702319e89a7a250d0038393437f4) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "rom05",  0x00000, 0x20000, CRC(6506c982) SHA1(6d4c1ef91e5617724789ff196abb7abf23e4a7fb) )
	ROM_LOAD( "rom07",  0x20000, 0x20000, CRC(a3a0d993) SHA1(50311b9447eb04271b17b212ca31d083ab5b2414) )
	ROM_LOAD( "rom06",  0x40000, 0x20000, CRC(4fc5e5f3) SHA1(725d4b009d575ff8ffbe1c00df352ccf235465d7) )
	ROM_LOAD( "rom08",  0x60000, 0x20000, CRC(eb53ab09) SHA1(d98195cc1b65b76335b5b24adb31deae1b313f3a) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "rom01",  0x00000, 0x20000, CRC(1b3724e9) SHA1(3dbb0450ab1e40e6df2b7c7356352419cd3f113d) )
	ROM_LOAD( "rom02",  0x20000, 0x20000, CRC(7b20a44d) SHA1(4dc1a2fa2058077b112c73492808ee9381060ec7) )
	ROM_LOAD( "rom03",  0x40000, 0x20000, CRC(2cacdcd0) SHA1(92216d1c6859e05d39363c30e0beb45bc0ae4e1c) )
	ROM_LOAD( "rom04",  0x60000, 0x20000, CRC(76fd3201) SHA1(7a12737bf90bd9760074132edeb22f3fd3e16b4f) )

	ROM_REGION( 0x40, "proms", 0 )		/* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "prom12.bpr",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )	/* sprite attribute (flip/position) ?? */
	ROM_LOAD( "prom13.bpr",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )	/* ??? */
ROM_END

ROM_START( demonwld3 )
	ROM_REGION( 0x040000, "maincpu", 0 )	/* Main 68K code */
	ROM_LOAD16_BYTE( "o16-10.bin", 0x000000, 0x20000, CRC(6f7468e0) SHA1(87ef7733fd0d00d0d375dbf30332cf0614480dc2) )
	ROM_LOAD16_BYTE( "o16-09.bin", 0x000001, 0x20000, CRC(a572f5f7) SHA1(3d6a443cecd46734c7e1b761130909482c7a9914) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound Z80 code */
	ROM_LOAD( "rom11",  0x0000, 0x8000, CRC(397eca1b) SHA1(84073ff6d1bc46ec6162d66ec5de305700938380) )

	ROM_REGION( 0x2000, "dsp", 0 )	/* Co-Processor TMS320C10 MCU code */
	ROM_LOAD16_BYTE( "dsp_21.bin",  0x0000, 0x0800, CRC(2d135376) SHA1(67a2cc774d272ee1cd6e6bc1c5fc33fc6968837e) )
	ROM_LOAD16_BYTE( "dsp_22.bin",  0x0001, 0x0800, CRC(79389a71) SHA1(14ec4c1c9b06702319e89a7a250d0038393437f4) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "rom05",  0x00000, 0x20000, CRC(6506c982) SHA1(6d4c1ef91e5617724789ff196abb7abf23e4a7fb) )
	ROM_LOAD( "rom07",  0x20000, 0x20000, CRC(a3a0d993) SHA1(50311b9447eb04271b17b212ca31d083ab5b2414) )
	ROM_LOAD( "rom06",  0x40000, 0x20000, CRC(4fc5e5f3) SHA1(725d4b009d575ff8ffbe1c00df352ccf235465d7) )
	ROM_LOAD( "rom08",  0x60000, 0x20000, CRC(eb53ab09) SHA1(d98195cc1b65b76335b5b24adb31deae1b313f3a) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "rom01",  0x00000, 0x20000, CRC(1b3724e9) SHA1(3dbb0450ab1e40e6df2b7c7356352419cd3f113d) )
	ROM_LOAD( "rom02",  0x20000, 0x20000, CRC(7b20a44d) SHA1(4dc1a2fa2058077b112c73492808ee9381060ec7) )
	ROM_LOAD( "rom03",  0x40000, 0x20000, CRC(2cacdcd0) SHA1(92216d1c6859e05d39363c30e0beb45bc0ae4e1c) )
	ROM_LOAD( "rom04",  0x60000, 0x20000, CRC(76fd3201) SHA1(7a12737bf90bd9760074132edeb22f3fd3e16b4f) )

	ROM_REGION( 0x40, "proms", 0 )		/* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "prom12.bpr",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )	/* sprite attribute (flip/position) ?? */
	ROM_LOAD( "prom13.bpr",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )	/* ??? */
ROM_END

ROM_START( samesame )
	ROM_REGION( 0x080000, "maincpu", 0 )	/* Main 68K code */
	ROM_LOAD16_BYTE( "o17_09.bin",  0x000000, 0x08000, CRC(3f69e437) SHA1(f2a40fd42cb5ecb2e514b72e7550aa479a9f9ad6) )
	ROM_LOAD16_BYTE( "o17_10.bin",  0x000001, 0x08000, CRC(4e723e0a) SHA1(e06394d50addeda1045c02c646964afbc6005a82) )
	ROM_LOAD16_BYTE( "o17_11.bin",  0x040000, 0x20000, CRC(be07d101) SHA1(1eda14ba24532b565d6ad57490b73ff312f98b53) )
	ROM_LOAD16_BYTE( "o17_12.bin",  0x040001, 0x20000, CRC(ef698811) SHA1(4c729704eba0bf469599c79009327e4fa5dc540b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound HD647180 code */
	/* sound CPU is a HD647180 (Z180) with internal ROM - not yet supported */
	ROM_LOAD( "hd647180.017",  0x00000, 0x08000, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "o17_05.12j",  0x00000, 0x20000, CRC(565315f8) SHA1(6b1c5ef52359483228b329c89c2e1174e3fbf017) )
	ROM_LOAD( "o17_06.13j",  0x20000, 0x20000, CRC(95262d4c) SHA1(16f3aabecb1c87ce7eadf4f0ff61b29a4c017614) )
	ROM_LOAD( "o17_07.12l",  0x40000, 0x20000, CRC(4c4b735c) SHA1(812c3bf46bd7764b2bb812bd2b9eb0331ed257ae) )
	ROM_LOAD( "o17_08.13l",  0x60000, 0x20000, CRC(95c6586c) SHA1(ff87901f79d80f73ad09664b0c0d892898570616) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "o17_01.1d",  0x00000, 0x20000, CRC(ea12e491) SHA1(02190722b7c5383471e0af9596be7039a5367240) )
	ROM_LOAD( "o17_02.3d",  0x20000, 0x20000, CRC(32a13a9f) SHA1(1446acdfd21cd41f3d97aaf30f498c0c5d890605) )
	ROM_LOAD( "o17_03.5d",  0x40000, 0x20000, CRC(68723dc9) SHA1(4f1b7aa2469c955e03737b611a7d2524f1e4f61e) )
	ROM_LOAD( "o17_04.7d",  0x60000, 0x20000, CRC(fe0ecb13) SHA1(634a49262b9c092c25f11b14c6757fe94ea9eddc) )

	ROM_REGION( 0x40, "proms", 0 )		/* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "prom14.25b",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )	/* sprite attribute (flip/position) ?? */
	ROM_LOAD( "prom15.20c",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )	/* ??? */
ROM_END

ROM_START( samesame2 )
	ROM_REGION( 0x080000, "maincpu", 0 )	/* Main 68K code */
	ROM_LOAD16_BYTE( "o17_09x.bin", 0x000000, 0x08000, CRC(3472e03e) SHA1(a0f12622a1963bfac2d5f357afbfb5d7db2cd8df) )
	ROM_LOAD16_BYTE( "o17_10x.bin", 0x000001, 0x08000, CRC(a3ac49b5) SHA1(c5adf026b9129b64acee5a079e102377a8488220) )
	ROM_LOAD16_BYTE( "o17_11ii.7j", 0x040000, 0x20000, CRC(6beac378) SHA1(041ba98a89a4bac32575858db8a061bdf7804594) )
	ROM_LOAD16_BYTE( "o17_12ii.7l", 0x040001, 0x20000, CRC(6adb6eb5) SHA1(9b6e63aa50d271c2bb0b4cf822fc6f3684f10230) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound HD647180 code */
	/* sound CPU is a HD647180 (Z180) with internal ROM - not yet supported */
	ROM_LOAD( "hd647180.017",  0x00000, 0x08000, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "o17_05.12j",  0x00000, 0x20000, CRC(565315f8) SHA1(6b1c5ef52359483228b329c89c2e1174e3fbf017) )
	ROM_LOAD( "o17_06.13j",  0x20000, 0x20000, CRC(95262d4c) SHA1(16f3aabecb1c87ce7eadf4f0ff61b29a4c017614) )
	ROM_LOAD( "o17_07.12l",  0x40000, 0x20000, CRC(4c4b735c) SHA1(812c3bf46bd7764b2bb812bd2b9eb0331ed257ae) )
	ROM_LOAD( "o17_08.13l",  0x60000, 0x20000, CRC(95c6586c) SHA1(ff87901f79d80f73ad09664b0c0d892898570616) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "o17_01.1d",  0x00000, 0x20000, CRC(ea12e491) SHA1(02190722b7c5383471e0af9596be7039a5367240) )
	ROM_LOAD( "o17_02.3d",  0x20000, 0x20000, CRC(32a13a9f) SHA1(1446acdfd21cd41f3d97aaf30f498c0c5d890605) )
	ROM_LOAD( "o17_03.5d",  0x40000, 0x20000, CRC(68723dc9) SHA1(4f1b7aa2469c955e03737b611a7d2524f1e4f61e) )
	ROM_LOAD( "o17_04.7d",  0x60000, 0x20000, CRC(fe0ecb13) SHA1(634a49262b9c092c25f11b14c6757fe94ea9eddc) )

	ROM_REGION( 0x40, "proms", 0 )		/* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "prom14.25b",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )	/* sprite attribute (flip/position) ?? */
	ROM_LOAD( "prom15.20c",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )	/* ??? */
ROM_END

ROM_START( fireshrk )
	ROM_REGION( 0x080000, "maincpu", 0 )	/* Main 68K code */
	ROM_LOAD16_BYTE( "09.bin",      0x000000, 0x08000, CRC(f0c70e6f) SHA1(037690448786d61aa116b24b638430c577ea78e2) )
	ROM_LOAD16_BYTE( "10.bin",      0x000001, 0x08000, CRC(9d253d77) SHA1(0414d1f475abb9ccfd7daa11c2f400a14f25db09) )
	ROM_LOAD16_BYTE( "o17_11ii.7j", 0x040000, 0x20000, CRC(6beac378) SHA1(041ba98a89a4bac32575858db8a061bdf7804594) )
	ROM_LOAD16_BYTE( "o17_12ii.7l", 0x040001, 0x20000, CRC(6adb6eb5) SHA1(9b6e63aa50d271c2bb0b4cf822fc6f3684f10230) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound HD647180 code */
	/* sound CPU is a HD647180 (Z180) with internal ROM - not yet supported */
	ROM_LOAD( "hd647180.017",  0x00000, 0x08000, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "o17_05.12j",  0x00000, 0x20000, CRC(565315f8) SHA1(6b1c5ef52359483228b329c89c2e1174e3fbf017) )
	ROM_LOAD( "o17_06.13j",  0x20000, 0x20000, CRC(95262d4c) SHA1(16f3aabecb1c87ce7eadf4f0ff61b29a4c017614) )
	ROM_LOAD( "o17_07.12l",  0x40000, 0x20000, CRC(4c4b735c) SHA1(812c3bf46bd7764b2bb812bd2b9eb0331ed257ae) )
	ROM_LOAD( "o17_08.13l",  0x60000, 0x20000, CRC(95c6586c) SHA1(ff87901f79d80f73ad09664b0c0d892898570616) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "o17_01.1d",  0x00000, 0x20000, CRC(ea12e491) SHA1(02190722b7c5383471e0af9596be7039a5367240) )
	ROM_LOAD( "o17_02.3d",  0x20000, 0x20000, CRC(32a13a9f) SHA1(1446acdfd21cd41f3d97aaf30f498c0c5d890605) )
	ROM_LOAD( "o17_03.5d",  0x40000, 0x20000, CRC(68723dc9) SHA1(4f1b7aa2469c955e03737b611a7d2524f1e4f61e) )
	ROM_LOAD( "o17_04.7d",  0x60000, 0x20000, CRC(fe0ecb13) SHA1(634a49262b9c092c25f11b14c6757fe94ea9eddc) )

	ROM_REGION( 0x40, "proms", 0 )		/* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "prom14.25b",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )	/* sprite attribute (flip/position) ?? */
	ROM_LOAD( "prom15.20c",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )	/* ??? */
ROM_END

ROM_START( fireshrkd )
	ROM_REGION( 0x080000, "maincpu", 0 )	/* Main 68K code */
	ROM_LOAD16_BYTE( "o17_09dyn.8j",0x000000, 0x10000, CRC(e25eee27) SHA1(1ff3f838123180a0b6672c9beee6c0f0092a0f94) )
	ROM_LOAD16_BYTE( "o17_10dyn.8l",0x000001, 0x10000, CRC(c4c58cf6) SHA1(5867ecf66cd6c16cfcc54a581d3f4a8b666fd839) )
	ROM_LOAD16_BYTE( "o17_11ii.7j", 0x040000, 0x20000, CRC(6beac378) SHA1(041ba98a89a4bac32575858db8a061bdf7804594) )
	ROM_LOAD16_BYTE( "o17_12ii.7l", 0x040001, 0x20000, CRC(6adb6eb5) SHA1(9b6e63aa50d271c2bb0b4cf822fc6f3684f10230) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound HD647180 code */
	/* sound CPU is a HD647180 (Z180) with internal ROM - not yet supported */
	ROM_LOAD( "hd647180.017",  0x00000, 0x08000, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "o17_05.12j",  0x00000, 0x20000, CRC(565315f8) SHA1(6b1c5ef52359483228b329c89c2e1174e3fbf017) )
	ROM_LOAD( "o17_06.13j",  0x20000, 0x20000, CRC(95262d4c) SHA1(16f3aabecb1c87ce7eadf4f0ff61b29a4c017614) )
	ROM_LOAD( "o17_07.12l",  0x40000, 0x20000, CRC(4c4b735c) SHA1(812c3bf46bd7764b2bb812bd2b9eb0331ed257ae) )
	ROM_LOAD( "o17_08.13l",  0x60000, 0x20000, CRC(95c6586c) SHA1(ff87901f79d80f73ad09664b0c0d892898570616) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "o17_01.1d",  0x00000, 0x20000, CRC(ea12e491) SHA1(02190722b7c5383471e0af9596be7039a5367240) )
	ROM_LOAD( "o17_02.3d",  0x20000, 0x20000, CRC(32a13a9f) SHA1(1446acdfd21cd41f3d97aaf30f498c0c5d890605) )
	ROM_LOAD( "o17_03.5d",  0x40000, 0x20000, CRC(68723dc9) SHA1(4f1b7aa2469c955e03737b611a7d2524f1e4f61e) )
	ROM_LOAD( "o17_04.7d",  0x60000, 0x20000, CRC(fe0ecb13) SHA1(634a49262b9c092c25f11b14c6757fe94ea9eddc) )

	ROM_REGION( 0x40, "proms", 0 )		/* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "prom14.25b",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )	/* sprite attribute (flip/position) ?? */
	ROM_LOAD( "prom15.20c",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )	/* ??? */
ROM_END

ROM_START( fireshrkdh )
	ROM_REGION( 0x080000, "maincpu", 0 )	/* Main 68K code */
	ROM_LOAD16_BYTE( "o17_09dyh.8j",0x000000, 0x10000, CRC(7b4c14dd) SHA1(d40dcf223f16c0f507aeb282d1524dbf1349c536) )
	ROM_LOAD16_BYTE( "o17_10dyh.8l",0x000001, 0x10000, CRC(a3f159f9) SHA1(afc9630ca38da730f7cf4954d1333954e8d75787) )
	ROM_LOAD16_BYTE( "o17_11x.bin", 0x040000, 0x20000, CRC(6beac378) SHA1(041ba98a89a4bac32575858db8a061bdf7804594) )
	ROM_LOAD16_BYTE( "o17_12x.bin", 0x040001, 0x20000, CRC(6adb6eb5) SHA1(9b6e63aa50d271c2bb0b4cf822fc6f3684f10230) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound HD647180 code */
	/* sound CPU is a HD647180 (Z180) with internal ROM - not yet supported */
	ROM_LOAD( "hd647180.017",  0x00000, 0x08000, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "o17_05.12j",  0x00000, 0x20000, CRC(565315f8) SHA1(6b1c5ef52359483228b329c89c2e1174e3fbf017) )
	ROM_LOAD( "o17_06.13j",  0x20000, 0x20000, CRC(95262d4c) SHA1(16f3aabecb1c87ce7eadf4f0ff61b29a4c017614) )
	ROM_LOAD( "o17_07.12l",  0x40000, 0x20000, CRC(4c4b735c) SHA1(812c3bf46bd7764b2bb812bd2b9eb0331ed257ae) )
	ROM_LOAD( "o17_08.13l",  0x60000, 0x20000, CRC(95c6586c) SHA1(ff87901f79d80f73ad09664b0c0d892898570616) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "o17_01.1d",  0x00000, 0x20000, CRC(ea12e491) SHA1(02190722b7c5383471e0af9596be7039a5367240) )
	ROM_LOAD( "o17_02.3d",  0x20000, 0x20000, CRC(32a13a9f) SHA1(1446acdfd21cd41f3d97aaf30f498c0c5d890605) )
	ROM_LOAD( "o17_03.5d",  0x40000, 0x20000, CRC(68723dc9) SHA1(4f1b7aa2469c955e03737b611a7d2524f1e4f61e) )
	ROM_LOAD( "o17_04.7d",  0x60000, 0x20000, CRC(fe0ecb13) SHA1(634a49262b9c092c25f11b14c6757fe94ea9eddc) )

	ROM_REGION( 0x40, "proms", 0 )		/* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "prom14.25b",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )	/* sprite attribute (flip/position) ?? */
	ROM_LOAD( "prom15.20c",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )	/* ??? */
ROM_END


/*
Out Zone - Seems to be a later version, Differences:

. This version is a lot harder.
. Attract mode differs, player 1 dies early in "Demonstration 2"
. Special pick up is Super Ball instead of Shield in attract mode.
. Test mode can be entered and exited without any crash.
*/

ROM_START( outzone )
	ROM_REGION( 0x040000, "maincpu", 0 )	/* Main 68K code */
	ROM_LOAD16_BYTE( "tp018_7.bin",  0x000000, 0x20000, CRC(0c2ac02d) SHA1(78fda906ef7e0bb8e4ad44f34a8ac934b75d4bd8) )
	ROM_LOAD16_BYTE( "tp018_8.bin",  0x000001, 0x20000, CRC(ca7e48aa) SHA1(c5073e6c124d74f16d01e67949965fdca929a886) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound Z80 code */
	ROM_LOAD( "rom9.bin",  0x0000, 0x8000, CRC(73d8e235) SHA1(f37ad497259a467cdf2ec8b3e6e7d3e873087e6c) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "rom5.bin",  0x00000, 0x80000, CRC(c64ec7b6) SHA1(e73b51c3713c2ea7a572a02531c15d1261ddeaa0) )
	ROM_LOAD( "rom6.bin",  0x80000, 0x80000, CRC(64b6c5ac) SHA1(07fa20115f603445c0d51af3465c0471c09d76b1) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "rom2.bin",  0x00000, 0x20000, CRC(6bb72d16) SHA1(a127b10d9c255542bd09fcb5df057c12fd28c0d1) )
	ROM_LOAD( "rom1.bin",  0x20000, 0x20000, CRC(0934782d) SHA1(e4a775ead23227d7d6e76aea23aa3103b511d031) )
	ROM_LOAD( "rom3.bin",  0x40000, 0x20000, CRC(ec903c07) SHA1(75906f31200877fc8f6e78c2606ad5be49778165) )
	ROM_LOAD( "rom4.bin",  0x60000, 0x20000, CRC(50cbf1a8) SHA1(cfab1504746654b4a61912155e9aeca746c65321) )

	ROM_REGION( 0x40, "proms", 0 )		/* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "tp018_10.bpr",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )	/* sprite attribute (flip/position) ?? */
	ROM_LOAD( "tp018_11.bpr",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )	/* ??? */
ROM_END

ROM_START( outzonea )
	ROM_REGION( 0x040000, "maincpu", 0 )	/* Main 68K code */
	ROM_LOAD16_BYTE( "18.bin",  0x000000, 0x20000, CRC(31a171bb) SHA1(4ee707e758ab21d2809b65daf0081f86bd9328d9) )
	ROM_LOAD16_BYTE( "19.bin",  0x000001, 0x20000, CRC(804ecfd1) SHA1(7dead8064445c6d44ebd0889583deb5e17b1954a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound Z80 code */
	ROM_LOAD( "rom9.bin",  0x0000, 0x8000, CRC(73d8e235) SHA1(f37ad497259a467cdf2ec8b3e6e7d3e873087e6c) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "rom5.bin",  0x00000, 0x80000, CRC(c64ec7b6) SHA1(e73b51c3713c2ea7a572a02531c15d1261ddeaa0) )
	ROM_LOAD( "rom6.bin",  0x80000, 0x80000, CRC(64b6c5ac) SHA1(07fa20115f603445c0d51af3465c0471c09d76b1) )
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
	ROM_LOAD( "rom2.bin",  0x00000, 0x20000, CRC(6bb72d16) SHA1(a127b10d9c255542bd09fcb5df057c12fd28c0d1) )
	ROM_LOAD( "rom1.bin",  0x20000, 0x20000, CRC(0934782d) SHA1(e4a775ead23227d7d6e76aea23aa3103b511d031) )
	ROM_LOAD( "rom3.bin",  0x40000, 0x20000, CRC(ec903c07) SHA1(75906f31200877fc8f6e78c2606ad5be49778165) )
	ROM_LOAD( "rom4.bin",  0x60000, 0x20000, CRC(50cbf1a8) SHA1(cfab1504746654b4a61912155e9aeca746c65321) )

	ROM_REGION( 0x40, "proms", 0 )		/* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "tp018_10.bpr",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )	/* sprite attribute (flip/position) ?? */
	ROM_LOAD( "tp018_11.bpr",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )	/* ??? */
ROM_END

/* is this a prototype? */
ROM_START( outzoneb )					/* From board serial number 2122 */
	ROM_REGION( 0x040000, "maincpu", 0 )	/* Main 68K code */
	ROM_LOAD16_BYTE( "rom7.bin",  0x000000, 0x20000, CRC(936e25d8) SHA1(ffb7990ea1539d868a9ad2fb711b0febd90f098d) )
	ROM_LOAD16_BYTE( "rom8.bin",  0x000001, 0x20000, CRC(d19b3ecf) SHA1(b406999b9f1e2104d958b42cc745bf79dbfe50b3) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound Z80 code */
	ROM_LOAD( "rom9.bin",  0x0000, 0x8000, CRC(73d8e235) SHA1(f37ad497259a467cdf2ec8b3e6e7d3e873087e6c) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "rom5.bin",  0x00000, 0x80000, CRC(c64ec7b6) SHA1(e73b51c3713c2ea7a572a02531c15d1261ddeaa0) )
	ROM_LOAD( "rom6.bin",  0x80000, 0x80000, CRC(64b6c5ac) SHA1(07fa20115f603445c0d51af3465c0471c09d76b1) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "rom2.bin",  0x00000, 0x20000, CRC(6bb72d16) SHA1(a127b10d9c255542bd09fcb5df057c12fd28c0d1) )
	ROM_LOAD( "rom1.bin",  0x20000, 0x20000, CRC(0934782d) SHA1(e4a775ead23227d7d6e76aea23aa3103b511d031) )
	ROM_LOAD( "rom3.bin",  0x40000, 0x20000, CRC(ec903c07) SHA1(75906f31200877fc8f6e78c2606ad5be49778165) )
	ROM_LOAD( "rom4.bin",  0x60000, 0x20000, CRC(50cbf1a8) SHA1(cfab1504746654b4a61912155e9aeca746c65321) )

	ROM_REGION( 0x40, "proms", 0 )		/* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "tp018_10.bpr",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )	/* sprite attribute (flip/position) ?? */
	ROM_LOAD( "tp018_11.bpr",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )	/* ??? */
ROM_END

/* from a bootleg board, but probably an alt original set with different licenses */
ROM_START( outzonec )
	ROM_REGION( 0x040000, "maincpu", 0 )	/* Main 68K code */
	ROM_LOAD16_BYTE( "prg2.bin",  0x000001, 0x20000, CRC(9704db16) SHA1(12b43a6961a7f63f29563eb77aaacb70d3c368dd) )
	ROM_LOAD16_BYTE( "prg1.bin",  0x000000, 0x20000, CRC(127a38d7) SHA1(d7f1ed91ff7d4de9e8215aa3b5cb65693145e433) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound Z80 code */
	ROM_LOAD( "rom9.bin",  0x0000, 0x8000, CRC(73d8e235) SHA1(f37ad497259a467cdf2ec8b3e6e7d3e873087e6c) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "rom5.bin",  0x00000, 0x80000, CRC(c64ec7b6) SHA1(e73b51c3713c2ea7a572a02531c15d1261ddeaa0) )
	ROM_LOAD( "rom6.bin",  0x80000, 0x80000, CRC(64b6c5ac) SHA1(07fa20115f603445c0d51af3465c0471c09d76b1) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "rom2.bin",  0x00000, 0x20000, CRC(6bb72d16) SHA1(a127b10d9c255542bd09fcb5df057c12fd28c0d1) )
	ROM_LOAD( "rom1.bin",  0x20000, 0x20000, CRC(0934782d) SHA1(e4a775ead23227d7d6e76aea23aa3103b511d031) )
	ROM_LOAD( "rom3.bin",  0x40000, 0x20000, CRC(ec903c07) SHA1(75906f31200877fc8f6e78c2606ad5be49778165) )
	ROM_LOAD( "rom4.bin",  0x60000, 0x20000, CRC(50cbf1a8) SHA1(cfab1504746654b4a61912155e9aeca746c65321) )

	ROM_REGION( 0x40, "proms", 0 )		/* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "tp018_10.bpr",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )	/* sprite attribute (flip/position) ?? */
	ROM_LOAD( "tp018_11.bpr",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )	/* ??? */
ROM_END

ROM_START( outzoned )
	ROM_REGION( 0x040000, "maincpu", 0 )	/* Main 68K code */
	ROM_LOAD16_BYTE( "tp07.bin",  0x000000, 0x20000, CRC(a85a1d48) SHA1(74f16ef5126f0ce3d94a66849ccd7c28338e3974) )
	ROM_LOAD16_BYTE( "tp08.bin",  0x000001, 0x20000, CRC(d8cc44af) SHA1(da9c07e3670e5c7a2c1f9bc433e604a2a13b8a54) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound Z80 code */
	ROM_LOAD( "tp09.bin",  0x0000, 0x8000, CRC(dd56041f) SHA1(a481b8959b349761624166906175f8efcbebb7e7) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "rom5.bin",  0x00000, 0x80000, CRC(c64ec7b6) SHA1(e73b51c3713c2ea7a572a02531c15d1261ddeaa0) )
	ROM_LOAD( "rom6.bin",  0x80000, 0x80000, CRC(64b6c5ac) SHA1(07fa20115f603445c0d51af3465c0471c09d76b1) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "rom2.bin",  0x00000, 0x20000, CRC(6bb72d16) SHA1(a127b10d9c255542bd09fcb5df057c12fd28c0d1) )
	ROM_LOAD( "rom1.bin",  0x20000, 0x20000, CRC(0934782d) SHA1(e4a775ead23227d7d6e76aea23aa3103b511d031) )
	ROM_LOAD( "rom3.bin",  0x40000, 0x20000, CRC(ec903c07) SHA1(75906f31200877fc8f6e78c2606ad5be49778165) )
	ROM_LOAD( "rom4.bin",  0x60000, 0x20000, CRC(50cbf1a8) SHA1(cfab1504746654b4a61912155e9aeca746c65321) )

	ROM_REGION( 0x40, "proms", 0 )		/* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "tp018_10.bpr",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )	/* sprite attribute (flip/position) ?? */
	ROM_LOAD( "tp018_11.bpr",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )	/* ??? */
ROM_END

ROM_START( vimana )			/* From board serial number 1547.04 (July '94) */
	ROM_REGION( 0x040000, "maincpu", 0 )	/* Main 68K code */
	ROM_LOAD16_BYTE( "tp019-7a.bin",  0x000000, 0x20000, CRC(5a4bf73e) SHA1(9a43d822bc24b59278f294d0b3275595de997d16) )
	ROM_LOAD16_BYTE( "tp019-8a.bin",  0x000001, 0x20000, CRC(03ba27e8) SHA1(edb5fe741d2a6a7fe5cde9a82317ea1e9447cf73) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound HD647180 code */
	/* sound CPU is a HD647180 (Z180) with internal ROM - not yet supported */
	ROM_LOAD( "hd647180.019",  0x00000, 0x08000, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "vim6.bin",  0x00000, 0x20000, CRC(2886878d) SHA1(f44933d87bbcd3bd58f46e0f0f89b05c409b713b) )
	ROM_LOAD( "vim5.bin",  0x20000, 0x20000, CRC(61a63d7a) SHA1(5cdebc03110252cc43d31b6f87f9a23556892977) )
	ROM_LOAD( "vim4.bin",  0x40000, 0x20000, CRC(b0515768) SHA1(9907b52b4d30ce5324270a12c40250068adafca8) )
	ROM_LOAD( "vim3.bin",  0x60000, 0x20000, CRC(0b539131) SHA1(07f3e3b9b28c8218e36668c24d16dbb6e9a66889) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "vim1.bin",  0x00000, 0x80000, CRC(cdde26cd) SHA1(27893af4692ec7bcbaac9e790c0707c98df84e62) )
	ROM_LOAD( "vim2.bin",  0x80000, 0x80000, CRC(1dbfc118) SHA1(4fd039a3172f73ad910349b2d360e8ae77ccddb2) )

	ROM_REGION( 0x40, "proms", 0 )		/* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "tp019-09.bpr",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )	/* sprite attribute (flip/position) ?? */
	ROM_LOAD( "tp019-10.bpr",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )	/* ??? */
ROM_END

ROM_START( vimanan )
	ROM_REGION( 0x040000, "maincpu", 0 )	/* Main 68K code */
	ROM_LOAD16_BYTE( "tp019-07.rom",  0x000000, 0x20000, CRC(78888ff2) SHA1(7e1d248f806d585952eb35ceec6a7e63ae4e22f9) )
	ROM_LOAD16_BYTE( "tp019-08.rom",  0x000001, 0x20000, CRC(6cd2dc3c) SHA1(029d974eb938c5e2fbe7575f0dda342b4b12b731) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound HD647180 code */
	/* sound CPU is a HD647180 (Z180) with internal ROM - not yet supported */
	ROM_LOAD( "hd647180.019",  0x00000, 0x08000, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "vim6.bin",  0x00000, 0x20000, CRC(2886878d) SHA1(f44933d87bbcd3bd58f46e0f0f89b05c409b713b) )
	ROM_LOAD( "vim5.bin",  0x20000, 0x20000, CRC(61a63d7a) SHA1(5cdebc03110252cc43d31b6f87f9a23556892977) )
	ROM_LOAD( "vim4.bin",  0x40000, 0x20000, CRC(b0515768) SHA1(9907b52b4d30ce5324270a12c40250068adafca8) )
	ROM_LOAD( "vim3.bin",  0x60000, 0x20000, CRC(0b539131) SHA1(07f3e3b9b28c8218e36668c24d16dbb6e9a66889) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "vim1.bin",  0x00000, 0x80000, CRC(cdde26cd) SHA1(27893af4692ec7bcbaac9e790c0707c98df84e62) )
	ROM_LOAD( "vim2.bin",  0x80000, 0x80000, CRC(1dbfc118) SHA1(4fd039a3172f73ad910349b2d360e8ae77ccddb2) )

	ROM_REGION( 0x40, "proms", 0 )		/* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "tp019-09.bpr",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )	/* sprite attribute (flip/position) ?? */
	ROM_LOAD( "tp019-10.bpr",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )	/* ??? */
ROM_END

ROM_START( vimana1 )
	ROM_REGION( 0x040000, "maincpu", 0 )	/* Main 68K code */
	ROM_LOAD16_BYTE( "vim07.bin",  0x000000, 0x20000, CRC(1efaea84) SHA1(f9c5d2365d8948fa66dbe61d355919db15843a28) )
	ROM_LOAD16_BYTE( "vim08.bin",  0x000001, 0x20000, CRC(e45b7def) SHA1(6b92a91d64581954da8ecdbeb5fed79bcc9c5217) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound HD647180 code */
	/* sound CPU is a HD647180 (Z180) with internal ROM - not yet supported */
	ROM_LOAD( "hd647180.019",  0x00000, 0x08000, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "vim6.bin",  0x00000, 0x20000, CRC(2886878d) SHA1(f44933d87bbcd3bd58f46e0f0f89b05c409b713b) )
	ROM_LOAD( "vim5.bin",  0x20000, 0x20000, CRC(61a63d7a) SHA1(5cdebc03110252cc43d31b6f87f9a23556892977) )
	ROM_LOAD( "vim4.bin",  0x40000, 0x20000, CRC(b0515768) SHA1(9907b52b4d30ce5324270a12c40250068adafca8) )
	ROM_LOAD( "vim3.bin",  0x60000, 0x20000, CRC(0b539131) SHA1(07f3e3b9b28c8218e36668c24d16dbb6e9a66889) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "vim1.bin",  0x00000, 0x80000, CRC(cdde26cd) SHA1(27893af4692ec7bcbaac9e790c0707c98df84e62) )
	ROM_LOAD( "vim2.bin",  0x80000, 0x80000, CRC(1dbfc118) SHA1(4fd039a3172f73ad910349b2d360e8ae77ccddb2) )

	ROM_REGION( 0x40, "proms", 0 )		/* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "tp019-09.bpr",  0x00, 0x20, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )	/* sprite attribute (flip/position) ?? */
	ROM_LOAD( "tp019-10.bpr",  0x20, 0x20, CRC(a1e17492) SHA1(9ddec4c97f2d541f69f3c32c47aaa21fd9699ae2) )	/* ??? */
ROM_END


static DRIVER_INIT( toaplan1 )
{
	toaplan1_driver_savestate(machine);
}

static DRIVER_INIT( demonwld )
{
	toaplan1_driver_savestate(machine);
	demonwld_driver_savestate(machine);
}

static DRIVER_INIT( vimana )
{
	toaplan1_driver_savestate(machine);
	vimana_driver_savestate(machine);
}


//    YEAR, NAME,       PARENT,   MACHINE,  INPUT,      INIT,     MONITOR,COMPANY,FULLNAME,FLAGS
GAME( 1988, rallybik,   0,        rallybik, rallybik,   toaplan1, ROT270, "Toaplan / Taito Corporation", "Rally Bike / Dash Yarou", 0 )
GAME( 1988, truxton,    0,        truxton,  truxton,    toaplan1, ROT270, "Toaplan / Taito Corporation", "Truxton / Tatsujin", 0 )
GAME( 1989, hellfire,   0,        hellfire, hellfire,   toaplan1, ROT0,   "Toaplan (Taito license)", "Hellfire (2P set)", 0 ) // 2P = simultaneous players
GAME( 1989, hellfire1,  hellfire, hellfire, hellfire1,  toaplan1, ROT0,   "Toaplan (Taito license)", "Hellfire (1P set, ealier)", 0 ) // 1P = alternating players
GAME( 1989, hellfire2,  hellfire, hellfire, hellfire2,  toaplan1, ROT0,   "Toaplan (Taito license)", "Hellfire (2P set, ealier)", 0 )
GAME( 1989, hellfire3,  hellfire, hellfire, hellfire3,  toaplan1, ROT0,   "Toaplan (Taito license)", "Hellfire (1P set)", 0 )
GAME( 1989, zerowing,   0,        zerowing, zerowing,   toaplan1, ROT0,   "Toaplan", "Zero Wing (1P set)", 0 )
GAME( 1989, zerowing2,  zerowing, zerowing, zerowing2,  toaplan1, ROT0,   "Toaplan", "Zero Wing (2P set)", 0 )
GAME( 1990, demonwld,   0,        demonwld, demonwld,   demonwld, ROT0,   "Toaplan", "Demon's World / Horror Story (set 1)", 0 )
GAME( 1989, demonwld1,  demonwld, demonwld, demonwld1,  demonwld, ROT0,   "Toaplan", "Demon's World / Horror Story (set 2)", 0 )
GAME( 1989, demonwld2,  demonwld, demonwld, demonwld,   demonwld, ROT0,   "Toaplan", "Demon's World / Horror Story (set 3)", 0 )
GAME( 1989, demonwld3,  demonwld, demonwld, demonwld,   demonwld, ROT0,   "Toaplan", "Demon's World / Horror Story (set 4)", 0 )
GAME( 1990, fireshrk,   0,        samesame, fireshrk,   toaplan1, ROT270, "Toaplan", "Fire Shark", GAME_NO_SOUND )
GAME( 1990, fireshrkd,  fireshrk, samesame, samesame2,  toaplan1, ROT270, "Toaplan (Dooyong license)", "Fire Shark (Korea, set 1, easier)", GAME_NO_SOUND )
GAME( 1990, fireshrkdh, fireshrk, samesame, fireshrkdh, toaplan1, ROT270, "Toaplan (Dooyong license)", "Fire Shark (Korea, set 2, harder)", GAME_NO_SOUND )
GAME( 1989, samesame,   fireshrk, samesame, samesame,   toaplan1, ROT270, "Toaplan", "Same! Same! Same! (1P set)", GAME_NO_SOUND )
GAME( 1989, samesame2,  fireshrk, samesame, samesame2,  toaplan1, ROT270, "Toaplan", "Same! Same! Same! (2P set)", GAME_NO_SOUND )
GAME( 1990, outzone,    0,        outzone,  outzone,    toaplan1, ROT270, "Toaplan", "Out Zone (set 1)", 0 ) // later fixed version
GAME( 1990, outzonea,   outzone,  outzone,  outzonea,   toaplan1, ROT270, "Toaplan", "Out Zone (set 2)", 0 )
GAME( 1990, outzoneb,   outzone,  outzone,  outzoneb,   toaplan1, ROT270, "Toaplan", "Out Zone (set 3, prototype?)", 0 ) // early revision at least
GAME( 1990, outzonec,   outzone,  outzone,  outzone,    toaplan1, ROT270, "Toaplan", "Out Zone (set 4)", 0 )
GAME( 1990, outzoned,   outzone,  outzone,  outzonea,   toaplan1, ROT270, "Toaplan", "Out Zone (set 5)", 0 )
GAME( 1991, vimana,     0,        vimana,   vimana,     vimana,   ROT270, "Toaplan", "Vimana (World, set 1)", GAME_NO_SOUND )
GAME( 1991, vimanan,    vimana,   vimana,   vimanan,    vimana,   ROT270, "Toaplan", "Vimana (World, set 2)", GAME_NO_SOUND )
GAME( 1991, vimana1,    vimana,   vimana,   vimana1,    vimana,   ROT270, "Toaplan", "Vimana (Japan)", GAME_NO_SOUND )
