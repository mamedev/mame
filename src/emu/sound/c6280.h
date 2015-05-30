// license:BSD-3-Clause
// copyright-holders:Charles MacDonald
#pragma once

#ifndef __C6280_H__
#define __C6280_H__

#include "cpu/h6280/h6280.h"

class c6280_device : public device_t,
						public device_sound_interface
{
public:
	c6280_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static void set_devicecpu_tag(device_t &device, const char *tag) { downcast<c6280_device &>(device).m_cpudevice.set_tag(tag); }

	// read/write
	DECLARE_READ8_MEMBER( c6280_r );
	DECLARE_WRITE8_MEMBER( c6280_w );

protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

private:
	struct channel {
		UINT16 m_frequency;
		UINT8 m_control;
		UINT8 m_balance;
		UINT8 m_waveform[32];
		UINT8 m_index;
		INT16 m_dda;
		UINT8 m_noise_control;
		UINT32 m_noise_counter;
		UINT32 m_counter;
	};

	// internal state
	sound_stream *m_stream;
	required_device<h6280_device> m_cpudevice;
	UINT8 m_select;
	UINT8 m_balance;
	UINT8 m_lfo_frequency;
	UINT8 m_lfo_control;
	channel m_channel[8];
	INT16 m_volume_table[32];
	UINT32 m_noise_freq_tab[32];
	UINT32 m_wave_freq_tab[4096];
};

extern const device_type C6280;

#define MCFG_C6280_CPU(_tag) \
	c6280_device::set_devicecpu_tag(*device, "^" _tag);


#endif /* __C6280_H__ */
