// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Patrick Mackinlay

#ifndef MAME_SGI_KBD_H
#define MAME_SGI_KBD_H

#pragma once

#include "cpu/mcs51/mcs51.h"

class device_sgi_kbd_port_interface;

class sgi_kbd_port_device
	: public device_t
	, public device_single_card_slot_interface<device_sgi_kbd_port_interface>
{
	friend class device_sgi_kbd_port_interface;

public:
	template <typename T>
	sgi_kbd_port_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: sgi_kbd_port_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	sgi_kbd_port_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0);
	virtual ~sgi_kbd_port_device();

	auto rxd_handler() { return m_rxd_handler.bind(); }
	void write_txd(int state);

protected:
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;

private:
	devcb_write_line m_rxd_handler;
	device_sgi_kbd_port_interface *m_kbd;
};

class device_sgi_kbd_port_interface
	: public device_interface
{
public:
	virtual void write_txd(int state) = 0;

protected:
	device_sgi_kbd_port_interface(machine_config const &mconfig, device_t &device);
	virtual ~device_sgi_kbd_port_interface() override;

	void write_rxd(int state) { m_port->m_rxd_handler(state); }

private:
	sgi_kbd_port_device *m_port;
};

class sgi_kbd_device
	: public device_t
	, public device_sgi_kbd_port_interface
{
public:
	sgi_kbd_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

	virtual void write_txd(int state) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual tiny_rom_entry const *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	virtual void map_mem(address_map &map) ATTR_COLD;
	virtual void map_pio(address_map &map) ATTR_COLD;

	void scan_matrix(int state);
	void led_w(u8 data);

	required_device<i8031_device> m_mcu;
	required_ioport_array<16> m_matrix;
	output_finder<7> m_led;

	u8 m_col; // selected matrix column
	u8 m_row; // selected matrix row
	u8 m_p3;  // mcu port 3 input data
};

DECLARE_DEVICE_TYPE(SGI_KBD_PORT, sgi_kbd_port_device)
DECLARE_DEVICE_TYPE(SGI_KBD, sgi_kbd_device)

void default_sgi_kbd_devices(device_slot_interface &device);

#endif // MAME_SGI_KBD_H
