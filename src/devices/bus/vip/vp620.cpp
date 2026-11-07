// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA VIP VP-620 ASCII Keyboard Cable emulation

**********************************************************************/

#include "emu.h"
#include "vp620.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VP620, vp620_device, "vp620", "VP-620 ASCII Keyboard Cable")


//-------------------------------------------------
//  machine_config( vp620 )
//-------------------------------------------------

void vp620_device::device_add_mconfig(machine_config &config)
{
	VIP_ASCII_KEYBOARD_PORT(config, m_kb, vip_ascii_keyboards, "vp601");
	m_kb->data_callback().set(FUNC(vp620_device::data_w));
	m_kb->st_callback().set(FUNC(vp620_device::st_w));
	m_kb->kd_callback().set(FUNC(vp620_device::kd_w));
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
	m_kb(*this, "kb"),
	m_data(0),
	m_kd(false)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vp620_device::device_start()
{
	save_item(NAME(m_data));
	save_item(NAME(m_kd));
}


//-------------------------------------------------
//  vip_in_r - byte input read
//-------------------------------------------------

uint8_t vp620_device::vip_in_r()
{
	return m_data;
}


//-------------------------------------------------
//  vip_ef4_r - EF4 flag read
//-------------------------------------------------

int vp620_device::vip_ef4_r()
{
	return m_kd;
}
