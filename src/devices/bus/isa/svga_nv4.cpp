// license:BSD-3-Clause
// copyright-holders:Darius Goad
/***************************************************************************

  ISA SVGA NV4 wrapper

***************************************************************************/

#include "emu.h"
#include "svga_nv4.h"

ROM_START( nv4 )
	ROM_REGION(0x10000,"nv4", 0)

	ROM_SYSTEM_BIOS( 0, "rivatnt", "nVidia RIVA TNT" )
	ROMX_LOAD("rivatnt.bin", 0x00000, 0x10000, CRC(6c98a42c) SHA1(a77244d634b7aad2c8e7edb4bc188c116c75f57f), ROM_BIOS(1) )
	ROM_IGNORE( 0x10000 )

ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA16_SVGA_NV4 = &device_creator<isa16_svga_nv4_device>;

static MACHINE_CONFIG_FRAGMENT( vga_nv4 )
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_25_1748MHz,900,0,640,526,0,480)
	MCFG_SCREEN_UPDATE_DEVICE("vga", nv4_vga_device, screen_update)

	MCFG_PALETTE_ADD("palette", 0x100)

	MCFG_DEVICE_ADD("vga", NV4, 0)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor isa16_svga_nv4_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( vga_nv4 );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *isa16_svga_nv4_device::device_rom_region() const
{
	return ROM_NAME( nv4 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa16_vga_device - constructor
//-------------------------------------------------

isa16_svga_nv4_device::isa16_svga_nv4_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, ISA16_SVGA_NV4, "nVidia RIVA TNT Graphics Card", tag, owner, clock, "nv4", __FILE__),
		device_isa16_card_interface(mconfig, *this), m_vga(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
READ8_MEMBER(isa16_svga_nv4_device::input_port_0_r ) { return 0xff; } //return space.machine().root_device().ioport("IN0")->read(); }

void isa16_svga_nv4_device::device_start()
{
	set_isa_device();

	m_vga = subdevice<nv4_vga_device>("vga");

	m_isa->install_rom(this, 0xc0000, 0xcffff, 0, 0, "svga", "nv4");

	m_isa->install_device(0x03b0, 0x03bf, 0, 0, read8_delegate(FUNC(nv4_vga_device::port_03b0_r),m_vga), write8_delegate(FUNC(nv4_vga_device::port_03b0_w),m_vga));
	m_isa->install_device(0x03c0, 0x03cf, 0, 0, read8_delegate(FUNC(vga_device::port_03c0_r),m_vga), write8_delegate(FUNC(vga_device::port_03c0_w),m_vga));
	m_isa->install_device(0x03d0, 0x03df, 0, 0, read8_delegate(FUNC(nv4_vga_device::port_03d0_r),m_vga), write8_delegate(FUNC(nv4_vga_device::port_03d0_w),m_vga));

	m_isa->install_memory(0xa0000, 0xbffff, 0, 0, read8_delegate(FUNC(nv4_vga_device::mem_r),m_vga), write8_delegate(FUNC(nv4_vga_device::mem_w),m_vga));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa16_svga_nv4_device::device_reset()
{
}
