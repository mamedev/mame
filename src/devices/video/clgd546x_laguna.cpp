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
	: pci_device(mconfig, CIRRUS_GD5465_LAGUNA3D, tag, owner, clock)
	, m_svga(*this, "svga")
	, m_vga_rom(*this, "vga_rom")
{
	// device ID 0x1013 Cirrus Logic
	// 0x00dx for Laguna revs
	// subvendor ID: Returns values from ROM 0x7ff8-0x7ffb
	set_ids_agp(0x101300d6, 0x00, 0x10130000);
	// TODO: class can be 0x80 (other display controller) with P18 pin.
}

ROM_START( gd5465 )
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
	screen.set_screen_update(m_svga, FUNC(cirrus_gd5446_device::screen_update));

	// TODO: bump to GD5465
	CIRRUS_GD5446(config, m_svga, 0);
	m_svga->set_screen("screen");
	// FIXME: shared RAM
	// in 4 and 8 MB versions
	m_svga->set_vram_size(8*1024*1024);
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
//	map(0xf8, 0xf8) PCI VGA Shadow Register
//	map(0xfc, 0xfc) PCI Vendor Specific Control Register
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
	map(0x03b0, 0x03bf).rw(FUNC(cirrus_gd5465_laguna3d_device::vga_3b0_r), FUNC(cirrus_gd5465_laguna3d_device::vga_3b0_w));
	map(0x03c0, 0x03cf).rw(FUNC(cirrus_gd5465_laguna3d_device::vga_3c0_r), FUNC(cirrus_gd5465_laguna3d_device::vga_3c0_w));
	map(0x03d0, 0x03df).rw(FUNC(cirrus_gd5465_laguna3d_device::vga_3d0_r), FUNC(cirrus_gd5465_laguna3d_device::vga_3d0_w));
}

uint8_t cirrus_gd5465_laguna3d_device::vram_r(offs_t offset)
{
	return downcast<cirrus_gd5446_device *>(m_svga.target())->mem_r(offset);
}

void cirrus_gd5465_laguna3d_device::vram_w(offs_t offset, uint8_t data)
{
	downcast<cirrus_gd5446_device *>(m_svga.target())->mem_w(offset, data);
}

u32 cirrus_gd5465_laguna3d_device::vga_3b0_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = 0;
	if (ACCESSING_BITS_0_7)
		result |= downcast<cirrus_gd5446_device *>(m_svga.target())->port_03b0_r(offset * 4 + 0) << 0;
	if (ACCESSING_BITS_8_15)
		result |= downcast<cirrus_gd5446_device *>(m_svga.target())->port_03b0_r(offset * 4 + 1) << 8;
	if (ACCESSING_BITS_16_23)
		result |= downcast<cirrus_gd5446_device *>(m_svga.target())->port_03b0_r(offset * 4 + 2) << 16;
	if (ACCESSING_BITS_24_31)
		result |= downcast<cirrus_gd5446_device *>(m_svga.target())->port_03b0_r(offset * 4 + 3) << 24;
	return result;
}

void cirrus_gd5465_laguna3d_device::vga_3b0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		downcast<cirrus_gd5446_device *>(m_svga.target())->port_03b0_w(offset * 4 + 0, data >> 0);
	if (ACCESSING_BITS_8_15)
		downcast<cirrus_gd5446_device *>(m_svga.target())->port_03b0_w(offset * 4 + 1, data >> 8);
	if (ACCESSING_BITS_16_23)
		downcast<cirrus_gd5446_device *>(m_svga.target())->port_03b0_w(offset * 4 + 2, data >> 16);
	if (ACCESSING_BITS_24_31)
		downcast<cirrus_gd5446_device *>(m_svga.target())->port_03b0_w(offset * 4 + 3, data >> 24);
}


u32 cirrus_gd5465_laguna3d_device::vga_3c0_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = 0;
	if (ACCESSING_BITS_0_7)
		result |= downcast<cirrus_gd5446_device *>(m_svga.target())->port_03c0_r(offset * 4 + 0) << 0;
	if (ACCESSING_BITS_8_15)
		result |= downcast<cirrus_gd5446_device *>(m_svga.target())->port_03c0_r(offset * 4 + 1) << 8;
	if (ACCESSING_BITS_16_23)
		result |= downcast<cirrus_gd5446_device *>(m_svga.target())->port_03c0_r(offset * 4 + 2) << 16;
	if (ACCESSING_BITS_24_31)
		result |= downcast<cirrus_gd5446_device *>(m_svga.target())->port_03c0_r(offset * 4 + 3) << 24;
	return result;
}

void cirrus_gd5465_laguna3d_device::vga_3c0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		downcast<cirrus_gd5446_device *>(m_svga.target())->port_03c0_w(offset * 4 + 0, data >> 0);
	if (ACCESSING_BITS_8_15)
		downcast<cirrus_gd5446_device *>(m_svga.target())->port_03c0_w(offset * 4 + 1, data >> 8);
	if (ACCESSING_BITS_16_23)
		downcast<cirrus_gd5446_device *>(m_svga.target())->port_03c0_w(offset * 4 + 2, data >> 16);
	if (ACCESSING_BITS_24_31)
		downcast<cirrus_gd5446_device *>(m_svga.target())->port_03c0_w(offset * 4 + 3, data >> 24);
}

u32 cirrus_gd5465_laguna3d_device::vga_3d0_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = 0;
	if (ACCESSING_BITS_0_7)
		result |= downcast<cirrus_gd5446_device *>(m_svga.target())->port_03d0_r(offset * 4 + 0) << 0;
	if (ACCESSING_BITS_8_15)
		result |= downcast<cirrus_gd5446_device *>(m_svga.target())->port_03d0_r(offset * 4 + 1) << 8;
	if (ACCESSING_BITS_16_23)
		result |= downcast<cirrus_gd5446_device *>(m_svga.target())->port_03d0_r(offset * 4 + 2) << 16;
	if (ACCESSING_BITS_24_31)
		result |= downcast<cirrus_gd5446_device *>(m_svga.target())->port_03d0_r(offset * 4 + 3) << 24;
	return result;
}

void cirrus_gd5465_laguna3d_device::vga_3d0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		downcast<cirrus_gd5446_device *>(m_svga.target())->port_03d0_w(offset * 4 + 0, data >> 0);
	if (ACCESSING_BITS_8_15)
		downcast<cirrus_gd5446_device *>(m_svga.target())->port_03d0_w(offset * 4 + 1, data >> 8);
	if (ACCESSING_BITS_16_23)
		downcast<cirrus_gd5446_device *>(m_svga.target())->port_03d0_w(offset * 4 + 2, data >> 16);
	if (ACCESSING_BITS_24_31)
		downcast<cirrus_gd5446_device *>(m_svga.target())->port_03d0_w(offset * 4 + 3, data >> 24);
}

void cirrus_gd5465_laguna3d_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	if (m_vga_legacy_enable)
	{
		memory_space->install_readwrite_handler(0xa0000, 0xbffff, read8sm_delegate(*this, FUNC(cirrus_gd5465_laguna3d_device::vram_r)), write8sm_delegate(*this, FUNC(cirrus_gd5465_laguna3d_device::vram_w)));

		io_space->install_readwrite_handler(0x3b0, 0x3bf, read32s_delegate(*this, FUNC(cirrus_gd5465_laguna3d_device::vga_3b0_r)), write32s_delegate(*this, FUNC(cirrus_gd5465_laguna3d_device::vga_3b0_w)));
		io_space->install_readwrite_handler(0x3c0, 0x3cf, read32s_delegate(*this, FUNC(cirrus_gd5465_laguna3d_device::vga_3c0_r)), write32s_delegate(*this, FUNC(cirrus_gd5465_laguna3d_device::vga_3c0_w)));
		io_space->install_readwrite_handler(0x3d0, 0x3df, read32s_delegate(*this, FUNC(cirrus_gd5465_laguna3d_device::vga_3d0_r)), write32s_delegate(*this, FUNC(cirrus_gd5465_laguna3d_device::vga_3d0_w)));
		//memory_space->install_rom(0xc0000, 0xcffff, (void *)expansion_rom);
	}
}
