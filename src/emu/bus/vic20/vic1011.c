// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-1011A/B RS-232C Adapter emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "vic1011.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define RS232_TAG       "rs232"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type VIC1011 = &device_creator<vic1011_device>;


//-------------------------------------------------
//  MACHINE_DRIVER( vic1011 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( vic1011 )
	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, NULL)
	MCFG_SERIAL_OUT_RX_HANDLER(DEVWRITELINE(DEVICE_SELF, vic1011_device, write_rxd))
	MCFG_RS232_OUT_DCD_HANDLER(DEVWRITELINE(DEVICE_SELF, vic1011_device, write_dcdin))
	MCFG_RS232_OUT_CTS_HANDLER(DEVWRITELINE(DEVICE_SELF, vic1011_device, write_cts))
	MCFG_RS232_OUT_DSR_HANDLER(DEVWRITELINE(DEVICE_SELF, vic1011_device, write_dsr))
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor vic1011_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( vic1011 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic1011_device - constructor
//-------------------------------------------------

vic1011_device::vic1011_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, VIC1011, "VIC1011", tag, owner, clock, "vic1011", __FILE__),
		device_vic20_user_port_interface(mconfig, *this),
		m_rs232(*this, RS232_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic1011_device::device_start()
{
}

WRITE_LINE_MEMBER( vic1011_device::write_rxd )
{
	m_slot->m_b_handler(!state);
	m_slot->m_c_handler(!state);
}

void vic1011_device::write_d(int state)
{
	m_rs232->rts_w(state);
}

void vic1011_device::write_e(int state)
{
	m_rs232->dtr_w(state);
}

WRITE_LINE_MEMBER( vic1011_device::write_dcdin )
{
	m_slot->m_h_handler(state);
}

void vic1011_device::write_j(int state)
{
	/// dcdout
}

WRITE_LINE_MEMBER( vic1011_device::write_cts )
{
	m_slot->m_k_handler(state);
}

WRITE_LINE_MEMBER( vic1011_device::write_dsr )
{
	m_slot->m_l_handler(state);
}

void vic1011_device::write_m(int state)
{
	m_rs232->tx(!state);
}
