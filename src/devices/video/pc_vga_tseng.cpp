// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * Tseng Labs VGA family
 *
 * TODO:
 * - ET3000 (VGA only?)
 * - ET4000AX
 * \- No logging whatsoever;
 * \- Unsupported True Color modes, also "Return current video mode failed" in VESA24_2 test;
 * - ET4000/W32 (2d accelerator)
 * - ET4000/W32p (PCI version)
 *
 */

#include "emu.h"
#include "pc_vga_tseng.h"

// TODO: refactor this macro
#define GRAPHIC_MODE (vga.gc.alpha_dis) /* else text mode */

DEFINE_DEVICE_TYPE(TSENG_VGA,  tseng_vga_device,  "tseng_vga",  "Tseng Labs ET4000AX VGA i/f")

tseng_vga_device::tseng_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: svga_device(mconfig, TSENG_VGA, tag, owner, clock)
{
	m_main_if_space_config = address_space_config("io_regs", ENDIANNESS_LITTLE, 8, 4, 0, address_map_constructor(FUNC(tseng_vga_device::io_3bx_3dx_map), this));
	m_crtc_space_config = address_space_config("crtc_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(tseng_vga_device::crtc_map), this));
	m_seq_space_config = address_space_config("sequencer_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(tseng_vga_device::sequencer_map), this));
	m_atc_space_config = address_space_config("attribute_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(tseng_vga_device::attribute_map), this));
}

void tseng_vga_device::device_start()
{
	svga_device::device_start();
	memset(&et4k, 0, sizeof(et4k));

	save_item(NAME(et4k.reg_3d8));
	save_item(NAME(et4k.dac_ctrl));
	save_item(NAME(et4k.dac_state));
	save_item(NAME(et4k.horz_overflow));
	save_item(NAME(et4k.aux_ctrl));
	save_item(NAME(et4k.ext_reg_ena));
	save_item(NAME(et4k.misc1));
	save_item(NAME(et4k.misc2));
}

void tseng_vga_device::io_3bx_3dx_map(address_map &map)
{
	svga_device::io_3bx_3dx_map(map);
	map(0x08, 0x08).lrw8(
		NAME([this] (offs_t offset) {
			return et4k.reg_3d8;
		}),
		NAME([this] (offs_t offset, u8 data) {
			et4k.reg_3d8 = data;
			if(data == 0xa0)
				et4k.ext_reg_ena = true;
			else if(data == 0x29)
				et4k.ext_reg_ena = false;
		})
	);
}

void tseng_vga_device::io_3cx_map(address_map &map)
{
	svga_device::io_3cx_map(map);
	map(0x06, 0x06).rw(FUNC(tseng_vga_device::ramdac_hidden_mask_r), FUNC(tseng_vga_device::ramdac_hidden_mask_w));
	map(0x08, 0x08).r(FUNC(tseng_vga_device::ramdac_hidden_windex_r));
	map(0x0d, 0x0d).lrw8(
		NAME([this] (offs_t offset) {
			u8 res = svga.bank_w & 0xf;
			res   |= (svga.bank_r & 0xf) << 4;
			return res;
		}),
		NAME([this] (offs_t offset, u8 data) {
			svga.bank_w = data & 0xf;
			svga.bank_r = (data & 0xf0) >> 4;
		})
	);
}

u8 tseng_vga_device::ramdac_hidden_mask_r(offs_t offset)
{
	if(et4k.dac_state == 4)
	{
		if(!et4k.dac_ctrl)
			et4k.dac_ctrl = 0x80;
		return et4k.dac_ctrl;
	}
	if (!machine().side_effects_disabled())
		et4k.dac_state++;
	return vga_device::ramdac_mask_r(offset);
}

void tseng_vga_device::ramdac_hidden_mask_w(offs_t offset, u8 data)
{
	if(et4k.dac_state == 4)
	{
		et4k.dac_ctrl = data;
		recompute_params();
		return;
	}

	vga_device::ramdac_write_index_w(offset, data);
}

u8 tseng_vga_device::ramdac_hidden_windex_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		et4k.dac_state = 0;
	return vga_device::ramdac_write_index_r(offset);
}

// NOTE: the mapping notes comes from ET4000/W32i manual, unconfirmed if they are identical
void tseng_vga_device::crtc_map(address_map &map)
{
	svga_device::crtc_map(map);
//  map(0x30, 0x30) System Segment Map Comparator
//  map(0x31, 0x31) General Purpose (& Clock Select 3/4)
//  map(0x32, 0x32) RAS/CAS Configuration (RCCONF)
//  map(0x33, 0x33) Extended Start Address
	// Auxiliary Control
	map(0x34, 0x34).lrw8(
		NAME([this] (offs_t offset) {
			return et4k.aux_ctrl;
		}),
		NAME([this] (offs_t offset, u8 data) {
			et4k.aux_ctrl = data;
			recompute_params();
		})
	);
//  map(0x35, 0x35) Overflow High
//  map(0x36, 0x36) Video System Configuration 1 (VSCONF1)
	// Video System Configuration 2 (VSCONF2)
	// NOTE: reads memory installed from here
	map(0x37, 0x37).ram();
	// Horizontal overflow
	map(0x3f, 0x3f).lrw8(
		NAME([this] (offs_t offset) {
			return et4k.horz_overflow;
		}),
		NAME([this] (offs_t offset, u8 data) {
			et4k.horz_overflow = data;
			vga.crtc.horz_total = (vga.crtc.horz_total & 0xff) | ((data & 1) << 8);
			recompute_params();
		})
	);
}

void tseng_vga_device::sequencer_map(address_map &map)
{
	svga_device::sequencer_map(map);
	// TODO: preseve legacy hookup, to be investigated
	map(0x05, 0xff).unmaprw();
//  map(0x06, 0x06) TS State Control
//  map(0x07, 0x07) TS Auxiliary Mode
}

void tseng_vga_device::attribute_map(address_map &map)
{
	map.global_mask(0x3f);
	map.unmap_value_high();
	svga_device::attribute_map(map);
	// Miscellaneous 1
	/*
	 * x--- ---- Bypass the internal palette
	 * -x-- ---- 2 byte character code (presumably for the Korean variants TBD)
	 * --xx ---- Select High resolution/color mode
	 * --00 ---- Normal power-up
	 * --01 ---- <reserved>
	 * --10 ---- 8bpp
	 * --11 ---- 16bpp
	 * ---- xxxx <reserved>
	 */
	// TODO: implement KEY protection
	map(0x16, 0x16).mirror(0x20).lrw8(
		NAME([this] (offs_t offset) { return et4k.misc1; }),
		NAME([this] (offs_t offset, u8 data) {
			et4k.misc1 = data;
			// TODO: this should be taken into account for recompute_params
			#if 0
			svga.rgb8_en = 0;
			svga.rgb15_en = 0;
			svga.rgb16_en = 0;
			svga.rgb32_en = 0;
			switch(et4k.misc1 & 0x30)
			{
				case 0:
					// normal power-up mode
					break;
				case 0x10:
					svga.rgb8_en = 1;
					break;
				case 0x20:
				case 0x30:
					popmessage("Tseng 15/16 bit HiColor mode, contact MAMEdev");
					break;
			}
			#endif
		})
	);
	// Miscellaneous 2
	// TODO: not on stock et4k?
	map(0x17, 0x17).mirror(0x20).lrw8(
		NAME([this] (offs_t offset) { return et4k.misc2; }),
		NAME([this] (offs_t offset, u8 data) {
			et4k.misc2 = data;
		})
	);
}

void tseng_vga_device::recompute_params()
{
	int divisor;
	int xtal = 0;
	svga.rgb8_en = 0;
	svga.rgb15_en = 0;
	svga.rgb16_en = 0;
	svga.rgb24_en = 0;
	switch(((et4k.aux_ctrl << 1) & 4)|(vga.miscellaneous_output & 0xc)>>2)
	{
		case 0:
			xtal = XTAL(25'174'800).value();
			break;
		case 1:
			xtal = XTAL(28'636'363).value();
			break;
		case 2:
			xtal = 16257000*2; //2xEGA clock
			break;
		case 3:
			xtal = XTAL(40'000'000).value();
			break;
		case 4:
			xtal = XTAL(36'000'000).value();
			break;
		case 5:
			xtal = XTAL(45'000'000).value();
			break;
		case 6:
			xtal = 31000000;
			break;
		case 7:
			xtal = 38000000;
			break;
	}
	switch(et4k.dac_ctrl & 0xe0)
	{
		case 0xa0:
			svga.rgb15_en = 1;
			divisor = 2;
			break;
		case 0xe0:
			svga.rgb16_en = 1;
			divisor = 2;
			break;
		case 0x60:
			svga.rgb24_en = 1;
			divisor = 3;
			xtal *= 2.0f/3.0f;
			break;
		default:
			svga.rgb8_en = (!(vga.sequencer.data[1] & 8) && (vga.sequencer.data[4] & 8) && vga.gc.shift256 && vga.crtc.div2 && GRAPHIC_MODE);
			divisor = 1;
			break;
	}
	recompute_params_clock(divisor, xtal);
}


uint8_t tseng_vga_device::mem_r(offs_t offset)
{
	if(svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en)
	{
		offset &= 0xffff;
		return vga.memory[(offset+svga.bank_r*0x10000)];
	}

	return vga_device::mem_r(offset);
}

void tseng_vga_device::mem_w(offs_t offset, uint8_t data)
{
	if(svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en)
	{
		offset &= 0xffff;
		vga.memory[(offset+svga.bank_w*0x10000)] = data;
	}
	else
		vga_device::mem_w(offset,data);
}
