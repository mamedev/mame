/***************************************************************************

    ymz770.c

    Emulation by R. Belmont
    AMM decode by Olivier Galibert

***************************************************************************/

#include "emu.h"
#include "ymz770.h"
#include "mpeg_audio.h"

// device type definition
const device_type YMZ770 = &device_creator<ymz770_device>;

//-------------------------------------------------
//  ymz770_device - constructor
//-------------------------------------------------

ymz770_device::ymz770_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, YMZ770, "Yamaha YMZ770", tag, owner, clock, "ymz770", __FILE__),
		device_sound_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ymz770_device::device_start()
{
	// create the stream
	m_stream = machine().sound().stream_alloc(*this, 0, 2, 16000, this);
	m_rom_base = device().machine().root_device().memregion(":ymz770")->base();
	m_rom_size = device().machine().root_device().memregion(":ymz770")->bytes() * 8;

	for (int i = 0; i < 8; i++)
	{
		m_channels[i].is_playing = false;
		m_channels[i].is_seq_playing = false;
		m_channels[i].decoder = new mpeg_audio(m_rom_base, mpeg_audio::AMM, false, 0);
	}

	// register for save states
	save_item(NAME(m_cur_reg));
	for (int i = 0; i < 8; i++)
	{
		save_item(NAME(m_channels[i].phrase), i);
		save_item(NAME(m_channels[i].pan), i);
		save_item(NAME(m_channels[i].volume), i);
		save_item(NAME(m_channels[i].control), i);
		save_item(NAME(m_channels[i].is_playing), i);
		save_item(NAME(m_channels[i].last_block), i);
		save_item(NAME(m_channels[i].output_remaining), i);
		save_item(NAME(m_channels[i].output_ptr), i);
		save_item(NAME(m_channels[i].pptr), i);
		save_item(NAME(m_channels[i].sequence), i);
		save_item(NAME(m_channels[i].seqcontrol), i);
		save_item(NAME(m_channels[i].seqdelay), i);
		save_item(NAME(m_channels[i].is_seq_playing), i);
		save_item(NAME(m_channels[i].output_data), i);
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ymz770_device::device_reset()
{
	for (int i = 0; i < 8; i++)
	{
		m_channels[i].phrase = 0;
		m_channels[i].pan = 8;
		m_channels[i].volume = 0;
		m_channels[i].control = 0;
		m_channels[i].sequence = 0;
		m_channels[i].seqcontrol = 0;
		m_channels[i].seqdelay = 0;
		m_channels[i].is_playing = false;
		m_channels[i].is_seq_playing = false;
	}
}


//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void ymz770_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *outL, *outR;
	int i, ch;

	outL = outputs[0];
	outR = outputs[1];

	for (i = 0; i < samples; i++)
	{
		INT32 mix;

		mix = 0;

		for (ch = 0; ch < 8; ch++)
		{
			if (m_channels[ch].is_seq_playing)
			{
				if (m_channels[ch].seqdelay != 0)
				{
					m_channels[ch].seqdelay--;
				}
				else
				{
					int reg = *m_channels[ch].seqdata++;
					UINT8 data = *m_channels[ch].seqdata++;
					switch (reg)
					{
						case 0x0f:
							if (m_channels[ch].seqcontrol & 1)
							{
								UINT8 sqn = m_channels[ch].sequence;
								UINT32 pptr = m_rom_base[(4*sqn)+1+0x400]<<16 | m_rom_base[(4*sqn)+2+0x400]<<8 | m_rom_base[(4*sqn)+3+0x400];
								m_channels[ch].seqdata = &m_rom_base[pptr];
							}
							else
							{
								m_channels[ch].is_seq_playing = false;
							}
							break;
						case 0x0e:
							m_channels[ch].seqdelay = 32 - 1;
							break;
						default:
							internal_reg_write(reg, data);
							break;
					}
				}
			}
			if (m_channels[ch].is_playing)
			{
				if (m_channels[ch].output_remaining > 0)
				{
					mix += (m_channels[ch].output_data[m_channels[ch].output_ptr++]*2*m_channels[ch].volume);
					m_channels[ch].output_remaining--;
				}
				else
				{
retry:
					if (m_channels[ch].last_block)
					{
						if (m_channels[ch].control & 1)
						{
							UINT8 phrase = m_channels[ch].phrase;
							m_channels[ch].pptr = 8*(m_rom_base[(4*phrase)+1]<<16 | m_rom_base[(4*phrase)+2]<<8 | m_rom_base[(4*phrase)+3]);
						}
						else
						{
							m_channels[ch].is_playing = false;
						}
					}

					if (m_channels[ch].is_playing)
					{
						int sample_rate, channel_count;
						if(!m_channels[ch].decoder->decode_buffer(m_channels[ch].pptr,
																m_rom_size,
																m_channels[ch].output_data,
																m_channels[ch].output_remaining,
																sample_rate,
																channel_count))
						{
							m_channels[ch].last_block = true;
							goto retry;
						}

						m_channels[ch].last_block = m_channels[ch].output_remaining < 1152;
						m_channels[ch].output_remaining--;
						m_channels[ch].output_ptr = 1;

						mix += (m_channels[ch].output_data[0]*2*m_channels[ch].volume);
					}
				}
			}
		}

		outL[i] = outR[i] = mix>>8;
	}
}


//-------------------------------------------------
//  write - write to the chip's registers
//-------------------------------------------------

WRITE8_MEMBER( ymz770_device::write )
{
	if (offset & 1)
	{
		m_stream->update();
		internal_reg_write(m_cur_reg, data);
	}
	else
	{
		m_cur_reg = data;
	}
}


void ymz770_device::internal_reg_write(UINT8 reg, UINT8 data)
{
	if (reg >= 0x40 && reg <= 0x5f)
	{
		int voice = reg >> 2 & 0x07;

		switch (reg & 0x03)
		{
			case 0:
				m_channels[voice].phrase = data;
				break;

			case 1:
				m_channels[voice].volume = data;
				break;

			case 2:
				m_channels[voice].pan = data;
				break;

			case 3:
				if (data & 6)
				{
					UINT8 phrase = m_channels[voice].phrase;
					m_channels[voice].pptr = 8*(m_rom_base[(4*phrase)+1]<<16 | m_rom_base[(4*phrase)+2]<<8 | m_rom_base[(4*phrase)+3]);
					m_channels[voice].output_remaining = 0;
					m_channels[voice].output_ptr = 0;
					m_channels[voice].last_block = false;

					m_channels[voice].is_playing = true;
				}
				else
				{
					m_channels[voice].is_playing = false;
				}

				m_channels[voice].control = data;
				break;
		}
	}
	else if (reg >= 0x80)
	{
		int voice = reg >> 4 & 0x07;

		switch (reg & 0x0f)
		{
			case 0:
				m_channels[voice].sequence = data;
				break;
			case 1:
				if (data & 6)
				{
					UINT8 sqn = m_channels[voice].sequence;
					UINT32 pptr = m_rom_base[(4*sqn)+1+0x400]<<16 | m_rom_base[(4*sqn)+2+0x400]<<8 | m_rom_base[(4*sqn)+3+0x400];
					m_channels[voice].seqdata = &m_rom_base[pptr];
					m_channels[voice].seqdelay = 0;
					m_channels[voice].is_seq_playing = true;
				}
				else
				{
					m_channels[voice].is_seq_playing = false;
				}

				m_channels[voice].seqcontrol = data;
				break;

			default:
				break;
		}
	}
}
