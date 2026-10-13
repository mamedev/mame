// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
#ifndef MAME_SOUND_ROLAND_RPP324_H
#define MAME_SOUND_ROLAND_RPP324_H

#pragma once

#include "roland_rpp324_codec.h"

class roland_rpp324_device : public device_t, public device_sound_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	roland_rpp324_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <typename T> void set_rom_tag(T &&tag) { m_rom.set_tag(std::forward<T>(tag)); }

	u16 host_r(offs_t offset);
	void host_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);
	u16 status_r();
	u16 control_r() { return m_control; }
	void control_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	static constexpr unsigned VOICES = 32;
	static constexpr unsigned SEQUENCES = 8;
	static constexpr unsigned QUEUE = 8;

	// a level that moves linearly to its target over a fade time
	struct ramp
	{
		double value = 127;
		double target = 127;
		double delta = 0;
		u32 remaining = 0;

		void set(u8 level, u8 fade);
		double tick();
	};

	struct voice
	{
		roland_rdac::decoder decoder;
		u16 phrase = 0;
		u16 queued[QUEUE]{};
		u8 queued_loops[QUEUE]{};
		u8 queued_late[QUEUE]{};
		u8 queue_count = 0;
		u8 loops = 1;
		u8 channels = 1;
		u8 step = 8;
		u8 phase = 0;
		u8 late = 0;
		s8 owner = -1;
		s8 sequence = -1;
		bool playing = false;
		bool paused = false;
		bool release = false;
		bool next_valid = false;
		s16 current[2]{};
		s16 next[2]{};
	};

	struct sequence
	{
		u32 start = 0;
		u32 cursor = 0;
		u32 end = 0;
		u32 delay = 0;
		u8 loops = 1;
		bool playing = false;
		bool paused = false;
		bool ended = false;
		bool release = false;
	};

	struct port
	{
		u8 source = 0;
		u8 config = 0x40;
		u8 time = 1;
		u32 pulse = 0;
	};

	required_region_ptr<u8> m_rom;
	sound_stream *m_stream;

	voice m_voice[VOICES];
	sequence m_sequence[SEQUENCES];
	port m_port[8];
	ramp m_gain[VOICES];
	ramp m_pan[VOICES];
	ramp m_route[VOICES][4];
	ramp m_master[2];
	ramp m_output[4];

	u16 m_lowcut[4];
	u16 m_highcut[4];
	u8 m_lowgain[4];
	u8 m_effect[VOICES];
	u8 m_total_effect;
	bool m_mix_c;
	bool m_mono_c;

	u16 m_control;
	u8 m_handshake;
	u8 m_packet[6];
	u8 m_packet_pos;
	u32 m_sequence_table;
	u32 m_sequence_count;

	static unsigned command_size(u8 command);
	void save_ramp(ramp &value, unsigned index) ATTR_COLD;
	void soft_reset();
	void command_byte(u8 data);
	void execute(const u8 *packet, int seq = -1);
	void reset_players();
	void play(unsigned ch, unsigned phrase, u8 loops, u8 late, bool immediate, int seq);
	bool start_voice(unsigned ch, unsigned phrase, u8 loops, u8 late, int seq);
	void stop_voice(unsigned ch);
	void finish_voice(unsigned ch);
	void play_sequence(unsigned id, unsigned number, u8 loops);
	void stop_sequence(unsigned id);
	void step_sequence(unsigned id);
	void event(unsigned source, bool start);
	bool playing(unsigned source) const;
};

DECLARE_DEVICE_TYPE(ROLAND_RPP324, roland_rpp324_device)

#endif // MAME_SOUND_ROLAND_RPP324_H
