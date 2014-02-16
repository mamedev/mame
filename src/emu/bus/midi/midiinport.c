/*********************************************************************

    midiinport.c

    MIDI In serial port - glues the image device to the pluggable serial port

*********************************************************************/

#include "midiinport.h"

const device_type MIDIIN_PORT = &device_creator<midiin_port_device>;

midiin_port_device::midiin_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MIDIIN_PORT, "MIDI In port", tag, owner, clock, "midiin_port", __FILE__),
	device_midi_port_interface(mconfig, *this),
	m_midiin(*this, "midiinimg")
{
}

static midiin_config midiin_port_image_config =
{
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, midiin_port_device, read)
};

static MACHINE_CONFIG_FRAGMENT(midiin_port_config)
	MCFG_MIDIIN_ADD("midiinimg", midiin_port_image_config)
MACHINE_CONFIG_END

machine_config_constructor midiin_port_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(midiin_port_config);
}
