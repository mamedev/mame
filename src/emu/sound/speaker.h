// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/**********************************************************************

    speaker.h
    Sound driver to emulate a simple speaker,
    driven by one or more output bits

**********************************************************************/

#pragma once

#ifndef __SOUND_SPEAKER_H__
#define __SOUND_SPEAKER_H__

// Length of anti-aliasing filter kernel, measured in number of intermediate samples
enum
{
	FILTER_LENGTH = 64
};

#define MCFG_SPEAKER_LEVELS(_num, _levels) \
		speaker_sound_device::static_set_levels(*device, _num, _levels);

class speaker_sound_device : public device_t,
								public device_sound_interface
{
public:
	speaker_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~speaker_sound_device() {}

	// static configuration
	static void static_set_levels(device_t &device, int num_levels, const INT16 *levels) { downcast<speaker_sound_device &>(device).m_num_levels = num_levels; downcast<speaker_sound_device &>(device).m_levels = levels;}

	void level_w(int new_level);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

private:
	// internal state

	// Updates the composed volume array according to time
	void update_interm_samples(const attotime &time, int volume);

	// Updates the composed volume array and returns final filtered volume of next stream sample
	double update_interm_samples_get_filtered_volume(int volume);

	void finalize_interm_sample(int volume);
	void init_next_interm_sample();
	inline double make_fraction(const attotime &a, const attotime &b, double timediv);
	double get_filtered_volume();

	// Kernel (pulse response) for filtering across samples (while we avoid fancy filtering within samples)
	double m_ampl[FILTER_LENGTH];

	sound_stream *m_channel;
	int m_level;

	/* The volume of a composed sample grows incrementally each time the speaker is over-sampled.
	 * That is in effect a basic average filter.
	 * Another filter can and will be applied to the array of composed samples.
	 */
	double        m_composed_volume[FILTER_LENGTH];   /* integrator(s) */
	int           m_composed_sample_index;            /* array index for composed_volume */
	attoseconds_t m_channel_sample_period;            /* in as */
	double        m_channel_sample_period_secfrac;    /* in fraction of second */
	attotime      m_channel_last_sample_time;
	attotime      m_channel_next_sample_time;
	attoseconds_t m_interm_sample_period;
	double        m_interm_sample_period_secfrac;
	attotime      m_next_interm_sample_time;
	int           m_interm_sample_index;              /* counts interm. samples between stream samples */
	attotime      m_last_update_time;                 /* internal timestamp */

	void speaker_postload();

	// DC blocker state
	double  m_prevx, m_prevy;

	int          m_num_levels;  /* optional: number of levels (if not two) */
	const INT16  *m_levels;     /* optional: pointer to level lookup table */
};

extern const device_type SPEAKER_SOUND;


#endif /* __SPEAKER_H__ */
