/*********************************************************************

    a2midi.c

    Apple II 6850 MIDI card, as made by Passport, Yamaha, and others.

*********************************************************************/

#include "emu.h"
#include "includes/apple2.h"
#include "a2midi.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS_MIDI = &device_creator<a2bus_midi_device>;

#define MIDI_PTM_TAG     "midi_ptm"
#define MIDI_ACIA_TAG    "midi_acia"

static struct ptm6840_interface ptm_interface =
{
	1021800.0f,
	{ 1021800.0f, 1021800.0f, 1021800.0f },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, a2bus_midi_device, ptm_irq_w)
};

static struct acia6850_interface acia_interface =
{
	31250*16,   // tx clock
	0,          // rx clock (we manually clock rx)
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, a2bus_midi_device, rx_in),  // rx in
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, a2bus_midi_device, tx_out), // tx out
	DEVCB_NULL, // cts in
	DEVCB_NULL, // rts out
	DEVCB_NULL, // dcd in
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, a2bus_midi_device, acia_irq_w)
};

static SLOT_INTERFACE_START(midiin_slot)
	SLOT_INTERFACE("midiin", MIDIIN_PORT)
SLOT_INTERFACE_END

static const serial_port_interface midiin_intf =
{
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, a2bus_midi_device, midi_rx_w)
};

static SLOT_INTERFACE_START(midiout_slot)
	SLOT_INTERFACE("midiout", MIDIOUT_PORT)
SLOT_INTERFACE_END

static const serial_port_interface midiout_intf =
{
	DEVCB_NULL  // midi out ports don't transmit inward
};

MACHINE_CONFIG_FRAGMENT( midi )
	MCFG_PTM6840_ADD(MIDI_PTM_TAG, ptm_interface)
	MCFG_ACIA6850_ADD(MIDI_ACIA_TAG, acia_interface)
	MCFG_SERIAL_PORT_ADD("mdin", midiin_intf, midiin_slot, "midiin")
	MCFG_SERIAL_PORT_ADD("mdout", midiout_intf, midiout_slot, "midiout")
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor a2bus_midi_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( midi );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_midi_device::a2bus_midi_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, A2BUS_MIDI, "6850 MIDI card", tag, owner, clock, "a2midi", __FILE__),
		device_a2bus_card_interface(mconfig, *this),
		m_ptm(*this, MIDI_PTM_TAG),
		m_acia(*this, MIDI_ACIA_TAG),
		m_mdout(*this, "mdout")
{
}

a2bus_midi_device::a2bus_midi_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_a2bus_card_interface(mconfig, *this),
		m_ptm(*this, MIDI_PTM_TAG),
		m_acia(*this, MIDI_ACIA_TAG),
		m_mdout(*this, "mdout")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_midi_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();
}

void a2bus_midi_device::device_reset()
{
	m_acia_irq = m_ptm_irq = false;
	m_rx_state = 0;
}

/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

UINT8 a2bus_midi_device::read_c0nx(address_space &space, UINT8 offset)
{
	// PTM at C0n0-C0n7, ACIA at C0n8-C0n9, drum sync (?) at C0nA-C0nB

	if (offset < 8)
	{
		return m_ptm->read(space, offset & 7);
	}
	else if (offset == 8)
	{
		return m_acia->status_read(space, 0);
	}
	else if (offset == 9)
	{
		UINT8 ret = m_acia->data_read(space, 0);
		return ret;
	}

	return 0;
}

/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_midi_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
{
	if (offset < 8)
	{
		m_ptm->write(space, offset & 7, data);
	}
	else if (offset == 8)
	{
		// HACK: GS/OS's CARD6850.MIDI driver sets 8-N-2, which is not valid MIDI.
		// This works on h/w pretty much by accident; we'll make it right here.
		if ((data & 0x1c) == 0x10)
		{
			data |= 0x04;   // change wordbits from 0x10 to 0x14
		}

		m_acia->control_write(space, 0, data);
	}
	else if (offset == 9)
	{
		m_acia->data_write(space, 0, data);
	}
}

WRITE_LINE_MEMBER( a2bus_midi_device::acia_irq_w )
{
	m_acia_irq = state ? true : false;

	if (m_acia_irq || m_ptm_irq)
	{
		raise_slot_irq();
	}
	else
	{
		lower_slot_irq();
	}
}

WRITE_LINE_MEMBER( a2bus_midi_device::ptm_irq_w )
{
	m_acia_irq = state ? true : false;

	if (m_acia_irq || m_ptm_irq)
	{
		raise_slot_irq();
	}
	else
	{
		lower_slot_irq();
	}
}

WRITE_LINE_MEMBER( a2bus_midi_device::midi_rx_w )
{
	m_rx_state = state;
	for (int i = 0; i < 16; i++)    // divider is set to 16
	{
		m_acia->rx_clock_in();
	}
}

READ_LINE_MEMBER( a2bus_midi_device::rx_in )
{
	return m_rx_state;
}

WRITE_LINE_MEMBER( a2bus_midi_device::tx_out )
{
	m_mdout->tx(state);
}
