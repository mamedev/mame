// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2midi.c

    Apple II 6850 MIDI card, as made by Passport, Yamaha, and others.

*********************************************************************/

#include "emu.h"
#include "a2midi.h"
#include "machine/clock.h"
#include "bus/midi/midi.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_MIDI, a2bus_midi_device, "a2midi", "6850 MIDI card")

#define MIDI_PTM_TAG     "midi_ptm"
#define MIDI_ACIA_TAG    "midi_acia"

MACHINE_CONFIG_FRAGMENT( midi )
	MCFG_DEVICE_ADD(MIDI_PTM_TAG, PTM6840, 1021800)
	MCFG_PTM6840_EXTERNAL_CLOCKS(1021800.0f, 1021800.0f, 1021800.0f)
	MCFG_PTM6840_IRQ_CB(WRITELINE(a2bus_midi_device, ptm_irq_w))

	MCFG_DEVICE_ADD(MIDI_ACIA_TAG, ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(DEVWRITELINE("mdout", midi_port_device, write_txd))
	MCFG_ACIA6850_IRQ_HANDLER(WRITELINE(a2bus_midi_device, acia_irq_w))

	MCFG_MIDI_PORT_ADD("mdin", midiin_slot, "midiin")
	MCFG_MIDI_RX_HANDLER(DEVWRITELINE(MIDI_ACIA_TAG, acia6850_device, write_rxd))

	MCFG_MIDI_PORT_ADD("mdout", midiout_slot, "midiout")

	MCFG_DEVICE_ADD("acia_clock", CLOCK, 31250*16)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(a2bus_midi_device, write_acia_clock))
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

a2bus_midi_device::a2bus_midi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		a2bus_midi_device(mconfig, A2BUS_MIDI, tag, owner, clock)
{
}

a2bus_midi_device::a2bus_midi_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, type, tag, owner, clock),
		device_a2bus_card_interface(mconfig, *this),
		m_ptm(*this, MIDI_PTM_TAG),
		m_acia(*this, MIDI_ACIA_TAG), m_acia_irq(false),
		m_ptm_irq(false)
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
}

/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_midi_device::read_c0nx(address_space &space, uint8_t offset)
{
	// PTM at C0n0-C0n7, ACIA at C0n8-C0n9, drum sync (?) at C0nA-C0nB

	if (offset < 8)
	{
		return m_ptm->read(space, offset & 7);
	}
	else if (offset == 8)
	{
		return m_acia->status_r(space, 0);
	}
	else if (offset == 9)
	{
		uint8_t ret = m_acia->data_r(space, 0);
		return ret;
	}

	return 0;
}

/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_midi_device::write_c0nx(address_space &space, uint8_t offset, uint8_t data)
{
	if (offset < 8)
	{
		m_ptm->write(space, offset & 7, data);
	}
	else if (offset == 8)
	{
		m_acia->control_w(space, 0, data);
	}
	else if (offset == 9)
	{
		m_acia->data_w(space, 0, data);
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

WRITE_LINE_MEMBER( a2bus_midi_device::write_acia_clock )
{
	m_acia->write_txc(state);
	m_acia->write_rxc(state);
}
