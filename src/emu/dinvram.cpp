// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    dinvram.c

    Device NVRAM interfaces.

***************************************************************************/

#include "emu.h"



//**************************************************************************
//  DEVICE NVRAM INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_nvram_interface - constructor
//-------------------------------------------------

device_nvram_interface::device_nvram_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "nvram")
{
}


//-------------------------------------------------
//  ~device_nvram_interface - destructor
//-------------------------------------------------

device_nvram_interface::~device_nvram_interface()
{
}
