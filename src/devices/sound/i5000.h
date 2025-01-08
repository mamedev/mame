// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

    i5000.h - Imagetek I5000 sound emulator

***************************************************************************/

#ifndef MAME_SOUND_I5000_H
#define MAME_SOUND_I5000_H

#pragma once

#include "sound/okiadpcm.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class i5000snd_device : public device_t,
						public device_sound_interface
{
public:
	// construction/destruction
	i5000snd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t read(offs_t offset);
	void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	sound_stream *m_stream;

private:
	struct channel_t
	{
		bool is_playing;
		oki_adpcm_state m_adpcm;

		uint32_t address;
		int freq_timer;
		int freq_base;
		int freq_min;
		uint16_t sample;
		uint8_t shift_pos;
		uint8_t shift_amount;
		uint8_t shift_mask;
		int vol_r;
		int vol_l;
		int output_r;
		int output_l;
	};

	channel_t m_channels[16];

	uint16_t m_regs[0x80];

	uint16_t *m_rom_base;
	uint32_t m_rom_mask;

	int m_lut_volume[0x100];

	bool read_sample(int ch);
	void write_reg16(uint8_t reg, uint16_t data);
};


// device type definition
DECLARE_DEVICE_TYPE(I5000_SND, i5000snd_device)

#endif // MAME_SOUND_I5000_H
