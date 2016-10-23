// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#pragma once

#ifndef __K005289_H__
#define __K005289_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_K005289_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, K005289, _clock)
#define MCFG_K005289_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, K005289, _clock)


// ======================> k005289_device

class k005289_device : public device_t,
						public device_sound_interface
{
public:
	k005289_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~k005289_device() { }

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

public:
	void k005289_control_A_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void k005289_control_B_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ld1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ld2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tg1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tg2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

private:
	void make_mixer_table(int voices);

	required_region_ptr<uint8_t> m_sound_prom;
	sound_stream *m_stream;
	int m_rate;

	/* mixer tables and internal buffers */
	std::unique_ptr<int16_t[]> m_mixer_table;
	int16_t *m_mixer_lookup;
	std::unique_ptr<short[]> m_mixer_buffer;

	uint32_t m_counter[2];
	uint16_t m_frequency[2];
	uint16_t m_freq_latch[2];
	uint16_t m_waveform[2];
	uint8_t m_volume[2];
};

extern const device_type K005289;


#endif /* __K005289_H__ */
