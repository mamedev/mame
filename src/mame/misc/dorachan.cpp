// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/*
Dorachan (Dora-Chan ?) (c) 1980 Craul Denshi
Driver by Tomasz Slanina

Similar to Beam Invader
Todo:
- discrete sound
- dips (if any) - bits 5,6,7 of input port 0 ?

Gameplay: run over dots in lower half while avoiding monsters and trees. This draws
back the red curtain blocking access to top part of the screen. Go through and new dots
below are worth more points.

It appears that unused bits in port 03 are to operate the discrete sound channels.

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "emupal.h"
#include "screen.h"


namespace {

class dorachan_state : public driver_device
{
public:
	dorachan_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_videoram(*this, "videoram")
		, m_colors(*this, "colors")
	{ }

	void dorachan(machine_config &config);

private:
	void control_w(uint8_t data);
	void protection_w(uint8_t data);
	uint8_t protection_r();
	uint8_t v128_r();
	uint32_t screen_update_dorachan(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void dorachan_io_map(address_map &map) ATTR_COLD;
	void dorachan_map(address_map &map) ATTR_COLD;

	// internal state
	uint8_t m_flip_screen = 0;
	uint16_t m_prot_value = 0;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// devices, memory pointers
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	required_region_ptr<uint8_t> m_colors;
};



/*************************************
 *
 *  Video system
 *
 *************************************/

uint32_t dorachan_state::screen_update_dorachan(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (offs_t offs = 0; offs < m_videoram.bytes(); offs++)
	{
		uint8_t x = offs >> 8 << 3;
		uint8_t y = offs & 0xff;

		/* the need for +1 is extremely unusual, but definitely correct */
		offs_t color_address = ((((offs << 2) & 0x03e0) | (offs >> 8)) + 1) & 0x03ff;

		uint8_t data = m_videoram[offs];

		uint8_t fore_color;
		if (m_flip_screen)
			fore_color = (m_colors[color_address] >> 3) & 0x07;
		else
			fore_color = (m_colors[color_address] >> 0) & 0x07;

		for (int i = 0; i < 8; i++)
		{
			uint8_t color = BIT(data, i) ? fore_color : 0;
			bitmap.pix(y, x++) = m_palette->pen_color(color);
		}
	}

	return 0;
}



/*************************************
 *
 *  I/O handlers
 *
 *************************************/

void dorachan_state::protection_w(uint8_t data)
{
	// e0 seems like some sort of control byte?
	// ignore f3 writes, written after every command?
	if (data != 0xf3)
	{
		m_prot_value <<= 8;
		m_prot_value |= data;
	}
}

uint8_t dorachan_state::protection_r()
{
	switch (m_prot_value)
	{
	case 0xfbf7:
		return 0xf2;

	case 0xf9f7:
		return 0xd5;

	case 0xf7f4:
		return 0xcb;
	}

	return 0;
}

uint8_t dorachan_state::v128_r()
{
	// to avoid resetting (when player 2 starts) bit 0 need to be inverted when screen is flipped
	return 0xfe | (BIT(m_screen->vpos(), 7) ^ m_flip_screen);
}

void dorachan_state::control_w(uint8_t data)
{
	// d6: flip screen
	// other: ?
	m_flip_screen = BIT(data, 6);
}


void dorachan_state::dorachan_map(address_map &map)
{
	map(0x0000, 0x17ff).rom();
	map(0x1800, 0x1fff).ram();
	map(0x2000, 0x23ff).rom();
	map(0x2400, 0x2400).mirror(0x03ff).r(FUNC(dorachan_state::protection_r));
	map(0x2800, 0x2800).mirror(0x03ff).portr("IN0");
	map(0x2c00, 0x2c00).mirror(0x03ff).portr("IN1");
	map(0x3800, 0x3800).mirror(0x03ff).r(FUNC(dorachan_state::v128_r));
	map(0x4000, 0x5fff).ram().share("videoram");
	map(0x6000, 0x77ff).rom();
}

void dorachan_state::dorachan_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).nopw();
	map(0x02, 0x02).w(FUNC(dorachan_state::protection_w));
	map(0x03, 0x03).w(FUNC(dorachan_state::control_w));
}



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
	save_item(NAME(m_prot_value));
}

void dorachan_state::machine_reset()
{
	m_flip_screen = 0;
	m_prot_value = 0;
}

void dorachan_state::dorachan(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 2000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &dorachan_state::dorachan_map);
	m_maincpu->set_addrmap(AS_IO, &dorachan_state::dorachan_io_map);
	m_maincpu->set_periodic_int(FUNC(dorachan_state::irq0_line_hold), attotime::from_hz(2*60));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 1*8, 31*8-1);
	m_screen->set_refresh_hz(60);
	m_screen->set_screen_update(FUNC(dorachan_state::screen_update_dorachan));

	PALETTE(config, m_palette, palette_device::BGR_3BIT);
}



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

} // anonymous namespace


/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 1980, dorachan, 0, dorachan, dorachan, dorachan_state, empty_init, ROT270, "Alpha Denshi Co. / Craul Denshi", "Dora-chan (Japan)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
