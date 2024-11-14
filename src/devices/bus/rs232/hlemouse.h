// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**************************************************************************

    PC Serial Mouse Simulation

**************************************************************************/
#ifndef MAME_BUS_RS232_HLEMOUSE_H
#define MAME_BUS_RS232_HLEMOUSE_H

#pragma once

#include "rs232.h"
#include "diserial.h"

#include <type_traits>


namespace bus::rs232 {

//**************************************************
// Microsoft mouse base
//**************************************************

class hle_msmouse_device_base : public buffered_rs232_device<8>
{
public:
	DECLARE_INPUT_CHANGED_MEMBER(input_changed);

protected:
	hle_msmouse_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, uint32_t clock);

	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	virtual void input_dtr(int state) override;
	virtual void input_rts(int state) override;

	virtual void tra_complete() override;

	virtual void received_byte(u8 byte) override;

	virtual bool read_inputs();

private:
	TIMER_CALLBACK_MEMBER(start_mouse);
	void check_enable();
	void check_inputs();

	virtual void reset_and_identify() = 0;
	virtual void transmit_extensions(uint8_t btn_val, uint8_t btn_sent) = 0;

	required_ioport m_buttons, m_x_axis, m_y_axis;
	int16_t m_x_delta, m_y_delta;
	uint16_t m_x_val, m_y_val;
	uint8_t m_btn_val, m_btn_sent;
	uint8_t m_dtr, m_rts, m_enable;
};


//**************************************************
// Microsoft 2-button mouse
//**************************************************

class hle_msft_mouse_device : public hle_msmouse_device_base
{
public:
	hle_msft_mouse_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	virtual void reset_and_identify() override;
	virtual void transmit_extensions(uint8_t btn_val, uint8_t btn_sent) override;
};


//**************************************************
//  Logitech 3-button mouse
//**************************************************

class hle_logitech_mouse_device : public hle_msmouse_device_base
{
public:
	hle_logitech_mouse_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	virtual void reset_and_identify() override;
	virtual void transmit_extensions(uint8_t btn_val, uint8_t btn_sent) override;
};


//**************************************************
// Microsoft wheel mouse
//**************************************************

class hle_wheel_mouse_device : public hle_msmouse_device_base
{
public:
	hle_wheel_mouse_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	virtual bool read_inputs() override;

private:
	virtual void reset_and_identify() override;
	virtual void transmit_extensions(uint8_t btn_val, uint8_t btn_sent) override;

	required_ioport m_wheel;
	int16_t m_wheel_delta;
	uint16_t m_wheel_val;
};


//**************************************************
//  Mouse Systems mouse base
//**************************************************

class hle_msystems_device_base : public device_t, public device_rs232_port_interface, public device_serial_interface
{
public:
	DECLARE_INPUT_CHANGED_MEMBER(input_changed);

protected:
	hle_msystems_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;

	virtual void tra_callback() override;
	virtual void tra_complete() override;

private:
	TIMER_CALLBACK_MEMBER(start_mouse);

	virtual bool read_inputs() = 0;
	virtual uint8_t report_buttons() = 0;
	virtual uint8_t report_x1_delta() = 0;
	virtual uint8_t report_y1_delta() = 0;
	virtual uint8_t report_x2_delta() = 0;
	virtual uint8_t report_y2_delta() = 0;

	uint8_t m_phase;
};


//**************************************************
//  Mouse Systems non-rotatable mouse
//**************************************************

class hle_msystems_mouse_device : public hle_msystems_device_base
{
public:
	hle_msystems_mouse_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

protected:
	hle_msystems_mouse_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	virtual bool read_inputs() override;
	virtual uint8_t report_buttons() override;
	virtual uint8_t report_x1_delta() override;
	virtual uint8_t report_y1_delta() override;
	virtual uint8_t report_x2_delta() override;
	virtual uint8_t report_y2_delta() override;

	required_ioport m_buttons, m_x_axis, m_y_axis;
	int16_t m_x_delta, m_y_delta;
	uint16_t m_x_val, m_y_val;
	uint8_t m_btn_val, m_btn_sent;
};


//**************************************************
//  Mouse Systems rotatable mouse
//**************************************************

class hle_rotatable_mouse_device : public hle_msystems_device_base
{
public:
	hle_rotatable_mouse_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	virtual bool read_inputs() override;
	virtual uint8_t report_buttons() override;
	virtual uint8_t report_x1_delta() override;
	virtual uint8_t report_y1_delta() override;
	virtual uint8_t report_x2_delta() override;
	virtual uint8_t report_y2_delta() override;

	required_ioport m_buttons, m_x_axis, m_y_axis, m_rotation;
	int16_t m_x_delta[2], m_y_delta[2];
	uint16_t m_x_val, m_y_val, m_rot_val;
	uint8_t m_btn_val, m_btn_sent;
};

//**************************************************
//  SGI IRIS Indigo mouse
//**************************************************

class hle_sgi_mouse_device : public hle_msystems_mouse_device
{
public:
	hle_sgi_mouse_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};

} // namespace bus::rs232


//**************************************************
//  Device type globals
//**************************************************

DECLARE_DEVICE_TYPE_NS(MSFT_HLE_SERIAL_MOUSE,      bus::rs232, hle_msft_mouse_device)
DECLARE_DEVICE_TYPE_NS(LOGITECH_HLE_SERIAL_MOUSE,  bus::rs232, hle_logitech_mouse_device)
DECLARE_DEVICE_TYPE_NS(WHEEL_HLE_SERIAL_MOUSE,     bus::rs232, hle_wheel_mouse_device)
DECLARE_DEVICE_TYPE_NS(MSYSTEMS_HLE_SERIAL_MOUSE,  bus::rs232, hle_msystems_mouse_device)
DECLARE_DEVICE_TYPE_NS(ROTATABLE_HLE_SERIAL_MOUSE, bus::rs232, hle_rotatable_mouse_device)
DECLARE_DEVICE_TYPE_NS(SGI_HLE_SERIAL_MOUSE,       bus::rs232, hle_sgi_mouse_device)

#endif // MAME_BUS_RS232_SER_MOUSE_H
