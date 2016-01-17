// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    midioutport.c

    MIDI Out serial port - glues the image device to the pluggable serial port

*********************************************************************/

#include "midioutport.h"

const device_type MIDIOUT_PORT = &device_creator<midiout_port_device>;

midiout_port_device::midiout_port_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MIDIOUT_PORT, "MIDI Out port", tag, owner, clock, "midiout_port", __FILE__),
	device_midi_port_interface(mconfig, *this),
	m_midiout(*this, "midioutimg")
{
}

static MACHINE_CONFIG_FRAGMENT(midiout_port_config)
	MCFG_MIDIOUT_ADD("midioutimg")
MACHINE_CONFIG_END

machine_config_constructor midiout_port_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(midiout_port_config);
}
