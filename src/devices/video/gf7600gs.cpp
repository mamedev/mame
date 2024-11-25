// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "gf7600gs.h"

DEFINE_DEVICE_TYPE(GEFORCE_7600GS, geforce_7600gs_device, "geforce_7600gs", "NVIDIA GeForce 7600GS")

void geforce_7600gs_device::map1(address_map &map)
{
}

void geforce_7600gs_device::map2(address_map &map)
{
}

void geforce_7600gs_device::map3(address_map &map)
{
}

geforce_7600gs_device::geforce_7600gs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, GEFORCE_7600GS, tag, owner, clock)
	, m_vga(*this, "vga")
{
}

void geforce_7600gs_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update(m_vga, FUNC(nvidia_nv3_vga_device::screen_update));

	// TODO: very late superset (G73)
	NVIDIA_NV3_VGA(config, m_vga, 0);
	m_vga->set_screen("screen");
	// FIXME: shared RAM
	m_vga->set_vram_size(256*1024*1024);
}

void geforce_7600gs_device::device_start()
{
	pci_device::device_start();
	add_map( 16*1024*1024, M_MEM, FUNC(geforce_7600gs_device::map1));
	add_map(256*1024*1024, M_MEM, FUNC(geforce_7600gs_device::map2));
	add_map( 16*1024*1024, M_MEM, FUNC(geforce_7600gs_device::map3));
	add_rom_from_region();
}

void geforce_7600gs_device::device_reset()
{
	pci_device::device_reset();
}

void geforce_7600gs_device::legacy_memory_map(address_map &map)
{
	map(0xa0000, 0xbffff).rw(FUNC(geforce_7600gs_device::vram_r), FUNC(geforce_7600gs_device::vram_w));
}

void geforce_7600gs_device::legacy_io_map(address_map &map)
{
	map(0, 0x02f).m(m_vga, FUNC(nvidia_nv3_vga_device::io_map));
}

uint8_t geforce_7600gs_device::vram_r(offs_t offset)
{
	return downcast<nvidia_nv3_vga_device *>(m_vga.target())->mem_r(offset);
}

void geforce_7600gs_device::vram_w(offs_t offset, uint8_t data)
{
	downcast<nvidia_nv3_vga_device *>(m_vga.target())->mem_w(offset, data);
}

void geforce_7600gs_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	if (BIT(command, 1))
	{
		memory_space->install_readwrite_handler(0xa0000, 0xbffff, read8sm_delegate(*this, FUNC(geforce_7600gs_device::vram_r)), write8sm_delegate(*this, FUNC(geforce_7600gs_device::vram_w)));
	}

	if (BIT(command, 0))
	{
		io_space->install_device(0x03b0, 0x03df, *this, &geforce_7600gs_device::legacy_io_map);
		//memory_space->install_rom(0xc0000, 0xcffff, (void *)expansion_rom);
	}
}
