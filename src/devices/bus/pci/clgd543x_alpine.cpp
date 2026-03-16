// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Cirrus Logic CL-GD5430/-34/-36/-40 "Alpine"

**************************************************************************************************/

#include "emu.h"
#include "clgd543x_alpine.h"

#include "screen.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

DEFINE_DEVICE_TYPE(GD5434_PCI, cirrus_gd5434_pci_device, "clgd5434_pci", "Cirrus Logic GD5434 card")

cirrus_gd5434_pci_device::cirrus_gd5434_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, GD5434_PCI, tag, owner, clock)
	, m_vga(*this, "vga")
	, m_bios(*this, "bios")
{
	// device IDs:
	// (*) denotes stuff not from manual but from pciids
	// - 00a0 for '30
	// * 00a2 for '32
	// * 00a4 for '34-4
	// - 00a8 for '34-[8]
	// - 00ac for '36
	// - 00a0 for '40 (for Alpine version?)
	// * 00b0 for '40 (non-Alpine?)
	set_ids(0x101300a8, 0x00, 0x030000, 0x00000000);
}

ROM_START( gd5434 )
	//  PCI card CL543XPCI Ver 4.0 - ROM: CL-GD543x PCI VGA BIOS Version 1.10B - RAM: 1MB, 2MB or 4MB - Chip: CL-GD5430-0C-C - OSC: 14.318 MHz - Connector: DB15
	// VESA feature connector - Jumpers: IRQ9 enable/disable, Green control
	ROM_REGION32_LE( 0x8000, "bios", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("122")
    ROM_SYSTEM_BIOS(0, "110b", "PCI VGA BIOS Version 1.10B")
    ROMX_LOAD("cl543x_pci_rev4.bin", 0x00000, 0x8000, CRC(553171a3) SHA1(b72edc318710c46c9cb280e9b1b3c9f8a34844f2), ROM_BIOS(0) )
    ROM_IGNORE( 0x8000 )
    ROM_SYSTEM_BIOS(1, "100e", "PCI VGA BIOS Version 1.00e")
    ROMX_LOAD("cirrus5434.bin", 0x00000, 0x8000, CRC(bc271648) SHA1(08afd4468c4c1a1630a200de1007b1671f109b3c), ROM_BIOS(1) )
    ROM_SYSTEM_BIOS(2, "122", "PCI VGA BIOS Version 1.22")
    ROMX_LOAD("gd5434pci.bin", 0x00000, 0x8000, CRC(fa76dabf) SHA1(0310ef02df941e7d35e1d832400e2e4dd07d6309), ROM_BIOS(2) )
    ROM_SYSTEM_BIOS(3, "110b_jap", "PCI VGA BIOS Version 1.10B Japan chip")
    ROMX_LOAD("japan.bin", 0x00000, 0x8000, CRC(46fe9efa) SHA1(58712b00faf102509c4129c0babeb98df2b6e042), ROM_BIOS(3) )
//    ROM_SYSTEM_BIOS(4, "124", "CL-GD543x PCI VGA BIOS Version 1.24")
//    ROMX_LOAD("1.24.bin", 0x00000,0x8000, CRC(174fa427) SHA1(3e490f3cc3af33dbfac6ff3ddac5e90bdc895646), ROM_BIOS(4) )
ROM_END

const tiny_rom_entry *cirrus_gd5434_pci_device::device_rom_region() const
{
	return ROM_NAME(gd5434);
}

void cirrus_gd5434_pci_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update(m_vga, FUNC(cirrus_gd5430_vga_device::screen_update));

	// TODO: '34
	CIRRUS_GD5430_VGA(config, m_vga, 0);
	m_vga->set_screen("screen");
	// 1MB or 2MB, max 4MB
	m_vga->set_vram_size(2*1024*1024);
}

void cirrus_gd5434_pci_device::device_start()
{
	pci_device::device_start();

	// TODO: can be M_PREF in '30/'40
	add_map( 4*1024*1024, M_MEM, FUNC(cirrus_gd5434_pci_device::vram_aperture_map));
	// TODO: BAR1 available on '30/'40 with pull-down option in MD51
//	add_map( 512, M_MEM, FUNC(cirrus_gd5434_pci_device::mmio_map));

	add_rom((u8 *)m_bios->base(), 0x8000);
	expansion_rom_base = 0xc0000;

	// INTA#
	// TODO: MD62 pull-down option
	intr_pin = 1;
}

void cirrus_gd5434_pci_device::device_reset()
{
	pci_device::device_reset();

	command = 0x0000;
	command_mask = 0x0023;
	// fast DEVSEL#
	status = 0x0000;

	remap_cb();
}

void cirrus_gd5434_pci_device::config_map(address_map &map)
{
	pci_device::config_map(map);
}

void cirrus_gd5434_pci_device::vram_aperture_map(address_map &map)
{
	map(0x000000, 0x3fffff).rw(m_vga, FUNC(cirrus_gd5430_vga_device::mem_linear_r), FUNC(cirrus_gd5430_vga_device::mem_linear_w));
}

// TODO: this should really be a subclass of VGA
void cirrus_gd5434_pci_device::legacy_memory_map(address_map &map)
{
	map(0xa0000, 0xbffff).rw(FUNC(cirrus_gd5434_pci_device::vram_r), FUNC(cirrus_gd5434_pci_device::vram_w));
}

void cirrus_gd5434_pci_device::legacy_io_map(address_map &map)
{
	map(0x03b0, 0x03df).m(m_vga, FUNC(cirrus_gd5430_vga_device::io_map));
}

uint8_t cirrus_gd5434_pci_device::vram_r(offs_t offset)
{
	return downcast<cirrus_gd5430_vga_device *>(m_vga.target())->mem_r(offset);
}

void cirrus_gd5434_pci_device::vram_w(offs_t offset, uint8_t data)
{
	downcast<cirrus_gd5430_vga_device *>(m_vga.target())->mem_w(offset, data);
}

void cirrus_gd5434_pci_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	if (BIT(command, 1))
		memory_space->install_readwrite_handler(0xa0000, 0xbffff, read8sm_delegate(*this, FUNC(cirrus_gd5434_pci_device::vram_r)), write8sm_delegate(*this, FUNC(cirrus_gd5434_pci_device::vram_w)));

	if (BIT(command, 0))
		io_space->install_device(0x0000, 0xffff, *this, &cirrus_gd5434_pci_device::legacy_io_map);
}

/*
    Cirrus Logic CL-GD54M30 - PCI
ROM_START( clgd54m30 )
    ROM_REGION(0x10000, "clgd54m30", 0)
    // Chip: CL-GD54M30-I-QC-A, ROM: 32KB, RAM: 1MB, 2MB, Connector: DB15, VESA veature connector
    ROM_SYSTEM_BIOS(0, "version_1.00d", "VGA BIOS Version 1.00d")
    ROMX_LOAD("korea.bin", 0x00000, 0x10000, CRC(0870507b) SHA1(b3873cb8834767250fcb7043918a35b1b895b0b1), ROM_BIOS(0) )
    ROM_SYSTEM_BIOS(1, "stlab_ver_1.00d", "STlab Version 1.00d")
    ROMX_LOAD("gd54m30stlab.bin", 0x08000, 0x8000, CRC(72116f4e) SHA1(4618640aa0f07586ebab02260fc58aa3035114fb), ROM_BIOS(1) )
    ROM_SYSTEM_BIOS(2, "clgd54m30_ver1.00d", "CL-GD54M30 Ver.1.00d")
    ROMX("gd54m30.bin", 0x08000, 0x000, CRC(45a1d9d0) SHA1(0e8a88ff354699bfb75a1de807f2187ff9910c67), ROM_BIOS(2) )
ROM_END

*/


/*
    Cirrus Logic VL-GD5436 - PCI cards

//Chip: CL-GD5436-I-QC-A - ROM: CL-GD5436 VL/PCI VGA BIOS Version 1.00b - RAM: 1MB, up to 4MB - Connector: DB15 - VESA connector - OSC: 14.31818MHz
// Jumpers: JG1 Green Control, IRQ9 enable/disable
ROM_START( clgd5436 )
    ROM_REGION(0x08000, "clgd5436", 0)
    ROM_LOAD("5436.vbi", 0x00000, 0x08000, CRC(7737d03e) SHA1(e4d0b4e7262887dc5a6473ea8909fdc13a6a02c1) )
ROM_END

*/

/*
    Cirrus Logic CL-GD5440 - PCI cards

// Chip: Cirrus Logic CL-GD5440-J-QC-B - ROM: CL-GD5440 VGA BIOS Version 1.07 - RAM: 1MB, max. 2MB - OSC:14.31818MHz - Connector: DB15 - VESA connector
ROM_START( clgd5440 )
    ROM_REGION(0x10000, "clgd5440", 0)
    ROM_LOAD("bios.bin", 0x00000, 0x10000, CRC(f0d3d0b4) SHA1(620b0727a20b127f5f32576ec54fbc6f9f437ad3) )
ROM_END
*/

