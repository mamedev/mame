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

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

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
	: device_t(mconfig, SAMPLES, "Samples", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
}

samples_device::samples_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, type, name, tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  static_set_interface - configuration helper
//  to set the interface
//-------------------------------------------------

void samples_device::static_set_interface(device_t &device, const samples_interface &interface)
{
	samples_device &samples = downcast<samples_device &>(device);
	static_cast<samples_interface &>(samples) = interface;
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
	if (m_sample.count() == 0)
		return;

    assert(samplenum < m_sample.count());
    assert(channel < m_channels);

	// force an update before we start
    channel_t &chan = m_channel[channel];
	chan.stream->update();

	// update the parameters
	loaded_sample &sample = m_sample[samplenum];
	chan.source = sample.data;
	chan.source_length = sample.length;
	chan.source_num = (sample.data.count() > 0) ? samplenum : -1;
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
	if (m_start != NULL)
		(*m_start)(*this);
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
		if (chan.source_num >= 0 && chan.source_num < m_sample.count())
		{
			loaded_sample &sample = m_sample[chan.source_num];
			chan.source = sample.data;
			chan.source_length = sample.length;
			if (sample.data == NULL)
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
							samples = 0;
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

bool samples_device::read_sample(emu_file &file, loaded_sample &sample)
{
	// read the core header and make sure it's a proper file
	UINT8 buf[4];
	UINT32 offset = file.read(buf, 4);
	if (offset < 4)
	{
		mame_printf_warning("Unable to read %s, 0-byte file?\n", file.filename());
		return false;
	}

	// look for the appropriate RIFF tag
	if (memcmp(&buf[0], "RIFF", 4) == 0)
		return read_wav_sample(file, sample);
	else if (memcmp(&buf[0], "fLaC", 4) == 0)
		return read_flac_sample(file, sample);

	// if nothing appropriate, emit a warning
	mame_printf_warning("Unable to read %s, corrupt file?\n", file.filename());
	return false;
}


//-------------------------------------------------
//  read_wav_sample - read a WAV file as a sample
//-------------------------------------------------

bool samples_device::read_wav_sample(emu_file &file, loaded_sample &sample)
{
printf("Reading %s as WAV\n", file.filename());
	// we already read the opening 'WAVE' header
	UINT32 offset = 4;

	// get the total size
	UINT32 filesize;
	offset += file.read(&filesize, 4);
	if (offset < 8)
		return false;
	filesize = LITTLE_ENDIANIZE_INT32(filesize);

	// read the RIFF file type and make sure it's a WAVE file
	char buf[32];
	offset += file.read(buf, 4);
	if (offset < 12)
		return false;
	if (memcmp(&buf[0], "WAVE", 4) != 0)
		return false;

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
			return false;
	}

	// read the format -- make sure it is PCM
	UINT16 temp16;
	offset += file.read(&temp16, 2);
	temp16 = LITTLE_ENDIANIZE_INT16(temp16);
	if (temp16 != 1)
		return false;

	// number of channels -- only mono is supported
	offset += file.read(&temp16, 2);
	temp16 = LITTLE_ENDIANIZE_INT16(temp16);
	if (temp16 != 1)
		return false;

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
		return false;

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
			return false;
	}

	// if there was a 0 length data block, we're done
	if (length == 0)
		return false;

	// fill in the sample data
	sample.length = length / 2;
	sample.frequency = rate;

	// read the data in
	if (bits == 8)
	{
		sample.data.resize(length);
		file.read(sample.data, length);

		// convert 8-bit data to signed samples
		UINT8 *tempptr = reinterpret_cast<UINT8 *>(&sample.data[0]);
		for (UINT32 sindex = length - 1; sindex >= 0; sindex--)
			sample.data[sindex] = INT8(tempptr[sindex] ^ 0x80) * 256;
	}
	else
	{
		// 16-bit data is fine as-is
		sample.data.resize(length / 2);
		file.read(sample.data, length);

		// swap high/low on big-endian systems
		if (ENDIANNESS_NATIVE != ENDIANNESS_LITTLE)
			for (UINT32 sindex = 0; sindex < sample.length; sindex++)
				sample.data[sindex] = LITTLE_ENDIANIZE_INT16(sample.data[sindex]);
	}
	return true;
}


//-------------------------------------------------
//  read_flac_sample - read a FLAC file as a sample
//-------------------------------------------------

bool samples_device::read_flac_sample(emu_file &file, loaded_sample &sample)
{
printf("Reading %s as FLAC\n", file.filename());
	// seek back to the start of the file
	file.seek(0, SEEK_SET);

	// create the FLAC decoder and fill in the sample data
	flac_decoder decoder(file);
	sample.length = decoder.total_samples();
	sample.frequency = decoder.sample_rate();

	// error if more than 1 channel or not 16bpp
	if (decoder.channels() != 1)
		return false;
	if (decoder.bits_per_sample() != 16)
		return false;

	// resize the array and read
	sample.data.resize(sample.length);
	if (!decoder.decode_interleaved(sample.data, sample.length))
		return false;

	// finish up and clean up
	decoder.finish();
	return true;
}


//-------------------------------------------------
//  load_samples - load all the samples in our
//  attached interface
//-------------------------------------------------

void samples_device::load_samples()
{
	// if the user doesn't want to use samples, bail
	if (!machine().options().samples())
		return;

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
printf("Sample %d = %s\n", index, samplename);
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
			mame_printf_warning("Sample '%s' NOT FOUND\n", samplename);
	}
}
