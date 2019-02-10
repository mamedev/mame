// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
    SGI "Newport" graphics board used in the Indy and some Indigo2s
*/

#ifndef MAME_VIDEO_NEWPORT_H
#define MAME_VIDEO_NEWPORT_H

#pragma once

#include "machine/hpc3.h"

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

	DECLARE_READ32_MEMBER(rex3_r);
	DECLARE_WRITE32_MEMBER(rex3_w);

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
		uint32_t m_ls_mode;
		uint32_t m_ls_pattern;
		uint32_t m_ls_pattern_saved;
		uint32_t m_z_pattern;
		uint32_t m_color_back;
		uint32_t m_color_vram;
		uint32_t m_alpha_ref;
		uint32_t m_setup;
		uint32_t m_step_z;
		uint32_t m_x_start;
		uint32_t m_y_start;
		uint32_t m_x_end;
		uint32_t m_y_end;
		uint32_t m_x_save;
		uint32_t m_xy_move;
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
		uint32_t m_x_start_i;
		uint32_t m_xy_start_i;
		uint32_t m_xy_end_i;
		uint32_t m_x_start_end_i;
		uint32_t m_color_red;
		uint32_t m_color_alpha;
		uint32_t m_color_green;
		uint32_t m_color_blue;
		uint32_t m_slope_red;
		uint32_t m_slope_alpha;
		uint32_t m_slope_green;
		uint32_t m_slope_blue;
		uint32_t m_write_mask;
		uint32_t m_zero_fract;
		uint32_t m_zero_overflow;
		uint32_t m_host_dataport_msw;
		uint32_t m_host_dataport_lsw;
		uint32_t m_dcb_mode;
		uint32_t m_dcb_reg_select;
		uint32_t m_dcb_slave_select;
		uint32_t m_dcb_data_msw;
		uint32_t m_dcb_data_lsw;
		uint32_t m_smask_x[5];
		uint32_t m_smask_y[5];
		uint32_t m_top_scanline;
		uint32_t m_xy_window;
		uint32_t m_clip_mode;
		uint32_t m_config;
		uint32_t m_status;
		int16_t m_iter_x;
		int16_t m_iter_y;
		uint8_t m_xfer_width;
		bool m_read_active;
	};

	struct cmap_t
	{
		uint16_t m_palette_idx;
		uint32_t m_palette[0x10000];
	};

	uint32_t get_cursor_pixel(int x, int y);

	// internal state

	DECLARE_READ32_MEMBER(cmap0_r);
	DECLARE_WRITE32_MEMBER(cmap0_w);
	DECLARE_READ32_MEMBER(cmap1_r);
	DECLARE_READ32_MEMBER(xmap0_r);
	DECLARE_WRITE32_MEMBER(xmap0_w);
	DECLARE_READ32_MEMBER(xmap1_r);
	DECLARE_WRITE32_MEMBER(xmap1_w);
	DECLARE_READ32_MEMBER(vc2_r);
	DECLARE_WRITE32_MEMBER(vc2_w);

	void write_pixel(uint32_t x, uint32_t y, uint8_t color);
	void do_v_iline(uint16_t x1, uint16_t y1, uint16_t y2, uint8_t color, bool skip_last);
	void do_h_iline(uint16_t x1, uint16_t y1, uint16_t x2, uint8_t color, bool skip_last);
	void do_iline(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color, bool skip_last);
	uint8_t do_pixel_read();
	uint32_t do_pixel_word_read();
	void do_rex3_command();

	required_device<cpu_device> m_maincpu;
	required_device<hpc3_device> m_hpc3;
	vc2_t  m_vc2;
	xmap_t m_xmap0;
	xmap_t m_xmap1;
	rex3_t m_rex3;
	std::unique_ptr<uint8_t[]> m_base;
	cmap_t m_cmap0;
};

DECLARE_DEVICE_TYPE(NEWPORT_VIDEO, newport_video_device)


#endif // MAME_VIDEO_NEWPORT_H
