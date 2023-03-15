// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/***************************************************************************

  ISA SVGA Tseng wrapper

***************************************************************************/

#include "emu.h"
#include "svga_tseng.h"
#include "video/pc_vga.h"

#include "screen.h"


ROM_START( et4000 )
	ROM_REGION(0x8000,"et4000", 0)
	ROM_LOAD("et4000.bin", 0x00000, 0x8000, CRC(f1e817a8) SHA1(945d405b0fb4b8f26830d495881f8587d90e5ef9) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA8_SVGA_ET4K, isa8_svga_et4k_device, "et4000", "SVGA Tseng ET4000 Graphics Card")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa8_svga_et4k_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update(m_vga, FUNC(tseng_vga_device::screen_update));

	TSENG_VGA(config, m_vga, 0);
	m_vga->set_screen("screen");
	m_vga->set_vram_size(0x100000);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *isa8_svga_et4k_device::device_rom_region() const
{
	return ROM_NAME( et4000 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_vga_device - constructor
//-------------------------------------------------

isa8_svga_et4k_device::isa8_svga_et4k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA8_SVGA_ET4K, tag, owner, clock),
	device_isa8_card_interface(mconfig, *this),
	m_vga(*this, "vga")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
uint8_t isa8_svga_et4k_device::input_port_0_r() { return 0xff; } //return machine().root_device().ioport("IN0")->read(); }

void isa8_svga_et4k_device::device_start()
{
	set_isa_device();

	map_io();
	map_ram();
	map_rom();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_svga_et4k_device::device_reset()
{
}

//-------------------------------------------------
//  remap - remap ram/io since something
//  could have unmapped it
//-------------------------------------------------

void isa8_svga_et4k_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		map_ram();
		map_rom();
	}
	else if (space_id == AS_IO)
		map_io();
}

void isa8_svga_et4k_device::map_io()
{
	m_isa->install_device(0x3b0, 0x3bf, read8sm_delegate(*m_vga, FUNC(tseng_vga_device::port_03b0_r)), write8sm_delegate(*m_vga, FUNC(tseng_vga_device::port_03b0_w)));
	m_isa->install_device(0x3c0, 0x3cf, read8sm_delegate(*m_vga, FUNC(tseng_vga_device::port_03c0_r)), write8sm_delegate(*m_vga, FUNC(tseng_vga_device::port_03c0_w)));
	m_isa->install_device(0x3d0, 0x3df, read8sm_delegate(*m_vga, FUNC(tseng_vga_device::port_03d0_r)), write8sm_delegate(*m_vga, FUNC(tseng_vga_device::port_03d0_w)));
}

void isa8_svga_et4k_device::map_ram()
{
	m_isa->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_vga, FUNC(tseng_vga_device::mem_r)), write8sm_delegate(*m_vga, FUNC(tseng_vga_device::mem_w)));
}

void isa8_svga_et4k_device::map_rom()
{
	m_isa->install_rom(this, 0xc0000, 0xc7fff, "et4000");
}
