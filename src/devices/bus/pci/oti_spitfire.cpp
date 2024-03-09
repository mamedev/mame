// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

OAK OTI Spitfire PCI cards

VBE 1.2, XGA

TODO:
- can strap base class and header to be multifunction

**************************************************************************************************/

#include "emu.h"
#include "oti_spitfire.h"

#define LOG_WARN      (1U << 1)

#define VERBOSE (LOG_GENERAL | LOG_WARN)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)


//OTI-64105
//OTI-64107
DEFINE_DEVICE_TYPE(OTI64111_PCI, oti64111_pci_device,   "oti64111_pci",   "OTI-64111 \"Spitfire\"")
//OTI-64217
//OTI-64317


oti64111_pci_device::oti64111_pci_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
	, m_svga(*this, "svga")
	, m_vga_rom(*this, "vga_rom")
{
	// Rev C
	set_ids(0x104e0111, 0x30, 0x030000, 0x104e0111);
}

oti64111_pci_device::oti64111_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: oti64111_pci_device(mconfig, OTI64111_PCI, tag, owner, clock)
{
}

ROM_START( oti64111 )
	ROM_REGION32_LE( 0x8000, "vga_rom", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "spitfire", "v111.20 10/10/96" )
	ROMX_LOAD( "ot64111spitfire.vbi", 0x0000, 0x8000, CRC(fb031e98) SHA1(16826d93bc2a09d91b52cca3f72447501daee2d2), ROM_BIOS(0) )
ROM_END

const tiny_rom_entry *oti64111_pci_device::device_rom_region() const
{
	return ROM_NAME(oti64111);
}

void oti64111_pci_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update(m_svga, FUNC(oak_oti111_vga_device::screen_update));

	OTI111(config, m_svga, 0);
	m_svga->set_screen("screen");
	m_svga->set_vram_size(4*1024*1024);
}

void oti64111_pci_device::device_start()
{
	pci_card_device::device_start();

	add_map(        256, M_MEM, FUNC(oti64111_pci_device::mmio_map));
	add_map(8*1024*1024, M_MEM, FUNC(oti64111_pci_device::vram_aperture_map));
	add_map(        256, M_IO,  FUNC(oti64111_pci_device::extio_map));

	add_rom((u8 *)m_vga_rom->base(), 0x8000);
	expansion_rom_base = 0xc0000;

	// INTA#
	intr_pin = 1;

	// TODO: (default?) min_gnt = 0xff, max_lat = 0x01
}

void oti64111_pci_device::device_reset()
{
	pci_card_device::device_reset();

	// enable address/data stepping, enable bus snoop for palette regs
	command = 0x00a0;
	// fast back-to-back, medium DEVSEL#
	status = 0x0280;

	remap_cb();
}

void oti64111_pci_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);
}

void oti64111_pci_device::mmio_map(address_map &map)
{
	map(0x00, 0x7f).rw(m_svga, FUNC(oak_oti111_vga_device::xga_read), FUNC(oak_oti111_vga_device::xga_write));
	map(0x80, 0xbf).m(m_svga, FUNC(oak_oti111_vga_device::multimedia_map));
}

void oti64111_pci_device::vram_aperture_map(address_map &map)
{
	map(0x000000, 0x7fffff).rw(m_svga, FUNC(oak_oti111_vga_device::mem_linear_r), FUNC(oak_oti111_vga_device::mem_linear_w));
}

void oti64111_pci_device::extio_map(address_map &map)
{
	map(0x00e0, 0x00ef).m(m_svga, FUNC(oak_oti111_vga_device::ramdac_mmio_map));
}

void oti64111_pci_device::legacy_io_map(address_map &map)
{
	map(0, 0x02f).m(m_svga, FUNC(oak_oti111_vga_device::io_map));
}

uint8_t oti64111_pci_device::vram_r(offs_t offset)
{
	return downcast<oak_oti111_vga_device *>(m_svga.target())->mem_r(offset);
}

void oti64111_pci_device::vram_w(offs_t offset, uint8_t data)
{
	downcast<oak_oti111_vga_device *>(m_svga.target())->mem_w(offset, data);
}

void oti64111_pci_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	if (BIT(command, 1))
	{
		memory_space->install_readwrite_handler(0xa0000, 0xbffff, read8sm_delegate(*this, FUNC(oti64111_pci_device::vram_r)), write8sm_delegate(*this, FUNC(oti64111_pci_device::vram_w)));
	}

	if (BIT(command, 0))
	{
		io_space->install_device(0x03b0, 0x03df, *this, &oti64111_pci_device::legacy_io_map);
	}
}
