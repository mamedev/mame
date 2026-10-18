// license:GPL-2.0
// copyright-holders:Giulio Zausa
#ifndef MAME_SOUND_MB87126_H
#define MAME_SOUND_MB87126_H
#pragma once

#include <array>
#include <cstdint>
#include <functional>

class mb87126_device : public device_t, public device_sound_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; }
	static constexpr u8 BUS_DRIVE = 0x20;
	static constexpr u8 BUS_CAS = 0x40;

	mb87126_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	auto bus_callback() { return m_bus_cb.bind(); }
	// Called by the board during bus_callback; does not update the stream.
	void bus_w(u32 data, u32 mem_mask = 0xffffff) { m_core.bus_w(data, mem_mask); }
	// External frame interval, in half-oscillator periods, applied next frame.
	void frame_clocks_w(u32 clocks);
	void serial_w(bool clock, bool data);
	void control_w(bool inck, bool ercl);
	int busy_r();
	void update() { m_stream->update(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	class mb87126_core
	{
	public:
		static constexpr unsigned STEPS = 160;
		static constexpr unsigned DRAM_WORDS = 65536;
		static constexpr uint8_t BUS_DRIVE = 0x20;
		static constexpr uint8_t BUS_CAS = 0x40;
		// Every program word: signals[4:0] are RD4..RD0, bit 5 is data drive,
		// bit 6 is an active delay-RAM cycle.
		// A receiver can use bus_w to drive selected bits before the bus is latched.
		using bus_handler = std::function<void(uint8_t signals, uint32_t data)>;

		mb87126_core();
		void reset();
		void write_parameter(uint8_t selector, uint16_t delay, uint16_t coefficient);
		void run_frame(const uint8_t *program, int16_t adc, const bus_handler &bus_cycle = {});
		void bus_w(uint32_t data, uint32_t mem_mask = 0xffffff);

		static int64_t term(int32_t operand, uint16_t coefficient);
		static int64_t wrap28(int64_t value);
		static int32_t clip24(int64_t value);
		static uint64_t program_word(const uint8_t *program, unsigned pc);

		// Persistent datapath, parameter RAM and external delay RAM.
		std::array<uint16_t, STEPS> m_dc, m_dd;
		std::array<int32_t, DRAM_WORDS> m_dram;
		std::array<int32_t, 4> m_latch;
		std::array<int64_t, 4> m_register;
		uint32_t m_base;
		int64_t m_sum, m_older_sum;
		int32_t m_capture, m_operand, m_kept_operand, m_fetch;
		int32_t m_bus, m_converter;
		uint16_t m_trial;
		int32_t m_trial_bit;
		std::array<int32_t, 2> m_output;
		bool m_output_select;
	};

	bool write_serial_pins(bool clock, bool data);
	sound_stream *m_stream;
	required_region_ptr<u8> m_program;
	devcb_write32 m_bus_cb;
	mb87126_core m_core;
	std::array<u8, 5> m_serial;
	u8 m_serial_bits;
	bool m_serial_clock, m_ercl;
	attotime m_busy_until;
	bool m_sync_error;
	u32 m_frame_clocks, m_next_frame_clocks, m_frame_accumulator;
};
DECLARE_DEVICE_TYPE(MB87126, mb87126_device)
#endif // MAME_SOUND_MB87126_H
