// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
/***************************************************************************

    ymz770.h

***************************************************************************/

#ifndef MAME_SOUND_YMZ770_H
#define MAME_SOUND_YMZ770_H

#pragma once

//**************************************************************************
//  CONSTANTS
//**************************************************************************

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_YMZ770_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, YMZ770, _clock)

#define MCFG_YMZ770_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, YMZ770, _clock)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward definition
class mpeg_audio;

// ======================> ymz770_device

class ymz770_device : public device_t, public device_sound_interface
{
public:
	// construction/destruction
	ymz770_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE8_MEMBER(write);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	void internal_reg_write(uint8_t reg, uint8_t data);

	sound_stream *m_stream;

	// data
	uint8_t m_cur_reg;
	uint8_t m_mute;         // mute chip
	uint8_t m_doen;         // digital output enable
	uint8_t m_vlma;         // overall AAM volume
	uint8_t m_bsl;          // boost level
	uint8_t m_cpl;          // clip limiter
	required_region_ptr<uint8_t> m_rom;

private:
	struct ymz_channel
	{
		uint8_t phrase;
		uint8_t pan;
		uint8_t volume;
		uint8_t control;

		bool is_playing, last_block;

		mpeg_audio *decoder;

		int16_t output_data[0x1000];
		int output_remaining;
		int output_ptr;
		int atbl;
		int pptr;

		uint8_t sequence;
		uint8_t seqcontrol;
		uint8_t seqdelay;
		uint8_t *seqdata;
		bool is_seq_playing;
	};

	ymz_channel m_channels[8];
};


// device type definition
DECLARE_DEVICE_TYPE(YMZ770, ymz770_device)

#endif // MAME_SOUND_YMZ770_H
