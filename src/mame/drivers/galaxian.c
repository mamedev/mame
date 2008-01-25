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


Notes about 'azurian' :
-----------------------

  bit 6 of IN1 is linked with bit 2 of IN2 (check code at 0x05b3) to set difficulty :

    bit 6  bit 2        contents of
     IN1     IN2          0x40f4            consequences                   difficulty

     OFF     OFF             2          aliens move 2 frames out of 3       easy
     ON      OFF             4          aliens move 4 frames out of 5       hard
     OFF     ON              3          aliens move 3 frames out of 4       normal
     ON      ON              5          aliens move 5 frames out of 6       very hard

  aliens movements is handled by routine at 0x1d59 :

    - alien 1 moves when 0x4044 != 0 else contents of 0x40f4 is stored at 0x4044
    - alien 2 moves when 0x4054 != 0 else contents of 0x40f4 is stored at 0x4054
    - alien 3 moves when 0x4064 != 0 else contents of 0x40f4 is stored at 0x4064


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



TO DO :
-------

  - smooncrs : fix read/writes at/to unmapped memory (when player 2, "cocktail" mode)
               fix the ?#! bug with "bullets" (when player 2, "cocktail" mode)
  - zigzag   : full Dip Switches and Inputs
  - zigzag2  : full Dip Switches and Inputs
  - jumpbug  : full Dip Switches and Inputs
  - jumpbugb : full Dip Switches and Inputs
  - levers   : full Dip Switches and Inputs
  - kingball : full Dip Switches and Inputs
  - kingbalj : full Dip Switches and Inputs
  - frogg    : fix read/writes at/to unmapped/wrong memory
  - scprpng  : fix read/writes at/to unmapped/wrong memory

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "cpu/z80/z80.h"
#include "cpu/s2650/s2650.h"
#include "galaxian.h"
#include "sound/ay8910.h"
#include "sound/sn76496.h"
#include "sound/dac.h"
#include "sound/flt_rc.h"
#include "includes/cclimber.h"


/*************************************
 *
 *  Constants
 *
 *************************************/

#define MASTER_CLOCK		(18432000)

#define PIXEL_CLOCK			(MASTER_CLOCK/3)

/* H counts from 128->511, HBLANK starts at 128 and ends at 256 */
#define HTOTAL				(384)
#define HBEND				(0)		/*(256)*/
#define HBSTART				(256)	/*(128)*/

#define VTOTAL				(264)
#define VBEND				(16)
#define VBSTART				(224+16)



/* Send sound data to the sound cpu and cause an nmi */
static WRITE8_HANDLER( checkman_sound_command_w )
{
	soundlatch_w (0,data);
	cpunum_set_input_line (Machine, 1, INPUT_LINE_NMI, PULSE_LINE);
}

static READ8_HANDLER( drivfrcg_port0_r )
{
	switch (activecpu_get_pc())
	{
		case 0x002e:
		case 0x0297:
			return 0x01;
	}

    return 0;
}


static ADDRESS_MAP_START( galaxian_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x4000, 0x47ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x5000, 0x53ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x5400, 0x57ff) AM_READ(galaxian_videoram_r)
	AM_RANGE(0x5800, 0x58ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x6000, 0x6000) AM_READ(input_port_0_r)
	AM_RANGE(0x6800, 0x6800) AM_READ(input_port_1_r)
	AM_RANGE(0x7000, 0x7000) AM_READ(input_port_2_r)
	AM_RANGE(0x7800, 0x7fff) AM_READ(watchdog_reset_r)
	AM_RANGE(0xfffc, 0xffff) AM_READ(MRA8_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( galaxian_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x4000, 0x47ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x5000, 0x53ff) AM_WRITE(galaxian_videoram_w) AM_BASE(&galaxian_videoram)
	AM_RANGE(0x5800, 0x583f) AM_WRITE(galaxian_attributesram_w) AM_BASE(&galaxian_attributesram)
	AM_RANGE(0x5840, 0x585f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_spriteram) AM_SIZE(&galaxian_spriteram_size)
	AM_RANGE(0x5860, 0x587f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_bulletsram) AM_SIZE(&galaxian_bulletsram_size)
	AM_RANGE(0x5880, 0x58ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x6000, 0x6001) AM_WRITE(galaxian_leds_w)
	AM_RANGE(0x6002, 0x6002) AM_WRITE(galaxian_coin_lockout_w)
	AM_RANGE(0x6003, 0x6003) AM_WRITE(galaxian_coin_counter_w)
	AM_RANGE(0x6004, 0x6007) AM_WRITE(galaxian_lfo_freq_w)
	AM_RANGE(0x6800, 0x6802) AM_WRITE(galaxian_background_enable_w)
	AM_RANGE(0x6803, 0x6803) AM_WRITE(galaxian_noise_enable_w)
	AM_RANGE(0x6805, 0x6805) AM_WRITE(galaxian_shoot_enable_w)
	AM_RANGE(0x6806, 0x6807) AM_WRITE(galaxian_vol_w)
	AM_RANGE(0x7001, 0x7001) AM_WRITE(galaxian_nmi_enable_w)
	AM_RANGE(0x7004, 0x7004) AM_WRITE(galaxian_stars_enable_w)
	AM_RANGE(0x7006, 0x7006) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0x7007, 0x7007) AM_WRITE(galaxian_flip_screen_y_w)
	AM_RANGE(0x7800, 0x7800) AM_WRITE(galaxian_pitch_w)
	AM_RANGE(0xfffc, 0xffff) AM_WRITE(MWA8_RAM)
ADDRESS_MAP_END


static ADDRESS_MAP_START( gmgalax_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_READ(MRA8_BANK1)	/* banked code */
	AM_RANGE(0x4000, 0x47ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x5000, 0x53ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x5400, 0x57ff) AM_READ(galaxian_videoram_r)
	AM_RANGE(0x5800, 0x58ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x6000, 0x6000) AM_READ(gmgalax_input_port_0_r)
	AM_RANGE(0x6800, 0x6800) AM_READ(gmgalax_input_port_1_r)
	AM_RANGE(0x7000, 0x7000) AM_READ(gmgalax_input_port_2_r)
	AM_RANGE(0x7800, 0x78ff) AM_READ(watchdog_reset_r)
ADDRESS_MAP_END


static ADDRESS_MAP_START( mooncrst_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x8000, 0x87ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x9000, 0x93ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x9400, 0x97ff) AM_READ(galaxian_videoram_r)
	AM_RANGE(0x9800, 0x98ff) AM_READ(MRA8_RAM)
	AM_RANGE(0xa000, 0xa000) AM_READ(input_port_0_r)
	AM_RANGE(0xa800, 0xa800) AM_READ(input_port_1_r)
	AM_RANGE(0xb000, 0xb000) AM_READ(input_port_2_r)
	AM_RANGE(0xb800, 0xb800) AM_READ(watchdog_reset_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mooncrst_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x8000, 0x87ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x9000, 0x93ff) AM_WRITE(galaxian_videoram_w) AM_BASE(&galaxian_videoram)
	AM_RANGE(0x9800, 0x983f) AM_WRITE(galaxian_attributesram_w) AM_BASE(&galaxian_attributesram)
	AM_RANGE(0x9840, 0x985f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_spriteram) AM_SIZE(&galaxian_spriteram_size)
	AM_RANGE(0x9860, 0x987f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_bulletsram) AM_SIZE(&galaxian_bulletsram_size)
	AM_RANGE(0x9880, 0x98ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0xa003, 0xa003) AM_WRITE(galaxian_coin_counter_w)
	AM_RANGE(0xa004, 0xa007) AM_WRITE(galaxian_lfo_freq_w)
	AM_RANGE(0xa800, 0xa802) AM_WRITE(galaxian_background_enable_w)
	AM_RANGE(0xa803, 0xa803) AM_WRITE(galaxian_noise_enable_w)
	AM_RANGE(0xa805, 0xa805) AM_WRITE(galaxian_shoot_enable_w)
	AM_RANGE(0xa806, 0xa807) AM_WRITE(galaxian_vol_w)
	AM_RANGE(0xb000, 0xb000) AM_WRITE(galaxian_nmi_enable_w)
	AM_RANGE(0xb004, 0xb004) AM_WRITE(galaxian_stars_enable_w)
	AM_RANGE(0xb006, 0xb006) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0xb007, 0xb007) AM_WRITE(galaxian_flip_screen_y_w)
	AM_RANGE(0xb800, 0xb800) AM_WRITE(galaxian_pitch_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( rockclim_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x4000, 0x47ff) AM_READ(rockclim_videoram_r)
	AM_RANGE(0x5000, 0x53ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x6000, 0x7fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x8000, 0x87ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x8800, 0x8800) AM_READ(input_port_3_r)
	AM_RANGE(0x9000, 0x93ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x9400, 0x97ff) AM_READ(galaxian_videoram_r)
	AM_RANGE(0x9800, 0x98ff) AM_READ(MRA8_RAM)
	AM_RANGE(0xa000, 0xa000) AM_READ(input_port_0_r)
	AM_RANGE(0xa800, 0xa800) AM_READ(input_port_1_r)
	AM_RANGE(0xb000, 0xb000) AM_READ(input_port_2_r)
	AM_RANGE(0xb800, 0xb800) AM_READ(watchdog_reset_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( rockclim_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x4000, 0x47ff) AM_WRITE(rockclim_videoram_w) AM_BASE(&rockclim_videoram)//4800 - 4803 = bg scroll ?
	AM_RANGE(0x4800, 0x4803) AM_WRITE(rockclim_scroll_w)
	AM_RANGE(0x5000, 0x53ff) AM_WRITE(MWA8_RAM)//?
	AM_RANGE(0x6000, 0x7fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x8000, 0x87ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x9000, 0x93ff) AM_WRITE(galaxian_videoram_w) AM_BASE(&galaxian_videoram)
	AM_RANGE(0x9800, 0x983f) AM_WRITE(galaxian_attributesram_w) AM_BASE(&galaxian_attributesram)
	AM_RANGE(0x9840, 0x985f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_spriteram) AM_SIZE(&galaxian_spriteram_size)
	AM_RANGE(0x9860, 0x987f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_bulletsram) AM_SIZE(&galaxian_bulletsram_size)
	AM_RANGE(0x9880, 0x98ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0xa000, 0xa002) AM_WRITE(galaxian_gfxbank_w)// a002 - sprite bank
	AM_RANGE(0xa003, 0xa003) AM_WRITE(galaxian_coin_counter_w)
	AM_RANGE(0xa004, 0xa007) AM_WRITE(galaxian_lfo_freq_w)
	AM_RANGE(0xa800, 0xa802) AM_WRITE(galaxian_background_enable_w)
	AM_RANGE(0xa803, 0xa803) AM_WRITE(galaxian_noise_enable_w)
	AM_RANGE(0xa805, 0xa805) AM_WRITE(galaxian_shoot_enable_w)
	AM_RANGE(0xa806, 0xa807) AM_WRITE(galaxian_vol_w)
	AM_RANGE(0xb000, 0xb000) AM_WRITE(galaxian_nmi_enable_w)
	AM_RANGE(0xb006, 0xb006) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0xb007, 0xb007) AM_WRITE(galaxian_flip_screen_y_w)
	AM_RANGE(0xb800, 0xb800) AM_WRITE(galaxian_pitch_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( mshuttle_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x4fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x8000, 0x83ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x9000, 0x93ff) AM_WRITE(galaxian_videoram_w) AM_BASE(&galaxian_videoram)
	AM_RANGE(0x9800, 0x983f) AM_WRITE(galaxian_attributesram_w) AM_BASE(&galaxian_attributesram)
	AM_RANGE(0x9840, 0x985f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_spriteram) AM_SIZE(&galaxian_spriteram_size)
	AM_RANGE(0x9860, 0x987f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_bulletsram) AM_SIZE(&galaxian_bulletsram_size)
	AM_RANGE(0x9880, 0x98ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0xa000, 0xa000) AM_WRITE(galaxian_nmi_enable_w)
	AM_RANGE(0xa001, 0xa001) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0xa002, 0xa002) AM_WRITE(galaxian_flip_screen_y_w)
	AM_RANGE(0xa004, 0xa004) AM_WRITE(cclimber_sample_trigger_w)
	AM_RANGE(0xa800, 0xa800) AM_WRITE(cclimber_sample_rate_w)
	AM_RANGE(0xb000, 0xb000) AM_WRITE(cclimber_sample_volume_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mshuttle_readport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x0c, 0x0c) AM_READ(AY8910_read_port_0_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mshuttle_writeport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x08, 0x08) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x09, 0x09) AM_WRITE(AY8910_write_port_0_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( skybase_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x8000, 0x87ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x9000, 0x93ff) AM_WRITE(galaxian_videoram_w) AM_BASE(&galaxian_videoram)
	AM_RANGE(0x9800, 0x983f) AM_WRITE(galaxian_attributesram_w) AM_BASE(&galaxian_attributesram)
	AM_RANGE(0x9840, 0x985f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_spriteram) AM_SIZE(&galaxian_spriteram_size)
	AM_RANGE(0x9860, 0x987f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_bulletsram) AM_SIZE(&galaxian_bulletsram_size)
	AM_RANGE(0x9880, 0x98ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0xa002, 0xa002) AM_WRITE(galaxian_gfxbank_w)
	AM_RANGE(0xa004, 0xa007) AM_WRITE(galaxian_lfo_freq_w)
	AM_RANGE(0xa800, 0xa802) AM_WRITE(galaxian_background_enable_w)
	AM_RANGE(0xa803, 0xa803) AM_WRITE(galaxian_noise_enable_w)
	AM_RANGE(0xa805, 0xa805) AM_WRITE(galaxian_shoot_enable_w)
	AM_RANGE(0xa806, 0xa807) AM_WRITE(galaxian_vol_w)
	AM_RANGE(0xb000, 0xb000) AM_WRITE(galaxian_nmi_enable_w)
	AM_RANGE(0xb004, 0xb004) AM_WRITE(galaxian_stars_enable_w)
	AM_RANGE(0xb006, 0xb006) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0xb007, 0xb007) AM_WRITE(galaxian_flip_screen_y_w)
	AM_RANGE(0xb800, 0xb800) AM_WRITE(galaxian_pitch_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( scramblb_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x4000, 0x4bff) AM_READ(MRA8_RAM)
	AM_RANGE(0x5000, 0x50ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x6000, 0x6000) AM_READ(input_port_0_r)
	AM_RANGE(0x6800, 0x6800) AM_READ(input_port_1_r)
	AM_RANGE(0x7000, 0x7000) AM_READ(input_port_2_r)
	AM_RANGE(0x7800, 0x7800) AM_READ(watchdog_reset_r)
	AM_RANGE(0x8102, 0x8102) AM_READ(scramblb_protection_1_r)
	AM_RANGE(0x8202, 0x8202) AM_READ(scramblb_protection_2_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( scramblb_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x4000, 0x47ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x4800, 0x4bff) AM_WRITE(galaxian_videoram_w) AM_BASE(&galaxian_videoram)
	AM_RANGE(0x5000, 0x503f) AM_WRITE(galaxian_attributesram_w) AM_BASE(&galaxian_attributesram)
	AM_RANGE(0x5040, 0x505f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_spriteram) AM_SIZE(&galaxian_spriteram_size)
	AM_RANGE(0x5060, 0x507f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_bulletsram) AM_SIZE(&galaxian_bulletsram_size)
	AM_RANGE(0x5080, 0x50ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x6000, 0x6001) AM_WRITE(MWA8_NOP)  /* sound triggers */
	AM_RANGE(0x6003, 0x6003) AM_WRITE(galaxian_coin_counter_w)
	AM_RANGE(0x6004, 0x6007) AM_WRITE(galaxian_lfo_freq_w)
	AM_RANGE(0x6800, 0x6802) AM_WRITE(galaxian_background_enable_w)
	AM_RANGE(0x6803, 0x6803) AM_WRITE(galaxian_noise_enable_w)
	AM_RANGE(0x6805, 0x6805) AM_WRITE(galaxian_shoot_enable_w)
	AM_RANGE(0x6806, 0x6807) AM_WRITE(galaxian_vol_w)
	AM_RANGE(0x7001, 0x7001) AM_WRITE(galaxian_nmi_enable_w)
	AM_RANGE(0x7002, 0x7002) AM_WRITE(galaxian_coin_counter_w)
	AM_RANGE(0x7003, 0x7003) AM_WRITE(scramble_background_enable_w)
	AM_RANGE(0x7004, 0x7004) AM_WRITE(galaxian_stars_enable_w)
	AM_RANGE(0x7006, 0x7006) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0x7007, 0x7007) AM_WRITE(galaxian_flip_screen_y_w)
	AM_RANGE(0x7800, 0x7800) AM_WRITE(galaxian_pitch_w)
ADDRESS_MAP_END



static READ8_HANDLER( scramb2_protection_r ) { return 0x25; }
static READ8_HANDLER( scramb2_port0_r ) { return (readinputport(0)>>offset)&0x1; }
static READ8_HANDLER( scramb2_port1_r ) { return (readinputport(1)>>offset)&0x1; }
static READ8_HANDLER( scramb2_port2_r ) { return (readinputport(2)>>offset)&0x1; }

static ADDRESS_MAP_START( scramb2_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x4000, 0x4bff) AM_READ(MRA8_RAM)
	AM_RANGE(0x5000, 0x50ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x5800, 0x5fff) AM_READ(scramb2_protection_r) // must return 0x25
	AM_RANGE(0x6000, 0x6007) AM_READ(scramb2_port0_r) // reads from 8 addresses, 1 bit per address
	AM_RANGE(0x6800, 0x6807) AM_READ(scramb2_port1_r) // reads from 8 addresses, 1 bit per address
	AM_RANGE(0x7000, 0x7007) AM_READ(watchdog_reset_r)
	AM_RANGE(0x7800, 0x7807) AM_READ(scramb2_port2_r) // reads from 8 addresses, 1 bit per address
ADDRESS_MAP_END

static ADDRESS_MAP_START( scramb2_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x4000, 0x47ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x4800, 0x4bff) AM_WRITE(galaxian_videoram_w) AM_BASE(&galaxian_videoram)
	AM_RANGE(0x4c00, 0x4fff) AM_WRITE(galaxian_videoram_w) // mirror
	AM_RANGE(0x5000, 0x503f) AM_WRITE(galaxian_attributesram_w) AM_BASE(&galaxian_attributesram)
	AM_RANGE(0x5040, 0x505f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_spriteram) AM_SIZE(&galaxian_spriteram_size)
	AM_RANGE(0x5060, 0x507f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_bulletsram) AM_SIZE(&galaxian_bulletsram_size)
	AM_RANGE(0x5080, 0x50ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x6801, 0x6801) AM_WRITE(galaxian_nmi_enable_w)
	AM_RANGE(0x6802, 0x6802) AM_WRITE(galaxian_coin_counter_w)
	AM_RANGE(0x6804, 0x6804) AM_WRITE(galaxian_stars_enable_w)
	AM_RANGE(0x6806, 0x6806) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0x6807, 0x6807) AM_WRITE(galaxian_flip_screen_y_w)
	AM_RANGE(0x7006, 0x7006) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x7007, 0x7007) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x7800, 0x7800) AM_WRITE(galaxian_pitch_w)
ADDRESS_MAP_END



static ADDRESS_MAP_START( jumpbug_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x4000, 0x4bff) AM_READ(MRA8_RAM)
	AM_RANGE(0x4c00, 0x4fff) AM_READ(galaxian_videoram_r)
	AM_RANGE(0x5000, 0x50ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x6000, 0x6000) AM_READ(input_port_0_r)
	AM_RANGE(0x6800, 0x6800) AM_READ(input_port_1_r)
	AM_RANGE(0x7000, 0x7000) AM_READ(input_port_2_r)
	AM_RANGE(0x8000, 0xafff) AM_READ(MRA8_ROM)
	AM_RANGE(0xb000, 0xbfff) AM_READ(jumpbug_protection_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( jumpbug_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x4000, 0x47ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x4800, 0x4bff) AM_WRITE(galaxian_videoram_w) AM_BASE(&galaxian_videoram)
	AM_RANGE(0x4c00, 0x4fff) AM_WRITE(galaxian_videoram_w)
	AM_RANGE(0x5000, 0x503f) AM_WRITE(galaxian_attributesram_w) AM_BASE(&galaxian_attributesram)
	AM_RANGE(0x5040, 0x505f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_spriteram) AM_SIZE(&galaxian_spriteram_size)
	AM_RANGE(0x5060, 0x507f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_bulletsram) AM_SIZE(&galaxian_bulletsram_size)
	AM_RANGE(0x5080, 0x50ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x5800, 0x5800) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x5900, 0x5900) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x6002, 0x6006) AM_WRITE(galaxian_gfxbank_w)
	AM_RANGE(0x7001, 0x7001) AM_WRITE(galaxian_nmi_enable_w)
	AM_RANGE(0x7002, 0x7002) AM_WRITE(galaxian_coin_counter_w)
	AM_RANGE(0x7004, 0x7004) AM_WRITE(galaxian_stars_enable_w)
	AM_RANGE(0x7006, 0x7006) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0x7007, 0x7007) AM_WRITE(galaxian_flip_screen_y_w)
	AM_RANGE(0x8000, 0xafff) AM_WRITE(MWA8_ROM)
ADDRESS_MAP_END


static ADDRESS_MAP_START( checkman_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x8000, 0x83ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x9000, 0x93ff) AM_WRITE(galaxian_videoram_w) AM_BASE(&galaxian_videoram)
	AM_RANGE(0x9800, 0x983f) AM_WRITE(galaxian_attributesram_w) AM_BASE(&galaxian_attributesram)
	AM_RANGE(0x9840, 0x985f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_spriteram) AM_SIZE(&galaxian_spriteram_size)
	AM_RANGE(0x9860, 0x987f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_bulletsram) AM_SIZE(&galaxian_bulletsram_size)
	AM_RANGE(0x9880, 0x98ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0xa004, 0xa007) AM_WRITE(galaxian_lfo_freq_w)
	AM_RANGE(0xa800, 0xa802) AM_WRITE(galaxian_background_enable_w)
	AM_RANGE(0xa803, 0xa803) AM_WRITE(galaxian_noise_enable_w)
	AM_RANGE(0xa805, 0xa805) AM_WRITE(galaxian_shoot_enable_w)
	AM_RANGE(0xa806, 0xa807) AM_WRITE(galaxian_vol_w)
	AM_RANGE(0xb001, 0xb001) AM_WRITE(galaxian_nmi_enable_w)
	AM_RANGE(0xb004, 0xb004) AM_WRITE(galaxian_stars_enable_w)
	AM_RANGE(0xb006, 0xb006) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0xb007, 0xb007) AM_WRITE(galaxian_flip_screen_y_w)
	AM_RANGE(0xb800, 0xb800) AM_WRITE(galaxian_pitch_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( checkmaj_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x4000, 0x47ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x5000, 0x53ff) AM_WRITE(galaxian_videoram_w) AM_BASE(&galaxian_videoram)
	AM_RANGE(0x5800, 0x583f) AM_WRITE(galaxian_attributesram_w) AM_BASE(&galaxian_attributesram)
	AM_RANGE(0x5840, 0x585f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_spriteram) AM_SIZE(&galaxian_spriteram_size)
	AM_RANGE(0x5860, 0x587f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_bulletsram) AM_SIZE(&galaxian_bulletsram_size)
	AM_RANGE(0x5880, 0x58ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x7001, 0x7001) AM_WRITE(galaxian_nmi_enable_w)
	AM_RANGE(0x7006, 0x7006) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0x7007, 0x7007) AM_WRITE(galaxian_flip_screen_y_w)
	AM_RANGE(0x7800, 0x7800) AM_WRITE(checkman_sound_command_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( checkman_writeport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0, 0) AM_WRITE(checkman_sound_command_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( checkman_sound_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x2000, 0x23ff) AM_READ(MRA8_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( checkman_sound_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x2000, 0x23ff) AM_WRITE(MWA8_RAM)
ADDRESS_MAP_END


static ADDRESS_MAP_START( checkman_sound_readport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x03, 0x03) AM_READ(soundlatch_r)
	AM_RANGE(0x06, 0x06) AM_READ(AY8910_read_port_0_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( checkman_sound_writeport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x04, 0x04) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(AY8910_write_port_0_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( checkmaj_sound_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x8000, 0x83ff) AM_READ(MRA8_RAM)
	AM_RANGE(0xa002, 0xa002) AM_READ(AY8910_read_port_0_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( checkmaj_sound_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x8000, 0x83ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0xa000, 0xa000) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0xa001, 0xa001) AM_WRITE(AY8910_write_port_0_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( kingball_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x2fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x8000, 0x83ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x9000, 0x93ff) AM_WRITE(galaxian_videoram_w) AM_BASE(&galaxian_videoram)
	AM_RANGE(0x9800, 0x983f) AM_WRITE(galaxian_attributesram_w) AM_BASE(&galaxian_attributesram)
	AM_RANGE(0x9840, 0x985f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_spriteram) AM_SIZE(&galaxian_spriteram_size)
	AM_RANGE(0x9860, 0x987f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_bulletsram) AM_SIZE(&galaxian_bulletsram_size)
	AM_RANGE(0x9880, 0x98ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0xa000, 0xa001) AM_WRITE(galaxian_leds_w)
	AM_RANGE(0xa002, 0xa002) AM_WRITE(galaxian_coin_lockout_w)
	AM_RANGE(0xa003, 0xa003) AM_WRITE(galaxian_coin_counter_w)
	AM_RANGE(0xa004, 0xa007) AM_WRITE(galaxian_lfo_freq_w)
	AM_RANGE(0xa800, 0xa802) AM_WRITE(galaxian_background_enable_w)
	AM_RANGE(0xa803, 0xa803) AM_WRITE(galaxian_noise_enable_w)
	AM_RANGE(0xa805, 0xa805) AM_WRITE(galaxian_shoot_enable_w)
	AM_RANGE(0xa806, 0xa807) AM_WRITE(galaxian_vol_w) //
	AM_RANGE(0xb000, 0xb000) AM_WRITE(kingball_sound1_w)
	AM_RANGE(0xb001, 0xb001) AM_WRITE(galaxian_nmi_enable_w)
	AM_RANGE(0xb002, 0xb002) AM_WRITE(kingball_sound2_w)
	AM_RANGE(0xb003, 0xb003) AM_WRITE(kingball_speech_dip_w)
	AM_RANGE(0xb004, 0xb004) AM_WRITE(MWA8_NOP)					/* noise generator enable */
	AM_RANGE(0xb006, 0xb006) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0xb007, 0xb007) AM_WRITE(galaxian_flip_screen_y_w)
	AM_RANGE(0xb800, 0xb800) AM_WRITE(galaxian_pitch_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( kingball_sound_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_READ(MRA8_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( kingball_sound_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_WRITE(MWA8_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( kingball_sound_readport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00, 0x00) AM_READ(soundlatch_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( kingball_sound_writeport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00, 0x00) AM_WRITE(DAC_0_data_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( _4in1_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_READ(MRA8_BANK1)	/* banked game code */
	AM_RANGE(0x4000, 0x47ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x5000, 0x53ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x5400, 0x57ff) AM_READ(galaxian_videoram_r)
	AM_RANGE(0x5800, 0x58ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x6000, 0x6000) AM_READ(input_port_0_r)
	AM_RANGE(0x6800, 0x6800) AM_READ(_4in1_input_port_1_r)
	AM_RANGE(0x7000, 0x7000) AM_READ(_4in1_input_port_2_r)
	AM_RANGE(0x7800, 0x78ff) AM_READ(watchdog_reset_r)
	AM_RANGE(0xc000, 0xdfff) AM_READ(MRA8_ROM)	/* fixed menu code */
ADDRESS_MAP_END

static ADDRESS_MAP_START( _4in1_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_WRITE(MWA8_ROM)	/* banked game code */
	AM_RANGE(0x4000, 0x47ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x5000, 0x53ff) AM_WRITE(galaxian_videoram_w) AM_BASE(&galaxian_videoram)
	AM_RANGE(0x5800, 0x583f) AM_WRITE(galaxian_attributesram_w) AM_BASE(&galaxian_attributesram)
	AM_RANGE(0x5840, 0x585f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_spriteram) AM_SIZE(&galaxian_spriteram_size)
	AM_RANGE(0x5860, 0x587f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_bulletsram) AM_SIZE(&galaxian_bulletsram_size)
	AM_RANGE(0x5880, 0x58ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x6000, 0x6001) AM_WRITE(galaxian_leds_w)
//  AM_RANGE(0x6002, 0x6002) AM_WRITE(galaxian_coin_lockout_w)
	AM_RANGE(0x6003, 0x6003) AM_WRITE(galaxian_coin_counter_w)
	AM_RANGE(0x6004, 0x6007) AM_WRITE(galaxian_lfo_freq_w)
	AM_RANGE(0x6800, 0x6802) AM_WRITE(galaxian_background_enable_w)
//  AM_RANGE(0x6803, 0x6803) AM_WRITE(galaxian_noise_enable_w) /* not hooked up? */
	AM_RANGE(0x6805, 0x6805) AM_WRITE(galaxian_shoot_enable_w)
	AM_RANGE(0x6806, 0x6807) AM_WRITE(galaxian_vol_w)
	AM_RANGE(0x7001, 0x7001) AM_WRITE(galaxian_nmi_enable_w)
	AM_RANGE(0x7004, 0x7004) AM_WRITE(galaxian_stars_enable_w)
	AM_RANGE(0x7006, 0x7006) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0x7007, 0x7007) AM_WRITE(galaxian_flip_screen_y_w)
	AM_RANGE(0x7800, 0x7800) AM_WRITE(galaxian_pitch_w)
	AM_RANGE(0x8000, 0x8000) AM_WRITE(_4in1_bank_w)
	AM_RANGE(0xc000, 0xdfff) AM_WRITE(MWA8_ROM) /* Fixed Menu Code */
ADDRESS_MAP_END


static ADDRESS_MAP_START( bagmanmc_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x6000, 0x67ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x9000, 0x93ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x9400, 0x97ff) AM_READ(galaxian_videoram_r)
	AM_RANGE(0x9800, 0x98ff) AM_READ(MRA8_RAM)
	AM_RANGE(0xa000, 0xa000) AM_READ(input_port_0_r)
	AM_RANGE(0xa800, 0xa800) AM_READ(input_port_1_r)
	AM_RANGE(0xb000, 0xb000) AM_READ(input_port_2_r)
	AM_RANGE(0xb800, 0xb800) AM_READ(watchdog_reset_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( bagmanmc_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x6000, 0x67ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x9000, 0x93ff) AM_WRITE(galaxian_videoram_w) AM_BASE(&galaxian_videoram)
	AM_RANGE(0x9800, 0x983f) AM_WRITE(galaxian_attributesram_w) AM_BASE(&galaxian_attributesram)
	AM_RANGE(0x9840, 0x985f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_spriteram) AM_SIZE(&galaxian_spriteram_size)
	AM_RANGE(0x9860, 0x987f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_bulletsram) AM_SIZE(&galaxian_bulletsram_size)
	AM_RANGE(0x9880, 0x98ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0xa003, 0xa003) AM_WRITE(galaxian_coin_counter_w)
	AM_RANGE(0xa803, 0xa803) AM_WRITE(galaxian_noise_enable_w)
	AM_RANGE(0xb001, 0xb001) AM_WRITE(galaxian_nmi_enable_w)
	AM_RANGE(0xb006, 0xb006) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0xb007, 0xb007) AM_WRITE(galaxian_flip_screen_y_w)
	AM_RANGE(0xb800, 0xb800) AM_WRITE(galaxian_pitch_w)
ADDRESS_MAP_END

static int latch;

static WRITE8_HANDLER( zigzag_8910_latch_w )
{
	latch = offset;
}

static WRITE8_HANDLER( zigzag_8910_data_trigger_w )
{
	AY8910_write_port_0_w(0,latch);
}

static WRITE8_HANDLER( zigzag_8910_control_trigger_w )
{
	AY8910_control_port_0_w(0,latch);
}

static ADDRESS_MAP_START( zigzag_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x2000, 0x2fff) AM_READ(MRA8_BANK1)
	AM_RANGE(0x3000, 0x3fff) AM_READ(MRA8_BANK2)
	AM_RANGE(0x4000, 0x47ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x5000, 0x53ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x5800, 0x58ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x6000, 0x6000) AM_READ(input_port_0_r)
	AM_RANGE(0x6800, 0x6800) AM_READ(input_port_1_r)
	AM_RANGE(0x7000, 0x7000) AM_READ(input_port_2_r)
	AM_RANGE(0x7800, 0x7800) AM_READ(watchdog_reset_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( zigzag_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x4000, 0x47ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x4800, 0x4800) AM_WRITE(MWA8_NOP)	/* part of the 8910 interface */
	AM_RANGE(0x4801, 0x4801) AM_WRITE(zigzag_8910_data_trigger_w)
	AM_RANGE(0x4803, 0x4803) AM_WRITE(zigzag_8910_control_trigger_w)
	AM_RANGE(0x4900, 0x49ff) AM_WRITE(zigzag_8910_latch_w)
	AM_RANGE(0x4a00, 0x4a00) AM_WRITE(MWA8_NOP)	/* part of the 8910 interface */
	AM_RANGE(0x5000, 0x53ff) AM_WRITE(galaxian_videoram_w) AM_BASE(&galaxian_videoram)
	AM_RANGE(0x5800, 0x583f) AM_WRITE(galaxian_attributesram_w) AM_BASE(&galaxian_attributesram)
	AM_RANGE(0x5840, 0x587f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_spriteram) AM_SIZE(&galaxian_spriteram_size)	/* no bulletsram, all sprites */
	AM_RANGE(0x5880, 0x58ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x7001, 0x7001) AM_WRITE(galaxian_nmi_enable_w)
	AM_RANGE(0x7002, 0x7002) AM_WRITE(zigzag_sillyprotection_w)
	AM_RANGE(0x7006, 0x7006) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0x7007, 0x7007) AM_WRITE(galaxian_flip_screen_y_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( scorpnmc_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x4000, 0x47ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x5000, 0x67ff) AM_READ(MRA8_ROM)
	AM_RANGE(0x9000, 0x93ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x9400, 0x97ff) AM_READ(galaxian_videoram_r)
	AM_RANGE(0x9800, 0x98ff) AM_READ(MRA8_RAM)
	AM_RANGE(0xa000, 0xa000) AM_READ(input_port_0_r)
	AM_RANGE(0xa800, 0xa800) AM_READ(input_port_1_r)
	AM_RANGE(0xb001, 0xb001) AM_READ(input_port_2_r)
	AM_RANGE(0xb002, 0xb002) AM_READ(input_port_3_r)
	AM_RANGE(0xb800, 0xb800) AM_READ(watchdog_reset_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( scorpnmc_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x4000, 0x47ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x5000, 0x67ff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x9000, 0x93ff) AM_WRITE(galaxian_videoram_w) AM_BASE(&galaxian_videoram)
	AM_RANGE(0x9800, 0x983f) AM_WRITE(galaxian_attributesram_w) AM_BASE(&galaxian_attributesram)
	AM_RANGE(0x9840, 0x985f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_spriteram) AM_SIZE(&galaxian_spriteram_size)
	AM_RANGE(0x9860, 0x987f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_bulletsram) AM_SIZE(&galaxian_bulletsram_size)
	AM_RANGE(0x9880, 0x98ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0xa003, 0xa003) AM_WRITE(galaxian_coin_counter_w)
	AM_RANGE(0xa004, 0xa007) AM_WRITE(galaxian_lfo_freq_w)
	AM_RANGE(0xa800, 0xa802) AM_WRITE(galaxian_background_enable_w)
	AM_RANGE(0xa803, 0xa803) AM_WRITE(galaxian_noise_enable_w)
	AM_RANGE(0xa805, 0xa805) AM_WRITE(galaxian_shoot_enable_w)
	AM_RANGE(0xa806, 0xa807) AM_WRITE(galaxian_vol_w)
	AM_RANGE(0xb001, 0xb001) AM_WRITE(galaxian_nmi_enable_w)
	AM_RANGE(0xb004, 0xb004) AM_WRITE(galaxian_stars_enable_w)
	AM_RANGE(0xb006, 0xb006) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0xb007, 0xb007) AM_WRITE(galaxian_flip_screen_y_w)
	AM_RANGE(0xb800, 0xb800) AM_WRITE(galaxian_pitch_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( dkongjrm_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x6000, 0x6fff) AM_READ(MRA8_RAM)
	AM_RANGE(0x7000, 0x7fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x9000, 0x93ff) AM_READ(MRA8_RAM)
	AM_RANGE(0xa000, 0xa0ff) AM_READ(input_port_0_r)
	AM_RANGE(0xa800, 0xa8ff) AM_READ(input_port_1_r)
	AM_RANGE(0xb000, 0xb0ff) AM_READ(input_port_2_r)
	AM_RANGE(0xb800, 0xb800) AM_READ(watchdog_reset_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dkongjrm_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x6000, 0x6fff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x7000, 0x7fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x9000, 0x93ff) AM_WRITE(galaxian_videoram_w) AM_BASE(&galaxian_videoram)
	AM_RANGE(0x9800, 0x983f) AM_WRITE(galaxian_attributesram_w) AM_BASE(&galaxian_attributesram)
	AM_RANGE(0x9840, 0x987f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_spriteram) AM_SIZE(&galaxian_spriteram_size)
	AM_RANGE(0x98c0, 0x98ff) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_spriteram2) AM_SIZE(&galaxian_spriteram2_size)
	AM_RANGE(0xa003, 0xa003) AM_WRITE(galaxian_coin_counter_w)
  //AM_RANGE(0xa004, 0xa007) AM_WRITE(galaxian_lfo_freq_w)
	AM_RANGE(0xa800, 0xa802) AM_WRITE(galaxian_background_enable_w)
	AM_RANGE(0xa803, 0xa803) AM_WRITE(galaxian_noise_enable_w)
  //AM_RANGE(0xa805, 0xa805) AM_WRITE(galaxian_shoot_enable_w)
	AM_RANGE(0xa806, 0xa807) AM_WRITE(galaxian_vol_w)
	AM_RANGE(0xb000, 0xb000) AM_WRITE(galaxian_gfxbank_w)
	AM_RANGE(0xb001, 0xb001) AM_WRITE(galaxian_nmi_enable_w)
  //AM_RANGE(0xb004, 0xb004) AM_WRITE(galaxian_stars_enable_w)
	AM_RANGE(0xb006, 0xb006) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0xb007, 0xb007) AM_WRITE(galaxian_flip_screen_y_w)
	AM_RANGE(0xb800, 0xb800) AM_WRITE(galaxian_pitch_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( ozon1_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x2fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x4000, 0x4200) AM_READ(MRA8_RAM)
	AM_RANGE(0x4300, 0x43ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x4800, 0x4bff) AM_READ(galaxian_videoram_r)
	AM_RANGE(0x5000, 0x503f) AM_READ(MRA8_RAM)
	AM_RANGE(0x5040, 0x505f) AM_READ(MRA8_RAM)
	AM_RANGE(0x8100, 0x8100) AM_READ(input_port_0_r)
	AM_RANGE(0x8101, 0x8101) AM_READ(input_port_1_r)
	AM_RANGE(0x8102, 0x8102) AM_READ(input_port_2_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( ozon1_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x2fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x4000, 0x4200) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x4300, 0x43ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x4800, 0x4bff) AM_WRITE(galaxian_videoram_w) AM_BASE(&galaxian_videoram)
	AM_RANGE(0x4c00, 0x4fff) AM_WRITE(galaxian_videoram_w)
	AM_RANGE(0x5000, 0x503f) AM_WRITE(galaxian_attributesram_w) AM_BASE(&galaxian_attributesram)
	AM_RANGE(0x5040, 0x505f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_spriteram) AM_SIZE(&galaxian_spriteram_size)
	AM_RANGE(0x6801, 0x6801) AM_WRITE(MWA8_NOP) //continuosly 0 and 1
	AM_RANGE(0x6802, 0x6802) AM_WRITE(galaxian_coin_counter_w)
	AM_RANGE(0x6806, 0x6806) AM_WRITE(MWA8_NOP) //only one 0 at reset
	AM_RANGE(0x6807, 0x6807) AM_WRITE(MWA8_NOP) //only one 0 at reset
	AM_RANGE(0x8103, 0x8103) AM_WRITE(MWA8_NOP) //only one 9b at reset
ADDRESS_MAP_END

static ADDRESS_MAP_START( ozon1_writeport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00, 0x00) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(AY8910_control_port_0_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( drivfrcg, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x1480, 0x14bf) AM_MIRROR(0x6000) AM_WRITE(galaxian_attributesram_w) AM_BASE(&galaxian_attributesram)
	AM_RANGE(0x14c0, 0x14ff) AM_MIRROR(0x6000) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_spriteram) AM_SIZE(&galaxian_spriteram_size)
	AM_RANGE(0x1500, 0x1500) AM_MIRROR(0x6000) AM_READ(input_port_0_r)
	AM_RANGE(0x1503, 0x1503) AM_MIRROR(0x6000) AM_WRITE(galaxian_coin_counter_w)
	AM_RANGE(0x1580, 0x1580) AM_MIRROR(0x6000) AM_READ(input_port_1_r)
	AM_RANGE(0x1580, 0x1582) AM_MIRROR(0x6000) AM_WRITE(galaxian_background_enable_w)
	AM_RANGE(0x1583, 0x1583) AM_MIRROR(0x6000) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x1585, 0x1585) AM_MIRROR(0x6000) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x1586, 0x1587) AM_MIRROR(0x6000) AM_WRITE(galaxian_lfo_freq_w)
	AM_RANGE(0x1600, 0x1600) AM_MIRROR(0x6000) AM_READWRITE(input_port_2_r, galaxian_pitch_w)
	AM_RANGE(0x1700, 0x1700) AM_MIRROR(0x6000) AM_READWRITE(input_port_3_r, MWA8_NOP)
	AM_RANGE(0x1701, 0x1701) AM_MIRROR(0x6000) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x1704, 0x1707) AM_MIRROR(0x6000) AM_WRITE(galaxian_vol_w)
	AM_RANGE(0x1800, 0x1bff) AM_MIRROR(0x6000) AM_WRITE(galaxian_videoram_w) AM_BASE(&galaxian_videoram)
	AM_RANGE(0x1c00, 0x1fff) AM_MIRROR(0x6000) AM_RAM
	AM_RANGE(0x2000, 0x2fff) AM_ROM
	AM_RANGE(0x4000, 0x4fff) AM_ROM
	AM_RANGE(0x6000, 0x6fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( drivfrcg_io, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x00, 0x00) AM_READ(drivfrcg_port0_r)
	AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READWRITE(input_port_4_r, MWA8_NOP)
ADDRESS_MAP_END

static ADDRESS_MAP_START( bongo, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM
	AM_RANGE(0x8400, 0x87ff) AM_WRITE(MWA8_NOP) // not used
	AM_RANGE(0x9000, 0x93ff) AM_READWRITE(MRA8_RAM, galaxian_videoram_w) AM_BASE(&galaxian_videoram)
	AM_RANGE(0x9400, 0x97ff) AM_WRITE(MWA8_NOP) // not used
	AM_RANGE(0x9800, 0x983f) AM_READWRITE(MRA8_RAM, galaxian_attributesram_w) AM_BASE(&galaxian_attributesram)
	AM_RANGE(0x9840, 0x985f) AM_RAM AM_BASE(&galaxian_spriteram) AM_SIZE(&galaxian_spriteram_size)
	AM_RANGE(0x9860, 0x987f) AM_RAM AM_BASE(&galaxian_bulletsram) AM_SIZE(&galaxian_bulletsram_size)
	AM_RANGE(0xa000, 0xa000) AM_READ(input_port_0_r)
	AM_RANGE(0xa800, 0xa800) AM_READ(input_port_1_r)
	AM_RANGE(0xb000, 0xb000) AM_READ(input_port_2_r)
	AM_RANGE(0xb001, 0xb001) AM_WRITE(galaxian_nmi_enable_w)
	AM_RANGE(0xb004, 0xb004) AM_WRITE(galaxian_stars_enable_w)
	AM_RANGE(0xb006, 0xb006) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0xb007, 0xb007) AM_WRITE(galaxian_flip_screen_y_w)
	AM_RANGE(0xb800, 0xb800) AM_READWRITE(watchdog_reset_r, MWA8_NOP)
ADDRESS_MAP_END

static ADDRESS_MAP_START( bongo_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00, 0x00) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x02, 0x02) AM_READ(AY8910_read_port_0_r)
ADDRESS_MAP_END


static ADDRESS_MAP_START( hunchbkg, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x1480, 0x14bf) AM_MIRROR(0x6000) AM_READWRITE(MRA8_RAM, galaxian_attributesram_w) AM_BASE(&galaxian_attributesram)
	AM_RANGE(0x14c0, 0x14ff) AM_MIRROR(0x6000) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_spriteram) AM_SIZE(&galaxian_spriteram_size)
	AM_RANGE(0x1500, 0x1500) AM_MIRROR(0x6000) AM_READ(input_port_0_r)
	AM_RANGE(0x1503, 0x1503) AM_MIRROR(0x6000) AM_WRITE(galaxian_coin_counter_w)
	AM_RANGE(0x1580, 0x1580) AM_MIRROR(0x6000) AM_READ(input_port_1_r)
	AM_RANGE(0x1580, 0x1582) AM_MIRROR(0x6000) AM_WRITE(galaxian_background_enable_w)
	AM_RANGE(0x1583, 0x1583) AM_MIRROR(0x6000) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x1584, 0x1587) AM_MIRROR(0x6000) AM_WRITE(galaxian_lfo_freq_w)
	AM_RANGE(0x1600, 0x1600) AM_MIRROR(0x6000) AM_READ(input_port_2_r)
	AM_RANGE(0x1600, 0x1601) AM_MIRROR(0x6000) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x1604, 0x1604) AM_MIRROR(0x6000) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x1606, 0x1606) AM_MIRROR(0x6000) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0x1607, 0x1607) AM_MIRROR(0x6000) AM_WRITE(galaxian_flip_screen_y_w)
	AM_RANGE(0x1680, 0x1680) AM_MIRROR(0x6000) AM_READWRITE(MRA8_NOP, galaxian_pitch_w)
	AM_RANGE(0x1800, 0x1bff) AM_MIRROR(0x6000) AM_WRITE(galaxian_videoram_w) AM_BASE(&galaxian_videoram)
	AM_RANGE(0x1c00, 0x1fff) AM_MIRROR(0x6000) AM_RAM
	AM_RANGE(0x2000, 0x2fff) AM_ROM
	AM_RANGE(0x4000, 0x4fff) AM_ROM
	AM_RANGE(0x6000, 0x6fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( hunchbkg_io, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(S2650_DATA_PORT,  S2650_DATA_PORT) AM_READ(MRA8_NOP) // not used
	AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READ(input_port_3_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( harem_cpu1, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x27ff) AM_RAM
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x4800, 0x4fff) AM_READWRITE(galaxian_videoram_r, galaxian_videoram_w) AM_BASE(&galaxian_videoram)
	AM_RANGE(0x5000, 0x5000) AM_WRITENOP
	AM_RANGE(0x5800, 0x5800) AM_READWRITE(MRA8_NOP, interrupt_enable_w)
	AM_RANGE(0x5801, 0x5807) AM_WRITENOP
	AM_RANGE(0x6101, 0x6101) AM_READ(input_port_0_r)
	AM_RANGE(0x6102, 0x6102) AM_READ(input_port_1_r)
	AM_RANGE(0x6103, 0x6103) AM_WRITENOP
	AM_RANGE(0x6200, 0x6203) AM_WRITENOP AM_BASE(&galaxian_attributesram)
	AM_RANGE(0x8000, 0x9fff) AM_ROM
	AM_RANGE(0xffe6, 0xffff) AM_RAM AM_BASE(&galaxian_spriteram) AM_SIZE(&galaxian_spriteram_size)
ADDRESS_MAP_END

static ADDRESS_MAP_START( harem_cpu2, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM
	AM_RANGE(0xa000, 0xa000) AM_WRITENOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( harem_cpu2_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x04, 0x04) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x08, 0x08) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x10, 0x10) AM_WRITE(AY8910_control_port_1_w)
	AM_RANGE(0x20, 0x20) AM_WRITE(AY8910_write_port_1_w)
	AM_RANGE(0x40, 0x40) AM_WRITE(AY8910_control_port_2_w)
	AM_RANGE(0x80, 0x80) AM_WRITE(AY8910_write_port_2_w)
	AM_RANGE(0x80, 0x80) AM_READ(soundlatch_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tazzmang, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x7000, 0x7000) AM_READ(input_port_2_r) /* mirror */
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0x883f) AM_READWRITE(MRA8_RAM, galaxian_attributesram_w) AM_BASE(&galaxian_attributesram)
	AM_RANGE(0x8840, 0x885f) AM_RAM AM_BASE(&galaxian_spriteram) AM_SIZE(&galaxian_spriteram_size)
	AM_RANGE(0x8860, 0x887f) AM_RAM AM_BASE(&galaxian_bulletsram) AM_SIZE(&galaxian_bulletsram_size)
	AM_RANGE(0x8880, 0x8bff) AM_WRITENOP
	AM_RANGE(0x9000, 0x93ff) AM_READWRITE(MRA8_RAM, galaxian_videoram_w) AM_BASE(&galaxian_videoram)
	AM_RANGE(0x9800, 0x9800) AM_READ(watchdog_reset_r)
	AM_RANGE(0xa000, 0xa000) AM_READ(input_port_0_r)
	AM_RANGE(0xa7ff, 0xa7ff) AM_READ(input_port_0_r) /* mirror */
	AM_RANGE(0xa800, 0xa800) AM_READ(input_port_1_r) AM_WRITE(galaxian_background_enable_w)
	AM_RANGE(0xa803, 0xa803) AM_WRITE(galaxian_coin_counter_w)
	AM_RANGE(0xa805, 0xa805) AM_WRITE(galaxian_shoot_enable_w)
	AM_RANGE(0xa806, 0xa807) AM_WRITE(galaxian_vol_w)
	AM_RANGE(0xb000, 0xb000) AM_READ(input_port_2_r)
	AM_RANGE(0xb001, 0xb001) AM_WRITE(galaxian_nmi_enable_w)
	AM_RANGE(0xb004, 0xb004) AM_WRITE(galaxian_stars_enable_w)
	AM_RANGE(0xb006, 0xb006) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0xb007, 0xb007) AM_WRITE(galaxian_flip_screen_y_w)
	AM_RANGE(0xb800, 0xb800) AM_READ(watchdog_reset_r) AM_WRITE(galaxian_pitch_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( racknrol, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x1400, 0x143f) AM_MIRROR(0x6000) AM_READWRITE(MRA8_RAM, galaxian_attributesram_w) AM_BASE(&galaxian_attributesram)
	AM_RANGE(0x1440, 0x14bf) AM_MIRROR(0x6000) AM_RAM AM_BASE(&galaxian_spriteram) AM_SIZE(&galaxian_spriteram_size)
	AM_RANGE(0x14c0, 0x14ff) AM_MIRROR(0x6000) AM_RAM
	AM_RANGE(0x1500, 0x1500) AM_MIRROR(0x6000) AM_READ(input_port_0_r)
	AM_RANGE(0x1580, 0x1580) AM_MIRROR(0x6000) AM_READ(input_port_1_r)
	AM_RANGE(0x1600, 0x1600) AM_MIRROR(0x6000) AM_READ(input_port_2_r)
	AM_RANGE(0x1600, 0x1601) AM_MIRROR(0x6000) AM_WRITENOP
	AM_RANGE(0x1606, 0x1606) AM_MIRROR(0x6000) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0x1607, 0x1607) AM_MIRROR(0x6000) AM_WRITE(galaxian_flip_screen_y_w)
	AM_RANGE(0x1680, 0x1680) AM_MIRROR(0x6000) AM_READNOP
//  AM_RANGE(0x1700, 0x1700) AM_MIRROR(0x6000) AM_READ(trvchlng_question_r)
//  AM_RANGE(0x1701, 0x1703) AM_MIRROR(0x6000) AM_READ(trvchlng_question_w)
	AM_RANGE(0x1800, 0x1bff) AM_MIRROR(0x6000) AM_WRITE(galaxian_videoram_w) AM_BASE(&galaxian_videoram)
	AM_RANGE(0x1c00, 0x1fff) AM_MIRROR(0x6000) AM_RAM
	AM_RANGE(0x2000, 0x2fff) AM_ROM
	AM_RANGE(0x4000, 0x4fff) AM_ROM
	AM_RANGE(0x6000, 0x6fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( racknrol_io, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x1d, 0x1d) AM_WRITE(SN76496_0_w)
	AM_RANGE(0x1e, 0x1e) AM_WRITE(SN76496_1_w)
	AM_RANGE(0x1f, 0x1f) AM_WRITE(SN76496_2_w)
	AM_RANGE(0x20, 0x3f) AM_WRITE(racknrol_tiles_bank_w) AM_BASE(&racknrol_tiles_bank)
	AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READ(input_port_3_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( ckongg_readmem, ADDRESS_SPACE_PROGRAM, 8 )

	AM_RANGE(0x0000, 0x5fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x6000, 0x6fff) AM_READ(MRA8_RAM)
	AM_RANGE(0x9000, 0x93ff) AM_READ(galaxian_videoram_r)
	AM_RANGE(0x9800, 0x98ff) AM_READ(MRA8_RAM)
	AM_RANGE(0xc000, 0xc000) AM_READ(input_port_0_r)
	AM_RANGE(0xc400, 0xc400) AM_READ(input_port_1_r)
	AM_RANGE(0xc800, 0xc800) AM_READ(input_port_2_r)
	AM_RANGE(0xcc00, 0xcc00) AM_READ(watchdog_reset_r)

ADDRESS_MAP_END

static ADDRESS_MAP_START( ckongg_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x6000, 0x6fff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x9000, 0x93ff) AM_WRITE(galaxian_videoram_w) AM_BASE(&galaxian_videoram)
	AM_RANGE(0x9800, 0x983f) AM_WRITE(galaxian_attributesram_w) AM_BASE(&galaxian_attributesram)
	AM_RANGE(0x9840, 0x985f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_spriteram) AM_SIZE(&galaxian_spriteram_size)
	AM_RANGE(0x9860, 0x987f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_bulletsram) AM_SIZE(&galaxian_bulletsram_size)
	AM_RANGE(0x9880, 0x98ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0xc000, 0xc001) AM_WRITE(galaxian_leds_w)
	AM_RANGE(0xc002, 0xc002) AM_WRITE(galaxian_coin_lockout_w)
	AM_RANGE(0xc003, 0xc003) AM_WRITE(galaxian_coin_counter_w)
	AM_RANGE(0xc004, 0xc007) AM_WRITE(galaxian_lfo_freq_w)
	AM_RANGE(0xc400, 0xc402) AM_WRITE(galaxian_background_enable_w)
	AM_RANGE(0xc403, 0xc403) AM_WRITE(galaxian_noise_enable_w)
	AM_RANGE(0xc405, 0xc405) AM_WRITE(galaxian_shoot_enable_w)
	AM_RANGE(0xc406, 0xc407) AM_WRITE(galaxian_vol_w)
	AM_RANGE(0xc801, 0xc801) AM_WRITE(galaxian_nmi_enable_w)
	AM_RANGE(0xc804, 0xc804) AM_WRITE(MWA8_NOP) // link cut
	AM_RANGE(0xc806, 0xc806) AM_WRITE(galaxian_flip_screen_x_w)
	AM_RANGE(0xc807, 0xc807) AM_WRITE(galaxian_flip_screen_y_w)
	AM_RANGE(0xcc00, 0xcc00) AM_WRITE(galaxian_pitch_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( kkgalax_readmem, ADDRESS_SPACE_PROGRAM, 8 )

	AM_RANGE(0x0000, 0x5fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x6000, 0x6fff) AM_READ(MRA8_RAM)
	AM_RANGE(0x9000, 0x93ff) AM_READ(galaxian_videoram_r)
	AM_RANGE(0x9800, 0x98ff) AM_READ(MRA8_RAM)
	AM_RANGE(0xa000, 0xa000) AM_READ(input_port_0_r)
	AM_RANGE(0xa800, 0xa800) AM_READ(input_port_1_r)
	AM_RANGE(0xb000, 0xb000) AM_READ(input_port_2_r)
//  AM_RANGE(0xcc00, 0xcc00) AM_READ(watchdog_reset_r)

ADDRESS_MAP_END

static ADDRESS_MAP_START( kkgalax_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x6000, 0x6fff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x9000, 0x93ff) AM_WRITE(galaxian_videoram_w) AM_BASE(&galaxian_videoram)
	AM_RANGE(0x9800, 0x983f) AM_WRITE(galaxian_attributesram_w) AM_BASE(&galaxian_attributesram)
	AM_RANGE(0x9840, 0x985f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_spriteram) AM_SIZE(&galaxian_spriteram_size)
	AM_RANGE(0x9860, 0x987f) AM_WRITE(MWA8_RAM) AM_BASE(&galaxian_bulletsram) AM_SIZE(&galaxian_bulletsram_size)
	AM_RANGE(0x9880, 0x98ff) AM_WRITE(MWA8_RAM)
//  AM_RANGE(0xc000, 0xc001) AM_WRITE(galaxian_leds_w)
//  AM_RANGE(0xc002, 0xc002) AM_WRITE(galaxian_coin_lockout_w)
//  AM_RANGE(0xc003, 0xc003) AM_WRITE(galaxian_coin_counter_w)
//  AM_RANGE(0xc004, 0xc007) AM_WRITE(galaxian_lfo_freq_w)
//  AM_RANGE(0xc400, 0xc402) AM_WRITE(galaxian_background_enable_w)
//  AM_RANGE(0xc403, 0xc403) AM_WRITE(galaxian_noise_enable_w)
//  AM_RANGE(0xc405, 0xc405) AM_WRITE(galaxian_shoot_enable_w)
//  AM_RANGE(0xc406, 0xc407) AM_WRITE(galaxian_vol_w)
	AM_RANGE(0xb001, 0xb001) AM_WRITE(galaxian_nmi_enable_w)
//  AM_RANGE(0xc804, 0xc804) AM_WRITE(MWA8_NOP) // link cut
//  AM_RANGE(0xc806, 0xc806) AM_WRITE(galaxian_flip_screen_x_w)
//  AM_RANGE(0xc807, 0xc807) AM_WRITE(galaxian_flip_screen_y_w)
//  AM_RANGE(0xcc00, 0xcc00) AM_WRITE(galaxian_pitch_w)
ADDRESS_MAP_END

static READ8_HANDLER( hexpoola_data_port_r )
{
	switch (activecpu_get_pc())
	{
		case 0x0022:
			return 0;

		case 0x0031:
			return 1;
	}

    return 0;
}

static ADDRESS_MAP_START( hexpoola_io, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x00, 0x00) AM_READNOP
	AM_RANGE(0x20, 0x3f) AM_WRITE(racknrol_tiles_bank_w) AM_BASE(&racknrol_tiles_bank)
	AM_RANGE(S2650_DATA_PORT, S2650_DATA_PORT) AM_READWRITE(hexpoola_data_port_r, SN76496_0_w)
	AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READ(input_port_3_r)
ADDRESS_MAP_END

#define GAL_IN0\
	PORT_START_TAG("IN0")\
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )\
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )\
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY\
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY\
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )\
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )\
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )\
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )\
	PORT_SERVICE( 0x40, IP_ACTIVE_HIGH )\
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1 )

#define GAL_IN1\
	PORT_START_TAG("IN1")\
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )\
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )\
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL\
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL\
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL\
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )\
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )\
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )\
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )\
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )\
	PORT_DIPSETTING(    0xc0, DEF_STR( Free_Play ) )

static INPUT_PORTS_START( galaxian )
GAL_IN0
GAL_IN1

	PORT_START_TAG("DSW0")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "7000" )
	PORT_DIPSETTING(    0x01, "10000" )
	PORT_DIPSETTING(    0x02, "12000" )
	PORT_DIPSETTING(    0x03, "20000" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( superg )
GAL_IN0
GAL_IN1

	PORT_START_TAG("DSW0")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x01, "4000" )
	PORT_DIPSETTING(    0x02, "5000" )
	PORT_DIPSETTING(    0x03, "7000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( swarm )
GAL_IN0
GAL_IN1

	PORT_START_TAG("DSW0")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x01, "10000" )
	PORT_DIPSETTING(    0x02, "20000" )
	PORT_DIPSETTING(    0x03, "40000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Difficulty ) )	/* aliens "flying" simultaneously */
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )				/* less aliens */
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )				/* more aliens */
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( gmgalax )
	PORT_START_TAG("GMIN0")      /* Ghost Muncher - IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x20, 0x00, "Ghost Muncher - Cabinet" )	PORT_CONDITION("FAKE",0x01,PORTCOND_NOTEQUALS,0x01)
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY

	PORT_START_TAG("GMIN1")      /* Ghost Muncher - IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_DIPNAME( 0xc0, 0x40, "Ghost Muncher - Bonus Life" )	PORT_CONDITION("FAKE",0x01,PORTCOND_NOTEQUALS,0x01)
	PORT_DIPSETTING(    0x40, "10000" )
	PORT_DIPSETTING(    0x80, "15000" )
	PORT_DIPSETTING(    0xc0, "20000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )

	PORT_START_TAG("GMDSW0")      /* Ghost Muncher - DSW0 */
	PORT_DIPNAME( 0x03, 0x02, "Ghost Muncher - Coinage" )	PORT_CONDITION("FAKE",0x01,PORTCOND_NOTEQUALS,0x01)
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x04, "Ghost Muncher - Lives" )	PORT_CONDITION("FAKE",0x01,PORTCOND_NOTEQUALS,0x01)
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START_TAG("IN0")      /* Galaxian - IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_DIPNAME( 0x20, 0x00, "Galaxian - Cabinet" )	PORT_CONDITION("FAKE",0x01,PORTCOND_EQUALS,0x01)
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_SERVICE( 0x40, IP_ACTIVE_HIGH )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1 )

	PORT_START_TAG("IN1")      /* Galaxian - IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0xc0, 0x00, "Galaxian - Coinage" )	PORT_CONDITION("FAKE",0x01,PORTCOND_EQUALS,0x01)
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Free_Play ) )

	PORT_START_TAG("DSW0")      /* Galaxian - DSW0 */
	PORT_DIPNAME( 0x03, 0x01, "Galaxian - Bonus Life" )	PORT_CONDITION("FAKE",0x01,PORTCOND_EQUALS,0x01)
	PORT_DIPSETTING(    0x01, "4000" )
	PORT_DIPSETTING(    0x02, "5000" )
	PORT_DIPSETTING(    0x03, "7000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x04, 0x00, "Galaxian - Lives" )	PORT_CONDITION("FAKE",0x01,PORTCOND_EQUALS,0x01)
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START_TAG("FAKE")      /* fake - game select */
	PORT_BIT( 0x01, 0x00, IPT_DIPSWITCH_NAME ) PORT_NAME("Game Select") PORT_CODE(KEYCODE_F1) PORT_TOGGLE
	PORT_DIPSETTING( 0x00, "Ghost Muncher" )
	PORT_DIPSETTING( 0x01, "Galaxian" )
INPUT_PORTS_END

static INPUT_PORTS_START( zerotime )
GAL_IN0
	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x40, "A 1C/1C 2C/2C  B 1C/2C" )
	PORT_DIPSETTING(    0xc0, "A 1C/1C 2C/3C  B 1C/3C" )
	PORT_DIPSETTING(    0x00, "A 1C/2C 2C/4C  B 1C/4C" )
	PORT_DIPSETTING(    0x80, "A 1C/2C 2C/5C  B 1C/5C" )

	PORT_START_TAG("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x03, "6000" )
	PORT_DIPSETTING(    0x02, "7000" )
	PORT_DIPSETTING(    0x01, "9000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Difficulty ) )	/* player's bullet speed */
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )				/* gap of 6 pixels */
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )				/* gap of 8 pixels */
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

#define PISCES_COMMON\
	PORT_START_TAG("IN0")\
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )\
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )\
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY\
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY\
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )\
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )\
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )\
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )\
	PORT_START_TAG("IN1")\
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )\
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )\
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL\
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL\
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL\
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )\
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Lives ) )\
	PORT_DIPSETTING(    0x00, "3" )\
	PORT_DIPSETTING(    0x40, "4" )\
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )\
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )\
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )


static INPUT_PORTS_START( pisces )
PISCES_COMMON

	PORT_START_TAG("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x01, "20000" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x02, "A 2C/1C  B 1C/3C" )
	PORT_DIPSETTING(    0x00, "A 1C/1C  B 1C/6C" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

/* Same as 'pisces', but different "Coinage" Dip Switch */
static INPUT_PORTS_START( piscesb )
PISCES_COMMON
	PORT_START_TAG("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x01, "20000" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x02, "A 2C/1C  B 1C/2C 2C/5C" )
	PORT_DIPSETTING(    0x00, "A 1C/1C  B 1C/5C" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

#define GTEIKOB_COMMON\
	PORT_START_TAG("IN0")\
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )\
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )\
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY\
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY\
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )\
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )\
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )		/* Not tested due to code removed at 0x00ab, 0x1b26 and 0x1c97*/\
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )		/* Not tested due to code removed at 0x1901*/\
	PORT_START_TAG("IN1")\
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )\
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )\
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL\
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL\
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL\
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )\
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coin_A ) )\
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )\
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )\
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coin_B ) )\
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )\
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )

static INPUT_PORTS_START( gteikokb )
GTEIKOB_COMMON
	PORT_START_TAG("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "7000" )
	PORT_DIPSETTING(    0x01, DEF_STR( None ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unused ) )		// Not read due to code at 0x012b
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

/* same as gteikokb with cabinet reversed */
static INPUT_PORTS_START( gteikob2 )
GTEIKOB_COMMON
	PORT_START_TAG("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "7000" )
	PORT_DIPSETTING(    0x01, DEF_STR( None ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unused ) )		// Not read due to code at 0x012b
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( spacbatt )
GAL_IN0

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x40, "A 2C/1C  B 1C/3C" )
	PORT_DIPSETTING(    0x00, "A 1C/1C  B 1C/6C" )
	PORT_DIPSETTING(    0x80, "A 1C/2C  B 1C/6C" )
	PORT_DIPSETTING(    0xc0, DEF_STR( Free_Play ) )

	PORT_START_TAG("DSW0")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x01, "4000" )
	PORT_DIPSETTING(    0x02, "5000" )
	PORT_DIPSETTING(    0x03, "7000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( batman2 )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )

	PORT_START_TAG("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPSETTING(    0x01, "50000" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( warofbug )
	PORT_START_TAG("IN0")
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

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Free_Play ) )

	PORT_START_TAG("DSW0")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "500000" )
	PORT_DIPSETTING(    0x00, "750000" )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( redufo )
GAL_IN0

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x40, "A 2C/1C  B 1C/3C" )
	PORT_DIPSETTING(    0x00, "A 1C/1C  B 1C/6C" )
	PORT_DIPSETTING(    0x80, "A 1C/2C  B 1C/12C" )
	PORT_DIPSETTING(    0xc0, DEF_STR( Free_Play ) )

	PORT_START_TAG("DSW0")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x01, "4000" )
	PORT_DIPSETTING(    0x02, "5000" )
	PORT_DIPSETTING(    0x03, "7000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( exodus )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_SERVICE( 0x40, IP_ACTIVE_HIGH )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )		// Not tested due to code removed at 0x1901 and 0x191a

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )

	PORT_START_TAG("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "7000" )
	PORT_DIPSETTING(    0x01, DEF_STR( None ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unused ) )		// Not read due to code at 0x012b
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( streakng )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY

	PORT_START_TAG("IN1")
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

	PORT_START_TAG("DSW0")
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

static INPUT_PORTS_START( pacmanbl )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )

	PORT_START_TAG("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "15000" )
	PORT_DIPSETTING(    0x01, "20000" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( devilfsg )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )

	PORT_START_TAG("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x01, "15000" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( zigzag )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL

	PORT_START_TAG("IN1")
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

	PORT_START_TAG("DSW0")
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

static INPUT_PORTS_START( scramblb )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL

	PORT_START_TAG("IN2")
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


/* the cocktail controls only seem to be used in upright mode, is the flip flag wrong for this bootleg? */
static INPUT_PORTS_START( scramb2 )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
 	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
    PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
 	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START_TAG("IN1")
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

	PORT_START_TAG("IN2")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
    PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
    PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( jumpbug )
	PORT_START_TAG("IN0")
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

	PORT_START_TAG("IN1")
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

	PORT_START_TAG("DSW0")
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
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)

	PORT_START_TAG("DSW0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )	/* probably unused */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  /* probably unused */
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Leave On")	/* used - MUST be ON */
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )	/* probably unused */
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )  /* probably unused */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )	/* probably unused */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )	/* probably unused */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( azurian )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL )		/* "linked" with bit 2 of IN2 */
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x80, "5" )

	PORT_START_TAG("IN2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPSETTING(    0x02, "7000" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL )		/* "linked" with bit 6 of IN1 */
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START_TAG("FAKE")      /* fake port to handle routine at 0x05b3 that stores value at 0x40f4 */
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
INPUT_PORTS_END

static INPUT_PORTS_START( orbitron )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Coinage ) )		/* Routine at 0x00e1 */
	PORT_DIPSETTING(    0x00, "A 2C/1C  B 1C/3C" )
//  PORT_DIPSETTING(    0x20, "A 2C/1C  B 1C/3C" )
	PORT_DIPSETTING(    0x40, "A 1C/1C  B 1C/6C" )
//  PORT_DIPSETTING(    0x60, "A 1C/1C  B 1C/6C" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL

	PORT_START_TAG("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( blkhole )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )

	PORT_START_TAG("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPSETTING(    0x01, "10000" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( checkman )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Tiles Right")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )

	PORT_START_TAG("IN1")
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

	PORT_START_TAG("DSW0")
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
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_COCKTAIL PORT_NAME("P2 Tiles Right")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_COCKTAIL PORT_NAME("P2 Tiles Left")
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x40, "A 2C/1C  B 1C/3C" )
	PORT_DIPSETTING(    0x00, "A 1C/1C  B 1C/6C" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START_TAG("DSW")
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
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Tiles Right")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Tiles Left")
INPUT_PORTS_END

static INPUT_PORTS_START( dingo )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL	/* 1st Button 1 */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL	/* 2nd Button 1 */
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x80, "A 1C/1C  B 1C/5C" )
	PORT_DIPSETTING(    0x00, "A 2C/1C  B 1C/3C" )

	PORT_START_TAG("DSW")
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
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )	/* Yes, the game reads both of these */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )	/* Check code at 0x22e1 */
INPUT_PORTS_END

#define MOON_IN0\
	PORT_START_TAG("IN0")\
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )\
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )\
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY\
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY\
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )\
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )\
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )\
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )\
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* "reset" on schematics */\
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

static INPUT_PORTS_START( mooncrst )
MOON_IN0
	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPSETTING(    0x40, "50000" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x80, DEF_STR( English ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Japanese ) )

	PORT_START_TAG("DSW")
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
MOON_IN0
	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPSETTING(    0x40, "50000" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x80, DEF_STR( English ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Japanese ) )

	PORT_START_TAG("DSW")
//  PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )           Not used due to code at 0x01c0
//  PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
//  PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
//  PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Free_Play ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( mooncrsg )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* "reset" on schematics */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPSETTING(    0x40, "50000" )
//  PORT_DIPNAME( 0x80, 0x80, DEF_STR( Language ) )         Always non-Japanese due to code at 0x2f77
//  PORT_DIPSETTING(    0x80, DEF_STR( English ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( English ) )

	PORT_START_TAG("DSW")
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

static INPUT_PORTS_START( fantazia )
MOON_IN0
	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPSETTING(    0x40, "50000" )
//  PORT_DIPNAME( 0x80, 0x80, DEF_STR( Language ) )         Always non-Japanese due to code at 0x2f53
//  PORT_DIPSETTING(    0x80, DEF_STR( English ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Japanese ) )

	PORT_START_TAG("DSW")
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
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( eagle )
MOON_IN0
	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPSETTING(    0x40, "50000" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x80, DEF_STR( English ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Japanese ) )

	PORT_START_TAG("DSW")
//  PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )           Not used due to code at 0x01c0
//  PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
//  PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
//  PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Free_Play ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( eagle2 )
MOON_IN0
	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPSETTING(    0x40, "50000" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x80, DEF_STR( English ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Japanese ) )

	PORT_START_TAG("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
//  PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) )           Not used due to code at 0x01c0,
//  PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )            but "Free Play" is checked
//  PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
//  PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
//  PORT_DIPSETTING(    0x0c, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( smooncrs )
MOON_IN0
	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )			/* Not read due to code at 0x2b1c and 0x3313 */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )			/* Not read due to code at 0x2b1c and 0x3313 */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )			/* Not read due to code at 0x2b1c and 0x3313 */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
//  PORT_DIPNAME( 0x40, 0x00, DEF_STR( Bonus_Life ) )       Always '50000' due to code at 0x2f68
//  PORT_DIPSETTING(    0x00, "30000" )
//  PORT_DIPSETTING(    0x40, "50000" )
//  PORT_DIPNAME( 0x80, 0x80, DEF_STR( Language ) )         Always non-Japanese due to code at 0x2f53
//  PORT_DIPSETTING(    0x80, DEF_STR( English ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Japanese ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )		/* code at 0x2962 */
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x80, "Player's Bullet Speed" )		/* code at 0x0007 */
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x80, "Fast" )

	PORT_START_TAG("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )			/* see notes */
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

static INPUT_PORTS_START( spcdrag )
MOON_IN0
	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )			/* Not read due to code at 0x2b1c and 0x3313 */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )			/* Not read due to code at 0x2b1c and 0x3313 */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )			/* Not read due to code at 0x2b1c and 0x3313 */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
//  PORT_DIPNAME( 0x40, 0x00, DEF_STR( Bonus_Life ) )       Always '50000' due to code at 0x2f68
//  PORT_DIPSETTING(    0x00, "30000" )
//  PORT_DIPSETTING(    0x40, "50000" )
//  PORT_DIPNAME( 0x80, 0x80, DEF_STR( Language ) )         Always non-Japanese due to code at 0x2f53
//  PORT_DIPSETTING(    0x80, DEF_STR( English ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Japanese ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )		/* code at 0x2962 */
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x80, "Player's Bullet Speed" )		/* code at 0x0007 */
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x80, "Fast" )

	PORT_START_TAG("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )			/* see notes */
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Free_Play ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( mooncrgx )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )

	PORT_START_TAG("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x01, "30000" )
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
//  PORT_DIPNAME( 0x04, 0x04, DEF_STR( Language ) )         Always non-Japanese due to code removed at 0x2f4b
//  PORT_DIPSETTING(    0x04, DEF_STR( English ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Japanese ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( skybase )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
 	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x80, "1C/1C (2 to start)" )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Free_Play ) )

	PORT_START_TAG("IN2")
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

static INPUT_PORTS_START( omega )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )

	PORT_START_TAG("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x01, "20000" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( moonqsr )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* "reset" on schematics */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Hardest ) )

	PORT_START_TAG("DSW")
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

static INPUT_PORTS_START( moonal2 )
GAL_IN0
GAL_IN1
	PORT_START_TAG("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x01, "4000" )
	PORT_DIPSETTING(    0x02, "5000" )
	PORT_DIPSETTING(    0x03, "7000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( mshuttle )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START_TAG("IN1")
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

	PORT_START_TAG("DSW")
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
GAL_IN0
	/* Relating to above port:Hack? - possibly multiplexed via writes to $b003 */
	//PORT_DIPNAME( 0x80, 0x80, "Speech" )
	//PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	//PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL )	/* NOISE line */
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )

	PORT_START_TAG("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x01, "12000" )
	PORT_DIPSETTING(    0x02, "15000" )
	PORT_DIPSETTING(    0x03, DEF_STR( None ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPNAME( 0xf8, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0xf8, DEF_STR( On ) )

	PORT_START_TAG("FAKE")
	/* Hack? - possibly multiplexed via writes to $b003 - marked as SLAM */
	PORT_DIPNAME( 0x01, 0x01, "Speech" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( scorpnmc )
	PORT_START_TAG("IN0")      /* 0xa000 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )		// COIN2? (it ALWAYS adds 1 credit)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY

	PORT_START_TAG("IN1")      /* 0xa800 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start 1 / P1 Button 1")		/* also P1 Button 1 */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("Start 2 / P1 Button 2")		/* also P1 Button 2 */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Difficulty ) )	// Check code at 0x0118
	PORT_DIPSETTING(	0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(	0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(	0xc0, DEF_STR( Hardest ) )

	PORT_START_TAG("DSW0")      /* DSW0? - 0xb001 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Lives ) )		// Check code at 0x00eb
	PORT_DIPSETTING(	0x00, "2" )
	PORT_DIPSETTING(	0x04, "3" )
	PORT_DIPSETTING(	0x08, "4" )
	PORT_DIPSETTING(	0x0c, "5" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START_TAG("DSW1")      /* DSW1? - 0xb002 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )		// Check code at 0x00fe
	PORT_DIPSETTING(	0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(	0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x06, 0x02, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( frogg )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_SPECIAL )		// See notes
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_SPECIAL )		// See notes
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )
	PORT_DIPSETTING(	0xc0, "3" )
	PORT_DIPSETTING(	0x40, "5" )
	PORT_DIPSETTING(	0x80, "7" )
	PORT_DIPSETTING(	0x00, "255 (Cheat)")

	PORT_START_TAG("DSW0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )		// also affects coinage (see 'res' intruction at 0x3084)
	PORT_DIPSETTING(	0x01, DEF_STR( Upright ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unused ) )		// not tested due to code at 0x3084
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Coinage ) )		// when "Cabinet" Dip Switch set to "Upright"
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )		// "A 1/1 B 1/6" if "Cabinet" Dip Switch set to "Cocktail"
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )		// "A 2/1 B 1/3" if "Cabinet" Dip Switch set to "Cocktail"
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( 4in1 )
	PORT_START_TAG("IN0")
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

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_SPECIAL )	// See fake ports

	PORT_START_TAG("DSW0")
	PORT_BIT( 0x3b, IP_ACTIVE_HIGH, IPT_SPECIAL )	// See fake ports
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )			// 2 when continue (Scramble PT2)
	PORT_DIPSETTING(    0x04, "5" )			// 2 when continue (Scramble PT2)
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START_TAG("FAKE1")      /* The Ghost Muncher PT3 - FAKE DSW0 (bits 0 to 5) and IN1 (bits 6 and 7) */
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

	PORT_START_TAG("FAKE2")      /* Scramble PT2 - FAKE DSW0 (bits 0 to 5) and IN1 (bits 6 and 7) */
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

	PORT_START_TAG("FAKE3")      /* Galaxian PT5 - FAKE DSW0 (bits 0 to 5) and IN1 (bits 6 and 7) */
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

	PORT_START_TAG("FAKE4")      /* Galactic Convoy - FAKE DSW0 (bits 0 to 5) and IN1 (bits 6 and 7) */
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

static INPUT_PORTS_START( bagmanmc )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(	0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Cocktail ) )

	PORT_START_TAG("DSW")
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
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(	0x40, "30000" )
	PORT_DIPSETTING(	0x00, "40000" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )	// Check code at 0x2d78 and 0x2e6b
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( dkongjrm )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(	0xc0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(	0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x80, DEF_STR( 1C_2C ) )

	PORT_START_TAG("IN2")
	PORT_DIPNAME( 0x01, 0x00, "Coin Multiplier" )
	PORT_DIPSETTING(    0x00, "*1" )
	PORT_DIPSETTING(    0x01, "*2" )
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(	0x00, "3" )
	PORT_DIPSETTING(	0x02, "4" )
	PORT_DIPSETTING(	0x04, "5" )
	PORT_DIPSETTING(	0x06, "6" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(	0x08, DEF_STR( Upright ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( rockclim )

	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY
	PORT_DIPNAME( 0x40, 0x40, "Unknown 1" ) //code at $7713
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x40, "1" )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT  ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT  ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "30,000" )
	PORT_DIPSETTING(    0x40, "50,000" )
	PORT_DIPNAME( 0x80, 0x80, "Coin Slots" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x80, "1" )

	PORT_START_TAG("DSW0")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )

	PORT_START_TAG("DSW1")
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

/*

Game bug:
- you can insert 99 credits 3 times consecutively, then it resets

*/
static INPUT_PORTS_START( ozon1 )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START_TAG("IN1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START_TAG("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, "A 1C/1C  B 2C/1C" )
	PORT_DIPSETTING(    0x02, "A 1C/2C  B 1C/1C" ) // when you insert a coin with COIN2 it starts an infinite loop
	PORT_DIPSETTING(    0x04, "A 1C/3C  B 3C/1C" )
	PORT_DIPSETTING(    0x06, "A 1C/4C  B 4C/1C" )
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
INPUT_PORTS_END

static INPUT_PORTS_START( ladybugg )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )

	PORT_START_TAG("IN1")
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

	PORT_START_TAG("DSW")
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
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) /* Yes, the game reads both of these */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) /* Check code at 0x22e1 */
INPUT_PORTS_END

static INPUT_PORTS_START( vpool )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x40, IP_ACTIVE_HIGH )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
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

static INPUT_PORTS_START( drivfrcg )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START_TAG("IN1")
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

	PORT_START_TAG("DSW0")
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

	PORT_START_TAG("DSW1")
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

	PORT_START_TAG("SENSE")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )
INPUT_PORTS_END

static INPUT_PORTS_START( bongo )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // coin? it jumps to an unmapped area at $C003
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START_TAG("DSW0")
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

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x06, "5" )
	PORT_DIPNAME( 0x08, 0x00, "Infinite Lives" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static INPUT_PORTS_START( hunchbkg )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Start 1 / P1 Button 1") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Start 2 / P1 Button 1") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START_TAG("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
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

	PORT_START_TAG("SENSE")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )
INPUT_PORTS_END

static INPUT_PORTS_START( harem )
	PORT_START_TAG("IN0")//Change tag when major usage uncovered.
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

	PORT_START_TAG("IN1")
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

static INPUT_PORTS_START( tazzmang )
	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start 1 / P1 and P2 Button 2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL

	PORT_START
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x06, 0x04, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, "A 4C/1C  B 1C/4C" )
	PORT_DIPSETTING(    0x02, "A 3C/1C  B 1C/3C" )
	PORT_DIPSETTING(    0x06, "A 2C/1C  B 1C/2C" )
	PORT_DIPSETTING(    0x04, "A 1C/1C  B 1C/1C" )
	PORT_DIPNAME( 0x08, 0x00, "3" )
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
INPUT_PORTS_END

static INPUT_PORTS_START( racknrol )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY

	PORT_START_TAG("IN1")
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

	PORT_START_TAG("DSW0")
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

	PORT_START_TAG("SENSE")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_VBLANK )
INPUT_PORTS_END

static INPUT_PORTS_START( trvchlng )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON4)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 )

	PORT_START_TAG("IN1")
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

	PORT_START_TAG("DSW0")
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

	PORT_START_TAG("SENSE")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_VBLANK )
INPUT_PORTS_END

static INPUT_PORTS_START( catacomb )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )

	PORT_START_TAG("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
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

	PORT_START_TAG("SENSE")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_VBLANK )
INPUT_PORTS_END

static INPUT_PORTS_START( luctoday )
   PORT_START_TAG("IN0") //These inputs are clearly wrong, they need a full test
   PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
   PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
   PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )PORT_2WAY PORT_NAME("Add Credit to Bet")
   PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )PORT_2WAY PORT_NAME("Remove Credit from Bet")
   PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
   PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
   PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
   PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BILL1 )

   PORT_START_TAG("IN1")
   PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
   PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
   PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
   PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
   PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
   PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
   PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
   PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

   PORT_START_TAG("DSW0")
   PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
   PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
   PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
   PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
   PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( ckongg )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
//      PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Free_Play ) )

	PORT_START_TAG("DSW0")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "500000" )
	PORT_DIPSETTING(    0x00, "750000" )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

/* not correct */
static INPUT_PORTS_START( kkgalax )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) // button 1 and start 1
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL // button 1 and start 2?
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Free_Play ) )

	PORT_START_TAG("DSW0")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "500000" )
	PORT_DIPSETTING(    0x00, "750000" )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( porter )
	PORT_START_TAG("IN0")\
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
    PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) // and START
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) // and START2
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
    PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW")
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

static const gfx_layout galaxian_charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};
static const gfx_layout galaxian_spritelayout =
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

static const gfx_layout pacmanbl_charlayout =
{
	8,8,
	256,
	2,
	{ 0, 256*8*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
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
	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, galaxian_charlayout,   32, 8 )
	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, galaxian_spritelayout, 32, 8 )
	GFXDECODE_ENTRY( REGION_GFX2, 0x0000, rockclim_charlayout, 0, 1 )
GFXDECODE_END




static GFXDECODE_START( galaxian )
	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, galaxian_charlayout,   0, 8 )
	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, galaxian_spritelayout, 0, 8 )
GFXDECODE_END

static GFXDECODE_START( gmgalax )
	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, galaxian_charlayout,   0, 16 )
	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, galaxian_spritelayout, 0, 16 )
GFXDECODE_END

/* separate character and sprite ROMs */
static GFXDECODE_START( pacmanbl )
	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, pacmanbl_charlayout,   0, 8 )
	GFXDECODE_ENTRY( REGION_GFX1, 0x1000, pacmanbl_spritelayout, 0, 8 )
GFXDECODE_END

static GFXDECODE_START( bagmanmc )
	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, bagmanmc_charlayout,    0, 8 )
	GFXDECODE_ENTRY( REGION_GFX1, 0x2000, pacmanbl_spritelayout, 0, 8 )
GFXDECODE_END

static GFXDECODE_START( _4in1 )
	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, _4in1_charlayout,      0, 8 )
	GFXDECODE_ENTRY( REGION_GFX1, 0x4000, _4in1_spritelayout,    0, 8 )
GFXDECODE_END

static const struct AY8910interface checkmaj_ay8910_interface =
{
	soundlatch_r
};

static const struct AY8910interface bongo_ay8910_interface =
{
	input_port_3_r
};


static MACHINE_DRIVER_START( galaxian_base )

	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main", Z80, PIXEL_CLOCK/2)	/* 3.072 MHz */
	MDRV_CPU_PROGRAM_MAP(galaxian_readmem,galaxian_writemem)

	MDRV_MACHINE_RESET(galaxian)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_GFXDECODE(galaxian)
	MDRV_PALETTE_LENGTH(32+2+64)		/* 32 for the characters, 2 for the bullets, 64 for the stars */

	MDRV_SCREEN_ADD("main", 0)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)

	MDRV_PALETTE_INIT(galaxian)
	MDRV_VIDEO_START(galaxian)
	MDRV_VIDEO_UPDATE(galaxian)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( galaxian )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian_base)

	/* sound hardware */
	MDRV_SOUND_ADD(SAMPLES, 0)
	MDRV_SOUND_CONFIG(galaxian_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( gmgalax )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(gmgalax_readmem,galaxian_writemem)
	MDRV_CPU_VBLANK_INT(gmgalax_vh_interrupt,1)

	/* video hardware */
	MDRV_GFXDECODE(gmgalax)
	MDRV_PALETTE_LENGTH(64+2+64)		/* 64 for the characters, 2 for the bullets, 64 for the stars */

	MDRV_VIDEO_START(gmgalax)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( pisces )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian)

	/* video hardware */
	MDRV_VIDEO_START(pisces)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( gteikob2 )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian)

	/* video hardware */
	MDRV_VIDEO_START(gteikob2)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( batman2 )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian)

	/* video hardware */
	MDRV_VIDEO_START(batman2)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( mooncrgx )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian)

	/* video hardware */
	MDRV_VIDEO_START(mooncrgx)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( pacmanbl )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian)

	/* video hardware */
	MDRV_GFXDECODE(pacmanbl)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( devilfsg )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian)
	MDRV_CPU_MODIFY("main")

	MDRV_MACHINE_RESET(devilfsg)

	/* video hardware */
	MDRV_GFXDECODE(pacmanbl)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( mooncrst )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(mooncrst_readmem,mooncrst_writemem)

	/* video hardware */
	MDRV_VIDEO_START(mooncrst)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( skybase )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(mooncrst_readmem,skybase_writemem)

	/* video hardware */
	MDRV_VIDEO_START(pisces)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( moonqsr )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(mooncrst_readmem,mooncrst_writemem)

	/* video hardware */
	MDRV_VIDEO_START(moonqsr)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( mshuttle )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian_base)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(mooncrst_readmem,mshuttle_writemem)
	MDRV_CPU_IO_MAP(mshuttle_readport,mshuttle_writeport)

	MDRV_MACHINE_RESET(devilfsg)

	/* video hardware */
	MDRV_VIDEO_START(mshuttle)

	/* sound hardware */
	MDRV_SOUND_ADD(AY8910, PIXEL_CLOCK/4)
	MDRV_SOUND_CONFIG(cclimber_ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD(SAMPLES, 0)
	MDRV_SOUND_CONFIG(cclimber_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( scramblb )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(scramblb_readmem,scramblb_writemem)

	/* video hardware */
	MDRV_PALETTE_LENGTH(32+2+64+1)	/* 32 for the characters, 2 for the bullets, 64 for the stars, 1 for background */

	MDRV_PALETTE_INIT(scramble)
	MDRV_VIDEO_START(scramble)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( scramb2 )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(scramb2_readmem,scramb2_writemem)

	/* video hardware */
	MDRV_PALETTE_LENGTH(32+2+64+1)	/* 32 for the characters, 2 for the bullets, 64 for the stars, 1 for background */

	MDRV_PALETTE_INIT(scramble)
	MDRV_VIDEO_START(scramble)
MACHINE_DRIVER_END



static MACHINE_DRIVER_START( zigzag )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian_base)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(zigzag_readmem,zigzag_writemem)

	/* video hardware */
	MDRV_GFXDECODE(pacmanbl)
	MDRV_PALETTE_LENGTH(32+2+64)	/* 32 for the characters, 2 for the bullets, 64 for the stars */


	MDRV_VIDEO_START(galaxian_plain)

	/* sound hardware */
	MDRV_SOUND_ADD(AY8910, 1789750)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( jumpbug )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian_base)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(jumpbug_readmem,jumpbug_writemem)

	/* video hardware */
	MDRV_PALETTE_LENGTH(32+2+64)	/* 32 for the characters, 2 for the bullets, 64 for the stars */

	MDRV_VIDEO_START(jumpbug)

	/* sound hardware */
	MDRV_SOUND_ADD(AY8910, 1789750)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( azurian )
	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian)

	/* video hardware */
	MDRV_VIDEO_START(azurian)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( checkman )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian_base)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(mooncrst_readmem,checkman_writemem)
	MDRV_CPU_IO_MAP(0,checkman_writeport)

	MDRV_CPU_ADD(Z80, 1620000)
	/* audio CPU */	/* 1.62 MHz */
	MDRV_CPU_PROGRAM_MAP(checkman_sound_readmem,checkman_sound_writemem)
	MDRV_CPU_IO_MAP(checkman_sound_readport,checkman_sound_writeport)
	MDRV_CPU_VBLANK_INT(irq0_line_hold,1)	/* NMIs are triggered by the main CPU */

	/* video hardware */
	MDRV_PALETTE_LENGTH(32+2+64)	/* 32 for the characters, 2 for the bullets, 64 for the stars */

	MDRV_VIDEO_START(mooncrst)

	/* sound hardware */
	MDRV_SOUND_ADD(AY8910, 1789750)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD(SAMPLES, 0)
	MDRV_SOUND_CONFIG(galaxian_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( checkmaj )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian_base)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(galaxian_readmem,checkmaj_writemem)

	MDRV_CPU_ADD(Z80, 1620000)
	/* audio CPU */
	MDRV_CPU_PROGRAM_MAP(checkmaj_sound_readmem,checkmaj_sound_writemem)
	MDRV_CPU_VBLANK_INT(irq0_line_hold,32)	/* NMIs are triggered by the main CPU */


	/* video hardware */
	MDRV_PALETTE_LENGTH(32+2+64)	/* 32 for the characters, 2 for the bullets, 64 for the stars */

	/* sound hardware */
	MDRV_SOUND_ADD(AY8910, 1620000)
	MDRV_SOUND_CONFIG(checkmaj_ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( dingoe )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian_base)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(mooncrst_readmem,checkman_writemem)
	MDRV_CPU_IO_MAP(0,checkman_writeport)

	MDRV_CPU_ADD(Z80, 1620000)
	/* audio CPU */	/* 1.62 MHz */
	MDRV_CPU_PROGRAM_MAP(checkman_sound_readmem,checkman_sound_writemem)
	MDRV_CPU_IO_MAP(checkman_sound_readport,checkman_sound_writeport)
	MDRV_CPU_VBLANK_INT(irq0_line_hold,1)	/* NMIs are triggered by the main CPU */

	/* video hardware */
	MDRV_PALETTE_LENGTH(32+2+64)	/* 32 for the characters, 2 for the bullets, 64 for the stars */

	/* sound hardware */
	MDRV_SOUND_ADD(AY8910, 1789750)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( kingball )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(mooncrst_readmem,kingball_writemem)

	MDRV_CPU_ADD(Z80,5000000/2)
	/* audio CPU */	/* 2.5 MHz */
	MDRV_CPU_PROGRAM_MAP(kingball_sound_readmem,kingball_sound_writemem)
	MDRV_CPU_IO_MAP(kingball_sound_readport,kingball_sound_writeport)

	/* video hardware */
	MDRV_PALETTE_LENGTH(32+2+64)	/* 32 for the characters, 2 for the bullets, 64 for the stars */

	/* sound hardware */
	MDRV_SOUND_ADD_TAG("DAC", DAC, 0)
	MDRV_SOUND_ROUTE(0, "filter", 1.0)

	MDRV_SOUND_ADD_TAG("filter", FILTER_RC, 0)
	MDRV_SOUND_CONFIG(flt_rc_ac_default)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( scorpnmc )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(scorpnmc_readmem,scorpnmc_writemem)

	/* video hardware */
	MDRV_VIDEO_START(batman2)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( frogg )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian)

	/* video hardware */
	MDRV_PALETTE_LENGTH(32+64+2+1)  /* 32 for characters, 64 for stars, 2 for bullets, 1 for background */

	MDRV_PALETTE_INIT(frogger)
	MDRV_VIDEO_START(froggers)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( 4in1 )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(_4in1_readmem,_4in1_writemem)

	/* video hardware */
	MDRV_GFXDECODE(_4in1)

	MDRV_VIDEO_START(pisces)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( bagmanmc )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(bagmanmc_readmem,bagmanmc_writemem)

	MDRV_MACHINE_RESET( devilfsg )

	/* video hardware */
	MDRV_GFXDECODE(bagmanmc)

	MDRV_VIDEO_START(pisces)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( dkongjrm )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(dkongjrm_readmem,dkongjrm_writemem)

	/* video hardware */
	MDRV_VIDEO_START(dkongjrm)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( rockclim )
	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(rockclim_readmem,rockclim_writemem)
	MDRV_GFXDECODE(rockclim)
	/* video hardware */
	MDRV_VIDEO_START(rockclim)
	MDRV_PALETTE_LENGTH(64+64+2)	/* 64 colors only, but still uses bullets so we need to keep the palette big */
	MDRV_PALETTE_INIT(rockclim)
	MDRV_SCREEN_SIZE(64*8, 32*8)

MACHINE_DRIVER_END

static MACHINE_DRIVER_START( ozon1 )
	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian_base)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(ozon1_readmem,ozon1_writemem)
	MDRV_CPU_IO_MAP(0,ozon1_writeport)
	MDRV_CPU_VBLANK_INT(nmi_line_pulse,1)

	MDRV_MACHINE_RESET(NULL)

	MDRV_PALETTE_INIT(rockclim)
	MDRV_PALETTE_LENGTH(32)

	MDRV_VIDEO_START(galaxian_plain)
	MDRV_SOUND_ADD(AY8910, 1789750)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( drivfrcg )

	/* basic machine hardware */
	MDRV_CPU_ADD(S2650, 18432000/6)
	MDRV_CPU_PROGRAM_MAP(drivfrcg,0)
	MDRV_CPU_IO_MAP(drivfrcg_io,0)
	MDRV_CPU_VBLANK_INT(hunchbks_vh_interrupt,1)

	MDRV_SCREEN_REFRESH_RATE(16000.0/132/2)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_REAL_60HZ_VBLANK_DURATION)

	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_PALETTE_LENGTH(64)
	MDRV_GFXDECODE(gmgalax)

	MDRV_PALETTE_INIT(rockclim)

	MDRV_VIDEO_START(drivfrcg)
	MDRV_VIDEO_UPDATE(galaxian)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD(SAMPLES, 0)
	MDRV_SOUND_CONFIG(galaxian_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( bongo )
	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian_base)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(bongo,0)
	MDRV_CPU_IO_MAP(bongo_io,0)

	MDRV_VIDEO_START(bongo)
	MDRV_VIDEO_UPDATE(galaxian)

	MDRV_SOUND_ADD(AY8910, 1789750)
	MDRV_SOUND_CONFIG(bongo_ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( hunchbkg )

	/* basic machine hardware */
	MDRV_CPU_ADD(S2650, PIXEL_CLOCK/2)
	MDRV_CPU_PROGRAM_MAP(hunchbkg,0)
	MDRV_CPU_IO_MAP(hunchbkg_io,0)
	MDRV_CPU_VBLANK_INT(hunchbks_vh_interrupt,1)

	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_GFXDECODE(galaxian)
	MDRV_PALETTE_LENGTH(32+2+64)		/* 32 for the characters, 2 for the bullets, 64 for the stars */

	MDRV_SCREEN_ADD("main", 0)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)

	MDRV_PALETTE_INIT(galaxian)

	MDRV_VIDEO_UPDATE(galaxian)
	MDRV_VIDEO_START(galaxian_plain)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD(SAMPLES, 0)
	MDRV_SOUND_CONFIG(galaxian_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( harem )
	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian_base)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(harem_cpu1,0)
	MDRV_CPU_VBLANK_INT(nmi_line_pulse,1)

	MDRV_CPU_ADD(Z80, 1620000)
	/* audio CPU */
	MDRV_CPU_PROGRAM_MAP(harem_cpu2,0)
	MDRV_CPU_IO_MAP(harem_cpu2_io,0)

	MDRV_MACHINE_RESET(NULL)

	MDRV_PALETTE_INIT(rockclim)
	MDRV_PALETTE_LENGTH(32)

	MDRV_VIDEO_START(galaxian_plain)

	MDRV_SOUND_ADD(AY8910, 2000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.33/3)

	MDRV_SOUND_ADD(AY8910, 2000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.33/3)

	MDRV_SOUND_ADD(AY8910, 2000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.33/3)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( tazzmang )
	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(tazzmang,0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( racknrol )

	/* basic machine hardware */
	MDRV_CPU_ADD(S2650, PIXEL_CLOCK/2)
	MDRV_CPU_PROGRAM_MAP(racknrol,0)
	MDRV_CPU_IO_MAP(racknrol_io,0)
	MDRV_CPU_VBLANK_INT(hunchbks_vh_interrupt,1)

	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_GFXDECODE(galaxian)
	MDRV_PALETTE_LENGTH(32)

	MDRV_SCREEN_ADD("main", 0)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)

	MDRV_PALETTE_INIT(rockclim)
	MDRV_VIDEO_UPDATE(galaxian)
	MDRV_VIDEO_START(racknrol)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD(SN76496, PIXEL_CLOCK/2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD(SN76496, PIXEL_CLOCK/2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD(SN76496, PIXEL_CLOCK/2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( ckongg )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(ckongg_readmem,ckongg_writemem)

	MDRV_GFXDECODE(gmgalax)

	MDRV_VIDEO_START(ckongs)

MACHINE_DRIVER_END

static MACHINE_DRIVER_START( kkgalax )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(galaxian)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(kkgalax_readmem,kkgalax_writemem)

	MDRV_GFXDECODE(gmgalax)

	MDRV_VIDEO_START(ckongs)

MACHINE_DRIVER_END


static MACHINE_DRIVER_START( hexpoola )

	/* basic machine hardware */
	MDRV_CPU_ADD(S2650, PIXEL_CLOCK/2)
	MDRV_CPU_PROGRAM_MAP(racknrol,0)
	MDRV_CPU_IO_MAP(hexpoola_io,0)
	MDRV_CPU_VBLANK_INT(hunchbks_vh_interrupt,1)

	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_GFXDECODE(galaxian)
	MDRV_PALETTE_LENGTH(32)

	MDRV_SCREEN_ADD("main", 0)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)

	MDRV_PALETTE_INIT(rockclim)
	MDRV_VIDEO_UPDATE(galaxian)
	MDRV_VIDEO_START(racknrol)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD(SN76496, PIXEL_CLOCK/2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( galaxian )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "galmidw.u",    0x0000, 0x0800, CRC(745e2d61) SHA1(e65f74e35b1bfaccd407e168ea55678ae9b68edf) )
	ROM_LOAD( "galmidw.v",    0x0800, 0x0800, CRC(9c999a40) SHA1(02fdcd95d8511e64c0d2b007b874112d53e41045) )
	ROM_LOAD( "galmidw.w",    0x1000, 0x0800, CRC(b5894925) SHA1(0046b9ed697a34d088de1aead8bd7cbe526a2396) )
	ROM_LOAD( "galmidw.y",    0x1800, 0x0800, CRC(6b3ca10b) SHA1(18d8714e5ef52f63ba8888ecc5a25b17b3bf17d1) )
	ROM_LOAD( "7l",           0x2000, 0x0800, CRC(1b933207) SHA1(8b44b0f74420871454e27894d0f004859f9e59a9) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "1h.bin",       0x0000, 0x0800, CRC(39fb43a4) SHA1(4755609bd974976f04855d51e08ec0d62ab4bc07) )
	ROM_LOAD( "1k.bin",       0x0800, 0x0800, CRC(7e3f56a2) SHA1(a9795d8b7388f404f3b0e2c6ce15d713a4c5bafa) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( galaxiaj )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "7f.bin",       0x0000, 0x1000, CRC(4335b1de) SHA1(e41e3d90dac738cf71377f3b476ec67b14dee27a) )
	ROM_LOAD( "7j.bin",       0x1000, 0x1000, CRC(4e6f66a1) SHA1(ee2a675ab34485c0f58c51be7630a51e27a7a8f3) )
	ROM_LOAD( "7l.bin",       0x2000, 0x0800, CRC(5341d75a) SHA1(40bc8fcc598f58c6ff944e2a4a9288463e75a09d) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "1h.bin",       0x0000, 0x0800, CRC(39fb43a4) SHA1(4755609bd974976f04855d51e08ec0d62ab4bc07) )
	ROM_LOAD( "1k.bin",       0x0800, 0x0800, CRC(7e3f56a2) SHA1(a9795d8b7388f404f3b0e2c6ce15d713a4c5bafa) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( galmidw )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "galmidw.u",    0x0000, 0x0800, CRC(745e2d61) SHA1(e65f74e35b1bfaccd407e168ea55678ae9b68edf) )
	ROM_LOAD( "galmidw.v",    0x0800, 0x0800, CRC(9c999a40) SHA1(02fdcd95d8511e64c0d2b007b874112d53e41045) )
	ROM_LOAD( "galmidw.w",    0x1000, 0x0800, CRC(b5894925) SHA1(0046b9ed697a34d088de1aead8bd7cbe526a2396) )
	ROM_LOAD( "galmidw.y",    0x1800, 0x0800, CRC(6b3ca10b) SHA1(18d8714e5ef52f63ba8888ecc5a25b17b3bf17d1) )
	ROM_LOAD( "galmidw.z",    0x2000, 0x0800, CRC(cb24f797) SHA1(e6bb977ded0654c2c7388aad188059e1e0647908) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "galaxian.j1",  0x0000, 0x0800, CRC(84decf98) SHA1(2e565cb6057b1816a6b4541e6dfadd3c3762fa36) )
	ROM_LOAD( "galaxian.l1",  0x0800, 0x0800, CRC(c31ada9e) SHA1(237ebb48549b34ca59a13cc2706512d957413ec4) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( galmidwo )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "galaxian.u",   0x0000, 0x0800, CRC(fac42d34) SHA1(0b96d9f1c6bf0e0b7f757dcbaeacfbfafefc54d1) )
	ROM_LOAD( "galaxian.v",   0x0800, 0x0800, CRC(f58283e3) SHA1(edc6e72516c50fd3402281d9936574d276581ce9) )
	ROM_LOAD( "galaxian.w",   0x1000, 0x0800, CRC(4c7031c0) SHA1(97f7ab0cedcd8eba1c8f6f516d84d672a2108258) )
	ROM_LOAD( "galaxian.y",   0x1800, 0x0800, CRC(96a7ac94) SHA1(c3c7a43117c8b9fd8621823c872889f8e31bf935) )
	ROM_LOAD( "7l.bin",       0x2000, 0x0800, CRC(5341d75a) SHA1(40bc8fcc598f58c6ff944e2a4a9288463e75a09d) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "galaxian.j1",  0x0000, 0x0800, CRC(84decf98) SHA1(2e565cb6057b1816a6b4541e6dfadd3c3762fa36) )
	ROM_LOAD( "galaxian.l1",  0x0800, 0x0800, CRC(c31ada9e) SHA1(237ebb48549b34ca59a13cc2706512d957413ec4) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( superg )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "7f.bin",       0x0000, 0x1000, CRC(4335b1de) SHA1(e41e3d90dac738cf71377f3b476ec67b14dee27a) )
	ROM_LOAD( "superg.w",     0x1000, 0x0800, CRC(ddeabdae) SHA1(daa5109a32c7c9a80bdb212dc3e4e3e3c104a731) )
	ROM_LOAD( "superg.y",     0x1800, 0x0800, CRC(9463f753) SHA1(d9cb35c19aafec43d08b048bbe2337a790f6ba9d) )
	ROM_LOAD( "superg.z",     0x2000, 0x0800, CRC(e6312e35) SHA1(c4010459379d7fe00f605aaf288928b2deffb8b2) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "galmidw.1j",   0x0000, 0x0800, CRC(84decf98) SHA1(2e565cb6057b1816a6b4541e6dfadd3c3762fa36) )
	ROM_LOAD( "galmidw.1k",   0x0800, 0x0800, CRC(c31ada9e) SHA1(237ebb48549b34ca59a13cc2706512d957413ec4) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( galapx )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "galx.u",       0x0000, 0x0800, CRC(79e4007d) SHA1(d55050498a670d1c022ba3caad34f8fcaccf4a30) )
	ROM_LOAD( "galx.v",       0x0800, 0x0800, CRC(bc16064e) SHA1(4e3220fd63c8184bf9581a89dffb6944d8fae3bb) )
	ROM_LOAD( "galx.w",       0x1000, 0x0800, CRC(72d2d3ee) SHA1(96e0c5824e46d7398c7e58dd6b75a9f4ead6f3f5) )
	ROM_LOAD( "galx.y",       0x1800, 0x0800, CRC(afe397f3) SHA1(283c6f3b3f07581d88f7a6e11fc36947a9d90e2e) )
	ROM_LOAD( "galx.z",       0x2000, 0x0800, CRC(778c0d3c) SHA1(6a81875abfea515d379c6212cb57f8e54573e943) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "galx.1h",      0x0000, 0x0800, CRC(e8810654) SHA1(b6924c7ad765c32714e6abd5bb56b2732edd5855) )
	ROM_LOAD( "galx.1k",      0x0800, 0x0800, CRC(cbe84a76) SHA1(c6d72fb452e8213dd40a2eb5dcca726d7cdca658) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( moonaln )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "galx.u",       0x0000, 0x0800, CRC(79e4007d) SHA1(d55050498a670d1c022ba3caad34f8fcaccf4a30) ) // prg1.bin
	ROM_LOAD( "prg2.bin",     0x0800, 0x0800, CRC(59580b30) SHA1(e659426ad8c4e5e10a7cdd07d8b4fea93f875026) )
	ROM_LOAD( "prg3.bin",     0x1000, 0x0800, CRC(b64e9d12) SHA1(3b07902ea61388f54c03d65082e78dfc0fa8d3d2) )
	ROM_LOAD( "superg.y",     0x1800, 0x0800, CRC(9463f753) SHA1(d9cb35c19aafec43d08b048bbe2337a790f6ba9d) ) // prg4.bin
	ROM_LOAD( "prg5.bin",     0x2000, 0x0800, CRC(8bb78987) SHA1(5f24dba0bb31fc8bda5bf570d568472befc4d740) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "ca1.bin",   0x0000, 0x0800, CRC(074271dd) SHA1(cd6a40b493bc51c5340d7083f83c51834b95b5fe) )
	ROM_LOAD( "ca2.bin",   0x0800, 0x0800, CRC(84d90397) SHA1(93e6ded079c9721d3f9c003e378e8121584671c9) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( galap1 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "7f.bin",       0x0000, 0x1000, CRC(4335b1de) SHA1(e41e3d90dac738cf71377f3b476ec67b14dee27a) )
	ROM_LOAD( "galaxian.w",   0x1000, 0x0800, CRC(4c7031c0) SHA1(97f7ab0cedcd8eba1c8f6f516d84d672a2108258) )
	ROM_LOAD( "galx_1_4.rom", 0x1800, 0x0800, CRC(e71e1d9e) SHA1(32bf22b06c84d36de7c1280740b9c11e8d6a12b6) )
	ROM_LOAD( "galx_1_5.rom", 0x2000, 0x0800, CRC(6e65a3b2) SHA1(c9f20645ad2882e937245a9e90504423bb492158) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "galmidw.1j",   0x0000, 0x0800, CRC(84decf98) SHA1(2e565cb6057b1816a6b4541e6dfadd3c3762fa36) )
	ROM_LOAD( "galmidw.1k",   0x0800, 0x0800, CRC(c31ada9e) SHA1(237ebb48549b34ca59a13cc2706512d957413ec4) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( galap4 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "galnamco.u",   0x0000, 0x0800, CRC(acfde501) SHA1(4b72c1ffecaccadc541da2367f3ef70a2a9aed64) )
	ROM_LOAD( "galnamco.v",   0x0800, 0x0800, CRC(65cf3c77) SHA1(1c5249815816b395e1e04bf6a7dbb63e40faa0e3) )
	ROM_LOAD( "galnamco.w",   0x1000, 0x0800, CRC(9eef9ae6) SHA1(b2282e4edb8911e6aabfa936c3526f90381e1320) )
	ROM_LOAD( "galnamco.y",   0x1800, 0x0800, CRC(56a5ddd1) SHA1(1f87f647ebdffba28d5957f195448f6bce17f4d5) )
	ROM_LOAD( "galnamco.z",   0x2000, 0x0800, CRC(f4bc7262) SHA1(c4b70e474d49f45cec96f7c250bd77e01e18601a) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "galx_4c1.rom", 0x0000, 0x0800, CRC(d5e88ab4) SHA1(737a22e406fd0a97d10e93a2c91c3aa61aebbdef) )
	ROM_LOAD( "galx_4c2.rom", 0x0800, 0x0800, CRC(a57b83e4) SHA1(335d8674df1d237a4b83da00eb9aee346bc2e901) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( galturbo )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "galturbo.u",   0x0000, 0x0800, CRC(e8f3aa67) SHA1(a0e9576784dbe602dd9780e667f01f31defd7c00) )
	ROM_LOAD( "galx.v",       0x0800, 0x0800, CRC(bc16064e) SHA1(4e3220fd63c8184bf9581a89dffb6944d8fae3bb) )
	ROM_LOAD( "superg.w",     0x1000, 0x0800, CRC(ddeabdae) SHA1(daa5109a32c7c9a80bdb212dc3e4e3e3c104a731) )
	ROM_LOAD( "galturbo.y",   0x1800, 0x0800, CRC(a44f450f) SHA1(4009834afb45e9b23c7cf058bcd3378ef8601872) )
	ROM_LOAD( "galturbo.z",   0x2000, 0x0800, CRC(3247f3d4) SHA1(5754dedc2d06736629d85514b2e7c262ce27bf2d) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "galturbo.1h",  0x0000, 0x0800, CRC(a713fd1a) SHA1(abf86fe5cb7243a1a36d7ac0a868577a3360dcca) )
	ROM_LOAD( "galturbo.1k",  0x0800, 0x0800, CRC(28511790) SHA1(dec2e183a753295d033a56184c973bbc810abf55) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( swarm )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "swarm1.bin",   0x0000, 0x0800, CRC(21eba3d0) SHA1(d07f141d785c86faca8c40af034c26f2789e9346) )
	ROM_LOAD( "swarm2.bin",   0x0800, 0x0800, CRC(f3a436cd) SHA1(8d64e61b823e22f17cb79bf9e0c7b3c80c76413f) )
	ROM_LOAD( "swarm3.bin",   0x1000, 0x0800, CRC(2915e38b) SHA1(045d4cc2c363b9ba8d066f902f03b7eacbeb1f5e) )
	ROM_LOAD( "swarm4.bin",   0x1800, 0x0800, CRC(8bbbf486) SHA1(84c975562c9c359069fb70f7f416420c74d40622) )
	ROM_LOAD( "swarm5.bin",   0x2000, 0x0800, CRC(f1b1987e) SHA1(0c8b57cb156fdd1a81a5e4535464cafab737185b) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "swarma.bin",   0x0000, 0x0800, CRC(ef8657bb) SHA1(c942db83231b04041e2794a08ce779331613edcf) )
	ROM_LOAD( "swarmb.bin",   0x0800, 0x0800, CRC(60c4bd31) SHA1(a8f22f8d7a9fca2c29091888e243dfa10211e138) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( zerotime )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "zt-p01c.016",  0x0000, 0x0800, CRC(90a2bc61) SHA1(9d23dfcf5310cf1d4aa1b473ec84279585e1a876) )
	ROM_LOAD( "zt-2.016",     0x0800, 0x0800, CRC(a433067e) SHA1(1aed1a2153c4a32a9996fc709e544f2063885599) )
	ROM_LOAD( "zt-3.016",     0x1000, 0x0800, CRC(aaf038d4) SHA1(2d070fe7c4e9b26092f0f12a9db3392f7d8a65f1) )
	ROM_LOAD( "zt-4.016",     0x1800, 0x0800, CRC(786d690a) SHA1(50c5c07941006e3b71afbf057d27daa2f2274925) )
	ROM_LOAD( "zt-5.016",     0x2000, 0x0800, CRC(af9260d7) SHA1(955e466a8989993351dc69d73ca322c1c9af7b63) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "ztc-2.016",    0x0000, 0x0800, CRC(1b13ca05) SHA1(6999068771dacc6bf6c17eb858af593a929d09af) )
	ROM_LOAD( "ztc-1.016",    0x0800, 0x0800, CRC(5cd7df03) SHA1(77873408c89546a17b1da3f64b7e96e314fadb17) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( starfght )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
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

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "k1.7a",        0x0000, 0x0800, CRC(977e37cf) SHA1(88ff1e4edadf5cfc83413a1fe999aecf4ba72232) )
	ROM_LOAD( "k2.9a",        0x0800, 0x0800, CRC(15e387ce) SHA1(d804b1391de5a15c336aa53c812b4a885f830191) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "mmi6331.7f",   0x0000, 0x0020, CRC(24652bc4) SHA1(d89575f3749c75dc963317fe451ffeffd9856e4d) ) /* Compatible with 82s123 prom */
ROM_END

/* was marked 'star fighter' but doesn't appear to be the above game */
ROM_START( galaxbsf )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
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

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "11.bn",       0x0000, 0x0800, CRC(39fb43a4) SHA1(4755609bd974976f04855d51e08ec0d62ab4bc07) )
	ROM_LOAD( "12.bn",       0x0800, 0x0800, CRC(7e3f56a2) SHA1(a9795d8b7388f404f3b0e2c6ce15d713a4c5bafa) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END


ROM_START( tst_galx )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "test.u",    0x0000, 0x0800, CRC(0614cd7f) SHA1(12440678be8a27a6c3032b6e43c45e27905ffa83) )   /*  The Test ROM */
	ROM_LOAD( "galmidw.v",    0x0800, 0x0800, CRC(9c999a40) SHA1(02fdcd95d8511e64c0d2b007b874112d53e41045) )
	ROM_LOAD( "galmidw.w",    0x1000, 0x0800, CRC(b5894925) SHA1(0046b9ed697a34d088de1aead8bd7cbe526a2396) )
	ROM_LOAD( "galmidw.y",    0x1800, 0x0800, CRC(6b3ca10b) SHA1(18d8714e5ef52f63ba8888ecc5a25b17b3bf17d1) )

	ROM_LOAD( "7l",           0x2000, 0x0800, CRC(1b933207) SHA1(8b44b0f74420871454e27894d0f004859f9e59a9) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "1h.bin",       0x0000, 0x0800, CRC(39fb43a4) SHA1(4755609bd974976f04855d51e08ec0d62ab4bc07) )
	ROM_LOAD( "1k.bin",       0x0800, 0x0800, CRC(7e3f56a2) SHA1(a9795d8b7388f404f3b0e2c6ce15d713a4c5bafa) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( gmgalax )
	ROM_REGION( 0x18000, REGION_CPU1, 0 )	/* 64k for code + 32k for banked code */
	ROM_LOAD( "pcb1_pm1.bin",0x10000, 0x1000, CRC(19338c70) SHA1(cc2665b7d534d324627d12025ee099ff415d4214) )
	ROM_LOAD( "pcb1_pm2.bin",0x11000, 0x1000, CRC(18db074d) SHA1(a70ed18f632e947493e648e6fc057dfb7a2a3322) )
	ROM_LOAD( "pcb1_pm3.bin",0x12000, 0x1000, CRC(abb98b1d) SHA1(bb0109d353359bb192a3e6856a857c2f842838cb) )
	ROM_LOAD( "pcb1_pm4.bin",0x13000, 0x1000, CRC(2403c78e) SHA1(52d8c8a4efcf47871485080ab217098a019e6579) )
	ROM_LOAD( "pcb1_gx1.bin",0x14000, 0x1000, CRC(2faa9f53) SHA1(1e7010d407601c5da1adc68bc9f4742c79d57286) )
	ROM_LOAD( "pcb1_gx2.bin",0x15000, 0x1000, CRC(121c5f16) SHA1(cb1806fa984870133fd883969838dca85f992515) )
	ROM_LOAD( "pcb1_gx3.bin",0x16000, 0x1000, CRC(02d81a21) SHA1(39209cfb7cf142a65e157544d93803ea542a8efb) )

	ROM_REGION( 0x4000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "pcb2gfx1.bin", 0x0000, 0x0800, CRC(7021bbc0) SHA1(52d2983d74e722fccb31eb02ca56255850c4f41c) )
	ROM_CONTINUE(             0x1000, 0x0800 )
	ROM_LOAD( "pcb2gfx3.bin", 0x0800, 0x0800, CRC(089c922b) SHA1(f1b81999f63677d4cd58cd547353170e348a1423) )
	ROM_CONTINUE(             0x1800, 0x0800 )
	ROM_LOAD( "pcb2gfx2.bin", 0x2000, 0x0800, CRC(51bf58ee) SHA1(3546ff03c76a6422b0515bd5c695674bfb032089) )
	ROM_CONTINUE(             0x3000, 0x0800 )
	ROM_LOAD( "pcb2gfx4.bin", 0x2800, 0x0800, CRC(908fd0dc) SHA1(ac278bd82730e92ff312793244340748b93fa9bb) )
	ROM_CONTINUE(             0x3800, 0x0800 )

	ROM_REGION( 0x0040, REGION_PROMS, 0 )
	ROM_LOAD( "gmgalax2.clr", 0x0000, 0x0020, CRC(499f4440) SHA1(66d6463a145087041934bdab8bfa6c3db6375317) )
	ROM_LOAD( "l06_prom.bin", 0x0020, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( pisces )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "p1.bin",       0x0000, 0x0800, CRC(40c5b0e4) SHA1(6c18e6f4719eb0d7eb13b778d7ea58e4b87ac35c) )
	ROM_LOAD( "p2.bin",       0x0800, 0x0800, CRC(055f9762) SHA1(9d821874dd48a80651adc58a2f7fe5d2b3ed67bc) )
	ROM_LOAD( "p3.bin",       0x1000, 0x0800, CRC(3073dd04) SHA1(b93913a988f412d565abd19dc668976585cc8066) )
	ROM_LOAD( "p4.bin",       0x1800, 0x0800, CRC(44aaf525) SHA1(667bf4c3a36169c3ddddd22b2f1f90bcc9308548) )
	ROM_LOAD( "p5.bin",       0x2000, 0x0800, CRC(fade512b) SHA1(ccef2650f1d9dc3fdde2d441774246d47febc2cc) )
	ROM_LOAD( "p6.bin",       0x2800, 0x0800, CRC(5ab2822f) SHA1(bbcac3aab943dd9b173de11ddf02ff75d16b1582) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
//  ROM_LOAD( "pisces.1j",    0x0000, 0x1000, CRC(2dba9e0e) )
//  ROM_LOAD( "pisces.1k",    0x1000, 0x1000, CRC(cdc5aa26) )
	ROM_LOAD( "g09.bin",      0x0000, 0x0800, CRC(9503a23a) SHA1(23848de56841dd1de9ef74d5a9c981c784098175) )
	ROM_LOAD( "g11.bin",      0x0800, 0x0800, CRC(0adfc3fe) SHA1(a4da488632d9906066db45ae62747caf5ffbf2d8) )
	ROM_LOAD( "g10.bin",      0x1000, 0x0800, CRC(3e61f849) SHA1(efa0059bc843af0c3bb94f4bc0a8286ca5069179) )
	ROM_LOAD( "g12.bin",      0x1800, 0x0800, CRC(7130e9eb) SHA1(e6bb7a9b4f2fc001296e1060d0671b7a88599c8b) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "colour.bin",   0x0000, 0x0020, CRC(57a45057) SHA1(d4ec6a54f72640e5b485aa59e206c090e67ff640) )	// same as checkman.clr
ROM_END

ROM_START( piscesb )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "pisces.a1",    0x0000, 0x0800, CRC(856b8e1f) SHA1(24d468b5f06f54c3fa1cb54ceec8a0c8e285430e) )
	ROM_LOAD( "p2.bin",       0x0800, 0x0800, CRC(055f9762) SHA1(9d821874dd48a80651adc58a2f7fe5d2b3ed67bc) )
	ROM_LOAD( "pisces.b2",    0x1000, 0x0800, CRC(5540f2e4) SHA1(b069a7e46fa2c1f732371ef056caaf8f343e11a8) )
	ROM_LOAD( "p4.bin",       0x1800, 0x0800, CRC(44aaf525) SHA1(667bf4c3a36169c3ddddd22b2f1f90bcc9308548) )
	ROM_LOAD( "p5.bin",       0x2000, 0x0800, CRC(fade512b) SHA1(ccef2650f1d9dc3fdde2d441774246d47febc2cc) )
	ROM_LOAD( "p6.bin",       0x2800, 0x0800, CRC(5ab2822f) SHA1(bbcac3aab943dd9b173de11ddf02ff75d16b1582) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
//  ROM_LOAD( "pisces.1j",    0x0000, 0x1000, CRC(2dba9e0e) )
//  ROM_LOAD( "pisces.1k",    0x1000, 0x1000, CRC(cdc5aa26) )
	ROM_LOAD( "g09.bin",      0x0000, 0x0800, CRC(9503a23a) SHA1(23848de56841dd1de9ef74d5a9c981c784098175) )
	ROM_LOAD( "g11.bin",      0x0800, 0x0800, CRC(0adfc3fe) SHA1(a4da488632d9906066db45ae62747caf5ffbf2d8) )
	ROM_LOAD( "g10.bin",      0x1000, 0x0800, CRC(3e61f849) SHA1(efa0059bc843af0c3bb94f4bc0a8286ca5069179) )
	ROM_LOAD( "g12.bin",      0x1800, 0x0800, CRC(7130e9eb) SHA1(e6bb7a9b4f2fc001296e1060d0671b7a88599c8b) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
//  ROM_LOAD( "6331-1j.86",   0x0000, 0x0020, CRC(24652bc4) SHA1(d89575f3749c75dc963317fe451ffeffd9856e4d) ) /* very close to Galaxian */
	ROM_LOAD( "colour.bin",   0x0000, 0x0020, CRC(57a45057) SHA1(d4ec6a54f72640e5b485aa59e206c090e67ff640) )	// same as checkman.clr
ROM_END

ROM_START( omni )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "omni1.7f",     0x0000, 0x1000, CRC(a9b7acc6) SHA1(0c6319957b760fea3cfa6c29b37c25f5a89a6d77) )
	ROM_LOAD( "omni2.7j",     0x1000, 0x1000, CRC(6ade29b7) SHA1(64f1ce82c761db11d26c385299a7063f5971c99a) )
	ROM_LOAD( "omni3.7f",     0x2000, 0x1000, CRC(9e37bb24) SHA1(d90b2ff0297d87687561e1e9b29510b6c051760b) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "omni5b.l1",    0x0000, 0x0800, CRC(9503a23a) SHA1(23848de56841dd1de9ef74d5a9c981c784098175) )
	ROM_LOAD( "omni6c.j22",   0x0800, 0x0800, CRC(0adfc3fe) SHA1(a4da488632d9906066db45ae62747caf5ffbf2d8) )
	ROM_LOAD( "omni4a.j1",    0x1000, 0x0800, CRC(3e61f849) SHA1(efa0059bc843af0c3bb94f4bc0a8286ca5069179) )
	ROM_LOAD( "omni7d.l2",    0x1800, 0x0800, CRC(7130e9eb) SHA1(e6bb7a9b4f2fc001296e1060d0671b7a88599c8b) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "colour.bin",   0x0000, 0x0020, CRC(57a45057) SHA1(d4ec6a54f72640e5b485aa59e206c090e67ff640) )
ROM_END

ROM_START( uniwars )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "f07_1a.bin",   0x0000, 0x0800, CRC(d975af10) SHA1(a2e2a36a75db8fd09441308b08b6ae073c68b8cf) )
	ROM_LOAD( "h07_2a.bin",   0x0800, 0x0800, CRC(b2ed14c3) SHA1(7668df11f64b8e296eedfee53437777dc53a56d5) )
	ROM_LOAD( "k07_3a.bin",   0x1000, 0x0800, CRC(945f4160) SHA1(5fbe879f51e14c4c7ae551e5b3089f8e148770a4) )
	ROM_LOAD( "m07_4a.bin",   0x1800, 0x0800, CRC(ddc80bc5) SHA1(18c3920198baf87267bc7f12db6b23b090d3577a) )
	ROM_LOAD( "d08p_5a.bin",  0x2000, 0x0800, CRC(62354351) SHA1(85bf18942f73023b8be0c3659a0dcd3dfcccfc2c) )
	ROM_LOAD( "gg6",          0x2800, 0x0800, CRC(270a3f4d) SHA1(20f5097033fca515d70fe47178cbd341a1d07443) )
	ROM_LOAD( "m08p_7a.bin",  0x3000, 0x0800, CRC(c9245346) SHA1(239bad3fe64eaab2dfc3febd06d1124103a10504) )
	ROM_LOAD( "n08p_8a.bin",  0x3800, 0x0800, CRC(797d45c7) SHA1(76fb8b45fcce3622c59c04af32cfa001ef7bf71d) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "egg10",        0x0000, 0x0800, CRC(012941e0) SHA1(4f7ec4d95939cb7c4086bb7df43759ac504ae47c) )
	ROM_LOAD( "h01_2.bin",    0x0800, 0x0800, CRC(c26132af) SHA1(7ae125a911dfd47aeca4f129f580762ce4d8d91a) )
	ROM_LOAD( "egg9",         0x1000, 0x0800, CRC(fc8b58fd) SHA1(72553e2735b0dcc2dcfce9698d49566732492588) )
	ROM_LOAD( "k01_2.bin",    0x1800, 0x0800, CRC(dcc2b33b) SHA1(c3a5ac935c519400dfabb28909f7e460769d1837) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "uniwars.clr",  0x0000, 0x0020, CRC(25c79518) SHA1(e8f7e8b3d0cf1ed9d723948548f58abf0e2c6d1f) )
ROM_END

ROM_START( gteikoku )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "f07_1a.bin",   0x0000, 0x0800, CRC(d975af10) SHA1(a2e2a36a75db8fd09441308b08b6ae073c68b8cf) )
	ROM_LOAD( "h07_2a.bin",   0x0800, 0x0800, CRC(b2ed14c3) SHA1(7668df11f64b8e296eedfee53437777dc53a56d5) )
	ROM_LOAD( "k07_3a.bin",   0x1000, 0x0800, CRC(945f4160) SHA1(5fbe879f51e14c4c7ae551e5b3089f8e148770a4) )
	ROM_LOAD( "m07_4a.bin",   0x1800, 0x0800, CRC(ddc80bc5) SHA1(18c3920198baf87267bc7f12db6b23b090d3577a) )
	ROM_LOAD( "d08p_5a.bin",  0x2000, 0x0800, CRC(62354351) SHA1(85bf18942f73023b8be0c3659a0dcd3dfcccfc2c) )
	ROM_LOAD( "e08p_6a.bin",  0x2800, 0x0800, CRC(d915a389) SHA1(0e2ff6eec9453856a1276889946b463cfae58eba) )
	ROM_LOAD( "m08p_7a.bin",  0x3000, 0x0800, CRC(c9245346) SHA1(239bad3fe64eaab2dfc3febd06d1124103a10504) )
	ROM_LOAD( "n08p_8a.bin",  0x3800, 0x0800, CRC(797d45c7) SHA1(76fb8b45fcce3622c59c04af32cfa001ef7bf71d) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "h01_1.bin",    0x0000, 0x0800, CRC(8313c959) SHA1(b09157c6f824d6e94647728cbb329877fcb4e502) )
	ROM_LOAD( "h01_2.bin",    0x0800, 0x0800, CRC(c26132af) SHA1(7ae125a911dfd47aeca4f129f580762ce4d8d91a) )
	ROM_LOAD( "k01_1.bin",    0x1000, 0x0800, CRC(c9d4537e) SHA1(65d27066ffec04b755d2f5d3f36f5ec6792e8d6c) )
	ROM_LOAD( "k01_2.bin",    0x1800, 0x0800, CRC(dcc2b33b) SHA1(c3a5ac935c519400dfabb28909f7e460769d1837) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "l06_prom.bin", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( gteikokb )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "1.bin",        0x0000, 0x0800, CRC(bf00252f) SHA1(a4ec48c6b9468f52bcf8b01d1bdb908dcf81d42d) )
	ROM_LOAD( "2.bin",   	  0x0800, 0x0800, CRC(f712b7d5) SHA1(c269db2e9984a3fbd33888bd426c53d319cad36f) )
	ROM_LOAD( "k07_3a.bin",   0x1000, 0x0800, CRC(945f4160) SHA1(5fbe879f51e14c4c7ae551e5b3089f8e148770a4) )
	ROM_LOAD( "4.bin",   	  0x1800, 0x0800, CRC(808a39a8) SHA1(f3db5175d0c2d10e9e3ded400888f6541490597e) )
	ROM_LOAD( "5.bin",  	  0x2000, 0x0800, CRC(36fe6e67) SHA1(e54a19ad6611fefcdfcf74019a63cc6cea6cf433) )
	ROM_LOAD( "6.bin",  	  0x2800, 0x0800, CRC(c5ea67e8) SHA1(0157eb2ef5ab56cd00e5f4fafd618271d2d4862b) )
	ROM_LOAD( "m08p_7a.bin",  0x3000, 0x0800, CRC(c9245346) SHA1(239bad3fe64eaab2dfc3febd06d1124103a10504) )
	ROM_LOAD( "8.bin",  	  0x3800, 0x0800, CRC(28df3229) SHA1(fd307c6a7de4fcddce1c2f36a957a31b9a6aaa21) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "h01_1.bin",    0x0000, 0x0800, CRC(8313c959) SHA1(b09157c6f824d6e94647728cbb329877fcb4e502) )
	ROM_LOAD( "h01_2.bin",    0x0800, 0x0800, CRC(c26132af) SHA1(7ae125a911dfd47aeca4f129f580762ce4d8d91a) )
	ROM_LOAD( "k01_1.bin",    0x1000, 0x0800, CRC(c9d4537e) SHA1(65d27066ffec04b755d2f5d3f36f5ec6792e8d6c) )
	ROM_LOAD( "k01_2.bin",    0x1800, 0x0800, CRC(dcc2b33b) SHA1(c3a5ac935c519400dfabb28909f7e460769d1837) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "l06_prom.bin", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( gteikob2 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "94gnog.bin",   0x0000, 0x0800, CRC(67ec3235) SHA1(f250db867257f474f693012c11008bf92f038cc7) )
	ROM_LOAD( "92gnog.bin",   0x0800, 0x0800, CRC(813c41f2) SHA1(bd92e0b53e3c8874d63f3444bca02246cd74b1c6) )
	ROM_LOAD( "k07_3a.bin",   0x1000, 0x0800, CRC(945f4160) SHA1(5fbe879f51e14c4c7ae551e5b3089f8e148770a4) )
	ROM_LOAD( "1gnog.bin",    0x1800, 0x0800, CRC(49ff9658) SHA1(3b7f3dc40b3fbc7d4abe5f5d534951c70409148c) )
	ROM_LOAD( "5.bin",  	  0x2000, 0x0800, CRC(36fe6e67) SHA1(e54a19ad6611fefcdfcf74019a63cc6cea6cf433) )
	ROM_LOAD( "e08p_6a.bin",  0x2800, 0x0800, CRC(d915a389) SHA1(0e2ff6eec9453856a1276889946b463cfae58eba) )
	ROM_LOAD( "m08p_7a.bin",  0x3000, 0x0800, CRC(c9245346) SHA1(239bad3fe64eaab2dfc3febd06d1124103a10504) )
	ROM_LOAD( "98gnog.bin",   0x3800, 0x0800, CRC(e9d4ad3c) SHA1(b32b96bebbf59e23b06958f6b16790e9f9f334e2) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "h01_1.bin",    0x0000, 0x0800, CRC(8313c959) SHA1(b09157c6f824d6e94647728cbb329877fcb4e502) )
	ROM_LOAD( "h01_2.bin",    0x0800, 0x0800, CRC(c26132af) SHA1(7ae125a911dfd47aeca4f129f580762ce4d8d91a) )
	ROM_LOAD( "k01_1.bin",    0x1000, 0x0800, CRC(c9d4537e) SHA1(65d27066ffec04b755d2f5d3f36f5ec6792e8d6c) )
	ROM_LOAD( "k01_2.bin",    0x1800, 0x0800, CRC(dcc2b33b) SHA1(c3a5ac935c519400dfabb28909f7e460769d1837) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "l06_prom.bin", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( spacbatt )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "sb1",    0x0000, 0x0800, CRC(d975af10) SHA1(a2e2a36a75db8fd09441308b08b6ae073c68b8cf) ) /* Same as f07_1a.bin above */
	ROM_LOAD( "sb2",    0x0800, 0x0800, CRC(b2ed14c3) SHA1(7668df11f64b8e296eedfee53437777dc53a56d5) ) /* Same as h07_2a.bin above */
	ROM_LOAD( "sb3",    0x1000, 0x0800, CRC(945f4160) SHA1(5fbe879f51e14c4c7ae551e5b3089f8e148770a4) ) /* Same as k07_3a.bin above */
	ROM_LOAD( "sb4",    0x1800, 0x0800, CRC(8229835c) SHA1(8cfd8f6cab6f80ca69645a184f7e841fc69f47f6) )
	ROM_LOAD( "sb5",    0x2000, 0x0800, CRC(f51ef930) SHA1(213e68571a0c7d5ba33a7170d5fa4aea898ea0b9) )
	ROM_LOAD( "sb6",    0x2800, 0x0800, CRC(d915a389) SHA1(0e2ff6eec9453856a1276889946b463cfae58eba) ) /* Same as e08p_6a.bin above */
	ROM_LOAD( "sb7",    0x3000, 0x0800, CRC(c9245346) SHA1(239bad3fe64eaab2dfc3febd06d1124103a10504) ) /* Same as m08p_7a.bin above */
	ROM_LOAD( "sb8",    0x3800, 0x0800, CRC(e59ff1ae) SHA1(fef22885cbd3273882f8c7755dd04c28e843b9ea) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "sb12",   0x0000, 0x0800, CRC(8313c959) SHA1(b09157c6f824d6e94647728cbb329877fcb4e502) ) /* Same as h01_1.bin above */
	ROM_LOAD( "sb14",   0x0800, 0x0800, CRC(c26132af) SHA1(7ae125a911dfd47aeca4f129f580762ce4d8d91a) ) /* Same as h01_2.bin above */
	ROM_LOAD( "sb11",   0x1000, 0x0800, CRC(c9d4537e) SHA1(65d27066ffec04b755d2f5d3f36f5ec6792e8d6c) ) /* Same as k01_1.bin above */
	ROM_LOAD( "sb13",   0x1800, 0x0800, CRC(92454380) SHA1(f0cd67b39c760c2b5ac549b27b0a5f83fbb3a86b) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "6l.bpr", 0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) ) /* MMI 6331 bp-prom, compatible with 82s123 */
ROM_END

ROM_START( spacbat2 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "sb1",    0x0000, 0x0800, CRC(d975af10) SHA1(a2e2a36a75db8fd09441308b08b6ae073c68b8cf) ) /* Same as f07_1a.bin above */
	ROM_LOAD( "sb2",    0x0800, 0x0800, CRC(b2ed14c3) SHA1(7668df11f64b8e296eedfee53437777dc53a56d5) ) /* Same as h07_2a.bin above */
	ROM_LOAD( "sb.3",   0x1000, 0x0800, CRC(c25ce4c1) SHA1(d7a5d435df7868155523d2fb90f331d4b6d9eaa1) )
	ROM_LOAD( "sb4",    0x1800, 0x0800, CRC(8229835c) SHA1(8cfd8f6cab6f80ca69645a184f7e841fc69f47f6) )
	ROM_LOAD( "sb5",    0x2000, 0x0800, CRC(f51ef930) SHA1(213e68571a0c7d5ba33a7170d5fa4aea898ea0b9) )
	ROM_LOAD( "sb6",    0x2800, 0x0800, CRC(d915a389) SHA1(0e2ff6eec9453856a1276889946b463cfae58eba) ) /* Same as e08p_6a.bin above */
	ROM_LOAD( "sb7",    0x3000, 0x0800, CRC(c9245346) SHA1(239bad3fe64eaab2dfc3febd06d1124103a10504) ) /* Same as m08p_7a.bin above */
	ROM_LOAD( "sb8",    0x3800, 0x0800, CRC(e59ff1ae) SHA1(fef22885cbd3273882f8c7755dd04c28e843b9ea) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "sb12",      0x0000, 0x0800, CRC(8313c959) SHA1(b09157c6f824d6e94647728cbb329877fcb4e502) ) /* Same as h01_1.bin above */
	ROM_LOAD( "sb14",      0x0800, 0x0800, CRC(c26132af) SHA1(7ae125a911dfd47aeca4f129f580762ce4d8d91a) ) /* Same as h01_2.bin above */
	ROM_LOAD( "sb11",      0x1000, 0x0800, CRC(c9d4537e) SHA1(65d27066ffec04b755d2f5d3f36f5ec6792e8d6c) ) /* Same as k01_1.bin above */
	ROM_LOAD( "k01_2.bin", 0x1800, 0x0800, CRC(dcc2b33b) SHA1(c3a5ac935c519400dfabb28909f7e460769d1837) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "l06_prom.bin", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( skyraidr )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "f07_1a.bin",   0x0000, 0x0800, CRC(d975af10) SHA1(a2e2a36a75db8fd09441308b08b6ae073c68b8cf) )
	ROM_LOAD( "h07_2a.bin",   0x0800, 0x0800, CRC(b2ed14c3) SHA1(7668df11f64b8e296eedfee53437777dc53a56d5) )
	ROM_LOAD( "k07_3a.bin",   0x1000, 0x0800, CRC(945f4160) SHA1(5fbe879f51e14c4c7ae551e5b3089f8e148770a4) )
	ROM_LOAD( "sr.04",        0x1800, 0x0800, CRC(9f61d1f8) SHA1(389b0a0d1a577b302907b2ea4c119aa18a6120d9) )
	ROM_LOAD( "sr.05",        0x2000, 0x0800, CRC(4352af0a) SHA1(1b31846ea7025aaf3a79141dfa5a089b8d12d982) )
	ROM_LOAD( "sr.06",        0x2800, 0x0800, CRC(d915a389) SHA1(0e2ff6eec9453856a1276889946b463cfae58eba) )
	ROM_LOAD( "m08p_7a.bin",  0x3000, 0x0800, CRC(c9245346) SHA1(239bad3fe64eaab2dfc3febd06d1124103a10504) )
	ROM_LOAD( "n08p_8a.bin",  0x3800, 0x0800, CRC(797d45c7) SHA1(76fb8b45fcce3622c59c04af32cfa001ef7bf71d) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "sr.10",        0x0000, 0x0800, CRC(af069cba) SHA1(12b7d0a57f43613c80afd51c417628090740aabe) )
	ROM_LOAD( "h01_2.bin",    0x0800, 0x0800, CRC(c26132af) SHA1(7ae125a911dfd47aeca4f129f580762ce4d8d91a) )
	ROM_LOAD( "sr.09",        0x1000, 0x0800, CRC(ff2c20d5) SHA1(48668dc4f008f44f5c15bdcc331cfe133da99cd4) )
	ROM_LOAD( "k01_2.bin",    0x1800, 0x0800, CRC(dcc2b33b) SHA1(c3a5ac935c519400dfabb28909f7e460769d1837) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "uniwars.clr",  0x0000, 0x0020, CRC(25c79518) SHA1(e8f7e8b3d0cf1ed9d723948548f58abf0e2c6d1f) )
ROM_END

ROM_START( batman2 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "01.bin",    	  0x0000, 0x0800, CRC(150fbca5) SHA1(a5dc104169eb3225c6200e7e07102f8a9bee6861) )
	ROM_LOAD( "02.bin",       0x0800, 0x0800, CRC(b1624fd0) SHA1(ca4678cf7a8b935be2f68d6e342c1f961bf6f1a2) )
	ROM_LOAD( "03.bin",       0x1000, 0x0800, CRC(93774188) SHA1(8bdd3290db43459c56b932b582f555d89df30bd1) )
	ROM_LOAD( "04.bin",       0x1800, 0x0800, CRC(8a94ec6c) SHA1(dacadab9a05ddee2de188b368f795d74213e020d) )
	ROM_LOAD( "05.bin",       0x2000, 0x0800, CRC(a3669461) SHA1(11ea7aa9b55f5790cc2451d80d0eb84388cf47eb) )
	ROM_LOAD( "06.bin",       0x2800, 0x0800, CRC(fa1efbfe) SHA1(f7222dd21e0810d0c8c32919ebb6e0e7bbb4c68e) )
	ROM_LOAD( "07.bin",       0x3000, 0x0800, CRC(9b77debd) SHA1(1f5521bc0f701d86e61219ad3b9516aaa71a68da) )
	ROM_LOAD( "08.bin",       0x3800, 0x0800, CRC(6466177e) SHA1(fc359eadee34586576c557ff7c1dd2c8d49bdf3f) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "09.bin",       0x0000, 0x0800, CRC(1a657b1f) SHA1(42149dafdde7d9104f0bddda2223bfc211d0154a) )
	ROM_LOAD( "11.bin",       0x0800, 0x0800, CRC(7a2b48e5) SHA1(f559799c685dd2cb9de06a356bee95b7d6ffadfc) )
	ROM_LOAD( "10.bin",       0x1000, 0x0800, CRC(9b570016) SHA1(44fd2b1caeecdc5200d63c35636f0a605943d30c) )
	ROM_LOAD( "12.bin",       0x1800, 0x0800, CRC(73956244) SHA1(e464b587b5ed636816cc9688593f5b6005cb5216) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "l06_prom.bin", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( warofbug )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "warofbug.u",   0x0000, 0x0800, CRC(b8dfb7e3) SHA1(c7c675b2638869a9cd7dbd554e6131d8c71b567a) )
	ROM_LOAD( "warofbug.v",   0x0800, 0x0800, CRC(fd8854e0) SHA1(b39ab41b834f18341968dd780f0a3cd07d70c16c) )
	ROM_LOAD( "warofbug.w",   0x1000, 0x0800, CRC(4495aa14) SHA1(f1be281db1d831770efa9cc41ea87eb348e70108) )
	ROM_LOAD( "warofbug.y",   0x1800, 0x0800, CRC(c14a541f) SHA1(d32e89fd18d9e1db2e4a545186eac728c0b02255) )
	ROM_LOAD( "warofbug.z",   0x2000, 0x0800, CRC(c167fe55) SHA1(d85c4d1bd7aa5e14eb2f11dfa14979e5dbc084a8) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "warofbug.1k",  0x0000, 0x0800, CRC(8100fa85) SHA1(06641c431cace36dec98b87555f62e72f3e53a31) )
	ROM_LOAD( "warofbug.1j",  0x0800, 0x0800, CRC(d1220ae9) SHA1(e892bc8b0b71d8b07503e474e9c30e6cab460682) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "warofbug.clr", 0x0000, 0x0020, CRC(8688e64b) SHA1(ed13414257f580b98b50c9892a14159c55e7838d) )
ROM_END

ROM_START( redufo )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "ru1a",         0x0000, 0x0800, CRC(5a8e4f37) SHA1(c0957ede91e2dc3f80e4912b877843aed5d15779) )
	ROM_LOAD( "ru2a",         0x0800, 0x0800, CRC(c624f52d) SHA1(119a660513ad33e35c9bdaecd588219bf8026d82) )
	ROM_LOAD( "ru3a",         0x1000, 0x0800, CRC(e1030d1c) SHA1(80640fbbfa7f84c016366b1084e7f8a7acdcd440) )
	ROM_LOAD( "ru4a",         0x1800, 0x0800, CRC(7692069e) SHA1(5130d61c857c3b85eadabcf10f3a6771c72f0f56) )
	ROM_LOAD( "ru5a",         0x2000, 0x0800, CRC(cb648ff3) SHA1(e0042251ca7f4a31b5bd9f8cca35278a1e152899) )
	ROM_LOAD( "ru6a",         0x2800, 0x0800, CRC(e1a9f58e) SHA1(4fc7489fca057156a7cf5efcb01058ce4f0db69e) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "ruhja",        0x0000, 0x0800, CRC(8a422b0d) SHA1(b886157518f73e7115a225ba230e456179f6e18f) )
	ROM_LOAD( "rukla",        0x0800, 0x0800, CRC(1eb84cb1) SHA1(08f360802a90039c0499a1417d06b6eb5f89d67e) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( exodus )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "exodus1.bin",  0x0000, 0x0800, CRC(5dfe65e1) SHA1(5f1ce289b3c98a89d61d4dea952b4b8888d92ed7) )
	ROM_LOAD( "exodus2.bin",  0x0800, 0x0800, CRC(6559222f) SHA1(520497f6fb2b0c76be8419702e8af894283ebf0b) )
	ROM_LOAD( "exodus3.bin",  0x1000, 0x0800, CRC(bf7030e8) SHA1(59b0624dd91527a916ee6a27d61def82c3c14f49) )
	ROM_LOAD( "exodus4.bin",  0x1800, 0x0800, CRC(3607909e) SHA1(93d074fe4b258d496a0998acb3fc47f0a762227a) )
	ROM_LOAD( "exodus9.bin",  0x2000, 0x0800, CRC(994a90c4) SHA1(a07e3ce8f69042c45ebe00ab1d40dbb85602a7a2) )
	ROM_LOAD( "exodus10.bin", 0x2800, 0x0800, CRC(fbd11187) SHA1(a3bd49c4a79e76b08e6b343b94689159dc239458) )
	ROM_LOAD( "exodus11.bin", 0x3000, 0x0800, CRC(fd07d811) SHA1(6b968a7ce452f76a8d26fe694aa4ea6b16e8b6fa) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "exodus5.bin",  0x0000, 0x0800, CRC(b34c7cb4) SHA1(146ed4a02d7540378f4a27a6643055216ad403f7) )
	ROM_LOAD( "exodus6.bin",  0x0800, 0x0800, CRC(50a2d447) SHA1(1f97d1096ad2a3a43a480cb1f040f4534fada3c3) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "l06_prom.bin", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( streakng )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "sk1",          0x0000, 0x1000, CRC(c8866ccb) SHA1(1fc8bc643ecbfa86a50448d79b299f5a3dd586c5) )
	ROM_LOAD( "sk2",          0x1000, 0x1000, CRC(7caea29b) SHA1(5b3946ee914b1637db9046abf92d66ceaeb4fc5f) )
	ROM_LOAD( "sk3",          0x2000, 0x1000, CRC(7b4bfa76) SHA1(9223bec0c1cc39bc84670869b2a4fab0d0167c6e) )
	ROM_LOAD( "sk4",          0x3000, 0x1000, CRC(056fc921) SHA1(de8525571e5a82ddf74dd57b1a6c5bc9f2d2c0fe) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "sk5",          0x0000, 0x1000, CRC(d27f1e0c) SHA1(c3b4ae55a93516b034a16c9f943b360b24c933d6) )
	ROM_LOAD( "sk6",          0x1000, 0x1000, CRC(a7089588) SHA1(e76242b043b1d8f060f669da3ddeee3d10122cdb) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "sk.bpr",       0x0000, 0x0020, CRC(bce79607) SHA1(49d60fde149240bcd025f721b0fbbbdbc549a42f) )
ROM_END

ROM_START( pacmanbl )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "blpac1b",      0x0000, 0x0800, CRC(6718df42) SHA1(ee15c3f583d381fba4878f824f83d04479a0cee5) )
	ROM_LOAD( "blpac2b",      0x0800, 0x0800, CRC(33be3648) SHA1(50175889cf37fe8a81c931e009b55d10f8d0444a) )
	ROM_LOAD( "blpac3b",      0x1000, 0x0800, CRC(f98c0ceb) SHA1(4faf8b2fb3f109d1196a9ea256328485074a31b9) )
	ROM_LOAD( "blpac4b",      0x1800, 0x0800, CRC(a9cd0082) SHA1(f44ff1ad15d5ee3096f8f44f9c605f32ae2737d9) )
	ROM_LOAD( "blpac5b",      0x2000, 0x0800, CRC(6d475afc) SHA1(4fe6bde352c7dd9572fefaae4b59640b4f4eb8ba) )
	ROM_LOAD( "blpac6b",      0x2800, 0x0800, CRC(cbe863d3) SHA1(97a2ffa6ab33e6061c664dcd1ee57c86a456782f) )
	ROM_LOAD( "blpac7b",      0x3000, 0x0800, CRC(7daef758) SHA1(4dc8ec0ea8fc04d5bffc1c1335407729309c17f0) )

	/* note from f205v: on the PCB I have, 10b and 11b have been joined into one single 2732 EPROM labeled "pmc31"
    The same goes for 9b and 12b, joined into one single 2732 EPROM labeled "pmc42" */
	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "blpac12b",     0x0000, 0x0800, CRC(b2ed320b) SHA1(680a6fdcb65cc2d88d10bc85e0b2628f43375c5c) )
	ROM_LOAD( "blpac11b",     0x0800, 0x0800, CRC(ab88b2c4) SHA1(d0c829ea8021eae81a2b82d36c35ad8258b115e0) )
	ROM_LOAD( "blpac10b",     0x1000, 0x0800, CRC(44a45b72) SHA1(8abd0684a01d6c23ef5cf5f0765458f982316acf) )
	ROM_LOAD( "blpac9b",      0x1800, 0x0800, CRC(fa84659f) SHA1(20c212723f9062f052539190dfe3fc41577543eb) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "blpaccp",      0x0000, 0x0020, CRC(24652bc4) SHA1(d89575f3749c75dc963317fe451ffeffd9856e4d) ) /* same as pisces */
ROM_END

ROM_START( devilfsg )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "dfish1.7f",    0x2000, 0x0800, CRC(2ab19698) SHA1(8450981d3cf3fa8abf2fb5487aa98b03a4cf03a1) )
	ROM_CONTINUE(             0x0000, 0x0800 )
	ROM_LOAD( "dfish2.7h",    0x2800, 0x0800, CRC(4e77f097) SHA1(aeaa5ff210ccbbe77114edf5dee992d2720636ae) )
	ROM_CONTINUE(             0x0800, 0x0800 )
	ROM_LOAD( "dfish3.7k",    0x3000, 0x0800, CRC(3f16a4c6) SHA1(cc30b27070a12c250cdc2f7289bae7c7a4c05c2c) )
	ROM_CONTINUE(             0x1000, 0x0800 )
	ROM_LOAD( "dfish4.7m",    0x3800, 0x0800, CRC(11fc7e59) SHA1(2c0182a75bfca085e67483b421f40b3bc9b8ef24) )
	ROM_CONTINUE(             0x1800, 0x0800 )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "dfish5.1h",    0x1000, 0x0800, CRC(ace6e31f) SHA1(23df890fdf8ef275af79e10c8e43ff3a617b28ac) )
	ROM_CONTINUE(             0x0000, 0x0800 )
	ROM_LOAD( "dfish6.1k",    0x1800, 0x0800, CRC(d7a6c4c4) SHA1(ec5f9182657edb11884ab93f868f1bb3569461ae) )
	ROM_CONTINUE(             0x0800, 0x0800 )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.6e",    0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )
ROM_END

ROM_START( zigzag )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "zz_d1.bin",    0x0000, 0x1000, CRC(8cc08d81) SHA1(be671192ef06dc3ed6963dc39e6bdce3275300e9) )
	ROM_LOAD( "zz_d2.bin",    0x1000, 0x1000, CRC(326d8d45) SHA1(563b9fc64c34e36cfadffb107ce30d3a04d62d9c) )
	ROM_LOAD( "zz_d4.bin",    0x2000, 0x1000, CRC(a94ed92a) SHA1(d56f32fc2b3f0f7affe658b7726682c60d09bc16) )
	ROM_LOAD( "zz_d3.bin",    0x3000, 0x1000, CRC(ce5e7a00) SHA1(93c47d22698a016cb0f0b654ade9ccab0cd1c88b) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "zz_6_h1.bin",  0x0000, 0x0800, CRC(780c162a) SHA1(b0cac68258281917bcada52ce26e0ce38721d633) )
	ROM_CONTINUE(             0x1000, 0x0800 )
	ROM_LOAD( "zz_5.bin",     0x0800, 0x0800, CRC(f3cdfec5) SHA1(798d631c72d8e6b2e372b4b3ab0c10d8365a1359) )
	ROM_CONTINUE(             0x1800, 0x0800 )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "zzbp_e9.bin",  0x0000, 0x0020, CRC(aa486dd0) SHA1(b845b52715bf6361ceee8c1ac541733963bd47af) )
ROM_END

ROM_START( zigzag2 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "z1",           0x0000, 0x1000, CRC(4c28349a) SHA1(646134ce506deaee88cc2ec5a973f8fedaddb66b) )
	ROM_LOAD( "zz_d2.bin",    0x1000, 0x1000, CRC(326d8d45) SHA1(563b9fc64c34e36cfadffb107ce30d3a04d62d9c) )
	ROM_LOAD( "zz_d4.bin",    0x2000, 0x1000, CRC(a94ed92a) SHA1(d56f32fc2b3f0f7affe658b7726682c60d09bc16) )
	ROM_LOAD( "zz_d3.bin",    0x3000, 0x1000, CRC(ce5e7a00) SHA1(93c47d22698a016cb0f0b654ade9ccab0cd1c88b) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "zz_6_h1.bin",  0x0000, 0x0800, CRC(780c162a) SHA1(b0cac68258281917bcada52ce26e0ce38721d633) )
	ROM_CONTINUE(             0x1000, 0x0800 )
	ROM_LOAD( "zz_5.bin",     0x0800, 0x0800, CRC(f3cdfec5) SHA1(798d631c72d8e6b2e372b4b3ab0c10d8365a1359) )
	ROM_CONTINUE(             0x1800, 0x0800 )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "zzbp_e9.bin",  0x0000, 0x0020, CRC(aa486dd0) SHA1(b845b52715bf6361ceee8c1ac541733963bd47af) )
ROM_END

ROM_START( mooncrgx )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "1",            0x0000, 0x0800, CRC(84cf420b) SHA1(82c979467c51df699337d5878340d05bee606480) )
	ROM_LOAD( "2",            0x0800, 0x0800, CRC(4c2a61a1) SHA1(a3759bd2c062f2843cd5b812529c798d5d12086c) )
	ROM_LOAD( "3",            0x1000, 0x0800, CRC(1962523a) SHA1(56ea003c3ff37c2bc33383207fccde0ba0ed781a) )
	ROM_LOAD( "4",            0x1800, 0x0800, CRC(75dca896) SHA1(017d04501d3d1305491ba843d92ebd74d47d2f9c) )
	ROM_LOAD( "5",            0x2000, 0x0800, CRC(32483039) SHA1(23baf136d5b7fc02f999dcb31b8daf68b6ffafd1) )
	ROM_LOAD( "6",            0x2800, 0x0800, CRC(43f2ab89) SHA1(f7f0802a12fd89d61f6f00044e077f34a9d3955f) )
	ROM_LOAD( "7",            0x3000, 0x0800, CRC(1e9c168c) SHA1(891dc159dfc343322c3241980a0ef76dee510ca9) )
	ROM_LOAD( "8",            0x3800, 0x0800, CRC(5e09da94) SHA1(677890912db12df6fa2cb515c198f8ac3f7187af) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "1h_1_10.bin",  0x0000, 0x0800, CRC(528da705) SHA1(d726ee18b79774c982f88afb2a508eb5d5783193) )
	ROM_LOAD( "12.chr",       0x0800, 0x0800, CRC(5a4b17ea) SHA1(8a879dc34fdecc8a121c4a87abb981212fb05945) )
	ROM_LOAD( "9.chr",        0x1000, 0x0800, CRC(70df525c) SHA1(f771293494a2234bf80f206ecf1e88773322e503) )
	ROM_LOAD( "11.chr",       0x1800, 0x0800, CRC(e0edccbd) SHA1(0839a4c9b6e863d12253ae8e1732e80e08702228) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "l06_prom.bin", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( omega )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "omega1.bin",   0x0000, 0x0800, CRC(fc2a096b) SHA1(071ff30060a1aa0a47ae6e88140b80caed00fc4e) )
	ROM_LOAD( "omega2.bin",   0x0800, 0x0800, CRC(ad100357) SHA1(7c5e82c25e65b4a390cf5607f15bf4df407f7f11) )
	ROM_LOAD( "omega3.bin",   0x1000, 0x0800, CRC(d7e3be79) SHA1(ffa228043c6c717bee8bbec16432dcfe2e348aef) )
	ROM_LOAD( "omega4.bin",   0x1800, 0x0800, CRC(42068171) SHA1(940ca30a5772940b8a437498d22c6121482b38e6) )
	ROM_LOAD( "omega5.bin",   0x2000, 0x0800, CRC(d8a93383) SHA1(5f60f127360b14206d4df638e528bf961049e37d) )
	ROM_LOAD( "omega6.bin",   0x2800, 0x0800, CRC(32a42f44) SHA1(94f458997ec279dce218a17b665fa8c46067e646) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "omega1h.bin",  0x0000, 0x0800, CRC(527fd384) SHA1(92a384899d5acd2c689f637da16a0e2d11a9d9c6) )
	ROM_LOAD( "omega1k.bin",  0x0800, 0x0800, CRC(36de42c6) SHA1(6fd93d439e3b8eab62049f925d9e8f8deeda2ae3) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "6331-1j.86",   0x0000, 0x0020, NO_DUMP )	/* missing */
ROM_END

ROM_START( scramblb )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "scramble.1k",  0x0000, 0x0800, CRC(9e025c4a) SHA1(a8cc9391bdd01a5a2fe7f0c4e889b4e2495df891) )
	ROM_LOAD( "scramble.2k",  0x0800, 0x0800, CRC(306f783e) SHA1(92d19f90f1123cd211706294d668ab23c8b0760b) )
	ROM_LOAD( "scramble.3k",  0x1000, 0x0800, CRC(0500b701) SHA1(54c84ccad2aae34f42fdddcfcd92cd9da2cd7119) )
	ROM_LOAD( "scramble.4k",  0x1800, 0x0800, CRC(dd380a22) SHA1(125e713a58cc5f2c1e38f67dad29f8c985ce5a8b) )
	ROM_LOAD( "scramble.5k",  0x2000, 0x0800, CRC(df0b9648) SHA1(4ae9150c9441897d5ab7c5a0b3f10e1e8c8e2f6c) )
	ROM_LOAD( "scramble.1j",  0x2800, 0x0800, CRC(b8c07b3c) SHA1(33eaedef4b7f49eeef072425541c17206d0a00ec) )
	ROM_LOAD( "scramble.2j",  0x3000, 0x0800, CRC(88ac07a0) SHA1(c57061db5984b472039356bf84a050b5b66e3813) )
	ROM_LOAD( "scramble.3j",  0x3800, 0x0800, CRC(c67d57ca) SHA1(ba8b14289aef47d48d9750cf2ef3c368e74a60e8) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "5f.k",         0x0000, 0x0800, CRC(4708845b) SHA1(a8b1ad19a95a9d35050a2ab7194cc96fc5afcdc9) )
	ROM_LOAD( "5h.k",         0x0800, 0x0800, CRC(11fd2887) SHA1(69844e48bb4d372cac7ae83c953df573c7ecbb7f) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.6e",    0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )
ROM_END


ROM_START( scramb2 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
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

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "r6.1j",         0x0000, 0x0800, CRC(4708845b) SHA1(a8b1ad19a95a9d35050a2ab7194cc96fc5afcdc9) )
	ROM_LOAD( "r5.1l",         0x0800, 0x0800, CRC(11fd2887) SHA1(69844e48bb4d372cac7ae83c953df573c7ecbb7f) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.6e",    0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )
ROM_END


ROM_START( jumpbug )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "jb1",          0x0000, 0x1000, CRC(415aa1b7) SHA1(4f9edd7e9720acf085dd8910849c2f2fac5cb547) )
	ROM_LOAD( "jb2",          0x1000, 0x1000, CRC(b1c27510) SHA1(66fbe0b94b6c101cb50d7a3ff78160110415dff9) )
	ROM_LOAD( "jb3",          0x2000, 0x1000, CRC(97c24be2) SHA1(1beb9fbc3a52610b416af8b5fee156d8b6b3125a) )
	ROM_LOAD( "jb4",          0x3000, 0x1000, CRC(66751d12) SHA1(26c68cfb59596ae164ee9ae4a24ddf8dc7a923a7) )
	ROM_LOAD( "jb5",          0x8000, 0x1000, CRC(e2d66faf) SHA1(3dec0796642856359de57afb896cc668c0245b40) )
	ROM_LOAD( "jb6",          0x9000, 0x1000, CRC(49e0bdfd) SHA1(8d89d9cd7134b153264fdc49d2c68e8c14004b0d) )
	ROM_LOAD( "jb7",          0xa000, 0x0800, CRC(83d71302) SHA1(9292088d26ba29fbf8817df03461b8bb6bf27639) )

	ROM_REGION( 0x3000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "jbl",          0x0000, 0x0800, CRC(9a091b0a) SHA1(19b88f802ee80ff8901ef99e3688f2869f1a69c5) )
	ROM_LOAD( "jbm",          0x0800, 0x0800, CRC(8a0fc082) SHA1(58b72a3161950a2fb71cdab3f30bb3abb19c7978) )
	ROM_LOAD( "jbn",          0x1000, 0x0800, CRC(155186e0) SHA1(717ddaecc52a4ef03a01fcddb520acdbfb0d722a) )
	ROM_LOAD( "jbi",          0x1800, 0x0800, CRC(7749b111) SHA1(55071ce04708bd52177644298f76ae79d23f6ac9) )
	ROM_LOAD( "jbj",          0x2000, 0x0800, CRC(06e8d7df) SHA1(d04f1503d9fde5aae92652cb9d2eb16bd6a0fe9c) )
	ROM_LOAD( "jbk",          0x2800, 0x0800, CRC(b8dbddf3) SHA1(043de444890a93459789dc99c43ef88ff66b79e4) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "l06_prom.bin", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( jumpbugb )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "jb1",          0x0000, 0x1000, CRC(415aa1b7) SHA1(4f9edd7e9720acf085dd8910849c2f2fac5cb547) )
	ROM_LOAD( "jb2",          0x1000, 0x1000, CRC(b1c27510) SHA1(66fbe0b94b6c101cb50d7a3ff78160110415dff9) )
	ROM_LOAD( "jb3b",         0x2000, 0x1000, CRC(cb8b8a0f) SHA1(9e8591471dda2cb964ba2a866d4a5a3ef65d8707) )
	ROM_LOAD( "jb4",          0x3000, 0x1000, CRC(66751d12) SHA1(26c68cfb59596ae164ee9ae4a24ddf8dc7a923a7) )
	ROM_LOAD( "jb5b",         0x8000, 0x1000, CRC(7553b5e2) SHA1(6439585e713581dd36cea6324414f803d683216f) )
	ROM_LOAD( "jb6b",         0x9000, 0x1000, CRC(47be9843) SHA1(495d6fc732267bfd19a953b0b70df3f94b3c1e38) )
	ROM_LOAD( "jb7b",         0xa000, 0x0800, CRC(460aed61) SHA1(449ab1bb502f98da74c0955ce1364f8708fd3f81) )

	ROM_REGION( 0x3000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "jbl",          0x0000, 0x0800, CRC(9a091b0a) SHA1(19b88f802ee80ff8901ef99e3688f2869f1a69c5) )
	ROM_LOAD( "jbm",          0x0800, 0x0800, CRC(8a0fc082) SHA1(58b72a3161950a2fb71cdab3f30bb3abb19c7978) )
	ROM_LOAD( "jbn",          0x1000, 0x0800, CRC(155186e0) SHA1(717ddaecc52a4ef03a01fcddb520acdbfb0d722a) )
	ROM_LOAD( "jbi",          0x1800, 0x0800, CRC(7749b111) SHA1(55071ce04708bd52177644298f76ae79d23f6ac9) )
	ROM_LOAD( "jbj",          0x2000, 0x0800, CRC(06e8d7df) SHA1(d04f1503d9fde5aae92652cb9d2eb16bd6a0fe9c) )
	ROM_LOAD( "jbk",          0x2800, 0x0800, CRC(b8dbddf3) SHA1(043de444890a93459789dc99c43ef88ff66b79e4) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "l06_prom.bin", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( levers )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "g96059.a8", 	  0x0000, 0x1000, CRC(9550627a) SHA1(3da9a614622d5b880852fe2bb2e8e4a60afb2d34) )
	ROM_LOAD( "g96060.d8", 	  0x2000, 0x1000, CRC(5ac64646) SHA1(459755932a033095eff72d78d1e916932964c5cc) )
	ROM_LOAD( "g96061.e8", 	  0x3000, 0x1000, CRC(9db8e520) SHA1(1ff10e221e45cc4afb77571a171937f8501aa509) )
	ROM_LOAD( "g96062.h8", 	  0x8000, 0x1000, CRC(7c8e8b3a) SHA1(ad281f801e818ea529be8ec43096212e834f69ef) )
	ROM_LOAD( "g96063.j8", 	  0x9000, 0x1000, CRC(fa61e793) SHA1(7aad77f3de05a7bd3dcb0c9c97a3cccd1136f352) )
	ROM_LOAD( "g96064.l8", 	  0xa000, 0x1000, CRC(f797f389) SHA1(b961f0506defa9884ac47b2316884318e1e90bff) )

	ROM_REGION( 0x3000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "g95948.n1", 	  0x0000, 0x0800, CRC(d8a0c692) SHA1(dd64623f4072bcb8c528b5b7b95a7bd858b79d6c) )
							/*0x0800- 0x0fff empty */
	ROM_LOAD( "g95949.s1", 	  0x1000, 0x0800, CRC(3660a552) SHA1(bebfd30f90da55d6d42945717b9b38d5b0c9623a) )
	ROM_LOAD( "g95946.j1", 	  0x1800, 0x0800, CRC(73b61b2d) SHA1(fdb75eea1778daa6f9c48243361e418044b471f8) )
							/*0x2000- 0x27ff empty */
	ROM_LOAD( "g95947.m1", 	  0x2800, 0x0800, CRC(72ff67e2) SHA1(dcc12f17a857271c253d06d5ac170b9d6bb6a2bd) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "g960lev.clr",  0x0000, 0x0020, CRC(01febbbe) SHA1(11b1dab7983ba29e830ccb7f14eb1a99465c9e81) )
ROM_END

ROM_START( azurian )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "pgm.1",        0x0000, 0x1000, CRC(17a0fca7) SHA1(0ffb80d433fbaa0631d0d982a453f9e6cccab297) )
	ROM_LOAD( "pgm.2",        0x1000, 0x1000, CRC(14659848) SHA1(bb9d9c01b074bf7ed7a1c29379bbef41728dd27a) )
	ROM_LOAD( "pgm.3",        0x2000, 0x1000, CRC(8f60fb97) SHA1(d0f4d65e568ac1a5d41e550f2f626cbf72884959) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "gfx.1",        0x0000, 0x0800, CRC(f5afb803) SHA1(ffc8f86a35179d7715ef618004b79003e0236a93) )
	ROM_LOAD( "gfx.2",        0x0800, 0x0800, CRC(ae96e5d1) SHA1(df667fb96d7353ccf9ce0acf788371ef2221e97d) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "l06_prom.bin", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( orbitron )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "orbitron.3",   0x0600, 0x0200, CRC(419f9c9b) SHA1(788a3920f4270b886b3a578f8c2df33e6314a1c3) )
	ROM_CONTINUE(			  0x0400, 0x0200)
	ROM_CONTINUE(			  0x0200, 0x0200)


	ROM_CONTINUE(			  0x0000, 0x0200)
	ROM_LOAD( "orbitron.4",   0x0e00, 0x0200, CRC(44ad56ac) SHA1(3a8339cdee50912a16ac0fb448e6659e32542c0c) )
	ROM_CONTINUE(			  0x0c00, 0x0200)
	ROM_CONTINUE(			  0x0a00, 0x0200)
	ROM_CONTINUE(			  0x0800, 0x0200)
	ROM_LOAD( "orbitron.1",   0x1600, 0x0200, CRC(da3f5168) SHA1(1927cc7cd3b9d15b629e09781557f4c75d684182) )
	ROM_CONTINUE(			  0x1400, 0x0200)
	ROM_CONTINUE(			  0x1200, 0x0200)
	ROM_CONTINUE(			  0x1000, 0x0200)
	ROM_LOAD( "orbitron.2",   0x1e00, 0x0200, CRC(a3b813fc) SHA1(7f0f22667bee897b474fb485d65a74d74a36991a) )
	ROM_CONTINUE(			  0x1c00, 0x0200)
	ROM_CONTINUE(			  0x1a00, 0x0200)
	ROM_CONTINUE(			  0x1800, 0x0200)
	ROM_LOAD( "orbitron.5",   0x2000, 0x0800, CRC(20cd8bb8) SHA1(a5309cb04a656c6e1e18bb19910474af8ef814a5) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "orbitron.6",   0x0000, 0x0800, CRC(2c91b83f) SHA1(29c73b7ad0dc5a3ba739492c902ad9201eae6ef2) )
	ROM_LOAD( "orbitron.7",   0x0800, 0x0800, CRC(46f4cca4) SHA1(e5fb616b1d17b5b5167f05f7840638840deb2d13) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "l06_prom.bin", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( checkman )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "cm1",          0x0000, 0x0800, CRC(e8cbdd28) SHA1(ba0b41e375b94bbfed6a2c949cc7958474c8ba6e) )
	ROM_LOAD( "cm2",          0x0800, 0x0800, CRC(b8432d4d) SHA1(d331476f1f88b7ef1426bed7442392f369e0650b) )
	ROM_LOAD( "cm3",          0x1000, 0x0800, CRC(15a97f61) SHA1(3c06c734cef1eed68b401d0d36f7ec9126986d73) )
	ROM_LOAD( "cm4",          0x1800, 0x0800, CRC(8c12ecc0) SHA1(1c2d61ef84404b6a524c453a3d339aaaadb38229) )
	ROM_LOAD( "cm5",          0x2000, 0x0800, CRC(2352cfd6) SHA1(77db5f925ee5f83c17a05a78af5191eefe70ca5a) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* 64k for sound code */
	ROM_LOAD( "cm13",         0x0000, 0x0800, CRC(0b09a3e8) SHA1(e4e65da306e22f61790f0a68d953cc017c3ce762) )
	ROM_LOAD( "cm14",         0x0800, 0x0800, CRC(47f043be) SHA1(44d8892d93849cbc989561387c0a05baead58446) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "cm11",         0x0000, 0x0800, CRC(8d1bcca0) SHA1(28fc7fb76180820e84d59e6836ed1f8136e8f138) )
	ROM_RELOAD(	              0x0800, 0x0800 )
	ROM_LOAD( "cm9",          0x1000, 0x0800, CRC(3cd5c751) SHA1(a769fdd30752da8fb331aa0f7a0181a93f0b3378) )
	ROM_RELOAD(	              0x1800, 0x0800 )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "checkman.clr", 0x0000, 0x0020, CRC(57a45057) SHA1(d4ec6a54f72640e5b485aa59e206c090e67ff640) )
ROM_END

ROM_START( checkmaj )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "cm_1.bin",     0x0000, 0x1000, CRC(456a118f) SHA1(7c2e8343360f446af4391012784a1ccfecae3299) )
	ROM_LOAD( "cm_2.bin",     0x1000, 0x1000, CRC(146b2c44) SHA1(80455396a9b1802fcefaec1340b76461c0601bf9) )
	ROM_LOAD( "cm_3.bin",     0x2000, 0x0800, CRC(73e1c945) SHA1(bcf2558958a30e5936f19ff53687f2316e0b822e) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* 64k for sound code */
	ROM_LOAD( "cm_4.bin",     0x0000, 0x1000, CRC(923cffa1) SHA1(132822d20de2ad1ecc561e811ca40c5642500631) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "cm_6.bin",     0x0000, 0x0800, CRC(476a7cc3) SHA1(3c343b0dcfb2f4cbec2f8b5854a303a1660fea22) )
	ROM_LOAD( "cm_5.bin",     0x0800, 0x0800, CRC(b3df2b5f) SHA1(519a0894d1794211659abeb6b2a2c610e6c2af25) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "checkman.clr", 0x0000, 0x0020, CRC(57a45057) SHA1(d4ec6a54f72640e5b485aa59e206c090e67ff640) )
ROM_END

ROM_START( dingo )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "003.e7",       0x0000, 0x1000, CRC(d088550f) SHA1(13b87995881e484825c45ab4e558ac3d90bf162a) )
	ROM_LOAD( "004.h7",       0x1000, 0x1000, CRC(a228446a) SHA1(4b7e611edd6bce308cc7b17caa068445f5438f4f) )
	ROM_LOAD( "005.j7",       0x2000, 0x0800, CRC(14d680bb) SHA1(e9d84d1a62ed5300c390a7326c16cebd0aceae3b) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* 64k for sound code */
	ROM_LOAD( "6.7l",         0x0000, 0x1000, CRC(047092e0) SHA1(24014c999c904b4be571121b0f6808713d95add1) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "001.h1",       0x0000, 0x0800, CRC(1ab1dd4d) SHA1(74ef2226e1f1d2583b0c7718325da193f411a97d) )
	ROM_LOAD( "002.k1",       0x0800, 0x0800, CRC(4be375ee) SHA1(7379b037887baca0f932d910f8f94f7edf39bb26) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "18s030.l6",	  0x0000, 0x0020, CRC(3061d0f9) SHA1(5af85499c6219137dc57d9fba79cb5afa3548ab1) )
ROM_END

ROM_START( dingoe )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "unk.2b",       0x0000, 0x1000, CRC(0df7ac6d) SHA1(c1d45a7694848e66426c3510d0749c98e51571cb) )
	ROM_LOAD( "unk.2d",       0x1000, 0x1000, CRC(0881e204) SHA1(4ba59d73e04b5337cfbd68d6a708e7321cb629f1) )
	ROM_LOAD( "unk.3b",       0x2000, 0x1000, BAD_DUMP CRC(0b6aeab5) SHA1(ebfab3227dd23e3e1802b881a5662f634f86e382) ) // both halves identical (bad?)

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* 64k for sound code */
	ROM_LOAD( "unk.1c",       0x0000, 0x0800, CRC(8e354c38) SHA1(87608c1fa55e6fcf482f5d3bcc506a84673719cc) )
	ROM_LOAD( "unk.1d",       0x0800, 0x0800, CRC(092878d6) SHA1(8a3b25e27df5aee2023a7e1a193ab152df171ede) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "unk.4d",       0x0000, 0x0800, CRC(76a00a56) SHA1(2a696b9ce3e148529c731231852dc104729bb916) )
	ROM_LOAD( "unk.4b",       0x0800, 0x0800, CRC(5acf57aa) SHA1(bb05be53728e7867085dad5854fcadfa687ff5d7) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "82s123n.001",  0x0000, 0x0020, CRC(02b11865) SHA1(70053db9635a9194e4372835379a82f6ea64ef83) ) /* Unknown */
ROM_END


ROM_START( blkhole )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "bh1",          0x0000, 0x0800, CRC(64998819) SHA1(69fe5dfbe6cde18ef4cae62da12b5c692c2c72b9) )
	ROM_LOAD( "bh2",          0x0800, 0x0800, CRC(26f26ce4) SHA1(720ce7af05ef596fb9a109591534c74d282955e8) )
	ROM_LOAD( "bh3",          0x1000, 0x0800, CRC(3418bc45) SHA1(088bbbde66b7b5c36fa48cf14c22146e1444e67c) )
	ROM_LOAD( "bh4",          0x1800, 0x0800, CRC(735ff481) SHA1(d9b32db048a0e2a1195cd6f7326005e6622242a9) )
	ROM_LOAD( "bh5",          0x2000, 0x0800, CRC(3f657be9) SHA1(3ed1ee0bc199c1625156d2771eecd18a57a0e6ed) )
	ROM_LOAD( "bh6",          0x2800, 0x0800, CRC(a057ab35) SHA1(430261bafe20fc182e6e6659019cf42643e95d54) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "bh7",          0x0000, 0x0800, CRC(975ba821) SHA1(c50d55f6ab81b803d67f5e18c1243ef85a1a2df1) )
	ROM_LOAD( "bh8",          0x0800, 0x0800, CRC(03d11020) SHA1(5768b573fac9aac168db2723462cca76d4d80552) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( mooncrst )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "mc1",          0x0000, 0x0800, CRC(7d954a7a) SHA1(a93ee403cfd7887538ad12d33f6dd6c71bea2a32) )
	ROM_LOAD( "mc2",          0x0800, 0x0800, CRC(44bb7cfa) SHA1(349c2e23a9fce73f95bb8168d369082fa129fe3d) )
	ROM_LOAD( "mc3",          0x1000, 0x0800, CRC(9c412104) SHA1(1b40054ebb1ace965a8522119bb23f09797bc5f6) )
	ROM_LOAD( "mc4",          0x1800, 0x0800, CRC(7e9b1ab5) SHA1(435f603c0c3e788a509dd144a7916a34e791ae44) )
	ROM_LOAD( "mc5.7r",       0x2000, 0x0800, CRC(16c759af) SHA1(3b48050411f65f9d3fb41ff22901e22d82bf1cf6) )
	ROM_LOAD( "mc6.8d",       0x2800, 0x0800, CRC(69bcafdb) SHA1(939c8c6ed1cd4660a3d99b8f17ed99cbd7e1352a) )
	ROM_LOAD( "mc7.8e",       0x3000, 0x0800, CRC(b50dbc46) SHA1(4fa084fd1ba5f78e7703e684c57af15ca7a844e4) )
	ROM_LOAD( "mc8",          0x3800, 0x0800, CRC(18ca312b) SHA1(39219059003b949e38305553fea2d33071062c64) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "mcs_b",        0x0000, 0x0800, CRC(fb0f1f81) SHA1(38a6679a8b69bc1870a0e67e692131c42f9535c8) )
	ROM_LOAD( "mcs_d",        0x0800, 0x0800, CRC(13932a15) SHA1(b8885c555c6ad7021be55c6925a0a0872c1b6abd) )
	ROM_LOAD( "mcs_a",        0x1000, 0x0800, CRC(631ebb5a) SHA1(5bc9493afa76c55858b8c8849524cbc77dc838fc) )
	ROM_LOAD( "mcs_c",        0x1800, 0x0800, CRC(24cfd145) SHA1(08c6599db170dd6ee364c44f70a0f5c0f881b6ef) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "mmi6331.6l", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END

ROM_START( mooncrsu )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "smc1f",        0x0000, 0x0800, CRC(389ca0d6) SHA1(51cf6d190a0ebf23b70c2bcf1ccaa4705e29cd09) )
	ROM_LOAD( "smc2f",        0x0800, 0x0800, CRC(410ab430) SHA1(d89abff6ac4afbf69377a1d63043d629a634aab7) )
	ROM_LOAD( "smc3f",        0x1000, 0x0800, CRC(a6b4144b) SHA1(2b27ad54d716286c0dc9476d47df182ae01bcfd7) )
	ROM_LOAD( "smc4f",        0x1800, 0x0800, CRC(4cc046fe) SHA1(465eaacd50967d768babadd09ab9cad35380f6bf) )
	ROM_LOAD( "e5",       	  0x2000, 0x0800, CRC(06d378a6) SHA1(99dbe9fc7f95f8fdce86eb5c32bd1ca1bea0ca3c) )
	ROM_LOAD( "bepr199",      0x2800, 0x0800, CRC(6e84a927) SHA1(82e8e825d157c3c947a3a222bca059a735169c7d) )
	ROM_LOAD( "e7",           0x3000, 0x0800, CRC(b45af1e8) SHA1(d7020774707234acdaef5c655f667d5ee9e54a13) )
	ROM_LOAD( "smc8f",        0x3800, 0x0800, CRC(f42164c5) SHA1(e0d1680f193889568edf005786e2767d4fb086f4) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "mcs_b",        0x0000, 0x0800, CRC(fb0f1f81) SHA1(38a6679a8b69bc1870a0e67e692131c42f9535c8) )
	ROM_LOAD( "mcs_d",        0x0800, 0x0800, CRC(13932a15) SHA1(b8885c555c6ad7021be55c6925a0a0872c1b6abd) )
	ROM_LOAD( "mcs_a",        0x1000, 0x0800, CRC(631ebb5a) SHA1(5bc9493afa76c55858b8c8849524cbc77dc838fc) )
	ROM_LOAD( "mcs_c",        0x1800, 0x0800, CRC(24cfd145) SHA1(08c6599db170dd6ee364c44f70a0f5c0f881b6ef) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "mmi6331.6l", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END

ROM_START( mooncrsa )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "mc1.7d",       0x0000, 0x0800, CRC(92a86aac) SHA1(f5818ac97d8b779e1fb29bf903f74185d24afb0d) )
	ROM_LOAD( "mc2.7e",       0x0800, 0x0800, CRC(438c2b4b) SHA1(11f56b489b5489999952e91919c5e1f622c59c36) )
	ROM_LOAD( "mc3.7j",       0x1000, 0x0800, CRC(67e3d21d) SHA1(59579d19931ef11b30fdc3912d838200bef92c81) )
	ROM_LOAD( "mc4.7p",       0x1800, 0x0800, CRC(f4db39f6) SHA1(454931f80b35608793590b3843c69ba64cbf6772) )
	ROM_LOAD( "mc5.7r",       0x2000, 0x0800, CRC(16c759af) SHA1(3b48050411f65f9d3fb41ff22901e22d82bf1cf6) )
	ROM_LOAD( "mc6.8d",       0x2800, 0x0800, CRC(69bcafdb) SHA1(939c8c6ed1cd4660a3d99b8f17ed99cbd7e1352a) )
	ROM_LOAD( "mc7.8e",       0x3000, 0x0800, CRC(b50dbc46) SHA1(4fa084fd1ba5f78e7703e684c57af15ca7a844e4) )
	ROM_LOAD( "mc8.8h",       0x3800, 0x0800, CRC(7e2b1928) SHA1(4f0de8e80c2e2ec6df8612755caf93671ea965b0) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "mcs_b",        0x0000, 0x0800, CRC(fb0f1f81) SHA1(38a6679a8b69bc1870a0e67e692131c42f9535c8) )
	ROM_LOAD( "mcs_d",        0x0800, 0x0800, CRC(13932a15) SHA1(b8885c555c6ad7021be55c6925a0a0872c1b6abd) )
	ROM_LOAD( "mcs_a",        0x1000, 0x0800, CRC(631ebb5a) SHA1(5bc9493afa76c55858b8c8849524cbc77dc838fc) )
	ROM_LOAD( "mcs_c",        0x1800, 0x0800, CRC(24cfd145) SHA1(08c6599db170dd6ee364c44f70a0f5c0f881b6ef) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "mmi6331.6l", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END

ROM_START( mooncrsg )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "epr194",       0x0000, 0x0800, CRC(0e5582b1) SHA1(946ad4aeb10c0b7b3f93fd24925cc9bcb49e443c) )
	ROM_LOAD( "epr195",       0x0800, 0x0800, CRC(12cb201b) SHA1(ebb01ec646b9e015cbcb93f70dfdaf448afefc12) )
	ROM_LOAD( "epr196",       0x1000, 0x0800, CRC(18255614) SHA1(b373e22d47c0f7facba13148ca9c462ec9a0d732) )
	ROM_LOAD( "epr197",       0x1800, 0x0800, CRC(05ac1466) SHA1(cbf93a8ce0925fa1c073c74f1274b190d9faefaf) )
	ROM_LOAD( "epr198",       0x2000, 0x0800, CRC(c28a2e8f) SHA1(9ff6bab1e1185597ba55cb0d6086091a1fce01a6) )
	ROM_LOAD( "epr199",       0x2800, 0x0800, CRC(5a4571de) SHA1(2a4170dee105922fc69c99b79f6f328098e81918) )
	ROM_LOAD( "epr200",       0x3000, 0x0800, CRC(b7c85bf1) SHA1(cc9f593658ea39c849d80c83ee0c2170cc29879e) )
	ROM_LOAD( "epr201",       0x3800, 0x0800, CRC(2caba07f) SHA1(8fec4904e12b4cfb6068784007278be986a3eede) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "epr203",       0x0000, 0x0800, CRC(be26b561) SHA1(cc27de6888eaf4ee18c0d37d9bcb528dd282b838) )
	ROM_LOAD( "mcs_d",        0x0800, 0x0800, CRC(13932a15) SHA1(b8885c555c6ad7021be55c6925a0a0872c1b6abd) )
	ROM_LOAD( "epr202",       0x1000, 0x0800, CRC(26c7e800) SHA1(034192e5e2cbac4b66a9828f5ec2311c2c368781) )
	ROM_LOAD( "mcs_c",        0x1800, 0x0800, CRC(24cfd145) SHA1(08c6599db170dd6ee364c44f70a0f5c0f881b6ef) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "mmi6331.6l", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END

ROM_START( smooncrs )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "927",          0x0000, 0x0800, CRC(55c5b994) SHA1(3451b121fa22361b2684385cf5d4455fa6963215) )
	ROM_LOAD( "928a",         0x0800, 0x0800, CRC(77ae26d3) SHA1(cbc16a024b73bedff76a6c47336d6ef098e92c53) )
	ROM_LOAD( "929",          0x1000, 0x0800, CRC(716eaa10) SHA1(780fc785e6651f19dc1a0ccf48cf9485d6562a71) )
	ROM_LOAD( "930",          0x1800, 0x0800, CRC(cea864f2) SHA1(aaaf9f8dd126dfb4a4f52f39863fee02a56a6485) )
	ROM_LOAD( "931",          0x2000, 0x0800, CRC(702c5f51) SHA1(5ba8d87c93c4810b8e7c2ad4ee376cd806e83686) )
	ROM_LOAD( "932a",         0x2800, 0x0800, CRC(e6a2039f) SHA1(f0f240dd8ac7cd2d9994cb7341b59d7a0a3eaf26) )
	ROM_LOAD( "933",          0x3000, 0x0800, CRC(73783cee) SHA1(69760e25ba22645572ec16b4f9136ee84ed0c766) )
	ROM_LOAD( "934",          0x3800, 0x0800, CRC(c1a14aa2) SHA1(99f6b01a0acd5e936d6ae61c13599db603b73191) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "epr203",       0x0000, 0x0800, CRC(be26b561) SHA1(cc27de6888eaf4ee18c0d37d9bcb528dd282b838) )
	ROM_LOAD( "mcs_d",        0x0800, 0x0800, CRC(13932a15) SHA1(b8885c555c6ad7021be55c6925a0a0872c1b6abd) )
	ROM_LOAD( "epr202",       0x1000, 0x0800, CRC(26c7e800) SHA1(034192e5e2cbac4b66a9828f5ec2311c2c368781) )
	ROM_LOAD( "mcs_c",        0x1800, 0x0800, CRC(24cfd145) SHA1(08c6599db170dd6ee364c44f70a0f5c0f881b6ef) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "mmi6331.6l", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END

ROM_START( mooncrsb )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "bepr194",      0x0000, 0x0800, CRC(6a23ec6d) SHA1(df2214bdde26a71db59ffd39a745052076563f65) )
	ROM_LOAD( "bepr195",      0x0800, 0x0800, CRC(ee262ff2) SHA1(4e2202023ad53109ea58304071735d2425a617f3) )
	ROM_LOAD( "f03.bin",      0x1000, 0x0800, CRC(29a2b0ab) SHA1(e9fc7161d0566e36307c45b7132e2262c0af4845) )
	ROM_LOAD( "f04.bin",      0x1800, 0x0800, CRC(4c6a5a6d) SHA1(366516f63c9b5239e703e4dfb672659049ddbf44) )
	ROM_LOAD( "e5",           0x2000, 0x0800, CRC(06d378a6) SHA1(99dbe9fc7f95f8fdce86eb5c32bd1ca1bea0ca3c) )
	ROM_LOAD( "bepr199",      0x2800, 0x0800, CRC(6e84a927) SHA1(82e8e825d157c3c947a3a222bca059a735169c7d) )
	ROM_LOAD( "e7",           0x3000, 0x0800, CRC(b45af1e8) SHA1(d7020774707234acdaef5c655f667d5ee9e54a13) )
	ROM_LOAD( "bepr201",      0x3800, 0x0800, CRC(66da55d5) SHA1(39e2f6107e77ee97860147f64b9673cd9a2ae612) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "epr203",       0x0000, 0x0800, CRC(be26b561) SHA1(cc27de6888eaf4ee18c0d37d9bcb528dd282b838) )
	ROM_LOAD( "mcs_d",        0x0800, 0x0800, CRC(13932a15) SHA1(b8885c555c6ad7021be55c6925a0a0872c1b6abd) )
	ROM_LOAD( "epr202",       0x1000, 0x0800, CRC(26c7e800) SHA1(034192e5e2cbac4b66a9828f5ec2311c2c368781) )
	ROM_LOAD( "mcs_c",        0x1800, 0x0800, CRC(24cfd145) SHA1(08c6599db170dd6ee364c44f70a0f5c0f881b6ef) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "mmi6331.6l", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END

ROM_START( mooncrs2 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "f8.bin",       0x0000, 0x0800, CRC(d36003e5) SHA1(562b27f1bccce6ae29de18b93fa51c508446cda9) )
	ROM_LOAD( "bepr195",      0x0800, 0x0800, CRC(ee262ff2) SHA1(4e2202023ad53109ea58304071735d2425a617f3) )
	ROM_LOAD( "f03.bin",      0x1000, 0x0800, CRC(29a2b0ab) SHA1(e9fc7161d0566e36307c45b7132e2262c0af4845) )
	ROM_LOAD( "f04.bin",      0x1800, 0x0800, CRC(4c6a5a6d) SHA1(366516f63c9b5239e703e4dfb672659049ddbf44) )
	ROM_LOAD( "e5",           0x2000, 0x0800, CRC(06d378a6) SHA1(99dbe9fc7f95f8fdce86eb5c32bd1ca1bea0ca3c) )
	ROM_LOAD( "bepr199",      0x2800, 0x0800, CRC(6e84a927) SHA1(82e8e825d157c3c947a3a222bca059a735169c7d) )
	ROM_LOAD( "e7",           0x3000, 0x0800, CRC(b45af1e8) SHA1(d7020774707234acdaef5c655f667d5ee9e54a13) )
	ROM_LOAD( "m7.bin",       0x3800, 0x0800, CRC(957ee078) SHA1(472038dedfc01c995be889ea93d4df8bef2b874c) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "1h_1_10.bin",  0x0000, 0x0800, CRC(528da705) SHA1(d726ee18b79774c982f88afb2a508eb5d5783193) )
	ROM_LOAD( "12.chr",       0x0800, 0x0200, CRC(5a4b17ea) SHA1(8a879dc34fdecc8a121c4a87abb981212fb05945) )
	ROM_CONTINUE(             0x0c00, 0x0200 )	/* this version of the gfx ROMs has two */
	ROM_CONTINUE(             0x0a00, 0x0200 )	/* groups of 16 sprites swapped */
	ROM_CONTINUE(             0x0e00, 0x0200 )
	ROM_LOAD( "1k_1_11.bin",  0x1000, 0x0800, CRC(4e79ff6b) SHA1(f72386a3766a7fcc7b4b8cedfa58b8d57f911f6f) )
	ROM_LOAD( "11.chr",       0x1800, 0x0200, CRC(e0edccbd) SHA1(0839a4c9b6e863d12253ae8e1732e80e08702228) )
	ROM_CONTINUE(             0x1c00, 0x0200 )
	ROM_CONTINUE(             0x1a00, 0x0200 )
	ROM_CONTINUE(             0x1e00, 0x0200 )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "mmi6331.6l", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END

ROM_START( mooncrs3 ) /* Bootleg by Jeutel, very simular to Moon Cresta (bootleg set 2) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "b1.7f",  0x0000, 0x0800, CRC(0b28cd8a) SHA1(a1aa0ec63e1dddf4263aa39f6a5fda93108b6e98) )
	ROM_CONTINUE(       0x2000, 0x0800 )
	ROM_LOAD( "b2.7h",  0x0800, 0x0800, CRC(74a6f0ca) SHA1(cc8e8193bb6bd62f6cb9ea924e4da5ddc44c4685) )
	ROM_CONTINUE(       0x2800, 0x0800 )
	ROM_LOAD( "b3.7j",  0x1000, 0x0800, CRC(eeb34cc9) SHA1(c5e7d5e1989211be949972e4281403b7b4866922) )
	ROM_CONTINUE(       0x3000, 0x0800 )
	ROM_LOAD( "b4.7k",  0x1800, 0x0800, CRC(714330e5) SHA1(c681752732c73a6c9bcc9acdcd5c978c455acba0) )
	ROM_CONTINUE(       0x3800, 0x0800 )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "o.1h",  0x0000, 0x0800, CRC(528da705) SHA1(d726ee18b79774c982f88afb2a508eb5d5783193) )
	ROM_LOAD( "q.1h",  0x0800, 0x0200, CRC(5a4b17ea) SHA1(8a879dc34fdecc8a121c4a87abb981212fb05945) )
	ROM_CONTINUE(      0x0c00, 0x0200 )	/* this version of the gfx ROMs has two */
	ROM_CONTINUE(      0x0a00, 0x0200 )	/* groups of 16 sprites swapped */
	ROM_CONTINUE(      0x0e00, 0x0200 )
	ROM_LOAD( "p.1k",  0x1000, 0x0800, CRC(4e79ff6b) SHA1(f72386a3766a7fcc7b4b8cedfa58b8d57f911f6f) )
	ROM_LOAD( "r.1k",  0x1800, 0x0200, CRC(e0edccbd) SHA1(0839a4c9b6e863d12253ae8e1732e80e08702228) )
	ROM_CONTINUE(      0x1c00, 0x0200 )
	ROM_CONTINUE(      0x1a00, 0x0200 )
	ROM_CONTINUE(      0x1e00, 0x0200 )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "mmi6331.6l", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END


ROM_START( mooncmw )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "60.1x",      0x0000, 0x0800, CRC(322859e6) SHA1(292dccb66c38c8de837ec3ac10928d092494958e) )
	ROM_LOAD( "61.2x",      0x0800, 0x0800, CRC(c249902d) SHA1(0015461173fb991fd99c824e0eab054c3c17d0f1) )
	ROM_LOAD( "62.3x",      0x1000, 0x0800, CRC(29a2b0ab) SHA1(e9fc7161d0566e36307c45b7132e2262c0af4845) )
	ROM_LOAD( "63.4x",      0x1800, 0x0800, CRC(4c6a5a6d) SHA1(366516f63c9b5239e703e4dfb672659049ddbf44) )
	ROM_LOAD( "64.5x",      0x2000, 0x0800, CRC(06d378a6) SHA1(99dbe9fc7f95f8fdce86eb5c32bd1ca1bea0ca3c) )
	ROM_LOAD( "65.6x",      0x2800, 0x0800, CRC(6e84a927) SHA1(82e8e825d157c3c947a3a222bca059a735169c7d) )
	ROM_LOAD( "66.7x",      0x3000, 0x0800, CRC(f23cd8ce) SHA1(a77e7eca239de6a72a8cabed6444ae8efb9e40bd) )
	ROM_LOAD( "67.8x",      0x3800, 0x0800, CRC(66da55d5) SHA1(39e2f6107e77ee97860147f64b9673cd9a2ae612) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "68.1h",      0x0000, 0x1000, CRC(78663d86) SHA1(8648a3e60259404a05ad58b1641190e5b33a24eb) )
	ROM_LOAD( "69.1k",      0x1000, 0x1000, CRC(162c50d3) SHA1(67d9c87782cf29c443590d7ad687fbeaa6218346) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "prom-sn74s288n-71.6l", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END

ROM_START( spcdrag )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "a.bin",      0x0000, 0x0800, CRC(38cc9839) SHA1(71c5853fc14a9c0b93e3b7660b925021680a0fe1) )
	ROM_LOAD( "b.bin",      0x0800, 0x0800, CRC(419fa8d6) SHA1(709b096d43c15cbfb98745e1f5e7c1bc921e3241) )
	ROM_LOAD( "c.bin",      0x1000, 0x0800, CRC(a1939def) SHA1(c9be93d325dde496d89e0735ec4e7abca932c0f6) )
	ROM_LOAD( "d.bin",      0x1800, 0x0800, CRC(cbcf17c5) SHA1(9aa3ca6dc30e4a19ed2bdb2be6ba90bde4cb7542) )
	ROM_LOAD( "em.bin",     0x2000, 0x0800, CRC(eb81c19c) SHA1(e5dd61704938c837b87a3155d54698482235c513) )
	ROM_LOAD( "fm.bin",     0x2800, 0x0800, CRC(757b7672) SHA1(d042e4bc17d2a8c9f1db55d57d5c235338cdb20c) )
	ROM_LOAD( "g.bin",      0x3000, 0x0800, CRC(57713b91) SHA1(ba01ed3f047ebbd0f9e6956e649bec0e8b730a45) )
	ROM_LOAD( "h.bin",      0x3800, 0x0800, CRC(159ad847) SHA1(9d46f380c868ac07964e571c54e800c683a6a679) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "203.bin",  0x0000, 0x0800, CRC(a2e82527) SHA1(5e9236ba102728213b4651db984b3a169b4a0410) )
	ROM_LOAD( "172.bin",  0x0800, 0x0800, CRC(13932a15) SHA1(b8885c555c6ad7021be55c6925a0a0872c1b6abd) )
	ROM_LOAD( "202.bin",  0x1000, 0x0800, CRC(80c3ad74) SHA1(0fd2269543d123bd427f5a648a17f8bee65b20a2) )
	ROM_LOAD( "171.bin",  0x1800, 0x0800, CRC(24cfd145) SHA1(08c6599db170dd6ee364c44f70a0f5c0f881b6ef) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	// not present in this set
	ROM_LOAD( "mmi6331.6l", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END

ROM_START( spcdraga )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "1.7g",      0x0000, 0x0800, CRC(38cc9839) SHA1(71c5853fc14a9c0b93e3b7660b925021680a0fe1) )
	ROM_LOAD( "2.7g",      0x0800, 0x0800, CRC(29e00ae4) SHA1(574bdfb621e084485e6621229cd569486831e4ba) )
	ROM_LOAD( "3.7g",      0x1000, 0x0800, CRC(a1939def) SHA1(c9be93d325dde496d89e0735ec4e7abca932c0f6) )
	ROM_LOAD( "4.7g",      0x1800, 0x0800, CRC(068f8830) SHA1(e12d590401878d9f2695e5c7aa38387ed9ccfb06) )
	ROM_LOAD( "5.10g",     0x2000, 0x0800, CRC(32cd9adc) SHA1(3143690712465d092d6c63f4826f220839d78958) )
	ROM_LOAD( "6.10g",     0x2800, 0x0800, CRC(50db67c5) SHA1(69ad219332ac0d9f4e328b314f7bdc34d5599393) )
	ROM_LOAD( "7.10g",     0x3000, 0x0800, CRC(22415271) SHA1(60b1ca2dc044c0863c6f38280a3bd0ff9397c869) )
	ROM_LOAD( "8.10g",     0x3800, 0x0800, CRC(159ad847) SHA1(9d46f380c868ac07964e571c54e800c683a6a679) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "a2.7a",  0x0000, 0x0800, CRC(38b042dd) SHA1(bd452dae4cbc22a900cf783f84d1f9d8cb1218f9) )
	ROM_LOAD( "a4.7a",  0x0800, 0x0200, CRC(5a4b17ea) SHA1(8a879dc34fdecc8a121c4a87abb981212fb05945) )
	ROM_CONTINUE(      0x0c00, 0x0200 )	/* this version of the gfx ROMs has two */
	ROM_CONTINUE(      0x0a00, 0x0200 )	/* groups of 16 sprites swapped */
	ROM_CONTINUE(      0x0e00, 0x0200 )
	ROM_LOAD( "a1.9a",  0x1000, 0x0800, CRC(24441ab3) SHA1(8c9d2bd062cb2360f3dd3df2d7d212e9485f91ad) )
	ROM_LOAD( "a3.9a",  0x1800, 0x0200, CRC(e0edccbd) SHA1(0839a4c9b6e863d12253ae8e1732e80e08702228) )
	ROM_CONTINUE(      0x1c00, 0x0200 )
	ROM_CONTINUE(      0x1a00, 0x0200 )
	ROM_CONTINUE(      0x1e00, 0x0200 )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	// not present in this set
	ROM_LOAD( "prom_6331.10f", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END


ROM_START( fantazia )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "f01.bin",      0x0000, 0x0800, CRC(d3e23863) SHA1(f0a6f7491fdf8aae214f40078b29b7aecdcf2f1e) )
	ROM_LOAD( "f02.bin",      0x0800, 0x0800, CRC(63fa4149) SHA1(603ee6d4d2952cc08b3f6e98b1a2053671875e44) )
	ROM_LOAD( "f03.bin",      0x1000, 0x0800, CRC(29a2b0ab) SHA1(e9fc7161d0566e36307c45b7132e2262c0af4845) )
	ROM_LOAD( "f04.bin",      0x1800, 0x0800, CRC(4c6a5a6d) SHA1(366516f63c9b5239e703e4dfb672659049ddbf44) )
	ROM_LOAD( "f09.bin",      0x2000, 0x0800, CRC(75fd5ca1) SHA1(45f2dd33f0e437cb95d9373f86490e5432338737) )
	ROM_LOAD( "f10.bin",      0x2800, 0x0800, CRC(e4da2dd4) SHA1(7a53efd5b583f656c87b7d7a5ba7c239ced7d87b) )
	ROM_LOAD( "f11.bin",      0x3000, 0x0800, CRC(42869646) SHA1(a3640b2ace31ce99c056bc14d1d96f3404698d6a) )
	ROM_LOAD( "f12.bin",      0x3800, 0x0800, CRC(a48d7fb0) SHA1(6206036a9d85e87fb7f8a88c17bfe090fc70caf4) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "1h_1_10.bin",  0x0000, 0x0800, CRC(528da705) SHA1(d726ee18b79774c982f88afb2a508eb5d5783193) )
	ROM_LOAD( "mcs_d",        0x0800, 0x0800, CRC(13932a15) SHA1(b8885c555c6ad7021be55c6925a0a0872c1b6abd) )
	ROM_LOAD( "1k_1_11.bin",  0x1000, 0x0800, CRC(4e79ff6b) SHA1(f72386a3766a7fcc7b4b8cedfa58b8d57f911f6f) )
	ROM_LOAD( "mcs_c",        0x1800, 0x0800, CRC(24cfd145) SHA1(08c6599db170dd6ee364c44f70a0f5c0f881b6ef) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "fantazia.clr", 0x0000, 0x0020, CRC(a84ff0af) SHA1(c300dc937c608d2d1c113ca7a53c649472c72379) )
ROM_END

ROM_START( eagle )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "e1",           0x0000, 0x0800, CRC(224c9526) SHA1(4c014d60d4ee80de7f60b4609269461688c181d0) )
	ROM_LOAD( "e2",           0x0800, 0x0800, CRC(cc538ebd) SHA1(4ef3c7363e2dcd9ed99779039ccc50a9f2084dbd) )
	ROM_LOAD( "f03.bin",      0x1000, 0x0800, CRC(29a2b0ab) SHA1(e9fc7161d0566e36307c45b7132e2262c0af4845) )
	ROM_LOAD( "f04.bin",      0x1800, 0x0800, CRC(4c6a5a6d) SHA1(366516f63c9b5239e703e4dfb672659049ddbf44) )
	ROM_LOAD( "e5",           0x2000, 0x0800, CRC(06d378a6) SHA1(99dbe9fc7f95f8fdce86eb5c32bd1ca1bea0ca3c) )
	ROM_LOAD( "e6",           0x2800, 0x0800, CRC(0dea20d5) SHA1(405b51d4e3b1065f78afd2297e075e977ae19196) )
	ROM_LOAD( "e7",           0x3000, 0x0800, CRC(b45af1e8) SHA1(d7020774707234acdaef5c655f667d5ee9e54a13) )
	ROM_LOAD( "e8",           0x3800, 0x0800, CRC(c437a876) SHA1(845941b873970ac62ba9bb6353bee53d0fcfa292) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "e10",          0x0000, 0x0800, CRC(40ce58bf) SHA1(67ea99e1afe4fff3e17252b22d11d3c96a416041) )
	ROM_LOAD( "e12",          0x0800, 0x0200, CRC(628fdeed) SHA1(a798530c65e639fbf00ed3a4e8c428935bf5f38e) )
	ROM_CONTINUE(             0x0c00, 0x0200 )	/* this version of the gfx ROMs has two */
	ROM_CONTINUE(             0x0a00, 0x0200 )	/* groups of 16 sprites swapped */
	ROM_CONTINUE(             0x0e00, 0x0200 )
	ROM_LOAD( "e9",           0x1000, 0x0800, CRC(ba664099) SHA1(9509123bed02a9d47f2c056e1562b80206da5579) )
	ROM_LOAD( "e11",          0x1800, 0x0200, CRC(ee4ec5fd) SHA1(bf08b3f111f780dc8c81275e4e6247388183a8da) )
	ROM_CONTINUE(             0x1c00, 0x0200 )
	ROM_CONTINUE(             0x1a00, 0x0200 )
	ROM_CONTINUE(             0x1e00, 0x0200 )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "l06_prom.bin", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( eagle2 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "e1.7f",        0x0000, 0x0800, CRC(45aab7a3) SHA1(52ae0463f363dc0964b976faa2c0c428d85a4f12) )
	ROM_LOAD( "e2",           0x0800, 0x0800, CRC(cc538ebd) SHA1(4ef3c7363e2dcd9ed99779039ccc50a9f2084dbd) )
	ROM_LOAD( "f03.bin",      0x1000, 0x0800, CRC(29a2b0ab) SHA1(e9fc7161d0566e36307c45b7132e2262c0af4845) )
	ROM_LOAD( "f04.bin",      0x1800, 0x0800, CRC(4c6a5a6d) SHA1(366516f63c9b5239e703e4dfb672659049ddbf44) )
	ROM_LOAD( "e5",           0x2000, 0x0800, CRC(06d378a6) SHA1(99dbe9fc7f95f8fdce86eb5c32bd1ca1bea0ca3c) )
	ROM_LOAD( "e6.6",         0x2800, 0x0800, CRC(9f09f8c6) SHA1(47c600629e02357389dd78c7fcaec862e0da4ef0) )
	ROM_LOAD( "e7",           0x3000, 0x0800, CRC(b45af1e8) SHA1(d7020774707234acdaef5c655f667d5ee9e54a13) )
	ROM_LOAD( "e8",           0x3800, 0x0800, CRC(c437a876) SHA1(845941b873970ac62ba9bb6353bee53d0fcfa292) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "e10.2",        0x0000, 0x0800, CRC(25b38ebd) SHA1(f679c2f2cb5892680fec102fafbdfeae156ce373) )
	ROM_LOAD( "e12",          0x0800, 0x0200, CRC(628fdeed) SHA1(a798530c65e639fbf00ed3a4e8c428935bf5f38e) )
	ROM_CONTINUE(             0x0c00, 0x0200 )	/* this version of the gfx ROMs has two */
	ROM_CONTINUE(             0x0a00, 0x0200 )	/* groups of 16 sprites swapped */
	ROM_CONTINUE(             0x0e00, 0x0200 )
	ROM_LOAD( "e9",           0x1000, 0x0800, CRC(ba664099) SHA1(9509123bed02a9d47f2c056e1562b80206da5579) )
	ROM_LOAD( "e11",          0x1800, 0x0200, CRC(ee4ec5fd) SHA1(bf08b3f111f780dc8c81275e4e6247388183a8da) )
	ROM_CONTINUE(             0x1c00, 0x0200 )
	ROM_CONTINUE(             0x1a00, 0x0200 )
	ROM_CONTINUE(             0x1e00, 0x0200 )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "l06_prom.bin", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( eagle3 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "e1",           0x0000, 0x0800, CRC(224c9526) SHA1(4c014d60d4ee80de7f60b4609269461688c181d0) )
	ROM_LOAD( "e2",           0x0800, 0x0800, CRC(cc538ebd) SHA1(4ef3c7363e2dcd9ed99779039ccc50a9f2084dbd) )
	ROM_LOAD( "f03.bin",      0x1000, 0x0800, CRC(29a2b0ab) SHA1(e9fc7161d0566e36307c45b7132e2262c0af4845) )
	ROM_LOAD( "f04.bin",      0x1800, 0x0800, CRC(4c6a5a6d) SHA1(366516f63c9b5239e703e4dfb672659049ddbf44) )
	ROM_LOAD( "e5",           0x2000, 0x0800, CRC(06d378a6) SHA1(99dbe9fc7f95f8fdce86eb5c32bd1ca1bea0ca3c) )
	ROM_LOAD( "e6",           0x2800, 0x0800, CRC(0dea20d5) SHA1(405b51d4e3b1065f78afd2297e075e977ae19196) )
	ROM_LOAD( "e7",           0x3000, 0x0800, CRC(b45af1e8) SHA1(d7020774707234acdaef5c655f667d5ee9e54a13) )
	ROM_LOAD( "e8",           0x3800, 0x0800, CRC(c437a876) SHA1(845941b873970ac62ba9bb6353bee53d0fcfa292) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "e10a",         0x0000, 0x0800, CRC(e3c63d4c) SHA1(ad2b22e316da6bb819c58934d51cd4b2819b18f0) )
	ROM_LOAD( "e12",          0x0800, 0x0200, CRC(628fdeed) SHA1(a798530c65e639fbf00ed3a4e8c428935bf5f38e) )
	ROM_CONTINUE(             0x0c00, 0x0200 )	/* this version of the gfx ROMs has two */
	ROM_CONTINUE(             0x0a00, 0x0200 )	/* groups of 16 sprites swapped */
	ROM_CONTINUE(             0x0e00, 0x0200 )
	ROM_LOAD( "e9a",          0x1000, 0x0800, CRC(59429e47) SHA1(b7629c81d122fd1e4d390aa7abba44df898387d3) )
	ROM_LOAD( "e11",          0x1800, 0x0200, CRC(ee4ec5fd) SHA1(bf08b3f111f780dc8c81275e4e6247388183a8da) )
	ROM_CONTINUE(             0x1c00, 0x0200 )
	ROM_CONTINUE(             0x1a00, 0x0200 )
	ROM_CONTINUE(             0x1e00, 0x0200 )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "l06_prom.bin", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( spctbird )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "tssa-7f",      0x0000, 0x0800, CRC(45aab7a3) SHA1(52ae0463f363dc0964b976faa2c0c428d85a4f12) )
	ROM_LOAD( "tssa-7h",      0x0800, 0x0800, CRC(8b328f48) SHA1(d4f549e90e0bf1f546e2c3dc5a5a16e0415e709e) )
	ROM_LOAD( "tssa-7k",      0x1000, 0x0800, CRC(29a2b0ab) SHA1(e9fc7161d0566e36307c45b7132e2262c0af4845) )
	ROM_LOAD( "tssa-7m",      0x1800, 0x0800, CRC(99c9166d) SHA1(c108d84330bc958ff2812dc807e68c246a5a5ad5) )
	ROM_LOAD( "tssa-5",       0x2000, 0x0800, CRC(797b6261) SHA1(9a60e504e2aa0201b7311485c0dd411bbe2dc70b) )
	ROM_LOAD( "tssa-6",       0x2800, 0x0800, CRC(4825692c) SHA1(41a7e305c3d93f2245fb0413398d951eab9d16c0) )
	ROM_LOAD( "tssa-7",       0x3000, 0x0800, CRC(b45af1e8) SHA1(d7020774707234acdaef5c655f667d5ee9e54a13) )
	ROM_LOAD( "tssa-8",       0x3800, 0x0800, CRC(c9b77b85) SHA1(00797f126b4cdacd9ec2df7e747aa1892933b8b8) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
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

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "l06_prom.bin", 0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( skybase )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "skybase.9a",   0x0000, 0x1000, CRC(845b87a5) SHA1(8a249c1ec921532cb1bb85ed7fec11396634ca38) )
	ROM_LOAD( "skybase.8a",   0x1000, 0x1000, CRC(096785c2) SHA1(a0833bc1984e1f198587195e58b6fed6657922bd) )
	ROM_LOAD( "skybase.7a",   0x2000, 0x1000, CRC(d50c715b) SHA1(3d0fa15514b210bccd4aeed06540122a4f56fd7a) )
	ROM_LOAD( "skybase.6a",   0x3000, 0x1000, CRC(f57edb27) SHA1(4b5c376017700315345241fad96c00478a14fc8f) )
	ROM_LOAD( "skybase.5a",   0x4000, 0x1000, CRC(50365d95) SHA1(9b3d360c9d1df0ebf047bef1b30765ea9bb42b42) )
	ROM_LOAD( "skybase.4a",   0x5000, 0x1000, CRC(cbd6647f) SHA1(7a167c9df6b5f3346c37e5c45d0680b0b29852a6) )

	ROM_REGION( 0x4000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "skybase.7t",   0x0000, 0x1000, CRC(9b471686) SHA1(b35831daa8ce57e498c2c4f75763a74c340cfaf0) )
	ROM_LOAD( "skybase.8t",   0x1000, 0x1000, CRC(1cf723da) SHA1(f2e41ab89413298571626d13b2b5853eb35dcb96) )
	ROM_LOAD( "skybase.10t",  0x2000, 0x1000, CRC(fe02e72c) SHA1(bf7c078e984b13dcc12d529904f1096d65e41bec) )
	ROM_LOAD( "skybase.9t",   0x3000, 0x1000, CRC(0871291f) SHA1(2e4e802316b55711bcfeb48d84bacd11afff8cb3) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.bpr",  0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Color prom */
ROM_END

ROM_START( moonqsr )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "mq1",          0x0000, 0x0800, CRC(132c13ec) SHA1(d95166b025442f184e44a70312fb3b4f6366f324) )
	ROM_LOAD( "mq2",          0x0800, 0x0800, CRC(c8eb74f1) SHA1(4efa85c40349852da47a0f725ae06873efe4ce1c) )
	ROM_LOAD( "mq3",          0x1000, 0x0800, CRC(33965a89) SHA1(92912cea76a472d9b709c664d9818844a07fcc32) )
	ROM_LOAD( "mq4",          0x1800, 0x0800, CRC(a3861d17) SHA1(d7037d93b7838ccdd9a6a1a1476571cfa869fca1) )
	ROM_LOAD( "mq5",          0x2000, 0x0800, CRC(8bcf9c67) SHA1(7af0d9308d20c52675301acf5d1a5d62358352a6) )
	ROM_LOAD( "mq6",          0x2800, 0x0800, CRC(5750cda9) SHA1(17c2bc38037833fdb8923d4a2262264386ef916b) )
	ROM_LOAD( "mq7",          0x3000, 0x0800, CRC(78d7fe5b) SHA1(4085562a0af94c65dad2a3550409727e597c0d5b) )
	ROM_LOAD( "mq8",          0x3800, 0x0800, CRC(4919eed5) SHA1(526aaedd25e0f7c525eb7c66519218ae09b0407e) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )

	ROM_LOAD( "mqb",          0x0000, 0x0800, CRC(b55ec806) SHA1(fb52e53dfa3ae9dec162622d22de9cfdb0b5f9d6) )
	ROM_LOAD( "mqd",          0x0800, 0x0800, CRC(9e7d0e13) SHA1(18951080d307ac13344f89745f671595e26d282c) )
	ROM_LOAD( "mqa",          0x1000, 0x0800, CRC(66eee0db) SHA1(eeb08efd226e15e248999558240488ffd0e39688) )
	ROM_LOAD( "mqc",          0x1800, 0x0800, CRC(a6db5b0d) SHA1(476e197df047e991d2ea3c1fad92c799510f1647) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "vid_e6.bin",   0x0000, 0x0020, CRC(0b878b54) SHA1(3667aca564ebfef5b88d7f74fabbd16dd23183b4) )
ROM_END

ROM_START( moonal2 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
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

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "ali13.1h",     0x0000, 0x0800, CRC(a1287bf6) SHA1(eeeaba4b9e186454a5e2f1c26e333e8fccd97af8) )
	ROM_RELOAD(	              0x0800, 0x0800 )
	ROM_LOAD( "ali12.1k",     0x1000, 0x0800, CRC(528f1481) SHA1(e266a75c3109bcfa2a0394f2ed0ac136fc3158ba) )
	ROM_RELOAD(	              0x1800, 0x0800 )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( moonal2b )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
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

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "ali13.1h",     0x0000, 0x0800, CRC(a1287bf6) SHA1(eeeaba4b9e186454a5e2f1c26e333e8fccd97af8) )
	ROM_RELOAD(	              0x0800, 0x0800 )
	ROM_LOAD( "ali12.1k",     0x1000, 0x0800, CRC(528f1481) SHA1(e266a75c3109bcfa2a0394f2ed0ac136fc3158ba) )
	ROM_RELOAD(	              0x1800, 0x0800 )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(c3ac9467) SHA1(f382ad5a34d282056c78a5ec00c30ec43772bae2) )
ROM_END

ROM_START( supergx )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "sg1",          0x0000, 0x0800, CRC(b83f4578) SHA1(9a5d5fc291839f7f1e0a52cca7bea29e99c13315) )
	ROM_LOAD( "sg2",          0x0800, 0x0800, CRC(d12ca054) SHA1(8eb7f6904c3c650bfa80908a5988622d5e693bd1) )
	ROM_LOAD( "sg3",          0x1000, 0x0800, CRC(53714cb1) SHA1(7dffcd3ced1c3354339bb69477f8aa4c708708db) )
	ROM_LOAD( "sg4",          0x1800, 0x0800, CRC(2f36fc69) SHA1(d310dcb0a79b03ee26b0575db9cba6d920cb9273) )
	ROM_LOAD( "sg5",          0x2000, 0x0800, CRC(1e0ed4fd) SHA1(183d8990dbff1954921f8c5b67cec09f2d380794) )
	ROM_LOAD( "sg6",          0x2800, 0x0800, BAD_DUMP CRC(4f3d97a8) SHA1(b9fcab182ab57e8374fef93f7fd314a155a8d04d) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "sgg1",         0x0000, 0x0800, CRC(a1287bf6) SHA1(eeeaba4b9e186454a5e2f1c26e333e8fccd97af8) )
	ROM_RELOAD(               0x0800, 0x0800 )
	ROM_LOAD( "sgg2",         0x1000, 0x0800, CRC(528f1481) SHA1(e266a75c3109bcfa2a0394f2ed0ac136fc3158ba) )
	ROM_RELOAD(               0x1800, 0x0800 )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "supergx.prm",  0x0000, 0x0020, NO_DUMP )
ROM_END

ROM_START( mshuttle )
	ROM_REGION( 2*0x10000, REGION_CPU1, 0 )	/* 64k for code + 64k for decrypted opcodes */
	ROM_LOAD( "my05",         0x0000, 0x1000, CRC(83574af1) SHA1(d69c2a0538a49d6c72c3346ac4e3959d91da6c98) )
	ROM_LOAD( "my04",         0x1000, 0x1000, CRC(1cfae2c8) SHA1(6c7eeee70e91b8498c41525dcc60f8086cff8da7) )
	ROM_LOAD( "my03",         0x2000, 0x1000, CRC(c8b8a368) SHA1(140ba60f55285d1e9f7a262634f5ce5c3470ab71) )
	ROM_LOAD( "my02",         0x3000, 0x1000, CRC(b6aeee6e) SHA1(032af7000aebe9d34319231cdb3f2fe5de7158ba) )
	ROM_LOAD( "my01",         0x4000, 0x1000, CRC(def82adc) SHA1(2fb963299468c52d50b7460b55bf69c9659ee21d) )

	ROM_REGION( 0x4000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "my09",         0x0000, 0x1000, CRC(3601b380) SHA1(c0b9d1801f58a16449708d514d2fd88e34af340b) )
	ROM_LOAD( "my11",         0x1000, 0x0800, CRC(b659e932) SHA1(3f63c99e81cb93c9553a5e274546525f598d50c4) )
	ROM_LOAD( "my08",         0x2000, 0x1000, CRC(992b06cd) SHA1(8645ccad8169601bbe25b9f2b17b99004c0a584f) )
	ROM_LOAD( "my10",         0x3000, 0x0800, CRC(d860e6ce) SHA1(2912d13bf69496f8f18358a36366a1f60afd0070) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "mscprom1.bin", 0x0000, 0x0020, CRC(ea0d1af0) SHA1(cb59e04c02307dfe847e3170cf0a7f62829b6094) )

	ROM_REGION( 0x2000, REGION_SOUND1, 0 )	/* samples */
	ROM_LOAD( "my07",         0x0000, 0x1000, CRC(522a2920) SHA1(a64d821a8ff6bd6e2b0bdb1e632181e65a97363b) )
	ROM_LOAD( "my06",         0x1000, 0x1000, CRC(466415f2) SHA1(a05f8238cdcebe926a564ef6268b3cd677987fa2) )
ROM_END

ROM_START( mshuttlj )
	ROM_REGION( 2*0x10000, REGION_CPU1, 0 )	/* 64k for code + 64k for decrypted opcodes */
	ROM_LOAD( "mcs.5",        0x0000, 0x1000, CRC(a5a292b4) SHA1(b4e9d969c762f4114eba88051917df122fc7181f) )
	ROM_LOAD( "mcs.4",        0x1000, 0x1000, CRC(acdc0f9e) SHA1(8cd6d6566fe3f4090ccb625c3c1e5850a371826f) )
	ROM_LOAD( "mcs.3",        0x2000, 0x1000, CRC(c1e3f5d8) SHA1(d3af89d485b1ca21ac879dbe15490dcd1cd64f2a) )
	ROM_LOAD( "mcs.2",        0x3000, 0x1000, CRC(14577703) SHA1(51537982dd06ba44e95e4c7d1f7fa41ff186421d) )

	ROM_LOAD( "mcs.1",        0x4000, 0x1000, CRC(27d46772) SHA1(848a47ba30823a55933bb55792991f0535078f0c) )

	ROM_REGION( 0x4000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "my09",         0x0000, 0x1000, CRC(3601b380) SHA1(c0b9d1801f58a16449708d514d2fd88e34af340b) )
	ROM_LOAD( "my11",         0x1000, 0x0800, CRC(b659e932) SHA1(3f63c99e81cb93c9553a5e274546525f598d50c4) )
	ROM_LOAD( "my08",         0x2000, 0x1000, CRC(992b06cd) SHA1(8645ccad8169601bbe25b9f2b17b99004c0a584f) )
	ROM_LOAD( "my10",         0x3000, 0x0800, CRC(d860e6ce) SHA1(2912d13bf69496f8f18358a36366a1f60afd0070) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "mscprom1.bin", 0x0000, 0x0020, CRC(ea0d1af0) SHA1(cb59e04c02307dfe847e3170cf0a7f62829b6094) )

	ROM_REGION( 0x2000, REGION_SOUND1, 0 )	/* samples */
	ROM_LOAD( "my07",         0x0000, 0x1000, CRC(522a2920) SHA1(a64d821a8ff6bd6e2b0bdb1e632181e65a97363b) )
	ROM_LOAD( "my06",         0x1000, 0x1000, CRC(466415f2) SHA1(a05f8238cdcebe926a564ef6268b3cd677987fa2) )
ROM_END

ROM_START( mshutlj2 )
	ROM_REGION( 2*0x10000, REGION_CPU1, 0 )	/* 64k for code + 64k for decrypted opcodes */
	ROM_LOAD( "ali5.bin",     0x0000, 0x1000, CRC(320fe630) SHA1(df4fe25989783c8851f41c9b4b63dedfa365c1e9) )
	ROM_LOAD( "mcs.4",        0x1000, 0x1000, CRC(acdc0f9e) SHA1(8cd6d6566fe3f4090ccb625c3c1e5850a371826f) )
	ROM_LOAD( "mcs.3",        0x2000, 0x1000, CRC(c1e3f5d8) SHA1(d3af89d485b1ca21ac879dbe15490dcd1cd64f2a) )
	ROM_LOAD( "ali2.bin",     0x3000, 0x1000, CRC(9ed169e1) SHA1(75a24d0fcbdfc7c4e6fa0d8c7f8b4a3bccaa4439) )

	ROM_LOAD( "ali1.bin",     0x4000, 0x1000, CRC(7f8a52d9) SHA1(4e62f6265289bae1a46e60cdd3230e188b2aec3c) )

	ROM_REGION( 0x4000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "my09",         0x0000, 0x1000, CRC(3601b380) SHA1(c0b9d1801f58a16449708d514d2fd88e34af340b) )
	ROM_LOAD( "my11",         0x1000, 0x0800, CRC(b659e932) SHA1(3f63c99e81cb93c9553a5e274546525f598d50c4) )
	ROM_LOAD( "my08",         0x2000, 0x1000, CRC(992b06cd) SHA1(8645ccad8169601bbe25b9f2b17b99004c0a584f) )
	ROM_LOAD( "my10",         0x3000, 0x0800, CRC(d860e6ce) SHA1(2912d13bf69496f8f18358a36366a1f60afd0070) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "mscprom1.bin", 0x0000, 0x0020, CRC(ea0d1af0) SHA1(cb59e04c02307dfe847e3170cf0a7f62829b6094) )

	ROM_REGION( 0x2000, REGION_SOUND1, 0 )	/* samples */
	ROM_LOAD( "my07",         0x0000, 0x1000, CRC(522a2920) SHA1(a64d821a8ff6bd6e2b0bdb1e632181e65a97363b) )
	ROM_LOAD( "my06",         0x1000, 0x1000, CRC(466415f2) SHA1(a05f8238cdcebe926a564ef6268b3cd677987fa2) )
ROM_END

ROM_START( kingball )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "prg1.7f",      0x0000, 0x1000, CRC(6cb49046) SHA1(a0891605dff7f9ff51bc7ad85f831a749f2f61e9) )
	ROM_LOAD( "prg2.7j",      0x1000, 0x1000, CRC(c223b416) SHA1(ca2d9f6b8ef6db4f382089161f4147d9828c3554) )
	ROM_LOAD( "prg3.7l",      0x2000, 0x0800, CRC(453634c0) SHA1(0025ccd91e165692092a37541e730010e85e37f2) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* 64k for sound code */
	ROM_LOAD( "kbe1.ic4",     0x0000, 0x0800, CRC(5be2c80a) SHA1(f719a80357bed3d66bce40569690f419740148c5) )
	ROM_LOAD( "kbe2.ic5",     0x0800, 0x0800, CRC(bb59e965) SHA1(830e0c415f051e932d76df604025e4e33118a799) )
	ROM_LOAD( "kbe3.ic6",     0x1000, 0x0800, CRC(1c94dd31) SHA1(14ab59b8eee741eb1f10ae99ddb99bf7c2dab957) )
	ROM_LOAD( "kbe2.ic7",     0x1800, 0x0800, CRC(bb59e965) SHA1(830e0c415f051e932d76df604025e4e33118a799) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "chg1.1h",      0x0000, 0x0800, CRC(9cd550e7) SHA1(d2989e6b7a4d7b37a711ef1cfb536fe13e0c5482) )
	ROM_RELOAD(	              0x0800, 0x0800 )
	ROM_LOAD( "chg2.1k",      0x1000, 0x0800, CRC(a206757d) SHA1(46b50005876b7f61ab4a118d0a4caaebce8ce3e1) )
	ROM_RELOAD(	              0x1800, 0x0800 )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "kb2-1",        0x0000, 0x0020, CRC(15dd5b16) SHA1(3d2ca2b42bf508a9e5198e970abcbbedf5729164) )
ROM_END

ROM_START( kingbalj )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "prg1.7f",      0x0000, 0x1000, CRC(6cb49046) SHA1(a0891605dff7f9ff51bc7ad85f831a749f2f61e9) )
	ROM_LOAD( "prg2.7j",      0x1000, 0x1000, CRC(c223b416) SHA1(ca2d9f6b8ef6db4f382089161f4147d9828c3554) )
	ROM_LOAD( "prg3.7l",      0x2000, 0x0800, CRC(453634c0) SHA1(0025ccd91e165692092a37541e730010e85e37f2) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* 64k for sound code */
	ROM_LOAD( "kbj1.ic4",     0x0000, 0x0800, CRC(ba16beb7) SHA1(8c2c91a9e941d858a49edd6c0c8a912e1135653e) )
	ROM_LOAD( "kbj2.ic5",     0x0800, 0x0800, CRC(56686a63) SHA1(8e624df57a63a556941fdbebcd886488799fad17) )
	ROM_LOAD( "kbj3.ic6",     0x1000, 0x0800, CRC(fbc570a5) SHA1(d0dbaf86396bca65e067338a3b5b60b24990b8be) )
	ROM_LOAD( "kbj2.ic7",     0x1800, 0x0800, CRC(56686a63) SHA1(8e624df57a63a556941fdbebcd886488799fad17) )


	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "chg1.1h",      0x0000, 0x0800, CRC(9cd550e7) SHA1(d2989e6b7a4d7b37a711ef1cfb536fe13e0c5482) )
	ROM_RELOAD(	              0x0800, 0x0800 )
	ROM_LOAD( "chg2.1k",      0x1000, 0x0800, CRC(a206757d) SHA1(46b50005876b7f61ab4a118d0a4caaebce8ce3e1) )
	ROM_RELOAD(	              0x1800, 0x0800 )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "kb2-1",        0x0000, 0x0020, CRC(15dd5b16) SHA1(3d2ca2b42bf508a9e5198e970abcbbedf5729164) )
ROM_END

ROM_START( scorpnmc )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
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

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "h.bin",        0x0000, 0x1000, CRC(1e5da9d6) SHA1(ca8b27e6dd40e4ca13e7e6b5f813bafca78b62f4) )
	ROM_LOAD( "k.bin",        0x1000, 0x1000, CRC(a57adb0a) SHA1(d97c7dc4a6c5efb59cc0148e2498156c682c6714) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "mmi6331.bpr",  0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END

ROM_START( frogg )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "p1.bin",       0x0000, 0x0800, CRC(1762b266) SHA1(2cf34dcfe00dc476b327f9d762a8d2aa268a2d25) )
	ROM_LOAD( "p2.bin",       0x0800, 0x0800, CRC(322f3916) SHA1(9236aaa260c4db4adbd92c8bba3674d07d7235a8) )
	ROM_LOAD( "p3.bin",       0x1000, 0x0800, CRC(28bd6151) SHA1(1a5bc540168fa5fef01bd7bc2cdbdb910c9a4ba4) )
	ROM_LOAD( "p4.bin",       0x1800, 0x0800, CRC(5a69ab18) SHA1(40b7bf200f87e0fb3fb54726ba79387889446052) )
	ROM_LOAD( "p5.bin",       0x2000, 0x0800, CRC(b4f17745) SHA1(2f237a667f6c95af213b787620142c1530d3cdd8) )
	ROM_LOAD( "p6.bin",       0x2800, 0x0800, CRC(34be71b5) SHA1(3088fc5817a397d0a87610d62845c7b8c4440f57) )
	ROM_LOAD( "p7.bin",       0x3000, 0x0800, CRC(de3edc8c) SHA1(634d54fb19b422b56576a196bdaf95733c52c7ee) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "k.bin",        0x0000, 0x0800, CRC(05f7d883) SHA1(78831fd287da18928651a8adb7e578d291493eff) )
	ROM_LOAD( "h.bin",        0x0800, 0x0800, CRC(658745f8) SHA1(e4e5c3e011c8a7233a36d29e10e08905873500aa) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "pr-91.6l",     0x0000, 0x0020, CRC(413703bf) SHA1(66648b2b28d3dcbda5bdb2605d1977428939dd3c) )
ROM_END

ROM_START( 4in1 )
	ROM_REGION( 0x20000, REGION_CPU1, 0 )   /* 64k for code  64k for banked code, encrypted */
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

	ROM_REGION( 0x8000, REGION_GFX1, ROMREGION_DISPOSE )
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

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
    ROM_LOAD( "6l.bpr",       0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( bagmanmc )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "b1.bin",       0x0000, 0x1000, CRC(b74c75ee) SHA1(620083c30136e24a37b79eb4647d99b997107693) )
	ROM_LOAD( "b2.bin",       0x1000, 0x1000, CRC(a7d99916) SHA1(13185e8ff6de92ad5135895e5a7fc8b956f009d3) )
	ROM_LOAD( "b3.bin",       0x2000, 0x1000, CRC(c78f5360) SHA1(7ce9e94c33f1b8e60cc12a3df5f9555f1ca6130f) )
	ROM_LOAD( "b4.bin",       0x3000, 0x1000, CRC(eebd3bd1) SHA1(03200383e87b0759f607888d9b290a0a777b597e) )
	ROM_LOAD( "b5.bin",       0x4000, 0x1000, CRC(0fe24b8c) SHA1(205a36fd346d49d2dda6911198295e202caae81f) )
	ROM_LOAD( "b6.bin",       0x5000, 0x1000, CRC(f50390e7) SHA1(b4ebe647458c26e52461750d63856aea4262f110) )

	ROM_REGION( 0x3000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "g1-u.bin",     0x0000, 0x0800, CRC(b63cfae4) SHA1(3e0cb3dbeec8ad790bc482176ca599721bac31ee) )
	ROM_CONTINUE(             0x2000, 0x0800 )
	ROM_LOAD( "g2-u.bin",     0x1000, 0x0800, CRC(a2790089) SHA1(7eb8634f26f6af52fb79bf90ec90b4e258c7c79f) )
	ROM_CONTINUE(             0x2800, 0x0800 )
	ROM_LOAD( "g1-l.bin",     0x0800, 0x0800, CRC(2ae6b5ab) SHA1(59bdebf75d28a247293440ec2ad83eaf30e3de00) )
	ROM_LOAD( "g2-l.bin",     0x1800, 0x0800, CRC(98b37397) SHA1(29914435a10cebbbce04382c45e13a64a0cd18cb) )


	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "bagmanmc.clr", 0x0000, 0x0020, NO_DUMP )	// missing
ROM_END

ROM_START( dkongjrm )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "a1",           0x0000, 0x1000, CRC(299486e9) SHA1(cc4143ff8cb7a37c151bebab007a932381ae733b) )
	ROM_LOAD( "a2",           0x1000, 0x1000, CRC(a74a193b) SHA1(46f208293c0944b468550738d1238de9b672f403) )
	ROM_LOAD( "b2",           0x2000, 0x1000, CRC(7bc4f236) SHA1(84e7f5fcbea7d047f2a9a9006ae3ed646417c5e0) )
	ROM_LOAD( "c1",           0x3000, 0x1000, CRC(0f594c21) SHA1(eb15bd9cc37794786e2ad24753172e88aa7c4f98) )
	ROM_LOAD( "d1",           0x4000, 0x1000, CRC(cf7d7296) SHA1(9a817eca2ebef3f5208bb29ee7eece2ec0efe158) )
	ROM_LOAD( "e2",           0x5000, 0x1000, CRC(f7528a52) SHA1(e9d3c57934ee97fcc1f17ecdf3bc954574212220) )
	ROM_LOAD( "f1",           0x7000, 0x1000, CRC(9b1d4cc5) SHA1(9a412fec82f39b9389ff99cceba2e49b2a74df17) )

	ROM_REGION( 0x4000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "v_3pa.bin",    0x0000, 0x1000, CRC(4974ffef) SHA1(7bb1e207dd3c5214e405bf32c57ec1b048061050) )
	ROM_LOAD( "a2.gfx",       0x1000, 0x1000, CRC(51845eaf) SHA1(43970d69329f3d49ea1ff57d54abe8340ceef275) )
	ROM_LOAD( "v_3na.bin",    0x2000, 0x1000, CRC(a95c4c63) SHA1(75e312b6872958f3bfc7bafd0743efdf7a74e8f0) )
	ROM_LOAD( "b2.gfx",       0x3000, 0x1000, CRC(7b39c3d0) SHA1(4b8cebb4cdaaca9e1b6fd378f6c390ab05984590) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "hustler.clr",  0x0000, 0x0020, CRC(aa1f7f5e) SHA1(311dd17aa11490a1173c76223e4ccccf8ea29850) )
ROM_END

ROM_START( rockclim )

	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "lc01.a1",   0x0000, 0x1000, CRC(8601ae8d) SHA1(6e0c3b34ce5e6879ce7a116c5c2660889a68320d) )
	ROM_LOAD( "lc02.a2",   0x1000, 0x1000, CRC(2dde9d4c) SHA1(7e343113116b94894558819a7f77f77e4e952da7) )
	ROM_LOAD( "lc03.a3",   0x2000, 0x1000, CRC(82c48a67) SHA1(abf95062eb5c9bd4bb3c9b9af59396a4ca6905d8) )
	ROM_LOAD( "lc04.a4",   0x3000, 0x1000, CRC(7cd3a04a) SHA1(756c12288e120e6f761b266b91920d17cab6926c) )
	ROM_LOAD( "lc05.a5",   0x6000, 0x1000, CRC(5e542149) SHA1(425a5a8769c3fa0887db8ff04e2a4f32f18d2679) )
	ROM_LOAD( "lc06.a6",   0x7000, 0x1000, CRC(b2bdca64) SHA1(e72e63725164c922816dda90ac964a94062eab1b) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "lc08.a9",   0x0000, 0x800, CRC(7f18e1ef) SHA1(2a160b994708ec0f06774dde3ec613af7e3f32c6) )
	ROM_LOAD( "lc07.a7",   0x0800, 0x800, CRC(f18b50ac) SHA1(a2328eb55882a09403cae1a497c611b494649cac) )
	ROM_LOAD( "lc10.c9",   0x1000, 0x800, CRC(dec5781b) SHA1(b6277fc890d153db24bd48293780cf239a6aa0e7) )
	ROM_LOAD( "lc09.c7",   0x1800, 0x800, CRC(06c0b5de) SHA1(561cf99a6be03205c7bc5fd15d4d51ee4d6d164b) )

	ROM_REGION( 0x2000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "lc13.g5",   0x0000, 0x1000, CRC(19475f2b) SHA1(5d42aa45a7b519dacdecd3d2edbfee6971693034) )
	ROM_LOAD( "lc14.g7",   0x1000, 0x1000, CRC(cc96d1db) SHA1(9713b47b723a5d8837f2a8e8c43e46dc41a62e5b) )

	ROM_REGION( 0x0040, REGION_PROMS, 0 )
	ROM_LOAD( "lc12.e9",  0x0000, 0x0020, CRC(f6e76547) SHA1(c9ea78d1876156561b3bbf327d7e0299e1d9fd4a) )
	ROM_LOAD( "lc11.f4",  0x0020, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( ozon1 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "rom1.bin",     0x0000, 0x1000, CRC(54899e8b) SHA1(270af76ae4396ebda767f160535fa77c0b49726a) )
	ROM_LOAD( "rom2.bin",     0x1000, 0x1000, CRC(3c90fbfc) SHA1(92da614dba3a644eac144bb0ed434d78a31fcb1a) )
	ROM_LOAD( "rom3.bin",     0x2000, 0x1000, CRC(79fe313b) SHA1(ef8fd70f5669b7e7d7184eca2baaddcecb55c22d) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "rom7.bin",     0x0000, 0x0800, CRC(464285e8) SHA1(fff36b034b95050219c70cdfe05ff3bbc452b73e) )
	ROM_LOAD( "rom8.bin",     0x0800, 0x0800, CRC(92056dcc) SHA1(b162da8701bfee465205e8f274ee494063c52c7b) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "ozon1.clr", 0x0000, 0x0020, CRC(605ea6e9) SHA1(d3471e6ef756059c2f7feb32fb8e41181cc1718e) )
ROM_END

ROM_START( ladybugg )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "lbuggx.1",   0x0000, 0x0800, CRC(e67e241d) SHA1(42b8eaca71c6b346ab54bc722850d6e6d169c517) )
	ROM_LOAD( "lbuggx.2",   0x0800, 0x0800, CRC(3cb1fb9a) SHA1(ee76758c94329dfcc740571195a74d9242aaf49f) )
	ROM_LOAD( "lbuggx.3",   0x1000, 0x0800, CRC(0937009e) SHA1(ef57ebf3d6ab3d6ac0e1faa10c3109d2c80a1248) )
	ROM_LOAD( "lbuggx.4",   0x1800, 0x0800, CRC(3e773f62) SHA1(6348e61f48e5d1f04289098c4c0395335ea5e2a5) )
	ROM_LOAD( "lbuggx.5",   0x2000, 0x0800, CRC(2b0d42e5) SHA1(1547b8127f964eb10862b566f5779f8011c3441d) )
	ROM_LOAD( "lbuggx.6",   0x2800, 0x0800, CRC(159f9433) SHA1(93341a4de1e1e4a3fb004019fc1edba73db6a4c8) )
	ROM_LOAD( "lbuggx.7",   0x3000, 0x0800, CRC(f2be06d5) SHA1(1354332d2d107ad810aa2e261b595285394dfb49) )
	ROM_LOAD( "lbuggx.8",   0x3800, 0x0800, CRC(646fe79f) SHA1(03223d6c4f9050fd6c1c313f0e366ab4989feca4) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "lbuggx.a",   0x0800, 0x0800, CRC(7efb9dc5) SHA1(5e02ea8cd1a1c8efa6708a8615cc2dc9da65a455) )
	ROM_CONTINUE ( 0x0000, 0x0800)
	ROM_LOAD( "lbuggx.b",   0x1800, 0x0800, CRC(351d4ddc) SHA1(048e8a60e57c6eb0a4d7c2175ddd46c4273756c5) )
	ROM_CONTINUE ( 0x1000, 0x0800)

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "lbuggx.clr", 0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )
ROM_END

ROM_START( vpool )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "vidpool1.bin", 0x0000, 0x0800, CRC(333f4732) SHA1(b57460c039c69137645bd4280ad877aa789277d6) )
	ROM_CONTINUE(             0x2000, 0x0800 )
	ROM_LOAD( "vidpool2.bin", 0x0800, 0x0800, CRC(eea6c0f1) SHA1(5b18caa78e246f55fd9cd778d6e83f79f0b3f157) )
	ROM_CONTINUE(             0x2800, 0x0800 )
	ROM_LOAD( "vidpool3.bin", 0x1000, 0x0800, CRC(309972a6) SHA1(8269d2f677f55dda71d6a7b0796d2d53a4def59d) )
	ROM_CONTINUE(             0x3000, 0x0800 )
	ROM_LOAD( "vidpool4.bin", 0x1800, 0x0800, CRC(c4f71c1d) SHA1(e1d01135d5ccc1a53308ce89dc2a8fc0992207d5) )
	ROM_CONTINUE(             0x3800, 0x0800 )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "hustler.5f", 0x0000, 0x0800, CRC(0bdfad0e) SHA1(8e6f1737604f3801c03fa2e9a5e6a2778b54bae8) ) // vidpoolh.bin
	ROM_LOAD( "hustler.5h", 0x0800, 0x0800, CRC(8e062177) SHA1(7e52a1669804b6c2f694cfc64b04abc8246bb0c2) ) // vidpoolk.bin

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "hustler.clr",  0x0000, 0x0020, CRC(aa1f7f5e) SHA1(311dd17aa11490a1173c76223e4ccccf8ea29850) )
ROM_END

ROM_START( drivfrcg )
	ROM_REGION( 0x8000, REGION_CPU1, 0 )
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

	ROM_REGION( 0x4000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "dfgj2.bin",    0x0000, 0x1000, CRC(8e19f1e7) SHA1(addd5add2117ef29ce38c0c80584e5d481b9d820) )
	ROM_LOAD( "dfgj1.bin",    0x1000, 0x1000, CRC(86b60ca8) SHA1(be266e2d69e12a196c2195d48b495c0fb9ef8a43) )
	ROM_LOAD( "dfgl2.bin",    0x2000, 0x1000, CRC(ea5e9959) SHA1(6b638d22adf19224cf741458c8ad34d7f7e17e58) )
	ROM_LOAD( "dfgl1.bin",    0x3000, 0x1000, CRC(b7ed195c) SHA1(81b2b444153dacb962a33a5d86a280ed5088637a) )

	/* piggy-backed colour proms */
	ROM_REGION( 0x0040, REGION_PROMS, 0 )
	ROM_LOAD( "top.clr",      0x0000, 0x0020, CRC(3110ddae) SHA1(53b2e1cc07915592f6c868131ec296c63a407f04) )
	ROM_LOAD( "bot.clr",      0x0020, 0x0020, CRC(0f0782af) SHA1(32c0dd09ead5c70cee2657e9cb8cb9fcf54c5a6a) )
ROM_END

ROM_START( drivfrcb )
	ROM_REGION( 0x8000, REGION_CPU1, 0 )
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

	ROM_REGION( 0x4000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "df1.bin",      0x1000, 0x1000, CRC(8adc3de0) SHA1(046fb92913171c621bb62edb0174f04298bfd283) )
	ROM_CONTINUE(			  0x0000, 0x1000 )
	ROM_LOAD( "df2.bin",      0x3000, 0x1000, CRC(6d95ec35) SHA1(c745ee2bc7b1fb53e8bc1ac3a4238bbe00f30cfe) )
	ROM_CONTINUE(			  0x2000, 0x1000 )

	/* piggy-backed colour proms */
	ROM_REGION( 0x0040, REGION_PROMS, 0 )
	ROM_LOAD( "top.clr",      0x0000, 0x0020, CRC(3110ddae) SHA1(53b2e1cc07915592f6c868131ec296c63a407f04) )
	ROM_LOAD( "bot.clr",      0x0020, 0x0020, CRC(0f0782af) SHA1(32c0dd09ead5c70cee2657e9cb8cb9fcf54c5a6a) )
ROM_END

ROM_START( bongo )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "bg1.bin",    0x0000, 0x1000, CRC(de9a8ec6) SHA1(b5ee99b26d1a39e31b643ad0f5723ee8e364023e) )
	ROM_LOAD( "bg2.bin",    0x1000, 0x1000, CRC(a19da662) SHA1(a2674392d489c5e5eeb9abc51572a37cc6045220) )
	ROM_LOAD( "bg3.bin",    0x2000, 0x1000, CRC(9f6f2150) SHA1(26a1f872686ddddcdb690d7b826ba26c20cdec35) )
	ROM_LOAD( "bg4.bin",    0x3000, 0x1000, CRC(f80372d2) SHA1(078e2c8b947103c168c0c85430f8ebc9d09f8ba7) )
	ROM_LOAD( "bg5.bin",    0x4000, 0x1000, CRC(fc92eade) SHA1(f4012a1c4631388a3e8109a8381bc4084ddc8757) )
	ROM_LOAD( "bg6.bin",    0x5000, 0x1000, CRC(561d9e5d) SHA1(68d7fab3cfb5b3360fe8064c70bf21bb1341032f) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "b-h.bin",    0x0000, 0x1000, CRC(fc79d103) SHA1(dac1152221ebdc4cd9bf353b4cc5d45021ca5d9e) )
	ROM_LOAD( "b-k.bin",    0x1000, 0x1000, CRC(94d17bf3) SHA1(2a70968249946de52c5a4cfabafbbf4ecda844a8) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "b-clr.bin",  0x0000, 0x0020, CRC(c4761ada) SHA1(067d12b2d3635ffa6337ed234ba42717447bea00) )
ROM_END

ROM_START( hunchbkg )
	ROM_REGION( 0x8000, REGION_CPU1, 0 )
	ROM_LOAD( "gal_hb_1",     0x0000, 0x0800, CRC(46590e9b) SHA1(5d26578c91adec20d8d8a17d5dade9ef2febcbe5) )
	ROM_LOAD( "gal_hb_2",     0x0800, 0x0800, CRC(4e6e671c) SHA1(5948fc7f390f0343b367d333395427ce2f9b2931) )
	ROM_LOAD( "gal_hb_3",     0x2000, 0x0800, CRC(d29dc242) SHA1(3f6087fe962ee63c2886ad3f502c1a37d357ba87) )
	ROM_LOAD( "gal_hb_4",     0x2800, 0x0800, CRC(d409d292) SHA1(d631c9106106b31b605b6fdf1d4f40e237a725ac) )
	ROM_LOAD( "gal_hb_5",     0x4000, 0x0800, CRC(29d3a8c4) SHA1(2e1ef20d980e5033503d8095e9576dcb8f532f41) )
	ROM_LOAD( "gal_hb_6",     0x4800, 0x0800, CRC(b016fd15) SHA1(cdfbd531e23438f05a7c3aad99a94ce55912aac3) )
	ROM_LOAD( "gal_hb_7",     0x6000, 0x0800, CRC(d2731d27) SHA1(8c4a3d2303d85c3b11803c577a9ad21e6e69011e) )
	ROM_LOAD( "gal_hb_8",     0x6800, 0x0800, CRC(e4b1a666) SHA1(9f73d17cff208374d587536e783be024fc9ab700) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "gal_hb_kl",    0x0000, 0x0800, CRC(3977650e) SHA1(1de05d6ceed3f2ed0925caa8235b63a93f03f61e) )
	ROM_LOAD( "gal_hb_hj",    0x0800, 0x0800, CRC(db489c3d) SHA1(df08607ad07222c1c1c4b3589b50b785bdeefbf2) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "gal_hb_cp",    0x0000, 0x0020, CRC(cbff6762) SHA1(4515a6e12a0a5c485a55291feee17a571120a549) )
ROM_END

ROM_START( harem )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "p0_ic85.bin",  0x0000, 0x2000, CRC(4521b753) SHA1(9033f9c3be8fec1e5ff251e9f60faaf3848a1a1e) )
	ROM_LOAD( "p1_ic87.bin",  0x8000, 0x2000, BAD_DUMP CRC(3cc5d1e8) SHA1(827e2d20de2a00ec016ead249ed3afdccd0c856c) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "s1_ic12.bin",  0x0000, 0x2000, CRC(b54799dd) SHA1(b6aeb010257cba48a52afd33b4f8031c7d99550c) )
	ROM_LOAD( "s2_ic13.bin",  0x2000, 0x1000, CRC(2d5573a4) SHA1(1fdcd99d89e078509634742b2116a35bb199fe4b) )

	ROM_REGION( 0x2000, REGION_SOUND1, 0 )
	ROM_LOAD( "a1_ic25.bin",  0x0000, 0x2000, CRC(279f923a) SHA1(166b1b625997766f0de7cc18af52c42268022fcb) )

	ROM_REGION( 0x4000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "m0_ic36.bin",  0x0000, 0x2000, CRC(64b3c6d6) SHA1(e71092585f7ffdae85b2a4c9add1bc71e5a608a8) )
	ROM_LOAD( "m1_ic37.bin",  0x2000, 0x2000, CRC(cb0324fb) SHA1(61612f683810339d5d5f31daa4c475d0338d446f) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "harem.clr",    0x0000, 0x0020, CRC(c9a2bf73) SHA1(dad65ebf43a5df147e334afd552e67f5fcd26df7) )
ROM_END

ROM_START( tazzmang )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "tazzm1.4k",    0x0000, 0x1000, CRC(a14480a1) SHA1(60dac6b57e8331cc4daedaf87faf3e3acc68f378) )
	ROM_LOAD( "tazzm2.5j",    0x1000, 0x1000, CRC(5609f5db) SHA1(3fc50109ea0e012e3e310ae4f5dd0cf460bdca52) )
	ROM_LOAD( "tazzm3.6f",    0x2000, 0x1000, CRC(fe7f7002) SHA1(ac4134c07a798328b18994010bcaf6b3f728466a) )
	ROM_LOAD( "tazzm4.7e",    0x3000, 0x1000, CRC(c9ca1d0a) SHA1(d420ca2e926174e17215212278c86ba9bbb3d9dc) )
	ROM_LOAD( "tazzm5.7l",    0x4000, 0x1000, CRC(f50cd8a6) SHA1(b59ca37171b9acc9854f1beae43cfa5643219a5f) )
	ROM_LOAD( "tazzm6.7l",    0x5000, 0x1000, CRC(5cf2e7d2) SHA1(ad89e2655164e0fc5ecc9af70c5f0dd9b094d432) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "tazm8.1lk",    0x0000, 0x0800, CRC(2c5b612b) SHA1(32e3a41a9a4a8b1285b6a195213ff0d98012360a) )
	ROM_LOAD( "tazzm7.1jh",   0x0800, 0x0800, CRC(3f5ff3ac) SHA1(bc70eef54a45b52c14e35464e5f06b5eec554eb6) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "prom.6l",      0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( racknrol )
	ROM_REGION( 0x8000, REGION_CPU1, 0 )
	ROM_LOAD( "horz_p.bin",   0x0000, 0x1000, CRC(32ca5b43) SHA1(f3e7662f947dcdd80f6eae4f002d2fe64a825aff) )
	ROM_CONTINUE(			  0x2000, 0x1000 )
	ROM_CONTINUE(			  0x4000, 0x1000 )
	ROM_CONTINUE(			  0x6000, 0x1000 )

	ROM_REGION( 0x8000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "horz_g.bin",   0x0000, 0x4000, CRC(97069ad5) SHA1(50199c7bc5083be23a34849cff17906795bf4067) )
	ROM_LOAD( "horz_r.bin",   0x4000, 0x4000, CRC(ff64e84b) SHA1(ceabd522bae26743804987632f35f3c4b5aff179) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.bin",   0x0000, 0x0020, CRC(737802bf) SHA1(9b0476c51ce63898cd690e01e16ee83bae361cb2) )

	ROM_REGION( 0x0200, REGION_USER1, 0 ) /* unknown */
	ROM_LOAD( "82s147.bin",   0x0000, 0x0200, CRC(aace7fa5) SHA1(6761530bb3585d2eaa97b7ae77b52e96782ffe0a) )
ROM_END

ROM_START( hexpool )
	ROM_REGION( 0x8000, REGION_CPU1, 0 )
	ROM_LOAD( "vert_p.bin",   0x0000, 0x1000, CRC(bdb078fc) SHA1(85a65c3038dc05a98eae71edf9efdd6659a2966a) )
	ROM_CONTINUE(			  0x2000, 0x1000 )
	ROM_CONTINUE(			  0x4000, 0x1000 )
	ROM_CONTINUE(			  0x6000, 0x1000 )

	ROM_REGION( 0x8000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "vert_g.bin",   0x0000, 0x4000, CRC(7e257e80) SHA1(dabb10d076dc49fc130f58e6d1c4b04e6debce55) )
	ROM_LOAD( "vert_r.bin",   0x4000, 0x4000, CRC(c5f0851e) SHA1(cedcdb29962c6cd65af9d57d0cb2533397d58f99) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.bin",   0x0000, 0x0020, CRC(737802bf) SHA1(9b0476c51ce63898cd690e01e16ee83bae361cb2) )

	ROM_REGION( 0x0200, REGION_USER1, 0 ) /* unknown */
	ROM_LOAD( "82s147.bin",   0x0000, 0x0200, CRC(aace7fa5) SHA1(6761530bb3585d2eaa97b7ae77b52e96782ffe0a) )
ROM_END

ROM_START( hexpoola )
	ROM_REGION( 0x8000, REGION_CPU1, 0 )
	ROM_LOAD( "rom.4l",       0x0000, 0x1000, CRC(2ca8018d) SHA1(f0784d18bc7e77515bf2140d8993ae8178919853) )
	ROM_CONTINUE(			  0x2000, 0x1000 )
	ROM_CONTINUE(			  0x4000, 0x1000 )
	ROM_CONTINUE(			  0x6000, 0x1000 )

	ROM_REGION( 0x8000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "rom.1m",       0x0000, 0x4000, CRC(7e257e80) SHA1(dabb10d076dc49fc130f58e6d1c4b04e6debce55) )
	ROM_LOAD( "rom.1l",       0x4000, 0x4000, CRC(c5f0851e) SHA1(cedcdb29962c6cd65af9d57d0cb2533397d58f99) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.11r",   0x0000, 0x0020, CRC(deb2fcf4) SHA1(cdec737a9d9feae912f7cc04ca0adb48f859b5c2) )

	ROM_REGION( 0x0200, REGION_USER1, 0 ) /* unknown */
	ROM_LOAD( "82s147.5pr",   0x0000, 0x0200, CRC(cf496b1e) SHA1(5b5ca52b3cc46e18990dae53a98984aeaf264241) )

	ROM_REGION( 0x00eb, REGION_PLDS, 0 )
	ROM_LOAD( "82s153.6pr.bin", 0x0000, 0x00eb, CRC(bc07939a) SHA1(615b085575ad215662eab2777a2d8b9167c4b9c3) )
ROM_END

ROM_START( trvchlng )
	ROM_REGION( 0x8000, REGION_CPU1, 0 )
	ROM_LOAD( "senko11.bin",  0x0000, 0x1000, CRC(3657331d) SHA1(d9a9a4e4e2e696e70dfb888725c959ec8ce24e3d) )
	ROM_CONTINUE(			  0x2000, 0x1000 )
	ROM_CONTINUE(			  0x4000, 0x1000 )
	ROM_CONTINUE(			  0x6000, 0x1000 )

	ROM_REGION( 0x100000, REGION_USER1, 0 )
	ROM_LOAD( "questions",    0x000000, 0x100000, NO_DUMP )

	ROM_REGION( 0x8000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "senko10.bin",  0x0000, 0x4000, CRC(234b59d0) SHA1(5eafdfc6d6a73575835b68361fe29a2dc61e8a83) )
	ROM_LOAD( "senko12.bin",  0x4000, 0x4000, CRC(0bf6b92d) SHA1(6ca993c0642949a52fafea3bc57a08c6881e8120) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "senko1.bin",   0x0000, 0x0020, CRC(1434c7ff) SHA1(0ee5f5351dd84fbf8d3d8eaafbdbe86dd29960f8) )
ROM_END

ROM_START( luctoday )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "ltprog1.bin", 0x0000, 0x0800, CRC(59c389b9) SHA1(1e158ced3b56db2c51e422fb4c0b8893565f1956))
	ROM_LOAD( "ltprog2.bin", 0x2000, 0x0800, CRC(ac3893b1) SHA1(f6b9cd8111b367ff7030cba52fe965959d92568f))

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "ltchar2.bin", 0x0000, 0x0800, CRC(8cd73bdc) SHA1(6174f7347d2c96f9c5074bc0da5a370c9b07461b))
	ROM_LOAD( "ltchar1.bin", 0x0800, 0x0800, CRC(b5ba9946) SHA1(7222cbe8c41ca74b214f4dd5439bf69d90f4644e))

	ROM_REGION( 0x0020, REGION_PROMS, 0 )//This may not be the correct prom
	ROM_LOAD( "74s288.ch", 0x0000, 0x0020, BAD_DUMP CRC(24652bc4) SHA1(d89575f3749c75dc963317fe451ffeffd9856e4d))
ROM_END

ROM_START( chewing )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "1.bin", 0x0000, 0x1000, CRC(7470b347) SHA1(315d2631b50a6e469b9538318d95452e8d2e1f69) )
	ROM_LOAD( "7l.bin", 0x2000, 0x0800, CRC(78ebed36) SHA1(e80185737c8ac448901cf0e60ca50d967c323b34) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "2.bin", 0x0000, 0x0800, CRC(88c605f3) SHA1(938a9fadfa0994a1d2fc9b3266ec4ccdb5ec6d3a) )
	ROM_LOAD( "3.bin", 0x0800, 0x0800, CRC(77ac016a) SHA1(fa5b1e79603ca8d2ee7b3d0a78f12d9ffeec3fd4) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "74s288.ch", 0x0000, 0x0020, CRC(24652bc4) SHA1(d89575f3749c75dc963317fe451ffeffd9856e4d) )
ROM_END

ROM_START( catacomb )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "catacomb.u",    0x0000, 0x0800, CRC(35cc28d2) SHA1(e1dbd75fc21ec88b8119bf9508c87d78e1d5c4f6) )
	ROM_LOAD( "catacomb.v",    0x0800, 0x0800, CRC(1d1ce133) SHA1(e22a169003a2238004bdf6c2558198216c2353b7) )
	ROM_LOAD( "catacomb.w",    0x1000, 0x0800, CRC(479bbde7) SHA1(9981662cb6351de7c1730de45f645fb0e26ea467) )
	/* no .x */
	ROM_LOAD( "catacomb.y",    0x2000, 0x0800, CRC(5e3da534) SHA1(a9b960ae96c8ef0b2d590bc58b711aad949025e2) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "cat-gfx1",       0x0000, 0x0800, CRC(e871e65c) SHA1(0b528dfab0f57153db9406798848cdedee0323a0) )
	ROM_LOAD( "cat-gfx2",       0x0800, 0x0800, CRC(b14dafaa) SHA1(592d5931a76563b3565f22ac4c0120b9a120193f) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	/* No color PROM came with the conversion - the Moon Cresta one seems more appropriate than Galaxian,
       (the game is unplayable with a Galaxian PROM) but which was intended for use with the kit is unclear */
	ROM_LOAD( "mmi6331.6l", 0x0000, 0x0020, BAD_DUMP CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( ckongg )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "ck1.bin",       0x2400, 0x0400, CRC(a4323b94) SHA1(1fed47e1df5efa8f40585bedab07b60067edc2bb) )
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

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "ckvid10.bin",   0x0000, 0x1000, CRC(7866d2cb) SHA1(62dd8b80bc0459c7337d8a8cb83e53b999e7f4a9) )
	ROM_LOAD( "ckvid7.bin",    0x1000, 0x1000, CRC(7311a101) SHA1(49d54c8b94cae4ba81d7a7684eaa4e87815bb4da) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "ck_cp.bin",     0x0000, 0x0020, CRC(7e0b79cb) SHA1(72ef3eb5f09e10c13dcf6fd568a6d16658055a16) )
ROM_END

ROM_START( kkgalax )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
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

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "kc8carat.bin",   0x0000, 0x1000, CRC(7866d2cb) SHA1(62dd8b80bc0459c7337d8a8cb83e53b999e7f4a9) )
	ROM_LOAD( "kc9carat.bin",   0x1000, 0x1000, CRC(7311a101) SHA1(49d54c8b94cae4ba81d7a7684eaa4e87815bb4da) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 ) // not in this set
	ROM_LOAD( "ck_cp.bin",     0x0000, 0x0020, CRC(7e0b79cb) SHA1(72ef3eb5f09e10c13dcf6fd568a6d16658055a16) )
ROM_END


ROM_START( porter )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
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

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "port7.bin",          0x0000, 0x1000, CRC(603294f9) SHA1(168b90fdf38cd2e2c7f54cde16b4d83dc5bb3046) )
	ROM_LOAD( "port8.bin",          0x1000, 0x1000, CRC(b66a763d) SHA1(995b473b1942ff666b0989993587e41e89542172) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 ) // not in the set
	ROM_LOAD( "mmi6331.6l", 0x0000, 0x0020, BAD_DUMP CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) /* Compatible with 82s123 prom */
ROM_END

GAME( 1979, galaxian, 0,        galaxian, galaxian, 0,        ROT90,  "Namco", "Galaxian (Namco set 1)", GAME_SUPPORTS_SAVE )
GAME( 1979, galaxiaj, galaxian, galaxian, superg,   0,        ROT90,  "Namco", "Galaxian (Namco set 2)", GAME_SUPPORTS_SAVE )
GAME( 1979, galmidw,  galaxian, galaxian, galaxian, 0,        ROT90,  "[Namco] (Midway license)", "Galaxian (Midway)", GAME_SUPPORTS_SAVE )
GAME( 1979, galmidwo, galaxian, galaxian, galaxian, 0,        ROT90,  "[Namco] (Midway license)", "Galaxian (Midway, old rev)", GAME_SUPPORTS_SAVE )
GAME( 1979, superg,   galaxian, galaxian, superg,   0,        ROT90,  "hack", "Super Galaxians", GAME_SUPPORTS_SAVE )
GAME( 1979, galapx,   galaxian, galaxian, superg,   0,        ROT90,  "hack", "Galaxian Part X", GAME_SUPPORTS_SAVE )
GAME( 19??, moonaln,  galaxian, galaxian, superg,   0,        ROT90,  "[Nichibutsu] (Karateco license)", "Moon Alien", GAME_SUPPORTS_SAVE )
GAME( 1979, galap1,   galaxian, galaxian, superg,   0,        ROT90,  "hack", "Space Invaders Galactica", GAME_SUPPORTS_SAVE )
GAME( 1979, galap4,   galaxian, galaxian, superg,   0,        ROT90,  "hack", "Galaxian Part 4", GAME_SUPPORTS_SAVE )
GAME( 1979, galturbo, galaxian, galaxian, superg,   0,        ROT90,  "hack", "Galaxian Turbo", GAME_SUPPORTS_SAVE )
GAME( 1979, swarm,    galaxian, galaxian, swarm,    0,        ROT90,  "[Namco] (Sub-Electro bootleg)", "Swarm (hack of Galaxian)", GAME_SUPPORTS_SAVE )
GAME( 1979, zerotime, galaxian, galaxian, zerotime, 0,        ROT90,  "Petaco S.A.", "Zero Time", GAME_SUPPORTS_SAVE )
GAME( 1979, starfght, galaxian, galaxian, swarm,    0,        ROT90,  "Jeutel", "Star Fighter (bootleg of Galaxian)", GAME_SUPPORTS_SAVE )
GAME( 1979, galaxbsf, galaxian, galaxian, galaxian, 0,        ROT90,  "bootleg", "Galaxian (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 19??, tst_galx, galaxian, galaxian, galaxian, 0,        ROT90,  "Test ROM", "Galaxian Test ROM", GAME_SUPPORTS_SAVE )
GAME( 1981, gmgalax,  0,        gmgalax,  gmgalax,  gmgalax,  ROT90,  "bootleg", "Ghostmuncher Galaxian (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 19??, pisces,   0,        pisces,   pisces,   pisces,   ROT90,  "Subelectro", "Pisces", GAME_SUPPORTS_SAVE )
GAME( 19??, piscesb,  pisces,   pisces,   piscesb,  pisces,   ROT90,  "bootleg", "Pisces (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 19??, omni,     pisces,   pisces,   piscesb,  pisces,   ROT90,  "bootleg", "Omni", GAME_SUPPORTS_SAVE )
GAME( 1980, uniwars,  0,        pisces,   superg,   pisces,   ROT90,  "Irem", "UniWar S", GAME_SUPPORTS_SAVE )
GAME( 1980, gteikoku, uniwars,  pisces,   superg,   pisces,   ROT90,  "Irem", "Gingateikoku No Gyakushu", GAME_SUPPORTS_SAVE )
GAME( 1980, gteikokb, uniwars,  pisces,   gteikokb, pisces,   ROT270, "bootleg", "Gingateikoku No Gyakushu (bootleg set 1)", GAME_SUPPORTS_SAVE )
GAME( 1980, gteikob2, uniwars,  gteikob2, gteikob2, gteikob2, ROT270, "bootleg", "Gingateikoku No Gyakushu (bootleg set 2)", GAME_SUPPORTS_SAVE )
GAME( 1980, spacbatt, uniwars,  pisces,   spacbatt, pisces,   ROT90,  "bootleg", "Space Battle (bootleg set 1)", GAME_SUPPORTS_SAVE )
GAME( 1980, spacbat2, uniwars,  pisces,   spacbatt, pisces,   ROT90,  "bootleg", "Space Battle (bootleg set 2)", GAME_SUPPORTS_SAVE )
GAME( 1980, skyraidr, uniwars,  pisces,   superg,   pisces,   ROT90,  "bootleg", "Sky Raiders", GAME_SUPPORTS_SAVE )
GAME( 1981, batman2,  phoenix,  batman2,  batman2,  pisces,   ROT270, "bootleg", "Batman Part 2", GAME_SUPPORTS_SAVE )
GAME( 1981, warofbug, 0,        galaxian, warofbug, pisces,   ROT90,  "Armenia", "War of the Bugs or Monsterous Manouvers in a Mushroom Maze", GAME_SUPPORTS_SAVE )
GAME( 19??, redufo,   0,        galaxian, redufo,   pisces,   ROT90,  "bootleg", "Defend the Terra Attack on the Red UFO", GAME_SUPPORTS_SAVE )
GAME( 19??, exodus,   redufo,   galaxian, exodus,   pisces,   ROT90,  "Subelectro", "Exodus (bootleg?)", GAME_SUPPORTS_SAVE )
GAME( 1981, streakng, 0,        pacmanbl, streakng, 0,        ROT90,  "Shoei", "Streaking", GAME_IMPERFECT_COLORS | GAME_SUPPORTS_SAVE )
GAME( 1981, pacmanbl, puckman,  pacmanbl, pacmanbl, pisces,   ROT270, "bootleg", "Pac-Man (Galaxian hardware)", GAME_SUPPORTS_SAVE )
GAME( 1984, devilfsg, devilfsh, devilfsg, devilfsg, 0,        ROT270, "Vision / Artic", "Devil Fish (Galaxian hardware, bootleg?)", GAME_SUPPORTS_SAVE )
GAME( 1982, zigzag,   0,        zigzag,   zigzag,   zigzag,   ROT90,  "LAX", "Zig Zag (Galaxian hardware, set 1)", GAME_SUPPORTS_SAVE )
GAME( 1982, zigzag2,  zigzag,   zigzag,   zigzag,   zigzag,   ROT90,  "LAX", "Zig Zag (Galaxian hardware, set 2)", GAME_SUPPORTS_SAVE )
GAME( 1981, scramblb, scramble, scramblb, scramblb, 0,        ROT90,  "bootleg", "Scramble (Galaxian hardware)", GAME_SUPPORTS_SAVE )
GAME( 1981, scramb2,  scramble, scramb2,  scramb2,  0,        ROT90,  "bootleg", "Scramble (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1981, jumpbug,  0,        jumpbug,  jumpbug,  0,        ROT90,  "Rock-ola", "Jump Bug", GAME_SUPPORTS_SAVE )
GAME( 1981, jumpbugb, jumpbug,  jumpbug,  jumpbug,  0,        ROT90,  "bootleg", "Jump Bug (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1983, levers,   0,        jumpbug,  levers,   0,        ROT90,  "Rock-ola", "Levers", GAME_SUPPORTS_SAVE )
GAME( 1982, azurian,  0,        azurian,  azurian,  azurian,  ROT90,  "Rait Electronics Ltd", "Azurian Attack", GAME_SUPPORTS_SAVE )
GAME( 19??, orbitron, 0,        galaxian, orbitron, pisces,   ROT270, "Signatron USA", "Orbitron", GAME_SUPPORTS_SAVE )
GAME( 1982, checkman, 0,        checkman, checkman, checkman, ROT90,  "Zilec-Zenitone", "Check Man", GAME_SUPPORTS_SAVE )
GAME( 1982, checkmaj, checkman, checkmaj, checkmaj, checkmaj, ROT90,  "Jaleco", "Check Man (Japan)", GAME_SUPPORTS_SAVE )
GAME( 1983, dingo,    0,        checkmaj, dingo,    dingo,    ROT90,  "Ashby Computers and Graphics LTD. (Jaleco license)", "Dingo", GAME_SUPPORTS_SAVE )
GAME( 1983, dingoe,   0,        dingoe,   dingo,    dingoe,   ROT90,  "Ashby Computers and Graphics LTD.", "Dingo (encrypted)", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
GAME( 1981, blkhole,  0,        galaxian, blkhole,  0,        ROT90,  "TDS", "Black Hole", GAME_SUPPORTS_SAVE )
GAME( 1980, mooncrst, 0,        mooncrst, mooncrst, mooncrst, ROT90,  "Nichibutsu", "Moon Cresta (Nichibutsu)", GAME_SUPPORTS_SAVE )
GAME( 1980, mooncrsu, mooncrst, mooncrst, mooncrst, mooncrsu, ROT90,  "Nichibutsu USA", "Moon Cresta (Nichibutsu, unencrypted)", GAME_SUPPORTS_SAVE )
GAME( 1980, mooncrsa, mooncrst, mooncrst, mooncrsa, mooncrst, ROT90,  "Nichibutsu", "Moon Cresta (Nichibutsu, old rev)", GAME_SUPPORTS_SAVE )
GAME( 1980, mooncrsg, mooncrst, mooncrst, mooncrsg, mooncrsu, ROT90,  "Gremlin", "Moon Cresta (Gremlin)", GAME_SUPPORTS_SAVE )
GAME( 1980, mooncrsb, mooncrst, mooncrst, mooncrsa, mooncrsu, ROT90,  "bootleg", "Moon Cresta (bootleg set 1)", GAME_SUPPORTS_SAVE )
GAME( 1980, mooncrs2, mooncrst, mooncrst, mooncrsa, mooncrsu, ROT90,  "Nichibutsu", "Moon Cresta (bootleg set 2)", GAME_SUPPORTS_SAVE )
GAME( 1980, mooncrs3, mooncrst, mooncrst, mooncrst, mooncrsu, ROT90,  "bootleg", "Moon Cresta (bootleg set 3)", GAME_SUPPORTS_SAVE ) /* Jeutel bootleg, similar to bootleg set 2 */
GAME( 1980, fantazia, mooncrst, mooncrst, fantazia, mooncrsu, ROT90,  "Subelectro", "Fantazia (bootleg?)", GAME_SUPPORTS_SAVE )
GAME( 1980, eagle,    mooncrst, mooncrst, eagle,    mooncrsu, ROT90,  "Centuri", "Eagle (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1980, eagle2,   mooncrst, mooncrst, eagle2,   mooncrsu, ROT90,  "Centuri", "Eagle (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1980, eagle3,   mooncrst, mooncrst, eagle,    mooncrsu, ROT90,  "Centuri", "Eagle (set 3)", GAME_SUPPORTS_SAVE )
GAME( 1981?,spctbird, mooncrst, mooncrst, eagle2,   mooncrsu, ROT90,  "Fortrek", "Space Thunderbird", GAME_SUPPORTS_SAVE )
GAME( 1980?,smooncrs, mooncrst, mooncrst, smooncrs, mooncrsu, ROT90,  "Gremlin", "Super Moon Cresta", GAME_SUPPORTS_SAVE )
GAME( 198?, mooncmw,  mooncrst, mooncrst, mooncrsa, mooncrsu, ROT90,  "bootleg", "Moon War (Moon Cresta bootleg)", GAME_SUPPORTS_SAVE )
// The boards were marked 'Space Dragon' although this doesn't appear in the games.
GAME( 1980, spcdrag,  mooncrst, mooncrst, spcdrag,  mooncrsu, ROT90,  "bootleg", "Space Dragon (Moon Cresta bootleg, set 1)", GAME_SUPPORTS_SAVE )
GAME( 1980, spcdraga, mooncrst, mooncrst, spcdrag,  mooncrsu, ROT90,  "bootleg", "Space Dragon (Moon Cresta bootleg, set 2)", GAME_SUPPORTS_SAVE )
GAME( 1980, mooncrgx, mooncrst, mooncrgx, mooncrgx, mooncrgx, ROT270, "bootleg", "Moon Cresta (Galaxian hardware)", GAME_SUPPORTS_SAVE )
GAME( 1980, moonqsr,  0,        moonqsr,  moonqsr,  moonqsr,  ROT90,  "Nichibutsu", "Moon Quasar", GAME_SUPPORTS_SAVE )
GAME( 1981, mshuttle, 0,        mshuttle, mshuttle, mshuttle, ROT0,   "Nichibutsu", "Moon Shuttle (US?)", GAME_SUPPORTS_SAVE )
GAME( 1981, mshuttlj, mshuttle, mshuttle, mshuttle, cclimbrj, ROT0,   "Nichibutsu", "Moon Shuttle (Japan set 1)", GAME_SUPPORTS_SAVE )
GAME( 1981, mshutlj2, mshuttle, mshuttle, mshuttle, cclimbrj, ROT0,   "Nichibutsu", "Moon Shuttle (Japan set 2)", GAME_SUPPORTS_SAVE )
GAME( 1980, moonal2,  0,        mooncrst, moonal2,  0,        ROT90,  "Nichibutsu", "Moon Alien Part 2", GAME_SUPPORTS_SAVE )
GAME( 1980, moonal2b, moonal2,  mooncrst, moonal2,  0,        ROT90,  "Nichibutsu", "Moon Alien Part 2 (older version)", GAME_SUPPORTS_SAVE )
GAME( 1980, supergx,  moonal2,  galaxian, superg,   0,        ROT90,  "Nichibutsu", "Super GX", GAME_NOT_WORKING | GAME_WRONG_COLORS | GAME_SUPPORTS_SAVE )
GAME( 1982, skybase,  0,        skybase,  skybase,  0,        ROT90,  "Omori Electric Co., Ltd.", "Sky Base", GAME_SUPPORTS_SAVE )
GAME( 19??, omega,    theend,   galaxian, omega,    0,        ROT270, "bootleg?", "Omega", GAME_SUPPORTS_SAVE )
GAME( 1980, kingball, 0,        kingball, kingball, kingball, ROT90,  "Namco", "King & Balloon (US)", GAME_SUPPORTS_SAVE )
GAME( 1980, kingbalj, kingball, kingball, kingball, kingball, ROT90,  "Namco", "King & Balloon (Japan)", GAME_SUPPORTS_SAVE )
GAME( 19??, scorpnmc, scorpion, scorpnmc, scorpnmc, 0,        ROT90,  "Dorneer", "Scorpion (Moon Cresta hardware)", GAME_SUPPORTS_SAVE )
GAME( 1981, frogg,    frogger,  frogg,    frogg,    0,        ROT90,  "bootleg", "Frog (Galaxian hardware)", GAME_SUPPORTS_SAVE )
GAME( 1981, 4in1,     0,        4in1,     4in1,     4in1,     ROT90,  "Armenia / Food and Fun", "4 Fun in 1", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1982, bagmanmc, bagman,   bagmanmc, bagmanmc, 0,        ROT90,  "bootleg", "Bagman (Moon Cresta hardware)", GAME_WRONG_COLORS | GAME_SUPPORTS_SAVE )
GAME( 1982, dkongjrm, dkongjr,  dkongjrm, dkongjrm, 0,        ROT90,  "bootleg", "Donkey Kong Jr. (Moon Cresta hardware)", GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1981, rockclim, 0,        rockclim, rockclim, 0,	      ROT180, "Taito", "Rock Climber", GAME_SUPPORTS_SAVE )
GAME( 1983, ozon1,    0,        ozon1,    ozon1,    0,	      ROT90,  "Proma", "Ozon I", GAME_SUPPORTS_SAVE )
GAME( 1983, ladybugg, ladybug,  batman2,  ladybugg, ladybugg, ROT270, "bootleg", "Ladybug (bootleg on Galaxian hardware)", GAME_SUPPORTS_SAVE )
GAME( 1980, vpool,    hustler,  mooncrst, vpool,    0,        ROT90,  "bootleg", "Video Pool (bootleg on Moon Cresta hardware)", GAME_SUPPORTS_SAVE )
GAME( 1984, drivfrcg, drivfrcp, drivfrcg, drivfrcg, 0,        ROT90,  "Shinkai Inc. (Magic Eletronics USA licence)", "Driving Force (Galaxian conversion)", GAME_SUPPORTS_SAVE )
GAME( 1985, drivfrcb, drivfrcp, drivfrcg, drivfrcg, 0,        ROT90,  "bootleg", "Driving Force (Galaxian conversion bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1983, bongo,    0,        bongo,    bongo,    0,	      ROT90,  "Jetsoft", "Bongo", GAME_SUPPORTS_SAVE )
GAME( 1983, hunchbkg, hunchbak,	hunchbkg, hunchbkg, 0,        ROT90,  "Century Electronics", "Hunchback (Galaxian hardware)", GAME_SUPPORTS_SAVE )
GAME( 1983, harem,    0,        harem,    harem,    0,        ROT90,  "I.G.R.", "Harem", GAME_NO_SOUND | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
GAME( 1982, tazzmang, tazmania,	tazzmang, tazzmang, 0,        ROT90,  "bootleg", "Tazz-Mania (Galaxian Hardware)", GAME_SUPPORTS_SAVE )
GAME( 1986, racknrol, 0,        racknrol, racknrol, 0,	      ROT0,   "Status (Shinkai License)", "Rack + Roll", GAME_SUPPORTS_SAVE )
GAME( 1986, hexpool,  racknrol, racknrol, racknrol, 0,	      ROT90,  "Shinkai", "Hex Pool (Shinkai)", GAME_SUPPORTS_SAVE )
GAME( 1985, hexpoola, racknrol, hexpoola, racknrol, 0,	      ROT90,  "Senko", "Hex Pool (Senko)", GAME_SUPPORTS_SAVE )
GAME( 1985, trvchlng, 0,        racknrol, trvchlng, 0,	      ROT90,  "Joyland (Senko License)", "Trivia Challenge", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
GAME( 1980, luctoday, 0,        galaxian, luctoday, 0,        ROT270, "Sigma", "Lucky Today",GAME_WRONG_COLORS | GAME_SUPPORTS_SAVE )
GAME( 19??, chewing,  0,        galaxian, luctoday, 0,        ROT90,  "unknown", "Chewing Gum", GAME_SUPPORTS_SAVE )
GAME( 1982, catacomb, 0,        galaxian, catacomb, 0,        ROT90,  "MTM Games", "Catacomb", GAME_WRONG_COLORS | GAME_SUPPORTS_SAVE )
GAME( 1981, ckongg,   0,        ckongg,   ckongg,   0,        ROT90,  "bootleg", "Crazy Kong (bootleg on Galaxian hardware, set 1)", GAME_SUPPORTS_SAVE )
GAME( 1981, kkgalax,  ckongg,   kkgalax,  kkgalax,  0,        ROT90,  "bootleg", "Crazy Kong (bootleg on Galaxian hardware, set 2)", GAME_NO_SOUND | GAME_SUPPORTS_SAVE ) // set was marked as 'King Kong on Galaxian'
GAME( 19??, porter,   0,        mooncrst, porter,   0,        ROT90,  "[Nova Games Ltd.] (bootleg)", "Port Man (bootleg on Moon Cresta hardware)", GAME_IMPERFECT_GRAPHICS ) // missing GFX bank switch!
