// license:BSD-3-Clause
// copyright-holders: Devin Acker

/***************************************************************************
    NEC/Casio uPD933 "Phase Distortion" synthesis chip
***************************************************************************/

#ifndef MAME_SOUND_UPD933_H
#define MAME_SOUND_UPD933_H

#pragma once

#include <array>

class upd933_device : public device_t, public device_sound_interface
{
public:
	upd933_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto irq_cb() { return m_irq_cb.bind(); }

	int rq_r();
	void cs_w(int state); // chip select, active low
	void id_w(int state); // irq disable, active low

	void write(u8 data);
	u8 read();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	static constexpr unsigned NOTE_SHIFT = 9;
	static constexpr unsigned PITCH_SHIFT = 20;
	static constexpr unsigned PITCH_FINE_SHIFT = 12;
	static constexpr unsigned VOLUME_SHIFT = 12;

	static constexpr unsigned ENV_DCA_SHIFT = 16;
	static constexpr unsigned ENV_DCW_SHIFT = 16;
	static constexpr unsigned ENV_DCO_SHIFT = 11;

	struct env_t
	{
		u8 m_direction = 0, m_sustain = 1, m_irq = 0;
		u32 m_rate = 0, m_target = 0, m_current = 0;

		void update();
		// calculate the next time this envelope generates an interrupt
		bool calc_timeout(unsigned &samples);
	};

	struct voice_t
	{
		u8 m_wave[2] = {0};
		u8 m_window = 0, m_ring_mod = 0, m_pitch_mod = 0, m_mute_other = 0;

		u16 m_pitch = 0;
		u32 m_position = 0, m_pitch_step = 0;
		u16 m_dcw_limit = 0;
		s16 m_pm_level = 0;
	};

	TIMER_CALLBACK_MEMBER(timer_tick);

	s16 update(int vnum);
	u8 irq_data();
	void update_pending_irq();
	void update_irq();

	u32 env_rate(u8 data) const;
	void update_pitch_step(int vnum);

	sound_stream *m_stream;
	static constexpr unsigned CLOCKS_PER_SAMPLE = 112;

	devcb_write_line m_irq_cb;
	u8 m_irq_pending, m_irq_state;
	u8 m_cs, m_id;
	emu_timer *m_irq_timer;

	u16 m_cosine[0x800];
	u32 m_pitch[0x80];
	u16 m_pitch_fine[0x200];
	u16 m_volume[0x200];

	u8 m_sound_data[2];
	u8 m_sound_data_pos;
	std::array<u16, 256> m_sound_regs;

	u32 m_sample_count;
	s16 m_last_sample;

	std::array<voice_t, 8> m_voice;
	std::array<env_t, 8> m_dca, m_dco, m_dcw;
};

DECLARE_DEVICE_TYPE(UPD933, upd933_device)

#endif // MAME_SOUND_UPD933_H
