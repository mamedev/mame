// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Sequential Circuits MIDI Interface cartridge emulation

**********************************************************************/

#include "emu.h"
#include "midi_sci.h"
#include "machine/clock.h"
#include "bus/midi/midi.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define MC6850_TAG      "mc6850"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_MIDI_SCI, c64_sequential_midi_cartridge_device, "c64_midisci", "C64 Sequential Circuits MIDI")


WRITE_LINE_MEMBER( c64_sequential_midi_cartridge_device::acia_irq_w )
{
	m_slot->irq_w(state);
}

WRITE_LINE_MEMBER( c64_sequential_midi_cartridge_device::write_acia_clock )
{
	m_acia->write_txc(state);
	m_acia->write_rxc(state);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c64_sequential_midi_cartridge_device::device_add_mconfig(machine_config &config)
{
	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set("mdout", FUNC(midi_port_device::write_txd));
	m_acia->irq_handler().set(FUNC(c64_sequential_midi_cartridge_device::acia_irq_w));

	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));

	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	clock_device &acia_clock(CLOCK(config, "acia_clock", 31250*16));
	acia_clock.signal_handler().set(FUNC(c64_sequential_midi_cartridge_device::write_acia_clock));
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_sequential_midi_cartridge_device - constructor
//-------------------------------------------------

c64_sequential_midi_cartridge_device::c64_sequential_midi_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_MIDI_SCI, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	m_acia(*this, MC6850_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_sequential_midi_cartridge_device::device_start()
{
	// state saving
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_sequential_midi_cartridge_device::device_reset()
{
	m_acia->reset();
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_sequential_midi_cartridge_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
	{
		switch (offset & 0xff)
		{
		case 2:
		case 3:
			data = m_acia->read(offset & 1);
			break;
		}
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_sequential_midi_cartridge_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
	{
		switch (offset & 0xff)
		{
		case 0:
		case 1:
			m_acia->write(offset & 1, data);
			break;
		}
	}
}
