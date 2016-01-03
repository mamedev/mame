// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Vas Crabb
/*
    Laser Battle / Lazarian (c) 1981 Zaccaria
    Cat and Mouse           (c) 1982 Zaccaria

    original driver by Pierpaolo Prazzoli

    The two games have a similar video hardware, but sound hardware is
    very different and they don't use the collision detection provided
    by the s2636 chips.

    Game board supports two different sound board interfaces: 16-bit
    unidirectional bus on J3 and 8-bit bidirectional bus on J7.
    Lazarian uses only the 16-bit unidirectional interface.  The 16-bit
    interface is controlled by latches at I/O addresses 2 (bits 1-8)
    and 7 (bits 9-16).  The 8-bit interface is read at I/O address 0 and
    written at I/O address 3.  The sound board controls data direction
    on J7 and when input from sound board to game board is latched.

    Laser Battle/Lazarian notes:
    * Cocktail cabinet has an additional "image commutation board"
      consuming the screen flip output, presumably flipping the image by
      means of dark magic
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

    TODO:
    - work out where all the magic layer offsets come from
    - second bank of DIP switches in laserbat
    - sound in laserbat (with schematics) and in catnmous
*/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "cpu/s2650/s2650.h"
#include "machine/6821pia.h"
#include "includes/laserbat.h"


WRITE8_MEMBER(laserbat_state_base::ct_io_w)
{
	/*
	    Uses a hex buffer, so bits 6 and 7 are not physically present.

	    Bits 0-2 are open collector outputs with a diode connected to
	    the return line to suppress solenoid switching transients.
	    These are used to drive coin counters.

	    Bit 3 is an open collector output with a 2k2 pull-up resistor.
	    It is used to drive the "image commutation board" used to flip
	    the screen for player 2 in cocktail configuration.

	    Bits 4-5 feed the input row select decoder that switches between
	    ROW0, ROW1, SW1 and SW2 (ROW2 is selected using a bit in the
	    video effects register, just to be confusing).

	    +-----+-----------------------------+-------------------+------------------------------+
	    | bit | output                      | Laser Battle      | Lazarian                     |
	    +-----+-----------------------------+-------------------+------------------------------+
	    |  0  | J2-3 solenoid driver        | 1*Credits         | 1*Coin C                     |
	    |     |                             |                   |                              |
	    |  1  | J2-8 solenoid driver        |  5*Coin A         | 1*Coin A                     |
	    |     |                             | 10*Coin B         |                              |
	    |     |                             |  1*Coin C         |                              |
	    |     |                             |                   |                              |
	    |  2  | J2-6 solenoid driver        |                   | 1*Coin B                     |
	    |     |                             |                   |                              |
	    |  3  | J3-4 open collector output  | Connected to cocktail "image commutation board"  |
	    |     |                             |                   |                              |
	    |  4  | input row select A          |                   |                              |
	    |     |                             |                   |                              |
	    |  5  | input row select B          |                   |                              |
	    +-----+-----------------------------+-------------------+------------------------------+
	*/

	coin_counter_w(machine(), 0, data & 0x01);
	coin_counter_w(machine(), 1, data & 0x02);
	coin_counter_w(machine(), 2, data & 0x04);
	flip_screen_set((bool(data & 0x08) && !bool(m_row1->read() & 0x10)) ? 1 : 0);
	m_input_mux = (data >> 4) & 0x03;

//  popmessage("ct io: %02X", data);
}

READ8_MEMBER(laserbat_state_base::rrowx_r)
{
	ioport_port *const mux_ports[] = { m_row0, m_row1, m_sw1, m_sw2 };
	return (m_mpx_p_1_2 ? m_row2 : mux_ports[m_input_mux])->read();
}

/*

    Color handling with 2716.14L and 82S100.10M

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

static ADDRESS_MAP_START( laserbat_map, AS_PROGRAM, 8, laserbat_state_base )
	ADDRESS_MAP_UNMAP_HIGH

	AM_RANGE(0x0000, 0x13ff) AM_ROM
	AM_RANGE(0x2000, 0x33ff) AM_ROM
	AM_RANGE(0x3800, 0x3bff) AM_ROM
	AM_RANGE(0x4000, 0x53ff) AM_ROM
	AM_RANGE(0x6000, 0x73ff) AM_ROM
	AM_RANGE(0x7800, 0x7bff) AM_ROM

	AM_RANGE(0x1400, 0x14ff) AM_MIRROR(0x6000) AM_WRITENOP // always 0 (bullet ram in Quasar)
	AM_RANGE(0x1500, 0x15ff) AM_MIRROR(0x6000) AM_DEVREADWRITE("pvi1", s2636_device, read_data, write_data)
	AM_RANGE(0x1600, 0x16ff) AM_MIRROR(0x6000) AM_DEVREADWRITE("pvi2", s2636_device, read_data, write_data)
	AM_RANGE(0x1700, 0x17ff) AM_MIRROR(0x6000) AM_DEVREADWRITE("pvi3", s2636_device, read_data, write_data)
	AM_RANGE(0x1800, 0x1bff) AM_MIRROR(0x6000) AM_WRITE(videoram_w)
	AM_RANGE(0x1c00, 0x1fff) AM_MIRROR(0x6000) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( laserbat_io_map, AS_IO, 8, laserbat_state_base )
	AM_RANGE(0x00, 0x00) AM_READ(rhsc_r)    AM_WRITE(cnt_eff_w)
	AM_RANGE(0x01, 0x01) /* RBALL */        AM_WRITE(cnt_nav_w)
	AM_RANGE(0x02, 0x02) AM_READ(rrowx_r)   AM_WRITE(csound1_w)
	AM_RANGE(0x03, 0x03)                    AM_WRITE(whsc_w)
	AM_RANGE(0x04, 0x04)                    AM_WRITE(wcoh_w)
	AM_RANGE(0x05, 0x05)                    AM_WRITE(wcov_w)
	AM_RANGE(0x06, 0x06)                    AM_WRITE(ct_io_w)
	AM_RANGE(0x07, 0x07)                    AM_WRITE(csound2_w)

	AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READ_PORT("SENSE")
ADDRESS_MAP_END


static ADDRESS_MAP_START( catnmous_sound_map, AS_PROGRAM, 8, catnmous_state )
	AM_RANGE(0x0000, 0x007f) AM_RAM
	AM_RANGE(0x500c, 0x500f) AM_DEVREADWRITE("pia", pia6821_device, read, write)
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END


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
	PORT_DIPSETTING(     0x10, DEF_STR(Upright) )
	PORT_DIPSETTING(     0x00, DEF_STR(Cocktail) )
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
	PORT_DIPNAME( 0x40, 0x40, DEF_STR(Unknown) )        PORT_DIPLOCATION("SW-1:7")
	PORT_DIPSETTING(    0x40, DEF_STR(Off) )
	PORT_DIPSETTING(    0x00, DEF_STR(On) )
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
	PORT_DIPNAME( 0x08, 0x08, DEF_STR(Unknown) )        PORT_DIPLOCATION("SW-2:4")
	PORT_DIPSETTING(    0x08, DEF_STR(Off) )
	PORT_DIPSETTING(    0x00, DEF_STR(On) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR(Unknown) )        PORT_DIPLOCATION("SW-2:5")
	PORT_DIPSETTING(    0x10, DEF_STR(Off) )
	PORT_DIPSETTING(    0x00, DEF_STR(On) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR(Unknown) )        PORT_DIPLOCATION("SW-2:6")
	PORT_DIPSETTING(    0x20, DEF_STR(Off) )
	PORT_DIPSETTING(    0x00, DEF_STR(On) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR(Unknown) )        PORT_DIPLOCATION("SW-2:7")
	PORT_DIPSETTING(    0x40, DEF_STR(Off) )
	PORT_DIPSETTING(    0x00, DEF_STR(On) )
	PORT_DIPNAME( 0x80, 0x80, "Coin C" )                PORT_DIPLOCATION("SW-2:8")
	PORT_DIPSETTING(    0x00, DEF_STR(2C_1C) )
	PORT_DIPSETTING(    0x80, DEF_STR(1C_1C) )

	PORT_START("SENSE")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END


static INPUT_PORTS_START( laserbat )
	PORT_INCLUDE(laserbat_base)

	PORT_MODIFY("ROW0")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Fire Left")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Fire Right")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Fire Up")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 Fire Down")

	PORT_MODIFY("ROW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Fire Left")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Fire Right")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Fire Up")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 Fire Down")

	PORT_MODIFY("SW1")
	PORT_DIPNAME( 0x70, 0x10, DEF_STR(Lives) )          PORT_DIPLOCATION("SW-1:5,6,7")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "6" )
	PORT_DIPSETTING(    0x40, DEF_STR(Infinite) )
//  PORT_DIPSETTING(    0x50, DEF_STR(Infinite) )
//  PORT_DIPSETTING(    0x60, DEF_STR(Infinite) )
//  PORT_DIPSETTING(    0x70, DEF_STR(Infinite) )
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
	PORT_DIPNAME( 0x18, 0x08, DEF_STR(Difficulty) )     PORT_DIPLOCATION("SW-2:4,5")
	PORT_DIPSETTING(    0x00, DEF_STR(Easy) )
	PORT_DIPSETTING(    0x08, DEF_STR(Medium) )
	PORT_DIPSETTING(    0x10, DEF_STR(Difficult) )
	PORT_DIPSETTING(    0x18, DEF_STR(Very_Difficult) )
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
	PORT_DIPNAME( 0x70, 0x10, DEF_STR(Lives) )          PORT_DIPLOCATION("SW-1:5,6,7")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x30, "5" )
	PORT_DIPSETTING(    0x40, DEF_STR(Infinite) )
//  PORT_DIPSETTING(    0x50, DEF_STR(Infinite) )
//  PORT_DIPSETTING(    0x60, DEF_STR(Infinite) )
//  PORT_DIPSETTING(    0x70, DEF_STR(Infinite) )
	PORT_DIPNAME( 0x80, 0x80, "Game Over Melody" )  PORT_DIPLOCATION("SW-1:8")
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )
	PORT_DIPSETTING(    0x80, DEF_STR(On) )
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

static GFXDECODE_START( laserbat )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout,       0, 256 ) /* Rom chars */
	GFXDECODE_ENTRY( "gfx2", 0x0000, sprites_layout,   0,   8 ) /* Sprites   */
GFXDECODE_END


/* Cat'N Mouse sound ***********************************/

WRITE_LINE_MEMBER(catnmous_state::zaccaria_irq0a)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(catnmous_state::zaccaria_irq0b)
{
	m_audiocpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}

READ8_MEMBER(catnmous_state::zaccaria_port0a_r)
{
	ay8910_device *ay8910 = (m_active_8910 == 0) ? m_ay1 : m_ay2;
	return ay8910->data_r(space, 0);
}

WRITE8_MEMBER(catnmous_state::zaccaria_port0a_w)
{
	m_port0a = data;
}

WRITE8_MEMBER(catnmous_state::zaccaria_port0b_w)
{
	/* bit 1 goes to 8910 #0 BDIR pin  */
	if ((m_last_port0b & 0x02) == 0x02 && (data & 0x02) == 0x00)
	{
		/* bit 0 goes to the 8910 #0 BC1 pin */
		m_ay1->data_address_w(space, m_last_port0b >> 0, m_port0a);
	}
	else if ((m_last_port0b & 0x02) == 0x00 && (data & 0x02) == 0x02)
	{
		/* bit 0 goes to the 8910 #0 BC1 pin */
		if (m_last_port0b & 0x01)
			m_active_8910 = 0;
	}
	/* bit 3 goes to 8910 #1 BDIR pin  */
	if ((m_last_port0b & 0x08) == 0x08 && (data & 0x08) == 0x00)
	{
		/* bit 2 goes to the 8910 #1 BC1 pin */
		m_ay2->data_address_w(space, m_last_port0b >> 2, m_port0a);
	}
	else if ((m_last_port0b & 0x08) == 0x00 && (data & 0x08) == 0x08)
	{
		/* bit 2 goes to the 8910 #1 BC1 pin */
		if (m_last_port0b & 0x04)
			m_active_8910 = 1;
	}

	m_last_port0b = data;
}

INTERRUPT_GEN_MEMBER(laserbat_state_base::laserbat_interrupt)
{
	device.execute().set_input_line_and_vector(0, HOLD_LINE, 0x0a);
}

INTERRUPT_GEN_MEMBER(catnmous_state::zaccaria_cb1_toggle)
{
	m_pia->cb1_w(m_cb1_toggle & 1);
	m_cb1_toggle ^= 1;
}

DRIVER_INIT_MEMBER(laserbat_state_base, laserbat)
{
	m_scanline_timer = timer_alloc(TIMER_SCANLINE);

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

	save_item(NAME(m_csound1));
	save_item(NAME(m_csound2));
	save_item(NAME(m_rhsc));
	save_item(NAME(m_whsc));
}

void laserbat_state::machine_start()
{
	laserbat_state_base::machine_start();

	save_item(NAME(m_keys));
}

void catnmous_state::machine_start()
{
	laserbat_state_base::machine_start();

	save_item(NAME(m_active_8910));
	save_item(NAME(m_port0a));
	save_item(NAME(m_last_port0b));
	save_item(NAME(m_cb1_toggle));
}

void catnmous_state::machine_reset()
{
	laserbat_state_base::machine_reset();

	m_active_8910 = 0;
	m_port0a = 0;
	m_last_port0b = 0;
	m_cb1_toggle = 0;
}

void laserbat_state_base::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_SCANLINE:
		video_line(ptr, param);
		break;
	default:
		assert_always(FALSE, "Unknown id in laserbat_state_base::device_timer");
	}
}


static MACHINE_CONFIG_START( laserbat_base, laserbat_state_base )

	// basic machine hardware
	MCFG_CPU_ADD("maincpu", S2650, XTAL_14_31818MHz/4)
	MCFG_CPU_PROGRAM_MAP(laserbat_map)
	MCFG_CPU_IO_MAP(laserbat_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", laserbat_state_base, laserbat_interrupt)

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_14_31818MHz, 227*4, 43*4-1, 227*4-1, 312, 8, 255)
	MCFG_SCREEN_UPDATE_DRIVER(laserbat_state_base, screen_update_laserbat)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(laserbat_state_base, laserbat)

	MCFG_PLS100_ADD("gfxmix")

	MCFG_DEVICE_ADD("pvi1", S2636, XTAL_14_31818MHz/3)
	MCFG_S2636_OFFSETS(-8, -16)
	MCFG_S2636_DIVIDER(3)

	MCFG_DEVICE_ADD("pvi2", S2636, XTAL_14_31818MHz/3)
	MCFG_S2636_OFFSETS(-8, -16)
	MCFG_S2636_DIVIDER(3)

	MCFG_DEVICE_ADD("pvi3", S2636, XTAL_14_31818MHz/3)
	MCFG_S2636_OFFSETS(-8, -16)
	MCFG_S2636_DIVIDER(3)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", laserbat)

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( laserbat, laserbat_base, laserbat_state )

	// sound board devices
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("csg", SN76477, 0) // audio output not used
	MCFG_SN76477_NOISE_PARAMS(RES_K(47), RES_K(270), CAP_P(1000)) // R21, switchable R30/R23/R24/R25/R29/R28/R27/R26, C21
	MCFG_SN76477_DECAY_RES(RES_INF)                 // NC
	MCFG_SN76477_ATTACK_PARAMS(0, RES_INF)          // NC, NC
	MCFG_SN76477_AMP_RES(RES_K(47))                 // R26 47k
	MCFG_SN76477_FEEDBACK_RES(RES_INF)              // NC
	MCFG_SN76477_VCO_PARAMS(5.0 * RES_VOLTAGE_DIVIDER(RES_K(4.7), RES_K(2.2)), 0, RES_K(47)) // R22/R19, NC, switchable R47/R40/R41/R42/R46/R45/R44/R43
	MCFG_SN76477_PITCH_VOLTAGE(5.0)                 // tied to Vreg
	MCFG_SN76477_SLF_PARAMS(CAP_U(4.7), RES_INF)    // C24, switchable NC/R54/R53/R52/R51
	MCFG_SN76477_ONESHOT_PARAMS(0, RES_INF)         // NC, NC
	MCFG_SN76477_VCO_MODE(1)                        // BIT15
	MCFG_SN76477_MIXER_PARAMS(0, 0, 0)              // GND, VCO/NOISE, GND
	MCFG_SN76477_ENVELOPE_PARAMS(0, 1)              // GND, Vreg
	MCFG_SN76477_ENABLE(0)                          // AB SOUND

	MCFG_TMS3615_ADD("synth_high", XTAL_4MHz/16/2) // from the other one's /2 clock output
	MCFG_SOUND_ROUTE(TMS3615_FOOTAGE_8, "mono", 1.0)

	MCFG_TMS3615_ADD("synth_low", XTAL_4MHz/16) // 4MHz divided down with a 74LS161
	MCFG_SOUND_ROUTE(TMS3615_FOOTAGE_8, "mono", 1.0)

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED_CLASS( catnmous, laserbat_base, catnmous_state )

	// sound board devices
	MCFG_CPU_ADD("audiocpu", M6802, 3580000) // ?
	MCFG_CPU_PROGRAM_MAP(catnmous_sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(catnmous_state, zaccaria_cb1_toggle,  (double)3580000/4096)

	MCFG_DEVICE_ADD("pia", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(catnmous_state, zaccaria_port0a_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(catnmous_state, zaccaria_port0a_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(catnmous_state, zaccaria_port0b_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(catnmous_state, zaccaria_irq0a))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(catnmous_state, zaccaria_irq0b))

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, 3580000/2) // ?
	MCFG_AY8910_PORT_B_READ_CB(READ8(driver_device, soundlatch_byte_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("ay2", AY8910, 3580000/2) // ?
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_CONFIG_END


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

similar to "Quasar" execept it uses an 82s100 for color table lookup
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
8910
*/

ROM_START( catnmous )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "02-1.7c",      0x0000, 0x0400, CRC(d26ec566) SHA1(ceb16f64a3c1ff25a9eab6549f1ae24085bb9e27) )
	ROM_CONTINUE(             0x4000, 0x0400 )
	ROM_LOAD( "02-2.6c",      0x0400, 0x0400, CRC(02a7e36c) SHA1(8495b2906ecb0791a47e9b6f1959ed6cbc14cce8) )
	ROM_CONTINUE(             0x4400, 0x0400 )
	ROM_LOAD( "02-3.5c",      0x0800, 0x0400, CRC(ee9f90ee) SHA1(dc280dae3a18a9044497bdee41827d2510a04d06) )
	ROM_CONTINUE(             0x4800, 0x0400 )
	ROM_LOAD( "02-4.3c",      0x0c00, 0x0400, CRC(71b97af9) SHA1(6735184dc16c8db3050be3b7b5dfdb7d46a671fe) )
	ROM_CONTINUE(             0x4c00, 0x0400 )
	ROM_LOAD( "02-5.2c",      0x1000, 0x0400, CRC(887a1da2) SHA1(9e2548d1792c2d2b76811a1e0daae4d378f1f354) )
	ROM_CONTINUE(             0x5000, 0x0400 )
	ROM_LOAD( "02-6.7b",      0x2000, 0x0400, CRC(22e045e9) SHA1(dd332e918500d8024d1329bc12c6f939fd41e4a7) )
	ROM_CONTINUE(             0x6000, 0x0400 )
	ROM_LOAD( "02-7.6b",      0x2400, 0x0400, CRC(af330ad2) SHA1(cac70341687edd1daee323c0e332297c80057e1e) )
	ROM_CONTINUE(             0x6400, 0x0400 )
	ROM_LOAD( "02-8.5b",      0x2800, 0x0400, CRC(c7d38401) SHA1(33a3bb393451cd3fefa23b5c8013068b5b0de7a5) )
	ROM_CONTINUE(             0x6800, 0x0400 )
	ROM_LOAD( "02-9.3b",      0x2c00, 0x0400, CRC(c4a33f20) SHA1(355c4345daa681fa2bcfa1e345d2db34f9d94113) )
	ROM_CONTINUE(             0x6c00, 0x0400 )
	ROM_LOAD( "02-10-11.2b",  0x3800, 0x0400, CRC(3f7d4b89) SHA1(c8e9be0149a2f728526a416ec5663e69cc2e6758) )
	ROM_CONTINUE(             0x7800, 0x0400 )
	ROM_CONTINUE(             0x3000, 0x0400 )
	ROM_CONTINUE(             0x7000, 0x0400 )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "type01.8g",    0x0000, 0x0800, CRC(2b180d4a) SHA1(b6f48ffdbad64b4d9f1fe838000187800c51228c) )
	ROM_LOAD( "type01.10g",   0x0800, 0x0800, CRC(e5259f9b) SHA1(396753291ab36c3ed72208d619665fc0f33d1e17) )
	ROM_LOAD( "type01.11g",   0x1000, 0x0800, CRC(2999f378) SHA1(929082383b2b0006de171587adb932ce57316963) )

	ROM_REGION( 0x0800, "gfx2", 0 )
	ROM_LOAD( "type01.14l",   0x0000, 0x0800, CRC(af79179a) SHA1(de61af7d02c93be326a33ee51572e3da7a25dab0) )

	ROM_REGION( 0x0100, "gfxmix", 0 )
	ROM_LOAD( "82s100.13m",   0x0000, 0x00f5, CRC(6b724cdb) SHA1(8a0ca3b171b103661a3b2fffbca3d7162089e243) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sound01.1d",   0xd000, 0x1000, CRC(f65cb9d0) SHA1(a2fe7563c6da055bf6aa20797b2d9fa184f0133c) )
	ROM_LOAD( "sound01.1f",   0xe000, 0x1000, CRC(473c44de) SHA1(ff08b02d45a2c23cabb5db716aa203225a931424) )
	ROM_LOAD( "sound01.1e",   0xf000, 0x1000, CRC(1bd90c93) SHA1(20fd2b765a42e25cf7f716e6631b8c567785a866) )
ROM_END

ROM_START( catnmousa )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "catnmous.7c",  0x0000, 0x0400, CRC(0bf9fc06) SHA1(7d5857121fe51f43e4ae7db34df720198994afdd) )
	ROM_CONTINUE(             0x4000, 0x0400 )
	ROM_LOAD( "catnmous.6c",  0x0400, 0x0400, CRC(b0e140a0) SHA1(68d8ca25642e872f2177d09b78d553c033411dd5) )
	ROM_CONTINUE(             0x4400, 0x0400 )
	ROM_LOAD( "catnmous.5c",  0x0800, 0x0400, CRC(7bbc0fe5) SHA1(d20e89d89a0958d45ac31b6d2c540fcf3d326068) )
	ROM_CONTINUE(             0x4800, 0x0400 )
	ROM_LOAD( "catnmous.3c",  0x0c00, 0x0400, CRC(0350531d) SHA1(6115f907544ab317e0090a10cce3adce26f4afd9) )
	ROM_CONTINUE(             0x4c00, 0x0400 )
	ROM_LOAD( "catnmous.2c",  0x1000, 0x0400, CRC(4a26e963) SHA1(be8dd98d3810319a228ce4c07b097eb75f2d1e5c) )
	ROM_CONTINUE(             0x5000, 0x0400 )
	ROM_LOAD( "catnmous.7b",  0x2000, 0x0400, CRC(d8d6a029) SHA1(7e5688fd3af97620ed07d9375335fe1deb6e483f) )
	ROM_CONTINUE(             0x6000, 0x0400 )
	ROM_LOAD( "catnmous.6b",  0x2400, 0x0400, CRC(ccc871d9) SHA1(355eff250ab3d1a75ed690369add1639e7061ee8) )
	ROM_CONTINUE(             0x6400, 0x0400 )
	ROM_LOAD( "catnmous.5b",  0x2800, 0x0400, CRC(23783b84) SHA1(97a3ef7c64e1ded5cc1999d3aa58652ca541166c) )
	ROM_CONTINUE(             0x6800, 0x0400 )
	ROM_LOAD( "catnmous.3b",  0x2c00, 0x0400, CRC(e99fce4b) SHA1(2c8efdea55bae5526b547fec53e8f3642fe2bd2e) )
	ROM_CONTINUE(             0x6c00, 0x0400 )
	// missing half rom
	ROM_LOAD( "catnmous.2b",  0x3000, 0x0400, BAD_DUMP CRC(880728fa) SHA1(f204d669c190ad0cf2c885af12625026534db655) )
	ROM_CONTINUE(             0x7000, 0x0400 )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "catnmous.8g",  0x0000, 0x0800, CRC(2b180d4a) SHA1(b6f48ffdbad64b4d9f1fe838000187800c51228c) )
	ROM_LOAD( "catnmous.10g", 0x0800, 0x0800, CRC(e5259f9b) SHA1(396753291ab36c3ed72208d619665fc0f33d1e17) )
	ROM_LOAD( "catnmous.11g", 0x1000, 0x0800, CRC(2999f378) SHA1(929082383b2b0006de171587adb932ce57316963) )

	ROM_REGION( 0x0800, "gfx2", 0 )
	ROM_LOAD( "catnmous.14l", 0x0000, 0x0800, CRC(af79179a) SHA1(de61af7d02c93be326a33ee51572e3da7a25dab0) )

	ROM_REGION( 0x0100, "gfxmix", 0 )
	// copied from parent set to give working graphics, need dump to confirm
	ROM_LOAD( "catnmousa_82s100.13m", 0x0000, 0x00f5, CRC(6b724cdb) SHA1(8a0ca3b171b103661a3b2fffbca3d7162089e243) BAD_DUMP )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "snd.1d",       0xd000, 0x1000, CRC(f65cb9d0) SHA1(a2fe7563c6da055bf6aa20797b2d9fa184f0133c) )
	ROM_LOAD( "snd.1f",       0xe000, 0x1000, CRC(473c44de) SHA1(ff08b02d45a2c23cabb5db716aa203225a931424) )
	ROM_LOAD( "snd.1e",       0xf000, 0x1000, CRC(1bd90c93) SHA1(20fd2b765a42e25cf7f716e6631b8c567785a866) )
ROM_END


GAME( 1981, laserbat, 0,        laserbat, laserbat, laserbat_state_base, laserbat, ROT0,  "Zaccaria", "Laser Battle",                    MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1981, lazarian, laserbat, laserbat, lazarian, laserbat_state_base, laserbat, ROT0,  "Zaccaria (Bally Midway license)", "Lazarian", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1982, catnmous, 0,        catnmous, catnmous, laserbat_state_base, laserbat, ROT90, "Zaccaria", "Cat and Mouse (set 1)",           MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE)
GAME( 1982, catnmousa,catnmous, catnmous, catnmous, laserbat_state_base, laserbat, ROT90, "Zaccaria", "Cat and Mouse (set 2)",           MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE)
