// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2midi.c

    Apple II 6850 MIDI card, as made by Passport, Yamaha, and others.

*********************************************************************/

#include "emu.h"
#include "a2midi.h"

#include "bus/midi/midi.h"
#include "machine/6840ptm.h"
#include "machine/6850acia.h"
#include "machine/clock.h"


namespace {

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define MIDI_PTM_TAG     "midi_ptm"
#define MIDI_ACIA_TAG    "midi_acia"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_midi_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_midi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_midi_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;

	required_device<ptm6840_device> m_ptm;
	required_device<acia6850_device> m_acia;

private:
	void acia_irq_w(int state);
	void ptm_irq_w(int state);
	void write_acia_clock(int state);

	bool m_acia_irq, m_ptm_irq;
};

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_midi_device::device_add_mconfig(machine_config &config)
{
	PTM6840(config, m_ptm, 1021800);
	m_ptm->set_external_clocks(1021800.0f, 1021800.0f, 1021800.0f);
	m_ptm->irq_callback().set(FUNC(a2bus_midi_device::ptm_irq_w));

	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set("mdout", FUNC(midi_port_device::write_txd));
	m_acia->irq_handler().set(FUNC(a2bus_midi_device::acia_irq_w));

	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));

	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	clock_device &acia_clock(CLOCK(config, "acia_clock", 31250*16));
	acia_clock.signal_handler().set(FUNC(a2bus_midi_device::write_acia_clock));
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
}

void a2bus_midi_device::device_reset()
{
	m_acia_irq = m_ptm_irq = false;
}

/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_midi_device::read_c0nx(uint8_t offset)
{
	// PTM at C0n0-C0n7, ACIA at C0n8-C0n9, drum sync (?) at C0nA-C0nB

	if (offset < 8)
	{
		return m_ptm->read(offset & 7);
	}
	else if (offset == 8 || offset == 9)
	{
		return m_acia->read(offset & 1);
	}

	return 0;
}

/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_midi_device::write_c0nx(uint8_t offset, uint8_t data)
{
	if (offset < 8)
	{
		m_ptm->write(offset & 7, data);
	}
	else if (offset == 8 || offset == 9)
	{
		m_acia->write(offset & 1, data);
	}
}

void a2bus_midi_device::acia_irq_w(int state)
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

void a2bus_midi_device::ptm_irq_w(int state)
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

void a2bus_midi_device::write_acia_clock(int state)
{
	m_acia->write_txc(state);
	m_acia->write_rxc(state);
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_MIDI, device_a2bus_card_interface, a2bus_midi_device, "a2midi", "6850 MIDI card")
