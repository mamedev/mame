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

// MAME utility library
#include "util/corealloc.h"

// stdlib includes
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstring>
#include <condition_variable>
#include <iterator>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

// standard windows headers
#include <windows.h>

#include <combaseapi.h>
#include <objbase.h>

#include <wrl/client.h>

// sound API headers
#include <functiondiscoverykeys_devpkey.h>
#include <mmdeviceapi.h>
#include <mmreg.h>
#include <xaudio2.h>


namespace osd {

namespace {

//============================================================
//  Constants
//============================================================

constexpr float SUBMIT_INTERVAL_TARGET = 10e-3;
constexpr float RESAMPLE_TOLERANCE = 1.20F;

//============================================================
//  Macros
//============================================================

// Check HRESULT result and log if error, then take an optional action on failure
#define HR_LOG( CALL, LOGFN, ONFAIL ) do { \
	result = CALL; \
	if (FAILED(result)) { \
		LOGFN("Sound: " #CALL " failed with error 0x%X\n", (unsigned int)result); \
		ONFAIL; } \
} while (0)

// Variant of HR_LOG to log using osd_printf_error
#define HR_LOGE( CALL, ONFAIL ) HR_LOG(CALL, osd_printf_error, ONFAIL)

// Macro to check for a failed HRESULT and if failed, goto a label called Error:
#define HR_GOERR( CALL ) HR_LOGE( CALL, goto Error;)

//============================================================
//  Structs and typedefs
//============================================================

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
			obj->Stop(0, XAUDIO2_COMMIT_NOW);
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
	std::vector<std::unique_ptr<BYTE []> > m_pool;
	CRITICAL_SECTION m_critical_section;
	int m_initial;
	int m_buffersize;

public:
	// constructor
	bufferpool(int capacity, int bufferSize) :
		m_initial(capacity),
		m_buffersize(bufferSize)
	{
		InitializeCriticalSectionAndSpinCount(&m_critical_section, 4096);

		m_pool.reserve(m_initial * 2);
		for (int i = 0; i < m_initial; i++)
			m_pool.emplace_back(util::make_unique_clear<BYTE []>(m_buffersize));
	}

	~bufferpool()
	{
		DeleteCriticalSection(&m_critical_section);
	}

	// get next buffer element from the pool
	std::unique_ptr<BYTE []> next()
	{
		std::unique_ptr<BYTE []> next_buffer;

		EnterCriticalSection(&m_critical_section);
		if (!m_pool.empty())
		{
			next_buffer = std::move(m_pool.back());
			m_pool.pop_back();
		}
		LeaveCriticalSection(&m_critical_section);

		if (!next_buffer)
			next_buffer = util::make_unique_clear<BYTE []>(m_buffersize);

		return next_buffer;
	}

	// release element, make it available back in the pool
	void return_to_pool(std::unique_ptr<BYTE []> &&buffer)
	{
		assert(buffer);
		memset(buffer.get(), 0, m_buffersize);
		EnterCriticalSection(&m_critical_section);
		try
		{
			m_pool.emplace_back(std::move(buffer));
			LeaveCriticalSection(&m_critical_section);
		}
		catch (...)
		{
			LeaveCriticalSection(&m_critical_section);
			throw;
		}
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
		m_overflows(0),
		m_underflows(0)
	{
	}

	// osd_module implementation
	virtual bool probe() override;
	virtual int init(osd_interface &osd, osd_options const &options) override;
	virtual void exit() override;

	// sound_module implementation
	virtual uint32_t get_generation() override;
	virtual audio_info get_information() override;
	virtual bool external_per_channel_volume() override { return true; }
	virtual bool split_streams_per_source() override { return true; }
	virtual uint32_t stream_sink_open(uint32_t node, std::string name, uint32_t rate) override;
	virtual void stream_set_volumes(uint32_t id, std::vector<float> const &db) override;
	virtual void stream_close(uint32_t id) override;
	virtual void stream_sink_update(uint32_t id, int16_t const *buffer, int samples_this_frame) override;

private:
	class device_info final : public IXAudio2EngineCallback
	{
	public:
		device_info(sound_xaudio2 &host) : engine_error(false), m_host(host) { }
		~device_info();
		device_info(device_info const &) = delete;
		device_info(device_info &&) = delete;

		std::wstring            device_id;
		mm_device_ptr           device;
		x_audio_2_ptr           engine;
		mastering_voice_ptr     mastering_voice;
		audio_info::node_info   info;
		bool                    engine_error;

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
		void stop();

		source_voice_ptr            voice;
		audio_info::stream_info     info;
		std::unique_ptr<float []>   volume_matrix;

	private:
		void submit_buffer(std::unique_ptr<BYTE []> &&data, DWORD length);

		virtual void STDMETHODCALLTYPE OnVoiceProcessingPassStart(UINT32 BytesRequired) noexcept override;
		virtual void STDMETHODCALLTYPE OnVoiceProcessingPassEnd() noexcept override { }
		virtual void STDMETHODCALLTYPE OnStreamEnd() noexcept override { }
		virtual void STDMETHODCALLTYPE OnBufferStart(void* pBufferContext) noexcept override { }
		virtual void STDMETHODCALLTYPE OnBufferEnd(void *pBufferContext) noexcept override;
		virtual void STDMETHODCALLTYPE OnLoopEnd(void *pBufferContext) noexcept override { }
		virtual void STDMETHODCALLTYPE OnVoiceError(void *pBufferContext, HRESULT error) noexcept override;

		static uint32_t calculate_packet_bytes(WAVEFORMATEX const &format);

		sound_xaudio2 &     m_host;
		bufferpool          m_buffer_pool;
		abuffer             m_buffer;
		uint32_t const      m_packet_bytes;
		unsigned const      m_sample_bytes;
		std::atomic<bool>   m_need_update;
		bool                m_underflowing;
	};

	using device_info_ptr = std::unique_ptr<device_info>;
	using device_info_vector = std::vector<device_info_ptr>;
	using device_info_vector_iterator = device_info_vector::iterator;
	using voice_info_ptr = std::unique_ptr<voice_info>;
	using voice_info_vector = std::vector<voice_info_ptr>;

	// IMMNotificationClient callbacks
	virtual HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState) override;
	virtual HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId) override;
	virtual HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId) override;
	virtual HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId) override;
	virtual HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDeviceId, PROPERTYKEY const key) override;

	// stub IUnknown implementation
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) noexcept override;
	virtual ULONG STDMETHODCALLTYPE AddRef() noexcept override { return 1; }
	virtual ULONG STDMETHODCALLTYPE Release() noexcept override { return 1; }

	device_info_vector_iterator find_device(std::wstring_view device_id);
	HRESULT add_device(device_info_vector_iterator pos, LPCWSTR device_id);
	HRESULT add_device(device_info_vector_iterator pos, std::wstring_view device_id, mm_device_ptr &&device);
	HRESULT remove_device(device_info_vector_iterator pos);
	void purge_voices(uint32_t node);

	void audio_task();
	void cleanup_task();

	// tracking audio endpoint devices
	mm_device_enumerator_ptr    m_device_enum;
	std::wstring                m_default_sink_id;
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

	// tracking streams
	voice_info_vector           m_voice_info;
	std::mutex                  m_voice_mutex;
	uint32_t                    m_next_voice_id;

	uint32_t                    m_generation;
	bool                        m_exiting;

	float                       m_audio_latency;
	handle_ptr                  m_need_update_event;
	handle_ptr                  m_engine_error_event;
	handle_ptr                  m_exiting_event;
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
		osd_printf_error("Sound: Could not find XAudio2 library.\n");
		return 1;
	}

	// get relevant options
	m_audio_latency = options.audio_latency() * 20e-3;
	if (m_audio_latency == 0.0F)
		m_audio_latency = 0.03F;
	m_audio_latency = std::clamp(m_audio_latency, 0.01F, 1.0F);

	// create a multimedia device enumerator and enumerate devices
	HR_GOERR(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&m_device_enum)));
	{
		std::lock_guard device_lock(m_device_mutex);

		HR_GOERR(m_device_enum->RegisterEndpointNotificationCallback(this));
		m_endpoint_notifications = true;

		{
			m_default_sink_id.clear();
			m_default_sink = 0;
			co_task_wstr_ptr default_id;
			result = get_default_audio_device_id(*m_device_enum.Get(), eRender, eMultimedia, default_id);
			if (HRESULT_FROM_WIN32(ERROR_NOT_FOUND) == result)
			{
				// no active output devices, hence no default
			}
			else if (FAILED(result) || !default_id)
			{
				// this is non-fatal, they just might get the wrong default output device
				osd_printf_error("Sound: Error getting default audio output device ID. Error: 0x%X\n", result);
			}
			else
			{
				// reserve double the space to avoid allocations when the default device changes
				std::wstring_view default_id_str(default_id.get());
				m_default_sink_id.reserve(default_id_str.length() * 2);
				m_default_sink_id = default_id_str;
				osd_printf_verbose(
						"Sound: Got default output device ID: %s\n",
						osd::text::from_wstring(m_default_sink_id));
			}
		}

		result = enumerate_audio_endpoints(
				*m_device_enum.Get(),
				eRender,
				DEVICE_STATE_ACTIVE,
				[this] (HRESULT hr, mm_device_ptr &dev) -> bool
				{
					if (FAILED(hr) || !dev)
					{
						osd_printf_error("Sound: Error getting audio device. Error: 0x%X\n", hr);
						return true;
					}

					// skip devices that are disabled or not present
					DWORD state;
					hr = dev->GetState(&state);
					if (FAILED(hr))
					{
						osd_printf_error("Sound: Error getting audio device state. Error: 0x%X\n", hr);
						return true;
					}
					if (DEVICE_STATE_ACTIVE != state)
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
					assert((m_device_info.end() == pos) || ((*pos)->device_id != device_id));
					m_zombie_devices.reserve(m_zombie_devices.size() + m_device_info.size() + 1);
					device_info_ptr devinfo = std::make_unique<device_info>(*this);
					info.m_id = m_next_node_id++;
					devinfo->device_id = std::move(device_id);
					devinfo->device = std::move(dev);
					devinfo->info = std::move(info);

					osd_printf_verbose(
							"Sound: Found audio device %s (%s), assigned ID %u.\n",
							devinfo->info.m_display_name,
							devinfo->info.m_name,
							devinfo->info.m_id);
					if (devinfo->device_id == m_default_sink_id)
						m_default_sink = devinfo->info.m_id;
					m_device_info.emplace(pos, std::move(devinfo));

					return true;
				});
		if (!m_default_sink && !m_device_info.empty())
			m_default_sink = m_device_info.front()->info.m_id;
	}
	if (FAILED(result))
	{
		osd_printf_error("Sound: Error enumerating audio endpoints. Error: 0x%X\n", result);
		goto Error;
	}

	// start a thread to clean up removed voices and devices
	m_cleanup_thread = std::thread([] (sound_xaudio2 *self) { self->cleanup_task(); }, this);

	// Initialize our events
	m_need_update_event.reset(CreateEvent(nullptr, FALSE, FALSE, nullptr));
	m_engine_error_event.reset(CreateEvent(nullptr, FALSE, FALSE, nullptr));
	m_exiting_event.reset(CreateEvent(nullptr, FALSE, FALSE, nullptr));
	if (!m_need_update_event || !m_engine_error_event || !m_exiting_event)
		goto Error;

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
	// unsubscribe from device events
	if (m_endpoint_notifications)
	{
		m_device_enum->UnregisterEndpointNotificationCallback(this);
		m_endpoint_notifications = false;
	}

	// schedule destruction of voices
	{
		std::lock_guard voice_lock(m_voice_mutex);
		while (!m_voice_info.empty())
		{
			{
				std::lock_guard cleanup_lock(m_cleanup_mutex);
				m_zombie_voices.emplace_back(std::move(m_voice_info.back()));
				m_cleanup_condition.notify_all();
			}
			m_voice_info.pop_back();
		}
	}

	// schedule destruction of devices
	{
		std::lock_guard device_lock(m_device_mutex);
		while (!m_device_info.empty())
		{
			{
				std::lock_guard cleanup_lock(m_cleanup_mutex);
				m_zombie_devices.emplace_back(std::move(m_device_info.back()));
				m_cleanup_condition.notify_all();
			}
			m_device_info.pop_back();
		}
	}

	// Wait on processing thread to end
	if (m_exiting_event)
		SetEvent(m_exiting_event.get());

	if (m_audioThread.joinable())
		m_audioThread.join();

	m_need_update_event.reset();
	m_engine_error_event.reset();
	m_exiting_event.reset();

	if (m_cleanup_thread.joinable())
	{
		{
			std::lock_guard cleanup_lock(m_cleanup_mutex);
			m_exiting = true;
			m_cleanup_condition.notify_all();
		}
		m_cleanup_thread.join();
		m_exiting = false;
	}

	// just to be extra sure
	m_zombie_voices.clear();
	m_voice_info.clear();
	m_zombie_devices.clear();
	m_device_info.clear();

	m_device_enum = nullptr;

	if (m_overflows != 0 || m_underflows != 0)
		osd_printf_verbose("Sound: overflows=%u, underflows=%u\n", m_overflows, m_underflows);
	m_overflows.store(0, std::memory_order_relaxed);
	m_underflows.store(0, std::memory_order_relaxed);

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

	try
	{
		// always check for stale argument values
		if (m_device_info.end() == device)
		{
			osd_printf_verbose("Sound: Attempt to open audio output stream %s on unknown node %u.\n", name, node);
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
						"Sound: Error creating XAudio2 engine for audio device %s. Error: 0x%X\n",
						(*device)->info.m_display_name,
						result);
				return 0;
			}

			result = (*device)->engine->RegisterForCallbacks(device->get());
			if (FAILED(result))
			{
				(*device)->engine = nullptr;
				osd_printf_error(
						"Sound: Error registering to receive XAudio2 engine callbacks for audio device %s. Error: 0x%X\n",
						(*device)->info.m_display_name,
						result);
				return 0;
			}

			osd_printf_verbose(
					"Sound: Created XAudio2 engine for audio device %s.\n",
					(*device)->info.m_display_name);
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
						"Sound: Error creating mastering voice for audio device %s. Error: 0x%X\n",
						(*device)->info.m_display_name,
						result);
				return 0;
			}

			osd_printf_verbose(
					"Sound: Created XAudio2 mastering voice for audio device %s.\n",
					(*device)->info.m_display_name);
		}

		// set up desired input format
		WAVEFORMATEXTENSIBLE format;
		populate_wave_format(format, (*device)->info.m_sinks, rate, std::nullopt);

		// set up destinations
		XAUDIO2_SEND_DESCRIPTOR destination;
		XAUDIO2_VOICE_SENDS sends;
		destination.Flags = 0;
		destination.pOutputVoice = (*device)->mastering_voice.get();
		sends.SendCount = 1;
		sends.pSends = &destination;

		// create the voice info object
		voice_info_ptr info = std::make_unique<voice_info>(*this, format.Format);
		info->info.m_node = node;
		info->info.m_volumes.resize((*device)->info.m_sinks, 0.0F);

		// create a source voice for this stream
		IXAudio2SourceVoice *source_voice_raw = nullptr;
		UINT32 flags = XAUDIO2_VOICE_NOPITCH;
		if (rate == (*device)->info.m_rate.m_default_rate)
			flags |= XAUDIO2_VOICE_NOSRC;
		result = (*device)->engine->CreateSourceVoice(
				&source_voice_raw,
				&format.Format,
				flags,
				1.0F,
				info.get(),
				&sends,
				nullptr);
		info->voice.reset(std::exchange(source_voice_raw, nullptr));
		if (FAILED(result) || !info->voice)
		{
			osd_printf_error(
					"Sound: Error creating source voice for audio device %s. Error: 0x%X\n",
					(*device)->info.m_display_name,
					result);
			return 0;
		}
		osd_printf_verbose(
				"Sound: Created XAudio2 source voice for %s on audio device %s.\n",
				name,
				(*device)->info.m_display_name);

		// set the channel mapping
		result = info->voice->SetOutputMatrix(
				nullptr,
				format.Format.nChannels,
				format.Format.nChannels,
				info->volume_matrix.get(),
				XAUDIO2_COMMIT_NOW);
		if (FAILED(result))
		{
			osd_printf_error(
					"Sound: Error setting source voice output matrix for audio device %s. Error: 0x%X\n",
					(*device)->info.m_display_name,
					result);
			return 0;
		}

		// start the voice
		result = info->voice->Start();
		if (FAILED(result))
		{
			osd_printf_error(
					"Sound: Error starting source voice for audio device %s. Error: 0x%X\n",
					(*device)->info.m_display_name,
					result);
			return 0;
		}

		// try to add it to the collection
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
				"Sound: Error setting source voice output matrix for audio stream %u. Error: 0x%X\n",
				(*pos)->info.m_id,
				result);
	}
}

//============================================================
//  stream_close
//============================================================

void sound_xaudio2::stream_close(uint32_t id)
{
	voice_info_ptr voice;
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

		voice = std::move(*pos);
		m_voice_info.erase(pos);
	}

	// try to clean up on a separate thread to avoid potentially blocking here
	try
	{
		std::lock_guard cleanup_lock(m_cleanup_mutex);
		m_zombie_voices.emplace_back(std::move(voice));
	}
	catch (std::bad_alloc const &)
	{
		// voice will be destroyed when it falls out of scope
	}
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
//  ~device_info
//============================================================

sound_xaudio2::device_info::~device_info()
{
	// try to clean up in an orderly fashion
	mastering_voice.reset();
	if (engine)
	{
		engine->UnregisterForCallbacks(this);
		engine = nullptr;
	}
}

//============================================================
//  IXAudio2EngineCallback::OnCriticalError
//============================================================

void sound_xaudio2::device_info::OnCriticalError(HRESULT error)
{
	std::string name;
	{
		std::lock_guard device_lock(m_host.m_device_mutex);
		engine_error = true;
		SetEvent(m_host.m_engine_error_event.get());

		try
		{
			name = info.m_display_name;
		}
		catch (std::bad_alloc const &)
		{
		}
	}

	try
	{
		osd_printf_verbose("Sound: XAudio2 critical error for device %s. Error: 0x%X\n", name, error);
	}
	catch (std::bad_alloc const &)
	{
	}
}

//============================================================
//  voice_info
//============================================================

sound_xaudio2::voice_info::voice_info(sound_xaudio2 &host, WAVEFORMATEX const &format) :
	volume_matrix(std::make_unique<float []>(format.nChannels * format.nChannels)),
	m_host(host),
	m_buffer_pool(4, calculate_packet_bytes(format)),
	m_buffer(format.nChannels),
	m_packet_bytes(calculate_packet_bytes(format)),
	m_sample_bytes(format.nBlockAlign),
	m_need_update(false),
	m_underflowing(false)
{
	// set default volume matrix
	for (unsigned i = 0; format.nChannels > i; ++i)
	{
		for (unsigned j = 0; format.nChannels > j; ++j)
			volume_matrix[(format.nChannels * i) + j] = (i == j) ? 1.0F : 0.0F;
	}
}

//============================================================
//  ~voice_info
//============================================================

sound_xaudio2::voice_info::~voice_info()
{
	// stop this before any buffers get destructed
	stop();
	voice.reset();
}

//============================================================
//  update
//============================================================

void sound_xaudio2::voice_info::update(int16_t const *buffer, int samples_this_frame)
{
	try
	{
		m_buffer.push(buffer, samples_this_frame);
	}
	catch (...)
	{
	}
	m_need_update.store(true, std::memory_order_relaxed);
	SetEvent(m_host.m_need_update_event.get());
}

//============================================================
//  submit_if_needed
//============================================================

void sound_xaudio2::voice_info::submit_if_needed()
{
	if (!m_need_update.load(std::memory_order_relaxed))
		return;

	m_need_update = false;

	XAUDIO2_VOICE_STATE state;
	voice->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);

	uint32_t available_bytes = m_buffer.available() * m_sample_bytes;

	// if there's at least one packet queued and we don't have enough to fill another packet, wait
	if ((1 <= state.BuffersQueued) && (m_packet_bytes > available_bytes))
		return;

	// don't keep more than two packets queued in the engine
	if (2 <= state.BuffersQueued)
		return;

	if (available_bytes)
	{
		auto buffers = state.BuffersQueued;
		do
		{
			std::unique_ptr<BYTE []> packet;
			try
			{
				packet = m_buffer_pool.next();
			}
			catch (...)
			{
				return;
			}
			auto const packet_size = std::min(available_bytes, m_packet_bytes);
			m_buffer.get(reinterpret_cast<int16_t *>(packet.get()), packet_size / m_sample_bytes);
			submit_buffer(std::move(packet), packet_size);
			available_bytes -= packet_size;
			buffers++;
		}
		while ((2 > buffers) && (m_packet_bytes <= available_bytes));
	}
}

//============================================================
//  stop
//============================================================

void sound_xaudio2::voice_info::stop()
{
	m_buffer.clear();
	m_need_update.store(false, std::memory_order_relaxed);
	if (voice)
	{
		voice->Stop(0, XAUDIO2_COMMIT_NOW);
		voice->FlushSourceBuffers();
	}
}

//============================================================
//  submit_buffer
//============================================================

inline void sound_xaudio2::voice_info::submit_buffer(std::unique_ptr<BYTE []> &&data, DWORD length)
{
	assert(length);

	XAUDIO2_BUFFER buf = { 0 };
	buf.AudioBytes = length;
	buf.pAudioData = data.get();
	buf.PlayBegin = 0;
	buf.PlayLength = length / m_sample_bytes;
	buf.Flags = XAUDIO2_END_OF_STREAM; // this flag just suppresses underrun warnings
	buf.pContext = data.get();

	HRESULT const result = voice->SubmitSourceBuffer(&buf);
	if (SUCCEEDED(result))
	{
		// If we succeeded, relinquish the buffer allocation to the XAudio2 runtime
		// The buffer will be freed on the OnBufferCompleted callback
		data.release();
	}
	else
	{
		osd_printf_verbose("Sound: XAudio2 failed to submit source buffer (non-fatal). Error: 0x%X\n", result);
		m_buffer_pool.return_to_pool(std::move(data));
	}
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
	if (completed_buffer)
		m_buffer_pool.return_to_pool(std::unique_ptr<BYTE []>(completed_buffer));

	m_need_update.store(true, std::memory_order_relaxed);
	SetEvent(m_host.m_need_update_event.get());
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

		osd_printf_verbose("Sound: XAudio2 voice error for stream %u. Error: 0x%X\n", info.m_id, error);

		if (completed_buffer)
			m_buffer_pool.return_to_pool(std::unique_ptr<BYTE []>(completed_buffer));

		auto const pos = std::find_if(
				m_host.m_voice_info.begin(),
				m_host.m_voice_info.end(),
				[this] (voice_info_ptr const &value) { return value.get() == this; });
		if (m_host.m_voice_info.end() == pos)
			return;

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
//  calculate_packet_bytes
//============================================================

uint32_t sound_xaudio2::voice_info::calculate_packet_bytes(WAVEFORMATEX const &format)
{
	return format.nBlockAlign * uint32_t(format.nSamplesPerSec * SUBMIT_INTERVAL_TARGET * RESAMPLE_TOLERANCE);
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

		if (DEVICE_STATE_ACTIVE == dwNewState)
		{
			if ((m_device_info.end() == pos) || ((*pos)->device_id != device_id))
			{
				HRESULT result;

				mm_device_ptr device;
				result = m_device_enum->GetDevice(pwstrDeviceId, device.GetAddressOf());
				if (FAILED(result))
				{
					osd_printf_error(
							"Sound: Error getting audio device %s. Error: 0x%X\n",
							osd::text::from_wstring(device_id),
							result);
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
					"Sound: Added sound device %s appears to already be present.\n",
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
//  IMMNotificationClient::OnDefaultDeviceChanged
//============================================================

HRESULT sound_xaudio2::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId)
{
	try
	{
		if ((eRender == flow) && (eMultimedia == role))
		{
			co_task_wstr_ptr default_id_str;
			std::wstring_view default_id;
			if (pwstrDefaultDeviceId)
			{
				default_id = pwstrDefaultDeviceId;
			}
			else
			{
				// changing the app setting to "Default" in mixer controls gives a null string here
				HRESULT const result = get_default_audio_device_id(*m_device_enum.Get(), flow, role, default_id_str);
				if (HRESULT_FROM_WIN32(ERROR_NOT_FOUND) == result)
					return S_OK;
				else if (FAILED(result))
					return result;
				else if (!default_id_str)
					return E_POINTER;
				default_id = default_id_str.get();
			}

			std::lock_guard device_lock(m_device_mutex);
			if (m_default_sink_id != default_id)
			{
				m_default_sink_id = default_id;
				auto const pos = find_device(m_default_sink_id);
				if ((m_device_info.end() != pos) && ((*pos)->device_id == m_default_sink_id) && ((*pos)->info.m_id != m_default_sink))
				{
					try
					{
						osd_printf_verbose(
								"Sound: Default output device changed to %s.\n",
								(*pos)->info.m_display_name);
					}
					catch (std::bad_alloc const &)
					{
					}
					m_default_sink = (*pos)->info.m_id;

					++m_generation;
				}
				else
				{
					try
					{
						osd_printf_verbose(
								"Sound: Default output device changed ID changed to %s (not found).\n",
								osd::text::from_wstring(m_default_sink_id));
					}
					catch (std::bad_alloc const &)
					{
					}
				}
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
		if (PKEY_Device_FriendlyName == key)
		{
			std::lock_guard device_lock(m_device_mutex);
			std::wstring_view device_id(pwstrDeviceId);
			auto const pos = find_device(device_id);

			if ((m_device_info.end() != pos) && ((*pos)->device_id == device_id))
			{
				HRESULT result;

				property_store_ptr properties;
				result = (*pos)->device->OpenPropertyStore(STGM_READ, properties.GetAddressOf());
				if (FAILED(result))
				{
					osd_printf_error(
							"Sound: Error opening property store for audio device %s. Error: 0x%X\n",
							(*pos)->info.m_display_name,
							result);
					return result;
				}

				std::optional<std::string> name;
				result = get_string_property_value(*properties.Get(), PKEY_Device_FriendlyName, name);
				if (FAILED(result))
				{
					osd_printf_error(
							"Sound: Error getting updated display name for audio device %s. Error: 0x%X\n",
							(*pos)->info.m_display_name,
							result);
					return result;
				}

				if (name)
				{
					(*pos)->info.m_display_name = std::move(*name);

					++m_generation;
				}
			}
		}
		else if (PKEY_AudioEngine_DeviceFormat == key)
		{
			std::lock_guard device_lock(m_device_mutex);
			std::wstring_view device_id(pwstrDeviceId);
			auto const pos = find_device(device_id);

			if ((m_device_info.end() != pos) && ((*pos)->device_id == device_id))
			{
				// XAudio2 can't deal with this - it needs to be torn down and re-created,
				remove_device(pos);
				add_device(find_device(device_id), pwstrDeviceId);
			}
			else
			{
				// deal with devices added with the format property empty and updated later
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

HRESULT sound_xaudio2::QueryInterface(REFIID riid, void **ppvObject) noexcept
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
				"Sound: Error getting audio device %s. Error: 0x%X\n",
				osd::text::from_wstring(device_id),
				result);
		return result;
	}

	// ignore if it's disabled or not present
	DWORD state;
	result = device->GetState(&state);
	if (FAILED(result))
	{
		osd_printf_error(
				"Sound: Error getting audio device %s state. Error: 0x%X\n",
				osd::text::from_wstring(device_id),
				result);
		return result;
	}
	if (DEVICE_STATE_ACTIVE != state)
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
			if (!m_default_sink || (devinfo->device_id == m_default_sink_id))
				m_default_sink = devinfo->info.m_id;
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
	purge_voices((*pos)->info.m_id);

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

void sound_xaudio2::purge_voices(uint32_t node)
{
	std::lock_guard voice_lock(m_voice_mutex);
	auto it = m_voice_info.begin();
	while (m_voice_info.end() != it)
	{
		if ((*it)->info.m_node == node)
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

void sound_xaudio2::audio_task()
{
	bool exiting = FALSE;
	HANDLE hEvents[] = { m_need_update_event.get(), m_engine_error_event.get(), m_exiting_event.get() };
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
		case 1: // engine error
			{
				std::lock_guard device_lock(m_device_mutex);
				auto it = m_device_info.begin();
				bool errors = false;
				for (auto const &device : m_device_info)
				{
					if (!device->engine_error)
						continue;

					// clean up any voices associated with this device
					purge_voices((*it)->info.m_id);

					// tear down the XAudio2 engine - it will be recreated next time a stream is opened
					device->engine->UnregisterForCallbacks(device.get());
					x_audio_2_ptr old_engine = std::move(device->engine);
					mastering_voice_ptr old_mastering_voice = std::move(device->mastering_voice);
					device->mastering_voice.reset();
					device->engine = nullptr;
					device->engine_error = false;

					// bump this so the sound manager recreates its streams
					device->info.m_id = m_next_node_id++;

					// try to get another thread to do the cleanup
					device_info_ptr zombie;
					try
					{
						zombie = std::make_unique<device_info>(*this);
						zombie->engine = std::move(old_engine);
						zombie->mastering_voice = std::move(old_mastering_voice);
						old_mastering_voice.reset();
						old_engine = nullptr;
						{
							std::lock_guard cleanup_lock(m_cleanup_mutex);
							m_zombie_devices.emplace_back(std::move(zombie));
							m_cleanup_condition.notify_one();
						}
					}
					catch (std::bad_alloc const &)
					{
						// make sure the cleanup thread wakes up
						std::lock_guard cleanup_lock(m_cleanup_mutex);
						m_cleanup_condition.notify_one();

						// old engine and mastering voice will fall out of scope and be destroyed
						// this could block things it shouldn't, but leaking would be worse
					}
				}

				// bump generation if we actually changed something
				if (errors)
					++m_generation;
			}
			break;
		case 2: // exiting
			exiting = true;
			break;
		}
	}
}

void sound_xaudio2::cleanup_task()
{
	// clean up removed voices and devices on a separate thread to avoid deadlocks
	std::unique_lock cleanup_lock(m_cleanup_mutex);
	while (!m_exiting || !m_zombie_devices.empty() || !m_zombie_voices.empty())
	{
		if (m_zombie_devices.empty() && m_zombie_voices.empty())
			m_cleanup_condition.wait(cleanup_lock);

		while (!m_zombie_devices.empty() || !m_zombie_voices.empty())
		{
			while (!m_zombie_voices.empty())
			{
				voice_info_ptr voice(std::move(m_zombie_voices.back()));
				m_zombie_voices.pop_back();
				cleanup_lock.unlock();
				voice.reset();
				cleanup_lock.lock();
			}
			if (!m_zombie_devices.empty())
			{
				device_info_ptr device(std::move(m_zombie_devices.back()));
				m_zombie_devices.pop_back();
				cleanup_lock.unlock();
				device.reset();
				cleanup_lock.lock();
			}
		}
	}
}

} // anonymous namespace

} // namespace osd


#else

namespace osd { namespace { MODULE_NOT_SUPPORTED(sound_xaudio2, OSD_SOUND_PROVIDER, "xaudio2") } }

#endif

MODULE_DEFINITION(SOUND_XAUDIO2, osd::sound_xaudio2)
