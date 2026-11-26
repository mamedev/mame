// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_SOUND_ES5503_H
#define MAME_SOUND_ES5503_H

#pragma once

#include "dirom.h"

// ======================> es5503_device

class es5503_device : public device_t,
					  public device_sound_interface,
					  public device_rom_interface<17>
{
public:
	// construction/destruction
	es5503_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// channels must be a power of two
	void set_channels(int channels) { m_output_channels = channels; }

	auto irq_func() { return m_irq_func.bind(); }
	auto adc_func() { return m_adc_func.bind(); }

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	// for hosts that time their own bus cycles
	void run_to(u64 clock);
	u8 bus_r() const { return m_bus; }

	uint8_t get_channel_strobe() { return m_channel_strobe; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override;
	virtual void device_reset() override ATTR_COLD;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream) override;

	// device_rom_interface overrides
	virtual void rom_bank_pre_change() override;

	TIMER_CALLBACK_MEMBER(scan_tick);

	sound_stream *m_stream;

	devcb_write_line   m_irq_func;
	devcb_read8        m_adc_func;

private:
	enum
	{
		MODE_FREE = 0,
		MODE_ONESHOT = 1,
		MODE_SYNCAM = 2,
		MODE_SWAP = 3
	};

	struct ES5503Osc
	{
		uint16_t freq;
		uint8_t  control;
		uint8_t  vol;
		uint8_t  data;
		uint32_t wavetblpointer;
		uint8_t  wavetblsize;
		uint8_t  resolution;

		uint32_t accumulator;
		uint8_t  irqpend;
		uint8_t  prevhalt;      // halt bit at the previous visit
		uint64_t refresh;       // slot in which the registers were last refreshed
	};

	static constexpr int FIFO_FRAMES = 8192;
	static constexpr int IDLE_SCAN = 21;    // the scan counter sits here and at the next value in the refresh slots

	ES5503Osc m_oscillators[32];

	int8_t  m_oscsenabled;      // # of oscillators enabled
	uint8_t m_rege0;            // oscillator named in register 0xe0
	bool m_irq_active;          // an interrupt is being presented
	uint64_t m_present_slot;    // slot in which it was presented

	uint64_t m_slot;            // next slot to run
	uint8_t m_scan;             // where the scan counter is
	uint8_t m_idle;             // nonzero in the 2 refresh slots after the last oscillator
	bool m_scan_off;            // no oscillators are serviced after a reset
	uint64_t m_drift_slots;     // unrefreshed registers are lost after this long
	uint8_t m_bus;              // last byte fetched from wave memory

	uint8_t m_channel_strobe;

	int m_output_channels;
	uint32_t m_output_rate;
	int32_t m_volume[256];      // output level for each volume register value

	emu_timer *m_timer;

	int32_t m_mix[16];
	int32_t m_last[16];
	std::vector<int32_t> m_fifo;
	int m_fifo_head, m_fifo_count;

	u64 now_clock() const { return attotime_to_clocks(machine().time()); }
	void update_rate();
	void run_slot();
	void end_round();
	void visit(int onum);
	void drift(ES5503Osc &osc);
	void check_drift(int onum);
};


// device type definition
DECLARE_DEVICE_TYPE(ES5503, es5503_device)

#endif // MAME_SOUND_ES5503_H
