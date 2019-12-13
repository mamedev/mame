// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#ifndef DEVICES_BUS_SGIKBD_SGIKBD_H
#define DEVICES_BUS_SGIKBD_SGIKBD_H

#pragma once

#include "diserial.h"

class device_sgi_keyboard_port_interface;

class sgi_keyboard_port_device : public device_t, public device_single_card_slot_interface<device_sgi_keyboard_port_interface>
{
	friend class device_sgi_keyboard_port_interface;

public:
	template <typename T>
	sgi_keyboard_port_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: sgi_keyboard_port_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	sgi_keyboard_port_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0);
	virtual ~sgi_keyboard_port_device();

	// configuration helpers
	auto rxd_handler() { return m_rxd_handler.bind(); }

	DECLARE_WRITE_LINE_MEMBER( write_txd );

	DECLARE_READ_LINE_MEMBER( rxd_r ) { return m_rxd; }

protected:
	sgi_keyboard_port_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, uint32_t clock);

	virtual void device_config_complete() override;
	virtual void device_resolve_objects() override;
	virtual void device_start() override;

	int m_rxd;

	devcb_write_line m_rxd_handler;

private:
	device_sgi_keyboard_port_interface *m_dev;
};


class device_sgi_keyboard_port_interface : public device_interface
{
	friend class sgi_keyboard_port_device;

public:
	virtual ~device_sgi_keyboard_port_interface() override;

protected:
	device_sgi_keyboard_port_interface(machine_config const &mconfig, device_t &device);

	virtual DECLARE_WRITE_LINE_MEMBER(input_txd) { }

	DECLARE_WRITE_LINE_MEMBER(output_rxd) { m_port->m_rxd = state; m_port->m_rxd_handler(state); }

	sgi_keyboard_port_device *m_port;

	static constexpr int START_BIT_COUNT = 1;
	static constexpr int DATA_BIT_COUNT = 8;
	static constexpr device_serial_interface::parity_t PARITY = device_serial_interface::PARITY_ODD;
	static constexpr device_serial_interface::stop_bits_t STOP_BITS = device_serial_interface::STOP_BITS_1;
	static constexpr int BAUD = 600;
};


DECLARE_DEVICE_TYPE(SGIKBD_PORT, sgi_keyboard_port_device)


void default_sgi_keyboard_devices(device_slot_interface &device);

#endif // DEVICES_BUS_SGIKBD_SGIKBD_H
