// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS Technology 6702 Mystery Device emulation

**********************************************************************/

#include "mos6702.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type MOS6702 = &device_creator<mos6702_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mos6702_device - constructor
//-------------------------------------------------

mos6702_device::mos6702_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MOS6702, "MOS6702", tag, owner, clock, "mos6702", __FILE__)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mos6702_device::device_start()
{
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t mos6702_device::read(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return 0;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void mos6702_device::write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
}
