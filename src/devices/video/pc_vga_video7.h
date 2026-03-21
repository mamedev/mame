// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_VIDEO_PC_VGA_VIDEO7_H
#define MAME_VIDEO_PC_VGA_VIDEO7_H

#pragma once

#include "video/pc_vga.h"

class ht208_video7_vga_device :  public svga_device
{
public:
	ht208_video7_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// black screen, several missing features
	static constexpr feature_type unemulated_features() { return feature::GRAPHICS; }

protected:
	ht208_video7_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void sequencer_map(address_map &map) override ATTR_COLD;

private:
	bool m_seq_unlock_reg;

	u8 m_ext_b0_scratch[16];

	u8 m_ext_clock_select;
	u8 m_ext_fbctrl;
	u8 m_ext_16bit;
};

DECLARE_DEVICE_TYPE(HT208_VIDEO7_VGA, ht208_video7_vga_device)


#endif // MAME_VIDEO_PC_VGA_VIDEO7_H
