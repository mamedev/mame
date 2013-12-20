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
	MCFG_SERIAL_OUT_RX_HANDLER(DEVWRITELINE(DEVICE_SELF, vic1011_device, rxd_w))
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


//-------------------------------------------------
//  vic20_pb_r - port B read
//-------------------------------------------------

UINT8 vic1011_device::vic20_pb_r(address_space &space, offs_t offset)
{
	/*

	    bit     description

	    0       Sin
	    1
	    2
	    3
	    4       DCDin
	    5
	    6       CTS
	    7       DSR

	*/

	UINT8 data = 0;

	data |= !m_rs232->rx();
	data |= m_rs232->dcd_r() << 4;
	data |= m_rs232->cts_r() << 6;
	data |= m_rs232->dsr_r() << 7;

	return data;
}


//-------------------------------------------------
//  vic20_pb_w - port B write
//-------------------------------------------------

void vic1011_device::vic20_pb_w(address_space &space, offs_t offset, UINT8 data)
{
	/*

	    bit     description

	    0
	    1       RTS
	    2       DTR
	    3
	    4
	    5       DCDout
	    6
	    7

	*/

	m_rs232->rts_w(BIT(data, 1));
	m_rs232->dtr_w(BIT(data, 2));
}


//-------------------------------------------------
//  vic20_cb2_w - CB2 write
//-------------------------------------------------

void vic1011_device::vic20_cb2_w(int state)
{
	m_rs232->tx(!state);
}


//-------------------------------------------------
//  rxd_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( vic1011_device::rxd_w )
{
	m_slot->via_cb1_w(!state);
}
