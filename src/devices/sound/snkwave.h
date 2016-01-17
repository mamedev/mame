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
	snkwave_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~snkwave_device() { }

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

public:
	DECLARE_WRITE8_MEMBER( snkwave_w );

private:
	void update_waveform(unsigned int offset, UINT8 data);

private:
	sound_stream *m_stream;
	int m_external_clock;
	int m_sample_rate;

	// data about the sound system
	UINT32 m_frequency;
	UINT32 m_counter;
	int m_waveform_position;

	// decoded waveform table
	INT16 m_waveform[SNKWAVE_WAVEFORM_LENGTH];
};

extern const device_type SNKWAVE;


#endif /* __SNKWAVE_H__ */
