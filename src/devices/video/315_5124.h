// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Enik Land
/*************************************************************************

    sega315_5124.h

    Implementation of Sega VDP chips used in System E, Master System and Game Gear

**************************************************************************/

#ifndef MAME_VIDEO_315_5124_H
#define MAME_VIDEO_315_5124_H

#pragma once

#include "sound/sn76496.h"
#include "emupal.h"
#include "screen.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

DECLARE_DEVICE_TYPE(SEGA315_5124, sega315_5124_device)      /* aka SMS1 vdp */
DECLARE_DEVICE_TYPE(SEGA315_5246, sega315_5246_device)      /* aka SMS2 vdp */
DECLARE_DEVICE_TYPE(SEGA315_5377, sega315_5377_device)      /* aka Gamegear (2 ASIC version) vdp */


class sega315_5124_device : public device_t,
							public device_memory_interface,
							public device_video_interface,
							public device_mixer_interface
{
public:
	static constexpr unsigned WIDTH                      = 342;    /* 342 pixels */
	static constexpr unsigned HEIGHT_NTSC                = 262;    /* 262 lines */
	static constexpr unsigned HEIGHT_PAL                 = 313;    /* 313 lines */
	static constexpr unsigned LBORDER_START              = 26 + 2 + 14 + 8;
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
	sega315_5124_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void set_hcounter_divide(unsigned divide) { m_hcounter_divide = divide; }
	void set_is_pal(bool is_pal) { m_is_pal = is_pal; }

	auto vblank() { return m_vblank_cb.bind(); }
	auto n_csync() { return m_n_csync_cb.bind(); }
	auto n_int() { return m_n_int_cb.bind(); }
	auto n_nmi() { return m_n_nmi_cb.bind(); }

	void psg_w(u8 data) { m_snsnd->write(data); }
	void psg_stereo_w(u8 data) { m_snsnd->stereo_w(data); }
	void n_nmi_in_write(int state) { m_n_nmi_in_state = state; } /* /NMI-IN line input, controls the /NMI state */
	u8 data_read();
	void data_write(u8 data);
	u8 control_read();
	void control_write(u8 data);
	u8 vcount_read();
	u8 hcount_read();

	void hcount_latch();
	bool hcount_latched() { return m_hcounter_latched; }

	bitmap_rgb32 &get_bitmap() { return m_tmpbitmap; }
	bitmap_ind8 &get_y1_bitmap() { return m_y1_bitmap; }

	/* update the screen */
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	virtual void set_sega315_5124_compatibility_mode(bool sega315_5124_compatibility_mode) { }

protected:
	static constexpr unsigned SEGA315_5377_CRAM_SIZE        = 0x40; /* 32 colors x 2 bytes per color = 64 bytes */
	static constexpr unsigned SEGA315_5124_CRAM_SIZE        = 0x20; /* 32 colors x 1 bytes per color = 32 bytes */

	sega315_5124_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 cram_size, u8 palette_offset, u8 reg_num_mask, int max_sprite_zoom_hcount, int max_sprite_zoom_vcount, const u8 *line_timing);

	// device-level overrides
	virtual void device_post_load() override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual space_config_vector memory_space_config() const override;

	void sega315_5124_palette(palette_device &palette) const;

	void sega315_5124(address_map &map);

	virtual int screen_hpos() { return screen().hpos() / ((m_hcounter_divide == 0) ? 1 : m_hcounter_divide); }
	void set_display_settings();
	void set_frame_timing();
	virtual void vblank_end(int vpos);
	virtual void update_palette();
	virtual void write_memory(u8 data);
	virtual void cram_write(u8 data);
	virtual void load_vram_addr(u8 data);
	virtual void select_display_mode();
	virtual void select_extended_res_mode4(bool M1, bool M2, bool M3);
	virtual void select_sprites(int line);
	virtual void sprite_collision(int line, int sprite_col_x);
	virtual void sprite_count_overflow(int line, int sprite_index);
	virtual void draw_scanline(int pixel_offset_x, int pixel_plot_y, int line);
	virtual void blit_scanline(int *line_buffer, int *priority_selected, int pixel_offset_x, int pixel_plot_y, int line);
	virtual void draw_leftmost_pixels_mode4(int *line_buffer, int *priority_selected, int fine_x_scroll, int palette_selected, int tile_line);
	virtual u16 name_row_mode4(u16 row);
	virtual u16 tile1_select_mode4(u16 tile_number);
	virtual u16 tile2_select_mode4(u16 tile_number);
	virtual u8 sprite_attribute_extra_offset_mode4(u8 offset);
	virtual u8 sprite_tile_select_mode4(u8 tile_number);
	void process_line_timer();
	void draw_scanline_mode4(int *line_buffer, int *priority_selected, int line);
	void draw_sprites_mode4(int *line_buffer, int *priority_selected, int line);
	void draw_sprites_tms9918_mode(int *line_buffer, int line);
	void draw_scanline_mode3(int *line_buffer, int line);
	void draw_scanline_mode2(int *line_buffer, int line);
	void draw_scanline_mode1(int *line_buffer, int line);
	void draw_scanline_mode0(int *line_buffer, int line);
	void check_pending_flags();
	u8 vcount();
	u8 hcount();

	unsigned         m_hcounter_divide;
	u8               m_reg[16];                  /* All the registers */
	u8               m_status;                   /* Status register */
	u8               m_pending_status;           /* Pending status flags */
	u8               m_reg8copy;                 /* Internal copy of register 8 (X-Scroll) */
	u8               m_reg9copy;                 /* Internal copy of register 9 (Y-Scroll) */
	u8               m_addrmode;                 /* Type of VDP action */
	u16              m_addr;                     /* Contents of internal VDP address register */
	const u8         m_cram_size;                /* CRAM size */
	u8               m_cram_mask;                /* Mask to switch between SMS and GG CRAM sizes */
	bool             m_cram_dirty;               /* Have there been any changes to the CRAM area */
	bool             m_hcounter_latched;
	bool             m_hint_occurred;
	bool             m_pending_hint;
	bool             m_pending_control_write;
	int              m_pending_sprcol_x;
	u8               m_buffer;
	u8               m_control_write_data_latch;
	bool             m_sega315_5124_compatibility_mode;    /* when true, GG VDP behaves as SMS VDP */
	int              m_n_int_state;                /* The status of the /INT line of the VDP */
	int              m_n_nmi_state;                /* The status of the /NMI line of the VDP */
	int              m_n_nmi_in_state;             /* The status of the /NMI-IN line of the VDP */
	int              m_vdp_mode;                 /* Current mode of the VDP: 0,1,2,3,4 */
	int              m_y_pixels;                 /* 192, 224, 240 */
	int              m_draw_time;
	u8               m_line_counter;
	u8               m_hcounter;
	u8               m_CRAM[SEGA315_5377_CRAM_SIZE];  /* CRAM */
	const u8         *m_frame_timing;
	const u8         *m_line_timing;
	bitmap_rgb32     m_tmpbitmap;
	bitmap_ind8      m_y1_bitmap;
	const u8         m_palette_offset;
	const u8         m_reg_num_mask;
	bool             m_display_disabled;
	u16              m_sprite_attribute_base;
	u16              m_sprite_pattern_line[8];
	int              m_sprite_tile_selected[8];
	int              m_sprite_x[8];
	u8               m_sprite_flags[8];
	int              m_sprite_count;
	int              m_sprite_height;
	int              m_sprite_zoom_scale;
	int              m_max_sprite_zoom_hcount;
	int              m_max_sprite_zoom_vcount;
	int              m_current_palette[32];
	bool             m_is_pal;             /* false = NTSC, true = PAL */
	devcb_write_line m_vblank_cb;      /* VBlank line callback function */
	devcb_write_line m_n_csync_cb;     /* /C-SYNC line callback function */
	devcb_write_line m_n_int_cb;       /* /INT (Interrupt) line callback function */
	devcb_write_line m_n_nmi_cb;       /* /NMI (Non-Maskable Interrupt) line callback function */
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

	required_device<palette_device> m_palette_lut;
	required_device<sn76496_base_device> m_snsnd;
};


class sega315_5246_device : public sega315_5124_device
{
public:
	sega315_5246_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	sega315_5246_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 cram_size, u8 palette_offset, u8 reg_num_mask, int max_sprite_zoom_hcount, int max_sprite_zoom_vcount, const u8 *line_timing);

	virtual void device_add_mconfig(machine_config &config) override;

	virtual u16 name_row_mode4(u16 row) override;
	virtual u16 tile1_select_mode4(u16 tile_number) override;
	virtual u16 tile2_select_mode4(u16 tile_number) override;
	virtual u8 sprite_attribute_extra_offset_mode4(u8 offset) override;
	virtual u8 sprite_tile_select_mode4(u8 tile_number) override;
	virtual void select_extended_res_mode4(bool M1, bool M2, bool M3) override;

private:
	void sega315_5246_palette(palette_device &palette) const;
};


class sega315_5377_device : public sega315_5246_device
{
public:
	sega315_5377_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void set_sega315_5124_compatibility_mode(bool sega315_5124_compatibility_mode) override;

protected:
	sega315_5377_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 cram_size, u8 palette_offset, u8 reg_num_mask, int max_sprite_zoom_hcount, int max_sprite_zoom_vcount, const u8 *line_timing);

	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual void vblank_end(int vpos) override;
	virtual void update_palette() override;
	virtual void cram_write(u8 data) override;
	virtual void blit_scanline(int *line_buffer, int *priority_selected, int pixel_offset_x, int pixel_plot_y, int line) override;

private:
	void sega315_5377_palette(palette_device &palette) const;
};


// Embedded mode 4 support of the 315-5313 (Mega Drive) VDP
class sega315_5313_mode4_device : public sega315_5246_device
{
public:
	void stop_timers();

protected:
	sega315_5313_mode4_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 cram_size, u8 palette_offset, u8 reg_num_mask, int max_sprite_zoom_hcount, int max_sprite_zoom_vcount, const u8 *line_timing);

	virtual void device_add_mconfig(machine_config &config) override;

	virtual void update_palette() override;
	virtual void write_memory(u8 data) override;
	virtual void load_vram_addr(u8 data) override;
	virtual void select_sprites(int line) override;
	virtual void sprite_collision(int line, int sprite_col_x) override;
	virtual void sprite_count_overflow(int line, int sprite_index) override;
	virtual void select_display_mode() override;
	virtual void select_extended_res_mode4(bool M1, bool M2, bool M3) override;
	virtual void draw_leftmost_pixels_mode4(int *line_buffer, int *priority_selected, int fine_x_scroll, int palette_selected, int tile_line) override;

private:
	void sega315_5313_palette(palette_device &palette) const;
};

#endif // MAME_VIDEO_315_5124_H
