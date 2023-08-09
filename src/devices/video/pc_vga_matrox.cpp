// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#include "emu.h"
#include "pc_vga_matrox.h"

DEFINE_DEVICE_TYPE(MATROX_VGA,  matrox_vga_device,  "matrox_vga",  "Matrox MGA2064W VGA")

matrox_vga_device::matrox_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: svga_device(mconfig, MATROX_VGA, tag, owner, clock)
{
	m_main_if_space_config = address_space_config("io_regs", ENDIANNESS_LITTLE, 8, 4, 0, address_map_constructor(FUNC(matrox_vga_device::io_3bx_3dx_map), this));
	// 3 bits of address space?
	m_crtcext_space_config = address_space_config("crtcext_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(matrox_vga_device::crtcext_map), this));
	// TODO: docs mentions using 0x22 / 0x24 / 0x26 for regular CRTC (coming from plain VGA?)

	m_ramdac_indexed_space_config = address_space_config("ramdac_indexed_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(matrox_vga_device::ramdac_indexed_map), this));
}

device_memory_interface::space_config_vector matrox_vga_device::memory_space_config() const
{
	auto r = svga_device::memory_space_config();
	r.emplace_back(std::make_pair(EXT_REG,     &m_crtcext_space_config));
	r.emplace_back(std::make_pair(EXT_REG + 1, &m_ramdac_indexed_space_config));
	return r;
}

void matrox_vga_device::device_reset()
{
	svga_device::device_reset();

	m_crtcext_index = 0;
	m_crtcext_misc = 0;
	m_crtcext_horz_counter = 0;
	m_mgamode = false;
	m_interlace_mode = false;
	m_truecolor_ctrl = 0x80;
	m_multiplex_ctrl = 0x98;
}

void matrox_vga_device::io_3bx_3dx_map(address_map &map)
{
	svga_device::io_3bx_3dx_map(map);
	map(0x0e, 0x0e).lrw8(
		NAME([this] (offs_t offset) {
			return m_crtcext_index;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_crtcext_index = data;
		})
	);
	map(0x0f, 0x0f).lrw8(
		NAME([this] (offs_t offset) {
			return space(EXT_REG).read_byte(m_crtcext_index);
		}),
		NAME([this] (offs_t offset, u8 data) {
			space(EXT_REG).write_byte(m_crtcext_index, data);
		})
	);
}

// "CRTCEXT*"
void matrox_vga_device::crtcext_map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(matrox_vga_device::crtcext0_address_gen_r), FUNC(matrox_vga_device::crtcext0_address_gen_w));
	map(0x01, 0x01).rw(FUNC(matrox_vga_device::crtcext1_horizontal_counter_r), FUNC(matrox_vga_device::crtcext1_horizontal_counter_w));
	map(0x02, 0x02).rw(FUNC(matrox_vga_device::crtcext2_vertical_counter_r), FUNC(matrox_vga_device::crtcext2_vertical_counter_w));
	map(0x03, 0x03).rw(FUNC(matrox_vga_device::crtcext3_misc_r), FUNC(matrox_vga_device::crtcext3_misc_w));
	// CRTCEXT4 Memory Page register
	map(0x04, 0x04).lrw8(
		NAME([this] (offs_t offset) { return svga.bank_w & 0x7f; }),
		NAME([this] (offs_t offset, u8 data) { svga.bank_w = data & 0x7f; })
	);
//  map(0x05, 0x05) Horizontal Video Half Count
//  map(0x06, 0x07) <Reserved>
//  \- $07 is actually checked by VESA test (PC=0xc62bb in rev3),
//     seems to disable SVGA drawing -> diagnostic check?
}

/*
 * CRTCEXT0 Address Generator extensions
 *
 * x--- ---- Interlace mode
 * --xx ---- Offset bits 9-8
 * ---- xxxx Start Address bits 19-16
 */
u8 matrox_vga_device::crtcext0_address_gen_r()
{
	u8 res = ((vga.crtc.start_addr >> 16) & 0xf);
	res   |= ((vga.crtc.offset >> 8) & 3) << 4;
	res   |= (m_interlace_mode) << 7;
	return res;
}

void matrox_vga_device::crtcext0_address_gen_w(offs_t offset, u8 data)
{
	m_interlace_mode = bool(BIT(data, 7));
	vga.crtc.offset     = (vga.crtc.offset & 0xff)       | ((data & 0x30) << 4);
	vga.crtc.start_addr = (vga.crtc.start_addr & 0xffff) | ((data & 0xf) << 16);
	if (m_interlace_mode)
		popmessage("MGA2064W: interlace mode enable");
//  recompute_params();
}

/*
 * CRTCEXT1 Horizontal Counter Extensions
 *
 * x--- ---- VRSTEN Vertical reset enable
 * -x-- ---- HBLKEND Horizontal end blank bit 6 (MGA mode only)
 * --x- ---- VSYNCOFF
 * ---x ---- HSYNCOFF
 * ---- x--- HRSTEN Horizontal reset enable
 * ---- -x-- HSYNCSTR horizontal retrace start bit 8
 * ---- --x- HBLKSTR horizontal blanking start bit 8
 * ---- ---x HTOTAL bit 8
 */
u8 matrox_vga_device::crtcext1_horizontal_counter_r()
{
	return m_crtcext_horz_counter;
}

void matrox_vga_device::crtcext1_horizontal_counter_w(offs_t offset, u8 data)
{
	// TODO: honor CRTC protect enable
	m_crtcext_horz_counter = data;
	vga.crtc.horz_total         = (vga.crtc.horz_total & 0xff)         | (BIT(data, 0) << 8);
	vga.crtc.horz_blank_start   = (vga.crtc.horz_blank_start & 0xff)   | (BIT(data, 1) << 8);
	vga.crtc.horz_retrace_start = (vga.crtc.horz_retrace_start & 0xff) | (BIT(data, 2) << 8);
	vga.crtc.horz_blank_end     = (vga.crtc.horz_blank_end & 0x3f)     | (BIT(data, 6) << 6);

	logerror("MGA2064W: CRTCEXT1 reset enable %02x syncoff %02x\n",data & 0x88, data & 0x30);
	recompute_params();
}

/*
 * CRTCEXT1 Vertical Counter Extensions
 *
 * x--- ---- LINECOMP Line compare bit 10
 * -xx- ---- VSYNCSTR Vertical retrace start bits 11-10
 * ---x x--- VBLKSTR Vertical blank start bits 11-10
 * ---- -x-- VDISPEND Vertical display end bit 10
 * ---- --xx VTOTAL bits 11-10
 */
u8 matrox_vga_device::crtcext2_vertical_counter_r()
{
	return m_crtcext_vert_counter;
}

void matrox_vga_device::crtcext2_vertical_counter_w(offs_t offset, u8 data)
{
	// TODO: honor CRTC protect enable
	m_crtcext_vert_counter = data;
	vga.crtc.vert_total         = (vga.crtc.vert_total & 0x3ff)         | ((data & 3) << 10);
	vga.crtc.vert_disp_end      = (vga.crtc.vert_disp_end & 0x3ff)      | (BIT(data, 2) << 10);
	vga.crtc.vert_blank_start   = (vga.crtc.vert_blank_start & 0x3ff)   | ((data & 0x18) << (10-3));
	vga.crtc.vert_retrace_start = (vga.crtc.vert_retrace_start & 0x3ff) | ((data & 0x60) << (10-5));
	vga.crtc.line_compare       = (vga.crtc.line_compare & 0x3ff)       | (BIT(data, 7) << 10);
	recompute_params();
}

/*
 * CRTCEXT3 Miscellaneous
 *
 * x--- ---- MGA mode (SVGA and accelerated modes)
 * -x-- ---- CSYNCEN
 * --x- ---- SLOW256 disables HW acceleration if '1' for VGA mode 13h
 * ---x x--- VIDDELAY delay for CRTC signals, depends on RAM configuration
 * ---0 0--- 4MB board
 * ---0 1--- 2MB board
 * ---1 x--- 8MB board
 * ---- -xxx SCALE dot clock scaling factor
 * ---- -000 /1
 * ---- -001 /2
 * ---- -010 /3
 * ---- -011 /4
 * ---- -100 <reserved>
 * ---- -101 /6
 * ---- -110 <reserved>
 * ---- -111 /8
 */
u8 matrox_vga_device::crtcext3_misc_r()
{
	return m_crtcext_misc;
}

void matrox_vga_device::crtcext3_misc_w(offs_t offset, u8 data)
{
	logerror("CRTCEXT3: %02x\n", data);
	m_crtcext_misc = data;
	m_mgamode = bool(BIT(data, 7));
}

/*
 * RAMDAC
 * - paired with a Texas Instruments TVP3026 here -> a superset of INMOS IMSG176/IMSG178
 * - integrated and customized with supersets in the next iteration (Mystique MGA-1064SG) and onward
 */
void matrox_vga_device::ramdac_ext_map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(matrox_vga_device::ramdac_write_index_r), FUNC(matrox_vga_device::ramdac_write_index_w));
	map(0x01, 0x01).rw(FUNC(matrox_vga_device::ramdac_data_r), FUNC(matrox_vga_device::ramdac_data_w));
	map(0x02, 0x02).rw(FUNC(matrox_vga_device::ramdac_mask_r), FUNC(matrox_vga_device::ramdac_mask_w));
	map(0x03, 0x03).lr8(
		NAME([this] (offs_t offset) { return vga.dac.read_index; })
	).w(FUNC(matrox_vga_device::ramdac_read_index_w));
//  map(0x04, 0x04) Cursor/Overscan Color Write Index
//  map(0x05, 0x05) Cursor/Overscan Color data
//  map(0x07, 0x07) Cursor/Overscan Color Read Index
//  map(0x09, 0x09) Direct Cursor control
	map(0x0a, 0x0a).rw(FUNC(matrox_vga_device::ramdac_ext_indexed_r), FUNC(matrox_vga_device::ramdac_ext_indexed_w));
//  map(0x0b, 0x0b) Cursor RAM data
//  map(0x0c, 0x0f) Cursor X/Y positions
//  map(0x10, 0x1f) <reserved>
}

u8 matrox_vga_device::ramdac_ext_indexed_r()
{
	// Unclear from the docs, according to usage seems to be the write index with no autoincrement
	logerror("RAMDAC ext read [%02x]\n", vga.dac.write_index);
	return space(EXT_REG + 1).read_byte(vga.dac.write_index);
;
}

void matrox_vga_device::ramdac_ext_indexed_w(offs_t offset, u8 data)
{
	logerror("RAMDAC ext [%02x] %02x\n", vga.dac.write_index, data);
	space(EXT_REG + 1).write_byte(vga.dac.write_index, data);
}

void matrox_vga_device::ramdac_indexed_map(address_map &map)
{
	map(0x18, 0x18).rw(FUNC(matrox_vga_device::truecolor_ctrl_r), FUNC(matrox_vga_device::truecolor_ctrl_w));
	map(0x19, 0x19).rw(FUNC(matrox_vga_device::multiplex_ctrl_r), FUNC(matrox_vga_device::multiplex_ctrl_w));
}

u8 matrox_vga_device::truecolor_ctrl_r()
{
	return m_truecolor_ctrl;
}

void matrox_vga_device::truecolor_ctrl_w(offs_t offset, u8 data)
{
	m_truecolor_ctrl = data;
	flush_true_color_mode();
}

u8 matrox_vga_device::multiplex_ctrl_r()
{
	return m_multiplex_ctrl;
}

void matrox_vga_device::multiplex_ctrl_w(offs_t offset, u8 data)
{
	m_multiplex_ctrl = data;
	flush_true_color_mode();
}

void matrox_vga_device::flush_true_color_mode()
{
	logerror("New video mode %02x %02x\n", m_truecolor_ctrl, m_multiplex_ctrl);
	svga.rgb8_en = svga.rgb15_en = svga.rgb16_en = svga.rgb24_en = svga.rgb32_en = 0;
	if (m_truecolor_ctrl == 0x80 && m_multiplex_ctrl == 0x98)
	{
		logerror("\tVGA mode\n");
		return;
	}

	switch(m_truecolor_ctrl)
	{
		case 0x80:
			if (m_multiplex_ctrl >= 0x49 && m_multiplex_ctrl <= 0x4c)
			{
				logerror("\tSVGA 8-bit mode\n");
				svga.rgb8_en = 1;
			}
			else
			{
				// [0x41, 0x42, 0x43, 0x44] for normal
				// [0x61, 0x62, 0x63, 0x64] for nibble swapped
				popmessage("TVP3026: Unemulated 4-bit %s with multiplex: %02x"
					, m_multiplex_ctrl & 0x20 ? "normal" : "nibble swapped"
					, m_multiplex_ctrl
				);
			}
			break;
		case 0x16:
		case 0x1e:
		case 0x56:
		case 0x5e:
			if (m_multiplex_ctrl == 0x5b || m_multiplex_ctrl == 0x5c)
			{
				logerror("\tRGB 8-8-8 mode\n");
				svga.rgb24_en = 1;
			}
			break;
		case 0x17:
		case 0x1f:
		case 0x57:
		case 0x5f:
			if (m_multiplex_ctrl == 0x5b || m_multiplex_ctrl == 0x5c)
				popmessage("TVP3026: Unemulated BGR 8-8-8 mode");
			break;
		case 0x06:
		case 0x46:
			if (m_multiplex_ctrl == 0x5b || m_multiplex_ctrl == 0x5c)
			{
				logerror("\t%cRGB 8-8-8-8 mode\n", m_truecolor_ctrl & 0x40 ? "X" : "O");
				svga.rgb32_en = 1;
			}
			break;
		case 0x07:
		case 0x47:
			if (m_multiplex_ctrl == 0x5b || m_multiplex_ctrl == 0x5c)
				popmessage("TVP3026: Unemulated BGR%c 8-8-8-8 mode", m_truecolor_ctrl & 0x40 ? "X" : "O");
			break;
		case 0x05:
		case 0x45:
			if (m_multiplex_ctrl >= 0x52 && m_multiplex_ctrl <= 0x54)
			{
				logerror("\tXGA RGB 5-6-5 mode\n");
				svga.rgb16_en = 1;
			}
			break;
		case 0x04:
		case 0x44:
			// 0x04 is selected by VESA 2.4 test, which may be buggy?
			// (extended mode 110h, while above uses 111h)
			if (m_multiplex_ctrl >= 0x52 && m_multiplex_ctrl <= 0x54)
			{
				logerror("\tTarga %cRGB 1-5-5-5 mode\n", m_truecolor_ctrl & 0x40 ? "X" : "O");
				svga.rgb16_en = 1;
			}
			break;
		case 0x03:
		case 0x43:
			if (m_multiplex_ctrl >= 0x52 && m_multiplex_ctrl <= 0x54)
				popmessage("TVP3026: Unemulated RGB 6-6-4 mode");
			break;
		case 0x01:
		case 0x41:
			if (m_multiplex_ctrl >= 0x52 && m_multiplex_ctrl <= 0x54)
				popmessage("TVP3026: Unemulated RGB%c 4-4-4-4 mode", m_truecolor_ctrl & 0x40 ? "X" : "O");
			break;
	}

	recompute_params();
}

uint8_t matrox_vga_device::mem_r(offs_t offset)
{
	if (m_mgamode)
		return svga_device::mem_linear_r(offset + (svga.bank_w * 0x10000));
	return svga_device::mem_r(offset);
}

void matrox_vga_device::mem_w(offs_t offset, uint8_t data)
{
	if (m_mgamode)
	{
		svga_device::mem_linear_w(offset + (svga.bank_w * 0x10000), data);
		return;
	}
	svga_device::mem_w(offset, data);
}

void matrox_vga_device::recompute_params()
{
	u8 xtal_select = (vga.miscellaneous_output & 0x0c) >> 2;
	int xtal;

	switch(xtal_select & 3)
	{
		case 0: xtal = XTAL(25'174'800).value(); break;
		case 1: xtal = XTAL(28'636'363).value(); break;
		// TODO: stub, derives from RAMDAC PLLs
		case 2:
		default:
			xtal = XTAL(50'000'000).value();
			break;
	}

	recompute_params_clock(1, xtal);
}

uint16_t matrox_vga_device::offset()
{
	// TODO: shifts depending on RAMDAC mode + CRTCEXT0 bits 5-4
	if (svga.rgb16_en)
		return (vga.crtc.offset << 4);
	return svga_device::offset();
}
