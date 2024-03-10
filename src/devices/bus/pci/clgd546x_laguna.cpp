// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#include "emu.h"
#include "clgd546x_laguna.h"

#include "screen.h"

#define LOG_WARN      (1U << 1)
#define LOG_TODO      (1U << 2) // log unimplemented registers

#define VERBOSE (LOG_GENERAL | LOG_WARN | LOG_TODO)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)
#define LOGTODO(...)            LOGMASKED(LOG_TODO, __VA_ARGS__)

DEFINE_DEVICE_TYPE(CIRRUS_GD5465_LAGUNA3D, cirrus_gd5465_laguna3d_device, "clgd5465_laguna", "Cirrus Logic GD-5465 \"Laguna 3D\"")

cirrus_gd5465_laguna3d_device::cirrus_gd5465_laguna3d_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, CIRRUS_GD5465_LAGUNA3D, tag, owner, clock)
	, m_vga(*this, "vga")
	, m_vga_rom(*this, "vga_rom")
{
	// device ID 0x1013 Cirrus Logic
	// 0x00dx for Laguna revs
	// subvendor ID: Returns values from ROM 0x7ff8-0x7ffb
	set_ids_agp(0x101300d6, 0x00, 0x10130000);
	// TODO: class can be 0x80 (other display controller) with P18 pin.
}

/*
    Cirrus Logic CL-GD5464 Laguna3D
// DSystems Wizar3D PCI graphics card - Chip: CL-GD5464-HC-A - ROM: DSystems Wizar3D 144a.10H - RAM: 2MB, 4MB, 8MB - Connector: DB15 - VESA feature connector
ROM_START( clgd5464 )
    ROM_REGION32_LE( 0x08000, "vga_rom", ROMREGION_ERASEFF )
    ROM_LOAD("dsystems_wizard3d.vbi", 0x00000, 0x08000, CRC(df9f1570), SHA1(4e611f4039b851fd4237d450e38c9d764920a747) )
ROM_END

*/

ROM_START( gd5465 )
	// Chaintech GA-5465AS AGP graphics card - Chip: CL-GD5465 - ROM: CL-GD546x Laguna PCI VGA BIOS Version 1.62c - RAM: 4MB, 8MB - OSC: 14.3C7Y - Connector: DB15 - VESA feature connector
	ROM_REGION32_LE( 0x8000, "vga_rom", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "chaintech", "Chaintech GA-5465AS 1.62c" )
	ROMX_LOAD( "chaintech.vbi", 0x0000, 0x8000, CRC(8afa1afb) SHA1(251a953d442dc34738f80371cfbd0fd9f1097635), ROM_BIOS(0) )
ROM_END

const tiny_rom_entry *cirrus_gd5465_laguna3d_device::device_rom_region() const
{
	return ROM_NAME(gd5465);
}

void cirrus_gd5465_laguna3d_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update(m_vga, FUNC(cirrus_gd5446_vga_device::screen_update));

	// TODO: bump to GD5465
	CIRRUS_GD5446_VGA(config, m_vga, 0);
	m_vga->set_screen("screen");
	// FIXME: shared RAM
	// in 4 and 8 MB versions
	m_vga->set_vram_size(8*1024*1024);
}

void cirrus_gd5465_laguna3d_device::device_start()
{
	pci_device::device_start();

	add_map( 32*1024, M_MEM, FUNC(cirrus_gd5465_laguna3d_device::mmio_map));
	add_map( 32*1024*1024, M_MEM, FUNC(cirrus_gd5465_laguna3d_device::vram_aperture_map));

	add_rom((u8 *)m_vga_rom->base(), 0x8000);
	expansion_rom_base = 0xc0000;

	// INTA#
	intr_pin = 1;
	save_item(NAME(m_vga_legacy_enable));

	// TODO: fast back-to-back
}

void cirrus_gd5465_laguna3d_device::device_reset()
{
	pci_device::device_reset();

	// TODO: to be checked
	command = 0x0000;
	status = 0x0000;

	m_vga_legacy_enable = true;
	remap_cb();
}

void cirrus_gd5465_laguna3d_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	// TODO: AGP CAPPTR
//  map(0xf8, 0xf8) PCI VGA Shadow Register
//  map(0xfc, 0xfc) PCI Vendor Specific Control Register
}

void cirrus_gd5465_laguna3d_device::mmio_map(address_map &map)
{

}

void cirrus_gd5465_laguna3d_device::vram_aperture_map(address_map &map)
{

}

// TODO: this should really be a subclass of VGA
void cirrus_gd5465_laguna3d_device::legacy_memory_map(address_map &map)
{
	map(0xa0000, 0xbffff).rw(FUNC(cirrus_gd5465_laguna3d_device::vram_r), FUNC(cirrus_gd5465_laguna3d_device::vram_w));
}

void cirrus_gd5465_laguna3d_device::legacy_io_map(address_map &map)
{
	map(0, 0x02f).m(m_vga, FUNC(cirrus_gd5446_vga_device::io_map));
}

uint8_t cirrus_gd5465_laguna3d_device::vram_r(offs_t offset)
{
	return downcast<cirrus_gd5446_vga_device *>(m_vga.target())->mem_r(offset);
}

void cirrus_gd5465_laguna3d_device::vram_w(offs_t offset, uint8_t data)
{
	downcast<cirrus_gd5446_vga_device *>(m_vga.target())->mem_w(offset, data);
}

void cirrus_gd5465_laguna3d_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	if (m_vga_legacy_enable)
	{
		memory_space->install_readwrite_handler(0xa0000, 0xbffff, read8sm_delegate(*this, FUNC(cirrus_gd5465_laguna3d_device::vram_r)), write8sm_delegate(*this, FUNC(cirrus_gd5465_laguna3d_device::vram_w)));

		io_space->install_device(0x03b0, 0x03df, *this, &cirrus_gd5465_laguna3d_device::legacy_io_map);
		//memory_space->install_rom(0xc0000, 0xcffff, (void *)expansion_rom);
	}
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
    Cirrus Logic CL-GD543x - PCI cards

//  PCI card CL543XPCI Ver 4.0 - ROM: CL-GD543x PCI VGA BIOS Version 1.10B - RAM: 1MB, 2MB or 4MB - Chip: CL-GD5430-0C-C - OSC: 14.318 MHz - Connector: DB15
// VESA feature connector - Jumpers: IRQ9 enable/disable, Green control
ROM_START( clgd543x_pci )
    ROM_REGION(0x08000, "clgd543x_pci", 0)
    ROM_SYSTEM_BIOS(0, "pci_version_1.10b", "PCI VGA BIOS Version 1.10B")
    ROMX_LOAD("cl543x_pci_rev4.bin", 0x00000, 0x8000, CRC(553171a3) SHA1(b72edc318710c46c9cb280e9b1b3c9f8a34844f2), ROM_BIOS(0) )
    ROM_IGNORE( 0x8000 )
    ROM_SYSTEM_BIOS(1, "pci_version_1.00e", "PCI VGA BIOS Version 1.00e")
    ROMX_LOAD("cirrus5434.bin", 0x00000, 0x8000, CRC(bc271648) SHA1(08afd4468c4c1a1630a200de1007b1671f109b3c), ROM_BIOS(1) )
    ROM_SYSTEM_BIOS(2, "pci_version_1.22", "PCI VGA BIOS Version 1.22")
    ROMX_LOAD("gd5434pci.bin", 0x00000, 0x8000, CRC(fa76dabf) SHA1(0310ef02df941e7d35e1d832400e2e4dd07d6309), ROM_BIOS(2) )
    ROM_SYSTEM_BIOS(3, "pci_ver_1.10b_jap", "PCI VGA BIOS Version 1.10B Japan chip")
    ROMX_LOAD("japan.bin", 0x00000, 0x8000, CRC(46fe9efa) SHA1(58712b00faf102509c4129c0babeb98df2b6e042), ROM_BIOS(3) )
    ROM_SYSTEM_BIOS(4, "gd5434pciv124", "CL-GD543x PCI VGA BIOS Version 1.24")
    ROMX_LOAD("1.24.bin", 0x00000,0x8000, CRC(174fa427) SHA1(3e490f3cc3af33dbfac6ff3ddac5e90bdc895646), ROM_BIOS(4) )
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
