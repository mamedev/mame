// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * s3virge.cpp
 *
 * Implementation of the S3 ViRGE series of video card
 * ViRGE = Video and Rendering Graphics Engine
 *
 * TODO:
 * - Complete FIFO details, fix remaining stalls (win2k);
 * - Backport 2d engine to Trio64;
 * - Implement 3d commands, cfr. s3dsdk demos;
 * - Implement remaining ROP commands;
 * - Primary stream details, namely FIFO thresholds and start address override (any DirectX 5 2D game)
 * - Secondary stream mixing;
 * - S3 Scenic Highway i/f (SAA7110 + S3 Scenic/MX2 MPEG-1);
 * - DMAs;
 * - interrupts;
 * - big endian support for non-x86 machines;
 * - DDC/I2C i/f, cfr. serial port on MMFF20;
 * - Fix PLL calculation for 1k+ width VESA modes (tends to either be too fast or too slow);
 * - 1600x1200x4 needs line compare fix in downstream pc_vga (cuts too early);
 * - 1280x1024x16 draws 256 H and stupid high refresh rate;
 *
 * Notes:
 * - Most Windows s3dsdk demos starts in software render (at least with win98se base S3 drivers,
 *   cfr. cube.exe), work around by reselecting File -> Direct3D HAL or flip the
 *   Maximize/Restore Down window button
 *
 */

#include "emu.h"
#include "s3virge.h"

#include "screen.h"

//#include <iostream>

#define LOG_REG     (1U << 1)
#define LOG_CMD     (1U << 2)
#define LOG_MMIO    (1U << 3)
#define LOG_PIXEL   (1U << 4) // log pixel writes (verbose)
#define LOG_FIFO    (1U << 5)
#define LOG_STREAMS (1U << 6)

#define VERBOSE (LOG_REG | LOG_CMD | LOG_MMIO | LOG_FIFO | LOG_STREAMS)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGREG(...)       LOGMASKED(LOG_REG, __VA_ARGS__)
#define LOGCMD(...)       LOGMASKED(LOG_CMD, __VA_ARGS__)
#define LOGMMIO(...)      LOGMASKED(LOG_MMIO, __VA_ARGS__)
#define LOGPIXEL(...)     LOGMASKED(LOG_PIXEL, __VA_ARGS__)
#define LOGFIFO(...)      LOGMASKED(LOG_FIFO, __VA_ARGS__)
#define LOGSTREAMS(...)   LOGMASKED(LOG_STREAMS, __VA_ARGS__)

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
	: s3trio64_vga_device(mconfig, type, tag, owner, clock)
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

	m_vblank_timer = timer_alloc(FUNC(s3virge_vga_device::vblank_timer_cb), this);
	m_op_timer = timer_alloc(FUNC(s3virge_vga_device::op_timer_cb), this);

	memset(&s3, 0, sizeof(s3));
	m_linear_address = 0x70000000;
	m_linear_address_size_full = 0x10000;

	save_item(m_bitblt.pattern,"S3D Pattern Data");
//  save_item(m_bitblt.reg[0],"S3D Registers: BitBLT");
//  save_item(m_bitblt.reg[1],"S3D Registers: 2D Line");
//  save_item(m_bitblt.reg[2],"S3D Registers: 2D Polygon");
//  save_item(m_bitblt.reg[3],"S3D Registers: 3D Line");
//  save_item(m_bitblt.reg[4],"S3D Registers: 3D Triangle");

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

void s3virge_vga_device::s3d_reset()
{
	LOGFIFO("S3D Reset\n");
	m_streams.psidf = 0;
	m_streams.pshfc = 0;
	m_bitblt_fifo.clear();
	m_xfer_fifo.clear();
	m_bitblt.xfer_mode = false;
	m_op_timer->adjust(attotime::never);

	m_s3d_state = S3D_STATE_IDLE;
	// beos 4.x (and presumably Win 3.1) never sets this when using BitBlt,
	// expecting 0xf0 ROPs to be 1-filled rather than 0
	m_bitblt_latch[5] = 0xffff'ffff;
	m_bitblt_latch[6] = 0xffff'ffff;
	// TODO: the rest of the pipeline, particularly more state reset on dual boot transitions.
	// Notice that some stuff may really need real HW checks, namely streams processor or irq enable
}

void s3virge_vga_device::device_reset()
{
	s3trio64_vga_device::device_reset();
	// Power-on strapping bits.  Sampled at reset, but can be modified later.
	// These are just assumed defaults.
	s3.strapping = 0x000f0912;

	s3d_reset();
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

// base 0x8180
void s3virge_vga_device::streams_control_map(address_map &map)
{
	map(0x0000, 0x0003).lrw32(
		NAME([this] (offs_t offset) {
			return (m_streams.psidf << 24) | (m_streams.pshfc << 28);
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_24_31)
			{
				m_streams.psidf = (data >> 24) & 7;
				m_streams.pshfc = (data >> 28) & 7;
			}
			LOGSTREAMS("MM8180 (Primary Stream Control) %08x & %08x\n", data, mem_mask);
		})
	);
//  map(0x0004, 0x0007) Color/Chroma Key Control (MM8184)
//  map(0x0010, 0x0013) Secondary Stream Control (MM8190)
//  map(0x0014, 0x0017) Chroma Key Upper Bound (MM8194)
//  map(0x0018, 0x001b) Secondary Stream Stretch/Filter Constants (MM8198)
//  map(0x0020, 0x0023) Blend Control (MM81A0)
//  map(0x0040, 0x0043) Primary Stream Frame Buffer Address 0 (MM81C0)
//  map(0x0044, 0x0047) Primary Stream Frame Buffer Address 1 (MM81C4)
	map(0x0048, 0x004b).lrw32(
		NAME([this] (offs_t offset) {
			return (m_streams.primary_stride);
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_streams.primary_stride);
			m_streams.primary_stride &= 0xfff;
			LOGSTREAMS("MM81C8 (Primary Stream Stride) %08x & %08x\n", data, mem_mask);
		})
	);
}

uint16_t s3virge_vga_device::offset()
{
	if (s3.ext_misc_ctrl_2 & 0xc)
		return m_streams.primary_stride;

	// NOTE: same as Vision968
	if (s3.memory_config & 0x08)
		return vga.crtc.offset << 3;
	return vga_device::offset();
}

void s3virge_vga_device::crtc_map(address_map &map)
{
	s3trio64_vga_device::crtc_map(map);
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
			m_linear_config_changed_cb(m_linear_address_enable);
		})
	);
	map(0x58, 0x58).lrw8(
		NAME([this] (offs_t offset) {
			u8 res = m_linear_address_size & 0x03;
			res   |= m_linear_address_enable ? 0x10 : 0x00;
			return res;
		}),
		NAME([this] (offs_t offset, u8 data) {
			const uint8_t old_size = m_linear_address_size;
			const bool old_enable = m_linear_address_enable;
			const bool size_changed = old_size != m_linear_address_size;

			m_linear_address_size = data & 0x03;
			m_linear_address_enable = data & 0x10;

			switch(data & 0x03)
			{
				case LAW_64K:
					m_linear_address_size_full = 0x10000;
					break;
				case LAW_1MB:
					m_linear_address_size_full = 0x100000;
					break;
				case LAW_2MB:
					m_linear_address_size_full = 0x200000;
					break;
				case LAW_4MB:
					m_linear_address_size_full = 0x400000;
					break;
			}

			if ((m_linear_address_enable != old_enable) || size_changed)
			{
				m_linear_config_changed_cb(m_linear_address_enable);
			}

			LOGREG("CR58: write %02x\n", data);
		})
	);
	map(0x59, 0x59).lrw8(
		NAME([this] (offs_t offset) {
			return (m_linear_address & 0xff000000) >> 24;
		}),
		NAME([this] (offs_t offset, u8 data) {
			const uint32_t old_address = m_linear_address;
			m_linear_address = (m_linear_address & 0x00ff0000) | (data << 24);
			LOGREG("Linear framebuffer address = %08x\n",m_linear_address);

			if (old_address != m_linear_address && m_linear_address_enable)
			{
				m_linear_config_changed_cb(1);
			}
		})
	);
	map(0x5a, 0x5a).lrw8(
		NAME([this] (offs_t offset) {
			u8 res = 0;
			switch(m_linear_address_size & 0x03)
			{
				case 0:  // 64kB
				default:
					res = (m_linear_address & 0x00ff0000) >> 16;
					break;
				case 1:  // 1MB
					res = (m_linear_address & 0x00f00000) >> 16;
					break;
				case 2:  // 2MB
					res = (m_linear_address & 0x00e00000) >> 16;
					break;
				case 3:  // 4MB
					res = (m_linear_address & 0x00c00000) >> 16;
					break;
			}
			return res;
		}),
		NAME([this] (offs_t offset, u8 data) {
			const uint32_t old_address = m_linear_address;
			m_linear_address = (m_linear_address & 0xff000000) | (data << 16);
			LOGREG("Linear framebuffer address = %08x\n",m_linear_address);

			if (old_address != m_linear_address && m_linear_address_enable)
			{
				m_linear_config_changed_cb(1);
			}
		})
	);
	//map(0x5d, 0x5e).unmapr();
	map(0x66, 0x66).lrw8(
		NAME([this] (offs_t offset) {
			return m_cr66;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_cr66 = data;
			LOGREG("CR66: write %02x\n", data);
			// bit 0: ENBL ENH Enable Enhanced Functions
			if (BIT(data, 1))
				s3d_reset();
			// bit 6: TOFF PADT - Tri-State Off Pixel Address bus
			// TODO: bit 7 enables, bit 3 will disconnect from the PCI bus if FIFO under/overflow happens
			// This is enabled by win98se already during startup.
			//if (data & 0x88)
			//  popmessage("s3virge.cpp: PCI disconnect enabled warning");
		})
	);
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
		svga.rgb32_en = 0;
		switch((s3.ext_misc_ctrl_2) >> 4)
		{
			case 0x01: svga.rgb8_en = 1; break;
			case 0x03: svga.rgb15_en = 1; divisor = 2; break;
			case 0x05: svga.rgb16_en = 1; divisor = 2; break;
			case 0x0d:
			{
				// if streams disabled run RAMDAC in unpacked mode
				// NOTE: it matches original Vision968 behaviour
				// - SDD and Tiny Core Linux relies on this
				if (s3.ext_misc_ctrl_2 & 0xc)
					svga.rgb24_en = 1;
				else
					svga.rgb32_en = 1;
				divisor = 1;
				break;
			}
			default: popmessage("video/s3virge.cpp: video mode not implemented %02x\n",((s3.ext_misc_ctrl_2) >> 4));
		}
	}
	else
	{
		svga.rgb8_en = (s3.cr3a & 0x10) >> 4;
		svga.rgb15_en = 0;
		svga.rgb16_en = 0;
		svga.rgb24_en = 0;
		svga.rgb32_en = 0;
	}
	if(s3.cr43 & 0x80)  // Horizontal clock doubling (technically, doubles horizontal CRT parameters)
		divisor *= 2;

	//popmessage("%02x %02x %d", s3.cr43, s3.sr15, divisor);
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
	if(offset < m_linear_address_size_full)
		return vga.memory[offset % vga.svga_intf.vram_size];
	return 0xff;
}

void s3virge_vga_device::fb_w(offs_t offset, uint8_t data)
{
	if(offset < m_linear_address_size_full)
		vga.memory[offset % vga.svga_intf.vram_size] = data;
}

void s3virge_vga_device::add_command(u8 cmd_type)
{
	// TODO: handle full FIFO (should discard operation?)
	if (m_bitblt_fifo.full())
		throw emu_fatalerror("s3virge: FIFO full");

	for (int i = 0; i < 15; i++)
		m_bitblt_fifo.enqueue(m_bitblt_latch[i]);

	if (m_s3d_state == S3D_STATE_IDLE)
	{
		m_op_timer->adjust(attotime::from_nsec(250), 0, attotime::from_nsec(250));
		m_s3d_state = S3D_STATE_COMMAND_RX;
	}

	LOGFIFO("Enqueue command type %i %08x (%d free)\n", cmd_type, m_bitblt_latch[11], 16 - m_bitblt_fifo.queue_length() / 15);
}

TIMER_CALLBACK_MEMBER(s3virge_vga_device::op_timer_cb)
{
	switch (m_s3d_state)
	{
		case S3D_STATE_IDLE:
			return;
		case S3D_STATE_COMMAND_RX:
			// start next command in FIFO
			//const u8 cmd_type = 0; //m_bitblt.cmd_fifo[m_bitblt.cmd_fifo_current_ptr].op_type & 0xf;
			assert(!m_bitblt_fifo.empty());
			command_dequeue(OP_BITBLT);
			break;
		case S3D_STATE_BITBLT:
			if (m_bitblt.xfer_mode == true)
			{
				if (m_xfer_fifo.empty())
					return;
				m_bitblt.image_xfer = m_xfer_fifo.dequeue();
			}
			bitblt_step();
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
	}
}

void s3virge_vga_device::command_dequeue(u8 op_type)
{
	switch(op_type)
	{
		case OP_2DLINE:
			//LOGCMD("2D Line command (unsupported) [%u]\n", m_bitblt.cmd_fifo_current_ptr);
			LOGCMD("2D Line command (unsupported)\n");
			break;
		case OP_2DPOLY:
			//LOGCMD("2D Poly command (unsupported) [%u]\n", m_bitblt.cmd_fifo_current_ptr);
			LOGCMD("2D Poly command (unsupported)\n");
			break;
		case OP_3DLINE:
			//LOGCMD("3D Line command (unsupported) [%u]\n", m_bitblt.cmd_fifo_current_ptr);
			LOGCMD("3D Poly command (unsupported)\n");
			break;
		case OP_3DTRI:
			//LOGCMD("3D Tri command (unsupported) [%u]\n", m_bitblt.cmd_fifo_current_ptr);
			LOGCMD("3D Tri command (unsupported)\n");
			break;
		case OP_BITBLT:
		{
			u32 tmp;
			//const bitblt_struct command_struct = m_bitblt_fifo.dequeue();
			m_bitblt.src_base = m_bitblt_fifo.dequeue();
			m_bitblt.dest_base = m_bitblt_fifo.dequeue();
			tmp = m_bitblt_fifo.dequeue();
			m_bitblt.clip_r = tmp & 0x000007ff;
			m_bitblt.clip_l = (tmp & 0x07ff0000) >> 16;
			// $a4e0
			tmp = m_bitblt_fifo.dequeue();
			m_bitblt.clip_b = tmp & 0x000007ff;
			m_bitblt.clip_t = (tmp & 0x07ff0000) >> 16;
			tmp = m_bitblt_fifo.dequeue();
			m_bitblt.src_stride = (tmp >> 0) & 0xfff8;
			m_bitblt.dest_stride = (tmp >> 16) & 0xfff8;
			tmp = m_bitblt_fifo.dequeue();
			m_bitblt.mono_pattern = ((u64)m_bitblt_fifo.dequeue() << 32) | tmp;
			// $a4f0
			m_bitblt.pat_bg_clr = m_bitblt_fifo.dequeue();
			m_bitblt.pat_fg_clr = m_bitblt_fifo.dequeue();
			m_bitblt.src_bg_clr = m_bitblt_fifo.dequeue();
			m_bitblt.src_fg_clr = m_bitblt_fifo.dequeue();
			// $a500
			m_bitblt.command = m_bitblt_fifo.dequeue();
			tmp = m_bitblt_fifo.dequeue();
			m_bitblt.width = ((tmp & 0xffff0000) >> 16) + 1;
			m_bitblt.height = (tmp & 0x0000ffff);
			tmp = m_bitblt_fifo.dequeue();
			m_bitblt.x_src = (tmp & 0x07ff0000) >> 16;
			m_bitblt.y_src = (tmp & 0x000007ff);
			tmp = m_bitblt_fifo.dequeue();
			m_bitblt.x_dst = (tmp & 0x07ff0000) >> 16;
			m_bitblt.y_dst = (tmp & 0x000007ff);

			// TODO: these four goes negative at second transfer of beos 4 (two's complement?)
			m_bitblt.x_current = m_bitblt.x_dst;
			m_bitblt.x_src_current = m_bitblt.x_src;
			m_bitblt.y_current = m_bitblt.y_dst;
			m_bitblt.y_src_current = m_bitblt.y_src;
			m_bitblt.pat_x = m_bitblt.x_current % 8;
			m_bitblt.pat_y = m_bitblt.y_current % 8;
			m_bitblt.step_count = 0;
			m_bitblt.current_pixel = 0;
			m_bitblt.pixel_pos = 0;

			LOGFIFO("Dequeued command %08x (%d free)\n", m_bitblt.command, 16 - (m_bitblt_fifo.queue_length() / 15));

			const u32 current_command = m_bitblt.command;

			m_bitblt.xfer_mode = bool(BIT(current_command, 7));
			m_s3d_state = S3D_STATE_BITBLT;

			const u8 command_type = BIT(current_command, 27, 4);

			// NOP disables autoexecute without executing a command
			// win2k relies on this at explorer startup
			if (command_type == 0xf)
			{
				LOGCMD("BitBLT NOP encountered\n");
				m_bitblt_latch[11] &= ~1;
				m_bitblt.xfer_mode = false;
				command_finish();
				return;
			}
			else
			{
				LOGCMD("Started BitBLT command [%08x type=%02x xfer_mode=%d]\n"
					, m_bitblt.command
					, command_type
					, m_bitblt.xfer_mode
				);
			}

			break;
		}
		default:
			LOGCMD("<reserved %02x> command detected [%08x]\n", op_type, m_bitblt.command);
			break;
	}
}

void s3virge_vga_device::command_finish()
{
	LOGFIFO("Command finished (%u free) ", 16 - (m_bitblt_fifo.queue_length() / 15));

	if (m_bitblt_fifo.empty())
	{
		m_s3d_state = S3D_STATE_IDLE;
		m_op_timer->adjust(attotime::never);
		LOGFIFO("- state idle\n");
	}
	else
	{
		m_s3d_state = S3D_STATE_COMMAND_RX;
		const auto xfer_fifo = m_xfer_fifo.queue_length();
		// FIXME: without flushing xfer FIFO GFXs will go awry in win98se (i.e. when calling shutdown menu)
		// root cause of this is also causing hangs in win2k when bringing up explorer window,
		// possibly a command is misbehaving in size.
		if (xfer_fifo)
			LOGFIFO("Warning: non-empty xfer FIFO at command end (%lld)\n", xfer_fifo);
		LOGFIFO("- state new command\n");
		m_xfer_fifo.clear();
	}
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
	xpos = m_bitblt.command  & 0x02000000;  // X Positive
	ypos = m_bitblt.command  & 0x04000000;  // Y Positive
	if(xpos)
	{
		left = m_bitblt.x_dst;
		right = m_bitblt.x_dst + m_bitblt.width;
		m_bitblt.x_current++;
		m_bitblt.x_src_current++;
		m_bitblt.pat_x++;
	}
	else
	{
		// FIXME: beos 4 dominos demo
		left = m_bitblt.x_dst - m_bitblt.width;
		right = m_bitblt.x_dst;
		m_bitblt.x_current--;
		m_bitblt.x_src_current--;
		m_bitblt.pat_x--;
//      machine().debug_break();
	}
	if(ypos)
	{
		top = m_bitblt.y_dst;
		bottom = m_bitblt.y_dst + m_bitblt.height;
	}
	else
	{
		top = m_bitblt.y_dst - m_bitblt.height;
		bottom = m_bitblt.y_dst;
	}
	if(m_bitblt.pat_x < 0 || m_bitblt.pat_x >= 8)
		m_bitblt.pat_x = m_bitblt.x_current % 8;
	if((m_bitblt.x_current >= right) || (m_bitblt.x_current <= left))
	{
		m_bitblt.x_current = m_bitblt.x_dst;
		m_bitblt.x_src_current = m_bitblt.x_src;
		if(ypos)
		{
			m_bitblt.y_current++;
			m_bitblt.y_src_current++;
			m_bitblt.pat_y++;
		}
		else
		{
			m_bitblt.y_current--;
			m_bitblt.y_src_current--;
			m_bitblt.pat_y--;
		}
		m_bitblt.pat_x = m_bitblt.x_current % 8;
		if(m_bitblt.pat_y >= 8 || m_bitblt.pat_y < 0)
			m_bitblt.pat_y = m_bitblt.y_current % 8;
		LOGPIXEL("SRC: %i,%i  DST: %i,%i PAT: %i,%i Bounds: %i,%i,%i,%i\n",
				m_bitblt.x_src_current,m_bitblt.y_src_current,
				m_bitblt.x_current,m_bitblt.y_current,
				m_bitblt.pat_x,m_bitblt.pat_y,
				left,right,top,bottom);
		if((m_bitblt.y_current >= bottom) || (m_bitblt.y_current <= top))
			return true;
	}
	return false;
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

void s3virge_vga_device::bitblt_step()
{
	if(BIT(m_bitblt.command, 6))
		bitblt_monosrc_step();
	else
		bitblt_colour_step();
}

void s3virge_vga_device::bitblt_colour_step()
{
	// progress current BitBLT operation
	// get source and destination addresses
	uint32_t src_base = m_bitblt.src_base & 0x003ffff8;
	uint32_t dst_base = m_bitblt.dest_base & 0x003ffff8;
	const u32 current_command = m_bitblt.command;
	const uint8_t pixel_size = (current_command & 0x0000001c) >> 2;
	const uint8_t rop = (current_command & 0x01fe0000) >> 17;
	const int align = (current_command & 0x000000c00) >> 10;
	//const bool tp = bool(BIT(current_command, 9));
	const bool mp = bool(BIT(current_command, 8));
	const bool ids = bool(BIT(current_command, 7));
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
				if(ids)
				{
					src = m_bitblt.image_xfer >> (x*8);
					src &= 0xff;
				}
				else
					src = read_pixel8(src_base, m_bitblt.x_src_current, m_bitblt.y_src_current, m_bitblt.src_stride);
				if(mp)
				{
					//pat = (m_bitblt.mono_pattern & (1 << ((m_bitblt.pat_y*8) + (7-m_bitblt.pat_x))) ? m_bitblt.pat_fg_clr : m_bitblt.pat_bg_clr);

					pat = BIT(m_bitblt.mono_pattern, (m_bitblt.pat_y*8) + (7-m_bitblt.pat_x)) ? m_bitblt.pat_fg_clr : m_bitblt.pat_bg_clr;
				}
				else
				{
					pat = m_bitblt.pattern[(m_bitblt.pat_y * 8) + m_bitblt.pat_x];
				}
				dst = read_pixel8(dst_base, m_bitblt.x_current, m_bitblt.y_current, m_bitblt.dest_stride);

				if (de)
				{
					write_pixel8(dst_base, m_bitblt.x_current, m_bitblt.y_current, GetROP(rop, src, dst, pat) & 0xff);
				}

				done = advance_pixel();
				if(done)
				{
					command_finish();
					break;
				}
				if((ids) && m_bitblt.x_current == m_bitblt.x_dst)
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
			if(ids)
			{
				src = m_bitblt.image_xfer;
				src &= 0xffff;
			}
			else
				src = read_pixel16(src_base, m_bitblt.x_src_current, m_bitblt.y_src_current, m_bitblt.src_stride);
			dst = read_pixel16(dst_base, m_bitblt.x_current, m_bitblt.y_current, m_bitblt.dest_stride);
			if(mp)
			{
				pat = (m_bitblt.mono_pattern & (1 << ((m_bitblt.pat_y*8) + (7-m_bitblt.pat_x))))
					? m_bitblt.pat_fg_clr
					: m_bitblt.pat_bg_clr;
			}
			else
				pat = m_bitblt.pattern[(m_bitblt.pat_y*16) + (m_bitblt.pat_x*2)] | (m_bitblt.pattern[(m_bitblt.pat_y*16) + (m_bitblt.pat_x*2) + 1]) << 8;

			if (de)
				write_pixel16(dst_base, m_bitblt.x_current, m_bitblt.y_current, GetROP(rop, src, dst, pat) & 0xffff);
			done = advance_pixel();
			if(done)
			{
				command_finish();
				break;
			}
			if((ids) && m_bitblt.x_current == m_bitblt.x_dst && align == 2)
				break;  // if a new line of an image transfer, and is dword aligned, stop here
			if(ids)
				src = m_bitblt.image_xfer >> 16;
			else
				src = read_pixel16(src_base, m_bitblt.x_src_current, m_bitblt.y_src_current, m_bitblt.src_stride);
			dst = read_pixel16(dst_base, m_bitblt.x_current, m_bitblt.y_current, m_bitblt.dest_stride);
			if(mp)
			{
				pat = (m_bitblt.mono_pattern & (1 << ((m_bitblt.pat_y*8) + (7-m_bitblt.pat_x)))
					? m_bitblt.pat_fg_clr
					: m_bitblt.pat_bg_clr);
			}
			else
				pat = m_bitblt.pattern[(m_bitblt.pat_y*16) + (m_bitblt.pat_x*2)] | (m_bitblt.pattern[(m_bitblt.pat_y*16) + (m_bitblt.pat_x*2) + 1]) << 8;
			if (de)
				write_pixel16(dst_base, m_bitblt.x_current, m_bitblt.y_current, GetROP(rop, src, dst, pat) & 0xffff);
			if(advance_pixel())
				command_finish();
			break;
		case 2:  // 24bpp
			if(ids)
			{
				src = m_bitblt.image_xfer;
				for(x=0;x<4;x++)
				{
					m_bitblt.current_pixel |= ((m_bitblt.image_xfer >> (x*8)) & 0xff) << m_bitblt.pixel_pos*8;
					m_bitblt.pixel_pos++;
					if(m_bitblt.pixel_pos > 2)
					{
						m_bitblt.pixel_pos = 0;
						dst = read_pixel24(dst_base, m_bitblt.x_current, m_bitblt.y_current, m_bitblt.dest_stride);
						if(mp)
						{
							pat = (m_bitblt.mono_pattern & (1 << ((m_bitblt.pat_y*8) + (7-m_bitblt.pat_x)))
								? m_bitblt.pat_fg_clr : m_bitblt.pat_bg_clr);
						}
						else
							pat = m_bitblt.pattern[(m_bitblt.pat_y*24) + (m_bitblt.pat_x*3)] | (m_bitblt.pattern[(m_bitblt.pat_y*24) + (m_bitblt.pat_x*3) + 1]) << 8
								| (m_bitblt.pattern[(m_bitblt.pat_y*24) + (m_bitblt.pat_x*3) + 2]) << 16;
						if (de)
							write_pixel24(dst_base, m_bitblt.x_current, m_bitblt.y_current, GetROP(rop, m_bitblt.current_pixel, dst, pat));
						m_bitblt.current_pixel = 0;
						done = advance_pixel();
						if((ids) && m_bitblt.x_current == m_bitblt.x_dst)
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
				src = read_pixel24(src_base, m_bitblt.x_src_current, m_bitblt.y_src_current, m_bitblt.src_stride);
				dst = read_pixel24(dst_base, m_bitblt.x_current, m_bitblt.y_current, m_bitblt.dest_stride);
				if(mp)
				{
					pat = (m_bitblt.mono_pattern & (1 << ((m_bitblt.pat_y*8) + (7-m_bitblt.pat_x))))
						? m_bitblt.pat_fg_clr : m_bitblt.pat_bg_clr;
				}
				else
					pat = m_bitblt.pattern[(m_bitblt.pat_y*24) + (m_bitblt.pat_x*3)] | (m_bitblt.pattern[(m_bitblt.pat_y*24) + (m_bitblt.pat_x*3) + 1]) << 8
						| (m_bitblt.pattern[(m_bitblt.pat_y*24) + (m_bitblt.pat_x*3) + 2]) << 16;
			}
			if (de)
				write_pixel24(dst_base, m_bitblt.x_current, m_bitblt.y_current,

			GetROP(rop, src, dst, pat));
			if(advance_pixel())
				command_finish();
			break;
	}

	m_bitblt.step_count++;
}

void s3virge_vga_device::bitblt_monosrc_step()
{
	// progress current monochrome source BitBLT operation
	uint32_t src_base = m_bitblt.src_base & 0x003ffff8;
	uint32_t dst_base = m_bitblt.dest_base & 0x003ffff8;

	const u32 current_command = m_bitblt.command;
	const uint8_t pixel_size = (current_command & 0x0000001c) >> 2;
	const uint8_t rop = (current_command & 0x01fe0000) >> 17;
	const bool tp = bool(BIT(current_command, 9));
	//const bool mp = bool(BIT(current_command, 8));
	const bool ids = bool(BIT(current_command, 7));
	const bool de = bool(BIT(current_command, 5));
	const int align = (current_command & 0x000000c00) >> 10;

	uint32_t src = 0;
	uint32_t dst = 0;
	// Windows 98 cares about this being initialized to non-zero for
	// greyed back/forward icons in Explorer, system icons and right click disabled Paste command.
	uint32_t pat = m_bitblt.pat_fg_clr;
	int x;
	bool done = false;

	switch(pixel_size)
	{
		case 0:  // 8bpp
			for(x=31;x>=0;x--)
			{
				if(ids)
					src = bitswap<32>(m_bitblt.image_xfer,7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8,23,22,21,20,19,18,17,16,31,30,29,28,27,26,25,24);
				else
					src = read_pixel8(src_base,m_bitblt.x_src_current,m_bitblt.y_src_current, m_bitblt.src_stride);
				dst = read_pixel8(dst_base,m_bitblt.x_current,m_bitblt.y_current, m_bitblt.dest_stride);

				if (de)
				{
					if(src & (1 << x))
						write_pixel8(dst_base,m_bitblt.x_current,m_bitblt.y_current,GetROP(rop, m_bitblt.src_fg_clr, dst, pat) & 0xff);
					else if(!tp)
					{
						write_pixel8(dst_base,m_bitblt.x_current,m_bitblt.y_current,GetROP(rop, m_bitblt.src_bg_clr, dst, pat) & 0xff);
					}
				}
				//printf("Pixel write(%i): X: %i Y: %i SRC: %04x DST: %04x PAT: %04x ROP: %02x\n",x,m_bitblt.x_current, m_bitblt.y_current, src, dst, pat, rop);
				done = advance_pixel();
				if((ids) && m_bitblt.x_current == m_bitblt.x_dst)
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
				if(ids)
					src = bitswap<32>(m_bitblt.image_xfer,7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8,23,22,21,20,19,18,17,16,31,30,29,28,27,26,25,24);
				else
					src = read_pixel16(src_base,m_bitblt.x_src_current,m_bitblt.y_src_current, m_bitblt.src_stride);
				dst = read_pixel16(dst_base,m_bitblt.x_current,m_bitblt.y_current, m_bitblt.dest_stride);

				if (de)
				{
					if(src & (1 << x))
						write_pixel16(dst_base,m_bitblt.x_current,m_bitblt.y_current,GetROP(rop, m_bitblt.src_fg_clr, dst, pat) & 0xffff);
					else if(!tp)
					{
						// only draw background colour if transparency is not set
						write_pixel16(dst_base,m_bitblt.x_current,m_bitblt.y_current,GetROP(rop, m_bitblt.src_bg_clr, dst, pat) & 0xffff);
					}
				}
				//printf("Pixel write(%i): X: %i Y: %i SRC: %04x DST: %04x PAT: %04x ROP: %02x\n",x,m_bitblt.x_current, m_bitblt.y_current, src, dst, pat, rop);
				done = advance_pixel();
				if((ids) && m_bitblt.x_current == m_bitblt.x_dst)
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
				if(ids)
					src = bitswap<32>(m_bitblt.image_xfer,7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8,23,22,21,20,19,18,17,16,31,30,29,28,27,26,25,24);
				else
					src = read_pixel24(src_base,m_bitblt.x_src_current,m_bitblt.y_src_current, m_bitblt.src_stride);
				dst = read_pixel24(dst_base,m_bitblt.x_current,m_bitblt.y_current, m_bitblt.dest_stride);

				if (de)
				{
					if(src & (1 << x))
						write_pixel24(dst_base,m_bitblt.x_current,m_bitblt.y_current,GetROP(rop, m_bitblt.src_fg_clr, dst, pat));
					else if(!tp)
					{
						// only draw background colour if transparency is not set
						// TODO: shouldn't be supported by 24bpp?
						write_pixel24(dst_base,m_bitblt.x_current,m_bitblt.y_current,GetROP(rop, m_bitblt.src_bg_clr, dst, pat));
					}
				}
				//printf("Pixel write(%i): X: %i Y: %i SRC: %04x DST: %04x PAT: %04x ROP: %02x\n",x,m_bitblt.x_current, m_bitblt.y_current, src, dst, pat, rop);
				done = advance_pixel();
				if((ids) && m_bitblt.x_current == m_bitblt.x_dst)
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

	m_bitblt.step_count++;
}

inline void s3virge_vga_device::write_pixel24(uint32_t base, uint16_t x, uint16_t y, uint32_t val)
{
	if(BIT(m_bitblt.command, 1))
	{
		if(x < m_bitblt.clip_l || x > m_bitblt.clip_r || y < m_bitblt.clip_t || y > m_bitblt.clip_b)
			return;
	}

	for (int i = 0; i < 3; i ++)
	{
		const u8 data = (val >> (8 * i)) & 0xff;
		vga.memory[(base + i + (x * 3) + (y * m_bitblt.dest_stride)) % vga.svga_intf.vram_size] = data;
	}
}

inline void s3virge_vga_device::write_pixel16(uint32_t base, uint16_t x, uint16_t y, uint16_t val)
{
	if(BIT(m_bitblt.command, 1))
	{
		if(x < m_bitblt.clip_l || x > m_bitblt.clip_r || y < m_bitblt.clip_t || y > m_bitblt.clip_b)
			return;
	}

	for (int i = 0; i < 2; i ++)
	{
		const u8 data = (val >> (8 * i)) & 0xff;
		vga.memory[(base + i + (x * 2) + (y * m_bitblt.dest_stride)) % vga.svga_intf.vram_size] = data;
	}
}

inline void s3virge_vga_device::write_pixel8(uint32_t base, uint16_t x, uint16_t y, uint8_t val)
{
	if(BIT(m_bitblt.command, 1))
	{
		if(x < m_bitblt.clip_l || x > m_bitblt.clip_r || y < m_bitblt.clip_t || y > m_bitblt.clip_b)
			return;
	}
	vga.memory[(base + x + (y * m_bitblt.dest_stride)) % vga.svga_intf.vram_size] = val;
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

// MM8504
uint32_t s3virge_vga_device::s3d_sub_status_r()
{
	uint32_t res = 0x00000000;

	// check for idle
	res |= (m_s3d_state == S3D_STATE_IDLE) << 13;
	//if (m_s3d_state == S3D_STATE_BITBLT && m_bitblt.xfer_mode == true && m_xfer_fifo.empty())
	//  res |= 1 << 13;

	//res |= (m_bitblt.cmd_fifo_slots_free << 8);
	// NOTE: can actually be 24 FIFO depth with specific Scenic Mode
	// (looks different FIFO altogether)
	// & 0x1f00
	//res |= std::min(m_bitblt_fifo.queue_length(), 16) << 8;
	// TODO: this likely listens for xfer FIFO, not command
	res |= (16 - (m_bitblt_fifo.queue_length() / 15)) << 8;

	return res;
}

/*
 * xx-- ---- ---- ---- S3d Engine Software Reset
 * 00-- ---- ---- ---- |- No change
 * 01-- ---- ---- ---- |- S3d Engine enabled
 * 10-- ---- ---- ---- |- Reset (alias of CR66 bit 1 = 1)
 * 11-- ---- ---- ---- |- <Reserved>
 * --x- ---- ---- ---- 3DF ENB S3d FIFO Empty IRQ Enable
 * ---x ---- ---- ---- CDD ENB Command DMA Done IRQ Enable
 * ---- x--- ---- ---- FIFO ENB EMP Command FIFO Empty IRQ Enable
 * ---- -x-- ---- ---- FIFO ENB OVF Command FIFO Overflow IRQ Enable
 * ---- --x- ---- ---- 3DD ENB S3d Engine Done IRQ Enable
 * ---- ---x ---- ---- VSY ENB Vertical Sync IRQ Enable
 * ---- ---- x--- ---- HDD ENB Host DMA Done IRQ Enable
 * <notice slightly different arrangement than above>
 * ---- ---- -1-- ---- 3DF CLR S3d FIFO Empty IRQ Acknowledge
 * ---- ---- --1- ---- CDD CLR Command DMA Done IRQ Acknowledge
 * ---- ---- ---1 ---- HDD CLR Host DMA Done IRQ Acknowledge
 * ---- ---- ---- 1--- FIFO CLE Command FIFO Empty IRQ Acknowledge
 * ---- ---- ---- -1-- FIFO CLO Command FIFO Overflow IRQ Acknowledge
 * ---- ---- ---- --1- 3DD CLR S3d Engine DOne IRQ Acknowledge
 * ---- ---- ---- ---1 VSY CLR Vertical Sync IRQ Acknowledge
 */
void s3virge_vga_device::s3d_sub_control_w(uint32_t data)
{
	LOGMMIO("Sub control = %08x\n", data);
	const u8 s3d_rst = (data >> 14) & 3;
	switch(s3d_rst)
	{
		// NOP
		case 0: break;
		// TODO: case 1 S3d Engine enabled
		case 2: s3d_reset(); break;
		default:
		case 3:
			// happens in BeOS 4.0 already, alias for a reset?
			LOG("S3D RST <reserved> state set\n");
			break;
	}

	m_interrupt_enable = data & 0x00003f80;
	if (m_interrupt_enable)
		popmessage("s3virge.cpp: IRQ enable warning %08x", m_interrupt_enable);
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

//  ret |= (s3d_fifo_size << 6);
	ret |= 0xf << 6;
	return ret;
}

// base 0xa000
void s3virge_vga_device::s3d_register_map(address_map &map)
{
	map(0x0100, 0x01bf).lrw8(
		NAME([this] (offs_t offset) {
			return m_bitblt.pattern[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_bitblt.pattern[offset] = data;
		})
	);
	map(0x04d4, 0x050f).lrw32(
		NAME([this] (offs_t offset) {
			return m_bitblt_latch[offset];
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			const u32 native_offset = (offset * 4) + 0xa4d4;
			LOGMMIO("MM%04X = %08x & %08x\n", native_offset, data, mem_mask);
			COMBINE_DATA(&m_bitblt_latch[offset]);
			if (native_offset == 0xa500 && !(BIT(data, 0)))
				add_command(0);
			if (native_offset == 0xa50c && BIT(m_bitblt_latch[11], 0))
				add_command(0);
		})
	);
}
