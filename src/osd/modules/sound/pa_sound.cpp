// license:BSD-3-Clause
// copyright-holders:intealls, R.Belmont
/***************************************************************************

    pa_sound.c

    PortAudio interface.

*******************************************************************c********/

#include "sound_module.h"

#include "modules/osdmodule.h"

#ifndef NO_USE_PORTAUDIO

#include "modules/lib/osdobj_common.h"
#include "osdcore.h"

#include <portaudio.h>

#include <algorithm>
#include <atomic>
#include <climits>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include "pa_win_wasapi.h"
#endif


namespace osd {

namespace {

#define LOG_FILE   "pa.log"
#define LOG_BUFCNT 0

class sound_pa : public osd_module, public sound_module
{
public:
	sound_pa() : osd_module(OSD_SOUND_PROVIDER, "portaudio")
	{
	}
	virtual ~sound_pa() { }

	virtual int init(osd_interface &osd, osd_options const &options) override;
	virtual void exit() override;

	// sound_module

	virtual void stream_sink_update(uint32_t, const s16 *buffer, int samples_this_frame) override;

private:
	// Lock free SPSC ring buffer
	template <typename T>
	struct audio_buffer {
		T*               buf;
		int              size;
		int              reserve;
		std::atomic<int> playpos, writepos;

		audio_buffer(int size, int reserve) : size(size + reserve), reserve(reserve) {
			playpos = writepos = 0;
			buf = new T[this->size];
		}

		~audio_buffer() { delete[] buf; }

		int count() {
			int diff = writepos - playpos;
			return diff < 0 ? size + diff : diff;
		}

		void increment_writepos(int n) {
			writepos.store((writepos + n) % size);
		}

		int write(const T* src, int n) {
			n = std::min<int>(n, size - reserve - count());

			if (writepos + n > size) {
				memcpy(buf + writepos, src, sizeof(T) * (size - writepos));
				memcpy(buf, src + (size - writepos), sizeof(T) * (n - (size - writepos)));
			} else {
				memcpy(buf + writepos, src, sizeof(T) * n);
			}

			increment_writepos(n);

			return n;
		}

		void increment_playpos(int n) {
			playpos.store((playpos + n) % size);
		}

		int read(T* dst, int n) {
			n = std::min<int>(n, count());

			if (playpos + n > size) {
				std::memcpy(dst, buf + playpos, sizeof(T) * (size - playpos));
				std::memcpy(dst + (size - playpos), buf, sizeof(T) * (n - (size - playpos)));
			} else {
				std::memcpy(dst, buf + playpos, sizeof(T) * n);
			}

			increment_playpos(n);

			return n;
		}

		int clear(int n) {
			n = std::min<int>(n, size - reserve - count());

			if (writepos + n > size) {
				std::memset(buf + writepos, 0, sizeof(T) * (size - writepos));
				std::memset(buf, 0, sizeof(T) * (n - (size - writepos)));
			} else {
				std::memset(buf + writepos, 0, sizeof(T) * n);
			}

			increment_writepos(n);

			return n;
		}
	};

	enum
	{
		LATENCY_MIN = 0,
		LATENCY_MAX = 5,
	};

	int                 callback(s16* output_buffer, size_t number_of_frames);
	static int          _callback(const void*,
								  void *output_buffer,
								  unsigned long number_of_frames,
								  const PaStreamCallbackTimeInfo*,
								  PaStreamCallbackFlags,
								  void *arg) { return static_cast<sound_pa*> (arg)->
									callback((s16*) output_buffer, number_of_frames * 2); }

	PaDeviceIndex       list_get_devidx(const char* api_str, const char* device_str);

	PaStream*           m_pa_stream;
	PaError             err;

	int                 m_sample_rate;
	int                 m_audio_latency;

	audio_buffer<s16>*  m_ab;

	std::atomic<bool>   m_has_underflowed;
	std::atomic<bool>   m_has_overflowed;
	unsigned            m_underflows;
	unsigned            m_overflows;

	int                 m_skip_threshold; // this many samples in the buffer ~1 second in a row count as an overflow
	osd_ticks_t         m_osd_ticks;
	osd_ticks_t         m_skip_threshold_ticks;
	osd_ticks_t         m_osd_tps;
	int                 m_buffer_min_ct;

#if LOG_BUFCNT
	std::stringstream   m_log;
#endif
};

int sound_pa::init(osd_interface &osd, osd_options const &options)
{
	m_sample_rate = options.sample_rate();
	if (!m_sample_rate)
		return 0;

	PaStreamParameters   stream_params;
	const PaStreamInfo*  stream_info;
	const PaHostApiInfo* api_info;
	const PaDeviceInfo*  device_info;

	unsigned long        frames_per_callback = paFramesPerBufferUnspecified;
	double               callback_interval;

	m_underflows            = 0;
	m_overflows             = 0;
	m_has_overflowed        = false;
	m_has_underflowed       = false;
	m_osd_ticks             = 0;
	m_skip_threshold_ticks  = 0;
	m_osd_tps               = osd_ticks_per_second();
	m_buffer_min_ct         = INT_MAX;
	m_audio_latency         = std::clamp<int>(options.audio_latency(), LATENCY_MIN, LATENCY_MAX);

	try {
		m_ab = new audio_buffer<s16>(m_sample_rate, 2);
	} catch (std::bad_alloc&) {
		osd_printf_error("PortAudio: Unable to allocate audio buffer, sound is disabled\n");
		goto error;
	}

	err = Pa_Initialize();

	if (err != paNoError) goto pa_error;

	stream_params.device = list_get_devidx(options.pa_api(), options.pa_device());

	stream_params.channelCount = 2;
	stream_params.sampleFormat = paInt16;
	stream_params.hostApiSpecificStreamInfo = NULL;

	device_info = Pa_GetDeviceInfo(stream_params.device);

	// 0 = use default
	stream_params.suggestedLatency = options.pa_latency() ? options.pa_latency() : device_info->defaultLowOutputLatency;

#ifdef _WIN32
	PaWasapiStreamInfo wasapi_stream_info;

	// if requested latency is less than 20 ms, we need to use exclusive mode
	if (Pa_GetHostApiInfo(device_info->hostApi)->type == paWASAPI && stream_params.suggestedLatency < 0.020)
	{
		wasapi_stream_info.size = sizeof(PaWasapiStreamInfo);
		wasapi_stream_info.hostApiType = paWASAPI;
		wasapi_stream_info.flags = paWinWasapiExclusive;
		wasapi_stream_info.version = 1;

		stream_params.hostApiSpecificStreamInfo = &wasapi_stream_info;

		// for latencies lower than ~16 ms, we need to use event mode
		if (stream_params.suggestedLatency < 0.016)
		{
			// only way to control output latency with event mode
			frames_per_callback = stream_params.suggestedLatency * m_sample_rate;

			// needed for event mode to work
			stream_params.suggestedLatency = 0;
		}
	}
#endif

	err = Pa_OpenStream(&m_pa_stream,
						NULL,
						&stream_params,
						m_sample_rate,
						frames_per_callback,
						paClipOff,
						_callback,
						this);

	if (err != paNoError) goto pa_error;

	stream_info = Pa_GetStreamInfo(m_pa_stream);
	api_info = Pa_GetHostApiInfo(device_info->hostApi);

	// in milliseconds
	callback_interval = static_cast<double>(stream_info->outputLatency) * 1000.0;

	// clamp to a probable figure
	callback_interval = std::min<double>(callback_interval, 20.0);

	if (m_audio_latency == 0)
	{
		// very-low-latency mode (set audio_latency to 0); pa_latency controls allowable audio jitter
		m_skip_threshold = (options.pa_latency() ? options.pa_latency() : device_info->defaultLowOutputLatency) * m_sample_rate * 2 + 0.5f;
	}
	else
	{
		// set the best guess callback interval to allowed count, each audio_latency step > 1 adds 20 ms
		m_skip_threshold = ((std::max<double>(callback_interval, 10.0) + (m_audio_latency - 1) * 20.0) / 1000.0) * m_sample_rate * 2 + 0.5f;
	}

	osd_printf_verbose("PortAudio: Using device \"%s\" on API \"%s\"\n", device_info->name, api_info->name);
	osd_printf_verbose("PortAudio: Sample rate is %0.0f Hz, device output latency is %0.2f ms\n",
		stream_info->sampleRate, stream_info->outputLatency * 1000.0);
	osd_printf_verbose("PortAudio: Allowed additional buffering latency is %0.2f ms/%d frames\n",
		(m_skip_threshold / 2.0) / (m_sample_rate / 1000.0), m_skip_threshold / 2);

	err = Pa_StartStream(m_pa_stream);

	if (err != paNoError) goto pa_error;

	return 0;

pa_error:
	delete m_ab;
	osd_printf_error("PortAudio error: %s\n", Pa_GetErrorText(err));
	Pa_Terminate();
error:
	m_sample_rate = 0;
	return -1;
}

PaDeviceIndex sound_pa::list_get_devidx(const char* api_str, const char* device_str)
{
	PaDeviceIndex selected_devidx = -1;

	for (PaHostApiIndex api_idx = 0; api_idx < Pa_GetHostApiCount(); api_idx++)
	{
		const PaHostApiInfo *api_info = Pa_GetHostApiInfo(api_idx);

		osd_printf_verbose("PortAudio: API %s has %d devices\n", api_info->name, api_info->deviceCount);

		for (int api_devidx = 0; api_devidx < api_info->deviceCount; api_devidx++)
		{
			PaDeviceIndex devidx = Pa_HostApiDeviceIndexToDeviceIndex(api_idx, api_devidx);
			const PaDeviceInfo *device_info = Pa_GetDeviceInfo(devidx);

			// specified API and device is found
			if (!strcmp(api_str, api_info->name) && !strcmp(device_str, device_info->name))
				selected_devidx = devidx;

			// if specified device cannot be found, use the default device of the specified API
			if (!strcmp(api_str, api_info->name) && api_devidx == api_info->deviceCount - 1 && selected_devidx == -1)
				selected_devidx = api_info->defaultOutputDevice;

			osd_printf_verbose("PortAudio: %s: \"%s\"%s\n",
				api_info->name, device_info->name, api_info->defaultOutputDevice == devidx ? " (default)" : "");
		}
	}

	if (selected_devidx < 0)
	{
		osd_printf_verbose("PortAudio: Unable to find specified API or device or none set, reverting to default\n");
		return Pa_GetDefaultOutputDevice();
	}

	return selected_devidx;
}

int sound_pa::callback(s16* output_buffer, size_t number_of_samples)
{
	int buf_ct = m_ab->count();

	if (buf_ct >= number_of_samples)
	{
		m_ab->read(output_buffer, number_of_samples);

		// keep track of the minimum buffer count, skip samples adaptively to respect the audio_latency setting
		buf_ct -= number_of_samples;

		if (buf_ct < m_buffer_min_ct)
			m_buffer_min_ct = buf_ct;

		// if we are below the threshold, reset the counter
		if (buf_ct < m_skip_threshold)
			m_skip_threshold_ticks = m_osd_ticks;

		// if we have been above the set threshold for ~1 second, skip forward
		if (m_osd_ticks - m_skip_threshold_ticks > m_osd_tps)
		{
			if (m_audio_latency == 0)
			{
				// in very-low-latency mode, always skip forward the whole way
				// to prevent input from appearing delayed (due to sound cues getting delayed)
				m_ab->increment_playpos(m_buffer_min_ct);
				//osd_printf_verbose("PortAudio: skip ahead %d samples\n", m_buffer_min_ct);
				m_has_overflowed = true;
			}
			else
			{
				int adjust = m_buffer_min_ct - m_skip_threshold / 2;

				// if adjustment is less than two milliseconds, don't bother
				if (adjust / 2 > m_sample_rate / 500) {
					m_ab->increment_playpos(adjust);
					m_has_overflowed = true;
				}
			}

			m_skip_threshold_ticks = m_osd_ticks;
			m_buffer_min_ct = INT_MAX;
		}
	}
	else
	{
		m_ab->read(output_buffer, buf_ct);
		std::memset(output_buffer + buf_ct, 0, (number_of_samples - buf_ct) * sizeof(s16));

		// if stream_sink_update has been called, note the underflow
		if (m_osd_ticks)
			m_has_underflowed = true;

		m_skip_threshold_ticks = m_osd_ticks;
	}

	return paContinue;
}

void sound_pa::stream_sink_update(uint32_t, const s16 *buffer, int samples_this_frame)
{
	if (!m_sample_rate)
		return;

#if LOG_BUFCNT
	if (m_log.good())
		m_log << m_ab->count() << std::endl;
#endif

	if (m_has_underflowed)
	{
		m_underflows++;
		// add some silence to prevent immediate underflows
		m_ab->clear(m_skip_threshold / 4);
		m_has_underflowed = false;
	}

	if (m_has_overflowed)
	{
		m_overflows++;
		m_has_overflowed = false;
	}

	m_ab->write(buffer, samples_this_frame * 2);

	// for determining buffer overflows, take the sample here instead of in the callback
	m_osd_ticks = osd_ticks();
}

void sound_pa::exit()
{
	if (!m_sample_rate)
		return;

#if LOG_BUFCNT
	std::ofstream m_logfile(LOG_FILE);

	if (m_log.good() && m_logfile.is_open()) {
		m_logfile << m_log.str();
		m_logfile.close();
	}

	if (!m_log.good() || m_logfile.fail())
		osd_printf_error("PortAudio: Error writing log.\n");
#endif

	err = Pa_StopStream(m_pa_stream);  if (err != paNoError) goto error;
	err = Pa_CloseStream(m_pa_stream); if (err != paNoError) goto error;

	error:
	if (err != paNoError)
		osd_printf_error("PortAudio error: %s\n", Pa_GetErrorText(err));

	Pa_Terminate();

	delete m_ab;

	if (m_overflows || m_underflows)
		osd_printf_verbose("Sound: overflows=%d underflows=%d\n", m_overflows, m_underflows);
}

} // anonymous namespace

} // namespace osd

#else

namespace osd { namespace { MODULE_NOT_SUPPORTED(sound_pa, OSD_SOUND_PROVIDER, "portaudio") } }

#endif

MODULE_DEFINITION(SOUND_PORTAUDIO, osd::sound_pa)
