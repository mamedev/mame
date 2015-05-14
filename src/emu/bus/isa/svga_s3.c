// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/***************************************************************************

  ISA SVGA S3 wrapper

***************************************************************************/

#include "emu.h"
#include "svga_s3.h"

ROM_START( s3_764 )
	ROM_REGION(0x8000,"s3_764", 0)
	ROM_DEFAULT_BIOS("9fxv330")

	ROM_SYSTEM_BIOS( 0, "9fxv330", "PCI Number Nine 9FX Vision 330 2.03.10 (S3 Trio64)" )
	ROMX_LOAD("s3_764.bin", 0x00000, 0x8000, CRC(4f10aac7) SHA1(c77b3f11cc15679121314823588887dd547cd715), ROM_BIOS(1) )
	ROM_IGNORE( 0x8000 )

	// The following are from Trio64V2/DX based cards
	ROM_SYSTEM_BIOS( 1, "trio64v2", "PCI S3 86C765 v1.03-08N (S3 Trio64V2/DX)" )
	ROMX_LOAD("pci_9503-62_s3.bin", 0x00000, 0x8000, CRC(0e9d79d8) SHA1(274b5b98cc998f2783567000cdb12b14308bc290), ROM_BIOS(2) )

	ROM_SYSTEM_BIOS( 2, "winner1k", "PCI Elsa Winner 1000/T2D 6.01.00 (S3 Trio64V2/DX)" )
	ROMX_LOAD("pci_elsa_winner_1000-t2d_6.01.00.bin", 0x00000, 0x8000, CRC(1c9532b8) SHA1(d27d60b9a3566aa42a01ad497046af16eaa2ed87), ROM_BIOS(3) )

ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA16_SVGA_S3 = &device_creator<isa16_svga_s3_device>;


static MACHINE_CONFIG_FRAGMENT( vga_s3 )
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_25_1748MHz,900,0,640,526,0,480)
	MCFG_SCREEN_UPDATE_DEVICE("vga", s3_vga_device, screen_update)

	MCFG_PALETTE_ADD("palette", 0x100)

	MCFG_DEVICE_ADD("vga", S3_VGA, 0)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor isa16_svga_s3_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( vga_s3 );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *isa16_svga_s3_device::device_rom_region() const
{
	return ROM_NAME( s3_764 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa16_vga_device - constructor
//-------------------------------------------------

isa16_svga_s3_device::isa16_svga_s3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, ISA16_SVGA_S3, "Number Nine 9FX Vision 330 (S3 764) Graphics Card", tag, owner, clock, "s3_764", __FILE__),
		device_isa16_card_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
READ8_MEMBER(isa16_svga_s3_device::input_port_0_r ) { return 0xff; } //return space.machine().root_device().ioport("IN0")->read(); }

void isa16_svga_s3_device::device_start()
{
	set_isa_device();

	m_vga = subdevice<s3_vga_device>("vga");
	m_8514 = subdevice<ibm8514a_device>("vga:8514a");

	m_isa->install_rom(this, 0xc0000, 0xc7fff, 0, 0, "svga", "s3_764");

	m_isa->install_device(0x03b0, 0x03bf, 0, 0, read8_delegate(FUNC(s3_vga_device::port_03b0_r),m_vga), write8_delegate(FUNC(s3_vga_device::port_03b0_w),m_vga));
	m_isa->install_device(0x03c0, 0x03cf, 0, 0, read8_delegate(FUNC(s3_vga_device::port_03c0_r),m_vga), write8_delegate(FUNC(s3_vga_device::port_03c0_w),m_vga));
	m_isa->install_device(0x03d0, 0x03df, 0, 0, read8_delegate(FUNC(s3_vga_device::port_03d0_r),m_vga), write8_delegate(FUNC(s3_vga_device::port_03d0_w),m_vga));
	m_isa->install16_device(0x82e8, 0x82eb, 0, 0, read16_delegate(FUNC(ibm8514a_device::ibm8514_currenty_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_currenty_w),m_8514));
	m_isa->install16_device(0x86e8, 0x86eb, 0, 0, read16_delegate(FUNC(ibm8514a_device::ibm8514_currentx_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_currentx_w),m_8514));
	m_isa->install16_device(0x8ae8, 0x8aeb, 0, 0, read16_delegate(FUNC(ibm8514a_device::ibm8514_desty_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_desty_w),m_8514));
	m_isa->install16_device(0x8ee8, 0x8eeb, 0, 0, read16_delegate(FUNC(ibm8514a_device::ibm8514_destx_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_destx_w),m_8514));
	m_isa->install16_device(0x92e8, 0x92eb, 0, 0, read16_delegate(FUNC(ibm8514a_device::ibm8514_line_error_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_line_error_w),m_8514));
	m_isa->install16_device(0x96e8, 0x96eb, 0, 0, read16_delegate(FUNC(ibm8514a_device::ibm8514_width_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_width_w),m_8514));
	m_isa->install16_device(0x9ae8, 0x9aeb, 0, 0, read16_delegate(FUNC(ibm8514a_device::ibm8514_gpstatus_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_cmd_w),m_8514));
	m_isa->install16_device(0x9ee8, 0x9eeb, 0, 0, read16_delegate(FUNC(ibm8514a_device::ibm8514_ssv_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_ssv_w),m_8514));
	m_isa->install16_device(0xa2e8, 0xa2eb, 0, 0, read16_delegate(FUNC(ibm8514a_device::ibm8514_bgcolour_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_bgcolour_w),m_8514));
	m_isa->install16_device(0xa6e8, 0xa6eb, 0, 0, read16_delegate(FUNC(ibm8514a_device::ibm8514_fgcolour_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_fgcolour_w),m_8514));
	m_isa->install16_device(0xaae8, 0xaaeb, 0, 0, read16_delegate(FUNC(ibm8514a_device::ibm8514_write_mask_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_write_mask_w),m_8514));
	m_isa->install16_device(0xaee8, 0xaeeb, 0, 0, read16_delegate(FUNC(ibm8514a_device::ibm8514_read_mask_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_read_mask_w),m_8514));
	m_isa->install16_device(0xb6e8, 0xb6eb, 0, 0, read16_delegate(FUNC(ibm8514a_device::ibm8514_backmix_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_backmix_w),m_8514));
	m_isa->install16_device(0xbae8, 0xbaeb, 0, 0, read16_delegate(FUNC(ibm8514a_device::ibm8514_foremix_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_foremix_w),m_8514));
	m_isa->install16_device(0xbee8, 0xbeeb, 0, 0, read16_delegate(FUNC(ibm8514a_device::ibm8514_multifunc_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_multifunc_w),m_8514));
	m_isa->install16_device(0xe2e8, 0xe2eb, 0, 0, read16_delegate(FUNC(ibm8514a_device::ibm8514_pixel_xfer_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_pixel_xfer_w),m_8514));

	m_isa->install_memory(0xa0000, 0xbffff, 0, 0, read8_delegate(FUNC(s3_vga_device::mem_r),m_vga), write8_delegate(FUNC(s3_vga_device::mem_w),m_vga));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa16_svga_s3_device::device_reset()
{
}


/*
 *  S3 Virge 2D/3D accelerator
 */

ROM_START( s3virge )
	ROM_REGION(0x8000,"s3virge", 0)
	ROM_DEFAULT_BIOS("virge")

	ROM_SYSTEM_BIOS( 0, "virge", "PCI S3 ViRGE v1.00-10" )
	ROMX_LOAD("pci_m-v_virge-4s3.bin", 0x00000, 0x8000, CRC(d0a0f1de) SHA1(b7b41081974762a199610219bdeab149b7c7143d), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS( 1, "virgeo", "PCI S3 ViRGE v1.00-05" )
	ROMX_LOAD("s3virge.bin", 0x00000, 0x8000, CRC(a7983a85) SHA1(e885371816d3237f7badd57ccd602cd863c9c9f8), ROM_BIOS(2) )
	ROM_IGNORE( 0x8000 )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA16_S3VIRGE = &device_creator<isa16_s3virge_device>;


static MACHINE_CONFIG_FRAGMENT( vga_s3virge )
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_25_1748MHz,900,0,640,526,0,480)
	MCFG_SCREEN_UPDATE_DEVICE("vga", s3virge_vga_device, screen_update)

	MCFG_PALETTE_ADD("palette", 0x100)

	MCFG_DEVICE_ADD("vga", S3VIRGE, 0)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor isa16_s3virge_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( vga_s3virge );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *isa16_s3virge_device::device_rom_region() const
{
	return ROM_NAME( s3virge );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa16_vga_device - constructor
//-------------------------------------------------

isa16_s3virge_device::isa16_s3virge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, ISA16_S3VIRGE, "S3 ViRGE Graphics Card", tag, owner, clock, "s3virge", __FILE__),
		device_isa16_card_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
READ8_MEMBER(isa16_s3virge_device::input_port_0_r ) { return 0xff; } //return space.machine().root_device().ioport("IN0")->read(); }

void isa16_s3virge_device::device_start()
{
	set_isa_device();

	m_vga = subdevice<s3virge_vga_device>("vga");

	m_isa->install_rom(this, 0xc0000, 0xc7fff, 0, 0, "svga", "s3virge");

	m_isa->install_device(0x03b0, 0x03bf, 0, 0, read8_delegate(FUNC(s3virge_vga_device::port_03b0_r),m_vga), write8_delegate(FUNC(s3virge_vga_device::port_03b0_w),m_vga));
	m_isa->install_device(0x03c0, 0x03cf, 0, 0, read8_delegate(FUNC(s3virge_vga_device::port_03c0_r),m_vga), write8_delegate(FUNC(s3virge_vga_device::port_03c0_w),m_vga));
	m_isa->install_device(0x03d0, 0x03df, 0, 0, read8_delegate(FUNC(s3virge_vga_device::port_03d0_r),m_vga), write8_delegate(FUNC(s3virge_vga_device::port_03d0_w),m_vga));

	m_isa->install_memory(0xa0000, 0xbffff, 0, 0, read8_delegate(FUNC(s3virge_vga_device::mem_r),m_vga), write8_delegate(FUNC(s3virge_vga_device::mem_w),m_vga));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa16_s3virge_device::device_reset()
{
}

/*
 *  S3 ViRGE/DX
 */

static MACHINE_CONFIG_FRAGMENT( vga_s3virgedx )
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_25_1748MHz,900,0,640,526,0,480)
	MCFG_SCREEN_UPDATE_DEVICE("vga", s3virgedx_vga_device, screen_update)

	MCFG_PALETTE_ADD("palette", 0x100)

	MCFG_DEVICE_ADD("vga", S3VIRGEDX, 0)
MACHINE_CONFIG_END

ROM_START( s3virgedx )
	ROM_REGION(0x8000,"s3virgedx", 0)
	ROM_LOAD("s3virgedx.bin", 0x00000, 0x8000, CRC(0da83bd3) SHA1(228a2d644e1732cb5a2eb1291608c7050cf39229) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA16_S3VIRGEDX = &device_creator<isa16_s3virgedx_device>;


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor isa16_s3virgedx_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( vga_s3virgedx );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *isa16_s3virgedx_device::device_rom_region() const
{
	return ROM_NAME( s3virgedx );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa16_vga_device - constructor
//-------------------------------------------------

isa16_s3virgedx_device::isa16_s3virgedx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, ISA16_S3VIRGEDX, "S3 ViRGE/DX Graphics Card", tag, owner, clock, "s3virgedx", __FILE__),
		device_isa16_card_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
READ8_MEMBER(isa16_s3virgedx_device::input_port_0_r ) { return 0xff; } //return space.machine().root_device().ioport("IN0")->read(); }

void isa16_s3virgedx_device::device_start()
{
	set_isa_device();

	m_vga = subdevice<s3virgedx_vga_device>("vga");

	m_isa->install_rom(this, 0xc0000, 0xc7fff, 0, 0, "svga", "s3virgedx");

	m_isa->install_device(0x03b0, 0x03bf, 0, 0, read8_delegate(FUNC(s3virge_vga_device::port_03b0_r),m_vga), write8_delegate(FUNC(s3virge_vga_device::port_03b0_w),m_vga));
	m_isa->install_device(0x03c0, 0x03cf, 0, 0, read8_delegate(FUNC(s3virge_vga_device::port_03c0_r),m_vga), write8_delegate(FUNC(s3virge_vga_device::port_03c0_w),m_vga));
	m_isa->install_device(0x03d0, 0x03df, 0, 0, read8_delegate(FUNC(s3virge_vga_device::port_03d0_r),m_vga), write8_delegate(FUNC(s3virge_vga_device::port_03d0_w),m_vga));

	m_isa->install_memory(0xa0000, 0xbffff, 0, 0, read8_delegate(FUNC(s3virge_vga_device::mem_r),m_vga), write8_delegate(FUNC(s3virge_vga_device::mem_w),m_vga));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa16_s3virgedx_device::device_reset()
{
}


/*
 *  Diamond Stealth 3D 2000 Pro
 */

static MACHINE_CONFIG_FRAGMENT( vga_stealth3d2kpro )
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_25_1748MHz,900,0,640,526,0,480)
	MCFG_SCREEN_UPDATE_DEVICE("vga", s3virgedx_rev1_vga_device, screen_update)

	MCFG_PALETTE_ADD("palette", 0x100)

	MCFG_DEVICE_ADD("vga", S3VIRGEDX1, 0)
MACHINE_CONFIG_END

ROM_START( stealth3d2kpro )
	ROM_REGION(0x8000,"stealth3d", 0)
	ROM_LOAD("virgedxdiamond.bin", 0x00000, 0x8000, CRC(58b0dcda) SHA1(b13ae6b04db6fc05a76d924ddf2efe150b823029) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA16_DMS3D2KPRO = &device_creator<isa16_stealth3d2kpro_device>;


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor isa16_stealth3d2kpro_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( vga_stealth3d2kpro );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *isa16_stealth3d2kpro_device::device_rom_region() const
{
	return ROM_NAME( stealth3d2kpro );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa16_vga_device - constructor
//-------------------------------------------------

isa16_stealth3d2kpro_device::isa16_stealth3d2kpro_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, ISA16_DMS3D2KPRO, "Diamond Stealth 3D 2000 Pro", tag, owner, clock, "dms3d2kp", __FILE__),
		device_isa16_card_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
READ8_MEMBER(isa16_stealth3d2kpro_device::input_port_0_r ) { return 0xff; } //return space.machine().root_device().ioport("IN0")->read(); }

void isa16_stealth3d2kpro_device::device_start()
{
	set_isa_device();

	m_vga = subdevice<s3virgedx_vga_device>("vga");

	m_isa->install_rom(this, 0xc0000, 0xc7fff, 0, 0, "svga", "stealth3d");

	m_isa->install_device(0x03b0, 0x03bf, 0, 0, read8_delegate(FUNC(s3virge_vga_device::port_03b0_r),m_vga), write8_delegate(FUNC(s3virge_vga_device::port_03b0_w),m_vga));
	m_isa->install_device(0x03c0, 0x03cf, 0, 0, read8_delegate(FUNC(s3virge_vga_device::port_03c0_r),m_vga), write8_delegate(FUNC(s3virge_vga_device::port_03c0_w),m_vga));
	m_isa->install_device(0x03d0, 0x03df, 0, 0, read8_delegate(FUNC(s3virge_vga_device::port_03d0_r),m_vga), write8_delegate(FUNC(s3virge_vga_device::port_03d0_w),m_vga));

	m_isa->install_memory(0xa0000, 0xbffff, 0, 0, read8_delegate(FUNC(s3virge_vga_device::mem_r),m_vga), write8_delegate(FUNC(s3virge_vga_device::mem_w),m_vga));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa16_stealth3d2kpro_device::device_reset()
{
}
