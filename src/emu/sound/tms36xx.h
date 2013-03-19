#pragma once

#ifndef __TMS36XX_H__
#define __TMS36XX_H__

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_TMS36XX_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, TMS36XX, _clock)
#define MCFG_TMS36XX_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, TMS36XX, _clock)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// subtypes
#define MM6221AA    21      // Phoenix (fixed melodies)
#define TMS3615     15      // Naughty Boy, Pleiads (13 notes, one output)
#define TMS3617     17      // Monster Bash (13 notes, six outputs)


// ======================> tms36xx_interface

struct tms36xx_interface
{
	int subtype;
	double decay[6];    // decay times for the six harmonic notes
	double speed;       // tune speed (meaningful for the TMS3615 only)
};


// ======================> tms36xx_device

class tms36xx_device : public device_t,
						public device_sound_interface
{
public:
	tms36xx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tms36xx_device() { }

protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

public:
	// MM6221AA interface functions
	void mm6221aa_tune_w(int tune);

	// TMS3615/17 interface functions
	void tms36xx_note_w(int octave, int note);

	// TMS3617 interface functions
	void tms3617_enable_w(int enable);

private:
	void tms36xx_reset_counters();
	void tms3617_enable(int enable);

private:
	char *m_subtype;      // subtype name MM6221AA, TMS3615 or TMS3617
	sound_stream *m_channel; // returned by stream_create()

	int m_samplerate;     // output sample rate

	int m_basefreq;       // chip's base frequency
	int m_octave;         // octave select of the TMS3615

	int m_speed;          // speed of the tune
	int m_tune_counter;   // tune counter
	int m_note_counter;   // note counter

	int m_voices;         // active voices
	int m_shift;          // shift toggles between 0 and 6 to allow decaying voices
	int m_vol[12];        // (decaying) volume of harmonics notes
	int m_vol_counter[12];// volume adjustment counter
	int m_decay[12];      // volume adjustment rate - dervied from decay

	int m_counter[12];    // tone frequency counter
	int m_frequency[12];  // tone frequency
	int m_output;         // output signal bits
	int m_enable;         // mask which harmoics

	int m_tune_num;       // tune currently playing
	int m_tune_ofs;       // note currently playing
	int m_tune_max;       // end of tune

	const tms36xx_interface *m_intf;
};

extern const device_type TMS36XX;


#endif /* __TMS36XX_H__ */
