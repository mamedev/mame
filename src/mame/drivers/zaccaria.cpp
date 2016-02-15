// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Jack Rabbit memory map (preliminary)

driver by Nicola Salmoria
thanks to Andrea Babich for the manual.

TODO:

- implement discrete filters for analog signals 1 to 5 and attenuation control
  for signal 5 (74LS156)

- The 8910 outputs go through some analog circuitry to make them sound more like
  real intruments.
  #0 Ch. A = "rullante"/"cassa" (drum roll/bass drum) (selected by bits 3&4 of port A)
  #0 Ch. B = "basso" (bass)
  #0 Ch. C = straight out through an optional filter
  #1 Ch. A = "piano"
  #1 Ch. B = "tromba" (trumpet) (level selected by bit 0 of port A)
  #1 Ch. C = disabled (there's an open jumper, otherwise would go out through a filter)

- some minor color issues (see video)


Notes:
- There is a protection device which I haven't located on the schematics. It
  sits on bits 4-7 of the data bus, and is read from locations where only bits
  0-3 are connected to regular devices (6400-6407 has 4-bit RAM, while 6c00-6c07
  has a 4-bit input port).

- The 6802 driving the TMS5220 has a push button connected to the NMI line. On
  Zaccaria pinballs, when pressed, this causes the speech 6802 and the slave
  sound 6802 to play a few speech effects and sound effects;
  This currently doesn't work in MAME, and tracing the code it seems the code is
  possibly broken, made up of nonfunctional leftovers of some of the pinball test
  mode code.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "includes/zaccaria.h"


void zaccaria_state::machine_start()
{
	save_item(NAME(m_dsw_sel));
	save_item(NAME(m_nmi_mask));
}

void zaccaria_state::machine_reset()
{
	m_dsw_sel = 0;
	m_nmi_mask = 0;
}

WRITE8_MEMBER(zaccaria_state::dsw_sel_w)
{
	switch (data & 0xf0)
	{
	case 0xe0:
		m_dsw_sel = 0;
		break;

	case 0xd0:
		m_dsw_sel = 1;
		break;

	case 0xb0:
		m_dsw_sel = 2;
		break;

	default:
		logerror("%s: portsel = %02x\n", machine().describe_context(), data);
		break;
	}
}

READ8_MEMBER(zaccaria_state::dsw_r)
{
	return m_dsw_port[m_dsw_sel]->read();
}


GAME_EXTERN(monymony);

READ8_MEMBER(zaccaria_state::prot1_r)
{
	switch (offset)
	{
		case 0:
			return 0x50;    /* Money Money */

		case 4:
			return 0x40;    /* Jack Rabbit */

		case 6:
			if (&machine().system() == &GAME_NAME(monymony))
				return 0x70;    /* Money Money */
			return 0xa0;    /* Jack Rabbit */

		default:
			return 0;
	}
}

READ8_MEMBER(zaccaria_state::prot2_r)
{
	switch (offset)
	{
		case 0:
			return ioport("COINS")->read();   /* bits 4 and 5 must be 0 in Jack Rabbit */

		case 2:
			return 0x10;    /* Jack Rabbit */

		case 4:
			return 0x80;    /* Money Money */

		case 6:
			return 0x00;    /* Money Money */

		default:
			return 0;
	}
}


WRITE8_MEMBER(zaccaria_state::coin_w)
{
	machine().bookkeeping().coin_counter_w(0,data & 1);
}

WRITE8_MEMBER(zaccaria_state::nmi_mask_w)
{
	m_nmi_mask = data & 1;
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, zaccaria_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x63ff) AM_READONLY
	AM_RANGE(0x6400, 0x6407) AM_READ(prot1_r)
	AM_RANGE(0x6000, 0x67ff) AM_WRITE(videoram_w) AM_SHARE("videoram") /* 6400-67ff is 4 bits wide */
	AM_RANGE(0x6800, 0x683f) AM_WRITE(attributes_w) AM_SHARE("attributesram")
	AM_RANGE(0x6840, 0x685f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x6881, 0x68c0) AM_RAM AM_SHARE("spriteram2")
	AM_RANGE(0x6c00, 0x6c00) AM_MIRROR(0x81f8) AM_WRITE(flip_screen_x_w)
	AM_RANGE(0x6c01, 0x6c01) AM_MIRROR(0x81f8) AM_WRITE(flip_screen_y_w)
	AM_RANGE(0x6c02, 0x6c02) AM_MIRROR(0x81f8) AM_WRITE(ressound_w)
	AM_RANGE(0x6c06, 0x6c06) AM_MIRROR(0x81f8) AM_WRITE(coin_w)
	AM_RANGE(0x6c07, 0x6c07) AM_MIRROR(0x81f8) AM_WRITE(nmi_mask_w)
	AM_RANGE(0x6c00, 0x6c07) AM_MIRROR(0x81f8) AM_READ(prot2_r)
	AM_RANGE(0x6e00, 0x6e00) AM_MIRROR(0x81f8) AM_READ(dsw_r) AM_DEVWRITE("audiopcb", zac1b11142_audio_device, hs_w)
	AM_RANGE(0x7000, 0x77ff) AM_RAM
	AM_RANGE(0x7800, 0x7803) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
	AM_RANGE(0x7c00, 0x7c00) AM_READ(watchdog_reset_r)
	AM_RANGE(0x8000, 0xdfff) AM_ROM
ADDRESS_MAP_END


WRITE8_MEMBER(zaccaria_state::ressound_w)
{
	m_audiopcb->ressound_w(data & 0x01);
}

static INPUT_PORTS_START( monymony )
	PORT_START("DSW.0")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW 5I:1,2")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME( 0x04, 0x00, "Infinite Lives (Cheat)")     PORT_DIPLOCATION("SW 5I:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW 5I:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW 5I:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x00, "Freeze" )                    PORT_DIPLOCATION("SW 5I:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Cross Hatch Pattern" )       PORT_DIPLOCATION("SW 5I:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW 5I:8") /* random high scores? */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW.1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW 4I:1,2")
	PORT_DIPSETTING(    0x01, "200000" )
	PORT_DIPSETTING(    0x02, "300000" )
	PORT_DIPSETTING(    0x03, "400000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x04, 0x00, "Table Title" )               PORT_DIPLOCATION("SW 4I:3")
	PORT_DIPSETTING(    0x00, "Todays High Scores" )
	PORT_DIPSETTING(    0x04, "High Scores" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )           PORT_DIPLOCATION("SW 4I:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) )           PORT_DIPLOCATION("SW 4I:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) )           PORT_DIPLOCATION("SW 4I:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )           PORT_DIPLOCATION("SW 4I:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_HIGH )                    PORT_DIPLOCATION("SW 4I:8")

	PORT_START("DSW.2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW 3I:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x8c, 0x84, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW 3I:3,4,5")
	PORT_DIPSETTING(    0x8c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x88, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x84, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x70, 0x50, "Coin C" )                    PORT_DIPLOCATION("SW 3I:6,7,8")
	PORT_DIPSETTING(    0x70, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_7C ) )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	/* other bits are outputs */

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("audiopcb", zac1b11142_audio_device, acs_r)
	/* other bits come from a protection device */
INPUT_PORTS_END

static INPUT_PORTS_START( jackrabt )
	PORT_INCLUDE( monymony )

	PORT_MODIFY("DSW.0")
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW 5I:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW.1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW 4I:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW 4I:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Table Title" )               PORT_DIPLOCATION("SW 4I:3")
	PORT_DIPSETTING(    0x00, "Todays High Scores" )
	PORT_DIPSETTING(    0x04, "High Scores" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW 4I:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW 4I:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW 4I:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW 4I:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};

static GFXDECODE_START( zaccaria )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x3_planar, 0, 32 )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout, 32*8, 32 )
GFXDECODE_END


INTERRUPT_GEN_MEMBER(zaccaria_state::vblank_irq)
{
	if(m_nmi_mask)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}


static MACHINE_CONFIG_START( zaccaria, zaccaria_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,XTAL_18_432MHz/6)   /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", zaccaria_state,  vblank_irq)
//  MCFG_QUANTUM_TIME(attotime::from_hz(1000000))

	MCFG_DEVICE_ADD("ppi8255", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("P1"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("P2"))
	MCFG_I8255_IN_PORTC_CB(IOPORT("SYSTEM"))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(zaccaria_state, dsw_sel_w))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60.57) /* verified on pcb */
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(zaccaria_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", zaccaria)
	MCFG_PALETTE_ADD("palette", 32*8+32*8)
	MCFG_PALETTE_INDIRECT_ENTRIES(512)
	MCFG_PALETTE_INIT_OWNER(zaccaria_state, zaccaria)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_ZACCARIA_1B11142("audiopcb")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( monymony )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cpu1.1a",           0x0000, 0x1000, CRC(13c227ca) SHA1(be305d112917904dd130b08f6b5186e3fbcb858a) )
	ROM_CONTINUE(             0x8000, 0x1000 )
	ROM_LOAD( "cpu2.1b",           0x1000, 0x1000, CRC(87372545) SHA1(04618d007a93b3f6706f56b10bdf39727d7d748d) )
	ROM_CONTINUE(             0x9000, 0x1000 )
	ROM_LOAD( "cpu3.1c",           0x2000, 0x1000, CRC(6aea9c01) SHA1(36a57f4dfae52d674dcf55d2b93dbacf734866b1) )
	ROM_CONTINUE(             0xa000, 0x1000 )
	ROM_LOAD( "cpu4.1d",           0x3000, 0x1000, CRC(5fdec451) SHA1(0f955c907e0a61a725a951018fdf5cc321139863) )
	ROM_CONTINUE(             0xb000, 0x1000 )
	ROM_LOAD( "cpu5.2a",           0x4000, 0x1000, CRC(af830e3c) SHA1(bed57c341ae3500f147efe31bcf01f81466ec1c0) )
	ROM_CONTINUE(             0xc000, 0x1000 )
	ROM_LOAD( "cpu6.2c",           0x5000, 0x1000, CRC(31da62b1) SHA1(486f07087244f8537510afacb64ddd59eb512a4d) )
	ROM_CONTINUE(             0xd000, 0x1000 )

	ROM_REGION( 0x10000, "audiopcb:melodycpu", 0 ) /* 64k for first 6802 */
	ROM_LOAD( "snd13.2g",           0x8000, 0x2000, CRC(78b01b98) SHA1(2aabed56cdae9463deb513c0c5021f6c8dfd271e) )
	ROM_LOAD( "snd9.1i",           0xc000, 0x2000, CRC(94e3858b) SHA1(04961f67b95798b530bd83355dec612389f22255) )

	ROM_REGION( 0x10000, "audiopcb:audiocpu", 0 ) /* 64k for second 6802 */
	ROM_LOAD( "snd8.1h",           0x2000, 0x1000, CRC(aad76193) SHA1(e08fc184efced392ee902c4cc9daaaf3310cdfe2) )
	ROM_CONTINUE(             0x6000, 0x1000 )
	ROM_LOAD( "snd7.1g",           0x3000, 0x1000, CRC(1e8ffe3e) SHA1(858ee7abe88d5801237e519cae2b50ae4bf33a58) )
	ROM_CONTINUE(             0x7000, 0x1000 )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "bg1.2d",           0x0000, 0x2000, CRC(82ab4d1a) SHA1(5aaf42a508df236f2e7c844d377132d73053907b) )
	ROM_LOAD( "bg2.1f",           0x2000, 0x2000, CRC(40d4e4d1) SHA1(79cbade30f1c9269e70ddb9c4332cfe1e8dc50a9) )
	ROM_LOAD( "bg3.1e",           0x4000, 0x2000, CRC(36980455) SHA1(4140b0cd4137c8f209124b12d9c0eb3b04f91991) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "9g",  0x0000, 0x0200, CRC(fc9a0f21) SHA1(2a93d684645ee1b70315386127223151582ab370) )
	ROM_LOAD( "9f",  0x0200, 0x0200, CRC(93106704) SHA1(d3b8281c87d253a2ed40ff400438e879ca40c2b7) )
ROM_END

ROM_START( jackrabt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cpu-01.1a",    0x0000, 0x1000, CRC(499efe97) SHA1(f0efc910a5343001b27637779e1d4de218d44a4e) )
	ROM_CONTINUE(             0x8000, 0x1000 )
	ROM_LOAD( "cpu-01.2l",    0x1000, 0x1000, CRC(4772e557) SHA1(71c1eb49c978799294e732e65a77eba330d8da9b) )
	ROM_LOAD( "cpu-01.3l",    0x2000, 0x1000, CRC(1e844228) SHA1(0525fe95a0f90c50b54c0bf618eb083ccf20e6c4) )
	ROM_LOAD( "cpu-01.4l",    0x3000, 0x1000, CRC(ebffcc38) SHA1(abaf0e96d92f9c828a95446af6d5301053416f3d) )
	ROM_LOAD( "cpu-01.5l",    0x4000, 0x1000, CRC(275e0ed6) SHA1(c0789007a4de1aa848b7e5d26cf9fe847cc5d8a4) )
	ROM_LOAD( "cpu-01.6l",    0x5000, 0x1000, CRC(8a20977a) SHA1(ba15f4c62f600372390e56c2067b4a8ab1f2dba9) )
	ROM_LOAD( "cpu-01.2h",    0x9000, 0x1000, CRC(21f2be2a) SHA1(7d10489ca7325eebfa309ae4ffd4962a4310c403) )
	ROM_LOAD( "cpu-01.3h",    0xa000, 0x1000, CRC(59077027) SHA1(d6c2e68b4b2f1dce8a2141ec259812e732c1c69c) )
	ROM_LOAD( "cpu-01.4h",    0xb000, 0x1000, CRC(0b9db007) SHA1(836f8cacf2a097fd80d5c045bdc49b3a3174b89e) )
	ROM_LOAD( "cpu-01.5h",    0xc000, 0x1000, CRC(785e1a01) SHA1(a748d300be9455cad4f912e01c2279bb8465edfe) )
	ROM_LOAD( "cpu-01.6h",    0xd000, 0x1000, CRC(dd5979cf) SHA1(e9afe7002b2258a1c3132bdd951c6e20d473fb6a) )

	ROM_REGION( 0x10000, "audiopcb:melodycpu", 0 ) /* 64k for first 6802 */
	ROM_LOAD( "13snd.2g",     0x8000, 0x2000, CRC(fc05654e) SHA1(ed9c66672fe89c41e320e1d27b53f5efa92dce9c) )
	ROM_LOAD( "9snd.1i",      0xc000, 0x2000, CRC(3dab977f) SHA1(3e79c06d2e70b050f01b7ac58be5127ba87904b0) )

	ROM_REGION( 0x10000, "audiopcb:audiocpu", 0 ) /* 64k for second 6802 */
	ROM_LOAD( "8snd.1h",      0x2000, 0x1000, CRC(f4507111) SHA1(0513f0831b94aeda84aa4f3b4a7c60dfc5113b2d) )
	ROM_CONTINUE(             0x6000, 0x1000 )
	ROM_LOAD( "7snd.1g",      0x3000, 0x1000, CRC(c722eff8) SHA1(d8d1c091ab80ea2d6616e4dc030adc9905c0a496) )
	ROM_CONTINUE(             0x7000, 0x1000 )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "1bg.2d",       0x0000, 0x2000, CRC(9f880ef5) SHA1(0ee20fb7c794f6dafdaf2c9ee8456221c9d668c5) )
	ROM_LOAD( "2bg.1f",       0x2000, 0x2000, CRC(afc04cd7) SHA1(f4349e86b9caee71c9bf9faf68b86603417d9a2b) )
	ROM_LOAD( "3bg.1e",       0x4000, 0x2000, CRC(14f23cdd) SHA1(e5f3dac52288c56f2fd2940b397bb6c896131a26) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "jr-ic9g",      0x0000, 0x0200, CRC(85577107) SHA1(76575fa68b66130b18dfe7374d1a03740963cc73) )
	ROM_LOAD( "jr-ic9f",      0x0200, 0x0200, CRC(085914d1) SHA1(3d6f9318f5a9f08ce89e4184e3efb9881f671fa7) )

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "jr-pal16l8.6j",   0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "jr-pal16l8.6k",   0x0200, 0x0104, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( jackrabt2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1cpu2.1a",     0x0000, 0x1000, CRC(f9374113) SHA1(521f293f1894bcaf21e44bc7841a20ae29232da3) )
	ROM_CONTINUE(             0x8000, 0x1000 )
	ROM_LOAD( "2cpu2.1b",     0x1000, 0x1000, CRC(0a0eea4a) SHA1(4dfd9b2511d480bb5cc918f7d91013205911d377) )
	ROM_CONTINUE(             0x9000, 0x1000 )
	ROM_LOAD( "3cpu2.1c",     0x2000, 0x1000, CRC(291f5772) SHA1(958c2601d43de3c95ed5e3d79737199703263a6a) )
	ROM_CONTINUE(             0xa000, 0x1000 )
	ROM_LOAD( "4cpu2.1d",     0x3000, 0x1000, CRC(10972cfb) SHA1(30dd473b3416ee37f887d930ba0017b5b694398e) )
	ROM_CONTINUE(             0xb000, 0x1000 )
	ROM_LOAD( "5cpu2.2a",     0x4000, 0x1000, CRC(aa95d06d) SHA1(2216effe6cacd02a5320e71a85842087dda5f85a) )
	ROM_CONTINUE(             0xc000, 0x1000 )
	ROM_LOAD( "6cpu2.2c",     0x5000, 0x1000, CRC(404496eb) SHA1(44381e27e540fe9d8cacab4c3b1fe9a4f20d26a8) )
	ROM_CONTINUE(             0xd000, 0x1000 )

	ROM_REGION( 0x10000, "audiopcb:melodycpu", 0 ) /* 64k for first 6802 */
	ROM_LOAD( "13snd.2g",     0x8000, 0x2000, CRC(fc05654e) SHA1(ed9c66672fe89c41e320e1d27b53f5efa92dce9c) )
	ROM_LOAD( "9snd.1i",      0xc000, 0x2000, CRC(3dab977f) SHA1(3e79c06d2e70b050f01b7ac58be5127ba87904b0) )

	ROM_REGION( 0x10000, "audiopcb:audiocpu", 0 ) /* 64k for second 6802 */
	ROM_LOAD( "8snd.1h",      0x2000, 0x1000, CRC(f4507111) SHA1(0513f0831b94aeda84aa4f3b4a7c60dfc5113b2d) )
	ROM_CONTINUE(             0x6000, 0x1000 )
	ROM_LOAD( "7snd.1g",      0x3000, 0x1000, CRC(c722eff8) SHA1(d8d1c091ab80ea2d6616e4dc030adc9905c0a496) )
	ROM_CONTINUE(             0x7000, 0x1000 )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "1bg.2d",       0x0000, 0x2000, CRC(9f880ef5) SHA1(0ee20fb7c794f6dafdaf2c9ee8456221c9d668c5) )
	ROM_LOAD( "2bg.1f",       0x2000, 0x2000, CRC(afc04cd7) SHA1(f4349e86b9caee71c9bf9faf68b86603417d9a2b) )
	ROM_LOAD( "3bg.1e",       0x4000, 0x2000, CRC(14f23cdd) SHA1(e5f3dac52288c56f2fd2940b397bb6c896131a26) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "jr-ic9g",      0x0000, 0x0200, CRC(85577107) SHA1(76575fa68b66130b18dfe7374d1a03740963cc73) )
	ROM_LOAD( "jr-ic9f",      0x0200, 0x0200, CRC(085914d1) SHA1(3d6f9318f5a9f08ce89e4184e3efb9881f671fa7) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal16l8.6j",   0x0000, 0x0104, CRC(a88e52d6) SHA1(32efecb91843d5d1bdace86cbcc94ebacf1b9389) )
	ROM_LOAD( "pal16l8.6k",   0x0200, 0x0104, NO_DUMP )
	ROM_LOAD( "82s100.8c",    0x0400, 0x00f5, CRC(70ddfa6d) SHA1(904347cc63e88413c393f14b5f1260a57ab72677) )
	ROM_LOAD( "82s100.8n",    0x0500, 0x00f5, CRC(e00625ee) SHA1(88bbd020be67355dc0eb58b79f7deb77cbe505bb) )
ROM_END

ROM_START( jackrabts )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1cpu.1a",      0x0000, 0x1000, CRC(6698dc65) SHA1(33e3518846e88dc34f4b6c4e9ca9f8999c0460c8) )
	ROM_CONTINUE(             0x8000, 0x1000 )
	ROM_LOAD( "2cpu.1b",      0x1000, 0x1000, CRC(42b32929) SHA1(5b400d434ce903c74f58780a422a8c2594af90be) )
	ROM_CONTINUE(             0x9000, 0x1000 )
	ROM_LOAD( "3cpu.1c",      0x2000, 0x1000, CRC(89b50c9a) SHA1(5ab56247de013b5196c1c5765ead4361a5df53e0) )
	ROM_CONTINUE(             0xa000, 0x1000 )
	ROM_LOAD( "4cpu.1d",      0x3000, 0x1000, CRC(d5520665) SHA1(69b34d87d50e6d6e8d365ba0479405380ba3cf11) )
	ROM_CONTINUE(             0xb000, 0x1000 )
	ROM_LOAD( "5cpu.2a",      0x4000, 0x1000, CRC(0f9a093c) SHA1(7fba0d2b8d5d4d1597decec96ed93b997c721d99) )
	ROM_CONTINUE(             0xc000, 0x1000 )
	ROM_LOAD( "6cpu.2c",      0x5000, 0x1000, CRC(f53d6356) SHA1(9b167edca59cf81a2468368a372bab132f15e2ea) )
	ROM_CONTINUE(             0xd000, 0x1000 )

	ROM_REGION( 0x10000, "audiopcb:melodycpu", 0 ) /* 64k for first 6802 */
	ROM_LOAD( "13snd.2g",     0x8000, 0x2000, CRC(fc05654e) SHA1(ed9c66672fe89c41e320e1d27b53f5efa92dce9c) )
	ROM_LOAD( "9snd.1i",      0xc000, 0x2000, CRC(3dab977f) SHA1(3e79c06d2e70b050f01b7ac58be5127ba87904b0) )

	ROM_REGION( 0x10000, "audiopcb:audiocpu", 0 ) /* 64k for second 6802 */
	ROM_LOAD( "8snd.1h",      0x2000, 0x1000, CRC(f4507111) SHA1(0513f0831b94aeda84aa4f3b4a7c60dfc5113b2d) )
	ROM_CONTINUE(             0x6000, 0x1000 )
	ROM_LOAD( "7snd.1g",      0x3000, 0x1000, CRC(c722eff8) SHA1(d8d1c091ab80ea2d6616e4dc030adc9905c0a496) )
	ROM_CONTINUE(             0x7000, 0x1000 )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "1bg.2d",       0x0000, 0x2000, CRC(9f880ef5) SHA1(0ee20fb7c794f6dafdaf2c9ee8456221c9d668c5) )
	ROM_LOAD( "2bg.1f",       0x2000, 0x2000, CRC(afc04cd7) SHA1(f4349e86b9caee71c9bf9faf68b86603417d9a2b) )
	ROM_LOAD( "3bg.1e",       0x4000, 0x2000, CRC(14f23cdd) SHA1(e5f3dac52288c56f2fd2940b397bb6c896131a26) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "jr-ic9g",      0x0000, 0x0200, CRC(85577107) SHA1(76575fa68b66130b18dfe7374d1a03740963cc73) )
	ROM_LOAD( "jr-ic9f",      0x0200, 0x0200, CRC(085914d1) SHA1(3d6f9318f5a9f08ce89e4184e3efb9881f671fa7) )
ROM_END



GAME( 1983, monymony,  0,        zaccaria, monymony, driver_device, 0, ROT90, "Zaccaria", "Money Money", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, jackrabt,  0,        zaccaria, jackrabt, driver_device, 0, ROT90, "Zaccaria", "Jack Rabbit (set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, jackrabt2, jackrabt, zaccaria, jackrabt, driver_device, 0, ROT90, "Zaccaria", "Jack Rabbit (set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, jackrabts, jackrabt, zaccaria, jackrabt, driver_device, 0, ROT90, "Zaccaria", "Jack Rabbit (special)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
