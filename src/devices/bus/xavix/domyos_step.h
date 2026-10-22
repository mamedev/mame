// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_BUS_XAVIX_DOMYOS_STEP_H
#define MAME_BUS_XAVIX_DOMYOS_STEP_H

#pragma once

#include <array>
#include <utility>

class device_domyos_step_interface;

class domyos_step_port_device : public device_t, public device_single_card_slot_interface<device_domyos_step_interface>
{
public:
	domyos_step_port_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template <typename... T> void set_event_callback(T &&... args) { m_event_cb.set(std::forward<T>(args)...); }
	u16 report_r();
	u8 pio_r();
	void pio_w(offs_t offset, u8 data, u8 direction);
	void trigger_event();

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	device_delegate<void ()> m_event_cb;
	device_domyos_step_interface *m_peripheral;
};

class device_domyos_step_interface : public device_interface
{
public:
	virtual u16 report_r();
	virtual u8 pio_r();
	virtual void pio_w(u8 data, u8 direction);

protected:
	device_domyos_step_interface(const machine_config &mconfig, device_t &device);

	void trigger_event();

private:
	domyos_step_port_device *const m_port;
};

class domyos_step_mat_device : public device_t, public device_domyos_step_interface
{
public:
	enum class key_layout { SINGLE, PLAYER1, PLAYER2 };

	domyos_step_mat_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void set_key_layout(key_layout layout) { m_key_layout = layout; }
	key_layout get_key_layout() const { return m_key_layout; }
	u8 contacts_r();

	// device_domyos_step_interface implementation
	virtual u16 report_r() override;

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	required_ioport m_contacts;
	key_layout m_key_layout;
};

class domyos_step_hub_device : public device_t, public device_domyos_step_interface
{
public:
	domyos_step_hub_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// device_domyos_step_interface implementation
	virtual u8 pio_r() override;
	virtual void pio_w(u8 data, u8 direction) override;

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	TIMER_CALLBACK_MEMBER(event_timer);
	void latch_report();
	void command_bit_w(int state);

	required_device_array<domyos_step_mat_device, 2> m_mats;
	emu_timer *m_event_timer;
	std::array<u8, 5> m_report;
	std::array<u8, 3> m_command;
	u8 m_bit;
	u8 m_command_bits;
	u8 m_report_bytes;
	bool m_request;
	bool m_request_output;
	bool m_clock;
	bool m_host_transmit;
	bool m_active;
	bool m_transfer;
};

DECLARE_DEVICE_TYPE(DOMYOS_STEP_PORT, domyos_step_port_device)
DECLARE_DEVICE_TYPE(DOMYOS_STEP_MAT, domyos_step_mat_device)
DECLARE_DEVICE_TYPE(DOMYOS_STEP_HUB, domyos_step_hub_device)

#endif // MAME_BUS_XAVIX_DOMYOS_STEP_H
