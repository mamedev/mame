// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_CARDNIGHT_H
#define MAME_TVGAMES_XAVIX_IO_CARDNIGHT_H

#pragma once

#include "xavix_io_ir.h"

class xavix_cardnight_remote_device : public xavix_ir_run_length_device
{
public:
	xavix_cardnight_remote_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	auto dock_callback() { return m_dock_cb.bind(); }
	DECLARE_INPUT_CHANGED_MEMBER(reset_changed);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual u32 read_report(unsigned report) override;
	virtual bool reports_enabled() override;

private:
	static constexpr u8 DIRECTION_REPORTS[16] =
	{
		0x00, 0x11, 0x91, 0x00, 0x51, 0xa9, 0x15, 0x00,
		0x29, 0x49, 0x89, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	devcb_read_line m_dock_cb;
	required_ioport m_directions;
	required_ioport m_buttons;
	required_ioport m_remote_reset;
	required_ioport m_factory_test;
};

DECLARE_DEVICE_TYPE(XAVIX_CARDNIGHT_REMOTE, xavix_cardnight_remote_device)

#endif // MAME_TVGAMES_XAVIX_IO_CARDNIGHT_H
