// license:BSD-3-Clause
// copyright-holders:Anthony Kruize, Wilbert Pol
/**********************************************************************

 wswan.h

 File to handle video emulation of the Bandai WonderSwan.

 Anthony Kruize
 Wilbert Pol

**********************************************************************/

#ifndef MAME_BANDAI_WSWAN_V_H
#define MAME_BANDAI_WSWAN_V_H

#pragma once

class wswan_video_device : public device_t, public device_video_interface, public device_palette_interface
{
public:
	typedef device_delegate<void (int irq)> irq_cb_delegate;

	wswan_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~wswan_video_device();

	// static configuration
	template <typename... T> void set_irq_callback(T &&... args) { m_set_irq_cb.set(std::forward<T>(args)...); }

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	u16 vram_r(offs_t offset, u16 mem_mask);
	void vram_w(offs_t offset, u16 data, u16 mem_mask);
	u16 reg_r(offs_t offset, u16 mem_mesk);
	void reg_w(offs_t offset, u16 data, u16 mem_mask);

	auto icons_cb() { return m_icons_cb.bind(); }
	auto color_mode_cb() { return m_color_mode_cb.bind(); }

	bool color_mode() { return m_color_mode; }

	static const u16 WSWAN_X_PIXELS = (28*8);
	static const u16 WSWAN_Y_PIXELS = (18*8);

protected:
	wswan_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int vdp_type);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual u32 palette_entries() const noexcept override { return 2 + 64; }
	virtual u32 palette_indirect_entries() const noexcept override { return 8 + 1; }

	bool is_color_vdp() { return m_vdp_type == VDP_TYPE_WSC; }

	void init_palettes();
	void setup_palettes();
	void get_tile_data(u16 map_addr, u8 scrolly, u16 &data, u16 &number, u16 &palette, int &line);
	void get_planes(bool bank, u32 number, int line, u32 &plane0, u32 &plane1, u32 &plane2, u32 &plane3);
	u8 extract_planes(u32 &plane0, u32 &plane1, u32 &plane2, u32 &plane3);
	int get_xoffset(bool hflip, int x, int column, u8 scrollx);
	void draw_pixel(int x_offset, u8 tile_palette, u8 pixel);
	void draw_background();
	void draw_foreground_0();
	void draw_foreground_2();
	void draw_foreground_3();
	void handle_sprites(bool mask);
	void refresh_scanline();
	TIMER_CALLBACK_MEMBER(scanline_interrupt);

	bitmap_ind16 m_bitmap;
	bool m_layer_bg_enable;        // Background layer on/off
	bool m_layer_fg_enable;        // Foreground layer on/off
	bool m_sprites_enable;         // Sprites on/off
	bool m_window_sprites_enable;  // Sprite window on/off
	u8 m_window_fg_mode;           // 0:inside/outside, 1:??, 2:inside, 3:outside
	u8 m_bg_control;
	u8 m_current_line;             // Current scanline : 0-158 (159?)
	u8 m_line_compare;             // Line to trigger line interrupt on
	u32 m_sprite_table_address;    // Address of the sprite table
	u16 m_sprite_table_buffer[256];
	u8 m_sprite_first;             // First sprite to draw
	u8 m_sprite_count;             // Number of sprites to draw
	u8 m_sprite_first_latch;
	u8 m_sprite_count_latch;
	u16 m_layer_bg_address;        // Address of the background screen map
	u16 m_layer_fg_address;        // Address of the foreground screen map
	u8 m_window_fg_left;           // Left coordinate of foreground window
	u8 m_window_fg_top;            // Top coordinate of foreground window
	u8 m_window_fg_right;          // Right coordinate of foreground window
	u8 m_window_fg_bottom;         // Bottom coordinate of foreground window
	u8 m_window_sprites_left;      // Left coordinate of sprites window
	u8 m_window_sprites_top;       // Top coordinate of sprites window
	u8 m_window_sprites_right;     // Right coordinate of sprites window
	u8 m_window_sprites_bottom;    // Bottom coordinate of sprites window
	u8 m_layer_bg_scroll_x;        // Background layer X scroll
	u8 m_layer_bg_scroll_y;        // Background layer Y scroll
	u8 m_layer_fg_scroll_x;        // Foreground layer X scroll
	u8 m_layer_fg_scroll_y;        // Foreground layer Y scroll
	u8 m_lcd_control;              // LCD on/off
	bool m_color_mode;             // monochrome/color mode
	bool m_colors_16;              // 4/16 colors mode
	bool m_tile_packed;            // layered/packed tile mode switch
	bool m_timer_hblank_enable;    // Horizontal blank interrupt on/off
	bool m_timer_hblank_mode;      // Horizontal blank timer mode
	u16 m_timer_hblank_reload;     // Horizontal blank timer reload value
	u16 m_timer_hblank_count;      // Horizontal blank timer counter value
	bool m_timer_vblank_enable;    // Vertical blank interrupt on/off
	bool m_timer_vblank_mode;      // Vertical blank timer mode
	u16 m_timer_vblank_reload;     // Vertical blank timer reload value
	u16 m_timer_vblank_count;      // Vertical blank timer counter value
	emu_timer *m_timer;

	std::unique_ptr<u16 []> m_vram;
	u16 m_palette_port[0x10];
	u16 m_regs[128];

	irq_cb_delegate m_set_irq_cb;
	const int m_vdp_type;

	devcb_write8 m_icons_cb;
	devcb_write_line m_color_mode_cb;

	// interrupt flags
	// these are the same as the bandai/wswan.cpp ones
	static constexpr u8 WSWAN_VIDEO_IFLAG_LCMP   = 0x10;
	static constexpr u8 WSWAN_VIDEO_IFLAG_VBLTMR = 0x20;
	static constexpr u8 WSWAN_VIDEO_IFLAG_VBL    = 0x40;
	static constexpr u8 WSWAN_VIDEO_IFLAG_HBLTMR = 0x80;

	static constexpr size_t WSC_VRAM_PALETTE = 0xfe00 >> 1;

	static constexpr u16 BG_COLOR = 64; // background color of wonderswan mono
	static constexpr u16 COLOR_12BIT = 8; // 12 bit color table of wonderswan color

	enum
	{
		VDP_TYPE_WSWAN = 0,
		VDP_TYPE_WSC
	};
};

class wswan_color_video_device : public wswan_video_device
{
public:
	wswan_color_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual u32 palette_entries() const noexcept override { return 2 + 256; }
	virtual u32 palette_indirect_entries() const noexcept override { return 8 + 4096; }
};

DECLARE_DEVICE_TYPE(WSWAN_VIDEO, wswan_video_device)
DECLARE_DEVICE_TYPE(WSWAN_COLOR_VIDEO, wswan_color_video_device)


#endif // MAME_BANDAI_WSWAN_V_H
