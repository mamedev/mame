// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC 800/802/806/1600 keyboard port emulation

**********************************************************************/

#ifndef MAME_BUS_ABCKB_ABCKB_H
#define MAME_BUS_ABCKB_ABCKB_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class abc_keyboard_interface;

class abc_keyboard_port_device : public device_t, public device_single_card_slot_interface<abc_keyboard_interface>
{
public:
	// construction/destruction
	template <typename T>
	abc_keyboard_port_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: abc_keyboard_port_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	abc_keyboard_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto out_rx_handler() { return m_out_rx_handler.bind(); }
	auto out_trxc_handler() { return m_out_trxc_handler.bind(); }
	auto out_keydown_handler() { return m_out_keydown_handler.bind(); }

	// computer interface
	void txd_w(int state);

	// peripheral interface
	void write_rx(int state);
	void trxc_w(int state);
	void keydown_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	devcb_write_line m_out_rx_handler;
	devcb_write_line m_out_trxc_handler;
	devcb_write_line m_out_keydown_handler;

	abc_keyboard_interface *m_card;
};


class abc_keyboard_interface : public device_interface
{
public:
	virtual void txd_w(int state) { }
	virtual void reset_w(int state) { }

protected:
	// construction/destruction
	abc_keyboard_interface(const machine_config &mconfig, device_t &device);

	abc_keyboard_port_device *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(ABC_KEYBOARD_PORT, abc_keyboard_port_device)


// supported devices
void abc800_keyboard_devices(device_slot_interface &device);
void abc_keyboard_devices(device_slot_interface &device);

#endif // MAME_BUS_ABCKB_ABCKB_H
