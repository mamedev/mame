// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_EBOX_GLOVES_H
#define MAME_TVGAMES_XAVIX_IO_EBOX_GLOVES_H

#pragma once

#include "xavix_io_ir.h"

class xavix_ebox_gloves_device : public device_t
{
public:
	xavix_ebox_gloves_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void set_input_source(device_t &source) ATTR_COLD;
	auto event_callback() { return m_ir.lookup()->event_callback(); }
	int data_r() { return m_ir->data_r(); }
	void vblank_w(int state);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	template <unsigned Glove> u16 glove_report_r();
	template <unsigned Glove> int glove_enabled_r();
	int ir_enable_r();

	required_device<xavix_ir_report_device> m_ir;
	required_ioport m_punch;
	required_ioport m_guard;
	required_ioport m_glove_power;
};

DECLARE_DEVICE_TYPE(XAVIX_EBOX_GLOVES, xavix_ebox_gloves_device)

INPUT_PORTS_EXTERN( xavix_ebox_gloves );

#endif // MAME_TVGAMES_XAVIX_IO_EBOX_GLOVES_H
