// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 Emerson Arcadia 2001 cart emulation

 Golf carts have the "extra_rom" handler installed at $4000 instead of $2000

 ***********************************************************************************************************/


#include "emu.h"
#include "rom.h"


//-------------------------------------------------
//  arcadia_rom_device - constructor
//-------------------------------------------------

const device_type ARCADIA_ROM_STD = &device_creator<arcadia_rom_device>;
const device_type ARCADIA_ROM_GOLF = &device_creator<arcadia_golf_device>;


arcadia_rom_device::arcadia_rom_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_arcadia_cart_interface( mconfig, *this )
{
}

arcadia_rom_device::arcadia_rom_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, ARCADIA_ROM_STD, "Emerson Arcadia Standard Carts", tag, owner, clock, "arcadia_rom", __FILE__),
						device_arcadia_cart_interface( mconfig, *this )
{
}

arcadia_golf_device::arcadia_golf_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: arcadia_rom_device(mconfig, ARCADIA_ROM_GOLF, "Emerson Arcadia Golf Cart", tag, owner, clock, "arcadia_golf", __FILE__)
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

READ8_MEMBER(arcadia_rom_device::read_rom)
{
	if (offset < m_rom_size)
		return m_rom[offset];
	else
		return 0xff;
}


READ8_MEMBER(arcadia_rom_device::extra_rom)
{
	if (offset + 0x1000 < m_rom_size)
		return m_rom[offset + 0x1000];
	else
		return 0xff;
}
