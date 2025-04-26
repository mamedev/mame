// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "spg110_video.h"

#include <algorithm>

#include "logmacro.h"


DEFINE_DEVICE_TYPE(SPG110_VIDEO, spg110_video_device, "spg110_video", "SPG110 System-on-a-Chip (Video)")

spg110_video_device::spg110_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_space_config("spg110_video", ENDIANNESS_BIG, 16, 32, 0, address_map_constructor(FUNC(spg110_video_device::map_video), this)),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_screen(*this, finder_base::DUMMY_TAG),
	m_palette(*this, "palette"),
	m_gfxdecode(*this, "gfxdecode"),
	m_palram(*this, "palram"),
	m_palctrlram(*this, "palctrlram"),
	m_sprtileno(*this, "sprtileno"),
	m_sprattr1(*this, "sprattr1"),
	m_sprattr2(*this, "sprattr2"),
	m_video_irq_cb(*this)
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

	uint32_t palette_offset = pal;

	if (bpp == 3)
		palette_offset = 0;

	palette_offset <<= 4;

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

		uint8_t pennum = (palette_offset + (bits >> 16));
		bits &= 0xffff;

		xx &= 0x01ff;
		if (xx >= 0x01c0)
			xx -= 0x0200;

		if (xx >= 0 && xx < 320)
		{
			int pix_index = xx + y_index;
			const pen_t *pens = m_palette->pens();
			uint32_t paldata = pens[pennum];

			int transmap = m_palctrlram[(pennum & 0xf0)>>4];

			bool trans = false;

			if (transmap & 0x10) // maybe values other than 0x010 have other meanings, like blending?
				if ((pennum & 0x0f) == (transmap & 0xf))
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
	uint8_t bpp = (attr & 0x0003);
	// flip               0x000c
	uint32_t tile_w = 8 << ((attr & 0x0030) >> PAGE_TILE_WIDTH_SHIFT);
	uint32_t tile_h = 8 << ((attr & 0x00c0) >> PAGE_TILE_HEIGHT_SHIFT);
	uint8_t pal = (attr & 0x0f00)>>8;
	uint8_t pri = (attr & 0x3000)>>12;

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

		bool flip_x = false;
		bool flip_y = false;

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
			flip_y = extra_attribute & 0x80;
		}

		const uint32_t yflipmask = flip_y ? tile_h - 1 : 0;

		if (pri == priority)
		{
			if (flip_x)
				draw<FlipXOn>(cliprect, tile_scanline, xx, yy, ctrl, bitmap_addr, tile, yflipmask, pal, tile_h, tile_w, bpp);
			else
				draw<FlipXOff>(cliprect, tile_scanline, xx, yy, ctrl, bitmap_addr, tile, yflipmask, pal, tile_h, tile_w, bpp);
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

	uint32_t bitmap_addr = 0x40 * m_tilebase[0];
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

	const uint32_t h = 8 << ((attr2 & 0x00c0) >> PAGE_TILE_HEIGHT_SHIFT);
	const uint32_t w = 8 << ((attr2 & 0x0030) >> PAGE_TILE_WIDTH_SHIFT);

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
	uint32_t palette_offset = (attr2 & 0x0f00) >> 8;

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


/* 0x2063 P_DMA_control
despite the name this address is more IRQ control than DMA, although there are some DMA flags in here

bit15     bit14     bit13     bit12     bit11     bit10     bit9    bit8    bit7    bit6      bit5          bit4        bit3          bit2        bit1          bit0
(unused)  (unused)  (unused)  (unused)  DMA_Busy  wrtS      readS   testmd  extsrc  (unused)  VDO_IRQ_Flag  VDO_IRQ_EN  BLK_IRQ_Flag  BLK_IRQ_EN  DMA_IRQ_Flag  DMA_IRQ_EN

DMA_Busy - 0 = free, 1 = busy
wrtS - used when writing 0x2065
readS - used when reading 0x2065
testmd - memory dump mode, causes data from dma_dst to be read in 0x2065?
extsrc - 0 = work RAM, 1 = External Memory
VDO_IRQ_Flag (aka TMc or VDO_IRQ) - Positional IRQ Status / Clear IRQ on write
VDO_IRQ_EN (aka Tme) - Positional IRQ Enable
BLK_IRQ_Flag - VBlank IRQ Status / Clear IRQ on write
BLK_IRQ_EN - VBlank IRQ Enable
DMA_IRQ_Flag - DMA IRQ Status / Clear IRQ on write (can also be cleared by starting a new DMA)
DMA_IRQ_EN - DMA IRQ Enable

NOTE: if an IRQ flag is active when the IRQ Enable is turned on the IRQ will be taken
      writing 0 to IRQ enable does not clear IRQ flag

by default:
BLK IRQ is asserted during the Vblank period, and deasserted when it ends
DMA IRQ is asserted at the end of a DMA, and deasserted when a new one starts
VDO IRQ is asserted when screen position in 2036 / 2037 is hit (has to be manually deasserted?)

*/

uint16_t spg110_video_device::spg110_2063_r()
{
	// TODO these need to be handled properly
	uint8_t readS = 1;
	uint8_t wrtS = 1;
	uint8_t testmd = 0;
	uint8_t extsrc = 0;

	uint16_t ret = 0;

	ret |= m_dma_irq_enable ? 0x0001 : 0x0000;
	ret |= m_dma_irq_flag ? 0x0002 : 0x0000;
	ret |= m_blk_irq_enable ? 0x0004 : 0x0000;
	ret |= m_blk_irq_flag ? 0x0008 : 0x0000;
	ret |= m_vdo_irq_enable ? 0x0010 : 0x0000;
	ret |= m_vdo_irq_flag ? 0x0020 : 0x0000;
	// 0x0040 unused
	ret |= extsrc ? 0x0080 : 0x0000;
	ret |= testmd ? 0x0100 : 0x0000;
	ret |= readS ? 0x0200 : 0x0000;
	ret |= wrtS ? 0x0400 : 0x0000;
	ret |= m_dma_busy ? 0x0800 : 0x0000;
	// 0x1000 unused
	// 0x2000 unused
	// 0x4000 unused
	// 0x8000 unused

	return ret;
}

void spg110_video_device::spg110_2063_w(uint16_t data)
{
	m_dma_irq_enable = data & 0x01;
	m_blk_irq_enable = data & 0x04;
	m_vdo_irq_enable = data & 0x10;

	if (data & 0x02)
		m_dma_irq_flag = 0;

	if (data & 0x08)
		m_blk_irq_flag = 0;

	if (data & 0x20)
		m_vdo_irq_flag = 0;

	// still need to handle wrtS, readS, testmd, extsrc

	update_video_irqs();
}


void spg110_video_device::update_video_irqs()
{
	bool irq_on = false;

	if ((m_blk_irq_enable) && (m_blk_irq_flag))
		irq_on = true;

	if ((m_dma_irq_enable) && (m_dma_irq_flag))
		irq_on = true;

	if ((m_vdo_irq_enable) && (m_vdo_irq_flag))
		irq_on = true;

	m_video_irq_cb(irq_on ? ASSERT_LINE : CLEAR_LINE);
}



void spg110_video_device::vcomp_val_201c_w(uint16_t data) { LOG("%s: vcomp_val_201c_w: %04x\n", machine().describe_context(), data); } // during startup text only
void spg110_video_device::segment_202x_w(offs_t offset, uint16_t data, uint16_t mem_mask) { COMBINE_DATA(&m_tilebase[offset]); LOG("%s: segment/tilebase write: %02x %04x\n", machine().describe_context(), offset, data); } // confirmed as tile base, seems to apply to both layers and sprites, unlike spg2xx which has separate registers

void spg110_video_device::adr_mode_2028_w(uint16_t data) { LOG("%s: adr_mode_2028_w: %04x\n", machine().describe_context(), data); } // startup
uint16_t spg110_video_device::adr_mode_2028_r() { return 0x0000; }

void spg110_video_device::ext_bus_2029_w(uint16_t data) { LOG("%s: ext_bus_2029_w: %04x\n", machine().describe_context(), data); } // 0006, 0008 on startup
uint16_t spg110_video_device::ext_bus_2029_r() { return 0x0000; }

void spg110_video_device::win_mask_1_2031_w(uint16_t data) { LOG("%s: win_mask_1_2031_w: %04x\n", machine().describe_context(), data); } // 014a or 0000 when ball is in trap
void spg110_video_device::win_mask_2_2032_w(uint16_t data) { LOG("%s: win_mask_2_2032_w: %04x\n", machine().describe_context(), data); } // 014a most of the time, 0000 very rarely
void spg110_video_device::win_attribute_w(uint16_t data) { LOG("%s: win_attribute_w: %04x\n", machine().describe_context(), data); } // changes, situational, eg when pausing
void spg110_video_device::win_mask_3_2034_w(uint16_t data) { LOG("%s: win_mask_3_2034_w: %04x\n", machine().describe_context(), data); } // 0141 on every scene transition
void spg110_video_device::win_mask_4_2035_w(uint16_t data) { LOG("%s: win_mask_4_2035_w: %04x\n", machine().describe_context(), data); } // 0141 on every scene transition

void spg110_video_device::irq_tm_v_2036_w(uint16_t data)
{
	// used for scanline raster effects, including ball shadow in capb
	m_tm_v_2036 = data & 0x1ff;
	update_raster_interrupt_timer();
}

uint16_t spg110_video_device::irq_tm_h_2037_r()
{
	// added to value from the PRNG for some random number generation cases
	// should this return the *current* horizontal position? or the register value written?
	return m_screen->hpos();
}

void spg110_video_device::irq_tm_h_2037_w(uint16_t data)
{
	// horizontal position for the scanline IRQ
	m_tm_h_2037 = data & 0x1ff;
	update_raster_interrupt_timer();
}

void spg110_video_device::effect_control_2039_w(uint16_t data)
{
	// 0803 on every scene transition
	LOG("%s: effect_control_2039_w: %04x\n", machine().describe_context(), data);
}

void spg110_video_device::huereference_203c_w(uint16_t data)
{
	// 0006 on startup, twice
	LOG("%s: huereference_203c_w: %04x\n", machine().describe_context(), data);
}

void spg110_video_device::lum_adjust_203d_w(uint16_t data)
{
	// changes, usually between scenes (brightness)
	LOG("%s: lum_adjust_203d_w: %04x\n", machine().describe_context(), data);
}

uint16_t spg110_video_device::sp_control_2042_r()
{
	return 0x0000;
}

void spg110_video_device::sp_control_2042_w(uint16_t data)
{
	// sets bit 0x0004, masks with 0xfffb etc.
	LOG("%s: sp_control_2042_w: %04x\n", machine().describe_context(), data);
}

void spg110_video_device::spg110_2045_w(uint16_t data)
{
	// 0006 on startup, once
	LOG("%s: spg110_2045_w: %04x\n", machine().describe_context(), data);
}

void spg110_video_device::transparent_color_205x_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_palctrlram[offset]);
}

void spg110_video_device::dma_dst_seg_2061_w(offs_t offset, uint16_t data, uint16_t mem_mask) { COMBINE_DATA(&m_dma_dst_seg); }
void spg110_video_device::dma_dst_step_2064_w(offs_t offset, uint16_t data, uint16_t mem_mask) { COMBINE_DATA(&m_dma_dst_step); }
void spg110_video_device::dma_source_seg_2067_w(offs_t offset, uint16_t data, uint16_t mem_mask) { COMBINE_DATA(&m_dma_src_seg); }
void spg110_video_device::dma_src_step_2068_w(offs_t offset, uint16_t data, uint16_t mem_mask) { COMBINE_DATA(&m_dma_src_step); }
uint16_t spg110_video_device::dma_src_step_2068_r(offs_t offset, uint16_t mem_mask) { return m_dma_src_step; }

void spg110_video_device::dma_dst_2060_w(offs_t offset, uint16_t data, uint16_t mem_mask) { COMBINE_DATA(&m_dma_dst); }
void spg110_video_device::dma_source_2066_w(offs_t offset, uint16_t data, uint16_t mem_mask) { COMBINE_DATA(&m_dma_src); }

void spg110_video_device::dma_len_trigger_2062_w(uint16_t data)
{
	int length = data & 0x1fff;
	m_dma_irq_flag = 0;
	m_dma_busy = 1;
	update_video_irqs();

	// this is presumably a counter that underflows to 0x1fff, because that's what the wait loop waits for?
	LOG("%s: (trigger len) %04x with values (dstseg) %04x (dststep) %04x (srchigh) %04x (src step) %04x | (dst) %04x (src) %04x\n", machine().describe_context(), data, m_dma_dst_seg, m_dma_dst_step, m_dma_src_seg, m_dma_src_step, m_dma_dst, m_dma_src);

	/*
	if (m_dma_dst_seg != 0x0000)
	{
	    LOG("unknown DMA params are not zero!\n");
	}
	*/

	int source = m_dma_src | m_dma_src_seg << 16;
	int dest = m_dma_dst;

	// bits 0xc000 are 'DMABANK'
	// the use of other bits change slightly depending on them, for example
	// palette tranfsers only use the bottom 8 bits of dest, jak_bobb relies on this or bridge pieces have no palette
	if ((m_dma_dst & 0xc000) == 0x4000)
		dest &= 0x40ff;

	for (int i = 0; i < length; i++)
	{
		address_space &mem = m_cpu->space(AS_PROGRAM);
		uint16_t val = mem.read_word(source);

		this->space(0).write_word(dest * 2, val, 0xffff);

		source+=m_dma_src_step;
		dest+=m_dma_dst_step;
	}

	// not sure, spiderman would suggest that some of these need to reset (unless a missing IRQ clears them)
	m_dma_dst_seg = 0;
	//m_dma_dst_step = 0; // conyteni says no
	m_dma_src_seg = 0;
	//m_dma_src_step = 0; // conyteni says no
	m_dma_dst = dest;
	m_dma_src = source;

	m_dma_timer->adjust(attotime::from_usec(20));
}

void spg110_video_device::dma_manual_2065_w(uint16_t data)
{
	this->space(0).write_word(m_dma_dst * 2, data, 0xffff);
}

uint16_t spg110_video_device::dma_manual_2065_r()
{
	uint16_t val = this->space(0).read_word(m_dma_dst * 2);
	return val;
}

uint16_t spg110_video_device::dma_len_status_2062_r()
{
	return 0x1fff; // DMA related?
}


uint16_t spg110_video_device::tmap0_regs_r(offs_t offset) { return tmap0_regs[offset]; }
uint16_t spg110_video_device::tmap1_regs_r(offs_t offset) { return tmap1_regs[offset]; }

void spg110_video_device::tilemap_write_regs(int which, uint16_t* regs, int regno, uint16_t data)
{
	switch (regno)
	{
	case 0x0: // Page X scroll
		LOG("video_w: Page %d X Scroll = %04x\n", which, data & 0x01ff);
		regs[regno] = data & 0x01ff;
		break;

	case 0x1: // Page Y scroll
		LOG("video_w: Page %d Y Scroll = %04x\n", which, data & 0x00ff);
		regs[regno] = data & 0x00ff;
		break;

	case 0x2: // Page Attributes
		LOG("video_w: Page %d Attributes = %04x (Priority:%d, Palette:%d, VSize:%d, HSize:%d, FlipY:%d, FlipX:%d, BPP:%d)\n", which, data
			, (data >> 12) & 3, (data >> 8) & 15, 8 << ((data >> 6) & 3), 8 << ((data >> 4) & 3), BIT(data, 3), BIT(data, 2), 2 * ((data & 3) + 1));
		regs[regno] = data;
		break;

	case 0x3: // Page Control
		LOG("video_w: Page %d Control = %04x (Blend:%d, HiColor:%d, RowScroll:%d, Enable:%d, Wallpaper:%d, RegSet:%d, Bitmap:%d)\n", which, data
			, BIT(data, 8), BIT(data, 7), BIT(data, 4), BIT(data, 3), BIT(data, 2), BIT(data, 1), BIT(data, 0));
		regs[regno] = data;
		break;

	case 0x4: // Page Tile Address
		LOG("video_w: Page %d Tile Address = %04x\n", which, data);
		regs[regno] = data;
		break;

	case 0x5: // Page Attribute Address
		LOG("video_w: Page %d Attribute Address = %04x\n", which, data);
		regs[regno] = data;
		break;
	}
}


void spg110_video_device::tmap0_regs_w(offs_t offset, uint16_t data)
{
	tilemap_write_regs(0, tmap0_regs,offset,data);
}


void spg110_video_device::tmap1_regs_w(offs_t offset, uint16_t data)
{
	tilemap_write_regs(1, tmap1_regs,offset,data);
}

// this seems to be a different, non-cpu mapped space only accessible via the DMA?
void spg110_video_device::map_video(address_map &map)
{
	map(0x00000, 0x02fff).ram();

	map(0x04000, 0x041ff).ram().share("sprtileno");
	map(0x04200, 0x043ff).ram().share("sprattr1");
	map(0x04400, 0x045ff).ram().share("sprattr2");

	map(0x08000, 0x081ff).ram().w(FUNC(spg110_video_device::palette_w)).share("palram");
}

// Not used, for reference
uint16_t spg110_video_device::rgb_to_hsl(uint8_t r, uint8_t g, uint8_t b)
{
	uint16_t lum = uint16_t(0.0748*r + 0.1467*g + 0.0286*b + 0.5);
	if(lum >= 64)
		lum = 63;

	uint16_t sat = uint16_t(fabs(0.0308*r - 0.0142*g - 0.0166*b) + fabs(0.0110*r - 0.0270*g + 0.0160*b));
	if(sat >= 8)
		sat = 7;

	uint8_t mi = std::min(r, std::min(g, b));
	uint8_t mx = std::max(r, std::max(g, b));

	int16_t hue;
	if(mi == mx)
		hue = 47;

	else {
		if(r == mx)
			hue = int16_t((44/3.0)*(g-b)/(mx-mi) - 11);
		else if(g == mx)
			hue = int16_t((44/3.0)*(b-r)/(mx-mi) + 18.33);
		else
			hue = int16_t((44/3.0)*(r-g)/(mx-mi) + 47.67);
		if(hue < 0)
			hue += 88;
	}

	return (lum << 10) | (sat << 7) | hue;
}

std::tuple<uint8_t, uint8_t, uint8_t> spg110_video_device::hsl_to_rgb(uint16_t hsl)
{
	// r/g/b is mostly linear with l, so this is a table of
	// intercept/slope (e.g. color = intercept + l*slope) for each h/s
	// value

	static const struct {
		float ri, rs;
		float gi, gs;
		float bi, bs;
	} color_bases[88][8] = {
		{ {    5.4, 4.01,    0.2, 4.00,  -14.6, 3.95 }, {   13.4, 4.00,    1.2, 3.97,  -35.7, 3.94 }, {   22.7, 3.99,    1.0, 4.00,  -59.0, 3.91 }, {   32.2, 3.95,    0.3, 4.02,  -81.2, 3.85 }, {   43.3, 3.90,   -1.0, 4.06, -104.1, 3.81 }, {   59.1, 3.73,   -3.5, 4.13, -119.4, 3.61 }, {   94.4, 3.13,  -15.1, 4.39, -126.9, 3.28 }, {  189.0, 1.28,  -57.3, 5.30, -102.3, 2.33 } },
		{ {    3.7, 4.01,    1.1, 4.00,  -15.2, 3.94 }, {    9.5, 4.01,    2.7, 4.00,  -36.9, 3.94 }, {   16.3, 4.01,    4.2, 4.00,  -61.6, 3.92 }, {   22.6, 4.00,    6.1, 4.00,  -84.2, 3.85 }, {   29.9, 3.98,    7.3, 4.01, -108.8, 3.83 }, {   37.1, 3.96,    8.0, 4.03, -123.8, 3.61 }, {   50.6, 3.81,    7.8, 4.07, -139.6, 3.44 }, {   95.2, 3.01,   -1.2, 4.28, -139.6, 3.03 } },
		{ {    2.9, 4.00,    1.8, 4.00,  -16.1, 3.96 }, {    7.6, 4.00,    3.9, 4.00,  -37.9, 3.95 }, {   12.8, 4.00,    6.3, 3.99,  -62.5, 3.92 }, {   17.4, 4.01,    8.5, 4.01,  -86.5, 3.85 }, {   23.3, 3.99,   10.9, 4.02, -107.9, 3.76 }, {   30.0, 3.95,   12.5, 4.03, -128.9, 3.66 }, {   37.8, 3.88,   13.1, 4.07, -144.3, 3.48 }, {   68.0, 3.39,    9.1, 4.19, -127.4, 2.71 } },
		{ {    1.8, 4.01,    1.8, 4.00,  -12.2, 3.91 }, {    5.7, 4.00,    4.8, 4.00,  -39.0, 3.92 }, {    8.9, 4.00,    8.7, 3.99,  -64.0, 3.90 }, {   13.0, 4.00,   11.4, 4.01,  -87.5, 3.85 }, {   16.7, 4.00,   14.3, 4.01, -110.5, 3.77 }, {   20.7, 4.00,   15.8, 4.06, -130.8, 3.64 }, {   28.9, 3.90,   18.2, 4.07, -138.5, 3.30 }, {   46.9, 3.63,   19.4, 4.10, -139.6, 2.89 } },
		{ {    1.8, 4.00,    2.9, 4.00,  -17.8, 3.95 }, {    4.1, 3.98,    5.8, 4.00,  -38.2, 3.93 }, {    5.9, 4.00,   10.8, 3.98,  -65.0, 3.90 }, {    8.0, 4.00,   14.2, 4.01,  -89.8, 3.85 }, {   12.2, 3.96,   17.8, 4.02, -112.0, 3.76 }, {   14.3, 3.97,   21.4, 4.02, -132.0, 3.63 }, {   16.2, 3.98,   25.2, 4.02, -143.7, 3.35 }, {   16.1, 4.04,   30.4, 3.99, -135.7, 2.77 } },
		{ {    1.1, 4.00,    3.9, 3.99,  -15.8, 3.93 }, {    1.2, 3.99,    7.6, 4.00,  -39.9, 3.92 }, {    2.2, 3.99,   12.1, 4.02,  -64.7, 3.87 }, {    3.2, 4.00,   16.8, 4.02,  -90.9, 3.84 }, {    4.6, 3.99,   21.6, 4.02, -115.2, 3.78 }, {    6.1, 3.98,   26.7, 4.01, -135.9, 3.65 }, {    6.5, 4.00,   31.6, 4.00, -147.5, 3.37 }, {   13.8, 3.89,   55.0, 3.62, -160.2, 3.21 } },
		{ {   -0.6, 3.99,    3.4, 4.01,  -15.8, 3.93 }, {   -0.7, 3.99,    8.4, 4.01,  -39.9, 3.90 }, {   -1.3, 3.99,   14.3, 4.01,  -66.5, 3.88 }, {   -1.5, 3.97,   20.5, 4.00,  -92.3, 3.84 }, {   -2.1, 3.98,   25.4, 4.02, -117.7, 3.77 }, {   -2.6, 3.98,   31.8, 4.00, -137.9, 3.63 }, {    1.4, 3.89,   34.8, 4.05, -134.6, 3.04 }, {   18.2, 3.60,   67.0, 3.50, -136.2, 2.70 } },
		{ {   -1.5, 3.99,    4.3, 4.01,  -16.4, 3.92 }, {   -3.3, 3.98,   10.2, 4.01,  -40.7, 3.90 }, {   -4.9, 3.96,   16.7, 4.02,  -68.5, 3.87 }, {   -7.8, 3.98,   23.8, 4.01,  -95.6, 3.85 }, {   -8.4, 3.94,   29.0, 4.05, -117.1, 3.70 }, {  -12.5, 3.99,   38.9, 3.97, -144.5, 3.71 }, {  -11.4, 3.94,   46.1, 3.94, -143.1, 3.17 }, {   90.0, 2.00,  150.0, 2.00, -102.0, 2.00 } },
		{ {   -2.3, 3.98,    4.3, 4.01,  -16.4, 3.93 }, {   -6.3, 3.99,   12.0, 4.01,  -43.0, 3.93 }, {   -9.5, 3.96,   19.6, 4.02,  -70.3, 3.88 }, {  -13.4, 3.95,   26.6, 4.04,  -95.4, 3.79 }, {  -17.0, 3.95,   33.9, 4.05, -120.4, 3.70 }, {  -14.8, 3.82,   39.4, 4.08, -131.5, 3.35 }, {  -12.6, 3.73,   55.0, 3.88, -139.6, 3.05 }, {   66.7, 2.17,  139.8, 2.30,  -98.8, 2.00 } },
		{ {   -3.0, 3.97,    5.2, 4.02,  -17.0, 3.92 }, {   -9.2, 3.99,   14.0, 4.01,  -43.8, 3.93 }, {  -14.7, 3.95,   22.3, 4.03,  -70.7, 3.84 }, {  -20.8, 3.94,   31.0, 4.03,  -98.8, 3.81 }, {  -26.2, 3.94,   40.0, 4.03, -124.1, 3.72 }, {  -26.5, 3.83,   49.7, 3.99, -137.7, 3.43 }, {   -4.0, 3.30,   60.6, 3.90, -120.8, 2.60 }, {   43.3, 2.33,  129.5, 2.60,  -95.5, 2.00 } },
		{ {   -4.4, 3.97,    5.7, 4.02,  -16.0, 3.90 }, {  -12.5, 3.97,   15.5, 4.02,  -44.6, 3.90 }, {  -21.1, 3.97,   26.2, 4.02,  -74.7, 3.90 }, {  -28.7, 3.94,   35.8, 4.03, -101.9, 3.80 }, {  -37.3, 3.95,   48.2, 3.97, -128.9, 3.76 }, {  -31.4, 3.68,   54.4, 4.03, -132.0, 3.24 }, {   95.0, 1.00,  110.0, 3.00,  -47.0, 1.00 }, {   20.0, 2.50,  119.2, 2.90,  -92.2, 2.00 } },
		{ {   -6.9, 3.98,    8.1, 4.01,  -20.0, 3.95 }, {  -16.0, 3.96,   17.6, 4.02,  -45.3, 3.91 }, {  -27.1, 3.95,   29.1, 4.02,  -74.5, 3.87 }, {  -36.7, 3.93,   39.7, 4.05,  -97.7, 3.69 }, {  -47.0, 3.91,   50.8, 4.05, -114.5, 3.39 }, {  -70.4, 4.23,   69.6, 3.85, -106.9, 2.66 }, {   19.7, 2.25,   91.2, 3.50,  -70.8, 1.66 }, {   -3.3, 2.67,  109.0, 3.20,  -89.0, 2.00 } },
		{ {   -6.8, 3.98,    7.1, 4.01,  -16.1, 3.95 }, {  -17.7, 3.96,   17.6, 4.02,  -40.7, 3.93 }, {  -28.8, 3.94,   27.9, 4.03,  -64.7, 3.84 }, {  -39.7, 3.92,   38.8, 4.05,  -88.1, 3.73 }, {  -49.5, 3.86,   48.2, 4.10, -105.3, 3.51 }, {  -57.8, 3.79,   56.0, 4.17, -109.6, 3.05 }, {  -55.6, 3.50,   72.3, 4.00,  -94.7, 2.32 }, {  -26.7, 2.83,   98.8, 3.50,  -85.8, 2.00 } },
		{ {   -7.8, 3.98,    7.0, 4.02,  -14.8, 3.96 }, {  -18.4, 3.97,   16.8, 4.02,  -35.8, 3.95 }, {  -30.8, 3.96,   27.5, 4.02,  -58.3, 3.91 }, {  -42.6, 3.94,   38.1, 4.03,  -80.1, 3.84 }, {  -53.2, 3.88,   47.6, 4.08,  -98.4, 3.70 }, {  -64.8, 3.87,   60.2, 4.02, -116.9, 3.61 }, {  -70.8, 3.74,   68.0, 4.08, -108.7, 2.95 }, {  -50.0, 3.00,   88.5, 3.80,  -82.5, 2.00 } },
		{ {   -8.0, 3.98,    6.5, 4.01,  -13.3, 3.97 }, {  -19.2, 3.97,   16.1, 4.01,  -31.2, 3.95 }, {  -32.2, 3.97,   27.1, 4.01,  -52.7, 3.94 }, {  -44.3, 3.94,   37.3, 4.03,  -72.1, 3.88 }, {  -56.4, 3.93,   47.8, 4.03,  -91.2, 3.83 }, {  -67.8, 3.88,   57.0, 4.07, -104.5, 3.66 }, {  -74.3, 3.73,   64.6, 4.14, -106.4, 3.21 }, {  -84.9, 3.72,   81.3, 3.94, -108.8, 2.91 } },
		{ {   -8.2, 3.99,    6.6, 4.00,  -11.8, 3.98 }, {  -19.8, 3.98,   16.4, 4.00,  -28.3, 3.97 }, {  -32.7, 3.95,   26.0, 4.02,  -46.4, 3.93 }, {  -45.9, 3.96,   37.0, 4.01,  -65.3, 3.93 }, {  -58.9, 3.94,   46.6, 4.02,  -82.8, 3.90 }, {  -69.5, 3.88,   56.0, 4.06,  -95.1, 3.72 }, {  -82.6, 3.87,   66.6, 4.05, -110.6, 3.66 }, {  -86.9, 3.65,   71.1, 4.19, -101.2, 2.98 } },
		{ {   -8.4, 3.99,    6.8, 4.00,  -11.1, 3.98 }, {  -20.5, 3.97,   15.5, 4.00,  -25.3, 3.97 }, {  -33.5, 3.98,   25.5, 4.00,  -41.8, 3.96 }, {  -46.5, 3.95,   35.9, 4.01,  -57.6, 3.93 }, {  -60.6, 3.95,   46.0, 4.01,  -74.8, 3.92 }, {  -73.4, 3.95,   56.6, 4.01,  -90.0, 3.88 }, {  -85.5, 3.91,   65.8, 4.03, -101.2, 3.74 }, {  -94.2, 3.72,   74.9, 4.09, -110.4, 3.50 } },
		{ {  -10.7, 4.00,    8.0, 3.99,  -11.7, 4.00 }, {  -21.0, 3.98,   15.6, 4.00,  -23.0, 3.98 }, {  -35.2, 3.99,   25.8, 3.99,  -38.9, 3.99 }, {  -47.0, 3.94,   35.2, 4.01,  -52.4, 3.95 }, {  -62.1, 3.97,   46.0, 3.98,  -68.3, 3.96 }, {  -75.5, 3.95,   55.3, 4.00,  -81.9, 3.92 }, {  -88.0, 3.93,   65.8, 3.98,  -94.1, 3.86 }, {  -97.9, 3.70,   72.0, 4.18, -100.9, 3.50 } },
		{ {   -7.0, 3.98,    5.5, 4.01,   -7.0, 3.98 }, {  -21.0, 3.98,   15.7, 4.00,  -21.2, 3.99 }, {  -34.8, 3.98,   24.7, 4.00,  -34.0, 3.99 }, {  -49.8, 3.98,   34.7, 4.00,  -48.6, 3.98 }, {  -63.0, 3.96,   44.7, 4.00,  -61.3, 3.97 }, {  -76.8, 3.96,   54.4, 4.00,  -75.0, 3.96 }, {  -89.5, 3.93,   64.4, 4.00,  -88.0, 3.94 }, { -102.0, 3.70,   73.9, 4.14,  -99.4, 3.69 } },
		{ {   -9.5, 3.99,    7.0, 4.00,   -8.5, 3.99 }, {  -22.1, 3.98,   14.8, 4.00,  -19.2, 3.99 }, {  -35.7, 3.98,   24.7, 4.00,  -30.8, 3.98 }, {  -49.7, 3.98,   33.7, 4.00,  -42.9, 3.98 }, {  -62.9, 3.96,   43.6, 4.00,  -54.2, 3.96 }, {  -77.9, 3.96,   54.1, 3.98,  -68.1, 4.00 }, {  -91.9, 3.97,   63.7, 3.99,  -78.8, 3.95 }, { -104.8, 3.75,   74.7, 4.07,  -93.0, 3.85 } },
		{ {   -9.4, 3.99,    6.8, 4.00,   -7.4, 3.99 }, {  -22.1, 3.98,   14.6, 4.00,  -16.3, 3.99 }, {  -35.7, 3.98,   23.7, 4.00,  -27.0, 3.98 }, {  -49.9, 3.98,   33.8, 3.99,  -38.2, 3.99 }, {  -64.6, 3.98,   42.8, 4.00,  -48.9, 3.98 }, {  -78.4, 3.95,   52.7, 3.99,  -59.3, 3.97 }, {  -91.8, 3.93,   62.9, 3.97,  -70.0, 3.96 }, { -107.9, 3.78,   73.3, 4.07,  -82.2, 3.86 } },
		{ {   -9.4, 3.99,    5.8, 4.00,   -5.6, 3.99 }, {  -22.0, 3.98,   14.6, 4.00,  -14.3, 3.99 }, {  -35.8, 3.96,   23.5, 4.00,  -23.0, 3.98 }, {  -50.6, 3.98,   32.7, 4.00,  -33.0, 3.98 }, {  -65.3, 3.97,   41.7, 4.00,  -41.9, 3.98 }, {  -78.5, 3.93,   51.2, 4.01,  -50.7, 3.96 }, {  -93.6, 3.94,   61.0, 3.98,  -61.0, 3.99 }, { -103.5, 3.62,   68.6, 4.16,  -68.5, 3.80 } },
		{ {  -10.2, 3.99,    6.8, 4.00,   -5.5, 3.99 }, {  -22.9, 3.98,   13.7, 4.00,  -12.4, 3.99 }, {  -37.2, 3.99,   22.8, 4.00,  -19.7, 3.98 }, {  -51.5, 3.98,   31.6, 4.00,  -26.7, 3.97 }, {  -65.2, 3.96,   41.1, 3.99,  -35.3, 3.99 }, {  -80.7, 3.96,   50.2, 4.01,  -43.8, 3.98 }, {  -93.2, 3.90,   59.8, 3.99,  -51.6, 3.98 }, { -107.5, 3.71,   69.3, 4.09,  -59.6, 3.89 } },
		{ {   -8.7, 3.98,    5.6, 4.01,   -3.4, 3.99 }, {  -22.8, 3.98,   13.7, 4.00,   -9.6, 3.99 }, {  -36.9, 3.98,   22.7, 4.00,  -16.4, 3.99 }, {  -51.3, 3.97,   31.5, 4.00,  -21.9, 3.98 }, {  -66.3, 3.97,   40.7, 4.00,  -29.0, 3.98 }, {  -81.1, 3.97,   50.2, 3.98,  -36.1, 3.99 }, {  -95.8, 3.94,   57.9, 4.01,  -41.9, 3.98 }, { -108.7, 3.69,   67.3, 4.11,  -48.7, 3.89 } },
		{ {   -9.0, 3.97,    5.8, 4.00,   -3.7, 3.99 }, {  -23.1, 3.99,   13.9, 3.99,   -7.5, 3.99 }, {  -37.8, 3.98,   21.7, 4.00,  -12.5, 3.99 }, {  -52.8, 3.98,   30.8, 4.00,  -17.7, 3.99 }, {  -66.8, 3.96,   39.6, 4.00,  -22.3, 3.99 }, {  -81.6, 3.96,   48.9, 3.97,  -26.9, 3.97 }, {  -97.3, 3.96,   57.5, 3.97,  -33.1, 3.99 }, { -105.7, 3.58,   62.9, 4.19,  -35.9, 3.86 } },
		{ {   -8.7, 3.98,    5.4, 4.01,   -1.7, 3.99 }, {  -23.1, 3.99,   13.9, 3.98,   -5.6, 3.99 }, {  -37.4, 3.95,   21.5, 4.01,   -8.3, 3.98 }, {  -53.6, 3.98,   29.7, 4.00,  -12.6, 3.99 }, {  -68.4, 3.97,   38.8, 4.00,  -16.6, 3.99 }, {  -82.1, 3.94,   46.5, 4.00,  -19.2, 3.98 }, {  -97.1, 3.92,   55.8, 3.99,  -23.3, 3.99 }, { -108.6, 3.63,   63.5, 4.12,  -26.2, 3.92 } },
		{ {  -10.3, 3.99,    5.8, 4.00,   -1.6, 3.99 }, {  -23.9, 3.99,   12.8, 4.00,   -3.5, 3.99 }, {  -38.8, 3.98,   21.1, 3.99,   -5.6, 3.99 }, {  -53.4, 3.97,   29.4, 4.01,   -7.5, 3.99 }, {  -69.2, 3.97,   37.6, 4.00,   -9.7, 3.99 }, {  -83.8, 3.96,   45.7, 4.00,  -11.8, 3.99 }, {  -96.7, 3.89,   54.4, 4.00,  -14.7, 4.00 }, { -104.9, 3.50,   59.4, 4.19,  -15.9, 3.95 } },
		{ {  -10.2, 3.99,    5.7, 4.00,   -0.8, 4.00 }, {  -23.9, 3.98,   12.7, 4.00,   -0.9, 3.99 }, {  -38.8, 3.98,   20.7, 4.00,   -1.9, 4.00 }, {  -54.6, 3.98,   28.7, 4.00,   -2.6, 3.99 }, {  -70.1, 3.97,   37.3, 3.98,   -3.5, 3.99 }, {  -85.1, 3.97,   45.4, 3.98,   -3.9, 4.00 }, {  -99.2, 3.92,   52.8, 4.02,   -4.7, 3.99 }, { -107.4, 3.52,   57.1, 4.21,   -5.2, 3.97 } },
		{ {   -9.1, 3.96,    4.8, 4.00,    0.8, 3.98 }, {  -23.8, 3.98,   12.7, 4.00,    1.2, 3.99 }, {  -39.5, 3.98,   19.8, 4.00,    2.3, 3.99 }, {  -55.1, 3.97,   27.9, 3.99,    2.2, 3.99 }, {  -70.1, 3.95,   36.0, 3.99,    3.4, 3.99 }, {  -85.8, 3.94,   44.0, 3.99,    4.4, 3.99 }, { -100.0, 3.92,   52.1, 3.99,    4.3, 3.99 }, { -110.6, 3.58,   59.0, 4.12,    4.7, 4.01 } },
		{ {  -10.2, 3.98,    4.7, 4.00,    1.0, 4.00 }, {  -23.8, 3.98,   11.6, 4.00,    3.2, 3.99 }, {  -39.5, 3.97,   19.6, 4.00,    5.2, 3.99 }, {  -55.1, 3.96,   27.6, 4.00,    7.3, 3.99 }, {  -70.8, 3.96,   34.6, 4.00,   10.1, 3.98 }, {  -86.1, 3.94,   42.5, 4.00,   11.9, 4.00 }, {  -99.9, 3.88,   50.1, 4.01,   13.7, 4.00 }, { -106.5, 3.44,   54.9, 4.19,   14.3, 4.06 } },
		{ {  -10.1, 3.98,    4.8, 4.00,    2.0, 4.00 }, {  -23.8, 3.98,   11.7, 4.00,    5.1, 3.99 }, {  -39.5, 3.97,   18.6, 4.00,    8.9, 4.00 }, {  -55.1, 3.96,   26.6, 4.00,   12.0, 4.00 }, {  -71.1, 3.94,   33.5, 4.01,   15.7, 4.00 }, {  -86.0, 3.92,   41.2, 4.01,   20.0, 3.99 }, { -100.2, 3.88,   48.7, 4.02,   21.9, 4.02 }, { -109.1, 3.50,   55.9, 4.11,   22.5, 4.14 } },
		{ {  -10.3, 3.98,    4.8, 4.00,    3.0, 4.00 }, {  -23.7, 3.98,   10.7, 4.00,    7.7, 4.00 }, {  -40.2, 3.97,   18.6, 4.00,   12.7, 4.00 }, {  -55.6, 3.95,   25.5, 4.01,   17.8, 3.98 }, {  -70.8, 3.93,   32.6, 4.01,   21.6, 4.01 }, {  -89.0, 3.97,   41.1, 3.99,   26.9, 4.00 }, { -104.1, 3.95,   48.2, 3.99,   31.8, 4.00 }, { -111.2, 3.50,   53.0, 4.15,   34.4, 4.13 } },
		{ {   -8.5, 3.97,    3.6, 4.01,    3.6, 4.01 }, {  -24.4, 3.97,   10.7, 4.00,    9.7, 4.00 }, {  -40.1, 3.96,   17.7, 4.00,   15.8, 4.00 }, {  -57.2, 3.97,   24.7, 4.00,   22.5, 4.01 }, {  -73.5, 3.98,   32.7, 4.00,   28.8, 4.00 }, {  -89.3, 3.95,   39.7, 4.00,   35.2, 4.01 }, { -104.0, 3.90,   46.4, 4.00,   41.2, 4.01 }, { -107.7, 3.38,   49.7, 4.20,   42.2, 4.23 } },
		{ {  -12.6, 3.99,    5.2, 3.99,    6.2, 3.99 }, {  -24.7, 3.98,   10.8, 4.00,   12.5, 4.00 }, {  -40.9, 3.96,   17.6, 4.00,   19.7, 4.00 }, {  -57.5, 3.97,   24.6, 4.00,   27.7, 4.00 }, {  -73.8, 3.96,   31.6, 4.00,   35.7, 4.00 }, {  -90.5, 3.97,   39.2, 3.98,   45.0, 3.95 }, { -106.0, 3.93,   45.5, 4.00,   52.9, 3.95 }, { -113.3, 3.54,   51.0, 4.11,   58.8, 4.06 } },
		{ {  -11.1, 3.98,    4.8, 4.00,    5.9, 4.00 }, {  -24.7, 3.98,   10.0, 3.99,   14.6, 4.00 }, {  -41.8, 3.98,   16.8, 4.00,   23.6, 4.00 }, {  -58.5, 3.98,   23.8, 4.00,   33.8, 3.99 }, {  -75.0, 3.97,   30.9, 3.99,   43.4, 3.97 }, {  -90.9, 3.95,   37.8, 3.99,   53.5, 3.95 }, { -109.4, 4.00,   44.2, 3.98,   66.2, 3.85 }, { -118.7, 3.69,   48.5, 4.10,   77.4, 3.87 } },
		{ {  -10.2, 3.99,    3.9, 4.00,    6.7, 4.00 }, {  -25.5, 3.98,    9.8, 4.00,   17.4, 4.00 }, {  -42.7, 3.98,   16.7, 4.00,   28.6, 4.00 }, {  -59.4, 3.98,   23.0, 3.99,   40.9, 3.98 }, {  -75.4, 3.96,   29.7, 3.99,   51.9, 3.97 }, {  -94.8, 4.01,   36.6, 3.97,   67.0, 3.87 }, { -109.8, 3.97,   43.3, 3.97,   79.9, 3.81 }, { -120.7, 3.78,   48.0, 4.03,   96.3, 3.68 } },
		{ {  -11.2, 3.99,    3.8, 4.00,    8.6, 4.00 }, {  -25.4, 3.97,    9.7, 4.00,   20.3, 4.01 }, {  -43.7, 3.99,   15.9, 3.99,   34.6, 3.99 }, {  -60.1, 3.98,   21.9, 3.99,   48.5, 3.98 }, {  -78.8, 4.01,   28.0, 3.99,   63.1, 3.93 }, {  -95.2, 4.00,   34.5, 3.98,   79.2, 3.85 }, { -110.5, 3.97,   41.7, 3.95,   97.4, 3.68 }, { -130.3, 4.07,   48.6, 3.92,  131.7, 3.05 } },
		{ {   -9.7, 3.98,    3.9, 4.00,    9.3, 4.00 }, {  -26.4, 3.98,    8.9, 3.99,   25.1, 3.99 }, {  -43.8, 3.99,   14.9, 3.99,   41.5, 3.98 }, {  -60.8, 3.97,   20.9, 3.99,   57.5, 3.97 }, {  -80.2, 4.03,   27.4, 3.97,   77.1, 3.85 }, {  -96.0, 4.01,   32.2, 3.98,   95.5, 3.76 }, { -115.3, 4.07,   38.4, 3.98,  125.3, 3.30 }, { -118.8, 3.75,   50.4, 3.79,  155.9, 2.71 } },
		{ {  -11.5, 4.00,    4.0, 3.99,   12.7, 3.99 }, {  -27.3, 4.01,    8.0, 3.99,   30.6, 3.97 }, {  -45.0, 4.01,   14.1, 3.98,   50.1, 3.94 }, {  -63.3, 4.02,   19.4, 3.98,   71.4, 3.89 }, {  -81.0, 4.04,   25.7, 3.95,   93.1, 3.77 }, { -101.0, 4.11,   30.2, 3.98,  120.4, 3.48 }, { -112.3, 3.99,   37.4, 3.91,  151.4, 2.92 }, {  -96.3, 3.08,   37.3, 4.05,  168.8, 2.49 } },
		{ {  -11.4, 4.00,    3.1, 3.99,   14.8, 3.98 }, {  -25.7, 3.99,    7.0, 3.99,   33.1, 3.99 }, {  -42.0, 3.98,   12.0, 3.97,   54.3, 3.98 }, {  -57.1, 3.93,   15.4, 4.01,   75.5, 3.99 }, {  -71.1, 3.84,   19.4, 4.02,   98.3, 3.94 }, {  -84.1, 3.72,   20.9, 4.11,  127.3, 3.64 }, {  -57.6, 2.29,   10.3, 4.61,  164.1, 2.96 }, {  -73.8, 2.41,   24.1, 4.31,  181.6, 2.27 } },
		{ {   -9.2, 4.00,    2.1, 3.99,   14.2, 3.99 }, {  -23.4, 4.01,    5.1, 3.99,   34.8, 3.97 }, {  -37.4, 3.98,    8.0, 3.99,   57.0, 3.98 }, {  -52.2, 3.98,   12.0, 3.99,   80.0, 3.94 }, {  -63.3, 3.87,   14.0, 4.02,  102.5, 3.92 }, {  -74.7, 3.76,   15.7, 4.07,  129.4, 3.70 }, {  -67.8, 3.00,   11.1, 4.35,  162.1, 3.18 }, {  -51.2, 1.74,   11.0, 4.58,  194.5, 2.04 } },
		{ {   -8.4, 4.00,    1.2, 3.99,   15.5, 3.98 }, {  -20.3, 4.00,    3.2, 3.99,   35.8, 3.97 }, {  -32.7, 3.99,    6.0, 3.99,   58.7, 3.96 }, {  -44.8, 3.97,    7.9, 3.99,   82.3, 3.95 }, {  -57.6, 3.96,    9.8, 4.00,  108.7, 3.81 }, {  -63.9, 3.73,   10.1, 4.06,  130.9, 3.78 }, {  -65.9, 3.32,    8.4, 4.21,  164.7, 3.25 }, {  -28.7, 1.07,   -2.2, 4.84,  207.3, 1.82 } },
		{ {   -7.6, 4.00,    1.2, 3.99,   15.7, 3.97 }, {  -17.8, 4.01,    2.2, 3.99,   38.4, 3.93 }, {  -28.1, 4.00,    3.3, 3.99,   61.0, 3.95 }, {  -38.9, 3.99,    4.1, 3.99,   85.8, 3.90 }, {  -50.1, 3.97,    4.9, 4.00,  111.1, 3.83 }, {  -57.0, 3.82,    5.1, 4.03,  135.8, 3.74 }, {  -60.7, 3.52,    3.5, 4.14,  165.6, 3.37 }, {   -6.2, 0.40,  -15.3, 5.10,  220.2, 1.60 } },
		{ {   -6.6, 4.00,    0.3, 3.99,   17.2, 3.96 }, {  -14.6, 4.00,    0.3, 3.99,   38.2, 3.95 }, {  -24.8, 4.01,    0.1, 4.00,   63.7, 3.93 }, {  -33.4, 4.00,    0.1, 4.00,   87.9, 3.91 }, {  -42.3, 3.97,    1.0, 3.98,  113.1, 3.86 }, {  -49.6, 3.87,   -0.0, 4.04,  139.5, 3.74 }, {  -51.7, 3.55,   -2.0, 4.13,  166.4, 3.50 }, {  -30.6, 1.94,  -11.3, 4.63,  208.0, 2.29 } },
		{ {   -4.9, 4.00,   -0.8, 3.99,   17.2, 3.96 }, {  -12.9, 4.01,   -0.9, 4.00,   40.6, 3.94 }, {  -20.7, 4.01,   -1.9, 3.99,   65.0, 3.92 }, {  -28.7, 4.01,   -3.0, 4.00,   91.7, 3.86 }, {  -35.2, 3.96,   -3.8, 3.99,  115.5, 3.89 }, {  -42.2, 3.93,   -5.6, 4.04,  142.4, 3.75 }, {  -46.2, 3.74,   -7.0, 4.09,  171.4, 3.45 }, {  -36.8, 2.71,  -15.3, 4.55,  206.0, 2.55 } },
		{ {   -3.7, 3.99,   -0.8, 3.99,   18.6, 3.94 }, {  -10.1, 4.01,   -3.0, 4.00,   41.1, 3.93 }, {  -16.9, 4.01,   -3.9, 4.00,   67.0, 3.91 }, {  -22.9, 4.01,   -5.9, 4.00,   93.6, 3.87 }, {  -29.3, 4.00,   -8.1, 4.01,  118.9, 3.83 }, {  -33.7, 3.92,  -10.4, 4.03,  144.7, 3.79 }, {  -38.9, 3.84,  -12.3, 4.07,  175.5, 3.41 }, {  -32.9, 3.03,  -16.6, 4.28,  204.5, 2.84 } },
		{ {   -2.9, 3.98,   -1.8, 4.00,   18.4, 3.96 }, {   -7.9, 4.00,   -3.9, 4.00,   41.9, 3.93 }, {  -12.7, 4.00,   -6.8, 4.00,   67.5, 3.93 }, {  -17.3, 3.99,   -8.8, 4.00,   95.0, 3.88 }, {  -21.8, 3.97,  -12.0, 4.01,  120.9, 3.86 }, {  -26.8, 3.95,  -14.4, 4.03,  150.2, 3.64 }, {  -30.1, 3.86,  -17.8, 4.07,  178.7, 3.40 }, {  -30.1, 3.46,  -22.5, 4.28,  212.4, 2.53 } },
		{ {   -2.0, 4.00,   -2.0, 4.00,   17.0, 3.94 }, {   -6.0, 4.01,   -5.0, 4.00,   44.4, 3.91 }, {  -10.0, 4.01,   -9.0, 4.01,   70.6, 3.89 }, {  -12.9, 4.00,  -12.2, 4.01,   97.5, 3.83 }, {  -16.9, 4.00,  -15.1, 4.01,  125.1, 3.76 }, {  -19.8, 3.96,  -19.1, 4.03,  151.0, 3.73 }, {  -22.2, 3.90,  -22.0, 4.03,  179.2, 3.54 }, {  -23.2, 3.61,  -26.9, 4.20,  212.9, 2.67 } },
		{ {   -1.8, 4.00,   -2.8, 4.00,   20.3, 3.98 }, {   -3.9, 4.00,   -6.0, 4.00,   43.2, 3.92 }, {   -5.9, 4.00,  -11.0, 4.01,   71.5, 3.89 }, {   -7.8, 3.99,  -15.0, 4.01,   98.8, 3.86 }, {  -11.1, 4.01,  -19.0, 4.02,  127.3, 3.79 }, {  -12.6, 3.97,  -23.0, 4.03,  156.1, 3.60 }, {  -14.6, 3.95,  -26.9, 4.03,  186.5, 3.26 }, {  -17.6, 3.92,  -30.2, 4.04,  216.1, 2.58 } },
		{ {   -0.8, 4.00,   -3.1, 3.99,   19.5, 3.96 }, {   -0.9, 3.99,   -8.0, 4.01,   45.1, 3.91 }, {   -1.8, 3.99,  -12.8, 4.01,   72.0, 3.90 }, {   -2.9, 3.99,  -17.5, 4.00,   99.9, 3.87 }, {   -3.7, 3.98,  -23.1, 4.03,  130.0, 3.74 }, {   -4.7, 3.97,  -27.9, 4.04,  159.0, 3.59 }, {   -5.9, 3.96,  -32.3, 4.04,  188.6, 3.27 }, {   -6.1, 3.81,  -33.9, 3.88,  213.6, 2.93 } },
		{ {    1.3, 3.97,   -4.0, 4.01,   19.8, 3.94 }, {    1.2, 3.99,   -8.8, 4.00,   45.8, 3.91 }, {    2.2, 3.99,  -15.0, 4.01,   74.3, 3.89 }, {    3.2, 3.98,  -20.7, 4.00,  102.1, 3.87 }, {    3.1, 3.99,  -27.0, 4.02,  133.1, 3.73 }, {    4.3, 3.96,  -32.8, 4.04,  162.5, 3.58 }, {    4.6, 3.93,  -36.3, 3.94,  190.0, 3.36 }, {   10.8, 3.30,  -36.4, 3.60,  225.0, 2.20 } },
		{ {    2.3, 3.99,   -5.0, 4.01,   20.7, 3.94 }, {    4.3, 3.99,  -11.0, 4.01,   47.2, 3.91 }, {    6.2, 3.99,  -17.7, 4.01,   75.7, 3.90 }, {    9.3, 3.97,  -24.8, 4.02,  105.9, 3.82 }, {   11.7, 3.94,  -32.4, 4.06,  137.0, 3.66 }, {   13.8, 3.96,  -37.6, 4.00,  164.4, 3.62 }, {   17.1, 3.82,  -43.9, 4.05,  197.5, 3.06 }, {   24.3, 3.00,  -22.3, 2.00,  223.0, 2.50 } },
		{ {    3.2, 3.99,   -5.9, 4.01,   20.6, 3.94 }, {    7.3, 3.98,  -13.0, 4.02,   47.9, 3.91 }, {   11.3, 3.98,  -21.0, 4.02,   77.6, 3.87 }, {   16.1, 3.97,  -29.0, 4.03,  107.8, 3.82 }, {   20.9, 3.92,  -37.0, 4.05,  139.7, 3.66 }, {   26.1, 3.86,  -44.6, 4.06,  171.9, 3.40 }, {   26.8, 3.90,  -43.6, 3.65,  194.4, 3.38 }, {   39.2, 3.00,  -31.2, 2.30,  222.3, 2.35 } },
		{ {    4.2, 3.99,   -5.8, 4.01,   20.5, 3.94 }, {   10.2, 3.98,  -14.8, 4.01,   48.5, 3.92 }, {   17.0, 3.98,  -23.4, 4.01,   78.6, 3.91 }, {   23.8, 3.94,  -32.9, 4.03,  111.5, 3.78 }, {   30.5, 3.92,  -42.5, 4.05,  142.7, 3.68 }, {   35.2, 3.95,  -49.3, 3.99,  173.1, 3.50 }, {   43.5, 3.57,  -47.4, 3.49,  202.4, 3.03 }, {   54.2, 3.00,  -40.1, 2.60,  221.5, 2.20 } },
		{ {    6.4, 3.98,   -8.1, 4.02,   22.8, 3.94 }, {   14.5, 3.96,  -17.2, 4.03,   51.4, 3.88 }, {   23.3, 3.96,  -27.7, 4.03,   81.8, 3.87 }, {   32.7, 3.92,  -37.9, 4.04,  114.3, 3.78 }, {   40.7, 3.92,  -47.5, 4.02,  145.2, 3.71 }, {   49.8, 3.81,  -55.3, 3.95,  179.8, 3.31 }, {   49.0, 4.00,  -15.0, 1.00,  222.0, 2.00 }, {   69.1, 3.00,  -49.0, 2.90,  220.8, 2.05 } },
		{ {    7.1, 3.98,   -7.8, 4.02,   21.0, 3.94 }, {   18.4, 3.96,  -19.0, 4.03,   51.8, 3.88 }, {   29.3, 3.96,  -30.9, 4.03,   83.5, 3.84 }, {   41.5, 3.93,  -43.1, 4.06,  117.0, 3.73 }, {   51.5, 3.94,  -54.1, 4.04,  152.2, 3.46 }, {   56.5, 4.18,  -61.0, 3.89,  193.1, 2.63 }, {   67.0, 3.82,  -43.1, 2.47,  213.3, 2.10 }, {   84.1, 3.00,  -57.9, 3.20,  220.1, 1.90 } },
		{ {    8.2, 3.98,   -7.6, 4.01,   19.0, 3.96 }, {   19.4, 3.96,  -18.9, 4.02,   45.6, 3.92 }, {   32.6, 3.94,  -30.0, 4.03,   74.4, 3.87 }, {   44.7, 3.93,  -42.2, 4.05,  104.5, 3.77 }, {   58.6, 3.85,  -55.0, 4.11,  137.9, 3.45 }, {   71.6, 3.77,  -67.3, 4.19,  173.7, 2.90 }, {   85.1, 3.63,  -71.2, 3.94,  204.7, 2.20 }, {   99.0, 3.00,  -66.8, 3.50,  219.3, 1.75 } },
		{ {    9.1, 3.97,   -7.7, 4.01,   16.7, 3.97 }, {   20.3, 3.97,  -17.9, 4.02,   39.5, 3.93 }, {   33.7, 3.94,  -29.2, 4.03,   64.2, 3.90 }, {   47.1, 3.92,  -40.4, 4.02,   90.6, 3.82 }, {   59.2, 3.91,  -51.5, 4.04,  115.1, 3.77 }, {   72.8, 3.86,  -63.1, 4.08,  143.9, 3.53 }, {   84.9, 3.84,  -73.9, 4.11,  176.0, 2.97 }, {  114.0, 3.00,  -75.7, 3.80,  218.6, 1.60 } },
		{ {    8.9, 3.99,   -6.5, 3.98,   14.7, 3.98 }, {   21.2, 3.97,  -16.8, 4.01,   34.1, 3.97 }, {   34.0, 3.97,  -27.3, 4.00,   55.7, 3.96 }, {   47.4, 3.95,  -38.6, 4.01,   78.7, 3.91 }, {   61.0, 3.92,  -50.0, 4.04,  102.9, 3.79 }, {   75.3, 3.85,  -62.0, 4.09,  127.6, 3.61 }, {   88.1, 3.86,  -69.7, 4.00,  147.4, 3.59 }, {  102.5, 3.73,  -79.2, 4.00,  178.8, 2.92 } },
		{ {    8.9, 3.99,   -6.6, 4.00,   12.8, 3.99 }, {   21.2, 3.97,  -16.8, 4.01,   30.3, 3.96 }, {   34.9, 3.98,  -26.5, 4.00,   50.0, 3.95 }, {   48.5, 3.95,  -37.8, 4.02,   70.0, 3.91 }, {   61.7, 3.95,  -49.3, 4.04,   89.9, 3.87 }, {   76.3, 3.90,  -58.5, 4.02,  111.0, 3.78 }, {   91.3, 3.84,  -70.4, 4.07,  134.4, 3.58 }, {  109.5, 3.64,  -84.2, 4.21,  166.1, 3.01 } },
		{ {    9.2, 3.98,   -6.9, 4.00,   12.2, 3.98 }, {   22.2, 3.97,  -15.9, 4.00,   27.0, 3.98 }, {   34.9, 3.98,  -25.6, 4.00,   43.9, 3.98 }, {   49.2, 3.97,  -36.7, 4.01,   61.5, 3.95 }, {   63.0, 3.96,  -46.3, 4.00,   79.3, 3.93 }, {   76.9, 3.94,  -57.8, 4.03,   98.3, 3.85 }, {   92.4, 3.87,  -68.4, 4.05,  118.6, 3.70 }, {  112.7, 3.69,  -82.8, 4.16,  145.7, 3.37 } },
		{ {   11.2, 3.99,   -7.7, 4.00,   12.1, 3.99 }, {   22.1, 3.98,  -15.8, 4.00,   24.2, 3.98 }, {   35.9, 3.98,  -25.6, 4.00,   39.6, 3.97 }, {   49.7, 3.96,  -35.9, 4.01,   55.8, 3.94 }, {   64.3, 3.96,  -45.5, 4.00,   71.8, 3.93 }, {   77.8, 3.95,  -56.0, 4.02,   87.8, 3.89 }, {   92.3, 3.95,  -66.1, 4.01,  102.2, 3.90 }, {  119.2, 3.62,  -85.3, 4.23,  135.8, 3.40 } },
		{ {    9.4, 4.00,   -6.6, 4.00,    9.9, 4.00 }, {   27.1, 4.00,  -18.8, 3.99,   28.1, 4.00 }, {   35.0, 3.99,  -24.5, 3.99,   36.0, 3.99 }, {   51.4, 3.96,  -36.0, 4.01,   52.4, 3.96 }, {   64.0, 3.98,  -44.0, 3.97,   66.2, 3.96 }, {   80.0, 3.93,  -56.1, 4.02,   81.1, 3.93 }, {   92.4, 3.95,  -64.0, 3.99,   94.4, 3.95 }, {  122.5, 3.61,  -84.3, 4.19,  125.3, 3.59 } },
		{ {    7.7, 4.00,   -5.5, 4.00,    7.7, 4.00 }, {   22.0, 3.98,  -14.8, 4.00,   21.1, 3.98 }, {   36.1, 3.98,  -24.6, 4.00,   35.1, 3.98 }, {   49.8, 3.99,  -34.2, 3.99,   48.6, 3.99 }, {   64.9, 3.98,  -44.3, 3.99,   63.1, 3.96 }, {   78.9, 3.97,  -53.7, 3.98,   75.5, 3.99 }, {   93.2, 3.97,  -64.0, 3.99,   90.2, 3.96 }, {  118.3, 3.79,  -79.7, 4.04,  113.3, 3.82 } },
		{ {   10.0, 4.00,   -6.8, 4.00,    8.9, 4.00 }, {   23.2, 3.98,  -14.9, 4.00,   20.1, 3.98 }, {   36.3, 3.97,  -24.7, 4.00,   32.2, 3.98 }, {   50.8, 3.98,  -33.4, 3.99,   43.8, 3.99 }, {   65.1, 3.98,  -43.6, 4.00,   56.2, 3.98 }, {   80.4, 3.96,  -53.3, 3.99,   69.0, 3.97 }, {   94.8, 3.94,  -63.5, 4.00,   81.4, 3.97 }, {  122.2, 3.69,  -80.6, 4.12,  104.3, 3.80 } },
		{ {   10.1, 3.99,   -6.9, 4.00,    8.1, 3.99 }, {   22.8, 3.99,  -14.4, 3.99,   17.0, 3.99 }, {   37.0, 3.99,  -23.7, 4.00,   28.0, 3.99 }, {   51.0, 3.98,  -33.4, 3.99,   39.1, 3.98 }, {   66.4, 3.97,  -42.4, 3.98,   50.3, 3.97 }, {   81.0, 3.97,  -52.1, 3.99,   60.7, 3.99 }, {   95.3, 3.96,  -61.6, 3.98,   71.3, 4.00 }, {  124.2, 3.69,  -79.8, 4.13,   93.0, 3.79 } },
		{ {    9.9, 3.99,   -5.6, 3.99,    6.0, 3.99 }, {   22.8, 3.99,  -14.5, 4.00,   15.0, 3.99 }, {   37.0, 3.99,  -23.8, 4.00,   24.0, 3.99 }, {   52.2, 3.97,  -32.6, 4.00,   34.4, 3.96 }, {   66.8, 3.98,  -41.3, 3.99,   42.9, 3.99 }, {   82.3, 3.96,  -51.7, 4.00,   53.1, 3.98 }, {   97.3, 3.92,  -60.4, 4.00,   62.3, 3.97 }, {  123.6, 3.75,  -75.7, 4.05,   78.4, 3.89 } },
		{ {   11.2, 3.98,   -6.9, 4.00,    6.2, 3.99 }, {   23.9, 3.97,  -14.1, 3.99,   13.0, 3.99 }, {   38.2, 3.98,  -22.8, 4.00,   20.1, 3.99 }, {   53.5, 3.96,  -31.7, 4.00,   28.3, 3.98 }, {   67.3, 3.97,  -41.3, 3.99,   36.2, 3.98 }, {   82.0, 3.97,  -49.8, 3.98,   44.5, 4.00 }, {   98.6, 3.95,  -59.5, 4.00,   53.2, 3.97 }, {  128.1, 3.63,  -76.7, 4.14,   68.1, 3.85 } },
		{ {    9.8, 3.99,   -5.6, 3.99,    4.0, 3.99 }, {   23.9, 3.99,  -13.6, 4.00,   10.2, 3.99 }, {   38.2, 3.98,  -22.9, 4.01,   17.2, 3.99 }, {   53.1, 3.98,  -31.7, 4.00,   23.1, 3.99 }, {   68.5, 3.96,  -40.6, 4.00,   30.2, 3.98 }, {   83.3, 3.96,  -49.3, 3.99,   36.1, 3.99 }, {   98.3, 3.96,  -57.5, 3.97,   42.3, 4.01 }, {  129.6, 3.64,  -75.9, 4.16,   56.8, 3.83 } },
		{ {   10.5, 3.98,   -5.6, 3.99,    4.3, 3.97 }, {   24.0, 3.98,  -13.6, 4.00,    8.2, 3.99 }, {   38.8, 3.99,  -21.4, 3.99,   13.1, 3.99 }, {   54.3, 3.97,  -30.8, 4.00,   18.2, 3.99 }, {   69.2, 3.97,  -39.6, 4.00,   23.0, 3.99 }, {   84.8, 3.94,  -48.5, 4.00,   28.3, 3.98 }, {  100.8, 3.93,  -56.2, 3.99,   34.3, 3.97 }, {  128.9, 3.69,  -72.3, 4.11,   42.5, 3.93 } },
		{ {    9.8, 3.99,   -5.6, 4.00,    2.1, 3.99 }, {   24.1, 3.98,  -13.4, 4.00,    6.2, 3.99 }, {   39.0, 3.98,  -21.6, 4.00,    9.2, 3.99 }, {   54.9, 3.98,  -29.2, 3.97,   13.1, 3.99 }, {   70.4, 3.97,  -38.8, 4.00,   17.3, 3.98 }, {   85.4, 3.96,  -47.6, 4.00,   20.3, 3.99 }, {  102.6, 3.90,  -55.8, 4.01,   24.2, 3.98 }, {  133.4, 3.57,  -72.4, 4.16,   31.8, 3.90 } },
		{ {   11.3, 3.98,   -6.0, 4.00,    2.2, 3.99 }, {   25.1, 3.98,  -12.8, 4.00,    4.4, 3.98 }, {   40.0, 3.98,  -20.4, 3.98,    6.2, 3.99 }, {   55.1, 3.98,  -29.5, 4.00,    8.2, 3.99 }, {   71.0, 3.97,  -37.3, 3.99,   10.3, 3.99 }, {   86.1, 3.97,  -44.9, 3.98,   11.9, 4.00 }, {  102.9, 3.93,  -54.9, 4.02,   15.3, 3.98 }, {  135.9, 3.55,  -71.4, 4.17,   19.3, 3.94 } },
		{ {   10.9, 3.99,   -5.7, 4.00,    1.2, 3.99 }, {   25.1, 3.98,  -12.7, 4.00,    1.2, 3.99 }, {   40.1, 3.98,  -20.7, 4.00,    2.2, 3.99 }, {   56.1, 3.97,  -28.7, 4.00,    3.3, 3.99 }, {   72.3, 3.96,  -36.6, 4.00,    4.3, 3.98 }, {   87.5, 3.95,  -44.4, 3.99,    4.3, 3.99 }, {  103.6, 3.94,  -53.1, 3.99,    5.1, 3.99 }, {  134.9, 3.62,  -68.1, 4.12,    6.2, 4.00 } },
		{ {    9.9, 3.99,   -4.7, 3.98,   -0.8, 3.99 }, {   25.2, 3.97,  -12.7, 4.00,   -0.9, 4.00 }, {   41.2, 3.97,  -19.6, 3.99,   -1.7, 3.99 }, {   57.3, 3.96,  -27.5, 3.98,   -1.7, 3.99 }, {   72.5, 3.96,  -35.7, 4.00,   -2.8, 3.99 }, {   88.7, 3.94,  -43.7, 4.00,   -3.7, 3.99 }, {  105.1, 3.92,  -51.6, 4.00,   -3.9, 3.99 }, {  139.1, 3.51,  -68.2, 4.18,   -5.5, 4.01 } },
		{ {   10.9, 3.99,   -4.7, 4.00,   -0.6, 3.98 }, {   25.2, 3.97,  -11.9, 4.00,   -2.8, 3.99 }, {   41.3, 3.97,  -19.8, 4.00,   -4.9, 4.00 }, {   57.6, 3.96,  -27.8, 4.01,   -6.5, 3.97 }, {   73.9, 3.94,  -35.3, 4.00,   -9.8, 4.00 }, {   89.4, 3.96,  -42.7, 4.00,  -12.0, 4.00 }, {  105.7, 3.94,  -50.8, 4.01,  -13.9, 4.00 }, {  141.5, 3.48,  -67.1, 4.18,  -18.5, 4.06 } },
		{ {   11.0, 3.99,   -4.7, 4.00,   -1.8, 3.99 }, {   25.3, 3.97,  -11.8, 4.00,   -4.8, 3.98 }, {   41.5, 3.96,  -18.9, 4.01,   -8.8, 4.00 }, {   57.8, 3.95,  -26.9, 4.01,  -13.1, 4.01 }, {   74.5, 3.96,  -33.9, 4.00,  -16.0, 4.00 }, {   89.9, 3.97,  -41.1, 3.98,  -19.6, 3.99 }, {  106.2, 3.96,  -49.1, 3.99,  -23.0, 4.01 }, {  139.7, 3.57,  -63.7, 4.12,  -30.7, 4.10 } },
		{ {   11.2, 3.99,   -4.9, 4.00,   -2.9, 4.00 }, {   25.3, 3.97,  -10.9, 4.00,   -8.0, 4.00 }, {   42.7, 3.95,  -18.9, 4.01,  -13.0, 4.01 }, {   58.4, 3.97,  -25.9, 4.00,  -17.9, 4.00 }, {   74.0, 3.98,  -33.4, 4.00,  -22.6, 4.00 }, {   91.2, 3.96,  -40.5, 3.99,  -27.5, 3.99 }, {  107.9, 3.93,  -47.4, 3.99,  -32.8, 4.01 }, {  144.5, 3.43,  -63.8, 4.19,  -43.4, 4.15 } },
		{ {    9.9, 3.99,   -3.8, 4.00,   -3.8, 4.00 }, {   26.5, 3.96,  -10.9, 4.00,  -10.0, 4.00 }, {   42.3, 3.97,  -17.9, 4.00,  -16.0, 4.00 }, {   58.9, 3.98,  -24.5, 3.99,  -22.6, 4.00 }, {   75.3, 3.96,  -32.7, 4.00,  -28.7, 4.00 }, {   92.8, 3.93,  -39.7, 4.00,  -35.9, 4.01 }, {  110.8, 3.88,  -46.8, 4.01,  -42.0, 4.02 }, {  149.5, 3.28,  -63.4, 4.24,  -57.8, 4.27 } },
		{ {   13.1, 3.99,   -4.8, 4.00,   -5.8, 4.00 }, {   26.4, 3.97,  -10.9, 4.00,  -12.8, 4.00 }, {   43.1, 3.98,  -17.8, 4.00,  -19.8, 4.00 }, {   59.4, 3.96,  -24.8, 4.00,  -27.8, 4.00 }, {   76.1, 3.96,  -31.5, 4.00,  -34.7, 3.98 }, {   93.2, 3.95,  -38.6, 4.00,  -42.3, 3.96 }, {  111.2, 3.89,  -45.3, 3.99,  -50.5, 3.98 }, {  141.6, 3.61,  -58.4, 4.12,  -61.8, 4.00 } },
		{ {   12.3, 3.98,   -4.9, 4.00,   -6.4, 3.99 }, {   26.0, 3.97,  -10.6, 4.00,  -14.6, 4.00 }, {   42.5, 3.99,  -16.5, 3.99,  -23.2, 3.99 }, {   60.2, 3.96,  -23.8, 4.00,  -33.3, 4.00 }, {   77.3, 3.95,  -30.6, 4.00,  -41.9, 3.98 }, {   93.3, 3.98,  -37.0, 3.98,  -50.5, 3.94 }, {  110.4, 3.97,  -43.7, 3.98,  -58.5, 3.89 }, {  134.4, 3.83,  -52.4, 4.00,  -66.8, 3.79 } },
		{ {   11.1, 3.98,   -3.9, 4.00,   -6.9, 4.00 }, {   26.8, 3.99,   -9.7, 4.00,  -17.5, 4.00 }, {   43.5, 3.99,  -16.3, 3.99,  -28.0, 3.99 }, {   60.7, 3.97,  -22.5, 3.99,  -39.7, 3.98 }, {   77.5, 3.98,  -29.4, 3.99,  -50.3, 3.97 }, {   94.4, 4.00,  -35.1, 3.98,  -59.3, 3.89 }, {  112.6, 3.94,  -42.3, 4.00,  -69.1, 3.85 }, {  137.5, 3.67,  -50.7, 4.05,  -79.3, 3.81 } },
		{ {   11.5, 3.99,   -3.6, 3.99,   -8.2, 3.99 }, {   26.3, 4.00,   -9.5, 3.99,  -20.0, 3.99 }, {   43.3, 3.99,  -15.4, 3.99,  -33.3, 3.98 }, {   61.9, 3.96,  -21.7, 4.00,  -47.8, 4.00 }, {   78.1, 3.99,  -27.4, 3.99,  -58.7, 3.93 }, {   95.9, 3.97,  -34.0, 3.99,  -70.6, 3.88 }, {  114.0, 3.90,  -38.7, 3.95,  -79.8, 3.77 }, {  128.3, 3.98,  -44.9, 3.96,  -74.4, 3.16 } },
		{ {   11.0, 3.99,   -3.5, 3.99,   -9.8, 3.99 }, {   27.2, 4.00,   -8.4, 3.99,  -24.8, 3.99 }, {   44.6, 3.98,  -14.4, 3.99,  -40.5, 3.99 }, {   62.1, 4.00,  -20.4, 3.99,  -55.7, 3.96 }, {   79.0, 4.01,  -25.3, 3.96,  -68.8, 3.87 }, {   96.9, 3.96,  -30.8, 3.96,  -82.5, 3.84 }, {  112.1, 4.01,  -36.8, 3.98,  -84.3, 3.41 }, {  133.1, 3.79,  -33.9, 3.69,  -73.8, 2.73 } },
		{ {   11.5, 4.00,   -3.5, 3.99,  -12.2, 4.00 }, {   27.2, 4.00,   -7.6, 3.98,  -29.3, 3.98 }, {   44.8, 4.00,  -13.1, 3.99,  -47.1, 3.96 }, {   62.6, 4.00,  -18.2, 3.98,  -65.1, 3.91 }, {   79.5, 4.01,  -23.1, 3.96,  -79.9, 3.82 }, {   95.1, 4.07,  -29.4, 4.00,  -89.3, 3.55 }, {  117.0, 3.85,  -30.2, 3.86,  -90.7, 3.17 }, {  151.0, 3.17,  -37.2, 3.95,  -75.9, 2.58 } },
		{ {   11.3, 4.00,   -2.7, 3.99,  -13.7, 3.98 }, {   26.0, 4.00,   -6.4, 3.99,  -31.8, 3.98 }, {   43.6, 3.97,  -11.3, 3.99,  -54.0, 3.99 }, {   61.5, 3.93,  -15.4, 4.00,  -73.5, 3.96 }, {   79.3, 3.90,  -21.2, 4.03,  -93.4, 3.91 }, {  103.9, 3.66,  -27.6, 4.10, -107.7, 3.74 }, {  169.4, 2.20,  -46.9, 4.55,  -92.8, 2.82 }, {  169.0, 2.55,  -40.5, 4.21,  -78.1, 2.44 } },
		{ {    8.7, 4.02,   -1.5, 3.99,  -12.7, 3.96 }, {   23.0, 4.00,   -4.6, 3.99,  -33.9, 3.98 }, {   37.5, 4.01,   -7.4, 3.97,  -54.3, 3.94 }, {   53.8, 3.96,  -11.5, 4.00,  -76.4, 3.95 }, {   69.3, 3.94,  -14.9, 4.01,  -94.1, 3.83 }, {   91.0, 3.73,  -20.0, 4.07, -112.8, 3.76 }, {  135.0, 2.90,  -33.5, 4.35, -113.1, 3.27 }, {  186.9, 1.94,  -43.9, 4.48,  -80.2, 2.29 } },
		{ {    8.2, 4.01,   -0.6, 3.98,  -14.3, 3.97 }, {   20.2, 4.00,   -2.7, 3.99,  -34.2, 3.96 }, {   32.9, 4.01,   -5.3, 3.99,  -55.9, 3.93 }, {   45.8, 4.00,   -7.1, 3.98,  -77.0, 3.89 }, {   60.6, 3.95,   -9.8, 4.00,  -97.4, 3.83 }, {   77.6, 3.82,  -13.0, 4.04, -115.4, 3.72 }, {  110.0, 3.30,  -21.8, 4.21, -122.5, 3.39 }, {  204.9, 1.32,  -47.2, 4.74,  -82.4, 2.15 } },
		{ {    7.1, 4.00,   -0.8, 3.99,  -14.1, 3.97 }, {   17.1, 4.01,   -1.5, 3.99,  -35.1, 3.95 }, {   27.9, 4.01,   -2.5, 3.99,  -57.3, 3.93 }, {   39.7, 3.99,   -3.6, 3.99,  -80.6, 3.93 }, {   52.1, 3.96,   -5.1, 4.01, -101.6, 3.86 }, {   68.5, 3.81,   -7.2, 4.03, -122.2, 3.81 }, {   93.2, 3.47,  -11.9, 4.13, -129.7, 3.47 }, {  222.8, 0.70,  -50.5, 5.00,  -84.5, 2.00 } },
	};

	int h = hsl & 127;
	int s = (hsl >> 7) & 7;
	int l = hsl >> 10;

	if(h >= 88)
		h -= 88;

	const auto &cinf = color_bases[h][s];
	static auto conv = [](float i, float s, float l) -> uint8_t { int c = int(i + l*s + 0.5); if(c < 0) c = 0; if(c > 255) c = 255; return c; };

	return std::make_tuple(conv(cinf.ri, cinf.rs, l), conv(cinf.gi, cinf.gs, l), conv(cinf.bi, cinf.bs, l));
}

void spg110_video_device::device_start()
{
	save_item(NAME(m_dma_src_step));
	save_item(NAME(m_dma_dst_step));
	save_item(NAME(m_dma_dst_seg));
	save_item(NAME(m_dma_src_seg));
	save_item(NAME(m_dma_dst));
	save_item(NAME(m_dma_src));
	save_item(NAME(m_bg_scrollx));
	save_item(NAME(m_bg_scrolly));
	save_item(NAME(m_tm_v_2036));
	save_item(NAME(m_tm_h_2037));

	save_item(NAME(m_blk_irq_enable));
	save_item(NAME(m_blk_irq_flag));
	save_item(NAME(m_dma_irq_enable));
	save_item(NAME(m_dma_irq_flag));
	save_item(NAME(m_vdo_irq_enable));
	save_item(NAME(m_vdo_irq_flag));

	save_item(NAME(m_dma_busy));

	m_screenpos_timer = timer_alloc(FUNC(spg110_video_device::screenpos_hit), this);
	m_screenpos_timer->adjust(attotime::never);

	m_dma_timer = timer_alloc(FUNC(spg110_video_device::dma_done), this);
	m_dma_timer->adjust(attotime::never);
}

void spg110_video_device::device_reset()
{
	m_dma_src_step = 0;
	m_dma_dst_step = 0;
	m_dma_dst_seg = 0;
	m_dma_src_seg = 0;
	m_dma_dst = 0;
	m_dma_src = 0;
	m_bg_scrollx = 0;
	m_bg_scrolly = 0;
	m_tm_v_2036 = 0xffff;
	m_tm_h_2037 = 0xffff;

	std::fill(std::begin(tmap0_regs), std::end(tmap0_regs), 0);
	std::fill(std::begin(tmap1_regs), std::end(tmap1_regs), 0);

	// is there actually an enable register here?
	m_video_irq_enable = 0xffff;
	m_video_irq_status = 0x0000;

	m_blk_irq_enable = 0;
	m_blk_irq_flag = 0;
	m_dma_irq_enable = 0;
	m_dma_irq_flag = 0;
	m_vdo_irq_enable = 0;
	m_vdo_irq_flag = 0;

	m_dma_busy = 0;
}

TIMER_CALLBACK_MEMBER(spg110_video_device::dma_done)
{
	m_dma_irq_flag = 1;
	m_dma_busy = 0;
	update_video_irqs();
	m_dma_timer->adjust(attotime::never);
}

void spg110_video_device::update_raster_interrupt_timer()
{
	if (m_tm_h_2037 < 320 && m_tm_v_2036 < 240)
	{
		m_screenpos_timer->adjust(m_screen->time_until_pos(m_tm_v_2036, m_tm_h_2037));

	}
	else
		m_screenpos_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(spg110_video_device::screenpos_hit)
{
	m_vdo_irq_flag = 1;
	update_video_irqs();

	m_screen->update_partial(m_screen->vpos());

	// fire again (based on spg2xx logic)
	m_screenpos_timer->adjust(m_screen->time_until_pos(m_tm_v_2036, m_tm_h_2037));
}

void spg110_video_device::palette_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	auto rgb = hsl_to_rgb(data);
	m_palette->set_pen_color(offset, std::get<0>(rgb), std::get<1>(rgb), std::get<2>(rgb));
}

uint32_t spg110_video_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *pens = m_palette->pens();
	const uint32_t page1_addr = 0x40 * m_tilebase[0];
	const uint32_t page2_addr = 0x40 * m_tilebase[0];

	for (uint32_t scanline = (uint32_t)cliprect.min_y; scanline <= (uint32_t)cliprect.max_y; scanline++)
	{
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
			m_screenbuf[(320 * scanline) + x] = pens[0];

		for (int i = 0; i < 4; i++)
		{
			draw_page(cliprect, scanline, i, page1_addr, tmap0_regs);
			draw_page(cliprect, scanline, i, page2_addr, tmap1_regs);
			draw_sprites(cliprect, scanline, i);
		}
	}

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint32_t* dest = &bitmap.pix(y, cliprect.min_x);
		const uint32_t* src = &m_screenbuf[cliprect.min_x + 320 * y];
		std::copy_n(src, cliprect.width(), dest);
	}

	return 0;
}

void spg110_video_device::vblank(int state)
{
	m_blk_irq_flag = state;
	update_video_irqs();
}
