// license:BSD-3-Clause
// copyright-holders:


#include "emu.h"
#include "trio_pci.h"

#include "screen.h"

DEFINE_DEVICE_TYPE(TRIO64DX_PCI, trio64dx_pci_device, "trio64dx_pci", "S3 86C775 Trio64V2/DX")

trio64dx_pci_device::trio64dx_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: trio64dx_pci_device(mconfig, TRIO64DX_PCI, tag, owner, clock)
{
}

trio64dx_pci_device::trio64dx_pci_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
	, m_vga(*this, "vga")
	, m_8514(*this, "vga:8514a")
	, m_bios(*this, "bios")
	, m_screen(*this, finder_base::DUMMY_TAG)
{
}

// Default is for convenience: europl01 literally uses that
ROM_START( trio64dx_pci )
	ROM_REGION(0x8000, "bios", 0)
	// S3 86C775/86C785 Video BIOS. Version 1.01.04 775 EDO M50-02
	ROM_SYSTEM_BIOS( 0, "exprtclr_775", "PCI ExpertColor M50-02 (Trio64V2/DX, 86C775)" )
	ROMX_LOAD("s3_86c775-86c785_video_bios_v1.01.04.u5", 0x00000, 0x8000, CRC(e718418f) SHA1(1288ce51bb732a346eb7c61d5bdf80ea22454d45), ROM_BIOS(0) )

	// S3 86C765 Video BIOS. Version 1.03-08N Copyright 1996 S3 Incorporated.
	ROM_SYSTEM_BIOS( 1, "trio64v2_765", "PCI S3 86C765 v1.03-08N (S3 Trio64V2/DX)" )
	ROMX_LOAD("pci_9503-62_s3.bin", 0x00000, 0x8000, CRC(0e9d79d8) SHA1(274b5b98cc998f2783567000cdb12b14308bc290), ROM_BIOS(1) )

	// ELSA WINNER 1000/T2D VBE 2.0 DDC2B DPMS Video BIOS  Ver. 6.01.00  (IP/-/-) Copyright (c) 1994-97 ELSA GmbH, Aachen (Germany) All Rights Reserved
	ROM_SYSTEM_BIOS( 2, "winner1k", "PCI Elsa Winner 1000/T2D 6.01.00 (S3 Trio64V2/DX)" )
	ROMX_LOAD("pci_elsa_winner_1000-t2d_6.01.00.bin", 0x00000, 0x8000, CRC(1c9532b8) SHA1(d27d60b9a3566aa42a01ad497046af16eaa2ed87), ROM_BIOS(2) )

	// S3 86C775/86C785 Video BIOS. Version 1.01.04 Copyright 1996 S3 Incorporated.
	ROM_SYSTEM_BIOS( 3, "trio64v2_775", "PCI S3 86C775/86C785 v1.01.04 (S3 Trio64V2/DX)" )
	ROMX_LOAD("utd88a.bin", 0x00000, 0x8000, CRC(8df4524d) SHA1(e652dd2cec49d5edf3cb207e410233963efa22b8), ROM_BIOS(3) )
ROM_END

const tiny_rom_entry *trio64dx_pci_device::device_rom_region() const
{
	return ROM_NAME( trio64dx_pci );
}

void trio64dx_pci_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update("vga", FUNC(s3trio64_vga_device::screen_update));

	S3_TRIO64_VGA(config, m_vga, 0);
	m_vga->set_screen("screen");
	// 1MB or 2MB
	m_vga->set_vram_size(2*1024*1024);
}

void trio64dx_pci_device::device_start()
{
	pci_card_device::device_start();

	set_ids(0x53338811, 0x00, 0x030000, 0x000000);

	// TODO: same as ViRGE
	add_map(4*1024*1024, M_MEM, FUNC(trio64dx_pci_device::mmio_map));

	add_rom(m_bios->base(),0x8000);
	expansion_rom_base = 0xc0000;

	command = 0x0000;
	// DAC SNP / MEM / I/O
	command_mask = 0x23;
	// medium DEVSELB#
	status = 0x0200;

	// INTA#
	intr_pin = 1;

	remap_cb();
}


void trio64dx_pci_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);
}

void trio64dx_pci_device::mmio_map(address_map &map)
{
	map(0x0000'0000, 0x001f'ffff).rw(m_vga, FUNC(s3trio64_vga_device::mem_linear_r), FUNC(s3trio64_vga_device::mem_linear_w));
}


void trio64dx_pci_device::io_map(address_map &map)
{
	map(0x03b0, 0x03df).m(m_vga, FUNC(s3trio64_vga_device::io_map));

	// TODO: has the usual 8514/a issues, consider rewrite
	map(0x82e8, 0x82eb).rw(m_8514, FUNC(ibm8514a_device::ibm8514_currenty_r), FUNC(ibm8514a_device::ibm8514_currenty_w));
	map(0x86e8, 0x86eb).rw(m_8514, FUNC(ibm8514a_device::ibm8514_currentx_r), FUNC(ibm8514a_device::ibm8514_currentx_w));
	map(0x8ae8, 0x8aeb).rw(m_8514, FUNC(ibm8514a_device::ibm8514_desty_r), FUNC(ibm8514a_device::ibm8514_desty_w));
	map(0x8ee8, 0x8eeb).rw(m_8514, FUNC(ibm8514a_device::ibm8514_destx_r), FUNC(ibm8514a_device::ibm8514_destx_w));
	map(0x92e8, 0x92eb).rw(m_8514, FUNC(ibm8514a_device::ibm8514_line_error_r), FUNC(ibm8514a_device::ibm8514_line_error_w));
	map(0x96e8, 0x96eb).rw(m_8514, FUNC(ibm8514a_device::ibm8514_width_r), FUNC(ibm8514a_device::ibm8514_width_w));
	map(0x9ae8, 0x9aeb).rw(m_8514, FUNC(ibm8514a_device::ibm8514_gpstatus_r), FUNC(ibm8514a_device::ibm8514_cmd_w));
	map(0x9ee8, 0x9eeb).rw(m_8514, FUNC(ibm8514a_device::ibm8514_ssv_r), FUNC(ibm8514a_device::ibm8514_ssv_w));
	map(0xa2e8, 0xa2eb).rw(m_8514, FUNC(ibm8514a_device::ibm8514_bgcolour_r), FUNC(ibm8514a_device::ibm8514_bgcolour_w));
	map(0xa6e8, 0xa6eb).rw(m_8514, FUNC(ibm8514a_device::ibm8514_fgcolour_r), FUNC(ibm8514a_device::ibm8514_fgcolour_w));
	map(0xaae8, 0xaaeb).rw(m_8514, FUNC(ibm8514a_device::ibm8514_write_mask_r), FUNC(ibm8514a_device::ibm8514_write_mask_w));
	map(0xaee8, 0xaeeb).rw(m_8514, FUNC(ibm8514a_device::ibm8514_read_mask_r), FUNC(ibm8514a_device::ibm8514_read_mask_w));
	map(0xb6e8, 0xb6eb).rw(m_8514, FUNC(ibm8514a_device::ibm8514_backmix_r), FUNC(ibm8514a_device::ibm8514_backmix_w));
	map(0xbae8, 0xbaeb).rw(m_8514, FUNC(ibm8514a_device::ibm8514_foremix_r), FUNC(ibm8514a_device::ibm8514_foremix_w));
	map(0xbee8, 0xbeeb).rw(m_8514, FUNC(ibm8514a_device::ibm8514_multifunc_r), FUNC(ibm8514a_device::ibm8514_multifunc_w));
	map(0xe2e8, 0xe2eb).rw(m_8514, FUNC(ibm8514a_device::ibm8514_pixel_xfer_r), FUNC(ibm8514a_device::ibm8514_pixel_xfer_w));

}

uint8_t trio64dx_pci_device::vram_r(offs_t offset)
{
	return downcast<s3trio64_vga_device *>(m_vga.target())->mem_r(offset);
}

void trio64dx_pci_device::vram_w(offs_t offset, uint8_t data)
{
	downcast<s3trio64_vga_device *>(m_vga.target())->mem_w(offset, data);
}

void trio64dx_pci_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	if (BIT(command, 1))
	{
		memory_space->install_readwrite_handler(0xa0000, 0xbffff, read8sm_delegate(*this, FUNC(trio64dx_pci_device::vram_r)), write8sm_delegate(*this, FUNC(trio64dx_pci_device::vram_w)));
	}

	if (BIT(command, 0))
	{
		io_space->install_device(0x0000, 0xffff, *this, &trio64dx_pci_device::io_map);
	}
}

