// license:BSD-3-Clause
// copyright-holders:Paul Leaman, Miguel Angel Horna
/***************************************************************************

  Capcom System QSound™ (HLE)
  ===========================

  Driver by Paul Leaman and Miguel Angel Horna

  A 16 channel stereo sample player.

  QSpace position is simulated by panning the sound in the stereo space.

  Many thanks to CAB (the author of Amuse), without whom this probably would
  never have been finished.

  TODO:
  - hook up the DSP!
  - is master volume really linear?
  - understand higher bits of reg 0
  - understand reg 9
  - understand other writes to $90-$ff area

  Links:
  https://siliconpr0n.org/map/capcom/dl-1425

***************************************************************************/

#include "emu.h"
#include "qsoundhle.h"

// device type definition
DEFINE_DEVICE_TYPE(QSOUND_HLE, qsound_hle_device, "qsound_hle", "QSound (HLE)")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  qsound_hle_device - constructor
//-------------------------------------------------

qsound_hle_device::qsound_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, QSOUND_HLE, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_rom_interface(mconfig, *this, 24)
	, m_stream(nullptr)
	, m_data(0)
{
}


//-------------------------------------------------
//  rom_bank_updated - the rom bank has changed
//-------------------------------------------------

void qsound_hle_device::rom_bank_updated()
{
	m_stream->update();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void qsound_hle_device::device_start()
{
	m_stream = stream_alloc(0, 2, clock() / 2 / 1248); // DSP program uses 1248 machine cycles per iteration

	// create pan table
	for (int i = 0; i < 33; i++)
		m_pan_table[i] = (int)((256 / sqrt(32.0)) * sqrt((double)i));

	// init sound regs
	memset(m_channel, 0, sizeof(m_channel));

	for (int adr = 0x7f; adr >= 0; adr--)
		write_data(adr, 0);
	for (int adr = 0x80; adr < 0x90; adr++)
		write_data(adr, 0x120);

	// state save
	for (int i = 0; i < 16; i++)
	{
		save_item(NAME(m_channel[i].bank), i);
		save_item(NAME(m_channel[i].address), i);
		save_item(NAME(m_channel[i].freq), i);
		save_item(NAME(m_channel[i].loop), i);
		save_item(NAME(m_channel[i].end), i);
		save_item(NAME(m_channel[i].vol), i);
		save_item(NAME(m_channel[i].enabled), i);
		save_item(NAME(m_channel[i].lvol), i);
		save_item(NAME(m_channel[i].rvol), i);
		save_item(NAME(m_channel[i].step_ptr), i);
	}
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void qsound_hle_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// Clear the buffers
	memset(outputs[0], 0, samples * sizeof(*outputs[0]));
	memset(outputs[1], 0, samples * sizeof(*outputs[1]));

	for (auto & elem : m_channel)
	{
		if (elem.enabled)
		{
			stream_sample_t *lmix=outputs[0];
			stream_sample_t *rmix=outputs[1];

			// Go through the buffer and add voice contributions
			for (int i = 0; i < samples; i++)
			{
				elem.address += (elem.step_ptr >> 12);
				elem.step_ptr &= 0xfff;
				elem.step_ptr += elem.freq;

				if (elem.address >= elem.end)
				{
					if (elem.loop)
					{
						// Reached the end, restart the loop
						elem.address -= elem.loop;

						// Make sure we don't overflow (what does the real chip do in this case?)
						if (elem.address >= elem.end)
							elem.address = elem.end - elem.loop;

						elem.address &= 0xffff;
					}
					else
					{
						// Reached the end of a non-looped sample
						elem.enabled = false;
						break;
					}
				}

				int8_t sample = read_sample(elem.bank | elem.address);
				*lmix++ += ((sample * elem.lvol * elem.vol) >> 14);
				*rmix++ += ((sample * elem.rvol * elem.vol) >> 14);
			}
		}
	}
}


WRITE8_MEMBER(qsound_hle_device::qsound_w)
{
	switch (offset)
	{
		case 0:
			m_data = (m_data & 0x00ff) | (data << 8);
			break;

		case 1:
			m_data = (m_data & 0xff00) | data;
			break;

		case 2:
			m_stream->update();
			write_data(data, m_data);
			break;

		default:
			logerror("%s: qsound_w %d = %02x\n", machine().describe_context(), offset, data);
			break;
	}
}


READ8_MEMBER(qsound_hle_device::qsound_r)
{
	/* Port ready bit (0x80 if ready) */
	return 0x80;
}


void qsound_hle_device::write_data(uint8_t address, uint16_t data)
{
	int ch = 0, reg;

	// direct sound reg
	if (address < 0x80)
	{
		ch = address >> 3;
		reg = address & 7;
	}

	// >= 0x80 is probably for the dsp?
	else if (address < 0x90)
	{
		ch = address & 0xf;
		reg = 8;
	}
	else if (address >= 0xba && address < 0xca)
	{
		ch = address - 0xba;
		reg = 9;
	}
	else
	{
		// unknown
		reg = address;
	}

	switch (reg)
	{
		case 0:
			// bank, high bits unknown
			ch = (ch + 1) & 0xf; // strange ...
			m_channel[ch].bank = data << 16;
			break;

		case 1:
			// start/cur address
			m_channel[ch].address = data;
			break;

		case 2:
			// frequency
			m_channel[ch].freq = data;
			if (data == 0)
			{
				// key off
				m_channel[ch].enabled = false;
			}
			break;

		case 3:
			// key on (does the value matter? it always writes 0x8000)
			m_channel[ch].enabled = true;
			m_channel[ch].step_ptr = 0;
			break;

		case 4:
			// loop address
			m_channel[ch].loop = data;
			break;

		case 5:
			// end address
			m_channel[ch].end = data;
			break;

		case 6:
			// master volume
			m_channel[ch].vol = data;
			break;

		case 7:
			// unused?
			break;

		case 8:
		{
			// panning (left=0x0110, centre=0x0120, right=0x0130)
			// looks like it doesn't write other values than that
			int pan = (data & 0x3f) - 0x10;
			if (pan > 0x20)
				pan = 0x20;
			if (pan < 0)
				pan = 0;

			m_channel[ch].rvol = m_pan_table[pan];
			m_channel[ch].lvol = m_pan_table[0x20 - pan];
			break;
		}

		case 9:
			// unknown
			break;

		default:
			//logerror("%s: write_data %02x = %04x\n", machine().describe_context(), address, data);
			break;
	}
}
