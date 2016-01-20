// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha
/*****************************************************************************
 *
 * dai_snd.h
 *
 ****************************************************************************/

#ifndef DAI_SND_H_
#define DAI_SND_H_

#include "emu.h"

// ======================> dai_sound_device

class dai_sound_device : public device_t,
							public device_sound_interface
{
public:
	// construction/destruction
	dai_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

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
	UINT8               m_osc_volume[3];
	UINT8               m_noise_volume;

	static const UINT16 s_osc_volume_table[];
	static const UINT16 s_noise_volume_table[];
};

extern const device_type DAI_SOUND;

#endif /* DAI_H_ */
