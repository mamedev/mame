// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * svga_trident.c
 *
 *  Created on: 6/09/2014
 */

#include "emu.h"
#include "svga_trident.h"
#include "video/pc_vga.h"


ROM_START( tgui9680 )
	ROM_REGION( 0x8000, "tgui9680", 0 )
	ROM_LOAD16_BYTE( "trident_tgui9680_bios.bin", 0x0000, 0x4000, CRC(1eebde64) SHA1(67896a854d43a575037613b3506aea6dae5d6a19) )
	ROM_CONTINUE(                                 0x0001, 0x4000 )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA16_SVGA_TGUI9680 = &device_creator<isa16_svga_tgui9680_device>;


static MACHINE_CONFIG_FRAGMENT( vga_trident )
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_25_1748MHz,900,0,640,526,0,480)
	MCFG_SCREEN_UPDATE_DEVICE("vga", trident_vga_device, screen_update)

	MCFG_PALETTE_ADD("palette", 0x100)

	MCFG_DEVICE_ADD("vga", TRIDENT_VGA, 0)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor isa16_svga_tgui9680_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( vga_trident );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *isa16_svga_tgui9680_device::device_rom_region() const
{
	return ROM_NAME( tgui9680 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_vga_device - constructor
//-------------------------------------------------

isa16_svga_tgui9680_device::isa16_svga_tgui9680_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, ISA16_SVGA_TGUI9680, "Trident TGUI9680 Graphics Card (BIOS X5.5 (02) 02/13/96)", tag, owner, clock, "tgui9680", __FILE__),
		device_isa16_card_interface(mconfig, *this), m_vga(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
READ8_MEMBER(isa16_svga_tgui9680_device::input_port_0_r ) { return 0xff; } //return space.machine().root_device().ioport("IN0")->read(); }

void isa16_svga_tgui9680_device::device_start()
{
	set_isa_device();

	m_vga = subdevice<trident_vga_device>("vga");

	m_isa->install_rom(this, 0xc0000, 0xc7fff, 0, 0, "tgui9680", "tgui9680");

	m_isa->install_device(0x3b0, 0x3bf, 0, 0, read8_delegate(FUNC(trident_vga_device::port_03b0_r),m_vga), write8_delegate(FUNC(trident_vga_device::port_03b0_w),m_vga));
	m_isa->install_device(0x3c0, 0x3cf, 0, 0, read8_delegate(FUNC(trident_vga_device::port_03c0_r),m_vga), write8_delegate(FUNC(trident_vga_device::port_03c0_w),m_vga));
	m_isa->install_device(0x3d0, 0x3df, 0, 0, read8_delegate(FUNC(trident_vga_device::port_03d0_r),m_vga), write8_delegate(FUNC(trident_vga_device::port_03d0_w),m_vga));
	m_isa->install_device(0x43c4, 0x43cb, 0, 0, read8_delegate(FUNC(trident_vga_device::port_43c6_r),m_vga), write8_delegate(FUNC(trident_vga_device::port_43c6_w),m_vga));
	m_isa->install_device(0x83c4, 0x83cb, 0, 0, read8_delegate(FUNC(trident_vga_device::port_83c6_r),m_vga), write8_delegate(FUNC(trident_vga_device::port_83c6_w),m_vga));

	m_isa->install_memory(0xa0000, 0xbffff, 0, 0, read8_delegate(FUNC(trident_vga_device::mem_r),m_vga), write8_delegate(FUNC(trident_vga_device::mem_w),m_vga));

	// uncomment to test Windows 3.1 TGUI9440AGi driver
//  m_isa->install_memory(0x4400000, 0x45fffff, 0, 0, read8_delegate(FUNC(trident_vga_device::vram_r),m_vga), write8_delegate(FUNC(trident_vga_device::vram_w),m_vga));

	// win95 drivers
//  m_isa->install_memory(0x4000000, 0x41fffff, 0, 0, read8_delegate(FUNC(trident_vga_device::vram_r),m_vga), write8_delegate(FUNC(trident_vga_device::vram_w),m_vga));

	// acceleration ports
	m_isa->install_device(0x2120, 0x21ff, 0, 0, read8_delegate(FUNC(trident_vga_device::accel_r),m_vga), write8_delegate(FUNC(trident_vga_device::accel_w),m_vga));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa16_svga_tgui9680_device::device_reset()
{
}
