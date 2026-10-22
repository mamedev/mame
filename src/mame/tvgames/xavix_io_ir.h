// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_IR_H
#define MAME_TVGAMES_XAVIX_IO_IR_H

#pragma once

#include <algorithm>
#include <array>
#include <utility>

// Controllers supply reports; encoder bases own transmission and saved state.
class xavix_ir_report_device : public device_t
{
public:
	// Ordinary all-ones payloads are transmitted; this value suppresses a report.
	static constexpr u32 NO_SEQUENCE = 0xffffffff;
	template <typename... T> void set_event_callback(T &&... args) { m_event_out_cb.set(std::forward<T>(args)...); }
	int data_r() const { return m_data; }
	void vblank_w(int state);

protected:
	xavix_ir_report_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual u32 read_report(unsigned report) = 0;
	virtual bool reports_enabled() { return true; }
	virtual bool report_enabled(unsigned report) { return true; }
	virtual void begin_report() = 0;
	virtual void advance_report() = 0;

	void set_bit_ticks(u32 ticks) { m_bit_ticks = ticks; }
	void set_report_count(u8 count) { m_report_count = count; }
	template <unsigned N> void set_report_bits(u8 bits) { static_assert(N < 8); m_report_bits[N] = bits; }
	void receiver_event() { m_event_out_cb(); }
	void schedule_bit(u32 ticks) { m_bit_timer->adjust(clocks_to_attotime(ticks)); }
	void finish_report(bool stop_cell = false, u32 gap_ticks = 0);
	unsigned report_bits() const { return m_report_bits[m_report_index]; }

	u32 m_shift;
	u8 m_report_index;
	u8 m_bits_remaining;
	u8 m_data;
	u32 m_bit_ticks;

private:
	TIMER_CALLBACK_MEMBER(frame_start);
	TIMER_CALLBACK_MEMBER(bit_tick);
	void start_report(unsigned report);
	unsigned next_report(unsigned report) const;

	device_delegate<void ()> m_event_out_cb;
	emu_timer *m_frame_timer;
	emu_timer *m_bit_timer;
	std::array<u32, 8> m_latched_report;
	std::array<u8, 8> m_report_bits;
	u8 m_report_count;
	bool m_frame_pending;
	bool m_between_reports;
};

// MSB-first data, optionally with a distinct duration for each sample.
class xavix_ir_msb_device : public xavix_ir_report_device
{
protected:
	using xavix_ir_report_device::xavix_ir_report_device;
	virtual void device_start() override ATTR_COLD;
	virtual void begin_report() override;
	virtual void advance_report() override;
	template <std::size_t N> void set_sample_ticks(std::array<u32, N> const &ticks)
	{
		static_assert(N <= 32);
		std::copy(ticks.begin(), ticks.end(), m_sample_ticks.begin());
		m_sample_tick_count = N;
	}

private:
	u32 current_bit_ticks() const;
	std::array<u32, 32> m_sample_ticks{};
	u8 m_sample_tick_count = 0;
};

// Start cell, LSB-first data and one idle stop cell.
class xavix_ir_framed_device : public xavix_ir_report_device
{
protected:
	using xavix_ir_report_device::xavix_ir_report_device;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void begin_report() override;
	virtual void advance_report() override;
	void set_start_ticks(u32 ticks) { m_start_ticks.fill(ticks); }
	template <unsigned N> void set_start_ticks(u32 ticks) { static_assert(N < 8); m_start_ticks[N] = ticks; }

private:
	std::array<u32, 8> m_start_ticks{};
	bool m_start_cell;
};

// Each receiver event terminates a bit encoded by its interval.
class xavix_ir_pulse_distance_device : public xavix_ir_report_device
{
protected:
	using xavix_ir_report_device::xavix_ir_report_device;
	virtual void device_start() override ATTR_COLD;
	virtual void begin_report() override;
	virtual void advance_report() override;
	void set_pulse_distance_ticks(u32 one_ticks, u32 zero_ticks, u32 first_extension);
	static u16 complemented_report(u8 code) { return code | (u16(code ^ 0x1f) << 5); }

private:
	u32 current_pulse_ticks() const;
	u32 m_pulse_one_ticks = 0;
	u32 m_pulse_zero_ticks = 0;
	u32 m_first_pulse_extension = 0;
};

// Events delimit runs beginning with a one; the final zero run ends in silence.
class xavix_ir_run_length_device : public xavix_ir_report_device
{
protected:
	using xavix_ir_report_device::xavix_ir_report_device;
	virtual void begin_report() override;
	virtual void advance_report() override;
	void set_report_gap_ticks(u32 ticks) { m_report_gap_ticks = ticks; }

private:
	unsigned current_run_bits() const;
	u32 m_report_gap_ticks = 0;
};

// Carrier envelope reception; each controller owns its packet framing.
class xavix_ir_receiver_interface : public device_interface
{
protected:
	xavix_ir_receiver_interface(device_t &device, u32 quiet_ticks, bool rising_start);

	void ir_receiver_w(int state);
	void ir_receiver_reset(bool reset_level) ATTR_COLD;
	void ir_receiver_wait(u32 ticks);
	bool ir_receiver_active() const { return m_ir_carrier; }
	bool ir_receiver_level() const { return m_ir_level; }

	virtual void ir_burst_started(u64 gap_ticks) = 0;
	virtual void ir_burst_finished(u64 width_ticks) = 0;
	virtual void ir_idle_timeout() = 0;
	virtual void interface_post_start() override ATTR_COLD;

private:
	TIMER_CALLBACK_MEMBER(quiet_tick);

	const u32 m_quiet_ticks;
	const bool m_rising_start;
	emu_timer *m_ir_timer;
	attotime m_ir_burst_start;
	attotime m_ir_last_edge;
	bool m_ir_level;
	bool m_ir_carrier;
};

#endif // MAME_TVGAMES_XAVIX_IO_IR_H
