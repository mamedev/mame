// license:BSD-3-Clause
// copyright-holders: Carl, Angelo Salese

#include "emu.h"
#include "pc_vga_oak.h"

#define LOG_BANK       (1U << 2) // extended segment regs

#define VERBOSE (LOG_GENERAL)

#include "logmacro.h"

#define LOGBANK(...)          LOGMASKED(LOG_BANK, __VA_ARGS__)


DEFINE_DEVICE_TYPE(OTI111,     oak_oti111_vga_device,  "oti111_vga",  "Oak Technologies Spitfire 64111 i/f")

oak_oti111_vga_device::oak_oti111_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: svga_device(mconfig, OTI111, tag, owner, clock)
	, m_xga(*this, "xga")
{
	m_main_if_space_config = address_space_config("io_regs", ENDIANNESS_LITTLE, 8, 4, 0, address_map_constructor(FUNC(oak_oti111_vga_device::io_3bx_3dx_map), this));
	m_oak_space_config = address_space_config("oak_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(oak_oti111_vga_device::oak_map), this));
}

void oak_oti111_vga_device::device_add_mconfig(machine_config &config)
{
	XGA_COPRO(config, m_xga, 0);
	m_xga->mem_read_callback().set(FUNC(oak_oti111_vga_device::mem_linear_r));
	m_xga->mem_write_callback().set(FUNC(oak_oti111_vga_device::mem_linear_w));
	m_xga->set_type(xga_copro_device::TYPE::OTI111);
}

device_memory_interface::space_config_vector oak_oti111_vga_device::memory_space_config() const
{
	auto r = svga_device::memory_space_config();
	r.emplace_back(std::make_pair(EXT_REG,     &m_oak_space_config));
	return r;
}

void oak_oti111_vga_device::device_start()
{
	svga_device::device_start();
}

void oak_oti111_vga_device::device_reset()
{
	svga_device::device_reset();
	// Spitfire BIOS doesn't explicitly set IOAS at boot
	m_ioas = true;
	m_memory_size = 0x0a;
	m_oti_map_select = false;
	m_oti_aperture_mask = 0x3ffff;
	m_oak_gfx_mode = false;

	m_cursor_control = 0;
}

void oak_oti111_vga_device::io_3bx_3dx_map(address_map &map)
{
	svga_device::io_3bx_3dx_map(map);
	map(0x0e, 0x0e).rw(FUNC(oak_oti111_vga_device::oak_index_r), FUNC(oak_oti111_vga_device::oak_index_w));
	map(0x0f, 0x0f).rw(FUNC(oak_oti111_vga_device::oak_data_r), FUNC(oak_oti111_vga_device::oak_data_w));
}

u8 oak_oti111_vga_device::oak_index_r(offs_t offset)
{
	return m_oak_idx;
}

void oak_oti111_vga_device::oak_index_w(offs_t offset, u8 data)
{
	m_oak_idx = data;
}

u8 oak_oti111_vga_device::oak_data_r(offs_t offset)
{
	return space(EXT_REG).read_byte(m_oak_idx);
}

void oak_oti111_vga_device::oak_data_w(offs_t offset, u8 data)
{
	space(EXT_REG).write_byte(m_oak_idx, data);
}

void oak_oti111_vga_device::oak_map(address_map &map)
{
	// (undocumented) Revision ID
	// win98se tests 0x06 / 0x07 / 0x0a / 0x0b paths, failing in case it doesn't find a valid value.
	// 64111 BIOS wants it to be == 6 at POST, printing 64107 in case it isn't
	map(0x00, 0x00).lr8(
		NAME([] () {
			//machine().debug_break();
			return 0x06;
		})
	// win98se also manages to write here a lot ...
	).nopw();

	// status, set by BIOS for memory size
	map(0x02, 0x02).lrw8(
		NAME([this] (offs_t offset) {
			//LOG("OAK02: Status read\n");
			return m_memory_size;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("OAK02: Status %02x\n", data);
			m_memory_size = data & 0x0e;
		})
	);
	//map(0x03, 0x03) OTI Test 1
	//map(0x04, 0x04) OTI Test 2
	//map(0x06, 0x06) Video Clock Select
	// Hardware Configuration 1
	map(0x07, 0x07).lr8(
		NAME([] () {
			// TODO: MD[7:0] pins
			return 0x91;
		})
	);
	// Hardware Configuration 2
	map(0x08, 0x08).lr8(
		NAME([] () {
			// TODO: MD[15:8] pins
			return 0x81;
		})
	);
	// Hardware Configuration 3
	map(0x09, 0x09).lr8(
		NAME([] () {
			// TODO: MD[23:16] pins
			return 0x00;
		})
	);
	// i2c Control
	map(0x0c, 0x0c).lrw8(
		NAME([this] (offs_t offset) {
			// HACK: loopback DDC for now
			return (m_i2c_data & 0x3) << 4;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_i2c_data = data & 3;
		})
	);
	// DSW port
	map(0x0d, 0x0d).lr8(
		NAME([] () {
			// TODO: read_cb
			return 0xff;
		})
	);
	//map(0x0e, 0x0e) EEPROM Control
	//map(0x0f, 0x0f) Power Management Control

	//map(0x10, 0x10) Local Bus Control
	map(0x11, 0x11).lrw8(
		NAME([this] (offs_t offset) {
			return (svga.bank_w << 4) | (svga.bank_r & 0xf);
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOGBANK("OAK11: (backward-)Compatible Segment %02x\n", data);
			svga.bank_r = data & 0xf;
			svga.bank_w = data >> 4;
		})
	);
	//map(0x13, 0x13) ISA Bus Control
	map(0x14, 0x14).lrw8(
		NAME([this] (offs_t offset) {
			return (m_oti_map_select << 0) | (m_oti_aperture_select << 2);
		}),
		NAME([this] (offs_t offset, u8 data) {
			// TODO: what this really selects?
			m_oti_map_select = bool(BIT(data, 0));
			m_oti_aperture_select = (data & 0x18) >> 2;
			m_oti_aperture_mask = (1 << (m_oti_aperture_select + 18)) - 1;
			LOG("OAK14: Video Memory Mapping %02x (aperture mask %08x)\n", data, m_oti_aperture_mask);
		})
	);
	//map(0x15, 0x15) Memory & MMIO Enable
	//(bit 7 -> enable MMIO)

	//map(0x19, 0x19) Configuration/DAC/Auxiliary Range
	//map(0x20, 0x20) Display FIFO Depth
	map(0x21, 0x21).lrw8(
		NAME([this] (offs_t offset) {
			return m_oak_gfx_mode << 2;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("OAK21: Mode Select %02x\n", data);
			m_oak_gfx_mode = BIT(data, 2);
			recompute_params();
		})
	);
	//map(0x22, 0x22) Feature Select
	map(0x23, 0x23).lrw8(
		NAME([this] (offs_t offset) {
			return svga.bank_r & 0x7f;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOGBANK("OAK23: Extended Read Segment %02x\n", data);
			svga.bank_r = data & 0x7f;
		})
	);
	map(0x24, 0x24).lrw8(
		NAME([this] (offs_t offset) {
			return svga.bank_w & 0x7f;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOGBANK("OAK24: Extended Read Segment %02x\n", data);
			svga.bank_w = data & 0x7f;
		})
	);
	map(0x25, 0x25).lrw8(
		NAME([this] (offs_t offset) {
			return svga.bank_w & 0x7f;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOGBANK("OAK25: Extended Common R/W Segment %02x\n", data);
			svga.bank_r = data & 0x7f;
			svga.bank_w = data & 0x7f;
		})
	);
	//map(0x26, 0x26) RASn Control
	//map(0x27, 0x27) CASn Control
	//map(0x28, 0x28) Refresh Control
	//map(0x29, 0x29) Hardware Window Arbitration

	//map(0x30, 0x30) OTI CRT Overflow
	//map(0x31, 0x31) CRT Start Address
	//map(0x32, 0x32) HSync/2 Start
	map(0x33, 0x33).lrw8(
		NAME([this] (offs_t offset) {
			return vga.crtc.no_wrap;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("OAK33: CRT Address Compatibility %02x\n", data);
			vga.crtc.no_wrap = BIT(data, 0);
		})
	);
	map(0x38, 0x38).lrw8(
		NAME([this] (offs_t offset) {
			return (m_pixel_mode & 0xf) | (m_color_swap << 4) | (m_bpp << 5);
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOG("OAK38: Pixel i/f %02x\n", data);
			m_pixel_mode = data & 0xf;
			// TODO: swaps R and B guns in 16bpp / 24bpp modes
			m_color_swap = bool(BIT(data, 4));
			m_bpp = (data >> 5) & 3;
			recompute_params();
		})
	);
	//map(0x39, 0x3a) Extended Overscan Color

	// Scratch Pad
	map(0xf0, 0xf7).lrw8(
		NAME([this] (offs_t offset) {
			return m_scratchpad[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_scratchpad[offset] = data;
		})
	);
}

// TODO: move this logic to standalone OAK XGA core
u8 oak_oti111_vga_device::xga_read(offs_t offset)
{
	switch(offset)
	{
		case 0x13: //fifo status
			return 0xf;
		default:
			return m_xga->xga_read(offset);
	}
	return 0;
}

void oak_oti111_vga_device::xga_write(offs_t offset, u8 data)
{
	m_xga->xga_write(offset, data);
}

void oak_oti111_vga_device::multimedia_map(address_map &map)
{
	//HC = Hardware Cursor
	//HW = Hardware Window
	map(0x00, 0x01).lrw16(
		NAME([this] (offs_t offset) {
			return m_cursor_x;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_cursor_x);
		})
	);
	map(0x02, 0x03).lrw16(
		NAME([this] (offs_t offset) {
			return m_cursor_y;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_cursor_y);
		})
	);
	//map(0x04, 0x04) HC Horizontal Preset/HW Width Low
	//map(0x05, 0x05) HW Width High
	//map(0x06, 0x06) HC Vertical Preset/HW Height Low
	//map(0x07, 0x07) HW Height High
	map(0x08, 0x0b).lrw32(
		NAME([this] (offs_t offset) {
			return m_cursor_address_base;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_cursor_address_base);
			// clamp to 24-bits
			m_cursor_address_base &= 0xffffff;
			//LOG("HC Start Address %08x & %08x\n", data, mem_mask);

		})
	);
	map(0x0c, 0x13).lrw32(
		NAME([this] (offs_t offset) {
			return m_cursor_color[offset];
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_cursor_color[offset]);
			LOG("HC color %d %08x & %08x\n", offset, data, mem_mask);
		})
	);
	map(0x14, 0x14).lrw8(
		NAME([this] (offs_t offset) {
			return m_cursor_control;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_cursor_control = data;
			LOG("HC control $94 %02x\n", data);
		})
	);
	//map(0x15, 0x15) Multimedia Port
	//map(0x16, 0x17) HW Control
	//map(0x18, 0x1a) HW Mask Map Start Address
	//map(0x1b, 0x1b) Multimedia Mask Map Offset
	//map(0x1c, 0x1e) HW Start Address
	//map(0x1f, 0x1f) HW Address Offset
	//map(0x20, 0x21) Video Window Width
	//map(0x22, 0x23) Video Window Height
}

void oak_oti111_vga_device::ramdac_mmio_map(address_map &map)
{
	map.unmap_value_high();
	map(0x04, 0x04).rw(FUNC(oak_oti111_vga_device::oak_index_r), FUNC(oak_oti111_vga_device::oak_index_w));
	map(0x05, 0x05).rw(FUNC(oak_oti111_vga_device::oak_data_r), FUNC(oak_oti111_vga_device::oak_data_w));
	map(0x06, 0x06).rw(FUNC(oak_oti111_vga_device::ramdac_mask_r), FUNC(oak_oti111_vga_device::ramdac_mask_w));
	map(0x07, 0x07).rw(FUNC(oak_oti111_vga_device::ramdac_state_r), FUNC(oak_oti111_vga_device::ramdac_read_index_w));
	map(0x08, 0x08).rw(FUNC(oak_oti111_vga_device::ramdac_write_index_r), FUNC(oak_oti111_vga_device::ramdac_write_index_w));
	map(0x09, 0x09).rw(FUNC(oak_oti111_vga_device::ramdac_data_r), FUNC(oak_oti111_vga_device::ramdac_data_w));
}

uint16_t oak_oti111_vga_device::offset()
{
	if (m_oak_gfx_mode)
		return vga.crtc.offset << 4;
	return svga_device::offset();
}

u8 oak_oti111_vga_device::mem_r(offs_t offset)
{
	if (((offset & 0x10000) == 0) && m_oak_gfx_mode)
		return svga_device::mem_linear_r((offset + svga.bank_r * 0x10000) & m_oti_aperture_mask);
	return svga_device::mem_r(offset);
}

void oak_oti111_vga_device::mem_w(offs_t offset, uint8_t data)
{
	if (((offset & 0x10000) == 0) && m_oak_gfx_mode)
	{
		svga_device::mem_linear_w((offset + svga.bank_w * 0x10000) & m_oti_aperture_mask, data);
		return;
	}
	svga_device::mem_w(offset, data);
}

void oak_oti111_vga_device::recompute_params()
{
	u8 xtal_select = (vga.miscellaneous_output & 0x0c) >> 2;
	int xtal;

	svga.rgb8_en = svga.rgb15_en = svga.rgb16_en = svga.rgb24_en = svga.rgb32_en = 0;

	if (m_oak_gfx_mode)
	{
		switch(m_pixel_mode)
		{
			case 0:
				// VGA mode
				break;
			case 2:
			case 4:
				svga.rgb8_en = 1;
				break;
			case 3:
				svga.rgb24_en = 1;
				break;
			case 5:
				// TODO: SciTech uses this in both 15 and 16bpp
				svga.rgb16_en = 1;
				break;
			case 7:
				svga.rgb24_en = 1;
				break;
			case 8:
				// SciTech can't detect this, assumed by win98se
				svga.rgb32_en = 1;
				break;
			default:
				popmessage("pc_vga_oak: unhandled pixel mode %02x", m_pixel_mode);
				break;
		}
	}

	switch(xtal_select & 3)
	{
		case 0: xtal = XTAL(25'174'800).value(); break;
		case 1: xtal = XTAL(28'636'363).value(); break;
		// TODO: to external PLL (OTI-088 or ATT20C409 / ATT20C499)
		// gs471 seems to use former
		case 2:
			// TODO: Video Clock Reg 2 / Set C
			xtal = XTAL(25'174'800).value();
			//xtal = XTAL(50'144'xxx)
			break;
		case 3:
			// TODO: Video Clock Reg 3 / Set D
			xtal = XTAL(28'636'363).value();
			//xtal = XTAL(75'170'xxx)
			break;
	}

	recompute_params_clock(1, xtal);
}

uint32_t oak_oti111_vga_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	svga_device::screen_update(screen, bitmap, cliprect);

	// HW cursor

	if (BIT(m_cursor_control, 0))
	{
		// TODO: preliminary, should be 64x64, win98se just uses this portion
		// Drawing specifics aren't really documented beyond what the register does.

		// TODO: x4 in planar mode, x8 in packed pixel
		const u32 base_offs = (m_cursor_address_base * 8) + 0x200;
		const u8 transparent_pen = 2;

		for (int y = 0; y < 32; y ++)
		{
			int res_y = y + m_cursor_y;
			for (int x = 0; x < 32; x++)
			{
				int res_x = x + m_cursor_x;
				if (!cliprect.contains(res_x, res_y))
					continue;
				const u32 cursor_address = ((x >> 3) + y * 16) + base_offs;
				// TODO: std::function for Intel format (win98se uses Motorola)
				const int xi = 7 - (x & 7);
				u8 cursor_gfx =  (vga.memory[(cursor_address + 0x8) % vga.svga_intf.vram_size] >> (xi) & 1);
				cursor_gfx   |= ((vga.memory[(cursor_address + 0xc) % vga.svga_intf.vram_size] >> (xi)) & 1) << 1;

				if (cursor_gfx & transparent_pen)
					continue;

				// TODO: should really mask by pixel depth
				bitmap.pix(res_y, res_x) = m_cursor_color[cursor_gfx] & 0xffffff;
			}
		}
	}

	return 0;
}
