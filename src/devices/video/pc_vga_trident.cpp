// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * trident.cpp
 *
 * Implementation of Trident VGA GUI accelerators
 *
 * TODO:
 * - TVGA8200LX (just bog standard VGA?)
 * - TVGA8800 early SVGA
 * - TVGA8900 (2MB VRAM)
 * - TVGA9000 needs to be downgraded from '9680
 * \- none of the SVGA modes works properly;
 * \- subclassed from TGUI9680 just for pntnpuzl, consider swapping inheritance or even decouple;
 * \- it's also really a downgraded version of '8900
 * - TVGA92xx, TVGA938x (2d accelerator)
 * - TVGA94xx (PCI version of above)
 * - TGUI9680 is a PCI SVGA
 * \- several missing features (namely YUV-to-RGB conversions)
 * - ProVidia 968x ('9680 + TV video out & AD724 for NTSC/PAL conversion)
 * - AGP cards (3DImage, Blade3D, XP series)
 *
 */

#include "emu.h"
#include "pc_vga_trident.h"

#include "screen.h"

#define LOG_WARN  (1U << 1)
#define LOG_TODO  (1U << 2)
#define LOG_ACCEL (1U << 3)
#define LOG_CRTC  (1U << 4)

#define VERBOSE (LOG_GENERAL | LOG_WARN | LOG_ACCEL | LOG_TODO | LOG_CRTC)

#include "logmacro.h"

#define LOGWARN(...)      LOGMASKED(LOG_WARN,   __VA_ARGS__)
#define LOGACCEL(...)     LOGMASKED(LOG_ACCEL,  __VA_ARGS__)
#define LOGTODO(...)      LOGMASKED(LOG_TODO,   __VA_ARGS__)
#define LOGCRTC(...)      LOGMASKED(LOG_CRTC,   __VA_ARGS__)

DEFINE_DEVICE_TYPE(TGUI9680_VGA, tgui9680_device, "tgui9680_vga", "Trident TGUI9860 VGA i/f")
DEFINE_DEVICE_TYPE(TVGA9000_VGA, tvga9000_device, "tvga9000_vga", "Trident TVGA9000 VGA i/f")

trident_vga_device::trident_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: svga_device(mconfig, type, tag, owner, clock)
{
	m_main_if_space_config = address_space_config("io_regs", ENDIANNESS_LITTLE, 8, 4, 0, address_map_constructor(FUNC(trident_vga_device::io_3bx_3dx_map), this));
	m_crtc_space_config = address_space_config("crtc_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(trident_vga_device::crtc_map), this));
	m_gc_space_config = address_space_config("gc_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(trident_vga_device::gc_map), this));
	m_seq_space_config = address_space_config("sequencer_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(trident_vga_device::sequencer_map), this));
}

tgui9680_device::tgui9680_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: trident_vga_device(mconfig, TGUI9680_VGA, tag, owner, clock)
{
	m_version = 0xd3;   // 0xd3 identifies at TGUI9660XGi (set to 0xe3 to identify at TGUI9440AGi)
}

tvga9000_device::tvga9000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: trident_vga_device(mconfig, TVGA9000_VGA, tag, owner, clock)
{
	m_version = 0x43;
}

void trident_vga_device::io_3cx_map(address_map &map)
{
	svga_device::io_3cx_map(map);
	map(0x06, 0x06).rw(FUNC(trident_vga_device::ramdac_hidden_mask_r), FUNC(trident_vga_device::ramdac_hidden_mask_w));
	map(0x07, 0x09).rw(FUNC(trident_vga_device::ramdac_overlay_r), FUNC(trident_vga_device::ramdac_overlay_w));
}

u8 trident_vga_device::ramdac_hidden_mask_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		tri.dac_count++;
	if(tri.dac_count > 3)
		tri.dac_active = true;
	if(tri.dac_active)
		return tri.dac;

	return vga_device::ramdac_mask_r(offset);
}

void trident_vga_device::ramdac_hidden_mask_w(offs_t offset, u8 data)
{
	if(tri.dac_active)
	{
		tri.dac = data;  // DAC command register
		tri.dac_active = false;
		tri.dac_count = 0;
		recompute_params();
		return;
	}

	vga_device::ramdac_mask_w(offset, data);
}

u8 trident_vga_device::ramdac_overlay_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		tri.dac_active = false;
		tri.dac_count = 0;
	}

	switch(offset)
	{
		case 0:
			return vga_device::ramdac_state_r(0);
		case 1:
			return vga_device::ramdac_write_index_r(0);
		case 2:
			return vga_device::ramdac_data_r(0);
	}

	return space().unmap();
}

void trident_vga_device::ramdac_overlay_w(offs_t offset, u8 data)
{
	tri.dac_active = false;
	tri.dac_count = 0;

	switch(offset)
	{
		case 0:
			vga_device::ramdac_read_index_w(0, data);
			break;
		case 1:
			vga_device::ramdac_write_index_w(0, data);
			break;
		case 2:
			vga_device::ramdac_data_w(0, data);
			break;
	}
}

void trident_vga_device::io_3bx_3dx_map(address_map &map)
{
	svga_device::io_3bx_3dx_map(map);
	map(0x08, 0x08).rw(FUNC(trident_vga_device::svga_bank_write_r), FUNC(trident_vga_device::svga_bank_write_w));
	map(0x09, 0x09).rw(FUNC(trident_vga_device::svga_bank_read_r), FUNC(trident_vga_device::svga_bank_read_w));
	// no info on this port
	// Bit 5 appears to be a clock divider
	map(0x0b, 0x0b).lrw8(
		NAME([this] (offs_t offset) {
			return tri.port_3db;
		}),
		NAME([this] (offs_t offset, u8 data) {
			tri.port_3db = data;
			recompute_params();
		})
	);
}

u8 trident_vga_device::svga_bank_write_r(offs_t offset)
{
	// if enabled
	if(tri.gc0f & 0x04)
		return svga.bank_w & 0x3f;

	return space().unmap();
}

void trident_vga_device::svga_bank_write_w(offs_t offset, u8 data)
{
	// if enabled
	if(tri.gc0f & 0x04)
	{
		svga.bank_w = data & 0x3f;
		LOG("Trident: Write Bank set to %02x\n",data);
		// if bank regs are not separated ...
		if(!(tri.gc0f & 0x01))
		{
			// ... then this is also the read bank register
			svga.bank_r = data & 0x3f;
			LOG("Trident: Read Bank set to %02x\n",data);
		}
	}
}

u8 trident_vga_device::svga_bank_read_r(offs_t offset)
{
	// if enabled & if bank regs are separated
	if(tri.gc0f & 0x04)
		return svga.bank_r & 0x3f;

	return space().unmap();
}

void trident_vga_device::svga_bank_read_w(offs_t offset, u8 data)
{
	// if enabled & if bank regs are separated
	if(tri.gc0f & 0x04 && tri.gc0f & 0x01)
	{
		svga.bank_r = data & 0x3f;
		LOG("Trident: Read Bank set to %02x\n",data);
	}
}

void trident_vga_device::crtc_map(address_map &map)
{
	svga_device::crtc_map(map);
	// Module Testing Register
	map(0x1e, 0x1e).lrw8(
		NAME([this] (offs_t offset) {
			return tri.cr1e;
		}),
		NAME([this] (offs_t offset, u8 data) {
			tri.cr1e = data;
			vga.crtc.start_addr = (vga.crtc.start_addr & 0xfffeffff) | ((data & 0x20)<<11);
			recompute_params();
		})
	);
	// "Software Programming Register"  written to by the BIOS
	map(0x1f, 0x1f).lrw8(
		NAME([this] (offs_t offset) {
			return tri.cr1f;
		}),
		NAME([this] (offs_t offset, u8 data) {
			tri.cr1f = data;
		})
	);
	// FIFO Control (old MMIO enable? no documentation of this register)
	map(0x20, 0x20).lrw8(
		NAME([this] (offs_t offset) {
			return tri.cr20;
		}),
		NAME([this] (offs_t offset, u8 data) {
			tri.cr20 = data;
		})
	);
	// Linear aperture
	map(0x21, 0x21).lrw8(
		NAME([this] (offs_t offset) {
			return tri.cr21;
		}),
		NAME([this] (offs_t offset, u8 data) {
			tri.cr21 = data;
			tri.linear_address = ((data & 0xc0)<<18) | ((data & 0x0f)<<20);
			tri.linear_active = data & 0x20;
			if(tri.linear_active)
				popmessage("Trident: Linear Aperture active - %08x, %s",tri.linear_address,(tri.cr21 & 0x10) ? "2MB" : "1MB" );
		})
	);
	map(0x27, 0x27).lrw8(
		NAME([this] (offs_t offset) {
			return (vga.crtc.start_addr & 0x60000) >> 17;
		}),
		NAME([this] (offs_t offset, u8 data) {
			vga.crtc.start_addr = (vga.crtc.start_addr & 0xfff9ffff) | ((data & 0x03)<<17);
		})
	);
	map(0x29, 0x29).lrw8(
		NAME([this] (offs_t offset) {
			return tri.cr29;
		}),
		NAME([this] (offs_t offset, u8 data) {
			tri.cr29 = data;
			vga.crtc.offset = (vga.crtc.offset & 0xfeff) | ((data & 0x10)<<4);
		})
	);
	map(0x2a, 0x2a).lrw8(
		NAME([this] (offs_t offset) {
			return tri.cr2a;
		}),
		NAME([this] (offs_t offset, u8 data) {
			tri.cr2a = data;
		})
	);
	map(0x38, 0x38).lrw8(
		NAME([this] (offs_t offset) {
			return tri.pixel_depth;
		}),
		NAME([this] (offs_t offset, u8 data) {
			// bit 0: 16 bit bus
			// bits 2-3: pixel depth (1=15/16bit, 2=24/32bit, 0=anything else)
			// bit 5: packed mode
			tri.pixel_depth = data;
			recompute_params();
		})
	);
	map(0x39, 0x39).lrw8(
		NAME([this] (offs_t offset) {
			return tri.cr39;
		}),
		NAME([this] (offs_t offset, u8 data) {
			tri.cr39 = data;
			tri.mmio_active = data & 0x01;
			if(tri.mmio_active)
				popmessage("Trident: MMIO activated");
		})
	);
	map(0x40, 0x40).lrw8(
		NAME([this] (offs_t offset) {
			return (tri.cursor_x & 0x00ff);
		}),
		NAME([this] (offs_t offset, u8 data) {
			tri.cursor_x = (tri.cursor_x & 0xff00) | data;
		})
	);
	map(0x41, 0x41).lrw8(
		NAME([this] (offs_t offset) {
			return (tri.cursor_x & 0xff00) >> 8;
		}),
		NAME([this] (offs_t offset, u8 data) {
			tri.cursor_x = (tri.cursor_x & 0x00ff) | (data << 8);
		})
	);
	map(0x42, 0x42).lrw8(
		NAME([this] (offs_t offset) {
			return (tri.cursor_y & 0x00ff);
		}),
		NAME([this] (offs_t offset, u8 data) {
			tri.cursor_y = (tri.cursor_y & 0xff00) | data;
		})
	);
	map(0x43, 0x43).lrw8(
		NAME([this] (offs_t offset) {
			return (tri.cursor_y & 0xff00) >> 8;
		}),
		NAME([this] (offs_t offset, u8 data) {
			tri.cursor_y = (tri.cursor_y & 0x00ff) | (data << 8);
		})
	);
	map(0x44, 0x44).lrw8(
		NAME([this] (offs_t offset) {
			return (tri.cursor_loc & 0x00ff);
		}),
		NAME([this] (offs_t offset, u8 data) {
			tri.cursor_loc = (tri.cursor_loc & 0xff00) | data;
		})
	);
	map(0x45, 0x45).lrw8(
		NAME([this] (offs_t offset) {
			return (tri.cursor_loc & 0xff00) >> 8;
		}),
		NAME([this] (offs_t offset, u8 data) {
			tri.cursor_loc = (tri.cursor_loc & 0x00ff) | (data << 8);
		})
	);
	map(0x46, 0x46).lrw8(
		NAME([this] (offs_t offset) {
			return tri.cursor_x_off;
		}),
		NAME([this] (offs_t offset, u8 data) {
			tri.cursor_x_off = data;
		})
	);
	map(0x47, 0x47).lrw8(
		NAME([this] (offs_t offset) {
			return tri.cursor_y_off;
		}),
		NAME([this] (offs_t offset, u8 data) {
			tri.cursor_y_off = data;
		})
	);
	map(0x48, 0x48).lrw8(
		NAME([this] (offs_t offset) {
			return (tri.cursor_fg & 0x000000ff);
		}),
		NAME([this] (offs_t offset, u8 data) {
			tri.cursor_fg = (tri.cursor_fg & 0xffffff00) | data;
		})
	);
	map(0x49, 0x49).lrw8(
		NAME([this] (offs_t offset) {
			return (tri.cursor_fg & 0x0000ff00) >> 8;
		}),
		NAME([this] (offs_t offset, u8 data) {
			tri.cursor_fg = (tri.cursor_fg & 0xffff00ff) | (data << 8);
		})
	);
	map(0x4a, 0x4a).lrw8(
		NAME([this] (offs_t offset) {
			return (tri.cursor_fg & 0x00ff0000) >> 16;
		}),
		NAME([this] (offs_t offset, u8 data) {
			tri.cursor_fg = (tri.cursor_fg & 0xff00ffff) | (data << 16);
		})
	);
	map(0x4b, 0x4b).lrw8(
		NAME([this] (offs_t offset) {
			return (tri.cursor_fg & 0xff000000) >> 24;
		}),
		NAME([this] (offs_t offset, u8 data) {
			tri.cursor_fg = (tri.cursor_fg & 0x00ffffff) | (data << 24);
		})
	);
	map(0x4c, 0x4c).lrw8(
		NAME([this] (offs_t offset) {
			return (tri.cursor_bg & 0x000000ff);
		}),
		NAME([this] (offs_t offset, u8 data) {
			tri.cursor_bg = (tri.cursor_bg & 0xffffff00) | data;
		})
	);
	map(0x4d, 0x4d).lrw8(
		NAME([this] (offs_t offset) {
			return (tri.cursor_bg & 0x0000ff00) >> 8;
		}),
		NAME([this] (offs_t offset, u8 data) {
			tri.cursor_bg = (tri.cursor_bg & 0xffff00ff) | (data << 8);
		})
	);
	map(0x4e, 0x4e).lrw8(
		NAME([this] (offs_t offset) {
			return (tri.cursor_bg & 0x00ff0000) >> 16;
		}),
		NAME([this] (offs_t offset, u8 data) {
			tri.cursor_bg = (tri.cursor_bg & 0xff00ffff) | (data << 16);
		})
	);
	map(0x4f, 0x4f).lrw8(
		NAME([this] (offs_t offset) {
			return (tri.cursor_bg & 0xff000000) >> 24;
		}),
		NAME([this] (offs_t offset, u8 data) {
			tri.cursor_bg = (tri.cursor_bg & 0x00ffffff) | (data << 24);
		})
	);
	map(0x50, 0x50).lrw8(
		NAME([this] (offs_t offset) {
			return tri.cursor_ctrl;
		}),
		NAME([this] (offs_t offset, u8 data) {
			tri.cursor_ctrl = data;
		})
	);
}

void trident_vga_device::gc_map(address_map &map)
{
	svga_device::gc_map(map);
	// New Source Address Register (bit 1 is inverted here, also)
	map(0x0e, 0x0e).lrw8(
		NAME([this] (offs_t offset) {
			return tri.gc0e;
		}),
		NAME([this] (offs_t offset, u8 data) {
			tri.gc0e = data ^ 0x02;
			if(!(tri.gc0f & 0x04))  // if bank regs at 0x3d8/9 are not enabled
			{
				if(tri.gc0f & 0x01)  // if bank regs are separated
					svga.bank_r = (data & 0x1f) ^ 0x02;
			}
		})
	);
	map(0x0f, 0x0f).lrw8(
		NAME([this] (offs_t offset) {
			return tri.gc0f;
		}),
		NAME([this] (offs_t offset, u8 data) {
			tri.gc0f = data;
			recompute_params();
		})
	);
	// XFree86 refers to this register as "MiscIntContReg", setting bit 2, but gives no indication as to what it does
	map(0x2f, 0x2f).lrw8(
		NAME([this] (offs_t offset) {
			return tri.gc2f;
		}),
		NAME([this] (offs_t offset, u8 data) {
			tri.gc2f = data;
		})
	);
}

void trident_vga_device::sequencer_map(address_map &map)
{
	svga_device::sequencer_map(map);
	map(0x09, 0x09).lr8(
		NAME([this] (offs_t offset) {
			return tri.revision;
		})
	);
	map(0x0b, 0x0b).lrw8(
		NAME([this] (offs_t offset) {
			if (!machine().side_effects_disabled())
				tri.new_mode = true;
			return svga.id;
		}),
		NAME([this] (offs_t offset, u8 data) {
			tri.new_mode = false;
		})
	);
	// Power Up Mode register 1
	map(0x0c, 0x0c).lrw8(
		NAME([this] (offs_t offset) {
			u8 res = tri.sr0c & 0xef;
			if(tri.port_3c3)
				res |= 0x10;
			return res;
		}),
		NAME([this] (offs_t offset, u8 data) {
			// ---1 ---- 'post port at 0x3c3'
			// ---0 ---- 'post port at 0x46e8'
			tri.port_3c3 = bool(BIT(data, 4));
			tri.sr0c = data;
		})
	);
	// Mode Control 2
	map(0x0d, 0x0d).lrw8(
		NAME([this] (offs_t offset) {
			return (tri.new_mode) ? tri.sr0d_new : tri.sr0d_old;
		}),
		NAME([this] (offs_t offset, u8 data) {
			if(tri.new_mode)
			{
				tri.sr0d_new = data;
				tri.clock = ((vga.miscellaneous_output & 0x0c) >> 2) | ((data & 0x01) << 2) | ((data & 0x40) >> 3);
				recompute_params();
			}
			else
				tri.sr0d_old = data;
		})
	);
	// Mode Control 1
	map(0x0e, 0x0e).lrw8(
		NAME([this] (offs_t offset) {
			return (tri.new_mode) ? tri.sr0e_new : tri.sr0e_old;
		}),
		NAME([this] (offs_t offset, u8 data) {
			if(tri.new_mode)
			{
				tri.sr0e_new = data ^ 0x02;
				svga.bank_w = (data & 0x3f) ^ 0x02;  // bit 1 is inverted, used for card detection, it is not XORed on reading
				if(!(tri.gc0f & 0x01))
					svga.bank_r = (data & 0x3f) ^ 0x02;
				// TODO: handle planar modes, where bits 0 and 2 only are used
			}
			else
			{
				tri.sr0e_old = data;
				svga.bank_w = data & 0x0e;
				if(!(tri.gc0f & 0x01))
					svga.bank_r = data & 0x0e;
			}
		})
	);
	// Power Up Mode 2
	map(0x0f, 0x0f).lrw8(
		NAME([this] (offs_t offset) {
			return tri.sr0f;
		}),
		NAME([this] (offs_t offset, u8 data) {
			tri.sr0f = data;
		})
	);
}

void trident_vga_device::device_start()
{
	zero();

	for (int i = 0; i < 0x100; i++)
		set_pen_color(i, 0, 0, 0);

	// Avoid an infinite loop when displaying.  0 is not possible anyway.
	vga.crtc.maximum_scan_line = 1;


	// copy over interfaces
	vga.memory = std::make_unique<uint8_t []>(vga.svga_intf.vram_size);
	memset(&vga.memory[0], 0, vga.svga_intf.vram_size);

	save_pointer(NAME(vga.memory), vga.svga_intf.vram_size);
	save_pointer(vga.crtc.data,"CRTC Registers",0x100);
	save_pointer(vga.sequencer.data,"Sequencer Registers",0x100);
	save_pointer(vga.attribute.data,"Attribute Registers", 0x15);
	save_pointer(tri.accel_pattern,"Pattern Data", 0x80);
	save_pointer(tri.lutdac_reg,"LUTDAC registers", 0x100);

	m_vblank_timer = timer_alloc(FUNC(trident_vga_device::vblank_timer_cb), this);
	svga.ignore_chain4 = true;
	memset(&tri, 0, sizeof(tri));
}

void trident_vga_device::device_reset()
{
	svga_device::device_reset();
	svga.id = m_version;
	tri.revision = 0x01;  // revision identifies as TGUI9680
	tri.new_mode = false;  // start up in old mode
	tri.dac_active = false;
	tri.linear_active = false;
	tri.mmio_active = false;
	tri.sr0f = 0x6f;
	tri.sr0c = 0x70;
	tri.cr2a = 0x03;  // set ISA interface?
	tri.mem_clock = 0x2c6;  // 50MHz default
	tri.vid_clock = 0;
	tri.port_3c3 = true;
	tri.accel_busy = false;
	tri.accel_memwrite_active = false;
	// Windows 3.1 TGUI9440AGi drivers do not set the pointer colour registers?
	tri.cursor_bg = 0x00000000;
	tri.cursor_fg = 0xffffffff;
	tri.pixel_depth = 0x10;  //disable 8bpp mode by default
}

uint8_t trident_vga_device::READPIXEL8(int16_t x, int16_t y)
{
	return (vga.memory[((y & 0xfff)*offset() + (x & 0xfff)) % vga.svga_intf.vram_size]);
}

uint16_t trident_vga_device::READPIXEL15(int16_t x, int16_t y)
{
	return (vga.memory[((y & 0xfff)*offset() + (x & 0xfff)*2) % vga.svga_intf.vram_size] |
			(vga.memory[((y & 0xfff)*offset() + ((x & 0xfff)*2)+1) % vga.svga_intf.vram_size] << 8));
}

uint16_t trident_vga_device::READPIXEL16(int16_t x, int16_t y)
{
	return (vga.memory[((y & 0xfff)*offset() + (x & 0xfff)*2) % vga.svga_intf.vram_size] |
			(vga.memory[((y & 0xfff)*offset() + ((x & 0xfff)*2)+1) % vga.svga_intf.vram_size] << 8));
}

uint32_t trident_vga_device::READPIXEL32(int16_t x, int16_t y)
{
	return (vga.memory[((y & 0xfff)*offset() + (x & 0xfff)*4) % vga.svga_intf.vram_size] |
			(vga.memory[((y & 0xfff)*offset() + ((x & 0xfff)*4)+1) % vga.svga_intf.vram_size] << 8) |
			(vga.memory[((y & 0xfff)*offset() + ((x & 0xfff)*4)+2) % vga.svga_intf.vram_size] << 16) |
			(vga.memory[((y & 0xfff)*offset() + ((x & 0xfff)*4)+3) % vga.svga_intf.vram_size] << 24));
}

void trident_vga_device::WRITEPIXEL8(int16_t x, int16_t y, uint8_t data)
{
	if((x & 0xfff)<tri.accel_dest_x_clip && (y & 0xfff)<tri.accel_dest_y_clip)
	{
		data = handle_rop(data,READPIXEL8(x,y)) & 0xff;
		vga.memory[((y & 0xfff)*offset() + (x & 0xfff)) % vga.svga_intf.vram_size] = data;
	}
}

void trident_vga_device::WRITEPIXEL15(int16_t x, int16_t y, uint16_t data)
{
	if((x & 0xfff)<tri.accel_dest_x_clip && (y & 0xfff)<tri.accel_dest_y_clip)
	{
		data = handle_rop(data,READPIXEL8(x,y)) & 0x7fff;
		vga.memory[((y & 0xfff)*offset() + (x & 0xfff)*2) % vga.svga_intf.vram_size] = data & 0x00ff;
		vga.memory[((y & 0xfff)*offset() + ((x & 0xfff)*2)+1) % vga.svga_intf.vram_size] = (data & 0x7f00) >> 8;
	}
}

void trident_vga_device::WRITEPIXEL16(int16_t x, int16_t y, uint16_t data)
{
	if((x & 0xfff)<tri.accel_dest_x_clip && (y & 0xfff)<tri.accel_dest_y_clip)
	{
		data = handle_rop(data,READPIXEL8(x,y)) & 0xffff;
		vga.memory[((y & 0xfff)*offset() + (x & 0xfff)*2) % vga.svga_intf.vram_size] = data & 0x00ff;
		vga.memory[((y & 0xfff)*offset() + ((x & 0xfff)*2)+1) % vga.svga_intf.vram_size] = (data & 0xff00) >> 8;
	}
}

void trident_vga_device::WRITEPIXEL32(int16_t x, int16_t y, uint32_t data)
{
	if((x & 0xfff)<tri.accel_dest_x_clip && (y & 0xfff)<tri.accel_dest_y_clip)
	{
		data = handle_rop(data,READPIXEL8(x,y));
		vga.memory[((y & 0xfff)*offset() + (x & 0xfff)*4) % vga.svga_intf.vram_size] = data & 0x000000ff;
		vga.memory[((y & 0xfff)*offset() + ((x & 0xfff)*4)+1) % vga.svga_intf.vram_size] = (data & 0x0000ff00) >> 8;
		vga.memory[((y & 0xfff)*offset() + ((x & 0xfff)*4)+2) % vga.svga_intf.vram_size] = (data & 0x00ff0000) >> 16;
		vga.memory[((y & 0xfff)*offset() + ((x & 0xfff)*4)+3) % vga.svga_intf.vram_size] = (data & 0xff000000) >> 24;
	}
}

uint32_t trident_vga_device::handle_rop(uint32_t src, uint32_t dst)
{
	switch(tri.accel_fmix)  // TODO: better understand this register
	{
	case 0xf0:  // PAT
	case 0xcc:  // SRC
		break;  // pass data through
	case 0x00:  // 0
		src = 0;
		break;
	case 0xff:  // 1
		src = 0xffffffff;
		break;
	case 0x66:  // XOR
	case 0x5a:  // XOR PAT
		src = dst ^ src;
		break;
	case 0xb8:  // PAT xor (SRC and (DST xor PAT)) (correct?)
		src = src & (dst ^ src);
		break;
	}
	return src;
}

uint32_t trident_vga_device::READPIXEL(int16_t x,int16_t y)
{
	if(svga.rgb8_en)
		return READPIXEL8(x,y) & 0xff;
	if(svga.rgb15_en)
		return READPIXEL15(x,y) & 0x7fff;
	if(svga.rgb16_en)
		return READPIXEL16(x,y) & 0xffff;
	if(svga.rgb32_en)
		return READPIXEL32(x,y);
	return 0;  // should never reach here
}

void trident_vga_device::WRITEPIXEL(int16_t x,int16_t y, uint32_t data)
{
	if(svga.rgb8_en)
		WRITEPIXEL8(x,y,(((data >> 8) & 0xff) | (data & 0xff)));  // XFree86 3.3 sets bits 0-7 to 0 when using mono patterns, does it OR each byte?
	if(svga.rgb15_en)
		WRITEPIXEL15(x,y,data & 0x7fff);
	if(svga.rgb16_en)
		WRITEPIXEL16(x,y,data & 0xffff);
	if(svga.rgb32_en)
		WRITEPIXEL32(x,y,data);
}

uint32_t trident_vga_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	svga_device::screen_update(screen,bitmap,cliprect);
	uint8_t const cur_mode = pc_vga_choosevideomode();

	// draw hardware graphics cursor
	if(tri.cursor_ctrl & 0x80)  // if cursor is enabled
	{
		uint16_t const cx = tri.cursor_x & 0x0fff;
		uint16_t const cy = tri.cursor_y & 0x0fff;
		uint8_t const cursor_size = (tri.cursor_ctrl & 0x01) ? 64 : 32;

		if(cur_mode == SCREEN_OFF || cur_mode == TEXT_MODE || cur_mode == MONO_MODE || cur_mode == CGA_MODE || cur_mode == EGA_MODE)
			return 0;  // cursor only works in VGA or SVGA modes

		uint32_t src = tri.cursor_loc * 1024;  // start address is in units of 1024 bytes

		uint32_t bg_col, fg_col;
		if(cur_mode == RGB16_MODE)
		{
			bg_col = tri.cursor_bg;
			fg_col = tri.cursor_fg;
		}
		else /* TODO: other modes */
		{
			bg_col = pen(tri.cursor_bg & 0xff);
			fg_col = pen(tri.cursor_fg & 0xff);
		}

		for(int y=0;y<cursor_size;y++)
		{
			uint8_t bitcount = 31;
			uint32_t *const dst = &bitmap.pix(cy + y, cx);
			for(int x=0;x<cursor_size;x++)
			{
				uint32_t bitb = (vga.memory[(src+3) % vga.svga_intf.vram_size]
							| ((vga.memory[(src+2) % vga.svga_intf.vram_size]) << 8)
							| ((vga.memory[(src+1) % vga.svga_intf.vram_size]) << 16)
							| ((vga.memory[(src+0) % vga.svga_intf.vram_size]) << 24));
				uint32_t bita = (vga.memory[(src+7) % vga.svga_intf.vram_size]
							| ((vga.memory[(src+6) % vga.svga_intf.vram_size]) << 8)
							| ((vga.memory[(src+5) % vga.svga_intf.vram_size]) << 16)
							| ((vga.memory[(src+4) % vga.svga_intf.vram_size]) << 24));
				uint8_t const val = (BIT(bita << 1,bitcount+1) << 1 | BIT(bitb,bitcount));
				if(tri.cursor_ctrl & 0x40)
				{  // X11 mode
					switch(val)
					{
					case 0x00:
						// no change
						break;
					case 0x01:
						dst[x] = bg_col;
						break;
					case 0x02:
						// no change
						break;
					case 0x03:
						dst[x] = fg_col;
						break;
					}
				}
				else
				{  // Windows mode
					switch(val)
					{
					case 0x00:
						dst[x] = bg_col;
						break;
					case 0x01:
						// no change
						break;
					case 0x02:  // screen data
						dst[x] = fg_col;
						break;
					case 0x03:  // inverted screen data
						dst[x] = ~(dst[x]);
						break;
					}
				}
				bitcount--;
				if(x % 32 == 31)
				{
					src+=8;
					bitcount=31;
				}
			}
		}
	}

	return 0;
}

uint16_t trident_vga_device::offset()
{
	// don't know if this is right, but Eggs Playing Chicken switches off doubleword mode, but expects the same offset length
	if (svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb32_en)
		return vga.crtc.offset << 3;
	return svga_device::offset();
}

int trident_vga_device::calculate_clock()
{
	// Bits 0-6: M
	// Bits 7-11: N
	// Bit 12: K
	// Later formula extends each variable by one extra bit (Providia 9685 and later)
	double freq;
	uint8_t m,n,k;

	m = tri.vid_clock & 0x007f;
	n = (tri.vid_clock & 0x0f80) >> 7;
	k = (tri.vid_clock & 0x1000) >> 12;
	freq = ((double)(m+8) / (double)((n+2)*(pow(2.0,k)))) * 14.31818; // there is a 14.31818MHz clock on the board

	return freq * 1000000;
}

void trident_vga_device::recompute_params()
{
	int divisor = 1;
	int xtal;

	/*  // clock select for TGUI9440CXi and earlier
	switch(tri.clock)
	{
	case 0:
	default: xtal = 25174800; break;
	case 1:  xtal = 28636363; break;
	case 2:  xtal = 44900000; break;
	case 3:  xtal = 36000000; break;
	case 4:  xtal = 57272000; break;
	case 5:  xtal = 65000000; break;
	case 6:  xtal = 50350000; break;
	case 7:  xtal = 40000000; break;
	case 8:  xtal = 88000000; break;
	case 9:  xtal = 98000000; break;
	case 10: xtal = 118800000; break;
	case 11: xtal = 108000000; break;
	case 12: xtal = 72000000; break;
	case 13: xtal = 77000000; break;
	case 14: xtal = 80000000; break;
	case 15: xtal = 75000000; break;
	}

	switch((tri.sr0d_new & 0x06) >> 1)
	{
	case 0:
	default:  break;  // no division
	case 1:   xtal = xtal / 2; break;
	case 2:   xtal = xtal / 4; break;
	case 3:   xtal = xtal / 1.5; break;
	}*/


	// TGUI9440AGi/9660/9680/9682 programmable clock
	switch((vga.miscellaneous_output & 0x0c) >> 2)
	{
	case 0:
	default: xtal = 25174800; break;
	case 1:  xtal = 28636363; break;
	case 2:  xtal = calculate_clock(); break;
	}

	if(tri.gc0f & 0x08)  // 16 pixels per character clock
		xtal = xtal / 2;

	if(tri.port_3db & 0x20)
		xtal = xtal / 2;  // correct?

	svga.rgb8_en = svga.rgb15_en = svga.rgb16_en = svga.rgb32_en = 0;
	switch((tri.pixel_depth & 0x0c) >> 2)
	{
	case 0:
	default: if(!(tri.pixel_depth & 0x10) || (tri.cr1e & 0x80)) svga.rgb8_en = 1; break;
	case 1:  if((tri.dac & 0xf0) == 0x30) svga.rgb16_en = 1; else svga.rgb15_en = 1; break;
	case 2:  svga.rgb32_en = 1; break;
	}

	if((tri.cr1e & 0x80) && (svga.id == 0x43))
		divisor = 2;

	recompute_params_clock(divisor, xtal);
}

uint8_t trident_vga_device::port_43c6_r(offs_t offset)
{
	uint8_t res = 0xff;
	switch(offset)
	{
	case 2:
		res = tri.mem_clock & 0xff;
		break;
	case 3:
		res = tri.mem_clock >> 8;
		break;
	case 4:
		res = tri.vid_clock & 0xff;
		break;
	case 5:
		res = tri.vid_clock >> 8;
		break;
	}
	return res;
}

void trident_vga_device::port_43c6_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
	case 2:
		if(!(tri.sr0e_new & 0x02) && (tri.sr0e_new & 0x80))
		{
			tri.mem_clock = (tri.mem_clock & 0xff00) | (data);
			LOG("Trident: Memory clock write %04x\n",tri.mem_clock);
		}
		break;
	case 3:
		if(!(tri.sr0e_new & 0x02) && (tri.sr0e_new & 0x80))
		{
			tri.mem_clock = (tri.mem_clock & 0x00ff) | (data << 8);
			LOG("Trident: Memory clock write %04x\n",tri.mem_clock);
		}
		break;
	case 4:
		if(!(tri.sr0e_new & 0x02) && (tri.sr0e_new & 0x80))
		{
			tri.vid_clock = (tri.vid_clock & 0xff00) | (data);
			LOG("Trident: Video clock write %04x\n",tri.vid_clock);
		}
		break;
	case 5:
		if(!(tri.sr0e_new & 0x02) && (tri.sr0e_new & 0x80))
		{
			tri.vid_clock = (tri.vid_clock & 0x00ff) | (data << 8);
			LOG("Trident: Video clock write %04x\n",tri.vid_clock);
		}
		break;
	}
}

// Trident refers to these registers as a LUTDAC
// Not much else is known.  XFree86 uses register 4 for something related to DPMS
uint8_t trident_vga_device::port_83c6_r(offs_t offset)
{
	uint8_t res = 0xff;
	switch(offset)
	{
	case 2:
		res = tri.lutdac_reg[tri.lutdac_index];
		LOG("Trident: LUTDAC reg read %02x\n",res);
		break;
	case 4:
		res = tri.lutdac_index;
		LOG("Trident: LUTDAC index read %02x\n",res);
		break;
	}
	return res;
}

void trident_vga_device::port_83c6_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
	case 2:
		LOG("Trident: LUTDAC reg write %02x\n",data);
		tri.lutdac_reg[tri.lutdac_index] = data;
		break;
	case 4:
		LOG("Trident: LUTDAC index write %02x\n",data);
		tri.lutdac_index = data;
		break;
	}
}

uint8_t trident_vga_device::vram_r(offs_t offset)
{
	if (tri.linear_active)
		return vga.memory[offset % vga.svga_intf.vram_size];
	else
		return 0xff;
}

void trident_vga_device::vram_w(offs_t offset, uint8_t data)
{
	if (tri.linear_active)
	{
		if(tri.accel_memwrite_active)
		{
			tri.accel_transfer = (tri.accel_transfer & (~(0x000000ff << (24-(8*(offset % 4)))))) | (data << (24-(8 * (offset % 4))));
			if(offset % 4 == 3)
				accel_data_write(tri.accel_transfer);
			return;
		}
		vga.memory[offset % vga.svga_intf.vram_size] = data;
	}
}

uint8_t trident_vga_device::mem_r(offs_t offset)
{
	if((tri.cr20 & 0x10) && (offset >= 0x1ff00)) // correct for old MMIO?
	{
		return old_mmio_r(offset-0x1ff00);
	}

	if (svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb32_en)
	{
		int data;

		if(tri.new_mode)  // 64k from 0xA0000-0xAFFFF
		{
			offset &= 0xffff;
			data=vga.memory[(offset + (svga.bank_r*0x10000)) % vga.svga_intf.vram_size];
		}
		else   // 128k from 0xA0000-0xBFFFF
		{
			data=vga.memory[(offset + (svga.bank_r*0x10000)) % vga.svga_intf.vram_size];
		}
		return data;
	}

	return vga_device::mem_r(offset);
}

void trident_vga_device::mem_w(offs_t offset, uint8_t data)
{
	if((tri.cr20 & 0x10) && (offset >= 0x1ff00)) // correct for old MMIO?
	{
		old_mmio_w(offset-0x1ff00,data);
		return;
	}

	if(tri.accel_memwrite_active)
	{
		tri.accel_transfer = (tri.accel_transfer & (~(0x000000ff << (24-(8*(offset % 4)))))) | (data << (24-(8 * (offset % 4))));
		if(offset % 4 == 3)
			accel_data_write(tri.accel_transfer);
		return;
	}

	if (svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb32_en)
	{
		if(tri.new_mode)  // 64k from 0xA0000-0xAFFFF
		{
			offset &= 0xffff;
			vga.memory[(offset + (svga.bank_w*0x10000)) % vga.svga_intf.vram_size] = data;
		}
		else   // 128k from 0xA0000-0xBFFFF
		{
			vga.memory[(offset + (svga.bank_w*0x10000)) % vga.svga_intf.vram_size] = data;
		}
		return;
	}

	vga_device::mem_w(offset,data);
}

// Old style MMIO (maps to 0xbff00)
void trident_vga_device::old_mmio_w(offs_t offset, uint8_t data)
{
	if(offset >= 0x20)
		accel_w(offset-0x20,data);
}

uint8_t trident_vga_device::old_mmio_r(offs_t offset)
{
	if(offset == 0x20)
	{
		if(tri.accel_busy)
			return 0x20;
	}
	if(offset > 0x20)
		return accel_r(offset-0x20);
	else
		return 0x00;
}


// 2D Acceleration functions (very WIP)

// From XFree86 source:
/*
Graphics Engine for 9440/9660/9680

#define GER_STATUS  0x2120
#define     GE_BUSY 0x80
#define GER_OPERMODE    0x2122       Byte for 9440, Word for 96xx
#define     DST_ENABLE  0x200   // Destination Transparency
#define GER_COMMAND 0x2124
#define     GE_NOP      0x00    // No Operation
#define     GE_BLT      0x01    // BitBLT ROP3 only
#define     GE_BLT_ROP4 0x02    // BitBLT ROP4 (96xx only)
#define     GE_SCANLINE 0x03    // Scan Line
#define     GE_BRESLINE 0x04    // Bresenham Line
#define     GE_SHVECTOR 0x05    // Short Vector
#define     GE_FASTLINE 0x06    // Fast Line (96xx only)
#define     GE_TRAPEZ   0x07    // Trapezoidal fill (96xx only)
#define     GE_ELLIPSE  0x08    // Ellipse (96xx only) (RES)
#define     GE_ELLIP_FILL   0x09    // Ellipse Fill (96xx only) (RES)
#define GER_FMIX    0x2127
#define GER_DRAWFLAG    0x2128      // long
#define     FASTMODE    1<<28
#define     STENCIL     0x8000
#define     SOLIDFILL   0x4000
#define     TRANS_ENABLE    0x1000
#define     TRANS_REVERSE   0x2000
#define     YMAJ        0x0400
#define     XNEG        0x0200
#define     YNEG        0x0100
#define     SRCMONO     0x0040
#define     PATMONO     0x0020
#define     SCR2SCR     0x0004
#define     PAT2SCR     0x0002
#define GER_FCOLOUR 0x212C      // Word for 9440, long for 96xx
#define GER_BCOLOUR 0x2130      // Word for 9440, long for 96xx
#define GER_PATLOC  0x2134      // Word
#define GER_DEST_XY 0x2138
#define GER_DEST_X  0x2138      // Word
#define GER_DEST_Y  0x213A      // Word
#define GER_SRC_XY  0x213C
#define GER_SRC_X   0x213C      // Word
#define GER_SRC_Y   0x213E      // Word
#define GER_DIM_XY  0x2140
#define GER_DIM_X   0x2140      // Word
#define GER_DIM_Y   0x2142      // Word
#define GER_STYLE   0x2144      // Long
#define GER_CKEY    0x2168      // Long
#define GER_FPATCOL 0x2178
#define GER_BPATCOL 0x217C
#define GER_PATTERN 0x2180      // from 0x2180 to 0x21FF

 Additional - Graphics Engine for 96xx
#define GER_SRCCLIP_XY  0x2148
#define GER_SRCCLIP_X   0x2148      // Word
#define GER_SRCCLIP_Y   0x214A      // Word
#define GER_DSTCLIP_XY  0x214C
#define GER_DSTCLIP_X   0x214C      // Word
#define GER_DSTCLIP_Y   0x214E      // Word
*/

uint8_t trident_vga_device::accel_r(offs_t offset)
{
	uint8_t res = 0xff;

	if(offset >= 0x60)
		return tri.accel_pattern[(offset-0x60) % 0x80];

	switch(offset)
	{
	case 0x00:  // Status
		if(tri.accel_busy)
			res = 0x80;
		else
			res = 0x00;
		break;
	// Operation mode:
	// bit 8: disable clipping if set
	case 0x02:  // Operation Mode
		res = tri.accel_opermode & 0x00ff;
		break;
	case 0x03:
		res = (tri.accel_opermode & 0xff00) >> 8;
		break;
	case 0x04:  // Command register
		res = tri.accel_command;
		break;
	case 0x07:  // Foreground Mix?
		res = tri.accel_fmix;
		break;
	default:
		LOGTODO("Trident: unimplemented acceleration register offset %02x read\n",offset);
	}
	return res;
}

void trident_vga_device::accel_w(offs_t offset, uint8_t data)
{
	if(offset >= 0x60)
	{
		tri.accel_pattern[(offset-0x60) % 0x80] = data;
		return;
	}

	switch(offset)
	{
	case 0x02:  // Operation Mode
		tri.accel_opermode = (tri.accel_opermode & 0xff00) | data;
		LOGACCEL("Trident: Operation Mode set to %04x\n",tri.accel_opermode);
		break;
	case 0x03:
		tri.accel_opermode = (tri.accel_opermode & 0x00ff) | (data << 8);
		LOGACCEL("Trident: Operation Mode set to %04x\n",tri.accel_opermode);
		break;
	case 0x04:  // Command register
		tri.accel_command = data;
		accel_command();
		break;
	case 0x07:  // Foreground Mix?
		tri.accel_fmix = data;
		LOGACCEL("Trident: FMIX set to %02x\n",data);
		break;
	case 0x08:  // Draw flags
		tri.accel_drawflags = (tri.accel_drawflags & 0xffffff00) | data;
		LOGACCEL("Trident: Draw flags set to %08x\n",tri.accel_drawflags);
		break;
	case 0x09:
		tri.accel_drawflags = (tri.accel_drawflags & 0xffff00ff) | (data << 8);
		LOGACCEL("Trident: Draw flags set to %08x\n",tri.accel_drawflags);
		break;
	case 0x0a:
		tri.accel_drawflags = (tri.accel_drawflags & 0xff00ffff) | (data << 16);
		LOGACCEL("Trident: Draw flags set to %08x\n",tri.accel_drawflags);
		break;
	case 0x0b:
		tri.accel_drawflags = (tri.accel_drawflags & 0x00ffffff) | (data << 24);
		LOGACCEL("Trident: Draw flags set to %08x\n",tri.accel_drawflags);
		break;
	case 0x0c:  // Foreground Colour
		tri.accel_fgcolour = (tri.accel_fgcolour & 0xffffff00) | data;
		LOGACCEL("Trident: Foreground Colour set to %08x\n",tri.accel_fgcolour);
		break;
	case 0x0d:
		tri.accel_fgcolour = (tri.accel_fgcolour & 0xffff00ff) | (data << 8);
		LOGACCEL("Trident: Foreground Colour set to %08x\n",tri.accel_fgcolour);
		break;
	case 0x0e:
		tri.accel_fgcolour = (tri.accel_fgcolour & 0xff00ffff) | (data << 16);
		LOGACCEL("Trident: Foreground Colour set to %08x\n",tri.accel_fgcolour);
		break;
	case 0x0f:
		tri.accel_fgcolour = (tri.accel_fgcolour & 0x00ffffff) | (data << 24);
		LOGACCEL("Trident: Foreground Colour set to %08x\n",tri.accel_fgcolour);
		break;
	case 0x10:  // Background Colour
		tri.accel_bgcolour = (tri.accel_bgcolour & 0xffffff00) | data;
		LOGACCEL("Trident: Background Colour set to %08x\n",tri.accel_bgcolour);
		break;
	case 0x11:
		tri.accel_bgcolour = (tri.accel_bgcolour & 0xffff00ff) | (data << 8);
		LOGACCEL("Trident: Background Colour set to %08x\n",tri.accel_bgcolour);
		break;
	case 0x12:
		tri.accel_bgcolour = (tri.accel_bgcolour & 0xff00ffff) | (data << 16);
		LOGACCEL("Trident: Background Colour set to %08x\n",tri.accel_bgcolour);
		break;
	case 0x13:
		tri.accel_bgcolour = (tri.accel_bgcolour & 0x00ffffff) | (data << 24);
		LOGACCEL("Trident: Background Colour set to %08x\n",tri.accel_bgcolour);
		break;
	case 0x14:  // Pattern Location
		tri.accel_pattern_loc = (tri.accel_pattern_loc & 0xff00) | data;
		LOGACCEL("Trident: Pattern Location set to %04x\n",tri.accel_pattern_loc);
		break;
	case 0x15:
		tri.accel_pattern_loc = (tri.accel_pattern_loc & 0x00ff) | (data << 8);
		LOGACCEL("Trident: Pattern Location set to %04x\n",tri.accel_pattern_loc);
		break;
	case 0x18:  // Destination X
		tri.accel_dest_x = (tri.accel_dest_x & 0xff00) | data;
		LOGACCEL("Trident: Destination X set to %04x\n",tri.accel_dest_x);
		break;
	case 0x19:
		tri.accel_dest_x = (tri.accel_dest_x & 0x00ff) | (data << 8);
		LOGACCEL("Trident: Destination X set to %04x\n",tri.accel_dest_x);
		break;
	case 0x1a:  // Destination Y
		tri.accel_dest_y = (tri.accel_dest_y & 0xff00) | data;
		LOGACCEL("Trident: Destination Y set to %04x\n",tri.accel_dest_y);
		break;
	case 0x1b:
		tri.accel_dest_y = (tri.accel_dest_y & 0x00ff) | (data << 8);
		LOGACCEL("Trident: Destination Y set to %04x\n",tri.accel_dest_y);
		break;
	case 0x1c:  // Source X
		tri.accel_source_x = (tri.accel_source_x & 0xff00) | data;
		LOGACCEL("Trident: Source X set to %04x\n",tri.accel_source_x);
		break;
	case 0x1d:
		tri.accel_source_x = (tri.accel_source_x & 0x00ff) | (data << 8);
		LOGACCEL("Trident: Source X set to %04x\n",tri.accel_source_x);
		break;
	case 0x1e:  // Source Y
		tri.accel_source_y = (tri.accel_source_y & 0xff00) | data;
		LOGACCEL("Trident: Source Y set to %04x\n",tri.accel_source_y);
		break;
	case 0x1f:
		tri.accel_source_y = (tri.accel_source_y & 0x00ff) | (data << 8);
		LOGACCEL("Trident: Source Y set to %04x\n",tri.accel_source_y);
		break;
	case 0x20:  // Dimension(?) X
		tri.accel_dim_x = (tri.accel_dim_x & 0xff00) | data;
		LOGACCEL("Trident: Dimension X set to %04x\n",tri.accel_dim_x);
		break;
	case 0x21:
		tri.accel_dim_x = (tri.accel_dim_x & 0x00ff) | (data << 8);
		LOGACCEL("Trident: Dimension X set to %04x\n",tri.accel_dim_x);
		break;
	case 0x22:  // Dimension(?) Y
		tri.accel_dim_y = (tri.accel_dim_y & 0xff00) | data;
		LOGACCEL("Trident: Dimension y set to %04x\n",tri.accel_dim_y);
		break;
	case 0x23:
		tri.accel_dim_y = (tri.accel_dim_y & 0x00ff) | (data << 8);
		LOGACCEL("Trident: Dimension y set to %04x\n",tri.accel_dim_y);
		break;
	case 0x24:  // Style
		tri.accel_style = (tri.accel_style & 0xffffff00) | data;
		LOGACCEL("Trident: Style set to %08x\n",tri.accel_style);
		break;
	case 0x25:
		tri.accel_style = (tri.accel_style & 0xffff00ff) | (data << 8);
		LOGACCEL("Trident: Style set to %08x\n",tri.accel_style);
		break;
	case 0x26:
		tri.accel_style = (tri.accel_style & 0xff00ffff) | (data << 16);
		LOGACCEL("Trident: Style set to %08x\n",tri.accel_style);
		break;
	case 0x27:
		tri.accel_style = (tri.accel_style & 0x00ffffff) | (data << 24);
		LOGACCEL("Trident: Style set to %08x\n",tri.accel_style);
		break;
	case 0x28:  // Source Clip X
		tri.accel_source_x_clip = (tri.accel_source_x_clip & 0xff00) | data;
		LOGACCEL("Trident: Source X Clip set to %04x\n",tri.accel_source_x_clip);
		break;
	case 0x29:
		tri.accel_source_x_clip = (tri.accel_source_x_clip & 0x00ff) | (data << 8);
		LOGACCEL("Trident: Source X Clip set to %04x\n",tri.accel_source_x_clip);
		break;
	case 0x2a:  // Source Clip Y
		tri.accel_source_y_clip = (tri.accel_source_y_clip & 0xff00) | data;
		LOGACCEL("Trident: Source Y Clip set to %04x\n",tri.accel_source_y_clip);
		break;
	case 0x2b:
		tri.accel_source_y_clip = (tri.accel_source_y_clip & 0x00ff) | (data << 8);
		LOGACCEL("Trident: Source Y Clip set to %04x\n",tri.accel_source_y_clip);
		break;
	case 0x2c:  // Destination Clip X
		tri.accel_dest_x_clip = (tri.accel_dest_x_clip & 0xff00) | data;
		LOGACCEL("Trident: Destination X Clip set to %04x\n",tri.accel_dest_x_clip);
		break;
	case 0x2d:
		tri.accel_dest_x_clip = (tri.accel_dest_x_clip & 0x00ff) | (data << 8);
		LOGACCEL("Trident: Destination X Clip set to %04x\n",tri.accel_dest_x_clip);
		break;
	case 0x2e:  // Destination Clip Y
		tri.accel_dest_y_clip = (tri.accel_dest_y_clip & 0xff00) | data;
		LOGACCEL("Trident: Destination Y Clip set to %04x\n",tri.accel_dest_y_clip);
		break;
	case 0x2f:
		tri.accel_dest_y_clip = (tri.accel_dest_y_clip & 0x00ff) | (data << 8);
		LOGACCEL("Trident: Destination Y Clip set to %04x\n",tri.accel_dest_y_clip);
		break;
	case 0x48:  // CKEY (Chromakey?)
		tri.accel_ckey = (tri.accel_ckey & 0xffffff00) | data;
		LOGACCEL("Trident: CKey set to %08x\n",tri.accel_ckey);
		break;
	case 0x49:
		tri.accel_ckey = (tri.accel_ckey & 0xffff00ff) | (data << 8);
		LOGACCEL("Trident: CKey set to %08x\n",tri.accel_ckey);
		break;
	case 0x4a:
		tri.accel_ckey = (tri.accel_ckey & 0xff00ffff) | (data << 16);
		LOGACCEL("Trident: CKey set to %08x\n",tri.accel_ckey);
		break;
	case 0x4b:
		tri.accel_ckey = (tri.accel_ckey & 0x00ffffff) | (data << 24);
		LOGACCEL("Trident: CKey set to %08x\n",tri.accel_ckey);
		break;
	case 0x58:  // Foreground Pattern Colour
		tri.accel_fg_pattern_colour = (tri.accel_fg_pattern_colour & 0xffffff00) | data;
		LOGACCEL("Trident: FG Pattern Colour set to %08x\n",tri.accel_fg_pattern_colour);
		break;
	case 0x59:
		tri.accel_fg_pattern_colour = (tri.accel_fg_pattern_colour & 0xffff00ff) | (data << 8);
		LOGACCEL("Trident: FG Pattern Colour set to %08x\n",tri.accel_fg_pattern_colour);
		break;
	case 0x5a:
		tri.accel_fg_pattern_colour = (tri.accel_fg_pattern_colour & 0xff00ffff) | (data << 16);
		LOGACCEL("Trident: FG Pattern Colour set to %08x\n",tri.accel_fg_pattern_colour);
		break;
	case 0x5b:
		tri.accel_fg_pattern_colour = (tri.accel_fg_pattern_colour & 0x00ffffff) | (data << 24);
		LOGACCEL("Trident: FG Pattern Colour set to %08x\n",tri.accel_fg_pattern_colour);
		break;
	case 0x5c:  // Background Pattern Colour
		tri.accel_bg_pattern_colour = (tri.accel_bg_pattern_colour & 0xffffff00) | data;
		LOGACCEL("Trident: BG Pattern Colour set to %08x\n",tri.accel_bg_pattern_colour);
		break;
	case 0x5d:
		tri.accel_bg_pattern_colour = (tri.accel_bg_pattern_colour & 0xffff00ff) | (data << 8);
		LOGACCEL("Trident: BG Pattern Colour set to %08x\n",tri.accel_bg_pattern_colour);
		break;
	case 0x5e:
		tri.accel_bg_pattern_colour = (tri.accel_bg_pattern_colour & 0xff00ffff) | (data << 16);
		LOGACCEL("Trident: BG Pattern Colour set to %08x\n",tri.accel_bg_pattern_colour);
		break;
	case 0x5f:
		tri.accel_bg_pattern_colour = (tri.accel_bg_pattern_colour & 0x00ffffff) | (data << 24);
		LOGACCEL("Trident: BG Pattern Colour set to %08x\n",tri.accel_bg_pattern_colour);
		break;
	default:
		LOGTODO("Trident: unimplemented acceleration register offset %02x write %02x\n",offset,data);
	}
}

void trident_vga_device::accel_command()
{
	switch(tri.accel_command)
	{
	case 0x00:
		LOG("Trident: Command: NOP\n");
		break;
	case 0x01:
		LOG("Trident: Command: BitBLT ROP3 (Source %i,%i Dest %i,%i Size %i,%i)\n",tri.accel_source_x,tri.accel_source_y,tri.accel_dest_x,tri.accel_dest_y,tri.accel_dim_x,tri.accel_dim_y);
		LOG("BitBLT: Drawflags = %08x FMIX = %02x\n",tri.accel_drawflags,tri.accel_fmix);
		accel_bitblt();
		break;
	case 0x02:
		LOG("Trident: Command: BitBLT ROP4\n");
		break;
	case 0x03:
		LOG("Trident: Command: Scanline\n");
		break;
	case 0x04:
		LOG("Trident: Command: Bresenham Line (Source %i,%i Dest %i,%i Size %i,%i)\n",tri.accel_source_x,tri.accel_source_y,tri.accel_dest_x,tri.accel_dest_y,tri.accel_dim_x,tri.accel_dim_y);
		LOG("BLine: Drawflags = %08x FMIX = %02x\n",tri.accel_drawflags,tri.accel_fmix);
		accel_line();
		break;
	case 0x05:
		LOG("Trident: Command: Short Vector\n");
		break;
	case 0x06:
		LOG("Trident: Command: Fast Line\n");
		break;
	case 0x07:
		LOG("Trident: Command: Trapezoid Fill\n");
		break;
	case 0x08:
		LOG("Trident: Command: Ellipse\n");
		break;
	case 0x09:
		LOG("Trident: Command: Ellipse Fill\n");
		break;
	default:
		LOGTODO("Trident: Unknown acceleration command %02x\n",tri.accel_command);
	}
}

void trident_vga_device::accel_bitblt()
{
	int x,y;
	int sx,sy;
	int xdir,ydir;
	int xstart,xend,ystart,yend;

	if(tri.accel_drawflags & 0x0040)  // TODO: handle PATMONO also
	{
		tri.accel_mem_x = tri.accel_dest_x;
		tri.accel_mem_y = tri.accel_dest_y;
		tri.accel_memwrite_active = true;
		return;
	}

	if(tri.accel_drawflags & 0x0200)
	{
		xdir = -1;
		xstart = tri.accel_dest_x;
		xend = tri.accel_dest_x-tri.accel_dim_x-1;
	}
	else
	{
		xdir = 1;
		xstart = tri.accel_dest_x;
		xend = tri.accel_dest_x+tri.accel_dim_x+1;
	}
	if(tri.accel_drawflags & 0x0100)
	{
		ydir = -1;
		ystart = tri.accel_dest_y;
		yend = tri.accel_dest_y-tri.accel_dim_y-1;
	}
	else
	{
		ydir = 1;
		ystart = tri.accel_dest_y;
		yend = tri.accel_dest_y+tri.accel_dim_y+1;
	}
	sy = tri.accel_source_y;

	for(y=ystart;y!=yend;y+=ydir,sy+=ydir)
	{
		sx = tri.accel_source_x;
		for(x=xstart;x!=xend;x+=xdir,sx+=xdir)
		{
			if(tri.accel_drawflags & 0x4000)  // Solid fill
			{
				WRITEPIXEL(x,y,tri.accel_fgcolour);
			}
			else
			{
				WRITEPIXEL(x,y,READPIXEL(sx,sy));
			}
		}
	}
}

void trident_vga_device::accel_line()
{
	uint32_t col = tri.accel_fgcolour;
//    TGUI_SRC_XY(dmin-dmaj,dmin);
//    TGUI_DEST_XY(x,y);
//    TGUI_DIM_XY(dmin+e,len);
	int16_t dx = tri.accel_source_y - tri.accel_source_x;
	int16_t dy = tri.accel_source_y;
	int16_t err = tri.accel_dim_x + tri.accel_source_y;
	int sx = (tri.accel_drawflags & 0x0200) ? -1 : 1;
	int sy = (tri.accel_drawflags & 0x0100) ? -1 : 1;
	int x,y,z;

	x = tri.accel_dest_x;
	y = tri.accel_dest_y;

	WRITEPIXEL(x,y,col);
	for(z=0;z<tri.accel_dim_y;z++)
	{
		if(tri.accel_drawflags & 0x0400)
			y += sy;
		else
			x += sx;
		if(err > 0)
		{
			if(tri.accel_drawflags & 0x0400)
				x += sx;
			else
				y += sy;
			WRITEPIXEL(x,y,col);
			err += (dy-dx);
		}
		else
		{
			WRITEPIXEL(x,y,col);
			err += dy;
		}
	}
}

// feed data written to VRAM to an active BitBLT command
void trident_vga_device::accel_data_write(uint32_t data)
{
	int xdir = 1,ydir = 1;

	if(tri.accel_drawflags & 0x0200)  // XNEG
		xdir = -1;
	if(tri.accel_drawflags & 0x0100)  // YNEG
		ydir = -1;

	for(int x=31;x>=0;x--)
	{
		if(tri.accel_mem_x <= tri.accel_dest_x+tri.accel_dim_x && tri.accel_mem_x >= tri.accel_dest_x-tri.accel_dim_x)
		{
			if(((data >> x) & 0x01) != 0)
				WRITEPIXEL(tri.accel_mem_x,tri.accel_mem_y,tri.accel_fgcolour);
			else
				WRITEPIXEL(tri.accel_mem_x,tri.accel_mem_y,tri.accel_bgcolour);
		}
		tri.accel_mem_x+=xdir;
	}
	if(tri.accel_mem_x > tri.accel_dest_x+tri.accel_dim_x || tri.accel_mem_x < tri.accel_dest_x-tri.accel_dim_x)
	{
		tri.accel_mem_x = tri.accel_dest_x;
		tri.accel_mem_y+=ydir;
		if(tri.accel_mem_y > tri.accel_dest_y+tri.accel_dim_y || tri.accel_mem_y < tri.accel_dest_y-tri.accel_dim_y)
			tri.accel_memwrite_active = false;  // completed
	}
}

// tvga9000 CRTC override
// not extensively tested, just enough for pntnpuzl to not throw a 168 Hz refresh rate.
void tvga9000_device::recompute_params()
{
	int divisor = 1;
	int xtal;

	u8 xtal_select = (vga.miscellaneous_output & 0x0c) >> 2;
	xtal_select |= (tri.sr0d_new & 0x01) << 2;
	xtal_select |= (tri.sr0d_new & 0x40) >> 3;

	switch(xtal_select)
	{
		case 0:  xtal = 25174800; break;
		case 1:  xtal = 28636363; break;
		case 2:  xtal = 44900000; break;
		case 3:  xtal = 36000000; break;
		case 4:  xtal = 57272000; break;
		case 5:  xtal = 65000000; break;
		case 6:  xtal = 50350000; break;
		case 7:  xtal = 40000000; break;
		case 8:  xtal = 25174800; break;
		case 9:  xtal = 28636363; break;
		case 10: xtal = 62300000; break;
		case 11: xtal = 44900000; break;
		case 12: xtal = 72000000; break;
		case 13: xtal = 77000000; break;
		case 14: xtal = 80000000; break;
		case 15: xtal = 75000000; break;
	}

	LOGCRTC("trident_define_video_mode: %d (%d)\n", xtal_select, xtal);

	switch((tri.sr0d_new & 0x06) >> 1)
	{
		case 0: break; // no division
		case 1: xtal /= 2; break;
		case 2: xtal /= 4; break;
		case 3: xtal /= 1.5; break;
	}

	if(tri.gc0f & 0x08)  // 16 pixels per character clock
		xtal = xtal / 2;

	if(tri.port_3db & 0x20)
		xtal = xtal / 2;  // correct?

	LOGCRTC("division setting %d %02x %02x (new xtal %d)\n", (tri.sr0d_new & 0x06) >> 1, tri.gc0f, tri.port_3db, xtal);

	svga.rgb8_en = svga.rgb15_en = svga.rgb16_en = svga.rgb32_en = 0;
	switch((tri.pixel_depth & 0x0c) >> 2)
	{
	case 0:
	default: if(!(tri.pixel_depth & 0x10) || (tri.cr1e & 0x80)) svga.rgb8_en = 1; break;
	case 1:  if((tri.dac & 0xf0) == 0x30) svga.rgb16_en = 1; else svga.rgb15_en = 1; break;
	case 2:  svga.rgb32_en = 1; break;
	}

	// TODO: is this right?
	// documentation claims to be "host address bit 16"
	// pntnpuzl definitely needs to halve divisor and xtal otherwise it will draw in 800 x 240.
	if(BIT(tri.cr1e, 7))
	{
		divisor = 2;
		xtal /= 2;
	}
	// TODO: tri.cr1e bit 2 for interlace

	LOGCRTC("pixel depth %02x module testing %02x\n", tri.pixel_depth, tri.cr1e);

	recompute_params_clock(divisor, xtal);
}
