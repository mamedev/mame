// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton device for Plextor PleXCombo PX-320A IDE DVD/CD-RW drive.

*******************************************************************************/

#include "emu.h"
#include "px320a.h"
#include "cpu/fr/fr.h"

DEFINE_DEVICE_TYPE(PX320A, px320a_device, "px320a", "PleXCombo PX-320A CD-RW/DVD-ROM Drive")

px320a_device::px320a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, PX320A, tag, owner, clock)
	, device_ata_interface(mconfig, *this)
	, m_frcpu(*this, "frcpu")
{
}

void px320a_device::device_start()
{
}


u16 px320a_device::read_dma()
{
	return 0;
}

u16 px320a_device::read_cs0(offs_t offset, u16 mem_mask)
{
	return 0;
}

u16 px320a_device::read_cs1(offs_t offset, u16 mem_mask)
{
	return 0;
}

void px320a_device::write_dma(u16 data)
{
}

void px320a_device::write_cs0(offs_t offset, u16 data, u16 mem_mask)
{
}

void px320a_device::write_cs1(offs_t offset, u16 data, u16 mem_mask)
{
}

WRITE_LINE_MEMBER(px320a_device::write_dmack)
{
}

WRITE_LINE_MEMBER(px320a_device::write_csel)
{
}

WRITE_LINE_MEMBER(px320a_device::write_dasp)
{
}

WRITE_LINE_MEMBER(px320a_device::write_pdiag)
{
}


void px320a_device::frcpu_map(address_map &map)
{
	map(0x00080000, 0x000fffff).rom().region("firmware", 0);
}

void px320a_device::device_add_mconfig(machine_config &config)
{
	MB91101A(config, m_frcpu, 8000000); // FR type guessed; clock unknown
	m_frcpu->set_addrmap(AS_PROGRAM, &px320a_device::frcpu_map);
}

ROM_START(px320a)
	ROM_REGION32_BE(0x80000, "firmware", 0)
	ROM_SYSTEM_BIOS(0, "v106", "Firmware v1.06")
	ROM_LOAD("sapp106.bin", 0x00000, 0x80000, CRC(2ba54c7a) SHA1(032cc5009fe84db902096ea2bb9dc2221601ab20))
	ROM_SYSTEM_BIOS(1, "v105", "Firmware v1.05")
	ROM_LOAD("sapp105.bin", 0x00000, 0x80000, CRC(3755a053) SHA1(fb966d506edb6e0a97ad64de3898ce9e75e22d7d))
	ROM_SYSTEM_BIOS(2, "v104", "Firmware v1.04a")
	ROM_LOAD("sapp104.bin", 0x00000, 0x80000, CRC(7197425f) SHA1(0b02508d668439a78ea8e3d97eddf5a3de6e2ba5))
	ROM_SYSTEM_BIOS(3, "v103", "Firmware v1.03")
	ROM_LOAD("sap1032.bin", 0x00000, 0x80000, CRC(9ff1c729) SHA1(6ed44bf8decaac2ee9127bdd66e0ff1fd52969c3))
	ROM_SYSTEM_BIOS(4, "v102", "Firmware v1.02")
	ROM_LOAD("sapp102.bin", 0x00000, 0x80000, CRC(9f96bea4) SHA1(5b70f32e83d8c19d2895d38d0f7e15f2eee326fd))
ROM_END

const tiny_rom_entry *px320a_device::device_rom_region() const
{
	return ROM_NAME(px320a);
}
