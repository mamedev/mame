// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#ifndef MAME_SOUND_K005289_H
#define MAME_SOUND_K005289_H

#pragma once


// ======================> k005289_device

class k005289_device : public device_t,
						public device_sound_interface
{
public:
	k005289_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE8_MEMBER( k005289_control_A_w );
	DECLARE_WRITE8_MEMBER( k005289_control_B_w );
	DECLARE_WRITE8_MEMBER( ld1_w );
	DECLARE_WRITE8_MEMBER( ld2_w );
	DECLARE_WRITE8_MEMBER( tg1_w );
	DECLARE_WRITE8_MEMBER( tg2_w );

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

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

DECLARE_DEVICE_TYPE(K005289, k005289_device)

#endif // MAME_SOUND_K005289_H
