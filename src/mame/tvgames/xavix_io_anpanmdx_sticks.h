// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_ANPANMDX_STICKS_H
#define MAME_TVGAMES_XAVIX_IO_ANPANMDX_STICKS_H

#pragma once

#include "xavix_io_ir.h"

#include <array>

class xavix_anpanman_sticks_device : public device_t
{
public:
	xavix_anpanman_sticks_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void set_input_source(device_t &source) ATTR_COLD;
	auto event_callback() { return m_ir.lookup()->event_callback(); }
	int data_r() { return m_ir->data_r(); }
	void vblank_w(int state);
	DECLARE_INPUT_CHANGED_MEMBER(stick_activate);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr u32 STICK_SEQUENCES[2][4] =
	{
		{ 0x03dd5, 0x03cdf, 0x03d50, 0x03c5a },
		{ 0x03d92, 0x03c98, 0x03d37, 0x03c3d }
	};

	int ir_enable_r();
	template <unsigned Player> u32 stick_sequence_r();

	required_device<xavix_ir_report_device> m_ir;
	required_ioport_array<2> m_motion_level;
	required_ioport m_stick_batteries;

	std::array<bool, 2> m_stick_awake;
};

DECLARE_DEVICE_TYPE(XAVIX_ANPANMAN_STICKS, xavix_anpanman_sticks_device)

INPUT_PORTS_EXTERN( xavix_anpanman_sticks );

#endif // MAME_TVGAMES_XAVIX_IO_ANPANMDX_STICKS_H
