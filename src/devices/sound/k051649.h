// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#pragma once

#ifndef __K051649_H__
#define __K051649_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_K051649_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, K051649, _clock)
#define MCFG_K051649_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, K051649, _clock)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// Parameters for a channel
struct k051649_sound_channel
{
	k051649_sound_channel() :
		counter(0),
		frequency(0),
		volume(0),
		key(0)
	{
		memset(waveram, 0, sizeof(signed char)*32);
	}

	unsigned long counter;
	int frequency;
	int volume;
	int key;
	signed char waveram[32];
};


// ======================> k051649_device

class k051649_device : public device_t,
						public device_sound_interface
{
public:
	k051649_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~k051649_device() { }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

public:
	void k051649_waveform_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t k051649_waveform_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void k051649_volume_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void k051649_frequency_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void k051649_keyonoff_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void k051649_test_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t k051649_test_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void k052539_waveform_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t k052539_waveform_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

private:
	void make_mixer_table(int voices);

private:
	k051649_sound_channel m_channel_list[5];

	/* global sound parameters */
	sound_stream *m_stream;
	int m_mclock;
	int m_rate;

	/* mixer tables and internal buffers */
	std::unique_ptr<int16_t[]> m_mixer_table;
	int16_t *m_mixer_lookup;
	std::unique_ptr<short[]> m_mixer_buffer;

	/* chip registers */
	uint8_t m_test;
};

extern const device_type K051649;


#endif /* __K051649_H__ */
