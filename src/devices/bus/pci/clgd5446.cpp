// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Cirrus Logic CL-GD5446

Assume Rev. B

**************************************************************************************************/

#include "emu.h"
#include "clgd5446.h"

#include "screen.h"

#define LOG_WARN      (1U << 1)
#define LOG_TODO      (1U << 2) // log unimplemented registers

#define VERBOSE (LOG_GENERAL | LOG_WARN | LOG_TODO)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)
#define LOGTODO(...)            LOGMASKED(LOG_TODO, __VA_ARGS__)

DEFINE_DEVICE_TYPE(GD5446_PCI, cirrus_gd5446_pci_device, "clgd5446_pci", "Cirrus Logic GD5446 card")

cirrus_gd5446_pci_device::cirrus_gd5446_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, GD5446_PCI, tag, owner, clock)
	, m_vga(*this, "svga")
	, m_vga_rom(*this, "vga_rom")
{
	// subvendor ID: Returns values from ROM 0x7ffc-0x7ffe
	set_ids(0x101300b8, 0x00, 0x030000, 0x10130000);
}

ROM_START( gd5446 )
	// Cirrus Logic/Quadtel CL-GD5446 PCI VGA BIOS v1.31, AT27C256R
	// from photoply
	ROM_REGION32_LE( 0x8000, "vga_rom", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "quadtel", "Quadtel CL-GD5446 1.31" )
	ROMX_LOAD( "cl-gd5446_pci_vga_bios_version_1.31.u2", 0x0000, 0x8000, CRC(61f8cac7) SHA1(6e54aadfe10dfa5c7e417a054e9a64499a99083c), ROM_BIOS(0) )
	// Chip:CL-GD5446-HC-A - ROM: CL-GD5436/46 PCI VGA BIOS Version 1.25 - RAM: 1MB, 2MB, 4MB - 
	// OSC: 14.31818MHz - Connector: DB15 - VESA feature connector
	ROM_SYSTEM_BIOS( 1, "quadtelo", "Quadtel CL-GD5446 1.25" )
	ROMX_LOAD("cl-gd5446_pci_vga_bios_version_1.25.u2",  0x0000, 0x8000, CRC(7a859659) SHA1(ff667218261969c48082ec12aa91088a01b0cb2a), ROM_BIOS(1) )
//	ROMX_LOAD("5446bv.vbi", 0x00000, 0x10000, CRC(7a859659) SHA1(ff667218261969c48082ec12aa91088a01b0cb2a) )
ROM_END

const tiny_rom_entry *cirrus_gd5446_pci_device::device_rom_region() const
{
	return ROM_NAME(gd5446);
}

void cirrus_gd5446_pci_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update(m_vga, FUNC(cirrus_gd5446_vga_device::screen_update));

	CIRRUS_GD5446_VGA(config, m_vga, 0);
	m_vga->set_screen("screen");
	// 1MB or 2MB, max 4MB
	m_vga->set_vram_size(4*1024*1024);
}

void cirrus_gd5446_pci_device::device_start()
{
	pci_device::device_start();

	add_map( 32*1024*1024, M_MEM, FUNC(cirrus_gd5446_pci_device::vram_aperture_map));
	// TODO: special in Rev A, can be either M_MEM or M_IO thru CF8 / CF4 / CF3
	//add_map( 4096, M_MEM, FUNC(cirrus_gd5446_pci_device::mmio_map));
	add_map( 512, M_MEM, FUNC(cirrus_gd5446_pci_device::mmio_map));
	// TODO: Rev B do the same, except it just maps GPIO here thru CF8 / CF4
	add_map( 512, M_MEM, FUNC(cirrus_gd5446_pci_device::gpio_map));

	add_rom((u8 *)m_vga_rom->base(), 0x8000);
	expansion_rom_base = 0xc0000;

	// INTA#
	intr_pin = 1;
}

void cirrus_gd5446_pci_device::device_reset()
{
	pci_device::device_reset();

	command = 0x0000;
	command_mask = 0x0023;
	// medium DEVSEL#
	status = 0x0200;

	remap_cb();
}

void cirrus_gd5446_pci_device::config_map(address_map &map)
{
	pci_device::config_map(map);
}

void cirrus_gd5446_pci_device::vram_aperture_map(address_map &map)
{
	map(0x000000, 0x3fffff).rw(m_vga, FUNC(cirrus_gd5446_vga_device::mem_linear_r), FUNC(cirrus_gd5446_vga_device::mem_linear_w));
}

void cirrus_gd5446_pci_device::mmio_map(address_map &map)
{

}

void cirrus_gd5446_pci_device::gpio_map(address_map &map)
{

}

// TODO: this should really be a subclass of VGA
void cirrus_gd5446_pci_device::legacy_memory_map(address_map &map)
{
	map(0xa0000, 0xbffff).rw(FUNC(cirrus_gd5446_pci_device::vram_r), FUNC(cirrus_gd5446_pci_device::vram_w));
}

void cirrus_gd5446_pci_device::legacy_io_map(address_map &map)
{
	map(0, 0x02f).m(m_vga, FUNC(cirrus_gd5446_vga_device::io_map));
}

uint8_t cirrus_gd5446_pci_device::vram_r(offs_t offset)
{
	return downcast<cirrus_gd5446_vga_device *>(m_vga.target())->mem_r(offset);
}

void cirrus_gd5446_pci_device::vram_w(offs_t offset, uint8_t data)
{
	downcast<cirrus_gd5446_vga_device *>(m_vga.target())->mem_w(offset, data);
}

void cirrus_gd5446_pci_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	{
		memory_space->install_readwrite_handler(0xa0000, 0xbffff, read8sm_delegate(*this, FUNC(cirrus_gd5446_pci_device::vram_r)), write8sm_delegate(*this, FUNC(cirrus_gd5446_pci_device::vram_w)));

		io_space->install_device(0x03b0, 0x03df, *this, &cirrus_gd5446_pci_device::legacy_io_map);
	}
}
