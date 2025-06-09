// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    Konami 005289 - SCC sound as used in Bubblesystem

    This file is pieced together by Bryan McPhail from a combination of
    Namco Sound, Amuse by Cab, Nemesis schematics and whoever first
    figured out SCC!

    The 005289 is a 2 channel sound generator. Each channel gets its
    waveform from a prom (4 bits wide).

    (From Nemesis schematics)

    Address lines A0-A4 of the prom run to the 005289, giving 32 bytes
    per waveform.  Address lines A5-A7 of the prom run to PA5-PA7 of
    the AY8910 control port A, giving 8 different waveforms. PA0-PA3
    of the AY8910 control volume.

    The second channel is the same as above except port B is used.

    The 005289 has 12 address inputs and 4 control inputs: LD1, LD2, TG1, TG2.
    It has no data bus, so data values written don't matter.
    When LD1 or LD2 is asserted, the 12 bit value on the address bus is
    latched. Each of the two channels has its own latch.
    When TG1 or TG2 is asserted, the frequency of the respective channel is
    set to the previously latched value.

    The 005289 itself is nothing but an address generator. Digital to analog
    conversion, volume control and mixing of the channels is all done
    externally via resistor networks and 4066 switches and is only implemented
    here for convenience.

***************************************************************************/

#include "emu.h"
#include "k005289.h"

// device type definition
DEFINE_DEVICE_TYPE(K005289, k005289_device, "k005289", "K005289 SCC")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  k005289_device - constructor
//-------------------------------------------------

k005289_device::k005289_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, K005289, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_sound_prom(*this, DEVICE_SELF)
	, m_stream(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k005289_device::device_start()
{
	/* get stream channels */
	m_stream = stream_alloc(0, 1, clock());

	/* reset all the voices */
	for (auto & voice : m_voice)
	{
		voice.reset();
	}

	save_item(STRUCT_MEMBER(m_voice, counter));
	save_item(STRUCT_MEMBER(m_voice, frequency));
	save_item(STRUCT_MEMBER(m_voice, pitch));
	save_item(STRUCT_MEMBER(m_voice, waveform));
	save_item(STRUCT_MEMBER(m_voice, volume));
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void k005289_device::sound_stream_update(sound_stream &stream)
{
	for (int sampid = 0; sampid < stream.samples(); sampid++)
	{
		for (int i = 0; i < 2; i++)
		{
			voice_t &v = m_voice[i];
			if (--v.counter < 0)
			{
				v.waveform = (v.waveform & ~0x1f) | ((v.waveform + 1) & 0x1f);
				v.counter = v.frequency;
			}
			stream.add_int(0, sampid, ((m_sound_prom[((i & 1) << 8) | v.waveform] & 0xf) - 8) * v.volume, 512);
		}
	}
}


/********************************************************************************/

void k005289_device::control_A_w(u8 data)
{
	m_stream->update();

	m_voice[0].volume = data & 0xf;
	m_voice[0].waveform = (m_voice[0].waveform & ~0xe0) | (data & 0xe0);
}


void k005289_device::control_B_w(u8 data)
{
	m_stream->update();

	m_voice[1].volume = data & 0xf;
	m_voice[1].waveform = (m_voice[1].waveform & ~0xe0) | (data & 0xe0);
}


void k005289_device::ld1_w(offs_t offset, u8 data)
{
	m_voice[0].pitch = 0xfff - offset;
}


void k005289_device::ld2_w(offs_t offset, u8 data)
{
	m_voice[1].pitch = 0xfff - offset;
}


void k005289_device::tg1_w(u8 data)
{
	m_stream->update();

	m_voice[0].frequency = m_voice[0].pitch;
}


void k005289_device::tg2_w(u8 data)
{
	m_stream->update();

	m_voice[1].frequency = m_voice[1].pitch;
}
