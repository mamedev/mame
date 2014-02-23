#include "midi.h"

const device_type MIDI_PORT = &device_creator<midi_port_device>;

device_midi_port_interface::device_midi_port_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
}

device_midi_port_interface::~device_midi_port_interface()
{
}

midi_port_device::midi_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MIDI_PORT, "Midi Port", tag, owner, clock, "midi_port", __FILE__),
	device_slot_interface(mconfig, *this),
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
	if (m_dev)
	{
		m_dev->set_my_port_device(this); 
	}
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

