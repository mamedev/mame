// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_ANPANMDX_STICKS_H
#define MAME_TVGAMES_XAVIX_IO_ANPANMDX_STICKS_H

#pragma once

#include "xavix_io_ir.h"

#include <array>

class xavix_anpanman_sticks_device : public xavix_ir_msb_device
{
public:
	xavix_anpanman_sticks_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	DECLARE_INPUT_CHANGED_MEMBER(stick_activate);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual u32 read_report(unsigned report) override;
	virtual bool reports_enabled() override;

private:
	static constexpr u32 STICK_SEQUENCES[2][4] =
	{
		{ 0x03dd5, 0x03cdf, 0x03d50, 0x03c5a },
		{ 0x03d92, 0x03c98, 0x03d37, 0x03c3d }
	};

	required_ioport_array<2> m_motion_level;
	required_ioport m_stick_batteries;

	std::array<bool, 2> m_stick_awake;
};

DECLARE_DEVICE_TYPE(XAVIX_ANPANMAN_STICKS, xavix_anpanman_sticks_device)

#endif // MAME_TVGAMES_XAVIX_IO_ANPANMDX_STICKS_H
