// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/***************************************************************************

  ISA VGA wrapper

***************************************************************************/

#include "emu.h"
#include "vga.h"
#include "video/pc_vga.h"
#include "screen.h"

ROM_START( ibm_vga )
	ROM_REGION(0x8000,"ibm_vga", 0)
	ROM_LOAD("ibm-vga.bin", 0x00000, 0x8000, BAD_DUMP CRC(74e3fadb) SHA1(dce6491424f1726203776dfae9a967a98a4ba7b5) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA8_VGA, isa8_vga_device, "ibm_vga", "IBM VGA Graphics Card")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa8_vga_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update(m_vga, FUNC(vga_device::screen_update));

	VGA(config, m_vga, 0);
	m_vga->set_screen("screen");
	m_vga->set_vram_size(0x100000);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *isa8_vga_device::device_rom_region() const
{
	return ROM_NAME( ibm_vga );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_vga_device - constructor
//-------------------------------------------------

isa8_vga_device::isa8_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA8_VGA, tag, owner, clock),
	device_isa8_card_interface(mconfig, *this),
	m_vga(*this, "vga")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
uint8_t isa8_vga_device::input_port_0_r() { return 0xff; } //return machine().root_device().ioport("IN0")->read(); }

void isa8_vga_device::device_start()
{
	set_isa_device();

	m_isa->install_rom(this, 0xc0000, 0xc7fff, "ibm_vga");

	m_isa->install_device(0x3b0, 0x3bf, read8sm_delegate(*m_vga, FUNC(vga_device::port_03b0_r)), write8sm_delegate(*m_vga, FUNC(vga_device::port_03b0_w)));
	m_isa->install_device(0x3c0, 0x3cf, read8sm_delegate(*m_vga, FUNC(vga_device::port_03c0_r)), write8sm_delegate(*m_vga, FUNC(vga_device::port_03c0_w)));
	m_isa->install_device(0x3d0, 0x3df, read8sm_delegate(*m_vga, FUNC(vga_device::port_03d0_r)), write8sm_delegate(*m_vga, FUNC(vga_device::port_03d0_w)));

	m_isa->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_vga, FUNC(vga_device::mem_r)), write8sm_delegate(*m_vga, FUNC(vga_device::mem_w)));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_vga_device::device_reset()
{
}
