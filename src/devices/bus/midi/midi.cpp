// license:BSD-3-Clause
// copyright-holders:R. Belmont
#include "midi.h"

const device_type MIDI_PORT = &device_creator<midi_port_device>;

midi_port_device::midi_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MIDI_PORT, "Midi Port", tag, owner, clock, "midi_port", __FILE__),
	device_slot_interface(mconfig, *this),
	m_rxd(0),
	m_rxd_handler(*this),
	m_dev(NULL)
{
}

midi_port_device::~midi_port_device()
{
}

void midi_port_device::device_config_complete()
{
	m_dev = dynamic_cast<device_midi_port_interface *>(get_card_device());
}

void midi_port_device::device_start()
{
	m_rxd_handler.resolve_safe();
}

WRITE_LINE_MEMBER( midi_port_device::write_txd )
{
	if(m_dev)
		m_dev->input_txd(state);
}

device_midi_port_interface::device_midi_port_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
	m_port = dynamic_cast<midi_port_device *>(device.owner());
}

device_midi_port_interface::~device_midi_port_interface()
{
}

#include "bus/midi/midiinport.h"

SLOT_INTERFACE_START(midiin_slot)
	SLOT_INTERFACE("midiin", MIDIIN_PORT)
SLOT_INTERFACE_END

#include "bus/midi/midioutport.h"

SLOT_INTERFACE_START(midiout_slot)
	SLOT_INTERFACE("midiout", MIDIOUT_PORT)
SLOT_INTERFACE_END
