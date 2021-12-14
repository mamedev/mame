// license:BSD-3-Clause
// copyright-holders: Devin Acker
/***************************************************************************
    Casio GT913 sound (HLE)
***************************************************************************/

#ifndef MAME_AUDIO_GT913_H
#define MAME_AUDIO_GT913_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> gt913_sound_hle_device

class gt913_sound_hle_device : public device_t
{
public:
	// construction/destruction
	gt913_sound_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void data_w(offs_t offset, uint16_t data);
	uint16_t data_r(offs_t offset);
	void command_w(uint16_t data);
	uint16_t status_r();

protected:
	// device_t overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint8_t m_gain;
	uint16_t m_data[3];

	uint16_t m_volume_target[24];
	uint8_t m_volume_rate[24];
};

// device type definition
DECLARE_DEVICE_TYPE(GT913_SOUND_HLE, gt913_sound_hle_device)

#endif // MAME_AUDIO_GT913_H
