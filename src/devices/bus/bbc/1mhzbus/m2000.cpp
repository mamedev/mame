// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Hybrid Music 2000 Interface

    https://www.retro-kit.co.uk/page.cfm/content/Hybrid-Music-2000-Interface/
    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Hybrid_M2000.html

**********************************************************************/


#include "emu.h"
#include "m2000.h"
#include "bus/midi/midi.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_M2000, bbc_m2000_device, "bbc_m2000", "Hybrid Music 2000 Interface");


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_m2000_device::device_add_mconfig(machine_config &config)
{
	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::irq_w));

	ACIA6850(config, m_acia1, 0);
	m_acia1->txd_handler().set("mdout1", FUNC(midi_port_device::write_txd));
	m_acia1->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));

	ACIA6850(config, m_acia2, 0);
	m_acia2->txd_handler().set("mdout2", FUNC(midi_port_device::write_txd));
	m_acia2->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<1>));

	ACIA6850(config, m_acia3, 0);
	m_acia3->txd_handler().set("mdout3", FUNC(midi_port_device::write_txd));
	m_acia3->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<2>));

	CLOCK(config, m_acia_clock, DERIVED_CLOCK(1, 2));
	m_acia_clock->signal_handler().set(FUNC(bbc_m2000_device::write_acia_clock));

	midiout_slot(MIDI_PORT(config, "mdout1"));
	midiout_slot(MIDI_PORT(config, "mdout2"));
	midiout_slot(MIDI_PORT(config, "mdout3"));

	auto &mdin(MIDI_PORT(config, "mdin"));
	midiin_slot(mdin);
	mdin.rxd_handler().set(m_acia3, FUNC(acia6850_device::write_rxd));

	BBC_1MHZBUS_SLOT(config, m_1mhzbus, DERIVED_CLOCK(1, 1), bbc_1mhzbus_devices, nullptr);
	m_1mhzbus->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<3>));
	m_1mhzbus->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::nmi_w));
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_m2000_device - constructor
//-------------------------------------------------

bbc_m2000_device::bbc_m2000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_M2000, tag, owner, clock)
	, device_bbc_1mhzbus_interface(mconfig, *this)
	, m_1mhzbus(*this, "1mhzbus")
	, m_acia1(*this, "acia1")
	, m_acia2(*this, "acia2")
	, m_acia3(*this, "acia3")
	, m_acia_clock(*this, "acia_clock")
	, m_irqs(*this, "irqs")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_m2000_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER(bbc_m2000_device::fred_r)
{
	uint8_t data = 0xff;

	if (offset >= 0x08 && offset < 0x10)
	{
		switch (offset & 0x06)
		{
		case 0x00:
			data = m_acia1->read(offset & 1);
			break;
		case 0x02:
			data = m_acia2->read(offset & 1);
			break;
		case 0x04:
			data = m_acia3->read(offset & 1);
			break;
		case 0x06:
			break;
		}
	}

	data &= m_1mhzbus->fred_r(space, offset);

	return data;
}

WRITE8_MEMBER(bbc_m2000_device::fred_w)
{
	if (offset >= 0x08 && offset < 0x10)
	{
		switch (offset & 0x06)
		{
		case 0x00:
			m_acia1->write(offset & 1, data);
			break;
		case 0x02:
			m_acia2->write(offset & 1, data);
			break;
		case 0x04:
			m_acia3->write(offset & 1, data);
			break;
		case 0x06:
			break;
		}
	}

	m_1mhzbus->fred_w(space, offset, data);
}

READ8_MEMBER(bbc_m2000_device::jim_r)
{
	uint8_t data = 0xff;

	data &= m_1mhzbus->jim_r(space, offset);

	return data;
}

WRITE8_MEMBER(bbc_m2000_device::jim_w)
{
	m_1mhzbus->jim_w(space, offset, data);
}

WRITE_LINE_MEMBER(bbc_m2000_device::write_acia_clock)
{
	m_acia1->write_txc(state);
	m_acia1->write_rxc(state);
	m_acia2->write_txc(state);
	m_acia2->write_rxc(state);
	m_acia3->write_txc(state);
	m_acia3->write_rxc(state);
}
