// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

An Hitachi HD637A01X0 MCU programmed to act as a sample player.
Used by some Namco System 86 games.

The MCU has internal ROM which hasn't been dumped, so here we simulate its
simple functions.

The chip can address ROM space up to 8 block of 0x10000 bytes. At the beginning
of each block there's a table listing the start offset of each sample.
Samples are 8 bit unsigned, 0xff marks the end of the sample. 0x00 is used for
silence compression: '00 nn' must be replaced by nn+1 times '80'.

***************************************************************************/

#include "emu.h"
#include "n63701x.h"


/* volume control has three resistors: 22000, 10000 and 3300 Ohm.
   22000 is always enabled, the other two can be turned off.
   Since 0x00 and 0xff samples have special meaning, the available range is
   0x01 to 0xfe, therefore 258 * (0x01 - 0x80) = 0x8002 just keeps us
   inside 16 bits without overflowing.
 */
static const int vol_table[4] = { 26, 84, 200, 258 };


// device type definition
const device_type NAMCO_63701X = &device_creator<namco_63701x_device>;

namco_63701x_device::namco_63701x_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, NAMCO_63701X, "Namco 63701X", tag, owner, clock, "namco_63701x", __FILE__),
		device_sound_interface(mconfig, *this),
		m_rom(*this, DEVICE_SELF),
		m_stream(NULL)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void namco_63701x_device::device_start()
{
	m_stream = stream_alloc(0, 2, clock()/1000);

	for (int i = 0; i < 2; i++)
	{
		save_item(NAME(m_voices[i].select), i);
		save_item(NAME(m_voices[i].playing), i);
		save_item(NAME(m_voices[i].base_addr), i);
		save_item(NAME(m_voices[i].position), i);
		save_item(NAME(m_voices[i].volume), i);
		save_item(NAME(m_voices[i].silence_counter), i);
	}
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void namco_63701x_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	int ch;

	for (ch = 0;ch < 2;ch++)
	{
		stream_sample_t *buf = outputs[ch];
		voice_63701x *v = &m_voices[ch];

		if (v->playing)
		{
			UINT8 *base = m_rom + v->base_addr;
			int pos = v->position;
			int vol = vol_table[v->volume];
			int p;

			for (p = 0;p < samples;p++)
			{
				if (v->silence_counter)
				{
					v->silence_counter--;
					*(buf++) = 0;
				}
				else
				{
					int data = base[(pos++) & 0xffff];

					if (data == 0xff)   /* end of sample */
					{
						v->playing = 0;
						break;
					}
					else if (data == 0x00)  /* silence compression */
					{
						data = base[(pos++) & 0xffff];
						v->silence_counter = data;
						*(buf++) = 0;
					}
					else
					{
						*(buf++) = vol * (data - 0x80);
					}
				}
			}

			v->position = pos;
		}
		else
			memset(buf, 0, samples * sizeof(*buf));
	}
}



WRITE8_MEMBER( namco_63701x_device::namco_63701x_w )
{
	int ch = offset / 2;

	if (offset & 1)
		m_voices[ch].select = data;
	else
	{
		/*
		  should we stop the playing sample if voice_select[ch] == 0 ?
		  originally we were, but this makes us lose a sample in genpeitd,
		  after the continue counter reaches 0. Either we shouldn't stop
		  the sample, or genpeitd is returning to the title screen too soon.
		 */
		if (m_voices[ch].select & 0x1f)
		{
			int rom_offs;

			/* update the streams */
			m_stream->update();

			m_voices[ch].playing = 1;
			m_voices[ch].base_addr = 0x10000 * ((m_voices[ch].select & 0xe0) >> 5);
			rom_offs = m_voices[ch].base_addr + 2 * ((m_voices[ch].select & 0x1f) - 1);
			m_voices[ch].position = (m_rom[rom_offs] << 8) + m_rom[rom_offs+1];
			/* bits 6-7 = volume */
			m_voices[ch].volume = data >> 6;
			/* bits 0-5 = counter to indicate new sample start? we don't use them */

			m_voices[ch].silence_counter = 0;
		}
	}
}
