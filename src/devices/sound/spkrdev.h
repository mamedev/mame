// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Anders Hallström
/**********************************************************************

    Sound driver to emulate a simple speaker,
    driven by one or more output bits

**********************************************************************/

#ifndef MAME_SOUND_SPKRDEV_H
#define MAME_SOUND_SPKRDEV_H

#pragma once


class speaker_sound_device : public device_t,
								public device_sound_interface
{
public:
	speaker_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	~speaker_sound_device() {}

	// configuration
	void set_levels(int num_levels, const double *levels) { m_num_levels = num_levels; m_levels = levels; }

	void level_w(int new_level); // can use as writeline

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	// Length of anti-aliasing filter kernel, measured in number of intermediate samples
	enum
	{
		FILTER_LENGTH = 64
	};

	// internal state

	// Updates the composed volume array according to time
	void update_interm_samples(const attotime &time, double volume);

	// Updates the composed volume array and returns final filtered volume of next stream sample
	double update_interm_samples_get_filtered_volume(double volume);

	void finalize_interm_sample(double volume);
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

	// DC blocker state
	double  m_prevx, m_prevy;

	int          m_num_levels;  /* optional: number of levels (if not two) */
	const double  *m_levels;     /* optional: pointer to level lookup table */
};

DECLARE_DEVICE_TYPE(SPEAKER_SOUND, speaker_sound_device)

#endif // MAME_SOUND_SPKRDEV_H
