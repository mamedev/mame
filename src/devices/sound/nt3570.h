// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
#ifndef MAME_SOUND_NT3570_H
#define MAME_SOUND_NT3570_H

#pragma once

#include "dirom.h"

// Novatek NT3570F. Register meanings are inferred from the NT3580 firmware.
class nt3570_device : public device_t, public device_sound_interface, public device_rom_interface<24>
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	nt3570_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto irq_callback() { return m_irq_cb.bind(); }

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream) override;

	// device_rom_interface implementation
	virtual void rom_bank_pre_change() override;

private:
	enum envelope_state : u8 { ENV_OFF, ENV_ATTACK, ENV_DECAY, ENV_SUSTAIN, ENV_RELEASE, ENV_BYPASS };
	static constexpr u32 ENV_ONE = 1U << 24;

	static u32 envelope_step(u8 rate);
	void envelope_start(unsigned ch);
	void envelope_advance(unsigned ch);
	void voice_stop(unsigned ch);
	void update_irq();
	void key_w(u8 data);
	void register_w(u8 reg, u8 data);
	TIMER_CALLBACK_MEMBER(service_tick);

	devcb_write_line m_irq_cb;
	sound_stream *m_stream;
	emu_timer *m_service_timer;
	u8 m_registers[256];
	u8 m_select;
	u16 m_enable;
	u16 m_pending;
	bool m_irq_state;
	attotime m_busy_until;
	bool m_active[16];
	bool m_streaming[16];
	bool m_last[16];
	bool m_next_valid[16];
	u16 m_page[16];
	u16 m_next_page[16];
	u32 m_length[16];
	u32 m_position[16]; // 16.16 position within the waveform or streamed page
	u8 m_envelope_state[16];
	u32 m_envelope_level[16]; // 0..ENV_ONE, linear gain in 0.24 format
};

DECLARE_DEVICE_TYPE(NT3570, nt3570_device)

#endif // MAME_SOUND_NT3570_H
