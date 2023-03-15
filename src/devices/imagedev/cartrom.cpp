// license:BSD-3-Clause
// copyright-holders:AJR
/*********************************************************************

    cartrom.cpp

    Base classes for ROM and cartridge ROM image devices.

*********************************************************************/

#include "emu.h"
#include "cartrom.h"

#include "softlist_dev.h"


device_rom_image_interface::device_rom_image_interface(const machine_config &mconfig, device_t &device)
	: device_image_interface(mconfig, device)
{
}

device_cartrom_image_interface::device_cartrom_image_interface(const machine_config &mconfig, device_t &device)
	: device_rom_image_interface(mconfig, device)
{
}

const software_list_loader &device_rom_image_interface::get_software_list_loader() const
{
	return rom_software_list_loader::instance();
}
