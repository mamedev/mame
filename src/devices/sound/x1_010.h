// license:BSD-3-Clause
// copyright-holders:Luca Elia
#ifndef MAME_SOUND_X1_010_H
#define MAME_SOUND_X1_010_H

#pragma once

#include "dirom.h"

class x1_010_device : public device_t, public device_sound_interface, public device_rom_interface<20>
{
public:
	x1_010_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	u16 word_r(offs_t offset);
	void word_w(offs_t offset, u16 data);

	void enable_w(int data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

	// device_rom_interface overrides
	virtual void rom_bank_pre_change() override;

private:
	static constexpr unsigned NUM_CHANNELS = 16;

	// internal state

	/* Variables only used here */
	int m_rate;                              // Output sampling rate (Hz)
	sound_stream *  m_stream;                // Stream handle
	int m_sound_enable;                      // sound output enable/disable
	std::unique_ptr<u8[]>   m_reg;                 // X1-010 Register & wave form area
	std::unique_ptr<u8[]>   m_HI_WORD_BUF;         // X1-010 16bit access ram check avoidance work
	u32  m_smp_offset[NUM_CHANNELS];
	u32  m_env_offset[NUM_CHANNELS];

	u32 m_base_clock;
};

DECLARE_DEVICE_TYPE(X1_010, x1_010_device)

#endif // MAME_SOUND_X1_010_H
