// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha
/*****************************************************************************
 *
 * dai_snd.h
 *
 ****************************************************************************/

#ifndef MAME_AUDIO_DAI_SND_H
#define MAME_AUDIO_DAI_SND_H


// ======================> dai_sound_device

class dai_sound_device : public device_t, public device_sound_interface
{
public:
	// construction/destruction
	dai_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	DECLARE_WRITE_LINE_MEMBER(set_input_ch0);
	DECLARE_WRITE_LINE_MEMBER(set_input_ch1);
	DECLARE_WRITE_LINE_MEMBER(set_input_ch2);
	DECLARE_WRITE8_MEMBER(set_volume);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	sound_stream *      m_mixer_channel;
	int                 m_dai_input[3];
	uint8_t             m_osc_volume[3];
	uint8_t             m_noise_volume;

	static const uint16_t s_osc_volume_table[];
	static const uint16_t s_noise_volume_table[];
};

DECLARE_DEVICE_TYPE(DAI_SOUND, dai_sound_device)

#endif // MAME_AUDIO_DAI_SND_H
