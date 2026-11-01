// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_BUS_XAVIX_BEYBLADE_LINK_H
#define MAME_BUS_XAVIX_BEYBLADE_LINK_H

#pragma once

#include <array>

class device_xavix_beyblade_link_interface;

class xavix_beyblade_link_port_device : public device_t, public device_single_card_slot_interface<device_xavix_beyblade_link_interface>
{
public:
	xavix_beyblade_link_port_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto controls_callback() { return m_controls_cb.bind(); }
	auto host_reset_callback() { return m_host_reset_cb.bind(); }
	u8 io_r(offs_t offset, u8 direction);
	void io_w(offs_t offset, u8 levels, u8 direction);
	void vblank_w(int state);
	bool host_reset_r() { return m_host_reset_cb(); }

protected:
	virtual void device_start() override ATTR_COLD;

private:
	devcb_read8 m_controls_cb;
	devcb_read_line m_host_reset_cb;
	device_xavix_beyblade_link_interface *m_peer;
};

class device_xavix_beyblade_link_interface : public device_interface
{
public:
	virtual u8 lines_r() = 0;
	virtual void lines_w(u8 levels, u8 direction) = 0;
	virtual void vblank_w(int state) = 0;

protected:
	device_xavix_beyblade_link_interface(const machine_config &mconfig, device_t &device);

	bool host_reset_r() { return m_port && m_port->host_reset_r(); }

private:
	xavix_beyblade_link_port_device *const m_port;
};

class xavix_beyblade_peer_device : public device_t, public device_xavix_beyblade_link_interface
{
public:
	enum class model : u8 { A55 = 1, A62, FACTORY };
	void set_model(model type) { m_model = type; }

	virtual u8 lines_r() override { return m_peer_lines; }
	virtual void lines_w(u8 levels, u8 direction) override;
	virtual void vblank_w(int state) override;

protected:
	xavix_beyblade_peer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, model variant);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	enum : u8
	{
		TEST_CONNECT,
		TEST_CONNECT_RELEASE,
		WAIT_CONNECT,
		CONNECT_RELEASE,
		RECEIVE_ASSERT,
		RECEIVE_RELEASE,
		TRANSMIT_TURNAROUND,
		TRANSMIT_DELAY,
		TRANSMIT_ACK,
		TRANSMIT_RELEASE
	};

	enum : u8
	{
		FACTORY_ENTRY,
		FACTORY_LINK_CHECK,
		FACTORY_REPLY,
		FACTORY_READY
	};

	static constexpr std::array<u8, 35> RPM_FILTER =
	{
		0x02, 0x06, 0x0d, 0x17, 0x24, 0x32, 0x42, 0x53, 0x64, 0x75, 0x86, 0x96,
		0xa4, 0xb1, 0xbb, 0xc2, 0xc6, 0xc8, 0xc6, 0xc2, 0xbb, 0xb1, 0xa4, 0x96,
		0x86, 0x75, 0x64, 0x53, 0x42, 0x32, 0x24, 0x17, 0x0d, 0x06, 0x02
	};

	TIMER_CALLBACK_MEMBER(transmit_timer);
	TIMER_CALLBACK_MEMBER(idle_timer);
	void reset_protocol();
	void prepare_response(u8 command);
	void begin_transmit_turnaround();
	void advance_transmit();

	model m_model;
	required_ioport m_peer_buttons;
	required_ioport m_peer_dial;
	required_ioport m_peer_ripcord;
	emu_timer *m_transmit_timer;
	emu_timer *m_idle_timer;

	std::array<u8, 7> m_response;
	u16 m_peer_rpm;
	u8 m_phase;
	u8 m_host_lines;
	u8 m_host_direction;
	u8 m_peer_lines;
	u8 m_receive_byte;
	u8 m_receive_bits;
	u8 m_response_bytes;
	u8 m_response_byte;
	u8 m_response_bit;
	u8 m_peer_button_levels;
	u8 m_peer_button_edges;
	u8 m_last_peer_buttons;
	u8 m_peer_dial_position;
	u8 m_peer_dial_delta;
	u8 m_peer_ripcord_position;
	std::array<u8, RPM_FILTER.size()> m_peer_ripcord_history;
	u8 m_peer_ripcord_history_position;
	bool m_peer_inputs_valid;
	bool m_expect_diagnostic_token;
	u8 m_factory_stage;
};

class xavix_beyblade_a55_device : public xavix_beyblade_peer_device
{
public:
	xavix_beyblade_a55_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

class xavix_beyblade_a62_device : public xavix_beyblade_peer_device
{
public:
	xavix_beyblade_a62_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

DECLARE_DEVICE_TYPE(XAVIX_BEYBLADE_LINK_PORT, xavix_beyblade_link_port_device)
DECLARE_DEVICE_TYPE(XAVIX_BEYBLADE_A55, xavix_beyblade_a55_device)
DECLARE_DEVICE_TYPE(XAVIX_BEYBLADE_A62, xavix_beyblade_a62_device)

#endif // MAME_BUS_XAVIX_BEYBLADE_LINK_H
