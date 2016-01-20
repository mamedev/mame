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





class fresh_state : public driver_device
{
public:
	fresh_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_bg_videoram(*this, "bg_videoram"),
		m_bg_2_videoram(*this, "bg_videoram_2"),
		m_attr_videoram(*this, "attr_videoram"),
		m_attr_2_videoram(*this, "attr_videoram_2"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_bg_2_tilemap;

	required_shared_ptr<UINT16> m_bg_videoram;
	required_shared_ptr<UINT16> m_bg_2_videoram;
	required_shared_ptr<UINT16> m_attr_videoram;
	required_shared_ptr<UINT16> m_attr_2_videoram;


	DECLARE_WRITE16_MEMBER(fresh_bg_videoram_w);
	DECLARE_WRITE16_MEMBER(fresh_attr_videoram_w);
	TILE_GET_INFO_MEMBER(get_fresh_bg_tile_info);
	DECLARE_WRITE16_MEMBER(fresh_bg_2_videoram_w);
	DECLARE_WRITE16_MEMBER(fresh_attr_2_videoram_w);
	TILE_GET_INFO_MEMBER(get_fresh_bg_2_tile_info);


	UINT16 m_d30000_value;

	DECLARE_WRITE16_MEMBER( d30000_write )
	{
		m_d30000_value = data;
	}

	DECLARE_WRITE16_MEMBER( c71000_write )
	{
		logerror("c74000_write (scroll 0) %04x (m_d30000_value = %04x)\n", data, m_d30000_value);
	}
	DECLARE_WRITE16_MEMBER( c74000_write )
	{
		logerror("c74000_write (scroll 1) %04x (m_d30000_value = %04x)\n", data, m_d30000_value);
	}
	DECLARE_WRITE16_MEMBER( c75000_write )
	{
		logerror("c75000_write (scroll 2) %04x (m_d30000_value = %04x)\n", data, m_d30000_value);
	}
	DECLARE_WRITE16_MEMBER( c76000_write )
	{
		logerror("c76000_write (scroll 3) %04x (m_d30000_value = %04x)\n", data, m_d30000_value);
	}

	DECLARE_READ16_MEMBER( unk_r )
	{
		return machine().rand();
	}
	DECLARE_READ16_MEMBER( unk2_r )
	{
		return 0x10;
	}

	TIMER_DEVICE_CALLBACK_MEMBER(fake_scanline);


	virtual void video_start() override;
	UINT32 screen_update_fresh(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


TILE_GET_INFO_MEMBER(fresh_state::get_fresh_bg_tile_info)
{
	int tileno, pal;
	tileno = m_bg_videoram[tile_index];
	pal = m_attr_videoram[tile_index];
	SET_TILE_INFO_MEMBER(1, tileno, pal, 0);
}


WRITE16_MEMBER(fresh_state::fresh_bg_videoram_w)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(fresh_state::fresh_attr_videoram_w)
{
	COMBINE_DATA(&m_attr_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(fresh_state::get_fresh_bg_2_tile_info)
{
	int tileno, pal;
	tileno = m_bg_2_videoram[tile_index];
	pal = m_attr_2_videoram[tile_index];
	SET_TILE_INFO_MEMBER(0, tileno, pal, 0);
}


WRITE16_MEMBER(fresh_state::fresh_bg_2_videoram_w)
{
	COMBINE_DATA(&m_bg_2_videoram[offset]);
	m_bg_2_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(fresh_state::fresh_attr_2_videoram_w)
{
	COMBINE_DATA(&m_attr_2_videoram[offset]);
	m_bg_2_tilemap->mark_tile_dirty(offset);
}




void fresh_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(fresh_state::get_fresh_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8,  64, 512);
	m_bg_2_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(fresh_state::get_fresh_bg_2_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8,  64, 512);

	m_bg_tilemap->set_transparent_pen(255);
}

UINT32 fresh_state::screen_update_fresh(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_2_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


static ADDRESS_MAP_START( fresh_map, AS_PROGRAM, 16, fresh_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM

	AM_RANGE(0xC00000, 0xC0ffff) AM_RAM_WRITE( fresh_bg_2_videoram_w ) AM_SHARE( "bg_videoram_2" )
	AM_RANGE(0xC10000, 0xC1ffff) AM_RAM_WRITE( fresh_attr_2_videoram_w ) AM_SHARE( "attr_videoram_2" )
	AM_RANGE(0xC20000, 0xC2ffff) AM_RAM_WRITE( fresh_bg_videoram_w ) AM_SHARE( "bg_videoram" )
	AM_RANGE(0xC30000, 0xC3ffff) AM_RAM_WRITE( fresh_attr_videoram_w ) AM_SHARE( "attr_videoram" )

//  AM_RANGE(0xC70000, 0xC70001) AM_RAM
//  AM_RANGE(0xC70002, 0xC70003) AM_RAM
	AM_RANGE(0xC71000, 0xC71001) AM_WRITE(c71000_write)
//  AM_RANGE(0xC72000, 0xC72001) AM_RAM
//  AM_RANGE(0xC72002, 0xC72003) AM_RAM
//  AM_RANGE(0xC73000, 0xC73001) AM_RAM
//  AM_RANGE(0xC73002, 0xC73003) AM_RAM
	AM_RANGE(0xC74000, 0xC74001) AM_WRITE(c74000_write)
	AM_RANGE(0xC75000, 0xC75001) AM_WRITE(c75000_write)
	AM_RANGE(0xC76000, 0xC76001) AM_WRITE(c76000_write)
//  AM_RANGE(0xC77000, 0xC77001) AM_RAM
//  AM_RANGE(0xC77002, 0xC77003) AM_RAM


	// written together
	AM_RANGE(0xC40000, 0xC417ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xC50000, 0xC517ff) AM_RAM_DEVWRITE("palette", palette_device, write_ext) AM_SHARE("palette_ext")

//  AM_RANGE(0xD00000, 0xD00001) AM_RAM
//  AM_RANGE(0xD10000, 0xD10001) AM_RAM
	AM_RANGE(0xD30000, 0xD30001) AM_WRITE(d30000_write)
	AM_RANGE(0xD40000, 0xD40001) AM_READ_PORT("IN0") //AM_WRITENOP // checks for 0x10
//  AM_RANGE(0xD40002, 0xD40003) AM_WRITENOP
	AM_RANGE(0xD70000, 0xD70001) AM_READ_PORT("IN1") // checks for 0x10, dead loop if fail

	AM_RANGE(0xE00000, 0xE00001) AM_READ_PORT("DSW0") //AM_WRITENOP
	AM_RANGE(0xE20000, 0xE20001) AM_READ_PORT("DSW1") //AM_WRITENOP
	AM_RANGE(0xE40000, 0xE40001) AM_READ_PORT("DSW2")
	AM_RANGE(0xE60000, 0xE60001) AM_READ_PORT("DSW3")
	AM_RANGE(0xE80000, 0xE80001) AM_READ_PORT("IN6")
	AM_RANGE(0xEA0000, 0xEA0001) AM_READ_PORT("IN7")
	AM_RANGE(0xEC0000, 0xEC0001) AM_READ_PORT("IN8")
	AM_RANGE(0xEE0000, 0xEE0001) AM_READ_PORT("IN9")


	AM_RANGE(0xF00000, 0xF0FFFF) AM_RAM


ADDRESS_MAP_END

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

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0,1, 2,3, 4,5,6,7 },
	{ 0, 8, 16, 24, 32, 40, 48, 56 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	64*8
};


static GFXDECODE_START( fresh )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout, 0, 16 )
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


static MACHINE_CONFIG_START( fresh, fresh_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 24000000/2 )
	MCFG_CPU_PROGRAM_MAP(fresh_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", fresh_state, fake_scanline, "screen", 0, 1)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(fresh_state, screen_update_fresh)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x1000) // or 0xc00
	MCFG_PALETTE_FORMAT(XBGR)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", fresh)

	/* sound hw? */
MACHINE_CONFIG_END


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



// title shows Fruit Fresh but on resetting you get text strings of 'Dream World V2.41SI 97. 1.28'
GAME( 1996, fresh, 0, fresh, fresh, driver_device, 0, ROT0, "Chain Leisure", "Fruit Fresh (Italy)", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
