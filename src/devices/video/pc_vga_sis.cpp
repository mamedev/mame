// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Implementation of SiS family (S)VGA chipset

TODO:
- interlace (cfr. xubuntu 6.10 splash screen on 1024x768x32);
- backport to earlier variants;

**************************************************************************************************/

#include "emu.h"
#include "pc_vga_sis.h"

#include "screen.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

// TODO: later variant of 5598
// (definitely doesn't have dual segment mode for instance)
DEFINE_DEVICE_TYPE(SIS630_SVGA, sis630_svga_device, "sis630_svga", "SiS 630 SVGA")

sis630_svga_device::sis630_svga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: svga_device(mconfig, SIS630_SVGA, tag, owner, clock)
{
	m_seq_space_config = address_space_config("sequencer_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(sis630_svga_device::sequencer_map), this));
}

void sis630_svga_device::device_start()
{
	svga_device::device_start();
	zero();

	// Avoid an infinite loop when displaying.  0 is not possible anyway.
	vga.crtc.maximum_scan_line = 1;

	// copy over interfaces
	vga.svga_intf.crtc_regcount = 0x27;
	vga.svga_intf.vram_size = 64*1024*1024;
	//vga.memory = std::make_unique<uint8_t []>(vga.svga_intf.vram_size);
}

void sis630_svga_device::device_reset()
{
	svga_device::device_reset();

	m_svga_bank_reg_w = m_svga_bank_reg_r = 0;
	m_unlock_reg = false;
	//m_dual_seg_mode = false;
}

// Page 144
uint8_t sis630_svga_device::crtc_reg_read(uint8_t index)
{
	if (index < 0x19)
		return svga_device::crtc_reg_read(index);

	// make sure '301 CRT2 is not enabled
	if (index == 0x30)
		return 0;

	if (index == 0x31)
		return 0x60;

	if (index == 0x32)
		return 0x20;

	// TODO: if one of these is 0xff then it enables a single port transfer to $b8000
	return m_crtc_ext_regs[index];
}

void sis630_svga_device::crtc_reg_write(uint8_t index, uint8_t data)
{
	if (index < 0x19)
		svga_device::crtc_reg_write(index, data);
	else
	{
		m_crtc_ext_regs[index] = data;
	}
}

void sis630_svga_device::sequencer_map(address_map &map)
{
	svga_device::sequencer_map(map);
	// extended ID register
	map(0x05, 0x05).lrw8(
		NAME([this] (offs_t offset) {
			return m_unlock_reg ? 0xa1 : 0x21;
		}),
		NAME([this] (offs_t offset, u8 data) {
			// TODO: reimplement me thru memory_view
			m_unlock_reg = (data == 0x86);
			LOG("Unlock register write %02x (%s)\n", data, m_unlock_reg ? "unlocked" : "locked");
		})
	);
	/*
	 * x--- ---- GFX mode linear addressing enable
	 * -x-- ---- GFX hardware cursor display
	 * --x- ---- GFX mode interlace
	 * ---x ---- True Color enable (ties with index 0x07 bit 2)
	 * ---- x--- RGB16 enable
	 * ---- -x-- RGB15 enable
	 * ---- --x- enhanced GFX mode enable
	 * ---- ---x enhanced text mode enable
	 */
	map(0x06, 0x06).lrw8(
		NAME([this] (offs_t offset) {
			return m_ramdac_mode;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_ramdac_mode = data;
			LOG("RAMDAC mode %02x\n", data);

			if (!BIT(data, 1))
			{
				svga.rgb8_en = svga.rgb15_en = svga.rgb16_en = svga.rgb24_en = svga.rgb32_en = 0;
			}
			else
			{
				if (BIT(data, 2))
					svga.rgb15_en = 1;
				if (BIT(data, 3))
					svga.rgb16_en = 1;
				std::tie(svga.rgb24_en, svga.rgb32_en) = flush_true_color_mode();
			}
		})
	);
	map(0x07, 0x07).lrw8(
		NAME([this] (offs_t offset) {
			return m_ext_misc_ctrl_0;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("Extended Misc. Control register 0 SR07 %02x\n", data);
			m_ext_misc_ctrl_0 = data;
			std::tie(svga.rgb24_en, svga.rgb32_en) = flush_true_color_mode();
		})
	);
	map(0x0a, 0x0a).lrw8(
		NAME([this] (offs_t offset) {
			return m_ext_vert_overflow;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("Extended vertical Overflow register SR0A %02x\n", data);
			m_ext_vert_overflow = data;
			vga.crtc.vert_retrace_end  =  (vga.crtc.vert_retrace_end & 0xf)      | ((data & 0x20) >> 1);
			vga.crtc.vert_blank_end  =    (vga.crtc.vert_blank_end & 0x00ff)     | ((data & 0x10) << 4);
			vga.crtc.vert_retrace_start = (vga.crtc.vert_retrace_start & 0x03ff) | ((data & 0x08) << 7);
			vga.crtc.vert_blank_start =   (vga.crtc.vert_blank_start & 0x03ff)   | ((data & 0x04) << 8);
			vga.crtc.vert_disp_end =      (vga.crtc.vert_disp_end & 0x03ff)      | ((data & 0x02) << 9);
			vga.crtc.vert_total =         (vga.crtc.vert_total & 0x03ff)         | ((data & 0x01) << 10);
			recompute_params();
		})
	);
	map(0x0b, 0x0c).lr8(
		NAME([this] (offs_t offset) {
			return m_ext_horz_overflow[offset];
		})
	);
	map(0x0b, 0x0b).lw8(
		NAME([this] (offs_t offset, u8 data) {
			//m_dual_seg_mode = bool(BIT(data, 3));
			LOG("Extended horizontal Overflow 1 SR0B %02x\n", data);
			m_ext_horz_overflow[0] = data;

			vga.crtc.horz_retrace_start = (vga.crtc.horz_retrace_start & 0x00ff) | ((data & 0xc0) << 2);
			vga.crtc.horz_blank_start =   (vga.crtc.horz_blank_start & 0x00ff)   | ((data & 0x30) << 4);
			vga.crtc.horz_disp_end =      (vga.crtc.horz_disp_end & 0x00ff)      | ((data & 0x0c) << 6);
			vga.crtc.horz_total =         (vga.crtc.horz_total & 0x00ff)         | ((data & 0x03) << 8);

			recompute_params();
		})
	);
	map(0x0c, 0x0c).lw8(
		NAME([this] (offs_t offset, u8 data) {
			LOG("Extended horizontal Overflow 2 SR0C %02x\n", data);
			m_ext_horz_overflow[1] = data;

			vga.crtc.horz_retrace_end =   (vga.crtc.horz_retrace_end & 0x001f) | ((data & 0x04) << 3);
			vga.crtc.horz_blank_end =     (vga.crtc.horz_blank_end & 0x003f)   | ((data & 0x03) << 6);
			recompute_params();
		})
	);
	map(0x0d, 0x0d).lrw8(
		NAME([this] (offs_t offset) {
			return vga.crtc.start_addr_latch >> 16;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("Extended starting address register SR0D %02x\n", data);
			vga.crtc.start_addr_latch &= ~0xff0000;
			vga.crtc.start_addr_latch |= data << 16;
		})
	);
	map(0x0e, 0x0e).lw8(
		NAME([this] (offs_t offset, u8 data) {
			LOG("Extended pitch register SR0E %02x\n", data);
			// sis_main.c implicitly sets this with bits 0-3 granularity, assume being right
			vga.crtc.offset = (vga.crtc.offset & 0x00ff) | ((data & 0x0f) << 8);
		})
	);
	map(0x14, 0x14).lrw8(
		NAME([this] (offs_t offset) {
			// sis_main.c calculates VRAM size in two ways:
			// 1. the legacy way ('300), by probing this register
			// 2. by reading '630 PCI host register $63 (as shared DRAM?)
			// Method 1 seems enough to enforce "64MB" message at POST,
			// 2 is probably more correct but unsure about how to change the shared area in BIOS
			// (shutms11 will always write a "0x41" on fresh CMOS then a "0x47"
			//  on successive boots no matter what)
			return (m_bus_width) | ((vga.svga_intf.vram_size / (1024 * 1024) - 1) & 0x3f);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_bus_width = data & 0xc0;
		})
	);
	map(0x1e, 0x1e).lw8(
		NAME([this] (offs_t offset, u8 data) {
			if (BIT(data, 6))
				popmessage("Warning: enable 2d engine");
		})
	);
	map(0x20, 0x20).lw8(
		NAME([this] (offs_t offset, u8 data) {
			if (data & 0x81)
				popmessage("Warning: %s %s", BIT(data, 7) ? "PCI address enabled" : "", BIT(data, 0) ? "memory map I/O enable" : "");
		})
	);
}

std::tuple<u8, u8> sis630_svga_device::flush_true_color_mode()
{
	// punt if extended or true color is off
	if ((m_ramdac_mode & 0x12) != 0x12)
		return std::make_tuple(0, 0);

	const u8 res = (m_ext_misc_ctrl_0 & 4) >> 2;

	return std::make_tuple(res, res ^ 1);
}

void sis630_svga_device::recompute_params()
{
	// TODO: ext clock
	recompute_params_clock(1, XTAL(25'174'800).value());
}

uint16_t sis630_svga_device::offset()
{
	if (svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en || svga.rgb32_en)
		return vga.crtc.offset << 3;
	return svga_device::offset();
}

// read by gamecstl Kontron BIOS
u8 sis630_svga_device::port_03c0_r(offs_t offset)
{
	if (offset == 0xd)
		return m_svga_bank_reg_w;
	if (offset == 0xb)
		return m_svga_bank_reg_r;

	return svga_device::port_03c0_r(offset);
}

void sis630_svga_device::port_03c0_w(offs_t offset, uint8_t data)
{
	// TODO: for '630 it's always with dual segment enabled?

	if (offset == 0xd)
	{
		//if (m_dual_seg_mode)
			m_svga_bank_reg_w = (data & 0x3f) * 0x10000;
		//else
		{
		//  m_svga_bank_reg_w = (data >> 4) * 0x10000;
		//  m_svga_bank_reg_r = (data & 0xf) * 0x10000;
		}
		return;
	}

	if (offset == 0xb)
	{
		//if (m_dual_seg_mode)
			m_svga_bank_reg_r = (data & 0x3f) * 0x10000;
		// otherwise ignored if dual segment mode disabled
		return;
	}

	svga_device::port_03c0_w(offset, data);
}

uint8_t sis630_svga_device::mem_r(offs_t offset)
{
	if (svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en || svga.rgb32_en)
		return svga_device::mem_linear_r(offset + m_svga_bank_reg_r);
	return svga_device::mem_r(offset);
}

void sis630_svga_device::mem_w(offs_t offset, uint8_t data)
{
	if (svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en || svga.rgb32_en)
	{
		svga_device::mem_linear_w(offset + m_svga_bank_reg_w, data);
		return;
	}
	svga_device::mem_w(offset, data);
}
