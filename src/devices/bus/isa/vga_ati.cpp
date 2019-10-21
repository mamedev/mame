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
#include "screen.h"


ROM_START( gfxultra )
	ROM_REGION(0x8000,"gfxultra", 0)
	ROM_LOAD("113-11504-002.bin", 0x00000, 0x8000, CRC(f498b36a) SHA1(117cfc972ce4645538ba7262222d8ff38bc2c58c) )
	ROM_IGNORE( 0x8000 )
ROM_END

ROM_START( gfxultrp )
	ROM_REGION(0x8000,"gfxultrapro", 0)

	ROM_DEFAULT_BIOS("isa")

	ROM_SYSTEM_BIOS( 0, "isa", "ISA BIOS 112-18900-100" )
	ROMX_LOAD("gfxultrapro.bin", 0x00000, 0x8000, CRC(4e5effd7) SHA1(84ad3abf7653e4734bf39f5d5c8b88e74527e8ce), ROM_BIOS(0) )

	// We can separate out this BIOS once a proper VLB bus emulation is available
	ROM_SYSTEM_BIOS( 1, "vlb", "VLB BIOS 113-19500-100" )
	ROMX_LOAD("gfxultrapro_vlb.bin", 0x00000, 0x8000, CRC(5018f71e) SHA1(61321dfecf1bcdd8043836fabbe41786dbf3001b), ROM_BIOS(1) )
ROM_END

ROM_START( mach64 )
	ROM_REGION(0x8000,"mach64", 0)

	ROM_SYSTEM_BIOS( 0, "isa", "ISA BIOS 112-28122-101" )
	ROMX_LOAD("mach64.bin", 0x00000, 0x8000, CRC(1300aa8f) SHA1(dfc7f817900f125b89b0bda16fcb205f066a47fc), ROM_BIOS(0) )

	// We can separate out these BIOSes once a proper PCI and VLB bus emulation is available
	ROM_SYSTEM_BIOS( 1, "vlb_d", "VLB DRAM BIOS 113-27803-102" )
	ROMX_LOAD("mach64_vlb_dram.bin", 0x00000, 0x8000, CRC(f2a24699) SHA1(580401a8bdfc379180a8d7d77305fc529b2a8374), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS( 2, "gfxultrapt_vlb", "ATi Graphics Pro Turbo VLB VRAM BIOS 113-26900-103" )
	ROMX_LOAD("mach64_vlb_vram.bin", 0x00000, 0x8000, CRC(47779d8f) SHA1(87b01b7a16d9c79dfc6c5aa8a39455c725d2e455), ROM_BIOS(2) )

	ROM_SYSTEM_BIOS( 3, "pci", "PCI BIOS 113-25420-100" )
	ROMX_LOAD("pci_mach64__113-25420-100-1995.27c256.u1.bin", 0x00000, 0x8000, CRC(762596e8) SHA1(9544b073ac182ec2990e18f54afbb96d52db744a), ROM_BIOS(3) )

	ROM_SYSTEM_BIOS( 4, "pci_v1", "PCI mach64 V1 BIOS 113-34404-104" )
	ROMX_LOAD("pci_mach64_v1_113-34404-104_1996.bin", 0x00000, 0x8000, CRC(c6a39c3f) SHA1(0f4cf9221179c675dafafde638bc00244b6feb63), ROM_BIOS(4) )
	ROM_IGNORE(0x8000)

ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA16_VGA_GFXULTRA,     isa16_vga_gfxultra_device,    "gfxultra",   "ATi Graphics Ultra Card")
DEFINE_DEVICE_TYPE(ISA16_SVGA_GFXULTRAPRO, isa16_vga_gfxultrapro_device, "gfxultrap", "ATi Graphics Ultra Pro Card")
DEFINE_DEVICE_TYPE(ISA16_SVGA_MACH64,      isa16_vga_mach64_device,      "mach64isa", "ATi mach64 Card")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa16_vga_gfxultra_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update("vga", FUNC(ati_vga_device::screen_update));

	ATI_VGA(config, "vga", 0).set_screen("screen");
}

void isa16_vga_gfxultrapro_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update("vga", FUNC(mach32_device::screen_update));

	ATIMACH32(config, "vga", 0).set_screen("screen");
}

void isa16_vga_mach64_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update("vga", FUNC(mach64_device::screen_update));

	ATIMACH64(config, "vga", 0).set_screen("screen");
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *isa16_vga_gfxultra_device::device_rom_region() const
{
	return ROM_NAME( gfxultra );
}

const tiny_rom_entry *isa16_vga_gfxultrapro_device::device_rom_region() const
{
	return ROM_NAME( gfxultrp );
}

const tiny_rom_entry *isa16_vga_mach64_device::device_rom_region() const
{
	return ROM_NAME( mach64 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_vga_device - constructor
//-------------------------------------------------

isa16_vga_gfxultra_device::isa16_vga_gfxultra_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA16_VGA_GFXULTRA, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this),
	m_vga(nullptr), m_8514(nullptr)
{
}

isa16_vga_gfxultrapro_device::isa16_vga_gfxultrapro_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA16_SVGA_GFXULTRAPRO, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this),
	m_vga(nullptr)
{
}

isa16_vga_mach64_device::isa16_vga_mach64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA16_SVGA_MACH64, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this),
	m_vga(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
READ8_MEMBER(isa16_vga_gfxultra_device::input_port_0_r ) { return 0xff; } //return machine().root_device().ioport("IN0")->read(); }
READ8_MEMBER(isa16_vga_gfxultrapro_device::input_port_0_r ) { return 0xff; } //return machine().root_device().ioport("IN0")->read(); }
READ8_MEMBER(isa16_vga_mach64_device::input_port_0_r ) { return 0xff; } //return machine().root_device().ioport("IN0")->read(); }

void isa16_vga_gfxultra_device::device_start()
{
	set_isa_device();

	m_vga = subdevice<ati_vga_device>("vga");
	m_8514 = subdevice<mach8_device>("vga:8514a");

	m_isa->install_rom(this, 0xc0000, 0xc7fff, "vga", "gfxultra");

	m_isa->install_device(0x1ce, 0x1cf, read8_delegate(FUNC(ati_vga_device::ati_port_ext_r),m_vga), write8_delegate(FUNC(ati_vga_device::ati_port_ext_w),m_vga));
	m_isa->install_device(0x2e8, 0x2ef, read8_delegate(FUNC(mach8_device::ibm8514_status_r),m_8514), write8_delegate(FUNC(mach8_device::ibm8514_htotal_w),m_8514));
	m_isa->install_device(0x3b0, 0x3bf, read8_delegate(FUNC(ati_vga_device::port_03b0_r),m_vga), write8_delegate(FUNC(vga_device::port_03b0_w),m_vga));
	m_isa->install_device(0x3c0, 0x3cf, read8_delegate(FUNC(ati_vga_device::port_03c0_r),m_vga), write8_delegate(FUNC(vga_device::port_03c0_w),m_vga));
	m_isa->install_device(0x3d0, 0x3df, read8_delegate(FUNC(ati_vga_device::port_03d0_r),m_vga), write8_delegate(FUNC(vga_device::port_03d0_w),m_vga));
	m_isa->install16_device(0x12e8, 0x12eb, read16_delegate(FUNC(mach8_device::ibm8514_vtotal_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_vtotal_w),m_8514));
	m_isa->install16_device(0x12ec, 0x12ef, read16_delegate(FUNC(mach8_device::mach8_config1_r),m_8514), write16_delegate());
	m_isa->install16_device(0x16e8, 0x16eb, read16_delegate(FUNC(mach8_device::ibm8514_vdisp_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_vdisp_w),m_8514));
	m_isa->install16_device(0x16ec, 0x16ef, read16_delegate(FUNC(mach8_device::mach8_config2_r),m_8514), write16_delegate());
	m_isa->install16_device(0x1ae8, 0x1aeb, read16_delegate(FUNC(mach8_device::ibm8514_vsync_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_vsync_w),m_8514));
	m_isa->install16_device(0x22e8, 0x22eb, read16_delegate(FUNC(mach8_device::mach8_readonly_r),m_8514),write16_delegate(FUNC(mach8_device::ibm8514_display_ctrl_w),m_8514));
	m_isa->install16_device(0x26e8, 0x26eb, read16_delegate(FUNC(mach8_device::ibm8514_htotal_r),m_8514),write16_delegate());
	m_isa->install16_device(0x26ec, 0x26ef, read16_delegate(FUNC(mach8_device::mach8_readonly_r),m_8514),write16_delegate(FUNC(mach8_device::mach8_crt_pitch_w),m_8514));
	m_isa->install16_device(0x2ee8, 0x2eeb, read16_delegate(FUNC(mach8_device::ibm8514_subcontrol_r),m_8514),write16_delegate());
	m_isa->install16_device(0x42e8, 0x42eb, read16_delegate(FUNC(mach8_device::ibm8514_substatus_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_subcontrol_w),m_8514));
	m_isa->install16_device(0x4ae8, 0x4aeb, read16_delegate(FUNC(mach8_device::mach8_readonly_r),m_8514), write16_delegate(FUNC(mach8_device::mach8_advfunc_w),m_8514));
	m_isa->install16_device(0x4aec, 0x4aef, read16_delegate(FUNC(mach8_device::mach8_clksel_r),m_8514), write16_delegate(FUNC(mach8_device::mach8_clksel_w),m_8514));
	m_isa->install16_device(0x52e8, 0x52eb, read16_delegate(FUNC(mach8_device::mach8_ec0_r),m_8514), write16_delegate(FUNC(mach8_device::mach8_ec0_w),m_8514));
	m_isa->install16_device(0x52ec, 0x52ef, read16_delegate(FUNC(mach8_device::mach8_scratch0_r),m_8514), write16_delegate(FUNC(mach8_device::mach8_scratch0_w),m_8514));
	m_isa->install16_device(0x56e8, 0x56eb, read16_delegate(FUNC(mach8_device::mach8_ec1_r),m_8514), write16_delegate(FUNC(mach8_device::mach8_ec1_w),m_8514));
	m_isa->install16_device(0x56ec, 0x56ef, read16_delegate(FUNC(mach8_device::mach8_scratch0_r),m_8514), write16_delegate(FUNC(mach8_device::mach8_scratch0_w),m_8514));
	m_isa->install16_device(0x5ae8, 0x5aeb, read16_delegate(FUNC(mach8_device::mach8_ec2_r),m_8514), write16_delegate(FUNC(mach8_device::mach8_ec2_w),m_8514));
	m_isa->install16_device(0x5ee8, 0x5eeb, read16_delegate(FUNC(mach8_device::mach8_ec3_r),m_8514), write16_delegate(FUNC(mach8_device::mach8_ec3_w),m_8514));
	m_isa->install16_device(0x6eec, 0x6eef, read16_delegate(FUNC(mach8_device::mach8_readonly_r),m_8514), write16_delegate(FUNC(mach8_device::mach8_ge_offset_l_w),m_8514));
	m_isa->install16_device(0x72ec, 0x72ef, read16_delegate(FUNC(mach8_device::mach8_readonly_r),m_8514), write16_delegate(FUNC(mach8_device::mach8_ge_offset_h_w),m_8514));
	m_isa->install16_device(0x76ec, 0x76ef, read16_delegate(FUNC(mach8_device::mach8_readonly_r),m_8514), write16_delegate(FUNC(mach8_device::mach8_ge_pitch_w),m_8514));
	m_isa->install16_device(0x7aec, 0x7aef, read16_delegate(FUNC(mach8_device::mach8_readonly_r),m_8514), write16_delegate(FUNC(mach8_device::mach8_ge_ext_config_w),m_8514));
	m_isa->install16_device(0x82e8, 0x82eb, read16_delegate(FUNC(mach8_device::ibm8514_currenty_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_currenty_w),m_8514));
	m_isa->install16_device(0x86e8, 0x86eb, read16_delegate(FUNC(mach8_device::ibm8514_currentx_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_currentx_w),m_8514));
	m_isa->install16_device(0x8ae8, 0x8aeb, read16_delegate(FUNC(mach8_device::ibm8514_desty_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_desty_w),m_8514));
	m_isa->install16_device(0x8ee8, 0x8eeb, read16_delegate(FUNC(mach8_device::ibm8514_destx_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_destx_w),m_8514));
	m_isa->install16_device(0x8eec, 0x8eef, read16_delegate(FUNC(mach8_device::mach8_ge_ext_config_r),m_8514), write16_delegate(FUNC(mach8_device::mach8_patt_data_w),m_8514));
	m_isa->install16_device(0x92e8, 0x92eb, read16_delegate(FUNC(mach8_device::ibm8514_line_error_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_line_error_w),m_8514));
	m_isa->install16_device(0x96e8, 0x96eb, read16_delegate(FUNC(mach8_device::ibm8514_width_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_width_w),m_8514));
	m_isa->install16_device(0x96ec, 0x96ef, read16_delegate(FUNC(mach8_device::mach8_bresenham_count_r),m_8514), write16_delegate(FUNC(mach8_device::mach8_bresenham_count_w),m_8514));
	m_isa->install16_device(0x9ae8, 0x9aeb, read16_delegate(FUNC(mach8_device::ibm8514_gpstatus_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_cmd_w),m_8514));
	m_isa->install16_device(0x9aec, 0x9aef, read16_delegate(FUNC(mach8_device::mach8_ext_fifo_r),m_8514), write16_delegate(FUNC(mach8_device::mach8_linedraw_index_w),m_8514));
	m_isa->install16_device(0x9ee8, 0x9eeb, read16_delegate(FUNC(mach8_device::ibm8514_ssv_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_ssv_w),m_8514));
	m_isa->install16_device(0xa2e8, 0xa2eb, read16_delegate(FUNC(mach8_device::ibm8514_bgcolour_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_bgcolour_w),m_8514));
	m_isa->install16_device(0xa6e8, 0xa6eb, read16_delegate(FUNC(mach8_device::ibm8514_fgcolour_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_fgcolour_w),m_8514));
	m_isa->install16_device(0xaae8, 0xaaeb, read16_delegate(FUNC(ibm8514a_device::ibm8514_write_mask_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_write_mask_w),m_8514));
	m_isa->install16_device(0xaee8, 0xaeeb, read16_delegate(FUNC(ibm8514a_device::ibm8514_read_mask_r),m_8514), write16_delegate(FUNC(ibm8514a_device::ibm8514_read_mask_w),m_8514));
	m_isa->install16_device(0xb6e8, 0xb6eb, read16_delegate(FUNC(mach8_device::ibm8514_backmix_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_backmix_w),m_8514));
	m_isa->install16_device(0xbae8, 0xbaeb, read16_delegate(FUNC(mach8_device::ibm8514_foremix_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_foremix_w),m_8514));
	m_isa->install16_device(0xbee8, 0xbeeb, read16_delegate(FUNC(mach8_device::ibm8514_multifunc_r),m_8514), write16_delegate(FUNC(mach8_device::ibm8514_multifunc_w),m_8514));
	m_isa->install16_device(0xcaec, 0xcaef, read16_delegate(FUNC(mach8_device::mach8_readonly_r),m_8514), write16_delegate(FUNC(mach8_device::mach8_scan_x_w),m_8514));
	m_isa->install16_device(0xceec, 0xceef, read16_delegate(FUNC(mach8_device::mach8_readonly_r),m_8514), write16_delegate(FUNC(mach8_device::mach8_dp_config_w),m_8514));
	m_isa->install16_device(0xe2e8, 0xe2eb, read16_delegate(FUNC(mach8_device::ibm8514_pixel_xfer_r),m_8514), write16_delegate(FUNC(mach8_device::mach8_pixel_xfer_w),m_8514));
	m_isa->install16_device(0xdaec, 0xdaef, read16_delegate(FUNC(mach8_device::mach8_sourcex_r),m_8514), write16_delegate(FUNC(mach8_device::mach8_ext_leftscissor_w),m_8514));
	m_isa->install16_device(0xdeec, 0xdeef, read16_delegate(FUNC(mach8_device::mach8_sourcey_r),m_8514), write16_delegate(FUNC(mach8_device::mach8_ext_topscissor_w),m_8514));
	m_isa->install16_device(0xfeec, 0xfeef, read16_delegate(FUNC(mach8_device::mach8_linedraw_r),m_8514), write16_delegate(FUNC(mach8_device::mach8_linedraw_w),m_8514));

	m_isa->install_memory(0xa0000, 0xbffff, read8_delegate(FUNC(ati_vga_device::mem_r),m_vga), write8_delegate(FUNC(ati_vga_device::mem_w),m_vga));
}

void isa16_vga_gfxultrapro_device::device_start()
{
	set_isa_device();

	m_vga = subdevice<mach32_device>("vga");

	m_isa->install_rom(this, 0xc0000, 0xc7fff, "vga", "gfxultrapro");

	m_isa->install_device(0x1ce, 0x1cf, read8_delegate(FUNC(mach32_device::ati_port_ext_r),m_vga), write8_delegate(FUNC(mach32_device::ati_port_ext_w),m_vga));
	m_isa->install_device(0x2e8, 0x2ef, read8_delegate(FUNC(mach32_device::mach32_status_r),m_vga), write8_delegate(FUNC(mach32_device::ibm8514_htotal_w),m_vga));
	m_isa->install_device(0x3b0, 0x3bf, read8_delegate(FUNC(mach32_device::port_03b0_r),m_vga), write8_delegate(FUNC(mach32_device::port_03b0_w),m_vga));
	m_isa->install_device(0x3c0, 0x3cf, read8_delegate(FUNC(mach32_device::port_03c0_r),m_vga), write8_delegate(FUNC(mach32_device::port_03c0_w),m_vga));
	m_isa->install_device(0x3d0, 0x3df, read8_delegate(FUNC(mach32_device::port_03d0_r),m_vga), write8_delegate(FUNC(mach32_device::port_03d0_w),m_vga));
	m_isa->install16_device(0xaec, 0xaef, read16_delegate(FUNC(mach32_device::mach32_readonly_r),m_vga), write16_delegate(FUNC(mach32_device::mach32_cursor_l_w),m_vga));
	m_isa->install16_device(0xeec, 0xeef, read16_delegate(FUNC(mach32_device::mach32_readonly_r),m_vga), write16_delegate(FUNC(mach32_device::mach32_cursor_h_w),m_vga));
	m_isa->install16_device(0x12e8, 0x12eb, read16_delegate(FUNC(mach32_device::ibm8514_vtotal_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_vtotal_w),m_vga));
	m_isa->install16_device(0x12ec, 0x12ef, read16_delegate(FUNC(mach32_device::mach32_config1_r),m_vga), write16_delegate(FUNC(mach32_device::mach32_cursor_pos_h),m_vga));
	m_isa->install16_device(0x16e8, 0x16eb, read16_delegate(FUNC(mach32_device::ibm8514_vdisp_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_vdisp_w),m_vga));
	m_isa->install16_device(0x16ec, 0x16ef, read16_delegate(FUNC(mach32_device::mach8_config2_r),m_vga), write16_delegate(FUNC(mach32_device::mach32_cursor_pos_v),m_vga));
	m_isa->install16_device(0x1ae8, 0x1aeb, read16_delegate(FUNC(mach32_device::ibm8514_vsync_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_vsync_w),m_vga));
	m_isa->install16_device(0x1aec, 0x1aef, read16_delegate(FUNC(mach32_device::mach32_readonly_r),m_vga), write16_delegate(FUNC(mach32_device::mach32_cursor_colour_b_w),m_vga));
	m_isa->install16_device(0x1eec, 0x1eef, read16_delegate(FUNC(mach32_device::mach32_readonly_r),m_vga), write16_delegate(FUNC(mach32_device::mach32_cursor_offset_w),m_vga));
	m_isa->install16_device(0x26e8, 0x26eb, read16_delegate(FUNC(mach32_device::ibm8514_htotal_r),m_vga),write16_delegate());
	m_isa->install16_device(0x26ec, 0x26ef, read16_delegate(FUNC(mach32_device::mach32_readonly_r),m_vga),write16_delegate(FUNC(mach32_device::mach8_crt_pitch_w),m_vga));
	m_isa->install16_device(0x2ee8, 0x2eeb, read16_delegate(FUNC(mach32_device::ibm8514_subcontrol_r),m_vga),write16_delegate());
	m_isa->install16_device(0x3aec, 0x3aef, read16_delegate(FUNC(mach32_device::mach32_readonly_r),m_vga), write16_delegate(FUNC(mach32_device::mach32_cursor_colour_0_w),m_vga));
	m_isa->install16_device(0x3eec, 0x3eef, read16_delegate(FUNC(mach32_device::mach32_readonly_r),m_vga), write16_delegate(FUNC(mach32_device::mach32_cursor_colour_0_w),m_vga));
	m_isa->install16_device(0x42e8, 0x42eb, read16_delegate(FUNC(mach32_device::ibm8514_substatus_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_subcontrol_w),m_vga));
	m_isa->install16_device(0x42ec, 0x42ef, read16_delegate(FUNC(mach32_device::mach32_mem_boundary_r),m_vga), write16_delegate(FUNC(mach32_device::mach32_mem_boundary_w),m_vga));
	m_isa->install16_device(0x4ae8, 0x4aeb, read16_delegate(FUNC(mach32_device::mach32_readonly_r),m_vga), write16_delegate(FUNC(mach32_device::mach8_advfunc_w),m_vga));
	m_isa->install16_device(0x4aec, 0x4aef, read16_delegate(FUNC(mach32_device::mach8_clksel_r),m_vga), write16_delegate(FUNC(mach32_device::mach8_clksel_w),m_vga));
	m_isa->install16_device(0x52e8, 0x52eb, read16_delegate(FUNC(mach32_device::mach8_ec0_r),m_vga), write16_delegate(FUNC(mach32_device::mach8_ec0_w),m_vga));
	m_isa->install16_device(0x52ec, 0x52ef, read16_delegate(FUNC(mach32_device::mach8_scratch0_r),m_vga), write16_delegate(FUNC(mach32_device::mach8_scratch0_w),m_vga));
	m_isa->install16_device(0x56e8, 0x56eb, read16_delegate(FUNC(mach32_device::mach8_ec1_r),m_vga), write16_delegate(FUNC(mach32_device::mach8_ec1_w),m_vga));
	m_isa->install16_device(0x56ec, 0x56ef, read16_delegate(FUNC(mach32_device::mach8_scratch0_r),m_vga), write16_delegate(FUNC(mach32_device::mach8_scratch0_w),m_vga));
	m_isa->install16_device(0x5ae8, 0x5aeb, read16_delegate(FUNC(mach32_device::mach8_ec2_r),m_vga), write16_delegate(FUNC(mach32_device::mach8_ec2_w),m_vga));
	m_isa->install16_device(0x5ee8, 0x5eeb, read16_delegate(FUNC(mach32_device::mach8_ec3_r),m_vga), write16_delegate(FUNC(mach32_device::mach8_ec3_w),m_vga));
	m_isa->install16_device(0x62ec, 0x62ef, read16_delegate(FUNC(mach32_device::mach32_ext_ge_r),m_vga), write16_delegate(FUNC(mach32_device::mach32_horz_overscan_w),m_vga));
	m_isa->install16_device(0x6eec, 0x6eef, read16_delegate(FUNC(mach32_device::mach32_readonly_r),m_vga), write16_delegate(FUNC(mach32_device::mach8_ge_offset_l_w),m_vga));
	m_isa->install16_device(0x72ec, 0x72ef, read16_delegate(FUNC(mach32_device::mach32_readonly_r),m_vga), write16_delegate(FUNC(mach32_device::mach8_ge_offset_h_w),m_vga));
	m_isa->install16_device(0x76ec, 0x76ef, read16_delegate(FUNC(mach32_device::mach32_readonly_r),m_vga), write16_delegate(FUNC(mach32_device::mach8_ge_pitch_w),m_vga));
	m_isa->install16_device(0x7aec, 0x7aef, read16_delegate(FUNC(mach32_device::mach32_readonly_r),m_vga), write16_delegate(FUNC(mach32_device::mach32_ge_ext_config_w),m_vga));
	m_isa->install16_device(0x82e8, 0x82eb, read16_delegate(FUNC(mach32_device::ibm8514_currenty_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_currenty_w),m_vga));
	m_isa->install16_device(0x86e8, 0x86eb, read16_delegate(FUNC(mach32_device::ibm8514_currentx_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_currentx_w),m_vga));
	m_isa->install16_device(0x8ae8, 0x8aeb, read16_delegate(FUNC(mach32_device::ibm8514_desty_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_desty_w),m_vga));
	m_isa->install16_device(0x8ee8, 0x8eeb, read16_delegate(FUNC(mach32_device::ibm8514_destx_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_destx_w),m_vga));
	m_isa->install16_device(0x8eec, 0x8eef, read16_delegate(FUNC(mach32_device::mach8_ge_ext_config_r),m_vga), write16_delegate(FUNC(mach32_device::mach8_patt_data_w),m_vga));
	m_isa->install16_device(0x92e8, 0x92eb, read16_delegate(FUNC(mach32_device::ibm8514_line_error_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_line_error_w),m_vga));
	m_isa->install16_device(0x96e8, 0x96eb, read16_delegate(FUNC(mach32_device::ibm8514_width_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_width_w),m_vga));
	m_isa->install16_device(0x96ec, 0x96ef, read16_delegate(FUNC(mach32_device::mach8_bresenham_count_r),m_vga), write16_delegate(FUNC(mach32_device::mach8_bresenham_count_w),m_vga));
	m_isa->install16_device(0x9ae8, 0x9aeb, read16_delegate(FUNC(mach32_device::ibm8514_gpstatus_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_cmd_w),m_vga));
	m_isa->install16_device(0x9aec, 0x9aef, read16_delegate(FUNC(mach32_device::mach8_ext_fifo_r),m_vga), write16_delegate(FUNC(mach32_device::mach8_linedraw_index_w),m_vga));
	m_isa->install16_device(0x9ee8, 0x9eeb, read16_delegate(FUNC(mach32_device::ibm8514_ssv_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_ssv_w),m_vga));
	m_isa->install16_device(0xa2e8, 0xa2eb, read16_delegate(FUNC(mach32_device::ibm8514_bgcolour_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_bgcolour_w),m_vga));
	m_isa->install16_device(0xa6e8, 0xa6eb, read16_delegate(FUNC(mach32_device::ibm8514_fgcolour_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_fgcolour_w),m_vga));
	m_isa->install16_device(0xaae8, 0xaaeb, read16_delegate(FUNC(mach32_device::ibm8514_write_mask_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_write_mask_w),m_vga));
	m_isa->install16_device(0xaee8, 0xaeeb, read16_delegate(FUNC(mach32_device::ibm8514_read_mask_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_read_mask_w),m_vga));
	m_isa->install16_device(0xb6e8, 0xb6eb, read16_delegate(FUNC(mach32_device::ibm8514_backmix_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_backmix_w),m_vga));
	m_isa->install16_device(0xbae8, 0xbaeb, read16_delegate(FUNC(mach32_device::ibm8514_foremix_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_foremix_w),m_vga));
	m_isa->install16_device(0xbee8, 0xbeeb, read16_delegate(FUNC(mach32_device::ibm8514_multifunc_r),m_vga), write16_delegate(FUNC(mach32_device::ibm8514_multifunc_w),m_vga));
	m_isa->install16_device(0xcaec, 0xcaef, read16_delegate(FUNC(mach32_device::mach32_readonly_r),m_vga), write16_delegate(FUNC(mach32_device::mach8_scan_x_w),m_vga));
	m_isa->install16_device(0xceec, 0xceef, read16_delegate(FUNC(mach32_device::mach32_readonly_r),m_vga), write16_delegate(FUNC(mach32_device::mach8_dp_config_w),m_vga));
	m_isa->install16_device(0xe2e8, 0xe2eb, read16_delegate(FUNC(mach32_device::ibm8514_pixel_xfer_r),m_vga), write16_delegate(FUNC(mach32_device::mach8_pixel_xfer_w),m_vga));
	m_isa->install16_device(0xdaec, 0xdaef, read16_delegate(FUNC(mach32_device::mach8_sourcex_r),m_vga), write16_delegate(FUNC(mach32_device::mach8_ext_leftscissor_w),m_vga));
	m_isa->install16_device(0xdeec, 0xdeef, read16_delegate(FUNC(mach32_device::mach8_sourcey_r),m_vga), write16_delegate(FUNC(mach32_device::mach8_ext_topscissor_w),m_vga));
	m_isa->install16_device(0xfaec, 0xfaef, read16_delegate(FUNC(mach32_device::mach32_chipid_r),m_vga), write16_delegate());
	m_isa->install16_device(0xfeec, 0xfeef, read16_delegate(FUNC(mach32_device::mach8_linedraw_r),m_vga), write16_delegate(FUNC(mach32_device::mach8_linedraw_w),m_vga));

	m_isa->install_memory(0xa0000, 0xbffff, read8_delegate(FUNC(mach32_device::mem_r),m_vga), write8_delegate(FUNC(mach32_device::mem_w),m_vga));
}

void isa16_vga_mach64_device::device_start()
{
	set_isa_device();

	m_vga = subdevice<mach64_device>("vga");

	m_isa->install_rom(this, 0xc0000, 0xc7fff, "vga", "mach64");

	m_isa->install_device(0x1ce, 0x1cf, read8_delegate(FUNC(mach64_device::ati_port_ext_r),m_vga), write8_delegate(FUNC(mach64_device::ati_port_ext_w),m_vga));
	m_isa->install_device(0x2e8, 0x2ef, read8_delegate(FUNC(mach64_device::mach32_status_r),m_vga), write8_delegate(FUNC(mach64_device::ibm8514_htotal_w),m_vga));
	m_isa->install_device(0x3b0, 0x3bf, read8_delegate(FUNC(mach64_device::port_03b0_r),m_vga), write8_delegate(FUNC(mach64_device::port_03b0_w),m_vga));
	m_isa->install_device(0x3c0, 0x3cf, read8_delegate(FUNC(mach64_device::port_03c0_r),m_vga), write8_delegate(FUNC(mach64_device::port_03c0_w),m_vga));
	m_isa->install_device(0x3d0, 0x3df, read8_delegate(FUNC(mach64_device::port_03d0_r),m_vga), write8_delegate(FUNC(mach64_device::port_03d0_w),m_vga));
	m_isa->install16_device(0x12e8, 0x12eb, read16_delegate(FUNC(mach64_device::ibm8514_vtotal_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_vtotal_w),m_vga));
	m_isa->install16_device(0x12ec, 0x12ef, read16_delegate(FUNC(mach64_device::mach32_config1_r),m_vga), write16_delegate(FUNC(mach64_device::mach64_config1_w),m_vga));
	m_isa->install16_device(0x16e8, 0x16eb, read16_delegate(FUNC(mach64_device::ibm8514_vdisp_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_vdisp_w),m_vga));
	m_isa->install16_device(0x16ec, 0x16ef, read16_delegate(FUNC(mach64_device::mach8_config2_r),m_vga), write16_delegate(FUNC(mach64_device::mach64_config2_w),m_vga));
	m_isa->install16_device(0x1ae8, 0x1aeb, read16_delegate(FUNC(mach64_device::ibm8514_vsync_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_vsync_w),m_vga));
	m_isa->install16_device(0x26e8, 0x26eb, read16_delegate(FUNC(mach64_device::ibm8514_htotal_r),m_vga),write16_delegate());
	m_isa->install16_device(0x26ec, 0x26ef, read16_delegate(FUNC(mach64_device::mach32_readonly_r),m_vga),write16_delegate(FUNC(mach64_device::mach8_crt_pitch_w),m_vga));
	m_isa->install16_device(0x2ee8, 0x2eeb, read16_delegate(FUNC(mach64_device::ibm8514_subcontrol_r),m_vga),write16_delegate());
	m_isa->install16_device(0x42e8, 0x42eb, read16_delegate(FUNC(mach64_device::ibm8514_substatus_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_subcontrol_w),m_vga));
	m_isa->install16_device(0x42ec, 0x42ef, read16_delegate(FUNC(mach64_device::mach32_mem_boundary_r),m_vga), write16_delegate(FUNC(mach64_device::mach32_mem_boundary_w),m_vga));
	m_isa->install16_device(0x4ae8, 0x4aeb, read16_delegate(FUNC(mach64_device::mach32_readonly_r),m_vga), write16_delegate(FUNC(mach64_device::mach8_advfunc_w),m_vga));
	m_isa->install16_device(0x4aec, 0x4aef, read16_delegate(FUNC(mach64_device::mach8_clksel_r),m_vga), write16_delegate(FUNC(mach64_device::mach8_clksel_w),m_vga));
	m_isa->install16_device(0x52e8, 0x52eb, read16_delegate(FUNC(mach64_device::mach8_ec0_r),m_vga), write16_delegate(FUNC(mach64_device::mach8_ec0_w),m_vga));
	m_isa->install16_device(0x52ec, 0x52ef, read16_delegate(FUNC(mach64_device::mach8_scratch0_r),m_vga), write16_delegate(FUNC(mach64_device::mach8_scratch0_w),m_vga));
	m_isa->install16_device(0x56e8, 0x56eb, read16_delegate(FUNC(mach64_device::mach8_ec1_r),m_vga), write16_delegate(FUNC(mach64_device::mach8_ec1_w),m_vga));
	m_isa->install16_device(0x56ec, 0x56ef, read16_delegate(FUNC(mach64_device::mach8_scratch0_r),m_vga), write16_delegate(FUNC(mach64_device::mach8_scratch0_w),m_vga));
	m_isa->install16_device(0x5ae8, 0x5aeb, read16_delegate(FUNC(mach64_device::mach8_ec2_r),m_vga), write16_delegate(FUNC(mach64_device::mach8_ec2_w),m_vga));
	m_isa->install16_device(0x5ee8, 0x5eeb, read16_delegate(FUNC(mach64_device::mach8_ec3_r),m_vga), write16_delegate(FUNC(mach64_device::mach8_ec3_w),m_vga));
	m_isa->install16_device(0x6eec, 0x6eef, read16_delegate(FUNC(mach64_device::mach32_readonly_r),m_vga), write16_delegate(FUNC(mach64_device::mach8_ge_offset_l_w),m_vga));
	m_isa->install16_device(0x72ec, 0x72ef, read16_delegate(FUNC(mach64_device::mach32_readonly_r),m_vga), write16_delegate(FUNC(mach64_device::mach8_ge_offset_h_w),m_vga));
	m_isa->install16_device(0x76ec, 0x76ef, read16_delegate(FUNC(mach64_device::mach32_readonly_r),m_vga), write16_delegate(FUNC(mach64_device::mach8_ge_pitch_w),m_vga));
	m_isa->install16_device(0x7aec, 0x7aef, read16_delegate(FUNC(mach64_device::mach32_readonly_r),m_vga), write16_delegate(FUNC(mach64_device::mach32_ge_ext_config_w),m_vga));
	m_isa->install16_device(0x82e8, 0x82eb, read16_delegate(FUNC(mach64_device::ibm8514_currenty_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_currenty_w),m_vga));
	m_isa->install16_device(0x86e8, 0x86eb, read16_delegate(FUNC(mach64_device::ibm8514_currentx_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_currentx_w),m_vga));
	m_isa->install16_device(0x8ae8, 0x8aeb, read16_delegate(FUNC(mach64_device::ibm8514_desty_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_desty_w),m_vga));
	m_isa->install16_device(0x8ee8, 0x8eeb, read16_delegate(FUNC(mach64_device::ibm8514_destx_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_destx_w),m_vga));
	m_isa->install16_device(0x8eec, 0x8eef, read16_delegate(FUNC(mach64_device::mach8_ge_ext_config_r),m_vga), write16_delegate(FUNC(mach64_device::mach8_patt_data_w),m_vga));
	m_isa->install16_device(0x92e8, 0x92eb, read16_delegate(FUNC(mach64_device::ibm8514_line_error_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_line_error_w),m_vga));
	m_isa->install16_device(0x96e8, 0x96eb, read16_delegate(FUNC(mach64_device::ibm8514_width_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_width_w),m_vga));
	m_isa->install16_device(0x96ec, 0x96ef, read16_delegate(FUNC(mach64_device::mach8_bresenham_count_r),m_vga), write16_delegate(FUNC(mach64_device::mach8_bresenham_count_w),m_vga));
	m_isa->install16_device(0x9ae8, 0x9aeb, read16_delegate(FUNC(mach64_device::ibm8514_gpstatus_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_cmd_w),m_vga));
	m_isa->install16_device(0x9aec, 0x9aef, read16_delegate(FUNC(mach64_device::mach8_ext_fifo_r),m_vga), write16_delegate(FUNC(mach64_device::mach8_linedraw_index_w),m_vga));
	m_isa->install16_device(0x9ee8, 0x9eeb, read16_delegate(FUNC(mach64_device::ibm8514_ssv_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_ssv_w),m_vga));
	m_isa->install16_device(0xa2e8, 0xa2eb, read16_delegate(FUNC(mach64_device::ibm8514_bgcolour_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_bgcolour_w),m_vga));
	m_isa->install16_device(0xa6e8, 0xa6eb, read16_delegate(FUNC(mach64_device::ibm8514_fgcolour_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_fgcolour_w),m_vga));
	m_isa->install16_device(0xaae8, 0xaaeb, read16_delegate(FUNC(mach64_device::ibm8514_write_mask_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_write_mask_w),m_vga));
	m_isa->install16_device(0xaee8, 0xaeeb, read16_delegate(FUNC(mach64_device::ibm8514_read_mask_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_read_mask_w),m_vga));
	m_isa->install16_device(0xb6e8, 0xb6eb, read16_delegate(FUNC(mach64_device::ibm8514_backmix_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_backmix_w),m_vga));
	m_isa->install16_device(0xbae8, 0xbaeb, read16_delegate(FUNC(mach64_device::ibm8514_foremix_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_foremix_w),m_vga));
	m_isa->install16_device(0xbee8, 0xbeeb, read16_delegate(FUNC(mach64_device::ibm8514_multifunc_r),m_vga), write16_delegate(FUNC(mach64_device::ibm8514_multifunc_w),m_vga));
	m_isa->install16_device(0xcaec, 0xcaef, read16_delegate(FUNC(mach64_device::mach32_readonly_r),m_vga), write16_delegate(FUNC(mach64_device::mach8_scan_x_w),m_vga));
	m_isa->install16_device(0xceec, 0xceef, read16_delegate(FUNC(mach64_device::mach32_readonly_r),m_vga), write16_delegate(FUNC(mach64_device::mach8_dp_config_w),m_vga));
	m_isa->install16_device(0xe2e8, 0xe2eb, read16_delegate(FUNC(mach64_device::ibm8514_pixel_xfer_r),m_vga), write16_delegate(FUNC(mach64_device::mach8_pixel_xfer_w),m_vga));
	m_isa->install16_device(0xdaec, 0xdaef, read16_delegate(FUNC(mach64_device::mach8_sourcex_r),m_vga), write16_delegate(FUNC(mach64_device::mach8_ext_leftscissor_w),m_vga));
	m_isa->install16_device(0xdeec, 0xdeef, read16_delegate(FUNC(mach64_device::mach8_sourcey_r),m_vga), write16_delegate(FUNC(mach64_device::mach8_ext_topscissor_w),m_vga));
	m_isa->install16_device(0xfaec, 0xfaef, read16_delegate(FUNC(mach64_device::mach32_chipid_r),m_vga), write16_delegate());
	m_isa->install16_device(0xfeec, 0xfeef, read16_delegate(FUNC(mach64_device::mach8_linedraw_r),m_vga), write16_delegate(FUNC(mach64_device::mach8_linedraw_w),m_vga));

	m_isa->install_memory(0xa0000, 0xbffff, read8_delegate(FUNC(mach64_device::mem_r),m_vga), write8_delegate(FUNC(mach64_device::mem_w),m_vga));
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
