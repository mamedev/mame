// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Bally/Sente SAC-1 system

    driver by Aaron Giles

    Games supported:
        * Chicken Shift (11/23/84)
        * Gimme A Break (7/7/85)
        * Goalie Ghost
        * Grudge Match (v00.90, Italy, location test?)
        * Grudge Match (v00.80, prototype)
        * Hat Trick  (11/12/84)
        * Mini Golf (set 1)
        * Mini Golf (11/25/85)
        * Mini Golf (10/8/85)
        * Name That Tune (3/31/86)
        * Name That Tune (3/23/86)
        * Night Stocker (10/6/86)
        * Night Stocker (8/27/86)
        * Off the Wall (10/16/84)
        * Rescue Raider (5/11/87) (non-cartridge)
        * Rescue Raider (stand-alone)
        * Sente Diagnostic Cartridge
        * Shrike Avenger (prototype)
        * Snacks'n Jaxson
        * Snake Pit
        * Snake Pit (9/14/84)
        * Spiker (6/9/86)
        * Spiker (5/5/86)
        * Spiker (earliest)
        * Stocker (3/19/85)
        * Stompin' (4/4/86)
        * Street Football (11/12/86)
        * Team Hat Trick  (11/16/84)
        * Toggle (prototype)
        * Trivial Pursuit (Think Tank - Genus Edition) (2/12/85)
        * Trivial Pursuit (Think Tank - Genus Edition) (12/14/84)
        * Trivial Pursuit (Baby Boomer Edition) (3/20/85)
        * Trivial Pursuit (Genus II Edition) (3/22/85)
        * Trivial Pursuit (Young Players Edition) (3/29/85)
        * Trivial Pursuit (All Star Sports Edition)
        * Trivial Pursuit (Volumen III, Spanish, Maibesa license)
        * Trivial Pursuit (Volumen II, Spanish, Maibesa license)
        * Trivial Pursuit (Volumen IV, Spanish, Maibesa hardware)
        * Trivial Pursuit (Volumen V, Spanish, Maibesa hardware)

    Looking for ROMs for these:
        * Euro Stocker
        * Trivial Pursuit (Volumen I, Spanish, Maibesa)

    Known bugs:
        * CEM3394 emulation is not perfect
        * Shrike Avenger doesn't work properly
        * triviaes4 and triviaes5 sets run on different hardware (from Maibesa) which isn't emulated yet

    Other:
        * Some of the cartridge types are unknown
        * Do any of the remaining unknown cartridge types contain a PAL
          that needs to be dumped?

****************************************************************************

    Memory map

****************************************************************************

    ========================================================================
    68A09E CPU
    ========================================================================
    0000-007F   R/W   xxxxxxxx    Sprite RAM (32 entries x 4 bytes)
                R/W   x-------       (0: Vertical flip)
                R/W   -x------       (0: Horizontal flip)
                R/W   ------xx       (0: Upper 2 bits of image number)
                R/W   xxxxxxxx       (1: Lower 8 bits of image number)
                R/W   xxxxxxxx       (2: Y position, offset by 17 pixels)
                R/W   xxxxxxxx       (3: X position)
    0080-00DF   R/W   xxxxxxxx    Program RAM
    00E0-00FF   R/W   xxxxxxxx    Additional sprite RAM (8 entries x 4 bytes)
    0100-07FF   R/W   xxxxxxxx    Program RAM
    0800-7FFF   R/W   xxxxxxxx    Video RAM (256x240 pixels)
                R/W   xxxx----       (left pixel)
                R/W   ----xxxx       (right pixel)
    8000-8FFF   R/W   ----xxxx    Palette RAM (1024 entries x 4 bytes)
                R/W   ----xxxx       (0: red entry)
                R/W   ----xxxx       (1: green entry)
                R/W   ----xxxx       (2: blue entry)
    9000-9007     W   --------    ADC start trigger, inputs 0-7
    9400        R     xxxxxxxx    ADC data read
    9800-9801     W   x-------    External output #0
    9802-9803     W   x-------    External output #1
    9804-9805     W   x-------    External output #2
    9806-9807     W   x-------    External output #3
    9808-9809     W   x-------    External output #4
    980A-980B     W   x-------    External output #5
    980C-980D     W   x-------    External output #6
    980E-980F     W   x-------    NVRAM recall
    9880          W   --------    Random number generator reset
    98A0          W   -xxx----    A000-DFFF bank select
    98C0          W   ------xx    Palette bank select
    98E0          W   --------    Watchdog reset
    9900        R     xxxxxxxx    DIP switch bank 1 (G) (active low)
    9901        R     xxxxxxxx    DIP switch bank 2 (H) (active low)
    9902        R     x-------    Self test (active low)
                R     -x------    Left coin (active low)
                R     --xxxxxx    External inputs (active low)
    9903        R     x-------    VBLANK state (active high)
                R     -x------    Right coin (active low)
                R     --xxxx--    External inputs (active low)
                R     ------x-    Player 2 start (active low)
                R     -------x    Player 1 start (active low)
    9A00        R     xxxxxxxx    Random number generator
    9A04-9A05   R/W   xxxxxxxx    6850 UART I/O (to sound board)
    9B00-9CFF   R/W   xxxxxxxx    NOVRAM
    9F00          W   --x--xxx    Independent bank select (Night Stocker only?)
    9e00-9fff   R/W               Shrike Avenger shares with 68k at 0x18000 (see Shrike notes below)
    A000-BFFF   R     xxxxxxxx    Banked A/B ROM
    C000-DFFF   R     xxxxxxxx    Banked C/D ROM
    E000-FFFF   R     xxxxxxxx    Fixed program ROM
    ========================================================================
    Interrupts:
        NMI not connected
        IRQ generated by 32L
        FIRQ generated by 6850 UART
    ========================================================================


    ========================================================================
    Shrike SHM
    Many thanks to Owen Rubin and Brian Deuel ( http://www.atarimuseum.com/orubin/ ) for their time,
    interest, and memories!

    From Owen: The motor drive included 2 motors side by side at the rear of the cabinet with a U joint pivot
    at the front. L & R motors were used independently for side to side "roll" motion, and together for pitch.
    The motors were guarded by two sets of h/w limit switches - stop switch and (emergency) auto-reverse
    switch - in tandem with soft limiting. The software calibrated the motors by running the motors slowly to
    full limits and using the data for the soft limiting. (max chops?)

    The proto was never completed, there was to be a final round against a mother ship where you would have
    to shoot out 4 engines and a target array. (He thinks there was another bank of sprite ROMs for this that
    may never have been included.) He also says 'There was going to be a "death blossom" shot you could
    use once that would have been a wild ride as well, but that motion was VERY tough in the simulator, so I
    did not complete it.'

    Owen's recollection of the motion diagnostics screen, the second cursor is the controllers feedback
    and should match the yoke cursor. Two of the channels (sine/bar) are probably calculated/reported
    motor pos. Red sine meant over/underspeed or calculated/reported discrepancy. All memories came with
    a disclaimer ;)

    Shrike shares 9e00 - 9fff as 18000 - 181ff, 9e00-9e0f as registers, and the rest as GFX RAM.
    10000-1001f appear to be the interface to the motors/sensors.

    For more detailed (but unfinished as yet) disassembly of 68000, get me at my hotmail address, 'nuapete'
    ========================================================================
    m6809        m68000
    9e00 RW - RW 18000 ($0,A3) : 6809 command register, commands in range 0-19
                                cmd $0 nop
                                cmd $10 check RAM
                                cmd $11 check u22 ( 0000-3FFE )
                                cmd $12 check u24 ( 0001-3FFF )
                                cmd $13 check "u26" ( 8000-BFFE ) \ these appear to be for unused expansion slots
                                cmd $14 check "u28" ( 8001-BFFF ) /
                                cmd $15 check IRQs
                                cmd $16 check FIRQs
                                cmd $17 fetch max chops
                                cmd $18 fetch pulse width
    9e01 W  - R  18001 ($1,A3) : &0x80 sprite bank select
    9e02 W  - R  18002 ($2,A3) : \ joy x
    9e03 W  - R  18003 ($3,A3) : / joy y
    9e04 R  - W  18004 ($4,A3) : \ cursor y pos in diags screen
    9e05 R  - W  18005 ($5,A3) : / cursor x pos in diags screen
    9e06 R  - W  18006 ($6,A3) : 68k status
                                    00 = OK
                                    02 = cmd 3 or a failed
                                    01 = Initial status (not OK)
                                    10 = RAM bad
                                    11 = ROM(s) bad
                                    15 = IRQs bad
                                    16 = FIRQs bad
                                    F7 = 68k didn't get handshake from 6809
                                    F8 = Too many spurious interrupts
                                    F9 = Both limit switches at once
                                    FA = 120 Hz signal slower than 80Hz
                                    FB = Excess current sensor not working
                                    FC = Motor range outside of expected
                                    FD = Failed to detect limit switch
                                    FE = No mech movement detected
                                    FF = Excess current for too long
    9e07 W  -  R 18007 ($7,A3) : \ writes random stuff from 9A00 which is the random number generator?
    9e08 RW -  R 18008 ($8,A3) : / as 9e07
    9e09 RW -  W 18009 ($9,A3) : \ 68k watchdog writes 0xaa
    9e0a W  - RW 1800a ($a,A3) : / 6809 watchdog writes 0x55
    9e0b    - RW 1800b ($b,A3) : Only writes are 0
    9e0c R  - RW 1800c ($c,A3) : \ ypos returned from controller (affects enemy ship pos)
    9e0d R  -  W 1800d ($d,A3) : / xpos returned from controller
    9e0e W  -  R 1800e ($e,A3) : \
    9e0f W  -    1800f ($f,A3) : / partial pointer into SHM gfx data

***************************************************************************

DIP locations verified for:
    - stompin (manual)

***************************************************************************/

#include "emu.h"
#include "balsente.h"

#include "cpu/z80/z80.h"
#include "cpu/m6809/m6809.h"
#include "cpu/m68000/m68000.h"
#include "machine/nvram.h"
#include "machine/watchdog.h"
#include "sente6vb.h"
#include "machine/6821pia.h"
#include "machine/z80ctc.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"
#include "video/mc6845.h"
#include "speaker.h"

#include "stocker.lh"



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void balsente_state::cpu1_base_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("spriteram");
	map(0x0800, 0x7fff).ram().w(FUNC(balsente_state::videoram_w)).share("videoram");
	map(0x8000, 0x8fff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x9000, 0x9007).w(FUNC(balsente_state::adc_select_w));
	map(0x9400, 0x9401).r(FUNC(balsente_state::adc_data_r));
	map(0x9800, 0x981f).mirror(0x0060).lw8(NAME([this] (offs_t offset, u8 data) { m_outlatch->write_d7(offset >> 2, data); }));
	map(0x9880, 0x989f).w(FUNC(balsente_state::random_reset_w));
	map(0x98a0, 0x98bf).w(FUNC(balsente_state::rombank_select_w));
	map(0x98c0, 0x98df).w(FUNC(balsente_state::palette_select_w));
	map(0x98e0, 0x98ff).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x9900, 0x9900).portr("SWH");
	map(0x9901, 0x9901).portr("SWG");
	map(0x9902, 0x9902).portr("IN0");
	map(0x9903, 0x9903).portr("IN1").nopw();
	map(0x9a00, 0x9a03).r(FUNC(balsente_state::random_num_r));
	map(0x9a04, 0x9a05).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0xa000, 0xbfff).bankr("bankab");
	map(0xc000, 0xdfff).bankr("bankcd");
	map(0xe000, 0xffff).bankr("bankef");
}

void balsente_state::cpu1_map(address_map &map)
{
	cpu1_base_map(map);
	map(0x9b00, 0x9bff).rw("nov0", FUNC(x2212_device::read), FUNC(x2212_device::write));
	map(0x9c00, 0x9cff).rw("nov1", FUNC(x2212_device::read), FUNC(x2212_device::write));
}

void balsente_state::cpu1_teamht_map(address_map &map)
{
	cpu1_map(map);
	map(0x9404, 0x9404).r(FUNC(balsente_state::teamht_extra_r));
	map(0x9000, 0x9007).w(FUNC(balsente_state::teamht_multiplex_select_w));
}

void balsente_state::cpu1_grudge_map(address_map &map)
{
	cpu1_map(map);
	map(0x9400, 0x9400).r(FUNC(balsente_state::grudge_steering_r));
}

void balsente_state::cpu1_st1002_map(address_map &map)
{
	cpu1_map(map);
	map(0x9f00, 0x9f00).w(FUNC(balsente_state::rombank2_select_w));
}

void balsente_state::cpu1_spiker_map(address_map &map)
{
	cpu1_st1002_map(map);
	map(0x9f80, 0x9f8f).rw(FUNC(balsente_state::spiker_expand_r), FUNC(balsente_state::spiker_expand_w));
}

void balsente_state::cpu1_shrike_map(address_map &map)
{
	cpu1_map(map);
	map(0x9e00, 0x9fff).rw(FUNC(balsente_state::shrike_shared_6809_r), FUNC(balsente_state::shrike_shared_6809_w));
	map(0x9e01, 0x9e01).w(FUNC(balsente_state::shrike_sprite_select_w));
}

void balsente_state::cpu1_smudge_map(address_map &map)
{
	cpu1_base_map(map);
	map(0x9b00, 0x9bff).rw(FUNC(balsente_state::novram_8bit_r), FUNC(balsente_state::novram_8bit_w));
}

void balsente_state::cpu1_triviamb_map(address_map &map)
{
	map(0x0000, 0x00ff).ram().share("spriteram");
	map(0x0100, 0x0100).portr("SWH");
	map(0x0101, 0x0101).portr("SWG");
	map(0x0102, 0x0102).portr("IN0");
	map(0x0103, 0x0103).portr("IN1");
	map(0x0320, 0x0323).rw("pia", FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt));
	map(0x0340, 0x0340).rw("crtc", FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x0341, 0x0341).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x0360, 0x0360).w(FUNC(balsente_state::random_reset_w));
	map(0x0380, 0x0380).r(FUNC(balsente_state::random_num_r));
	map(0x03a0, 0x03a0).w(FUNC(balsente_state::rombank_select_w));
	map(0x03c0, 0x03c0).w(FUNC(balsente_state::palette_select_w));
	map(0x03e0, 0x03e0).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x0800, 0x7fff).ram().w(FUNC(balsente_state::videoram_w)).share("videoram");
	map(0x8000, 0x83ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x8800, 0x8fff).ram().share("nvram");
	map(0xa000, 0xbfff).bankr("bankab");
	map(0xc000, 0xdfff).bankr("bankcd");
	map(0xe000, 0xffff).bankr("bankef");
}



/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

// TODO: banking (Trivial hardware from Maibesa)
void balsente_state::cpu2_triviamb_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).ram();
}

// TODO: hookup 2x Z80CTC, 2x AY8910A, 1x M5205 (Trivial hardware from Maibesa)
void balsente_state::cpu2_triviamb_io_map(address_map &map)
{
	map.global_mask(0xff);
}


/*************************************
 *
 *  Shrike Avenger CPU memory handlers
 *
 *************************************/

/* CPU 1 read addresses */
void balsente_state::shrike68k_map(address_map &map)
{
	map(0x000000, 0x003fff).rom();
	map(0x010000, 0x01001f).ram().share("shrike_io").rw(FUNC(balsente_state::shrike_io_68k_r), FUNC(balsente_state::shrike_io_68k_w));
	map(0x018000, 0x018fff).ram().share("shrike_shared");
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

#define UNUSED_ANALOG PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

#define BALSENTE_COINAGE_ALT \
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )      PORT_DIPLOCATION("H1:1,2") \
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )

#define BALSENTE_COINAGE_ALT2 \
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("H1:1,2") \
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )

#define BALSENTE_PLAYERS_PER_COIN \
	PORT_DIPNAME( 0x04, 0x04, "Players Per Credit" )    PORT_DIPLOCATION("H1:3") \
	PORT_DIPSETTING(    0x00, "1" ) \
	PORT_DIPSETTING(    0x04, "1 Or 2" )

static INPUT_PORTS_START( sentetst )
	PORT_START("SWH")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )      PORT_DIPLOCATION("H1:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "H1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "H1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "H1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "H1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "H1:7" )
	PORT_DIPNAME( 0x80, 0x80, "High Scores" )           PORT_DIPLOCATION("H1:8")
	PORT_DIPSETTING(    0x80, "Keep Top 5" )
	PORT_DIPSETTING(    0x00, "Keep All" )

	PORT_START("SWG")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("G1:1,2")
	PORT_DIPSETTING(    0x00, "Every 10,000" )
	PORT_DIPSETTING(    0x01, "Every 15,000" )
	PORT_DIPSETTING(    0x02, "Every 20,000" )
	PORT_DIPSETTING(    0x03, "Every 25,000" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Lives ) )        PORT_DIPLOCATION("G1:3,4")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "G1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "G1:6" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("G1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("G1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x38, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	/* Analog ports */
	PORT_START("AN0")
	UNUSED_ANALOG

	PORT_START("AN1")
	UNUSED_ANALOG

	/* Player 1 Trackball */
	PORT_START("AN2")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_RESET PORT_PLAYER(1)

	PORT_START("AN3")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_RESET PORT_REVERSE PORT_PLAYER(1)
INPUT_PORTS_END


static INPUT_PORTS_START( cshift )
	PORT_INCLUDE( sentetst )

	PORT_MODIFY("SWG")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "G1:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "G1:2" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) )        PORT_DIPLOCATION("G1:3")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "G1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "G1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "G1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "G1:7" )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x3c, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x3c, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("AN2")
	UNUSED_ANALOG

	PORT_MODIFY("AN3")
	UNUSED_ANALOG
INPUT_PORTS_END


static INPUT_PORTS_START( gghost )
	PORT_INCLUDE( sentetst )

	PORT_MODIFY("SWH")
	BALSENTE_PLAYERS_PER_COIN
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "H1:7" )

	PORT_MODIFY("SWG")
	PORT_DIPNAME( 0x07, 0x05, "Game Duration" )         PORT_DIPLOCATION("G1:1,2,3")
	PORT_DIPSETTING(    0x00, "9 Points" )
	PORT_DIPSETTING(    0x02, "11 Points" )
	PORT_DIPSETTING(    0x04, "15 Points" )
	PORT_DIPSETTING(    0x06, "21 Points" )
	PORT_DIPSETTING(    0x01, "Timed, 1:15" )
	PORT_DIPSETTING(    0x03, "Timed, 1:30" )
	PORT_DIPSETTING(    0x05, "Timed, 2:00" )
	PORT_DIPSETTING(    0x07, "Timed, 2:30" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "G1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "G1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "G1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "G1:7" )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )

	/* Player 2 Trackball */
	PORT_MODIFY("AN0")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_RESET PORT_PLAYER(2)

	PORT_MODIFY("AN1")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_RESET PORT_REVERSE PORT_PLAYER(2)

	/* Player 1 Trackball */
	PORT_MODIFY("AN2")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_RESET PORT_PLAYER(1)

	PORT_MODIFY("AN3")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_RESET PORT_REVERSE PORT_PLAYER(1)
INPUT_PORTS_END


static INPUT_PORTS_START( hattrick )
	PORT_INCLUDE( sentetst )

	PORT_MODIFY("SWH")
	BALSENTE_PLAYERS_PER_COIN
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "H1:8" )

	PORT_MODIFY("SWG")
	PORT_DIPNAME( 0x07, 0x02, DEF_STR( Game_Time ) )    PORT_DIPLOCATION("G1:1,2,3")
	PORT_DIPSETTING(    0x00, "1:15" )
	PORT_DIPSETTING(    0x01, "1:30" )
	PORT_DIPSETTING(    0x02, "1:45" )
	PORT_DIPSETTING(    0x03, "2:00" )
	PORT_DIPSETTING(    0x04, "2:15" )
	PORT_DIPSETTING(    0x05, "2:30" )
	PORT_DIPSETTING(    0x06, "2:45" )
	PORT_DIPSETTING(    0x07, "3:00" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "G1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "G1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "G1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "G1:7" )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)

	PORT_MODIFY("AN2")
	UNUSED_ANALOG

	PORT_MODIFY("AN3")
	UNUSED_ANALOG
INPUT_PORTS_END

static INPUT_PORTS_START( teamht )
	PORT_INCLUDE( sentetst )

	PORT_MODIFY("SWH")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )      PORT_DIPLOCATION("H1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x1c, 0x00, "Bonus Coin" )    PORT_DIPLOCATION("H1:3,4,5")
	PORT_DIPSETTING(    0x00, "None" )
	PORT_DIPSETTING(    0x04, "2 Coins +1" )
	PORT_DIPSETTING(    0x08, "3 Coins +1" )
	PORT_DIPSETTING(    0x0c, "4 Coins +1" )
	PORT_DIPSETTING(    0x10, "4 Coins +2" )
	PORT_DIPSETTING(    0x14, "5 Coins +1" )
	PORT_DIPSETTING(    0x18, "5 Coins +2" )
	PORT_DIPSETTING(    0x1c, "5 Coins +3" )
	PORT_DIPNAME( 0x20, 0x00, "Left Multiplier" )    PORT_DIPLOCATION("H1:6")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPNAME( 0xc0, 0x00, "Right Multiplier" )    PORT_DIPLOCATION("H1:7,8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0xc0, "6" )

	PORT_MODIFY("SWG")
	PORT_DIPNAME( 0x07, 0x02, DEF_STR( Game_Time ) )    PORT_DIPLOCATION("G1:1,2,3")
	PORT_DIPSETTING(    0x00, "1:15" )
	PORT_DIPSETTING(    0x01, "1:30" )
	PORT_DIPSETTING(    0x02, "1:45" )
	PORT_DIPSETTING(    0x03, "2:00" )
	PORT_DIPSETTING(    0x04, "2:15" )
	PORT_DIPSETTING(    0x05, "2:30" )
	PORT_DIPSETTING(    0x06, "2:45" )
	PORT_DIPSETTING(    0x07, "3:00" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "G1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "G1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "G1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "G1:7" )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("AN2")
	UNUSED_ANALOG

	PORT_MODIFY("AN3")
	UNUSED_ANALOG

	PORT_START("EX0")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)

	PORT_START("EX1")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)

	// reversed wrt p1 and p2
	PORT_START("EX2")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)

	PORT_START("EX3")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4)
INPUT_PORTS_END


static INPUT_PORTS_START( otwalls )
	PORT_INCLUDE( sentetst )

	PORT_MODIFY("SWH")
	BALSENTE_PLAYERS_PER_COIN
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "H1:8" )

	PORT_MODIFY("SWG")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "G1:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "G1:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "G1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "G1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "G1:7" )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_PLAYER(1)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )

	/* Player 1 Dial */
	PORT_MODIFY("AN2")
	PORT_BIT( 0xff, 0, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_RESET PORT_REVERSE PORT_PLAYER(1)

	/* Player 2 Dial */
	PORT_MODIFY("AN3")
	PORT_BIT( 0xff, 0, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_RESET PORT_REVERSE PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( snakjack )
	PORT_INCLUDE( sentetst )

	PORT_MODIFY("SWG")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("G1:1,2")
	PORT_DIPSETTING(    0x00, "Every 15,000" )
	PORT_DIPSETTING(    0x01, "Every 20,000" )
	PORT_DIPSETTING(    0x02, "Every 25,000" )
	PORT_DIPSETTING(    0x03, "Every 30,000" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "G1:7" )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( stocker )
	PORT_INCLUDE( sentetst )

	PORT_MODIFY("SWH")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )      PORT_DIPLOCATION("H1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )

	PORT_DIPNAME( 0x1c, 0x00, "Bonus Coins" )       PORT_DIPLOCATION("H1:3,4,5")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x04, "2 Coins = 1 Bonus" )
	PORT_DIPSETTING(    0x08, "3 Coins = 1 Bonus" )
	PORT_DIPSETTING(    0x0c, "4 Coins = 1 Bonus" )
	PORT_DIPSETTING(    0x10, "4 Coins = 2 Bonus" )
	PORT_DIPSETTING(    0x14, "5 Coins = 1 Bonus" )
	PORT_DIPSETTING(    0x18, "5 Coins = 2 Bonus" )
	PORT_DIPSETTING(    0x1c, "5 Coins = 3 Bonus" )
	PORT_DIPNAME( 0x20, 0x00, "Left Coin Mech" )    PORT_DIPLOCATION("H1:6")
	PORT_DIPSETTING(    0x00, "x1" )
	PORT_DIPSETTING(    0x20, "x2" )
	PORT_DIPNAME( 0xc0, 0x00, "Right Coin Mech" )   PORT_DIPLOCATION("H1:7,8")
	PORT_DIPSETTING(    0x00, "x1" )
	PORT_DIPSETTING(    0x40, "x4" )
	PORT_DIPSETTING(    0x80, "x5" )
	PORT_DIPSETTING(    0xc0, "x6" )

	PORT_MODIFY("SWG")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "G1:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "G1:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "G1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "G1:4" )
	PORT_DIPNAME( 0x40, 0x40, "End Of Game" )       PORT_DIPLOCATION("G1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "3 Tickets" )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("AN2")
	UNUSED_ANALOG

	/* Player 1 Wheel */
	PORT_MODIFY("AN3")
	PORT_BIT( 0xff, 0, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_RESET PORT_PLAYER(1)
INPUT_PORTS_END


static INPUT_PORTS_START( triviag1 )
	PORT_INCLUDE( sentetst )

	PORT_MODIFY("SWH")
	PORT_DIPNAME( 0x20, 0x00, "Sound" )         PORT_DIPLOCATION("H1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Sound Test" )    PORT_DIPLOCATION("H1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "High Scores" )   PORT_DIPLOCATION("H1:8")
	PORT_DIPSETTING(    0x00, "Keep Top 5" )
	PORT_DIPSETTING(    0x80, "Keep Top 10" )

	PORT_MODIFY("SWG")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "G1:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "G1:2" )
	PORT_DIPNAME( 0x0c, 0x04, "Guesses" )       PORT_DIPLOCATION("G1:3,4")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x0c, "6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "G1:7" )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Red Button")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Green Button")

	PORT_MODIFY("AN2")
	UNUSED_ANALOG

	PORT_MODIFY("AN3")
	UNUSED_ANALOG
INPUT_PORTS_END


static INPUT_PORTS_START( triviaes )
	PORT_INCLUDE( triviag1 )

	PORT_MODIFY("SWH")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )  PORT_DIPLOCATION("H1:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )

	PORT_MODIFY("SWG")
	PORT_DIPNAME( 0x0c, 0x04, "Guesses" )           PORT_DIPLOCATION("G1:3,4")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x0c, "5" )
INPUT_PORTS_END


static INPUT_PORTS_START( gimeabrk )
	PORT_INCLUDE( stocker )

	PORT_MODIFY("SWH")
	BALSENTE_COINAGE_ALT

	PORT_MODIFY("SWG")
	PORT_DIPNAME( 0x03, 0x01, "Bonus Shot" )            PORT_DIPLOCATION("G1:1,2")
	PORT_DIPSETTING(    0x00, "Every 6 Balls" )
	PORT_DIPSETTING(    0x01, "Every 8 Balls" )
	PORT_DIPSETTING(    0x02, "Every 10 Balls" )
	PORT_DIPSETTING(    0x03, "Every 12 Balls" )
	PORT_DIPNAME( 0x0c, 0x08, "Initial Shots" )         PORT_DIPLOCATION("G1:3,4")
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x08, "12" )
	PORT_DIPSETTING(    0x0c, "14" )
	PORT_DIPNAME( 0x10, 0x00, "Players Per Credit" )    PORT_DIPLOCATION("G1:5")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "1 Or 2" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("G1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, "High Scores" )           PORT_DIPLOCATION("G1:7")
	PORT_DIPSETTING(    0x40, "Keep Top 5" )
	PORT_DIPSETTING(    0x00, "Keep All" )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )    PORT_COCKTAIL

	PORT_MODIFY("IN1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )

	/* Player 1 Trackball, Cocktail acts as Player 2*/
	PORT_MODIFY("AN0")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_RESET PORT_PLAYER(1)
	PORT_MODIFY("AN1")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_RESET PORT_REVERSE PORT_PLAYER(1)

	/* Player 2 Trackball, Cocktail acts as Player 1 */
	PORT_MODIFY("AN2")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_Y ) PORT_COCKTAIL PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_RESET PORT_PLAYER(2)
	PORT_MODIFY("AN3")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_X ) PORT_COCKTAIL PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_RESET PORT_REVERSE PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( minigolf )
	PORT_INCLUDE( stocker )

	PORT_MODIFY("SWH")
	BALSENTE_COINAGE_ALT2

	PORT_MODIFY("SWG")
	PORT_DIPNAME( 0x01, 0x01, "Add-A-Coin" )            PORT_DIPLOCATION("G1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Display Kids" )          PORT_DIPLOCATION("G1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "Kid On Left Located" )   PORT_DIPLOCATION("G1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "Kid On Right Located" )  PORT_DIPLOCATION("G1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "G1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "G1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "G1:7" )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	/* Player 1 Trackball */
	PORT_MODIFY("AN2")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_RESET PORT_PLAYER(1)

	PORT_MODIFY("AN3")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_RESET PORT_REVERSE PORT_PLAYER(1)
INPUT_PORTS_END


static INPUT_PORTS_START( minigolf2 )
	PORT_INCLUDE( minigolf )

	PORT_MODIFY("SWH")
	BALSENTE_COINAGE_ALT

	PORT_MODIFY("SWG")
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "G1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "G1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "G1:4" )
INPUT_PORTS_END


static INPUT_PORTS_START( toggle )
	PORT_INCLUDE( sentetst )

	PORT_MODIFY("SWG")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "G1:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "G1:2" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) )    PORT_DIPLOCATION("G1:3")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "G1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "G1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "G1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "G1:7" )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)

	PORT_MODIFY("AN2")
	UNUSED_ANALOG

	PORT_MODIFY("AN3")
	UNUSED_ANALOG
INPUT_PORTS_END


static INPUT_PORTS_START( nametune )
	PORT_INCLUDE( stocker )

	PORT_MODIFY("SWH")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )  PORT_DIPLOCATION("H1:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )

	PORT_MODIFY("SWG")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "G1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "G1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "G1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "G1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "G1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "G1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "G1:7" )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 Blue Button") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Green Button") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Yellow Button") PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Red Button") PORT_PLAYER(1)
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Red Button") PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Yellow Button") PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P2 Green Button") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P2 Blue Button") PORT_PLAYER(2)

	PORT_MODIFY("AN3")
	UNUSED_ANALOG
INPUT_PORTS_END


static INPUT_PORTS_START( nstocker )
	PORT_INCLUDE( stocker )

	PORT_MODIFY("SWH")
	BALSENTE_COINAGE_ALT2

	PORT_MODIFY("SWG")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("G1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "G1:7" )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(balsente_state::nstocker_bits_r))

	/* cheese alert -- we have to map this to player 2 so that it doesn't interfere with
	   the crosshair controls */
	PORT_MODIFY("AN3")
	PORT_BIT( 0xff, 0, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20)
					PORT_CODE_DEC(KEYCODE_S) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(KEYCODE_F) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH) PORT_RESET PORT_PLAYER(2)

	/* extra ports for shooters */
	PORT_START("FAKEX") /* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_START("FAKEY") /* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10)
INPUT_PORTS_END


static INPUT_PORTS_START( sfootbal )
	PORT_INCLUDE( stocker )

	PORT_MODIFY("SWH")
	BALSENTE_COINAGE_ALT2

	PORT_MODIFY("SWG")
	PORT_DIPNAME( 0x07, 0x03, DEF_STR( Game_Time ) )    PORT_DIPLOCATION("G1:1,2,3")
	PORT_DIPSETTING(    0x00, "1:30" )
	PORT_DIPSETTING(    0x01, "1:40" )
	PORT_DIPSETTING(    0x02, "1:50" )
	PORT_DIPSETTING(    0x03, "2:00" )
	PORT_DIPSETTING(    0x04, "2:20" )
	PORT_DIPSETTING(    0x05, "2:40" )
	PORT_DIPSETTING(    0x06, "3:00" )
	PORT_DIPSETTING(    0x07, "3:30" )
	PORT_DIPNAME( 0x08, 0x00, "Players Per Credit" )    PORT_DIPLOCATION("G1:4")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "1 Or 2" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "G1:7" )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x38, IP_ACTIVE_LOW, IPT_UNUSED )

	/* Player 2 Analog Joystick */
	PORT_MODIFY("AN0")
	PORT_BIT( 0xff, 0, IPT_AD_STICK_Y ) PORT_MINMAX(0x80,0x7f) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(2)

	PORT_MODIFY("AN1")
	PORT_BIT( 0xff, 0, IPT_AD_STICK_X ) PORT_MINMAX(0x80,0x7f) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_REVERSE PORT_PLAYER(2)

	/* Player 1 Analog Joystick */
	PORT_MODIFY("AN2")
	PORT_BIT( 0xff, 0, IPT_AD_STICK_Y ) PORT_MINMAX(0x80,0x7f) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(1)

	PORT_MODIFY("AN3")
	PORT_BIT( 0xff, 0, IPT_AD_STICK_X ) PORT_MINMAX(0x80,0x7f) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_REVERSE PORT_PLAYER(1)
INPUT_PORTS_END


static INPUT_PORTS_START( spiker )
	PORT_INCLUDE( stocker )

	PORT_MODIFY("SWH")
	BALSENTE_COINAGE_ALT2

	PORT_MODIFY("SWG")
	PORT_DIPNAME( 0x01, 0x00, "Game Duration" )         PORT_DIPLOCATION("G1:1")
	PORT_DIPSETTING(    0x00, "11 Points" )
	PORT_DIPSETTING(    0x01, "15 Points" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "G1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "G1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "G1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "G1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "G1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "G1:7" )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)

	/* Player 2 Trackball */
	PORT_MODIFY("AN0")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_RESET PORT_PLAYER(2)

	PORT_MODIFY("AN1")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_RESET PORT_REVERSE PORT_PLAYER(2)

	/* Player 1 Trackball */
	PORT_MODIFY("AN2")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_RESET PORT_PLAYER(1)

	PORT_MODIFY("AN3")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_RESET PORT_REVERSE PORT_PLAYER(1)
INPUT_PORTS_END


static INPUT_PORTS_START( stompin )
	PORT_INCLUDE( stocker )
	/* Set H:1-8 all to OFF to enable Free Play */
	PORT_MODIFY("SWH")
	BALSENTE_COINAGE_ALT2

	PORT_MODIFY("SWG")
	PORT_DIPNAME( 0x01, 0x00, "Display Kids" )          PORT_DIPLOCATION("G1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Kid On Right Located" )  PORT_DIPLOCATION("G1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "Kid On Left Located" )   PORT_DIPLOCATION("G1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("G1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Bee In Game" )           PORT_DIPLOCATION("G1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, "Bug Generation" )        PORT_DIPLOCATION("G1:8")
	PORT_DIPSETTING(    0x00, "Regular" )
	PORT_DIPSETTING(    0x80, DEF_STR( None ) )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x3c, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("AN0")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Top-Right") PORT_CODE(KEYCODE_9_PAD) PORT_PLAYER(1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Top") PORT_CODE(KEYCODE_8_PAD) PORT_PLAYER(1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Top-Left") PORT_CODE(KEYCODE_7_PAD) PORT_PLAYER(1)

	PORT_MODIFY("AN1")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Right") PORT_CODE(KEYCODE_6_PAD) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Left") PORT_CODE(KEYCODE_4_PAD) PORT_PLAYER(1)

	PORT_MODIFY("AN2")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Bot-Right") PORT_CODE(KEYCODE_3_PAD) PORT_PLAYER(1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Bottom") PORT_CODE(KEYCODE_2_PAD) PORT_PLAYER(1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Bot-Left") PORT_CODE(KEYCODE_1_PAD) PORT_PLAYER(1)

	PORT_MODIFY("AN3")
	UNUSED_ANALOG
INPUT_PORTS_END

static INPUT_PORTS_START( stompina )
	PORT_INCLUDE( stompin )

	PORT_MODIFY("SWG")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "G1:1" ) // not listed in test mode
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "G1:2" ) // not listed in test mode
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "G1:3" ) // not listed in test mode
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "G1:4" ) // not listed in test mode
	PORT_DIPNAME(           0x10, 0x00, "Invulnerability (Cheat)" )  PORT_DIPLOCATION("G1:5") // not listed in test mode, but..
	PORT_DIPSETTING(              0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(              0x10, DEF_STR( On ) )
	PORT_DIPNAME(           0x20, 0x20, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("G1:6")
	PORT_DIPSETTING(              0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(              0x20, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "G1:7" )
	PORT_DIPNAME(           0x80, 0x00, "Bug Generation" )        PORT_DIPLOCATION("G1:8")
	PORT_DIPSETTING(              0x00, "Regular" )
	PORT_DIPSETTING(              0x80, DEF_STR( None ) )
INPUT_PORTS_END

static INPUT_PORTS_START( grudge )
	PORT_INCLUDE( sentetst )

	PORT_MODIFY("SWH")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Unknown ) )      PORT_DIPLOCATION("H1:1,2")
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("H1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, "On (buggy)" )

	PORT_MODIFY("SWG")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "G1:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "G1:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "G1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "G1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "G1:7" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Players ) )      PORT_DIPLOCATION("G1:8")
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0x00, "3" )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("AN0")
	PORT_BIT( 0xff, 0, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(20) PORT_PLAYER(1)

	PORT_MODIFY("AN1")
	PORT_BIT( 0xff, 0, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(20) PORT_PLAYER(2)

	PORT_MODIFY("AN2")
	PORT_BIT( 0xff, 0, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(20) PORT_PLAYER(3)

	PORT_MODIFY("AN3")
	UNUSED_ANALOG
INPUT_PORTS_END

static INPUT_PORTS_START( grudgep )
	PORT_INCLUDE( grudge )

	PORT_MODIFY("SWH")
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("H1:8") // default to "ON" because Coin mode is buggy on this revision of the prototype
	PORT_DIPSETTING(    0x00, "Off (buggy)" )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( rescraid )
	PORT_INCLUDE( stocker )

	PORT_MODIFY("SWH")
	BALSENTE_COINAGE_ALT2

	PORT_MODIFY("SWG")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )        PORT_DIPLOCATION("G1:1")
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPNAME( 0x0c, 0x04, "Minimum Game Time" )     PORT_DIPLOCATION("G1:3,4")
	PORT_DIPSETTING(    0x08, "45" )
	PORT_DIPSETTING(    0x04, "60" )
	PORT_DIPSETTING(    0x00, "90" )
	PORT_DIPSETTING(    0x0c, "120" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "G1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "G1:6" )
	PORT_DIPNAME( 0x40, 0x40, "Keep High Scores" )      PORT_DIPLOCATION("G1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_PLAYER(1)

	PORT_MODIFY("AN3")
	UNUSED_ANALOG
INPUT_PORTS_END


static INPUT_PORTS_START( shrike )
	PORT_INCLUDE( sentetst )

	PORT_MODIFY("SWH")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ))   PORT_DIPLOCATION("H1:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x03, DEF_STR( Free_Play ))
	PORT_DIPNAME( 0x40, 0x40, "Reset High Scores" ) PORT_DIPLOCATION("H1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ))
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ))
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "H1:8" )

	PORT_MODIFY("SWG")
	PORT_DIPNAME( 0x03, 0x03, "Minimum Game Time" ) PORT_DIPLOCATION("G1:1,2")
	PORT_DIPSETTING(    0x00, "1:00" )
	PORT_DIPSETTING(    0x01, "1:30" )
	PORT_DIPSETTING(    0x02, "2:00" )
	PORT_DIPSETTING(    0x03, "2:30" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "G1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "G1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x20, "G1:7" )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x1c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) // carpet switch

	PORT_MODIFY("AN0")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(1)

	PORT_MODIFY("AN1")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(1)

	PORT_MODIFY("AN2")
	UNUSED_ANALOG

	PORT_MODIFY("AN3")
	UNUSED_ANALOG
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void balsente_state::balsente(machine_config &config)
{
	/* basic machine hardware */
	MC6809E(config, m_maincpu, 20_MHz_XTAL / 16); /* xtal verified but not speed */
	m_maincpu->set_addrmap(AS_PROGRAM, &balsente_state::cpu1_map);
	m_maincpu->set_vblank_int("screen", FUNC(balsente_state::update_analog_inputs));

	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set("audio6vb", FUNC(sente6vb_device::rec_w));
	m_acia->irq_handler().set_inputline(m_maincpu, M6809_FIRQ_LINE);

	X2212(config, "nov0").set_auto_save(true); // system NOVRAM
	X2212(config, "nov1").set_auto_save(true); // cart NOVRAM

	WATCHDOG_TIMER(config, "watchdog");

	TIMER(config, m_scanline_timer, 0).configure_generic(FUNC(balsente_state::interrupt_timer));

	LS259(config, m_outlatch); // U9H
	// these outputs are generally used to control the various lamps
	m_outlatch->q_out_cb<0>().set(FUNC(balsente_state::out0_w));
	m_outlatch->q_out_cb<1>().set(FUNC(balsente_state::out1_w));
	m_outlatch->q_out_cb<2>().set(FUNC(balsente_state::out2_w));
	m_outlatch->q_out_cb<3>().set(FUNC(balsente_state::out3_w));
	m_outlatch->q_out_cb<4>().set(FUNC(balsente_state::out4_w));
	m_outlatch->q_out_cb<5>().set(FUNC(balsente_state::out5_w));
	m_outlatch->q_out_cb<6>().set(FUNC(balsente_state::out6_w));
	// special case is output 7, which recalls the NVRAM data
	m_outlatch->q_out_cb<7>().set(FUNC(balsente_state::nvrecall_w));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(BALSENTE_PIXEL_CLOCK, BALSENTE_HTOTAL, BALSENTE_HBEND, BALSENTE_HBSTART, BALSENTE_VTOTAL, BALSENTE_VBEND, BALSENTE_VBSTART);
	m_screen->set_screen_update(FUNC(balsente_state::screen_update_balsente));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(4, raw_to_rgb_converter::standard_rgb_decoder<4,4,4, 24,16,8>, 1024);


	/* sound hardware */
	sente6vb_device &audio6vb(SENTE6VB(config, "audio6vb"));
	audio6vb.send_cb().set(m_acia, FUNC(acia6850_device::write_rxd));
	audio6vb.clock_out_cb().set(m_acia, FUNC(acia6850_device::write_txc));
	audio6vb.clock_out_cb().append(m_acia, FUNC(acia6850_device::write_rxc));
}


void balsente_state::teamht(machine_config &config)
{
	balsente(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &balsente_state::cpu1_teamht_map);
}


void balsente_state::grudge(machine_config &config)
{
	balsente(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &balsente_state::cpu1_grudge_map);
}


void balsente_state::st1002(machine_config &config)
{
	balsente(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &balsente_state::cpu1_st1002_map);
}


void balsente_state::spiker(machine_config &config)
{
	balsente(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &balsente_state::cpu1_spiker_map);
}


void balsente_state::shrike(machine_config &config)
{
	balsente(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &balsente_state::cpu1_shrike_map);

	M68000(config, m_68k, 8000000);
	m_68k->set_addrmap(AS_PROGRAM, &balsente_state::shrike68k_map);

	config.set_maximum_quantum(attotime::from_hz(6000));
}


void balsente_state::rescraid(machine_config &config)
{
	balsente(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &balsente_state::cpu1_smudge_map);
}

/*  Trivial hardware from Maibesa */
void balsente_state::triviamb(machine_config &config)
{
	balsente(config);

	config.device_remove("outlatch");
	config.device_remove("acia");
	config.device_remove("audio6vb");
	config.device_remove("nov0");
	config.device_remove("nov1");

	m_maincpu->set_clock(10_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &balsente_state::cpu1_triviamb_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // HY6116AP-10 + battery (only 512x4 bits are actually saved)

	pia6821_device &pia(PIA6821(config, "pia")); // Thomson EF6821P
	pia.writepb_handler().set(FUNC(balsente_state::out0_w)).bit(0);
	pia.writepb_handler().append(FUNC(balsente_state::out1_w)).bit(1);
	pia.writepb_handler().append(FUNC(balsente_state::out2_w)).bit(2);
	pia.writepb_handler().append(FUNC(balsente_state::out3_w)).bit(3);
	pia.writepb_handler().append(FUNC(balsente_state::out4_w)).bit(4);
	pia.writepb_handler().append(FUNC(balsente_state::out5_w)).bit(5);
	pia.writepb_handler().append(FUNC(balsente_state::out6_w)).bit(6);

	mc6845_device &crtc(C6545_1(config, "crtc", 10_MHz_XTAL / 8)); // specific type unknown, but must allow VBLANK polling
	crtc.set_screen(nullptr);
	crtc.set_show_border_area(false);
	crtc.set_char_width(4);

	m_palette->set_format(palette_device::BGR_233, 1024);

	// sound PCB has: 2x Z80CTC, 2x AY8910A, 1x M5205, 1x 8MHz XTAL (divisor unknown for every device)
	Z80(config, m_audiocpu, 8_MHz_XTAL / 2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &balsente_state::cpu2_triviamb_map);
	m_audiocpu->set_addrmap(AS_IO, &balsente_state::cpu2_triviamb_io_map);

	Z80CTC(config, "ctc1", 8_MHz_XTAL / 2);
	Z80CTC(config, "ctc2", 8_MHz_XTAL / 2);

	SPEAKER(config, "mono").front_center();

	AY8910(config, "ay1", 8_MHz_XTAL / 6).add_route(ALL_OUTPUTS, "mono", 0.90);
	AY8910(config, "ay2", 8_MHz_XTAL / 6).add_route(ALL_OUTPUTS, "mono", 0.90);

	MSM5205(config, "msm", 384000).add_route(ALL_OUTPUTS, "mono", 0.90);
}

/*************************************
 *
 *  ROM definitions
 *
 *************************************/

#define MOTHERBOARD_PALS \
	ROM_REGION( 0x00104, "motherbrd_pals", 0) /* Motherboard PAL's */ \
	ROM_LOAD( "u01508001100b.u20f", 0x00000, 0x00104, CRC(2d2e2102) SHA1(de094f9955d6085f1714f1aa7c71e1f047e96c5f) ) /* PAL16L8, dumped from Board 007-8001-01-0C Rev C1 */

ROM_START( sentetst )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "sdiagef.bin",  0x1e000, 0x2000, CRC(2a39fc53) SHA1(04ea68bfad455cc928e57390eba5597c38bbab69) )

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "sdiaggr0.bin", 0x00000, 0x2000, CRC(5e0ff62a) SHA1(3f0ebebb2f58530af7fac57a4780dfb37ef1ee1d) )

	MOTHERBOARD_PALS
ROM_END


/*

Chicken Shift (11/23/84)

Cartridge Type:
  006-8003-01-0B REV B
Label:
+-----------------+
|    CHICKEN      |
|     SHIFT       |
|      EF         |
|   11/23/84      |
+-----------------+

*/
ROM_START( cshift ) /* Cart: 006-8003-01-0B REV B */
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "chicken_shift_ab_0_11-23-84.u9a", 0x00000, 0x2000, CRC(d2069e75) SHA1(17d5719e6e1976cebb332932cf3e900a88136928) )
	ROM_LOAD( "chicken_shift_ab_1_11-23-84.u8a", 0x02000, 0x2000, CRC(198f25a8) SHA1(5ca25fe57e94d8362896c903196e0080efd35ef5) )
	ROM_LOAD( "chicken_shift_ab_2_11-23-84.u7a", 0x04000, 0x2000, CRC(2e2b2b82) SHA1(a540f3ff2a0a10b19aafe1528b7dcaeae9b7393d) )
	ROM_LOAD( "chicken_shift_ab_3_11-23-84.u6a", 0x06000, 0x2000, CRC(b97fc520) SHA1(f45c5ec93eab1bfd1f9533df7ac624c2e99f6573) )
	ROM_LOAD( "chicken_shift_ab_4_11-23-84.u5a", 0x08000, 0x2000, CRC(b4f0d673) SHA1(cb97dc8836c497fa03a862227340f8c351986a39) )
	ROM_LOAD( "chicken_shift_ab_5_11-23-84.u4a", 0x0a000, 0x2000, CRC(b1f8e589) SHA1(d837beff063ed987571c5af6130f2c7d637d7c39) )
	ROM_LOAD( "chicken_shift_cd_11-23-84.u3a",   0x1c000, 0x2000, CRC(f555a0b2) SHA1(49668f8363fdcec4686ec80bf2e99003cd11e2c1) )
	ROM_LOAD( "chicken_shift_ef_11-23-84.u2a",   0x1e000, 0x2000, CRC(368b1ce3) SHA1(8003ef99adcb26feb42e1b0945b1185e438582b2) )

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "chicken_shift_gr-0_11-23-84.u9b", 0x00000, 0x2000, CRC(67f9d3b3) SHA1(4f3f80e4272b20611206636b6ccb627087efd0c3) )
	ROM_LOAD( "chicken_shift_gr-1_11-23-84.u8b", 0x02000, 0x2000, CRC(78973d50) SHA1(de7891ef47c277d733d9b4810d68621718644655) )
	ROM_LOAD( "chicken_shift_gr-2_11-23-84.u7b", 0x04000, 0x2000, CRC(1784f939) SHA1(ff7f43451580e3b314c24b00a66765c0b395ddf6) )
	ROM_LOAD( "chicken_shift_gr-3_11-23-84.u6b", 0x06000, 0x2000, CRC(b43916a2) SHA1(8d42fb6ae7cf8b2d94eb0c14e00bb115f8ef01b4) )
	ROM_LOAD( "chicken_shift_gr-4_11-23-84.u5b", 0x08000, 0x2000, CRC(a94cd35b) SHA1(0ca0497a1b055ff1ae6b7bc36ae45749dff50caa) )

	MOTHERBOARD_PALS
ROM_END


ROM_START( gghost ) /* Cart: 006-8003-01-0B REV B */
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "ggh-ab0.u9a", 0x00000, 0x2000, CRC(ed0fdeac) SHA1(294cee47c0541c58d4d766388c281ed30b8f5426) )
	ROM_LOAD( "ggh-ab1.u8a", 0x02000, 0x2000, CRC(5bfbae58) SHA1(65c795354223cd5e2474ad9e779b77f58ed5b896) )
	ROM_LOAD( "ggh-ab2.u7a", 0x04000, 0x2000, CRC(f0baf921) SHA1(4b7ee06838dcdb68ddec51f5eafab53ff3f25bfe) )
	ROM_LOAD( "ggh-ab3.u6a", 0x06000, 0x2000, CRC(ed0fdeac) SHA1(294cee47c0541c58d4d766388c281ed30b8f5426) )
	ROM_LOAD( "ggh-ab4.u5a", 0x08000, 0x2000, CRC(5bfbae58) SHA1(65c795354223cd5e2474ad9e779b77f58ed5b896) )
	ROM_LOAD( "ggh-ab5.u4a", 0x0a000, 0x2000, CRC(f0baf921) SHA1(4b7ee06838dcdb68ddec51f5eafab53ff3f25bfe) )
	ROM_LOAD( "ggh-cd.u3a",  0x1c000, 0x2000, CRC(d3d75f84) SHA1(f19f99ea05ad5b7e4b0485e80d7b6a329b8ef4d8) )
	ROM_LOAD( "ggh-ef.u2a",  0x1e000, 0x2000, CRC(a02b4243) SHA1(f242fc017c9ae1997409825c34e8f5c6e6a0615e) )

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "ggh-gr0.u8b", 0x00000, 0x2000, CRC(03515526) SHA1(bceb7c8c3aa4c39b6cf1b976c5765c920399fe31) )
	ROM_LOAD( "ggh-gr1.u8b", 0x02000, 0x2000, CRC(b4293435) SHA1(5e2b96c19c4f5c63a5afa2de504d29fe64a4c908) )
	ROM_LOAD( "ggh-gr2.u7b", 0x04000, 0x2000, CRC(ece0cb97) SHA1(13bfb38de30992b9597c9d0f87f7b2a5c061ba51) )
	ROM_LOAD( "ggh-gr3.u6b", 0x06000, 0x2000, CRC(dd7e25d0) SHA1(cc6402835d1b46d160869ba1d1cad54f24d3fe86) )
	ROM_LOAD( "ggh-gr4.u5b", 0x08000, 0x2000, CRC(b4293435) SHA1(5e2b96c19c4f5c63a5afa2de504d29fe64a4c908) )
	ROM_LOAD( "ggh-gr5.u4b", 0x0a000, 0x2000, CRC(d3da0093) SHA1(7474901b089ea62abad0a2f657fd8c4a1be09bf0) )

	MOTHERBOARD_PALS
ROM_END


/*

Hat Trick (11/12/84)

Cartridge Type:
  006-8003-01-0D REV D  (also commonly found on REV B PCBs)
Label:
+-----------------+      +-----------------+
| HAT TRK         |      |   HAT           |
| CD              |  or  |  TRICK          |
| 11/12/84        |      |    CD           |
+-----------------+      | 11/12/84        |
                         +-----------------+
Cartridge Type:
  007-8003-01 REV A  (Yes, it's actually 007 and NOT 006)
Label:
+-----------------+
| H.T.            |
| CD              |
| 11/12/84        |
+-----------------+

*/
ROM_START( hattrick ) /* Cart: 006-8003-01-0D REV D */
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "hat_trk_ab0_11-12-84.u9a", 0x00000, 0x2000, CRC(f25c1b99) SHA1(43b2334be7cfb8091eea963e10547295362372d3) )
	ROM_LOAD( "hat_trk_ab1_11-12-84.u8a", 0x02000, 0x2000, CRC(c1df3d1f) SHA1(754f537d12efe8891638fd11a2ee8a5b234fb079) )
	ROM_LOAD( "hat_trk_ab2_11-12-84.u7a", 0x04000, 0x2000, CRC(f6c41257) SHA1(05f5e71d08241c559da3bfc286c76cbb22710586) )
	ROM_LOAD( "hat_trk_cd_11-12-84.u3a",  0x1c000, 0x2000, CRC(fc44f36c) SHA1(227d0c93c579d743b615b1fa6da56128e8202e51) )
	ROM_LOAD( "hat_trk_ef_11-12-84.u2a",  0x1e000, 0x2000, CRC(d8f910fb) SHA1(b74a305dd848c7bf574e4b0aa32147b8d5c89e9e) )

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "hat_trk_gr0_11-12-84.u9b", 0x00000, 0x2000, CRC(9f41baba) SHA1(fa817a8e4d2f7b86a2294132e3991f7b6d8cb11a) )
	ROM_LOAD( "hat_trk_gr1_11-12-84.u8b", 0x02000, 0x2000, CRC(951f08c9) SHA1(059a575dd35cd8e822e12ac2606b47b6272bbb41) )

	MOTHERBOARD_PALS
ROM_END


/*

Team Hat Trick (11/16/84)

Cartridge Type:
  006-8025-01-0B REV B
Label:
+-----------------+
| HAT TRK         |
| CD              |
| 11/16/84        |
+-----------------+

NOTE: ROMs CD, AB-1 & GR-0 were dated 12/16/84, while AB-0 was dated 11/12/84
      Cartridge also contains an unlabeled PAL at U1C

*/
ROM_START( teamht ) /* Cart: 006-8025-01-0B REV B */
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "hat_trk_ab-0_11-12-84.u8a", 0x00000, 0x4000, CRC(cb746de8) SHA1(b0e5003370b65f2aed4dc9ccb2a2d3eb29050245) ) /* ONLY this ROM was dated 11/12/84 */
	ROM_LOAD( "hat_trk_ab-1_11-16-84.u7a", 0x04000, 0x4000, CRC(5f2a0b24) SHA1(da1950a7e11014e47438a7c5831433390c1b1fd3) )
	ROM_LOAD( "hat_trk_cd_11-16-84.u1a",   0x1c000, 0x4000, CRC(6c6cf2be) SHA1(80e82ae4bd129000e74c4a5fd06d2109d5417e39) )

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "hat_trk_gr-0_11-16-84.u6b", 0x00000, 0x4000, CRC(6e299728) SHA1(f10fc020fdf8f61d059ac57306b0353ac7dbfb24) )

	ROM_REGION( 0x00001, "cart_pals", 0) /* PAL's located on the cartridge */
	ROM_LOAD( "pal10l8.u1c", 0x0000, 0x0001, NO_DUMP ) /* PAL10L8CN */

	MOTHERBOARD_PALS
ROM_END


/*

Off The Wall (10/16/84)

Cartridge Type:
  006-8003-01-0D REV D
Label:
+-----------------+
|    OFF THE      |
|     WALL        |
|      EF         |
|   10/16/84      |
+-----------------+

*/
ROM_START( otwalls ) /* Cart: 006-8003-01-0D REV D */
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "off_the_wall_ab0_10-16-84.u9a", 0x00000, 0x2000, CRC(474441c7) SHA1(16fb5be9f94e072d4f3003abcc9dcf6d7af2359a) )
	ROM_LOAD( "off_the_wall_ab1_10-16-84.u8a", 0x02000, 0x2000, CRC(2e9e9411) SHA1(7dfd8dafa34e4d22fa0c5e472e3e98a1c0969f43) )
	ROM_LOAD( "off_the_wall_ab2_10-16-84.u7a", 0x04000, 0x2000, CRC(ba092128) SHA1(a38305c3ea9c8bf3596c18829655049f9468166e) )
	ROM_LOAD( "off_the_wall_ab3_10-16-84.u6a", 0x06000, 0x2000, CRC(74bc479d) SHA1(905dab90aa11f3f4359185bb67d8c2bdc957516d) )
	ROM_LOAD( "off_the_wall_ab4_10-16-84.u5a", 0x08000, 0x2000, CRC(f5f67619) SHA1(e3eb1434dff987d27056ae0749046f32f280160b) )
	ROM_LOAD( "off_the_wall_ab5_10-16-84.u4a", 0x0a000, 0x2000, CRC(f5f67619) SHA1(e3eb1434dff987d27056ae0749046f32f280160b) )
	ROM_LOAD( "off_the_wall_cd_10-16-84.u3a",  0x1c000, 0x2000, CRC(8e2d15ab) SHA1(8043fdf637de7752e8d42554ebad2e155a6f5939) )
	ROM_LOAD( "off_the_wall_ef_10-16-84.u2a",  0x1e000, 0x2000, CRC(57eab299) SHA1(475d800c03d6b2786bd23861d61dc113b837a585) )

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "off_the_wall_gr0_10-16-84.u9b", 0x00000, 0x2000, CRC(210bad3c) SHA1(703769c6a569b17f2ad18441da7de0237be4721e) )
	ROM_LOAD( "off_the_wall_gr1_10-16-84.u8b", 0x02000, 0x2000, CRC(13e6aaa5) SHA1(ac8b9d16d2159d4a578d8fa988b59c058c5efc88) )
	ROM_LOAD( "off_the_wall_gr2_10-16-84.u7b", 0x04000, 0x2000, CRC(5cfefee5) SHA1(9aa74f0e1116098f43a4f8b4957db8923ddaf780) )
	ROM_LOAD( "off_the_wall_gr3_10-16-84.u6b", 0x06000, 0x2000, CRC(6b17e4a9) SHA1(f9c57da863d613a456ee056569a87a9552ad3874) )
	ROM_LOAD( "off_the_wall_gr4_10-16-84.u5b", 0x08000, 0x2000, CRC(15985c8c) SHA1(94f21c348bfbe4db6d0cfa5b5e35d2df4b8f936d) )
	ROM_LOAD( "off_the_wall_gr5_10-16-84.u4b", 0x0a000, 0x2000, CRC(448f7e3c) SHA1(505724e90f17b05ccf0137dbed0d33e39db1d5ab) )

	MOTHERBOARD_PALS
ROM_END


ROM_START( snakepit ) /* Cart: 006-8003-01-0D REV D */
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "spit-ab0.u9a", 0x00000, 0x2000, CRC(5aa86081) SHA1(e65e256661b13a0631398e115dd02fce281bafa4) )
	ROM_LOAD( "spit-ab1.u8a", 0x02000, 0x2000, CRC(588228b8) SHA1(b64032a4fd1f52179d38e2073380bba6ec321302) )
	ROM_LOAD( "spit-ab2.u7a", 0x04000, 0x2000, CRC(60173ab6) SHA1(45b27492023771a53ea5857592a2a113746a72b6) )
	ROM_LOAD( "spit-ab3.u6a", 0x06000, 0x2000, CRC(56cb51a8) SHA1(fceb2fbae91bbab0b25410072805449ef531f360) )
	ROM_LOAD( "spit-ab4.u5a", 0x08000, 0x2000, CRC(40ba61e0) SHA1(91b06d116633c5261f3aa97d4e65bd61bae3c0eb) )
	ROM_LOAD( "spit-ab5.u4a", 0x0a000, 0x2000, CRC(2a1d9d8f) SHA1(3364f4bc507576323560bf14fc99036c47d0297c) )
	ROM_LOAD( "spit-cd.u3a",  0x1c000, 0x2000, CRC(54095cbb) SHA1(a43b78b2876359a29ecb2f169c876a0026375ea2) )
	ROM_LOAD( "spit-ef.u2a",  0x1e000, 0x2000, CRC(5f836a66) SHA1(cc3c11003f9e49cac10c0296ab6d156e5677d0f8) )

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "spit-gr0.u9b", 0x00000, 0x2000, CRC(f77fd85d) SHA1(f8e69d1d0030412d6129a8ebfee40b3f1f189d8d) )
	ROM_LOAD( "spit-gr1.u8b", 0x02000, 0x2000, CRC(3ad10334) SHA1(1d82a7948fbee627c80a9e03ade90e57972a6a31) )
	ROM_LOAD( "spit-gr2.u7b", 0x04000, 0x2000, CRC(24887703) SHA1(089f077400c9a3e3f5b43e8aa60b41160e296d52) )
	ROM_LOAD( "spit-gr3.u6b", 0x06000, 0x2000, CRC(c6703ec2) SHA1(0f5d7c17ee508f8fea316b7f92cdd7cc174b155f) )
	ROM_LOAD( "spit-gr4.u5b", 0x08000, 0x2000, CRC(b4293435) SHA1(5e2b96c19c4f5c63a5afa2de504d29fe64a4c908) )
	ROM_LOAD( "spit-gr5.u4b", 0x0a000, 0x2000, CRC(dc27c970) SHA1(291ef10a8c330ef8e47622246b6301d2e5171df7) )

	MOTHERBOARD_PALS
ROM_END

ROM_START( snakepita ) /* Cart: 006-8003-01-0D REV D */
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "ab0.bin", 0x00000, 0x2000, CRC(5aa86081) SHA1(e65e256661b13a0631398e115dd02fce281bafa4) )
	ROM_LOAD( "ab1.bin", 0x02000, 0x2000, CRC(588228b8) SHA1(b64032a4fd1f52179d38e2073380bba6ec321302) )
	ROM_LOAD( "ab2.bin", 0x04000, 0x2000, CRC(d573e33e) SHA1(47ff4f2f28f3d1830da82bef09ebc9b4f5cedaa5) )
	ROM_LOAD( "ab3.bin", 0x06000, 0x2000, CRC(0e3b6cfe) SHA1(2bac5869d1ed01744a7fc07679fc59307e845f97) )
	ROM_LOAD( "ab4.bin", 0x08000, 0x2000, CRC(5de588a7) SHA1(aebd6710bee56fdfa15916404f120469ab710de0) )
	ROM_LOAD( "ab5.bin", 0x0a000, 0x2000, CRC(2a1d9d8f) SHA1(3364f4bc507576323560bf14fc99036c47d0297c) )
	ROM_LOAD( "cd.bin",  0x1c000, 0x2000, CRC(f357172e) SHA1(822012360526459e85196692e4cb408fe25fb1cc) )
	ROM_LOAD( "ef.bin",  0x1e000, 0x2000, CRC(5e9d1de2) SHA1(5a04c4444aed9c2677fc85ad733fec69398403d6) )

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "gr0.bin", 0x00000, 0x2000, CRC(f77fd85d) SHA1(f8e69d1d0030412d6129a8ebfee40b3f1f189d8d) )
	ROM_LOAD( "gr1.bin", 0x02000, 0x2000, CRC(3ad10334) SHA1(1d82a7948fbee627c80a9e03ade90e57972a6a31) )
	ROM_LOAD( "gr2.bin", 0x04000, 0x2000, CRC(24887703) SHA1(089f077400c9a3e3f5b43e8aa60b41160e296d52) )
	ROM_LOAD( "gr3.bin", 0x06000, 0x2000, CRC(c6703ec2) SHA1(0f5d7c17ee508f8fea316b7f92cdd7cc174b155f) )

	MOTHERBOARD_PALS
ROM_END


ROM_START( snakjack )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "rom-ab0.u9a", 0x00000, 0x2000, CRC(da2dd119) SHA1(85ae452b137e69e051fa66648f295d180339794e) )
	ROM_LOAD( "rom-ab1.u8a", 0x02000, 0x2000, CRC(657ddf26) SHA1(48591a6b0c30d576f0e08dd54c95cbda76b5dfbd) )
	ROM_LOAD( "rom-ab2.u7a", 0x04000, 0x2000, CRC(15333dcf) SHA1(13546bd058a10513fe4cbe3a3fa268b7c38b5993) )
	ROM_LOAD( "rom-ab3.u6a", 0x06000, 0x2000, CRC(57671f6f) SHA1(49e76e03d828fed28e7e0608985172af20084f7f) )
	ROM_LOAD( "rom-ab4.u5a", 0x08000, 0x2000, CRC(c16c5dc0) SHA1(93e36758f4e5bb8dac29f9a2bc3ac5f9589e8c9a) )
	ROM_LOAD( "rom-ab5.u4a", 0x0a000, 0x2000, CRC(d7019747) SHA1(c8b1a6ea463b5932bc9ed2c91faea2e2639d7934) )
	ROM_LOAD( "rom-cd.u3a",  0x1c000, 0x2000, CRC(7b44ca4c) SHA1(8697055da489fcf0244dc94fe5393418a8003bf7) )
	ROM_LOAD( "rom-ef.u1a",  0x1e000, 0x2000, CRC(f5309b38) SHA1(864f759dc6822b548742140b7ea2ea2aba43beba) )

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "rom-gr0.u9b", 0x00000, 0x2000, CRC(3e64b5d5) SHA1(ab681eabb4f8e5b946c288ffb8df0624c0473d82) )
	ROM_LOAD( "rom-gr1.u8b", 0x02000, 0x2000, CRC(b3b8baee) SHA1(b37638784a3903f2dcd698104da75b4ab59e8257) )
	ROM_LOAD( "rom-gr2.u7b", 0x04000, 0x2000, CRC(e9d89dac) SHA1(570809ec5f8a64f280e13cbf801664cb548997e9) )
	ROM_LOAD( "rom-gr3.u6b", 0x06000, 0x2000, CRC(b6602be8) SHA1(c5bc95e0116fb2cf86a694561dc2c21612ba4434) )
	ROM_LOAD( "rom-gr4.u5b", 0x08000, 0x2000, CRC(3fbfa686) SHA1(6c137d177c7aa2701497ac3ac922fdb8cd9f52b3) )
	ROM_LOAD( "rom-gr5.u4b", 0x0a000, 0x2000, CRC(345f94fb) SHA1(0af24f4e1a797efe5272f64b8a34483fe6002436) )

	/* From a picture in an eBay auction the board appears to have a PAL that needs to be dumped. */

	MOTHERBOARD_PALS
ROM_END


/*

Stocker

Cartridge Type:
  006-8025-01-0B REV B
Label:
+-----------------+
| STOCKER         |
| EF              |
| 3/19/85         |
+-----------------+

NOTE: Cartridge contains an unlabeled PAL at U1C

*/
ROM_START( stocker ) /* Cart: 006-8025-01-0B REV B */
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "stocker_ab_01_3-19-85.u8a", 0x00000, 0x4000, CRC(6a914d99) SHA1(0df23fcdcb3743d84ce0363424b7c5dd249c6dcf) )
	ROM_LOAD( "stocker_ab_23_3-19-85.u7a", 0x04000, 0x4000, CRC(48e432c2) SHA1(af87009089a3e83fab5c935696edbbf2a15215f9) )
	ROM_LOAD( "stocker_ef_3-19-85.u1a",    0x1c000, 0x4000, CRC(83e6e5c9) SHA1(f0e38a95cb2ea385a587f330c48fc787db0cc65e) )

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "stocker_gr_01_3-19-85.u6b", 0x00000, 0x4000, CRC(2e66ac35) SHA1(c65b4991a88f8359c85f904f66a7fe73330aface) )
	ROM_LOAD( "stocker_gr_23_3-19-85.u5b", 0x04000, 0x4000, CRC(6fa43631) SHA1(7000907b914bf851b09811e3736af8c02e1aeda9) )

	ROM_REGION( 0x00100, "cart_pals", 0) /* PAL's located on the cartridge */
	ROM_LOAD( "pal10l8.u1c", 0x0000, 0x002c, CRC(b4f6b0b7) SHA1(5a439bfb02b4b2cbcbd9b009ccfce1d300a2435e) ) /* PAL10L8CN */

	MOTHERBOARD_PALS
ROM_END

// This is Stocker on a different (likely a 006-8003-01-0D REV D) cartridge type, it needs to be redumped and re-added
#if 0
ROM_START( stockera )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "stkr-ab0.bin", 0x00000, 0x2000, CRC(784a00ad) SHA1(33e76be44207bc24dbb9c2f04204df22ba5154ff) )
	ROM_LOAD( "stkr-ab1.bin", 0x02000, 0x2000, CRC(cdae01dc) SHA1(7c2956acae639fd2f2cf061d1c32ae9edabe9270) )
	ROM_LOAD( "stkr-ab2.bin", 0x04000, 0x2000, CRC(18527d57) SHA1(cbb85f9e0b6169f4c2e03dc54b4937043535fc42) )
	ROM_LOAD( "stkr-ab3.bin", 0x06000, 0x2000, CRC(028f6c06) SHA1(f1d30efcd7e967b0390f441848bb655111fdde65) )
	ROM_LOAD( "stkr-cd.bin",  0x1c000, 0x2000, BAD_DUMP CRC(53dbc4e5) SHA1(e389978b5472174681fa180c6a2edf49903a6514) ) // 1 bad byte
	ROM_LOAD( "stkr-ef.bin",  0x1e000, 0x2000, BAD_DUMP CRC(cdcf46bc) SHA1(8b1e801dab1efed002d484135264998d255dc041) ) // 1 bad byte

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "stkr-gr0.bin", 0x00000, 0x2000, CRC(76d5694c) SHA1(e2b155fc7178886eb37a532d961b99b8c864397c) )
	ROM_LOAD( "stkr-gr1.bin", 0x02000, 0x2000, CRC(4a5cc00b) SHA1(9ce46ed94e715a5997998aee6377baf2869ab3a6) )
	ROM_LOAD( "stkr-gr2.bin", 0x04000, 0x2000, CRC(70002382) SHA1(c151ad3df2714a2f9f8b047894e7585ca16bd29e) )
	ROM_LOAD( "stkr-gr3.bin", 0x06000, 0x2000, CRC(68c862d8) SHA1(302ce10e23d17af9aa7fa13d18c602656a262eaa) ) // the data here also differs from the good set, although the change is meaningless (data at the end blanked out here)

	ROM_REGION( 0x00001, "cart_pals", 0) /* PAL's located on the cartridge */
	ROM_LOAD( "pal10l8.u1c", 0x0000, 0x0001, NO_DUMP ) /* PAL10L8CN */

	MOTHERBOARD_PALS
ROM_END
#endif

/*

Trivial Pursuit (Think Tank - Genus Edition) (2/12/85)

Cartridge Type:
  006-8003-01-0D REV D
Label:
+-----------------+
|  T.PRST         |
|  U2A            |
|  2/12/85        |
+-----------------+

*/
ROM_START( triviag1 ) /* Cart: 006-8003-01-0D REV D */
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "t.prst_u9a_2-12-85.u9a", 0x00000, 0x2000, CRC(79fd3ac3) SHA1(52db0ba445f9a953f6ceb43c3d173b73c71af192) )
	ROM_LOAD( "t.prst_u8a_2-12-85.u8a", 0x02000, 0x2000, CRC(0ff677e9) SHA1(14fdc1ee87893ea91eea40949aeac5381c569bdd) )
	ROM_LOAD( "t.prst_u7a_2-12-85.u7a", 0x04000, 0x2000, CRC(3b4d03e7) SHA1(b5bb541daf59b2a62b17a10afa37bfae50563393) )
	ROM_LOAD( "t.prst_u6a_2-12-85.u6a", 0x06000, 0x2000, CRC(2c6c0651) SHA1(9ff5dcc4a54df653ae43d503e153f4e48ea4735b) )
	ROM_LOAD( "t.prst_u5a_2-12-85.u5a", 0x08000, 0x2000, CRC(397529e7) SHA1(af1898dc35545981513ec251eed162b329709692) )
	ROM_LOAD( "t.prst_u4a_2-12-85.u4a", 0x0a000, 0x2000, CRC(499773a4) SHA1(c0c0ad2a63a9dbb7585cab7e21162bbc58fec0d8) )
	ROM_LOAD( "t.prst_u3a_2-12-85.u3a", 0x1c000, 0x2000, CRC(35c9b9c2) SHA1(aac57022098656dac99bf9ceeaa2bf9a3d139986) )
	ROM_LOAD( "t.prst_u2a_2-12-85.u2a", 0x1e000, 0x2000, CRC(64878342) SHA1(dd93d64b3fe351a9d2bd4c473ecefde58f0b0041) )

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "t.prst_u9b_2-12-85.u9b", 0x00000, 0x2000, CRC(20c9217a) SHA1(79ef058633149da8d2835405954ac31c661bf660) )
	ROM_LOAD( "t.prst_u8b_2-12-85.u8b", 0x02000, 0x2000, CRC(d7f44504) SHA1(804dbc4c006b20bdb01bdf02754e0d98f6fbacbe) )
	ROM_LOAD( "t.prst_u7b_2-12-85.u7b", 0x04000, 0x2000, CRC(4e59a15d) SHA1(c584bae32e2e5d8b5a48c44a31272b4f9dadfcd1) )
	ROM_LOAD( "t.prst_u6b_2-12-85.u6b", 0x06000, 0x2000, CRC(323a8640) SHA1(7ec6f8f9bcfa5de442dce4f6e81e697da34dbab8) )
	ROM_LOAD( "t.prst_u5b_2-12-85.u5b", 0x08000, 0x2000, CRC(673acf42) SHA1(7b36a86441732ba14576f9c1dd14fe0da575d4bf) )
	ROM_LOAD( "t.prst_u4b_2-12-85.u4b", 0x0a000, 0x2000, CRC(067bfd66) SHA1(32f5973f2f0aed67c8f9b5886f52b9dc516a611e) )

	MOTHERBOARD_PALS
ROM_END

/*

Trivial Pursuit (Think Tank - Genus Edition) (12/14/84)

Cartridge Type:
  006-8003-01-0D REV D
Label:
+-----------------+
|     TRIVIAL     |
|     PERSUIT     |
|       EF        |
|    12/14/84     |
+-----------------+

*/
ROM_START( triviag1a ) /* Cart: 006-8003-01-0D REV D */
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "trivial_pursuit_ab0_12-14-84.u9a", 0x00000, 0x2000, CRC(41ca9a81) SHA1(127beee924d4213de874f7def9875fd3a26c6b5f) )
	ROM_LOAD( "trivial_pursuit_ab1_12-14-84.u8a", 0x02000, 0x2000, CRC(b3b48a3d) SHA1(e9554887430014116ff8e5e3d0ef5678d13f224c) )
	ROM_LOAD( "trivial_pursuit_ab2_12-14-84.u7a", 0x04000, 0x2000, CRC(ab652ce9) SHA1(06f47c274b94f046a59a1dc432c55ee8f450a246) )
	ROM_LOAD( "trivial_pursuit_ab3_12-14-84.u6a", 0x06000, 0x2000, CRC(4b382c77) SHA1(4a14166c90542ecec3677d9098a26723be6a26e2) )
	ROM_LOAD( "trivial_pursuit_ab4_12-14-84.u5a", 0x08000, 0x2000, CRC(9b4a8c4e) SHA1(c0354862b428ad8a5b1d229cadfcfc7e688b06c1) )
	ROM_LOAD( "trivial_pursuit_ab5_12-14-84.u4a", 0x0a000, 0x2000, CRC(499773a4) SHA1(c0c0ad2a63a9dbb7585cab7e21162bbc58fec0d8) )
	ROM_LOAD( "trivial_pursuit_cd_12-14-84.u3a",  0x1c000, 0x2000, CRC(12d870ba) SHA1(b86a8cbf8037df78437056f5ff57e7b8b5e4c94e) )
	ROM_LOAD( "trivial_pursuit_ef_12-14-84.u2a",  0x1e000, 0x2000, CRC(d902ee28) SHA1(18e3c96e1ac50f847d1b9f4f868f19e074d147ff) )

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "trivial_pursuit_gr0_12-14-84.u9b", 0x00000, 0x2000, CRC(20c9217a) SHA1(79ef058633149da8d2835405954ac31c661bf660) )
	ROM_LOAD( "trivial_pursuit_gr1_12-14-84.u8b", 0x02000, 0x2000, CRC(d7f44504) SHA1(804dbc4c006b20bdb01bdf02754e0d98f6fbacbe) )
	ROM_LOAD( "trivial_pursuit_gr2_12-14-84.u7b", 0x04000, 0x2000, CRC(4e59a15d) SHA1(c584bae32e2e5d8b5a48c44a31272b4f9dadfcd1) )
	ROM_LOAD( "trivial_pursuit_gr3_12-14-84.u6b", 0x06000, 0x2000, CRC(323a8640) SHA1(7ec6f8f9bcfa5de442dce4f6e81e697da34dbab8) )
	ROM_LOAD( "trivial_pursuit_gr4_12-14-84.u5b", 0x08000, 0x2000, CRC(673acf42) SHA1(7b36a86441732ba14576f9c1dd14fe0da575d4bf) )
	ROM_LOAD( "trivial_pursuit_gr5_12-14-84.u4b", 0x0a000, 0x2000, CRC(d17d5431) SHA1(b92741f6eda01f2e360e73a9f4df728fc44d7e1b) )

	MOTHERBOARD_PALS
ROM_END


/*

Trivial Pursuit (Baby Boomer Edition) (3/20/85)

Cartridge Type:
  006-8025-01-0B REV B
Label:
+-----------------+      +-----------------+
| B.BOOMER        |      | T.P.BABYBOOM    |
| ROM CD6EFR      |  or  | CD6EFR          |
| 3/20/85         |      | 3/20/85         |
+-----------------+      +-----------------+

NOTE: Cartridge contains an unlabeled PAL at U1C

*/
ROM_START( triviabb ) /* Cart: 006-8025-01-0B REV B */
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "b.boomer_rom_ab01r_3-20-85.u8a",  0x00000, 0x4000, CRC(1b7c439d) SHA1(8b3020dcb375b2f2e5e975a8067df6504aa8691e) )
	ROM_LOAD( "b.boomer_rom_ab23r_3-20-85.u7a",  0x04000, 0x4000, CRC(e4f1e704) SHA1(e5135134b54e1e2e95c5bfe6e5f0e2dd280db69d) )
	ROM_LOAD( "b.boomer_rom_ab45r_3-20-85.u6a",  0x08000, 0x4000, CRC(daa2d8bc) SHA1(feae215877ba42ab33182dfd74083f1d48443d8c) )
	ROM_LOAD( "b.boomer_rom_ab67r_3-20-85.u5a",  0x0c000, 0x4000, CRC(3622c4f1) SHA1(d180bb1c4a73d95c369cc507697421fb38a92d2c) )
	ROM_LOAD( "b.boomer_rom_cd45r_3-20-85.u2a",  0x18000, 0x4000, CRC(07fd88ff) SHA1(c3168ecf6562e09790c4f18cdd91c7a347223323) )
	ROM_LOAD( "b.boomer_rom_cd6efr_3-20-85.u1a", 0x1c000, 0x4000, CRC(2d03f241) SHA1(986ca6ea20c306e83ae88acc2d6837c7ed5fe351) )

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "b.boomer_gr01r_3-20-85.u6b", 0x00000, 0x4000, CRC(6829de8e) SHA1(4ec494883ba358f2ac7ce8d5a623a2f34b5bc843) ) /* these 3 didn't have "ROM" on the label */
	ROM_LOAD( "b.boomer_gr23r_3-20-85.u5b", 0x04000, 0x4000, CRC(89398700) SHA1(771ee04baa9a31d435a6234490105878713e7845) )
	ROM_LOAD( "b.boomer_gr45r_3-20-85.u4b", 0x08000, 0x4000, CRC(92fb6fb1) SHA1(1a322bd3cfacdf82d4fcc4b4d47f78a701411919) )

	ROM_REGION( 0x00100, "cart_pals", 0) /* PAL's located on the cartridge */
	ROM_LOAD( "pal10l8.u1c", 0x0000, 0x002c, CRC(175a5168) SHA1(4f5e090a8ae1e35f9cb1b649ef1e1805f6f32284) ) /* PAL10L8CN */

	MOTHERBOARD_PALS
ROM_END


/*

Trivial Pursuit: Genus II (3/22/85)

Cartridge Type:
  006-8025-01-0B REV B
Label:
+-----------------+
| GENUS II        |
| ROM D6EFR       |
| 3/22/85         |
+-----------------+

NOTE: Cartridge contains an unlabeled PAL at U1C

*/
ROM_START( triviag2 ) /* Cart: 006-8025-01-0B REV B */
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "genus_ii_ab01_r_3-22-85.u8a",    0x00000, 0x4000, CRC(4fca20c5) SHA1(595b32ff035036cafbf49d75aa170f39e9f52b38) )
	ROM_LOAD( "genus_ii_ab23_r_3-22-85.u7a",    0x04000, 0x4000, CRC(6cf2ddeb) SHA1(0d6667babd9ab70820cf165900d90003f0893be7) )
	ROM_LOAD( "genus_ii_ab45_r_3-22-85.u6a",    0x08000, 0x4000, CRC(a7ff789c) SHA1(a3421ae46dadd6f514cfc514ff07dfcca2cb1478) )
	ROM_LOAD( "genus_ii_ab67_r_3-22-85.u5a",    0x0c000, 0x4000, CRC(cc5c68ef) SHA1(38713796e07f84c9a1b21d8c66f76e620132d77e) )
	ROM_LOAD( "genus_ii_rom_cd45r_3-22-85.u2a", 0x18000, 0x4000, CRC(fc9c752a) SHA1(239507fb5d75e86aca295978aab1dd4514d8d761) )
	ROM_RELOAD(                                 0x10000, 0x4000 )
	ROM_RELOAD(                                 0x14000, 0x4000 )
	ROM_LOAD( "genus_ii_rom_d6efr_3-22-85.u1a", 0x1c000, 0x4000, CRC(23b56fb8) SHA1(9ac726de69e4b374886a3542829745f7477d7556) ) /* yes, it's actually D6EFR and not CD6EFR */

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "genus_ii_gr0_r_3-22-85.u6b",  0x00000, 0x4000, CRC(6829de8e) SHA1(4ec494883ba358f2ac7ce8d5a623a2f34b5bc843) ) /* Cart silscreened:  U6B GR01 */
	ROM_LOAD( "genus_ii_gr1_r_3-22-85.u5b",  0x04000, 0x4000, CRC(89398700) SHA1(771ee04baa9a31d435a6234490105878713e7845) ) /* Cart silscreened:  U5B GR23 */
	ROM_LOAD( "genus_ii_grz_r_3-22-85.u4b",  0x08000, 0x4000, CRC(1e870293) SHA1(32149c9c8047854f2b2ad8844c4bd00a8ded588e) ) /* Cart silscreened:  U4B GR45 - yes, it's GRZ and not GR2 */

	ROM_REGION( 0x00001, "cart_pals", 0) /* PAL's located on the cartridge */
	ROM_LOAD( "pal10l8.u1c", 0x0000, 0x0001, NO_DUMP ) /* PAL10L8CN */

	MOTHERBOARD_PALS
ROM_END


/*

Trivial Pursuit (Young Players Edition) (3/29/85)

Cartridge Type:
  006-8025-01-0B REV B
Label:
+-----------------+
| YOUNG           |
| ROM CD6EF R     |
| 3/29/85         |
+-----------------+

NOTE: Cartridge contains an unlabeled PAL at U1C

*/
ROM_START( triviayp ) /* Cart: 006-8025-01-0B REV B */
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "young_rom_ab01_r_3-29-85.u8a",  0x00000, 0x4000, CRC(97d35a85) SHA1(3ee8400fc3a2bf8a2f6374ffc34a4d295ee13bab) )
	ROM_LOAD( "young_rom_ab23_r_3-29-85.u7a",  0x04000, 0x4000, CRC(2ff67c70) SHA1(c45b5fde4ec979322c9e251e66183632552d35bd) )
	ROM_LOAD( "young_rom_ab45_r_3-29-85.u6a",  0x08000, 0x4000, CRC(511a0fab) SHA1(a2fefe2b86028c7e8c15d6a737509b7dc30430cd) )
	ROM_LOAD( "young_rom_ab67_r_3-29-85.u5a",  0x0c000, 0x4000, CRC(df99d00c) SHA1(7eba6b85e2d9a06635e97d12123fd2a17368e6bc) )
	ROM_LOAD( "young_rom_cd45_r_3-29-85.u2a",  0x18000, 0x4000, CRC(ac45809e) SHA1(1151c4e55f21a7e2eb8e163ac782b4449af84cdc) )
	ROM_LOAD( "young_rom_cd6ef_r_3-29-85.u1a", 0x1c000, 0x4000, CRC(a008059f) SHA1(45e4cfc259e801a189ec19fdc58135dbbbe130ea) )

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "young_gr01_r_3-29-85.u6b", 0x00000, 0x4000, CRC(6829de8e) SHA1(4ec494883ba358f2ac7ce8d5a623a2f34b5bc843) ) /* these 3 didn't have "ROM" on the label */
	ROM_LOAD( "young_gr23_r_3-29-85.u5b", 0x04000, 0x4000, CRC(89398700) SHA1(771ee04baa9a31d435a6234490105878713e7845) )
	ROM_LOAD( "young_gr45_r_3-29-85.u4b", 0x08000, 0x4000, CRC(1242033e) SHA1(1a3fe186bb261e2c7d9fbbb2a3103b39bf029b35) )

	ROM_REGION( 0x00001, "cart_pals", 0) /* PAL's located on the cartridge */
	ROM_LOAD( "pal10l8.u1c", 0x0000, 0x0001, NO_DUMP ) /* PAL10L8CN */

	MOTHERBOARD_PALS
ROM_END


ROM_START( triviasp ) /* Cart: 006-8025-01-0B REV B */
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "allsport.u8a", 0x00000, 0x4000, CRC(54b7ff31) SHA1(1bdf9c9eb1a0fb4c1013680372d289882abf4b47) )
	ROM_LOAD( "allsport.u7a", 0x04000, 0x4000, CRC(59fae9d2) SHA1(a555f0679c59bf7c9dad0ecb9656a2f8faf39902) )
	ROM_LOAD( "allsport.u6a", 0x08000, 0x4000, CRC(237b6b95) SHA1(9d2937c1ecea9d92775f380d40f465f68c44fe06) )
	ROM_LOAD( "allsport.u5a", 0x0c000, 0x4000, CRC(b64d7f61) SHA1(25a7034b18a1623209dc0d06bdb4490243d43261) )
	ROM_LOAD( "allsport.u2a", 0x18000, 0x4000, CRC(e45d09d6) SHA1(8bde18d25f8bd1056e42672d428473be23eab260) )
	ROM_LOAD( "allsport.u1a", 0x1c000, 0x4000, CRC(8bb3e831) SHA1(ecc8fb0f2143e3ea03bb52773cc0a81d4dcc742d) )

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "allsport.u6b", 0x00000, 0x4000, CRC(6829de8e) SHA1(4ec494883ba358f2ac7ce8d5a623a2f34b5bc843) )
	ROM_LOAD( "allsport.u5b", 0x04000, 0x4000, CRC(89398700) SHA1(771ee04baa9a31d435a6234490105878713e7845) )
	ROM_LOAD( "allsport.u4b", 0x08000, 0x4000, CRC(7415a7fc) SHA1(93d832434f359ce7b02aef276c89456b16438979) )

	ROM_REGION( 0x00001, "cart_pals", 0) /* PAL's located on the cartridge */
	ROM_LOAD( "pal10l8.u1c", 0x0000, 0x0001, NO_DUMP ) /* PAL10L8CN */

	MOTHERBOARD_PALS
ROM_END


ROM_START( triviaes )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "tp_a2.bin",  0x00000, 0x04000, CRC(b4d69463) SHA1(8d6b2024600ab0a5d76d2b8ec53cf4c6c6618901) )
	ROM_LOAD( "tp_a7.bin",  0x04000, 0x04000, CRC(d78bd4b6) SHA1(0542fc4ef2501c7649b9fd257340c4392a19d7ad) )
	ROM_LOAD( "tp_a4.bin",  0x08000, 0x04000, CRC(0de9e14d) SHA1(3d5fdf8531cb10a41e3f604165fce682e7e019d5) )
	ROM_LOAD( "tp_a5.bin",  0x0c000, 0x04000, CRC(e749adac) SHA1(426665249a57ba6f4a890808a1c84edeade149bb) )
	ROM_LOAD( "tp_a8.bin",  0x10000, 0x04000, CRC(168ef5ed) SHA1(677a83dfcb12af7e13f00213e2eec48fa2fa63c8) )
	ROM_LOAD( "tp_a1.bin",  0x14000, 0x04000, CRC(1f6ef37f) SHA1(c399404e05d817ffb361eb8ef274a86f07085940) )
	ROM_LOAD( "tp_a6.bin",  0x18000, 0x04000, CRC(421c1a29) SHA1(3e0de8734a39fb887aff40e89cb0936d4cacf9a5) )
	ROM_LOAD( "tp_a3.bin",  0x1c000, 0x04000, CRC(c6254f46) SHA1(47f3d05d0c31983ed1576f91fa193fe58e80bb60) )

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "tp_gr3.bin", 0x00000, 0x4000, CRC(6829de8e) SHA1(4ec494883ba358f2ac7ce8d5a623a2f34b5bc843) )
	ROM_LOAD( "tp_gr2.bin", 0x04000, 0x4000, CRC(89398700) SHA1(771ee04baa9a31d435a6234490105878713e7845) )
	ROM_LOAD( "tp_gr1.bin", 0x08000, 0x4000, CRC(1242033e) SHA1(1a3fe186bb261e2c7d9fbbb2a3103b39bf029b35) )

	MOTHERBOARD_PALS
ROM_END

ROM_START( triviaes2 )
	ROM_REGION( 0x20000, "maincpu", 0 ) // all 27128
	ROM_LOAD( "tpe-2cd45.u2a",  0x00000, 0x04000, CRC(ef26b178) SHA1(7bf0453de9192f37f7c8855aaa752c5374e72eb8) )
	ROM_LOAD( "tpe-2ab23.u7a",  0x04000, 0x04000, CRC(348dc874) SHA1(eb5719db02f9cdfcfba47a93f0a4f2745ba96836) )
	ROM_LOAD( "tpe-2cd01.u4a",  0x08000, 0x04000, CRC(9695d8ed) SHA1(9849dbe3303335f7f0568aa0f45a431d60602e54) )
	ROM_LOAD( "tpe-2ab67.u5a",  0x0c000, 0x04000, CRC(808a3e1e) SHA1(ac34a131fea30729bb81a47cc6742a296ce65770) )
	ROM_LOAD( "tpe-2ab01.u8a",  0x10000, 0x04000, CRC(39ddbafd) SHA1(bb06ad80be7c49d0e2c6762b3e2220a85c273c99) )
	ROM_LOAD( "tpe-2cd23.u3a",  0x1c000, 0x04000, CRC(dacd287e) SHA1(3667c835a2b1f35ff69aa28d4f33824f4e457e1a) )
	ROM_LOAD( "tpe-2cdef.u1a",  0x14000, 0x04000, CRC(22f9e1b4) SHA1(f5f5d9dadcd12f8e8f3a715854243f6da8678c23) )
	ROM_LOAD( "tpe-2ab45.u6a",  0x18000, 0x04000, CRC(cf48b8eb) SHA1(f63590bcdd7e17d85f4f490640785e8828358f93) )

	// 2764 on sound board labeled with handwritten 'PANEA'

	ROM_REGION( 0x10000, "gfx1", 0 ) // all 27128
	ROM_LOAD( "tpegr01.u6b", 0x00000, 0x4000, CRC(6829de8e) SHA1(4ec494883ba358f2ac7ce8d5a623a2f34b5bc843) )
	ROM_LOAD( "tpegr23.u5b", 0x04000, 0x4000, CRC(89398700) SHA1(771ee04baa9a31d435a6234490105878713e7845) )
	ROM_LOAD( "tpegr45.u4b", 0x08000, 0x4000, CRC(1242033e) SHA1(1a3fe186bb261e2c7d9fbbb2a3103b39bf029b35) )

	MOTHERBOARD_PALS
ROM_END


/*

Gimme A Break (7/7/85)

Cartridge Type:
  006-8025-01-0B REV B
Label:
+-----------------+
| GimmeABreak     |
| CD 6 EF         |
| 7/7/85          |
+-----------------+

NOTE: Cartridge contains an unlabeled PAL at U1C

*/
ROM_START( gimeabrk ) /* Cart: 006-8025-01-0B REV B */
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "gimmeabreak_ab01_7-7-85.u8a",    0x00000, 0x4000, CRC(18cc53db) SHA1(3bb47c349b3ab7b81e3557e3b4877617fb549c9e) )
	ROM_LOAD( "gimmeabreak_ab23_7-7-85.u7a",    0x04000, 0x4000, CRC(6bd4190a) SHA1(b6562b3575dc8265c01719cfbcb554b69bc1b37f) )
	ROM_LOAD( "gimmeabreak_ab45_7-7-85.u6a",    0x08000, 0x4000, CRC(5dca4f33) SHA1(aa45d5a960491c85f332f22cffe61999fe3db826) )
	ROM_LOAD( "gimmeabreak_cd_6_ef_7-7-85.u1a", 0x1c000, 0x4000, CRC(5e2b3510) SHA1(e3501b9bd73bc724aee0436700625bd2af94f72d) )

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "gimmeabreak_gr01_7-7-85.u6b", 0x00000, 0x4000, CRC(e3cdc476) SHA1(2f17c3f84767850d45192dfb507dd2716ecadc20) )
	ROM_LOAD( "gimmeabreak_gr23_7-7-85.u5b", 0x04000, 0x4000, CRC(0555d9c0) SHA1(da0d1f207ad056b2d82a5ad6382372066883d161) )

	ROM_REGION( 0x00100, "cart_pals", 0) /* PAL's located on the cartridge */
	ROM_LOAD( "pal10l8.u1c", 0x0000, 0x002c, CRC(b4f6b0b7) SHA1(5a439bfb02b4b2cbcbd9b009ccfce1d300a2435e) ) /* PAL10L8CN */

	MOTHERBOARD_PALS
ROM_END


ROM_START( grudge )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "ab0.8a.romab0",                0x00000, 0x8000, CRC(eabeec2b) SHA1(92098512e3dbcda36f42e10fada01323fab4b08a) ) // blank label
	ROM_LOAD( "ab4.9a.romab4",                0x08000, 0x8000, CRC(2dddb371) SHA1(fbe53a78fb17e5dc17bf6a475a170b850e63cdb6) ) // handwritten label "AB4"
	ROM_LOAD( "g.m._cd-0_9-21-87.13a.romcd0", 0x10000, 0x8000, CRC(ad168726) SHA1(c4d084e3752d6c4365d2460ca3146b148dcccc1d) ) // handwritten label "G.M.  CD-0  9-21-87"
	ROM_LOAD( "cd4.15a.romcd4",               0x18000, 0x8000, CRC(1de8dd2e) SHA1(6b538dcf35105bca1ae1bb5387a08b4d1d4f410c) ) // handwritten label "CD4"
	ROM_LOAD( "cd12.18a.romcd12",             0x18000, 0x8000, CRC(1de8dd2e) SHA1(6b538dcf35105bca1ae1bb5387a08b4d1d4f410c) ) // handwritten label "CD12" - same as CD4, confirmed as identical on PCB

	ROM_REGION( 0x10000, "gfx1", 0 )     /* up to 64k of sprites */
	ROM_LOAD( "g.m._gr0_9-21-87.8a.gr0", 0x00000, 0x8000, CRC(b9681f53) SHA1(bb0c516408f1769e018f0ec8707786d4d1e9ef7e) ) // handwritten label "G.M.  GR0  9-21-87"

	MOTHERBOARD_PALS
ROM_END

/*

Grudge Match (v00.90, Italy, location test?)

Cartridge Type:
  006-8025-01-0B REV B
Small rectangle label:
+------+
| 3.V. |
+------+

Small round label with PCB location hand-written plus "M" in red ink

NOTE: Cartridge contains an unlabeled PAL at U1C

*/
ROM_START( grudgei ) /* Cart: 006-8025-01-0B REV B */
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "gm-3a.bin", 0x00000, 0x8000, CRC(eabeec2b) SHA1(92098512e3dbcda36f42e10fada01323fab4b08a) ) // == ab0.8a.romab0
	ROM_LOAD( "gm-4a.bin", 0x08000, 0x8000, CRC(72664f18) SHA1(98202d7a775792d2d1c44a26540ac35afaffa6b2) ) // Modified from above AB4 ROM to use the Italian language
	ROM_LOAD( "gm-1a.bin", 0x10000, 0x8000, CRC(ad168726) SHA1(c4d084e3752d6c4365d2460ca3146b148dcccc1d) ) // == cd0.13a.romcd0
	ROM_LOAD( "gm-2a.bin", 0x18000, 0x8000, CRC(1de8dd2e) SHA1(6b538dcf35105bca1ae1bb5387a08b4d1d4f410c) ) // == cd4.15a.romcd4 & cd12.18a.romcd12
	ROM_LOAD( "gm-5a.u5a", 0x18000, 0x8000, CRC(1de8dd2e) SHA1(6b538dcf35105bca1ae1bb5387a08b4d1d4f410c) ) // == cd4.15a.romcd4 & cd12.18a.romcd12, confirmed as identical on PCB
	ROM_LOAD( "gm-6a.u6a", 0x1e000, 0x2000, CRC(513d8cdd) SHA1(563e5a2b7e71b4e1447bd41339174129a5884517) ) // mostly the same as 2a/5a except for a small table, used for Italian text (corrupt text if we don't use this here.. )

	ROM_REGION( 0x10000, "gfx1", 0 )     /* up to 64k of sprites */
	ROM_LOAD( "gm-6b.u6b", 0x00000, 0x8000, CRC(b9681f53) SHA1(bb0c516408f1769e018f0ec8707786d4d1e9ef7e) ) // == g.m._gr0_9-21-87.8a.gr0

	ROM_REGION( 0x00001, "cart_pals", 0) /* PAL's located on the cartridge */
	ROM_LOAD( "pal10l8.u1c", 0x0000, 0x0001, NO_DUMP ) /* PAL10L8CN */

	MOTHERBOARD_PALS
ROM_END

ROM_START( grudgep )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "grudge.ab0", 0x00000, 0x8000, CRC(260965ca) SHA1(79eb5dc6605974ece3d5564f10c4598204907398) )
	ROM_LOAD( "grudge.ab4", 0x08000, 0x8000, CRC(c6cd734d) SHA1(076546569e9c8ff40f96bd2cac014bcabc53099d) )
	ROM_LOAD( "grudge.cd0", 0x10000, 0x8000, CRC(e51db1f2) SHA1(57fc0f1df358dd6ea982dcbe9c3f79b3f072be53) )
	ROM_LOAD( "grudge.cd4", 0x18000, 0x8000, CRC(6b60e47e) SHA1(5a399942d4ef9b7349fffd07c07092b667cf6247) )

	ROM_REGION( 0x8000, "gfx1", 0 )     /* up to 64k of sprites */
	ROM_LOAD( "grudge.gr0", 0x00000, 0x8000, CRC(b9681f53) SHA1(bb0c516408f1769e018f0ec8707786d4d1e9ef7e) ) // == g.m._gr0_9-21-87.8a.gr0

	MOTHERBOARD_PALS
ROM_END


/*

Mini Golf - Unknown version - shows (C) BALLY SENTE on title screen, other 2 sets show (C) 1985 BALLY SENTE

*/
ROM_START( minigolf )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "ab01.u8a",  0x00000, 0x4000,  CRC(348f827f) SHA1(a013ef3068e14e0738bcfa4de26c0c2df4c0a7f6) )
	ROM_LOAD( "ab23.u7a",  0x04000, 0x4000,  CRC(19a6ff47) SHA1(70b6da3b4186e5b9463f2ea0fefefad21ec80637) )
	ROM_LOAD( "ab45.u6a",  0x08000, 0x4000,  CRC(925d76eb) SHA1(29d2d7b26d2e81817c4d135935dab70a5aa2d146) )
	ROM_LOAD( "ab67.u5a",  0x0c000, 0x4000,  CRC(6a311c9a) SHA1(b0409e5f4bd3bf898b8701561aac6dbbc28417bd) )
	ROM_LOAD( "1a-ver2",   0x10000, 0x10000, CRC(60b6cd58) SHA1(f79bf2d1f6c4e63f666073c5ecb22604c1ab57d8) )

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "gr01.u6b", 0x00000, 0x4000, CRC(8e24d594) SHA1(d35329fb78f90ec478418917aa1ef06d0967e6f8) )
	ROM_LOAD( "gr23.u5b", 0x04000, 0x4000, CRC(3bf355ef) SHA1(691df25b35b00e21ad09d17a21fe98a353aa3dda) )
	ROM_LOAD( "gr45.u4b", 0x08000, 0x4000, CRC(8eb14921) SHA1(fda8b8f8e801360310f7cb1aa4c6aea1fa0a4b25) )

	ROM_REGION( 0x00100, "cart_pals", 0) /* PAL's located on the cartridge */
	ROM_LOAD( "pal10l8.u1c", 0x0000, 0x002c, CRC(5cc09374) SHA1(07798579aeb1e2514034acea6555c0f81c48a41c) ) /* PAL10L8CN */

	MOTHERBOARD_PALS
ROM_END

/*

Mini Golf (11/25/85)

Cartridge Type:
  006-8025-01-0B REV B
Label:
+-----------------+
| MINI GOLF UR    |
| CD6EF           |
| 11/25/85        |
+-----------------+

NOTE: Cartridge contains an unlabeled PAL at U1C
      All ROMs dated 11/25/85 and match the 10/8/85 set except for the CD23
      and CD6EF ROMs which were updated. It's unknown if PAL data is different.

*/
ROM_START( minigolfa ) /* Cart: 006-8025-01-0B REV B */
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "mini_golf_ur_ab01_11-25-85.u8a",  0x00000, 0x4000, CRC(348f827f) SHA1(a013ef3068e14e0738bcfa4de26c0c2df4c0a7f6) )
	ROM_LOAD( "mini_golf_ur_ab23_11-25-85.u7a",  0x04000, 0x4000, CRC(19a6ff47) SHA1(70b6da3b4186e5b9463f2ea0fefefad21ec80637) )
	ROM_LOAD( "mini_golf_ur_ab45_11-25-85.u6a",  0x08000, 0x4000, CRC(925d76eb) SHA1(29d2d7b26d2e81817c4d135935dab70a5aa2d146) )
	ROM_LOAD( "mini_golf_ur_ab67_11-25-85.u5a",  0x0c000, 0x4000, CRC(6a311c9a) SHA1(b0409e5f4bd3bf898b8701561aac6dbbc28417bd) )
	ROM_LOAD( "mini_golf_ur_cd23_11-25-85.u3a",  0x14000, 0x4000, CRC(8c18b38d) SHA1(c6e7b95c59603066050d42de8609dc15ad5898f6) ) /* Only these 2 ROMs were updated from the 10/8/85 set */
	/* U2A is unpopulated */
	ROM_LOAD( "mini_golf_ur_cd6ef_11-25-85.u1a", 0x1c000, 0x4000, CRC(38bf5962) SHA1(e1a31cfb4d89b7a4b59acf99325b823a1a389029) ) /* Only these 2 ROMs were updated from the 10/8/85 set */

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "mini_golf_ur_gr01_11-25-85.u6b", 0x00000, 0x4000, CRC(8e24d594) SHA1(d35329fb78f90ec478418917aa1ef06d0967e6f8) )
	ROM_LOAD( "mini_golf_ur_gr23_11-25-85.u5b", 0x04000, 0x4000, CRC(3bf355ef) SHA1(691df25b35b00e21ad09d17a21fe98a353aa3dda) )
	ROM_LOAD( "mini_golf_ur_gr45_11-25-85.u4b", 0x08000, 0x4000, CRC(8eb14921) SHA1(fda8b8f8e801360310f7cb1aa4c6aea1fa0a4b25) )

	ROM_REGION( 0x00100, "cart_pals", 0) /* PAL's located on the cartridge */
	ROM_LOAD( "pal10l8.u1c", 0x0000, 0x002c, CRC(5cc09374) SHA1(07798579aeb1e2514034acea6555c0f81c48a41c) ) /* PAL10L8CN */

	MOTHERBOARD_PALS
ROM_END

/*

Currently undumped:

Mini Golf Cocktail (10/18/85)

Cartridge Type:
  006-8025-01-0B REV B
Label:
+-----------------+
| MINI GOLF CT    |
| CD6EF           |
| 10/18/85        |
+-----------------+

NOTE: Cartridge contains an unlabeled PAL at U1C

*/

/*

Mini Golf (10/8/85)

Cartridge Type:
  006-8025-01-0B REV B
Label:
+-----------------+
| MINI GOLF UR    |
| CD6EF           |
| 10/8/85         |
+-----------------+

NOTE: Cartridge contains an unlabeled PAL at U1C

*/
ROM_START( minigolfb ) /* Cart: 006-8025-01-0B REV B */
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "mini_golf_ur_ab01_10-8-85.u8a",  0x00000, 0x4000, CRC(348f827f) SHA1(a013ef3068e14e0738bcfa4de26c0c2df4c0a7f6) )
	ROM_LOAD( "mini_golf_ur_ab23_10-8-85.u7a",  0x04000, 0x4000, CRC(19a6ff47) SHA1(70b6da3b4186e5b9463f2ea0fefefad21ec80637) )
	ROM_LOAD( "mini_golf_ur_ab45_10-8-85.u6a",  0x08000, 0x4000, CRC(925d76eb) SHA1(29d2d7b26d2e81817c4d135935dab70a5aa2d146) )
	ROM_LOAD( "mini_golf_ur_ab67_10-8-85.u5a",  0x0c000, 0x4000, CRC(6a311c9a) SHA1(b0409e5f4bd3bf898b8701561aac6dbbc28417bd) )
	ROM_LOAD( "mini_golf_ur_cd23_10-8-85.u3a",  0x14000, 0x4000, CRC(52279801) SHA1(d8de92c296d5c91db3bea7a0093260158961036e) )
	/* U2A is unpopulated */
	ROM_LOAD( "mini_golf_ur_cd6ef_10-8-85.u1a", 0x1c000, 0x4000, CRC(34c64f4c) SHA1(ce55f5f6ebddcacf20cb78fb738b5f569b531b61) )

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "mini_golf_ur_gr01_10-8-85.u6b", 0x00000, 0x4000, CRC(8e24d594) SHA1(d35329fb78f90ec478418917aa1ef06d0967e6f8) )
	ROM_LOAD( "mini_golf_ur_gr23_10-8-85.u5b", 0x04000, 0x4000, CRC(3bf355ef) SHA1(691df25b35b00e21ad09d17a21fe98a353aa3dda) )
	ROM_LOAD( "mini_golf_ur_gr45_10-8-85.u4b", 0x08000, 0x4000, CRC(8eb14921) SHA1(fda8b8f8e801360310f7cb1aa4c6aea1fa0a4b25) )

	ROM_REGION( 0x00100, "cart_pals", 0) /* PAL's located on the cartridge */
	ROM_LOAD( "pal10l8.u1c", 0x0000, 0x002c, CRC(5cc09374) SHA1(07798579aeb1e2514034acea6555c0f81c48a41c) ) /* PAL10L8CN */

	MOTHERBOARD_PALS
ROM_END


ROM_START( toggle )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "tgle-ab0.bin", 0x00000, 0x2000, CRC(8c7b7fad) SHA1(13eaf5b3727ff8b7ce2cfbab99541ca1e538aeba) )
	ROM_LOAD( "tgle-ab1.bin", 0x02000, 0x2000, CRC(771e5434) SHA1(b1bcefc81054c81a22a91106c5bc99ef204cd009) )
	ROM_LOAD( "tgle-ab2.bin", 0x04000, 0x2000, CRC(9b4baa3f) SHA1(5b0776d983ad40a0051939810bb854f014fea28b) )
	ROM_LOAD( "tgle-ab3.bin", 0x06000, 0x2000, CRC(35308a41) SHA1(3846446b60897bfce8fcfd1561b5b74cdd19c36e) )
	ROM_LOAD( "tgle-ab4.bin", 0x08000, 0x2000, CRC(baf5617b) SHA1(95c91fc81c975f522c1bd4f14bfb5f453801ffb6) )
	ROM_LOAD( "tgle-ab5.bin", 0x0a000, 0x2000, CRC(88077dad) SHA1(51b36177a4bfbb62c91d87282bfc1ff791626d19) )
	ROM_LOAD( "tgle-cd.bin",  0x1c000, 0x2000, CRC(0a2bb949) SHA1(350dc782fc21640794c6ecb502554cb693adbb7d) )
	ROM_LOAD( "tgle-ef.bin",  0x1e000, 0x2000, CRC(3ec10804) SHA1(ae719081e8114ccc23c6b24c7fe904a11fbdd992) )

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "tgle-gr0.bin", 0x00000, 0x2000, CRC(0e0e5d0e) SHA1(363858ce08767f8a9b8eaec56405377cdd74b178) )
	ROM_LOAD( "tgle-gr1.bin", 0x02000, 0x2000, CRC(3b141ad2) SHA1(72430fd616adbc72d86a5f10672572a31bed0b5d) )

	MOTHERBOARD_PALS
ROM_END


/*

Currently undumped:

Name That Tune (Bally, 4/22/86)

Cartridge Type:
  006-8030-01-0A REV A

Label:
+-----------------+
| NAMETHATTUNE UR |
| CD 6 EF         |
| 4/22/86         |
+-----------------+

NOTE: Cartridge contains Sente ST1002 40pin DIP chip at U6B
      Cartridge contains an unlabeled PAL at U7C

*/

/*

Name That Tune (Bally, 3/31/86)

Cartridge Type:
  006-8030-01-0A REV A

Label:
+-----------------+
| NAMETHATTUNE UR |
| CD 6 EF         |
| 3/31/86         |
+-----------------+

NOTE: Cartridge contains Sente ST1002 40pin DIP chip at U6B
      Cartridge contains an unlabeled PAL at U7C

*/
ROM_START( nametune ) /* Cart: 006-8030-01-0A REV A */
	ROM_REGION( 0x40000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "namethattune_ur_ab_01_3-31-86.u8a",   0x00000, 0x4000, CRC(f99054f1) SHA1(aaa3aae71f67be2df34b9682b1b4092a208fbf26) )
	ROM_CONTINUE(                                    0x20000, 0x4000 )
	ROM_LOAD( "namethattune_ur_ab_23_3-31-86.u7a",   0x04000, 0x4000, CRC(f2b8f7fa) SHA1(b9f81a29b031af31118b77e77fc29e59f2059109) )
	ROM_CONTINUE(                                    0x24000, 0x4000 )
	ROM_LOAD( "namethattune_ur_ab_45_3-31-86.u6a",   0x08000, 0x4000, CRC(89e1c769) SHA1(8e976182d99b93bb1cf6e306d134b66ba6fe6052) )
	ROM_CONTINUE(                                    0x28000, 0x4000 )
	ROM_LOAD( "namethattune_ur_ab_67_3-31-86.u5a",   0x0c000, 0x4000, CRC(7e5572a1) SHA1(d957a495ad4100b857e163d7399528f62e8a39a7) )
	ROM_CONTINUE(                                    0x2c000, 0x4000 )
	ROM_LOAD( "namethattune_ur_cd_01_3-31-86.u4a",   0x10000, 0x4000, CRC(db9d6154) SHA1(8db17fda6c4113f5b791163fc9e289cf3f003a51) )
	ROM_CONTINUE(                                    0x30000, 0x4000 )
	ROM_LOAD( "namethattune_ur_cd_23_3-31-86.u3a",   0x14000, 0x4000, CRC(9d2e458f) SHA1(f08c2d7ba6be9745d13fc9dc7141ad101a8b747e) )
	ROM_CONTINUE(                                    0x34000, 0x4000 )
	ROM_LOAD( "namethattune_ur_cd_45_3-31-86.u2a",   0x18000, 0x4000, CRC(9a4b87aa) SHA1(ca82ddd4d8d40b35ba21cb9333e182b8a2e7f95e) )
	ROM_CONTINUE(                                    0x38000, 0x4000 )
	ROM_LOAD( "namethattune_ur_cd_6_ef_3-31-86.u1a", 0x1c000, 0x4000, CRC(0459e6f8) SHA1(7dbdbfa8f2e9e3956af926f5f782b8d3c3334099) )
	ROM_CONTINUE(                                    0x3c000, 0x4000 )

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "namethattune_ur_gr_0_3-31-86.u3c",  0x00000, 0x8000, CRC(6b75bb4b) SHA1(e7131d112fb0b36985c5b6383700f55728a1c4fd) )

	ROM_REGION( 0x00001, "cart_pals", 0) /* PAL's located on the cartridge */
	ROM_LOAD( "pal16r8.u7c", 0x00000, 0x0001, NO_DUMP )  /* PAL16R8ANC */

	MOTHERBOARD_PALS
ROM_END

/*

Name That Tune (Bally, 3/23/86)

Cartridge Type:
  006-8030-01-0A REV A
Label:
+-----------------+
| NMETNEUR        |
| CD 6 EF         |
| 3/23/86         |
+-----------------+

NOTE: Cartridge contains Sente ST1002 40pin DIP chip at U6B
      Cartridge contains an unlabeled PAL at U7C

*/
ROM_START( nametunea ) /* Cart: 006-8030-01-0A REV A */
	ROM_REGION( 0x40000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "nmetneur_ab_01_3-23-86.u8a",   0x00000, 0x4000, CRC(4044891d) SHA1(4e1e7cb9846939e03b035b95ba04f62a78719bb2) )
	ROM_CONTINUE(                             0x20000, 0x4000 )
	ROM_LOAD( "nmetneur_ab_23_3-23-86.u7a",   0x04000, 0x4000, CRC(df3454bc) SHA1(82faf87ca8974629e546b6854718908721b64ad0) )
	ROM_CONTINUE(                             0x24000, 0x4000 )
	ROM_LOAD( "nmetneur_ab_45_3-23-86.u6a",   0x08000, 0x4000, CRC(fb4050b0) SHA1(a4d185e02aa08e886b90454f681f49de3de76f86) )
	ROM_CONTINUE(                             0x28000, 0x4000 )
	ROM_LOAD( "nmetneur_ab_67_3-23-86.u5a",   0x0c000, 0x4000, CRC(276a28f4) SHA1(cceeb9c05ff72cfe86ab55555055cf8195d3ea16) )
	ROM_CONTINUE(                             0x2c000, 0x4000 )
	ROM_LOAD( "nmetneur_cd_01_3-23-86.u4a",   0x10000, 0x4000, CRC(88bed028) SHA1(69c83ba07f34dd1d45f432e2ed6a50e2d13c4acb) )
	ROM_CONTINUE(                             0x30000, 0x4000 )
	ROM_LOAD( "nmetneur_cd_23_3-23-86.u3a",   0x14000, 0x4000, CRC(38c63308) SHA1(1a26642cbe91ebc96444eb05fa1454c9175d370c) )
	ROM_CONTINUE(                             0x34000, 0x4000 )
	ROM_LOAD( "nmetneur_cd_45_3-23-86.u2a",   0x18000, 0x4000, CRC(d19a3671) SHA1(b68010ad235175c73258f64f9a64b37b2a06efdc) )
	ROM_CONTINUE(                             0x38000, 0x4000 )
	ROM_LOAD( "nmetneur_cd_6_ef_3-23-86.u1a", 0x1c000, 0x4000, CRC(e73c7cda) SHA1(c6f751923d0c7930db2e173f680674759f94c8bb) )
	ROM_CONTINUE(                             0x3c000, 0x4000 )

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "nmetneur_gr_0_3-23-86.u3c",    0x00000, 0x8000, CRC(a0121b80) SHA1(ba38e9b738baac85fa33ae3751d02cb223fa3e65) )

	ROM_REGION( 0x00001, "cart_pals", 0) /* PAL's located on the cartridge */
	ROM_LOAD( "pal16r8.u7c", 0x00000, 0x0001, NO_DUMP )  /* PAL16R8ANC */

	MOTHERBOARD_PALS
ROM_END


/*
    Night Stocker

    This game utilizes the standard motherboard and sound board, but in addition it
    also uses a Gun Interface Board (Board: 006-8032-01-0B) and video interface? board.


Night Stocker (10/6/86)

Cartridge Type:
  006-8027-01-0B REV B
Label:
+-----------------+
| NIGHT STOCKER   |
| CD 6 EF         |
| 10/06/86        |
+-----------------+

NOTE: Cartridge contains Sente ST1002 40pin DIP chip at U6B
      Cartridge contains a PAL at U7C labeled NITESTKR10/6/86
      All ROMs dated 10/06/86 and match the 8/27/86 set except for the
      CD 6 EF ROM which was updated. It's unknown if PAL data is different.

*/
ROM_START( nstocker ) /* Cart: 006-8027-01-0B REV B */
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "night_stocker_ab_01_10-06-86.u8a",   0x00000, 0x4000, CRC(a635f973) SHA1(edb12469818a3114fb97d21e11c63eb37678a07b) )
	ROM_LOAD( "night_stocker_ab_23_10-06-86.u7a",   0x04000, 0x4000, CRC(223acbb2) SHA1(195ebd349722cce323616c81cc4e86f0a9c6fa13) )
	ROM_LOAD( "night_stocker_ab_45_10-06-86.u6a",   0x08000, 0x4000, CRC(27a728b5) SHA1(c72634112a04d58a695fb43bf30f44e3f7ba7de2) )
	ROM_LOAD( "night_stocker_ab_67_10-06-86.u5a",   0x0c000, 0x4000, CRC(2999cdf2) SHA1(a64ae04f264ad286a87069cfb176e7511df08e78) )
	ROM_LOAD( "night_stocker_cd_01_10-06-86.u4a",   0x10000, 0x4000, CRC(75e9b51a) SHA1(dbe575d37836245746ea85ffe85e8e6665ec37ea) )
	ROM_LOAD( "night_stocker_cd_23_10-06-86.u3a",   0x14000, 0x4000, CRC(0a32e0a5) SHA1(dedbe08aed483bae27e1a607334e24cdfcb2f851) )
	ROM_LOAD( "night_stocker_cd_45_10-06-86.u2a",   0x18000, 0x4000, CRC(9bb292fe) SHA1(6fc7abcc110c2cf7399d11a478cfdadb3439b6ab) )
	ROM_LOAD( "night_stocker_cd_6_ef_10-06-86.u1a", 0x1c000, 0x4000, CRC(e77c1aea) SHA1(9e2e595530cb15c634a6052c773ff5d998c0c828) ) /* The only ROM updated from the 8/27/86 set */

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "night_stocker_gr_01_10-06-86.u4c", 0x00000, 0x4000, CRC(fd0c38be) SHA1(b9e12e76f44f2b2b3ca6a57c58f0cbb019b1971f) )
	ROM_LOAD( "night_stocker_gr_23_10-06-86.u3c", 0x04000, 0x4000, CRC(35d4433e) SHA1(399d04c2a29d993f77d0d5c2d62915081d4a85dd) )
	ROM_LOAD( "night_stocker_gr_45_10-06-86.u2c", 0x08000, 0x4000, CRC(734b858a) SHA1(71763789807021938b840a88af34aad7f4751298) )
	ROM_LOAD( "night_stocker_gr_67_10-06-86.u1c", 0x0c000, 0x4000, CRC(3311f9c0) SHA1(63b185c761b258113c31cc269ce0b1462bf37f40) )

	ROM_REGION( 0x00001, "cart_pals", 0) /* PAL's located on the cartridge */
	ROM_LOAD( "nitestkr10-6-86.u7c", 0x0000, 0x0001, NO_DUMP ) /* PAL16R8 */

	ROM_REGION( 0x00004, "gun_io_bd_pals", 0) /* Gun I/O Board PALs */
	ROM_LOAD( "pal16r8acn.u6", 0x0000, 0x0001, NO_DUMP ) /* PAL16R8 */
	ROM_LOAD( "pal16r8acn.u7", 0x0000, 0x0001, NO_DUMP ) /* PAL16R8 */
	ROM_LOAD( "pal16r8acn.u8", 0x0000, 0x0001, NO_DUMP ) /* PAL16R8 */
	ROM_LOAD( "pal16r8acn.u9", 0x0000, 0x0001, NO_DUMP ) /* PAL16R8 */

	MOTHERBOARD_PALS
ROM_END

/*

Night Stocker (8/27/86)

Cartridge Type:
  006-8027-01-0B REV B
Label:
+-----------------+
| NIGHT STOCKER   |
| CD 6 EF         |
| 8/27/86         |
+-----------------+

NOTE: Cartridge contains Sente ST1002 40pin DIP chip at U6B
      Cartridge contains a PAL at U7C labeled NITESTKR8/27/86

*/
ROM_START( nstockera ) /* Cart: 006-8027-01-0B REV B */
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "night_stocker_ab_01_8-27-86.u8a",   0x00000, 0x4000, CRC(a635f973) SHA1(edb12469818a3114fb97d21e11c63eb37678a07b) )
	ROM_LOAD( "night_stocker_ab_23_8-27-86.u7a",   0x04000, 0x4000, CRC(223acbb2) SHA1(195ebd349722cce323616c81cc4e86f0a9c6fa13) )
	ROM_LOAD( "night_stocker_ab_45_8-27-86.u6a",   0x08000, 0x4000, CRC(27a728b5) SHA1(c72634112a04d58a695fb43bf30f44e3f7ba7de2) )
	ROM_LOAD( "night_stocker_ab_67_8-27-86.u5a",   0x0c000, 0x4000, CRC(2999cdf2) SHA1(a64ae04f264ad286a87069cfb176e7511df08e78) )
	ROM_LOAD( "night_stocker_cd_01_8-27-86.u4a",   0x10000, 0x4000, CRC(75e9b51a) SHA1(dbe575d37836245746ea85ffe85e8e6665ec37ea) )
	ROM_LOAD( "night_stocker_cd_23_8-27-86.u3a",   0x14000, 0x4000, CRC(0a32e0a5) SHA1(dedbe08aed483bae27e1a607334e24cdfcb2f851) )
	ROM_LOAD( "night_stocker_cd_45_8-27-86.u2a",   0x18000, 0x4000, CRC(9bb292fe) SHA1(6fc7abcc110c2cf7399d11a478cfdadb3439b6ab) )
	ROM_LOAD( "night_stocker_cd_6_ef_8-27-86.u1a", 0x1c000, 0x4000, CRC(c77d2302) SHA1(2b0956a7d6bdff5e4f77084149a9528fb07154dc) )

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "night_stocker_gr_01_8-27-86.u4c", 0x00000, 0x4000, CRC(fd0c38be) SHA1(b9e12e76f44f2b2b3ca6a57c58f0cbb019b1971f) )
	ROM_LOAD( "night_stocker_gr_23_8-27-86.u3c", 0x04000, 0x4000, CRC(35d4433e) SHA1(399d04c2a29d993f77d0d5c2d62915081d4a85dd) )
	ROM_LOAD( "night_stocker_gr_45_8-27-86.u2c", 0x08000, 0x4000, CRC(734b858a) SHA1(71763789807021938b840a88af34aad7f4751298) )
	ROM_LOAD( "night_stocker_gr_67_8-27-86.u1c", 0x0c000, 0x4000, CRC(3311f9c0) SHA1(63b185c761b258113c31cc269ce0b1462bf37f40) )

	ROM_REGION( 0x00001, "cart_pals", 0) /* PAL's located on the cartridge */
	ROM_LOAD( "nitestkr8-27-86.u7c", 0x0000, 0x0001, NO_DUMP ) /* PAL16R8 */

	ROM_REGION( 0x00004, "gun_io_bd_pals", 0) /* Gun I/O Board PALs */
	ROM_LOAD( "pal16r8acn.u6", 0x0000, 0x0001, NO_DUMP ) /* PAL16R8 */
	ROM_LOAD( "pal16r8acn.u7", 0x0000, 0x0001, NO_DUMP ) /* PAL16R8 */
	ROM_LOAD( "pal16r8acn.u8", 0x0000, 0x0001, NO_DUMP ) /* PAL16R8 */
	ROM_LOAD( "pal16r8acn.u9", 0x0000, 0x0001, NO_DUMP ) /* PAL16R8 */

	MOTHERBOARD_PALS
ROM_END


/*

Street Football (11/12/86)

Cartridge Type:
  006-8027-01-0B REV B
Label:
+-----------------+
| STREET FOOTBALL |
| CD 6 EF         |
| 11/12/86        |
+-----------------+

NOTE: Cartridge contains Sente ST1002 40pin DIP chip at U6B
      Cartridge contains a PAL at U7C labeled STR.FTBALL10/31/86

* There might be an undumped version dated 10/31/86 like the PAL

*/
ROM_START( sfootbal ) /* Cart: 006-8027-01-0B REV B */
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "street_football_ab_01_11-12-86.u8a",   0x00000, 0x4000, CRC(2a69803f) SHA1(ca86c9d079fbebae4c93c889d98a8573facc05da) )
	ROM_LOAD( "street_football_ab_23_11-12-86.u7a",   0x04000, 0x4000, CRC(89f157c2) SHA1(59701b7770dce7ec01d0feb01d67450943e6cfbb) )
	ROM_LOAD( "street_football_ab_45_11-12-86.u6a",   0x08000, 0x4000, CRC(91ad42c5) SHA1(0b6fc3ed3a633c825809668d49f209c130f3e978) )
	ROM_LOAD( "street_football_cd_6_ef_11-12-86.u1a", 0x1c000, 0x4000, CRC(bf80bb1a) SHA1(2b70b36d946c36e3f354c7edfd3e34784ffce406) )

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "street_football_gr_01_11-12-86.u4c", 0x00000, 0x4000, CRC(e3108d35) SHA1(05b7f1a1a18d7f72a3d3f6102cb8ab42421b7366) )
	ROM_LOAD( "street_football_gr_23_11-12-86.u3c", 0x04000, 0x4000, CRC(5c5af726) SHA1(04cdd476e6689d17273659fb1fe0ca642edbe5a8) )
	ROM_LOAD( "street_football_gr_45_11-12-86.u2c", 0x08000, 0x4000, CRC(e767251e) SHA1(3c05295317a673fb1de5924f27de276d2846d805) )
	ROM_LOAD( "street_football_gr_67_11-12-86.u1c", 0x0c000, 0x4000, CRC(42452a7a) SHA1(37479d6e9071ac775215a6815dbaf280b3c6a57f) )

	ROM_REGION( 0x00001, "cart_pals", 0) /* PAL's located on the cartridge */
	ROM_LOAD( "str.ftball10-31-86.u7c", 0x0000, 0x0001, NO_DUMP ) /* PAL16R8ANC */

	MOTHERBOARD_PALS
ROM_END


/*

Spiker (6/9/86)

Cartridge Type:
  006-8027-01-0B REV B
Label:
+-----------------+
| SPIKER U R      |
| CD 6 EF         |
| 6/09/86         |
+-----------------+

NOTE: Only the CD6EF ROM was dated 6/09/86
      Cartridge contains an unlabeled PAL at U7C

*/
ROM_START( spiker ) /* Cart: 006-8027-01-0B REV B */
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "spiker_u_r_ab01_5-05-86.u8a",    0x00000, 0x4000, CRC(2d53d023) SHA1(01c1d2cd7d8be60c40527e9c1571b84388a39bd8) )
	ROM_LOAD( "spiker_u_r_ab23_5-05-86.u7a",    0x04000, 0x4000, CRC(3be87edf) SHA1(0d4f1ff501d5d865abc3906f6b232ec04586d3dc) )
	ROM_LOAD( "spiker_u_r_cd_6_ef_6-09-86.u1a", 0x1c000, 0x4000, CRC(5b5a6d86) SHA1(a173637991601adc87f0fc8fd1ee9102f5fb2b81) ) /* The only ROM updated from the 5/05/86 set */

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "spiker_u_r_gr01_5-05-86.u4c", 0x00000, 0x4000, CRC(0caa6e3e) SHA1(ce6765d44e444d24129ec99f04a41a866a32eee2) )
	ROM_LOAD( "spiker_u_r_gr23_5-05-86.u3c", 0x04000, 0x4000, CRC(970c81f6) SHA1(f22189e172a795d115597feb48ccbc04be3859b9) )
	ROM_LOAD( "spiker_u_r_gr45_5-05-86.u2c", 0x08000, 0x4000, CRC(90ddd737) SHA1(8e1dde2f42e9bf755dedeef218745d1fc54faac7) )

	ROM_REGION( 0x00001, "cart_pals", 0) /* PAL's located on the cartridge */
	ROM_LOAD( "pal16r8.u7c", 0x0000, 0x0001, NO_DUMP ) /* PAL16R8ANC - One cart showed hand written label of:  SPKR */

	MOTHERBOARD_PALS
ROM_END

/*

Spiker (5/5/86)

Cartridge Type:
  006-8027-01-0B REV B
Label:
+-----------------+
| SPIKER U R      |
| CD 6 EF         |
| 5/05/86         |
+-----------------+

NOTE: Cartridge contains an unlabeled PAL at U7C

*/
ROM_START( spikera ) /* Cart: 006-8027-01-0B REV B */
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "spiker_u_r_ab01_5-05-86.u8a",    0x00000, 0x4000, CRC(2d53d023) SHA1(01c1d2cd7d8be60c40527e9c1571b84388a39bd8) )
	ROM_LOAD( "spiker_u_r_ab23_5-05-86.u7a",    0x04000, 0x4000, CRC(3be87edf) SHA1(0d4f1ff501d5d865abc3906f6b232ec04586d3dc) )
	ROM_LOAD( "spiker_u_r_cd_6_ef_5-05-86.u1a", 0x1c000, 0x4000, CRC(f2c73ece) SHA1(4fc108823102fd17c5b7d9be1a0c76667788ba1a) )

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "spiker_u_r_gr01_5-05-86.u4c", 0x00000, 0x4000, CRC(0caa6e3e) SHA1(ce6765d44e444d24129ec99f04a41a866a32eee2) )
	ROM_LOAD( "spiker_u_r_gr23_5-05-86.u3c", 0x04000, 0x4000, CRC(970c81f6) SHA1(f22189e172a795d115597feb48ccbc04be3859b9) )
	ROM_LOAD( "spiker_u_r_gr45_5-05-86.u2c", 0x08000, 0x4000, CRC(90ddd737) SHA1(8e1dde2f42e9bf755dedeef218745d1fc54faac7) )

	ROM_REGION( 0x00001, "cart_pals", 0) /* PAL's located on the cartridge */
	ROM_LOAD( "pal16r8.u7c", 0x0000, 0x0001, NO_DUMP ) /* PAL16R8ANC */

	MOTHERBOARD_PALS
ROM_END

/*

Spiker - Earliest set:

 Doesn't show "ONE MOMENT PLEASE" at boot screen or when the service button is pressed
 Doesn't show the "HINT: TRY TO KEEP YOUR FEET ON THE SHADOW OF BALL." message during demo play

*/
ROM_START( spikerb ) /* Cart: 006-8027-01-0B REV B */
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "ab01.u8a",  0x00000, 0x4000, CRC(59025e39) SHA1(f0e3e45bb32cc6664831c4ef6b0cfabf3fc71f58) )
	ROM_LOAD( "ab23.u7a",  0x04000, 0x4000, CRC(ffb23288) SHA1(3458e486794f6c936d15e837be0f419027b01311) )
	ROM_LOAD( "cd6ef.u1a", 0x1c000, 0x4000, CRC(7f04774d) SHA1(c49ac3aa86425cdbab9877fc253999329bb99a49) )

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "gr01.u4c", 0x00000, 0x4000, CRC(1228b7a3) SHA1(70a207714ba7bc4f4dbc492768480afa424b31c0) )
	ROM_LOAD( "gr23.u3c", 0x04000, 0x4000, CRC(970c81f6) SHA1(f22189e172a795d115597feb48ccbc04be3859b9) )
	ROM_LOAD( "gr45.u2c", 0x08000, 0x4000, CRC(bf2b413d) SHA1(f0f797853ac1b6e45ff606d7aa5c9350765efd48) )

	ROM_REGION( 0x00001, "cart_pals", 0) /* PAL's located on the cartridge */
	ROM_LOAD( "pal16r8.u7c", 0x0000, 0x0001, NO_DUMP ) /* PAL16R8ANC */

	MOTHERBOARD_PALS
ROM_END


ROM_START( stompin ) /* Cart: 006-8027-01-0B REV B */
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "ab 01.u8a",   0x00000, 0x4000, CRC(46f428c6) SHA1(06c59d06ccc0bd7067e419f12781050ab4ac98c2) )
	ROM_LOAD( "ab 23.u7a",   0x04000, 0x4000, CRC(0e13132f) SHA1(d572e5d170df99bb99db7d41ede881c24e5b8d1c) )
	ROM_LOAD( "ab 45.u6a",   0x08000, 0x4000, CRC(6ed26069) SHA1(35f6b8cff54c35a1a0eeb9c23e446ade69d13375) )
	ROM_LOAD( "ab 67.u5a",   0x0c000, 0x4000, CRC(7f63b516) SHA1(4ffd9dd579c8c4574f2f039b30761e901ee6dd5c) )
	ROM_LOAD( "cd 23.u3a",   0x14000, 0x4000, CRC(52b29048) SHA1(e0873137201ad9b2e87a17dd68046e88dbeeb5e1) )
	ROM_LOAD( "cd 6 ef.u1a", 0x1c000, 0x4000, CRC(b880961a) SHA1(11700af516517b7176a840fd5a8fd5ed0fb9bd6e) )

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "gr 01.u4c",   0x00000, 0x4000, CRC(14ffdd1e) SHA1(4528548c07789f9dca2cabd2c64ea1ff8f36a99e) )
	ROM_LOAD( "gr 23.u3c",   0x04000, 0x4000, CRC(761abb80) SHA1(a1278e93a4fa66cc4d347954dd45121120da568d) )
	ROM_LOAD( "gr 45.u2c",   0x08000, 0x4000, CRC(0d2cf2e6) SHA1(beccb1342127e79a845c4b6b20f20052097ebb98) )
	ROM_LOAD( "gr 67.u2c",   0x0c000, 0x4000, CRC(2bab2784) SHA1(a4020fd8f5ca2fdb37efd37cbccf86cae0468eb0) )

	ROM_REGION( 0x00001, "cart_pals", 0) /* PAL's located on the cartridge */
	ROM_LOAD( "pal16r8.u7c", 0x00000, 0x0001, NO_DUMP ) /* PAL16R8ACN */

	MOTHERBOARD_PALS
ROM_END

// earlier version with various differences (different cart type, copyright date, one less point in the instructions screen, less dip definitions in test mode, etc..)
// all main CPU ROMs test bad (same on PCB), but game seems to work fine. Possibly a prototype for which they didn't bother to correct the check? Also has an undocumented
// invulnerability switch that was apparently removed from the later release.
ROM_START( stompina ) /* Cart: 006-8025-01-0B REV B */
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "st-ab 01.u8a",   0x00000, 0x4000, BAD_DUMP CRC(b2d6d1cb) SHA1(54b22f045889aed91bedcf403f3db2b9df9e3706) ) // this ROM and the following are probably good, but fail test mode, so..
	ROM_LOAD( "st-ab 23.u7a",   0x04000, 0x4000, BAD_DUMP CRC(cbf7e8cf) SHA1(6fb72c505372dec6a13e9bef7af83df4105b1813) )
	ROM_LOAD( "st-ab 45.u6a",   0x08000, 0x4000, BAD_DUMP CRC(5b61e80d) SHA1(644f05cbce484bf37075c60d9cc439cee6a71c70) )
	ROM_LOAD( "st-ab 67.u5a",   0x0c000, 0x4000, BAD_DUMP CRC(84514d4e) SHA1(350a4948e260e50f317bc948a85a87f3a9e34991) )
	ROM_LOAD( "st-cd 6 ef.u1a", 0x1c000, 0x4000, BAD_DUMP CRC(ca4a7f0e) SHA1(94dbe21bd639c32918d924cfa5b11f438f88a0a7) )

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "st-gr 01.u6b",   0x00000, 0x4000, CRC(03f85f1b) SHA1(2d2815fbe80fc63dc1dab2fe4b2deb286d200530) )
	ROM_LOAD( "st-gr 23.u5b",   0x04000, 0x4000, CRC(583f6d8c) SHA1(289a377fe76c944d4a409e2d025e57a621fbdcec) )
	ROM_LOAD( "st-gr 45.u4b",   0x08000, 0x4000, CRC(bbc5714e) SHA1(70a6a3453f97d6b5b7c3e317bffbc2f103643531) )
	ROM_LOAD( "st-gr 67.u3b",   0x0c000, 0x4000, CRC(b7347814) SHA1(c6ad850a3368bb733d61435caa569e03814e94cf) )

	ROM_REGION( 0x00001, "cart_pals", 0) /* PAL's located on the cartridge */
	ROM_LOAD( "pal16l8.u1c", 0x00000, 0x0001, NO_DUMP ) /* PAL16R8ACN */

	MOTHERBOARD_PALS
ROM_END


ROM_START( rescraid )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "ab 1.a10",  0x00000, 0x8000, CRC(33a76b47) SHA1(72cefb3ae7d0ecfc099f9d09a26533dd7ca7c4f2) )
	ROM_LOAD( "ab 12.a12", 0x08000, 0x8000, CRC(7c7a9f12) SHA1(2dbe1158d124ecd24aeb6e46079a8e08fda61208) )
	ROM_LOAD( "cd 8.a16",  0x10000, 0x8000, CRC(90917a43) SHA1(3abd68d0c147ed792ace41f701c04bc225efede4) )
	ROM_LOAD( "cd 12.a18", 0x18000, 0x8000, CRC(0450e9d7) SHA1(b5d0a79d1bac3596d241f80ac4e3e13c98d28709) )

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "gr 0.a5",   0x00000, 0x8000, CRC(e0dfc133) SHA1(0b120b4410098d8db26b5819043d4fe7c426b948) )
	ROM_LOAD( "gr 4.a7",   0x08000, 0x8000, CRC(952ade30) SHA1(f065368f645616d6d84be469ba45a9afa8788eda) )

	ROM_REGION( 0x000100, "pals", 0) /* PAL's */
	ROM_LOAD( "12 b.12b",  0x00000, 0x00001, NO_DUMP ) /* PAL16L8 */
	ROM_LOAD( "11b.11b.bin", 0x00000, 0x0002c, CRC(771eff5d) SHA1(4f008659f30bc9b0ec37e543ccafd7893e53d5a6) ) /* PAL10L8 */
	ROM_LOAD( "4 c.4c",    0x00000, 0x00001, NO_DUMP ) /* PAL16R6 */
	ROM_LOAD( "10 d.10d",  0x00000, 0x00001, NO_DUMP ) /* PAL16R6 */
	ROM_LOAD( "16 e.16e",  0x00000, 0x00001, NO_DUMP ) /* PAL16R6 */
	ROM_LOAD( "15 e.15e",  0x00000, 0x00001, NO_DUMP ) /* PAL16R6 */
	ROM_LOAD( "8 g.8g",    0x00000, 0x00001, NO_DUMP ) /* PAL16R6 */
ROM_END


ROM_START( rescraida )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "ab1-sa.a10",   0x00000, 0x8000, CRC(aa0a9f48) SHA1(b871573df0abdba20de78f655da846423191f0b4) )
	ROM_LOAD( "ab12-sa.a12",  0x08000, 0x8000, CRC(16d4da86) SHA1(240cfe8c5c4c005da9b9f370a04ed32fc245ec64) )
	ROM_LOAD( "cd8-sa.a16",   0x10000, 0x8000, CRC(9dfb50c2) SHA1(24280b48106cbcedeb6d7b10f951db906a123819) )
	ROM_LOAD( "cd12-sa.a18",  0x18000, 0x8000, CRC(18c62613) SHA1(a55b4b948805bdd5d1e8c8ff803826a7bbfa383e) )

	ROM_REGION( 0x10000, "gfx1", 0 )        /* up to 64k of sprites */
	ROM_LOAD( "gr0.a5",    0x00000, 0x8000, CRC(e0dfc133) SHA1(0b120b4410098d8db26b5819043d4fe7c426b948) )
	ROM_LOAD( "gr4.a7",    0x08000, 0x8000, CRC(952ade30) SHA1(f065368f645616d6d84be469ba45a9afa8788eda) )

	MOTHERBOARD_PALS
ROM_END


ROM_START( shrike )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code for the first CPU, plus 128k of banked ROMs */
	ROM_LOAD( "savgu35.bin", 0x00000, 0x2000, CRC(dd2230a0) SHA1(72be0e07d76ee1f170ab457ae62db87111758697) )
	ROM_LOAD( "savgu20.bin", 0x02000, 0x2000, CRC(3d140edc) SHA1(6c7e7dda7718e3f9644aad317da0b2277c2c1402) )
	ROM_LOAD( "savgu34.bin", 0x04000, 0x2000, CRC(779eca9d) SHA1(6783a62885ed129f436471a1c4a93ad898eb7965) )
	ROM_LOAD( "savgu19.bin", 0x06000, 0x2000, CRC(9ec89a80) SHA1(0a862d2a58adaf6726654a9a7b1b4b13e14d4d4b) )
	ROM_LOAD( "savgu33.bin", 0x08000, 0x2000, CRC(20596f48) SHA1(11827b86d184231d3d4f82496a0bb9ac7ac874dc) )
	ROM_LOAD( "savgu18.bin", 0x0a000, 0x2000, CRC(7abc3f14) SHA1(0a18be804927181c1bc86def595b22b3249fb6a0) )
	ROM_LOAD( "savgu32.bin", 0x0c000, 0x2000, CRC(807f0a3b) SHA1(b2df2422751b32a25258134f571a5f874ebc3a09) )
	ROM_LOAD( "savgu17.bin", 0x0e000, 0x2000, CRC(e0dbf6ad) SHA1(4618723116e2d83f9a775bb8b503faea995fda1b) )
	ROM_LOAD( "savgu21.bin", 0x1c000, 0x2000, CRC(c22b93e1) SHA1(15d3925abb3e7e928925f5781f228d1bc0dfe31c) )
	ROM_LOAD( "savgu36.bin", 0x1e000, 0x2000, CRC(28431c4a) SHA1(522df8224c559f51c36d2bc01c189b019fabc5eb) )

	ROM_REGION( 0x4000, "68k", 0 )      /* 16k for M68000 */
	ROM_LOAD16_BYTE( "savgu22.bin", 0x00000, 0x2000, CRC(c7787162) SHA1(52d8d148206c6ceb9c28ba747b301121a7790802) )
	ROM_LOAD16_BYTE( "savgu24.bin", 0x00001, 0x2000, CRC(a9105ca8) SHA1(1a94a052a4a8d221e1eafec0cd5b0ada6f1987f4) )

	ROM_REGION( 0x20000, "gfx1", 0 )        /* up to 128k of banked sprites */
	ROM_LOAD( "savgu8.bin",  0x00000, 0x2000, CRC(499a1d06) SHA1(0f3ed5ff345abb655f5a9f926ac3eb5dbca72a14) )
	ROM_LOAD( "savgu7.bin",  0x02000, 0x2000, CRC(ce0607f9) SHA1(0f6708d92e69a67b3eaba98f7ab4ad70eda3c854) )
	ROM_LOAD( "savgu6.bin",  0x04000, 0x2000, CRC(01d1b31e) SHA1(8061227f18f08e3b74bc6fc341ed4902c415db6c) )
	ROM_LOAD( "savgu5.bin",  0x06000, 0x2000, CRC(8bc6d101) SHA1(24f0b3ec3ed56b0496d07caa2475fca49a4a9b19) )
	ROM_LOAD( "savgu4.bin",  0x08000, 0x2000, CRC(72644753) SHA1(01bdb39d32df6d8cf69cbc9370033db46e18cb59) )
	ROM_LOAD( "savgu3.bin",  0x0a000, 0x2000, CRC(606a9cfd) SHA1(ce99a0e6d09580d35ec423177cdf41c35c7eecb7) )
	ROM_LOAD( "savgu2.bin",  0x0c000, 0x2000, CRC(69f600f6) SHA1(5b9545897f59b5049adc0fd910c7d65f38696d30) )
	ROM_LOAD( "savgu1.bin",  0x0e000, 0x2000, CRC(303b8e7b) SHA1(29055b621c68e93649eb0aa9cc9ecc43ac6f6eb8) )
	ROM_LOAD( "savgu16.bin", 0x10000, 0x2000, CRC(b8f60607) SHA1(4971db01a87bd80c23b7a0ab8aaa7c8300be4ec9) )
	ROM_LOAD( "savgu15.bin", 0x12000, 0x2000, CRC(6b332a5d) SHA1(58939cec237db1f741d24eb9f94488e3cf8700d2) )
	ROM_LOAD( "savgu14.bin", 0x14000, 0x2000, CRC(8d5117aa) SHA1(a82911219c49ff96e3c16acec7ef37406dae2be4) )
	ROM_LOAD( "savgu13.bin", 0x16000, 0x2000, CRC(d3ce645e) SHA1(4e775af7886d699675941f74e18be2d4dbd6f41b) )
	ROM_LOAD( "savgu12.bin", 0x18000, 0x2000, CRC(ccdfedb1) SHA1(b87e885df46e814626f46102f323ccd8396bcf8f) )
	ROM_LOAD( "savgu11.bin", 0x1a000, 0x2000, CRC(db11ff4c) SHA1(cd85486cd08ec4392421e9b94d380b81a575c811) )
	ROM_LOAD( "savgu10.bin", 0x1c000, 0x2000, CRC(6f3d9aa1) SHA1(7616dd016f5c8990b4972cf6edf758e27857aa1e) )

	MOTHERBOARD_PALS
ROM_END


/* Trivial Pursuit running on Maibesa PCB MAB-016 connected to a separate sound board (BSU) using a 14-pin connector with this pinout:
-Pin  1 : GND
-Pin  2 : S0
-Pin  3 : S1
-Pin  4 : S2
-Pin  5 : S3
-Pin  6 : S4
-Pin  7 : S5
-Pin  8 : S6
-Pin  9 : S7
-Pin 10 : STROBE
-Pin 11 : CHANNEL A
-Pin 12 : CHANNEL B
-Pin 13 : key (unused)
-Pin 14 : GND
*/
ROM_START( triviaes4 )
	ROM_REGION( 0x20000, "maincpu", 0 ) // all 27256, ROM loading order probably wrong
	ROM_LOAD( "tpe-35-volumen 4.ic35",  0x00000, 0x02000, CRC(8233c9af) SHA1(1853cbff5ff9b0bed4c12717ef705f6ee9679622) )
	ROM_CONTINUE(                       0x08000, 0x02000 )
	ROM_CONTINUE(                       0x10000, 0x02000 )
	ROM_CONTINUE(                       0x18000, 0x02000 )
	ROM_LOAD( "tpe-43-volumen 4.ic43",  0x02000, 0x02000, CRC(b404b163) SHA1(de30b47d08765a953b01cc3a6bdd95938af6b3d8) )
	ROM_CONTINUE(                       0x0a000, 0x02000 )
	ROM_CONTINUE(                       0x12000, 0x02000 )
	ROM_CONTINUE(                       0x1a000, 0x02000 )
	ROM_LOAD( "tpe-53-volumen 4.ic53",  0x04000, 0x02000, CRC(64e439d9) SHA1(f5fe3fa38997c1088c16361f8949648acc353c57) )
	ROM_CONTINUE(                       0x0c000, 0x02000 )
	ROM_CONTINUE(                       0x14000, 0x02000 )
	ROM_CONTINUE(                       0x1c000, 0x02000 )
	ROM_LOAD( "tpe-60-volumen 4.ic60",  0x06000, 0x02000, CRC(0773a142) SHA1(5654ece65be7714b25970f08ba876b9766d8ebb5) )
	ROM_CONTINUE(                       0x0e000, 0x02000 )
	ROM_CONTINUE(                       0x16000, 0x02000 )
	ROM_CONTINUE(                       0x1e000, 0x02000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // all 27256
	ROM_LOAD( "tpe-2a0.bin", 0x00000, 0x8000, CRC(9aefea1d) SHA1(2af60e19de37533a5ad111de4c6b58de41be92fd) )
	ROM_LOAD( "tpe-2b0.bin", 0x08000, 0x8000, CRC(ddcb4f6f) SHA1(f29c97ccc6711c433e104a8fc738ff390ba102e8) )

	ROM_REGION( 0x10000, "gfx1", 0 ) // all 27128, ROM loading order probably wrong
	ROM_LOAD( "tpe-8.ic8",   0x00000, 0x4000, CRC(0cde2421) SHA1(54604817a456f78110458e588c91c5029cf3189b) )
	ROM_LOAD( "tpe-33.ic33", 0x04000, 0x4000, CRC(552c2f4f) SHA1(a72d112c70b2c7ffbb8d51cc76124d507a543e2b) )
	ROM_LOAD( "tpe-57.ic57", 0x08000, 0x4000, CRC(90c8948a) SHA1(4b19bed71889756162dfe226eb531084603cf76f) )
	ROM_LOAD( "tpe-73.ic73", 0x0c000, 0x4000, CRC(b15bc90b) SHA1(dc84717178a177904eb3ddbeeaae5fc9b19b4a12) )

	ROM_REGION( 0x208, "motherbrd_pals", 0) /* Motherboard PAL's */
	ROM_LOAD( "pal16l8a.ic31", 0x000, 0x104, NO_DUMP ) /* PAL16L8 */
	ROM_LOAD( "pal16l8a.ic51", 0x104, 0x104, NO_DUMP ) /* PAL16L8 */
ROM_END


/* Trivial Pursuit running on Maibesa PCB MAB-016 connected to a separate sound board (BSU) using a 14-pin connector with this pinout:
-Pin  1 : GND
-Pin  2 : S0
-Pin  3 : S1
-Pin  4 : S2
-Pin  5 : S3
-Pin  6 : S4
-Pin  7 : S5
-Pin  8 : S6
-Pin  9 : S7
-Pin 10 : STROBE
-Pin 11 : CHANNEL A
-Pin 12 : CHANNEL B
-Pin 13 : key (unused)
-Pin 14 : GND
*/
ROM_START( triviaes5 )
	ROM_REGION( 0x20000, "maincpu", 0 ) // all 27256, ROM loading order probably wrong
	ROM_LOAD( "volu5-trivial.ic35",  0x00000, 0x02000, CRC(011c150e) SHA1(352cab76e91c6e8a2f06db5e0f67a05b47e5d0ae) )
	ROM_CONTINUE(                    0x08000, 0x02000 )
	ROM_CONTINUE(                    0x10000, 0x02000 )
	ROM_CONTINUE(                    0x18000, 0x02000 )
	ROM_LOAD( "volu5-trivial.ic43",  0x02000, 0x02000, CRC(8c13f091) SHA1(799a16a6fd68e9a3a7eafbe7aa02cb647e3161f5) )
	ROM_CONTINUE(                    0x0a000, 0x02000 )
	ROM_CONTINUE(                    0x12000, 0x02000 )
	ROM_CONTINUE(                    0x1a000, 0x02000 )
	ROM_LOAD( "volu5-trivial.ic53",  0x04000, 0x02000, CRC(09c43229) SHA1(8d0a7d1f335903ade2743cffb89a3949a5806218) )
	ROM_CONTINUE(                    0x0c000, 0x02000 )
	ROM_CONTINUE(                    0x14000, 0x02000 )
	ROM_CONTINUE(                    0x1c000, 0x02000 )
	ROM_LOAD( "volu5-trivial.ic60",  0x06000, 0x02000, CRC(30cc920d) SHA1(1adb5beb575d03d2c495db007529ff7abf5ee9f6) )
	ROM_CONTINUE(                    0x0e000, 0x02000 )
	ROM_CONTINUE(                    0x16000, 0x02000 )
	ROM_CONTINUE(                    0x1e000, 0x02000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // all 27256
	ROM_LOAD( "tpe-2a0.bin", 0x00000, 0x8000, CRC(9aefea1d) SHA1(2af60e19de37533a5ad111de4c6b58de41be92fd) )
	ROM_LOAD( "tpe-2b0.bin", 0x08000, 0x8000, CRC(ddcb4f6f) SHA1(f29c97ccc6711c433e104a8fc738ff390ba102e8) )

	ROM_REGION( 0x10000, "gfx1", 0 ) // all 27128, ROM loading order probably wrong
	ROM_LOAD( "tpe-8.ic8",   0x00000, 0x4000, CRC(0cde2421) SHA1(54604817a456f78110458e588c91c5029cf3189b) )
	ROM_LOAD( "tpe-33.ic33", 0x04000, 0x4000, CRC(552c2f4f) SHA1(a72d112c70b2c7ffbb8d51cc76124d507a543e2b) )
	ROM_LOAD( "tpe-57.ic57", 0x08000, 0x4000, CRC(90c8948a) SHA1(4b19bed71889756162dfe226eb531084603cf76f) )
	ROM_LOAD( "tpe-73.ic73", 0x0c000, 0x4000, CRC(b15bc90b) SHA1(dc84717178a177904eb3ddbeeaae5fc9b19b4a12) )

	ROM_REGION( 0x30c, "motherbrd_pals", 0) /* Motherboard PAL's */
	ROM_LOAD( "pal16l8a-tpe-v.ic31", 0x000, 0x104, NO_DUMP ) /* PAL16L8 */
	ROM_LOAD( "pal16l8a.ic61",       0x104, 0x104, NO_DUMP ) /* PAL16L8 */
	ROM_LOAD( "pal16l8a.ic96",       0x208, 0x104, NO_DUMP ) /* PAL16L8 */
ROM_END

/*************************************
 *
 *  Driver initialization
 *
 *************************************/

#define EXPAND_ALL      0x00
#define EXPAND_NONE     0x3f
#define SWAP_HALVES     0x80

void balsente_state::expand_roms(uint8_t cd_rom_mask)
{
	/* load AB bank data from 0x00000-0x10000 */
	/* load CD bank data from 0x10000-0x1e000 */
	/* load EF           from 0x1e000-0x20000 */

	uint8_t *rom = m_mainrom->base();
	uint32_t len = m_mainrom->bytes();

	int numbanks = (len > 0x20000) ? 16 : 8;
	uint32_t bxor = (cd_rom_mask & SWAP_HALVES) ? 0x02000 : 0;

	for (int b = 0; b < numbanks; b += 8)
	{
		uint32_t base = 0x00000 + 0x4000 * b;
		uint8_t *ab_base = &rom[base + 0x00000];
		uint8_t *cd_base = &rom[base + 0x10000];
		uint8_t *cd_common = &rom[base + (0x1c000 ^ bxor)];
		uint8_t *ef_common = &rom[base + (0x1e000 ^ bxor)];

		m_bankef->configure_entry(b / 8, ef_common);

		m_bankcd->configure_entry(b + 7, cd_common);
		m_bankab->configure_entry(b + 7, &ab_base[0xe000 ^ bxor]);

		m_bankcd->configure_entry(b + 6, cd_common);
		m_bankab->configure_entry(b + 6, &ab_base[0xc000 ^ bxor]);

		m_bankcd->configure_entry(b + 5, BIT(cd_rom_mask, 5) ? &cd_base[0xa000 ^ bxor] : cd_common);
		m_bankab->configure_entry(b + 5, &ab_base[0xa000 ^ bxor]);

		m_bankcd->configure_entry(b + 4, BIT(cd_rom_mask, 4) ? &cd_base[0x8000 ^ bxor] : cd_common);
		m_bankab->configure_entry(b + 4, &ab_base[0x8000 ^ bxor]);

		m_bankcd->configure_entry(b + 3, BIT(cd_rom_mask, 3) ? &cd_base[0x6000 ^ bxor] : cd_common);
		m_bankab->configure_entry(b + 3, &ab_base[0x6000 ^ bxor]);

		m_bankcd->configure_entry(b + 2, BIT(cd_rom_mask, 2) ? &cd_base[0x4000 ^ bxor] : cd_common);
		m_bankab->configure_entry(b + 2, &ab_base[0x4000 ^ bxor]);

		m_bankcd->configure_entry(b + 1, BIT(cd_rom_mask, 1) ? &cd_base[0x2000 ^ bxor] : cd_common);
		m_bankab->configure_entry(b + 1, &ab_base[0x2000 ^ bxor]);

		m_bankcd->configure_entry(b + 0, BIT(cd_rom_mask, 0) ? &cd_base[0x0000 ^ bxor] : cd_common);
		m_bankab->configure_entry(b + 0, &ab_base[0x0000 ^ bxor]);
	}
}

inline void balsente_state::config_shooter_adc(uint8_t shooter, uint8_t adc_shift)
{
	m_shooter = shooter;
	m_adc_shift = adc_shift;
}

void balsente_state::init_sentetst()  { expand_roms(EXPAND_ALL);  config_shooter_adc(false, 0 /* noanalog */); }
void balsente_state::init_cshift()    { expand_roms(EXPAND_ALL);  config_shooter_adc(false, 0 /* noanalog */); }
void balsente_state::init_gghost()    { expand_roms(EXPAND_ALL);  config_shooter_adc(false, 1); }
void balsente_state::init_hattrick()  { expand_roms(EXPAND_ALL);  config_shooter_adc(false, 0 /* noanalog */); }

void balsente_state::init_otwalls()   { expand_roms(EXPAND_ALL);  config_shooter_adc(false, 0); }
void balsente_state::init_snakepit()  { expand_roms(EXPAND_ALL);  config_shooter_adc(false, 1); }
void balsente_state::init_snakjack()  { expand_roms(EXPAND_ALL);  config_shooter_adc(false, 1); }
void balsente_state::init_stocker()   { expand_roms(EXPAND_ALL);  config_shooter_adc(false, 0); }
void balsente_state::init_triviag1()  { expand_roms(EXPAND_ALL);  config_shooter_adc(false, 0 /* noanalog */); }
void balsente_state::init_triviag2()  { expand_roms(EXPAND_NONE); config_shooter_adc(false, 0 /* noanalog */); }
void balsente_state::init_triviaes()  { expand_roms(EXPAND_NONE | SWAP_HALVES); config_shooter_adc(false, 0 /* noanalog */); }
void balsente_state::init_triviaes2() { expand_roms(EXPAND_NONE); config_shooter_adc(false, 0 /* noanalog */); }
void balsente_state::init_gimeabrk()  { expand_roms(EXPAND_ALL);  config_shooter_adc(false, 1); }
void balsente_state::init_minigolf()  { expand_roms(EXPAND_NONE); config_shooter_adc(false, 2); }
void balsente_state::init_minigolf2() { expand_roms(0x0c);        config_shooter_adc(false, 2); }
void balsente_state::init_toggle()    { expand_roms(EXPAND_ALL);  config_shooter_adc(false, 0 /* noanalog */); }
void balsente_state::init_nametune()  { expand_roms(EXPAND_NONE | SWAP_HALVES); config_shooter_adc(false, 0 /* noanalog */); }
void balsente_state::init_nstocker()  { expand_roms(EXPAND_NONE | SWAP_HALVES); config_shooter_adc(true, 1); }
void balsente_state::init_sfootbal()  { expand_roms(EXPAND_ALL  | SWAP_HALVES); config_shooter_adc(false, 0); }
void balsente_state::init_spiker()    { expand_roms(EXPAND_ALL  | SWAP_HALVES); config_shooter_adc(false, 1); }
void balsente_state::init_stompin()   { expand_roms(0x0c | SWAP_HALVES); config_shooter_adc(false, 32); }
void balsente_state::init_rescraid()  { expand_roms(EXPAND_NONE); config_shooter_adc(false, 0 /* noanalog */); }
void balsente_state::init_grudge()    { expand_roms(EXPAND_NONE); config_shooter_adc(false, 0); }
void balsente_state::init_shrike()    { expand_roms(EXPAND_ALL);  config_shooter_adc(false, 32); }



/*************************************
 *
 *  Game drivers
 *
 *************************************/

/* Board: Unknown */
GAME( 1984, sentetst,  0,        balsente, sentetst, balsente_state, init_sentetst, ROT0, "Bally/Sente",  "Sente Diagnostic Cartridge", MACHINE_SUPPORTS_SAVE )

/* Board: 006-8003-01-0D Rev D */
GAME( 1984, cshift,    0,        balsente, cshift,   balsente_state, init_cshift,   ROT0, "Bally/Sente",  "Chicken Shift (11/23/84)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, hattrick,  0,        balsente, hattrick, balsente_state, init_hattrick, ROT0, "Bally/Sente",  "Hat Trick (11/12/84)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, gghost,    0,        balsente, gghost,   balsente_state, init_gghost,   ROT0, "Bally/Sente",  "Goalie Ghost", MACHINE_SUPPORTS_SAVE )
GAME( 1984, otwalls,   0,        balsente, otwalls,  balsente_state, init_otwalls,  ROT0, "Bally/Sente",  "Off the Wall (Sente) (10/16/84)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, snakepit,  0,        balsente, sentetst, balsente_state, init_snakepit, ROT0, "Bally/Sente",  "Snake Pit", MACHINE_SUPPORTS_SAVE )
GAME( 1984, snakepita, snakepit, balsente, sentetst, balsente_state, init_snakepit, ROT0, "Sente Technologies Inc.", "Snake Pit (9/14/84)", MACHINE_SUPPORTS_SAVE ) // 1984, even though titlescreen says 1983
GAME( 1984, triviag1,  0,        balsente, triviag1, balsente_state, init_triviag1, ROT0, "Bally/Sente",  "Trivial Pursuit (Think Tank - Genus Edition) (2/12/85)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, triviag1a, triviag1, balsente, triviag1, balsente_state, init_triviag1, ROT0, "Bally/Sente",  "Trivial Pursuit (Think Tank - Genus Edition) (12/14/84)", MACHINE_SUPPORTS_SAVE )

/* Board: Unknown (From a picture on eBay Snacks'n Jaxson does not match any documented types here.) */
GAME( 1984, snakjack,  0,        balsente, snakjack, balsente_state, init_snakjack,  ROT0, "Bally/Sente",  "Snacks'n Jaxson", MACHINE_SUPPORTS_SAVE )

/* Board: 006-8025-01-0B Rev B */
GAMEL(1984, stocker,   0,        balsente, stocker,  balsente_state, init_stocker,   ROT0, "Bally/Sente",  "Stocker (3/19/85)", MACHINE_SUPPORTS_SAVE, layout_stocker ) // date from ROM chips
GAME( 1984, triviabb,  0,        balsente, triviag1, balsente_state, init_triviag2,  ROT0, "Bally/Sente",  "Trivial Pursuit (Baby Boomer Edition) (3/20/85)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, triviag2,  0,        balsente, triviag1, balsente_state, init_triviag2,  ROT0, "Bally/Sente",  "Trivial Pursuit (Genus II Edition) (3/22/85)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, triviayp,  0,        balsente, triviag1, balsente_state, init_triviag2,  ROT0, "Bally/Sente",  "Trivial Pursuit (Young Players Edition) (3/29/85)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, triviasp,  0,        balsente, triviag1, balsente_state, init_triviag2,  ROT0, "Bally/Sente",  "Trivial Pursuit (All Star Sports Edition)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, gimeabrk,  0,        balsente, gimeabrk, balsente_state, init_gimeabrk,  ROT0, "Bally/Sente",  "Gimme A Break (7/7/85)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, minigolf,  0,        balsente, minigolf, balsente_state, init_minigolf,  ROT0, "Bally/Sente",  "Mini Golf (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, minigolfa, minigolf, balsente, minigolf, balsente_state, init_minigolf2, ROT0, "Bally/Sente",  "Mini Golf (11/25/85)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, minigolfb, minigolf, balsente, minigolf2,balsente_state, init_minigolf2, ROT0, "Bally/Sente",  "Mini Golf (10/8/85)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, stompina,  stompin,  balsente, stompina, balsente_state, init_shrike,    ROT0, "Bally/Sente",  "Stompin' (prototype?)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, teamht,    0,        teamht,   teamht,   balsente_state, init_hattrick,  ROT0, "Bally/Sente",  "Team Hat Trick (11/16/84)", MACHINE_SUPPORTS_SAVE ) // ROM chips dated 11/16/84
GAME( 1987, grudge,    0,        grudge,   grudge,   balsente_state, init_grudge,    ROT0, "Bally Midway", "Grudge Match (v00.90, prototype)", MACHINE_SUPPORTS_SAVE ) // only the PCB was found
GAME( 1987, grudgei,   grudge,   grudge,   grudge,   balsente_state, init_grudge,    ROT0, "Bally Midway", "Grudge Match (v00.90, Italy, location test?)", MACHINE_SUPPORTS_SAVE ) // PCB came from a dedicated cabinet complete with artwork
GAME( 1987, grudgep,   grudge,   grudge,   grudgep,  balsente_state, init_grudge,    ROT0, "Bally Midway", "Grudge Match (v00.80, prototype)", MACHINE_SUPPORTS_SAVE )

/* Board: Unknown  */
GAME( 1987, triviaes,  0,        balsente, triviaes, balsente_state, init_triviaes,  ROT0, "Bally/Sente (Maibesa license)",  "Trivial Pursuit (Volumen III, Spanish, Maibesa license)", MACHINE_SUPPORTS_SAVE ) // Genus Edition?
GAME( 1985, toggle,    0,        balsente, toggle,   balsente_state, init_toggle,    ROT0, "Bally/Sente",  "Toggle (prototype)", MACHINE_SUPPORTS_SAVE )

/* Board: 007-8001-01-0C Rev C1 */
GAME( 1987, triviaes2, triviaes, balsente, triviaes, balsente_state, init_triviaes2, ROT0, "Bally/Sente (Maibesa license)",  "Trivial Pursuit (Volumen II, Spanish, Maibesa license)", MACHINE_SUPPORTS_SAVE ) // "Jovenes Carrozas" Edition?

/* Board: 006-8027-01-0B Rev B */
GAME( 1986, nstocker,  0,        st1002,   nstocker, balsente_state, init_nstocker,  ROT0, "Bally/Sente",  "Night Stocker (10/6/86)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, nstockera, nstocker, st1002,   nstocker, balsente_state, init_nstocker,  ROT0, "Bally/Sente",  "Night Stocker (8/27/86)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, sfootbal,  0,        st1002,   sfootbal, balsente_state, init_sfootbal,  ROT0, "Bally/Sente",  "Street Football (11/12/86)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, spiker,    0,        spiker,   spiker,   balsente_state, init_spiker,    ROT0, "Bally/Sente",  "Spiker (6/9/86)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, spikera,   spiker,   spiker,   spiker,   balsente_state, init_spiker,    ROT0, "Bally/Sente",  "Spiker (5/5/86)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, spikerb,   spiker,   spiker,   spiker,   balsente_state, init_spiker,    ROT0, "Bally/Sente",  "Spiker (earliest?)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, stompin,   0,        st1002,   stompin,  balsente_state, init_stompin,   ROT0, "Bally/Sente",  "Stompin' (4/4/86)", MACHINE_SUPPORTS_SAVE )

/* Board: 006-8030-01-0A Rev A */
GAME( 1986, nametune,  0,        st1002,   nametune, balsente_state, init_nametune,  ROT0, "Bally/Sente",  "Name That Tune (Bally, 3/31/86)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, nametunea, nametune, st1002,   nametune, balsente_state, init_nametune,  ROT0, "Bally/Sente",  "Name That Tune (Bally, 3/23/86)", MACHINE_SUPPORTS_SAVE )

/* Board: A084-91889-A000 (Not a cartridge, but dedicated board) */
GAME( 1987, rescraid,  0,        rescraid, rescraid, balsente_state, init_rescraid,  ROT0, "Bally Midway", "Rescue Raider (5/11/87) (non-cartridge)", MACHINE_SUPPORTS_SAVE )

/* Board: Unknown */
GAME( 1986, shrike,    0,        shrike,   shrike,   balsente_state, init_shrike,    ROT0, "Bally/Sente",  "Shrike Avenger (prototype)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, rescraida, rescraid, rescraid, rescraid, balsente_state, init_rescraid,  ROT0, "Bally Midway", "Rescue Raider (stand-alone)", MACHINE_SUPPORTS_SAVE )

/* Trivial Pursuit running on Maibesa hardware (with Bally/Sente license) */
GAME( 1988, triviaes4, 0,        triviamb, triviaes, balsente_state, init_triviaes2, ROT0, "Bally/Sente (Maibesa license)", "Trivial Pursuit (Volumen IV, Spanish, Maibesa hardware)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // different (bootleg?) hardware. maincpu ROMs structure clearly similar to Trivial Pursuit games
GAME( 19??, triviaes5, 0,        triviamb, triviaes, balsente_state, init_triviaes2, ROT0, "Bally/Sente (Maibesa license)", "Trivial Pursuit (Volumen V, Spanish, Maibesa hardware)",  MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // different (bootleg?) hardware. maincpu ROMs structure clearly similar to Trivial Pursuit games
