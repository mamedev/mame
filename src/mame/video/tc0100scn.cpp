// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*
Taito TC0100SCN
---------
Tilemap generator. Manages two background tilemaps with 8x8 tiles fetched
from ROM, and one foreground text tilemap with tiles fetched from RAM.
Both background layers support rowscroll and one of them additionally
supports columnscroll. The three tilemaps are mixed internally (the text
tilemap is always on top, the other two are selectable) and output as
15 bits of pixel data.
The TC0100SCN uses 0x10000 bytes of RAM, plus an optional 0x4000 bytes to
allow wider tilemaps (mainly used by multiscreen games). It can address
up to 0x200000 bytes of ROM (0x10000 tiles), 16 bits at a time.

Inputs and outputs (based on Operation Thunderbolt schematics):
- CPU address bus (VA1-VA17)
- CPU data bus (D0-D15)
- CPU control lines (CS, UDS, LDS, R/W, DTACK)
- RAM address bus (SA0-SA14)
- RAM data bus (SD0-SD15)
- RAM control lines (WEH, WEL, SCE0, SCE1)
  (SCE0 is connected to CS of two 32Kx8 SRAMs. SCE1 is unconnected.
   SCE1 is probably for the optional RAM, which isn't present on
   Operation Thunderbolt)
- ROM address bus (AD0-AD19)
- ROM data bus (RD0-RD15)
- Pixel output (SC0-SC14)
- Clocks and video sync (HSYNC, HBLANK, VSYNC, VBLANK)

Standard memory layout (three 64x64 tilemaps)

0000-3fff BG0
4000-5fff FG0
6000-6fff gfx data for FG0
7000-7fff unused (probably)
8000-bfff BG1
c000-c3ff BG0 rowscroll (second half unused*)
c400-c7ff BG1 rowscroll (second half unused*)
c800-dfff unused (probably)
e000-e0ff BG1 colscroll [see info below]
e100-ffff unused (probably)

Double width tilemaps memory layout (two 128x64 tilemaps, one 128x32 tilemap)

00000-07fff BG0 (128x64)
08000-0ffff BG1 (128x64)
10000-103ff BG0 rowscroll (second half unused*)
10400-107ff BG1 rowscroll (second half unused*)
10800-108ff BG1 colscroll [evidenced by Warriorb inits from $1634]
10900-10fff unused (probably)
11000-11fff gfx data for FG0
12000-13fff FG0 (128x32)

* Perhaps Taito wanted potential for double height tilemaps on the
  TC0100SCN. The inits state the whole area is "linescroll".

Control registers

000-001 BG0 scroll X
002-003 BG1 scroll X
004-005 FG0 scroll X
006-007 BG0 scroll Y
008-009 BG1 scroll Y
00a-00b FG0 scroll Y
00c-00d ---------------x BG0 disable
        --------------x- BG1 disable
        -------------x-- FG0 disable
        ------------x--- change priority order from BG0-BG1-FG0 to BG1-BG0-FG0
        -----------x---- double width tilemaps + different memory map
                              (cameltru and all the multi-screen games)
        ----------x----- unknown (set in most of the TaitoZ games and Cadash)
00e-00f ---------------x flip screen
        ----------x----- this TC0100SCN is subsidiary [= not the main one]
                              (Multi-screen games only. Could it mean: "write
                               through what is written into main TC0100SCN" ?)
        --x------------- unknown (thunderfox)


Colscroll [standard layout]
=========

The e000-ff area is not divided into two halves, it appears to refer only
to bg1 - the top most layer unless bg0/1 are flipped.

128 words are available in 0xe0?? area. Every word scrolls 8
pixels.

Growl
-----
This uses column scroll in the boat scene [that's just after you have
disposed of the fat men in fezzes] and in the underground lava cavern
scene.

Boat scene: code from $2eb58 appears to be doing see-saw motion for
water layer under boat. $e08c is highest word written, it oscillates
between fffa and 0005. Going back towards $e000 a middle point is reached
which sticks at zero. By $e000 written values oscillate again.

A total of 80 words are being written to [some below 0xe000, I think those
won't do anything, sloppy coding...]

Cavern scene: code from $3178a moves a sequence of 0s, 1s and 0x1ffs
along. These words equate to 0, +1, -1 so will gently ripple bg 0
up and down adding to the shimmering heat effect.

Ninja Kids
----------
This uses column scroll in the fat flame boss scene [that's the end of
round 2] and in the last round in the final confrontation with Satan scene.

Fat flame boss: code at $8eee moves a sequence of 1s and 0s along. This
is similar to the heat shimmer in Growl cavern scene.

Final boss: code at $a024 moves a sine wave of values 0-4 along. When
you are close to getting him the range of values expands to 0-10.

Gunfront
--------
In demo mode when the boss appears with the clouds, a sequence of 40 words
forming sine wave between 0xffd0 and ffe0 is moved along. Bg0 has been
given priority over bg1 so it's the foreground (clouds) affected.

The 40 words will affect 40 8-pixel columns [rows, as this game is
rotated] i.e. what is visible on screen at any point.

Galmedes
--------
Towards end of first round in empty starfield area, about three big ship
sprites cross the screen (scrolling down with the starfield). 16 starfield
columns [rows, as the game is rotated] scroll across with the ship.
$84fc0 and neighbouring routines poke col scroll area.

TC0620SCC
---------
The TC0620SCC seems to be similar to the TC0100SCN except that the ROM tiles
are 6bpp instead of 4bpp. It probably has a 24-bit bus to the ROMs instead
of 16-bit, but nothing else is known about it (such as whether it supports
the wide tilemap mode)

*/

#include "emu.h"
#include "tc0100scn.h"
#include "screen.h"

#include <algorithm>

#define TC0100SCN_RAM_SIZE        0x14000   /* enough for double-width tilemaps */
#define TC0100SCN_TOTAL_CHARS     256

DEFINE_DEVICE_TYPE(TC0100SCN, tc0100scn_device, "tc0100scn", "Taito TC0100SCN")
DEFINE_DEVICE_TYPE(TC0620SCC, tc0620scc_device, "tc0620scc", "Taito TC0620SCC")

tc0100scn_base_device::tc0100scn_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_gfx_interface(mconfig, *this)
	, m_gfxlayout(TC0100SCN_LAYOUT_DEFAULT)
	, m_ram(nullptr)
	, m_bgscroll_ram(nullptr)
	, m_fgscroll_ram(nullptr)
	, m_colscroll_ram(nullptr)
	, m_bgscrollx(0)
	, m_bgscrolly(0)
	, m_fgscrollx(0)
	, m_fgscrolly(0)
	, m_tx_colbank(0)
	, m_dblwidth(0)
	, m_dirty(false)
	, m_x_offset(0)
	, m_y_offset(0)
	, m_flip_xoffs(0)
	, m_flip_yoffs(0)
	, m_flip_text_xoffs(0)
	, m_flip_text_yoffs(0)
	, m_multiscrn_xoffs(0)
	, m_multiscrn_hack(0)
	, m_col_base(0)
{
	std::fill(std::begin(m_bg_colbank), std::end(m_bg_colbank), 0);
}

tc0100scn_device::tc0100scn_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tc0100scn_base_device(mconfig, TC0100SCN, tag, owner, clock)
{
}

tc0620scc_device::tc0620scc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tc0100scn_base_device(mconfig, TC0620SCC, tag, owner, clock)
{
}

/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

GFXDECODE_MEMBER(tc0100scn_device::gfxinfo_default)
	GFXDECODE_DEVICE(DEVICE_SELF, 0, gfx_8x8x4_packed_msb, 0, 256)
GFXDECODE_END

GFXDECODE_MEMBER(tc0100scn_device::gfxinfo_1bpp)
	GFXDECODE_DEVICE(DEVICE_SELF, 0, gfx_8x8x1, 0, 256)
GFXDECODE_END

static const gfx_layout layout_scc_6bpp_hi =
{
	8,8,
	RGN_FRAC(1,1),
	6,
	{ STEP2(0,1)/**/,0,0,0,0/**/ },
	{ STEP8(0,2) },
	{ STEP8(0,8*2) },
	8*8*2
};

GFXDECODE_MEMBER(tc0620scc_device::gfxinfo_6bpp)
	GFXDECODE_DEVICE(DEVICE_SELF, 0, gfx_8x8x4_packed_msb, 0, 256) // low 4bpp of 6bpp tiles
	GFXDECODE_DEVICE("hi_gfx",    0, layout_scc_6bpp_hi,   0, 256) // hi 2bpp of 6bpp tiles
GFXDECODE_END

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tc0100scn_base_device::device_start()
{
	// bind callbacks
	m_tc0100scn_cb.bind_relative_to(*owner());

	static const gfx_layout charlayout =
	{
	8,8,    /* 8*8 characters */
	256,    /* 256 characters */
	2,  /* 2 bits per pixel */
	{ 0, 8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8    /* every sprite takes 16 consecutive bytes */
	};

	int xd, yd;

	/* Set up clipping for multi-TC0100SCN games. We assume
	   this code won't ever affect single screen games:
	   Thundfox is the only one of those with two chips, and
	   we're safe as it uses single width tilemaps. */

	/* Single width versions */
	m_tilemap[0][0] = &machine().tilemap().create(*this, tilemap_get_info_delegate(&tc0100scn_base_device::get_bg_tile_info<0x00000, 0>, "bg0_std", this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[1][0] = &machine().tilemap().create(*this, tilemap_get_info_delegate(&tc0100scn_base_device::get_bg_tile_info<0x04000, 1>, "bg1_std", this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_tilemap[2][0] = &machine().tilemap().create(*this, tilemap_get_info_delegate(&tc0100scn_base_device::get_tx_tile_info<0x02000, 1>, "txt_std", this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	/* Double width versions */
	m_tilemap[0][1] = &machine().tilemap().create(*this, tilemap_get_info_delegate(&tc0100scn_base_device::get_bg_tile_info<0x00000, 0>, "bg0_wide", this), TILEMAP_SCAN_ROWS, 8, 8, 128, 64);
	m_tilemap[1][1] = &machine().tilemap().create(*this, tilemap_get_info_delegate(&tc0100scn_base_device::get_bg_tile_info<0x04000, 1>, "bg1_wide", this), TILEMAP_SCAN_ROWS, 8, 8, 128, 64);
	m_tilemap[2][1] = &machine().tilemap().create(*this, tilemap_get_info_delegate(&tc0100scn_base_device::get_tx_tile_info<0x09000, 2>, "txt_wide", this), TILEMAP_SCAN_ROWS, 8, 8, 128, 32);

	m_tilemap[0][0]->set_transparent_pen(0);
	m_tilemap[1][0]->set_transparent_pen(0);
	m_tilemap[2][0]->set_transparent_pen(0);

	m_tilemap[0][1]->set_transparent_pen(0);
	m_tilemap[1][1]->set_transparent_pen(0);
	m_tilemap[2][1]->set_transparent_pen(0);

	/* Standard width tilemaps. I'm setting the optional chip #2
	   7 bits higher and 2 pixels to the left than chip #1 because
	   that's how thundfox wants it. */

	xd = (m_multiscrn_hack == 0) ?  (-m_x_offset) : (-m_x_offset - 2);
	yd = (m_multiscrn_hack == 0) ?  (8 - m_y_offset) : (1 - m_y_offset);

	m_tilemap[0][0]->set_scrolldx(xd - 16, -m_flip_xoffs - xd - 16);
	m_tilemap[0][0]->set_scrolldy(yd,      -m_flip_yoffs - yd);
	m_tilemap[1][0]->set_scrolldx(xd - 16, -m_flip_xoffs - xd - 16);
	m_tilemap[1][0]->set_scrolldy(yd,      -m_flip_yoffs - yd);
	m_tilemap[2][0]->set_scrolldx(xd - 16, -m_flip_text_xoffs - xd - 16 - 7);
	m_tilemap[2][0]->set_scrolldy(yd,      -m_flip_text_yoffs - yd);

	/* Double width tilemaps. We must correct offsets for
	   extra chips, as MAME sees offsets from LHS of whole
	   display not from the edges of individual screens.
	   NB flipscreen tilemap offsets are based on Cameltry */

	xd = -m_x_offset - m_multiscrn_xoffs;
	yd = 8 - m_y_offset;

	m_tilemap[0][1]->set_scrolldx(xd - 16, -m_flip_xoffs - xd - 16);
	m_tilemap[0][1]->set_scrolldy(yd,      -m_flip_yoffs - yd);
	m_tilemap[1][1]->set_scrolldx(xd - 16, -m_flip_xoffs - xd - 16);
	m_tilemap[1][1]->set_scrolldy(yd,      -m_flip_yoffs - yd);
	m_tilemap[2][1]->set_scrolldx(xd - 16, -m_flip_text_xoffs - xd - 16 - 7);
	m_tilemap[2][1]->set_scrolldy(yd,      -m_flip_text_yoffs - yd);

	m_tilemap[0][0]->set_scroll_rows(512);
	m_tilemap[1][0]->set_scroll_rows(512);
	m_tilemap[0][1]->set_scroll_rows(512);
	m_tilemap[1][1]->set_scroll_rows(512);

	m_ram = make_unique_clear<u16[]>(TC0100SCN_RAM_SIZE / 2);

	set_layer_ptrs();

	/* create the char set (gfx will then be updated dynamically from RAM) */
	gfx(0)->set_colorbase(m_col_base);
	set_gfx(1, std::make_unique<gfx_element>(&palette(), charlayout, (u8 *)&m_ram[0x6000 / 2], NATIVE_ENDIAN_VALUE_LE_BE(8,0), 64, m_col_base));
	set_gfx(2, std::make_unique<gfx_element>(&palette(), charlayout, (u8 *)&m_ram[0x11000 / 2], NATIVE_ENDIAN_VALUE_LE_BE(8,0), 64, m_col_base));

	gfx_element *bg_gfx = gfx(0);
	gfx_element *txt0 = gfx(1);
	gfx_element *txt1 = gfx(2);

	if (bg_gfx->granularity() == 2)    /* Yuyugogo, Yesnoj */
		bg_gfx->set_granularity(16);

	txt0->set_granularity(bg_gfx->granularity());
	txt1->set_granularity(bg_gfx->granularity());

	set_colbanks(0, 0, 0);  /* standard values, only Wgp & multiscreen games change them */
									/* we call this here, so that they can be modified at video_start*/

	save_pointer(NAME(m_ram), TC0100SCN_RAM_SIZE / 2);
	save_item(NAME(m_ctrl));
	save_item(NAME(m_dblwidth));
}

void tc0100scn_device::device_start()
{
	switch (m_gfxlayout)
	{
		case TC0100SCN_LAYOUT_DEFAULT:
		default:
			decode_gfx(gfxinfo_default);
			break;
		case TC0100SCN_LAYOUT_1BPP:
			decode_gfx(gfxinfo_1bpp);
			break;
	}
	tc0100scn_base_device::device_start();
}

void tc0620scc_device::device_start()
{
	decode_gfx(gfxinfo_6bpp);
	/* make SCC tile GFX format suitable for gfxdecode */
	gfx_element *gx0 = gfx(0);
	gfx_element *gx1 = gfx(1);

	// allocate memory for the assembled data
	u8 *srcdata = auto_alloc_array(machine(), u8, gx0->elements() * gx0->width() * gx0->height());

	// loop over elements
	u8 *dest = srcdata;
	for (int c = 0; c < gx0->elements(); c++)
	{
		const u16 *c0base = gx0->get_data(c);
		const u16 *c1base = gx1->get_data(c);

		// loop over height
		for (int y = 0; y < gx0->height(); y++)
		{
			const u16 *c0 = c0base;
			const u16 *c1 = c1base;

			for (int x = 0; x < gx0->width(); x++)
				*dest++ = (*c0++ & 0xf) | (*c1++ & 0x30);

			c0base += gx0->rowbytes();
			c1base += gx1->rowbytes();
		}
	}

	gx0->set_raw_layout(srcdata, gx0->width(), gx0->height(), gx0->elements(), 8 * gx0->width(), 8 * gx0->width() * gx0->height());
	gx0->set_granularity(64);
	tc0100scn_base_device::device_start();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tc0100scn_base_device::device_reset()
{
	m_dblwidth = 0;

	for (auto & elem : m_ctrl)
		elem = 0;
}


/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

template<unsigned Offset, unsigned Colbank>
TILE_GET_INFO_MEMBER(tc0100scn_base_device::get_bg_tile_info)
{
	u32 code = m_ram[(2 * tile_index) + 1 + Offset];
	const u16 attr = m_ram[(2 * tile_index) + Offset];
	u16 color = attr & 0xff;

	/* Mahjong Quest (F2 system) inexplicably has a banking feature */
	if (!m_tc0100scn_cb.isnull())
		m_tc0100scn_cb(&code, &color);

	SET_TILE_INFO_MEMBER(0,
			code,
			((color + m_bg_colbank[Colbank]) & 0xff),
			TILE_FLIPYX((attr & 0xc000) >> 14));
}

template<unsigned Offset, unsigned Gfx>
TILE_GET_INFO_MEMBER(tc0100scn_base_device::get_tx_tile_info)
{
	int attr = m_ram[Offset + tile_index];

	SET_TILE_INFO_MEMBER(Gfx,
			attr & 0x00ff,
			((attr & 0x3f00) >> 8) + m_tx_colbank,
			TILE_FLIPYX((attr & 0xc000) >> 14));
}

void tc0100scn_base_device::set_colbanks( int bg0, int bg1, int tx )
{
	m_bg_colbank[0] = bg0;
	m_bg_colbank[1] = bg1;
	m_tx_colbank = tx;
}

void tc0100scn_base_device::tilemap_set_dirty()
{
	if (!m_dirty)
		m_dirty = true;
}

void tc0100scn_base_device::set_layer_ptrs()
{
	if (!m_dblwidth)
	{
		m_bgscroll_ram  = m_ram.get() + 0xc000 /2;
		m_fgscroll_ram  = m_ram.get() + 0xc400 /2;
		m_colscroll_ram = m_ram.get() + 0xe000 /2;
	}
	else
	{
		m_bgscroll_ram  = m_ram.get() + 0x10000 /2;
		m_fgscroll_ram  = m_ram.get() + 0x10400 /2;
		m_colscroll_ram = m_ram.get() + 0x10800 /2;
	}
}

void tc0100scn_base_device::restore_scroll()
{
	int flip;

	m_bgscrollx = -m_ctrl[0];
	m_fgscrollx = -m_ctrl[1];
	m_tilemap[2][0]->set_scrollx(0, -m_ctrl[2]);
	m_tilemap[2][1]->set_scrollx(0, -m_ctrl[2]);

	m_bgscrolly = -m_ctrl[3];
	m_fgscrolly = -m_ctrl[4];
	m_tilemap[2][0]->set_scrolly(0, -m_ctrl[5]);
	m_tilemap[2][1]->set_scrolly(0, -m_ctrl[5]);

	flip = (m_ctrl[7] & 0x01) ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0;
	m_tilemap[0][0]->set_flip(flip);
	m_tilemap[1][0]->set_flip(flip);
	m_tilemap[2][0]->set_flip(flip);
	m_tilemap[0][1]->set_flip(flip);
	m_tilemap[1][1]->set_flip(flip);
	m_tilemap[2][1]->set_flip(flip);
}

//-------------------------------------------------
//  device_post_load - device-specific postload
//-------------------------------------------------

void tc0100scn_base_device::device_post_load()
{
	set_layer_ptrs();
	restore_scroll();

	m_tilemap[0][0]->mark_all_dirty();
	m_tilemap[1][0]->mark_all_dirty();
	m_tilemap[2][0]->mark_all_dirty();
	m_tilemap[0][1]->mark_all_dirty();
	m_tilemap[1][1]->mark_all_dirty();
	m_tilemap[2][1]->mark_all_dirty();
}

u16 tc0100scn_base_device::ram_r(offs_t offset)
{
	return m_ram[offset];
}

void tc0100scn_base_device::ram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_ram[offset]);
	/* Double-width tilemaps have a different memory map */
	if (offset < 0x2000)
	{
		m_tilemap[0][0]->mark_tile_dirty(offset / 2);
		m_tilemap[0][1]->mark_tile_dirty(offset / 2);
	}
	else if (offset < 0x3000)
	{
		m_tilemap[2][0]->mark_tile_dirty((offset & 0x0fff));
		m_tilemap[0][1]->mark_tile_dirty(offset / 2);
	}
	else if (offset < 0x3800)
	{
		gfx(1)->mark_dirty((offset - 0x3000) / 8);
		m_tilemap[0][1]->mark_tile_dirty(offset / 2);
	}
	else if (offset < 0x4000)
		m_tilemap[0][1]->mark_tile_dirty(offset / 2);
	else if (offset < 0x6000)
	{
		m_tilemap[1][0]->mark_tile_dirty((offset & 0x1fff) / 2);
		m_tilemap[1][1]->mark_tile_dirty((offset & 0x3fff) / 2);
	}
	else if (offset < 0x8000)
		m_tilemap[1][1]->mark_tile_dirty((offset & 0x3fff) / 2);
	else if (offset >= 0x8800 && offset < 0x9000)
		gfx(2)->mark_dirty((offset - 0x8800) / 8);
	else if (offset >= 0x9000)
		m_tilemap[2][1]->mark_tile_dirty((offset & 0x0fff));

}

u16 tc0100scn_base_device::ctrl_r(offs_t offset)
{
	return m_ctrl[offset];
}

void tc0100scn_base_device::ctrl_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_ctrl[offset]);

	data = m_ctrl[offset];

	switch (offset)
	{
		case 0x00:
			m_bgscrollx = -data;
			break;

		case 0x01:
			m_fgscrollx = -data;
			break;

		case 0x02:
			m_tilemap[2][0]->set_scrollx(0, -data);
			m_tilemap[2][1]->set_scrollx(0, -data);
			break;

		case 0x03:
			m_bgscrolly = -data;
			break;

		case 0x04:
			m_fgscrolly = -data;
			break;

		case 0x05:
			m_tilemap[2][0]->set_scrolly(0, -data);
			m_tilemap[2][1]->set_scrolly(0, -data);
			break;

		case 0x06:
		{
			int old_width = m_dblwidth;
			m_dblwidth = (data & 0x10) >> 4;

			if (m_dblwidth != old_width)   /* tilemap width is changing */
			{
				/* Reinitialise layer pointers */
				set_layer_ptrs();
			}

			break;
		}

		case 0x07:
		{
			int flip = (data & 0x01) ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0;

			m_tilemap[0][0]->set_flip(flip);
			m_tilemap[1][0]->set_flip(flip);
			m_tilemap[2][0]->set_flip(flip);
			m_tilemap[0][1]->set_flip(flip);
			m_tilemap[1][1]->set_flip(flip);
			m_tilemap[2][1]->set_flip(flip);

			break;
		}
	}
}


void tc0100scn_base_device::tilemap_update()
{
	if (m_dirty)
	{
		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				m_tilemap[i][j]->mark_all_dirty();
			}
		}
		m_dirty = false;
	}

	int j;

	m_tilemap[0][m_dblwidth]->set_scrolly(0, m_bgscrolly);
	m_tilemap[1][m_dblwidth]->set_scrolly(0, m_fgscrolly);

	for (j = 0; j < 256; j++)
		m_tilemap[0][m_dblwidth]->set_scrollx((j + m_bgscrolly) & 0x1ff, m_bgscrollx - m_bgscroll_ram[j]);
	for (j = 0; j < 256; j++)
		m_tilemap[1][m_dblwidth]->set_scrollx((j + m_fgscrolly) & 0x1ff, m_fgscrollx - m_fgscroll_ram[j]);
}

void tc0100scn_base_device::tilemap_draw_fg( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, tilemap_t* tmap, int flags, u8 priority, u8 pmask )
{
	const bitmap_ind16 &src_bitmap = tmap->pixmap();
	int width_mask, height_mask, x, y, p;
	int column_offset, src_x = 0, src_y = 0;
	int scrollx_delta = - tmap->scrolldx();
	int scrolly_delta = - tmap->scrolldy();

	width_mask = src_bitmap.width() - 1;
	height_mask = src_bitmap.height() - 1;

	src_y = (m_fgscrolly + scrolly_delta) & height_mask;
	if (m_ctrl[0x7] & 1) // Flipscreen
		src_y = (256 - src_y) & height_mask;

	//We use cliprect.max_y and cliprect.max_x to support games which use more than 1 screen

	src_y += cliprect.top();
	// Row offsets are 'screen space' 0-255 regardless of Y scroll
	for (y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		src_x = (m_fgscrollx - m_fgscroll_ram[(y + scrolly_delta) & 0x1ff] + scrollx_delta + cliprect.min_x) & width_mask;
		if (m_ctrl[0x7] & 1) // Flipscreen
			src_x = (256 - 64 - src_x) & width_mask;

		// Col offsets are 'tilemap' space 0-511, and apply to blocks of 8 pixels at once
		for (x = 0; x < cliprect.width(); x++)
		{
			column_offset = m_colscroll_ram[(src_x & 0x3ff) / 8];
			p = src_bitmap.pix16((src_y - column_offset) & height_mask, src_x);

			if ((p & 0xf)!= 0 || (flags & TILEMAP_DRAW_OPAQUE))
			{
				bitmap.pix16(y, x + cliprect.min_x) = p;
				if (screen.priority().valid())
				{
					u8 *pri = &screen.priority().pix8(y);
					pri[x + cliprect.min_x] = (pri[x + cliprect.min_x] & pmask) | priority;
				}
			}
			src_x = (src_x + 1) & width_mask;
		}
		src_y = (src_y + 1) & height_mask;
	}
}

int tc0100scn_base_device::tilemap_draw( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, u8 priority, u8 pmask )
{
	int disable = m_ctrl[6] & 0xf7;
	rectangle clip = cliprect;
	clip &= screen.visible_area();

#if 0
if (disable != 0 && disable != 3 && disable != 7)
	popmessage("layer disable = %x",disable);
#endif

	switch (layer)
	{
		case 0:
			if (disable & 0x01)
				return 1;
			m_tilemap[0][m_dblwidth]->draw(screen, bitmap, clip, flags, priority, pmask);
			break;
		case 1:
			if (disable & 0x02)
				return 1;
			tilemap_draw_fg(screen, bitmap, clip, m_tilemap[1][m_dblwidth], flags, priority, pmask);
			break;
		case 2:
			if (disable & 0x04)
				return 1;
			m_tilemap[2][m_dblwidth]->draw(screen, bitmap, clip, flags, priority, pmask);
			break;
	}
	return 0;
}

int tc0100scn_base_device::bottomlayer()
{
	return (m_ctrl[6] & 0x8) >> 3;
}
