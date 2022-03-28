// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    The Serial Port MIDI Interface

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesH2Z/TheSerialPort_MIDI.html

    The Serial Port Sampler and MIDI Interface

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesH2Z/TheSerialPort_SamplerMIDI.html

    TODO:
    - add sampler using ZN439E

**********************************************************************/

#include "emu.h"
#include "midi.h"
#include "bus/midi/midi.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ARC_SERIAL_MIDI, arc_serial_midi_device, "arc_serial_midi", "The Serial Port MIDI Interface");
DEFINE_DEVICE_TYPE(ARC_SERIAL_SAMPLER, arc_serial_sampler_device, "arc_serial_sampler", "The Serial Port Sampler and MIDI Interface");


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_serial_midi_device::device_add_mconfig(machine_config &config)
{
	MC6854(config, m_adlc);
	m_adlc->out_txd_cb().set("mdout", FUNC(midi_port_device::write_txd));
	m_adlc->out_irq_cb().set(DEVICE_SELF_OWNER, FUNC(archimedes_econet_slot_device::efiq_w));

	midi_port_device &mdin(MIDI_PORT(config, "mdin", midiin_slot, "midiin"));
	mdin.rxd_handler().set(m_adlc, FUNC(mc6854_device::set_rx));
	MIDI_PORT(config, "mdout", midiout_slot, "midiout");
}

void arc_serial_sampler_device::device_add_mconfig(machine_config &config)
{
	arc_serial_midi_device::device_add_mconfig(config);

	// ZN439E
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_serial_midi_device - constructor
//-------------------------------------------------

arc_serial_midi_device::arc_serial_midi_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_archimedes_econet_interface(mconfig, *this)
	, m_adlc(*this, "mc6854")
{
}

arc_serial_midi_device::arc_serial_midi_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: arc_serial_midi_device(mconfig, ARC_SERIAL_MIDI, tag, owner, clock)
{
}

arc_serial_sampler_device::arc_serial_sampler_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: arc_serial_midi_device(mconfig, ARC_SERIAL_SAMPLER, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_serial_midi_device::device_start()
{
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

u8 arc_serial_midi_device::read(offs_t offset)
{
	return m_adlc->read(offset);
}

void arc_serial_midi_device::write(offs_t offset, u8 data)
{
	m_adlc->write(offset, data);
}
