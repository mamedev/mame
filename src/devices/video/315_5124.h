// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Enik Land
/*************************************************************************

    sega315_5124.h

    Implementation of Sega VDP chips used in System E, Master System and Game Gear

**************************************************************************/

#ifndef MAME_VIDEO_315_5124_H
#define MAME_VIDEO_315_5124_H

#pragma once

#include "screen.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

DECLARE_DEVICE_TYPE(SEGA315_5124, sega315_5124_device)      /* aka SMS1 vdp */
DECLARE_DEVICE_TYPE(SEGA315_5246, sega315_5246_device)      /* aka SMS2 vdp */
DECLARE_DEVICE_TYPE(SEGA315_5378, sega315_5378_device)      /* aka Gamegear vdp */


class sega315_5124_device : public device_t,
							public device_memory_interface,
							public device_video_interface
{
public:
	static constexpr unsigned WIDTH                      = 342;    /* 342 pixels */
	static constexpr unsigned HEIGHT_NTSC                = 262;    /* 262 lines */
	static constexpr unsigned HEIGHT_PAL                 = 313;    /* 313 lines */
	static constexpr unsigned LBORDER_START              = 9 + 2 + 14 + 8;
	static constexpr unsigned LBORDER_WIDTH              = 13;     /* 13 pixels */
	static constexpr unsigned RBORDER_WIDTH              = 15;     /* 15 pixels */
	static constexpr unsigned TBORDER_START              = 3 + 13;
	static constexpr unsigned NTSC_192_TBORDER_HEIGHT    = 0x1b;   /* 27 lines */
	//static constexpr unsigned NTSC_192_BBORDER_HEIGHT  = 0x18;   /* 24 lines */
	static constexpr unsigned NTSC_224_TBORDER_HEIGHT    = 0x0b;   /* 11 lines */
	//static constexpr unsigned NTSC_224_BBORDER_HEIGHT  = 0x08;   /* 8 lines */
	//static constexpr unsigned PAL_192_TBORDER_HEIGHT   = 0x36;   /* 54 lines */
	//static constexpr unsigned PAL_192_BBORDER_HEIGHT   = 0x30;   /* 48 lines */
	//static constexpr unsigned PAL_224_TBORDER_HEIGHT   = 0x26;   /* 38 lines */
	//static constexpr unsigned PAL_224_BBORDER_HEIGHT   = 0x20;   /* 32 lines */
	static constexpr unsigned PAL_240_TBORDER_HEIGHT     = 0x1e;   /* 30 lines */
	//static constexpr unsigned PAL_240_BBORDER_HEIGHT   = 0x18;   /* 24 lines */


	// construction/destruction
	sega315_5124_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_signal_type(bool is_pal) { m_is_pal = is_pal; }

	template <class Object> devcb_base &set_int_callback(Object &&cb) { return m_int_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_csync_callback(Object &&cb) { return m_csync_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_pause_callback(Object &&cb) { return m_pause_cb.set_callback(std::forward<Object>(cb)); }

	DECLARE_READ8_MEMBER( vram_read );
	DECLARE_WRITE8_MEMBER( vram_write );
	DECLARE_READ8_MEMBER( register_read );
	DECLARE_WRITE8_MEMBER( register_write );
	DECLARE_READ8_MEMBER( vcount_read );
	DECLARE_READ8_MEMBER( hcount_read );

	DECLARE_PALETTE_INIT( sega315_5124 );

	void hcount_latch() { hcount_latch_at_hpos(screen().hpos()); };
	void hcount_latch_at_hpos(int hpos);
	void stop_timers();

	bitmap_rgb32 &get_bitmap() { return m_tmpbitmap; };
	bitmap_ind8 &get_y1_bitmap() { return m_y1_bitmap; };

	/* update the screen */
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	virtual void set_sega315_5124_compatibility_mode(bool sega315_5124_compatibility_mode) { }

	void sega315_5124(address_map &map);
protected:
	static constexpr unsigned SEGA315_5378_CRAM_SIZE        = 0x40; /* 32 colors x 2 bytes per color = 64 bytes */
	static constexpr unsigned SEGA315_5124_CRAM_SIZE        = 0x20; /* 32 colors x 1 bytes per color = 32 bytes */


	sega315_5124_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t cram_size, uint8_t palette_offset, bool supports_224_240);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual space_config_vector memory_space_config() const override;

	void set_display_settings();
	void set_frame_timing();
	virtual void update_palette();
	virtual void cram_write(uint8_t data);
	virtual void draw_scanline(int pixel_offset_x, int pixel_plot_y, int line);
	virtual void blit_scanline(int *line_buffer, int *priority_selected, int pixel_offset_x, int pixel_plot_y, int line);
	virtual uint16_t get_name_table_row(int row);
	void process_line_timer();
	void select_sprites(int line);
	void draw_scanline_mode4(int *line_buffer, int *priority_selected, int line);
	void draw_sprites_mode4(int *line_buffer, int *priority_selected, int line);
	void draw_sprites_tms9918_mode(int *line_buffer, int line);
	void draw_scanline_mode2(int *line_buffer, int line);
	void draw_scanline_mode0(int *line_buffer, int line);
	void check_pending_flags();

	void vdp_postload();

	uint8_t            m_reg[16];                  /* All the registers */
	uint8_t            m_status;                   /* Status register */
	uint8_t            m_pending_status;           /* Pending status flags */
	uint8_t            m_reg8copy;                 /* Internal copy of register 8 (X-Scroll) */
	uint8_t            m_reg9copy;                 /* Internal copy of register 9 (Y-Scroll) */
	uint8_t            m_addrmode;                 /* Type of VDP action */
	uint16_t           m_addr;                     /* Contents of internal VDP address register */
	const uint8_t      m_cram_size;                /* CRAM size */
	uint8_t            m_cram_mask;                /* Mask to switch between SMS and GG CRAM sizes */
	int              m_cram_dirty;               /* Have there been any changes to the CRAM area */
	int              m_pending_reg_write;
	int              m_pending_sprcol_x;
	uint8_t            m_buffer;
	bool             m_sega315_5124_compatibility_mode;    /* when true, GG VDP behaves as SMS VDP */
	int              m_irq_state;                /* The status of the IRQ line of the VDP */
	int              m_vdp_mode;                 /* Current mode of the VDP: 0,1,2,3,4 */
	int              m_y_pixels;                 /* 192, 224, 240 */
	int              m_draw_time;
	uint8_t            m_line_counter;
	uint8_t            m_hcounter;
	uint8_t            m_CRAM[SEGA315_5378_CRAM_SIZE];  /* CRAM */
	const uint8_t      *m_frame_timing;
	bitmap_rgb32     m_tmpbitmap;
	bitmap_ind8      m_y1_bitmap;
	const uint8_t      m_palette_offset;
	const bool       m_supports_224_240;
	bool             m_display_disabled;
	uint16_t           m_sprite_base;
	uint16_t           m_sprite_pattern_line[8];
	int              m_sprite_tile_selected[8];
	int              m_sprite_x[8];
	uint8_t            m_sprite_flags[8];
	int              m_sprite_count;
	int              m_sprite_height;
	int              m_sprite_zoom;
	int              m_current_palette[32];
	bool             m_is_pal;             /* false = NTSC, true = PAL */
	devcb_write_line m_int_cb;       /* Interrupt callback function */
	devcb_write_line m_csync_cb;     /* C-Sync callback function */
	devcb_write_line m_pause_cb;     /* Pause callback function */
	emu_timer        *m_display_timer;
	emu_timer        *m_hint_timer;
	emu_timer        *m_vint_timer;
	emu_timer        *m_nmi_timer;
	emu_timer        *m_draw_timer;
	emu_timer        *m_lborder_timer;
	emu_timer        *m_rborder_timer;
	emu_timer        *m_pending_flags_timer;

	const address_space_config  m_space_config;

	/* Timers */
	static constexpr device_timer_id TIMER_LINE = 0;
	static constexpr device_timer_id TIMER_DRAW = 1;
	static constexpr device_timer_id TIMER_LBORDER = 2;
	static constexpr device_timer_id TIMER_RBORDER = 3;
	static constexpr device_timer_id TIMER_HINT = 4;
	static constexpr device_timer_id TIMER_VINT = 5;
	static constexpr device_timer_id TIMER_NMI = 6;
	static constexpr device_timer_id TIMER_FLAGS = 7;

	required_device<palette_device> m_palette;
};


class sega315_5246_device : public sega315_5124_device
{
public:
	sega315_5246_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual uint16_t get_name_table_row(int row) override;
};


class sega315_5378_device : public sega315_5124_device
{
public:
	sega315_5378_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void set_sega315_5124_compatibility_mode(bool sega315_5124_compatibility_mode) override;

protected:
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual void update_palette() override;
	virtual void cram_write(uint8_t data) override;
	virtual void blit_scanline( int *line_buffer, int *priority_selected, int pixel_offset_x, int pixel_plot_y, int line ) override;
	virtual uint16_t get_name_table_row(int row) override;

private:
	DECLARE_PALETTE_INIT( sega315_5378 );
};


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_SEGA315_5124_SET_SCREEN MCFG_VIDEO_SET_SCREEN

#define MCFG_SEGA315_5124_IS_PAL(_bool) \
	downcast<sega315_5124_device &>(*device).set_signal_type(_bool);

#define MCFG_SEGA315_5124_INT_CB(_devcb) \
	devcb = &downcast<sega315_5124_device &>(*device).set_int_callback(DEVCB_##_devcb);

#define MCFG_SEGA315_5124_CSYNC_CB(_devcb) \
	devcb = &downcast<sega315_5124_device &>(*device).set_csync_callback(DEVCB_##_devcb);

#define MCFG_SEGA315_5124_PAUSE_CB(_devcb) \
	devcb = &downcast<sega315_5124_device &>(*device).set_pause_callback(DEVCB_##_devcb);


#define MCFG_SEGA315_5246_SET_SCREEN MCFG_VIDEO_SET_SCREEN

#define MCFG_SEGA315_5246_IS_PAL(_bool) \
	downcast<sega315_5246_device &>(*device).set_signal_type(_bool);

#define MCFG_SEGA315_5246_INT_CB(_devcb) \
	devcb = &downcast<sega315_5246_device &>(*device).set_int_callback(DEVCB_##_devcb);

#define MCFG_SEGA315_5246_CSYNC_CB(_devcb) \
	devcb = &downcast<sega315_5246_device &>(*device).set_csync_callback(DEVCB_##_devcb);

#define MCFG_SEGA315_5246_PAUSE_CB(_devcb) \
	devcb = &downcast<sega315_5246_device &>(*device).set_pause_callback(DEVCB_##_devcb);


#define MCFG_SEGA315_5378_SET_SCREEN MCFG_VIDEO_SET_SCREEN

#define MCFG_SEGA315_5378_IS_PAL(_bool) \
	downcast<sega315_5378_device &>(*device).set_signal_type(_bool);

#define MCFG_SEGA315_5378_INT_CB(_devcb) \
	devcb = &downcast<sega315_5378_device &>(*device).set_int_callback(DEVCB_##_devcb);

#define MCFG_SEGA315_5378_CSYNC_CB(_devcb) \
	devcb = &downcast<sega315_5378_device &>(*device).set_csync_callback(DEVCB_##_devcb);

#define MCFG_SEGA315_5378_PAUSE_CB(_devcb) \
	devcb = &downcast<sega315_5378_device &>(*device).set_pause_callback(DEVCB_##_devcb);


#endif // MAME_VIDEO_315_5124_H
