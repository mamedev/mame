// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_IR_H
#define MAME_TVGAMES_XAVIX_IO_IR_H

#pragma once

#include "machine/timer.h"

#include <array>
#include <cstddef>
#include <utility>

class xavix_ir_report_device : public device_t
{
public:
	enum class mode
	{
		MSB_FIRST,
		FRAMED_LSB,
		PULSE_DISTANCE,
		SAMPLE_SEQUENCE,
		PULSE_RUN_LENGTH
	};

	// Sequence callbacks may return NO_SEQUENCE; per-report enable callbacks also use it internally.
	// Ordinary all-ones report payloads are still transmitted.
	static constexpr u32 NO_SEQUENCE = 0xffffffff;
	static u8 motion_report(unsigned player, bool button, u8 strength);
	static u16 baseball_bat_report(bool bunt, bool swing);

	xavix_ir_report_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <unsigned N> auto report_callback() { return m_report_cb[N].bind(); }
	template <unsigned N> auto report_enable_callback() { return m_report_enable_cb[N].bind(); }
	template <unsigned N> auto sequence_callback() { return m_sequence_cb[N].bind(); }
	auto enable_callback() { return m_enable_cb.bind(); }
	template <typename... T> void set_event_callback(T &&... args) { m_event_out_cb.set(std::forward<T>(args)...); }
	void set_bit_ticks(u32 bit_ticks) { m_bit_ticks = bit_ticks; }
	void set_start_ticks(u32 start_ticks) { m_start_ticks.fill(start_ticks); }
	template <unsigned N> void set_start_ticks(u32 start_ticks) { static_assert(N < 8); m_start_ticks[N] = start_ticks; }
	void set_report_count(u8 report_count) { m_report_count = report_count; }
	void set_report_gap_ticks(u32 ticks) { m_report_gap_ticks = ticks; }
	template <unsigned N> void set_report_bits(u8 bits) { static_assert(N < 8); m_report_bits[N] = bits; }
	void set_mode(mode report_mode) { m_mode = report_mode; }
	void set_pulse_distance_ticks(u32 one_ticks, u32 zero_ticks, u32 first_extension)
	{
		m_pulse_one_ticks = one_ticks;
		m_pulse_zero_ticks = zero_ticks;
		m_first_pulse_extension = first_extension;
	}
	template <std::size_t N> void set_sample_ticks(std::array<u32, N> const &ticks)
	{
		static_assert(N <= 32);
		for (std::size_t index = 0; index != N; ++index)
			m_sample_ticks[index] = ticks[index];
		m_sample_tick_count = N;
	}

	int data_r() { return m_data; }
	void vblank_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr u8 MOTION_REPORTS[2][2][4] =
	{
		{ { 0x75, 0x5d, 0x4b, 0x2d }, { 0x69, 0x57, 0x37, 0x15 } },
		{ { 0x73, 0x59, 0x45, 0x2b }, { 0x65, 0x51, 0x31, 0x13 } }
	};
	static constexpr u8 BASEBALL_BAT_PAYLOADS[3] = { 0x43, 0x4a, 0x51 };

	TIMER_CALLBACK_MEMBER(frame_start);
	void start_report(unsigned report);
	TIMER_CALLBACK_MEMBER(bit_tick);
	unsigned next_report(unsigned report) const;
	u32 current_bit_ticks() const;
	u32 current_pulse_ticks() const;
	unsigned current_run_bits() const;

	devcb_read16::array<8> m_report_cb;
	devcb_read_line::array<8> m_report_enable_cb;
	devcb_read32::array<8> m_sequence_cb;
	devcb_read_line m_enable_cb;
	device_delegate<void ()> m_event_out_cb;
	emu_timer *m_frame_timer;
	emu_timer *m_bit_timer;

	std::array<u32, 8> m_latched_report;
	std::array<u8, 8> m_report_bits;
	std::array<u32, 32> m_sample_ticks;
	u32 m_shift;
	u8 m_report_index;
	u8 m_bits_remaining;
	u8 m_data;
	bool m_frame_pending;
	bool m_between_reports;
	bool m_start_cell;
	u32 m_bit_ticks;
	u32 m_pulse_one_ticks;
	u32 m_pulse_zero_ticks;
	u32 m_first_pulse_extension;
	u32 m_report_gap_ticks;
	std::array<u32, 8> m_start_ticks;
	u8 m_report_count;
	u8 m_sample_tick_count;
	mode m_mode;
};

DECLARE_DEVICE_TYPE(XAVIX_IR_REPORT, xavix_ir_report_device)

#endif // MAME_TVGAMES_XAVIX_IO_IR_H
