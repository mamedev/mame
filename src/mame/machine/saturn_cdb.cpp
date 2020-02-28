// license:BSD-3-Clause
// copyright-holders:David Haywood

/* Notes

YGR019B - Hitachi YGR019B CD-Subsystem LSI. Earlier revision is YGR019A. Later revision combines this IC and the SH1 together
        into one IC (YGR022 315-5962). The SH1 and the YGR019B make up the 'CD Block' CD Authentication and CD I/O data controller.
        Another of it's functions is to prevent copied CDs from being played

*/

#include "emu.h"
#include "machine/saturn_cdb.h"

DEFINE_DEVICE_TYPE(SATURN_CDB, saturn_cdb_device, "satcdb", "Saturn CDB (CD Block)")

saturn_cdb_device::saturn_cdb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SATURN_CDB, tag, owner, clock)
{
}

void saturn_cdb_device::device_start()
{
}

void saturn_cdb_device::saturn_cdb_map(address_map &map)
{
	map(0x00000000, 0x0000ffff).rom();
}

void saturn_cdb_device::device_add_mconfig(machine_config &config)
{
	sh1_device &cdbcpu(SH1(config, "cdbcpu", DERIVED_CLOCK(1, 1)));
	cdbcpu.set_addrmap(AS_PROGRAM, &saturn_cdb_device::saturn_cdb_map);
	cdbcpu.set_disable(); // we're not actually using the CD Block ROM for now
}

ROM_START( satcdb )
	ROM_REGION( 0x10000, "cdbcpu", 0 )
	ROM_DEFAULT_BIOS("cdb106")
	ROM_SYSTEM_BIOS( 0, "cdb106", "Saturn CD Block 1.06" )
	ROMX_LOAD( "cdb106.bin", 0x00000, 0x10000, CRC(3681d3b0) SHA1(b3c20fbe57cd2eb595e9edac86817e5948dccae4), ROM_BIOS(0) ) // for YGR019B?
	ROM_SYSTEM_BIOS( 1, "cdb105", "Saturn CD Block 1.05" )
	ROMX_LOAD( "cdb105.bin", 0x00000, 0x10000, CRC(2a2ced5c) SHA1(eb8393058f324e922c11b43709b64fc6ca94ab86), ROM_BIOS(1) ) // for YGR019A?
	ROM_SYSTEM_BIOS( 2, "ygr022", "Saturn CD Block (YGR022 315-5962)" )
	ROMX_LOAD( "ygr022.bin", 0x00000, 0x10000, CRC(1c8b9f38) SHA1(f4f6c2aac68c352814d396ae41f81f54ad228e68), ROM_BIOS(2) ) // combined package?
ROM_END

const tiny_rom_entry *saturn_cdb_device::device_rom_region() const
{
	return ROM_NAME(satcdb);
}
