// license:BSD-3-Clause
// copyright-holders:Brad Hughes
//====================================================================
//
//  xaudio2_sound.cpp - XAudio2 implementation of MAME sound routines
//
//====================================================================

#include "sound_module.h"
#include "modules/osdmodule.h"

#if (defined(OSD_WINDOWS) && USE_XAUDIO2)

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mutex>

#pragma warning( push )
#pragma warning( disable: 4068 )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
// XAudio2 include
#include <xaudio2.h>
#pragma GCC diagnostic pop
#pragma warning( pop )


#include <mmsystem.h>

// stdlib includes
#include <thread>
#include <queue>

#undef interface

// MAME headers
#include "emu.h"
#include "osdepend.h"
#include "emuopts.h"

//============================================================
//  Constants
//============================================================

#define INITIAL_BUFFER_COUNT 4
#define SUBMIT_FREQUENCY_TARGET_MS 20

//============================================================
//  Macros
//============================================================

// Check HRESULT result and log if error, then take an optional action on failure
#define HR_LOG( CALL, LOGFN, ONFAIL ) do { \
	result = CALL; \
	if (FAILED(result)) { \
		LOGFN(#CALL " failed with error 0x%X\n", (unsigned int)result); \
		ONFAIL; } \
} while (0)

// Variant of HR_LOG to log using osd_printf_error
#define HR_LOGE( CALL, ONFAIL ) HR_LOG(CALL, osd_printf_error, ONFAIL)

// Variant of HR_LOG to log using osd_printf_verbose
#define HR_LOGV( CALL, ONFAIL ) HR_LOG(CALL, osd_printf_verbose, ONFAIL)

// Macro to check for a failed HRESULT and if failed, goto a label called Error:
#define HR_GOERR( CALL ) HR_LOGE( CALL, goto Error;)

// Macro to check for a failed HRESULT and if failed, return the specified value
#define HR_RET( CALL, ret ) HR_LOGE(CALL, return ret;)

// Macro to check for a failed HRESULT and if failed, return nothing (void function)
#define HR_RETV( CALL ) HR_RET(CALL,)

// Macro to check for a failed HRESULT and if failed, return 0
#define HR_RET0( CALL ) HR_RET(CALL, 0)

// Macro to check for a failed HRESULT and if failed, return the HRESULT
#define HR_RETHR( CALL ) HR_RET(CALL, result)

// Macro to check for a failed HRESULT and if failed, return 1
#define HR_RET1( CALL ) HR_RET(CALL, 1)

// Macro to check for a failed HRESULT and if failed, log verbose, and proceed as normal
#define HR_IGNORE( CALL ) HR_LOGV(CALL,)

//============================================================
//  Structs and typedefs
//============================================================

// A stucture to hold a pointer and the count of bytes of the data it points to
struct xaudio2_buffer
{
	std::unique_ptr<BYTE[]> AudioData;
	DWORD                   AudioSize;
};

// Custom deleter with overloads to free smart pointer types used in the implementations
struct xaudio2_custom_deleter
{
public:
	void operator()(IXAudio2* obj) const
	{
		if (obj != nullptr)
		{
			obj->Release();
		}
	}

	void operator()(IXAudio2MasteringVoice* obj) const
	{
		if (obj != nullptr)
		{
			obj->DestroyVoice();
		}
	}

	void operator()(IXAudio2SourceVoice* obj) const
	{
		if (obj != nullptr)
		{
			obj->Stop(0);
			obj->FlushSourceBuffers();
			obj->DestroyVoice();
		}
	}
};

// Typedefs for smart pointers used with customer deleters
typedef std::unique_ptr<IXAudio2, xaudio2_custom_deleter> xaudio2_ptr;
typedef std::unique_ptr<IXAudio2MasteringVoice, xaudio2_custom_deleter> mastering_voice_ptr;
typedef std::unique_ptr<IXAudio2SourceVoice, xaudio2_custom_deleter> src_voice_ptr;

// Typedef for pointer to XAudio2Create
typedef HRESULT(__stdcall* PFN_XAUDIO2CREATE)(IXAudio2**, UINT32, XAUDIO2_PROCESSOR);

//============================================================
//  Helper classes
//============================================================

// Provides a pool of buffers
class bufferpool
{
private:
	int m_initial;
	int m_buffersize;
	std::queue<std::unique_ptr<BYTE[]>> m_queue;

public:
	// constructor
	bufferpool(int capacity, int bufferSize) :
		m_initial(capacity),
		m_buffersize(bufferSize)
	{
		for (int i = 0; i < m_initial; i++)
		{
			auto newBuffer = std::make_unique<BYTE[]>(m_buffersize);
			memset(newBuffer.get(), 0, m_buffersize);
			m_queue.push(std::move(newBuffer));
		}
	}

	// get next buffer element from the pool
	BYTE* next()
	{
		BYTE* next_buffer;
		if (!m_queue.empty())
		{
			next_buffer = m_queue.front().release();
			m_queue.pop();
		}
		else
		{
			next_buffer = new BYTE[m_buffersize];
			memset(next_buffer, 0, m_buffersize);
		}

		return next_buffer;
	}

	// release element, make it available back in the pool
	void return_to_pool(BYTE* buffer)
	{
		auto returned_buf = std::unique_ptr<BYTE[]>(buffer);
		memset(returned_buf.get(), 0, m_buffersize);
		m_queue.push(std::move(returned_buf));
	}
};

//============================================================
//  sound_xaudio2 class
//============================================================

// The main class for the XAudio2 sound module implementation
class sound_xaudio2 : public osd_module, public sound_module, public IXAudio2VoiceCallback
{
private:
	xaudio2_ptr                                 m_xAudio2;
	mastering_voice_ptr                         m_masterVoice;
	src_voice_ptr                               m_sourceVoice;
	DWORD                                       m_sample_bytes;
	std::unique_ptr<BYTE[]>                     m_buffer;
	DWORD                                       m_buffer_size;
	DWORD                                       m_buffer_count;
	DWORD                                       m_writepos;
	std::mutex                                  m_buffer_lock;
	HANDLE                                      m_hEventBufferCompleted;
	HANDLE                                      m_hEventDataAvailable;
	HANDLE                                      m_hEventExiting;
	std::thread                                 m_audioThread;
	std::queue<xaudio2_buffer>                  m_queue;
	std::unique_ptr<bufferpool>                 m_buffer_pool;
	HMODULE                                     m_xaudio2_module;
	PFN_XAUDIO2CREATE                           m_pfnxaudio2create;
	UINT32                                      m_overflows;
	UINT32                                      m_underflows;
	BOOL                                        m_in_underflow;

public:
	sound_xaudio2() :
		osd_module(OSD_SOUND_PROVIDER, "xaudio2"),
		sound_module(),
		m_xAudio2(nullptr),
		m_masterVoice(nullptr),
		m_sourceVoice(nullptr),
		m_sample_bytes(0),
		m_buffer(nullptr),
		m_buffer_size(0),
		m_buffer_count(0),
		m_writepos(0),
		m_hEventBufferCompleted(NULL),
		m_hEventDataAvailable(NULL),
		m_hEventExiting(NULL),
		m_buffer_pool(nullptr),
		m_xaudio2_module(NULL),
		m_pfnxaudio2create(nullptr),
		m_overflows(0),
		m_underflows(0),
		m_in_underflow(FALSE)
	{
	}

	virtual int init(osd_options const &options) override;
	virtual void exit() override;

	// sound_module
	virtual void update_audio_stream(bool is_throttled, INT16 const *buffer, int samples_this_frame) override;
	virtual void set_mastervolume(int attenuation) override;

	// Xaudio callbacks
	void OnVoiceProcessingPassStart(UINT32 bytes_required) override;
	void OnVoiceProcessingPassEnd() override {}
	void OnStreamEnd() override {}
	void OnBufferStart(void* pBufferContext) override {}
	void OnLoopEnd(void* pBufferContext) override {}
	void OnVoiceError(void* pBufferContext, HRESULT error) override {}
	void OnBufferEnd(void *pBufferContext) override;

private:
	void create_buffers(const WAVEFORMATEX &format);
	HRESULT create_voices(const WAVEFORMATEX &format);
	void process_audio();
	void submit_buffer(std::unique_ptr<BYTE[]> audioData, DWORD audioLength);
	void submit_needed();
	HRESULT xaudio2_create(IXAudio2 ** xaudio2_interface);
	void roll_buffer();
	BOOL submit_next_queued();
};

//============================================================
//  init
//============================================================

int sound_xaudio2::init(osd_options const &options)
{
	HRESULT result = S_OK;

	// Create the IXAudio2 object
	IXAudio2 *temp_xaudio2 = nullptr;
	HR_RET1(xaudio2_create(&temp_xaudio2));
	m_xAudio2 = xaudio2_ptr(temp_xaudio2);

	// make a format description for what we want
	WAVEFORMATEX format = { 0 };
	format.wBitsPerSample = 16;
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = 2;
	format.nSamplesPerSec = sample_rate();
	format.nBlockAlign = format.wBitsPerSample * format.nChannels / 8;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;

	m_sample_bytes = format.nBlockAlign;

#if defined(_DEBUG)
	XAUDIO2_DEBUG_CONFIGURATION debugConfig = { 0 };
	debugConfig.TraceMask = XAUDIO2_LOG_WARNINGS | XAUDIO2_LOG_TIMING | XAUDIO2_LOG_STREAMING;
	debugConfig.LogFunctionName = TRUE;
	m_xAudio2->SetDebugConfiguration(&debugConfig);
#endif

	// Create the buffers
	create_buffers(format);

	// Initialize our events
	m_hEventBufferCompleted = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hEventDataAvailable = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hEventExiting = CreateEvent(NULL, FALSE, FALSE, NULL);

	// create the voices and start them
	HR_RET1(create_voices(format));
	HR_RET1(m_sourceVoice->Start());

	// Start the thread listening
	m_audioThread = std::thread([](sound_xaudio2* self) { self->process_audio(); }, this);

	osd_printf_verbose("Sound: XAudio2 initialized\n");

	return 0;
}

//============================================================
//  exit
//============================================================

void sound_xaudio2::exit()
{
	// Wait on processing thread to end
	SetEvent(m_hEventExiting);
	m_audioThread.join();

	CloseHandle(m_hEventBufferCompleted);
	CloseHandle(m_hEventDataAvailable);
	CloseHandle(m_hEventExiting);

	m_sourceVoice.reset();
	m_masterVoice.reset();
	m_xAudio2.reset();
	m_buffer.reset();
	m_buffer_pool.reset();

	if (m_overflows != 0 || m_underflows != 0)
		osd_printf_verbose("Sound: overflows=%u, underflows=%u\n", m_overflows, m_underflows);

	osd_printf_verbose("Sound: XAudio2 deinitialized\n");
}

//============================================================
//  update_audio_stream
//============================================================

void sound_xaudio2::update_audio_stream(
	bool is_throttled,
	INT16 const *buffer,
	int samples_this_frame)
{
	if ((sample_rate() == 0) || !m_buffer)
		return;

	UINT32 const bytes_this_frame = samples_this_frame * m_sample_bytes;

	std::lock_guard<std::mutex> lock(m_buffer_lock);

	UINT32 bytes_left = bytes_this_frame;

	while (bytes_left > 0)
	{
		UINT32 chunk = MIN(m_buffer_size, bytes_left);

		// Roll the buffer if needed
		if (m_writepos + chunk >= m_buffer_size)
		{
			roll_buffer();
		}

		// Copy in the data
		memcpy(m_buffer.get() + m_writepos, buffer, chunk);
		m_writepos += chunk;
		bytes_left -= chunk;
	}

	// Signal data available
	SetEvent(m_hEventDataAvailable);
}

//============================================================
//  set_mastervolume
//============================================================

void sound_xaudio2::set_mastervolume(int attenuation)
{
	assert(m_sourceVoice);

	HRESULT result;

	// clamp the attenuation to 0-32 range
	attenuation = MAX(MIN(attenuation, 0), -32);

	// Ranges from 1.0 to XAUDIO2_MAX_VOLUME_LEVEL indicate additional gain
	// Ranges from 0 to 1.0 indicate a reduced volume level
	// 0 indicates silence
	// We only support a reduction from 1.0, so we generate values in the range 0.0 to 1.0
	float scaledVolume = (32.0f + attenuation) / 32.0f;

	// set the master volume
	HR_RETV(m_sourceVoice->SetVolume(scaledVolume));
}

//============================================================
//  IXAudio2VoiceCallback::OnBufferEnd
//============================================================

// The XAudio2 voice callback triggered when a buffer finishes playing
void sound_xaudio2::OnBufferEnd(void *pBufferContext)
{
	BYTE* completed_buffer = (BYTE*)pBufferContext;
	if (completed_buffer != nullptr)
	{
		std::lock_guard<std::mutex> lock(m_buffer_lock);
		m_buffer_pool->return_to_pool(completed_buffer);
	}

	SetEvent(m_hEventBufferCompleted);
}

//============================================================
//  IXAudio2VoiceCallback::OnVoiceProcessingPassStart
//============================================================

// The XAudio2 voice callback triggered on every pass
void sound_xaudio2::OnVoiceProcessingPassStart(UINT32 bytes_required)
{
	if (bytes_required == 0)
	{
		// Reset underflow indicator if we're caught up
		if (m_in_underflow) m_in_underflow = FALSE;

		return;
	}

	// Since there are bytes required, we're going to be in underflow
	if (!m_in_underflow)
	{
		m_underflows++;
		m_in_underflow = TRUE;
	}
}

//============================================================
//  xaudio2_create
//============================================================

// Dynamically loads the XAudio2 DLL and calls the exported XAudio2Create()
HRESULT sound_xaudio2::xaudio2_create(IXAudio2 ** ppxaudio2_interface)
{
	HRESULT result;

	if (nullptr == m_pfnxaudio2create)
	{
		if (nullptr == m_xaudio2_module)
		{
			m_xaudio2_module = LoadLibrary(XAUDIO2_DLL);
			if (nullptr == m_xaudio2_module)
			{
				osd_printf_error("Failed to load module '%S', error: 0x%X\n", XAUDIO2_DLL, (unsigned int)GetLastError());
				HR_RETHR(E_FAIL);
			}
		}

		m_pfnxaudio2create = (PFN_XAUDIO2CREATE)GetProcAddress(m_xaudio2_module, "XAudio2Create");
		if (nullptr == m_pfnxaudio2create)
		{
			osd_printf_error("Failed to get adddress of exported function XAudio2Create, error: 0x%X\n", (unsigned int)GetLastError());
			HR_RETHR(E_FAIL);
		}
	}

	HR_RETHR(m_pfnxaudio2create(ppxaudio2_interface, 0, XAUDIO2_DEFAULT_PROCESSOR));

	return S_OK;
}

//============================================================
//  create_buffers
//============================================================

void sound_xaudio2::create_buffers(const WAVEFORMATEX &format)
{
	// Compute the buffer size
	// buffer size is equal to the bytes we need to hold in memory per X tenths of a second where X is audio_latency
	float audio_latency_in_seconds = m_audio_latency / 10.0f;
	UINT32 format_bytes_per_second = format.nSamplesPerSec * format.nBlockAlign;
	UINT32 total_buffer_size = format_bytes_per_second * audio_latency_in_seconds;

	// We want to be able to submit buffers every X milliseconds
	// I want to divide these up into "packets" so figure out how many buffers we need
	m_buffer_count = (audio_latency_in_seconds * 1000.0f) / SUBMIT_FREQUENCY_TARGET_MS;

	// Now record the size of the individual buffers
	m_buffer_size = MAX(1024, total_buffer_size / m_buffer_count);

	// Make the buffer a multiple of the format size bytes (rounding up)
	UINT32 remainder = m_buffer_size % format.nBlockAlign;
	if (remainder != 0)
		m_buffer_size += format.nBlockAlign - remainder;

	// get our initial buffer pool and our first buffer
	m_buffer_pool = std::make_unique<bufferpool>(m_buffer_count + 1, m_buffer_size);
	m_buffer = std::unique_ptr<BYTE[]>(m_buffer_pool->next());

	osd_printf_verbose(
		"Sound: XAudio2 created initial buffers. total size: %u, count %u, size each %u\n",
		(unsigned int)total_buffer_size,
		(unsigned int)m_buffer_count,
		(unsigned int)m_buffer_size);

	// reset buffer states
	m_writepos = 0;
	m_overflows = 0;
	m_underflows = 0;
}

//============================================================
//  create_voices
//============================================================

HRESULT sound_xaudio2::create_voices(const WAVEFORMATEX &format)
{
	assert(m_xAudio2);
	assert(!m_masterVoice);
	HRESULT result;

	IXAudio2MasteringVoice *temp_master_voice = nullptr;
	HR_RET1(
		m_xAudio2->CreateMasteringVoice(
			&temp_master_voice,
			format.nChannels,
			sample_rate()));

	m_masterVoice = mastering_voice_ptr(temp_master_voice);

	// create the source voice
	IXAudio2SourceVoice *temp_source_voice = nullptr;
	HR_RET1(m_xAudio2->CreateSourceVoice(
		&temp_source_voice,
		&format,
		XAUDIO2_VOICE_NOSRC | XAUDIO2_VOICE_NOPITCH,
		1.0,
		this));

	m_sourceVoice = src_voice_ptr(temp_source_voice);

	return S_OK;
}

//============================================================
//  process_audio
//============================================================

// submits audio events on another thread in a loop
void sound_xaudio2::process_audio()
{
	BOOL exiting = FALSE;
	HANDLE hEvents[] = { m_hEventBufferCompleted, m_hEventDataAvailable, m_hEventExiting };
	while (!exiting)
	{
		DWORD wait_result = WaitForMultipleObjects(3, hEvents, FALSE, INFINITE);
		switch (wait_result)
		{
			// Buffer is complete or new data is available
		case 0:
		case 1:
			submit_needed();
			break;
		case 2:
			// exiting
			exiting = TRUE;
			break;
		}
	}
}

//============================================================
//  submit_needed
//============================================================

// Submits any buffers that have currently been queued,
// assuming they are needed based on current queue depth
void sound_xaudio2::submit_needed()
{
	XAUDIO2_VOICE_STATE state;
	m_sourceVoice->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);

	// If we have a buffer on the queue, no reason to submit
	if (state.BuffersQueued >= 1)
		return;

	std::lock_guard<std::mutex> lock(m_buffer_lock);

	// Roll the buffer
	roll_buffer();

	// Submit the next buffer
	submit_next_queued();
}

//============================================================
//  submit_buffer
//============================================================

void sound_xaudio2::submit_buffer(std::unique_ptr<BYTE[]> audioData, DWORD audioLength)
{
	assert(audioLength != 0);

	XAUDIO2_BUFFER buf = { 0 };
	buf.AudioBytes = audioLength;
	buf.pAudioData = audioData.get();
	buf.PlayBegin = 0;
	buf.PlayLength = audioLength / m_sample_bytes;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	buf.pContext = audioData.get();

	HRESULT result;
	if (FAILED(result = m_sourceVoice->SubmitSourceBuffer(&buf)))
	{
		osd_printf_verbose("Sound: XAudio2 failed to submit source buffer (non-fatal). Error: 0x%X\n", (unsigned int)result);
		m_buffer_pool->return_to_pool(audioData.release());
		return;
	}

	// If we succeeded, relinquish the buffer allocation to the XAudio2 runtime
	// The buffer will be freed on the OnBufferCompleted callback
	audioData.release();
}

//============================================================
//  submit_next_queued
//============================================================

BOOL sound_xaudio2::submit_next_queued()
{
	if (!m_queue.empty())
	{
		// Get a reference to the buffer
		auto buf = &m_queue.front();

		// submit the buffer data
		submit_buffer(std::move(buf->AudioData), buf->AudioSize);

		// Remove it from the queue
		assert(buf->AudioSize > 0);
		m_queue.pop();

		return !m_queue.empty();
	}

	// queue was already empty
	return FALSE;
}

//============================================================
//  roll_buffer
//============================================================

// Queues the current buffer, and gets a new write buffer
void sound_xaudio2::roll_buffer()
{
	// Don't queue a buffer if it is empty
	if (m_writepos == 0)
		return;

	// Queue the current buffer
	xaudio2_buffer buf;
	buf.AudioData = std::move(m_buffer);
	buf.AudioSize = m_writepos;
	m_queue.push(std::move(buf));

	// Get a new buffer
	m_buffer = std::unique_ptr<BYTE[]>(m_buffer_pool->next());
	m_writepos = 0;

	// We only want to keep a maximum number of buffers at any given time
	// so remove any from queue greater than MAX_QUEUED_BUFFERS
	if (m_queue.size() > m_buffer_count)
	{
		xaudio2_buffer *next_buffer = &m_queue.front();

		// return the oldest buffer to the pool, and remove it from queue
		m_buffer_pool->return_to_pool(next_buffer->AudioData.release());
		m_queue.pop();

		m_overflows++;
	}
}


#else
MODULE_NOT_SUPPORTED(sound_xaudio2, OSD_SOUND_PROVIDER, "xaudio2")
#endif

MODULE_DEFINITION(SOUND_XAUDIO2, sound_xaudio2)
