// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS 8706 Speech Glue Logic ASIC emulation

**********************************************************************/

#include "mos8706.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definition
const device_type MOS8706 = &device_creator<mos8706_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mos8706_device - constructor
//-------------------------------------------------

mos8706_device::mos8706_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MOS8706, "MOS8706", tag, owner, clock, "mos8706", __FILE__)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mos8706_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mos8706_device::device_reset()
{
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t mos8706_device::read(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return 0;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void mos8706_device::write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
}
