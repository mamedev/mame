// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    vrc6.h
    Konami VRC6 add-on sound

***************************************************************************/

#ifndef MAME_SOUND_VRC6_H
#define MAME_SOUND_VRC6_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vrc6snd_device

class vrc6snd_device : public device_t, public device_sound_interface
{
public:
	// construction/destruction
	vrc6snd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void write(offs_t offset, u8 data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	u8 m_freqctrl, m_pulsectrl[2], m_sawrate, m_master_freq;
	u8 m_pulsefrql[2], m_pulsefrqh[2], m_pulseduty[2];
	u8 m_sawfrql, m_sawfrqh, m_sawclock, m_sawaccum;
	u16 m_ticks[3];
	u8 m_output[3];

	sound_stream *m_stream;
};


// device type definition
DECLARE_DEVICE_TYPE(VRC6, vrc6snd_device)

#endif // MAME_SOUND_VRC6_H
