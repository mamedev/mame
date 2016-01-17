// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina
/*
Dorachan (Dora-Chan ?) (c) 1980 Craul Denshi
Driver by Tomasz Slanina

Similar to Beam Invader
Todo:
- discrete sound
- dips (if any) - bits 5,6,7 of input port 0 ?
*/

#include "emu.h"
#include "cpu/z80/z80.h"


class dorachan_state : public driver_device
{
public:
	dorachan_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_colors(*this, "colors")
	{ }

	// devices, memory pointers
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_videoram;
	required_region_ptr<UINT8> m_colors;

	// internal state
	UINT8 m_flip_screen;

	DECLARE_WRITE8_MEMBER(control_w);
	DECLARE_READ8_MEMBER(protection_r);
	DECLARE_READ8_MEMBER(v128_r);
	UINT32 screen_update_dorachan(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	virtual void machine_start() override;
	virtual void machine_reset() override;
};



/*************************************
 *
 *  Video system
 *
 *************************************/

UINT32 dorachan_state::screen_update_dorachan(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (offs_t offs = 0; offs < m_videoram.bytes(); offs++)
	{
		UINT8 fore_color;

		UINT8 x = offs >> 8 << 3;
		UINT8 y = offs & 0xff;

		/* the need for +1 is extremely unusual, but definitely correct */
		offs_t color_address = ((((offs << 2) & 0x03e0) | (offs >> 8)) + 1) & 0x03ff;

		UINT8 data = m_videoram[offs];

		if (m_flip_screen)
			fore_color = (m_colors[color_address] >> 3) & 0x07;
		else
			fore_color = (m_colors[color_address] >> 0) & 0x07;

		for (int i = 0; i < 8; i++)
		{
			UINT8 color = (data & 0x01) ? fore_color : 0;
			bitmap.pix32(y, x) = m_palette->pen_color(color);

			data = data >> 1;
			x = x + 1;
		}
	}

	return 0;
}



/*************************************
 *
 *  I/O handlers
 *
 *************************************/

READ8_MEMBER(dorachan_state::protection_r)
{
	UINT8 ret = 0;

	switch (m_maincpu->pcbase())
	{
	case 0x70ce: ret = 0xf2; break;
	case 0x72a2: ret = 0xd5; break;
	case 0x72b5: ret = 0xcb; break;

	default:
		osd_printf_debug("unhandled $2400 read @ %x\n", m_maincpu->pcbase());
		break;
	}

	return ret;
}

READ8_MEMBER(dorachan_state::v128_r)
{
	// to avoid resetting (when player 2 starts) bit 0 need to be inverted when screen is flipped
	return 0xfe | ((m_screen->vpos() >> 7 & 1) ^ m_flip_screen);
}

WRITE8_MEMBER(dorachan_state::control_w)
{
	// d6: flip screen
	// other: ?
	m_flip_screen = data >> 6 & 1;
}


static ADDRESS_MAP_START( dorachan_map, AS_PROGRAM, 8, dorachan_state )
	AM_RANGE(0x0000, 0x17ff) AM_ROM
	AM_RANGE(0x1800, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x23ff) AM_ROM
	AM_RANGE(0x2400, 0x2400) AM_MIRROR(0x03ff) AM_READ(protection_r)
	AM_RANGE(0x2800, 0x2800) AM_MIRROR(0x03ff) AM_READ_PORT("IN0")
	AM_RANGE(0x2c00, 0x2c00) AM_MIRROR(0x03ff) AM_READ_PORT("IN1")
	AM_RANGE(0x3800, 0x3800) AM_MIRROR(0x03ff) AM_READ(v128_r)
	AM_RANGE(0x4000, 0x5fff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x6000, 0x77ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( dorachan_io_map, AS_IO, 8, dorachan_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x01, 0x01) AM_WRITENOP
	AM_RANGE(0x02, 0x02) AM_WRITENOP
	AM_RANGE(0x03, 0x03) AM_WRITE(control_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Port definition
 *
 *************************************/

static INPUT_PORTS_START( dorachan )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void dorachan_state::machine_start()
{
	save_item(NAME(m_flip_screen));
}

void dorachan_state::machine_reset()
{
	m_flip_screen = 0;
}

static MACHINE_CONFIG_START( dorachan, dorachan_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 2000000)
	MCFG_CPU_PROGRAM_MAP(dorachan_map)
	MCFG_CPU_IO_MAP(dorachan_io_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(dorachan_state, irq0_line_hold, 2*60)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_UPDATE_DRIVER(dorachan_state, screen_update_dorachan)

	MCFG_PALETTE_ADD_3BIT_BGR("palette")
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition
 *
 *************************************/

ROM_START( dorachan )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c1.e1",      0x0000, 0x0400, CRC(29d66a96) SHA1(a0297d87574af65c6ded99aeb377ac407f6f163f) )
	ROM_LOAD( "d2.e2",      0x0400, 0x0400, CRC(144b6cd1) SHA1(195ce86e912a4b395097008c6d812fd75a1a2482) )
	ROM_LOAD( "d3.e3",      0x0800, 0x0400, CRC(a9a1bed7) SHA1(98af6f851c4477f770b6bd67e5465b5a271311ee) )
	ROM_LOAD( "d4.e5",      0x0c00, 0x0400, CRC(099ddf4b) SHA1(e4dd2b17a4320615204c66c24f60e58db13a5319) )
	ROM_LOAD( "c5.e6",      0x1000, 0x0400, CRC(49449dab) SHA1(3627c16cc17fae9de2294a37602b726e107d0a13) )
	ROM_LOAD( "d6.e7",      0x1400, 0x0400, CRC(5e409680) SHA1(f5e4d820c0f0493d724cd0d3da1113bccc09c2c3) )
	ROM_LOAD( "c7.e8",      0x2000, 0x0400, CRC(b331a5ff) SHA1(1053953c76dddff450b9c9037e7797d50f9c7046) )
	ROM_LOAD( "d8.rom",     0x6000, 0x0400, CRC(5fe1e731) SHA1(8e5dcb5f8d1d6f8c06808dd808f8bce7b07014ee) )
	ROM_LOAD( "d9.rom",     0x6400, 0x0400, CRC(338881a8) SHA1(cd725b42c3f96826e94345698738f6b5a532d3d5) )
	ROM_LOAD( "d10.rom",    0x6800, 0x0400, CRC(f8c59517) SHA1(655a976b1221e5aff69e0c0cc58d02c0b7bb6197) )
	ROM_LOAD( "d11.rom",    0x6c00, 0x0400, CRC(c2e0f066) SHA1(be6b780a8957d945e5634ac9689b440a41e9a2a4) )
	ROM_LOAD( "d12.rom",    0x7000, 0x0400, CRC(275e5dc1) SHA1(ac07db4b428daa49a52c679de95ddedbea0076b9) )
	ROM_LOAD( "d13.rom",    0x7400, 0x0400, CRC(24ccfcf9) SHA1(85e5052ee657f518b0509eb64e494bc3a74e651e) )

	ROM_REGION( 0x0400, "colors", 0 )
	ROM_LOAD( "d14.rom",    0x0000, 0x0400, CRC(c0d3ee84) SHA1(f2207c685ce8d5144a373c28f11d2cebf9518b65) )
ROM_END



/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 1980, dorachan, 0, dorachan, dorachan, driver_device, 0, ROT270, "Alpha Denshi Co. / Craul Denshi", "Dora-chan (Japan)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
