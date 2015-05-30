// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * isa_vga_ati.c
 *
 *  ATi Graphics Ultra ISA Video card
 *   - Uses ATi 28800-6 (VGA Wonder) and ATi 38800-1 (Mach8, 8514/A clone)
 *
 *  ATi Graphics Ultra Pro ISA Video card
 *   - ATi 68800-3 (Mach32, combined VGA and 8514/A)
 *
 *  ATi mach64 ISA Video card
 *   - ATi 88800 (Mach64, combined VGA and 8514/A)
 *
 *  Created on: 9/09/2012
 */

#include "emu.h"
#include "vga_ati.h"
#include "mach32.h"
#include "video/pc_vga.h"

ROM_START( gfxultra )
	ROM_REGION(0x8000,"gfxultra", 0)
	ROM_LOAD("113-11504-002.bin", 0x00000, 0x8000, CRC(f498b36a) SHA1(117cfc972ce4645538ba7262222d8ff38bc2c58c) )
	ROM_IGNORE( 0x8000 )
ROM_END

ROM_START( gfxultrp )
	ROM_REGION(0x8000,"gfxultrapro", 0)

	ROM_DEFAULT_BIOS("isa")

	ROM_SYSTEM_BIOS( 0, "isa", "ISA BIOS 112-18900-100" )
	ROMX_LOAD("gfxultrapro.bin", 0x00000, 0x8000, CRC(4e5effd7) SHA1(84ad3abf7653e4734bf39f5d5c8b88e74527e8ce), ROM_BIOS(1) )

	// We can separate out this BIOS once a proper VLB bus emulation is available
	ROM_SYSTEM_BIOS( 1, "vlb", "VLB BIOS 113-19500-100" )
	ROMX_LOAD("gfxultrapro_vlb.bin", 0x00000, 0x8000, CRC(5018f71e) SHA1(61321dfecf1bcdd8043836fabbe41786dbf3001b), ROM_BIOS(2) )
ROM_END

ROM_START( mach64 )
	ROM_REGION(0x8000,"mach64", 0)

	ROM_SYSTEM_BIOS( 0, "isa", "ISA BIOS 112-28122-101" )
	ROMX_LOAD("mach64.bin", 0x00000, 0x8000, CRC(1300aa8f) SHA1(dfc7f817900f125b89b0bda16fcb205f066a47fc), ROM_BIOS(1) )

	// We can separate out these BIOSes once a proper PCI and VLB bus emulation is available
	ROM_SYSTEM_BIOS( 1, "vlb_d", "VLB DRAM BIOS 113-27803-102" )
	ROMX_LOAD("mach64_vlb_dram.bin", 0x00000, 0x8000, CRC(f2a24699) SHA1(580401a8bdfc379180a8d7d77305fc529b2a8374), ROM_BIOS(2) )

	ROM_SYSTEM_BIOS( 2, "gfxultrapt_vlb", "ATi Graphics Pro Turbo VLB VRAM BIOS 113-26900-103" )
	ROMX_LOAD("mach64_vlb_vram.bin", 0x00000, 0x8000, CRC(47779d8f) SHA1(87b01b7a16d9c79dfc6c5aa8a39455c725d2e455), ROM_BIOS(3) )

	ROM_SYSTEM_BIOS( 3, "pci", "PCI BIOS 113-25420-100" )
	ROMX_LOAD("pci_mach64__113-25420-100-1995.27c256.u1.bin", 0x00000, 0x8000, CRC(762596e8) SHA1(9544b073ac182ec2990e18f54afbb96d52db744a), ROM_BIOS(4) )

	ROM_SYSTEM_BIOS( 4, "pci_v1", "PCI mach64 V1 BIOS 113-34404-104" )
	ROMX_LOAD("pci_mach64_v1_113-34404-104_1996.bin", 0x00000, 0x8000, CRC(c6a39c3f) SHA1(0f4cf9221179c675dafafde638bc00244b6feb63), ROM_BIOS(5) )
	ROM_IGNORE(0x8000)

ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA16_VGA_GFXULTRA = &device_creator<isa16_vga_gfxultra_device>;
const device_type ISA16_SVGA_GFXULTRAPRO = &device_creator<isa16_vga_gfxultrapro_device>;
const device_type ISA16_SVGA_MACH64 = &device_creator<isa16_vga_mach64_device>;

static MACHINE_CONFIG_FRAGMENT( vga_ati )
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_25_1748MHz,900,0,640,526,0,480)
	MCFG_SCREEN_UPDATE_DEVICE("vga", ati_vga_device, screen_update)

	MCFG_PALETTE_ADD("palette", 0x100)

	MCFG_DEVICE_ADD("vga", ATI_VGA, 0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( vga_mach32 )
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_25_1748MHz,900,0,640,526,0,480)
	MCFG_SCREEN_UPDATE_DEVICE("vga", mach32_device, screen_update)

	MCFG_PALETTE_ADD("palette", 0x100)

	MCFG_DEVICE_ADD("vga", ATIMACH32, 0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( vga_mach64 )
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_25_1748MHz,900,0,640,526,0,480)
	MCFG_SCREEN_UPDATE_DEVICE("vga", mach64_device, screen_update)

	MCFG_PALETTE_ADD("palette", 0x100)

	MCFG_DEVICE_ADD("vga", ATIMACH64, 0)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor isa16_vga_gfxultra_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( vga_ati );
}

machine_config_constructor isa16_vga_gfxultrapro_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( vga_mach32 );
}

machine_config_constructor isa16_vga_mach64_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( vga_mach64 );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *isa16_vga_gfxultra_device::device_rom_region() const
{
	return ROM_NAME( gfxultra );
}

const rom_entry *isa16_vga_gfxultrapro_device::device_rom_region() const
{
	return ROM_NAME( gfxultrp );
}

const rom_entry *isa16_vga_mach64_device::device_rom_region() const
{
	return ROM_NAME( mach64 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_vga_device - constructor
//-------------------------------------------------

isa16_vga_gfxultra_device::isa16_vga_gfxultra_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, ISA16_VGA_GFXULTRA, "ATi Graphics Ultra Card", tag, owner, clock, "gfxultra", __FILE__),
		device_isa16_card_interface(mconfig, *this)
{
}

isa16_vga_gfxultrapro_device::isa16_vga_gfxultrapro_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, ISA16_SVGA_GFXULTRAPRO, "ATi Graphics Ultra Pro Card", tag, owner, clock, "gfxultrp", __FILE__),
		device_isa16_card_interface(mconfig, *this)
{
}

isa16_vga_mach64_device::isa16_vga_mach64_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, ISA16_SVGA_MACH64, "ATi mach64 Card", tag, owner, clock, "mach64", __FILE__),
		device_isa16_card_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
READ8_MEMBER(isa16_vga_gfxultra_device::input_port_0_r ) { return 0xff; } //return space.machine().root_device().ioport("IN0")->read(); }
READ8_MEMBER(isa16_vga_gfxultrapro_device::input_port_0_r ) { return 0xff; } //return space.machine().root_device().ioport("IN0")->read(); }
READ8_MEMBER(isa16_vga_mach64_device::input_port_0_r ) { return 0xff; } //return space.machine().root_device().ioport("IN0")->read(); }

void isa16_vga_gfxultra_device::device_start()
{
	set_isa_device();

	m_vga = subdevice<ati_vga_device>("vga");
	m_8514 = subdevice<mach8_device>("vga:8514a");

	m_isa->install_rom(this, 0xc0000, 0xc7fff, 0, 0, "vga", "gfxultra");

	m_isa->install_device(0x1ce, 0x1cf, 0, 0, read8_delegate(FUNC(ati_vga_device::ati_port_ext_r),m_vga), write8_delegate(FUNC(ati_vga_device::ati_port_ext_w),m_vga));
	m_isa->install16_device(0x2e8, 0x2eb, 0, 0, read16_delegate(FUNC(mach8_device::ibm8514_status_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_htotal_w),m_8514));
	m_isa->install_device(0x3b0, 0x3bf, 0, 0, read8_delegate(FUNC(ati_vga_device::port_03b0_r),m_vga), write8_delegate(FUNC(vga_device::port_03b0_w),m_vga));
	m_isa->install_device(0x3c0, 0x3cf, 0, 0, read8_delegate(FUNC(ati_vga_device::port_03c0_r),m_vga), write8_delegate(FUNC(vga_device::port_03c0_w),m_vga));
	m_isa->install_device(0x3d0, 0x3df, 0, 0, read8_delegate(FUNC(ati_vga_device::port_03d0_r),m_vga), write8_delegate(FUNC(vga_device::port_03d0_w),m_vga));
	m_isa->install16_device(0x12e8, 0x12eb, 0, 0, read16_delegate(FUNC(mach8_device::ibm8514_vtotal_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_vtotal_w),m_8514));
	m_isa->install16_device(0x12ec, 0x12ef, 0, 0, read16_delegate(FUNC(mach8_device::mach8_config1_r),m_8514), write16_delegate());
	m_isa->install16_device(0x16e8, 0x16eb, 0, 0, read16_delegate(FUNC(mach8_device::ibm8514_vdisp_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_vdisp_w),m_8514));
	m_isa->install16_device(0x16ec, 0x16ef, 0, 0, read16_delegate(FUNC(mach8_device::mach8_config2_r),m_8514), write16_delegate());
	m_isa->install16_device(0x1ae8, 0x1aeb, 0, 0, read16_delegate(FUNC(mach8_device::ibm8514_vsync_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_vsync_w),m_8514));
	m_isa->install16_device(0x26e8, 0x26eb, 0, 0, read16_delegate(FUNC(mach8_device::ibm8514_htotal_r),m_8514),write16_delegate());
	m_isa->install16_device(0x2ee8, 0x2eeb, 0, 0, read16_delegate(FUNC(mach8_device::ibm8514_subcontrol_r),m_8514),write16_delegate());
	m_isa->install16_device(0x42e8, 0x42eb, 0, 0, read16_delegate(FUNC(mach8_device::ibm8514_substatus_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_subcontrol_w),m_8514));
	m_isa->install16_device(0x52e8, 0x52eb, 0, 0, read16_delegate(FUNC(mach8_device::mach8_ec0_r),m_8514), write16_delegate(FUNC(mach8_device::mach8_ec0_w),m_8514));
	m_isa->install16_device(0x52ec, 0x52ef, 0, 0, read16_delegate(FUNC(mach8_device::mach8_scratch0_r),m_8514), write16_delegate(FUNC(mach8_device::mach8_scratch0_w),m_8514));
	m_isa->install16_device(0x56e8, 0x56eb, 0, 0, read16_delegate(FUNC(mach8_device::mach8_ec1_r),m_8514), write16_delegate(FUNC(mach8_device::mach8_ec1_w),m_8514));
	m_isa->install16_device(0x56ec, 0x56ef, 0, 0, read16_delegate(FUNC(mach8_device::mach8_scratch0_r),m_8514), write16_delegate(FUNC(mach8_device::mach8_scratch0_w),m_8514));
	m_isa->install16_device(0x5ae8, 0x5aeb, 0, 0, read16_delegate(FUNC(mach8_device::mach8_ec2_r),m_8514), write16_delegate(FUNC(mach8_device::mach8_ec2_w),m_8514));
	m_isa->install16_device(0x5ee8, 0x5eeb, 0, 0, read16_delegate(FUNC(mach8_device::mach8_ec3_r),m_8514), write16_delegate(FUNC(mach8_device::mach8_ec3_w),m_8514));
	m_isa->install16_device(0x82e8, 0x82eb, 0, 0, read16_delegate(FUNC(mach8_device::ibm8514_currenty_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_currenty_w),m_8514));
	m_isa->install16_device(0x86e8, 0x86eb, 0, 0, read16_delegate(FUNC(mach8_device::ibm8514_currentx_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_currentx_w),m_8514));
	m_isa->install16_device(0x8ae8, 0x8aeb, 0, 0, read16_delegate(FUNC(mach8_device::ibm8514_desty_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_desty_w),m_8514));
	m_isa->install16_device(0x8ee8, 0x8eeb, 0, 0, read16_delegate(FUNC(mach8_device::ibm8514_destx_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_destx_w),m_8514));
	m_isa->install16_device(0x92e8, 0x92eb, 0, 0, read16_delegate(FUNC(mach8_device::ibm8514_line_error_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_line_error_w),m_8514));
	m_isa->install16_device(0x96e8, 0x96eb, 0, 0, read16_delegate(FUNC(mach8_device::ibm8514_width_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_width_w),m_8514));
	m_isa->install16_device(0x96ec, 0x96ef, 0, 0, read16_delegate(FUNC(mach8_device::mach8_bresenham_count_r),m_8514), write16_delegate(FUNC(mach8_device::mach8_bresenham_count_w),m_8514));
	m_isa->install16_device(0x9ae8, 0x9aeb, 0, 0, read16_delegate(FUNC(mach8_device::ibm8514_gpstatus_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_cmd_w),m_8514));
	m_isa->install16_device(0x9aec, 0x9aef, 0, 0, read16_delegate(FUNC(mach8_device::mach8_ext_fifo_r),m_8514), write16_delegate(FUNC(mach8_device::mach8_linedraw_index_w),m_8514));
	m_isa->install16_device(0x9ee8, 0x9eeb, 0, 0, read16_delegate(FUNC(mach8_device::ibm8514_ssv_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_ssv_w),m_8514));
	m_isa->install16_device(0xa2e8, 0xa2eb, 0, 0, read16_delegate(FUNC(mach8_device::ibm8514_bgcolour_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_bgcolour_w),m_8514));
	m_isa->install16_device(0xa6e8, 0xa6eb, 0, 0, read16_delegate(FUNC(mach8_device::ibm8514_fgcolour_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_fgcolour_w),m_8514));
	m_isa->install16_device(0xaae8, 0xaaeb, 0, 0, read16_delegate(FUNC(ibm8514a_device::ibm8514_write_mask_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_write_mask_w),m_8514));
	m_isa->install16_device(0xaee8, 0xaeeb, 0, 0, read16_delegate(FUNC(ibm8514a_device::ibm8514_read_mask_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_read_mask_w),m_8514));
	m_isa->install16_device(0xb6e8, 0xb6eb, 0, 0, read16_delegate(FUNC(mach8_device::ibm8514_backmix_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_backmix_w),m_8514));
	m_isa->install16_device(0xbae8, 0xbaeb, 0, 0, read16_delegate(FUNC(mach8_device::ibm8514_foremix_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_foremix_w),m_8514));
	m_isa->install16_device(0xbee8, 0xbeeb, 0, 0, read16_delegate(FUNC(mach8_device::ibm8514_multifunc_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_multifunc_w),m_8514));
	m_isa->install16_device(0xe2e8, 0xe2eb, 0, 0, read16_delegate(FUNC(mach8_device::ibm8514_pixel_xfer_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_pixel_xfer_w),m_8514));
	m_isa->install16_device(0xdaec, 0xdaef, 0, 0, read16_delegate(FUNC(mach8_device::mach8_sourcex_r),m_8514), write16_delegate(FUNC(mach8_device::mach8_ext_leftscissor_w),m_8514));
	m_isa->install16_device(0xdeec, 0xdeef, 0, 0, read16_delegate(FUNC(mach8_device::mach8_sourcey_r),m_8514), write16_delegate(FUNC(mach8_device::mach8_ext_topscissor_w),m_8514));
	m_isa->install16_device(0xfeec, 0xfeef, 0, 0, read16_delegate(FUNC(mach8_device::mach8_linedraw_r),m_8514), write16_delegate(FUNC(mach8_device::mach8_linedraw_w),m_8514));

	m_isa->install_memory(0xa0000, 0xbffff, 0, 0, read8_delegate(FUNC(ati_vga_device::mem_r),m_vga), write8_delegate(FUNC(ati_vga_device::mem_w),m_vga));
}

void isa16_vga_gfxultrapro_device::device_start()
{
	set_isa_device();

	m_vga = subdevice<mach32_device>("vga");

	m_isa->install_rom(this, 0xc0000, 0xc7fff, 0, 0, "vga", "gfxultrapro");

	m_isa->install_device(0x1ce, 0x1cf, 0, 0, read8_delegate(FUNC(mach32_device::ati_port_ext_r),m_vga), write8_delegate(FUNC(mach32_device::ati_port_ext_w),m_vga));
	m_isa->install16_device(0x2e8, 0x2eb, 0, 0, read16_delegate(FUNC(mach32_device::mach32_status_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_htotal_w),m_vga));
	m_isa->install_device(0x3b0, 0x3bf, 0, 0, read8_delegate(FUNC(mach32_device::port_03b0_r),m_vga), write8_delegate(FUNC(mach32_device::port_03b0_w),m_vga));
	m_isa->install_device(0x3c0, 0x3cf, 0, 0, read8_delegate(FUNC(mach32_device::port_03c0_r),m_vga), write8_delegate(FUNC(mach32_device::port_03c0_w),m_vga));
	m_isa->install_device(0x3d0, 0x3df, 0, 0, read8_delegate(FUNC(mach32_device::port_03d0_r),m_vga), write8_delegate(FUNC(mach32_device::port_03d0_w),m_vga));
	m_isa->install16_device(0x12e8, 0x12eb, 0, 0, read16_delegate(FUNC(mach32_device::ibm8514_vtotal_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_vtotal_w),m_vga));
	m_isa->install16_device(0x12ec, 0x12ef, 0, 0, read16_delegate(FUNC(mach32_device::mach8_config1_r),m_vga), write16_delegate());
	m_isa->install16_device(0x16e8, 0x16eb, 0, 0, read16_delegate(FUNC(mach32_device::ibm8514_vdisp_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_vdisp_w),m_vga));
	m_isa->install16_device(0x16ec, 0x16ef, 0, 0, read16_delegate(FUNC(mach32_device::mach8_config2_r),m_vga), write16_delegate());
	m_isa->install16_device(0x1ae8, 0x1aeb, 0, 0, read16_delegate(FUNC(mach32_device::ibm8514_vsync_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_vsync_w),m_vga));
	m_isa->install16_device(0x26e8, 0x26eb, 0, 0, read16_delegate(FUNC(mach32_device::ibm8514_htotal_r),m_vga),write16_delegate());
	m_isa->install16_device(0x2ee8, 0x2eeb, 0, 0, read16_delegate(FUNC(mach32_device::ibm8514_subcontrol_r),m_vga),write16_delegate());
	m_isa->install16_device(0x42e8, 0x42eb, 0, 0, read16_delegate(FUNC(mach32_device::ibm8514_substatus_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_subcontrol_w),m_vga));
	m_isa->install16_device(0x42ec, 0x42ef, 0, 0, read16_delegate(FUNC(mach32_device::mach32_mem_boundary_r),m_vga), write16_delegate(FUNC(mach32_device::mach32_mem_boundary_w),m_vga));
	m_isa->install16_device(0x4aec, 0x4aef, 0, 0, read16_delegate(FUNC(mach32_device::mach8_clksel_r),m_vga), write16_delegate(FUNC(mach32_device::mach32_clksel_w),m_vga));
	m_isa->install16_device(0x52e8, 0x52eb, 0, 0, read16_delegate(FUNC(mach32_device::mach8_ec0_r),m_vga), write16_delegate(FUNC(mach32_device::mach8_ec0_w),m_vga));
	m_isa->install16_device(0x52ec, 0x52ef, 0, 0, read16_delegate(FUNC(mach32_device::mach8_scratch0_r),m_vga), write16_delegate(FUNC(mach32_device::mach8_scratch0_w),m_vga));
	m_isa->install16_device(0x56e8, 0x56eb, 0, 0, read16_delegate(FUNC(mach32_device::mach8_ec1_r),m_vga), write16_delegate(FUNC(mach32_device::mach8_ec1_w),m_vga));
	m_isa->install16_device(0x56ec, 0x56ef, 0, 0, read16_delegate(FUNC(mach32_device::mach8_scratch0_r),m_vga), write16_delegate(FUNC(mach32_device::mach8_scratch0_w),m_vga));
	m_isa->install16_device(0x5ae8, 0x5aeb, 0, 0, read16_delegate(FUNC(mach32_device::mach8_ec2_r),m_vga), write16_delegate(FUNC(mach32_device::mach8_ec2_w),m_vga));
	m_isa->install16_device(0x5ee8, 0x5eeb, 0, 0, read16_delegate(FUNC(mach32_device::mach8_ec3_r),m_vga), write16_delegate(FUNC(mach32_device::mach8_ec3_w),m_vga));
	m_isa->install16_device(0x82e8, 0x82eb, 0, 0, read16_delegate(FUNC(mach32_device::ibm8514_currenty_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_currenty_w),m_vga));
	m_isa->install16_device(0x86e8, 0x86eb, 0, 0, read16_delegate(FUNC(mach32_device::ibm8514_currentx_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_currentx_w),m_vga));
	m_isa->install16_device(0x8ae8, 0x8aeb, 0, 0, read16_delegate(FUNC(mach32_device::ibm8514_desty_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_desty_w),m_vga));
	m_isa->install16_device(0x8ee8, 0x8eeb, 0, 0, read16_delegate(FUNC(mach32_device::ibm8514_destx_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_destx_w),m_vga));
	m_isa->install16_device(0x92e8, 0x92eb, 0, 0, read16_delegate(FUNC(mach32_device::ibm8514_line_error_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_line_error_w),m_vga));
	m_isa->install16_device(0x96e8, 0x96eb, 0, 0, read16_delegate(FUNC(mach32_device::ibm8514_width_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_width_w),m_vga));
	m_isa->install16_device(0x96ec, 0x96ef, 0, 0, read16_delegate(FUNC(mach32_device::mach8_bresenham_count_r),m_vga), write16_delegate(FUNC(mach32_device::mach8_bresenham_count_w),m_vga));
	m_isa->install16_device(0x9ae8, 0x9aeb, 0, 0, read16_delegate(FUNC(mach32_device::ibm8514_gpstatus_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_cmd_w),m_vga));
	m_isa->install16_device(0x9aec, 0x9aef, 0, 0, read16_delegate(FUNC(mach32_device::mach8_ext_fifo_r),m_vga), write16_delegate(FUNC(mach32_device::mach8_linedraw_index_w),m_vga));
	m_isa->install16_device(0x9ee8, 0x9eeb, 0, 0, read16_delegate(FUNC(mach32_device::ibm8514_ssv_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_ssv_w),m_vga));
	m_isa->install16_device(0xa2e8, 0xa2eb, 0, 0, read16_delegate(FUNC(mach32_device::ibm8514_bgcolour_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_bgcolour_w),m_vga));
	m_isa->install16_device(0xa6e8, 0xa6eb, 0, 0, read16_delegate(FUNC(mach32_device::ibm8514_fgcolour_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_fgcolour_w),m_vga));
	m_isa->install16_device(0xaae8, 0xaaeb, 0, 0, read16_delegate(FUNC(mach32_device::ibm8514_write_mask_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_write_mask_w),m_vga));
	m_isa->install16_device(0xaee8, 0xaeeb, 0, 0, read16_delegate(FUNC(mach32_device::ibm8514_read_mask_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_read_mask_w),m_vga));
	m_isa->install16_device(0xb6e8, 0xb6eb, 0, 0, read16_delegate(FUNC(mach32_device::ibm8514_backmix_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_backmix_w),m_vga));
	m_isa->install16_device(0xbae8, 0xbaeb, 0, 0, read16_delegate(FUNC(mach32_device::ibm8514_foremix_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_foremix_w),m_vga));
	m_isa->install16_device(0xbee8, 0xbeeb, 0, 0, read16_delegate(FUNC(mach32_device::ibm8514_multifunc_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_multifunc_w),m_vga));
	m_isa->install16_device(0xe2e8, 0xe2eb, 0, 0, read16_delegate(FUNC(mach32_device::ibm8514_pixel_xfer_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_pixel_xfer_w),m_vga));
	m_isa->install16_device(0xdaec, 0xdaef, 0, 0, read16_delegate(FUNC(mach32_device::mach8_sourcex_r),m_vga), write16_delegate(FUNC(mach32_device::mach8_ext_leftscissor_w),m_vga));
	m_isa->install16_device(0xdeec, 0xdeef, 0, 0, read16_delegate(FUNC(mach32_device::mach8_sourcey_r),m_vga), write16_delegate(FUNC(mach32_device::mach8_ext_topscissor_w),m_vga));
	m_isa->install16_device(0xfaec, 0xfaef, 0, 0, read16_delegate(FUNC(mach32_device::mach32_chipid_r),m_vga), write16_delegate());
	m_isa->install16_device(0xfeec, 0xfeef, 0, 0, read16_delegate(FUNC(mach32_device::mach8_linedraw_r),m_vga), write16_delegate(FUNC(mach32_device::mach8_linedraw_w),m_vga));

	m_isa->install_memory(0xa0000, 0xbffff, 0, 0, read8_delegate(FUNC(mach32_device::mem_r),m_vga), write8_delegate(FUNC(mach32_device::mem_w),m_vga));
}

void isa16_vga_mach64_device::device_start()
{
	set_isa_device();

	m_vga = subdevice<mach64_device>("vga");

	m_isa->install_rom(this, 0xc0000, 0xc7fff, 0, 0, "vga", "mach64");

	m_isa->install_device(0x1ce, 0x1cf, 0, 0, read8_delegate(FUNC(mach64_device::ati_port_ext_r),m_vga), write8_delegate(FUNC(mach64_device::ati_port_ext_w),m_vga));
	m_isa->install16_device(0x2e8, 0x2eb, 0, 0, read16_delegate(FUNC(mach64_device::mach32_status_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_htotal_w),m_vga));
	m_isa->install_device(0x3b0, 0x3bf, 0, 0, read8_delegate(FUNC(mach64_device::port_03b0_r),m_vga), write8_delegate(FUNC(mach64_device::port_03b0_w),m_vga));
	m_isa->install_device(0x3c0, 0x3cf, 0, 0, read8_delegate(FUNC(mach64_device::port_03c0_r),m_vga), write8_delegate(FUNC(mach64_device::port_03c0_w),m_vga));
	m_isa->install_device(0x3d0, 0x3df, 0, 0, read8_delegate(FUNC(mach64_device::port_03d0_r),m_vga), write8_delegate(FUNC(mach64_device::port_03d0_w),m_vga));
	m_isa->install16_device(0x12e8, 0x12eb, 0, 0, read16_delegate(FUNC(mach64_device::ibm8514_vtotal_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_vtotal_w),m_vga));
	m_isa->install16_device(0x12ec, 0x12ef, 0, 0, read16_delegate(FUNC(mach64_device::mach8_config1_r),m_vga), write16_delegate(FUNC(mach64_device::mach64_config1_w),m_vga));
	m_isa->install16_device(0x16e8, 0x16eb, 0, 0, read16_delegate(FUNC(mach64_device::ibm8514_vdisp_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_vdisp_w),m_vga));
	m_isa->install16_device(0x16ec, 0x16ef, 0, 0, read16_delegate(FUNC(mach64_device::mach8_config2_r),m_vga), write16_delegate(FUNC(mach64_device::mach64_config2_w),m_vga));
	m_isa->install16_device(0x1ae8, 0x1aeb, 0, 0, read16_delegate(FUNC(mach64_device::ibm8514_vsync_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_vsync_w),m_vga));
	m_isa->install16_device(0x26e8, 0x26eb, 0, 0, read16_delegate(FUNC(mach64_device::ibm8514_htotal_r),m_vga),write16_delegate());
	m_isa->install16_device(0x2ee8, 0x2eeb, 0, 0, read16_delegate(FUNC(mach64_device::ibm8514_subcontrol_r),m_vga),write16_delegate());
	m_isa->install16_device(0x42e8, 0x42eb, 0, 0, read16_delegate(FUNC(mach64_device::ibm8514_substatus_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_subcontrol_w),m_vga));
	m_isa->install16_device(0x42ec, 0x42ef, 0, 0, read16_delegate(FUNC(mach64_device::mach32_mem_boundary_r),m_vga), write16_delegate(FUNC(mach64_device::mach32_mem_boundary_w),m_vga));
	m_isa->install16_device(0x4aec, 0x4aef, 0, 0, read16_delegate(FUNC(mach64_device::mach8_clksel_r),m_vga), write16_delegate(FUNC(mach64_device::mach32_clksel_w),m_vga));
	m_isa->install16_device(0x52e8, 0x52eb, 0, 0, read16_delegate(FUNC(mach64_device::mach8_ec0_r),m_vga), write16_delegate(FUNC(mach64_device::mach8_ec0_w),m_vga));
	m_isa->install16_device(0x52ec, 0x52ef, 0, 0, read16_delegate(FUNC(mach64_device::mach8_scratch0_r),m_vga), write16_delegate(FUNC(mach64_device::mach8_scratch0_w),m_vga));
	m_isa->install16_device(0x56e8, 0x56eb, 0, 0, read16_delegate(FUNC(mach64_device::mach8_ec1_r),m_vga), write16_delegate(FUNC(mach64_device::mach8_ec1_w),m_vga));
	m_isa->install16_device(0x56ec, 0x56ef, 0, 0, read16_delegate(FUNC(mach64_device::mach8_scratch0_r),m_vga), write16_delegate(FUNC(mach64_device::mach8_scratch0_w),m_vga));
	m_isa->install16_device(0x5ae8, 0x5aeb, 0, 0, read16_delegate(FUNC(mach64_device::mach8_ec2_r),m_vga), write16_delegate(FUNC(mach64_device::mach8_ec2_w),m_vga));
	m_isa->install16_device(0x5ee8, 0x5eeb, 0, 0, read16_delegate(FUNC(mach64_device::mach8_ec3_r),m_vga), write16_delegate(FUNC(mach64_device::mach8_ec3_w),m_vga));
	m_isa->install16_device(0x82e8, 0x82eb, 0, 0, read16_delegate(FUNC(mach64_device::ibm8514_currenty_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_currenty_w),m_vga));
	m_isa->install16_device(0x86e8, 0x86eb, 0, 0, read16_delegate(FUNC(mach64_device::ibm8514_currentx_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_currentx_w),m_vga));
	m_isa->install16_device(0x8ae8, 0x8aeb, 0, 0, read16_delegate(FUNC(mach64_device::ibm8514_desty_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_desty_w),m_vga));
	m_isa->install16_device(0x8ee8, 0x8eeb, 0, 0, read16_delegate(FUNC(mach64_device::ibm8514_destx_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_destx_w),m_vga));
	m_isa->install16_device(0x92e8, 0x92eb, 0, 0, read16_delegate(FUNC(mach64_device::ibm8514_line_error_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_line_error_w),m_vga));
	m_isa->install16_device(0x96e8, 0x96eb, 0, 0, read16_delegate(FUNC(mach64_device::ibm8514_width_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_width_w),m_vga));
	m_isa->install16_device(0x96ec, 0x96ef, 0, 0, read16_delegate(FUNC(mach64_device::mach8_bresenham_count_r),m_vga), write16_delegate(FUNC(mach64_device::mach8_bresenham_count_w),m_vga));
	m_isa->install16_device(0x9ae8, 0x9aeb, 0, 0, read16_delegate(FUNC(mach64_device::ibm8514_gpstatus_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_cmd_w),m_vga));
	m_isa->install16_device(0x9aec, 0x9aef, 0, 0, read16_delegate(FUNC(mach64_device::mach8_ext_fifo_r),m_vga), write16_delegate(FUNC(mach64_device::mach8_linedraw_index_w),m_vga));
	m_isa->install16_device(0x9ee8, 0x9eeb, 0, 0, read16_delegate(FUNC(mach64_device::ibm8514_ssv_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_ssv_w),m_vga));
	m_isa->install16_device(0xa2e8, 0xa2eb, 0, 0, read16_delegate(FUNC(mach64_device::ibm8514_bgcolour_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_bgcolour_w),m_vga));
	m_isa->install16_device(0xa6e8, 0xa6eb, 0, 0, read16_delegate(FUNC(mach64_device::ibm8514_fgcolour_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_fgcolour_w),m_vga));
	m_isa->install16_device(0xaae8, 0xaaeb, 0, 0, read16_delegate(FUNC(mach64_device::ibm8514_write_mask_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_write_mask_w),m_vga));
	m_isa->install16_device(0xaee8, 0xaeeb, 0, 0, read16_delegate(FUNC(mach64_device::ibm8514_read_mask_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_read_mask_w),m_vga));
	m_isa->install16_device(0xb6e8, 0xb6eb, 0, 0, read16_delegate(FUNC(mach64_device::ibm8514_backmix_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_backmix_w),m_vga));
	m_isa->install16_device(0xbae8, 0xbaeb, 0, 0, read16_delegate(FUNC(mach64_device::ibm8514_foremix_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_foremix_w),m_vga));
	m_isa->install16_device(0xbee8, 0xbeeb, 0, 0, read16_delegate(FUNC(mach64_device::ibm8514_multifunc_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_multifunc_w),m_vga));
	m_isa->install16_device(0xe2e8, 0xe2eb, 0, 0, read16_delegate(FUNC(mach64_device::ibm8514_pixel_xfer_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_pixel_xfer_w),m_vga));
	m_isa->install16_device(0xdaec, 0xdaef, 0, 0, read16_delegate(FUNC(mach64_device::mach8_sourcex_r),m_vga), write16_delegate(FUNC(mach64_device::mach8_ext_leftscissor_w),m_vga));
	m_isa->install16_device(0xdeec, 0xdeef, 0, 0, read16_delegate(FUNC(mach64_device::mach8_sourcey_r),m_vga), write16_delegate(FUNC(mach64_device::mach8_ext_topscissor_w),m_vga));
	m_isa->install16_device(0xfaec, 0xfaef, 0, 0, read16_delegate(FUNC(mach64_device::mach32_chipid_r),m_vga), write16_delegate());
	m_isa->install16_device(0xfeec, 0xfeef, 0, 0, read16_delegate(FUNC(mach64_device::mach8_linedraw_r),m_vga), write16_delegate(FUNC(mach64_device::mach8_linedraw_w),m_vga));

	m_isa->install_memory(0xa0000, 0xbffff, 0, 0, read8_delegate(FUNC(mach64_device::mem_r),m_vga), write8_delegate(FUNC(mach64_device::mem_w),m_vga));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa16_vga_gfxultra_device::device_reset()
{
}

void isa16_vga_gfxultrapro_device::device_reset()
{
}

void isa16_vga_mach64_device::device_reset()
{
}
