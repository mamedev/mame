// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA VIP ASCII Keyboard Interface VP-620 emulation

**********************************************************************/

#include "emu.h"
#include "vp620.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VP620, vp620_device, "vp620", "VP-620 ASCII Keyboard")


//-------------------------------------------------
//  ASCII_KEYBOARD_INTERFACE( kb_intf )
//-------------------------------------------------

void vp620_device::kb_w(uint8_t data)
{
	m_keydata = data;

	m_slot->inst_w(0);
	m_slot->inst_w(1);

	m_keystb = ASSERT_LINE;
}

//-------------------------------------------------
//  MACHINE_CONFIG_START( vp620 )
//-------------------------------------------------

static MACHINE_CONFIG_START( vp620 )
	MCFG_DEVICE_ADD("keyboard", GENERIC_KEYBOARD, 0)
	MCFG_GENERIC_KEYBOARD_CB(PUT(vp620_device, kb_w))
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor vp620_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( vp620 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vp620_device - constructor
//-------------------------------------------------

vp620_device::vp620_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VP620, tag, owner, clock),
	device_vip_byteio_port_interface(mconfig, *this),
	m_keydata(0),
	m_keystb(CLEAR_LINE)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vp620_device::device_start()
{
}


//-------------------------------------------------
//  vip_in_r - byte input read
//-------------------------------------------------

uint8_t vp620_device::vip_in_r()
{
	return m_keydata;
}


//-------------------------------------------------
//  vip_ef3_r - EF3 flag read
//-------------------------------------------------

int vp620_device::vip_ef4_r()
{
	int state = m_keystb;

	m_keystb = CLEAR_LINE;

	return state;
}
