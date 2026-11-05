// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_STEP_MAT_H
#define MAME_TVGAMES_XAVIX_IO_STEP_MAT_H

#pragma once

class xavix_step_mat_device : public device_t
{
public:
	xavix_step_mat_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	u16 report_r();

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	required_ioport m_contacts;
	required_ioport m_power;
};

DECLARE_DEVICE_TYPE(XAVIX_STEP_MAT, xavix_step_mat_device)

INPUT_PORTS_EXTERN( xavix_step_sensor );

#endif // MAME_TVGAMES_XAVIX_IO_STEP_MAT_H
