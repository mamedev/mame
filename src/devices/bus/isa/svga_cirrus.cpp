// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/***************************************************************************

  ISA SVGA Cirrus Logic wrapper

***************************************************************************/

#include "emu.h"
#include "svga_cirrus.h"

#include "screen.h"


ROM_START( dm_clgd5430 )
	ROM_REGION(0x8000,"dm_clgd5430", 0)
	ROM_LOAD("speedstar_pro_se_v1.00.u2", 0x00000, 0x8000, CRC(ed79572c) SHA1(15131e2b2db7a34971083a250e4a21ab7bd64a9d) )
	ROM_IGNORE( 0x8000 )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA16_SVGA_CIRRUS,        isa16_svga_cirrus_device,        "dm_clgd5430", "Diamond Speedstar Pro SE ISA Graphics Card (BIOS v1.00)")
DEFINE_DEVICE_TYPE(ISA16_SVGA_CIRRUS_GD542X, isa16_svga_cirrus_gd542x_device, "clgd542x",    "Generic Cirrus Logic CD542 Graphics Card (BIOS v1.20)")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa16_svga_cirrus_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update("vga", FUNC(cirrus_gd5430_device::screen_update));

	CIRRUS_GD5430(config, m_vga, 0);
	m_vga->set_screen("screen");
	m_vga->set_vram_size(0x200000);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *isa16_svga_cirrus_device::device_rom_region() const
{
	return ROM_NAME( dm_clgd5430 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa16_vga_device - constructor
//-------------------------------------------------

isa16_svga_cirrus_device::isa16_svga_cirrus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA16_SVGA_CIRRUS, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this),
	m_vga(*this, "vga")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
uint8_t isa16_svga_cirrus_device::input_port_0_r() { return 0xff; } //return machine().root_device().ioport("IN0")->read(); }

void isa16_svga_cirrus_device::device_start()
{
	set_isa_device();

	m_isa->install_rom(this, 0xc0000, 0xc7fff, "dm_clgd5430");

	m_isa->install_device(0x03b0, 0x03bf, read8sm_delegate(*m_vga, FUNC(cirrus_gd5430_device::port_03b0_r)), write8sm_delegate(*m_vga, FUNC(cirrus_gd5430_device::port_03b0_w)));
	m_isa->install_device(0x03c0, 0x03cf, read8sm_delegate(*m_vga, FUNC(cirrus_gd5430_device::port_03c0_r)), write8sm_delegate(*m_vga, FUNC(cirrus_gd5430_device::port_03c0_w)));
	m_isa->install_device(0x03d0, 0x03df, read8sm_delegate(*m_vga, FUNC(cirrus_gd5430_device::port_03d0_r)), write8sm_delegate(*m_vga, FUNC(cirrus_gd5430_device::port_03d0_w)));

	m_isa->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_vga, FUNC(cirrus_gd5430_device::mem_r)), write8sm_delegate(*m_vga, FUNC(cirrus_gd5430_device::mem_w)));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa16_svga_cirrus_device::device_reset()
{
}


/*
    Generic CL-GD542x video card
*/

ROM_START( clgd542x )
	ROM_REGION(0x08000, "clgd542x", 0)
	ROM_LOAD16_BYTE("techyosd-isa-bios-v1.2.u8",     0x00000, 0x04000, CRC(6adf7e71) SHA1(2b07d964cc7c2c0aa560625b7c12f38d4537d652) )
	ROM_CONTINUE(                                    0x00001, 0x04000 )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa16_svga_cirrus_gd542x_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update("vga", FUNC(cirrus_gd5428_device::screen_update));

	CIRRUS_GD5428(config, m_vga, 0);
	m_vga->set_screen("screen");
	m_vga->set_vram_size(0x200000);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *isa16_svga_cirrus_gd542x_device::device_rom_region() const
{
	return ROM_NAME( clgd542x );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa16_vga_device - constructor
//-------------------------------------------------

isa16_svga_cirrus_gd542x_device::isa16_svga_cirrus_gd542x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA16_SVGA_CIRRUS_GD542X, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this),
	m_vga(*this, "vga")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
uint8_t isa16_svga_cirrus_gd542x_device::input_port_0_r() { return 0xff; } //return machine().root_device().ioport("IN0")->read(); }

void isa16_svga_cirrus_gd542x_device::device_start()
{
	set_isa_device();

	m_isa->install_rom(this, 0xc0000, 0xc7fff, "clgd542x");

	m_isa->install_device(0x03b0, 0x03bf, read8sm_delegate(*m_vga, FUNC(cirrus_gd5428_device::port_03b0_r)), write8sm_delegate(*m_vga, FUNC(cirrus_gd5428_device::port_03b0_w)));
	m_isa->install_device(0x03c0, 0x03cf, read8sm_delegate(*m_vga, FUNC(cirrus_gd5428_device::port_03c0_r)), write8sm_delegate(*m_vga, FUNC(cirrus_gd5428_device::port_03c0_w)));
	m_isa->install_device(0x03d0, 0x03df, read8sm_delegate(*m_vga, FUNC(cirrus_gd5428_device::port_03d0_r)), write8sm_delegate(*m_vga, FUNC(cirrus_gd5428_device::port_03d0_w)));

	m_isa->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_vga, FUNC(cirrus_gd5428_device::mem_r)), write8sm_delegate(*m_vga, FUNC(cirrus_gd5428_device::mem_w)));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa16_svga_cirrus_gd542x_device::device_reset()
{
}
