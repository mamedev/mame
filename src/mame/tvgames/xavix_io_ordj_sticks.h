// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_ORDJ_STICKS_H
#define MAME_TVGAMES_XAVIX_IO_ORDJ_STICKS_H

#pragma once

#include "xavix_io_ir_motion.h"

#include <array>

class xavix_hibiki_sticks_device : public xavix_ir_motion_base_device
{
public:
	xavix_hibiki_sticks_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	DECLARE_INPUT_CHANGED_MEMBER(stick_swing);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual u32 read_report(unsigned report) override;
	virtual bool reports_enabled() override;

private:
	required_ioport_array<2> m_swing_strength;
	required_ioport m_stick_power;

	std::array<u8, 2> m_latched_strength;
	std::array<u8, 2> m_reports_remaining;
};

DECLARE_DEVICE_TYPE(XAVIX_HIBIKI_STICKS, xavix_hibiki_sticks_device)

#endif // MAME_TVGAMES_XAVIX_IO_ORDJ_STICKS_H
