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

	void flt_volume_set_volume(float volume);

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	sound_stream* m_stream;
	float m_gain;
};

DECLARE_DEVICE_TYPE(FILTER_VOLUME, filter_volume_device)

#endif // MAME_SOUND_FLT_VOL_H
