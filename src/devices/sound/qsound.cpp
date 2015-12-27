// license:???
// copyright-holders:Paul Leaman, Miguel Angel Horna
/***************************************************************************

  Capcom System QSound(tm)
  ========================

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

***************************************************************************/

#include "emu.h"
#include "qsound.h"

// device type definition
const device_type QSOUND = &device_creator<qsound_device>;


// program map for the DSP (points to internal 4096 words of internal ROM)
static ADDRESS_MAP_START( dsp16_program_map, AS_PROGRAM, 16, qsound_device )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
ADDRESS_MAP_END


// data map for the DSP (the dsp16 appears to use 2048 words of internal RAM)
static ADDRESS_MAP_START( dsp16_data_map, AS_DATA, 16, qsound_device )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_RAM
ADDRESS_MAP_END


// machine fragment
static MACHINE_CONFIG_FRAGMENT( qsound )
	MCFG_CPU_ADD("qsound", DSP16, QSOUND_CLOCK)
	MCFG_CPU_PROGRAM_MAP(dsp16_program_map)
	MCFG_CPU_DATA_MAP(dsp16_data_map)
MACHINE_CONFIG_END


// ROM definition for the Qsound program ROM
// NOTE: ROM is marked as bad since a handful of questionable bits haven't been fully examined.
ROM_START( qsound )
	ROM_REGION( 0x2000, "qsound", 0 )
	ROM_LOAD16_WORD( "qsound.bin", 0x0000, 0x2000, BAD_DUMP CRC(059c847d) SHA1(229cead1be2f86733dd80573d4983ba482355ece) )
ROM_END


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  qsound_device - constructor
//-------------------------------------------------

qsound_device::qsound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, QSOUND, "Q-Sound", tag, owner, clock, "qsound", __FILE__),
		device_sound_interface(mconfig, *this),
		m_cpu(*this, "qsound"),
		m_sample_rom(*this, DEVICE_SELF),
		m_data(0),
		m_stream(nullptr)
{
}


//-------------------------------------------------
//  rom_region - return a pointer to the device's
//  internal ROM region
//-------------------------------------------------

const rom_entry *qsound_device::device_rom_region() const
{
	return ROM_NAME( qsound );
}


//-------------------------------------------------
//  machine_config_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor qsound_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( qsound );
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void qsound_device::device_start()
{
	m_stream = stream_alloc(0, 2, clock() / 166); // /166 clock divider?

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

void qsound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
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

				INT8 sample = read_sample(elem.bank | elem.address);
				*lmix++ += ((sample * elem.lvol * elem.vol) >> 14);
				*rmix++ += ((sample * elem.rvol * elem.vol) >> 14);
			}
		}
	}
}


WRITE8_MEMBER(qsound_device::qsound_w)
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


READ8_MEMBER(qsound_device::qsound_r)
{
	/* Port ready bit (0x80 if ready) */
	return 0x80;
}


void qsound_device::write_data(UINT8 address, UINT16 data)
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
