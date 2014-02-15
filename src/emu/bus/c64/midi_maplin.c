// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Maplin MIDI Interface cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "midi_maplin.h"
#include "bus/midi/midi.h"
#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define MC6850_TAG      "mc6850"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_MIDI_MAPLIN = &device_creator<c64_maplin_midi_cartridge_device>;


//-------------------------------------------------
//  ACIA6850_INTERFACE( acia_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( c64_maplin_midi_cartridge_device::acia_irq_w )
{
	m_slot->irq_w(state);
}

static ACIA6850_INTERFACE( acia_intf )
{
	500000,
	0,          // rx clock (we manually clock rx)
	DEVCB_DEVICE_LINE_MEMBER("mdout", midi_port_device, write_txd),
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, c64_maplin_midi_cartridge_device, acia_irq_w)
};


//-------------------------------------------------
//  SLOT_INTERFACE( midiin_slot )
//-------------------------------------------------

static SLOT_INTERFACE_START( midiin_slot )
	SLOT_INTERFACE("midiin", MIDIIN_PORT)
SLOT_INTERFACE_END

WRITE_LINE_MEMBER( c64_maplin_midi_cartridge_device::midi_rx_w )
{
	m_acia->write_rx(state);

	for (int i = 0; i < 16; i++)    // divider is set to 64
	{
		m_acia->rx_clock_in();
	}
}


//-------------------------------------------------
//  SLOT_INTERFACE( midiout_slot )
//-------------------------------------------------

static SLOT_INTERFACE_START( midiout_slot )
	SLOT_INTERFACE("midiout", MIDIOUT_PORT)
SLOT_INTERFACE_END


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c64_maplin_midi )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c64_maplin_midi )
	MCFG_ACIA6850_ADD(MC6850_TAG, acia_intf)

	MCFG_MIDI_PORT_ADD("mdin", midiin_slot, "midiin")
	MCFG_MIDI_RX_HANDLER(DEVWRITELINE(DEVICE_SELF, c64_maplin_midi_cartridge_device, midi_rx_w))

	MCFG_MIDI_PORT_ADD("mdout", midiout_slot, "midiout")
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c64_maplin_midi_cartridge_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c64_maplin_midi );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_maplin_midi_cartridge_device - constructor
//-------------------------------------------------

c64_maplin_midi_cartridge_device::c64_maplin_midi_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_MIDI_MAPLIN, "C64 Maplin MIDI", tag, owner, clock, "c64_midimap", __FILE__),
	device_c64_expansion_card_interface(mconfig, *this),
	m_acia(*this, MC6850_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_maplin_midi_cartridge_device::device_start()
{
	// state saving
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_maplin_midi_cartridge_device::device_reset()
{
	m_acia->reset();
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_maplin_midi_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io2)
	{
		switch (offset & 0xff)
		{
		case 0:
			data = m_acia->status_read(space, 0);
			break;

		case 1:
			data = m_acia->data_read(space, 0);
			break;
		}
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_maplin_midi_cartridge_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io2)
	{
		switch (offset & 0xff)
		{
		case 0:
			m_acia->control_write(space, 0, data);
			break;

		case 1:
			m_acia->data_write(space, 0, data);
			break;
		}
	}
}
