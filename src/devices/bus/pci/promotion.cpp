// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

Alliance Semiconductor ProMotion family

- aT3D is the 2d base of 3dfx Voodoo Rush on Hercules Stingray 128/3D cards at least
  (thru THP i/f, reportedly slow)
- On its own aT3D is infamously known to be a terrible 3D card
  cfr. https://www.youtube.com/watch?v=vCz-rSlSREA

TODO:
- Requires own family of VGA based devices;
- Catch is that said device mirrors the PCI space in memory mapped regs, along with the
  (more or less) typical SVGA regs of the mid-90s;
- Black screens when mounted;
- Bare documentation, covering pinout and register names only. aT3D datasheet available for
crosschecking;

**************************************************************************************************/

#include "emu.h"
#include "promotion.h"

#define LOG_WARN      (1U << 1)

#define VERBOSE (LOG_GENERAL | LOG_WARN)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)


DEFINE_DEVICE_TYPE(PROMOTION3210,   promotion3210_device,   "promotion3210",   "Alliance Semiconductor ProMotion 3210")
//DEFINE_DEVICE_TYPE(PROMOTION6410,   promotion6410_device,   "promotion6410",   "Alliance Semiconductor ProMotion 6410")
//DEFINE_DEVICE_TYPE(PROMOTION6422,   promotion6422_device,   "promotion6422",   "Alliance Semiconductor ProMotion 6422")
//DEFINE_DEVICE_TYPE(PROMOTION6424,   promotion6424_device,   "promotion6424",   "Alliance Semiconductor ProMotion 6424")
// Alias of above?
//DEFINE_DEVICE_TYPE(PROMOTIONAT24,   promotionat24_device,   "promotionat24",   "Alliance Semiconductor ProMotion aT24")
//DEFINE_DEVICE_TYPE(PROMOTIONAT3D,   promotionat3d_device,   "promotionat3d",   "Alliance Semiconductor ProMotion aT3D")
//DEFINE_DEVICE_TYPE(PROMOTIONAT25,   promotionat25_device,   "promotionat25",   "Alliance Semiconductor ProMotion aT25")


promotion3210_device::promotion3210_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
	, m_vga(*this, "vga")
	, m_vga_rom(*this, "vga_rom")
{
	// vendor ID 0x1142 Alliance Semiconductor Corporation
	// subvendor unknown
	set_ids(0x11423210, 0x00, 0x030000, 0x11423210);
}

promotion3210_device::promotion3210_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: promotion3210_device(mconfig, PROMOTION3210, tag, owner, clock)
{
}

ROM_START( promotion3210 )
	ROM_REGION32_LE( 0x8000, "vga_rom", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "miro", "miro Video 12PD" )
	ROMX_LOAD( "mirovideo12pd.vbi", 0x0000, 0x8000, CRC(46041709) SHA1(bd43f05ae7ddb4bbf515132b72b32719b60e6950), ROM_BIOS(0) )
ROM_END

const tiny_rom_entry *promotion3210_device::device_rom_region() const
{
	return ROM_NAME(promotion3210);
}

void promotion3210_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update(m_vga, FUNC(vga_device::screen_update));

	PROMOTION_VGA(config, m_vga, 0);
	m_vga->set_screen("screen");
	// TODO: configurable between 1 and 4 MB (2x EDO slots on board)
	// Only known OEM Board (Miro) has 1MB
	m_vga->set_vram_size(1*1024*1024);

	// AT&T ATT20C408-13 PrecisionDAC
	// Reused by ATI Mach64?
}

void promotion3210_device::device_start()
{
	pci_card_device::device_start();

	add_map( 4*1024*1024, M_MEM, FUNC(promotion3210_device::vram_aperture_map));

	add_rom((u8 *)m_vga_rom->base(), 0x8000);
	expansion_rom_base = 0xc0000;

	// INTA#
	intr_pin = 1;
}

void promotion3210_device::device_reset()
{
	pci_card_device::device_reset();

	// TODO: to be checked
	command = 0x0000;
	status = 0x0000;
	command_mask = 0x23;

	remap_cb();
}

// bare mapping, except for stuff being mirrored at memory-mapped offsets
void promotion3210_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);
}

void promotion3210_device::vram_aperture_map(address_map &map)
{

}

// TODO: this should really be a subclass of VGA
void promotion3210_device::legacy_memory_map(address_map &map)
{
	map(0xa0000, 0xbffff).rw(FUNC(promotion3210_device::vram_r), FUNC(promotion3210_device::vram_w));
}

void promotion3210_device::legacy_io_map(address_map &map)
{
	map(0, 0x02f).m(m_vga, FUNC(vga_device::io_map));
}

uint8_t promotion3210_device::vram_r(offs_t offset)
{
	return downcast<vga_device *>(m_vga.target())->mem_r(offset);
}

void promotion3210_device::vram_w(offs_t offset, uint8_t data)
{
	downcast<vga_device *>(m_vga.target())->mem_w(offset, data);
}

void promotion3210_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
//  if (1)
	{
		memory_space->install_readwrite_handler(0xa0000, 0xbffff, read8sm_delegate(*this, FUNC(promotion3210_device::vram_r)), write8sm_delegate(*this, FUNC(promotion3210_device::vram_w)));

		io_space->install_device(0x03b0, 0x03df, *this, &promotion3210_device::legacy_io_map);
		//memory_space->install_rom(0xc0000, 0xcffff, (void *)expansion_rom);
	}
}
