// license:BSD-3-Clause
// copyright-holders:Devin Acker
/***************************************************************************
    Casio GT913 sound (HLE)

    This is the sound portion of the GT913.
    Up to 24 voices can be mixed into a 16-bit stereo serial bitstream,
    which is then input to either a serial DAC or a HG51B-based DSP,
    depending on the model of keyboard.

    Currently, the actual sample format in ROM is unknown.
    The serial output is twos-complement 16-bit PCM, but the data in ROM
    doesn't seem to be - reading it as such produces sounds that are
    somewhat recognizable, but highly distorted.

    For now, all known (and unknown) register writes are just logged
    without generating any sound.

***************************************************************************/

#include "emu.h"
#include "gt913_snd.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(GT913_SOUND_HLE, gt913_sound_hle_device, "gt913_sound_hle", "Casio GT913F sound (HLE)")

gt913_sound_hle_device::gt913_sound_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, GT913_SOUND_HLE, tag, owner, clock)
{
}

void gt913_sound_hle_device::device_start()
{
	save_item(NAME(m_gain));
	save_item(NAME(m_data));

	save_item(NAME(m_volume_target));
	save_item(NAME(m_volume_rate));
}

void gt913_sound_hle_device::device_reset()
{
	m_gain = 0;
	std::memset(m_data, 0, sizeof(m_data));
	std::memset(m_volume_target, 0, sizeof(m_volume_target));
	std::memset(m_volume_rate, 0, sizeof(m_volume_rate));
}

void gt913_sound_hle_device::data_w(offs_t offset, uint16_t data)
{
	assert(offset < 3);
	m_data[offset] = data;
}

uint16_t gt913_sound_hle_device::data_r(offs_t offset)
{
	assert(offset < 3);
	return m_data[offset];
}

void gt913_sound_hle_device::command_w(uint16_t data)
{
	uint8_t voicenum = (data & 0x1f00) >> 8;
	uint16_t voicecmd = data & 0x60ff;

	if (data == 0x0012)
	{
		uint8_t gain = m_data[0] & 0x3f;
		if (gain != m_gain)
			logerror("gain %u\n", gain);
		m_gain = gain;
	}
	else if (voicenum >= 24)
	{
		return;
	}
	else if (voicecmd == 0x0008)
	{
		/*
		Set the voice's sample start point as a ROM address.
		This is usually word-aligned, but not always
		(e.g. ctk551's lowest piano sample is at address 0x5a801)
		*/
		uint32_t samplestart = (m_data[1] | (m_data[2] << 16)) & 0xfffff;
		logerror("voice %u sample start 0x%06x\n", voicenum, samplestart);
	}
	else if (voicecmd == 0x0000)
	{
		/*
		Set the voice's sample end point as a ROM address.
		*/
		uint32_t sampleend = (m_data[0] | (m_data[1] << 16)) & 0xfffff;
		logerror("voice %u sample end   0x%06x\n", voicenum, sampleend);
	}
	else if (voicecmd == 0x2000)
	{
		/*
		Set the voice's sample loop point as a ROM address.
		*/
		uint32_t sampleloop = (m_data[0] | (m_data[1] << 16)) & 0xfffff;
		logerror("voice %u sample loop  0x%06x\n", voicenum, sampleloop);
	}
	else if (voicecmd == 0x200b)
	{
		/*
		Turn this voice on/off.
		ctk551 turns output off before assigning a new note or instrument to this voice,
		then turns output back on afterward
		*/
		logerror("voice %u output %s\n", voicenum, BIT(m_data[2], 7) ? "on" : "off");
	}
	else if (voicecmd == 0x4004)
	{
		/*
		Set this voice's panning, in the form of left and right volume levels (3 bits each)
		*/
		uint8_t left = (m_data[1] & 0xe0) >> 5;
		uint8_t right = (m_data[1] & 0x1c) >> 2;
		logerror("voice %u left %u right %u\n", voicenum, left, right);
	}
	else if (voicecmd == 0x4005)
	{
		/*
		Set the current pitch of this voice.
		The actual format of the value is unknown, but presumably some kind of fixed point
		*/
		uint32_t pitch = (m_data[0] << 8) | (m_data[1] >> 8);
		logerror("voice %u pitch 0x%06x\n", voicenum, pitch);
	}
	else if (voicecmd == 0x6007)
	{
		/*
		Raise or lower the volume to a specified level at a specified rate.
		The actual volume level is probably 7.8 fixed point or something like that, but this command
		only sets the most significant bits.
		*/
		logerror("voice %u volume %u rate %u\n", voicenum, (m_data[0] >> 8) & 0x7f, m_data[0] & 0xff);
		m_volume_target[voicenum] = m_data[0] & 0x7f00;
		m_volume_rate[voicenum] = m_data[0] & 0xff;
	}
	else if (voicecmd == 0x2028)
	{
		/*
		ctk551 issues this command and then reads the voice's current volume from data0
		to determine if it's time to start the next part of the volume envelope or not.
		For now, just return the "target" volume immediately
		(TODO: also figure out what it expects to be returned in data1)
		*/
		m_data[0] = m_volume_target[voicenum];
		m_data[1] = 0;
	}
	else
	{
		logerror("unknown sound write %04x (data: %04x %04x %04x)\n", data, m_data[0], m_data[1], m_data[2]);
	}
}

uint16_t gt913_sound_hle_device::status_r()
{
	/* ctk551 reads the current gain level out of the lower 6 bits and ignores the rest
	it's unknown what, if anything, the other bits are supposed to contain */
	return m_gain & 0x3f;
}
