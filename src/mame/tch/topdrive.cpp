// license:BSD-3-Clause
// copyright-holders:David Haywood

// Top Driving by Proyesel - PRO-4/B PCB
// an alt version of this called Mortal Race has been seen, a logo for it also exists in the ROM.
// https://www.recreativas.org/mortal-race-2435-ecogames-sl "1995 New Dream Games S.L. Palma MCA Spain"

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

// TODO:
// Wheel support - what is IRQ6 for? wheel?
// Measure clocks, refresh frequency
// Is it worth merging with kickgoal.cpp?

namespace {

class topdrive_state : public driver_device
{
public:
	topdrive_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_eeprom(*this, "eeprom") ,
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_scrram(*this, "scrram"),
		m_spriteram(*this, "spriteram"),
		m_bg2_videoram(*this, "bg2_videoram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_fg_videoram(*this, "fg_videoram")
	{ }

	void topdrive(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint16_t> m_scrram;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_bg2_videoram;
	required_shared_ptr<uint16_t> m_bg_videoram;
	required_shared_ptr<uint16_t> m_fg_videoram;

	// video-related
	tilemap_t *m_bg2_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;

	void bg2_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void bg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void soundbank_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0)
	{
		// In theory this would be sample banking (it writes a value of 01 on startup)
		// however all samples addresses in header are sequential, and data after
		// the last used sample doesn't appear to be sound data anyway.
		// Furthermore no other values are ever written here
	}

	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TILEMAP_MAPPER_MEMBER(tilemap_scan_16x16);

	void topdrive_map(address_map &map) ATTR_COLD;

	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect, int drawpri);
};


TILEMAP_MAPPER_MEMBER(topdrive_state::tilemap_scan_16x16)
{
	// logical (col,row) -> memory offset
	return (row & 0xf) | ((col & 0x3f) << 4) | ((row & 0x30) << 6);
}

void topdrive_state::draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect, int drawpri)
{
	for (int offs = 0; offs < m_spriteram.length(); offs += 4)
	{
		int xpos         = m_spriteram[offs + 3];
		int ypos         = m_spriteram[offs + 0] & 0x00ff;
		u16 const tileno = m_spriteram[offs + 2] & 0x3fff;
		bool const pri   = (m_spriteram[offs + 1] & 0x0010)>>4; // 0x0020 is NOT flip like kickgoal.cpp, probably another priority bit
		u16 const color  = m_spriteram[offs + 1] & 0x000f;

		if (m_spriteram[offs + 0] & 0x0100) break;

		if (pri != drawpri)
			continue;

		ypos = 0x110 - ypos;

		m_gfxdecode->gfx(0)->transpen(bitmap, cliprect,
				tileno,
				color + 0x30,
				0,0,
				xpos-64+2, ypos-31, 15);
	}
}

void topdrive_state::bg2_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bg2_videoram[offset]);
	m_bg2_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(topdrive_state::get_bg2_tile_info)
{
	int const tileno = m_bg2_videoram[tile_index] & 0x1fff;
	int const color = (m_bg2_videoram[tile_index] & 0xe000) >> 13;
	tileinfo.set(0, tileno+0x3000, color+0x20, 0);
}

void topdrive_state::bg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(topdrive_state::get_bg_tile_info)
{
	int const tileno = m_bg_videoram[tile_index] & 0x1fff;
	int const color = (m_bg_videoram[tile_index] & 0xe000) >> 13;
	tileinfo.set(0, tileno+0x6000, color+0x10, 0);
}

void topdrive_state::fg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(topdrive_state::get_fg_tile_info)
{
	int const tileno = m_fg_videoram[tile_index] & 0x1fff;
	int const color = (m_fg_videoram[tile_index] & 0xe000) >> 13;
	tileinfo.set(0, tileno+0x4000, color+0x00, 0);
}

void topdrive_state::video_start()
{
	m_bg2_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(topdrive_state::get_bg2_tile_info)), tilemap_mapper_delegate(*this, FUNC(topdrive_state::tilemap_scan_16x16)), 16, 16, 32, 16);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(topdrive_state::get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(topdrive_state::tilemap_scan_16x16)), 16, 16, 32, 16);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(topdrive_state::get_fg_tile_info)), tilemap_mapper_delegate(*this, FUNC(topdrive_state::tilemap_scan_16x16)), 16, 16, 32, 16);

	m_bg_tilemap->set_transparent_pen(0xf);
	m_fg_tilemap->set_transparent_pen(0xf);
}

uint32_t topdrive_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_fg_tilemap->set_scrollx(0, m_scrram[0]+50);
	m_fg_tilemap->set_scrolly(0, m_scrram[1]);
	m_bg_tilemap->set_scrollx(0, m_scrram[2]+50);
	m_bg_tilemap->set_scrolly(0, m_scrram[3]);
	m_bg2_tilemap->set_scrollx(0, m_scrram[4]+50);
	m_bg2_tilemap->set_scrolly(0, m_scrram[5]);

	m_bg2_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect, 1);

	return 0;
}

void topdrive_state::topdrive_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();

	map(0x800000, 0x800001).portr("P1_P2");
	map(0x800002, 0x800003).portr("SYSTEM");
	// map(0x800006, 0x800007) // accessed in service menu, wheel maybe?

	map(0x900001, 0x900001).lw8(NAME([this] (u8 data) { m_eeprom->cs_write(BIT(data, 0)); }));
	map(0x900003, 0x900003).lw8(NAME([this] (u8 data) { m_eeprom->clk_write(BIT(data, 0)); }));
	map(0x900005, 0x900005).lw8(NAME([this] (u8 data) { m_eeprom->di_write(BIT(data, 0)); }));
	map(0x900007, 0x900007).lr8(NAME([this] () { return u8(m_eeprom->do_read()); }));

	map(0xa00000, 0xa003ff).ram().w(FUNC(topdrive_state::fg_videoram_w)).share(m_fg_videoram);
	map(0xa00400, 0xa01fff).ram();

	map(0xa02000, 0xa03fff).ram(); // buffer for scroll regs? or layer configs?

	map(0xa04000, 0xa043ff).ram().w(FUNC(topdrive_state::bg_videoram_w)).share(m_bg_videoram);
	map(0xa04400, 0xa07fff).ram();

	map(0xa08000, 0xa083ff).ram().w(FUNC(topdrive_state::bg2_videoram_w)).share(m_bg2_videoram);
	map(0xa08400, 0xa0bfff).ram();

	map(0xa0c000, 0xa0c3ff).ram(); // seems to be a buffer for data that gets put at 0xa00000?
	map(0xa0c400, 0xa0ffff).ram();

	map(0xa10000, 0xa1000f).ram().share(m_scrram);

	map(0xb00000, 0xb007ff).ram().share(m_spriteram);
	map(0xc00000, 0xc007ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xe00003, 0xe00003).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xe00004, 0xe00005).w(FUNC(topdrive_state::soundbank_w));

	map(0xf00000, 0xf2ffff).ram();
	map(0xff0000, 0xffffff).ram();
}

static INPUT_PORTS_START( topdrive )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x0800, IP_ACTIVE_LOW )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static GFXDECODE_START( gfx_topdrive )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_16x16x4_planar,  0x000, 0x40 )
GFXDECODE_END


void topdrive_state::topdrive(machine_config &config)
{
	M68000(config, m_maincpu, XTAL(12'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &topdrive_state::topdrive_map);
	m_maincpu->set_vblank_int("screen", FUNC(topdrive_state::irq2_line_hold)); // irq6 also looks valid?

	EEPROM_93C46_16BIT(config, m_eeprom);

	GFXDECODE(config, m_gfxdecode, "palette", gfx_topdrive);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(58.75); // verified
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2000));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 0*8, 30*8-1);
	screen.set_screen_update(FUNC(topdrive_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x400);

	SPEAKER(config, "mono").front_center();

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(16'000'000) / 16, okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 verified
	oki.add_route(ALL_OUTPUTS, "mono", 1.00);
}

ROM_START( topdrive )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "2-27c040.bin", 0x00000, 0x80000, CRC(37798c4e) SHA1(708a64b416bd2104fbc4b72a37bfeae33bbab454) )
	ROM_LOAD16_BYTE( "1-27c040.bin", 0x00001, 0x80000, CRC(e2dc5096) SHA1(82b22e03be225ab7f20eff6314383a9f28d52294) )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD( "4-27c040.bin",  0x000000, 0x80000, CRC(a81ca7f7) SHA1(cc2030a9bea90b694adbf222389766945ce9552b) )
	ROM_LOAD( "5-27c040.bin",  0x080000, 0x80000, CRC(a756d2b2) SHA1(59ddef858850b0f6c5865d555d6402c41cc3cb6c) )
	ROM_LOAD( "6-27c040.bin",  0x100000, 0x80000, CRC(90c778a2) SHA1(8122ee085e388bb1f7952edb6a99dffc466f2e2c) )
	ROM_LOAD( "7-27c040.bin",  0x180000, 0x80000, CRC(db219087) SHA1(c79145555678971db29e91a24d69738da7d8f07f) )
	ROM_LOAD( "8-27c040.bin",  0x200000, 0x80000, CRC(0e5f4419) SHA1(4fc8173001e2b412f4a7b0b5160c853436bbb139) )
	ROM_LOAD( "9-27c040.bin",  0x280000, 0x80000, CRC(159a7426) SHA1(6851fbc1fe11ae72a86d35011730d2df641e8fc5) )
	ROM_LOAD( "10-27c040.bin", 0x300000, 0x80000, CRC(54c1617a) SHA1(7bb4faaa54581f080f19f98e78fa9cae899f4c2a) )
	ROM_LOAD( "11-27c040.bin", 0x380000, 0x80000, CRC(6b3c3c73) SHA1(8ac76abdc4676cfcd9dc66a4c7b55010de099133) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "3-27c040.bin",      0x00000, 0x80000, CRC(2894b89b) SHA1(cf884042edd2fc05e04d21ccd36f5183f9a7ec5c) )
ROM_END

} // anonymous namespace

GAME( 1995, topdrive, 0, topdrive, topdrive, topdrive_state, empty_init, ROT0, "Proyesel", "Top Driving (version 1.1)", MACHINE_SUPPORTS_SAVE )
