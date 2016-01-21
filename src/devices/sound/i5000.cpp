// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

    i5000.c - Imagetek I5000 sound emulator

    Imagetek I5000 is a multi-purpose chip, this covers the sound part.
    No official documentation is known to exist. It seems to be a simple
    16-channel ADPCM player.

    TODO:
    - verify that ADPCM is the same as standard OKI ADPCM
    - verify volume balance
    - sample command 0x0007
    - any more sound formats than 3-bit and 4-bit ADPCM?

***************************************************************************/

#include "emu.h"
#include "i5000.h"


// device type definition
const device_type I5000_SND = &device_creator<i5000snd_device>;

i5000snd_device::i5000snd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, I5000_SND, "I5000", tag, owner, clock, "i5000snd", __FILE__),
		device_sound_interface(mconfig, *this), m_stream(nullptr), m_rom_base(nullptr), m_rom_mask(0)
{
}


void i5000snd_device::device_start()
{
	// fill volume table
	double div = 1.032;
	double vol = 2047.0;
	for (auto & elem : m_lut_volume)
	{
		elem = vol + 0.5;
		vol /= div;
	}
	m_lut_volume[0xff] = 0;

	// create the stream
	m_stream = machine().sound().stream_alloc(*this, 0, 2, clock() / 0x400);

	m_rom_base = (UINT16 *)device().machine().root_device().memregion(":i5000snd")->base();
	m_rom_mask = device().machine().root_device().memregion(":i5000snd")->bytes() / 2 - 1;

	// register for savestates
	for (int ch = 0; ch < 16; ch++)
	{
		save_item(NAME(m_channels[ch].is_playing), ch);
		save_item(NAME(m_channels[ch].m_adpcm.m_signal), ch);
		save_item(NAME(m_channels[ch].m_adpcm.m_step), ch);

		save_item(NAME(m_channels[ch].address), ch);
		save_item(NAME(m_channels[ch].freq_timer), ch);
		save_item(NAME(m_channels[ch].freq_base), ch);
		save_item(NAME(m_channels[ch].freq_min), ch);
		save_item(NAME(m_channels[ch].sample), ch);
		save_item(NAME(m_channels[ch].shift_pos), ch);
		save_item(NAME(m_channels[ch].shift_amount), ch);
		save_item(NAME(m_channels[ch].shift_mask), ch);
		save_item(NAME(m_channels[ch].vol_r), ch);
		save_item(NAME(m_channels[ch].vol_l), ch);
		save_item(NAME(m_channels[ch].output_r), ch);
		save_item(NAME(m_channels[ch].output_l), ch);
	}

	save_item(NAME(m_regs));
}


void i5000snd_device::device_reset()
{
	// stop playing
	write_reg16(0x43, 0xffff);

	// reset channel regs
	for (int i = 0; i < 0x40; i++)
		write_reg16(i, 0);
}


bool i5000snd_device::read_sample(int ch)
{
	m_channels[ch].shift_pos &= 0xf;
	m_channels[ch].sample = m_rom_base[m_channels[ch].address];
	m_channels[ch].address = (m_channels[ch].address + 1) & m_rom_mask;

	// handle command
	if (m_channels[ch].sample == 0x7f7f)
	{
		UINT16 cmd = m_rom_base[m_channels[ch].address];
		m_channels[ch].address = (m_channels[ch].address + 1) & m_rom_mask;

		// volume envelope? or loop sample?
		if ((cmd & 0x00ff) == 0x0007)
		{
			// TODO
			return false;
		}

		// cmd 0x0000 = end sample
		// other values: unused
		else return false;

	}

	return true;
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

			m_channels[ch].freq_timer -= m_channels[ch].freq_min;
			if (m_channels[ch].freq_timer > 0)
			{
				mix_r += m_channels[ch].output_r;
				mix_l += m_channels[ch].output_l;
				continue;
			}
			m_channels[ch].freq_timer += m_channels[ch].freq_base;

			int adpcm_data = m_channels[ch].sample >> m_channels[ch].shift_pos;
			m_channels[ch].shift_pos += m_channels[ch].shift_amount;
			if (m_channels[ch].shift_pos & 0x10)
			{
				if (!read_sample(ch))
				{
					m_channels[ch].is_playing = false;
					continue;
				}

				adpcm_data |= (m_channels[ch].sample << (m_channels[ch].shift_amount - m_channels[ch].shift_pos));
			}

			adpcm_data = m_channels[ch].m_adpcm.clock(adpcm_data & m_channels[ch].shift_mask);

			m_channels[ch].output_r = adpcm_data * m_channels[ch].vol_r / 128;
			m_channels[ch].output_l = adpcm_data * m_channels[ch].vol_l / 128;
			mix_r += m_channels[ch].output_r;
			mix_l += m_channels[ch].output_l;
		}

		outputs[0][i] = mix_r / 16;
		outputs[1][i] = mix_l / 16;
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
				m_channels[ch].freq_base = (0x1ff - (data & 0xff)) << (~data >> 8 & 3);
				break;

			// 3: left/right volume
			case 3:
				m_channels[ch].vol_r = m_lut_volume[data & 0xff];
				m_channels[ch].vol_l = m_lut_volume[data >> 8 & 0xff];
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
					if (data & (1 << ch) && !m_channels[ch].is_playing)
					{
						UINT32 address = m_regs[ch << 2 | 1] << 16 | m_regs[ch << 2];
						UINT16 start = m_rom_base[(address + 0) & m_rom_mask];
						UINT16 param = m_rom_base[(address + 1) & m_rom_mask];

						// check sample start ID
						if (start != 0x7f7f)
						{
							logerror("i5000snd: channel %d wrong sample start ID %04X!\n", ch, start);
							continue;
						}

						switch (param)
						{
							// 3-bit ADPCM
							case 0x0104:
							case 0x0304: // same?
								m_channels[ch].freq_min = 0x140;
								m_channels[ch].shift_amount = 3;
								m_channels[ch].shift_mask = 0xe;
								break;

							default:
								logerror("i5000snd: channel %d unknown sample param %04X!\n", ch, param);
								// fall through (take settings from 0x0184)
							// 4-bit ADPCM
							case 0x0184:
								m_channels[ch].freq_min = 0x100;
								m_channels[ch].shift_amount = 4;
								m_channels[ch].shift_mask = 0xf;
								break;
						}

						m_channels[ch].address = (address + 4) & m_rom_mask;

						m_channels[ch].freq_timer = 0;
						m_channels[ch].shift_pos = 0;

						m_channels[ch].m_adpcm.reset();
						m_channels[ch].is_playing = read_sample(ch);
					}
				}
				break;

			// channel key off (0 has no effect)
			case 0x43:
				for (int ch = 0; ch < 16; ch++)
				{
					if (data & (1 << ch))
						m_channels[ch].is_playing = false;
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
	m_stream->update();

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
