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
	es5503_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// channels must be a power of two
	void set_channels(int channels) { output_channels = channels; }

	auto irq_func() { return m_irq_func.bind(); }
	auto adc_func() { return m_adc_func.bind(); }

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

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

	TIMER_CALLBACK_MEMBER(delayed_stream_update);

	sound_stream *m_stream;

	devcb_write_line   m_irq_func;
	devcb_read8        m_adc_func;

	emu_timer *m_sync_timer;

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
		uint16_t wtsize;
		uint8_t  control;
		uint8_t  vol;
		uint8_t  data;
		uint32_t wavetblpointer;
		uint8_t  wavetblsize;
		uint8_t  resolution;

		uint32_t accumulator;
		uint8_t  irqpend;
	};

	ES5503Osc oscillators[32];

	int8_t  oscsenabled;      // # of oscillators enabled
	int   rege0;            // contents of register 0xe0

	uint8_t m_channel_strobe;

	int output_channels;
	uint32_t output_rate;

	emu_timer *m_timer;

	std::vector<int32_t> m_mix_buffer;

	void halt_osc(int onum, int type, uint32_t *accumulator, int resshift);
};


// device type definition
DECLARE_DEVICE_TYPE(ES5503, es5503_device)

#endif // MAME_SOUND_ES5503_H
