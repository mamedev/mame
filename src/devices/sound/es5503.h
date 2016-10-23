// license:BSD-3-Clause
// copyright-holders:R. Belmont
#pragma once

#ifndef __ES5503_H__
#define __ES5503_H__

// channels must be a power of two

#define MCFG_ES5503_ADD(_tag, _clock)  \
	MCFG_DEVICE_ADD(_tag, ES5503, _clock)

#define MCFG_ES5503_OUTPUT_CHANNELS(_channels) \
	es5503_device::static_set_channels(*device, _channels);

#define MCFG_ES5503_IRQ_FUNC(_write) \
	devcb = &es5503_device::static_set_irqf(*device, DEVCB_##_write);

#define MCFG_ES5503_ADC_FUNC(_read) \
	devcb = &es5503_device::static_set_adcf(*device, DEVCB_##_read);

// ======================> es5503_device

class es5503_device : public device_t,
						public device_sound_interface,
						public device_rom_interface
{
public:
	// construction/destruction
	es5503_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void static_set_channels(device_t &device, int channels);

	template<class _Object> static devcb_base &static_set_irqf(device_t &device, _Object object) { return downcast<es5503_device &>(device).m_irq_func.set_callback(object); }
	template<class _Object> static devcb_base &static_set_adcf(device_t &device, _Object object) { return downcast<es5503_device &>(device).m_adc_func.set_callback(object); }

	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t get_channel_strobe() { return m_channel_strobe; }

	sound_stream *m_stream;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr) override;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	// device_rom_interface overrides
	virtual void rom_bank_updated() override;

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
extern const device_type ES5503;

#endif /* __ES5503_H__ */
