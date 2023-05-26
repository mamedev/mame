// license:BSD-3-Clause
// copyright-holders:Barry Rodewald

#ifndef MAME_VIDEO_PC_VGA_TSENG_H
#define MAME_VIDEO_PC_VGA_TSENG_H

#pragma once

#include "screen.h"
#include "video/pc_vga.h"

class tseng_vga_device :  public svga_device
{
public:
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

	// construction/destruction
	tseng_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t port_03b0_r(offs_t offset) override;
	virtual void port_03b0_w(offs_t offset, uint8_t data) override;
	virtual uint8_t port_03c0_r(offs_t offset) override;
	virtual void port_03c0_w(offs_t offset, uint8_t data) override;
	virtual uint8_t port_03d0_r(offs_t offset) override;
	virtual void port_03d0_w(offs_t offset, uint8_t data) override;
	virtual uint8_t mem_r(offs_t offset) override;
	virtual void mem_w(offs_t offset, uint8_t data) override;

protected:
	virtual void device_start() override;

private:
	void tseng_define_video_mode();
	uint8_t tseng_crtc_reg_read(uint8_t index);
	void tseng_crtc_reg_write(uint8_t index, uint8_t data);
	uint8_t tseng_seq_reg_read(uint8_t index);
	void tseng_seq_reg_write(uint8_t index, uint8_t data);
	void tseng_attribute_reg_write(uint8_t index, uint8_t data);

	struct
	{
		uint8_t reg_3d8;
		uint8_t dac_ctrl;
		uint8_t dac_state;
		uint8_t horz_overflow;
		uint8_t aux_ctrl;
		bool ext_reg_ena;
		uint8_t misc1;
		uint8_t misc2;
	}et4k;
};


// device type definition
DECLARE_DEVICE_TYPE(TSENG_VGA, tseng_vga_device)

#endif // MAME_VIDEO_PC_VGA_TSENG_H
