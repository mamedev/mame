// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    UltraStor Ultra 12F ESDI Caching Disk Controller

    The original version of this card supported drive speeds up to 22 MHz.
    The 12F/24 model supports a 24 MHz speed as well, and the 12F/32 model
    presumably supports up to 32 MHz. There do not appear to be any major
    hardware differences between these models.

***************************************************************************/

#include "emu.h"
#include "ultra12f.h"

DEFINE_DEVICE_TYPE(ULTRA12F, ultra12f_device, "ultra12f", "Ultra 12F ESDI Caching Disk Controller")
DEFINE_DEVICE_TYPE(ULTRA12F32, ultra12f32_device, "ultra12f32", "Ultra 12F/32 ESDI Caching Disk Controller")

ultra12f_device::ultra12f_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_hpc(*this, "hpc")
	, m_fdc(*this, "fdc")
	, m_bios(*this, "bios")
{
}

ultra12f_device::ultra12f_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ultra12f_device(mconfig, ULTRA12F, tag, owner, clock)
{
}

ultra12f32_device::ultra12f32_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ultra12f_device(mconfig, ULTRA12F32, tag, owner, clock)
{
}

void ultra12f_device::device_start()
{
	// Firmware EPROM has a couple of lines scrambled
	u8 *firmware = memregion("firmware")->base();
	for (offs_t b = 0; b < 0x8000; b += 0x100)
	{
		std::swap_ranges(&firmware[b | 0x10], &firmware[b | 0x20], &firmware[b | 0x80]);
		std::swap_ranges(&firmware[b | 0x30], &firmware[b | 0x40], &firmware[b | 0xa0]);
		std::swap_ranges(&firmware[b | 0x50], &firmware[b | 0x60], &firmware[b | 0xc0]);
		std::swap_ranges(&firmware[b | 0x70], &firmware[b | 0x80], &firmware[b | 0xe0]);
	}
}

void ultra12f_device::hpc_map(address_map &map)
{
	map(0x8000, 0xffff).rom().region("firmware", 0);
}

static INPUT_PORTS_START(ultra12f)
	PORT_START("BIOSADDR")
	PORT_DIPNAME(0x7, 0x6, "BIOS Address") PORT_DIPLOCATION("JP10:3,2,1") // numbered as 5-6, 3-4 and 1-2
	PORT_DIPSETTING(0x7, "Disable BIOS")
	PORT_DIPSETTING(0x6, "C800:0") // Primary port
	PORT_DIPSETTING(0x5, "CC00:0") // Secondary port
	PORT_DIPSETTING(0x4, "D000:0") // Primary port
	PORT_DIPSETTING(0x3, "D400:0") // Secondary port
	PORT_DIPSETTING(0x2, "D800:0") // Primary port
	PORT_DIPSETTING(0x1, "DC00:0") // Secondary port
	//PORT_DIPSETTING(0x0, "Disable BIOS")

	PORT_START("FDCCONF")
	PORT_DIPNAME(0x1, 0x0, "Floppy Controller") PORT_DIPLOCATION("JP17:1")
	PORT_DIPSETTING(0x1, "Disable")
	PORT_DIPSETTING(0x0, "Enable")
	// TODO: other jumpers on JP11

	PORT_START("HDCPORT")
	PORT_DIPNAME(0x1, 0x1, "Hard Disk I/O Port Address") PORT_DIPLOCATION("JP12:1")
	PORT_DIPSETTING(0x0, "170-177") // Secondary
	PORT_DIPSETTING(0x1, "1F0-1F7") // Primary

	PORT_START("IRQSEL")
	PORT_DIPNAME(0x3, 0x1, "IRQ Select") PORT_DIPLOCATION("JP20:1,2")
	PORT_DIPSETTING(0x1, "IRQ14") // 2-3 jumpered
	PORT_DIPSETTING(0x2, "IRQ15") // 1-2 jumpered
INPUT_PORTS_END

ioport_constructor ultra12f_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ultra12f);
}

void ultra12f_device::device_add_mconfig(machine_config &config)
{
	HPC46003(config, m_hpc, 24_MHz_XTAL); // apparently a HPC46003V30 custom-marked as "USC010-1-30"
	m_hpc->set_addrmap(AS_PROGRAM, &ultra12f_device::hpc_map);

	//CL_SH260(config, "diskc", 24_MHz_XTAL / 2); // 84-pin PLCC IC custom-marked as "USC030-1-15"

	DP8473(config, m_fdc, 24_MHz_XTAL); // unknown 52-pin PLCC floppy controller marked as "USC020-1-24"
}

ROM_START(ultra12f)
	ROM_REGION(0x4000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "v202", "UBIOS 2.02") // "Date : 04/05/90 Version 2.02"
	ROMX_LOAD("38001-009_ultrastor_corp_89_otp.u15", 0x0000, 0x4000, CRC(b96c4839) SHA1(4e601c4ff1deae959c0807479d25de6db4e6cf3e), ROM_BIOS(0)) // 27C128, 150ns EPROM
	ROM_SYSTEM_BIOS(1, "v300", "UBIOS 3.00") // "Date : 06/18/91 Version 3.00"
	ROMX_LOAD("38001-010.bin", 0x0000, 0x4000, CRC(aa80d74e) SHA1(b507e6ead79e836969b8b6f2fa88ca99d6b354ad), ROM_BIOS(1)) // 27C128-12

	ROM_REGION16_LE(0x8000, "firmware", 0)
	ROMX_LOAD("28001-009_uc_89.u1", 0x0000, 0x8000, CRC(0b6c74a1) SHA1(1a9eb5866f2104e94295d0915fe10c4c1745665b), ROM_BIOS(0)) // 27C256, 120ns EPROM
	ROMX_LOAD("28001-012.bin", 0x0000, 0x8000, CRC(62fd2f69) SHA1(48d6e65001a262b3e99d373fa59e0672c5ec4284), ROM_BIOS(1)) // 27C256-12
ROM_END

ROM_START(ultra12f32)
	ROM_REGION(0x4000, "bios", 0) // "UBIOS 1.15"
	ROM_LOAD("38001-006_usc_89_4cf3.u15", 0x0000, 0x4000, CRC(f09faeca) SHA1(b9e4edd5b16089c07d87b0c07a927c2d741b1f5e))

	ROM_REGION16_LE(0x8000, "firmware", 0)
	ROM_LOAD("28001-005_usc_89_beeb.u1", 0x0000, 0x8000, CRC(e28640ac) SHA1(110e724b4116fc2cbd050d9f2e541d2366fd9f4c))
ROM_END

const tiny_rom_entry *ultra12f_device::device_rom_region() const
{
	return ROM_NAME(ultra12f);
}

const tiny_rom_entry *ultra12f32_device::device_rom_region() const
{
	return ROM_NAME(ultra12f32);
}
