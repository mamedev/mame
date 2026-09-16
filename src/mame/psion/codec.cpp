// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Generic CODEC device for use in Psion machines

    This is a temporary device until Oki MSM7702/MSM7717 devices
    are emulated.

**********************************************************************/

#include "emu.h"
#include "codec.h"


DEFINE_DEVICE_TYPE(PSION_CODEC, psion_codec_device, "psion_codec", "Psion A-law CODEC")

psion_codec_device::psion_codec_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PSION_CODEC, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_audio_in(0)
	, m_audio_out(0)
{
}

void psion_codec_device::device_start()
{
	m_stream = stream_alloc(1, 1, SAMPLE_RATE_ADAPTIVE);
	m_pwd = false;

	save_item(NAME(m_pwd));
}

void psion_codec_device::sound_stream_update(sound_stream &stream)
{
	if (m_pwd) // power down
		return;

	// speaker output
	stream.fill(0, sound_stream::sample_t(m_audio_out) * (1.0 / 32768.0));

	// microphone input
	m_audio_in = stream.get(0, stream.samples() - 1) * 32768.0;
}

void psion_codec_device::pcm_in(uint8_t data)
{
	m_stream->update();

	// convert A-law value to linear PCM
	data ^= 0x55;

	int16_t pcm = (data & 0x0f) << 4;
	uint8_t seg = (data & 0x70) >> 4;

	switch (seg)
	{
	case 0:
		pcm += 0x008;
		break;
	case 1:
		pcm += 0x108;
		break;
	default:
		pcm += 0x108;
		pcm <<= (seg - 1);
		break;
	}

	m_audio_out = (data & 0x80) ? pcm : -pcm;
}

uint8_t psion_codec_device::pcm_out()
{
	m_stream->update();

	int16_t pcm = m_audio_in >> 3;
	uint8_t seg = 0;
	uint8_t mask = 0x55;

	// A-law using even bit inversion
	if (BIT(pcm, 12))
		pcm = ~pcm;
	else
		mask |= 0x80;

	// convert the scaled magnitude to segment number
	static const uint16_t seg_aend[8] = { 0x1f, 0x3f, 0x7f, 0xff, 0x1ff, 0x3ff, 0x7ff, 0xfff };

	for (int i = 0; i < 8; i++)
	{
		if ((pcm & 0xfff) <= seg_aend[i])
		{
			seg = i;
			break;
		}
	}

	// combine the sign, segment, and quantization bits
	uint8_t aval = seg << 4;
	if (seg < 2)
		aval |= (pcm >> 1) & 0x0f;
	else
		aval |= (pcm >> seg) & 0x0f;

	return (aval ^ mask);
}
