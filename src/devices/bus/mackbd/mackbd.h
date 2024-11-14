// license: BSD-3-Clause
// copyright-holders: Vas Crabb
/***************************************************************************

    Mac 128k/512k/Plus keyboard interface (pre-ADB)

    4p4c RJ11 connector (cables wired straight through):
    1: power/signal ground (black)
    2: clock (red, peripheral to host)
    3: data (green, bidirectional)
    4: +5V power (yellow, maximum 200mA for all peripherals)

***************************************************************************/
#ifndef MAME_BUS_MACKBD_MACKBD_H
#define MAME_BUS_MACKBD_MACKBD_H

#pragma once


class device_mac_keyboard_interface;

DECLARE_DEVICE_TYPE(MAC_KEYBOARD_PORT, mac_keyboard_port_device)

void mac_keyboard_devices(device_slot_interface &device);


//**************************************************************************
//  HOST PORT
//**************************************************************************

class mac_keyboard_port_device : public device_t, public device_single_card_slot_interface<device_mac_keyboard_interface>
{
public:
	template <typename T>
	mac_keyboard_port_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: mac_keyboard_port_device(mconfig, tag, owner, 0U)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	mac_keyboard_port_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0U);
	virtual ~mac_keyboard_port_device() override;

	auto clock_cb() { return m_clock_cb.bind(); }
	auto data_cb() { return m_data_cb.bind(); }

	void data_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;

private:
	devcb_write_line m_clock_cb;
	devcb_write_line m_data_cb;

	device_mac_keyboard_interface *m_peripheral;

	friend class device_mac_keyboard_interface;
};


//**************************************************************************
//  PERIPHERAL INTERFACE
//**************************************************************************

class device_mac_keyboard_interface : public device_interface
{
public:
	virtual ~device_mac_keyboard_interface() override;

protected:
	device_mac_keyboard_interface(machine_config const &mconfig, device_t &device);

	virtual void interface_validity_check(validity_checker &valid) const override;

	void write_clock(int state) { if (m_host) m_host->m_clock_cb(state); }
	void write_data(int state) { if (m_host) m_host->m_data_cb(state); }

	virtual void data_w(int state) = 0;

private:
	mac_keyboard_port_device *const m_host;

	friend class mac_keyboard_port_device;
};


#endif // MAME_BUS_MACKBD_MACKBD_H
