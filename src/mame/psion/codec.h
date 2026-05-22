// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Generic CODEC device for use in Psion machines

**********************************************************************/

#ifndef MAME_PSION_CODEC_H
#define MAME_PSION_CODEC_H

#pragma once


class psion_codec_device : public device_t, public device_sound_interface
{
public:
	psion_codec_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void pcm_in(uint8_t data);
	uint8_t pcm_out();

	void pdn_w(int state) { m_pwd = !state; }

protected:
	void device_start() override ATTR_COLD;
	void sound_stream_update(sound_stream &stream) override;

private:
	sound_stream *m_stream;
	int16_t m_audio_in;
	int16_t m_audio_out;
	bool m_pwd;
};


// device type declarations
DECLARE_DEVICE_TYPE(PSION_CODEC, psion_codec_device)

#endif // MAME_PSION_CODEC_H
