// license:BSD-3-Clause
// copyright-holders:Barry Rodewald, Angelo Salese
/*
 * Tseng Labs VGA family
 *
 * TODO:
 * - ET3000 (VGA only?)
 * - ET4000AX
 * \- No logging whatsoever;
 * \- Unsupported True Color modes, also "Return current video mode failed" in VESA24_2 test;
 * - ET4000/W32i (2d accelerator, VBE 1.2)
 * \- MMIO ports
 * \- ACL BitBlt at I/O $21xy (x = IOD pin selectable?)
 * \- Secondary CRTC controller (CRTCB/Sprite)
 * - ET4000/W32p (PCI version of above)
 * - ET6000/ET6100
 *
 * Notes:
 * - Regular et4k BIOS isn't VBE compliant, fails SDD/VBETEST setups and (at very least) has issues
 *   rendering Toshinden title/gameplay demo. These issues are not present with W32i.
 *
 */

#include "emu.h"
#include "pc_vga_tseng.h"

// TODO: refactor this macro
#define GRAPHIC_MODE (vga.gc.alpha_dis) /* else text mode */

DEFINE_DEVICE_TYPE(TSENG_VGA,    tseng_vga_device,    "tseng_vga",    "Tseng Labs ET4000AX VGA i/f")
DEFINE_DEVICE_TYPE(ET4KW32I_VGA, et4kw32i_vga_device, "et4kw32i_vga", "Tseng Labs ET4000/W32i TC6167HF VGA i/f")

tseng_vga_device::tseng_vga_device(const machine_config &mconfig, const char *tag, device_type type, device_t *owner, uint32_t clock)
	: svga_device(mconfig, type, tag, owner, clock)
{
	m_main_if_space_config = address_space_config("io_regs", ENDIANNESS_LITTLE, 8, 4, 0, address_map_constructor(FUNC(tseng_vga_device::io_3bx_3dx_map), this));
	m_crtc_space_config = address_space_config("crtc_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(tseng_vga_device::crtc_map), this));
	m_seq_space_config = address_space_config("sequencer_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(tseng_vga_device::sequencer_map), this));
	m_atc_space_config = address_space_config("attribute_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(tseng_vga_device::attribute_map), this));
}

tseng_vga_device::tseng_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tseng_vga_device(mconfig, tag, TSENG_VGA, owner, clock)
{
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
	save_item(NAME(et4k.rcconf));
	save_item(NAME(et4k.vsconf1));
	save_item(NAME(et4k.vsconf2));
	save_item(NAME(et4k.crtc_reg31));
	save_item(NAME(et4k.crtc_ext_start));
	save_item(NAME(et4k.crtc_overflow_high));
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

void tseng_vga_device::crtc_map(address_map &map)
{
	svga_device::crtc_map(map);
//  map(0x30, 0x30) System Segment Map Comparator
//  General Purpose (& Clock Select 3/4)
	map(0x31, 0x31).lrw8(
		NAME([this] (offs_t offset) {
			return et4k.crtc_reg31;
		}),
		NAME([this] (offs_t offset, u8 data) {
			et4k.crtc_reg31 = data;
			// TODO: recompute_params
		})
	);
//  RAS/CAS Configuration (RCCONF)
	map(0x32, 0x32).lrw8(
		NAME([this] (offs_t offset) {
			return et4k.rcconf;
		}),
		NAME([this] (offs_t offset, u8 data) {
			et4k.rcconf = data;
		})
	);
	/*
	 * ---- xx-- Cursor address bits 16-17
	 * ---- --xx Start address bits 16-17
	 */
//  Extended Start Address
	map(0x33, 0x33).lrw8(
		NAME([this] (offs_t offset) {
			return et4k.crtc_ext_start;
		}),
		NAME([this] (offs_t offset, u8 data) {
			et4k.crtc_ext_start = data;
			vga.crtc.start_addr_latch &= ~0x30000;
			vga.crtc.start_addr_latch |= ((data & 0x3) << 16);
			vga.crtc.cursor_addr &= ~0x30000;
			vga.crtc.cursor_addr |= ((data & 0xc) << 14);
		})
	);
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
//  Overflow High
	map(0x35, 0x35).lrw8(
		NAME([this] (offs_t offset) {
			return et4k.crtc_overflow_high;
		}),
		NAME([this] (offs_t offset, u8 data) {
			et4k.crtc_overflow_high = data;
			vga.crtc.vert_blank_start = (vga.crtc.vert_blank_start & 0x03ff) | ((BIT(data, 0) << 10));
			vga.crtc.vert_total = (vga.crtc.vert_total & 0x03ff) | ((BIT(data, 1) << 10));
			vga.crtc.vert_disp_end = (vga.crtc.vert_total & 0x03ff) | ((BIT(data, 2) << 10));
			// TODO: vertical sync start -> retrace?
			vga.crtc.vert_retrace_start = (vga.crtc.vert_retrace_start & 0x03ff) | ((BIT(data, 3) << 10));
			vga.crtc.line_compare = (vga.crtc.line_compare & 0x03ff) | ((BIT(data, 4) << 10));
			// TODO: bit 5: external sync reset (genlock)
			// TODO: bit 6 Alternate RMW control
			// TODO: bit 7 vertical interlace mode
		})
	);
	//  Video System Configuration 1 (VSCONF1)
	map(0x36, 0x36).lrw8(
		NAME([this] (offs_t offset) {
			return et4k.vsconf1;
		}),
		NAME([this] (offs_t offset, u8 data) {
			et4k.vsconf1 = data;
		})
	);
	// Video System Configuration 2 (VSCONF2)
	map(0x37, 0x37).lrw8(
		NAME([this] (offs_t offset) {
			// NOTE: reads memory installed from here and rcconf
			return et4k.vsconf2;
		}),
		NAME([this] (offs_t offset, u8 data) {
			et4k.vsconf2 = data;
		})
	);
	// Horizontal overflow
	// NOTE: undocumented in ET4000AX, may apply to w32i only
	map(0x3f, 0x3f).lrw8(
		NAME([this] (offs_t offset) {
			return et4k.horz_overflow;
		}),
		NAME([this] (offs_t offset, u8 data) {
			et4k.horz_overflow = data;
			vga.crtc.horz_total = (vga.crtc.horz_total & 0xff) | ((data & 1) << 8);
			vga.crtc.offset = (vga.crtc.offset & 0x00ff) | ((data & 0x80) << 1);
			// TODO: bits 4 & 2 (horizontal sync and blank start, bit 8)
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
			recompute_params();
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
	// TODO: also read et4k.misc1?
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
		return svga_device::mem_linear_r(offset + svga.bank_r * 0x10000);
	}

	return vga_device::mem_r(offset);
}

void tseng_vga_device::mem_w(offs_t offset, uint8_t data)
{
	if(svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en)
	{
		offset &= 0xffff;
		svga_device::mem_linear_w(offset + svga.bank_w * 0x10000, data);
		return;
	}

	vga_device::mem_w(offset,data);
}

uint32_t tseng_vga_device::latch_start_addr()
{
	// TODO: condition for this (SDD scroll/buffer tests)
	if(svga.rgb8_en)
	{
		return vga.crtc.start_addr_latch << 2;
	}
	return vga.crtc.start_addr_latch;
}

/**************************************
 *
 * ET4000W32/i overrides
 *
 *************************************/

et4kw32i_vga_device::et4kw32i_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tseng_vga_device(mconfig, tag, ET4KW32I_VGA, owner, clock)
{
	m_acl_space_config = address_space_config("acl_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(et4kw32i_vga_device::acl_map), this));
	m_mmu_space_config = address_space_config("mmu_regs", ENDIANNESS_LITTLE, 8, 15, 0, address_map_constructor(FUNC(et4kw32i_vga_device::mmu_map), this));

	m_acl_idx = 0;
	m_ima.control = 0;
}

device_memory_interface::space_config_vector et4kw32i_vga_device::memory_space_config() const
{
	auto r = svga_device::memory_space_config();
	r.emplace_back(std::make_pair(EXT_REG,     &m_acl_space_config));
	r.emplace_back(std::make_pair(EXT_REG + 1, &m_mmu_space_config));
	return r;
}

void et4kw32i_vga_device::device_start()
{
	tseng_vga_device::device_start();

	save_item(NAME(m_acl_idx));

	save_item(NAME(m_crtcb.xpos));
	save_item(NAME(m_crtcb.ypos));
	save_item(NAME(m_crtcb.address));

	save_item(NAME(m_ima.control));
}

void et4kw32i_vga_device::crtc_map(address_map &map)
{
	tseng_vga_device::crtc_map(map);
	/*
	 * xxxx ---- Cursor address bits 16-19
	 * ---- xxxx Start address bits 16-19
	 */
//  Extended Start Address
	map(0x33, 0x33).lrw8(
		NAME([this] (offs_t offset) {
			return et4k.crtc_ext_start;
		}),
		NAME([this] (offs_t offset, u8 data) {
			et4k.crtc_ext_start = data;
			vga.crtc.start_addr_latch &= ~0xf0000;
			vga.crtc.start_addr_latch |= ((data & 0xf) << 16);
			vga.crtc.cursor_addr &= ~0xf0000;
			vga.crtc.cursor_addr |= ((data & 0xf0) << 12);
		})
	);
}

void et4kw32i_vga_device::io_3cx_map(address_map &map)
{
	tseng_vga_device::io_3cx_map(map);
	map(0x0b, 0x0b).lrw8(
		NAME([this] (offs_t offset) {
			u8 res = (svga.bank_w & 0x30) >> 4;
			res   |= (svga.bank_r & 0x30);
			return res;
		}),
		NAME([this] (offs_t offset, u8 data) {
			svga.bank_w &= 0x0f;
			svga.bank_w |= (data & 0x3) << 4;
			svga.bank_r &= 0x0f;
			svga.bank_r |= (data & 0x30);
		})
	);
	map(0x0d, 0x0d).lrw8(
		NAME([this] (offs_t offset) {
			u8 res = svga.bank_w & 0xf;
			res   |= (svga.bank_r & 0xf) << 4;
			return res;
		}),
		NAME([this] (offs_t offset, u8 data) {
			svga.bank_w &= 0x30;
			svga.bank_w |= (data & 0xf);
			svga.bank_r &= 0x30;
			svga.bank_r |= (data & 0xf0) >> 4;
		})
	);
}

u8 et4kw32i_vga_device::acl_index_r(offs_t offset)
{
	return m_acl_idx;
}

void et4kw32i_vga_device::acl_index_w(offs_t offset, u8 data)
{
	m_acl_idx = data;
}

u8 et4kw32i_vga_device::acl_data_r(offs_t offset)
{
	return space(EXT_REG).read_byte(m_acl_idx);
}

void et4kw32i_vga_device::acl_data_w(offs_t offset, u8 data)
{
	space(EXT_REG).write_byte(m_acl_idx, data);
}

// TODO: sketchy, essentially stacks MMU on top of normally mirrored VGA memory
uint8_t et4kw32i_vga_device::mem_r(offs_t offset)
{
	if(svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en)
	{
		if (et4k.vsconf1 & 0x28 && vga.gc.memory_map_sel)
		{
			const u32 mmu_address = vga.gc.memory_map_sel & 2 ? 0x08000 : 0x18000;

			if ((offset & 0x18000) == mmu_address)
				return space(EXT_REG + 1).read_byte(offset & 0x7fff);
		}

		offset &= 0xffff;
		return svga_device::mem_linear_r(offset + svga.bank_r * 0x10000);
	}

	return vga_device::mem_r(offset);
}

void et4kw32i_vga_device::mem_w(offs_t offset, uint8_t data)
{
	if(svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en)
	{
		if (et4k.vsconf1 & 0x28 && vga.gc.memory_map_sel)
		{
			const u32 mmu_address = vga.gc.memory_map_sel & 2 ? 0x08000 : 0x18000;

			if ((offset & 0x18000) == mmu_address)
			{
				space(EXT_REG + 1).write_byte(offset & 0x7fff, data);
				return;
			}
		}

		offset &= 0xffff;
		svga_device::mem_linear_w(offset + svga.bank_w * 0x10000, data);
		return;
	}

	vga_device::mem_w(offset,data);
}

/*
 * MMU & ACL interactions
 */

template <unsigned N> u8 et4kw32i_vga_device::mmu_blit_r(offs_t offset)
{
	if (m_mmu.control & 1 << N)
	{
		// To FIFO, TBD
		return 0;
	}

	return svga_device::mem_linear_r(offset + m_mmu.base_address[N]);
}

template <unsigned N> void et4kw32i_vga_device::mmu_blit_w(offs_t offset, u8 data)
{
	if (m_mmu.control & 1 << N)
	{
		// To FIFO, TBD
		return;
	}

	svga_device::mem_linear_w(offset + m_mmu.base_address[N], data);
}

template <unsigned N> u8 et4kw32i_vga_device::mmu_base_address_r(offs_t offset)
{
	return m_mmu.base_address[N] >> (offset * 8);
}

template <unsigned N> void et4kw32i_vga_device::mmu_base_address_w(offs_t offset, u8 data)
{
	const u8 shift = offset * 8;
	const u32 mask = ~(0xff << shift);
	m_mmu.base_address[N] &= mask;
	m_mmu.base_address[N] |= (data << shift);
}


void et4kw32i_vga_device::mmu_map(address_map &map)
{
	map(0x0000, 0x1fff).rw(FUNC(et4kw32i_vga_device::mmu_blit_r<0>), FUNC(et4kw32i_vga_device::mmu_blit_w<0>));
	map(0x2000, 0x3fff).rw(FUNC(et4kw32i_vga_device::mmu_blit_r<1>), FUNC(et4kw32i_vga_device::mmu_blit_w<1>));
	map(0x4000, 0x5fff).rw(FUNC(et4kw32i_vga_device::mmu_blit_r<2>), FUNC(et4kw32i_vga_device::mmu_blit_w<2>));
	map(0x6000, 0x6003).mirror(0x1f00).rw(FUNC(et4kw32i_vga_device::mmu_base_address_r<0>), FUNC(et4kw32i_vga_device::mmu_base_address_w<0>));
	map(0x6004, 0x6007).mirror(0x1f00).rw(FUNC(et4kw32i_vga_device::mmu_base_address_r<1>), FUNC(et4kw32i_vga_device::mmu_base_address_w<1>));
	map(0x6008, 0x600b).mirror(0x1f00).rw(FUNC(et4kw32i_vga_device::mmu_base_address_r<2>), FUNC(et4kw32i_vga_device::mmu_base_address_w<2>));
	map(0x6013, 0x6013).mirror(0x1f00).lrw8(
		NAME([this] (offs_t offset) {
			return m_mmu.control;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_mmu.control = data;
		})
	);
}

void et4kw32i_vga_device::acl_map(address_map &map)
{
	map(0xe0, 0xe1).lrw8(
		NAME([this] (offs_t offset) {
			return m_crtcb.xpos >> (offset * 8);
		}),
		NAME([this] (offs_t offset, u8 data) {
			if (offset)
			{
				m_crtcb.xpos &= 0xff;
				m_crtcb.xpos |= (data & 7) << 8;
			}
			else
			{
				m_crtcb.xpos &= 0x700;
				m_crtcb.xpos |= (data & 0xff);
			}
		})
	);
	map(0xe4, 0xe5).lrw8(
		NAME([this] (offs_t offset) {
			return m_crtcb.ypos >> (offset * 8);
		}),
		NAME([this] (offs_t offset, u8 data) {
			if (offset)
			{
				m_crtcb.ypos &= 0xff;
				m_crtcb.ypos |= (data & 7) << 8;
			}
			else
			{
				m_crtcb.ypos &= 0x700;
				m_crtcb.ypos |= (data & 0xff);
			}
		})
	);

	map(0xe8, 0xea).lrw8(
		NAME([this] (offs_t offset) {
			return m_crtcb.address >> (offset * 8);
		}),
		NAME([this] (offs_t offset, u8 data) {
			const u8 shift = offset * 8;
			const u32 mask = ~(0xff << shift);
			m_crtcb.address &= mask;
			m_crtcb.address |= (data << shift);
			m_crtcb.address &= 0xfffff;
		})
	);

	/*
	 * x--- ---- CRTCB enable
	 * -x-- ---- Token outputs/External Sprite enable
	 * --xx xx-- <reserved>, always 1000
	 * ---- --x- Interlace Image Port address
	 * ---- ---x Image Port enable
	 */
	map(0xf7, 0xf7).lrw8(
		NAME([this] (offs_t offset) {
			return m_ima.control;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_ima.control = data;
			if (data & 0x5f)
				popmessage("pc_vga_tseng.cpp: IMA $f7 write %02x", data);
		})
	);
}

uint32_t et4kw32i_vga_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	svga_device::screen_update(screen, bitmap, cliprect);

	// HW cursor
	// TODO: enough for Win95 in 8bpp and not much else
	if (BIT(m_ima.control, 7))
	{
		const u32 base_offs = (m_crtcb.address << 2) + 0x3f0;
		const u8 transparent_pen = 2;

		for (int y = 0; y < 32; y ++)
		{
			int res_y = y + m_crtcb.ypos;
			for (int x = 0; x < 32; x++)
			{
				int res_x = x + m_crtcb.xpos;
				if (!cliprect.contains(res_x, res_y))
					continue;
				// TODO: odd bytes
				const u32 cursor_address = (((x >> 2) + y * 16) << 1) + base_offs;

				const int xi = (x & 3) * 2;
				u8 cursor_gfx =  (vga.memory[(cursor_address) % vga.svga_intf.vram_size] >> xi) & 3;

				// TODO: pen 3 really RMW with a xor
				if (cursor_gfx == transparent_pen)
					continue;

				bitmap.pix(res_y, res_x) = cursor_gfx & 1 ? 0xffffff : 0x000000;
			}
		}
	}

	return 0;
}

