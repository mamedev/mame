// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#include "emu.h"
#include "pc_vga_matrox.h"

#define DEBUG_VRAM_VIEWER 0

DEFINE_DEVICE_TYPE(MATROX_VGA,  matrox_vga_device,  "matrox_vga",  "Matrox MGA2064W VGA i/f")

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

void matrox_vga_device::device_start()
{
	svga_device::device_start();

	save_item(NAME(m_cursor_read_index));
	save_item(NAME(m_cursor_write_index));
	save_item(NAME(m_cursor_index_state));
	save_item(NAME(m_cursor_ccr));
	save_item(NAME(m_cursor_dcc));
	save_pointer(NAME(m_cursor_color), 12);
	save_pointer(NAME(m_cursor_ram), 0x400);

	save_item(NAME(m_msc));
	save_item(NAME(m_truecolor_ctrl));
	save_item(NAME(m_multiplex_ctrl));

	save_item(NAME(m_pll_par));
	save_pointer(NAME(m_pll_data), 12);
}

void matrox_vga_device::device_reset()
{
	svga_device::device_reset();

	m_crtcext_index = 0;
	m_crtcext_misc = 0;
	m_crtcext_horz_counter = 0;
	m_crtcext_horz_half_count = 0;

	m_mgamode = false;
	m_interlace_mode = false;

	m_cursor_read_index = m_cursor_write_index = m_cursor_index_state = 0;
	m_cursor_ccr = 0;
	m_cursor_x = 0;
	m_cursor_y = 0;

	m_msc = 0;
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
	// CRTCEXT5 Horizontal Video Half Count
	map(0x05, 0x05).lrw8(
		NAME([this] (offs_t offset) { return m_crtcext_horz_half_count; }),
		NAME([this] (offs_t offset, u8 data) { m_crtcext_horz_half_count = data; })
	);
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
	map(0x04, 0x04).rw(FUNC(matrox_vga_device::cursor_write_index_r), FUNC(matrox_vga_device::cursor_write_index_w));
	map(0x05, 0x05).rw(FUNC(matrox_vga_device::cursor_data_r), FUNC(matrox_vga_device::cursor_data_w));
	map(0x07, 0x07).rw(FUNC(matrox_vga_device::cursor_read_index_r), FUNC(matrox_vga_device::cursor_read_index_w));
	// DDC Direct Cursor control
	map(0x09, 0x09).lrw8(
		NAME([this] (offs_t offset) {
			return m_cursor_dcc & 3;
		}),
		NAME([this] (offs_t offset, u8 data) {
			// compatible alias to setup cursor mode
			m_cursor_dcc = (data & 3);
		})
	);
	map(0x0a, 0x0a).rw(FUNC(matrox_vga_device::ramdac_ext_indexed_r), FUNC(matrox_vga_device::ramdac_ext_indexed_w));
//  map(0x0b, 0x0b) Cursor RAM data
	map(0x0b, 0x0b).lrw8(
		NAME([this] (offs_t offset) {
			u16 cursor_address = vga.dac.read_index | ((m_cursor_ccr & 0xc) >> 2) << 8;
			u8 res = m_cursor_ram[cursor_address++];
			cursor_address &= 0x3ff;
			vga.dac.read_index = cursor_address & 0xff;
			m_cursor_ccr = (((cursor_address & 0x300) >> 8) << 2) | (m_cursor_ccr & 0xf3);
			return res;
		}),
		NAME([this] (offs_t offset, u8 data) {
			u16 cursor_address = vga.dac.write_index | ((m_cursor_ccr & 0xc) >> 2) << 8;
			m_cursor_ram[cursor_address++] = data;
			cursor_address &= 0x3ff;
			vga.dac.write_index = cursor_address & 0xff;
			m_cursor_ccr = (((cursor_address & 0x300) >> 8) << 2) | (m_cursor_ccr & 0xf3);
		})
	);
	map(0x0c, 0x0d).lrw8(
		NAME([this] (offs_t offset) {
			return m_cursor_x >> (offset * 8);
		}),
		NAME([this] (offs_t offset, u8 data) {
			const u8 shift_mask = 0xff00 >> (offset * 8);
			m_cursor_x = (data << (offset * 8)) | (m_cursor_x & shift_mask);
			m_cursor_x &= 0xfff;
		})
	);
	map(0x0e, 0x0f).lrw8(
		NAME([this] (offs_t offset) {
			return m_cursor_y >> (offset * 8);
		}),
		NAME([this] (offs_t offset, u8 data) {
			const u8 shift_mask = 0xff00 >> (offset * 8);
			m_cursor_y = (data << (offset * 8)) | (m_cursor_y & shift_mask);
			m_cursor_y &= 0xfff;
		})
	);
//  map(0x10, 0x1f) <reserved>
}

u8 matrox_vga_device::ramdac_ext_indexed_r()
{
	// Unclear from the docs, according to usage seems to be the write index with no autoincrement
//  logerror("RAMDAC ext read [%02x]\n", vga.dac.write_index);
	return space(EXT_REG + 1).read_byte(vga.dac.write_index);
}

void matrox_vga_device::ramdac_ext_indexed_w(offs_t offset, u8 data)
{
//  logerror("RAMDAC ext [%02x] %02x\n", vga.dac.write_index, data);
	space(EXT_REG + 1).write_byte(vga.dac.write_index, data);
}

u8 matrox_vga_device::cursor_write_index_r()
{
	return m_cursor_write_index;
}

void matrox_vga_device::cursor_write_index_w(offs_t offset, u8 data)
{
	m_cursor_write_index = data & 3;
	m_cursor_index_state = 0;
	if (data & 0xfc)
		logerror("RAMDAC cursor_write_index_w > 3 -> %02x\n", data);
}

u8 matrox_vga_device::cursor_read_index_r()
{
	return m_cursor_read_index;
}

void matrox_vga_device::cursor_read_index_w(offs_t offset, u8 data)
{
	m_cursor_read_index = data & 3;
	m_cursor_index_state = 0;
	if (data & 0xfc)
		logerror("RAMDAC cursor_read_index_w > 3 -> %02x\n", data);
}

u8 matrox_vga_device::cursor_data_r()
{
	const u8 res = m_cursor_color[(m_cursor_read_index * 3) + m_cursor_index_state];
	if (!machine().side_effects_disabled())
	{
		m_cursor_index_state ++;
		if (m_cursor_index_state > 2)
		{
			m_cursor_index_state = 0;
			m_cursor_read_index ++;
			m_cursor_read_index &= 3;
		}
	}
	return res;
}

void matrox_vga_device::cursor_data_w(offs_t offset, u8 data)
{
	m_cursor_color[(m_cursor_write_index * 3) + m_cursor_index_state] = data;
	if (!machine().side_effects_disabled())
	{
		m_cursor_index_state ++;
		if (m_cursor_index_state > 2)
		{
			m_cursor_index_state = 0;
			m_cursor_write_index ++;
			m_cursor_write_index &= 3;
		}
	}
}

// map(0x2d, 0x2d) PPD pixel clock PLL
// map(0x2e, 0x2e) MPD memory clock PLL
// map(0x2f, 0x2f) LPD loop clock PLL
u8 matrox_vga_device::pll_data_r(offs_t offset)
{
	assert(offset < 3);
	const std::string source_pll[] = { "Pixel", "MCLK", "Loop" };
	const std::string value_pll[] = { "N-value", "M-value", "P-value", "Status" };

	const u8 par = (m_pll_par >> (offset * 2)) & 3;

	logerror("PLL %s %s R\n", source_pll[offset], value_pll[par]);
	u8 res = m_pll_data[(offset << 2) | par];
	// each of these registers wants specific signatures, beos 4 cares
	switch(par)
	{
		case 0: res |= 0xc0; break;
		case 1: res &= 0x3f; break;
		case 2:
			switch(offset)
			{
				case 0:
					res |= 0x30;
					// TODO: why beos 4 also expects bit 6 to be on specifically for Pixel clock?
					// TVP documentation claims to be PCLKEN
					res |= 0x40;
					break;
				case 1:
					res &= 0x83;
					res |= 0x30;
					break;
				case 2:
					res &= 0x8b;
					res |= 0x70;
					break;
			}

			break;
		case 3:
			// HACK: always lock for now
			res = 0x40;
			break;
	}

	return res;
}

void matrox_vga_device::pll_data_w(offs_t offset, u8 data)
{
	assert(offset < 3);
	const std::string source_pll[] = { "Pixel", "MCLK", "Loop" };
	const std::string value_pll[] = { "N-value", "M-value", "P-value", "<Status?>" };

	const u8 par = (m_pll_par >> (offset * 2)) & 3;

	logerror("PLL %s %s %02x W\n", source_pll[offset], value_pll[par], data);
	// status is read-only
	if (par == 3)
		return;
	m_pll_data[(offset << 2) | par] = data;
}

void matrox_vga_device::ramdac_indexed_map(address_map &map)
{
	// silicon revision
	map(0x01, 0x01).lr8(
		NAME([] (offs_t offset) { return 0x00; })
	);
	// CCR indirect cursor control
	map(0x06, 0x06).lrw8(
		NAME([this] (offs_t offset) {
			return m_cursor_ccr;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_cursor_ccr = data;
		})
	);
//  map(0x0f, 0x0f) LCR latch control

	map(0x18, 0x18).rw(FUNC(matrox_vga_device::truecolor_ctrl_r), FUNC(matrox_vga_device::truecolor_ctrl_w));
	map(0x19, 0x19).rw(FUNC(matrox_vga_device::multiplex_ctrl_r), FUNC(matrox_vga_device::multiplex_ctrl_w));
//  map(0x1a, 0x1a) CSR clock selection
//  map(0x1c, 0x1c) palette page
//  map(0x1d, 0x1d) GCR general control
	// MSC misc control
	map(0x1e, 0x1e).lrw8(
		NAME([this] (offs_t offset) {
			logerror("$1e MSC R\n");
			return m_msc;
		}),
		NAME([this] (offs_t offset, u8 data) {
			logerror("$1e MSC W %02x\n", data);
			if ((m_msc & 0xc) != (data & 0xc))
				vga.dac.dirty = 1;
			m_msc = data;
		})
	);
//  map(0x2a, 0x2a) IOC GPIO control (bits 4-0, 1 = data bit as output, 0 = data bit as input)
//  map(0x2b, 0x2b) GPIO data (bits 4-0)
	// PLL Address Register
	// bits 5-4 Loop clock PLL, 3-2 MCLK PLL, 1-0 Pixel clock PLL
	map(0x2c, 0x2c).lrw8(
		NAME([this] (offs_t offset) {
			logerror("$2c PLL PAR R\n");
			return m_pll_par;
		}),
		NAME([this] (offs_t offset, u8 data) {
			logerror("$2c PLL PAR W %02x\n", data);
			m_pll_par = data;
		})
	);
	map(0x2d, 0x2f).rw(FUNC(matrox_vga_device::pll_data_r), FUNC(matrox_vga_device::pll_data_w));
//  map(0x30, 0x31) color key overlay
//  map(0x32, 0x37) color key r/g/b
//  map(0x38, 0x38) CKC color key control
//  map(0x39, 0x39) MKC MCLK & loop clock control
//  map(0x3a, 0x3a) sense test
//  map(0x3b, 0x3b) Test mode data (r/o)
//  map(0x3c, 0x3d) CRC signal test (r/o)
//  map(0x3e, 0x3e) BSR CRC bit select
	// chip ID
	map(0x3f, 0x3f).lr8(
		NAME([] (offs_t offset) { return 0x26; })
	);

//  map(0xff, 0xff) software reset (w/o)
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
				// Used a lot on intermediate states between VGA and Power Modes ...
				logerror("TVP3026: Unemulated 4-bit %s with multiplex: %02x\n"
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
			// 0x04 is selected by VESA 2.4 test for rgb16 mode,
			// assume program misuse of the mode given that SDD vbetest really expects rgb15 here.
			// (extended mode 110h, while above uses 111h)
			if (m_multiplex_ctrl >= 0x52 && m_multiplex_ctrl <= 0x54)
			{
				logerror("\tTarga %cRGB 1-5-5-5 mode\n", m_truecolor_ctrl & 0x40 ? "X" : "O");
				svga.rgb15_en = 1;
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

void matrox_vga_device::palette_update()
{
	// TODO: terminal pin handling
	// (which does the same thing but externally controlled)
	if ((m_msc & 0xc) != 0xc)
		vga_device::palette_update();
	else
	{
		for (int i = 0; i < 256; i++)
		{
			set_pen_color(
				i,
				vga.dac.color[3*(i & vga.dac.mask) + 0],
				vga.dac.color[3*(i & vga.dac.mask) + 1],
				vga.dac.color[3*(i & vga.dac.mask) + 2]
			);
		}
	}

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
		// TODO: stub, derives from RAMDAC PLLs (MGA2064W has usual rates of 220 MHz)
		case 2:
		default:
			xtal = XTAL(50'000'000).value();
			break;
	}

	recompute_params_clock(1, xtal);
}

uint16_t matrox_vga_device::offset()
{
	// SuperVGA modes expects a fixed offset multiplier of x16
	// TODO: is multiplex ratio taken into account here?
	if (m_mgamode)
		return (vga.crtc.offset << 4);

	return svga_device::offset();
}

uint32_t matrox_vga_device::latch_start_addr()
{
	// TODO: fails SDD scrolling tests
	// Looks like it can latch per byte in SVGA modes, which contradicts what's in pc_vga
	// drawing functions.
	//if (m_mgamode)
	//	return (vga.crtc.start_addr << 4);

	return vga.crtc.start_addr_latch;
}

u16 matrox_vga_device::line_compare_mask()
{
	return m_mgamode ? 0x7ff : 0x3ff;
}

uint32_t matrox_vga_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	svga_device::screen_update(screen, bitmap, cliprect);
	// HW cursor
	const u8 cursor_mode = (BIT(m_cursor_ccr, 7) ? m_cursor_dcc : m_cursor_ccr) & 3;

	if (cursor_mode)
	{
		// partial XGA mode only for now (Win 3.1), others TBD
		// mode 1: 3 color mode -> transparent/color 0-1-2
		// mode 2: xga mode -> color 0-1/transparent/complement
		// mode 3: x-window mode -> transparent/transparent/color 0-1
		const u8 transparent_pen = 2;
		for (int y = 0; y < 64; y ++)
		{
			int res_y = y + m_cursor_y - 64;
			for (int x = 0; x < 64; x++)
			{
				int res_x = x + m_cursor_x - 64;
				if (!cliprect.contains(res_x, res_y))
					continue;
				const u16 cursor_address = (x >> 3) + y * 8;
				const int xi = 7 - (x & 7);
				u8 cursor_gfx = (m_cursor_ram[cursor_address] >> (xi) & 1) | ((m_cursor_ram[cursor_address + 0x200] >> (xi)) & 1) << 1;
				if (cursor_gfx == transparent_pen)
					continue;
				// FIXME: Win 3.1 writes to clut 2 for white, may be wrong
				cursor_gfx ++;
				cursor_gfx &= 3;
				const u8 r = m_cursor_color[3 * cursor_gfx + 0];
				const u8 g = m_cursor_color[3 * cursor_gfx + 1];
				const u8 b = m_cursor_color[3 * cursor_gfx + 2];

				bitmap.pix(res_y, res_x) = r << 16 | g << 8 | b;
			}
		}
	}

#if DEBUG_VRAM_VIEWER
	static int m_test_x = 640, m_start_offs;
	static int m_test_trigger = 1;
	const int m_test_y = cliprect.max_y;

	if(machine().input().code_pressed(JOYCODE_X_RIGHT_SWITCH))
		m_test_x += 1 << (machine().input().code_pressed(JOYCODE_BUTTON2) ? 4 : 0);

	if(machine().input().code_pressed(JOYCODE_X_LEFT_SWITCH))
		m_test_x -= 1 << (machine().input().code_pressed(JOYCODE_BUTTON2) ? 4 : 0);

	//if(machine().input().code_pressed(JOYCODE_Y_DOWN_SWITCH))
	//  m_test_y++;

	//if(machine().input().code_pressed(JOYCODE_Y_UP_SWITCH))
	//  m_test_y--;

	if(machine().input().code_pressed(JOYCODE_Y_DOWN_SWITCH))
		m_start_offs+= 0x100 << (machine().input().code_pressed(JOYCODE_BUTTON2) ? 8 : 0);

	if(machine().input().code_pressed(JOYCODE_Y_UP_SWITCH))
		m_start_offs-= 0x100 << (machine().input().code_pressed(JOYCODE_BUTTON2) ? 8 : 0);

	m_start_offs %= vga.svga_intf.vram_size;

	if(machine().input().code_pressed_once(JOYCODE_BUTTON1))
		m_test_trigger ^= 1;

	if (!m_test_trigger)
		return 0;

	popmessage("%d %d %04x", m_test_x, m_test_y, m_start_offs);

	bitmap.fill(0, cliprect);

	int count = m_start_offs;

	for(int y = 0; y < m_test_y; y++)
	{
		for(int x = 0; x < m_test_x; x ++)
		{
			u8 color = vga.memory[count % vga.svga_intf.vram_size];

			if(cliprect.contains(x, y))
			{
				//bitmap.pix(y, x) = pal565(color, 11, 5, 0);
				bitmap.pix(y, x) = pen(color);
			}

			count ++;
			// count += 2;
		}
	}
#endif
	return 0;
}
