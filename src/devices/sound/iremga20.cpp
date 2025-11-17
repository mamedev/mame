// license:BSD-3-Clause
// copyright-holders:Acho A. Tang,R. Belmont, Valley Bell
/*********************************************************

Irem GA20 PCM Sound Chip
80 pin QFP, label NANAO GA20 (Nanao Corporation was Irem's parent company)

TODO:
- It's not currently known whether this chip is stereo.
- Is sample position base(regs 0,1) used while sample is playing, or
  latched at key on? We've always emulated it the latter way.
  gunforc2 seems to be the only game updating the address regs sometimes
  while a sample is playing, but it doesn't seem intentional.
- What is the 2nd sample address for? Is it end(cut-off) address, or
  loop start address? Every game writes a value that's past sample end.
- All games write either 0 or 2 to reg #6, do other bits have any function?


Revisions:

04-15-2002 Acho A. Tang
- rewrote channel mixing
- added prelimenary volume and sample rate emulation

05-30-2002 Acho A. Tang
- applied hyperbolic gain control to volume and used
  a musical-note style progression in sample rate
  calculation(still very inaccurate)

02-18-2004 R. Belmont
- sample rate calculation reverse-engineered.
  Thanks to Fujix, Yasuhiro Ogawa, the Guru, and Tormod
  for real PCB samples that made this possible.

02-03-2007 R. Belmont
- Cleaned up faux x86 assembly.

09-25-2018 Valley Bell & co
- rewrote channel update to make data 0 act as sample terminator

*********************************************************/

#include "emu.h"
#include "iremga20.h"

#include <algorithm>


// device type definition
DEFINE_DEVICE_TYPE(IREMGA20, iremga20_device, "iremga20", "Irem GA20")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  iremga20_device - constructor
//-------------------------------------------------

iremga20_device::iremga20_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, IREMGA20, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	device_rom_interface(mconfig, *this),
	m_stream(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void iremga20_device::device_start()
{
	m_stream = stream_alloc(0, 2, clock()/4);

	save_item(NAME(m_regs));
	for (int i = 0; i < 4; i++)
	{
		save_item(NAME(m_channel[i].rate), i);
		save_item(NAME(m_channel[i].pos), i);
		save_item(NAME(m_channel[i].counter), i);
		save_item(NAME(m_channel[i].end), i);
		save_item(NAME(m_channel[i].volume), i);
		save_item(NAME(m_channel[i].play), i);
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void iremga20_device::device_reset()
{
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	for (int i = 0; i < 4; i++)
	{
		m_channel[i].rate = 0;
		m_channel[i].pos = 0;
		m_channel[i].counter = 0;
		m_channel[i].end = 0;
		m_channel[i].volume = 0;
		m_channel[i].play = 0;
	}
}

//-------------------------------------------------
//  device_clock_changed - called if the clock
//  changes
//-------------------------------------------------

void iremga20_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock()/4);
}

//-------------------------------------------------
//  rom_bank_pre_change - refresh the stream if the
//  ROM banking changes
//-------------------------------------------------

void iremga20_device::rom_bank_pre_change()
{
	m_stream->update();
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void iremga20_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < stream.samples(); i++)
	{
		s32 sampleout = 0;

		for (auto &ch : m_channel)
		{
			if (ch.play)
			{
				int sample = read_byte(ch.pos);
				if (sample == 0x00) // check for sample end marker
					ch.play = 0;
				else
				{
					sampleout += (sample - 0x80) * (int32_t)ch.volume;
					ch.counter--;
					if (ch.counter <= ch.rate)
					{
						ch.pos++;
						ch.counter = 0x100;
					}
				}
			}
		}

		stream.put_int(0, i, sampleout, 32768 * 4);
		stream.put_int(1, i, sampleout, 32768 * 4);
	}
}

void iremga20_device::write(offs_t offset, uint8_t data)
{
	m_stream->update();

	offset &= 0x1f;
	m_regs[offset] = data;
	int ch = offset >> 3;

	// channel regs:
	// 0,1: start address
	// 2,3: end? address
	// 4: rate
	// 5: volume
	// 6: control
	// 7: voice status (read-only)

	switch (offset & 0x7)
	{
		case 4:
			m_channel[ch].rate = data;
			break;

		case 5:
			m_channel[ch].volume = (data * 256) / (data + 10);
			break;

		case 6:
			// d1: key on/off
			if (data & 2)
			{
				m_channel[ch].play = 1;
				m_channel[ch].pos = (m_regs[ch << 3 | 0] | m_regs[ch << 3 | 1] << 8) << 4;
				m_channel[ch].end = (m_regs[ch << 3 | 2] | m_regs[ch << 3 | 3] << 8) << 4;
				m_channel[ch].counter = 0x100;
			}
			else
				m_channel[ch].play = 0;

			// other: unknown/unused
			// possibilities are: loop flag, left/right speaker(stereo)
			break;
	}
}

uint8_t iremga20_device::read(offs_t offset)
{
	m_stream->update();

	offset &= 0x1f;
	int ch = offset >> 3;

	switch (offset & 0x7)
	{
		case 7: // voice status. bit 0 is 1 if active. (routine around 0xccc in rtypeleo)
			return m_channel[ch].play;

		default:
			logerror("GA20: read unk. register %d, channel %d\n", offset & 0x7, ch);
			break;
	}

	return 0;
}
