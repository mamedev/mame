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
	: device_t(mconfig, YMZ770, "Yamaha YMZ770", tag, owner, clock),
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
	rom_base = device().machine().root_device().memregion(":ymz770")->base();
	rom_size = device().machine().root_device().memregion(":ymz770")->bytes() * 8;

	for (int i = 0; i < 8; i++)
	{
		channels[i].is_playing = false;
		channels[i].is_seq_playing = false;
		channels[i].decoder = new mpeg_audio(rom_base, mpeg_audio::AMM, false, 0);
	}


	save_item(NAME(cur_reg));
	for (int i = 0; i < 8; i++)
	{
		save_item(NAME(channels[i].phrase), i);
		save_item(NAME(channels[i].pan), i);
		save_item(NAME(channels[i].volume), i);
		save_item(NAME(channels[i].control), i);
		save_item(NAME(channels[i].is_playing), i);
		save_item(NAME(channels[i].last_block), i);
		save_item(NAME(channels[i].output_remaining), i);
		save_item(NAME(channels[i].output_ptr), i);
		save_item(NAME(channels[i].pptr), i);
		save_item(NAME(channels[i].sequence), i);
		save_item(NAME(channels[i].seqcontrol), i);
		save_item(NAME(channels[i].seqdelay), i);
		save_item(NAME(channels[i].is_seq_playing), i);
		save_item(NAME(channels[i].output_data), i);
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ymz770_device::device_reset()
{
	for (int i = 0; i < 8; i++)
	{
		channels[i].phrase = 0;
		channels[i].pan = 8;
		channels[i].volume = 0;
		channels[i].control = 0;
		channels[i].sequence = 0;
		channels[i].seqcontrol = 0;
		channels[i].seqdelay = 0;
		channels[i].is_playing = false;
		channels[i].is_seq_playing = false;
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
			if (channels[ch].is_seq_playing)
			{
				if (channels[ch].seqdelay != 0)
				{
					channels[ch].seqdelay--;
				}
				else
				{
					int reg = *channels[ch].seqdata++;
					UINT8 data = *channels[ch].seqdata++;
					switch (reg)
					{
						case 0x0f:
							if (channels[ch].seqcontrol & 1)
							{
								UINT8 sqn = channels[ch].sequence;
								UINT32 pptr = rom_base[(4*sqn)+1+0x400]<<16 | rom_base[(4*sqn)+2+0x400]<<8 | rom_base[(4*sqn)+3+0x400];
								channels[ch].seqdata = &rom_base[pptr];
							}
							else
							{
								channels[ch].is_seq_playing = false;
							}
							break;
						case 0x0e:
							channels[ch].seqdelay = 32 - 1;
							break;
						default:
							cur_reg = reg;
							internal_reg_write(1, data);
							break;
					}
				}
			}
			if (channels[ch].is_playing)
			{
				if (channels[ch].output_remaining > 0)
				{
					mix += (channels[ch].output_data[channels[ch].output_ptr++]*2*channels[ch].volume);
					channels[ch].output_remaining--;
				}
				else
				{
				retry:
					if (channels[ch].last_block)
					{
						if (channels[ch].control & 1)
						{
								UINT8 phrase = channels[ch].phrase;
								channels[ch].pptr = 8*(rom_base[(4*phrase)+1]<<16 | rom_base[(4*phrase)+2]<<8 | rom_base[(4*phrase)+3]);
						}
						else
						{
								channels[ch].is_playing = false;
						}
					}

					if (channels[ch].is_playing)
					{
						int sample_rate, channel_count;
						if(!channels[ch].decoder->decode_buffer(channels[ch].pptr,
																rom_size,
																channels[ch].output_data,
																channels[ch].output_remaining,
																sample_rate,
																channel_count))
						{
							channels[ch].last_block = true;
							goto retry;
						}

						channels[ch].last_block = channels[ch].output_remaining < 1152;
						channels[ch].output_remaining--;
						channels[ch].output_ptr = 1;

						mix += (channels[ch].output_data[0]*2*channels[ch].volume);
					}
				}
			}
		}

		outL[i] = outR[i] = mix>>8;
	}
}

//-------------------------------------------------
//  read - read from the chip's registers and internal RAM
//-------------------------------------------------

READ8_MEMBER( ymz770_device::read )
{
	return 0;
}

void ymz770_device::internal_reg_write(int offset, UINT8 data)
{
	if (!offset)
	{
		cur_reg = data;
		return;
	}

	if (cur_reg >= 0x40 && cur_reg <= 0x5f)
	{
		cur_reg -= 0x40;

		int voice = cur_reg / 4;
		int reg = cur_reg % 4;

		switch (reg)
		{
			case 0:
				channels[voice].phrase = data;
				break;

			case 1:
				channels[voice].volume = data;
				break;

			case 2:
				channels[voice].pan = data;
				break;

			case 3:
				if (data & 6)
				{
					UINT8 phrase = channels[voice].phrase;
					channels[voice].pptr = 8*(rom_base[(4*phrase)+1]<<16 | rom_base[(4*phrase)+2]<<8 | rom_base[(4*phrase)+3]);
					channels[voice].output_remaining = 0;
					channels[voice].output_ptr = 0;
					channels[voice].last_block = false;

					channels[voice].is_playing = true;
				}
				else
				{
					channels[voice].is_playing = false;
				}

				channels[voice].control = data;
				break;
		}
	}
	else if (cur_reg >= 0x80)
	{
		int voice = (cur_reg & 0x70)>>4;
		int reg = cur_reg & 0x0f;
		switch (reg)
		{
			case 0:
				channels[voice].sequence = data;
				break;
			case 1:
				if (data & 6)
				{
					UINT8 sqn = channels[voice].sequence;
					UINT32 pptr = rom_base[(4*sqn)+1+0x400]<<16 | rom_base[(4*sqn)+2+0x400]<<8 | rom_base[(4*sqn)+3+0x400];
					channels[voice].seqdata = &rom_base[pptr];
					channels[voice].seqdelay = 0;
					channels[voice].is_seq_playing = true;
				}
				else
				{
						channels[voice].is_seq_playing = false;
				}
				channels[voice].seqcontrol = data;
				break;
		}
	}
}

//-------------------------------------------------
//  write - write to the chip's registers and internal RAM
//-------------------------------------------------

WRITE8_MEMBER( ymz770_device::write )
{
	m_stream->update();
	internal_reg_write(offset, data);
}
