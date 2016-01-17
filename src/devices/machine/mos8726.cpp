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

mos8726_device::mos8726_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
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

READ8_MEMBER( mos8726_device::read )
{
	UINT8 data = 0;

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( mos8726_device::write )
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
