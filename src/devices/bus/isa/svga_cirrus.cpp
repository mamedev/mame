// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/***************************************************************************

  ISA SVGA Cirrus Logic wrapper

***************************************************************************/

#include "emu.h"
#include "svga_cirrus.h"

ROM_START( dm_clgd5430 )
	ROM_REGION(0x8000,"dm_clgd5430", 0)
	ROM_LOAD("speedstar_pro_se_v1.00.u2", 0x00000, 0x8000, CRC(ed79572c) SHA1(15131e2b2db7a34971083a250e4a21ab7bd64a9d) )
	ROM_IGNORE( 0x8000 )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA16_SVGA_CIRRUS = &device_creator<isa16_svga_cirrus_device>;
const device_type ISA16_SVGA_CIRRUS_GD542X = &device_creator<isa16_svga_cirrus_gd542x_device>;

static MACHINE_CONFIG_FRAGMENT( vga_cirrus )
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_25_1748MHz,900,0,640,526,0,480)
	MCFG_SCREEN_UPDATE_DEVICE("vga", cirrus_gd5430_device, screen_update)

	MCFG_PALETTE_ADD("palette", 0x100)

	MCFG_DEVICE_ADD("vga", CIRRUS_GD5430, 0)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor isa16_svga_cirrus_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( vga_cirrus );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *isa16_svga_cirrus_device::device_rom_region() const
{
	return ROM_NAME( dm_clgd5430 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa16_vga_device - constructor
//-------------------------------------------------

isa16_svga_cirrus_device::isa16_svga_cirrus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, ISA16_SVGA_CIRRUS, "Diamond Speedstar Pro SE ISA Graphics Card (BIOS v1.00)", tag, owner, clock, "dm_clgd5430", __FILE__),
		device_isa16_card_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
READ8_MEMBER(isa16_svga_cirrus_device::input_port_0_r ) { return 0xff; } //return space.machine().root_device().ioport("IN0")->read(); }

void isa16_svga_cirrus_device::device_start()
{
	set_isa_device();

	m_vga = subdevice<cirrus_gd5430_device>("vga");

	m_isa->install_rom(this, 0xc0000, 0xc7fff, 0, 0, "svga", "dm_clgd5430");

	m_isa->install_device(0x03b0, 0x03bf, 0, 0, read8_delegate(FUNC(cirrus_gd5430_device::port_03b0_r),m_vga), write8_delegate(FUNC(cirrus_gd5430_device::port_03b0_w),m_vga));
	m_isa->install_device(0x03c0, 0x03cf, 0, 0, read8_delegate(FUNC(cirrus_gd5430_device::port_03c0_r),m_vga), write8_delegate(FUNC(cirrus_gd5430_device::port_03c0_w),m_vga));
	m_isa->install_device(0x03d0, 0x03df, 0, 0, read8_delegate(FUNC(cirrus_gd5430_device::port_03d0_r),m_vga), write8_delegate(FUNC(cirrus_gd5430_device::port_03d0_w),m_vga));

	m_isa->install_memory(0xa0000, 0xbffff, 0, 0, read8_delegate(FUNC(cirrus_gd5430_device::mem_r),m_vga), write8_delegate(FUNC(cirrus_gd5430_device::mem_w),m_vga));
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

static MACHINE_CONFIG_FRAGMENT( vga_cirrus_gd542x )
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_25_1748MHz,900,0,640,526,0,480)
	MCFG_SCREEN_UPDATE_DEVICE("vga", cirrus_gd5428_device, screen_update)

	MCFG_PALETTE_ADD("palette", 0x100)

	MCFG_DEVICE_ADD("vga", CIRRUS_GD5428, 0)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor isa16_svga_cirrus_gd542x_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( vga_cirrus_gd542x );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *isa16_svga_cirrus_gd542x_device::device_rom_region() const
{
	return ROM_NAME( clgd542x );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa16_vga_device - constructor
//-------------------------------------------------

isa16_svga_cirrus_gd542x_device::isa16_svga_cirrus_gd542x_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, ISA16_SVGA_CIRRUS_GD542X, "Generic Cirrus Logic GD542x Graphics Card (BIOS v1.20)", tag, owner, clock, "clgd542x", __FILE__),
		device_isa16_card_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
READ8_MEMBER(isa16_svga_cirrus_gd542x_device::input_port_0_r ) { return 0xff; } //return space.machine().root_device().ioport("IN0")->read(); }

void isa16_svga_cirrus_gd542x_device::device_start()
{
	set_isa_device();

	m_vga = subdevice<cirrus_gd5428_device>("vga");

	m_isa->install_rom(this, 0xc0000, 0xc7fff, 0, 0, "svga", "clgd542x");

	m_isa->install_device(0x03b0, 0x03bf, 0, 0, read8_delegate(FUNC(cirrus_gd5428_device::port_03b0_r),m_vga), write8_delegate(FUNC(cirrus_gd5428_device::port_03b0_w),m_vga));
	m_isa->install_device(0x03c0, 0x03cf, 0, 0, read8_delegate(FUNC(cirrus_gd5428_device::port_03c0_r),m_vga), write8_delegate(FUNC(cirrus_gd5428_device::port_03c0_w),m_vga));
	m_isa->install_device(0x03d0, 0x03df, 0, 0, read8_delegate(FUNC(cirrus_gd5428_device::port_03d0_r),m_vga), write8_delegate(FUNC(cirrus_gd5428_device::port_03d0_w),m_vga));

	m_isa->install_memory(0xa0000, 0xbffff, 0, 0, read8_delegate(FUNC(cirrus_gd5428_device::mem_r),m_vga), write8_delegate(FUNC(cirrus_gd5428_device::mem_w),m_vga));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa16_svga_cirrus_gd542x_device::device_reset()
{
}
