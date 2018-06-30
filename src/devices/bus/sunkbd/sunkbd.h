// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_BUS_SUNKBD_SUNKBD_H
#define MAME_BUS_SUNKBD_SUNKBD_H

#pragma once

#include "diserial.h"


#define MCFG_SUNKBD_RXD_HANDLER(cb) \
	devcb = &downcast<sun_keyboard_port_device &>(*device).set_rxd_handler(DEVCB_##cb);


class device_sun_keyboard_port_interface;


class sun_keyboard_port_device : public device_t, public device_slot_interface
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

	// static configuration helpers
	template <class Object> devcb_base &set_rxd_handler(Object &&cb) { return m_rxd_handler.set_callback(std::forward<Object>(cb)); }

	DECLARE_WRITE_LINE_MEMBER( write_txd );

	DECLARE_READ_LINE_MEMBER( rxd_r ) { return m_rxd; }

protected:
	sun_keyboard_port_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, uint32_t clock);

	virtual void device_resolve_objects() override;
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
	virtual ~device_sun_keyboard_port_interface() override;

	virtual DECLARE_WRITE_LINE_MEMBER( input_txd ) { }

	DECLARE_WRITE_LINE_MEMBER( output_rxd ) { m_port->m_rxd = state; m_port->m_rxd_handler(state); }

protected:
	device_sun_keyboard_port_interface(machine_config const &mconfig, device_t &device);

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
