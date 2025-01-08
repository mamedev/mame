// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Alex W. Jackson
/*********************************************************

    Konami 053260 KDSC

*********************************************************/

#ifndef MAME_SOUND_K053260_H
#define MAME_SOUND_K053260_H

#pragma once

#include "dirom.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> k053260_device

class k053260_device : public device_t,
					   public device_sound_interface,
					   public device_rom_interface<21>
{
public:
	k053260_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	u8 main_read(offs_t offset);
	void main_write(offs_t offset, u8 data);
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	auto sh1_cb() { return m_sh1_cb.bind(); }
	auto sh2_cb() { return m_sh2_cb.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override;
	virtual void device_reset() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	// device_rom_interface overrides
	virtual void rom_bank_pre_change() override;

	TIMER_CALLBACK_MEMBER(update_state_outputs);

private:
	// Pan multipliers
	static const int pan_mul[8][2];

	// Sample hold lines callbacks (often used for interrupts)
	devcb_write_line m_sh1_cb;
	devcb_write_line m_sh2_cb;

	// configuration
	sound_stream *  m_stream;
	emu_timer   *m_timer;

	// live state
	u8           m_portdata[4];
	u8           m_keyon;
	u8           m_mode;
	int          m_timer_state;

	// per voice state
	class KDSC_Voice
	{
	public:
		KDSC_Voice(k053260_device &device) : m_device(device), m_pan_volume{ 0, 0 } { }

		inline void voice_start(int index);
		inline void voice_reset();
		inline void set_register(offs_t offset, u8 data);
		inline void set_loop(int state);
		inline void set_kadpcm(int state);
		inline void set_reverse(int state);
		inline void set_pan(u8 data);
		inline void update_pan_volume();
		inline void key_on();
		inline void key_off();
		inline void play(s32 *outputs);
		inline bool playing() { return m_playing; }
		inline u8 read_rom(bool side_effects);

	private:
		// pointer to owning device
		k053260_device &m_device;

		// live state
		u32  m_position;
		int  m_pan_volume[2];
		u16  m_counter;
		s8   m_output;
		bool m_playing;

		// per voice registers
		u32 m_start;
		u16 m_length;
		u16 m_pitch;
		u8  m_volume;

		// bit packed registers
		u8   m_pan;
		bool m_loop;
		bool m_kadpcm;
		bool m_reverse;
	} m_voice[4];
};

DECLARE_DEVICE_TYPE(K053260, k053260_device)

#endif // MAME_SOUND_K053260_H
