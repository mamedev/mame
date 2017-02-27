// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    samples.h

    Sound device for sample playback.

***************************************************************************/

#ifndef MAME_DEVICES_SOUND_SAMPLES_H
#define MAME_DEVICES_SOUND_SAMPLES_H

#pragma once


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
extern const device_type SAMPLES;



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SAMPLES_CHANNELS(_channels) \
	samples_device::static_set_channels(*device, _channels);

#define MCFG_SAMPLES_NAMES(_names) \
	samples_device::static_set_samples_names(*device, _names);

typedef device_delegate<void ()> samples_start_cb_delegate;

#define SAMPLES_START_CB_MEMBER(_name) void _name()

#define MCFG_SAMPLES_START_CB(_class, _method) \
	samples_device::set_samples_start_callback(*device, samples_start_cb_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> samples_device

class samples_device :  public device_t,
						public device_sound_interface
{
public:
	// construction/destruction
	samples_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration helpers
	static void static_set_channels(device_t &device, uint8_t channels) { downcast<samples_device &>(device).m_channels = channels; }
	static void static_set_samples_names(device_t &device, const char *const *names) { downcast<samples_device &>(device).m_names = names; }
	static void set_samples_start_callback(device_t &device, samples_start_cb_delegate callback) { downcast<samples_device &>(device).m_samples_start_cb = callback; }

	// getters
	bool playing(uint8_t channel) const;
	uint32_t base_frequency(uint8_t channel) const;

	// start/stop helpers
	void start(uint8_t channel, uint32_t samplenum, bool loop = false);
	void start_raw(uint8_t channel, const int16_t *sampledata, uint32_t samples, uint32_t frequency, bool loop = false);
	void pause(uint8_t channel, bool pause = true);
	void stop(uint8_t channel);
	void stop_all();

	// dynamic control
	void set_frequency(uint8_t channel, uint32_t frequency);
	void set_volume(uint8_t channel, float volume);

	// helpers
	struct sample_t
	{
		// shouldn't need a copy, but in case it happens, catch it here
		sample_t &operator=(const sample_t &rhs) { assert(false); return *this; }

		uint32_t          frequency;      // frequency of the sample
		std::vector<int16_t> data;      // 16-bit signed data
	};
	static bool read_sample(emu_file &file, sample_t &sample);

	// interface
	uint8_t       m_channels;         // number of discrete audio channels needed
	const char *const *m_names;     // array of sample names
	samples_start_cb_delegate m_samples_start_cb; // optional callback

protected:
	// subclasses can do it this way
	samples_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	// internal classes
	struct channel_t
	{
		sound_stream *  stream;
		const int16_t *   source;
		int32_t           source_length;
		int32_t           source_num;
		uint32_t          pos;
		uint32_t          frac;
		uint32_t          step;
		uint32_t          basefreq;
		bool            loop;
		bool            paused;
	};

	// internal helpers
	static bool read_wav_sample(emu_file &file, sample_t &sample);
	static bool read_flac_sample(emu_file &file, sample_t &sample);
	bool load_samples();

	// internal state
	std::vector<channel_t>    m_channel;
	std::vector<sample_t>     m_sample;

	// internal constants
	static const uint8_t FRAC_BITS = 24;
	static const uint32_t FRAC_ONE = 1 << FRAC_BITS;
	static const uint32_t FRAC_MASK = FRAC_ONE - 1;
};

// iterator, since lots of people are interested in these devices
typedef device_type_iterator<SAMPLES, samples_device> samples_device_iterator;


// ======================> samples_iterator

class samples_iterator
{
public:
	// construction/destruction
	samples_iterator(samples_device &device)
		: m_samples(device)
		, m_current(-1)
	{
	}

	// getters
	const char *altbasename() const { return (m_samples.m_names != nullptr && m_samples.m_names[0] != nullptr && m_samples.m_names[0][0] == '*') ? &m_samples.m_names[0][1] : nullptr; }

	// iteration
	const char *first()
	{
		if (m_samples.m_names == nullptr || m_samples.m_names[0] == nullptr)
			return nullptr;
		m_current = 0;
		if (m_samples.m_names[0][0] == '*')
			m_current++;
		return m_samples.m_names[m_current++];
	}

	const char *next()
	{
		if (m_current == -1 || m_samples.m_names[m_current] == nullptr)
			return nullptr;
		return m_samples.m_names[m_current++];
	}

	// counting
	int count()
	{
		int save = m_current;
		int result = 0;
		for (const char *scan = first(); scan != nullptr; scan = next())
			result++;
		m_current = save;
		return result;
	}

private:
	// internal state
	samples_device &m_samples;
	int                     m_current;
};


#endif // MAME_DEVICES_SOUND_SAMPLES_H
