// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
    SGI "Newport" graphics board used in the Indy and some Indigo2s
*/

#ifndef MAME_VIDEO_NEWPORT_H
#define MAME_VIDEO_NEWPORT_H

#pragma once

#include "machine/hpc3.h"

#define ENABLE_NEWVIEW_LOG      (0)

class newport_video_device : public device_t
{
public:
	template <typename T, typename U>
	newport_video_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu_tag, U &&hpc3_tag)
		: newport_video_device(mconfig, tag, owner, (uint32_t)0) // TODO: Use actual pixel clock
	{
		m_maincpu.set_tag(std::forward<T>(cpu_tag));
		m_hpc3.set_tag(std::forward<U>(hpc3_tag));
	}

	newport_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ64_MEMBER(rex3_r);
	DECLARE_WRITE64_MEMBER(rex3_w);

	uint32_t screen_update(screen_device &device, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE_LINE_MEMBER(vblank_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	enum
	{
		DCR_CURSOR_FUNC_ENABLE_BIT = 4,
		DCR_CURSOR_ENABLE_BIT = 7,

		DCR_CURSOR_MODE_BIT = 8,
		DCR_CURSOR_MODE_GLYPH = 0,
		DCR_CURSOR_MODE_CROSSHAIR = 1,

		DCR_CURSOR_SIZE_BIT = 9,
		DCR_CURSOR_SIZE_32 = 0,
		DCR_CURSOR_SIZE_64 = 1
	};

	struct vc2_t
	{
		uint16_t m_vid_entry;
		uint16_t m_cursor_entry;
		uint16_t m_cursor_x;
		uint16_t m_cursor_y;
		uint16_t m_cur_cursor_x;
		uint16_t m_did_entry;
		uint16_t m_scanline_len;
		uint16_t m_ram_addr;
		uint16_t m_vt_frame_ptr;
		uint16_t m_vt_line_ptr;
		uint16_t m_vt_line_run;
		uint16_t m_vt_line_count;
		uint16_t m_cursor_table_ptr;
		uint16_t m_work_cursor_y;
		uint16_t m_did_frame_ptr;
		uint16_t m_did_line_ptr;
		uint16_t m_display_ctrl;
		uint16_t m_config;
		uint16_t m_ram[0x8000];
		uint8_t m_reg_idx;
		uint16_t m_reg_data;
	};


	struct xmap_t
	{
		uint32_t m_config;
		uint32_t m_revision;
		uint32_t m_entries;
		uint32_t m_cursor_cmap;
		uint32_t m_popup_cmap;
		uint32_t m_mode_table_idx;
		uint32_t m_mode_table[0x20];
	};

	struct rex3_t
	{
		uint32_t m_draw_mode0;
		uint32_t m_draw_mode1;
		uint32_t m_write_width;
		uint32_t m_ls_mode;
		uint32_t m_ls_pattern;
		uint32_t m_ls_pattern_saved;
		uint32_t m_z_pattern;
		uint32_t m_color_back;
		uint32_t m_color_vram;
		uint32_t m_alpha_ref;
		uint32_t m_setup;
		uint32_t m_step_z;
		int32_t m_x_start;
		int32_t m_y_start;
		int32_t m_x_end;
		int32_t m_y_end;
		int16_t m_x_save;
		uint32_t m_xy_move;
		int16_t m_x_move;
		int16_t m_y_move;
		uint32_t m_bres_d;
		uint32_t m_bres_s1;
		uint32_t m_bres_octant_inc1;
		uint32_t m_bres_round_inc2;
		uint32_t m_bres_e1;
		uint32_t m_bres_s2;
		uint32_t m_a_weight0;
		uint32_t m_a_weight1;
		uint32_t m_x_start_f;
		uint32_t m_y_start_f;
		uint32_t m_x_end_f;
		uint32_t m_y_end_f;
		int16_t m_x_start_i;
		uint32_t m_xy_start_i;
		int16_t m_y_start_i;
		uint32_t m_xy_end_i;
		int16_t m_x_end_i;
		int16_t m_y_end_i;
		uint32_t m_x_start_end_i;
		int32_t m_color_red;
		int32_t m_color_alpha;
		int32_t m_color_green;
		int32_t m_color_blue;
		int32_t m_slope_red;
		int32_t m_slope_alpha;
		int32_t m_slope_green;
		int32_t m_slope_blue;
		uint32_t m_write_mask;
		uint32_t m_color_i;
		uint32_t m_zero_overflow;
		uint64_t m_host_dataport;
		uint32_t m_dcb_mode;
		uint32_t m_dcb_reg_select;
		uint32_t m_dcb_slave_select;
		uint32_t m_dcb_data_msw;
		uint32_t m_dcb_data_lsw;
		uint32_t m_smask_x[5];
		uint32_t m_smask_y[5];
		uint32_t m_top_scanline;
		uint32_t m_xy_window;
		int16_t m_x_window;
		int16_t m_y_window;
		uint32_t m_clip_mode;
		uint32_t m_config;
		uint32_t m_status;
		uint8_t m_xfer_width;
		bool m_read_active;
	};

	struct cmap_t
	{
		uint16_t m_palette_idx;
		uint32_t m_palette[0x10000];
	};

	uint8_t get_cursor_pixel(int x, int y);

	// internal state

	uint32_t cmap0_read();
	void cmap0_write(uint32_t data);
	uint32_t cmap1_read();
	uint32_t xmap0_read();
	void xmap0_write(uint32_t data);
	uint32_t xmap1_read();
	void xmap1_write(uint32_t data);
	uint32_t vc2_read();
	void vc2_write(uint32_t data);

	void write_x_start(int32_t val);
	void write_y_start(int32_t val);
	void write_x_end(int32_t val);
	void write_y_end(int32_t val);

	bool pixel_clip_pass(int16_t x, int16_t y);
	void write_pixel(uint8_t color, bool shade);
	void write_pixel(int16_t x, int16_t y, uint8_t color);
	void store_pixel(uint8_t *dest_buf, uint8_t src);

	void do_v_iline(uint8_t color, bool skip_last, bool shade);
	void do_h_iline(uint8_t color, bool skip_last, bool shade);
	void do_iline(uint8_t color, bool skip_last, bool shade);
	uint8_t do_pixel_read();
	uint64_t do_pixel_word_read();
	void do_rex3_command();

	required_device<cpu_device> m_maincpu;
	required_device<hpc3_device> m_hpc3;
	vc2_t  m_vc2;
	xmap_t m_xmap0;
	xmap_t m_xmap1;
	rex3_t m_rex3;
	std::unique_ptr<uint8_t[]> m_rgbci;
	std::unique_ptr<uint8_t[]> m_olay;
	std::unique_ptr<uint8_t[]> m_pup;
	std::unique_ptr<uint8_t[]> m_cid;
	cmap_t m_cmap0;

#if ENABLE_NEWVIEW_LOG
	void start_logging();
	void stop_logging();

	FILE *m_newview_log;
#endif
};

DECLARE_DEVICE_TYPE(NEWPORT_VIDEO, newport_video_device)


#endif // MAME_VIDEO_NEWPORT_H
