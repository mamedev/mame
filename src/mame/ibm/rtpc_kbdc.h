// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_IBM_RTPC_KBDC_H
#define MAME_IBM_RTPC_KBDC_H

#pragma once

class device_rtpc_kbd_interface;

class rtpc_kbdc_device
	: public device_t
	, public device_single_card_slot_interface<device_rtpc_kbd_interface>
{
public:
	template <typename T>
	rtpc_kbdc_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: rtpc_kbdc_device(mconfig, tag, owner, 0U)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	rtpc_kbdc_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0U);

	auto out_clock_cb() { return m_out_clock_cb.bind(); }
	auto out_data_cb() { return m_out_data_cb.bind(); }

	int clock_signal() { return m_clock_state; }
	int data_signal() { return m_data_state; }

	void clock_write_from_mb(int state);
	void data_write_from_mb(int state);
	void clock_write_from_kb(int state);
	void data_write_from_kb(int state);

protected:
	virtual void device_config_complete() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	void update_clock_state(bool fromkb);
	void update_data_state(bool fromkb);

private:
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

class device_rtpc_kbd_interface : public device_interface
{
public:
	virtual ~device_rtpc_kbd_interface() {}

	virtual void clock_w(int state) {}
	virtual void data_w(int state) {}

protected:
	device_rtpc_kbd_interface(machine_config const &mconfig, device_t &device);

	int clock_signal() const { return m_port->clock_signal(); }
	int data_signal() const { return m_port->data_signal(); }

	rtpc_kbdc_device *m_port;
};

DECLARE_DEVICE_TYPE(RTPC_KBDC, rtpc_kbdc_device)

#endif // MAME_IBM_RTPC_KBDC_H
