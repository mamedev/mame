// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 Mattel Intellivision cart emulation


 ***********************************************************************************************************/


#include "emu.h"
#include "rom.h"


//-------------------------------------------------
//  intv_rom_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(INTV_ROM_STD,   intv_rom_device,   "intv_rom",   "Intellivision Standard Carts")
DEFINE_DEVICE_TYPE(INTV_ROM_RAM,   intv_ram_device,   "intv_ram",   "Intellivision Carts w/RAM")
DEFINE_DEVICE_TYPE(INTV_ROM_GFACT, intv_gfact_device, "intv_gfact", "Intellivision Game Factory Cart")
DEFINE_DEVICE_TYPE(INTV_ROM_WSMLB, intv_wsmlb_device, "intv_wsmlb", "Intellivision World Series Baseball Cart")


intv_rom_device::intv_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_intv_cart_interface(mconfig, *this)
{
}

intv_rom_device::intv_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: intv_rom_device(mconfig, INTV_ROM_STD, tag, owner, clock)
{
}

intv_ram_device::intv_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: intv_rom_device(mconfig, INTV_ROM_RAM, tag, owner, clock)
{
}

intv_gfact_device::intv_gfact_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: intv_rom_device(mconfig, INTV_ROM_GFACT, tag, owner, clock)
{
}

intv_wsmlb_device::intv_wsmlb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: intv_rom_device(mconfig, INTV_ROM_WSMLB, tag, owner, clock)
{
}
