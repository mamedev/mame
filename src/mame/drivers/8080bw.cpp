// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Tormod Tjaberg, Mirko Buffoni,Lee Taylor, Valerio Verrando, Zsolt Vasvari,Aaron Giles,Jonathan Gevaryahu,hap,Robbbert
// thanks-to:Michael Strutts, Marco Cassili
/*****************************************************************************

    8080bw.c

    Michael Strutts, Nicola Salmoria, Tormod Tjaberg, Mirko Buffoni
    Lee Taylor, Valerio Verrando, Marco Cassili, Zsolt Vasvari, and others

    Much information about Space Invaders PCBs and other Taito and Midway
    sets and hardware contributed by Andrew Welburn


    Notes:
    -----

    - Space Invaders Deluxe still says Space Invaders Part II,
      because according to KLOV, Midway was only allowed to make minor
      modifications of the Taito code.

    - DIP settings/locations verified from manual for:
      sitv, sicv, invadpt2, lrescue, invasion, invrvnge

    - The Taito Space invaders hardware comes on at least five board types;
      The Taito manufactured ones are:
      * The "L-shaped" PCB set, Upright, B&W only
      * Three pcbs in a stack, often called the '3 layer pcb set' (most common)
      * Two pcbs in a stack, with the function of two of the three pcb stack
        pcbs combined.
      * In general, discounting revision specific differences, the pcbs are
        arranged in a stack, connected end-to-end by ribbon cables and folded
        such that the middle pcb (of 3 pcb stack) or the bottom pcb (of 2 pcb
        stack) is upside down.
      * Keep in mind specific differences on the pcbs (especially the TVN pcbs
        vs the others) sometimes prevent exchanging 'equivalent' pcbs between stacks.
        * L-shaped pcb set details:
          * One large, square board with ROM, RAM, CPU, Video circuitry on it.
          * One smaller pcb with audio/io/shifter circuitry on it, plugged into
            the main pcb at a right angle (hence the entire pcb set is 'L-shaped')
          * Does not have the capability of any sort of electronic color overlay.
        * 3 layer pcb set details:
          * This pcb set came in 3 versions: TVN, CVN, PVN; see below for differences.
          * Top pcb: Audio/IO/Shifter pcb
            - discrete audio and sn76477, and volume knobs
            - data shifter, using either ~11 74xx chips, AM25S10s, Fujitsu MB14221
              or Fujitsu MB14241 chips, which all do the same thing.
            - has the dipswitches
            - has the main "G" edge connector for the wiring harness
            - has the gating circuitry for the color overlay
              (these are not present and/or populated on some TVN pcbs)
            - despite there being at least six versions of this pcb, the discrete
              audio section is identical in all of them.
          * Middle pcb: CPU/RAM/Color overlay pcb
            - has DRAMS on it
            - has the 8080 CPU on it
            - has the two PROMS for color overlay on it (one for each player flip)
              (these are not populated on TVN pcbs and the related circuitry is not
              present and/or populated on some TVN pcbs)
          * Bottom pcb: Power/Video/ROM pcb
            - has the game roms on it
            - has the main B&W video generation logic on it
            - has the larger connection to the PSU, and B&W composite output "T" connector
        * 2 layer pcb set details:
          * This pcb set came in one version: PVN, and is entirely exchangeable
            with the 3 layer PVN pcb set.
          * Top pcb is same as 3 layer pcb set
          * Bottom pcb combines the function of the Middle and Bottom pcbs
            of the 3 layer set.

       * The different pcb set versions are noted by a different version code on
         a paper sticker on the pcb; The code will be of the format mVNnnnnn where
         m is a letter and nnnnn is a number.
         Codes:
         * TVNxxxxx (3 layer pcbset) - B&W only, used on "T.T Space Invaders"
           cocktail with 'blended' single-sheet gel color overlay.
     ***TODO: this overlay is not supported yet!
           Several revisions (at least 5 rom, 3 cpu, 3 audio) of each pcb exist
           for this set.
           Does support flipscreen.
           Does not have the color overlay circuitry nor places for it on the pcbs.
           (Later TVNxxxxx are actually rebadged CVNxxxxx, see below)
           Came from factory with one of the SV or TV romsets.
           Capable of running TV, SV or CV romsets.
           This pcb set is probably the oldest one and was designed at Taito.
         * SVNxxxxx? (L-shaped pcbset) - B&W only, used on "Space Invaders" Upright
           with 3-separate-sheets-of-gel 'strips' color overlay.
     ***TODO: this overlay might not be supported properly yet!
           Does not support flipscreen, was intended for upright cabinets only.
           Audio PCB daughterboard has part number SVN00001 or SVN00003.
           Came from factory with one of the 6x 0x400 or 4x 0x800 romsets
           Capable of running TV, SV or CV romsets.
           This is probably the second-oldest pcb set and may have been made to
           allow closer physical interchangability with Midway's m8080bw hardware,
           which had a similar board shape?
         * CVNxxxxx (3 layer pcbset) - Color, used on "T.T Space Invaders Color"
           cocktail with electronic color overlay.
           Does support flipscreen.
           Note that later TVNxxxxx pcbsets are actually 'rebadged' CVNxxxxx
           pcbsets with the color overlay circuitry unpopulated, and can be
           'upgraded' to CVNxxxxx by adding a few components and proms.
           Came from factory with one of the CV romsets.
           Capable of running TV, SV or CV romsets.
         * PVNxxxxx (2&3 layer pcbsets) - Color, used on "T.T Space Invaders Part
           II" cocktail with electronic color overlay.
           Several revisions (at least 3 rom, 1 cpu, 2 audio) of each pcb exist
           for this set.
           Came from factory with UV (2708) or PV (2716) romsets.
           Capable of running TV, SV, CV, UV or PV romsets.

       * The following Romsets are known, ROUGHLY from oldest to newest:
         SV01, SV02, SV03, SV04, SV05, SV06 - undumped (rev 1), If this exists at all this would be the very first japan release of space invaders (Andy W may call this 'SV0'?)
         SV01, SV02, SV10, SV04, SV09, SV06 - sisv2 (rev 2) (Andy W calls this 'SV1', and the midway 'invaders' set is based on this romset)
         SV0H, SV02, SV10, SV04, SV09, SV06 - sisv3 (rev 3) (Andy W calls this 'SV2')
         SV0H, SV11, SV12, SV04, SV13, SV14 - sisv (rev 4, 5-digit scoring) (Andy W calls this 'SV3') (this set is likely newer than the TV0x sets)
         TV01, TV02, TV03, TV04 - sitv1 (rev 1)
         TV0H, TV02, TV03, TV04 - sitv (rev 2 with bug fixes)
         CV03, CV04, CV05, CV06 w/proms - undumped (but may be the same as one of the sisv sets with the roms combined to 2716 size)
         CV17, CV18, CV19, CV20 w/proms - sicv
         UV1, UV2, UV3, UV4, UV5, UV6, UV7, UV8, UV9, UV10 w/proms - undumped (probably same as pvxx set just split differently)
         PV01, PV02, PV03, PV04, PV05 w/proms - invadpt2
         Note: SV0H and TV0H are called in taito documentation "SV01-1" and "TV01-1" most likely due to someone along the line mistaking the '1-1' for an H or vice versa when writing the documentation or creating the labels.

    - Midway PCB sets: (cursory descripton)
      * All Midway Space Invaders games ([Space Invader Upright], [Space Invader Cocktail],
        [Deluxe Space Invaders Upright], [Deluxe Space Invaders Cocktail], and [Space Invaders II])
        use the same m8080bw mainboard, with no emulation-relevant differences between revisions.
      * [Space Invaders II] from Midway (only produced as a cocktail) uses
        an extra sound board for the simultaneous 2 player head-to-head sounds.

    - Taito-USA-made 'trimline' PCBS do not match the taito japan-made pcbs either.


    To Do:
    -----

    - Midway PCB sets
      * The discrete components, particularly for the shot sound, differ
        between Taito and Midway audio daughterboards.
        + Figure out the difference between the Taito and Midway discrete
          boards and emulate them both properly.
        + Figure out what the current discrete setup is trying to emulate.
      * Remove Space Invaders 'invaders' set from mw8080bw.c, it does not belong
        there at all

    - Space Chaser (schaser)
      1. Schematic has SX2 & 4 swapped by mistake.

      2. Dipswitch 4 we have listed as "Easy/Hard", however the manual says
         it should not be used. The Hard position displays many bugs.

      3. Confirmation of these on a real machine (schaserb set) have been
         received from the owner of the PCB.
        "Hi Rob,
         I seem to get the same bugs as you with Dip4 set to off.  Score starts at 9000,
         2 missiles on first level etc..  It makes no mention of dip4 adjusting the hardness
         in the manual, are you guys sure that's what it was intended for "in the factory"?
         However you were correct in thinking the schematics were wrong.  My multimeter found
         the following:
         15k resistor - pin 7
         39k - pin 10
         82k - pin 12
         It looks like they have Sx2 and Sx4 the wrong way round on a latter page of the schematics."
           Thanks to Andy Raven for getting this information.

      4. The "Hard" mode bug can be fixed with a single byte patch: ROM_FILL( 0x47e5, 1, 0xc3)

      5. I have seen real machines with Hard mode set, that worked properly, thus there
         must be yet another romset still waiting to be found.

      6. Strange bug; sometimes the missile sound continues into and past the
         music/explosion. This didn't happen on the real machines I played.

    - Space Chaser (schasercv)
         These cheats exist in this game:
         1. Hold down 2P DOWN (the F key) while it says INSERT COIN. Then
            insert a coin and play. You will have 2 extra ships.
         2. In the Hard difficulty setting, you normally start at level 4.
            Hold down the 1P START (the 1 key) while it says INSERT COIN.
            Then insert a coin and play. You will start at level 5.

    - Space War (Sanritsu)
      * I seem to recall that the flashing ufo had its own sample
        sound, a sort of rattling noise. Unable to find evidence
        of this (so far).

    - Steel Worker, Space Combat
        Holding down the coin button causes the credits to rapidly increase.

    - Solfight
        On the stage with rolling balls, lots of random dashes appear.


*****************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "cpu/i8085/i8085.h"
#include "sound/ay8910.h"
#include "sound/speaker.h"
#include "includes/8080bw.h"

#include "attackfc.lh"
#include "cosmicm.lh"
#include "galactic.lh"
#include "gunchamp.lh"
#include "shuttlei.lh"
#include "spacecom.lh"


/*******************************************************/
/*                                                     */
/* Games with additional sound / video hardware        */
/*                                                     */
/*******************************************************/

MACHINE_START_MEMBER(_8080bw_state,extra_8080bw)
{
	MACHINE_START_CALL_MEMBER(extra_8080bw_sh);
	MACHINE_START_CALL_MEMBER(extra_8080bw_vh);
	MACHINE_START_CALL_MEMBER(mw8080bw);
}

/*******************************************************/
/*                                                     */
/* Space Invaders CV Version (Taito)                   */
/*                                                     */
/*******************************************************/

static INPUT_PORTS_START( sicv )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x70, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, mw8080bw_state,invaders_in1_control_r, NULL)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_TILT )          PORT_DIPLOCATION("SW1:3")
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "1500" )
	/* SW1:5,6,7: In OFF, PL2 can have no control of joystick, going auto left/right and other problems like no laser gun.
	Be sure these are always ON */
	PORT_BIT( 0x70, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, mw8080bw_state,invaders_in2_control_r, NULL) PORT_DIPLOCATION("SW1:5,6,7") /* Labeled as "FACTORY" */
	PORT_DIPNAME( 0x80, 0x00, "Coin Info" )             PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	/* Dummy controls port, P1 */
	INVADERS_CONTROL_PORT_P1

	/* Dummy controls port, P2 */
	INVADERS_CONTROL_PORT_P2

	/* Dummy port for cocktail mode */
	INVADERS_CAB_TYPE_PORT
INPUT_PORTS_END



/*******************************************************/
/*                                                     */
/* Space Invaders TV Version (Taito)                   */
/*                                                     */
/*******************************************************/

/* same as the CV version with a test mode switch */
static INPUT_PORTS_START( sitv )
	PORT_INCLUDE( sicv )

	PORT_MODIFY("IN0")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )
INPUT_PORTS_END



/*******************************************************/
/*                                                     */
/* Alien Invasion                                      */
/*                                                     */
/*******************************************************/

static INPUT_PORTS_START( alieninv )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x70, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, mw8080bw_state,invaders_in1_control_r, NULL)
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPNAME( 0x02, 0x02, "Pence Coinage" )     PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 )     PORT_DIPLOCATION("SW1:3") /* Pence Coin */
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW1:4" )   /* Not bonus */
	PORT_BIT( 0x70, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, mw8080bw_state,invaders_in2_control_r, NULL) PORT_DIPLOCATION("SW1:5,6,7")
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coinage ) )                  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "2C/1C 50p/3C (+ Bonus Life)" )
	PORT_DIPSETTING(    0x80, "1C/1C 50p/5C" )

	/* Dummy controls port, P1 */
	INVADERS_CONTROL_PORT_P1

	/* Dummy controls port, P2 */
	INVADERS_CONTROL_PORT_P2

	/* Dummy port for cocktail mode */
	INVADERS_CAB_TYPE_PORT
INPUT_PORTS_END



/*******************************************************/
/*                                                     */
/* Space Invaders Model Racing                         */
/*                                                     */
/*******************************************************/

static INPUT_PORTS_START( invadrmr )
	PORT_INCLUDE( sicv )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "1500" )
	PORT_DIPSETTING(    0x08, "3000" ) /* This is different to invaders */
INPUT_PORTS_END



/*******************************************************/
/*                                                     */
/* Midway "Space Invaders Part II"                     */
/*                                                     */
/*******************************************************/

static ADDRESS_MAP_START( invadpt2_io_map, AS_IO, 8, _8080bw_state )
	AM_RANGE(0x00, 0x00) AM_READ_PORT("IN0")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN2") AM_DEVWRITE("mb14241", mb14241_device, shift_count_w)
	AM_RANGE(0x03, 0x03) AM_DEVREAD("mb14241", mb14241_device, shift_result_r) AM_WRITE(invadpt2_sh_port_1_w)
	AM_RANGE(0x04, 0x04) AM_DEVWRITE("mb14241", mb14241_device, shift_data_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(invadpt2_sh_port_2_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(watchdog_reset_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( invadpt2 )
	PORT_INCLUDE( sicv )

	PORT_MODIFY("IN0")
	// Name Reset - if name of high scorer was rude, owner can press this button
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Name Reset") PORT_CODE(KEYCODE_F1)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	/* SW1:2 doesn't seem to work? */
	PORT_DIPNAME( 0x02, 0x00, "Rotate Images" ) PORT_DIPLOCATION("SW1:2") /* "When ON, the images on screen will be rotated. Default is ON." */
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Preset Mode" ) PORT_DIPLOCATION("SW1:4") /* Preset Mode: "Switch for checking, when OFF checking can be done." */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
INPUT_PORTS_END


/* same as regular invaders, but with a color board added */
static MACHINE_CONFIG_DERIVED_CLASS( invadpt2, mw8080bw_root, _8080bw_state )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(invadpt2_io_map)
	MCFG_MACHINE_START_OVERRIDE(_8080bw_state,extra_8080bw)

	/* 60 Hz signal clocks two LS161. Ripple carry will */
	/* reset circuit, if LS161 not cleared before.      */
	MCFG_WATCHDOG_VBLANK_INIT(255)

	/* add shifter */
	MCFG_MB14241_ADD("mb14241")

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(_8080bw_state, screen_update_invadpt2)

	MCFG_PALETTE_ADD_3BIT_RBG("palette")

	/* sound hardware */
	MCFG_FRAGMENT_ADD(invaders_samples_audio)
MACHINE_CONFIG_END



/*******************************************************/
/*                                                     */
/* Space Ranger                                        */
/*                                                     */
/*******************************************************/

static ADDRESS_MAP_START( spacerng_io_map, AS_IO, 8, _8080bw_state )
	AM_RANGE(0x00, 0x00) AM_READ_PORT("IN0")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN2") AM_DEVWRITE("mb14241", mb14241_device, shift_count_w)
	AM_RANGE(0x03, 0x03) AM_DEVREAD("mb14241", mb14241_device, shift_result_r) AM_WRITE(invadpt2_sh_port_1_w)
	AM_RANGE(0x04, 0x04) AM_DEVWRITE("mb14241", mb14241_device, shift_data_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(spacerng_sh_port_2_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(watchdog_reset_w)
ADDRESS_MAP_END

static MACHINE_CONFIG_DERIVED_CLASS( spacerng, invadpt2, _8080bw_state )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(spacerng_io_map)
MACHINE_CONFIG_END



/*******************************************************/
/*                                                     */
/* Space Wars (Sanritsu)                               */
/*                                                     */
/*******************************************************/

static ADDRESS_MAP_START( spcewars_io_map, AS_IO, 8, _8080bw_state )
	AM_RANGE(0x00, 0x00) AM_READ_PORT("IN0")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN2") AM_DEVWRITE("mb14241", mb14241_device, shift_count_w)
	AM_RANGE(0x03, 0x03) AM_DEVREAD("mb14241", mb14241_device, shift_result_r) AM_WRITE(spcewars_sh_port_w)
	AM_RANGE(0x04, 0x04) AM_DEVWRITE("mb14241", mb14241_device, shift_data_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(invadpt2_sh_port_2_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( spcewars )
	PORT_INCLUDE( sicv )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "SW1:2" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "1500" )
	PORT_DIPSETTING(    0x08, "2000" )
INPUT_PORTS_END

static MACHINE_CONFIG_DERIVED_CLASS( spcewars, mw8080bw_root, _8080bw_state )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(spcewars_io_map)
	MCFG_MACHINE_START_OVERRIDE(_8080bw_state,extra_8080bw)

	/* add shifter */
	MCFG_MB14241_ADD("mb14241")

	/* sound hardware */
	MCFG_FRAGMENT_ADD(invaders_samples_audio)

	/* extra audio channel */
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(_8080bw_state, screen_update_invaders)
MACHINE_CONFIG_END



/*******************************************************/
/*                                                     */
/* Astro Sidam?                                        */
/*                                                     */
/*******************************************************/

static ADDRESS_MAP_START( astropal_io_map, AS_IO, 8, _8080bw_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7)
	AM_RANGE(0x00, 0x00) AM_MIRROR(0x04) AM_READ_PORT("IN0")
	AM_RANGE(0x01, 0x01) AM_MIRROR(0x04) AM_READ_PORT("IN1")
	AM_RANGE(0x03, 0x03) AM_MIRROR(0x04) AM_READ_PORT("IN3")

	AM_RANGE(0x03, 0x03) AM_WRITE(invaders_audio_1_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(invaders_audio_2_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(watchdog_reset_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( astropal )
	PORT_START("IN0")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW,  IPT_UNUSED )        /* never read */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	/* PORT_START("IN2") - never read */

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 )

	/* Dummy port for cocktail mode (not used) */
	PORT_START(INVADERS_CAB_TYPE_PORT_TAG)
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static MACHINE_CONFIG_DERIVED_CLASS( astropal, invaders, _8080bw_state )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(astropal_io_map)
MACHINE_CONFIG_END



/*******************************************************/
/*                                                     */
/* Cosmo                                               */
/*                                                     */
/*******************************************************/

static ADDRESS_MAP_START( cosmo_map, AS_PROGRAM, 8, _8080bw_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x3fff) AM_RAM AM_SHARE("main_ram")
	AM_RANGE(0x4000, 0x57ff) AM_ROM
	AM_RANGE(0x5c00, 0x5fff) AM_RAM AM_SHARE("colorram")
ADDRESS_MAP_END

/* at least one of these MWA8_NOPs must be sound related */
static ADDRESS_MAP_START( cosmo_io_map, AS_IO, 8, _8080bw_state )
	AM_RANGE(0x00, 0x00) AM_READ_PORT("IN0") AM_WRITENOP
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1") AM_WRITENOP
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN2") AM_WRITENOP
	AM_RANGE(0x03, 0x03) AM_WRITE(invadpt2_sh_port_1_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(cosmo_sh_port_2_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x07, 0x07) AM_WRITENOP
ADDRESS_MAP_END


static INPUT_PORTS_START( cosmo )
	PORT_INCLUDE( sicv )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW1:8" ) /* must be HIGH normally or the joystick won't work */
INPUT_PORTS_END

static MACHINE_CONFIG_DERIVED_CLASS( cosmo, mw8080bw_root, _8080bw_state )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(cosmo_map)
	MCFG_CPU_IO_MAP(cosmo_io_map)
	MCFG_MACHINE_START_OVERRIDE(_8080bw_state,extra_8080bw)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(_8080bw_state, screen_update_cosmo)

	MCFG_PALETTE_ADD_3BIT_RGB("palette")

	/* sound hardware */
	MCFG_FRAGMENT_ADD(invaders_samples_audio)
MACHINE_CONFIG_END



/*******************************************************/
/*                                                     */
/* bootleg "Super Earth Invasion"                      */
/*                                                     */
/*******************************************************/

static INPUT_PORTS_START( searthin )
	PORT_INCLUDE( sicv )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPNAME( 0x02, 0x02, "Pence Coinage" )     PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 )     PORT_DIPLOCATION("SW1:3") /* Pence Coin */
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW1:4" )   /* Not bonus */
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "2C/1C 50p/3C (+ Bonus Life)" )
	PORT_DIPSETTING(    0x80, "1C/1C 50p/5C" )
INPUT_PORTS_END



/*******************************************************/
/*                                                     */
/* bootleg "Super Invaders (Zenitone-Microsec)"        */
/*                                                     */
/*******************************************************/

static INPUT_PORTS_START( sinvzen )
	PORT_INCLUDE( sicv )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
INPUT_PORTS_END

/*******************************************************/
/*                                                     */
/* bootleg "Space Attack II"                           */
/*                                                     */
/*******************************************************/

static INPUT_PORTS_START( spaceat2 )
	PORT_INCLUDE( sicv )

	PORT_MODIFY("IN2")
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x00, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x00, "SW1:7" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )

	/* Dummy port for cocktail mode (not used) */
	PORT_MODIFY(INVADERS_CAB_TYPE_PORT_TAG)
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END



/*******************************************************/
/*                                                     */
/* bootleg "Space Combat", 1979                        */
/*  8080A + 18MHz XTAL, SN76477, 10MHz XTAL            */
/*  8*8116 2KB RAM(!), 8*1KB ROM, maybe some PROMs     */
/*                                                     */
/*  Preliminary emulation. PCB was working fine, but   */
/*  it's not certain that this is a good dump          */
/*                                                     */
/* TODO:                                               */
/*  - dip settings/locs need confirming                */
/*  - it doesn't have a mb14241 video shifter?         */
/*  - using space invaders audio as placeholder until  */
/*    more is known about the sound hw                 */
/*  - always in cocktail mode but flipscreen not found */
/*                                                     */
/*******************************************************/

static INPUT_PORTS_START( spacecom )
	PORT_START("IN0") // 5-pos dipsw at ic79 (row F)
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "2500" ) // not confirmed
	PORT_DIPSETTING(    0x08, "1500" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_SERVICE( 0x08, IP_ACTIVE_HIGH )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // unused?

	PORT_START(CABINET_PORT_TAG)        /* Dummy port for cocktail mode */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static ADDRESS_MAP_START( spacecom_map, AS_PROGRAM, 8, _8080bw_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x3fff) AM_RAM AM_SHARE("main_ram") // other RAM not hooked up?
ADDRESS_MAP_END

static ADDRESS_MAP_START( spacecom_io_map, AS_IO, 8, _8080bw_state )
	AM_RANGE(0x41, 0x41) AM_READ_PORT("IN0")
	AM_RANGE(0x42, 0x42) AM_READ_PORT("IN1") AM_WRITE(invaders_audio_1_w)
	AM_RANGE(0x44, 0x44) AM_READ_PORT("IN2") AM_WRITE(invaders_audio_2_w)
ADDRESS_MAP_END

MACHINE_CONFIG_START( spacecom, _8080bw_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8080A, XTAL_18MHz / 10) // divider guessed
	// TODO: move irq handling away from mw8080.c, this game runs on custom hardware
	MCFG_CPU_PROGRAM_MAP(spacecom_map)
	MCFG_CPU_IO_MAP(spacecom_io_map)

	MCFG_MACHINE_START_OVERRIDE(mw8080bw_state, mw8080bw)
	MCFG_MACHINE_RESET_OVERRIDE(mw8080bw_state, mw8080bw)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(_8080bw_state, screen_update_spacecom)

	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	/* sound hardware */
	MCFG_FRAGMENT_ADD(invaders_audio)
MACHINE_CONFIG_END

DRIVER_INIT_MEMBER(_8080bw_state, spacecom)
{
	UINT8 *ROM = memregion("maincpu")->base();

	// bad byte: should be push a at RST 10h
	ROM[0x10] = 0xf5;
}



/*******************************************************/
/*                                                     */
/* Zenitone Microsec "Invaders Revenge"                */
/*                                                     */
/*******************************************************/

READ8_MEMBER(_8080bw_state::invrvnge_02_r)
{
	UINT8 data = ioport("IN2")->read();
	if (m_flip_screen) return data;
	return (data & 0x8f) | (ioport("IN1")->read() & 0x70);
}

static ADDRESS_MAP_START( invrvnge_io_map, AS_IO, 8, _8080bw_state )
	AM_RANGE(0x00, 0x00) AM_READ_PORT("IN0")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_READ(invrvnge_02_r) AM_DEVWRITE("mb14241", mb14241_device, shift_count_w)
	AM_RANGE(0x03, 0x03) AM_DEVREAD("mb14241", mb14241_device, shift_result_r) AM_WRITE(invrvnge_sh_port_1_w)
	AM_RANGE(0x04, 0x04) AM_DEVWRITE("mb14241", mb14241_device, shift_data_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(invrvnge_sh_port_2_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(watchdog_reset_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( invrvnge_sound_map, AS_PROGRAM, 8, _8080bw_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xffff) AM_ROM // dummy prg map, TODO: decrypt ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( invrvnge )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Harder ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Hardest ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // must be low or game won't boot [code: 0x1a9-1af]
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x80, 0x80, "Fuel Destroyed by Comet" )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x80, "6" )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_2WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "1500" )
	PORT_DIPSETTING(    0x08, "2000" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) ) // 1 play 10p, 2 play 20p, 6 play 50p
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) ) // 1 play 20p, 2 play 40p, 3 play 50p

	/* Dummy port for cocktail mode */
	INVADERS_CAB_TYPE_PORT
INPUT_PORTS_END


static MACHINE_CONFIG_DERIVED_CLASS( invrvnge, mw8080bw_root, _8080bw_state )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(invrvnge_io_map)

	MCFG_CPU_ADD("audiocpu", M6808, XTAL_4MHz/2) // MC6808P
	MCFG_CPU_PROGRAM_MAP(invrvnge_sound_map)

	/* add shifter */
	MCFG_MB14241_ADD("mb14241")

	MCFG_MACHINE_START_OVERRIDE(_8080bw_state,extra_8080bw)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(_8080bw_state, screen_update_invadpt2)

	MCFG_PALETTE_ADD_3BIT_RBG("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, XTAL_4MHz/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END



/*******************************************************/
/*                                                     */
/* Taito "Space Laser"                                 */
/*                                                     */
/*******************************************************/

static INPUT_PORTS_START( spclaser )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) /* This is not 2 Player ??? */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	// TODO: figure out where dipswitch is read, it's not IN0 or IN2 in the current implementation.
	// ROM disassembly doesn't show any dipswitch reads on portmapped I/O, maybe the manual is for a different ROM set? (that we don't have the dump for)
#if 0
	// these are the settings according to Gameplan Intruder manual
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "1500" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_DIPNAME( 0x80, 0x80, "Display Coinage" )           PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
#endif

	PORT_START("IN2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "SW1:4" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_DIPNAME( 0x80, 0x00, DEF_STR(Coinage) )        PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "1 Coin/1 Or 2 Players" )
	PORT_DIPSETTING(    0x80, "1 Coin/1 Player  2 Coins/2 Players" )   /* Irrelevant, causes bugs */

	/* Dummy port for cocktail mode (not used) */
	PORT_START(CABINET_PORT_TAG)
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END



/*******************************************************/
/*                                                     */
/* Taito "Galaxy Wars"                                 */
/*                                                     */
/*******************************************************/

static INPUT_PORTS_START( galxwars )
	PORT_INCLUDE( sicv )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN ) /* must be IP_ACTIVE_LOW for Universal Sets */

	PORT_MODIFY("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW1:3" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "3000" )
	PORT_DIPSETTING(    0x08, "5000" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
INPUT_PORTS_END

static ADDRESS_MAP_START( starw1_io_map, AS_IO, 8, _8080bw_state )
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN2")
	AM_RANGE(0x03, 0x03) AM_WRITENOP    /* writes 9B at boot */
	AM_RANGE(0x04, 0x04) AM_WRITE(invadpt2_sh_port_1_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(invadpt2_sh_port_2_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x07, 0x07) AM_WRITENOP    /* writes 89 at boot */
ADDRESS_MAP_END

static MACHINE_CONFIG_DERIVED_CLASS( starw1, invadpt2, _8080bw_state )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(starw1_io_map)
MACHINE_CONFIG_END

/*******************************************************/
/*                                                     */
/* Taito "Lunar Rescue"                                */
/*                                                     */
/*******************************************************/

static ADDRESS_MAP_START( lrescue_io_map, AS_IO, 8, _8080bw_state )
	AM_RANGE(0x00, 0x00) AM_READ_PORT("IN0")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN2") AM_DEVWRITE("mb14241", mb14241_device, shift_count_w)
	AM_RANGE(0x03, 0x03) AM_DEVREAD("mb14241", mb14241_device, shift_result_r) AM_WRITE(lrescue_sh_port_1_w)
	AM_RANGE(0x04, 0x04) AM_DEVWRITE("mb14241", mb14241_device, shift_data_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(lrescue_sh_port_2_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( lrescue )
	PORT_INCLUDE( sicv )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_MODIFY("IN2")
	/* SW1:3-8 Unused according to manual: "Factory Adjustments". Default is ON. */
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x00, "SW1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "SW1:8" )
INPUT_PORTS_END

static MACHINE_CONFIG_DERIVED_CLASS( lrescue, mw8080bw_root, _8080bw_state )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(lrescue_io_map)
	MCFG_MACHINE_START_OVERRIDE(_8080bw_state,extra_8080bw)

	/* add shifter */
	MCFG_MB14241_ADD("mb14241")

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(_8080bw_state, screen_update_invadpt2)

	MCFG_PALETTE_ADD_3BIT_RBG("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(4)
	MCFG_SAMPLES_NAMES(lrescue_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)

	/* extra audio channel */
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/*******************************************************/
/*                                                     */
/* Universal "Cosmic Monsters"                         */
/*  The dipswitches are as stated in the manual, but   */
/*  some of them are incorrect.                        */
/*  - You need at the very least 3000 points to get    */
/*    a bonus life.                                    */
/*  - The cabinet switch does nothing in the cpu, it   */
/*    is all done by wires.                            */
/*                                                     */
/*  These issues may be due to manual/romset conflicts */
/*                                                     */
/*******************************************************/

static INPUT_PORTS_START( cosmicmo )
	PORT_INCLUDE( sicv )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "1500" )
	PORT_DIPSETTING(    0x08, "2500" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )

	PORT_MODIFY(CABINET_PORT_TAG)
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

WRITE8_MEMBER(_8080bw_state::cosmicmo_05_w)
{
	invaders_audio_2_w(space, offset, data);
	m_flip_screen = BIT(data, 5) & BIT(ioport("IN2")->read(), 2);
}

static ADDRESS_MAP_START( cosmicmo_io_map, AS_IO, 8, _8080bw_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7)
	AM_RANGE(0x00, 0x00) AM_MIRROR(0x04) AM_READ_PORT("IN0")
	AM_RANGE(0x01, 0x01) AM_MIRROR(0x04) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_MIRROR(0x04) AM_READ(invrvnge_02_r)
	AM_RANGE(0x03, 0x03) AM_MIRROR(0x04) AM_DEVREAD("mb14241", mb14241_device, shift_result_r)

	AM_RANGE(0x02, 0x02) AM_DEVWRITE("mb14241", mb14241_device, shift_count_w)
	AM_RANGE(0x03, 0x03) AM_WRITE(invaders_audio_1_w)
	AM_RANGE(0x04, 0x04) AM_DEVWRITE("mb14241", mb14241_device, shift_data_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(cosmicmo_05_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(watchdog_reset_w)
ADDRESS_MAP_END

static MACHINE_CONFIG_DERIVED_CLASS( cosmicmo, mw8080bw_root, _8080bw_state )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(cosmicmo_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", _8080bw_state,  irq0_line_hold)

	MCFG_MACHINE_START_OVERRIDE(_8080bw_state,extra_8080bw)

	MCFG_WATCHDOG_VBLANK_INIT(255)

	/* add shifter */
	MCFG_MB14241_ADD("mb14241")

	/* sound hardware */
	MCFG_FRAGMENT_ADD(invaders_audio)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(_8080bw_state, screen_update_invaders)
MACHINE_CONFIG_END


/*******************************************************/
/*                                                     */
/* Sidam "Invasion"                                    */
/*                                                     */
/*******************************************************/

static INPUT_PORTS_START( invasion )
	PORT_INCLUDE( sicv )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN2 )

	PORT_MODIFY("IN2")
	/* SW1:5-8 Unused according to manual. */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:4,3") /* Default is OFF, OFF (6) */
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW1:8" )        /* Default OFF */
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:2") /* Default is ON (2500) */
	PORT_DIPSETTING(    0x08, "1500" )
	PORT_DIPSETTING(    0x00, "2500" )
	/* SW1:5,6,7 Default ON */
	PORT_DIPNAME( 0x80, 0x80, "Laser Bonus Info" )      PORT_DIPLOCATION("SW1:1") /* Default is OFF (Info on) */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END



/*******************************************************/
/*                                                     */
/* bootleg "Super Invaders"                            */
/*                                                     */
/*******************************************************/

static INPUT_PORTS_START( superinv )
	PORT_INCLUDE( sicv )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN2 )

	PORT_MODIFY("IN2")
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW1:3" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "1500" )
	PORT_DIPSETTING(    0x00, "2500" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x00, "SW1:8" )
INPUT_PORTS_END



/*******************************************************/
/*                                                     */
/* Nichibutsu "Rolling Crash"                          */
/* Press left-arrow to play "Moon Base"                */
/*                                                     */
/*******************************************************/

static ADDRESS_MAP_START( rollingc_map, AS_PROGRAM, 8, _8080bw_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x3fff) AM_RAM AM_SHARE("main_ram")
	AM_RANGE(0x4000, 0x5fff) AM_ROM
	AM_RANGE(0xa000, 0xbfff) AM_MIRROR(0x00e0) AM_RAM AM_SHARE("colorram")
	AM_RANGE(0xe000, 0xffff) AM_MIRROR(0x00e0) AM_RAM AM_SHARE("colorram2")
ADDRESS_MAP_END


static ADDRESS_MAP_START( rollingc_io_map, AS_IO, 8, _8080bw_state )
	AM_RANGE(0x00, 0x00) AM_READ_PORT("IN0") AM_WRITE(rollingc_sh_port_w)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN2") AM_DEVWRITE("mb14241", mb14241_device, shift_count_w)
	AM_RANGE(0x03, 0x03) AM_DEVREAD("mb14241", mb14241_device, shift_result_r) AM_WRITE(invadpt2_sh_port_1_w)
	AM_RANGE(0x04, 0x04) AM_DEVWRITE("mb14241", mb14241_device, shift_data_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(invadpt2_sh_port_2_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( rollingc )
	PORT_INCLUDE( sicv )

	PORT_MODIFY("IN0")
	/* Duplicate control required (both must be mapped to same key for input to work for game select and in-game) */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) /* Game Select */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) /* Game Select */

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_MODIFY("IN2")
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "SW1:4" )
INPUT_PORTS_END

static MACHINE_CONFIG_DERIVED_CLASS( rollingc, mw8080bw_root, _8080bw_state )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(rollingc_map)
	MCFG_CPU_IO_MAP(rollingc_io_map)

	/* add shifter */
	MCFG_MB14241_ADD("mb14241")

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(_8080bw_state, screen_update_rollingc)

	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_INIT_OWNER(_8080bw_state, rollingc)

	/* sound hardware */
	MCFG_FRAGMENT_ADD(invaders_samples_audio)
MACHINE_CONFIG_END



/*******************************************************/
/*                                                     */
/* Taito "Space Chaser"                                */
/*                                                     */
/*******************************************************/

static ADDRESS_MAP_START( schaser_map, AS_PROGRAM, 8, _8080bw_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x3fff) AM_RAM AM_SHARE("main_ram")
	AM_RANGE(0x4000, 0x5fff) AM_ROM
	AM_RANGE(0xc000, 0xdfff) AM_MIRROR(0x0060) AM_RAM AM_SHARE("colorram")
ADDRESS_MAP_END


static ADDRESS_MAP_START( schaser_io_map, AS_IO, 8, _8080bw_state )
	AM_RANGE(0x00, 0x00) AM_READ_PORT("IN0")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN2") AM_DEVWRITE("mb14241", mb14241_device, shift_count_w)
	AM_RANGE(0x03, 0x03) AM_DEVREAD("mb14241", mb14241_device, shift_result_r) AM_WRITE(schaser_sh_port_1_w)
	AM_RANGE(0x04, 0x04) AM_DEVWRITE("mb14241", mb14241_device, shift_data_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(schaser_sh_port_2_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(watchdog_reset_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( schaser )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_DIPNAME( 0x60, 0x00, "Hard Starting Level" )   PORT_DIPLOCATION("SW1:5,7")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x60, "6" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN2")   // port 2
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW1:3" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	// Name Reset - if name of high scorer was rude, owner can press this button
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Name Reset") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("VR1")
	PORT_ADJUSTER( 70, "VR1 - Music Volume" )

	PORT_START("VR2")
	PORT_ADJUSTER( 90, "VR2 - Explosion/Effect Volume" )

	PORT_START("VR3")
	PORT_ADJUSTER( 70, "VR3 - Dot Volume" )
INPUT_PORTS_END


static INPUT_PORTS_START( schaserm )
	PORT_INCLUDE( schaser )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "4" )
INPUT_PORTS_END

MACHINE_START_MEMBER(_8080bw_state,schaser)
{
	MACHINE_START_CALL_MEMBER(schaser_sh);
	MACHINE_START_CALL_MEMBER(extra_8080bw_vh);
	MACHINE_START_CALL_MEMBER(mw8080bw);
}

MACHINE_RESET_MEMBER(_8080bw_state,schaser)
{
	MACHINE_RESET_CALL_MEMBER(schaser_sh);
	MACHINE_RESET_CALL_MEMBER(mw8080bw);
}

static MACHINE_CONFIG_DERIVED_CLASS( schaser, mw8080bw_root, _8080bw_state )

	/* basic machine hardware */
	MCFG_CPU_REPLACE("maincpu",I8080,1996800)       /* 19.968MHz / 10 */
	MCFG_CPU_PROGRAM_MAP(schaser_map)
	MCFG_CPU_IO_MAP(schaser_io_map)
	MCFG_WATCHDOG_VBLANK_INIT(255)
	MCFG_MACHINE_START_OVERRIDE(_8080bw_state,schaser)
	MCFG_MACHINE_RESET_OVERRIDE(_8080bw_state,schaser)

	MCFG_TIMER_DRIVER_ADD("schaser_sh_555", _8080bw_state, schaser_effect_555_cb)

	/* add shifter */
	MCFG_MB14241_ADD("mb14241")

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(_8080bw_state, screen_update_schaser)

	MCFG_PALETTE_ADD_3BIT_RBG("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("snsnd", SN76477, 0)
	MCFG_SN76477_NOISE_PARAMS(RES_K(47), RES_K(330), CAP_P(470)) // noise + filter
	MCFG_SN76477_DECAY_RES(RES_M(2.2))                   // decay_res
	MCFG_SN76477_ATTACK_PARAMS(CAP_U(1.0), RES_K(4.7))   // attack_decay_cap + attack_res
	MCFG_SN76477_AMP_RES(0)                              // amplitude_res
	MCFG_SN76477_FEEDBACK_RES(RES_K(33))                 // feedback_res
	MCFG_SN76477_VCO_PARAMS(0, CAP_U(0.1), RES_K(39))    // VCO volt + cap + res
	MCFG_SN76477_PITCH_VOLTAGE(5.0)                      // pitch_voltage
	MCFG_SN76477_SLF_PARAMS(CAP_U(1.0), RES_K(120))      // slf caps + res
	MCFG_SN76477_ONESHOT_PARAMS(CAP_U(0.1), RES_K(220))  // oneshot caps + res
	MCFG_SN76477_VCO_MODE(1)                             // VCO mode
	MCFG_SN76477_MIXER_PARAMS(0, 0, 0)                   // mixer A, B, C
	MCFG_SN76477_ENVELOPE_PARAMS(1, 0)                   // envelope 1, 2
	MCFG_SN76477_ENABLE(1)                               // enable
	MCFG_SOUND_ROUTE_EX(0, "discrete", 1.0, 0)

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(schaser)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



/*******************************************************/
/*                                                     */
/* Taito "Space Chaser" (CV version)                   */
/*                                                     */
/*******************************************************/


READ8_MEMBER(_8080bw_state::schasercv_02_r)
{
	UINT8 data = ioport("IN2")->read();
	if (m_flip_screen) return data;
	UINT8 in1 = ioport("IN1")->read();
	return (data & 0x89) | (in1 & 0x70) | (BIT(in1, 3) << 1) | (BIT(in1, 7) << 2);
}

static ADDRESS_MAP_START( schasercv_io_map, AS_IO, 8, _8080bw_state )
	AM_RANGE(0x00, 0x00) AM_READ_PORT("IN0")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_READ(schasercv_02_r) AM_DEVWRITE("mb14241", mb14241_device, shift_count_w)
	AM_RANGE(0x03, 0x03) AM_DEVREAD("mb14241", mb14241_device, shift_result_r) AM_WRITE(schasercv_sh_port_1_w)
	AM_RANGE(0x04, 0x04) AM_DEVWRITE("mb14241", mb14241_device, shift_data_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(schasercv_sh_port_2_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( schasercv )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Lives ) )                                    PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)    PORT_DIPLOCATION("SW1:2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)      PORT_DIPLOCATION("SW1:3")
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Difficulty ) )                               PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)                    PORT_DIPLOCATION("SW1:5")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)    PORT_DIPLOCATION("SW1:6")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)   PORT_DIPLOCATION("SW1:7")
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x00, "SW1:8" )

	/* Dummy port for cocktail mode */
	INVADERS_CAB_TYPE_PORT
INPUT_PORTS_END

static MACHINE_CONFIG_DERIVED_CLASS( schasercv, mw8080bw_root, _8080bw_state )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(schaser_map)
	MCFG_CPU_IO_MAP(schasercv_io_map)
	MCFG_MACHINE_START_OVERRIDE(_8080bw_state,extra_8080bw)

	/* add shifter */
	MCFG_MB14241_ADD("mb14241")

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(_8080bw_state, screen_update_schasercv)

	MCFG_PALETTE_ADD_3BIT_RBG("palette")

	/* sound hardware */
	MCFG_FRAGMENT_ADD(invaders_samples_audio)

	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/*******************************************************/
/*                                                     */
/* Taito "Straight Flush"                              */
/*                                                     */
/*******************************************************/

CUSTOM_INPUT_MEMBER(_8080bw_state::sflush_80_r)
{
	return (m_screen->vpos() & 0x80) ? 1 : 0;
}

static ADDRESS_MAP_START( sflush_map, AS_PROGRAM, 8, _8080bw_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x4000, 0x5fff) AM_RAM AM_SHARE("main_ram")
	AM_RANGE(0x8008, 0x8008) AM_READ_PORT("PADDLE")
	AM_RANGE(0x8009, 0x8009) AM_DEVREAD("mb14241", mb14241_device, shift_result_r)
	AM_RANGE(0x800a, 0x800a) AM_READ_PORT("IN2")
	AM_RANGE(0x800b, 0x800b) AM_READ_PORT("IN0")
	AM_RANGE(0x8018, 0x8018) AM_DEVWRITE("mb14241", mb14241_device, shift_data_w)
	AM_RANGE(0x8019, 0x8019) AM_DEVWRITE("mb14241", mb14241_device, shift_count_w)
	AM_RANGE(0x801a, 0x801a) AM_WRITENOP
	AM_RANGE(0x801c, 0x801c) AM_WRITENOP
	AM_RANGE(0x801d, 0x801d) AM_WRITENOP
	AM_RANGE(0xa000, 0xbfff) AM_MIRROR(0x0060) AM_RAM AM_SHARE("colorram")
	AM_RANGE(0xd800, 0xffff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( sflush )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x08, 0x00, "Hiscore" )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x08, "30 000" )
	PORT_DIPNAME( 0x40, 0x00, "Coinage Display" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, _8080bw_state,sflush_80_r, NULL) // 128V?

	PORT_START("PADDLE")
	PORT_BIT( 0xff, 0x6a, IPT_PADDLE ) PORT_MINMAX(0x16,0xbf) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_CENTERDELTA(0)
INPUT_PORTS_END


static MACHINE_CONFIG_DERIVED_CLASS( sflush, mw8080bw_root, _8080bw_state )

	/* basic machine hardware */
	MCFG_CPU_REPLACE("maincpu",M6800,1500000) // ?
	MCFG_CPU_PROGRAM_MAP(sflush_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", _8080bw_state, irq0_line_hold)
	MCFG_MACHINE_START_OVERRIDE(_8080bw_state,mw8080bw)

	/* add shifter */
	MCFG_MB14241_ADD("mb14241")

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(_8080bw_state, screen_update_sflush)

	MCFG_PALETTE_ADD("palette", 8)
	MCFG_PALETTE_INIT_OWNER(_8080bw_state, sflush)
MACHINE_CONFIG_END



/*******************************************************************************************/
/*                                                                                         */
/* Taito "Lupin III"                                                                       */
/*                                                                                         */
/*  The rom at 5000 is optional. It contains code for colour ram, and all tunes except     */
/*  when the moneybag has been stolen. If you remove this rom, bits 0 and 1 of port 0      */
/*  must be set High. The colour will then be determined by the 2 proms instead. Bit 6     */
/*  of port 5 will select which prom to use.                                               */
/*                                                                                         */
/*  Differences between the 2 sets:                                                        */
/*  - Set 2 has a language selection switch. In the Japanese position, it looks            */
/*    the same as set 1. Set 1 is always in Japanese.                                      */
/*  - Set 1, bit 6 of port 3 activates when the wife is kicking the man.                   */
/*  - The dogs and policemen are yellow in set 1, and different colours in set 2.          */
/*                                                                                         */
/*******************************************************************************************/

static ADDRESS_MAP_START( lupin3_io_map, AS_IO, 8, _8080bw_state )
	AM_RANGE(0x00, 0x00) AM_READ_PORT("IN0") AM_WRITE(lupin3_00_w)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN2") AM_DEVWRITE("mb14241", mb14241_device, shift_count_w)
	AM_RANGE(0x03, 0x03) AM_DEVREAD("mb14241", mb14241_device, shift_result_r) AM_WRITE(lupin3_sh_port_1_w)
	AM_RANGE(0x04, 0x04) AM_DEVWRITE("mb14241", mb14241_device, shift_data_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(lupin3_sh_port_2_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(watchdog_reset_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( lupin3 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x00, "Bags To Collect" )       PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x00, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW1:7" )
	PORT_DIPNAME(0x80,  0x00, "Invulnerability (Cheat)") PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( lupin3a )
	PORT_INCLUDE( lupin3 )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* selects color mode (dynamic vs. static) */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* something has to do with sound */

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Language ) )     PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Japanese ) )
INPUT_PORTS_END

static MACHINE_CONFIG_DERIVED_CLASS( lupin3, mw8080bw_root, _8080bw_state )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(lupin3_io_map)
	MCFG_MACHINE_START_OVERRIDE(_8080bw_state,extra_8080bw)

	/* add shifter */
	MCFG_MB14241_ADD("mb14241")

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(_8080bw_state, screen_update_indianbt)

	MCFG_PALETTE_ADD_3BIT_RGB("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("snsnd", SN76477, 0)
	MCFG_SN76477_NOISE_PARAMS(0, 0, 0)                  // noise + filter: N/C
	MCFG_SN76477_DECAY_RES(0)                           // decay_res: N/C
	MCFG_SN76477_ATTACK_PARAMS(0, RES_K(100))           // attack_decay_cap + attack_res
	MCFG_SN76477_AMP_RES(RES_K(56))                     // amplitude_res
	MCFG_SN76477_FEEDBACK_RES(RES_K(10))                // feedback_res
	MCFG_SN76477_VCO_PARAMS(0, CAP_U(0.1), RES_K(8.2))  // VCO volt + cap + res
	MCFG_SN76477_PITCH_VOLTAGE(5.0)                     // pitch_voltage
	MCFG_SN76477_SLF_PARAMS(CAP_U(1.0), RES_K(120))     // slf caps + res
	MCFG_SN76477_ONESHOT_PARAMS(0, 0)                   // oneshot caps + res: N/C
	MCFG_SN76477_VCO_MODE(1)                            // VCO mode
	MCFG_SN76477_MIXER_PARAMS(0, 0, 0)                  // mixer A, B, C
	MCFG_SN76477_ENVELOPE_PARAMS(1, 0)                  // envelope 1, 2
	MCFG_SN76477_ENABLE(1)                              // enable
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(4)
	MCFG_SAMPLES_NAMES(lupin3_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(indianbt)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( lupin3a, mw8080bw_root, _8080bw_state )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(schaser_map)
	MCFG_CPU_IO_MAP(lupin3_io_map)
	MCFG_MACHINE_START_OVERRIDE(_8080bw_state,extra_8080bw)

	/* add shifter */
	MCFG_MB14241_ADD("mb14241")

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(_8080bw_state, screen_update_lupin3)

	MCFG_PALETTE_ADD_3BIT_RBG("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("snsnd", SN76477, 0)
	MCFG_SN76477_NOISE_PARAMS(0, 0, 0)                  // noise + filter: N/C
	MCFG_SN76477_DECAY_RES(0)                           // decay_res: N/C
	MCFG_SN76477_ATTACK_PARAMS(0, RES_K(100))           // attack_decay_cap + attack_res
	MCFG_SN76477_AMP_RES(RES_K(56))                     // amplitude_res
	MCFG_SN76477_FEEDBACK_RES(RES_K(10))                // feedback_res
	MCFG_SN76477_VCO_PARAMS(0, CAP_U(0.1), RES_K(8.2))  // VCO volt + cap + res
	MCFG_SN76477_PITCH_VOLTAGE(5.0)                     // pitch_voltage
	MCFG_SN76477_SLF_PARAMS(CAP_U(1.0), RES_K(120))     // slf caps + res
	MCFG_SN76477_ONESHOT_PARAMS(0, 0)                   // oneshot caps + res: N/C
	MCFG_SN76477_VCO_MODE(1)                            // VCO mode
	MCFG_SN76477_MIXER_PARAMS(0, 0, 0)                  // mixer A, B, C
	MCFG_SN76477_ENVELOPE_PARAMS(1, 0)                  // envelope 1, 2
	MCFG_SN76477_ENABLE(1)                              // enable
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(4)
	MCFG_SAMPLES_NAMES(lupin3_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(indianbt)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



/*******************************************************/
/*                                                     */
/* Taito "Polaris"                                     */
/*                                                     */
/*******************************************************/

INTERRUPT_GEN_MEMBER(_8080bw_state::polaris_interrupt)
{
	m_polaris_cloud_speed++;

	if (m_polaris_cloud_speed >= 4) /* every 4 frames - this was verified against real machine */
	{
		m_polaris_cloud_speed = 0;
		m_polaris_cloud_pos++;
	}
}

MACHINE_START_MEMBER(_8080bw_state,polaris)
{
	save_item(NAME(m_polaris_cloud_speed));
	save_item(NAME(m_polaris_cloud_pos));

	MACHINE_START_CALL_MEMBER(mw8080bw);
}

READ8_MEMBER(_8080bw_state::polaris_port00_r)
{
	UINT8 data = ioport("IN0")->read();
	if (m_flip_screen) return data;
	return (data & 7) | (ioport("IN1")->read() & 0xf8);
}

// Port 5 is used to reset the watchdog timer.
// This port is also written to when the boss plane is going up and down.
// If you write this value to a note ciruit similar to the music,
// you will get a nice sound that accurately follows the plane.
// It sounds better then the actual circuit used.
// Probably an unfinished feature.
static ADDRESS_MAP_START( polaris_io_map, AS_IO, 8, _8080bw_state )
	AM_RANGE(0x00, 0x00) AM_READ(polaris_port00_r) AM_DEVWRITE("mb14241", mb14241_device, shift_count_w)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN2") AM_WRITE(polaris_sh_port_1_w)
	AM_RANGE(0x03, 0x03) AM_DEVREADWRITE("mb14241", mb14241_device, shift_result_r, shift_data_w)
	AM_RANGE(0x04, 0x04) AM_WRITE(polaris_sh_port_2_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(polaris_sh_port_3_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( polaris )
	PORT_START("IN0")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x00, "SW?:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x00, "SW?:2" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x00, "Invincible Test" )       PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	/* The Demo Sounds dip switch does function.
	   It allows the sonar sounds to play in demo mode. */
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW1:7" )
	PORT_DIPNAME( 0x80, 0x00, "High Score Preset Mode" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("VR1")
	PORT_ADJUSTER( 80, "Sub Volume VR1" )

	PORT_START("VR2")
	PORT_ADJUSTER( 70, "Sub Volume VR2" )

	PORT_START("VR3")
	PORT_ADJUSTER( 90, "Sub Volume VR3" )
INPUT_PORTS_END

static MACHINE_CONFIG_DERIVED_CLASS( polaris, mw8080bw_root, _8080bw_state )

	/* basic machine hardware */
	MCFG_CPU_REPLACE("maincpu",I8080,1996800)       /* 19.968MHz / 10 */
	MCFG_CPU_PROGRAM_MAP(schaser_map)
	MCFG_CPU_IO_MAP(polaris_io_map)
	MCFG_WATCHDOG_VBLANK_INIT(255)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", _8080bw_state,  polaris_interrupt)
	MCFG_MACHINE_START_OVERRIDE(_8080bw_state,polaris)

	/* add shifter */
	MCFG_MB14241_ADD("mb14241")

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(_8080bw_state, screen_update_polaris)

	MCFG_PALETTE_ADD_3BIT_RBG("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(polaris)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



/*******************************************************/
/*                                                     */
/* Taito "Ozma Wars"                                   */
/*                                                     */
/*******************************************************/

static INPUT_PORTS_START( ozmawars )
	PORT_INCLUDE( sicv )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x03, 0x00, "Energy" )            PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "15000" )
	PORT_DIPSETTING(    0x01, "20000" )
	PORT_DIPSETTING(    0x02, "25000" )
	PORT_DIPSETTING(    0x03, "35000" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW1:3" )
	PORT_DIPNAME( 0x08, 0x00, "Bonus Energy" )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "15000" )
	PORT_DIPSETTING(    0x08, "10000" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
INPUT_PORTS_END


static INPUT_PORTS_START( spaceph )
	PORT_INCLUDE( ozmawars )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



/*******************************************************/
/*                                                     */
/* Emag "Super Invaders"                               */
/*                                                     */
/*******************************************************/

static INPUT_PORTS_START( sinvemag )
	PORT_INCLUDE( sicv )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_MODIFY("IN2")
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW1:3" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "2000" )
	PORT_DIPSETTING(    0x00, "3000" )
INPUT_PORTS_END



/*******************************************************/
/*                                                     */
/* Jatre Specter (Taito?)                              */
/*                                                     */
/*******************************************************/

static INPUT_PORTS_START( jspecter )
	PORT_INCLUDE( sicv )

	PORT_MODIFY("IN2")
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW1:3" )
INPUT_PORTS_END



/*******************************************************/
/*                                                     */
/* Taito "Balloon Bomber"                              */
/*                                                     */
/*******************************************************/

static ADDRESS_MAP_START( ballbomb_io_map, AS_IO, 8, _8080bw_state )
	AM_RANGE(0x00, 0x00) AM_READ_PORT("IN0")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1") AM_WRITE(ballbomb_01_w)
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN2") AM_DEVWRITE("mb14241", mb14241_device, shift_count_w)
	AM_RANGE(0x03, 0x03) AM_DEVREAD("mb14241", mb14241_device, shift_result_r) AM_WRITE(ballbomb_sh_port_1_w)
	AM_RANGE(0x04, 0x04) AM_DEVWRITE("mb14241", mb14241_device, shift_data_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(ballbomb_sh_port_2_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( ballbomb )
	PORT_INCLUDE( sicv )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_MODIFY("IN2")
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x00, "SW1:8" )
INPUT_PORTS_END

static MACHINE_CONFIG_DERIVED_CLASS( ballbomb, mw8080bw_root, _8080bw_state )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(ballbomb_io_map)
	MCFG_MACHINE_START_OVERRIDE(_8080bw_state,extra_8080bw)

	/* add shifter */
	MCFG_MB14241_ADD("mb14241")

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(_8080bw_state, screen_update_ballbomb)

	MCFG_PALETTE_ADD_3BIT_RBG("palette")

	/* sound hardware */
	MCFG_FRAGMENT_ADD(invaders_samples_audio)

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(ballbomb)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



/*******************************************************/
/*                                                     */
/* Wing "Yosaku To Donbei"                             */
/*                                                     */
/*******************************************************/

static ADDRESS_MAP_START( yosakdon_map, AS_PROGRAM, 8, _8080bw_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x3fff) AM_RAM AM_SHARE("main_ram")
	AM_RANGE(0x4000, 0x43ff) AM_WRITEONLY /* what's this? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( yosakdon_io_map, AS_IO, 8, _8080bw_state )
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN0")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN1")
	AM_RANGE(0x03, 0x03) AM_WRITE(yosakdon_sh_port_1_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(yosakdon_sh_port_2_w)
	AM_RANGE(0x06, 0x06) AM_WRITENOP /* character numbers */
ADDRESS_MAP_END


static INPUT_PORTS_START( yosakdon )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x70, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, mw8080bw_state,invaders_in1_control_r, NULL)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x70, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, mw8080bw_state,invaders_in2_control_r, NULL)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	/* Dummy controls port, P1 */
	INVADERS_CONTROL_PORT_P1
	INVADERS_CONTROL_PORT_P2

	/* Dummy port for cocktail mode (not used) */
	INVADERS_CAB_TYPE_PORT
INPUT_PORTS_END


static MACHINE_CONFIG_DERIVED_CLASS( yosakdon, mw8080bw_root, _8080bw_state )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(yosakdon_map)
	MCFG_CPU_IO_MAP(yosakdon_io_map)
	MCFG_MACHINE_START_OVERRIDE(_8080bw_state,extra_8080bw)

	/* sound hardware */
	MCFG_FRAGMENT_ADD(invaders_samples_audio)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(_8080bw_state, screen_update_invaders)
MACHINE_CONFIG_END



/*******************************************************/
/*                                                     */
/* Taito  "Indian battle"                              */
/* In "indianbtbr", the "number of animals" dipswitch  */
/*  is ineffective because they compare for 8 kills at */
/*  0x811, which is not possible. This byte should be  */
/*  0x03.                                              */
/*                                                     */
/*******************************************************/

static INPUT_PORTS_START( indianbt )
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_DIPNAME( 0x08, 0x00, "Number of Catch Animals" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_DIPNAME(0x80,  0x00, "Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	/* Dummy port for cocktail mode */
	INVADERS_CAB_TYPE_PORT
INPUT_PORTS_END

static INPUT_PORTS_START( indianbtbr )
	PORT_START("IN0")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) //Enable color map to be in C400-DFFF
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) //Length of manufacturer's logo (0x11 or 0x16)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME(0x08,  0x00, "Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	/* Dummy port for cocktail mode */
	INVADERS_CAB_TYPE_PORT
INPUT_PORTS_END


/*
 Protection / sound hw checks ?

 ld    a ,$b
 out  ($03),a
 out  ($01),a
 in   a,($00)
 and  $f0
 cp   $10
 jp   nz,$3000
 ld   a,$03
 out  ($03),a
 out  ($01),a
 in   a,($00)
 jp   $5de7
 and  $f0
 jp   z,$052b
 jp   $3000

*/

READ8_MEMBER(_8080bw_state::indianbt_r)
{
	switch(space.device().safe_pc())
	{
		case 0x5fed:    return 0x10;
		case 0x5ffc:    return 0;
	}
	logerror("unknown port 0 read @ %x\n",space.device().safe_pc());
	return machine().rand();
}

READ8_MEMBER(_8080bw_state::indianbtbr_01_r)
{
	UINT8 data = ioport("IN1")->read();
	if (!m_flip_screen) return data;
	return (data & 0x8f) | (ioport("IN2")->read() & 0x70);
}

static ADDRESS_MAP_START( indianbt_io_map, AS_IO, 8, _8080bw_state )
	AM_RANGE(0x00, 0x00) AM_READ(indianbt_r)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_READ(invrvnge_02_r) AM_DEVWRITE("mb14241", mb14241_device, shift_count_w)
	AM_RANGE(0x03, 0x03) AM_DEVREAD("mb14241", mb14241_device, shift_result_r) AM_WRITE(indianbt_sh_port_1_w)
	AM_RANGE(0x04, 0x04) AM_DEVWRITE("mb14241", mb14241_device, shift_data_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(indianbt_sh_port_2_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x07, 0x07) AM_WRITE(indianbt_sh_port_3_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( indianbtbr_io_map, AS_IO, 8, _8080bw_state )
	AM_RANGE(0x00, 0x00) AM_READ_PORT("IN0")
	AM_RANGE(0x01, 0x01) AM_READ(indianbtbr_01_r)
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN2") AM_DEVWRITE("mb14241", mb14241_device, shift_count_w)
	AM_RANGE(0x03, 0x03) AM_DEVREAD("mb14241", mb14241_device, shift_result_r) AM_WRITE(indianbtbr_sh_port_1_w)
	AM_RANGE(0x04, 0x04) AM_DEVWRITE("mb14241", mb14241_device, shift_data_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(indianbtbr_sh_port_2_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x07, 0x07) AM_WRITENOP
ADDRESS_MAP_END


static MACHINE_CONFIG_DERIVED_CLASS( indianbt, mw8080bw_root, _8080bw_state )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(indianbt_io_map)
	MCFG_MACHINE_START_OVERRIDE(_8080bw_state,extra_8080bw)

	/* add shifter */
	MCFG_MB14241_ADD("mb14241")

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(_8080bw_state, screen_update_indianbt)

	MCFG_PALETTE_ADD_3BIT_RGB("palette")

	/* sound hardware */
	MCFG_FRAGMENT_ADD(invaders_samples_audio)

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(indianbt)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( indianbtbr, mw8080bw_root, _8080bw_state )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(schaser_map)
	MCFG_CPU_IO_MAP(indianbtbr_io_map)
	MCFG_MACHINE_START_OVERRIDE(_8080bw_state,extra_8080bw)

	/* add shifter */
	MCFG_MB14241_ADD("mb14241")

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(_8080bw_state, screen_update_indianbt)

	MCFG_PALETTE_ADD_3BIT_RGB("palette")

	/* sound hardware */
	MCFG_FRAGMENT_ADD(invaders_samples_audio)
MACHINE_CONFIG_END



/*******************************************************/
/*                                                     */
/* Taito "Steel Worker"                                */
/*                                                     */
/*******************************************************/

WRITE8_MEMBER(_8080bw_state::steelwkr_sh_port_3_w)
{
	machine().bookkeeping().coin_lockout_global_w(!(~data & 0x03));      /* possibly */
}

static ADDRESS_MAP_START( steelwkr_io_map, AS_IO, 8, _8080bw_state )
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_READ(invrvnge_02_r) AM_DEVWRITE("mb14241", mb14241_device, shift_count_w)
	AM_RANGE(0x03, 0x03) AM_DEVREAD("mb14241", mb14241_device, shift_result_r) AM_WRITE(invadpt2_sh_port_1_w)
	AM_RANGE(0x04, 0x04) AM_DEVWRITE("mb14241", mb14241_device, shift_data_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(invadpt2_sh_port_2_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(steelwkr_sh_port_3_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( steelwkr )
	/* PORT_START("IN0") - never read */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_BUTTON2 )

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_TILT )          PORT_DIPLOCATION("SW1:3")
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "SW1:4" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(2)

	/* Dummy port for cocktail mode */
	INVADERS_CAB_TYPE_PORT
INPUT_PORTS_END


static MACHINE_CONFIG_DERIVED_CLASS( steelwkr, mw8080bw_root, _8080bw_state )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(steelwkr_io_map)
	MCFG_MACHINE_START_OVERRIDE(_8080bw_state,extra_8080bw)

	/* add shifter */
	MCFG_MB14241_ADD("mb14241")

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(_8080bw_state, screen_update_invadpt2)

	MCFG_PALETTE_ADD_3BIT_RBG("palette")

	/* sound hardware */
	MCFG_FRAGMENT_ADD(invaders_samples_audio)
MACHINE_CONFIG_END



/*****************************************************

 Omori "Shuttle Invader" ??

 8080 CPU

 1x  SN76477
g 2x  SN75452
 4x  8216 RAM
 2x  TMS4045 RAM
 16x MCM4027 RAM
 1x  empty small socket. maybe (missing) PROM?
 1x  8 position dipsw
 1x  556
 1x  458
 1x  lm380 (amp chip)

 xtal 18MHz
 xtal 5.545MHz

******************************************************/

static INPUT_PORTS_START( shuttlei )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x06, "6" )
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

	PORT_START("INPUTS")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)

	/* Dummy port for cocktail mode */
	INVADERS_CAB_TYPE_PORT
INPUT_PORTS_END

// 'no 1' which is displayed before each player plays actually refers to the wave number, not the player number!
static INPUT_PORTS_START( skylove )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x06, "6" )
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

	PORT_START("INPUTS")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) ) // must be off to boot
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)

	/* Dummy port for cocktail mode */
	INVADERS_CAB_TYPE_PORT
INPUT_PORTS_END

READ8_MEMBER(_8080bw_state::shuttlei_ff_r)
{
	UINT8 data = ioport("INPUTS")->read();
	if (!m_flip_screen) return data;
	return (data & 0x3b) | ioport("P2")->read();
}

WRITE8_MEMBER(_8080bw_state::shuttlei_ff_w)
{
		/* bit 0 goes high when first coin inserted
		   bit 1 also goes high when subsequent coins are inserted
		      These may be for indicator lamps under the start buttons.
		   bit 2 goes high while player 2 is playing     */

	m_flip_screen = BIT(data, 2) & BIT(ioport(CABINET_PORT_TAG)->read(), 0);
}

static ADDRESS_MAP_START( shuttlei_map, AS_PROGRAM, 8, _8080bw_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x37ff) AM_RAM AM_SHARE("main_ram")
	AM_RANGE(0x3800, 0x3fff) AM_RAM
	AM_RANGE(0x4000, 0x43ff) AM_RAM AM_SHARE("share1") // shuttlei
	AM_RANGE(0x6000, 0x63ff) AM_RAM AM_SHARE("share1") // skylove (is it mirrored, or different PCB hookup?)
ADDRESS_MAP_END

static ADDRESS_MAP_START( shuttlei_io_map, AS_IO, 8, _8080bw_state )
	AM_RANGE(0xfc, 0xfc) AM_WRITENOP /* game writes 0xAA every so often (perhaps when base hit?) */
	AM_RANGE(0xfd, 0xfd) AM_WRITE(shuttlei_sh_port_1_w)
	AM_RANGE(0xfe, 0xfe) AM_READ_PORT("DSW") AM_WRITE(shuttlei_sh_port_2_w)
	AM_RANGE(0xff, 0xff) AM_READWRITE(shuttlei_ff_r,shuttlei_ff_w)
ADDRESS_MAP_END


MACHINE_CONFIG_START( shuttlei, _8080bw_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8080, XTAL_18MHz / 10) // divider guessed
	// TODO: move irq handling away from mw8080.c, this game runs on custom hardware
	MCFG_CPU_PROGRAM_MAP(shuttlei_map)
	MCFG_CPU_IO_MAP(shuttlei_io_map)

	MCFG_MACHINE_START_OVERRIDE(_8080bw_state, extra_8080bw)
	MCFG_MACHINE_RESET_OVERRIDE(_8080bw_state, mw8080bw)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 24*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(_8080bw_state, screen_update_shuttlei)

	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	/* sound hardware */
	MCFG_FRAGMENT_ADD(invaders_samples_audio)
MACHINE_CONFIG_END



/*

------------------------------------
Darth Vader - Space Invaders bootleg
------------------------------------

Location     Device     File ID     Checksum
--------------------------------------------
0             2708       ROM0         6F9A
1             2708       ROM1         7D2A
2             2708       ROM2         67AA
3             2708       ROM3         7D8D
4             2708       ROM4         493D
5             2708       ROM5         12CE


Notes:  PCB No. DV-SI-7811M2a
        CPU - 8080

Another (same checksums) dump came from board labeled SI-7811M-2

*/

MACHINE_START_MEMBER(_8080bw_state,darthvdr)
{
	/* do nothing for now - different interrupt system */
	m_fleet_step = 3;
}


MACHINE_RESET_MEMBER(_8080bw_state,darthvdr)
{
	/* do nothing for now - different interrupt system */
}

READ8_MEMBER(_8080bw_state::darthvdr_01_r)
{
	UINT8 data = ioport("P2")->read();
	if (m_flip_screen) return data;
	return (data & 0xe1) | (ioport("P1")->read() & 0x0e);
}

static ADDRESS_MAP_START( darthvdr_map, AS_PROGRAM, 8, _8080bw_state )
	AM_RANGE(0x0000, 0x17ff) AM_ROM
	AM_RANGE(0x1800, 0x1fff) AM_RAM
	AM_RANGE(0x4000, 0x5fff) AM_RAM AM_SHARE("main_ram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( darthvdr_io_map, AS_IO, 8, _8080bw_state )
	AM_RANGE(0x00, 0x00) AM_READ_PORT("P1")
	AM_RANGE(0x01, 0x01) AM_READ(darthvdr_01_r)

	AM_RANGE(0x00, 0x00) AM_WRITE(darthvdr_00_w) // flipscreen
	AM_RANGE(0x04, 0x04) AM_WRITENOP
	AM_RANGE(0x08, 0x08) AM_WRITE(darthvdr_08_w) // sound
ADDRESS_MAP_END


static INPUT_PORTS_START( darthvdr )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPSETTING(    0x20, "3000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x80, "6" )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_DIPNAME( 0x10, 0x10, "One less life" )
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

	/* Dummy port for cocktail mode */
	INVADERS_CAB_TYPE_PORT
INPUT_PORTS_END

static MACHINE_CONFIG_DERIVED_CLASS( darthvdr, mw8080bw_root, _8080bw_state )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(darthvdr_map)
	MCFG_CPU_IO_MAP(darthvdr_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", _8080bw_state,  irq0_line_hold)

	MCFG_MACHINE_START_OVERRIDE(_8080bw_state,darthvdr)
	MCFG_MACHINE_RESET_OVERRIDE(_8080bw_state,darthvdr)

	/* sound hardware */
	MCFG_FRAGMENT_ADD(invaders_samples_audio)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(_8080bw_state, screen_update_invaders)
MACHINE_CONFIG_END



/*************************************
 *
 * Vortex (by Zilec AKA Zenitone-Microsec)
 * Runs on Space Invaders CV (color)/PV (part 2) board with
 * some color mods, and an epoxy brick for rom encryption
 * see below for decryption function (A0, A3, A9 invert)
 * It uses its own I/O function since A9 is inverted (and A9 mirrors A1 for I/O)
 *
 * Hold down fire button to activate thrust.
 *
 *************************************/

static ADDRESS_MAP_START( vortex_io_map, AS_IO, 8, _8080bw_state )
	// I/O map is same as invaders but with A9 (used as A1 for I/O) inverted
	ADDRESS_MAP_GLOBAL_MASK(0xFF)
	AM_RANGE(0x02, 0x02) AM_MIRROR(0x04) AM_READ_PORT("IN0")
	AM_RANGE(0x03, 0x03) AM_MIRROR(0x04) AM_READ_PORT("IN1")
	AM_RANGE(0x00, 0x00) AM_MIRROR(0x04) AM_READ_PORT("IN2")
	AM_RANGE(0x01, 0x01) AM_MIRROR(0x04) AM_DEVREAD("mb14241", mb14241_device, shift_result_r)

	AM_RANGE(0x00, 0x00) AM_DEVWRITE("mb14241", mb14241_device, shift_count_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(invaders_audio_1_w)
	AM_RANGE(0x06, 0x06) AM_DEVWRITE("mb14241", mb14241_device, shift_data_w)
	AM_RANGE(0x07, 0x07) AM_WRITE(invaders_audio_2_w)
	AM_RANGE(0x04, 0x04) AM_WRITE(watchdog_reset_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( vortex )
	PORT_INCLUDE( sicv )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_1C ) )
INPUT_PORTS_END

MACHINE_CONFIG_DERIVED_CLASS( vortex, mw8080bw_root, _8080bw_state )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(vortex_io_map)
	MCFG_MACHINE_START_OVERRIDE(_8080bw_state,extra_8080bw)
	MCFG_WATCHDOG_TIME_INIT(attotime::from_usec(255000000 / (MW8080BW_PIXEL_CLOCK / MW8080BW_HTOTAL / MW8080BW_VTOTAL)))

	/* video hardware */
	// TODO: replace with modified invaders color renderer code allowing midscanline color writes
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(_8080bw_state, screen_update_invaders)

	/* add shifter */
	MCFG_MB14241_ADD("mb14241")

	/* audio hardware */
	MCFG_FRAGMENT_ADD(invaders_audio)
MACHINE_CONFIG_END

/* decrypt function for vortex */
DRIVER_INIT_MEMBER(_8080bw_state,vortex)
{
	UINT8 *rom = memregion("maincpu")->base();
	int length = memregion("maincpu")->bytes();
	dynamic_buffer buf1(length);
	UINT32 x;
	for (x = 0; x < length; x++)
	{
		UINT32 addr = x;
		/*
		A15 A14 A13     A0  A3  A9
		0   0   0       I   I   I
		0   0   1       I   I   I
		0   1   0       N   N   N
		0   1   1       N   I   I
		1   0   0       N   I   I
		1   0   1       N   I   I
		1   1   0       N   I   I
		1   1   1       N   I   I
		*/
		switch (x&0xE000) // inputs are A13 A14 A15
		{
			case 0x0000: case 0x2000: // A0 A3 A9
				addr ^= 0x0209;
				break;
			case 0x4000: // none, but doesn't decode right with none
				addr ^= 0x0209; // hack: this doesn't match schematic but gets code running. Why does this work? Is there something I'm not undertstanding about how the memory_region maps? or was the zilec/zinitone-microsec epoxy brick simply a bad design which is always stuck on the 0x0000 encryption no matter what?
				break;
			case 0x6000: case 0x8000: case 0xa000: case 0xc000: case 0xe000: // A3 and A9
				addr ^= 0x0208;
				break;
		/*
		    case 0x0000: case 0x2000: // A0 A3 A9
		        addr ^= 0x0001;
		        break;
		    case 0x4000:
		        addr ^= 0x0208;
		        break;
		    case 0x6000: case 0x8000: case 0xa000: case 0xc000: case 0xe000:
		        break;*/
		}
		buf1[addr] = rom[x];
	}

	memcpy(rom, &buf1[0], length);
}



/* unlabeled gun game by Model Racing, verified to be Gun Champ

BOARD 1:
 _________________________________________________________________________________________________________________________________
|                                                          12     13     14     15     16     17     18     19                    |
|                            _________    _______         ___    ___    ___    ___    ___    ___    ___    ___                    |
|___  11                    |74LS241N |  |74LS159|       |AM9|  |AM9|  |AM9|  |AM9|  |AM9|  |AM9|  |AM9|  |AM9|                   |
|  _|                       |_________|  |_______|       |060|  |060|  |060|  |060|  |060|  |060|  |060|  |060|                   |
|  _|      _____________                                 |CPC|  |CPC|  |CPC|  |CPC|  |CPC|  |CPC|  |CPC|  |CPC|                   |
|  _|     |258          |                                |   |  |   |  |   |  |   |  |   |  |   |  |   |  |   |                   |
|  _|     |             |    _________    _______        |   |  |   |  |   |  |   |  |   |  |   |  |   |  |   |                   |
|  _| 10  |         2708|   |74LS241N |  |74LS153|       |   |  |   |  |   |  |   |  |   |  |   |  |   |  |   |                   |
|  _|     |_____________|   |_________|  |_______|       |   |  |   |  |   |  |   |  |   |  |   |  |   |  |   |                   |
|  _|                                                    |___|  |___|  |___|  |___|  |___|  |___|  |___|  |___|                   |
|  _|      _____________                                                                                                          |
|  _|     |257          |    _______      _______                                                                                 |
|  _| 9   |             |   |74LS174|    |74LS153|                                                                              __|
|  _|     |         2708|   |_______|    |_______|        ___    ___    ___    ___    ___    ___    ___    ___                 |
|  _|     |_____________|                                |AM9|  |AM9|  |AM9|  |AM9|  |AM9|  |AM9|  |AM9|  |AM9|                |__
|  _|                                                    |060|  |060|  |060|  |060|  |060|  |060|  |060|  |060|                  =|
|  _|      _____________     _______      _______        |CPC|  |CPC|  |CPC|  |CPC|  |CPC|  |CPC|  |CPC|  |CPC|                  =|
|  _| 8   |256          |   |74LS174|    |74LS153|       |   |  |   |  |   |  |   |  |   |  |   |  |   |  |   |                  =|
|  _|     |             |   |_______|    |_______|       |   |  |   |  |   |  |   |  |   |  |   |  |   |  |   |                  =|
|  _|     |         2708|                                |   |  |   |  |   |  |   |  |   |  |   |  |   |  |   |                  =|
|  _|     |_____________|                                |   |  |   |  |   |  |   |  |   |  |   |  |   |  |   |                  =|
|  _|                        _______      _______        |___|  |___|  |___|  |___|  |___|  |___|  |___|  |___|                  =|
|___| 7    _____________    |74LS166|    |74LS174|                                                                               =|
|         |255          |   |_______|    |_______|                                                                               =|
|         |             |                                                                                                        =|
|         |         2708|    _______      _______                                                                                =|
|     6   |_____________|   |74LS86 |    | 74LS04|                                                                               =|
|                           |_______|    |_______|                                                                               =|
|___       _____________                                                                                                         =|
|  _|     |254          |    _______      _______      _______      _______      _______      _______      _______               =|
|  _|     |             |   |74LS00N|    |SN7404N|    |74LS74 |    |74LS157|    |74LS157|    |74LS157|    |74LS157|     5        =|
|  _|     |         2708|   |_______|    |_______|    |_______|    |_______|    |_______|    |_______|    |_______|              =|
|  _|     |_____________|                                                                                                        =|
|  _|                                     _________    _________    _______      _______      _______      _______               =|
|  _|      _____________                 |74LS244N |  |74LS244N |  |74161N |    |74161N |    |74161N |    |74161N |     4       __|
|  _|     |253          |                |_________|  |_________|  |_______|    |_______|    |_______|    |_______|            |
|  _|     |             |                                                                                                      |__
|  _|     |         2708|                                                                                                         |
|  _|     |_____________|                                           _______      _______      _______      _______                |
|  _|                                                              |74LS04N|    |SN7474N|    |74LS04N|    |74160N |     3         |
|  _|      _____________                  _____________________    |_______|    |_______|    |_______|    |_______|               |
|  _|     |252          |                |         341         |                               .XTAL.                             |
|  _|     |             |    _______     |      INS8080AD      |    _______      _______      _______      _______                |
|  _|     |         2708|   |74LS42 |    |        C8080A       |   |74LS00 |    |74LS55 |    |74LS00 |    |74LS42 |     2         |
|  _|     |_____________|   |_______|    |_____________________|   |_______|    |_______|    |_______|    |_______|               |
|  _|                                                                                                                             |
|  _|      _____________                                            _______      _______      _______      _______                |
|  _|     |251          |                                          |74LS02N|    |74LS20N|    |75365N |    |SN7474N|     1         |
|  _|     |             |                                          |_______|    |_______|    |_______|    |_______|               |
|___|     |         2708|                                                                                                         |
|         |_____________|                                                                                            Model Racing |
|                                                                                                                    CS 235A      |
|               A               B            C            D            E            F            G            H                   |
|_________________________________________________________________________________________________________________________________|


                                                XTAL=19,66080MHz


BOARD 2:
 _________________________________________________________________________________________________________________________________
|                                                                                                                                 |
|                            _______     _______     _______     _______     _________                                            |
|___             11         |74LS175|   |74LS151|   |74LS151|   |74LS153|   |74LS244N |                  Model                    |
|  _|                       |_______|   |_______|   |_______|   |_______|   |_________|                  Racing                   |
|  _|                                                                                                                             |
|  _|                                                                                                    CS 238A                  |
|  _|                        _______     _______     _______     _______                                                          |
|  _|            10         |74LS174|   |74LS151|   |74LS151|   |74LS153|                                                         |
|  _|                       |_______|   |_______|   |_______|   |_______|                                                         |
|  _|                                                                                                                             |
|  _|                                                                                                                             |
|  _|                        _______     _______     _______     _______                ____________                              |
|  _|                       |74LS174|   |74LS151|   |74LS151|   |74LS153|      9       |            |                           __|
|  _|                       |_______|   |_______|   |_______|   |_______|              |            |                          |
|  _|                                                                                  |            |                          |__
|  _|                                                                                  |            |                            =|
|  _|                        _______     _______     _______     _______               |            |                            =|
|  _|                       |74LS174|   |74LS151|   |74LS151|   |74LS153|      8       |            |                            =|
|  _|                       |_______|   |_______|   |_______|   |_______|              |            |                            =|
|  _|                                                                                  |            |                            =|
|  _|                                                                                  |            |                            =|
|  _|                        _______     _______     _______     _______               |   UNKNOWN  |                            =|
|___|                       |74LS273|   |74LS175|   |74LS14N|   |74LS42N|      7       |            |                            =|
|                           |_______|   |_______|   |_______|   |_______|              |            |                            =|
|                                                                                      |            |                            =|
|                                                                                      |            |                            =|
|                                                                                      |            |                            =|
|                                                                                      |            |    _______                 =|
|___                         _______     _______     _______     _________             |            |   |CA8100 |   6            =|
|  _|                       |74LS161|   |74LS74A|   |74LS161|   |74LS374N |    5       |            |   |_______|                =|
|  _|                       |_______|   |_______|   |_______|   |_________|            |            |                            =|
|  _|                                                                                  |            |                            =|
|  _|                                                                                  |____________|                            =|
|  _|                        _______     _______     _______     _________                                                       =|
|  _|                       |74LS161|   |74LS00 |   |74LS161|   |74LS374N |    4                                                 =|
|  _|                       |_______|   |_______|   |_______|   |_________|                                                      =|
|  _|                                                                                                                            =|
|  _|                                                                                                                            =|
|  _|                        _______     _______     ______________                        _______     ____                     __|
|  _|                 3     |74LS14N|   |74LS74A|   |   SN76477N   |                      |74LS107|   |DIP1|                   |
|  _|                       |_______|   |_______|   |    7923XY    |                      |_______|   |____|                   |__
|  _|                                               |   SINGAPORE  |                                                              |
|  _|                                               |______________|                                                              |
|  _|                        _______     _______                                           _______     _______    _______         |
|  _|                 2     |74LS26 |   |74LS00N|                                         |CD4016B|   |74LS221|  |74LS00N|        |
|  _|                       |_______|   |_______|                                         |_______|   |_______|  |_______|        |
|  _|                                                                                                                             |
|  _|                                                                                   NE555P                                    |
|  _|                        _______     _______     _______                               ___         _______    _______         |
|___|                 1     |74LS74A|   |74LS74A|   |74LS74A|                             |   |       |74LS90N|  |74LS14N|        |
|                           |_______|   |_______|   |_______|                             |___|       |_______|  |_______|        |
|                                                                                                                                 |
|                               P           R           S            T             U         V            W          X            |
|_________________________________________________________________________________________________________________________________|


Claybuster is on the same hardware, PCB labels CS 235A and CS 238A as well

*/

TIMER_DEVICE_CALLBACK_MEMBER(_8080bw_state::claybust_gun_callback)
{
	// reset gun latch
	m_claybust_gun_pos = 0;
}

CUSTOM_INPUT_MEMBER(_8080bw_state::claybust_gun_on_r)
{
	return (m_claybust_gun_pos != 0) ? 1 : 0;
}

INPUT_CHANGED_MEMBER(_8080bw_state::claybust_gun_trigger)
{
	if (newval)
	{
		/*
		    The game registers a valid shot after the gun trigger is pressed, and IN1 d0 is high.
		    It latches the gun position and then compares it with VRAM contents: 1 byte/8 pixels, 0 means miss.
		    IN1 d0 probably indicates if the latch is ready or not (glitches happen otherwise)

		    in   $06
		    cpi  $04
		    rc
		    mov  h,a
		    in   $02
		    mov  l,a
		    lxi  d,$1ffe  <-- this is where the +2 comes from
		    dad  d
		    out  $00
		    mov  a,m
		    ana  a
		    rz
		*/
		UINT8 const gunx = read_safe(ioport("GUNX"), 0x00);
		UINT8 const guny = read_safe(ioport("GUNY"), 0x20);
		m_claybust_gun_pos = ((gunx >> 3) | (guny << 5)) + 2;
		m_claybust_gun_on->adjust(attotime::from_msec(250)); // timing is a guess
	}
}

READ8_MEMBER(_8080bw_state::claybust_gun_lo_r)
{
	return m_claybust_gun_pos & 0xff;
}

READ8_MEMBER(_8080bw_state::claybust_gun_hi_r)
{
	return m_claybust_gun_pos >> 8;
}

static ADDRESS_MAP_START( claybust_io_map, AS_IO, 8, _8080bw_state )
	//AM_RANGE(0x00, 0x00) AM_WRITENOP // ?
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1") AM_DEVWRITE("mb14241", mb14241_device, shift_count_w)
	AM_RANGE(0x02, 0x02) AM_READ(claybust_gun_lo_r) AM_DEVWRITE("mb14241", mb14241_device, shift_data_w)
	AM_RANGE(0x03, 0x03) AM_DEVREAD("mb14241", mb14241_device, shift_result_r) //AM_WRITENOP // port3 write looks sound-related
	AM_RANGE(0x04, 0x04) AM_WRITE(watchdog_reset_w)
	//AM_RANGE(0x05, 0x05) AM_WRITENOP // ?
	AM_RANGE(0x06, 0x06) AM_READ(claybust_gun_hi_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( claybust )
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, _8080bw_state, claybust_gun_on_r, NULL)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_IMPULSE(2) PORT_CHANGED_MEMBER(DEVICE_SELF, _8080bw_state, claybust_gun_trigger, NULL)
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 )

	// switch is 6-pos, but DNS06:5 and DNS06:6 are not connected
	PORT_DIPNAME( 0x10, 0x10, "Shots" )             PORT_DIPLOCATION("DNS06:1")
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DNS06:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DNS06:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DNS06:4" )

	PORT_START( "GUNX" )
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_MINMAX(0x00, 0xff) PORT_CROSSHAIR(X, 1.0 - (MW8080BW_HPIXCOUNT-256)/256.0, (MW8080BW_HPIXCOUNT-256)/256.0, 0) PORT_SENSITIVITY(56) PORT_KEYDELTA(5)
	PORT_START( "GUNY" )
	PORT_BIT( 0xff, 0xa0, IPT_LIGHTGUN_Y ) PORT_MINMAX(0x20, 0xff) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(64) PORT_KEYDELTA(5)
INPUT_PORTS_END

static INPUT_PORTS_START( gunchamp )
	PORT_INCLUDE( claybust )

	PORT_MODIFY("IN1")

	// switch is 6-pos, but DNS06:5 and DNS06:6 are not connected
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DNS06:1")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xe0, 0x40, DEF_STR( Coinage ) )  PORT_DIPLOCATION("DNS06:2,3,4")
	PORT_DIPSETTING(    0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
INPUT_PORTS_END


MACHINE_START_MEMBER(_8080bw_state, claybust)
{
	m_claybust_gun_pos = 0;
	save_item(NAME(m_claybust_gun_pos));

	MACHINE_START_CALL_MEMBER(mw8080bw);
}

MACHINE_CONFIG_DERIVED_CLASS( claybust, invaders, _8080bw_state )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(claybust_io_map)

	MCFG_TIMER_DRIVER_ADD("claybust_gun", _8080bw_state, claybust_gun_callback)

	MCFG_MACHINE_START_OVERRIDE(_8080bw_state, claybust)

	/* sound hardware */
	// TODO: discrete sound
MACHINE_CONFIG_END



/**************************************************************************************

Taito Galactica / Space Missile

This game was officially only distributed in Brazil.
Regarding release data, not much information is available online.

ROM dump came from a collection of old 5 1/4 disks (Apple II) that used to be in the
 possession of an arcade operator in the early 80s.

TODO:
- correct sound (currently same as invaders)
  * sound mutes when a few aliens are left?
  * port 7 write is used too, looks like it's for music similar to indianbt
  Note that bass background hum and sound effects are already basically correct.

***************************************************************************************/

static INPUT_PORTS_START( galactic )
	PORT_START("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // gets read into memory (0x2012) then never used

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x70, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, mw8080bw_state,invaders_in1_control_r, NULL)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, "4000" )
	PORT_DIPSETTING(    0x02, "5000" )
	PORT_DIPSETTING(    0x03, "7000" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x08, "6" )
	PORT_BIT( 0x70, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, mw8080bw_state,invaders_in2_control_r, NULL)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	/* Dummy controls port, P1 */
	INVADERS_CONTROL_PORT_P1

	/* Dummy controls port, P2 */
	INVADERS_CONTROL_PORT_P2

	/* Dummy port for cocktail mode */
	INVADERS_CAB_TYPE_PORT
INPUT_PORTS_END



/*****************************************************

  Attack Force, by E.G.S., Italy
  Not much information is available for this game.

  20MHz XTAL, 2MHz CPU
  video: 15625Hz

  TODO: sound
  PORT 02 : 10 while your missile is on-screen
  PORT 04 : 01 while game is playing. Sound enable.
  PORT 05 : Watchdog?
  PORT 06 : 01=Helicopter; 02=Tank; 03=Motorcycle
            08=Explosion; 10=Walking


*****************************************************/

static ADDRESS_MAP_START( attackfc_io_map, AS_IO, 8, _8080bw_state )
	AM_RANGE(0x00, 0x00) AM_READ_PORT("IN0")
	AM_RANGE(0x02, 0x02) AM_WRITENOP // lamp?
	AM_RANGE(0x03, 0x03) AM_DEVREADWRITE("mb14241", mb14241_device, shift_result_r, shift_data_w)
	AM_RANGE(0x04, 0x04) AM_WRITENOP // sound enable?
	AM_RANGE(0x05, 0x05) AM_WRITENOP // watchdog?
	AM_RANGE(0x06, 0x06) AM_WRITENOP // sound?
	AM_RANGE(0x07, 0x07) AM_DEVWRITE("mb14241", mb14241_device, shift_count_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( attackfc )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
INPUT_PORTS_END


static MACHINE_CONFIG_DERIVED_CLASS( attackfc, mw8080bw_root, _8080bw_state )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(attackfc_io_map)

	/* add shifter */
	MCFG_MB14241_ADD("mb14241")

	/* sound hardware */
	// TODO: custom discrete
MACHINE_CONFIG_END


DRIVER_INIT_MEMBER(_8080bw_state,attackfc)
{
	UINT8 *rom = memregion("maincpu")->base();
	UINT32 len = memregion("maincpu")->bytes();
	dynamic_buffer buffer(len);

	// swap a8/a9
	for (int i = 0; i < len; i++)
		buffer[BITSWAP16(i, 15,14,13,12,11,10,8,9, 7,6,5,4,3,2,1,0)] = rom[i];

	memcpy(rom, &buffer[0], len);
}



/*****************************************************

 Space Invaders Multigame kit, Braze Technologies,
 produced from 2002(version 1A) to 2006(version 3D).
 This is an 8-in-1 hack on a daughterboard, containing:

 - 8080 CPU taken from main PCB
 - SST 29EE010 or AM27C010 (or other similar) 128KB EEPROM
   (EEPROM functionality not used)
 - 93C46P E2PROM for saving highscore/settings
 - PALCE22V10H-25PC/4

 The kit is compatible with the original Midway boardset

******************************************************/

static INPUT_PORTS_START( invmulti )
	/* same as Midway Space Invaders, except that SW is unused */
	PORT_START("IN0")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x00, "SW:8" )
	PORT_BIT( 0x06, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, mw8080bw_state,invaders_sw6_sw7_r, NULL)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x70, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, mw8080bw_state,invaders_in0_control_r, NULL)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, mw8080bw_state,invaders_sw5_r, NULL)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, mw8080bw_state,invaders_coin_input_r, NULL)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x70, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, mw8080bw_state,invaders_in1_control_r, NULL)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x00, "SW:3" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x00, "SW:4" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x00, "SW:2" )
	PORT_BIT( 0x70, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, mw8080bw_state,invaders_in2_control_r, NULL)
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "SW:1" )

	/* fake port for reading the coin input */
	PORT_START(INVADERS_COIN_INPUT_PORT_TAG)
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	/* fake port for cabinet type */
	PORT_START(INVADERS_CAB_TYPE_PORT_TAG)
	PORT_CONFNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_CONFSETTING(    0x00, DEF_STR( Upright ) )
	PORT_CONFSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	/* fake ports for handling the various input ports based on cabinet type */
	PORT_START(INVADERS_SW6_SW7_PORT_TAG)
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x00, "SW:7" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x00, "SW:6" )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START(INVADERS_SW5_PORT_TAG)
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x00, "SW:5" )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START(INVADERS_P1_CONTROL_PORT_TAG)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START(INVADERS_P2_CONTROL_PORT_TAG)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static ADDRESS_MAP_START( invmulti_map, AS_PROGRAM, 8, _8080bw_state )
	AM_RANGE(0x0000, 0x1fff) AM_MIRROR(0x8000) AM_ROMBANK("bank1")
	AM_RANGE(0x2000, 0x3fff) AM_MIRROR(0x8000) AM_RAM AM_SHARE("main_ram")
	AM_RANGE(0x4000, 0x5fff) AM_MIRROR(0x8000) AM_ROMBANK("bank2")
	AM_RANGE(0x6000, 0x6000) AM_MIRROR(0x1fff) AM_READWRITE(invmulti_eeprom_r, invmulti_eeprom_w)
	AM_RANGE(0xe000, 0xe000) AM_MIRROR(0x1fff) AM_WRITE(invmulti_bank_w)
ADDRESS_MAP_END

READ8_MEMBER(_8080bw_state::invmulti_eeprom_r)
{
	return m_eeprom->do_read();
}

WRITE8_MEMBER(_8080bw_state::invmulti_eeprom_w)
{
	// d0: latch bit
	m_eeprom->di_write(data & 1);

	// d6: reset
	m_eeprom->cs_write((data & 0x40) ? ASSERT_LINE : CLEAR_LINE);

	// d4: write latch or select next bit to read
	m_eeprom->clk_write((data & 0x10) ? ASSERT_LINE : CLEAR_LINE);
}

WRITE8_MEMBER(_8080bw_state::invmulti_bank_w)
{
	// d0, d4, d6: bank
	int bank = (data & 1) | (data >> 3 & 2) | (data >> 4 & 4);
	membank("bank1")->set_entry(bank);
	membank("bank2")->set_entry(bank);
}

MACHINE_CONFIG_DERIVED_CLASS( invmulti, invaders, _8080bw_state )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(invmulti_map)

	MCFG_EEPROM_SERIAL_93C46_8BIT_ADD("eeprom")

	MCFG_MACHINE_RESET_OVERRIDE(_8080bw_state, mw8080bw)
MACHINE_CONFIG_END

DRIVER_INIT_MEMBER(_8080bw_state,invmulti)
{
	UINT8 *src = memregion("user1")->base();
	int len = memregion("user1")->bytes();
	UINT8 *dest = memregion("maincpu")->base();

	// decrypt rom
	for (int i = 0; i < len; i++)
		dest[i] = BITSWAP8(src[(i & 0x100ff) | (BITSWAP8(i >> 8 & 0xff, 7,3,4,5,0,6,1,2) << 8)],0,6,5,7,4,3,1,2);

	membank("bank1")->configure_entries(0, 8, memregion("maincpu")->base(), 0x4000);
	membank("bank1")->set_entry(0);
	membank("bank2")->configure_entries(0, 8, memregion("maincpu")->base() + 0x2000, 0x4000);
	membank("bank2")->set_entry(0);
}



/**************************************************************************************************************/

ROM_START( searthin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "earthinv.h",   0x0000, 0x0800, CRC(58a750c8) SHA1(90bfa4ea06f38e67fe4286d37d151632439249d2) )
	ROM_LOAD( "earthinv.g",   0x0800, 0x0800, CRC(b91742f1) SHA1(8d9ca92405fbaf1d5a7138d400986616378d061e) )
	ROM_LOAD( "earthinv.f",   0x1000, 0x0800, CRC(4acbbc60) SHA1(b8c1efb4251a1e690ff6936ec956d6f66136a085) )
	ROM_LOAD( "earthinv.e",   0x1800, 0x0800, CRC(df397b12) SHA1(e7e8c080cb6baf342ec637532e05d38129ae73cf) )
ROM_END

ROM_START( searthina )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "unkh.h1",   0x0000, 0x0400, CRC(272b9bf3) SHA1(dd57d6a88d42024a39640931114107b547b4c520) )
	ROM_LOAD( "unkg.g1",   0x0400, 0x0400, CRC(61bb6101) SHA1(8fc8bbd8ac93d239e0cf0e4881f709860ec2c973) )
	ROM_LOAD( "unkf.f1",   0x0800, 0x0400, CRC(2a8d9cd5) SHA1(7948d79b326e729bcb629607c8797156ff9fb0e8) )
	ROM_LOAD( "unke.e1",   0x0c00, 0x0400, CRC(1938d349) SHA1(3bd2a0deb126cf2e22bc3cb53e9a59c3875be260) )
	ROM_LOAD( "unkd.d1",   0x1000, 0x0400, CRC(9bc2ab88) SHA1(1e9f3b780135827d16ba25978382b097a8110828) )
	ROM_LOAD( "unkc.c1",   0x1400, 0x0400, CRC(d4e2dada) SHA1(e98271212fc89e240fdf97d292edd17dc8dd4191) )
	ROM_LOAD( "unkb.b1",   0x1800, 0x0400, CRC(ab645a9c) SHA1(9c286f8a031a8babfb8e9b594e05e133c338b342) )
	ROM_LOAD( "unka.a1",   0x1c00, 0x0400, CRC(4b65bd7c) SHA1(3931f9f5b0e3339ab484eee14473d3a474935fd9) )
ROM_END

ROM_START( searthie )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "searthie.h",   0x0000, 0x0800, CRC(92b08b91) SHA1(4cebb70735e5231717619c7b8e5d3080694338b7) )
	ROM_LOAD( "searthie.g",   0x0800, 0x0800, CRC(23e24bcc) SHA1(a62e8422554f7db34796d4fb1c01e8ddebc7e978) )
	ROM_LOAD( "searthie.f",   0x1000, 0x0800, CRC(8700286a) SHA1(e0a3c099bc60e70bc9a6c0325944454d9d26428f) )
	ROM_LOAD( "searthie.e",   0x1800, 0x0800, CRC(baf949b0) SHA1(bfda97a3ef59fcdf87814afc6918507190c3e315) )
ROM_END

ROM_START( invadrmr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "11.1t",       0x0000, 0x0400, CRC(389d44b6) SHA1(5d2581b8bc0da918ce57cf319e06b5b31989c681) )
	ROM_LOAD( "sv02.1p",     0x0400, 0x0400, CRC(0e159534) SHA1(94b2015a9d38ca738705b8d024a79fd2f9855b98) )
	ROM_LOAD( "20.1n",       0x0800, 0x0400, CRC(805b04f0) SHA1(209f42dfde1593699ccf3755e9267d425416d910) )
	ROM_LOAD( "sv04.1j",     0x1400, 0x0400, CRC(1293b826) SHA1(165cd5d08a19eadbe954145b12807f10df9e691a) )
	ROM_LOAD( "13.1h",       0x1800, 0x0400, CRC(76b4a6ea) SHA1(076f8d12ba7ebe66b83a40d9a848075627776554) )
	ROM_LOAD( "sv06.1g",     0x1c00, 0x0400, CRC(2c68e0b4) SHA1(a5e5357120102ad32792bf3ef6362f45b7ba7070) )
ROM_END

ROM_START( claybust )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "0.a1",         0x0000, 0x0400, CRC(90810582) SHA1(a5c3655bae6f92a3cd0eae3a5a3c25e414d4fdf0) )
	ROM_LOAD( "1.a2",         0x0400, 0x0400, CRC(5ce6fb0e) SHA1(19fa3fbc0dd7e0fa4fffc005ded5a814c3b48f2d) )
	ROM_LOAD( "2.a4",         0x0800, 0x0400, CRC(d4c1d523) SHA1(1a4785095caa8200d7e1d8d53a93c8e298f52c65) )
	ROM_LOAD( "3.a5",         0x0c00, 0x0400, CRC(1ca00825) SHA1(74633a4903a51f1eebdd09679597dbe86db2e001) )
	ROM_LOAD( "4.a6",         0x1000, 0x0400, CRC(09a21120) SHA1(e976d2c173c649e51b032bc5dad54f006864155c) )
	ROM_LOAD( "5.a8",         0x1400, 0x0400, CRC(92cd4da8) SHA1(217e00012a52c479bf0b0cf37ce556387755740d) )
ROM_END

ROM_START( gunchamp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "251.a1",       0x0000, 0x0400, CRC(f27a8c1e) SHA1(510debd1ac2c0986f99c217e3078208a39d7837c) )
	ROM_LOAD( "252.a2",       0x0400, 0x0400, CRC(d53b8f91) SHA1(56919f4c88fb3b5c23b5365f0866698bfceb2762) )
	ROM_LOAD( "253.a4",       0x0800, 0x0400, CRC(9ef35c6c) SHA1(95bda3e2cdd50f7ac989c581481bad5f1ef2992f) )
	ROM_LOAD( "254.a5",       0x0c00, 0x0400, CRC(ba5b562d) SHA1(47819d7e5ef3700e700a5f2faa9537bc2199561c) )
	ROM_LOAD( "255.a6",       0x1000, 0x0400, CRC(00ea8293) SHA1(9c921fa4bafc36fc16a3f5f8588887342936d433) )
	ROM_LOAD( "256.a8",       0x1400, 0x0400, CRC(e271150c) SHA1(36d0c0c1335036b4a994e8a38904adcf74161c59) )
	ROM_LOAD( "257.a9",       0x1800, 0x0400, CRC(0da5d9ad) SHA1(c87c6ab248bfd2b75f070343a8f7fcbaed13f4e3) )
	ROM_LOAD( "258.a10",      0x1c00, 0x0400, CRC(471d4052) SHA1(c8ccda2eba44c2ab49f5fc2874fe70c2bdae35d3) )
ROM_END

ROM_START( spaceatt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "h",            0x0000, 0x0400, CRC(d0c32d72) SHA1(b3bd950b1ba940fbeb5d95e55113ed8f4c311434) ) // == SV01
	ROM_LOAD( "sv02.bin",     0x0400, 0x0400, CRC(0e159534) SHA1(94b2015a9d38ca738705b8d024a79fd2f9855b98) ) // == SV02
	ROM_LOAD( "f",            0x0800, 0x0400, CRC(483e651e) SHA1(ae795ee3bc53ac3936f6cf2c72cca7a890783513) ) // == SV10
	ROM_LOAD( "c",            0x1400, 0x0400, CRC(1293b826) SHA1(165cd5d08a19eadbe954145b12807f10df9e691a) ) // == SV04
	ROM_LOAD( "b",            0x1800, 0x0400, CRC(6fc782aa) SHA1(0275adbeec455e146f4443b0b836b1171436b79b) )
	ROM_LOAD( "a",            0x1c00, 0x0400, CRC(211ac4a3) SHA1(e08e90a4e77cfa30400626a484c9f37c87ea13f9) )
ROM_END

/* SPACE ATTACK set is from Video Game GmbH - 1010 A / Top board shows Video-Games - 6302 LICH - 1034
   Roms are set up as 1k bproms (82S137) and data is 1 rom top 4 bits, another bottom 4.  This data once assembled matches original spaceatt set */
ROM_START( spaceattbp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROMX_LOAD( "06e.bin",      0x0000, 0x0400, CRC(68301d05) SHA1(b0c33a982b42378da828281e74356d58fbea1d86), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI )
	ROMX_LOAD( "12l.bin",      0x0000, 0x0400, CRC(c5a5228f) SHA1(7861b5567d44e972d728551d47aab9b92d71ffc7), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO )
	ROMX_LOAD( "05de.bin",     0x0400, 0x0400, CRC(42032c14) SHA1(753948e7f52b88655c894b48d419b76de07c14f2), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI )
	ROMX_LOAD( "11hl.bin",     0x0400, 0x0400, CRC(d5d3811a) SHA1(7d2d983fa88b0349a90a6331ca3e18583125d21e), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO )
	ROMX_LOAD( "04d.bin",      0x0800, 0x0400, CRC(5f5e540c) SHA1(9092794a878494dbe34c2f05a212ff7b9d00fc55), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI )
	ROMX_LOAD( "10h.bin",      0x0800, 0x0400, CRC(9d5ef6f1) SHA1(ef584678373375a7f13307d7c4597639a5f6010e), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO )
	ROMX_LOAD( "03b.bin",      0x1400, 0x0400, CRC(89e13008) SHA1(1ad82ae0607af27925b42758f8c86a0e89079620), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI )
	ROMX_LOAD( "09g.bin",      0x1400, 0x0400, CRC(c16f5503) SHA1(cf36beac472c5c405342193b7ef434d32b37a4a8), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO )
	ROMX_LOAD( "02ab.bin",     0x1800, 0x0400, CRC(ffa166c2) SHA1(10496fcbb272130cc200dfb1886808559be8d6ea), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI )
	ROMX_LOAD( "08f.bin",      0x1800, 0x0400, CRC(b5fa1a2b) SHA1(7eab1cb9a9f95520a37ee4fb2b246ef072dedcbd), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO )
	ROMX_LOAD( "01a.bin",      0x1c00, 0x0400, CRC(44f8e99c) SHA1(9adecdadb16edaebde02892e30f9f87fb98f4ae1), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI )
	ROMX_LOAD( "07ef.bin",     0x1c00, 0x0400, CRC(9560880d) SHA1(866d6c3714b939814ce48707be53a69ef8355b34), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO )
ROM_END

/* SPACE ATTACK set is from Video Games GmbH - Board Typ 1010 C / Top board shows Video-Games - 6302 LICH - 1034B
   Contains same data as spaceatt but with added 00 fill to make larger roms (b+a=E1, 00fill+c=F1, f+00fill=G1, h+sv02=H1) */
ROM_START( spaceatt2k )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "H1.bin",     0x0000, 0x0800, CRC(734f5ad8) SHA1(ff6200af4c9110d8181249cbcef1a8a40fa40b7f) )
	ROM_LOAD( "G1.bin",     0x0800, 0x0800, CRC(6bfaca4a) SHA1(16f48649b531bdef8c2d1446c429b5f414524350) )
	ROM_LOAD( "F1.bin",     0x1000, 0x0800, CRC(0ccead96) SHA1(537aef03468f63c5b9e11dd61e253f7ae17d9743) )
	ROM_LOAD( "E1.bin",     0x1800, 0x0800, CRC(19971ca7) SHA1(373900e6796aa681f35158e2c4c7665574990906) )
ROM_END

ROM_START( spaceat2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "spaceatt.h",   0x0000, 0x0800, CRC(a31d0756) SHA1(2b76929654ed0b180091348546dac29fc6e5438e) )
	ROM_LOAD( "spaceatt.g",   0x0800, 0x0800, CRC(f41241f7) SHA1(d93cead75922510075433849c4f7099279eafc18) )
	ROM_LOAD( "spaceatt.f",   0x1000, 0x0800, CRC(4c060223) SHA1(957e75a978aa600627399061cae0a6525e92ad11) )
	ROM_LOAD( "spaceatt.e",   0x1800, 0x0800, CRC(7cf6f604) SHA1(469557de15178c4b2d686e5724e1006f7c20d7a4) )
ROM_END

ROM_START( galmonst )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "h.5m",            0x0000, 0x0400, CRC(8a0395e9) SHA1(f456aaa0301a1d0f3f9f45cbe39c5ff14909ecd3) )
	ROM_LOAD( "g.5n",            0x0400, 0x0400, CRC(6183ed16) SHA1(8e0bc13cafa237daa5fdeda9a5d6df8f491eabc2) )
	ROM_LOAD( "f.5p",            0x0800, 0x0400, CRC(b6047fdd) SHA1(bc324a9bf7829a2c2bb2bbf965d64272b0d07223) )
	ROM_LOAD( "c.5t",            0x1400, 0x0400, CRC(e88e8c96) SHA1(43108ddb328914c68977c7c49b4c1f71073ca36b) )
	ROM_LOAD( "b.5u",            0x1800, 0x0400, CRC(34678b80) SHA1(17f01facb3272c963a8bca290c4ca36411b8de31) )
	ROM_LOAD( "a.5v",            0x1c00, 0x0400, CRC(05a6806b) SHA1(ea884110d0ea6463801cbc2f87ce9c4921b49e33) )
ROM_END

ROM_START( spacecom )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1f.ic67",      0x0000, 0x0400, BAD_DUMP CRC(703f2cbe) SHA1(b183f9fbedd8658399555c0ba21ecab6370e86cb) )
	ROM_LOAD( "2g.ic82",      0x0400, 0x0400, CRC(7269b719) SHA1(6fd5879a6f2a5b1d38c7f00996037418df9491d3) )
	ROM_LOAD( "3f.ic68",      0x0800, 0x0400, CRC(6badac4f) SHA1(7b998d8fb21d143f26d605fe2a7dbbe1cf65210f) )
	ROM_LOAD( "4g.ic83",      0x0c00, 0x0400, CRC(75b59ea7) SHA1(e00eb4a9cf662c84e18fc9efc29cedebf0c5af67) )
	ROM_LOAD( "5f.ic69",      0x1000, 0x0400, CRC(84b61117) SHA1(3e41ff74ad02a7da4bbc22f3b84917eec067bbca) )
	ROM_LOAD( "6g.ic84",      0x1400, 0x0400, CRC(de383625) SHA1(7ec0d7171e771c4b43e026f3f50a88d8ab2236bb) )
	ROM_LOAD( "7f.ic70",      0x1800, 0x0400, CRC(5a23dbc8) SHA1(4d193bb7b38fb7ccd57d2c72463a3fe123dbca58) )
	ROM_LOAD( "8g.ic85",      0x1c00, 0x0400, CRC(a5a467e3) SHA1(ef591059e55d21f14baa8af1f1324a9bc2ada8c4) )
ROM_END

ROM_START( sinvzen )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin",        0x0000, 0x0400, CRC(9b0da779) SHA1(a52ccdb252eb69c497aa5eafb35d7f25a311b44e) )
	ROM_LOAD( "2.bin",        0x0400, 0x0400, CRC(9858ccab) SHA1(5ad8e5ef0d95779f0e513634b97bc330c9269ce4) )
	ROM_LOAD( "3.bin",        0x0800, 0x0400, CRC(a1cc38b5) SHA1(45fc9466b548d511b8174f6f3a4783164dd59489) )
	ROM_LOAD( "4.bin",        0x0c00, 0x0400, CRC(1f2db7a8) SHA1(354ad155743f724f2bebcab422f1ef96cb57c683) )
	ROM_LOAD( "5.bin",        0x1000, 0x0400, CRC(9b505fcd) SHA1(7461b7087d31dbe09f7b3078584ccaa2c9122c95) )
	ROM_LOAD( "6.bin",        0x1400, 0x0400, CRC(de0ca0ae) SHA1(a15d1218361839a2a2bf8da3f78d81621251fe1c) )
	ROM_LOAD( "7.bin",        0x1800, 0x0400, CRC(25a296f6) SHA1(37df98384c1513f0e33a350dfcaa99655f91c9ba) )
	ROM_LOAD( "8.bin",        0x1c00, 0x0400, CRC(f4bc4a98) SHA1(bff3806750a3695a136f398c7dbb69a0b7daa88a) )
ROM_END

ROM_START( sinvemag )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sv01.36",      0x0000, 0x0400, CRC(86bb8cb6) SHA1(a75648e7f2446c756d86624b15d387d25ce47b66) ) // sldh - == SV0H
	ROM_LOAD( "emag_si.b",    0x0400, 0x0400, CRC(febe6d1a) SHA1(e1c3a24b4fa5862107ada1f9d7249466e8c3f06a) )
	ROM_LOAD( "emag_si.c",    0x0800, 0x0400, CRC(aafb24f7) SHA1(6718cdfae09f77d735be5145b9d202a73d8ed9db) )
	ROM_LOAD( "emag_si.d",    0x1400, 0x0400, CRC(68c4b9da) SHA1(8953dc0427b09b71bd763e65caa7deaca09a15da) )
	ROM_LOAD( "emag_si.e",    0x1800, 0x0400, CRC(c4e80586) SHA1(3d427d5a2eea3c911ec7bd055e06e6747ce5e84d) )
	ROM_LOAD( "emag_si.f",    0x1c00, 0x0400, CRC(077f5ef2) SHA1(625de6839073ac4c904f949efc1b2e0afea5d676) )
ROM_END

ROM_START( tst_invd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "test.h",       0x0000, 0x0800, CRC(f86a2eea) SHA1(4a72ff01f3e6d16bbe9bf7f123cd98895bfbed9a) )   /*  The Test ROM */
	ROM_LOAD( "invaders.g",   0x0800, 0x0800, CRC(6bfaca4a) SHA1(16f48649b531bdef8c2d1446c429b5f414524350) )
	ROM_LOAD( "invaders.f",   0x1000, 0x0800, CRC(0ccead96) SHA1(537aef03468f63c5b9e11dd61e253f7ae17d9743) )
	ROM_LOAD( "invaders.e",   0x1800, 0x0800, CRC(14e538b0) SHA1(1d6ca0c99f9df71e2990b610deb9d7da0125e2d8) )
ROM_END

ROM_START( alieninv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "alieninv.h",   0x0000, 0x0800, CRC(6ad601c3) SHA1(9fc88698f98ce43992a5044d28d3e19751f82772) )
	ROM_LOAD( "alieninv.g",   0x0800, 0x0800, CRC(c6bb6fb3) SHA1(01a12163309f967dcffce19890b1e0d079021fc2) )
	ROM_LOAD( "alieninv.f",   0x1000, 0x0800, CRC(1d2ff324) SHA1(209766a981fdd3a68e36da3d8122a244c883cae7) )
	ROM_LOAD( "alieninv.e",   0x1800, 0x0800, CRC(2f2e6791) SHA1(08a1f17bcfec598182386f1c43e4fc7b476212de) )
ROM_END

ROM_START( alieninvp2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1h.bin",       0x0000, 0x0800, CRC(c46df7f4) SHA1(eec34b3d5585bae03c7b80585daaa05ddfcc2164) )
	ROM_LOAD( "1g.bin",       0x0800, 0x0800, CRC(4b1112d6) SHA1(b693667656e5d8f44eeb2ea730f4d4db436da579) )
	ROM_LOAD( "1f.bin",       0x1000, 0x0800, CRC(adca18a5) SHA1(7e02651692113db31fd469868ae5ffdb0f941ecf) )
	ROM_LOAD( "1e.bin",       0x1800, 0x0800, CRC(0449cb52) SHA1(8adcb7cd4492fa6649d9ee81172d8dff56621d64) )
ROM_END

ROM_START( sitv1 ) // rev 1
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tv01.s1",      0x0000, 0x0800, CRC(9f37b146) SHA1(0b7ef79dbc3de3beeae3bf222d086b60249d429f) )
	ROM_LOAD( "tv02.rp1",     0x0800, 0x0800, CRC(3c759a90) SHA1(d847d592dee592b1d3a575c21d89eaf3f7f6ae1b) )
	ROM_LOAD( "tv03.n1",      0x1000, 0x0800, CRC(0ad3657f) SHA1(a501f316535c50f7d7a20ef8e6dede1526a3f2a8) )
	ROM_LOAD( "tv04.m1",      0x1800, 0x0800, CRC(cd2c67f6) SHA1(60f9d8fe2d36ff589277b607f07c1edc917c755c) )
ROM_END

ROM_START( sitv ) // rev 2, minor bug fixes of sitv1; delay when writing to sound latch 0x05, and another unknown change
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tv0h.s1",      0x0000, 0x0800, CRC(fef18aad) SHA1(043edeefe6a6d4934bd384eafea19326de1dbeec) )
	ROM_LOAD( "tv02.rp1",     0x0800, 0x0800, CRC(3c759a90) SHA1(d847d592dee592b1d3a575c21d89eaf3f7f6ae1b) )
	ROM_LOAD( "tv03.n1",      0x1000, 0x0800, CRC(0ad3657f) SHA1(a501f316535c50f7d7a20ef8e6dede1526a3f2a8) )
	ROM_LOAD( "tv04.m1",      0x1800, 0x0800, CRC(cd2c67f6) SHA1(60f9d8fe2d36ff589277b607f07c1edc917c755c) )
ROM_END

ROM_START( sicv ) // likely not the first sicv version...
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cv17.36",     0x0000, 0x0800, CRC(3dfbe9e6) SHA1(26487df7fa0bbd0b9b7f74347c4b9318b0a73b89) )
	ROM_LOAD( "cv18.35",     0x0800, 0x0800, CRC(bc3c82bf) SHA1(33e39fc97bd46699be1f9b9741a86f433efdc911) )
	ROM_LOAD( "cv19.34",     0x1000, 0x0800, CRC(d202b41c) SHA1(868fe938ef768655c894ec95b7d9a81bf21f69ca) )
	ROM_LOAD( "cv20.33",     0x1800, 0x0800, CRC(c74ee7b6) SHA1(4f52db274a2d4433ab67c099ee805e8eb8516c0f) )

	ROM_REGION( 0x0800, "proms", 0 )        /* color maps player 1/player 2 */
	ROM_LOAD( "cv01.1",      0x0000, 0x0400, CRC(037e16ac) SHA1(d585030aaff428330c91ae94d7cd5c96ebdd67dd) )
	ROM_LOAD( "cv02.2",      0x0400, 0x0400, CRC(8263da38) SHA1(2e7c769d129e6f8a1a31eba1e02777bb94ac32b2) )
ROM_END

ROM_START( sisv1 ) // rev 1, this version may or may not really exist (may have been test/prototype only?)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sv01.36",     0x0000, 0x0400, CRC(d0c32d72) SHA1(b3bd950b1ba940fbeb5d95e55113ed8f4c311434) )
	ROM_LOAD( "sv02.35",     0x0400, 0x0400, CRC(0e159534) SHA1(94b2015a9d38ca738705b8d024a79fd2f9855b98) )
	ROM_LOAD( "sv03.34",     0x0800, 0x0400, NO_DUMP )
	ROM_LOAD( "sv04.31",     0x1400, 0x0400, CRC(1293b826) SHA1(165cd5d08a19eadbe954145b12807f10df9e691a) )
	ROM_LOAD( "sv05.42",     0x1800, 0x0400, NO_DUMP )
	ROM_LOAD( "sv06.41",     0x1c00, 0x0400, CRC(2c68e0b4) SHA1(a5e5357120102ad32792bf3ef6362f45b7ba7070) )
ROM_END

ROM_START( sisv2 ) // rev 2
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sv01.36",     0x0000, 0x0400, CRC(d0c32d72) SHA1(b3bd950b1ba940fbeb5d95e55113ed8f4c311434) )
	ROM_LOAD( "sv02.35",     0x0400, 0x0400, CRC(0e159534) SHA1(94b2015a9d38ca738705b8d024a79fd2f9855b98) )
	ROM_LOAD( "sv10.34",     0x0800, 0x0400, CRC(483e651e) SHA1(ae795ee3bc53ac3936f6cf2c72cca7a890783513) )
	ROM_LOAD( "sv04.31",     0x1400, 0x0400, CRC(1293b826) SHA1(165cd5d08a19eadbe954145b12807f10df9e691a) )
	ROM_LOAD( "sv09.42",     0x1800, 0x0400, CRC(cd80b13f) SHA1(0f4b9537b99fe3cdeebe525efb1869a1be0bc704) )
	ROM_LOAD( "sv06.41",     0x1c00, 0x0400, CRC(2c68e0b4) SHA1(a5e5357120102ad32792bf3ef6362f45b7ba7070) )
ROM_END

ROM_START( sisv3 ) // rev 3
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sv0h.36",     0x0000, 0x0400, CRC(86bb8cb6) SHA1(a75648e7f2446c756d86624b15d387d25ce47b66) )
	ROM_LOAD( "sv02.35",     0x0400, 0x0400, CRC(0e159534) SHA1(94b2015a9d38ca738705b8d024a79fd2f9855b98) )
	ROM_LOAD( "sv10.34",     0x0800, 0x0400, CRC(483e651e) SHA1(ae795ee3bc53ac3936f6cf2c72cca7a890783513) )
	ROM_LOAD( "sv04.31",     0x1400, 0x0400, CRC(1293b826) SHA1(165cd5d08a19eadbe954145b12807f10df9e691a) )
	ROM_LOAD( "sv09.42",     0x1800, 0x0400, CRC(cd80b13f) SHA1(0f4b9537b99fe3cdeebe525efb1869a1be0bc704) )
	ROM_LOAD( "sv06.41",     0x1c00, 0x0400, CRC(2c68e0b4) SHA1(a5e5357120102ad32792bf3ef6362f45b7ba7070) )
ROM_END

ROM_START( sisv ) // rev 4, with 5-digit scoring
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sv0h.36",     0x0000, 0x0400, CRC(86bb8cb6) SHA1(a75648e7f2446c756d86624b15d387d25ce47b66) )
	ROM_LOAD( "sv11.35",     0x0400, 0x0400, CRC(febe6d1a) SHA1(e1c3a24b4fa5862107ada1f9d7249466e8c3f06a) )
	ROM_LOAD( "sv12.34",     0x0800, 0x0400, CRC(a08e7202) SHA1(de9f7c851d1b894915e720cfc5d794cdb31752f6) )
	ROM_LOAD( "sv04.31",     0x1400, 0x0400, CRC(1293b826) SHA1(165cd5d08a19eadbe954145b12807f10df9e691a) )
	ROM_LOAD( "sv13.42",     0x1800, 0x0400, CRC(a9011634) SHA1(1f1369ecb02078042cfdf17a497b8dda6dd23793) )
	ROM_LOAD( "sv14.41",     0x1c00, 0x0400, CRC(58730370) SHA1(13dc806bcecd2d6089a85dd710ac2869413f7475) )
ROM_END

ROM_START( spacerng )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sr1.u36",    0x0000, 0x0800, CRC(b984f52d) SHA1(fdc8b249c0b65339977f91b674bdcb435aa99474) )
	ROM_LOAD( "sr2.u35",    0x0800, 0x0800, CRC(4b4f07e6) SHA1(408dcdae3e80a09584d8ebd6491bc90c4def1fcf) )
	ROM_LOAD( "sr3.u34",    0x1000, 0x0800, CRC(edc28ba9) SHA1(c96668f709d3fa0b97a6b118614e9c139f8f54cc) )
	ROM_LOAD( "sr4.u33",    0x1800, 0x0800, CRC(a95f559f) SHA1(f597c7af96a9d039fd8e54d976d68a065f6bf0c8) )

	ROM_REGION( 0x0800, "proms", 0 )        /* color maps player 1/player 2 */
	/* !! not dumped yet, these were taken from sisv */
	// NOTE: SISV (L-shaped boardset) was not supposed to HAVE color proms and hence they are removed. Maybe this is the correct set for these?
	ROM_LOAD( "cv01.1",     0x0000, 0x0400, BAD_DUMP CRC(aac24f34) SHA1(ad110e776547fb48baac568bb50d61854537ca34) ) // sldh
	ROM_LOAD( "cv02.2",     0x0400, 0x0400, BAD_DUMP CRC(2bdf83a0) SHA1(01ffbd43964c41987e7d44816271308f9a70802b) ) // sldh
ROM_END

ROM_START( spceking )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "invaders.h",   0x0000, 0x0800, CRC(734f5ad8) SHA1(ff6200af4c9110d8181249cbcef1a8a40fa40b7f) )
	ROM_LOAD( "spcekng2",     0x0800, 0x0800, CRC(96dcdd42) SHA1(e18d7ffca92e863ef40e235b2be973d8c5879fdb) )
	ROM_LOAD( "spcekng3",     0x1000, 0x0800, CRC(95fc96ad) SHA1(38175edad0e538a1561cec8f7613f15ae274dd14) )
	ROM_LOAD( "spcekng4",     0x1800, 0x0800, CRC(54170ada) SHA1(1e8b3774355ec0d448f04805a917f4c1fe64bceb) )
ROM_END

ROM_START( spcewars )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sanritsu.1",   0x0000, 0x0400, CRC(ca331679) SHA1(5c362c3d1c721d293bcddbef4033533769c8f0e0) )
	ROM_LOAD( "sanritsu.2",   0x0400, 0x0400, CRC(48dc791c) SHA1(91a98205c83ca38961e6ba2ac43a41e6e8bc2675) )
	ROM_LOAD( "ic35.bin",     0x0800, 0x0800, CRC(40c2d55b) SHA1(b641b63046d242ad23911143ed840011fc98eaff) )
	ROM_LOAD( "sanritsu.5",   0x1000, 0x0400, CRC(77475431) SHA1(15a04a2655847ee462be65d1065d643c872bb47c) )
	ROM_LOAD( "sanritsu.6",   0x1400, 0x0400, CRC(392ef82c) SHA1(77c98c11ee727ed3ed6e118f13d97aabdb555540) )
	ROM_LOAD( "sanritsu.7",   0x1800, 0x0400, CRC(b3a93df8) SHA1(3afc96814149d4d5343fe06eac09f808384d02c4) )
	ROM_LOAD( "sanritsu.8",   0x1c00, 0x0400, CRC(64fdc3e1) SHA1(c3c278bc236ced7fc85e1a9b018e80be6ab33402) )
	ROM_LOAD( "sanritsu.9",   0x4000, 0x0400, CRC(b2f29601) SHA1(ce855e312f50df7a74682974803cb4f9b2d184f3) )
ROM_END

ROM_START( spacewr3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic36.bin",     0x0000, 0x0800, CRC(9e30f88a) SHA1(314dfb2920d9b43b977cc19e40ac315e6933c3b9) )
	ROM_LOAD( "ic35.bin",     0x0800, 0x0800, CRC(40c2d55b) SHA1(b641b63046d242ad23911143ed840011fc98eaff) )
	ROM_LOAD( "ic34.bin",     0x1000, 0x0800, CRC(b435f021) SHA1(2d0d813b99d571b53770fa878a1f82ca67827caa) )
	ROM_LOAD( "ic33.bin",     0x1800, 0x0800, CRC(cbdc6fe8) SHA1(63038ea09d320c54e3d1cf7f043c17bba71bf13c) )
	ROM_LOAD( "ic32.bin",     0x4000, 0x0800, CRC(1e5a753c) SHA1(5b7cd7b347203f4edf816f02c366bd3b1b9517c4) )
ROM_END

ROM_START( invaderl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c01",          0x0000, 0x0400, CRC(499f253a) SHA1(e13353194277f5d35e92db9b11912b5f392f51b7) )
	ROM_LOAD( "c02",          0x0400, 0x0400, CRC(2d0b2e1f) SHA1(2e0262d9dba607824fcd720d2995531649bdd03d) )
	ROM_LOAD( "c03",          0x0800, 0x0400, CRC(03033dc2) SHA1(87d7838e6a6542c2c5510af593df45137cb397c6) )
	ROM_LOAD( "c07",          0x1000, 0x0400, CRC(5a7bbf1f) SHA1(659f2a8c646660d316d6e70f1d9548375f1da63f) )
	ROM_LOAD( "c04",          0x1400, 0x0400, CRC(455b1fa7) SHA1(668800a0a3ba18d8b54c2aa4dfd4bd01a667d679) )
	ROM_LOAD( "c05",          0x1800, 0x0400, CRC(40cbef75) SHA1(15994ed8bb8ab8faed6198926873851062c9d95f) )
	ROM_LOAD( "sv06.bin",     0x1c00, 0x0400, CRC(2c68e0b4) SHA1(a5e5357120102ad32792bf3ef6362f45b7ba7070) )
ROM_END

ROM_START( invader4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "spin4.a",      0x0000, 0x0800, CRC(bb386dfe) SHA1(cc00f3e4f6ca4c05bae038a24ccdb213fb951cfc) )
	ROM_LOAD( "spin4.b",      0x0800, 0x0800, CRC(63afa11d) SHA1(d8cedfa010a49237e31f6ebaed35134cb1c3ce68) )
	ROM_LOAD( "spin4.c",      0x1000, 0x0800, CRC(22b0317c) SHA1(8fd037bf5f89a7bcb06042697410566d5180912a) )
	ROM_LOAD( "spin4.d",      0x1800, 0x0800, CRC(9102fd68) SHA1(3523e69314844fcd1863b1e9a9d7fcebe9ee174b) )
ROM_END

ROM_START( jspecter )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3305.u6",      0x0000, 0x1000, CRC(ab211a4f) SHA1(d675ed29c3479d7318f8559bd56dd619cf631b6a) )
	ROM_LOAD( "3306.u7",      0x1400, 0x1000, CRC(0df142a7) SHA1(2f1c32d6fe7eafb7808fef0bdeb69b4909427417) )
ROM_END

ROM_START( jspecter2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "unksi.b2",     0x0000, 0x1000, CRC(0584b6c4) SHA1(c130021b878bde2beda4a189f71bbfed61088535) )
	ROM_LOAD( "unksi.a2",     0x1400, 0x1000, CRC(58095955) SHA1(545df3bb9ee4ff09f491d7a4b704e31aa311a8d7) )
ROM_END

ROM_START( invadpt2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pv01",        0x0000, 0x0800, CRC(7288a511) SHA1(ff617872784c28ed03591aefa9f0519e5651701f) )
	ROM_LOAD( "pv02",        0x0800, 0x0800, CRC(097dd8d5) SHA1(8d68654d54d075c0f0d7f63c87ff4551ce8b7fbf) )
	ROM_LOAD( "pv03",        0x1000, 0x0800, CRC(1766337e) SHA1(ea959bf06c9930d83a07559e191a28641efb07ac) )
	ROM_LOAD( "pv04",        0x1800, 0x0800, CRC(8f0e62e0) SHA1(a967b155f15f8432222fcc78b23121b00c405c5c) )
	ROM_LOAD( "pv05",        0x4000, 0x0800, CRC(19b505e9) SHA1(6a31a37586782ce421a7d2cffd8f958c00b7b415) )

	ROM_REGION( 0x0800, "proms", 0 )        /* color maps player 1/player 2 */
	ROM_LOAD( "pv06.1",   0x0000, 0x0400, CRC(a732810b) SHA1(a5fabffa73ca740909e23b9530936f9274dff356) )
	ROM_LOAD( "pv07.2",   0x0400, 0x0400, CRC(2c5b91cb) SHA1(7fa4d4aef85473b1b4f18734230c164e72be44e7) )
ROM_END

ROM_START( invadpt2br )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pv01",        0x0000, 0x0800, CRC(7288a511) SHA1(ff617872784c28ed03591aefa9f0519e5651701f) )
	/* pv01 had weird encryption applied to it, very likely to have been done post-dump. */
//  for (offs = 0x4fc; offs < 0x5fc; offs++)
//      rom[offs] ^= 0x6c;

	// 0x4fc + 1 * 0x56
//  for (offs = 0x54e; offs < 0x552; offs++)
//      rom[offs] ^= 0x03;

	// 0x4fc + 2 * 0x56
//  for (offs = 0x5a4; offs < 0x5a8; offs++)
//      rom[offs] ^= 0x01;

	// 0x4fc + 3 * 0x56
//  for (offs = 0x5fa; offs < 0x5fc; offs++)
//      rom[offs] ^= 0x02;

	ROM_LOAD( "br_pv02",     0x0800, 0x0800, CRC(420c7c35) SHA1(b51265f4d9e5a8cf9d53099a97cadd25ea0b34ce) )
	ROM_LOAD( "br_pv03",     0x1000, 0x0800, CRC(dffd04b9) SHA1(d51a0f27e90b0a49cf2d57ec82a863dcae9f3ea4) )
	ROM_LOAD( "br_pv04",     0x1800, 0x0800, CRC(b0626aff) SHA1(b7de6c21030732bd0479228f057ca4c87b913b0a) )
	ROM_LOAD( "br_pv05",     0x4000, 0x0800, CRC(84c70bb8) SHA1(75fef3ee6da3e7e01a257629016bc10a23691d62) )

	ROM_REGION( 0x0800, "proms", 0 )        /* color maps player 1/player 2 (taken from parent set) */
	ROM_LOAD( "pv06.1",   0x0000, 0x0400, CRC(a732810b) SHA1(a5fabffa73ca740909e23b9530936f9274dff356) )
	ROM_LOAD( "pv07.2",   0x0400, 0x0400, CRC(2c5b91cb) SHA1(7fa4d4aef85473b1b4f18734230c164e72be44e7) )
ROM_END

ROM_START( invaddlx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "invdelux.h",   0x0000, 0x0800, CRC(e690818f) SHA1(0860fb03a64d34a9704a1459a5e96929eafd39c7) )
	ROM_LOAD( "invdelux.g",   0x0800, 0x0800, CRC(4268c12d) SHA1(df02419f01cf0874afd1f1aa16276751acd0604a) )
	ROM_LOAD( "invdelux.f",   0x1000, 0x0800, CRC(f4aa1880) SHA1(995d77b67cb4f2f3781c2c8747cb058b7c1b3412) )
	ROM_LOAD( "invdelux.e",   0x1800, 0x0800, CRC(408849c1) SHA1(f717e81017047497a2e9f33f0aafecfec5a2ed7d) )
	ROM_LOAD( "invdelux.d",   0x4000, 0x0800, CRC(e8d5afcd) SHA1(91fde9a9e7c3dd53aac4770bd169721a79b41ed1) )
ROM_END

/* Runs on a Space Invaders Part II boardset with an epoxy module in place of the 8080 CPU */
ROM_START( vortex )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.t36",        0x0000, 0x0800, CRC(577417a6) SHA1(13ed1b989b8ea27cea88be7872921ff9283b5dd6) )
	ROM_LOAD( "2.t35",        0x0800, 0x0800, CRC(126d0049) SHA1(4c189a2364bca8682543d605e84d458bf81ee489) )
	ROM_LOAD( "3.t34",        0x1000, 0x0800, CRC(4a2510b3) SHA1(1c62583b7baf8ee2b6014a6e5dfc7e2d516886d1) )
	ROM_LOAD( "4.t33",        0x1800, 0x0800, CRC(da0274fe) SHA1(b8ab1b16d66700f9ca6a2380a5b6796eaef6e1bd) )
	ROM_LOAD( "5.t32",        0x4000, 0x0800, CRC(a3de49d6) SHA1(e302c6fd2705c6e7f9125b52b2dcb034cc88a90e) )
	ROM_LOAD( "6.t31",        0x4800, 0x0800, CRC(271085d0) SHA1(a772cec8135bc746f6c56aa294eb22c0604e16f9) )
ROM_END


ROM_START( moonbase )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ze3-1.a4",     0x0000, 0x0400, CRC(82dbf2c7) SHA1(c767d8b866db4a5059bd79f962a90ce3a962e1e6) )
	ROM_LOAD( "ze3-2.c4",     0x0400, 0x0400, CRC(c867f5b4) SHA1(686318fda6edde297aecaf33f480bfa075fa6eca) )
	ROM_LOAD( "ze3-3.e4",     0x0800, 0x0400, CRC(cb23ccc1) SHA1(86be2d14d52b3404e1a25c573bd25b97729d82a1) )
	ROM_LOAD( "ze3-4.f4",     0x0c00, 0x0400, CRC(9a11abe2) SHA1(f5337183c7f279d75ddeeab24f4f132aa2ee103b) ) // 'Taito Corp' string hidden in ROM
	ROM_LOAD( "ze3-5.h4",     0x1000, 0x0400, CRC(2b105ed3) SHA1(fa0767089b3aaec25be39e950e7163ecbdc2f39f) )
	ROM_LOAD( "ze3-6.l4",     0x1400, 0x0400, CRC(cb3d6dcb) SHA1(b4923b12a141c76b7d50274f19a3224db26a5669) )
	ROM_LOAD( "ze3-7.a5",     0x1800, 0x0400, CRC(774b52c9) SHA1(ddbbba874ac069fb930b364a890c45675ec389f7) )
	ROM_LOAD( "ze3-8.c5",     0x1c00, 0x0400, CRC(e88ea83b) SHA1(ef05be4783c860369ee5ecd4844837207e99ad9f) )
	ROM_LOAD( "ze3-9.e5",     0x4000, 0x0400, CRC(2dd5adfa) SHA1(62cb98cad1e48de0e0cbf30392d35834b38dadbd) )
	ROM_LOAD( "ze3-10.f5",    0x4400, 0x0400, CRC(1e7c22a4) SHA1(b34173375494ffbf5400dd4014a683a9807f4f08) )

	ROM_REGION( 0x0800, "proms", 0 )        /* color maps player 1/player 2 */
	ROM_LOAD( "cv02.h7",      0x0000, 0x0400, CRC(2bdf83a0) SHA1(01ffbd43964c41987e7d44816271308f9a70802b) ) /* NEC B406 or compatible BPROM, like the 82S137 */
	ROM_LOAD( "cv01.g7",      0x0400, 0x0400, CRC(aac24f34) SHA1(ad110e776547fb48baac568bb50d61854537ca34) ) /* NEC B406 or compatible BPROM, like the 82S137 */
ROM_END

ROM_START( moonbasea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ze3-1.a4",     0x0000, 0x0400, CRC(82dbf2c7) SHA1(c767d8b866db4a5059bd79f962a90ce3a962e1e6) )
	ROM_LOAD( "ze3-2.c4",     0x0400, 0x0400, CRC(c867f5b4) SHA1(686318fda6edde297aecaf33f480bfa075fa6eca) )
	ROM_LOAD( "ze3-3.e4",     0x0800, 0x0400, CRC(cb23ccc1) SHA1(86be2d14d52b3404e1a25c573bd25b97729d82a1) )
	ROM_LOAD( "ze3-4_alt.f4", 0x0c00, 0x0400, CRC(86a00411) SHA1(f518f5098512d6d23a8887605707844c1b32e54f) ) // 'Nichibutsu' string hidden in ROM
	ROM_LOAD( "ze3-5.h4",     0x1000, 0x0400, CRC(2b105ed3) SHA1(fa0767089b3aaec25be39e950e7163ecbdc2f39f) )
	ROM_LOAD( "ze3-6.l4",     0x1400, 0x0400, CRC(cb3d6dcb) SHA1(b4923b12a141c76b7d50274f19a3224db26a5669) )
	ROM_LOAD( "ze3-7.a5",     0x1800, 0x0400, CRC(774b52c9) SHA1(ddbbba874ac069fb930b364a890c45675ec389f7) )
	ROM_LOAD( "ze3-8.c5",     0x1c00, 0x0400, CRC(e88ea83b) SHA1(ef05be4783c860369ee5ecd4844837207e99ad9f) )
	ROM_LOAD( "ze3-9.e5",     0x4000, 0x0400, CRC(2dd5adfa) SHA1(62cb98cad1e48de0e0cbf30392d35834b38dadbd) )
	ROM_LOAD( "ze3-10.f5",    0x4400, 0x0400, CRC(1e7c22a4) SHA1(b34173375494ffbf5400dd4014a683a9807f4f08) )

	ROM_REGION( 0x0800, "proms", 0 )        /* color maps player 1/player 2 */
	ROM_LOAD( "cv02.h7",      0x0000, 0x0400, CRC(2bdf83a0) SHA1(01ffbd43964c41987e7d44816271308f9a70802b) ) /* NEC B406 or compatible BPROM, like the 82S137 */
	ROM_LOAD( "cv01.g7",      0x0400, 0x0400, CRC(aac24f34) SHA1(ad110e776547fb48baac568bb50d61854537ca34) ) /* NEC B406 or compatible BPROM, like the 82S137 */
ROM_END


ROM_START( invrvnge ) // Space Invaders hw + sound daughterboard
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "h.ic36",      0x0000, 0x0800, CRC(0e229b9f) SHA1(617197bf94e9700cbbb2f32487dc47b318d4f2af) )
	ROM_LOAD( "g.ic35",      0x0800, 0x0800, CRC(26b38aa4) SHA1(f281c7ec47ce6ab61bfda2e7aa6a5b8a01f2c11e) )
	ROM_LOAD( "f.ic34",      0x1000, 0x0800, CRC(b3b2749e) SHA1(4f854f981396e2d6a959dd48cff12234074fb69b) )
	ROM_LOAD( "e.ic33",      0x1800, 0x0800, CRC(d8e75102) SHA1(86d5618944265947e3ce60fdf048d8fff4a55744) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // encrypted
	ROM_LOAD( "snd.2c",      0xf000, 0x0800, CRC(135f3b16) SHA1(d472a6ca32c4a16cc1faf09f4a4876d75cd4ba24) )
	ROM_LOAD( "snd.1c",      0xf800, 0x0800, CRC(152fc85e) SHA1(df207d6e690287a56e4e330deaa5ee40a179f1fc) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "colour.bin",  0x0000, 0x0800, CRC(7de74988) SHA1(0b8c94b2bfdbc3921d60aad765df8af611f3fdd7) )
ROM_END

ROM_START( invrvngea ) // Space Invaders hw + sound daughterboard
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "h.ic36",      0x0000, 0x0800, CRC(0914b279) SHA1(91e465f56ed0dc8c68e109e33ec9d2bda2616a21) ) // sldh
	ROM_LOAD( "g.ic35",      0x0800, 0x0800, CRC(84d9497c) SHA1(fb1b5fc49365fbf89e5418789e64efd186cdeecf) ) // sldh
	ROM_LOAD( "f.ic34",      0x1000, 0x0800, CRC(78d34d97) SHA1(a50c19df12e75c644b014d74a463094e249db207) ) // sldh
	ROM_LOAD( "e.ic33",      0x1800, 0x0800, CRC(30c71887) SHA1(17c9e905eb327435d52b6d51842f7f42a5e6ab7d) ) // sldh

	ROM_REGION( 0x10000, "audiocpu", 0 ) // encrypted
	ROM_LOAD( "snd.2c",      0xf000, 0x0800, CRC(135f3b16) SHA1(d472a6ca32c4a16cc1faf09f4a4876d75cd4ba24) )
	ROM_LOAD( "snd.1c",      0xf800, 0x0800, CRC(152fc85e) SHA1(df207d6e690287a56e4e330deaa5ee40a179f1fc) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "colour.bin",  0x0000, 0x0800, CRC(7de74988) SHA1(0b8c94b2bfdbc3921d60aad765df8af611f3fdd7) )
ROM_END

ROM_START( invrvngeb ) // source unknown
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "invrvnge.h",  0x0000, 0x0800, CRC(aca41bbb) SHA1(ca71f792abd6d9a44d15b19d2ccf678e82ccba4f) )
	ROM_LOAD( "invrvnge.g",  0x0800, 0x0800, CRC(cfe89dad) SHA1(218b6a0b636c49c4cdc3667e8b1387ef0e257115) )
	ROM_LOAD( "invrvnge.f",  0x1000, 0x0800, CRC(e350de2c) SHA1(e845565e2f96f9dec3242ec5ab75910a515428c9) )
	ROM_LOAD( "invrvnge.e",  0x1800, 0x0800, CRC(1ec8dfc8) SHA1(fc8fbe1161958f57c9f4ccbcab8a769184b1c562) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // encrypted
	ROM_LOAD( "snd.2c",      0xf000, 0x0800, BAD_DUMP CRC(135f3b16) SHA1(d472a6ca32c4a16cc1faf09f4a4876d75cd4ba24) ) // not dumped, taken from parent
	ROM_LOAD( "snd.1c",      0xf800, 0x0800, BAD_DUMP CRC(152fc85e) SHA1(df207d6e690287a56e4e330deaa5ee40a179f1fc) ) // not dumped, taken from parent

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "colour.bin",  0x0000, 0x0800, BAD_DUMP CRC(7de74988) SHA1(0b8c94b2bfdbc3921d60aad765df8af611f3fdd7) ) // not dumped, taken from parent
ROM_END

ROM_START( invrvngedu ) // single PCB
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ir.5m",       0x0000, 0x0800, CRC(b145cb71) SHA1(127eb11de7ab9835f06510fb12838c0b728c0d42) )
	ROM_LOAD( "ir.5n",       0x0800, 0x0800, CRC(660e8af3) SHA1(bd52eadf4ee3d717fd5bd7206e1e87d729250c92) )
	ROM_LOAD( "ir.5p",       0x1000, 0x0800, CRC(6ec5a9ad) SHA1(d1e84d2d60c6128c092f2cd20a2b87216df3034b) )
	ROM_LOAD( "ir.5r",       0x1800, 0x0800, CRC(74516811) SHA1(0f595c7b0fae5f3f83fdd1ffed5a408ee77c9438) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // encrypted
	ROM_LOAD( "ir.1t",       0xf000, 0x0800, BAD_DUMP CRC(135f3b16) SHA1(d472a6ca32c4a16cc1faf09f4a4876d75cd4ba24) ) // not dumped, taken from parent
	ROM_LOAD( "ir.1u",       0xf800, 0x0800, BAD_DUMP CRC(152fc85e) SHA1(df207d6e690287a56e4e330deaa5ee40a179f1fc) ) // not dumped, taken from parent

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "ir.3r",       0x0000, 0x0800, CRC(57da51a9) SHA1(a8cb0b45c52eef353b83fe75b61e4990e27eb124) )
ROM_END

ROM_START( invrvngegw ) // single PCB
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ir.5m",       0x0000, 0x0800, CRC(4fe35d1f) SHA1(469d563f88229cf163f8b21dce9e68f75d3d214e) ) // sldh
	ROM_LOAD( "ir.5n",       0x0800, 0x0800, CRC(92d0442c) SHA1(1d104fbb225ce1a3a72e47af396a641030d990c2) ) // sldh
	ROM_LOAD( "ir.5p",       0x1000, 0x0800, CRC(18d2372d) SHA1(d19b7bd315226ef0a565b296964b221fa4714413) ) // sldh
	ROM_LOAD( "ir.5r",       0x1800, 0x0800, CRC(657ddf27) SHA1(957c6bbdb2133d4697d3302b2358979d1451b6d5) ) // sldh

	ROM_REGION( 0x10000, "audiocpu", 0 ) // encrypted
	ROM_LOAD( "ir.1t",       0xf000, 0x0800, CRC(64e9e81e) SHA1(3390f8bab219cf134b33ae21c473da0873e01929) ) // sldh - bad?
	ROM_LOAD( "ir.1u",       0xf800, 0x0800, CRC(152fc85e) SHA1(df207d6e690287a56e4e330deaa5ee40a179f1fc) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "ir.3r",       0x0000, 0x0800, CRC(6ce639bf) SHA1(73752f5886dcf8729d9853ddc258770f5c724ca3) ) // sldh
ROM_END


ROM_START( spclaser )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "la01",         0x0000, 0x0800, CRC(bedc0078) SHA1(a5bb0cbbb8e3f27d03beb8101b2be1111d73689d) )
	ROM_LOAD( "la02",         0x0800, 0x0800, CRC(43bc65c5) SHA1(5f9827c02c2d221e1607359c840374ff7fb92fbf) )
	ROM_LOAD( "la03",         0x1000, 0x0800, CRC(1083e9cc) SHA1(7ad45c6230c9e02fcf51e3414c15e2237eebbd7a) )
	ROM_LOAD( "la04",         0x1800, 0x0800, CRC(5116b234) SHA1(b165b2574cbcb26a5bb43f91df5f8be5f111f486) )

	ROM_REGION( 0x0800, "proms", 0 )        /* color maps player 1/player 2 */
	/* !! not dumped yet, these were taken from sisv/intruder */
	ROM_LOAD( "01.1",         0x0000, 0x0400, BAD_DUMP CRC(aac24f34) SHA1(ad110e776547fb48baac568bb50d61854537ca34) )
	ROM_LOAD( "02.2",         0x0400, 0x0400, BAD_DUMP CRC(2bdf83a0) SHA1(01ffbd43964c41987e7d44816271308f9a70802b) )
ROM_END

ROM_START( intruder )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "la01-1.36",    0x0000, 0x0800, CRC(bedc0078) SHA1(a5bb0cbbb8e3f27d03beb8101b2be1111d73689d) )
	ROM_LOAD( "la02-1.35",    0x0800, 0x0800, CRC(43bc65c5) SHA1(5f9827c02c2d221e1607359c840374ff7fb92fbf) )
	ROM_LOAD( "la03-1.34",    0x1000, 0x0800, CRC(278ef9cf) SHA1(74a9c1d3500ea28e50d07363a547c381999c84fa) )
	ROM_LOAD( "la04-1.33",    0x1800, 0x0800, CRC(5116b234) SHA1(b165b2574cbcb26a5bb43f91df5f8be5f111f486) )

	ROM_REGION( 0x0800, "proms", 0 )        /* color maps player 1/player 2 */
	ROM_LOAD( "01.1",         0x0000, 0x0400, CRC(aac24f34) SHA1(ad110e776547fb48baac568bb50d61854537ca34) )
	ROM_LOAD( "02.2",         0x0400, 0x0400, CRC(2bdf83a0) SHA1(01ffbd43964c41987e7d44816271308f9a70802b) )
ROM_END

ROM_START( laser )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.u36",        0x0000, 0x0800, CRC(b44e2c41) SHA1(00e0b2e088495d6f3bc175e8a53dcb3686ea8484) )
	ROM_LOAD( "2.u35",        0x0800, 0x0800, CRC(9876f331) SHA1(14e36b26d186d9a195492834ef989ed5664d7b65) )
	ROM_LOAD( "3.u34",        0x1000, 0x0800, CRC(ed79000b) SHA1(bfe0407e833ce61aa909f5f1f93c3fc1d46605e9) )
	ROM_LOAD( "4.u33",        0x1800, 0x0800, CRC(10a160a1) SHA1(e2d4208af11b65fc42d2856e57ee3c196f89d360) )

	ROM_REGION( 0x0800, "proms", 0 )        /* color maps player 1/player 2 */
	/* !! not dumped yet, these were taken from intruder */
	ROM_LOAD( "01.1",         0x0000, 0x0400, BAD_DUMP CRC(aac24f34) SHA1(ad110e776547fb48baac568bb50d61854537ca34) )
	ROM_LOAD( "02.2",         0x0400, 0x0400, BAD_DUMP CRC(2bdf83a0) SHA1(01ffbd43964c41987e7d44816271308f9a70802b) )
ROM_END

ROM_START( spcewarl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "spcewarl.1",   0x0000, 0x0800, CRC(1fcd34d2) SHA1(674139944e0d842a85bd21b326bd735e15453038) )
	ROM_LOAD( "spcewarl.2",   0x0800, 0x0800, CRC(43bc65c5) SHA1(5f9827c02c2d221e1607359c840374ff7fb92fbf) )
	ROM_LOAD( "spcewarl.3",   0x1000, 0x0800, CRC(7820df3a) SHA1(53315857f4282c68624b338b068d80ee6828af4c) )
	ROM_LOAD( "spcewarl.4",   0x1800, 0x0800, CRC(adc05b8d) SHA1(c4acf75537c0662a4785d5d6a90643239a54bf43) )

	ROM_REGION( 0x0800, "proms", 0 )        /* color maps player 1/player 2 */
	/* !! not dumped yet, these were taken from intruder */
	ROM_LOAD( "01.1",         0x0000, 0x0400, BAD_DUMP CRC(aac24f34) SHA1(ad110e776547fb48baac568bb50d61854537ca34) )
	ROM_LOAD( "02.2",         0x0400, 0x0400, BAD_DUMP CRC(2bdf83a0) SHA1(01ffbd43964c41987e7d44816271308f9a70802b) )
ROM_END

ROM_START( galxwars )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "univgw3.0",    0x0000, 0x0400, CRC(937796f4) SHA1(88e9494cc532498e51e3a68fa1122c40f22b27dd) )
	ROM_LOAD( "univgw4.1",    0x0400, 0x0400, CRC(4b86e7a6) SHA1(167f9f7491a2de39d08e3e6f7057cc75b36c9340) )
	ROM_LOAD( "univgw5.2",    0x0800, 0x0400, CRC(47a187cd) SHA1(640c896ba25f34d323624005bd676257ad17b687) )
	ROM_LOAD( "univgw6.3",    0x0c00, 0x0400, CRC(7b7d22ff) SHA1(74364cf2b04dcfbbc8e0131fa12c0e574f693d34) )
	ROM_LOAD( "univgw1.4",    0x4000, 0x0400, CRC(0871156e) SHA1(3726d0bfe153a0afc62ea56737662074986064b0) )
	ROM_LOAD( "univgw2.5",    0x4400, 0x0400, CRC(6036d7bf) SHA1(36c2ad2ffdb47bbecc40fd67ced6ab51a5cd2f3e) )

	ROM_REGION( 0x0800, "proms", 0 )        /* color maps player 1/player 2 */
	/* !! not dumped yet, these were taken from sisv/intruder */
	/* Or are colormaps generated by a group of TTLs, similar to dai3wksi? */
	ROM_LOAD( "01.1",         0x0000, 0x0400, BAD_DUMP CRC(aac24f34) SHA1(ad110e776547fb48baac568bb50d61854537ca34) )
	ROM_LOAD( "02.2",         0x0400, 0x0400, BAD_DUMP CRC(2bdf83a0) SHA1(01ffbd43964c41987e7d44816271308f9a70802b) )
ROM_END

ROM_START( galxwars2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3192.h6",      0x0000, 0x1000, CRC(bde6860b) SHA1(e04b8add32d8f7ea588fae6d6a387f1d40495f1b) )
	ROM_LOAD( "3193.h7",      0x4000, 0x1000, CRC(a17cd507) SHA1(554ab0e8bdc0e7af4a30b0ddc8aa053c8e70255c) ) /* 2nd half unused */

	ROM_REGION( 0x0800, "proms", 0 )        /* color maps player 1/player 2 */
	/* !! not dumped yet, these were taken from sisv/intruder */
	/* Or are colormaps generated by a group of TTLs, similar to dai3wksi? */
	ROM_LOAD( "01.1",         0x0000, 0x0400, BAD_DUMP CRC(aac24f34) SHA1(ad110e776547fb48baac568bb50d61854537ca34) )
	ROM_LOAD( "02.2",         0x0400, 0x0400, BAD_DUMP CRC(2bdf83a0) SHA1(01ffbd43964c41987e7d44816271308f9a70802b) )
ROM_END

ROM_START( galxwarst )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "galxwars.0",   0x0000, 0x0400, CRC(608bfe7f) SHA1(a41a40a2f0a1bb61a70b9ff8a7da925ab1db7f74) )
	ROM_LOAD( "galxwars.1",   0x0400, 0x0400, CRC(a810b258) SHA1(030a72fffcf240f643bc3006028cb4883cf58bbc) )
	ROM_LOAD( "galxwars.2",   0x0800, 0x0400, CRC(74f31781) SHA1(1de70e8ebbb26eea20ffedb7bd0ca051a67f45e7) )
	ROM_LOAD( "galxwars.3",   0x0c00, 0x0400, CRC(c88f886c) SHA1(4d705fbb97e3868c3f6c90c5e5753ad17cfbf5d6) )
	ROM_LOAD( "galxwars.4",   0x4000, 0x0400, CRC(ae4fe8fb) SHA1(494f44167dc84e4515b769c12f6e24419461dce4) )
	ROM_LOAD( "galxwars.5",   0x4400, 0x0400, CRC(37708a35) SHA1(df6fd521ddfa146ef93e390e47741bdbfda1e7ba) )

	ROM_REGION( 0x0800, "proms", 0 )        /* color maps player 1/player 2 */
	/* !! not dumped yet, these were taken from sisv/intruder */
	/* Or are colormaps generated by a group of TTLs, similar to dai3wksi? */
	ROM_LOAD( "01.1",         0x0000, 0x0400, BAD_DUMP CRC(aac24f34) SHA1(ad110e776547fb48baac568bb50d61854537ca34) )
	ROM_LOAD( "02.2",         0x0400, 0x0400, BAD_DUMP CRC(2bdf83a0) SHA1(01ffbd43964c41987e7d44816271308f9a70802b) )
ROM_END

ROM_START( starw )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "roma",         0x0000, 0x0400, CRC(60e8993c) SHA1(0bdf163ff0f2e6a8771987d4e7ac604c45af21b8) )
	ROM_LOAD( "romb",         0x0400, 0x0400, CRC(b8060773) SHA1(92aa358c338ef8f5773bccada8988d068764e7ea) )
	ROM_LOAD( "romc",         0x0800, 0x0400, CRC(307ce6b8) SHA1(f4b6f54db3d2377ec27d62d33fa1c4946559a092) )
	ROM_LOAD( "romd",         0x1400, 0x0400, CRC(2b0d0a88) SHA1(d079d12b6d4136519ded32415d668a02147b7601) )
	ROM_LOAD( "rome",         0x1800, 0x0400, CRC(5b1c3ad0) SHA1(edb42eec59c3dd7e274e2ea08fed0f3e8fc72e9e) )
	ROM_LOAD( "romf",         0x1c00, 0x0400, CRC(c8e42d3d) SHA1(841b27af251b9c3a964972e864fb7c88acc742e0) )
ROM_END

ROM_START( starw1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gc.75",        0x0000, 0x0400, CRC(ad10c128) SHA1(c30ff9ff5cf8dedf7654c8e2799a4bb79a30104a) )
	ROM_LOAD( "gc.77",        0x0400, 0x0400, CRC(ab77c474) SHA1(eb07dcad1f265834b93a8108298d4441d6a74b2e) )
	ROM_LOAD( "gc.76",        0x0800, 0x0400, CRC(3638aed4) SHA1(1426c9270f248fd2ab134dc35526599c02051ccd) )
	ROM_LOAD( "gc.80",        0x1400, 0x0400, CRC(4c67957b) SHA1(dda7bbd54e7395dea80d224e487318fb4429f027) )
	ROM_LOAD( "gc.81",        0x1800, 0x0400, CRC(246621ef) SHA1(bddc5253f735fa81266d725a24b1c14faabe0c6a) )
	ROM_LOAD( "gc.82",        0x1c00, 0x0400, CRC(19bd32ee) SHA1(a1718a6a6300c3d7df469793cb0d590c4a966aff) )

	ROM_REGION( 0x0800, "proms", 0 )        /* color maps player 1/player 2 */
	ROM_LOAD( "cv01",         0x0000, 0x0400, CRC(8d892ef3) SHA1(c471dd6197a3c779d89c33fcb425cf3bbdf4fc15) )
	ROM_IGNORE( 0x0400 )
	ROM_LOAD( "cv02",         0x0400, 0x0400, CRC(b44ddde8) SHA1(8793f370526c072e645d8d0b9794b1b64a7701ef) )
	ROM_IGNORE( 0x0400 )
ROM_END

ROM_START( lrescue )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lrescue.1",    0x0000, 0x0800, CRC(2bbc4778) SHA1(0167f1ac1501ab0b4c4e555023fa5efed59d56ae) )
	ROM_LOAD( "lrescue.2",    0x0800, 0x0800, CRC(49e79706) SHA1(bed675bb97d59ae0132c007ccead0d096ed2ddf1) )
	ROM_LOAD( "lrescue.3",    0x1000, 0x0800, CRC(1ac969be) SHA1(67ac47f45b9fa5c530bf6047bb7d5776b52847be) )
	ROM_LOAD( "lrescue.4",    0x1800, 0x0800, CRC(782fee3c) SHA1(668295e9d6d99084bb4e7c5491f00fe75f4f5a88) )
	ROM_LOAD( "lrescue.5",    0x4000, 0x0800, CRC(58fde8bc) SHA1(663665ac5254204c1eba18357d9867034eae55eb) )
	ROM_LOAD( "lrescue.6",    0x4800, 0x0800, CRC(bfb0f65d) SHA1(ea0943d764a16094b6e2289f62ef117c9f838c98) )

	ROM_REGION( 0x0800, "proms", 0 )        /* color map */
	ROM_LOAD( "7643-1.cpu",   0x0000, 0x0400, CRC(8b2e38de) SHA1(d6a757be31c3a179d31bd3709e71f9e38ec632e9) )
	ROM_RELOAD(               0x0400, 0x0400 )
ROM_END

/*

MOON LANDER
Manufacturer: Leisure Time Electronics
Year: 1980
Orientation: Vertical B/W
Cabinet: Cocktail


Leisure Time Electronics produced three games: Astro Laser, Moon Lander, and Space Ranger.
The games were designed to be interchangeable with a universal cocktail cabinet which they designed and named "Star Series". The cocktail cabinets share the same artwork with all three games and has a different instruction card for each game.
There were no upright or cabaret cabinets. The game ROMs operate on Taito pc boards.

Moon Lander is a clone/ripoff of Lunar Rescue/Destination Earth. I do not have a manual or schematics for this pcb although
I was able to confirm 5 out of 8 dipswitch settings. I was surprised to hear the sounds effects are almost exactly like when Lunar Rescue used the "invaders" external samples in MAME. The pcb does not play the invader "hit" sound for some reason.
I couldn't find anything obviously wrong in the sound section so it must be that it's just not hooked up as-is from the factory. There does not appear to be a sound-in-attract option.



CPU - Mitsubishi M5L8080AP
X-tal - 19968 mhz (decimal not shown, device stamped very lightly)
Sound - discrete, SN76477N
I/O board - Taito # CV070001A/CVN00001A label= serial# 172190
CPU board - Taito # AA017757   label= CVN00004 serial# 802868
ROM board - Taito # AA017756A  label= CVN00006 serial# 046120



EPROMs - 6x 2716
ML1.u36 checksum 0002CA52
ML2.u35 checksum 0002C999
ML3.u34 checksum 0002BD5C
ML4.u33 checksum 000252EE
ML5.u32 checksum 00029365
ML6.u31 checksum 0002C624



Dipswitch sw1, 8-bank


sw1 - # ships
sw2 - # ships
sw3 - not used/unknown
sw4 - not used/unknown
sw5 - ON= enable player2 move right  OFF= disabled
sw6 - ON= enable player2 move left   OFF= disabled
sw7 - ON= enable player2 fire/shoot  OFF= disabled
sw8 - not used/unknown
--------------------------------------------------------------------------
        1   2   3   4   5   6   7   8
--------------------------------------------------------------------------
# of player ships
- 3 ships   on  on
- 4 ships   off on
- 5 ships   on  off
- 6 ships   off off


Maximum Credits= 9

Sound Pots:
VR1 = engine sound
VR2 = beam gun
VR3 = ship explosion sound
VR4 = enemy explosion sound (not connected?)
VR5 = bonus ship sound
VR6 = bonus music, footsteps, and docking sound
VR7 = shooting star/ship descending sound
VR8 = pot for adjusting total sounds

*/

ROM_START( mlander )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ml1.u36",    0x0000, 0x0800, CRC(69df529a) SHA1(ded3b4a04e28dc341b1fc5a8880bc48aa332bdb5) )
	ROM_LOAD( "ml2.u35",    0x0800, 0x0800, CRC(3b503337) SHA1(d1056c0161d481202996811503e9970d0a0c9147) )
	ROM_LOAD( "ml3.u34",    0x1000, 0x0800, CRC(64e53458) SHA1(629f2434eea4d31dc9db0ee7bc8364cd2bf08a04) )
	ROM_LOAD( "ml4.u33",    0x1800, 0x0800, CRC(c9a74571) SHA1(b1671d19eff17f7adb274013c8f11eb044ebdd28) )
	ROM_LOAD( "ml5.u32",    0x4000, 0x0800, CRC(88291fa2) SHA1(40c4eb51f75b5ca81a62121231d22b9f48d0f628) )
	ROM_LOAD( "ml6.u31",    0x4800, 0x0800, CRC(bfb0f65d) SHA1(ea0943d764a16094b6e2289f62ef117c9f838c98) )

	ROM_REGION( 0x0800, "proms", 0 )        /* color map */
	ROM_LOAD( "01.bin",     0x0000, 0x0400, CRC(aac24f34) SHA1(ad110e776547fb48baac568bb50d61854537ca34) )
	ROM_LOAD( "02.bin",     0x0400, 0x0400, CRC(2bdf83a0) SHA1(01ffbd43964c41987e7d44816271308f9a70802b) )
ROM_END

ROM_START( grescue )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lrescue.1",    0x0000, 0x0800, CRC(2bbc4778) SHA1(0167f1ac1501ab0b4c4e555023fa5efed59d56ae) )
	ROM_LOAD( "lrescue.2",    0x0800, 0x0800, CRC(49e79706) SHA1(bed675bb97d59ae0132c007ccead0d096ed2ddf1) )
	ROM_LOAD( "lrescue.3",    0x1000, 0x0800, CRC(1ac969be) SHA1(67ac47f45b9fa5c530bf6047bb7d5776b52847be) )
	ROM_LOAD( "grescue.4",    0x1800, 0x0800, CRC(ca412991) SHA1(41b59f338a6c246e0942a8bfa3c0bca2c24c7f81) )
	ROM_LOAD( "grescue.5",    0x4000, 0x0800, CRC(a419a4d6) SHA1(8eeeb31cbebffc98d2c6c5b964f9b320fcf303d2) )
	ROM_LOAD( "lrescue.6",    0x4800, 0x0800, CRC(bfb0f65d) SHA1(ea0943d764a16094b6e2289f62ef117c9f838c98) )

	ROM_REGION( 0x0800, "proms", 0 )        /* color map */
	ROM_LOAD( "7643-1.cpu",   0x0000, 0x0400, CRC(8b2e38de) SHA1(d6a757be31c3a179d31bd3709e71f9e38ec632e9) )
	ROM_RELOAD(               0x0400, 0x0400 )
ROM_END

ROM_START( desterth )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "36_h.bin",     0x0000, 0x0800, CRC(f86923e5) SHA1(d19935ba3d2c1c2553b3779f1a7ad8856c003dae) )
	ROM_LOAD( "35_g.bin",     0x0800, 0x0800, CRC(797f440d) SHA1(a96917f2296ae467acc795eacc1533a2a2d2f401) )
	ROM_LOAD( "34_f.bin",     0x1000, 0x0800, CRC(993d0846) SHA1(6be0c45add41fa7e43cac96c776cd0ebb45ade7b) )
	ROM_LOAD( "33_e.bin",     0x1800, 0x0800, CRC(8d155fc5) SHA1(1ef5e62d71abbf870c027fa1e477121ff124b8da) )
	ROM_LOAD( "32_d.bin",     0x4000, 0x0800, CRC(3f531b6f) SHA1(2fc1f4912688986650e20a050a5d63ddecd4267e) )
	ROM_LOAD( "31_c.bin",     0x4800, 0x0800, CRC(ab019c30) SHA1(33931510a722168bcf7c30d22eac9345576b6631) )
	ROM_LOAD( "42_b.bin",     0x5000, 0x0800, CRC(ed9dbac6) SHA1(4553f445ac32ebb1be490b02df4924f76557e8f9) )

	ROM_REGION( 0x0800, "proms", 0 )        /* color map */
	ROM_LOAD( "7643-1.cpu",   0x0000, 0x0400, CRC(8b2e38de) SHA1(d6a757be31c3a179d31bd3709e71f9e38ec632e9) )
	ROM_RELOAD(               0x0400, 0x0400 )
ROM_END

ROM_START( lrescuem )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "48.ic36",    0x0000, 0x0400, CRC(bad5ba48) SHA1(6d8a2df172e058d16f196ad7f29430e9fd1fdaa8) )
	ROM_LOAD( "49.ic35",    0x0400, 0x0400, CRC(a6dc23d6) SHA1(76b9105935bf239ae90b47900f64dac3032ceecd) )
	ROM_LOAD( "50.ic34",    0x0800, 0x0400, CRC(90179fee) SHA1(35059f7399229b8d9588d34f79073fa4d3301614) )
	ROM_LOAD( "51.ic33",    0x0c00, 0x0400, CRC(1d197d87) SHA1(21e049f9c2a0fe1c0403d9d1a2dc695c4ee764f9) )
	ROM_LOAD( "52.ic32",    0x1000, 0x0400, CRC(4326d338) SHA1(ac31645bdf292f28dfcfcb9d5e158e5df7a6f95d) )
	ROM_LOAD( "53.ic31",    0x1400, 0x0400, CRC(3b272372) SHA1(39b807c810d093d7a34b102eec16f3d9baeb21f1) )
	ROM_LOAD( "54.ic42",    0x1800, 0x0400, CRC(a877c5b6) SHA1(862871c3dd18221d5713fe1fd2dc4f5b7cb913c1) )
	ROM_LOAD( "55.ic41",    0x1c00, 0x0400, CRC(c9a93407) SHA1(604bcace8e3bec07db6ca8a8918b306b77643e14) )
	ROM_LOAD( "56.ic40",    0x4000, 0x0400, CRC(3398798f) SHA1(d7dd9e65a1048df8edd217f4206b19cd01f143f4) )
	ROM_LOAD( "57.ic39",    0x4400, 0x0400, CRC(37c5bfc6) SHA1(b0aec85e6f979cdf7a3a985830c8530302804837) )
	ROM_LOAD( "58.ic38",    0x4800, 0x0400, CRC(1b7a5644) SHA1(d26530ea11ada86f7c99b11d6faf4416a8f5a9eb) )
	ROM_LOAD( "59.ic37",    0x4c00, 0x0400, CRC(c342b907) SHA1(327da029420c4eedabc2a0534199a008a3f341b8) )

	ROM_REGION( 0x0800, "proms", 0 )        /* color maps player 1/player 2 - these don't really fit this game, but were on the PCB */
	ROM_LOAD( "cv01-7643.2c",   0x0000, 0x0400, CRC(aac24f34) SHA1(ad110e776547fb48baac568bb50d61854537ca34) )
	ROM_LOAD( "cv02-7643.1c",   0x0400, 0x0400, CRC(2bdf83a0) SHA1(01ffbd43964c41987e7d44816271308f9a70802b) )
ROM_END

ROM_START( lrescuem2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "0.bin",    0x0000, 0x0800, CRC(27d37ad6) SHA1(18b2de9f9c022a31187b2a4049573e7f204e84c9) )
	ROM_LOAD( "1.bin",    0x0800, 0x0800, CRC(d8ed56f0) SHA1(d3f02d43f59d8ee83b4ed94f58f1bd25dca1a8de) )
	ROM_LOAD( "2.bin",    0x1000, 0x0800, CRC(3aed9788) SHA1(1be3c2f9f3a0f7d187a6faa2b020979027fa60e9) )
	ROM_LOAD( "3.bin",    0x1800, 0x0800, CRC(fa121b92) SHA1(2753b8b93d69d49e85075765630958038aa21ce3) )
	ROM_LOAD( "4.bin",    0x4000, 0x0800, CRC(535b4a78) SHA1(dd5613f47a3c7e15701c5d1dbac4a2228b9c28f2) )
	ROM_LOAD( "5.bin",    0x4800, 0x0800, CRC(0613a977) SHA1(47b85efdc436b39f8fb12355f9b87cb791f2d3b1) )
	ROM_LOAD( "6.bin",    0x5000, 0x0800, CRC(8fe51cc0) SHA1(1a98044ab95a1559362813a3961c1436267dcf63) )

	ROM_REGION( 0x0800, "proms", 0 )        /* color maps player 1/player 2 - these don't really fit this game, but were on the PCB */
	ROM_LOAD( "cv01-7643.2c",   0x0000, 0x0400, CRC(aac24f34) SHA1(ad110e776547fb48baac568bb50d61854537ca34) )
	ROM_LOAD( "cv02-7643.1c",   0x0400, 0x0400, CRC(2bdf83a0) SHA1(01ffbd43964c41987e7d44816271308f9a70802b) )
ROM_END


/*
Cosmo
TDS & Mints, 1979/80?

Notes:
This game runs on modified "original" Taito (3 board) Space Invaders hardware.
There are approx. 70 (or more) wires tied to various parts of the boards, plus
there is an extra board on top of the sound board with a *HUGE* amount of wires
running to it from the main boards. There are 2 EPROMs on the top board that appear
to be for use with colour generation or extra sounds(?) The PROMs on the middle board
have been removed and in their place are a pile of wires that join to the top board.
The remainder of the hardware is just standard Taito Space Invaders..... including
a SN76477 and the discrete components for sound generation.
Note that the sounds and gameplay of Cosmo are VERY different from Space Invaders.
*/

ROM_START( cosmo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.36",         0x0000, 0x0800, CRC(445c9a98) SHA1(89bce80a061e9c12544231f970d9dec801eb1b94) )
	ROM_LOAD( "2.35",         0x0800, 0x0800, CRC(df3eb731) SHA1(fb90c1d0f2518195dd49062c9f0fd890536d89f4) )
	ROM_LOAD( "3.34",         0x1000, 0x0800, CRC(772c813f) SHA1(a1c0d857c660fb0b838dd0466af7bf5d73bcd55d) )
	ROM_LOAD( "4.33",         0x1800, 0x0800, CRC(279f66e6) SHA1(8ce71c08cca0bdde2f2e0ef21622731c4610c030) )
	ROM_LOAD( "5.32",         0x4000, 0x0800, CRC(cefb18df) SHA1(bb500cf3f7d1a54045a165d3613a92ab3f11d3e8) )
	ROM_LOAD( "6.31",         0x4800, 0x0800, CRC(b037f6c4) SHA1(b9a42948052b8cda8d2e4575e59909589f4e7a8d) )
	ROM_LOAD( "7.42",         0x5000, 0x0800, CRC(c3831ea2) SHA1(8c67ef0312656ef0eeff34b8463376c736bd8ea1) )

	ROM_REGION( 0x1000, "proms", 0 )        /* color map */
	ROM_LOAD( "n-1.7d",       0x0800, 0x0800, CRC(bd8576f1) SHA1(aa5fe0a4d024f21a3bca7a6b3f5022779af6f3f4) )
	ROM_LOAD( "n-2.6e",       0x0000, 0x0800, CRC(48f1ade5) SHA1(a1b45f82f3649cde8ae6a2ef494a3a6cdb5e65d0) )
ROM_END


ROM_START( cosmicmo ) /*  Roms stamped with "II", denoting version II  */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ii-1.h1",   0x0000, 0x0400, CRC(d6e4e5da) SHA1(8b4275a3c71ac3fa80d17237dc04de5f586645f4) )
	ROM_LOAD( "ii-2.h2",   0x0400, 0x0400, CRC(8f7988e6) SHA1(b6a01d5dcab013350f8f7f3e3ebfc986bb939fe0) )
	ROM_LOAD( "ii-3.h3",   0x0800, 0x0400, CRC(2d2e9dc8) SHA1(dd3da4fc752e003e5e7c64bf189288133aed545b) )
	ROM_LOAD( "ii-4.h4",   0x0c00, 0x0400, CRC(26cae456) SHA1(2f2262340c10e5c29d71317f6eb8072c26655563) )
	ROM_LOAD( "ii-5.h5",   0x4000, 0x0400, CRC(b13f228e) SHA1(a0de05aa36435e72c77f5333f3ad964ec448a8f0) )
	ROM_LOAD( "ii-6.h6",   0x4400, 0x0400, CRC(4ae1b9c4) SHA1(8eed87eebe68caa775fa679363b0fe3728d98c34) )
	ROM_LOAD( "ii-7.h7",   0x4800, 0x0400, CRC(6a13b15b) SHA1(dc03a6c3e938cfd08d16bd1660899f951ba72ea2) )

	/* There is no colour circuits or tracking on the game pcb, its a black and white composite video signal only */
		/* The PCB is etched with Universal 7814A-3 */
ROM_END

ROM_START( cosmicm2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3907.bin",   0x0000, 0x1000, CRC(bbffede6) SHA1(e7505ee8e3f19557ebbfd0145dc2ae0d1c529eba) )
	ROM_LOAD( "3906.bin",   0x4000, 0x1000, CRC(b841f894) SHA1(b1f9e1800969baab14da2fd8873b58d4707b7236) )
ROM_END

ROM_START( superinv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "00",           0x0000, 0x0400, CRC(7a9b4485) SHA1(dde918ec106971972bf7c7e5085c1262522f7e35) )
	ROM_LOAD( "01",           0x0400, 0x0400, CRC(7c86620d) SHA1(9e92ec0aa4eee96a7fa115a14a611c488d13b9dd) )
	ROM_LOAD( "02",           0x0800, 0x0400, CRC(ccaf38f6) SHA1(8eb0456e8abdba0d1dda20a335a9ecbe7c38f9ed) )
	ROM_LOAD( "03",           0x1400, 0x0400, CRC(8ec9eae2) SHA1(48d7a7dc61e0417ca4093e5c2a36efd96e359233) )
	ROM_LOAD( "04",           0x1800, 0x0400, CRC(68719b30) SHA1(2084bd63cd61ef1d2497c32112cdb42b7b582da4) )
	ROM_LOAD( "05",           0x1c00, 0x0400, CRC(8abe2466) SHA1(17494b1e5db207e37a7d28d7c89cbc5f36b7aefc) )
ROM_END

ROM_START( invasion )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "10136-0.0k",   0x0000, 0x0400, CRC(7a9b4485) SHA1(dde918ec106971972bf7c7e5085c1262522f7e35) )
	ROM_LOAD( "10136-1.1k",   0x0400, 0x0400, CRC(7c86620d) SHA1(9e92ec0aa4eee96a7fa115a14a611c488d13b9dd) )
	ROM_LOAD( "10136-2.2k",   0x0800, 0x0400, CRC(ccaf38f6) SHA1(8eb0456e8abdba0d1dda20a335a9ecbe7c38f9ed) )
	ROM_LOAD( "10136-5.5k",   0x1400, 0x0400, CRC(8ec9eae2) SHA1(48d7a7dc61e0417ca4093e5c2a36efd96e359233) )
	ROM_LOAD( "10136-6.6k",   0x1800, 0x0400, CRC(ff0b0690) SHA1(8547c4b2a228f1690287217a916613c8f0caccf6) )
	ROM_LOAD( "10136-7.7k",   0x1c00, 0x0400, CRC(75d7acaf) SHA1(977d146d7df555cea1bb2156d29d88bec9731f98) )
ROM_END

ROM_START( invasiona )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "invasiona_0.bin",   0x0000, 0x0400, CRC(c2fe6197) SHA1(823d02c2790711f40c167544a55e1669a97d93b4) )
	ROM_LOAD( "invasiona_1.bin",   0x0400, 0x0400, CRC(7c86620d) SHA1(9e92ec0aa4eee96a7fa115a14a611c488d13b9dd) )
	ROM_LOAD( "invasiona_2.bin",   0x0800, 0x0400, CRC(ccaf38f6) SHA1(8eb0456e8abdba0d1dda20a335a9ecbe7c38f9ed) )
	ROM_LOAD( "invasiona_3.bin",   0x1400, 0x0400, CRC(8ec9eae2) SHA1(48d7a7dc61e0417ca4093e5c2a36efd96e359233) )
	ROM_LOAD( "invasiona_4.bin",   0x1800, 0x0400, CRC(24b39879) SHA1(c93530ac20c412b516fbcba8220d85a9bd4fa804) )
	ROM_LOAD( "invasiona_5.bin",   0x1c00, 0x0400, CRC(59134ff8) SHA1(2e6a040066b35b10f867a3e500e3b13922c0eb7a) )
ROM_END

ROM_START( invasionb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "invasionb_0.bin",   0x0000, 0x0400, CRC(7a9b4485) SHA1(dde918ec106971972bf7c7e5085c1262522f7e35) )
	ROM_LOAD( "invasionb_1.bin",   0x0400, 0x0400, CRC(7c86620d) SHA1(9e92ec0aa4eee96a7fa115a14a611c488d13b9dd) )
	ROM_LOAD( "invasionb_2.bin",   0x0800, 0x0400, CRC(ccaf38f6) SHA1(8eb0456e8abdba0d1dda20a335a9ecbe7c38f9ed) )
	ROM_LOAD( "invasionb_5.bin",   0x1400, 0x0400, CRC(8ec9eae2) SHA1(48d7a7dc61e0417ca4093e5c2a36efd96e359233) )
	ROM_LOAD( "invasionb_6.bin",   0x1800, 0x0400, CRC(ec0edb4a) SHA1(8c6946b50ba5c319fe03c55b43c4e714387719b8) )
	ROM_LOAD( "invasionb_7.bin",   0x1c00, 0x0400, CRC(6aac1281) SHA1(f071a21de72d2c9f7851195592c828fa501197ce) )
ROM_END

ROM_START( invasionrz )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rz0.0k",   0x0000, 0x0400, CRC(7a9b4485) SHA1(dde918ec106971972bf7c7e5085c1262522f7e35) )
	ROM_LOAD( "rz1.1k",   0x0400, 0x0400, CRC(7c86620d) SHA1(9e92ec0aa4eee96a7fa115a14a611c488d13b9dd) )
	ROM_LOAD( "rz2.2k",   0x0800, 0x0400, CRC(ccaf38f6) SHA1(8eb0456e8abdba0d1dda20a335a9ecbe7c38f9ed) )
	ROM_LOAD( "rz5.5k",   0x1400, 0x0400, CRC(8ec9eae2) SHA1(48d7a7dc61e0417ca4093e5c2a36efd96e359233) )
	ROM_LOAD( "rz6.6k",   0x1800, 0x0400, CRC(ec0edb4a) SHA1(8c6946b50ba5c319fe03c55b43c4e714387719b8) )
	ROM_LOAD( "rz7.7k",   0x1c00, 0x0400, CRC(e4ab9012) SHA1(4f54e3fd3e3835a7b7d3b8d77929f4d9e42a4917) )
ROM_END

ROM_START( invasionrza )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rz0-0.9k",   0x0000, 0x0400, CRC(3044806f) SHA1(7eaedd7fd7fcfd421432d5f6970ede12f586f644) )
	ROM_LOAD( "rz1-1.8k",   0x0400, 0x0400, CRC(7c86620d) SHA1(9e92ec0aa4eee96a7fa115a14a611c488d13b9dd) )
	ROM_LOAD( "rz2-2.7k",   0x0800, 0x0400, CRC(c808e941) SHA1(c17f2171d82df1984c4b048f2664dea5bd9c136b) )
	ROM_LOAD( "rz5-5.4k",   0x1400, 0x0400, CRC(8ec9eae2) SHA1(48d7a7dc61e0417ca4093e5c2a36efd96e359233) )
	// 2 reads of the bad rom
	ROM_LOAD( "rz6-6.3k",   0x1800, 0x0400, BAD_DUMP CRC(c48df3ca) SHA1(d92064d171e099a45821c944324b993e39d894f7) )
	ROM_LOAD( "rz6-6.3ka",  0x1800, 0x0400, BAD_DUMP CRC(aa51b2c3) SHA1(bb30f3827a66ec3cb8436566f6b865995d702f76) )
	ROM_LOAD( "rz7-7.2k",   0x1c00, 0x0400, CRC(27dbea48) SHA1(f0bf5d31424dc72ac2e6fa01c528365efff838d2) )
ROM_END

/*

Space Invaders (Electromar, Madrid) 1980

Board by Roselson
Dumped by Ricky2001 from Aumap

This game runs in a clone of a Midway L-Shape Space Invaders pcb with different connectors, but identical.
The board is updated with a litthe daughter board for the reset, instead of being generated in the Power supply.
Most of the Texts are in Spanish but keeps the original name "Space Invaders", also in the psb is writen a Patent number, I think this means it was a licensed version.

Boards Electromar 1007-A / 1007B
Patente N MU221718


*/

ROM_START( invadersem )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "h.bin",    0x0000, 0x0400, CRC(7fc672a5) SHA1(93c8dd27769e9c1ab812fd68031c67a5dc79d0da) )
	ROM_LOAD( "g.bin",    0x0400, 0x0400, CRC(ad518883) SHA1(8f7f1f520287b738ebb6f2c70b7da2cae5db2be8) )
	ROM_LOAD( "f.bin",    0x0800, 0x0400, CRC(f4a6c480) SHA1(eb179a46345d652ffd74f77956d361cebfbb1112))
	ROM_LOAD( "c.bin",    0x1400, 0x0400, CRC(8f62c513) SHA1(87570241d4ab7df3ef380d57d27055af3cca7845) )
	ROM_LOAD( "b.bin",    0x1800, 0x0400, CRC(2808e5c0) SHA1(aef4821d6d6e7f062e3ebecb878e6370b604224e) )
	ROM_LOAD( "a.bin",    0x1c00, 0x0400, CRC(04c9b084) SHA1(d267438589de2d8332410e9641164fe68f337f73) )
ROM_END

ROM_START( ultrainv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "in-01.bin",   0x0000, 0x0400, CRC(db9de599) SHA1(ccee1116ca924b520a126b63088a76d2ce8c396f) )
	ROM_LOAD( "in-02.bin",   0x0400, 0x0400, CRC(febe6d1a) SHA1(e1c3a24b4fa5862107ada1f9d7249466e8c3f06a) )
	ROM_LOAD( "in-03.bin",   0x0800, 0x0400, CRC(3d5c9820) SHA1(f3c83c660edf56a04148e2aa1c8e00427b86ca07) )
	ROM_LOAD( "in-04.bin",   0x1400, 0x0400, CRC(1293b826) SHA1(165cd5d08a19eadbe954145b12807f10df9e691a) )
	ROM_LOAD( "in-05.bin",   0x1800, 0x0400, CRC(e315a8c4) SHA1(dffec9e8bd7014fa34500b4bdac7feadac090482) )
	ROM_LOAD( "in-06.bin",   0x1c00, 0x0400, CRC(d958478c) SHA1(9df38c400c500b45d306d52fe74cd4d5ca92c0f0) )
ROM_END


ROM_START( invmulti )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00 ) // decrypted rom goes here

	ROM_REGION( 0x20000, "user1", 0 )
	ROM_LOAD("m803d.bin", 0x00000, 0x20000, CRC(6a62cb3c) SHA1(eb7b567098ad596859f417dd5c59c2cf1ebf1154) )
ROM_END

ROM_START( invmultim3a )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000, "user1", 0 )
	ROM_LOAD("m803a.bin", 0x00000, 0x20000, CRC(6d538828) SHA1(9a80c67abd32c4c8cd04320501a2aa4e2a308fc9) )
ROM_END

ROM_START( invmultim2c )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000, "user1", 0 )
	ROM_LOAD("m802c.bin", 0x00000, 0x20000, CRC(5b537de5) SHA1(4d8a6b622b818e88383d011c25f8f34b7372db6d) )
ROM_END

ROM_START( invmultim2a )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000, "user1", 0 )
	ROM_LOAD("m802a.bin", 0x00000, 0x20000, CRC(8079b1d0) SHA1(b13d910f314550eef468ee819b92788d2a002d82) )
ROM_END

ROM_START( invmultim1a )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000, "user1", 0 )
	ROM_LOAD("m801a.bin", 0x00000, 0x20000, CRC(f28536d2) SHA1(08ef3ea3fac38c7a478f094bfa7c369ac39515c4) )
ROM_END

ROM_START( invmultit3d )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000, "user1", 0 )
	ROM_LOAD("t803d.bin", 0x00000, 0x20000, CRC(4d53173c) SHA1(a9caf7fd8e2fea86ca1cf7edc104bdacf09203f8) )
ROM_END

ROM_START( invmultis3a )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000, "user1", 0 )
	ROM_LOAD("s083a.bin", 0x00000, 0x20000, CRC(f426d43b) SHA1(a299472f1d65f356ec01ca7cc8d3039abac20019) )
ROM_END

ROM_START( invmultis2a )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000, "user1", 0 )
	ROM_LOAD("s082a.bin", 0x00000, 0x20000, CRC(25f0f17e) SHA1(a3ccf823399e23dd9fdb38fd58c0acfe80b57fe3) )
ROM_END

ROM_START( invmultis1a )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000, "user1", 0 )
	ROM_LOAD("s081a.bin", 0x00000, 0x20000, CRC(daa77345) SHA1(0fdc9c2a6d9c0aa3233c5d31433adb1ea4e5b250) )
ROM_END

ROM_START( invmultip )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000, "user1", 0 )
	ROM_LOAD("s10.bin",  0x00000, 0x20000, CRC(1b43e4d3) SHA1(c50decd9caaec7f2d8b3ba74f718372d31bc1c3b) )
ROM_END


ROM_START( rollingc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rc01.bin",     0x0000, 0x0400, CRC(66fa50bf) SHA1(7451d4ff8d3b351a324aaecdbdc5b46672f5fdd0) )
	ROM_LOAD( "rc02.bin",     0x0400, 0x0400, CRC(61c06ae4) SHA1(7685c806e20e4a4a0508a547ac08ca8f6d75bb79) )
	ROM_LOAD( "rc03.bin",     0x0800, 0x0400, CRC(77e39fa0) SHA1(16bf88af1b97c5a2a81e105af08b8d9d1f10dcc8) )
	ROM_LOAD( "rc04.bin",     0x0c00, 0x0400, CRC(3fdfd0f3) SHA1(4c5e7136a766f3f16399e61eaaa0e00ef6b619f7) )
	ROM_LOAD( "rc05.bin",     0x1000, 0x0400, CRC(c26a8f5b) SHA1(f7a541999cfe04c6d6927d285484f0f81857e04a) )
	ROM_LOAD( "rc06.bin",     0x1400, 0x0400, CRC(0b98dbe5) SHA1(33cedab82ddccb4caaf681fce553b5230a8d6f92) )
	ROM_LOAD( "rc07.bin",     0x1800, 0x0400, CRC(6242145c) SHA1(b01bb02835dda89dc02604ec52e423167183e8c9) )
	ROM_LOAD( "rc08.bin",     0x1c00, 0x0400, CRC(d23c2ef1) SHA1(909e3d53291dbd219f4f9e0047c65317b9f6d5bd) )

	ROM_LOAD( "rc09.bin",     0x4000, 0x0800, CRC(2e2c5b95) SHA1(33f4e2789d67e355ccd99d2c0d07301ec2bd3bc1) )
	ROM_LOAD( "rc10.bin",     0x4800, 0x0800, CRC(ef94c502) SHA1(07c0504b2ebce0fa6e53e6957e7b6c0e9caab430) )
	ROM_LOAD( "rc11.bin",     0x5000, 0x0800, CRC(a3164b18) SHA1(7270af25fa4171f86476f5dc409e658da7fba7fc) )
	ROM_LOAD( "rc12.bin",     0x5800, 0x0800, CRC(2052f6d9) SHA1(036702fc40cf133eb374ed674695d7c6c79e8311) )
ROM_END

ROM_START( schaser )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rt13.bin",     0x0000, 0x0400, CRC(0dfbde68) SHA1(7367b138ad8448aba9222fed632a892df65cecbd) )
	ROM_LOAD( "rt14.bin",     0x0400, 0x0400, CRC(5a508a25) SHA1(c681d0bbf49317e79b596fb094e66b8912f0e409) )
	ROM_LOAD( "rt15.bin",     0x0800, 0x0400, CRC(2ac43a93) SHA1(d364f0940681a888c0147e06bcb01f8a0d4a24c8) )
	ROM_LOAD( "rt16.bin",     0x0c00, 0x0400, CRC(f5583afc) SHA1(5e8edb43ccb138fd47ac8f3da1af79b4444a4a82) )
	ROM_LOAD( "rt17.bin",     0x1000, 0x0400, CRC(51cf1155) SHA1(fd8c82d951602fd7e0ada65fc7cdee9f277c70db) )
	ROM_LOAD( "rt18.bin",     0x1400, 0x0400, CRC(3f0fc73a) SHA1(b801c3f1e8e6e41c564432db7c5891f6b27293b2) )
	ROM_LOAD( "rt19.bin",     0x1800, 0x0400, CRC(b66ea369) SHA1(d277f572f9c7c4301518546cf60671a6539326ee) )
	ROM_LOAD( "rt20.bin",     0x1c00, 0x0400, CRC(e3a7466a) SHA1(2378970f38b0cec066ef853a6540500e468e4ab4) )
	ROM_LOAD( "rt21.bin",     0x4000, 0x0400, CRC(b368ac98) SHA1(6860efe0496955db67611183be0efecda92c9c98) )
	ROM_LOAD( "rt22.bin",     0x4400, 0x0400, CRC(6e060dfb) SHA1(614e2ecf676c3ea2f9ea869125cfffef2f713684) )

	ROM_REGION( 0x0400, "proms", 0 )        /* background color map */
	ROM_LOAD( "rt06.ic2",     0x0000, 0x0400, CRC(950cf973) SHA1(d22df09b325835a0057ccd0d54f827b374254ac6) )
ROM_END

ROM_START( schasera )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rt13.bin",     0x0000, 0x0800, CRC(7b0bfeed) SHA1(832fe90430653d03cd0e7ea1b046524a2ca292ea) ) // sldh
	ROM_LOAD( "rt15.bin",     0x0800, 0x0800, CRC(825fc8ac) SHA1(176ff0f4d0cd55be30efb184bd5bef62b92d0333) ) // sldh
	ROM_LOAD( "rt17.bin",     0x1000, 0x0800, CRC(de9d3f85) SHA1(13a71fdd889023cfc65ed2c0a65236884b79b1f0) ) // sldh
	ROM_LOAD( "rt19.bin",     0x1800, 0x0800, CRC(c0adab87) SHA1(4bb8e4ccfb5eaa052584555bfa03fecf19ab8a29) ) // sldh
	ROM_LOAD( "rt21.bin",     0x4000, 0x0800, CRC(a3b31070) SHA1(af0108e1446a2be66cfc00d0b837fa91ab882441) ) // sldh

	ROM_REGION( 0x0400, "proms", 0 )        /* background color map */
	ROM_LOAD( "rt06.ic2",     0x0000, 0x0400, CRC(950cf973) SHA1(d22df09b325835a0057ccd0d54f827b374254ac6) )
ROM_END

ROM_START( schaserb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rt33.bin",     0x0000, 0x0800, CRC(eec6b032) SHA1(da14fcd862d6b80531cd3b858034bc5a120ed8ae) )
	ROM_LOAD( "rt34.bin",     0x0800, 0x0800, CRC(13a73701) SHA1(48ddbc10dec458070274c9fabbb0c420e2a07c96) )
	ROM_LOAD( "rt35.bin",     0x1000, 0x0800, CRC(de9d3f85) SHA1(13a71fdd889023cfc65ed2c0a65236884b79b1f0) )
	ROM_LOAD( "rt36.bin",     0x1800, 0x0800, CRC(521ec25e) SHA1(ce53e882c11a4c36f3edc3b389d3f5ad0e0ec151) )
	ROM_LOAD( "rt37.bin",     0x4000, 0x0800, CRC(44f65f19) SHA1(ee97d7987f54c9c26f5a20c72bdae04c46f94dc4) )

	ROM_REGION( 0x0400, "proms", 0 )        /* background color map */
	ROM_LOAD( "rt06.ic2",     0x0000, 0x0400, CRC(950cf973) SHA1(d22df09b325835a0057ccd0d54f827b374254ac6) )
ROM_END

ROM_START( schaserm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "MR26.71",     0x0000, 0x0800, CRC(4e547879) SHA1(464fab35373d6bd6218474e7f5109425376f1db2) )
	ROM_LOAD( "RT08.70",     0x0800, 0x0800, CRC(825fc8ac) SHA1(176ff0f4d0cd55be30efb184bd5bef62b92d0333) )
	ROM_LOAD( "RT09.69",     0x1000, 0x0800, CRC(de9d3f85) SHA1(13a71fdd889023cfc65ed2c0a65236884b79b1f0) )
	ROM_LOAD( "MR27.62",     0x1800, 0x0800, CRC(069ec108) SHA1(b12cd288d7e42002d01290f0572f9074adf2cdca) )
	ROM_LOAD( "RT11.61",     0x4000, 0x0800, CRC(17a7ef7a) SHA1(1a7b3f9393dceddcd1e220cadbff7e619594f884) )

	ROM_REGION( 0x0400, "proms", 0 )        /* background color map */
	ROM_LOAD( "rt06.ic2",     0x0000, 0x0400, CRC(950cf973) SHA1(d22df09b325835a0057ccd0d54f827b374254ac6) )
ROM_END

ROM_START( sflush )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fr05.sc2",     0xd800, 0x800, CRC(c4f08f9f) SHA1(997f216f5244942fc1a19f5c1988adbfadc301fc) )
	ROM_LOAD( "fr04.sc3",     0xe000, 0x800, CRC(87a754a5) SHA1(07c0e2c3cb7aa0086d8f4dd202a452bc6c20d4ee) )
	ROM_LOAD( "fr03.sc4",     0xe800, 0x800, CRC(5b12847f) SHA1(4b62342723dd49a387fae6637c331d7c853712a3) )
	ROM_LOAD( "fr02.sc5",     0xf000, 0x800, CRC(291c9b1f) SHA1(7e5b3e1605581abf3d8165f4de9d4e32a5ee3bb0) )
	ROM_LOAD( "fr01.sc6",     0xf800, 0x800, CRC(55d688c6) SHA1(574a3a2ca73cabb4b8f3444aa4464e6d64daa3ad) )
ROM_END

ROM_START( schasercv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1",            0x0000, 0x0400, CRC(bec2b16b) SHA1(c62210ecb64d7c38e5b63481d7fe04eb59bb1068) )
	ROM_LOAD( "2",            0x0400, 0x0400, CRC(9d25e608) SHA1(4cc52a93a3ab96a0ec1d07593e17832fa59b30a1) )
	ROM_LOAD( "3",            0x0800, 0x0400, CRC(113d0635) SHA1(ab5e98d0b5fc37d7d69bb5c541681a0f66460440) )
	ROM_LOAD( "4",            0x0c00, 0x0400, CRC(f3a43c8d) SHA1(29a7a8b7d1de763a255cfec79157fd95e7bff551) )
	ROM_LOAD( "5",            0x1000, 0x0400, CRC(47c84f23) SHA1(61b475fa92b8335f8edd3a128d8ac8561658e464) )
	ROM_LOAD( "6",            0x1400, 0x0400, CRC(02ff2199) SHA1(e12c235b2064cb4bb426145172e523256e3c6358) )
	ROM_LOAD( "7",            0x1800, 0x0400, CRC(87d06b88) SHA1(2d743161f85e47cb8ee2a600cbee790b1ad7ad99) )
	ROM_LOAD( "8",            0x1c00, 0x0400, CRC(6dfaad08) SHA1(2184c4e2f4b6bffdc4fe13e178134331fcd43253) )
	ROM_LOAD( "9",            0x4000, 0x0400, CRC(3d1a2ae3) SHA1(672ad6590aebdfebc2748455fa638107f3934c41) )
	ROM_LOAD( "10",           0x4400, 0x0400, CRC(037edb99) SHA1(f2fc5e61f962666e7f6bb81753ac24ea0b97e581) )

	ROM_REGION( 0x0800, "proms", 0 )        /* color maps player 1/player 2 (not used, but they were on the board) */
	ROM_LOAD( "cv01",         0x0000, 0x0400, CRC(037e16ac) SHA1(d585030aaff428330c91ae94d7cd5c96ebdd67dd) )
	ROM_LOAD( "cv02",         0x0400, 0x0400, CRC(8263da38) SHA1(2e7c769d129e6f8a1a31eba1e02777bb94ac32b2) )
ROM_END

ROM_START( schaserc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "45.ic30",      0x0000, 0x0400, CRC(ca90619c) SHA1(d2f9b29290d720c57f867d1dc193e877248e6afd) )
	ROM_LOAD( "46.ic36",      0x0400, 0x0400, CRC(6a016895) SHA1(6984d9d002e5d8fa14bdaf16f6ba9ca02136372c) )
	ROM_LOAD( "rt15.bin",     0x0800, 0x0400, CRC(2ac43a93) SHA1(d364f0940681a888c0147e06bcb01f8a0d4a24c8) )
	ROM_LOAD( "rt16.bin",     0x0c00, 0x0400, CRC(f5583afc) SHA1(5e8edb43ccb138fd47ac8f3da1af79b4444a4a82) )
	ROM_LOAD( "rt17.bin",     0x1000, 0x0400, CRC(51cf1155) SHA1(fd8c82d951602fd7e0ada65fc7cdee9f277c70db) )
	ROM_LOAD( "rt18.bin",     0x1400, 0x0400, CRC(3f0fc73a) SHA1(b801c3f1e8e6e41c564432db7c5891f6b27293b2) )
	ROM_LOAD( "rt19.bin",     0x1800, 0x0400, CRC(b66ea369) SHA1(d277f572f9c7c4301518546cf60671a6539326ee) )
	ROM_LOAD( "47.ic39",      0x1c00, 0x0400, CRC(d476e182) SHA1(87428bf0131f8bf39d506b8df424af94cd944d82) )
	ROM_LOAD( "rt21.bin",     0x4000, 0x0400, CRC(b368ac98) SHA1(6860efe0496955db67611183be0efecda92c9c98) )
	ROM_LOAD( "rt22.bin",     0x4400, 0x0400, CRC(6e060dfb) SHA1(614e2ecf676c3ea2f9ea869125cfffef2f713684) )

	ROM_REGION( 0x0400, "proms", 0 )        /* background color map */
	ROM_LOAD( "rt06.ic2",     0x0000, 0x0400, CRC(950cf973) SHA1(d22df09b325835a0057ccd0d54f827b374254ac6) )
ROM_END

ROM_START( lupin3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lp01.36",      0x0000, 0x0800, CRC(fd506ee8) SHA1(67ce62f24892f0eddf3e47913dff541f41493a17) )
	ROM_LOAD( "lp02.35",      0x0800, 0x0800, CRC(ec4225f8) SHA1(cd7360b3b339e5050075b498226070914fb7a031) )
	ROM_LOAD( "lp03.34",      0x1000, 0x0800, CRC(9307d377) SHA1(081f6c63ff2dcc549e44ab5ff5f5ddf99d544640) )
	ROM_LOAD( "lp04.33",      0x1800, 0x0800, CRC(e41e8b2b) SHA1(e67eaa8aeaf13f706afc17074fbbde3ad2cc9548) )
	ROM_LOAD( "lp05.32",      0x4000, 0x0800, CRC(f5c2faf4) SHA1(8d056f8c630e4659c02dd5da759dd497e4734292) )
	ROM_LOAD( "lp06.31",      0x4800, 0x0800, CRC(66289ab2) SHA1(fc9b4a7b7a08d43f34beaf1a8e68ed0ff6148534) )

	ROM_REGION( 0x0800, "proms", 0 )        /* color map */
	ROM_LOAD( "lp08.1",       0x0000, 0x0400, CRC(33dbd03a) SHA1(1e0ae1cad1e9a90642886ae2ef726d3f383dd6cf) )
	ROM_LOAD( "lp09.2",       0x0400, 0x0400, CRC(9eaee652) SHA1(a4d2d8282ba825f3a8c0cc9bca16e1d36a0d0796) )
ROM_END

ROM_START( lupin3a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lp12.bin",     0x0000, 0x0800, CRC(68a7f47a) SHA1(dce99b3810331d7603fa468f1dea984e571f709b) )
	ROM_LOAD( "lp13.bin",     0x0800, 0x0800, CRC(cae9a17b) SHA1(a333ba7db45325996e3254ab36162bb7577e8a38) )
	ROM_LOAD( "lp14.bin",     0x1000, 0x0800, CRC(3553b9e4) SHA1(6affb5b6caf08f365c0dce669e44046295c3df91) )
	ROM_LOAD( "lp15.bin",     0x1800, 0x0800, CRC(acbeef64) SHA1(50d78cdc9938285b6bf9fa81fa0f6c30b23e0756) )
	ROM_LOAD( "lp16.bin",     0x4000, 0x0800, CRC(19fcdc54) SHA1(2f18ee8158321fff68886ffe793724001e8b18c2) )
	ROM_LOAD( "lp17.bin",     0x4800, 0x0800, CRC(66289ab2) SHA1(fc9b4a7b7a08d43f34beaf1a8e68ed0ff6148534) )
	ROM_LOAD( "lp18.bin",     0x5000, 0x0800, CRC(2f07b4ba) SHA1(982e4c437b39b45e23d15af1b2fc8c7aa3034559) )
ROM_END

ROM_START( polaris )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps01-1.30",   0x0000, 0x0800, CRC(7d41007c) SHA1(168f002fe997aac6e4141292de826d389859bb04) )
	ROM_LOAD( "ps09.36",     0x0800, 0x0800, CRC(9a5c8cb2) SHA1(7a8c5d74f8b431072d9476d3ef65a3fe1d639813) )
	ROM_LOAD( "ps03-1.31",   0x1000, 0x0800, CRC(21f32415) SHA1(6ac9ae9b55e342729fe260147021ed3911a24dc2) )
	ROM_LOAD( "ps04.37",     0x1800, 0x0800, CRC(65694948) SHA1(de92a7f3e3ef732b573254baa60df60f8e068a5d) )
	ROM_LOAD( "ps05.32",     0x4000, 0x0800, CRC(772e31f3) SHA1(fa0b866b6df1a9217e286ca880b3bb3fb0644bf3) )
	ROM_LOAD( "ps10.38",     0x4800, 0x0800, CRC(3df77bac) SHA1(b3275c34b8d42df83df2c404c5b7d220aae651fa) )
	ROM_LOAD( "ps26",        0x5000, 0x0800, CRC(9d5c3d50) SHA1(a6acf9ca6e807625156cb1759269014d5830a44f) )

	ROM_REGION( 0x0400, "proms", 0 )        /* background color map */
	ROM_LOAD( "ps08.1b", 0x0000, 0x0400, CRC(164aa05d) SHA1(41c699ce45c76a60c71294f25d8df6c6e6c1280a) ) /* NEC B406 or compatible BPROM (82S137) */

	ROM_REGION( 0x0100, "user1", 0 )        /* cloud graphics */
	ROM_LOAD( "ps07.2c", 0x0000, 0x0100, CRC(2953253b) SHA1(2fb851bc9652ca4e51d473b484ede6dab05f1b51) ) /* MB7052 or compatible BPROM (82S129) */
ROM_END

ROM_START( polarisa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps01.30",     0x0000, 0x0800, CRC(c04ce5a9) SHA1(62cc9b3b682ebecfb7600393862c65e26ff5263f) )
	ROM_LOAD( "ps09.36",     0x0800, 0x0800, CRC(9a5c8cb2) SHA1(7a8c5d74f8b431072d9476d3ef65a3fe1d639813) )
	ROM_LOAD( "ps03.31",     0x1000, 0x0800, CRC(8680d7ea) SHA1(7fd4b8a415666c36842fed80d2798b48f8b29d0d) )
	ROM_LOAD( "ps04.37",     0x1800, 0x0800, CRC(65694948) SHA1(de92a7f3e3ef732b573254baa60df60f8e068a5d) )
	ROM_LOAD( "ps05.32",     0x4000, 0x0800, CRC(772e31f3) SHA1(fa0b866b6df1a9217e286ca880b3bb3fb0644bf3) )
	ROM_LOAD( "ps10.38",     0x4800, 0x0800, CRC(3df77bac) SHA1(b3275c34b8d42df83df2c404c5b7d220aae651fa) )

	ROM_REGION( 0x0400, "proms", 0 )        /* background color map */
	ROM_LOAD( "ps08.1b", 0x0000, 0x0400, CRC(164aa05d) SHA1(41c699ce45c76a60c71294f25d8df6c6e6c1280a) ) /* NEC B406 or compatible BPROM (82S137) */

	ROM_REGION( 0x0100, "user1", 0 )        /* cloud graphics */
	ROM_LOAD( "ps07.2c", 0x0000, 0x0100, CRC(2953253b) SHA1(2fb851bc9652ca4e51d473b484ede6dab05f1b51) ) /* MB7052 or compatible BPROM (82S129) */
ROM_END


ROM_START( polariso )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ps01.30",     0x0000, 0x0800, CRC(c04ce5a9) SHA1(62cc9b3b682ebecfb7600393862c65e26ff5263f) )
	ROM_LOAD( "ps02.36",     0x0800, 0x0800, CRC(18648e4f) SHA1(9f672e108177d5d9bc004b41eec00dc4d19269ff) )
	ROM_LOAD( "ps03.31",     0x1000, 0x0800, CRC(8680d7ea) SHA1(7fd4b8a415666c36842fed80d2798b48f8b29d0d) )
	ROM_LOAD( "ps04.37",     0x1800, 0x0800, CRC(65694948) SHA1(de92a7f3e3ef732b573254baa60df60f8e068a5d) )
	ROM_LOAD( "ps05.32",     0x4000, 0x0800, CRC(772e31f3) SHA1(fa0b866b6df1a9217e286ca880b3bb3fb0644bf3) )
	ROM_LOAD( "ps06.38",     0x4800, 0x0800, CRC(f577cb72) SHA1(7a931b5ebaf0d6941191f21afb9ed670d0251e07) )

	ROM_REGION( 0x0400, "proms", 0 )        /* background color map */
	ROM_LOAD( "ps08.1b", 0x0000, 0x0400, CRC(164aa05d) SHA1(41c699ce45c76a60c71294f25d8df6c6e6c1280a) ) /* NEC B406 or compatible BPROM (82S137) */

	ROM_REGION( 0x0100, "user1", 0 )        /* cloud graphics */
	ROM_LOAD( "ps07.2c", 0x0000, 0x0100, CRC(2953253b) SHA1(2fb851bc9652ca4e51d473b484ede6dab05f1b51) ) /* MB7052 or compatible BPROM (82S129) */
ROM_END

ROM_START( polarisbr ) /* aka Polaris II on flyers? */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1",   0x0000, 0x0800, CRC(17015f52) SHA1(8beb4d927c08420f9990fac787a81d4bd6dd419c) )
	ROM_LOAD( "2",   0x0800, 0x0800, CRC(9a5c8cb2) SHA1(7a8c5d74f8b431072d9476d3ef65a3fe1d639813) )
	ROM_LOAD( "3",   0x1000, 0x0800, CRC(60118368) SHA1(e1189fd88b943fcf77a5c41c519cccdb8196910c) )
	ROM_LOAD( "4",   0x1800, 0x0800, CRC(65694948) SHA1(de92a7f3e3ef732b573254baa60df60f8e068a5d) )
	ROM_LOAD( "5",   0x4000, 0x0800, CRC(6cb21b31) SHA1(f9d435a3aa905f124cb87c139b047e1585d0997b) )
	ROM_LOAD( "6",   0x4800, 0x0800, CRC(3df77bac) SHA1(b3275c34b8d42df83df2c404c5b7d220aae651fa) )
	ROM_LOAD( "7",   0x5000, 0x0800, CRC(0d811b92) SHA1(09af62997e1e0da0525ab4f6ced775d3673d8f35) )

	ROM_REGION( 0x0400, "proms", 0 )        /* background color map */
	ROM_LOAD( "ps08.1b", 0x0000, 0x0400, CRC(164aa05d) SHA1(41c699ce45c76a60c71294f25d8df6c6e6c1280a) ) /* NEC B406 or compatible BPROM (82S137) */

	ROM_REGION( 0x0100, "user1", 0 )        /* cloud graphics */
	ROM_LOAD( "ps07.2c", 0x0000, 0x0100, CRC(2953253b) SHA1(2fb851bc9652ca4e51d473b484ede6dab05f1b51) ) /* MB7052 or compatible BPROM (82S129) */
ROM_END

ROM_START( ozmawars )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mw01",         0x0000, 0x0800, CRC(31f4397d) SHA1(bba9765aadd608d19e2515a5edf8e0eceb70916a) )
	ROM_LOAD( "mw02",         0x0800, 0x0800, CRC(d8e77c62) SHA1(84fc81cf9a924ecbb13a008cd7435b7d465bddf6) )
	ROM_LOAD( "mw03",         0x1000, 0x0800, CRC(3bfa418f) SHA1(7318878202322a2263551ca463e4c70943401f68) )
	ROM_LOAD( "mw04",         0x1800, 0x0800, CRC(e190ce6c) SHA1(120898e9a683f5ce874c6fde761570a26de2fa8c) )
	ROM_LOAD( "mw05",         0x4000, 0x0800, CRC(3bc7d4c7) SHA1(b084f8cd2ce0f502c2e915da3eceffcbb448e9c0) )
	ROM_LOAD( "mw06",         0x4800, 0x0800, CRC(99ca2eae) SHA1(8d0f220f68043eff0c85d2de7bee7fd4365fb51c) )
ROM_END

ROM_START( ozmawars2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mw01",         0x0000, 0x0800, CRC(31f4397d) SHA1(bba9765aadd608d19e2515a5edf8e0eceb70916a) )
	ROM_LOAD( "mw02",         0x0800, 0x0800, CRC(d8e77c62) SHA1(84fc81cf9a924ecbb13a008cd7435b7d465bddf6) )
	ROM_LOAD( "oz5",          0x1000, 0x0400, CRC(5597bf52) SHA1(626c7348365ed974d416485d94d057745b5d9b96) )
	ROM_LOAD( "oz6",          0x1400, 0x0400, CRC(19b43578) SHA1(3609b7c77f5ee6f10f302892f56fcc8375577f20) )
	ROM_LOAD( "oz7",          0x1800, 0x0400, CRC(a285bfde) SHA1(ed7a9fce4d887d3b5d596645893ea87c0bafda02) )
	ROM_LOAD( "oz8",          0x1c00, 0x0400, CRC(ae59a629) SHA1(0c9ea67dc35f93ec65ec91e1dab2e4b6212428bf) )
	ROM_LOAD( "mw05",         0x4000, 0x0800, CRC(3bc7d4c7) SHA1(b084f8cd2ce0f502c2e915da3eceffcbb448e9c0) )
	ROM_LOAD( "oz11",         0x4800, 0x0400, CRC(660e934c) SHA1(1d50ae3a9de041b908e256892203ce1738d588f6) )
	ROM_LOAD( "oz12",         0x4c00, 0x0400, CRC(8b969f61) SHA1(6d12cacc73c31a897812ccd8de24725ee56dd975) )
ROM_END

ROM_START( solfight )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "solfight.m",   0x0000, 0x0800, CRC(a4f2814e) SHA1(e2437e3543dcc97eeaea32babcd4aec6455581ac) )
	ROM_LOAD( "solfight.n",   0x0800, 0x0800, CRC(5657ec07) SHA1(9a2fb398841160f59483bb70060caba37addb8a4) )
	ROM_LOAD( "solfight.p",   0x1000, 0x0800, CRC(ef9ce96d) SHA1(96867b4f2d72f3a8827b1eb3a0748922eaa8d608) )
	ROM_LOAD( "solfight.r",   0x1800, 0x0800, CRC(4f1ef540) SHA1(a798e57959e72bfb554dd2fed0e37027312f9ed3) )
	ROM_LOAD( "mw05",         0x4000, 0x0800, CRC(3bc7d4c7) SHA1(b084f8cd2ce0f502c2e915da3eceffcbb448e9c0) )
	ROM_LOAD( "solfight.t",   0x4800, 0x0800, CRC(3b6fb206) SHA1(db631f4a0bd5344d130ff8d723d949e9914b6f92) )
ROM_END

ROM_START( spaceph ) /* Also seen in a 6-rom version which matches contents exactly (sv01+sv02, sv03+sv04, etc)*/
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sv01.bin",     0x0000, 0x0400, CRC(de84771d) SHA1(13a7e5eedb826cca4d59634d38db9fcf5e65b732) )
	ROM_LOAD( "sv02.bin",     0x0400, 0x0400, CRC(957fc661) SHA1(ac0edc901d8033619f62967f8eaf53a02947e109) )
	ROM_LOAD( "sv03.bin",     0x0800, 0x0400, CRC(dbda38b9) SHA1(73a277616a0c236b07c9ffa66f16a27a78c12d70) )
	ROM_LOAD( "sv04.bin",     0x0c00, 0x0400, CRC(f51544a5) SHA1(368411a2dadaebcbb4d5b6cf6c2beec036ce817f) )
	ROM_LOAD( "sv05.bin",     0x1000, 0x0400, CRC(98d02683) SHA1(f13958df8d385f532e993e4c34569d992904a4ed) )
	ROM_LOAD( "sv06.bin",     0x1400, 0x0400, CRC(4ec390fd) SHA1(ade23efde5d55d282fbb28a5f8a1346601501b79) )
	ROM_LOAD( "sv07.bin",     0x1800, 0x0400, CRC(170862fd) SHA1(ac64a97b1510ca81d4ef3a5fcf45b7e6c7414914) )
	ROM_LOAD( "sv08.bin",     0x1c00, 0x0400, CRC(511b12cf) SHA1(08ba43024c8574ded11aa457eca24b72984f5ea9) )
	ROM_LOAD( "sv09.bin",     0x4000, 0x0400, CRC(af1cd1af) SHA1(286d77e8556e475b291a3b1a53acaca8b7dc3678) )
	ROM_LOAD( "sv10.bin",     0x4400, 0x0400, CRC(31b7692e) SHA1(043880750d134d04311eab55e30ee223977d3d17) )
	ROM_LOAD( "sv11.bin",     0x4800, 0x0400, CRC(50257351) SHA1(5c3eb29f36f04b7fb8f0351ccf9c8cfc7587f927) )
	ROM_LOAD( "sv12.bin",     0x4c00, 0x0400, CRC(a2a3366a) SHA1(87032787450216d378406122effa95ea01145bf7) )
ROM_END

ROM_START( ballbomb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tn01",         0x0000, 0x0800, CRC(551585b5) SHA1(7c17b046bdfca6ab107b7e68ba9bde6ca590c3d4) )
	ROM_LOAD( "tn02",         0x0800, 0x0800, CRC(7e1f734f) SHA1(a15656818cd730d9bc98d00ff1e7fe3f860bd624) )
	ROM_LOAD( "tn03",         0x1000, 0x0800, CRC(d93e20bc) SHA1(2bf72f813750cef8fad572a18fb8e9fd5bf38804) )
	ROM_LOAD( "tn04",         0x1800, 0x0800, CRC(d0689a22) SHA1(1f6b258431b7eb878853ff979e4d97a05fb6b797) )
	ROM_LOAD( "tn05-1",       0x4000, 0x0800, CRC(5d5e94f1) SHA1(b9f8ba38161ef4f0940c274e9d93fed4bb7db017) )

	ROM_REGION( 0x0800, "proms", 0 )        /* color maps player 1/player 2 */
	ROM_LOAD( "tn06",         0x0000, 0x0400, CRC(7ec554c4) SHA1(b638605ba2043fdca4c5e18755fa5fa81ed3db07) )
	ROM_LOAD( "tn07",         0x0400, 0x0400, CRC(deb0ac82) SHA1(839581c4e58cb7b0c2c14cf4f239220017cc26eb) )

	ROM_REGION( 0x0100, "user1", 0 )        /* cloud graphics (missing) */
	ROM_LOAD( "mb7052.2c",    0x0000, 0x0100, NO_DUMP )
ROM_END

ROM_START( yosakdon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "yd1.bin",      0x0000, 0x0400, CRC(607899c9) SHA1(219c0c99894715818606fba49cc75517f6f43e0c) )
	ROM_LOAD( "yd2.bin",      0x0400, 0x0400, CRC(78336df4) SHA1(b0b6254568d191d2d0b9c9280a3ccf2417ef3f38) )
	ROM_LOAD( "yd3.bin",      0x0800, 0x0400, CRC(c5af6d52) SHA1(c40af79fe060562c64fc316881b7d0348e11ee3f) )
	ROM_LOAD( "yd4.bin",      0x0c00, 0x0400, CRC(dca8064f) SHA1(77a58137cc7f0b5fbe0e9e8deb9c5be88b1ebbcf) )
	ROM_LOAD( "yd5.bin",      0x1400, 0x0400, CRC(38804ff1) SHA1(9b7527b9d2b106355f0c8df46666b1e3f286b2e3) )
	ROM_LOAD( "yd6.bin",      0x1800, 0x0400, CRC(988d2362) SHA1(deaf864b4e287cbc2585c2a11343b1ae82e15463) )
	ROM_LOAD( "yd7.bin",      0x1c00, 0x0400, CRC(2744e68b) SHA1(5ad5a7a615d36f57b6d560425e035c15e25e9005) )
ROM_END

ROM_START( yosakdona )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "yosaku1",      0x0000, 0x0400, CRC(d132f4f0) SHA1(373c7ea1bd6debcb3dad5881793b8c31dc7a01e6) )
	ROM_LOAD( "yd2.bin",      0x0400, 0x0400, CRC(78336df4) SHA1(b0b6254568d191d2d0b9c9280a3ccf2417ef3f38) )
	ROM_LOAD( "yosaku3",      0x0800, 0x0400, CRC(b1a0b3eb) SHA1(4eb80668920b45dc6216424f8ca53d753a35f4f1) )
	ROM_LOAD( "yosaku4",      0x0c00, 0x0400, CRC(c06c225e) SHA1(2699e3c13b09b6de16bd3ca3ca2e9d7a91b7e268) )
	ROM_LOAD( "yosaku5",      0x1400, 0x0400, CRC(ae422a43) SHA1(5219680f9d6c5d984b29167f85106fa375856121) )
	ROM_LOAD( "yosaku6",      0x1800, 0x0400, CRC(26b24a12) SHA1(387589fa4027d41b6fb06555661d4f92fe2f990c) )
	ROM_LOAD( "yosaku7",      0x1c00, 0x0400, CRC(878d5a18) SHA1(6adc8763d5644602eed7fe6d9186a48be105aace) )
ROM_END

ROM_START( indianbt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.36",       0x0000, 0x0800, CRC(ddc2b25d) SHA1(120ae17492b79d7d2ad515de9f1e3be7f8b9d4eb) )
	ROM_LOAD( "2.35",       0x0800, 0x0800, CRC(6499b062) SHA1(62a301d532b9fc4e7a17cbe8d2061eb0e842bdfa) )
	ROM_LOAD( "3.34",       0x1000, 0x0800, CRC(5c51675d) SHA1(1313e8794ee6cd0252452b96d42cff7907eeaa21) )
	ROM_LOAD( "4.33",       0x1800, 0x0800, CRC(70ebec95) SHA1(f6e1e7a28033d89e49b88c559ea8926b1b4ff21b) )
	ROM_LOAD( "5.32",       0x4000, 0x0800, CRC(7b4022f4) SHA1(10dec8110e8f4bc79764d3183bdfb3c135e27faf) )
	ROM_LOAD( "6.31",       0x4800, 0x0800, CRC(89bd6f73) SHA1(5dc63871252c530ef0aae4f4cd02fee44b397815) )
	ROM_LOAD( "7.42",       0x5000, 0x0800, CRC(7060ba0b) SHA1(366ce02b7b0a3391afef23b8b41cd98a91034830) )
	ROM_LOAD( "8.41",       0x5800, 0x0800, CRC(eaccfc0a) SHA1(c6c2d702243bdd1d2ad5fbaaceadb5a5798577bc) )

	ROM_REGION( 0x0800, "proms", 0 )        /* color maps player 1/player 2 */
	ROM_LOAD( "mb7054.1",   0x0000, 0x0400, CRC(4acf4db3) SHA1(842a6c9f91806b424b7cc437670b4fe0bd57dff1) )
	ROM_LOAD( "mb7054.2",   0x0400, 0x0400, CRC(62cb3419) SHA1(3df65062945589f1df37359dbd3e30ae4b23f469) )
ROM_END

ROM_START( indianbtbr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.36",       0x0000, 0x0800, CRC(5cf6316b) SHA1(9812fbb7139d6f33a832a2485f9cd6422146d1ae) ) // sldh
	ROM_LOAD( "2.35",       0x0800, 0x0800, CRC(882c7421) SHA1(b2cc15c8693bd1fc74dddfcf52bf08984423f4bf) ) // sldh
	ROM_LOAD( "3.34",       0x1000, 0x0800, CRC(5c51675d) SHA1(1313e8794ee6cd0252452b96d42cff7907eeaa21) )
	ROM_LOAD( "4.33",       0x1800, 0x0800, CRC(70ebec95) SHA1(f6e1e7a28033d89e49b88c559ea8926b1b4ff21b) )
	ROM_LOAD( "5.32",       0x4000, 0x0800, CRC(aa12dbae) SHA1(083425b82cfdc0f037afcf293ad03b98fc6af3e5) ) // sldh
	ROM_LOAD( "6.31",       0x4800, 0x0800, CRC(d9cb1691) SHA1(c13cd8479914ba6719427b408ed589c9892f832c) ) // sldh
	ROM_LOAD( "7.42",       0x5000, 0x0800, CRC(7060ba0b) SHA1(366ce02b7b0a3391afef23b8b41cd98a91034830) )
	ROM_LOAD( "8.41",       0x5800, 0x0800, CRC(e96699d6) SHA1(701d370ae28608221fb4d00e12877d30122c848e) )

	ROM_REGION( 0x0800, "proms", 0 )        /* color maps player 1/player 2 */
	ROM_LOAD( "mb7054.1",   0x0000, 0x0400, CRC(4acf4db3) SHA1(842a6c9f91806b424b7cc437670b4fe0bd57dff1) )
	ROM_LOAD( "mb7054.2",   0x0400, 0x0400, CRC(62cb3419) SHA1(3df65062945589f1df37359dbd3e30ae4b23f469) )
ROM_END

ROM_START( shuttlei )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.13c",      0x0000, 0x0400, CRC(b6d4f0cd) SHA1(f855a793e78ff6283288c815b59e6942513ab4f8) )
	ROM_LOAD( "2.11c",      0x0400, 0x0400, CRC(168d6138) SHA1(e0e5ba58eb5a3a00802504c48a96d63522f9865f) )
	ROM_LOAD( "3.13d",      0x0800, 0x0400, CRC(804bd7fb) SHA1(f019bcc2894f9b819a14c069de8f1a7d228b79eb) )
	ROM_LOAD( "4.11d",      0x0c00, 0x0400, CRC(8205b369) SHA1(685dd244881f5762d0f53cbfa935da2b857e3fba) )
	ROM_LOAD( "5.13e",      0x1000, 0x0400, CRC(b50df820) SHA1(27a846ac3da4c0890a80f60483ed5750cb0b2476) )
	ROM_LOAD( "8.11f",      0x1c00, 0x0400, CRC(4978552b) SHA1(5a6b6e39f57a353580ed9281d7da24950f058426) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129.2b",  0x0000, 0x0100, CRC(f108d00d) SHA1(de0cb9d18e4c9920495011f962c4497a789f651f) )
ROM_END

ROM_START( skylove )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "01",   0x0000, 0x0400, CRC(391ad7d0) SHA1(73358fff44da5fffd4e08fbb615ccc0245e3365b) )
	ROM_LOAD( "02",   0x0400, 0x0400, CRC(365ba070) SHA1(8493bde493ea0d04b3563f9bc752a6ec57022524) )
	ROM_LOAD( "03",   0x0800, 0x0400, CRC(47364dad) SHA1(b49704f8d49a0866cb9cd8bb867f30246e3dabc9) )
	ROM_LOAD( "04",   0x0c00, 0x0400, CRC(9d76f33d) SHA1(5aa6a081a3609e6c036843049d58cc763a86fedb) )
	ROM_LOAD( "05",   0x1000, 0x0400, CRC(09084954) SHA1(f5c826188ffb7a572c45aad94e794f31bebfebe5) )
	ROM_LOAD( "06",   0x1400, 0x0400, CRC(6d494e82) SHA1(8e5ee1b842621cd088e80124b92b8a517e8dfbb9) )
	ROM_LOAD( "07",   0x1800, 0x0400, CRC(1a9aa4b8) SHA1(0da553c6343a2740312ebafc2b936ffbbf24af04) )
	ROM_LOAD( "08",   0x1c00, 0x0400, CRC(ecaacacc) SHA1(b815366d3aaa8ef311cd54a5be9fb4d60324e5a7) )
ROM_END


ROM_START( darthvdr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom0",           0x0000, 0x0400, CRC(b15785b6) SHA1(f453a006019dc83bd746f3a26736e913186332e6) )
	ROM_LOAD( "rom1",           0x0400, 0x0400, CRC(95947743) SHA1(59f414de21f680e0d68ca8c4b6b538c8006cfdd6) )
	ROM_LOAD( "rom2",           0x0800, 0x0400, CRC(19b1731f) SHA1(2383c241de8a1ed57f03ecc7ded97585a6c10c91) )
	ROM_LOAD( "rom3",           0x0c00, 0x0400, CRC(ca1b5e3c) SHA1(e54ca4a3f36b2ed5e4e42c1e8bbbde43c92796e9) )
	ROM_LOAD( "rom4",           0x1000, 0x0400, CRC(eede5f41) SHA1(cd9f023057eb9598bad01b9e9d91bb4866b9bd3b) )
	ROM_LOAD( "rom5",           0x1400, 0x0400, CRC(cc52a4bb) SHA1(857b75a8b01fc707db940197d6bf3b0466c4a7b5) )
ROM_END

ROM_START( astropal )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2708.0a",   0x0000, 0x0400, CRC(e6883322) SHA1(05b0ab0dc6297209dcfdd173e762bfae3a720e8d) )
	ROM_LOAD( "2708.1a",   0x0400, 0x0400, CRC(4401df1d) SHA1(16f3b957278aa67cb37bcd5defb6e4dd8ccf7d1f) )
	ROM_LOAD( "2708.2a",   0x0800, 0x0400, CRC(5bac1ee4) SHA1(8c3e5f882f4798f8ed0523b60a216c989324a7c2) )
	ROM_LOAD( "2708.3a",   0x0c00, 0x0400, CRC(a870afad) SHA1(1a256db2bc6baa238ee1df4eff2fdce0888f812c) )
	ROM_LOAD( "2708.4a",   0x1000, 0x0400, CRC(8bd2d985) SHA1(3ff9110c1bad7d4562664da772d14750d738c2d6) )
	ROM_LOAD( "2708.5a",   0x1400, 0x0400, CRC(5e97a86b) SHA1(f3500d48ecb3969b8aaea9c4e812fbf6cf4170af) )
	ROM_LOAD( "2708.6a",   0x1800, 0x0400, CRC(22c354d0) SHA1(c465ca5787ad8de3be97deac1214d3abd0b27e6b) )
	ROM_LOAD( "2708.7a",   0x1c00, 0x0400, CRC(aeca51c1) SHA1(767bca1e6bca41327b9ff6c3570edcabe46dec21) )
ROM_END

ROM_START( steelwkr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.36",             0x0000, 0x0400, CRC(5d78873a) SHA1(293cbc067937668148181453877239cb5ed57600) )
	ROM_LOAD( "2.35",             0x0400, 0x0400, CRC(99cd70c6) SHA1(a08bf4db6b39d22dfcf052cc6603aab041db0208) )
	ROM_LOAD( "3.34",             0x0800, 0x0400, CRC(18103b67) SHA1(45929ea56ab15769fc68873570aab3d403e8e913) )
	ROM_LOAD( "4.33",             0x0c00, 0x0400, CRC(c413ae82) SHA1(302b933b45b2aaa515434b5268fd74aec4160e3f) )
	ROM_LOAD( "5.32",             0x1000, 0x0400, CRC(ca7b07b5) SHA1(cbea221c4daf84825f99bbef6d731fc2ef88feeb) )
	ROM_LOAD( "6.31",             0x1400, 0x0400, CRC(f8181fa0) SHA1(a907611529a1500a2ae118e834c2d4b6d11974f1) )
	ROM_LOAD( "7.42",             0x1800, 0x0400, CRC(a35f113e) SHA1(53073037db55c14055810c0bee7b85eb75bbaa72) )
	ROM_LOAD( "8.41",             0x1c00, 0x0400, CRC(af208370) SHA1(ccbd002accda26cc0a02987d9801a47e5f49921a) )

	ROM_REGION( 0x0800, "proms", 0 )        /* color maps player 1/player 2 (not used, but they were on the board) */
	ROM_LOAD( "la05.1",         0x0000, 0x0400, CRC(98f31392) SHA1(ccdd1bd2ddd24bd6b1f8255a87e138f937eaf5b4) )
	ROM_LOAD( "la06.2",         0x0400, 0x0400, CRC(98f31392) SHA1(ccdd1bd2ddd24bd6b1f8255a87e138f937eaf5b4) )
ROM_END

ROM_START( galactic )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1",       0x0000, 0x0800, CRC(b5098f1e) SHA1(9d1d045d8abeafd4716d3052fe93e52c6b347049) ) // sldh
	ROM_LOAD( "2",       0x0800, 0x0800, CRC(f97410ee) SHA1(47f1f296c905fa13f6c521edc12c10f1f0e42400) )
	ROM_LOAD( "3",       0x1000, 0x0800, CRC(c1175feb) SHA1(83bf955ed3a52e1ce8c688d89725d8dee1bcc866) )
	ROM_LOAD( "4",       0x1800, 0x0800, CRC(b4451d7c) SHA1(62a18e8e927ef00a7f6cb933cdc5aeae9f074dc0) )
	ROM_LOAD( "5",       0x4000, 0x0800, CRC(74c9da61) SHA1(cb98105729f0fa4343b71af3c658b378ade1ed46) )
	ROM_LOAD( "6",       0x4800, 0x0800, CRC(5e7c6c44) SHA1(be7eeef10462377909018cf40503766f38466022) )
	ROM_LOAD( "7",       0x5000, 0x0800, CRC(02619e18) SHA1(4c59f17fbc96ca08090f08c41ca9fc72c074e9c0) )
ROM_END

ROM_START( spacmiss )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1",       0x0000, 0x0800, CRC(e212dc88) SHA1(bc56052bf43d18081f777b936b2be792e91ba842) ) // sldh
	ROM_LOAD( "2",       0x0800, 0x0800, CRC(f97410ee) SHA1(47f1f296c905fa13f6c521edc12c10f1f0e42400) )
	ROM_LOAD( "3",       0x1000, 0x0800, CRC(c1175feb) SHA1(83bf955ed3a52e1ce8c688d89725d8dee1bcc866) )
	ROM_LOAD( "4",       0x1800, 0x0800, CRC(b4451d7c) SHA1(62a18e8e927ef00a7f6cb933cdc5aeae9f074dc0) )
	ROM_LOAD( "5",       0x4000, 0x0800, CRC(74c9da61) SHA1(cb98105729f0fa4343b71af3c658b378ade1ed46) )
	ROM_LOAD( "6",       0x4800, 0x0800, CRC(5e7c6c44) SHA1(be7eeef10462377909018cf40503766f38466022) )
	ROM_LOAD( "7",       0x5000, 0x0800, CRC(02619e18) SHA1(4c59f17fbc96ca08090f08c41ca9fc72c074e9c0) )

	ROM_REGION( 0x0800, "user1", 0 )
	ROM_LOAD( "8",       0x0000, 0x0800, CRC(942e5261) SHA1(e8af51d644eab4e7b31c14dc66bb036ad8940c42) ) // ?
ROM_END

ROM_START( attackfc )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "30a.bin",       0x0000, 0x0400, CRC(c12e3386) SHA1(72b1d3d67a83edf0be0b0c37ef6dcffba450f16f) )
	ROM_LOAD( "36a.bin",       0x0400, 0x0400, CRC(6738dcb9) SHA1(e4c68553fc3f2d3db3d251b9cb325e2409d9c02a) )
	ROM_LOAD( "31a.bin",       0x0800, 0x0400, CRC(787a4658) SHA1(5be3143bdba6a32256603be94400034a8ea1fda6) )
	ROM_LOAD( "37a.bin",       0x0c00, 0x0400, CRC(ad6bfbbe) SHA1(5f5437b6c8e7dfe9649b25040862f8a51d8c43ed) )
	ROM_LOAD( "32a.bin",       0x1000, 0x0400, CRC(cbe0a711) SHA1(6e5f4214a4b48b70464005f4263c9b1ec3cbbeb1) )
	ROM_LOAD( "33a.bin",       0x1800, 0x0400, CRC(53147393) SHA1(57e078f1734e382e8a46be09c133daab30c75681) )
	ROM_LOAD( "39a.bin",       0x1c00, 0x0400, CRC(f538cf08) SHA1(4a375a41ab5d9f0d9f9a2ebef4c448038c139204) )
ROM_END


/* board #  rom         parent    machine    inp        init              monitor, .. */

// Taito games (+clones), starting with Space Invaders
GAME( 1978, sisv1,      invaders, invaders,  sitv,      driver_device, 0, ROT270, "Taito", "Space Invaders (SV Version rev 1)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1978, sisv2,      invaders, invaders,  sitv,      driver_device, 0, ROT270, "Taito", "Space Invaders (SV Version rev 2)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1978, sisv3,      invaders, invaders,  sitv,      driver_device, 0, ROT270, "Taito", "Space Invaders (SV Version rev 3)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1978, sisv,       invaders, invaders,  sitv,      driver_device, 0, ROT270, "Taito", "Space Invaders (SV Version rev 4)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAMEL(1978, sitv1,      invaders, invaders,  sitv,      driver_device, 0, ROT270, "Taito", "Space Invaders (TV Version rev 1)", MACHINE_SUPPORTS_SAVE, layout_invaders )
GAMEL(1978, sitv,       invaders, invaders,  sitv,      driver_device, 0, ROT270, "Taito", "Space Invaders (TV Version rev 2)", MACHINE_SUPPORTS_SAVE, layout_invaders )
GAME( 1979, sicv,       invaders, invadpt2,  sicv,      driver_device, 0, ROT270, "Taito", "Space Invaders (CV Version)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAMEL(1978, invadrmr,   invaders, invaders,  invadrmr,  driver_device, 0, ROT270, "Taito / Model Racing", "Space Invaders (Model Racing)", MACHINE_SUPPORTS_SAVE, layout_invaders ) // unclassified, licensed or bootleg?
GAMEL(1978, invaderl,   invaders, invaders,  sicv,      driver_device, 0, ROT270, "Taito / Logitec", "Space Invaders (Logitec)", MACHINE_SUPPORTS_SAVE, layout_invaders ) // unclassified, licensed or bootleg?
GAMEL(1978, spcewars,   invaders, spcewars,  spcewars,  driver_device, 0, ROT270, "Taito / Sanritsu", "Space War (Sanritsu)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_invaders ) // unclassified, licensed or bootleg?
GAMEL(1978, spceking,   invaders, invaders,  sicv,      driver_device, 0, ROT270, "Taito / Leijac Corporation", "Space King", MACHINE_SUPPORTS_SAVE, layout_invaders ) // unclassified, licensed or bootleg?
GAMEL(1979, cosmicmo,   invaders, cosmicmo,  cosmicmo,  driver_device, 0, ROT270, "Taito / Universal", "Cosmic Monsters (version II)", MACHINE_SUPPORTS_SAVE, layout_cosmicm ) // unclassified, licensed or bootleg?
GAMEL(1979, cosmicm2,   invaders, cosmicmo,  cosmicmo,  driver_device, 0, ROT270, "Taito / Universal", "Cosmic Monsters 2", MACHINE_SUPPORTS_SAVE, layout_cosmicm ) // unclassified, licensed or bootleg?
GAMEL(1980?,sinvzen,    invaders, invaders,  sinvzen,   driver_device, 0, ROT270, "Taito / Zenitone-Microsec Ltd.", "Super Invaders (Zenitone-Microsec)", MACHINE_SUPPORTS_SAVE, layout_invaders ) // unclassified, licensed or bootleg?
GAMEL(1980, ultrainv,   invaders, invaders,  sicv,      driver_device, 0, ROT270, "Taito / Konami", "Ultra Invaders", MACHINE_SUPPORTS_SAVE, layout_invaders ) // unclassified, licensed or bootleg?
GAMEL(1978, spaceatt,   invaders, invaders,  sicv,      driver_device, 0, ROT270, "bootleg (Video Games GmbH)", "Space Attack (bootleg of Space Invaders)", MACHINE_SUPPORTS_SAVE, layout_invaders )
GAMEL(1978, spaceattbp, invaders, invaders,  sicv,      driver_device, 0, ROT270, "bootleg (Video Games GmbH)", "Space Attack (bproms)(bootleg of Space Invaders)", MACHINE_SUPPORTS_SAVE, layout_invaders )
GAMEL(1978, spaceatt2k, invaders, invaders,  sicv,      driver_device, 0, ROT270, "bootleg (Video Games GmbH)", "Space Attack (2k roms)(bootleg of Space Invaders)", MACHINE_SUPPORTS_SAVE, layout_invaders )
GAMEL(1978, galmonst,   invaders, invaders,  sicv,      driver_device, 0, ROT270, "bootleg (Laguna S.A.)", "Galaxy Monsters (Laguna S.A. Spanish bootleg of Space Invaders)", MACHINE_SUPPORTS_SAVE, layout_invaders )
GAMEL(1980, spaceat2,   invaders, invaders,  spaceat2,  driver_device, 0, ROT270, "bootleg (Video Games UK)", "Space Attack II (bootleg of Super Invaders)", MACHINE_SUPPORTS_SAVE, layout_invaders ) // bootleg of Zenitone-Microsec Super Invaders
GAMEL(1979, spacecom,   invaders, spacecom,  spacecom,  _8080bw_state, spacecom, ROT270, "bootleg", "Space Combat (bootleg of Space Invaders)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_spacecom )
GAME( 1978, spacerng,   invaders, spacerng,  sitv,      driver_device, 0, ROT90,  "bootleg (Leisure Time Electronics)", "Space Ranger (bootleg of Space Invaders)", MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND ) // many modifications
GAMEL(19??, invasion,   invaders, invaders,  invasion,  driver_device, 0, ROT270, "bootleg (Sidam)", "Invasion (Sidam)", MACHINE_SUPPORTS_SAVE, layout_invaders )
GAMEL(19??, invasiona,  invaders, invaders,  invasion,  driver_device, 0, ROT270, "bootleg", "Invasion (bootleg set 1, normal graphics)", MACHINE_SUPPORTS_SAVE, layout_invaders ) // has Sidam replaced with 'Ufo Monster Attack' and standard GFX
GAMEL(19??, invasionb,  invaders, invaders,  invasion,  driver_device, 0, ROT270, "bootleg", "Invasion (bootleg set 2, no copyright)", MACHINE_SUPPORTS_SAVE, layout_invaders )
GAMEL(19??, invasionrz, invaders, invaders,  invasion,  driver_device, 0, ROT270, "bootleg (R Z SRL Bologna)", "Invasion (bootleg set 3, R Z SRL Bologna)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING, layout_invaders )
GAMEL(19??, invasionrza,invaders, invaders,  invasion,  driver_device, 0, ROT270, "bootleg (R Z SRL Bologna)", "Invasion (bootleg set 4, R Z SRL Bologna)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING, layout_invaders )
GAMEL(19??, invadersem, invaders, invaders,  sitv,      driver_device, 0, ROT270, "Electromar", "Space Invaders (Electromar, Spanish)", MACHINE_SUPPORTS_SAVE, layout_invaders ) // possibly licensed
GAMEL(1978, superinv,   invaders, invaders,  superinv,  driver_device, 0, ROT270, "bootleg", "Super Invaders (bootleg set 1)", MACHINE_SUPPORTS_SAVE, layout_invaders ) // not related to Zenitone-Microsec version
GAMEL(1978, sinvemag,   invaders, invaders,  sinvemag,  driver_device, 0, ROT270, "bootleg (Emag)", "Super Invaders (bootleg set 2)", MACHINE_SUPPORTS_SAVE, layout_invaders ) // not related to Zenitone-Microsec version
GAMEL(1980, searthin,   invaders, invaders,  searthin,  driver_device, 0, ROT270, "bootleg (Competitive Video)", "Super Earth Invasion (set 1)", MACHINE_SUPPORTS_SAVE, layout_invaders )
GAMEL(1980, searthina,  invaders, invaders,  searthin,  driver_device, 0, ROT270, "bootleg (Competitive Video)", "Super Earth Invasion (set 2)", MACHINE_SUPPORTS_SAVE, layout_invaders )
GAMEL(1980, searthie,   invaders, invaders,  searthin,  driver_device, 0, ROT270, "bootleg (Electrocoin)", "Super Earth Invasion (set 3)", MACHINE_SUPPORTS_SAVE, layout_invaders )
GAMEL(19??, alieninv,   invaders, invaders,  alieninv,  driver_device, 0, ROT270, "bootleg (Margamatics)", "Alien Invasion", MACHINE_SUPPORTS_SAVE, layout_invaders )
GAMEL(19??, alieninvp2, invaders, invaders,  searthin,  driver_device, 0, ROT270, "bootleg", "Alien Invasion Part II", MACHINE_SUPPORTS_SAVE, layout_invaders )
GAMEL(1979, jspecter,   invaders, invaders,  jspecter,  driver_device, 0, ROT270, "bootleg (Jatre)", "Jatre Specter (set 1)", MACHINE_SUPPORTS_SAVE, layout_invaders )
GAMEL(1979, jspecter2,  invaders, invaders,  jspecter,  driver_device, 0, ROT270, "bootleg (Jatre)", "Jatre Specter (set 2)", MACHINE_SUPPORTS_SAVE, layout_invaders )
GAMEL(1978, spacewr3,   invaders, spcewars,  sicv,      driver_device, 0, ROT270, "bootleg", "Space War Part 3", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_invaders ) // unrelated to Sanritsu's version?
GAMEL(1978, invader4,   invaders, invaders,  sicv,      driver_device, 0, ROT270, "bootleg", "Space Invaders Part Four", MACHINE_SUPPORTS_SAVE, layout_invaders )
GAME( 1978, darthvdr,   invaders, darthvdr,  darthvdr,  driver_device, 0, ROT270, "bootleg", "Darth Vader (bootleg of Space Invaders)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAMEL(19??, tst_invd,   invaders, invaders,  sicv,      driver_device, 0, ROT0,   "<unknown>", "Space Invaders Test ROM", MACHINE_SUPPORTS_SAVE, layout_invaders )

// other Taito
GAME( 1979, invadpt2,   0,        invadpt2,  invadpt2,  driver_device, 0, ROT270, "Taito", "Space Invaders Part II (Taito)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1979, invadpt2br, invadpt2, invadpt2,  invadpt2,  driver_device, 0, ROT270, "Taito do Brasil", "Space Invaders Part II (Brazil)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1980, invaddlx,   invadpt2, invaders,  invadpt2,  driver_device, 0, ROT270, "Taito (Midway license)", "Space Invaders Deluxe", MACHINE_SUPPORTS_SAVE )
GAME( 1979, moonbase,   invadpt2, invadpt2,  invadpt2,  driver_device, 0, ROT270, "Taito / Nichibutsu", "Moon Base (set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND ) // this has a 'Taito Corp' string hidden away in the rom - how do you get it to display?
GAME( 1979, moonbasea,  invadpt2, invadpt2,  invadpt2,  driver_device, 0, ROT270, "Taito / Nichibutsu", "Moon Base (set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND ) // this has the same string replaced with Nichibutsu, no other differences

GAME( 1980, spclaser,   0,        invadpt2,  spclaser,  driver_device, 0, ROT270, "Taito", "Space Laser", MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1980, intruder,   spclaser, invadpt2,  spclaser,  driver_device, 0, ROT270, "Taito (Game Plan license)", "Intruder", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1980, laser,      spclaser, invadpt2,  spclaser,  driver_device, 0, ROT270, "bootleg (Leisure Time Electronics Inc.)", "Astro Laser (bootleg of Space Laser)", MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1979, spcewarl,   spclaser, invadpt2,  spclaser,  driver_device, 0, ROT270, "Leijac Corporation", "Space War (Leijac Corporation)", MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE ) // Taito's version is actually a spin-off of this?

GAME( 1979, lrescue,    0,        lrescue,   lrescue,   driver_device, 0, ROT270, "Taito", "Lunar Rescue", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1979, grescue,    lrescue,  lrescue,   lrescue,   driver_device, 0, ROT270, "Taito (Universal license?)", "Galaxy Rescue", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1980, mlander,    lrescue,  lrescue,   lrescue,   driver_device, 0, ROT270, "bootleg (Leisure Time Electronics)", "Moon Lander (bootleg of Lunar Rescue)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, lrescuem,   lrescue,  lrescue,   lrescue,   driver_device, 0, ROT270, "bootleg (Model Racing)", "Lunar Rescue (Model Racing bootleg, set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1979, lrescuem2,  lrescue,  lrescue,   lrescue,   driver_device, 0, ROT270, "bootleg (Model Racing)", "Lunar Rescue (Model Racing bootleg, set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1979, desterth,   lrescue,  lrescue,   lrescue,   driver_device, 0, ROT270, "bootleg", "Destination Earth (bootleg of Lunar Rescue)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )

GAME( 1979, schaser,    0,        schaser,   schaser,   driver_device, 0, ROT270, "Taito", "Space Chaser (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, schasera,   schaser,  schaser,   schaser,   driver_device, 0, ROT270, "Taito", "Space Chaser (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, schaserb,   schaser,  schaser,   schaser,   driver_device, 0, ROT270, "Taito", "Space Chaser (set 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, schaserc,   schaser,  schaser,   schaser,   driver_device, 0, ROT270, "Taito", "Space Chaser (set 4)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_COLORS )
GAME( 1979, schasercv,  schaser,  schasercv, schasercv, driver_device, 0, ROT270, "Taito", "Space Chaser (CV version - set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_COLORS )
GAME( 1979, schaserm,   schaser,  schaser,   schaserm,  driver_device, 0, ROT270, "bootleg (Model Racing)", "Space Chaser (Model Racing bootleg)", MACHINE_SUPPORTS_SAVE ) // on original Taito PCB, hacked to be harder?

GAME( 1979, sflush,     0,        sflush,    sflush,    driver_device, 0, ROT270, "Taito", "Straight Flush", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND | MACHINE_IMPERFECT_COLORS | MACHINE_NO_COCKTAIL)

GAME( 1980, lupin3,     0,        lupin3,    lupin3,    driver_device, 0, ROT270, "Taito", "Lupin III (set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1980, lupin3a,    lupin3,   lupin3a,   lupin3a,   driver_device, 0, ROT270, "Taito", "Lupin III (set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )

GAME( 1980, polaris,    0,        polaris,   polaris,   driver_device, 0, ROT270, "Taito", "Polaris (Latest version)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, polarisa,   polaris,  polaris,   polaris,   driver_device, 0, ROT270, "Taito", "Polaris (First revision)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, polariso,   polaris,  polaris,   polaris,   driver_device, 0, ROT270, "Taito", "Polaris (Original version)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, polarisbr,  polaris,  polaris,   polaris,   driver_device, 0, ROT270, "Taito do Brasil", "Polaris (Brazil)", MACHINE_SUPPORTS_SAVE )

GAME( 1980, ballbomb,   0,        ballbomb,  ballbomb,  driver_device, 0, ROT270, "Taito", "Balloon Bomber", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )  /* missing clouds */

GAME( 1980, indianbt,   0,        indianbt,  indianbt,  driver_device, 0, ROT270, "Taito", "Indian Battle", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1983, indianbtbr, indianbt, indianbtbr,indianbtbr,driver_device, 0, ROT270, "Taito do Brasil", "Indian Battle (Brazil)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )

GAME( 1980, steelwkr,   0,        steelwkr,  steelwkr,  driver_device, 0, ROT0  , "Taito", "Steel Worker", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )

GAMEL(1980?,galactic,   0,        invaders,  galactic,  driver_device, 0, ROT270, "Taito do Brasil", "Galactica - Batalha Espacial", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND, layout_galactic )
GAMEL(1980?,spacmiss,   galactic, invaders,  galactic,  driver_device, 0, ROT270, "bootleg?", "Space Missile - Space Fighting Game", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND, layout_galactic )

// Misc. manufacturers
GAME( 1979, galxwars,   0,        invadpt2,  galxwars,  driver_device, 0, ROT270, "Universal", "Galaxy Wars (Universal set 1)", MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1979, galxwars2,  galxwars, invadpt2,  galxwars,  driver_device, 0, ROT270, "Universal", "Galaxy Wars (Universal set 2)", MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1979, galxwarst,  galxwars, invadpt2,  galxwars,  driver_device, 0, ROT270, "Universal (Taito license?)", "Galaxy Wars (Taito?)" , MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE ) // Copyright not displayed
GAME( 1979, starw,      galxwars, invaders,  galxwars,  driver_device, 0, ROT270, "bootleg", "Star Wars (bootleg of Galaxy Wars, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, starw1,     galxwars, starw1,    galxwars,  driver_device, 0, ROT270, "bootleg (Yamashita)", "Star Wars (bootleg of Galaxy Wars, set 2)", MACHINE_SUPPORTS_SAVE )

GAME( 1979, cosmo,      0,        cosmo,     cosmo,     driver_device, 0, ROT90,  "TDS & MINTS", "Cosmo", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )

GAME( 1980?,invrvnge,   0,        invrvnge,  invrvnge,  driver_device, 0, ROT270, "Zenitone-Microsec Ltd.", "Invader's Revenge (set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND ) // copyright is either late-1980, or early-1981
GAME( 1980?,invrvngea,  invrvnge, invrvnge,  invrvnge,  driver_device, 0, ROT270, "Zenitone-Microsec Ltd.", "Invader's Revenge (set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND )
GAME( 1980?,invrvngeb,  invrvnge, invrvnge,  invrvnge,  driver_device, 0, ROT270, "Zenitone-Microsec Ltd.", "Invader's Revenge (set 3)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND )
GAME( 1980?,invrvngedu, invrvnge, invrvnge,  invrvnge,  driver_device, 0, ROT270, "Zenitone-Microsec Ltd. (Dutchford license)", "Invader's Revenge (Dutchford, single PCB)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND )
GAME( 1980?,invrvngegw, invrvnge, invrvnge,  invrvnge,  driver_device, 0, ROT270, "Zenitone-Microsec Ltd. (Game World license)", "Invader's Revenge (Game World, single PCB)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND )

GAME( 1980, vortex,     0,        vortex,    vortex,    _8080bw_state, vortex, ROT270, "Zilec Electronics", "Vortex", MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND ) /* Encrypted 8080/IO */

GAME( 1979, rollingc,   0,        rollingc,  rollingc,  driver_device, 0, ROT270, "Nichibutsu", "Rolling Crash / Moon Base", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )

GAME( 1979, ozmawars,   0,        invaders,  ozmawars,  driver_device, 0, ROT270, "SNK", "Ozma Wars (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, ozmawars2,  ozmawars, invaders,  ozmawars,  driver_device, 0, ROT270, "SNK", "Ozma Wars (set 2)", MACHINE_SUPPORTS_SAVE ) /* Uses Taito's three board color version of Space Invaders PCB */
GAME( 1979, spaceph,    ozmawars, invaders,  spaceph,   driver_device, 0, ROT270, "bootleg? (Zilec Games)", "Space Phantoms (bootleg of Ozma Wars)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, solfight,   ozmawars, invaders,  ozmawars,  driver_device, 0, ROT270, "bootleg", "Solar Fight (bootleg of Ozma Wars)", MACHINE_SUPPORTS_SAVE )

GAME( 1979, yosakdon,   0,        yosakdon,  yosakdon,  driver_device, 0, ROT270, "Wing", "Yosaku To Donbei (set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1979, yosakdona,  yosakdon, yosakdon,  yosakdon,  driver_device, 0, ROT270, "Wing", "Yosaku To Donbei (set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )

GAMEL(1979, shuttlei,   0,        shuttlei,  shuttlei,  driver_device, 0, ROT270, "Omori Electric Co., Ltd.", "Shuttle Invader", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND, layout_shuttlei )

GAMEL(1979, skylove,    0,        shuttlei,  skylove,   driver_device, 0, ROT270, "Omori Electric Co., Ltd.", "Sky Love", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND, layout_shuttlei )

GAME (1978, claybust,   0,        claybust,  claybust,  driver_device, 0, ROT0,   "Model Racing", "Claybuster", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND ) // no titlescreen, Claybuster according to flyers

GAMEL(1980, gunchamp,   0,        claybust,  gunchamp,  driver_device, 0, ROT0,   "Model Racing", "Gun Champ", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND, layout_gunchamp ) // no titlescreen, Gun Champ according to original cab

GAME( 1980?,astropal,   0,        astropal,  astropal,  driver_device, 0, ROT0,   "Sidam?", "Astropal", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )

GAMEL(1979?,attackfc,   0,        attackfc,  attackfc,  _8080bw_state, attackfc, ROT0, "Electronic Games Systems", "Attack Force", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND, layout_attackfc )

GAME( 2002, invmulti,   0,        invmulti,  invmulti,  _8080bw_state, invmulti, ROT270, "hack (Braze Technologies)", "Space Invaders Multigame (M8.03D)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2002, invmultim3a,invmulti, invmulti,  invmulti,  _8080bw_state, invmulti, ROT270, "hack (Braze Technologies)", "Space Invaders Multigame (M8.03A)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2002, invmultim2c,invmulti, invmulti,  invmulti,  _8080bw_state, invmulti, ROT270, "hack (Braze Technologies)", "Space Invaders Multigame (M8.02C)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2002, invmultim2a,invmulti, invmulti,  invmulti,  _8080bw_state, invmulti, ROT270, "hack (Braze Technologies)", "Space Invaders Multigame (M8.02A)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2002, invmultim1a,invmulti, invmulti,  invmulti,  _8080bw_state, invmulti, ROT270, "hack (Braze Technologies)", "Space Invaders Multigame (M8.01A)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2002, invmultit3d,invmulti, invmulti,  invmulti,  _8080bw_state, invmulti, ROT270, "hack (Braze Technologies)", "Space Invaders Multigame (T8.03D)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2002, invmultis3a,invmulti, invmulti,  invmulti,  _8080bw_state, invmulti, ROT270, "hack (Braze Technologies)", "Space Invaders Multigame (S0.83A)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2002, invmultis2a,invmulti, invmulti,  invmulti,  _8080bw_state, invmulti, ROT270, "hack (Braze Technologies)", "Space Invaders Multigame (S0.82A)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2002, invmultis1a,invmulti, invmulti,  invmulti,  _8080bw_state, invmulti, ROT270, "hack (Braze Technologies)", "Space Invaders Multigame (S0.81A)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2002, invmultip,  invmulti, invmulti,  invmulti,  _8080bw_state, invmulti, ROT270, "hack (Braze Technologies)", "Space Invaders Multigame (prototype)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
