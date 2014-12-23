// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    didisasm.c

    Device disasm interfaces.

***************************************************************************/

#include "emu.h"


//**************************************************************************
//  DEVICE DISASM INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_disasm_interface - constructor
//-------------------------------------------------

device_disasm_interface::device_disasm_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "disasm")
{
}


//-------------------------------------------------
//  ~device_disasm_interface - destructor
//-------------------------------------------------

device_disasm_interface::~device_disasm_interface()
{
}
