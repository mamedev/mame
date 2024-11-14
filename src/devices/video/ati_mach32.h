// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * mach32.h
 *
 *  Created on: 16/05/2014
 */

#ifndef MAME_BUS_ISA_MACH32_H
#define MAME_BUS_ISA_MACH32_H

#pragma once

#include "machine/eepromser.h"
#include "video/ati_mach8.h"
#include "video/pc_vga_ati.h"


// 8514/A module of the Mach32
class mach32_8514a_device : public mach8_device
{
public:
	// construction/destruction
	mach32_8514a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t mach32_chipid_r() { return m_chip_ID; }
	uint16_t mach32_mem_boundary_r() { return m_membounds; }
	void mach32_mem_boundary_w(uint16_t data);
	void mach32_ge_ext_config_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t mach32_config1_r();
	void mach32_horz_overscan_w(uint16_t data) {}  // TODO
	uint16_t mach32_ext_ge_r() { return 0x0000; }  // TODO

	bool has_display_mode_changed() { if(display_mode_change) { display_mode_change = false; return true; } else return false; }

protected:
	mach32_8514a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	uint16_t m_chip_ID;
	uint16_t m_membounds;
	bool display_mode_change;

};

// main SVGA device
class mach32_device : public ati_vga_device
{
public:
	// construction/destruction
	mach32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;
	uint32_t draw_hw_cursor(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<mach32_8514a_device> m_8514a;  // provides accelerated 2D drawing, derived from the Mach8 device

	// map 8514/A functions to 8514/A module
	uint16_t mach8_ec0_r() { return m_8514a->mach8_ec0_r(); }
	void mach8_ec0_w(uint16_t data) { m_8514a->mach8_ec0_w(data); }
	uint16_t mach8_ec1_r() { return m_8514a->mach8_ec1_r(); }
	void mach8_ec1_w(uint16_t data) { m_8514a->mach8_ec1_w(data); }
	uint16_t mach8_ec2_r() { return m_8514a->mach8_ec2_r(); }
	void mach8_ec2_w(uint16_t data) { m_8514a->mach8_ec2_w(data); }
	uint16_t mach8_ec3_r() { return m_8514a->mach8_ec3_r(); }
	void mach8_ec3_w(uint16_t data) { m_8514a->mach8_ec3_w(data); }
	uint16_t mach8_ext_fifo_r() { return m_8514a->mach8_ext_fifo_r(); }
	void mach8_linedraw_index_w(uint16_t data) { m_8514a->mach8_linedraw_index_w(data); }
	uint16_t mach8_bresenham_count_r() { return m_8514a->mach8_bresenham_count_r(); }
	void mach8_bresenham_count_w(uint16_t data) { m_8514a->mach8_bresenham_count_w(data); }
	void mach8_linedraw_w(uint16_t data) { m_8514a->mach8_linedraw_w(data); }
	uint16_t mach8_linedraw_r() { return m_8514a->mach8_linedraw_r(); }
	uint16_t mach8_scratch0_r() { return m_8514a->mach8_scratch0_r(); }
	void mach8_scratch0_w(uint16_t data) { m_8514a->mach8_scratch0_w(data); }
	uint16_t mach8_scratch1_r() { return m_8514a->mach8_scratch1_r(); }
	void mach8_scratch1_w(uint16_t data) { m_8514a->mach8_scratch1_w(data); }
	void mach8_crt_pitch_w(uint16_t data) { m_8514a->mach8_crt_pitch_w(data); }
	uint16_t mach8_config1_r() { return m_8514a->mach8_config1_r(); }
	uint16_t mach8_config2_smo_r() { return m_8514a->mach8_config2_r(); }
	uint16_t mach8_config2_sm_r(offs_t offset) { return m_8514a->mach8_config2_r(); }
	uint16_t mach8_sourcex_r() { return m_8514a->mach8_sourcex_r(); }
	uint16_t mach8_sourcey_r() { return m_8514a->mach8_sourcey_r(); }
	void mach8_ext_leftscissor_w(uint16_t data) { m_8514a->mach8_ext_leftscissor_w(data); }
	void mach8_ext_topscissor_w(uint16_t data) { m_8514a->mach8_ext_topscissor_w(data); }
	void mach8_ge_offset_l_w(uint16_t data) { m_8514a->mach8_ge_offset_l_w(data); }
	void mach8_ge_offset_h_w(uint16_t data) { m_8514a->mach8_ge_offset_h_w(data); }
	void mach8_scan_x_w(uint16_t data) { m_8514a->mach8_scan_x_w(data); }
	void mach8_dp_config_w(uint16_t data) { m_8514a->mach8_dp_config_w(data); }
	void mach8_ge_pitch_w(uint16_t data) { m_8514a->mach8_ge_pitch_w(data); }
	uint16_t mach8_ge_ext_config_r() { return m_8514a->mach8_ge_ext_config_r(); }
	void mach8_patt_data_w(uint16_t data) { m_8514a->mach8_patt_data_w(data); }

	uint16_t ibm8514_vtotal_r() { return m_8514a->ibm8514_vtotal_r(); }
	void ibm8514_vtotal_w(uint16_t data) { m_8514a->ibm8514_vtotal_w(data); }
	uint16_t ibm8514_htotal_r() { return m_8514a->ibm8514_htotal_r(); }
	void ibm8514_htotal_w(offs_t offset, uint8_t data) { m_8514a->ibm8514_htotal_w(offset,data); }
	uint16_t ibm8514_vdisp_r() { return m_8514a->ibm8514_vdisp_r(); }
	void ibm8514_vdisp_w(uint16_t data) { m_8514a->ibm8514_vdisp_w(data); }
	uint16_t ibm8514_vsync_r() { return m_8514a->ibm8514_vsync_r(); }
	void ibm8514_vsync_w(uint16_t data) { m_8514a->ibm8514_vsync_w(data); }
	uint16_t ibm8514_substatus_r() { return m_8514a->ibm8514_substatus_r(); }
	void ibm8514_subcontrol_w(uint16_t data) { m_8514a->ibm8514_subcontrol_w(data); }
	uint16_t ibm8514_subcontrol_r() { return m_8514a->ibm8514_subcontrol_r(); }
	uint16_t ibm8514_currentx_r() { return m_8514a->ibm8514_currentx_r(); }
	void ibm8514_currentx_w(uint16_t data) { m_8514a->ibm8514_currentx_w(data); }
	uint16_t ibm8514_currenty_r() { return m_8514a->ibm8514_currenty_r(); }
	void ibm8514_currenty_w(uint16_t data) { m_8514a->ibm8514_currenty_w(data); }
	uint16_t ibm8514_desty_r() { return m_8514a->ibm8514_desty_r(); }
	void ibm8514_desty_w(uint16_t data) { m_8514a->ibm8514_desty_w(data); }
	uint16_t ibm8514_destx_r() { return m_8514a->ibm8514_destx_r(); }
	void ibm8514_destx_w(uint16_t data) { m_8514a->ibm8514_destx_w(data); }
	uint16_t ibm8514_line_error_r() { return m_8514a->ibm8514_line_error_r(); }
	void ibm8514_line_error_w(uint16_t data) { m_8514a->ibm8514_line_error_w(data); }
	uint16_t ibm8514_width_r() { return m_8514a->ibm8514_width_r(); }
	void ibm8514_width_w(uint16_t data) { m_8514a->ibm8514_width_w(data); }
	uint16_t ibm8514_gpstatus_r() { return m_8514a->ibm8514_gpstatus_r(); }
	void ibm8514_cmd_w(uint16_t data) { m_8514a->ibm8514_cmd_w(data); }
	uint16_t ibm8514_ssv_r() { return m_8514a->ibm8514_ssv_r(); }
	void ibm8514_ssv_w(uint16_t data) { m_8514a->ibm8514_ssv_w(data); }
	uint16_t ibm8514_fgcolour_r() { return m_8514a->ibm8514_fgcolour_r(); }
	void ibm8514_fgcolour_w(uint16_t data) { m_8514a->ibm8514_fgcolour_w(data); }
	uint16_t ibm8514_bgcolour_r() { return m_8514a->ibm8514_bgcolour_r(); }
	void ibm8514_bgcolour_w(uint16_t data) { m_8514a->ibm8514_bgcolour_w(data); }
	uint16_t ibm8514_read_mask_r() { return m_8514a->ibm8514_read_mask_r(); }
	void ibm8514_read_mask_w(uint16_t data) { m_8514a->ibm8514_read_mask_w(data); }
	uint16_t ibm8514_write_mask_r() { return m_8514a->ibm8514_write_mask_r(); }
	void ibm8514_write_mask_w(uint16_t data) { m_8514a->ibm8514_write_mask_w(data); }
	uint16_t ibm8514_backmix_r() { return m_8514a->ibm8514_backmix_r(); }
	void ibm8514_backmix_w(uint16_t data) { m_8514a->ibm8514_backmix_w(data); }
	uint16_t ibm8514_foremix_r() { return m_8514a->ibm8514_foremix_r(); }
	void ibm8514_foremix_w(uint16_t data) { m_8514a->ibm8514_foremix_w(data); }
	uint16_t ibm8514_multifunc_r() { return m_8514a->ibm8514_multifunc_r(); }
	void ibm8514_multifunc_w(uint16_t data) { m_8514a->ibm8514_multifunc_w(data); }
	uint16_t ibm8514_pixel_xfer_r(offs_t offset) { return m_8514a->ibm8514_pixel_xfer_r(offset); }
	void mach8_pixel_xfer_w(offs_t offset, uint16_t data) { m_8514a->mach8_pixel_xfer_w(offset, data); }
	void mach8_advfunc_w(uint16_t data) { m_8514a->mach8_advfunc_w(data); }

	uint16_t mach32_chipid_r() { return m_8514a->mach32_chipid_r(); }
	uint16_t mach8_clksel_r() { return m_8514a->mach8_clksel_r(); }
	void mach8_clksel_w(uint16_t data) { m_8514a->mach8_clksel_w(data); }  // read only on the mach8
	uint16_t mach32_mem_boundary_r() { return m_8514a->mach32_mem_boundary_r(); }
	void mach32_mem_boundary_w(uint16_t data) { m_8514a->mach32_mem_boundary_w(data); }  // read only on the mach8
	uint8_t mach32_status_r(offs_t offset) { return m_8514a->ibm8514_status_r(offset); }
	uint16_t mach32_config1_smo_r() { return m_8514a->mach32_config1_r(); }
	uint16_t mach32_config1_sm_r(offs_t offset) { return m_8514a->mach32_config1_r(); }
	void mach32_horz_overscan_w(uint16_t data) { m_8514a->mach32_horz_overscan_w(data); }
	void mach32_ge_ext_config_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { m_8514a->mach32_ge_ext_config_w(offset, data, mem_mask); ati_define_video_mode(); }
	uint16_t mach32_ext_ge_r() { return m_8514a->mach32_ext_ge_r(); }
	uint16_t mach32_readonly_s_r(offs_t offset, uint16_t mem_mask) { return 0; }
	uint16_t mach32_readonly_sm_r(offs_t offset) { return 0; }
	uint16_t mach32_readonly_smo_r() { return 0; }
	void mach32_cursor_pos_h(offs_t offset, uint16_t data);
	void mach32_cursor_pos_v(offs_t offset, uint16_t data);
	void mach32_cursor_colour_b_w(offs_t offset, uint16_t data);
	void mach32_cursor_colour_0_w(offs_t offset, uint16_t data);
	void mach32_cursor_colour_1_w(offs_t offset, uint16_t data);
	void mach32_cursor_l_w(offs_t offset, uint16_t data);
	void mach32_cursor_h_w(offs_t offset, uint16_t data);
	void mach32_cursor_offset_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

protected:
	mach32_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void ati_define_video_mode() override;
	virtual uint16_t offset() override;

	// hardware pointer
	bool m_cursor_enable;
	uint32_t m_cursor_address;
	uint16_t m_cursor_horizontal;
	uint16_t m_cursor_vertical;
	uint8_t m_cursor_colour0_b;
	uint8_t m_cursor_colour0_r;
	uint8_t m_cursor_colour0_g;
	uint8_t m_cursor_colour1_b;
	uint8_t m_cursor_colour1_r;
	uint8_t m_cursor_colour1_g;
	uint8_t m_cursor_offset_horizontal;
	uint8_t m_cursor_offset_vertical;

};

/*
 *   ATi mach64
 */

// 8514/A module of the Mach64
class mach64_8514a_device : public mach32_8514a_device
{
public:
	// construction/destruction
	mach64_8514a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	mach64_8514a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};

// main SVGA device
class mach64_device : public mach32_device
{
public:
	// construction/destruction
	mach64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void mach64_config1_w(uint16_t data) { }  // why does the mach64 BIOS write to these, they are read only on the mach32 and earlier
	void mach64_config2_w(uint16_t data) { }

	void set_color(u8 index, u32 color);
	u32 framebuffer_r(offs_t offset, u32 mem_mask);
	void framebuffer_w(offs_t offset, u32 data, u32 mem_mask);
	u32 framebuffer_be_r(offs_t offset, u32 mem_mask);
	void framebuffer_be_w(offs_t offset, u32 data, u32 mem_mask);
	u8 *get_framebuffer_addr() { return &vga.memory[0]; }

protected:
	mach64_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<mach64_8514a_device> m_8514a;  // provides accelerated 2D drawing, derived from the Mach8 device

private:
};

// device type definition
DECLARE_DEVICE_TYPE(ATIMACH32,       mach32_device)
DECLARE_DEVICE_TYPE(ATIMACH32_8514A, mach32_8514a_device)
DECLARE_DEVICE_TYPE(ATIMACH64,       mach64_device)
DECLARE_DEVICE_TYPE(ATIMACH64_8514A, mach64_8514a_device)

#endif // MAME_BUS_ISA_MACH32_H
