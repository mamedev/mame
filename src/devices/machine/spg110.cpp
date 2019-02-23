// license:BSD-3-Clause
// copyright-holders:David Haywood
/*****************************************************************************

    SunPlus SPG110-series SoC peripheral emulation

    0032xx looks like it could be the same as 003dxx on spg2xx
	but the video seems to have differences, and data
	is fetched from private buffers filled by DMA instead of
	main space? tile attributes different? palette format different

**********************************************************************/

#include "emu.h"
#include "spg110.h"

DEFINE_DEVICE_TYPE(SPG110, spg110_device, "spg110", "SPG110 System-on-a-Chip")

spg110_device::spg110_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_space_config("spg110", ENDIANNESS_BIG, 16, 32, 0, address_map_constructor(FUNC(spg110_device::map_video), this)),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_palette(*this, "palette"),
	m_gfxdecode(*this, "gfxdecode"),
	m_palram(*this, "palram")
{
}

template<spg110_device::flipx_t FlipX>
void spg110_device::blit(const rectangle &cliprect, uint32_t line, uint32_t xoff, uint32_t yoff, uint32_t attr, uint32_t ctrl, uint32_t bitmap_addr, uint16_t tile)
{
	address_space &space = m_cpu->space(AS_PROGRAM);

	int32_t h = 8 << ((attr & PAGE_TILE_HEIGHT_MASK) >> PAGE_TILE_HEIGHT_SHIFT);
	int32_t w = 8 << ((attr & PAGE_TILE_WIDTH_MASK) >> PAGE_TILE_WIDTH_SHIFT);

	uint32_t yflipmask = attr & TILE_Y_FLIP ? h - 1 : 0;

	uint32_t nc = ((attr & 0x0003) + 1) << 1;

	uint32_t palette_offset = (attr & 0x0f00) >> 4;

	palette_offset >>= nc;
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
			// TODO, this is completely wrong for this palette system
			int pix_index = xx + y_index;
			uint16_t rawpal = m_palram[pal];
			const pen_t *pens = m_palette->pens();
			uint32_t paldata = pens[pal];

			if (!(rawpal & 0x8000))
			{
				m_screenbuf[pix_index] = paldata;
			}
		}
	}
}

void spg110_device::blit_page(const rectangle &cliprect, uint32_t scanline, int depth, uint32_t bitmap_addr, uint16_t *regs)
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

	if (((attr & PAGE_DEPTH_FLAG_MASK) >> PAGE_DEPTH_FLAG_SHIFT) != depth)
	{
		return;
	}

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
		uint16_t palette = 0;

		if (!tile)
			continue;

		palette = space2.read_word(palette_map + tile_address / 2);
		if (x0 & 1)
			palette = (palette & 0xff00) >> 8;
		else
			palette = (palette & 0x00ff);


		bool flip_x = 0;//(tileattr & TILE_X_FLIP);

		if (flip_x)
			blit<FlipXOn>(cliprect, tile_scanline, xx, yy, attr, ctrl, bitmap_addr, tile);
		else
			blit<FlipXOff>(cliprect, tile_scanline, xx, yy, attr, ctrl, bitmap_addr, tile);
	
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



void spg110_device::device_add_mconfig(machine_config &config)
{
//	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x100);
//	PALETTE(config, m_palette).set_format(palette_device::RGB_565, 0x100);
//	PALETTE(config, m_palette).set_format(palette_device::IRGB_4444, 0x100);
//	PALETTE(config, m_palette).set_format(palette_device::RGBI_4444, 0x100);
//	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x100);
	PALETTE(config, m_palette, palette_device::BLACK, 256);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx);
}


device_memory_interface::space_config_vector spg110_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

spg110_device::spg110_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spg110_device(mconfig, SPG110, tag, owner, clock)
{
}

// irq source or similar?
READ16_MEMBER(spg110_device::spg110_2063_r)
{
	// checks for bits 0x20 and 0x08 in the IRQ function (all IRQs point to the same place)
	return 0x0008;
}

WRITE16_MEMBER(spg110_device::spg110_2063_w)
{
	// writes 0x28, probably clears the IRQ / IRQ sources? 0x63 is the same offset for this in spg2xx but bits used seem to be different
	m_cpu->set_state_unsynced(UNSP_IRQ0_LINE, CLEAR_LINE);
}

READ16_MEMBER(spg110_device::datasegment_r)
{
	uint16_t val = m_cpu->get_ds();
	return val;
}

WRITE16_MEMBER(spg110_device::datasegment_w)
{
	m_cpu->set_ds(data & 0x3f);
}

WRITE16_MEMBER(spg110_device::spg110_3221_w)
{
	/* first write on startup? */
}

WRITE16_MEMBER(spg110_device::spg110_3223_w) { }
WRITE16_MEMBER(spg110_device::spg110_3225_w) { }


WRITE16_MEMBER(spg110_device::spg110_201c_w) { }
WRITE16_MEMBER(spg110_device::spg110_2020_w) { }
WRITE16_MEMBER(spg110_device::spg110_2042_w) { }
WRITE16_MEMBER(spg110_device::spg110_2031_w) { }
WRITE16_MEMBER(spg110_device::spg110_2032_w) { }
WRITE16_MEMBER(spg110_device::spg110_2033_w) { }
WRITE16_MEMBER(spg110_device::spg110_2034_w) { }
WRITE16_MEMBER(spg110_device::spg110_2035_w) { }
WRITE16_MEMBER(spg110_device::spg110_2036_w) { COMBINE_DATA(&m_2036_scroll); }
WRITE16_MEMBER(spg110_device::spg110_2039_w) { }
WRITE16_MEMBER(spg110_device::spg110_2037_w) { }
WRITE16_MEMBER(spg110_device::spg110_203c_w) { }
WRITE16_MEMBER(spg110_device::spg110_203d_w) { }
WRITE16_MEMBER(spg110_device::spg110_2045_w) { }


WRITE16_MEMBER(spg110_device::spg110_2028_w) { }
WRITE16_MEMBER(spg110_device::spg110_2029_w) { }

READ16_MEMBER(spg110_device::spg110_2028_r) { return 0x0000; }
READ16_MEMBER(spg110_device::spg110_2029_r) { return 0x0000; }


WRITE16_MEMBER(spg110_device::spg110_2050_w) { }
WRITE16_MEMBER(spg110_device::spg110_2051_w) { }
WRITE16_MEMBER(spg110_device::spg110_2052_w) { }
WRITE16_MEMBER(spg110_device::spg110_2053_w) { }
WRITE16_MEMBER(spg110_device::spg110_2054_w) { }
WRITE16_MEMBER(spg110_device::spg110_2055_w) { }
WRITE16_MEMBER(spg110_device::spg110_2056_w) { }
WRITE16_MEMBER(spg110_device::spg110_2057_w) { }
WRITE16_MEMBER(spg110_device::spg110_2058_w) { }
WRITE16_MEMBER(spg110_device::spg110_2059_w) { }
WRITE16_MEMBER(spg110_device::spg110_205a_w) { }
WRITE16_MEMBER(spg110_device::spg110_205b_w) { }
WRITE16_MEMBER(spg110_device::spg110_205c_w) { }
WRITE16_MEMBER(spg110_device::spg110_205d_w) { }
WRITE16_MEMBER(spg110_device::spg110_205e_w) { }
WRITE16_MEMBER(spg110_device::spg110_205f_w) { }

WRITE16_MEMBER(spg110_device::dma_unk_2061_w) { COMBINE_DATA(&m_dma_unk_2061); }
WRITE16_MEMBER(spg110_device::dma_dst_step_w) { COMBINE_DATA(&m_dma_dst_step); }
WRITE16_MEMBER(spg110_device::dma_unk_2067_w) { COMBINE_DATA(&m_dma_unk_2067); }
WRITE16_MEMBER(spg110_device::dma_src_step_w) { COMBINE_DATA(&m_dma_src_step); }

WRITE16_MEMBER(spg110_device::dma_dst_w) { COMBINE_DATA(&m_dma_dst); }
WRITE16_MEMBER(spg110_device::dma_src_w) { COMBINE_DATA(&m_dma_src); }

WRITE16_MEMBER(spg110_device::dma_len_trigger_w)
{
	int length = data & 0x1fff;

	// this is presumably a counter that underflows to 0x1fff, because that's what the wait loop waits for?
	logerror("%s: (trigger len) %04x with values (unk) %04x (dststep) %04x (unk) %04x (src step) %04x | (dst) %04x (src) %04x\n", machine().describe_context(), data, m_dma_unk_2061, m_dma_dst_step, m_dma_unk_2067, m_dma_src_step, m_dma_dst, m_dma_src);

	if ((m_dma_unk_2061!=0x0000) || (m_dma_unk_2067 != 0x0000))
		fatalerror("unknown DMA params are not zero!\n");

	int source = m_dma_src;
	int dest = m_dma_dst;

	for (int i = 0; i < length; i++)
	{
		address_space &mem = m_cpu->space(AS_PROGRAM);
		uint16_t val = mem.read_word(source);

		this->space(0).write_word(dest * 2, val, 0xffff);
	
		source+=m_dma_src_step;
		dest+=m_dma_dst_step;
	}
}

READ16_MEMBER(spg110_device::dma_len_status_r)
{
	return 0x1fff; // DMA related?
}

READ16_MEMBER(spg110_device::spg110_2037_r) { return 0x0000; }
READ16_MEMBER(spg110_device::spg110_2042_r) { return 0x0000; }

WRITE16_MEMBER(spg110_device::spg110_3200_w) { }
WRITE16_MEMBER(spg110_device::spg110_3201_w) { }
WRITE16_MEMBER(spg110_device::spg110_3203_w) { }
WRITE16_MEMBER(spg110_device::spg110_3204_w) { }
WRITE16_MEMBER(spg110_device::spg110_3206_w) { }
WRITE16_MEMBER(spg110_device::spg110_3208_w) { }
WRITE16_MEMBER(spg110_device::spg110_3209_w) { }

READ16_MEMBER(spg110_device::spg110_3201_r) { return 0x0000; }
READ16_MEMBER(spg110_device::spg110_3225_r) { return 0x0000; }
READ16_MEMBER(spg110_device::spg110_322c_r) { return 0x0000; }

WRITE16_MEMBER(spg110_device::spg110_3100_w) { }
WRITE16_MEMBER(spg110_device::spg110_3101_w) { }
WRITE16_MEMBER(spg110_device::spg110_3102_w) { }
WRITE16_MEMBER(spg110_device::spg110_3104_w) { }
WRITE16_MEMBER(spg110_device::spg110_3105_w) { }
WRITE16_MEMBER(spg110_device::spg110_3106_w) { }
WRITE16_MEMBER(spg110_device::spg110_3107_w) { }
WRITE16_MEMBER(spg110_device::spg110_3108_w) { }
WRITE16_MEMBER(spg110_device::spg110_3109_w) { }
WRITE16_MEMBER(spg110_device::spg110_310b_w) { }
WRITE16_MEMBER(spg110_device::spg110_310c_w) { }
WRITE16_MEMBER(spg110_device::spg110_310d_w) { }

READ16_MEMBER(spg110_device::spg110_310f_r) { return 0x0000; }

READ16_MEMBER(spg110_device::tmap0_regs_r) { return tmap0_regs[offset]; }
READ16_MEMBER(spg110_device::tmap1_regs_r) { return tmap1_regs[offset]; }

void spg110_device::tilemap_write_regs(int which, uint16_t* regs, int regno, uint16_t data)
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
		// 'depth' (aka z value) can't be depth here as it is on spg2xx, or the scores in attract will be behind the table, it really seems to be per attribute bit instead

		logerror("video_w: Page %d Attributes = %04x (Depth:%d, Palette:%d, VSize:%d, HSize:%d, FlipY:%d, FlipX:%d, BPP:%d)\n", which, data
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


WRITE16_MEMBER(spg110_device::tmap0_regs_w)
{
	tilemap_write_regs(0, tmap0_regs,offset,data);
}


WRITE16_MEMBER(spg110_device::tmap1_regs_w)
{
	tilemap_write_regs(1, tmap1_regs,offset,data);
}

void spg110_device::map(address_map &map)
{
	map(0x000000, 0x000fff).ram();
	
	
	// vregs are at 2000?
	map(0x002010, 0x002015).rw(FUNC(spg110_device::tmap0_regs_r), FUNC(spg110_device::tmap0_regs_w));
	map(0x002016, 0x00201b).rw(FUNC(spg110_device::tmap1_regs_r), FUNC(spg110_device::tmap1_regs_w));

	map(0x00201c, 0x00201c).w(FUNC(spg110_device::spg110_201c_w));
	
	map(0x002020, 0x002020).w(FUNC(spg110_device::spg110_2020_w));

	map(0x002028, 0x002028).rw(FUNC(spg110_device::spg110_2028_r), FUNC(spg110_device::spg110_2028_w));
	map(0x002029, 0x002029).rw(FUNC(spg110_device::spg110_2029_r), FUNC(spg110_device::spg110_2029_w));

	map(0x002031, 0x002031).w(FUNC(spg110_device::spg110_2031_w)); // sometimes 14a?
	map(0x002032, 0x002032).w(FUNC(spg110_device::spg110_2032_w)); // always 14a?
	map(0x002033, 0x002033).w(FUNC(spg110_device::spg110_2033_w));
	map(0x002034, 0x002034).w(FUNC(spg110_device::spg110_2034_w));
	map(0x002035, 0x002035).w(FUNC(spg110_device::spg110_2035_w));
	map(0x002036, 0x002036).w(FUNC(spg110_device::spg110_2036_w)); // possible scroll register?
	map(0x002037, 0x002037).rw(FUNC(spg110_device::spg110_2037_r), FUNC(spg110_device::spg110_2037_w));

	map(0x002039, 0x002039).w(FUNC(spg110_device::spg110_2039_w));

	map(0x00203c, 0x00203c).w(FUNC(spg110_device::spg110_203c_w));

	map(0x00203d, 0x00203d).w(FUNC(spg110_device::spg110_203d_w)); // possible scroll register?

	map(0x002042, 0x002042).rw(FUNC(spg110_device::spg110_2042_r),FUNC(spg110_device::spg110_2042_w));

	map(0x002045, 0x002045).w(FUNC(spg110_device::spg110_2045_w));

	// seems to be 16 entries for.. something?
	map(0x002050, 0x002050).w(FUNC(spg110_device::spg110_2050_w));
	map(0x002051, 0x002051).w(FUNC(spg110_device::spg110_2051_w));
	map(0x002052, 0x002052).w(FUNC(spg110_device::spg110_2052_w));
	map(0x002053, 0x002053).w(FUNC(spg110_device::spg110_2053_w));
	map(0x002054, 0x002054).w(FUNC(spg110_device::spg110_2054_w));
	map(0x002055, 0x002055).w(FUNC(spg110_device::spg110_2055_w));
	map(0x002056, 0x002056).w(FUNC(spg110_device::spg110_2056_w));
	map(0x002057, 0x002057).w(FUNC(spg110_device::spg110_2057_w));
	map(0x002058, 0x002058).w(FUNC(spg110_device::spg110_2058_w));
	map(0x002059, 0x002059).w(FUNC(spg110_device::spg110_2059_w));
	map(0x00205a, 0x00205a).w(FUNC(spg110_device::spg110_205a_w));
	map(0x00205b, 0x00205b).w(FUNC(spg110_device::spg110_205b_w));
	map(0x00205c, 0x00205c).w(FUNC(spg110_device::spg110_205c_w));
	map(0x00205d, 0x00205d).w(FUNC(spg110_device::spg110_205d_w));
	map(0x00205e, 0x00205e).w(FUNC(spg110_device::spg110_205e_w));
	map(0x00205f, 0x00205f).w(FUNC(spg110_device::spg110_205f_w));
	
	//map(0x002010, 0x00205f).ram();

	// everything (dma? and interrupt flag?!)
	map(0x002060, 0x002060).w(FUNC(spg110_device::dma_dst_w));
	map(0x002061, 0x002061).w(FUNC(spg110_device::dma_unk_2061_w));
	map(0x002062, 0x002062).rw(FUNC(spg110_device::dma_len_status_r),FUNC(spg110_device::dma_len_trigger_w));
	map(0x002063, 0x002063).rw(FUNC(spg110_device::spg110_2063_r),FUNC(spg110_device::spg110_2063_w)); // this looks like interrupt stuff and is checked in the irq like an irq source, but why in the middle of what otherwise look like some kind of DMA?
	map(0x002064, 0x002064).w(FUNC(spg110_device::dma_dst_step_w));
	map(0x002066, 0x002066).w(FUNC(spg110_device::dma_src_w));
	map(0x002067, 0x002067).w(FUNC(spg110_device::dma_unk_2067_w));
	map(0x002068, 0x002068).w(FUNC(spg110_device::dma_src_step_w));

	map(0x002200, 0x0022ff).ram(); // looks like per-pen brightness or similar? strange because palette isn't memory mapped here
	
	map(0x003000, 0x00307f).ram(); // sound registers? seems to be 8 long entries, only uses up to 0x7f?
	map(0x003080, 0x0030ff).ram(); 

	map(0x003100, 0x003100).w(FUNC(spg110_device::spg110_3100_w));
	map(0x003101, 0x003101).w(FUNC(spg110_device::spg110_3101_w));
	map(0x003102, 0x003102).w(FUNC(spg110_device::spg110_3102_w));

	map(0x003104, 0x003104).w(FUNC(spg110_device::spg110_3104_w));
	map(0x003105, 0x003105).w(FUNC(spg110_device::spg110_3105_w));
	map(0x003106, 0x003106).w(FUNC(spg110_device::spg110_3106_w));
	map(0x003107, 0x003107).w(FUNC(spg110_device::spg110_3107_w));
	map(0x003108, 0x003108).w(FUNC(spg110_device::spg110_3108_w));
	map(0x003109, 0x003109).w(FUNC(spg110_device::spg110_3109_w));

	map(0x00310b, 0x00310b).w(FUNC(spg110_device::spg110_310b_w));
	map(0x00310c, 0x00310c).w(FUNC(spg110_device::spg110_310c_w));
	map(0x00310d, 0x00310d).w(FUNC(spg110_device::spg110_310d_w));

	map(0x00310f, 0x00310f).r(FUNC(spg110_device::spg110_310f_r));

	// 0032xx looks like it could be the same as 003d00 on spg2xx
	map(0x003200, 0x003200).w(FUNC(spg110_device::spg110_3200_w));

	map(0x003201, 0x003201).rw(FUNC(spg110_device::spg110_3201_r),FUNC(spg110_device::spg110_3201_w));

	map(0x003203, 0x003203).w(FUNC(spg110_device::spg110_3203_w));
	map(0x003204, 0x003204).w(FUNC(spg110_device::spg110_3204_w));

	map(0x003206, 0x003206).w(FUNC(spg110_device::spg110_3206_w));

	map(0x003208, 0x003208).w(FUNC(spg110_device::spg110_3208_w));
	map(0x003209, 0x003209).w(FUNC(spg110_device::spg110_3209_w));

	map(0x003221, 0x003221).w(FUNC(spg110_device::spg110_3221_w));
	map(0x003223, 0x003223).w(FUNC(spg110_device::spg110_3223_w));
	map(0x003225, 0x003225).rw(FUNC(spg110_device::spg110_3225_r),FUNC(spg110_device::spg110_3225_w));
	map(0x00322c, 0x00322c).r(FUNC(spg110_device::spg110_322c_r));

	map(0x00322f, 0x00322f).rw(FUNC(spg110_device::datasegment_r),FUNC(spg110_device::datasegment_w));
}

// this seems to be a different, non-cpu mapped space only accessible via the DMA?
void spg110_device::map_video(address_map &map)
{
	// are these addresses hardcoded, or can they move (in which case tilemap system isn't really suitable)
	map(0x00000, 0x03fff).ram(); // 2fff?

	map(0x04000, 0x04fff).ram(); // seems to be 3 blocks, almost certainly spritelist

//	map(0x08000, 0x081ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette"); // probably? format unknown tho
	map(0x08000, 0x081ff).ram().share("palram");
}


/*
TIMER_CALLBACK_MEMBER(spg110_device::test_timer)
{
    //
}
*/



void spg110_device::device_start()
{
//  m_test_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(spg110_device::test_timer), this));
	save_item(NAME(m_dma_src_step));
	save_item(NAME(m_dma_dst_step));
	save_item(NAME(m_dma_unk_2061));
	save_item(NAME(m_dma_unk_2067));
	save_item(NAME(m_dma_dst));
	save_item(NAME(m_dma_src));
	save_item(NAME(m_bg_scrollx));
	save_item(NAME(m_bg_scrolly));
	save_item(NAME(m_2036_scroll));
}

void spg110_device::device_reset()
{
	m_dma_src_step = 0;
	m_dma_dst_step = 0;
	m_dma_unk_2061 = 0;
	m_dma_unk_2067 = 0;
	m_dma_dst = 0;
	m_dma_src = 0;
	m_bg_scrollx = 0;
	m_bg_scrolly = 0;
	m_2036_scroll = 0;
}

double spg110_device::hue2rgb(double p, double q, double t)
{
	if (t < 0) t += 1;
	if (t > 1) t -= 1;
	if (t < 1 / 6.0f) return p + (q - p) * 6 * t;
	if (t < 1 / 2.0f) return q;
	if (t < 2 / 3.0f) return p + (q - p) * (2 / 3.0f - t) * 6;
	return p;
}

uint32_t spg110_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// Palette, this is still wrong!
	int offs = 0;
	for (int index = 0;index < 256; index++)
	{
		uint16_t dat = m_palram[offs++];

		// llll lsss sshh hhhh
		int l_raw =  (dat & 0xf800) >> 11;
		int sl_raw = (dat & 0x07c0) >> 6;
		int h_raw =  (dat & 0x003f) >> 0;

		double l = (double)l_raw / 31.0f;
		double s = (double)sl_raw / 31.0f;
		double h = (double)h_raw / 47.0f;

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

		m_palette->set_pen_color(index, r_real, g_real, b_real);
	}

	memset(&m_screenbuf[320 * cliprect.min_y], 0, 4 * 320 * ((cliprect.max_y - cliprect.min_y) + 1));

	const uint32_t page1_addr = 0;//0x40 * m_video_regs[0x20];
	const uint32_t page2_addr = 0;//0x40 * m_video_regs[0x21];
	uint16_t *page1_regs = tmap0_regs;
	uint16_t *page2_regs = tmap1_regs;

	for (uint32_t scanline = (uint32_t)cliprect.min_y; scanline <= (uint32_t)cliprect.max_y; scanline++)
	{
		for (int i = 0; i < 4; i++)
		{
			blit_page(cliprect, scanline, i, page2_addr, page2_regs);
			blit_page(cliprect, scanline, i, page1_addr, page1_regs);
			//blit_sprites(cliprect, scanline, i);
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

WRITE_LINE_MEMBER(spg110_device::vblank)
{
	if (!state)
	{
		m_cpu->set_state_unsynced(UNSP_IRQ0_LINE, ASSERT_LINE);
	//  m_test_timer->adjust(attotime::from_usec(100), 0);
	}

	return;
}
