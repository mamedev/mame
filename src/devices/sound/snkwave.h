// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#pragma once

#ifndef __SNKWAVE_H__
#define __SNKWAVE_H__

#define SNKWAVE_WAVEFORM_LENGTH 16

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SNKWAVE_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, SNKWAVE, _clock)
#define MCFG_SNKWAVE_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, SNKWAVE, _clock)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> snkwave_device

class snkwave_device : public device_t,
						public device_sound_interface
{
public:
	snkwave_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~snkwave_device() { }

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

public:
	void snkwave_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

private:
	void update_waveform(unsigned int offset, uint8_t data);

private:
	sound_stream *m_stream;
	int m_external_clock;
	int m_sample_rate;

	// data about the sound system
	uint32_t m_frequency;
	uint32_t m_counter;
	int m_waveform_position;

	// decoded waveform table
	int16_t m_waveform[SNKWAVE_WAVEFORM_LENGTH];
};

extern const device_type SNKWAVE;


#endif /* __SNKWAVE_H__ */
