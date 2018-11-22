// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Henson CFA 3000 Analogue

**********************************************************************/


#include "emu.h"
#include "cfa3000a.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CFA3000_ANLG, cfa3000_anlg_device, "cfa3000a", "Henson CFA 3000 Analogue")


//-------------------------------------------------
//  INPUT_PORTS( cfa3000a )
//-------------------------------------------------

static INPUT_PORTS_START( cfa3000a )
	PORT_START("CHANNEL0")
	PORT_BIT(0xff, 0x80, IPT_PADDLE) PORT_NAME("Background Intensity") PORT_SENSITIVITY(25) PORT_KEYDELTA(5) PORT_CENTERDELTA(0) PORT_REVERSE

	PORT_START("CHANNEL1")
	PORT_BIT(0xff, 0x00, IPT_UNKNOWN)

	PORT_START("CHANNEL2")
	PORT_BIT(0xff, 0x80, IPT_PADDLE) PORT_NAME("Age") PORT_SENSITIVITY(25) PORT_KEYDELTA(5) PORT_CENTERDELTA(0) PORT_REVERSE

	PORT_START("CHANNEL3")
	PORT_BIT(0xff, 0x00, IPT_UNKNOWN)

	PORT_START("BUTTONS")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor cfa3000_anlg_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( cfa3000a );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cfa3000_anlg_device - constructor
//-------------------------------------------------

cfa3000_anlg_device::cfa3000_anlg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CFA3000_ANLG, tag, owner, clock),
	device_bbc_analogue_interface(mconfig, *this),
	m_channel(*this, "CHANNEL%u", 0),
	m_buttons(*this, "BUTTONS")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cfa3000_anlg_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cfa3000_anlg_device::device_reset()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t cfa3000_anlg_device::ch_r(int channel)
{
	return m_channel[channel]->read();
}

uint8_t cfa3000_anlg_device::pb_r()
{
	return m_buttons->read() & 0x30;
}
