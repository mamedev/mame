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

DEFINE_DEVICE_TYPE(APF_ROM_STD,      apf_rom_device,      "apf_rom",      "APF Standard Carts")
DEFINE_DEVICE_TYPE(APF_ROM_BASIC,    apf_basic_device,    "apf_basic",    "APF BASIC Carts")
DEFINE_DEVICE_TYPE(APF_ROM_SPACEDST, apf_spacedst_device, "apf_spacedst", "APF Space Destroyer Cart")


apf_rom_device::apf_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock), device_apf_cart_interface(mconfig, *this)
{
}

apf_rom_device::apf_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: apf_rom_device(mconfig, APF_ROM_STD, tag, owner, clock)
{
}

apf_basic_device::apf_basic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: apf_rom_device(mconfig, APF_ROM_BASIC, tag, owner, clock)
{
}

apf_spacedst_device::apf_spacedst_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: apf_rom_device(mconfig, APF_ROM_SPACEDST, tag, owner, clock)
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

READ8_MEMBER(apf_rom_device::read_rom)
{
	if (offset < m_rom_size)
		return m_rom[offset];
	else
		return 0xff;
}


READ8_MEMBER(apf_basic_device::extra_rom)
{
	if (offset < (m_rom_size - 0x2000))
		return m_rom[offset + 0x2000];
	else
		return 0xff;
}


READ8_MEMBER(apf_spacedst_device::read_ram)
{
	return m_ram[offset];
}

WRITE8_MEMBER(apf_spacedst_device::write_ram)
{
	m_ram[offset] = data;
}
