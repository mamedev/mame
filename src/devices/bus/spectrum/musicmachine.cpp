// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/*
 * The Music Machine - MIDI and sampling expansion
 * by Ram Electronics Ltd
 */

#include "emu.h"
#include "musicmachine.h"

#include "bus/midi/midi.h"
#include "machine/clock.h"
#include "speaker.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SPECTRUM_MUSICMACHINE, spectrum_musicmachine_device, "spectrummusic", "The Music Machine (ZX)")


void spectrum_musicmachine_device::device_add_mconfig(machine_config &config)
{
	ACIA6850(config, m_acia).txd_handler().set("mdout", FUNC(midi_port_device::write_txd));
	m_acia->irq_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::nmi_w));
	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));
	MIDI_PORT(config, "mdout", midiout_slot, "midiout");
	clock_device &acia_clock(CLOCK(config, "acia_clock", 31250*16));
	acia_clock.signal_handler().set(FUNC(spectrum_musicmachine_device::write_acia_clock));

	SPEAKER(config, "speaker").front_center();
	ZN429E(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.2);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

spectrum_musicmachine_device::spectrum_musicmachine_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SPECTRUM_MUSICMACHINE, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_acia(*this,"acia")
	, m_dac(*this,"dac")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_musicmachine_device::device_start()
{
	save_item(NAME(m_irq_select));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_musicmachine_device::device_reset()
{
	m_irq_select = false;
}

void spectrum_musicmachine_device::write_acia_clock(u8 data)
{
	m_acia->write_txc(data);
	m_acia->write_rxc(data);
}

u8 spectrum_musicmachine_device::iorq_r(offs_t offset)
{
	u8 data = offset & 1 ? m_slot->fb_r() : 0xff;

	switch (offset & 0xff)
	{
		case 0x7f:
			if ((offset & 0x3ff) == 0x27f)
				data = m_acia->status_r();
			else if ((offset & 0x3ff) == 0x37f)
				data = m_acia->data_r();
			break;
		case 0xbf:
			// TODO ADC_READ
			break;
		case 0xdf:
			// TODO Strobe: ADC_START
			break;
	}

	return data;
}

void spectrum_musicmachine_device::iorq_w(offs_t offset, u8 data)
{
	switch (offset & 0xff)
	{
		case 0x5f:
			m_irq_select = data & 1;
			break;
		case 0x7f:
			if ((offset & 0x3ff) == 0x07f)
				m_acia->control_w(data);
			else if ((offset & 0x3ff) == 0x17f)
				m_acia->data_w(data);
			break;
		case 0x9f:
			m_dac->write(data);
			break;
		case 0xdf:
			// TODO Strobe: ADC_START
			break;
	}
}
