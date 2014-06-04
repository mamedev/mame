// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CST Q+4 emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "cst_q_plus4.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type CST_Q_PLUS4 = &device_creator<cst_q_plus4_t>;


//-------------------------------------------------
//  ROM( cst_q_plus4 )
//-------------------------------------------------

ROM_START( cst_q_plus4 )
	ROM_REGION( 0x2000, "rom", 0 )
	ROM_LOAD( "qplus4.rom", 0x0000, 0x2000, CRC(53a078fb) SHA1(53d6828c1b6ba052b862fd80ac8a364b0078330d) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *cst_q_plus4_t::device_rom_region() const
{
	return ROM_NAME( cst_q_plus4 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cst_q_plus4_t - constructor
//-------------------------------------------------

cst_q_plus4_t::cst_q_plus4_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, CST_Q_PLUS4, "CST Q+4", tag, owner, clock, "ql_qplus4", __FILE__),
	device_ql_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cst_q_plus4_t::device_start()
{
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

UINT8 cst_q_plus4_t::read(address_space &space, offs_t offset, UINT8 data)
{
	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void cst_q_plus4_t::write(address_space &space, offs_t offset, UINT8 data)
{
}
