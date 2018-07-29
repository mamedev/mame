// license:BSD-3-Clause
// copyright-holders:Paul Leaman, Miguel Angel Horna
/*********************************************************

    Capcom System QSound™ (HLE)

*********************************************************/
#ifndef MAME_SOUND_QSOUNDHLE_H
#define MAME_SOUND_QSOUNDHLE_H

#pragma once

#include "cpu/dsp16/dsp16.h"


class qsound_hle_device : public device_t, public device_sound_interface, public device_rom_interface
{
public:
	// default 60MHz clock (divided by 2 for DSP core clock, and then by 1248 for sample rate)
	qsound_hle_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 60'000'000);

	DECLARE_WRITE8_MEMBER(qsound_w);
	DECLARE_READ8_MEMBER(qsound_r);

protected:
	// device_t implementation
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	// device_rom_interface implementation
	virtual void rom_bank_updated() override;

private:
	// MAME resources
	sound_stream *m_stream;

	struct qsound_channel
	{
		u16 reg[8];    // channel control registers

		// work variables
		int lvol;           // left volume
		int rvol;           // right volume
	} m_channel[16];

	int m_pan_table[33];    // pan volume table
	u16 m_data;        // register latch data

	inline s16 read_sample(u32 offset) { return u16(read_byte(offset)) << 8; }
	void write_data(u8 address, u16 data);
};

DECLARE_DEVICE_TYPE(QSOUND_HLE, qsound_hle_device)

#endif // MAME_SOUND_QSOUNDHLE_H
