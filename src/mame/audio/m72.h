// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    M72 audio interface

****************************************************************************/
#ifndef MAME_AUDIO_M72_H
#define MAME_AUDIO_M72_H

#pragma once

#include "sound/dac.h"

class m72_audio_device : public device_t
{
public:
	m72_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	~m72_audio_device() {}

	// configuration
	void set_dac_tag(const char *tag) { m_dac.set_tag(tag); }
	void set_samples_tag(const char *tag) { m_samples.set_tag(tag); }

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

private:
	// internal state
	uint32_t m_sample_addr;
	optional_region_ptr<uint8_t> m_samples;
	optional_device<dac_byte_interface> m_dac;
};

DECLARE_DEVICE_TYPE(IREM_M72_AUDIO, m72_audio_device)

#define MCFG_IREM_M72_AUDIO_DAC(_tag) \
	downcast<m72_audio_device &>(*device).set_dac_tag(_tag);

#define MCFG_IREM_M72_AUDIO_SAMPLE(_tag) \
	downcast<m72_audio_device &>(*device).set_samples_tag(_tag);

#endif // MAME_AUDIO_M72_H
