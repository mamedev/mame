// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 APF Imagination / M-1000 cart emulation


 ***********************************************************************************************************/


#include "emu.h"
#include "rom.h"


//-------------------------------------------------
//  apf_rom_device - constructor
//-------------------------------------------------

const device_type APF_ROM_STD = &device_creator<apf_rom_device>;
const device_type APF_ROM_BASIC = &device_creator<apf_basic_device>;
const device_type APF_ROM_SPACEDST = &device_creator<apf_spacedst_device>;


apf_rom_device::apf_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_apf_cart_interface( mconfig, *this )
{
}

apf_rom_device::apf_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
					: device_t(mconfig, APF_ROM_STD, "APF Standard Carts", tag, owner, clock, "apf_rom", __FILE__),
						device_apf_cart_interface( mconfig, *this )
{
}

apf_basic_device::apf_basic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
					: apf_rom_device(mconfig, APF_ROM_BASIC, "APF BASIC Carts", tag, owner, clock, "apf_basic", __FILE__)
{
}

apf_spacedst_device::apf_spacedst_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
					: apf_rom_device(mconfig, APF_ROM_SPACEDST, "APF Space Destroyer Cart", tag, owner, clock, "apf_spacedst", __FILE__)
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

uint8_t apf_rom_device::read_rom(address_space &space, offs_t offset, uint8_t mem_mask)
{
	if (offset < m_rom_size)
		return m_rom[offset];
	else
		return 0xff;
}


uint8_t apf_basic_device::extra_rom(address_space &space, offs_t offset, uint8_t mem_mask)
{
	if (offset < (m_rom_size - 0x2000))
		return m_rom[offset + 0x2000];
	else
		return 0xff;
}


uint8_t apf_spacedst_device::read_ram(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return m_ram[offset];
}

void apf_spacedst_device::write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_ram[offset] = data;
}
