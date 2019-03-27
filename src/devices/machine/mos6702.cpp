// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS Technology 6702 Mystery Device emulation

**********************************************************************/

#include "emu.h"
#include "mos6702.h"

//#define VERBOSE 1
#include "logmacro.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MOS6702, mos6702_device, "mos6702", "MOS 6702")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mos6702_device - constructor
//-------------------------------------------------

mos6702_device::mos6702_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MOS6702, tag, owner, clock)
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

uint8_t mos6702_device::read(offs_t offset)
{
	return 0;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void mos6702_device::write(offs_t offset, uint8_t data)
{
}
