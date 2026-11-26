// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * svga_trident.cpp
 *
 *  Created on: 6/09/2014
 *
 *
 */

#include "emu.h"
#include "svga_trident.h"
#include "video/pc_vga_trident.h"

#include "screen.h"

ROM_START( tvga9000 )
	ROM_REGION( 0x8000, "bios", 0 )
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
	screen_device &screen(SCREEN(config, "screen"));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update(m_vga, FUNC(trident_vga_device::screen_update));

	TVGA9000_VGA(config, m_vga);
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

uint8_t isa16_svga_tvga9000_device::input_port_0_r() { return 0xff; } //return machine().root_device().ioport("IN0")->read(); }

void isa16_svga_tvga9000_device::device_start()
{
	set_isa_device();
}

void isa16_svga_tvga9000_device::device_reset()
{
	remap(AS_PROGRAM, 0, 0xfffff);
	remap(AS_IO, 0, 0xffff);
}

void isa16_svga_tvga9000_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		m_isa->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_vga, FUNC(tvga9000_vga_device::mem_r)), write8sm_delegate(*m_vga, FUNC(tvga9000_vga_device::mem_w)));
		m_isa->install_rom(this, 0xc0000, 0xc7fff, "bios");
	}
	else if (space_id == AS_IO)
		m_isa->install_device(0x0000, 0xffff, *this, &isa16_svga_tvga9000_device::io_isa_map);
}

void isa16_svga_tvga9000_device::io_isa_map(address_map &map)
{
	map(0x03b0, 0x03df).m(m_vga, FUNC(tvga9000_vga_device::io_map));
	map(0x43c4, 0x43cb).rw(m_vga, FUNC(tvga9000_vga_device::port_43c6_r), FUNC(tvga9000_vga_device::port_43c6_w));
	map(0x83c4, 0x83cb).rw(m_vga, FUNC(tvga9000_vga_device::port_83c6_r), FUNC(tvga9000_vga_device::port_83c6_w));
}
