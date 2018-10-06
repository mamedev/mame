// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*****************************************************************************

    SunPlus SPG2xx-series SoC peripheral emulation

    TODO:
        - Serial UART
        - I2C
        - SPI

**********************************************************************/

#include "spg2xx.h"

DEFINE_DEVICE_TYPE(SPG2XX, spg2xx_device, "spg2xx", "SPG200-series System-on-a-Chip")

#define VERBOSE_LEVEL   (4)

#define ENABLE_VERBOSE_LOG (1)

inline void spg2xx_device::verboselog(int n_level, const char *s_fmt, ...)
{
#if ENABLE_VERBOSE_LOG
	if (VERBOSE_LEVEL >= n_level)
	{
		va_list v;
		char buf[32768];
		va_start(v, s_fmt);
		vsprintf(buf, s_fmt, v);
		logerror("%s: %s", machine().describe_context(), buf);
		va_end(v);
	}
#endif
}

#define SPG_DEBUG_VIDEO		(0)

#define IO_IRQ_ENABLE		m_io_regs[0x21]
#define IO_IRQ_STATUS		m_io_regs[0x22]
#define VIDEO_IRQ_ENABLE    m_video_regs[0x62]
#define VIDEO_IRQ_STATUS    m_video_regs[0x63]

spg2xx_device::spg2xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPG2XX, tag, owner, clock)
	, m_porta_out(*this)
	, m_portb_out(*this)
	, m_portc_out(*this)
	, m_porta_in(*this)
	, m_portb_in(*this)
	, m_portc_in(*this)
	, m_eeprom_w(*this)
	, m_eeprom_r(*this)
	, m_uart_tx(*this)
	, m_uart_rx(*this)
	, m_cpu(*this, finder_base::DUMMY_TAG)
	, m_screen(*this, finder_base::DUMMY_TAG)
	, m_scrollram(*this, "scrollram")
	, m_paletteram(*this, "paletteram")
	, m_spriteram(*this, "spriteram")
{
}

void spg2xx_device::map(address_map &map)
{
	map(0x000000, 0x0027ff).ram();
	map(0x002800, 0x0028ff).rw(FUNC(spg2xx_device::video_r), FUNC(spg2xx_device::video_w));
	map(0x002900, 0x002aff).ram().share("scrollram");
	map(0x002b00, 0x002bff).ram().share("paletteram");
	map(0x002c00, 0x002fff).ram().share("spriteram");
	map(0x003000, 0x0037ff).rw(FUNC(spg2xx_device::audio_r), FUNC(spg2xx_device::audio_w));
	map(0x003d00, 0x003eff).rw(FUNC(spg2xx_device::io_r), FUNC(spg2xx_device::io_w));
}

void spg2xx_device::device_start()
{
	m_porta_out.resolve_safe();
	m_portb_out.resolve_safe();
	m_portc_out.resolve_safe();
	m_porta_in.resolve_safe(0);
	m_portb_in.resolve_safe(0);
	m_portc_in.resolve_safe(0);
	m_eeprom_w.resolve_safe();
	m_eeprom_r.resolve_safe(0);
	m_uart_tx.resolve_safe();
	m_uart_rx.resolve_safe(0);

	m_tmb1 = timer_alloc(TIMER_TMB1);
	m_tmb2 = timer_alloc(TIMER_TMB2);
	m_tmb1->adjust(attotime::never);
	m_tmb2->adjust(attotime::never);

	m_screenpos_timer = timer_alloc(TIMER_SCREENPOS);
	m_screenpos_timer->adjust(attotime::never);
}

void spg2xx_device::device_reset()
{
	memset(m_video_regs, 0, 0x100 * sizeof(m_video_regs[0]));
	memset(m_io_regs, 0, 0x200 * sizeof(m_io_regs[0]));

	m_video_regs[0x36] = 0xffff;
	m_video_regs[0x37] = 0xffff;

	m_hide_page0 = false;
	m_hide_page1 = false;
	m_hide_sprites = false;
	m_debug_sprites = false;
	m_debug_blit = false;
	m_sprite_index_to_debug = 0;
}


/*************************
*     Video Hardware     *
*************************/

inline uint8_t spg2xx_device::expand_rgb5_to_rgb8(uint8_t val)
{
	uint8_t temp = val & 0x1f;
	return (temp << 3) | (temp >> 2);
}

// Perform a lerp between a and b
inline uint8_t spg2xx_device::mix_channel(uint8_t a, uint8_t b)
{
	uint8_t alpha = (m_video_regs[0x2a] & 3) << 6;
	return ((255 - alpha) * a + alpha * b) / 255;
}

void spg2xx_device::mix_pixel(uint32_t offset, uint16_t rgb)
{
	m_screenbuf[offset].r = mix_channel(m_screenbuf[offset].r, expand_rgb5_to_rgb8(rgb >> 10));
	m_screenbuf[offset].g = mix_channel(m_screenbuf[offset].g, expand_rgb5_to_rgb8(rgb >> 5));
	m_screenbuf[offset].b = mix_channel(m_screenbuf[offset].b, expand_rgb5_to_rgb8(rgb));
}

void spg2xx_device::set_pixel(uint32_t offset, uint16_t rgb)
{
	m_screenbuf[offset].r = expand_rgb5_to_rgb8(rgb >> 10);
	m_screenbuf[offset].g = expand_rgb5_to_rgb8(rgb >> 5);
	m_screenbuf[offset].b = expand_rgb5_to_rgb8(rgb);
}

void spg2xx_device::blit(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint32_t xoff, uint32_t yoff, uint32_t attr, uint32_t ctrl, uint32_t bitmap_addr, uint16_t tile)
{
	address_space &space = m_cpu->space(AS_PROGRAM);

	uint32_t h = 8 << ((attr & PAGE_TILE_HEIGHT_MASK) >> PAGE_TILE_HEIGHT_SHIFT);
	uint32_t w = 8 << ((attr & PAGE_TILE_WIDTH_MASK) >> PAGE_TILE_WIDTH_SHIFT);

	uint32_t yflipmask = attr & TILE_Y_FLIP ? h - 1 : 0;
	uint32_t xflipmask = attr & TILE_X_FLIP ? w - 1 : 0;

	uint32_t nc = ((attr & 0x0003) + 1) << 1;

	uint32_t palette_offset = (attr & 0x0f00) >> 4;
	if (m_debug_blit && SPG_DEBUG_VIDEO)
	{
		printf("xy:%08x,%08x attr:%08x ctrl:%08x bitmap_addr:%08x tile:%04x\n", xoff, yoff, attr, ctrl, bitmap_addr, tile);
		printf("hw:%d,%d f:%d,%d fm:%d,%d ncols:%d pobs:%02x ", w, h, (attr & TILE_X_FLIP) ? 1 : 0, (attr & TILE_Y_FLIP) ? 1 : 0, xflipmask, yflipmask, nc, palette_offset);
	}
	palette_offset >>= nc;
	palette_offset <<= nc;
	if (m_debug_blit && SPG_DEBUG_VIDEO)
	{
		printf("poas:%02x\n", palette_offset);
	}

	uint32_t m = bitmap_addr + nc * w*h / 16 * tile;
	uint32_t bits = 0;
	uint32_t nbits = 0;

	for (uint32_t y = 0; y < h; y++)
	{
		int yy = (yoff + (y ^ yflipmask)) & 0x1ff;
		if (yy >= 0x01c0)
			yy -= 0x0200;

		if (m_debug_blit && SPG_DEBUG_VIDEO)
			printf("%3d:\n", yy);

		for (uint32_t x = 0; x < w; x++)
		{
			int xx = xoff + (x ^ xflipmask);

			bits <<= nc;
			if (m_debug_blit && SPG_DEBUG_VIDEO)
				printf("    %08x:%d ", bits, nbits);
			if (nbits < nc)
			{
				uint16_t b = space.read_word(m++ & 0x3fffff);
				b = (b << 8) | (b >> 8);
				bits |= b << (nc - nbits);
				nbits += 16;
				if (m_debug_blit && SPG_DEBUG_VIDEO)
					printf("(%04x:%08x:%d) ", b, bits, nbits);
			}
			nbits -= nc;

			uint32_t pal = palette_offset + (bits >> 16);
			if (m_debug_blit && SPG_DEBUG_VIDEO)
				printf("%02x:%02x:%04x ", bits >> 16, pal, bits & 0xffff);
			bits &= 0xffff;

			if ((ctrl & 0x0010) && yy < 240)
				xx -= (int16_t)m_scrollram[yy + 15];

			xx &= 0x01ff;
			if (xx >= 0x01c0)
				xx -= 0x0200;

			if (xx >= 0 && xx < 320 && yy >= 0 && yy < 240)
			{
				uint16_t rgb = m_paletteram[pal];
				if (m_debug_blit && SPG_DEBUG_VIDEO)
					printf("rgb:%04x ", rgb);
				if (!(rgb & 0x8000))
				{
					if (attr & 0x4000 || ctrl & 0x0100)
					{
						if (m_debug_blit && SPG_DEBUG_VIDEO)
							printf("M\n");
						mix_pixel(xx + 320 * yy, rgb);
					}
					else
					{
						if (m_debug_blit && SPG_DEBUG_VIDEO)
							printf("S\n");
						set_pixel(xx + 320 * yy, rgb);
					}
				}
				else if (m_debug_blit && SPG_DEBUG_VIDEO)
				{
					printf("X\n");
				}
			}
		}
	}
}

void spg2xx_device::blit_page(bitmap_rgb32 &bitmap, const rectangle &cliprect, int depth, uint32_t bitmap_addr, uint16_t *regs)
{
	uint32_t xscroll = regs[0];
	uint32_t yscroll = regs[1];
	uint32_t attr = regs[2];
	uint32_t ctrl = regs[3];
	uint32_t tilemap = regs[4];
	uint32_t palette_map = regs[5];
	address_space &space = m_cpu->space(AS_PROGRAM);

	if (!(ctrl & PAGE_ENABLE_MASK))
	{
		return;
	}

	if (((attr & PAGE_DEPTH_FLAG_MASK) >> PAGE_DEPTH_FLAG_SHIFT) != depth)
	{
		return;
	}

	uint32_t h = 8 << ((attr & PAGE_TILE_HEIGHT_MASK) >> PAGE_TILE_HEIGHT_SHIFT);
	uint32_t w = 8 << ((attr & PAGE_TILE_WIDTH_MASK) >> PAGE_TILE_WIDTH_SHIFT);

	uint32_t hn = 256 / h;
	uint32_t wn = 512 / w;

	for (uint32_t y0 = 0; y0 < hn; y0++)
	{
		for (uint32_t x0 = 0; x0 < wn; x0++)
		{
			uint16_t tile = (ctrl & PAGE_BLANK_MASK) ? 0 : space.read_word(tilemap + x0 + wn * y0);
			uint16_t palette = 0;
			uint32_t xx, yy;

			if (!tile)
			{
				continue;
			}

			palette = space.read_word(palette_map + (x0 + wn * y0) / 2);
			if (x0 & 1)
			{
				palette >>= 8;
			}

			uint32_t tileattr = attr;
			uint32_t tilectrl = ctrl;
			if ((ctrl & 2) == 0)
			{   // -(1) bld(1) flip(2) pal(4)
				tileattr &= ~0x000c;
				tileattr |= (palette >> 2) & 0x000c;    // flip

				tileattr &= ~0x0f00;
				tileattr |= (palette << 8) & 0x0f00;    // palette

				tilectrl &= ~0x0100;
				tilectrl |= (palette << 2) & 0x0100;    // blend
			}

			yy = ((h*y0 - yscroll + 0x10) & 0xff) - 0x10;
			xx = (w*x0 - xscroll) & 0x1ff;

			blit(bitmap, cliprect, xx, yy, tileattr, tilectrl, bitmap_addr, tile);
		}
	}
}

void spg2xx_device::blit_sprite(bitmap_rgb32 &bitmap, const rectangle &cliprect, int depth, uint32_t base_addr)
{
	address_space &space = m_cpu->space(AS_PROGRAM);
	uint32_t bitmap_addr = 0x40 * m_video_regs[0x22];

	uint16_t tile = space.read_word(base_addr + 0);
	int16_t x = space.read_word(base_addr + 1);
	int16_t y = space.read_word(base_addr + 2);
	uint16_t attr = space.read_word(base_addr + 3);

	if (!tile)
	{
		return;
	}

	if (((attr & PAGE_DEPTH_FLAG_MASK) >> PAGE_DEPTH_FLAG_SHIFT) != depth)
	{
		return;
	}

	if (!(m_video_regs[0x42] & SPRITE_COORD_TL_MASK))
	{
		x = 160 + x;
		y = 120 - y;

		uint32_t h = 8 << ((attr & PAGE_TILE_HEIGHT_MASK) >> PAGE_TILE_HEIGHT_SHIFT);
		uint32_t w = 8 << ((attr & PAGE_TILE_WIDTH_MASK) >> PAGE_TILE_WIDTH_SHIFT);

		x -= (w / 2);
		y -= (h / 2) - 8;
	}

	x &= 0x01ff;
	y &= 0x01ff;

#if SPG_DEBUG_VIDEO
	if (m_debug_sprites && machine().input().code_pressed(KEYCODE_MINUS))
		m_debug_blit = true;
	blit(bitmap, cliprect, x, y, attr, 0, bitmap_addr, tile);
	m_debug_blit = false;
#else
	blit(bitmap, cliprect, x, y, attr, 0, bitmap_addr, tile);
#endif
}

void spg2xx_device::blit_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, int depth)
{
	if (!(m_video_regs[0x42] & SPRITE_ENABLE_MASK))
	{
		return;
	}

#if SPG_DEBUG_VIDEO
	if (!m_debug_sprites)
	{
#endif
		for (uint32_t n = 0; n < 256; n++)
		{
			blit_sprite(bitmap, cliprect, depth, 0x2c00 + 4 * n);
		}
#if SPG_DEBUG_VIDEO
	}
	else
	{
		blit_sprite(bitmap, cliprect, depth, 0x2c00 + 4 * m_sprite_index_to_debug);
	}
#endif
}

uint32_t spg2xx_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	memset(&m_screenbuf[320 * cliprect.min_y], 0, 3 * 320 * ((cliprect.max_y - cliprect.min_y) + 1));

	for (int i = 0; i < 4; i++)
	{
		if (!m_hide_page0)
			blit_page(bitmap, cliprect, i, 0x40 * m_video_regs[0x20], m_video_regs + 0x10);
		if (!m_hide_page1)
			blit_page(bitmap, cliprect, i, 0x40 * m_video_regs[0x21], m_video_regs + 0x16);
		if (!m_hide_sprites)
			blit_sprites(bitmap, cliprect, i);
	}

	bitmap.fill(0, cliprect);
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			bitmap.pix32(y, x) = (m_screenbuf[x + 320 * y].r << 16) | (m_screenbuf[x + 320 * y].g << 8) | m_screenbuf[x + 320 * y].b;
		}
	}

	return 0;
}

void spg2xx_device::do_video_dma(uint32_t len)
{
	address_space &mem = m_cpu->space(AS_PROGRAM);

	uint32_t src = m_video_regs[0x70] & 0x3fff;
	uint32_t dst = (m_video_regs[0x71] & 0x3ff) + 0x2c00;

	for (uint32_t j = 0; j < len; j++)
	{
		mem.write_word(dst + j, mem.read_word(src + j));
	}

	m_video_regs[0x72] = 0;
	VIDEO_IRQ_STATUS |= 4;
}

READ16_MEMBER(spg2xx_device::video_r)
{
	switch (offset)
	{
	case 0x38: // Current Line
		verboselog(3, "video_r: Current Line: %04x\n", m_screen->vpos());
		return m_screen->vpos();

	case 0x62: // Video IRQ Enable
		verboselog(0, "video_r: Video IRQ Enable: %04x\n", VIDEO_IRQ_ENABLE);
		return VIDEO_IRQ_ENABLE;

	case 0x63: // Video IRQ Status
		verboselog(0, "video_r: Video IRQ Status: %04x\n", VIDEO_IRQ_STATUS);
		return VIDEO_IRQ_STATUS;

	default:
		verboselog(0, "video_r: Unknown register %04x = %04x\n", 0x2800 + offset, m_video_regs[offset]);
		break;
	}
	return m_video_regs[offset];
}

WRITE16_MEMBER(spg2xx_device::video_w)
{
	switch (offset)
	{
	case 0x10:	// Page 1 X scroll
		verboselog(4, "video_w: Page 1 X Scroll: %04x = %04x\n", 0x2800 | offset, data & 0x01ff);
		m_video_regs[offset] = data & 0x01ff;
		break;

	case 0x16:	// Page 2 X scroll
		verboselog(4, "video_w: Page 2 X Scroll: %04x = %04x\n", 0x2800 | offset, data & 0x01ff);
		m_video_regs[offset] = data & 0x01ff;
		break;

	case 0x11:	// Page 1 Y scroll
		verboselog(4, "video_w: Page 1 Y Scroll: %04x = %04x\n", 0x2800 | offset, data & 0x00ff);
		m_video_regs[offset] = data & 0x00ff;
		break;

	case 0x17:	// Page 2 Y scroll
		verboselog(4, "video_w: Page 2 Y Scroll: %04x = %04x\n", 0x2800 | offset, data & 0x00ff);
		m_video_regs[offset] = data & 0x00ff;
		break;

	case 0x36:      // IRQ pos V
	case 0x37:      // IRQ pos H
		m_video_regs[offset] = data & 0x01ff;
		verboselog(0, "video_w: Video IRQ Position: %04x,%04x (%04x)\n", m_video_regs[0x37], m_video_regs[0x36], 0x2800 | offset);
		if (m_video_regs[0x37] < 160 && m_video_regs[0x36] < 240)
			m_screenpos_timer->adjust(m_screen->time_until_pos(m_video_regs[0x36], m_video_regs[0x37] << 1));
		else
			m_screenpos_timer->adjust(attotime::never);
		break;

	case 0x62: // Video IRQ Enable
	{
		verboselog(0, "video_w: Video IRQ Enable = %04x (%04x)\n", data, mem_mask);
		const uint16_t old = VIDEO_IRQ_ENABLE & VIDEO_IRQ_STATUS;
		COMBINE_DATA(&VIDEO_IRQ_ENABLE);
		const uint16_t changed = old ^ (VIDEO_IRQ_ENABLE & VIDEO_IRQ_STATUS);
		if (changed)
			check_video_irq();
		break;
	}

	case 0x63: // Video IRQ Acknowledge
	{
		verboselog(0, "video_w: Video IRQ Acknowledge = %04x (%04x)\n", data, mem_mask);
		const uint16_t old = VIDEO_IRQ_ENABLE & VIDEO_IRQ_STATUS;
		VIDEO_IRQ_STATUS &= ~data;
		verboselog(4, "Setting video IRQ status to %04x\n", VIDEO_IRQ_STATUS);
		const uint16_t changed = old ^ (VIDEO_IRQ_ENABLE & VIDEO_IRQ_STATUS);
		if (changed)
			check_video_irq();
		break;
	}

	case 0x70: // Video DMA Source
		verboselog(0, "video_w: Video DMA Source = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_video_regs[offset]);
		break;

	case 0x71: // Video DMA Dest
		verboselog(0, "video_w: Video DMA Dest = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_video_regs[offset]);
		break;

	case 0x72: // Video DMA Length
		verboselog(0, "video_w: Video DMA Length = %04x (%04x)\n", data, mem_mask);
		do_video_dma(data & 0x3ff);
		break;

	default:
		verboselog(0, "video_w: Unknown register %04x = %04x (%04x)\n", 0x2800 + offset, data, mem_mask);
		COMBINE_DATA(&m_video_regs[offset]);
		break;
	}
}

WRITE_LINE_MEMBER(spg2xx_device::vblank)
{
	if (!state)
		return;

#if SPG_DEBUG_VIDEO
	if (machine().input().code_pressed_once(KEYCODE_5))
		m_hide_page0 = !m_hide_page0;
	if (machine().input().code_pressed_once(KEYCODE_6))
		m_hide_page1 = !m_hide_page1;
	if (machine().input().code_pressed_once(KEYCODE_7))
		m_hide_sprites = !m_hide_sprites;
	if (machine().input().code_pressed_once(KEYCODE_8))
		m_debug_sprites = !m_debug_sprites;
	if (machine().input().code_pressed_once(KEYCODE_9))
		m_sprite_index_to_debug--;
	if (machine().input().code_pressed_once(KEYCODE_0))
		m_sprite_index_to_debug++;
#endif

	const uint16_t old = VIDEO_IRQ_ENABLE & VIDEO_IRQ_STATUS;
	VIDEO_IRQ_STATUS |= 1;
	verboselog(4, "Setting video IRQ status to %04x\n", VIDEO_IRQ_STATUS);
	const uint16_t changed = old ^ (VIDEO_IRQ_ENABLE & VIDEO_IRQ_STATUS);
	if (changed)
		check_video_irq();

	// For now, manually trigger controller IRQs
	if (IO_IRQ_ENABLE & 0x2100)
	{
		IO_IRQ_STATUS |= 0x0100;
		m_cpu->set_input_line(UNSP_IRQ3_LINE, ASSERT_LINE);
	}
}

void spg2xx_device::check_video_irq()
{
	m_cpu->set_input_line(UNSP_IRQ0_LINE, (VIDEO_IRQ_STATUS & VIDEO_IRQ_ENABLE) ? ASSERT_LINE : CLEAR_LINE);
}


/*************************
*    Machine Hardware    *
*************************/

READ16_MEMBER(spg2xx_device::audio_r)
{
	switch (offset)
	{
	default:
		verboselog(4, "audio_r: Unknown register %04x\n", 0x3000 + offset);
		break;
	}
	return 0;
}

WRITE16_MEMBER(spg2xx_device::audio_w)
{
	switch (offset)
	{
	default:
		verboselog(5, "audio_w: Unknown register %04x = %04x (%04x)\n", 0x3000 + offset, data, mem_mask);
		break;
	}
}

READ16_MEMBER(spg2xx_device::io_r)
{
	static const char *const gpioregs[] = { "GPIO Data Port", "GPIO Buffer Port", "GPIO Direction Port", "GPIO Attribute Port", "GPIO IRQ/Latch Port" };
	static const char gpioports[] = { 'A', 'B', 'C' };

	uint16_t val = m_io_regs[offset];

	switch (offset)
	{
	case 0x01: case 0x06: case 0x0b: // GPIO Data Port A/B/C
		do_gpio(offset);
		verboselog(3, "io_r: %s %c = %04x (%04x)\n", gpioregs[(offset - 1) % 5], gpioports[(offset - 1) / 5], m_io_regs[offset], mem_mask);
		val = m_io_regs[offset];
		break;

	case 0x02: case 0x03: case 0x04: case 0x05:
	case 0x07: case 0x08: case 0x09: case 0x0a:
	case 0x0c: case 0x0d: case 0x0e: case 0x0f: // Other GPIO regs
		verboselog(3, "io_r: %s %c = %04x (%04x)\n", gpioregs[(offset - 1) % 5], gpioports[(offset - 1) / 5], m_io_regs[offset], mem_mask);
		break;

	case 0x1c: // Video line counter
		val = m_screen->vpos();
		verboselog(3, "io_r: Video Line = %04x (%04x)\n", val, mem_mask);
		break;

	case 0x21: // IRQ Control
		verboselog(3, "io_r: Controller IRQ Control = %04x (%04x)\n", val, mem_mask);
		break;

	case 0x22: // IRQ Status
		verboselog(3, "io_r: Controller IRQ Status = %04x (%04x)\n", val, mem_mask);
		break;

	case 0x2b:
		return 0x0000;

	case 0x2c: case 0x2d: // PRNG 0/1
		val = machine().rand() & 0x0000ffff;
		verboselog(3, "io_r: PRNG %d = %04x (%04x)\n", offset - 0x2c, val, mem_mask);
		break;

	case 0x2f: // Data Segment
		val = m_cpu->state_int(UNSP_SR) >> 10;
		verboselog(3, "io_r: Data Segment = %04x (%04x)\n", val, mem_mask);
		break;

	case 0x31: // UART Status
		verboselog(3, "io_r: UART Status = %04x (%04x)\n", 3, mem_mask);
		val = 3; // HACK
		break;

	case 0x36: // UART RX Data
		val = m_uart_rx();
		verboselog(3, "io_r: UART RX Data = %04x (%04x)\n", val, mem_mask);
		break;

	case 0x59: // I2C Status
		verboselog(3, "io_r: I2C Status = %04x (%04x)\n", val, mem_mask);
		break;

	case 0x5e: // I2C Data In
		verboselog(3, "io_r: I2C Data In = %04x (%04x)\n", val, mem_mask);
		break;

	default:
		verboselog(3, "io_r: Unknown register %04x\n", 0x3d00 + offset);
		break;
	}

	return val;
}

WRITE16_MEMBER(spg2xx_device::io_w)
{
	static const char *const gpioregs[] = { "GPIO Data Port", "GPIO Buffer Port", "GPIO Direction Port", "GPIO Attribute Port", "GPIO IRQ/Latch Port" };
	static const char gpioports[3] = { 'A', 'B', 'C' };

	uint16_t temp = 0;

	switch (offset)
	{
	case 0x00: // GPIO special function select
		verboselog(3, "io_w: GPIO Function Select = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x01: case 0x06: case 0x0b: // GPIO data, port A/B/C
		offset++;
		// Intentional fallthrough - we redirect data register writes to the buffer register.

	case 0x02: case 0x03: case 0x04: case 0x05: // Port A
	case 0x07: case 0x08: case 0x09: case 0x0a: // Port B
	case 0x0c: case 0x0d: case 0x0e: case 0x0f: // Port C
		verboselog(3, "io_w: %s %c = %04x (%04x)\n", gpioregs[(offset - 1) % 5], gpioports[(offset - 1) / 5], data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		do_gpio(offset);
		break;

	case 0x10:      // timebase control
		if ((m_io_regs[offset] & 0x0003) != (data & 0x0003))
		{
			uint16_t hz = 8 << (data & 0x0003);
			verboselog(3, "*** TMB1 FREQ set to %dHz\n", hz);
			m_tmb1->adjust(attotime::from_hz(hz), 0, attotime::from_hz(hz));
		}
		if ((m_io_regs[offset] & 0x000c) != (data & 0x000c))
		{
			uint16_t hz = 128 << ((data & 0x000c) >> 2);
			verboselog(3, "*** TMB2 FREQ set to %dHz\n", hz);
			m_tmb2->adjust(attotime::from_hz(hz), 0, attotime::from_hz(hz));
		}
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x21: // IRQ Enable
	{
		verboselog(3, "io_w: IRQ Control = %04x (%04x)\n", data, mem_mask);
		const uint16_t old = IO_IRQ_ENABLE & IO_IRQ_STATUS;
		COMBINE_DATA(&IO_IRQ_ENABLE);
		const uint16_t changed = old ^ (IO_IRQ_ENABLE & IO_IRQ_STATUS);
		if (changed)
			check_irqs(changed);
		break;
	}

	case 0x22: // IRQ Acknowledge
	{
		verboselog(3, "io_w: IRQ Acknowledge = %04x (%04x)\n", data, mem_mask);
		const uint16_t old = IO_IRQ_STATUS;
		IO_IRQ_STATUS &= ~data;
		const uint16_t changed = old ^ (IO_IRQ_ENABLE & IO_IRQ_STATUS);
		if (changed)
			check_irqs(changed);
		break;
	}

	case 0x2f: // Data Segment
		temp = m_cpu->state_int(UNSP_SR);
		m_cpu->set_state_int(UNSP_SR, (temp & 0x03ff) | ((data & 0x3f) << 10));
		verboselog(3, "io_w: Data Segment = %04x (%04x)\n", data, mem_mask);
		break;

	case 0x31: // Unknown UART
		verboselog(3, "io_w: Unknown UART = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x32: // UART Reset
		verboselog(3, "io_w: UART Reset\n");
		break;

	case 0x33: // UART Baud Rate
		verboselog(3, "io_w: UART Baud Rate = %u\n", 27000000 / 16 / (0x10000 - (m_io_regs[0x34] << 8) - data));
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x35: // UART TX Data
		verboselog(3, "io_w: UART Baud Rate = %u\n", 27000000 / 16 / (0x10000 - (data << 8) - m_io_regs[0x33]));
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x58: // I2C Command
		verboselog(3, "io_w: I2C Command = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		do_i2c();
		break;

	case 0x59: // I2C Status / Acknowledge
		verboselog(3, "io_w: I2C Acknowledge = %04x (%04x)\n", data, mem_mask);
		m_io_regs[offset] &= ~data;
		break;

	case 0x5a: // I2C Access Mode
		verboselog(3, "io_w: I2C Access Mode = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x5b: // I2C Device Address
		verboselog(3, "io_w: I2C Device Address = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x5c: // I2C Sub-Address
		verboselog(3, "io_w: I2C Sub-Address = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x5d: // I2C Data Out
		verboselog(3, "io_w: I2C Data Out = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x5e: // I2C Data In
		verboselog(3, "io_w: I2C Data In = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x5f: // I2C Controller Mode
		verboselog(3, "io_w: I2C Controller Mode = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x100: // DMA Source (L)
		verboselog(3, "io_w: DMA Source (L) 3e00 = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x101: // DMA Source (H)
		verboselog(3, "io_w: DMA Source (H) 3e01 = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x103: // DMA Destination
		verboselog(3, "io_w: DMA Dest 3e03 = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x102: // DMA Length
		verboselog(3, "io_w: DMA Length 3e02 = %04x (%04x)\n", data, mem_mask);
		do_cpu_dma(data);
		break;

	default:
		verboselog(3, "io_w: Unknown register %04x = %04x (%04x)\n", 0x3d00 + offset, data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		break;
	}
}

void spg2xx_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_TMB1:
		{
			const uint16_t old = (IO_IRQ_ENABLE & IO_IRQ_STATUS);
			IO_IRQ_STATUS |= 1;
			const uint16_t changed = old ^ (IO_IRQ_ENABLE & IO_IRQ_STATUS);
			if (changed)
				check_irqs(changed);
			break;
		}
		case TIMER_TMB2:
		{
			const uint16_t old = m_io_regs[0x22] & m_io_regs[0x21];
			IO_IRQ_STATUS |= 2;
			const uint16_t changed = old ^ (IO_IRQ_ENABLE & IO_IRQ_STATUS);
			if (changed)
				check_irqs(changed);
			break;
		}
		case TIMER_SCREENPOS:
		{
			const uint16_t old = VIDEO_IRQ_ENABLE & VIDEO_IRQ_STATUS;
			VIDEO_IRQ_STATUS |= 2;
			const uint16_t changed = old ^ (VIDEO_IRQ_ENABLE & VIDEO_IRQ_STATUS);
			if (changed)
			{
				check_video_irq();
			}
			m_screen->update_partial(m_screen->vpos());
			break;
		}
	}
}

void spg2xx_device::check_irqs(const uint16_t changed)
{
	//  {
	//      verboselog(0, "audio 1 IRQ\n");
	//      m_cpu->set_input_line(UNSP_IRQ1_LINE, ASSERT_LINE);
	//  }
	if (changed & 0x0c00) // Timer A, Timer B IRQ
	{
		m_cpu->set_input_line(UNSP_IRQ2_LINE, (IO_IRQ_ENABLE & IO_IRQ_STATUS & 0x0c00) ? ASSERT_LINE : CLEAR_LINE);
	}

	if (changed & 0x2100) // UART, ADC IRQ
		m_cpu->set_input_line(UNSP_IRQ3_LINE, (IO_IRQ_ENABLE & IO_IRQ_STATUS & 0x2100) ? ASSERT_LINE : CLEAR_LINE);

	//  {
	//      verboselog(0, "audio 4 IRQ\n");
	//      m_cpu->set_input_line(UNSP_IRQ4_LINE, ASSERT_LINE);
	//  }

	if (changed & 0x1200) // External IRQ
	{
		m_cpu->set_input_line(UNSP_IRQ5_LINE, (IO_IRQ_ENABLE & IO_IRQ_STATUS & 0x1200) ? ASSERT_LINE : CLEAR_LINE);
	}

	if (changed & 0x0070) // 1024Hz, 2048Hz, 4096Hz IRQ
	{
		m_cpu->set_input_line(UNSP_IRQ6_LINE, (IO_IRQ_ENABLE & IO_IRQ_STATUS & 0x0070) ? ASSERT_LINE : CLEAR_LINE);
	}

	if (changed & 0x008b) // TMB1, TMB2, 4Hz, key change IRQ
	{
		m_cpu->set_input_line(UNSP_IRQ7_LINE, (IO_IRQ_ENABLE & IO_IRQ_STATUS & 0x008b) ? ASSERT_LINE : CLEAR_LINE);
	}
}

void spg2xx_device::do_gpio(uint32_t offset)
{
	uint32_t index = (offset - 1) / 5;
	uint16_t buffer = m_io_regs[5 * index + 2];
	uint16_t dir = m_io_regs[5 * index + 3];
	uint16_t attr = m_io_regs[5 * index + 4];
	uint16_t special = m_io_regs[5 * index + 5];

	uint16_t push = dir;
	uint16_t pull = (~dir) & (~attr);
	uint16_t what = (buffer & (push | pull));
	what ^= (dir & ~attr);
	what &= ~special;

	switch (index)
	{
		case 0:
			m_porta_out(what);
			what = (what & ~pull) | (m_porta_in() & pull);
			break;
		case 1:
			m_portb_out(what);
			what = (what & ~pull) | (m_portb_in() & pull);
			break;
		case 2:
			m_portc_out(what);
			what = (what & ~pull) | (m_portc_in() & pull);
			break;
	}

	m_io_regs[5 * index + 1] = what;
}

void spg2xx_device::do_i2c()
{
	const uint16_t addr = ((m_io_regs[0x5b] & 0x06) << 7) | (uint8_t)m_io_regs[0x5c];

	if (m_io_regs[0x58] & 0x40) // Serial EEPROM read
		m_io_regs[0x5e] = m_eeprom_r(addr);
	else
		m_eeprom_w(addr, m_io_regs[0x5d]);

	m_io_regs[0x59] |= 1;
}

void spg2xx_device::do_cpu_dma(uint32_t len)
{
	address_space &mem = m_cpu->space(AS_PROGRAM);

	uint32_t src = ((m_io_regs[0x101] & 0x3f) << 16) | m_io_regs[0x100];
	uint32_t dst = m_io_regs[0x103] & 0x3fff;

	for (uint32_t j = 0; j < len; j++)
	{
		mem.write_word(dst + j, mem.read_word(src + j));
	}

	m_io_regs[0x102] = 0;
}
