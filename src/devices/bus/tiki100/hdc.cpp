// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    TIKI-100 Winchester controller card emulation

**********************************************************************/

#include "hdc.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define WD1010_TAG  "hdc"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type TIKI100_HDC = &device_creator<tiki100_hdc_t>;


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( tiki100_hdc )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( tiki100_hdc )
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


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor tiki100_hdc_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( tiki100_hdc );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tiki100_hdc_t - constructor
//-------------------------------------------------

tiki100_hdc_t::tiki100_hdc_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, TIKI100_HDC, "TIKI-100 Winchester controller", tag, owner, clock, "tiki100_hdc", __FILE__),
	device_tiki100bus_card_interface(mconfig, *this),
	m_hdc(*this, WD1010_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tiki100_hdc_t::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tiki100_hdc_t::device_reset()
{
	m_hdc->reset();
}


//-------------------------------------------------
//  tiki100bus_iorq_r - I/O read
//-------------------------------------------------

UINT8 tiki100_hdc_t::iorq_r(address_space &space, offs_t offset, UINT8 data)
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

void tiki100_hdc_t::iorq_w(address_space &space, offs_t offset, UINT8 data)
{
	if ((offset & 0xf8) == 0x20)
	{
		m_hdc->write(space, offset, data);
	}
}
