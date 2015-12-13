// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*
Taito TC0480SCP
---------
Tilemap generator. Manages four background tilemaps with 16x16 tiles fetched
from ROM, and one foreground text tilemap with 8x8 tiles fetched from RAM.
All four background tilemaps support zooming and rowscroll, and two of them
additionally support per-row zooming and column scroll. The five tilemaps
are mixed internally (the text tilemap is always on top, the order of the
other four is selectable) and output as 16 bits of pixel data.
The TC0480SCP uses 0x10000 bytes of RAM. It seems to be able to address
up to 0x800000 bytes of ROM (0x10000 tiles) as it has 21 address lines and
32 data lines, but no known game uses more than 0x400000 bytes.

Inputs and outputs (based on Gunbuster schematics):
- CPU address bus (VA1-VA17)
- CPU data bus (VD0-VD15)
- CPU control lines (CS, UDS, LDS, R/W, DTACK)
- RAM address bus (RA0-RA14)
- RAM data bus (RAD0-RAD15)
- RAM control lines (RWAH, RWAL, RAOE)
- ROM address bus (CH0-CH20)
- ROM data bus (RD0-RD31)
- Pixel output (SD0-SD15)
- Clocks and video sync (HSYNC, HBLANK, VSYNC, VBLANK)

Standard memory layout (four 32x32 bg tilemaps, one 64x64 fg tilemap)

0000-0fff BG0
1000-1fff BG1
2000-2fff BG2
3000-3fff BG3
4000-43ff BG0 rowscroll
4400-47ff BG1 rowscroll
4800-4bff BG2 rowscroll
4c00-4fff BG3 rowscroll
5000-53ff BG0 rowscroll low order bytes (see info below)
5400-57ff BG1 rowscroll low order bytes
5800-5bff BG2 rowscroll low order bytes
5c00-5fff BG3 rowscroll low order bytes
6000-63ff BG2 row zoom
6400-67ff BG3 row zoom
6800-6bff BG2 source colscroll
6c00-6fff BG3 source colscroll
7000-bfff unknown/unused?
c000-dfff FG0
e000-ffff gfx data for FG0 (4bpp)

Double width tilemaps memory layout (four 64x32 bg tilemaps, one 64x64 fg tilemap)

0000-1fff BG0
2000-3fff BG1
4000-5fff BG2
6000-7fff BG3
8000-83ff BG0 rowscroll
8400-87ff BG1 rowscroll
8800-8bff BG2 rowscroll
8c00-8fff BG3 rowscroll
9000-93ff BG0 rowscroll low order bytes (used for accuracy with row zoom or layer zoom)
9400-97ff BG1 rowscroll low order bytes [*]
9800-9bff BG2 rowscroll low order bytes
9c00-9fff BG3 rowscroll low order bytes
a000-a3ff BG2 row zoom [+]
a400-a7ff BG3 row zoom
a800-abff BG2 source colscroll
ac00-afff BG3 source colscroll
b000-bfff unknown (Slapshot and Superchs poke in TBA OVER error message in FG0 format)
c000-dfff FG0
e000-ffff gfx data for FG0 (4bpp)

[* Gunbustr suggests that high bytes are irrelevant: it leaves them
all zeroed. Superchs is the only game which uses high bytes that
aren't the low byte of the main rowscroll (Footchmp/Undrfire have
this verified in the code).]

[+ Usual row zoom values are 0 - 0x7f. Gunbustr also uses 0x80-d0
approx. Undrfire keeps to the 0-0x7f range but oddly also uses
the high byte with a mask of 0x3f. Meaning of this high byte is
unknown.]

Bg layers tile word layout

+0x00   %yx..bbbb cccccccc      b=control bits(?) c=color .=unused(?)
+0x02   tilenum
[y=yflip x=xflip b=unknown seen in Metalb]

Control registers

000-001 BG0 x scroll    (layer priority order is definable)
002-003 BG1 x scroll
004-005 BG2 x scroll
006-007 BG3 x scroll
008-009 BG0 y scroll
00a-00b BG1 y scroll
00c-00d BG2 y scroll
00e-00f BG3 y scroll
010-011 BG0 zoom        (high byte = X zoom, low byte = Y zoom,
012-013 BG1 zoom         compression is allowed on Y axis only)
014-015 BG2 zoom
016-017 BG3 zoom
018-019 Text layer x scroll
01a-01b Text layer y scroll
01c-01d Unused (not written)
01e-01f Layer Control register
        x-------    Double width tilemaps (4 bg tilemaps become 64x32, and the
                    memory layout changes). Slapshot changes this on the fly.
        -x------    Flip screen
        --x-----    unknown

                Set in Metalb init by whether a byte in prg ROM $7fffe is zero.
                Subsequently Metalb changes it for some screen layer layouts.
                Footchmp clears it, Hthero sets it [then both leave it alone].
                Deadconx code at $10e2 is interesting, with possible values of:
                0x0, 0x20, 0x40, 0x60 poked in (via ram buffer) to control reg,
                dependent on byte in prg ROM $7fffd and whether screen is flipped.

        ---xxx--    BG layer priority order

        ...000..    0  1  2  3
        ...001..    1  2  3  0  (no evidence of this)
        ...010..    2  3  0  1  (no evidence of this)
        ...011..    3  0  1  2
        ...100..    3  2  1  0
        ...101..    2  1  0  3  [Gunbustr attract and Metalb (c) screen]
        ...110..    1  0  3  2  (no evidence of this)
        ...111..    0  3  2  1

        ------x-    BG3 row zoom enable
        -------x    BG2 row zoom enable

020-021 BG0 dx  (provides extra precision to x-scroll, only changed with xscroll)
022-023 BG1 dx
024-025 BG2 dx
026-027 BG3 dx
028-029 BG0 dy  (provides extra precision to y-scroll, only changed with yscroll)
02a-02b BG1 dy
02c-02d BG2 dy
02e-02f BG3 dy

[see code at $1b4a in Slapshot and $xxxxx in Undrfire for evidence of row areas]
*/

#include "emu.h"
#include "tc0480scp.h"
#include "video/taito_helper.h"

#define TC0480SCP_RAM_SIZE 0x10000
#define TC0480SCP_TOTAL_CHARS 256


const device_type TC0480SCP = &device_creator<tc0480scp_device>;

tc0480scp_device::tc0480scp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TC0480SCP, "Taito TC0480SCP", tag, owner, clock, "tc0480scp", __FILE__),
	m_tx_ram(nullptr),
	m_char_ram(nullptr),
	m_pri_reg(0),
	m_dblwidth(0),
	m_gfxnum(0),
	m_txnum(0),
	m_x_offset(0),
	m_y_offset(0),
	m_text_xoffs(0),
	m_text_yoffs(0),
	m_flip_xoffs(0),
	m_flip_yoffs(0),
	m_col_base(0),
	m_gfxdecode(*this),
	m_palette(*this)
{
	memset(m_ctrl, 0, sizeof(m_ctrl));

	for (int i = 0; i < 4; i++)
	{
		m_bg_ram[i] = nullptr;
		m_bgscroll_ram[i] = nullptr;
		m_rowzoom_ram[i] = nullptr;
		m_bgcolumn_ram[i] = nullptr;
		m_bgscrollx[i] = 0;
		m_bgscrolly[i] = 0;
	}
}

//-------------------------------------------------
//  static_set_gfxdecode_tag: Set the tag of the
//  gfx decoder
//-------------------------------------------------

void tc0480scp_device::static_set_gfxdecode_tag(device_t &device, const char *tag)
{
	downcast<tc0480scp_device &>(device).m_gfxdecode.set_tag(tag);
}

//-------------------------------------------------
//  static_set_palette_tag: Set the tag of the
//  palette device
//-------------------------------------------------

void tc0480scp_device::static_set_palette_tag(device_t &device, const char *tag)
{
	downcast<tc0480scp_device &>(device).m_palette.set_tag(tag);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tc0480scp_device::device_start()
{
	if(!m_gfxdecode->started())
		throw device_missing_dependencies();

	int xd, yd;

	static const gfx_layout tc0480scp_charlayout =
	{
		8,8,    /* 8*8 characters */
		256,    /* 256 characters */
		4,  /* 4 bits per pixel */
		{ 0, 1, 2, 3 },
		{ 3*4, 2*4, 1*4, 0*4, 7*4, 6*4, 5*4, 4*4 },
		{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
		32*8    /* every sprite takes 32 consecutive bytes */
	};


	/* Single width versions */
	m_tilemap[0][0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tc0480scp_device::get_bg0_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap[1][0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tc0480scp_device::get_bg1_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap[2][0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tc0480scp_device::get_bg2_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap[3][0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tc0480scp_device::get_bg3_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap[4][0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tc0480scp_device::get_tx_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	/* Double width versions */
	m_tilemap[0][1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tc0480scp_device::get_bg0_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_tilemap[1][1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tc0480scp_device::get_bg1_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_tilemap[2][1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tc0480scp_device::get_bg2_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_tilemap[3][1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tc0480scp_device::get_bg3_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_tilemap[4][1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tc0480scp_device::get_tx_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	for (int i = 0; i < 2; i++)
	{
		m_tilemap[0][i]->set_transparent_pen(0);
		m_tilemap[1][i]->set_transparent_pen(0);
		m_tilemap[2][i]->set_transparent_pen(0);
		m_tilemap[3][i]->set_transparent_pen(0);
		m_tilemap[4][i]->set_transparent_pen(0);
	}

	xd = -m_x_offset;
	yd =  m_y_offset;

	/* Metalb and Deadconx have minor screenflip issues: blue planet
	   is off on x axis by 1 and in Deadconx the dark blue screen
	   between stages also seems off by 1 pixel. */

	/* It's not possible to get the text scrolldx calculations
	   harmonised with the other layers: xd-2, 315-xd is the
	   next valid pair:- the numbers diverge from xd, 319-xd */

	/* Single width offsets */
	m_tilemap[0][0]->set_scrolldx(xd,     320 - xd + m_flip_xoffs);
	m_tilemap[0][0]->set_scrolldy(yd,     256 - yd + m_flip_yoffs);
	m_tilemap[1][0]->set_scrolldx(xd,     320 - xd + m_flip_xoffs);
	m_tilemap[1][0]->set_scrolldy(yd,     256 - yd + m_flip_yoffs);
	m_tilemap[2][0]->set_scrolldx(xd,     320 - xd + m_flip_xoffs);
	m_tilemap[2][0]->set_scrolldy(yd,     256 - yd + m_flip_yoffs);
	m_tilemap[3][0]->set_scrolldx(xd,     320 - xd + m_flip_xoffs);
	m_tilemap[3][0]->set_scrolldy(yd,     256 - yd + m_flip_yoffs);
	m_tilemap[4][0]->set_scrolldx(xd - 3, 316 - xd);   /* text layer */
	m_tilemap[4][0]->set_scrolldy(yd,     256 - yd);   /* text layer */

	/* Double width offsets */
	m_tilemap[0][1]->set_scrolldx(xd,     320 - xd + m_flip_xoffs);
	m_tilemap[0][1]->set_scrolldy(yd,     256 - yd + m_flip_yoffs);
	m_tilemap[1][1]->set_scrolldx(xd,     320 - xd + m_flip_xoffs);
	m_tilemap[1][1]->set_scrolldy(yd,     256 - yd + m_flip_yoffs);
	m_tilemap[2][1]->set_scrolldx(xd,     320 - xd + m_flip_xoffs);
	m_tilemap[2][1]->set_scrolldy(yd,     256 - yd + m_flip_yoffs);
	m_tilemap[3][1]->set_scrolldx(xd,     320 - xd + m_flip_xoffs);
	m_tilemap[3][1]->set_scrolldy(yd,     256 - yd + m_flip_yoffs);
	m_tilemap[4][1]->set_scrolldx(xd - 3, 317 - xd);   /* text layer */
	m_tilemap[4][1]->set_scrolldy(yd,     256 - yd);   /* text layer */

	for (int i = 0; i < 2; i++)
	{
		/* Both sets of bg tilemaps scrollable per pixel row */
		m_tilemap[0][i]->set_scroll_rows(512);
		m_tilemap[1][i]->set_scroll_rows(512);
		m_tilemap[2][i]->set_scroll_rows(512);
		m_tilemap[3][i]->set_scroll_rows(512);
	}

	m_ram.resize(TC0480SCP_RAM_SIZE / 2);
	memset(&m_ram[0], 0, TC0480SCP_RAM_SIZE);
	set_layer_ptrs();

	/* create the char set (gfx will then be updated dynamically from RAM) */
	m_gfxdecode->set_gfx(m_txnum, global_alloc(gfx_element(m_palette, tc0480scp_charlayout, (UINT8 *)m_char_ram, NATIVE_ENDIAN_VALUE_LE_BE(8,0), 64, m_col_base)));
	m_gfxdecode->gfx(m_gfxnum)->set_colorbase(m_col_base);

	save_item(NAME(m_ram));
	save_item(NAME(m_ctrl));
	save_item(NAME(m_bgscrollx));
	save_item(NAME(m_bgscrolly));
	save_item(NAME(m_pri_reg));
	save_item(NAME(m_dblwidth));
	machine().save().register_postload(save_prepost_delegate(FUNC(tc0480scp_device::postload), this));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tc0480scp_device::device_reset()
{
	m_dblwidth = 0;

	for (auto & elem : m_ctrl)
		elem = 0;

}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/


void tc0480scp_device::common_get_tc0480bg_tile_info( tile_data &tileinfo, int tile_index, UINT16 *ram, int gfxnum )
{
	int code = ram[2 * tile_index + 1] & 0x7fff;
	int attr = ram[2 * tile_index];
	SET_TILE_INFO_MEMBER(gfxnum,
			code,
			(attr & 0xff),
			TILE_FLIPYX((attr & 0xc000) >> 14));
}

void tc0480scp_device::common_get_tc0480tx_tile_info( tile_data &tileinfo, int tile_index, UINT16 *ram, int gfxnum )
{
	int attr = ram[tile_index];
	SET_TILE_INFO_MEMBER(gfxnum,
			attr & 0xff,
			((attr & 0x3f00) >> 8),
			TILE_FLIPYX((attr & 0xc000) >> 14));
}

TILE_GET_INFO_MEMBER(tc0480scp_device::get_bg0_tile_info)
{
	common_get_tc0480bg_tile_info(tileinfo, tile_index, m_bg_ram[0], m_gfxnum );
}

TILE_GET_INFO_MEMBER(tc0480scp_device::get_bg1_tile_info)
{
	common_get_tc0480bg_tile_info(tileinfo, tile_index, m_bg_ram[1], m_gfxnum);
}

TILE_GET_INFO_MEMBER(tc0480scp_device::get_bg2_tile_info)
{
	common_get_tc0480bg_tile_info(tileinfo, tile_index, m_bg_ram[2], m_gfxnum);
}

TILE_GET_INFO_MEMBER(tc0480scp_device::get_bg3_tile_info)
{
	common_get_tc0480bg_tile_info(tileinfo, tile_index, m_bg_ram[3], m_gfxnum);
}

TILE_GET_INFO_MEMBER(tc0480scp_device::get_tx_tile_info)
{
	common_get_tc0480tx_tile_info(tileinfo, tile_index, m_tx_ram, m_txnum);
}

void tc0480scp_device::dirty_tilemaps()
{
	m_tilemap[0][m_dblwidth]->mark_all_dirty();
	m_tilemap[1][m_dblwidth]->mark_all_dirty();
	m_tilemap[2][m_dblwidth]->mark_all_dirty();
	m_tilemap[3][m_dblwidth]->mark_all_dirty();
	m_tilemap[4][m_dblwidth]->mark_all_dirty();
}


void tc0480scp_device::set_layer_ptrs()
{
	if (!m_dblwidth)
	{
		m_bg_ram[0]       = &m_ram[0x0000]; //0000
		m_bg_ram[1]       = &m_ram[0x0800]; //1000
		m_bg_ram[2]       = &m_ram[0x1000]; //2000
		m_bg_ram[3]       = &m_ram[0x1800]; //3000
		m_bgscroll_ram[0] = &m_ram[0x2000]; //4000
		m_bgscroll_ram[1] = &m_ram[0x2200]; //4400
		m_bgscroll_ram[2] = &m_ram[0x2400]; //4800
		m_bgscroll_ram[3] = &m_ram[0x2600]; //4c00
		m_rowzoom_ram[2]  = &m_ram[0x3000]; //6000
		m_rowzoom_ram[3]  = &m_ram[0x3200]; //6400
		m_bgcolumn_ram[2] = &m_ram[0x3400]; //6800
		m_bgcolumn_ram[3] = &m_ram[0x3600]; //6c00
		m_tx_ram          = &m_ram[0x6000]; //c000
		m_char_ram        = &m_ram[0x7000]; //e000
	}
	else
	{
		m_bg_ram[0]       = &m_ram[0x0000]; //0000
		m_bg_ram[1]       = &m_ram[0x1000]; //2000
		m_bg_ram[2]       = &m_ram[0x2000]; //4000
		m_bg_ram[3]       = &m_ram[0x3000]; //6000
		m_bgscroll_ram[0] = &m_ram[0x4000]; //8000
		m_bgscroll_ram[1] = &m_ram[0x4200]; //8400
		m_bgscroll_ram[2] = &m_ram[0x4400]; //8800
		m_bgscroll_ram[3] = &m_ram[0x4600]; //8c00
		m_rowzoom_ram[2]  = &m_ram[0x5000]; //a000
		m_rowzoom_ram[3]  = &m_ram[0x5200]; //a400
		m_bgcolumn_ram[2] = &m_ram[0x5400]; //a800
		m_bgcolumn_ram[3] = &m_ram[0x5600]; //ac00
		m_tx_ram          = &m_ram[0x6000]; //c000
		m_char_ram        = &m_ram[0x7000]; //e000
	}
}

READ16_MEMBER( tc0480scp_device::word_r )
{
	return m_ram[offset];
}

WRITE16_MEMBER( tc0480scp_device::word_w )
{
	COMBINE_DATA(&m_ram[offset]);

	if (!m_dblwidth)
	{
		if (offset < 0x2000)
		{
			m_tilemap[(offset / 0x800)][m_dblwidth]->mark_tile_dirty(((offset % 0x800) / 2));
		}
		else if (offset < 0x6000)
		{   /* do nothing */
		}
		else if (offset < 0x7000)
		{
			m_tilemap[4][m_dblwidth]->mark_tile_dirty((offset - 0x6000));
		}
		else if (offset <= 0x7fff)
		{
			m_gfxdecode->gfx(m_txnum)->mark_dirty((offset - 0x7000) / 16);
		}
	}
	else
	{
		if (offset < 0x4000)
		{
			m_tilemap[(offset / 0x1000)][m_dblwidth]->mark_tile_dirty(((offset % 0x1000) / 2));
		}
		else if (offset < 0x6000)
		{   /* do nothing */
		}
		else if (offset < 0x7000)
		{
			m_tilemap[4][m_dblwidth]->mark_tile_dirty((offset - 0x6000));
		}
		else if (offset <= 0x7fff)
		{
			m_gfxdecode->gfx(m_txnum)->mark_dirty((offset - 0x7000) / 16);
		}
	}
}

READ16_MEMBER( tc0480scp_device::ctrl_word_r )
{
	return m_ctrl[offset];
}

WRITE16_MEMBER( tc0480scp_device::ctrl_word_w )
{
	int flip = m_pri_reg & 0x40;

	COMBINE_DATA(&m_ctrl[offset]);
	data = m_ctrl[offset];

	switch (offset)
	{
		/* The x offsets of the four bg layers are staggered by intervals of 4 pixels */
		case 0x00:   /* bg0 x */
			if (!flip)  data = -data;
			m_bgscrollx[0] = data;
			break;

		case 0x01:   /* bg1 x */
			data += 4;
			if (!flip)  data = -data;
			m_bgscrollx[1] = data;
			break;

		case 0x02:   /* bg2 x */
			data += 8;
			if (!flip)  data = -data;
			m_bgscrollx[2] = data;
			break;

		case 0x03:   /* bg3 x */
			data += 12;
			if (!flip)  data = -data;
			m_bgscrollx[3] = data;
			break;

		case 0x04:   /* bg0 y */
			if (flip)  data = -data;
			m_bgscrolly[0] = data;
			break;

		case 0x05:   /* bg1 y */
			if (flip)  data = -data;
			m_bgscrolly[1] = data;
			break;

		case 0x06:   /* bg2 y */
			if (flip)  data = -data;
			m_bgscrolly[2] = data;
			break;

		case 0x07:   /* bg3 y */
			if (flip)  data = -data;
			m_bgscrolly[3] = data;
			break;

		case 0x08:   /* bg0 zoom */
		case 0x09:   /* bg1 zoom */
		case 0x0a:   /* bg2 zoom */
		case 0x0b:   /* bg3 zoom */
			break;

		case 0x0c:   /* fg (text) x */

			/* Text layer can be offset from bg0 (e.g. Metalb) */
			if (!flip)  data -= m_text_xoffs;
			if (flip)   data += m_text_xoffs;

			m_tilemap[4][0]->set_scrollx(0, -data);
			m_tilemap[4][1]->set_scrollx(0, -data);
			break;

		case 0x0d:   /* fg (text) y */

			/* Text layer can be offset from bg0 (e.g. Slapshot) */
			if (!flip)  data -= m_text_yoffs;
			if (flip)   data += m_text_yoffs;

			m_tilemap[4][0]->set_scrolly(0, -data);
			m_tilemap[4][1]->set_scrolly(0, -data);
			break;

		/* offset 0x0e unused */

		case 0x0f:   /* control register */
		{
			int old_width = (m_pri_reg & 0x80) >> 7;
			flip = (data & 0x40) ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0;
			m_pri_reg = data;

			m_tilemap[0][0]->set_flip(flip);
			m_tilemap[1][0]->set_flip(flip);
			m_tilemap[2][0]->set_flip(flip);
			m_tilemap[3][0]->set_flip(flip);
			m_tilemap[4][0]->set_flip(flip);

			m_tilemap[0][1]->set_flip(flip);
			m_tilemap[1][1]->set_flip(flip);
			m_tilemap[2][1]->set_flip(flip);
			m_tilemap[3][1]->set_flip(flip);
			m_tilemap[4][1]->set_flip(flip);

			m_dblwidth = (m_pri_reg & 0x80) >> 7;

			if (m_dblwidth != old_width)   /* tilemap width is changing */
			{
				/* Reinitialise layer pointers */
				set_layer_ptrs();

				/* and ensure full redraw of tilemaps */
				dirty_tilemaps();
			}

			break;
		}

		/* Rest are layer specific delta x and y, used while scrolling that layer */
	}
}


READ32_MEMBER( tc0480scp_device::ctrl_long_r )
{
	return (ctrl_word_r(space, offset * 2, 0xffff) << 16) | ctrl_word_r(space, offset * 2 + 1, 0xffff);
}

/* TODO: byte access ? */

WRITE32_MEMBER( tc0480scp_device::ctrl_long_w )
{
	if (ACCESSING_BITS_16_31)
		ctrl_word_w(space, offset * 2, data >> 16, mem_mask >> 16);
	if (ACCESSING_BITS_0_15)
		ctrl_word_w(space, (offset * 2) + 1, data & 0xffff, mem_mask & 0xffff);
}

READ32_MEMBER( tc0480scp_device::long_r )
{
	return (word_r(space, offset * 2, 0xffff) << 16) | word_r(space, offset * 2 + 1, 0xffff);
}

WRITE32_MEMBER( tc0480scp_device::long_w )
{
	if (ACCESSING_BITS_16_31)
	{
		int oldword = word_r(space, offset * 2, 0xffff);
		int newword = data >> 16;
		if (!ACCESSING_BITS_16_23)
			newword |= (oldword & 0x00ff);
		if (!ACCESSING_BITS_24_31)
			newword |= (oldword & 0xff00);
		word_w(space, offset * 2, newword, 0xffff);
	}
	if (ACCESSING_BITS_0_15)
	{
		int oldword = word_r(space, (offset * 2) + 1, 0xffff);
		int newword = data & 0xffff;
		if (!ACCESSING_BITS_0_7)
			newword |= (oldword & 0x00ff);
		if (!ACCESSING_BITS_8_15)
			newword |= (oldword & 0xff00);
		word_w(space, (offset * 2) + 1, newword, 0xffff);
	}
}


void tc0480scp_device::tilemap_update()
{
	int layer, zoom, i, j;
	int flip = m_pri_reg & 0x40;

	for (layer = 0; layer < 4; layer++)
	{
		m_tilemap[layer][m_dblwidth]->set_scrolly(0, m_bgscrolly[layer]);
		zoom = 0x10000 + 0x7f - m_ctrl[0x08 + layer];

		if (zoom != 0x10000)    /* can't use scroll rows when zooming */
		{
			m_tilemap[layer][m_dblwidth]->set_scrollx(0, m_bgscrollx[layer]);
		}
		else
		{
			for (j = 0; j < 512; j++)
			{
				i = m_bgscroll_ram[layer][j];

				if (!flip)
					m_tilemap[layer][m_dblwidth]->set_scrollx(j & 0x1ff, m_bgscrollx[layer] - i);
				else
					m_tilemap[layer][m_dblwidth]->set_scrollx(j & 0x1ff, m_bgscrollx[layer] + i);
			}
		}
	}
}


/*********************************************************************
                BG0,1 LAYER DRAW

TODO
----

Wouldn't work if y needs to be > 255 (i.e. if some game uses a
bigger than usual vertical visible area). Refer to tc0080vco
custom draw routine for an example of dealing with this.


Historical Issues
-----------------

1) bg layers got too far left and down, the greater the magnification.
   Largely fixed by adding offsets (to sx&y) which get bigger as we
   zoom in (why we have *zoomx and *zoomy in the calculations).

2) Hthero and Footchmp bg layers behaved differently when zoomed.
   Fixed by bringing tc0480scp_x&y_offs into calculations.

3) Metalb "TAITO" text in attract too far to the right. Fixed by
   bringing (layer*4) into offset calculations. But might be possible
   to avoid this by stepping the scroll deltas for the four layers -
   currently they are the same, and we have to kludge the offsets in
   TC0480SCP_ctrl_word_write.

4) Zoom movement was jagged: improved by bringing in scroll delta
   values... but the results are noticeably imperfect.

**********************************************************************/

void tc0480scp_device::bg01_draw( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority )
{
	/* X-axis zoom offers expansion only: 0 = no zoom, 0xff = max
	   Y-axis zoom offers expansion/compression: 0x7f = no zoom, 0xff = max
	   (0x1a in Footchmp hiscore = shrunk) */

	int zoomx = 0x10000 - (m_ctrl[0x08 + layer] & 0xff00);
	int zoomy = 0x10000 - (((m_ctrl[0x08 + layer] & 0xff) - 0x7f) * 512);

	if ((zoomx == 0x10000) && (zoomy == 0x10000))   /* no zoom, simple */
	{
		/* Prevent bad things */
		m_tilemap[layer][m_dblwidth]->draw(screen, bitmap, cliprect, flags, priority);
	}
	else    /* zoom */
	{
		UINT16 *dst16, *src16;
		UINT8 *tsrc;
		UINT16 scanline[512];
		UINT32 sx;
		bitmap_ind16 &srcbitmap = m_tilemap[layer][m_dblwidth]->pixmap();
		bitmap_ind8 &flagsbitmap = m_tilemap[layer][m_dblwidth]->flagsmap();
		int flip = m_pri_reg & 0x40;
		int y_index, src_y_index, row_index;
		int x_index, x_step;

		UINT16 screen_width = 512; //cliprect.width();
		UINT16 min_y = cliprect.min_y;
		UINT16 max_y = cliprect.max_y;

		int width_mask = 0x1ff;
		if (m_dblwidth)
			width_mask = 0x3ff;

		if (!flip)
		{
			sx = ((m_bgscrollx[layer] + 15 + layer * 4) << 16) + ((255 - (m_ctrl[0x10 + layer] & 0xff)) << 8);
			sx += (m_x_offset - 15 - layer * 4) * zoomx;

			y_index = (m_bgscrolly[layer] << 16) + ((m_ctrl[0x14 + layer] & 0xff) << 8);
			y_index -= (m_y_offset - min_y) * zoomy;
		}
		else    /* TC0480SCP tiles flipscreen */
		{
			sx = ((-m_bgscrollx[layer] + 15 + layer * 4 + m_flip_xoffs ) << 16) + ((255-(m_ctrl[0x10 + layer] & 0xff)) << 8);
			sx += (m_x_offset - 15 - layer * 4) * zoomx;

			y_index = ((-m_bgscrolly[layer] + m_flip_yoffs) << 16) + ((m_ctrl[0x14 + layer] & 0xff) << 8);
			y_index -= (m_y_offset - min_y) * zoomy;
		}

		for (int y = min_y; y <= max_y; y++)
		{
			src_y_index = (y_index >> 16) & 0x1ff;

			/* row areas are the same in flipscreen, so we must read in reverse */
			row_index = src_y_index;
			if (flip)
				row_index = 0x1ff - row_index;

			x_index = sx - ((m_bgscroll_ram[layer][row_index] << 16)) - ((m_bgscroll_ram[layer][row_index + 0x800] << 8) & 0xffff);

			src16 = &srcbitmap.pix16(src_y_index);
			tsrc = &flagsbitmap.pix8(src_y_index);
			dst16 = scanline;

			x_step = zoomx;

			if (flags & TILEMAP_DRAW_OPAQUE)
			{
				for (int i = 0; i < screen_width; i++)
				{
					*dst16++ = src16[(x_index >> 16) & width_mask];
					x_index += x_step;
				}
			}
			else
			{
				for (int i = 0; i < screen_width; i++)
				{
					if (tsrc[(x_index >> 16) & width_mask])
						*dst16++ = src16[(x_index >> 16) & width_mask];
					else
						*dst16++ = 0x8000;
					x_index += x_step;
				}
			}

			taitoic_drawscanline(bitmap, cliprect, 0, y, scanline, (flags & TILEMAP_DRAW_OPAQUE) ? 0 : 1, ROT0, screen.priority(), priority);

			y_index += zoomy;
		}
	}
}


/****************************************************************
                BG2,3 LAYER DRAW

TODO
----

Wouldn't work if y needs to be > 255 (i.e. if some game uses a
bigger than usual vertical visible area). Refer to tc0080vco
custom draw routine for an example of dealing with this.

Low order words for overall layer zoom are not really understood.
In Metalbj initial text screen zoom you can see they ARE words
(not separate bytes); however, I just use the low byte to smooth
the zooming sequences. This is noticeably imperfect on the Y axis.

Verify behaviour of Taito logo (Gunbustr) against real machine
to perfect the row zoom emulation.

What do high bytes of row zoom do - if anything - in UndrFire?
There is still jaggedness to the road in this game and Superchs.


Historical Issues
-----------------

Sometimes BG2/3 were misaligned by 1 pixel horizontally: this
was due to low order byte of 0 causing different (sx >> 16) than
when it was 1-255. To prevent this we use (255-byte) so
(sx >> 16) no longer depends on the low order byte.

In flipscreen we have to bring in extra offsets, since various
games don't have exactly (320-,256-) tilemap scroll deltas in
flipscreen.

****************************************************************/

void tc0480scp_device::bg23_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority )
{
	bitmap_ind16 &srcbitmap = m_tilemap[layer][m_dblwidth]->pixmap();
	bitmap_ind8 &flagsbitmap = m_tilemap[layer][m_dblwidth]->flagsmap();

	UINT16 *dst16, *src16;
	UINT8 *tsrc;
	int y_index, src_y_index, row_index, row_zoom;
	int sx, x_index, x_step;
	UINT32 zoomx, zoomy;
	UINT16 scanline[512];
	int flipscreen = m_pri_reg & 0x40;

	UINT16 screen_width = 512; //cliprect.width();
	UINT16 min_y = cliprect.min_y;
	UINT16 max_y = cliprect.max_y;

	int width_mask = 0x1ff;
	if (m_dblwidth)
		width_mask = 0x3ff;

	/* X-axis zoom offers expansion only: 0 = no zoom, 0xff = max
	   Y-axis zoom offers expansion/compression: 0x7f = no zoom, 0xff = max
	   (0x1a in Footchmp hiscore = shrunk) */

	zoomx = 0x10000 - (m_ctrl[0x08 + layer] & 0xff00);
	zoomy = 0x10000 - (((m_ctrl[0x08 + layer] & 0xff) - 0x7f) * 512);

	if (!flipscreen)
	{
		sx = ((m_bgscrollx[layer] + 15 + layer * 4) << 16) + ((255-(m_ctrl[0x10 + layer] & 0xff)) << 8);
		sx += (m_x_offset - 15 - layer * 4) * zoomx;

		y_index = (m_bgscrolly[layer] << 16) + ((m_ctrl[0x14 + layer] & 0xff) << 8);
		y_index -= (m_y_offset - min_y) * zoomy;
	}
	else    /* TC0480SCP tiles flipscreen */
	{
		sx = ((-m_bgscrollx[layer] + 15 + layer * 4 + m_flip_xoffs ) << 16) + ((255 - (m_ctrl[0x10 + layer] & 0xff)) << 8);
		sx += (m_x_offset - 15 - layer * 4) * zoomx;

		y_index = ((-m_bgscrolly[layer] + m_flip_yoffs) << 16) + ((m_ctrl[0x14 + layer] & 0xff) << 8);
		y_index -= (m_y_offset - min_y) * zoomy;
	}

	for (int y = min_y; y <= max_y; y++)
	{
		if (!flipscreen)
			src_y_index = ((y_index>>16) + m_bgcolumn_ram[layer][(y - m_y_offset) & 0x1ff]) & 0x1ff;
		else    /* colscroll area is back to front in flipscreen */
			src_y_index = ((y_index>>16) + m_bgcolumn_ram[layer][0x1ff - ((y - m_y_offset) & 0x1ff)]) & 0x1ff;

		/* row areas are the same in flipscreen, so we must read in reverse */
		row_index = src_y_index;
		if (flipscreen)
			row_index = 0x1ff - row_index;

		if (m_pri_reg & (layer - 1))   /* bit0 enables for BG2, bit1 for BG3 */
			row_zoom = m_rowzoom_ram[layer][row_index];
		else
			row_zoom = 0;

		x_index = sx - ((m_bgscroll_ram[layer][row_index] << 16)) - ((m_bgscroll_ram[layer][row_index + 0x800] << 8) & 0xffff);

		/* flawed calc ?? */
		x_index -= (m_x_offset - 0x1f + layer * 4) * ((row_zoom & 0xff) << 8);

/* We used to kludge 270 multiply factor, before adjusting x_index instead */

		x_step = zoomx;
		if (row_zoom)   /* need to reduce x_step */
		{
			if (!(row_zoom & 0xff00))
				x_step -= ((row_zoom * 256) & 0xffff);
			else    /* Undrfire uses the hi byte, why? */
				x_step -= (((row_zoom & 0xff) * 256) & 0xffff);
		}

		src16 = &srcbitmap.pix16(src_y_index);
		tsrc = &flagsbitmap.pix8(src_y_index);
		dst16 = scanline;

		if (flags & TILEMAP_DRAW_OPAQUE)
		{
			for (int i = 0; i < screen_width; i++)
			{
				*dst16++ = src16[(x_index >> 16) & width_mask];
				x_index += x_step;
			}
		}
		else
		{
			for (int i = 0; i < screen_width; i++)
			{
				if (tsrc[(x_index >> 16) & width_mask])
					*dst16++ = src16[(x_index >> 16) & width_mask];
				else
					*dst16++ = 0x8000;
				x_index += x_step;
			}
		}

		taitoic_drawscanline(bitmap, cliprect, 0, y, scanline, (flags & TILEMAP_DRAW_OPAQUE) ? 0 : 1, ROT0, screen.priority(), priority);

		y_index += zoomy;
	}
}


void tc0480scp_device::tilemap_draw( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority )
{
	/* no layer disable bits */
	switch (layer)
	{
		case 0:
			bg01_draw(screen, bitmap, cliprect, 0, flags, priority);
			break;
		case 1:
			bg01_draw(screen, bitmap, cliprect, 1, flags, priority);
			break;
		case 2:
			bg23_draw(screen, bitmap, cliprect, 2, flags, priority);
			break;
		case 3:
			bg23_draw(screen, bitmap, cliprect, 3, flags, priority);
			break;
		case 4:
			m_tilemap[4][m_dblwidth]->draw(screen, bitmap, cliprect, flags, priority);
			break;
	}
}

/* For evidence table of TC0480SCP bg layer priorities, refer to mame55 source */

static const UINT16 tc0480scp_bg_pri_lookup[8] =
{
	0x0123,
	0x1230,
	0x2301,
	0x3012,
	0x3210,
	0x2103,
	0x1032,
	0x0321
};

int tc0480scp_device::get_bg_priority()
{
	return tc0480scp_bg_pri_lookup[(m_pri_reg & 0x1c) >> 2];
}

// undrfire.c also needs to directly access the priority reg
READ8_MEMBER( tc0480scp_device::pri_reg_r )
{
	return m_pri_reg;
}

void tc0480scp_device::postload()
{
	int reg;
	int flip = m_ctrl[0xf] & 0x40;

	set_layer_ptrs();

	m_tilemap[0][0]->set_flip(flip);
	m_tilemap[1][0]->set_flip(flip);
	m_tilemap[2][0]->set_flip(flip);
	m_tilemap[3][0]->set_flip(flip);
	m_tilemap[4][0]->set_flip(flip);

	m_tilemap[0][1]->set_flip(flip);
	m_tilemap[1][1]->set_flip(flip);
	m_tilemap[2][1]->set_flip(flip);
	m_tilemap[3][1]->set_flip(flip);
	m_tilemap[4][1]->set_flip(flip);

	reg = m_ctrl[0];
	if (!flip)  reg = -reg;
	m_bgscrollx[0] = reg;

	reg = m_ctrl[1] + 4;
	if (!flip)  reg = -reg;
	m_bgscrollx[1] = reg;

	reg = m_ctrl[2] + 8;
	if (!flip)  reg = -reg;
	m_bgscrollx[2] = reg;

	reg = m_ctrl[3] + 12;
	if (!flip)  reg = -reg;
	m_bgscrollx[3] = reg;

	reg = m_ctrl[4];
	if (flip)  reg = -reg;
	m_bgscrolly[0] = reg;

	reg = m_ctrl[5];
	if (flip)  reg = -reg;
	m_bgscrolly[1] = reg;

	reg = m_ctrl[6];
	if (flip)  reg = -reg;
	m_bgscrolly[2] = reg;

	reg = m_ctrl[7];
	if (flip)  reg = -reg;
	m_bgscrolly[3] = reg;

	reg = m_ctrl[0x0c];
	if (!flip)  reg -= m_text_xoffs;
	if (flip)   reg += m_text_xoffs;
	m_tilemap[4][0]->set_scrollx(0, -reg);
	m_tilemap[4][1]->set_scrollx(0, -reg);

	reg = m_ctrl[0x0d];
	if (!flip)  reg -= m_text_yoffs;
	if (flip)   reg += m_text_yoffs;
	m_tilemap[4][0]->set_scrolly(0, -reg);
	m_tilemap[4][1]->set_scrolly(0, -reg);
}
