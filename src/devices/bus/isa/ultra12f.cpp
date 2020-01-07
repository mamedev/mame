// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    UltraStor Ultra 12F ESDI Caching Disk Controller

    Two versions of this card are known as 12F/24 and 12F/32. The hardware
    differences between these apparently have to do with drive speed; the
    same firmware and microcode may be applicable to both.

***************************************************************************/

#include "emu.h"
#include "ultra12f.h"

DEFINE_DEVICE_TYPE(ULTRA12F, ultra12f_device, "ultra12f", "Ultra 12F ESDI Caching Disk Controller")

ultra12f_device::ultra12f_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ULTRA12F, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_hpc(*this, "hpc")
	, m_fdc(*this, "fdc")
	, m_bios(*this, "bios")
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

void ultra12f_device::device_add_mconfig(machine_config &config)
{
	HPC46003(config, m_hpc, 24_MHz_XTAL); // apparently a HPC46003V30 custom-marked as "USC010-1-30"
	m_hpc->set_addrmap(AS_PROGRAM, &ultra12f_device::hpc_map);

	DP8473(config, m_fdc, 24_MHz_XTAL); // unknown quad 52-pin floppy controller marked as "USC020-1-24"
}

ROM_START(ultra12f)
	ROM_REGION(0x4000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "v202", "UBIOS 2.02") // "Date : 04/05/90 Version 2.02"
	ROMX_LOAD("38001-009.bin", 0x0000, 0x4000, CRC(b96c4839) SHA1(4e601c4ff1deae959c0807479d25de6db4e6cf3e), ROM_BIOS(0)) // 27C128, 150ns EPROM
	ROM_SYSTEM_BIOS(1, "v300", "UBIOS 3.00") // "Date : 06/18/91 Version 3.00"
	ROMX_LOAD("38001-010.bin", 0x0000, 0x4000, CRC(aa80d74e) SHA1(b507e6ead79e836969b8b6f2fa88ca99d6b354ad), ROM_BIOS(1)) // 27C128-12

	ROM_REGION16_LE(0x8000, "firmware", 0)
	ROMX_LOAD("28001-009.bin", 0x0000, 0x8000, CRC(0b6c74a1) SHA1(1a9eb5866f2104e94295d0915fe10c4c1745665b), ROM_BIOS(0)) // 27C256, 120ns EPROM
	ROMX_LOAD("28001-012.bin", 0x0000, 0x8000, CRC(62fd2f69) SHA1(48d6e65001a262b3e99d373fa59e0672c5ec4284), ROM_BIOS(1)) // 27C256-12
ROM_END

const tiny_rom_entry *ultra12f_device::device_rom_region() const
{
	return ROM_NAME(ultra12f);
}
