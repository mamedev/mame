// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay
#ifndef MAME_BUS_INTERPRO_KEYBOARD_KEYBOARD_H
#define MAME_BUS_INTERPRO_KEYBOARD_KEYBOARD_H

#pragma once

#define MCFG_INTERPRO_KEYBOARD_PORT_ADD(tag, slot_intf, def_slot) \
	MCFG_DEVICE_ADD(tag, INTERPRO_KEYBOARD_PORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(slot_intf, def_slot, false)

#define MCFG_INTERPRO_KEYBOARD_RXD_HANDLER(cb) \
	devcb = &downcast<interpro_keyboard_port_device &>(*device).set_rxd_handler(DEVCB_##cb);

class device_interpro_keyboard_port_interface;

class interpro_keyboard_port_device : public device_t, public device_slot_interface
{
	friend class device_interpro_keyboard_port_interface;

public:
	interpro_keyboard_port_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);
	virtual ~interpro_keyboard_port_device();

	// static configuration helpers
	template <class Object> devcb_base &set_rxd_handler(Object &&cb) { return m_rxd_handler.set_callback(std::forward<Object>(cb)); }

	DECLARE_WRITE_LINE_MEMBER(write_txd);

protected:
	interpro_keyboard_port_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_config_complete() override;

	int m_rxd;

	devcb_write_line m_rxd_handler;

private:
	device_interpro_keyboard_port_interface *m_dev;
};

class device_interpro_keyboard_port_interface : public device_slot_card_interface
{
	friend class interpro_keyboard_port_device;

public:
	virtual ~device_interpro_keyboard_port_interface() override;

	virtual DECLARE_WRITE_LINE_MEMBER(input_txd) { }

	DECLARE_WRITE_LINE_MEMBER(output_rxd) { m_port->m_rxd = state; m_port->m_rxd_handler(state); }

protected:
	device_interpro_keyboard_port_interface(machine_config const &mconfig, device_t &device);

	interpro_keyboard_port_device *m_port;
};

DECLARE_DEVICE_TYPE(INTERPRO_KEYBOARD_PORT, interpro_keyboard_port_device)

void interpro_keyboard_devices(device_slot_interface &device);

#endif // MAME_BUS_INTERPRO_KEYBOARD_KEYBOARD_H
