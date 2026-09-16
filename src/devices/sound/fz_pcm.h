// license:BSD-3-Clause
// copyright-holders:Devin Acker
#ifndef MAME_SOUND_FZ_PCM_H
#define MAME_SOUND_FZ_PCM_H

#pragma once

#include "dimemory.h"

class fz_pcm_device : public device_t,
	public device_sound_interface,
	public device_memory_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	fz_pcm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_cb() { return m_irq_cb.bind(); }

	u16 gaa_r(offs_t offset);
	u16 gab_r(offs_t offset);

	void gaa_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);
	void gab_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	static constexpr unsigned CLOCKS_PER_SAMPLE = 384;
	static constexpr unsigned ADDR_FRAC_SHIFT = 13;

	struct voice_t
	{
		bool update();
		bool calc_timeout(u32 &samples) const;

		u16 m_flags = 0;

		u32 m_addr_start = 0, m_addr_end = 0;
		u32 m_loop_start = 0, m_loop_end = 0;
		u8  m_loop_start_fine = 0;
		u32 m_loop_len = 0;
		u8  m_loop_trace = 1;
		u16 m_loop_xfade = 0;

		u16 m_pitch = 0;
		u32 m_addr = 0, m_addr_frac = 0;
		s16 m_sample = 0, m_sample_last = 0;
	};

	enum
	{
		CMD_ADDR_START,
		CMD_ADDR_END,
		CMD_LOOP_START,
		CMD_LOOP_END,
		CMD_LOOP_LEN,
		CMD_LOOP_TRACE,
		CMD_LOOP_XFADE,
		CMD_PITCH,
		CMD_FLAG_SET,
		CMD_FLAG_CLR,
		CMD_GET_ADDR,
		CMD_GET_STATUS,
	};

	enum
	{
		FLAG_PLAY    = 0,
		FLAG_LOOP    = 1,
		FLAG_REVERSE = 3,
		FLAG_OUTPUT  = 4,
		FLAG_INT     = 5,
		FLAG_XFADE   = 6,
	};

	TIMER_CALLBACK_MEMBER(timer_tick);

	void update_pending_irq();
	void voice_cmd(unsigned cmd);

	sound_stream *m_stream;

	devcb_write_line m_irq_cb;
	emu_timer *m_irq_timer;

	address_space_config m_ram_config;
	memory_access<21, 1, -1, ENDIANNESS_LITTLE>::specific m_ram;

	u16 m_gaa_param[3];
	u16 m_gab_param[2];
	u16 m_gaa_cmd, m_gab_cmd;
	u8 m_voice_mask;

	u8 m_irq_stat;

	voice_t m_voices[8];
};

DECLARE_DEVICE_TYPE(FZ_PCM, fz_pcm_device)

#endif // MAME_SOUND_FZ_PCM_H
