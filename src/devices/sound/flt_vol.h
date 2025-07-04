// license:BSD-3-Clause
// copyright-holders:Derrick Renaud, Couriersud
#ifndef MAME_SOUND_FLT_VOL_H
#define MAME_SOUND_FLT_VOL_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> filter_volume_device

class filter_volume_device : public device_t, public device_sound_interface
{
public:
	filter_volume_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	filter_volume_device &set_gain(float gain); // also may be used in mcfg to set initial value (default is 1.0)
	float gain() { return m_gain; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	sound_stream* m_stream;
	float m_gain;
};

DECLARE_DEVICE_TYPE(FILTER_VOLUME, filter_volume_device)

#endif // MAME_SOUND_FLT_VOL_H
