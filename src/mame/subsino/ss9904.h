// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
// thanks-to:Guru
/***************************************************************************

    Subsino SS9804 / SS9904 4-channel sample player

***************************************************************************/

#ifndef MAME_SUBSINO_SS9904_H
#define MAME_SUBSINO_SS9904_H

#pragma once

#include "dirom.h"


class ss9904_device : public device_t, public device_sound_interface, public device_rom_interface<22>
{
public:
	ss9904_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// clock divider producing the sample rate (6144 measured on the 44.1 and 48 MHz boards, 4608 on Last Fighting's 32 MHz board)
	void set_divider(u32 divider) { m_divider = divider; }

	// force the ROM address scrambler on/off instead of taking it from the init command
	void set_scrambled(bool scrambled) { m_scramble_override = scrambled ? 1 : 0; }

	// host interface: single byte-wide port
	void write(u8 data);
	u8 read();

	// handshake: 1 while the chip is still absorbing the last command byte
	int busy_r() const { return m_busy ? 1 : 0; }

protected:
	ss9904_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 divider);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream) override;

	// device_rom_interface implementation
	virtual void rom_bank_pre_change() override;

private:
	struct voice
	{
		bool playing;   // busy bit
		u32 pos;        // current nibble address
		u32 remain;     // nibbles left
		u8 step;        // adaptive step (starts at 1)
		s32 acc;        // output accumulator
	};

	static const u8 s_scramble[256];
	static const u8 s_step_mul[8];

	sound_stream *m_stream;
	emu_timer *m_busy_timer;
	u32 m_divider;
	int m_scramble_override;
	bool m_scrambled;
	bool m_busy;
	u8 m_cmd;
	u8 m_args[4];
	u8 m_argcnt;
	u8 m_argneed;
	voice m_voice[4];

	TIMER_CALLBACK_MEMBER(busy_done);

	u8 rom_byte(u32 addr);
	void key_on(int ch, u8 page, u8 number);
	void key_off(u8 mask);
};


class ss9804_device : public ss9904_device
{
public:
	ss9804_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


DECLARE_DEVICE_TYPE(SS9904, ss9904_device)
DECLARE_DEVICE_TYPE(SS9804, ss9804_device)

#endif // MAME_SUBSINO_SS9904_H
