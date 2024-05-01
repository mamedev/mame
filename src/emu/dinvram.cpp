// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    dinvram.cpp

    Device NVRAM interfaces.

***************************************************************************/

#include "emu.h"



//**************************************************************************
//  DEVICE NVRAM INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_nvram_interface - constructor
//-------------------------------------------------

device_nvram_interface::device_nvram_interface(const machine_config &mconfig, device_t &device, bool backup_enabled)
	: device_interface(device, "nvram")
	, m_backup_enabled(backup_enabled)
{
}


//-------------------------------------------------
//  ~device_nvram_interface - destructor
//-------------------------------------------------

device_nvram_interface::~device_nvram_interface()
{
}
