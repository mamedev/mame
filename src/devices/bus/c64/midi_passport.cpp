// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Passport/Syntech MIDI Interface cartridge emulation

**********************************************************************/

#include "emu.h"
#include "midi_passport.h"
#include "machine/clock.h"
#include "bus/midi/midi.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define MC6840_TAG      "mc6840"
#define MC6850_TAG      "mc6850"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_MIDI_PASSPORT, c64_passport_midi_cartridge_device, "c64_midipp", "C64 Passport MIDI")


//-------------------------------------------------
//  ptm6840_interface ptm_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( c64_passport_midi_cartridge_device::ptm_irq_w )
{
	m_ptm_irq = state;

	m_slot->irq_w(m_ptm_irq || m_acia_irq);
}

WRITE_LINE_MEMBER( c64_passport_midi_cartridge_device::acia_irq_w )
{
	m_acia_irq = state;

	m_slot->irq_w(m_ptm_irq || m_acia_irq);
}

WRITE_LINE_MEMBER( c64_passport_midi_cartridge_device::write_acia_clock )
{
	m_acia->write_txc(state);
	m_acia->write_rxc(state);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(c64_passport_midi_cartridge_device::device_add_mconfig)
	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set("mdout", FUNC(midi_port_device::write_txd));
	m_acia->irq_handler().set(FUNC(c64_passport_midi_cartridge_device::acia_irq_w));

	PTM6840(config, m_ptm, 1021800);
	m_ptm->set_external_clocks(1021800.0f, 1021800.0f, 1021800.0f);
	m_ptm->irq_callback().set(FUNC(c64_passport_midi_cartridge_device::ptm_irq_w));

	MCFG_MIDI_PORT_ADD("mdin", midiin_slot, "midiin")
	MCFG_MIDI_RX_HANDLER(WRITELINE(MC6850_TAG, acia6850_device, write_rxd))

	MCFG_MIDI_PORT_ADD("mdout", midiout_slot, "midiout")

	MCFG_DEVICE_ADD("acia_clock", CLOCK, 31250*16) /// TODO: work out if the clock should come from the 6840
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(*this, c64_passport_midi_cartridge_device, write_acia_clock))
MACHINE_CONFIG_END



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_passport_midi_cartridge_device - constructor
//-------------------------------------------------

c64_passport_midi_cartridge_device::c64_passport_midi_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_MIDI_PASSPORT, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	m_acia(*this, MC6850_TAG),
	m_ptm(*this, MC6840_TAG),
	m_ptm_irq(CLEAR_LINE),
	m_acia_irq(CLEAR_LINE)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_passport_midi_cartridge_device::device_start()
{
	// state saving
	save_item(NAME(m_ptm_irq));
	save_item(NAME(m_acia_irq));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_passport_midi_cartridge_device::device_reset()
{
	m_acia->reset();
	m_ptm->reset();
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_passport_midi_cartridge_device::c64_cd_r(address_space &space, offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
	{
		switch (offset & 0xff)
		{
		case 0: case 1: case 2: case 3:
		case 4: case 5: case 6: case 7:
			data = m_ptm->read(space, offset & 0x07);
			break;

		case 8: case 9:
			data = m_acia->read(space, offset & 0x01);
			break;
		}
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_passport_midi_cartridge_device::c64_cd_w(address_space &space, offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
	{
		switch (offset & 0xff)
		{
		case 0: case 1: case 2: case 3:
		case 4: case 5: case 6: case 7:
			m_ptm->write(space, offset & 0x07, data);
			break;

		case 8: case 9:
			m_acia->write(space, offset & 0x01, data);
			break;

		case 0x30:
			// Drum sync SET
			break;

		case 0x38:
			// Drum sync CLEAR
			break;
		}
	}
}
