// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    TIKI-100 Winchester controller card emulation

**********************************************************************/

#include "emu.h"
#include "hdc.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define WD1010_TAG  "hdc"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(TIKI100_HDC, tiki100_hdc_device, "tiki100_hdc", "TIKI-100 Winchester controller")


//-------------------------------------------------
//  MACHINE_CONFIG_START( tiki100_hdc )
//-------------------------------------------------

MACHINE_CONFIG_START(tiki100_hdc_device::device_add_mconfig)
	MCFG_DEVICE_ADD(WD1010_TAG, WD2010, 5000000)
	//MCFG_WD2010_OUT_INTRQ_CB()
	MCFG_WD2010_IN_DRDY_CB(VCC)
	MCFG_WD2010_IN_INDEX_CB(VCC)
	MCFG_WD2010_IN_WF_CB(VCC)
	MCFG_WD2010_IN_TK000_CB(VCC)
	MCFG_WD2010_IN_SC_CB(VCC)

	MCFG_HARDDISK_ADD("hard0")
	MCFG_HARDDISK_ADD("hard1")
MACHINE_CONFIG_END



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tiki100_hdc_device - constructor
//-------------------------------------------------

tiki100_hdc_device::tiki100_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TIKI100_HDC, tag, owner, clock),
	device_tiki100bus_card_interface(mconfig, *this),
	m_hdc(*this, WD1010_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tiki100_hdc_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tiki100_hdc_device::device_reset()
{
	m_hdc->reset();
}


//-------------------------------------------------
//  tiki100bus_iorq_r - I/O read
//-------------------------------------------------

uint8_t tiki100_hdc_device::iorq_r(address_space &space, offs_t offset, uint8_t data)
{
	if ((offset & 0xf8) == 0x20)
	{
		data = m_hdc->read(space, offset & 0x07);
	}

	return data;
}


//-------------------------------------------------
//  tiki100bus_iorq_w - I/O write
//-------------------------------------------------

void tiki100_hdc_device::iorq_w(address_space &space, offs_t offset, uint8_t data)
{
	if ((offset & 0xf8) == 0x20)
	{
		m_hdc->write(space, offset, data);
	}
}
