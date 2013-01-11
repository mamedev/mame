#pragma once

#ifndef __ES5503_H__
#define __ES5503_H__

// channels must be a power of two

#define MCFG_ES5503_ADD(_tag, _clock, _channels, _irqf, _adcf)  \
	MCFG_DEVICE_ADD(_tag, ES5503, _clock) \
	MCFG_ES5503_OUTPUT_CHANNELS(_channels) \
	MCFG_ES5503_IRQ_FUNC(_irqf) \
	MCFG_ES5503_ADC_FUNC(_adcf)

#define MCFG_ES5503_REPLACE(_tag, _clock, _channels, _irqf, _adcf) \
	MCFG_DEVICE_REPLACE(_tag, ES5503, _clock) \
	MCFG_ES5503_OUTPUT_CHANNELS(_channels) \
	MCFG_ES5503_IRQ_FUNC(_irqf) \
	MCFG_ES5503_ADC_FUNC(_adcf)

#define MCFG_ES5503_OUTPUT_CHANNELS(_channels) \
	es5503_device::static_set_channels(*device, _channels); \

#define MCFG_ES5503_IRQ_FUNC(_irqf) \
	es5503_device::static_set_irqf(*device, _irqf); \

#define MCFG_ES5503_ADC_FUNC(_adcf) \
	es5503_device::static_set_adcf(*device, _adcf); \

// ======================> es5503_device

class es5503_device : public device_t,
						public device_sound_interface,
						public device_memory_interface
{
public:
	// construction/destruction
	es5503_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static void static_set_channels(device_t &device, int channels);
	static void static_set_irqf(device_t &device, void (*irqf)(device_t *device, int state));
	static void static_set_adcf(device_t &device, UINT8 (*adcf)(device_t *device));

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

		UINT8 get_channel_strobe() { return m_channel_strobe; }

	sound_stream *m_stream;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr);

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

	const address_space_config  m_space_config;

	void (*m_irq_func)(device_t *device, int state);
	UINT8 (*m_adc_func)(device_t *device);

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
		UINT16 freq;
		UINT16 wtsize;
		UINT8  control;
		UINT8  vol;
		UINT8  data;
		UINT32 wavetblpointer;
		UINT8  wavetblsize;
		UINT8  resolution;

		UINT32 accumulator;
		UINT8  irqpend;
	};

	ES5503Osc oscillators[32];

	INT8  oscsenabled;      // # of oscillators enabled
	int   rege0;            // contents of register 0xe0

	UINT8 m_channel_strobe;

	int output_channels;
	UINT32 output_rate;

	emu_timer *m_timer;

	direct_read_data *m_direct;

	void halt_osc(int onum, int type, UINT32 *accumulator, int resshift);
};


// device type definition
extern const device_type ES5503;

#endif /* __ES5503_H__ */
