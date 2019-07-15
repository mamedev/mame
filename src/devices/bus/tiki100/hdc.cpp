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
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void tiki100_hdc_device::device_add_mconfig(machine_config & config)
{
	WD2010(config, m_hdc, 5000000);
	//m_hdc->out_intr_callback().set();
	m_hdc->in_drdy_callback().set_constant(1);
	m_hdc->in_index_callback().set_constant(1);
	m_hdc->in_wf_callback().set_constant(1);
	m_hdc->in_tk000_callback().set_constant(1);
	m_hdc->in_sc_callback().set_constant(1);

	HARDDISK(config, "hard0", 0);
	HARDDISK(config, "hard1", 0);
}



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

uint8_t tiki100_hdc_device::iorq_r(offs_t offset, uint8_t data)
{
	if ((offset & 0xf8) == 0x20)
	{
		data = m_hdc->read(offset & 0x07);
	}

	return data;
}


//-------------------------------------------------
//  tiki100bus_iorq_w - I/O write
//-------------------------------------------------

void tiki100_hdc_device::iorq_w(offs_t offset, uint8_t data)
{
	if ((offset & 0xf8) == 0x20)
	{
		m_hdc->write(offset, data);
	}
}
