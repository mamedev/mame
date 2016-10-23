// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    vrc6.h
    Konami VRC6 add-on sound

***************************************************************************/

#pragma once

#ifndef __VRC6_H__
#define __VRC6_H__

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_VRC6_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, VRC6, _clock)

#define MCFG_VRC6_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, VRC6, _clock)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vrc6snd_device

class vrc6snd_device : public device_t, public device_sound_interface
{
public:
	// construction/destruction
	vrc6snd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	uint8_t m_freqctrl, m_pulsectrl[2], m_sawrate;
	uint8_t m_pulsefrql[2], m_pulsefrqh[2], m_pulseduty[2];
	uint8_t m_sawfrql, m_sawfrqh, m_sawclock, m_sawaccum;
	uint16_t m_ticks[3];
	uint8_t m_output[3];

	sound_stream *m_stream;
};


// device type definition
extern const device_type VRC6;


#endif /* __VRC6_H__ */
