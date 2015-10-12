// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/********************************************************

  KO Punch (c) 1981 Sega

  XTAL: ?
  CPU: 8085 (proof: it uses SIM opcode)
  Other: 4 x i8255 for all I/O

  TODO:
  - discrete sound?
  - figure out sensors
  - coins sometimes don't register


*********************************************************

  This is a simple boxing bag game, but for visual feedback
  it has a small CRT instead of LEDs or a dial.

  Insert coin, select your weightclass (7 buttons on cab), and punch.

  Heavyweight   - 300K
  Middleweight  - 260K
  Welterweight  - 230K
  Lightweight   - 200K
  Featherweight - 170K
  Bantamweight  - 140K
  Flyweight     - 100K

********************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"
#include "includes/kopunch.h"


/********************************************************

  Interrupts

********************************************************/

INTERRUPT_GEN_MEMBER(kopunch_state::vblank_interrupt)
{
	device.execute().set_input_line(I8085_RST75_LINE, ASSERT_LINE);
	device.execute().set_input_line(I8085_RST75_LINE, CLEAR_LINE);
}

INPUT_CHANGED_MEMBER(kopunch_state::left_coin_inserted)
{
	// left coin insertion causes a rst6.5 (vector 0x34)
	if (newval)
		m_maincpu->set_input_line(I8085_RST65_LINE, HOLD_LINE);
}

INPUT_CHANGED_MEMBER(kopunch_state::right_coin_inserted)
{
	// right coin insertion causes a rst5.5 (vector 0x2c)
	if (newval)
		m_maincpu->set_input_line(I8085_RST55_LINE, HOLD_LINE);
}


/********************************************************

  Memory Maps

********************************************************/

static ADDRESS_MAP_START( kopunch_map, AS_PROGRAM, 8, kopunch_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
	AM_RANGE(0x6000, 0x63ff) AM_RAM_WRITE(vram_fg_w) AM_SHARE("vram_fg")
	AM_RANGE(0x7000, 0x70ff) AM_RAM_WRITE(vram_bg_w) AM_SHARE("vram_bg")
	AM_RANGE(0x7100, 0x73ff) AM_RAM // unused vram
	AM_RANGE(0x7400, 0x7bff) AM_RAM // more unused vram? or accidental writes?
ADDRESS_MAP_END

static ADDRESS_MAP_START( kopunch_io_map, AS_IO, 8, kopunch_state )
	AM_RANGE(0x30, 0x33) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0x34, 0x37) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
	AM_RANGE(0x38, 0x3b) AM_DEVREADWRITE("ppi8255_2", i8255_device, read, write)
	AM_RANGE(0x3c, 0x3f) AM_DEVREADWRITE("ppi8255_3", i8255_device, read, write)
ADDRESS_MAP_END



/********************************************************

  PPI I/O

********************************************************/

READ8_MEMBER(kopunch_state::sensors1_r)
{
	// punch strength low bits
	return machine().rand();
}

READ8_MEMBER(kopunch_state::sensors2_r)
{
	// d0-d2: punch strength high bits
	// d3: coin 2
	// d4: unknown sensor
	// d5: unknown sensor
	// d6: unknown sensor
	// d7: coin 1
	return (machine().rand() & 0x07) | ioport("SYSTEM")->read();
}

WRITE8_MEMBER(kopunch_state::lamp_w)
{
	set_led_status(machine(), 0, ~data & 0x80);
}

WRITE8_MEMBER(kopunch_state::coin_w)
{
	coin_counter_w(machine(), 0, ~data & 0x80);
	coin_counter_w(machine(), 1, ~data & 0x40);

//  if ((data & 0x3f) != 0x3e)
//      printf("port 34 = %02x   ",data);
}

/********************************************************

  Inputs

********************************************************/

static INPUT_PORTS_START( kopunch )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // related to above startbuttons

	PORT_START("SYSTEM")
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_SPECIAL ) // punch strength (high 3 bits)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, kopunch_state, right_coin_inserted, 0)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SPECIAL ) // sensor
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SPECIAL ) // sensor
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SPECIAL ) // sensor
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, kopunch_state, left_coin_inserted, 0)

	PORT_START("DSW")
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

	PORT_START("P2")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) // ? these 3 are read at the same time: p2>>4, and 7, <<1, read from a table
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // "
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // "
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )
INPUT_PORTS_END



/*******************************************************/

static const gfx_layout fg_layout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout bg_layout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 7,7, 6,6, 5,5, 4,4, 3,3, 2,2, 1,1, 0,0 },
	{ 0*8,0*8, 1*8,1*8, 2*8,2*8, 3*8,3*8, 4*8,4*8, 5*8,5*8, 6*8,6*8, 7*8,7*8 },
	8*8
};

static GFXDECODE_START( kopunch )
	GFXDECODE_ENTRY( "gfx1", 0, fg_layout, 0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, bg_layout, 0, 1 )
GFXDECODE_END


void kopunch_state::machine_start()
{
	// zerofill
	m_gfxbank = 0;
	m_scrollx = 0;

	// savestates
	save_item(NAME(m_gfxbank));
	save_item(NAME(m_scrollx));
}

static MACHINE_CONFIG_START( kopunch, kopunch_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8085A, 4000000) // 4 MHz?
	MCFG_CPU_PROGRAM_MAP(kopunch_map)
	MCFG_CPU_IO_MAP(kopunch_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", kopunch_state, vblank_interrupt)

	MCFG_DEVICE_ADD("ppi8255_0", I8255A, 0)
	// $30 - always $9b (PPI mode 0, ports A & B & C as input)
	MCFG_I8255_IN_PORTA_CB(IOPORT("P1"))
	MCFG_I8255_IN_PORTB_CB(READ8(kopunch_state, sensors1_r))
	MCFG_I8255_IN_PORTC_CB(READ8(kopunch_state, sensors2_r))

	MCFG_DEVICE_ADD("ppi8255_1", I8255A, 0)
	// $34 - always $80 (PPI mode 0, ports A & B & C as output)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(kopunch_state, coin_w))
	MCFG_I8255_OUT_PORTB_CB(LOGGER("PPI8255 - unmapped write port B", 0))
	MCFG_I8255_OUT_PORTC_CB(LOGGER("PPI8255 - unmapped write port C", 0))

	MCFG_DEVICE_ADD("ppi8255_2", I8255A, 0)
	// $38 - always $89 (PPI mode 0, ports A & B as output, port C as input)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(kopunch_state, lamp_w))
	MCFG_I8255_OUT_PORTB_CB(LOGGER("PPI8255 - unmapped write port B", 0))
	MCFG_I8255_IN_PORTC_CB(IOPORT("DSW"))

	MCFG_DEVICE_ADD("ppi8255_3", I8255A, 0)
	// $3c - always $88 (PPI mode 0, ports A & B & lower C as output, upper C as input)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(kopunch_state, scroll_x_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(kopunch_state, scroll_y_w))
	MCFG_I8255_IN_PORTC_CB(IOPORT("P2"))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(kopunch_state, gfxbank_w))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(kopunch_state, screen_update_kopunch)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", kopunch)
	MCFG_PALETTE_ADD("palette", 8)
	MCFG_PALETTE_INIT_OWNER(kopunch_state, kopunch)

	/* sound hardware */
	// ...
MACHINE_CONFIG_END



/********************************************************

  Game driver(s)

********************************************************/

ROM_START( kopunch )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr1105.x",    0x0000, 0x1000, CRC(34ef5e79) SHA1(2827c68f4c902f447a304d3ab0258c7819a0e4ca) )
	ROM_LOAD( "epr1106.x",    0x1000, 0x1000, CRC(25a5c68b) SHA1(9761418c6f3903f8aaceece658739fe5bf5c0803) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "epr1103",      0x0000, 0x0800, CRC(bae5e054) SHA1(95373123ab64543cdffb7ee9e02d0613c5c494bf) )
	ROM_LOAD( "epr1104",      0x0800, 0x0800, CRC(7b119a0e) SHA1(454f01355fa9512a7442990cc92da7bc7a8d6b68) )
	ROM_LOAD( "epr1102",      0x1000, 0x0800, CRC(8a52de96) SHA1(5abdaa83c6bfea81395cb190f5364b72811927ba) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "epr1107",      0x0000, 0x1000, CRC(ca00244d) SHA1(690931ea1bef9d80dcd7bd2ea2462b083c884a89) )
	ROM_LOAD( "epr1108",      0x1000, 0x1000, CRC(cc17c5ed) SHA1(693df076e16cc3a3dd54f6680691e658da3942fe) )
	ROM_LOAD( "epr1110",      0x2000, 0x1000, CRC(ae0aff15) SHA1(7f71c94bacdb444e5ed4f917c5a7de17012027a9) )
	ROM_LOAD( "epr1109",      0x3000, 0x1000, CRC(625446ba) SHA1(e4acedc8ddaf7e825930260d12601085ed89dced) )
	ROM_LOAD( "epr1112",      0x4000, 0x1000, CRC(ef6994df) SHA1(2d68650b6b875bcfdc9f977f96044c6867aa40a6) )
	ROM_LOAD( "epr1111",      0x5000, 0x1000, CRC(28530ec9) SHA1(1a8782d37128cdb43133fc891cde93d2bdd5476b) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "epr1101",      0x0000, 0x0020, CRC(15600f5d) SHA1(130179f79761cb16316c544e3c689bc10431db30) ) /* palette */
	ROM_LOAD( "epr1099",      0x0020, 0x0020, CRC(fc58c456) SHA1(f27c3ad669dfdc33bcd7e0481fa01bf34973e816) ) /* unknown */
	ROM_LOAD( "epr1100",      0x0040, 0x0020, CRC(bedb66b1) SHA1(8e78bb205d900075b761e1baa5f5813174ff28ba) ) /* unknown */
ROM_END


GAME( 1981, kopunch, 0, kopunch, kopunch, driver_device, 0, ROT270, "Sega", "KO Punch", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
