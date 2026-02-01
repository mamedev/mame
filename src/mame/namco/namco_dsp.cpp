// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

  This is simply a TMS320C25 with internal ROM and Namco code '67', '71'

  C67 was used by Namco System 21 for both the master and slave DSPs
  (configuration is decided based on a port read)

  C71 was used by Namco System (Super) 22 for both master and slave DSPs

******************************************************************************/


#include "emu.h"
#include "namco_dsp.h"

DEFINE_DEVICE_TYPE(NAMCO_C67, namco_c67_device, "namcoc67", "Namco C67 (TMS320C25)")
DEFINE_DEVICE_TYPE(NAMCO_C71, namco_c71_device, "namcoc71", "Namco C71 (TMS320C25)")


namco_c67_device::namco_c67_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	tms320c25_device(mconfig, NAMCO_C67, tag, owner, clock)
{
	set_mp_mc(false);
}

namco_c71_device::namco_c71_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	tms320c25_device(mconfig, NAMCO_C71, tag, owner, clock)
{
	set_mp_mc(false);
}

ROM_START( c67 )
	ROM_REGION16_BE( 0x2000, "internal", 0 )
	ROM_LOAD( "c67.bin", 0, 0x2000, CRC(6bd8988e) SHA1(c9ec18d5f88d53976b94444eedc64d5568155958) )
ROM_END

ROM_START( c71 )
	ROM_REGION16_BE( 0x2000, "internal", 0 )
	ROM_LOAD( "c71.bin", 0,0x2000, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )
ROM_END

const tiny_rom_entry *namco_c67_device::device_rom_region() const
{
	return ROM_NAME(c67);
}

const tiny_rom_entry *namco_c71_device::device_rom_region() const
{
	return ROM_NAME(c71);
}
