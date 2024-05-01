// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton device for Conner Peripherals CP-2024 2.5″ IDE hard disk drive.

    Codename: "Kato"
    ICs: Linear Technology LT1072 (DIP8)
         Cirrus Logic SH110-00PC (PLCC28)
         Cirrus Logic CL-SH265-25QC-C (QFP100)
         Microchip 27C256-20/L (PLCC32)
         Motorola SC80566FN (PLCC52)
         Texas Instruments LS01 (SOP14)
         S-MOS 61100-003 (QFP64)
         Cirrus Logic 61094-4 (PLCC44)
         LSI Logic(?) 61089-003 (PLCC52)

*******************************************************************************/

#include "emu.h"
#include "cp2024.h"
#include "cpu/mc68hc11/mc68hc11.h"

DEFINE_DEVICE_TYPE(CP2024, cp2024_device, "cp2024", "Conner Peripherals CP-2024 hard disk")

cp2024_device::cp2024_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, CP2024, tag, owner, clock)
	, device_ata_interface(mconfig, *this)
{
}

void cp2024_device::device_start()
{
}


u16 cp2024_device::read_dma()
{
	return 0;
}

u16 cp2024_device::read_cs0(offs_t offset, u16 mem_mask)
{
	return 0;
}

u16 cp2024_device::read_cs1(offs_t offset, u16 mem_mask)
{
	return 0;
}

void cp2024_device::write_dma(u16 data)
{
}

void cp2024_device::write_cs0(offs_t offset, u16 data, u16 mem_mask)
{
}

void cp2024_device::write_cs1(offs_t offset, u16 data, u16 mem_mask)
{
}

void cp2024_device::write_dmack(int state)
{
}

void cp2024_device::write_csel(int state)
{
}

void cp2024_device::write_dasp(int state)
{
}

void cp2024_device::write_pdiag(int state)
{
}


void cp2024_device::mcu_map(address_map &map)
{
	//map(0x0000, 0x00ff).m("sh265", FUNC(cl_sh265_device::map));
	map(0x8000, 0xffff).rom().region("eprom", 0);
}

void cp2024_device::device_add_mconfig(machine_config &config)
{
	mc68hc11a1_device &mcu(MC68HC11A1(config, "mcu", 10'000'000)); // clock unknown
	mcu.set_addrmap(AS_PROGRAM, &cp2024_device::mcu_map);
}

ROM_START(cp2024)
	ROM_REGION(0x8000, "eprom", 0)
	ROM_LOAD("27c256.bin", 0x0000, 0x8000, CRC(574b5904) SHA1(79bccc33835ba01fc4eb33fb4d412a691e7790c2))
ROM_END

const tiny_rom_entry *cp2024_device::device_rom_region() const
{
	return ROM_NAME(cp2024);
}
