// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

3Dlabs Permedia 2

TODO:
- placeholder for roms (non-SDD );
- bios=creative black screen when mounted;
- bios=elsaper2 wants something from COM3 at $2e8 (?), writes subvendor ID swapped up
  (PCI database bug, revision ID meme or really somehow expects a swapped write once?);

**************************************************************************************************/

#include "emu.h"
#include "permedia2.h"

#define LOG_WARN      (1U << 1)

#define VERBOSE (LOG_GENERAL | LOG_WARN)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)

// permedia original BIOS, TBD (PCI, has 2 extra BARs)
//ROM_START( permedia )
//	ROM_REGION32_LE( 0x8000, "bios", ROMREGION_ERASE00 )
//	VGA Legacy MKIII dump (black screen like creative BIOS)
//	ROM_SYSTEM_BIOS( 0, "c3000",   "PowerColor C3000 Rev. 1.0" )
//	ROMX_LOAD( "permedia-c3000.vbi", 0x0000, 0x8000, CRC(97f80ddf) SHA1(f370aa17239a340d415e1ca20cdd2c982f365f04), ROM_BIOS(0) )
	// The Retro Web "dataexpert pm3040 pci (permedia 3000).bin" dump (black screen, bad dump?)
//	ROMX_LOAD( "dataexpert pm3040 pci.bin",  0x00000, 0x06600, BAD_DUMP CRC(763f016b) SHA1(e25b58cd532e5872be303439168330bd5e2641b6), ROM_BIOS(0) )
//ROM_END


DEFINE_DEVICE_TYPE(PERMEDIA2,  permedia2_device,    "permedia2",    "3Dlabs Permedia 2")
DEFINE_DEVICE_TYPE(PERMEDIA2V, permedia2v_device,   "permedia2v",   "3Dlabs Permedia 2V")

permedia2_device::permedia2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
	, m_vga(*this, "vga")
	, m_bios(*this, "bios")
{
}

permedia2_device::permedia2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: permedia2_device(mconfig, PERMEDIA2, tag, owner, clock)
{
	// TODO: can change class code, revision is unverified
	// 1048 0a32 GLoria Synergy
	// 1092 0126 Fire GL 1000 Pro
	// 1102 100f Graphics Blaster Extreme
	// 3d3d 0100 Reference Permedia 2 3D (bios=ms4413 and permedia2v,bios=accstar does this)
	set_ids(0x104c3d07, 0x01, 0x030100, 0x104c3d07);
}

ROM_START( permedia2 )
	ROM_REGION32_LE( 0x20000, "bios", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("diamond")
	ROM_SYSTEM_BIOS( 0, "creative",   "Creative Graphics Blaster Exxtreme (CT6610 03/03/98)" )
	ROMX_LOAD( "creative.vbi",  0x00000, 0x08000, CRC(3e622da7) SHA1(b06f89777badfdb7cd4c76b414ba19fcaed243c9), ROM_BIOS(0) )
	// 1xxxxxxxxxxxxxxx = 0xFF
	ROM_SYSTEM_BIOS( 1, "diamond",    "Diamond Multimedia Systems Fire GL 1000 Pro (1.36 10/02/97)" )
	ROMX_LOAD( "diamond.bin",   0x00000, 0x10000, CRC(57f869a8) SHA1(26cee5fa649a77799068df3a21a5dc076146f1f7), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "elsaper2",   "ELSA GLoria Synergy (Ver. 8.04.01)" )
	ROMX_LOAD( "elsaper2.vbi",  0x00000, 0x8000, CRC(e40605fc) SHA1(da13a1b7b27cc764ac7bc953291805408bdaaf66), ROM_BIOS(2) )
	// generic BIOS, OEM?
	ROM_SYSTEM_BIOS( 3, "ms4413",     "3Dlabs Enhanced Permedia BIOS (v1.38 11/21/97)" )
	ROMX_LOAD( "ms4413.bin",  0x00000, 0x8000, CRC(096fbf4a) SHA1(0246117e5d513dfc1e1fba309c2234cc486c1571), ROM_BIOS(3) )
	// Tech-Source Inc. Raptor GFX
	// TODO: 1.60 has Texas Instruments PCIR but 1.1x has 3DLabs
	ROM_SYSTEM_BIOS( 4, "sparc_1_60", "Tech-Source Raptor GFX for Solaris SPARC (18/May/2000 Rev 1.60)" )
	ROMX_LOAD( "raptor160.u10", 0x00000, 0x20000, CRC(8d6706d5) SHA1(6df6719aa8f46176d837b1b05e90f7e541416b4c), ROM_BIOS(4) )
	// Tech-Source Inc. Raptor GFX // Half-size ROM?
	ROM_SYSTEM_BIOS( 5, "sparc_1_11", "Tech-Source Raptor GFX for Solaris SPARC (07/Jun/1999 Rev 1.11)" )
	ROMX_LOAD( "raptor111.bin", 0x00000, 0x10000, CRC(ee21d1f4) SHA1(04845a2e0938b9ecd88934c148e637e6f00e2578), ROM_BIOS(5) )
	// Tech-Source Inc. Raptor GFX
	ROM_SYSTEM_BIOS( 6, "sparc_1_10", "Tech-Source Raptor GFX for Solaris SPARC (15/Mar/1999 Rev 1.10)" )
	ROMX_LOAD( "raptor110.bin", 0x00000, 0x20000, CRC(6a6f4ba4) SHA1(89df1a7bc52693e34b79587c16f2c2efb30bd3f1), ROM_BIOS(6) )
ROM_END

const tiny_rom_entry *permedia2_device::device_rom_region() const
{
	return ROM_NAME(permedia2);
}

void permedia2_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen"));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update(m_vga, FUNC(vga_device::screen_update));

	// TODO: bump to TVP4020 core
	VGA(config, m_vga);
	m_vga->set_screen("screen");
	// either 4 or 8MB
	m_vga->set_vram_size(8*1024*1024);

}

void permedia2_device::device_start()
{
	pci_card_device::device_start();

	add_map(   128*1024, M_MEM, FUNC(permedia2_device::control_map));
	add_map(8*1024*1024, M_MEM, FUNC(permedia2_device::aperture1_map));
	add_map(8*1024*1024, M_MEM, FUNC(permedia2_device::aperture2_map));

	// TODO: whatever the SPARC ROMs have as extra is TBD in their own overrides
	// Anything beyond 64*1024 is blank anyway
	add_rom((u8 *)m_bios->base(), 32*1024 );
	expansion_rom_base = 0xc0000;

	// INTA#
	intr_pin = 1;

	save_item(NAME(m_subsystem_logger_mask));
}

void permedia2_device::device_reset()
{
	pci_card_device::device_reset();

	// bus master capable
	command = 0x0000;
	command_mask = 0x0027;
	// Fast back-to-back, medium DEVSEL#
	status = 0x0280;

	// TODO: both can be 0x00 / 0x40 / 0x80 / 0xc0 (needs lspci to tell)
	minimum_grant = 0x00;
	maximum_latency = 0x00;

	m_subsystem_logger_mask = 0;

	remap_cb();
}

// either write once or strapped from ROM dword at $fffc
void permedia2_device::subvendor_w(offs_t offset, u32 data, u32 mem_mask)
{
	// write once
	if (m_subsystem_logger_mask & mem_mask)
	{
		logerror("Warning: subvendor ID possible rewrite! old=%08x & %08x data=%08x & %08x\n"
			, subsystem_id
			, m_subsystem_logger_mask
			, data
			, mem_mask
		);
		return;
	}
	m_subsystem_logger_mask |= mem_mask;

	COMBINE_DATA(&subsystem_id);
	logerror("subsystem ID write [$2c] %08x & %08x (%08x)\n", data, mem_mask, subsystem_id);
	// if we have a complete subvendor rewrite the subsystem_id
	// NOTE: it actually expects swapped words (vendor at 15:0, device at 31:16), cfr. pci.exe
	if (m_subsystem_logger_mask == 0xffff'ffff)
		subsystem_id = (subsystem_id >> 16) | ((subsystem_id & 0xffff) << 16);
}

u8 permedia2_device::capptr_r()
{
	return 0x40;
}

void permedia2_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);
	map(0x2c, 0x2d).r(FUNC(permedia2_device::subvendor_r));
	map(0x2e, 0x2f).r(FUNC(permedia2_device::subsystem_r));
	map(0x2c, 0x2f).w(FUNC(permedia2_device::subvendor_w));
	map(0x40, 0x43).lr32(NAME([] { return 0x0010'0002; }));
	// AGP 1x + SBA + 31 RQ
	map(0x44, 0x47).lr32(NAME([] { return 0x1f00'0201; }));

//  map(0xf8, 0xf8) CFGIndirectAddress
//  map(0xfc, 0xfc) CFGIndirectData
}

void permedia2_device::control_map(address_map &map)
{

}

void permedia2_device::aperture1_map(address_map &map)
{

}

void permedia2_device::aperture2_map(address_map &map)
{

}

void permedia2_device::legacy_memory_map(address_map &map)
{
	map(0xa0000, 0xbffff).rw(FUNC(permedia2_device::vram_r), FUNC(permedia2_device::vram_w));
}

void permedia2_device::legacy_io_map(address_map &map)
{
	map(0x03b0, 0x03df).m(m_vga, FUNC(vga_device::io_map));
}

uint8_t permedia2_device::vram_r(offs_t offset)
{
	return downcast<vga_device *>(m_vga.target())->mem_r(offset);
}

void permedia2_device::vram_w(offs_t offset, uint8_t data)
{
	downcast<vga_device *>(m_vga.target())->mem_w(offset, data);
}

void permedia2_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	if (BIT(command, 1))
		memory_space->install_readwrite_handler(0xa0000, 0xbffff, read8sm_delegate(*this, FUNC(permedia2_device::vram_r)), write8sm_delegate(*this, FUNC(permedia2_device::vram_w)));

	if (BIT(command, 0))
		io_space->install_device(0x0000, 0xffff, *this, &permedia2_device::legacy_io_map);
}

/*
 *
 * Permedia 2V overrides
 *
 */

permedia2v_device::permedia2v_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: permedia2_device(mconfig, PERMEDIA2V, tag, owner, clock)
{
	// TODO: can change class code, revision is unverified
	set_ids(0x3d3d0009, 0x01, 0x030100, 0x3d3d0009);
}

ROM_START( permedia2v )
	ROM_REGION32_LE( 0x8000, "bios", ROMREGION_ERASEFF )
	// Phoenix / 3DLabs generic BIOS for IBM PC Compatible systems
	// TODO: writes an unknown subsystem vendor ID 484c device ID 4133
	ROM_SYSTEM_BIOS( 0, "pc_99",      "Permedia 2 VGA (v2.2 02/25/99)" )
	ROMX_LOAD( "permed2pc.bin", 0x00000, 0x08000, CRC(6711ddf8) SHA1(9f02e4d1a64c42e4a17df8428d8cc9a72e78e0d5), ROM_BIOS(0) )
	ROM_IGNORE( 0x18000 )
	ROM_SYSTEM_BIOS( 1, "accstar",    "Viper-II AccelGraphics AccelSTAR IIv AGP (v1.44 03/02/98)" )
	ROMX_LOAD( "accstar.vbi",  0x00000, 0x08000, CRC(b67ca80e) SHA1(6bc6053183b7288453631440defd296196e6ea57), ROM_BIOS(1) )
ROM_END

const tiny_rom_entry *permedia2v_device::device_rom_region() const
{
	return ROM_NAME(permedia2v);
}
