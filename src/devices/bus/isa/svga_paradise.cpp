// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

ISA SVGA Paradise / Western Digital wrapper

TODO:
- Add ISA8 and MCA variants for pvga1a_jk;
- Nokia NVGA2 doesn't boot;

***************************************************************************/

#include "emu.h"
#include "svga_paradise.h"

#include "screen.h"

DEFINE_DEVICE_TYPE(ISA16_PVGA1A,    isa16_pvga1a_device,    "pvga1a", "Paradise Systems PVGA1A Graphics Card")
DEFINE_DEVICE_TYPE(ISA16_PVGA1A_JK, isa16_pvga1a_jk_device, "pvga1a_jk", "Paradise Systems PVGA1A-JK Graphics Card")
DEFINE_DEVICE_TYPE(ISA8_WD90C90_JK, isa8_wd90c90_jk_device, "wd90c90_jk", "Western Digital WD90C90-JK Graphics Card")


isa16_pvga1a_device::isa16_pvga1a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA16_PVGA1A, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this),
	m_vga(*this, "vga")
{
}

ROM_START( pvga1a )
	ROM_REGION(0x8000,"vga_rom", 0)
	ROM_SYSTEM_BIOS(0, "pvga1a", "Paradise Systems PVGA1A")
	// TODO: pinpoint what rom size are these
	// BIOS.BIN     1xxxxxxxxxxxxxxx = 0xFF
	ROMX_LOAD("bios.bin", 0x000000, 0x008000, CRC(2be5d405) SHA1(5e3b4ebae221b7ad02f3eaa052178cb39d1d9bbe), ROM_BIOS(0))
	ROM_IGNORE( 0x8000 )
	// Western Digital licensed
	ROM_SYSTEM_BIOS(1, "go481", "Olivetti GO481")
	ROMX_LOAD("oli_go481_hi.bin", 0x000001, 0x004000, CRC(cda447be) SHA1(f397e3ddd3885666e3151fb4f681abf47fef36ba), ROM_SKIP(1) | ROM_BIOS(1))
	ROM_IGNORE( 0x4000 )
	ROMX_LOAD("oli_go481_lo.bin", 0x000000, 0x004000, CRC(7db97902) SHA1(b93aa9d45942e98fb4932b4db34b82d459060adf), ROM_SKIP(1) | ROM_BIOS(1))
	ROM_IGNORE( 0x4000 )

	// There's also a:
	// ROMX_LOAD( "paradisepvga1a.bin", 0x000000, 0x008000, CRC(e7c6883a) SHA1(61ae199d3a9077844c8a1aa80c3f5804c29383e8), ROM_BIOS(N) )
	// BIOS.BIN     [1/4]      paradisepvga1a.BIN [1/2]      IDENTICAL
	// Which doesn't boot with invalid REP opcode, assume bad.
ROM_END

const tiny_rom_entry *isa16_pvga1a_device::device_rom_region() const
{
	return ROM_NAME( pvga1a );
}

void isa16_pvga1a_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update("vga", FUNC(pvga1a_vga_device::screen_update));

	PVGA1A(config, m_vga, 0);
	m_vga->set_screen("screen");
	m_vga->set_vram_size(0x100000);
}

void isa16_pvga1a_device::io_isa_map(address_map &map)
{
	map(0x00, 0x2f).m(m_vga, FUNC(pvga1a_vga_device::io_map));
}

void isa16_pvga1a_device::device_start()
{
	set_isa_device();

	m_isa->install_rom(this, 0xc0000, 0xc7fff, "vga_rom");

	m_isa->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_vga, FUNC(pvga1a_vga_device::mem_r)), write8sm_delegate(*m_vga, FUNC(pvga1a_vga_device::mem_w)));
	m_isa->install_device(0x03b0, 0x03df, *this, &isa16_pvga1a_device::io_isa_map);
}

/******************
 *
 * PVGA1A-JK
 *
 *****************/

isa16_pvga1a_jk_device::isa16_pvga1a_jk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA16_PVGA1A_JK, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this),
	m_vga(*this, "vga")
{
}

ROM_START( pvga1a_jk )
	ROM_REGION(0x8000,"vga_rom", ROMREGION_ERASEFF)
	// "Paradise Basic VGA"?
	ROM_SYSTEM_BIOS(0, "pvga1a_jk", "Paradise Systems PVGA1A-JK")
	// BIOS.BIN     1xxxxxxxxxxxxxxx = 0xFF
    ROMX_LOAD( "bios.bin",     0x000000, 0x08000, CRC(2bfe8adb) SHA1(afd8c33f24e28b025e43ae68d95fd6811659013b), ROM_BIOS(0) )
	ROM_IGNORE( 0x8000 )
	ROM_SYSTEM_BIOS(1, "nvga2", "Nokia NVGA2")
	ROMX_LOAD( "nokia.vbi",    0x000000, 0x06000, CRC(9f430ae7) SHA1(3d37b86853347d43ebc85a7e92e4a609b13406bb), ROM_BIOS(1) )
ROM_END

const tiny_rom_entry *isa16_pvga1a_jk_device::device_rom_region() const
{
	return ROM_NAME( pvga1a_jk );
}

void isa16_pvga1a_jk_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update("vga", FUNC(pvga1a_vga_device::screen_update));

	// TODO: is there any real difference between PVGA1A and PVGA1A-JK VGA controller wise?
	PVGA1A(config, m_vga, 0);
	m_vga->set_screen("screen");
	// 256kB to 1MB
	m_vga->set_vram_size(0x100000);
}

void isa16_pvga1a_jk_device::io_isa_map(address_map &map)
{
	map(0x00, 0x2f).m(m_vga, FUNC(pvga1a_vga_device::io_map));
}

void isa16_pvga1a_jk_device::device_start()
{
	set_isa_device();

	m_isa->install_rom(this, 0xc0000, 0xc7fff, "vga_rom");

	m_isa->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_vga, FUNC(pvga1a_vga_device::mem_r)), write8sm_delegate(*m_vga, FUNC(pvga1a_vga_device::mem_w)));
	m_isa->install_device(0x03b0, 0x03df, *this, &isa16_pvga1a_jk_device::io_isa_map);
}

/******************
 *
 * WD90C90-JK
 *
 *****************/

isa8_wd90c90_jk_device::isa8_wd90c90_jk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA8_WD90C90_JK, tag, owner, clock),
	device_isa8_card_interface(mconfig, *this),
	m_vga(*this, "vga")
{
}

ROM_START( wd90c90_jk )
	ROM_REGION(0x8000,"vga_rom", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "wd90c90_jk", "ZTECH ZPVGA")
	ROMX_LOAD( "bios.bin", 0x000000, 0x008000, CRC(a7f0e81c) SHA1(578ee0e8f9a5e56df6c05386978d0f41c638ddcf), ROM_BIOS(0) )
	ROM_IGNORE( 0x8000 )
ROM_END

const tiny_rom_entry *isa8_wd90c90_jk_device::device_rom_region() const
{
	return ROM_NAME( wd90c90_jk );
}

void isa8_wd90c90_jk_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update("vga", FUNC(pvga1a_vga_device::screen_update));

	PVGA1A(config, m_vga, 0);
	m_vga->set_screen("screen");
	// 256kB to 1MB
	m_vga->set_vram_size(0x100000);
}

void isa8_wd90c90_jk_device::io_isa_map(address_map &map)
{
	map(0x00, 0x2f).m(m_vga, FUNC(pvga1a_vga_device::io_map));
}

void isa8_wd90c90_jk_device::device_start()
{
	set_isa_device();

	m_isa->install_rom(this, 0xc0000, 0xc7fff, "vga_rom");

	m_isa->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_vga, FUNC(pvga1a_vga_device::mem_r)), write8sm_delegate(*m_vga, FUNC(pvga1a_vga_device::mem_w)));
	m_isa->install_device(0x03b0, 0x03df, *this, &isa8_wd90c90_jk_device::io_isa_map);
}
