// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS 8726R1 DMA Controller emulation

**********************************************************************/

/*

    TODO:

    - all

*/

#include "mos8726.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************



//**************************************************************************
//  DEVICE TYPE DEFINITIONS
//**************************************************************************

const device_type MOS8726 = &device_creator<mos8726_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mos8726_device - constructor
//-------------------------------------------------

mos8726_device::mos8726_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MOS8726, "MOS8726", tag, owner, clock, "mos8726", __FILE__),
		device_execute_interface(mconfig, *this),
		m_icount(0),
		m_bs(1)
{ }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mos8726_device::device_start()
{
	// set our instruction counter
	m_icountptr = &m_icount;

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

uint8_t mos8726_device::read(address_space &space, offs_t offset, uint8_t mem_mask)
{
	uint8_t data = 0;

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void mos8726_device::write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
}


//-------------------------------------------------
//  bs_w - bank select write
//-------------------------------------------------

void mos8726_device::bs_w(int state)
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
