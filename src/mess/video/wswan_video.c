// license:BSD-3-Clause
// copyright-holders:Anthony Kruize, Wilbert Pol
/***************************************************************************

 wswan_video.c

 File to handle video emulation of the Bandai WonderSwan VDP.

 Anthony Kruize
 Wilbert Pol

 TODO:
   - remove the redundant parts of m_regs
   - split the Color VDP from the Mono VDP?

 ***************************************************************************/

#include "wswan_video.h"

const device_type WSWAN_VIDEO = &device_creator<wswan_video_device>;


wswan_video_device::wswan_video_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, WSWAN_VIDEO, "Bandai WonderSwan VDP", tag, owner, clock, "wswan_video", __FILE__),
					m_vdp_type(VDP_TYPE_WSWAN)
{
}


void wswan_video_device::common_save()
{
	save_item(NAME(m_bitmap));
	save_item(NAME(m_vram));
	save_item(NAME(m_palette_port));
	save_item(NAME(m_pal));
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
	save_item(NAME(m_icons));
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
	save_item(NAME(m_main_palette));
}

void wswan_video_device::device_start()
{
	machine().first_screen()->register_screen_bitmap(m_bitmap);

	m_timer = timer_alloc(TIMER_SCANLINE);
	m_timer->adjust(attotime::from_ticks(256, 3072000), 0, attotime::from_ticks(256, 3072000));

	// bind callbacks
	m_set_irq_cb.bind_relative_to(*owner());
	m_snd_dma_cb.bind_relative_to(*owner());

	if (m_vdp_type == VDP_TYPE_WSC)
	{
		m_vram.resize(0x10000);
		memset(&m_vram[0], 0, 0x10000);
		m_palette_vram = &m_vram[0xfe00];
	}
	else
	{
		m_vram.resize(0x4000);
		memset(&m_vram[0], 0, 0x4000);
		m_palette_vram = &m_vram[0];
	}

	common_save();
}

// This is a copy of ws_portram_init
// TODO: remove unneeded parts!
static const UINT8 vdp_regs_init[256] =
{
	0x00, 0x00, 0x00/*?*/, 0xbb, 0x00, 0x00, 0x00, 0x26, 0xfe, 0xde, 0xf9, 0xfb, 0xdb, 0xd7, 0x7f, 0xf5,
	0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x9e, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x99, 0xfd, 0xb7, 0xdf,
	0x30, 0x57, 0x75, 0x76, 0x15, 0x73, 0x70/*77?*/, 0x77, 0x20, 0x75, 0x50, 0x36, 0x70, 0x67, 0x50, 0x77,
	0x57, 0x54, 0x75, 0x77, 0x75, 0x17, 0x37, 0x73, 0x50, 0x57, 0x60, 0x77, 0x70, 0x77, 0x10, 0x73,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00,
	0x87, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x4f, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xdb, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x42, 0x00, 0x83, 0x00,
	0x2f, 0x3f, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1,
	0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1,
	0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1,
	0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1
};


void wswan_video_device::device_reset()
{
	m_layer_bg_enable = 0;
	m_layer_fg_enable = 0;
	m_sprites_enable = 0;
	m_window_sprites_enable = 0;
	m_window_fg_mode = 0;
	m_bg_control = 0;
	m_current_line = 145;  // Randomly chosen, beginning of VBlank period to give cart some time to boot up
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
	m_lcd_control = 0x01;
	m_icons = 0;
	m_color_mode = 0;
	m_colors_16 = 0;
	m_tile_packed = 0;
	m_timer_hblank_enable = 0;
	m_timer_hblank_mode = 0;
	m_timer_hblank_reload = 0;
	m_timer_hblank_count = 0;
	m_timer_vblank_enable = 0;
	m_timer_vblank_mode = 0;
	m_timer_vblank_reload = 0;
	m_timer_vblank_count = 0;      /* Vertical blank timer counter value */

	memset(m_sprite_table_buffer, 0, sizeof(m_sprite_table_buffer));
	memset(m_main_palette, 0, sizeof(m_main_palette));
	memcpy(m_regs, vdp_regs_init, 256);
	for (int i = 0; i < 0x20; i++)
		m_palette_port[i] = m_regs[i + 0x20];

	setup_palettes();
}


void wswan_video_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_SCANLINE:
			scanline_interrupt();
			break;
	}
}


void wswan_video_device::setup_palettes()
{
	if (m_color_mode)
	{
		for (int i = 0; i < 16; i++)
			for (int j = 0; j < 16; j++)
				m_pal[i][j] = ((m_palette_vram[(i << 5) + j * 2 + 1] << 8) | m_palette_vram[(i << 5) + j * 2]) & 0x0fff;
	}
	else
	{
		for (int  i = 0; i < 16; i++)
		{
			m_pal[i][0] = (m_palette_port[(i << 1)] >> 0) & 0x07;
			m_pal[i][1] = (m_palette_port[(i << 1)] >> 4) & 0x07;
			m_pal[i][2] = (m_palette_port[(i << 1) + 1] >> 0) & 0x07;
			m_pal[i][3] = (m_palette_port[(i << 1) + 1] >> 4) & 0x07;
		}
	}
}

void wswan_video_device::draw_background()
{
	UINT16 map_addr = m_layer_bg_address + (((m_current_line + m_layer_bg_scroll_y) & 0xf8) << 3);
	UINT8 start_column = (m_layer_bg_scroll_x >> 3);

	for (int column = 0; column < 29; column++)
	{
		UINT32 plane0 = 0, plane1 = 0, plane2 = 0, plane3 = 0;
		int x_offset, tile_line, tile_address;
		int tile_data =  (m_vram[map_addr + (((start_column + column) & 0x1f) << 1) + 1] << 8)
						| m_vram[map_addr + (((start_column + column) & 0x1f) << 1)];
		int tile_number = tile_data & 0x01ff;
		int tile_palette = (tile_data >> 9) & 0x0f;

		tile_line = (m_current_line + m_layer_bg_scroll_y) & 0x07;
		if (tile_data & 0x8000) // vflip
			tile_line = 7 - tile_line;

		if (m_colors_16)
		{
			tile_address = ((tile_data & 0x2000) ? 0x8000 : 0x4000) + (tile_number * 32) + (tile_line << 2);
			if (m_tile_packed)
			{
				plane0 = (m_vram[tile_address + 0] << 24) | (m_vram[tile_address + 1] << 16) | (m_vram[tile_address + 2] << 8) | m_vram[tile_address + 3];
			}
			else
			{
				plane0 = m_vram[tile_address + 0];
				plane1 = m_vram[tile_address + 1] << 1;
				plane2 = m_vram[tile_address + 2] << 2;
				plane3 = m_vram[tile_address + 3] << 3;
			}
		}
		else
		{
			tile_address = 0x2000 + (tile_number * 16) + (tile_line << 1);
			if (m_tile_packed)
			{
				plane0 = (m_vram[tile_address + 0] << 8) | m_vram[tile_address + 1];
			}
			else
			{
				plane0 = m_vram[tile_address + 0];
				plane1 = m_vram[tile_address + 1] << 1;
				plane2 = 0;
				plane3 = 0;
			}
		}

		for (int x = 0; x < 8; x++)
		{
			int col;
			if (m_tile_packed)
			{
				if (m_colors_16)
				{
					col = plane0 & 0x0f;
					plane0 = plane0 >> 4;
				}
				else
				{
					col = plane0 & 0x03;
					plane0 = plane0 >> 2;
				}
			}
			else
			{
				col = (plane3 & 8) | (plane2 & 4) | (plane1 & 2) | (plane0 & 1);
				plane3 = plane3 >> 1;
				plane2 = plane2 >> 1;
				plane1 = plane1 >> 1;
				plane0 = plane0 >> 1;
			}

			if (tile_data & 0x4000)
				x_offset = x + (column << 3) - (m_layer_bg_scroll_x & 0x07);
			else
				x_offset = 7 - x + (column << 3) - (m_layer_bg_scroll_x & 0x07);

			if (x_offset >= 0 && x_offset < WSWAN_X_PIXELS)
			{
				if (m_colors_16)
				{
					if (col)
					{
						if (m_color_mode)
							m_bitmap.pix16(m_current_line, x_offset) = m_pal[tile_palette][col];
						else
						{
							/* Hmmmm, what should we do here... Is this correct?? */
							m_bitmap.pix16(m_current_line, x_offset) = m_pal[tile_palette][col];
						}
					}
				}
				else
				{
					if (col || !(tile_palette & 4))
					{
						if (m_color_mode)
							m_bitmap.pix16(m_current_line, x_offset) = m_pal[tile_palette][col];
						else
							m_bitmap.pix16(m_current_line, x_offset) = m_main_palette[m_pal[tile_palette][col]];
					}
				}
			}
		}
	}
}

void wswan_video_device::draw_foreground_0()
{
	UINT16 map_addr = m_layer_fg_address + (((m_current_line + m_layer_fg_scroll_y) & 0xf8) << 3);
	UINT8 start_column = (m_layer_fg_scroll_x >> 3);

	for (int column = 0; column < 29; column++)
	{
		UINT32 plane0 = 0, plane1 = 0, plane2 = 0, plane3 = 0;
		int x_offset, tile_line, tile_address;
		int tile_data =  (m_vram[map_addr + (((start_column + column) & 0x1f) << 1) + 1] << 8)
						| m_vram[map_addr + (((start_column + column) & 0x1f) << 1)];
		int tile_number = tile_data & 0x01ff;
		int tile_palette = (tile_data >> 9) & 0x0f;

		tile_line = (m_current_line + m_layer_fg_scroll_y) & 0x07;
		if (tile_data & 0x8000) // vflip
			tile_line = 7 - tile_line;

		if (m_colors_16)
		{
			tile_address = ((tile_data & 0x2000) ? 0x8000 : 0x4000) + (tile_number * 32) + (tile_line << 2);
			if (m_tile_packed)
			{
				plane0 = (m_vram[tile_address + 0] << 24) | (m_vram[tile_address + 1] << 16) | (m_vram[tile_address + 2] << 8) | m_vram[tile_address + 3];
			}
			else
			{
				plane0 = m_vram[tile_address + 0];
				plane1 = m_vram[tile_address + 1] << 1;
				plane2 = m_vram[tile_address + 2] << 2;
				plane3 = m_vram[tile_address + 3] << 3;
			}
		}
		else
		{
			tile_address = 0x2000 + (tile_number * 16) + (tile_line << 1);
			if (m_tile_packed)
			{
				plane0 = (m_vram[tile_address + 0] << 8) | m_vram[tile_address + 1];
			}
			else
			{
				plane0 = m_vram[tile_address + 0];
				plane1 = m_vram[tile_address + 1] << 1;
				plane2 = 0;
				plane3 = 0;
			}
		}

		for (int x = 0; x < 8; x++ )
		{
			int col;
			if (m_tile_packed)
			{
				if (m_colors_16)
				{
					col = plane0 & 0x0f;
					plane0 = plane0 >> 4;
				}
				else
				{
					col = plane0 & 0x03;
					plane0 = plane0 >> 2;
				}
			}
			else
			{
				col = (plane3 & 8) | (plane2 & 4) | (plane1 & 2) | (plane0 & 1);
				plane3 = plane3 >> 1;
				plane2 = plane2 >> 1;
				plane1 = plane1 >> 1;
				plane0 = plane0 >> 1;
			}

			if (tile_data & 0x4000)
				x_offset = x + (column << 3) - (m_layer_fg_scroll_x & 0x07);
			else
				x_offset = 7 - x + (column << 3) - (m_layer_fg_scroll_x & 0x07);

			if (x_offset >= 0 && x_offset < WSWAN_X_PIXELS)
			{
				if (m_colors_16)
				{
					if (col)
					{
//                      if (m_color_mode) {
						m_bitmap.pix16(m_current_line, x_offset) = m_pal[tile_palette][col];
//                      } else {
//                          /* Hmmmm, what should we do here... Is this correct?? */
//                          m_bitmap.pix16(m_current_line, x_offset) = m_pal[tile_palette][col];
//                      }
					}
				}
				else
				{
					if (col || !(tile_palette & 4))
					{
						if (m_color_mode)
							m_bitmap.pix16(m_current_line, x_offset) = m_pal[tile_palette][col];
						else
							m_bitmap.pix16(m_current_line, x_offset) = m_main_palette[m_pal[tile_palette][col]];
					}
				}
			}
		}
	}
}

void wswan_video_device::draw_foreground_2()
{
	UINT16 map_addr = m_layer_fg_address + (((m_current_line + m_layer_fg_scroll_y) & 0xf8) << 3);
	UINT8 start_column = (m_layer_fg_scroll_x >> 3);

	for (int column = 0; column < 29; column++)
	{
		UINT32 plane0 = 0, plane1 = 0, plane2 = 0, plane3 = 0;
		int x_offset, tile_line, tile_address;
		int tile_data =  (m_vram[map_addr + (((start_column + column) & 0x1f) << 1) + 1] << 8)
						| m_vram[map_addr + (((start_column + column) & 0x1f) << 1)];
		int tile_number = tile_data & 0x01ff;
		int tile_palette = (tile_data >> 9) & 0x0f;

		tile_line = (m_current_line + m_layer_fg_scroll_y) & 0x07;
		if (tile_data & 0x8000) // vflip
			tile_line = 7 - tile_line;


		if (m_colors_16)
		{
			tile_address = ((tile_data & 0x2000) ? 0x8000 : 0x4000) + (tile_number * 32) + (tile_line << 2);
			if (m_tile_packed)
			{
				plane0 = (m_vram[tile_address + 0] << 24) | (m_vram[tile_address + 1] << 16) | (m_vram[tile_address + 2] << 8) | m_vram[tile_address + 3];
			}
			else
			{
				plane0 = m_vram[tile_address + 0];
				plane1 = m_vram[tile_address + 1] << 1;
				plane2 = m_vram[tile_address + 2] << 2;
				plane3 = m_vram[tile_address + 3] << 3;
			}
		}
		else
		{
			tile_address = 0x2000 + (tile_number * 16) + (tile_line << 1);
			if (m_tile_packed)
			{
				plane0 = (m_vram[tile_address + 0] << 8) | m_vram[tile_address + 1];
			}
			else
			{
				plane0 = m_vram[tile_address + 0];
				plane1 = m_vram[tile_address + 1] << 1;
				plane2 = 0;
				plane3 = 0;
			}
		}

		for (int x = 0; x < 8; x++)
		{
			int col;
			if (m_tile_packed)
			{
				if (m_colors_16)
				{
					col = plane0 & 0x0f;
					plane0 = plane0 >> 4;
				}
				else
				{
					col = plane0 & 0x03;
					plane0 = plane0 >> 2;
				}
			}
			else
			{
				col = (plane3 & 8) | (plane2 & 4) | (plane1 & 2) | (plane0 & 1);
				plane3 = plane3 >> 1;
				plane2 = plane2 >> 1;
				plane1 = plane1 >> 1;
				plane0 = plane0 >> 1;
			}

			if (tile_data & 0x4000)
				x_offset = x + (column << 3) - (m_layer_fg_scroll_x & 0x07);
			else
				x_offset = 7 - x + (column << 3) - (m_layer_fg_scroll_x & 0x07);

			if (x_offset >= 0 && x_offset >= m_window_fg_left && x_offset < m_window_fg_right && x_offset < WSWAN_X_PIXELS)
			{
				if (m_colors_16)
				{
					if (col)
					{
						if (m_color_mode)
							m_bitmap.pix16(m_current_line, x_offset) = m_pal[tile_palette][col];
						else
							/* Hmmmm, what should we do here... Is this correct?? */
							m_bitmap.pix16(m_current_line, x_offset) = m_pal[tile_palette][col];
					}
				}
				else
				{
					if (col || !(tile_palette & 4))
					{
						if (m_color_mode)
							m_bitmap.pix16(m_current_line, x_offset) = m_pal[tile_palette][col];
						else
							m_bitmap.pix16(m_current_line, x_offset) = m_main_palette[m_pal[tile_palette][col]];
					}
				}
			}
		}
	}
}

void wswan_video_device::draw_foreground_3()
{
	UINT16 map_addr = m_layer_fg_address + (((m_current_line + m_layer_fg_scroll_y) & 0xf8) << 3);
	UINT8 start_column = (m_layer_fg_scroll_x >> 3);

	for (int column = 0; column < 29; column++)
	{
		UINT32 plane0 = 0, plane1 = 0, plane2 = 0, plane3 = 0;
		int x_offset, tile_line, tile_address;
		int tile_data =  (m_vram[map_addr + (((start_column + column) & 0x1f) << 1) + 1] << 8)
						| m_vram[map_addr + (((start_column + column) & 0x1f) << 1)];
		int tile_number = tile_data & 0x01ff;
		int tile_palette = (tile_data >> 9) & 0x0f;

		tile_line = (m_current_line + m_layer_fg_scroll_y) & 0x07;
		if (tile_data & 0x8000) // vflip
			tile_line = 7 - tile_line;

		if (m_colors_16)
		{
			tile_address = ((tile_data & 0x2000) ? 0x8000 : 0x4000) + (tile_number * 32) + (tile_line << 2);
			if (m_tile_packed)
			{
				plane0 = (m_vram[tile_address + 0] << 24) | (m_vram[tile_address + 1] << 16) | (m_vram[tile_address + 2] << 8) | m_vram[tile_address + 3];
			}
			else
			{
				plane0 = m_vram[tile_address + 0];
				plane1 = m_vram[tile_address + 1] << 1;
				plane2 = m_vram[tile_address + 2] << 2;
				plane3 = m_vram[tile_address + 3] << 3;
			}
		}
		else
		{
			tile_address = 0x2000 + (tile_number * 16) + (tile_line << 1);
			if (m_tile_packed)
			{
				plane0 = (m_vram[tile_address + 0] << 8) | m_vram[tile_address + 1];
			}
			else
			{
				plane0 = m_vram[tile_address + 0];
				plane1 = m_vram[tile_address + 1] << 1;
				plane2 = 0;
				plane3 = 0;
			}
		}

		for (int x = 0; x < 8; x++)
		{
			int col;
			if (m_tile_packed)
			{
				if (m_colors_16)
				{
					col = plane0 & 0x0f;
					plane0 = plane0 >> 4;
				}
				else
				{
					col = plane0 & 0x03;
					plane0 = plane0 >> 2;
				}
			}
			else
			{
				col = (plane3 & 8) | (plane2 & 4) | (plane1 & 2) | (plane0 & 1);
				plane3 = plane3 >> 1;
				plane2 = plane2 >> 1;
				plane1 = plane1 >> 1;
				plane0 = plane0 >> 1;
			}

			if (tile_data & 0x4000)
				x_offset = x + (column << 3) - (m_layer_fg_scroll_x & 0x07);
			else
				x_offset = 7 - x + (column << 3) - (m_layer_fg_scroll_x & 0x07);

			if ((x_offset >= 0 && x_offset < m_window_fg_left) || (x_offset >= m_window_fg_right && x_offset < WSWAN_X_PIXELS))
			{
				if (m_colors_16)
				{
					if (col)
					{
						if (m_color_mode)
							m_bitmap.pix16(m_current_line, x_offset) = m_pal[tile_palette][col];
						else
							/* Hmmmm, what should we do here... Is this correct?? */
							m_bitmap.pix16(m_current_line, x_offset) = m_pal[tile_palette][col];
					}
				}
				else
				{
					if (col || !(tile_palette & 4))
					{
						if (m_color_mode)
							m_bitmap.pix16(m_current_line, x_offset) = m_pal[tile_palette][col];
						else
							m_bitmap.pix16(m_current_line, x_offset) = m_main_palette[m_pal[tile_palette][col]];
					}
				}
			}
		}
	}
}

void wswan_video_device::handle_sprites(int mask)
{
	if (m_sprite_count == 0)
		return;

	for (int i = m_sprite_first + m_sprite_count - 1; i >= m_sprite_first; i--)
	{
		UINT16 tile_data = (m_sprite_table_buffer[i * 4 + 1] << 8) | m_sprite_table_buffer[i * 4];
		UINT8 y = m_sprite_table_buffer[ i * 4 + 2 ];
		UINT8 x = m_sprite_table_buffer[ i * 4 + 3 ];
		int tile_line = (m_current_line - y) & 0xff;

		if ((tile_line >= 0) && (tile_line < 8) && ((tile_data & 0x2000) == mask))
		{
			UINT32 plane0 = 0, plane1 = 0, plane2 = 0, plane3 = 0;
			int x_offset, tile_address;
			int tile_number = tile_data & 0x01ff;
			int tile_palette = 8 + ((tile_data >> 9) & 0x07);
			int check_clip = 0;

			if (tile_data & 0x8000)
				tile_line = 7 - tile_line;

			if (m_colors_16)
			{
				tile_address = 0x4000 + (tile_number * 32) + (tile_line << 2);
				if (m_tile_packed)
				{
					plane0 = (m_vram[tile_address + 0] << 24) | (m_vram[tile_address + 1] << 16) | (m_vram[tile_address + 2] << 8) | m_vram[tile_address + 3];
				}
				else
				{
					plane0 = m_vram[tile_address + 0];
					plane1 = m_vram[tile_address + 1] << 1;
					plane2 = m_vram[tile_address + 2] << 2;
					plane3 = m_vram[tile_address + 3] << 3;
				}
			}
			else
			{
				tile_address = 0x2000 + (tile_number * 16) + (tile_line << 1);
				if (m_tile_packed)
				{
					plane0 = (m_vram[tile_address + 0] << 8) | m_vram[tile_address + 1];
				}
				else
				{
					plane0 = m_vram[tile_address + 0];
					plane1 = m_vram[tile_address + 1] << 1;
					plane2 = 0;
					plane3 = 0;
				}
			}

			if (m_window_sprites_enable)
			{
				if (tile_data & 0x1000)
				{
					if (m_current_line >= m_window_sprites_top && m_current_line <= m_window_sprites_bottom)
						check_clip = 1;
				}
				else
				{
					if (m_current_line < m_window_sprites_top || m_current_line > m_window_sprites_bottom)
						continue;
				}
			}

			for (int j = 0; j < 8; j++)
			{
				int col;
				if (m_tile_packed)
				{
					if (m_colors_16)
					{
						col = plane0 & 0x0f;
						plane0 = plane0 >> 4;
					}
					else
					{
						col = plane0 & 0x03;
						plane0 = plane0 >> 2;
					}
				}
				else
				{
					col = (plane3 & 8) | (plane2 & 4) | (plane1 & 2) | (plane0 & 1);
					plane3 = plane3 >> 1;
					plane2 = plane2 >> 1;
					plane1 = plane1 >> 1;
					plane0 = plane0 >> 1;
				}

				if (tile_data & 0x4000)
					x_offset = x + j;
				else
					x_offset = x + 7 - j;

				x_offset = x_offset & 0xff;

				if (m_window_sprites_enable)
				{
					if (tile_data & 0x1000 && check_clip)
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
					if (m_colors_16)
					{
						if (col)
						{
							if (m_color_mode)
								m_bitmap.pix16(m_current_line, x_offset) = m_pal[tile_palette][col];
							else
								/* Hmmmm, what should we do here... Is this correct?? */
								m_bitmap.pix16(m_current_line, x_offset) = m_pal[tile_palette][col];
						}
					}
					else
					{
						if (col || !(tile_palette & 4))
						{
							if (m_color_mode)
								m_bitmap.pix16(m_current_line, x_offset) = m_pal[tile_palette][col];
							else
								m_bitmap.pix16(m_current_line, x_offset) = m_main_palette[m_pal[tile_palette][col]];
						}
					}
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
		/* Not sure if these background color checks and settings are correct */
		if (m_color_mode && m_colors_16)
			m_bitmap.fill(m_pal[m_bg_control >> 4][m_bg_control & 0x0f], rec);
		else
			m_bitmap.fill(m_main_palette[m_bg_control & 0x07], rec);
	}
	else
	{
		m_bitmap.fill(0, rec);
		return;
	}

	// Draw background layer
	if (m_layer_bg_enable)
		draw_background();

	// Draw sprites between background and foreground layers
	if (m_sprites_enable)
		handle_sprites(0);

	// Draw foreground layer, taking window settings into account
	if (m_layer_fg_enable)
	{
		switch (m_window_fg_mode)
		{
			case 0: // FG inside & outside window area
				draw_foreground_0();
				break;
			case 1: // ???
				logerror("Unknown foreground mode 1 set\n");
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
		handle_sprites(0x2000);
}



UINT32 wswan_video_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}


READ8_MEMBER(wswan_video_device::reg_r)
{
	UINT8 value = m_regs[offset];

	if (offset >= 0x20 && offset < 0x40)
		return m_palette_port[offset & 0x1f];

	switch (offset)
	{
		case 0x01:
			value = m_bg_control;
			break;
		case 0x02:
			value = m_current_line;
			break;
		case 0x14:
			value = m_lcd_control;
			break;
		case 0xa8:
			value = m_timer_hblank_count & 0xff;
			break;
		case 0xa9:
			value = m_timer_hblank_count >> 8;
			break;
		case 0xaa:
			value = m_timer_vblank_count & 0xff;
			break;
		case 0xab:
			value = m_timer_vblank_count >> 8;
			break;
	}

	return value;
}


WRITE8_MEMBER(wswan_video_device::reg_w)
{
	if (offset >= 0x20 && offset < 0x40)
	{
		// 0x20-0x3f tile/sprite palette settings
		// even offs
		//   Bit 0-3 - Palette (offs & 0x1f)/2 index 0
		//   Bit 4-7 - Palette (offs & 0x1f)/2 index 1
		// odd offs
		//   Bit 0-3 - Palette (offs & 0x1f)/2 index 2
		//   Bit 4-7 - Palette (offs & 0x1f)/2 index 3
		m_palette_port[offset & 0x1f] = data;
		return;
	}

	switch (offset)
	{
		case 0x00:  // Display control
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
			m_layer_bg_enable = data & 0x1;
			m_layer_fg_enable = (data & 0x2) >> 1;
			m_sprites_enable = (data & 0x4) >> 2;
			m_window_sprites_enable = (data & 0x8) >> 3;
			m_window_fg_mode = (data & 0x30) >> 4;
			break;
		case 0x01:  // Background colour
					// In 16 colour mode:
					//   Bit 0-3 - Palette index
					//   Bit 4-7 - Palette number
					// Otherwise:
					//   Bit 0-2 - Main palette index
					//   Bit 3-7 - Unknown
			m_bg_control = data;
			break;
		case 0x02:  // Current scanline (Most likely read-only)
			logerror("Write to current scanline! Current value: %d  Data to write: %d\n", m_current_line, data);
			// Returning so we don't overwrite the value here, not that it really matters
			return;
		case 0x03:  // Line compare
			m_line_compare = data;
			logerror("Write to line compare: %d\n", data);
			break;
		case 0x04:  // Sprite table base address
					// Bit 0-5 - Determine sprite table base address 0 0xxxxxx0 00000000
					// Bit 6-7 - Unknown
			m_sprite_table_address = (data & 0x3f) << 9;
			break;
		case 0x05:  // First sprite number (the one we start drawing with)
			m_sprite_first_latch = data;
			if (data) logerror("non-zero first sprite %d\n", data);
			break;
		case 0x06:  // Number of sprites to draw
			m_sprite_count_latch = data;
			break;
		case 0x07:  // Background/Foreground table base addresses
					// Bit 0-2 - Determine background table base address 00xxx000 00000000
					// Bit 3   - Unknown
					// Bit 4-6 - Determine foreground table base address 00xxx000 00000000
					// Bit 7   - Unknown
			m_layer_bg_address = (data & 0x7) << 11;
			m_layer_fg_address = (data & 0x70) << 7;
			break;
		case 0x08:  // Left coordinate of foreground window
			m_window_fg_left = data;
			break;
		case 0x09:  // Top coordinate of foreground window
			m_window_fg_top = data;
			break;
		case 0x0a:  // Right coordinate of foreground window
			m_window_fg_right = data;
			break;
		case 0x0b:  // Bottom coordinate of foreground window
			m_window_fg_bottom = data;
			break;
		case 0x0c:  // Left coordinate of sprite window
			m_window_sprites_left = data;
			break;
		case 0x0d:  // Top coordinate of sprite window
			m_window_sprites_top = data;
			break;
		case 0x0e:  // Right coordinate of sprite window
			m_window_sprites_right = data;
			break;
		case 0x0f:  // Bottom coordinate of sprite window
			m_window_sprites_bottom = data;
			break;
		case 0x10:  // Background layer X scroll
			m_layer_bg_scroll_x = data;
			break;
		case 0x11:  // Background layer Y scroll
			m_layer_bg_scroll_y = data;
			break;
		case 0x12:  // Foreground layer X scroll
			m_layer_fg_scroll_x = data;
			break;
		case 0x13:  // Foreground layer Y scroll
			m_layer_fg_scroll_y = data;
			break;
		case 0x14:  // LCD control
					// Bit 0   - LCD enable
					// Bit 1-7 - Unknown
			m_lcd_control = data;
			break;
		case 0x15:  // LCD icons
					// Bit 0   - LCD sleep icon enable
					// Bit 1   - Vertical position icon enable
					// Bit 2   - Horizontal position icon enable
					// Bit 3   - Dot 1 icon enable
					// Bit 4   - Dot 2 icon enable
					// Bit 5   - Dot 3 icon enable
					// Bit 6-7 - Unknown
			m_icons = data; /* ummmmm */
			break;
		case 0x1c:  // Palette colors 0 and 1
					// Bit 0-3 - Gray tone setting for main palette index 0
					// Bit 4-7 - Gray tone setting for main palette index 1
			if (m_vdp_type == VDP_TYPE_WSC)
			{
				int i = 15 - (data & 0x0f);
				int j = 15 - ((data & 0xf0) >> 4);
				m_main_palette[0] = (i << 8) | (i << 4) | i;
				m_main_palette[1] = (j << 8) | (j << 4) | j;
			}
			else
			{
				m_main_palette[0] = data & 0x0f;
				m_main_palette[1] = (data & 0xf0) >> 4;
			}
			break;
		case 0x1d:  // Palette colors 2 and 3
					// Bit 0-3 - Gray tone setting for main palette index 2
					// Bit 4-7 - Gray tone setting for main palette index 3
			if (m_vdp_type == VDP_TYPE_WSC)
			{
				int i = 15 - (data & 0x0f);
				int j = 15 - ((data & 0xf0) >> 4);
				m_main_palette[2] = (i << 8) | (i << 4) | i;
				m_main_palette[3] = (j << 8) | (j << 4) | j;
			}
			else
			{
				m_main_palette[2] = data & 0x0f;
				m_main_palette[3] = (data & 0xf0) >> 4;
			}
			break;
		case 0x1e:  // Palette colors 4 and 5
					// Bit 0-3 - Gray tone setting for main palette index 4
					// Bit 4-7 - Gray tone setting for main palette index 5
			if (m_vdp_type == VDP_TYPE_WSC)
			{
				int i = 15 - (data & 0x0f);
				int j = 15 - ((data & 0xf0) >> 4);
				m_main_palette[4] = (i << 8) | (i << 4) | i;
				m_main_palette[5] = (j << 8) | (j << 4) | j;
			}
			else
			{
				m_main_palette[4] = data & 0x0f;
				m_main_palette[5] = (data & 0xf0) >> 4;
			}
			break;
		case 0x1f:  // Palette colors 6 and 7
					// Bit 0-3 - Gray tone setting for main palette index 6
					// Bit 4-7 - Gray tone setting for main palette index 7
			if (m_vdp_type == VDP_TYPE_WSC)
			{
				int i = 15 - (data & 0x0f);
				int j = 15 - ((data & 0xf0) >> 4);
				m_main_palette[6] = (i << 8) | (i << 4) | i;
				m_main_palette[7] = (j << 8) | (j << 4) | j;
			}
			else
			{
				m_main_palette[6] = data & 0x0f;
				m_main_palette[7] = (data & 0xf0) >> 4;
			}
			break;
		case 0x60:  // Video mode
					// Bit 0-4 - Unknown
					// Bit 5   - Packed mode 0 = not packed mode, 1 = packed mode
					// Bit 6   - 4/16 colour mode select: 0 = 4 colour mode, 1 = 16 colour mode
					// Bit 7   - monochrome/colour mode select: 0 = monochrome mode, 1 = colour mode
			/*
			 * 111  - packed, 16 color, use 4000/8000, color
			 * 110  - not packed, 16 color, use 4000/8000, color
			 * 101  - packed, 4 color, use 2000, color
			 * 100  - not packed, 4 color, use 2000, color
			 * 011  - packed, 16 color, use 4000/8000, monochrome
			 * 010  - not packed, 16 color , use 4000/8000, monochrome
			 * 001  - packed, 4 color, use 2000, monochrome
			 * 000  - not packed, 4 color, use 2000, monochrome - Regular WS monochrome
			 */
			if (m_vdp_type == VDP_TYPE_WSC)
			{
				m_color_mode = data & 0x80;
				m_colors_16 = data & 0x40;
				m_tile_packed = data & 0x20;
			}
			break;
		case 0xa2:  // Timer control
					// Bit 0   - HBlank Timer enable
					// Bit 1   - HBlank Timer mode: 0 = one shot, 1 = auto reset
					// Bit 2   - VBlank Timer(1/75s) enable
					// Bit 3   - VBlank Timer mode: 0 = one shot, 1 = auto reset
					// Bit 4-7 - Unknown
			m_timer_hblank_enable = BIT(data, 0);
			m_timer_hblank_mode =   BIT(data, 1);
			m_timer_vblank_enable = BIT(data, 2);
			m_timer_vblank_mode =   BIT(data, 3);
			break;
		case 0xa4:  // HBlank timer frequency reload value (bits 0-7)
			m_timer_hblank_reload &= 0xff00;
			m_timer_hblank_reload += data;
			m_timer_hblank_count = m_timer_hblank_reload;
			break;
		case 0xa5:  // HBlank timer frequency reload value (bits 8-15)
			m_timer_hblank_reload &= 0xff;
			m_timer_hblank_reload += data << 8;
			m_timer_hblank_count = m_timer_hblank_reload;
			break;
		case 0xa6:  // VBlank timer frequency reload value (bits 0-7)
			m_timer_vblank_reload &= 0xff00;
			m_timer_vblank_reload += data;
			m_timer_vblank_count = m_timer_vblank_reload;
			break;
		case 0xa7:  // VBlank timer frequency reload value (bits 8-15)
			m_timer_vblank_reload &= 0xff;
			m_timer_vblank_reload += data << 8;
			m_timer_vblank_count = m_timer_vblank_reload;
			break;
		case 0xa8:  // HBlank counter (bits 0-7)
		case 0xa9:  // HBlank counter (bits 8-15)
		case 0xaa:  // VBlank counter (bits 0-7)
		case 0xab:  // VBlank counter (bits 8-15)
			break;
	}

	m_regs[offset] = data;
}


void wswan_video_device::scanline_interrupt()
{
	if (m_current_line < 144)
		refresh_scanline();

	// Decrement 12kHz (HBlank) counter
	if (m_timer_hblank_enable && m_timer_hblank_reload != 0)
	{
		m_timer_hblank_count--;
		logerror("timer_hblank_count: %X\n", m_timer_hblank_count);
		if (m_timer_hblank_count == 0)
		{
			if (m_timer_hblank_mode)
				m_timer_hblank_count = m_timer_hblank_reload;
			else
				m_timer_hblank_reload = 0;

			logerror( "triggering hbltmr interrupt\n" );
			m_set_irq_cb(WSWAN_VIDEO_IFLAG_HBLTMR);
		}
	}

	// Handle Sound DMA
	m_snd_dma_cb();

//  m_current_line = (m_current_line + 1) % 159;

	if (m_current_line == 144) // buffer sprite table
	{
		memcpy(m_sprite_table_buffer, &m_vram[m_sprite_table_address], 512);
		m_sprite_first = m_sprite_first_latch; // always zero?
		m_sprite_count = m_sprite_count_latch;
	}

	if (m_current_line == 144)
	{
		m_set_irq_cb(WSWAN_VIDEO_IFLAG_VBL);
		/* Decrement 75Hz (VBlank) counter */
		if (m_timer_vblank_enable && m_timer_vblank_reload != 0)
		{
			m_timer_vblank_count--;
			logerror("timer_vblank_count: %X\n", m_timer_vblank_count);
			if (m_timer_vblank_count == 0)
			{
				if (m_timer_vblank_mode)
					m_timer_vblank_count = m_timer_vblank_reload;
				else
					m_timer_vblank_reload = 0;

				logerror("triggering vbltmr interrupt\n");
				m_set_irq_cb(WSWAN_VIDEO_IFLAG_VBLTMR);
			}
		}
	}

//  m_current_line = (m_current_line + 1) % 159;

	if (m_current_line == m_line_compare)
		m_set_irq_cb(WSWAN_VIDEO_IFLAG_LCMP);

	m_current_line = (m_current_line + 1) % 159;
}


READ8_MEMBER(wswan_video_device::vram_r)
{
	return m_vram[offset];
}

WRITE8_MEMBER(wswan_video_device::vram_w)
{
	m_vram[offset] = data;
}
