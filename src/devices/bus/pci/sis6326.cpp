// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

SiS 6326

**************************************************************************************************/

#include "emu.h"
#include "sis6326.h"

#define LOG_WARN      (1U << 1)
#define LOG_AGP       (1U << 2)

#define VERBOSE (LOG_GENERAL | LOG_WARN | LOG_AGP)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)
#define LOGAGP(...)             LOGMASKED(LOG_AGP, __VA_ARGS__)


DEFINE_DEVICE_TYPE(SIS6326_AGP, sis6326_agp_device,   "sis6326_agp",   "SiS 6326 AGP card")



sis6326_agp_device::sis6326_agp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
	, m_vga(*this, "vga")
	, m_vga_rom(*this, "vga_rom")
{
	set_ids(0x10396326, 0xa0, 0x030000, 0x10396326);
}

sis6326_agp_device::sis6326_agp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sis6326_agp_device(mconfig, SIS6326_AGP, tag, owner, clock)
{
}

ROM_START( sis6326agp )
	ROM_REGION32_LE( 0x10000, "vga_rom", ROMREGION_ERASEFF )
	// Default is AOpen for supporting OpenGL
	ROM_DEFAULT_BIOS("aopen")

	ROM_SYSTEM_BIOS( 0, "aopen", "AOpen PA50V 2.25" )
	ROMX_LOAD( "aopenpa50v.vbi", 0x000000, 0x008000, CRC(6c4c7518) SHA1(36bb29a23d565e34d548701acc248d50c99d7da4), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "3dpro", "3DPro 4MB EDO 1.06" )
	ROMX_LOAD( "3dpro4mbedo.bin", 0x000000, 0x010000, CRC(f04b374c) SHA1(6eb96f7517df6eb566c615c2ab9ec5567035b6a5), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "siso", "SiS6326 4MB 1.21d" )
	ROMX_LOAD( "4mbsdr.vbi",   0x000000, 0x00c000, CRC(a5f8c7f7) SHA1(9e5cfcf8d34e0c5829343179c87fb2b97f7a7f9c), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "sis", "SiS6326 4MB 1.25" )
	ROMX_LOAD( "sis6326agp.bin", 0x000000, 0x010000, CRC(a671255c) SHA1(332ab9499142e8f7a235afe046e8e8d21a28580d), ROM_BIOS(3) )

	// basically identical to siso
//  ROM_SYSTEM_BIOS( 3, "sisvbi", "SiS6326 4MB PCI 1.21d" )
//  ROMX_LOAD( "4mb_pci.vbi",  0x000000, 0x00c000, CRC(8fca47be) SHA1(7ce995ec0d8b9ac4f0f40ccd0a61d7fc209d313f), ROM_BIOS(3) )
ROM_END

const tiny_rom_entry *sis6326_agp_device::device_rom_region() const
{
	return ROM_NAME(sis6326agp);
}

void sis6326_agp_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update(m_vga, FUNC(sis6236_vga_device::screen_update));

	// TODO: barely similar, to be changed.
	SIS6236_VGA(config, m_vga, 0);
	m_vga->set_screen("screen");
	// 4MB, max 8MB
	m_vga->set_vram_size(4*1024*1024);
}

void sis6326_agp_device::device_start()
{
	pci_card_device::device_start();

	add_map(4*1024*1024, M_MEM, FUNC(sis6326_agp_device::vram_aperture_map));
	add_map(64*1024, M_MEM, FUNC(sis6326_agp_device::mmio_map));
	add_map(16, M_IO, FUNC(sis6326_agp_device::vmi_map));

	add_rom((u8 *)m_vga_rom->base(), 0x10000);
	expansion_rom_base = 0xc0000;

	// INTA#
	// TODO: SRE D3 can strap this to no irq pin
	intr_pin = 1;
}

void sis6326_agp_device::device_reset()
{
	pci_card_device::device_reset();

	// doc makes multiple ninja jumps in messing up these defaults
	// bus master (hardwired)
	command = 0x0004;
	command_mask = 0x23;
	// medium DEVSEL#, 66 MHz capable, has capabilities list
	status = 0x0230;

	remap_cb();
}

u8 sis6326_agp_device::capptr_r()
{
	return 0x50;
}

u32 sis6326_agp_device::agp_command_r(offs_t offset, uint32_t mem_mask)
{
	LOGAGP("Read AGP command [$58] %d %d %08x\n", m_agp.enable, m_agp.data_rate, mem_mask);
	// TODO: enable gets cleared by AGP_RESET, or even from PCI RST#
	return m_agp.enable << 8 | (m_agp.data_rate & 7);
}

void sis6326_agp_device::agp_command_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGAGP("Write AGP command [$c8] %08x & %08x\n", data, mem_mask);

	if (ACCESSING_BITS_8_15)
	{
		m_agp.enable = bool(BIT(m_agp.enable, 8));
		LOGAGP("- AGP_ENABLE = %d\n", m_agp.enable);
	}

	if (ACCESSING_BITS_0_7)
	{
		// quick checker, to be translated into an AGP interface
		std::map<u8, std::string> agp_transfer_rates = {
			{ 0, "(illegal 0)" },
			{ 1, "1X" },
			{ 2, "2X" },
			{ 3, "(illegal 3)" }
		};

		// make sure the AGP DATA_RATE specs are honored
		const u8 data_rate = data & 3;
		LOGAGP("- DATA_RATE = %s enabled=%d\n", agp_transfer_rates.at(data_rate), m_agp.enable);
		m_agp.data_rate = data_rate;
	}
}

void sis6326_agp_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);
	// AGP
	map(0x50, 0x53).lr32(NAME([] () { return 0x00105c02; } ));
	map(0x54, 0x57).lr32(NAME([] () { return 0x01000003; } ));
	map(0x58, 0x5b).rw(FUNC(sis6326_agp_device::agp_command_r), FUNC(sis6326_agp_device::agp_command_w));
	map(0x5c, 0x5f).lr32(NAME([] () { return 0x00000000; } )); // NULL terminator
}

void sis6326_agp_device::vram_aperture_map(address_map &map)
{
	map(0x0000000, 0x3ffffff).rw(m_vga, FUNC(sis6236_vga_device::mem_linear_r), FUNC(sis6236_vga_device::mem_linear_w));
}

void sis6326_agp_device::mmio_map(address_map &map)
{
}

void sis6326_agp_device::vmi_map(address_map &map)
{
}

// TODO: this should really be a subclass of VGA
void sis6326_agp_device::legacy_memory_map(address_map &map)
{
	map(0xa0000, 0xbffff).rw(FUNC(sis6326_agp_device::vram_r), FUNC(sis6326_agp_device::vram_w));
}

void sis6326_agp_device::legacy_io_map(address_map &map)
{
	map(0, 0x02f).m(m_vga, FUNC(sis6236_vga_device::io_map));
}

uint8_t sis6326_agp_device::vram_r(offs_t offset)
{
	return downcast<sis6236_vga_device *>(m_vga.target())->mem_r(offset);
}

void sis6326_agp_device::vram_w(offs_t offset, uint8_t data)
{
	downcast<sis6236_vga_device *>(m_vga.target())->mem_w(offset, data);
}

void sis6326_agp_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	if (BIT(command, 1))
	{
		memory_space->install_readwrite_handler(0xa0000, 0xbffff, read8sm_delegate(*this, FUNC(sis6326_agp_device::vram_r)), write8sm_delegate(*this, FUNC(sis6326_agp_device::vram_w)));
	}

	if (BIT(command, 0))
	{
		io_space->install_device(0x03b0, 0x03df, *this, &sis6326_agp_device::legacy_io_map);
	}
}
