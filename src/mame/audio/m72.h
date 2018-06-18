// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    M72 audio interface

****************************************************************************/
#ifndef MAME_AUDIO_M72_H
#define MAME_AUDIO_M72_H

#pragma once

#include "sound/dac.h"

class m72_audio_device : public device_t, public device_sound_interface
{
public:
	m72_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	~m72_audio_device() {}

	DECLARE_READ8_MEMBER(sample_r);
	DECLARE_WRITE8_MEMBER(sample_w);

	/* the port goes to different address bits depending on the game */
	void set_sample_start(int start);
	DECLARE_WRITE8_MEMBER(vigilant_sample_addr_w);
	DECLARE_WRITE8_MEMBER(shisen_sample_addr_w);
	DECLARE_WRITE8_MEMBER(rtype2_sample_addr_w);
	DECLARE_WRITE8_MEMBER(poundfor_sample_addr_w);

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	// internal state
	uint32_t m_sample_addr;
	optional_region_ptr<uint8_t> m_samples;
	uint32_t m_samples_size;
	optional_device<dac_byte_interface> m_dac;
};

DECLARE_DEVICE_TYPE(IREM_M72_AUDIO, m72_audio_device)

#endif // MAME_AUDIO_M72_H
