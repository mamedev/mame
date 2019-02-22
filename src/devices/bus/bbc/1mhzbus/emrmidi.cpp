// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    EMR BBC Midi Interface

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/EMR_BBCMIDI.html

    IUB-1 standard model.
    IUB-2 updated model to also sync with Korg rhythm units.
    IUB-2T, as IUB-2, with Sync to Tape for supplying click track
    to multitrackers.

    The Clock Start/Stop port is not implemented. It requires the
    device to be also connected to the Userport, providing timing
    control on PB0, PB1.

**********************************************************************/


#include "emu.h"
#include "emrmidi.h"
#include "bus/midi/midi.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_EMRMIDI, bbc_emrmidi_device, "bbc_emrmidi", "EMR BBC Midi Interface");


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_emrmidi_device::device_add_mconfig(machine_config &config)
{
	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set("mdout1", FUNC(midi_port_device::write_txd));
	m_acia->txd_handler().append("mdout2", FUNC(midi_port_device::write_txd));
	m_acia->irq_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::irq_w));

	CLOCK(config, m_acia_clock, 2_MHz_XTAL / 4);
	m_acia_clock->signal_handler().set(FUNC(bbc_emrmidi_device::write_acia_clock));

	midiout_slot(MIDI_PORT(config, "mdout1"));
	midiout_slot(MIDI_PORT(config, "mdout2"));

	auto &mdin(MIDI_PORT(config, "mdin"));
	midiin_slot(mdin);
	mdin.rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_emrmidi_device - constructor
//-------------------------------------------------

bbc_emrmidi_device::bbc_emrmidi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_EMRMIDI, tag, owner, clock)
	, device_bbc_1mhzbus_interface(mconfig, *this)
	, m_acia(*this, "acia1")
	, m_acia_clock(*this, "acia_clock")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_emrmidi_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER(bbc_emrmidi_device::fred_r)
{
	uint8_t data = 0xff;

	if (offset >= 0xf0 && offset < 0xf2)
	{
		data = m_acia->read(offset & 1);
	}

	return data;
}

WRITE8_MEMBER(bbc_emrmidi_device::fred_w)
{
	if (offset >= 0xf0 && offset < 0xf2)
	{
		m_acia->write(offset & 1, data);
	}
}

WRITE_LINE_MEMBER(bbc_emrmidi_device::write_acia_clock)
{
	m_acia->write_txc(state);
	m_acia->write_rxc(state);
}
