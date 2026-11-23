// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_SKELETON_C2COLOR_CPU_H
#define MAME_SKELETON_C2COLOR_CPU_H

#pragma once

#include "cpu/mcs51/i8052.h"

DECLARE_DEVICE_TYPE(C2_COLOR_CPU, c2_color_cpu_device)

// The exact SoC is still unidentified.  Use the ROM-less 8052 configuration
// with the additional arithmetic registers exercised by the firmware.
class c2_color_cpu_device : public i8052_device
{
public:
	c2_color_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void sfr_map(address_map &map) override ATTR_COLD;

private:
	u8 math_r(offs_t offset) { return m_math[offset]; }
	void math_w(offs_t offset, u8 data);
	u8 unknown_r() { return m_unknown; }
	void unknown_w(u8 data) { m_unknown = data; }
	void clear_state();

	u8 m_math[6];
	u8 m_math_written;
	u8 m_unknown;
};

#endif // MAME_SKELETON_C2COLOR_CPU_H
