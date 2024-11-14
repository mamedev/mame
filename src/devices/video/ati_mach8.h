// license:BSD-3-Clause
// copyright-holders:Barry Rodewald

#ifndef MAME_VIDEO_MACH8_H
#define MAME_VIDEO_MACH8_H

#pragma once

#include "video/ibm8514a.h"
#include "video/pc_vga.h"


class mach8_device : public ibm8514a_device
{
public:
	mach8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t mach8_ec0_r();
	void mach8_ec0_w(uint16_t data);
	uint16_t mach8_ec1_r();
	void mach8_ec1_w(uint16_t data);
	uint16_t mach8_ec2_r();
	void mach8_ec2_w(uint16_t data);
	uint16_t mach8_ec3_r();
	void mach8_ec3_w(uint16_t data);
	uint16_t mach8_ext_fifo_r();
	void mach8_linedraw_index_w(uint16_t data);
	uint16_t mach8_bresenham_count_r();
	void mach8_bresenham_count_w(uint16_t data);
	void mach8_linedraw_w(uint16_t data);
	uint16_t mach8_linedraw_r();
	uint16_t mach8_scratch0_r();
	void mach8_scratch0_w(uint16_t data);
	uint16_t mach8_scratch1_r();
	void mach8_scratch1_w(uint16_t data);
	uint16_t mach8_config1_r();
	uint16_t mach8_config2_r();
	uint16_t mach8_sourcex_r();
	uint16_t mach8_sourcey_r();
	void mach8_ext_leftscissor_w(uint16_t data);
	void mach8_ext_topscissor_w(uint16_t data);
	uint16_t mach8_clksel_r() { return mach8.clksel; }
	void mach8_crt_pitch_w(uint16_t data);
	void mach8_patt_data_w(uint16_t data) { logerror("Mach8: Pattern Data write (unimplemented)\n"); }
	void mach8_ge_offset_l_w(uint16_t data);
	void mach8_ge_offset_h_w(uint16_t data);
	void mach8_ge_pitch_w(uint16_t data);
	uint16_t mach8_ge_ext_config_r() { return mach8.ge_ext_config; }
	void mach8_ge_ext_config_w(uint16_t data);  // TODO: handle 8-bit I/O
	void mach8_scan_x_w(uint16_t data);
	void mach8_dp_config_w(uint16_t data);
	uint16_t mach8_readonly_r() { return 0; }
	void mach8_pixel_xfer_w(offs_t offset, uint16_t data);
	void mach8_clksel_w(uint16_t data) { mach8.ati_mode = true; ibm8514.passthrough = data & 0x0001; mach8.clksel = data; }  // read only on the mach8
	void mach8_advfunc_w(uint16_t data) { mach8.ati_mode = false; ibm8514_advfunc_w(data); }
	uint16_t get_ext_config() { return mach8.ge_ext_config; }
	uint16_t offset() { if(mach8.ati_mode) return mach8.ge_pitch; else return 128; }

protected:
	mach8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual void device_start() override ATTR_COLD;
	struct
	{
		uint16_t scratch0;
		uint16_t scratch1;
		uint16_t linedraw;
		uint16_t clksel;
		uint16_t crt_pitch;
		uint16_t dp_config;
		uint32_t ge_offset;
		uint16_t ge_pitch;
		uint16_t ge_ext_config;  // usage varies between the mach8 and mach32 (except for 8514/A monitor alias)
		uint16_t scan_x;
		bool ati_mode;
	} mach8;

private:
	void mach8_wait_scan();

};

// device type definition
DECLARE_DEVICE_TYPE(MACH8, mach8_device)

#endif // MAME_VIDEO_MACH8_H
