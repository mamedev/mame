// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

S3 Vision 864 / 868 / 964 / 968

TODO:
- Add Vision868
- Add Trio32/Trio64, pillage roms from isa/svga_s3
- Make ViRGE to derive from here rather than reinventing the wheel

Notes:
- Some of these BIOSes are buggy in SDD VBETEST.EXE, doesn't return any video mode,
  Reportedly mirocrys (vision964) and no9fx771 (vision968) has this inconvenient.

**************************************************************************************************/

#include "emu.h"
#include "vision.h"

#define LOG_WARN      (1U << 1)

#define VERBOSE (LOG_GENERAL | LOG_WARN)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)


DEFINE_DEVICE_TYPE(VISION864_PCI, vision864_pci_device,   "vision864",   "S3 86C864 Vision864")
// Vision868
DEFINE_DEVICE_TYPE(VISION964_PCI, vision964_pci_device,   "vision964",   "S3 86C964 Vision964")
DEFINE_DEVICE_TYPE(VISION968_PCI, vision968_pci_device,   "vision968",   "S3 86C968 Vision968")



vision864_pci_device::vision864_pci_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
	, m_vga(*this, "vga")
	, m_bios(*this, "bios")
{
}

vision864_pci_device::vision864_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vision864_pci_device(mconfig, VISION864_PCI, tag, owner, clock)
{
	// device IDs:
	// 88c0 = 86c864 DRAM v0
	// 88c1 = 86c864 DRAM v1
	// 88c2 = 86c864-P DRAM v2
	// 88c3 = 86c864-P DRAM v3
	// NOTE: class code = 0 (backward compatible VGA device)
	set_ids(0x533388c1, 0x00, 0x000100, 0x00000000);
}

ROM_START( vision864 )
	ROM_REGION32_LE( 0x8000, "bios", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "vision864", "Phoenix S3 Vision864 1.04-01" )
	ROMX_LOAD( "bios.bin", 0x0000, 0x8000, CRC(791c9e0d) SHA1(340a64402958d2ee734d929dfce147d9afcf23f4), ROM_BIOS(0) )
	ROM_IGNORE( 0x8000 )
ROM_END

const tiny_rom_entry *vision864_pci_device::device_rom_region() const
{
	return ROM_NAME(vision864);
}

void vision864_pci_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update("vga", FUNC(s3vision864_vga_device::screen_update));

	S3_VISION864_VGA(config, m_vga, 0);
	m_vga->set_screen("screen");
	// 1MB, option for 2MB
	m_vga->set_vram_size(2*1024*1024);
}

void vision864_pci_device::device_start()
{
	pci_card_device::device_start();

//  add_map(64 * 1024 * 1024, M_MEM | M_DISABLED, FUNC(vision864_pci_device::lfb_map));
//  set_map_address(0, 0x70000000);

	add_rom((u8 *)m_bios->base(), 0x8000);
	expansion_rom_base = 0xc0000;

	// TODO: can't read the intr pin reg but still has an INTA#
}

void vision864_pci_device::device_reset()
{
	pci_card_device::device_reset();

	command = 0x0020;
	command_mask = 0x23;
	// Medium DEVSEL
	status = 0x0200;

	remap_cb();
}

void vision864_pci_device::legacy_io_map(address_map &map)
{
	map(0, 0x02f).m(m_vga, FUNC(s3vision864_vga_device::io_map));
}

uint8_t vision864_pci_device::vram_r(offs_t offset)
{
	return downcast<s3vision864_vga_device *>(m_vga.target())->mem_r(offset);
}

void vision864_pci_device::vram_w(offs_t offset, uint8_t data)
{
	downcast<s3vision864_vga_device *>(m_vga.target())->mem_w(offset, data);
}

void vision864_pci_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	if (BIT(command, 1))
	{
		memory_space->install_readwrite_handler(0xa0000, 0xbffff, read8sm_delegate(*this, FUNC(vision864_pci_device::vram_r)), write8sm_delegate(*this, FUNC(vision864_pci_device::vram_w)));
	}

	if (BIT(command, 0))
	{
		io_space->install_device(0x03b0, 0x03df, *this, &vision864_pci_device::legacy_io_map);
	}
}

/******************
 *
 * Vision964
 *
 *****************/

vision964_pci_device::vision964_pci_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: vision864_pci_device(mconfig, type, tag, owner, clock)
{
}

vision964_pci_device::vision964_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vision964_pci_device(mconfig, VISION964_PCI, tag, owner, clock)
{
	// device IDs:
	// 88d0-88d1 = 86c964 VRAM v0-1
	// 88d2-88d3 = 86c964-P VRAM v2-3
	// NOTE: class code = 0 (backward compatible VGA device)
	set_ids(0x533388d0, 0x00, 0x000100, 0x00000000);
}

ROM_START( vision964 )
	ROM_REGION32_LE( 0x8000, "bios", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("mirocrys")

	ROM_SYSTEM_BIOS( 0, "mirocrys", "miroCRYSTAL Rev.2.13" )
	ROMX_LOAD( "mirocrystal.vbi", 0x0000, 0x8000, CRC(d0b0aa1c) SHA1(004e2432c4783f1539a7989e7d9ee422df09e695), ROM_BIOS(0) )
ROM_END

const tiny_rom_entry *vision964_pci_device::device_rom_region() const
{
	return ROM_NAME(vision964);
}

void vision964_pci_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update("vga", FUNC(s3vision964_vga_device::screen_update));

	S3_VISION964_VGA(config, m_vga, 0);
	m_vga->set_screen("screen");
	// 2MB/4MB/8MB
	m_vga->set_vram_size(4*1024*1024);
}


/******************
 *
 * Vision968
 *
 *****************/

vision968_pci_device::vision968_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vision964_pci_device(mconfig, VISION968_PCI, tag, owner, clock)
{
	// device IDs:
	// 88f0-88f3 = 86c968 RAM v0-3
	// first device to actually have a real class code
	set_ids(0x533388f0, 0x00, 0x030000, 0x00000000);
}

ROM_START( vision968 )
	ROM_REGION32_LE( 0x8000, "bios", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("elsaw2k")

	ROM_SYSTEM_BIOS( 0, "no9fx771", "Number Nine 9FX MotionFX 771 v2.45.11" )
	ROMX_LOAD( "no9motionfx771.bin", 0x0000, 0x8000, CRC(7732e382) SHA1(9ec2fe056712cef39bd8380d406be3c874ea5ec9), ROM_BIOS(0) )
	ROM_IGNORE( 0x8000 )

	ROM_SYSTEM_BIOS( 1, "elsaw2k", "Elsa Winner 2000Pro/X-8 v1.21.01-B" )
	ROMX_LOAD( "elsaw20008m.bin", 0x0000, 0x8000, CRC(47563211) SHA1(f51a40956c3e6e7c86851d81f81ba5f77509d361), ROM_BIOS(1) )
	ROM_IGNORE( 0x8000 )

	ROM_SYSTEM_BIOS( 2, "speamp64", "SPEA/Videoseven V7-Mercury P-64 v1.01-08" )
	ROMX_LOAD( "spea.bin",     0x0000, 0x8000, CRC(2caeadaf) SHA1(236829f1e6065a2f0ebee91f71891d8402f0ab5a), ROM_BIOS(2) )
	ROM_IGNORE( 0x8000 )
ROM_END

const tiny_rom_entry *vision968_pci_device::device_rom_region() const
{
	return ROM_NAME(vision968);
}

void vision968_pci_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update("vga", FUNC(s3vision968_vga_device::screen_update));

	S3_VISION968_VGA(config, m_vga, 0);
	m_vga->set_screen("screen");
	// 2MB/4MB/8MB
	m_vga->set_vram_size(4*1024*1024);
//  m_vga->linear_config_changed().set(FUNC(s3vision864_vga_device::linear_config_changed_w));
}

void vision968_pci_device::device_start()
{
	pci_card_device::device_start();

//  add_map(64 * 1024 * 1024, M_MEM | M_DISABLED, FUNC(vision968_pci_device::lfb_map));
	add_map(64 * 1024 * 1024, M_MEM, FUNC(vision968_pci_device::lfb_map));
	set_map_address(0, 0x70000000);

	add_rom((u8 *)m_bios->base(), 0x8000);
	expansion_rom_base = 0xc0000;

	// INTA#
	intr_pin = 1;
}

void vision968_pci_device::device_reset()
{
	pci_card_device::device_reset();

	command = 0x0020;
	command_mask = 0x23;
	// Adds fast back-to-back
	status = 0x0280;

	remap_cb();
}

// TODO: 0x0200'0000 "mirror" (really an endian relocation?)
void vision968_pci_device::lfb_map(address_map &map)
{
	map(0x0000'0000, 0x00ff'ffff).rw(m_vga, FUNC(s3vision864_vga_device::mem_linear_r), FUNC(s3vision864_vga_device::mem_linear_w));
//  map(0x0100'0000, 0x0100'7fff) image transfer data
	map(0x0100'8000, 0x0100'803f).m(FUNC(vision968_pci_device::config_map));
//  map(0x0100'8100, 0x0100'816f) packed copro regs
//  map(0x0100'82e8, 0x0100'82e8) current ypos
//  map(0x0100'82ea, 0x0100'82ea) current ypos-2
	map(0x0100'83b0, 0x0100'83df).m(m_vga, FUNC(s3vision968_vga_device::io_map));
//  map(0x0100'8502, 0x0100'8502) (VGA $0102 alias)
//  map(0x0100'8504, 0x0100'8504) (VGA $42e8 alias)
//  map(0x0100'8508, 0x0100'8508) (VGA $46e8 alias)
//  map(0x0100'850c, 0x0100'850c) (VGA $4ae8 alias)
//  map(0x0100'86e8, 0x0100'8eea) PnP copro region
//  map(0x0101'0000, 0x0101'3fff) Pixel formatter data transfer
//  map(0x0101'4000, 0x0101'7fff) Pixel formatter Mask data
//  map(0x0101'8080, 0x0101'809f) Pixel formatter regs
}

void vision968_pci_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	vision964_pci_device::map_extra(
		memory_window_start, memory_window_end, memory_offset, memory_space,
		io_window_start, io_window_end, io_offset, io_space
	);
	// TODO: new MMIO goes here
}
