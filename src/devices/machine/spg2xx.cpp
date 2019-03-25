// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*****************************************************************************

    SunPlus SPG2xx-series SoC peripheral emulation

    TODO:
        - Serial UART
        - I2C
        - SPI

**********************************************************************/

#include "emu.h"
#include "spg2xx.h"

DEFINE_DEVICE_TYPE(SPG24X, spg24x_device, "spg24x", "SPG240-series System-on-a-Chip")
DEFINE_DEVICE_TYPE(SPG28X, spg28x_device, "spg28x", "SPG280-series System-on-a-Chip")

#define LOG_UNKNOWN_IO      (1U << 3)
#define LOG_IRQS            (1U << 4)
#define LOG_VLINES          (1U << 5)
#define LOG_DMA             (1U << 9)
#define LOG_PPU_READS       (1U << 22)
#define LOG_PPU_WRITES      (1U << 23)
#define LOG_UNKNOWN_PPU     (1U << 24)
#define LOG_IO              (LOG_IRQS | LOG_DMA | LOG_UNKNOWN_IO)
#define LOG_PPU             (LOG_PPU_READS | LOG_PPU_WRITES | LOG_UNKNOWN_PPU)
#define LOG_ALL             (LOG_IO | LOG_PPU | LOG_VLINES )

#define VERBOSE             (0)
#include "logmacro.h"

#define SPG_DEBUG_VIDEO     (0)

#define VIDEO_IRQ_ENABLE    m_video_regs[0x62]
#define VIDEO_IRQ_STATUS    m_video_regs[0x63]

spg2xx_device::spg2xx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_mixer_interface(mconfig, *this, 2)
	, m_spg_audio(*this, "spgaudio")
	, m_spg_io(*this, "spgio")
	, m_rowscrolloffset(15)
	, m_porta_out(*this)
	, m_portb_out(*this)
	, m_portc_out(*this)
	, m_porta_in(*this)
	, m_portb_in(*this)
	, m_portc_in(*this)
	, m_adc_in{{*this}, {*this}}
	, m_eeprom_w(*this)
	, m_eeprom_r(*this)
	, m_uart_tx(*this)
	, m_chip_sel(*this)
	, m_cpu(*this, finder_base::DUMMY_TAG)
	, m_screen(*this, finder_base::DUMMY_TAG)
	, m_scrollram(*this, "scrollram")
	, m_paletteram(*this, "paletteram")
	, m_spriteram(*this, "spriteram")
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
	map(0x003000, 0x0031ff).rw(m_spg_audio, FUNC(spg2xx_audio_device::audio_r), FUNC(spg2xx_audio_device::audio_w));
	map(0x003200, 0x0033ff).rw(m_spg_audio, FUNC(spg2xx_audio_device::audio_phase_r), FUNC(spg2xx_audio_device::audio_phase_w));
	map(0x003400, 0x0037ff).rw(m_spg_audio, FUNC(spg2xx_audio_device::audio_ctrl_r), FUNC(spg2xx_audio_device::audio_ctrl_w));
	map(0x003d00, 0x003d2f).rw(m_spg_io, FUNC(spg2xx_io_device::io_r), FUNC(spg2xx_io_device::io_w));
	map(0x003d30, 0x003dff).rw(m_spg_io, FUNC(spg2xx_io_device::io_extended_r), FUNC(spg2xx_io_device::io_extended_w));
	map(0x003e00, 0x003e03).rw(FUNC(spg2xx_device::dma_r), FUNC(spg2xx_device::dma_w));
}

void spg2xx_device::device_start()
{
	for (uint8_t i = 0; i < 32; i++)
	{
		m_rgb5_to_rgb8[i] = (i << 3) | (i >> 2);
	}
	for (uint16_t i = 0; i < 0x8000; i++)
	{
		m_rgb555_to_rgb888[i] = (m_rgb5_to_rgb8[(i >> 10) & 0x1f] << 16) |
								(m_rgb5_to_rgb8[(i >>  5) & 0x1f] <<  8) |
								(m_rgb5_to_rgb8[(i >>  0) & 0x1f] <<  0);
	}
	m_porta_out.resolve_safe();
	m_portb_out.resolve_safe();
	m_portc_out.resolve_safe();
	m_porta_in.resolve_safe(0);
	m_portb_in.resolve_safe(0);
	m_portc_in.resolve_safe(0);
	m_adc_in[0].resolve_safe(0x0fff);
	m_adc_in[1].resolve_safe(0x0fff);
	m_eeprom_w.resolve_safe();
	m_eeprom_r.resolve_safe(0);
	m_uart_tx.resolve_safe();
	m_chip_sel.resolve_safe();

	m_screenpos_timer = timer_alloc(TIMER_SCREENPOS);
	m_screenpos_timer->adjust(attotime::never);

	save_item(NAME(m_hide_page0));
	save_item(NAME(m_hide_page1));
	save_item(NAME(m_hide_sprites));
	save_item(NAME(m_debug_sprites));
	save_item(NAME(m_debug_blit));
	save_item(NAME(m_debug_palette));
	save_item(NAME(m_sprite_index_to_debug));

	save_item(NAME(m_dma_regs));

	save_item(NAME(m_video_regs));
	save_item(NAME(m_sprite_limit));
	save_item(NAME(m_pal_flag));
}

void spg2xx_device::device_reset()
{
	memset(m_video_regs, 0, 0x100 * sizeof(uint16_t));
	memset(m_dma_regs, 0, 0x4 * sizeof(uint16_t));

	m_video_regs[0x36] = 0xffff;
	m_video_regs[0x37] = 0xffff;
	m_video_regs[0x3c] = 0x0020;
	m_video_regs[0x42] = 0x0001;

	m_hide_page0 = false;
	m_hide_page1 = false;
	m_hide_sprites = false;
	m_debug_sprites = false;
	m_debug_blit = false;
	m_debug_palette = false;
	m_sprite_index_to_debug = 0;
}


WRITE_LINE_MEMBER(spg2xx_device::audioirq_w)
{
	m_cpu->set_state_unsynced(UNSP_IRQ4_LINE, state);
}

WRITE_LINE_MEMBER(spg2xx_device::timerirq_w)
{
	m_cpu->set_state_unsynced(UNSP_IRQ2_LINE, state);
}

WRITE_LINE_MEMBER(spg2xx_device::uartirq_w)
{
	m_cpu->set_state_unsynced(UNSP_IRQ3_LINE, state);
}

WRITE_LINE_MEMBER(spg2xx_device::extirq_w)
{
	m_cpu->set_state_unsynced(UNSP_IRQ5_LINE, state);
}

WRITE_LINE_MEMBER(spg2xx_device::ffreq1_w)
{
	m_cpu->set_state_unsynced(UNSP_IRQ6_LINE, state);
}

WRITE_LINE_MEMBER(spg2xx_device::ffreq2_w)
{
	m_cpu->set_state_unsynced(UNSP_IRQ7_LINE, state);
}



READ16_MEMBER(spg2xx_device::space_r)
{
	address_space &cpuspace = m_cpu->space(AS_PROGRAM);
	return cpuspace.read_word(offset);
}

/*************************
*     Video Hardware     *
*************************/

// Perform a lerp between a and b
inline uint8_t spg2xx_device::mix_channel(uint8_t bottom, uint8_t top)
{
	uint8_t alpha = (m_video_regs[0x2a] & 3) << 6;
	return ((256 - alpha) * bottom + alpha * top) >> 8;
}

template<spg2xx_device::blend_enable_t Blend, spg2xx_device::rowscroll_enable_t RowScroll, spg2xx_device::flipx_t FlipX>
void spg2xx_device::blit(const rectangle &cliprect, uint32_t line, uint32_t xoff, uint32_t yoff, uint32_t attr, uint32_t ctrl, uint32_t bitmap_addr, uint16_t tile)
{
	address_space &space = m_cpu->space(AS_PROGRAM);

	int32_t h = 8 << ((attr & PAGE_TILE_HEIGHT_MASK) >> PAGE_TILE_HEIGHT_SHIFT);
	int32_t w = 8 << ((attr & PAGE_TILE_WIDTH_MASK) >> PAGE_TILE_WIDTH_SHIFT);

	uint32_t yflipmask = attr & TILE_Y_FLIP ? h - 1 : 0;

	uint32_t nc = ((attr & 0x0003) + 1) << 1;

	uint32_t palette_offset = (attr & 0x0f00) >> 4;
	if (SPG_DEBUG_VIDEO && m_debug_blit)
	{
		printf("s:%d line:%d xy:%08x,%08x attr:%08x ctrl:%08x bitmap_addr:%08x tile:%04x\n", cliprect.min_x, line, xoff, yoff, attr, ctrl, bitmap_addr, tile);
		printf("hw:%d,%d f:%d,%d yfm:%d ncols:%d pobs:%02x ", w, h, (attr & TILE_X_FLIP) ? 1 : 0, (attr & TILE_Y_FLIP) ? 1 : 0, yflipmask, nc, palette_offset);
	}
	palette_offset >>= nc;
	palette_offset <<= nc;
	if (SPG_DEBUG_VIDEO && m_debug_blit)
	{
		printf("poas:%02x\n", palette_offset);
	}

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

	if (SPG_DEBUG_VIDEO && m_debug_blit)
		printf("%3d:\n", yy);

	int y_index = yy * 320;

	for (int32_t x = FlipX ? (w - 1) : 0; FlipX ? x >= 0 : x < w; FlipX ? x-- : x++)
	{
		int xx = xoff + x;

		bits <<= nc;
		if (SPG_DEBUG_VIDEO && m_debug_blit)
			printf("    %08x:%d ", bits, nbits);
		if (nbits < nc)
		{
			uint16_t b = space.read_word(m++ & 0x3fffff);
			b = (b << 8) | (b >> 8);
			bits |= b << (nc - nbits);
			nbits += 16;
			if (SPG_DEBUG_VIDEO && m_debug_blit)
				printf("(%04x:%08x:%d) ", b, bits, nbits);
		}
		nbits -= nc;

		uint32_t pal = palette_offset + (bits >> 16);
		if (SPG_DEBUG_VIDEO && m_debug_blit)
			printf("%02x:%02x:%04x ", bits >> 16, pal, bits & 0xffff);
		bits &= 0xffff;

		if (RowScroll)
			xx -= (int16_t)m_scrollram[(yy + m_rowscrolloffset) & 0x1ff];

		xx &= 0x01ff;
		if (xx >= 0x01c0)
			xx -= 0x0200;

		if (xx >= 0 && xx < 320)
		{
			int pix_index = xx + y_index;

			uint16_t rgb = m_paletteram[pal];
			if (SPG_DEBUG_VIDEO && m_debug_blit)
				printf("rgb:%04x ", rgb);

			if (!(rgb & 0x8000))
			{
				if (Blend)
				{
					if (SPG_DEBUG_VIDEO && m_debug_blit)
						printf("M\n");
					m_screenbuf[pix_index] = (mix_channel((uint8_t)(m_screenbuf[pix_index] >> 16), m_rgb5_to_rgb8[(rgb >> 10) & 0x1f]) << 16) |
											 (mix_channel((uint8_t)(m_screenbuf[pix_index] >>  8), m_rgb5_to_rgb8[(rgb >> 5) & 0x1f]) << 8) |
											 (mix_channel((uint8_t)(m_screenbuf[pix_index] >>  0), m_rgb5_to_rgb8[rgb & 0x1f]));
				}
				else
				{
					if (SPG_DEBUG_VIDEO && m_debug_blit)
						printf("S\n");
					m_screenbuf[pix_index] = m_rgb555_to_rgb888[rgb];
				}
			}
			else if (SPG_DEBUG_VIDEO && m_debug_blit)
			{
				printf("X\n");
			}
		}
	}
}

void spg2xx_device::blit_page(const rectangle &cliprect, uint32_t scanline, int depth, uint32_t bitmap_addr, uint16_t *regs)
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

	uint32_t tile_h = 8 << ((attr & PAGE_TILE_HEIGHT_MASK) >> PAGE_TILE_HEIGHT_SHIFT);
	uint32_t tile_w = 8 << ((attr & PAGE_TILE_WIDTH_MASK) >> PAGE_TILE_WIDTH_SHIFT);

	uint32_t tile_count_x = 512 / tile_w;

	uint32_t bitmap_y = (scanline + yscroll) & 0xff;
	uint32_t y0 = bitmap_y / tile_h;
	uint32_t tile_scanline = bitmap_y % tile_h;
	uint32_t tile_address = tile_count_x * y0;
	if (SPG_DEBUG_VIDEO && machine().input().code_pressed(KEYCODE_H))
		printf("s:%3d | baddr:%08x | yscr:%3d | bity:%3d | y0:%2d | ts:%2d\n", scanline, bitmap_addr, yscroll, bitmap_y, y0, tile_scanline);

	if (SPG_DEBUG_VIDEO && machine().input().code_pressed(KEYCODE_EQUALS))
		m_debug_blit = true;
	for (uint32_t x0 = 0; x0 < tile_count_x; x0++, tile_address++)
	{
		uint32_t yy = ((tile_h * y0 - yscroll + 0x10) & 0xff) - 0x10;
		uint32_t xx = (tile_w * x0 - xscroll) & 0x1ff;
		uint16_t tile = (ctrl & PAGE_WALLPAPER_MASK) ? space.read_word(tilemap) : space.read_word(tilemap + tile_address);
		uint16_t palette = 0;

		if (!tile)
			continue;

		palette = (ctrl & PAGE_WALLPAPER_MASK) ? space.read_word(palette_map) : space.read_word(palette_map + tile_address / 2);
		if (x0 & 1)
			palette >>= 8;

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

		bool blend = (tileattr & 0x4000 || tilectrl & 0x0100);
		bool row_scroll = (tilectrl & 0x0010);
		bool flip_x = (tileattr & TILE_X_FLIP);

		if (blend)
		{
			if (row_scroll)
			{
				if (flip_x)
					blit<BlendOn, RowScrollOn, FlipXOn>(cliprect, tile_scanline, xx, yy, tileattr, tilectrl, bitmap_addr, tile);
				else
					blit<BlendOn, RowScrollOn, FlipXOff>(cliprect, tile_scanline, xx, yy, tileattr, tilectrl, bitmap_addr, tile);
			}
			else
			{
				if (flip_x)
					blit<BlendOn, RowScrollOff, FlipXOn>(cliprect, tile_scanline, xx, yy, tileattr, tilectrl, bitmap_addr, tile);
				else
					blit<BlendOn, RowScrollOff, FlipXOff>(cliprect, tile_scanline, xx, yy, tileattr, tilectrl, bitmap_addr, tile);
			}
		}
		else
		{
			if (row_scroll)
			{
				if (flip_x)
					blit<BlendOff, RowScrollOn, FlipXOn>(cliprect, tile_scanline, xx, yy, tileattr, tilectrl, bitmap_addr, tile);
				else
					blit<BlendOff, RowScrollOn, FlipXOff>(cliprect, tile_scanline, xx, yy, tileattr, tilectrl, bitmap_addr, tile);
			}
			else
			{
				if (flip_x)
					blit<BlendOff, RowScrollOff, FlipXOn>(cliprect, tile_scanline, xx, yy, tileattr, tilectrl, bitmap_addr, tile);
				else
					blit<BlendOff, RowScrollOff, FlipXOff>(cliprect, tile_scanline, xx, yy, tileattr, tilectrl, bitmap_addr, tile);
			}
		}
	}
	if (SPG_DEBUG_VIDEO && machine().input().code_pressed(KEYCODE_EQUALS))
		m_debug_blit = false;
}

void spg2xx_device::blit_sprite(const rectangle &cliprect, uint32_t scanline, int depth, uint32_t base_addr)
{
	uint32_t bitmap_addr = 0x40 * m_video_regs[0x22];
	uint16_t tile = m_spriteram[base_addr + 0];
	int16_t x = m_spriteram[base_addr + 1];
	int16_t y = m_spriteram[base_addr + 2];
	uint16_t attr = m_spriteram[base_addr + 3];

	if (!tile)
	{
		return;
	}

	if (((attr & PAGE_DEPTH_FLAG_MASK) >> PAGE_DEPTH_FLAG_SHIFT) != depth)
	{
		return;
	}

	const uint32_t h = 8 << ((attr & PAGE_TILE_HEIGHT_MASK) >> PAGE_TILE_HEIGHT_SHIFT);
	const uint32_t w = 8 << ((attr & PAGE_TILE_WIDTH_MASK) >> PAGE_TILE_WIDTH_SHIFT);

	if (!(m_video_regs[0x42] & SPRITE_COORD_TL_MASK))
	{
		x = (160 + x) - w / 2;
		y = (120 - y) - (h / 2) + 8;
	}

	x &= 0x01ff;
	y &= 0x01ff;

	uint32_t tile_line = ((scanline - y) + 0x200) % h;
	int16_t test_y = (y + tile_line) & 0x1ff;
	if (test_y >= 0x01c0)
		test_y -= 0x0200;

	if (test_y != scanline)
	{
		return;
	}

	bool blend = (attr & 0x4000);
	bool flip_x = (attr & TILE_X_FLIP);

#if SPG_DEBUG_VIDEO
	if (m_debug_sprites && machine().input().code_pressed(KEYCODE_MINUS))
		m_debug_blit = true;
	if (blend)
	{
		if (flip_x)
			blit<BlendOn, RowScrollOff, FlipXOn>(cliprect, tile_line, x, y, attr, 0, bitmap_addr, tile);
		else
			blit<BlendOn, RowScrollOff, FlipXOff>(cliprect, tile_line, x, y, attr, 0, bitmap_addr, tile);
	}
	else
	{
		if (flip_x)
			blit<BlendOff, RowScrollOff, FlipXOn>(cliprect, tile_line, x, y, attr, 0, bitmap_addr, tile);
		else
			blit<BlendOff, RowScrollOff, FlipXOff>(cliprect, tile_line, x, y, attr, 0, bitmap_addr, tile);
	}
	m_debug_blit = false;
#else
	if (blend)
	{
		if (flip_x)
			blit<BlendOn, RowScrollOff, FlipXOn>(cliprect, tile_line, x, y, attr, 0, bitmap_addr, tile);
		else
			blit<BlendOn, RowScrollOff, FlipXOff>(cliprect, tile_line, x, y, attr, 0, bitmap_addr, tile);
	}
	else
	{
		if (flip_x)
			blit<BlendOff, RowScrollOff, FlipXOn>(cliprect, tile_line, x, y, attr, 0, bitmap_addr, tile);
		else
			blit<BlendOff, RowScrollOff, FlipXOff>(cliprect, tile_line, x, y, attr, 0, bitmap_addr, tile);
	}
#endif
}

void spg2xx_device::blit_sprites(const rectangle &cliprect, uint32_t scanline, int depth)
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
			blit_sprite(cliprect, scanline, depth, 4 * n);
		}
#if SPG_DEBUG_VIDEO
	}
	else
	{
		blit_sprite(cliprect, scanline, depth, 4 * m_sprite_index_to_debug);
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
		uint32_t *src = &m_screenbuf[cliprect.min_x + 320 * y];
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			const uint32_t src_rgb = *src;
			const float src_r = (uint8_t)(src_rgb >> 16) * s_u8_to_float;
			const float src_g = (uint8_t)(src_rgb >>  8) * s_u8_to_float;
			const float src_b = (uint8_t)(src_rgb >>  0) * s_u8_to_float;
			const float luma = src_r * s_gray_r + src_g * s_gray_g + src_b * s_gray_b;
			const float adjusted_r = luma + (src_r - luma) * sat_adjust;
			const float adjusted_g = luma + (src_g - luma) * sat_adjust;
			const float adjusted_b = luma + (src_b - luma) * sat_adjust;
			const int integer_r = (int)floor(adjusted_r * 255.0f);
			const int integer_g = (int)floor(adjusted_g * 255.0f);
			const int integer_b = (int)floor(adjusted_b * 255.0f);
			*src++ = (integer_r > 255 ? 0xff0000 : (integer_r < 0 ? 0 : ((uint8_t)integer_r << 16))) |
					 (integer_g > 255 ? 0x00ff00 : (integer_g < 0 ? 0 : ((uint8_t)integer_g << 8))) |
					 (integer_b > 255 ? 0x0000ff : (integer_b < 0 ? 0 : (uint8_t)integer_b));
		}
	}
}

void spg2xx_device::apply_fade(const rectangle &cliprect)
{
	const uint16_t fade_offset = m_video_regs[0x30];
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint32_t *src = &m_screenbuf[cliprect.min_x + 320 * y];
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			const uint32_t src_rgb = *src;
			const uint8_t src_r = (src_rgb >> 16) & 0xff;
			const uint8_t src_g = (src_rgb >>  8) & 0xff;
			const uint8_t src_b = (src_rgb >>  0) & 0xff;
			const uint8_t r = src_r - fade_offset;
			const uint8_t g = src_g - fade_offset;
			const uint8_t b = src_b - fade_offset;
			*src++ = (r > src_r ? 0 : (r << 16)) |
					 (g > src_g ? 0 : (g <<  8)) |
					 (b > src_b ? 0 : (b <<  0));
		}
	}
}

uint32_t spg2xx_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	memset(&m_screenbuf[320 * cliprect.min_y], 0, 4 * 320 * ((cliprect.max_y - cliprect.min_y) + 1));

	const uint32_t page1_addr = 0x40 * m_video_regs[0x20];
	const uint32_t page2_addr = 0x40 * m_video_regs[0x21];
	uint16_t *page1_regs = m_video_regs + 0x10;
	uint16_t *page2_regs = m_video_regs + 0x16;

	for (uint32_t scanline = (uint32_t)cliprect.min_y; scanline <= (uint32_t)cliprect.max_y; scanline++)
	{
		for (int i = 0; i < 4; i++)
		{
			if (!SPG_DEBUG_VIDEO || !m_hide_page0)
				blit_page(cliprect, scanline, i, page1_addr, page1_regs);
			if (!SPG_DEBUG_VIDEO || !m_hide_page1)
				blit_page(cliprect, scanline, i, page2_addr, page2_regs);
			if (!SPG_DEBUG_VIDEO || !m_hide_sprites)
				blit_sprites(cliprect, scanline, i);
		}
	}

	if ((m_video_regs[0x3c] & 0x00ff) != 0x0020)
	{
		apply_saturation(cliprect);
	}

	if (m_video_regs[0x30] != 0)
	{
		apply_fade(cliprect);
	}

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint32_t *dest = &bitmap.pix32(y, cliprect.min_x);
		uint32_t *src = &m_screenbuf[cliprect.min_x + 320 * y];
		memcpy(dest, src, sizeof(uint32_t) * ((cliprect.max_x - cliprect.min_x) + 1));
	}

	if (SPG_DEBUG_VIDEO && m_debug_palette)
	{
		for (int y = cliprect.min_y; y <= cliprect.max_y && y < 128; y++)
		{
			const uint16_t high_nybble = (y / 8) << 4;
			uint32_t *dest = &bitmap.pix32(y, cliprect.min_x);
			for (int x = cliprect.min_x; x <= cliprect.max_x && x < 256; x++)
			{
				const uint16_t low_nybble = x / 16;
				const uint16_t palette_entry = high_nybble | low_nybble;
				const uint16_t color = m_paletteram[palette_entry];
				if (!(color & 0x8000))
				{
					*dest = m_rgb555_to_rgb888[color & 0x7fff];
				}
				dest++;
			}
		}
	}

	return 0;
}

void spg2xx_device::do_sprite_dma(uint32_t len)
{
	address_space &mem = m_cpu->space(AS_PROGRAM);

	uint32_t src = m_video_regs[0x70] & 0x3fff;
	uint32_t dst = m_video_regs[0x71];

	for (uint32_t j = 0; j < len; j++)
	{
		m_spriteram[(dst + j) & 0x3ff] = mem.read_word(src + j);
	}

	m_video_regs[0x72] = 0;
	if (VIDEO_IRQ_ENABLE & 4)
	{
		const uint16_t old = VIDEO_IRQ_STATUS;
		VIDEO_IRQ_STATUS |= 4;
		const uint16_t changed = old ^ (VIDEO_IRQ_ENABLE & VIDEO_IRQ_STATUS);
		if (changed)
			check_video_irq();
	}
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
		LOGMASKED(LOG_UNKNOWN_PPU, "video_r: Unknown register %04x = %04x\n", 0x2800 + offset, m_video_regs[offset]);
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
	{
		LOGMASKED(LOG_DMA, "video_w: Sprite DMA Length = %04x\n", data & 0x03ff);
		uint16_t length = data & 0x3ff;
		do_sprite_dma(length ? length : 0x400);
		break;
	}

	default:
		LOGMASKED(LOG_UNKNOWN_PPU, "video_w: Unknown register %04x = %04x\n", 0x2800 + offset, data);
		m_video_regs[offset] = data;
		break;
	}
}

WRITE_LINE_MEMBER(spg2xx_device::vblank)
{
	if (!state)
	{
		VIDEO_IRQ_STATUS &= ~1;
		LOGMASKED(LOG_IRQS, "Setting video IRQ status to %04x\n", VIDEO_IRQ_STATUS);
		check_video_irq();
		return;
	}

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
	if (machine().input().code_pressed_once(KEYCODE_L))
		m_debug_palette = !m_debug_palette;
#endif

	if (VIDEO_IRQ_ENABLE & 1)
	{
		VIDEO_IRQ_STATUS |= 1;
		LOGMASKED(LOG_IRQS, "Setting video IRQ status to %04x\n", VIDEO_IRQ_STATUS);
		check_video_irq();
	}
}

void spg2xx_device::check_video_irq()
{
	LOGMASKED(LOG_IRQS, "%ssserting IRQ0 (%04x, %04x)\n", (VIDEO_IRQ_STATUS & VIDEO_IRQ_ENABLE) ? "A" : "Dea", VIDEO_IRQ_STATUS, VIDEO_IRQ_ENABLE);
	m_cpu->set_state_unsynced(UNSP_IRQ0_LINE, (VIDEO_IRQ_STATUS & VIDEO_IRQ_ENABLE) ? ASSERT_LINE : CLEAR_LINE);
}


/*************************
*    Machine Hardware    *
*************************/

READ16_MEMBER(spg2xx_device::dma_r)
{
	uint16_t val = m_dma_regs[offset];
	switch (offset)
	{

	case 0x000: // DMA Source (L)
		LOGMASKED(LOG_DMA, "dma_r: DMA Source (lo) = %04x\n", val);
		break;

	case 0x001: // DMA Source (H)
		LOGMASKED(LOG_DMA, "dma_r: DMA Source (hi) = %04x\n", val);
		break;

	case 0x002: // DMA Length
		LOGMASKED(LOG_DMA, "dma_r: DMA Length = %04x\n", 0);
		val = 0;
		break;

	case 0x003: // DMA Destination
		LOGMASKED(LOG_DMA, "dma_r: DMA Dest = %04x\n", val);
		break;

	default:
		LOGMASKED(LOG_UNKNOWN_IO, "dma_r: Unknown register %04x\n", 0x3d00 + offset);
		break;
	}

	return val;
}


WRITE16_MEMBER(spg2xx_device::dma_w)
{
	switch (offset)
	{
	case 0x000: // DMA Source (lo)
		LOGMASKED(LOG_DMA, "dma_w: DMA Source (lo) = %04x\n", data);
		m_dma_regs[offset] = data;
		break;

	case 0x001: // DMA Source (hi)
		LOGMASKED(LOG_DMA, "dma_w: DMA Source (hi) = %04x\n", data);
		m_dma_regs[offset] = data;
		break;

	case 0x002: // DMA Length
		LOGMASKED(LOG_DMA, "dma_w: DMA Length = %04x\n", data);
		if (!(data & 0xc000))  // jak_dora writes 0xffff here which ends up trashing registers etc. why? such writes can't be valid
			do_cpu_dma(data);
		break;

	case 0x003: // DMA Destination
		LOGMASKED(LOG_DMA, "dma_w: DMA Dest = %04x\n", data);
		m_dma_regs[offset] = data;
		break;

	default:
		LOGMASKED(LOG_UNKNOWN_IO, "dma_w: Unknown register %04x = %04x\n", 0x3d00 + offset, data);
		m_dma_regs[offset] = data;
		break;
	}
}

void spg2xx_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_SCREENPOS:
		{
			if (VIDEO_IRQ_ENABLE & 2)
			{
				VIDEO_IRQ_STATUS |= 2;
				check_video_irq();
			}
			m_screen->update_partial(m_screen->vpos());

			// fire again, jak_dbz pinball needs this
			m_screenpos_timer->adjust(m_screen->time_until_pos(m_video_regs[0x36], m_video_regs[0x37] << 1));
			break;
		}
	}
}


void spg2xx_device::do_cpu_dma(uint32_t len)
{
	address_space &mem = m_cpu->space(AS_PROGRAM);

	uint32_t src = ((m_dma_regs[0x001] & 0x3f) << 16) | m_dma_regs[0x000];
	uint32_t dst = m_dma_regs[0x003] & 0x3fff;

	for (uint32_t j = 0; j < len; j++)
	{
		mem.write_word((dst + j) & 0x3fff, mem.read_word(src + j));
	}

	src += len;
	m_dma_regs[0x000] = (uint16_t)src;
	m_dma_regs[0x001] = (src >> 16) & 0x3f;
	m_dma_regs[0x002] = 0;
	m_dma_regs[0x003] = (dst + len) & 0x3fff;
}

void spg2xx_device::configure_spg_io(spg2xx_io_device* io)
{
	io->porta_in().set(FUNC(spg2xx_device::porta_r));
	io->portb_in().set(FUNC(spg2xx_device::portb_r));
	io->portc_in().set(FUNC(spg2xx_device::portc_r));
	io->porta_out().set(FUNC(spg2xx_device::porta_w));
	io->portb_out().set(FUNC(spg2xx_device::portb_w));
	io->portc_out().set(FUNC(spg2xx_device::portc_w));
	io->adc_in<0>().set(FUNC(spg2xx_device::adc_r<0>));
	io->adc_in<1>().set(FUNC(spg2xx_device::adc_r<1>));
	io->eeprom_w().set(FUNC(spg2xx_device::eepromx_w));
	io->eeprom_r().set(FUNC(spg2xx_device::eepromx_r));
	io->uart_tx().set(FUNC(spg2xx_device::tx_w));
	io->chip_select().set(FUNC(spg2xx_device::cs_w));
	io->pal_read_callback().set(FUNC(spg2xx_device::get_pal_r));
	io->write_timer_irq_callback().set(FUNC(spg2xx_device::timerirq_w));
	io->write_uart_adc_irq_callback().set(FUNC(spg2xx_device::uartirq_w));
	io->write_external_irq_callback().set(FUNC(spg2xx_device::extirq_w));
	io->write_ffrq_tmr1_irq_callback().set(FUNC(spg2xx_device::ffreq1_w));
	io->write_ffrq_tmr2_irq_callback().set(FUNC(spg2xx_device::ffreq2_w));
}

void spg24x_device::device_add_mconfig(machine_config &config)
{
	SPG2XX_AUDIO(config, m_spg_audio, DERIVED_CLOCK(1, 1));
	m_spg_audio->write_irq_callback().set(FUNC(spg24x_device::audioirq_w));
	m_spg_audio->space_read_callback().set(FUNC(spg24x_device::space_r));

	m_spg_audio->add_route(0, *this, 1.0, AUTO_ALLOC_INPUT, 0);
	m_spg_audio->add_route(1, *this, 1.0, AUTO_ALLOC_INPUT, 1);

	SPG24X_IO(config, m_spg_io, DERIVED_CLOCK(1, 1), m_cpu, m_screen);
	configure_spg_io(m_spg_io);
}

void spg28x_device::device_add_mconfig(machine_config &config)
{
	SPG2XX_AUDIO(config, m_spg_audio, DERIVED_CLOCK(1, 1));
	m_spg_audio->write_irq_callback().set(FUNC(spg28x_device::audioirq_w));
	m_spg_audio->space_read_callback().set(FUNC(spg28x_device::space_r));

	m_spg_audio->add_route(0, *this, 1.0, AUTO_ALLOC_INPUT, 0);
	m_spg_audio->add_route(1, *this, 1.0, AUTO_ALLOC_INPUT, 1);

	SPG28X_IO(config, m_spg_io, DERIVED_CLOCK(1, 1), m_cpu, m_screen);
	configure_spg_io(m_spg_io);
}
