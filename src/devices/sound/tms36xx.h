// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
#ifndef MAME_SOUND_TMS36XX_H
#define MAME_SOUND_TMS36XX_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************



// ======================> tms36xx_device

class tms36xx_device : public device_t,
						public device_sound_interface
{
public:
	enum class subtype
	{
		MM6221AA    = 21,     // Phoenix (fixed melodies)
		TMS3615     = 15,     // Naughty Boy, Pleiads (13 notes, one output)
		TMS3617     = 17      // Monster Bash (13 notes, six outputs)
	};

	tms36xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_subtype(subtype type)
	{
		switch (type)
		{
		case subtype::MM6221AA:
			m_subtype = "MM6221AA";
			break;
		case subtype::TMS3615:
			m_subtype = "TMS3615";
			break;
		case subtype::TMS3617:
			m_subtype = "TMS3617";
			break;
		default:
			fatalerror("Invalid TMS36XX type: %d\n", int(type));
			break;
		}
	}
	void set_tune_speed(double speed) { m_speed = (speed > 0) ? TMS36XX_VMAX / speed : TMS36XX_VMAX; }
	void set_decays(double decay_0, double decay_1, double decay_2, double decay_3, double decay_4, double decay_5)
	{
		m_decay_time[0] = decay_0;
		m_decay_time[1] = decay_1;
		m_decay_time[2] = decay_2;
		m_decay_time[3] = decay_3;
		m_decay_time[4] = decay_4;
		m_decay_time[5] = decay_5;
	}

protected:
	static constexpr unsigned TMS36XX_VMIN = 0x0000;
	static constexpr unsigned TMS36XX_VMAX = 0x7fff;

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

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

	double m_decay_time[6];  // decay times for the six harmonic notes

	const char *m_subtype;      // subtype name MM6221AA, TMS3615 or TMS3617
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
	int m_decay[12];      // volume adjustment rate - derived from m_intf_decay

	int m_counter[12];    // tone frequency counter
	int m_frequency[12];  // tone frequency
	int m_output;         // output signal bits
	int m_enable;         // mask which harmonics

	int m_tune_num;       // tune currently playing
	int m_tune_ofs;       // note currently playing
	int m_tune_max;       // end of tune
};

DECLARE_DEVICE_TYPE(TMS36XX, tms36xx_device)


#endif // MAME_SOUND_TMS36XX_H
