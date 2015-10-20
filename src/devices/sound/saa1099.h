// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller, Manuel Abadia
/**********************************************
    Philips SAA1099 Sound driver
**********************************************/

#pragma once

#ifndef __SAA1099_H__
#define __SAA1099_H__

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SAA1099_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, SAA1099, _clock)
#define MCFG_SAA1099_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, SAA1099, _clock)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct saa1099_channel
{
	saa1099_channel() :
		frequency(0),
		freq_enable(0),
		noise_enable(0),
		octave(0),
		counter(0.0),
		freq(0.0),
		level(0)
	{
		memset(amplitude, 0, sizeof(int)*2);
		memset(envelope, 0, sizeof(int)*2);
	}

	int frequency;          /* frequency (0x00..0xff) */
	int freq_enable;        /* frequency enable */
	int noise_enable;       /* noise enable */
	int octave;             /* octave (0x00..0x07) */
	int amplitude[2];       /* amplitude (0x00..0x0f) */
	int envelope[2];        /* envelope (0x00..0x0f or 0x10 == off) */

	/* vars to simulate the square wave */
	double counter;
	double freq;
	int level;
};

struct saa1099_noise
{
	saa1099_noise() :
		counter(0.0),
		freq(0.0),
		level(0) {}

	/* vars to simulate the noise generator output */
	double counter;
	double freq;
	int level;                      /* noise polynomal shifter */
};


// ======================> saa1099_device

class saa1099_device : public device_t,
						public device_sound_interface
{
public:
	saa1099_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~saa1099_device() { }

protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

public:
	DECLARE_WRITE8_MEMBER( control_w );
	DECLARE_WRITE8_MEMBER( data_w );

private:
	void envelope_w(int ch);

private:
	sound_stream *m_stream;          /* our stream */
	int m_noise_params[2];            /* noise generators parameters */
	int m_env_enable[2];              /* envelope generators enable */
	int m_env_reverse_right[2];       /* envelope reversed for right channel */
	int m_env_mode[2];                /* envelope generators mode */
	int m_env_bits[2];                /* non zero = 3 bits resolution */
	int m_env_clock[2];               /* envelope clock mode (non-zero external) */
	int m_env_step[2];                /* current envelope step */
	int m_all_ch_enable;              /* all channels enable */
	int m_sync_state;                 /* sync all channels */
	int m_selected_reg;               /* selected register */
	saa1099_channel m_channels[6];    /* channels */
	saa1099_noise m_noise[2];         /* noise generators */
	double m_sample_rate;
	int m_master_clock;
};

extern const device_type SAA1099;


#endif /* __SAA1099_H__ */
