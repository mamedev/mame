// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_EBOX_GLOVES_H
#define MAME_TVGAMES_XAVIX_IO_EBOX_GLOVES_H

#pragma once

#include "xavix_io_ir_motion.h"

class xavix_ebox_gloves_device : public xavix_ir_motion_base_device
{
public:
	xavix_ebox_gloves_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual u32 read_report(unsigned report) override;
	virtual bool reports_enabled() override;
	virtual bool report_enabled(unsigned report) override;

private:
	required_ioport m_punch;
	required_ioport m_guard;
	required_ioport m_glove_power;
};

DECLARE_DEVICE_TYPE(XAVIX_EBOX_GLOVES, xavix_ebox_gloves_device)

#endif // MAME_TVGAMES_XAVIX_IO_EBOX_GLOVES_H
