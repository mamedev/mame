// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
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

DEFINE_DEVICE_TYPE(CPC_MUSICMACHINE, cpc_musicmachine_device, "cpcmusic", "The Music Machine (CPC)")


void cpc_musicmachine_device::device_add_mconfig(machine_config &config)
{
	ACIA6850(config, m_acia).txd_handler().set("mdout", FUNC(midi_port_device::write_txd));
	m_acia->irq_handler().set(DEVICE_SELF_OWNER, FUNC(cpc_expansion_slot_device::nmi_w));
	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));
	MIDI_PORT(config, "mdout", midiout_slot, "midiout");
	clock_device &acia_clock(CLOCK(config, "acia_clock", 31250*16));
	acia_clock.signal_handler().set(FUNC(cpc_musicmachine_device::write_acia_clock));

	SPEAKER(config, "speaker").front_center();
	ZN429E(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.2);

	// no pass-through
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

cpc_musicmachine_device::cpc_musicmachine_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CPC_MUSICMACHINE, tag, owner, clock),
	device_cpc_expansion_card_interface(mconfig, *this),
	m_slot(nullptr),
	m_acia(*this,"acia"),
	m_dac(*this,"dac"),
	m_irq_select(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cpc_musicmachine_device::device_start()
{
	m_slot = dynamic_cast<cpc_expansion_slot_device *>(owner());
	address_space &space = m_slot->cpu().space(AS_IO);

	space.install_write_handler(0xf8e8,0xf8e8, write8smo_delegate(*this, FUNC(cpc_musicmachine_device::irqsel_w)));
	space.install_readwrite_handler(0xf8ec,0xf8ef, read8sm_delegate(*this, FUNC(cpc_musicmachine_device::acia_r)), write8sm_delegate(*this, FUNC(cpc_musicmachine_device::acia_w)));
	space.install_write_handler(0xf8f0,0xf8f0, write8smo_delegate(*this, FUNC(cpc_musicmachine_device::dac_w)));
	// 0xf8f4 - ADC read8_delegate
	// 0xf8f8 - ADC start
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cpc_musicmachine_device::device_reset()
{
	// TODO
}

void cpc_musicmachine_device::dac_w(uint8_t data)
{
	m_dac->write(data);
}

uint8_t cpc_musicmachine_device::acia_r(offs_t offset)
{
	uint8_t ret = 0;

	switch(offset)
	{
		case 2:
			ret = m_acia->status_r();
			break;
		case 3:
			ret = m_acia->data_r();
			break;
	}

	return ret;
}

void cpc_musicmachine_device::acia_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 0:
			m_acia->control_w(data);
			break;
		case 1:
			m_acia->data_w(data);
			break;
	}
}

void cpc_musicmachine_device::irqsel_w(uint8_t data)
{
	if(data == 0x01)
		m_irq_select = true;
	else
		m_irq_select = false;
}

void cpc_musicmachine_device::irq_w(int state)
{
	if(m_irq_select)
		m_slot->nmi_w(state);
}

