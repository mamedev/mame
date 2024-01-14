// license:BSD-3-Clause
// copyright-holders: Angelo Salese
// thanks-to: Marcelina Koscielnicka
/**************************************************************************************************

nVidia NV3/NV3T Riva 128

TODO:
- Windows 98 punts device detection by attempting to modify the (supposedly) PMC ID.
- Maybe the card is supposed to send an INTA trap on the write attempt?

References:
- https://envytools.readthedocs.io/en/latest/hw/mmio.html?highlight=mmio#nv3-g80-mmio-map

**************************************************************************************************/

#include "emu.h"
#include "riva128.h"

#define LOG_WARN      (1U << 1)
#define LOG_TODO      (1U << 2) // log unimplemented registers

#define VERBOSE (LOG_GENERAL | LOG_WARN | LOG_TODO)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)
#define LOGTODO(...)            LOGMASKED(LOG_TODO, __VA_ARGS__)


DEFINE_DEVICE_TYPE(RIVA128,   riva128_device,   "riva128",   "SGS-Thompson/nVidia Riva 128 (NV3)")
DEFINE_DEVICE_TYPE(RIVA128ZX, riva128zx_device, "riva128zx", "SGS-Thompson/nVidia Riva 128 ZX (NV3T)")

riva128_device::riva128_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
	, m_svga(*this, "svga")
	, m_vga_rom(*this, "vga_rom")
{
	// device ID 0x12d2 SGS-Thompson/nVidia joint venture
	// 0x0018 RIVA 128 (NV3)
	// 0x0019 RIVA 128 ZX (NV3T)
	// TODO: STB uses 0x10b4xxxx, unknown for ASUS
	set_ids_agp(0x12d20018, 0x00, 0x10921092);
}

riva128_device::riva128_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: riva128_device(mconfig, RIVA128, tag, owner, clock)
{
}

ROM_START( riva128 )
	ROM_REGION32_LE( 0x8000, "vga_rom", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "diamond", "Diamond Viper V330 1.62-CO 01/14/98" )
	ROMX_LOAD( "diamond_v330_rev-e.vbi", 0x0000, 0x8000, CRC(68686ddc) SHA1(cd2e299acd79624c7d82ce3317004c96bd4e36f7), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "asus", "ASUS AGP/3DP-V3000 1.51B 09/06/97" )
	ROMX_LOAD( "riva128_asus.vbi", 0x0000, 0x8000, CRC(cc57586f) SHA1(5e6ec14c66ec38b21013ee6b7582f6b6a4586e2e), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "stb", "STB Velocity 128 AGP 1.82 12/17/97" )
	ROMX_LOAD( "riva128_stb.vbi", 0x0000, 0x8000, CRC(80da0245) SHA1(894c855c6d676de0d04d396f7e93a0e6cb98a4b3), ROM_BIOS(2) )
ROM_END

const tiny_rom_entry *riva128_device::device_rom_region() const
{
	return ROM_NAME(riva128);
}

void riva128_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update(m_svga, FUNC(nvidia_nv3_vga_device::screen_update));

	NVIDIA_NV3_VGA(config, m_svga, 0);
	m_svga->set_screen("screen");
	// FIXME: shared RAM
	// reports as 4MB in AIDA16
	m_svga->set_vram_size(4*1024*1024);
}

void riva128_device::device_start()
{
	pci_card_device::device_start();

	add_map( 16*1024*1024, M_MEM, FUNC(riva128_device::mmio_map));
	add_map(128*1024*1024, M_MEM, FUNC(riva128_device::vram_aperture_map));
	// indirect memory access I/Os (NV3 only)
	add_map(0x100, M_IO, FUNC(riva128_device::indirect_io_map));
	// TODO: Windows 98 expects an extra range mapped at 0x10000000-0x10007fff

	add_rom((u8 *)m_vga_rom->base(), 0x8000);
	expansion_rom_base = 0xc0000;

	// INTA#
	intr_pin = 1;
	save_item(NAME(m_vga_legacy_enable));
}

void riva128_device::device_reset()
{
	pci_card_device::device_reset();

	// TODO: to be checked
	command = 0x0000;
	status = 0x0000;

	m_vga_legacy_enable = true;
	m_main_scratchpad_id = 0x00030310;
	remap_cb();
}

// TODO: counter-check everything
void riva128_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);
	map(0x34, 0x34).lr8(NAME([] () { return 0x44; }));

//  map(0x40, 0x43) subsystem ID alias (writeable)
//  map(0x44, 0x4f) AGP i/f
//  map(0x50, 0x53) ROM shadow enable
	map(0x54, 0x57).lrw8(
		NAME([this] (offs_t offset) { return m_vga_legacy_enable; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_0_7)
			{
				m_vga_legacy_enable = BIT(data, 0);
				remap_cb();
			}
			//printf("- %s VGA control %08x & %08x\n", m_vga_legacy_enable ? "Enable" : "Disable", data, mem_mask);
		})
	);
	map(0x58, 0x58).lr8(NAME([] () { return 0x00; }));

}

void riva128_device::mmio_map(address_map &map)
{
	map(0x00000000, 0x00ffffff).rw(FUNC(riva128_device::unmap_log_r), FUNC(riva128_device::unmap_log_w));
	map(0x00000000, 0x00000003).lrw32(
		NAME([this] (offs_t offset) {
			machine().debug_break();
			LOGTODO("MMIO ID readback\n");
			return m_main_scratchpad_id;
			//return 0x00030310;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_main_scratchpad_id);
		})
	);
//  map(0x00000000, 0x00000fff) PMC card master control
//  map(0x00001000, 0x00001fff) PBUS bus control
//  map(0x00002000, 0x00003fff) PFIFO
//  map(0x00007000, 0x00007***) PRMA real mode BAR Access
//  map(0x00009000, 0x00009***) PTIMER
//  map(0x000a0000, 0x000bffff) PRMFB legacy VGA memory
//  map(0x000c0000, 0x000c****) PRMVIO VGA sequencer & VGA gfx regs (multiple on NV40+)
//  map(0x00100000, 0x0010*fff) PFB memory interface
//  map(0x00110000, 0x0011ffff) PROM ROM access window
//  map(0x00120000, 0x0012ffff) PALT External memory access window
//  map(0x00400000, 0x00400fff) PGRAPH 2d/3d graphics engine
//  map(0x00401000, 0x00401***) PDMA system memory DMA engine (NV3/NV4 only)
//  map(0x00600000, 0x00600***) PCRTC CRTC controls (on NV4+ only?)
//  map(0x00601000, 0x0060****) PRMCIO VGA CRTC controls
//  map(0x00680000, 0x0068****) PRAMDAC
//  map(0x00681000, 0x00681***) VGA DAC registers
//  map(0x00800000, 0x00******) PFIFO MMIO submission area
}

void riva128_device::vram_aperture_map(address_map &map)
{

}

void riva128_device::indirect_io_map(address_map &map)
{

}

u32 riva128_device::unmap_log_r(offs_t offset, u32 mem_mask)
{
	LOGTODO("MMIO Unemulated [%08x] & %08x R\n", offset * 4, mem_mask);
	return 0;
}

void riva128_device::unmap_log_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGTODO("MMIO Unemulated [%08x] %08x & %08x W\n", offset * 4, data, mem_mask);
}


// TODO: this should really be a subclass of VGA
void riva128_device::legacy_memory_map(address_map &map)
{
	map(0xa0000, 0xbffff).rw(FUNC(riva128_device::vram_r), FUNC(riva128_device::vram_w));
}

void riva128_device::legacy_io_map(address_map &map)
{
	map(0, 0x02f).m(m_svga, FUNC(nvidia_nv3_vga_device::io_map));
}

uint8_t riva128_device::vram_r(offs_t offset)
{
	return downcast<nvidia_nv3_vga_device *>(m_svga.target())->mem_r(offset);
}

void riva128_device::vram_w(offs_t offset, uint8_t data)
{
	downcast<nvidia_nv3_vga_device *>(m_svga.target())->mem_w(offset, data);
}

void riva128_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	if (m_vga_legacy_enable)
	{
		memory_space->install_readwrite_handler(0xa0000, 0xbffff, read8sm_delegate(*this, FUNC(riva128_device::vram_r)), write8sm_delegate(*this, FUNC(riva128_device::vram_w)));

		io_space->install_device(0x03b0, 0x03df, *this, &riva128_device::legacy_io_map);
		//memory_space->install_rom(0xc0000, 0xcffff, (void *)expansion_rom);
	}
}

/********************************************
 *
 * Riva 128ZX overrides
 *
 *******************************************/

riva128zx_device::riva128zx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: riva128_device(mconfig, RIVA128ZX, tag, owner, clock)
{
	// $54-$57 in ROM for subvendor ID (if FBA[1] config is 1)
	set_ids_agp(0x12d20019, 0x00, 0x12d20019);
}

ROM_START( riva128zx )
	ROM_REGION32_LE( 0x8000, "vga_rom", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "asus", "ASUS AGP-V3000 ZX TV (V1.70D.03)" )
	ROMX_LOAD( "asus_agp-v3000zx.vbi", 0x000000, 0x008000, CRC(8319de18) SHA1(837fe8afdf03196550c51ff4987e7f25cc75222c), ROM_BIOS(0) )
	// TODO: confirm if this is really a 128ZX
	// (underlying ROM PCIR has 0x0018 not 0x0019)
	ROM_SYSTEM_BIOS( 1, "elsa", "ELSA VICTORY Erazor/LT (Ver. 1.58.00)" )
	ROMX_LOAD( "elsa.vbi",     0x000000, 0x008000, CRC(8dd4627a) SHA1(cfb10d9a370a951f9ed23719b2c5fa79c9e49668), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "creative", "Creative Graphics Blaster Riva 128ZX (V1.72.3D)" )
	ROMX_LOAD( "creative_ct6730.vbi", 0x000000, 0x008000, CRC(72f03a0e) SHA1(7126c4c4d20c48848defc5dd05f0d5b698948015), ROM_BIOS(2) )
ROM_END

const tiny_rom_entry *riva128zx_device::device_rom_region() const
{
	return ROM_NAME(riva128zx);
}
