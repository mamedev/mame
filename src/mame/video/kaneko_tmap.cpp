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

const device_type KANEKO_TMAP = &device_creator<kaneko_view2_tilemap_device>;

kaneko_view2_tilemap_device::kaneko_view2_tilemap_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, KANEKO_TMAP, "Kaneko VIEW2 Tilemaps", tag, owner, clock, "kaneko_view2_tilemap", __FILE__),
	m_gfxdecode(*this)
{
	m_invert_flip = 0;
}

//-------------------------------------------------
//  static_set_gfxdecode_tag: Set the tag of the
//  gfx decoder
//-------------------------------------------------

void kaneko_view2_tilemap_device::static_set_gfxdecode_tag(device_t &device, const char *tag)
{
	downcast<kaneko_view2_tilemap_device &>(device).m_gfxdecode.set_tag(tag);
}

void kaneko_view2_tilemap_device::set_gfx_region(device_t &device, int region)
{
	kaneko_view2_tilemap_device &dev = downcast<kaneko_view2_tilemap_device &>(device);
	dev.m_tilebase = region;
}

void kaneko_view2_tilemap_device::set_offset(device_t &device, int dx, int dy, int xdim, int ydim)
{
	kaneko_view2_tilemap_device &dev = downcast<kaneko_view2_tilemap_device &>(device);
	dev.m_dx = dx;
	dev.m_dy = dy;
	dev.m_xdim = xdim;
	dev.m_ydim = ydim;
}

void kaneko_view2_tilemap_device::set_invert_flip(device_t &device, int invert_flip)
{
	kaneko_view2_tilemap_device &dev = downcast<kaneko_view2_tilemap_device &>(device);
	dev.m_invert_flip = invert_flip;
}

void kaneko_view2_tilemap_device::get_tile_info(tile_data &tileinfo, tilemap_memory_index tile_index, int _N_)
{
	UINT16 code_hi = m_vram[_N_][ 2 * tile_index + 0];
	UINT16 code_lo = m_vram[_N_][ 2 * tile_index + 1];
	SET_TILE_INFO_MEMBER(m_tilebase, code_lo + m_vram_tile_addition[_N_], (code_hi >> 2) & 0x3f, TILE_FLIPXY( code_hi & 3 ));
	tileinfo.category   =   (code_hi >> 8) & 7;
}

TILE_GET_INFO_MEMBER(kaneko_view2_tilemap_device::get_tile_info_0) { get_tile_info(tileinfo, tile_index, 0); }
TILE_GET_INFO_MEMBER(kaneko_view2_tilemap_device::get_tile_info_1) { get_tile_info(tileinfo, tile_index, 1); }


void kaneko_view2_tilemap_device::device_start()
{
	if(!m_gfxdecode->started())
		throw device_missing_dependencies();

	m_vram[0] = make_unique_clear<UINT16[]>(0x1000/2);
	m_vram[1] = make_unique_clear<UINT16[]>(0x1000/2);
	m_vscroll[0] = make_unique_clear<UINT16[]>(0x1000/2);
	m_vscroll[1] = make_unique_clear<UINT16[]>(0x1000/2);
	m_regs = make_unique_clear<UINT16[]>(0x20/2);

	m_tmap[0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(kaneko_view2_tilemap_device::get_tile_info_0),this), TILEMAP_SCAN_ROWS,
											16,16, 0x20,0x20    );
	m_tmap[1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(kaneko_view2_tilemap_device::get_tile_info_1),this), TILEMAP_SCAN_ROWS,
											16,16, 0x20,0x20    );

	m_tmap[0]->set_transparent_pen(0);
	m_tmap[1]->set_transparent_pen(0);

	m_tmap[0]->set_scroll_rows(0x200);  // Line Scroll
	m_tmap[1]->set_scroll_rows(0x200);


	m_tmap[0]->set_scrolldx(-m_dx,      m_xdim + m_dx -1        );
	m_tmap[1]->set_scrolldx(-(m_dx+2),  m_xdim + (m_dx + 2) - 1 );

	m_tmap[0]->set_scrolldy(-m_dy,      m_ydim + m_dy -1 );
	m_tmap[1]->set_scrolldy(-m_dy,      m_ydim + m_dy -1 );

	save_pointer(NAME(m_vram[0].get()), 0x1000/2);
	save_pointer(NAME(m_vram[1].get()), 0x1000/2);
	save_pointer(NAME(m_vscroll[0].get()), 0x1000/2);
	save_pointer(NAME(m_vscroll[1].get()), 0x1000/2);
	save_pointer(NAME(m_regs.get()), 0x20/2);
	save_item(NAME(m_vram_tile_addition[0]));
	save_item(NAME(m_vram_tile_addition[1]));
}

void kaneko_view2_tilemap_device::device_reset()
{
	m_vram_tile_addition[0] = 0;
	m_vram_tile_addition[1] = 0;
}


void kaneko_view2_tilemap_device::kaneko16_vram_w(offs_t offset, UINT16 data, UINT16 mem_mask, int _N_)
{
	COMBINE_DATA(&m_vram[_N_][offset]);
	m_tmap[_N_]->mark_tile_dirty(offset/2);
}

void kaneko_view2_tilemap_device::kaneko16_prepare(bitmap_ind16 &bitmap, const rectangle &cliprect) { kaneko16_prepare_common(bitmap, cliprect); }
void kaneko_view2_tilemap_device::kaneko16_prepare(bitmap_rgb32 &bitmap, const rectangle &cliprect) { kaneko16_prepare_common(bitmap, cliprect); }

template<class _BitmapClass>
void kaneko_view2_tilemap_device::kaneko16_prepare_common(_BitmapClass &bitmap, const rectangle &cliprect)
{
	int layers_flip_0;
	UINT16 layer0_scrollx, layer0_scrolly;
	UINT16 layer1_scrollx, layer1_scrolly;
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

	m_tmap[0]->set_scrolly(0,layer0_scrolly);
	m_tmap[1]->set_scrolly(0,layer1_scrolly);

	for (i=0; i<0x200; i++)
	{
		UINT16 scroll;
		scroll = (layers_flip_0 & 0x0800) ? m_vscroll[0][i] : 0;
		m_tmap[0]->set_scrollx(i,(layer0_scrollx + scroll) >> 6 );
		scroll = (layers_flip_0 & 0x0008) ? m_vscroll[1][i] : 0;
		m_tmap[1]->set_scrollx(i,(layer1_scrollx + scroll) >> 6 );
	}
}

void kaneko_view2_tilemap_device::render_tilemap_chip(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri) { render_tilemap_chip_common(screen, bitmap, cliprect, pri); }
void kaneko_view2_tilemap_device::render_tilemap_chip(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int pri) { render_tilemap_chip_common(screen, bitmap, cliprect, pri); }

template<class _BitmapClass>
void kaneko_view2_tilemap_device::render_tilemap_chip_common(screen_device &screen, _BitmapClass &bitmap, const rectangle &cliprect, int pri)
{
	m_tmap[0]->draw(screen, bitmap, cliprect, pri, pri, 0);
	m_tmap[1]->draw(screen, bitmap, cliprect, pri, pri, 0);
}

void kaneko_view2_tilemap_device::render_tilemap_chip_alt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri, int v2pri) { render_tilemap_chip_alt_common(screen, bitmap, cliprect, pri, v2pri); }
void kaneko_view2_tilemap_device::render_tilemap_chip_alt(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int pri, int v2pri) { render_tilemap_chip_alt_common(screen, bitmap, cliprect, pri, v2pri); }

template<class _BitmapClass>
void kaneko_view2_tilemap_device::render_tilemap_chip_alt_common(screen_device &screen, _BitmapClass &bitmap, const rectangle &cliprect, int pri, int v2pri)
{
	m_tmap[0]->draw(screen, bitmap, cliprect, pri, v2pri ? pri : 0, 0);
	m_tmap[1]->draw(screen, bitmap, cliprect, pri, v2pri ? pri : 0, 0);
}

WRITE16_MEMBER(kaneko_view2_tilemap_device::kaneko16_vram_0_w){ kaneko16_vram_w(offset, data, mem_mask, 0); }
WRITE16_MEMBER(kaneko_view2_tilemap_device::kaneko16_vram_1_w){ kaneko16_vram_w(offset, data, mem_mask, 1); }

READ16_MEMBER(kaneko_view2_tilemap_device::kaneko16_vram_0_r){ return m_vram[0][offset]; }
READ16_MEMBER(kaneko_view2_tilemap_device::kaneko16_vram_1_r){ return m_vram[1][offset]; }


WRITE16_MEMBER(kaneko_view2_tilemap_device::kaneko16_scroll_0_w){ COMBINE_DATA(&m_vscroll[0][offset]); }
WRITE16_MEMBER(kaneko_view2_tilemap_device::kaneko16_scroll_1_w){ COMBINE_DATA(&m_vscroll[1][offset]); }

READ16_MEMBER(kaneko_view2_tilemap_device::kaneko16_scroll_0_r){ return m_vscroll[0][offset]; }
READ16_MEMBER(kaneko_view2_tilemap_device::kaneko16_scroll_1_r){ return m_vscroll[1][offset]; }


READ16_MEMBER( kaneko_view2_tilemap_device::kaneko_tmap_vram_r )
{
	if      (offset<(0x1000/2)) return kaneko16_vram_1_r(space, offset&((0x1000/2)-1),mem_mask);
	else if (offset<(0x2000/2)) return kaneko16_vram_0_r(space, offset&((0x1000/2)-1),mem_mask);
	else if (offset<(0x3000/2)) return kaneko16_scroll_1_r(space, offset&((0x1000/2)-1),mem_mask);
	else if (offset<(0x4000/2)) return kaneko16_scroll_0_r(space, offset&((0x1000/2)-1),mem_mask);

	return 0x0000;
}

WRITE16_MEMBER( kaneko_view2_tilemap_device::kaneko_tmap_vram_w )
{
	if      (offset<(0x1000/2)) kaneko16_vram_1_w(space, offset&((0x1000/2)-1),data,mem_mask);
	else if (offset<(0x2000/2)) kaneko16_vram_0_w(space, offset&((0x1000/2)-1),data,mem_mask);
	else if (offset<(0x3000/2)) kaneko16_scroll_1_w(space, offset&((0x1000/2)-1),data,mem_mask);
	else if (offset<(0x4000/2)) kaneko16_scroll_0_w(space, offset&((0x1000/2)-1),data,mem_mask);
}

READ16_MEMBER( kaneko_view2_tilemap_device::kaneko_tmap_regs_r )
{
	return m_regs[offset];
}

WRITE16_MEMBER( kaneko_view2_tilemap_device::kaneko_tmap_regs_w )
{
	COMBINE_DATA(&m_regs[offset]);
}


/* some weird logic needed for Gals Panic on the EXPRO02 board */
WRITE16_MEMBER(kaneko_view2_tilemap_device::galsnew_vram_0_tilebank_w)
{
	if (mem_mask & 0x00ff)
	{
		int val = (data & 0x00ff)<<8;

		if (m_vram_tile_addition[0] != val)
		{
			m_vram_tile_addition[0] = val;
			m_tmap[0]->mark_all_dirty();
		}
	}
}

WRITE16_MEMBER(kaneko_view2_tilemap_device::galsnew_vram_1_tilebank_w)
{
	if (mem_mask & 0x00ff)
	{
		int val = (data & 0x00ff)<<8;

		if (m_vram_tile_addition[1] != val)
		{
			m_vram_tile_addition[1] = val;
			m_tmap[1]->mark_all_dirty();
		}
	}
}
