// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/*********************************************************************

    floppysound.h

    MZ, August 2015
    Updated April 2026

    In order to activate floppy drive sounds with predefined samples for 3.5"
    and 5.25" drives, call

    * enable_sound() or enable_sound(true) or enable_sound(nullptr)

    on the instances of floppy_connector, usually appearing in device_add_mconfig
    of the device where the drives are connected. If you prefer custom sounds
    for the drive, create an instance of floppy_sound_samples, and register
    samples on it as follows:

    * clear()
        Clear the sample list. Recommended to use at code locations that
        may be called several times (like device_add_mconfig).

    * set_form_factor(form_factor, directory)
        All following add operations will use the given form factor and
        assume that the samples are found in the provided directory. May be
        called several times in order to add samples for different form factors.

    * add_spin_sample(filename, type):
        For spinning motor samples. See the enum below for type values.

    * add_step_sample(filename, start, end, dir):
        Stepper sound for single steps, used in the track range from start to
        end; when start and end are omitted, 0 and 99 are assumed, covering the
        whole disk. The dir parameter can be used to distinguish between steps
        towards the center or towards the rim.

    * add_seek_sample(filename, nominal_rate, max_rate, start, end, dir):
        Stepper sound for continuous movement for a rate not exceeding max_rate.
        The pitch is adjusted according to the ratio of the actual rate and
        the nominal rate, thus, the sample is played back at natural speed when
        the actual rate matches the nominal rate. The sample is selected whose
        maximum rate is the minimum among those whose maximum rate is higher
        than the actual rate, and if its range contains the current track number.
        When not specified, the range covers the whole disk (0..99).
        The dir parameter can be used to distinguish between seeks
        towards the center or towards the rim.

    For an example, see the predefined sample list in the constructor of
    floppy_sound_device.

    For custom samples, pass the address of the specific floppy_sound_samples
    instance as

    *  enable_sound(&myfloppysamples);

    If the custom samples cannot be found, the default samples are used. If
    those cannot be found either, sound is disabled.

    If the samples list does not contain a matching form factor, the following
    replacement strategy is used:

    * If 3" samples are requested but not found, 3.5" samples are used.
    * If 3.5" or 8" samples are requested but not found, 5.25" samples are used.

*********************************************************************/

#ifndef MAME_SOUND_DRIVESOUND_H
#define MAME_SOUND_DRIVESOUND_H

#pragma once

#include "samples.h"

class floppy_sound_samples
{
public:
	floppy_sound_samples();

	/* Clear the list. */
	void clear() { m_fulllist.clear(); }

	/* Set the form factor for the following add operations. */
	void set_form_factor(int form_factor, const char* dir);

	enum  // spin type
	{
		QUIET=-1,               // Also used as silence for steps and seeks
		START_EMPTY=0,          // Start spinning without disk
		SPIN_EMPTY,             // Spinning without disk
		END_EMPTY,              // Stop spinning spinning without disk
		START_LOADED_INITIAL,   // Start spinning with disk, 3.5" drives make a click when latching in
		START_LOADED,           // Start spinning with disk, already latched in
		SPIN_LOADED,            // Spinning with disk (mandatory sample)
		END_LOADED              // Stop spinning with disk
	};

	enum // direction
	{
		BOTH=0,
		IN,
		OUT
	};

	/* Add spin, step, and seek samples. */
	void add_spin_sample(const char* filename, int type);
	void add_step_sample(const char* filename, int dir=BOTH);
	void add_step_sample(const char* filename, int start, int end, int dir=BOTH);
	void add_seek_sample(const char* filename, int nominal_rate, int max_rate, int dir=BOTH);
	void add_seek_sample(const char* filename, int nominal_rate, int max_rate, int mintrack, int maxtrack, int dir=BOTH);

	/* Deliver the list of names for the parent class samples_device. */
	const char* const* get_names();

	/* Selects the matching form factor and prepares the samples list. */
	void select(int form_factor);
	int get_assumed_form_factor() { return m_current_form_factor; }

	/* Search for a suitable spinning sample. Return the index into the
	   samples list. */
	int find_spin(int kind) const;

	/* Search for a suitable step sample. */
	int find_step(int track, int dir) const;

	/* Search for a suitable seek sample. */
	int find_seek(double rate, int track, int dir, double& pitch) const;

private:
	enum
	{
		SPIN = 0,
		STEP,
		SEEK
	};

	struct floppy_sound_entry
	{
		int index = 0;
		int type = 0;        // type: SPIN, STEP, SEEK
		int form_factor;     // indicates the form factor of the drive
		int mintrack = 0;    // valid from here (including), meaningless for spin entries
		int maxtrack = 99;   // to here (including), meaningless for spin entries
		int rate = 0;        // rate of the seek sample
		int maxrate = 0;     // max rate for pitching up the seek sample
		int spintype = 0;    // type for spin entries
		int dir = BOTH;      // Direction of the seek or step
		const char *directory;  // directory where the sample is stored
		const char *filename;
	};

	std::string m_basedir;          // Subdirectory which contains the samples
	std::vector<const char*> m_samplenames;

	std::vector<floppy_sound_entry> m_fulllist;

	int m_current_form_factor;
	const char* m_current_dir;
};

class floppy_sound_device : public samples_device
{
public:
	floppy_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void motor(bool on, bool withdisk);
	void step(int track, int subtrack=0);
	void unload() { m_firstturn = true; }
	bool samples_loaded() { return m_samples_available; }
	void register_for_save_states();
	void set_samples(floppy_sound_samples *samples, int form_factor, int maxtrack);

protected:
	void device_start() override ATTR_COLD;
	void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream) override;
	sound_stream*   m_sound;

	TIMER_CALLBACK_MEMBER(spin_start_timeout);
	attotime spin_start_delay() const;

	floppy_sound_samples* m_samplelist;
	floppy_sound_samples m_default_samples;

	int    m_max_track;
	int    m_last_track;
	int    m_last_subtrack;

	emu_timer* m_spin_start_timer;   // delay between motor on and the spin-up sample actually starting
	bool   m_spin_start_withdisk;    // withdisk value pending on m_spin_start_timer

	bool   m_motor_on;
	bool   m_with_disk;
	int    m_spin_kind;
	int    m_spin_sample;
	int    m_spin_samplepos;
	int    m_step_sample;
	int    m_step_samplepos;
	int    m_seek_sample;
	double m_seek_samplepos;    // we may using a non-integer pitch
	double m_seek_pitch;
	int    m_seek_sound_timeout;
	attotime m_last_step_time;
	bool   m_firstturn;           // see START_LOADED_INITIAL
	bool   m_samples_available;
	bool   m_in_seek;
	double m_step_rate;
};

DECLARE_DEVICE_TYPE(FLOPPYSOUND, floppy_sound_device)

#endif // MAME_SOUND_DRIVESOUND_H
