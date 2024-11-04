// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_DEVICES_SUNMOUSE_HLEMOUSE_H
#define MAME_DEVICES_SUNMOUSE_HLEMOUSE_H

#pragma once

#include "sunmouse.h"
#include "diserial.h"


namespace bus::sunmouse {

class hle_device_base
	: public device_t
	, public device_serial_interface
	, public device_sun_mouse_port_interface
{
public:
	virtual ~hle_device_base() override;

	DECLARE_INPUT_CHANGED_MEMBER( input_changed );

protected:
	// constructor/destructor
	hle_device_base(
			machine_config const &mconfig,
			device_type type,
			char const *tag,
			device_t *owner,
			uint32_t clock,
			unsigned multiplier);

	// device overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_serial_interface overrides
	virtual void tra_callback() override;
	virtual void tra_complete() override;

private:
	void check_inputs();

	required_ioport m_buttons, m_x_axis, m_y_axis;
	unsigned const m_multiplier;
	int32_t m_x_delta, m_y_delta;
	uint16_t m_x_val, m_y_val;
	uint8_t m_btn_sent, m_btn_val;
	uint8_t m_phase;
};


class hle_1200baud_device : public hle_device_base
{
public:
	hle_1200baud_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			uint32_t clock);
};


class hle_4800baud_device : public hle_device_base
{
public:
	hle_4800baud_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			uint32_t clock);
};

} // namespace bus::sunmouse


DECLARE_DEVICE_TYPE_NS(SUN_1200BAUD_HLE_MOUSE, bus::sunmouse, hle_1200baud_device)
DECLARE_DEVICE_TYPE_NS(SUN_4800BAUD_HLE_MOUSE, bus::sunmouse, hle_4800baud_device)

#endif // MAME_DEVICES_SUNMOUSE_HLEMOUSE_H
