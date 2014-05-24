// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    QIMI (QL Internal Mouse Interface) emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "qimi.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define MOUSEX_TAG              "MOUSEX"
#define MOUSEY_TAG              "MOUSEY"
#define MOUSEB_TAG              "MOUSEB"


// Mouse bits in Sandy port order
#define MOUSE_MIDDLE            0x02
#define MOUSE_RIGHT             0x04
#define MOUSE_LEFT              0x08
#define MOUSE_DIRY              0x10
#define MOUSE_DIRX              0x20
#define MOUSE_INTY              0x40
#define MOUSE_INTX              0x80
#define MOUSE_INT_MASK          (MOUSE_INTX | MOUSE_INTY)



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type QIMI = &device_creator<qimi_t>;


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( qimi )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( qimi )
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor qimi_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( qimi );
}


//-------------------------------------------------
//  INPUT_PORTS( qimi )
//-------------------------------------------------

static INPUT_PORTS_START( qimi )
	PORT_START(MOUSEX_TAG)
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_MINMAX(0, 255) PORT_PLAYER(1)

	PORT_START(MOUSEY_TAG)
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_MINMAX(0, 255) PORT_PLAYER(1)

	PORT_START(MOUSEB_TAG)  /* Mouse buttons */
	PORT_BIT( MOUSE_RIGHT, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Mouse Button 1") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT( MOUSE_LEFT,  IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Mouse Button 2") PORT_CODE(MOUSECODE_BUTTON2)
	PORT_BIT( MOUSE_MIDDLE, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("Mouse Button 3") PORT_CODE(MOUSECODE_BUTTON3)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor qimi_t::device_input_ports() const
{
	return INPUT_PORTS_NAME( qimi );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  qimi_t - constructor
//-------------------------------------------------

qimi_t::qimi_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, QIMI, "QIMI", tag, owner, clock, "qimi", __FILE__),
	device_ql_expansion_card_interface(mconfig, *this),
	m_mousex(*this, MOUSEX_TAG),
	m_mousey(*this, MOUSEY_TAG),
	m_mouseb(*this, MOUSEB_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void qimi_t::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void qimi_t::device_reset()
{
}
// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    QIMI (QL Internal Mouse Interface) emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "qimi.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define MOUSEX_TAG              "MOUSEX"
#define MOUSEY_TAG              "MOUSEY"
#define MOUSEB_TAG              "MOUSEB"


// Mouse bits in Sandy port order
#define MOUSE_MIDDLE            0x02
#define MOUSE_RIGHT             0x04
#define MOUSE_LEFT              0x08
#define MOUSE_DIRY              0x10
#define MOUSE_DIRX              0x20
#define MOUSE_INTY              0x40
#define MOUSE_INTX              0x80
#define MOUSE_INT_MASK          (MOUSE_INTX | MOUSE_INTY)



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type QIMI = &device_creator<qimi_t>;


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( qimi )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( qimi )
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor qimi_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( qimi );
}


//-------------------------------------------------
//  INPUT_PORTS( qimi )
//-------------------------------------------------

static INPUT_PORTS_START( qimi )
	PORT_START(MOUSEX_TAG)
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_MINMAX(0, 255) PORT_PLAYER(1)

	PORT_START(MOUSEY_TAG)
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_MINMAX(0, 255) PORT_PLAYER(1)

	PORT_START(MOUSEB_TAG)  /* Mouse buttons */
	PORT_BIT( MOUSE_RIGHT, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Mouse Button 1") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT( MOUSE_LEFT,  IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Mouse Button 2") PORT_CODE(MOUSECODE_BUTTON2)
	PORT_BIT( MOUSE_MIDDLE, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("Mouse Button 3") PORT_CODE(MOUSECODE_BUTTON3)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor qimi_t::device_input_ports() const
{
	return INPUT_PORTS_NAME( qimi );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  qimi_t - constructor
//-------------------------------------------------

qimi_t::qimi_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, QIMI, "QIMI", tag, owner, clock, "qimi", __FILE__),
	device_ql_expansion_card_interface(mconfig, *this),
	m_mousex(*this, MOUSEX_TAG),
	m_mousey(*this, MOUSEY_TAG),
	m_mouseb(*this, MOUSEB_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void qimi_t::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void qimi_t::device_reset()
{
}
