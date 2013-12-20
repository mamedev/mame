// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Sequential Circuits MIDI Interface cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "midi_sci.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define MC6850_TAG      "mc6850"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_MIDI_SCI = &device_creator<c64_sequential_midi_cartridge_device>;


//-------------------------------------------------
//  ACIA6850_INTERFACE( acia_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( c64_sequential_midi_cartridge_device::acia_irq_w )
{
	m_slot->irq_w(state);
}

READ_LINE_MEMBER( c64_sequential_midi_cartridge_device::rx_in )
{
	return m_rx_state;
}

WRITE_LINE_MEMBER( c64_sequential_midi_cartridge_device::tx_out )
{
	m_mdout->tx(state);
}

static ACIA6850_INTERFACE( acia_intf )
{
	500000,
	0,          // rx clock (we manually clock rx)
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, c64_sequential_midi_cartridge_device, rx_in),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, c64_sequential_midi_cartridge_device, tx_out),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, c64_sequential_midi_cartridge_device, acia_irq_w)
};


//-------------------------------------------------
//  SLOT_INTERFACE( midiin_slot )
//-------------------------------------------------

static SLOT_INTERFACE_START( midiin_slot )
	SLOT_INTERFACE("midiin", MIDIIN_PORT)
SLOT_INTERFACE_END

WRITE_LINE_MEMBER( c64_sequential_midi_cartridge_device::midi_rx_w )
{
	m_rx_state = state;

	for (int i = 0; i < 16; i++)    // divider is set to 16
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
//  MACHINE_CONFIG_FRAGMENT( c64_sequential_midi )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c64_sequential_midi )
	MCFG_ACIA6850_ADD(MC6850_TAG, acia_intf)

	MCFG_SERIAL_PORT_ADD("mdin", midiin_slot, "midiin")
	MCFG_SERIAL_OUT_RX_HANDLER(DEVWRITELINE(DEVICE_SELF, c64_sequential_midi_cartridge_device, midi_rx_w))

	MCFG_SERIAL_PORT_ADD("mdout", midiout_slot, "midiout")
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c64_sequential_midi_cartridge_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c64_sequential_midi );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_sequential_midi_cartridge_device - constructor
//-------------------------------------------------

c64_sequential_midi_cartridge_device::c64_sequential_midi_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_MIDI_SCI, "C64 Sequential Circuits MIDI", tag, owner, clock, "c64_midisci", __FILE__),
	device_c64_expansion_card_interface(mconfig, *this),
	m_acia(*this, MC6850_TAG),
	m_mdout(*this, "mdout"),
	m_rx_state(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_sequential_midi_cartridge_device::device_start()
{
	// state saving
	save_item(NAME(m_rx_state));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_sequential_midi_cartridge_device::device_reset()
{
	m_acia->reset();

	m_rx_state = 0;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_sequential_midi_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
	{
		switch (offset & 0xff)
		{
		case 2:
			data = m_acia->status_read(space, 0);
			break;

		case 3:
			data = m_acia->data_read(space, 0);
			break;
		}
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_sequential_midi_cartridge_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
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
