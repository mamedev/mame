// license:BSD-3-Clause
// copyright-holders:David Haywood

// hardware is loosely derived from snowbros / Kaneko Pandora

// background image is visible briefly at the start of level, is this correct?

// is more than one button used?


#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "machine/eepromser.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"
#include "sound/okim6295.h"
#include "sound/ym2151.h"
#include "speaker.h"

class goori_state : public driver_device
{
public:
	goori_state(const machine_config& mconfig, device_type type, const char* tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_oki(*this, "oki"),
		m_spriteram(*this, "spriteram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_screen(*this, "screen"),
		m_eeprom(*this, "eeprom")
	{ }

	void goori(machine_config& config);

protected:

	virtual void machine_start() override;
	virtual void video_start() override;

	uint32_t screen_update(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect);

private:
	void goori_map(address_map& map);

	DECLARE_WRITE16_MEMBER(goori_300008_w);
	DECLARE_WRITE16_MEMBER(goori_30000e_w);

	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<okim6295_device> m_oki;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_bg_videoram;
	required_device<screen_device> m_screen;
	required_device<eeprom_serial_93cxx_device> m_eeprom;

	tilemap_t* m_bg_tilemap;
	DECLARE_WRITE16_MEMBER(bg_videoram_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
};


WRITE16_MEMBER(goori_state::bg_videoram_w)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset/2);
}

TILE_GET_INFO_MEMBER(goori_state::get_bg_tile_info)
{
	int tilenolow = (m_bg_videoram[(tile_index * 2) + 1] & 0xff00) >> 8;
	int tilenohigh = (m_bg_videoram[(tile_index * 2) + 0] & 0xff00) >> 8;

	int tile = tilenolow | (tilenohigh << 8);

	tileinfo.set(1, tile, 0x1f, 0);
}

void goori_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(goori_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
}

uint32_t goori_state::screen_update(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect)
{
	//bitmap.fill(0, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	// is this sprite format a clone of anything? (looks VERY similar to snowbros but this hardware also has a higer resolution, a tile layer and 8bpp)
	for (int i = 0; i < 0x2000 / 2; i += 8)
	{
		// 0 unused
		// 1 unused
		// 2 unused
		// 3 colour and xpos msb
		// 4 xpos
		// 5 ypos
		// 6 tile low
		// 7 tile high (and flip)

		uint16_t tile = m_spriteram[i + 6] | ((m_spriteram[i + 7] & 0x3f) << 8);
		int x = m_spriteram[i + 4];
		int y = m_spriteram[i + 5];
		uint16_t colour = (m_spriteram[i + 3] & 0xf8) >> 3;

		int flipx = m_spriteram[i + 7] & 0x80;

		x |= (m_spriteram[i + 3] & 1) << 8;

		int realx = x;

		if (realx >= 0x3e0)
			realx -= 0x400;

		gfx_element *gfx = m_gfxdecode->gfx(0);
		gfx->transpen(bitmap,cliprect,tile,colour,flipx,0,realx,y,0xff);
	}
	
	return 0;
}

WRITE16_MEMBER(goori_state::goori_300008_w)
{
	//popmessage("goori_300008_w %04x %04x\n", data, mem_mask); // possibly display disable?
}

WRITE16_MEMBER(goori_state::goori_30000e_w)
{
	// eeprom writes?
	logerror("%06x goori_30000e_w %04x %04x\n", machine().describe_context(), data, mem_mask); // startup only?

	if (mem_mask & 0x00ff)
	{
		m_eeprom->di_write((data & 0x0004) >> 2);
		m_eeprom->cs_write((data & 0x0001) ? ASSERT_LINE : CLEAR_LINE);
		m_eeprom->clk_write((data & 0x0002) ? ASSERT_LINE : CLEAR_LINE);
	}
}

void goori_state::goori_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram(); // RAM main

	map(0x300000, 0x300003).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write)).umask16(0xff00);
	map(0x300004, 0x300004).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));

	map(0x300008, 0x300009).w(FUNC(goori_state::goori_300008_w));

	map(0x30000e, 0x30000f).w(FUNC(goori_state::goori_30000e_w));

	map(0x400000, 0x400fff).ram().w(FUNC(goori_state::bg_videoram_w)).share("bg_videoram"); // 8-bit?

	map(0x500000, 0x500001).portr("DSW1");
	map(0x500002, 0x500003).portr("DSW2");
	map(0x500004, 0x500005).portr("SYSTEM");

	map(0x600000, 0x603fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x700000, 0x701fff).ram().share("spriteram"); // RAM sprites (8-bit?)

	map(0x800000, 0x800001).noprw(); // irq ack?
}

static INPUT_PORTS_START( goori )
	PORT_START("DSW1")  /* 500001 */
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED ) // no dips on this PCB
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW2")  /* 500003 */
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED ) // no dips on this PCB
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")    /* 500005 */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x2000, IP_ACTIVE_LOW )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1 ) // makes sound but doesn't add a coin
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read) // EEPROM
INPUT_PORTS_END

static const gfx_layout layout_16x16x8 =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 8,9,10,11, 0,1,2,3},
	{ 0,4,16,20,32,36,48,52, 512+0, 512+4, 512+16, 512+20, 512+32, 512+36, 512+48, 512+52 },
	{ STEP8(0,8*8), STEP8(1024,8*8)},
	8*8*8*4,
};


static const gfx_layout layout_16x16x8_alt =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0 * 8, 1 * 8, 2 * 8, 3 * 8, 4 * 8, 5 * 8, 6 * 8, 7 * 8, 8 * 8, 9 * 8, 10 * 8, 11 * 8, 12 * 8, 13 * 8, 14 * 8, 15 * 8 },
	{ STEP16(0,16*8) },
	16*16*8,
};

static GFXDECODE_START( gfx_unico )
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x16x8, 0x0, 0x20 )
	GFXDECODE_ENTRY( "gfx2", 0, layout_16x16x8_alt, 0x0, 0x20 )
GFXDECODE_END

void goori_state::machine_start()
{
}

void goori_state::goori(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 32_MHz_XTAL/2); /* 16MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &goori_state::goori_map);
	m_maincpu->set_vblank_int("screen", FUNC(goori_state::irq4_line_hold));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60); // not verified
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(48*8, 262);
	m_screen->set_visarea(0*8, 48*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(goori_state::screen_update));
	m_screen->set_palette(m_palette);

	EEPROM_93C46_16BIT(config, "eeprom"); // 93C46C81

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_unico);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x2000);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	OKIM6295(config, m_oki, 32_MHz_XTAL/32, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 not verified
	m_oki->add_route(ALL_OUTPUTS, "lspeaker", 0.80);
	m_oki->add_route(ALL_OUTPUTS, "rspeaker", 0.80);

	YM2151(config, "ymsnd", XTAL(3'579'545)).add_route(0, "lspeaker", 0.40).add_route(1, "rspeaker", 0.40); // not verified
}

ROM_START( goori )
	ROM_REGION( 0x080000, "maincpu", 0 ) // first half empty
	ROM_LOAD16_BYTE( "goori02.4m", 0x000000, 0x040000, CRC(65a8568e) SHA1(c1a07f3a009df4af898ab62c15416073fcf768d9) )
	ROM_CONTINUE(0x000000, 0x040000)
	ROM_LOAD16_BYTE( "goori03.4m", 0x000001, 0x040000, CRC(de4b8818) SHA1(599ab6a354ab21d50c1b8c11e980f0b16f18e4dd) )
	ROM_CONTINUE(0x000001, 0x040000)

	ROM_REGION( 0x400000, "gfx1", 0 ) // sprites
	ROM_LOAD( "goori04ol.16m", 0x000000, 0x200000, CRC(f26451b9) SHA1(c6818a44115d3efed2566442295dc0b253057602) )
	ROM_LOAD( "goori05oh.16m", 0x200000, 0x200000, CRC(058ceaec) SHA1(8639d41685a6f3fb2d81b9aaf3c160666de8155d) )

	ROM_REGION( 0x400000, "gfx2", 0 ) // bgs
	ROM_LOAD( "goori06cl.16m", 0x000000, 0x200000, CRC(8603a662) SHA1(fbe5ccb3fded60b431ffee27471158c95a8328f8) )
	ROM_LOAD( "goori07ch.16m", 0x200000, 0x200000, CRC(4223383e) SHA1(aa17eab343dad3f6eab05a844081370e3eebcd2e) )

	ROM_REGION( 0x040000, "oki", 0 )
	ROM_LOAD( "goori01.2m", 0x000000, 0x040000, CRC(c74351b9) SHA1(397f4b6aea23e6619e099c5cc99f38bae74bc3e8) )
ROM_END

GAME( 1999, goori, 0,       goori, goori, goori_state,    empty_init, ROT0, "Unico", "Goori Goori", 0 )

