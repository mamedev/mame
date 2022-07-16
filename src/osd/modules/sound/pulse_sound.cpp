// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    pulse_sound.c

    PulseAudio interface.

*******************************************************************c********/

#include "sound_module.h"
#include "modules/osdmodule.h"

#ifndef NO_USE_PULSEAUDIO

#define GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>

#include <mutex>
#include <thread>
#include <pulse/pulseaudio.h>

#include "modules/lib/osdobj_common.h"

using osd::s16;
using osd::u32;

class sound_pulse : public osd_module, public sound_module
{
public:
	sound_pulse()
		: osd_module(OSD_SOUND_PROVIDER, "pulse"), sound_module()
	{
	}
	virtual ~sound_pulse() { }

	virtual int init(osd_options const &options) override;
	virtual void exit() override;
	virtual void update_audio_stream(bool is_throttled, const s16 *buffer, int samples_this_frame) override;
	virtual void set_mastervolume(int attenuation) override;

private:
	struct abuffer {
		size_t cpos;
		std::vector<u32> data;
	};

	std::thread *m_thread;
	pa_mainloop *m_mainloop;
	pa_context *m_context;
	pa_stream *m_stream;
	std::mutex m_mutex;

	std::vector<abuffer> m_buffers;

	u32 m_last_sample;
	int m_new_volume_value;
	bool m_setting_volume;
	bool m_new_volume;

	int m_pipe_to_sub[2];
	int m_pipe_to_main[2];

	static void i_volume_set_notify(pa_context *, int success, void *self);
	void volume_set_notify(int success);
	static void i_context_notify(pa_context *, void *self);
	void context_notify();
	static void i_stream_notify(pa_stream *, void *self);
	void stream_notify();
	static void i_stream_write_request(pa_stream *, size_t size, void *self);
	void stream_write_request(size_t size);

	void make_pipes();

	void mainloop_thread();
	void send_main(char c);
	void send_sub(char c);
	char get_main();
	char peek_main();
	void stop_mainloop(int err);
	void generic_error(const char *msg);
	void generic_pa_error(const char *msg, int err);
};

void sound_pulse::generic_error(const char *msg)
{
	perror(msg);
	::exit(1);
}

void sound_pulse::generic_pa_error(const char *msg, int err)
{
	fprintf(stderr, "%s: %s\n", msg, pa_strerror(err));
	::exit(1);
}

char sound_pulse::peek_main()
{
	char c;
	int err = read(m_pipe_to_main[0], &c, 1);
	if(err != 1) {
		if(err >= 0) {
			fprintf(stderr, "peek_main: read returned %d, that's supposedly impossible\n", err);
			::exit(1);
		}
		if(errno == EAGAIN || errno == EWOULDBLOCK) {
			// No data, no problem
			return -1;
		}
		generic_error("peek_main: read");
	}
	return c;
}

char sound_pulse::get_main()
{
	pollfd pfds[1];
	pfds[0].fd = m_pipe_to_main[0];
	pfds[0].events = POLL_IN;
	pfds[0].revents = 0;
	int err = poll(pfds, 1, -1);
	if(err < 0)
		generic_error("get_main: poll");

	char c;
	err = read(m_pipe_to_main[0], &c, 1);
	if(err != 1) {
		if(err >= 0) {
			fprintf(stderr, "get_main: read returned %d, that's supposedly impossible\n", err);
			::exit(1);
		}
		generic_error("get_main: read");
	}
	return c;
}

void sound_pulse::send_main(char c)
{
	int err = write(m_pipe_to_main[1], &c, 1);
	if(err != 1) {
		if(err >= 0) {
			fprintf(stderr, "send_main: write returned %d, that's supposedly impossible\n", err);
			::exit(1);
		}
		if(errno == EAGAIN || errno == EWOULDBLOCK) {
			fprintf(stderr, "send_main: write would block, pipe buffer overflowed, something is going very badly\n");
			::exit(1);
		}
		generic_error("send_main: write");
	}
}

void sound_pulse::send_sub(char c)
{
	int err = write(m_pipe_to_sub[1], &c, 1);
	if(err != 1) {
		if(err >= 0) {
			fprintf(stderr, "send_sub: write returned %d, that's supposedly impossible\n", err);
			::exit(1);
		}
		if(errno == EAGAIN || errno == EWOULDBLOCK) {
			fprintf(stderr, "send_sub: write would block, pipe buffer overflowed, something is going very badly\n");
			::exit(1);
		}
		generic_error("send_sub: write");
	}
}

void sound_pulse::make_pipes()
{
	if(pipe2(m_pipe_to_sub, O_NONBLOCK))
		generic_error("pipe2 pipe_to_sub");
	if(pipe2(m_pipe_to_main, O_NONBLOCK))
		generic_error("pipe2 pipe_to_main");
}

void sound_pulse::context_notify()
{
	pa_context_state state = pa_context_get_state(m_context);
	if(state == PA_CONTEXT_READY)
		send_main('r');

	else if(state == PA_CONTEXT_FAILED) {
		send_main('f');
		stop_mainloop(pa_context_errno(m_context));

	} else if(state == PA_CONTEXT_TERMINATED) {
		send_main('t');
		stop_mainloop(0);
	}
}

void sound_pulse::i_context_notify(pa_context *, void *self)
{
	static_cast<sound_pulse *>(self)->context_notify();
}

void sound_pulse::stream_notify()
{
	pa_stream_state state = pa_stream_get_state(m_stream);

	if(state == PA_STREAM_READY)
		send_main('r');

	else if(state == PA_STREAM_FAILED) {
		send_main('f');
		stop_mainloop(pa_context_errno(m_context));

	} else if(state == PA_STREAM_TERMINATED)
		pa_context_disconnect(m_context);
}

void sound_pulse::i_stream_notify(pa_stream *, void *self)
{
	static_cast<sound_pulse *>(self)->stream_notify();
}

void sound_pulse::stream_write_request(size_t size)
{
	if(size & 3) {
		fprintf(stderr, "stream request with size %d not a multiple of 4.\n", int(size));
		::exit(1);
	}
	size >>= 2;

	std::unique_lock<std::mutex> lock(m_mutex);
	while(size) {
		if(m_buffers.empty()) {
			std::vector<u32> zero(size, m_last_sample);
			int err = pa_stream_write(m_stream, zero.data(), size << 2, nullptr, 0, PA_SEEK_RELATIVE);
			if(err)
				generic_pa_error("stream write", err);
			size = 0;

		} else {
			auto &buf = m_buffers[0];
			size_t csz = size;
			size_t cur = buf.data.size() - buf.cpos;
			if(csz > cur)
				csz = cur;
			int err = pa_stream_write(m_stream, buf.data.data() + buf.cpos, csz << 2, nullptr, 0, PA_SEEK_RELATIVE);
			if(err)
				generic_pa_error("stream write", err);
			if(csz == cur)
				m_buffers.erase(m_buffers.begin());
			else
				buf.cpos += csz;
			size -= csz;
		}
	}
}

void sound_pulse::i_stream_write_request(pa_stream *, size_t size, void *self)
{
	static_cast<sound_pulse *>(self)->stream_write_request(size);
}


void sound_pulse::mainloop_thread()
{
	int err = 0;
	pa_mainloop_run(m_mainloop, &err);
	if(err)
		generic_pa_error("mainloop stopped", err);
}

void sound_pulse::stop_mainloop(int err)
{
	pa_mainloop_quit(m_mainloop, err);
}

int sound_pulse::init(osd_options const &options)
{
	m_last_sample = 0;
	m_setting_volume = false;
	m_new_volume = false;
	m_new_volume_value = 0;

	m_mainloop = pa_mainloop_new();
	m_context = pa_context_new(pa_mainloop_get_api(m_mainloop), "MAME");
	pa_context_set_state_callback(m_context, i_context_notify, this);
	int err = pa_context_connect(m_context, nullptr, PA_CONTEXT_NOFLAGS, nullptr);

	if(err)
		generic_pa_error("pa_connect", err);

	make_pipes();
	m_thread = new std::thread(&sound_pulse::mainloop_thread, this);

	char res = get_main();

	if(res != 'r')
		return 1;

	pa_sample_spec ss;
#ifdef LSB_FIRST
	ss.format = PA_SAMPLE_S16LE;
#else
	ss.format = PA_SAMPLE_S16BE;
#endif
	ss.rate = sample_rate();
	ss.channels = 2;
	m_stream = pa_stream_new(m_context, "main output", &ss, nullptr);
	pa_stream_set_state_callback(m_stream, i_stream_notify, this);
	pa_stream_set_write_callback(m_stream, i_stream_write_request, this);

	pa_buffer_attr battr;
	battr.fragsize = sample_rate() / 1000;
	battr.maxlength = uint32_t(-1);
	battr.minreq = sample_rate() / 1000;
	battr.prebuf = uint32_t(-1);
	battr.tlength = sample_rate() / 1000;

	err = pa_stream_connect_playback(m_stream, nullptr, &battr, PA_STREAM_ADJUST_LATENCY, nullptr, nullptr);
	if(err)
		generic_pa_error("stream connect playback", err);

	res = get_main();
	if(res != 'r')
		return 1;
	return 0;
}

void sound_pulse::update_audio_stream(bool is_throttled, const s16 *buffer, int samples_this_frame)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_buffers.resize(m_buffers.size() + 1);
	auto &buf = m_buffers.back();
	buf.cpos = 0;
	buf.data.resize(samples_this_frame);
	memcpy(buf.data.data(), buffer, samples_this_frame*4);
	m_last_sample = buf.data.back();

	if(m_buffers.size() > 10)
		// If there way too many buffers, drop some so only 10 are left (roughly 0.2s)
		m_buffers.erase(m_buffers.begin(), m_buffers.begin() + m_buffers.size() - 10);

	else if(m_buffers.size() >= 5)
		// If there are too many buffers, remove five sample per buffer
		// to slowly resync to reduce latency (4 seconds to
		// compensate one buffer roughly)
		buf.cpos = 5;
}

void sound_pulse::volume_set_notify(int success)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	if(m_new_volume) {
		m_new_volume = false;
		pa_cvolume vol;
		pa_cvolume_set(&vol, 2, pa_sw_volume_from_dB(m_new_volume_value));
		pa_context_set_sink_input_volume(m_context, pa_stream_get_index(m_stream), &vol, i_volume_set_notify, this);
	} else
		m_setting_volume = false;
}

void sound_pulse::i_volume_set_notify(pa_context *, int success, void *self)
{
	static_cast<sound_pulse *>(self)->volume_set_notify(success);
}

void sound_pulse::set_mastervolume(int attenuation)
{
	if(!m_stream)
		return;


	std::unique_lock<std::mutex> lock(m_mutex);
	if(m_setting_volume) {
		m_new_volume = true;
		m_new_volume_value = attenuation;
	} else {
		m_setting_volume = true;
		pa_cvolume vol;
		pa_cvolume_set(&vol, 2, pa_sw_volume_from_dB(attenuation));
		pa_context_set_sink_input_volume(m_context, pa_stream_get_index(m_stream), &vol, i_volume_set_notify, this);
	}
}

void sound_pulse::exit()
{
	if(!m_stream)
		return;

	pa_stream_disconnect(m_stream);
	while(get_main() != 't') {}
	pa_stream_unref(m_stream);
	pa_context_unref(m_context);
	m_thread->join();
	pa_mainloop_free(m_mainloop);
	delete m_thread;

	close(m_pipe_to_sub[0]);
	close(m_pipe_to_sub[1]);
	close(m_pipe_to_main[0]);
	close(m_pipe_to_main[1]);

	m_thread = nullptr;
	m_mainloop = nullptr;
	m_context = nullptr;
	m_stream = nullptr;
	m_buffers.clear();
}

#else
	MODULE_NOT_SUPPORTED(sound_pulse, OSD_SOUND_PROVIDER, "pulse")
#endif

MODULE_DEFINITION(SOUND_PULSEAUDIO, sound_pulse)
