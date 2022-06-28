// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton device for Philips CDD2000/HP4020 CD-R.

*******************************************************************************/

#include "emu.h"
#include "bus/nscsi/cdd2000.h"
#include "cpu/mc68hc11/mc68hc11.h"
#include "machine/ncr5390.h"

DEFINE_DEVICE_TYPE(CDD2000, cdd2000_device, "cdd2000", "Philips CDD2000 CD-R")

cdd2000_device::cdd2000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, CDD2000, tag, owner, clock)
	, nscsi_slot_card_interface(mconfig, *this, "scsic")
	, m_cdcpu(*this, "cdcpu")
{
}

void cdd2000_device::device_start()
{
}

void cdd2000_device::mem_map(address_map &map)
{
	map(0x0400, 0x040f).m("scsic", FUNC(ncr53cf94_device::map));
	// 0x0800, 0x080f is another device
	map(0x1000, 0xffff).rom().region("flash", 0x31000); // TODO: banking
	map(0x2000, 0x3fff).ram();
}

void cdd2000_device::device_add_mconfig(machine_config &config)
{
	mc68hc11f1_device &cdcpu(MC68HC11F1(config, m_cdcpu, 8'000'000)); // type and clock guessed
	cdcpu.set_addrmap(AS_PROGRAM, &cdd2000_device::mem_map);
	cdcpu.set_default_config(0x0b);

	NCR53CF94(config, "scsic", 25'000'000); // type and clock guessed
}

ROM_START(cdd2000)
	ROM_REGION(0x40000, "flash", 0)
	ROM_SYSTEM_BIOS(0, "v126", "Firmware v1.26")
	ROMX_LOAD("cdd2_126.bin", 0x00000, 0x40000, CRC(8a9f0f85) SHA1(efc6c696b12af7d29fcc37c641bc5879517d6fd8), ROM_BIOS(0)) // 9F9E checksum
	ROM_SYSTEM_BIOS(1, "v125", "Firmware v1.25")
	ROMX_LOAD("cdd2_125.bin", 0x00000, 0x40000, CRC(17f1c04a) SHA1(882be4ed5daf70a686929fffcb66fa95b431bbe2), ROM_BIOS(1)) // A29B checksum
ROM_END

const tiny_rom_entry *cdd2000_device::device_rom_region() const
{
	return ROM_NAME(cdd2000);
}
