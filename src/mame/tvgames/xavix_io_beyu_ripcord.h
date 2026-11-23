// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_BEYU_RIPCORD_H
#define MAME_TVGAMES_XAVIX_IO_BEYU_RIPCORD_H

#pragma once

#include "xavix_io_ir.h"

class xavix_beyblade_ripcord_device : public xavix_optical_pulse_device_base
{
public:
	xavix_beyblade_ripcord_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void vblank_w(int state);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_ioport m_position;
	u8 m_last_position;
	bool m_position_valid;
};

DECLARE_DEVICE_TYPE(XAVIX_BEYBLADE_RIPCORD, xavix_beyblade_ripcord_device)

#endif // MAME_TVGAMES_XAVIX_IO_BEYU_RIPCORD_H
