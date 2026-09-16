// license:BSD-3-Clause
// copyright-holders: Luca Elia, David Haywood

/*
    Fuuki Tilemap hardware

    Used by:
    fuukifg2.cpp
    fuukifg3.cpp
*/

#include "emu.h"
#include "fuukitmap.h"

#include <algorithm>


void fuukitmap_device::vram_map(address_map &map)
{
	map(0x0000, 0x1fff).rw(FUNC(fuukitmap_device::vram_r<0>), FUNC(fuukitmap_device::vram_w<0>));
	map(0x2000, 0x3fff).rw(FUNC(fuukitmap_device::vram_r<1>), FUNC(fuukitmap_device::vram_w<1>));
	map(0x4000, 0x5fff).rw(FUNC(fuukitmap_device::vram_r<2>), FUNC(fuukitmap_device::vram_buffered_w<2>));
	map(0x6000, 0x7fff).rw(FUNC(fuukitmap_device::vram_r<3>), FUNC(fuukitmap_device::vram_buffered_w<3>));
}

void fuukitmap_device::vregs_map(address_map &map)
{
	map(0x00000, 0x0001f).rw(FUNC(fuukitmap_device::vregs_r), FUNC(fuukitmap_device::vregs_w));
	map(0x10000, 0x10003).rw(FUNC(fuukitmap_device::unknown_r), FUNC(fuukitmap_device::unknown_w)); // Flipscreen related
	map(0x20000, 0x20001).rw(FUNC(fuukitmap_device::priority_r), FUNC(fuukitmap_device::priority_w)); // Controls layer order
}


DEFINE_DEVICE_TYPE(FUUKI_TILEMAP, fuukitmap_device, "fuukitmap", "Fuuki Tilemap hardware")

fuukitmap_device::fuukitmap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, FUUKI_TILEMAP, tag, owner, clock)
	, device_gfx_interface(mconfig, *this, nullptr)
	, device_video_interface(mconfig, *this, true)
	, m_colour_cb(*this)
	, m_level_1_irq_cb(*this)
	, m_vblank_irq_cb(*this)
	, m_raster_irq_cb(*this)
	, m_xoffs(0)
	, m_xoffs_flip(0)
	, m_yoffs(0)
	, m_yoffs_flip(0)
	, m_layer2_xoffs(0)
	, m_layer2_yoffs(0)
	, m_tilemap{ nullptr, nullptr, nullptr }
	, m_vram(*this, "vram%u", 0U, 0x2000U, ENDIANNESS_BIG)
	, m_unknown{ 0, 0 }
	, m_priority(0)
	, m_flip(false)
	, m_tmap_front(0)
	, m_tmap_middle(1)
	, m_tmap_back(2)
	, m_level_1_interrupt_timer(nullptr)
	, m_vblank_interrupt_timer(nullptr)
	, m_raster_interrupt_timer(nullptr)
{
}

void fuukitmap_device::device_start()
{
	m_colour_cb.resolve();

	std::fill(std::begin(m_vregs), std::end(m_vregs), 0);

	m_tilemap[0] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(fuukitmap_device::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_tilemap[1] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(fuukitmap_device::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_tilemap[2] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(fuukitmap_device::get_tile_info<2>)), TILEMAP_SCAN_ROWS,  8,  8, 64, 32);

	m_level_1_interrupt_timer = timer_alloc(FUNC(fuukitmap_device::level1_interrupt), this);
	m_vblank_interrupt_timer = timer_alloc(FUNC(fuukitmap_device::vblank_interrupt), this);
	m_raster_interrupt_timer = timer_alloc(FUNC(fuukitmap_device::raster_interrupt), this);

	save_item(NAME(m_vregs));
	save_item(NAME(m_unknown));
	save_item(NAME(m_priority));
	save_item(NAME(m_flip));
	save_item(NAME(m_tmap_front));
	save_item(NAME(m_tmap_middle));
	save_item(NAME(m_tmap_back));
}

void fuukitmap_device::device_reset()
{
	const rectangle &visarea = screen().visible_area();

	m_level_1_interrupt_timer->adjust(screen().time_until_pos(248));
	m_vblank_interrupt_timer->adjust(screen().time_until_vblank_start());
	m_raster_interrupt_timer->adjust(screen().time_until_pos(0, visarea.max_x + 1));
}


TIMER_CALLBACK_MEMBER(fuukitmap_device::level1_interrupt)
{
	m_level_1_irq_cb(ASSERT_LINE);
	m_level_1_interrupt_timer->adjust(screen().time_until_pos(248));
}

TIMER_CALLBACK_MEMBER(fuukitmap_device::vblank_interrupt)
{
	m_vblank_irq_cb(ASSERT_LINE);    // VBlank IRQ
	m_vblank_interrupt_timer->adjust(screen().time_until_vblank_start());
}

TIMER_CALLBACK_MEMBER(fuukitmap_device::raster_interrupt)
{
	m_raster_irq_cb(ASSERT_LINE);    // Raster Line IRQ
	screen().update_partial(screen().vpos());
	m_raster_interrupt_timer->adjust(screen().frame_period());
}


template <int Layer>
void fuukitmap_device::vram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_vram[Layer][offset]);
	m_tilemap[Layer]->mark_tile_dirty(offset / 2);
}

template <int Layer>
void fuukitmap_device::vram_buffered_w(offs_t offset, u16 data, u16 mem_mask)
{
	const int buffer = (m_vregs[0x1e / 2] & 0x40) >> 6;
	COMBINE_DATA(&m_vram[Layer][offset]);
	if ((Layer & 1) == buffer)
		m_tilemap[2]->mark_tile_dirty(offset / 2);
}


void fuukitmap_device::vregs_w(offs_t offset, u16 data, u16 mem_mask)
{
	const u16 old = m_vregs[offset];
	data = COMBINE_DATA(&m_vregs[offset]);
	if (old != data)
	{
		if (offset == 0x1c / 2)
		{
			const rectangle &visarea = screen().visible_area();
			attotime period = screen().frame_period();
			m_raster_interrupt_timer->adjust(screen().time_until_pos(data, visarea.max_x + 1), 0, period);
		}
		if (offset == 0x1e / 2)
		{
			if ((old ^ data) & 0x40)
				m_tilemap[2]->mark_all_dirty();

			m_flip = BIT(m_vregs[0x1e / 2], 0);
		}
	}
}


/***************************************************************************


                                    Tilemaps

    Offset:     Bits:                   Value:

        0.w                             Code

        2.w     fedc ba98 ---- ----
                ---- ---- 7--- ----     Flip Y
                ---- ---- -6-- ----     Flip X
                ---- ---- --54 3210     Color


***************************************************************************/

template <int Layer>
TILE_GET_INFO_MEMBER(fuukitmap_device::get_tile_info)
{
	const int buffer = (Layer < 2) ? 0 : (m_vregs[0x1e / 2] & 0x40) >> 6;
	const u16 code = m_vram[Layer|buffer][2 * tile_index + 0];
	const u16 attr = m_vram[Layer|buffer][2 * tile_index + 1];
	u32 colour = attr & 0x3f;
	if (!m_colour_cb.isnull())
		m_colour_cb(Layer, colour);
	tileinfo.set(Layer, code, colour, TILE_FLIPYX((attr >> 6) & 3));
}


/***************************************************************************


                                Screen Drawing

    Video Registers (vregs):

        00.w        Layer 0 Scroll Y
        02.w        Layer 0 Scroll X
        04.w        Layer 1 Scroll Y
        06.w        Layer 1 Scroll X
        08.w        Layer 2 Scroll Y
        0a.w        Layer 2 Scroll X
        0c.w        Layers Y Offset
        0e.w        Layers X Offset

        10-1a.w     ? 0
        1c.w        Trigger a level 5 irq on this raster line
        1e.w        ? $3390/$3393 (Flip Screen Off/On), $0040 is buffer for tilemap 2

    Priority Register (priority):

        fedc ba98 7654 3---
        ---- ---- ---- -210     Layer Order


    Unknown Registers (unknown):

        00.w        ? $0200/$0201   (Flip Screen Off/On)
        02.w        ? $f300/$0330

***************************************************************************/

// Wrapper to handle bg and bg2 together
void fuukitmap_device::draw_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u8 i, int flag, u8 pri, u8 primask)
{
	m_tilemap[i]->draw(screen, bitmap, cliprect, flag, pri, primask);
}

void fuukitmap_device::prepare()
{
	/*
	It's not independent bits causing layers to switch, that wouldn't make sense with 3 bits.
	*/

	static const u8 pri_table[6][3] = {
		{ 0, 1, 2 }, // Special moves 0>1, 0>2 (0,1,2 or 0,2,1)
		{ 0, 2, 1 }, // Two Levels - 0>1 (0,1,2 or 0,2,1 or 2,0,1)
		{ 1, 0, 2 }, // Most Levels - 2>1 1>0 2>0 (1,0,2)
		{ 1, 2, 0 }, // Not used?
		{ 2, 0, 1 }, // Title etc. - 0>1 (0,1,2 or 0,2,1 or 2,0,1)
		{ 2, 1, 0 }}; // Char Select, prison stage 1>0 (leaves 1,2,0 or 2,1,0)

	m_tmap_front  = pri_table[m_priority & 0x0f][0];
	m_tmap_middle = pri_table[m_priority & 0x0f][1];
	m_tmap_back   = pri_table[m_priority & 0x0f][2];

	// Layers scrolling
	const u16 scrolly_offs = m_vregs[0xc / 2] - (m_flip ? m_xoffs_flip : m_xoffs);
	const u16 scrollx_offs = m_vregs[0xe / 2] - (m_flip ? m_yoffs_flip : m_yoffs);

	const u16 layer0_scrolly = m_vregs[0x0 / 2] + scrolly_offs;
	const u16 layer0_scrollx = m_vregs[0x2 / 2] + scrollx_offs;
	const u16 layer1_scrolly = m_vregs[0x4 / 2] + scrolly_offs;
	const u16 layer1_scrollx = m_vregs[0x6 / 2] + scrollx_offs;

	const u16 layer2_scrolly = m_vregs[0x8 / 2];
	const u16 layer2_scrollx = m_vregs[0xa / 2];

	m_tilemap[0]->set_scrollx(0, layer0_scrollx);
	m_tilemap[0]->set_scrolly(0, layer0_scrolly);
	m_tilemap[1]->set_scrollx(0, layer1_scrollx);
	m_tilemap[1]->set_scrolly(0, layer1_scrolly);

	m_tilemap[2]->set_scrollx(0, layer2_scrollx + m_layer2_xoffs);
	m_tilemap[2]->set_scrolly(0, layer2_scrolly + m_layer2_yoffs);
}
