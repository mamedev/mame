// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    samples.h

    Sound device for sample playback.

***************************************************************************/

#pragma once

#ifndef __SAMPLES_H__
#define __SAMPLES_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SAMPLES_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, SAMPLES, 0) \
	samples_device::static_set_interface(*device, _interface);

#define MCFG_SAMPLES_REPLACE(_tag, _interface) \
	MCFG_DEVICE_REPLACE(_tag, SAMPLES, 0) \
	samples_device::static_set_interface(*device, _interface);


#define SAMPLES_START(name) void name(samples_device &device)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class samples_device;


// ======================> samples_interface sample

struct samples_interface
{
	UINT8       m_channels;         // number of discrete audio channels needed
	const char *const *m_names;     // array of sample names
	void        (*m_start)(samples_device &device); // optional callback
};


// ======================> samples_device

class samples_device :  public device_t,
						public device_sound_interface,
						public samples_interface
{
public:
	// construction/destruction
	samples_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	static void static_set_interface(device_t &device, const samples_interface &interface);

	// getters
	bool playing(UINT8 channel) const;
	UINT32 base_frequency(UINT8 channel) const;

	// start/stop helpers
	void start(UINT8 channel, UINT32 samplenum, bool loop = false);
	void start_raw(UINT8 channel, const INT16 *sampledata, UINT32 samples, UINT32 frequency, bool loop = false);
	void pause(UINT8 channel, bool pause = true);
	void stop(UINT8 channel);
	void stop_all();

	// dynamic control
	void set_frequency(UINT8 channel, UINT32 frequency);
	void set_volume(UINT8 channel, float volume);

	// helpers
	struct sample_t
	{
		// shouldn't need a copy, but in case it happens, catch it here
		sample_t &operator=(const sample_t &rhs) { assert(false); return *this; }

		UINT32          frequency;      // frequency of the sample
		dynamic_array<INT16> data;      // 16-bit signed data
	};
	static bool read_sample(emu_file &file, sample_t &sample);

protected:
	// subclasses can do it this way
	samples_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_post_load();

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

private:
	// internal classes
	struct channel_t
	{
		sound_stream *  stream;
		const INT16 *   source;
		INT32           source_length;
		INT32           source_num;
		UINT32          pos;
		UINT32          frac;
		UINT32          step;
		UINT32          basefreq;
		bool            loop;
		bool            paused;
	};

	// internal helpers
	static bool read_wav_sample(emu_file &file, sample_t &sample);
	static bool read_flac_sample(emu_file &file, sample_t &sample);
	void load_samples();

	// internal state
	dynamic_array<channel_t>    m_channel;
	dynamic_array<sample_t>     m_sample;

	// internal constants
	static const UINT8 FRAC_BITS = 24;
	static const UINT32 FRAC_ONE = 1 << FRAC_BITS;
	static const UINT32 FRAC_MASK = FRAC_ONE - 1;
};

// iterator, since lots of people are interested in these devices
typedef device_type_iterator<&device_creator<samples_device>, samples_device> samples_device_iterator;


// ======================> samples_iterator

class samples_iterator
{
public:
	// construction/destruction
	samples_iterator(samples_interface &intf)
		: m_intf(intf),
			m_current(-1) { }

	// getters
	const char *altbasename() const { return (m_intf.m_names != NULL && m_intf.m_names[0] != NULL && m_intf.m_names[0][0] == '*') ? &m_intf.m_names[0][1] : NULL; }

	// iteration
	const char *first()
	{
		if (m_intf.m_names == NULL || m_intf.m_names[0] == NULL)
			return NULL;
		m_current = 0;
		if (m_intf.m_names[0][0] == '*')
			m_current++;
		return m_intf.m_names[m_current++];
	}

	const char *next()
	{
		if (m_current == -1 || m_intf.m_names[m_current] == NULL)
			return NULL;
		return m_intf.m_names[m_current++];
	}

	// counting
	int count()
	{
		int save = m_current;
		int result = 0;
		for (const char *scan = first(); scan != NULL; scan = next())
			result++;
		m_current = save;
		return result;
	}

private:
	// internal state
	samples_interface &     m_intf;
	int                     m_current;
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
extern const device_type SAMPLES;


#endif
