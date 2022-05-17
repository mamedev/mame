// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
    Laser Battle / Lazarian (c) 1981 Zaccaria
    Cat and Mouse           (c) 1982 Zaccaria

    original driver by Pierpaolo Prazzoli

    The two games have near identical game/video boards hardware, but
    completely different sound boards.  Laser Battle/Lazarian have a
    dumb sound board with TMS organ and CSG chips driven directly by the
    game program.  Cat and Mouse uses an intelligent sound board with
    its own CPU that plays melodies on a pair of AY-3-8910 PSGs.

    The video hardware uses a PLA to mix TTL-generated background,
    effect and sprite layers with the S2636 PVI outputs.  The collision
    detection and interrupt generation capabilities of the S2636 PVIs
    are not used.

    Game board supports two different sound board interfaces: 16-bit
    unidirectional bus on J3 and 8-bit bidirectional bus on J7.
    Lazarian uses only the 16-bit unidirectional interface.  The 16-bit
    interface is controlled by latches at I/O addresses 2 (bits 1-8)
    and 7 (bits 9-16).  The 8-bit interface is read at I/O address 0 and
    written at I/O address 3.  The sound board controls data direction
    on J7 and when input from sound board to game board is latched.

    Both Laser Battle/Lazarian and Cat and Mouse use the unidirectional
    interface on J3.  It seems there are no games that actually use the
    bidirectional interface on J7.

    The game board appears to have had some last-minute design changes
    that aren't reflected in the Midway schematics, for example the last
    program ROM being double the size of the others.  There are also
    some errors in the schematic like missing connections and incorrect
    logic gate symbols.

    Laser Battle/Lazarian notes:
    * Manuals clearly indicate the controls to fire in four directions
      are four buttons arranged in a diamond, not a four-way joystick
    * Cocktail cabinet has an additional "image commutation board"
      consuming the screen flip output, presumably flipping the image by
      reversing the deflection coil connections
    * Player 2 inputs are only used in cocktail mode
    * Tilt input resets Laser Battle, but just causes loss of one credit
      in Lazarian
    * Service coin 1 input grants two credits the first time it's
      pushed, but remembers this and won't grant credits again unless
      unless you trigger the tilt input
    * Laser Battle has a credit limit of 61 while Lazarian has a much
      lower credit limit of 9
    * Laser Battle is far less forgiving, sending you back to the start
      of an area on dying and not giving continues

    Cat and Mouse notes:
    * This game is designed to work with two-way joysticks - up/down
      directions are ignored and not shown on wiring diagram
    * The input lines used for the fire buttons are chosen so that if
      you plug it in to a Laser Battle control panel, the Fire Up button
      will be used
    * Tilt input causes loss of one credit
    * Service coin 1 input grants two credits the first time it's
      pushed, but remembers this and won't grant credits again unless
      unless you trigger the tilt input
    * The sprite ROM is twice the size as Laser Battle with the bank
      selected using bit 9 of the 16-bit sound interface (there's a wire
      making this connection visible on the component side of the PCB)
    * At least some boards have IC13I pins 8, 9, 10 and 11 bent out of
      the socket, tied together, and pulled high via a 4k7 resistor,
      which quantises the shell/area effect 2 to four-pixel boundaries
      (implemented as m_eff2_mask) - would be good to see whether this
      mod is present on all boards
    * If demo sounds are enabled (using DIP switches), background music
      is played every sixth time through the attract loop
    * Sound board emulation is based on tracing the program and guessing
      what's connected where - we really need someone to trace out the
      1b11107 sound board if we want to get this right

    TODO:
    - work out where all the magic layer offsets come from
    - sound in laserbat (with schematics) and in catnmous
*/

#include "emu.h"
#include "includes/laserbat.h"

#include "cpu/m6800/m6800.h"

#include "machine/clock.h"

#include "speaker.h"


void laserbat_state_base::ct_io_w(uint8_t data)
{
	/*
	    Uses a hex buffer, so bits 6 and 7 are not physically present.

	    Bits 0-2 are open collector outputs with a diode connected to
	    the return line to suppress solenoid switching transients.
	    These are used to drive coin counters.

	    Bit 3 is an open collector output with a 2k2 pull-up resistor.
	    It is used to drive the "image commutation board" used to flip
	    the screen for player 2 in cocktail configuration.  Note that
	    this output is asserted when player 2 is active even in upright
	    configuration, it's only supposed to be connected in a cocktail
	    cabinet.

	    Bits 4-5 feed the input row select decoder that switches between
	    ROW0, ROW1, SW1 and SW2 (ROW2 is selected using a bit in the
	    video effects register, just to be confusing).

	    +-----+-----------------------------+--------------------+--------------+
	    | bit | output                      | laserbat/catnmous  | lazarian     |
	    +-----+-----------------------------+--------------------+--------------+
	    |  0  | J2-3 solenoid driver        | 1*Credits          | 1*Coin C     |
	    |     |                             |                    |              |
	    |  1  | J2-8 solenoid driver        |  5*Coin A          | 1*Coin A     |
	    |     |                             | 10*Coin B          |              |
	    |     |                             |  1*Coin C          |              |
	    |     |                             |                    |              |
	    |  2  | J2-6 solenoid driver        |                    | 1*Coin B     |
	    |     |                             |                    |              |
	    |  3  | J3-4 open collector output  | Screen flip        | Screen flip  |
	    |     |                             |                    |              |
	    |  4  | input row select A          |                    |              |
	    |     |                             |                    |              |
	    |  5  | input row select B          |                    |              |
	    +-----+-----------------------------+--------------------+--------------+
	*/

	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));
	machine().bookkeeping().coin_counter_w(2, BIT(data, 2));
	flip_screen_set((bool(data & 0x08) && !bool(m_row1->read() & 0x10)) ? 1 : 0);
	m_input_mux = (data >> 4) & 0x03;

//  popmessage("ct io: %02X", data);
}

uint8_t laserbat_state_base::rrowx_r()
{
	return (m_mpx_p_1_2 ? m_row2 : m_mux_ports[m_input_mux])->read();
}

/*

    2716.14L address lines are connected as follows:

    A0  4H
    A1  8H
    A2  16H
    A3  1V
    A4  2V
    A5  4V
    A6  8V
    A7  16V
    A8  SHPA
    A9  SHPB
    A10 SHPC

    The output of the 2716.14L is sent to the 82S100.10M
    through a parallel-to-serial shift register that is clocked
    on (1H && 2H). The serial data sent is as follows:

    NAV0    D6, D4, D2, D0, 0, 0, 0, 0
    NAV1    D7, D5, D3, D1, 0, 0, 0, 0

*/

void laserbat_state_base::laserbat_map(address_map &map)
{
	map.unmap_value_high();

	map(0x0000, 0x13ff).rom();
	map(0x2000, 0x33ff).rom();
	map(0x3800, 0x3bff).rom();
	map(0x4000, 0x53ff).rom();
	map(0x6000, 0x73ff).rom();
	map(0x7800, 0x7bff).rom();

	map(0x1400, 0x14ff).mirror(0x6000).nopw();
	map(0x1500, 0x15ff).mirror(0x6000).rw(m_pvi[0], FUNC(s2636_device::read_data), FUNC(s2636_device::write_data));
	map(0x1600, 0x16ff).mirror(0x6000).rw(m_pvi[1], FUNC(s2636_device::read_data), FUNC(s2636_device::write_data));
	map(0x1700, 0x17ff).mirror(0x6000).rw(m_pvi[2], FUNC(s2636_device::read_data), FUNC(s2636_device::write_data));
	map(0x1800, 0x1bff).mirror(0x6000).w(FUNC(laserbat_state_base::videoram_w));
	map(0x1c00, 0x1fff).mirror(0x6000).ram();
}

void laserbat_state_base::laserbat_io_map(address_map &map)
{
	map(0x00, 0x00).r(FUNC(laserbat_state_base::rhsc_r)).w(FUNC(laserbat_state_base::cnt_eff_w));
	map(0x01, 0x01) /* RBALL */ .w(FUNC(laserbat_state_base::cnt_nav_w));
	map(0x02, 0x02).r(FUNC(laserbat_state_base::rrowx_r)).w(FUNC(laserbat_state_base::csound1_w));
	map(0x03, 0x03).w(FUNC(laserbat_state_base::whsc_w));
	map(0x04, 0x04).w(FUNC(laserbat_state_base::wcoh_w));
	map(0x05, 0x05).w(FUNC(laserbat_state_base::wcov_w));
	map(0x06, 0x06).w(FUNC(laserbat_state_base::ct_io_w));
	map(0x07, 0x07).w(FUNC(laserbat_state_base::csound2_w));
}


static INPUT_PORTS_START( laserbat_base )
	PORT_START("ROW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_START("ROW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_CONFNAME( 0x10, 0x10, DEF_STR(Cabinet) ) // sense line on wiring harness
	PORT_CONFSETTING(    0x10, DEF_STR(Upright) )
	PORT_CONFSETTING(    0x00, DEF_STR(Cocktail) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_TILT )

	PORT_START("ROW2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)

	PORT_START("SW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR(Coin_A) )         PORT_DIPLOCATION("SW-1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR(1C_1C) )
	PORT_DIPSETTING(    0x01, DEF_STR(1C_2C) )
	PORT_DIPSETTING(    0x02, DEF_STR(1C_3C) )
	PORT_DIPSETTING(    0x03, DEF_STR(1C_5C) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR(Coin_B) )         PORT_DIPLOCATION("SW-1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR(1C_2C) )
	PORT_DIPSETTING(    0x04, DEF_STR(1C_3C) )
	PORT_DIPSETTING(    0x08, DEF_STR(1C_5C) )
	PORT_DIPSETTING(    0x0c, DEF_STR(1C_7C) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR(Unknown) )        PORT_DIPLOCATION("SW-1:5")
	PORT_DIPSETTING(    0x10, DEF_STR(Off) )
	PORT_DIPSETTING(    0x00, DEF_STR(On) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR(Unknown) )        PORT_DIPLOCATION("SW-1:6")
	PORT_DIPSETTING(    0x20, DEF_STR(Off) )
	PORT_DIPSETTING(    0x00, DEF_STR(On) )
	PORT_DIPNAME( 0x40, 0x00, "Infinite Lives" )        PORT_DIPLOCATION("SW-1:7")
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )
	PORT_DIPSETTING(    0x40, DEF_STR(On) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR(Unknown) )        PORT_DIPLOCATION("SW-1:8")
	PORT_DIPSETTING(    0x80, DEF_STR(Off) )
	PORT_DIPSETTING(    0x00, DEF_STR(On) )

	PORT_START("SW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR(Unknown) )        PORT_DIPLOCATION("SW-2:1")
	PORT_DIPSETTING(    0x01, DEF_STR(Off) )
	PORT_DIPSETTING(    0x00, DEF_STR(On) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR(Unknown) )        PORT_DIPLOCATION("SW-2:2")
	PORT_DIPSETTING(    0x02, DEF_STR(Off) )
	PORT_DIPSETTING(    0x00, DEF_STR(On) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR(Unknown) )        PORT_DIPLOCATION("SW-2:3")
	PORT_DIPSETTING(    0x04, DEF_STR(Off) )
	PORT_DIPSETTING(    0x00, DEF_STR(On) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR(Difficulty) )     PORT_DIPLOCATION("SW-2:4,5")
	PORT_DIPSETTING(    0x00, DEF_STR(Easy) )
	PORT_DIPSETTING(    0x08, DEF_STR(Medium) )
	PORT_DIPSETTING(    0x10, DEF_STR(Difficult) )
	PORT_DIPSETTING(    0x18, DEF_STR(Very_Difficult) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR(Unknown) )        PORT_DIPLOCATION("SW-2:6")
	PORT_DIPSETTING(    0x20, DEF_STR(Off) )
	PORT_DIPSETTING(    0x00, DEF_STR(On) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR(Unknown) )        PORT_DIPLOCATION("SW-2:7")
	PORT_DIPSETTING(    0x40, DEF_STR(Off) )
	PORT_DIPSETTING(    0x00, DEF_STR(On) )
	PORT_DIPNAME( 0x80, 0x80, "Coin C" )                PORT_DIPLOCATION("SW-2:8")
	PORT_DIPSETTING(    0x00, DEF_STR(2C_1C) )
	PORT_DIPSETTING(    0x80, DEF_STR(1C_1C) )
INPUT_PORTS_END


static INPUT_PORTS_START( laserbat )
	PORT_INCLUDE(laserbat_base)

	PORT_MODIFY("SW1")
	PORT_DIPNAME( 0x30, 0x10, DEF_STR(Lives) )          PORT_DIPLOCATION("SW-1:5,6")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "6" )
	PORT_DIPNAME( 0x80, 0x80, "Collision Detection" )   PORT_DIPLOCATION("SW-1:8")
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )
	PORT_DIPSETTING(    0x80, DEF_STR(On) )

	PORT_MODIFY("SW2")
	PORT_DIPNAME( 0x60, 0x40, DEF_STR(Bonus_Life) )     PORT_DIPLOCATION("SW-2:6,7")
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )
	PORT_DIPSETTING(    0x20, "10,000" )
	PORT_DIPSETTING(    0x40, "14,000" )
	PORT_DIPSETTING(    0x60, "18,000" )
INPUT_PORTS_END

static INPUT_PORTS_START( lazarian )
	PORT_INCLUDE(laserbat)

	PORT_MODIFY("SW1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR(Coin_A) )         PORT_DIPLOCATION("SW-1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR(2C_1C) )
	PORT_DIPSETTING(    0x01, DEF_STR(1C_1C) )
	PORT_DIPSETTING(    0x02, DEF_STR(1C_2C) )
	PORT_DIPSETTING(    0x03, DEF_STR(1C_3C) )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR(Lives) )          PORT_DIPLOCATION("SW-1:5,6")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x30, "5" )
	PORT_DIPNAME( 0x40, 0x00, "Calibration Display" )   PORT_DIPLOCATION("SW-1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )

	PORT_MODIFY("SW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR(Unused) )         PORT_DIPLOCATION("SW-2:1") // manual says not used
	PORT_DIPSETTING(    0x01, DEF_STR(Off) )
	PORT_DIPSETTING(    0x00, DEF_STR(On) )
	PORT_DIPNAME( 0x02, 0x02, "Firing" )                PORT_DIPLOCATION("SW-2:2")
	PORT_DIPSETTING(    0x02, "Rapid" )
	PORT_DIPSETTING(    0x00, DEF_STR(Normal) )
	PORT_DIPNAME( 0x04, 0x00, "Freeze" )                PORT_DIPLOCATION("SW-2:3")
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )
	PORT_DIPSETTING(    0x04, DEF_STR(On) )
INPUT_PORTS_END

static INPUT_PORTS_START( catnmous )
	PORT_INCLUDE(laserbat_base)

	PORT_MODIFY("ROW0")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("ROW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("SW1")
	PORT_DIPNAME( 0x30, 0x10, DEF_STR(Lives) )          PORT_DIPLOCATION("SW-1:5,6")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x30, "5" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR(Demo_Sounds) )    PORT_DIPLOCATION("SW-1:8")
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )
	PORT_DIPSETTING(    0x80, DEF_STR(On) )

	PORT_MODIFY("SW2")
	PORT_DIPNAME( 0x01, 0x01, "Free Play" )             PORT_DIPLOCATION("SW-2:1") // taken from manual, assuming poor translation
	PORT_DIPSETTING(    0x01, "Win Play" )
	PORT_DIPSETTING(    0x00, "No Win Play" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR(Unused) )         PORT_DIPLOCATION("SW-2:2") // manual says not used
	PORT_DIPSETTING(    0x02, DEF_STR(Off) )
	PORT_DIPSETTING(    0x00, DEF_STR(On) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR(Unused) )         PORT_DIPLOCATION("SW-2:3") // manual says not used
	PORT_DIPSETTING(    0x04, DEF_STR(Off) )
	PORT_DIPSETTING(    0x00, DEF_STR(On) )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR(Bonus_Life) )     PORT_DIPLOCATION("SW-2:6,7")
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )
	PORT_DIPSETTING(    0x20, "20,000" )
	PORT_DIPSETTING(    0x40, "24,000" )
	PORT_DIPSETTING(    0x60, "28,000" )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout sprites_layout =
{
	32,32,
	RGN_FRAC(1,1),
	2,
	{ 0, 1 },
	{    0, 2, 4, 6, 8,10,12,14,16,18,20,22,24,26,28,30,
		32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62
	},
	{    0*32, 2*32, 4*32, 6*32, 8*32,10*32,12*32,14*32,
		16*32,18*32,20*32,22*32,24*32,26*32,28*32,30*32,
		32*32,34*32,36*32,38*32,40*32,42*32,44*32,46*32,
		48*32,50*32,52*32,54*32,56*32,58*32,60*32,62*32
	},
	32*32*2
};

static GFXDECODE_START( gfx_laserbat )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout,       0, 256 ) // ROM chars
	GFXDECODE_ENTRY( "gfx2", 0x0000, sprites_layout,   0,   8 ) // sprites
GFXDECODE_END


INTERRUPT_GEN_MEMBER(laserbat_state_base::laserbat_interrupt)
{
	m_maincpu->set_input_line(0, ASSERT_LINE);
}

void laserbat_state_base::machine_start()
{
	// start rendering scanlines
	m_screen->register_screen_bitmap(m_bitmap);
	m_scanline_timer = timer_alloc(FUNC(laserbat_state_base::video_line), this);
	m_scanline_timer->adjust(m_screen->time_until_pos(1, 0));

	save_item(NAME(m_gfx2_base));

	save_item(NAME(m_input_mux));
	save_item(NAME(m_mpx_p_1_2));

	save_item(NAME(m_bg_ram));
	save_item(NAME(m_eff_ram));
	save_item(NAME(m_mpx_bkeff));

	save_item(NAME(m_nave));
	save_item(NAME(m_clr_lum));
	save_item(NAME(m_shp));
	save_item(NAME(m_wcoh));
	save_item(NAME(m_wcov));

	save_item(NAME(m_abeff1));
	save_item(NAME(m_abeff2));
	save_item(NAME(m_mpx_eff2_sh));
	save_item(NAME(m_coleff));
	save_item(NAME(m_neg1));
	save_item(NAME(m_neg2));

	save_item(NAME(m_rhsc));
	save_item(NAME(m_whsc));
	save_item(NAME(m_csound1));
	save_item(NAME(m_csound2));
}

void laserbat_state::machine_start()
{
	laserbat_state_base::machine_start();

	save_item(NAME(m_keys));
}

void laserbat_state_base::laserbat_base(machine_config &config)
{
	// basic machine hardware
	S2650(config, m_maincpu, XTAL(14'318'181)/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &laserbat_state_base::laserbat_map);
	m_maincpu->set_addrmap(AS_IO, &laserbat_state_base::laserbat_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(laserbat_state_base::laserbat_interrupt));
	m_maincpu->sense_handler().set(m_screen, FUNC(screen_device::vblank));
	m_maincpu->intack_handler().set([this]() { m_maincpu->set_input_line(0, CLEAR_LINE); return 0x0a; });

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(14'318'181), 227*4, 43*4-1, 227*4-1, 312, 8, 255);
	m_screen->set_screen_update(FUNC(laserbat_state_base::screen_update_laserbat));
	m_screen->set_palette(m_palette);

	PLS100(config, m_gfxmix);

	S2636(config, m_pvi[0], XTAL(14'318'181)/3);
	m_pvi[0]->set_offsets(-8, -16);
	m_pvi[0]->set_divider(3);

	S2636(config, m_pvi[1], XTAL(14'318'181)/3);
	m_pvi[1]->set_offsets(-8, -16);
	m_pvi[1]->set_divider(3);

	S2636(config, m_pvi[2], XTAL(14'318'181)/3);
	m_pvi[2]->set_offsets(-8, -16);
	m_pvi[2]->set_divider(3);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_laserbat);
}

void laserbat_state::laserbat(machine_config &config)
{
	laserbat_base(config);

	// video hardware
	PALETTE(config, m_palette, FUNC(laserbat_state::laserbat_palette), 256);

	// sound board devices
	SPEAKER(config, "speaker").front_center();

	SN76477(config, m_csg); // audio output not used
	m_csg->set_noise_params(RES_K(47), RES_K(270), CAP_P(1000)); // R21, switchable R30/R23/R24/R25/R29/R28/R27/R26, C21
	m_csg->set_decay_res(RES_INF);                  // NC
	m_csg->set_attack_params(0, RES_INF);           // NC, NC
	m_csg->set_amp_res(RES_K(47));                  // R26 47k
	m_csg->set_feedback_res(RES_INF);               // NC
	m_csg->set_vco_params(5.0 * RES_VOLTAGE_DIVIDER(RES_K(4.7), RES_K(2.2)), 0, RES_K(47)); // R22/R19, NC, switchable R47/R40/R41/R42/R46/R45/R44/R43
	m_csg->set_pitch_voltage(5.0);                  // tied to Vreg
	m_csg->set_slf_params(CAP_U(4.7), RES_INF);     // C24, switchable NC/R54/R53/R52/R51
	m_csg->set_oneshot_params(0, RES_INF);          // NC, NC
	m_csg->set_vco_mode(1);                         // BIT15
	m_csg->set_mixer_params(0, 0, 0);               // GND, VCO/NOISE, GND
	m_csg->set_envelope_params(0, 1);               // GND, Vreg
	m_csg->set_enable(0);                           // AB SOUND

	TMS3615(config, m_synth_low, 4_MHz_XTAL/16/2); // from the other one's /2 clock output
	m_synth_low->add_route(tms3615_device::FOOTAGE_8, "speaker", 1.0);

	TMS3615(config, m_synth_high, 4_MHz_XTAL/16); // 4MHz divided down with a 74LS161
	m_synth_high->add_route(tms3615_device::FOOTAGE_8, "speaker", 1.0);
}

void catnmous_state::catnmous(machine_config &config)
{
	laserbat_base(config);

	// video hardware
	PALETTE(config, m_palette, FUNC(catnmous_state::catnmous_palette), 256);

	// sound board devices
	SPEAKER(config, "speaker").front_center();
	ZACCARIA_1B11107(config, m_audiopcb).add_route(ALL_OUTPUTS, "speaker", 1.0);
}


ROM_START( laserbat )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "lb02.7c",      0x0000, 0x0400, CRC(23a257cd) SHA1(08d9e1ff1a5cd8a5e5af6a12ba6104d3b2ccfddf) )
	ROM_CONTINUE(             0x4000, 0x0400 )
	ROM_LOAD( "lb02.6c",      0x0400, 0x0400, CRC(d1d6a67a) SHA1(727898c733633daffb0193cf4a556f89fe7e8a5a) )
	ROM_CONTINUE(             0x4400, 0x0400 )
	ROM_LOAD( "lb02.5c",      0x0800, 0x0400, CRC(8116f1d3) SHA1(f84ace44434c55ca5d0be9f0beb2d4df75694b2f) )
	ROM_CONTINUE(             0x4800, 0x0400 )
	ROM_LOAD( "lb02.3c",      0x0c00, 0x0400, CRC(443ef61e) SHA1(2849af0551bba7be2b4792739e04f18d6ace254c) )
	ROM_CONTINUE(             0x4c00, 0x0400 )
	ROM_LOAD( "lb02.2c",      0x1000, 0x0400, CRC(0cb8f5f1) SHA1(4ce22c5ae277033cb9905339d24cad272a878088) )
	ROM_CONTINUE(             0x5000, 0x0400 )
	ROM_LOAD( "lb02.7b",      0x2000, 0x0400, CRC(bdc769d1) SHA1(1291c159e779187efbdc3eb4a59a57d8d25ce08e) )
	ROM_CONTINUE(             0x6000, 0x0400 )
	ROM_LOAD( "lb02.6b",      0x2400, 0x0400, CRC(2103646f) SHA1(bbd15a19524aeb8647014914a0b3025a975dfe7c) )
	ROM_CONTINUE(             0x6400, 0x0400 )
	ROM_LOAD( "lb02.5b",      0x2800, 0x0400, CRC(3f8c4246) SHA1(b0d5e3733327140f54ac5a93f3f14d4afe085514) )
	ROM_CONTINUE(             0x6800, 0x0400 )
	ROM_LOAD( "lb02.3b",      0x2c00, 0x0400, CRC(3e557d52) SHA1(860046fcc2d952f3e677e576f1ac23deac2e7caf) )
	ROM_CONTINUE(             0x6c00, 0x0400 )
	ROM_LOAD( "lb02.2b",      0x3000, 0x0400, CRC(39000248) SHA1(58c6d1c588f4d1a3f579fe14faa8d2ccdfdc001e) )
	ROM_CONTINUE(             0x7000, 0x0400 )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "lb02.8g",      0x0000, 0x0800, CRC(4bb9f452) SHA1(1ff4ef94f0da3b59377548f3341b083af83f83c6) )
	ROM_LOAD( "lb02.10g",     0x0800, 0x0800, CRC(5fec6517) SHA1(868e57e8498cf1ab0fa3635845cdb5800fd96855) )
	ROM_LOAD( "lb02.11g",     0x1000, 0x0800, CRC(ceaf00a4) SHA1(2e789898207caa7619dcbb01f52c3532d1482618) )

	ROM_REGION( 0x0800, "gfx2", 0 )
	ROM_LOAD( "lb02.14l",     0x0000, 0x0800, CRC(d29962d1) SHA1(5b6d0856c3ebbd5833b522f7c0240309cf3c9777) )

	ROM_REGION( 0x0100, "gfxmix", 0 )
	// copied from lazarian to give working graphics, need dump to confirm
	ROM_LOAD( "82s100_prom",  0x0000, 0x00f5, CRC(c3eb562a) SHA1(65dff81b2e5321d530e5171dab9aa3809ab38b4d) BAD_DUMP )
ROM_END

ROM_START( lazarian )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "laz.7c",      0x0000, 0x0400, CRC(a2454cf2) SHA1(163b9323e77ee0107e13860b3468e002c335df9e) )
	ROM_CONTINUE(            0x4000, 0x0400 )
	ROM_LOAD( "laz.6c",      0x0400, 0x0400, CRC(23ee6013) SHA1(7ad53d6c321b0161906a512f6575620fd049d2f7) )
	ROM_CONTINUE(            0x4400, 0x0400 )
	ROM_LOAD( "laz.5c",      0x0800, 0x0400, CRC(4234a2ed) SHA1(dc98b04ae7dd1c35687bd8bdf42e8feb5eed321d) )
	ROM_CONTINUE(            0x4800, 0x0400 )
	ROM_LOAD( "laz.3c",      0x0c00, 0x0400, CRC(e901a636) SHA1(86320181a4d697fedfe8d8cbf9189854781e3d8c) )
	ROM_CONTINUE(            0x4c00, 0x0400 )
	ROM_LOAD( "laz.2c",      0x1000, 0x0400, CRC(657ed7c2) SHA1(8611912001d18af8c932efc7700c0d8b60efb2e8) )
	ROM_CONTINUE(            0x5000, 0x0400 )
	ROM_LOAD( "laz.7b",      0x2000, 0x0400, CRC(43135808) SHA1(2b704ca2f7a0fc46fddd5d7fb7d832a29d0562d0) )
	ROM_CONTINUE(            0x6000, 0x0400 )
	ROM_LOAD( "laz.6b",      0x2400, 0x0400, CRC(95701e50) SHA1(61d6a268696cefb760bf288bcc4eab7ac5f32ec7) )
	ROM_CONTINUE(            0x6400, 0x0400 )
	ROM_LOAD( "laz.5b",      0x2800, 0x0400, CRC(685842ba) SHA1(ee842d1d2c0676fddddf6e4e9cfd0b2962ae900d) )
	ROM_CONTINUE(            0x6800, 0x0400 )
	ROM_LOAD( "laz.3b",      0x2c00, 0x0400, CRC(9ddbe048) SHA1(70d1e8af073c85aba08e5251691842069617e6ac) )
	ROM_CONTINUE(            0x6c00, 0x0400 )
	ROM_LOAD( "laz10-62.2b", 0x3800, 0x0400, CRC(4ad9f7af) SHA1(71bcb9d148a7372b7be0abccdf71eeedba8b6c0a) )
	ROM_CONTINUE(            0x7800, 0x0400 )
	ROM_CONTINUE(            0x3000, 0x0400 )
	ROM_CONTINUE(            0x7000, 0x0400 )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "laz.8g",      0x0000, 0x0800, CRC(3cf76c01) SHA1(1824bc05e8dd2a522409e95fe81d2ad64182dcac) )
	ROM_LOAD( "laz.10g",     0x0800, 0x0800, CRC(256ae65d) SHA1(7f9e8ea1bbcb9e2175544556795c88c9981db571) )
	ROM_LOAD( "laz.11g",     0x1000, 0x0800, CRC(fec8266a) SHA1(7b90ae8d9eeb148012cca1bc93546dc3bf509258) )

	ROM_REGION( 0x0800, "gfx2", 0 )
	ROM_LOAD( "laz.14l",      0x0000, 0x0800, CRC(d29962d1) SHA1(5b6d0856c3ebbd5833b522f7c0240309cf3c9777) )

	ROM_REGION( 0x0100, "gfxmix", 0 )
	ROM_LOAD( "lz82s100.10m", 0x0000, 0x00f5, CRC(c3eb562a) SHA1(65dff81b2e5321d530e5171dab9aa3809ab38b4d) )
ROM_END

/*
Zaccaria "Cat 'N Mouse" 1982

similar to "Quasar" except it uses an 82s100 for color table lookup
and has a larger program prom


Cat N Mouse (Zaccaria 1982)

CPU Board

               2650    7b 6b 5b 3b 2b
                       7c 6c 5c 3c 2c

                       2636 2636 2636
        11g 10g 8g
     14l
                  clr

Sound Board 1b11107

6802
6821
2*8910

Labels are in the following format:

   CAT'N MOUSE      <-- Game name, printed
TYPE ____________   <-- Line to hand write type (revision and/or license?)
MEM. N. _________   <-- Line to hand write ROM number or PCB location

* Sound ROM labels have "SOUND" in place of "TYPE"
*/

ROM_START( catnmous )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "cat_n_mouse_type_02_mem_n_1.7c",     0x0000, 0x0400, CRC(d26ec566) SHA1(ceb16f64a3c1ff25a9eab6549f1ae24085bb9e27) )
	ROM_CONTINUE(                                   0x4000, 0x0400 )
	ROM_LOAD( "cat_n_mouse_type_02_mem_n_2.6c",     0x0400, 0x0400, CRC(02a7e36c) SHA1(8495b2906ecb0791a47e9b6f1959ed6cbc14cce8) )
	ROM_CONTINUE(                                   0x4400, 0x0400 )
	ROM_LOAD( "cat_n_mouse_type_02_mem_n_3.5c",     0x0800, 0x0400, CRC(ee9f90ee) SHA1(dc280dae3a18a9044497bdee41827d2510a04d06) )
	ROM_CONTINUE(                                   0x4800, 0x0400 )
	ROM_LOAD( "cat_n_mouse_type_02_mem_n_4.3c",     0x0c00, 0x0400, CRC(71b97af9) SHA1(6735184dc16c8db3050be3b7b5dfdb7d46a671fe) )
	ROM_CONTINUE(                                   0x4c00, 0x0400 )
	ROM_LOAD( "cat_n_mouse_type_02_mem_n_5.2c",     0x1000, 0x0400, CRC(887a1da2) SHA1(9e2548d1792c2d2b76811a1e0daae4d378f1f354) )
	ROM_CONTINUE(                                   0x5000, 0x0400 )
	ROM_LOAD( "cat_n_mouse_type_02_mem_n_6.7b",     0x2000, 0x0400, CRC(22e045e9) SHA1(dd332e918500d8024d1329bc12c6f939fd41e4a7) )
	ROM_CONTINUE(                                   0x6000, 0x0400 )
	ROM_LOAD( "cat_n_mouse_type_02_mem_n_7.6b",     0x2400, 0x0400, CRC(af330ad2) SHA1(cac70341687edd1daee323c0e332297c80057e1e) )
	ROM_CONTINUE(                                   0x6400, 0x0400 )
	ROM_LOAD( "cat_n_mouse_type_02_mem_n_8.5b",     0x2800, 0x0400, CRC(c7d38401) SHA1(33a3bb393451cd3fefa23b5c8013068b5b0de7a5) )
	ROM_CONTINUE(                                   0x6800, 0x0400 )
	ROM_LOAD( "cat_n_mouse_type_02_mem_n_9.3b",     0x2c00, 0x0400, CRC(c4a33f20) SHA1(355c4345daa681fa2bcfa1e345d2db34f9d94113) )
	ROM_CONTINUE(                                   0x6c00, 0x0400 )
	ROM_LOAD( "cat_n_mouse_type_02_mem_n_10-11.2b", 0x3800, 0x0400, CRC(3f7d4b89) SHA1(c8e9be0149a2f728526a416ec5663e69cc2e6758) ) // labeled: CAT'N MOUSE   TYPE 02  MEM. N. 10/11 - "02" and "10/11" are hand written
	ROM_CONTINUE(                                   0x7800, 0x0400 )
	ROM_CONTINUE(                                   0x3000, 0x0400 )
	ROM_CONTINUE(                                   0x7000, 0x0400 )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "cat_n_mouse_type_01_mem_n_8g.8g",   0x0000, 0x0800, CRC(2b180d4a) SHA1(b6f48ffdbad64b4d9f1fe838000187800c51228c) )
	ROM_LOAD( "cat_n_mouse_type_01_mem_n_10g.10g", 0x0800, 0x0800, CRC(e5259f9b) SHA1(396753291ab36c3ed72208d619665fc0f33d1e17) )
	ROM_LOAD( "cat_n_mouse_type_01_mem_n_11g.11g", 0x1000, 0x0800, CRC(2999f378) SHA1(929082383b2b0006de171587adb932ce57316963) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "cat_n_mouse_type_01_mem_n_14l.14l", 0x0000, 0x1000, CRC(83502383) SHA1(9561f87e1a6425bb9544e71340336db8d43c1fd9) )

	ROM_REGION( 0x0100, "gfxmix", 0 )
	ROM_LOAD( "82s100.10m",   0x0000, 0x00f5, CRC(6b724cdb) SHA1(8a0ca3b171b103661a3b2fffbca3d7162089e243) )

	ROM_REGION( 0x10000, "audiopcb:melodycpu", 0 )
	ROM_LOAD( "cat_n_mouse_sound_01_mem_n_1f.1f", 0xc000, 0x1000, CRC(473c44de) SHA1(ff08b02d45a2c23cabb5db716aa203225a931424) )
	ROM_LOAD( "cat_n_mouse_sound_01_mem_n_1d.1d", 0xe000, 0x1000, CRC(f65cb9d0) SHA1(a2fe7563c6da055bf6aa20797b2d9fa184f0133c) )
	ROM_LOAD( "cat_n_mouse_sound_01_mem_n_1e.1e", 0xf000, 0x1000, CRC(1bd90c93) SHA1(20fd2b765a42e25cf7f716e6631b8c567785a866) )
ROM_END

ROM_START( catnmousa )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "cat_n_mouse_type_01_mem_n_1.7c",  0x0000, 0x0400, CRC(0bf9fc06) SHA1(7d5857121fe51f43e4ae7db34df720198994afdd) )
	ROM_CONTINUE(                                0x4000, 0x0400 )
	ROM_LOAD( "cat_n_mouse_type_01_mem_n_2.6c",  0x0400, 0x0400, CRC(b0e140a0) SHA1(68d8ca25642e872f2177d09b78d553c033411dd5) )
	ROM_CONTINUE(                                0x4400, 0x0400 )
	ROM_LOAD( "cat_n_mouse_type_01_mem_n_3.5c",  0x0800, 0x0400, CRC(7bbc0fe5) SHA1(d20e89d89a0958d45ac31b6d2c540fcf3d326068) )
	ROM_CONTINUE(                                0x4800, 0x0400 )
	ROM_LOAD( "cat_n_mouse_type_01_mem_n_4.3c",  0x0c00, 0x0400, CRC(0350531d) SHA1(6115f907544ab317e0090a10cce3adce26f4afd9) )
	ROM_CONTINUE(                                0x4c00, 0x0400 )
	ROM_LOAD( "cat_n_mouse_type_01_mem_n_5.2c",  0x1000, 0x0400, CRC(4a26e963) SHA1(be8dd98d3810319a228ce4c07b097eb75f2d1e5c) )
	ROM_CONTINUE(                                0x5000, 0x0400 )
	ROM_LOAD( "cat_n_mouse_type_01_mem_n_6.7b",  0x2000, 0x0400, CRC(d8d6a029) SHA1(7e5688fd3af97620ed07d9375335fe1deb6e483f) )
	ROM_CONTINUE(                                0x6000, 0x0400 )
	ROM_LOAD( "cat_n_mouse_type_01_mem_n_7.6b",  0x2400, 0x0400, CRC(ccc871d9) SHA1(355eff250ab3d1a75ed690369add1639e7061ee8) )
	ROM_CONTINUE(                                0x6400, 0x0400 )
	ROM_LOAD( "cat_n_mouse_type_01_mem_n_8.5b",  0x2800, 0x0400, CRC(23783b84) SHA1(97a3ef7c64e1ded5cc1999d3aa58652ca541166c) )
	ROM_CONTINUE(                                0x6800, 0x0400 )
	ROM_LOAD( "cat_n_mouse_type_01_mem_n_9.3b",  0x2c00, 0x0400, CRC(e99fce4b) SHA1(2c8efdea55bae5526b547fec53e8f3642fe2bd2e) )
	ROM_CONTINUE(                                0x6c00, 0x0400 )
	ROM_LOAD( "cat_n_mouse_type_01_mem_n_10.2b", 0x3800, 0x0400, CRC(807b7109) SHA1(6c29197b437ab0132d8361f921e6d0d9b10f917e) )
	ROM_CONTINUE(                                0x7800, 0x0400 )
	ROM_CONTINUE(                                0x3000, 0x0400 )
	ROM_CONTINUE(                                0x7000, 0x0400 )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "cat_n_mouse_type_01_mem_n_8g.8g",   0x0000, 0x0800, CRC(2b180d4a) SHA1(b6f48ffdbad64b4d9f1fe838000187800c51228c) )
	ROM_LOAD( "cat_n_mouse_type_01_mem_n_10g.10g", 0x0800, 0x0800, CRC(e5259f9b) SHA1(396753291ab36c3ed72208d619665fc0f33d1e17) )
	ROM_LOAD( "cat_n_mouse_type_01_mem_n_11g.11g", 0x1000, 0x0800, CRC(2999f378) SHA1(929082383b2b0006de171587adb932ce57316963) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "cat_n_mouse_type_01_mem_n_14l.14l", 0x0000, 0x1000, CRC(83502383) SHA1(9561f87e1a6425bb9544e71340336db8d43c1fd9) )

	ROM_REGION( 0x0100, "gfxmix", 0 ) // copied from parent set to give working graphics, need dump to confirm
	ROM_LOAD( "cnm_82s100.10m", 0x0000, 0x00f5, CRC(6b724cdb) SHA1(8a0ca3b171b103661a3b2fffbca3d7162089e243) BAD_DUMP ) // labeled C.N.M - 82S100

	ROM_REGION( 0x10000, "audiopcb:melodycpu", 0 )
	ROM_LOAD( "cat_n_mouse_sound_01_mem_n_1f.1f", 0xc000, 0x1000, CRC(473c44de) SHA1(ff08b02d45a2c23cabb5db716aa203225a931424) )
	ROM_LOAD( "cat_n_mouse_sound_01_mem_n_1d.1d", 0xe000, 0x1000, CRC(f65cb9d0) SHA1(a2fe7563c6da055bf6aa20797b2d9fa184f0133c) )
	ROM_LOAD( "cat_n_mouse_sound_01_mem_n_1e.1e", 0xf000, 0x1000, CRC(1bd90c93) SHA1(20fd2b765a42e25cf7f716e6631b8c567785a866) )
ROM_END


GAME( 1981, laserbat,  0,        laserbat, laserbat, laserbat_state, empty_init, ROT0,  "Zaccaria", "Laser Battle",                    MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1981, lazarian,  laserbat, laserbat, lazarian, laserbat_state, empty_init, ROT0,  "Zaccaria (Bally Midway license)", "Lazarian", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1982, catnmous,  0,        catnmous, catnmous, catnmous_state, empty_init, ROT90, "Zaccaria", "Cat and Mouse (type 02 program)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1982, catnmousa, catnmous, catnmous, catnmous, catnmous_state, empty_init, ROT90, "Zaccaria", "Cat and Mouse (type 01 program)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
