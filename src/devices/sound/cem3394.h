// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#pragma once

#ifndef __CEM3394_H__
#define __CEM3394_H__

#define CEM3394_SAMPLE_RATE     (44100*4)

// inputs
enum
{
	CEM3394_VCO_FREQUENCY = 0,
	CEM3394_MODULATION_AMOUNT,
	CEM3394_WAVE_SELECT,
	CEM3394_PULSE_WIDTH,
	CEM3394_MIXER_BALANCE,
	CEM3394_FILTER_RESONANCE,
	CEM3394_FILTER_FREQENCY,
	CEM3394_FINAL_GAIN
};

typedef device_delegate<void (int count, short *buffer)> cem3394_ext_input_delegate;

#define CEM3394_EXT_INPUT(_name) void _name(int count, short *buffer)

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CEM3394_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, CEM3394, _clock)
#define MCFG_CEM3394_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, CEM3394, _clock)

#define MCFG_CEM3394_EXT_INPUT_CB(_class, _method) \
	cem3394_device::set_ext_input_callback(*device, cem3394_ext_input_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_CEM3394_VCO_ZERO(_freq) \
	cem3394_device::set_vco_zero_freq(*device, _freq);

#define MCFG_CEM3394_FILTER_ZERO(_freq) \
	cem3394_device::set_filter_zero_freq(*device, _freq);


class cem3394_device : public device_t,
						public device_sound_interface
{
public:
	cem3394_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~cem3394_device() { }

	static void set_ext_input_callback(device_t &device, cem3394_ext_input_delegate callback) { downcast<cem3394_device &>(device).m_ext_cb = callback; }
	static void set_vco_zero_freq(device_t &device, double freq) { downcast<cem3394_device &>(device).m_vco_zero_freq = freq; }
	static void set_filter_zero_freq(device_t &device, double freq) { downcast<cem3394_device &>(device).m_filter_zero_freq = freq; }

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

public:
	// Set the voltage going to a particular parameter
	void set_voltage(int input, double voltage);

	// Get the translated parameter associated with the given input as follows:
	//    CEM3394_VCO_FREQUENCY:      frequency in Hz
	//    CEM3394_MODULATION_AMOUNT:  scale factor, 0.0 to 2.0
	//    CEM3394_WAVE_SELECT:        voltage from this line
	//    CEM3394_PULSE_WIDTH:        width fraction, from 0.0 to 1.0
	//    CEM3394_MIXER_BALANCE:      balance, from -1.0 to 1.0
	//    CEM3394_FILTER_RESONANCE:   resonance, from 0.0 to 1.0
	//    CEM3394_FILTER_FREQENCY:    frequency, in Hz
	//    CEM3394_FINAL_GAIN:         gain, in dB
	double get_parameter(int input);

private:
	double compute_db(double voltage);
	UINT32 compute_db_volume(double voltage);

private:
	cem3394_ext_input_delegate m_ext_cb; /* callback to generate external samples */

	sound_stream *m_stream;           /* our stream */
	double m_vco_zero_freq;           /* frequency of VCO at 0.0V */
	double m_filter_zero_freq;        /* frequency of filter at 0.0V */

	double m_values[8];               /* raw values of registers */
	UINT8 m_wave_select;              /* flags which waveforms are enabled */

	UINT32 m_volume;                  /* linear overall volume (0-256) */
	UINT32 m_mixer_internal;          /* linear internal volume (0-256) */
	UINT32 m_mixer_external;          /* linear external volume (0-256) */

	UINT32 m_position;                /* current VCO frequency position (0.FRACTION_BITS) */
	UINT32 m_step;                    /* per-sample VCO step (0.FRACTION_BITS) */

	UINT32 m_filter_position;         /* current filter frequency position (0.FRACTION_BITS) */
	UINT32 m_filter_step;             /* per-sample filter step (0.FRACTION_BITS) */
	UINT32 m_modulation_depth;        /* fraction of total by which we modulate (0.FRACTION_BITS) */
	INT16 m_last_ext;                 /* last external sample we read */

	UINT32 m_pulse_width;             /* fractional pulse width (0.FRACTION_BITS) */

	double m_inv_sample_rate;
	int m_sample_rate;

	std::unique_ptr<INT16[]> m_mixer_buffer;
	std::unique_ptr<INT16[]> m_external_buffer;
};

extern const device_type CEM3394;


#endif /* __CEM3394_H__ */
