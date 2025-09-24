// license:BSD-3-Clause
// copyright-holders:

/*******************************************************************************************

    Skeleton device for 3DLabs Permedia 2 based video cards.

*******************************************************************************************/

#include "emu.h"
#include "permedia2.h"

DEFINE_DEVICE_TYPE(PERMEDIA2, permedia2_device, "permedia2", "3DLabs Permedia 2 based video card")

permedia2_device::permedia2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, PERMEDIA2, tag, owner, clock)
{
}

void permedia2_device::device_start()
{
}

void permedia2_device::device_add_mconfig(machine_config &config)
{
}

ROM_START(permedia2)
	ROM_REGION( 0x20000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "sparc_1_60", "18/May/2000 (Rev 1.60) for Solaris SPARC" )
	ROMX_LOAD( "raptor160.u10", 0x00000, 0x20000, CRC(8d6706d5) SHA1(6df6719aa8f46176d837b1b05e90f7e541416b4c), ROM_BIOS(0) ) // Tech-Source Inc. Raptor GFX
	ROM_SYSTEM_BIOS( 1, "sparc_1_11", "07/Jun/1999 (Rev 1.11) for Solaris SPARC" )
	ROMX_LOAD( "raptor111.bin", 0x00000, 0x10000, CRC(ee21d1f4) SHA1(04845a2e0938b9ecd88934c148e637e6f00e2578), ROM_BIOS(1) ) // Tech-Source Inc. Raptor GFX // Half-size ROM?
	ROM_SYSTEM_BIOS( 2, "sparc_1_10", "15/Mar/1999 (Rev 1.10) for Solaris SPARC" )
	ROMX_LOAD( "raptor110.bin", 0x00000, 0x20000, CRC(6a6f4ba4) SHA1(89df1a7bc52693e34b79587c16f2c2efb30bd3f1), ROM_BIOS(2) ) // Tech-Source Inc. Raptor GFX
	ROM_SYSTEM_BIOS( 3, "pc_99",      "25/Feb/1999 for IBM PC Compatible" )
	ROMX_LOAD( "permed2pc.bin", 0x00000, 0x20000, CRC(6711ddf8) SHA1(9f02e4d1a64c42e4a17df8428d8cc9a72e78e0d5), ROM_BIOS(3) ) // Phoenix / 3DLabs generic BIOS for IBM PC Compatible systems
ROM_END

const tiny_rom_entry *permedia2_device::device_rom_region() const
{
	return ROM_NAME(permedia2);
}
