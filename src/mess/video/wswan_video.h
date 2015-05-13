// license:BSD-3-Clause
// copyright-holders:Anthony Kruize, Wilbert Pol
/**********************************************************************

 wswan.h

 File to handle video emulation of the Bandai WonderSwan.

 Anthony Kruize
 Wilbert Pol

**********************************************************************/

#ifndef __WSWAN_VIDEO__
#define __WSWAN_VIDEO__

#include "emu.h"

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

#define MCFG_WSWAN_VIDEO_IRQ_CB(_class, _method) \
	wswan_video_device::set_irq_callback(*device, wswan_video_irq_cb_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_WSWAN_VIDEO_DMASND_CB(_class, _method) \
	wswan_video_device::set_dmasnd_callback(*device, wswan_video_dmasnd_cb_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_WSWAN_VIDEO_TYPE( _type) \
	wswan_video_device::set_vdp_type(*device, _type);


class wswan_video_device : public device_t
{
public:
	wswan_video_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~wswan_video_device() {}

	// static configuration
	static void set_irq_callback(device_t &device, wswan_video_irq_cb_delegate callback) { downcast<wswan_video_device &>(device).m_set_irq_cb = callback; }
	static void set_dmasnd_callback(device_t &device, wswan_video_dmasnd_cb_delegate callback) { downcast<wswan_video_device &>(device).m_snd_dma_cb = callback; }
	static void set_vdp_type(device_t &device, int type) { downcast<wswan_video_device &>(device).m_vdp_type = type; }

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	virtual DECLARE_READ8_MEMBER(vram_r);
	virtual DECLARE_WRITE8_MEMBER(vram_w);
	virtual DECLARE_READ8_MEMBER(reg_r);
	virtual DECLARE_WRITE8_MEMBER(reg_w);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	void setup_palettes();
	void draw_background();
	void draw_foreground_0();
	void draw_foreground_2();
	void draw_foreground_3();
	void handle_sprites(int mask);
	void refresh_scanline();
	void scanline_interrupt();
	void common_save();

	bitmap_ind16 m_bitmap;
	UINT8 m_layer_bg_enable;          /* Background layer on/off */
	UINT8 m_layer_fg_enable;          /* Foreground layer on/off */
	UINT8 m_sprites_enable;           /* Sprites on/off */
	UINT8 m_window_sprites_enable;        /* Sprite window on/off */
	UINT8 m_window_fg_mode;           /* 0:inside/outside, 1:??, 2:inside, 3:outside */
	UINT8 m_bg_control;
	UINT8 m_current_line;         /* Current scanline : 0-158 (159?) */
	UINT8 m_line_compare;         /* Line to trigger line interrupt on */
	UINT32 m_sprite_table_address;        /* Address of the sprite table */
	UINT8 m_sprite_table_buffer[512];
	UINT8 m_sprite_first;         /* First sprite to draw */
	UINT8 m_sprite_count;         /* Number of sprites to draw */
	UINT8 m_sprite_first_latch;
	UINT8 m_sprite_count_latch;
	UINT16 m_layer_bg_address;        /* Address of the background screen map */
	UINT16 m_layer_fg_address;        /* Address of the foreground screen map */
	UINT8 m_window_fg_left;           /* Left coordinate of foreground window */
	UINT8 m_window_fg_top;            /* Top coordinate of foreground window */
	UINT8 m_window_fg_right;          /* Right coordinate of foreground window */
	UINT8 m_window_fg_bottom;         /* Bottom coordinate of foreground window */
	UINT8 m_window_sprites_left;      /* Left coordinate of sprites window */
	UINT8 m_window_sprites_top;       /* Top coordinate of sprites window */
	UINT8 m_window_sprites_right;     /* Right coordinate of sprites window */
	UINT8 m_window_sprites_bottom;        /* Bottom coordinate of sprites window */
	UINT8 m_layer_bg_scroll_x;        /* Background layer X scroll */
	UINT8 m_layer_bg_scroll_y;        /* Background layer Y scroll */
	UINT8 m_layer_fg_scroll_x;        /* Foreground layer X scroll */
	UINT8 m_layer_fg_scroll_y;        /* Foreground layer Y scroll */
	UINT8 m_lcd_control;           /* LCD on/off */
	UINT8 m_icons;                /* FIXME: What do we do with these? Maybe artwork? */
	UINT8 m_color_mode;           /* monochrome/color mode */
	UINT8 m_colors_16;            /* 4/16 colors mode */
	UINT8 m_tile_packed;          /* layered/packed tile mode switch */
	UINT8 m_timer_hblank_enable;      /* Horizontal blank interrupt on/off */
	UINT8 m_timer_hblank_mode;        /* Horizontal blank timer mode */
	UINT16 m_timer_hblank_reload;     /* Horizontal blank timer reload value */
	UINT16 m_timer_hblank_count;      /* Horizontal blank timer counter value */
	UINT8 m_timer_vblank_enable;      /* Vertical blank interrupt on/off */
	UINT8 m_timer_vblank_mode;        /* Vertical blank timer mode */
	UINT16 m_timer_vblank_reload;     /* Vertical blank timer reload value */
	UINT16 m_timer_vblank_count;      /* Vertical blank timer counter value */
	int m_main_palette[8];
	emu_timer *m_timer;

	dynamic_buffer m_vram;
	UINT8 *m_palette_vram;
	UINT8 m_palette_port[0x20];
	int m_pal[16][16];
	UINT8 m_regs[256];

	wswan_video_irq_cb_delegate m_set_irq_cb;
	wswan_video_dmasnd_cb_delegate m_snd_dma_cb;
	int m_vdp_type;

	// timer IDs
	static const device_timer_id TIMER_SCANLINE = 0;

	// interrupt flags
	// these are the same as the wswan.h ones
	static const UINT8 WSWAN_VIDEO_IFLAG_LCMP   = 0x10;
	static const UINT8 WSWAN_VIDEO_IFLAG_VBLTMR = 0x20;
	static const UINT8 WSWAN_VIDEO_IFLAG_VBL    = 0x40;
	static const UINT8 WSWAN_VIDEO_IFLAG_HBLTMR = 0x80;
};

extern const device_type WSWAN_VIDEO;


#endif
