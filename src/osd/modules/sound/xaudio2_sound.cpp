// license:BSD-3-Clause
// copyright-holders:Vas Crabb, Brad Hughes
//====================================================================
//
//  xaudio2_sound.cpp - XAudio2 implementation of MAME sound routines
//
//====================================================================

#include "sound_module.h"

#include "modules/osdmodule.h"

#if defined(OSD_WINDOWS) | defined(SDLMAME_WIN32)

// local headers
#include "mmdevice_helpers.h"

// OSD headers
#include "modules/lib/osdlib.h"
#include "modules/lib/osdobj_common.h"
#include "osdcore.h"
#include "osdepend.h"
#include "strconv.h"
#include "windows/winutil.h"

// stdlib includes
#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <iterator>
#include <mutex>
#include <queue>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

// standard windows headers
#include <windows.h>

#include <combaseapi.h>
#include <objbase.h>

#include <wrl/client.h>

// XAudio2 include
#include <xaudio2.h>

// MMDevice API headers
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>


namespace osd {

namespace {

//============================================================
//  Constants
//============================================================

constexpr unsigned SUBMIT_FREQUENCY_TARGET_MS = 10;
constexpr float RESAMPLE_TOLERANCE = 1.20F;

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

// Macro to check for a failed HRESULT and if failed, goto a label called Error:
#define HR_GOERR( CALL ) HR_LOGE( CALL, goto Error;)

//============================================================
//  Structs and typedefs
//============================================================

// A structure to hold a pointer and the count of bytes of the data it points to
struct xaudio2_buffer
{
	std::unique_ptr<BYTE[]> AudioData;
	DWORD                   AudioSize;
};

// Custom deleter with overloads to free smart pointer types used in the implementations
struct xaudio2_custom_deleter
{
public:
	void operator()(IXAudio2MasteringVoice *obj) const
	{
		if (obj)
			obj->DestroyVoice();
	}

	void operator()(IXAudio2SourceVoice *obj) const
	{
		if (obj)
		{
			obj->Stop(0);
			obj->FlushSourceBuffers();
			obj->DestroyVoice();
		}
	}
};

// smart pointers used with customer deleters
using mastering_voice_ptr = std::unique_ptr<IXAudio2MasteringVoice, xaudio2_custom_deleter>;
using source_voice_ptr    = std::unique_ptr<IXAudio2SourceVoice, xaudio2_custom_deleter>;

// smart pointers for things using COM ABI
using x_audio_2_ptr       = Microsoft::WRL::ComPtr<IXAudio2>;

//============================================================
//  Helper classes
//============================================================

// Provides a pool of buffers
class bufferpool
{
private:
	int m_initial;
	int m_buffersize;
	std::queue<std::unique_ptr<BYTE []>> m_queue;

public:
	// constructor
	bufferpool(int capacity, int bufferSize) :
		m_initial(capacity),
		m_buffersize(bufferSize)
	{
		for (int i = 0; i < m_initial; i++)
		{
			auto newBuffer = std::make_unique<BYTE []>(m_buffersize);
			memset(newBuffer.get(), 0, m_buffersize);
			m_queue.push(std::move(newBuffer));
		}
	}

	// get next buffer element from the pool
	std::unique_ptr<BYTE []> next()
	{
		std::unique_ptr<BYTE []> next_buffer;
		if (!m_queue.empty())
		{
			next_buffer = std::move(m_queue.front());
			m_queue.pop();
		}
		else
		{
			next_buffer.reset(new BYTE[m_buffersize]);
			memset(next_buffer.get(), 0, m_buffersize);
		}

		return next_buffer;
	}

	// release element, make it available back in the pool
	void return_to_pool(std::unique_ptr<BYTE []> &&buffer)
	{
		assert(buffer);
		memset(buffer.get(), 0, m_buffersize);
		m_queue.push(std::move(buffer));
	}
};

//============================================================
//  sound_xaudio2 class
//============================================================

// The main class for the XAudio2 sound module implementation
class sound_xaudio2 :
		public osd_module,
		public sound_module,
		public IMMNotificationClient
{
public:
	sound_xaudio2() :
		osd_module(OSD_SOUND_PROVIDER, "xaudio2"),
		sound_module(),
		m_next_node_id(1),
		m_default_sink(0),
		m_endpoint_notifications(false),
		m_next_voice_id(1),
		m_generation(1),
		m_exiting(false),
		m_audio_latency(0.0F),
		m_hEventNeedUpdate(nullptr),
		m_hEventExiting(nullptr),
		m_overflows(0),
		m_underflows(0)
	{
	}

	// osd_module implementation
	virtual bool probe() override;
	virtual int init(osd_interface &osd, osd_options const &options) override;
	virtual void exit() override;

	// sound_module
	virtual uint32_t get_generation() override;
	virtual audio_info get_information() override;
	virtual bool external_per_channel_volume() override { return true; }
	virtual bool split_streams_per_source() override { return true; }
	virtual uint32_t stream_sink_open(uint32_t node, std::string name, uint32_t rate) override;
	virtual void stream_set_volumes(uint32_t id, const std::vector<float> &db) override;
	virtual void stream_close(uint32_t id) override;
	virtual void stream_sink_update(uint32_t id, int16_t const *buffer, int samples_this_frame) override;

private:
	class device_info final : public IXAudio2EngineCallback
	{
	public:
		device_info(sound_xaudio2 &host) : m_host(host) { }
		device_info(device_info const &) = delete;
		device_info(device_info &&) = delete;

		std::wstring            device_id;
		mm_device_ptr           device;
		x_audio_2_ptr           engine;
		mastering_voice_ptr     mastering_voice;
		audio_info::node_info   info;

	private:
		virtual void STDMETHODCALLTYPE OnProcessingPassStart() override { }
		virtual void STDMETHODCALLTYPE OnProcessingPassEnd() override { }
		virtual void STDMETHODCALLTYPE OnCriticalError(HRESULT error) override;

		sound_xaudio2 &         m_host;
	};

	class voice_info final : public IXAudio2VoiceCallback
	{
	public:
		voice_info(sound_xaudio2 &h, WAVEFORMATEX const &format);
		voice_info(voice_info const &) = delete;
		voice_info(voice_info &&) = delete;
		~voice_info();

		void update(int16_t const *buffer, int samples_this_frame);
		void submit_if_needed();

		source_voice_ptr            voice;
		audio_info::stream_info     info;
		std::unique_ptr<float []>   volume_matrix;

	private:
		void roll_buffer();
		void submit_buffer(std::unique_ptr<BYTE []> &&data, DWORD length) const;

		virtual void STDMETHODCALLTYPE OnVoiceProcessingPassStart(UINT32 BytesRequired) noexcept override;
		virtual void STDMETHODCALLTYPE OnVoiceProcessingPassEnd() noexcept override { }
		virtual void STDMETHODCALLTYPE OnStreamEnd() noexcept override { }
		virtual void STDMETHODCALLTYPE OnBufferStart(void* pBufferContext) noexcept override { }
		virtual void STDMETHODCALLTYPE OnBufferEnd(void *pBufferContext) noexcept override;
		virtual void STDMETHODCALLTYPE OnLoopEnd(void *pBufferContext) noexcept override { }
		virtual void STDMETHODCALLTYPE OnVoiceError(void *pBufferContext, HRESULT error) noexcept override;

		sound_xaudio2 &             m_host;
		std::unique_ptr<bufferpool> m_buffer_pool;
		std::unique_ptr<BYTE []>    m_current_buffer;
		std::queue<xaudio2_buffer>  m_buffer_queue;
		uint32_t                    m_buffer_size;
		unsigned const              m_sample_bytes;
		unsigned                    m_buffer_count;
		uint32_t                    m_write_position;
		bool                        m_need_update;
		bool                        m_underflowing;
	};

	using device_info_ptr = std::unique_ptr<device_info>;
	using device_info_vector = std::vector<device_info_ptr>;
	using device_info_vector_iterator = device_info_vector::iterator;
	using voice_info_ptr = std::unique_ptr<voice_info>;
	using voice_info_vector = std::vector<voice_info_ptr>;

	// IMMNotificationClient callbacks
	virtual HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId) override;
	virtual HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId) override;
	virtual HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId) override;
	virtual HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState) override;
	virtual HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDefaultDeviceId, PROPERTYKEY const key) override;

	// stub IUnknown implementation
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) override;
	virtual ULONG STDMETHODCALLTYPE AddRef() override { return 1; }
	virtual ULONG STDMETHODCALLTYPE Release() override { return 1; }

	device_info_vector_iterator find_device(std::wstring_view device_id);
	HRESULT add_device(device_info_vector_iterator pos, LPCWSTR device_id);
	HRESULT add_device(device_info_vector_iterator pos, std::wstring_view device_id, mm_device_ptr &&device);
	HRESULT remove_device(device_info_vector_iterator pos);

	void audio_task();
	void cleanup_task();

	// tracking audio endpoint devices
	mm_device_enumerator_ptr    m_device_enum;
	device_info_vector          m_device_info;
	std::mutex                  m_device_mutex;
	uint32_t                    m_next_node_id;
	uint32_t                    m_default_sink;
	bool                        m_endpoint_notifications;

	// managing cleanup
	device_info_vector          m_zombie_devices;
	voice_info_vector           m_zombie_voices;
	std::condition_variable     m_cleanup_condition;
	std::thread                 m_cleanup_thread;
	std::mutex                  m_cleanup_mutex;

	voice_info_vector           m_voice_info;
	std::mutex                  m_voice_mutex;
	uint32_t                    m_next_voice_id;

	uint32_t                    m_generation;
	bool                        m_exiting;

	float                       m_audio_latency;
	HANDLE                      m_hEventNeedUpdate;
	HANDLE                      m_hEventExiting;
	std::thread                 m_audioThread;
	std::atomic<uint32_t>       m_overflows;
	std::atomic<uint32_t>       m_underflows;

	OSD_DYNAMIC_API(xaudio2, "XAudio2_9.dll", "XAudio2_8.dll");
	OSD_DYNAMIC_API_FN(xaudio2, HRESULT, WINAPI, XAudio2Create, IXAudio2 **, uint32_t, XAUDIO2_PROCESSOR);
};

//============================================================
//  probe
//============================================================

bool sound_xaudio2::probe()
{
	return OSD_DYNAMIC_API_TEST(XAudio2Create);
}

//============================================================
//  init
//============================================================

int sound_xaudio2::init(osd_interface &osd, osd_options const &options)
{
	HRESULT result;

	// make sure our XAudio2Create entry point is bound
	if (!OSD_DYNAMIC_API_TEST(XAudio2Create))
	{
		osd_printf_error("Could not find XAudio2. Please try to reinstall DirectX runtime package.\n");
		return 1;
	}

	// get relevant options
	m_audio_latency = options.audio_latency();
	if (m_audio_latency == 0.0F)
		m_audio_latency = 0.03F;

	// create a multimedia device enumerator and enumerate devices
	HR_GOERR(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&m_device_enum)));
	{
		co_task_wstr_ptr default_id;
		std::lock_guard device_lock(m_device_mutex);

		HR_GOERR(m_device_enum->RegisterEndpointNotificationCallback(this));
		m_endpoint_notifications = true;

		{
			mm_device_ptr default_device;
			LPWSTR id_raw = nullptr;
			HR_GOERR(m_device_enum->GetDefaultAudioEndpoint(eRender, eMultimedia, default_device.GetAddressOf()));
			HR_GOERR(default_device->GetId(&id_raw));
			default_id.reset(std::exchange(id_raw, nullptr));
		}

		result = enumerate_audio_endpoints(
				*m_device_enum.Get(),
				eAll,
				DEVICE_STATE_ACTIVE | DEVICE_STATE_UNPLUGGED,
				[this, &default_id] (HRESULT hr, mm_device_ptr &dev) -> bool
				{
					if (FAILED(hr) || !dev)
					{
						osd_printf_error("Error getting audio device. Error: 0x%X\n", static_cast<unsigned int>(hr));
						return true;
					}

					// skip devices that are disabled or not present
					DWORD state;
					hr = dev->GetState(&state);
					if (FAILED(hr))
					{
						osd_printf_error("Error getting audio device state. Error: 0x%X\n", static_cast<unsigned int>(hr));
						return true;
					}
					if ((DEVICE_STATE_ACTIVE != state) && (DEVICE_STATE_UNPLUGGED != state))
						return true;

					// populate node info structure
					std::wstring device_id;
					audio_info::node_info info;
					hr = populate_audio_node_info(*dev.Get(), device_id, info);
					if (FAILED(hr))
						return true;

					// skip devices that have no outputs
					if (0 >= info.m_sinks)
						return true;

					// add a device ID mapping
					auto const pos = find_device(device_id);
					m_zombie_devices.reserve(m_zombie_devices.size() + m_device_info.size() + 1);
					device_info_ptr devinfo = std::make_unique<device_info>(*this);
					info.m_id = m_next_node_id++;
					devinfo->device_id = std::move(device_id);
					devinfo->device = std::move(dev);
					devinfo->info = std::move(info);
					if (!m_default_sink && default_id && (devinfo->device_id == default_id.get()))
						m_default_sink = devinfo->info.m_id;
					m_device_info.emplace(pos, std::move(devinfo));

					return true;
				});
		if (!m_default_sink && !m_device_info.empty())
			m_default_sink = m_device_info.front()->info.m_id;
	}
	if (FAILED(result))
	{
		osd_printf_error("Error enumerating audio endpoints. Error: 0x%X\n", static_cast<unsigned int>(result));
		goto Error;
	}

	// start a thread to clean up removed devices
	m_cleanup_thread = std::thread([] (sound_xaudio2 *self) { self->cleanup_task(); }, this);

	// Initialize our events
	m_hEventNeedUpdate = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	m_hEventExiting = CreateEvent(nullptr, FALSE, FALSE, nullptr);

	// Start the thread listening
	m_audioThread = std::thread([] (sound_xaudio2 *self) { self->audio_task(); }, this);

	return 0;

Error:
	this->exit();
	return 1;
}

//============================================================
//  exit
//============================================================

void sound_xaudio2::exit()
{
	if (m_endpoint_notifications)
	{
		m_device_enum->UnregisterEndpointNotificationCallback(this);
	}

	// Wait on processing thread to end
	if (m_hEventExiting)
		SetEvent(m_hEventExiting);

	if (m_audioThread.joinable())
		m_audioThread.join();

	if (m_hEventNeedUpdate)
	{
		CloseHandle(m_hEventNeedUpdate);
		m_hEventNeedUpdate = nullptr;
	}

	if (m_hEventExiting)
	{
		CloseHandle(m_hEventExiting);
		m_hEventExiting = nullptr;
	}

	{
		std::lock_guard voice_lock(m_voice_mutex);
		m_voice_info.clear();
	}

	if (m_cleanup_thread.joinable())
	{
		{
			std::lock_guard cleanup_lock(m_cleanup_mutex);
			m_exiting = true;
			m_cleanup_condition.notify_all();
		}
		m_cleanup_thread.join();
	}

	{
		std::lock_guard device_lock(m_device_mutex);
		m_device_info.clear();
	}

	m_device_enum = nullptr;

	if (m_overflows != 0 || m_underflows != 0)
		osd_printf_verbose("Sound: overflows=%u, underflows=%u\n", m_overflows, m_underflows);

	osd_printf_verbose("Sound: XAudio2 deinitialized\n");
}

//============================================================
//  get_generation
//============================================================

uint32_t sound_xaudio2::get_generation()
{
	uint32_t result;
	{
		std::lock_guard device_lock(m_device_mutex);
		result = m_generation;
	}
	return result;
}

//============================================================
//  get_information
//============================================================

audio_info sound_xaudio2::get_information()
{
	audio_info result;
	{
		std::lock_guard device_lock(m_device_mutex);

		result.m_generation = m_generation;
		result.m_default_sink = m_default_sink;
		result.m_default_source = 0;

		result.m_nodes.reserve(m_device_info.size());
		for (auto const &device : m_device_info)
			result.m_nodes.emplace_back(device->info);

		std::lock_guard voice_lock(m_voice_mutex);
		result.m_streams.reserve(m_voice_info.size());
		for (auto const &voice : m_voice_info)
			result.m_streams.emplace_back(voice->info);
	}
	return result;
}

//============================================================
//  stream_sink_open
//============================================================

uint32_t sound_xaudio2::stream_sink_open(uint32_t node, std::string name, uint32_t rate)
{
	std::lock_guard device_lock(m_device_mutex);
	HRESULT result;

	// find the requested device
	auto const device = std::find_if(
			m_device_info.begin(),
			m_device_info.end(),
			[node] (device_info_ptr const &value) { return value->info.m_id == node; });
	if (m_device_info.end() == device)
	{
		osd_printf_error("Attempt to open audio stream %s for unknown node %u.\n", name, node);
		return 0;
	}

	// instantiate XAudio2 engine if necessary
	if (!(*device)->engine)
	{
		result = OSD_DYNAMIC_CALL(XAudio2Create, &(*device)->engine, 0, XAUDIO2_DEFAULT_PROCESSOR);
		if (FAILED(result) || !(*device)->engine)
		{
			(*device)->engine = nullptr;
			osd_printf_error(
					"Error creating XAudio2 engine for audio device %s. Error: 0x%X\n",
					(*device)->info.m_name,
					static_cast<unsigned int>(result));
			return 0;
		}

		result = (*device)->engine->RegisterForCallbacks(device->get());
		if (FAILED(result))
		{
			(*device)->engine = nullptr;
			osd_printf_error(
					"Error registering to receive XAudio2 engine callbacks for audio device %s. Error: 0x%X\n",
					(*device)->info.m_name,
					static_cast<unsigned int>(result));
			return 0;
		}
	}

	// create a mastering voice if we don't already have one for this device
	if (!(*device)->mastering_voice)
	{
		IXAudio2MasteringVoice *mastering_voice_raw = nullptr;
		result = (*device)->engine->CreateMasteringVoice(
				&mastering_voice_raw,
				(*device)->info.m_sinks,
				(*device)->info.m_rate.m_default_rate,
				0,
				(*device)->device_id.c_str(),
				nullptr,
				AudioCategory_Other);
		(*device)->mastering_voice.reset(std::exchange(mastering_voice_raw, nullptr));
		if (FAILED(result) || !(*device)->mastering_voice)
		{
			(*device)->mastering_voice.reset();
			osd_printf_error(
					"Error creating mastering voice for audio device %s. Error: 0x%X\n",
					(*device)->info.m_name,
					static_cast<unsigned int>(result));
			return 0;
		}
	}

	voice_info_ptr info;
	WAVEFORMATEX format;
	XAUDIO2_SEND_DESCRIPTOR destination;
	XAUDIO2_VOICE_SENDS sends;

	try
	{
		// set up desired input format and destination
		format.wFormatTag = WAVE_FORMAT_PCM;
		format.nChannels = (*device)->info.m_sinks;
		format.nSamplesPerSec = rate;
		format.nAvgBytesPerSec = 2 * format.nChannels * rate;
		format.nBlockAlign = 2 * format.nChannels;
		format.wBitsPerSample = 16;
		format.cbSize = 0;
		destination.Flags = 0;
		destination.pOutputVoice = (*device)->mastering_voice.get();
		sends.SendCount = 1;
		sends.pSends = &destination;

		// create the voice info object
		info = std::make_unique<voice_info>(*this, format);
		info->info.m_node = node;
		info->info.m_volumes.resize((*device)->info.m_sinks, 0.0F);
	}
	catch (std::bad_alloc const &)
	{
		return 0;
	}

	// create a source voice for this stream
	IXAudio2SourceVoice *source_voice_raw = nullptr;
	result = (*device)->engine->CreateSourceVoice(
			&source_voice_raw,
			&format,
			XAUDIO2_VOICE_NOPITCH,
			1.0F,
			info.get(),
			&sends,
			nullptr);
	info->voice.reset(std::exchange(source_voice_raw, nullptr));
	if (FAILED(result) || !info->voice)
	{
		osd_printf_error(
				"Error creating source voice for audio device %s. Error: 0x%X\n",
				(*device)->info.m_name,
				static_cast<unsigned int>(result));
		return 0;
	}

	// set the channel mapping
	result = info->voice->SetOutputMatrix(
			nullptr,
			format.nChannels,
			format.nChannels,
			info->volume_matrix.get(),
			XAUDIO2_COMMIT_NOW);
	if (FAILED(result))
	{
		osd_printf_error(
				"Error setting source voice output matrix for audio device %s. Error: 0x%X\n",
				(*device)->info.m_name,
				static_cast<unsigned int>(result));
		return 0;
	}

	// start the voice
	result = info->voice->Start();
	if (FAILED(result))
	{
		osd_printf_error(
				"Error starting source voice for audio device %s. Error: 0x%X\n",
				(*device)->info.m_name,
				static_cast<unsigned int>(result));
		return 0;
	}

	try
	{
		std::lock_guard voice_lock(m_voice_mutex);
		{
			std::lock_guard cleanup_lock(m_cleanup_mutex);
			m_zombie_voices.reserve(m_zombie_voices.size() + m_voice_info.size() + 1);
		}
		m_voice_info.reserve(m_voice_info.size() + 1);
		info->info.m_id = m_next_voice_id++;
		assert(m_voice_info.empty() || (m_voice_info.back()->info.m_id < info->info.m_id));
		auto &pos = m_voice_info.emplace_back(std::move(info));
		return pos->info.m_id;
	}
	catch (std::bad_alloc const &)
	{
		return 0;
	}
}

//============================================================
//  stream_set_volumes
//============================================================

void sound_xaudio2::stream_set_volumes(uint32_t id, const std::vector<float> &db)
{
	std::lock_guard voice_lock(m_voice_mutex);
	auto const pos = std::lower_bound(
			m_voice_info.begin(),
			m_voice_info.end(),
			id,
			[] (voice_info_ptr const &a, uint32_t b) { return a->info.m_id < b; });
	if ((m_voice_info.end() == pos) || ((*pos)->info.m_id != id))
		return;

	auto const bound = std::min((*pos)->info.m_volumes.size(), db.size());
	for (unsigned i = 0; bound > i; ++i)
	{
		(*pos)->info.m_volumes[i] = db[i];
		(*pos)->volume_matrix[((*pos)->info.m_volumes.size() * i) + i] = db_to_linear(db[i]);
	}

	HRESULT const result = (*pos)->voice->SetOutputMatrix(
			nullptr,
			(*pos)->info.m_volumes.size(),
			(*pos)->info.m_volumes.size(),
			(*pos)->volume_matrix.get(),
			XAUDIO2_COMMIT_NOW);
	if (FAILED(result))
	{
		osd_printf_error(
				"Error setting source voice output matrix for audio stream %u. Error: 0x%X\n",
				(*pos)->info.m_id,
				static_cast<unsigned int>(result));
	}
}

//============================================================
//  stream_close
//============================================================

void sound_xaudio2::stream_close(uint32_t id)
{
	std::lock_guard voice_lock(m_voice_mutex);
	auto const pos = std::lower_bound(
			m_voice_info.begin(),
			m_voice_info.end(),
			id,
			[] (voice_info_ptr const &a, uint32_t b) { return a->info.m_id < b; });
	if ((m_voice_info.end() == pos) || ((*pos)->info.m_id != id))
	{
		// the sound manager tries to close streams that have disappeared due to device disconnection
		return;
	}

	m_voice_info.erase(pos);
}

//============================================================
//  stream_sink_update
//============================================================

void sound_xaudio2::stream_sink_update(
		uint32_t id,
		int16_t const *buffer,
		int samples_this_frame)
{
	std::lock_guard voice_lock(m_voice_mutex);
	auto const pos = std::lower_bound(
			m_voice_info.begin(),
			m_voice_info.end(),
			id,
			[] (voice_info_ptr const &a, uint32_t b) { return a->info.m_id < b; });
	if ((m_voice_info.end() == pos) || ((*pos)->info.m_id != id))
		return;

	(*pos)->update(buffer, samples_this_frame);
}

//============================================================
//  IXAudio2EngineCallback::OnCriticalError
//============================================================

void sound_xaudio2::device_info::OnCriticalError(HRESULT error)
{
	std::lock_guard device_lock(m_host.m_device_mutex);

	osd_printf_verbose(
			"XAudio2 critical error for device %s. Error: 0x%X\n",
			info.m_name,
			static_cast<unsigned int>(error));

	auto const pos = std::find_if(
			m_host.m_device_info.begin(),
			m_host.m_device_info.end(),
			[this] (device_info_ptr const &value) { return value.get() == this; });
	assert(m_host.m_device_info.end() != pos);

	m_host.remove_device(pos);
}

//============================================================
//  voice_info
//============================================================

sound_xaudio2::voice_info::voice_info(sound_xaudio2 &host, WAVEFORMATEX const &format) :
	volume_matrix(std::make_unique<float []>(format.nChannels * format.nChannels)),
	m_host(host),
	m_sample_bytes(format.nBlockAlign),
	m_write_position(0),
	m_need_update(false),
	m_underflowing(false)
{
	// set default volume matrix
	for (unsigned i = 0; format.nChannels > i; ++i)
	{
		for (unsigned j = 0; format.nChannels > j; ++j)
			volume_matrix[(format.nChannels * i) + j] = (i == j) ? 1.0F : 0.0F;
	}

	// calculate required buffer size
	int const audio_latency_ms = std::max(unsigned((m_host.m_audio_latency * 1000.0F) + 0.5F), SUBMIT_FREQUENCY_TARGET_MS);
	uint32_t const buffer_total = m_sample_bytes * format.nSamplesPerSec * (audio_latency_ms / 1000.0F) * RESAMPLE_TOLERANCE;
	m_buffer_count = audio_latency_ms / SUBMIT_FREQUENCY_TARGET_MS;
	m_buffer_size = std::max<uint32_t>(1024, buffer_total / m_buffer_count);

	// force to a whole number of samples
	uint32_t const remainder = m_buffer_size % m_sample_bytes;
	if (remainder)
		m_buffer_size += m_sample_bytes - remainder;

	// allocate the initial buffers
	m_buffer_pool = std::make_unique<bufferpool>(m_buffer_count, m_buffer_size);
	m_current_buffer = m_buffer_pool->next();
}

//============================================================
//  ~voice_info
//============================================================

sound_xaudio2::voice_info::~voice_info()
{
	// stop this before any buffers get destructed
	voice.reset();
}

//============================================================
//  update
//============================================================

void sound_xaudio2::voice_info::update(int16_t const *buffer, int samples_this_frame)
{
	uint32_t const bytes_this_frame = samples_this_frame * m_sample_bytes;
	uint32_t bytes_left = bytes_this_frame;
	while (bytes_left)
	{
		uint32_t const chunk = std::min(uint32_t(m_buffer_size), bytes_left);

		// roll buffer if the chunk won't fit
		if ((m_write_position + chunk) >= m_buffer_size)
			roll_buffer();

		// copy sample data
		memcpy(&m_current_buffer[m_write_position], buffer, chunk);
		m_write_position += chunk;
		buffer += chunk / 2;
		bytes_left -= chunk;
	}

	m_need_update = true;
	SetEvent(m_host.m_hEventNeedUpdate);
}

//============================================================
//  submit_if_needed
//============================================================

void sound_xaudio2::voice_info::submit_if_needed()
{
	if (!m_need_update)
		return;

	m_need_update = false;

	XAUDIO2_VOICE_STATE state;
	voice->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);

	// If we have buffers queued into XAudio and our current in-memory buffer
	// isn't yet full, there's no need to submit it
	if ((1 <= state.BuffersQueued) && m_buffer_queue.empty())
		return;

	// We do however want to achieve some kind of minimal latency, so if the queued buffers
	// are greater than 2, flush them to re-sync the audio
	if (2 < state.BuffersQueued)
	{
		voice->FlushSourceBuffers();
		m_host.m_overflows.fetch_add(1, std::memory_order_relaxed);
	}

	if (m_buffer_queue.empty())
		roll_buffer();

	if (!m_buffer_queue.empty())
	{
		auto buffers = state.BuffersQueued;
		do
		{
			auto &buf = m_buffer_queue.front();
			assert(0 < buf.AudioSize);
			submit_buffer(std::move(buf.AudioData), buf.AudioSize);
			m_buffer_queue.pop();
			buffers++;
		}
		while (!m_buffer_queue.empty() && (2 > buffers));
	}
}

//============================================================
//  roll_buffer
//============================================================

void sound_xaudio2::voice_info::roll_buffer()
{
	// don't queue an empty buffer
	if (!m_write_position)
		return;

	// queue the current buffer
	xaudio2_buffer buf;
	buf.AudioData = std::move(m_current_buffer);
	buf.AudioSize = m_write_position;
	m_buffer_queue.push(std::move(buf));

	// get a fresh buffer
	m_current_buffer = m_buffer_pool->next();
	m_write_position = 0;

	// remove excess buffers from queue
	if (m_buffer_queue.size() > m_buffer_count)
	{
		m_host.m_overflows.fetch_add(1, std::memory_order_relaxed);
		while (m_buffer_queue.size() > m_buffer_count)
		{
			// return the oldest buffer to the pool, and remove it from queue
			m_buffer_pool->return_to_pool(std::move(m_buffer_queue.front().AudioData));
			m_buffer_queue.pop();
		}
	}
}

//============================================================
//  submit_buffer
//============================================================

void sound_xaudio2::voice_info::submit_buffer(std::unique_ptr<BYTE []> &&data, DWORD length) const
{
	assert(length);

	XAUDIO2_BUFFER buf = { 0 };
	buf.AudioBytes = length;
	buf.pAudioData = data.get();
	buf.PlayBegin = 0;
	buf.PlayLength = length / m_sample_bytes;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	buf.pContext = data.get();

	HRESULT result;
	if (FAILED(result = voice->SubmitSourceBuffer(&buf)))
	{
		osd_printf_verbose("Sound: XAudio2 failed to submit source buffer (non-fatal). Error: 0x%X\n", static_cast<unsigned int>(result));
		m_buffer_pool->return_to_pool(std::move(data));
		return;
	}

	// If we succeeded, relinquish the buffer allocation to the XAudio2 runtime
	// The buffer will be freed on the OnBufferCompleted callback
	// FIXME: does this leak when the voice is destroyed?
	data.release();
}

//============================================================
//  IXAudio2VoiceCallback::OnVoiceProcessingPassStart
//============================================================

void sound_xaudio2::voice_info::OnVoiceProcessingPassStart(UINT32 BytesRequired) noexcept
{
	if (!BytesRequired)
	{
		m_underflowing = false;
	}
	else if (!m_underflowing)
	{
		m_underflowing = true;
		m_host.m_underflows.fetch_add(1, std::memory_order_relaxed);
	}
}

//============================================================
//  IXAudio2VoiceCallback::OnBufferEnd
//============================================================

void sound_xaudio2::voice_info::OnBufferEnd(void *pBufferContext) noexcept
{
	BYTE *const completed_buffer = static_cast<BYTE *>(pBufferContext);
	std::lock_guard voice_lock(m_host.m_voice_mutex);

	if (completed_buffer)
		m_buffer_pool->return_to_pool(std::unique_ptr<BYTE []>(completed_buffer));

	m_need_update = true;
	SetEvent(m_host.m_hEventNeedUpdate);
}

//============================================================
//  IXAudio2VoiceCallback::OnVoiceError
//============================================================

void sound_xaudio2::voice_info::OnVoiceError(void *pBufferContext, HRESULT error) noexcept
{
	voice_info_ptr our_pointer;
	{
		BYTE *const completed_buffer = static_cast<BYTE *>(pBufferContext);
		std::lock_guard device_lock(m_host.m_device_mutex);
		std::lock_guard voice_lock(m_host.m_voice_mutex);

		osd_printf_verbose(
				"XAudio2 voice error for stream %u. Error: 0x%X\n",
				info.m_id,
				static_cast<unsigned int>(error));

		if (completed_buffer)
			m_buffer_pool->return_to_pool(std::unique_ptr<BYTE []>(completed_buffer));

		auto const pos = std::find_if(
				m_host.m_voice_info.begin(),
				m_host.m_voice_info.end(),
				[this] (voice_info_ptr const &value) { return value.get() == this; });
		assert(m_host.m_voice_info.end() != pos);

		our_pointer = std::move(*pos);
		m_host.m_voice_info.erase(pos);

		++m_host.m_generation;
	}

	// flag ourselves for cleanup
	std::lock_guard cleanup_lock(m_host.m_cleanup_mutex);
	m_host.m_zombie_voices.emplace_back(std::move(our_pointer));
	m_host.m_cleanup_condition.notify_one();
}

//============================================================
//  IMMNotificationClient::OnDefaultDeviceChanged
//============================================================

HRESULT sound_xaudio2::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId)
{
	try
	{
		if ((eRender == flow) && (eMultimedia == role))
		{
			std::lock_guard device_lock(m_device_mutex);
			std::wstring_view device_id(pwstrDefaultDeviceId);
			auto const pos = find_device(device_id);
			if ((m_device_info.end() != pos) && ((*pos)->device_id == device_id) && ((*pos)->info.m_id != m_default_sink))
			{
				m_default_sink = (*pos)->info.m_id;

				++m_generation;
			}
		}
		return S_OK;
	}
	catch (std::bad_alloc const &)
	{
		return E_OUTOFMEMORY;
	}
}

//============================================================
//  IMMNotificationClient::OnDeviceAdded
//============================================================

HRESULT sound_xaudio2::OnDeviceAdded(LPCWSTR pwstrDeviceId)
{
	try
	{
		std::lock_guard device_lock(m_device_mutex);
		std::wstring_view device_id(pwstrDeviceId);
		HRESULT result;
		auto const pos = find_device(device_id);

		// if the device is already there, something went wrong
		if ((m_device_info.end() != pos) && ((*pos)->device_id == device_id))
		{
			osd_printf_error(
					"Added sound device %s appears to already be present.\n",
					osd::text::from_wstring(device_id));
			return S_OK;
		}

		// add it if it's an available output device
		result = add_device(pos, pwstrDeviceId);
		if (FAILED(result))
			return result;

		return S_OK;
	}
	catch (std::bad_alloc const &)
	{
		return E_OUTOFMEMORY;
	}
}

//============================================================
//  IMMNotificationClient::OnDeviceRemoved
//============================================================

HRESULT sound_xaudio2::OnDeviceRemoved(LPCWSTR pwstrDeviceId)
{
	try
	{
		std::lock_guard device_lock(m_device_mutex);
		std::wstring_view device_id(pwstrDeviceId);
		auto const pos = find_device(device_id);
		if ((m_device_info.end() != pos) && ((*pos)->device_id == device_id))
		{
			HRESULT const result = remove_device(pos);
			if (FAILED(result))
				return result;
		}

		return S_OK;
	}
	catch (std::bad_alloc const &)
	{
		return E_OUTOFMEMORY;
	}
}

//============================================================
//  IMMNotificationClient::OnDeviceStateChanged
//============================================================

HRESULT sound_xaudio2::OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
{
	try
	{
		std::lock_guard device_lock(m_device_mutex);
		std::wstring_view device_id(pwstrDeviceId);
		auto const pos = find_device(device_id);

		if ((DEVICE_STATE_ACTIVE == dwNewState) || (DEVICE_STATE_UNPLUGGED == dwNewState))
		{
			if ((m_device_info.end() == pos) || ((*pos)->device_id != device_id))
			{
				HRESULT result;

				mm_device_ptr device;
				result = m_device_enum->GetDevice(pwstrDeviceId, device.GetAddressOf());
				if (FAILED(result))
				{
					osd_printf_error(
							"Error getting audio device %s. Error: 0x%X\n",
							osd::text::from_wstring(device_id),
							static_cast<unsigned int>(result));
					return result;
				}

				result = add_device(pos, device_id, std::move(device));
				if (FAILED(result))
					return result;
			}
		}
		else
		{
			if ((m_device_info.end() != pos) && ((*pos)->device_id == device_id))
			{
				HRESULT const result = remove_device(pos);
				if (FAILED(result))
					return result;
			}
		}
		return S_OK;
	}
	catch (std::bad_alloc const &)
	{
		return E_OUTOFMEMORY;
	}
}

//============================================================
//  IMMNotificationClient::OnPropertyValueChanged
//============================================================

HRESULT sound_xaudio2::OnPropertyValueChanged(LPCWSTR pwstrDeviceId, PROPERTYKEY const key)
{
	try
	{
		std::lock_guard device_lock(m_device_mutex);
		std::wstring_view device_id(pwstrDeviceId);
		auto const pos = find_device(device_id);

		if (PKEY_Device_FriendlyName == key)
		{
			if ((m_device_info.end() != pos) && ((*pos)->device_id == device_id))
			{
				HRESULT result;

				property_store_ptr properties;
				result = (*pos)->device->OpenPropertyStore(STGM_READ, properties.GetAddressOf());
				if (FAILED(result))
				{
					osd_printf_error(
							"Error opening property store for audio device %s. Error: 0x%X\n",
							(*pos)->info.m_name,
							static_cast<unsigned int>(result));
					return result;
				}

				std::optional<std::string> name;
				result = get_string_property_value(*properties.Get(), PKEY_Device_FriendlyName, name);
				if (FAILED(result))
				{
					osd_printf_error(
							"Error getting updated display name for audio device %s. Error: 0x%X\n",
							(*pos)->info.m_name,
							static_cast<unsigned int>(result));
					return result;
				}

				if (name)
				{
					(*pos)->info.m_name = std::move(*name);

					++m_generation;
				}
			}
		}
		else if (PKEY_AudioEngine_DeviceFormat == key)
		{
			if ((m_device_info.end() != pos) && ((*pos)->device_id == device_id))
			{
				// FIXME: update format
			}
			else
			{
				// deal with empty prop
				HRESULT const result = add_device(pos, pwstrDeviceId);
				if (FAILED(result))
					return result;
			}
		}

		return S_OK;
	}
	catch (std::bad_alloc const &)
	{
		return E_OUTOFMEMORY;
	}
}

//============================================================
//  IUnknown::QueryInterface
//============================================================

HRESULT sound_xaudio2::QueryInterface(REFIID riid, void **ppvObject)
{
	if (!ppvObject)
		return E_POINTER;
	else if (__uuidof(IUnknown) == riid)
		*ppvObject = static_cast<IUnknown *>(this);
	else if (__uuidof(IMMNotificationClient) == riid)
		*ppvObject = static_cast<IMMNotificationClient *>(this);
	else
		*ppvObject = nullptr;

	return *ppvObject ? S_OK : E_NOINTERFACE;
}


inline sound_xaudio2::device_info_vector_iterator sound_xaudio2::find_device(std::wstring_view device_id)
{
	return std::lower_bound(
			m_device_info.begin(),
			m_device_info.end(),
			device_id,
			[] (device_info_ptr const &a, std::wstring_view const &b) { return a->device_id < b; });
}

HRESULT sound_xaudio2::add_device(device_info_vector_iterator pos, LPCWSTR device_id)
{
	HRESULT result;

	// get the device
	mm_device_ptr device;
	result = m_device_enum->GetDevice(device_id, device.GetAddressOf());
	if (FAILED(result))
	{
		osd_printf_error(
				"Error getting audio device %s. Error: 0x%X\n",
				osd::text::from_wstring(device_id),
				static_cast<unsigned int>(result));
		return result;
	}

	// ignore if it's disabled or not present
	DWORD state;
	result = device->GetState(&state);
	if (FAILED(result))
	{
		osd_printf_error(
				"Error getting audio device %s state. Error: 0x%X\n",
				osd::text::from_wstring(device_id),
				static_cast<unsigned int>(result));
		return result;
	}
	if ((DEVICE_STATE_ACTIVE != state) && (DEVICE_STATE_UNPLUGGED != state))
		return S_OK;

	// add it if it's an output device
	return add_device(pos, device_id, std::move(device));
}

HRESULT sound_xaudio2::add_device(device_info_vector_iterator pos, std::wstring_view device_id, mm_device_ptr &&device)
{
	std::wstring device_id_str;
	audio_info::node_info info;
	HRESULT const result = populate_audio_node_info(*device.Get(), device_id_str, info);
	if (FAILED(result))
		return result;
	assert(device_id == device_id_str);

	if (0 < info.m_sinks)
	{
		try
		{
			{
				std::lock_guard cleanup_lock(m_cleanup_mutex);
				m_zombie_devices.reserve(m_zombie_devices.size() + m_device_info.size() + 1);
			}
			device_info_ptr devinfo = std::make_unique<device_info>(*this);
			info.m_id = m_next_node_id++;
			devinfo->device_id = std::move(device_id_str);
			devinfo->device = std::move(device);
			devinfo->info = std::move(info);
			m_device_info.emplace(pos, std::move(devinfo));

			++m_generation;
		}
		catch (std::bad_alloc const &)
		{
			return E_OUTOFMEMORY;
		}
	}

	return S_OK;
}

HRESULT sound_xaudio2::remove_device(device_info_vector_iterator pos)
{
	// if this was the default device, choose a new default arbitrarily
	if ((*pos)->info.m_id == m_default_sink)
	{
		if (m_device_info.begin() != pos)
		{
			m_default_sink = m_device_info.front()->info.m_id;
		}
		else
		{
			auto const next = std::next(pos);
			if (m_device_info.end() != next)
				m_default_sink = (*next)->info.m_id;
			else
				m_default_sink = 0;
		}
	}

	// clean up any voices associated with this device
	{
		std::lock_guard voice_lock(m_voice_mutex);
		auto it = m_voice_info.begin();
		while (m_voice_info.end() != it)
		{
			if ((*it)->info.m_node == (*pos)->info.m_id)
			{
				voice_info_ptr voice = std::move(*it);
				it = m_voice_info.erase(it);
				{
					std::lock_guard cleanup_lock(m_cleanup_mutex);
					m_zombie_voices.emplace_back(std::move(voice));
				}
			}
			else
			{
				++it;
			}
		}
	}

	// flag the device for cleanup
	{
		std::lock_guard cleanup_lock(m_cleanup_mutex);
		m_zombie_devices.emplace_back(std::move(*pos));
		m_cleanup_condition.notify_one();
	}
	m_device_info.erase(pos);

	++m_generation;

	return S_OK;
}

void sound_xaudio2::audio_task()
{
	bool exiting = FALSE;
	HANDLE hEvents[] = { m_hEventNeedUpdate, m_hEventExiting };
	while (!exiting)
	{
		DWORD wait_result = WaitForMultipleObjects(std::size(hEvents), hEvents, FALSE, INFINITE);
		switch (wait_result)
		{
		case 0: // buffer is complete or new data is available
			{
				std::lock_guard voice_lock(m_voice_mutex);
				for (auto const &voice : m_voice_info)
					voice->submit_if_needed();
			}
			break;
		case 1: // exiting
			exiting = true;
			break;
		}
	}
}

void sound_xaudio2::cleanup_task()
{
	// clean up removed devices on a separate thread to avoid deadlocks
	std::unique_lock cleanup_lock(m_cleanup_mutex);
	while (!m_exiting)
	{
		if (m_zombie_devices.empty() && m_zombie_voices.empty())
			m_cleanup_condition.wait(cleanup_lock);

		while (!m_zombie_devices.empty() && !m_zombie_voices.empty())
		{
			while (!m_zombie_voices.empty())
			{
				voice_info_ptr voice(std::move(m_zombie_voices.back()));
				m_zombie_voices.pop_back();
				cleanup_lock.unlock();
				voice->voice->Stop(0, XAUDIO2_COMMIT_NOW);
				voice->voice->FlushSourceBuffers();
				voice.reset();
				cleanup_lock.lock();
			}
			device_info_ptr device(std::move(m_zombie_devices.back()));
			m_zombie_devices.pop_back();
			cleanup_lock.unlock();
			device->mastering_voice.reset();
			device->engine->UnregisterForCallbacks(device.get());
			device->engine = nullptr;
			cleanup_lock.lock();
		}
	}
}

} // anonymous namespace

} // namespace osd


#else

namespace osd { namespace { MODULE_NOT_SUPPORTED(sound_xaudio2, OSD_SOUND_PROVIDER, "xaudio2") } }

#endif

MODULE_DEFINITION(SOUND_XAUDIO2, osd::sound_xaudio2)
