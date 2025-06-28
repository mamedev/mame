// license:BSD-3-Clause
// copyright-holders:David Haywood, Luca Elia
/*
Vs. Janshi Brandnew Stars
(c)1997 Jaleco

Single board version with Dual Screen output
(MS32 version also exists)

for the time being most of this driver is copied
from ms32.cpp, with some adjustments for dual screen.

Main PCB
--------

PCB ID  : MB-93141 EB91022-20081
CPU     : NEC D70632GD-20 (V70)
OSC     : 48.000MHz, 40.000MHz
RAM     : Cypress CY7C199-25 (x20)
          OKI M511664-80 (x8)
DIPs    : 8 position (x3)
OTHER   : Some PALs

          Custom chips:
                       JALECO SS91022-01 (208 PIN PQFP)
                       JALECO SS91022-02 (100 PIN PQFP) (x2)
                       JALECO SS91022-03 (176 PIN PQFP) (x2)
                       JALECO SS91022-05 (120 PIN PQFP) (x2)
                       JALECO SS91022-07 (208 PIN PQFP) (x2)
                       JALECO GS91022-01 (120 PIN PQFP)
                       JALECO GS91022-02 (160 PIN PQFP)
                       JALECO GS91022-03 (100 PIN PQFP)
                       JALECO GS91022-04 (100 PIN PQFP)
                       JALECO GS90015-03 ( 80 PIN PQFP) (x3) (not present on MS32)


ROMs:     None


ROM PCB (equivalent to MS32 cartridge)
--------------------------------------
PCB ID  : MB-93142 EB93007-20082
DIPs    : determine the size of ROMs?
          8 position (x1, 6 and 8 is on, others are off)
          4 position (x2, both are off on off off)
OTHER   : Some PALs
          Custom chip: JALECO SS92046-01 (144 pin PQFP) (x2)
          (located on small plug-in board with) (ID: SE93139 EB91022-30056)
ROMs    : MB-93142.36   [2eb6a503] (IC42, also printed "?u?????j???[Ver1.2")
          MB-93142.37   [49f60882] (IC57, also printed "?u?????j???[Ver1.2")
          MB-93142.38   [6e1312cd] (IC58, also printed "?u?????j???[Ver1.2")
          MB-93142.39   [56b98539] (IC59, also printed "?u?????j???[Ver1.2")

          VSJANSHI5.6   [fdbbac21] (IC9, actual label is "VS?W?????V 5 Ver1.0")
          VSJANSHI6.5   [fdbbac21] (IC8, actual label is "VS?W?????V 6 Ver1.0")

          MR96004-01.20 [3366d104] (IC29)
          MR96004-02.28 [ad556664] (IC49)
          MR96004-03.21 [b399e2b1] (IC30)
          MR96004-04.29 [f4f4cf4a] (IC50)
          MR96004-05.22 [cd6c357e] (IC31)
          MR96004-06.30 [fc6daad7] (IC51)
          MR96004-07.23 [177e32fa] (IC32)
          MR96004-08.31 [f6df27b2] (IC52)

          MR96004-09.1  [603143e8] (IC4)
          MR96004-09.7  [603143e8] (IC10)

          MR96004-11.11 [e6da552c] (IC17)
          MR96004-11.13 [e6da552c] (IC19)


Sound PCB
---------
PCB ID  : SB-93143 EB91022-20083
SOUND   : Z80, YMF271-F, YAC513(x2)
OSC     : 8.000MHz, 16.9344MHz
DIPs    : 4 position (all off)
ROMs    : SB93145.5     [0424e899] (IC34, also printed "Ver1.0") - Sound program
          MR96004-10.1  [125661cd] (IC19 - Samples)

Sound sub PCB
-------------
PCB ID  : SB-93145 EB93007-20086
SOUND   : YMF271-F
ROMs    : MR96004-10.1  [125661cd] (IC5 - Samples)

*/

#include "emu.h"
#include "ms32.h"

#include "cpu/z80/z80.h"
#include "cpu/v60/v60.h"
#include "jalcrpt.h"

#include "layout/generic.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class ms32_bnstars_state : public ms32_base_state
{
public:
	ms32_bnstars_state(const machine_config &mconfig, device_type type, const char *tag)
		: ms32_base_state(mconfig, type, tag)
		, m_sysctrl(*this, "sysctrl")
		, m_ymf(*this, "ymf.%u", 1U)
		, m_screen(*this, "screen.%u", 0U)
		, m_gfxdecode(*this, "gfxdecode.%u", 0U)
		, m_palette(*this, "palette.%u", 0U)
		, m_paletteram(*this, "paletteram.%u", 0U, 0x20000U, ENDIANNESS_LITTLE)
		, m_ascii_vram(*this, "ascii_vram_%u", 0U, 0x4000U, ENDIANNESS_LITTLE)
		, m_ascii_ctrl(*this, "ascii_ctrl.%u", 0U)
		, m_scroll_vram(*this, "scroll_vram_%u", 0U, 0x4000U, ENDIANNESS_LITTLE)
		, m_scroll_ctrl(*this, "scroll_ctrl.%u", 0U)
		, m_rotate_vram(*this, "rotate_vram_%u", 0U, 0x10000U, ENDIANNESS_LITTLE)
		, m_rotate_ctrl(*this, "rotate_ctrl.%u", 0U)
		, m_sprite(*this, "sprite.%u", 0U)
		, m_object_vram(*this, "objram_%u", 0U, 0x10000U, ENDIANNESS_LITTLE)
		, m_p1_keys(*this, "P1KEY.%u", 0)
		, m_p2_keys(*this, "P2KEY.%u", 0)
	{ }

	void bnstars(machine_config &config);

	void init_bnstars();

	template <int P> ioport_value mahjong_ctrl_r();

private:

	// TODO: subclass this device for dual screen config
	required_device<jaleco_ms32_sysctrl_device> m_sysctrl;

	required_device_array<ymf271_device, 2> m_ymf;
	required_device_array<screen_device, 2> m_screen;

	required_device_array<gfxdecode_device, 2> m_gfxdecode;
	required_device_array<palette_device, 2> m_palette;
	memory_share_array_creator<u16, 2> m_paletteram;

	memory_share_array_creator<u16, 2> m_ascii_vram;
	required_shared_ptr_array<u32, 2> m_ascii_ctrl;
	memory_share_array_creator<u16, 2> m_scroll_vram;
	required_shared_ptr_array<u32, 2> m_scroll_ctrl;
	memory_share_array_creator<u16, 2> m_rotate_vram;
	required_shared_ptr_array<u32, 2> m_rotate_ctrl;

	required_device_array<ms32_sprite_device, 2> m_sprite;
	memory_share_array_creator<u16, 2> m_object_vram;

	required_ioport_array<4> m_p1_keys;
	required_ioport_array<4> m_p2_keys;

	u32 m_bnstars1_mahjong_select = 0;
	template <int chip> void ascii_vram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	template <int chip> u16 ascii_vram_r(offs_t offset);
	template <int chip> void scroll_vram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	template <int chip> u16 scroll_vram_r(offs_t offset);
	template <int chip> void rotate_vram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	template <int chip> u16 rotate_vram_r(offs_t offset);
	template <int chip> void object_vram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	template <int chip> u16 object_vram_r(offs_t offset);
	template <int chip> void palette_ram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	template <int chip> u16 palette_ram_r(offs_t offset);
	void flipscreen_dual_w(int state);
	template <int chip> TILE_GET_INFO_MEMBER(get_ascii_tile_info);
	template <int chip> TILE_GET_INFO_MEMBER(get_scroll_tile_info);
	template <int chip> TILE_GET_INFO_MEMBER(get_rotate_tile_info);
	template <int chip> void draw_roz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority);
	template <int chip> void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	tilemap_t *m_ascii_tilemap[2]{};
	tilemap_t *m_scroll_tilemap[2]{};
	tilemap_t *m_rotate_tilemap[2]{};

	virtual void video_start() override ATTR_COLD;
	template <int which> u32 screen_update_dual(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void bnstars_map(address_map &map) ATTR_COLD;
	void bnstars_sound_map(address_map &map) ATTR_COLD;

	void bnstars1_mahjong_select_w(u32 data);
};


void ms32_bnstars_state::flipscreen_dual_w(int state)
{
	for (int chip = 0; chip < 2; chip++)
	{
		m_scroll_tilemap[chip]->set_flip(state ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
		m_ascii_tilemap[chip]->set_flip(state ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
		m_rotate_tilemap[chip]->set_flip(state ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
		// TODO: sprite device
	}
}

template <int chip> u16 ms32_bnstars_state::ascii_vram_r(offs_t offset)
{
	return m_ascii_vram[chip][offset];
}

template <int chip> void ms32_bnstars_state::ascii_vram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_ascii_vram[chip][offset]);
	m_ascii_tilemap[chip]->mark_tile_dirty(offset/2);
}

template <int chip> u16 ms32_bnstars_state::scroll_vram_r(offs_t offset)
{
	return m_scroll_vram[chip][offset];
}

template <int chip> void ms32_bnstars_state::scroll_vram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_scroll_vram[chip][offset]);
	m_scroll_tilemap[chip]->mark_tile_dirty(offset/2);
}

template <int chip> u16 ms32_bnstars_state::rotate_vram_r(offs_t offset)
{
	return m_rotate_vram[chip][offset];
}

template <int chip> void ms32_bnstars_state::rotate_vram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_rotate_vram[chip][offset]);
	m_rotate_tilemap[chip]->mark_tile_dirty(offset/2);
}

template <int chip> u16 ms32_bnstars_state::object_vram_r(offs_t offset)
{
	return m_object_vram[chip][offset];
}

template <int chip> void ms32_bnstars_state::object_vram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_object_vram[chip][offset]);
}

template <int chip> TILE_GET_INFO_MEMBER(ms32_bnstars_state::get_ascii_tile_info)
{
	int tileno, colour;
//  const int gfx_region[2] = {2, 5};

	tileno = m_ascii_vram[chip][tile_index *2+0] & 0x0000ffff;
	colour = m_ascii_vram[chip][tile_index *2+1] & 0x0000000f;

	tileinfo.set(2, tileno, colour, 0);
}

template <int chip> TILE_GET_INFO_MEMBER(ms32_bnstars_state::get_scroll_tile_info)
{
	int tileno, colour;
//  const int gfx_region[2] = {1, 4};

	tileno = m_scroll_vram[chip][tile_index *2+0] & 0x0000ffff;
	colour = m_scroll_vram[chip][tile_index *2+1] & 0x0000000f;

	tileinfo.set(1, tileno, colour, 0);
}

template <int chip> TILE_GET_INFO_MEMBER(ms32_bnstars_state::get_rotate_tile_info)
{
	int tileno, colour;
//  const int gfx_region[2] = {0, 3};

	tileno = m_rotate_vram[chip][tile_index *2+0] & 0x0000ffff;
	colour = m_rotate_vram[chip][tile_index *2+1] & 0x0000000f;

	tileinfo.set(0, tileno, colour, 0);
}


/* ROZ Layers */

// TODO: merge with main ms32.cpp
template <int chip> void ms32_bnstars_state::draw_roz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority)
{
	// TODO: registers 0x40/4 / 0x44/4 and 0x50/4 / 0x54/4 are used, meaning unknown

	if (m_rotate_ctrl[chip][0x5c/4] & 1)   /* "super" mode */
		throw emu_fatalerror("Super mode in bnstars1?");
	else    /* "simple" mode */
	{
		int startx = (m_rotate_ctrl[chip][0x00/4] & 0xffff) | ((m_rotate_ctrl[chip][0x04/4] & 3) << 16);
		int starty = (m_rotate_ctrl[chip][0x08/4] & 0xffff) | ((m_rotate_ctrl[chip][0x0c/4] & 3) << 16);
		int incxx  = (m_rotate_ctrl[chip][0x10/4] & 0xffff) | ((m_rotate_ctrl[chip][0x14/4] & 1) << 16);
		int incxy  = (m_rotate_ctrl[chip][0x18/4] & 0xffff) | ((m_rotate_ctrl[chip][0x1c/4] & 1) << 16);
		int incyy  = (m_rotate_ctrl[chip][0x20/4] & 0xffff) | ((m_rotate_ctrl[chip][0x24/4] & 1) << 16);
		int incyx  = (m_rotate_ctrl[chip][0x28/4] & 0xffff) | ((m_rotate_ctrl[chip][0x2c/4] & 1) << 16);
		int offsx  = m_rotate_ctrl[chip][0x30/4];
		int offsy  = m_rotate_ctrl[chip][0x34/4];

		offsx += (m_rotate_ctrl[chip][0x38/4] & 1) * 0x400;    // ??? gratia, hayaosi1...
		offsy += (m_rotate_ctrl[chip][0x3c/4] & 1) * 0x400;    // ??? gratia, hayaosi1...

		/* extend sign */
		if (startx & 0x20000) startx |= ~0x3ffff;
		if (starty & 0x20000) starty |= ~0x3ffff;
		if (incxx & 0x10000) incxx |= ~0x1ffff;
		if (incxy & 0x10000) incxy |= ~0x1ffff;
		if (incyy & 0x10000) incyy |= ~0x1ffff;
		if (incyx & 0x10000) incyx |= ~0x1ffff;

		m_rotate_tilemap[chip]->draw_roz(screen, bitmap, cliprect,
				(startx+offsx)<<16, (starty+offsy)<<16,
				incxx<<8, incxy<<8, incyx<<8, incyy<<8,
				1, // Wrap
				0, priority);
	}
}

// TODO: move to the sprite chip, fix priority
template <int chip> void ms32_bnstars_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u16 *sprram_top = m_object_vram[chip];
	const size_t sprite_tail = m_object_vram[chip].length() - 8;
	u16 *source = sprram_top;
	u16 *finish = sprram_top + sprite_tail;
	const bool reverseorder = (m_sprite_ctrl[0x10/4] & 0x8000) == 0x0000;

	if (reverseorder == true)
	{
		source = sprram_top + sprite_tail;
		finish = sprram_top;
	}

	for (;reverseorder ? (source>=finish) : (source<finish); reverseorder ? (source-=8) : (source+=8))
	{
		bool disable;
		u8 pri;
		bool flipx, flipy;
		u32 code, color;
		u8 tx, ty;
		u16 xsize, ysize;
		s32 sx, sy;
		u16 xzoom, yzoom;
		u32 pri_mask = 0;

		m_sprite[chip]->extract_parameters(source, disable, pri, flipx, flipy, code, color, tx, ty, xsize, ysize, sx, sy, xzoom, yzoom);

		if (disable || !xzoom || !yzoom)
			continue;

		// there are surely also shadows (see gametngk) but how they're enabled we don't know

/*      if (m_flipscreen)
        {
            int yscale = 0x1000000/yzoom;
            int xscale = 0x1000000/xzoom;

            sx = 320 - ((xsize*xscale)>>16) - sx;
            sy = 224 - ((ysize*yscale)>>16) - sy;
            flipx = !flipx;
            flipy = !flipy;
        }*/

		pri >>= 4;
		if (pri == 0x0)
			pri_mask = 0x00;
		else if (pri <= 0xd)
			pri_mask = 0xf0;
		else if (pri <= 0xe)
			pri_mask = 0xfc;
		else
			pri_mask = 0xfe;

		m_sprite[chip]->prio_zoom_transpen(bitmap,cliprect,
				code,
				color,
				flipx, flipy,
				sx,sy,
				tx, ty, xsize, ysize,
				xzoom, yzoom, screen.priority(),pri_mask, 0);
	}   /* end sprite loop */
}


void ms32_bnstars_state::video_start()
{
	m_ascii_tilemap[0] = &machine().tilemap().create(*m_gfxdecode[0], tilemap_get_info_delegate(*this, FUNC(ms32_bnstars_state::get_ascii_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 64,64);
	m_ascii_tilemap[1] = &machine().tilemap().create(*m_gfxdecode[1], tilemap_get_info_delegate(*this, FUNC(ms32_bnstars_state::get_ascii_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 64,64);
	m_ascii_tilemap[0]->set_transparent_pen(0);
	m_ascii_tilemap[1]->set_transparent_pen(0);

	m_scroll_tilemap[0] = &machine().tilemap().create(*m_gfxdecode[0], tilemap_get_info_delegate(*this, FUNC(ms32_bnstars_state::get_scroll_tile_info<0>)), TILEMAP_SCAN_ROWS, 16,16, 64,64);
	m_scroll_tilemap[1] = &machine().tilemap().create(*m_gfxdecode[1], tilemap_get_info_delegate(*this, FUNC(ms32_bnstars_state::get_scroll_tile_info<1>)), TILEMAP_SCAN_ROWS, 16,16, 64,64);
	m_scroll_tilemap[0]->set_transparent_pen(0);
	m_scroll_tilemap[1]->set_transparent_pen(0);

	m_rotate_tilemap[0] = &machine().tilemap().create(*m_gfxdecode[0], tilemap_get_info_delegate(*this, FUNC(ms32_bnstars_state::get_rotate_tile_info<0>)),TILEMAP_SCAN_ROWS, 16,16,128,128);
	m_rotate_tilemap[1] = &machine().tilemap().create(*m_gfxdecode[1], tilemap_get_info_delegate(*this, FUNC(ms32_bnstars_state::get_rotate_tile_info<1>)),TILEMAP_SCAN_ROWS, 16,16,128,128);
	m_rotate_tilemap[0]->set_transparent_pen(0);
	m_rotate_tilemap[1]->set_transparent_pen(0);
}

template <int which> u32 ms32_bnstars_state::screen_update_dual(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	bitmap.fill(0, cliprect);   /* bg color */

	m_scroll_tilemap[which]->set_scrollx(0, m_scroll_ctrl[which][0x00/4] + m_scroll_ctrl[which][0x08/4] + 0x10 );
	m_scroll_tilemap[which]->set_scrolly(0, m_scroll_ctrl[which][0x0c/4] + m_scroll_ctrl[which][0x14/4] );
	m_scroll_tilemap[which]->draw(screen, bitmap, cliprect, 0, 1);

	draw_roz<which>(screen,bitmap,cliprect, 2);

	m_ascii_tilemap[which]->set_scrollx(0, m_ascii_ctrl[which][0x00/4] + m_ascii_ctrl[which][0x08/4] + 0x18);
	m_ascii_tilemap[which]->set_scrolly(0, m_ascii_ctrl[which][0x0c/4] + m_ascii_ctrl[which][0x14/4]);
	m_ascii_tilemap[which]->draw(screen, bitmap, cliprect, 0, 4);

	draw_sprites<which>(screen, bitmap, cliprect);

	return 0;
}

template <int chip> u16 ms32_bnstars_state::palette_ram_r(offs_t offset)
{
	return m_paletteram[chip][offset];
}

template <int chip> void ms32_bnstars_state::palette_ram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_paletteram[chip][offset]);
	// TODO: support for RGB brightness
	u32 pal_data = (m_paletteram[chip][offset | 1] << 16) | m_paletteram[chip][offset & ~1];
	const int b = ((pal_data & 0xff0000) >> 16);
	const int r = ((pal_data & 0x00ff00) >> 8);
	const int g = ((pal_data & 0x0000ff) >> 0);

	m_palette[chip]->set_pen_color(offset >> 1, rgb_t(r, g, b));
}

template <int P>
ioport_value ms32_bnstars_state::mahjong_ctrl_r()
{
	required_ioport_array<4> &keys = (P == 0) ? m_p1_keys : m_p2_keys;
	// different routing than other ms32.cpp mahjong games, using 0x2080 as mask
	u8 which = bitswap<2>(m_bnstars1_mahjong_select, 13, 7);
	return keys[which]->read();
}

void ms32_bnstars_state::bnstars1_mahjong_select_w(u32 data)
{
	m_bnstars1_mahjong_select = data;
//  logerror("%08x\n",m_bnstars1_mahjong_select);
}

void ms32_bnstars_state::bnstars_map(address_map &map)
{
	// TODO: derive from base ms32.cpp memory map
	map(0x00000000, 0x001fffff).rom();

	map(0xfc800000, 0xfc800003).w(FUNC(ms32_bnstars_state::sound_command_w));

	map(0xfcc00004, 0xfcc00007).portr("P1");
	map(0xfcc00008, 0xfcc0000b).portr("P2");
	map(0xfcc00010, 0xfcc00013).portr("DSW");

	map(0xfce00000, 0xfce0005f).m(m_sysctrl, FUNC(jaleco_ms32_sysctrl_device::amap)).umask32(0x0000ffff);
	map(0xfce00200, 0xfce0027f).ram().share(m_sprite_ctrl);
//  map(0xfce00280, 0xfce0028f) // left screen brightness control
//  map(0xfce00300, 0xfce0030f) // right screen brightness control
	map(0xfce00400, 0xfce0045f).writeonly().share(m_rotate_ctrl[0]);
	map(0xfce00700, 0xfce0075f).writeonly().share(m_rotate_ctrl[1]); // guess
	map(0xfce00a00, 0xfce00a17).writeonly().share(m_ascii_ctrl[0]);
	map(0xfce00a20, 0xfce00a37).writeonly().share(m_scroll_ctrl[0]);
	map(0xfce00c00, 0xfce00c17).writeonly().share(m_ascii_ctrl[1]);
	map(0xfce00c20, 0xfce00c37).writeonly().share(m_scroll_ctrl[1]);

	map(0xfce00e00, 0xfce00e03).w(FUNC(ms32_bnstars_state::bnstars1_mahjong_select_w));

	map(0xfd000000, 0xfd000003).r(FUNC(ms32_bnstars_state::sound_result_r));

	/* wrote together */
	map(0xfd040000, 0xfd047fff).ram(); // priority ram
	map(0xfd080000, 0xfd087fff).ram();
	// for some reason ***38*** isn't accessed by these two
	map(0xfd200000, 0xfd23ffff).rw(FUNC(ms32_bnstars_state::palette_ram_r<1>), FUNC(ms32_bnstars_state::palette_ram_w<1>)).umask32(0x0000ffff);
	map(0xfd400000, 0xfd43ffff).rw(FUNC(ms32_bnstars_state::palette_ram_r<0>), FUNC(ms32_bnstars_state::palette_ram_w<0>)).umask32(0x0000ffff);

	map(0xfe000000, 0xfe01ffff).rw(FUNC(ms32_bnstars_state::rotate_vram_r<1>), FUNC(ms32_bnstars_state::rotate_vram_w<1>)).umask32(0x0000ffff);
	map(0xfe400000, 0xfe41ffff).rw(FUNC(ms32_bnstars_state::rotate_vram_r<0>), FUNC(ms32_bnstars_state::rotate_vram_w<0>)).umask32(0x0000ffff);

	// TODO: ms32_state should internalize sprite RAM interfaces, also NOP to $ffffxxxx
	map(0xfe800000, 0xfe81ffff).rw(FUNC(ms32_bnstars_state::object_vram_r<0>), FUNC(ms32_bnstars_state::object_vram_w<0>)).umask32(0x0000ffff);
	map(0xfe820000, 0xfe83ffff).rw(FUNC(ms32_bnstars_state::object_vram_r<1>), FUNC(ms32_bnstars_state::object_vram_w<1>)).umask32(0x0000ffff);

	map(0xfea00000, 0xfea07fff).rw(FUNC(ms32_bnstars_state::ascii_vram_r<1>), FUNC(ms32_bnstars_state::ascii_vram_w<1>)).umask32(0x0000ffff);
	map(0xfea08000, 0xfea0ffff).rw(FUNC(ms32_bnstars_state::scroll_vram_r<1>), FUNC(ms32_bnstars_state::scroll_vram_w<1>)).umask32(0x0000ffff);
	map(0xfec00000, 0xfec07fff).rw(FUNC(ms32_bnstars_state::ascii_vram_r<0>), FUNC(ms32_bnstars_state::ascii_vram_w<0>)).umask32(0x0000ffff);
	map(0xfec08000, 0xfec0ffff).rw(FUNC(ms32_bnstars_state::scroll_vram_r<0>), FUNC(ms32_bnstars_state::scroll_vram_w<0>)).umask32(0x0000ffff);

	map(0xfee00000, 0xfee1ffff).ram();
	map(0xffe00000, 0xffffffff).rom().region("maincpu", 0);
}

void ms32_bnstars_state::bnstars_sound_map(address_map &map)
{
	base_sound_map(map);
	map(0x3f00, 0x3f0f).rw(m_ymf[1], FUNC(ymf271_device::read), FUNC(ymf271_device::write));
	map(0x3f20, 0x3f2f).rw(m_ymf[0], FUNC(ymf271_device::read), FUNC(ymf271_device::write));
}


static INPUT_PORTS_START( bnstars )
	PORT_START("P1")
	PORT_BIT( 0x000000ff, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(ms32_bnstars_state::mahjong_ctrl_r<0>))
	PORT_BIT( 0x0000ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Test?") PORT_CODE(KEYCODE_F1)

	PORT_START("P1KEY.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1KEY.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1KEY.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1KEY.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x000000ff, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(ms32_bnstars_state::mahjong_ctrl_r<1>))
	PORT_BIT( 0x0000ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Test?") PORT_CODE(KEYCODE_F2)

	PORT_START("P2KEY.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2KEY.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2KEY.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2KEY.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME(     0x00000001, 0x00000001, "Test Mode" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(  0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000002, 0x00000002, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(  0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x0000001c, 0x0000001c, "First Point" ) PORT_DIPLOCATION("SW1:6,5,4")
	PORT_DIPSETTING(  0x0000000c, "5000"  )
	PORT_DIPSETTING(  0x00000014, "10000" )
	PORT_DIPSETTING(  0x00000004, "15000" )
	PORT_DIPSETTING(  0x0000001c, "20000" )
	PORT_DIPSETTING(  0x00000018, "23000" )
	PORT_DIPSETTING(  0x00000008, "25000" )
	PORT_DIPSETTING(  0x00000010, "26000" )
	PORT_DIPSETTING(  0x00000000, "30000" )
	PORT_DIPNAME(     0x000000e0, 0x000000e0, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:3,2,1")
	PORT_DIPSETTING(  0x00000000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(  0x00000080, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(  0x00000040, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(  0x000000c0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(  0x000000e0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(  0x00000060, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(  0x000000a0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(  0x00000020, DEF_STR( 1C_4C ) )

	PORT_DIPUNUSED_DIPLOC( 0x00000100, 0x00000100, "SW2:8" )
	PORT_DIPUNUSED_DIPLOC( 0x00000200, 0x00000200, "SW2:7" )
	PORT_DIPNAME(     0x00000400, 0x00000400, "Taisen Only" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(  0x00000400, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000000, DEF_STR( On ) )
	PORT_DIPNAME(     0x00000800, 0x00000800, "Tumo Pinfu" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(  0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00000800, DEF_STR( On ) )
	PORT_DIPNAME(     0x00001000, 0x00001000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(  0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(  0x00001000, DEF_STR( On ) )
	PORT_DIPNAME(     0x0000e000, 0x0000e000, DEF_STR ( Difficulty ) ) PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(  0x00000000, DEF_STR( Easiest ) )
	PORT_DIPSETTING(  0x00008000, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(  0x00004000, DEF_STR( Easier ) )
	PORT_DIPSETTING(  0x0000c000, DEF_STR( Easy ) )
	PORT_DIPSETTING(  0x0000e000, DEF_STR( Normal ) )
	PORT_DIPSETTING(  0x00006000, DEF_STR( Hard ) )
	PORT_DIPSETTING(  0x0000a000, DEF_STR( Harder ) )
	PORT_DIPSETTING(  0x00002000, DEF_STR( Hardest ) )

	PORT_DIPUNUSED_DIPLOC( 0x00010000, 0x00010000, "SW3:8" )
	PORT_DIPUNUSED_DIPLOC( 0x00020000, 0x00020000, "SW3:7" )
	PORT_DIPUNUSED_DIPLOC( 0x00040000, 0x00040000, "SW3:6" )
	PORT_DIPUNUSED_DIPLOC( 0x00080000, 0x00080000, "SW3:5" )
	PORT_DIPUNUSED_DIPLOC( 0x00100000, 0x00100000, "SW3:4" )
	PORT_DIPUNUSED_DIPLOC( 0x00200000, 0x00200000, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x00400000, 0x00400000, "SW3:2" )
	PORT_DIPUNUSED_DIPLOC( 0x00800000, 0x00800000, "SW3:1" )
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNUSED )   // Unused?
INPUT_PORTS_END

static GFXDECODE_START( gfx_bnstars_left )
	GFXDECODE_ENTRY( "roztiles_l", 0, gfx_16x16x8_raw, 0x5000, 0x10 ) /* Roz scr1 */
	GFXDECODE_ENTRY( "bgtiles_l",  0, gfx_16x16x8_raw, 0x1000, 0x10 ) /* Bg scr1 */
	GFXDECODE_ENTRY( "txtiles_l",  0, gfx_8x8x8_raw,   0x6000, 0x10 ) /* Tx scr1 */
GFXDECODE_END

static GFXDECODE_START( gfx_bnstars_right )
	GFXDECODE_ENTRY( "roztiles_r", 0, gfx_16x16x8_raw, 0x5000, 0x10 ) /* Roz scr2 */
	GFXDECODE_ENTRY( "bgtiles_r",  0, gfx_16x16x8_raw, 0x1000, 0x10 ) /* Bg scr2 */
	GFXDECODE_ENTRY( "txtiles_r",  0, gfx_8x8x8_raw,   0x6000, 0x10 ) /* Tx scr2 */
GFXDECODE_END

void ms32_bnstars_state::bnstars(machine_config &config)
{
	V70(config, m_maincpu, XTAL(40'000'000)/2); // 20MHz (40MHz / 2)
	m_maincpu->set_addrmap(AS_PROGRAM, &ms32_bnstars_state::bnstars_map);
	m_maincpu->set_irq_acknowledge_callback(FUNC(ms32_bnstars_state::irq_callback));

	Z80(config, m_audiocpu, XTAL(8'000'000)); // 8MHz present on sound PCB, Verified
	m_audiocpu->set_addrmap(AS_PROGRAM, &ms32_bnstars_state::bnstars_sound_map);

	config.set_maximum_quantum(attotime::from_hz(60000));

	config.set_default_layout(layout_dualhsxs);

	for (unsigned i=0; i < 2; i++)
	{
		PALETTE(config, m_palette[i]).set_entries(0x8000);

		SCREEN(config, m_screen[i], SCREEN_TYPE_RASTER);
		m_screen[i]->set_raw(XTAL(48'000'000)/8, 384, 0, 320, 263, 0, 224); // default CRTC setup
		m_screen[i]->set_palette(m_palette[i]);

		JALECO_MEGASYSTEM32_SPRITE(config, m_sprite[i], XTAL(48'000'000)); // 48MHz for video?
		m_sprite[i]->set_palette(m_palette[i]);
		m_sprite[i]->set_color_base(0);
		m_sprite[i]->set_color_entries(16);
	}

	GFXDECODE(config, m_gfxdecode[0], m_palette[0], gfx_bnstars_left);
	GFXDECODE(config, m_gfxdecode[1], m_palette[1], gfx_bnstars_right);
	m_screen[0]->set_screen_update(FUNC(ms32_bnstars_state::screen_update_dual<0>));
	m_screen[1]->set_screen_update(FUNC(ms32_bnstars_state::screen_update_dual<1>));

	JALECO_MS32_SYSCTRL(config, m_sysctrl, XTAL(48'000'000), m_screen[0]);
	m_sysctrl->flip_screen_cb().set(FUNC(ms32_bnstars_state::flipscreen_dual_w));
	m_sysctrl->vblank_cb().set(FUNC(ms32_bnstars_state::vblank_irq_w));
	m_sysctrl->field_cb().set(FUNC(ms32_bnstars_state::field_irq_w));
	m_sysctrl->prg_timer_cb().set(FUNC(ms32_bnstars_state::timer_irq_w));
	m_sysctrl->sound_ack_cb().set(FUNC(ms32_bnstars_state::sound_ack_w));
	m_sysctrl->sound_reset_cb().set(FUNC(ms32_bnstars_state::sound_reset_line_w));
//  TODO: runs better with this on but eventually game crashes during match presentation
//  (may be due of the field irq positioning)
//  m_sysctrl->set_invert_vblank_lines(true);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	YMF271(config, m_ymf[0], XTAL(16'934'400)); // 16.9344MHz
	m_ymf[0]->add_route(0, "speaker", 1.0, 0);
	m_ymf[0]->add_route(1, "speaker", 1.0, 1);
// Output 2/3 not used?
//  m_ymf[0]->add_route(2, "speaker", 1.0);
//  m_ymf[0]->add_route(3, "speaker", 1.0);

	YMF271(config, m_ymf[1], XTAL(16'934'400)); // 16.9344MHz
	m_ymf[1]->add_route(0, "speaker", 1.0, 0);
	m_ymf[1]->add_route(1, "speaker", 1.0, 1);
// Output 2/3 not used?
//  m_ymf[1]->add_route(2, "speaker", 1.0);
//  m_ymf[1]->add_route(3, "speaker", 1.0);
}


ROM_START( bnstars1 )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* V70 code */
	ROM_LOAD32_BYTE( "mb-93142.36", 0x000003, 0x80000, CRC(2eb6a503) SHA1(27c02ab1b4321924fd4499844467ea4dc97de25d) )
	ROM_LOAD32_BYTE( "mb-93142.37", 0x000002, 0x80000, CRC(49f60882) SHA1(2ff5b0989aaf970103304a453773e0b9517ebb8d) )
	ROM_LOAD32_BYTE( "mb-93142.38", 0x000001, 0x80000, CRC(6e1312cd) SHA1(4c22f8f9f1574eefd96147453cf240f50c17f5dc) )
	ROM_LOAD32_BYTE( "mb-93142.39", 0x000000, 0x80000, CRC(56b98539) SHA1(5eb0e77729b31e6a100c1b43813a39fea57bedee) )

	/* Sprites - shared by both screens? */
	ROM_REGION( 0x1000000, "sprite.0", 0 ) /* sprites */
	ROM_LOAD32_WORD( "mr96004-01.20",   0x0000000, 0x200000, CRC(3366d104) SHA1(2de0cabe2ead777b5b02cade7f2003ef7f90b75b) )
	ROM_LOAD32_WORD( "mr96004-02.28",   0x0000002, 0x200000, CRC(ad556664) SHA1(4b36f8d8d9efa37cf515af41d14433e7eafa27a2) )
	ROM_LOAD32_WORD( "mr96004-03.21",   0x0400000, 0x200000, CRC(b399e2b1) SHA1(9b6a00a219db8d66dcf592160b7b5f7a86b8f0c9) )
	ROM_LOAD32_WORD( "mr96004-04.29",   0x0400002, 0x200000, CRC(f4f4cf4a) SHA1(fe497989cf96c68602f68f14920aed44fd934573) )
	ROM_LOAD32_WORD( "mr96004-05.22",   0x0800000, 0x200000, CRC(cd6c357e) SHA1(44cd2d0607c7ccd80f701cf1675fd283acb07252) )
	ROM_LOAD32_WORD( "mr96004-06.30",   0x0800002, 0x200000, CRC(fc6daad7) SHA1(99f14ac6b06ad9a8a3d2e9f69b693c7ce420a47d) )
	ROM_LOAD32_WORD( "mr96004-07.23",   0x0c00000, 0x200000, CRC(177e32fa) SHA1(3ca1f397dc28f1fa3a4136705b92c63e4e438f05) )
	ROM_LOAD32_WORD( "mr96004-08.31",   0x0c00002, 0x200000, CRC(f6df27b2) SHA1(60590976020d86bdccd4eaf57b349ea31bec6830) )

	ROM_REGION( 0x1000000, "sprite.1", 0 )
	ROM_COPY( "sprite.0", 0, 0, 0x1000000)

	/* Roz Tiles #1 (Screen 1) */
	ROM_REGION( 0x400000, "roztiles_l", 0 ) /* roz tiles */
	ROM_LOAD( "mr96004-09.1", 0x000000, 0x400000, CRC(7f8ea9f0) SHA1(f1fe682dcb884f1aa4a5536e17ab94157a99f519) )

	/* Roz Tiles #2 (Screen 2) */
	ROM_REGION( 0x400000, "roztiles_r", 0 ) /* roz tiles */
	ROM_LOAD( "mr96004-09.7", 0x000000, 0x400000, CRC(7f8ea9f0) SHA1(f1fe682dcb884f1aa4a5536e17ab94157a99f519) )

	/* BG Tiles #1 (Screen 1?) */
	ROM_REGION( 0x200000, "bgtiles_l", 0 ) /* bg tiles */
	ROM_LOAD( "mr96004-11.11", 0x000000, 0x200000,  CRC(e6da552c) SHA1(69a5af3015883793c7d1343243ccae23db9ef77c) )

	/* TX Tiles #1 (Screen 1?) */
	ROM_REGION( 0x080000, "txtiles_l", 0 ) /* tx tiles */
	ROM_LOAD( "vsjanshi6.5", 0x000000, 0x080000, CRC(fdbbac21) SHA1(c77d852e53126cc8ebfe1e79d1134e42b54d1aab) )

	/* BG Tiles #2 (Screen 2?) */
	ROM_REGION( 0x200000, "bgtiles_r", 0 ) /* bg tiles */
	ROM_LOAD( "mr96004-11.13", 0x000000, 0x200000, CRC(e6da552c) SHA1(69a5af3015883793c7d1343243ccae23db9ef77c) )

	/* TX Tiles #2 (Screen 2?) */
	ROM_REGION( 0x080000, "txtiles_r", 0 ) /* tx tiles */
	ROM_LOAD( "vsjanshi5.6", 0x000000, 0x080000, CRC(fdbbac21) SHA1(c77d852e53126cc8ebfe1e79d1134e42b54d1aab) )

	/* Sound Program (one, driving both screen sound) */
	ROM_REGION( 0x40000, "audiocpu", 0 ) /* z80 program */
	ROM_LOAD( "sb93145.5",  0x000000, 0x040000, CRC(0424e899) SHA1(fbcdebfa3d5f52b10cf30f7e416f5f53994e4d55) )

	/* Samples #1 (Screen 1?) */
	ROM_REGION( 0x400000, "ymf.1", 0 ) /* samples - 8-bit signed PCM */
	ROM_LOAD( "mr96004-10.1",  0x000000, 0x400000, CRC(83f4303a) SHA1(90ee010591afe1d35744925ef0e8d9a7e2ef3378) )

	/* Samples #2 (Screen 2?) */
	ROM_REGION( 0x400000, "ymf.2", 0 ) /* samples - 8-bit signed PCM */
	ROM_LOAD( "mr96004-10.1",  0x000000, 0x400000, CRC(83f4303a) SHA1(90ee010591afe1d35744925ef0e8d9a7e2ef3378) )
ROM_END


/* SS92046_01: bbbxing, f1superb, tetrisp, hayaosi1 */
void ms32_bnstars_state::init_bnstars()
{
	decrypt_ms32_tx(machine(), 0x00020,0x7e, "txtiles_l");
	decrypt_ms32_bg(machine(), 0x00001,0x9b, "bgtiles_l");
	decrypt_ms32_tx(machine(), 0x00020,0x7e, "txtiles_r");
	decrypt_ms32_bg(machine(), 0x00001,0x9b, "bgtiles_r");

	configure_banks();
}

} // anonymous namespace


GAME( 1997, bnstars1, 0, bnstars, bnstars, ms32_bnstars_state, init_bnstars, ROT0, "Jaleco", "Vs. Janshi Brandnew Stars", MACHINE_IMPERFECT_GRAPHICS )
