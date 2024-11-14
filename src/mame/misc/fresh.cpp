// license:BSD-3-Clause
// copyright-holders:David Haywood

/*

fruit fresh by chain leisure electronic co., ltd.

cpu 68000 xtal 24Mhz

4* 8 dipswitchs

2 jumpers HOP or SSR positions

SW1 for reset?

2x Altera epm7064lc84

rom 5 and 6 are prg roms

*/

// notes : is the reel scrolling (division of the screen) done with raster interrupts?

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/timer.h"
#include "sound/ymopl.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class fresh_state : public driver_device
{
public:
	fresh_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_bg_videoram(*this, "bg_videoram"),
		m_bg_2_videoram(*this, "bg_videoram_2"),
		m_attr_videoram(*this, "attr_videoram"),
		m_attr_2_videoram(*this, "attr_videoram_2"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void fresh(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_bg_2_tilemap = nullptr;

	required_shared_ptr<uint16_t> m_bg_videoram;
	required_shared_ptr<uint16_t> m_bg_2_videoram;
	required_shared_ptr<uint16_t> m_attr_videoram;
	required_shared_ptr<uint16_t> m_attr_2_videoram;

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	void fresh_bg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fresh_attr_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_fresh_bg_tile_info);
	void fresh_bg_2_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fresh_attr_2_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_fresh_bg_2_tile_info);

	uint16_t m_d30000_value = 0;

	void d30000_write(uint16_t data)
	{
		m_d30000_value = data;
	}

	void c71000_write(uint16_t data)
	{
		logerror("c74000_write (scroll 0) %04x (m_d30000_value = %04x)\n", data, m_d30000_value);
	}
	void c74000_write(uint16_t data)
	{
		logerror("c74000_write (scroll 1) %04x (m_d30000_value = %04x)\n", data, m_d30000_value);
	}
	void c75000_write(uint16_t data)
	{
		logerror("c75000_write (scroll 2) %04x (m_d30000_value = %04x)\n", data, m_d30000_value);
	}
	void c76000_write(uint16_t data)
	{
		logerror("c76000_write (scroll 3) %04x (m_d30000_value = %04x)\n", data, m_d30000_value);
	}

	uint16_t unk_r()
	{
		return machine().rand();
	}
	uint16_t unk2_r()
	{
		return 0x10;
	}

	TIMER_DEVICE_CALLBACK_MEMBER(fake_scanline);

	uint32_t screen_update_fresh(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void fresh_map(address_map &map) ATTR_COLD;
};


TILE_GET_INFO_MEMBER(fresh_state::get_fresh_bg_tile_info)
{
	int tileno, pal;
	tileno = m_bg_videoram[tile_index];
	pal = m_attr_videoram[tile_index];
	tileinfo.set(1, tileno, pal, 0);
}


void fresh_state::fresh_bg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

void fresh_state::fresh_attr_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_attr_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(fresh_state::get_fresh_bg_2_tile_info)
{
	int tileno, pal;
	tileno = m_bg_2_videoram[tile_index];
	pal = m_attr_2_videoram[tile_index];
	tileinfo.set(0, tileno, pal, 0);
}


void fresh_state::fresh_bg_2_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bg_2_videoram[offset]);
	m_bg_2_tilemap->mark_tile_dirty(offset);
}

void fresh_state::fresh_attr_2_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_attr_2_videoram[offset]);
	m_bg_2_tilemap->mark_tile_dirty(offset);
}




void fresh_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(fresh_state::get_fresh_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8,  64, 512);
	m_bg_2_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(fresh_state::get_fresh_bg_2_tile_info)), TILEMAP_SCAN_ROWS, 8, 8,  64, 512);

	m_bg_tilemap->set_transparent_pen(255);
}

uint32_t fresh_state::screen_update_fresh(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_2_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


void fresh_state::fresh_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();

	map(0xc00000, 0xc0ffff).ram().w(FUNC(fresh_state::fresh_bg_2_videoram_w)).share("bg_videoram_2");
	map(0xc10000, 0xc1ffff).ram().w(FUNC(fresh_state::fresh_attr_2_videoram_w)).share("attr_videoram_2");
	map(0xc20000, 0xc2ffff).ram().w(FUNC(fresh_state::fresh_bg_videoram_w)).share("bg_videoram");
	map(0xc30000, 0xc3ffff).ram().w(FUNC(fresh_state::fresh_attr_videoram_w)).share("attr_videoram");

//  map(0xc70000, 0xc70001).ram();
//  map(0xc70002, 0xc70003).ram();
	map(0xc71000, 0xc71001).w(FUNC(fresh_state::c71000_write));
//  map(0xc72000, 0xc72001).ram();
//  map(0xc72002, 0xc72003).ram();
//  map(0xc73000, 0xc73001).ram();
//  map(0xc73002, 0xc73003).ram();
	map(0xc74000, 0xc74001).w(FUNC(fresh_state::c74000_write));
	map(0xc75000, 0xc75001).w(FUNC(fresh_state::c75000_write));
	map(0xc76000, 0xc76001).w(FUNC(fresh_state::c76000_write));
//  map(0xc77000, 0xc77001).ram();
//  map(0xc77002, 0xc77003).ram();

	// written together
	map(0xc40000, 0xc417ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xc50000, 0xc517ff).ram().w(m_palette, FUNC(palette_device::write16_ext)).share("palette_ext");

	map(0xd00001, 0xd00001).w("ymsnd", FUNC(ym2413_device::address_w));
	map(0xd10001, 0xd10001).w("ymsnd", FUNC(ym2413_device::data_w));

	map(0xd30000, 0xd30001).w(FUNC(fresh_state::d30000_write));
	map(0xd40000, 0xd40001).portr("IN0"); //.nopw(); // checks for 0x10
//  map(0xd40002, 0xd40003).nopw();
	map(0xd70000, 0xd70001).portr("IN1"); // checks for 0x10, dead loop if fail

	map(0xe00000, 0xe00001).portr("DSW0"); //.nopw();
	map(0xe20000, 0xe20001).portr("DSW1"); //.nopw();
	map(0xe40000, 0xe40001).portr("DSW2");
	map(0xe60000, 0xe60001).portr("DSW3");
	map(0xe80000, 0xe80001).portr("IN6");
	map(0xea0000, 0xea0001).portr("IN7");
	map(0xec0000, 0xec0001).portr("IN8");
	map(0xee0000, 0xee0001).portr("IN9");


	map(0xf00000, 0xf0ffff).ram();


}

static INPUT_PORTS_START( fresh )
	PORT_START("IN0")
	PORT_DIPNAME( 0x0001, 0x0000, "IN-0:0" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, "IN-0:1" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, "IN-0:2" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, "IN-0:3" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "IN-0:4" ) // startup test
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, "IN-0:5" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, "IN-0:6" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, "IN-0:7" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0000, "IN-0:8" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, "IN-0:9" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, "IN-0:a" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, "IN-0:b" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, "IN-0:c" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, "IN-0:d" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, "IN-0:e" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, "IN-0:f" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x0001, 0x0000, "IN-1:0" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, "IN-1:1" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, "IN-1:2" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, "IN-1:3" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "IN-1:4" ) // startup test
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, "IN-1:5" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, "IN-1:6" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, "IN-1:7" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0000, "IN-1:8" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, "IN-1:9" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, "IN-1:a" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, "IN-1:b" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, "IN-1:c" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, "IN-1:d" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, "IN-1:e" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, "IN-1:f" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x0001, 0x0000, "DSW-0:0" ) // SWITCH 1 in test mode
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, "DSW-0:1" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, "DSW-0:2" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, "DSW-0:3" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, "DSW-0:4" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, "DSW-0:5" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, "DSW-0:6" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, "DSW-0:7" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0000, "DSW-1:0" ) // SWITCH 2 in test mode
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, "DSW-1:1" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, "DSW-1:2" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, "DSW-1:3" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, "DSW-1:4" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, "DSW-1:5" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, "DSW-1:6" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, "DSW-1:7" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0001, 0x0000, "DSW-2:0" ) // SWITCH 3 in test mode
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, "DSW-2:1" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, "DSW-2:2" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, "DSW-2:3" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, "DSW-2:4" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, "DSW-2:5" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, "DSW-2:6 (keep off)" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, "DSW-2:7" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x0001, 0x0000, "DSW-3:0" ) // SWITCH 4 in test mode
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, "DSW-3:1" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, "DSW-3:2" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, "DSW-3:3" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, "DSW-3:4" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, "DSW-3:5" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, "DSW-3:6" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, "DSW-3:7" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )

	PORT_START("IN6")
	PORT_DIPNAME( 0x0001, 0x0001, "IN-6:0" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "IN-6:1" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "IN-6:2" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "IN-6:3" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "IN-6:4" ) // KEYOUT
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "IN-6:5" ) // PAYOUT
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "IN-6:6 (dip info)" ) // TEST
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "IN-6:7 (info)" ) // BOOK
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )

	PORT_START("IN7")
	PORT_DIPNAME( 0x0001, 0x0001, "IN-7:0" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "IN-7:1" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "IN-7:2 (red)" ) // RED
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "IN-7:3 (double)" ) // DOUBLE
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "IN-7:4 (take)" ) // TAKE
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_DIPNAME( 0x0040, 0x0040, "IN-7:6 (blue)" ) // BLUE
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN8")
	PORT_DIPNAME( 0x0001, 0x0001, "IN-8:0" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "IN-8:1" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "IN-8:2" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "IN-8:3" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "IN-8:4" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "IN-8:5" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "IN-8:6" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "IN-8:7" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "IN-8:8" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "IN-8:9" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, "IN-8:a" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "IN-8:b" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "IN-8:c" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "IN-8:d" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "IN-8:e" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "IN-8:f" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("IN9")
	PORT_DIPNAME( 0x0001, 0x0001, "IN-9:0 (reset)" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "IN-9:1" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "IN-9:2" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "IN-9:3" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "IN-9:4" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "IN-9:5" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "IN-9:6 (keyin)" ) // KEY-IN
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN1 ) // COIN

INPUT_PORTS_END

static GFXDECODE_START( gfx_fresh )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x8_raw, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_8x8x8_raw, 0, 16 )
GFXDECODE_END


TIMER_DEVICE_CALLBACK_MEMBER(fresh_state::fake_scanline)
{
	int scanline = param;

	if(scanline == 0)
	{
		logerror("new frame\n");
		m_maincpu->set_input_line(4, HOLD_LINE);

	}

//  if(scanline == 32)
//      m_maincpu->set_input_line(4, HOLD_LINE);

	if(scanline == 64)
		m_maincpu->set_input_line(5, HOLD_LINE);

//  if(scanline == 96)
//      m_maincpu->set_input_line(5, HOLD_LINE);


	if(scanline == 200) // vbl?
		m_maincpu->set_input_line(6, HOLD_LINE);

}


void fresh_state::fresh(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 24000000/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &fresh_state::fresh_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(fresh_state::fake_scanline), "screen", 0, 1);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(fresh_state::screen_update_fresh));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_888, 0x1000); // or 0xc00
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_fresh);

	/* sound hw? */
	SPEAKER(config, "mono").front_center();

	YM2413(config, "ymsnd", 4000000).add_route(ALL_OUTPUTS, "mono", 1.0); // actual clock and type unknown
}


ROM_START( fresh )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "fruit-fresh5.u44", 0x00001, 0x20000, CRC(cb37d3c5) SHA1(3b7797d475769d37ed1e9774df8d4b5899fb92a3) )
	ROM_LOAD16_BYTE( "fruit-fresh6.u59", 0x00000, 0x20000, CRC(fc0290be) SHA1(02e3b3563b15ae585684a8f510f48a8c90b248fa) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "fruit-fresh1.u18", 0x00000, 0x80000, CRC(ee77cdcd) SHA1(8e162640d23bd1b5a2ed9305cc4b9df1cb0f3e80) )
	ROM_LOAD( "fruit-fresh3.u19", 0x80000, 0x80000, CRC(80cc71b3) SHA1(89a2272266ccdbd01abbc85c1f8200fa9d8aa441) )
	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "fruit-fresh2.u45", 0x00000, 0x80000, CRC(8a06a1ab) SHA1(4bc020e4a031df995e6ebaf49d62989004092b60) )
	ROM_LOAD( "fruit-fresh4.u46", 0x80000, 0x80000, CRC(9b6c7571) SHA1(649cf3c50e2cd8c02f0f730e5ded59cf0ea37c37) )
ROM_END

} // anonymous namespace


// title shows Fruit Fresh but on resetting you get text strings of 'Dream World V2.41SI 97. 1.28'
GAME( 1996, fresh, 0, fresh, fresh, fresh_state, empty_init, ROT0, "Chain Leisure", "Fruit Fresh (Italy)", MACHINE_NOT_WORKING )
