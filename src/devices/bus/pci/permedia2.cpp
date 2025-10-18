// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************


**************************************************************************************************/

#include "emu.h"
#include "permedia2.h"

#define LOG_WARN      (1U << 1)

#define VERBOSE (LOG_GENERAL | LOG_WARN)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)


DEFINE_DEVICE_TYPE(PERMEDIA2, permedia2_device,   "permedia2",   "3Dlabs Permedia 2")

permedia2_device::permedia2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
	, m_vga(*this, "vga")
	, m_vga_rom(*this, "vga_rom")
{
    // TODO: can change class code
	set_ids(0x104c3d07, 0x01, 0x030100, 0x104c3d07);
}

permedia2_device::permedia2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: permedia2_device(mconfig, PERMEDIA2, tag, owner, clock)

{
}

ROM_START( permedia2 )
	ROM_REGION32_LE( 0x20000, "vga_rom", ROMREGION_ERASEFF )
    // TODO: 4 other dumps available on VGALegacyMK3, separate Sun versions to own subclass
	ROM_SYSTEM_BIOS( 0, "creative",   "03/03/98 Creative Graphics Blaster Exxtreme" )
	ROMX_LOAD( "creative.vbi",  0x00000, 0x08000, CRC(3e622da7) SHA1(b06f89777badfdb7cd4c76b414ba19fcaed243c9), ROM_BIOS(0) )
	// Tech-Source Inc. Raptor GFX
	ROM_SYSTEM_BIOS( 1, "sparc_1_60", "18/May/2000 (Rev 1.60) for Solaris SPARC" )
	ROMX_LOAD( "raptor160.u10", 0x00000, 0x20000, CRC(8d6706d5) SHA1(6df6719aa8f46176d837b1b05e90f7e541416b4c), ROM_BIOS(1) )
	// Tech-Source Inc. Raptor GFX // Half-size ROM?
	ROM_SYSTEM_BIOS( 2, "sparc_1_11", "07/Jun/1999 (Rev 1.11) for Solaris SPARC" )
	ROMX_LOAD( "raptor111.bin", 0x00000, 0x10000, CRC(ee21d1f4) SHA1(04845a2e0938b9ecd88934c148e637e6f00e2578), ROM_BIOS(2) )
	// Tech-Source Inc. Raptor GFX
	ROM_SYSTEM_BIOS( 3, "sparc_1_10", "15/Mar/1999 (Rev 1.10) for Solaris SPARC" )
	ROMX_LOAD( "raptor110.bin", 0x00000, 0x20000, CRC(6a6f4ba4) SHA1(89df1a7bc52693e34b79587c16f2c2efb30bd3f1), ROM_BIOS(3) )
	// Phoenix / 3DLabs generic BIOS for IBM PC Compatible systems
	ROM_SYSTEM_BIOS( 4, "pc_99",      "25/Feb/1999 for IBM PC Compatible" )
	ROMX_LOAD( "permed2pc.bin", 0x00000, 0x20000, CRC(6711ddf8) SHA1(9f02e4d1a64c42e4a17df8428d8cc9a72e78e0d5), ROM_BIOS(4) )

ROM_END

const tiny_rom_entry *permedia2_device::device_rom_region() const
{
	return ROM_NAME(permedia2);
}


void permedia2_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update(m_vga, FUNC(vga_device::screen_update));

	// TODO: bump to TVP4020 core
	VGA(config, m_vga, 0);
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

	add_rom((u8 *)m_vga_rom->base(), 128*1024 );
	expansion_rom_base = 0xc0000;

	// INTA#
	intr_pin = 1;
}

void permedia2_device::device_reset()
{
	pci_card_device::device_reset();

	// TODO: to be checked
	command = 0x0000;
	command_mask = 0x0027;
	status = 0x0000;

	remap_cb();
}

u8 permedia2_device::capptr_r()
{
	return 0x40;
}

void permedia2_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);
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
	map(0, 0x02f).m(m_vga, FUNC(vga_device::io_map));
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
	if (command & 7)
	{
		memory_space->install_readwrite_handler(0xa0000, 0xbffff, read8sm_delegate(*this, FUNC(permedia2_device::vram_r)), write8sm_delegate(*this, FUNC(permedia2_device::vram_w)));

		io_space->install_device(0x03b0, 0x03df, *this, &permedia2_device::legacy_io_map);
		//memory_space->install_rom(0xc0000, 0xcffff, (void *)expansion_rom);
	}
}

