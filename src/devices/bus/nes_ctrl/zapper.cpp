// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer & Entertainment System Zapper Lightgun

**********************************************************************/

#include "emu.h"
#include "zapper.h"
#include "screen.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(NES_ZAPPER, nes_zapper_device, "nes_zapper", "Nintendo Zapper Lightgun")


static INPUT_PORTS_START( nes_zapper )
	PORT_START("ZAPPER_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(30) PORT_MINMAX(0,255)
	PORT_START("ZAPPER_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(30) PORT_MINMAX(0,255)
	PORT_START("ZAPPER_T")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Lightgun Trigger")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor nes_zapper_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( nes_zapper );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nes_zapper_device - constructor
//-------------------------------------------------

nes_zapper_device::nes_zapper_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NES_ZAPPER, tag, owner, clock),
	device_video_interface(mconfig, *this),
	device_nes_control_port_interface(mconfig, *this),
	m_lightx(*this, "ZAPPER_X"),
	m_lighty(*this, "ZAPPER_Y"),
	m_trigger(*this, "ZAPPER_T")
{
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void nes_zapper_device::device_start()
{
}


//-------------------------------------------------
//  device_reset
//-------------------------------------------------

void nes_zapper_device::device_reset()
{
}


//-------------------------------------------------
//  read
//-------------------------------------------------

uint8_t nes_zapper_device::read_bit34()
{
	uint8_t ret = m_trigger->read();
	int x = m_lightx->read();
	int y = m_lighty->read();

	// update the screen if necessary
	if (!screen().vblank())
	{
		int vpos = screen().vpos();
		int hpos = screen().hpos();

		if (vpos > y || (vpos == y && hpos >= x))
			screen().update_now();
	}

	// get the pixel at the gun position
	rgb_t pix = screen().pixel(x, y);

	// check if the cursor is over a bright pixel
	// FIXME: still a gross hack
	if (pix.r() == 0xff && pix.b() == 0xff && pix.g() > 0x90)
		ret &= ~0x08; // sprite hit
	else
		ret |= 0x08;  // no sprite hit

	return ret;
}

uint8_t nes_zapper_device::read_exp(offs_t offset)
{
	uint8_t ret = 0;
	if (offset == 1)    // $4017
		ret |= nes_zapper_device::read_bit34();
	return ret;
}
