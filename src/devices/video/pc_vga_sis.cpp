// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Implementation of SiS family (S)VGA chipset (SiS630)

VBE 3.0, Multi Buffering & Virtual Scrolling available

TODO:
- Refresh rate for extended modes;
- interlace;
- linear addressing;
- HW cursor;
- Output scaling, cfr. xubuntu 6.10 splash screen at 1024x768x32;
- Interrupts;
- Dual segment;
- AGP/HostBus/Turbo Queue i/f;
- 2D/3D pipeline;
- DDC;
- Bridge with a secondary TV out (SiS301);
- Verify matches with other SiS PCI cards, backport;

**************************************************************************************************/

#include "emu.h"
#include "pc_vga_sis.h"

#include "screen.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

// TODO: later variant of 5598
// (definitely doesn't have dual segment mode for instance)
DEFINE_DEVICE_TYPE(SIS630_SVGA, sis630_svga_device, "sis630_svga", "SiS 630 VGA i/f")

sis630_svga_device::sis630_svga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: svga_device(mconfig, SIS630_SVGA, tag, owner, clock)
{
	m_crtc_space_config = address_space_config("crtc_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(sis630_svga_device::crtc_map), this));
	m_seq_space_config = address_space_config("sequencer_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(sis630_svga_device::sequencer_map), this));
}

void sis630_svga_device::device_start()
{
	svga_device::device_start();
	zero();

	// Avoid an infinite loop when displaying.  0 is not possible anyway.
	vga.crtc.maximum_scan_line = 1;

	// copy over interfaces
	vga.svga_intf.vram_size = 64*1024*1024;
	//vga.memory = std::make_unique<uint8_t []>(vga.svga_intf.vram_size);
}

void sis630_svga_device::device_reset()
{
	svga_device::device_reset();

	m_unlock_reg = false;
	//m_dual_seg_mode = false;
}

void sis630_svga_device::io_3cx_map(address_map &map)
{
	svga_device::io_3cx_map(map);
	// TODO: for '630 it's always with dual segment enabled?
	// May be like trident_vga where there's a specific register

	// read by gamecstl Kontron BIOS
	map(0x0b, 0x0b).lrw8(
		NAME([this] (offs_t offset) {
			return svga.bank_r;
		}),
		NAME([this] (offs_t offset, u8 data) {
			svga.bank_r = data;
		})
	);
	map(0x0d, 0x0d).lrw8(
		NAME([this] (offs_t offset) {
			return svga.bank_w;
		}),
		NAME([this] (offs_t offset, u8 data) {
			svga.bank_w = data & 0x3f;
		})
	);
}

// Page 144
void sis630_svga_device::crtc_map(address_map &map)
{
	svga_device::crtc_map(map);
	// CR19/CR1A Extended Signature Read-Back 0/1
	// CR1B CRT horizontal counter (r/o)
	// CR1C CRT vertical counter (r/o)
	// CR1D CRT overflow counter (r/o)
	// CR1E Extended Signature Read-Back 2
	// CR26 Attribute Controller Index read-back
	// TODO: is this an undocumented VGA or a SiS extension?
	map(0x26, 0x26).lr8(
		NAME([this] (offs_t offset) { return vga.attribute.index; })
	);
	// TODO: very preliminary, this section is undocumented in '630 doc
	map(0x30, 0xff).lrw8(
		NAME([this] (offs_t offset) {
			return vga.crtc.data[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			// TODO: if one of these is 0xff then it enables a single port transfer to $b8000
			// Older style MMIO?
			vga.crtc.data[offset] = data;
		})
	);
	// make sure '301 CRT2 is not enabled for now
	// TODO: BeMAME (0.36b5) under BeOS 5.0 detects a secondary monitor by default anyway
	map(0x30, 0x30).lr8(
		NAME([] (offs_t offset) { return 0; })
	);
	map(0x31, 0x31).lr8(
		NAME([] (offs_t offset) { return 0x60; })
	);
	map(0x32, 0x32).lr8(
		NAME([] (offs_t offset) { return 0x20; })
	);
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
			// TODO: reimplement me thru memory_view or direct handler override
			m_unlock_reg = (data == 0x86);
			//LOG("SR5: Unlock register write %02x (%s)\n", data, m_unlock_reg ? "unlocked" : "locked");
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
			LOG("SR06: RAMDAC mode %02x\n", data);

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
			LOG("SR07: Extended Misc. Control 0 %02x\n", data);
			m_ext_misc_ctrl_0 = data;
			std::tie(svga.rgb24_en, svga.rgb32_en) = flush_true_color_mode();
		})
	);
	//map(0x08, 0x09) CRT threshold
	map(0x0a, 0x0a).lrw8(
		NAME([this] (offs_t offset) {
			return m_ext_vert_overflow;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR0A: Extended Vertical Overflow %02x\n", data);
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
			LOG("SR0B: Extended Horizontal Overflow 1 %02x\n", data);
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
			LOG("SR0C: Extended Horizontal Overflow 2 %02x\n", data);
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
			LOG("SR0D: Extended Starting Address %02x\n", data);
			vga.crtc.start_addr_latch &= ~0xff0000;
			vga.crtc.start_addr_latch |= data << 16;
		})
	);
	map(0x0e, 0x0e).lw8(
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR0E: Extended pitch register %02x\n", data);
			// sis_main.c implicitly sets this with bits 0-3 granularity, assume being right
			vga.crtc.offset = (vga.crtc.offset & 0x00ff) | ((data & 0x0f) << 8);
		})
	);
	//map(0x0f, 0x0f) CRT misc. control
	//map(0x10, 0x10) Display line width register
	//map(0x11, 0x11) DDC register
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
			LOG("SR14: <unknown> %02x\n", data);
			m_bus_width = data & 0xc0;
		})
	);
	//map(0x1d, 0x1d) Segment Selection Overflow
	map(0x1e, 0x1e).lw8(
		NAME([this] (offs_t offset, u8 data) {
			if (BIT(data, 6))
				popmessage("pc_vga_sis: enable 2d engine");
		})
	);
	//map(0x1f, 0x1f) Power management
	map(0x20, 0x20).lw8(
		NAME([this] (offs_t offset, u8 data) {
			// GUI address decoder setting
			if (data & 0x81)
				popmessage("pc_vga_sis: SR20 %s %s", BIT(data, 7) ? "PCI address enabled" : "", BIT(data, 0) ? "memory map I/O enable" : "");
		})
	);
	//map(0x21, 0x21) GUI HostBus state machine setting
	//map(0x22, 0x22) GUI HostBus controller timing
	//map(0x23, 0x23) GUI HostBus timer

	//map(0x26, 0x26) Turbo Queue base address
	//map(0x27, 0x27) Turbo Queue control

	map(0x2b, 0x2d).lrw8(
		NAME([this] (offs_t offset) {
			return m_ext_dclk[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR%02X: Extended DCLK %02x\n", offset + 0x2b, data);
			m_ext_dclk[offset] = data;
			recompute_params();
		})
	);
	map(0x2e, 0x30).lrw8(
		NAME([this] (offs_t offset) {
			return m_ext_eclk[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR%02X: Extended ECLK %02x\n", offset + 0x2e, data);
			m_ext_eclk[offset] = data;
			recompute_params();
		})
	);
	map(0x31, 0x31).lrw8(
		NAME([this] (offs_t offset) {
			return m_ext_clock_gen;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR31: Extended clock generator misc. %02x\n", data);
			m_ext_clock_gen = data;
			recompute_params();
		})
	);
	map(0x32, 0x32).lrw8(
		NAME([this] (offs_t offset) {
			return m_ext_clock_source_select;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("SR32: Extended clock source selection %02x\n", data);
			m_ext_clock_source_select = data;
			recompute_params();
		})
	);

	//map(0x34, 0x34) Interrupt status
	//map(0x35, 0x35) Interrupt enable
	//map(0x36, 0x36) Interrupt reset

	//map(0x38, 0x3a) Power on trapping
	//map(0x3c, 0x3c) Synchronous reset
	//map(0x3d, 0x3d) Test enable
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
	u8 xtal_select = (vga.miscellaneous_output & 0x0c) >> 2;
	int xtal;

	switch(xtal_select & 3)
	{
		case 0: xtal = XTAL(25'174'800).value(); break;
		case 1: xtal = XTAL(28'636'363).value(); break;
		// TODO: stub, barely enough to make BeOS 5 to set ~60 Hz for 640x480x16
		case 2:
		default:
			xtal = XTAL(25'174'800).value();
			break;
	}

	recompute_params_clock(1, xtal);
}

uint16_t sis630_svga_device::offset()
{
	if (svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en || svga.rgb32_en)
		return vga.crtc.offset << 3;
	return svga_device::offset();
}

uint8_t sis630_svga_device::mem_r(offs_t offset)
{
	if (svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en || svga.rgb32_en)
		return svga_device::mem_linear_r(offset + svga.bank_r * 0x10000);
	return svga_device::mem_r(offset);
}

void sis630_svga_device::mem_w(offs_t offset, uint8_t data)
{
	if (svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en || svga.rgb32_en)
	{
		svga_device::mem_linear_w(offset + svga.bank_w * 0x10000, data);
		return;
	}
	svga_device::mem_w(offset, data);
}
