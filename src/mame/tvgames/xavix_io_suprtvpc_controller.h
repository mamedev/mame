// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_SUPRTVPC_CONTROLLER_H
#define MAME_TVGAMES_XAVIX_IO_SUPRTVPC_CONTROLLER_H

#pragma once

#include "dirtc.h"

#include <array>

class xavix_tvpc_controller_device : public device_t, public device_rtc_interface
{
public:
	xavix_tvpc_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void set_input_source(device_t &source) ATTR_COLD;
	auto event_callback() { return m_event_out_cb.bind(); }

	u8 data_r() const { return m_data; }
	void host_w(u8 data, u8 direction);
	void vblank_w(int state);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_rtc_interface implementation
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

private:
	static constexpr unsigned QUEUE_SIZE = 128;
	static constexpr u8 DAYS_PER_MONTH[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	static constexpr u8 MONTH_OFFSETS[12] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
	static constexpr u8 PAYLOAD_BITS[8] = { 8, 8, 8, 8, 5, 5, 7, 8 };

	static u8 days_in_month(u8 year, u8 month);
	static u8 day_of_week(u8 year, u8 month, u8 day);

	TIMER_CALLBACK_MEMBER(event_timer);
	TIMER_CALLBACK_MEMBER(clock_timer);

	u64 read_keyboard();
	u32 packed_clock();
	void set_packed_clock(u32 packed);
	void scan_clock();
	void scan_inputs();
	bool queue_report(u8 type, u8 data);
	void start_report();
	void schedule_event();

	devcb_write_line m_event_out_cb;
	required_ioport_array<8> m_keys;
	optional_ioport m_right_shift;
	optional_ioport_array<2> m_players;
	emu_timer *m_event_timer;
	emu_timer *m_clock_timer;

	std::array<u16, QUEUE_SIZE> m_queue;
	u8 m_queue_head;
	u8 m_queue_count;
	u64 m_keyboard_state;
	std::array<u8, 2> m_player_state;
	u32 m_clock_state;
	u16 m_shift;
	u8 m_bits_remaining;
	u8 m_data;
	bool m_clock;
	bool m_link_active;
	bool m_sending;
	bool m_host_sending;
	u8 m_host_bits;
	u32 m_host_word;
};

DECLARE_DEVICE_TYPE(XAVIX_TVPC_CONTROLLER, xavix_tvpc_controller_device)

#endif // MAME_TVGAMES_XAVIX_IO_SUPRTVPC_CONTROLLER_H
