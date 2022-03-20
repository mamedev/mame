// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Zilog eZ80 CPU

    TODO: all differences from Z80

***************************************************************************/

#include "emu.h"
#include "ez80.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(EZ80, ez80_device, "ez80", "Zilog eZ80")


//**************************************************************************
//  DEVICE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  ez80_device - constructor
//-------------------------------------------------

ez80_device::ez80_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: z80_device(mconfig, type, tag, owner, clock)
{
}

ez80_device::ez80_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ez80_device(mconfig, EZ80, tag, owner, clock)
{
}
