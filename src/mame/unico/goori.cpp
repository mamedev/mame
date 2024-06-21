// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

  Goori Goori (c) Unico 1999

 hardware is loosely derived from kaneko/snowbros.cpp / Kaneko Pandora

 ToDo:
 is more than one button used?

Goori Goori PCB layout:
+-----------------------------------------------+
|               1         29F1610_OL 29F1610_SL |
|LED VR       M6295       29F1610_OH 29F1610_SH |
| YM3012  YM2151 3.579545MHz                    |
|                                               |
|                      GAL                      |
|J                 6164   +---------+  681000   |
|A                 6164   |  Actel  |           |
|M                        |A40MX04-F|  681000   |
|M                        |  PL84   |           |
|A                        +---------+  681000   |
|                       +---------+             |
|                       |  Actel  |    681000   |
|                  2    |A40MX04-F|             |
|            3   62H256 |  PL84   |             |
| 93C46    62H256       +---------+ 76C88       |
|    S1     68000    W2465                 6116 |
|16.000MHz                                      |
+-----------------------------------------------+

   CPU: MC68HC000FN16
 Sound: OKI M6295, YM2151+YM3012 (rebadged as AD-65, BS901+BS902)
   OSC: 16.000MHz, 3.579545MHz
EEPROM: ST 93C46CB1 (set to 16bit with jumper pad)
   RAM: SEC KM681000BLG-7L (x4), HMC62H256AK-15 (x2), UM6164DK-12 (x2)
        Winbond W2465AK-15, LGS GM76C88ALK-15 & HT6116-70
 Other: GAL16V8D, LED power LED, VR volume pot, S1 Service button

ROMS:
   1 - TMS 27C020-15 - Sound samples
   2 - unknown type  - 68000 Program code EVEN bytes (jumper pad made for 2M ROMs)
   3 - unknown type  - 68000 Program code ODD bytes  (jumper pad made for 2M ROMs)
   29F1610_OL - MX 29F1610ML - PCB silkscreened OBJ 16M/L
   29F1610_OH - MX 29F1610ML - PCB silkscreened OBJ 16M/H
   29F1610_SL - MX 29F1610ML - PCB silkscreened SCR 16M/L
   29F1610_SH - MX 29F1610ML - PCB silkscreened SCR 16M/H

*/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class goori_state : public driver_device
{
public:
	goori_state(const machine_config& mconfig, device_type type, const char* tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_oki(*this, "oki"),
		m_screen(*this, "screen"),
		m_eeprom(*this, "eeprom"),
		m_spriteram(*this, "spriteram"),
		m_bg_videoram(*this, "bg_videoram")
	{ }

	void goori(machine_config& config);

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	void goori_map(address_map& map);

	void bg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	uint32_t screen_update(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect);

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<okim6295_device> m_oki;
	required_device<screen_device> m_screen;
	required_device<eeprom_serial_93cxx_device> m_eeprom;

	// memory pointers
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_bg_videoram;

	// video related
	tilemap_t* m_bg_tilemap = nullptr;

	bool m_display_enable = false;
};


void goori_state::bg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset/2);
}

TILE_GET_INFO_MEMBER(goori_state::get_bg_tile_info)
{
	uint32_t const tilenolow = (m_bg_videoram[(tile_index * 2) + 1] & 0xff00) >> 8;
	uint32_t const tilenohigh = (m_bg_videoram[(tile_index * 2) + 0] & 0xff00) >> 8;

	uint32_t const tile = tilenolow | (tilenohigh << 8);

	tileinfo.set(1, tile, 0x1f, 0);
}

void goori_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(goori_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
}

uint32_t goori_state::screen_update(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect)
{
	bitmap.fill(0, cliprect);

	if (!m_display_enable)
		return 0;

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	gfx_element *gfx = m_gfxdecode->gfx(0);
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

		uint16_t const tile = m_spriteram[i + 6] | ((m_spriteram[i + 7] & 0x3f) << 8);
		int x = m_spriteram[i + 4];
		int const y = m_spriteram[i + 5];
		uint16_t const colour = (m_spriteram[i + 3] & 0xf8) >> 3;

		bool const flipx = BIT(m_spriteram[i + 7], 7);

		x |= (m_spriteram[i + 3] & 1) << 8;

		int realx = x;

		if (realx >= 0x3e0)
			realx -= 0x400;

		gfx->transpen(bitmap, cliprect, tile, colour, flipx, 0, realx, y, 0xff);
	}

	return 0;
}

void goori_state::goori_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram(); // RAM main

	map(0x300000, 0x300003).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write)).umask16(0xff00);
	map(0x300004, 0x300004).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));

	map(0x300008, 0x300008).lw8(
		NAME([this] (offs_t offset, u8 data) {
			if (data & 0xef)
				logerror("$300008: write %02x\n", data);
			m_display_enable = bool(BIT(data, 4));
		})
	);

	map(0x30000e, 0x30000f).portw("EEPROM");

	map(0x400000, 0x400fff).ram().w(FUNC(goori_state::bg_videoram_w)).share(m_bg_videoram); // 8-bit?

	map(0x500000, 0x500001).portr("DSW1");
	map(0x500002, 0x500003).portr("DSW2");
	map(0x500004, 0x500005).portr("SYSTEM");

	map(0x600000, 0x603fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x700000, 0x701fff).ram().share(m_spriteram); // RAM sprites (8-bit?)

	map(0x800000, 0x800001).noprw(); // irq ack?
}

static INPUT_PORTS_START( goori )
	PORT_START("EEPROM")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
	PORT_BIT( 0xfff8, IP_ACTIVE_HIGH, IPT_UNKNOWN )

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


static GFXDECODE_START( gfx_goori )
	GFXDECODE_ENTRY( "sprites", 0, layout_16x16x8,  0x0, 0x20 )
	GFXDECODE_ENTRY( "tiles",   0, gfx_16x16x8_raw, 0x0, 0x20 )
GFXDECODE_END

void goori_state::machine_start()
{
	save_item(NAME(m_display_enable));
}

void goori_state::goori(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 16_MHz_XTAL); /* 16MHz MC68HC000FN16 */
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

	EEPROM_93C46_16BIT(config, "eeprom"); // 93C46CB1 - Jumper pads for 16bit made

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_goori);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x2000);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	OKIM6295(config, m_oki, 16_MHz_XTAL/16, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 not verified
	m_oki->add_route(ALL_OUTPUTS, "lspeaker", 0.80);
	m_oki->add_route(ALL_OUTPUTS, "rspeaker", 0.80);

	YM2151(config, "ymsnd", 3.579545_MHz_XTAL).add_route(0, "lspeaker", 0.40).add_route(1, "rspeaker", 0.40);
}

ROM_START( goori )
	ROM_REGION( 0x080000, "maincpu", 0 ) // Support for 2M or 4M ROMs, jumper pad made for 2M
	ROM_LOAD16_BYTE( "2", 0x000000, 0x040000, CRC(82eae7bf) SHA1(a76743f8134f1614ec7fed76f33c5ee8dfe8ab2c) ) // Unico style label, simply labeled 2
	ROM_LOAD16_BYTE( "3", 0x000001, 0x040000, CRC(39093929) SHA1(2e13690ee4994cc5225a96ee47cfcb84cf63041f) ) // Unico style label, simply labeled 3

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD( "mx29f1610ml_obj_16m_l", 0x000000, 0x200000, CRC(f26451b9) SHA1(c6818a44115d3efed2566442295dc0b253057602) ) // MX 29F1610ML - PCB silkscreened OBJ 16M/L
	ROM_LOAD( "mx29f1610ml_obj_16m_h", 0x200000, 0x200000, CRC(058ceaec) SHA1(8639d41685a6f3fb2d81b9aaf3c160666de8155d) ) // MX 29F1610ML - PCB silkscreened OBJ 16M/H

	ROM_REGION( 0x400000, "tiles", 0 )
	ROM_LOAD( "mx29f1610ml_scr_16m_l", 0x000000, 0x200000, CRC(8603a662) SHA1(fbe5ccb3fded60b431ffee27471158c95a8328f8) ) // MX 29F1610ML - PCB silkscreened SCR 16M/L
	ROM_LOAD( "mx29f1610ml_scr_16m_h", 0x200000, 0x200000, CRC(4223383e) SHA1(aa17eab343dad3f6eab05a844081370e3eebcd2e) ) // MX 29F1610ML - PCB silkscreened SCR 16M/H

	ROM_REGION( 0x040000, "oki", 0 )
	ROM_LOAD( "1", 0x000000, 0x040000, CRC(c74351b9) SHA1(397f4b6aea23e6619e099c5cc99f38bae74bc3e8) ) // Unico style label, simply labeled 1
ROM_END

} // anonymous namespace


GAME( 1999, goori, 0, goori, goori, goori_state, empty_init, ROT0, "Unico", "Goori Goori", 0 )

