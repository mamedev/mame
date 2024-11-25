// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

WD9710 PCI cards

Essentially a WD90C33 upgraded to PCI.

TODO:
- Any mode beyond 4bpp is bound to fail;

**************************************************************************************************/

#include "emu.h"
#include "wd9710_pci.h"

#define LOG_WARN      (1U << 1)

#define VERBOSE (LOG_GENERAL | LOG_WARN)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)


DEFINE_DEVICE_TYPE(WD9710_PCI, wd9710_pci_device,   "wd9710_pci",   "Western Digital WD9710-MZ")
//DEFINE_DEVICE_TYPE(WD9712_PCI, wd9712_pci_device,   "wd9712_pci",   "Western Digital WD9712-??")


wd9710_pci_device::wd9710_pci_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
	, m_svga(*this, "svga")
	, m_vga_rom(*this, "vga_rom")
{
	set_ids(0x101c9710, 0x00, 0x030000, 0x101c9710);
}

wd9710_pci_device::wd9710_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: wd9710_pci_device(mconfig, WD9710_PCI, tag, owner, clock)
{
}

ROM_START( wd9710mz )
	ROM_REGION32_LE( 0x8000, "vga_rom", ROMREGION_ERASEFF )
	// Philips Paradise Pipeline 64
	ROM_SYSTEM_BIOS( 0, "paradise", "Paradise WD9710 v1.01.00 12/06/95")
	ROMX_LOAD( "bios.bin", 0x0000, 0x8000, CRC(98338249) SHA1(b5dbb28d60adf53c711ec16bc45eab102e40b6a2), ROM_BIOS(0) )
	ROM_IGNORE( 0x8000 )
ROM_END

const tiny_rom_entry *wd9710_pci_device::device_rom_region() const
{
	return ROM_NAME(wd9710mz);
}

void wd9710_pci_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update("svga", FUNC(wd90c33_vga_device::screen_update));

	// TODO: bump to specific WD9710 VGA core
	WD90C33(config, m_svga, 0);
	m_svga->set_screen("screen");
	// 2MB, 4MB
	m_svga->set_vram_size(4*1024*1024);
}

void wd9710_pci_device::device_start()
{
	pci_card_device::device_start();

	add_map(4*1024*1024, M_MEM, FUNC(wd9710_pci_device::vram_aperture_map));

	add_rom((u8 *)m_vga_rom->base(), 0x8000);
	expansion_rom_base = 0xc0000;

	// INTA#
	intr_pin = 1;
}

void wd9710_pci_device::device_reset()
{
	pci_card_device::device_reset();

	command = 0x0000;
	command_mask = 0x23;
	// TODO: configurable thru config bit 20
	status = 0x0200;

	remap_cb();
}

void wd9710_pci_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);
}

void wd9710_pci_device::vram_aperture_map(address_map &map)
{
	map(0x000000, 0x3fffff).rw(m_svga, FUNC(wd90c33_vga_device::mem_linear_r), FUNC(wd90c33_vga_device::mem_linear_w));
}

void wd9710_pci_device::legacy_io_map(address_map &map)
{
	map(0, 0x02f).m(m_svga, FUNC(wd90c33_vga_device::io_map));
}

uint8_t wd9710_pci_device::vram_r(offs_t offset)
{
	return downcast<wd90c33_vga_device *>(m_svga.target())->mem_r(offset);
}

void wd9710_pci_device::vram_w(offs_t offset, uint8_t data)
{
	downcast<wd90c33_vga_device *>(m_svga.target())->mem_w(offset, data);
}

void wd9710_pci_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	if (BIT(command, 1))
	{
		memory_space->install_readwrite_handler(0xa0000, 0xbffff, read8sm_delegate(*this, FUNC(wd9710_pci_device::vram_r)), write8sm_delegate(*this, FUNC(wd9710_pci_device::vram_w)));
	}

	if (BIT(command, 0))
	{
		io_space->install_device(0x03b0, 0x03df, *this, &wd9710_pci_device::legacy_io_map);
		// TODO: PnP ports (shouldn't have any other BAR space)
	}
}
