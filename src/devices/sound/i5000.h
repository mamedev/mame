// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

    i5000.h - Imagetek I5000 sound emulator

***************************************************************************/

#pragma once

#ifndef __I5000_H__
#define __I5000_H__

#include "sound/okiadpcm.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_I5000_SND_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, I5000_SND, _clock)

#define MCFG_I5000_SND_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, I5000_SND, _clock)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class i5000snd_device : public device_t,
						public device_sound_interface
{
public:
	// construction/destruction
	i5000snd_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	DECLARE_READ16_MEMBER(read);
	DECLARE_WRITE16_MEMBER(write);

	sound_stream *m_stream;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	struct channel_t
	{
		bool is_playing;
		oki_adpcm_state m_adpcm;

		UINT32 address;
		int freq_timer;
		int freq_base;
		int freq_min;
		UINT16 sample;
		UINT8 shift_pos;
		UINT8 shift_amount;
		UINT8 shift_mask;
		int vol_r;
		int vol_l;
		int output_r;
		int output_l;

	};

	channel_t m_channels[16];

	UINT16 m_regs[0x80];

	UINT16 *m_rom_base;
	UINT32 m_rom_mask;

	int m_lut_volume[0x100];

	bool read_sample(int ch);
	void write_reg16(UINT8 reg, UINT16 data);
};


// device type definition
extern const device_type I5000_SND;

#endif /* __I5000_H__ */
