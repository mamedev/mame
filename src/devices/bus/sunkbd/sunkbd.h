// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_DEVICES_SUNKBD_SUNKBD_H
#define MAME_DEVICES_SUNKBD_SUNKBD_H

#pragma once

#include "emu.h"


#define MCFG_SUNKBD_PORT_ADD(tag, slot_intf, def_slot) \
	MCFG_DEVICE_ADD(tag, SUNKBD_PORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(slot_intf, def_slot, false)

#define MCFG_SUNKBD_RXD_HANDLER(cb) \
	devcb = &sun_keyboard_port_device::set_rxd_handler(*device, DEVCB_##cb);


class device_sun_keyboard_port_interface;


class sun_keyboard_port_device : public device_t, public device_slot_interface
{
	friend class device_sun_keyboard_port_interface;

public:
	sun_keyboard_port_device(machine_config const &mconfig, char const *tag, device_t *owner, UINT32 clock);
	sun_keyboard_port_device(machine_config const &mconfig, device_type type, char const *name, char const *tag, device_t *owner, UINT32 clock, char const *shortname, char const *source);
	virtual ~sun_keyboard_port_device();

	// static configuration helpers
	template <class Object> static devcb_base &set_rxd_handler(device_t &device, Object object) { return downcast<sun_keyboard_port_device &>(device).m_rxd_handler.set_callback(object); }

	DECLARE_WRITE_LINE_MEMBER( write_txd );

	DECLARE_READ_LINE_MEMBER( rxd_r ) { return m_rxd; }

protected:
	virtual void device_start() override;
	virtual void device_config_complete() override;

	int m_rxd;

	devcb_write_line m_rxd_handler;

private:
	device_sun_keyboard_port_interface *m_dev;
};


class device_sun_keyboard_port_interface : public device_slot_card_interface
{
	friend class sun_keyboard_port_device;

public:
	device_sun_keyboard_port_interface(machine_config const &mconfig, device_t &device);
	virtual ~device_sun_keyboard_port_interface() override;

	virtual DECLARE_WRITE_LINE_MEMBER( input_txd ) { }

	DECLARE_WRITE_LINE_MEMBER( output_rxd ) { m_port->m_rxd = state; m_port->m_rxd_handler(state); }

protected:
	sun_keyboard_port_device *m_port;

	static constexpr int START_BIT_COUNT = 1;
	static constexpr int DATA_BIT_COUNT = 8;
	static constexpr device_serial_interface::parity_t PARITY = device_serial_interface::PARITY_NONE;
	static constexpr device_serial_interface::stop_bits_t STOP_BITS = device_serial_interface::STOP_BITS_1;
	static constexpr int BAUD = 1'200;
};


extern device_type const SUNKBD_PORT;


SLOT_INTERFACE_EXTERN( default_sun_keyboard_devices );

#endif // MAME_DEVICES_SUNKBD_SUNKBD_H
