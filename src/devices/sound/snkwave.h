// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_SOUND_SNKWAVE_H
#define MAME_SOUND_SNKWAVE_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> snkwave_device

class snkwave_device : public device_t,
						public device_sound_interface
{
public:
	snkwave_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void snkwave_w(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	static constexpr unsigned WAVEFORM_LENGTH = 16;
	static constexpr unsigned CLOCK_SHIFT = 8;

	void update_waveform(unsigned int offset, uint8_t data);

	sound_stream *m_stream;
	int m_external_clock;
	int m_sample_rate;

	// data about the sound system
	uint32_t m_frequency;
	uint32_t m_counter;
	int m_waveform_position;

	// decoded waveform table
	int16_t m_waveform[WAVEFORM_LENGTH];
};

DECLARE_DEVICE_TYPE(SNKWAVE, snkwave_device)

#endif // MAME_SOUND_SNKWAVE_H
