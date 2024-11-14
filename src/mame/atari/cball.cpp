// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/***************************************************************************

    Atari Cannonball (prototype) driver

***************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


namespace {

class cball_state : public driver_device
{
public:
	cball_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_video_ram(*this, "video_ram")
	{ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_video_ram;

	/* video-related */
	tilemap_t* m_bg_tilemap = nullptr;

	emu_timer *m_int_timer = nullptr;
	TIMER_CALLBACK_MEMBER(interrupt_callback);

	void vram_w(offs_t offset, uint8_t data);
	uint8_t wram_r(offs_t offset);
	void wram_w(offs_t offset, uint8_t data);

	TILE_GET_INFO_MEMBER(get_tile_info);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	void cball_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void cball(machine_config &config);
	void cpu_map(address_map &map) ATTR_COLD;
};


TILE_GET_INFO_MEMBER(cball_state::get_tile_info)
{
	uint8_t code = m_video_ram[tile_index];

	tileinfo.set(0, code, code >> 7, 0);
}


void cball_state::vram_w(offs_t offset, uint8_t data)
{
	m_video_ram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


void cball_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cball_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}


uint32_t cball_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* draw playfield */
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* draw sprite */
	m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
		m_video_ram[0x399] >> 4,
		0,
		0, 0,
		240 - m_video_ram[0x390],
		240 - m_video_ram[0x398], 0);
	return 0;
}


TIMER_CALLBACK_MEMBER(cball_state::interrupt_callback)
{
	int scanline = param;

	m_maincpu->pulse_input_line(0, m_maincpu->minimum_quantum_time());

	scanline = scanline + 32;

	if (scanline >= 262)
		scanline = 16;

	m_int_timer->adjust(m_screen->time_until_pos(scanline), scanline);
}


void cball_state::machine_start()
{
	m_int_timer = timer_alloc(FUNC(cball_state::interrupt_callback), this);
}

void cball_state::machine_reset()
{
	m_int_timer->adjust(m_screen->time_until_pos(16), 16);
}


void cball_state::cball_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0x80, 0x80, 0x80));
	palette.set_pen_color(1, rgb_t(0x00, 0x00, 0x00));
	palette.set_pen_color(2, rgb_t(0x80, 0x80, 0x80));
	palette.set_pen_color(3, rgb_t(0xff, 0xff, 0xff));
	palette.set_pen_color(4, rgb_t(0x80, 0x80, 0x80));
	palette.set_pen_color(5, rgb_t(0xc0, 0xc0, 0xc0));
}


uint8_t cball_state::wram_r(offs_t offset)
{
	return m_video_ram[0x380 + offset];
}


void cball_state::wram_w(offs_t offset, uint8_t data)
{
	m_video_ram[0x380 + offset] = data;
}



void cball_state::cpu_map(address_map &map)
{
	map.global_mask(0x7fff);

	map(0x0000, 0x03ff).r(FUNC(cball_state::wram_r)).mask(0x7f);
	map(0x1001, 0x1001).portr("1001");
	map(0x1003, 0x1003).portr("1003");
	map(0x1020, 0x1020).portr("1020");
	map(0x1040, 0x1040).portr("1040");
	map(0x1060, 0x1060).portr("1060");
	map(0x2000, 0x2001).noprw();
	map(0x2800, 0x2800).portr("2800");

	map(0x0000, 0x03ff).w(FUNC(cball_state::wram_w)).mask(0x7f);
	map(0x0400, 0x07ff).ram().w(FUNC(cball_state::vram_w)).share("video_ram");
	map(0x1800, 0x1800).noprw(); /* watchdog? */
	map(0x1810, 0x1811).noprw();
	map(0x1820, 0x1821).noprw();
	map(0x1830, 0x1831).noprw();
	map(0x1840, 0x1841).noprw();
	map(0x1850, 0x1851).noprw();
	map(0x1870, 0x1871).noprw();

	map(0x7000, 0x7fff).rom();
}


static INPUT_PORTS_START( cball )
	PORT_START("1001")
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, "2 Coins each" )
	PORT_DIPSETTING(    0xc0, "1 Coin each" )
	PORT_DIPSETTING(    0x80, "1 Coin 1 Game" )
	PORT_DIPSETTING(    0x40, "1 Coin 2 Games" )

	PORT_START("1003")
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x40, "7" )
	PORT_DIPSETTING(    0x00, "9" )

	PORT_START("1020")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("1040")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("1060")
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("2800")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )
INPUT_PORTS_END


static const gfx_layout tile_layout =
{
	8, 8,
	64,
	1,
	{ 0 },
	{
		0, 1, 2, 3, 4, 5, 6, 7
	},
	{
		0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38
	},
	0x40
};


static const gfx_layout sprite_layout =
{
	16, 16,
	16,
	1,
	{ 0 },
	{
		0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0,
		0xf, 0xe, 0xd, 0xc, 0xb, 0xa, 0x9, 0x8
	},
	{
		0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
		0x80, 0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0
	},
	0x100
};


static GFXDECODE_START( gfx_cball )
	GFXDECODE_ENTRY( "gfx1", 0, tile_layout, 0, 2 )
	GFXDECODE_ENTRY( "gfx2", 0, sprite_layout, 4, 1 )
GFXDECODE_END


void cball_state::cball(machine_config &config)
{
	/* basic machine hardware */
	M6800(config, m_maincpu, XTAL(12'096'000) / 16); /* ? */
	m_maincpu->set_addrmap(AS_PROGRAM, &cball_state::cpu_map);


	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(256, 262);
	m_screen->set_visarea(0, 255, 0, 223);
	m_screen->set_screen_update(FUNC(cball_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cball);

	PALETTE(config, m_palette, FUNC(cball_state::cball_palette), 6);

	/* sound hardware */
}


ROM_START( cball )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD_NIB_LOW ( "canball.1e", 0x7400, 0x0400, CRC(0b34823b) SHA1(0db6b9f78f7c07ee7d35f2bf048ba61fe43b1e26) )
	ROM_LOAD_NIB_HIGH( "canball.1l", 0x7400, 0x0400, CRC(b43ca275) SHA1(a03e03f6366877cfdcec71030a5fb2c5171c8d8a) )
	ROM_LOAD_NIB_LOW ( "canball.1f", 0x7800, 0x0400, CRC(29b4e1f7) SHA1(8cef944b6e0153c304aa2d4cfdc530b8a4eef021) )
	ROM_LOAD_NIB_HIGH( "canball.1k", 0x7800, 0x0400, CRC(a4d1cf12) SHA1(99de6470efd16e57d72019e065f55bc740f3c7fc) )
	ROM_LOAD_NIB_LOW ( "canball.1h", 0x7c00, 0x0400, CRC(13f55937) SHA1(7514c27e60944c4e00992c8ecbc5115f8ff948bb) )
	ROM_LOAD_NIB_HIGH( "canball.1j", 0x7c00, 0x0400, CRC(5b905d69) SHA1(2408dd6e44c51c0c9bdb82d2d33826c03f8308c4) )

	ROM_REGION( 0x0200, "gfx1", 0 ) /* tiles */
	ROM_LOAD_NIB_LOW ( "canball.6m", 0x0000, 0x0200, BAD_DUMP CRC(b2aa7578) SHA1(5c3eb80066420002bc3dcc7ca4ab6efad7ed4ae5) ) // missing rom, zerofilled
	ROM_LOAD_NIB_HIGH( "canball.6l", 0x0000, 0x0200, CRC(5b1c9e88) SHA1(6e9630db9907170c53942a21302bcf8b721590a3) )

	ROM_REGION( 0x0200, "gfx2", 0 ) /* sprites */
	ROM_LOAD_NIB_LOW ( "canball.5l", 0x0000, 0x0200, CRC(3d0d1569) SHA1(1dfcf5cf9468d476c4b7d76a261c6fec87a99f93) )
	ROM_LOAD_NIB_HIGH( "canball.5k", 0x0000, 0x0200, CRC(c5fdd3c8) SHA1(5aae148439683ff1cf0005a810c81fdcbed525c3) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "canball.6h", 0x0000, 0x0100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) ) /* sync */
ROM_END

} // anonymous namespace


GAME( 1976, cball, 0, cball, cball, cball_state, empty_init, ROT0, "Atari", "Cannonball (Atari, prototype)", MACHINE_NO_SOUND | MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
