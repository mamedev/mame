// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
/* Kaneko View2 Tilemaps */

/*
    [ Scrolling Layers ]

        Each VIEW2 chip generates 2 layers. Up to 2 chips are used
        (4 layers)

        Layer Size:             512 x 512
        Tiles:                  16 x 16 x 4

        Line scroll is supported by the chip: each layer has RAM
        for 512 horizontal scroll offsets (one per tilemap line)
        that are added to the global scroll values.
        See e.g. blazeon (2nd demo level), mgcrystl, sandscrp.



***************************************************************************

                        Callbacks for the TileMap code

                              [ Tiles Format ]

Offset:

0000.w          fedc b--- ---- ----     unused?
                ---- -a9- ---- ----     High Priority (vs Sprites)
                ---- ---8 ---- ----     High Priority (vs Tiles)
                ---- ---- 7654 32--     Color
                ---- ---- ---- --1-     Flip X
                ---- ---- ---- ---0     Flip Y

0002.w                                  Code

***************************************************************************

***************************************************************************

                            Layers Registers


    Offset:         Format:                     Value:

    0000.w                                      FG Scroll X
    0002.w                                      FG Scroll Y

    0004.w                                      BG Scroll X
    0006.w                                      BG Scroll Y

    0008.w          Layers Control

                    fed- ---- ---- ----
                    ---c ---- ---- ----     BG Disable
                    ---- b--- ---- ----     Line Scroll (Always 1 in berlwall & bakubrkr)
                    ---- -a-- ---- ----     ? Always 1 in gtmr     & bakubrkr ?
                    ---- --9- ---- ----     BG Flip X
                    ---- ---8 ---- ----     BG Flip Y

                    ---- ---- 765- ----
                    ---- ---- ---4 ----     FG Disable
                    ---- ---- ---- 3---     Line Scroll (Always 1 in berlwall & bakubrkr)
                    ---- ---- ---- -2--     ? Always 1 in gtmr     & bakubrkr ?
                    ---- ---- ---- --1-     FG Flip X
                    ---- ---- ---- ---0     FG Flip Y

    000a.w                                      ? always 0x0002 ?

There are more!

***************************************************************************

  [gtmr]

    car select screen scroll values:
    Flipscreen off:
        $6x0000: $72c0 ; $fbc0 ; 7340 ; 0
        $72c0/$40 = $1cb = $200-$35 /   $7340/$40 = $1cd = $1cb+2

        $fbc0/$40 = -$11

    Flipscreen on:
        $6x0000: $5d00 ; $3780 ; $5c80 ; $3bc0
        $5d00/$40 = $174 = $200-$8c /   $5c80/$40 = $172 = $174-2

        $3780/$40 = $de /   $3bc0/$40 = $ef



*/

#include "emu.h"
#include "kaneko_tmap.h"

void kaneko_view2_tilemap_device::vram_map(address_map &map)
{
	map(0x0000, 0x0fff).rw(FUNC(kaneko_view2_tilemap_device::vram_1_r), FUNC(kaneko_view2_tilemap_device::vram_1_w)).share("vram_1");
	map(0x1000, 0x1fff).rw(FUNC(kaneko_view2_tilemap_device::vram_0_r), FUNC(kaneko_view2_tilemap_device::vram_0_w)).share("vram_0");
	map(0x2000, 0x2fff).rw(FUNC(kaneko_view2_tilemap_device::scroll_1_r), FUNC(kaneko_view2_tilemap_device::scroll_1_w)).share("scroll_1");
	map(0x3000, 0x3fff).rw(FUNC(kaneko_view2_tilemap_device::scroll_0_r), FUNC(kaneko_view2_tilemap_device::scroll_0_w)).share("scroll_0");
}

DEFINE_DEVICE_TYPE(KANEKO_TMAP, kaneko_view2_tilemap_device, "kaneko_view2", "Kaneko VIEW2 Tilemaps")

kaneko_view2_tilemap_device::kaneko_view2_tilemap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, KANEKO_TMAP, tag, owner, clock)
	, m_vram(*this, "vram_%u", 0U)
	, m_vscroll(*this, "scroll_%u", 0U)
	, m_gfxdecode(*this, finder_base::DUMMY_TAG)
{
	m_invert_flip = 0;
}

template<unsigned Layer>
TILE_GET_INFO_MEMBER(kaneko_view2_tilemap_device::get_tile_info)
{
	u16 attr = m_vram[Layer][ 2 * tile_index + 0];
	u32 code = m_vram[Layer][ 2 * tile_index + 1];
	if (!m_view2_cb.isnull())
		m_view2_cb(Layer, &code);

	SET_TILE_INFO_MEMBER(m_tilebase, code, (attr >> 2) & 0x3f, TILE_FLIPXY(attr & 3));
	tileinfo.category   =   (attr >> 8) & 7;
}


void kaneko_view2_tilemap_device::device_start()
{
	if(!m_gfxdecode->started())
		throw device_missing_dependencies();

	m_view2_cb.bind_relative_to(*owner());
	m_regs = make_unique_clear<u16[]>(0x20/2);

	m_tmap[0] = &machine().tilemap().create(
			*m_gfxdecode,
			tilemap_get_info_delegate(FUNC(kaneko_view2_tilemap_device::get_tile_info<0>),this),
			TILEMAP_SCAN_ROWS,
			16,16, 0x20,0x20);
	m_tmap[1] = &machine().tilemap().create(
			*m_gfxdecode,
			tilemap_get_info_delegate(FUNC(kaneko_view2_tilemap_device::get_tile_info<1>),this),
			TILEMAP_SCAN_ROWS,
			16,16, 0x20,0x20);

	m_tmap[0]->set_transparent_pen(0);
	m_tmap[1]->set_transparent_pen(0);

	m_tmap[0]->set_scroll_rows(0x200);  // Line Scroll
	m_tmap[1]->set_scroll_rows(0x200);

	m_tmap[0]->set_scrolldx(-m_dx,      m_xdim + m_dx -1        );
	m_tmap[1]->set_scrolldx(-(m_dx+2),  m_xdim + (m_dx + 2) - 1 );

	m_tmap[0]->set_scrolldy(-m_dy,      m_ydim + m_dy -1 );
	m_tmap[1]->set_scrolldy(-m_dy,      m_ydim + m_dy -1 );

	save_pointer(NAME(m_regs), 0x20/2);
}

void kaneko_view2_tilemap_device::device_reset()
{
}


void kaneko_view2_tilemap_device::vram_w(int _N_, offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_vram[_N_][offset]);
	m_tmap[_N_]->mark_tile_dirty(offset/2);
}

void kaneko_view2_tilemap_device::prepare(bitmap_ind16 &bitmap, const rectangle &cliprect) { prepare_common(bitmap, cliprect); }
void kaneko_view2_tilemap_device::prepare(bitmap_rgb32 &bitmap, const rectangle &cliprect) { prepare_common(bitmap, cliprect); }

template<class _BitmapClass>
void kaneko_view2_tilemap_device::prepare_common(_BitmapClass &bitmap, const rectangle &cliprect)
{
	int layers_flip_0;
	u16 layer0_scrollx, layer0_scrolly;
	u16 layer1_scrollx, layer1_scrolly;
	int i;

	layers_flip_0 = m_regs[ 4 ];

	/* Enable layers */
	m_tmap[0]->enable(~layers_flip_0 & 0x1000);
	m_tmap[1]->enable(~layers_flip_0 & 0x0010);

	/* Flip layers */
	if (!m_invert_flip)
	{
		m_tmap[0]->set_flip(    ((layers_flip_0 & 0x0100) ? TILEMAP_FLIPY : 0) |
								((layers_flip_0 & 0x0200) ? TILEMAP_FLIPX : 0) );
		m_tmap[1]->set_flip(    ((layers_flip_0 & 0x0100) ? TILEMAP_FLIPY : 0) |
								((layers_flip_0 & 0x0200) ? TILEMAP_FLIPX : 0) );
	}
	else
	{
		m_tmap[0]->set_flip(    ((layers_flip_0 & 0x0100) ? 0 : TILEMAP_FLIPY) |
								((layers_flip_0 & 0x0200) ? 0 : TILEMAP_FLIPX) );
		m_tmap[1]->set_flip(    ((layers_flip_0 & 0x0100) ? 0 : TILEMAP_FLIPY) |
								((layers_flip_0 & 0x0200) ? 0 : TILEMAP_FLIPX) );
	}

	/* Scroll layers */
	layer0_scrollx      =   m_regs[ 2 ];
	layer0_scrolly      =   m_regs[ 3 ] >> 6;
	layer1_scrollx      =   m_regs[ 0 ];
	layer1_scrolly      =   m_regs[ 1 ] >> 6;

	m_tmap[0]->set_scrolly(0, layer0_scrolly);
	m_tmap[1]->set_scrolly(0, layer1_scrolly);

	for (i = 0; i < 0x200; i++)
	{
		u16 scroll;
		scroll = (layers_flip_0 & 0x0800) ? m_vscroll[0][i] : 0;
		m_tmap[0]->set_scrollx(i,(layer0_scrollx + scroll) >> 6 );
		scroll = (layers_flip_0 & 0x0008) ? m_vscroll[1][i] : 0;
		m_tmap[1]->set_scrollx(i,(layer1_scrollx + scroll) >> 6 );
	}
}

void kaneko_view2_tilemap_device::render_tilemap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri) { render_tilemap_common(screen, bitmap, cliprect, pri); }
void kaneko_view2_tilemap_device::render_tilemap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int pri) { render_tilemap_common(screen, bitmap, cliprect, pri); }

template<class _BitmapClass>
void kaneko_view2_tilemap_device::render_tilemap_common(screen_device &screen, _BitmapClass &bitmap, const rectangle &cliprect, int pri)
{
	m_tmap[0]->draw(screen, bitmap, cliprect, pri, pri, 0);
	m_tmap[1]->draw(screen, bitmap, cliprect, pri, pri, 0);
}

void kaneko_view2_tilemap_device::render_tilemap_alt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri, int v2pri) { render_tilemap_alt_common(screen, bitmap, cliprect, pri, v2pri); }
void kaneko_view2_tilemap_device::render_tilemap_alt(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int pri, int v2pri) { render_tilemap_alt_common(screen, bitmap, cliprect, pri, v2pri); }

template<class _BitmapClass>
void kaneko_view2_tilemap_device::render_tilemap_alt_common(screen_device &screen, _BitmapClass &bitmap, const rectangle &cliprect, int pri, int v2pri)
{
	m_tmap[0]->draw(screen, bitmap, cliprect, pri, v2pri ? pri : 0, 0);
	m_tmap[1]->draw(screen, bitmap, cliprect, pri, v2pri ? pri : 0, 0);
}

void kaneko_view2_tilemap_device::vram_0_w(offs_t offset, u16 data, u16 mem_mask){ vram_w(0, offset, data, mem_mask); }
void kaneko_view2_tilemap_device::vram_1_w(offs_t offset, u16 data, u16 mem_mask){ vram_w(1, offset, data, mem_mask); }

u16 kaneko_view2_tilemap_device::vram_0_r(offs_t offset){ return m_vram[0][offset]; }
u16 kaneko_view2_tilemap_device::vram_1_r(offs_t offset){ return m_vram[1][offset]; }


void kaneko_view2_tilemap_device::scroll_0_w(offs_t offset, u16 data, u16 mem_mask){ COMBINE_DATA(&m_vscroll[0][offset]); }
void kaneko_view2_tilemap_device::scroll_1_w(offs_t offset, u16 data, u16 mem_mask){ COMBINE_DATA(&m_vscroll[1][offset]); }

u16 kaneko_view2_tilemap_device::scroll_0_r(offs_t offset){ return m_vscroll[0][offset]; }
u16 kaneko_view2_tilemap_device::scroll_1_r(offs_t offset){ return m_vscroll[1][offset]; }


u16 kaneko_view2_tilemap_device::regs_r(offs_t offset)
{
	return m_regs[offset];
}

void kaneko_view2_tilemap_device::regs_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_regs[offset]);
}

void kaneko_view2_tilemap_device::mark_layer_dirty(u8 Layer)
{
	m_tmap[Layer]->mark_all_dirty();
}
