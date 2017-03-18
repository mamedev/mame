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


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  provided file
//-------------------------------------------------

void device_nvram_interface::nvram_read(emu_file &file)
{
	if (memarray().base() != nullptr)
		file.read(memarray().base(), memarray().bytes());
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  provided file
//-------------------------------------------------

void device_nvram_interface::nvram_write(emu_file &file)
{
	if (memarray().base() != nullptr)
		file.write(memarray().base(), memarray().bytes());
}


//**************************************************************************
//  OPTIONAL NVRAM INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_optional_nvram_interface - constructor
//-------------------------------------------------

device_optional_nvram_interface::device_optional_nvram_interface(const machine_config &mconfig, device_t &device, bool has_battery_by_default)
	: device_nvram_interface(mconfig, device),
		m_has_battery(has_battery_by_default)
{
}


//-------------------------------------------------
//  static_set_has_battery - configuration helper
//-------------------------------------------------

void device_optional_nvram_interface::static_set_has_battery(device_t &device, bool has_battery)
{
	device_optional_nvram_interface *optnvram;
	if (!device.interface(optnvram))
		throw emu_fatalerror("MCFG_DEVICE_HAS_BATTERY called on device '%s' with no optional NVRAM interface\n", device.tag());

	optnvram->m_has_battery = has_battery;
}
