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
#define MIN_QUEUE_DEPTH 1

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

    void operator()(osd_lock* obj) const
    {
        if (obj != nullptr)
        {
            osd_lock_free(obj);
        }
    }
};

// Typedefs for smart pointers used with customer deleters
typedef std::unique_ptr<IXAudio2, xaudio2_custom_deleter> xaudio2_ptr;
typedef std::unique_ptr<IXAudio2MasteringVoice, xaudio2_custom_deleter> mastering_voice_ptr;
typedef std::unique_ptr<IXAudio2SourceVoice, xaudio2_custom_deleter> src_voice_ptr;
typedef std::unique_ptr<osd_lock, xaudio2_custom_deleter> osd_lock_ptr;

// Typedef for pointer to XAudio2Create
typedef HRESULT(__stdcall* PFN_XAUDIO2CREATE)(IXAudio2**, UINT32, XAUDIO2_PROCESSOR);

//============================================================
//  Helper classes
//============================================================

// Helper for locking within a particular scope without having to manually release
class osd_scoped_lock
{
private:
    osd_lock *  m_lock;
public:
    osd_scoped_lock(osd_lock* lock)
    {
        m_lock = lock;
        osd_lock_acquire(m_lock);
    }

    ~osd_scoped_lock()
    {
        if (m_lock != nullptr)
        {
            osd_lock_release(m_lock);
        }
    }
};

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
    DWORD	                                    m_sample_bytes;
    std::unique_ptr<BYTE[]>                     m_buffer;
    DWORD                                       m_buffer_size;
    DWORD                                       m_writepos;
    osd_lock_ptr                                m_buffer_lock;
    HANDLE                                      m_hEventBufferCompleted;
    HANDLE                                      m_hEventDataAvailable;
    HANDLE                                      m_hEventExiting;
    std::thread                                 m_audioThread;
    std::queue<xaudio2_buffer>                  m_queue;
    std::unique_ptr<bufferpool>                 m_buffer_pool;
    HMODULE                                     m_xaudio2_module;
    PFN_XAUDIO2CREATE                           m_pfnxaudio2create;

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
        m_writepos(0),
        m_buffer_lock(osd_lock_alloc()),
        m_hEventBufferCompleted(NULL),
        m_hEventDataAvailable(NULL),
        m_hEventExiting(NULL),
        m_buffer_pool(nullptr),
        m_xaudio2_module(NULL)
    {
    }

    virtual int init(osd_options const &options) override;
    virtual void exit() override;

    // sound_module
    virtual void update_audio_stream(bool is_throttled, INT16 const *buffer, int samples_this_frame) override;
    virtual void set_mastervolume(int attenuation) override;

    // Xaudio callbacks
    void OnVoiceProcessingPassStart(UINT32 bytes_required) override {}
    void OnVoiceProcessingPassEnd() override {}
    void OnStreamEnd() override {}
    void OnBufferStart(void* pBufferContext) override {}
    void OnLoopEnd(void* pBufferContext) override {}
    void OnVoiceError(void* pBufferContext, HRESULT error) override {}
    void OnBufferEnd(void *pBufferContext) override;
    
private:
    HRESULT create_voices(const WAVEFORMATEX &format);
    void process_audio();
    void submit_buffer(std::unique_ptr<BYTE[]> audioData, DWORD audioLength);
    void submit_needed();
    HRESULT xaudio2_create(IXAudio2 ** xaudio2_interface);
};

//============================================================
//  init
//============================================================

int sound_xaudio2::init(osd_options const &options)
{
    HRESULT result = S_OK;

    HR_IGNORE(CoInitializeEx(NULL, COINITBASE_MULTITHREADED));

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

    // Compute the buffer size
    m_buffer_size = format.nSamplesPerSec * format.nBlockAlign * m_audio_latency / 10;
    m_buffer_size = MAX(1024, (m_buffer_size / 1024) * 1024);
    m_buffer_size = MIN(m_buffer_size, XAUDIO2_MAX_BUFFER_BYTES);

    // Create the buffer pool
    m_buffer_pool = std::make_unique<bufferpool>(INITIAL_BUFFER_COUNT, m_buffer_size);

    // get our initial buffer
    m_buffer = std::unique_ptr<BYTE[]>(m_buffer_pool->next());
    osd_printf_verbose("Sound: XAudio2 created initial buffer size: %u\n", (unsigned int)m_buffer_size);

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

    osd_scoped_lock scope_lock(m_buffer_lock.get());

    // Ensure this is going to fit in the current buffer
    if (m_writepos + bytes_this_frame > m_buffer_size)
    {
        // Queue the current buffer
        xaudio2_buffer buf;
        buf.AudioData = std::move(m_buffer);
        buf.AudioSize = m_writepos;
        m_queue.push(std::move(buf));

        // Get a new buffer
        m_buffer = std::unique_ptr<BYTE[]>(m_buffer_pool->next());
        m_writepos = 0;
    }

    // Copy in the data
    memcpy(m_buffer.get() + m_writepos, buffer, bytes_this_frame);
    m_writepos += bytes_this_frame;

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
        auto scoped_lock = osd_scoped_lock(m_buffer_lock.get());
        m_buffer_pool->return_to_pool(completed_buffer);
        completed_buffer = nullptr;
    }

    SetEvent(m_hEventBufferCompleted);
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
    m_sourceVoice->GetState(&state);

    if (state.BuffersQueued >= MIN_QUEUE_DEPTH)
        return;

    osd_scoped_lock lock_scope(m_buffer_lock.get());

    for (;;)
    {
        // Take buffers from the queue first
        if (!m_queue.empty())
        {
            // Get a reference to the buffer
            auto buf = &m_queue.front();

            // submit the buffer data
            submit_buffer(std::move(buf->AudioData), buf->AudioSize);

            // Remove it from the queue
            m_queue.pop();
        }
        else
        {
            // submit the main buffer
            submit_buffer(std::move(m_buffer), m_writepos);

            // Get a new buffer since this one is gone
            m_buffer = std::unique_ptr<BYTE[]>(m_buffer_pool->next());
            m_writepos = 0;

            // break out, this was the last buffer to submit
            break;
        }
    }
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

#else
MODULE_NOT_SUPPORTED(sound_xaudio2, OSD_SOUND_PROVIDER, "xaudio2")
#endif

MODULE_DEFINITION(SOUND_XAUDIO2, sound_xaudio2)