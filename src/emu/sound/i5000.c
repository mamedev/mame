/***************************************************************************

    i5000.c - Imagetek I5000 sound emulator

    Imagetek I5000 is a multi-purpose chip, this covers the sound part.
    No official documentation is known to exist. It seems to be a simple
    16-channel ADPCM player.

    TODO:
    - Tokimeki Mahjong Paradise uses several different sample formats
      (4-bit ADPCM, 3-bit ADPCM, and unknown(s))
    - improve volume balance (is it really linear?)
    - verify that 4-bit ADPCM is the same as standard OKI ADPCM

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
	m_stream = machine().sound().stream_alloc(*this, 0, 2, clock() / 0x400, this);

	m_rom_base = device().machine().region(":i5000snd")->base();
	m_rom_mask = device().machine().region(":i5000snd")->bytes() - 1;
}


void i5000snd_device::device_reset()
{
	// stop playing
	write_reg16(0x43, 0xffff);
	
	// reset channel regs
	for (int i = 0; i < 0x40; i++)
		write_reg16(i, 0);
}


void i5000snd_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	for (int i = 0; i < samples; i++)
	{
		INT32 mix_l = 0;
		INT32 mix_r = 0;
		
		// loop over all channels
		for (int ch = 0; ch < 16; ch++)
		{
			if (!m_channels[ch].is_playing)
				continue;

			m_channels[ch].timer -= 0x100;
			if (m_channels[ch].timer > 0)
			{
				mix_r += m_channels[ch].output_r;
				mix_l += m_channels[ch].output_l;
				continue;
			}
			m_channels[ch].timer += m_channels[ch].freq;
			
			int data = m_rom_base[m_channels[ch].address >> 1 & m_rom_mask];
			
			// check sample end ID
			if (data == 0x7f && data == m_rom_base[((m_channels[ch].address >> 1) + 1) & m_rom_mask])
			{
				m_channels[ch].is_playing = false;
				continue;
			}

			// get amplitude for 4-bit adpcm
			if (m_channels[ch].address & 1) data >>= 4;
			m_channels[ch].address++;
			data = m_channels[ch].m_adpcm.clock(data & 0xf);
			
			m_channels[ch].output_r = data * m_channels[ch].vol_r;
			m_channels[ch].output_l = data * m_channels[ch].vol_l;
			mix_r += m_channels[ch].output_r;
			mix_l += m_channels[ch].output_l;
		}
		
		outputs[0][i] = mix_r / (16*8);
		outputs[1][i] = mix_l / (16*8);
	}
}


void i5000snd_device::write_reg16(UINT8 reg, UINT16 data)
{
	// channel regs
	if (reg < 0x40)
	{
		int ch = reg >> 2;
		switch (reg & 3)
		{
			// 0, 1: address
			
			// 2: frequency
			case 2:
				m_channels[ch].freq = (0x1ff - (data & 0xff)) << (~data >> 8 & 3);
				break;

			// 3: left/right volume
			case 3:
				m_channels[ch].vol_r = ~data & 0xff;
				m_channels[ch].vol_l = ~data >> 8 & 0xff;
				break;
			
			default:
				break;
		}
	}
	
	// global regs
	else
	{
		switch (reg)
		{
			// channel key on (0 has no effect)
			case 0x42:
				for (int ch = 0; ch < 16; ch++)
				{
					if (data & (1 << ch))
					{
						UINT32 address = (m_regs[ch << 2 | 1] << 16 | m_regs[ch << 2]) << 1;
						UINT16 start = m_rom_base[(address + 0) & m_rom_mask] << 8 | m_rom_base[(address + 1) & m_rom_mask];
//						UINT16 param = m_rom_base[(address + 2) & m_rom_mask] << 8 | m_rom_base[(address + 3) & m_rom_mask];
						
						// check sample start ID
						if (start != 0x7f7f)
						{
							logerror("i5000snd: channel %d wrong sample start ID %04X!\n", ch, start);
							continue;
						}
						
						m_channels[ch].address = (address + 4) << 1;

						m_channels[ch].is_playing = true;
						m_channels[ch].timer = 0;
						m_channels[ch].output_l = 0;
						m_channels[ch].output_r = 0;
						
						m_channels[ch].m_adpcm.reset();
					}
				}
				break;
			
			// channel key off (0 has no effect)
			case 0x43:
				for (int ch = 0; ch < 16; ch++)
				{
					if (data & (1 << ch))
					{
						m_channels[ch].is_playing = false;
					}
				}
				break;

			default:
				// not accessed often, assume that these are chip init registers
				// 0x40: ?
				// 0x41: ?
				// 0x45: ?
				// 0x46: ?
				break;
		}
	}
	
	m_regs[reg] = data;
}


READ16_MEMBER( i5000snd_device::read )
{
	UINT16 ret = 0;

	switch (offset)
	{
		// channel active state
		case 0x42:
			for (int ch = 0; ch < 16; ch++)
			{
				if (m_channels[ch].is_playing)
					ret |= (1 << ch);
			}
			break;
		
		default:
			// 0x41: ?
			break;
	}
	
	return ret;
}


WRITE16_MEMBER( i5000snd_device::write )
{
	if (mem_mask != 0xffff)
	{
		logerror("i5000snd: wrong mask %04X!\n", mem_mask);
		return;
	}
	m_stream->update();

	write_reg16(offset, data);
}

