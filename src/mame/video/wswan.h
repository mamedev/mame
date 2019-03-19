// license:BSD-3-Clause
// copyright-holders:Anthony Kruize, Wilbert Pol
/**********************************************************************

 wswan.h

 File to handle video emulation of the Bandai WonderSwan.

 Anthony Kruize
 Wilbert Pol

**********************************************************************/

#ifndef MAME_VIDEO_WSWAN_H
#define MAME_VIDEO_WSWAN_H

#pragma once


enum
{
	VDP_TYPE_WSWAN = 0,
	VDP_TYPE_WSC
};

#define WSWAN_X_PIXELS  (28*8)
#define WSWAN_Y_PIXELS  (18*8)



typedef device_delegate<void (int irq)> wswan_video_irq_cb_delegate;
#define WSWAN_VIDEO_IRQ_CB_MEMBER(_name)   void _name(int irq)

typedef device_delegate<void (void)> wswan_video_dmasnd_cb_delegate;
#define WSWAN_VIDEO_DMASND_CB_MEMBER(_name)   void _name(void)

class wswan_video_device : public device_t
{
public:
	wswan_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~wswan_video_device() {}

	// static configuration
	template <typename... T> void set_irq_callback(T &&... args) { m_set_irq_cb = wswan_video_irq_cb_delegate(std::forward<T>(args)...); }
	template <typename... T> void set_dmasnd_callback(T &&... args) { m_snd_dma_cb = wswan_video_dmasnd_cb_delegate(std::forward<T>(args)...); }
	void set_vdp_type(int type) { m_vdp_type = type; }

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint8_t vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);
	uint8_t reg_r(offs_t offset);
	void reg_w(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	void setup_palettes();
	void draw_background();
	void draw_foreground_0();
	void draw_foreground_2();
	void draw_foreground_3();
	void handle_sprites(int mask);
	void refresh_scanline();
	void scanline_interrupt();
	void common_save();

	required_device<screen_device> m_screen;

	bitmap_ind16 m_bitmap;
	uint8_t m_layer_bg_enable;          /* Background layer on/off */
	uint8_t m_layer_fg_enable;          /* Foreground layer on/off */
	uint8_t m_sprites_enable;           /* Sprites on/off */
	uint8_t m_window_sprites_enable;        /* Sprite window on/off */
	uint8_t m_window_fg_mode;           /* 0:inside/outside, 1:??, 2:inside, 3:outside */
	uint8_t m_bg_control;
	uint8_t m_current_line;         /* Current scanline : 0-158 (159?) */
	uint8_t m_line_compare;         /* Line to trigger line interrupt on */
	uint32_t m_sprite_table_address;        /* Address of the sprite table */
	uint8_t m_sprite_table_buffer[512];
	uint8_t m_sprite_first;         /* First sprite to draw */
	uint8_t m_sprite_count;         /* Number of sprites to draw */
	uint8_t m_sprite_first_latch;
	uint8_t m_sprite_count_latch;
	uint16_t m_layer_bg_address;        /* Address of the background screen map */
	uint16_t m_layer_fg_address;        /* Address of the foreground screen map */
	uint8_t m_window_fg_left;           /* Left coordinate of foreground window */
	uint8_t m_window_fg_top;            /* Top coordinate of foreground window */
	uint8_t m_window_fg_right;          /* Right coordinate of foreground window */
	uint8_t m_window_fg_bottom;         /* Bottom coordinate of foreground window */
	uint8_t m_window_sprites_left;      /* Left coordinate of sprites window */
	uint8_t m_window_sprites_top;       /* Top coordinate of sprites window */
	uint8_t m_window_sprites_right;     /* Right coordinate of sprites window */
	uint8_t m_window_sprites_bottom;        /* Bottom coordinate of sprites window */
	uint8_t m_layer_bg_scroll_x;        /* Background layer X scroll */
	uint8_t m_layer_bg_scroll_y;        /* Background layer Y scroll */
	uint8_t m_layer_fg_scroll_x;        /* Foreground layer X scroll */
	uint8_t m_layer_fg_scroll_y;        /* Foreground layer Y scroll */
	uint8_t m_lcd_control;           /* LCD on/off */
	uint8_t m_icons;                /* FIXME: What do we do with these? Maybe artwork? */
	uint8_t m_color_mode;           /* monochrome/color mode */
	uint8_t m_colors_16;            /* 4/16 colors mode */
	uint8_t m_tile_packed;          /* layered/packed tile mode switch */
	uint8_t m_timer_hblank_enable;      /* Horizontal blank interrupt on/off */
	uint8_t m_timer_hblank_mode;        /* Horizontal blank timer mode */
	uint16_t m_timer_hblank_reload;     /* Horizontal blank timer reload value */
	uint16_t m_timer_hblank_count;      /* Horizontal blank timer counter value */
	uint8_t m_timer_vblank_enable;      /* Vertical blank interrupt on/off */
	uint8_t m_timer_vblank_mode;        /* Vertical blank timer mode */
	uint16_t m_timer_vblank_reload;     /* Vertical blank timer reload value */
	uint16_t m_timer_vblank_count;      /* Vertical blank timer counter value */
	int m_main_palette[8];
	emu_timer *m_timer;

	std::vector<uint8_t> m_vram;
	uint8_t *m_palette_vram;
	uint8_t m_palette_port[0x20];
	int m_pal[16][16];
	uint8_t m_regs[256];

	wswan_video_irq_cb_delegate m_set_irq_cb;
	wswan_video_dmasnd_cb_delegate m_snd_dma_cb;
	int m_vdp_type;

	// timer IDs
	static const device_timer_id TIMER_SCANLINE = 0;

	// interrupt flags
	// these are the same as the wswan.h ones
	static const uint8_t WSWAN_VIDEO_IFLAG_LCMP   = 0x10;
	static const uint8_t WSWAN_VIDEO_IFLAG_VBLTMR = 0x20;
	static const uint8_t WSWAN_VIDEO_IFLAG_VBL    = 0x40;
	static const uint8_t WSWAN_VIDEO_IFLAG_HBLTMR = 0x80;
};

DECLARE_DEVICE_TYPE(WSWAN_VIDEO, wswan_video_device)


#endif // MAME_VIDEO_WSWAN_H
