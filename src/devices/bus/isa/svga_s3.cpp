// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/**************************************************************************************************

  ISA SVGA S3 wrapper

  ISA16
  - P86C911
  - P86C911A
  - P86C924
  - P86C801
  - 86C801-R
  ISA16 / VL-Bus
  - P86C928
  - P86C805 / 86C805-P / 86C805-Q
  - 86C805i
  VL-Bus / PCI
  - Vision864
  - Vision868
  - Vision964
  - Vision968
  - Trio64 (Spea V7-MIRAGE P64, probably last card before full transition to PCI)

  TODO:
  - all s3_764 roms are really PCI and needs to be moved to video/virge_pci.cpp.

**************************************************************************************************/

#include "emu.h"
#include "svga_s3.h"

#include "screen.h"


ROM_START( s3_764 )
	ROM_REGION(0x8000,"s3_764", 0)
	ROM_DEFAULT_BIOS("9fxv330")

	// Number Nine Visual Technology 9FX Vision 330. Now I'm 64... (c)1995 Number Nine Visual Technology Corp. All Rights Reserved. #9-764 BIOS Version 2.03.10. (c)1995 S3 Inc Version 2.00.1.3-08
	ROM_SYSTEM_BIOS( 0, "9fxv330", "PCI Number Nine 9FX Vision 330 2.03.10 (S3 Trio64)" )
	ROMX_LOAD("s3_764.bin", 0x00000, 0x8000, CRC(4f10aac7) SHA1(c77b3f11cc15679121314823588887dd547cd715), ROM_BIOS(0) )
	ROM_IGNORE( 0x8000 )

	// The following are from Trio64V2/DX based cards

	// S3 86C765 Video BIOS. Version 1.03-08N Copyright 1996 S3 Incorporated.
	ROM_SYSTEM_BIOS( 1, "trio64v2_765", "PCI S3 86C765 v1.03-08N (S3 Trio64V2/DX)" )
	ROMX_LOAD("pci_9503-62_s3.bin", 0x00000, 0x8000, CRC(0e9d79d8) SHA1(274b5b98cc998f2783567000cdb12b14308bc290), ROM_BIOS(1) )

	// ELSA WINNER 1000/T2D VBE 2.0 DDC2B DPMS Video BIOS  Ver. 6.01.00  (IP/-/-) Copyright (c) 1994-97 ELSA GmbH, Aachen (Germany) All Rights Reserved
	ROM_SYSTEM_BIOS( 2, "winner1k", "PCI Elsa Winner 1000/T2D 6.01.00 (S3 Trio64V2/DX)" )
	ROMX_LOAD("pci_elsa_winner_1000-t2d_6.01.00.bin", 0x00000, 0x8000, CRC(1c9532b8) SHA1(d27d60b9a3566aa42a01ad497046af16eaa2ed87), ROM_BIOS(2) )

	// S3 86C775/86C785 Video BIOS. Version 1.01.04 Copyright 1996 S3 Incorporated.
	ROM_SYSTEM_BIOS( 3, "trio64v2_775", "PCI S3 86C775/86C785 v1.01.04 (S3 Trio64V2/DX)" )
	ROMX_LOAD("utd88a.bin", 0x00000, 0x8000, CRC(8df4524d) SHA1(e652dd2cec49d5edf3cb207e410233963efa22b8), ROM_BIOS(3) )

	// S3 86C775/86C785 Video BIOS. Version 1.01.04 775 EDO M50-02
	ROM_SYSTEM_BIOS( 4, "exprtclr_775", "PCI ExpertColor M50-02 (Trio64V2/DX, 86C775)" )
	ROMX_LOAD("s3_86c775-86c785_video_bios_v1.01.04.u5", 0x00000, 0x8000, CRC(e718418f) SHA1(1288ce51bb732a346eb7c61d5bdf80ea22454d45), ROM_BIOS(4) )
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
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update(m_vga, FUNC(s3trio64_vga_device::screen_update));

	S3_TRIO64_VGA(config, m_vga, 0);
	m_vga->set_screen("screen");
	m_vga->set_vram_size(0x100000);
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
	m_vga(*this, "vga"), m_8514(*this, "vga:8514a")
{
}

void isa16_svga_s3_device::io_isa_map(address_map &map)
{
	map(0x00, 0x2f).m(m_vga, FUNC(s3trio64_vga_device::io_map));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
uint8_t isa16_svga_s3_device::input_port_0_r() { return 0xff; } //return machine().root_device().ioport("IN0")->read(); }

void isa16_svga_s3_device::device_start()
{
	set_isa_device();

	m_isa->install_rom(this, 0xc0000, 0xc7fff, "s3_764");

	m_isa->install_device(0x03b0, 0x03df, *this, &isa16_svga_s3_device::io_isa_map);

	m_isa->install16_device(0x82e8, 0x82eb, read16smo_delegate(*m_8514, FUNC(ibm8514a_device::ibm8514_currenty_r)), write16smo_delegate(*m_8514, FUNC(ibm8514a_device::ibm8514_currenty_w)));
	m_isa->install16_device(0x86e8, 0x86eb, read16smo_delegate(*m_8514, FUNC(ibm8514a_device::ibm8514_currentx_r)), write16smo_delegate(*m_8514, FUNC(ibm8514a_device::ibm8514_currentx_w)));
	m_isa->install16_device(0x8ae8, 0x8aeb, read16smo_delegate(*m_8514, FUNC(ibm8514a_device::ibm8514_desty_r)), write16smo_delegate(*m_8514, FUNC(ibm8514a_device::ibm8514_desty_w)));
	m_isa->install16_device(0x8ee8, 0x8eeb, read16smo_delegate(*m_8514, FUNC(ibm8514a_device::ibm8514_destx_r)), write16smo_delegate(*m_8514, FUNC(ibm8514a_device::ibm8514_destx_w)));
	m_isa->install16_device(0x92e8, 0x92eb, read16smo_delegate(*m_8514, FUNC(ibm8514a_device::ibm8514_line_error_r)), write16smo_delegate(*m_8514, FUNC(ibm8514a_device::ibm8514_line_error_w)));
	m_isa->install16_device(0x96e8, 0x96eb, read16smo_delegate(*m_8514, FUNC(ibm8514a_device::ibm8514_width_r)), write16smo_delegate(*m_8514, FUNC(ibm8514a_device::ibm8514_width_w)));
	m_isa->install16_device(0x9ae8, 0x9aeb, read16smo_delegate(*m_8514, FUNC(ibm8514a_device::ibm8514_gpstatus_r)), write16smo_delegate(*m_8514, FUNC(ibm8514a_device::ibm8514_cmd_w)));
	m_isa->install16_device(0x9ee8, 0x9eeb, read16smo_delegate(*m_8514, FUNC(ibm8514a_device::ibm8514_ssv_r)), write16smo_delegate(*m_8514, FUNC(ibm8514a_device::ibm8514_ssv_w)));
	m_isa->install16_device(0xa2e8, 0xa2eb, read16smo_delegate(*m_8514, FUNC(ibm8514a_device::ibm8514_bgcolour_r)), write16smo_delegate(*m_8514, FUNC(ibm8514a_device::ibm8514_bgcolour_w)));
	m_isa->install16_device(0xa6e8, 0xa6eb, read16smo_delegate(*m_8514, FUNC(ibm8514a_device::ibm8514_fgcolour_r)), write16smo_delegate(*m_8514, FUNC(ibm8514a_device::ibm8514_fgcolour_w)));
	m_isa->install16_device(0xaae8, 0xaaeb, read16smo_delegate(*m_8514, FUNC(ibm8514a_device::ibm8514_write_mask_r)), write16smo_delegate(*m_8514, FUNC(ibm8514a_device::ibm8514_write_mask_w)));
	m_isa->install16_device(0xaee8, 0xaeeb, read16smo_delegate(*m_8514, FUNC(ibm8514a_device::ibm8514_read_mask_r)), write16smo_delegate(*m_8514, FUNC(ibm8514a_device::ibm8514_read_mask_w)));
	m_isa->install16_device(0xb6e8, 0xb6eb, read16smo_delegate(*m_8514, FUNC(ibm8514a_device::ibm8514_backmix_r)), write16smo_delegate(*m_8514, FUNC(ibm8514a_device::ibm8514_backmix_w)));
	m_isa->install16_device(0xbae8, 0xbaeb, read16smo_delegate(*m_8514, FUNC(ibm8514a_device::ibm8514_foremix_r)), write16smo_delegate(*m_8514, FUNC(ibm8514a_device::ibm8514_foremix_w)));
	m_isa->install16_device(0xbee8, 0xbeeb, read16smo_delegate(*m_8514, FUNC(ibm8514a_device::ibm8514_multifunc_r)), write16smo_delegate(*m_8514, FUNC(ibm8514a_device::ibm8514_multifunc_w)));
	m_isa->install16_device(0xe2e8, 0xe2eb, read16sm_delegate(*m_8514, FUNC(ibm8514a_device::ibm8514_pixel_xfer_r)), write16sm_delegate(*m_8514, FUNC(ibm8514a_device::ibm8514_pixel_xfer_w)));

	m_isa->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_vga, FUNC(s3trio64_vga_device::mem_r)), write8sm_delegate(*m_vga, FUNC(s3trio64_vga_device::mem_w)));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa16_svga_s3_device::device_reset()
{
}
