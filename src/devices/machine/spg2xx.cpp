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

#define LOG_IO_READS		(1U << 0)
#define LOG_IO_WRITES		(1U << 1)
#define LOG_IRQS			(1U << 2)
#define LOG_VLINES			(1U << 3)
#define LOG_GPIO			(1U << 4)
#define LOG_UART			(1U << 5)
#define LOG_I2C				(1U << 6)
#define LOG_DMA				(1U << 7)
#define LOG_WATCHDOG		(1U << 8)
#define LOG_SPU_READS		(1U << 9)
#define LOG_SPU_WRITES		(1U << 10)
#define LOG_CHANNEL_READS	(1U << 11)
#define LOG_CHANNEL_WRITES	(1U << 12)
#define LOG_ENVELOPES		(1U << 13)
#define LOG_SAMPLES			(1U << 14)
#define LOG_RAMPDOWN		(1U << 15)
#define LOG_BEAT			(1U << 16)
#define LOG_PPU_READS		(1U << 17)
#define LOG_PPU_WRITES		(1U << 18)
#define LOG_IO				(LOG_IO_READS | LOG_IO_WRITES | LOG_IRQS | LOG_GPIO | LOG_UART | LOG_I2C | LOG_DMA)
#define LOG_CHANNELS		(LOG_CHANNEL_READS | LOG_CHANNEL_WRITES)
#define LOG_SPU				(LOG_SPU_READS | LOG_SPU_WRITES | LOG_CHANNEL_READS | LOG_CHANNEL_WRITES | LOG_ENVELOPES | LOG_SAMPLES | LOG_RAMPDOWN | LOG_BEAT)
#define LOG_PPU				(LOG_PPU_READS | LOG_PPU_WRITES)
#define LOG_ALL				(LOG_IO | LOG_SPU | LOG_PPU | LOG_VLINES)

#define VERBOSE				(0)
#include "logmacro.h"

#define SPG_DEBUG_VIDEO		(0)
#define SPG_DEBUG_AUDIO		(0)

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
	m_audio_debug_buffer = std::make_unique<uint8_t[]>(1024*768);
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

	m_tmb1 = timer_alloc(TIMER_TMB1);
	m_tmb2 = timer_alloc(TIMER_TMB2);
	m_tmb1->adjust(attotime::never);
	m_tmb2->adjust(attotime::never);

	m_screenpos_timer = timer_alloc(TIMER_SCREENPOS);
	m_screenpos_timer->adjust(attotime::never);

	m_audio_beat = timer_alloc(TIMER_BEAT);
	m_audio_beat->adjust(attotime::never);

	m_stream = stream_alloc(0, 2, 44100);

	m_channel_debug = -1;
}

void spg2xx_device::device_reset()
{
	memset(m_audio_regs, 0, 0x800 * sizeof(uint16_t));
	memset(m_sample_shift, 0, 16);
	memset(m_sample_count, 0, sizeof(uint32_t) * 16);
	memset(m_sample_addr, 0, sizeof(uint32_t) * 16);
	memset(m_channel_rate, 0, sizeof(double) * 16);
	memset(m_channel_rate_accum, 0, sizeof(double) * 16);
	memset(m_rampdown_frame, 0, sizeof(uint32_t) * 16);
	memset(m_envclk_frame, 4, sizeof(uint32_t) * 16);
	memset(m_envelope_addr, 0, sizeof(uint32_t) * 16);

	memset(m_video_regs, 0, 0x100 * sizeof(uint16_t));
	memset(m_io_regs, 0, 0x200 * sizeof(uint16_t));

	m_io_regs[0x23] = 0x0028;

	m_video_regs[0x36] = 0xffff;
	m_video_regs[0x37] = 0xffff;
	m_video_regs[0x3c] = 0x0020;

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
inline uint8_t spg2xx_device::mix_channel(uint8_t bottom, uint8_t top)
{
	uint8_t alpha = (m_video_regs[0x2a] & 3) << 6;
	return ((256 - alpha) * bottom + alpha * top) >> 8;
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

void spg2xx_device::blit(const rectangle &cliprect, uint32_t xoff, uint32_t yoff, uint32_t attr, uint32_t ctrl, uint32_t bitmap_addr, uint16_t tile)
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

void spg2xx_device::blit_page(const rectangle &cliprect, int depth, uint32_t bitmap_addr, uint16_t *regs)
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

			blit(cliprect, xx, yy, tileattr, tilectrl, bitmap_addr, tile);
		}
	}
}

void spg2xx_device::blit_sprite(const rectangle &cliprect, int depth, uint32_t base_addr)
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
	blit(cliprect, x, y, attr, 0, bitmap_addr, tile);
	m_debug_blit = false;
#else
	blit(cliprect, x, y, attr, 0, bitmap_addr, tile);
#endif
}

void spg2xx_device::blit_sprites(const rectangle &cliprect, int depth)
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
			blit_sprite(cliprect, depth, 0x2c00 + 4 * n);
		}
#if SPG_DEBUG_VIDEO
	}
	else
	{
		blit_sprite(cliprect, depth, 0x2c00 + 4 * m_sprite_index_to_debug);
	}
#endif
}

void spg2xx_device::apply_saturation(const rectangle &cliprect)
{
	static const float s_u8_to_float = 1.0f / 255.0f;
	static const float s_gray_r = 0.299f;
	static const float s_gray_g = 0.587f;
	static const float s_gray_b = 0.114f;
	const float sat_adjust = (0xff - (m_video_regs[0x3c] & 0x00ff)) / (float)(0xff - 0x20);
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		rgbtriad_t *src = &m_screenbuf[cliprect.min_x + 320 * y];
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			const float src_r = src->r * s_u8_to_float;
			const float src_g = src->g * s_u8_to_float;
			const float src_b = src->b * s_u8_to_float;
			const float luma = src_r * s_gray_r + src_g * s_gray_g + src_b * s_gray_b;
			const float adjusted_r = luma + (src_r - luma) * sat_adjust;
			const float adjusted_g = luma + (src_g - luma) * sat_adjust;
			const float adjusted_b = luma + (src_b - luma) * sat_adjust;
			const int integer_r = (int)floor(adjusted_r * 255.0f);
			const int integer_g = (int)floor(adjusted_g * 255.0f);
			const int integer_b = (int)floor(adjusted_b * 255.0f);
			src->r = integer_r > 255 ? 255 : (integer_r < 0 ? 0 : (uint8_t)integer_r);
			src->g = integer_g > 255 ? 255 : (integer_g < 0 ? 0 : (uint8_t)integer_g);
			src->b = integer_b > 255 ? 255 : (integer_b < 0 ? 0 : (uint8_t)integer_b);
			src++;
		}
	}
}

void spg2xx_device::apply_fade(const rectangle &cliprect)
{
	const uint16_t fade_offset = m_video_regs[0x30] << 1;
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		rgbtriad_t *src = &m_screenbuf[cliprect.min_x + 320 * y];
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			const uint16_t r = (uint16_t)src->r - fade_offset;
			const uint16_t g = (uint16_t)src->g - fade_offset;
			const uint16_t b = (uint16_t)src->b - fade_offset;
			src->r = (r > src->r ? 0 : r);
			src->g = (g > src->g ? 0 : g);
			src->b = (b > src->b ? 0 : b);
			src++;
		}
	}
}

#if SPG2XX_VISUAL_AUDIO_DEBUG
uint32_t spg2xx_device::debug_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	for (int y = 0; y < 768; y++)
	{
		for (int x = 0; x < 1024; x++)
		{
			bitmap.pix32(y, x) = s_visual_debug_palette[m_audio_debug_buffer[y*1024+x]];
		}
	}
	return 0;
}

void spg2xx_device::advance_debug_pos()
{
	m_audio_debug_x++;
	if (m_audio_debug_x == 1024)
	{
		m_audio_debug_x = 0;
	}
}
#endif

uint32_t spg2xx_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	memset(&m_screenbuf[320 * cliprect.min_y], 0, 3 * 320 * ((cliprect.max_y - cliprect.min_y) + 1));

	const uint32_t page1_addr = 0x40 * m_video_regs[0x20];
	const uint32_t page2_addr = 0x40 * m_video_regs[0x21];
	uint16_t *page1_regs = m_video_regs + 0x10;
	uint16_t *page2_regs = m_video_regs + 0x16;

	for (int i = 0; i < 4; i++)
	{
		if (!m_hide_page0)
			blit_page(cliprect, i, page1_addr, page1_regs);
		if (!m_hide_page1)
			blit_page(cliprect, i, page2_addr, page2_regs);
		if (!m_hide_sprites)
			blit_sprites(cliprect, i);
	}

	if ((m_video_regs[0x3c] & 0x00ff) != 0x0020)
	{
		apply_saturation(cliprect);
	}

	if (m_video_regs[0x30] != 0)
	{
		apply_fade(cliprect);
	}

	bitmap.fill(0, cliprect);
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint32_t *dest = &bitmap.pix32(y, cliprect.min_x);
		rgbtriad_t *src = &m_screenbuf[cliprect.min_x + 320 * y];
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			*dest++ = (src->r << 16) | (src->g << 8) | src->b;
			src++;
		}
	}

	return 0;
}

void spg2xx_device::do_sprite_dma(uint32_t len)
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
		LOGMASKED(LOG_VLINES, "video_r: Current Line: %04x\n", m_screen->vpos());
		return m_screen->vpos();

	case 0x62: // Video IRQ Enable
		LOGMASKED(LOG_IRQS, "video_r: Video IRQ Enable: %04x\n", VIDEO_IRQ_ENABLE);
		return VIDEO_IRQ_ENABLE;

	case 0x63: // Video IRQ Status
		LOGMASKED(LOG_IRQS, "video_r: Video IRQ Status: %04x\n", VIDEO_IRQ_STATUS);
		return VIDEO_IRQ_STATUS;

	default:
		LOGMASKED(LOG_PPU_READS, "video_r: Unknown register %04x = %04x\n", 0x2800 + offset, m_video_regs[offset]);
		break;
	}
	return m_video_regs[offset];
}

WRITE16_MEMBER(spg2xx_device::video_w)
{
	switch (offset)
	{
	case 0x10: // Page 1 X scroll
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 1 X Scroll = %04x\n", data & 0x01ff);
		m_video_regs[offset] = data & 0x01ff;
		break;

	case 0x11: // Page 1 Y scroll
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 1 Y Scroll = %04x\n", data & 0x00ff);
		m_video_regs[offset] = data & 0x00ff;
		break;

	case 0x12: // Page 1 Attributes
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 1 Attributes = %04x (Depth:%d, Palette:%d, VSize:%d, HSize:%d, FlipY:%d, FlipX:%d, BPP:%d)\n", data
			, (data >> 12) & 3, (data >> 8) & 15, 8 << ((data >> 6) & 3), 8 << ((data >> 4) & 3), BIT(data, 3), BIT(data, 2), 2 * ((data & 3) + 1));
		m_video_regs[offset] = data;
		break;

	case 0x13: // Page 1 Control
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 1 Control = %04x (Blend:%d, HiColor:%d, RowScroll:%d, Enable:%d, Wallpaper:%d, RegSet:%d, Bitmap:%d)\n", data
			, BIT(data, 8), BIT(data, 7), BIT(data, 4), BIT(data, 3), BIT(data, 2), BIT(data, 1), BIT(data, 0));
		m_video_regs[offset] = data;
		break;

	case 0x14: // Page 1 Tile Address
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 1 Tile Address = %04x\n", data & 0x1fff);
		m_video_regs[offset] = data;
		break;

	case 0x15: // Page 1 Attribute Address
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 1 Attribute Address = %04x\n", data & 0x1fff);
		m_video_regs[offset] = data;
		break;

	case 0x16: // Page 2 X scroll
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 2 X Scroll = %04x\n", data & 0x01ff);
		m_video_regs[offset] = data & 0x01ff;
		break;

	case 0x17: // Page 2 Y scroll
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 2 Y Scroll: %04x = %04x\n", 0x2800 | offset, data & 0x00ff);
		m_video_regs[offset] = data & 0x00ff;
		break;

	case 0x18: // Page 2 Attributes
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 2 Attributes = %04x (Depth:%d, Palette:%d, VSize:%d, HSize:%d, FlipY:%d, FlipX:%d, BPP:%d)\n", data
			, (data >> 12) & 3, (data >> 8) & 15, 8 << ((data >> 6) & 3), 8 << ((data >> 4) & 3), BIT(data, 3), BIT(data, 2), 2 * ((data & 3) + 1));
		m_video_regs[offset] = data;
		break;

	case 0x19: // Page 2 Control
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 2 Control = %04x (Blend:%d, HiColor:%d, RowScroll:%d, Enable:%d, Wallpaper:%d, RegSet:%d, Bitmap:%d)\n", data
			, BIT(data, 8), BIT(data, 7), BIT(data, 4), BIT(data, 3), BIT(data, 2), BIT(data, 1), BIT(data, 0));
		m_video_regs[offset] = data;
		break;

	case 0x1a: // Page 2 Tile Address
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 2 Tile Address = %04x\n", data & 0x1fff);
		m_video_regs[offset] = data;
		break;

	case 0x1b: // Page 2 Attribute Address
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 2 Attribute Address = %04x\n", data & 0x1fff);
		m_video_regs[offset] = data;
		break;

	case 0x20: // Page 1 Segment Address
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 1 Segment Address = %04x\n", data);
		m_video_regs[offset] = data;
		break;

	case 0x21: // Page 2 Segment Address
		LOGMASKED(LOG_PPU_WRITES, "video_w: Page 2 Segment Address = %04x\n", data);
		m_video_regs[offset] = data;
		break;

	case 0x22: // Sprite Segment Address
		LOGMASKED(LOG_PPU_WRITES, "video_w: Sprite Segment Address = %04x\n", data);
		m_video_regs[offset] = data;
		break;

	case 0x2a: // Blend Level Control
		LOGMASKED(LOG_PPU_WRITES, "video_w: Blend Level Control = %04x\n", data & 0x0003);
		m_video_regs[offset] = data & 0x0003;
		break;

	case 0x30: // Fade Effect Control
		LOGMASKED(LOG_PPU_WRITES, "video_w: Fade Effect Control = %04x\n", data & 0x00ff);
		m_video_regs[offset] = data & 0x00ff;
		break;

	case 0x36: // IRQ pos V
	case 0x37: // IRQ pos H
		m_video_regs[offset] = data & 0x01ff;
		LOGMASKED(LOG_IRQS, "video_w: Video IRQ Position: %04x,%04x (%04x)\n", m_video_regs[0x37], m_video_regs[0x36], 0x2800 | offset);
		if (m_video_regs[0x37] < 160 && m_video_regs[0x36] < 240)
			m_screenpos_timer->adjust(m_screen->time_until_pos(m_video_regs[0x36], m_video_regs[0x37] << 1));
		else
			m_screenpos_timer->adjust(attotime::never);
		break;

	case 0x39: // Latch 1st Line Pen Pulse
		LOGMASKED(LOG_PPU_WRITES, "video_w: Latch 1st Line Pen Pulse = %04x\n", data & 0x0001);
		m_video_regs[offset] = data & 0x0001;
		break;

	case 0x3c: // TV Control 1
		LOGMASKED(LOG_PPU_WRITES, "video_w: TV Control 1 = %04x (Hue:%02x, Saturation:%02x)\n", data, data >> 8, data & 0x00ff);
		m_video_regs[offset] = data;
		break;

	case 0x3d: // TV Control 2
	{
		static const char* const s_lpf_mode[4] = { "LPF1", "LPF2", "All", "Edge" };
		LOGMASKED(LOG_PPU_WRITES, "video_w: TV Control 2 = %04x (LPFMode:%s, Enable:%d, Interlace:%d)\n", data & 0x000f
			, s_lpf_mode[(data >> 2) & 3], BIT(data, 1), BIT(data, 0));
		m_video_regs[offset] = data & 0x000f;
		break;
	}

	case 0x3e: // Light Pen Y Position
		LOGMASKED(LOG_PPU_WRITES, "video_w: Light Pen Y (read only) = %04x\n", data & 0x01ff);
		break;

	case 0x3f: // Light Pen YXPosition
		LOGMASKED(LOG_PPU_WRITES, "video_w: Light Pen X (read only) = %04x\n", data & 0x01ff);
		break;

	case 0x42: // Sprite Control
		LOGMASKED(LOG_PPU_WRITES, "video_w: Sprite Control = %04x (TopLeft:%d, Enable:%d)\n", data & 0x0003, BIT(data, 1), BIT(data, 0));
		m_video_regs[offset] = data & 0x0003;
		break;

	case 0x62: // Video IRQ Enable
	{
		LOGMASKED(LOG_IRQS, "video_w: Video IRQ Enable = %04x (DMA:%d, Timing:%d, Blanking:%d)\n", data & 0x0007, BIT(data, 2), BIT(data, 1), BIT(data, 0));
		const uint16_t old = VIDEO_IRQ_ENABLE & VIDEO_IRQ_STATUS;
		VIDEO_IRQ_ENABLE = data & 0x0007;
		const uint16_t changed = old ^ (VIDEO_IRQ_ENABLE & VIDEO_IRQ_STATUS);
		if (changed)
			check_video_irq();
		break;
	}

	case 0x63: // Video IRQ Acknowledge
	{
		LOGMASKED(LOG_IRQS, "video_w: Video IRQ Acknowledge = %04x\n", data);
		const uint16_t old = VIDEO_IRQ_ENABLE & VIDEO_IRQ_STATUS;
		VIDEO_IRQ_STATUS &= ~data;
		const uint16_t changed = old ^ (VIDEO_IRQ_ENABLE & VIDEO_IRQ_STATUS);
		if (changed)
			check_video_irq();
		break;
	}

	case 0x70: // Sprite DMA Source
		LOGMASKED(LOG_DMA, "video_w: Sprite DMA Source = %04x\n", data & 0x3fff);
		m_video_regs[offset] = data & 0x3fff;
		break;

	case 0x71: // Sprite DMA Dest
		LOGMASKED(LOG_DMA, "video_w: Sprite DMA Dest = %04x\n", data & 0x03ff);
		m_video_regs[offset] = data & 0x03ff;
		break;

	case 0x72: // Sprite DMA Length
		LOGMASKED(LOG_DMA, "video_w: Sprite DMA Length = %04x\n", data & 0x03ff);
		do_sprite_dma(data & 0x3ff);
		break;

	default:
		LOGMASKED(LOG_PPU_WRITES, "video_w: Unknown register %04x = %04x\n", 0x2800 + offset, data);
		m_video_regs[offset] = data;
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
	if (machine().input().code_pressed_once(KEYCODE_1))
	{
		m_channel_debug--;
		if (m_channel_debug < -1)
			m_channel_debug = 15;
	}
	if (machine().input().code_pressed_once(KEYCODE_2))
	{
		m_channel_debug++;
		if (m_channel_debug == 16)
			m_channel_debug = -1;
	}
#endif

	const uint16_t old = VIDEO_IRQ_ENABLE & VIDEO_IRQ_STATUS;
	VIDEO_IRQ_STATUS |= 1;
	LOGMASKED(LOG_IRQS, "Setting video IRQ status to %04x\n", VIDEO_IRQ_STATUS);
	const uint16_t changed = old ^ (VIDEO_IRQ_ENABLE & VIDEO_IRQ_STATUS);
	if (changed)
		check_video_irq();
}

void spg2xx_device::check_video_irq()
{
	m_cpu->set_input_line(UNSP_IRQ0_LINE, (VIDEO_IRQ_STATUS & VIDEO_IRQ_ENABLE) ? ASSERT_LINE : CLEAR_LINE);
}


/*************************
*    Machine Hardware    *
*************************/

void spg2xx_device::uart_rx(uint8_t data)
{
	if (m_uart_rx_index < 8)
	{
		m_uart_rx_fifo[m_uart_rx_index] = data;
		m_uart_rx_index++;
		if (m_uart_rx_index > (m_io_regs[0x37] & 7))
		{
			const uint16_t old = IO_IRQ_STATUS;
			IO_IRQ_STATUS |= 0x0100;
			const uint16_t changed = old ^ IO_IRQ_STATUS;
			if (changed & IO_IRQ_ENABLE)
				check_irqs(0x0100);
		}
	}
	else
	{
		m_io_regs[0x37] |= 0x4000;
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
		LOGMASKED(LOG_GPIO, "io_r: %s %c = %04x\n", gpioregs[(offset - 1) % 5], gpioports[(offset - 1) / 5], m_io_regs[offset]);
		val = m_io_regs[offset];
		break;

	case 0x02: case 0x03: case 0x04: case 0x05:
	case 0x07: case 0x08: case 0x09: case 0x0a:
	case 0x0c: case 0x0d: case 0x0e: case 0x0f: // Other GPIO regs
		LOGMASKED(LOG_GPIO, "io_r: %s %c = %04x\n", gpioregs[(offset - 1) % 5], gpioports[(offset - 1) / 5], m_io_regs[offset]);
		break;

	case 0x10: // Timebase Control
		LOGMASKED(LOG_IO_READS, "io_r: Timebase Control = %04x\n", val);
		break;

	case 0x1c: // Video line counter
		val = m_screen->vpos();
		LOGMASKED(LOG_VLINES, "io_r: Video Line = %04x\n", val);
		break;

	case 0x20: // System Control
		LOGMASKED(LOG_IO_READS, "io_r: System Control = %04x\n", val);
		break;

	case 0x21: // IRQ Control
		LOGMASKED(LOG_IRQS, "io_r: I/O IRQ Control = %04x\n", val);
		break;

	case 0x22: // IRQ Status
		LOGMASKED(LOG_IRQS, "io_r: I/O IRQ Status = %04x\n", val);
		break;

	case 0x23: // External Memory Control
		LOGMASKED(LOG_IO_READS, "io_r: Ext. Memory Control = %04x\n", val);
		break;

	case 0x25: // ADC Control
		LOGMASKED(LOG_IO_READS, "io_r: ADC Control = %04x\n", val);
		break;

	case 0x29: // Wakeup Source
		LOGMASKED(LOG_IO_READS, "io_r: Wakeup Source = %04x\n", val);
		break;

	case 0x2b:
		LOGMASKED(LOG_IO_READS, "io_r: NTSC/PAL = %04x\n", m_pal_flag);
		return m_pal_flag;

	case 0x2c: case 0x2d: // PRNG 0/1
		val = machine().rand() & 0x0000ffff;
		LOGMASKED(LOG_IO_READS, "io_r: PRNG %d = %04x\n", offset - 0x2c, val);
		break;

	case 0x2e: // FIQ Source Select
		LOGMASKED(LOG_IRQS, "io_r: FIQ Source Select = %04x\n", val);
		break;

	case 0x2f: // Data Segment
		val = m_cpu->state_int(UNSP_SR) >> 10;
		LOGMASKED(LOG_IO_READS, "io_r: Data Segment = %04x\n", val);
		break;

	case 0x31: // UART Status
		val = 0x0002 | (m_uart_rx_index ? 1 : 0) | (m_uart_rx_index == 8 ? 0x80 : 0);
		LOGMASKED(LOG_UART, "io_r: UART Status = %04x\n", val);
		break;

	case 0x36: // UART RX Data
		if (m_uart_rx_index)
		{
			val = m_uart_rx_fifo[0];
			m_uart_rx_fifo[0] = 0;
			m_uart_rx_index--;
			for (uint8_t i = 0; i < m_uart_rx_index; i++)
			{
				m_uart_rx_fifo[i] = m_uart_rx_fifo[i + 1];
			}
		}
		else
		{
			m_io_regs[0x37] |= 0x2000;
			val = 0;
		}
		LOGMASKED(LOG_UART, "io_r: UART Rx Data = %04x\n", val);
		break;

	case 0x37: // UART Rx FIFO Control
		val &= ~0x0070;
		val |= (m_uart_rx_index > 7 ? 7 : m_uart_rx_index) << 4;
		LOGMASKED(LOG_UART, "io_r: UART Rx FIFO Control = %04x\n", val);
		break;

	case 0x59: // I2C Status
		LOGMASKED(LOG_I2C, "io_r: I2C Status = %04x\n", val);
		break;

	case 0x5e: // I2C Data In
		LOGMASKED(LOG_I2C, "io_r: I2C Data In = %04x\n", val);
		break;

	case 0x100: // DMA Source (L)
		LOGMASKED(LOG_DMA, "io_r: DMA Source (lo) = %04x\n", val);
		break;

	case 0x101: // DMA Source (H)
		LOGMASKED(LOG_DMA, "io_r: DMA Source (hi) = %04x\n", val);
		break;

	case 0x102: // DMA Length
		LOGMASKED(LOG_DMA, "io_r: DMA Length = %04x\n", 0);
		val = 0;
		break;

	case 0x103: // DMA Destination
		LOGMASKED(LOG_DMA, "io_r: DMA Dest = %04x\n", val);
		break;

	default:
		LOGMASKED(LOG_IO_READS, "io_r: Unknown register %04x\n", 0x3d00 + offset);
		break;
	}

	return val;
}

void spg2xx_device::update_porta_special_modes()
{
	static const char* const s_pa_special[4][16] =
	{
		// Input,  Special 0
		// Input,  Special 1
		// Output, Special 0
		// Output, Special 1

		{ "LP",   "ExtClk2", "ExtClk1", "-",   "SDA", "SlvRDY", "-",     "-",       "SPICLK", "-",   "RxD", "SPISSB", "-",     "-",     "-",     "-"     },
		{ "-",    "-",       "-",       "SCK", "-",   "SWS",    "-",     "-",       "-",      "-",   "-",   "-",      "IRQ2B", "-",     "-",     "IRQ1B" },
		{ "-",    "-",       "-",       "SCK", "SDA", "SWS",    "-",     "-",       "SPICLK", "TxD", "-",   "SPISSB", "TAPWM", "TM1",   "TBPWM", "TM2"   },
		{ "CSB3", "CSB2",    "CSB1",    "SCK", "SDA", "VSYNC",  "HSYNC", "SYSCLK3", "SPICLK", "TxD", "SWS", "SPISSB", "-",     "VSYNC", "HSYNC", "CSYNC" },
	};
	for (int bit = 15; bit >= 0; bit--)
	{
		if (!BIT(m_io_regs[0x05], bit))
			continue;
		uint8_t type = (BIT(m_io_regs[0x03], bit) << 1) | BIT(m_io_regs[0x00], 0);
		LOGMASKED(LOG_GPIO, "      Bit %2d: %s\n", bit, s_pa_special[type][bit]);
	}
}

void spg2xx_device::update_portb_special_modes()
{
	static const char* const s_pb_special[4][8] =
	{
		// Input,  Special 0
		// Input,  Special 1
		// Output, Special 0
		// Output, Special 1

		{ "-",    "-",      "-",     "-",     "-",   "-",   "SDA", "SlvRDY"  },
		{ "-",    "-",      "-",     "-",     "-",   "-",   "SDA", "SlvRDY"  },
		{ "VSYNC", "HSYNC", "CSYNC", "-",     "-",   "SCK", "SDA", "SWS"     },
		{ "CSB3",  "CSB2",  "CSB1",  "TBPWM", "TM2", "-",   "-",   "SYSCLK2" },
	};
	for (int bit = 7; bit >= 0; bit--)
	{
		if (!BIT(m_io_regs[0x0a], bit))
			continue;
		uint8_t type = (BIT(m_io_regs[0x08], bit) << 1) | BIT(m_io_regs[0x00], 1);
		LOGMASKED(LOG_GPIO, "      Bit %2d: %s\n", bit, s_pb_special[type][bit]);
	}
}

WRITE16_MEMBER(spg2xx_device::io_w)
{
	static const char *const gpioregs[] = { "GPIO Data Port", "GPIO Buffer Port", "GPIO Direction Port", "GPIO Attribute Port", "GPIO IRQ/Latch Port" };
	static const char gpioports[3] = { 'A', 'B', 'C' };

	switch (offset)
	{
	case 0x00: // GPIO special function select
	{
		LOGMASKED(LOG_GPIO, "io_w: GPIO Configuration = %04x (IOBWake:%d, IOAWake:%d, IOBSpecSel:%d, IOASpecSel:%d)\n", data
			, BIT(data, 4), BIT(data, 3), BIT(data, 1), BIT(data, 0));
		const uint16_t old = m_io_regs[offset];
		m_io_regs[offset] = data;
		const uint16_t changed = old ^ data;
		if (BIT(changed, 0))
			update_porta_special_modes();
		if (BIT(changed, 1))
			update_portb_special_modes();
		break;
	}

	case 0x01: case 0x06: case 0x0b: // GPIO data, port A/B/C
		offset++;
		// Intentional fallthrough - we redirect data register writes to the buffer register.

	case 0x02: case 0x04: // Port A
	case 0x07: case 0x09: // Port B
	case 0x0c: case 0x0d: case 0x0e: case 0x0f: // Port C
		LOGMASKED(LOG_GPIO, "io_w: %s %c = %04x\n", gpioregs[(offset - 1) % 5], gpioports[(offset - 1) / 5], data);
		m_io_regs[offset] = data;
		do_gpio(offset);
		break;

	case 0x03: // Port A Direction
		LOGMASKED(LOG_GPIO, "io_w: GPIO Direction Port A = %04x\n", data);
		m_io_regs[offset] = data;
		update_porta_special_modes();
		do_gpio(offset);
		break;

	case 0x08: // Port B Direction
		LOGMASKED(LOG_GPIO, "io_w: GPIO Direction Port B = %04x\n", data);
		m_io_regs[offset] = data;
		update_portb_special_modes();
		do_gpio(offset);
		break;

	case 0x05: // Port A Special
		LOGMASKED(LOG_GPIO, "io_w: Port A Special Function Select: %04x\n", data);
		m_io_regs[offset] = data;
		update_porta_special_modes();
		break;

	case 0x0a: // Port B Special
		LOGMASKED(LOG_GPIO, "io_w: Port B Special Function Select: %04x\n", data);
		m_io_regs[offset] = data;
		update_portb_special_modes();
		break;

	case 0x10: // Timebase Control
	{
		static const char* const s_tmb1_sel[2][4] =
		{
			{ "8Hz", "16Hz", "32Hz", "64Hz" },
			{ "12kHz", "24kHz", "40kHz", "40kHz" }
		};
		static const char* const s_tmb2_sel[2][4] =
		{
			{ "128Hz", "256Hz", "512Hz", "1024Hz" },
			{ "105kHz", "210kHz", "420kHz", "840kHz" }
		};
		static const uint32_t s_tmb1_freq[2][4] =
		{
			{ 8, 16, 32, 64 },
			{ 12000, 24000, 40000, 40000 }
		};
		static const uint32_t s_tmb2_freq[2][4] =
		{
			{ 128, 256, 512, 1024 },
			{ 105000, 210000, 420000, 840000 }
		};
		LOGMASKED(LOG_IO_WRITES, "io_w: Timebase Control = %04x (Source:%s, TMB2:%s, TMB1:%s)\n", data,
			BIT(data, 4) ? "27MHz" : "32768Hz", s_tmb2_sel[BIT(data, 4)][(data >> 2) & 3], s_tmb1_sel[BIT(data, 4)][data & 3]);
		const uint16_t old = m_io_regs[offset];
		m_io_regs[offset] = data;
		const uint16_t changed = old ^ m_io_regs[offset];
		if (changed & 0x001f)
		{
			const uint8_t hifreq = BIT(data, 4);
			if (changed & 0x0013)
			{
				const uint32_t freq = s_tmb1_freq[hifreq][data & 3];
				m_tmb1->adjust(attotime::from_hz(freq), 0, attotime::from_hz(freq));
			}
			if (changed & 0x001c)
			{
				const uint32_t freq = s_tmb2_freq[hifreq][(data >> 2) & 3];
				m_tmb2->adjust(attotime::from_hz(freq), 0, attotime::from_hz(freq));
			}
		}
		break;
	}

	case 0x11: // Timebase Clear
		LOGMASKED(LOG_IO_WRITES, "io_w: Timebase Clear = %04x\n", data);
		break;

	case 0x20: // System Control
	{
		static const char* const s_sysclk[4] = { "13.5MHz", "27MHz", "27MHz NoICE", "54MHz" };
		static const char* const s_lvd_voltage[4] = { "2.7V", "2.9V", "3.1V", "3.3V" };
		static const char* const s_weak_strong[2] = { "Weak", "Strong" };
		LOGMASKED(LOG_IO_WRITES, "io_w: System Control = %04x (Watchdog:%d, Sleep:%d, SysClk:%s, SysClkInv:%d, LVROutEn:%d, LVREn:%d\n"
			, data, BIT(data, 15), BIT(data, 14), s_sysclk[(data >> 12) & 3], BIT(data, 11), BIT(data, 9), BIT(data, 8));
		LOGMASKED(LOG_IO_WRITES, "      LVDEn:%d, LVDVoltSel:%s, 32kHzDisable:%d, StrWkMode:%s, VDACDisable:%d, ADACDisable:%d, ADACOutDisable:%d)\n"
			, BIT(data, 7), s_lvd_voltage[(data >> 5) & 3], BIT(data, 4), s_weak_strong[BIT(data, 3)], BIT(data, 2), BIT(data, 1), BIT(data, 0));
		m_io_regs[offset] = data;
		break;
	}

	case 0x21: // IRQ Enable
	{
		LOGMASKED(LOG_IRQS, "io_w: IRQ Enable = %04x\n", data);
		const uint16_t old = IO_IRQ_ENABLE & IO_IRQ_STATUS;
		m_io_regs[offset] = data;
		const uint16_t changed = old ^ (IO_IRQ_ENABLE & IO_IRQ_STATUS);
		if (changed)
			check_irqs(changed);
		break;
	}

	case 0x22: // IRQ Acknowledge
	{
		LOGMASKED(LOG_IRQS, "io_w: IRQ Acknowledge = %04x\n", data);
		const uint16_t old = IO_IRQ_STATUS;
		IO_IRQ_STATUS &= ~data;
		const uint16_t changed = old ^ (IO_IRQ_ENABLE & IO_IRQ_STATUS);
		if (changed)
			check_irqs(changed);
		break;
	}

	case 0x23: // External Memory Control
	{
		static const char* const s_bus_arb[8] =
		{
			"Forbidden", "Forbidden", "Forbidden", "Forbidden", "Forbidden", "1:SPU/2:PPU/3:CPU", "Forbidden", "1:PPU/2:SPU/3:CPU"
		};
		static const char* const s_addr_decode[4] =
		{
			"ROMCSB: 4000-3fffff, CSB1: ---,           CSB2: ---,           CSB3: ---",
			"ROMCSB: 4000-1fffff, CSB1: 200000-3fffff, CSB2: ---,           CSB3: ---",
			"ROMCSB: 4000-0fffff, CSB1: 100000-1fffff, CSB2: 200000-2fffff, CSB3: 300000-3fffff",
			"ROMCSB: 4000-0fffff, CSB1: 100000-1fffff, CSB2: 200000-2fffff, CSB3: 300000-3fffff"
		};
		static const char* const s_ram_decode[16] =
		{
			"None", "None", "None", "None", "None", "None", "None", "None",
			"4KW,   3ff000-3fffff\n",
			"8KW,   3fe000-3fffff\n",
			"16KW,  3fc000-3fffff\n",
			"32KW,  3f8000-3fffff\n",
			"64KW,  3f0000-3fffff\n",
			"128KW, 3e0000-3fffff\n",
			"256KW, 3c0000-3fffff\n",
			"512KW, 380000-3fffff\n"
		};
		LOGMASKED(LOG_IO_WRITES, "io_w: Ext. Memory Control (not yet implemented) = %04x:\n", data);
		LOGMASKED(LOG_IO_WRITES, "      WaitStates:%d, BusArbPrio:%s\n", (data >> 1) & 3, s_bus_arb[(data >> 3) & 7]);
		LOGMASKED(LOG_IO_WRITES, "      ROMAddrDecode:%s\n", s_addr_decode[(data >> 6) & 3]);
		LOGMASKED(LOG_IO_WRITES, "      RAMAddrDecode:%s\n", s_ram_decode[(data >> 8) & 15]);
		m_io_regs[offset] = data;
		break;
	}

	case 0x24: // Watchdog
		LOGMASKED(LOG_WATCHDOG, "io_w: Watchdog Pet = %04x\n", data);
		break;

	case 0x25: // ADC Control
		LOGMASKED(LOG_IO_WRITES, "io_w: ADC Control = %04x\n", data);
		m_io_regs[offset] = data;
		break;

	case 0x28: // Sleep Mode
		LOGMASKED(LOG_IO_WRITES, "io_w: Sleep Mode (%s enter value) = %04x\n", data == 0xaa55 ? "valid" : "invalid", data);
		m_io_regs[offset] = data;
		break;

	case 0x29: // Wakeup Source
	{
		m_io_regs[offset] = data;
		static const char* const s_sources[8] =
		{
			"TMB1", "TMB2", "2Hz", "4Hz", "1024Hz", "2048Hz", "4096Hz", "Key"
		};

		LOGMASKED(LOG_IO_WRITES, "io_w: Wakeup Source = %04x:\n", data);
		bool comma = false;
		char buf[1024];
		int char_idx = 0;
		for (int i = 7; i >= 0; i--)
		{
			if (BIT(data, i))
			{
				char_idx += sprintf(&buf[char_idx], "%s%s", comma ? ", " : "", s_sources[i]);
				comma = true;
			}
		}
		buf[char_idx] = 0;
		LOGMASKED(LOG_IO_WRITES, "      %s\n", buf);
		break;
	}

	case 0x2e: // FIQ Source Select
	{
		static const char* const s_fiq_select[8] =
		{
			"PPU", "SPU Channel", "Timer A", "Timer B", "UART/SPI", "External", "Reserved", "None"
		};
		LOGMASKED(LOG_IRQS, "io_w: FIQ Source Select (not yet implemented) = %04x, %s\n", data, s_fiq_select[data & 7]);
		m_io_regs[offset] = data;
		break;
	}

	case 0x2f: // Data Segment
	{
		uint16_t ds = m_cpu->state_int(UNSP_SR);
		m_cpu->set_state_int(UNSP_SR, (ds & 0x03ff) | ((data & 0x3f) << 10));
		LOGMASKED(LOG_IO_WRITES, "io_w: Data Segment = %04x\n", data);
		break;
	}

	case 0x30: // UART Control
	{
		static const char* const s_9th_bit[4] = { "0", "1", "Odd", "Even" };
		LOGMASKED(LOG_UART, "io_w: UART Control = %04x (TxEn:%d, RxEn:%d, Bits:%d, MultiProc:%d, 9thBit:%s, TxIntEn:%d, RxIntEn:%d\n", data
			, BIT(data, 7), BIT(data, 6), BIT(data, 5) ? 9 : 8, BIT(data, 4), s_9th_bit[(data >> 2) & 3], BIT(data, 1), BIT(data, 0));
		m_io_regs[offset] = data;
		if (!BIT(data, 6))
		{
			m_uart_rx_index = 0;
			memset(m_uart_rx_fifo, 0, 8);
		}
		break;
	}

	case 0x31: // UART Status
		LOGMASKED(LOG_UART, "io_w: UART Status (read only) = %04x\n", data);
		m_io_regs[offset] = data;
		break;

	case 0x33: // UART Baud Rate
		LOGMASKED(LOG_UART, "io_w: UART Baud Rate = %d\n", 27000000 / (0x10000 - data));
		m_io_regs[offset] = data;
		break;

	case 0x35: // UART TX Data
		LOGMASKED(LOG_UART, "io_w: UART Tx Data = %02x\n", data & 0x00ff);
		m_io_regs[offset] = data;
		break;

	case 0x36: // UART RX Data
		LOGMASKED(LOG_UART, "io_w: UART Rx Data (read-only) = %04x\n", data);
		break;

	case 0x37: // UART Rx FIFO Control
		LOGMASKED(LOG_UART, "io_w: UART Rx FIFO Control = %04x (Reset:%d, Overrun:%d, Underrun:%d, Count:%d, Threshold:%d)\n", data
			, BIT(data, 15), BIT(data, 14), BIT(data, 13), (data >> 4) & 7, data & 7);
		if (data & 0x8000)
		{
			m_uart_rx_index = 0;
			memset(m_uart_rx_fifo, 0, 8);
		}
		m_io_regs[offset] &= ~data & 0x6000;
		m_io_regs[offset] &= ~0x0007;
		m_io_regs[offset] |= data & 0x0007;
		break;

	case 0x58: // I2C Command
		LOGMASKED(LOG_I2C, "io_w: I2C Command = %04x\n", data);
		m_io_regs[offset] = data;
		do_i2c();
		break;

	case 0x59: // I2C Status / Acknowledge
		LOGMASKED(LOG_I2C, "io_w: I2C Acknowledge = %04x\n", data);
		m_io_regs[offset] &= ~data;
		break;

	case 0x5a: // I2C Access Mode
		LOGMASKED(LOG_I2C, "io_w: I2C Access Mode = %04x\n", data);
		m_io_regs[offset] = data;
		break;

	case 0x5b: // I2C Device Address
		LOGMASKED(LOG_I2C, "io_w: I2C Device Address = %04x\n", data);
		m_io_regs[offset] = data;
		break;

	case 0x5c: // I2C Sub-Address
		LOGMASKED(LOG_I2C, "io_w: I2C Sub-Address = %04x\n", data);
		m_io_regs[offset] = data;
		break;

	case 0x5d: // I2C Data Out
		LOGMASKED(LOG_I2C, "io_w: I2C Data Out = %04x\n", data);
		m_io_regs[offset] = data;
		break;

	case 0x5e: // I2C Data In
		LOGMASKED(LOG_I2C, "io_w: I2C Data In = %04x\n", data);
		m_io_regs[offset] = data;
		break;

	case 0x5f: // I2C Controller Mode
		LOGMASKED(LOG_I2C, "io_w: I2C Controller Mode = %04x\n", data);
		m_io_regs[offset] = data;
		break;

	case 0x100: // DMA Source (lo)
		LOGMASKED(LOG_DMA, "io_w: DMA Source (lo) = %04x\n", data);
		m_io_regs[offset] = data;
		break;

	case 0x101: // DMA Source (hi)
		LOGMASKED(LOG_DMA, "io_w: DMA Source (hi) = %04x\n", data);
		m_io_regs[offset] = data;
		break;

	case 0x103: // DMA Destination
		LOGMASKED(LOG_DMA, "io_w: DMA Dest = %04x\n", data);
		m_io_regs[offset] = data;
		break;

	case 0x102: // DMA Length
		LOGMASKED(LOG_DMA, "io_w: DMA Length = %04x\n", data);
		do_cpu_dma(data);
		break;

	default:
		LOGMASKED(LOG_IO_WRITES, "io_w: Unknown register %04x = %04x\n", 0x3d00 + offset, data);
		m_io_regs[offset] = data;
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
	//      m_cpu->set_input_line(UNSP_IRQ1_LINE, ASSERT_LINE);
	//  }

	if (changed & 0x0c00) // Timer A, Timer B IRQ
		m_cpu->set_input_line(UNSP_IRQ2_LINE, (IO_IRQ_ENABLE & IO_IRQ_STATUS & 0x0c00) ? ASSERT_LINE : CLEAR_LINE);

	if (changed & 0x2100) // UART, ADC IRQ
		m_cpu->set_input_line(UNSP_IRQ3_LINE, (IO_IRQ_ENABLE & IO_IRQ_STATUS & 0x2100) ? ASSERT_LINE : CLEAR_LINE);

	if (changed & (AUDIO_BIS_MASK | AUDIO_BIE_MASK)) // Beat IRQ
	{
		if ((m_audio_regs[AUDIO_BEAT_COUNT] & (AUDIO_BIS_MASK | AUDIO_BIE_MASK)) == (AUDIO_BIS_MASK | AUDIO_BIE_MASK))
		{
			LOGMASKED(LOG_BEAT, "Asserting beat IRQ\n");
			m_cpu->set_input_line(UNSP_IRQ4_LINE, ASSERT_LINE);
		}
		else
		{
			LOGMASKED(LOG_BEAT, "Clearing beat IRQ\n");
			m_cpu->set_input_line(UNSP_IRQ4_LINE, CLEAR_LINE);
		}
	}

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
			m_porta_out(0, what, push &~ special);
			what = (what & ~pull) | (m_porta_in(0, pull &~ special) & pull);
			break;
		case 1:
			m_portb_out(0, what, push &~ special);
			what = (what & ~pull) | (m_portb_in(0, pull &~ special) & pull);
			break;
		case 2:
			m_portc_out(0, what, push &~ special);
			what = (what & ~pull) | (m_portc_in(0, pull &~ special) & pull);
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
		mem.write_word((dst + j) & 0x3fff, mem.read_word(src + j));
	}

	src += len;
	m_io_regs[0x100] = (uint16_t)src;
	m_io_regs[0x101] = (src >> 16) & 0x3f;
	m_io_regs[0x102] = 0;
	m_io_regs[0x103] = (dst + len) & 0x3fff;
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
			LOGMASKED(LOG_SPU_READS, "audio_r: Channel Enable: %04x\n", data);
			break;

		case AUDIO_MAIN_VOLUME:
			LOGMASKED(LOG_SPU_READS, "audio_r: Main Volume: %04x\n", data);
			break;

		case AUDIO_CHANNEL_FIQ_ENABLE:
			LOGMASKED(LOG_SPU_READS | LOG_IRQS, "audio_r: Channel FIQ Enable: %04x\n", data);
			break;

		case AUDIO_CHANNEL_FIQ_STATUS:
			LOGMASKED(LOG_SPU_READS | LOG_IRQS, "audio_r: Channel FIQ Acknowledge: %04x\n", data);
			break;

		case AUDIO_BEAT_BASE_COUNT:
			LOGMASKED(LOG_SPU_READS | LOG_BEAT, "audio_r: Beat Base Count: %04x\n", data);
			break;

		case AUDIO_BEAT_COUNT:
			LOGMASKED(LOG_SPU_READS | LOG_BEAT, "audio_r: Beat Count: %04x\n", data);
			break;

		case AUDIO_ENVCLK0:
		case AUDIO_ENVCLK1:
			LOGMASKED(LOG_SPU_READS | LOG_ENVELOPES, "audio_r: Envelope Interval %d (lo): %04x\n", offset == AUDIO_ENVCLK0 ? 0 : 1, data);
			break;

		case AUDIO_ENVCLK0_HIGH:
		case AUDIO_ENVCLK1_HIGH:
			LOGMASKED(LOG_SPU_READS | LOG_ENVELOPES, "audio_r: Envelope Interval %d (hi): %04x\n", offset == AUDIO_ENVCLK0_HIGH ? 0 : 1, data);
			break;

		case AUDIO_ENV_RAMP_DOWN:
			LOGMASKED(LOG_SPU_READS | LOG_RAMPDOWN, "audio_r: Envelope Fast Ramp Down: %04x\n", data);
			break;

		case AUDIO_CHANNEL_STOP:
			LOGMASKED(LOG_SPU_READS, "audio_r: Channel Stop Status: %04x\n", data);
			break;

		case AUDIO_CHANNEL_ZERO_CROSS:
			LOGMASKED(LOG_SPU_READS, "audio_r: Channel Zero-Cross Enable: %04x\n", data);
			break;

		case AUDIO_CONTROL:
			LOGMASKED(LOG_SPU_READS, "audio_r: Control: %04x\n", data);
			break;

		case AUDIO_COMPRESS_CTRL:
			LOGMASKED(LOG_SPU_READS, "audio_r: Compressor Control: %04x\n", data);
			break;

		case AUDIO_CHANNEL_STATUS:
			LOGMASKED(LOG_SPU_READS, "audio_r: Channel Status: %04x\n", data);
			break;

		case AUDIO_WAVE_IN_L:
			LOGMASKED(LOG_SPU_READS, "audio_r: Wave In (L) / FIFO Write Data: %04x\n", data);
			break;

		case AUDIO_WAVE_IN_R:
			LOGMASKED(LOG_SPU_READS, "audio_r: Wave In (R) / Software Channel FIFO IRQ Control: %04x\n", data);
			break;

		case AUDIO_WAVE_OUT_L:
			LOGMASKED(LOG_SPU_READS, "audio_r: Wave Out (L): %04x\n", data);
			break;

		case AUDIO_WAVE_OUT_R:
			LOGMASKED(LOG_SPU_READS, "audio_r: Wave Out (R): %04x\n", data);
			break;

		case AUDIO_CHANNEL_REPEAT:
			LOGMASKED(LOG_SPU_READS, "audio_r: Channel Repeat Enable: %04x\n", data);
			break;

		case AUDIO_CHANNEL_ENV_MODE:
			LOGMASKED(LOG_SPU_READS | LOG_ENVELOPES, "audio_r: Channel Envelope Enable: %04x\n", data);
			break;

		case AUDIO_CHANNEL_TONE_RELEASE:
			LOGMASKED(LOG_SPU_READS, "audio_r: Channel Tone Release Enable: %04x\n", data);
			break;

		case AUDIO_CHANNEL_ENV_IRQ:
			LOGMASKED(LOG_SPU_READS | LOG_IRQS, "audio_r: Channel Envelope IRQ Status: %04x\n", data);
			break;

		case AUDIO_CHANNEL_PITCH_BEND:
			LOGMASKED(LOG_SPU_READS, "audio_r: Channel Pitch Bend Enable: %04x\n", data);
			break;

		case AUDIO_SOFT_PHASE:
			LOGMASKED(LOG_SPU_READS, "audio_r: Software Channel Phase: %04x\n", data);
			break;

		case AUDIO_ATTACK_RELEASE:
			LOGMASKED(LOG_SPU_READS, "audio_r: Attack/Release Time Control: %04x\n", data);
			break;

		case AUDIO_EQ_CUTOFF10:
			LOGMASKED(LOG_SPU_READS, "audio_r: EQ Cutoff Frequency 0/1: %04x\n", data);
			break;

		case AUDIO_EQ_CUTOFF32:
			LOGMASKED(LOG_SPU_READS, "audio_r: EQ Cutoff Frequency 2/3: %04x\n", data);
			break;

		case AUDIO_EQ_GAIN10:
			LOGMASKED(LOG_SPU_READS, "audio_r: EQ Cutoff Gain 0/1: %04x\n", data);
			break;

		case AUDIO_EQ_GAIN32:
			LOGMASKED(LOG_SPU_READS, "audio_r: EQ Cutoff Gain 2/3: %04x\n", data);
			break;

		default:
			LOGMASKED(LOG_SPU_READS, "audio_r: Unknown register %04x = %04x\n", 0x3000 + offset, data);
			break;
		}
	}
	else if (channel < 16)
	{
		switch (offset & AUDIO_CHAN_OFFSET_MASK)
		{
		case AUDIO_WAVE_ADDR:
			LOGMASKED(LOG_CHANNEL_READS, "audio_r: Channel %d: Wave Addr (lo): %04x\n", channel, data);
			break;

		case AUDIO_MODE:
			LOGMASKED(LOG_CHANNEL_READS, "audio_r: Channel %d: Mode: %04x (ADPCM:%d, 16M:%d, TONE:%d, LADDR_HI:%04x, WADDR_HI:%04x)\n", channel, data,
				get_adpcm_bit(channel), get_16bit_bit(channel), get_tone_mode(channel), get_loop_addr_high(channel), get_wave_addr_high(channel));
			break;

		case AUDIO_LOOP_ADDR:
			LOGMASKED(LOG_CHANNEL_READS, "audio_r: Channel %d: Loop Addr: %04x\n", channel, data);
			break;

		case AUDIO_PAN_VOL:
			LOGMASKED(LOG_CHANNEL_READS, "audio_r: Channel %d: Pan/Vol: %04x (PAN:%02x, VOL:%02x)\n", channel, data,
				get_pan(channel), get_volume(channel));
			break;

		case AUDIO_ENVELOPE0:
			LOGMASKED(LOG_CHANNEL_READS | LOG_ENVELOPES, "audio_r: Channel %d: Envelope0: %04x (RPTPER:%d, TARGET:%02x, SIGN:%d, INC:%02x)\n", channel, data,
				get_repeat_period_bit(channel), get_envelope_target(channel), get_envelope_sign_bit(channel), get_envelope_inc(channel));
			break;

		case AUDIO_ENVELOPE_DATA:
			LOGMASKED(LOG_CHANNEL_READS | LOG_ENVELOPES, "audio_r: Channel %d: Envelope Data: %04x (CNT:%d, EDD:%02x)\n", channel, data,
				get_envelope_count(channel), get_edd(channel));
			break;

		case AUDIO_ENVELOPE1:
			LOGMASKED(LOG_CHANNEL_READS | LOG_ENVELOPES, "audio_r: Channel %d: Envelope1 Data: %04x (RPTCNT:%02x, RPT:%d, LOAD:%02x)\n", channel, data,
				get_envelope_repeat_count(channel), get_envelope_repeat_bit(channel), get_envelope_load(channel));
			break;

		case AUDIO_ENVELOPE_ADDR_HIGH:
			LOGMASKED(LOG_CHANNEL_READS | LOG_ENVELOPES, "audio_r: Channel %d: Envelope Addr (hi): %04x (IRQADDR:%03x, IRQEN:%d, EADDR_HI:%02x)\n", channel, data,
				get_audio_irq_addr(channel), get_audio_irq_enable_bit(channel), get_envelope_addr_high(channel));
			break;

		case AUDIO_ENVELOPE_ADDR:
			LOGMASKED(LOG_CHANNEL_READS | LOG_ENVELOPES, "audio_r: Channel %d: Envelope Addr (lo): %04x \n", channel, data);
			break;

		case AUDIO_WAVE_DATA_PREV:
			LOGMASKED(LOG_CHANNEL_READS, "audio_r: Channel %d: Wave Data Prev: %04x \n", channel, data);
			break;

		case AUDIO_ENVELOPE_LOOP_CTRL:
			LOGMASKED(LOG_CHANNEL_READS | LOG_ENVELOPES, "audio_r: Channel %d: Envelope Loop Ctrl: %04x (RDOFFS:%02x, EAOFFS:%03x)\n", channel, data,
				get_rampdown_offset(channel), get_envelope_eaoffset(channel));
			break;

		case AUDIO_WAVE_DATA:
			LOGMASKED(LOG_CHANNEL_READS, "audio_r: Channel %d: Wave Data: %04x\n", channel, data);
			break;

		case AUDIO_ADPCM_SEL:
			LOGMASKED(LOG_CHANNEL_READS, "audio_r: Channel %d: ADPCM Sel: %04x (ADPCM36:%d, POINTNUM:%02x\n", channel, data,
				get_adpcm36_bit(channel), get_point_number(channel));
			break;

		case AUDIO_PHASE_HIGH:
			LOGMASKED(LOG_CHANNEL_READS, "audio_r: Channel %d: Phase High: %04x\n", channel, data);
			break;

		case AUDIO_PHASE_ACCUM_HIGH:
			LOGMASKED(LOG_CHANNEL_READS, "audio_r: Channel %d: Phase Accum High: %04x\n", channel, data);
			break;

		case AUDIO_TARGET_PHASE_HIGH:
			LOGMASKED(LOG_CHANNEL_READS, "audio_r: Channel %d: Target Phase High: %04x\n", channel, data);
			break;

		case AUDIO_RAMP_DOWN_CLOCK:
			LOGMASKED(LOG_CHANNEL_READS | LOG_RAMPDOWN, "audio_r: Channel %d: Rampdown Clock: %04x\n", channel, data);
			break;

		case AUDIO_PHASE:
			LOGMASKED(LOG_CHANNEL_READS, "audio_r: Channel %d: Phase: %04x\n", channel, data);
			break;

		case AUDIO_PHASE_ACCUM:
			LOGMASKED(LOG_CHANNEL_READS, "audio_r: Channel %d: Phase Accum: %04x\n", channel, data);
			break;

		case AUDIO_TARGET_PHASE:
			LOGMASKED(LOG_CHANNEL_READS, "audio_r: Channel %d: Target Phase: %04x\n", channel, data);
			break;

		case AUDIO_PHASE_CTRL:
			LOGMASKED(LOG_CHANNEL_READS, "audio_r: Channel %d: Phase Ctrl: %04x (TIMESTEP:%d, SIGN:%d, OFFSET:%03x\n", channel, data,
				get_phase_time_step(channel), get_phase_sign_bit(channel), get_phase_offset(channel));
			break;

		default:
			LOGMASKED(LOG_CHANNEL_READS, "audio_r: Unknown register %04x\n", 0x3000 + offset);
			break;
		}
	}
	else if (channel >= 16)
	{
		LOGMASKED(LOG_CHANNEL_READS, "audio_r: Trying to read from channel %d\n", channel);
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
			LOGMASKED(LOG_SPU_WRITES, "audio_w: Channel Enable: %04x\n", data);
			const uint16_t changed = m_audio_regs[AUDIO_CHANNEL_ENABLE] ^ data;
			for (uint32_t channel_bit = 0; channel_bit < 16; channel_bit++)
			{
				const uint16_t mask = 1 << channel_bit;
				if (!(changed & mask))
					continue;

				//if (channel_bit == 10)
					//continue;
				if (data & mask)
				{
					LOGMASKED(LOG_SPU_WRITES, "Enabling channel %d\n", channel_bit);
					m_audio_regs[offset] |= mask;
					if (!(m_audio_regs[AUDIO_CHANNEL_STOP] & mask))
					{
						LOGMASKED(LOG_SPU_WRITES, "Stop not set, starting playback on channel %d, mask %04x\n", channel_bit, mask);
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
			LOGMASKED(LOG_SPU_WRITES, "audio_w: Main Volume: %04x\n", data);
			m_audio_regs[offset] = data & AUDIO_MAIN_VOLUME_MASK;
			break;

		case AUDIO_CHANNEL_FIQ_ENABLE:
			LOGMASKED(LOG_SPU_WRITES | LOG_IRQS, "audio_w: Channel FIQ Enable: %04x\n", data);
			m_audio_regs[offset] = data & AUDIO_CHANNEL_FIQ_ENABLE_MASK;
			break;

		case AUDIO_CHANNEL_FIQ_STATUS:
			LOGMASKED(LOG_SPU_WRITES | LOG_IRQS, "audio_w: Channel FIQ Acknowledge: %04x\n", data);
			m_audio_regs[offset] &= ~(data & AUDIO_CHANNEL_FIQ_STATUS_MASK);
			break;

		case AUDIO_BEAT_BASE_COUNT:
			LOGMASKED(LOG_SPU_WRITES | LOG_BEAT, "audio_w: Beat Base Count: %04x\n", data);
			m_audio_regs[offset] = data & AUDIO_BEAT_BASE_COUNT_MASK;
			m_audio_curr_beat_base_count = m_audio_regs[offset];
			break;

		case AUDIO_BEAT_COUNT:
		{
			LOGMASKED(LOG_SPU_WRITES | LOG_BEAT, "audio_w: Beat Count: %04x\n", data);
			const uint16_t old = m_audio_regs[offset];
			m_audio_regs[offset] &= ~(data & AUDIO_BIS_MASK);
			m_audio_regs[offset] &= AUDIO_BIS_MASK;
			m_audio_regs[offset] |= data & ~AUDIO_BIS_MASK;
			const uint16_t changed = old ^ m_audio_regs[offset];
			if (data == 0xc000 && m_audio_regs[offset])
			{
			}
			if (changed & (AUDIO_BIS_MASK | AUDIO_BIE_MASK))
			{
				LOGMASKED(LOG_BEAT, "BIS mask changed, updating IRQ\n");
				check_irqs(changed & (AUDIO_BIS_MASK | AUDIO_BIE_MASK));
			}
			break;
		}

		case AUDIO_ENVCLK0:
		case AUDIO_ENVCLK1:
		{
			LOGMASKED(LOG_SPU_WRITES | LOG_ENVELOPES, "audio_w: Envelope Interval %d (lo): %04x\n", offset == AUDIO_ENVCLK0 ? 0 : 1, data);
			const uint16_t old = m_audio_regs[offset];
			m_audio_regs[offset] = data;
			const uint16_t changed = old ^ m_audio_regs[offset];

			if (!changed)
				break;

			const uint8_t channel_offset = offset == AUDIO_ENVCLK0 ? 0 : 8;
			for (uint8_t channel_bit = 0; channel_bit < 4; channel_bit++)
			{
				const uint8_t shift = channel_bit << 2;
				const uint16_t mask = 0x0f << shift;
				if (changed & mask)
				{
					m_envclk_frame[channel_bit + channel_offset] = get_envclk_frame_count(channel_bit + channel_offset);
				}
			}
			break;
		}

		case AUDIO_ENVCLK0_HIGH:
		case AUDIO_ENVCLK1_HIGH:
		{
			LOGMASKED(LOG_SPU_WRITES | LOG_ENVELOPES, "audio_w: Envelope Interval %d (hi): %04x\n", offset == AUDIO_ENVCLK0_HIGH ? 0 : 1, data);
			const uint16_t old = m_audio_regs[offset];
			m_audio_regs[offset] = data;
			const uint16_t changed = old ^ m_audio_regs[offset];
			if (!changed)
				break;

			const uint8_t channel_offset = offset == AUDIO_ENVCLK0_HIGH ? 0 : 8;
			for (uint8_t channel_bit = 0; channel_bit < 4; channel_bit++)
			{
				const uint8_t shift = channel_bit << 2;
				const uint16_t mask = 0x0f << shift;
				if (changed & mask)
				{
					m_envclk_frame[channel_bit + channel_offset + 4] = get_envclk_frame_count(channel_bit + channel_offset);
				}
			}
			break;
		}

		case AUDIO_ENV_RAMP_DOWN:
		{
			LOGMASKED(LOG_SPU_WRITES | LOG_RAMPDOWN, "audio_w: Envelope Fast Ramp Down: %04x\n", data);
			const uint16_t old = m_audio_regs[offset];
			m_audio_regs[offset] = data & AUDIO_ENV_RAMP_DOWN_MASK;
			const uint16_t changed = old ^ m_audio_regs[offset];
			if (!changed)
				break;

			for (uint32_t channel_bit = 0; channel_bit < 16; channel_bit++)
			{
				const uint16_t mask = 1 << channel_bit;
				if ((changed & mask) && (data & mask))
				{
					m_rampdown_frame[channel_bit] = get_rampdown_frame_count(channel_bit);
					LOGMASKED(LOG_RAMPDOWN, "Preparing to ramp down channel %d in %d ticks\n", channel_bit, m_rampdown_frame[channel_bit] / 13);
				}
			}
			break;
		}

		case AUDIO_CHANNEL_STOP:
			LOGMASKED(LOG_SPU_WRITES, "audio_w: Channel Stop Status: %04x\n", data);
			m_audio_regs[offset] &= ~(data & AUDIO_CHANNEL_STOP_MASK);
			break;

		case AUDIO_CHANNEL_ZERO_CROSS:
			LOGMASKED(LOG_SPU_WRITES, "audio_w: Channel Zero-Cross Enable: %04x\n", data);
			m_audio_regs[offset] = data & AUDIO_CHANNEL_ZERO_CROSS_MASK;
			break;

		case AUDIO_CONTROL:
			LOGMASKED(LOG_SPU_WRITES, "audio_w: Control: %04x (SOFTCH:%d, COMPEN:%d, NOHIGH:%d, NOINT:%d, EQEN:%d\n", data
				, (data & AUDIO_CONTROL_SOFTCH_MASK) ? 1 : 0
				, (data & AUDIO_CONTROL_COMPEN_MASK) ? 1 : 0
				, (data & AUDIO_CONTROL_NOHIGH_MASK) ? 1 : 0
				, (data & AUDIO_CONTROL_NOINT_MASK) ? 1 : 0
				, (data & AUDIO_CONTROL_EQEN_MASK) ? 1 : 0);
			m_audio_regs[offset] = data & AUDIO_CONTROL_MASK;
			break;

		case AUDIO_COMPRESS_CTRL:
			LOGMASKED(LOG_SPU_WRITES, "audio_w: Compressor Control: %04x\n", data);
			m_audio_regs[offset] = data;
			break;

		case AUDIO_CHANNEL_STATUS:
			LOGMASKED(LOG_SPU_WRITES, "audio_w: Channel Status (read-only): %04x\n", data);
			break;

		case AUDIO_WAVE_IN_L:
			LOGMASKED(LOG_SPU_WRITES, "audio_w: Wave In (L) / FIFO Write Data: %04x\n", data);
			m_audio_regs[offset] = data;
			break;

		case AUDIO_WAVE_IN_R:
			LOGMASKED(LOG_SPU_WRITES, "audio_w: Wave In (R) / Software Channel FIFO IRQ Control: %04x\n", data);
			m_audio_regs[offset] = data;
			break;

		case AUDIO_WAVE_OUT_L:
			LOGMASKED(LOG_SPU_WRITES, "audio_w: Wave Out (L): %04x\n", data);
			m_audio_regs[offset] = data;
			break;

		case AUDIO_WAVE_OUT_R:
			LOGMASKED(LOG_SPU_WRITES, "audio_w: Wave Out (R): %04x\n", data);
			m_audio_regs[offset] = data;
			break;

		case AUDIO_CHANNEL_REPEAT:
			LOGMASKED(LOG_SPU_WRITES, "audio_w: Channel Repeat Enable: %04x\n", data);
			m_audio_regs[offset] = data & AUDIO_CHANNEL_REPEAT_MASK;
			break;

		case AUDIO_CHANNEL_ENV_MODE:
			LOGMASKED(LOG_SPU_WRITES | LOG_ENVELOPES, "audio_w: Channel Envelope Enable: %04x\n", data);
			m_audio_regs[offset] = data & AUDIO_CHANNEL_ENV_MODE_MASK;
			break;

		case AUDIO_CHANNEL_TONE_RELEASE:
			LOGMASKED(LOG_SPU_WRITES, "audio_w: Channel Tone Release Enable: %04x\n", data);
			m_audio_regs[offset] = data & AUDIO_CHANNEL_TONE_RELEASE_MASK;
			break;

		case AUDIO_CHANNEL_ENV_IRQ:
			LOGMASKED(LOG_SPU_WRITES | LOG_ENVELOPES | LOG_IRQS, "audio_w: Channel Envelope IRQ Acknowledge: %04x\n", data);
			m_audio_regs[offset] &= ~data & AUDIO_CHANNEL_ENV_IRQ_MASK;
			break;

		case AUDIO_CHANNEL_PITCH_BEND:
			LOGMASKED(LOG_SPU_WRITES, "audio_w: Channel Pitch Bend Enable: %04x\n", data);
			m_audio_regs[offset] = data & AUDIO_CHANNEL_PITCH_BEND_MASK;
			break;

		case AUDIO_SOFT_PHASE:
			LOGMASKED(LOG_SPU_WRITES, "audio_w: Software Channel Phase: %04x\n", data);
			m_audio_regs[offset] = data;
			break;

		case AUDIO_ATTACK_RELEASE:
			LOGMASKED(LOG_SPU_WRITES, "audio_w: Attack/Release Time Control: %04x\n", data);
			m_audio_regs[offset] = data;
			break;

		case AUDIO_EQ_CUTOFF10:
			LOGMASKED(LOG_SPU_WRITES, "audio_w: EQ Cutoff Frequency 0/1: %04x\n", data);
			m_audio_regs[offset] = data & AUDIO_EQ_CUTOFF10_MASK;
			break;

		case AUDIO_EQ_CUTOFF32:
			LOGMASKED(LOG_SPU_WRITES, "audio_w: EQ Cutoff Frequency 2/3: %04x\n", data);
			m_audio_regs[offset] = data & AUDIO_EQ_CUTOFF32_MASK;
			break;

		case AUDIO_EQ_GAIN10:
			LOGMASKED(LOG_SPU_WRITES, "audio_w: EQ Cutoff Gain 0/1: %04x\n", data);
			m_audio_regs[offset] = data & AUDIO_EQ_GAIN10_MASK;
			break;

		case AUDIO_EQ_GAIN32:
			LOGMASKED(LOG_SPU_WRITES, "audio_w: EQ Cutoff Gain 2/3: %04x\n", data);
			m_audio_regs[offset] = data & AUDIO_EQ_GAIN32_MASK;
			break;

		default:
			m_audio_regs[offset] = data;
			LOGMASKED(LOG_SPU_WRITES, "audio_w: Unknown register %04x = %04x\n", 0x3000 + offset, data);
			break;
		}
	}
	else if (channel < 16)
	{
		switch (offset & AUDIO_CHAN_OFFSET_MASK)
		{
		case AUDIO_WAVE_ADDR:
			m_audio_regs[offset] = data;
			LOGMASKED(LOG_CHANNEL_WRITES, "audio_w: Channel %d: Wave Addr (lo): %04x\n", channel, data);
			break;

		case AUDIO_MODE:
			m_audio_regs[offset] = data;
			LOGMASKED(LOG_CHANNEL_WRITES, "audio_w: Channel %d: Mode: %04x (ADPCM:%d, 16M:%d, TONE:%d, LADDR_HI:%04x, WADDR_HI:%04x)\n", channel, data,
				get_adpcm_bit(channel), get_16bit_bit(channel), get_tone_mode(channel), get_loop_addr_high(channel), get_wave_addr_high(channel));
			break;

		case AUDIO_LOOP_ADDR:
			m_audio_regs[offset] = data;
			LOGMASKED(LOG_CHANNEL_WRITES, "audio_w: Channel %d: Loop Addr: %04x\n", channel, data);
			break;

		case AUDIO_PAN_VOL:
			m_audio_regs[offset] = data & AUDIO_PAN_VOL_MASK;
			LOGMASKED(LOG_CHANNEL_WRITES, "audio_w: Channel %d: Pan/Vol: %04x (PAN:%02x, VOL:%02x)\n", channel, data,
				get_pan(channel), get_volume(channel));
			break;

		case AUDIO_ENVELOPE0:
			m_audio_regs[offset] = data;
			LOGMASKED(LOG_CHANNEL_WRITES | LOG_ENVELOPES, "audio_w: Channel %d: Envelope0: %04x (RPTPER:%d, TARGET:%02x, SIGN:%d, INC:%02x)\n", channel, data,
				get_repeat_period_bit(channel), get_envelope_target(channel), get_envelope_sign_bit(channel), get_envelope_inc(channel));
			break;

		case AUDIO_ENVELOPE_DATA:
			m_audio_regs[offset] = data & AUDIO_ENVELOPE_DATA_MASK;
			LOGMASKED(LOG_CHANNEL_WRITES | LOG_ENVELOPES, "audio_w: Channel %d: Envelope Data: %04x (CNT:%d, EDD:%02x)\n", channel, data,
				get_envelope_count(channel), get_edd(channel));
			break;

		case AUDIO_ENVELOPE1:
			m_audio_regs[offset] = data;
			LOGMASKED(LOG_CHANNEL_WRITES | LOG_ENVELOPES, "audio_w: Channel %d: Envelope1 Data: %04x (RPTCNT:%02x, RPT:%d, LOAD:%02x)\n", channel, data,
				get_envelope_repeat_count(channel), get_envelope_repeat_bit(channel), get_envelope_load(channel));
			break;

		case AUDIO_ENVELOPE_ADDR_HIGH:
			m_audio_regs[offset] = data;
			LOGMASKED(LOG_CHANNEL_WRITES | LOG_ENVELOPES, "audio_w: Channel %d: Envelope Addr (hi): %04x (IRQADDR:%03x, IRQEN:%d, EADDR_HI:%02x)\n", channel, data,
				get_audio_irq_addr(channel), get_audio_irq_enable_bit(channel), get_envelope_addr_high(channel));
			break;

		case AUDIO_ENVELOPE_ADDR:
			m_audio_regs[offset] = data;
			LOGMASKED(LOG_CHANNEL_WRITES | LOG_ENVELOPES, "audio_w: Channel %d: Envelope Addr (lo): %04x\n", channel, data);
			break;

		case AUDIO_WAVE_DATA_PREV:
			m_audio_regs[offset] = data;
			LOGMASKED(LOG_CHANNEL_WRITES, "audio_w: Channel %d: Wave Data Prev: %04x \n", channel, data);
			break;

		case AUDIO_ENVELOPE_LOOP_CTRL:
			m_audio_regs[offset] = data;
			LOGMASKED(LOG_CHANNEL_WRITES | LOG_ENVELOPES, "audio_w: Channel %d: Envelope Loop Ctrl: %04x (RDOFFS:%02x, EAOFFS:%03x)\n", channel, data,
				get_rampdown_offset(channel), get_envelope_eaoffset(channel));
			break;

		case AUDIO_WAVE_DATA:
			m_audio_regs[offset] = data;
			LOGMASKED(LOG_CHANNEL_WRITES, "audio_w: Channel %d: Wave Data: %04x\n", channel, data);
			break;

		case AUDIO_ADPCM_SEL:
			m_audio_regs[offset] = data & AUDIO_ADPCM_SEL_MASK;
			LOGMASKED(LOG_CHANNEL_WRITES, "audio_w: Channel %d: ADPCM Sel: %04x (ADPCM36:%d, POINTNUM:%02x\n", channel, data,
				get_adpcm36_bit(channel), get_point_number(channel));
			break;

		case AUDIO_PHASE_HIGH:
			m_audio_regs[offset] = data & AUDIO_PHASE_HIGH_MASK;
			m_channel_rate[channel] = ((double)get_phase(channel) * 140625.0 * 2.0) / (double)(1 << 19);
			m_channel_rate_accum[channel] = 0.0;
			LOGMASKED(LOG_CHANNEL_WRITES, "audio_w: Channel %d: Phase High: %04x (rate: %f)\n", channel, data, m_channel_rate[channel]);
			break;

		case AUDIO_PHASE_ACCUM_HIGH:
			m_audio_regs[offset] = data & AUDIO_PHASE_ACCUM_HIGH_MASK;
			LOGMASKED(LOG_CHANNEL_WRITES, "audio_w: Channel %d: Phase Accum High: %04x\n", channel, data);
			break;

		case AUDIO_TARGET_PHASE_HIGH:
			m_audio_regs[offset] = data & AUDIO_TARGET_PHASE_HIGH_MASK;
			LOGMASKED(LOG_CHANNEL_WRITES, "audio_w: Channel %d: Target Phase High: %04x\n", channel, data);
			break;

		case AUDIO_RAMP_DOWN_CLOCK:
			m_audio_regs[offset] = data & AUDIO_RAMP_DOWN_CLOCK_MASK;
			LOGMASKED(LOG_CHANNEL_WRITES | LOG_RAMPDOWN, "audio_w: Channel %d: Rampdown Clock: %04x\n", channel, data);
			break;

		case AUDIO_PHASE:
			m_audio_regs[offset] = data;
			m_channel_rate[channel] = ((double)get_phase(channel) * 140625.0 * 2.0) / (double)(1 << 19);
			m_channel_rate_accum[channel] = 0.0;
			LOGMASKED(LOG_CHANNEL_WRITES, "audio_w: Channel %d: Phase: %04x (rate: %f)\n", channel, data, m_channel_rate[channel]);
			break;

		case AUDIO_PHASE_ACCUM:
			m_audio_regs[offset] = data;
			LOGMASKED(LOG_CHANNEL_WRITES, "audio_w: Channel %d: Phase Accum: %04x\n", channel, data);
			break;

		case AUDIO_TARGET_PHASE:
			m_audio_regs[offset] = data;
			LOGMASKED(LOG_CHANNEL_WRITES, "audio_w: Channel %d: Target Phase: %04x\n", channel, data);
			break;

		case AUDIO_PHASE_CTRL:
			m_audio_regs[offset] = data;
			LOGMASKED(LOG_CHANNEL_WRITES, "audio_w: Channel %d: Phase Ctrl: %04x (TIMESTEP:%d, SIGN:%d, OFFSET:%03x\n", channel, data,
				get_phase_time_step(channel), get_phase_sign_bit(channel), get_phase_offset(channel));
			break;

		default:
			m_audio_regs[offset] = data;
			LOGMASKED(LOG_CHANNEL_WRITES, "audio_w: Unknown register %04x = %04x\n", 0x3000 + offset, data);
			break;
		}
	}
	else if (channel >= 16)
	{
		LOGMASKED(LOG_CHANNEL_WRITES, "audio_w: Trying to write to channel %d: %04x = %04x\n", channel, 0x3000 + offset, data);
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
		for (int y = 0; y < 768; y++)
		{
			m_audio_debug_buffer[y*1024 + m_audio_debug_x] = 0;
		}
#endif
		for (uint32_t ch_index = 0; ch_index < 16; ch_index++)
		{
#if SPG2XX_VISUAL_AUDIO_DEBUG
			uint32_t debug_y = 0;
			if (m_channel_debug < 0)
				debug_y = ch_index * 48;
#endif
			if (!get_channel_status(ch_index))
			{
#if SPG2XX_VISUAL_AUDIO_DEBUG
				if (m_channel_debug == -1)
				{
					m_audio_debug_buffer[(debug_y + 47)*1024 + m_audio_debug_x] |= SPG_VDB_WAVE;
					int edd_y = (int)((float)get_edd(ch_index) * 0.375f);
					m_audio_debug_buffer[(debug_y + (47 - edd_y))*1024 + m_audio_debug_x] |= SPG_VDB_EDD;
					int vol_y = (int)((float)get_volume(ch_index) * 0.375f);
					m_audio_debug_buffer[(debug_y + (47 - vol_y))*1024 + m_audio_debug_x] |= SPG_VDB_VOL;
				}
				else if (ch_index == m_channel_debug)
				{
					m_audio_debug_buffer[(debug_y + 767)*1024 + m_audio_debug_x] |= SPG_VDB_WAVE;
					int edd_y = (int)((float)get_edd(ch_index) * 3.74f);
					m_audio_debug_buffer[(debug_y + (767 - edd_y))*1024 + m_audio_debug_x] |= SPG_VDB_EDD;
					int vol_y = (int)((float)get_volume(ch_index) * 3.74f);
					m_audio_debug_buffer[(debug_y + (767 - vol_y))*1024 + m_audio_debug_x] |= SPG_VDB_VOL;
				}
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
				if (m_channel_debug == -1)
				{
					int sample_y = (int)((float)m_audio_regs[(ch_index << 4) | AUDIO_WAVE_DATA] / 1365.0f);
					m_audio_debug_buffer[(debug_y + (47 - sample_y))*1024 + m_audio_debug_x] |= SPG_VDB_WAVE;
					int edd_y = (int)((float)get_edd(ch_index) * 0.375f);
					m_audio_debug_buffer[(debug_y + (47 - edd_y))*1024 + m_audio_debug_x] |= SPG_VDB_EDD;
					int vol_y = (int)((float)get_volume(ch_index) * 0.375f);
					m_audio_debug_buffer[(debug_y + (47 - vol_y))*1024 + m_audio_debug_x] |= SPG_VDB_VOL;
				}
				else if (ch_index == m_channel_debug)
				{
					int sample_y = (int)((float)m_audio_regs[(ch_index << 4) | AUDIO_WAVE_DATA] / 136.0f);
					m_audio_debug_buffer[(debug_y + (767 - sample_y))*1024 + m_audio_debug_x] |= SPG_VDB_WAVE;
					int edd_y = (int)((float)get_edd(ch_index) * 0.375f);
					m_audio_debug_buffer[(debug_y + (767 - edd_y))*1024 + m_audio_debug_x] |= SPG_VDB_EDD;
					int vol_y = (int)((float)get_volume(ch_index) * 0.375f);
					m_audio_debug_buffer[(debug_y + (767 - vol_y))*1024 + m_audio_debug_x] |= SPG_VDB_VOL;
				}
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
			left_total /= active_count;
			right_total /= active_count;
			*out_l++ = (left_total * (int16_t)m_audio_regs[AUDIO_MAIN_VOLUME]) >> 7;
			*out_r++ = (right_total * (int16_t)m_audio_regs[AUDIO_MAIN_VOLUME]) >> 7;
		}
		else
		{
			*out_l++ = 0;
			*out_r++ = 0;
		}

#if SPG2XX_VISUAL_AUDIO_DEBUG
		for (int y = 0; y < 768; y++)
		{
			m_audio_debug_buffer[y*1024 + m_audio_debug_x] = 7;
		}
#endif
	}
	//printf("\n");
}

inline void spg2xx_device::stop_channel(const uint32_t channel)
{
	// TODO: IRQs
	m_audio_regs[AUDIO_CHANNEL_ENABLE] &= ~(1 << channel);
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
		m_audio_regs[wave_data_reg] = (uint16_t)(m_adpcm[channel].clock((uint8_t)(m_audio_regs[wave_data_reg] & 0x000f)) * 7) ^ 0x8000;
		if (tone_mode != 0 && raw_sample == 0xffff)
		{
			if (tone_mode == AUDIO_TONE_MODE_HW_ONESHOT)
			{
				LOGMASKED(LOG_SAMPLES, "ADPCM stopped after %d samples\n", m_sample_count[channel]);
				m_sample_count[channel] = 0;
				stop_channel(channel);
				return false;
			}
			else
			{
				LOGMASKED(LOG_SAMPLES, "ADPCM looping after %d samples\n", m_sample_count[channel]);
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
				LOGMASKED(LOG_SAMPLES, "16-bit PCM stopped after %d samples\n", m_sample_count[channel]);
				m_sample_count[channel] = 0;
				stop_channel(channel);
				return false;
			}
			else
			{
				LOGMASKED(LOG_SAMPLES, "16-bit PCM looping after %d samples\n", m_sample_count[channel]);
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
					LOGMASKED(LOG_SAMPLES, "8-bit PCM stopped after %d samples\n", m_sample_count[channel]);
					m_sample_count[channel] = 0;
					stop_channel(channel);
					return false;
				}
				else
				{
					LOGMASKED(LOG_SAMPLES, "8-bit PCM looping after %d samples\n", m_sample_count[channel]);
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
	bool any_changed = false;
	for (uint32_t channel = 0; channel < 16; channel++)
	{
		const uint16_t mask = (1 << channel);
		if (!(m_audio_regs[AUDIO_CHANNEL_STATUS] & mask))
		{
			continue;
		}

		if (m_audio_regs[AUDIO_ENV_RAMP_DOWN] & mask)
		{
			m_rampdown_frame[channel]--;
			if (m_rampdown_frame[channel] == 0)
			{
				LOGMASKED(LOG_RAMPDOWN, "Ticking rampdown for channel %d\n", channel);
				audio_rampdown_tick(channel);
				any_changed = true;
			}
			continue;
		}

		if (!(m_audio_regs[AUDIO_CHANNEL_ENV_MODE] & mask))
		{
			m_envclk_frame[channel]--;
			if (m_envclk_frame[channel] == 0)
			{
				LOGMASKED(LOG_ENVELOPES, "Ticking envelope for channel %d\n", channel);
				any_changed = audio_envelope_tick(space, channel) || any_changed;
				m_envclk_frame[channel] = get_envclk_frame_count(channel);
			}
		}
	}

	if (any_changed)
	{
		m_stream->update();
	}
}

void spg2xx_device::audio_beat_tick()
{
	if (m_audio_curr_beat_base_count == 0)
	{
		LOGMASKED(LOG_BEAT, "Beat base count elapsed, reloading with %d\n", m_audio_regs[AUDIO_BEAT_BASE_COUNT]);
		m_audio_curr_beat_base_count = m_audio_regs[AUDIO_BEAT_BASE_COUNT];

		uint16_t beat_count = m_audio_regs[AUDIO_BEAT_COUNT] & AUDIO_BEAT_COUNT_MASK;
		if (beat_count == 0)
		{
			if (m_audio_regs[AUDIO_BEAT_COUNT] & AUDIO_BIE_MASK)
			{
				LOGMASKED(LOG_BEAT, "Beat count elapsed, setting Status bit and checking IRQs\n");
				m_audio_regs[AUDIO_BEAT_COUNT] |= AUDIO_BIS_MASK;
				check_irqs(AUDIO_BIS_MASK);
			}
			else
			{
				LOGMASKED(LOG_BEAT, "Beat count elapsed but IRQ not enabled\n");
			}
		}
		else
		{
			beat_count--;
			m_audio_regs[AUDIO_BEAT_COUNT] = (m_audio_regs[AUDIO_BEAT_COUNT] & ~AUDIO_BEAT_COUNT_MASK) | beat_count;
		}
	}
	else
	{
		m_audio_curr_beat_base_count--;
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
		LOGMASKED(LOG_RAMPDOWN, "Channel %d preparing for next rampdown step (%02x)\n", channel, new_edd);
		const uint16_t channel_mask = channel << 4;
		m_audio_regs[channel_mask | AUDIO_ENVELOPE_DATA] &= ~AUDIO_EDD_MASK;
		m_audio_regs[channel_mask | AUDIO_ENVELOPE_DATA] |= new_edd & AUDIO_EDD_MASK;
		m_rampdown_frame[channel] = get_rampdown_frame_count(channel);
	}
	else
	{
		LOGMASKED(LOG_RAMPDOWN, "Stopping channel %d due to rampdown\n", channel);
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
		return (m_audio_regs[AUDIO_ENVCLK0] >> (channel << 2)) & 0x000f;
	else if (channel < 8)
		return (m_audio_regs[AUDIO_ENVCLK0_HIGH] >> ((channel - 4) << 2)) & 0x000f;
	else if (channel < 12)
		return (m_audio_regs[AUDIO_ENVCLK1] >> ((channel - 8) << 2)) & 0x000f;
	else
		return (m_audio_regs[AUDIO_ENVCLK1_HIGH] >> ((channel - 12) << 2)) & 0x000f;
}

bool spg2xx_device::audio_envelope_tick(address_space &space, const uint32_t channel)
{
	const uint16_t channel_mask = channel << 4;
	uint16_t new_count = get_envelope_count(channel);
	const uint16_t curr_edd = get_edd(channel);
	LOGMASKED(LOG_ENVELOPES, "envelope %d tick, count is %04x, curr edd is %04x\n", channel, new_count, curr_edd);
	bool edd_changed = false;
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
				LOGMASKED(LOG_ENVELOPES, "Envelope %d new EDD-: %04x (%04x), dec %04x\n", channel, new_edd, target, inc);
				if (new_edd > curr_edd)
					new_edd = 0;
				else if (new_edd < target)
					new_edd = target;

				if (new_edd == 0)
				{
					LOGMASKED(LOG_ENVELOPES, "Envelope %d at 0, stopping channel\n", channel);
					stop_channel(channel);
					return true;
				}
			}
			else
			{
				new_edd += inc;
				LOGMASKED(LOG_ENVELOPES, "Envelope %d new EDD+: %04x\n", channel, new_edd);
				if (new_edd >= target)
					new_edd = target;
			}
		}

		if (new_edd == target)
		{
			LOGMASKED(LOG_ENVELOPES, "Envelope %d at target %04x\n", channel, target);
			new_edd = target;

			if (get_envelope_repeat_bit(channel))
			{
				const uint16_t repeat_count = get_envelope_repeat_count(channel) - 1;
				LOGMASKED(LOG_ENVELOPES, "Repeating envelope, new repeat count %d\n", repeat_count);
				if (repeat_count == 0)
				{
					m_audio_regs[channel_mask | AUDIO_ENVELOPE0] = space.read_word(m_envelope_addr[channel]);
					m_audio_regs[channel_mask | AUDIO_ENVELOPE1] = space.read_word(m_envelope_addr[channel] + 1);
					m_audio_regs[channel_mask | AUDIO_ENVELOPE_LOOP_CTRL] = space.read_word(m_envelope_addr[channel] + 2);
					m_envelope_addr[channel] = get_envelope_addr(channel) + get_envelope_eaoffset(channel);
					LOGMASKED(LOG_ENVELOPES, "Envelope data after repeat: %04x %04x %04x (%08x)\n", m_audio_regs[channel_mask | AUDIO_ENVELOPE0], m_audio_regs[channel_mask | AUDIO_ENVELOPE1], m_audio_regs[channel_mask | AUDIO_ENVELOPE_LOOP_CTRL], m_envelope_addr[channel]);
				}
				else
				{
					set_envelope_repeat_count(channel, repeat_count);
				}
			}
			else
			{
				LOGMASKED(LOG_ENVELOPES, "Fetching envelope for channel %d from %08x\n", channel, m_envelope_addr[channel]);
				m_audio_regs[channel_mask | AUDIO_ENVELOPE0] = space.read_word(m_envelope_addr[channel]);
				m_audio_regs[channel_mask | AUDIO_ENVELOPE1] = space.read_word(m_envelope_addr[channel] + 1);
				LOGMASKED(LOG_ENVELOPES, "Fetched envelopes %04x %04x\n", m_audio_regs[channel_mask | AUDIO_ENVELOPE0], m_audio_regs[channel_mask | AUDIO_ENVELOPE1]);
				m_envelope_addr[channel] += 2;
			}
			new_count = get_envelope_load(channel);
			set_envelope_count(channel, new_count);
		}
		else
		{
			LOGMASKED(LOG_ENVELOPES, "Envelope %d not yet at target %04x (%04x)\n", channel, target, new_edd);
			new_count = get_envelope_load(channel);
			set_envelope_count(channel, new_count);
		}
		LOGMASKED(LOG_ENVELOPES, "Envelope %d new count %04x\n", channel, new_count);

		set_edd(channel, new_edd);
		edd_changed = true;
		LOGMASKED(LOG_ENVELOPES, "Setting channel %d edd to %04x, register is %04x\n", channel, new_edd, m_audio_regs[(channel << 4) | AUDIO_ENVELOPE_DATA]);
	}
	else
	{
		new_count--;
		set_envelope_count(channel, new_count);
	}
	LOGMASKED(LOG_ENVELOPES, "envelope %d post-tick, count is now %04x, register is %04x\n", channel, new_count, m_audio_regs[(channel << 4) | AUDIO_ENVELOPE_DATA]);
	return edd_changed;
}