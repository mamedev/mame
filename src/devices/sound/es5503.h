// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_SOUND_ES5503_H
#define MAME_SOUND_ES5503_H

#pragma once

// channels must be a power of two

#define MCFG_ES5503_ADD(_tag, _clock)  \
	MCFG_DEVICE_ADD(_tag, ES5503, _clock)

#define MCFG_ES5503_OUTPUT_CHANNELS(_channels) \
	downcast<es5503_device &>(*device).set_channels(_channels);

#define MCFG_ES5503_IRQ_FUNC(_write) \
	devcb = &downcast<es5503_device &>(*device).set_irqf(DEVCB_##_write);

#define MCFG_ES5503_ADC_FUNC(_read) \
	devcb = &downcast<es5503_device &>(*device).set_adcf(DEVCB_##_read);

// ======================> es5503_device

class es5503_device : public device_t,
						public device_sound_interface,
						public device_rom_interface
{
public:
	// construction/destruction
	es5503_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_channels(int channels) { output_channels = channels; }

	template <class Object> devcb_base &set_irqf(Object &&cb) { return m_irq_func.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_adcf(Object &&cb) { return m_adc_func.set_callback(std::forward<Object>(cb)); }

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	uint8_t get_channel_strobe() { return m_channel_strobe; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr) override;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	// device_rom_interface overrides
	virtual void rom_bank_updated() override;

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

	void halt_osc(int onum, int type, uint32_t *accumulator, int resshift);
};


// device type definition
DECLARE_DEVICE_TYPE(ES5503, es5503_device)

#endif // MAME_SOUND_ES5503_H
