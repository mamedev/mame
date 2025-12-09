// license:BSD-3-Clause
// copyright-holders: Angelo Salese
// thanks-to: Marcelina Koscielnicka
/**************************************************************************************************

nVidia NV3:G80 VGA core

TODO:
- Many blanks in the CRTC section;
\- Untested beyond 16bpp modes;
\- Regular VGA gfx modes don't draw text properly in some tests (common bug with other VGA archs);
\- Diamond Viper BIOS don't report back the video card string at POST;
\- STB BIOS sets 640x481 after POST;
\- ASUS BIOS sets 640x428 (?);
- The "red screen" from VGA SENSE may really be a ping from monitor signal;
- Sequencer register $6 for lock/unlock of extended VGA;
- I2C;
- Interrupts;
- RMA access;
- PLL portion(s), understand how it chooses extended clocks;
- Verify that NV10 really uses this chip (seems to access more regs?)
- Video overlay (PVIDEO, NV10+);
- VGA stack (NV40+);
- TV encoder (NV17:NV20, NV30:G80);

Notes:
- NV1 looks very different, not worth to subclass here;
- According to starfrost013 this is really a Weitek core ...

References:
- https://envytools.readthedocs.io/en/latest/hw/display/nv3/index.html
- https://86box.net/2025/02/25/riva128-part-1.html

**************************************************************************************************/

#include "emu.h"
#include "pc_vga_nvidia.h"

#define LOG_WARN      (1U << 1)
#define LOG_RMA       (1U << 2) // extensively accessed even on POST
#define LOG_CRTC      (1U << 3)

#define VERBOSE (LOG_GENERAL | LOG_WARN | LOG_CRTC)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)
#define LOGRMA(...)             LOGMASKED(LOG_RMA, __VA_ARGS__)
#define LOGCRTC(...)            LOGMASKED(LOG_CRTC, __VA_ARGS__)

DEFINE_DEVICE_TYPE(NVIDIA_NV3_VGA,  nvidia_nv3_vga_device,  "nvidia_nv3_vga",  "nVidia NV3 VGA i/f")

nvidia_nv3_vga_device::nvidia_nv3_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: svga_device(mconfig, NVIDIA_NV3_VGA, tag, owner, clock)
{
	m_main_if_space_config = address_space_config("io_regs", ENDIANNESS_LITTLE, 8, 4, 0, address_map_constructor(FUNC(nvidia_nv3_vga_device::io_3bx_3dx_map), this));
	m_crtc_space_config = address_space_config("crtc_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(nvidia_nv3_vga_device::crtc_map), this));
}

void nvidia_nv3_vga_device::device_add_mconfig(machine_config &config)
{
	// TODO: I2C, likely for DDC (AIDA16 extensively test that in non-safe mode)
}

static INPUT_PORTS_START(nv3_vga_sense)
	PORT_START("VGA_SENSE")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // causes a red screen at POST if low
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

ioport_constructor nvidia_nv3_vga_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(nv3_vga_sense);
}

void nvidia_nv3_vga_device::device_start()
{
	svga_device::device_start();
	zero();

	vga.crtc.maximum_scan_line = 1;

	// TODO: shared with main GPU, fake it for now
	// Start address ends at 20 bits so 0x1fffff mask / 2,097,151 bytes / 2MB window is (theorically) given
	// we assume that is really 4 MB, VESA tests extensively tests with 0x7f banks
	vga.svga_intf.vram_size = 4*1024*1024;

	for (int i = 0; i < 0x100; i++)
		set_pen_color(i, pal1bit(i & 1), pal1bit((i & 2) >> 1), pal1bit((i & 4) >> 2));

//  vga.miscellaneous_output = 0;
	m_ext_offset = 0;
	save_item(NAME(m_ext_offset));
}

void nvidia_nv3_vga_device::io_3bx_3dx_map(address_map &map)
{
	svga_device::io_3bx_3dx_map(map);
	map(0x00, 0x03).lrw8(
		NAME([this] (offs_t offset) {
			LOGRMA("RMA_ACCESS R %02x\n", offset & 3);
			return space().unmap();
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOGRMA("RMA_ACCESS W %02x -> %02x\n", offset & 3, data);
		})
	);
}

uint8_t nvidia_nv3_vga_device::mem_r(offs_t offset)
{
	if (svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en || svga.rgb32_en)
		return svga_device::mem_linear_r(offset + (svga.bank_r * 0x8000));
	return svga_device::mem_r(offset);
}

void nvidia_nv3_vga_device::mem_w(offs_t offset, uint8_t data)
{
	if (svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en || svga.rgb32_en)
	{
		svga_device::mem_linear_w(offset + (svga.bank_w * 0x8000), data);
		return;
	}
	svga_device::mem_w(offset, data);
}

uint16_t nvidia_nv3_vga_device::offset()
{
	// TODO: check true enable condition
	if (svga.rgb8_en || svga.rgb16_en || svga.rgb24_en || svga.rgb32_en)
		return (vga.crtc.offset << 3) + m_ext_offset;
	return svga_device::offset();
}

// TODO: lock mechanism
void nvidia_nv3_vga_device::crtc_map(address_map &map)
{
	svga_device::crtc_map(map);
	// REPAINT_0
/*
 * xxx- ---- Row offset bits 3-5
 * ---x xxxx Start address bits 16-20
 */
	map(0x19, 0x19).lrw8(
		NAME([this] (offs_t offset) {
			return m_repaint[0];
		}),
		NAME([this] (offs_t offset, u8 data) {
			// TODO: untested, likely used in extended text modes
			//vga.crtc.start_addr_latch &= ~0x1f0000;
			//vga.crtc.start_addr_latch |= ((data & 0x1f) << 16);
			m_ext_offset = ((data & 0xe0) >> 5) << 11;
			m_repaint[0] = data;
			//m_ext_offset = recalculate_ext_offset();
			LOGCRTC("EXT CRTC: REPAINT_0 %02x\n", data);
		})
	);
	// REPAINT_1
/*
 * ???? ?-?? <unknown, definitely used>
 * ---- -x-- Shifts row offset by 1 bit (verify)
 */
	map(0x1a, 0x1a).lrw8(
		NAME([this] (offs_t offset) {
			return m_repaint[1];
		}),
		NAME([this] (offs_t offset, u8 data) {
			// TODO: this definitely uses more regs in the equation
			//                      REPAINT_0 REPAINT_1 base_offset  <expected_extra_offset>
			// 10eh 320x200   16bpp 0x00      0x3c      0x50         560  / 0x230 (with scan doubler?)
			// 111h 640x480   16bpp 0x00      0x3c      0xa0         1120 / 0x460
			// 114h 800x600   16bpp 0x00      0x3c      0xc8         1400 / 0x578
			// 117h 1024x768  16bpp 0x20      0x3c      0x00         2048 / 0x800
			// 11ah 1280x1024 16bpp 0x20      0x38      0x40         2496 / 0x9c0
			// 132h 320x400   16bpp 0x00      0x3c      0x50         560  / 0x230
			// 13dh 640x400   16bpp 0x00      0x3c      0xa0         1120 / 0x460
			// 142h 1152x864  16bpp 0x20      0x3c      0x20         2272 / 0x8e0
			// 146h 1600x1200 16bpp 0x00      0x38      0x90         3056 / 0xbf0
			// 14bh 960x720   16bpp 0x10      0x3c      0xf0         1680 / 0x690
			m_repaint[1] = data;
			//m_ext_offset = recalculate_ext_offset();
			LOGCRTC("EXT CRTC: REPAINT_1 %02x\n", data);
		})
	);
	// WRITE_BANK
/*
 * --xx xxxx write bank, in 32k units
 */
	map(0x1d, 0x1d).lrw8(
		NAME([this] (offs_t offset) {
			return svga.bank_w;
		}),
		NAME([this] (offs_t offset, u8 data) {
			if (data & 0x80)
				LOGWARN("WRITE_BANK: attempt to write to undocumented bits %02x\n", data);
			svga.bank_w = data & 0x7f;
		})
	);
	// READ_BANK
/*
 * --xx xxxx read bank
 */
	map(0x1e, 0x1e).lrw8(
		NAME([this] (offs_t offset) {
			return svga.bank_r;
		}),
		NAME([this] (offs_t offset, u8 data) {
			if (data & 0x80)
				LOGWARN("READ_BANK: attempt to write to undocumented bits %02x\n", data);
			svga.bank_r = data & 0x7f;
		})
	);
	// EXTENDED_VERT
/*
 * ---x ---- Horizontal Total bit 8
 * ---- x--- Vertical Sync Start bit 10
 * ---- -x-- Vertical Blank Start bit 10
 * ---- --x- Vertical Display End bit 10
 * ---- ---x Vertical Total bit 10
 */
	map(0x25, 0x25).lrw8(
		NAME([this] (offs_t offset) {
			return vga.crtc.data[0x25];
		}),
		NAME([this] (offs_t offset, u8 data) {
			if (data & 0xe0)
				LOGWARN("EXTENDED_VERT: attempt to write to undocumented bits %02x\n", data);
			LOGCRTC("EXT CRTC: EXTENDED_VERT %02x\n", data & 0x1f);
			vga.crtc.horz_total =       (vga.crtc.horz_total & 0x00ff)       | ((BIT(data, 4) << 8));
			//vga.crtc.?? = (vga.crtc.?? & 0x03ff) | ((BIT(data, 3) << 8));
			vga.crtc.vert_blank_start = (vga.crtc.vert_blank_start & 0x03ff) | ((BIT(data, 2) << 10));
			vga.crtc.vert_disp_end =    (vga.crtc.vert_disp_end & 0x03ff)    | ((BIT(data, 1) << 10));
			vga.crtc.vert_total =       (vga.crtc.vert_total & 0x03ff)       | ((BIT(data, 0) << 10));
			vga.crtc.data[0x25] = data;
			recompute_params();
		})
	);
	// PIXEL_FMT
/*
 * ---- --xx Pixel Format
 * ---- --00 VGA
 * ---- --01 8bpp
 * ---- --10 16bpp
 * ---- --11 32bpp
 */
	map(0x28, 0x28).lrw8(
		NAME([this] (offs_t offset) {
			return svga.rgb32_en ? 3 : svga.rgb16_en ? 2 : svga.rgb8_en ? 1 : 0;
		}),
		NAME([this] (offs_t offset, u8 data) {
			if (data & 0xfc)
				LOGWARN("PIXEL_FMT: attempt to write to undocumented bits %02x\n", data);
			data &= 3;
			// NB: NV3 VESA doesn't have 15bpp support
			svga.rgb32_en = svga.rgb16_en = svga.rgb15_en = svga.rgb8_en = 0;
			switch(data)
			{
				case 0:
					LOGCRTC("PIXEL_FMT: VGA mode selected\n");
					break;
				case 1:
					svga.rgb8_en = 1;
					LOGCRTC("PIXEL_FMT: SVGA 8bpp mode selected\n");
					break;
				case 2:
					svga.rgb16_en = 1;
					LOGCRTC("PIXEL_FMT: SVGA 16bpp mode selected\n");
					break;
				case 3:
					svga.rgb32_en = 1;
					LOGCRTC("PIXEL_FMT: SVGA 32bpp mode selected\n");
					break;
			}
		})
	);
	// EXTENDED_HORZ
/*
 * ---- ---x Horizontal Display End bit 8
 */
	map(0x2d, 0x2d).lrw8(
		NAME([this] (offs_t offset) {
			return (vga.crtc.horz_disp_end >> 10) & 1;
		}),
		NAME([this] (offs_t offset, u8 data) {
			if (data & 0xfc)
				LOGWARN("EXTENDED_HORZ: attempt to write to undocumented bits %02x\n", data);
			LOGCRTC("EXT CRTC: EXTENDED_HORZ %02x\n", data & 1);
			vga.crtc.horz_disp_end = (vga.crtc.horz_disp_end & 0x00ff) | ((data & 0x01) << 8);
			recompute_params();
		})
	);
	// RMA_MODE
/*
 * ---- xxx- RMA index
 * ---- ---x enable RMA
 */
	map(0x38, 0x38).lrw8(
		NAME([this] (offs_t offset) {
			u8 res = vga.crtc.data[0x38];
			LOGRMA("RMA_MODE read %02x\n", res);
			return res;
		}),
		NAME([this] (offs_t offset, u8 data) {
			vga.crtc.data[0x38] = data;
			LOGRMA("RMA_MODE write %s, index %d\n", data & 1 ? "enable" : "disable", (data & 0xe) >> 1);
		})
	);
	// I2C_READ
/*
 * ---- x--- SDA
 * ---- -x-- SCL
 */
// map(0x3e, 0x3e)
	// I2C_WRITE
/*
 * ---x ---- SCL
 * ---- x--- SDA
 */
// map(0x3f, 0x3f)
}
