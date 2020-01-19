// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "spg110_video.h"

DEFINE_DEVICE_TYPE(SPG110_VIDEO, spg110_video_device, "spg110_video", "SPG110 System-on-a-Chip (Video)")

spg110_video_device::spg110_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_space_config("spg110_video", ENDIANNESS_BIG, 16, 32, 0, address_map_constructor(FUNC(spg110_video_device::map_video), this))
	, m_cpu(*this, finder_base::DUMMY_TAG)
	, m_screen(*this, finder_base::DUMMY_TAG)
	, m_palette(*this, "palette")
	, m_gfxdecode(*this, "gfxdecode")
	, m_palram(*this, "palram")
	, m_palctrlram(*this, "palctrlram")
	, m_sprtileno(*this, "sprtileno")
	, m_sprattr1(*this, "sprattr1")
	, m_sprattr2(*this, "sprattr2")
	, m_video_irq_cb(*this)
{
}

spg110_video_device::spg110_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spg110_video_device(mconfig, SPG110_VIDEO, tag, owner, clock)
{
}

template<spg110_video_device::flipx_t FlipX>
void spg110_video_device::draw(const rectangle &cliprect, uint32_t line, uint32_t xoff, uint32_t yoff, uint32_t ctrl, uint32_t bitmap_addr, uint16_t tile, uint8_t yflipmask, uint8_t pal, int32_t h, int32_t w, uint8_t bpp)
{
	address_space &space = m_cpu->space(AS_PROGRAM);

	uint32_t nc = (bpp + 1) << 1;

	switch (bpp)
	{
	case 0x03: pal = 0; break; // 8 bpp
	case 0x02: pal &=0x03; break; // 6 bpp
	case 0x01: break; // 4 bpp
	case 0x00: break; // 2 bpp
	}

	uint32_t palette_offset = pal;

	palette_offset <<= nc;

	uint32_t bits_per_row = nc * w / 16;
	uint32_t words_per_tile = bits_per_row * h;
	uint32_t m = bitmap_addr + words_per_tile * tile + bits_per_row * (line ^ yflipmask);
	uint32_t bits = 0;
	uint32_t nbits = 0;
	uint32_t y = line;

	int yy = (yoff + y) & 0x1ff;
	if (yy >= 0x01c0)
		yy -= 0x0200;

	if (yy > 240 || yy < 0)
		return;

	int y_index = yy * 320;

	for (int32_t x = FlipX ? (w - 1) : 0; FlipX ? x >= 0 : x < w; FlipX ? x-- : x++)
	{
		int xx = xoff + x;

		bits <<= nc;

		if (nbits < nc)
		{
			uint16_t b = space.read_word(m++ & 0x3fffff);
			//b = (b << 8) | (b >> 8);
			bits |= b << (nc - nbits);
			nbits += 16;
		}
		nbits -= nc;

		uint32_t pal = palette_offset + (bits >> 16);
		bits &= 0xffff;

		xx &= 0x01ff;
		if (xx >= 0x01c0)
			xx -= 0x0200;

		if (xx >= 0 && xx < 320)
		{
			int pix_index = xx + y_index;
			const pen_t *pens = m_palette->pens();
			uint32_t paldata = pens[pal];

			int transmap = m_palctrlram[(pal & 0xf0)>>4];

			bool trans = false;

			if (transmap & 0x10) // maybe values other than 0x010 have other meanings, like blending?
				if ((pal & 0x0f) == (transmap & 0xf))
					trans = true;

			if (!trans)
			{
				m_screenbuf[pix_index] = paldata;
			}
		}
	}
}

void spg110_video_device::draw_page(const rectangle &cliprect, uint32_t scanline, int priority, uint32_t bitmap_addr, uint16_t *regs)
{
	uint32_t xscroll = regs[0];
	uint32_t yscroll = regs[1];
	uint32_t attr = regs[2];
	uint32_t ctrl = regs[3];
	uint32_t tilemap = regs[4];
	uint32_t palette_map = regs[5];
	address_space &space2 = this->space(0);

	if (!(ctrl & PAGE_ENABLE_MASK))
	{
		return;
	}

//  if (((attr & PAGE_PRIORITY_FLAG_MASK) >> PAGE_PRIORITY_FLAG_SHIFT) != priority)
//  {
//      return;
//  }

	uint8_t bpp = attr & 0x03;

	uint32_t tile_h = 8 << ((attr & PAGE_TILE_HEIGHT_MASK) >> PAGE_TILE_HEIGHT_SHIFT);
	uint32_t tile_w = 8 << ((attr & PAGE_TILE_WIDTH_MASK) >> PAGE_TILE_WIDTH_SHIFT);

	uint32_t tile_count_x = 512 / tile_w;

	uint32_t bitmap_y = (scanline + yscroll) & 0xff;
	uint32_t y0 = bitmap_y / tile_h;
	uint32_t tile_scanline = bitmap_y % tile_h;
	uint32_t tile_address = tile_count_x * y0;

	for (uint32_t x0 = 0; x0 < tile_count_x; x0++, tile_address++)
	{
		uint32_t yy = ((tile_h * y0 - yscroll + 0x10) & 0xff) - 0x10;
		uint32_t xx = (tile_w * x0 - xscroll) & 0x1ff;
		uint16_t tile = (ctrl & PAGE_WALLPAPER_MASK) ? space2.read_word(tilemap*2) : space2.read_word((tilemap + tile_address)*2);

		if (!tile)
			continue;

		uint8_t pal = 0x000;
		uint8_t pri = 0x00;
		bool flip_x = false;

		if (!(ctrl & 0x0002)) // 'regset'
		{
			uint16_t extra_attribute = space2.read_word((palette_map * 2) + tile_address);

			if (x0 & 1)
				extra_attribute = (extra_attribute & 0x00ff);
			else
				extra_attribute = (extra_attribute & 0xff00) >> 8;

			pal = extra_attribute & 0x0f;
			pri = (extra_attribute & 0x30) >> 4;
			flip_x = extra_attribute & 0x40;
		}

		if (pri == priority)
		{

			if (flip_x)
				draw<FlipXOn>(cliprect, tile_scanline, xx, yy, ctrl, bitmap_addr, tile, 0, pal, tile_h, tile_w, bpp);
			else
				draw<FlipXOff>(cliprect, tile_scanline, xx, yy, ctrl, bitmap_addr, tile, 0, pal, tile_h, tile_w, bpp);
		}

	}
}


void spg110_video_device::draw_sprite(const rectangle &cliprect, uint32_t scanline, int priority, uint32_t base_addr)
{

	// m_sprtileno  tttt tttt tttt tttt    t =  tile number (all bits?)
	// m_sprattr1   xxxx xxxx yyyy yyyy    x = low x bits, y = low y bits
	// m_sprattr2   YXzz pppp hhww fFbb    X = high x bit, z = priority, p = palette, h = height, w = width, f = flipy,  F = flipx, b = bpp, Y = high y bit

	uint16_t tile = m_sprtileno[base_addr];

	if (!tile)
	{
		return;
	}

	uint32_t bitmap_addr = 0x40 * m_tilebase;
	uint16_t attr1 = m_sprattr1[base_addr];
	uint16_t attr2 = m_sprattr2[base_addr];

	int x = (attr1 >> 8) & 0xff;
	int y = (attr1) & 0xff;
	uint8_t pri = (attr2 & 0x3000)>>12;
	bool flip_x = (attr2 & 0x0004)>>2;
	bool flip_y = (attr2 & 0x0008)>>3;

	if (!(attr2 & 0x4000))
		x+= 0x100;

	if (!(attr2 & 0x8000))
		y+= 0x100;

	const uint32_t h = 8 << ((attr2 & PAGE_TILE_HEIGHT_MASK) >> PAGE_TILE_HEIGHT_SHIFT);
	const uint32_t w = 8 << ((attr2 & PAGE_TILE_WIDTH_MASK) >> PAGE_TILE_WIDTH_SHIFT);

//  if (!(m_video_regs[0x42] & SPRITE_COORD_TL_MASK))
//  {
//      x = (160 + x) - w / 2;
//      y = (120 - y) - (h / 2) + 8;
//  }

	y = 0x1ff - y - 128 + 1;
	x = x - 128 + 32;

	x -= (w / 2);
	y -= (h / 2);

	x &= 0x01ff;
	y &= 0x01ff;

	uint32_t tile_line = ((scanline - y)) % h;
	int16_t test_y = (y + tile_line) & 0x1ff;
	if (test_y >= 0x01c0)
		test_y -= 0x0200;

	if (test_y != scanline)
	{
		return;
	}

	//bool blend = (attr & 0x4000);
	const uint8_t bpp = attr2 & 0x0003;
	const uint32_t yflipmask = flip_y ? h - 1 : 0;
	const uint32_t palette_offset = (attr2 & 0x0f00) >> 8;

	if (pri == priority)
	{
		if (flip_x)
			draw<FlipXOn>(cliprect, tile_line, x, y, 0, bitmap_addr, tile, yflipmask, palette_offset, h, w, bpp);
		else
			draw<FlipXOff>(cliprect, tile_line, x, y, 0, bitmap_addr, tile, yflipmask, palette_offset, h, w, bpp);
	}
}



void spg110_video_device::draw_sprites(const rectangle &cliprect, uint32_t scanline, int priority)
{
	//if (!(m_video_regs[0x42] & SPRITE_ENABLE_MASK))
	//{
	//  return;
	//}

	for (uint32_t n = 0; n < 256; n++)
	{
		draw_sprite(cliprect, scanline, priority, n);
	}
}


/* correct, 4bpp gfxs */
static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ 0*4,1*4,2*4,3*4,4*4,5*4,6*4,7*4 },
	{ STEP8(0,4*8) },
	8*8*4
};

static const gfx_layout charlayout6 =
{
	8,8,
	RGN_FRAC(1,1),
	6,
	{ 0,1,2,3,4,5 },
	{ STEP8(0,6) },
	{ STEP8(0,6*8) },
	8*8*6
};

static const gfx_layout char16layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ 0*4,1*4,2*4,3*4,4*4,5*4,6*4,7*4, 8*4,9*4,10*4,11*4,12*4,13*4,14*4,15*4 },
	{ STEP16(0,4*16) },
	16*16*4
};

static const gfx_layout char32layout =
{
	32,32,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ STEP32(0,4) },
	{ STEP32(0,4*32) },
	32*32*4
};



static GFXDECODE_START( gfx )
	GFXDECODE_ENTRY( ":maincpu", 0, charlayout, 0, 16 )
	GFXDECODE_ENTRY( ":maincpu", 0, char16layout, 0, 16 )
	GFXDECODE_ENTRY( ":maincpu", 0, char32layout, 0, 16 )
	GFXDECODE_ENTRY( ":maincpu", 0, charlayout6, 0, 16 ) // correct for lots of the tiles inc. startup text
GFXDECODE_END


void spg110_video_device::device_add_mconfig(machine_config &config)
{
	PALETTE(config, m_palette).set_entries(0x100);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx);
}


device_memory_interface::space_config_vector spg110_video_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}


// irq source or similar?
READ16_MEMBER(spg110_video_device::spg110_2063_r)
{
	// checks for bits 0x20 and 0x08 in the IRQ function (all IRQs point to the same place)

	// HACK! jak_spdo checks for 0x400 or 0x200 starting some of the games
	return m_video_irq_status | 0x600;
}

WRITE16_MEMBER(spg110_video_device::spg110_2063_w)
{
	// writes 0x28, probably clears the IRQ / IRQ sources? 0x63 is the same offset for this in spg2xx but bits used seem to be different
	const uint16_t old = m_video_irq_enable & m_video_irq_status;
	m_video_irq_status &= ~data;
	const uint16_t changed = old ^ (m_video_irq_enable & m_video_irq_status);
	if (changed)
		check_video_irq();
}


WRITE16_MEMBER(spg110_video_device::spg110_201c_w) { logerror("%s: 201c: %04x\n", machine().describe_context(), data); } // during startup text only
WRITE16_MEMBER(spg110_video_device::spg110_2020_w) { COMBINE_DATA(&m_tilebase); logerror("%s: 2020: %04x\n", machine().describe_context(), data); } // confirmed as tile base, seems to apply to both layers and sprites, unlike spg2xx which has separate registers

WRITE16_MEMBER(spg110_video_device::spg110_2028_w) { logerror("%s: 2028: %04x\n", machine().describe_context(), data); } // startup
READ16_MEMBER(spg110_video_device::spg110_2028_r) { return 0x0000; }

WRITE16_MEMBER(spg110_video_device::spg110_2029_w) { logerror("%s: 2029: %04x\n", machine().describe_context(), data); } // 0006, 0008 on startup
READ16_MEMBER(spg110_video_device::spg110_2029_r) { return 0x0000; }

WRITE16_MEMBER(spg110_video_device::spg110_2031_w) { logerror("%s: 2031: %04x\n", machine().describe_context(), data); } // 014a or 0000 when ball is in trap
WRITE16_MEMBER(spg110_video_device::spg110_2032_w) { logerror("%s: 2032: %04x\n", machine().describe_context(), data); } // 014a most of the time, 0000 very rarely
WRITE16_MEMBER(spg110_video_device::spg110_2033_w) { logerror("%s: 2033: %04x\n", machine().describe_context(), data); } // changes, situational, eg when pausing
WRITE16_MEMBER(spg110_video_device::spg110_2034_w) { logerror("%s: 2034: %04x\n", machine().describe_context(), data); } // 0141 on every scene transition
WRITE16_MEMBER(spg110_video_device::spg110_2035_w) { logerror("%s: 2035: %04x\n", machine().describe_context(), data); } // 0141 on every scene transition
WRITE16_MEMBER(spg110_video_device::spg110_2036_w) { logerror("%s: 2036: %04x\n", machine().describe_context(), data); COMBINE_DATA(&m_2036_scroll); } // seems related to ball y position, not scrolling (possibly shadow sprite related?)

READ16_MEMBER(spg110_video_device::spg110_2037_r) { return 0x0000; } // added to something from the PRNG
WRITE16_MEMBER(spg110_video_device::spg110_2037_w) { logerror("%s: 2037: %04x\n", machine().describe_context(), data); } // 0126 (always?)

WRITE16_MEMBER(spg110_video_device::spg110_2039_w) { logerror("%s: 2039: %04x\n", machine().describe_context(), data); } // 0803 on every scene transition

WRITE16_MEMBER(spg110_video_device::spg110_203c_w) { logerror("%s: 203c: %04x\n", machine().describe_context(), data); } // 0006 on startup, twice

WRITE16_MEMBER(spg110_video_device::spg110_203d_w) { logerror("%s: 203d: %04x\n", machine().describe_context(), data); } // changes, usually between scenes

READ16_MEMBER(spg110_video_device::spg110_2042_r) { return 0x0000; }
WRITE16_MEMBER(spg110_video_device::spg110_2042_w) { logerror("%s: 2042: %04x\n", machine().describe_context(), data);  } // sets bit 0x0004, masks with 0xfffb etc.

WRITE16_MEMBER(spg110_video_device::spg110_2045_w) { logerror("%s: 2045: %04x\n", machine().describe_context(), data);  } // 0006 on startup, once


WRITE16_MEMBER(spg110_video_device::spg110_205x_w)
{
	COMBINE_DATA(&m_palctrlram[offset]);
}


WRITE16_MEMBER(spg110_video_device::dma_unk_2061_w) { COMBINE_DATA(&m_dma_unk_2061); }
WRITE16_MEMBER(spg110_video_device::dma_dst_step_w) { COMBINE_DATA(&m_dma_dst_step); }
WRITE16_MEMBER(spg110_video_device::dma_unk_2067_w) { COMBINE_DATA(&m_dma_src_high); }
WRITE16_MEMBER(spg110_video_device::dma_src_step_w) { COMBINE_DATA(&m_dma_src_step); }

WRITE16_MEMBER(spg110_video_device::dma_dst_w) { COMBINE_DATA(&m_dma_dst); }
WRITE16_MEMBER(spg110_video_device::dma_src_w) { COMBINE_DATA(&m_dma_src); }

WRITE16_MEMBER(spg110_video_device::dma_len_trigger_w)
{
	int length = data & 0x1fff;

	// this is presumably a counter that underflows to 0x1fff, because that's what the wait loop waits for?
	logerror("%s: (trigger len) %04x with values (unk) %04x (dststep) %04x (srchigh) %04x (src step) %04x | (dst) %04x (src) %04x\n", machine().describe_context(), data, m_dma_unk_2061, m_dma_dst_step, m_dma_src_high, m_dma_src_step, m_dma_dst, m_dma_src);

	/*
	if (m_dma_unk_2061 != 0x0000)
	{
	    logerror("unknown DMA params are not zero!\n");
	}
	*/

	int source = m_dma_src | m_dma_src_high << 16;
	int dest = m_dma_dst;

	for (int i = 0; i < length; i++)
	{
		address_space &mem = m_cpu->space(AS_PROGRAM);
		uint16_t val = mem.read_word(source);

		this->space(0).write_word(dest * 2, val, 0xffff);

		source+=m_dma_src_step;
		dest+=m_dma_dst_step;
	}

	// not sure, spiderman would suggest that some of these need to reset (unless a missing IRQ clears them)
	m_dma_unk_2061 = 0;
	//m_dma_dst_step = 0; // conyteni says no
	m_dma_src_high = 0;
	//m_dma_src_step = 0; // conyteni says no
	m_dma_dst = 0;
	m_dma_src = 0;

	// HACK: it really seems this interrupt status is related to the DMA, but jak_capb doesn't ack it, so must also be a way to disable it?
	if (m_is_spiderman)
	{
		const int i = 0x0002;

		if (m_video_irq_enable & 1)
		{
			m_video_irq_status |= i;
			check_video_irq();
		}
	}
}

WRITE16_MEMBER(spg110_video_device::dma_manual_w)
{
	this->space(0).write_word(m_dma_dst * 2, data, 0xffff);
}

READ16_MEMBER(spg110_video_device::dma_manual_r)
{
	uint16_t val = this->space(0).read_word(m_dma_dst * 2);
	return val;
}

READ16_MEMBER(spg110_video_device::dma_len_status_r)
{
	return 0x1fff; // DMA related?
}


READ16_MEMBER(spg110_video_device::tmap0_regs_r) { return tmap0_regs[offset]; }
READ16_MEMBER(spg110_video_device::tmap1_regs_r) { return tmap1_regs[offset]; }

void spg110_video_device::tilemap_write_regs(int which, uint16_t* regs, int regno, uint16_t data)
{
	switch (regno)
	{
	case 0x0: // Page X scroll
		logerror("video_w: Page %d X Scroll = %04x\n", which, data & 0x01ff);
		regs[regno] = data & 0x01ff;
		break;

	case 0x1: // Page Y scroll
		logerror("video_w: Page %d Y Scroll = %04x\n", which, data & 0x00ff);
		regs[regno] = data & 0x00ff;
		break;

	case 0x2: // Page Attributes
		// 'priority' can't be priority here as it is on spg2xx, or the scores in attract will be behind the table, it really seems to be per attribute bit instead

		logerror("video_w: Page %d Attributes = %04x (Priority:%d, Palette:%d, VSize:%d, HSize:%d, FlipY:%d, FlipX:%d, BPP:%d)\n", which, data
			, (data >> 12) & 3, (data >> 8) & 15, 8 << ((data >> 6) & 3), 8 << ((data >> 4) & 3), BIT(data, 3), BIT(data, 2), 2 * ((data & 3) + 1));
		regs[regno] = data;
		break;

	case 0x3: // Page Control
		logerror("video_w: Page %d Control = %04x (Blend:%d, HiColor:%d, RowScroll:%d, Enable:%d, Wallpaper:%d, RegSet:%d, Bitmap:%d)\n", which, data
			, BIT(data, 8), BIT(data, 7), BIT(data, 4), BIT(data, 3), BIT(data, 2), BIT(data, 1), BIT(data, 0));
		regs[regno] = data;
		break;

	case 0x4: // Page Tile Address
		logerror("video_w: Page %d Tile Address = %04x\n", which, data);
		regs[regno] = data;
		break;

	case 0x5: // Page Attribute Address
		logerror("video_w: Page %d Attribute Address = %04x\n", which, data);
		regs[regno] = data;
		break;
	}
}


WRITE16_MEMBER(spg110_video_device::tmap0_regs_w)
{
	tilemap_write_regs(0, tmap0_regs,offset,data);
}


WRITE16_MEMBER(spg110_video_device::tmap1_regs_w)
{
	tilemap_write_regs(1, tmap1_regs,offset,data);
}

// this seems to be a different, non-cpu mapped space only accessible via the DMA?
void spg110_video_device::map_video(address_map &map)
{
	// are these addresses hardcoded, or can they move (in which case tilemap system isn't really suitable)
	map(0x00000, 0x03fff).ram(); // 2fff?

	map(0x04000, 0x041ff).ram().share("sprtileno"); // seems to be 3 blocks, almost certainly spritelist
	map(0x04200, 0x043ff).ram().share("sprattr1");
	map(0x04400, 0x045ff).ram().share("sprattr2");

	map(0x08000, 0x081ff).ram().w(FUNC(spg110_video_device::palette_w)).share("palram"); // palette format unknown
}

void spg110_video_device::device_start()
{
	save_item(NAME(m_dma_src_step));
	save_item(NAME(m_dma_dst_step));
	save_item(NAME(m_dma_unk_2061));
	save_item(NAME(m_dma_src_high));
	save_item(NAME(m_dma_dst));
	save_item(NAME(m_dma_src));
	save_item(NAME(m_bg_scrollx));
	save_item(NAME(m_bg_scrolly));
	save_item(NAME(m_2036_scroll));

	m_video_irq_cb.resolve();

	if (!strcmp(machine().system().name, "jak_spdmo"))
		m_is_spiderman = true;
	else
		m_is_spiderman = false;
}

void spg110_video_device::device_reset()
{
	m_dma_src_step = 0;
	m_dma_dst_step = 0;
	m_dma_unk_2061 = 0;
	m_dma_src_high = 0;
	m_dma_dst = 0;
	m_dma_src = 0;
	m_bg_scrollx = 0;
	m_bg_scrolly = 0;
	m_2036_scroll = 0;

	// is there actually an enable register here?
	m_video_irq_enable = 0xffff;
	m_video_irq_status = 0x0000;
}

double spg110_video_device::hue2rgb(double p, double q, double t)
{
	if (t < 0) t += 1;
	if (t > 1) t -= 1;
	if (t < 1 / 6.0f) return p + (q - p) * 6 * t;
	if (t < 1 / 2.0f) return q;
	if (t < 2 / 3.0f) return p + (q - p) * (2 / 3.0f - t) * 6;
	return p;
}


// wrong format!
WRITE16_MEMBER(spg110_video_device::palette_w)
{
	// probably not
	const double h_add = 0.65f;
	const double h_divide = 43.2f;

	COMBINE_DATA(&m_palram[offset]);

	uint16_t dat = m_palram[offset];

	// llll lsss sshh hhhh
	int l_raw =  (dat & 0xfe00) >> 10;
	int sl_raw = (dat & 0x03c0) >> 6;
	int h_raw =  (dat & 0x003f) >> 0;

	double l = (double)l_raw / 63.0f;
	double s = (double)sl_raw / 15.0f;
	double h = (double)h_raw / h_divide;

	// probably not
	h += h_add;

	if (h>1.0f)
		h-= 1.0f;

	double r, g, b;

	if (s == 0) {
		r = g = b = l; // greyscale
	} else {
		double q = l < 0.5f ? l * (1 + s) : l + s - l * s;
		double p = 2 * l - q;
		r = hue2rgb(p, q, h + 1/3.0f);
		g = hue2rgb(p, q, h);
		b = hue2rgb(p, q, h - 1/3.0f);
	}

	int r_real = r * 255.0f;
	int g_real = g * 255.0f;
	int b_real = b * 255.0f;

	m_palette->set_pen_color(offset, r_real, g_real, b_real);
}

uint32_t spg110_video_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	memset(&m_screenbuf[320 * cliprect.min_y], 0, 4 * 320 * ((cliprect.max_y - cliprect.min_y) + 1));

	const uint32_t page1_addr = 0x40 * m_tilebase;//0x40 * m_video_regs[0x20];
	const uint32_t page2_addr = 0x40 * m_tilebase;//0x40 * m_video_regs[0x21];
	uint16_t *page1_regs = tmap0_regs;
	uint16_t *page2_regs = tmap1_regs;

	for (uint32_t scanline = (uint32_t)cliprect.min_y; scanline <= (uint32_t)cliprect.max_y; scanline++)
	{
		for (int i = 0; i < 4; i++)
		{
			draw_page(cliprect, scanline, i, page2_addr, page2_regs);
			draw_page(cliprect, scanline, i, page1_addr, page1_regs);
			draw_sprites(cliprect, scanline, i);
		}
	}

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint32_t *dest = &bitmap.pix32(y, cliprect.min_x);
		uint32_t *src = &m_screenbuf[cliprect.min_x + 320 * y];
		memcpy(dest, src, sizeof(uint32_t) * ((cliprect.max_x - cliprect.min_x) + 1));
	}

	return 0;
}

WRITE_LINE_MEMBER(spg110_video_device::vblank)
{
	const int i = 0x0008;

	if (!state)
	{
		m_video_irq_status &= ~i;
		check_video_irq();
		return;
	}

	if (m_video_irq_enable & 1)
	{
		m_video_irq_status |= i;
		check_video_irq();
	}
}

void spg110_video_device::check_video_irq()
{
	m_video_irq_cb((m_video_irq_status & m_video_irq_enable) ? ASSERT_LINE : CLEAR_LINE);
}
