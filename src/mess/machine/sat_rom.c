/***********************************************************************************************************

 Saturn ROM cart emulation

 ***********************************************************************************************************/


#include "emu.h"
#include "machine/sat_rom.h"


//-------------------------------------------------
//  saturn_rom_device - constructor
//-------------------------------------------------

const device_type SATURN_ROM = &device_creator<saturn_rom_device>;


saturn_rom_device::saturn_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, type, name, tag, owner, clock),
						device_sat_cart_interface( mconfig, *this )
{
}

saturn_rom_device::saturn_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, SATURN_ROM, "Saturn ROM Carts", tag, owner, clock),
						device_sat_cart_interface( mconfig, *this )
{
	m_cart_type = 0xff; // actually not clear if ROM carts have a type ID like DRAM/BRAM carts
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void saturn_rom_device::device_start()
{
}

void saturn_rom_device::device_reset()
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

READ32_MEMBER(saturn_rom_device::read_rom)
{
	return m_rom[offset & (m_rom_size - 1)];
}
