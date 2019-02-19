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
#include <SDL2/SDL.h>

// MAME headers
#include "emu.h"
#include "emuopts.h"

#include "../../sdl/osdsdl.h"

#include <algorithm>
#include <fstream>
#include <memory>

//============================================================
//  DEBUGGING
//============================================================

#define LOG_SOUND       0

//============================================================
//  CLASS
//============================================================

class sound_sdl : public osd_module, public sound_module
{
public:

	// number of samples per SDL callback
	static const int SDL_XFER_SAMPLES = 512;

	sound_sdl() :
		osd_module(OSD_SOUND_PROVIDER, "sdl"), sound_module(),
		stream_in_initialized(0),
		attenuation(0), buf_locked(0), stream_buffer(nullptr), stream_buffer_size(0), buffer_underflows(0), buffer_overflows(0)
{
		sdl_xfer_samples = SDL_XFER_SAMPLES;
	}
	virtual ~sound_sdl() { }

	virtual int init(const osd_options &options) override;
	virtual void exit() override;

	// sound_module

	virtual void update_audio_stream(bool is_throttled, const int16_t *buffer, int samples_this_frame) override;
	virtual void set_mastervolume(int attenuation) override;

private:
	class ring_buffer
	{
	public:
		ring_buffer(size_t size);

		size_t data_size() const { return (tail - head + buffer_size) % buffer_size; }
		size_t free_size() const { return (head - tail - 1 + buffer_size) % buffer_size; }
		int append(const void *data, size_t size);
		int pop(void *data, size_t size);

	private:
		std::unique_ptr<int8_t []> const buffer;
		size_t const buffer_size;
		int head = 0, tail = 0;
	};

	static void sdl_callback(void *userdata, Uint8 *stream, int len);

	void lock_buffer();
	void unlock_buffer();
	void attenuate(int16_t *data, int bytes);
	void copy_sample_data(bool is_throttled, const int16_t *data, int bytes_to_copy);
	int sdl_create_buffers();
	void sdl_destroy_buffers();

	int sdl_xfer_samples;
	int stream_in_initialized;
	int attenuation;

	int              buf_locked;
	std::unique_ptr<ring_buffer> stream_buffer;
	uint32_t         stream_buffer_size;


	// diagnostics
	int              buffer_underflows;
	int              buffer_overflows;
	std::unique_ptr<std::ofstream> sound_log;
};


//============================================================
//  PARAMETERS
//============================================================

// maximum audio latency
#define MAX_AUDIO_LATENCY       5

//============================================================
//  ring_buffer - constructor
//============================================================

sound_sdl::ring_buffer::ring_buffer(size_t size)
	: buffer(std::make_unique<int8_t []>(size + 1)), buffer_size(size + 1)
{
	// A size+1 bytes buffer is allocated because it can never be full.
	// Otherwise the case head == tail couldn't be distinguished between a
	// full buffer and an empty buffer.
	std::fill_n(buffer.get(), size + 1, 0);
}

//============================================================
//  ring_buffer::append
//============================================================

int sound_sdl::ring_buffer::append(const void *data, size_t size)
{
	if (free_size() < size)
		return -1;

	int8_t const *const data8 = reinterpret_cast<int8_t const *>(data);
	size_t sz = buffer_size - tail;
	if (size <= sz)
		sz = size;
	else
		std::copy_n(&data8[sz], size - sz, &buffer[0]);

	std::copy_n(data8, sz, &buffer[tail]);
	tail = (tail + size) % buffer_size;

	return 0;
}

//============================================================
//  ring_buffer::pop
//============================================================

int sound_sdl::ring_buffer::pop(void *data, size_t size)
{
	if (data_size() < size)
		return -1;

	int8_t *const data8 = reinterpret_cast<int8_t *>(data);
	size_t sz = buffer_size - head;
	if (size <= sz)
		sz = size;
	else
	{
		std::copy_n(&buffer[0], size - sz, &data8[sz]);
		std::fill_n(&buffer[0], size - sz, 0);
	}

	std::copy_n(&buffer[head], sz, data8);
	std::fill_n(&buffer[head], sz, 0);
	head = (head + size) % buffer_size;

	return 0;
}

//============================================================
//  sound_sdl - destructor
//============================================================

//============================================================
//  lock_buffer
//============================================================
void sound_sdl::lock_buffer()
{
	if (!buf_locked)
		SDL_LockAudio();
	buf_locked++;

	if (LOG_SOUND)
		*sound_log << "locking\n";
}

//============================================================
//  unlock_buffer
//============================================================
void sound_sdl::unlock_buffer()
{
	buf_locked--;
	if (!buf_locked)
		SDL_UnlockAudio();

	if (LOG_SOUND)
		*sound_log << "unlocking\n";

}

//============================================================
//  Apply attenuation
//============================================================

void sound_sdl::attenuate(int16_t *data, int bytes_to_copy)
{
	int level = (int) (pow(10.0, (double) attenuation / 20.0) * 128.0);
	int count = bytes_to_copy / sizeof(*data);
	while (count > 0)
	{
		*data = (*data * level) >> 7; /* / 128 */
		data++;
		count--;
	}
}

//============================================================
//  copy_sample_data
//============================================================

void sound_sdl::copy_sample_data(bool is_throttled, const int16_t *data, int bytes_to_copy)
{
	lock_buffer();
	int const err = stream_buffer->append(data, bytes_to_copy);
	unlock_buffer();

	if (LOG_SOUND && err)
		*sound_log << "Late detection of overflow. This shouldn't happen.\n";
}


//============================================================
//  update_audio_stream
//============================================================

void sound_sdl::update_audio_stream(bool is_throttled, const int16_t *buffer, int samples_this_frame)
{
	// if nothing to do, don't do it
	if (sample_rate() == 0 || !stream_buffer)
		return;


	if (!stream_in_initialized)
	{
		// Fill in some zeros to prevent an initial buffer underflow
		int8_t zero = 0;
		size_t zsize = stream_buffer->free_size() / 2;
		while (zsize--)
			stream_buffer->append(&zero, 1);

		// start playing
		SDL_PauseAudio(0);
		stream_in_initialized = 1;
	}

	size_t bytes_this_frame = samples_this_frame * sizeof(*buffer) * 2;
	size_t free_size = stream_buffer->free_size();
	size_t data_size = stream_buffer->data_size();

	if (stream_buffer->free_size() < bytes_this_frame) {
		if (LOG_SOUND)
			util::stream_format(*sound_log, "Overflow: DS=%u FS=%u BTF=%u\n", data_size, free_size, bytes_this_frame);
		buffer_overflows++;
		return;
	}

	copy_sample_data(is_throttled, buffer, bytes_this_frame);

	size_t nfree_size = stream_buffer->free_size();
	size_t ndata_size = stream_buffer->data_size();
	if (LOG_SOUND)
		util::stream_format(*sound_log, "Appended data: DS=%u(%u) FS=%u(%u) BTF=%u\n", data_size, ndata_size, free_size, nfree_size, bytes_this_frame);
}



//============================================================
//  set_mastervolume
//============================================================

void sound_sdl::set_mastervolume(int _attenuation)
{
	// clamp the attenuation to 0-32 range
	attenuation = std::max(std::min(_attenuation, 0), -32);

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
void sound_sdl::sdl_callback(void *userdata, Uint8 *stream, int len)
{
	sound_sdl *thiz = reinterpret_cast<sound_sdl *>(userdata);
	size_t const free_size = thiz->stream_buffer->free_size();
	size_t const data_size = thiz->stream_buffer->data_size();

	if (data_size < len)
	{
		thiz->buffer_underflows++;
		if (LOG_SOUND)
			util::stream_format(*thiz->sound_log, "Underflow at sdl_callback: DS=%u FS=%u Len=%d\n", data_size, free_size, len);

		// Maybe read whatever is left in the stream_buffer anyway?
		memset(stream, 0, len);
		return;
	}

	int err = thiz->stream_buffer->pop((void *)stream, len);
	if (LOG_SOUND && err)
		*thiz->sound_log << "Late detection of underflow. This shouldn't happen.\n";

	thiz->attenuate((int16_t *)stream, len);

	if (LOG_SOUND)
		util::stream_format(*thiz->sound_log, "callback: xfer DS=%u FS=%u Len=%d\n", data_size, free_size, len);
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
		sound_log = std::make_unique<std::ofstream>(SDLMAME_SOUND_LOG);

	// skip if sound disabled
	if (sample_rate() != 0)
	{
		if (SDL_InitSubSystem(SDL_INIT_AUDIO))
		{
			osd_printf_error("Could not initialize SDL %s\n", SDL_GetError());
			return -1;
		}

		osd_printf_verbose("Audio: Start initialization\n");
		strncpy(audio_driver, SDL_GetCurrentAudioDriver(), sizeof(audio_driver));
		osd_printf_verbose("Audio: Driver is %s\n", audio_driver);

		sdl_xfer_samples = SDL_XFER_SAMPLES;
		stream_in_initialized = 0;

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
		audio_latency = std::max(std::min(m_audio_latency, MAX_AUDIO_LATENCY), 1);

		// compute the buffer sizes
		stream_buffer_size = (sample_rate() * 2 * sizeof(int16_t) * (2 + audio_latency)) / 30;
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
		util::stream_format(*sound_log, "Sound buffer: overflows=%d underflows=%d\n", buffer_overflows, buffer_underflows);
		sound_log.reset();
	}
}



//============================================================
//  dsound_create_buffers
//============================================================

int sound_sdl::sdl_create_buffers()
{
	osd_printf_verbose("sdl_create_buffers: creating stream buffer of %u bytes\n", stream_buffer_size);

	stream_buffer = std::make_unique<ring_buffer>(stream_buffer_size);
	buf_locked = 0;
	return 0;
}

//============================================================
//  sdl_destroy_buffers
//============================================================

void sound_sdl::sdl_destroy_buffers()
{
	// release the buffer
	stream_buffer.reset();
}



#else /* SDLMAME_UNIX */
	MODULE_NOT_SUPPORTED(sound_sdl, OSD_SOUND_PROVIDER, "sdl")
#endif

MODULE_DEFINITION(SOUND_SDL, sound_sdl)
