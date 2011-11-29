/***************************************************************************

 Galaxian/Moon Cresta hardware


Main clock: XTAL = 18.432 MHz
Z80 Clock: XTAL/6 = 3.072 MHz
Horizontal video frequency: HSYNC = XTAL/3/192/2 = 16 kHz
Video frequency: VSYNC = HSYNC/132/2 = 60.606060 Hz
VBlank duration: 1/VSYNC * (20/132) = 2500 us


Notes:
-----

- The only code difference between 'galaxian' and 'galmidw' is that the
  'BONUS SHIP' text is printed on a different line.


TODO:
----

- Problems with Galaxian based on the observation of a real machine:

  - Starfield is incorrect.  The speed and flashing frequency is fine, but the
    stars appear in different positions.
  - Background humming is incorrect.  It's faster on a real machine
  - Explosion sound is much softer.  Filter involved?

- $4800-4bff in Streaking/Ghost Muncher



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


Stephh's notes (based on the games Z80 code and some tests) for other games :

1) 'scramblb' and 'scramb2'

  - Player 2 controls are used for player 2 regardless of the "Cabinet" Dip Switch
    (check code at 0x1043 which changes player and routines that handle players inputs :
    0x1741 UP and DOWN - 0x1796 LEFT and RIGHT - 0x24e6 BUTTON1 - 0x2615 BUTTON2).

2) 'bagmanmc'

  - DSW bit 6 was previously used for "Bonus Lives" settings, but it has no effect
    in this set because of 'NOP' instructions from 0x3501 to 0x3507.

3) 'porter'

  - It's difficult to map correctly players buttons because of what they do :
    on one side, both buttons do the same thing (code at 0x0940 for player 1 and
    player 2 in "Upright" cabinet, or 0x1cc0 for player 2 in "Cocktail" cabinet),
    but on the other side, due to code at 0x0910, player 1 BUTTON1 acts as a
    START1 button while player 1 BUTTON2 acts as a START2 button. Any help is welcome !

4) 'tazzmang'

  - If you press COIN2 during the boot-up sequence, you enter sort of "test mode"
    where you can access to all inputs, but this doesn't give a clue about what
    they do (only the status - 0 or 1 - is displayed).
  - IN1 bit 0 has 2 effects : it starts a one player game (code at 0x5585),
    but it also acts as 2nd button (bomb) for BOTH players regardless of "Cabinet"
    settings (code at 0x5805 for player 1 and player 2 in "Upright" cabinet, or
    0x5563 for player 2 in "Cocktail" cabinet).

5) 'bongo'

  - IN0 bit 1 is supposed to be COIN2 (see coinage routine at 0x0288), but
    there is a test on it at 0x0082 (in NMI routine) which jumps to 0xc003
    (unmapped memory) if it pressed (HIGH).
  - IN0 bit 7 is tested on startup (code at 0x0048) in combinaison with bits 0 and 1
    (which are supposed to be COIN1 and COIN2). If all of them are pressed (HIGH),
    the game displays a "CREDIT FAULT" message then jumps back to 0x0048.
  - IN0 bit 4 and IN1 bit 4 should have been IPT_JOYSTICK_DOWN (Upright and Cocktail)
    but their status is discarded with 3 'NOP' instructions at 0x06ca.
  - IN0 bit 7 and IN0 bit 6 should have been IPT_BUTTON1 (Upright and Cocktail)
    but their status is discarded with 3 'NOP' instructions at 0x06d1.
  - DSW0 is read via code at 0x2426, but its contents is directly overwritten
    with value read from DSW1 (AY port A) via code at 0x3647.

4) 'ozon1'

  - Player 2 controls are used for player 2 regardless of the "Cabinet" Dip Switch
    (check code at 0x03c6 which changes player and routines that handle players inputs :
    0x0dc3 and 0x1e31 LEFT and RIGHT - 0x0e76 BUTTON1).
  - Credits are coded on 1 byte (range 0x00-0xff) and stored at 0x4002.
    To display them, they are converted to BCD on 1 byte via routine at 0x1421.
    As a result, it will always display 0 to 99 (eg: 0xf0 = 240 will display 40).
    When you get 256 credits, 0x4002 = 0x00, so the game thinks you have no credit
    at all and enters "attract mode" again (but the game does NOT reset).
  - There's an ingame bug when you get 101 or 201 credits : due to code at 0x0239,
    the game checks the BCD value (0x01) instead of the correct one at 0x4002,
    so you can't start a 2 players game !
  - There is another ingame bug when "Coinage" settings are "A 1C/2C  B 1C/1C"
    and you press COIN2 : due to code at 0x0473, contents of 0x4004 is NEVER reset
    to 0x00, so routine at 0x042a ALWAYS thinks that you've pressed COIN2,
    and as a consequence, it ALWAYS adds 1 credit (even when you are playing) !


***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/s2650/s2650.h"
#include "machine/7474.h"
#include "sound/ay8910.h"
#include "sound/sn76496.h"
#include "sound/dac.h"
#include "sound/flt_rc.h"
#include "audio/galaxian.h"
#include "includes/galaxold.h"


/*************************************
 *
 *  Constants
 *
 *************************************/

#define MASTER_CLOCK		(XTAL_18_432MHz)

#define PIXEL_CLOCK			(MASTER_CLOCK/3)

/* H counts from 128->511, HBLANK starts at 128 and ends at 256 */
#define HTOTAL				(384)
#define HBEND				(0)		/*(256)*/
#define HBSTART				(256)	/*(128)*/

#define VTOTAL				(264)
#define VBEND				(16)
#define VBSTART				(224+16)



/* Send sound data to the sound cpu and cause an nmi */
static READ8_HANDLER( drivfrcg_port0_r )
{
	switch (cpu_get_pc(&space->device()))
	{
		case 0x002e:
		case 0x0297:
			return 0x01;
	}

    return 0;
}

static ADDRESS_MAP_START( galaxold_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x5000, 0x53ff) AM_RAM_WRITE(galaxold_videoram_w) AM_BASE_MEMBER(galaxold_state, m_videoram)
	AM_RANGE(0x5400, 0x57ff) AM_READ(galaxold_videoram_r)
	AM_RANGE(0x5800, 0x583f) AM_RAM_WRITE(galaxold_attributesram_w) AM_BASE_MEMBER(galaxold_state, m_attributesram)
	AM_RANGE(0x5840, 0x585f) AM_RAM AM_BASE_MEMBER(galaxold_state, m_spriteram) AM_SIZE_MEMBER(galaxold_state, m_spriteram_size)
	AM_RANGE(0x5860, 0x587f) AM_RAM AM_BASE_MEMBER(galaxold_state, m_bulletsram) AM_SIZE_MEMBER(galaxold_state, m_bulletsram_size)
	AM_RANGE(0x5880, 0x58ff) AM_RAM
	AM_RANGE(0x6000, 0x6000) AM_READ_PORT("IN0")
	AM_RANGE(0x6000, 0x6001) AM_WRITE(galaxold_leds_w)
	AM_RANGE(0x6002, 0x6002) AM_WRITE(galaxold_coin_lockout_w)
	AM_RANGE(0x6003, 0x6003) AM_WRITE(galaxold_coin_counter_w)
	AM_RANGE(0x6004, 0x6007) AM_DEVWRITE(GAL_AUDIO, galaxian_lfo_freq_w)
	AM_RANGE(0x6800, 0x6800) AM_READ_PORT("IN1")
	AM_RANGE(0x6800, 0x6802) AM_DEVWRITE(GAL_AUDIO, galaxian_background_enable_w)
	AM_RANGE(0x6803, 0x6803) AM_DEVWRITE(GAL_AUDIO, galaxian_noise_enable_w)
	AM_RANGE(0x6805, 0x6805) AM_DEVWRITE(GAL_AUDIO, galaxian_shoot_enable_w)
	AM_RANGE(0x6806, 0x6807) AM_DEVWRITE(GAL_AUDIO, galaxian_vol_w)
	AM_RANGE(0x7000, 0x7000) AM_READ_PORT("IN2")
	AM_RANGE(0x7001, 0x7001) AM_WRITE(galaxold_nmi_enable_w)
	AM_RANGE(0x7004, 0x7004) AM_WRITE(galaxold_stars_enable_w)
	AM_RANGE(0x7006, 0x7006) AM_WRITE(galaxold_flip_screen_x_w)
	AM_RANGE(0x7007, 0x7007) AM_WRITE(galaxold_flip_screen_y_w)
	AM_RANGE(0x7800, 0x7fff) AM_READ(watchdog_reset_r)
	AM_RANGE(0x7800, 0x7800) AM_DEVWRITE(GAL_AUDIO, galaxian_pitch_w)
	AM_RANGE(0xfffc, 0xffff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( mooncrst_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x9000, 0x93ff) AM_RAM_WRITE(galaxold_videoram_w) AM_BASE_MEMBER(galaxold_state, m_videoram)
	AM_RANGE(0x9400, 0x97ff) AM_READ(galaxold_videoram_r)
	AM_RANGE(0x9800, 0x983f) AM_RAM_WRITE(galaxold_attributesram_w) AM_BASE_MEMBER(galaxold_state, m_attributesram)
	AM_RANGE(0x9840, 0x985f) AM_RAM AM_BASE_MEMBER(galaxold_state, m_spriteram) AM_SIZE_MEMBER(galaxold_state, m_spriteram_size)
	AM_RANGE(0x9860, 0x987f) AM_RAM AM_BASE_MEMBER(galaxold_state, m_bulletsram) AM_SIZE_MEMBER(galaxold_state, m_bulletsram_size)
	AM_RANGE(0x9880, 0x98ff) AM_RAM
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("IN0")
	AM_RANGE(0xa003, 0xa003) AM_WRITE(galaxold_coin_counter_w)
	AM_RANGE(0xa004, 0xa007) AM_DEVWRITE(GAL_AUDIO, galaxian_lfo_freq_w)
	AM_RANGE(0xa800, 0xa800) AM_READ_PORT("IN1")
	AM_RANGE(0xa800, 0xa802) AM_DEVWRITE(GAL_AUDIO, galaxian_background_enable_w)
	AM_RANGE(0xa803, 0xa803) AM_DEVWRITE(GAL_AUDIO, galaxian_noise_enable_w)
	AM_RANGE(0xa805, 0xa805) AM_DEVWRITE(GAL_AUDIO, galaxian_shoot_enable_w)
	AM_RANGE(0xa806, 0xa807) AM_DEVWRITE(GAL_AUDIO, galaxian_vol_w)
	AM_RANGE(0xb000, 0xb000) AM_READ_PORT("DSW0") AM_WRITE(galaxold_nmi_enable_w)
	AM_RANGE(0xb004, 0xb004) AM_WRITE(galaxold_stars_enable_w)
	AM_RANGE(0xb006, 0xb006) AM_WRITE(galaxold_flip_screen_x_w)
	AM_RANGE(0xb007, 0xb007) AM_WRITE(galaxold_flip_screen_y_w)
	AM_RANGE(0xb800, 0xb800) AM_READ(watchdog_reset_r)
	AM_RANGE(0xb800, 0xb800) AM_DEVWRITE(GAL_AUDIO, galaxian_pitch_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( rockclim_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_READWRITE(rockclim_videoram_r, rockclim_videoram_w) AM_BASE_MEMBER(galaxold_state, m_rockclim_videoram)//4800 - 4803 = bg scroll ?
	AM_RANGE(0x4800, 0x4803) AM_WRITE(rockclim_scroll_w)
	AM_RANGE(0x5000, 0x53ff) AM_RAM //?
	AM_RANGE(0x5800, 0x5800) AM_READ_PORT("IN2")
	AM_RANGE(0x6000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0x8800) AM_READ_PORT("DSW1")
	AM_RANGE(0x9000, 0x93ff) AM_RAM_WRITE(galaxold_videoram_w) AM_BASE_MEMBER(galaxold_state, m_videoram)
	AM_RANGE(0x9400, 0x97ff) AM_READ(galaxold_videoram_r)
	AM_RANGE(0x9800, 0x983f) AM_RAM_WRITE(galaxold_attributesram_w) AM_BASE_MEMBER(galaxold_state, m_attributesram)
	AM_RANGE(0x9840, 0x985f) AM_RAM AM_BASE_MEMBER(galaxold_state, m_spriteram) AM_SIZE_MEMBER(galaxold_state, m_spriteram_size)
	AM_RANGE(0x9860, 0x987f) AM_RAM AM_BASE_MEMBER(galaxold_state, m_bulletsram) AM_SIZE_MEMBER(galaxold_state, m_bulletsram_size)
	AM_RANGE(0x9880, 0x98ff) AM_RAM
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("IN0")
	AM_RANGE(0xa000, 0xa002) AM_WRITE(galaxold_gfxbank_w)// a002 - sprite bank
	AM_RANGE(0xa003, 0xa003) AM_WRITE(galaxold_coin_counter_w)
	AM_RANGE(0xa004, 0xa007) AM_DEVWRITE(GAL_AUDIO, galaxian_lfo_freq_w)
	AM_RANGE(0xa800, 0xa800) AM_READ_PORT("IN1")
	AM_RANGE(0xa800, 0xa802) AM_DEVWRITE(GAL_AUDIO, galaxian_background_enable_w)
	AM_RANGE(0xa803, 0xa803) AM_DEVWRITE(GAL_AUDIO, galaxian_noise_enable_w)
	AM_RANGE(0xa805, 0xa805) AM_DEVWRITE(GAL_AUDIO, galaxian_shoot_enable_w)
	AM_RANGE(0xa806, 0xa807) AM_DEVWRITE(GAL_AUDIO, galaxian_vol_w)
	AM_RANGE(0xb000, 0xb000) AM_READ_PORT("DSW0") AM_WRITE(galaxold_nmi_enable_w)
	AM_RANGE(0xb006, 0xb006) AM_WRITE(galaxold_flip_screen_x_w)
	AM_RANGE(0xb007, 0xb007) AM_WRITE(galaxold_flip_screen_y_w)
	AM_RANGE(0xb800, 0xb800) AM_READ(watchdog_reset_r)
	AM_RANGE(0xb800, 0xb800) AM_DEVWRITE(GAL_AUDIO, galaxian_pitch_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( ckongg_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x6fff) AM_RAM
	AM_RANGE(0x9000, 0x93ff) AM_READWRITE(galaxold_videoram_r, galaxold_videoram_w) AM_BASE_MEMBER(galaxold_state, m_videoram)
	AM_RANGE(0x9800, 0x983f) AM_RAM_WRITE(galaxold_attributesram_w) AM_BASE_MEMBER(galaxold_state, m_attributesram)
	AM_RANGE(0x9840, 0x985f) AM_RAM AM_BASE_MEMBER(galaxold_state, m_spriteram) AM_SIZE_MEMBER(galaxold_state, m_spriteram_size)
	AM_RANGE(0x9860, 0x987f) AM_RAM AM_BASE_MEMBER(galaxold_state, m_bulletsram) AM_SIZE_MEMBER(galaxold_state, m_bulletsram_size)
	AM_RANGE(0x9880, 0x98ff) AM_RAM
	AM_RANGE(0xc000, 0xc000) AM_READ_PORT("IN0")
	AM_RANGE(0xc000, 0xc001) AM_WRITE(galaxold_leds_w)
	AM_RANGE(0xc002, 0xc002) AM_WRITE(galaxold_coin_lockout_w)
	AM_RANGE(0xc003, 0xc003) AM_WRITE(galaxold_coin_counter_w)
	AM_RANGE(0xc004, 0xc007) AM_DEVWRITE(GAL_AUDIO, galaxian_lfo_freq_w)
	AM_RANGE(0xc400, 0xc400) AM_READ_PORT("IN1")
	AM_RANGE(0xc400, 0xc402) AM_DEVWRITE(GAL_AUDIO, galaxian_background_enable_w)
	AM_RANGE(0xc403, 0xc403) AM_DEVWRITE(GAL_AUDIO, galaxian_noise_enable_w)
	AM_RANGE(0xc405, 0xc405) AM_DEVWRITE(GAL_AUDIO, galaxian_shoot_enable_w)
	AM_RANGE(0xc406, 0xc407) AM_DEVWRITE(GAL_AUDIO, galaxian_vol_w)
	AM_RANGE(0xc800, 0xc800) AM_READ_PORT("DSW")
	AM_RANGE(0xc801, 0xc801) AM_WRITE(galaxold_nmi_enable_w)
	AM_RANGE(0xc804, 0xc804) AM_WRITENOP // link cut
	AM_RANGE(0xc806, 0xc806) AM_WRITE(galaxold_flip_screen_x_w)
	AM_RANGE(0xc807, 0xc807) AM_WRITE(galaxold_flip_screen_y_w)
	AM_RANGE(0xcc00, 0xcc00) AM_READ(watchdog_reset_r) AM_DEVWRITE(GAL_AUDIO, galaxian_pitch_w)
ADDRESS_MAP_END

/* Memory map based on mooncrst_map according to Z80 code - seems to be good but needs further checking */
static ADDRESS_MAP_START( ckongmc_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x6fff) AM_RAM
	AM_RANGE(0x9000, 0x93ff) AM_READWRITE(galaxold_videoram_r, galaxold_videoram_w) AM_BASE_MEMBER(galaxold_state, m_videoram)
	AM_RANGE(0x9800, 0x983f) AM_RAM_WRITE(galaxold_attributesram_w) AM_BASE_MEMBER(galaxold_state, m_attributesram)
	AM_RANGE(0x9840, 0x985f) AM_RAM AM_BASE_MEMBER(galaxold_state, m_spriteram) AM_SIZE_MEMBER(galaxold_state, m_spriteram_size)
	AM_RANGE(0x9860, 0x987f) AM_RAM AM_BASE_MEMBER(galaxold_state, m_bulletsram) AM_SIZE_MEMBER(galaxold_state, m_bulletsram_size)
	AM_RANGE(0x9880, 0x98ff) AM_RAM
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("IN0")
	AM_RANGE(0xa001, 0xa002) AM_WRITE(galaxold_leds_w)                                              /* GUESS */
//  AM_RANGE(0xa002, 0xa002) AM_WRITE(galaxold_coin_lockout_w)                                      /* not written */
//  AM_RANGE(0xa003, 0xa003) AM_WRITE(galaxold_coin_counter_w)                                      /* not written */
	AM_RANGE(0xa004, 0xa007) AM_DEVWRITE(GAL_AUDIO, galaxian_lfo_freq_w)                            /* GUESS */
	AM_RANGE(0xa800, 0xa800) AM_READ_PORT("IN1")
	AM_RANGE(0xa800, 0xa802) AM_DEVWRITE(GAL_AUDIO, galaxian_background_enable_w)                   /* GUESS */
	AM_RANGE(0xa803, 0xa803) AM_DEVWRITE(GAL_AUDIO, galaxian_noise_enable_w)                        /* GUESS */
	AM_RANGE(0xa805, 0xa805) AM_DEVWRITE(GAL_AUDIO, galaxian_shoot_enable_w)                        /* GUESS */
	AM_RANGE(0xa806, 0xa807) AM_DEVWRITE(GAL_AUDIO, galaxian_vol_w)                                 /* GUESS */
	AM_RANGE(0xb000, 0xb000) AM_READ_PORT("DSW")
	AM_RANGE(0xb001, 0xb001) AM_WRITE(galaxold_nmi_enable_w)
	AM_RANGE(0xb004, 0xb004) AM_WRITENOP                                                            /* AM_WRITE(galaxold_stars_enable_w) */
	AM_RANGE(0xb006, 0xb006) AM_WRITE(galaxold_flip_screen_x_w)                                     /* GUESS */
	AM_RANGE(0xb007, 0xb007) AM_WRITE(galaxold_flip_screen_y_w)                                     /* GUESS */
	AM_RANGE(0xb800, 0xb800) AM_READ(watchdog_reset_r) AM_DEVWRITE(GAL_AUDIO, galaxian_pitch_w)     /* GUESS */
ADDRESS_MAP_END


static ADDRESS_MAP_START( scramblb_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x4800, 0x4bff) AM_RAM_WRITE(galaxold_videoram_w) AM_BASE_MEMBER(galaxold_state, m_videoram)
	AM_RANGE(0x5000, 0x503f) AM_RAM_WRITE(galaxold_attributesram_w) AM_BASE_MEMBER(galaxold_state, m_attributesram)
	AM_RANGE(0x5040, 0x505f) AM_RAM AM_BASE_MEMBER(galaxold_state, m_spriteram) AM_SIZE_MEMBER(galaxold_state, m_spriteram_size)
	AM_RANGE(0x5060, 0x507f) AM_RAM AM_BASE_MEMBER(galaxold_state, m_bulletsram) AM_SIZE_MEMBER(galaxold_state, m_bulletsram_size)
	AM_RANGE(0x5080, 0x50ff) AM_RAM
	AM_RANGE(0x6000, 0x6000) AM_READ_PORT("IN0")
	AM_RANGE(0x6000, 0x6001) AM_WRITENOP  /* sound triggers */
	AM_RANGE(0x6003, 0x6003) AM_WRITE(galaxold_coin_counter_w)
	AM_RANGE(0x6004, 0x6007) AM_DEVWRITE(GAL_AUDIO, galaxian_lfo_freq_w)
	AM_RANGE(0x6800, 0x6800) AM_READ_PORT("IN1")
	AM_RANGE(0x6800, 0x6802) AM_DEVWRITE(GAL_AUDIO, galaxian_background_enable_w)
	AM_RANGE(0x6803, 0x6803) AM_DEVWRITE(GAL_AUDIO, galaxian_noise_enable_w)
	AM_RANGE(0x6805, 0x6805) AM_DEVWRITE(GAL_AUDIO, galaxian_shoot_enable_w)
	AM_RANGE(0x6806, 0x6807) AM_DEVWRITE(GAL_AUDIO, galaxian_vol_w)
	AM_RANGE(0x7000, 0x7000) AM_READ_PORT("IN2")
	AM_RANGE(0x7001, 0x7001) AM_WRITE(galaxold_nmi_enable_w)
	AM_RANGE(0x7002, 0x7002) AM_WRITE(galaxold_coin_counter_w)
	AM_RANGE(0x7003, 0x7003) AM_WRITE(scrambold_background_enable_w)
	AM_RANGE(0x7004, 0x7004) AM_WRITE(galaxold_stars_enable_w)
	AM_RANGE(0x7006, 0x7006) AM_WRITE(galaxold_flip_screen_x_w)
	AM_RANGE(0x7007, 0x7007) AM_WRITE(galaxold_flip_screen_y_w)
	AM_RANGE(0x7800, 0x7800) AM_READ(watchdog_reset_r)
	AM_RANGE(0x7800, 0x7800) AM_DEVWRITE(GAL_AUDIO, galaxian_pitch_w)
	AM_RANGE(0x8102, 0x8102) AM_READ(scramblb_protection_1_r)
	AM_RANGE(0x8202, 0x8202) AM_READ(scramblb_protection_2_r)
ADDRESS_MAP_END

static READ8_HANDLER( scramb2_protection_r ) { return 0x25; }
static READ8_HANDLER( scramb2_port0_r ) { return (input_port_read(space->machine(), "IN0") >> offset) & 0x1; }
static READ8_HANDLER( scramb2_port1_r ) { return (input_port_read(space->machine(), "IN1") >> offset) & 0x1; }
static READ8_HANDLER( scramb2_port2_r ) { return (input_port_read(space->machine(), "IN2") >> offset) & 0x1; }

static ADDRESS_MAP_START( scramb2_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x4800, 0x4bff) AM_RAM_WRITE(galaxold_videoram_w) AM_BASE_MEMBER(galaxold_state, m_videoram)
	AM_RANGE(0x4c00, 0x4fff) AM_WRITE(galaxold_videoram_w) // mirror
	AM_RANGE(0x5000, 0x503f) AM_RAM_WRITE(galaxold_attributesram_w) AM_BASE_MEMBER(galaxold_state, m_attributesram)
	AM_RANGE(0x5040, 0x505f) AM_RAM AM_BASE_MEMBER(galaxold_state, m_spriteram) AM_SIZE_MEMBER(galaxold_state, m_spriteram_size)
	AM_RANGE(0x5060, 0x507f) AM_RAM AM_BASE_MEMBER(galaxold_state, m_bulletsram) AM_SIZE_MEMBER(galaxold_state, m_bulletsram_size)
	AM_RANGE(0x5080, 0x50ff) AM_RAM
	AM_RANGE(0x5800, 0x5fff) AM_READ(scramb2_protection_r) // must return 0x25
	AM_RANGE(0x6000, 0x6007) AM_READ(scramb2_port0_r) // reads from 8 addresses, 1 bit per address
	AM_RANGE(0x6800, 0x6807) AM_READ(scramb2_port1_r) // reads from 8 addresses, 1 bit per address
	AM_RANGE(0x6801, 0x6801) AM_WRITE(galaxold_nmi_enable_w)
	AM_RANGE(0x6802, 0x6802) AM_WRITE(galaxold_coin_counter_w)
	AM_RANGE(0x6804, 0x6804) AM_WRITE(galaxold_stars_enable_w)
	AM_RANGE(0x6806, 0x6806) AM_WRITE(galaxold_flip_screen_x_w)
	AM_RANGE(0x6807, 0x6807) AM_WRITE(galaxold_flip_screen_y_w)
	AM_RANGE(0x7000, 0x7007) AM_READ(watchdog_reset_r)
	AM_RANGE(0x7006, 0x7006) AM_WRITENOP
	AM_RANGE(0x7007, 0x7007) AM_WRITENOP
	AM_RANGE(0x7800, 0x7807) AM_READ(scramb2_port2_r) // reads from 8 addresses, 1 bit per address
	AM_RANGE(0x7800, 0x7800) AM_DEVWRITE(GAL_AUDIO, galaxian_pitch_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( _4in1_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROMBANK("bank1")	/* banked game code */
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x5000, 0x53ff) AM_RAM_WRITE(galaxold_videoram_w) AM_BASE_MEMBER(galaxold_state, m_videoram)
	AM_RANGE(0x5400, 0x57ff) AM_READ(galaxold_videoram_r)
	AM_RANGE(0x5800, 0x583f) AM_RAM_WRITE(galaxold_attributesram_w) AM_BASE_MEMBER(galaxold_state, m_attributesram)
	AM_RANGE(0x5840, 0x585f) AM_RAM AM_BASE_MEMBER(galaxold_state, m_spriteram) AM_SIZE_MEMBER(galaxold_state, m_spriteram_size)
	AM_RANGE(0x5860, 0x587f) AM_RAM AM_BASE_MEMBER(galaxold_state, m_bulletsram) AM_SIZE_MEMBER(galaxold_state, m_bulletsram_size)
	AM_RANGE(0x5880, 0x58ff) AM_RAM
	AM_RANGE(0x6000, 0x6000) AM_READ_PORT("IN0")
	AM_RANGE(0x6000, 0x6001) AM_WRITE(galaxold_leds_w)
//  AM_RANGE(0x6002, 0x6002) AM_WRITE(galaxold_coin_lockout_w)
	AM_RANGE(0x6003, 0x6003) AM_WRITE(galaxold_coin_counter_w)
	AM_RANGE(0x6004, 0x6007) AM_DEVWRITE(GAL_AUDIO, galaxian_lfo_freq_w)
	AM_RANGE(0x6800, 0x6800) AM_READ_PORT("IN1")
	AM_RANGE(0x6800, 0x6802) AM_DEVWRITE(GAL_AUDIO, galaxian_background_enable_w)
//  AM_RANGE(0x6803, 0x6803) AM_WRITE(galaxian_noise_enable_w) /* not hooked up? */
	AM_RANGE(0x6805, 0x6805) AM_DEVWRITE(GAL_AUDIO, galaxian_shoot_enable_w)
	AM_RANGE(0x6806, 0x6807) AM_DEVWRITE(GAL_AUDIO, galaxian_vol_w)
	AM_RANGE(0x7000, 0x7000) AM_READ_PORT("DSW0")
	AM_RANGE(0x7001, 0x7001) AM_WRITE(galaxold_nmi_enable_w)
	AM_RANGE(0x7004, 0x7004) AM_WRITE(galaxold_stars_enable_w)
	AM_RANGE(0x7006, 0x7006) AM_WRITE(galaxold_flip_screen_x_w)
	AM_RANGE(0x7007, 0x7007) AM_WRITE(galaxold_flip_screen_y_w)
	AM_RANGE(0x7800, 0x78ff) AM_READ(watchdog_reset_r)
	AM_RANGE(0x7800, 0x78ff) AM_DEVWRITE(GAL_AUDIO, galaxian_pitch_w)
	AM_RANGE(0x8000, 0x8000) AM_WRITE(_4in1_bank_w)
	AM_RANGE(0xc000, 0xdfff) AM_ROM		/* fixed menu code */
ADDRESS_MAP_END


static ADDRESS_MAP_START( bagmanmc_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x67ff) AM_RAM
	AM_RANGE(0x9000, 0x93ff) AM_RAM_WRITE(galaxold_videoram_w) AM_BASE_MEMBER(galaxold_state, m_videoram)
	AM_RANGE(0x9400, 0x97ff) AM_READ(galaxold_videoram_r)
	AM_RANGE(0x9800, 0x983f) AM_RAM_WRITE(galaxold_attributesram_w) AM_BASE_MEMBER(galaxold_state, m_attributesram)
	AM_RANGE(0x9840, 0x985f) AM_RAM AM_BASE_MEMBER(galaxold_state, m_spriteram) AM_SIZE_MEMBER(galaxold_state, m_spriteram_size)
	AM_RANGE(0x9860, 0x987f) AM_RAM AM_BASE_MEMBER(galaxold_state, m_bulletsram) AM_SIZE_MEMBER(galaxold_state, m_bulletsram_size)
	AM_RANGE(0x9880, 0x98ff) AM_RAM
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("IN0")
	AM_RANGE(0xa003, 0xa003) AM_WRITE(galaxold_coin_counter_w)
	AM_RANGE(0xa800, 0xa800) AM_READ_PORT("IN1")
	AM_RANGE(0xa803, 0xa803) AM_DEVWRITE(GAL_AUDIO, galaxian_noise_enable_w)
	AM_RANGE(0xb000, 0xb000) AM_READ_PORT("DSW")
	AM_RANGE(0xb001, 0xb001) AM_WRITE(galaxold_nmi_enable_w)
	AM_RANGE(0xb006, 0xb006) AM_WRITE(galaxold_flip_screen_x_w)
	AM_RANGE(0xb007, 0xb007) AM_WRITE(galaxold_flip_screen_y_w)
	AM_RANGE(0xb800, 0xb800) AM_READ(watchdog_reset_r) AM_DEVWRITE(GAL_AUDIO, galaxian_pitch_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( dkongjrm_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x6fff) AM_RAM
	AM_RANGE(0x7000, 0x7fff) AM_ROM
	AM_RANGE(0x9000, 0x93ff) AM_RAM_WRITE(galaxold_videoram_w) AM_BASE_MEMBER(galaxold_state, m_videoram)
	AM_RANGE(0x9800, 0x983f) AM_WRITE(galaxold_attributesram_w) AM_BASE_MEMBER(galaxold_state, m_attributesram)
	AM_RANGE(0x9840, 0x987f) AM_WRITEONLY AM_BASE_MEMBER(galaxold_state, m_spriteram) AM_SIZE_MEMBER(galaxold_state, m_spriteram_size)
	AM_RANGE(0x98c0, 0x98ff) AM_WRITEONLY AM_BASE_MEMBER(galaxold_state, m_spriteram2) AM_SIZE_MEMBER(galaxold_state, m_spriteram2_size)
	AM_RANGE(0xa000, 0xa0ff) AM_READ_PORT("IN0")
	AM_RANGE(0xa003, 0xa003) AM_WRITE(galaxold_coin_counter_w)
  //AM_RANGE(0xa004, 0xa007) AM_WRITE(galaxian_lfo_freq_w)
	AM_RANGE(0xa800, 0xa8ff) AM_READ_PORT("IN1")
	AM_RANGE(0xa800, 0xa802) AM_DEVWRITE(GAL_AUDIO, galaxian_background_enable_w)
	AM_RANGE(0xa803, 0xa803) AM_DEVWRITE(GAL_AUDIO, galaxian_noise_enable_w)
  //AM_RANGE(0xa805, 0xa805) AM_WRITE(galaxian)
	AM_RANGE(0xa806, 0xa807) AM_DEVWRITE(GAL_AUDIO, galaxian_vol_w)
	AM_RANGE(0xb000, 0xb0ff) AM_READ_PORT("DSW")
	AM_RANGE(0xb000, 0xb000) AM_WRITE(galaxold_gfxbank_w)
	AM_RANGE(0xb001, 0xb001) AM_WRITE(galaxold_nmi_enable_w)
  //AM_RANGE(0xb004, 0xb004) AM_WRITE(galaxold_stars_enable_w)
	AM_RANGE(0xb006, 0xb006) AM_WRITE(galaxold_flip_screen_x_w)
	AM_RANGE(0xb007, 0xb007) AM_WRITE(galaxold_flip_screen_y_w)
	AM_RANGE(0xb800, 0xb800) AM_READ(watchdog_reset_r) AM_DEVWRITE(GAL_AUDIO, galaxian_pitch_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( tazzmang, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x7000, 0x7000) AM_READ_PORT("DSW0") /* mirror */
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0x883f) AM_RAM_WRITE(galaxold_attributesram_w) AM_BASE_MEMBER(galaxold_state, m_attributesram)
	AM_RANGE(0x8840, 0x885f) AM_RAM AM_BASE_MEMBER(galaxold_state, m_spriteram) AM_SIZE_MEMBER(galaxold_state, m_spriteram_size)
	AM_RANGE(0x8860, 0x887f) AM_RAM AM_BASE_MEMBER(galaxold_state, m_bulletsram) AM_SIZE_MEMBER(galaxold_state, m_bulletsram_size)
	AM_RANGE(0x8880, 0x8bff) AM_WRITENOP
	AM_RANGE(0x9000, 0x93ff) AM_RAM_WRITE(galaxold_videoram_w) AM_BASE_MEMBER(galaxold_state, m_videoram)
	AM_RANGE(0x9800, 0x9800) AM_READ(watchdog_reset_r)
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("IN0")
	AM_RANGE(0xa7ff, 0xa7ff) AM_READ_PORT("IN0") /* mirror */
	AM_RANGE(0xa800, 0xa800) AM_READ_PORT("IN1") AM_DEVWRITE(GAL_AUDIO, galaxian_background_enable_w)
	AM_RANGE(0xa803, 0xa803) AM_WRITE(galaxold_coin_counter_w)
	AM_RANGE(0xa805, 0xa805) AM_DEVWRITE(GAL_AUDIO, galaxian_shoot_enable_w)
	AM_RANGE(0xa806, 0xa807) AM_DEVWRITE(GAL_AUDIO, galaxian_vol_w)
	AM_RANGE(0xb000, 0xb000) AM_READ_PORT("DSW0")
	AM_RANGE(0xb001, 0xb001) AM_WRITE(galaxold_nmi_enable_w)
	AM_RANGE(0xb004, 0xb004) AM_WRITE(galaxold_stars_enable_w)
	AM_RANGE(0xb006, 0xb006) AM_WRITE(galaxold_flip_screen_x_w)
	AM_RANGE(0xb007, 0xb007) AM_WRITE(galaxold_flip_screen_y_w)
	AM_RANGE(0xb800, 0xb800) AM_READ(watchdog_reset_r) AM_DEVWRITE(GAL_AUDIO, galaxian_pitch_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( bongo, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM
	AM_RANGE(0x8400, 0x87ff) AM_WRITENOP // not used
	AM_RANGE(0x9000, 0x93ff) AM_RAM_WRITE(galaxold_videoram_w) AM_BASE_MEMBER(galaxold_state, m_videoram)
	AM_RANGE(0x9400, 0x97ff) AM_WRITENOP // not used
	AM_RANGE(0x9800, 0x983f) AM_RAM_WRITE(galaxold_attributesram_w) AM_BASE_MEMBER(galaxold_state, m_attributesram)
	AM_RANGE(0x9840, 0x985f) AM_RAM AM_BASE_MEMBER(galaxold_state, m_spriteram) AM_SIZE_MEMBER(galaxold_state, m_spriteram_size)
	AM_RANGE(0x9860, 0x987f) AM_RAM AM_BASE_MEMBER(galaxold_state, m_bulletsram) AM_SIZE_MEMBER(galaxold_state, m_bulletsram_size)
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("IN0")
	AM_RANGE(0xa800, 0xa800) AM_READ_PORT("IN1")
	AM_RANGE(0xb000, 0xb000) AM_READ_PORT("DSW0")
	AM_RANGE(0xb001, 0xb001) AM_WRITE(galaxold_nmi_enable_w)
	AM_RANGE(0xb004, 0xb004) AM_WRITE(galaxold_stars_enable_w)
	AM_RANGE(0xb006, 0xb006) AM_WRITE(galaxold_flip_screen_x_w)
	AM_RANGE(0xb007, 0xb007) AM_WRITE(galaxold_flip_screen_y_w)
	AM_RANGE(0xb800, 0xb800) AM_READ(watchdog_reset_r) AM_WRITENOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( bongo_io, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("aysnd", ay8910_address_data_w)
	AM_RANGE(0x02, 0x02) AM_DEVREAD("aysnd", ay8910_r)
ADDRESS_MAP_END


static ADDRESS_MAP_START( ozon1_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0x4000, 0x4200) AM_RAM
	AM_RANGE(0x4300, 0x43ff) AM_RAM
	AM_RANGE(0x4800, 0x4bff) AM_READWRITE(galaxold_videoram_r, galaxold_videoram_w) AM_BASE_MEMBER(galaxold_state, m_videoram)
	AM_RANGE(0x4c00, 0x4fff) AM_WRITE(galaxold_videoram_w)
	AM_RANGE(0x5000, 0x503f) AM_RAM_WRITE(galaxold_attributesram_w) AM_BASE_MEMBER(galaxold_state, m_attributesram)
	AM_RANGE(0x5040, 0x505f) AM_RAM AM_BASE_MEMBER(galaxold_state, m_spriteram) AM_SIZE_MEMBER(galaxold_state, m_spriteram_size)
	AM_RANGE(0x6801, 0x6801) AM_WRITENOP //continuosly 0 and 1
	AM_RANGE(0x6802, 0x6802) AM_WRITE(galaxold_coin_counter_w)
	AM_RANGE(0x6806, 0x6806) AM_WRITE(galaxold_flip_screen_x_w)
	AM_RANGE(0x6807, 0x6807) AM_WRITE(galaxold_flip_screen_y_w)
	AM_RANGE(0x8100, 0x8100) AM_READ_PORT("IN0")
	AM_RANGE(0x8101, 0x8101) AM_READ_PORT("IN1")
	AM_RANGE(0x8102, 0x8102) AM_READ_PORT("IN2")
	AM_RANGE(0x8103, 0x8103) AM_WRITENOP //only one 9b at reset
ADDRESS_MAP_END

static ADDRESS_MAP_START( ozon1_io_map, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("aysnd", ay8910_data_address_w)
ADDRESS_MAP_END


static WRITE8_HANDLER( harem_nmi_mask_w )
{
	galaxold_state *state = space->machine().driver_data<galaxold_state>();

	state->m_nmi_mask = data & 1;
}

static ADDRESS_MAP_START( harem_cpu1, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x27ff) AM_RAM
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x4800, 0x4fff) AM_READWRITE(galaxold_videoram_r, galaxold_videoram_w) AM_BASE_MEMBER(galaxold_state, m_videoram)
	AM_RANGE(0x5000, 0x5000) AM_WRITENOP
	AM_RANGE(0x5800, 0x5800) AM_READNOP AM_WRITE(harem_nmi_mask_w)
	AM_RANGE(0x5801, 0x5807) AM_WRITENOP
	AM_RANGE(0x6101, 0x6101) AM_READ_PORT("IN0")
	AM_RANGE(0x6102, 0x6102) AM_READ_PORT("IN1")
	AM_RANGE(0x6103, 0x6103) AM_WRITENOP
	AM_RANGE(0x6200, 0x6203) AM_WRITENOP AM_BASE_MEMBER(galaxold_state, m_attributesram)
	AM_RANGE(0x8000, 0x9fff) AM_ROM
	AM_RANGE(0xffe6, 0xffff) AM_RAM AM_BASE_MEMBER(galaxold_state, m_spriteram) AM_SIZE_MEMBER(galaxold_state, m_spriteram_size)
ADDRESS_MAP_END

static ADDRESS_MAP_START( harem_cpu2, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM
	AM_RANGE(0xa000, 0xa000) AM_WRITENOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( harem_cpu2_io, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x04, 0x04) AM_DEVWRITE("ay1", ay8910_address_w)
	AM_RANGE(0x08, 0x08) AM_DEVWRITE("ay1", ay8910_data_w)
	AM_RANGE(0x10, 0x10) AM_DEVWRITE("ay2", ay8910_address_w)
	AM_RANGE(0x20, 0x20) AM_DEVWRITE("ay2", ay8910_data_w)
	AM_RANGE(0x40, 0x40) AM_DEVWRITE("ay3", ay8910_address_w)
	AM_RANGE(0x80, 0x80) AM_DEVWRITE("ay3", ay8910_data_w)
	AM_RANGE(0x80, 0x80) AM_READ(soundlatch_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( hunchbkg, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x1480, 0x14bf) AM_MIRROR(0x6000) AM_RAM_WRITE(galaxold_attributesram_w) AM_BASE_MEMBER(galaxold_state, m_attributesram)
	AM_RANGE(0x14c0, 0x14ff) AM_MIRROR(0x6000) AM_WRITEONLY AM_BASE_MEMBER(galaxold_state, m_spriteram) AM_SIZE_MEMBER(galaxold_state, m_spriteram_size)
	AM_RANGE(0x1500, 0x1500) AM_MIRROR(0x6000) AM_READ_PORT("IN0")
	AM_RANGE(0x1500, 0x1501) AM_MIRROR(0x6000) AM_WRITE(galaxold_leds_w)			/* not connected ... */
	AM_RANGE(0x1502, 0x1502) AM_MIRROR(0x6000) AM_WRITE(galaxold_coin_lockout_w)	/* not connected ... */
	AM_RANGE(0x1503, 0x1503) AM_MIRROR(0x6000) AM_WRITE(galaxold_coin_counter_w)
	AM_RANGE(0x1504, 0x1507) AM_MIRROR(0x6000) AM_DEVWRITE(GAL_AUDIO, galaxian_lfo_freq_w)
	AM_RANGE(0x1580, 0x1580) AM_MIRROR(0x6000) AM_READ_PORT("IN1")
	AM_RANGE(0x1580, 0x1587) AM_MIRROR(0x6000) AM_DEVWRITE(GAL_AUDIO, galaxian_sound_w)
	AM_RANGE(0x1583, 0x1583) AM_MIRROR(0x6000) AM_DEVWRITE(GAL_AUDIO, galaxian_noise_enable_w)
	AM_RANGE(0x1585, 0x1585) AM_MIRROR(0x6000) AM_DEVWRITE(GAL_AUDIO, galaxian_shoot_enable_w)
	AM_RANGE(0x1586, 0x1587) AM_MIRROR(0x6000) AM_DEVWRITE(GAL_AUDIO, galaxian_vol_w)
	AM_RANGE(0x1600, 0x1600) AM_MIRROR(0x6000) AM_READ_PORT("DSW0")
	AM_RANGE(0x1601, 0x1601) AM_MIRROR(0x6000) AM_WRITE(galaxold_nmi_enable_w)
	AM_RANGE(0x1604, 0x1604) AM_MIRROR(0x6000) AM_WRITE(galaxold_stars_enable_w)
	AM_RANGE(0x1606, 0x1606) AM_MIRROR(0x6000) AM_WRITE(galaxold_flip_screen_x_w)
	AM_RANGE(0x1607, 0x1607) AM_MIRROR(0x6000) AM_WRITE(galaxold_flip_screen_y_w)
	AM_RANGE(0x1680, 0x1680) AM_MIRROR(0x6000) AM_READ(watchdog_reset_r) AM_DEVWRITE(GAL_AUDIO, galaxian_pitch_w)
	AM_RANGE(0x1800, 0x1bff) AM_MIRROR(0x6000) AM_WRITE(galaxold_videoram_w) AM_BASE_MEMBER(galaxold_state, m_videoram)
	AM_RANGE(0x1c00, 0x1fff) AM_MIRROR(0x6000) AM_RAM
	AM_RANGE(0x2000, 0x2fff) AM_ROM
	AM_RANGE(0x4000, 0x4fff) AM_ROM
	AM_RANGE(0x6000, 0x6fff) AM_ROM
ADDRESS_MAP_END

/* the nmi line seems to be inverted on the cpu plugin board */
READ8_DEVICE_HANDLER( ttl7474_trampoline ) { return downcast<ttl7474_device *>(device)->output_comp_r(); }
static ADDRESS_MAP_START( hunchbkg_io, AS_IO, 8 )
	AM_RANGE(S2650_DATA_PORT,  S2650_DATA_PORT) AM_READNOP // not used
	AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_DEVREAD("7474_9m_1", ttl7474_trampoline)
ADDRESS_MAP_END


static ADDRESS_MAP_START( drivfrcg, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x1480, 0x14bf) AM_MIRROR(0x6000) AM_WRITE(galaxold_attributesram_w) AM_BASE_MEMBER(galaxold_state, m_attributesram)
	AM_RANGE(0x14c0, 0x14ff) AM_MIRROR(0x6000) AM_WRITEONLY AM_BASE_MEMBER(galaxold_state, m_spriteram) AM_SIZE_MEMBER(galaxold_state, m_spriteram_size)
	AM_RANGE(0x1500, 0x1500) AM_MIRROR(0x6000) AM_READ_PORT("IN0")
	AM_RANGE(0x1503, 0x1503) AM_MIRROR(0x6000) AM_WRITE(galaxold_coin_counter_w)
	AM_RANGE(0x1580, 0x1580) AM_MIRROR(0x6000) AM_READ_PORT("IN1")
	AM_RANGE(0x1580, 0x1582) AM_MIRROR(0x6000) AM_DEVWRITE(GAL_AUDIO, galaxian_background_enable_w)
	AM_RANGE(0x1583, 0x1583) AM_MIRROR(0x6000) AM_WRITENOP
	AM_RANGE(0x1585, 0x1585) AM_MIRROR(0x6000) AM_WRITENOP
	AM_RANGE(0x1586, 0x1587) AM_MIRROR(0x6000) AM_DEVWRITE(GAL_AUDIO, galaxian_lfo_freq_w)
	AM_RANGE(0x1600, 0x1600) AM_MIRROR(0x6000) AM_READ_PORT("DSW0") AM_DEVWRITE(GAL_AUDIO, galaxian_pitch_w)
	AM_RANGE(0x1700, 0x1700) AM_MIRROR(0x6000) AM_READ_PORT("DSW1") AM_WRITENOP
	AM_RANGE(0x1701, 0x1701) AM_MIRROR(0x6000) AM_WRITENOP
	AM_RANGE(0x1704, 0x1707) AM_MIRROR(0x6000) AM_DEVWRITE(GAL_AUDIO, galaxian_vol_w)
	AM_RANGE(0x1800, 0x1bff) AM_MIRROR(0x6000) AM_WRITE(galaxold_videoram_w) AM_BASE_MEMBER(galaxold_state, m_videoram)
	AM_RANGE(0x1c00, 0x1fff) AM_MIRROR(0x6000) AM_RAM
	AM_RANGE(0x2000, 0x2fff) AM_ROM
	AM_RANGE(0x4000, 0x4fff) AM_ROM
	AM_RANGE(0x6000, 0x6fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( drivfrcg_io, AS_IO, 8 )
	AM_RANGE(0x00, 0x00) AM_READ(drivfrcg_port0_r)
	AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READ_PORT("SENSE") AM_WRITENOP
ADDRESS_MAP_END


static ADDRESS_MAP_START( racknrol, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x1400, 0x143f) AM_MIRROR(0x6000) AM_RAM_WRITE(galaxold_attributesram_w) AM_BASE_MEMBER(galaxold_state, m_attributesram)
	AM_RANGE(0x1440, 0x14bf) AM_MIRROR(0x6000) AM_RAM AM_BASE_MEMBER(galaxold_state, m_spriteram) AM_SIZE_MEMBER(galaxold_state, m_spriteram_size)
	AM_RANGE(0x14c0, 0x14ff) AM_MIRROR(0x6000) AM_RAM
	AM_RANGE(0x1500, 0x1500) AM_MIRROR(0x6000) AM_READ_PORT("IN0")
	AM_RANGE(0x1580, 0x1580) AM_MIRROR(0x6000) AM_READ_PORT("IN1")
	AM_RANGE(0x1600, 0x1600) AM_MIRROR(0x6000) AM_READ_PORT("DSW0")
	AM_RANGE(0x1600, 0x1601) AM_MIRROR(0x6000) AM_WRITENOP
	AM_RANGE(0x1606, 0x1606) AM_MIRROR(0x6000) AM_WRITE(galaxold_flip_screen_x_w)
	AM_RANGE(0x1607, 0x1607) AM_MIRROR(0x6000) AM_WRITE(galaxold_flip_screen_y_w)
	AM_RANGE(0x1680, 0x1680) AM_MIRROR(0x6000) AM_READNOP
//  AM_RANGE(0x1700, 0x1700) AM_MIRROR(0x6000) AM_READ(trvchlng_question_r)
//  AM_RANGE(0x1701, 0x1703) AM_MIRROR(0x6000) AM_READ(trvchlng_question_w)
	AM_RANGE(0x1800, 0x1bff) AM_MIRROR(0x6000) AM_WRITE(galaxold_videoram_w) AM_BASE_MEMBER(galaxold_state, m_videoram)
	AM_RANGE(0x1c00, 0x1fff) AM_MIRROR(0x6000) AM_RAM
	AM_RANGE(0x2000, 0x2fff) AM_ROM
	AM_RANGE(0x4000, 0x4fff) AM_ROM
	AM_RANGE(0x6000, 0x6fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( racknrol_io, AS_IO, 8 )
	AM_RANGE(0x1d, 0x1d) AM_DEVWRITE("sn1", sn76496_w)
	AM_RANGE(0x1e, 0x1e) AM_DEVWRITE("sn2", sn76496_w)
	AM_RANGE(0x1f, 0x1f) AM_DEVWRITE("sn3", sn76496_w)
	AM_RANGE(0x20, 0x3f) AM_WRITE(racknrol_tiles_bank_w) AM_BASE_MEMBER(galaxold_state, m_racknrol_tiles_bank)
	AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READ_PORT("SENSE")
ADDRESS_MAP_END

static READ8_HANDLER( hexpoola_data_port_r )
{
	switch (cpu_get_pc(&space->device()))
	{
		case 0x0022:
			return 0;

		case 0x0031:
			return 1;
	}

    return 0;
}

static ADDRESS_MAP_START( hexpoola_io, AS_IO, 8 )
	AM_RANGE(0x00, 0x00) AM_READNOP
	AM_RANGE(0x20, 0x3f) AM_WRITE(racknrol_tiles_bank_w) AM_BASE_MEMBER(galaxold_state, m_racknrol_tiles_bank)
	AM_RANGE(S2650_DATA_PORT, S2650_DATA_PORT) AM_READ(hexpoola_data_port_r) AM_DEVWRITE("snsnd", sn76496_w)
	AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READ_PORT("SENSE")
ADDRESS_MAP_END


/* Lives Dips are spread across two input ports */
static CUSTOM_INPUT( vpool_lives_r )
{
	int bit_mask = (FPTR)param;

	switch (bit_mask)
	{
		case 0x40:  /* vpool : IN1 (0xa800) bit 6 */
			return ((input_port_read(field.machine(), "LIVES") & bit_mask) >> 6);
		case 0x01:  /* vpool : DSW (0xb000) bit 0 */
			return ((input_port_read(field.machine(), "LIVES") & bit_mask) >> 0);

		default:
			logerror("vpool_lives_r : invalid %02X bit_mask\n",bit_mask);
			return 0;
	}
}

/* verified from Z80 code */
static INPUT_PORTS_START( vpool )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1 )           /* uses same coinage as COIN1 and COIN2 */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vpool_lives_r, (void *)0x40)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vpool_lives_r, (void *)0x01)
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("LIVES")
	PORT_DIPNAME( 0x41, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x41, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "Infinite (Cheat)" )          /* also gives 99 credits after coin insertion regardless of coinage */
INPUT_PORTS_END


/* verified from Z80 code */
static INPUT_PORTS_START( rockclim )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )             /* only adds 1 credit if "Coin Slots" is set to 1 */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP )    PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN )  PORT_8WAY
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(	0x40, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT )   PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT )  PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP )     PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN )   PORT_8WAY
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPSETTING(    0x40, "50000" )
	PORT_DIPNAME( 0x80, 0x00, "Coin Slots" )
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x00, "2" )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT )   PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP )     PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN )   PORT_8WAY PORT_COCKTAIL

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_8C ) )
INPUT_PORTS_END


/* Coinage Dips are spread across two input ports */
static CUSTOM_INPUT( ckongg_coinage_r )
{
	int bit_mask = (FPTR)param;

	switch (bit_mask)
	{
		case 0x0c:  /* ckongg  : DSW (0xc800) bits 2 and 3 */
			return ((input_port_read(field.machine(), "COINAGE") & bit_mask) >> 2);
		case 0x40:  /* ckongg  : IN1 (0xc400) bit 6 */
			return ((input_port_read(field.machine(), "COINAGE") & bit_mask) >> 6);

		case 0xc0:  /* ckongmc : IN1 (0xa800) bits 6 and 7 */
			return ((input_port_read(field.machine(), "COINAGE") & bit_mask) >> 6);
		case 0x01:  /* ckongmc : DSW (0xb000) bit 0 */
			return ((input_port_read(field.machine(), "COINAGE") & bit_mask) >> 0);

		default:
			logerror("ckongg_coinage_r : invalid %02X bit_mask\n",bit_mask);
			return 0;
	}
}

/* verified from Z80 code */
static INPUT_PORTS_START( ckongg )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(ckongg_coinage_r, (void *)0x40)
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(ckongg_coinage_r, (void *)0x0c)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("COINAGE")
	PORT_DIPNAME( 0x4c, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x4c, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x44, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x48, DEF_STR( 1C_4C ) )
INPUT_PORTS_END

/* verified from Z80 code */
static INPUT_PORTS_START( ckongmc )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_4WAY

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) /* also START1 : code at 0x5064 for BUTTON1 and 0x514d for START1 */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(ckongg_coinage_r, (void *)0xc0)

	PORT_START("DSW")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(ckongg_coinage_r, (void *)0x01)
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("COINAGE")
	PORT_DIPNAME( 0xc1, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0xc1, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x41, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x81, DEF_STR( 1C_4C ) )
INPUT_PORTS_END


/* verified from Z80 code */
static INPUT_PORTS_START( scramblb )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x0c, "255 (Cheat)")
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

/* verified from Z80 code */
static INPUT_PORTS_START( scramb2 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )           /* uses same coinage as COIN1 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "255 (Cheat)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, "A 1/1  B 2/1" )
	PORT_DIPSETTING(    0x02, "A 1/2  B 1/1" )
	PORT_DIPSETTING(    0x04, "A 1/3  B 3/1" )
	PORT_DIPSETTING(    0x06, "A 1/4  B 4/1" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( 4in1 )
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
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(_4in1_fake_port_r, (void *)0x40)	// See fake ports
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(_4in1_fake_port_r, (void *)0x80)	// See fake ports

	PORT_START("DSW0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(_4in1_fake_port_r, (void *)0x01)	// See fake ports
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(_4in1_fake_port_r, (void *)0x02)	// See fake ports
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )			// 2 when continue (Scramble PT2)
	PORT_DIPSETTING(    0x04, "5" )			// 2 when continue (Scramble PT2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(_4in1_fake_port_r, (void *)0x08)	// See fake ports
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(_4in1_fake_port_r, (void *)0x10)	// See fake ports
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(_4in1_fake_port_r, (void *)0x20)	// See fake ports
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("FAKE1")      /* The Ghost Muncher PT3 - FAKE DSW0 (bits 0 to 5) and IN1 (bits 6 and 7) */
	PORT_DIPNAME( 0x03, 0x00, "Bonus Life (GM PT3)" )
	PORT_DIPSETTING(    0x01, "10000" )
	PORT_DIPSETTING(    0x02, "15000" )
	PORT_DIPSETTING(    0x03, "20000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL )	// Lives
//  PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x08, DEF_STR( On ) )
//  PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x10, DEF_STR( On ) )
//  PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Coinage (GM PT3)" )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Free_Play ) )

	PORT_START("FAKE2")      /* Scramble PT2 - FAKE DSW0 (bits 0 to 5) and IN1 (bits 6 and 7) */
//  PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unused ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x01, DEF_STR( On ) )
//  PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unused ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL )	// Lives
	PORT_DIPNAME( 0x08, 0x00, "Allow Continue (S PT2)" )
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )	// Scramble PT2 - Check code at 0x00c2
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )	// Scramble PT2 - Check code at 0x00cc
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Coinage (S PT2)" )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Free_Play ) )

	PORT_START("FAKE3")      /* Galaxian PT5 - FAKE DSW0 (bits 0 to 5) and IN1 (bits 6 and 7) */
	PORT_DIPNAME( 0x03, 0x00, "Bonus Life (G PT5)" )
	PORT_DIPSETTING(    0x01, "4000" )
	PORT_DIPSETTING(    0x02, "5000" )
	PORT_DIPSETTING(    0x03, "7000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL )	// Lives
//  PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x08, DEF_STR( On ) )
//  PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x10, DEF_STR( On ) )
//  PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Coinage (G PT5)" )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x80, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Free_Play ) )

	PORT_START("FAKE4")      /* Galactic Convoy - FAKE DSW0 (bits 0 to 5) and IN1 (bits 6 and 7) */
	PORT_DIPNAME( 0x01, 0x00, "Bonus Life (GC)" )
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPSETTING(    0x01, "80000" )
//  PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unused ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL )	// Lives
//  PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x08, DEF_STR( On ) )
//  PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x10, DEF_STR( On ) )
//  PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Coinage (GC)" )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )    // 1 credit for 1st coin !
	PORT_DIPSETTING(    0xc0, DEF_STR( Free_Play ) )
INPUT_PORTS_END


/* verified from Z80 code */
static INPUT_PORTS_START( bagmanmc )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )           /* stored to 0x6163 bit 4 but not tested */
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(	0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Cocktail ) )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )
	PORT_DIPSETTING(	0x03, "2" )
	PORT_DIPSETTING(	0x02, "3" )
	PORT_DIPSETTING(	0x01, "4" )
	PORT_DIPSETTING(	0x00, "5" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Coinage ) )
	PORT_DIPSETTING(	0x00, "A 2C/1C  B 1C/1C" )
	PORT_DIPSETTING(	0x04, "A 1C/1C  B 1C/2C" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(	0x18, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x10, DEF_STR( Medium ) )
	PORT_DIPSETTING(	0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Language ) )
	PORT_DIPSETTING(	0x20, DEF_STR( English ) )
	PORT_DIPSETTING(	0x00, DEF_STR( French ) )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )                   /* see notes */
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          /* check code at 0x2d78 and 0x2e6b - affect initials entry */
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
INPUT_PORTS_END


/* Coinage Dips are spread across two input ports */
static CUSTOM_INPUT( dkongjrm_coinage_r )
{
	int bit_mask = (FPTR)param;

	switch (bit_mask)
	{
		case 0xc0:  /* dkongjrm : IN1 (0xa8??) bits 6 and 7 */
			return ((input_port_read(field.machine(), "COINAGE") & bit_mask) >> 6);
		case 0x01:  /* dkongjrm : DSW (0xb0??) bit 0 */
			return ((input_port_read(field.machine(), "COINAGE") & bit_mask) >> 0);

		default:
			logerror("dkongjrm_coinage_r : invalid %02X bit_mask\n",bit_mask);
			return 0;
	}
}

/* verified from Z80 code */
static INPUT_PORTS_START( dkongjrm )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_4WAY

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(dkongjrm_coinage_r, (void *)0xc0)

	PORT_START("DSW")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(dkongjrm_coinage_r, (void *)0x01)
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(	0x00, "3" )
	PORT_DIPSETTING(	0x02, "4" )
	PORT_DIPSETTING(	0x04, "5" )
	PORT_DIPSETTING(	0x06, "6" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(	0x08, DEF_STR( Upright ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("COINAGE")
	PORT_DIPNAME( 0xc1, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0xc1, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x41, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x81, DEF_STR( 1C_4C ) )
INPUT_PORTS_END


/* verified from Z80 code */
static INPUT_PORTS_START( porter )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL       /* see notes */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_8WAY

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )                     /* also START1 - see notes */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 )                     /* also START2 - see notes */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL       /* see notes */
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(	0x00, "10000 only" )
	PORT_DIPSETTING(	0x40, "30000 only" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(	0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(	0x00, "A 2C/1C  B 1C/3C" )
	PORT_DIPSETTING(	0x01, "A 1C/1C  B 1C/6C" )
	PORT_DIPUNUSED( 0x02, IP_ACTIVE_LOW )                             /* stored to 0x8021 bit 1 but not tested */
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Lives ) )
	PORT_DIPSETTING(	0x00, "2" )
	PORT_DIPSETTING(	0x04, "3" )
	PORT_DIPSETTING(	0x08, "4" )
	PORT_DIPSETTING(	0x0c, "5" )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


/* verified from Z80 code */
static INPUT_PORTS_START( tazzmang )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_8WAY

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start 1 / P1 and P2 Button 2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(	0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(	0x00, "3" )
	PORT_DIPSETTING(	0x80, "5" )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x06, 0x04, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, "A 4C/1C  B 1C/4C" )
	PORT_DIPSETTING(    0x02, "A 3C/1C  B 1C/3C" )
	PORT_DIPSETTING(    0x06, "A 2C/1C  B 1C/2C" )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


/* verified from Z80 code */
static INPUT_PORTS_START( bongo )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )           /* see notes */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )            /* see notes */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )            /* see notes */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )            /* see notes */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )            /* see notes */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )            /* see notes */

	PORT_START("DSW1")
	PORT_DIPUNUSED( 0x01, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x06, 0x02, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x06, "5" )
	PORT_DIPNAME( 0x08, 0x00, "Infinite Lives (Cheat)" )    /* always gives 3 lives */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_HIGH )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )            /* also 1C_3C for Coin B if it existed */
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )            /* also 1C_6C for Coin B if it existed */
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END


/* verified from Z80 code */
static INPUT_PORTS_START( ozon1 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, "A 1C/1C  B 2C/1C" )
	PORT_DIPSETTING(    0x02, "A 1C/2C  B 1C/1C" )          /* see notes */
	PORT_DIPSETTING(    0x04, "A 1C/3C  B 3C/1C" )
	PORT_DIPSETTING(    0x06, "A 1C/4C  B 4C/1C" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


/* verified from Z80 code */
static INPUT_PORTS_START( ladybugg )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_4WAY

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_4WAY PORT_COCKTAIL
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Free_Play ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( harem )
	PORT_START("IN0")//Change tag when major usage uncovered.
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x80, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x80, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( hunchbkg )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )	/* labeled "NOT USED" in galaxian schematics */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Down") PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Down") PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Start/Red") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Start/Red") PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )	/* labeled "TABLE" in galaxian schematics */
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x40, "A 2C/1C  B 1C/3C" )
	PORT_DIPSETTING(    0x00, "A 1C/1C  B 1C/5C" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x02, "20000" )
	PORT_DIPSETTING(    0x04, "40000" )
	PORT_DIPSETTING(    0x06, "80000" )
	PORT_DIPUNUSED( 0x08, 0x00 )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( drivfrcg )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x40, "A 2C/1C  B 1C/3C" )
	PORT_DIPSETTING(    0x00, "A 1C/1C  B 1C/5C" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW0")
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

	PORT_START("DSW1")
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

	PORT_START("SENSE")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )
INPUT_PORTS_END


static INPUT_PORTS_START( racknrol )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
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

	PORT_START("DSW0")
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

	PORT_START("SENSE")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_VBLANK )
INPUT_PORTS_END


static INPUT_PORTS_START( trvchlng )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON4)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
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

	PORT_START("DSW0")
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

	PORT_START("SENSE")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_VBLANK )
INPUT_PORTS_END


static const gfx_layout galaxold_charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};
static const gfx_layout galaxold_spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};

static const gfx_layout pacmanbl_spritelayout =
{
	16,16,
	64,
	2,
	{ 0, 64*16*16 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};

static const gfx_layout bagmanmc_charlayout =
{
	8,8,
	512,
	2,
	{ 0, 512*8*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout _4in1_charlayout =
{
	8,8,
	1024,
	2,
	{ 0, 1024*8*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};
static const gfx_layout _4in1_spritelayout =
{
	16,16,
	256,
	2,
	{ 0, 256*16*16 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};

static const gfx_layout rockclim_charlayout =
{
	8,8,
	256,
	4,//?
	{ 4, 0,4096*8+4,4096*8 },
	{ 3, 2, 1, 0,11 ,10, 9, 8 },
	{ 0*8*2, 1*8*2, 2*8*2, 3*8*2, 4*8*2, 5*8*2, 6*8*2, 7*8*2 },
	8*8*2
};

static GFXDECODE_START( rockclim )
	GFXDECODE_ENTRY( "gfx1", 0x0000, galaxold_charlayout,   32, 8 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, galaxold_spritelayout, 32, 8 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, rockclim_charlayout, 0, 1 )
GFXDECODE_END




static GFXDECODE_START( galaxian )
	GFXDECODE_ENTRY( "gfx1", 0x0000, galaxold_charlayout,   0, 8 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, galaxold_spritelayout, 0, 8 )
GFXDECODE_END

static GFXDECODE_START( gmgalax )
	GFXDECODE_ENTRY( "gfx1", 0x0000, galaxold_charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, galaxold_spritelayout, 0, 16 )
GFXDECODE_END

/* separate character and sprite ROMs */
static GFXDECODE_START( bagmanmc )
	GFXDECODE_ENTRY( "gfx1", 0x0000, bagmanmc_charlayout,    0, 8 )
	GFXDECODE_ENTRY( "gfx1", 0x2000, pacmanbl_spritelayout, 0, 8 )
GFXDECODE_END

static GFXDECODE_START( _4in1 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, _4in1_charlayout,      0, 8 )
	GFXDECODE_ENTRY( "gfx1", 0x4000, _4in1_spritelayout,    0, 8 )
GFXDECODE_END

static const ay8910_interface bongo_ay8910_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("DSW1"),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


static MACHINE_CONFIG_START( galaxold_base, galaxold_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, PIXEL_CLOCK/2)	/* 3.072 MHz */
	MCFG_CPU_PROGRAM_MAP(galaxold_map)

	MCFG_MACHINE_RESET(galaxold)

	MCFG_7474_ADD("7474_9m_1", "7474_9m_1", galaxold_7474_9m_1_callback, NULL)
	MCFG_7474_ADD("7474_9m_2", "7474_9m_1", NULL, galaxold_7474_9m_2_q_callback)

	MCFG_TIMER_ADD("int_timer", galaxold_interrupt_timer)

	/* video hardware */
	MCFG_GFXDECODE(galaxian)
	MCFG_PALETTE_LENGTH(32+2+64)		/* 32 for the characters, 2 for the bullets, 64 for the stars */

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)
	MCFG_SCREEN_UPDATE(galaxold)

	MCFG_PALETTE_INIT(galaxold)
	MCFG_VIDEO_START(galaxold)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( galaxian, galaxold_base )

	/* basic machine hardware */

	/* sound hardware */
	MCFG_FRAGMENT_ADD(galaxian_audio)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( batman2, galaxian )

	/* basic machine hardware */

	/* video hardware */
	MCFG_VIDEO_START(batman2)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( mooncrst, galaxian )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(mooncrst_map)

	/* video hardware */
	MCFG_VIDEO_START(mooncrst)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( scramblb, galaxian )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(scramblb_map)

	/* video hardware */
	MCFG_PALETTE_LENGTH(32+2+64+1)	/* 32 for the characters, 2 for the bullets, 64 for the stars, 1 for background */

	MCFG_PALETTE_INIT(scrambold)
	MCFG_VIDEO_START(scrambold)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( scramb2, galaxian )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(scramb2_map)

	/* video hardware */
	MCFG_PALETTE_LENGTH(32+2+64+1)	/* 32 for the characters, 2 for the bullets, 64 for the stars, 1 for background */

	MCFG_PALETTE_INIT(scrambold)
	MCFG_VIDEO_START(scrambold)
MACHINE_CONFIG_END



static MACHINE_CONFIG_DERIVED( 4in1, galaxian )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(_4in1_map)

	/* video hardware */
	MCFG_GFXDECODE(_4in1)

	MCFG_VIDEO_START(pisces)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( bagmanmc, galaxian )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(bagmanmc_map)

	MCFG_MACHINE_RESET( devilfsg )

	/* video hardware */
	MCFG_GFXDECODE(bagmanmc)

	MCFG_VIDEO_START(pisces)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( dkongjrm, galaxian )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(dkongjrm_map)

	/* video hardware */
	MCFG_VIDEO_START(dkongjrm)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( rockclim, galaxian )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(rockclim_map)
	MCFG_GFXDECODE(rockclim)
	/* video hardware */
	MCFG_VIDEO_START(rockclim)
	MCFG_PALETTE_LENGTH(64+64+2)	/* 64 colors only, but still uses bullets so we need to keep the palette big */
	MCFG_PALETTE_INIT(rockclim)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_SIZE(64*8, 32*8)

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ozon1, galaxold_base )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(ozon1_map)
	MCFG_CPU_IO_MAP(ozon1_io_map)
	MCFG_CPU_VBLANK_INT("screen", nmi_line_pulse)

	MCFG_MACHINE_RESET(0)

	MCFG_PALETTE_INIT(rockclim)
	MCFG_PALETTE_LENGTH(32)

	MCFG_VIDEO_START(galaxold_plain)
	MCFG_SOUND_ADD("aysnd", AY8910, PIXEL_CLOCK/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( drivfrcg, galaxold_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", S2650, MASTER_CLOCK/6)
	MCFG_CPU_PROGRAM_MAP(drivfrcg)
	MCFG_CPU_IO_MAP(drivfrcg_io)
	MCFG_CPU_VBLANK_INT("screen", hunchbks_vh_interrupt)


	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(16000.0/132/2)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE(galaxold)

	MCFG_PALETTE_LENGTH(64)
	MCFG_GFXDECODE(gmgalax)

	MCFG_PALETTE_INIT(rockclim)

	MCFG_VIDEO_START(drivfrcg)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_FRAGMENT_ADD(galaxian_audio)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( bongo, galaxold_base )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(bongo)
	MCFG_CPU_IO_MAP(bongo_io)

	MCFG_VIDEO_START(bongo)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE(galaxold)

	MCFG_SOUND_ADD("aysnd", AY8910, PIXEL_CLOCK/4)
	MCFG_SOUND_CONFIG(bongo_ay8910_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( hunchbkg, galaxold_base )

	/* basic machine hardware */
	MCFG_CPU_REPLACE("maincpu", S2650, PIXEL_CLOCK / 4)

	MCFG_CPU_PROGRAM_MAP(hunchbkg)
	MCFG_CPU_IO_MAP(hunchbkg_io)

	MCFG_MACHINE_RESET(hunchbkg)

	MCFG_FRAGMENT_ADD(galaxian_audio)
MACHINE_CONFIG_END

static INTERRUPT_GEN( vblank_irq )
{
	galaxold_state *state = device->machine().driver_data<galaxold_state>();

	if(state->m_nmi_mask)
		device_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_CONFIG_DERIVED( harem, galaxold_base )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(harem_cpu1)
	MCFG_CPU_VBLANK_INT("screen", vblank_irq)

	MCFG_CPU_ADD("audiocpu", Z80, 1620000) //?
	MCFG_CPU_PROGRAM_MAP(harem_cpu2)
	MCFG_CPU_IO_MAP(harem_cpu2_io)

	MCFG_MACHINE_RESET(0)

	MCFG_PALETTE_INIT(rockclim)
	MCFG_PALETTE_LENGTH(32)

	MCFG_VIDEO_START(galaxold_plain)

	MCFG_SOUND_ADD("ay1", AY8910, 2000000) //?
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.33/3)

	MCFG_SOUND_ADD("ay2", AY8910, 2000000) //?
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.33/3)

	MCFG_SOUND_ADD("ay3", AY8910, 2000000) //?
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.33/3)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( tazzmang, galaxian )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(tazzmang)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( racknrol, galaxold_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", S2650, PIXEL_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(racknrol)
	MCFG_CPU_IO_MAP(racknrol_io)
	MCFG_CPU_VBLANK_INT("screen", hunchbks_vh_interrupt)

	MCFG_GFXDECODE(galaxian)
	MCFG_PALETTE_LENGTH(32)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)
	MCFG_SCREEN_UPDATE(galaxold)

	MCFG_PALETTE_INIT(rockclim)
	MCFG_VIDEO_START(racknrol)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("sn1", SN76496, PIXEL_CLOCK/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("sn2", SN76496, PIXEL_CLOCK/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("sn3", SN76496, PIXEL_CLOCK/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ckongg, galaxian )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(ckongg_map)

	MCFG_GFXDECODE(gmgalax)

	MCFG_VIDEO_START(ckongs)

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ckongmc, galaxian )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(ckongmc_map)

	MCFG_GFXDECODE(gmgalax)

	MCFG_VIDEO_START(ckongs)

MACHINE_CONFIG_END


static MACHINE_CONFIG_START( hexpoola, galaxold_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", S2650, PIXEL_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(racknrol)
	MCFG_CPU_IO_MAP(hexpoola_io)
	MCFG_CPU_VBLANK_INT("screen", hunchbks_vh_interrupt)

	MCFG_GFXDECODE(galaxian)
	MCFG_PALETTE_LENGTH(32)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)
	MCFG_SCREEN_UPDATE(galaxold)

	MCFG_PALETTE_INIT(rockclim)
	MCFG_VIDEO_START(racknrol)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("snsnd", SN76496, PIXEL_CLOCK/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( vpool )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vidpool1.bin", 0x0000, 0x0800, CRC(333f4732) SHA1(b57460c039c69137645bd4280ad877aa789277d6) )
	ROM_CONTINUE(             0x2000, 0x0800 )
	ROM_LOAD( "vidpool2.bin", 0x0800, 0x0800, CRC(eea6c0f1) SHA1(5b18caa78e246f55fd9cd778d6e83f79f0b3f157) )
	ROM_CONTINUE(             0x2800, 0x0800 )
	ROM_LOAD( "vidpool3.bin", 0x1000, 0x0800, CRC(309972a6) SHA1(8269d2f677f55dda71d6a7b0796d2d53a4def59d) )
	ROM_CONTINUE(             0x3000, 0x0800 )
	ROM_LOAD( "vidpool4.bin", 0x1800, 0x0800, CRC(c4f71c1d) SHA1(e1d01135d5ccc1a53308ce89dc2a8fc0992207d5) )
	ROM_CONTINUE(             0x3800, 0x0800 )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "hustler.5f", 0x0000, 0x0800, CRC(0bdfad0e) SHA1(8e6f1737604f3801c03fa2e9a5e6a2778b54bae8) ) // vidpoolh.bin
	ROM_LOAD( "hustler.5h", 0x0800, 0x0800, CRC(8e062177) SHA1(7e52a1669804b6c2f694cfc64b04abc8246bb0c2) ) // vidpoolk.bin

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "hustler.clr",  0x0000, 0x0020, CRC(aa1f7f5e) SHA1(311dd17aa11490a1173c76223e4ccccf8ea29850) )
ROM_END

ROM_START( rockclim )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lc01.a1",   0x0000, 0x1000, CRC(8601ae8d) SHA1(6e0c3b34ce5e6879ce7a116c5c2660889a68320d) )
	ROM_LOAD( "lc02.a2",   0x1000, 0x1000, CRC(2dde9d4c) SHA1(7e343113116b94894558819a7f77f77e4e952da7) )
	ROM_LOAD( "lc03.a3",   0x2000, 0x1000, CRC(82c48a67) SHA1(abf95062eb5c9bd4bb3c9b9af59396a4ca6905d8) )
	ROM_LOAD( "lc04.a4",   0x3000, 0x1000, CRC(7cd3a04a) SHA1(756c12288e120e6f761b266b91920d17cab6926c) )
	ROM_LOAD( "lc05.a5",   0x6000, 0x1000, CRC(5e542149) SHA1(425a5a8769c3fa0887db8ff04e2a4f32f18d2679) )
	ROM_LOAD( "lc06.a6",   0x7000, 0x1000, CRC(b2bdca64) SHA1(e72e63725164c922816dda90ac964a94062eab1b) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "lc08.a9",   0x0000, 0x800, CRC(7f18e1ef) SHA1(2a160b994708ec0f06774dde3ec613af7e3f32c6) )
	ROM_LOAD( "lc07.a7",   0x0800, 0x800, CRC(f18b50ac) SHA1(a2328eb55882a09403cae1a497c611b494649cac) )
	ROM_LOAD( "lc10.c9",   0x1000, 0x800, CRC(dec5781b) SHA1(b6277fc890d153db24bd48293780cf239a6aa0e7) )
	ROM_LOAD( "lc09.c7",   0x1800, 0x800, CRC(06c0b5de) SHA1(561cf99a6be03205c7bc5fd15d4d51ee4d6d164b) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "lc13.g5",   0x0000, 0x1000, CRC(19475f2b) SHA1(5d42aa45a7b519dacdecd3d2edbfee6971693034) )
	ROM_LOAD( "lc14.g7",   0x1000, 0x1000, CRC(cc96d1db) SHA1(9713b47b723a5d8837f2a8e8c43e46dc41a62e5b) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "lc12.e9",  0x0000, 0x0020, CRC(f6e76547) SHA1(c9ea78d1876156561b3bbf327d7e0299e1d9fd4a) )
	ROM_LOAD( "lc11.f4",  0x0020, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

/*
Crazy Kong
Bootleg, 1982

PCB Layout
----------

|----------------------------------------------|
|        AY3-8910    MB7051          2125 2125 |
| LM3900     ROM.5S  MB7051          2125 2125 |
|                    MB7051          2125 2125 |
|            ROM.5R                            |
|                                              |
|1           ROM.5N                            |
|8                                             |
|W           ROM.5M               ROM.11N      |
|A                                ROM.11L      |
|Y           ROM.5K          2114 ROM.11K      |
|   VOL                      2114 ROM.11H      |
|            ROM.5H 2114                       |
|  Z80              2114                       |
|            ROM.5F                            |
|     2114                                     |
|     2114   ROM.5D              5101 ROM.11C  |
|                                              |
|HA1368 DSW(8) 6116  18.432MHz   5101 ROM.11A  |
|----------------------------------------------|
Notes:
      Z80     : Clock running at 3.072MHz (18.432/6)
      AY3-8910: Clock running at 1.536MHz (18.432/12)
      2125    : 1K x1 SRAM (DIP16)
      2114    : 1K x4 SRAM (DIP18)
      6116    : 2K x8 SRAM (DIP24)
      5101    : 256 x4 SRAM (SDIP22)
      LM3900  : National Semiconductor LM3900 Quadruple Norton Operational Amplifier (DIP14)
      HA1368  : Hitachi HA1368 18V, 4.5A, 5.3W Audio Power Amplifier IC
      MB7051  : Hitachi MB7051 32bytes x8 Bipolar PROM (DIP16)
*/

ROM_START( ckongg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "g_ck1.bin",     0x2400, 0x0400, CRC(a4323b94) SHA1(1fed47e1df5efa8f40585bedab07b60067edc2bb) )
	ROM_CONTINUE(              0x1C00, 0x0400)
	ROM_CONTINUE(              0x4800, 0x0400)
	ROM_CONTINUE(              0x0C00, 0x0400)
	ROM_LOAD( "ck2.bin",       0x4400, 0x0400, CRC(1e532996) SHA1(fe1feeca347fccd266925614a46c98cff683f5d3) )
	ROM_CONTINUE(              0x0000, 0x0400)
	ROM_CONTINUE(              0x1800, 0x0400)
	ROM_CONTINUE(              0x2800, 0x0400)
	ROM_LOAD( "ck3.bin",       0x3400, 0x0400, CRC(65157cde) SHA1(572b9bd56894600e21220356d0bf193c7920672c) )
	ROM_CONTINUE(              0x4c00, 0x0400)
	ROM_CONTINUE(              0x5000, 0x0400)
	ROM_CONTINUE(              0x0400, 0x0400)
	ROM_LOAD( "ck4.bin",       0x2000, 0x0400, CRC(43827bc6) SHA1(a2ca9afff0dd1bdcfc3a6ead9ff30b7c91caa7ea) )
	ROM_CONTINUE(              0x3800, 0x0400)
	ROM_CONTINUE(              0x1000, 0x0400)
	ROM_CONTINUE(              0x4000, 0x0400)
	ROM_LOAD( "ck5.bin",       0x0800, 0x0400, CRC(a74ed96e) SHA1(1e845d693a728fea9d52953b5493ec98fdec63e3) )
	ROM_CONTINUE(              0x5400, 0x0400)  // fill
	ROM_CONTINUE(              0x2c00, 0x0400)
	ROM_CONTINUE(              0x1400, 0x0400)
	ROM_LOAD( "ck7.bin",       0x3000, 0x0400, CRC(2c4d8129) SHA1(ab1708ff72ee027106fe8da0caea03a796b3212b) )
	ROM_CONTINUE(              0x3c00, 0x0400)

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "ckvid10.bin",   0x0000, 0x1000, CRC(7866d2cb) SHA1(62dd8b80bc0459c7337d8a8cb83e53b999e7f4a9) )
	ROM_LOAD( "ckvid7.bin",    0x1000, 0x1000, CRC(7311a101) SHA1(49d54c8b94cae4ba81d7a7684eaa4e87815bb4da) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "ck_cp.bin",     0x0000, 0x0020, CRC(7e0b79cb) SHA1(72ef3eb5f09e10c13dcf6fd568a6d16658055a16) )
ROM_END

ROM_START( ckongmc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "kc1.bin",       0x0000, 0x0800, CRC(a87fc828) SHA1(f66b72427d8cdfabdf2274e22bdb10018ac7d2f9) )
	ROM_CONTINUE( 0x2000, 0x0800)
	ROM_LOAD( "kc2.bin",       0x0800, 0x0800, CRC(94a13dec) SHA1(d3bfd5a266bb1f0e66d847e15b51bdd4c9a15e37) )
	ROM_CONTINUE( 0x2800, 0x0800)
	ROM_LOAD( "kc3.bin",       0x1000, 0x0800, CRC(5efc6705) SHA1(9af59a9cb58599b1c7ce0a063929531f6c73b912) )
	ROM_CONTINUE( 0x3000, 0x0800)
	ROM_LOAD( "kc4.bin",       0x1800, 0x0800, CRC(ac917d66) SHA1(63a0db01bb93e052fec64fa69ebcbae3b0b8aa04) )
	ROM_CONTINUE( 0x3800, 0x0800)
	ROM_LOAD( "kc5.bin",       0x4000, 0x0800, CRC(5a9ee1ed) SHA1(1bc420a42a4931c389b4f8db451de7c59786dfbc) )
	ROM_LOAD( "kc6.bin",       0x4800, 0x0800, CRC(f787431e) SHA1(5cee497b8f4072509920d982470cbe06bd18f88b) )
	ROM_LOAD( "kc7.bin",       0x5000, 0x0800, CRC(7a185e31) SHA1(a257f32958af6b2c1c9007b46bd1dc984670b0d9) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "kc8carat.bin",   0x0000, 0x1000, CRC(7866d2cb) SHA1(62dd8b80bc0459c7337d8a8cb83e53b999e7f4a9) )
	ROM_LOAD( "kc9carat.bin",   0x1000, 0x1000, CRC(7311a101) SHA1(49d54c8b94cae4ba81d7a7684eaa4e87815bb4da) )

	ROM_REGION( 0x0020, "proms", 0 ) // not in this set
	ROM_LOAD( "ck_cp.bin",     0x0000, 0x0020, CRC(7e0b79cb) SHA1(72ef3eb5f09e10c13dcf6fd568a6d16658055a16) )
ROM_END

ROM_START( scramblb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "scramble.1k",  0x0000, 0x0800, CRC(9e025c4a) SHA1(a8cc9391bdd01a5a2fe7f0c4e889b4e2495df891) )
	ROM_LOAD( "scramble.2k",  0x0800, 0x0800, CRC(306f783e) SHA1(92d19f90f1123cd211706294d668ab23c8b0760b) )
	ROM_LOAD( "scramble.3k",  0x1000, 0x0800, CRC(0500b701) SHA1(54c84ccad2aae34f42fdddcfcd92cd9da2cd7119) )
	ROM_LOAD( "scramble.4k",  0x1800, 0x0800, CRC(dd380a22) SHA1(125e713a58cc5f2c1e38f67dad29f8c985ce5a8b) )
	ROM_LOAD( "scramble.5k",  0x2000, 0x0800, CRC(df0b9648) SHA1(4ae9150c9441897d5ab7c5a0b3f10e1e8c8e2f6c) )
	ROM_LOAD( "scramble.1j",  0x2800, 0x0800, CRC(b8c07b3c) SHA1(33eaedef4b7f49eeef072425541c17206d0a00ec) )
	ROM_LOAD( "scramble.2j",  0x3000, 0x0800, CRC(88ac07a0) SHA1(c57061db5984b472039356bf84a050b5b66e3813) )
	ROM_LOAD( "scramble.3j",  0x3800, 0x0800, CRC(c67d57ca) SHA1(ba8b14289aef47d48d9750cf2ef3c368e74a60e8) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5f.k",         0x0000, 0x0800, CRC(4708845b) SHA1(a8b1ad19a95a9d35050a2ab7194cc96fc5afcdc9) )
	ROM_LOAD( "5h.k",         0x0800, 0x0800, CRC(11fd2887) SHA1(69844e48bb4d372cac7ae83c953df573c7ecbb7f) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.6e",    0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )
ROM_END

ROM_START( scramb2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "r1.7f1",  0x0000, 0x0800, CRC(4a43148c) SHA1(ea27fd3acf661101296a58a7a50fb8e4d5292760) )
	ROM_LOAD( "r1.7f2",  0x0800, 0x0800, CRC(215a3b86) SHA1(bfddfea9f74064123629d89556240c7a59f7bea2) )
	ROM_LOAD( "r2.7h1",  0x1000, 0x0800, CRC(28779444) SHA1(0abd3a89c8cdd5af2ac06afd38bcd2dcd6010bee) )
	ROM_LOAD( "r2.7h2",  0x1800, 0x0800, CRC(5b4b300b) SHA1(6d69dbdab66bc8f4a16c3d9d3b4581799e4bbfab) )
	ROM_LOAD( "r3.7k1",  0x2000, 0x0800, CRC(b478aa53) SHA1(68cf134482092534ef0a3ceee3aa842f86660065) )
	ROM_LOAD( "r3.7k2",  0x2800, 0x0800, CRC(c33f072e) SHA1(28d61e35f3d5c971e070d7e0cc20b831fe8d52c5) )
	ROM_LOAD( "r4.7l1",  0x3000, 0x0800, CRC(88ac07a0) SHA1(c57061db5984b472039356bf84a050b5b66e3813) )
	ROM_LOAD( "r4.7l2",  0x3800, 0x0800, CRC(321fd003) SHA1(61f33c2709913da4cb20f311501df707d755917e) )
	/* Also exists in the following Rom config */
//  ROM_LOAD( "r1.7f",  0x0000, 0x1000, CRC(75208a74) SHA1(e77afe4b906d08d6763f31dd70d7cb772be97102) )
//  ROM_LOAD( "r2.7h",  0x1000, 0x1000, CRC(f2179cf5) SHA1(5c38aa9bd1d5ebdccf16d2e50acc56f0b3f042d0) )
//  ROM_LOAD( "r3.7k",  0x2000, 0x1000, CRC(941c804e) SHA1(f1eedf719a234cf98071e6a46120765e231f0730) )
//  ROM_LOAD( "r4.7l",  0x3000, 0x1000, CRC(f1506edc) SHA1(66689bb3d7570848e4d020a5f44d6de03b4bff99) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "r6.1j",         0x0000, 0x0800, CRC(4708845b) SHA1(a8b1ad19a95a9d35050a2ab7194cc96fc5afcdc9) )
	ROM_LOAD( "r5.1l",         0x0800, 0x0800, CRC(11fd2887) SHA1(69844e48bb4d372cac7ae83c953df573c7ecbb7f) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.6e",    0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )
ROM_END

ROM_START( 4in1 )
	ROM_REGION( 0x20000, "maincpu", 0 )   /* 64k for code  64k for banked code, encrypted */
	/* Menu Code, Fixed at 0xc000 - 0xdfff */
	ROM_LOAD( "rom1a",        0xc000, 0x1000, CRC(ce1af4d9) SHA1(260d81cb703ab33fa5f282454214dea06e59a5d6) )
	ROM_LOAD( "rom1b",        0xd000, 0x1000, CRC(18484f9b) SHA1(2439841ba5882c287bd9656fbf79190ff9efe4ee) )
	/* Ghost Muncher PT3 - banked at 0x0000 - 0x3fff */
	ROM_LOAD( "rom1c",       0x10000, 0x1000, CRC(83248a8b) SHA1(65af22b9a4516ab52c3327cb3b714d90c2c77284) )
	ROM_LOAD( "rom1d",       0x11000, 0x1000, CRC(053f6da0) SHA1(fa69de09a2162dfaa82ea566f0808433f26e4854) )
	ROM_LOAD( "rom1e",       0x12000, 0x1000, CRC(43c546f3) SHA1(c32a2281f8dca1f2b218dc76192d8e09f2eee460) )
	ROM_LOAD( "rom1f",       0x13000, 0x1000, CRC(3a086b46) SHA1(1fd65fd139a650a5c246cead5141b81764faf98c) )
	/* Scramble PT2 - banked at 0x0000 - 0x3fff */
	ROM_LOAD( "rom1g",       0x14000, 0x1000, CRC(ac0e2050) SHA1(02961a41f54d55f2ae07a2694a14fb6e6e4a766b) )
	ROM_LOAD( "rom1h",       0x15000, 0x1000, CRC(dc11a513) SHA1(2785c08d890f2f8e86b7f793f7989d7605570cc3) )
	ROM_LOAD( "rom1i",       0x16000, 0x1000, CRC(a5fb6be4) SHA1(f575ca70037134084aff152fcee7fdd0a1163c33) )
	ROM_LOAD( "rom1j",       0x17000, 0x1000, CRC(9054cfbe) SHA1(99ad74491cf8682daf45f2786e0bf275160c9826) )
	/* Galaxian PT5 - banked at 0x0000 - 0x3fff */
	ROM_LOAD( "rom2c",       0x18000, 0x1000, CRC(7cd98e11) SHA1(7ef49866a5c5fd871acf5bfe3d899a9ae0d37405) )
	ROM_LOAD( "rom2d",       0x19000, 0x1000, CRC(9402f32e) SHA1(feb5cb09ea719612a22949f34fb97e172305c7b0) )
	ROM_LOAD( "rom2e",       0x1a000, 0x1000, CRC(468e81df) SHA1(4ac30c170ce63637c77227833cef8839e2b0b8ab) )
	/* Galactic Convoy - banked at 0x0000 - 0x3fff */
	ROM_LOAD( "rom2g",       0x1c000, 0x1000, CRC(b1ce3976) SHA1(365e643948e982126198714bb1e07340ded7d4a5) )
	ROM_LOAD( "rom2h",       0x1d000, 0x1000, CRC(7eab5670) SHA1(d9648fc314bc6a685536c6acb17b17737813d902) )
	ROM_LOAD( "rom2i",       0x1e000, 0x1000, CRC(44565ac5) SHA1(cc8141cbdb9280a15b40761448e00a3b30a94ec7) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	/* Ghost Muncher PT3 GFX */
	ROM_LOAD( "rom4b",        0x4000, 0x0800, CRC(7e6495af) SHA1(32db70bca5c60eea6b37a943e076bc5a8dc3870b) )
	ROM_CONTINUE(             0x0000, 0x0800 )
	ROM_LOAD( "rom3b",        0x6000, 0x0800, CRC(7475f72f) SHA1(834873b6a587760cbbd0ac9435af55f6cb20097a) )
	ROM_CONTINUE(             0x2000, 0x0800 )
	/* Scramble PT2 GFX */
	ROM_LOAD( "rom4c",        0x4800, 0x0800, CRC(3355d46d) SHA1(e5476d2053298958f141e11a97017ea465621d89) )
	ROM_RELOAD(               0x0800, 0x0800)
	ROM_LOAD( "rom3c",        0x6800, 0x0800, CRC(ac755a25) SHA1(70af05d32554682be6c3f74936e57b4050d283c7) )
	ROM_RELOAD(               0x2800, 0x0800)
	/* Galaxians PT5 GFX */
	ROM_LOAD( "rom4d",        0x5000, 0x0800, CRC(bbdddb65) SHA1(fc2dcfd969b1ee51a6413117e83f8a0c29278658) )
	ROM_CONTINUE(             0x1000, 0x0800)
	ROM_LOAD( "rom3d",        0x7000, 0x0800, CRC(91a00204) SHA1(eea8a8bd8439260dde9131693e9b53b0238ce7a7) )
	ROM_CONTINUE(             0x3000, 0x0800)
	/* Galactic Convoy GFX */
	ROM_LOAD( "rom4e",        0x5800, 0x0800, CRC(0cb9e297) SHA1(a9be2951851deed0ffefb980fc7751a399dc131e) )
	ROM_CONTINUE(             0x1800, 0x0800 )
	ROM_LOAD( "rom3e",        0x7800, 0x0800, CRC(a1fe77f9) SHA1(dc7972b7aa77fb4f95d7349d4cd7fc4674f9032d) )
	ROM_CONTINUE(             0x3800, 0x0800 )

	ROM_REGION( 0x0020, "proms", 0 )
    ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( bagmanmc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b1.bin",       0x0000, 0x1000, CRC(b74c75ee) SHA1(620083c30136e24a37b79eb4647d99b997107693) )
	ROM_LOAD( "b2.bin",       0x1000, 0x1000, CRC(a7d99916) SHA1(13185e8ff6de92ad5135895e5a7fc8b956f009d3) )
	ROM_LOAD( "b3.bin",       0x2000, 0x1000, CRC(c78f5360) SHA1(7ce9e94c33f1b8e60cc12a3df5f9555f1ca6130f) )
	ROM_LOAD( "b4.bin",       0x3000, 0x1000, CRC(eebd3bd1) SHA1(03200383e87b0759f607888d9b290a0a777b597e) )
	ROM_LOAD( "b5.bin",       0x4000, 0x1000, CRC(0fe24b8c) SHA1(205a36fd346d49d2dda6911198295e202caae81f) )
	ROM_LOAD( "b6.bin",       0x5000, 0x1000, CRC(f50390e7) SHA1(b4ebe647458c26e52461750d63856aea4262f110) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "g1-u.bin",     0x0000, 0x0800, CRC(b63cfae4) SHA1(3e0cb3dbeec8ad790bc482176ca599721bac31ee) )
	ROM_CONTINUE(             0x2000, 0x0800 )
	ROM_LOAD( "g2-u.bin",     0x1000, 0x0800, CRC(a2790089) SHA1(7eb8634f26f6af52fb79bf90ec90b4e258c7c79f) )
	ROM_CONTINUE(             0x2800, 0x0800 )
	ROM_LOAD( "g1-l.bin",     0x0800, 0x0800, CRC(2ae6b5ab) SHA1(59bdebf75d28a247293440ec2ad83eaf30e3de00) )
	ROM_LOAD( "g2-l.bin",     0x1800, 0x0800, CRC(98b37397) SHA1(29914435a10cebbbce04382c45e13a64a0cd18cb) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "bagmanmc.clr", 0x0000, 0x0020, NO_DUMP )	// missing
ROM_END

ROM_START( dkongjrm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a1",           0x0000, 0x1000, CRC(299486e9) SHA1(cc4143ff8cb7a37c151bebab007a932381ae733b) )
	ROM_LOAD( "a2",           0x1000, 0x1000, CRC(a74a193b) SHA1(46f208293c0944b468550738d1238de9b672f403) )
	ROM_LOAD( "b2",           0x2000, 0x1000, CRC(7bc4f236) SHA1(84e7f5fcbea7d047f2a9a9006ae3ed646417c5e0) )
	ROM_LOAD( "c1",           0x3000, 0x1000, CRC(0f594c21) SHA1(eb15bd9cc37794786e2ad24753172e88aa7c4f98) )
	ROM_LOAD( "d1",           0x4000, 0x1000, CRC(cf7d7296) SHA1(9a817eca2ebef3f5208bb29ee7eece2ec0efe158) )
	ROM_LOAD( "e2",           0x5000, 0x1000, CRC(f7528a52) SHA1(e9d3c57934ee97fcc1f17ecdf3bc954574212220) )
	ROM_LOAD( "f1",           0x7000, 0x1000, CRC(9b1d4cc5) SHA1(9a412fec82f39b9389ff99cceba2e49b2a74df17) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "v_3pa.bin",    0x0000, 0x1000, CRC(4974ffef) SHA1(7bb1e207dd3c5214e405bf32c57ec1b048061050) )
	ROM_LOAD( "a2.gfx",       0x1000, 0x1000, CRC(51845eaf) SHA1(43970d69329f3d49ea1ff57d54abe8340ceef275) )
	ROM_LOAD( "v_3na.bin",    0x2000, 0x1000, CRC(a95c4c63) SHA1(75e312b6872958f3bfc7bafd0743efdf7a74e8f0) )
	ROM_LOAD( "b2.gfx",       0x3000, 0x1000, CRC(7b39c3d0) SHA1(4b8cebb4cdaaca9e1b6fd378f6c390ab05984590) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "hustler.clr",  0x0000, 0x0020, CRC(aa1f7f5e) SHA1(311dd17aa11490a1173c76223e4ccccf8ea29850) )
ROM_END

ROM_START( porter )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "port1.bin",          0x0000, 0x0800, CRC(babaf7fe) SHA1(2138abf57990df9b6f9953efd3be9b2bede49520) )
	ROM_CONTINUE(                   0x2000, 0x0800)
	ROM_LOAD( "port2.bin",          0x0800, 0x0800, CRC(8f7eb0e3) SHA1(7ac5bfc0bb8b6a7a3e9acab5ce9a53f7cba1fca5) )
	ROM_CONTINUE(                   0x2800, 0x0800)
	ROM_LOAD( "port3.bin",          0x1000, 0x0800, CRC(683939b5) SHA1(caf69b03794cb5cf63b1aa52cf8ef355a3aeef87) )
	ROM_CONTINUE(                   0x3000, 0x0800)
	ROM_LOAD( "port4.bin",          0x1800, 0x0800, CRC(6a65d58d) SHA1(05824a41b2912f12bff7887e7483cb3f4367d339) )
	ROM_CONTINUE(                   0x3800,0x0800)
	ROM_LOAD( "port5.bin",          0x4000, 0x0800, CRC(2978a9aa) SHA1(99ec75c7f83f4858b26e083b50fde41fbcfe449a) )
	ROM_LOAD( "port6.bin",          0x4800, 0x0800, CRC(7ecdffb5) SHA1(18ce71b670503bef039c6bfb0aed5e8c10e9eb2d) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "port7.bin",          0x0000, 0x1000, CRC(603294f9) SHA1(168b90fdf38cd2e2c7f54cde16b4d83dc5bb3046) )
	ROM_LOAD( "port8.bin",          0x1000, 0x1000, CRC(b66a763d) SHA1(995b473b1942ff666b0989993587e41e89542172) )

	ROM_REGION( 0x0020, "proms", 0 ) // not in the set
	ROM_LOAD( "mmi6331.6l", 0x0000, 0x0020, BAD_DUMP CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END

ROM_START( tazzmang )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tazzm1.4k",    0x0000, 0x1000, CRC(a14480a1) SHA1(60dac6b57e8331cc4daedaf87faf3e3acc68f378) )
	ROM_LOAD( "tazzm2.5j",    0x1000, 0x1000, CRC(5609f5db) SHA1(3fc50109ea0e012e3e310ae4f5dd0cf460bdca52) )
	ROM_LOAD( "tazzm3.6f",    0x2000, 0x1000, CRC(fe7f7002) SHA1(ac4134c07a798328b18994010bcaf6b3f728466a) )
	ROM_LOAD( "tazzm4.7e",    0x3000, 0x1000, CRC(c9ca1d0a) SHA1(d420ca2e926174e17215212278c86ba9bbb3d9dc) )
	ROM_LOAD( "tazzm5.7l",    0x4000, 0x1000, CRC(f50cd8a6) SHA1(b59ca37171b9acc9854f1beae43cfa5643219a5f) )
	ROM_LOAD( "tazzm6.7l",    0x5000, 0x1000, CRC(5cf2e7d2) SHA1(ad89e2655164e0fc5ecc9af70c5f0dd9b094d432) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "tazm8.1lk",    0x0000, 0x0800, CRC(2c5b612b) SHA1(32e3a41a9a4a8b1285b6a195213ff0d98012360a) )
	ROM_LOAD( "tazzm7.1jh",   0x0800, 0x0800, CRC(3f5ff3ac) SHA1(bc70eef54a45b52c14e35464e5f06b5eec554eb6) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "prom.6l",      0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( bongo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bg1.bin",    0x0000, 0x1000, CRC(de9a8ec6) SHA1(b5ee99b26d1a39e31b643ad0f5723ee8e364023e) )
	ROM_LOAD( "bg2.bin",    0x1000, 0x1000, CRC(a19da662) SHA1(a2674392d489c5e5eeb9abc51572a37cc6045220) )
	ROM_LOAD( "bg3.bin",    0x2000, 0x1000, CRC(9f6f2150) SHA1(26a1f872686ddddcdb690d7b826ba26c20cdec35) )
	ROM_LOAD( "bg4.bin",    0x3000, 0x1000, CRC(f80372d2) SHA1(078e2c8b947103c168c0c85430f8ebc9d09f8ba7) )
	ROM_LOAD( "bg5.bin",    0x4000, 0x1000, CRC(fc92eade) SHA1(f4012a1c4631388a3e8109a8381bc4084ddc8757) )
	ROM_LOAD( "bg6.bin",    0x5000, 0x1000, CRC(561d9e5d) SHA1(68d7fab3cfb5b3360fe8064c70bf21bb1341032f) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "b-h.bin",    0x0000, 0x1000, CRC(fc79d103) SHA1(dac1152221ebdc4cd9bf353b4cc5d45021ca5d9e) )
	ROM_LOAD( "b-k.bin",    0x1000, 0x1000, CRC(94d17bf3) SHA1(2a70968249946de52c5a4cfabafbbf4ecda844a8) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "b-clr.bin",  0x0000, 0x0020, CRC(c4761ada) SHA1(067d12b2d3635ffa6337ed234ba42717447bea00) )
ROM_END

ROM_START( ozon1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom1.bin",     0x0000, 0x1000, CRC(54899e8b) SHA1(270af76ae4396ebda767f160535fa77c0b49726a) )
	ROM_LOAD( "rom2.bin",     0x1000, 0x1000, CRC(3c90fbfc) SHA1(92da614dba3a644eac144bb0ed434d78a31fcb1a) )
	ROM_LOAD( "rom3.bin",     0x2000, 0x1000, CRC(79fe313b) SHA1(ef8fd70f5669b7e7d7184eca2baaddcecb55c22d) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "rom7.bin",     0x0000, 0x0800, CRC(464285e8) SHA1(fff36b034b95050219c70cdfe05ff3bbc452b73e) )
	ROM_LOAD( "rom8.bin",     0x0800, 0x0800, CRC(92056dcc) SHA1(b162da8701bfee465205e8f274ee494063c52c7b) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "ozon1.clr", 0x0000, 0x0020, CRC(605ea6e9) SHA1(d3471e6ef756059c2f7feb32fb8e41181cc1718e) )
ROM_END

ROM_START( ladybugg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lbuggx.1",   0x0000, 0x0800, CRC(e67e241d) SHA1(42b8eaca71c6b346ab54bc722850d6e6d169c517) )
	ROM_LOAD( "lbuggx.2",   0x0800, 0x0800, CRC(3cb1fb9a) SHA1(ee76758c94329dfcc740571195a74d9242aaf49f) )
	ROM_LOAD( "lbuggx.3",   0x1000, 0x0800, CRC(0937009e) SHA1(ef57ebf3d6ab3d6ac0e1faa10c3109d2c80a1248) )
	ROM_LOAD( "lbuggx.4",   0x1800, 0x0800, CRC(3e773f62) SHA1(6348e61f48e5d1f04289098c4c0395335ea5e2a5) )
	ROM_LOAD( "lbuggx.5",   0x2000, 0x0800, CRC(2b0d42e5) SHA1(1547b8127f964eb10862b566f5779f8011c3441d) )
	ROM_LOAD( "lbuggx.6",   0x2800, 0x0800, CRC(159f9433) SHA1(93341a4de1e1e4a3fb004019fc1edba73db6a4c8) )
	ROM_LOAD( "lbuggx.7",   0x3000, 0x0800, CRC(f2be06d5) SHA1(1354332d2d107ad810aa2e261b595285394dfb49) )
	ROM_LOAD( "lbuggx.8",   0x3800, 0x0800, CRC(646fe79f) SHA1(03223d6c4f9050fd6c1c313f0e366ab4989feca4) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "lbuggx.a",   0x0800, 0x0800, CRC(7efb9dc5) SHA1(5e02ea8cd1a1c8efa6708a8615cc2dc9da65a455) )
	ROM_CONTINUE ( 0x0000, 0x0800)
	ROM_LOAD( "lbuggx.b",   0x1800, 0x0800, CRC(351d4ddc) SHA1(048e8a60e57c6eb0a4d7c2175ddd46c4273756c5) )
	ROM_CONTINUE ( 0x1000, 0x0800)

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "lbuggx.clr", 0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )
ROM_END

/*

on a cocktail galaxian pcb (eagle style)

this had a CPU daughtercard with 3 unknown prom/pal/gal type things on it:

Harris M3-7643-5 x2
TI SN746471 ? (number has been partially obliterated on purpose.
and a 74ls126

The CPU has 'VIDEO STARS - V.S PRO - TEL: 03045 61541' on a sticky label on it.

the cpu daughterboard is etched 'competitive video'.

The rom daughtercard may not have come from this precise pcb,
i think it was on a fullsize pcb according to the spacing of the riser pins.
This daughterboard is also etched competitive video, and uses 4 2716's and 3 2732's.

there are 8 rom sockets on this daughterboard, 7 are roms, and the final socket is actually a 6116 ram.

there is a small prom in the midle of the pcb inbetween the risers,
with a circular red labelled saying 'k'. This might be a decryption prom or somethign? i dunno.

there is a TBP18s03 PROM installed at 6L which i guess is the colour prom :)

I think thats about it.

Dumped 26.05.04
Andy Welburn
www.andys-arcade.com

*/

/* this rom mapping probably isn't quite right */
ROM_START( vstars )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c-k2.bin",       0x0000, 0x0400, CRC(b0fcf6c1) SHA1(7dc8a7b99977ea9582c1ed36fa9f1fa502a70c6e) )
	ROM_CONTINUE(               0x0800, 0x0400)
	ROM_LOAD( "c-k1.bin",       0x0c00, 0x0400, CRC(ea9603b2) SHA1(f72202f17f862c7ea81e556690f8fcb9ee926e7f) )
	ROM_CONTINUE(               0x0400, 0x0400)
	ROM_LOAD( "c-k4.bin",       0x1c00, 0x0400, CRC(f5743990) SHA1(defd1577b935e3597eba74344dca5626ec2993dd) )
	ROM_CONTINUE(               0x1000, 0x0400)
	ROM_LOAD( "c-k3.bin",       0x1400, 0x0400, CRC(c4338a77) SHA1(b1ca2d43340b671ef33f3a96ce8e1c286a3e6d80) )
	ROM_CONTINUE(               0x1800, 0x0400)
	ROM_LOAD( "c-k6.bin",       0x2c00, 0x0400, CRC(184b9d7e) SHA1(80159ab19233ce95e9c74d039b6777d01b32e959) )
	ROM_CONTINUE(               0x2000, 0x0400)
	ROM_CONTINUE(               0x2400, 0x0400)
	ROM_CONTINUE(               0x2800, 0x0400)
	ROM_LOAD( "c-k7.bin",       0x4c00, 0x0400, CRC(9ddcc06f) SHA1(63bc77d8b3273681ca4e681105a117d19a0f23a5) )
	ROM_CONTINUE(               0x4000, 0x0400)
	ROM_CONTINUE(               0x4400, 0x0400)
	ROM_CONTINUE(               0x4800, 0x0400)

	/* seems to be data, can't rearrange based on the jumps */
	ROM_LOAD( "c-k5.bin",       0x3c00, 0x0400, CRC(d8df2ec4) SHA1(bef1d4b404cddb8a5f9d4e3f30ee09915c602f56) )
	ROM_CONTINUE(               0x3000, 0x0400)
	ROM_CONTINUE(               0x3400, 0x0400)
	ROM_CONTINUE(               0x3800, 0x0400)

	/* Crazy Kong gfx?! */
	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "1h.bin",   0x0000, 0x1000, CRC(7866d2cb) SHA1(62dd8b80bc0459c7337d8a8cb83e53b999e7f4a9) )
	ROM_LOAD( "1k.bin",   0x1000, 0x1000, CRC(7311a101) SHA1(49d54c8b94cae4ba81d7a7684eaa4e87815bb4da) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6l.bin",     0x0000, 0x0020, CRC(fd81e715) SHA1(eadafe88f26405e6540d4b248b940974e8c31145) )

	ROM_REGION( 0x0020, "proms2", 0 )
	ROM_LOAD( "k.bin",     0x0000, 0x0020, CRC(d46ed869) SHA1(9c0a11df11b1a24ee933d1aa435337b78c3ca643) )
ROM_END

ROM_START( harem )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p0_ic85.bin",  0x0000, 0x2000, CRC(4521b753) SHA1(9033f9c3be8fec1e5ff251e9f60faaf3848a1a1e) )
	ROM_LOAD( "p1_ic87.bin",  0x8000, 0x2000, BAD_DUMP CRC(3cc5d1e8) SHA1(827e2d20de2a00ec016ead249ed3afdccd0c856c) ) // encrypted?

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "s1_ic12.bin",  0x0000, 0x2000, CRC(b54799dd) SHA1(b6aeb010257cba48a52afd33b4f8031c7d99550c) )
	ROM_LOAD( "s2_ic13.bin",  0x2000, 0x1000, CRC(2d5573a4) SHA1(1fdcd99d89e078509634742b2116a35bb199fe4b) )

	ROM_REGION( 0x2000, "unknown", 0 ) /* TMS-based ROM? */
	ROM_LOAD( "a1_ic25.bin",  0x0000, 0x2000, CRC(279f923a) SHA1(166b1b625997766f0de7cc18af52c42268022fcb) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "m0_ic36.bin",  0x0000, 0x2000, CRC(64b3c6d6) SHA1(e71092585f7ffdae85b2a4c9add1bc71e5a608a8) )
	ROM_LOAD( "m1_ic37.bin",  0x2000, 0x2000, CRC(cb0324fb) SHA1(61612f683810339d5d5f31daa4c475d0338d446f) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "harem.clr",    0x0000, 0x0020, CRC(c9a2bf73) SHA1(dad65ebf43a5df147e334afd552e67f5fcd26df7) )
ROM_END

ROM_START( hunchbkg )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "gal_hb_1",     0x0000, 0x0800, CRC(46590e9b) SHA1(5d26578c91adec20d8d8a17d5dade9ef2febcbe5) )
	ROM_LOAD( "gal_hb_2",     0x0800, 0x0800, CRC(4e6e671c) SHA1(5948fc7f390f0343b367d333395427ce2f9b2931) )
	ROM_LOAD( "gal_hb_3",     0x2000, 0x0800, CRC(d29dc242) SHA1(3f6087fe962ee63c2886ad3f502c1a37d357ba87) )
	ROM_LOAD( "gal_hb_4",     0x2800, 0x0800, CRC(d409d292) SHA1(d631c9106106b31b605b6fdf1d4f40e237a725ac) )
	ROM_LOAD( "gal_hb_5",     0x4000, 0x0800, CRC(29d3a8c4) SHA1(2e1ef20d980e5033503d8095e9576dcb8f532f41) )
	ROM_LOAD( "gal_hb_6",     0x4800, 0x0800, CRC(b016fd15) SHA1(cdfbd531e23438f05a7c3aad99a94ce55912aac3) )
	ROM_LOAD( "gal_hb_7",     0x6000, 0x0800, CRC(d2731d27) SHA1(8c4a3d2303d85c3b11803c577a9ad21e6e69011e) )
	ROM_LOAD( "gal_hb_8",     0x6800, 0x0800, CRC(e4b1a666) SHA1(9f73d17cff208374d587536e783be024fc9ab700) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "gal_hb_kl",    0x0000, 0x0800, CRC(3977650e) SHA1(1de05d6ceed3f2ed0925caa8235b63a93f03f61e) )
	ROM_LOAD( "gal_hb_hj",    0x0800, 0x0800, CRC(db489c3d) SHA1(df08607ad07222c1c1c4b3589b50b785bdeefbf2) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "gal_hb_cp",    0x0000, 0x0020, CRC(cbff6762) SHA1(4515a6e12a0a5c485a55291feee17a571120a549) )
ROM_END

ROM_START( drivfrcg )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "dfgp1.bin",    0x2800, 0x0400, CRC(52d5e77d) SHA1(4e68ac1274bbc8cb5b6a7dfb511232bd83482453) )
	ROM_CONTINUE(			  0x2c00, 0x0400 )
	ROM_CONTINUE(			  0x0000, 0x0400 )
	ROM_CONTINUE(			  0x0400, 0x0400 )
	ROM_LOAD( "dfgp2.bin",    0x0800, 0x0400, CRC(9cf4dbce) SHA1(028c168ad0987f21d76c6ac4f756f4fa86c2f8e3) )
	ROM_CONTINUE(			  0x0c00, 0x0400 )
	ROM_CONTINUE(			  0x2000, 0x0400 )
	ROM_CONTINUE(			  0x2400, 0x0400 )
	ROM_LOAD( "dfgp3.bin",    0x6800, 0x0400, CRC(79763f62) SHA1(2bb8921fcd2a8b9543e398e248fd47d7e03dc24d) )
	ROM_CONTINUE(			  0x6c00, 0x0400 )
	ROM_CONTINUE(			  0x4000, 0x0400 )
	ROM_CONTINUE(			  0x4400, 0x0400 )
	ROM_LOAD( "dfgp4.bin",    0x4800, 0x0400, CRC(dd95338b) SHA1(9054986f7b8fee36f458362836ae969e7d1e2456) )
	ROM_CONTINUE(			  0x4c00, 0x0400 )
	ROM_CONTINUE(			  0x6000, 0x0400 )
	ROM_CONTINUE(			  0x6400, 0x0400 )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "dfgj2.bin",    0x0000, 0x1000, CRC(8e19f1e7) SHA1(addd5add2117ef29ce38c0c80584e5d481b9d820) )
	ROM_LOAD( "dfgj1.bin",    0x1000, 0x1000, CRC(86b60ca8) SHA1(be266e2d69e12a196c2195d48b495c0fb9ef8a43) )
	ROM_LOAD( "dfgl2.bin",    0x2000, 0x1000, CRC(ea5e9959) SHA1(6b638d22adf19224cf741458c8ad34d7f7e17e58) )
	ROM_LOAD( "dfgl1.bin",    0x3000, 0x1000, CRC(b7ed195c) SHA1(81b2b444153dacb962a33a5d86a280ed5088637a) )

	/* piggy-backed colour proms */
	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "top.clr",      0x0000, 0x0020, CRC(3110ddae) SHA1(53b2e1cc07915592f6c868131ec296c63a407f04) )
	ROM_LOAD( "bot.clr",      0x0020, 0x0020, CRC(0f0782af) SHA1(32c0dd09ead5c70cee2657e9cb8cb9fcf54c5a6a) )
ROM_END

ROM_START( drivfrcb )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "dfp.bin",      0x2800, 0x0400, CRC(b5b2981d) SHA1(c9ff19791895bf05b569457b1e53dfa0aaeb8e95) )
	ROM_CONTINUE(			  0x2c00, 0x0400 )
	ROM_CONTINUE(			  0x0000, 0x0400 )
	ROM_CONTINUE(			  0x0400, 0x0400 )
	ROM_CONTINUE(			  0x0800, 0x0400 )
	ROM_CONTINUE(			  0x0c00, 0x0400 )
	ROM_CONTINUE(			  0x2000, 0x0400 )
	ROM_CONTINUE(			  0x2400, 0x0400 )
	ROM_CONTINUE(			  0x6800, 0x0400 )
	ROM_CONTINUE(			  0x6c00, 0x0400 )
	ROM_CONTINUE(			  0x4000, 0x0400 )
	ROM_CONTINUE(			  0x4400, 0x0400 )
	ROM_CONTINUE(			  0x4800, 0x0400 )
	ROM_CONTINUE(			  0x4c00, 0x0400 )
	ROM_CONTINUE(			  0x6000, 0x0400 )
	ROM_CONTINUE(			  0x6400, 0x0400 )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "df1.bin",      0x1000, 0x1000, CRC(8adc3de0) SHA1(046fb92913171c621bb62edb0174f04298bfd283) )
	ROM_CONTINUE(			  0x0000, 0x1000 )
	ROM_LOAD( "df2.bin",      0x3000, 0x1000, CRC(6d95ec35) SHA1(c745ee2bc7b1fb53e8bc1ac3a4238bbe00f30cfe) )
	ROM_CONTINUE(			  0x2000, 0x1000 )

	/* piggy-backed colour proms */
	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "top.clr",      0x0000, 0x0020, CRC(3110ddae) SHA1(53b2e1cc07915592f6c868131ec296c63a407f04) )
	ROM_LOAD( "bot.clr",      0x0020, 0x0020, CRC(0f0782af) SHA1(32c0dd09ead5c70cee2657e9cb8cb9fcf54c5a6a) )
ROM_END

ROM_START( racknrol )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "horz_p.bin",   0x0000, 0x1000, CRC(32ca5b43) SHA1(f3e7662f947dcdd80f6eae4f002d2fe64a825aff) )
	ROM_CONTINUE(			  0x2000, 0x1000 )
	ROM_CONTINUE(			  0x4000, 0x1000 )
	ROM_CONTINUE(			  0x6000, 0x1000 )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "horz_g.bin",   0x0000, 0x4000, CRC(97069ad5) SHA1(50199c7bc5083be23a34849cff17906795bf4067) )
	ROM_LOAD( "horz_r.bin",   0x4000, 0x4000, CRC(ff64e84b) SHA1(ceabd522bae26743804987632f35f3c4b5aff179) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.bin",   0x0000, 0x0020, CRC(737802bf) SHA1(9b0476c51ce63898cd690e01e16ee83bae361cb2) )

	ROM_REGION( 0x0200, "user1", 0 ) /* unknown */
	ROM_LOAD( "82s147.bin",   0x0000, 0x0200, CRC(aace7fa5) SHA1(6761530bb3585d2eaa97b7ae77b52e96782ffe0a) )
ROM_END

ROM_START( hexpool )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "vert_p.bin",   0x0000, 0x1000, CRC(bdb078fc) SHA1(85a65c3038dc05a98eae71edf9efdd6659a2966a) )
	ROM_CONTINUE(			  0x2000, 0x1000 )
	ROM_CONTINUE(			  0x4000, 0x1000 )
	ROM_CONTINUE(			  0x6000, 0x1000 )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "vert_g.bin",   0x0000, 0x4000, CRC(7e257e80) SHA1(dabb10d076dc49fc130f58e6d1c4b04e6debce55) )
	ROM_LOAD( "vert_r.bin",   0x4000, 0x4000, CRC(c5f0851e) SHA1(cedcdb29962c6cd65af9d57d0cb2533397d58f99) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.bin",   0x0000, 0x0020, CRC(737802bf) SHA1(9b0476c51ce63898cd690e01e16ee83bae361cb2) )

	ROM_REGION( 0x0200, "user1", 0 ) /* unknown */
	ROM_LOAD( "82s147.bin",   0x0000, 0x0200, CRC(aace7fa5) SHA1(6761530bb3585d2eaa97b7ae77b52e96782ffe0a) )
ROM_END

ROM_START( hexpoola )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "rom.4l",       0x0000, 0x1000, CRC(2ca8018d) SHA1(f0784d18bc7e77515bf2140d8993ae8178919853) )
	ROM_CONTINUE(			  0x2000, 0x1000 )
	ROM_CONTINUE(			  0x4000, 0x1000 )
	ROM_CONTINUE(			  0x6000, 0x1000 )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "rom.1m",       0x0000, 0x4000, CRC(7e257e80) SHA1(dabb10d076dc49fc130f58e6d1c4b04e6debce55) )
	ROM_LOAD( "rom.1l",       0x4000, 0x4000, CRC(c5f0851e) SHA1(cedcdb29962c6cd65af9d57d0cb2533397d58f99) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.11r",   0x0000, 0x0020, CRC(deb2fcf4) SHA1(cdec737a9d9feae912f7cc04ca0adb48f859b5c2) )

	ROM_REGION( 0x0200, "user1", 0 ) /* unknown */
	ROM_LOAD( "82s147.5pr",   0x0000, 0x0200, CRC(cf496b1e) SHA1(5b5ca52b3cc46e18990dae53a98984aeaf264241) )

	ROM_REGION( 0x00eb, "plds", 0 )
	ROM_LOAD( "82s153.6pr.bin", 0x0000, 0x00eb, CRC(bc07939a) SHA1(615b085575ad215662eab2777a2d8b9167c4b9c3) )
ROM_END

ROM_START( trvchlng )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "senko11.bin",  0x0000, 0x1000, CRC(3657331d) SHA1(d9a9a4e4e2e696e70dfb888725c959ec8ce24e3d) )
	ROM_CONTINUE(			  0x2000, 0x1000 )
	ROM_CONTINUE(			  0x4000, 0x1000 )
	ROM_CONTINUE(			  0x6000, 0x1000 )

	ROM_REGION( 0x100000, "user1", 0 )
	ROM_LOAD( "questions",    0x000000, 0x100000, NO_DUMP )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "senko10.bin",  0x0000, 0x4000, CRC(234b59d0) SHA1(5eafdfc6d6a73575835b68361fe29a2dc61e8a83) )
	ROM_LOAD( "senko12.bin",  0x4000, 0x4000, CRC(0bf6b92d) SHA1(6ca993c0642949a52fafea3bc57a08c6881e8120) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "senko1.bin",   0x0000, 0x0020, CRC(1434c7ff) SHA1(0ee5f5351dd84fbf8d3d8eaafbdbe86dd29960f8) )
ROM_END

/* Z80 games */
GAME( 1980, vpool,    hustler,  mooncrst, vpool,    0,        ROT90,  "bootleg", "Video Pool (bootleg on Moon Cresta hardware)", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1981, rockclim, 0,        rockclim, rockclim, 0,	      ROT180, "Taito", "Rock Climber", GAME_SUPPORTS_SAVE )
GAME( 1981, ckongg,   ckong,    ckongg,   ckongg,   0,        ROT90,  "bootleg", "Crazy Kong (bootleg on Galaxian hardware)", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1981, ckongmc,  ckong,    ckongmc,  ckongmc,  0,        ROT90,  "bootleg", "Crazy Kong (bootleg on Moon Cresta hardware)", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE ) // set was marked as 'King Kong on Galaxian'
GAME( 1981, scramblb, scramble, scramblb, scramblb, 0,        ROT90,  "bootleg", "Scramble (bootleg on Galaxian hardware)", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1981, scramb2,  scramble, scramb2,  scramb2,  0,        ROT90,  "bootleg", "Scramble (bootleg)", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1981, 4in1,     0,        4in1,     4in1,     4in1,     ROT90,  "Armenia / Food and Fun", "4 Fun in 1", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1982, bagmanmc, bagman,   bagmanmc, bagmanmc, 0,        ROT90,  "bootleg", "Bagman (bootleg on Moon Cresta hardware)", GAME_WRONG_COLORS | GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1982, dkongjrm, dkongjr,  dkongjrm, dkongjrm, 0,        ROT90,  "bootleg", "Donkey Kong Jr. (bootleg on Moon Cresta hardware)", GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND | GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1982, porter,   dockman,  mooncrst, porter,   0,        ROT90,  "bootleg", "Port Man (bootleg on Moon Cresta hardware)", GAME_IMPERFECT_GRAPHICS | GAME_NO_COCKTAIL ) // missing GFX bank switch!
GAME( 1982, tazzmang, tazmania,	tazzmang, tazzmang, 0,        ROT90,  "bootleg", "Tazz-Mania (bootleg on Galaxian hardware)", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1983, bongo,    0,        bongo,    bongo,    0,	      ROT90,  "Jetsoft", "Bongo", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1983, ozon1,    0,        ozon1,    ozon1,    0,	      ROT90,  "Proma", "Ozon I", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1983, ladybugg, ladybug,  batman2,  ladybugg, ladybugg, ROT270, "bootleg", "Lady Bug (bootleg on Galaxian hardware)", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1982, vstars,   0,        mooncrst, porter,   0,        ROT90,  "Competitive Video?", "Video Stars", GAME_NOT_WORKING )
GAME( 1983, harem,    0,        harem,    harem,    0,        ROT90,  "I.G.R.", "Harem", GAME_NO_SOUND | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )

/* S2650 games */
GAME( 1983, hunchbkg, hunchbak,	hunchbkg, hunchbkg, 0,        ROT90,  "Century Electronics", "Hunchback (Galaxian hardware)", GAME_SUPPORTS_SAVE )
GAME( 1984, drivfrcg, drivfrcp, drivfrcg, drivfrcg, 0,        ROT90,  "Shinkai Inc. (Magic Eletronics USA license)", "Driving Force (Galaxian conversion)", GAME_SUPPORTS_SAVE )
GAME( 1985, drivfrcb, drivfrcp, drivfrcg, drivfrcg, 0,        ROT90,  "bootleg", "Driving Force (Galaxian conversion bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1986, racknrol, 0,        racknrol, racknrol, 0,	      ROT0,   "Senko Industries (Status license from Shinkai Inc.)", "Rack + Roll", GAME_SUPPORTS_SAVE )
GAME( 1986, hexpool,  racknrol, racknrol, racknrol, 0,	      ROT90,  "Senko Industries (Shinkai Inc. license)", "Hex Pool (Shinkai)", GAME_SUPPORTS_SAVE ) // still has Senko logo in gfx rom
GAME( 1985, hexpoola, racknrol, hexpoola, racknrol, 0,	      ROT90,  "Senko Industries", "Hex Pool (Senko)", GAME_SUPPORTS_SAVE )
GAME( 1985, trvchlng, 0,        racknrol, trvchlng, 0,	      ROT90,  "Joyland (Senko license)", "Trivia Challenge", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
