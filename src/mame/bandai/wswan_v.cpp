// license:BSD-3-Clause
// copyright-holders:Anthony Kruize, Wilbert Pol
/***************************************************************************

 File to handle video emulation of the Bandai WonderSwan VDP.

 Anthony Kruize
 Wilbert Pol

 TODO:
   - Add support for WSC high/low contrast (register 14, bit 1)

 ***************************************************************************/

#include "emu.h"
#include "wswan_v.h"
#include "screen.h"

#define LOG_UNKNOWN    (1 << 1)
#define LOG_SCANLINE   (1 << 2)

#define LOG_ALL        (LOG_UNKNOWN | LOG_SCANLINE)

#define VERBOSE        (0)

#include "logmacro.h"

#define LOGUNKNOWN(...)  LOGMASKED(LOG_UNKNOWN, __VA_ARGS__)
#define LOGSCANLINE(...) LOGMASKED(LOG_SCANLINE, __VA_ARGS__)

DEFINE_DEVICE_TYPE(WSWAN_VIDEO, wswan_video_device, "wswan_video", "Bandai WonderSwan VDP")
DEFINE_DEVICE_TYPE(WSWAN_COLOR_VIDEO, wswan_color_video_device, "wscolor_video", "Bandai WonderSwan Color VDP")


wswan_video_device::wswan_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int vdp_type)
	: device_t(mconfig, type, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, device_palette_interface(mconfig, *this)
	, m_set_irq_cb(*this)
	, m_vdp_type(vdp_type)
	, m_icons_cb(*this)
	, m_color_mode_cb(*this)
{
}

wswan_video_device::wswan_video_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: wswan_video_device(mconfig, WSWAN_VIDEO, tag, owner, clock, VDP_TYPE_WSWAN)
{
}

wswan_color_video_device::wswan_color_video_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: wswan_video_device(mconfig, WSWAN_COLOR_VIDEO, tag, owner, clock, VDP_TYPE_WSC)
{
}

wswan_video_device::~wswan_video_device()
{
}


void wswan_video_device::device_start()
{
	init_palettes();
	screen().register_screen_bitmap(m_bitmap);

	m_timer = timer_alloc(FUNC(wswan_video_device::scanline_interrupt), this);
	m_timer->adjust(attotime::from_ticks(256, clock()), 0, attotime::from_ticks(256, clock()));

	// bind callbacks
	m_set_irq_cb.resolve();

	const size_t vram_size = (is_color_vdp() ? 0x10000 : 0x4000) >> 1;
	m_vram = make_unique_clear<u16 []>(vram_size);

	save_item(NAME(m_bitmap));
	save_pointer(NAME(m_vram), vram_size);
	save_item(NAME(m_palette_port));
	save_item(NAME(m_regs));

	save_item(NAME(m_layer_bg_enable));
	save_item(NAME(m_layer_fg_enable));
	save_item(NAME(m_sprites_enable));
	save_item(NAME(m_window_sprites_enable));
	save_item(NAME(m_window_fg_mode));
	save_item(NAME(m_bg_control));
	save_item(NAME(m_current_line));
	save_item(NAME(m_line_compare));
	save_item(NAME(m_sprite_table_address));
	save_item(NAME(m_sprite_table_buffer));
	save_item(NAME(m_sprite_first));
	save_item(NAME(m_sprite_count));
	save_item(NAME(m_sprite_first_latch));
	save_item(NAME(m_sprite_count_latch));
	save_item(NAME(m_layer_bg_address));
	save_item(NAME(m_layer_fg_address));
	save_item(NAME(m_window_fg_left));
	save_item(NAME(m_window_fg_top));
	save_item(NAME(m_window_fg_right));
	save_item(NAME(m_window_fg_bottom));
	save_item(NAME(m_window_sprites_left));
	save_item(NAME(m_window_sprites_top));
	save_item(NAME(m_window_sprites_right));
	save_item(NAME(m_window_sprites_bottom));
	save_item(NAME(m_layer_bg_scroll_x));
	save_item(NAME(m_layer_bg_scroll_y));
	save_item(NAME(m_layer_fg_scroll_x));
	save_item(NAME(m_layer_fg_scroll_y));
	save_item(NAME(m_lcd_control));
	save_item(NAME(m_color_mode));
	save_item(NAME(m_colors_16));
	save_item(NAME(m_tile_packed));
	save_item(NAME(m_timer_hblank_enable));
	save_item(NAME(m_timer_hblank_mode));
	save_item(NAME(m_timer_hblank_reload));
	save_item(NAME(m_timer_hblank_count));
	save_item(NAME(m_timer_vblank_enable));
	save_item(NAME(m_timer_vblank_mode));
	save_item(NAME(m_timer_vblank_reload));
	save_item(NAME(m_timer_vblank_count));
}

void wswan_video_device::device_reset()
{
	m_layer_bg_enable = false;
	m_layer_fg_enable = false;
	m_sprites_enable = false;
	m_window_sprites_enable = false;
	m_window_fg_mode = 0;
	m_bg_control = 0;
	m_current_line = 0;
	m_line_compare = 0;
	m_sprite_table_address = 0;
	m_sprite_first = 0;
	m_sprite_count = 0;
	m_sprite_first_latch = 0;
	m_sprite_count_latch = 0;
	m_layer_bg_address = 0;
	m_layer_fg_address = 0;
	m_window_fg_left = 0;
	m_window_fg_top = 0;
	m_window_fg_right = 0;
	m_window_fg_bottom = 0;
	m_window_sprites_left = 0;
	m_window_sprites_top = 0;
	m_window_sprites_right = 0;
	m_window_sprites_bottom = 0;
	m_layer_bg_scroll_x = 0;
	m_layer_bg_scroll_y = 0;
	m_layer_fg_scroll_x = 0;
	m_layer_fg_scroll_y = 0;
	m_lcd_control = 0;
	m_color_mode = false;
	m_colors_16 = false;
	m_tile_packed = false;
	m_timer_hblank_enable = false;
	m_timer_hblank_mode = false;
	m_timer_hblank_reload = 0;
	m_timer_hblank_count = 0;
	m_timer_vblank_enable = false;
	m_timer_vblank_mode = false;
	m_timer_vblank_reload = 0;
	m_timer_vblank_count = 0;

	std::fill(std::begin(m_sprite_table_buffer), std::end(m_sprite_table_buffer), 0);
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	std::fill(std::begin(m_palette_port), std::end(m_palette_port), 0);

	setup_palettes();
}

void wswan_video_device::init_palettes()
{
	for (int i = 0; i < 8; i++)
	{
		set_indirect_color(i, rgb_t::white());
	}
	if (is_color_vdp())
	{
		for (int i = 0; i < 4096; i++)
		{
			u16 const r = (i & 0x0f00) >> 8;
			u16 const g = (i & 0x00f0) >> 4;
			u16 const b = i & 0x000f;
			set_indirect_color(COLOR_12BIT + i, rgb_t(pal4bit(r), pal4bit(g), pal4bit(b)));
		}
		set_pen_indirect(256, COLOR_12BIT + 0xfff); // background color for monochrome mode if LCD off
		set_pen_indirect(256 + 1, COLOR_12BIT); // background color for color mode if LCD off
	}
	else
	{
		// background color if LCD off
		set_indirect_color(8, rgb_t::white());
		set_pen_indirect(BG_COLOR + 1, 8);
	}
}

void wswan_video_device::setup_palettes()
{
	if (is_color_vdp() && m_color_mode)
	{
		for (int i = 0; i < 256; i++)
			set_pen_indirect(i, COLOR_12BIT + (m_vram[WSC_VRAM_PALETTE + i] & 0x0fff));
	}
	else
	{
		for (int i = 0; i < 16; i++)
		{
			set_pen_indirect((i << 2) + 0, (m_palette_port[i] >> 0) & 0x07);
			set_pen_indirect((i << 2) + 1, (m_palette_port[i] >> 4) & 0x07);
			set_pen_indirect((i << 2) + 2, (m_palette_port[i] >> 8) & 0x07);
			set_pen_indirect((i << 2) + 3, (m_palette_port[i] >> 12) & 0x07);
		}
		set_pen_indirect(BG_COLOR, m_bg_control & 0x07);
	}
}

inline void wswan_video_device::get_tile_data(u16 map_addr, u8 scrolly, u16 &data, u16 &number, u16 &palette, int &line)
{
	data = m_vram[map_addr];
	number = data & 0x01ff;
	palette = (data >> 9) & 0x0f;

	line = (m_current_line + scrolly) & 0x07;
	if (BIT(data, 15)) // vflip
		line = 7 - line;
}

inline void wswan_video_device::get_planes(bool bank, u32 number, int line, u32 &plane0, u32 &plane1, u32 &plane2, u32 &plane3)
{
	if (m_colors_16)
	{
		const u16 tile_address = (bank ? 0x4000 : 0x2000) + (number * 16) + (line << 1);
		if (m_tile_packed)
		{
			plane0 = (swapendian_int16(m_vram[tile_address]) << 16) | swapendian_int16(m_vram[tile_address + 1]);
		}
		else
		{
			plane0 = m_vram[tile_address] & 0xff;
			plane1 = (m_vram[tile_address] & 0xff00) >> 7;
			plane2 = (m_vram[tile_address + 1] & 0xff) << 2;
			plane3 = (m_vram[tile_address + 1] & 0xff00) >> 5;
		}
	}
	else
	{
		const u16 tile_address = ((m_color_mode && bank) ? 0x2000 : 0x1000) + (number * 8) + line;
		if (m_tile_packed)
		{
			plane0 = m_vram[tile_address];
		}
		else
		{
			plane0 = m_vram[tile_address] & 0xff;
			plane1 = (m_vram[tile_address] & 0xff00) >> 7;
			plane2 = 0;
			plane3 = 0;
		}
	}
}

inline u8 wswan_video_device::extract_planes(u32 &plane0, u32 &plane1, u32 &plane2, u32 &plane3)
{
	u8 col;
	if (m_tile_packed)
	{
		if (m_colors_16)
		{
			col = plane0 & 0x0f;
			plane0 >>= 4;
		}
		else
		{
			col = plane0 & 0x03;
			plane0 >>= 2;
		}
	}
	else
	{
		col = (plane3 & 8) | (plane2 & 4) | (plane1 & 2) | (plane0 & 1);
		plane3 >>= 1;
		plane2 >>= 1;
		plane1 >>= 1;
		plane0 >>= 1;
	}
	return col;
}

inline int wswan_video_device::get_xoffset(bool hflip, int x, int column, u8 scrollx)
{
	if (hflip)
		return x + (column << 3) - (scrollx & 0x07);
	else
		return 7 - x + (column << 3) - (scrollx & 0x07);
}

inline void wswan_video_device::draw_pixel(int x_offset, u8 tile_palette, u8 pixel)
{
	if (m_colors_16)
	{
		if (pixel)
		{
			m_bitmap.pix(m_current_line, x_offset) = (tile_palette << 4) | pixel;
		}
	}
	else
	{
		if (pixel || BIT(~tile_palette, 2))
		{
			if (m_color_mode)
				m_bitmap.pix(m_current_line, x_offset) = (tile_palette << 4) | pixel;
			else
				m_bitmap.pix(m_current_line, x_offset) = (tile_palette << 2) | pixel;
		}
	}
}

void wswan_video_device::draw_background()
{
	const u16 map_addr = (m_layer_bg_address & (m_color_mode ? 0x3fff : 0x1fff)) + (((m_current_line + m_layer_bg_scroll_y) & 0xf8) << 2);
	const u8 start_column = (m_layer_bg_scroll_x >> 3);

	for (int column = 0; column < 29; column++)
	{
		u32 plane0 = 0, plane1 = 0, plane2 = 0, plane3 = 0;
		u16 tile_data, tile_number, tile_palette;
		int tile_line;
		get_tile_data(map_addr + ((start_column + column) & 0x1f), m_layer_bg_scroll_y, tile_data, tile_number, tile_palette, tile_line);
		get_planes(BIT(tile_data, 13), tile_number, tile_line, plane0, plane1, plane2, plane3);

		for (int x = 0; x < 8; x++)
		{
			const u8 col = extract_planes(plane0, plane1, plane2, plane3);
			const int x_offset = get_xoffset(BIT(tile_data, 14), x, column, m_layer_bg_scroll_x);
			if (x_offset >= 0 && x_offset < WSWAN_X_PIXELS)
			{
				draw_pixel(x_offset, tile_palette, col);
			}
		}
	}
}

void wswan_video_device::draw_foreground_0()
{
	const u16 map_addr = (m_layer_fg_address & (m_color_mode ? 0x3fff : 0x1fff)) + (((m_current_line + m_layer_fg_scroll_y) & 0xf8) << 2);
	const u8 start_column = (m_layer_fg_scroll_x >> 3);

	for (int column = 0; column < 29; column++)
	{
		u32 plane0 = 0, plane1 = 0, plane2 = 0, plane3 = 0;
		u16 tile_data, tile_number, tile_palette;
		int tile_line;
		get_tile_data(map_addr + ((start_column + column) & 0x1f), m_layer_fg_scroll_y, tile_data, tile_number, tile_palette, tile_line);
		get_planes(BIT(tile_data, 13), tile_number, tile_line, plane0, plane1, plane2, plane3);

		for (int x = 0; x < 8; x++)
		{
			const u8 col = extract_planes(plane0, plane1, plane2, plane3);
			const int x_offset = get_xoffset(BIT(tile_data, 14), x, column, m_layer_fg_scroll_x);
			if (x_offset >= 0 && x_offset < WSWAN_X_PIXELS)
			{
				draw_pixel(x_offset, tile_palette, col);
			}
		}
	}
}

void wswan_video_device::draw_foreground_2()
{
	const u16 map_addr = (m_layer_fg_address & (m_color_mode ? 0x3fff : 0x1fff)) + (((m_current_line + m_layer_fg_scroll_y) & 0xf8) << 2);
	const u8 start_column = (m_layer_fg_scroll_x >> 3);

	for (int column = 0; column < 29; column++)
	{
		u32 plane0 = 0, plane1 = 0, plane2 = 0, plane3 = 0;
		u16 tile_data, tile_number, tile_palette;
		int tile_line;
		get_tile_data(map_addr + ((start_column + column) & 0x1f), m_layer_fg_scroll_y, tile_data, tile_number, tile_palette, tile_line);
		get_planes(BIT(tile_data, 13), tile_number, tile_line, plane0, plane1, plane2, plane3);

		for (int x = 0; x < 8; x++)
		{
			const u8 col = extract_planes(plane0, plane1, plane2, plane3);
			const int x_offset = get_xoffset(BIT(tile_data, 14), x, column, m_layer_fg_scroll_x);
			if (x_offset >= 0 && x_offset >= m_window_fg_left && x_offset <= m_window_fg_right && x_offset < WSWAN_X_PIXELS)
			{
				draw_pixel(x_offset, tile_palette, col);
			}
		}
	}
}

void wswan_video_device::draw_foreground_3()
{
	const u16 map_addr = (m_layer_fg_address & (m_color_mode ? 0x3fff : 0x1fff)) + (((m_current_line + m_layer_fg_scroll_y) & 0xf8) << 2);
	const u8 start_column = (m_layer_fg_scroll_x >> 3);

	for (int column = 0; column < 29; column++)
	{
		u32 plane0 = 0, plane1 = 0, plane2 = 0, plane3 = 0;
		u16 tile_data, tile_number, tile_palette;
		int tile_line;
		get_tile_data(map_addr + ((start_column + column) & 0x1f), m_layer_fg_scroll_y, tile_data, tile_number, tile_palette, tile_line);
		get_planes(BIT(tile_data, 13), tile_number, tile_line, plane0, plane1, plane2, plane3);

		for (int x = 0; x < 8; x++)
		{
			const u8 col = extract_planes(plane0, plane1, plane2, plane3);
			const int x_offset = get_xoffset(BIT(tile_data, 14), x, column, m_layer_fg_scroll_x);
			if ((x_offset >= 0 && x_offset < m_window_fg_left) || (x_offset > m_window_fg_right && x_offset < WSWAN_X_PIXELS))
			{
				draw_pixel(x_offset, tile_palette, col);
			}
		}
	}
}

void wswan_video_device::handle_sprites(bool mask)
{
	if (m_sprite_count == 0)
		return;

	for (int i = m_sprite_first + m_sprite_count - 1; i >= m_sprite_first; i--)
	{
		const u16 tile_data = m_sprite_table_buffer[i * 2];
		const bool pri = BIT(tile_data, 13);
		const u8 y = m_sprite_table_buffer[(i * 2) + 1] & 0xff;
		const u8 x = m_sprite_table_buffer[(i * 2) + 1] >> 8;
		int tile_line = (m_current_line - y) & 0xff;

		if ((tile_line >= 0) && (tile_line < 8) && (pri == mask))
		{
			u32 plane0 = 0, plane1 = 0, plane2 = 0, plane3 = 0;
			const int tile_number = tile_data & 0x01ff;
			const int tile_palette = 8 + ((tile_data >> 9) & 0x07);
			const u16 window = BIT(tile_data, 12);
			bool check_clip = false;

			if (BIT(tile_data, 15))
				tile_line = 7 - tile_line;

			get_planes(false, tile_number, tile_line, plane0, plane1, plane2, plane3);

			if (m_window_sprites_enable)
			{
				if (window)
				{
					if (m_current_line >= m_window_sprites_top && m_current_line <= m_window_sprites_bottom)
						check_clip = true;
				}
				else
				{
					if (m_current_line < m_window_sprites_top || m_current_line > m_window_sprites_bottom)
						continue;
				}
			}

			for (int j = 0; j < 8; j++)
			{
				const u8 col = extract_planes(plane0, plane1, plane2, plane3);

				int x_offset;
				if (BIT(tile_data, 14))
					x_offset = x + j;
				else
					x_offset = x + 7 - j;

				x_offset &= 0xff;

				if (m_window_sprites_enable)
				{
					if (window && check_clip)
					{
						if (x_offset >= m_window_sprites_left && x_offset <= m_window_sprites_right)
							continue;
					}
					else
					{
						if (x_offset < m_window_sprites_left || x_offset > m_window_sprites_right)
						{
//                          continue;
						}
					}
				}
				if (x_offset >= 0 && x_offset < WSWAN_X_PIXELS)
				{
					draw_pixel(x_offset, tile_palette, col);
				}
			}
		}
	}
}


void wswan_video_device::refresh_scanline()
{
	setup_palettes();

	rectangle rec(0, WSWAN_X_PIXELS, m_current_line, m_current_line);
	if (m_lcd_control)
	{
		m_bitmap.fill(m_color_mode ? m_bg_control : BG_COLOR, rec);
	}
	else
	{
		m_bitmap.fill(is_color_vdp() ? (256 + (m_color_mode ? 1 : 0)) : (BG_COLOR + 1), rec);
		return;
	}

	// Draw background layer
	if (m_layer_bg_enable)
		draw_background();

	// Draw sprites between background and foreground layers
	if (m_sprites_enable)
		handle_sprites(false);

	// Draw foreground layer, taking window settings into account
	if (m_layer_fg_enable)
	{
		switch (m_window_fg_mode)
		{
			case 0: // FG inside & outside window area
				draw_foreground_0();
				break;
			case 1: // ???
				LOGUNKNOWN("Unknown foreground mode 1 set\n");
				break;
			case 2: // FG only inside window area
				if (m_current_line >= m_window_fg_top && m_current_line <= m_window_fg_bottom)
					draw_foreground_2();
				break;
			case 3: // FG only outside window area
				if (m_current_line < m_window_fg_top || m_current_line > m_window_fg_bottom)
					draw_foreground_0();
				else
					draw_foreground_3();
				break;
		}
	}

	// Draw sprites in front of foreground layer
	if (m_sprites_enable)
		handle_sprites(true);
}



u32 wswan_video_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}


u16 wswan_video_device::reg_r(offs_t offset, u16 mem_mask)
{
	const u16 value = m_regs[offset & 0x7f];

	switch (offset)
	{
		case 0x02 / 2:
			return (value & 0xff00) | m_current_line;
		case 0xa8 / 2:
			return m_timer_hblank_count;
		case 0xaa / 2:
			return m_timer_vblank_count;
	}

	return value;
}


void wswan_video_device::reg_w(offs_t offset, u16 data, u16 mem_mask)
{
	switch (offset)
	{
		case 0x00 / 2:
			// Display control
			// Bit 0   - Background layer enable
			// Bit 1   - Foreground layer enable
			// Bit 2   - Sprites enable
			// Bit 3   - Sprite window enable
			// Bit 4-5 - Foreground window configuration
			//      00 - Foreground layer is displayed inside and outside foreground window area
			//      01 - Unknown
			//      10 - Foreground layer is displayed only inside foreground window area
			//      11 - Foreground layer is displayed outside foreground window area
			// Bit 6-7 - Unknown
			if (ACCESSING_BITS_0_7)
			{
				m_layer_bg_enable = BIT(data, 0);
				m_layer_fg_enable = BIT(data, 1);
				m_sprites_enable = BIT(data, 2);
				m_window_sprites_enable = BIT(data, 3);
				m_window_fg_mode = (data & 0x30) >> 4;
			}
			// Background colour
			// In 16 colour mode:
			//   Bit 0-3 - Palette index
			//   Bit 4-7 - Palette number
			// Otherwise:
			//   Bit 0-2 - Main palette index
			//   Bit 3-7 - Unknown
			if (ACCESSING_BITS_8_15)
				m_bg_control = data >> 8;
			break;
		case 0x02 / 2:
			// Current scanline (read-only)
			if (ACCESSING_BITS_0_7)
				LOGSCANLINE("Write to current scanline! Current value: %d  Data to write: %d\n", m_current_line, data);
			// Line compare
			if (ACCESSING_BITS_8_15)
				m_line_compare = data >> 8;
			break;
		case 0x04 / 2:
			// Sprite table base address
			// Bit 0-5 - Determine sprite table base address 0 0xxxxxx0 00000000
			// Bit 6-7 - Unknown
			if (ACCESSING_BITS_0_7)
				m_sprite_table_address = (data & (is_color_vdp() ? 0x3f : 0x1f)) << 8;
			// First sprite number (the one we start drawing with)
			if (ACCESSING_BITS_8_15)
				m_sprite_first_latch = data >> 8;
			break;
		case 0x06 / 2:
			// Number of sprites to draw
			if (ACCESSING_BITS_0_7)
				m_sprite_count_latch = data & 0xff;
			// Background/Foreground table base addresses
			// Bit 8-11  - Determine background table base address 0xxxx000 00000000 (in bytes)
			// Bit 12-15 - Determine foreground table base address 0xxxx000 00000000 (in bytes)
			if (ACCESSING_BITS_8_15)
			{
				m_layer_bg_address = (data & (is_color_vdp() ? 0x0f00 : 0x0700)) << 2;
				m_layer_fg_address = (data & (is_color_vdp() ? 0xf000 : 0x7000)) >> 2;
			}
			break;
		case 0x08 / 2:
			// Left coordinate of foreground window
			if (ACCESSING_BITS_0_7)
				m_window_fg_left = data & 0xff;
			// Top coordinate of foreground window
			if (ACCESSING_BITS_8_15)
				m_window_fg_top = data >> 8;
			break;
		case 0x0a / 2:
			// Right coordinate of foreground window
			if (ACCESSING_BITS_0_7)
				m_window_fg_right = data & 0xff;
			// Bottom coordinate of foreground window
			if (ACCESSING_BITS_8_15)
				m_window_fg_bottom = data >> 8;
			break;
		case 0x0c / 2:
			// Left coordinate of sprite window
			if (ACCESSING_BITS_0_7)
				m_window_sprites_left = data & 0xff;
			// Top coordinate of sprite window
			if (ACCESSING_BITS_8_15)
				m_window_sprites_top = data >> 8;
			break;
		case 0x0e / 2:
			// Right coordinate of sprite window
			if (ACCESSING_BITS_0_7)
				m_window_sprites_right = data & 0xff;
			// Bottom coordinate of sprite window
			if (ACCESSING_BITS_8_15)
				m_window_sprites_bottom = data >> 8;
			break;
		case 0x10 / 2:
			// Background layer X scroll
			if (ACCESSING_BITS_0_7)
				m_layer_bg_scroll_x = data & 0xff;
			// Background layer Y scroll
			if (ACCESSING_BITS_8_15)
				m_layer_bg_scroll_y = data >> 8;
			break;
		case 0x12 / 2:
			// Foreground layer X scroll
			if (ACCESSING_BITS_0_7)
				m_layer_fg_scroll_x = data & 0xff;
			// Foreground layer Y scroll
			if (ACCESSING_BITS_8_15)
				m_layer_fg_scroll_y = data >> 8;
			break;
		case 0x14 / 2:
			// LCD control
			// Bit 0   - LCD enable
			// Bit 1   - WSC only, brightness low/high
			// Bit 2-7 - Unknown
			if (ACCESSING_BITS_0_7)
				m_lcd_control = data & 0xff;
			// LCD icons
			// Bit 8     - LCD sleep icon enable
			// Bit 9     - Vertical position icon enable
			// Bit 10    - Horizontal position icon enable
			// Bit 11    - Dot 1 icon enable
			// Bit 12    - Dot 2 icon enable
			// Bit 13    - Dot 3 icon enable
			// Bit 14-15 - Unknown
			if (ACCESSING_BITS_8_15)
				m_icons_cb(data >> 8);
			break;
		case 0x1c / 2:
			// Palette colors 0 and 1
			// Bit 0-3 - Gray tone setting for main palette index 0
			// Bit 4-7 - Gray tone setting for main palette index 1
			if (ACCESSING_BITS_0_7)
			{
				int i = pal4bit(15 - (data & 0x0f));
				int j = pal4bit(15 - ((data >> 4) & 0x0f));
				set_indirect_color(0, rgb_t(i, i, i));
				set_indirect_color(1, rgb_t(j, j, j));
			}
			// Palette colors 2 and 3
			// Bit  8-11 - Gray tone setting for main palette index 2
			// Bit 12-15 - Gray tone setting for main palette index 3
			if (ACCESSING_BITS_8_15)
			{
				int i = pal4bit(15 - ((data >> 8) & 0x0f));
				int j = pal4bit(15 - ((data >> 12) & 0x0f));
				set_indirect_color(2, rgb_t(i, i, i));
				set_indirect_color(3, rgb_t(j, j, j));
			}
			break;
		case 0x1e / 2:
			// Palette colors 4 and 5
			// Bit 0-3 - Gray tone setting for main palette index 4
			// Bit 4-7 - Gray tone setting for main palette index 5
			if (ACCESSING_BITS_0_7)
			{
				int i = pal4bit(15 - (data & 0x0f));
				int j = pal4bit(15 - ((data >> 4) & 0x0f));
				set_indirect_color(4, rgb_t(i, i, i));
				set_indirect_color(5, rgb_t(j, j, j));
			}
			// Palette colors 6 and 7
			// Bit 0-3 - Gray tone setting for main palette index 6
			// Bit 4-7 - Gray tone setting for main palette index 7
			if (ACCESSING_BITS_8_15)
			{
				int i = pal4bit(15 - ((data >> 8) & 0x0f));
				int j = pal4bit(15 - ((data >> 12) & 0x0f));
				set_indirect_color(6, rgb_t(i, i, i));
				set_indirect_color(7, rgb_t(j, j, j));
			}
			break;
		case 0x20 / 2: case 0x22 / 2: case 0x24 / 2: case 0x26 / 2:
		case 0x28 / 2: case 0x2a / 2: case 0x2c / 2: case 0x2e / 2:
		case 0x30 / 2: case 0x32 / 2: case 0x34 / 2: case 0x36 / 2:
		case 0x38 / 2: case 0x3a / 2: case 0x3c / 2: case 0x3e / 2:
			// 0x20-0x3f tile/sprite palette settings
			//   Bit  0- 3 - Palette (offs & 0x1f)/2 index 0
			//   Bit  4- 7 - Palette (offs & 0x1f)/2 index 1
			//   Bit  8-11 - Palette (offs & 0x1f)/2 index 2
			//   Bit 12-15 - Palette (offs & 0x1f)/2 index 3
			data &= 0x7777;
			COMBINE_DATA(&m_palette_port[offset & 0x0f]);
			break;
		case 0x60 / 2:
			// Video mode
			// Bit 0-4 - Unknown
			// Bit 5   - Packed mode 0 = not packed mode, 1 = packed mode
			// Bit 6   - 4/16 colour mode select: 0 = 4 colour mode, 1 = 16 colour mode
			// Bit 7   - monochrome/colour mode select: 0 = monochrome mode, 1 = colour mode
			//    111  - packed, 16 color, use 4000/8000, color
			//    110  - not packed, 16 color, use 4000/8000, color
			//    101  - packed, 4 color, use 2000, color
			//    100  - not packed, 4 color, use 2000, color
			//    011  - packed, 16 color, use 4000/8000, monochrome
			//    010  - not packed, 16 color , use 4000/8000, monochrome
			//    001  - packed, 4 color, use 2000, monochrome
			//    000  - not packed, 4 color, use 2000, monochrome - Regular WS monochrome
			if (is_color_vdp() && ACCESSING_BITS_0_7)
			{
				m_color_mode = BIT(data, 7);
				m_colors_16 = BIT(data, 6);
				m_tile_packed = BIT(data, 5);
				m_color_mode_cb(m_color_mode);
			}
			break;
		case 0xa2 / 2:
			// Timer control
			// Bit 0   - HBlank Timer enable
			// Bit 1   - HBlank Timer mode: 0 = one shot, 1 = auto reset
			// Bit 2   - VBlank Timer(1/75s) enable
			// Bit 3   - VBlank Timer mode: 0 = one shot, 1 = auto reset
			// Bit 4-7 - Unknown
			if (ACCESSING_BITS_0_7)
			{
				m_timer_hblank_enable = BIT(data, 0);
				m_timer_hblank_mode =   BIT(data, 1);
				m_timer_vblank_enable = BIT(data, 2);
				m_timer_vblank_mode =   BIT(data, 3);
			}
			break;
		case 0xa4 / 2:  // HBlank timer frequency reload value
			COMBINE_DATA(&m_timer_hblank_reload);
			m_timer_hblank_count = m_timer_hblank_reload;
			break;
		case 0xa6 / 2:  // VBlank timer frequency reload value
			COMBINE_DATA(&m_timer_vblank_reload);
			m_timer_vblank_count = m_timer_vblank_reload;
			break;
	}

	COMBINE_DATA(&m_regs[offset & 0x7f]);
}


TIMER_CALLBACK_MEMBER(wswan_video_device::scanline_interrupt)
{
	if (m_current_line < 144)
		refresh_scanline();

	// Decrement 12kHz (HBlank) counter
	if (m_timer_hblank_enable && m_timer_hblank_reload != 0)
	{
		m_timer_hblank_count--;
		if (m_timer_hblank_count == 0)
		{
			if (m_timer_hblank_mode)
				m_timer_hblank_count = m_timer_hblank_reload;
			else
				m_timer_hblank_reload = 0;

			m_set_irq_cb(WSWAN_VIDEO_IFLAG_HBLTMR);
		}
	}

	if (m_current_line == 144) // buffer sprite table
	{
		memcpy(m_sprite_table_buffer, &m_vram[m_sprite_table_address & (m_color_mode ? 0x3fff : 0x1fff)], 512);
		m_sprite_first = m_sprite_first_latch;
		m_sprite_count = m_sprite_count_latch;
	}

	if (m_current_line == 144)
	{
		m_set_irq_cb(WSWAN_VIDEO_IFLAG_VBL);
		/* Decrement 75Hz (VBlank) counter */
		if (m_timer_vblank_enable && m_timer_vblank_reload != 0)
		{
			m_timer_vblank_count--;
			if (m_timer_vblank_count == 0)
			{
				if (m_timer_vblank_mode)
					m_timer_vblank_count = m_timer_vblank_reload;
				else
					m_timer_vblank_reload = 0;

				m_set_irq_cb(WSWAN_VIDEO_IFLAG_VBLTMR);
			}
		}
	}

	if (m_current_line == m_line_compare)
		m_set_irq_cb(WSWAN_VIDEO_IFLAG_LCMP);

	m_current_line = (m_current_line + 1) % 159;
}


u16 wswan_video_device::vram_r(offs_t offset, u16 mem_mask)
{
	return ((!m_color_mode) && (offset >= (0x4000 >> 1))) ? 0x9090 : m_vram[offset];
}


void wswan_video_device::vram_w(offs_t offset, u16 data, u16 mem_mask)
{
	if ((!m_color_mode) && (offset >= (0x4000 >> 1)))
		return;

	COMBINE_DATA(&m_vram[offset]);
}
