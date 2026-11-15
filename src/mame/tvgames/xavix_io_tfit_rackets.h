// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_TFIT_RACKETS_H
#define MAME_TVGAMES_XAVIX_IO_TFIT_RACKETS_H

#pragma once

#include "xavix_io_ir.h"

class xavix_tfit_rackets_device : public xavix_ir_msb_device
{
public:
	xavix_tfit_rackets_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual u32 read_report(unsigned report) override;

private:
	required_ioport_array<2> m_racket_strength;
	required_ioport_array<2> m_racket_buttons;
	required_ioport m_racket_power;
};

DECLARE_DEVICE_TYPE(XAVIX_TFIT_RACKETS, xavix_tfit_rackets_device)

#endif // MAME_TVGAMES_XAVIX_IO_TFIT_RACKETS_H
