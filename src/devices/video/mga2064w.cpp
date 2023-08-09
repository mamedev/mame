// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Angelo Salese
#include "emu.h"
#include "mga2064w.h"

#define LOG_WARN      (1U << 1)
#define LOG_ALIAS     (1U << 2) // log mgabase1 index setups thru the back door

#define VERBOSE (LOG_GENERAL | LOG_WARN | LOG_ALIAS)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)
#define LOGALIAS(...)           LOGMASKED(LOG_ALIAS, __VA_ARGS__)

DEFINE_DEVICE_TYPE(MGA2064W, mga2064w_device, "mga2064w", "Matrox Millennium \"IS-STORM / MGA-2064W\"")

mga2064w_device::mga2064w_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, MGA2064W, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_svga(*this, "svga")
	, m_vga_rom(*this, "vga_rom")
{
	set_ids(0x102b0519, 0x01, 0x030000, 0x00000000);
	m_mgabase1_real_space_config = address_space_config("mgabase1_regs", ENDIANNESS_LITTLE, 32, 14, 0, address_map_constructor(FUNC(mga2064w_device::mgabase1_map), this));
}

ROM_START( mga2064w )
	ROM_REGION32_LE( 0x10000, "vga_rom", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "rev3", "Matrox Power Graphics Accelerator V2.4 IS-MGA-2064W R3" )
	ROMX_LOAD( "rev3.bin",     0x000000, 0x010000, CRC(cb623dab) SHA1(4dc10755613a8fa9599331d78995cfb15145440b), ROM_BIOS(0) )
	// TODO: verify label naming for these
	ROM_SYSTEM_BIOS( 1, "rev2", "Matrox Power Graphics Accelerator V1.9 IS-MGA-2064W R2 2MB" )
	ROMX_LOAD( "rev2_2mb.bin", 0x000000, 0x010000, CRC(253c352b) SHA1(a5cd7e1c4903fcc89ea04cc1911b8d010e6513d1), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "rev2_storm", "Matrox Power Graphics Accelerator V1.9 IS-STORM R2 (vbi)" )
	ROMX_LOAD( "rev2_storm4mb.vbi", 0x000000, 0x008000, CRC(35660abe) SHA1(36ec630507548e1ef5fa7fdd07852d936fb614e5), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "rev2_isstorm", "Matrox Power Graphics Accelerator V1.9 IS-STORM R2" )
	ROMX_LOAD( "matroxisstormr2.bin", 0x000000, 0x010000, CRC(0cfceda4) SHA1(26a4fe291c738b4b138b522beb37b7db1b639634), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "rev2_r2", "Matrox Power Graphics Accelerator V1.9 IS-MGA-2064W R2" )
	ROMX_LOAD( "matrox2064wr2.bin", 0x000000, 0x010000, CRC(79920e74) SHA1(d62d6a57c75f2266e3d0f85916f366d62ad56ce4), ROM_BIOS(4) )
ROM_END

const tiny_rom_entry *mga2064w_device::device_rom_region() const
{
	return ROM_NAME(mga2064w);
}

void mga2064w_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update(m_svga, FUNC(matrox_vga_device::screen_update));

	MATROX_VGA(config, m_svga, 0);
	m_svga->set_screen("screen");
	m_svga->set_vram_size(8*1024*1024);
}

void mga2064w_device::device_start()
{
	pci_device::device_start();
	// NB: following is swapped on G400
	add_map(    16*1024, M_MEM, FUNC(mga2064w_device::mgabase1_map));
	add_map(8*1024*1024, M_MEM, FUNC(mga2064w_device::mgabase2_map));
	//  add_rom_from_region();

	add_rom((u8 *)m_vga_rom->base(), 0x10000);
}

void mga2064w_device::device_reset()
{
	pci_device::device_reset();

	// INTA#
	intr_pin = 1;
	m_mgabase1_real_index = 0;
}

device_memory_interface::space_config_vector mga2064w_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_IO, &m_mgabase1_real_space_config)
	};
}

void mga2064w_device::config_map(address_map &map)
{
	pci_device::config_map(map);
//  map(0x40, 0x43) OPTION
	map(0x44, 0x47).rw(FUNC(mga2064w_device::mga_index_r), FUNC(mga2064w_device::mga_index_w));
	map(0x48, 0x4b).rw(FUNC(mga2064w_device::mga_data_r), FUNC(mga2064w_device::mga_data_w));
}

void mga2064w_device::mgabase1_map(address_map &map)
{
//  map(0x0000, 0x1bff).rw(FUNC(mga2064w_device::dmawin_iload_r), FUNC(mga2064w_device::dmawin_idump_w));
//  map(0x1c00, 0x1cff).mirror(0x100).m(FUNC(mga2064w_device::dwgreg_map);
//  map(0x1c00, 0x1c03) DWGCTL
//  map(0x1c04, 0x1c07) MACCESS
//  map(0x1c08, 0x1c0b) <reserved> MCTLWTST
//  map(0x1c0c, 0x1c0f) ZORG
//  map(0x1c10, 0x1c13) PAT0
//  map(0x1c14, 0x1c17) PAT1
//  map(0x1c1c, 0x1c1f) PLNWT
//  map(0x1c20, 0x1c23) BCOL
//  map(0x1c24, 0x1c27) FCOL
//  map(0x1c2c, 0x1c2f) <reserved> SRCBLT
//  map(0x1c30, 0x1c3f) SRC0-3
//  map(0x1c40, 0x1c43) XYSTRT
//  map(0x1c44, 0x1c47) XYEND
//  map(0x1c50, 0x1c53) SHIFT
//  map(0x1c58, 0x1c5b) SGN
//  map(0x1c5c, 0x1c5f) LEN
//  map(0x1c60, 0x1c7b) AR0-6
//  map(0x1c80, 0x1c83) CXBNDRY
//  map(0x1c84, 0x1c87) FXBNDRY
//  map(0x1c88, 0x1c8b) YDSTLEN
//  map(0x1c8c, 0x1c8f) PITCH
//  map(0x1c90, 0x1c93) YDST
//  map(0x1c94, 0x1c97) YDSTORG
//  map(0x1c98, 0x1c9b) YTOP
//  map(0x1c9c, 0x1c9f) YBOT
//  map(0x1ca0, 0x1ca3) CXLEFT
//  map(0x1ca4, 0x1ca7) CXRIGHT
//  map(0x1ca8, 0x1cab) FXLEFT
//  map(0x1cac, 0x1caf) FXRIGHT
//  map(0x1cb0, 0x1cb3) XDST
//  map(0x1cc0, 0x1cff) DR0-DR15 (DR1-5-9-13 <reserved>)

//  map(0x1e00, 0x1eff) HSTREG Host registers
	map(0x1e10, 0x1e13).r(FUNC(mga2064w_device::fifo_status_r));
	map(0x1e14, 0x1e17).r(FUNC(mga2064w_device::status_r));
//  map(0x1e18, 0x1e1b) ICLEAR
//  map(0x1e1c, 0x1e1f) IEN
//  map(0x1e20, 0x1e23) VCOUNT (r/o)
//  map(0x1e40, 0x1e43) Reset
//  map(0x1e54, 0x1e57) OPMODE
//  map(0x1f00, 0x1fff) VGA CRTC linear I/O
	map(0x1fb0, 0x1fdf).m(m_svga, FUNC(matrox_vga_device::io_map));
	map(0x3c00, 0x3c1f).m(m_svga, FUNC(matrox_vga_device::ramdac_ext_map));
//  map(0x3e00, 0x3fff) EXPDEV Expansion bus
}

void mga2064w_device::mgabase2_map(address_map &map)
{
	map(0x000000, 0x7fffff).rw(m_svga, FUNC(matrox_vga_device::mem_linear_r), FUNC(matrox_vga_device::mem_linear_w));
}

/*
 * MGABASE1 + 1e10h FIFO Status (r/o)
 *
 * ---- -x-- ---- ---- BEMPTY Bus FIFO empty
 * ---- --x- ---- ---- BFULL Bus FIFO full
 * ---- ---- ---x xxxx FIFOCOUNT free locations in FIFO (max: 32)
 */
u32 mga2064w_device::fifo_status_r()
{
	return (1 << 9) | 32;
}

/*
 * MGABASE1 + 1e14h Status (r/o)
 *
 * ---- ---- ---- ---x ---- ---- ---- ---- DWGENGSTS
 * ---- ---- ---- ---- ---- ---- -x-- ---- EXTPEN
 * ---- ---- ---- ---- ---- ---- --x- ---- VLINEPEN
 * ---- ---- ---- ---- ---- ---- ---x ---- VSYNCPEN
 * ---- ---- ---- ---- ---- ---- ---- x--- VSYNCSTS
 * ---- ---- ---- ---- ---- ---- ---- -x-- PICKPEN
 */
u32 mga2064w_device::status_r()
{
	return m_svga->vsync_status() << 3;
}

// TODO: this should really be a subclass of VGA
void mga2064w_device::legacy_memory_map(address_map &map)
{
	map(0xa0000, 0xbffff).rw(FUNC(mga2064w_device::vram_r), FUNC(mga2064w_device::vram_w));
}

void mga2064w_device::legacy_io_map(address_map &map)
{
	map(0, 0x02f).m(m_svga, FUNC(matrox_vga_device::io_map));
}

uint8_t mga2064w_device::vram_r(offs_t offset)
{
	return downcast<matrox_vga_device *>(m_svga.target())->mem_r(offset);
}

void mga2064w_device::vram_w(offs_t offset, uint8_t data)
{
	downcast<matrox_vga_device *>(m_svga.target())->mem_w(offset, data);
}

void mga2064w_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	// TODO: both can be disabled thru config options
	{
		memory_space->install_readwrite_handler(0xa0000, 0xbffff, read8sm_delegate(*this, FUNC(mga2064w_device::vram_r)), write8sm_delegate(*this, FUNC(mga2064w_device::vram_w)));

		io_space->install_device(0x03b0, 0x03df, *this, &mga2064w_device::legacy_io_map);
	}
}

/*
 * MGA_INDEX / MGA_DATA
 * aliases for accessing mgabase1 thru PCI config space
 * i.e. a backdoor for x86 in real mode
 */

u32 mga2064w_device::mga_index_r()
{
	LOGALIAS("MGA_INDEX read\n");
	return m_mgabase1_real_index & 0x3ffc;
}

void mga2064w_device::mga_index_w(offs_t offset, u32 data, u32 mem_mask)
{
	// VESA BIOS sets up $3c0a while accessing with mask 0x00ff0000
	// bits 0-1 are reserved and don't respond, assume mistake
	LOGALIAS("MGA_INDEX write %08x %08x\n", data, mem_mask);
	COMBINE_DATA(&m_mgabase1_real_index);
	m_mgabase1_real_index &= 0x3ffc;
}

u32 mga2064w_device::mga_data_r(offs_t offset, u32 mem_mask)
{
	return space(AS_IO).read_dword(m_mgabase1_real_index, mem_mask);
}

void mga2064w_device::mga_data_w(offs_t offset, u32 data, u32 mem_mask)
{
	space(AS_IO).write_dword(m_mgabase1_real_index, data, mem_mask);
}
