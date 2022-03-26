// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    samples.c

    Sound device for sample playback.

****************************************************************************

    Playback of pre-recorded samples. Used for high-level simulation of
    discrete sound circuits where proper low-level simulation isn't
    available.  Also used for tape loops and similar.

    Current limitations
      - Only supports single channel samples!

    Considerations
      - Maybe this should be part of the presentation layer
        (artwork etc.) with samples specified in .lay files instead of
        in drivers?

***************************************************************************/

#include "emu.h"
#include "samples.h"

#include "emuopts.h"
#include "fileio.h"

#include "flac.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(SAMPLES, samples_device, "samples", "Samples")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  samples_device - constructors
//-------------------------------------------------

samples_device::samples_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: samples_device(mconfig, SAMPLES, tag, owner, clock)
{
}

samples_device::samples_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_channels(0)
	, m_names(nullptr)
	, m_samples_start_cb(*this)
{
}


//**************************************************************************
//  PUBLIC INTERFACE
//**************************************************************************

//-------------------------------------------------
//  start - start playing a loaded sample
//-------------------------------------------------

void samples_device::start(uint8_t channel, uint32_t samplenum, bool loop)
{
	// if samples are disabled, just return quietly
	if (m_sample.empty())
		return;

	assert(samplenum < m_sample.size());
	assert(channel < m_channels);

	// force an update before we start
	channel_t &chan = m_channel[channel];
	chan.stream->update();

	// update the parameters
	sample_t &sample = m_sample[samplenum];
	chan.source = (sample.data.size() > 0) ? &sample.data[0] : nullptr;
	chan.source_num = (chan.source_len > 0) ? samplenum : -1;
	chan.source_len = sample.data.size();
	chan.pos = 0;
	chan.basefreq = sample.frequency;
	chan.curfreq = sample.frequency;
	chan.loop = loop;
}


//-------------------------------------------------
//  start_raw - start playing an externally
//  provided sample
//-------------------------------------------------

void samples_device::start_raw(uint8_t channel, const int16_t *sampledata, uint32_t samples, uint32_t frequency, bool loop)
{
	assert(channel < m_channels);

	// force an update before we start
	channel_t &chan = m_channel[channel];
	chan.stream->update();

	// update the parameters
	chan.source = sampledata;
	chan.source_num = -1;
	chan.source_len = samples;
	chan.pos = 0;
	chan.basefreq = frequency;
	chan.curfreq = frequency;
	chan.loop = loop;
}


//-------------------------------------------------
//  set_frequency - set the playback frequency of
//  a sample
//-------------------------------------------------

void samples_device::set_frequency(uint8_t channel, uint32_t freq)
{
	assert(channel < m_channels);

	// force an update before we start
	channel_t &chan = m_channel[channel];
	chan.stream->update();
	chan.curfreq = freq;
}


//-------------------------------------------------
//  set_volume - set the playback volume of a
//  sample
//-------------------------------------------------

void samples_device::set_volume(uint8_t channel, float volume)
{
	assert(channel < m_channels);

	// force an update before we start
	set_output_gain(channel, volume);
}


//-------------------------------------------------
//  pause - pause playback on a channel
//-------------------------------------------------

void samples_device::pause(uint8_t channel, bool pause)
{
	assert(channel < m_channels);

	// force an update before we start
	channel_t &chan = m_channel[channel];
	chan.paused = pause;
}


//-------------------------------------------------
//  stop - stop playback on a channel
//-------------------------------------------------

void samples_device::stop(uint8_t channel)
{
	assert(channel < m_channels);

	// force an update before we start
	channel_t &chan = m_channel[channel];
	chan.source = nullptr;
	chan.source_num = -1;
}


//-------------------------------------------------
//  stop_all - stop playback on all channels
//-------------------------------------------------

void samples_device::stop_all()
{
	// just iterate over channels and stop them
	for (uint8_t channel = 0; channel < m_channels; channel++)
		stop(channel);
}


//-------------------------------------------------
//  base_frequency - return the base frequency of
//  a channel being played
//-------------------------------------------------

uint32_t samples_device::base_frequency(uint8_t channel) const
{
	assert(channel < m_channels);
	return m_channel[channel].basefreq;
}


//-------------------------------------------------
//  playing - return true if a sample is still
//  playing on the given channel
//-------------------------------------------------

bool samples_device::playing(uint8_t channel) const
{
	assert(channel < m_channels);

	// force an update before we start
	const channel_t &chan = m_channel[channel];
	chan.stream->update();
	return (chan.source != nullptr);
}



//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_start - handle device startup
//-------------------------------------------------

void samples_device::device_start()
{
	// read audio samples
	load_samples();

	// allocate channels
	m_channel.resize(m_channels);
	for (int channel = 0; channel < m_channels; channel++)
	{
		// initialize channel
		channel_t &chan = m_channel[channel];
		chan.stream = stream_alloc(0, 1, SAMPLE_RATE_OUTPUT_ADAPTIVE);
		chan.source = nullptr;
		chan.source_num = -1;
		chan.pos = 0;
		chan.loop = 0;
		chan.paused = 0;

		// register with the save state system
		save_item(NAME(chan.source_num), channel);
		save_item(NAME(chan.source_len), channel);
		save_item(NAME(chan.pos), channel);
		save_item(NAME(chan.loop), channel);
		save_item(NAME(chan.paused), channel);
	}

	// initialize any custom handlers
	m_samples_start_cb.resolve();

	if (!m_samples_start_cb.isnull())
		m_samples_start_cb();
}


//-------------------------------------------------
//  device_reset - handle device reset
//-------------------------------------------------

void samples_device::device_reset()
{
	stop_all();
}


//-------------------------------------------------
//  device_post_load - handle updating after a
//  restore
//-------------------------------------------------

void samples_device::device_post_load()
{
	// loop over channels
	for (int channel = 0; channel < m_channels; channel++)
	{
		// attach any samples that were loaded and playing
		channel_t &chan = m_channel[channel];
		if (chan.source_num >= 0 && chan.source_num < m_sample.size())
		{
			sample_t &sample = m_sample[chan.source_num];
			chan.source = &sample.data[0];
			chan.source_len = sample.data.size();
			if (sample.data.empty())
				chan.source_num = -1;
		}

		// validate the position against the length in case the sample is smaller
		double endpos = chan.source_len;
		if (chan.source != nullptr && chan.pos >= endpos)
		{
			if (chan.loop)
			{
				double posfloor = floor(chan.pos);
				chan.pos -= posfloor;
				chan.pos += double(int32_t(posfloor) % chan.source_len);
			}
			else
			{
				chan.source = nullptr;
				chan.source_num = -1;
			}
		}
	}
}


//-------------------------------------------------
//  sound_stream_update - update a sound stream
//-------------------------------------------------

void samples_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	// find the channel with this stream
	constexpr stream_buffer::sample_t sample_scale = 1.0 / 32768.0;
	for (int channel = 0; channel < m_channels; channel++)
		if (&stream == m_channel[channel].stream)
		{
			channel_t &chan = m_channel[channel];
			auto &buffer = outputs[0];

			// process if we still have a source and we're not paused
			if (chan.source != nullptr && !chan.paused)
			{
				// load some info locally
				double step = double(chan.curfreq) / double(buffer.sample_rate());
				double endpos = chan.source_len;
				const int16_t *sample = chan.source;

				for (int sampindex = 0; sampindex < buffer.samples(); sampindex++)
				{
					// do a linear interp on the sample
					double pos_floor = floor(chan.pos);
					double frac = chan.pos - pos_floor;
					int32_t ipos = int32_t(pos_floor);

					stream_buffer::sample_t sample1 = stream_buffer::sample_t(sample[ipos++]);
					stream_buffer::sample_t sample2 = stream_buffer::sample_t(sample[(ipos + 1) % chan.source_len]);
					buffer.put(sampindex, sample_scale * ((1.0 - frac) * sample1 + frac * sample2));

					// advance
					chan.pos += step;

					// handle looping/ending
					if (chan.pos >= endpos)
					{
						if (chan.loop)
							chan.pos -= endpos;
						else
						{
							chan.source = nullptr;
							chan.source_num = -1;
							buffer.fill(0, sampindex);
							break;
						}
					}
				}
			}
			else
				buffer.fill(0);
			break;
		}
}



//**************************************************************************
//  INTERNAL HELPERS
//**************************************************************************

//-------------------------------------------------
//  read_sample - read a WAV or FLAC file as a
//  sample
//-------------------------------------------------

bool samples_device::read_sample(emu_file &file, sample_t &sample)
{
	// read the core header and make sure it's a proper file
	uint8_t buf[4];
	uint32_t offset = file.read(buf, 4);
	if (offset < 4)
	{
		osd_printf_warning("Unable to read %s, 0-byte file?\n", file.filename());
		return false;
	}

	// look for the appropriate RIFF tag
	if (memcmp(&buf[0], "RIFF", 4) == 0)
		return read_wav_sample(file, sample);
	else if (memcmp(&buf[0], "fLaC", 4) == 0)
		return read_flac_sample(file, sample);

	// if nothing appropriate, emit a warning
	osd_printf_warning("Unable to read %s, corrupt file?\n", file.filename());
	return false;
}


//-------------------------------------------------
//  read_wav_sample - read a WAV file as a sample
//-------------------------------------------------

bool samples_device::read_wav_sample(emu_file &file, sample_t &sample)
{
	// we already read the opening 'RIFF' tag
	uint32_t offset = 4;

	// get the total size
	uint32_t filesize;
	offset += file.read(&filesize, 4);
	if (offset < 8)
	{
		osd_printf_warning("Unexpected size offset %u (%s)\n", offset, file.filename());
		return false;
	}
	filesize = little_endianize_int32(filesize);

	// read the RIFF file type and make sure it's a WAVE file
	char buf[32];
	offset += file.read(buf, 4);
	if (offset < 12)
	{
		osd_printf_warning("Unexpected WAVE offset %u (%s)\n", offset, file.filename());
		return false;
	}
	if (memcmp(&buf[0], "WAVE", 4) != 0)
	{
		osd_printf_warning("Could not find WAVE header (%s)\n", file.filename());
		return false;
	}

	// seek until we find a format tag
	uint32_t length;
	while (1)
	{
		offset += file.read(buf, 4);
		offset += file.read(&length, 4);
		length = little_endianize_int32(length);
		if (memcmp(&buf[0], "fmt ", 4) == 0)
			break;

		// seek to the next block
		file.seek(length, SEEK_CUR);
		offset += length;
		if (offset >= filesize)
		{
			osd_printf_warning("Could not find fmt tag (%s)\n", file.filename());
			return false;
		}
	}

	// read the format -- make sure it is PCM
	uint16_t temp16;
	offset += file.read(&temp16, 2);
	temp16 = little_endianize_int16(temp16);
	if (temp16 != 1)
	{
		osd_printf_warning("unsupported format %u - only PCM is supported (%s)\n", temp16, file.filename());
		return false;
	}

	// number of channels -- only mono is supported
	offset += file.read(&temp16, 2);
	temp16 = little_endianize_int16(temp16);
	if (temp16 != 1)
	{
		osd_printf_warning("unsupported number of channels %u - only mono is supported (%s)\n", temp16, file.filename());
		return false;
	}

	// sample rate
	uint32_t rate;
	offset += file.read(&rate, 4);
	rate = little_endianize_int32(rate);

	// bytes/second and block alignment are ignored
	offset += file.read(buf, 6);

	// bits/sample
	uint16_t bits;
	offset += file.read(&bits, 2);
	bits = little_endianize_int16(bits);
	if (bits != 8 && bits != 16)
	{
		osd_printf_warning("unsupported bits/sample %u - only 8 and 16 are supported (%s)\n", bits, file.filename());
		return false;
	}

	// seek past any extra data
	file.seek(length - 16, SEEK_CUR);
	offset += length - 16;

	// seek until we find a data tag
	while (1)
	{
		offset += file.read(buf, 4);
		offset += file.read(&length, 4);
		length = little_endianize_int32(length);
		if (memcmp(&buf[0], "data", 4) == 0)
			break;

		// seek to the next block
		file.seek(length, SEEK_CUR);
		offset += length;
		if (offset >= filesize)
		{
			osd_printf_warning("Could not find data tag (%s)\n", file.filename());
			return false;
		}
	}

	// if there was a 0 length data block, we're done
	if (length == 0)
	{
		osd_printf_warning("empty data block (%s)\n", file.filename());
		return false;
	}

	// fill in the sample data
	sample.frequency = rate;

	// read the data in
	if (bits == 8)
	{
		sample.data.resize(length);
		file.read(&sample.data[0], length);

		// convert 8-bit data to signed samples
		uint8_t *tempptr = reinterpret_cast<uint8_t *>(&sample.data[0]);
		for (int32_t sindex = length - 1; sindex >= 0; sindex--)
			sample.data[sindex] = int8_t(tempptr[sindex] ^ 0x80) * 256;
	}
	else
	{
		// 16-bit data is fine as-is
		sample.data.resize(length / 2);
		file.read(&sample.data[0], length);

		// swap high/low on big-endian systems
		if (ENDIANNESS_NATIVE != ENDIANNESS_LITTLE)
			for (uint32_t sindex = 0; sindex < length / 2; sindex++)
				sample.data[sindex] = little_endianize_int16(sample.data[sindex]);
	}
	return true;
}


//-------------------------------------------------
//  read_flac_sample - read a FLAC file as a sample
//-------------------------------------------------

bool samples_device::read_flac_sample(emu_file &file, sample_t &sample)
{
	// seek back to the start of the file
	file.seek(0, SEEK_SET);

	// create the FLAC decoder and fill in the sample data
	flac_decoder decoder((util::core_file &)file);
	sample.frequency = decoder.sample_rate();

	// error if more than 1 channel or not 16bpp
	if (decoder.channels() != 1)
		return false;
	if (decoder.bits_per_sample() != 16)
		return false;

	// resize the array and read
	sample.data.resize(decoder.total_samples());
	if (!decoder.decode_interleaved(&sample.data[0], sample.data.size()))
		return false;

	// finish up and clean up
	decoder.finish();
	return true;
}


//-------------------------------------------------
//  load_samples - load all the samples in our
//  attached interface
//  Returns true when all samples were successfully read, else false
//-------------------------------------------------

bool samples_device::load_samples()
{
	bool ok = true;
	// if the user doesn't want to use samples, bail
	if (!machine().options().samples())
		return false;

	// iterate over ourself
	const std::string &basename = machine().basename();
	samples_iterator iter(*this);
	const char *altbasename = iter.altbasename();

	// pre-size the array
	m_sample.resize(iter.count());

	// load the samples
	int index = 0;
	for (const char *samplename = iter.first(); samplename; index++, samplename = iter.next())
	{
		// attempt to open as FLAC first
		emu_file file(machine().options().sample_path(), OPEN_FLAG_READ);
		std::error_condition filerr = file.open(util::string_format("%s" PATH_SEPARATOR "%s.flac", basename, samplename));
		if (filerr && altbasename)
			filerr = file.open(util::string_format("%s" PATH_SEPARATOR "%s.flac", altbasename, samplename));

		// if not, try as WAV
		if (filerr)
			filerr = file.open(util::string_format("%s" PATH_SEPARATOR "%s.wav", basename, samplename));
		if (filerr && altbasename)
			filerr = file.open(util::string_format("%s" PATH_SEPARATOR "%s.wav", altbasename, samplename));

		// if opened, read it
		if (!filerr)
		{
			read_sample(file, m_sample[index]);
		}
		else
		{
			logerror("Error opening sample '%s' (%s:%d %s)\n", samplename, filerr.category().name(), filerr.value(), filerr.message());
			ok = false;
		}
	}
	return ok;
}
