// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    speaker.cpp

    Speaker output sound device.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "speaker.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(SPEAKER, speaker_device, "speaker", "Speaker")



//**************************************************************************
//  LIVE SPEAKER DEVICE
//**************************************************************************

//-------------------------------------------------
//  speaker_device - constructor
//-------------------------------------------------

speaker_device::speaker_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SPEAKER, tag, owner, clock)
	, device_mixer_interface(mconfig, *this)
	, m_x(0.0)
	, m_y(0.0)
	, m_z(0.0)
	, m_current_max(0)
	, m_samples_this_bucket(0)
{
}


//-------------------------------------------------
//  ~speaker_device - destructor
//-------------------------------------------------

speaker_device::~speaker_device()
{
}


//-------------------------------------------------
//  mix - mix in samples from the speaker's stream
//-------------------------------------------------

void speaker_device::mix(s32 *leftmix, s32 *rightmix, int &samples_this_update, bool suppress)
{
	// skip if no stream
	if (m_mixer_stream == nullptr)
		return;

	// update the stream, getting the start/end pointers around the operation
	int numsamples;
	const stream_sample_t *stream_buf = m_mixer_stream->output_since_last_update(0, numsamples);

	// set or assert that all streams have the same count
	if (samples_this_update == 0)
	{
		samples_this_update = numsamples;

		// reset the mixing streams
		std::fill_n(leftmix, samples_this_update, 0);
		std::fill_n(rightmix, samples_this_update, 0);
	}
	assert(samples_this_update == numsamples);

	// track maximum sample value for each 0.1s bucket
	if (machine().options().speaker_report() != 0)
	{
		u32 samples_per_bucket = m_mixer_stream->sample_rate() / BUCKETS_PER_SECOND;
		for (int sample = 0; sample < samples_this_update; sample++)
		{
			m_current_max = std::max(m_current_max, abs(stream_buf[sample]));
			if (++m_samples_this_bucket >= samples_per_bucket)
			{
				m_max_sample.push_back(m_current_max);
				m_current_max = 0;
				m_samples_this_bucket = 0;
			}
		}
	}

	// mix if sound is enabled
	if (!suppress)
	{
		// if the speaker is centered, send to both left and right
		if (m_x == 0)
			for (int sample = 0; sample < samples_this_update; sample++)
			{
				leftmix[sample] += stream_buf[sample];
				rightmix[sample] += stream_buf[sample];
			}

		// if the speaker is to the left, send only to the left
		else if (m_x < 0)
			for (int sample = 0; sample < samples_this_update; sample++)
				leftmix[sample] += stream_buf[sample];

		// if the speaker is to the right, send only to the right
		else
			for (int sample = 0; sample < samples_this_update; sample++)
				rightmix[sample] += stream_buf[sample];
	}
}


//-------------------------------------------------
//  device_start - handle device startup
//-------------------------------------------------

void speaker_device::device_start()
{
	// dummy save to make device.c happy
}


//-------------------------------------------------
//  device_stop - cleanup and report
//-------------------------------------------------

void speaker_device::device_stop()
{
	// level 1: just report if there was any clipping
	// level 2: report the overall maximum, even if no clipping
	// level 3: print a detailed list of all the times there was clipping
	// level 4: print a detailed list of every bucket
	int report = machine().options().speaker_report();
	if (report != 0)
	{
		m_max_sample.push_back(m_current_max);

		// determine overall maximum and number of clipped buckets
		s32 overallmax = 0;
		u32 clipped = 0;
		for (auto &curmax : m_max_sample)
		{
			overallmax = std::max(overallmax, curmax);
			if (curmax > 32767)
				clipped++;
		}

		// levels 1 and 2 just get a summary
		if (clipped != 0 || report == 2 || report == 4)
			osd_printf_info("Speaker \"%s\" - max = %d (gain *= %.3f) - clipped in %d/%d (%d%%) buckets\n", tag(), overallmax, 32767.0 / (overallmax ? overallmax : 1), clipped, m_max_sample.size(), clipped * 100 / m_max_sample.size());

		// levels 3 and 4 get a full dump
		if (report >= 3)
		{
			double t = 0;
			for (auto &curmax : m_max_sample)
			{
				if (curmax > 32767 || report == 4)
					osd_printf_info("   t=%5.1f  max=%6d\n", t, curmax);
				t += 1.0 / double(BUCKETS_PER_SECOND);
			}
		}
	}
}

