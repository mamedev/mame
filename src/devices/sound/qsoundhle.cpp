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

	for (int adr = 0x80; adr < 0x90; adr++)
		write_data(adr, 0x120);

	// state save
	for (int i = 0; i < 16; i++)
	{
		save_item(NAME(m_channel[i].reg), i);
		save_item(NAME(m_channel[i].lvol), i);
		save_item(NAME(m_channel[i].rvol), i);
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void qsound_hle_device::device_reset()
{
	for (qsound_channel &ch : m_channel)
		std::fill(std::begin(ch.reg), std::end(ch.reg), 0U);
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void qsound_hle_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// Clear the buffers
	memset(outputs[0], 0, samples * sizeof(*outputs[0]));
	memset(outputs[1], 0, samples * sizeof(*outputs[1]));

	for (unsigned n = 0; ARRAY_LENGTH(m_channel) > n; ++n)
	{
		qsound_channel &ch(m_channel[n]);
		stream_sample_t *lmix(outputs[0]);
		stream_sample_t *rmix(outputs[1]);

		// Go through the buffer and add voice contributions
		offs_t const bank(m_channel[(n + ARRAY_LENGTH(m_channel) - 1) & (ARRAY_LENGTH(m_channel) - 1)].reg[0] & 0x7fff);
		for (int i = 0; i < samples; i++)
		{
			// current sample address (bank comes from previous channel)
			offs_t const addr(ch.reg[1] | (bank << 16));

			// update based on playback rate
			uint32_t updated(uint32_t(ch.reg[2] << 4) + ((uint32_t(ch.reg[1]) << 16) | ch.reg[3]));
			ch.reg[3] = uint16_t(updated);
			if (updated >= (uint32_t(ch.reg[5]) << 16))
				updated -= uint32_t(ch.reg[4]) << 16;
			ch.reg[1] = uint16_t(updated >> 16);

			// get the scaled sample
			int32_t const scaled(int32_t(int16_t(ch.reg[6])) * read_sample(addr));

			// apply simple panning
			*lmix++ += (((scaled >> 8) * ch.lvol) >> 14);
			*rmix++ += (((scaled >> 8) * ch.rvol) >> 14);
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

	if (address < 0x80)
	{
		ch = address >> 3;
		reg = address & 7;
	}
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
	case 0: // bank
	case 1: // current sample
	case 2: // playback rate
	case 3: // sample interval counter
	case 4: // loop offset
	case 5: // end sample
	case 6: // channel volume
	case 7: // unused
		m_channel[ch].reg[reg] = data;
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
