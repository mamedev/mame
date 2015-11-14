// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/***************************************************************************

  ISA VGA wrapper

***************************************************************************/

#include "emu.h"
#include "vga.h"
#include "video/pc_vga.h"

ROM_START( ibm_vga )
	ROM_REGION(0x8000,"ibm_vga", 0)
	ROM_LOAD("ibm-vga.bin", 0x00000, 0x8000, BAD_DUMP CRC(74e3fadb) SHA1(dce6491424f1726203776dfae9a967a98a4ba7b5) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA8_VGA = &device_creator<isa8_vga_device>;


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor isa8_vga_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( pcvideo_vga );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *isa8_vga_device::device_rom_region() const
{
	return ROM_NAME( ibm_vga );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_vga_device - constructor
//-------------------------------------------------

isa8_vga_device::isa8_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, ISA8_VGA, "IBM VGA Graphics Card", tag, owner, clock, "ibm_vga", __FILE__),
		device_isa8_card_interface(mconfig, *this), m_vga(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
READ8_MEMBER( isa8_vga_device::input_port_0_r ) { return 0xff; } //return space.machine().root_device().ioport("IN0")->read(); }

void isa8_vga_device::device_start()
{
	set_isa_device();

	m_vga = subdevice<vga_device>("vga");

	m_isa->install_rom(this, 0xc0000, 0xc7fff, 0, 0, "ibm_vga", "ibm_vga");

	m_isa->install_device(0x3b0, 0x3bf, 0, 0, read8_delegate(FUNC(vga_device::port_03b0_r),m_vga), write8_delegate(FUNC(vga_device::port_03b0_w),m_vga));
	m_isa->install_device(0x3c0, 0x3cf, 0, 0, read8_delegate(FUNC(vga_device::port_03c0_r),m_vga), write8_delegate(FUNC(vga_device::port_03c0_w),m_vga));
	m_isa->install_device(0x3d0, 0x3df, 0, 0, read8_delegate(FUNC(vga_device::port_03d0_r),m_vga), write8_delegate(FUNC(vga_device::port_03d0_w),m_vga));

	m_isa->install_memory(0xa0000, 0xbffff, 0, 0, read8_delegate(FUNC(vga_device::mem_r),m_vga), write8_delegate(FUNC(vga_device::mem_w),m_vga));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_vga_device::device_reset()
{
}
