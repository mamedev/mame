// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_XAVGOLF_SWING_H
#define MAME_TVGAMES_XAVIX_IO_XAVGOLF_SWING_H

#pragma once

#include "xavix_io_ir.h"

class xavix_golf_swing_device : public xavix_ir_framed_device
{
public:
	xavix_golf_swing_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	DECLARE_INPUT_CHANGED_MEMBER(swing_action);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual u32 read_report(unsigned report) override;
	virtual bool reports_enabled() override;

private:
	required_ioport m_swing_active;
	required_ioport_array<2> m_swing_axis;
	required_ioport m_swing_buttons;
	required_ioport m_swing_power;

	u8 m_swing_phase;
	bool m_swing_reverse;
};

DECLARE_DEVICE_TYPE(XAVIX_GOLF_SWING, xavix_golf_swing_device)

#endif // MAME_TVGAMES_XAVIX_IO_XAVGOLF_SWING_H
