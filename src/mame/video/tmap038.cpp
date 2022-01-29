// license:BSD-3-Clause
// copyright-holders:Luca Elia, Paul Priest, David Haywood
/* 038 Tilemap generator manufactured by NEC */

/*
    [ Scrolling Layers ]

        Each 038 chip generates 1 layer. Up to 4 chips are used
        (4 layers)

        Layer Size:             512 x 512
        Tiles:                  8 x 8 & 16 x 16.

        There are 2 tilemaps in memory, one per tiles dimension.
        A bit decides which one gets displayed.
        The tiles depth varies with games, from 16 to 256 colors.

        A per layer row-scroll / row-select effect can be enabled:

        a different scroll value is fetched (from tile RAM) for each
        scan line, and a different tilemap line for each scan line



***************************************************************************

                        Callbacks for the TileMap code

                              [ Tiles Format ]

    Offset:     Bits:                   Value:

    0.w         fe-- ---- ---- ---      Priority
                --dc ba98 ---- ----     Color
                ---- ---- 7654 3210

    2.w                                 Code


    When a row-scroll / row-select effect is enabled, the scroll values are
    fetched starting from tile RAM + $1000, 4 bytes per scan line:

    Offset:     Value:

    0.w         Tilemap line to display
    2.w         X Scroll value

***************************************************************************

***************************************************************************

                            Layers Registers


        Offset:     Bits:                   Value:

        0.w         f--- ---- ---- ----     0 = Layer Flip X
                    -e-- ---- ---- ----     Activate Row-scroll
                    --d- ---- ---- ----
                    ---c ba9- ---- ----
                    ---- ---8 7654 3210     Scroll X

        2.w         f--- ---- ---- ----     0 = Layer Flip Y
                    -e-- ---- ---- ----     Activate Row-select
                    --d- ---- ---- ----     0 = 8x8 tiles, 1 = 16x16 tiles
                    ---c ba9- ---- ----
                    ---- ---8 7654 3210     Scroll Y

        4.w         fedc ba98 765- ----
                    ---- ---- ---4 ----     Layer Disable
                    ---- ---- ---- 3210     Varies*

        *color bank for mcatadv or Layer-Layer priority for cave

There are more!

***************************************************************************

TODO:
    de-fragmentation and merge drawing behavior into tmap038.cpp

***************************************************************************

*/

#include "emu.h"
#include "tmap038.h"

void tilemap038_device::vram_map(address_map &map)
{
	map(0x0000, 0x0fff).rw(FUNC(tilemap038_device::vram_16x16_r), FUNC(tilemap038_device::vram_16x16_w)).share("vram_16x16");
	map(0x1000, 0x17ff).rw(FUNC(tilemap038_device::lineram_r), FUNC(tilemap038_device::lineram_w)).share("lineram");
	map(0x1800, 0x3fff).ram().share("scratchpad"); // scratchpad?
	map(0x4000, 0x7fff).rw(FUNC(tilemap038_device::vram_8x8_r), FUNC(tilemap038_device::vram_8x8_w)).share("vram_8x8");
}

void tilemap038_device::vram_writeonly_map(address_map &map)
{
	map(0x0000, 0x0fff).w(FUNC(tilemap038_device::vram_16x16_w)).share("vram_16x16");
	map(0x1000, 0x17ff).w(FUNC(tilemap038_device::lineram_w)).share("lineram");
	map(0x1800, 0x3fff).writeonly().share("scratchpad"); // scratchpad?
	map(0x4000, 0x7fff).w(FUNC(tilemap038_device::vram_8x8_w)).share("vram_8x8");
}

void tilemap038_device::vram_16x16_map(address_map &map)
{
	map(0x0000, 0x0fff).rw(FUNC(tilemap038_device::vram_16x16_r), FUNC(tilemap038_device::vram_16x16_w)).share("vram_16x16");
	map(0x1000, 0x17ff).rw(FUNC(tilemap038_device::lineram_r), FUNC(tilemap038_device::lineram_w)).share("lineram");
	map(0x1800, 0x3fff).ram().share("scratchpad"); // scratchpad?
}

void tilemap038_device::vram_16x16_writeonly_map(address_map &map)
{
	map(0x0000, 0x0fff).w(FUNC(tilemap038_device::vram_16x16_w)).share("vram_16x16");
	map(0x1000, 0x17ff).w(FUNC(tilemap038_device::lineram_w)).share("lineram");
	map(0x1800, 0x3fff).writeonly().share("scratchpad"); // scratchpad?
}

/*  Some games, that only ever use the 8x8 tiles and no line scroll,
    use mirror ram. For example in donpachi, writes to 400000-403fff
    and 408000-407fff both go to the 8x8 tilemap ram. Use this function
    in this cases. Note that the get_tile_info function looks in the
    4000-7fff range for tiles, so we have to write the data there. */
void tilemap038_device::vram_8x8_map(address_map &map)
{
	map(0x0000, 0x3fff).rw(FUNC(tilemap038_device::vram_8x8_r), FUNC(tilemap038_device::vram_8x8_w)).share("vram_8x8");
}

DEFINE_DEVICE_TYPE(TMAP038, tilemap038_device, "tmap038", "038 Tilemap generator")

tilemap038_device::tilemap038_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, TMAP038, tag, owner, clock)
	, m_vram_8x8(*this, "vram_8x8")
	, m_vram_16x16(*this, "vram_16x16")
	, m_lineram(*this, "lineram")
	, m_vregs(nullptr)
	, m_tiledim(false)
	, m_gfxdecode(*this, finder_base::DUMMY_TAG)
	, m_gfxno(0)
	, m_038_cb(*this)
	, m_xoffs(0)
	, m_flipped_xoffs(0)
	, m_yoffs(0)
	, m_flipped_yoffs(0)
{
}

TILE_GET_INFO_MEMBER(tilemap038_device::get_tile_info)
{
	u32 tile, code = 0, color = 0, pri = 0;

	if (tile_is_16x16())
	{
		tile  = (tile_index % (512 / 8)) / 2 + ((tile_index / (512 / 8)) / 2) * (512 / 16);
		tile  = (m_vram_16x16 != nullptr) ? ((m_vram_16x16[tile * 2] << 16) + m_vram_16x16[(tile * 2) + 1]) : 0;

		color = (tile & 0x3f000000) >> (32 - 8);
		pri   = (tile & 0xc0000000) >> (32 - 2);
		code  = (tile & 0x0000ffff) * 4;

		code ^= tile_index & 1;
		code ^= ((tile_index / (512 / 8)) & 1) * 2;

		if (!m_038_cb.isnull())
			m_038_cb(true, color, pri, code);
	}
	else if (tile_is_8x8())
	{
		tile  = (m_vram_8x8 != nullptr) ? ((m_vram_8x8[tile_index * 2] << 16) + m_vram_8x8[(tile_index * 2) + 1]) : 0;

		color = (tile & 0x3f000000) >> (32 - 8);
		pri   = (tile & 0xc0000000) >> (32 - 2);
		code  = (tile & 0x0003ffff);

		if (!m_038_cb.isnull())
			m_038_cb(false, color, pri, code);
	}

	tileinfo.set(m_gfxno, code, color, 0);
	tileinfo.category = pri;
}


void tilemap038_device::device_start()
{
	m_038_cb.resolve();
	m_vregs = make_unique_clear<u16[]>(0x6/2);

	if (m_vram_16x16 == nullptr && m_vram_8x8 == nullptr)
		fatalerror("Tilemap 038 %s: VRAM not found",this->tag());

	m_tmap = &machine().tilemap().create(
			*m_gfxdecode,
			tilemap_get_info_delegate(*this, FUNC(tilemap038_device::get_tile_info)),
			TILEMAP_SCAN_ROWS,
			8,8, 512 / 8,512 / 8);

	m_tmap->set_transparent_pen(0);

	set_scroll_rows(1);  // Initialize scroll rows
	set_scroll_cols(1);  // Initialize scroll cols

	save_item(NAME(m_tiledim));
	save_pointer(NAME(m_vregs), 0x6/2);
}

void tilemap038_device::device_reset()
{
}

u16 tilemap038_device::vram_8x8_r(offs_t offset)
{
	return m_vram_8x8[offset];
}

void tilemap038_device::vram_8x8_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_vram_8x8[offset]);
	if (tile_is_8x8())
		m_tmap->mark_tile_dirty(offset >> 1);
}

u16 tilemap038_device::vram_16x16_r(offs_t offset)
{
	return m_vram_16x16[offset];
}

void tilemap038_device::vram_16x16_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_vram_16x16[offset]);
	if (tile_is_16x16())
	{
		offset >>= 1;
		offset = (offset % (512 / 16)) * 2 + (offset / (512 / 16)) * (512 / 8) * 2;
		m_tmap->mark_tile_dirty(offset + 0);
		m_tmap->mark_tile_dirty(offset + 1);
		m_tmap->mark_tile_dirty(offset + 0 + 512 / 8);
		m_tmap->mark_tile_dirty(offset + 1 + 512 / 8);
	}
}

void tilemap038_device::prepare()
{
	// refresh tile size
	if (m_vram_8x8 != nullptr && m_vram_16x16 != nullptr)
	{
		const bool new_tiledim = BIT(m_vregs[1], 13);
		if (m_tiledim != new_tiledim)
		{
			m_tmap->mark_all_dirty();
			m_tiledim = new_tiledim;
		}
	}
}

void tilemap038_device::draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u32 flags, u8 pri, u8 pri_mask) { draw_common(screen, bitmap, cliprect, flags, pri, pri_mask); }
void tilemap038_device::draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, u32 flags, u8 pri, u8 pri_mask) { draw_common(screen, bitmap, cliprect, flags, pri, pri_mask); }

template<class BitmapClass>
void tilemap038_device::draw_common(screen_device &screen, BitmapClass &bitmap, const rectangle &cliprect, u32 flags, u8 pri, u8 pri_mask)
{
	m_tmap->draw(screen, bitmap, cliprect, flags, pri, pri_mask);
}
