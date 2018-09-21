// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

  This is simply a TMS320C25 with internal ROM and Namco code '67'

  used by Namco System 21 for both the master and slave DSPs (configuration
  is decided based on a port read)

******************************************************************************/


#include "emu.h"
#include "namco_c67.h"

DEFINE_DEVICE_TYPE(NAMCO_C67, namco_c67_device, "namcoc67", "Namco C67 (TMS320C25)")

namco_c67_device::namco_c67_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	tms32025_device(mconfig, NAMCO_C67, tag, owner, clock)
{
	set_mp_mc(false);
}

ROM_START( c67 )
	ROM_REGION16_BE( 0x2000, "internal", 0 )
	ROM_LOAD( "c67.bin", 0, 0x2000, CRC(6bd8988e) SHA1(c9ec18d5f88d53976b94444eedc64d5568155958) )
ROM_END

const tiny_rom_entry *namco_c67_device::device_rom_region() const
{
	return ROM_NAME(c67);
}
