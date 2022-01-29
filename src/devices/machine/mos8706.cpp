// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS 8706 Speech Glue Logic ASIC emulation

**********************************************************************/

#include "emu.h"
#include "mos8706.h"

//#define VERBOSE 1
#include "logmacro.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(MOS8706, mos8706_device, "mos8706", "MOS 8706")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mos8706_device - constructor
//-------------------------------------------------

mos8706_device::mos8706_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MOS8706, tag, owner, clock)
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

uint8_t mos8706_device::read(offs_t offset)
{
	return 0;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void mos8706_device::write(offs_t offset, uint8_t data)
{
}
