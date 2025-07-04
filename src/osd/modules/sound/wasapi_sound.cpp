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
#include "modules/lib/osdobj_common.h"
#include "osdcore.h"
#include "strconv.h"

// C++ standard library
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cmath>
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

#include <wrl/client.h>

// sound API headers
#include <audioclient.h>
#include <functiondiscoverykeys_devpkey.h>
#include <mmdeviceapi.h>
#include <mmreg.h>


namespace osd {

namespace {

using co_task_wave_format_ptr   = std::unique_ptr<WAVEFORMATEX, co_task_mem_deleter>;

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
		device_info() = default;
		device_info(device_info &&) = default;
		device_info &operator=(device_info &&) = default;

		std::wstring            device_id;
		mm_device_ptr           device;
		audio_info::node_info   info;
	};

	class stream_info
	{
	public:
		stream_info(
				sound_wasapi &host,
				audio_info::node_info const &node,
				uint32_t rate,
				audio_client_ptr client,
				audio_render_client_ptr render,
				UINT32 buffer_frames,
				handle_ptr &&event);
		stream_info(
				sound_wasapi &host,
				audio_info::node_info const &node,
				uint32_t rate,
				audio_client_ptr client,
				audio_capture_client_ptr capture,
				UINT32 buffer_frames,
				handle_ptr &&event);
		~stream_info();

		stream_info(stream_info const &) = delete;
		stream_info(stream_info &&) = delete;

		HRESULT start();
		void render(int16_t const *buffer, int samples_this_frame);
		void capture(int16_t *buffer, int samples_this_frame);

		audio_info::stream_info info;

	private:
		stream_info(
				sound_wasapi &host,
				audio_info::node_info const &node,
				audio_client_ptr client,
				UINT32 buffer_frames,
				handle_ptr &&event,
				uint32_t channels,
				uint32_t rate);

		void stop();
		void purge();
		void render_task();
		void capture_task();

		// order is important so things unwind properly
		sound_wasapi &              m_host;
		handle_ptr                  m_event;
		std::thread                 m_thread;
		CRITICAL_SECTION            m_critical_section;
		abuffer                     m_buffer;
		std::vector<int16_t>        m_underflow_fill;
		audio_client_ptr            m_client;
		audio_render_client_ptr     m_render;
		audio_capture_client_ptr    m_capture;
		UINT32                      m_buffer_frames;
		UINT32                      m_minimum_headroom;
		bool                        m_underflowing;
		std::atomic<bool>           m_exiting;
	};

	using device_info_vector = std::vector<device_info>;
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

	bool activate_audio_client(
			uint32_t node,
			std::string_view name,
			uint32_t rate,
			bool input,
			device_info_vector_iterator &device,
			audio_client_ptr &client,
			co_task_wave_format_ptr &mix_format,
			WAVEFORMATEXTENSIBLE &format);

	void housekeeping_task();

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
	std::vector<std::wstring>   m_updated_devices;
	std::condition_variable     m_housekeeping_condition;
	std::thread                 m_housekeeping_thread;
	std::mutex                  m_housekeeping_mutex;

	float                       m_audio_latency = 0.0F;
	uint32_t                    m_generation = 1;
	bool                        m_exiting = false;
};



//============================================================
//  stream_info
//============================================================

sound_wasapi::stream_info::stream_info(
		sound_wasapi &host,
		audio_info::node_info const &node,
		uint32_t rate,
		audio_client_ptr client,
		audio_render_client_ptr render,
		UINT32 buffer_frames,
		handle_ptr &&event) :
	stream_info(host, node, std::move(client), buffer_frames, std::move(event), node.m_sinks, rate)
{
	m_render = std::move(render);
}


sound_wasapi::stream_info::stream_info(
		sound_wasapi &host,
		audio_info::node_info const &node,
		uint32_t rate,
		audio_client_ptr client,
		audio_capture_client_ptr capture,
		UINT32 buffer_frames,
		handle_ptr &&event) :
	stream_info(host, node, std::move(client), buffer_frames, std::move(event), node.m_sources, rate)
{
	m_capture = std::move(capture);
}


sound_wasapi::stream_info::stream_info(
		sound_wasapi &host,
		audio_info::node_info const &node,
		audio_client_ptr client,
		UINT32 buffer_frames,
		handle_ptr &&event,
		uint32_t channels,
		uint32_t rate) :
	m_host(host),
	m_event(std::move(event)),
	m_buffer(channels),
	m_underflow_fill(channels, 0U),
	m_client(std::move(client)),
	m_buffer_frames(buffer_frames),
	m_minimum_headroom(std::max<UINT32>(lround(host.m_audio_latency * rate), buffer_frames)),
	m_underflowing(true),
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
		m_exiting.store(true, std::memory_order_release);
		SetEvent(m_event.get());
		LeaveCriticalSection(&m_critical_section);
		m_thread.join();
		m_exiting.store(false, std::memory_order_release);
	}
}


void sound_wasapi::stream_info::purge()
{
	m_exiting.store(true, std::memory_order_relaxed);
	LeaveCriticalSection(&m_critical_section);
	{
		std::unique_lock device_lock(m_host.m_device_mutex);
		std::unique_lock stream_lock(m_host.m_stream_mutex);

		auto const pos = std::find_if(
				m_host.m_stream_info.begin(),
				m_host.m_stream_info.end(),
				[this] (stream_info_ptr const &value) { return value.get() == this; });
		if (m_host.m_stream_info.end() != pos)
		{
			stream_info_ptr our_pointer = std::move(*pos);
			m_host.m_stream_info.erase(pos);

			++m_host.m_generation;

			stream_lock.unlock();
			device_lock.unlock();

			std::lock_guard housekeeping_lock(m_host.m_housekeeping_mutex);
			m_host.m_zombie_streams.emplace_back(std::move(our_pointer));
			m_host.m_housekeeping_condition.notify_one();
		}
	}
	EnterCriticalSection(&m_critical_section);
}


void sound_wasapi::stream_info::render_task()
{
	EnterCriticalSection(&m_critical_section);
	while (!m_exiting.load(std::memory_order_relaxed))
	{
		LeaveCriticalSection(&m_critical_section);
		WaitForSingleObject(m_event.get(), INFINITE);

		if (!m_exiting.load(std::memory_order_relaxed))
		{
			HRESULT result;

			UINT32 padding = 0;
			BYTE *data = nullptr;
			UINT32 locked = 0;
			result = m_client->GetCurrentPadding(&padding);
			EnterCriticalSection(&m_critical_section);

			if (SUCCEEDED(result))
			{
				locked = m_buffer_frames - padding;
				if (locked)
					result = m_render->GetBuffer(locked, &data);
			}

			if (SUCCEEDED(result) && locked)
			{
				auto const available = m_buffer.available();
				auto *samples = reinterpret_cast<int16_t *>(data);
				if (!m_underflowing)
				{
					m_buffer.get(samples, locked);
					if (available < locked)
					{
						m_underflowing = true;
						m_buffer.get(m_underflow_fill.data(), 1);
					}
				}
				else if (available >= m_minimum_headroom)
				{
					m_buffer.get(samples, locked);
					m_underflowing = false;
				}
				else
				{
					for (UINT32 i = 0; locked > i; ++i, samples += m_buffer.channels())
						std::copy_n(m_underflow_fill.data(), m_buffer.channels(), samples);
				}
				result = m_render->ReleaseBuffer(locked, 0);
			}

			if (AUDCLNT_E_DEVICE_INVALIDATED == result)
				purge();
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
	while (!m_exiting.load(std::memory_order_relaxed))
	{
		LeaveCriticalSection(&m_critical_section);
		WaitForSingleObject(m_event.get(), INFINITE);

		if (!m_exiting.load(std::memory_order_relaxed))
		{
			HRESULT result;
			do
			{
				BYTE *data = nullptr;
				UINT32 frames = 0;
				DWORD flags = 0;
				result = m_capture->GetBuffer(&data, &frames, &flags, nullptr, nullptr);

				if (SUCCEEDED(result))
				{
					if (AUDCLNT_S_BUFFER_EMPTY != result)
					{
						EnterCriticalSection(&m_critical_section);
						try
						{
							m_buffer.push(reinterpret_cast<int16_t const *>(data), frames);
						}
						catch (...)
						{
						}
						LeaveCriticalSection(&m_critical_section);
					}
					result = m_capture->ReleaseBuffer(frames);
				}

				if (AUDCLNT_E_DEVICE_INVALIDATED == result)
				{
					EnterCriticalSection(&m_critical_section);
					purge();
					LeaveCriticalSection(&m_critical_section);
				}
			}
			while (!m_exiting.load(std::memory_order_relaxed) && SUCCEEDED(result) && (AUDCLNT_S_BUFFER_EMPTY != result));
		}

		EnterCriticalSection(&m_critical_section);
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

	// get relevant options
	m_audio_latency = options.audio_latency() * 20e-3F;
	if (m_audio_latency == 0.0F)
		m_audio_latency = 0.03F;
	m_audio_latency = std::clamp(m_audio_latency, 0.01F, 1.0F);

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
				osd_printf_error(
						"Sound: Error getting default audio output device ID. Error: 0x%X\n",
						result);
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

		{
			m_default_source_id.clear();
			m_default_source = 0;
			co_task_wstr_ptr default_id;
			result = get_default_audio_device_id(*m_device_enum.Get(), eCapture, eMultimedia, default_id);
			if (HRESULT_FROM_WIN32(ERROR_NOT_FOUND) == result)
			{
				// no active input devices, hence no default
			}
			else if (FAILED(result) || !default_id)
			{
				// this is non-fatal, they just might get the wrong default input device
				osd_printf_error(
						"Sound: Error getting default audio input device ID. Error: 0x%X\n",
						result);
			}
			else
			{
				// reserve double the space to avoid allocations when the default device changes
				std::wstring_view default_id_str(default_id.get());
				m_default_source_id.reserve(default_id_str.length() * 2);
				m_default_source_id = default_id_str;
				osd_printf_verbose(
						"Sound: Got default input device ID: %s\n",
						osd::text::from_wstring(m_default_source_id));
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
					assert((m_device_info.end() == pos) || (pos->device_id != device_id));
					device_info devinfo;
					info.m_id = m_next_node_id++;
					devinfo.device_id = std::move(device_id);
					devinfo.device = std::move(dev);
					devinfo.info = std::move(info);

					osd_printf_verbose(
							"Sound: Found audio %sdevice %s (%s), assigned ID %u.\n",
							devinfo.info.m_sinks ? "output " : devinfo.info.m_sources ? "input " : "",
							devinfo.info.m_display_name,
							devinfo.info.m_name,
							devinfo.info.m_id);
					if (devinfo.info.m_sinks)
					{
						if (!m_default_sink || (devinfo.device_id == m_default_sink_id))
							m_default_sink = devinfo.info.m_id;
					}
					else if (devinfo.info.m_sources)
					{
						if (!m_default_source || (devinfo.device_id == m_default_source_id))
							m_default_source = devinfo.info.m_id;
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
	m_updated_devices.reserve(m_device_info.size() * 4); // hopefully avoid reallocations
	m_housekeeping_thread = std::thread([] (sound_wasapi *self) { self->housekeeping_task(); }, this);

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

	if (m_housekeeping_thread.joinable())
	{
		{
			std::lock_guard housekeeping_lock(m_housekeeping_mutex);
			m_exiting = true;
			m_housekeeping_condition.notify_all();
		}
		m_housekeeping_thread.join();
		m_exiting = false;
	}

	m_updated_devices.clear();
	m_zombie_streams.clear();
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
			result.m_nodes.emplace_back(device.info);

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

	try
	{
		// get common stuff
		device_info_vector_iterator device;
		audio_client_ptr client;
		co_task_wave_format_ptr mix;
		WAVEFORMATEXTENSIBLE format;
		if (!activate_audio_client(node, name, rate, false, device, client, mix, format))
			return 0;

		// need sample rate conversion if the sample rates don't match
		DWORD stream_flags = AUDCLNT_STREAMFLAGS_EVENTCALLBACK;
		if (format.Format.nSamplesPerSec != mix->nSamplesPerSec)
			stream_flags |= AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY;

		// initialise the audio client interface
		result = client->Initialize(
				AUDCLNT_SHAREMODE_SHARED,
				stream_flags,
				10 * 10'000, // 10 ms in 100 ns units - it'll give a bigger buffer
				0,
				&format.Format,
				nullptr);
		if (FAILED(result))
		{
			osd_printf_error(
					"Sound: Error initializing audio client interface for output stream %s on device %s. Error: 0x%X\n",
					name,
					device->info.m_display_name,
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
					device->info.m_display_name,
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
					device->info.m_display_name,
					result);
			return 0;
		}

		// create an event handle to receive notifications
		handle_ptr event(CreateEvent(nullptr, FALSE, FALSE, nullptr));
		if (!event)
		{
			DWORD const err = GetLastError();
			osd_printf_error(
					"Sound: Error creating event handle for output stream %s on device %s. Error: %u\n",
					name,
					device->info.m_display_name,
					err);
			return 0;
		}
		result = client->SetEventHandle(event.get());
		if (FAILED(result))
		{
			osd_printf_error(
					"Sound: Error setting event handle for output stream %s on device %s. Error: 0x%X\n",
					name,
					device->info.m_display_name,
					result);
			return 0;
		}

		// create a stream info structure
		stream_info_ptr stream = std::make_unique<stream_info>(
				*this,
				device->info,
				rate,
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
					device->info.m_display_name,
					result);
			return 0;
		}

		// make sure we have space to add it and clean it up before consuming an ID
		std::lock_guard stream_lock(m_stream_mutex);
		{
			std::lock_guard housekeeping_lock(m_housekeeping_mutex);
			m_zombie_streams.reserve(m_zombie_streams.size() + m_stream_info.size() + 1);
		}
		m_stream_info.reserve(m_stream_info.size() + 1);

		// now add it to the collection
		osd_printf_verbose("Sound: Created output stream %s on device %s.\n", name, device->info.m_display_name);
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

	try
	{
		// get common stuff
		device_info_vector_iterator device;
		audio_client_ptr client;
		co_task_wave_format_ptr mix;
		WAVEFORMATEXTENSIBLE format;
		if (!activate_audio_client(node, name, rate, true, device, client, mix, format))
			return 0;

		// need sample rate conversion if the sample rates don't match, need loopback for output
		DWORD stream_flags = AUDCLNT_STREAMFLAGS_EVENTCALLBACK;
		if (device->info.m_sinks)
			stream_flags |= AUDCLNT_STREAMFLAGS_LOOPBACK;
		if (format.Format.nSamplesPerSec != mix->nSamplesPerSec)
			stream_flags |= AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY;

		// initialise the audio client interface
		result = client->Initialize(
				AUDCLNT_SHAREMODE_SHARED,
				stream_flags,
				10 * 10'000, // 10 ms in units of 100 ns
				0,
				&format.Format,
				nullptr);
		if (FAILED(result))
		{
			osd_printf_error(
					"Sound: Error initializing audio client interface for input stream %s on device %s. Error: 0x%X\n",
					name,
					device->info.m_display_name,
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
					device->info.m_display_name,
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
					device->info.m_display_name,
					result);
			return 0;
		}

		// create an event handle to receive notifications
		handle_ptr event(CreateEvent(nullptr, FALSE, FALSE, nullptr));
		if (!event)
		{
			DWORD const err = GetLastError();
			osd_printf_error(
					"Sound: Error creating event handle for input stream %s on device %s. Error: %u\n",
					name,
					device->info.m_display_name,
					err);
			return 0;
		}
		result = client->SetEventHandle(event.get());
		if (FAILED(result))
		{
			osd_printf_error(
					"Sound: Error setting event handle for input stream %s on device %s. Error: 0x%X\n",
					name,
					device->info.m_display_name,
					result);
			return 0;
		}

		// create a stream info structure
		stream_info_ptr stream = std::make_unique<stream_info>(
				*this,
				device->info,
				rate,
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
					device->info.m_display_name,
					result);
			return 0;
		}

		// make sure we have space to add it and clean it up before consuming an ID
		std::lock_guard stream_lock(m_stream_mutex);
		{
			std::lock_guard housekeeping_lock(m_housekeeping_mutex);
			m_zombie_streams.reserve(m_zombie_streams.size() + m_stream_info.size() + 1);
		}
		m_stream_info.reserve(m_stream_info.size() + 1);

		// try to add it to the collection
		osd_printf_verbose("Sound: Created input stream %s on device %s.\n", name, device->info.m_display_name);
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
			if ((m_device_info.end() == pos) || (pos->device_id != device_id))
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
			if ((m_device_info.end() != pos) && (pos->device_id == device_id))
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
		if ((m_device_info.end() != pos) && (pos->device_id == device_id))
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
		if ((m_device_info.end() != pos) && (pos->device_id == device_id))
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

			if (eRender == flow)
			{
				std::lock_guard device_lock(m_device_mutex);
				if (m_default_sink_id != default_id)
				{
					m_default_sink_id = default_id;
					auto const pos = find_device(m_default_sink_id);
					if ((m_device_info.end() != pos) && (pos->device_id == m_default_sink_id) && pos->info.m_sinks && (pos->info.m_id != m_default_sink))
					{
						try
						{
							osd_printf_verbose(
									"Sound: Default output device changed to %s.\n",
									pos->info.m_display_name);
						}
						catch (std::bad_alloc const &)
						{
						}
						m_default_sink = pos->info.m_id;

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
			else if (eCapture == flow)
			{
				std::lock_guard device_lock(m_device_mutex);
				if (m_default_source_id != default_id)
				{
					m_default_source_id = default_id;
					auto const pos = find_device(m_default_source_id);
					if ((m_device_info.end() != pos) && (pos->device_id == m_default_source_id) && pos->info.m_sources && (pos->info.m_id != m_default_source))
					{
						try
						{
							osd_printf_verbose(
									"Sound: Default input device changed to %s.\n",
									pos->info.m_display_name);
						}
						catch (std::bad_alloc const &)
						{
						}
						m_default_source = pos->info.m_id;

						++m_generation;
					}
					else
					{
						try
						{
							osd_printf_verbose(
									"Sound: Default input device changed ID changed to %s (not found).\n",
									osd::text::from_wstring(m_default_sink_id));
						}
						catch (std::bad_alloc const &)
						{
						}
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


HRESULT sound_wasapi::OnPropertyValueChanged(LPCWSTR pwstrDeviceId, PROPERTYKEY const key)
{
	try
	{
		// don't acquire the device lock until we know the callback won't block WASAPI
		if (PKEY_Device_FriendlyName == key)
		{
			std::lock_guard device_lock(m_device_mutex);
			std::wstring_view device_id(pwstrDeviceId);
			auto const pos = find_device(device_id);

			if ((m_device_info.end() != pos) && (pos->device_id == device_id))
			{
				HRESULT result;

				property_store_ptr properties;
				result = pos->device->OpenPropertyStore(STGM_READ, properties.GetAddressOf());
				if (FAILED(result))
				{
					osd_printf_error(
							"Sound: Error opening property store for audio device %s. Error: 0x%X\n",
							pos->info.m_display_name,
							static_cast<unsigned int>(result));
					return result;
				}

				std::optional<std::string> name;
				result = get_string_property_value(*properties.Get(), PKEY_Device_FriendlyName, name);
				if (FAILED(result))
				{
					osd_printf_error(
							"Sound: Error getting updated display name for audio device %s. Error: 0x%X\n",
							pos->info.m_display_name,
							static_cast<unsigned int>(result));
					return result;
				}

				if (name)
				{
					pos->info.m_display_name = std::move(*name);

					++m_generation;
				}
			}
		}
		else if (PKEY_AudioEngine_DeviceFormat == key)
		{
			// not safe to lock the device mutex here at all
			std::wstring device_id(pwstrDeviceId);
			std::lock_guard housekeeping_lock(m_housekeeping_mutex);
			m_updated_devices.emplace_back(std::move(device_id));
			m_housekeeping_condition.notify_one();
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
			[] (device_info const &a, std::wstring_view const &b) { return a.device_id < b; });
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
			device_info devinfo;
			info.m_id = m_next_node_id++;
			devinfo.device_id = std::move(device_id_str);
			devinfo.device = std::move(device);
			devinfo.info = std::move(info);
			if (devinfo.info.m_sinks)
			{
				if (!m_default_sink || (devinfo.device_id == m_default_sink_id))
					m_default_sink = devinfo.info.m_id;
			}
			else if (devinfo.info.m_sources)
			{
				if (!m_default_source || (devinfo.device_id == m_default_source_id))
					m_default_source = devinfo.info.m_id;
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
	if (pos->info.m_id == m_default_sink)
		m_default_sink = 0;
	if (pos->info.m_id == m_default_source)
		m_default_source = 0;
	for (auto it = m_device_info.begin(); (m_device_info.end() != it) && (!m_default_sink || !m_default_source); ++it)
	{
		if (it != pos)
		{
			if (it->info.m_sinks)
			{
				if (!m_default_sink)
					m_default_sink = it->info.m_id;
			}
			else if (it->info.m_sources)
			{
				if (!m_default_source)
					m_default_source = it->info.m_id;
			}
		}
	}

	// clean up any streams associated with this device
	bool purged_streams = false;
	{
		std::lock_guard stream_lock(m_stream_mutex);
		auto it = m_stream_info.begin();
		while (m_stream_info.end() != it)
		{
			if ((*it)->info.m_node == pos->info.m_id)
			{
				purged_streams = true;
				stream_info_ptr stream = std::move(*it);
				it = m_stream_info.erase(it);
				{
					std::lock_guard housekeeping_lock(m_housekeeping_mutex);
					m_zombie_streams.emplace_back(std::move(stream));
				}
			}
			else
			{
				++it;
			}
		}
	}
	if (purged_streams)
	{
		std::lock_guard housekeeping_lock(m_housekeeping_mutex);
		m_housekeeping_condition.notify_one();
	}

	m_device_info.erase(pos);

	++m_generation;

	return S_OK;
}


bool sound_wasapi::activate_audio_client(
		uint32_t node,
		std::string_view name,
		uint32_t rate,
		bool input,
		device_info_vector_iterator &device,
		audio_client_ptr &client,
		co_task_wave_format_ptr &mix_format,
		WAVEFORMATEXTENSIBLE &format)
{
	HRESULT result;

	// find the requested device
	device = std::find_if(
			m_device_info.begin(),
			m_device_info.end(),
			[node] (device_info const &value) { return value.info.m_id == node; });

	// always check for stale argument values
	if (m_device_info.end() == device)
	{
		osd_printf_verbose(
				"Sound: Attempt to open audio %s stream %s on unknown node %u.\n",
				input ? "input" : "output",
				name,
				node);
		return false;
	}

	// make sure it supports the requested direction
	if (input ? !device->info.m_sources : !device->info.m_sinks)
	{
		osd_printf_error(
				"Sound: Attempt to open audio %s stream %s on %s device %s.\n",
				input ? "input" : "output",
				name,
				input ? "output" : "input",
				device->info.m_display_name);
		return false;
	}

	// create an audio client interface
	result = device->device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, &client);
	if (FAILED(result) || !client)
	{
		osd_printf_error(
				"Sound: Error activating audio client interface for %s stream %s on device %s. Error: 0x%X\n",
				input ? "input" : "output",
				name,
				device->info.m_display_name,
				result);
		return false;
	}

	// get the mix format for the endpoint
	WAVEFORMATEX *mix_raw = nullptr;
	result = client->GetMixFormat(&mix_raw);
	mix_format.reset(std::exchange(mix_raw, nullptr));
	if (FAILED(result) || !mix_format)
	{
		osd_printf_error(
				"Sound: Error getting mix format for %s stream %s on device %s. Error: 0x%X\n",
				input ? "input" : "output",
				name,
				device->info.m_display_name,
				result);
		return false;
	}

	// set up desired format
	std::optional<DWORD> positions;
	if (WAVE_FORMAT_EXTENSIBLE == mix_format->wFormatTag)
		positions = reinterpret_cast<WAVEFORMATEXTENSIBLE const *>(mix_format.get())->dwChannelMask;
	populate_wave_format(
			format,
			input ? device->info.m_sources : device->info.m_sinks,
			rate,
			positions);

	return true;
}


void sound_wasapi::housekeeping_task()
{
	// clean up removed streams on a separate thread
	std::unique_lock housekeeping_lock(m_housekeeping_mutex);
	while (!m_exiting || !m_zombie_streams.empty())
	{
		if (m_zombie_streams.empty() && m_updated_devices.empty())
			m_housekeeping_condition.wait(housekeeping_lock);

		while (!m_exiting && !m_updated_devices.empty())
		{
			std::wstring const device_id(std::move(m_updated_devices.front()));
			m_updated_devices.erase(m_updated_devices.begin());
			for (auto it = m_updated_devices.begin(); m_updated_devices.end() != it; )
			{
				if (device_id == *it)
					it = m_updated_devices.erase(it);
				else
					++it;
			}
			housekeeping_lock.unlock();
			{
				std::lock_guard device_lock(m_device_mutex);
				auto const pos = find_device(device_id);
				if ((m_device_info.end() != pos) && (pos->device_id == device_id))
				{
					// if we don't purge the streams ourselves, they'll get fatal errors anyway
					remove_device(pos);
					add_device(find_device(device_id), device_id.c_str());
				}
				else
				{
					// deal with devices added with the format property empty and updated later
					add_device(pos, device_id.c_str());
				}
			}
			housekeeping_lock.lock();
		}

		while (!m_zombie_streams.empty())
		{
			stream_info_ptr stream(std::move(m_zombie_streams.back()));
			m_zombie_streams.pop_back();
			housekeeping_lock.unlock();
			stream.reset();
			housekeeping_lock.lock();
		}
	}
}

} // anonymous namespace

} // namespace osd

#else

namespace osd { namespace { MODULE_NOT_SUPPORTED(sound_wasapi, OSD_SOUND_PROVIDER, "wasapi") } }

#endif

MODULE_DEFINITION(SOUND_WASAPI, osd::sound_wasapi)
