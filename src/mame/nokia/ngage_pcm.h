// license:GPL-2.0+
// copyright-holders:Andrea Bogazzi
#ifndef MAME_NOKIA_NGAGE_PCM_H
#define MAME_NOKIA_NGAGE_PCM_H

#pragma once

#include <span>

class ngage_pcm_device : public device_t, public device_sound_interface
{
public:
	ngage_pcm_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	auto reply_ready() { return m_reply_ready.bind(); }
	void set_memory(address_space &memory) { m_memory = &memory; }
	void disconnect();
	void receive(std::span<u8 const> packet);
	static constexpr unsigned REPLY_WORDS = 7;
	std::span<u16 const> reply() const;
	void reply_consumed();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	static constexpr u32 STREAM_RATE = 16'000;
	static constexpr u8 NO_HALF = 2;
	void set_route(std::span<u8 const> packet);
	void make_reply(u8 transaction, u8 opcode, u16 value = 0);
	void service_reply();
	void start_half(u8 preferred);
	void stop();
	TIMER_CALLBACK_MEMBER(half_complete);

	devcb_write_line m_reply_ready;
	address_space *m_memory = nullptr;
	sound_stream *m_stream = nullptr;
	emu_timer *m_completion = nullptr;

	u16 m_reply[REPLY_WORDS]{};
	u8 m_media = 0;
	u8 m_device = 0;
	u8 m_object = 0;
	u8 m_dsp_device = 0;
	u8 m_dsp_object = 0;
	bool m_reply_pending = false;
	bool m_open = false;
	bool m_first = true;
	bool m_final_done = false;
	u8 m_requested = NO_HALF;
	u8 m_final = NO_HALF;
	u8 m_playing = NO_HALF;
	u8 m_free = 0;
	u16 m_capacity = 0;
	u16 m_final_frames = 0;
	u32 m_address[2]{};
	u16 m_frames[2]{};
	u32 m_output_position = 0;
	u8 m_repeat = 1;
};

DECLARE_DEVICE_TYPE(NGAGE_PCM, ngage_pcm_device)

#endif // MAME_NOKIA_NGAGE_PCM_H
