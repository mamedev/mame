// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * s3virge.cpp
 *
 * Implementation of the S3 Virge series of video card
 *
 * TODO:
 * - Proper FIFOs;
 * - Implement 3d commands;
 * - Implement remaining ROP commands;
 * - Secondary stream mixing;
 * - S3 Scenic Highway i/f (SAA7110 + S3 Scenic/MX2 MPEG-1);
 * - DMAs;
 * - interrupts;
 * - big endian support for non-x86 machines;
 * - DDC/I2C i/f, cfr. serial port on MMFF20;
 * - Fix PLL calculation for 1k+ width VESA modes (tends to either be too fast or too slow);
 * - Fix interlace mode line compare downstream (1600x1200 res);
 * - xubuntu: black screen after booting into GNOME,
 *            tries to setup linear address with new MMIO disabled,
 *            kernel driver has DDC checks around that ...
 * - win98se: doesn't show transparent layer on shut down screen;
 *
 */

#include "emu.h"
#include "s3virge.h"

#include "screen.h"

//#include <iostream>

#define VERBOSE (LOG_REG | LOG_CMD | LOG_MMIO)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOG_REG  (1U << 1)
#define LOG_CMD  (1U << 2)
#define LOG_MMIO (1U << 3)

#define LOGREG(...)  LOGMASKED(LOG_REG, __VA_ARGS__)
#define LOGCMD(...)  LOGMASKED(LOG_CMD, __VA_ARGS__)
#define LOGMMIO(...) LOGMASKED(LOG_MMIO, __VA_ARGS__)


#define CRTC_PORT_ADDR ((vga.miscellaneous_output & 1) ? 0x3d0 : 0x3b0)

DEFINE_DEVICE_TYPE(S3VIRGE,    s3virge_vga_device,        "virge_vga",      "S3 86C325 VGA core")
DEFINE_DEVICE_TYPE(S3VIRGEDX,  s3virgedx_vga_device,      "virgedx_vga",    "S3 86C375 VGA core")
DEFINE_DEVICE_TYPE(S3VIRGEDX1, s3virgedx_rev1_vga_device, "virgedx_vga_r1", "S3 86C375 (rev 1) VGA core")

s3virge_vga_device::s3virge_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: s3virge_vga_device(mconfig, S3VIRGE, tag, owner, clock)
{
	m_crtc_space_config = address_space_config("crtc_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(s3virge_vga_device::crtc_map), this));
	m_seq_space_config = address_space_config("sequencer_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(s3virge_vga_device::sequencer_map), this));
}

s3virge_vga_device::s3virge_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: s3_vga_device(mconfig, type, tag, owner, clock)
	, m_linear_config_changed_cb(*this)
{
}

s3virgedx_vga_device::s3virgedx_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: s3virgedx_vga_device(mconfig, S3VIRGEDX, tag, owner, clock)
{
	m_crtc_space_config = address_space_config("crtc_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(s3virgedx_vga_device::crtc_map), this));
	m_seq_space_config = address_space_config("sequencer_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(s3virgedx_vga_device::sequencer_map), this));
}

s3virgedx_vga_device::s3virgedx_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: s3virge_vga_device(mconfig, type, tag, owner, clock)
{
}

s3virgedx_rev1_vga_device::s3virgedx_rev1_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: s3virgedx_vga_device(mconfig, S3VIRGEDX1, tag, owner, clock)
{
	m_crtc_space_config = address_space_config("crtc_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(s3virgedx_rev1_vga_device::crtc_map), this));
	m_seq_space_config = address_space_config("sequencer_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(s3virgedx_rev1_vga_device::sequencer_map), this));
}

void s3virge_vga_device::device_start()
{
	zero();

	for (int i = 0; i < 0x100; i++)
		set_pen_color(i, 0, 0, 0);

	// Avoid an infinite loop when displaying.  0 is not possible anyway.
	vga.crtc.maximum_scan_line = 1;

	// copy over interfaces
	vga.memory = std::make_unique<uint8_t []>(vga.svga_intf.vram_size);
	memset(&vga.memory[0], 0, vga.svga_intf.vram_size);

	save_pointer(vga.memory, "Video RAM", vga.svga_intf.vram_size);
	save_item(vga.crtc.data,"CRTC Registers");
	save_item(vga.sequencer.data,"Sequencer Registers");
	save_item(vga.attribute.data,"Attribute Registers");

	m_vblank_timer = timer_alloc(FUNC(vga_device::vblank_timer_cb), this);
	m_draw_timer = timer_alloc(FUNC(s3virge_vga_device::draw_step_tick), this);

	memset(&s3, 0, sizeof(s3));
	memset(&s3virge, 0, sizeof(s3virge));
	s3virge.linear_address = 0x70000000;
	s3virge.linear_address_size_full = 0x10000;
	s3virge.s3d.cmd_fifo_slots_free = 16;
	save_item(s3virge.s3d.pattern,"S3D Pattern Data");
	save_item(s3virge.s3d.reg[0],"S3D Registers: BitBLT");
	save_item(s3virge.s3d.reg[1],"S3D Registers: 2D Line");
	save_item(s3virge.s3d.reg[2],"S3D Registers: 2D Polygon");
	save_item(s3virge.s3d.reg[3],"S3D Registers: 3D Line");
	save_item(s3virge.s3d.reg[4],"S3D Registers: 3D Triangle");

	// Initialise hardware graphics cursor colours, Windows 95 doesn't touch the registers for some reason
	for (int x = 0; x < 4; x++)
	{
		s3.cursor_fg[x] = 0xff;
		s3.cursor_bg[x] = 0x00;
	}
	// set device ID
	s3.id_high = 0x56;  // CR2D
	s3.id_low = 0x31;   // CR2E
	s3.revision = 0x00; // CR2F  (value unknown)
	s3.id_cr30 = 0xe1;  // CR30
}

void s3virgedx_vga_device::device_start()
{
	s3virge_vga_device::device_start();

	// set device ID
	s3.id_high = 0x8a;  // CR2D
	s3.id_low = 0x01;   // CR2E
	s3.revision = 0x00; // CR2F  (value unknown)
	s3.id_cr30 = 0xe1;  // CR30
}

void s3virgedx_rev1_vga_device::device_start()
{
	s3virge_vga_device::device_start();

	// set device ID
	s3.id_high = 0x8a;  // CR2D
	s3.id_low = 0x01;   // CR2E
	s3.revision = 0x01; // CR2F
	s3.id_cr30 = 0xe1;  // CR30
}

void s3virge_vga_device::device_reset()
{
	s3_vga_device::device_reset();
	// Power-on strapping bits.  Sampled at reset, but can be modified later.
	// These are just assumed defaults.
	s3.strapping = 0x000f0912;

	// TODO: fix soft reset state
	// On Windows 98 shutdown message sometimes leads to an hang the next boot around
	s3virge.s3d.state = S3D_STATE_IDLE;
	s3virge.s3d.cmd_fifo_current_ptr = 0;
	s3virge.s3d.cmd_fifo_slots_free = 16;
	s3virge.s3d.busy = false;
	//m_draw_timer->adjust(attotime::never);
}

void s3virgedx_vga_device::device_reset()
{
	s3virge_vga_device::device_reset();
	// Power-on strapping bits.  Sampled at reset, but can be modified later.
	// These are just assumed defaults.
	s3.strapping = 0x000f0912;
}

void s3virgedx_rev1_vga_device::device_reset()
{
	s3virgedx_vga_device::device_reset();
	// Power-on strapping bits.  Sampled at reset, but can be modified later.
	// These are based on results from a Diamond Stealth 3D 2000 Pro (Virge/DX based)
	// bits 8-15 are still unknown, S3ID doesn't show config register 2 (CR37)
	s3.strapping = 0x0aff0912;
}

uint16_t s3virge_vga_device::offset()
{
	// win98se expects 24bpp packed mode with x6 boundaries
	// this breaks VBETest, which detects these VESA modes as 32bpp.
	if(svga.rgb24_en)
		return vga.crtc.offset * 6;
	return s3_vga_device::offset();
}

void s3virge_vga_device::crtc_map(address_map &map)
{
	s3_vga_device::crtc_map(map);
	// TODO: verify these overrides
	map(0x3a, 0x3a).lw8(
		NAME([this] (offs_t offset, u8 data) {
			s3.cr3a = data;
			s3_define_video_mode();
		})
	);
	map(0x40, 0x40).lw8(
		NAME([this] (offs_t offset, u8 data) {
			// enable S3D registers
			s3.enable_s3d = data & 0x01;
		})
	);
	map(0x45, 0x45).lrw8(
		NAME([this] (offs_t offset) {
			return s3.cursor_mode;
		}),
		NAME([this] (offs_t offset, u8 data) {
			s3.cursor_mode = data;
		})
	);
	map(0x4a, 0x4a).lr8(
		NAME([this] (offs_t offset) {
			u8 res = s3.cursor_fg[s3.cursor_fg_ptr];
			s3.cursor_fg_ptr = 0;
			return res;
		})
	);
	map(0x4b, 0x4b).lr8(
		NAME([this] (offs_t offset) {
			u8 res = s3.cursor_bg[s3.cursor_bg_ptr];
			s3.cursor_bg_ptr = 0;
			return res;
		})
	);
	map(0x53, 0x53).lrw8(
		NAME([this] (offs_t offset) {
			return s3.cr53;
		}),
		NAME([this] (offs_t offset, u8 data) {
			s3.cr53 = data;
			LOGREG("CR53: write %02x\n", data);
			// FIXME: this is just to make PCI to catch up for the side effect of relocating MMIO.
			// TODO: Big Endian at bits 2-1
			m_linear_config_changed_cb(s3virge.linear_address_enable);
		})
	);
	map(0x58, 0x58).lrw8(
		NAME([this] (offs_t offset) {
			u8 res = s3virge.linear_address_size & 0x03;
			res   |= s3virge.linear_address_enable ? 0x10 : 0x00;
			return res;
		}),
		NAME([this] (offs_t offset, u8 data) {
			const uint8_t old_size = s3virge.linear_address_size;
			const bool old_enable = s3virge.linear_address_enable;
			const bool size_changed = old_size != s3virge.linear_address_size;

			s3virge.linear_address_size = data & 0x03;
			s3virge.linear_address_enable = data & 0x10;

			switch(data & 0x03)
			{
				case LAW_64K:
					s3virge.linear_address_size_full = 0x10000;
					break;
				case LAW_1MB:
					s3virge.linear_address_size_full = 0x100000;
					break;
				case LAW_2MB:
					s3virge.linear_address_size_full = 0x200000;
					break;
				case LAW_4MB:
					s3virge.linear_address_size_full = 0x400000;
					break;
			}

			if ((s3virge.linear_address_enable != old_enable) || size_changed)
			{
				m_linear_config_changed_cb(s3virge.linear_address_enable);
			}

			LOGREG("CR58: write %02x\n", data);
		})
	);
	map(0x59, 0x59).lrw8(
		NAME([this] (offs_t offset) {
			return (s3virge.linear_address & 0xff000000) >> 24;
		}),
		NAME([this] (offs_t offset, u8 data) {
			const uint32_t old_address = s3virge.linear_address;
			s3virge.linear_address = (s3virge.linear_address & 0x00ff0000) | (data << 24);
			LOGREG("Linear framebuffer address = %08x\n",s3virge.linear_address);

			if (old_address != s3virge.linear_address && s3virge.linear_address_enable)
			{
				m_linear_config_changed_cb(1);
			}
		})
	);
	map(0x5a, 0x5a).lrw8(
		NAME([this] (offs_t offset) {
			u8 res = 0;
			switch(s3virge.linear_address_size & 0x03)
			{
				case 0:  // 64kB
				default:
					res = (s3virge.linear_address & 0x00ff0000) >> 16;
					break;
				case 1:  // 1MB
					res = (s3virge.linear_address & 0x00f00000) >> 16;
					break;
				case 2:  // 2MB
					res = (s3virge.linear_address & 0x00e00000) >> 16;
					break;
				case 3:  // 4MB
					res = (s3virge.linear_address & 0x00c00000) >> 16;
					break;
			}
			return res;
		}),
		NAME([this] (offs_t offset, u8 data) {
			const uint32_t old_address = s3virge.linear_address;
			s3virge.linear_address = (s3virge.linear_address & 0xff000000) | (data << 16);
			LOGREG("Linear framebuffer address = %08x\n",s3virge.linear_address);

			if (old_address != s3virge.linear_address && s3virge.linear_address_enable)
			{
				m_linear_config_changed_cb(1);
			}
		})
	);
	//map(0x5d, 0x5e).unmapr();
}

void s3virge_vga_device::s3_define_video_mode()
{
	int divisor = 1;
	const XTAL base_xtal = XTAL(14'318'181);
	XTAL xtal = (vga.miscellaneous_output & 0xc) ? base_xtal*2 : base_xtal*1.75;

	if((vga.miscellaneous_output & 0xc) == 0x0c)
	{
		// Dot clock is set via SR12 and SR13
		// DCLK calculation
		double ratio = (double)(s3.clk_pll_m+2) / (double)((s3.clk_pll_n+2)*(pow(2.0,s3.clk_pll_r))); // clock between XIN and XOUT
		xtal = base_xtal * ratio;
		//printf("DCLK set to %dHz M=%i N=%i R=%i\n",xtal,s3.clk_pll_m,s3.clk_pll_n,s3.clk_pll_r);
	}

	if((s3.ext_misc_ctrl_2) >> 4)
	{
		svga.rgb8_en = 0;
		svga.rgb15_en = 0;
		svga.rgb16_en = 0;
		svga.rgb24_en = 0;
		switch((s3.ext_misc_ctrl_2) >> 4)
		{
			case 0x01: svga.rgb8_en = 1; break;
			case 0x03: svga.rgb15_en = 1; divisor = 2; break;
			case 0x05: svga.rgb16_en = 1; divisor = 2; break;
			case 0x0d: svga.rgb24_en = 1; divisor = 1; break;
			default: fatalerror("TODO: s3 video mode not implemented %02x\n",((s3.ext_misc_ctrl_2) >> 4));
		}
	}
	else
	{
		svga.rgb8_en = (s3.cr3a & 0x10) >> 4;
		svga.rgb15_en = 0;
		svga.rgb16_en = 0;
		svga.rgb24_en = 0;
	}
	if(s3.cr43 & 0x80)  // Horizontal clock doubling (technically, doubles horizontal CRT parameters)
		divisor *= 2;
	recompute_params_clock(divisor, xtal.value());
}

uint8_t s3virge_vga_device::mem_r(offs_t offset)
{
	if (svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en || svga.rgb32_en)
	{
		uint8_t data;
		if(offset & 0x10000)
			return 0;
		data = 0xff;
		if(vga.sequencer.data[4] & 0x8)
		{
			if(offset + (svga.bank_r*0x10000) < vga.svga_intf.vram_size)
				data = vga.memory[offset + (svga.bank_r*0x10000)];
		}
		else
		{
			int i;

			for(i=0;i<4;i++)
			{
				if(vga.sequencer.map_mask & 1 << i)
				{
					if(offset*4+i+(svga.bank_r*0x10000) < vga.svga_intf.vram_size)
						data |= vga.memory[offset*4+i+(svga.bank_r*0x10000)];
				}
			}
		}
		return data;
	}
	if((offset + (svga.bank_r*0x10000)) < vga.svga_intf.vram_size)
		return vga_device::mem_r(offset);
	else
		return 0xff;
}

void s3virge_vga_device::mem_w(offs_t offset, uint8_t data)
{
	// bit 4 of CR53 enables memory-mapped I/O
	if(s3.cr53 & 0x10)
	{
		// TODO
	}

	if (svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en || svga.rgb32_en)
	{
	//  printf("%08x %02x (%02x %02x) %02X\n",offset,data,vga.sequencer.map_mask,svga.bank_w,(vga.sequencer.data[4] & 0x08));
		if(offset & 0x10000)
			return;
		if(vga.sequencer.data[4] & 0x8)
		{
			if((offset + (svga.bank_w*0x10000)) < vga.svga_intf.vram_size)
				vga.memory[(offset + (svga.bank_w*0x10000))] = data;
		}
		else
		{
			int i;
			for(i=0;i<4;i++)
			{
				if(vga.sequencer.map_mask & 1 << i)
				{
					if((offset*4+i+(svga.bank_w*0x10000)) < vga.svga_intf.vram_size)
						vga.memory[(offset*4+i+(svga.bank_w*0x10000))] = data;
				}
			}
		}
		return;
	}

	if((offset + (svga.bank_w*0x10000)) < vga.svga_intf.vram_size)
		vga_device::mem_w(offset,data);
}

uint8_t s3virge_vga_device::fb_r(offs_t offset)
{
	if(offset < s3virge.linear_address_size_full)
		return vga.memory[offset % vga.svga_intf.vram_size];
	return 0xff;
}

void s3virge_vga_device::fb_w(offs_t offset, uint8_t data)
{
	if(offset < s3virge.linear_address_size_full)
		vga.memory[offset % vga.svga_intf.vram_size] = data;
}

void s3virge_vga_device::add_command(int cmd_type)
{
	// add command to S3D FIFO
	if(s3virge.s3d.cmd_fifo_slots_free == 0)
	{
		LOGCMD("Attempt to add command when all command slots are full\n");
		return;
	}
	memcpy(s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_next_ptr].reg,s3virge.s3d.reg[cmd_type],256*4);
	s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_next_ptr].op_type = cmd_type;
	LOGCMD("Added command type %i cmd %08x ptr %u\n",s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_next_ptr].op_type,s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_next_ptr].reg[S3D_REG_COMMAND],s3virge.s3d.cmd_fifo_next_ptr);
	s3virge.s3d.cmd_fifo_next_ptr++;
	if(s3virge.s3d.cmd_fifo_next_ptr >= 16)
		s3virge.s3d.cmd_fifo_next_ptr = 0;
	if(s3virge.s3d.cmd_fifo_slots_free == 16)  // if all slots are free, start command now
		command_start();
	s3virge.s3d.cmd_fifo_slots_free--;
	// TODO: handle full FIFO
}

void s3virge_vga_device::command_start()
{
	// start next command in FIFO
	int cmd_type = s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].op_type;

	switch(cmd_type)
	{
		case OP_2DLINE:
			LOGCMD("2D Line command (unsupported) [%u]\n", s3virge.s3d.cmd_fifo_current_ptr);
			break;
		case OP_2DPOLY:
			LOGCMD("2D Poly command (unsupported) [%u]\n", s3virge.s3d.cmd_fifo_current_ptr);
			break;
		case OP_3DLINE:
			LOGCMD("3D Line command (unsupported) [%u]\n", s3virge.s3d.cmd_fifo_current_ptr);
			break;
		case OP_3DTRI:
			LOGCMD("3D Tri command (unsupported) [%u]\n", s3virge.s3d.cmd_fifo_current_ptr);
			break;
		case OP_BITBLT:
			s3virge.s3d.state = S3D_STATE_BITBLT;
			s3virge.s3d.busy = true;
			s3virge.s3d.bitblt_x_src = (s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_RSRC_XY] & 0x07ff0000) >> 16;
			s3virge.s3d.bitblt_y_src = (s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_RSRC_XY] & 0x000007ff);
			s3virge.s3d.bitblt_x_dst = (s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_RDEST_XY] & 0x07ff0000) >> 16;
			s3virge.s3d.bitblt_y_dst = (s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_RDEST_XY] & 0x000007ff);
			s3virge.s3d.bitblt_width = (s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_RWIDTH_HEIGHT] & 0xffff0000) >> 16;
			s3virge.s3d.bitblt_height = (s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_RWIDTH_HEIGHT] & 0x0000ffff);
			// TODO: these four goes negative at second transfer of beos 4
			s3virge.s3d.bitblt_x_current = s3virge.s3d.bitblt_x_dst;
			s3virge.s3d.bitblt_x_src_current = s3virge.s3d.bitblt_x_src;
			s3virge.s3d.bitblt_y_current = s3virge.s3d.bitblt_y_dst;
			s3virge.s3d.bitblt_y_src_current = s3virge.s3d.bitblt_y_src;
			s3virge.s3d.bitblt_pat_x = s3virge.s3d.bitblt_x_current % 8;
			s3virge.s3d.bitblt_pat_y = s3virge.s3d.bitblt_y_current % 8;
			s3virge.s3d.clip_r = s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_CLIP_L_R] & 0x000007ff;
			s3virge.s3d.clip_l = (s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_CLIP_L_R] & 0x07ff0000) >> 16;
			s3virge.s3d.clip_b = s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_CLIP_T_B] & 0x000007ff;
			s3virge.s3d.clip_t = (s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_CLIP_T_B] & 0x07ff0000) >> 16;
			if(!(s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_COMMAND] & 0x00000080))
				m_draw_timer->adjust(attotime::from_nsec(250),0,attotime::from_nsec(250));
			s3virge.s3d.bitblt_step_count = 0;
			s3virge.s3d.bitblt_mono_pattern =
					s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_MONO_PAT_0] | (uint64_t)(s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_MONO_PAT_1]) << 32;
			s3virge.s3d.bitblt_current_pixel = 0;
			s3virge.s3d.bitblt_pixel_pos = 0;
			// FIXME: win31 & beos 4.x never sets this.
			// Latter definitely relies on what's set in PAT_FG_CLR for background pen being set (???)
			s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_PAT_BG_CLR] = s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_PAT_FG_CLR];
			// s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_PAT_BG_CLR] = 0xffffffff;
			LOGCMD("Started BitBLT command [%u]\n", s3virge.s3d.cmd_fifo_current_ptr);
			//if(((s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_COMMAND] & 0x01fe0000) >> 17) == 0xf0) machine().debug_break();
			break;
	}
}

void s3virge_vga_device::command_finish()
{
	s3virge.s3d.state = S3D_STATE_IDLE;
	s3virge.s3d.cmd_fifo_current_ptr++;
	if(s3virge.s3d.cmd_fifo_current_ptr >= 16)
		s3virge.s3d.cmd_fifo_current_ptr = 0;
	s3virge.s3d.cmd_fifo_slots_free++;
	if(s3virge.s3d.cmd_fifo_slots_free > 16)
		s3virge.s3d.cmd_fifo_slots_free = 16;
	m_draw_timer->adjust(attotime::never);

	// check if there is another command in the FIFO
	if(s3virge.s3d.cmd_fifo_slots_free < 16)
		command_start();
	else
		s3virge.s3d.busy = false;

	LOGMMIO("Command finished [%u] (%u slots free)\n",s3virge.s3d.cmd_fifo_current_ptr,s3virge.s3d.cmd_fifo_slots_free);
}

void s3virge_vga_device::line2d_step()
{
	command_finish();
}

void s3virge_vga_device::poly2d_step()
{
	command_finish();
}

void s3virge_vga_device::line3d_step()
{
	command_finish();
}

void s3virge_vga_device::poly3d_step()
{
	command_finish();
}


// ROP names are in Reverse Polish
// Upper cases denotes an entity (Dst, Src, Pat)
// Lower cases the type of instruction (xor, not, and, or)
// leftmost lowercase applies to rightmost uppercase first.
uint32_t s3virge_vga_device::GetROP(uint8_t rop, uint32_t src, uint32_t dst, uint32_t pat)
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
			popmessage("video/s3virge.cpp: Unimplemented ROP 0x%02x",rop);
	}

	return ret;
}

bool s3virge_vga_device::advance_pixel()
{
	bool xpos, ypos;
	int16_t top, left, right, bottom;
	// advance src/dst and pattern location
	xpos = s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_COMMAND] & 0x02000000;  // X Positive
	ypos = s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_COMMAND] & 0x04000000;  // Y Positive
	if(xpos)
	{
		left = s3virge.s3d.bitblt_x_dst;
		right = s3virge.s3d.bitblt_x_dst + s3virge.s3d.bitblt_width + 1;
		s3virge.s3d.bitblt_x_current++;
		s3virge.s3d.bitblt_x_src_current++;
		s3virge.s3d.bitblt_pat_x++;
	}
	else
	{
		left = s3virge.s3d.bitblt_x_dst - s3virge.s3d.bitblt_width - 1;
		right = s3virge.s3d.bitblt_x_dst;
		s3virge.s3d.bitblt_x_current--;
		s3virge.s3d.bitblt_x_src_current--;
		s3virge.s3d.bitblt_pat_x--;
//      machine().debug_break();
	}
	if(ypos)
	{
		top = s3virge.s3d.bitblt_y_dst;
		bottom = s3virge.s3d.bitblt_y_dst + s3virge.s3d.bitblt_height;
	}
	else
	{
		top = s3virge.s3d.bitblt_y_dst - s3virge.s3d.bitblt_height;
		bottom = s3virge.s3d.bitblt_y_dst;
	}
	if(s3virge.s3d.bitblt_pat_x < 0 || s3virge.s3d.bitblt_pat_x >= 8)
		s3virge.s3d.bitblt_pat_x = s3virge.s3d.bitblt_x_current % 8;
	if((s3virge.s3d.bitblt_x_current >= right) || (s3virge.s3d.bitblt_x_current <= left))
	{
		s3virge.s3d.bitblt_x_current = s3virge.s3d.bitblt_x_dst;
		s3virge.s3d.bitblt_x_src_current = s3virge.s3d.bitblt_x_src;
		if(ypos)
		{
			s3virge.s3d.bitblt_y_current++;
			s3virge.s3d.bitblt_y_src_current++;
			s3virge.s3d.bitblt_pat_y++;
		}
		else
		{
			s3virge.s3d.bitblt_y_current--;
			s3virge.s3d.bitblt_y_src_current--;
			s3virge.s3d.bitblt_pat_y--;
		}
		s3virge.s3d.bitblt_pat_x = s3virge.s3d.bitblt_x_current % 8;
		if(s3virge.s3d.bitblt_pat_y >= 8 || s3virge.s3d.bitblt_pat_y < 0)
			s3virge.s3d.bitblt_pat_y = s3virge.s3d.bitblt_y_current % 8;
		logerror("SRC: %i,%i  DST: %i,%i PAT: %i,%i Bounds: %i,%i,%i,%i\n",
				s3virge.s3d.bitblt_x_src_current,s3virge.s3d.bitblt_y_src_current,
				s3virge.s3d.bitblt_x_current,s3virge.s3d.bitblt_y_current,
				s3virge.s3d.bitblt_pat_x,s3virge.s3d.bitblt_pat_y,
				left,right,top,bottom);
		if((s3virge.s3d.bitblt_y_current >= bottom) || (s3virge.s3d.bitblt_y_current <= top))
			return true;
	}
	return false;
}

void s3virge_vga_device::bitblt_step()
{
	if((s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_COMMAND] & 0x40))
		bitblt_monosrc_step();
	else
		bitblt_colour_step();
}

void s3virge_vga_device::bitblt_colour_step()
{
	// progress current BitBLT operation
	// get source and destination addresses
	uint32_t src_base = s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_SRC_BASE] & 0x003ffff8;
	uint32_t dst_base = s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_DEST_BASE] & 0x003ffff8;

	const u32 current_command = s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_COMMAND];
	const uint8_t pixel_size = (current_command & 0x0000001c) >> 2;
	const uint8_t rop = (current_command & 0x01fe0000) >> 17;
	const int align = (current_command & 0x000000c00) >> 10;
	//const bool tp = bool(BIT(current_command, 9));
	const bool de = bool(BIT(current_command, 5));

	uint32_t src = 0;
	uint32_t dst = 0;
	uint32_t pat = 0;
	int x;
	bool done = false;

	switch(pixel_size)
	{
		case 0:  // 8bpp
			for(x=0;x<4;x++)
			{
				if(current_command & 0x80)
					src = s3virge.s3d.image_xfer >> (x*8);
				else
					src = read_pixel8(src_base,s3virge.s3d.bitblt_x_src_current,s3virge.s3d.bitblt_y_src_current, src_stride());
				if(current_command & 0x100)
				{
					pat = (s3virge.s3d.bitblt_mono_pattern & (1 << ((s3virge.s3d.bitblt_pat_y*8) + (7-s3virge.s3d.bitblt_pat_x)))
						? s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_PAT_FG_CLR] : s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_PAT_BG_CLR]);
				}
				else
				{
					pat = s3virge.s3d.pattern[(s3virge.s3d.bitblt_pat_y * 8) + s3virge.s3d.bitblt_pat_x];
				}
				dst = read_pixel8(dst_base,s3virge.s3d.bitblt_x_current,s3virge.s3d.bitblt_y_current, dest_stride());

				if (de)
					write_pixel8(dst_base, s3virge.s3d.bitblt_x_current, s3virge.s3d.bitblt_y_current, GetROP(rop, src, dst, pat) & 0xff);

				done = advance_pixel();
				if(done)
				{
					command_finish();
					break;
				}
				if((current_command & 0x80) && s3virge.s3d.bitblt_x_current == s3virge.s3d.bitblt_x_dst)
				{
					if(align == 2) // doubleword aligned, end here
						break;
					if(align == 1) // word aligned, move to next word
					{
						if(x < 2)
							x = 2;
						else
							break;
					}
				}
			}
			break;
		case 1:  // 16bpp
			if(current_command & 0x80)
				src = s3virge.s3d.image_xfer;
			else
				src = read_pixel16(src_base,s3virge.s3d.bitblt_x_src_current,s3virge.s3d.bitblt_y_src_current, src_stride());
			dst = read_pixel16(dst_base,s3virge.s3d.bitblt_x_current,s3virge.s3d.bitblt_y_current, dest_stride());
			if(current_command & 0x100)
			{
				pat = (s3virge.s3d.bitblt_mono_pattern & (1 << ((s3virge.s3d.bitblt_pat_y*8) + (7-s3virge.s3d.bitblt_pat_x)))
					? s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_PAT_FG_CLR] : s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_PAT_BG_CLR]);
			}
			else
				pat = s3virge.s3d.pattern[(s3virge.s3d.bitblt_pat_y*16) + (s3virge.s3d.bitblt_pat_x*2)] | (s3virge.s3d.pattern[(s3virge.s3d.bitblt_pat_y*16) + (s3virge.s3d.bitblt_pat_x*2) + 1]) << 8;

			if (de)
				write_pixel16(dst_base,s3virge.s3d.bitblt_x_current,s3virge.s3d.bitblt_y_current,GetROP(rop, src, dst, pat) & 0xffff);
			done = advance_pixel();
			if(done)
			{
				command_finish();
				break;
			}
			if((current_command & 0x80) && s3virge.s3d.bitblt_x_current == s3virge.s3d.bitblt_x_dst && align == 2)
				break;  // if a new line of an image transfer, and is dword aligned, stop here
			if(current_command & 0x80)
				src = s3virge.s3d.image_xfer >> 16;
			else
				src = read_pixel16(src_base,s3virge.s3d.bitblt_x_src_current,s3virge.s3d.bitblt_y_src_current, src_stride());
			dst = read_pixel16(dst_base,s3virge.s3d.bitblt_x_current,s3virge.s3d.bitblt_y_current, dest_stride());
			if(current_command & 0x100)
			{
				pat = (s3virge.s3d.bitblt_mono_pattern & (1 << ((s3virge.s3d.bitblt_pat_y*8) + (7-s3virge.s3d.bitblt_pat_x)))
					? s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_PAT_FG_CLR] : s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_PAT_BG_CLR]);
			}
			else
				pat = s3virge.s3d.pattern[(s3virge.s3d.bitblt_pat_y*16) + (s3virge.s3d.bitblt_pat_x*2)] | (s3virge.s3d.pattern[(s3virge.s3d.bitblt_pat_y*16) + (s3virge.s3d.bitblt_pat_x*2) + 1]) << 8;
			if (de)
				write_pixel16(dst_base,s3virge.s3d.bitblt_x_current,s3virge.s3d.bitblt_y_current,GetROP(rop, src, dst, pat) & 0xffff);
			if(advance_pixel())
				command_finish();
			break;
		case 2:  // 24bpp
			if(current_command & 0x80)
			{
				src = s3virge.s3d.image_xfer;
				for(x=0;x<4;x++)
				{
					s3virge.s3d.bitblt_current_pixel |= ((s3virge.s3d.image_xfer >> (x*8)) & 0xff) << s3virge.s3d.bitblt_pixel_pos*8;
					s3virge.s3d.bitblt_pixel_pos++;
					if(s3virge.s3d.bitblt_pixel_pos > 2)
					{
						s3virge.s3d.bitblt_pixel_pos = 0;
						dst = read_pixel24(dst_base,s3virge.s3d.bitblt_x_current,s3virge.s3d.bitblt_y_current, dest_stride());
						if(current_command & 0x100)
						{
							pat = (s3virge.s3d.bitblt_mono_pattern & (1 << ((s3virge.s3d.bitblt_pat_y*8) + (7-s3virge.s3d.bitblt_pat_x)))
								? s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_PAT_FG_CLR] : s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_PAT_BG_CLR]);
						}
						else
							pat = s3virge.s3d.pattern[(s3virge.s3d.bitblt_pat_y*24) + (s3virge.s3d.bitblt_pat_x*3)] | (s3virge.s3d.pattern[(s3virge.s3d.bitblt_pat_y*24) + (s3virge.s3d.bitblt_pat_x*3) + 1]) << 8
								| (s3virge.s3d.pattern[(s3virge.s3d.bitblt_pat_y*24) + (s3virge.s3d.bitblt_pat_x*3) + 2]) << 16;
						if (de)
							write_pixel24(dst_base,s3virge.s3d.bitblt_x_current,s3virge.s3d.bitblt_y_current,GetROP(rop, s3virge.s3d.bitblt_current_pixel, dst, pat));
						s3virge.s3d.bitblt_current_pixel = 0;
						done = advance_pixel();
						if((current_command & 0x80) && s3virge.s3d.bitblt_x_current == s3virge.s3d.bitblt_x_dst)
						{
							if(align == 2) // doubleword aligned, end here
								x = 4;
							if(align == 1) // word aligned, move to next word
							{
								if(x < 2)
									x = 2;
								else
									x = 4;
							}
						}
						if(done)
							command_finish();
					}
				}
				break;
			}
			else
			{
				src = read_pixel24(src_base,s3virge.s3d.bitblt_x_src_current,s3virge.s3d.bitblt_y_src_current, src_stride());
				dst = read_pixel24(dst_base,s3virge.s3d.bitblt_x_current,s3virge.s3d.bitblt_y_current, dest_stride());
				if(current_command & 0x100)
				{
					pat = (s3virge.s3d.bitblt_mono_pattern & (1 << ((s3virge.s3d.bitblt_pat_y*8) + (7-s3virge.s3d.bitblt_pat_x)))
						? s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_PAT_FG_CLR] : s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_PAT_BG_CLR]);
				}
				else
					pat = s3virge.s3d.pattern[(s3virge.s3d.bitblt_pat_y*24) + (s3virge.s3d.bitblt_pat_x*3)] | (s3virge.s3d.pattern[(s3virge.s3d.bitblt_pat_y*24) + (s3virge.s3d.bitblt_pat_x*3) + 1]) << 8
						| (s3virge.s3d.pattern[(s3virge.s3d.bitblt_pat_y*24) + (s3virge.s3d.bitblt_pat_x*3) + 2]) << 16;
			}
			if (de)
				write_pixel24(dst_base,s3virge.s3d.bitblt_x_current,s3virge.s3d.bitblt_y_current,GetROP(rop, src, dst, pat));
			if(advance_pixel())
				command_finish();
			break;
	}

	s3virge.s3d.bitblt_step_count++;
}

void s3virge_vga_device::bitblt_monosrc_step()
{
	// progress current monochrome source BitBLT operation
	uint32_t src_base = s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_SRC_BASE] & 0x003ffff8;
	uint32_t dst_base = s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_DEST_BASE] & 0x003ffff8;

	const u32 current_command = s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_COMMAND];
	const uint8_t pixel_size = (current_command & 0x0000001c) >> 2;
	const uint8_t rop = (current_command & 0x01fe0000) >> 17;
	//const bool tp = bool(BIT(current_command, 9));
	const bool de = bool(BIT(current_command, 5));
	const int align = (current_command & 0x000000c00) >> 10;

	uint32_t src = 0;
	uint32_t dst = 0;
	// Windows 98 cares about this being initialized to non-zero for
	// greyed back/forward icons in Explorer, system icons and right click disabled Paste command.
	uint32_t pat = s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_PAT_FG_CLR];
	int x;
	bool done = false;

	switch(pixel_size)
	{
		case 0:  // 8bpp
			for(x=31;x>=0;x--)
			{
				if(current_command & 0x80)
					src = bitswap<32>(s3virge.s3d.image_xfer,7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8,23,22,21,20,19,18,17,16,31,30,29,28,27,26,25,24);
				else
					src = read_pixel8(src_base,s3virge.s3d.bitblt_x_src_current,s3virge.s3d.bitblt_y_src_current, src_stride());
				dst = read_pixel8(dst_base,s3virge.s3d.bitblt_x_current,s3virge.s3d.bitblt_y_current, dest_stride());

				if (de)
				{
					if(src & (1 << x))
						write_pixel8(dst_base,s3virge.s3d.bitblt_x_current,s3virge.s3d.bitblt_y_current,GetROP(rop, s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_SRC_FG_CLR], dst, pat) & 0xff);
					else if(!(current_command & 0x200)) // only draw background colour if transparency is not set
						write_pixel8(dst_base,s3virge.s3d.bitblt_x_current,s3virge.s3d.bitblt_y_current,GetROP(rop, s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_SRC_BG_CLR], dst, pat) & 0xff);
				}
				//printf("Pixel write(%i): X: %i Y: %i SRC: %04x DST: %04x PAT: %04x ROP: %02x\n",x,s3virge.s3d.bitblt_x_current, s3virge.s3d.bitblt_y_current, src, dst, pat, rop);
				done = advance_pixel();
				if((current_command & 0x80) && s3virge.s3d.bitblt_x_current == s3virge.s3d.bitblt_x_dst)
				{
					switch(align)
					{
						case 0:
							x &= ~7;
							break;
						case 1:
							x &= ~15;
							break;
						case 2:
							x = -1;
							break;
					}
					if(done)
					{
						command_finish();
						break;
					}
				}
			}
			break;
		case 1:  // 16bpp
			for(x=31;x>=0;x--)
			{
				if(current_command & 0x80)
					src = bitswap<32>(s3virge.s3d.image_xfer,7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8,23,22,21,20,19,18,17,16,31,30,29,28,27,26,25,24);
				else
					src = read_pixel16(src_base,s3virge.s3d.bitblt_x_src_current,s3virge.s3d.bitblt_y_src_current, src_stride());
				dst = read_pixel16(dst_base,s3virge.s3d.bitblt_x_current,s3virge.s3d.bitblt_y_current, dest_stride());

				if (de)
				{
					if(src & (1 << x))
						write_pixel16(dst_base,s3virge.s3d.bitblt_x_current,s3virge.s3d.bitblt_y_current,GetROP(rop, s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_SRC_FG_CLR], dst, pat) & 0xffff);
					else if(!(current_command & 0x200)) // only draw background colour if transparency is not set
						write_pixel16(dst_base,s3virge.s3d.bitblt_x_current,s3virge.s3d.bitblt_y_current,GetROP(rop, s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_SRC_BG_CLR], dst, pat) & 0xffff);
				}
				//printf("Pixel write(%i): X: %i Y: %i SRC: %04x DST: %04x PAT: %04x ROP: %02x\n",x,s3virge.s3d.bitblt_x_current, s3virge.s3d.bitblt_y_current, src, dst, pat, rop);
				done = advance_pixel();
				if((current_command & 0x80) && s3virge.s3d.bitblt_x_current == s3virge.s3d.bitblt_x_dst)
				{
					switch(align)
					{
						case 0:
							x &= ~7;
							break;
						case 1:
							x &= ~15;
							break;
						case 2:
							x = -1;
							break;
					}
					if(done)
					{
						command_finish();
						break;
					}
				}
			}
			break;
		case 2:  // 24bpp
			for(x=31;x>=0;x--)
			{
				if(current_command & 0x80)
					src = bitswap<32>(s3virge.s3d.image_xfer,7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8,23,22,21,20,19,18,17,16,31,30,29,28,27,26,25,24);
				else
					src = read_pixel24(src_base,s3virge.s3d.bitblt_x_src_current,s3virge.s3d.bitblt_y_src_current, src_stride());
				dst = read_pixel24(dst_base,s3virge.s3d.bitblt_x_current,s3virge.s3d.bitblt_y_current, dest_stride());

				if (de)
				{
					if(src & (1 << x))
						write_pixel24(dst_base,s3virge.s3d.bitblt_x_current,s3virge.s3d.bitblt_y_current,GetROP(rop, s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_SRC_FG_CLR], dst, pat));
					else if(!(current_command & 0x200)) // only draw background colour if transparency is not set
						write_pixel24(dst_base,s3virge.s3d.bitblt_x_current,s3virge.s3d.bitblt_y_current,GetROP(rop, s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_SRC_BG_CLR], dst, pat));
				}
				//printf("Pixel write(%i): X: %i Y: %i SRC: %04x DST: %04x PAT: %04x ROP: %02x\n",x,s3virge.s3d.bitblt_x_current, s3virge.s3d.bitblt_y_current, src, dst, pat, rop);
				done = advance_pixel();
				if((current_command & 0x80) && s3virge.s3d.bitblt_x_current == s3virge.s3d.bitblt_x_dst)
				{
					switch(align)
					{
						case 0:
							x &= ~7;
							break;
						case 1:
							x &= ~15;
							break;
						case 2:
							x = -1;
							break;
					}
					if(done)
					{
						command_finish();
						break;
					}
				}
			}
			break;
	}

	s3virge.s3d.bitblt_step_count++;
}

TIMER_CALLBACK_MEMBER(s3virge_vga_device::draw_step_tick)
{
	// TODO: S3D state timing
	switch(s3virge.s3d.state)
	{
		case S3D_STATE_IDLE:
			m_draw_timer->adjust(attotime::zero);
			break;
		case S3D_STATE_2DLINE:
			line2d_step();
			break;
		case S3D_STATE_2DPOLY:
			poly2d_step();
			break;
		case S3D_STATE_3DLINE:
			line3d_step();
			break;
		case S3D_STATE_3DPOLY:
			poly3d_step();
			break;
		case S3D_STATE_BITBLT:
			bitblt_step();
			break;
	}
}

inline void s3virge_vga_device::write_pixel32(uint32_t base, uint16_t x, uint16_t y, uint32_t val)
{
	if(s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_COMMAND] & 0x00000002)
		if(x < s3virge.s3d.clip_l || x > s3virge.s3d.clip_r || y < s3virge.s3d.clip_t || y > s3virge.s3d.clip_b)
			return;
	vga.memory[(base + (x*4) + (y*dest_stride())) % vga.svga_intf.vram_size] = val & 0xff;
	vga.memory[(base + 1 + (x*4) + (y*dest_stride())) % vga.svga_intf.vram_size] = (val >> 8) & 0xff;
	vga.memory[(base + 2 + (x*4) + (y*dest_stride())) % vga.svga_intf.vram_size] = (val >> 16) & 0xff;
	vga.memory[(base + 3 + (x*4) + (y*dest_stride())) % vga.svga_intf.vram_size] = (val >> 24) & 0xff;
}

inline void s3virge_vga_device::write_pixel24(uint32_t base, uint16_t x, uint16_t y, uint32_t val)
{
	if(s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_COMMAND] & 0x00000002)
		if(x < s3virge.s3d.clip_l || x > s3virge.s3d.clip_r || y < s3virge.s3d.clip_t || y > s3virge.s3d.clip_b)
			return;
	vga.memory[(base + (x*3) + (y*dest_stride())) % vga.svga_intf.vram_size] = val & 0xff;
	vga.memory[(base + 1 + (x*3) + (y*dest_stride())) % vga.svga_intf.vram_size] = (val >> 8) & 0xff;
	vga.memory[(base + 2 + (x*3) + (y*dest_stride())) % vga.svga_intf.vram_size] = (val >> 16) & 0xff;
}

inline void s3virge_vga_device::write_pixel16(uint32_t base, uint16_t x, uint16_t y, uint16_t val)
{
	if(s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_COMMAND] & 0x00000002)
		if(x < s3virge.s3d.clip_l || x > s3virge.s3d.clip_r || y < s3virge.s3d.clip_t || y > s3virge.s3d.clip_b)
			return;
	vga.memory[(base + (x*2) + (y*dest_stride())) % vga.svga_intf.vram_size] = val & 0xff;
	vga.memory[(base + 1 + (x*2) + (y*dest_stride())) % vga.svga_intf.vram_size] = (val >> 8) & 0xff;
}

inline void s3virge_vga_device::write_pixel8(uint32_t base, uint16_t x, uint16_t y, uint8_t val)
{
	if(s3virge.s3d.cmd_fifo[s3virge.s3d.cmd_fifo_current_ptr].reg[S3D_REG_COMMAND] & 0x00000002)
		if(x < s3virge.s3d.clip_l || x > s3virge.s3d.clip_r || y < s3virge.s3d.clip_t || y > s3virge.s3d.clip_b)
			return;
	vga.memory[(base + x + (y*dest_stride())) % vga.svga_intf.vram_size] = val;
}

inline uint32_t s3virge_vga_device::read_pixel32(uint32_t base, uint16_t x, uint16_t y, u16 stride_select)
{
	return (vga.memory[(base + (x * 4) + (y * stride_select)) % vga.svga_intf.vram_size] << 24) |
		   (vga.memory[(base + 1 + (x * 4) + (y * stride_select)) % vga.svga_intf.vram_size] << 16) |
		   (vga.memory[(base + 2 + (x * 4) + (y * stride_select)) % vga.svga_intf.vram_size] << 8) |
		   vga.memory[(base + 3 + (x * 4) + (y * stride_select)) % vga.svga_intf.vram_size];
}

inline uint32_t s3virge_vga_device::read_pixel24(uint32_t base, uint16_t x, uint16_t y, u16 stride_select)
{
	return (vga.memory[(base + (x * 3) + (y * stride_select)) % vga.svga_intf.vram_size]) |
		   (vga.memory[(base + 1 + (x * 3) + (y * stride_select)) % vga.svga_intf.vram_size] << 8) |
		   (vga.memory[(base + 2 + (x * 3) + (y * stride_select)) % vga.svga_intf.vram_size] << 16);
}

inline uint16_t s3virge_vga_device::read_pixel16(uint32_t base, uint16_t x, uint16_t y, u16 stride_select)
{
	return (vga.memory[(base + (x * 2) + (y * stride_select) % vga.svga_intf.vram_size)]) |
		   (vga.memory[(base + 1 + (x * 2) + (y * stride_select)) % vga.svga_intf.vram_size] << 8);
}

inline uint8_t s3virge_vga_device::read_pixel8(uint32_t base, uint16_t x, uint16_t y, u16 stride_select)
{
	return vga.memory[(base + x + (y * stride_select)) % vga.svga_intf.vram_size];
}

// 2D command register format - A500 (BitBLT), A900 (2D line), AD00 (2D Polygon)
// bit 0 - Autoexecute, if set command is executed when the highest relevant register is written to (A50C / A97C / AD7C)
// bit 1 - Enable hardware clipping
// bits 2-4 - Destination colour format - (0 = 8bpp palettised, 1 = 16bpp RGB1555 or RGB565, 2 = 24bpp RGB888
// bit 5 - Draw enable - if reset, doesn't draw anything, but is still executed
// bit 6 - Image source Mono transfer, if set source is mono, otherwise source is the same pixel depth as the destination
// bit 7 - Image data source - 0 = source is in video memory, 1 = source is from the image transfer port (CPU / system memory)
// bit 8 - Mono pattern - if set, pattern data is mono, otherwise pattern data is the same pixel depth as the destination
//         Cleared to 0 if using an ROP with a colour source  Must be set to 1 if doing a rectangle fill operation
// bit 9 - Transparency - if set, does not update if a background colour is selected.  Effectively only if bit 7 is set,  Typically used for text display.
// bits 10-11 - Image transfer alignment - Data for an image transfer is byte (0), word (1), or doubleword (2) aligned.  All image transfers are doubleword in size.
// bits 12-13 - First doubleword offset - (Image transfers) - start with the given byte (+1) in a doubleword for an image transfer
// bits 17-24 - MS Windows Raster Operation
// bit 25 - X Positive - if set, BitBLT is performed from left to right, otherwise, from right to left
// bit 26 - Y Positive - if set, BitBLT is performed from top to bottom, otherwise from bottom to top
// bits 27-30 - 2D Command - 0000 = BitBLT, 0010 = Rectangle Fill, 0011 = Line Draw, 0101 = Polygon Fill, 1111 = NOP (Turns off autoexecute without executing a command)
// bit 31 - 2D / 3D Select


uint32_t s3virge_vga_device::s3d_sub_status_r()
{
	uint32_t res = 0x00000000;

	if(!s3virge.s3d.busy)
		res |= 0x00002000;  // S3d engine is idle

	//res |= (s3virge.s3d.cmd_fifo_slots_free << 8);
	if(s3virge.s3d.cmd_fifo_slots_free == 16)
		res |= 0x1f00;

	return res;
}

void s3virge_vga_device::s3d_sub_control_w(uint32_t data)
{
	s3virge.interrupt_enable = data & 0x00003f80;
	// TODO: bits 14-15==10 - reset engine
	LOGMMIO("Sub control = %08x\n", data);
}

/*
 * Advanced Function Control Register (MM850C)
 * ---- --xx xx-- ---- command fifo status
 * ---- ---- ---x ---- LA ENB Linear Addressing Enable (mirror of CR58 bit 4)
 * ---- ---- ---- --x- RST DM Reset read DMA
 * ---- ---- ---- ---x ENB EHFC Enable enhanced functions (mirror of CR66 bit 0)
 */
uint32_t s3virge_vga_device::s3d_func_ctrl_r()
{
	uint32_t ret = 0;

	ret |= (s3virge.s3d.cmd_fifo_slots_free << 6);
	return ret;
}

uint32_t s3virge_vga_device::s3d_register_r(offs_t offset)
{
	uint32_t res = 0;
	int op_type = (((offset*4) & 0x1c00) >> 10) - 1;

	// unused registers
	if(offset < 0x100/4)
		return 0;
	if(offset >= 0x1c0/4 && offset < 0x400/4)
		return 0;

	// handle BitBLT pattern registers
	if((offset >= 0x100/4) && (offset < 0x1c0/4))
		return s3virge.s3d.pattern[offset - (0x100/4)];

	res = s3virge.s3d.reg[op_type][((offset*4) & 0x03ff) / 4];
	LOGMMIO("MM%04X returning %08x\n", (offset*4)+0xa000, res);

	return res;
}

void s3virge_vga_device::s3d_register_w(offs_t offset, uint32_t data)
{
	int op_type = (((offset*4) & 0x1c00) >> 10) - 1;

	// unused registers
	if(offset < 0x100/4)
		return;
	if(offset >= 0x1c0/4 && offset < 0x400/4)
		return;

	// handle BitBLT pattern registers
	if((offset >= 0x100/4) && (offset < 0x1c0/4))
	{
		//COMBINE_DATA(&s3virge.s3d.pattern[(offset*4) - (0x100/4)]);
		s3virge.s3d.pattern[((offset - 0x100/4)*4)+3] = (data & 0xff000000) >> 24;
		s3virge.s3d.pattern[((offset - 0x100/4)*4)+2] = (data & 0x00ff0000) >> 16;
		s3virge.s3d.pattern[((offset - 0x100/4)*4)+1] = (data & 0x0000ff00) >> 8;
		s3virge.s3d.pattern[((offset - 0x100/4)*4)] = (data & 0x000000ff);
		return;
	}

	s3virge.s3d.reg[op_type][((offset*4) & 0x03ff) / 4] = data;
	LOGMMIO("MM%04X = %08x\n", (offset*4)+0xa000, data);
	switch(offset)
	{
		case 0x500/4:
			if(!(data & 0x00000001))
				add_command(op_type);
			break;
		case 0x50c/4:
			if(s3virge.s3d.reg[op_type][S3D_REG_COMMAND] & 0x00000001)  // autoexecute enabled
				add_command(op_type);
			break;
	}
}

