// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

SiS 6326

**************************************************************************************************/

#include "emu.h"
#include "sis6326.h"

#define LOG_BLIT      (1U << 1)
#define LOG_AGP       (1U << 2)

#define VERBOSE (LOG_GENERAL | LOG_AGP)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

#define LOGBLIT(...)            LOGMASKED(LOG_BLIT, __VA_ARGS__)
#define LOGAGP(...)             LOGMASKED(LOG_AGP, __VA_ARGS__)


DEFINE_DEVICE_TYPE(SIS6326_PCI, sis6326_pci_device,   "sis6326_pci",   "SiS 6326 PCI card")
DEFINE_DEVICE_TYPE(SIS6326_AGP, sis6326_agp_device,   "sis6326_agp",   "SiS 6326 AGP card")



sis6326_pci_device::sis6326_pci_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
	, m_vga(*this, "vga")
	, m_bios(*this, "bios")
{
	set_ids(0x10396326, 0xa0, 0x030000, 0x10396326);
}

sis6326_pci_device::sis6326_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sis6326_pci_device(mconfig, SIS6326_PCI, tag, owner, clock)
{
}

ROM_START( sis6326pci )
	ROM_REGION32_LE( 0x10000, "bios", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("sis")

	ROM_SYSTEM_BIOS( 0, "sis", "SiS6326 4MB 1.25" )
	ROMX_LOAD( "sis6326_75mhz.vbi", 0x000000, 0x008000, CRC(1c74109d) SHA1(c9180a32e78481c9082ad5bc75082ef4b289ad76), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "3dpro", "3DPro 4MB EDO 1.06 (12-18-97)" )
	ROMX_LOAD( "3dpro4mbedo.bin", 0x000000, 0x010000, CRC(f04b374c) SHA1(6eb96f7517df6eb566c615c2ab9ec5567035b6a5), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "siso", "SiS6326 4MB 1.21d (08-18-98)" )
	ROMX_LOAD( "4mb_pci.vbi",  0x000000, 0x00c000, CRC(8fca47be) SHA1(7ce995ec0d8b9ac4f0f40ccd0a61d7fc209d313f), ROM_BIOS(2) )
ROM_END

const tiny_rom_entry *sis6326_pci_device::device_rom_region() const
{
	return ROM_NAME(sis6326pci);
}

void sis6326_pci_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800), 900, 0, 640, 526, 0, 480);
	screen.set_screen_update(m_vga, FUNC(sis6326_vga_device::screen_update));

	SIS6326_VGA(config, m_vga, 0);
	m_vga->set_screen("screen");
	// 4MB, max 8MB
	m_vga->set_vram_size(4*1024*1024);
	m_vga->md20_cb().set_constant(0);
	m_vga->md21_cb().set_constant(0);
	m_vga->md23_cb().set_constant(1);
	m_vga->md27_cb().set_constant(1);
}

void sis6326_pci_device::device_start()
{
	pci_card_device::device_start();

	add_map(4*1024*1024, M_MEM | M_PREF, FUNC(sis6326_pci_device::vram_aperture_map));
	add_map(64*1024, M_MEM, FUNC(sis6326_pci_device::mmio_map));
	// "32-bit for 16 I/O space"
	add_map(128, M_IO, FUNC(sis6326_pci_device::vmi_map));

	// TODO: should really read the actual BIOS size and set md23 accordingly
	add_rom((u8 *)m_bios->base(), 0x10000);
	expansion_rom_base = 0xc0000;

	// INTA#
	// TODO: VGA D3/MD27 can strap this to no irq pin
	intr_pin = 1;
}

void sis6326_pci_device::device_reset()
{
	pci_card_device::device_reset();

	// doc makes multiple ninja jumps in messing up these defaults
	// bus master (hardwired)
	command = 0x0004;
	command_mask = 0x23;
	// medium DEVSEL#
	// assume capability list & 66 MHz disabled in this variant
	status = 0x0200;

	remap_cb();
}

void sis6326_pci_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);
}


void sis6326_pci_device::vram_aperture_map(address_map &map)
{
	map(0x0000000, 0x3f'ffff).rw(m_vga, FUNC(sis6326_vga_device::mem_linear_r), FUNC(sis6326_vga_device::mem_linear_w));
}

void sis6326_pci_device::mmio_map(address_map &map)
{
	map(0x8280, 0x8283).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_src_start_addr);
			m_src_start_addr &= 0x3f'ffff;
		})
	);
	// TODO: bit 31 is read only for Enhanced Color Expansion busy bit
	map(0x8284, 0x8287).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_dst_start_addr);
			m_dst_start_addr &= 0x3f'ffff;
			if (ACCESSING_BITS_24_31)
				m_draw_sel = data >> 24;
		})
	);
	map(0x8288, 0x8289).lw16(
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_src_pitch);
			m_src_pitch &= 0xfff;
		})
	);
	map(0x828a, 0x828b).lw16(
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_dst_pitch);
			m_dst_pitch &= 0xfff;
		})
	);
	map(0x828c, 0x828d).lw16(
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_rect_width);
			m_rect_width &= 0xfff;
			m_rect_width ++;
		})
	);
	map(0x828e, 0x828f).lw16(
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_rect_height);
			m_rect_height &= 0xfff;
			m_rect_height ++;
		})
	);
	map(0x8290, 0x8293).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_fg_color);
			m_fg_color &= 0xff'ffff;
			if (ACCESSING_BITS_24_31)
				m_fg_rop = data >> 24;
		})
	);
	map(0x8294, 0x8297).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_bg_color);
			m_bg_color &= 0xff'ffff;
			if (ACCESSING_BITS_24_31)
				m_bg_rop = data >> 24;
		})
	);
	map(0x8298, 0x829f).lw8(NAME([this] (offs_t offset, u8 data) { m_mask[offset] = data; }));
	map(0x82a0, 0x82a1).lw16(
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_clip_left);
			m_clip_left &= 0xfff;
		})
	);
	map(0x82a2, 0x82a3).lw16(
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_clip_top);
			m_clip_top &= 0xfff;
		})
	);
	map(0x82a4, 0x82a5).lw16(
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_clip_right);
			m_clip_right &= 0xfff;
		})
	);
	map(0x82a6, 0x82a7).lw16(
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_clip_bottom);
			m_clip_bottom &= 0xfff;
		})
	);

	// HACK: fake Turbo Queue so to not lockup windows
	map(0x82a8, 0x82a9).lr16(
		NAME([] (offs_t offset) { return 0x0100; })
	);
	map(0x82aa, 0x82ab).lrw16(
		NAME([] (offs_t offset) { return 0x8000; }),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_draw_command);
			if (mem_mask == 0xffff)
				trigger_2d_command();
		})
	);

	map(0x82ac, 0x82eb).lw8(NAME([this] (offs_t offset, u8 data) {
		m_pattern_data[offset] = data;
	}));

	map(0x8378, 0x837f).w(m_vga, FUNC(sis6326_vga_device::cursor_mmio_w));
	// HACK: Same deal for 3D Engine Status
	map(0x89fc, 0x89ff).lr32(NAME([] () { return (0x3ff << 16) | 3; }));

	map(0x8afc, 0x8aff).nopw(); // winamp, verbose (TEND dummy register)
}

void sis6326_pci_device::vmi_map(address_map &map)
{
	// same as later '630, win98se expects VGA to be mapped here for extended GFX setups
	map(0x30, 0x5f).m(m_vga, FUNC(sis6326_vga_device::io_map));
}

// TODO: this should really be a subclass of VGA
void sis6326_pci_device::legacy_memory_map(address_map &map)
{
	map(0xa0000, 0xbffff).rw(FUNC(sis6326_pci_device::vram_r), FUNC(sis6326_pci_device::vram_w));
}

void sis6326_pci_device::legacy_io_map(address_map &map)
{
	map(0x03b0, 0x03df).m(m_vga, FUNC(sis6326_vga_device::io_map));
}

uint8_t sis6326_pci_device::vram_r(offs_t offset)
{
	return downcast<sis6326_vga_device *>(m_vga.target())->mem_r(offset);
}

void sis6326_pci_device::vram_w(offs_t offset, uint8_t data)
{
	downcast<sis6326_vga_device *>(m_vga.target())->mem_w(offset, data);
}

void sis6326_pci_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	if (BIT(command, 1))
	{
		memory_space->install_readwrite_handler(0x000a'0000, 0x000b'ffff, read8sm_delegate(*this, FUNC(sis6326_pci_device::vram_r)), write8sm_delegate(*this, FUNC(sis6326_pci_device::vram_w)));
	}

	if (BIT(command, 0))
	{
		io_space->install_device(0x0000, 0xffff, *this, &sis6326_pci_device::legacy_io_map);
	}
}

/*
 * 2d drawing
 */

const sis6326_pci_device::get_src_func sis6326_pci_device::get_src_table[4] =
{
	&sis6326_pci_device::get_src_bgcol,
	&sis6326_pci_device::get_src_fgcol,
	&sis6326_pci_device::get_src_mem,
	&sis6326_pci_device::get_src_cpu
};


u8 sis6326_pci_device::get_src_bgcol(u32 offset_base, u32 x)
{
	return m_bg_color;
}

u8 sis6326_pci_device::get_src_fgcol(u32 offset_base, u32 x)
{
	return m_fg_color;
}

u8 sis6326_pci_device::get_src_mem(u32 offset_base, u32 x)
{
	return m_vga->read_memory(offset_base + x);
}

// TODO: no idea about this yet
u8 sis6326_pci_device::get_src_cpu(u32 offset_base, u32 x)
{
	return x & 1 ? 0x55 : 0xaa;
}

const sis6326_pci_device::get_pat_func sis6326_pci_device::get_pat_table[4] =
{
	&sis6326_pci_device::get_pat_bgcol,
	&sis6326_pci_device::get_pat_fgcol,
	&sis6326_pci_device::get_pat_regs,
	&sis6326_pci_device::get_pat_ddraw
};

u8 sis6326_pci_device::get_pat_bgcol(u32 y, u32 x)
{
	return m_bg_color;
}

u8 sis6326_pci_device::get_pat_fgcol(u32 y, u32 x)
{
	return m_fg_color;
}

// testable in win98se shutdown options
u8 sis6326_pci_device::get_pat_regs(u32 y, u32 x)
{
	return m_pattern_data[((y & 7) << 3) | (x & 7)];
}

// TODO: this is special, uses regs in a different way
u8 sis6326_pci_device::get_pat_ddraw(u32 y, u32 x)
{
	return x & 1 ? 0x55 : 0xaa;
}

// TODO: copied from s3virge, really needs moving in a common interface
// https://learn.microsoft.com/en-us/windows/win32/gdi/ternary-raster-operations
uint32_t sis6326_pci_device::GetROP(uint8_t rop, uint32_t src, uint32_t dst, uint32_t pat)
{
	uint32_t ret = 0;

	switch(rop)
	{
		case 0x00:  // 0
			ret = 0;
			break;
		case 0x0a:  // DPna
			ret = (dst & (~pat));
			break;
		case 0x22:  // DSna
			ret = (dst & (~src));
			break;
		case 0x33:  // Sn
			ret = ~src;
			break;
		case 0x55:  // Dn
			ret = ~dst;
			break;
		case 0x5a:  // DPx
			ret = dst ^ pat;
			break;
		case 0x66:  // DSx
			ret = dst ^ src;
			break;
		case 0x88:  // DSa
			ret = dst & src;
			break;
		case 0xa0:  // DPa (win98se help tooltip borders)
			ret = dst & pat;
			break;
		case 0xaa:  // D
			ret = dst;
			break;
		case 0xb8:  // PSDPxax
			ret = ((dst ^ pat) & src) ^ pat;
			break;
		case 0xbb:  // DSno
			ret = (dst | (~src));
			break;
		case 0xca:  // DPSDxax (moneynet install screen, no effective change?)
			ret = ((src ^ dst) & pat) ^ dst;
			break;
		case 0xcc:
			ret = src;
			break;
		case 0xe2:  // DSPDxax
			ret = ((pat ^ dst) & src) ^ dst;
			break;
		case 0xee:  // DSo
			ret = (dst | src);
			break;
		case 0xf0:
			ret = pat;
			break;
		case 0xff:  // 1
			ret = 0xffffffff;
			break;
		default:
			popmessage("bus/pci/sis6326.cpp: Unimplemented ROP 0x%02x",rop);
	}

	return ret;
}


void sis6326_pci_device::trigger_2d_command()
{
	const u8 src_select = m_draw_command & 3;
	const u8 pat_select = (m_draw_command >> 2) & 3;
	const s8 x_dir = BIT(m_draw_command, 4) ? 1 : -1;
	const s8 y_dir = BIT(m_draw_command, 5) ? 1 : -1;
	const bool clip_enable = !!BIT(m_draw_command, 6);
	const bool clip_external = !!BIT(m_draw_command, 7);
	const u8 cmd_type = (m_draw_command >> 8) & 3;
	const std::string src_types[] = { "bgcol", "fgcol", "memory", "CPU" };
	// NOTE: pat type 3 may really select Direct Draw mode
	const std::string pat_types[] = { "bgcol", "fgcol", "pattern", "<reserved>" };
	LOGBLIT("Command: %04x: SRC %s, PAT %s, XY: %d|%d, clip: %d%d\n"
		, m_draw_command, src_types[src_select], pat_types[pat_select]
		, x_dir, y_dir, clip_enable, clip_external
	);
	if (clip_enable)
	{
		LOGBLIT("cliprange X %d %d ~ Y %d %d\n", m_clip_left, m_clip_right, m_clip_top, m_clip_bottom);
		// (a very unlikely) sanity check
		if (m_dst_pitch == 0)
		{
			LOGBLIT("\tWarning: dst pitch == 0, ignored!\n");
			return;
		}
	}

	// bits 15-14 are r/o
	// TODO: currently using 256 colors only
	// how this even acknowledge a change in color mode?
	switch(cmd_type)
	{
		case 0:
		{
			LOGBLIT("\tBitBlt\n");
			LOGBLIT("\tSRC Addr %06x DST Addr %06x Sel %02x\n", m_src_start_addr, m_dst_start_addr, m_draw_sel);
			LOGBLIT("\tSRC Pitch %d DST Pitch %d Rect %dx%d\n", m_src_pitch, m_dst_pitch, m_rect_width, m_rect_height);
			LOGBLIT("\tFG ROP %02x Color %06x | BG ROP %02x Color %06x\n", m_fg_rop, m_fg_color, m_bg_rop, m_bg_color);

			for (s32 y = 0; y < m_rect_height; y++)
			{
				const s32 yi = y * y_dir;
				if (clip_enable)
				{
					// clipping ranges are derived from the addresses (no explicit X/Y)
					const s16 dst_y = yi + (m_dst_start_addr / m_dst_pitch);
					if ((dst_y == std::clamp<u16>(dst_y, m_clip_top, m_clip_bottom)) ^ clip_external)
						continue;
				}

				const u32 src_base = (yi * m_src_pitch) + m_src_start_addr;
				const u32 dst_base = (yi * m_dst_pitch) + m_dst_start_addr;
				for (s32 x = 0; x < m_rect_width; x++)
				{
					const s32 xi = x * x_dir;

					if (clip_enable)
					{
						const u16 dst_x = xi + (m_dst_start_addr % m_dst_pitch);
						if ((dst_x == std::clamp<u16>(dst_x, m_clip_left, m_clip_right)) ^ clip_external)
							continue;
					}

					const u8 src = (this->*get_src_table[src_select])(src_base, xi);
					const u8 dst = m_vga->read_memory(xi + dst_base);
					const u8 pat = (this->*get_pat_table[pat_select])(y, x);
					const u8 res = GetROP(m_fg_rop, src, dst, pat) & 0xff;
					m_vga->write_memory(xi + dst_base, res);
				}
			}

			break;
		}
		case 1:
		{
			LOGBLIT("\tBitBlt with mask\n");
			// ...
			break;
		}
		case 2:
		{
			const bool color_exp = !!BIT(m_draw_command, 12);
			const bool font_exp = !!BIT(m_draw_command, 13);
			LOGBLIT("\tColor/Font expansion: Color enhanced %d Font enhanced %d\n", color_exp, font_exp);
			LOGBLIT("\tSRC Addr %06x DST Addr %06x Sel %02x\n", m_src_start_addr, m_dst_start_addr, m_draw_sel);
			LOGBLIT("\tSRC Pitch %d DST Pitch %d Rect %dx%d\n", m_src_pitch, m_dst_pitch, m_rect_width, m_rect_height);
			LOGBLIT("\tFG ROP %02x Color %06x | BG ROP %02x Color %06x\n", m_fg_rop, m_fg_color, m_bg_rop, m_bg_color);

			if (m_rect_width > 8)
			{
				LOGBLIT("\tWarning: rect width > 8, ignored!\n");
				return;
			}

			for (s32 y = 0; y < m_rect_height; y++)
			{
				const s32 yi = y * y_dir;

				if (clip_enable)
				{
					const u16 dst_y = yi + (m_dst_start_addr / m_dst_pitch);
					if ((dst_y == std::clamp<u16>(dst_y, m_clip_top, m_clip_bottom)) ^ clip_external)
						continue;
				}

				const u8 pattern_base = y;
				const u32 src_base = (y * m_src_pitch * y_dir) + m_src_start_addr;
				const u32 dst_base = (y * m_dst_pitch * y_dir) + m_dst_start_addr;

				for (s32 x = 0; x < m_rect_width; x++)
				{
					const s32 xi = x * x_dir;

					if (clip_enable)
					{
						const u16 dst_x = xi + (m_dst_start_addr % m_dst_pitch);
						if ((dst_x == std::clamp<u16>(dst_x, m_clip_left, m_clip_right)) ^ clip_external)
							continue;
					}

					const u8 src = (this->*get_src_table[src_select])(src_base, xi);
					const u8 dst = m_vga->read_memory(xi + dst_base);
					const u8 dot = (m_pattern_data[pattern_base] >> (7 - x) & 1);
					// ROP and pattern depends on pattern
					// - win98se start menu hovering
					const u8 res = GetROP(dot ? m_fg_rop : m_bg_rop, src, dst, dot ? m_fg_color : m_bg_color) & 0xff;
					m_vga->write_memory(xi + dst_base, res & 0xff);
				}
			}

			break;
		}
		case 3:
		{
			const bool major_axis = !!BIT(m_draw_command, 10);
			const bool last_pixel = !!BIT(m_draw_command, 11);
			LOGBLIT("\tLine: major axis %s %d\n", major_axis ? "Y" : "X", !last_pixel);
			// ...
			break;
		}
	}

}

/*
 * AGP overrides
 */

sis6326_agp_device::sis6326_agp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sis6326_pci_device(mconfig, SIS6326_AGP, tag, owner, clock)
{
}

ROM_START( sis6326agp )
	ROM_REGION32_LE( 0x10000, "bios", ROMREGION_ERASEFF )
	// Default is AOpen for supporting OpenGL in its driver
	ROM_DEFAULT_BIOS("aopen")

	ROM_SYSTEM_BIOS( 0, "aopen", "AOpen PA50V 4MB 2.25" )
	ROMX_LOAD( "aopenpa50v.vbi", 0x000000, 0x008000, CRC(6c4c7518) SHA1(36bb29a23d565e34d548701acc248d50c99d7da4), ROM_BIOS(0) )
	// AGP/PCI
	ROM_SYSTEM_BIOS( 1, "siso", "SiS6326 4MB 1.21d (08-18-98)" )
	ROMX_LOAD( "4mbsdr.vbi",   0x000000, 0x00c000, CRC(a5f8c7f7) SHA1(9e5cfcf8d34e0c5829343179c87fb2b97f7a7f9c), ROM_BIOS(1) )
	// AGP variant of the sis6326pci one
	ROM_SYSTEM_BIOS( 2, "sis", "SiS6326 4MB 1.25" )
	ROMX_LOAD( "sis6326agp.bin", 0x000000, 0x010000, CRC(a671255c) SHA1(332ab9499142e8f7a235afe046e8e8d21a28580d), ROM_BIOS(2) )
ROM_END

const tiny_rom_entry *sis6326_agp_device::device_rom_region() const
{
	return ROM_NAME(sis6326agp);
}

void sis6326_agp_device::device_add_mconfig(machine_config &config)
{
	sis6326_pci_device::device_add_mconfig(config);
	m_vga->md20_cb().set_constant(1);
	m_vga->md21_cb().set_constant(1);
	m_vga->md23_cb().set_constant(1);
	m_vga->md27_cb().set_constant(1);
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
	sis6326_pci_device::config_map(map);
	// AGP
	map(0x50, 0x53).lr32(NAME([] () { return 0x00105c02; } ));
	map(0x54, 0x57).lr32(NAME([] () { return 0x01000003; } ));
	map(0x58, 0x5b).rw(FUNC(sis6326_agp_device::agp_command_r), FUNC(sis6326_agp_device::agp_command_w));
	map(0x5c, 0x5f).lr32(NAME([] () { return 0x00000000; } )); // NULL terminator
}

