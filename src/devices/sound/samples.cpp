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
#include "emuopts.h"
#include "samples.h"
#include "flac.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type SAMPLES = &device_creator<samples_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  samples_device - constructors
//-------------------------------------------------

samples_device::samples_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SAMPLES, "Samples", tag, owner, clock, "samples", __FILE__),
		device_sound_interface(mconfig, *this),
		m_channels(0),
		m_names(NULL)
{
}

samples_device::samples_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_sound_interface(mconfig, *this),
		m_channels(0),
		m_names(NULL)
{
}


//**************************************************************************
//  PUBLIC INTERFACE
//**************************************************************************

//-------------------------------------------------
//  start - start playing a loaded sample
//-------------------------------------------------

void samples_device::start(UINT8 channel, UINT32 samplenum, bool loop)
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
	chan.source = (sample.data.size() > 0) ? &sample.data[0] : NULL;
	chan.source_length = sample.data.size();
	chan.source_num = (chan.source_length > 0) ? samplenum : -1;
	chan.pos = 0;
	chan.frac = 0;
	chan.basefreq = sample.frequency;
	chan.step = (INT64(chan.basefreq) << FRAC_BITS) / machine().sample_rate();
	chan.loop = loop;
}


//-------------------------------------------------
//  start_raw - start playing an externally
//  provided sample
//-------------------------------------------------

void samples_device::start_raw(UINT8 channel, const INT16 *sampledata, UINT32 samples, UINT32 frequency, bool loop)
{
	assert(channel < m_channels);

	// force an update before we start
	channel_t &chan = m_channel[channel];
	chan.stream->update();

	// update the parameters
	chan.source = sampledata;
	chan.source_length = samples;
	chan.source_num = -1;
	chan.pos = 0;
	chan.frac = 0;
	chan.basefreq = frequency;
	chan.step = (INT64(chan.basefreq) << FRAC_BITS) / machine().sample_rate();
	chan.loop = loop;
}


//-------------------------------------------------
//  set_frequency - set the playback frequency of
//  a sample
//-------------------------------------------------

void samples_device::set_frequency(UINT8 channel, UINT32 freq)
{
	assert(channel < m_channels);

	// force an update before we start
	channel_t &chan = m_channel[channel];
	chan.stream->update();
	chan.step = (INT64(freq) << FRAC_BITS) / machine().sample_rate();
}


//-------------------------------------------------
//  set_volume - set the playback volume of a
//  sample
//-------------------------------------------------

void samples_device::set_volume(UINT8 channel, float volume)
{
	assert(channel < m_channels);

	// force an update before we start
	channel_t &chan = m_channel[channel];
	chan.stream->set_output_gain(0, volume);
}


//-------------------------------------------------
//  pause - pause playback on a channel
//-------------------------------------------------

void samples_device::pause(UINT8 channel, bool pause)
{
	assert(channel < m_channels);

	// force an update before we start
	channel_t &chan = m_channel[channel];
	chan.paused = pause;
}


//-------------------------------------------------
//  stop - stop playback on a channel
//-------------------------------------------------

void samples_device::stop(UINT8 channel)
{
	assert(channel < m_channels);

	// force an update before we start
	channel_t &chan = m_channel[channel];
	chan.source = NULL;
	chan.source_num = -1;
}


//-------------------------------------------------
//  stop_all - stop playback on all channels
//-------------------------------------------------

void samples_device::stop_all()
{
	// just iterate over channels and stop them
	for (UINT8 channel = 0; channel < m_channels; channel++)
		stop(channel);
}


//-------------------------------------------------
//  base_frequency - return the base frequency of
//  a channel being played
//-------------------------------------------------

UINT32 samples_device::base_frequency(UINT8 channel) const
{
	assert(channel < m_channels);

	// force an update before we start
	const channel_t &chan = m_channel[channel];
	chan.stream->update();
	return chan.basefreq;
}


//-------------------------------------------------
//  playing - return true if a sample is still
//  playing on the given channel
//-------------------------------------------------

bool samples_device::playing(UINT8 channel) const
{
	assert(channel < m_channels);

	// force an update before we start
	const channel_t &chan = m_channel[channel];
	chan.stream->update();
	return (chan.source != NULL);
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
		chan.stream = stream_alloc(0, 1, machine().sample_rate());
		chan.source = NULL;
		chan.source_num = -1;
		chan.step = 0;
		chan.loop = 0;
		chan.paused = 0;

		// register with the save state system
		save_item(NAME(chan.source_length), channel);
		save_item(NAME(chan.source_num), channel);
		save_item(NAME(chan.pos), channel);
		save_item(NAME(chan.frac), channel);
		save_item(NAME(chan.step), channel);
		save_item(NAME(chan.loop), channel);
		save_item(NAME(chan.paused), channel);
	}

	// initialize any custom handlers
	m_samples_start_cb.bind_relative_to(*owner());

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
			chan.source_length = sample.data.size();
			if (sample.data.empty())
				chan.source_num = -1;
		}

		// validate the position against the length in case the sample is smaller
		if (chan.source != NULL && chan.pos >= chan.source_length)
		{
			if (chan.loop)
				chan.pos %= chan.source_length;
			else
			{
				chan.source = NULL;
				chan.source_num = -1;
			}
		}
	}
}


//-------------------------------------------------
//  sound_stream_update - update a sound stream
//-------------------------------------------------

void samples_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// find the channel with this stream
	for (int channel = 0; channel < m_channels; channel++)
		if (&stream == m_channel[channel].stream)
		{
			channel_t &chan = m_channel[channel];
			stream_sample_t *buffer = outputs[0];

			// process if we still have a source and we're not paused
			if (chan.source != NULL && !chan.paused)
			{
				// load some info locally
				UINT32 pos = chan.pos;
				UINT32 frac = chan.frac;
				UINT32 step = chan.step;
				const INT16 *sample = chan.source;
				UINT32 sample_length = chan.source_length;

				while (samples--)
				{
					// do a linear interp on the sample
					INT32 sample1 = sample[pos];
					INT32 sample2 = sample[(pos + 1) % sample_length];
					INT32 fracmult = frac >> (FRAC_BITS - 14);
					*buffer++ = ((0x4000 - fracmult) * sample1 + fracmult * sample2) >> 14;

					// advance
					frac += step;
					pos += frac >> FRAC_BITS;
					frac = frac & ((1 << FRAC_BITS) - 1);

					// handle looping/ending
					if (pos >= sample_length)
					{
						if (chan.loop)
							pos %= sample_length;
						else
						{
							chan.source = NULL;
							chan.source_num = -1;
							if (samples > 0)
								memset(buffer, 0, samples * sizeof(*buffer));
							break;
						}
					}
				}

				// push position back out
				chan.pos = pos;
				chan.frac = frac;
			}
			else
				memset(buffer, 0, samples * sizeof(*buffer));
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
	UINT8 buf[4];
	UINT32 offset = file.read(buf, 4);
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
	UINT32 offset = 4;

	// get the total size
	UINT32 filesize;
	offset += file.read(&filesize, 4);
	if (offset < 8)
	{
		osd_printf_warning("Unexpected size offset %u (%s)\n", offset, file.filename());
		return false;
	}
	filesize = LITTLE_ENDIANIZE_INT32(filesize);

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
	UINT32 length;
	while (1)
	{
		offset += file.read(buf, 4);
		offset += file.read(&length, 4);
		length = LITTLE_ENDIANIZE_INT32(length);
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
	UINT16 temp16;
	offset += file.read(&temp16, 2);
	temp16 = LITTLE_ENDIANIZE_INT16(temp16);
	if (temp16 != 1)
	{
		osd_printf_warning("unsupported format %u - only PCM is supported (%s)\n", temp16, file.filename());
		return false;
	}

	// number of channels -- only mono is supported
	offset += file.read(&temp16, 2);
	temp16 = LITTLE_ENDIANIZE_INT16(temp16);
	if (temp16 != 1)
	{
		osd_printf_warning("unsupported number of channels %u - only mono is supported (%s)\n", temp16, file.filename());
		return false;
	}

	// sample rate
	UINT32 rate;
	offset += file.read(&rate, 4);
	rate = LITTLE_ENDIANIZE_INT32(rate);

	// bytes/second and block alignment are ignored
	offset += file.read(buf, 6);

	// bits/sample
	UINT16 bits;
	offset += file.read(&bits, 2);
	bits = LITTLE_ENDIANIZE_INT16(bits);
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
		length = LITTLE_ENDIANIZE_INT32(length);
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
		UINT8 *tempptr = reinterpret_cast<UINT8 *>(&sample.data[0]);
		for (INT32 sindex = length - 1; sindex >= 0; sindex--)
			sample.data[sindex] = INT8(tempptr[sindex] ^ 0x80) * 256;
	}
	else
	{
		// 16-bit data is fine as-is
		sample.data.resize(length / 2);
		file.read(&sample.data[0], length);

		// swap high/low on big-endian systems
		if (ENDIANNESS_NATIVE != ENDIANNESS_LITTLE)
			for (UINT32 sindex = 0; sindex < length / 2; sindex++)
				sample.data[sindex] = LITTLE_ENDIANIZE_INT16(sample.data[sindex]);
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
	flac_decoder decoder((core_file&) file);
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
	const char *basename = machine().basename();
	samples_iterator iter(*this);
	const char *altbasename = iter.altbasename();

	// pre-size the array
	m_sample.resize(iter.count());

	// load the samples
	int index = 0;
	for (const char *samplename = iter.first(); samplename != NULL; index++, samplename = iter.next())
	{
		// attempt to open as FLAC first
		emu_file file(machine().options().sample_path(), OPEN_FLAG_READ);
		file_error filerr = file.open(basename, PATH_SEPARATOR, samplename, ".flac");
		if (filerr != FILERR_NONE && altbasename != NULL)
			filerr = file.open(altbasename, PATH_SEPARATOR, samplename, ".flac");

		// if not, try as WAV
		if (filerr != FILERR_NONE)
			filerr = file.open(basename, PATH_SEPARATOR, samplename, ".wav");
		if (filerr != FILERR_NONE && altbasename != NULL)
			filerr = file.open(altbasename, PATH_SEPARATOR, samplename, ".wav");

		// if opened, read it
		if (filerr == FILERR_NONE)
			read_sample(file, m_sample[index]);
		else if (filerr == FILERR_NOT_FOUND)
		{
			logerror("%s: Sample '%s' NOT FOUND\n", tag(), samplename);
			ok = false;
		}
	}
	return ok;
}
