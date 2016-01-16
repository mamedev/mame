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


apf_rom_device::apf_rom_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_apf_cart_interface( mconfig, *this )
{
}

apf_rom_device::apf_rom_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, APF_ROM_STD, "APF Standard Carts", tag, owner, clock, "apf_rom", __FILE__),
						device_apf_cart_interface( mconfig, *this )
{
}

apf_basic_device::apf_basic_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: apf_rom_device(mconfig, APF_ROM_BASIC, "APF BASIC Carts", tag, owner, clock, "apf_basic", __FILE__)
{
}

apf_spacedst_device::apf_spacedst_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: apf_rom_device(mconfig, APF_ROM_SPACEDST, "APF Space Destroyer Cart", tag, owner, clock, "apf_spacedst", __FILE__)
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
