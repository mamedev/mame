// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*
Taito tc0080vco
---------
Combined tilemap and motion object generator. The front tilemap
fetches 3bpp gfx data from ram and only has 8 colors available.
The other tilemaps use ROMs as usual. The same gfx set is used
for both tilemaps and motion objects.

There are two 64x64 tilemaps with 16x16 tiles; the optional
front tilemap is 64x64 with 8x8 tiles.

00000-00fff gfx data for FG0        (lo 2 bits per pixel)
01000-01fff FG0 (64x64)             (two tilenums per word, no color bits)
02000-0bfff chain ram               (sprite tile mapping/colors)
0c000-0dfff BG0 tile numbers (64x64)
0e000-0ffff BG1 tile numbers (64x64)
10000-10fff gfx data for FG0        (hi bit per pixel: Ainferno proves not 4bpp)
11000-11fff unknown / unused ?
12000-1bfff chain ram               (sprite tile mapping/colors)
1c000-1dfff BG0 color / flip bits (64x64)
1e000-1ffff BG1 color / flip bits (64x64)
20000-203ff BG0 rowscroll           (see Dleague title screen *)
20400-207ff spriteram
20800-2080f control registers

[*only used in Dleague AFAIK. Note 0x200 words is not enough for a
64x16 => 0x400 pixel high tilemap. So probably it wraps around and
each rowscroll word affects 2 separate lines. Tacky, but wouldn't be
noticeable unless y zoom more than halved the apparent pixel height
meaning you could see more than half of the total tilemap...]

[There is an oddity with this chip: FG0 areas can be addressed
as chain ram, since the offsets used in spriteram are from the
start of the tc0080vco address space - not the start of chain ram.
In practice only Dleague seems to do this, for c.10 frames as the
pitcher bowls, and I think it's a coding error. Log it and see.]

[Parentj, Ainferno and Syvalion are only games using FG0 layer.]

Control registers

000-001 ----xx---------- screen invert
        ------xx-------- unknown (always set)
        ---------------- any others ???

002-003 BG0 scroll X  (0x3ff is the tilemap span)
004-005 BG1 scroll X
006-007 BG0 scroll Y  (0x3ff is the tilemap span)
008-009 BG1 scroll Y
00a-00b unknown (Syvalion - FG0 scroll? - and Recordbr)
00c-00d BG0 zoom (hi byte=X, lo byte=Y *)
00e-00f BG1 zoom (hi byte=X, lo byte=Y *)

[* X zoom normal=0x3f   Y zoom normal=0x7f]

All we know is that as y zoom gets bigger the magnification grows:
this seems to be the only zoom feature actually used in the games.

*/

#include "emu.h"
#include "tc0080vco.h"
#include "taito_helper.h"

#include "drawgfxt.ipp"
#include "screen.h"


#define TC0080VCO_RAM_SIZE 0x21000
#define TC0080VCO_CHAR_RAM_SIZE 0x2000
#define TC0080VCO_TOTAL_CHARS 256


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/


DEFINE_DEVICE_TYPE(TC0080VCO, tc0080vco_device, "tc0080vco", "Taito TC0080VCO")

tc0080vco_device::tc0080vco_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, TC0080VCO, tag, owner, clock)
	, device_gfx_interface(mconfig, *this, nullptr)
	, m_ram(nullptr)
	, m_bg0_ram_0(nullptr)
	, m_bg0_ram_1(nullptr)
	, m_bg1_ram_0(nullptr)
	, m_bg1_ram_1(nullptr)
	, m_tx_ram_0(nullptr)
	, m_tx_ram_1(nullptr)
	, m_char_ram(nullptr)
	, m_bgscroll_ram(nullptr)
	, m_chain_ram_0(nullptr)
	, m_chain_ram_1(nullptr)
	, m_spriteram(nullptr)
	, m_scroll_ram(nullptr)
	, m_bg0_scrollx(0)
	, m_bg0_scrolly(0)
	, m_bg1_scrollx(0)
	, m_bg1_scrolly(0)
	, m_flipscreen(0)
	, m_bg_xoffs(0)
	, m_bg_yoffs(0)
	, m_bg_flip_yoffs(0)
	, m_has_fg0(1)
	, m_colbase(0)
	, m_gfx_region(*this, DEVICE_SELF)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tc0080vco_device::device_start()
{
	/* Is this endian-correct ??? */

	#define XOR(a) WORD_XOR_BE(a)

	gfx_layout tilelayout =
	{
		16,16,  /* 16x16 pixels */
		0,
		4,
		{ STEP4(0,1) },
		{ STEP16(15*4, -4) },
		{ STEP16(0, 16*4) },
		16*16*4
	};
	tilelayout.total = m_gfx_region->bytes() / ((16*16*4) / 8);

	static const gfx_layout charlayout =
	{
		8, 8,   /* 8x8 pixels */
		256,    /* 256 chars */
		3,      /* 3 bits per pixel */
		{ 0x10000*8 + XOR(2)*4, XOR(0)*4, XOR(2)*4 },
		{ STEP8(0,1) },
		{ STEP8(0,8*2) },
		16*8
	};

	m_tilemap[0] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(tc0080vco_device::get_bg0_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_tilemap[1] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(tc0080vco_device::get_bg1_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);

//	m_tilemap[0]->set_transparent_pen(0);
//	m_tilemap[1]->set_transparent_pen(0);

	// ainferno expects upper color bank to be transparent for hud and landing monitor to display properly.
	// This is drawn with bit 15, is it expecting some kind of shadowing/blending? The PCB refs doesn't seem to show anything of the like.
	// The only other place that sets upper color bank is tetristh, and doesn't seem to have any real effect.
	// TODO: this arrangement may be controlled by m_scroll_ram[0] bits 7 and 6, which is either b11 or b00.
	m_tilemap[0]->set_transmask(0, 0x0001, 0xfffe);
	m_tilemap[0]->set_transmask(1, 0x8001, 0x7ffe);
	m_tilemap[1]->set_transmask(0, 0x0001, 0xfffe);
	m_tilemap[1]->set_transmask(1, 0x8001, 0x7ffe);
	m_tilemap[0]->set_scrolldx(m_bg_xoffs, 512);
	m_tilemap[1]->set_scrolldx(m_bg_xoffs, 512);
	m_tilemap[0]->set_scrolldy(m_bg_yoffs, m_bg_flip_yoffs);
	m_tilemap[1]->set_scrolldy(m_bg_yoffs, m_bg_flip_yoffs);

	/* bg0 tilemap scrollable per pixel row */
	m_tilemap[0]->set_scroll_rows(512);

	/* Perform extra initialisations for text layer */
	m_tilemap[2] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(tc0080vco_device::get_tx_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	m_tilemap[2]->set_scrolldx(0, 0);
	m_tilemap[2]->set_scrolldy(48, -448);

	m_tilemap[2]->set_transparent_pen(0);

	m_ram = make_unique_clear<u16[]>(TC0080VCO_RAM_SIZE / 2);

	m_char_ram      = m_ram.get() + 0x00000 / 2;    /* continues at +0x10000 */
	m_tx_ram_0      = m_ram.get() + 0x01000 / 2;
	m_chain_ram_0   = m_ram.get() + 0x00000 / 2;    /* only used from +0x2000 */

	m_bg0_ram_0     = m_ram.get() + 0x0c000 / 2;
	m_bg1_ram_0     = m_ram.get() + 0x0e000 / 2;

	m_tx_ram_1      = m_ram.get() + 0x11000 / 2;
	m_chain_ram_1   = m_ram.get() + 0x10000 / 2;    /* only used from +0x12000 */

	m_bg0_ram_1     = m_ram.get() + 0x1c000 / 2;
	m_bg1_ram_1     = m_ram.get() + 0x1e000 / 2;
	m_bgscroll_ram  = m_ram.get() + 0x20000 / 2;
	m_spriteram     = m_ram.get() + 0x20400 / 2;
	m_scroll_ram    = m_ram.get() + 0x20800 / 2;

	/* create the char set (gfx will then be updated dynamically from RAM) */
	set_gfx(0, std::make_unique<gfx_element>(&palette(), tilelayout, m_gfx_region->base(), 0, 0x20, m_colbase));
	set_gfx(1, std::make_unique<gfx_element>(&palette(), charlayout, (u8 *)m_char_ram,     0, 0x01, m_colbase + 512));

	save_pointer(NAME(m_ram), TC0080VCO_RAM_SIZE / 2);
}


/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

static const int tc0080vco_zoomy_conv_table[] =
{
/*      These are hand-tuned values...      */
/*    +0   +1   +2   +3   +4   +5   +6   +7    +8   +9   +a   +b   +c   +d   +e   +f */
	0x00,0x01,0x01,0x02,0x02,0x03,0x04,0x05, 0x06,0x06,0x07,0x08,0x09,0x0a,0x0a,0x0b,   /* 0x00 */
	0x0b,0x0c,0x0c,0x0d,0x0e,0x0e,0x0f,0x10, 0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x16,
	0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e, 0x1f,0x20,0x21,0x22,0x24,0x25,0x26,0x27,
	0x28,0x2a,0x2b,0x2c,0x2e,0x30,0x31,0x32, 0x34,0x36,0x37,0x38,0x3a,0x3c,0x3e,0x3f,

	0x40,0x41,0x42,0x42,0x43,0x43,0x44,0x44, 0x45,0x45,0x46,0x46,0x47,0x47,0x48,0x49,   /* 0x40 */
	0x4a,0x4a,0x4b,0x4b,0x4c,0x4d,0x4e,0x4f, 0x4f,0x50,0x51,0x51,0x52,0x53,0x54,0x55,
	0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d, 0x5e,0x5f,0x60,0x61,0x62,0x63,0x64,0x66,
	0x67,0x68,0x6a,0x6b,0x6c,0x6e,0x6f,0x71, 0x72,0x74,0x76,0x78,0x80,0x7b,0x7d,0x7f
};

/* Y chain size is 16/32?/64/64? pixels. X chain size
   is always 64 pixels. */

static const u8 size[] = { 1, 2, 4, 4 };

void tc0080vco_device::get_sprite_params(int offs, bool zoomy_enable)
{
	m_sprite.tile_offs = (m_spriteram[offs + 3] & 0x1fff) << 2;
	if (m_sprite.tile_offs)
	{
		m_sprite.x0    =  m_spriteram[offs + 1] & 0x03ff;
		m_sprite.y0    =  m_spriteram[offs + 0] & 0x03ff;
		m_sprite.zoomx = (m_spriteram[offs + 2] & 0x7f00) >> 8;
		/* Convert zoomy value to real value as zoomx */
		m_sprite.zoomy = zoomy_enable ? tc0080vco_zoomy_conv_table[m_spriteram[offs + 2] & 0x007f] : m_sprite.zoomx;
		m_sprite.ysize = size[(m_spriteram[offs + 0] & 0x0c00) >> 10];

		/* The increasing ratio of expansion is different whether zoom value */
		/* is less or more than 63.                                          */
		if (m_sprite.zoomx < 63)
		{
			m_sprite.dx = 8 + (m_sprite.zoomx + 2) / 8;
			m_sprite.ex = (m_sprite.zoomx + 2) % 8;
			m_sprite.zx = ((m_sprite.dx << 1) + m_sprite.ex) << 11;
		}
		else
		{
			m_sprite.dx = 16 + (m_sprite.zoomx - 63) / 4;
			m_sprite.ex = (m_sprite.zoomx - 63) % 4;
			m_sprite.zx = (m_sprite.dx + m_sprite.ex) << 12;
		}

		if (m_sprite.zoomx == m_sprite.zoomy)
		{
			m_sprite.dy = m_sprite.dx;
			m_sprite.ey = m_sprite.ex;
			m_sprite.zy = m_sprite.zx;
		}
		else
		{
			if (m_sprite.zoomy < 63)
			{
				m_sprite.dy = 8 + (m_sprite.zoomy + 2) / 8;
				m_sprite.ey = (m_sprite.zoomy + 2) % 8;
				m_sprite.zy = ((m_sprite.dy << 1) + m_sprite.ey) << 11;
			}
			else
			{
				m_sprite.dy = 16 + (m_sprite.zoomy - 63) / 4;
				m_sprite.ey = (m_sprite.zoomy - 63) % 4;
				m_sprite.zy = (m_sprite.dy + m_sprite.ey) << 12;
			}
		}

		if (m_sprite.x0 >= 0x200) m_sprite.x0 -= 0x400;
		if (m_sprite.y0 >= 0x200) m_sprite.y0 -= 0x400;

		if (m_flipscreen)
		{
			m_sprite.x0 = 497 - m_sprite.x0;
			m_sprite.y0 = 498 - m_sprite.y0;
			m_sprite.dx = -m_sprite.dx;
			m_sprite.dy = -m_sprite.dy;
		}
		else
		{
			m_sprite.x0 += 1;
			m_sprite.y0 += 2;
		}
	}
}

void tc0080vco_device::draw_single_sprite(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u32 tile_offs = m_sprite.tile_offs;
	int y = m_sprite.y0;
	for (int j = 0; j < m_sprite.ysize; j++)
	{
		int x = m_sprite.x0;
		for (int k = 0; k < 4; k++)
		{
			if (tile_offs >= 0x1000)    /* or dleague pitcher gets blanked */
			{
				const u32 tile  = m_chain_ram_0[tile_offs] & 0x7fff;
				const u32 color = m_chain_ram_1[tile_offs] & 0x001f;
				int flipx       = m_chain_ram_1[tile_offs] & 0x0040;
				int flipy       = m_chain_ram_1[tile_offs] & 0x0080;

				if (m_flipscreen)
				{
					flipx ^= 0x0040;
					flipy ^= 0x0080;
				}

				gfx(0)->zoom_transpen(bitmap,cliprect,
							tile,
							color,
							flipx, flipy,
							x, y,
							m_sprite.zx, m_sprite.zy, 0);
			}
			tile_offs++;
			x += m_sprite.dx;
		}
		y += m_sprite.dy;
	}
}

TILE_GET_INFO_MEMBER(tc0080vco_device::get_bg0_tile_info)
{
	int color, tile;

	color = m_bg0_ram_1[tile_index] & 0x001f;
	tile  = m_bg0_ram_0[tile_index] & 0x7fff;

	tileinfo.group = color >> 4;

	tileinfo.set(0,
			tile,
			color,
			TILE_FLIPYX((m_bg0_ram_1[tile_index] & 0x00c0) >> 6));
}

TILE_GET_INFO_MEMBER(tc0080vco_device::get_bg1_tile_info)
{
	int color, tile;

	color = m_bg1_ram_1[tile_index] & 0x001f;
	tile  = m_bg1_ram_0[tile_index] & 0x7fff;

	tileinfo.group = color >> 4;

	tileinfo.set(0,
			tile,
			color,
			TILE_FLIPYX((m_bg1_ram_1[tile_index] & 0x00c0) >> 6));
}

TILE_GET_INFO_MEMBER(tc0080vco_device::get_tx_tile_info)
{
	int tile;

	if (!m_flipscreen)
	{
		if ((tile_index & 1))
			tile = (m_tx_ram_0[tile_index >> 1] & 0x00ff);
		else
			tile = (m_tx_ram_0[tile_index >> 1] & 0xff00) >> 8;
		tileinfo.category = 0;
	}
	else
	{
		if ((tile_index & 1))
			tile = (m_tx_ram_0[tile_index >> 1] & 0xff00) >> 8;
		else
			tile = (m_tx_ram_0[tile_index >> 1] & 0x00ff);
		tileinfo.category = 0;
	}

	tileinfo.set(1,
			tile,
			0,
			0);
}


void tc0080vco_device::scrollram_w(offs_t offset, u16 data)
{
	switch (offset)
	{
		case 0x00:          /* screen invert control */
			m_flipscreen = m_scroll_ram[0] & 0x0c00;

			m_tilemap[0]->set_flip(m_flipscreen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
			m_tilemap[1]->set_flip(m_flipscreen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
			m_tilemap[2]->set_flip(m_flipscreen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);

			m_bg0_scrollx = m_scroll_ram[1] & 0x03ff;
			m_bg1_scrollx = m_scroll_ram[2] & 0x03ff;
			m_bg0_scrolly = m_scroll_ram[3] & 0x03ff;
			m_bg1_scrolly = m_scroll_ram[4] & 0x03ff;
			break;

		case 0x01:          /* BG0 scroll X */
			m_bg0_scrollx = data & 0x03ff;
			break;

		case 0x02:          /* BG1 scroll X */
			m_bg1_scrollx = data & 0x03ff;
			break;

		case 0x03:          /* BG0 scroll Y */
			m_bg0_scrolly = data & 0x03ff;
			break;

		case 0x04:          /* BG1 scroll Y */
			m_bg1_scrolly = data & 0x03ff;
			break;

		default:
			break;
	}
}

u16 tc0080vco_device::word_r(offs_t offset)
{
	return m_ram[offset];
}

void tc0080vco_device::word_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_ram[offset]);

	/* A lot of tc0080vco writes require no action... */

	if (offset < 0x1000 / 2)
	{
		gfx(1)->mark_dirty(offset / 8);
#if 0
		if (!m_has_fg0)
		{
			if (m_ram[offset])
			popmessage("Write non-zero to tc0080vco char ram\nPlease report to MAMEDEV");
		}
#endif
	}
	else if (offset < 0x2000 / 2)   /* fg0 (text layer) */
	{
		m_tilemap[2]->mark_tile_dirty((offset & 0x07ff) * 2);
		m_tilemap[2]->mark_tile_dirty((offset & 0x07ff) * 2 + 1);
#if 0
		if (!m_has_fg0)
		{
			if (m_ram[offset])
			popmessage("Write non-zero to tc0080vco fg0\nPlease report to MAMEDEV");
		}
#endif
	}
	else if (offset < 0xc000 / 2)   /* chain ram */
	{}
	else if (offset < 0xe000 / 2)   /* bg0 (0) */
		m_tilemap[0]->mark_tile_dirty((offset & 0xfff));

	else if (offset < 0x10000 / 2)  /* bg1 (0) */
		m_tilemap[1]->mark_tile_dirty((offset & 0xfff));

	else if (offset < 0x11000 / 2)
	{
		gfx(1)->mark_dirty((offset - 0x10000 / 2) / 8);
#if 0
		if (!m_has_fg0)
		{
			if (m_ram[offset])
			popmessage("Write non-zero to tc0080vco char-hi ram\nPlease report to MAMEDEV");
		}
#endif
	}
	else if (offset < 0x12000 / 2)  /* unknown/unused */
	{
#if 1
		if (m_ram[offset])
		popmessage("Write non-zero to mystery tc0080vco area\nPlease report to MAMEDEV");
#endif
	}
	else if (offset < 0x1c000 / 2)  /* chain ram */
	{}
	else if (offset < 0x1e000 / 2)  /* bg0 (1) */
		m_tilemap[0]->mark_tile_dirty((offset & 0xfff));

	else if (offset < 0x20000 / 2)  /* bg1 (1) */
		m_tilemap[1]->mark_tile_dirty((offset & 0xfff));

	else if (offset < 0x20400 / 2)  /* bg0 rowscroll */
	{}
	else if (offset < 0x20800 / 2)  /* sprite ram */
	{}
	else if (offset < 0x20fff / 2)
		scrollram_w(offset - (0x20800 / 2), m_ram[offset]);
}

void tc0080vco_device::tilemap_update( )
{
	if (!m_flipscreen)
	{
		for (int j = 0; j < 0x400; j++)
			m_tilemap[0]->set_scrollx((j + 0) & 0x3ff, -m_bg0_scrollx - m_bgscroll_ram[j & 0x1ff]);
	}
	else
	{
		for (int j = 0; j < 0x400; j++)
			m_tilemap[0]->set_scrollx((j + 0) & 0x3ff, -m_bg0_scrollx + m_bgscroll_ram[j & 0x1ff]);
	}

	m_tilemap[0]->set_scrolly(0,  m_bg0_scrolly);
	m_tilemap[1]->set_scrollx(0, -m_bg1_scrollx);
	m_tilemap[1]->set_scrolly(0,  m_bg1_scrolly);
	m_tilemap[2]->set_scrollx(0, 0);   /* no scroll (maybe) */
	m_tilemap[2]->set_scrolly(0, 0);
}


/* NB: orientation_flipx code in following routine has not been tested */

void tc0080vco_device::bg0_tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flags, u8 priority, u8 pmask)
{
	const u16 zoom = m_scroll_ram[6];

	int zx = (zoom & 0xff00) >> 8;
	int zy = zoom & 0x00ff;

	if (zx == 0x3f && zy == 0x7f)       /* normal size */
	{
		m_tilemap[0]->draw(screen, bitmap, cliprect, flags, priority, pmask);
	}
	else        /* zoom + rowscroll = custom draw routine */
	{
		u16 scanline[512];
		bitmap_ind16 &srcbitmap = m_tilemap[0]->pixmap();
		bitmap_ind8 &flagsbitmap = m_tilemap[0]->flagsmap();

		int sx, zoomx, zoomy;
		int dx, ex, dy, ey;
		int y_index;

		const bool flip = m_flipscreen;

		const int min_x = cliprect.min_x;
		const int max_x = cliprect.max_x;
		const int min_y = cliprect.min_y;
		const int max_y = cliprect.max_y;
		const int screen_width = max_x + 1;
		const int width_mask = 0x3ff; /* underlying tilemap */


#if 0
{
	char buf[100];
	sprintf(buf, "xmin= %04x xmax= %04x ymin= %04x ymax= %04x", min_x, max_x, min_y, max_y);
	popmessage(buf);
}
#endif

		if (zx < 63)
		{
			/* no evidence for these calcs? */
			dx = 16 - (zx + 2) / 8;
			ex = (zx + 2) % 8;
			zoomx = ((dx << 3) - ex) << 10;
		}
		else
		{
			/* 256 is speculative, haven't found a game using zoomx yet */
			zoomx = 0x10000 - ((zx - 0x3f) * 256);
		}

		if (zy < 127)
		{
			/* no evidence for these calcs? */
			dy = 16 - (zy + 2) / 16;
			ey = (zy + 2) % 16;
			zoomy = ((dy << 4) - ey) << 9;
		}
		else
		{
			/* confirmed with real board */
			zoomy = 0x10000 - ((zy - 0x7f) * 512);
		}

		if (!flip)
		{
			sx = (-m_scroll_ram[1] - 1) << 16;
			y_index = (( m_scroll_ram[3] - 1) << 16) + min_y * zoomy;
		}
		else
		{
			/* adjustment for zx is entirely speculative */
			sx =  (( 0x200 + m_scroll_ram[1]) << 16) - (max_x + min_x) * (zoomx - 0x10000);

			/* 0x130 correct for Dleague. Syvalion correct with 0x1f0. min_y is 0x20 and 0x30; max_y is 0x10f and 0x1bf; max_y + min_y seems a good bet... */
			y_index = ((-m_scroll_ram[3] - 2) << 16) + min_y * zoomy - (max_y + min_y) * (zoomy - 0x10000);
		}

		for (int y = min_y; y <= max_y; y++)
		{
			int src_y_index = (y_index >> 16) & 0x3ff;  /* tilemaps are 1024 px up/down */

			/* row areas are the same in flipscreen, so we must read in reverse */
			int row_index = (src_y_index & 0x1ff);
			if (flip)
				row_index = 0x1ff - row_index;

			int x_index = sx - ((m_bgscroll_ram[row_index] << 16));

			u16 const *src16 = &srcbitmap.pix(src_y_index);
			u8 const *tsrc = &flagsbitmap.pix(src_y_index);
			u16 *dst16 = scanline;

			int x_step = zoomx;

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

			taitoic_drawscanline(bitmap, cliprect, 0, y, scanline, (flags & TILEMAP_DRAW_OPAQUE) ? false : true , ROT0, screen.priority(), priority, pmask);

			y_index += zoomy;
		}
	}
}


#define PIXEL_OP_COPY_TRANS0_SET_PRIORITY(DEST, PRIORITY, SOURCE)                   \
do                                                                                  \
{                                                                                   \
	u32 srcdata = (SOURCE);                                                         \
	if (srcdata != 0)                                                               \
	{                                                                               \
		(DEST) = SOURCE;                                                            \
		(PRIORITY) = ((PRIORITY) & primask) | privalue;                             \
	}                                                                               \
}                                                                                   \
while (0)
void tc0080vco_device::bg1_tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flags, u8 priority, u8 pmask)
{
	u8 layer = 1;
	const u16 zoom = m_scroll_ram[6 + layer];
	const int min_x = cliprect.min_x;
	const int max_x = cliprect.max_x;
	const int min_y = cliprect.min_y;
	const int max_y = cliprect.max_y;

	int zoomx = (zoom & 0xff00) >> 8;
	int zoomy =  zoom & 0x00ff;

	if (zoomx == 0x3f && zoomy == 0x7f)     /* normal size */
	{
		m_tilemap[layer]->draw(screen, bitmap, cliprect, flags, priority, pmask);
	}
	else        /* zoomed */
	{
		int zx, zy, dx, dy, ex, ey;
		int sx,sy;

		/* shouldn't we set no_clip before doing this (see TC0480SCP) ? */
		bitmap_ind16 &srcbitmap = m_tilemap[layer]->pixmap();

		if (zoomx < 63)
		{
			/* no evidence for these calcs? */
			dx = 16 - (zoomx + 2) / 8;
			ex = (zoomx + 2) % 8;
			zx = ((dx << 3) - ex) << 10;
		}
		else
		{
			/* 256 is speculative, haven't found a game using zoomx yet */
			zx = 0x10000 - ((zoomx - 0x3f) * 256);
		}

		if (zoomy < 127)
		{
			/* no evidence for these calcs? */
			dy = 16 - (zoomy + 2) / 16;
			ey = (zoomy + 2) % 16;
			zy = ((dy << 4) - ey) << 9;
		}
		else
		{
			/* confirmed with real board */
			zy = 0x10000 - ((zoomy - 0x7f) * 512);
		}

		if (!m_flipscreen)
		{
			sx = (-m_scroll_ram[layer + 1] - 1) << 16;
			sy = ( m_scroll_ram[layer + 3] - 1) << 16;
		}
		else
		{
			/* adjustment for zx is entirely speculative */
			sx =  (( 0x200 + m_scroll_ram[layer + 1]) << 16) - (max_x + min_x) * (zx - 0x10000);
			sy =  (( 0x3fe - m_scroll_ram[layer + 3]) << 16) - (max_y + min_y) * (zy - 0x10000);
		}

		copyrozbitmap_core(bitmap, cliprect, srcbitmap, sx, sy, zx, 0, 0, zy, false, screen.priority(), [privalue = priority, primask = pmask](u16 &destp, u8 &pri, const u16 &srcp) { PIXEL_OP_COPY_TRANS0_SET_PRIORITY(destp, pri, srcp); });
	}
}


void tc0080vco_device::tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, u8 priority, u8 pmask)
{
	int disable = 0x00; /* possibly layer disable bits do exist ?? */

#if 0
	popmessage("layer disable = %x", disable);
#endif

	switch (layer)
	{
		case 0:
			if (disable & 0x01)
				return;
			bg0_tilemap_draw(screen, bitmap, cliprect, flags, priority, pmask);
			break;
		case 1:
			if (disable & 0x02)
				return;
			bg1_tilemap_draw(screen, bitmap, cliprect, flags, priority, pmask);
			break;
		case 2:
			if (disable & 0x04)
				return;
			m_tilemap[2]->draw(screen, bitmap, cliprect, flags, priority, pmask);
			break;
	}
}

/* FIXME: maybe it would be better to provide pointers to these RAM regions
which can be accessed directly by the drivers... */
u16 tc0080vco_device::cram_0_r(int offset)
{
	return m_chain_ram_0[offset];
}

u16 tc0080vco_device::cram_1_r(int offset)
{
	return m_chain_ram_1[offset];
}

u16 tc0080vco_device::sprram_r(int offset)
{
	return m_spriteram[offset];
}

u16 tc0080vco_device::scrram_r(int offset)
{
	return m_scroll_ram[offset];
}

int tc0080vco_device::flipscreen_r()
{
	return m_flipscreen;
}

//-------------------------------------------------
//  device_post_load - device-specific postload
//-------------------------------------------------

void tc0080vco_device::device_post_load()
{
	m_flipscreen = m_scroll_ram[0] & 0x0c00;

	m_tilemap[0]->set_flip(m_flipscreen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
	m_tilemap[1]->set_flip(m_flipscreen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
	m_tilemap[2]->set_flip(m_flipscreen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);

	m_bg0_scrollx = m_scroll_ram[1] & 0x03ff;
	m_bg1_scrollx = m_scroll_ram[2] & 0x03ff;
	m_bg0_scrolly = m_scroll_ram[3] & 0x03ff;
	m_bg1_scrolly = m_scroll_ram[4] & 0x03ff;
}
