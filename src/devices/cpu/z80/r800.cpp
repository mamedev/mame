// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    ASCII R800 CPU

    TODO: this uses a sped-up Z80 core with added multiply instructions
    ('mulub', 'muluw').

***************************************************************************/

#include "emu.h"
#include "r800.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(R800, r800_device, "r800", "ASCII R800")


//**************************************************************************
//  DEVICE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  r800_device - constructor
//-------------------------------------------------

r800_device::r800_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: z80_device(mconfig, R800, tag, owner, clock)
{
}
