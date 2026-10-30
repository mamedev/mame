// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    TeleNova Compis Winchester Disk Controller emulation

**********************************************************************/

#include "emu.h"
#include "compis_hdc.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(COMPIS_HDC, compis_hdc_device, "compis_hdc", "Compis Winchester disk controller")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void compis_hdc_device::device_add_mconfig(machine_config &config)
{
	WD1002_HD0(config, m_hdc);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  compis_hdc_device - constructor
//-------------------------------------------------

compis_hdc_device::compis_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, COMPIS_HDC, tag, owner, clock),
	device_isbx_card_interface(mconfig, *this),
	m_hdc(*this, "wd1002")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void compis_hdc_device::device_start()
{
}


//-------------------------------------------------
//  mcs0_r - chip select 0 read
//-------------------------------------------------

uint8_t compis_hdc_device::mcs0_r(offs_t offset)
{
	return m_hdc->read(offset & 0x07);
}


//-------------------------------------------------
//  mcs0_w - chip select 0 write
//-------------------------------------------------

void compis_hdc_device::mcs0_w(offs_t offset, uint8_t data)
{
	m_hdc->write(offset & 0x07, data);
}
