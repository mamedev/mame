// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    midiinport.c

    MIDI In serial port - glues the image device to the pluggable serial port

*********************************************************************/

#include "midiinport.h"

const device_type MIDIIN_PORT = &device_creator<midiin_port_device>;

midiin_port_device::midiin_port_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MIDIIN_PORT, "MIDI In port", tag, owner, clock, "midiin_port", __FILE__),
	device_midi_port_interface(mconfig, *this),
	m_midiin(*this, "midiinimg")
{
}

static MACHINE_CONFIG_FRAGMENT(midiin_port_config)
	MCFG_DEVICE_ADD("midiinimg", MIDIIN, 0)
	MCFG_MIDIIN_INPUT_CB(WRITELINE(midiin_port_device, read))
MACHINE_CONFIG_END

machine_config_constructor midiin_port_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(midiin_port_config);
}
