// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    BBC Micro Sideways RAM (Battery Backup) emulation

***************************************************************************/

#include "emu.h"
#include "nvram.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_NVRAM, bbc_nvram_device, "bbc_nvram", "BBC Micro Sideways RAM (Battery Backup)")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_nvram_device - constructor
//-------------------------------------------------

bbc_nvram_device::bbc_nvram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_NVRAM, tag, owner, clock)
	, device_bbc_rom_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_nvram_device::device_start()
{
	nvram_alloc(0x4000);
}

//-------------------------------------------------
//  read
//-------------------------------------------------

uint8_t bbc_nvram_device::read(offs_t offset)
{
	return get_nvram_base()[offset & (get_nvram_size() - 1)];
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void bbc_nvram_device::write(offs_t offset, uint8_t data)
{
	get_nvram_base()[offset & (get_nvram_size() - 1)] = data;
}
