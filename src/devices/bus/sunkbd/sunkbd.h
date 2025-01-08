// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_BUS_SUNKBD_SUNKBD_H
#define MAME_BUS_SUNKBD_SUNKBD_H

#pragma once

#include "diserial.h"


class device_sun_keyboard_port_interface;


class sun_keyboard_port_device : public device_t, public device_single_card_slot_interface<device_sun_keyboard_port_interface>
{
	friend class device_sun_keyboard_port_interface;

public:
	template <typename T>
	sun_keyboard_port_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: sun_keyboard_port_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	sun_keyboard_port_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0);
	virtual ~sun_keyboard_port_device();

	// configuration helpers
	auto rxd_handler() { return m_rxd_handler.bind(); }

	void write_txd(int state);

	int rxd_r() { return m_rxd; }

protected:
	sun_keyboard_port_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, uint32_t clock);

	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;

	int m_rxd;

	devcb_write_line m_rxd_handler;

private:
	device_sun_keyboard_port_interface *m_dev;
};


class device_sun_keyboard_port_interface : public device_interface
{
	friend class sun_keyboard_port_device;

public:
	virtual ~device_sun_keyboard_port_interface() override;

protected:
	device_sun_keyboard_port_interface(machine_config const &mconfig, device_t &device);

	virtual void input_txd(int state) { }

	void output_rxd(int state) { m_port->m_rxd = state; m_port->m_rxd_handler(state); }

	sun_keyboard_port_device *m_port;

	static constexpr int START_BIT_COUNT = 1;
	static constexpr int DATA_BIT_COUNT = 8;
	static constexpr device_serial_interface::parity_t PARITY = device_serial_interface::PARITY_NONE;
	static constexpr device_serial_interface::stop_bits_t STOP_BITS = device_serial_interface::STOP_BITS_1;
	static constexpr int BAUD = 1'200;
};


DECLARE_DEVICE_TYPE(SUNKBD_PORT, sun_keyboard_port_device)


void default_sun_keyboard_devices(device_slot_interface &device);

#endif // MAME_DEVICES_SUNKBD_SUNKBD_H
