// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  sound.c - Win32 implementation of MAME sound routines
//
//============================================================

#include "sound_module.h"
#include "modules/osdmodule.h"

#if defined(OSD_WINDOWS) || defined(SDLMAME_WIN32)

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

// undef WINNT for dsound.h to prevent duplicate definition
#undef WINNT
#include <dsound.h>
#undef interface

// MAME headers
#include "emu.h"
#include "osdepend.h"
#include "emuopts.h"

#ifdef SDLMAME_WIN32
#include "../../sdl/osdsdl.h"
#if (SDLMAME_SDL2)
#include <SDL2/SDL_syswm.h>
#else
#include <SDL/SDL_syswm.h>
#endif
#include "../../sdl/window.h"
#else
#include "winmain.h"
#include "window.h"
#endif

//============================================================
//  DEBUGGING
//============================================================

#define LOG_SOUND   0

#define LOG(x)      do { if (LOG_SOUND) osd_printf_verbose x; } while(0)


class sound_direct_sound : public osd_module, public sound_module
{
public:

	sound_direct_sound() :
		osd_module(OSD_SOUND_PROVIDER, "dsound"),
		sound_module(),
		m_dsound(NULL),
		m_bytes_per_sample(0),
		m_primary_buffer(),
		m_stream_buffer(),
		m_stream_buffer_in(0),
		m_buffer_underflows(0),
		m_buffer_overflows(0)
	{
	}
	virtual ~sound_direct_sound() { }

	virtual int init(osd_options const &options);
	virtual void exit();

	// sound_module
	virtual void update_audio_stream(bool is_throttled, INT16 const *buffer, int samples_this_frame);
	virtual void set_mastervolume(int attenuation);

private:
	class buffer
	{
	public:
		buffer() : m_buffer(NULL) { }
		~buffer() { release(); }

		ULONG release()
		{
			ULONG const result = m_buffer ? m_buffer->Release() : 0;
			m_buffer = NULL;
			return result;
		}

		operator bool() const { return m_buffer; }

	protected:
		LPDIRECTSOUNDBUFFER m_buffer;
	};

	class primary_buffer : public buffer
	{
	public:
		HRESULT create(LPDIRECTSOUND dsound)
		{
			assert(!m_buffer);
			DSBUFFERDESC desc;
			memset(&desc, 0, sizeof(desc));
			desc.dwSize = sizeof(desc);
			desc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_GETCURRENTPOSITION2;
			desc.lpwfxFormat = NULL;
			return dsound->CreateSoundBuffer(&desc, &m_buffer, NULL);
		}

		HRESULT get_format(WAVEFORMATEX &format)
		{
			assert(m_buffer);
			return m_buffer->GetFormat(&format, sizeof(format), NULL);
		}
		HRESULT set_format(WAVEFORMATEX const &format)
		{
			assert(m_buffer);
			return m_buffer->SetFormat(&format);
		}
	};

	class stream_buffer : public buffer
	{
	public:
		stream_buffer() : m_size(0), m_bytes1(NULL), m_bytes2(NULL), m_locked1(0), m_locked2(0) { }

		HRESULT create(LPDIRECTSOUND dsound, DWORD size, WAVEFORMATEX &format)
		{
			assert(!m_buffer);
			DSBUFFERDESC desc;
			memset(&desc, 0, sizeof(desc));
			desc.dwSize = sizeof(desc);
			desc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2;
			desc.dwBufferBytes = size;
			desc.lpwfxFormat = &format;
			m_size = size;
			return dsound->CreateSoundBuffer(&desc, &m_buffer, NULL);
		}

		HRESULT play_looping()
		{
			assert(m_buffer);
			return m_buffer->Play(0, 0, DSBPLAY_LOOPING);
		}
		HRESULT stop()
		{
			assert(m_buffer);
			return m_buffer->Stop();
		}
		HRESULT set_volume(LONG volume)
		{
			assert(m_buffer);
			return m_buffer->SetVolume(volume);
		}
		HRESULT set_min_volume() { return set_volume(DSBVOLUME_MIN); }

		HRESULT get_current_positions(DWORD &play_pos, DWORD &write_pos)
		{
			assert(m_buffer);
			return m_buffer->GetCurrentPosition(&play_pos, &write_pos);
		}
		HRESULT copy_data(DWORD cursor, DWORD bytes, void const *data)
		{
			HRESULT result = lock(cursor, bytes);
			if (DS_OK != result)
				return result;

			assert(m_bytes1);
			assert((m_locked1 + m_locked2) >= bytes);
			memcpy(m_bytes1, data, MIN(m_locked1, bytes));
			if (m_locked1 < bytes)
			{
				assert(m_bytes2);
				memcpy(m_bytes2, (UINT8 const *)data + m_locked1, bytes - m_locked1);
			}

			unlock();
			return DS_OK;
		}
		HRESULT clear()
		{
			HRESULT result = lock_all();
			if (DS_OK != result)
				return result;

			assert(m_bytes1);
			assert(!m_bytes2);
			assert(m_size == m_locked1);
			assert(0U == m_locked2);
			memset(m_bytes1, 0, m_locked1);

			unlock();
			return DS_OK;
		}

		DWORD size() const { return m_size; }

	protected:
		HRESULT lock(DWORD cursor, DWORD bytes)
		{
			assert(cursor < m_size);
			assert(bytes <= m_size);
			assert(m_buffer);
			assert(!m_bytes1);
			return m_buffer->Lock(
					cursor, bytes,
					&m_bytes1,
					&m_locked1,
					&m_bytes2,
					&m_locked2,
					0);
		}
		HRESULT lock_all() { return lock(0, m_size); }
		HRESULT unlock()
		{
			assert(m_buffer);
			assert(m_bytes1);
			HRESULT const result = m_buffer->Unlock(
					m_bytes1,
					m_locked1,
					m_bytes2,
					m_locked2);
			m_bytes1 = m_bytes2 = NULL;
			m_locked1 = m_locked2 = 0;
			return result;
		}

		DWORD m_size;
		void *m_bytes1, *m_bytes2;
		DWORD m_locked1, m_locked2;
	};

	HRESULT         dsound_init();
	void            dsound_kill();
	HRESULT         create_buffers(DWORD size, WAVEFORMATEX &format);
	void            destroy_buffers();

	// DirectSound objects
	LPDIRECTSOUND   m_dsound;

	// descriptors and formats
	UINT32          m_bytes_per_sample;

	// sound buffers
	primary_buffer  m_primary_buffer;
	stream_buffer   m_stream_buffer;
	UINT32          m_stream_buffer_in;

	// buffer over/underflow counts
	unsigned        m_buffer_underflows;
	unsigned        m_buffer_overflows;
};


//============================================================
//  init
//============================================================

int sound_direct_sound::init(osd_options const &options)
{
	// attempt to initialize directsound
	// don't make it fatal if we can't -- we'll just run without sound
	dsound_init();
	m_buffer_underflows = m_buffer_overflows = 0;
	return 0;
}


//============================================================
//  exit
//============================================================

void sound_direct_sound::exit()
{
	// kill the buffers and dsound
	destroy_buffers();
	dsound_kill();

	// print out over/underflow stats
	if (m_buffer_overflows || m_buffer_underflows)
	{
		osd_printf_verbose(
				"Sound: buffer overflows=%u underflows=%u\n",
				m_buffer_overflows,
				m_buffer_underflows);
	}

	LOG(("Sound buffer: overflows=%u underflows=%u\n", m_buffer_overflows, m_buffer_underflows));
}


//============================================================
//  update_audio_stream
//============================================================

void sound_direct_sound::update_audio_stream(
		bool is_throttled,
		INT16 const *buffer,
		int samples_this_frame)
{
	int const bytes_this_frame = samples_this_frame * m_bytes_per_sample;
	HRESULT result;

	// if no sound, there is no buffer
	if (!m_stream_buffer)
		return;

	// determine the current play position
	DWORD play_position, write_position;
	result = m_stream_buffer.get_current_positions(play_position, write_position);
	if (DS_OK != result)
		return;

//DWORD orig_write = write_position;
	// normalize the write position so it is always after the play position
	if (write_position < play_position)
		write_position += m_stream_buffer.size();

	// normalize the stream in position so it is always after the write position
	DWORD stream_in = m_stream_buffer_in;
	if (stream_in < write_position)
		stream_in += m_stream_buffer.size();

	// now we should have, in order:
	//    <------pp---wp---si--------------->

	// if we're between play and write positions, then bump forward, but only in full chunks
	while (stream_in < write_position)
	{
//printf("Underflow: PP=%d  WP=%d(%d)  SI=%d(%d)  BTF=%d\n", (int)play_position, (int)write_position, (int)orig_write, (int)stream_in, (int)m_stream_buffer_in, (int)bytes_this_frame);
		m_buffer_underflows++;
		stream_in += bytes_this_frame;
	}

	// if we're going to overlap the play position, just skip this chunk
	if ((stream_in + bytes_this_frame) > (play_position + m_stream_buffer.size()))
	{
//printf("Overflow: PP=%d  WP=%d(%d)  SI=%d(%d)  BTF=%d\n", (int)play_position, (int)write_position, (int)orig_write, (int)stream_in, (int)m_stream_buffer_in, (int)bytes_this_frame);
		m_buffer_overflows++;
		return;
	}

	// now we know where to copy; let's do it
	m_stream_buffer_in = stream_in % m_stream_buffer.size();
	result = m_stream_buffer.copy_data(m_stream_buffer_in, bytes_this_frame, buffer);

	// if we failed, assume it was an underflow (i.e.,
	if (result != DS_OK)
	{
		m_buffer_underflows++;
		return;
	}

	// adjust the input pointer
	m_stream_buffer_in = (m_stream_buffer_in + bytes_this_frame) % m_stream_buffer.size();
}


//============================================================
//  set_mastervolume
//============================================================

void sound_direct_sound::set_mastervolume(int attenuation)
{
	// clamp the attenuation to 0-32 range
	attenuation = MAX(MIN(attenuation, 0), -32);

	// set the master volume
	if (m_stream_buffer)
	{
		if (-32 == attenuation)
			m_stream_buffer.set_min_volume();
		else
			m_stream_buffer.set_volume(100 * attenuation);
	}
}


//============================================================
//  dsound_init
//============================================================

HRESULT sound_direct_sound::dsound_init()
{
	assert(!m_dsound);
	HRESULT result;

	// create the DirectSound object
	result = DirectSoundCreate(NULL, &m_dsound, NULL);
	if (result != DS_OK)
	{
		osd_printf_error("Error creating DirectSound: %08x\n", (unsigned)result);
		goto error;
	}

	// get the capabilities
	DSCAPS dsound_caps;
	dsound_caps.dwSize = sizeof(dsound_caps);
	result = m_dsound->GetCaps(&dsound_caps);
	if (result != DS_OK)
	{
		osd_printf_error("Error getting DirectSound capabilities: %08x\n", (unsigned)result);
		goto error;
	}

	// set the cooperative level
	{
#ifdef SDLMAME_WIN32
		SDL_SysWMinfo wminfo;
		SDL_VERSION(&wminfo.version);
#if SDLMAME_SDL2
		SDL_GetWindowWMInfo(sdl_window_list->sdl_window(), &wminfo);
		HWND const window = wminfo.info.win.window;
#else // SDLMAME_SDL2
		SDL_GetWMInfo(&wminfo);
		HWND const window = wminfo.window;
#endif // SDLMAME_SDL2
#else // SDLMAME_WIN32
		HWND const window = win_window_list->m_hwnd;
#endif // SDLMAME_WIN32
		result = m_dsound->SetCooperativeLevel(window, DSSCL_PRIORITY);
	}
	if (result != DS_OK)
	{
		osd_printf_error("Error setting DirectSound cooperative level: %08x\n", (unsigned)result);
		goto error;
	}

	{
		// make a format description for what we want
		WAVEFORMATEX stream_format;
		stream_format.wBitsPerSample    = 16;
		stream_format.wFormatTag        = WAVE_FORMAT_PCM;
		stream_format.nChannels         = 2;
		stream_format.nSamplesPerSec    = sample_rate();
		stream_format.nBlockAlign       = stream_format.wBitsPerSample * stream_format.nChannels / 8;
		stream_format.nAvgBytesPerSec   = stream_format.nSamplesPerSec * stream_format.nBlockAlign;

		// compute the buffer size based on the output sample rate
		DWORD stream_buffer_size = stream_format.nSamplesPerSec * stream_format.nBlockAlign * m_audio_latency / 10;
		stream_buffer_size = MAX(1024, (stream_buffer_size / 1024) * 1024);

		LOG(("stream_buffer_size = %u\n", (unsigned)stream_buffer_size));

		// create the buffers
		m_bytes_per_sample = stream_format.nBlockAlign;
		m_stream_buffer_in = 0;
		result = create_buffers(stream_buffer_size, stream_format);
		if (result != DS_OK)
			goto error;
	}

	// start playing
	result = m_stream_buffer.play_looping();
	if (result != DS_OK)
	{
		osd_printf_error("Error playing: %08x\n", (UINT32)result);
		goto error;
	}
	return DS_OK;

	// error handling
error:
	destroy_buffers();
	dsound_kill();
	return result;
}


//============================================================
//  dsound_kill
//============================================================

void sound_direct_sound::dsound_kill()
{
	// release the object
	if (m_dsound)
		m_dsound->Release();
	m_dsound = NULL;
}


//============================================================
//  create_buffers
//============================================================

HRESULT sound_direct_sound::create_buffers(DWORD size, WAVEFORMATEX &format)
{
	assert(m_dsound);
	assert(!m_primary_buffer);
	assert(!m_stream_buffer);
	HRESULT result;

	// create the primary buffer
	result = m_primary_buffer.create(m_dsound);
	if (result != DS_OK)
	{
		osd_printf_error("Error creating primary DirectSound buffer: %08x\n", (unsigned)result);
		goto error;
	}

	// attempt to set the primary format
	result = m_primary_buffer.set_format(format);
	if (result != DS_OK)
	{
		osd_printf_error("Error setting primary DirectSound buffer format: %08x\n", (unsigned)result);
		goto error;
	}

	// log the primary format
	WAVEFORMATEX primary_format;
	result = m_primary_buffer.get_format(primary_format);
	if (result != DS_OK)
	{
		osd_printf_error("Error getting primary DirectSound buffer format: %08x\n", (unsigned)result);
		goto error;
	}
	osd_printf_verbose(
			"DirectSound: Primary buffer: %d Hz, %d bits, %d channels\n",
			(int)primary_format.nSamplesPerSec,
			(int)primary_format.wBitsPerSample,
			(int)primary_format.nChannels);

	// create the stream buffer
	result = m_stream_buffer.create(m_dsound, size, format);
	if (result != DS_OK)
	{
		osd_printf_error("Error creating DirectSound stream buffer: %08x\n", (unsigned)result);
		goto error;
	}

	// clear the buffer
	result = m_stream_buffer.clear();
	if (result != DS_OK)
	{
		osd_printf_error("Error locking DirectSound stream buffer: %08x\n", (unsigned)result);
		goto error;
	}

	return DS_OK;

	// error handling
error:
	destroy_buffers();
	return result;
}


//============================================================
//  destroy_buffers
//============================================================

void sound_direct_sound::destroy_buffers(void)
{
	// stop any playback
	if (m_stream_buffer)
		m_stream_buffer.stop();

	// release the stream buffer
	m_stream_buffer.release();

	// release the primary buffer
	m_primary_buffer.release();
}

#else // defined(OSD_WINDOWS) || defined(SDLMAME_WIN32)
	MODULE_NOT_SUPPORTED(sound_direct_sound, OSD_SOUND_PROVIDER, "dsound")
#endif // defined(OSD_WINDOWS) || defined(SDLMAME_WIN32)

MODULE_DEFINITION(SOUND_DSOUND, sound_direct_sound)
