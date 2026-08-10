// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/***************************************************************************

    Floppy sound
    For usage description see drivesound.h

***************************************************************************/

#include "emu.h"
#include "drivesound.h"
#include "formats/flopimg.h"
#include "speaker.h"

// Some debug output

#define LOG_CONFIG      (1U << 1)
#define LOG_SND         (1U << 2)
#define LOG_SND_DETAIL  (1U << 3)
#define VERBOSE ( LOG_CONFIG )

#include "logmacro.h"

#define FLOPSND_TAG "floppysound"
#define FLOPSPK "output"

DEFINE_DEVICE_TYPE(FLOPPYSOUND, floppy_sound_device, FLOPSND_TAG, "Floppy sound")

/* ===================================================================

    Some implementation details:

    Not all spin_kinds need to be defined. See the array replace_sample in
    find_spin for the rules by which samples are used when the proper ones
    are not available. The ultimate default is SPIN_LOADED, so this sample
    is mandatory.

    The fast repetition of a step sample does not yield a seek sound (a sequence
    of steps). Hence, the implementation must find out whether this is a single
    step or a seek); in the first case, a step sample is played, while in
    the second, a seek sample must be played. For this, it checks whether
    there is a new step during the playback of the step sample. In this case,
    a seek is assumed.

    The seek samples are chosen by the detected step rate n (in milliseconds).
    The seek sample whose max_rate is minimally higher than n is taken, and the
    playback is pitched up or down related to the sample's actual rate.

    This works well with the majority of the system, except for the Amiga. The
    Amiga seems to have direct control on the stepper motor in the drive so that
    the step rates vary widely. In order to avoid switching between different
    sample files all the time, a new rate is assumed only if it is more than
    10% off the current rate. This may make it difficult to reproduce music
    with the floppy hardware ("Floppytron"), but we'll probably have to go for
    a different approach in that case anyway.

    Step sounds are played when the interval between them is long enough, so no
    seek sound would be produced. If only one step sample shall be used,
    the range should be set as (0,99). Otherwise, the step sample is played
    whose range contains the current track.

=================================================================== */

floppy_sound_samples::floppy_sound_samples() :
	m_current_form_factor(floppy_image::FF_UNKNOWN),
	m_current_dir(nullptr)
{
};

void floppy_sound_samples::select(int form_factor)
{
	bool found = false;

	while (!found)
	{
		int index = 0; // index of the sample in the sample name list

		for (floppy_sound_entry& entry : m_fulllist)
		{
			if (entry.form_factor == form_factor && entry.directory != nullptr)
			{
				if (index == 0)   // new list
				{
					// Create the asterisked first entry for the subdirectory
					m_basedir = "*" + std::string(entry.directory);
					m_samplenames.push_back(m_basedir.c_str());
					index++;
					found = true;
				}
				entry.index = index++;   // keep record of position in the sample list
				m_samplenames.push_back(entry.filename);
			}
		}

		if (!found)
		{
			// If we don't have 3" samples, try to use 3.5" samples
			if (form_factor == floppy_image::FF_3)
				form_factor = floppy_image::FF_35;
			else
			{
				// If we don't find 3", 3.5", and 8" samples, try 5.25"
				if (form_factor != floppy_image::FF_525)
					form_factor = floppy_image::FF_525;
				else
				{
					// If this also fails, don't use sound at all.
					form_factor = 0;
					break;
				}
			}
		}
	}
	m_current_form_factor = form_factor;
}

void floppy_sound_samples::set_form_factor(int form_factor, const char* dir)
{
	m_current_dir = dir;
	m_current_form_factor = form_factor;
}

void floppy_sound_samples::add_spin_sample(const char* filename, int type)
{
	floppy_sound_entry entry;
	entry.type = SPIN;
	entry.spintype = type;
	entry.filename = filename;
	entry.form_factor = m_current_form_factor;
	entry.directory = m_current_dir;
	m_fulllist.push_back(entry);
}

void floppy_sound_samples::add_step_sample(const char* filename, int dir)
{
	add_step_sample(filename, 0, 99, dir);
}

void floppy_sound_samples::add_step_sample(const char* filename, int mintrack, int maxtrack, int dir)
{
	floppy_sound_entry entry;
	entry.type = STEP;
	entry.mintrack = mintrack;
	entry.maxtrack = maxtrack;
	entry.dir = dir;
	entry.filename = filename;
	entry.form_factor = m_current_form_factor;
	entry.directory = m_current_dir;
	m_fulllist.push_back(entry);
}

void floppy_sound_samples::add_seek_sample(const char* filename, int nominal_rate, int max_rate, int dir)
{
	add_seek_sample(filename, nominal_rate, max_rate, 0, 99, dir);
}

void floppy_sound_samples::add_seek_sample(const char* filename, int nominal_rate, int max_rate, int mintrack, int maxtrack, int dir)
{
	floppy_sound_entry entry;
	entry.type = SEEK;
	entry.rate = nominal_rate;
	entry.maxrate = max_rate;
	entry.mintrack = mintrack;
	entry.maxtrack = maxtrack;
	entry.dir = dir;
	entry.filename = filename;
	entry.form_factor = m_current_form_factor;
	entry.directory = m_current_dir;
	m_fulllist.push_back(entry);
}

const char* const* floppy_sound_samples::get_names()
{
	m_samplenames.push_back(nullptr);
	return &m_samplenames[0];
}

/*
    Find a suitable spinning sound in the list.
*/
int floppy_sound_samples::find_spin(int spintype) const
{
	// If a sample is not available (left), take the one on the right.
	// Simple index replacement.
	int replace_sample[7] =
	{
		/* START_EMPTY -> */            SPIN_EMPTY,
		/* SPIN_EMPTY -> */             SPIN_LOADED,
		/* END_EMPTY -> */              END_LOADED,
		/* START_LOADED_INITIAL -> */   START_LOADED,
		/* START_LOADED -> */           SPIN_LOADED,
		/* SPIN_LOADED -> */            QUIET,
		/* END_LOADED -> */             SPIN_LOADED
	};

	while (spintype != QUIET)
	{
		for (const floppy_sound_entry& entry : m_fulllist)
		{
			if (entry.form_factor == m_current_form_factor &&
				entry.type == SPIN &&
				entry.spintype == spintype)
				return entry.index;  // found it
		}
		// Not found, take another kind (maybe try several times)
		spintype = replace_sample[spintype];
	}
	return QUIET; // not found
}

/*
    Find a suitable step sample. The samples may be different by track.
    In the definition, the range must be specified, where (0, 99) is used for
    all tracks (all emulated drives have less than 99 tracks).
*/
int floppy_sound_samples::find_step(int track, int dir) const
{
	for (const floppy_sound_entry& entry : m_fulllist)
	{
		if (entry.form_factor == m_current_form_factor &&
			entry.type == STEP &&
			track >= entry.mintrack && track <= entry.maxtrack &&
			(entry.dir == BOTH || entry.dir == dir))
			return entry.index;  // found it
	}
	return QUIET;
}

/*
    Find a suitable seek sample. We allow for a given sample to be played
    for a rate that is in some range around that sample, defined in the list.
    That is, each seek sample defines its actual rate (e.g. 6 ms) and the
    slowest rate that it may be used for (e.g. 8 ms). If the determined rate
    is 7 ms, the 6 ms sample will be chosen, and playback will be pitched down
    by 6/7 = 0.86. If the rate is 5 ms, playback will be pitched up by 6/5 = 1.2,
    unless there is a sample for a faster rate that covers 5 ms.

    If the determined rate is slower than the maximum rate (here, 8 ms), the
    next sample will be used for a slower rate (e.g. 10 ms) if available. If
    there is no slower rate, -1 is returned. The caller should then use single
    step sounds.
*/
int floppy_sound_samples::find_seek(double rate, int track, int dir, double& pitch) const
{
	int index = QUIET;
	int maxrate = 100;

	pitch = 1.0;

	for (const floppy_sound_entry& entry : m_fulllist)
	{
		// Can the sample be used for this track?
		if (entry.form_factor == m_current_form_factor &&
			entry.type == SEEK &&
			track >= entry.mintrack &&
			track <= entry.maxtrack &&
			(entry.dir == BOTH || entry.dir == dir))
		{
			// The rate must not exceed the maxrate of the sample
			// Also, if we already found an entry with a lower maxrate,
			// skip this one
			if ((rate <= entry.maxrate) && (entry.maxrate < maxrate))
			{
				index = entry.index;
				maxrate = entry.maxrate;
				pitch = entry.rate / (double)rate;
			}
		}
	}
	return index;
}

// =================================

floppy_sound_device::floppy_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: samples_device(mconfig, FLOPPYSOUND, tag, owner, clock),
		m_sound(nullptr),
		m_samplelist(nullptr),
		m_last_track(0),
		m_last_subtrack(0),
		m_motor_on(false),
		m_with_disk(false),
		m_spin_kind(floppy_sound_samples::QUIET),
		m_spin_sample(floppy_sound_samples::QUIET),
		m_spin_samplepos(0),
		m_step_sample(floppy_sound_samples::QUIET),
		m_step_samplepos(0),
		m_seek_sample(floppy_sound_samples::QUIET),
		m_seek_samplepos(0.0),
		m_seek_pitch(1.0),
		m_seek_sound_timeout(0),
		m_last_step_time(),
		m_firstturn(true),
		m_samples_available(false),
		m_in_seek(false),
		m_step_rate(0.0)
{
	// Set up the default sample list

	// Unless labeled "constructed", all samples were recorded from real floppy drives.
	// The 3.5" floppy drive is a Sony MPF420-1.
	// The 5.25" floppy drive is a Chinon FZ502.
	// "floppy" is the subdirectory in the samples path where the following samples are stored

	m_default_samples.clear();
	m_default_samples.set_form_factor(floppy_image::FF_35, "floppy");
	m_default_samples.add_spin_sample("35_spin_start_empty", floppy_sound_samples::START_EMPTY);
	m_default_samples.add_spin_sample("35_spin_start_loaded", floppy_sound_samples::START_LOADED);
	m_default_samples.add_spin_sample("35_spin_empty", floppy_sound_samples::SPIN_EMPTY);
	m_default_samples.add_spin_sample("35_spin_loaded", floppy_sound_samples::SPIN_LOADED);
	m_default_samples.add_spin_sample("35_spin_end", floppy_sound_samples::END_LOADED);
	m_default_samples.add_step_sample("35_step_1_1");
	m_default_samples.add_seek_sample("35_seek_2ms", 2, 3);   // constructed
	m_default_samples.add_seek_sample("35_seek_6ms", 6, 9);
	m_default_samples.add_seek_sample("35_seek_12ms", 12, 15);
	m_default_samples.add_seek_sample("35_seek_20ms", 20, 50);

	m_default_samples.set_form_factor(floppy_image::FF_525, "floppy");
	m_default_samples.add_spin_sample("525_spin_start_empty", floppy_sound_samples::START_EMPTY);
	m_default_samples.add_spin_sample("525_spin_start_loaded", floppy_sound_samples::START_LOADED);
	m_default_samples.add_spin_sample("525_spin_empty", floppy_sound_samples::SPIN_EMPTY);
	m_default_samples.add_spin_sample("525_spin_loaded", floppy_sound_samples::SPIN_LOADED);
	m_default_samples.add_spin_sample("525_spin_end", floppy_sound_samples::END_LOADED);
	m_default_samples.add_step_sample("525_step_1_1");
	m_default_samples.add_seek_sample("525_seek_6ms", 6, 9);
	m_default_samples.add_seek_sample("525_seek_12ms", 12, 15);
	m_default_samples.add_seek_sample("525_seek_20ms", 20, 50);
}

void floppy_sound_device::register_for_save_states()
{
	save_item(NAME(m_last_track));
	save_item(NAME(m_last_subtrack));
	save_item(NAME(m_motor_on));
	save_item(NAME(m_with_disk));
	save_item(NAME(m_spin_kind));
	save_item(NAME(m_spin_sample));
	save_item(NAME(m_spin_samplepos));
	save_item(NAME(m_step_sample));
	save_item(NAME(m_step_samplepos));
	save_item(NAME(m_seek_sample));
	save_item(NAME(m_seek_samplepos));
	save_item(NAME(m_seek_pitch));
	save_item(NAME(m_seek_sound_timeout));
	save_item(NAME(m_firstturn));
	save_item(NAME(m_samples_available));
	save_item(NAME(m_in_seek));
	save_item(NAME(m_step_rate));
}

void floppy_sound_device::device_start()
{
	m_samples_available = false;

	// Set up floppy sound samples (for those systems which use the sound feature)
	if (m_samplelist != nullptr)
	{
		// Only load if there is a matching form factor in the list
		if (m_samplelist->get_assumed_form_factor() != 0)
		{
			set_samples_names(m_samplelist->get_names());
			LOGMASKED(LOG_CONFIG, "Loading custom samples\n");
			// Try to read the audio samples.
			m_samples_available = load_samples();
		}
	}

	// Cannot load custom samples, so try the predefined list
	if (!m_samples_available)
	{
		// The default list should always have a matching form factor
		if (m_default_samples.get_assumed_form_factor() != 0)
		{
			set_samples_names(m_default_samples.get_names());
			LOGMASKED(LOG_CONFIG, "Loading default samples\n");
			// Try to read the default audio samples
			m_samples_available = load_samples();
			m_samplelist = &m_default_samples;
		}
	}

	// If we don't have samples, don't allocate a sound stream
	if (m_samples_available)
		m_sound = stream_alloc(0, 1, clock()); // per-floppy stream

	register_for_save_states();

	m_motor_on = false;
	m_spin_kind = floppy_sound_samples::QUIET;
	m_spin_sample = floppy_sound_samples::QUIET;
	m_step_sample = floppy_sound_samples::QUIET;
	m_spin_samplepos = 0;
	m_step_samplepos = 0;
	m_seek_samplepos = 0;
	m_last_step_time = attotime::zero;
	m_in_seek = false;
	m_step_rate = 0;
	m_firstturn = true;
}


void floppy_sound_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, FLOPSPK).front_center();
	add_route(ALL_OUTPUTS, FLOPSPK, 0.5);
}

void floppy_sound_device::set_samples(floppy_sound_samples *samples, int form_factor, int maxtrack)
{
	m_samplelist = samples;
	if (m_samplelist != nullptr)
		m_samplelist->select(form_factor);

	m_default_samples.select(form_factor);
	m_max_track = maxtrack;
}

/*
    Motor sound. Select appropriate sound sample, depending on whether the
    motor is started or keeps running. Motor samples are always fully
    played.
*/
void floppy_sound_device::motor(bool running, bool withdisk)
{
	if (samples_loaded())
	{
		m_sound->update(); // required

		if ((m_spin_kind==floppy_sound_samples::QUIET
			|| m_spin_kind==floppy_sound_samples::END_EMPTY
			|| m_spin_kind==floppy_sound_samples::END_LOADED ) && running) // motor was either off or already spinning down
		{
			m_spin_samplepos = 0;
			// 3.5" floppy disks have a special first turn sound when the
			// spindle motor latch meets the central metal hub hole.
			m_spin_kind = withdisk? (m_firstturn? floppy_sound_samples::START_LOADED_INITIAL : floppy_sound_samples::START_LOADED) : floppy_sound_samples::START_EMPTY;
			m_firstturn = false;
		}
		else
		{
			// Motor has been running and is turned off now
			if ((m_spin_kind == floppy_sound_samples::SPIN_EMPTY || m_spin_kind == floppy_sound_samples::SPIN_LOADED) && !running)
			{
				m_spin_samplepos = 0;
				m_spin_kind = withdisk? floppy_sound_samples::END_LOADED : floppy_sound_samples::END_EMPTY; // go to spin down sound when loop is finished
			}
		}

		int old_sample = m_spin_sample;
		m_spin_sample = (m_spin_kind==floppy_sound_samples::QUIET)? floppy_sound_samples::QUIET : m_samplelist->find_spin(m_spin_kind);

		if (m_spin_sample == floppy_sound_samples::QUIET)
			LOGMASKED(LOG_SND, "Spin off\n");
		else
			if (m_spin_sample != old_sample)
				LOGMASKED(LOG_SND, "Spin sample = %d\n", m_spin_sample);
	}
	m_motor_on = running;
	m_with_disk = withdisk;
}

/*
    Activate the step sound.
*/
void floppy_sound_device::step(int track, int subtrack)
{
	if (samples_loaded())
	{
		m_sound->update();  // required

		int dir = 0;

		// Determine direction
		if (track >= m_last_track)
			if (track > m_last_track || subtrack > m_last_subtrack)
				dir = floppy_sound_samples::IN;

		if (track <= m_last_track)
			if (track < m_last_track || subtrack < m_last_subtrack)
				dir = floppy_sound_samples::OUT;

		m_last_track = track;
		m_last_subtrack = subtrack;

		double rate = 0;

		// Take the time only from subtrack 0 to subtrack 0
		if (subtrack == 0)
		{
			attotime now = machine().time();
			rate = (m_last_step_time == attotime::zero)? 0 : (now - m_last_step_time).as_double() * 1000;
			m_last_step_time = now;
		}

		// Wait until we can safely calculate a rate
		if (rate == 0)
			return;

		bool recalc = false;

		// Cases:
		// step, previous step sample completed (step_samplepos == 0) -> new step output
		// step, previous step sample not completed (step_samplepos > 0) ->
		//     not in seek -> determine seek sample, freeze step output
		//     in seek -> continue with seek sample
		// (seek sample timeout is set to twice the step rate)

		// If the step rate changed by more than 5%, we may have to change the
		// seek sample
		// If the track is outside of the valid range, we also have to switch the seek sound
		if (m_step_rate == 0 || track < 0 || track >= m_max_track)
		{
			recalc = true;
			m_step_rate = rate;
		}
		else
		{
			if (rate > 0 && rate < 200) // safe values
			{
				double raterel = (m_step_rate - rate) / m_step_rate;
				if (raterel < 0) raterel = -raterel;
				if (raterel > 0.05 && m_in_seek)
				{
					recalc = true;
					LOGMASKED(LOG_SND, "Step rate has changed from %.1f to %.1f ms\n", m_step_rate, rate);
				}
				m_step_rate = rate;
			}
		}

		if (m_step_samplepos > 0 && m_step_rate < 100)   // in seek, or transitioning into seek
		{
			if (recalc || !m_in_seek)
			{
				int newseek = m_samplelist->find_seek(m_step_rate, track, dir, m_seek_pitch);

				// If we get a QUIET, then there is no matching seek sample,
				// i.e. the step interval became too long for a seek; we have an isolated step sound

				if (newseek != floppy_sound_samples::QUIET)
				{
					// Start the new seek sound from the beginning (but only if
					// we changed it, or we will get ugly sounds in the output)
					if (newseek != m_seek_sample) m_seek_samplepos = 0;

					LOGMASKED(LOG_SND_DETAIL, "Step rate = %.1f ms, seek sample = %d, pitch = %f\n", m_step_rate, newseek, m_seek_pitch);
				}
				m_seek_sample = newseek;
			}
		}
		else
		{
			m_in_seek = false;
			// Last step sample was completed, this is not a seek process
			m_seek_sample = floppy_sound_samples::QUIET;
			m_seek_samplepos = 0;
		}

		// If we have a single step (outside of a seek), reset the step sample position
		if (m_seek_sample == floppy_sound_samples::QUIET)
		{
			m_step_sample = m_samplelist->find_step(track, dir);
			m_step_samplepos = 0;
			m_in_seek = false;
			LOGMASKED(LOG_SND_DETAIL, "Step rate = %.1f ms\n", m_step_rate);
		}
		else
		{
			// Keep the position of the current step sample and
			// enter or remain in seek mode
			m_in_seek = true;

			// Also keep the current step_sample for a later resume

			// Set the timeout for the seek sound. When it expires,
			// we assume that the seek process is over, and we'll play the
			// rest of the step sound.
			// This will be retriggered with each step pulse.
			// For rapid steps, set a minimum of 20 ms for the timeout.
			m_seek_sound_timeout = (m_step_rate < 10)? 20 : (m_step_rate * 2);

			// Number of updates with 44100 Hz per millisecond (rounded)
			// Will be decremented by one for each update
			m_seek_sound_timeout *= 44;
		}
	}
}

//-------------------------------------------------
//  sound_stream_update - update the sound stream
//-------------------------------------------------

void floppy_sound_device::sound_stream_update(sound_stream &stream)
{
	// We are using only one stream, unlike the parent class
	// Also, there is no need for interpolation, as we only expect
	// one sample rate of 44100 for all samples

	int16_t out;
	int sampleend = 0;

	for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
	{
		out = 0;

		// Motor sound
		if (m_spin_sample != floppy_sound_samples::QUIET)
		{
			// The samples list starts at 0 with the first entry after DIR,
			// so we adjust by -1
			sampleend = m_sample[m_spin_sample-1].data.size();
			out = m_sample[m_spin_sample-1].data[m_spin_samplepos++];

			if (m_spin_samplepos >= sampleend)
			{
				// LOGMASKED(LOG_SND_DETAIL, "Spin sample %d completed\n", m_spin_sample);
				// Motor sample has completed
				switch (m_spin_kind)
				{
				case floppy_sound_samples::START_EMPTY:
					// After start, switch to the continued spinning sound
					m_spin_kind = floppy_sound_samples::SPIN_EMPTY; // move to looping sound
					break;
				case floppy_sound_samples::START_LOADED:
				case floppy_sound_samples::START_LOADED_INITIAL:
					// After start, switch to the continued spinning sound
					m_spin_kind = floppy_sound_samples::SPIN_LOADED; // move to looping sound
					break;
				case floppy_sound_samples::SPIN_EMPTY:
					// As long as the motor pin is asserted, restart the sample
					// play the spindown sample
					if (!m_motor_on) m_spin_kind = floppy_sound_samples::END_EMPTY; // motor was turned off already (during spin-up maybe) -> spin down
					break;
				case floppy_sound_samples::SPIN_LOADED:
					if (!m_motor_on) m_spin_kind = floppy_sound_samples::END_LOADED; // motor was turned off already (during spin-up maybe) -> spin down
					break;
				case floppy_sound_samples::END_EMPTY:
				case floppy_sound_samples::END_LOADED:
					// Spindown sample over, be quiet or restart if the
					// motor has been restarted
					if (m_motor_on)
					{
						LOGMASKED(LOG_SND_DETAIL, "Restart spinning sound\n");
						m_spin_kind = m_with_disk ? floppy_sound_samples::START_LOADED : floppy_sound_samples::START_EMPTY;
					}
					else
						m_spin_kind = floppy_sound_samples::QUIET;
					break;

				default:
					break;
				}

				int old_sample = m_spin_sample;
				m_spin_sample = (m_spin_kind==floppy_sound_samples::QUIET)? floppy_sound_samples::QUIET : m_samplelist->find_spin(m_spin_kind);

				if (m_spin_sample == floppy_sound_samples::QUIET)
					LOGMASKED(LOG_SND, "Spin off\n");
				else
					if (m_spin_sample != old_sample)
						LOGMASKED(LOG_SND, "Spin sample = %d\n", m_spin_sample);

				// Restart the selected sample
				m_spin_samplepos = 0;
			}
		}

		// Seek sound
		// As long as we have a seek sound, there is a pending step sound
		if (m_seek_sound_timeout == 1)
		{
			LOGMASKED(LOG_SND_DETAIL, "Seek end, resume step sound\n");
			// Not retriggered; switch back to the last step sound
			m_seek_sample = floppy_sound_samples::QUIET;
			m_seek_sound_timeout = 0;
			// Skip 1/100 sec to dampen the loudest pulse
			// yep, a somewhat dirty trick; we don't have to record yet another sample
			m_step_samplepos += 441;
		}

		if (m_seek_sample != floppy_sound_samples::QUIET)
		{
			m_seek_sound_timeout--;

			sampleend = m_sample[m_seek_sample-1].data.size();

			// Mix it into the stream value
			out += m_sample[m_seek_sample-1].data[(int)m_seek_samplepos];

			// By adding different values than 1, we can change the playback speed
			// This will be used to adjust the seek sound
			m_seek_samplepos += m_seek_pitch;

			// The seek sample will be replayed without interrupt
			if (m_seek_samplepos >= sampleend)
				m_seek_samplepos = 0;
		}
		else
		{
			// Stepper sound
			if (m_step_sample != floppy_sound_samples::QUIET)
			{
				sampleend = m_sample[m_step_sample-1].data.size();

				// Mix it into the stream value
				if (m_step_samplepos < sampleend)
					out += m_sample[m_step_sample-1].data[m_step_samplepos++];
				if (m_step_samplepos >= sampleend)
				{
					// Step sample done
					m_step_samplepos = 0;
					m_step_sample = floppy_sound_samples::QUIET;
					LOGMASKED(LOG_SND_DETAIL, "Step sample completed\n");
				}
			}
		}

		// Write to the stream buffer
		stream.put_int(0, sampindex, out, 32768);
	}
}
