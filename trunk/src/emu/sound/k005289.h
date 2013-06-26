#pragma once

#ifndef __K005289_H__
#define __K005289_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_K005289_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, K005289, _clock)
#define MCFG_K005289_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, K005289, _clock)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct k005289_sound_channel
{
	int frequency;
	int counter;
	int volume;
	const unsigned char *wave;
};


// ======================> k005289_device

class k005289_device : public device_t,
						public device_sound_interface
{
public:
	k005289_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k005289_device() { }

protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

public:
	DECLARE_WRITE8_MEMBER( k005289_control_A_w );
	DECLARE_WRITE8_MEMBER( k005289_control_B_w );
	DECLARE_WRITE8_MEMBER( k005289_pitch_A_w );
	DECLARE_WRITE8_MEMBER( k005289_pitch_B_w );
	DECLARE_WRITE8_MEMBER( k005289_keylatch_A_w );
	DECLARE_WRITE8_MEMBER( k005289_keylatch_B_w );

private:
	void make_mixer_table(int voices);
	void k005289_recompute();

private:
	k005289_sound_channel m_channel_list[2];

	const unsigned char *m_sound_prom;
	sound_stream *m_stream;
	int m_mclock;
	int m_rate;

	/* mixer tables and internal buffers */
	INT16 *m_mixer_table;
	INT16 *m_mixer_lookup;
	short *m_mixer_buffer;

	int m_k005289_A_frequency;
	int m_k005289_B_frequency;
	int m_k005289_A_volume;
	int m_k005289_B_volume;
	int m_k005289_A_waveform;
	int m_k005289_B_waveform;
	int m_k005289_A_latch;
	int m_k005289_B_latch;
};

extern const device_type K005289;


#endif /* __K005289_H__ */
