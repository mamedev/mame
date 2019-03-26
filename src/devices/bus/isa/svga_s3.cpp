// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/***************************************************************************

  ISA SVGA S3 wrapper

***************************************************************************/

#include "emu.h"
#include "svga_s3.h"

#include "screen.h"


ROM_START( s3_764 )
	ROM_REGION(0x8000,"s3_764", 0)
	ROM_DEFAULT_BIOS("9fxv330")

	ROM_SYSTEM_BIOS( 0, "9fxv330", "PCI Number Nine 9FX Vision 330 2.03.10 (S3 Trio64)" )
	ROMX_LOAD("s3_764.bin", 0x00000, 0x8000, CRC(4f10aac7) SHA1(c77b3f11cc15679121314823588887dd547cd715), ROM_BIOS(0) )
	ROM_IGNORE( 0x8000 )

	// The following are from Trio64V2/DX based cards
	ROM_SYSTEM_BIOS( 1, "trio64v2", "PCI S3 86C765 v1.03-08N (S3 Trio64V2/DX)" )
	ROMX_LOAD("pci_9503-62_s3.bin", 0x00000, 0x8000, CRC(0e9d79d8) SHA1(274b5b98cc998f2783567000cdb12b14308bc290), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS( 2, "winner1k", "PCI Elsa Winner 1000/T2D 6.01.00 (S3 Trio64V2/DX)" )
	ROMX_LOAD("pci_elsa_winner_1000-t2d_6.01.00.bin", 0x00000, 0x8000, CRC(1c9532b8) SHA1(d27d60b9a3566aa42a01ad497046af16eaa2ed87), ROM_BIOS(2) )

ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA16_SVGA_S3, isa16_svga_s3_device, "s3_764", "Number Nine 9FX Vision 330 (S3 764)")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa16_svga_s3_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update("vga", FUNC(s3_vga_device::screen_update));

	S3_VGA(config, "vga", 0).set_screen("screen");
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *isa16_svga_s3_device::device_rom_region() const
{
	return ROM_NAME( s3_764 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa16_vga_device - constructor
//-------------------------------------------------

isa16_svga_s3_device::isa16_svga_s3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA16_SVGA_S3, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this),
	m_vga(nullptr), m_8514(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
READ8_MEMBER(isa16_svga_s3_device::input_port_0_r ) { return 0xff; } //return machine().root_device().ioport("IN0")->read(); }

void isa16_svga_s3_device::device_start()
{
	set_isa_device();

	m_vga = subdevice<s3_vga_device>("vga");
	m_8514 = subdevice<ibm8514a_device>("vga:8514a");

	m_isa->install_rom(this, 0xc0000, 0xc7fff, "svga", "s3_764");

	m_isa->install_device(0x03b0, 0x03bf, read8_delegate(FUNC(s3_vga_device::port_03b0_r),m_vga), write8_delegate(FUNC(s3_vga_device::port_03b0_w),m_vga));
	m_isa->install_device(0x03c0, 0x03cf, read8_delegate(FUNC(s3_vga_device::port_03c0_r),m_vga), write8_delegate(FUNC(s3_vga_device::port_03c0_w),m_vga));
	m_isa->install_device(0x03d0, 0x03df, read8_delegate(FUNC(s3_vga_device::port_03d0_r),m_vga), write8_delegate(FUNC(s3_vga_device::port_03d0_w),m_vga));
	m_isa->install16_device(0x82e8, 0x82eb, read16_delegate(FUNC(ibm8514a_device::ibm8514_currenty_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_currenty_w),m_8514));
	m_isa->install16_device(0x86e8, 0x86eb, read16_delegate(FUNC(ibm8514a_device::ibm8514_currentx_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_currentx_w),m_8514));
	m_isa->install16_device(0x8ae8, 0x8aeb, read16_delegate(FUNC(ibm8514a_device::ibm8514_desty_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_desty_w),m_8514));
	m_isa->install16_device(0x8ee8, 0x8eeb, read16_delegate(FUNC(ibm8514a_device::ibm8514_destx_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_destx_w),m_8514));
	m_isa->install16_device(0x92e8, 0x92eb, read16_delegate(FUNC(ibm8514a_device::ibm8514_line_error_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_line_error_w),m_8514));
	m_isa->install16_device(0x96e8, 0x96eb, read16_delegate(FUNC(ibm8514a_device::ibm8514_width_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_width_w),m_8514));
	m_isa->install16_device(0x9ae8, 0x9aeb, read16_delegate(FUNC(ibm8514a_device::ibm8514_gpstatus_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_cmd_w),m_8514));
	m_isa->install16_device(0x9ee8, 0x9eeb, read16_delegate(FUNC(ibm8514a_device::ibm8514_ssv_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_ssv_w),m_8514));
	m_isa->install16_device(0xa2e8, 0xa2eb, read16_delegate(FUNC(ibm8514a_device::ibm8514_bgcolour_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_bgcolour_w),m_8514));
	m_isa->install16_device(0xa6e8, 0xa6eb, read16_delegate(FUNC(ibm8514a_device::ibm8514_fgcolour_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_fgcolour_w),m_8514));
	m_isa->install16_device(0xaae8, 0xaaeb, read16_delegate(FUNC(ibm8514a_device::ibm8514_write_mask_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_write_mask_w),m_8514));
	m_isa->install16_device(0xaee8, 0xaeeb, read16_delegate(FUNC(ibm8514a_device::ibm8514_read_mask_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_read_mask_w),m_8514));
	m_isa->install16_device(0xb6e8, 0xb6eb, read16_delegate(FUNC(ibm8514a_device::ibm8514_backmix_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_backmix_w),m_8514));
	m_isa->install16_device(0xbae8, 0xbaeb, read16_delegate(FUNC(ibm8514a_device::ibm8514_foremix_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_foremix_w),m_8514));
	m_isa->install16_device(0xbee8, 0xbeeb, read16_delegate(FUNC(ibm8514a_device::ibm8514_multifunc_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_multifunc_w),m_8514));
	m_isa->install16_device(0xe2e8, 0xe2eb, read16_delegate(FUNC(ibm8514a_device::ibm8514_pixel_xfer_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_pixel_xfer_w),m_8514));

	m_isa->install_memory(0xa0000, 0xbffff, read8_delegate(FUNC(s3_vga_device::mem_r),m_vga), write8_delegate(FUNC(s3_vga_device::mem_w),m_vga));
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
	ROMX_LOAD("pci_m-v_virge-4s3.bin", 0x00000, 0x8000, CRC(d0a0f1de) SHA1(b7b41081974762a199610219bdeab149b7c7143d), ROM_BIOS(0) )

	ROM_SYSTEM_BIOS( 1, "virgeo", "PCI S3 ViRGE v1.00-05" )
	ROMX_LOAD("s3virge.bin", 0x00000, 0x8000, CRC(a7983a85) SHA1(e885371816d3237f7badd57ccd602cd863c9c9f8), ROM_BIOS(1) )
	ROM_IGNORE( 0x8000 )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA16_S3VIRGE, isa16_s3virge_device, "s3virge", "S3 ViRGE Graphics Card")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa16_s3virge_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update("vga", FUNC(s3virge_vga_device::screen_update));

	S3VIRGE(config, "vga", 0).set_screen("screen");
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *isa16_s3virge_device::device_rom_region() const
{
	return ROM_NAME( s3virge );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa16_vga_device - constructor
//-------------------------------------------------

isa16_s3virge_device::isa16_s3virge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA16_S3VIRGE, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this),
	m_vga(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
READ8_MEMBER(isa16_s3virge_device::input_port_0_r ) { return 0xff; } //return machine().root_device().ioport("IN0")->read(); }

void isa16_s3virge_device::device_start()
{
	set_isa_device();

	m_vga = subdevice<s3virge_vga_device>("vga");

	m_isa->install_rom(this, 0xc0000, 0xc7fff, "svga", "s3virge");

	m_isa->install_device(0x03b0, 0x03bf, read8_delegate(FUNC(s3virge_vga_device::port_03b0_r),m_vga), write8_delegate(FUNC(s3virge_vga_device::port_03b0_w),m_vga));
	m_isa->install_device(0x03c0, 0x03cf, read8_delegate(FUNC(s3virge_vga_device::port_03c0_r),m_vga), write8_delegate(FUNC(s3virge_vga_device::port_03c0_w),m_vga));
	m_isa->install_device(0x03d0, 0x03df, read8_delegate(FUNC(s3virge_vga_device::port_03d0_r),m_vga), write8_delegate(FUNC(s3virge_vga_device::port_03d0_w),m_vga));

	m_isa->install_memory(0xa0000, 0xbffff, read8_delegate(FUNC(s3virge_vga_device::mem_r),m_vga), write8_delegate(FUNC(s3virge_vga_device::mem_w),m_vga));
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

ROM_START( s3virgedx )
	ROM_REGION(0x8000,"s3virgedx", 0)
	ROM_LOAD("s3virgedx.bin", 0x00000, 0x8000, CRC(0da83bd3) SHA1(228a2d644e1732cb5a2eb1291608c7050cf39229) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA16_S3VIRGEDX, isa16_s3virgedx_device, "s3virgedx", "S3 ViRGE/DX Graphics Card")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa16_s3virgedx_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update("vga", FUNC(s3virgedx_vga_device::screen_update));

	S3VIRGEDX(config, "vga", 0).set_screen("screen");
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *isa16_s3virgedx_device::device_rom_region() const
{
	return ROM_NAME( s3virgedx );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa16_vga_device - constructor
//-------------------------------------------------

isa16_s3virgedx_device::isa16_s3virgedx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA16_S3VIRGEDX, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this),
	m_vga(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
READ8_MEMBER(isa16_s3virgedx_device::input_port_0_r ) { return 0xff; } //return machine().root_device().ioport("IN0")->read(); }

void isa16_s3virgedx_device::device_start()
{
	set_isa_device();

	m_vga = subdevice<s3virgedx_vga_device>("vga");

	m_isa->install_rom(this, 0xc0000, 0xc7fff, "svga", "s3virgedx");

	m_isa->install_device(0x03b0, 0x03bf, read8_delegate(FUNC(s3virge_vga_device::port_03b0_r),m_vga), write8_delegate(FUNC(s3virge_vga_device::port_03b0_w),m_vga));
	m_isa->install_device(0x03c0, 0x03cf, read8_delegate(FUNC(s3virge_vga_device::port_03c0_r),m_vga), write8_delegate(FUNC(s3virge_vga_device::port_03c0_w),m_vga));
	m_isa->install_device(0x03d0, 0x03df, read8_delegate(FUNC(s3virge_vga_device::port_03d0_r),m_vga), write8_delegate(FUNC(s3virge_vga_device::port_03d0_w),m_vga));

	m_isa->install_memory(0xa0000, 0xbffff, read8_delegate(FUNC(s3virge_vga_device::mem_r),m_vga), write8_delegate(FUNC(s3virge_vga_device::mem_w),m_vga));
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

ROM_START( stealth3d2kpro )
	ROM_REGION(0x8000,"stealth3d", 0)
	ROM_LOAD("virgedxdiamond.bin", 0x00000, 0x8000, CRC(58b0dcda) SHA1(b13ae6b04db6fc05a76d924ddf2efe150b823029) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA16_DMS3D2KPRO, isa16_stealth3d2kpro_device, "dms3d2kp", "Diamond Stealth 3D 2000 Pro")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa16_stealth3d2kpro_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update("vga", FUNC(s3virgedx_rev1_vga_device::screen_update));

	S3VIRGEDX1(config, "vga", 0).set_screen("screen");
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *isa16_stealth3d2kpro_device::device_rom_region() const
{
	return ROM_NAME( stealth3d2kpro );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa16_vga_device - constructor
//-------------------------------------------------

isa16_stealth3d2kpro_device::isa16_stealth3d2kpro_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA16_DMS3D2KPRO, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this),
	m_vga(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
READ8_MEMBER(isa16_stealth3d2kpro_device::input_port_0_r ) { return 0xff; } //return machine().root_device().ioport("IN0")->read(); }

void isa16_stealth3d2kpro_device::device_start()
{
	set_isa_device();

	m_vga = subdevice<s3virgedx_vga_device>("vga");

	m_isa->install_rom(this, 0xc0000, 0xc7fff, "svga", "stealth3d");

	m_isa->install_device(0x03b0, 0x03bf, read8_delegate(FUNC(s3virge_vga_device::port_03b0_r),m_vga), write8_delegate(FUNC(s3virge_vga_device::port_03b0_w),m_vga));
	m_isa->install_device(0x03c0, 0x03cf, read8_delegate(FUNC(s3virge_vga_device::port_03c0_r),m_vga), write8_delegate(FUNC(s3virge_vga_device::port_03c0_w),m_vga));
	m_isa->install_device(0x03d0, 0x03df, read8_delegate(FUNC(s3virge_vga_device::port_03d0_r),m_vga), write8_delegate(FUNC(s3virge_vga_device::port_03d0_w),m_vga));

	m_isa->install_memory(0xa0000, 0xbffff, read8_delegate(FUNC(s3virge_vga_device::mem_r),m_vga), write8_delegate(FUNC(s3virge_vga_device::mem_w),m_vga));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa16_stealth3d2kpro_device::device_reset()
{
}
