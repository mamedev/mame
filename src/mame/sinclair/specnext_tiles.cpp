// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************
    Spectrum Next Tiles
**********************************************************************/

#include "emu.h"
#include "specnext_tiles.h"

#include "screen.h"
#include "tilemap.h"


static const gfx_layout gfx_8x8x4 =
{
	8, 8, 256 * 2, 4,
	{ STEP4(0, 1) },
	{ STEP8(0, 4) },
	{ STEP8(0, 4 * 8) },
	4 * 8 * 8
};

static const gfx_layout gfx_8x8x4_r =
{
	8, 8, 256 * 2, 4,
	{ STEP4(0, 1) },
	{ STEP8(4 * 8 * 7, -4 * 8) },
	{ STEP8(0, 4) },
	4 * 8 * 8
};

static const gfx_layout gfx_text =
{
	8, 8, 256 * 2, 1,
	{ 0 },
	{ STEP8(0, 1) },
	{ STEP8(0, 8) },
	8 * 8
};

static GFXDECODE_START( gfx_tiles )
	GFXDECODE_SCALE( nullptr, 0, gfx_8x8x4, 0, 16, 2, 1 )   // 40x32
	GFXDECODE_ENTRY( nullptr, 0, gfx_8x8x4, 0, 16 )         // 80x32
	GFXDECODE_SCALE( nullptr, 0, gfx_8x8x4_r, 0, 16, 2, 1 ) // 40x32 rotated
	GFXDECODE_ENTRY( nullptr, 0, gfx_8x8x4_r, 0, 16 )       // 80x32 rotated
	GFXDECODE_SCALE( nullptr, 0, gfx_text, 0, 128, 2, 1 )   // 40x32
	GFXDECODE_ENTRY( nullptr, 0, gfx_text, 0, 128 )         // 80x32
GFXDECODE_END

specnext_tiles_device::specnext_tiles_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SPECNEXT_TILES, tag, owner, clock)
	, device_gfx_interface(mconfig, *this, gfx_tiles)
{
}

specnext_tiles_device &specnext_tiles_device::set_palette(const char *tag, u16 base_offset, u16 alt_offset)
{
	device_gfx_interface::set_palette(tag);
	m_palette_base_offset = base_offset,
	m_palette_alt_offset = alt_offset;
	tilemap_update();

	return *this;
}

TILE_GET_INFO_MEMBER(specnext_tiles_device::get_tile_info)
{
	const bool strip_flags_n = BIT(~m_control, 5);
	const u8 *data = &m_tiles_info[tile_index << (strip_flags_n ? 1 : 0)];
	u8 attr = strip_flags_n ? data[strip_flags_n] : m_default_flags;
	u16 code = data[0];
	u32 category = 1;

	if (BIT(m_control, 1))
		code |= BIT(attr, 0) << 8;
	else
		category = BIT(attr, 0) ? 1 : 2;

	const bool tm_on_top = BIT(m_control, 0); // tilemap always on top of ula
	tileinfo.category = tm_on_top ? 2 : category;

	if (BIT(m_control, 3)) // textmode
	{
		const rgb_t gt0 = rgbexpand<3,3,3>((m_global_transparent << 1) | 0, 6, 3, 0);
		const rgb_t gt1 = rgbexpand<3,3,3>((m_global_transparent << 1) | 1, 6, 3, 0);
		const u8 pal_offset = BIT(attr, 1, 7);
		const rgb_t bg = palette().pen(m_tilemap[1]->palette_offset() + (pal_offset << 1));
		const rgb_t fg = palette().pen(m_tilemap[1]->palette_offset() + (pal_offset << 1) + 1);

		u8 group = 0;
		if (bg == gt0 || bg == gt1) group |= 2;
		if (fg == gt0 || fg == gt1) group |= 1;

		tileinfo.set(4 | BIT(m_control, 6), code, pal_offset, 0);
		tileinfo.group = group;
	}
	else
		tileinfo.set((BIT(attr, 1) << 1) | BIT(m_control, 6), code, BIT(attr, 4, 4), (TILE_FLIPY * BIT(attr, 2) | (TILE_FLIPX * BIT(attr, 3))));
}

void specnext_tiles_device::tilemap_update()
{
	if (gfx(0) == nullptr) return;

	const u8 *tiles_offset = m_host_ram_ptr + ((BIT(m_tm_tile_base, 6) ? 7 : 5) << 14) + ((m_tm_tile_base & 0x3f) << 8);
	for (auto i = 0; i < 6; ++i)
	{
		gfx(i)->set_source(tiles_offset);
		gfx(i)->set_granularity(i < 4 ? 16 : 2);
	}
	for (auto i = 0; i < 2; ++i)
	{
		m_tilemap[i]->set_palette_offset(BIT(m_control, 4) ? m_palette_alt_offset : m_palette_base_offset);
		m_tilemap[i]->set_scrollx(-m_offset_h + (m_tm_scroll_x << (1 - i)));
		m_tilemap[i]->set_scrolly(-m_offset_v + m_tm_scroll_y);
		m_tilemap[i]->mark_mapping_dirty();
		m_tilemap[i]->mark_all_dirty();

		if (BIT(m_control, 3))
		{
			m_tilemap[i]->map_pen_to_layer(0, 0, TILEMAP_PIXEL_LAYER0);
			m_tilemap[i]->map_pen_to_layer(0, 1, TILEMAP_PIXEL_LAYER0);
			m_tilemap[i]->map_pen_to_layer(1, 0, TILEMAP_PIXEL_LAYER0);
			m_tilemap[i]->map_pen_to_layer(1, 1, TILEMAP_PIXEL_TRANSPARENT);
			m_tilemap[i]->map_pen_to_layer(2, 0, TILEMAP_PIXEL_TRANSPARENT);
			m_tilemap[i]->map_pen_to_layer(2, 1, TILEMAP_PIXEL_LAYER0);
			m_tilemap[i]->map_pen_to_layer(3, 0, TILEMAP_PIXEL_TRANSPARENT);
			m_tilemap[i]->map_pen_to_layer(3, 1, TILEMAP_PIXEL_TRANSPARENT);
		}
		else
		{
			m_tilemap[i]->set_transparent_pen(m_transp_colour);
		}
	}

	m_tiles_info = m_host_ram_ptr;
	if (BIT(m_tm_map_base, 6))
		m_tiles_info += (7 << 14) + ((m_tm_map_base & 0x1f) << 8);
	else
		m_tiles_info += (5 << 14) + ((m_tm_map_base & 0x3f) << 8);
}

void specnext_tiles_device::draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, u32 flags, u8 pcode, u8 priority_mask)
{
	rectangle clip = rectangle{ m_clip_x1 << 1, (m_clip_x2 << 1) | 1, m_clip_y1, m_clip_y2 };
	clip &= SCREEN_AREA;
	clip.setx(clip.left() << 1, (clip.right() << 1) | 1);
	clip.offset(m_offset_h, m_offset_v);
	clip &= cliprect;

	if (!clip.empty())
		m_tilemap[BIT(m_control, 6)]->draw(screen, bitmap, clip, flags, pcode, priority_mask);
}

void specnext_tiles_device::device_add_mconfig(machine_config &config)
{
	m_offset_h = 0;
	m_offset_v = 0;
	m_global_transparent = 0xaa;
}

void specnext_tiles_device::device_start()
{
	m_tilemap[0] = &machine().tilemap().create(*this
		, tilemap_get_info_delegate(*this, FUNC(specnext_tiles_device::get_tile_info))
		, TILEMAP_SCAN_ROWS, 16, 8, 40, 32);
	m_tilemap[1] = &machine().tilemap().create(*this
		, tilemap_get_info_delegate(*this, FUNC(specnext_tiles_device::get_tile_info))
		, TILEMAP_SCAN_ROWS, 8, 8, 80, 32);

	save_item(NAME(m_offset_h));
	save_item(NAME(m_offset_v));
	save_item(NAME(m_global_transparent));
	save_item(NAME(m_tm_palette_select));
	save_item(NAME(m_control));
	save_item(NAME(m_default_flags));
	save_item(NAME(m_transp_colour));
	save_item(NAME(m_tm_map_base));
	save_item(NAME(m_tm_tile_base));
	save_item(NAME(m_tm_scroll_x));
	save_item(NAME(m_tm_scroll_y));
	save_item(NAME(m_clip_x1));
	save_item(NAME(m_clip_x2));
	save_item(NAME(m_clip_y1));
	save_item(NAME(m_clip_y2));

	tilemap_update();
}

// device type definition
DEFINE_DEVICE_TYPE(SPECNEXT_TILES, specnext_tiles_device, "tiles", "Spectrum Next Tiles")
