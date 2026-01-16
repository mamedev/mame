// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * svga_trident.cpp
 *
 *  Created on: 6/09/2014
 *
 * TODO:
 * - tgui9680 is a PCI card, not ISA!
 * \- the only client is pc/calchase.cpp, which at the time of this writing uses legacy PCI bus;
 *
 */

#include "emu.h"
#include "svga_trident.h"
#include "video/pc_vga_trident.h"

#include "screen.h"

ROM_START( tvga9000 )
	ROM_REGION( 0x8000, "tvga9000", 0 )
	ROM_SYSTEM_BIOS(0, "v350", "Version D3.50 12/02/92")
	ROMX_LOAD( "trident_quadtel_tvga9000_isa16.bin", 0x0000, 0x8000, CRC(ad0e7351) SHA1(eb525460a80e1c1baa34642b93d54caf2607920d), ROM_BIOS(0) )
	ROM_IGNORE ( 0x8000 )
	ROM_SYSTEM_BIOS(1, "v300", "Version D3.00 11/12/91")
	ROMX_LOAD( "quadtel_tvga_bios_software_1991_ver_d3_0_lh.bin", 0x0000, 0x8000, CRC(d35a8ce9) SHA1(9cff3cb86a85dcbd34234bc4fd1dd0011f3d48fd), ROM_BIOS(1) )
	ROM_IGNORE ( 0x8000 )
ROM_END

DEFINE_DEVICE_TYPE(ISA16_SVGA_TVGA9000, isa16_svga_tvga9000_device, "tvga9000", "Trident/Quadtel TVGA9000B SVGA card")

void isa16_svga_tvga9000_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update(m_vga, FUNC(trident_vga_device::screen_update));

	TVGA9000_VGA(config, m_vga, 0);
	m_vga->set_screen("screen");
	m_vga->set_vram_size(512 * 1024); // 0x80000
}

const tiny_rom_entry *isa16_svga_tvga9000_device::device_rom_region() const
{
	return ROM_NAME( tvga9000 );
}

isa16_svga_tvga9000_device::isa16_svga_tvga9000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ISA16_SVGA_TVGA9000, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_vga(*this, "vga")
{
}

void isa16_svga_tvga9000_device::io_isa_map(address_map &map)
{
	map(0x00, 0x2f).m(m_vga, FUNC(tvga9000_device::io_map));
}

uint8_t isa16_svga_tvga9000_device::input_port_0_r() { return 0xff; } //return machine().root_device().ioport("IN0")->read(); }

void isa16_svga_tvga9000_device::device_start()
{
	set_isa_device();

	m_isa->install_rom(this, 0xc0000, 0xc7fff, "tvga9000");

	m_isa->install_device(0x03b0, 0x03df, *this, &isa16_svga_tvga9000_device::io_isa_map);

	m_isa->install_device(0x43c4, 0x43cb, read8sm_delegate(*m_vga, FUNC(tvga9000_device::port_43c6_r)), write8sm_delegate(*m_vga, FUNC(tvga9000_device::port_43c6_w)));
	m_isa->install_device(0x83c4, 0x83cb, read8sm_delegate(*m_vga, FUNC(tvga9000_device::port_83c6_r)), write8sm_delegate(*m_vga, FUNC(tvga9000_device::port_83c6_w)));

	m_isa->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_vga, FUNC(tvga9000_device::mem_r)), write8sm_delegate(*m_vga, FUNC(tvga9000_device::mem_w)));
}

/*
 * TGUI9680
 */

ROM_START( tgui9680 )
	ROM_REGION( 0x8000, "tgui9680", 0 )
	ROM_LOAD16_BYTE( "trident_tgui9680_bios.bin", 0x0000, 0x4000, CRC(1eebde64) SHA1(67896a854d43a575037613b3506aea6dae5d6a19) )
	ROM_CONTINUE(                                 0x0001, 0x4000 )
ROM_END


DEFINE_DEVICE_TYPE(ISA16_SVGA_TGUI9680, isa16_svga_tgui9680_device, "tgui9680", "Trident TGUI9680 Graphics Card (BIOS X5.5 (02) 02/13/96)")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa16_svga_tgui9680_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update(m_vga, FUNC(trident_vga_device::screen_update));

	TGUI9680_VGA(config, m_vga, 0);
	m_vga->set_screen("screen");
	m_vga->set_vram_size(0x200000);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *isa16_svga_tgui9680_device::device_rom_region() const
{
	return ROM_NAME( tgui9680 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_vga_device - constructor
//-------------------------------------------------

isa16_svga_tgui9680_device::isa16_svga_tgui9680_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ISA16_SVGA_TGUI9680, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_vga(*this, "vga")
{
}

void isa16_svga_tgui9680_device::io_isa_map(address_map &map)
{
	map(0x00, 0x2f).m(m_vga, FUNC(trident_vga_device::io_map));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
uint8_t isa16_svga_tgui9680_device::input_port_0_r() { return 0xff; } //return machine().root_device().ioport("IN0")->read(); }

void isa16_svga_tgui9680_device::device_start()
{
	set_isa_device();

	m_isa->install_rom(this, 0xc0000, 0xc7fff, "tgui9680");

	m_isa->install_device(0x03b0, 0x03df, *this, &isa16_svga_tgui9680_device::io_isa_map);

	m_isa->install_device(0x43c4, 0x43cb, read8sm_delegate(*m_vga, FUNC(trident_vga_device::port_43c6_r)), write8sm_delegate(*m_vga, FUNC(trident_vga_device::port_43c6_w)));
	m_isa->install_device(0x83c4, 0x83cb, read8sm_delegate(*m_vga, FUNC(trident_vga_device::port_83c6_r)), write8sm_delegate(*m_vga, FUNC(trident_vga_device::port_83c6_w)));

	m_isa->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_vga, FUNC(trident_vga_device::mem_r)), write8sm_delegate(*m_vga, FUNC(trident_vga_device::mem_w)));

	// uncomment to test Windows 3.1 TGUI9440AGi driver
//  m_isa->install_memory(0x4400000, 0x45fffff, read8sm_delegate(*m_vga, FUNC(trident_vga_device::vram_r)), write8sm_delegate(*m_vga, FUNC(trident_vga_device::vram_w)));

	// win95 drivers
//  m_isa->install_memory(0x4000000, 0x41fffff, read8sm_delegate(*m_vga, FUNC(trident_vga_device::vram_r)), write8sm_delegate(*m_vga, FUNC(trident_vga_device::vram_w)));

	// acceleration ports
	m_isa->install_device(0x2120, 0x21ff, read8sm_delegate(*m_vga, FUNC(trident_vga_device::accel_r)), write8sm_delegate(*m_vga, FUNC(trident_vga_device::accel_w)));
}
