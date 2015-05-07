// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
#pragma once

#ifndef __SOCR_SND_H__
#define __SOCR_SND_H__

class socrates_snd_device : public device_t,
							public device_sound_interface
{
public:
	socrates_snd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void reg0_w(int data);
	void reg1_w(int data);
	void reg2_w(int data);
	void reg3_w(int data);
	void reg4_w(int data);

protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	void snd_clock();
	static const UINT8 s_volumeLUT[];

	// internal state
	sound_stream *  m_stream;
	UINT8           m_freq[2];      // channel 1,2 frequencies
	UINT8           m_vol[2];       // channel 1,2 volume
	UINT8           m_enable[2];    // channel 1,2 enable
	UINT8           m_channel3;     // channel 3 weird register
	UINT8           m_state[3];     // output states for channels 1,2,3
	UINT8           m_accum[3];     // accumulators for channels 1,2,3
	UINT16          m_DAC_output;   // output
};

extern const device_type SOCRATES_SOUND;


#endif /* __SOCR_SND_H__ */
