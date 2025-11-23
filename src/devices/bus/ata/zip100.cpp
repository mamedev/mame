// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton device for Iomega Zip 100MB IDE drive.

    Main IC is a 100-pin QFP system-on-a-chip bearing the manufacturer logo.
    The SoC seems to be named "Rucify", which appears both on its silkscreened
    label and in its firmware EPROM.

*******************************************************************************/

#include "emu.h"
#include "zip100.h"
#include "cpu/mcs51/i80c52.h"

DEFINE_DEVICE_TYPE(ZIP100_IDE, zip100_ide_device, "zip100_ide", "Iomega Zip 100MB IDE Drive")

zip100_ide_device::zip100_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ZIP100_IDE, tag, owner, clock)
	, device_ata_interface(mconfig, *this)
{
}

void zip100_ide_device::device_start()
{
}


u16 zip100_ide_device::read_dma()
{
	return 0;
}

u16 zip100_ide_device::read_cs0(offs_t offset, u16 mem_mask)
{
	return 0;
}

u16 zip100_ide_device::read_cs1(offs_t offset, u16 mem_mask)
{
	return 0;
}

void zip100_ide_device::write_dma(u16 data)
{
}

void zip100_ide_device::write_cs0(offs_t offset, u16 data, u16 mem_mask)
{
}

void zip100_ide_device::write_cs1(offs_t offset, u16 data, u16 mem_mask)
{
}

void zip100_ide_device::write_dmack(int state)
{
}

void zip100_ide_device::write_csel(int state)
{
}

void zip100_ide_device::write_dasp(int state)
{
}

void zip100_ide_device::write_pdiag(int state)
{
}


void zip100_ide_device::mem_map(address_map &map)
{
	map(0x0000, 0xffff).rom().region("eprom", 0);
}

void zip100_ide_device::ext_map(address_map &map)
{
	// TODO
}

void zip100_ide_device::device_add_mconfig(machine_config &config)
{
	i80c32_device &mcu(I80C32(config, "mcu", 12'000'000)); // custom SoC; clock unknown
	mcu.set_addrmap(AS_PROGRAM, &zip100_ide_device::mem_map);
	mcu.set_addrmap(AS_DATA, &zip100_ide_device::ext_map);
}

ROM_START(zip100_ide)
	ROM_REGION(0x10000, "eprom", 0)
	// "IOMEGA  ZIP 100         14.A09/04/98"
	ROM_LOAD("nm27c520m.u1", 0x00000, 0x10000, CRC(67a04459) SHA1(b8df2a733838db5116982a7295484b77b272287c)) // 20-pin address-latched device
ROM_END

const tiny_rom_entry *zip100_ide_device::device_rom_region() const
{
	return ROM_NAME(zip100_ide);
}
