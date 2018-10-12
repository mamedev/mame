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

DEFINE_DEVICE_TYPE(SPG24X, spg24x_device, "spg24x", "SPG240-series System-on-a-Chip")
DEFINE_DEVICE_TYPE(SPG28X, spg28x_device, "spg28x", "SPG280-series System-on-a-Chip")

#define VERBOSE_LEVEL   (4)

#define ENABLE_VERBOSE_LOG (1)

#if SPG2XX_VISUAL_AUDIO_DEBUG
static const uint32_t s_visual_debug_palette[8] = {
	0xff000000,
	0xff0000ff,
	0xff00ff00,
	0xff00ffff,
	0xffff0000,
	0xffff00ff,
	0xffffff00,
	0xffffffff
};
#define SPG_VDB_BLACK 0
#define SPG_VDB_WAVE 1
#define SPG_VDB_EDD 2
#define SPG_VDB_VOL 4
#endif

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
#define SPG_DEBUG_AUDIO		(0)
#define SPG_DEBUG_ENVELOPES	(0)
#define SPG_DEBUG_SAMPLES	(1)

#define IO_IRQ_ENABLE		m_io_regs[0x21]
#define IO_IRQ_STATUS		m_io_regs[0x22]
#define VIDEO_IRQ_ENABLE    m_video_regs[0x62]
#define VIDEO_IRQ_STATUS    m_video_regs[0x63]

spg2xx_device::spg2xx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
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
#if SPG2XX_VISUAL_AUDIO_DEBUG
	, m_audio_screen(*this, finder_base::DUMMY_TAG)
#endif
{
}

spg24x_device::spg24x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spg2xx_device(mconfig, SPG24X, tag, owner, clock, 256)
{
}

spg28x_device::spg28x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spg2xx_device(mconfig, SPG28X, tag, owner, clock, 64)
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
#if SPG2XX_VISUAL_AUDIO_DEBUG
	m_audio_debug_buffer = std::make_unique<uint8_t[]>(640*480);
#endif
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

	m_audio_beat = timer_alloc(TIMER_BEAT);
	m_audio_beat->adjust(attotime::never);

	m_stream = stream_alloc(0, 2, 44100);
}

void spg2xx_device::device_reset()
{
	memset(m_audio_regs, 0, 0x800 * sizeof(uint16_t));
	memset(m_sample_shift, 0, 6);
	memset(m_sample_count, 0, sizeof(uint32_t) * 6);
	memset(m_sample_addr, 0, sizeof(uint32_t) * 6);
	memset(m_channel_rate, 0, sizeof(double) * 6);
	memset(m_channel_rate_accum, 0, sizeof(double) * 6);
	memset(m_rampdown_frame, 0, sizeof(uint32_t) * 6);
	memset(m_envclk_frame, 4, sizeof(uint32_t) * 6);
	memset(m_envelope_addr, 0, sizeof(uint32_t) * 6);

	memset(m_video_regs, 0, 0x100 * sizeof(uint16_t));
	memset(m_io_regs, 0, 0x200 * sizeof(uint16_t));

	m_video_regs[0x36] = 0xffff;
	m_video_regs[0x37] = 0xffff;

	m_hide_page0 = false;
	m_hide_page1 = false;
	m_hide_sprites = false;
	m_debug_sprites = false;
	m_debug_blit = false;
	m_sprite_index_to_debug = 0;

	m_debug_samples = false;
	m_debug_rates = false;
	m_audio_curr_beat_base_count = 0;

	m_audio_regs[AUDIO_CHANNEL_REPEAT] = 0x3f;
	m_audio_regs[AUDIO_CHANNEL_ENV_MODE] = 0x3f;

	m_audio_beat->adjust(attotime::from_ticks(4, 281250), 0, attotime::from_ticks(4, 281250));
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
		for (uint32_t n = 0; n < m_sprite_limit; n++)
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

#if SPG2XX_VISUAL_AUDIO_DEBUG
uint32_t spg2xx_device::debug_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	for (int y = 0; y < 480; y++)
	{
		for (int x = 0; x < 640; x++)
		{
			bitmap.pix32(y, x) = s_visual_debug_palette[m_audio_debug_buffer[y*640+x]];
		}
	}
	return 0;
}

void spg2xx_device::advance_debug_pos()
{
	m_audio_debug_x++;
	if (m_audio_debug_x == 640)
	{
		m_audio_debug_x = 0;
	}
}
#endif

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
		verboselog(5, "video_r: Current Line: %04x\n", m_screen->vpos());
		return m_screen->vpos();

	case 0x62: // Video IRQ Enable
		verboselog(5, "video_r: Video IRQ Enable: %04x\n", VIDEO_IRQ_ENABLE);
		return VIDEO_IRQ_ENABLE;

	case 0x63: // Video IRQ Status
		verboselog(5, "video_r: Video IRQ Status: %04x\n", VIDEO_IRQ_STATUS);
		return VIDEO_IRQ_STATUS;

	default:
		verboselog(5, "video_r: Unknown register %04x = %04x\n", 0x2800 + offset, m_video_regs[offset]);
		break;
	}
	return m_video_regs[offset];
}

WRITE16_MEMBER(spg2xx_device::video_w)
{
	switch (offset)
	{
	case 0x10:	// Page 1 X scroll
		verboselog(5, "video_w: Page 1 X Scroll: %04x = %04x\n", 0x2800 | offset, data & 0x01ff);
		m_video_regs[offset] = data & 0x01ff;
		break;

	case 0x16:	// Page 2 X scroll
		verboselog(5, "video_w: Page 2 X Scroll: %04x = %04x\n", 0x2800 | offset, data & 0x01ff);
		m_video_regs[offset] = data & 0x01ff;
		break;

	case 0x11:	// Page 1 Y scroll
		verboselog(5, "video_w: Page 1 Y Scroll: %04x = %04x\n", 0x2800 | offset, data & 0x00ff);
		m_video_regs[offset] = data & 0x00ff;
		break;

	case 0x17:	// Page 2 Y scroll
		verboselog(5, "video_w: Page 2 Y Scroll: %04x = %04x\n", 0x2800 | offset, data & 0x00ff);
		m_video_regs[offset] = data & 0x00ff;
		break;

	case 0x36:      // IRQ pos V
	case 0x37:      // IRQ pos H
		m_video_regs[offset] = data & 0x01ff;
		verboselog(5, "video_w: Video IRQ Position: %04x,%04x (%04x)\n", m_video_regs[0x37], m_video_regs[0x36], 0x2800 | offset);
		if (m_video_regs[0x37] < 160 && m_video_regs[0x36] < 240)
			m_screenpos_timer->adjust(m_screen->time_until_pos(m_video_regs[0x36], m_video_regs[0x37] << 1));
		else
			m_screenpos_timer->adjust(attotime::never);
		break;

	case 0x62: // Video IRQ Enable
	{
		verboselog(5, "video_w: Video IRQ Enable = %04x (%04x)\n", data, mem_mask);
		const uint16_t old = VIDEO_IRQ_ENABLE & VIDEO_IRQ_STATUS;
		COMBINE_DATA(&VIDEO_IRQ_ENABLE);
		const uint16_t changed = old ^ (VIDEO_IRQ_ENABLE & VIDEO_IRQ_STATUS);
		if (changed)
			check_video_irq();
		break;
	}

	case 0x63: // Video IRQ Acknowledge
	{
		verboselog(5, "video_w: Video IRQ Acknowledge = %04x (%04x)\n", data, mem_mask);
		const uint16_t old = VIDEO_IRQ_ENABLE & VIDEO_IRQ_STATUS;
		VIDEO_IRQ_STATUS &= ~data;
		//verboselog(4, "Setting video IRQ status to %04x\n", VIDEO_IRQ_STATUS);
		const uint16_t changed = old ^ (VIDEO_IRQ_ENABLE & VIDEO_IRQ_STATUS);
		if (changed)
			check_video_irq();
		break;
	}

	case 0x70: // Video DMA Source
		verboselog(5, "video_w: Video DMA Source = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_video_regs[offset]);
		break;

	case 0x71: // Video DMA Dest
		verboselog(5, "video_w: Video DMA Dest = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_video_regs[offset]);
		break;

	case 0x72: // Video DMA Length
		verboselog(5, "video_w: Video DMA Length = %04x (%04x)\n", data, mem_mask);
		do_video_dma(data & 0x3ff);
		break;

	default:
		verboselog(5, "video_w: Unknown register %04x = %04x (%04x)\n", 0x2800 + offset, data, mem_mask);
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

#if SPG_DEBUG_AUDIO
	if (machine().input().code_pressed_once(KEYCODE_3))
		m_debug_samples = !m_debug_samples;
	if (machine().input().code_pressed_once(KEYCODE_4))
		m_debug_rates = !m_debug_rates;
#endif

	const uint16_t old = VIDEO_IRQ_ENABLE & VIDEO_IRQ_STATUS;
	VIDEO_IRQ_STATUS |= 1;
	verboselog(5, "Setting video IRQ status to %04x\n", VIDEO_IRQ_STATUS);
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

READ16_MEMBER(spg2xx_device::io_r)
{
	static const char *const gpioregs[] = { "GPIO Data Port", "GPIO Buffer Port", "GPIO Direction Port", "GPIO Attribute Port", "GPIO IRQ/Latch Port" };
	static const char gpioports[] = { 'A', 'B', 'C' };

	uint16_t val = m_io_regs[offset];

	switch (offset)
	{
	case 0x01: case 0x06: case 0x0b: // GPIO Data Port A/B/C
		do_gpio(offset);
		verboselog(5, "io_r: %s %c = %04x (%04x)\n", gpioregs[(offset - 1) % 5], gpioports[(offset - 1) / 5], m_io_regs[offset], mem_mask);
		val = m_io_regs[offset];
		break;

	case 0x02: case 0x03: case 0x04: case 0x05:
	case 0x07: case 0x08: case 0x09: case 0x0a:
	case 0x0c: case 0x0d: case 0x0e: case 0x0f: // Other GPIO regs
		verboselog(5, "io_r: %s %c = %04x (%04x)\n", gpioregs[(offset - 1) % 5], gpioports[(offset - 1) / 5], m_io_regs[offset], mem_mask);
		break;

	case 0x1c: // Video line counter
		val = m_screen->vpos();
		verboselog(5, "io_r: Video Line = %04x (%04x)\n", val, mem_mask);
		break;

	case 0x21: // IRQ Control
		verboselog(5, "io_r: Controller IRQ Control = %04x (%04x)\n", val, mem_mask);
		break;

	case 0x22: // IRQ Status
		verboselog(5, "io_r: Controller IRQ Status = %04x (%04x)\n", val, mem_mask);
		break;

	case 0x2b:
		return 0x0000;

	case 0x2c: case 0x2d: // PRNG 0/1
		val = machine().rand() & 0x0000ffff;
		verboselog(5, "io_r: PRNG %d = %04x (%04x)\n", offset - 0x2c, val, mem_mask);
		break;

	case 0x2f: // Data Segment
		val = m_cpu->state_int(UNSP_SR) >> 10;
		verboselog(5, "io_r: Data Segment = %04x (%04x)\n", val, mem_mask);
		break;

	case 0x31: // UART Status
		verboselog(5, "io_r: UART Status = %04x (%04x)\n", 3, mem_mask);
		val = 3; // HACK
		break;

	case 0x36: // UART RX Data
		val = m_uart_rx();
		verboselog(5, "io_r: UART RX Data = %04x (%04x)\n", val, mem_mask);
		break;

	case 0x59: // I2C Status
		verboselog(5, "io_r: I2C Status = %04x (%04x)\n", val, mem_mask);
		break;

	case 0x5e: // I2C Data In
		verboselog(5, "io_r: I2C Data In = %04x (%04x)\n", val, mem_mask);
		break;

	default:
		verboselog(5, "io_r: Unknown register %04x\n", 0x3d00 + offset);
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
		verboselog(5, "io_w: GPIO Function Select = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x01: case 0x06: case 0x0b: // GPIO data, port A/B/C
		offset++;
		// Intentional fallthrough - we redirect data register writes to the buffer register.

	case 0x02: case 0x03: case 0x04: case 0x05: // Port A
	case 0x07: case 0x08: case 0x09: case 0x0a: // Port B
	case 0x0c: case 0x0d: case 0x0e: case 0x0f: // Port C
		verboselog(5, "io_w: %s %c = %04x (%04x)\n", gpioregs[(offset - 1) % 5], gpioports[(offset - 1) / 5], data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		do_gpio(offset);
		break;

	case 0x10:      // timebase control
		if ((m_io_regs[offset] & 0x0003) != (data & 0x0003))
		{
			uint16_t hz = 8 << (data & 0x0003);
			verboselog(5, "*** TMB1 FREQ set to %dHz\n", hz);
			m_tmb1->adjust(attotime::from_hz(hz), 0, attotime::from_hz(hz));
		}
		if ((m_io_regs[offset] & 0x000c) != (data & 0x000c))
		{
			uint16_t hz = 128 << ((data & 0x000c) >> 2);
			verboselog(5, "*** TMB2 FREQ set to %dHz\n", hz);
			m_tmb2->adjust(attotime::from_hz(hz), 0, attotime::from_hz(hz));
		}
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x21: // IRQ Enable
	{
		verboselog(5, "io_w: IRQ Control = %04x (%04x)\n", data, mem_mask);
		const uint16_t old = IO_IRQ_ENABLE & IO_IRQ_STATUS;
		COMBINE_DATA(&IO_IRQ_ENABLE);
		const uint16_t changed = old ^ (IO_IRQ_ENABLE & IO_IRQ_STATUS);
		if (changed)
			check_irqs(changed);
		break;
	}

	case 0x22: // IRQ Acknowledge
	{
		verboselog(5, "io_w: IRQ Acknowledge = %04x (%04x)\n", data, mem_mask);
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
		verboselog(5, "io_w: Data Segment = %04x (%04x)\n", data, mem_mask);
		break;

	case 0x31: // Unknown UART
		verboselog(5, "io_w: Unknown UART = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x32: // UART Reset
		verboselog(5, "io_w: UART Reset\n");
		break;

	case 0x33: // UART Baud Rate
		verboselog(5, "io_w: UART Baud Rate = %u\n", 27000000 / 16 / (0x10000 - (m_io_regs[0x34] << 8) - data));
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x35: // UART TX Data
		verboselog(5, "io_w: UART Baud Rate = %u\n", 27000000 / 16 / (0x10000 - (data << 8) - m_io_regs[0x33]));
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x58: // I2C Command
		verboselog(5, "io_w: I2C Command = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		do_i2c();
		break;

	case 0x59: // I2C Status / Acknowledge
		verboselog(5, "io_w: I2C Acknowledge = %04x (%04x)\n", data, mem_mask);
		m_io_regs[offset] &= ~data;
		break;

	case 0x5a: // I2C Access Mode
		verboselog(5, "io_w: I2C Access Mode = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x5b: // I2C Device Address
		verboselog(5, "io_w: I2C Device Address = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x5c: // I2C Sub-Address
		verboselog(5, "io_w: I2C Sub-Address = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x5d: // I2C Data Out
		verboselog(5, "io_w: I2C Data Out = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x5e: // I2C Data In
		verboselog(5, "io_w: I2C Data In = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x5f: // I2C Controller Mode
		verboselog(5, "io_w: I2C Controller Mode = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x100: // DMA Source (L)
		verboselog(5, "io_w: DMA Source (L) 3e00 = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x101: // DMA Source (H)
		verboselog(5, "io_w: DMA Source (H) 3e01 = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x103: // DMA Destination
		verboselog(5, "io_w: DMA Dest 3e03 = %04x (%04x)\n", data, mem_mask);
		COMBINE_DATA(&m_io_regs[offset]);
		break;

	case 0x102: // DMA Length
		verboselog(5, "io_w: DMA Length 3e02 = %04x (%04x)\n", data, mem_mask);
		do_cpu_dma(data);
		break;

	default:
		verboselog(5, "io_w: Unknown register %04x = %04x (%04x)\n", 0x3d00 + offset, data, mem_mask);
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

		case TIMER_BEAT:
		{
			audio_frame_tick();
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
		m_cpu->set_input_line(UNSP_IRQ2_LINE, (IO_IRQ_ENABLE & IO_IRQ_STATUS & 0x0c00) ? ASSERT_LINE : CLEAR_LINE);

	if (changed & 0x2100) // UART, ADC IRQ
		m_cpu->set_input_line(UNSP_IRQ3_LINE, (IO_IRQ_ENABLE & IO_IRQ_STATUS & 0x2100) ? ASSERT_LINE : CLEAR_LINE);

	if (changed & (AUDIO_BIS_MASK | AUDIO_BIE_MASK)) // Beat IRQ
		m_cpu->set_input_line(UNSP_IRQ4_LINE, (m_audio_regs[AUDIO_BEAT_COUNT] & (AUDIO_BIS_MASK | AUDIO_BIE_MASK)) == (AUDIO_BIS_MASK | AUDIO_BIE_MASK) ? ASSERT_LINE : CLEAR_LINE);

	if (changed & 0x1200) // External IRQ
		m_cpu->set_input_line(UNSP_IRQ5_LINE, (IO_IRQ_ENABLE & IO_IRQ_STATUS & 0x1200) ? ASSERT_LINE : CLEAR_LINE);

	if (changed & 0x0070) // 1024Hz, 2048Hz, 4096Hz IRQ
		m_cpu->set_input_line(UNSP_IRQ6_LINE, (IO_IRQ_ENABLE & IO_IRQ_STATUS & 0x0070) ? ASSERT_LINE : CLEAR_LINE);

	if (changed & 0x008b) // TMB1, TMB2, 4Hz, key change IRQ
		m_cpu->set_input_line(UNSP_IRQ7_LINE, (IO_IRQ_ENABLE & IO_IRQ_STATUS & 0x008b) ? ASSERT_LINE : CLEAR_LINE);
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

/***********************
*    Audio Hardware    *
***********************/

READ16_MEMBER(spg2xx_device::audio_r)
{
	const uint16_t channel = (offset & 0x00f0) >> 4;
	uint16_t data = m_audio_regs[offset];

	if (offset >= 0x400)
	{
		switch (offset)
		{
		case AUDIO_CHANNEL_ENABLE:
			verboselog(0, "audio_r: Channel Enable: %04x\n", data);
			break;

		case AUDIO_MAIN_VOLUME:
			verboselog(0, "audio_r: Main Volume: %04x\n", data);
			break;

		case AUDIO_CHANNEL_FIQ_ENABLE:
			verboselog(0, "audio_r: Channel FIQ Enable: %04x\n", data);
			break;

		case AUDIO_CHANNEL_FIQ_STATUS:
			verboselog(0, "audio_r: Channel FIQ Acknowledge: %04x\n", data);
			break;

		case AUDIO_BEAT_BASE_COUNT:
			verboselog(0, "audio_r: Beat Base Count: %04x\n", data);
			break;

		case AUDIO_BEAT_COUNT:
			verboselog(0, "audio_r: Beat Count: %04x\n", data);
			break;

		case AUDIO_ENVCLK:
			verboselog(0, "audio_r: Envelope Interval (lo): %04x\n", data);
			break;

		case AUDIO_ENVCLK_HIGH:
			verboselog(0, "audio_r: Envelope Interval (hi): %04x\n", data);
			break;

		case AUDIO_ENV_RAMP_DOWN:
			verboselog(0, "audio_r: Envelope Fast Ramp Down: %04x\n", data);
			break;

		case AUDIO_CHANNEL_STOP:
			verboselog(0, "audio_r: Channel Stop Status: %04x\n", data);
			break;

		case AUDIO_CHANNEL_ZERO_CROSS:
			verboselog(0, "audio_r: Channel Zero-Cross Enable: %04x\n", data);
			break;

		case AUDIO_CONTROL:
			verboselog(0, "audio_r: Control: %04x\n", data);
			break;

		case AUDIO_COMPRESS_CTRL:
			verboselog(0, "audio_r: Compressor Control: %04x\n", data);
			break;

		case AUDIO_CHANNEL_STATUS:
			verboselog(5, "audio_r: Channel Status: %04x\n", data);
			break;

		case AUDIO_WAVE_IN_L:
			verboselog(0, "audio_r: Wave In (L) / FIFO Write Data: %04x\n", data);
			break;

		case AUDIO_WAVE_IN_R:
			verboselog(0, "audio_r: Wave In (R) / Software Channel FIFO IRQ Control: %04x\n", data);
			break;

		case AUDIO_WAVE_OUT_L:
			verboselog(0, "audio_r: Wave Out (L): %04x\n", data);
			break;

		case AUDIO_WAVE_OUT_R:
			verboselog(0, "audio_r: Wave Out (R): %04x\n", data);
			break;

		case AUDIO_CHANNEL_REPEAT:
			verboselog(0, "audio_r: Channel Repeat Enable: %04x\n", data);
			break;

		case AUDIO_CHANNEL_ENV_MODE:
			verboselog(0, "audio_r: Channel Envelope Enable: %04x\n", data);
			break;

		case AUDIO_CHANNEL_TONE_RELEASE:
			verboselog(0, "audio_r: Channel Tone Release Enable: %04x\n", data);
			break;

		case AUDIO_CHANNEL_ENV_IRQ:
			verboselog(0, "audio_r: Channel Envelope IRQ Status: %04x\n", data);
			break;

		case AUDIO_CHANNEL_PITCH_BEND:
			verboselog(0, "audio_r: Channel Pitch Bend Enable: %04x\n", data);
			break;

		case AUDIO_SOFT_PHASE:
			verboselog(0, "audio_r: Software Channel Phase: %04x\n", data);
			break;

		case AUDIO_ATTACK_RELEASE:
			verboselog(0, "audio_r: Attack/Release Time Control: %04x\n", data);
			break;

		case AUDIO_EQ_CUTOFF10:
			verboselog(0, "audio_r: EQ Cutoff Frequency 0/1: %04x\n", data);
			break;

		case AUDIO_EQ_CUTOFF32:
			verboselog(0, "audio_r: EQ Cutoff Frequency 2/3: %04x\n", data);
			break;

		case AUDIO_EQ_GAIN10:
			verboselog(0, "audio_r: EQ Cutoff Gain 0/1: %04x\n", data);
			break;

		case AUDIO_EQ_GAIN32:
			verboselog(0, "audio_r: EQ Cutoff Gain 2/3: %04x\n", data);
			break;

		default:
			verboselog(0, "audio_r: Unknown register %04x (%04x & %04x)\n", 0x3000 + offset, data, mem_mask);
			break;
		}
	}
	else if (channel < 6)
	{
		switch (offset & AUDIO_CHAN_OFFSET_MASK)
		{
		case AUDIO_WAVE_ADDR:
			verboselog(0, "audio_r: Channel %d: Wave Addr (lo): %04x\n", channel, data);
			break;

		case AUDIO_MODE:
			verboselog(0, "audio_r: Channel %d: Mode: %04x (ADPCM:%d, 16M:%d, TONE:%d, LADDR_HI:%04x, WADDR_HI:%04x)\n", channel, data,
				get_adpcm_bit(channel), get_16bit_bit(channel), get_tone_mode(channel), get_loop_addr_high(channel), get_wave_addr_high(channel));
			break;

		case AUDIO_LOOP_ADDR:
			verboselog(0, "audio_r: Channel %d: Loop Addr: %04x\n", channel, data);
			break;

		case AUDIO_PAN_VOL:
			verboselog(0, "audio_r: Channel %d: Pan/Vol: %04x (PAN:%02x, VOL:%02x)\n", channel, data,
				get_pan(channel), get_volume(channel));
			break;

		case AUDIO_ENVELOPE0:
			verboselog(0, "audio_r: Channel %d: Envelope0: %04x (RPTPER:%d, TARGET:%02x, SIGN:%d, INC:%02x)\n", channel, data,
				get_repeat_period_bit(channel), get_envelope_target(channel), get_envelope_sign_bit(channel), get_envelope_inc(channel));
			break;

		case AUDIO_ENVELOPE_DATA:
			verboselog(0, "audio_r: Channel %d: Envelope Data: %04x (CNT:%d, EDD:%02x)\n", channel, data,
				get_envelope_count(channel), get_edd(channel));
			break;

		case AUDIO_ENVELOPE1:
			verboselog(0, "audio_r: Channel %d: Envelope1 Data: %04x (RPTCNT:%02x, RPT:%d, LOAD:%02x)\n", channel, data,
				get_envelope_repeat_count(channel), get_envelope_repeat_bit(channel), get_envelope_load(channel));
			break;

		case AUDIO_ENVELOPE_ADDR_HIGH:
			verboselog(0, "audio_r: Channel %d: Envelope Addr (hi): %04x (IRQADDR:%03x, IRQEN:%d, EADDR_HI:%02x)\n", channel, data,
				get_audio_irq_addr(channel), get_audio_irq_enable_bit(channel), get_envelope_addr_high(channel));
			break;

		case AUDIO_ENVELOPE_ADDR:
			verboselog(0, "audio_r: Channel %d: Envelope Addr (lo): %04x \n", channel, data);
			break;

		case AUDIO_WAVE_DATA_PREV:
			verboselog(0, "audio_r: Channel %d: Wave Data Prev: %04x \n", channel, data);
			break;

		case AUDIO_ENVELOPE_LOOP_CTRL:
			verboselog(0, "audio_r: Channel %d: Envelope Loop Ctrl: %04x (RDOFFS:%02x, EAOFFS:%03x)\n", channel, data,
				get_rampdown_offset(channel), get_envelope_eaoffset(channel));
			break;

		case AUDIO_WAVE_DATA:
			verboselog(0, "audio_r: Channel %d: Wave Data: %04x\n", channel, data);
			break;

		case AUDIO_ADPCM_SEL:
			verboselog(0, "audio_r: Channel %d: ADPCM Sel: %04x (ADPCM36:%d, POINTNUM:%02x\n", channel, data,
				get_adpcm36_bit(channel), get_point_number(channel));
			break;

		case AUDIO_PHASE_HIGH:
			verboselog(0, "audio_r: Channel %d: Phase High: %04x\n", channel, data);
			break;

		case AUDIO_PHASE_ACCUM_HIGH:
			verboselog(0, "audio_r: Channel %d: Phase Accum High: %04x\n", channel, data);
			break;

		case AUDIO_TARGET_PHASE_HIGH:
			verboselog(0, "audio_r: Channel %d: Target Phase High: %04x\n", channel, data);
			break;

		case AUDIO_RAMP_DOWN_CLOCK:
			verboselog(0, "audio_r: Channel %d: Rampdown Clock: %04x\n", channel, data);
			break;

		case AUDIO_PHASE:
			verboselog(0, "audio_r: Channel %d: Phase: %04x\n", channel, data);
			break;

		case AUDIO_PHASE_ACCUM:
			verboselog(0, "audio_r: Channel %d: Phase Accum: %04x\n", channel, data);
			break;

		case AUDIO_TARGET_PHASE:
			verboselog(0, "audio_r: Channel %d: Target Phase: %04x\n", channel, data);
			break;

		case AUDIO_PHASE_CTRL:
			verboselog(0, "audio_r: Channel %d: Phase Ctrl: %04x (TIMESTEP:%d, SIGN:%d, OFFSET:%03x\n", channel, data,
				get_phase_time_step(channel), get_phase_sign_bit(channel), get_phase_offset(channel));
			break;

		default:
			verboselog(4, "audio_r: Unknown register %04x\n", 0x3000 + offset);
			break;
		}
	}
	return data;
}

WRITE16_MEMBER(spg2xx_device::audio_w)
{
	const uint16_t channel = (offset & 0x00f0) >> 4;

	if (offset >= 0x400)
	{
		switch (offset)
		{
		case AUDIO_CHANNEL_ENABLE:
		{
			verboselog(0, "audio_w: Channel Enable: %04x\n", data);
			const uint16_t changed = m_audio_regs[AUDIO_CHANNEL_STATUS] ^ data;
			for (uint32_t channel_bit = 0; channel_bit < 6; channel_bit++)
			{
				const uint16_t mask = 1 << channel_bit;
				if (!(changed & mask))
					continue;

				if (data & mask)
				{
					//printf("Enabling channel %d\n", channel_bit);
					m_audio_regs[offset] |= mask;
					if (!(m_audio_regs[AUDIO_CHANNEL_STOP] & mask))
					{
						//printf("Stop not set, starting playback on channel %d, mask %04x\n", channel_bit, mask);
						m_audio_regs[AUDIO_CHANNEL_STATUS] |= mask;
						m_sample_addr[channel_bit] = get_wave_addr(channel_bit);
						m_envelope_addr[channel_bit] = get_envelope_addr(channel_bit);
						set_envelope_count(channel, get_envelope_load(channel));
					}
					m_adpcm[channel_bit].reset();
					m_sample_shift[channel_bit] = 0;
					m_sample_count[channel_bit] = 0;
				}
				else
				{
					m_audio_regs[offset] &= ~mask;
					m_audio_regs[AUDIO_CHANNEL_STATUS] &= ~mask;
					m_audio_regs[AUDIO_CHANNEL_STOP] |= mask;
					m_audio_regs[AUDIO_CHANNEL_TONE_RELEASE] &= ~mask;
				}
			}
			break;
		}

		case AUDIO_MAIN_VOLUME:
			verboselog(0, "audio_w: Main Volume: %04x\n", data);
			m_audio_regs[offset] = data & AUDIO_MAIN_VOLUME_MASK;
			break;

		case AUDIO_CHANNEL_FIQ_ENABLE:
			verboselog(0, "audio_w: Channel FIQ Enable: %04x\n", data);
			m_audio_regs[offset] = data & AUDIO_CHANNEL_FIQ_ENABLE_MASK;
			break;

		case AUDIO_CHANNEL_FIQ_STATUS:
			verboselog(0, "audio_w: Channel FIQ Acknowledge: %04x\n", data);
			m_audio_regs[offset] &= ~(data & AUDIO_CHANNEL_FIQ_STATUS_MASK);
			break;

		case AUDIO_BEAT_BASE_COUNT:
			verboselog(0, "audio_w: Beat Base Count: %04x\n", data);
			m_audio_regs[offset] = data & AUDIO_BEAT_BASE_COUNT_MASK;
			m_audio_curr_beat_base_count = m_audio_regs[offset];
			break;

		case AUDIO_BEAT_COUNT:
		{
			verboselog(0, "audio_w: Beat Count: %04x\n", data);
			const uint16_t old = m_audio_regs[offset];
			m_audio_regs[offset] &= ~(data & AUDIO_BIS_MASK);
			m_audio_regs[offset] &= AUDIO_BIS_MASK;
			m_audio_regs[offset] |= data & ~AUDIO_BIS_MASK;
			const uint16_t changed = old ^ m_audio_regs[offset];
			if (changed & (AUDIO_BIS_MASK | AUDIO_BIE_MASK))
			{
				verboselog(4, "BIS mask changed, updating IRQ\n");
				check_irqs(changed & (AUDIO_BIS_MASK | AUDIO_BIE_MASK));
			}
			break;
		}

		case AUDIO_ENVCLK:
		{
			verboselog(0, "audio_w: Envelope Interval (lo): %04x\n", data);
			const uint16_t old = m_audio_regs[offset];
			m_audio_regs[offset] = data;
			const uint16_t changed = old ^ m_audio_regs[offset];

			if (!changed)
			{
				if (SPG_DEBUG_ENVELOPES) logerror("No change, %04x %04x %04x\n", old, data, m_audio_regs[offset]);
				break;
			}

			for (uint8_t channel_bit = 0; channel_bit < 4; channel_bit++)
			{
				const uint8_t shift = channel_bit << 2;
				const uint16_t mask = 0x0f << shift;
				if (changed & mask)
				{
					m_envclk_frame[channel_bit] = get_envclk_frame_count(channel_bit);
					if (SPG_DEBUG_ENVELOPES) logerror("envclk_frame %d: %08x\n", channel_bit, m_envclk_frame[channel_bit]);
				}
			}
			break;
		}

		case AUDIO_ENVCLK_HIGH:
		{
			verboselog(0, "audio_w: Envelope Interval (hi): %04x\n", data);
			const uint16_t old = m_audio_regs[offset];
			m_audio_regs[offset] = data & AUDIO_ENVCLK_HIGH_MASK;
			const uint16_t changed = old ^ m_audio_regs[offset];
			if (!changed)
			{
				if (SPG_DEBUG_ENVELOPES) logerror("No change, %04x %04x %04x\n", old, data, m_audio_regs[offset]);
				break;
			}

			for (uint8_t channel_bit = 0; channel_bit < 2; channel_bit++)
			{
				const uint8_t shift = channel_bit << 2;
				const uint16_t mask = 0x0f << shift;
				if (changed & mask)
				{
					m_envclk_frame[channel_bit + 4] = get_envclk_frame_count(channel_bit);
					if (SPG_DEBUG_ENVELOPES) logerror("envclk_frame %d: %08x\n", channel_bit + 4, m_envclk_frame[channel_bit + 4]);
				}
			}
			break;
		}

		case AUDIO_ENV_RAMP_DOWN:
		{
			verboselog(0, "audio_w: Envelope Fast Ramp Down: %04x\n", data);
			const uint16_t old = m_audio_regs[offset];
			m_audio_regs[offset] = data & AUDIO_ENV_RAMP_DOWN_MASK;
			const uint16_t changed = old ^ m_audio_regs[offset];
			if (!changed)
			{
				//logerror("No change, %04x %04x %04x\n", old, data, m_audio_regs[offset]);
				//printf("No change, %04x %04x %04x\n", old, data, m_audio_regs[offset]);
				break;
			}

			for (uint32_t channel_bit = 0; channel_bit < 6; channel_bit++)
			{
				const uint16_t mask = 1 << channel_bit;
				if ((changed & mask) && (data & mask))
				{
					m_rampdown_frame[channel_bit] = get_rampdown_frame_count(channel_bit);
					//printf("Preparing to ramp down channel %d in %d ticks\n", channel_bit, m_rampdown_frame[channel_bit] / 13);
				}
			}
			break;
		}

		case AUDIO_CHANNEL_STOP:
			verboselog(0, "audio_w: Channel Stop Status: %04x\n", data);
			m_audio_regs[offset] &= ~(data & AUDIO_CHANNEL_STOP_MASK);
			break;

		case AUDIO_CHANNEL_ZERO_CROSS:
			verboselog(0, "audio_w: Channel Zero-Cross Enable: %04x\n", data);
			m_audio_regs[offset] = data & AUDIO_CHANNEL_ZERO_CROSS_MASK;
			break;

		case AUDIO_CONTROL:
			verboselog(0, "audio_w: Control: %04x (SOFTCH:%d, COMPEN:%d, NOHIGH:%d, NOINT:%d, EQEN:%d\n", data
				, (data & AUDIO_CONTROL_SOFTCH_MASK) ? 1 : 0
				, (data & AUDIO_CONTROL_COMPEN_MASK) ? 1 : 0
				, (data & AUDIO_CONTROL_NOHIGH_MASK) ? 1 : 0
				, (data & AUDIO_CONTROL_NOINT_MASK) ? 1 : 0
				, (data & AUDIO_CONTROL_EQEN_MASK) ? 1 : 0);
			m_audio_regs[offset] = data & AUDIO_CONTROL_MASK;
			break;

		case AUDIO_COMPRESS_CTRL:
			verboselog(0, "audio_w: Compressor Control: %04x\n", data);
			m_audio_regs[offset] = data;
			break;

		case AUDIO_CHANNEL_STATUS:
			verboselog(0, "audio_w: Channel Status (read-only): %04x\n", data);
			break;

		case AUDIO_WAVE_IN_L:
			verboselog(0, "audio_w: Wave In (L) / FIFO Write Data: %04x\n", data);
			m_audio_regs[offset] = data;
			break;

		case AUDIO_WAVE_IN_R:
			verboselog(0, "audio_w: Wave In (R) / Software Channel FIFO IRQ Control: %04x\n", data);
			m_audio_regs[offset] = data;
			break;

		case AUDIO_WAVE_OUT_L:
			verboselog(0, "audio_w: Wave Out (L): %04x\n", data);
			m_audio_regs[offset] = data;
			break;

		case AUDIO_WAVE_OUT_R:
			verboselog(0, "audio_w: Wave Out (R): %04x\n", data);
			m_audio_regs[offset] = data;
			break;

		case AUDIO_CHANNEL_REPEAT:
			verboselog(0, "audio_w: Channel Repeat Enable: %04x\n", data);
			m_audio_regs[offset] = data & AUDIO_CHANNEL_REPEAT_MASK;
			break;

		case AUDIO_CHANNEL_ENV_MODE:
			verboselog(0, "audio_w: Channel Envelope Enable: %04x\n", data);
			m_audio_regs[offset] = data & AUDIO_CHANNEL_ENV_MODE_MASK;
			break;

		case AUDIO_CHANNEL_TONE_RELEASE:
			verboselog(0, "audio_w: Channel Tone Release Enable: %04x\n", data);
			m_audio_regs[offset] = data & AUDIO_CHANNEL_TONE_RELEASE_MASK;
			break;

		case AUDIO_CHANNEL_ENV_IRQ:
			verboselog(0, "audio_w: Channel Envelope IRQ Acknowledge: %04x\n", data);
			m_audio_regs[offset] &= ~data & AUDIO_CHANNEL_ENV_IRQ_MASK;
			break;

		case AUDIO_CHANNEL_PITCH_BEND:
			verboselog(0, "audio_w: Channel Pitch Bend Enable: %04x\n", data);
			m_audio_regs[offset] = data & AUDIO_CHANNEL_PITCH_BEND_MASK;
			break;

		case AUDIO_SOFT_PHASE:
			verboselog(0, "audio_w: Software Channel Phase: %04x\n", data);
			m_audio_regs[offset] = data;
			break;

		case AUDIO_ATTACK_RELEASE:
			verboselog(0, "audio_w: Attack/Release Time Control: %04x\n", data);
			m_audio_regs[offset] = data;
			break;

		case AUDIO_EQ_CUTOFF10:
			verboselog(0, "audio_w: EQ Cutoff Frequency 0/1: %04x\n", data);
			m_audio_regs[offset] = data & AUDIO_EQ_CUTOFF10_MASK;
			break;

		case AUDIO_EQ_CUTOFF32:
			verboselog(0, "audio_w: EQ Cutoff Frequency 2/3: %04x\n", data);
			m_audio_regs[offset] = data & AUDIO_EQ_CUTOFF32_MASK;
			break;

		case AUDIO_EQ_GAIN10:
			verboselog(0, "audio_w: EQ Cutoff Gain 0/1: %04x\n", data);
			m_audio_regs[offset] = data & AUDIO_EQ_GAIN10_MASK;
			break;

		case AUDIO_EQ_GAIN32:
			verboselog(0, "audio_w: EQ Cutoff Gain 2/3: %04x\n", data);
			m_audio_regs[offset] = data & AUDIO_EQ_GAIN32_MASK;
			break;

		default:
			m_audio_regs[offset] = data;
			verboselog(0, "audio_w: Unknown register %04x = %04x (%04x)\n", 0x3000 + offset, data, mem_mask);
			break;
		}
	}
	else if (channel < 6)
	{
		switch (offset & AUDIO_CHAN_OFFSET_MASK)
		{
		case AUDIO_WAVE_ADDR:
			m_audio_regs[offset] = data;
			verboselog(0, "audio_w: Channel %d: Wave Addr (lo): %04x\n", channel, data);
			break;

		case AUDIO_MODE:
			m_audio_regs[offset] = data;
			verboselog(0, "audio_w: Channel %d: Mode: %04x (ADPCM:%d, 16M:%d, TONE:%d, LADDR_HI:%04x, WADDR_HI:%04x)\n", channel, data,
				get_adpcm_bit(channel), get_16bit_bit(channel), get_tone_mode(channel), get_loop_addr_high(channel), get_wave_addr_high(channel));
			break;

		case AUDIO_LOOP_ADDR:
			m_audio_regs[offset] = data;
			verboselog(0, "audio_w: Channel %d: Loop Addr: %04x\n", channel, data);
			break;

		case AUDIO_PAN_VOL:
			m_audio_regs[offset] = data & AUDIO_PAN_VOL_MASK;
			verboselog(0, "audio_w: Channel %d: Pan/Vol: %04x (PAN:%02x, VOL:%02x)\n", channel, data,
				get_pan(channel), get_volume(channel));
			break;

		case AUDIO_ENVELOPE0:
			m_audio_regs[offset] = data;
			verboselog(0, "audio_w: Channel %d: Envelope0: %04x (RPTPER:%d, TARGET:%02x, SIGN:%d, INC:%02x)\n", channel, data,
				get_repeat_period_bit(channel), get_envelope_target(channel), get_envelope_sign_bit(channel), get_envelope_inc(channel));
			break;

		case AUDIO_ENVELOPE_DATA:
			m_audio_regs[offset] = data & AUDIO_ENVELOPE_DATA_MASK;
			verboselog(0, "audio_w: Channel %d: Envelope Data: %04x (CNT:%d, EDD:%02x)\n", channel, data,
				get_envelope_count(channel), get_edd(channel));
			break;

		case AUDIO_ENVELOPE1:
			m_audio_regs[offset] = data;
			verboselog(0, "audio_w: Channel %d: Envelope1 Data: %04x (RPTCNT:%02x, RPT:%d, LOAD:%02x)\n", channel, data,
				get_envelope_repeat_count(channel), get_envelope_repeat_bit(channel), get_envelope_load(channel));
			break;

		case AUDIO_ENVELOPE_ADDR_HIGH:
			m_audio_regs[offset] = data;
			verboselog(0, "audio_w: Channel %d: Envelope Addr (hi): %04x (IRQADDR:%03x, IRQEN:%d, EADDR_HI:%02x)\n", channel, data,
				get_audio_irq_addr(channel), get_audio_irq_enable_bit(channel), get_envelope_addr_high(channel));
			break;

		case AUDIO_ENVELOPE_ADDR:
			m_audio_regs[offset] = data;
			verboselog(0, "audio_w: Channel %d: Envelope Addr (lo): %04x\n", channel, data);
			break;

		case AUDIO_WAVE_DATA_PREV:
			m_audio_regs[offset] = data;
			verboselog(0, "audio_w: Channel %d: Wave Data Prev: %04x \n", channel, data);
			break;

		case AUDIO_ENVELOPE_LOOP_CTRL:
			m_audio_regs[offset] = data;
			verboselog(0, "audio_w: Channel %d: Envelope Loop Ctrl: %04x (RDOFFS:%02x, EAOFFS:%03x)\n", channel, data,
				get_rampdown_offset(channel), get_envelope_eaoffset(channel));
			break;

		case AUDIO_WAVE_DATA:
			m_audio_regs[offset] = data;
			verboselog(0, "audio_w: Channel %d: Wave Data: %04x\n", channel, data);
			break;

		case AUDIO_ADPCM_SEL:
			m_audio_regs[offset] = data & AUDIO_ADPCM_SEL_MASK;
			verboselog(0, "audio_w: Channel %d: ADPCM Sel: %04x (ADPCM36:%d, POINTNUM:%02x\n", channel, data,
				get_adpcm36_bit(channel), get_point_number(channel));
			break;

		case AUDIO_PHASE_HIGH:
			m_audio_regs[offset] = data & AUDIO_PHASE_HIGH_MASK;
			m_channel_rate[channel] = ((double)get_phase(channel) * 140625.0 * 2.0) / (double)(1 << 19);
			m_channel_rate_accum[channel] = 0.0;
			verboselog(0, "audio_w: Channel %d: Phase High: %04x (rate: %f)\n", channel, data, m_channel_rate[channel]);
			break;

		case AUDIO_PHASE_ACCUM_HIGH:
			m_audio_regs[offset] = data & AUDIO_PHASE_ACCUM_HIGH_MASK;
			verboselog(0, "audio_w: Channel %d: Phase Accum High: %04x\n", channel, data);
			break;

		case AUDIO_TARGET_PHASE_HIGH:
			m_audio_regs[offset] = data & AUDIO_TARGET_PHASE_HIGH_MASK;
			verboselog(0, "audio_w: Channel %d: Target Phase High: %04x\n", channel, data);
			break;

		case AUDIO_RAMP_DOWN_CLOCK:
			m_audio_regs[offset] = data & AUDIO_RAMP_DOWN_CLOCK_MASK;
			verboselog(0, "audio_w: Channel %d: Rampdown Clock: %04x\n", channel, data);
			break;

		case AUDIO_PHASE:
			m_audio_regs[offset] = data;
			m_channel_rate[channel] = ((double)get_phase(channel) * 140625.0 * 2.0) / (double)(1 << 19);
			m_channel_rate_accum[channel] = 0.0;
			verboselog(0, "audio_w: Channel %d: Phase: %04x (rate: %f)\n", channel, data, m_channel_rate[channel]);
			break;

		case AUDIO_PHASE_ACCUM:
			m_audio_regs[offset] = data;
			verboselog(0, "audio_w: Channel %d: Phase Accum: %04x\n", channel, data);
			break;

		case AUDIO_TARGET_PHASE:
			m_audio_regs[offset] = data;
			verboselog(0, "audio_w: Channel %d: Target Phase: %04x\n", channel, data);
			break;

		case AUDIO_PHASE_CTRL:
			m_audio_regs[offset] = data;
			verboselog(0, "audio_w: Channel %d: Phase Ctrl: %04x (TIMESTEP:%d, SIGN:%d, OFFSET:%03x\n", channel, data,
				get_phase_time_step(channel), get_phase_sign_bit(channel), get_phase_offset(channel));
			break;

		default:
			m_audio_regs[offset] = data;
			verboselog(0, "audio_w: Unknown register %04x = %04x (%04x)\n", 0x3000 + offset, data, mem_mask);
			break;
		}
	}
	else
	{
		m_audio_regs[offset] = data;
	}
}

void spg2xx_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *out_l = outputs[0];
	stream_sample_t *out_r = outputs[1];
	address_space &space = m_cpu->space(AS_PROGRAM);

	for (int i = 0; i < samples; i++)
	{
		int32_t left_total = 0;
		int32_t right_total = 0;
		int32_t active_count = 0;

#if SPG2XX_VISUAL_AUDIO_DEBUG
		for (int y = 0; y < 480; y++)
		{
			m_audio_debug_buffer[y*640 + m_audio_debug_x] = 0;
		}
#endif
		for (uint32_t ch_index = 0; ch_index < 6; ch_index++)
		{
#if SPG2XX_VISUAL_AUDIO_DEBUG
			const uint32_t debug_y = ch_index * 80;
#endif
			if (!get_channel_status(ch_index))
			{
#if SPG2XX_VISUAL_AUDIO_DEBUG
				m_audio_debug_buffer[(debug_y + 79)*640 + m_audio_debug_x] |= SPG_VDB_WAVE;
				int edd_y = (int)((float)get_edd(ch_index) * 0.625f);
				m_audio_debug_buffer[(debug_y + edd_y)*640 + m_audio_debug_x] |= SPG_VDB_EDD;
				int vol_y = (int)((float)get_volume(ch_index) * 0.625f);
				m_audio_debug_buffer[(debug_y + vol_y)*640 + m_audio_debug_x] |= SPG_VDB_VOL;
#endif
				continue;
			}

			if (SPG_DEBUG_AUDIO && m_debug_rates)
				printf("%f:%f ", m_channel_rate[ch_index], m_channel_rate_accum[ch_index]);
			bool playing = advance_channel(space, ch_index);
			if (playing)
			{
				int32_t sample = (int16_t)(m_audio_regs[(ch_index << 4) | AUDIO_WAVE_DATA] ^ 0x8000);
#if SPG2XX_VISUAL_AUDIO_DEBUG
				int sample_y = (int)((float)m_audio_regs[(ch_index << 4) | AUDIO_WAVE_DATA] / 819.0f);
				m_audio_debug_buffer[(debug_y + sample_y)*640 + m_audio_debug_x] |= SPG_VDB_WAVE;
				int edd_y = (int)((float)get_edd(ch_index) * 0.625f);
				m_audio_debug_buffer[(debug_y + edd_y)*640 + m_audio_debug_x] |= SPG_VDB_EDD;
				int vol_y = (int)((float)get_volume(ch_index) * 0.625f);
				m_audio_debug_buffer[(debug_y + vol_y)*640 + m_audio_debug_x] |= SPG_VDB_VOL;
#endif
				if (!(m_audio_regs[AUDIO_CONTROL] & AUDIO_CONTROL_NOINT_MASK))
				{
					int32_t prev_sample = (int16_t)(m_audio_regs[(ch_index << 4) | AUDIO_WAVE_DATA_PREV] ^ 0x8000);
					int16_t lerp_factor = (int16_t)((m_channel_rate_accum[ch_index] / 44100.0) * 256.0);
					prev_sample = (prev_sample * (0x100 - lerp_factor)) >> 8;
					sample = (sample * lerp_factor) >> 8;
					sample += prev_sample;
				}

				sample = (sample * (int16_t)get_edd(ch_index)) >> 7;

				active_count++;

				int32_t vol = get_volume(ch_index);
				int32_t pan = get_pan(ch_index);

				int32_t pan_left, pan_right;
				if (pan < 0x40)
				{
					pan_left = 0x7f * vol;
					pan_right = pan * 2 * vol;
				}
				else
				{
					pan_left = (0x7f - pan) * 2 * vol;
					pan_right = 0x7f * vol;
				}

				left_total += ((int16_t)sample * (int16_t)pan_left) >> 14;
				right_total += ((int16_t)sample * (int16_t)pan_right) >> 14;
			}
		}

#if SPG2XX_VISUAL_AUDIO_DEBUG
		advance_debug_pos();
#endif

		if (active_count)
		{
			*out_l++ = left_total / active_count;
			*out_r++ = right_total / active_count;
		}
		else
		{
			*out_l++ = 0;
			*out_r++ = 0;
		}

#if SPG2XX_VISUAL_AUDIO_DEBUG
		for (int y = 0; y < 480; y++)
		{
			m_audio_debug_buffer[y*640 + m_audio_debug_x] = 7;
		}
#endif
	}
}

inline void spg2xx_device::stop_channel(const uint32_t channel)
{
	// TODO: IRQs
	m_audio_regs[AUDIO_CHANNEL_STATUS] &= ~(1 << channel);
	m_audio_regs[AUDIO_CHANNEL_STOP] |= (1 << channel);
	m_audio_regs[(channel << 4) | AUDIO_MODE] &= ~AUDIO_ADPCM_MASK;
	m_audio_regs[AUDIO_CHANNEL_TONE_RELEASE] &= ~(1 << channel);
}

bool spg2xx_device::advance_channel(address_space &space, const uint32_t channel)
{
	m_channel_rate_accum[channel] += m_channel_rate[channel];
	uint32_t samples_to_advance = 0;
	while (m_channel_rate_accum[channel] >= 44100.0)
	{
		m_channel_rate_accum[channel] -= 44100.0;
		samples_to_advance++;
	}

	if (!samples_to_advance)
		return true;

	bool playing = true;

	if (get_adpcm_bit(channel))
	{
		// ADPCM mode
		for (uint32_t sample = 0; sample < samples_to_advance && playing; sample++)
		{
			playing = fetch_sample(space, channel);
			if (playing)
			{
				m_sample_shift[channel] += 4;
				if (m_sample_shift[channel] == 16)
				{
					m_sample_shift[channel] = 0;
					m_sample_addr[channel]++;
				}
			}
		}
	}
	else if (get_16bit_bit(channel))
	{
		// 16-bit mode
		for (uint32_t sample = 0; sample < samples_to_advance && playing; sample++)
		{
			playing = fetch_sample(space, channel);
			if (playing)
				m_sample_addr[channel]++;
		}
	}
	else
	{
		// 8-bit mode
		for (uint32_t sample = 0; sample < samples_to_advance && playing; sample++)
		{
			playing = fetch_sample(space, channel);
			if (playing)
			{
				m_sample_shift[channel] += 8;
				if (m_sample_shift[channel] == 16)
				{
					m_sample_shift[channel] = 0;
					m_sample_addr[channel]++;
				}
			}
		}
	}

	return playing;
}

bool spg2xx_device::fetch_sample(address_space &space, const uint32_t channel)
{
	const uint32_t channel_mask = channel << 4;
	m_audio_regs[channel_mask | AUDIO_WAVE_DATA_PREV] = m_audio_regs[channel_mask | AUDIO_WAVE_DATA];

	const uint32_t wave_data_reg = channel_mask | AUDIO_WAVE_DATA;
	const uint16_t tone_mode = get_tone_mode(channel);
	const uint16_t raw_sample = tone_mode ? space.read_word(m_sample_addr[channel]) : m_audio_regs[wave_data_reg];

	m_audio_regs[wave_data_reg] = raw_sample;

	if (get_adpcm_bit(channel))
	{
		// ADPCM mode
		m_audio_regs[wave_data_reg] >>= m_sample_shift[channel];
		m_audio_regs[wave_data_reg] = (uint16_t)(m_adpcm[channel].clock((uint8_t)(m_audio_regs[wave_data_reg] & 0x000f)) * 8) ^ 0x8000;
		if (tone_mode != 0 && raw_sample == 0xffff)
		{
			if (tone_mode == AUDIO_TONE_MODE_HW_ONESHOT)
			{
				if (SPG_DEBUG_SAMPLES) logerror("ADPCM stopped after %d samples\n", m_sample_count[channel]);
				m_sample_count[channel] = 0;
				stop_channel(channel);
				return false;
			}
			else
			{
				if (SPG_DEBUG_SAMPLES) logerror("ADPCM looping after %d samples\n", m_sample_count[channel]);
				m_sample_count[channel] = 0;
				loop_channel(channel);
			}
		}
		m_sample_count[channel]++;
	}
	else if (get_16bit_bit(channel))
	{
		// 16-bit mode
		if (tone_mode != 0 && raw_sample == 0xffff)
		{
			if (tone_mode == AUDIO_TONE_MODE_HW_ONESHOT)
			{
				if (SPG_DEBUG_SAMPLES) logerror("16-bit PCM stopped after %d samples\n", m_sample_count[channel]);
				m_sample_count[channel] = 0;
				stop_channel(channel);
				return false;
			}
			else
			{
				if (SPG_DEBUG_SAMPLES) logerror("16-bit PCM looping after %d samples\n", m_sample_count[channel]);
				m_sample_count[channel] = 0;
				loop_channel(channel);
			}
		}
		m_sample_count[channel]++;
	}
	else
	{
		// 8-bit mode
		if (tone_mode != 0)
		{
			if (m_sample_shift[channel])
				m_audio_regs[wave_data_reg] <<= 8;
			else
				m_audio_regs[wave_data_reg] &= 0xff00;

			if (m_audio_regs[wave_data_reg] == 0xff00)
			{
				if (tone_mode == AUDIO_TONE_MODE_HW_ONESHOT)
				{
					if (SPG_DEBUG_SAMPLES) logerror("8-bit PCM stopped after %d samples\n", m_sample_count[channel]);
					m_sample_count[channel] = 0;
					stop_channel(channel);
					return false;
				}
				else
				{
					if (SPG_DEBUG_SAMPLES) logerror("8-bit PCM looping after %d samples\n", m_sample_count[channel]);
					m_sample_count[channel] = 0;
					loop_channel(channel);
				}
			}
		}
		m_sample_count[channel]++;
	}

	return true;
}

inline void spg2xx_device::loop_channel(const uint32_t channel)
{
	//printf("Looping from %08x to %08x\n", m_sample_addr[channel], get_loop_addr(channel));
	m_sample_addr[channel] = get_loop_addr(channel);
	m_sample_shift[channel] = 0;
}

void spg2xx_device::audio_frame_tick()
{
	audio_beat_tick();

	address_space &space = m_cpu->space(AS_PROGRAM);
	for (uint32_t channel = 0; channel < 6; channel++)
	{
		const uint16_t mask = (1 << channel);
		if (!(m_audio_regs[AUDIO_CHANNEL_STATUS] & mask))
		{
			//if (SPG_DEBUG_ENVELOPES) printf("Skipping channel due to status\n");
			continue;
		}

		if (m_audio_regs[AUDIO_ENV_RAMP_DOWN] & mask)
		{
			m_rampdown_frame[channel]--;
			if (m_rampdown_frame[channel] == 0)
			{
				logerror("Ticking rampdown for channel %d\n", channel);
				audio_rampdown_tick(channel);
				continue;
			}
		}

		if (!(m_audio_regs[AUDIO_CHANNEL_ENV_MODE] & mask))
		{
			m_envclk_frame[channel]--;
			if (m_envclk_frame[channel] == 0)
			{
				if (SPG_DEBUG_ENVELOPES) logerror("Ticking envelope for channel %d\n", channel);
				audio_envelope_tick(space, channel);
				m_envclk_frame[channel] = get_envclk_frame_count(channel);
			}
		}
		else if (SPG_DEBUG_ENVELOPES)
		{
			//logerror("Not ticking envelope for channel %d\n", channel);
		}
	}
}

void spg2xx_device::audio_beat_tick()
{
	uint16_t beat_count = m_audio_regs[AUDIO_BEAT_COUNT] & AUDIO_BEAT_COUNT_MASK;
	if (beat_count == 0)
		return;

	m_audio_curr_beat_base_count--;
	if (m_audio_curr_beat_base_count == 0)
	{
		//printf("Beat base count elapsed, reloading with %d\n", m_audio_regs[AUDIO_BEAT_BASE_COUNT]);
		m_audio_curr_beat_base_count = m_audio_regs[AUDIO_BEAT_BASE_COUNT];

		beat_count--;
		m_audio_regs[AUDIO_BEAT_COUNT] = (m_audio_regs[AUDIO_BEAT_COUNT] & ~AUDIO_BEAT_COUNT_MASK) | beat_count;

		if (beat_count == 0)
		{
			if (m_audio_regs[AUDIO_BEAT_COUNT] & AUDIO_BIE_MASK)
			{
				//printf("Beat count elapsed, triggering IRQ\n");
				m_audio_regs[AUDIO_BEAT_COUNT] |= AUDIO_BIS_MASK;
				check_irqs(AUDIO_BIS_MASK);
			}
			else
			{
				//printf("Beat count elapsed but IRQ not enabled\n");
			}
		}
	}
}

void spg2xx_device::audio_rampdown_tick(const uint32_t channel)
{
	const uint8_t old_edd = get_edd(channel);
	uint8_t new_edd = old_edd - get_rampdown_offset(channel);
	if (new_edd > old_edd)
		new_edd = 0;

	if (new_edd)
	{
		if (SPG_DEBUG_ENVELOPES) logerror("Channel %d preparing for next rampdown step (%02x)\n", channel, new_edd);
		const uint16_t channel_mask = channel << 4;
		m_audio_regs[channel_mask | AUDIO_ENVELOPE_DATA] &= ~AUDIO_EDD_MASK;
		m_audio_regs[channel_mask | AUDIO_ENVELOPE_DATA] |= new_edd & AUDIO_EDD_MASK;
		m_rampdown_frame[channel] = get_rampdown_frame_count(channel);
	}
	else
	{
		if (SPG_DEBUG_ENVELOPES) logerror("Stopping channel %d due to rampdown\n", channel);
		const uint16_t channel_mask = 1 << channel;
		m_audio_regs[AUDIO_CHANNEL_ENABLE] &= ~channel_mask;
		m_audio_regs[AUDIO_CHANNEL_STATUS] &= ~channel_mask;
		m_audio_regs[AUDIO_CHANNEL_STOP] |= channel_mask;
		m_audio_regs[AUDIO_ENV_RAMP_DOWN] &= ~channel_mask;
		m_audio_regs[AUDIO_CHANNEL_TONE_RELEASE] &= ~channel_mask;
	}
}

const uint32_t spg2xx_device::s_rampdown_frame_counts[8] =
{
	13*4, 13*16, 13*64, 13*256, 13*1024, 13*4096, 13*8192, 13*8192
};

uint32_t spg2xx_device::get_rampdown_frame_count(const uint32_t channel)
{
	return s_rampdown_frame_counts[get_rampdown_clock(channel)];
}

const uint32_t spg2xx_device::s_envclk_frame_counts[16] =
{
	4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 8192, 8192, 8192, 8192
};

uint32_t spg2xx_device::get_envclk_frame_count(const uint32_t channel)
{
	return s_envclk_frame_counts[get_envelope_clock(channel)];
}

uint32_t spg2xx_device::get_envelope_clock(const offs_t channel) const
{
	if (channel < 4)
		return (m_audio_regs[AUDIO_ENVCLK] >> (channel << 2)) & 0x000f;
	else
		return (m_audio_regs[AUDIO_ENVCLK_HIGH] >> ((channel - 4) << 2)) & 0x000f;
}

void spg2xx_device::audio_envelope_tick(address_space &space, const uint32_t channel)
{
	const uint16_t channel_mask = channel << 4;
	uint16_t new_count = get_envelope_count(channel);
	const uint16_t curr_edd = get_edd(channel);
	if (SPG_DEBUG_ENVELOPES) logerror("envelope %d tick, count is %04x, curr edd is %04x\n", channel, new_count, curr_edd);
	if (new_count == 0)
	{
		const uint16_t target = get_envelope_target(channel);
		uint16_t new_edd = curr_edd;
		const uint16_t inc = get_envelope_inc(channel);

		if (new_edd != target)
		{
			if (get_envelope_sign_bit(channel))
			{
				new_edd -= inc;
				if (SPG_DEBUG_ENVELOPES) logerror("Envelope %d new EDD-: %04x (%04x), dec %04x\n", channel, new_edd, target, inc);
				if (new_edd > curr_edd)
					new_edd = 0;
				else if (new_edd < target)
					new_edd = target;

				if (new_edd == 0)
				{
					if (SPG_DEBUG_ENVELOPES) logerror("Envelope %d at 0, stopping channel\n", channel);
					stop_channel(channel);
					return;
				}
			}
			else
			{
				new_edd += inc;
				if (SPG_DEBUG_ENVELOPES) logerror("Envelope %d new EDD+: %04x\n", channel, new_edd);
				if (new_edd >= target)
					new_edd = target;
			}
		}

		if (new_edd == target)
		{
			if (SPG_DEBUG_ENVELOPES) logerror("Envelope %d at target %04x\n", channel, target);
			new_edd = target;

			if (get_envelope_repeat_bit(channel))
			{
				const uint16_t repeat_count = get_envelope_repeat_count(channel) - 1;
				if (SPG_DEBUG_ENVELOPES) logerror("Repeating envelope, new repeat count %d\n", repeat_count);
				if (repeat_count == 0)
				{
					m_audio_regs[channel_mask | AUDIO_ENVELOPE0] = space.read_word(m_envelope_addr[channel]);
					m_audio_regs[channel_mask | AUDIO_ENVELOPE1] = space.read_word(m_envelope_addr[channel] + 1);
					m_audio_regs[channel_mask | AUDIO_ENVELOPE_LOOP_CTRL] = space.read_word(m_envelope_addr[channel] + 2);
					m_envelope_addr[channel] = get_envelope_addr(channel) + get_envelope_eaoffset(channel);
					if (SPG_DEBUG_ENVELOPES) logerror("Envelope data after repeat: %04x %04x %04x (%08x)\n", m_audio_regs[channel_mask | AUDIO_ENVELOPE0], m_audio_regs[channel_mask | AUDIO_ENVELOPE1], m_audio_regs[channel_mask | AUDIO_ENVELOPE_LOOP_CTRL], m_envelope_addr[channel]);
				}
				else
				{
					set_envelope_repeat_count(channel, repeat_count);
				}
			}
			else
			{
				if (SPG_DEBUG_ENVELOPES) logerror("Fetching envelope for channel %d from %08x\n", channel, m_envelope_addr[channel]);
				m_audio_regs[channel_mask | AUDIO_ENVELOPE0] = space.read_word(m_envelope_addr[channel]);
				m_audio_regs[channel_mask | AUDIO_ENVELOPE1] = space.read_word(m_envelope_addr[channel] + 1);
				if (SPG_DEBUG_ENVELOPES) logerror("Fetched envelopes %04x %04x\n", m_audio_regs[channel_mask | AUDIO_ENVELOPE0], m_audio_regs[channel_mask | AUDIO_ENVELOPE1]);
				m_envelope_addr[channel] += 2;
			}
			// FIXME: This makes things sound markedly worse even though the docs indicate
			// the envelope count should be reloaded by the envelope load register! Why?
			set_envelope_count(channel, get_envelope_load(channel));
		}
		else
		{
			if (SPG_DEBUG_ENVELOPES) logerror("Envelope %d not yet at target %04x (%04x)\n", channel, target, new_edd);
			// FIXME: This makes things sound markedly worse even though the docs indicate
			// the envelope count should be reloaded by the envelope load register! So instead,
			// we just reload with zero, which at least keeps the channels going. Why?
			new_count = get_envelope_load(channel);
			set_envelope_count(channel, new_count);
		}
		if (SPG_DEBUG_ENVELOPES) logerror("Envelope %d new count %04x\n", channel, new_count);

		set_edd(channel, new_edd);
		if (SPG_DEBUG_ENVELOPES) logerror("Setting channel %d edd to %04x, register is %04x\n", channel, new_edd, m_audio_regs[(channel << 4) | AUDIO_ENVELOPE_DATA]);
	}
	else
	{
		new_count--;
		set_envelope_count(channel, new_count);
	}
	if (SPG_DEBUG_ENVELOPES) logerror("envelope %d post-tick, count is now %04x, register is %04x\n", channel, new_count, m_audio_regs[(channel << 4) | AUDIO_ENVELOPE_DATA]);
}