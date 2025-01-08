// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_BUS_RTPC_KBD_CON_H
#define MAME_BUS_RTPC_KBD_CON_H

#pragma once

class device_rtpc_kbd_interface;

class rtpc_kbd_con_device
	: public device_t
	, public device_single_card_slot_interface<device_rtpc_kbd_interface>
{
public:
	template <typename T>
	rtpc_kbd_con_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: rtpc_kbd_con_device(mconfig, tag, owner, 0U)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	rtpc_kbd_con_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0U);

	auto out_clock_cb() { return m_out_clock_cb.bind(); }
	auto out_data_cb() { return m_out_data_cb.bind(); }

	int clock_signal() { return m_clock_state; }
	int data_signal() { return m_data_state; }

	void clock_write_from_mb(int state);
	void data_write_from_mb(int state);
	void clock_write_from_kb(int state);
	void data_write_from_kb(int state);

protected:
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;

	void update_clock_state(bool fromkb);
	void update_data_state(bool fromkb);

	devcb_write_line m_out_clock_cb;
	devcb_write_line m_out_data_cb;

	int8_t m_clock_state;
	int8_t m_data_state;

	uint8_t m_mb_clock_state;
	uint8_t m_mb_data_state;
	uint8_t m_kb_clock_state;
	uint8_t m_kb_data_state;

	device_rtpc_kbd_interface *m_keyboard;
};

DECLARE_DEVICE_TYPE(RTPC_KBD_CON, rtpc_kbd_con_device)

class device_rtpc_kbd_interface : public device_interface
{
	friend class rtpc_kbd_port_device;
public:
	virtual ~device_rtpc_kbd_interface() {}

	virtual void clock_w(int state) {}
	virtual void data_w(int state) {}

protected:
	device_rtpc_kbd_interface(machine_config const &mconfig, device_t &device);

	int clock_signal() const { return m_port->clock_signal(); }
	int data_signal() const { return m_port->data_signal(); }

	rtpc_kbd_con_device *m_port;
};

#endif // MAME_BUS_RTPC_KBD_CON_H
