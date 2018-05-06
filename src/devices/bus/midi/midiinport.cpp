// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    midiinport.cpp

    MIDI In serial port - glues the image device to the pluggable serial port

*********************************************************************/

#include "emu.h"
#include "midiinport.h"

DEFINE_DEVICE_TYPE(MIDIIN_PORT, midiin_port_device, "midiin_port", "MIDI In port")

midiin_port_device::midiin_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MIDIIN_PORT, tag, owner, clock),
	device_midi_port_interface(mconfig, *this),
	m_midiin(*this, "midiinimg")
{
}

MACHINE_CONFIG_START(midiin_port_device::device_add_mconfig)
	MCFG_DEVICE_ADD("midiinimg", MIDIIN, 0)
	MCFG_MIDIIN_INPUT_CB(WRITELINE(*this, midiin_port_device, read))
MACHINE_CONFIG_END
