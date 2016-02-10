// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  sound.c - SDL implementation of MAME sound routines
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#include "sound_module.h"
#include "modules/osdmodule.h"

#if (defined(OSD_SDL) || defined(USE_SDL_SOUND))

// standard sdl header
#include "../../sdl/sdlinc.h"

// MAME headers
#include "emu.h"
#include "emuopts.h"

#include "../../sdl/osdsdl.h"

//============================================================
//  DEBUGGING
//============================================================

#define LOG_SOUND       0

//============================================================
//  PROTOTYPES
//============================================================

static void sdl_callback(void *userdata, Uint8 *stream, int len);

//============================================================
//  CLASS
//============================================================

class sound_sdl : public osd_module, public sound_module
{
public:

	friend void sdl_callback(void *userdata, Uint8 *stream, int len);

	// number of samples per SDL callback
	static const int SDL_XFER_SAMPLES = 512;

	sound_sdl()
	: osd_module(OSD_SOUND_PROVIDER, "sdl"), sound_module(),
		stream_in_initialized(0),
		stream_loop(0),
		attenuation(0)
	{
		sdl_xfer_samples = SDL_XFER_SAMPLES;
	}
	virtual ~sound_sdl() { }

	virtual int init(const osd_options &options) override;
	virtual void exit() override;

	// sound_module

	virtual void update_audio_stream(bool is_throttled, const INT16 *buffer, int samples_this_frame) override;
	virtual void set_mastervolume(int attenuation) override;

private:
	int lock_buffer(bool is_throttled, long offset, long size, void **buffer1, long *length1, void **buffer2, long *length2);
	void unlock_buffer(void);
	void att_memcpy(void *dest, const INT16 *data, int bytes_to_copy);
	void copy_sample_data(bool is_throttled, const INT16 *data, int bytes_to_copy);
	int sdl_create_buffers(void);
	void sdl_destroy_buffers(void);

	int sdl_xfer_samples;
	int stream_in_initialized;
	int stream_loop;
	int attenuation;

	int              buf_locked;

	INT8             *stream_buffer;
	volatile INT32   stream_playpos;

	UINT32           stream_buffer_size;
	UINT32           stream_buffer_in;

	// buffer over/underflow counts
	int              buffer_underflows;
	int              buffer_overflows;


};


//============================================================
//  PARAMETERS
//============================================================

// maximum audio latency
#define MAX_AUDIO_LATENCY       5

//============================================================
//  LOCAL VARIABLES
//============================================================

// debugging
static FILE *sound_log;

//============================================================
//  sound_sdl - destructor
//============================================================

//============================================================
//  lock_buffer
//============================================================
int sound_sdl::lock_buffer(bool is_throttled, long offset, long size, void **buffer1, long *length1, void **buffer2, long *length2)
{
	volatile long pstart, pend, lstart, lend;

	if (!buf_locked)
	{
		if (is_throttled)
		{
			pstart = stream_playpos;
			pend = (pstart + sdl_xfer_samples);
			lstart = offset;
			lend = lstart+size;
			while (((pstart >= lstart) && (pstart <= lend)) ||
					((pend >= lstart) && (pend <= lend)))
			{
				pstart = stream_playpos;
				pend = pstart + sdl_xfer_samples;
			}
		}

		SDL_LockAudio();
		buf_locked++;
	}

	// init lengths
	*length1 = *length2 = 0;

	if ((offset + size) > stream_buffer_size)
	{
		// 2-piece case
		*length1 = stream_buffer_size - offset;
		*buffer1 = &stream_buffer[offset];
		*length2 = size - *length1;
		*buffer2 = stream_buffer;
	}
	else
	{
		// normal 1-piece case
		*length1 = size;
		*buffer1 = &stream_buffer[offset];
	}

	if (LOG_SOUND)
		fprintf(sound_log, "locking %ld bytes at offset %ld (total %d, playpos %d): len1 %ld len2 %ld\n",
			size, offset, stream_buffer_size, stream_playpos, *length1, *length2);

	return 0;
}

//============================================================
//  unlock_buffer
//============================================================
void sound_sdl::unlock_buffer(void)
{
	buf_locked--;
	if (!buf_locked)
		SDL_UnlockAudio();

	if (LOG_SOUND)
		fprintf(sound_log, "unlocking\n");

}

//============================================================
//  Apply attenuation
//============================================================

void sound_sdl::att_memcpy(void *dest, const INT16 *data, int bytes_to_copy)
{
	int level= (int) (pow(10.0, (double) attenuation / 20.0) * 128.0);
	INT16 *d = (INT16 *) dest;
	int count = bytes_to_copy/2;
	while (count>0)
	{
		*d++ = (*data++ * level) >> 7; /* / 128 */
		count--;
	}
}

//============================================================
//  copy_sample_data
//============================================================

void sound_sdl::copy_sample_data(bool is_throttled, const INT16 *data, int bytes_to_copy)
{
	void *buffer1, *buffer2 = (void *)NULL;
	long length1, length2;
	int cur_bytes;

	// attempt to lock the stream buffer
	if (lock_buffer(is_throttled, stream_buffer_in, bytes_to_copy, &buffer1, &length1, &buffer2, &length2) < 0)
	{
		buffer_underflows++;
		return;
	}

	// adjust the input pointer
	stream_buffer_in += bytes_to_copy;
	if (stream_buffer_in >= stream_buffer_size)
	{
		stream_buffer_in -= stream_buffer_size;
		stream_loop = 1;

		if (LOG_SOUND)
			fprintf(sound_log, "stream_loop set to 1 (stream_buffer_in looped)\n");
	}

	// copy the first chunk
	cur_bytes = (bytes_to_copy > length1) ? length1 : bytes_to_copy;
	att_memcpy(buffer1, data, cur_bytes);

	// adjust for the number of bytes
	bytes_to_copy -= cur_bytes;
	data = (INT16 *)((UINT8 *)data + cur_bytes);

	// copy the second chunk
	if (bytes_to_copy != 0)
	{
		cur_bytes = (bytes_to_copy > length2) ? length2 : bytes_to_copy;
		att_memcpy(buffer2, data, cur_bytes);
	}

	// unlock
	unlock_buffer();
}


//============================================================
//  update_audio_stream
//============================================================

void sound_sdl::update_audio_stream(bool is_throttled, const INT16 *buffer, int samples_this_frame)
{
	// if nothing to do, don't do it
	if (sample_rate() != 0 && stream_buffer)
	{
		int bytes_this_frame = samples_this_frame * sizeof(INT16) * 2;
		int play_position, write_position, stream_in;
		int orig_write; // used in LOG

		play_position = stream_playpos;

		write_position = stream_playpos + ((sample_rate() / 50) * sizeof(INT16) * 2);
		orig_write = write_position;

		if (!stream_in_initialized)
		{
			stream_in = stream_buffer_in = (write_position + stream_buffer_size) / 2;

			if (LOG_SOUND)
			{
				fprintf(sound_log, "stream_in = %d\n", (int)stream_in);
				fprintf(sound_log, "stream_playpos = %d\n", (int)stream_playpos);
				fprintf(sound_log, "write_position = %d\n", (int)write_position);
			}
			// start playing
			SDL_PauseAudio(0);

			stream_in_initialized = 1;
		}
		else
		{
			// normalize the stream in position
			stream_in = stream_buffer_in;
			if (stream_in < write_position && stream_loop == 1)
				stream_in += stream_buffer_size;

			// now we should have, in order:
			//    <------pp---wp---si--------------->

			// if we're between play and write positions, then bump forward, but only in full chunks
			while (stream_in < write_position)
			{
				if (LOG_SOUND)
					fprintf(sound_log, "Underflow: PP=%d  WP=%d(%d)  SI=%d(%d)  BTF=%d\n", (int)play_position, (int)write_position, (int)orig_write, (int)stream_in, (int)stream_buffer_in, (int)bytes_this_frame);

				buffer_underflows++;
				stream_in += bytes_this_frame;
			}

			// if we're going to overlap the play position, just skip this chunk
			if (stream_in + bytes_this_frame > play_position + stream_buffer_size)
			{
				if (LOG_SOUND)
					fprintf(sound_log, "Overflow: PP=%d  WP=%d(%d)  SI=%d(%d)  BTF=%d\n", (int)play_position, (int)write_position, (int)orig_write, (int)stream_in, (int)stream_buffer_in, (int)bytes_this_frame);

				buffer_overflows++;
				return;
			}
		}

		if (stream_in >= stream_buffer_size)
		{
			stream_in -= stream_buffer_size;
			stream_loop = 1;

			if (LOG_SOUND)
				fprintf(sound_log, "stream_loop set to 1 (stream_in looped)\n");

		}

		// now we know where to copy; let's do it
		stream_buffer_in = stream_in;
		copy_sample_data(is_throttled, buffer, bytes_this_frame);
	}
}



//============================================================
//  set_mastervolume
//============================================================

void sound_sdl::set_mastervolume(int _attenuation)
{
	// clamp the attenuation to 0-32 range
	attenuation = MAX(MIN(_attenuation, 0), -32);

	if (stream_in_initialized)
	{
		if (attenuation == -32)
			SDL_PauseAudio(1);
		else
			SDL_PauseAudio(0);
	}
}

//============================================================
//  sdl_callback
//============================================================
static void sdl_callback(void *userdata, Uint8 *stream, int len)
{
	sound_sdl *thiz = (sound_sdl *) userdata;
	int len1, len2, sb_in;

	sb_in = thiz->stream_buffer_in;
	if (thiz->stream_loop)
		sb_in += thiz->stream_buffer_size;

	if (sb_in < (thiz->stream_playpos+len))
	{
		if (LOG_SOUND)
			fprintf(sound_log, "Underflow at sdl_callback: SPP=%d SBI=%d(%d) Len=%d\n", (int)thiz->stream_playpos, (int)sb_in, (int)thiz->stream_buffer_in, (int)len);

		return;
	}
	else if ((thiz->stream_playpos+len) > thiz->stream_buffer_size)
	{
		len1 = thiz->stream_buffer_size - thiz->stream_playpos;
		len2 = len - len1;
	}
	else
	{
		len1 = len;
		len2 = 0;
	}

	memcpy(stream, thiz->stream_buffer + thiz->stream_playpos, len1);
	memset(thiz->stream_buffer + thiz->stream_playpos, 0, len1); // no longer needed
	if (len2)
	{
		memcpy(stream+len1, thiz->stream_buffer, len2);
		memset(thiz->stream_buffer, 0, len2); // no longer needed
	}


	// move the play cursor
	thiz->stream_playpos += len1 + len2;
	if (thiz->stream_playpos >= thiz->stream_buffer_size)
	{
		thiz->stream_playpos -= thiz->stream_buffer_size;
		thiz->stream_loop = 0;

		if (LOG_SOUND)
			fprintf(sound_log, "stream_loop set to 0 (stream_playpos looped)\n");
	}

	if (LOG_SOUND)
		fprintf(sound_log, "callback: xfer len1 %d len2 %d, playpos %d\n",
				len1, len2, thiz->stream_playpos);
}


//============================================================
//  sound_sdl::init
//============================================================

int sound_sdl::init(const osd_options &options)
{
	int         n_channels = 2;
	int         audio_latency;
	SDL_AudioSpec   aspec, obtained;
	char audio_driver[16] = "";

	if (LOG_SOUND)
		sound_log = fopen(SDLMAME_SOUND_LOG, "w");

	// skip if sound disabled
	if (sample_rate() != 0)
	{
		if (SDL_InitSubSystem(SDL_INIT_AUDIO)) {
			osd_printf_error("Could not initialize SDL %s\n", SDL_GetError());
			return -1;
		}

		osd_printf_verbose("Audio: Start initialization\n");
	#if (SDLMAME_SDL2)
		strncpy(audio_driver, SDL_GetCurrentAudioDriver(), sizeof(audio_driver));
	#else
		SDL_AudioDriverName(audio_driver, sizeof(audio_driver));
	#endif
		osd_printf_verbose("Audio: Driver is %s\n", audio_driver);

		sdl_xfer_samples = SDL_XFER_SAMPLES;
		stream_in_initialized = 0;
		stream_loop = 0;

		// set up the audio specs
		aspec.freq = sample_rate();
		aspec.format = AUDIO_S16SYS;    // keep endian independent
		aspec.channels = n_channels;
		aspec.samples = sdl_xfer_samples;
		aspec.callback = sdl_callback;
		aspec.userdata = this;

		if (SDL_OpenAudio(&aspec, &obtained) < 0)
			goto cant_start_audio;

		osd_printf_verbose("Audio: frequency: %d, channels: %d, samples: %d\n",
							obtained.freq, obtained.channels, obtained.samples);

		sdl_xfer_samples = obtained.samples;

		// pin audio latency
		audio_latency = MAX(MIN(m_audio_latency, MAX_AUDIO_LATENCY), 1);

		// compute the buffer sizes
		stream_buffer_size = (sample_rate() * 2 * sizeof(INT16) * (2 + audio_latency)) / 30;
		stream_buffer_size = (stream_buffer_size / 1024) * 1024;
		if (stream_buffer_size < 1024)
			stream_buffer_size = 1024;

		// create the buffers
		if (sdl_create_buffers())
			goto cant_create_buffers;

		// set the startup volume
		set_mastervolume(attenuation);
		osd_printf_verbose("Audio: End initialization\n");
		return 0;

		// error handling
	cant_create_buffers:
	cant_start_audio:
		osd_printf_verbose("Audio: Initialization failed. SDL error: %s\n", SDL_GetError());

		return -1;
	}

	return 0;
}



//============================================================
//  sdl_kill
//============================================================

void sound_sdl::exit()
{
	// if nothing to do, don't do it
	if (sample_rate() == 0)
		return;

	osd_printf_verbose("sdl_kill: closing audio\n");
	SDL_CloseAudio();

	SDL_QuitSubSystem(SDL_INIT_AUDIO);

	// kill the buffers
	sdl_destroy_buffers();

	// print out over/underflow stats
	if (buffer_overflows || buffer_underflows)
		osd_printf_verbose("Sound buffer: overflows=%d underflows=%d\n", buffer_overflows, buffer_underflows);

	if (LOG_SOUND)
	{
		fprintf(sound_log, "Sound buffer: overflows=%d underflows=%d\n", buffer_overflows, buffer_underflows);
		fclose(sound_log);
	}
}



//============================================================
//  dsound_create_buffers
//============================================================

int sound_sdl::sdl_create_buffers(void)
{
	osd_printf_verbose("sdl_create_buffers: creating stream buffer of %u bytes\n", stream_buffer_size);

	stream_buffer = global_alloc_array_clear<INT8>(stream_buffer_size);
	stream_playpos = 0;
	buf_locked = 0;
	return 0;
}

//============================================================
//  sdl_destroy_buffers
//============================================================

void sound_sdl::sdl_destroy_buffers(void)
{
	// release the buffer
	if (stream_buffer)
		global_free_array(stream_buffer);
	stream_buffer = NULL;
}



#else /* SDLMAME_UNIX */
	MODULE_NOT_SUPPORTED(sound_sdl, OSD_SOUND_PROVIDER, "sdl")
#endif

MODULE_DEFINITION(SOUND_SDL, sound_sdl)
