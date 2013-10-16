// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  sound.c - Win32 implementation of MAME sound routines
//
//============================================================

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

// MAMEOS headers
#include "winmain.h"
#include "window.h"
#include "video.h"
#include "config.h"
#include "sound.h"


//============================================================
//  DEBUGGING
//============================================================

#define LOG_SOUND               0

#define LOG(x) do { if (LOG_SOUND) logerror x; } while(0)


//============================================================
//  LOCAL VARIABLES
//============================================================

// DirectSound objects
static LPDIRECTSOUND        dsound;
static DSCAPS               dsound_caps;

// sound buffers
static LPDIRECTSOUNDBUFFER  primary_buffer;
static LPDIRECTSOUNDBUFFER  stream_buffer;
static UINT32               stream_buffer_size;
static UINT32               stream_buffer_in;

// descriptors and formats
static DSBUFFERDESC         primary_desc;
static DSBUFFERDESC         stream_desc;
static WAVEFORMATEX         primary_format;
static WAVEFORMATEX         stream_format;

// buffer over/underflow counts
static int                  buffer_underflows;
static int                  buffer_overflows;


//============================================================
//  PROTOTYPES
//============================================================

static void         sound_exit(running_machine &machine);
static HRESULT      dsound_init(running_machine &machine);
static void         dsound_kill(void);
static HRESULT      dsound_create_buffers(void);
static void         dsound_destroy_buffers(void);



//============================================================
//  winsound_init
//============================================================

void winsound_init(running_machine &machine)
{
	// if no sound, don't create anything
	if (!machine.options().sound())
		return;

	// ensure we get called on the way out
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(sound_exit), &machine));

	// attempt to initialize directsound
	// don't make it fatal if we can't -- we'll just run without sound
	dsound_init(machine);
}


//============================================================
//  sound_exit
//============================================================

static void sound_exit(running_machine &machine)
{
	// kill the buffers and dsound
	dsound_destroy_buffers();
	dsound_kill();

	// print out over/underflow stats
	if (buffer_overflows || buffer_underflows)
		mame_printf_verbose("Sound: buffer overflows=%d underflows=%d\n", buffer_overflows, buffer_underflows);

	LOG(("Sound buffer: overflows=%d underflows=%d\n", buffer_overflows, buffer_underflows));
}


//============================================================
//  copy_sample_data
//============================================================

static void copy_sample_data(const INT16 *data, int bytes_to_copy)
{
	void *buffer1, *buffer2;
	DWORD length1, length2;
	HRESULT result;
	int cur_bytes;

	// attempt to lock the stream buffer
	result = IDirectSoundBuffer_Lock(stream_buffer, stream_buffer_in, bytes_to_copy, &buffer1, &length1, &buffer2, &length2, 0);

	// if we failed, assume it was an underflow (i.e.,
	if (result != DS_OK)
	{
		buffer_underflows++;
		return;
	}

	// adjust the input pointer
	stream_buffer_in = (stream_buffer_in + bytes_to_copy) % stream_buffer_size;

	// copy the first chunk
	cur_bytes = (bytes_to_copy > length1) ? length1 : bytes_to_copy;
	memcpy(buffer1, data, cur_bytes);

	// adjust for the number of bytes
	bytes_to_copy -= cur_bytes;
	data = (INT16 *)((UINT8 *)data + cur_bytes);

	// copy the second chunk
	if (bytes_to_copy != 0)
	{
		cur_bytes = (bytes_to_copy > length2) ? length2 : bytes_to_copy;
		memcpy(buffer2, data, cur_bytes);
	}

	// unlock
	result = IDirectSoundBuffer_Unlock(stream_buffer, buffer1, length1, buffer2, length2);
}


//============================================================
//  update_audio_stream
//============================================================

void windows_osd_interface::update_audio_stream(const INT16 *buffer, int samples_this_frame)
{
	int bytes_this_frame = samples_this_frame * stream_format.nBlockAlign;
	DWORD play_position, write_position;
	HRESULT result;

	// if no sound, there is no buffer
	if (stream_buffer == NULL)
		return;

	// determine the current play position
	result = IDirectSoundBuffer_GetCurrentPosition(stream_buffer, &play_position, &write_position);
	if (result == DS_OK)
	{
		DWORD stream_in;

//DWORD orig_write = write_position;
		// normalize the write position so it is always after the play position
		if (write_position < play_position)
			write_position += stream_buffer_size;

		// normalize the stream in position so it is always after the write position
		stream_in = stream_buffer_in;
		if (stream_in < write_position)
			stream_in += stream_buffer_size;

		// now we should have, in order:
		//    <------pp---wp---si--------------->

		// if we're between play and write positions, then bump forward, but only in full chunks
		while (stream_in < write_position)
		{
//printf("Underflow: PP=%d  WP=%d(%d)  SI=%d(%d)  BTF=%d\n", (int)play_position, (int)write_position, (int)orig_write, (int)stream_in, (int)stream_buffer_in, (int)bytes_this_frame);
			buffer_underflows++;
			stream_in += bytes_this_frame;
		}

		// if we're going to overlap the play position, just skip this chunk
		if (stream_in + bytes_this_frame > play_position + stream_buffer_size)
		{
//printf("Overflow: PP=%d  WP=%d(%d)  SI=%d(%d)  BTF=%d\n", (int)play_position, (int)write_position, (int)orig_write, (int)stream_in, (int)stream_buffer_in, (int)bytes_this_frame);
			buffer_overflows++;
			return;
		}

		// now we know where to copy; let's do it
		stream_buffer_in = stream_in % stream_buffer_size;
		copy_sample_data(buffer, bytes_this_frame);
	}
}


//============================================================
//  set_mastervolume
//============================================================

void windows_osd_interface::set_mastervolume(int attenuation)
{
	// clamp the attenuation to 0-32 range
	if (attenuation > 0)
		attenuation = 0;
	if (attenuation < -32)
		attenuation = -32;

	// set the master volume
	if (stream_buffer != NULL)
		IDirectSoundBuffer_SetVolume(stream_buffer, (attenuation == -32) ? DSBVOLUME_MIN : attenuation * 100);
}


//============================================================
//  dsound_init
//============================================================

static HRESULT dsound_init(running_machine &machine)
{
	HRESULT result;

	// create the DirectSound object
	result = DirectSoundCreate(NULL, &dsound, NULL);
	if (result != DS_OK)
	{
		mame_printf_error("Error creating DirectSound: %08x\n", (UINT32)result);
		goto error;
	}

	// get the capabilities
	dsound_caps.dwSize = sizeof(dsound_caps);
	result = IDirectSound_GetCaps(dsound, &dsound_caps);
	if (result != DS_OK)
	{
		mame_printf_error("Error getting DirectSound capabilities: %08x\n", (UINT32)result);
		goto error;
	}

	// set the cooperative level
	result = IDirectSound_SetCooperativeLevel(dsound, win_window_list->hwnd, DSSCL_PRIORITY);
	if (result != DS_OK)
	{
		mame_printf_error("Error setting DirectSound cooperative level: %08x\n", (UINT32)result);
		goto error;
	}

	// make a format description for what we want
	stream_format.wBitsPerSample    = 16;
	stream_format.wFormatTag        = WAVE_FORMAT_PCM;
	stream_format.nChannels         = 2;
	stream_format.nSamplesPerSec    = machine.sample_rate();
	stream_format.nBlockAlign       = stream_format.wBitsPerSample * stream_format.nChannels / 8;
	stream_format.nAvgBytesPerSec   = stream_format.nSamplesPerSec * stream_format.nBlockAlign;

	// compute the buffer size based on the output sample rate
	stream_buffer_size = stream_format.nSamplesPerSec * stream_format.nBlockAlign * downcast<windows_options &>(machine.options()).audio_latency() / 10;
	stream_buffer_size = (stream_buffer_size / 1024) * 1024;
	if (stream_buffer_size < 1024)
		stream_buffer_size = 1024;

	LOG(("stream_buffer_size = %d\n", stream_buffer_size));

	// create the buffers
	result = dsound_create_buffers();
	if (result != DS_OK)
		goto error;

	// start playing
	result = IDirectSoundBuffer_Play(stream_buffer, 0, 0, DSBPLAY_LOOPING);
	if (result != DS_OK)
	{
		mame_printf_error("Error playing: %08x\n", (UINT32)result);
		goto error;
	}
	return DS_OK;

	// error handling
error:
	dsound_destroy_buffers();
	dsound_kill();
	return result;
}


//============================================================
//  dsound_kill
//============================================================

static void dsound_kill(void)
{
	// release the object
	if (dsound != NULL)
		IDirectSound_Release(dsound);
	dsound = NULL;
}


//============================================================
//  dsound_create_buffers
//============================================================

static HRESULT dsound_create_buffers(void)
{
	HRESULT result;
	void *buffer;
	DWORD locked;

	// create a buffer desc for the primary buffer
	memset(&primary_desc, 0, sizeof(primary_desc));
	primary_desc.dwSize = sizeof(primary_desc);
	primary_desc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_GETCURRENTPOSITION2;
	primary_desc.lpwfxFormat = NULL;

	// create the primary buffer
	result = IDirectSound_CreateSoundBuffer(dsound, &primary_desc, &primary_buffer, NULL);
	if (result != DS_OK)
	{
		mame_printf_error("Error creating primary DirectSound buffer: %08x\n", (UINT32)result);
		goto error;
	}

	// attempt to set the primary format
	result = IDirectSoundBuffer_SetFormat(primary_buffer, &stream_format);
	if (result != DS_OK)
	{
		mame_printf_error("Error setting primary DirectSound buffer format: %08x\n", (UINT32)result);
		goto error;
	}

	// get the primary format
	result = IDirectSoundBuffer_GetFormat(primary_buffer, &primary_format, sizeof(primary_format), NULL);
	if (result != DS_OK)
	{
		mame_printf_error("Error getting primary DirectSound buffer format: %08x\n", (UINT32)result);
		goto error;
	}
	mame_printf_verbose("DirectSound: Primary buffer: %d Hz, %d bits, %d channels\n",
				(int)primary_format.nSamplesPerSec, (int)primary_format.wBitsPerSample, (int)primary_format.nChannels);

	// create a buffer desc for the stream buffer
	memset(&stream_desc, 0, sizeof(stream_desc));
	stream_desc.dwSize = sizeof(stream_desc);
	stream_desc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2;
	stream_desc.dwBufferBytes = stream_buffer_size;
	stream_desc.lpwfxFormat = &stream_format;

	// create the stream buffer
	result = IDirectSound_CreateSoundBuffer(dsound, &stream_desc, &stream_buffer, NULL);
	if (result != DS_OK)
	{
		mame_printf_error("Error creating DirectSound stream buffer: %08x\n", (UINT32)result);
		goto error;
	}

	// lock the buffer
	result = IDirectSoundBuffer_Lock(stream_buffer, 0, stream_buffer_size, &buffer, &locked, NULL, NULL, 0);
	if (result != DS_OK)
	{
		mame_printf_error("Error locking DirectSound stream buffer: %08x\n", (UINT32)result);
		goto error;
	}

	// clear the buffer and unlock it
	memset(buffer, 0, locked);
	IDirectSoundBuffer_Unlock(stream_buffer, buffer, locked, NULL, 0);
	return DS_OK;

	// error handling
error:
	dsound_destroy_buffers();
	return result;
}


//============================================================
//  dsound_destroy_buffers
//============================================================

static void dsound_destroy_buffers(void)
{
	// stop any playback
	if (stream_buffer != NULL)
		IDirectSoundBuffer_Stop(stream_buffer);

	// release the stream buffer
	if (stream_buffer != NULL)
		IDirectSoundBuffer_Release(stream_buffer);
	stream_buffer = NULL;

	// release the primary buffer
	if (primary_buffer != NULL)
		IDirectSoundBuffer_Release(primary_buffer);
	primary_buffer = NULL;
}
