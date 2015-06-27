// license:BSD-3-Clause
// copyright-holders:Yochizo
/***************************************************************************

Functions to emulate the video hardware of the machine.


BG RAM format [Argus and Butasan]
-----------------------------------------------------------------------------
 +0         +1
 xxxx xxxx  ---- ---- = 1st - 8th bits of tile number
 ---- ----  xx-- ---- = 9th and 10th bit of tile number
 ---- ----  --x- ---- = flip y
 ---- ----  ---x ---- = flip x
 ---- ----  ---- xxxx = color

BG RAM format [Valtric]
-----------------------------------------------------------------------------
 +0         +1
 xxxx xxxx  ---- ---- = 1st - 8th bits of tile number
 ---- ----  xx-- ---- = 9th and 10th bit of tile number
 ---- ----  --x- ---- = 11th bit of tile number
 ---- ----  ---- xxxx = color


Text RAM format [Argus, Valtric and Butasan]
-----------------------------------------------------------------------------
 +0         +1
 xxxx xxxx  ---- ---- = low bits of tile number
 ---- ----  xx-- ---- = high bits of tile number
 ---- ----  --x- ---- = flip y
 ---- ----  ---x ---- = flip x
 ---- ----  ---- xxxx = color


Sprite RAM format [Argus]
-----------------------------------------------------------------------------
 +11        +12        +13        +14        +15
 xxxx xxxx  ---- ----  ---- ----  ---- ----  ---- ---- = sprite y
 ---- ----  xxxx xxxx  ---- ----  ---- ----  ---- ---- = low bits of sprite x
 ---- ----  ---- ----  xx-- ----  ---- ----  ---- ---- = high bits of tile number
 ---- ----  ---- ----  --x- ----  ---- ----  ---- ---- = flip y
 ---- ----  ---- ----  ---x ----  ---- ----  ---- ---- = flip x
 ---- ----  ---- ----  ---- --x-  ---- ----  ---- ---- = high bit of sprite y
 ---- ----  ---- ----  ---- ---x  ---- ----  ---- ---- = high bit of sprite x
 ---- ----  ---- ----  ---- ----  xxxx xxxx  ---- ---- = low bits of tile number
 ---- ----  ---- ----  ---- ----  ---- ----  ---- x--- = BG1 / sprite priority (Argus only)
 ---- ----  ---- ----  ---- ----  ---- ----  ---- -xxx = color

Sprite RAM format [Valtric]
-----------------------------------------------------------------------------
 +11        +12        +13        +14        +15
 xxxx xxxx  ---- ----  ---- ----  ---- ----  ---- ---- = sprite y
 ---- ----  xxxx xxxx  ---- ----  ---- ----  ---- ---- = low bits of sprite x
 ---- ----  ---- ----  xx-- ----  ---- ----  ---- ---- = high bits of tile number
 ---- ----  ---- ----  --x- ----  ---- ----  ---- ---- = flip y
 ---- ----  ---- ----  ---x ----  ---- ----  ---- ---- = flip x
 ---- ----  ---- ----  ---- --x-  ---- ----  ---- ---- = high bit of sprite y
 ---- ----  ---- ----  ---- ---x  ---- ----  ---- ---- = high bit of sprite x
 ---- ----  ---- ----  ---- ----  xxxx xxxx  ---- ---- = low bits of tile number
 ---- ----  ---- ----  ---- ----  ---- ----  ---- xxxx = color

Sprite RAM format [Butasan]
-----------------------------------------------------------------------------
 +8         +9         +10        +11        +12
 ---- -x--  ---- ----  ---- ----  ---- ----  ---- ---- = flip y
 ---- ---x  ---- ----  ---- ----  ---- ----  ---- ---- = flip x
 ---- ----  ---- xxxx  ---- ----  ---- ----  ---- ---- = color ($00 - $0B)
 ---- ----  ---- ----  xxxx xxxx  ---- ----  ---- ---- = low bits of sprite x
 ---- ----  ---- ----  ---- ----  ---- ---x  ---- ---- = top bit of sprite x
 ---- ----  ---- ----  ---- ----  ---- ----  xxxx xxxx = low bits of sprite y
 +13        +14        +15
 ---- ---x  ---- ----  ---- ---- = top bit of sprite y
 ---- ----  xxxx xxxx  ---- ---- = low bits of tile number
 ---- ----  ---- ----  ---- xxxx = top bits of tile number

(*) Sprite size is defined by its offset.
    $F000 - $F0FF : 16x32    $F100 - $F2FF : 16x16
    $F300 - $F3FF : 16x32    $F400 - $F57F : 16x16
    $F580 - $F61F : 32x32    $F620 - $F67F : 64x64


Scroll RAM of X and Y coordinates [Argus, Valtric and Butasan]
-----------------------------------------------------------------------------
 +0         +1
 xxxx xxxx  ---- ---- = scroll value
 ---- ----  ---- ---x = top bit of scroll value


Video effect RAM ( $C30C )
-----------------------------------------------------------------------------
 +0
 ---- ---x  = BG enable bit
 ---- --x-  = grey scale effect or tile bank select.


Flip screen controller
-----------------------------------------------------------------------------
 +0
 x--- ----  = flip screen


BG0 palette intensity ( $C47F, $C4FF )
-----------------------------------------------------------------------------
 +0 (c47f)  +1 (c4ff)
 xxxx ----  ---- ---- = red intensity
 ---- xxxx  ---- ---- = green intensity
 ---- ----  xxxx ---- = blue intensity


(*) Things which are not emulated.
 - Color $000 - 00f, $01e, $02e ... are half transparent color.
 - Sprite priority bit may be present in Butasan. But I don't know
   what happens when it is set.

***************************************************************************/

#include "emu.h"
#include "includes/argus.h"


/***************************************************************************
  Callbacks for the tilemap code
***************************************************************************/

TILE_GET_INFO_MEMBER(argus_state::argus_get_tx_tile_info)
{
	UINT8 hi, lo;

	tile_index <<= 1;

	lo = m_txram[tile_index];
	hi = m_txram[tile_index + 1];

	SET_TILE_INFO_MEMBER(3,
			((hi & 0xc0) << 2) | lo,
			hi & 0x0f,
			TILE_FLIPYX((hi & 0x30) >> 4));
}

TILE_GET_INFO_MEMBER(argus_state::argus_get_bg0_tile_info)
{
	UINT8 hi, lo;

	tile_index <<= 1;

	lo = m_dummy_bg0ram[tile_index];
	hi = m_dummy_bg0ram[tile_index + 1];

	SET_TILE_INFO_MEMBER(1,
			((hi & 0xc0) << 2) | lo,
			hi & 0x0f,
			TILE_FLIPYX((hi & 0x30) >> 4));
}

TILE_GET_INFO_MEMBER(argus_state::argus_get_bg1_tile_info)
{
	UINT8 hi, lo;

	tile_index <<= 1;

	lo = m_bg1ram[tile_index];
	hi = m_bg1ram[tile_index + 1];

	SET_TILE_INFO_MEMBER(2,
			lo,
			hi & 0x0f,
			TILE_FLIPYX((hi & 0x30) >> 4));
}

TILE_GET_INFO_MEMBER(argus_state::valtric_get_tx_tile_info)
{
	UINT8 hi, lo;

	tile_index <<= 1;

	lo = m_txram[tile_index];
	hi = m_txram[tile_index + 1];

	SET_TILE_INFO_MEMBER(2,
			((hi & 0xc0) << 2) | lo,
			hi & 0x0f,
			TILE_FLIPYX((hi & 0x30) >> 4));
}

TILE_GET_INFO_MEMBER(argus_state::valtric_get_bg_tile_info)
{
	UINT8 hi, lo;

	tile_index <<= 1;

	lo = m_bg1ram[tile_index];
	hi = m_bg1ram[tile_index + 1];

	SET_TILE_INFO_MEMBER(1,
			((hi & 0xc0) << 2) | ((hi & 0x20) << 5) | lo,
			hi & 0x0f,
			0);
}

TILE_GET_INFO_MEMBER(argus_state::butasan_get_tx_tile_info)
{
	UINT8 hi, lo;

	tile_index ^= 0x3e0;
	tile_index <<= 1;

	lo = m_butasan_txram[tile_index];
	hi = m_butasan_txram[tile_index + 1];

	SET_TILE_INFO_MEMBER(3,
			((hi & 0xc0) << 2) | lo,
			hi & 0x0f,
			TILE_FLIPYX((hi & 0x30) >> 4));
}

TILE_GET_INFO_MEMBER(argus_state::butasan_get_bg0_tile_info)
{
	UINT8 hi, lo;
	int attrib;

	attrib = (tile_index & 0x00f) | ((tile_index & 0x3e0) >> 1) | ((tile_index & 0x010) << 5);
	attrib ^= 0x0f0;
	attrib <<= 1;

	lo = m_butasan_bg0ram[attrib];
	hi = m_butasan_bg0ram[attrib + 1];

	SET_TILE_INFO_MEMBER(1,
			((hi & 0xc0) << 2) | lo,
			hi & 0x0f,
			TILE_FLIPYX((hi & 0x30) >> 4));
}

TILE_GET_INFO_MEMBER(argus_state::butasan_get_bg1_tile_info)
{
	int attrib, tile;

	attrib = (tile_index & 0x00f) | ((tile_index & 0x3e0) >> 1) | ((tile_index & 0x010) << 5);
	attrib ^= 0x0f0;

	tile = m_butasan_bg1ram[attrib] | ((m_butasan_bg1_status & 2) << 7);

	SET_TILE_INFO_MEMBER(2,
			tile,
			(tile & 0x80) >> 7,
			0);
}


/***************************************************************************
  Initialize and destroy video hardware emulation
***************************************************************************/

void argus_state::reset_common()
{
	m_bg_status = 0x01;
	m_flipscreen = 0;
	m_palette_intensity = 0;
}

VIDEO_START_MEMBER(argus_state,argus)
{
	/*                           info                     offset             w   h  col  row */
	m_bg0_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(argus_state::argus_get_bg0_tile_info),this), TILEMAP_SCAN_COLS, 16, 16, 32, 32);
	m_bg1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(argus_state::argus_get_bg1_tile_info),this), TILEMAP_SCAN_COLS, 16, 16, 32, 32);
	m_tx_tilemap  = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(argus_state::argus_get_tx_tile_info),this),  TILEMAP_SCAN_COLS,  8,  8, 32, 32);

	m_bg1_tilemap->set_transparent_pen(15);
	m_tx_tilemap->set_transparent_pen(15);

	/* dummy RAM for back ground */
	m_dummy_bg0ram = auto_alloc_array(machine(), UINT8, 0x800);

	save_item(NAME(m_bg_status));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_palette_intensity));
	save_pointer(NAME(m_dummy_bg0ram), 0x800);
	save_item(NAME(m_lowbitscroll));
	save_item(NAME(m_prvscrollx));
}

VIDEO_RESET_MEMBER(argus_state,argus)
{
	m_lowbitscroll = 0;
	m_prvscrollx = 0;
	m_bg0_scrollx[0] = 0;
	m_bg0_scrollx[1] = 0;
	memset(m_dummy_bg0ram, 0, 0x800);
	reset_common();
}

VIDEO_START_MEMBER(argus_state,valtric)
{
	/*                           info                      offset             w   h  col  row */
	m_bg1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(argus_state::valtric_get_bg_tile_info),this), TILEMAP_SCAN_COLS, 16, 16, 32, 32);
	m_tx_tilemap  = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(argus_state::valtric_get_tx_tile_info),this), TILEMAP_SCAN_COLS,  8,  8, 32, 32);

	m_tx_tilemap->set_transparent_pen(15);

	m_screen->register_screen_bitmap(m_mosaicbitmap);

	save_item(NAME(m_bg_status));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_palette_intensity));
	save_item(NAME(m_valtric_mosaic));
	save_item(NAME(m_valtric_unknown));
	save_item(NAME(m_mosaic));
}

VIDEO_RESET_MEMBER(argus_state,valtric)
{
	m_valtric_mosaic = 0x0f;
	reset_common();
}

VIDEO_START_MEMBER(argus_state,butasan)
{
	/*                           info                       offset             w   h  col  row */
	m_bg0_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(argus_state::butasan_get_bg0_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_bg1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(argus_state::butasan_get_bg1_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tx_tilemap  = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(argus_state::butasan_get_tx_tile_info),this),  TILEMAP_SCAN_ROWS,  8,  8, 32, 32);

	m_bg1_tilemap->set_transparent_pen(15);
	m_tx_tilemap->set_transparent_pen(15);

	m_butasan_pagedram[0] = auto_alloc_array(machine(), UINT8, 0x1000);
	m_butasan_pagedram[1] = auto_alloc_array(machine(), UINT8, 0x1000);

	m_butasan_bg0ram     = &m_butasan_pagedram[0][0x000];
	m_butasan_bg0backram = &m_butasan_pagedram[0][0x800];
	m_butasan_txram      = &m_butasan_pagedram[1][0x000];
	m_butasan_txbackram  = &m_butasan_pagedram[1][0x800];

	save_item(NAME(m_bg_status));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_palette_intensity));
	save_pointer(NAME(m_butasan_pagedram[0]), 0x1000);
	save_pointer(NAME(m_butasan_pagedram[1]), 0x1000);
	save_item(NAME(m_butasan_page_latch));
	save_item(NAME(m_butasan_bg1_status));
	save_item(NAME(m_butasan_unknown));
}

VIDEO_RESET_MEMBER(argus_state,butasan)
{
	m_butasan_page_latch = 0;
	m_butasan_bg1_status = 0x01;
	memset(m_butasan_pagedram[0], 0, 0x1000);
	memset(m_butasan_pagedram[1], 0, 0x1000);
	reset_common();
}


/***************************************************************************
  Functions for handler of MAP roms in Argus and palette color
***************************************************************************/

/* Write bg0 pattern data to dummy bg0 ram */
void argus_state::argus_write_dummy_rams(int dramoffs, int vromoffs)
{
	int i;
	int voffs;
	int offs;

	UINT8 *VROM1 = memregion("user1")->base();       /* "ag_15.bin" */
	UINT8 *VROM2 = memregion("user2")->base();       /* "ag_16.bin" */

	/* offset in pattern data */
	offs = VROM1[vromoffs] | (VROM1[vromoffs + 1] << 8);
	offs &= 0x7ff;

	voffs = offs * 16;
	for (i = 0; i < 8; i++)
	{
		m_dummy_bg0ram[dramoffs]     = VROM2[voffs];
		m_dummy_bg0ram[dramoffs + 1] = VROM2[voffs + 1];
		m_bg0_tilemap->mark_tile_dirty(dramoffs >> 1);
		dramoffs += 2;
		voffs += 2;
	}
}

void argus_state::change_palette(int color, int lo_offs, int hi_offs)
{
	UINT8 lo = m_paletteram[lo_offs];
	UINT8 hi = m_paletteram[hi_offs];
	m_blend->set(color, hi & 0x0f);
	m_palette->set_pen_color(color, pal4bit(lo >> 4), pal4bit(lo), pal4bit(hi >> 4));
}

void argus_state::change_bg_palette(int color, int lo_offs, int hi_offs)
{
	UINT8 r,g,b,lo,hi,ir,ig,ib,ix;
	rgb_t rgb,irgb;

	/* red,green,blue intensities */
	ir = pal4bit(m_palette_intensity >> 12);
	ig = pal4bit(m_palette_intensity >>  8);
	ib = pal4bit(m_palette_intensity >>  4);
	ix = m_palette_intensity & 0x0f;

	irgb = rgb_t(ir,ig,ib);

	lo = m_paletteram[lo_offs];
	hi = m_paletteram[hi_offs];

	/* red,green,blue component */
	r = pal4bit(lo >> 4);
	g = pal4bit(lo);
	b = pal4bit(hi >> 4);

	/* Grey background enable */
	if (m_bg_status & 2)
	{
		UINT8 val = (r + g + b) / 3;
		rgb = rgb_t(val,val,val);
	}
	else
	{
		rgb = rgb_t(r,g,b);
	}

	rgb = m_blend->func(rgb,irgb,ix);

	m_palette->set_pen_color(color,rgb);
}


/***************************************************************************
  Memory handler
***************************************************************************/

WRITE8_MEMBER(argus_state::valtric_mosaic_w)
{
	m_valtric_mosaic = data;
}

WRITE8_MEMBER(argus_state::txram_w)
{
	m_txram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset >> 1);
}

WRITE8_MEMBER(argus_state::bg1ram_w)
{
	m_bg1ram[offset] = data;
	m_bg1_tilemap->mark_tile_dirty(offset >> 1);
}

WRITE8_MEMBER(argus_state::argus_bg_status_w)
{
	if (m_bg_status != data)
	{
		m_bg_status = data;

		/* Gray / purple scale */
		if (m_bg_status & 2)
		{
			int offs;

			for (offs = 0x400; offs < 0x500; offs++)
			{
				change_bg_palette((offs - 0x400) + 0x080, offs, offs + 0x400);
			}
		}
	}
}

WRITE8_MEMBER(argus_state::valtric_bg_status_w)
{
	if (m_bg_status != data)
	{
		m_bg_status = data;

		/* Gray / purple scale */
		if (m_bg_status & 2)
		{
			int offs;

			for (offs = 0x400; offs < 0x600; offs += 2)
			{
				change_bg_palette(((offs - 0x400) >> 1) + 0x100, offs & ~1, offs | 1);
			}
		}
	}
}

WRITE8_MEMBER(argus_state::butasan_bg0_status_w)
{
	m_bg_status = data;
}

WRITE8_MEMBER(argus_state::butasan_bg1_status_w)
{
	if (m_butasan_bg1_status != data)
	{
		m_butasan_bg1_status = data;

		/* Bank changed */
		m_bg1_tilemap->mark_all_dirty();
	}
}

WRITE8_MEMBER(argus_state::flipscreen_w)
{
	m_flipscreen = data & 0x80;
}

WRITE8_MEMBER(argus_state::argus_paletteram_w)
{
	int offs;

	m_paletteram[offset] = data;

	if (offset <= 0x0ff)                                /* sprite color */
	{
		offset &= 0x07f;

		change_palette(offset, offset, offset + 0x080);

		if (offset == 0x07f || offset == 0x0ff)
		{
			m_palette_intensity = m_paletteram[0x0ff] | (m_paletteram[0x07f] << 8);

			for (offs = 0x400; offs < 0x500; offs++)
				change_bg_palette((offs & 0xff) + 0x080, offs, offs + 0x400);
		}
	}
	else if ((offset >= 0x400 && offset <= 0x4ff) ||
				(offset >= 0x800 && offset <= 0x8ff))       /* BG0 color */
	{
		offs = offset & 0xff;
		offset = offs | 0x400;

		change_bg_palette(offs + 0x080, offset, offset + 0x400);
	}
	else if ((offset >= 0x500 && offset <= 0x5ff) ||
				(offset >= 0x900 && offset <= 0x9ff))       /* BG1 color */
	{
		offs = offset & 0xff;
		offset = offs | 0x500;

		change_palette(offs + 0x180, offset, offset + 0x400);
	}
	else if ((offset >= 0x700 && offset <= 0x7ff) ||
				(offset >= 0xb00 && offset <= 0xbff))       /* text color */
	{
		offs = offset & 0xff;
		offset = offs | 0x700;

		change_palette(offs + 0x280, offset, offset + 0x400);
	}
}

WRITE8_MEMBER(argus_state::valtric_paletteram_w)
{
	m_paletteram[offset] = data;

	if (offset <= 0x1ff)                            /* Sprite color */
	{
		change_palette(offset >> 1, offset & ~1, offset | 1);

		if (offset == 0x1fe || offset == 0x1ff)
		{
			int offs;

			m_palette_intensity = m_paletteram[0x1ff] | (m_paletteram[0x1fe] << 8);

			for (offs = 0x400; offs < 0x600; offs += 2)
				change_bg_palette(((offs & 0x1ff) >> 1) + 0x100, offs & ~1, offs | 1);
		}
	}
	else if (offset >= 0x400 && offset <= 0x5ff)        /* BG color */
	{
		change_bg_palette(((offset & 0x1ff) >> 1) + 0x100, offset & ~1, offset | 1);
	}
	else if (offset >= 0x600 && offset <= 0x7ff)        /* Text color */
	{
		change_palette(((offset & 0x1ff) >> 1) + 0x200, offset & ~1, offset | 1);
	}
}

WRITE8_MEMBER(argus_state::butasan_paletteram_w)
{
	m_paletteram[offset] = data;

	if (offset <= 0x1ff)                            /* BG0 color */
	{
		change_palette((offset >> 1) + 0x100, offset & ~1, offset | 1);
	}
	else if (offset <= 0x23f)                       /* BG1 color */
	{
		change_palette(((offset & 0x3f) >> 1) + 0x0c0, offset & ~1, offset | 1);
	}
	else if (offset >= 0x400 && offset <= 0x47f)    /* Sprite color */
	{                                               /* 16 colors */
		change_palette((offset & 0x7f) >> 1, offset & ~1, offset | 1);
	}
	else if (offset >= 0x480 && offset <= 0x4ff)    /* Sprite color */
	{                                               /* 8  colors */
		int offs = (offset & 0x070) | ((offset & 0x00f) >> 1);

		change_palette(offs + 0x040, offset & ~1, offset | 1);
		change_palette(offs + 0x048, offset & ~1, offset | 1);
	}
	else if (offset >= 0x600 && offset <= 0x7ff)    /* Text color */
	{
		change_palette(((offset & 0x1ff) >> 1) + 0x200, offset & ~1, offset | 1);
	}
	else if (offset >= 0x240 && offset <= 0x25f)    // dummy
		change_palette(((offset & 0x1f) >> 1) + 0xe0, offset & ~1, offset | 1);
	else if (offset >= 0x500 && offset <= 0x51f)    // dummy
		change_palette(((offset & 0x1f) >> 1) + 0xf0, offset & ~1, offset | 1);
}

WRITE8_MEMBER(argus_state::butasan_bg1ram_w)
{
	int idx;

	m_butasan_bg1ram[offset] = data;

	idx = (offset & 0x00f) | ((offset & 0x200) >> 5) | ((offset & 0x1f0) << 1);
	idx ^= 0x0f0;

	m_bg1_tilemap->mark_tile_dirty(idx);
}

WRITE8_MEMBER(argus_state::butasan_pageselect_w)
{
	m_butasan_page_latch = data & 1;
}

READ8_MEMBER(argus_state::butasan_pagedram_r)
{
	if (offset <= 0x07ff)
		return m_butasan_pagedram[m_butasan_page_latch][offset];
	else
		return m_butasan_pagedram[0][offset];
}

WRITE8_MEMBER(argus_state::butasan_pagedram_w)
{
	m_butasan_pagedram[m_butasan_page_latch][offset] = data;

	if (!m_butasan_page_latch)
	{
		if (offset <= 0x07ff)
		{
			int idx;
			idx = ((offset & 0x01e) >> 1) | ((offset & 0x400) >> 6) | (offset & 0x3e0);
			idx ^= 0x1e0;
			m_bg0_tilemap->mark_tile_dirty(idx);
		}
	}
	else
	{
		if (offset <= 0x07ff)
			m_tx_tilemap->mark_tile_dirty((offset ^ 0x7c0) >> 1);
	}
}

WRITE8_MEMBER(argus_state::valtric_unknown_w)
{
	m_valtric_unknown = data;
}

WRITE8_MEMBER(argus_state::butasan_unknown_w)
{
	m_butasan_unknown = data;
}


/***************************************************************************
  Screen refresh
***************************************************************************/

#define bg0_scrollx (m_bg0_scrollx[0] | (m_bg0_scrollx[1] << 8))
#define bg0_scrolly (m_bg0_scrolly[0] | (m_bg0_scrolly[1] << 8))
#define bg1_scrollx (m_bg1_scrollx[0] | (m_bg1_scrollx[1] << 8))
#define bg1_scrolly (m_bg1_scrolly[0] | (m_bg1_scrolly[1] << 8))

void argus_state::bg_setting()
{
	machine().tilemap().set_flip_all(m_flipscreen ? TILEMAP_FLIPY|TILEMAP_FLIPX : 0);

	if (!m_flipscreen)
	{
		if (m_bg0_tilemap != NULL)
		{
			m_bg0_tilemap->set_scrollx(0, bg0_scrollx & 0x1ff);
			m_bg0_tilemap->set_scrolly(0, bg0_scrolly & 0x1ff);
		}
		m_bg1_tilemap->set_scrollx(0, bg1_scrollx & 0x1ff);
		m_bg1_tilemap->set_scrolly(0, bg1_scrolly & 0x1ff);
	}
	else
	{
		if (m_bg0_tilemap != NULL)
		{
			m_bg0_tilemap->set_scrollx(0, (bg0_scrollx + 256) & 0x1ff);
			m_bg0_tilemap->set_scrolly(0, (bg0_scrolly + 256) & 0x1ff);
		}
		m_bg1_tilemap->set_scrollx(0, (bg1_scrollx + 256) & 0x1ff);
		m_bg1_tilemap->set_scrolly(0, (bg1_scrolly + 256) & 0x1ff);
	}
}

void argus_state::argus_bg0_scroll_handle()
{
	int delta;
	int dcolumn;

	/* Deficit between previous and current scroll value */
	delta = bg0_scrollx - m_prvscrollx;
	m_prvscrollx = bg0_scrollx;

	if (delta == 0)
		return;

	if (delta > 0)
	{
		m_lowbitscroll += delta % 16;
		dcolumn = delta / 16;

		if (m_lowbitscroll >= 16)
		{
			dcolumn++;
			m_lowbitscroll -= 16;
		}

		if (dcolumn != 0)
		{
			int i, j;
			int col, woffs, roffs;

			col = ((bg0_scrollx / 16) + 16) % 32;
			woffs = 32 * 2 * col;
			roffs = (((bg0_scrollx / 16) + 16) * 8) % 0x8000;

			if (dcolumn >= 18)
				dcolumn = 18;

			for (i = 0; i < dcolumn; i++)
			{
				for (j = 0; j < 4; j++)
				{
					argus_write_dummy_rams(woffs, roffs);
					woffs += 16;
					roffs += 2;
				}
				woffs -= 128;
				roffs -= 16;
				if (woffs < 0)
					woffs += 0x800;
				if (roffs < 0)
					roffs += 0x8000;
			}
		}
	}
	else
	{
		m_lowbitscroll += (delta % 16);
		dcolumn = -(delta / 16);

		if (m_lowbitscroll <= 0)
		{
			dcolumn++;
			m_lowbitscroll += 16;
		}

		if (dcolumn != 0)
		{
			int i, j;
			int col, woffs, roffs;

			col = ((bg0_scrollx / 16) + 31) % 32;
			woffs = 32 * 2 * col;
			roffs = ((bg0_scrollx / 16) - 1) * 8;
			if (roffs < 0)
				roffs += 0x08000;

			if (dcolumn >= 18)
				dcolumn = 18;

			for (i = 0; i < dcolumn; i++)
			{
				for (j = 0; j < 4; j++)
				{
					argus_write_dummy_rams(woffs, roffs);
					woffs += 16;
					roffs += 2;
				}
				if (woffs >= 0x800)
					woffs -= 0x800;
				if (roffs >= 0x8000)
					roffs -= 0x8000;
			}
		}
	}
}

void argus_state::argus_draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, int priority)
{
	/* Draw the sprites */
	for (int offs = 0; offs < m_spriteram.bytes(); offs += 16)
	{
		if (!(m_spriteram[offs+15] == 0 && m_spriteram[offs+11] == 0xf0))
		{
			int sx, sy, tile, flipx, flipy, color, pri;

			sx = m_spriteram[offs+12]; if (m_spriteram[offs+13] & 0x01) sx -= 256;
			sy = m_spriteram[offs+11]; if (!(m_spriteram[offs+13] & 0x02)) sy -= 256;

			tile  = m_spriteram[offs+14] | ((m_spriteram[offs+13] & 0xc0) << 2);
			flipx = m_spriteram[offs+13] & 0x10;
			flipy = m_spriteram[offs+13] & 0x20;
			color = m_spriteram[offs+15] & 0x07;
			pri   = (m_spriteram[offs+15] & 0x08) >> 3;

			if (m_flipscreen)
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			if (priority != pri)
				m_blend->drawgfx(m_palette,
							bitmap,cliprect,m_gfxdecode->gfx(0),
							tile,
							color,
							flipx, flipy,
							sx, sy,
							15);
		}
	}
}

#if 1
void argus_state::valtric_draw_mosaic(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_valtric_mosaic!=0x80)
	{
		m_mosaic=0x0f-(m_valtric_mosaic&0x0f);
		if (m_mosaic!=0) m_mosaic++;
		if (m_valtric_mosaic&0x80) m_mosaic*=-1;
	}

	if (m_mosaic==0)
		m_bg1_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	else
	{
		m_bg1_tilemap->draw(screen, m_mosaicbitmap, cliprect, 0, 0);
		{
			int step=m_mosaic;
			UINT32 *dest;
			int x,y,xx,yy,c=0;
			int width = screen.width();
			int height = screen.height();

			if (m_mosaic<0)step*=-1;

			for (y=0;y<width+step;y+=step)
				for (x=0;x<height+step;x+=step)
				{
					if (y < height && x < width)
						c=m_mosaicbitmap.pix32(y, x);

					if (m_mosaic<0)
						if (y+step-1<height && x+step-1< width)
							c = m_mosaicbitmap.pix32(y+step-1, x+step-1);

					for (yy=0;yy<step;yy++)
						for (xx=0;xx<step;xx++)
						{
							if (xx+x < width && yy+y<height)
							{
								dest=&bitmap.pix32(y+yy, x+xx);
								*dest=c;
							}
						}
				}
		}
	}
}
#else
void argus_state::valtric_draw_mosaic(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int step = 0x10 - (m_valtric_mosaic & 0x0f);

	if (step == 1)
		m_bg1_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	else
	{
		m_bg1_tilemap->draw(screen, m_mosaicbitmap, cliprect, 0, 0);
		{
			UINT32 *dest;
			int x,y,xx,yy,c=0;
			int width = screen.width();
			int height = screen.height();

			for (y = 0; y < width+step; y += step)
				for (x = 0; x < height+step; x += step)
				{
					if (y < height && x < width)
						c = m_mosaicbitmap.pix32(y, x);

					if (m_valtric_mosaic & 0x80)
						if (y+step-1 < height && x+step-1 < width)
							c = m_mosaicbitmap.pix32(y+step-1, x+step-1);

					for (yy = 0; yy < step; yy++)
						for (xx = 0; xx < step; xx++)
						{
							if (xx+x < width && yy+y < height)
							{
								dest = &bitmap.pix32(y+yy, x+xx);
								*dest = c;
							}
						}
				}
		}
	}
}
#endif

void argus_state::valtric_draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/* Draw the sprites */
	for (int offs = 0; offs < m_spriteram.bytes(); offs += 16)
	{
		if (!(m_spriteram[offs+15] == 0 && m_spriteram[offs+11] == 0xf0))
		{
			int sx, sy, tile, flipx, flipy, color;

			sx = m_spriteram[offs+12]; if (m_spriteram[offs+13] & 0x01) sx -= 256;
			sy = m_spriteram[offs+11]; if (!(m_spriteram[offs+13] & 0x02)) sy -= 256;

			tile  = m_spriteram[offs+14] | ((m_spriteram[offs+13] & 0xc0) << 2);
			flipx = m_spriteram[offs+13] & 0x10;
			flipy = m_spriteram[offs+13] & 0x20;
			color = m_spriteram[offs+15] & 0x0f;

			if (m_flipscreen)
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			m_blend->drawgfx(m_palette,
						bitmap,cliprect,m_gfxdecode->gfx(0),
						tile,
						color,
						flipx, flipy,
						sx, sy,
						15);
		}
	}
}

void argus_state::butasan_draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/* Draw the sprites */
	for (int offs = 0; offs < m_spriteram.bytes(); offs += 16)
	{
		int sx, sy, tile, flipx, flipy, color;
		int fx, fy;

		tile  = m_spriteram[offs+14] | ((m_spriteram[offs+15] & 0x0f) << 8);
		flipx = m_spriteram[offs+8] & 0x01;
		flipy = m_spriteram[offs+8] & 0x04;
		color = m_spriteram[offs+9] & 0x0f;

		sx = m_spriteram[offs+10];
		sy = m_spriteram[offs+12];

		if (m_spriteram[offs+11] & 0x01) sx-=256;
		if (m_spriteram[offs+13] & 0x01) sy-=256;

		sy = 240 - sy;

		if (m_flipscreen)
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		fx = flipx;
		fy = flipy;

		{
			int i, j, td;

			if ((offs >= 0x100 && offs <= 0x2ff) || (offs >= 0x400 && offs <= 0x57f))
			{
				m_blend->drawgfx(m_palette,
							bitmap,cliprect,m_gfxdecode->gfx(0),
							tile,
							color,
							flipx, flipy,
							sx, sy,
							7);
			}
			else if ((offs >= 0x000 && offs <= 0x0ff) || (offs >= 0x300 && offs <= 0x3ff))
			{
				for (i = 0; i <= 1; i++)
				{
					td = (fx) ? (1 - i) : i;

					m_blend->drawgfx(m_palette,
								bitmap,cliprect,m_gfxdecode->gfx(0),
								tile + td,
								color,
								flipx, flipy,
								sx + i * 16, sy,
								7);
				}
			}
			else if (offs >= 0x580 && offs <= 0x61f)
			{
				for (i = 0; i <= 1; i++)
				{
					for (j = 0; j <= 1; j++)
					{
						if (fy)
							td = (fx) ? ((1 - i) * 2) + 1 - j : (1 - i) * 2 + j;
						else
							td = (fx) ? (i * 2) + 1 - j : i * 2 + j;

						m_blend->drawgfx(m_palette,
									bitmap,cliprect,m_gfxdecode->gfx(0),
									tile + td,
									color,
									flipx, flipy,
									sx + j * 16, sy - i * 16,
									7);
					}
				}
			}
			else if (offs >= 0x620 && offs <= 0x67f)
			{
				for (i = 0; i <= 3; i++)
				{
					for (j = 0; j <= 3; j++)
					{
						if (fy)
							td = (fx) ? ((3 - i) * 4) + 3 - j : (3 - i) * 4 + j;
						else
							td = (fx) ? (i * 4) + 3 - j : i * 4 + j;

						m_blend->drawgfx(m_palette,
									bitmap,cliprect,m_gfxdecode->gfx(0),
									tile + td,
									color,
									flipx, flipy,
									sx + j * 16, sy - i * 16,
									7);
					}
				}
			}
		}
	}
}


void argus_state::butasan_log_vram()
{
#ifdef MAME_DEBUG
	int offs;

	if (machine().input().code_pressed(KEYCODE_M))
	{
		UINT8 *spriteram = m_spriteram;
		int i;
		logerror("\nSprite RAM\n");
		logerror("---------------------------------------\n");
		logerror("       +0 +1 +2 +3 +4 +5 +6 +7  +8 +9 +a +b +c +d +e +f\n");
		for (offs = 0; offs < m_spriteram.bytes(); offs += 16)
		{
			for (i = 0; i < 16; i++)
			{
				if (i == 0)
				{
					logerror("%04x : ", offs + 0xf000);
					logerror("%02x ", spriteram[offs]);
				}
				else if (i == 7)
					logerror("%02x  ", spriteram[offs + 7]);
				else if (i == 15)
					logerror("%02x\n", spriteram[offs + 15]);
				else
					logerror("%02x ", spriteram[offs + i]);
			}
		}
		logerror("\nColor RAM\n");
		logerror("---------------------------------------\n");
		logerror("       +0 +1 +2 +3 +4 +5 +6 +7  +8 +9 +a +b +c +d +e +f\n");
		for (offs = 0; offs < 0xbf0; offs += 16)
		{
			for (i = 0; i < 16; i++)
			{
				if (i == 0)
				{
					logerror("%04x : ", offs + 0xc400);
					logerror("%02x ", m_paletteram[offs]);
				}
				else if (i == 7)
					logerror("%02x  ", m_paletteram[offs + 7]);
				else if (i == 15)
					logerror("%02x\n", m_paletteram[offs + 15]);
				else
					logerror("%02x ", m_paletteram[offs + i]);
			}
		}
	}
#endif
}

UINT32 argus_state::screen_update_argus(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bg_setting();

	/* scroll BG0 and render tile at proper position */
	argus_bg0_scroll_handle();

	m_bg0_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	argus_draw_sprites(bitmap, cliprect, 0);
	if (m_bg_status & 1)    /* Backgound enable */
		m_bg1_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	argus_draw_sprites(bitmap, cliprect, 1);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

UINT32 argus_state::screen_update_valtric(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bg_setting();

	if (m_bg_status & 1)    /* Backgound enable */
		valtric_draw_mosaic(screen, bitmap, cliprect);
	else
		bitmap.fill(m_palette->black_pen(), cliprect);
	valtric_draw_sprites(bitmap, cliprect);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

UINT32 argus_state::screen_update_butasan(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bg_setting();

	if (m_bg_status & 1)    /* Backgound enable */
		m_bg0_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	else
		bitmap.fill(m_palette->black_pen(), cliprect);
	if (m_butasan_bg1_status & 1) m_bg1_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	butasan_draw_sprites(bitmap, cliprect);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	butasan_log_vram();
	return 0;
}
