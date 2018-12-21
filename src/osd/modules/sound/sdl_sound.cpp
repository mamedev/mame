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

class ring_buffer
{
public:
	ring_buffer(size_t size);
	virtual ~ring_buffer();

	size_t data_size();
	size_t free_size();
	int append(const void *data, size_t size);
	int pop(void *data, size_t size);

private:
	int8_t *buffer;
	size_t buffer_size;
	int head, tail;
};

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
	int lock_buffer(void);
	void unlock_buffer(void);
	void attenuate(int16_t *data, int bytes);
	void copy_sample_data(bool is_throttled, const int16_t *data, int bytes_to_copy);
	int sdl_create_buffers(void);
	void sdl_destroy_buffers(void);

	int sdl_xfer_samples;
	int stream_in_initialized;
	int stream_loop;
	int attenuation;

	int              buf_locked;
	ring_buffer      *stream_buffer;
	uint32_t         stream_buffer_size;


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
//  ring_buffer - constructor
//============================================================

ring_buffer::ring_buffer(size_t size)
: buffer(global_alloc_array_clear<int8_t>(size + 1)), buffer_size(size + 1), head(0), tail(0)
{
	// A size+1 bytes buffer is allocated because it can never be full.
	// Otherwise the case head == tail couldn't be distinguished between a
	// full buffer and an empty buffer.
}

//============================================================
//  ring_buffer - destructor
//============================================================

ring_buffer::~ring_buffer()
{
	global_free_array(buffer);
}

//============================================================
//  ring_buffer::data_size
//============================================================

size_t ring_buffer::data_size()
{
	return (tail - head + buffer_size) % buffer_size;
}

//============================================================
//  ring_buffer::free_size
//============================================================

size_t ring_buffer::free_size() {
	return (head - tail - 1 + buffer_size) % buffer_size;
}

//============================================================
//  ring_buffer::append
//============================================================

int ring_buffer::append(const void *data, size_t size)
{
	int8_t *data8 = (int8_t *)data;
	size_t sz;

	if (free_size() < size)
		return -1;

	sz = buffer_size - tail;
	if (size <= sz)
		sz = size;
	else
		memcpy(buffer, &data8[sz], size - sz);

	memcpy(&buffer[tail], data8, sz);
	tail = (tail + size) % buffer_size;
	return 0;
}

//============================================================
//  ring_buffer::pop
//============================================================

int ring_buffer::pop(void *data, size_t size)
{
	int8_t *data8 = (int8_t *)data;
	size_t sz;

	if (data_size() < size)
		return -1;

	sz = buffer_size - head;
	if (size <= sz)
		sz = size;
	else {
		memcpy(&data8[sz], buffer, size - sz);
		memset(buffer, 0, size - sz);
	}

	memcpy(data8, &buffer[head], sz);
	memset(&buffer[head], 0, sz);
	head = (head + size) % buffer_size;
	return 0;
}

//============================================================
//  sound_sdl - destructor
//============================================================

//============================================================
//  lock_buffer
//============================================================
int sound_sdl::lock_buffer()
{
	if (!buf_locked)
		SDL_LockAudio();
	buf_locked++;

	if (LOG_SOUND)
		fprintf(sound_log, "locking\n");

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
	int err = stream_buffer->append((void *)data, bytes_to_copy);
	unlock_buffer();

	if (LOG_SOUND && err)
		fprintf(sound_log, "Late detection of overflow. This shouldn't happen.\n");
}


//============================================================
//  update_audio_stream
//============================================================

void sound_sdl::update_audio_stream(bool is_throttled, const int16_t *buffer, int samples_this_frame)
{
	// if nothing to do, don't do it
	if (sample_rate() == 0 || stream_buffer == nullptr)
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
			fprintf(sound_log, "Overflow: DS=%zu FS=%zu BTF=%zu\n", data_size, free_size, bytes_this_frame);
		buffer_overflows++;
		return;
	}

	copy_sample_data(is_throttled, buffer, bytes_this_frame);

	size_t nfree_size = stream_buffer->free_size();
	size_t ndata_size = stream_buffer->data_size();
	if (LOG_SOUND)
		fprintf(sound_log, "Appended data: DS=%zu(%zu) FS=%zu(%zu) BTF=%zu\n", data_size, ndata_size, free_size, nfree_size, bytes_this_frame);
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
static void sdl_callback(void *userdata, Uint8 *stream, int len)
{
	sound_sdl *thiz = (sound_sdl *) userdata;
	size_t free_size = thiz->stream_buffer->free_size();
	size_t data_size = thiz->stream_buffer->data_size();

	if (data_size < len)
	{
		thiz->buffer_underflows++;
		if (LOG_SOUND)
			fprintf(sound_log, "Underflow at sdl_callback: DS=%zu FS=%zu Len=%d\n", data_size, free_size, len);

		// Maybe read whatever is left in the stream_buffer anyway?
		memset(stream, 0, len);
		return;
	}

	int err = thiz->stream_buffer->pop((void *)stream, len);
	if (LOG_SOUND && err)
		fprintf(sound_log, "Late detection of underflow. This shouldn't happen.\n");

	thiz->attenuate((int16_t *)stream, len);

	if (LOG_SOUND)
		fprintf(sound_log, "callback: xfer DS=%zu FS=%zu Len=%d\n", data_size, free_size, len);
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
		strncpy(audio_driver, SDL_GetCurrentAudioDriver(), sizeof(audio_driver));
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

	stream_buffer = new ring_buffer(stream_buffer_size);
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
		delete stream_buffer;
	stream_buffer = nullptr;
}



#else /* SDLMAME_UNIX */
	MODULE_NOT_SUPPORTED(sound_sdl, OSD_SOUND_PROVIDER, "sdl")
#endif

MODULE_DEFINITION(SOUND_SDL, sound_sdl)
