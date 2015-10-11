// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Taxi Driver  (c) 1984 Graphic Techno

XTAL:  10MHz, 8MHz
CPU:   3 * NEC D780C (clocks unknown)
SOUND: 2 * AY-3-8910 (clocks unknown)
OTHER: 5 * M5L8255AP

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "includes/taxidriv.h"
#include "sound/ay8910.h"


void taxidriv_state::machine_start()
{
	save_item(NAME(m_s1));
	save_item(NAME(m_s2));
	save_item(NAME(m_s3));
	save_item(NAME(m_s4));
	save_item(NAME(m_latchA));
	save_item(NAME(m_latchB));
	save_item(NAME(m_bghide));
	save_item(NAME(m_spritectrl));
}

WRITE8_MEMBER(taxidriv_state::p2a_w){ spritectrl_w(space,0,data); }
WRITE8_MEMBER(taxidriv_state::p2b_w){ spritectrl_w(space,1,data); }
WRITE8_MEMBER(taxidriv_state::p2c_w){ spritectrl_w(space,2,data); }
WRITE8_MEMBER(taxidriv_state::p3a_w){ spritectrl_w(space,3,data); }
WRITE8_MEMBER(taxidriv_state::p3b_w){ spritectrl_w(space,4,data); }
WRITE8_MEMBER(taxidriv_state::p3c_w){ spritectrl_w(space,5,data); }
WRITE8_MEMBER(taxidriv_state::p4a_w){ spritectrl_w(space,6,data); }
WRITE8_MEMBER(taxidriv_state::p4b_w){ spritectrl_w(space,7,data); }
WRITE8_MEMBER(taxidriv_state::p4c_w){ spritectrl_w(space,8,data); }


READ8_MEMBER(taxidriv_state::p0a_r)
{
	return m_latchA;
}

READ8_MEMBER(taxidriv_state::p0c_r)
{
	return (m_s1 << 7);
}

WRITE8_MEMBER(taxidriv_state::p0b_w)
{
	m_latchB = data;
}

WRITE8_MEMBER(taxidriv_state::p0c_w)
{
	m_s2 = data & 1;

	m_bghide = data & 2;

	/* bit 2 toggles during gameplay */

	flip_screen_set(data & 8);

//  popmessage("%02x",data&0x0f);
}

READ8_MEMBER(taxidriv_state::p1b_r)
{
	return m_latchB;
}

READ8_MEMBER(taxidriv_state::p1c_r)
{
	return (m_s2 << 7) | (m_s4 << 6) | ((ioport("SERVCOIN")->read() & 1) << 4);
}

WRITE8_MEMBER(taxidriv_state::p1a_w)
{
	m_latchA = data;
}

WRITE8_MEMBER(taxidriv_state::p1c_w)
{
	m_s1 = data & 1;
	m_s3 = (data & 2) >> 1;
}

READ8_MEMBER(taxidriv_state::p8910_0a_r)
{
	return m_latchA;
}

READ8_MEMBER(taxidriv_state::p8910_1a_r)
{
	return m_s3;
}

/* note that a lot of writes happen with port B set as input. I think this is a bug in the
   original, since it works anyway even if the communication is flawed. */
WRITE8_MEMBER(taxidriv_state::p8910_0b_w)
{
	m_s4 = data & 1;
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, taxidriv_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8fff) AM_RAM /* ??? */
	AM_RANGE(0x9000, 0x9fff) AM_RAM /* ??? */
	AM_RANGE(0xa000, 0xafff) AM_RAM /* ??? */
	AM_RANGE(0xb000, 0xbfff) AM_RAM /* ??? */
	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_SHARE("vram4")           /* radar bitmap */
	AM_RANGE(0xc800, 0xcfff) AM_WRITEONLY AM_SHARE("vram5") /* "sprite1" bitmap */
	AM_RANGE(0xd000, 0xd7ff) AM_WRITEONLY AM_SHARE("vram6") /* "sprite2" bitmap */
	AM_RANGE(0xd800, 0xdfff) AM_RAM AM_SHARE("vram7")           /* "sprite3" bitmap */
	AM_RANGE(0xe000, 0xf3ff) AM_READONLY
	AM_RANGE(0xe000, 0xe3ff) AM_WRITEONLY AM_SHARE("vram1") /* car tilemap */
	AM_RANGE(0xe400, 0xebff) AM_WRITEONLY AM_SHARE("vram2") /* bg1 tilemap */
	AM_RANGE(0xec00, 0xefff) AM_WRITEONLY AM_SHARE("vram0") /* fg tilemap */
	AM_RANGE(0xf000, 0xf3ff) AM_WRITEONLY AM_SHARE("vram3") /* bg2 tilemap */
	AM_RANGE(0xf400, 0xf403) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0xf480, 0xf483) AM_DEVREADWRITE("ppi8255_2", i8255_device, read, write)    /* "sprite1" placement */
	AM_RANGE(0xf500, 0xf503) AM_DEVREADWRITE("ppi8255_3", i8255_device, read, write)    /* "sprite2" placement */
	AM_RANGE(0xf580, 0xf583) AM_DEVREADWRITE("ppi8255_4", i8255_device, read, write)    /* "sprite3" placement */
	//AM_RANGE(0xf780, 0xf781) AM_WRITEONLY     /* more scroll registers? */
	AM_RANGE(0xf782, 0xf787) AM_WRITEONLY AM_SHARE("scroll")    /* bg scroll (three copies always identical) */
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpu2_map, AS_PROGRAM, 8, taxidriv_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x6000, 0x67ff) AM_RAM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0xa000, 0xa003) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
	AM_RANGE(0xe000, 0xe000) AM_READ_PORT("DSW0")
	AM_RANGE(0xe001, 0xe001) AM_READ_PORT("DSW1")
	AM_RANGE(0xe002, 0xe002) AM_READ_PORT("DSW2")
	AM_RANGE(0xe003, 0xe003) AM_READ_PORT("P1")
	AM_RANGE(0xe004, 0xe004) AM_READ_PORT("P2")
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpu3_map, AS_PROGRAM, 8, taxidriv_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x2000) AM_READNOP /* irq ack? */
	AM_RANGE(0xfc00, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpu3_port_map, AS_IO, 8, taxidriv_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0x01, 0x01) AM_DEVREAD("ay1", ay8910_device, data_r)
	AM_RANGE(0x02, 0x03) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
	AM_RANGE(0x03, 0x03) AM_DEVREAD("ay2", ay8910_device, data_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( taxidriv )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 4C_2C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 4C_2C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( Free_Play ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "255 (Cheat)")
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x18, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x28, "6" )
	PORT_DIPSETTING(    0x30, "7" )
	PORT_DIPSETTING(    0x38, "8" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "Fuel Consumption" )
	PORT_DIPSETTING(    0x00, "Slowest" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x05, "6" )
	PORT_DIPSETTING(    0x06, "7" )
	PORT_DIPSETTING(    0x07, "Fastest" )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x18, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x28, "6" )
	PORT_DIPSETTING(    0x30, "7" )
	PORT_DIPSETTING(    0x38, "8" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "40/30" )
	PORT_DIPSETTING(    0x40, "30/20" )
	PORT_DIPSETTING(    0x80, "20/15" )
	PORT_DIPSETTING(    0xc0, "10/10" )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL

	PORT_START("SERVCOIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )   /* handled by p1c_r() */
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout charlayout2 =
{
	4,4,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 3*4, 2*4 },
	{ 0*16, 1*16, 2*16, 3*16 },
	16*4
};


static GFXDECODE_START( taxidriv )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout, 0, 1 )
	GFXDECODE_ENTRY( "gfx3", 0, charlayout, 0, 1 )
	GFXDECODE_ENTRY( "gfx4", 0, charlayout, 0, 1 )
	GFXDECODE_ENTRY( "gfx5", 0, charlayout2, 0, 1 )
GFXDECODE_END

PALETTE_INIT_MEMBER(taxidriv_state, taxidriv)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int bit0, bit1, r, g, b;
	int i;

	/* TODO: resistors, 1k & 470*/

	for (i = 0; i < 0x10; ++i)
	{
		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		r = 0x55 * bit0 + 0xaa * bit1;
		bit0 = (color_prom[0] >> 2) & 0x01;
		bit1 = (color_prom[0] >> 3) & 0x01;
		g = 0x55 * bit0 + 0xaa * bit1;
		bit0 = (color_prom[0] >> 4) & 0x01;
		bit1 = (color_prom[0] >> 5) & 0x01;
		b = 0x55 * bit0 + 0xaa * bit1;

		palette.set_pen_color(i, rgb_t(r, g, b));
		color_prom++;
	}
}

static MACHINE_CONFIG_START( taxidriv, taxidriv_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,4000000)    /* 4 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", taxidriv_state,  irq0_line_hold)

	MCFG_CPU_ADD("sub", Z80,4000000)    /* 4 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(cpu2_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", taxidriv_state,  irq0_line_hold)   /* ??? */

	MCFG_CPU_ADD("audiocpu", Z80,4000000)   /* 4 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(cpu3_map)
	MCFG_CPU_IO_MAP(cpu3_port_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", taxidriv_state,  irq0_line_hold)   /* ??? */

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))  /* 100 CPU slices per frame - an high value to ensure proper */
							/* synchronization of the CPUs */

	MCFG_DEVICE_ADD("ppi8255_0", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(taxidriv_state, p0a_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(taxidriv_state, p0b_w))
	MCFG_I8255_IN_PORTC_CB(READ8(taxidriv_state, p0c_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(taxidriv_state, p0c_w))

	MCFG_DEVICE_ADD("ppi8255_1", I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(taxidriv_state, p1a_w))
	MCFG_I8255_IN_PORTB_CB(READ8(taxidriv_state, p1b_r))
	MCFG_I8255_IN_PORTC_CB(READ8(taxidriv_state, p1c_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(taxidriv_state, p1c_w))

	MCFG_DEVICE_ADD("ppi8255_2", I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(taxidriv_state, p2a_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(taxidriv_state, p2b_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(taxidriv_state, p2c_w))

	MCFG_DEVICE_ADD("ppi8255_3", I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(taxidriv_state, p3a_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(taxidriv_state, p3b_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(taxidriv_state, p3c_w))

	MCFG_DEVICE_ADD("ppi8255_4", I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(taxidriv_state, p4a_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(taxidriv_state, p4b_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(taxidriv_state, p4c_w))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 27*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(taxidriv_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_ORIENTATION(ROT90)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", taxidriv)
	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_INIT_OWNER(taxidriv_state, taxidriv)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, 1250000)
	MCFG_AY8910_PORT_A_READ_CB(READ8(taxidriv_state, p8910_0a_r))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(taxidriv_state, p8910_0b_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay2", AY8910, 1250000)
	MCFG_AY8910_PORT_A_READ_CB(READ8(taxidriv_state, p8910_1a_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( taxidriv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.ic87",       0x0000, 0x2000, CRC(6b2424e9) SHA1(a65bb01da8f3b0649d945981cc4f1324b7fac5c7) )
	ROM_LOAD( "2.ic86",       0x2000, 0x2000, CRC(15111229) SHA1(0350918f9504b0e470684ebc94a823bb2513a54d) )
	ROM_LOAD( "3.ic85",       0x4000, 0x2000, CRC(a7782eee) SHA1(0f10b7876420f4237937b1b922aa410de3f79af1) )
	ROM_LOAD( "4.ic84",       0x6000, 0x2000, CRC(8eb0b16b) SHA1(a0015744373ee91bc505f077a04ab3546f8bb6fb) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "8.ic4",        0x0000, 0x2000, CRC(9f9a3865) SHA1(908cf4f2cc68c088649241997276ea25c27d9718) )
//  ROM_LOAD( "8.ic4",        0x0000, 0x2000, CRC(9835d517) SHA1(845f3efc54b64837c22dd06683c2950f2b8b03cb) ) // 0x1b5f = 0x04 instead of 0x03 from another set
	ROM_LOAD( "9.ic5",        0x2000, 0x2000, CRC(b28b766c) SHA1(21e08ef1e2671c8540380e3fa0858e8a4d821945) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "7.ic14",       0x0000, 0x2000, CRC(2b4cbfe6) SHA1(a2a900831116554d5aea1a81c93245d3bb424d48) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5.m.ic68",     0x0000, 0x2000, CRC(a3aa5f2f) SHA1(7e046e2a5d230c62d93a83f5a773e6e4d6e85961) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "6.1.ic35",     0x0000, 0x2000, CRC(bfddd550) SHA1(f528c2701c635bc61eda14fbe2cfe9b44cb75c20) )

	ROM_REGION( 0x6000, "gfx3", 0 )
	ROM_LOAD( "11.30.ic87",   0x0000, 0x2000, CRC(7485eaea) SHA1(8d69c61145470003cfeb33b11b81345c5e5e6503) )
	ROM_LOAD( "14.31.ic110",  0x2000, 0x2000, CRC(0d99a33e) SHA1(0df29464ea43aecd866ae322f4f7ca9152422023) )
	ROM_LOAD( "15.32.ic111",  0x4000, 0x2000, CRC(410fdf7c) SHA1(0957f335b84c4fbde983271786e7bf199fc22682) )

	ROM_REGION( 0x2000, "gfx4", 0 )
	ROM_LOAD( "10.40.ic99",   0x0000, 0x2000, CRC(c370b177) SHA1(4b3f73f764ff95cc7777fe01333558201658cead) )

	ROM_REGION( 0x4000, "gfx5", 0 ) /* not used?? */
	ROM_LOAD( "12.21.ic88",   0x0000, 0x2000, CRC(684b7bb0) SHA1(d83c45ff3adf94c649340227794020482231399f) )
	ROM_LOAD( "13.20.ic89",   0x2000, 0x2000, CRC(d1ef110e) SHA1(e34b6b4b70c783a8cf1296a05d3cec6af5820d0c) )

	ROM_REGION( 0x020, "proms", 0 )
	ROM_LOAD( "tbp18s030.ic2",  0x0000, 0x020, CRC(c366a9c5) SHA1(d38581e5c425cab4a4f216d99651e86d8034a7d2) ) // color prom located at edge of pcb
ROM_END


GAME( 1984, taxidriv,  0,        taxidriv, taxidriv, driver_device, 0, ROT90, "Graphic Techno", "Taxi Driver", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
