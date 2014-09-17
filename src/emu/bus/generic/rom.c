/***********************************************************************************************************


 Generic ROM emulation (for carts and ROM sockets)
 
 This offers generic access to a ROM
 
 generic_rom_plain  : returns 0xff when the system reads beyond the end of the ROM
 generic_rom_linear : maps linearly the ROM in the accessed area (i.e., read offset is masked with (ROM size - 1) )

 TODO:
   - possibly support linear mapping when non-power of 2 ROMs are mapped
   - add support for 32bit ROM access
 
 ***********************************************************************************************************/


#include "emu.h"
#include "rom.h"

//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type GENERIC_ROM_PLAIN  = &device_creator<generic_rom_plain_device>;
const device_type GENERIC_ROM_LINEAR = &device_creator<generic_rom_linear_device>;


generic_rom_device::generic_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_generic_cart_interface(mconfig, *this)
{
}

generic_rom_plain_device::generic_rom_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: generic_rom_device(mconfig, GENERIC_ROM_PLAIN, "Generic ROM (plain mapping)", tag, owner, clock, "generic_rom_plain", __FILE__)
{
}

generic_rom_linear_device::generic_rom_linear_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: generic_rom_device(mconfig, GENERIC_ROM_LINEAR, "Generic ROM (linear mapping)", tag, owner, clock, "generic_rom_linear", __FILE__)
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

READ8_MEMBER(generic_rom_plain_device::read_rom)
{
	if (offset < m_rom_size)
		return m_rom[offset];
	else
		return 0xff;
}

READ16_MEMBER(generic_rom_plain_device::read16_rom)
{
	UINT16 *ROM = (UINT16 *)m_rom;
	if (offset < m_rom_size/2)
		return ROM[offset];
	else
		return 0xffff;
}


READ8_MEMBER(generic_rom_linear_device::read_rom)
{
	return m_rom[offset % m_rom_size];
}

READ16_MEMBER(generic_rom_linear_device::read16_rom)
{
	UINT16 *ROM = (UINT16 *)m_rom;
	return ROM[offset % (m_rom_size/2)];
}

