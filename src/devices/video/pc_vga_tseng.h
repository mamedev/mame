// license:BSD-3-Clause
// copyright-holders:Barry Rodewald

#ifndef MAME_VIDEO_PC_VGA_TSENG_H
#define MAME_VIDEO_PC_VGA_TSENG_H

#pragma once

#include "video/pc_vga.h"

#include "screen.h"


class tseng_vga_device :  public svga_device
{
public:
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

	// construction/destruction
	tseng_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t mem_r(offs_t offset) override;
	virtual void mem_w(offs_t offset, uint8_t data) override;

protected:
	virtual void device_start() override ATTR_COLD;

	virtual void io_3bx_3dx_map(address_map &map) override ATTR_COLD;
	virtual void io_3cx_map(address_map &map) override ATTR_COLD;

	u8 ramdac_hidden_mask_r(offs_t offset);
	void ramdac_hidden_mask_w(offs_t offset, u8 data);
	u8 ramdac_hidden_windex_r(offs_t offset);

	virtual void crtc_map(address_map &map) override ATTR_COLD;
	virtual void sequencer_map(address_map &map) override ATTR_COLD;
	virtual void attribute_map(address_map &map) override ATTR_COLD;

	virtual void recompute_params() override;
private:
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
