// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_BEYU_LINK_H
#define MAME_TVGAMES_XAVIX_IO_BEYU_LINK_H

#pragma once

#include <array>

class xavix_beyblade_link_device : public device_t
{
public:
	xavix_beyblade_link_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto controls_callback() { return m_controls_cb.bind(); }
	auto host_reset_callback() { return m_host_reset_cb.bind(); }
	u8 io_r(offs_t offset, u8 direction);
	void io_w(offs_t offset, u8 levels, u8 direction);
	void vblank_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	enum : u8
	{
		DISCONNECTED,
		WAIT_CONNECT,
		CONNECT_RELEASE,
		RECEIVE_ASSERT,
		RECEIVE_RELEASE,
		TRANSMIT_TURNAROUND,
		TRANSMIT_DELAY,
		TRANSMIT_ACK,
		TRANSMIT_RELEASE
	};

	static constexpr std::array<u8, 35> RPM_FILTER =
	{
		0x02, 0x06, 0x0d, 0x17, 0x24, 0x32, 0x42, 0x53, 0x64, 0x75, 0x86, 0x96,
		0xa4, 0xb1, 0xbb, 0xc2, 0xc6, 0xc8, 0xc6, 0xc2, 0xbb, 0xb1, 0xa4, 0x96,
		0x86, 0x75, 0x64, 0x53, 0x42, 0x32, 0x24, 0x17, 0x0d, 0x06, 0x02
	};

	TIMER_CALLBACK_MEMBER(transmit_timer);
	TIMER_CALLBACK_MEMBER(idle_timer);
	bool sync_peer();
	void reset_protocol();
	void prepare_response(u8 command);
	void begin_transmit_turnaround();
	void advance_transmit();

	devcb_read8 m_controls_cb;
	devcb_read_line m_host_reset_cb;
	required_ioport m_peer_config;
	required_ioport m_peer_buttons;
	required_ioport m_peer_dial;
	required_ioport m_peer_ripcord;
	emu_timer *m_transmit_timer;
	emu_timer *m_idle_timer;

	std::array<u8, 7> m_response;
	u16 m_peer_rpm;
	u8 m_phase;
	u8 m_peer_selection;
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
};

DECLARE_DEVICE_TYPE(XAVIX_BEYBLADE_LINK, xavix_beyblade_link_device)

#endif // MAME_TVGAMES_XAVIX_IO_BEYU_LINK_H
