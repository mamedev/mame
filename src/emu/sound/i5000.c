/***************************************************************************

    i5000.c - Imagetek I5000 sound emulator
    
***************************************************************************/

#include "emu.h"
#include "i5000.h"


// device type definition
const device_type I5000_SND = &device_creator<i5000snd_device>;



i5000snd_device::i5000snd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, I5000_SND, "I5000", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
}


void i5000snd_device::device_start()
{
	// create the stream
	m_stream = machine().sound().stream_alloc(*this, 0, 2, 44100, this);

	m_rom_base = device().machine().region(":i5000snd")->base();
	m_rom_mask = device().machine().region(":i5000snd")->bytes() - 1;
}

void i5000snd_device::device_reset()
{
	for (int ch = 0; ch < 16; ch++)
	{
		m_channels[ch].is_playing = false;
	}
}


void i5000snd_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	for (int i = 0; i < samples; i++)
	{
		INT32 mix = 0;

		outputs[0][i] = mix;
		outputs[1][i] = mix;
	}
}


READ16_MEMBER( i5000snd_device::read )
{
	return 0;
}

WRITE16_MEMBER( i5000snd_device::write )
{
	if (mem_mask != 0xffff)
	{
		logerror("i5000snd: wrong mask %04X!\n", mem_mask);
		return;
	}
	m_stream->update();

	m_regs[offset] = data;
}

