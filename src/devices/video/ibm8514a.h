// license:BSD-3-Clause
// copyright-holders:Barry Rodewald

#ifndef MAME_VIDEO_IBM8514A_H
#define MAME_VIDEO_IBM8514A_H

#pragma once

#include "screen.h"
#include "video/pc_vga.h"

class svga_device;

class ibm8514a_device : public device_t
{
public:
	ibm8514a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_vga(T &&tag) { m_vga.set_tag(std::forward<T>(tag)); }
	void set_vga_owner() { m_vga.set_tag(DEVICE_SELF); }

	void enabled();

	bool is_8514a_enabled() { return ibm8514.enabled; }
	bool is_passthrough_set() { return ibm8514.passthrough; }

	uint16_t ibm8514_gpstatus_r();
	void ibm8514_cmd_w(uint16_t data);
	void ibm8514_display_ctrl_w(uint16_t data);
	uint16_t ibm8514_line_error_r();
	void ibm8514_line_error_w(uint16_t data);
	uint8_t ibm8514_status_r(offs_t offset);
	void ibm8514_htotal_w(offs_t offset, uint8_t data);
	uint16_t ibm8514_substatus_r();
	void ibm8514_subcontrol_w(uint16_t data);
	uint16_t ibm8514_subcontrol_r();
	uint16_t ibm8514_htotal_r();
	uint16_t ibm8514_vtotal_r();
	void ibm8514_vtotal_w(uint16_t data);
	uint16_t ibm8514_vdisp_r();
	void ibm8514_vdisp_w(uint16_t data);
	uint16_t ibm8514_vsync_r();
	void ibm8514_vsync_w(uint16_t data);
	uint16_t ibm8514_desty_r();
	void ibm8514_desty_w(uint16_t data);
	uint16_t ibm8514_destx_r();
	void ibm8514_destx_w(uint16_t data);
	uint16_t ibm8514_ssv_r();
	void ibm8514_ssv_w(uint16_t data);
	uint16_t ibm8514_currentx_r();
	void ibm8514_currentx_w(uint16_t data);
	uint16_t ibm8514_currenty_r();
	void ibm8514_currenty_w(uint16_t data);
	uint16_t ibm8514_width_r();
	void ibm8514_width_w(uint16_t data);
	uint16_t ibm8514_fgcolour_r();
	void ibm8514_fgcolour_w(uint16_t data);
	uint16_t ibm8514_bgcolour_r();
	void ibm8514_bgcolour_w(uint16_t data);
	uint16_t ibm8514_multifunc_r();
	void ibm8514_multifunc_w(uint16_t data);
	uint16_t ibm8514_backmix_r();
	void ibm8514_backmix_w(uint16_t data);
	uint16_t ibm8514_foremix_r();
	void ibm8514_foremix_w(uint16_t data);
	uint16_t ibm8514_pixel_xfer_r(offs_t offset);
	virtual void ibm8514_pixel_xfer_w(offs_t offset, uint16_t data);
	uint16_t ibm8514_read_mask_r();
	void ibm8514_read_mask_w(uint16_t data);
	uint16_t ibm8514_write_mask_r();
	void ibm8514_write_mask_w(uint16_t data);
	void ibm8514_advfunc_w(uint16_t data);

	void ibm8514_wait_draw();
	struct
	{
		uint16_t htotal;  // Horizontal total (9 bits)
		uint16_t vtotal;  // Vertical total adjust (3 bits), Vertical total base (9 bit)
		uint16_t vdisp;
		uint16_t vsync;
		uint16_t subctrl;
		uint16_t substatus;
		uint8_t display_ctrl;
		uint16_t ssv;
		uint16_t ec0;
		uint16_t ec1;
		uint16_t ec2;
		uint16_t ec3;
		bool gpbusy;
		bool data_avail;
		int16_t dest_x;
		int16_t dest_y;
		int16_t curr_x;
		int16_t curr_y;
		int16_t prev_x;
		int16_t prev_y;
		int16_t line_axial_step;
		int16_t line_diagonal_step;
		int16_t line_errorterm;
		uint16_t current_cmd;
		uint16_t src_x;
		uint16_t src_y;
		int16_t scissors_left;
		int16_t scissors_right;
		int16_t scissors_top;
		int16_t scissors_bottom;
		uint16_t rect_width;
		uint16_t rect_height;
		uint32_t fgcolour;
		uint32_t bgcolour;
		uint16_t fgmix;
		uint16_t bgmix;
		uint32_t pixel_xfer;
		uint16_t pixel_control;
		uint8_t bus_size;
		uint8_t multifunc_sel;
		uint16_t multifunc_misc;
		uint32_t read_mask;
		uint32_t write_mask;
		uint16_t advfunction_ctrl;
		bool enabled;
		bool passthrough;

		int state;
		uint8_t wait_vector_len;
		uint8_t wait_vector_dir;
		bool wait_vector_draw;
		uint8_t wait_vector_count;

	} ibm8514;

protected:
	ibm8514a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	void ibm8514_write(uint32_t offset, uint32_t src);
	void ibm8514_write_fg(uint32_t offset);
	void ibm8514_write_bg(uint32_t offset);

	required_device<svga_device> m_vga;  // for pass-through
private:
	void ibm8514_draw_vector(uint8_t len, uint8_t dir, bool draw);
	void ibm8514_wait_draw_ssv();
	void ibm8514_draw_ssv(uint8_t data);
	void ibm8514_wait_draw_vector();

	//uint8_t* m_vram;  // the original 8514/A has it's own VRAM, but most VGA+8514 combination cards will have
					// only one set of VRAM, so this will only be needed in standalone 8514/A cards
	//uint32_t m_vramsize;
};

// device type definition
DECLARE_DEVICE_TYPE(IBM8514A, ibm8514a_device)

#endif // MAME_VIDEO_IBM8514A_H
