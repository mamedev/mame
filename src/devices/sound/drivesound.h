// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/*********************************************************************

    drivesound.h

    MZ, August 2015
    Updated September 2026

    In order to activate floppy drive sounds with predefined samples for 3.5"
    and 5.25" drives, call

     * enable_sound() or enable_sound(true) or enable_sound(nullptr)
    
    on the instances of floppy_connector, usually appearing in device_add_mconfig
    of the device where the drives are connected. If you prefer custom sounds
    for the drive, call
    
     * enable_sound(const char* key)
     
    where key refers to the value of the name attribute which belongs to some
    element in the floppy.xml file. The first element whose form factor
    matches the current drive and whose name attribute matches the key is
    selected.    
    
    Document type definition:
	
	<!ELEMENT drivesound (drive)+>
	<!ELEMENT drive (spin | step | seek)+>
	
	<!ELEMENT spin EMPTY>
	<!ELEMENT step EMPTY>
	<!ELEMENT seek EMPTY>
	
	-------------
	<!ATTLIST drive name CDATA #REQUIRED>
	<!ATTLIST drive description CDATA #IMPLIED>
	<!ATTLIST drive path CDATA #REQUIRED>
	<!ATTLIST drive formfactor (3 | 3.5 | 5.25 | 8) #REQUIRED>
	----- or -----
	<!ATTLIST drive base CDATA #REQUIRED>
	-------------
	
	<!ATTLIST spin phase (start | run | stop) #REQUIRED>
	<!ATTLIST spin mode (empty | loaded) #REQUIRED>
	<!ATTLIST spin file CDATA #REQUIRED>
	
	<!ATTLIST step from CDATA #IMPLIED>
	<!ATTLIST step to CDATA #IMPLIED>
    <!ATTLIST step dir (in | out | both) #IMPLIED>
	<!ATTLIST step file CDATA #REQUIRED>
	
	<!ATTLIST seek from CDATA #IMPLIED>
	<!ATTLIST seek to CDATA #IMPLIED>
    <!ATTLIST seek nomrate CDATA #REQUIRED>
    <!ATTLIST seek maxrate CDATA #REQUIRED>
    <!ATTLIST seek dir (in | out | both) #IMPLIED>
	<!ATTLIST seek file CDATA #REQUIRED>
    
    If the custom samples cannot be found, the default samples are used. If
    those cannot be found either, sound is disabled.

    If the samples list does not contain a matching form factor, the following
    replacement strategy is used:

    * If 3" samples are requested but not found, 3.5" samples are used.
    * If 3.5" or 8" samples are requested but not found, 5.25" samples are used.
    
    Attributes:

    name: Key by which this sample set is referred (should be unique per form factor)
    description: Plain text description, shown in log
    path: subdirectory in the samples path where the samples are stored
    formfactor: Drive form factor for which this sample set applies
    base: Value of name attribute of another drive element or "none"
    
    from / to: Range of tracks for which this sample applies (default: 0/99). May
               be negative or outside of physical range.
    nomrate: Rate in milliseconds of head steps in this seek sample
    maxrate: Longest rate for which this sample may be pitched down
    in / out: Direction of head movement (towards center / rim)
    
*********************************************************************/

#ifndef MAME_SOUND_DRIVESOUND_H
#define MAME_SOUND_DRIVESOUND_H

#pragma once

#include "samples.h"

class floppy_sound_samples
{
public:
	floppy_sound_samples();

	/* Clear the sample list. Recommended to be used at code locations that
       may be called several times (like device_add_mconfig). 
    */
	void clear() { m_fulllist.clear(); }

	/* Set the form factor for the following add operations. 
	   All following add operations assume that the samples are found in the 
	   provided directory. May be called several times in order to add samples 
	   for different form factors.
	*/
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

	/*  For spinning motor samples. See the spin type enum for type values. */
	void add_spin_sample(const char* filename, int type);
	
	/*  Stepper sound for single steps, used in the track range from start to
        end; when start and end are omitted, 0 and 99 are assumed, covering the
        whole disk. The dir parameter can be used to distinguish between steps
        towards the center or towards the rim.
    */
	void add_step_sample(const char* filename, int dir=BOTH);
	void add_step_sample(const char* filename, int start, int end, int dir=BOTH);
		
	/*  Stepper sound for continuous movement for a rate not exceeding max_rate.
        The pitch is adjusted according to the ratio of the actual rate and
        the nominal rate, thus, the sample is played back at natural speed when
        the actual rate matches the nominal rate. The sample is selected whose
        maximum rate is the minimum among those whose maximum rate is higher
        than the actual rate, and if its range contains the current track number.
        When not specified, the range covers the whole disk (0..99).
        The dir parameter can be used to distinguish between seeks
        towards the center or towards the rim.
    */
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

	/* Append this sample collection to the one in the argument. */
	void append_to(floppy_sound_samples&);

	/* Delivers the length of the sample list. */
	int count() { return m_fulllist.size(); }

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
		std::string directory;  // directory where the sample is stored
		std::string filename;
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
	void set_samples(const char *name, int form_factor, int maxtrack);

protected:
	void device_start() override ATTR_COLD;
	void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	bool load_xml(emu_file &file, int maxtrack, const char* devname);

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream) override;
	sound_stream*   m_sound;

	floppy_sound_samples m_samples;

	int    m_max_track;
	int    m_last_track;
	int    m_last_subtrack;

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
