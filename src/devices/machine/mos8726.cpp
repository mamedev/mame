// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS 8726R1 DMA Controller emulation

**********************************************************************/

/*

    TODO:

    - all

*/

#include "emu.h"
#include "mos8726.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************



//**************************************************************************
//  DEVICE TYPE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MOS8726, mos8726_device, "mos8726", "MOS 8726 DMA Controller")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mos8726_device - constructor
//-------------------------------------------------

mos8726_device::mos8726_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MOS8726, tag, owner, clock)
	, device_execute_interface(mconfig, *this)
	, m_icount(0)
	, m_bs(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mos8726_device::device_start()
{
	// set our instruction counter
	set_icountptr(m_icount);

	// save state
	save_item(NAME(m_bs));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mos8726_device::device_reset()
{
}


//-------------------------------------------------
//  execute_run -
//-------------------------------------------------

void mos8726_device::execute_run()
{
	do
	{
		m_icount--;
	} while (m_icount > 0);
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t mos8726_device::read(offs_t offset)
{
	uint8_t data = 0;

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void mos8726_device::write(offs_t offset, uint8_t data)
{
}


//-------------------------------------------------
//  bs_w - bank select write
//-------------------------------------------------

WRITE_LINE_MEMBER( mos8726_device::bs_w )
{
	m_bs = state;
}


//-------------------------------------------------
//  romsel_r - ROM select read
//-------------------------------------------------

int mos8726_device::romsel_r(int roml, int romh)
{
	return roml && romh;
}
