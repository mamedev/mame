// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_TFIT_RACKETS_H
#define MAME_TVGAMES_XAVIX_IO_TFIT_RACKETS_H

#pragma once

#include "xavix_io_ir.h"

class xavix_tfit_rackets_device : public device_t
{
public:
	xavix_tfit_rackets_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void set_input_source(device_t &source) ATTR_COLD;
	auto event_callback() { return m_ir.lookup()->event_callback(); }
	int data_r() { return m_ir->data_r(); }
	void vblank_w(int state);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	template <unsigned Player> u32 racket_report_r();

	required_device<xavix_ir_report_device> m_ir;
	required_ioport_array<2> m_racket_strength;
	required_ioport_array<2> m_racket_buttons;
	required_ioport m_racket_power;
};

DECLARE_DEVICE_TYPE(XAVIX_TFIT_RACKETS, xavix_tfit_rackets_device)

INPUT_PORTS_EXTERN( xavix_tfit_rackets );

#endif // MAME_TVGAMES_XAVIX_IO_TFIT_RACKETS_H
