// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//====================================================================
//
//  wasapi_sound.cpp - WASAPI implementation of MAME sound routines
//
//====================================================================

#include "sound_module.h"

#include "modules/osdmodule.h"

#if defined(OSD_WINDOWS) | defined(SDLMAME_WIN32)

// local headers
#include "mmdevice_helpers.h"

// OSD headers
#include "osdcore.h"
#include "strconv.h"

// C++ standard library
#include <algorithm>
#include <cassert>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

// standard windows headers
#include <windows.h>

#include <combaseapi.h>
#include <objbase.h>

// sound API headers
#include <audioclient.h>
#include <functiondiscoverykeys_devpkey.h>
#include <mmdeviceapi.h>
#include <mmreg.h>


namespace osd {

namespace {

struct handle_deleter
{
	void operator()(HANDLE obj) const
	{
		if (obj)
			CloseHandle(obj);
	}
};

using handle_ptr                = std::unique_ptr<std::remove_pointer_t<HANDLE>, handle_deleter>;
using wave_format_ptr           = std::unique_ptr<WAVEFORMATEX, co_task_mem_deleter>;

using audio_client_ptr          = Microsoft::WRL::ComPtr<IAudioClient>;
using audio_render_client_ptr   = Microsoft::WRL::ComPtr<IAudioRenderClient>;
using audio_capture_client_ptr  = Microsoft::WRL::ComPtr<IAudioCaptureClient>;



class sound_wasapi :
		public osd_module,
		public sound_module,
		public IMMNotificationClient
{
public:
	sound_wasapi() :
		osd_module(OSD_SOUND_PROVIDER, "wasapi"),
		sound_module()
	{
	}

	// osd_module implementation
	virtual bool probe() override;
	virtual int init(osd_interface &osd, osd_options const &options) override;
	virtual void exit() override;

	// sound_module implementation
	virtual uint32_t get_generation() override;
	virtual audio_info get_information() override;
	virtual bool split_streams_per_source() override { return true; }
	virtual uint32_t stream_sink_open(uint32_t node, std::string name, uint32_t rate) override;
	virtual uint32_t stream_source_open(uint32_t node, std::string name, uint32_t rate) override;
	virtual void stream_close(uint32_t id) override;
	virtual void stream_sink_update(uint32_t id, int16_t const *buffer, int samples_this_frame) override;
	virtual void stream_source_update(uint32_t id, int16_t *buffer, int samples_this_frame) override;

private:
	struct device_info
	{
		std::wstring            device_id;
		mm_device_ptr           device;
		audio_info::node_info   info;
	};

	class stream_info
	{
	public:
		stream_info(
				audio_info::node_info const &node,
				audio_client_ptr client,
				audio_render_client_ptr render,
				UINT32 buffer_frames,
				handle_ptr &&event);
		stream_info(
				audio_info::node_info const &node,
				audio_client_ptr client,
				audio_capture_client_ptr capture,
				UINT32 buffer_frames,
				handle_ptr &&event);
		~stream_info();

		stream_info(stream_info const &) = delete;
		stream_info(stream_info &&) = delete;
		stream_info operator=(stream_info const &) = delete;
		stream_info operator=(stream_info &&) = delete;

		HRESULT start();
		void render(int16_t const *buffer, int samples_this_frame);
		void capture(int16_t *buffer, int samples_this_frame);

		audio_info::stream_info info;

	private:
		stream_info(
				audio_info::node_info const &node,
				audio_client_ptr client,
				UINT32 buffer_frames,
				handle_ptr &&event,
				uint32_t channels);

		void stop();
		void render_task();
		void capture_task();

		// order is important so things unwind properly
		handle_ptr                  m_event;
		std::thread                 m_thread;
		CRITICAL_SECTION            m_critical_section;
		abuffer                     m_buffer;
		audio_client_ptr            m_client;
		audio_render_client_ptr     m_render;
		audio_capture_client_ptr    m_capture;
		UINT32                      m_buffer_frames;
		bool                        m_exiting;
	};

	using device_info_ptr = std::unique_ptr<device_info>;
	using device_info_vector = std::vector<device_info_ptr>;
	using device_info_vector_iterator = device_info_vector::iterator;
	using stream_info_ptr = std::unique_ptr<stream_info>;
	using stream_info_vector = std::vector<stream_info_ptr>;

	// IMMNotificationClient implementation
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

	void cleanup_task();

	mm_device_enumerator_ptr    m_device_enum;
	std::wstring                m_default_sink_id;
	std::wstring                m_default_source_id;
	device_info_vector          m_device_info;
	std::mutex                  m_device_mutex;
	uint32_t                    m_next_node_id = 1;
	uint32_t                    m_default_sink = 0;
	uint32_t                    m_default_source = 0;
	bool                        m_endpoint_notifications = false;

	stream_info_vector          m_stream_info;
	std::mutex                  m_stream_mutex;
	uint32_t                    m_next_stream_id = 1;

	stream_info_vector          m_zombie_streams;
	std::condition_variable     m_cleanup_condition;
	std::thread                 m_cleanup_thread;
	std::mutex                  m_cleanup_mutex;

	uint32_t                    m_generation = 1;
	bool                        m_exiting = false;
};



//============================================================
//  stream_info
//============================================================

sound_wasapi::stream_info::stream_info(
		audio_info::node_info const &node,
		audio_client_ptr client,
		audio_render_client_ptr render,
		UINT32 buffer_frames,
		handle_ptr &&event) :
	stream_info(node, std::move(client), buffer_frames, std::move(event), node.m_sinks)
{
	m_render = std::move(render);
}


sound_wasapi::stream_info::stream_info(
		audio_info::node_info const &node,
		audio_client_ptr client,
		audio_capture_client_ptr capture,
		UINT32 buffer_frames,
		handle_ptr &&event) :
	stream_info(node, std::move(client), buffer_frames, std::move(event), node.m_sources)
{
	m_capture = std::move(capture);
}


sound_wasapi::stream_info::stream_info(
		audio_info::node_info const &node,
		audio_client_ptr client,
		UINT32 buffer_frames,
		handle_ptr &&event,
		uint32_t channels) :
	m_event(std::move(event)),
	m_buffer(channels),
	m_client(std::move(client)),
	m_buffer_frames(buffer_frames),
	m_exiting(false)
{
	info.m_node = node.m_id;
	InitializeCriticalSection(&m_critical_section);
}


sound_wasapi::stream_info::~stream_info()
{
	m_client->Stop();
	stop();
	DeleteCriticalSection(&m_critical_section);
}


HRESULT sound_wasapi::stream_info::start()
{
	try
	{
		if (m_render)
			m_thread = std::thread([] (stream_info *self) { self->render_task(); }, this);
		else if (m_capture)
			m_thread = std::thread([] (stream_info *self) { self->capture_task(); }, this);
		else
			return E_INVALIDARG;
	}
	catch (std::system_error const &)
	{
		return E_FAIL;
	}

	HRESULT const result = m_client->Start();
	if (FAILED(result))
		stop();
	return result;
}


void sound_wasapi::stream_info::render(int16_t const *buffer, int samples_this_frame)
{
	assert(m_render);

	EnterCriticalSection(&m_critical_section);
	try
	{
		m_buffer.push(buffer, samples_this_frame);
		LeaveCriticalSection(&m_critical_section);
	}
	catch (...)
	{
		LeaveCriticalSection(&m_critical_section);
		throw;
	}
}


void sound_wasapi::stream_info::capture(int16_t *buffer, int samples_this_frame)
{
	assert(m_capture);

	EnterCriticalSection(&m_critical_section);
	try
	{
		m_buffer.get(buffer, samples_this_frame);
		LeaveCriticalSection(&m_critical_section);
	}
	catch (...)
	{
		LeaveCriticalSection(&m_critical_section);
		throw;
	}
}


void sound_wasapi::stream_info::stop()
{
	if (m_thread.joinable())
	{
		EnterCriticalSection(&m_critical_section);
		m_exiting = true;
		SetEvent(m_event.get());
		LeaveCriticalSection(&m_critical_section);
		m_thread.join();
		m_exiting = false;
	}
}


void sound_wasapi::stream_info::render_task()
{
	EnterCriticalSection(&m_critical_section);
	while (!m_exiting)
	{
		LeaveCriticalSection(&m_critical_section);
		WaitForSingleObject(m_event.get(), INFINITE);

		if (!m_exiting)
		{
			// FIXME: need to deal with AUDCLNT_E_DEVICE_INVALIDATED
			HRESULT result;

			UINT32 padding = 0;
			BYTE *data = nullptr;
			result = m_client->GetCurrentPadding(&padding);
			if (SUCCEEDED(result))
				result = m_render->GetBuffer(m_buffer_frames - padding, &data);

			EnterCriticalSection(&m_critical_section);
			if (SUCCEEDED(result))
			{
				m_buffer.get(reinterpret_cast<int16_t *>(data), m_buffer_frames - padding);
				result = m_render->ReleaseBuffer(m_buffer_frames - padding, 0);
			}
		}
		else
		{
			EnterCriticalSection(&m_critical_section);
		}
	}
	LeaveCriticalSection(&m_critical_section);
}


void sound_wasapi::stream_info::capture_task()
{
	EnterCriticalSection(&m_critical_section);
	while (!m_exiting)
	{
		LeaveCriticalSection(&m_critical_section);
		WaitForSingleObject(m_event.get(), INFINITE);

		if (!m_exiting)
		{
			// FIXME: need to deal with AUDCLNT_E_DEVICE_INVALIDATED
			HRESULT result;

			BYTE *data = nullptr;
			UINT32 frames = 0;
			DWORD flags = 0;
			result = m_capture->GetBuffer(&data, &frames, &flags, nullptr, nullptr);

			EnterCriticalSection(&m_critical_section);
			if (SUCCEEDED(result))
			{
				if (AUDCLNT_S_BUFFER_EMPTY != result)
				{
					try
					{
						m_buffer.push(reinterpret_cast<int16_t const *>(data), frames);
					}
					catch (...)
					{
					}
				}
				result = m_capture->ReleaseBuffer(frames);
			}
		}
		else
		{
			EnterCriticalSection(&m_critical_section);
		}
	}
	LeaveCriticalSection(&m_critical_section);
}



//============================================================
//  osd_module implementation
//============================================================

bool sound_wasapi::probe()
{
	return true;
}


int sound_wasapi::init(osd_interface &osd, osd_options const &options)
{
	HRESULT result;

	// create a multimedia device enumerator and enumerate devices
	result = CoCreateInstance(
			__uuidof(MMDeviceEnumerator),
			nullptr,
			CLSCTX_ALL,
			IID_PPV_ARGS(&m_device_enum));
	if (FAILED(result) || !m_device_enum)
	{
		osd_printf_error(
				"Sound: Error creating multimedia device enumerator. Error: 0x%X\n",
				result);
		goto error;
	}
	{
		std::lock_guard device_lock(m_device_mutex);

		result = m_device_enum->RegisterEndpointNotificationCallback(this);
		if (FAILED(result))
		{
			osd_printf_error(
					"Sound: Error registering for multimedia device notifications. Error: 0x%X\n",
					result);
			goto error;
		}
		m_endpoint_notifications = true;

		{
			co_task_wstr_ptr default_id;
			result = get_default_audio_device_id(*m_device_enum.Get(), eRender, eMultimedia, default_id);
			if (FAILED(result))
			{
				// this is non-fatal, they just might get the wrong default output device
				osd_printf_error(
						"Sound: Error getting default audio output device ID. Error: 0x%X\n",
						result);
				m_default_sink_id.clear();
			}
			else
			{
				// reserve double the space to avoid allocations when the default device changes
				std::wstring_view default_id_str(default_id.get());
				m_default_sink_id.reserve(default_id_str.length() * 2);
				m_default_sink_id = default_id_str;
			}
		}

		{
			co_task_wstr_ptr default_id;
			result = get_default_audio_device_id(*m_device_enum.Get(), eCapture, eMultimedia, default_id);
			if (FAILED(result))
			{
				// this is non-fatal, they just might get the wrong default input device
				osd_printf_error(
						"Sound: Error getting default audio input device ID. Error: 0x%X\n",
						result);
				m_default_source_id.clear();
			}
			else
			{
				// reserve double the space to avoid allocations when the default device changes
				std::wstring_view default_id_str(default_id.get());
				m_default_source_id.reserve(default_id_str.length() * 2);
				m_default_source_id = default_id_str;
			}
		}

		result = enumerate_audio_endpoints(
				*m_device_enum.Get(),
				eAll,
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
					if (info.m_sinks && !info.m_sources)
						info.m_sources = info.m_sinks; // TODO: loopback event mode is only supported on Windows 10 1703 or later

					// skip devices that have no outputs or inputs
					if ((0 >= info.m_sinks) && (0 >= info.m_sources))
						return true;

					// add a device ID mapping
					auto const pos = find_device(device_id);
					device_info_ptr devinfo = std::make_unique<device_info>();
					info.m_id = m_next_node_id++;
					devinfo->device_id = std::move(device_id);
					devinfo->device = std::move(dev);
					devinfo->info = std::move(info);

					osd_printf_verbose(
							"Sound: Found audio %sdevice %s (%s), assigned ID %u.\n",
							devinfo->info.m_sinks ? "output " : devinfo->info.m_sources ? "input " : "",
							devinfo->info.m_name,
							osd::text::from_wstring(devinfo->device_id),
							devinfo->info.m_id);
					if (devinfo->info.m_sinks)
					{
						if (!m_default_sink || (devinfo->device_id == m_default_sink_id))
							m_default_sink = devinfo->info.m_id;
					}
					else if (devinfo->info.m_sources)
					{
						if (!m_default_source || (devinfo->device_id == m_default_source_id))
							m_default_source = devinfo->info.m_id;
					}
					m_device_info.emplace(pos, std::move(devinfo));

					return true;
				});
	}
	if (FAILED(result))
	{
		osd_printf_error("Sound: Error enumerating audio endpoints. Error: 0x%X\n", result);
		goto error;
	}

	// start a thread to clean up removed streams
	m_cleanup_thread = std::thread([] (sound_wasapi *self) { self->cleanup_task(); }, this);

	return 0;

error:
	exit();
	return 1;
}


void sound_wasapi::exit()
{
	// unsubscribe from device events
	if (m_endpoint_notifications)
	{
		m_device_enum->UnregisterEndpointNotificationCallback(this);
		m_endpoint_notifications = false;
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

	m_stream_info.clear();
	m_device_info.clear();

	m_device_enum = nullptr;

	osd_printf_verbose("Sound: WASAPI exited\n");
}



//============================================================
//  sound_module implementation
//============================================================

uint32_t sound_wasapi::get_generation()
{
	uint32_t result;
	{
		std::lock_guard device_lock(m_device_mutex);
		result = m_generation;
	}
	return result;
}


audio_info sound_wasapi::get_information()
{
	audio_info result;
	{
		std::lock_guard device_lock(m_device_mutex);

		result.m_generation = m_generation;
		result.m_default_sink = m_default_sink;
		result.m_default_source = m_default_source;

		result.m_nodes.reserve(m_device_info.size());
		for (auto const &device : m_device_info)
			result.m_nodes.emplace_back(device->info);

		std::lock_guard stream_lock(m_stream_mutex);
		result.m_streams.reserve(m_stream_info.size());
		for (auto const &stream : m_stream_info)
			result.m_streams.emplace_back(stream->info);
	}
	return result;
}


uint32_t sound_wasapi::stream_sink_open(uint32_t node, std::string name, uint32_t rate)
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

		// make sure it's an output device
		if (!(*device)->info.m_sinks)
		{
			osd_printf_error(
					"Sound: Attempt to open audio output stream %s on input device %s.\n",
					name,
					(*device)->info.m_name);
			return 0;
		}

		// create an audio client interface
		audio_client_ptr client;
		result = (*device)->device->Activate(
				__uuidof(IAudioClient),
				CLSCTX_ALL,
				nullptr,
				reinterpret_cast<void **>(client.GetAddressOf()));
		if (FAILED(result) || !client)
		{
			osd_printf_error(
					"Sound: Error activating audio client interface for output stream %s on device %s. Error: 0x%X\n",
					name,
					(*device)->info.m_name,
					result);
			return 0;
		}

		// get the mix format for the endpoint
		WAVEFORMATEX *mix_raw = nullptr;
		result = client->GetMixFormat(&mix_raw);
		wave_format_ptr mix(std::exchange(mix_raw, nullptr));
		if (FAILED(result))
		{
			osd_printf_error(
					"Sound: Error getting mix format for output stream %s on device %s. Error: 0x%X\n",
					name,
					(*device)->info.m_name,
					result);
			return 0;
		}

		// set up desired input format
		WAVEFORMATEX format;
		format.wFormatTag = WAVE_FORMAT_PCM;
		format.nChannels = (*device)->info.m_sinks;
		format.nSamplesPerSec = rate;
		format.nAvgBytesPerSec = 2 * format.nChannels * rate;
		format.nBlockAlign = 2 * format.nChannels;
		format.wBitsPerSample = 16;
		format.cbSize = 0;

		// need sample rate conversion if the sample rates don't match
		DWORD stream_flags = AUDCLNT_STREAMFLAGS_EVENTCALLBACK;
		if (format.nSamplesPerSec != mix->nSamplesPerSec)
			stream_flags |= AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY;

		// initialise the audio client interface
		result = client->Initialize(
				AUDCLNT_SHAREMODE_SHARED,
				stream_flags,
				10 * 10'000, // 10 ms in units of 100 ns
				0,
				&format,
				nullptr);
		if (FAILED(result))
		{
			osd_printf_error(
					"Sound: Error initializing audio client interface for output stream %s on device %s. Error: 0x%X\n",
					name,
					(*device)->info.m_name,
					result);
			return 0;
		}
		UINT32 buffer_frames;
		result = client->GetBufferSize(&buffer_frames);
		if (FAILED(result))
		{
			osd_printf_error(
					"Sound: Error getting audio client buffer size for output stream %s on device %s. Error: 0x%X\n",
					name,
					(*device)->info.m_name,
					result);
			return 0;
		}
		audio_render_client_ptr render;
		result = client->GetService(IID_PPV_ARGS(render.GetAddressOf()));
		if (FAILED(result) || !render)
		{
			osd_printf_error(
					"Sound: Error getting audio render client service for output stream %s on device %s. Error: 0x%X\n",
					name,
					(*device)->info.m_name,
					result);
			return 0;
		}

		// create an event handle to receive notifications
		handle_ptr event(CreateEvent(nullptr, FALSE, FALSE, nullptr));
		if (!event)
		{
			osd_printf_error(
					"Sound: Error creating event handle for output stream %s on device %s. Error: 0x%X\n",
					name,
					(*device)->info.m_name,
					result);
			return 0;
		}
		result = client->SetEventHandle(event.get());
		if (FAILED(result))
		{
			osd_printf_error(
					"Sound: Error setting event handle for output stream %s on device %s. Error: 0x%X\n",
					name,
					(*device)->info.m_name,
					result);
			return 0;
		}

		// create a stream info structure
		stream_info_ptr stream = std::make_unique<stream_info>(
				(*device)->info,
				std::move(client),
				std::move(render),
				buffer_frames,
				std::move(event));
		client = nullptr;
		render = nullptr;

		// start the machinery
		result = stream->start();
		if (FAILED(result))
		{
			osd_printf_error(
					"Sound: Error starting output stream %s on device %s. Error: 0x%X\n",
					name,
					(*device)->info.m_name,
					result);
			return 0;
		}

		// try to add it to the collection
		std::lock_guard stream_lock(m_stream_mutex);
		{
			std::lock_guard cleanup_lock(m_cleanup_mutex);
			m_zombie_streams.reserve(m_zombie_streams.size() + m_stream_info.size() + 1);
		}
		m_stream_info.reserve(m_stream_info.size() + 1);
		stream->info.m_id = m_next_stream_id++;
		assert(m_stream_info.empty() || (m_stream_info.back()->info.m_id < stream->info.m_id));
		auto &pos = m_stream_info.emplace_back(std::move(stream));
		return pos->info.m_id;
	}
	catch (std::bad_alloc const &)
	{
		return 0;
	}
}


uint32_t sound_wasapi::stream_source_open(uint32_t node, std::string name, uint32_t rate)
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
			osd_printf_verbose("Sound: Attempt to open audio input stream %s on unknown node %u.\n", name, node);
			return 0;
		}

		// create an audio client interface
		audio_client_ptr client;
		result = (*device)->device->Activate(
				__uuidof(IAudioClient),
				CLSCTX_ALL,
				nullptr,
				reinterpret_cast<void **>(client.GetAddressOf()));
		if (FAILED(result) || !client)
		{
			osd_printf_error(
					"Sound: Error activating audio client interface for input stream %s on device %s. Error: 0x%X\n",
					name,
					(*device)->info.m_name,
					result);
			return 0;
		}

		// get the mix format for the endpoint
		WAVEFORMATEX *mix_raw = nullptr;
		result = client->GetMixFormat(&mix_raw);
		wave_format_ptr mix(std::exchange(mix_raw, nullptr));
		if (FAILED(result))
		{
			osd_printf_error(
					"Sound: Error getting mix format for input stream %s on device %s. Error: 0x%X\n",
					name,
					(*device)->info.m_name,
					result);
			return 0;
		}

		// set up desired input format
		WAVEFORMATEX format;
		format.wFormatTag = WAVE_FORMAT_PCM;
		format.nChannels = (*device)->info.m_sources;
		format.nSamplesPerSec = rate;
		format.nAvgBytesPerSec = 2 * format.nChannels * rate;
		format.nBlockAlign = 2 * format.nChannels;
		format.wBitsPerSample = 16;
		format.cbSize = 0;

		// need sample rate conversion if the sample rates don't match, need loopback for output
		DWORD stream_flags = AUDCLNT_STREAMFLAGS_EVENTCALLBACK;
		if ((*device)->info.m_sinks)
			stream_flags |= AUDCLNT_STREAMFLAGS_LOOPBACK;
		if (format.nSamplesPerSec != mix->nSamplesPerSec)
			stream_flags |= AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY;

		// initialise the audio client interface
		result = client->Initialize(
				AUDCLNT_SHAREMODE_SHARED,
				stream_flags,
				10 * 10'000, // 10 ms in units of 100 ns
				0,
				&format,
				nullptr);
		if (FAILED(result))
		{
			osd_printf_error(
					"Sound: Error initializing audio client interface for input stream %s on device %s. Error: 0x%X\n",
					name,
					(*device)->info.m_name,
					result);
			return 0;
		}
		UINT32 buffer_frames;
		result = client->GetBufferSize(&buffer_frames);
		if (FAILED(result))
		{
			osd_printf_error(
					"Sound: Error getting audio client buffer size for input stream %s on device %s. Error: 0x%X\n",
					name,
					(*device)->info.m_name,
					result);
			return 0;
		}
		audio_capture_client_ptr capture;
		result = client->GetService(IID_PPV_ARGS(capture.GetAddressOf()));
		if (FAILED(result) || !capture)
		{
			osd_printf_error(
					"Sound: Error getting audio capture client service for input stream %s on device %s. Error: 0x%X\n",
					name,
					(*device)->info.m_name,
					result);
			return 0;
		}

		// create an event handle to receive notifications
		handle_ptr event(CreateEvent(nullptr, FALSE, FALSE, nullptr));
		if (!event)
		{
			osd_printf_error(
					"Sound: Error creating event handle for input stream %s on device %s. Error: 0x%X\n",
					name,
					(*device)->info.m_name,
					result);
			return 0;
		}
		result = client->SetEventHandle(event.get());
		if (FAILED(result))
		{
			osd_printf_error(
					"Sound: Error setting event handle for input stream %s on device %s. Error: 0x%X\n",
					name,
					(*device)->info.m_name,
					result);
			return 0;
		}

		// create a stream info structure
		stream_info_ptr stream = std::make_unique<stream_info>(
				(*device)->info,
				std::move(client),
				std::move(capture),
				buffer_frames,
				std::move(event));
		client = nullptr;
		capture = nullptr;

		// start the machinery
		result = stream->start();
		if (FAILED(result))
		{
			osd_printf_error(
					"Sound: Error starting input stream %s on device %s. Error: 0x%X\n",
					name,
					(*device)->info.m_name,
					result);
			return 0;
		}

		// try to add it to the collection
		std::lock_guard stream_lock(m_stream_mutex);
		{
			std::lock_guard cleanup_lock(m_cleanup_mutex);
			m_zombie_streams.reserve(m_zombie_streams.size() + m_stream_info.size() + 1);
		}
		m_stream_info.reserve(m_stream_info.size() + 1);
		stream->info.m_id = m_next_stream_id++;
		assert(m_stream_info.empty() || (m_stream_info.back()->info.m_id < stream->info.m_id));
		auto &pos = m_stream_info.emplace_back(std::move(stream));
		return pos->info.m_id;
	}
	catch (std::bad_alloc const &)
	{
		return 0;
	}
}


void sound_wasapi::stream_close(uint32_t id)
{
	stream_info_ptr stream; // make sure this falls out of scope after the lock
	{
		std::lock_guard stream_lock(m_stream_mutex);
		auto const pos = std::lower_bound(
				m_stream_info.begin(),
				m_stream_info.end(),
				id,
				[] (stream_info_ptr const &a, uint32_t b) { return a->info.m_id < b; });
		if ((m_stream_info.end() == pos) || ((*pos)->info.m_id != id))
		{
			// the sound manager tries to close streams that have disappeared due to device disconnection
			return;
		}

		stream = std::move(*pos);
		m_stream_info.erase(pos);
	}
}


void sound_wasapi::stream_sink_update(uint32_t id, int16_t const *buffer, int samples_this_frame)
{
	std::lock_guard stream_lock(m_stream_mutex);
	auto const pos = std::lower_bound(
			m_stream_info.begin(),
			m_stream_info.end(),
			id,
			[] (stream_info_ptr const &a, uint32_t b) { return a->info.m_id < b; });
	if ((m_stream_info.end() == pos) || ((*pos)->info.m_id != id))
		return;

	(*pos)->render(buffer, samples_this_frame);
}


void sound_wasapi::stream_source_update(uint32_t id, int16_t *buffer, int samples_this_frame)
{
	std::lock_guard stream_lock(m_stream_mutex);
	auto const pos = std::lower_bound(
			m_stream_info.begin(),
			m_stream_info.end(),
			id,
			[] (stream_info_ptr const &a, uint32_t b) { return a->info.m_id < b; });
	if ((m_stream_info.end() == pos) || ((*pos)->info.m_id != id))
		return;

	(*pos)->capture(buffer, samples_this_frame);
}



//============================================================
//  IMMNotificationClient implementation
//============================================================

HRESULT sound_wasapi::OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
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


HRESULT sound_wasapi::OnDeviceAdded(LPCWSTR pwstrDeviceId)
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

		// add it if it's available and has outputs or inputs
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


HRESULT sound_wasapi::OnDeviceRemoved(LPCWSTR pwstrDeviceId)
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


HRESULT sound_wasapi::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId)
{
	try
	{
		if (eMultimedia == role)
		{
			if (eRender == flow)
			{
				std::lock_guard device_lock(m_device_mutex);
				m_default_sink_id = pwstrDefaultDeviceId;
				auto const pos = find_device(m_default_sink_id);
				if ((m_device_info.end() != pos) && ((*pos)->device_id == m_default_sink_id) && (*pos)->info.m_sinks && ((*pos)->info.m_id != m_default_sink))
				{
					m_default_sink = (*pos)->info.m_id;

					++m_generation;
				}
			}
			else if (eCapture == flow)
			{
				std::lock_guard device_lock(m_device_mutex);
				m_default_source_id = pwstrDefaultDeviceId;
				auto const pos = find_device(m_default_source_id);
				if ((m_device_info.end() != pos) && ((*pos)->device_id == m_default_source_id) && (*pos)->info.m_sources && ((*pos)->info.m_id != m_default_source))
				{
					m_default_source = (*pos)->info.m_id;

					++m_generation;
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


HRESULT sound_wasapi::OnPropertyValueChanged(LPCWSTR pwstrDeviceId, PROPERTYKEY const key)
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
							"Sound: Error opening property store for audio device %s. Error: 0x%X\n",
							(*pos)->info.m_name,
							static_cast<unsigned int>(result));
					return result;
				}

				std::optional<std::string> name;
				result = get_string_property_value(*properties.Get(), PKEY_Device_FriendlyName, name);
				if (FAILED(result))
				{
					osd_printf_error(
							"Sound: Error getting updated display name for audio device %s. Error: 0x%X\n",
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

		return S_OK;
	}
	catch (std::bad_alloc const &)
	{
		return E_OUTOFMEMORY;
	}
}



//============================================================
//  IUnknown implementation
//============================================================

HRESULT sound_wasapi::QueryInterface(REFIID riid, void **ppvObject) noexcept
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



inline sound_wasapi::device_info_vector_iterator sound_wasapi::find_device(std::wstring_view device_id)
{
	return std::lower_bound(
			m_device_info.begin(),
			m_device_info.end(),
			device_id,
			[] (device_info_ptr const &a, std::wstring_view const &b) { return a->device_id < b; });
}


HRESULT sound_wasapi::add_device(device_info_vector_iterator pos, LPCWSTR device_id)
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
				static_cast<unsigned int>(result));
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
				static_cast<unsigned int>(result));
		return result;
	}
	if (DEVICE_STATE_ACTIVE != state)
		return S_OK;

	// add it if it has outputs or inputs
	return add_device(pos, device_id, std::move(device));
}


HRESULT sound_wasapi::add_device(device_info_vector_iterator pos, std::wstring_view device_id, mm_device_ptr &&device)
{
	std::wstring device_id_str;
	audio_info::node_info info;
	HRESULT const result = populate_audio_node_info(*device.Get(), device_id_str, info);
	if (FAILED(result))
		return result;
	assert(device_id == device_id_str);
	if (info.m_sinks && !info.m_sources)
		info.m_sources = info.m_sinks; // TODO: loopback event mode is only supported on Windows 10 1703 or later

	if ((0 < info.m_sinks) || (0 < info.m_sources))
	{
		try
		{
			device_info_ptr devinfo = std::make_unique<device_info>();
			info.m_id = m_next_node_id++;
			devinfo->device_id = std::move(device_id_str);
			devinfo->device = std::move(device);
			devinfo->info = std::move(info);
			if (devinfo->info.m_sinks)
			{
				if (!m_default_sink || (devinfo->device_id == m_default_sink_id))
					m_default_sink = devinfo->info.m_id;
			}
			else if (devinfo->info.m_sources)
			{
				if (!m_default_source || (devinfo->device_id == m_default_source_id))
					m_default_source = devinfo->info.m_id;
			}
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


HRESULT sound_wasapi::remove_device(device_info_vector_iterator pos)
{
	// if this was the default device, choose a new default arbitrarily
	if ((*pos)->info.m_id == m_default_sink)
		m_default_sink = 0;
	if ((*pos)->info.m_id == m_default_source)
		m_default_source = 0;
	for (auto it = m_device_info.begin(); (m_device_info.end() != it) && (!m_default_sink || !m_default_source); ++it)
	{
		if (it != pos)
		{
			if ((*it)->info.m_sinks)
			{
				if (!m_default_sink)
					m_default_sink = (*it)->info.m_id;
			}
			else if ((*it)->info.m_sources)
			{
				if (!m_default_source)
					m_default_source = (*it)->info.m_id;
			}
		}
	}

	{
		std::lock_guard stream_lock(m_stream_mutex);
		auto it = m_stream_info.begin();
		while (m_stream_info.end() != it)
		{
			if ((*it)->info.m_node == (*pos)->info.m_id)
			{
				stream_info_ptr stream = std::move(*it);
				it = m_stream_info.erase(it);
				{
					std::lock_guard cleanup_lock(m_cleanup_mutex);
					m_zombie_streams.emplace_back(std::move(stream));
				}
			}
			else
			{
				++it;
			}
		}
	}

	m_device_info.erase(pos);

	++m_generation;

	return S_OK;
}


void sound_wasapi::cleanup_task()
{
	// clean up removed streams on a separate thread
	std::unique_lock cleanup_lock(m_cleanup_mutex);
	while (!m_exiting || !m_zombie_streams.empty())
	{
		if (m_zombie_streams.empty())
			m_cleanup_condition.wait(cleanup_lock);

		while (!m_zombie_streams.empty())
		{
			stream_info_ptr stream(std::move(m_zombie_streams.back()));
			m_zombie_streams.pop_back();
			cleanup_lock.unlock();
			stream.reset();
			cleanup_lock.lock();
		}
	}
}

} // anonymous namespace

} // namespace osd


#else

namespace osd { namespace { MODULE_NOT_SUPPORTED(sound_wasapi, OSD_SOUND_PROVIDER, "wasapi") } }

#endif

MODULE_DEFINITION(SOUND_WASAPI, osd::sound_wasapi)
