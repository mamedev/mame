// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 *
 * Cirrus Logic CL-GD542x/3x video chipsets
 *
 * TODO:
 * - Original Acumos AVGA1/2 chipsets (Cirrus Logic eventually bought Acumos and rebranded);
 * - Fix or implement hidden DAC modes (15bpp + mixed, true color, others);
 * - zorro/picasso2: many blitting errors, verify HW cursor;
 * - Merge with trs/vis.cpp implementation (CL-GD5200 RAMDAC with custom VGA controller)
 *
 */

#include "emu.h"
#include "pc_vga_cirrus.h"

#include "screen.h"


#define LOG_REGS (1U << 1)
#define LOG_BLIT (1U << 2)
#define LOG_HDAC (1U << 3) // log hidden DAC
#define LOG_BANK (1U << 4) // log offset registers
#define LOG_PLL  (1U << 5)

#define VERBOSE (LOG_GENERAL | LOG_HDAC | LOG_REGS | LOG_BLIT)
#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


DEFINE_DEVICE_TYPE(CIRRUS_GD5428_VGA, cirrus_gd5428_vga_device, "clgd5428", "Cirrus Logic GD5428 VGA i/f")
DEFINE_DEVICE_TYPE(CIRRUS_GD5430_VGA, cirrus_gd5430_vga_device, "clgd5430", "Cirrus Logic GD5430 VGA i/f")
DEFINE_DEVICE_TYPE(CIRRUS_GD5446_VGA, cirrus_gd5446_vga_device, "clgd5446", "Cirrus Logic GD5446 VGA i/f")



cirrus_gd5428_vga_device::cirrus_gd5428_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cirrus_gd5428_vga_device(mconfig, CIRRUS_GD5428_VGA, tag, owner, clock)
{
	m_crtc_space_config = address_space_config("crtc_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(cirrus_gd5428_vga_device::crtc_map), this));
	m_gc_space_config = address_space_config("gc_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(cirrus_gd5428_vga_device::gc_map), this));
	m_seq_space_config = address_space_config("sequencer_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(cirrus_gd5428_vga_device::sequencer_map), this));
}

cirrus_gd5428_vga_device::cirrus_gd5428_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: svga_device(mconfig, type, tag, owner, clock)
{
}

cirrus_gd5430_vga_device::cirrus_gd5430_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cirrus_gd5430_vga_device(mconfig, CIRRUS_GD5430_VGA, tag, owner, clock)
{
	m_crtc_space_config = address_space_config("crtc_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(cirrus_gd5430_vga_device::crtc_map), this));
	m_gc_space_config = address_space_config("gc_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(cirrus_gd5430_vga_device::gc_map), this));
	m_seq_space_config = address_space_config("sequencer_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(cirrus_gd5430_vga_device::sequencer_map), this));
}

cirrus_gd5430_vga_device::cirrus_gd5430_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: cirrus_gd5428_vga_device(mconfig, type, tag, owner, clock)
{
}

cirrus_gd5446_vga_device::cirrus_gd5446_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cirrus_gd5430_vga_device(mconfig, CIRRUS_GD5446_VGA, tag, owner, clock)
{
	m_crtc_space_config = address_space_config("crtc_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(cirrus_gd5446_vga_device::crtc_map), this));
	m_gc_space_config = address_space_config("gc_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(cirrus_gd5446_vga_device::gc_map), this));
	m_seq_space_config = address_space_config("sequencer_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(cirrus_gd5446_vga_device::sequencer_map), this));
}

void cirrus_gd5428_vga_device::io_3cx_map(address_map &map)
{
	svga_device::io_3cx_map(map);
	map(0x06, 0x06).rw(FUNC(cirrus_gd5428_vga_device::ramdac_hidden_mask_r), FUNC(cirrus_gd5428_vga_device::ramdac_hidden_mask_w));
	map(0x09, 0x09).rw(FUNC(cirrus_gd5428_vga_device::ramdac_overlay_r), FUNC(cirrus_gd5428_vga_device::ramdac_overlay_w));
}

u8 cirrus_gd5428_vga_device::ramdac_hidden_mask_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		m_hidden_dac_phase ++;
	if (m_hidden_dac_phase >= 4)
	{
		u8 res;
		// TODO: '5420 doesn't have this
		// TODO: '5428 reads do not lock the Hidden DAC
		res = m_hidden_dac_mode;
		//m_hidden_dac_phase = 0;
		LOGMASKED(LOG_HDAC, "CL: Hidden DAC read (%02x)\n", res);
		return res;
	}

	return vga_device::ramdac_mask_r(0);
}

void cirrus_gd5428_vga_device::ramdac_hidden_mask_w(offs_t offset, u8 data)
{
	if (m_hidden_dac_phase >= 4)
	{
		// TODO: '5420 doesn't have this
		// TODO: '5428 reads do not lock the Hidden DAC
		m_hidden_dac_mode = data;
		m_hidden_dac_phase = 0;
		recompute_params();
		LOGMASKED(LOG_HDAC, "CL: Hidden DAC write %02x\n", data);
		return;
	}

	vga_device::ramdac_mask_w(0, data);
}

u8 cirrus_gd5428_vga_device::ramdac_overlay_r(offs_t offset)
{
	if(!m_ext_palette_enabled)
		return vga_device::ramdac_data_r(0);

	u8 res = 0xff;
	if (vga.dac.read && !machine().side_effects_disabled())
	{
		switch (vga.dac.state++)
		{
			case 0:
				res = m_ext_palette[vga.dac.read_index & 0x0f].red;
				break;
			case 1:
				res = m_ext_palette[vga.dac.read_index & 0x0f].green;
				break;
			case 2:
				res = m_ext_palette[vga.dac.read_index & 0x0f].blue;
				break;
		}

		if (vga.dac.state == 3)
		{
			vga.dac.state = 0;
			vga.dac.read_index++;
		}
	}

	return res;
}

void cirrus_gd5428_vga_device::ramdac_overlay_w(offs_t offset, u8 data)
{
	if(!m_ext_palette_enabled)
	{
		vga_device::ramdac_data_w(0, data);
		return;
	}

	if (!vga.dac.read)
	{
		switch (vga.dac.state++) {
		case 0:
			m_ext_palette[vga.dac.write_index & 0x0f].red=data;
			break;
		case 1:
			m_ext_palette[vga.dac.write_index & 0x0f].green=data;
			break;
		case 2:
			m_ext_palette[vga.dac.write_index & 0x0f].blue=data;
			break;
		}
		vga.dac.dirty=1;
		if (vga.dac.state==3)
		{
			vga.dac.state=0;
			vga.dac.write_index++;
		}
	}
}

void cirrus_gd5428_vga_device::crtc_map(address_map &map)
{
	svga_device::crtc_map(map);
	// VGA Vertical Blank end
	// some SVGA chipsets use all 8 bits, and this is one of them (according to MFGTST CRTC tests)
	map(0x16, 0x16).lrw8(
		NAME([this] (offs_t offset) {
			return vga.crtc.vert_blank_end & 0x00ff;
		}),
		NAME([this] (offs_t offset, u8 data) {
			vga.crtc.vert_blank_end &= ~0x00ff;
			vga.crtc.vert_blank_end |= data;
			recompute_params();
		})
	);
	map(0x19, 0x19).lrw8(
		NAME([this] (offs_t offset) {
			return m_cr19;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOGMASKED(LOG_REGS, "CR19: Interlace End %02x\n", data);
			m_cr19 = data;
		})
	);
	map(0x1a, 0x1a).lrw8(
		NAME([this] (offs_t offset) {
			return m_cr1a;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOGMASKED(LOG_REGS, "CR1A: Interlace Control %02x\n", data);
			m_cr1a = data;
			vga.crtc.horz_blank_end = (vga.crtc.horz_blank_end & 0xff3f) | ((data & 0x30) << 2);
			vga.crtc.vert_blank_end = (vga.crtc.vert_blank_end & 0xfcff) | ((data & 0xc0) << 2);
			recompute_params();
		})
	);
	map(0x1b, 0x1b).lrw8(
		NAME([this] (offs_t offset) {
			return m_cr1b;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOGMASKED(LOG_REGS, "CR1B: Extended Display Controls %02x\n", data);
			m_cr1b = data;
			vga.crtc.start_addr_latch &= ~0x070000;
			vga.crtc.start_addr_latch |= ((data & 0x01) << 16);
			vga.crtc.start_addr_latch |= ((data & 0x0c) << 15);
			vga.crtc.offset = (vga.crtc.offset & 0x00ff) | ((data & 0x10) << 4);
			recompute_params();
		})
	);
//  map(0x25, 0x25) PSR Part Status (r/o, "factory testing and internal tracking only")
	map(0x27, 0x27).lr8(
		NAME([this] (offs_t offset) {
			// NOTE: verbose
			//LOGMASKED(LOG_REGS, "CR27: Read ID\n");
			return m_chip_id;
		})
	);
}

void cirrus_gd5428_vga_device::gc_map(address_map &map)
{
	svga_device::gc_map(map);
	map(0x00, 0x00).lrw8(
		NAME([this](offs_t offset) {
			return vga.gc.set_reset & ((gc_mode_ext & 0x04) ? 0xff : 0x0f);
		}),
		NAME([this](offs_t offset, u8 data) {
			// if extended writes are enabled (bit 2 of index 0bh), then index 0 and 1 are extended to 8 bits,
			// however XFree86 does not appear to do this...
			vga.gc.set_reset = data & 0xff;
		})
	);
	map(0x01, 0x01).lrw8(
		NAME([this](offs_t offset) {
			return vga.gc.enable_set_reset & ((gc_mode_ext & 0x04) ? 0xff : 0x0f);
		}),
		NAME([this](offs_t offset, u8 data) {
			vga.gc.enable_set_reset = data & 0xff;
		})
	);
	map(0x05, 0x05).lrw8(
		NAME([this](offs_t offset) {
			u8 res = (vga.gc.shift256 & 1) << 6;
			res |= (vga.gc.shift_reg & 1) << 5;
			res |= (vga.gc.host_oe & 1) << 4;
			res |= (vga.gc.read_mode & 1) << 3;
			if(gc_mode_ext & 0x04)
				res |= (vga.gc.write_mode & 7);
			else
				res |= (vga.gc.write_mode & 3);
			return res;
		}),
		NAME([this](offs_t offset, u8 data) {
			vga.gc.shift256 = (data & 0x40) >> 6;
			vga.gc.shift_reg = (data & 0x20) >> 5;
			vga.gc.host_oe = (data & 0x10) >> 4;
			vga.gc.read_mode = (data & 8) >> 3;
			if(gc_mode_ext & 0x04)
				vga.gc.write_mode = data & 7;
			else
				vga.gc.write_mode = data & 3;
		})
	);
	// Offset register 0/1
	map(0x09, 0x0a).lrw8(
		NAME([this](offs_t offset) {
			return gc_bank[offset];
		}),
		NAME([this](offs_t offset, u8 data) {
			gc_bank[offset] = data;
			LOGMASKED(LOG_BANK, "GR%d: Offset register %d set to %02x\n", offset + 9, offset, data);
		})
	);
	// Graphics controller mode extensions
	map(0x0b, 0x0b).lrw8(
		NAME([this](offs_t offset) {
			return gc_mode_ext;
		}),
		NAME([this](offs_t offset, u8 data) {
			LOGMASKED(LOG_REGS, "GRB: Graphics Controller Mode Extensions %02x\n", data);
			gc_mode_ext = data;
			if(!(data & 0x04))
			{
				vga.gc.set_reset &= 0x0f;
				vga.gc.enable_set_reset &= 0x0f;
			}
			if(!(data & 0x08))
				vga.sequencer.map_mask &= 0x0f;
		})
	);
	// Colour Key
	// map(0x0c, 0x0c)
	// Colour Key Mask
	// map(0x0d, 0x0d)
	// Miscellaneous Control
	// map(0x0e, 0x0e)
	// Background Colour Byte 1
	map(0x10, 0x10).lrw8(
		NAME([this](offs_t offset) {
			return m_gr10;
		}),
		NAME([this](offs_t offset, u8 data) {
			m_gr10 = data;
		})
	);
	// Foreground Colour Byte 1
	map(0x11, 0x11).lrw8(
		NAME([this](offs_t offset) {
			return m_gr11;
		}),
		NAME([this](offs_t offset, u8 data) {
			m_gr11 = data;
		})
	);
	// BLT Width 0
	map(0x20, 0x20).lrw8(
		NAME([this](offs_t offset) {
			return m_blt_width & 0x00ff;
		}),
		NAME([this](offs_t offset, u8 data) {
			m_blt_width = (m_blt_width & 0xff00) | data;
			LOGMASKED(LOG_BLIT, "CL: blt_width %02x [0] (%04x)\n", data, m_blt_width);
		})
	);
	// BLT Width 1
	map(0x21, 0x21).lrw8(
		NAME([this](offs_t offset) {
			return m_blt_width >> 8;
		}),
		NAME([this](offs_t offset, u8 data) {
			m_blt_width = (m_blt_width & 0x00ff) | (data << 8);
			LOGMASKED(LOG_BLIT, "CL: blt_width %02x [1] (%04x)\n", data, m_blt_width);
		})
	);
	// BLT Height 0
	map(0x22, 0x22).lrw8(
		NAME([this](offs_t offset) {
			return m_blt_height & 0x00ff;
		}),
		NAME([this](offs_t offset, u8 data) {
			m_blt_height = (m_blt_height & 0xff00) | data;
			LOGMASKED(LOG_BLIT, "CL: m_blt_height %02x [0] (%04x)\n", data, m_blt_height);
		})
	);
	// BLT Height 1
	map(0x23, 0x23).lrw8(
		NAME([this](offs_t offset) {
			return m_blt_height >> 8;
		}),
		NAME([this](offs_t offset, u8 data) {
			m_blt_height = (m_blt_height & 0x00ff) | (data << 8);
			LOGMASKED(LOG_BLIT, "CL: m_blt_height %02x [1] (%04x)\n", data, m_blt_height);
		})
	);
	// BLT Destination Pitch 0
	map(0x24, 0x24).lrw8(
		NAME([this](offs_t offset) {
			return m_blt_dest_pitch & 0x00ff;
		}),
		NAME([this](offs_t offset, u8 data) {
			m_blt_dest_pitch = (m_blt_dest_pitch & 0xff00) | data;
		})
	);
	// BLT Destination Pitch 1
	map(0x25, 0x25).lrw8(
		NAME([this](offs_t offset) {
			return m_blt_dest_pitch >> 8;
		}),
		NAME([this](offs_t offset, u8 data) {
			m_blt_dest_pitch = (m_blt_dest_pitch & 0x00ff) | (data << 8);
		})
	);
	// BLT Source Pitch 0
	map(0x26, 0x26).lrw8(
		NAME([this](offs_t offset) {
			return m_blt_source_pitch & 0x00ff;
		}),
		NAME([this](offs_t offset, u8 data) {
			m_blt_source_pitch = (m_blt_source_pitch & 0xff00) | data;
		})
	);
	// BLT Source Pitch 1
	map(0x27, 0x27).lrw8(
		NAME([this](offs_t offset) {
			return m_blt_source_pitch >> 8;
		}),
		NAME([this](offs_t offset, u8 data) {
			m_blt_source_pitch = (m_blt_source_pitch & 0x00ff) | (data << 8);
		})
	);
	// BLT Destination start 0/1/2
	map(0x28, 0x2a).lrw8(
		NAME([this](offs_t offset) {
			return (m_blt_dest >> (8 * (offset & 3))) & 0xff;
		}),
		NAME([this](offs_t offset, u8 data) {
			const u8 byte_access = (8 * (offset & 3));
			const u32 old_mask = ~(0xff << byte_access);
			m_blt_dest = (m_blt_dest & old_mask) | (data << byte_access);
		})
	);
	// BLT source start 0/1/2
	map(0x2c, 0x2e).lrw8(
		NAME([this](offs_t offset) {
			return (m_blt_source >> (8 * (offset & 3))) & 0xff;
		}),
		NAME([this](offs_t offset, u8 data) {
			const u8 byte_access = (8 * (offset & 3));
			const u32 old_mask = ~(0xff << byte_access);
			m_blt_source = (m_blt_source & old_mask) | (data << byte_access);
		})
	);
	// BLT destination write mask (GD5430/36/40 only)
	// map(0x2f, 0x2f)
	// BLT Mode
	map(0x30, 0x30).lrw8(
		NAME([this](offs_t offset) {
			return m_blt_mode;
		}),
		NAME([this](offs_t offset, u8 data) {
			m_blt_mode = data;
		})
	);
	// BitBLT Start / Status
	map(0x31, 0x31).lrw8(
		NAME([this](offs_t offset) {
			return m_blt_status;
		}),
		NAME([this](offs_t offset, u8 data) {
			m_blt_status = data & ~0xf2;
			if (BIT(data, 2))
				m_blt_status &= ~9;
			if(data & 0x02)
			{
				if(m_blt_mode & 0x04)  // blit source is system memory
					start_system_bitblt();
				else
					start_bitblt();
			}
		})
	);
	// BitBLT ROP mode
	map(0x32, 0x32).lrw8(
		NAME([this](offs_t offset) {
			return m_blt_rop;
		}),
		NAME([this](offs_t offset, u8 data) {
			m_blt_rop = data;
		})
	);
	// BitBLT Transparent Colour 0
	map(0x34, 0x34).lrw8(
		NAME([this](offs_t offset) {
			return m_blt_trans_colour & 0xff;
		}),
		NAME([this](offs_t offset, u8 data) {
			m_blt_trans_colour = (m_blt_trans_colour & 0xff00) | data;
		})
	);
	// BitBLT Transparent Colour 1
	map(0x35, 0x35).lrw8(
		NAME([this](offs_t offset) {
			return m_blt_trans_colour >> 8;
		}),
		NAME([this](offs_t offset, u8 data) {
			m_blt_trans_colour = (m_blt_trans_colour & 0x00ff) | (data << 8);
		})
	);
	// BitBLT Transparent Colour Mask 0
	map(0x36, 0x36).lrw8(
		NAME([this](offs_t offset) {
			return m_blt_trans_colour_mask & 0xff;
		}),
		NAME([this](offs_t offset, u8 data) {
			m_blt_trans_colour_mask = (m_blt_trans_colour_mask & 0xff00) | data;
		})
	);
	// BitBLT Transparent Colour Mask 1
	map(0x37, 0x37).lrw8(
		NAME([this](offs_t offset) {
			return m_blt_trans_colour_mask >> 8;
		}),
		NAME([this](offs_t offset, u8 data) {
			m_blt_trans_colour_mask = (m_blt_trans_colour_mask & 0x00ff) | (data << 8);
		})
	);
}

void cirrus_gd5428_vga_device::sequencer_map(address_map &map)
{
	svga_device::sequencer_map(map);
	map(0x02, 0x02).lrw8(
		NAME([this] (offs_t offset) {
			return vga.sequencer.map_mask & ((gc_mode_ext & 0x08) ? 0xff : 0x0f);
		}),
		NAME([this] (offs_t offset, u8 data) {
			vga.sequencer.map_mask = data & ((gc_mode_ext & 0x08) ? 0xff : 0x0f);
		})
	);
	map(0x06, 0x06).lrw8(
		NAME([this] (offs_t offset) {
			return (gc_locked) ? 0x0f : m_lock_reg;
		}),
		NAME([this] (offs_t offset, u8 data) {
			// TODO: extensions are always enabled on the GD5429
			// bits 3,5,6,7 ignored

			gc_locked = (data & 0x17) != 0x12;
			LOG("Cirrus register extensions %s\n", gc_locked ? "unlocked" : "locked");
			m_lock_reg = data & 0x17;
			recompute_params();
		})
	);
	map(0x07, 0x07).lrw8(
		NAME([this] (offs_t offset) {
			return vga.sequencer.data[0x07];
		}),
		NAME([this] (offs_t offset, u8 data) {
			// xxxx ---- Memory Segment
			// 0000 ---- VGA segmenting
			// llll ---- (any other value) Linear addressing
			// ---- -xx- CRTC Character Clock Divider
			// ---- ---x Select 8bpp High Resolution Mode
			vga.sequencer.data[0x07] = data;
			LOGMASKED(LOG_REGS, "SR7: Extended Sequencer Mode %02x\n", data);
			recompute_params();
		})
	);
	// TODO: check me
	map(0x09, 0x09).lrw8(
		NAME([this] (offs_t offset) {
			return vga.sequencer.data[0x09];
		}),
		NAME([this] (offs_t offset, u8 data) {
			vga.sequencer.data[0x09] = data;
		})
	);
	map(0x0a, 0x0a).lrw8(
		NAME([this] (offs_t offset) {
			return m_scratchpad1;
		}),
		NAME([this] (offs_t offset, u8 data) {
			// GD5402/GD542x BIOS writes VRAM size here
			m_scratchpad1 = data;
		})
	);
	map(0x0b, 0x0e).lrw8(
		NAME([this] (offs_t offset) {
			return m_vclk_num[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_vclk_num[offset] = data;
			recompute_params();
		})
	);
	map(0x0f, 0x0f).lrw8(
		NAME([this] (offs_t offset) {
			u8 res = vga.sequencer.data[0x0f] & 0xe7;
			// 32-bit DRAM data bus width (1MB-2MB)
			res |= 0x18;
			return res;
		}),
		NAME([this] (offs_t offset, u8 data) {
			vga.sequencer.data[0x0f] = data;
		})
	);
	// bits 5-7 of the register index are the low bits of the X co-ordinate
	map(0x10, 0x10).select(0xe0).lw8(
		NAME([this] (offs_t offset, u8 data) {
			m_cursor_x = (data << 3) | ((offset & 0xe0) >> 5);
		})
	);
	// bits 5-7 of the register index are the low bits of the Y co-ordinate
	map(0x11, 0x11).select(0xe0).lw8(
		NAME([this] (offs_t offset, u8 data) {
			m_cursor_y = (data << 3) | ((offset & 0xe0) >> 5);
		})
	);
	map(0x12, 0x12).lrw8(
		NAME([this] (offs_t offset) {
			return m_cursor_attr;
		}),
		NAME([this] (offs_t offset, u8 data) {
			// bit 0 - enable cursor
			// bit 1 - enable extra palette (cursor colours are there)
			// bit 2 - 64x64 cursor (32x32 if clear, GD5422+)
			// bit 7 - overscan colour protect - if set, use colour 2 in the extra palette for the border (GD5424+)
			m_cursor_attr = data;
			m_ext_palette_enabled = data & 0x02;
		})
	);
	map(0x13, 0x13).lw8(
		NAME([this] (offs_t offset, u8 data) {
			// bits 0 and 1 are ignored if using 64x64 cursor
			m_cursor_addr = data;
		})
	);
	map(0x14, 0x14).lrw8(
		NAME([this] (offs_t offset) {
			return m_scratchpad2;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_scratchpad2 = data;
		})
	);
	map(0x15, 0x15).lrw8(
		NAME([this] (offs_t offset) {
			return m_scratchpad3;
		}),
		NAME([this] (offs_t offset, u8 data) {
			// GD543x BIOS writes VRAM size here
			m_scratchpad3 = data;
		})
	);
	map(0x1b, 0x1e).lrw8(
		NAME([this] (offs_t offset) {
			return m_vclk_denom[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_vclk_denom[offset] = data;
			recompute_params();
		})
	);
}

void cirrus_gd5428_vga_device::device_start()
{
	svga_device::device_start();
	zero();

	for (int i = 0; i < 0x100; i++)
		set_pen_color(i, 0, 0, 0);

	// Avoid an infinite loop when displaying.  0 is not possible anyway.
	vga.crtc.maximum_scan_line = 1;

	// copy over interfaces
	//vga.memory = std::make_unique<uint8_t []>(vga.svga_intf.vram_size);
	//memset(&vga.memory[0], 0, vga.svga_intf.vram_size);

	//save_pointer(NAME(vga.memory), vga.svga_intf.vram_size);
	save_pointer(vga.crtc.data,"CRTC Registers",0x100);
	save_pointer(vga.sequencer.data,"Sequencer Registers",0x100);
	save_pointer(vga.attribute.data,"Attribute Registers", 0x15);
	save_item(NAME(m_chip_id));
	save_item(NAME(m_hidden_dac_phase));
	save_item(NAME(m_hidden_dac_mode));
	save_pointer(NAME(gc_bank), 2);

	m_vblank_timer = timer_alloc(FUNC(cirrus_gd5428_vga_device::vblank_timer_cb), this);

	m_chip_id = 0x98;  // GD5428 - Rev 0
}

void cirrus_gd5430_vga_device::device_start()
{
	cirrus_gd5428_vga_device::device_start();
	m_chip_id = 0xa0;  // GD5430 - Rev 0
}

void cirrus_gd5446_vga_device::device_start()
{
	cirrus_gd5428_vga_device::device_start();
	m_chip_id = 0x80 | 0x39;  // GD5446
}


void cirrus_gd5428_vga_device::device_reset()
{
	svga_device::device_reset();
	gc_locked = true;
	gc_mode_ext = 0;
	gc_bank[0] = gc_bank[1] = 0;
	m_lock_reg = 0;
	m_blt_status = 0;
	m_cursor_attr = 0x00;  // disable hardware cursor and extra palette
	m_cursor_x = m_cursor_y = 0;
	m_cursor_addr = 0;
	m_scratchpad1 = m_scratchpad2 = m_scratchpad3 = 0;
	m_cr19 = m_cr1a = m_cr1b = 0;
	m_vclk_num[0] = 0x4a;
	m_vclk_denom[0] = 0x2b;
	m_vclk_num[1] = 0x5b;
	m_vclk_denom[1] = 0x2f;
	m_blt_source = m_blt_dest = m_blt_source_current = m_blt_dest_current = 0;
	memset(m_ext_palette, 0, sizeof(m_ext_palette));
	m_ext_palette_enabled = false;
	m_blt_system_transfer = false;
	m_hidden_dac_phase = 0;
	m_hidden_dac_mode = 0;
}

uint32_t cirrus_gd5428_vga_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint32_t ptr = (vga.svga_intf.vram_size - 0x4000);  // cursor patterns are stored in the last 16kB of VRAM
	svga_device::screen_update(screen, bitmap, cliprect);

	if(m_cursor_attr & 0x01)  // hardware cursor enabled
	{
		// draw hardware graphics cursor
		if(m_cursor_attr & 0x04)  // 64x64
		{
			ptr += ((m_cursor_addr & 0x3c) * 256);
			for(int y=0;y<64;y++)
			{
				for(int x=0;x<64;x+=8)
				{
					for(int bit=0;bit<8;bit++)
					{
						uint8_t pixel1 = vga.memory[ptr % vga.svga_intf.vram_size] >> (7-bit);
						uint8_t pixel2 = vga.memory[(ptr+512) % vga.svga_intf.vram_size] >> (7-bit);
						uint8_t output = ((pixel1 & 0x01) << 1) | (pixel2 & 0x01);
						switch(output)
						{
						case 0:  // transparent - do nothing
							break;
						case 1:  // background
							bitmap.pix(m_cursor_y+y,m_cursor_x+x+bit) = (m_ext_palette[0].red << 16) | (m_ext_palette[0].green << 8) | (m_ext_palette[0].blue);
							break;
						case 2:  // XOR
							bitmap.pix(m_cursor_y+y,m_cursor_x+x+bit) = ~bitmap.pix(m_cursor_y+y,m_cursor_x+x+bit);
							break;
						case 3:  // foreground
							bitmap.pix(m_cursor_y+y,m_cursor_x+x+bit) = (m_ext_palette[15].red << 16) | (m_ext_palette[15].green << 8) | (m_ext_palette[15].blue);
							break;
						}
					}
				}
			}
		}
		else
		{
			ptr += ((m_cursor_addr & 0x3f) * 256);
			for(int y=0;y<32;y++)
			{
				for(int x=0;x<32;x+=8)
				{
					for(int bit=0;bit<8;bit++)
					{
						uint8_t pixel1 = vga.memory[ptr % vga.svga_intf.vram_size] >> (7-bit);
						uint8_t pixel2 = vga.memory[(ptr+128) % vga.svga_intf.vram_size] >> (7-bit);
						uint8_t output = ((pixel1 & 0x01) << 1) | (pixel2 & 0x01);
						switch(output)
						{
						case 0:  // transparent - do nothing
							break;
						case 1:  // background
							bitmap.pix(m_cursor_y+y,m_cursor_x+x+bit) = (m_ext_palette[0].red << 18) | (m_ext_palette[0].green << 10) | (m_ext_palette[0].blue << 2);
							break;
						case 2:  // XOR
							bitmap.pix(m_cursor_y+y,m_cursor_x+x+bit) = ~bitmap.pix(m_cursor_y+y,m_cursor_x+x+bit);
							break;
						case 3:  // foreground
							bitmap.pix(m_cursor_y+y,m_cursor_x+x+bit) = (m_ext_palette[15].red << 18) | (m_ext_palette[15].green << 10) | (m_ext_palette[15].blue << 2);
							break;
						}
					}
					ptr++;
				}
			}
		}
	}
	return 0;
}

void cirrus_gd5428_vga_device::recompute_params()
{
	uint8_t divisor = 1;
	float clock;
	// TODO: coming from OSC, expose as this->clock()
	const XTAL xtal = XTAL(14'318'181);
	uint8_t clocksel = (vga.miscellaneous_output & 0xc) >> 2;

	if(gc_locked || m_vclk_num[clocksel] == 0 || m_vclk_denom[clocksel] == 0)
		clock = ((vga.miscellaneous_output & 0xc) ? xtal*2: xtal*1.75).dvalue();
	else
	{
		int numerator = m_vclk_num[clocksel] & 0x7f;
		int denominator = (m_vclk_denom[clocksel] & 0x3e) >> 1;
		int mul = m_vclk_denom[clocksel] & 0x01 ? 2 : 1;
		clock = (xtal * numerator / denominator / mul).dvalue();
		LOGMASKED(LOG_PLL, "CL: PLL setting %d num %d denom %d mul %d -> %f\n", clocksel, numerator, denominator, mul, clock);
	}

	svga.rgb8_en = svga.rgb15_en = svga.rgb16_en = svga.rgb24_en = svga.rgb32_en = 0;

	if (!gc_locked)
	{
		// gambl186 relies on this, don't setup any hidden DAC but only this
		if (vga.sequencer.data[0x07] & 0x01)
			svga.rgb8_en = 1;

		if (BIT(m_hidden_dac_mode, 7))
		{
			// TODO: needs subclassing, earlier chips don't have all of these modes
			if (BIT(m_hidden_dac_mode, 4))
				popmessage("pc_vga_cirrus: Unsupported mixed 5-5-5 / 8bpp mode selected");
			switch(m_hidden_dac_mode & 0x4f)
			{
				case 0x00:
				case 0x40:
					// 5-5-5 Sierra
					svga.rgb15_en = 1;
					break;
				case 0x41:
					svga.rgb16_en = 1;
					break;
				case 0x43: // CCIR601 YUV422 16-bit
				case 0x44: // YUV411 8-bit
				case 0x4a: // 16bpp + YUV422 overlay
				case 0x4b: // 16bpp + YUV411 overlay
					popmessage("pc_vga_cirrus: CL-GD545 YUV mode selected %02x", m_hidden_dac_mode);
					break;
				case 0x45:
					svga.rgb24_en = 1;
					break;
				case 0x46:
				case 0x47:
					popmessage("pc_vga_cirrus: CL-GD545+ DAC power down selected %02x", m_hidden_dac_mode);
					break;
				case 0x48:
					popmessage("pc_vga_cirrus: CL-GD545+ 8-bit grayscale selected");
					break;
				case 0x49:
					svga.rgb8_en = 1;
					break;
				default:
					// TODO: 0xff in pciagp (alias for a DAC power down?)
					popmessage("pc_vga_cirrus: reserved mode selected %02x", m_hidden_dac_mode);
					break;
			}
		}

		switch(vga.sequencer.data[0x07] & 0x06)  // bit 3 is reserved on GD542x
		{
			case 0x00: break;
			case 0x02: clock /= 2; break;  // Clock / 2 for 16-bit data
			case 0x04: clock /= 3; break; // Clock / 3 for 24-bit data
			case 0x06:
				// Clock rate for 16-bit data = VCLK
				// TODO: verify clock, may just be clock / 2 again?
				//divisor = 2;
				break;
		}
	}
	recompute_params_clock(divisor, (int)clock);
}

uint16_t cirrus_gd5428_vga_device::offset()
{
	// TODO: check true enable condition
	if (svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en || svga.rgb32_en)
		return vga.crtc.offset << 3;
	return svga_device::offset();
}

uint32_t cirrus_gd5428_vga_device::latch_start_addr()
{
	if (svga.rgb24_en || svga.rgb32_en)
		return vga.crtc.start_addr_latch >> 1;

	// FIXME: need to explicitly return earlier because rgb8_en is '1' in tandem with these
	if (svga.rgb15_en || svga.rgb16_en)
		return vga.crtc.start_addr_latch;

	if (svga.rgb8_en)
		return vga.crtc.start_addr_latch << 2;
	return vga.crtc.start_addr_latch;
}

void cirrus_gd5428_vga_device::start_bitblt()
{
	uint32_t x,y;

	if(m_blt_mode & 0x01)
	{
		start_reverse_bitblt();
		return;
	}

	LOGMASKED(LOG_BLIT, "CL: BitBLT started: Src: %06x Dst: %06x Width: %i Height %i ROP: %02x Mode: %02x\n",m_blt_source,m_blt_dest,m_blt_width,m_blt_height,m_blt_rop,m_blt_mode);

	m_blt_source_current = m_blt_source;
	m_blt_dest_current = m_blt_dest;

	for(y=0;y<=m_blt_height;y++)
	{
		for(x=0;x<=m_blt_width;x++)
		{
			if(m_blt_mode & 0x80)  // colour expand
			{
				if(m_blt_mode & 0x10)  // 16-bit colour expansion / transparency width
				{
					// use GR0/1/10/11 background/foreground regs
					uint16_t pixel = (vga.memory[m_blt_source_current % vga.svga_intf.vram_size] >> (7-((x/2) % 8)) & 0x01) ? ((m_gr11 << 8) | vga.gc.enable_set_reset) : ((m_gr10 << 8) | vga.gc.set_reset);

					if(m_blt_dest_current & 1)
						copy_pixel(pixel >> 8, vga.memory[m_blt_dest_current % vga.svga_intf.vram_size]);
					else
						copy_pixel(pixel & 0xff, vga.memory[m_blt_dest_current % vga.svga_intf.vram_size]);
					if((x % 8) == 7 && !(m_blt_mode & 0x40))  // don't increment if a pattern (it's only 8 bits)
						m_blt_source_current++;
				}
				else
				{
					uint8_t pixel = (vga.memory[m_blt_source_current % vga.svga_intf.vram_size] >> (7-(x % 8)) & 0x01) ? vga.gc.enable_set_reset : vga.gc.set_reset;  // use GR0/1/10/11 background/foreground regs

					copy_pixel(pixel, vga.memory[m_blt_dest_current % vga.svga_intf.vram_size]);
					if((x % 8) == 7 && !(m_blt_mode & 0x40))  // don't increment if a pattern (it's only 8 bits)
						m_blt_source_current++;
				}
			}
			else
			{
				copy_pixel(vga.memory[m_blt_source_current % vga.svga_intf.vram_size], vga.memory[m_blt_dest_current % vga.svga_intf.vram_size]);
				m_blt_source_current++;
			}

			m_blt_dest_current++;
			if(m_blt_mode & 0x40 && (x % 8) == 7)  // 8x8 pattern - reset pattern source location
			{
				if(m_blt_mode & 0x80) // colour expand
					m_blt_source_current = m_blt_source + (1*(y % 8)); // patterns are linear data
				else if(svga.rgb15_en || svga.rgb16_en)
				{
					if(m_blt_mode & 0x40 && (x % 16) == 15)
						m_blt_source_current = m_blt_source + (16*(y % 8));
				}
				else
					m_blt_source_current = m_blt_source + (8*(y % 8));
			}
		}
		if(m_blt_mode & 0x40)  // 8x8 pattern
		{
			if(m_blt_mode & 0x80) // colour expand
				m_blt_source_current = m_blt_source + (1*(y % 8)); // patterns are linear data
			else if(svga.rgb15_en || svga.rgb16_en)
			{
				if(m_blt_mode & 0x40 && (x % 16) == 15)
					m_blt_source_current = m_blt_source + (16*(y % 8));
			}
			else
				m_blt_source_current = m_blt_source + (8*(y % 8));
		}
		else
			m_blt_source_current = m_blt_source + (m_blt_source_pitch*(y+1));
		m_blt_dest_current = m_blt_dest + (m_blt_dest_pitch*(y+1));
	}
	m_blt_status &= ~0x09;
}

void cirrus_gd5428_vga_device::start_reverse_bitblt()
{
	uint32_t x,y;

	LOGMASKED(LOG_BLIT, "CL: Reverse BitBLT started: Src: %06x Dst: %06x Width: %i Height %i ROP: %02x Mode: %02x\n",m_blt_source,m_blt_dest,m_blt_width,m_blt_height,m_blt_rop,m_blt_mode);

	// Start at end of blit
	m_blt_source_current = m_blt_source;
	m_blt_dest_current = m_blt_dest;

	for(y=0;y<=m_blt_height;y++)
	{
		for(x=0;x<=m_blt_width;x++)
		{
			if(m_blt_mode & 0x80)  // colour expand
			{
				if(m_blt_mode & 0x10)  // 16-bit colour expansion / transparency width
				{
					// use GR0/1/10/11 background/foreground regs
					uint16_t pixel = (vga.memory[m_blt_source_current % vga.svga_intf.vram_size] >> (7-((x/2) % 8)) & 0x01) ? ((m_gr11 << 8) | vga.gc.enable_set_reset) : ((m_gr10 << 8) | vga.gc.set_reset);

					if(m_blt_dest_current & 1)
						copy_pixel(pixel >> 8, vga.memory[m_blt_dest_current % vga.svga_intf.vram_size]);
					else
						copy_pixel(pixel & 0xff, vga.memory[m_blt_dest_current % vga.svga_intf.vram_size]);
					if((x % 8) == 7 && !(m_blt_mode & 0x40))  // don't increment if a pattern (it's only 8 bits)
						m_blt_source_current--;
				}
				else
				{
				uint8_t pixel = (vga.memory[m_blt_source_current % vga.svga_intf.vram_size] >> (7-(x % 8)) & 0x01) ? vga.gc.enable_set_reset : vga.gc.set_reset;  // use GR0/1/10/11 background/foreground regs

				copy_pixel(pixel, vga.memory[m_blt_dest_current % vga.svga_intf.vram_size]);
				if((x % 8) == 7 && !(m_blt_mode & 0x40))  // don't decrement if a pattern (it's only 8 bits)
					m_blt_source_current--;
				}
			}
			else
			{
				copy_pixel(vga.memory[m_blt_source_current % vga.svga_intf.vram_size], vga.memory[m_blt_dest_current % vga.svga_intf.vram_size]);
				m_blt_source_current--;
			}
			m_blt_dest_current--;
			if(m_blt_mode & 0x40 && (x % 8) == 7)  // 8x8 pattern - reset pattern source location
			{
				if(m_blt_mode & 0x80) // colour expand
					m_blt_source_current = m_blt_source - (1*(y % 8)); // patterns are linear data
				else if(svga.rgb15_en || svga.rgb16_en)
				{
					if(m_blt_mode & 0x40 && (x % 16) == 15)
						m_blt_source_current = m_blt_source - (16*(y % 8));
				}
				else
					m_blt_source_current = m_blt_source - (8*(y % 8));
			}
		}
		if(m_blt_mode & 0x40)  // 8x8 pattern
		{
			if(m_blt_mode & 0x80) // colour expand
				m_blt_source_current = m_blt_source - (1*(y % 8)); // patterns are linear data
			else if(svga.rgb15_en || svga.rgb16_en)
				{
					if(m_blt_mode & 0x40 && (x % 16) == 15)
						m_blt_source_current = m_blt_source - (16*(y % 8));
				}
			else
				m_blt_source_current = m_blt_source - (8*(y % 8));
		}
		else
			m_blt_source_current = m_blt_source - (m_blt_source_pitch*(y+1));
		m_blt_dest_current = m_blt_dest - (m_blt_dest_pitch*(y+1));
	}
	m_blt_status &= ~0x09;
}

void cirrus_gd5428_vga_device::start_system_bitblt()
{
	LOGMASKED(LOG_BLIT, "CL: BitBLT from system memory started: Src: %06x Dst: %06x Width: %i Height %i ROP: %02x Mode: %02x\n",m_blt_source,m_blt_dest,m_blt_width,m_blt_height,m_blt_rop,m_blt_mode);
	m_blt_system_transfer = true;
	m_blt_system_count = 0;
	m_blt_system_buffer = 0;
	m_blt_pixel_count = m_blt_scan_count = 0;
	m_blt_source_current = m_blt_source;
	m_blt_dest_current = m_blt_dest;
	m_blt_status |= 0x09;
}

// non colour-expanded BitBLTs from system memory must be doubleword sized, extra bytes are ignored
void cirrus_gd5428_vga_device::blit_dword()
{
	// TODO: add support for reverse direction
	uint8_t x,pixel;

	for(x=0;x<32;x+=8)
	{
		pixel = ((m_blt_system_buffer & (0x000000ff << x)) >> x);
		if(m_blt_pixel_count <= m_blt_width)
			copy_pixel(pixel,vga.memory[m_blt_dest_current % vga.svga_intf.vram_size]);
		m_blt_dest_current++;
		m_blt_pixel_count++;
	}
	if(m_blt_pixel_count > m_blt_width)
	{
		m_blt_pixel_count = 0;
		m_blt_scan_count++;
		m_blt_dest_current = m_blt_dest + (m_blt_dest_pitch*m_blt_scan_count);
	}
	if(m_blt_scan_count > m_blt_height)
	{
		m_blt_system_transfer = false;  //  BitBLT complete
		m_blt_status &= ~0x0b;
	}
}

// colour-expanded BitBLTs from system memory are on a byte boundary, unused bits are ignored
void cirrus_gd5428_vga_device::blit_byte()
{
	// TODO: add support for reverse direction
	uint8_t x,pixel;

	for(x=0;x<8;x++)
	{
		// use GR0/1/10/11 background/foreground regs
		if(m_blt_dest_current & 1)
			pixel = ((m_blt_system_buffer & (0x00000001 << (7-x))) >> (7-x)) ? m_gr11 : m_gr10;
		else
			pixel = ((m_blt_system_buffer & (0x00000001 << (7-x))) >> (7-x)) ? vga.gc.enable_set_reset : vga.gc.set_reset;
		if(m_blt_pixel_count <= m_blt_width - 1)
			copy_pixel(pixel,vga.memory[m_blt_dest_current % vga.svga_intf.vram_size]);
		m_blt_dest_current++;
		m_blt_pixel_count++;
	}
	if(m_blt_pixel_count > m_blt_width)
	{
		m_blt_pixel_count = 0;
		m_blt_scan_count++;
		m_blt_dest_current = m_blt_dest + (m_blt_dest_pitch*m_blt_scan_count);
	}
	if(m_blt_scan_count > m_blt_height)
	{
		m_blt_system_transfer = false;  //  BitBLT complete
		m_blt_status &= ~0x0b;
	}
}

void cirrus_gd5428_vga_device::copy_pixel(uint8_t src, uint8_t dst)
{
	uint8_t res = src;

	switch(m_blt_rop)
	{
	case 0x00:  // BLACK
		res = 0x00;
		break;
	case 0x0b:  // DSTINVERT
		res = ~dst;
		break;
	case 0x0d:  // SRC
		res = src;
		break;
	case 0x0e:  // WHITE
		res = 0xff;
		break;
	case 0x50:  // DSna / DPna
		// used by zorro2:picasso2p, unknown purpose
		res = (dst & (~src));
		break;
	case 0x59:  // SRCINVERT
		res = src ^ dst;
		break;
	case 0x6d:  // SRCPAINT / DSo
		// zorro2:picasso2p on VGA Workbench (upper right icon)
		res = src | dst;
		break;
	default:
		popmessage("pc_vga_cirrus: Unsupported BitBLT ROP mode %02x",m_blt_rop);
	}

	// handle transparency compare
	if(m_blt_mode & 0x08)  // TODO: 16-bit compare
	{
		// if ROP result matches the transparency colour, don't change the pixel
		if((res & (~m_blt_trans_colour_mask & 0xff)) == ((m_blt_trans_colour & 0xff) & (~m_blt_trans_colour_mask & 0xff)))
			return;
	}

	vga.memory[m_blt_dest_current % vga.svga_intf.vram_size] = res;
}

uint8_t cirrus_gd5428_vga_device::vga_latch_write(int offs, uint8_t data)
{
	uint8_t res = 0;
	uint8_t mode_mask = (gc_mode_ext & 0x04) ? 0x07 : 0x03;

	switch (vga.gc.write_mode & mode_mask) {
	case 0:
	case 1:
	case 2:
	case 3:
		res = vga_device::vga_latch_write(offs, data);
		break;
	case 4:
		res = vga.gc.latch[offs];
		popmessage("pc_vga_cirrus: Unimplemented VGA write mode 4 enabled");
		break;
	case 5:
		res = vga.gc.latch[offs];
		popmessage("pc_vga_cirrus: Unimplemented VGA write mode 5 enabled");
		break;
	}

	return res;
}

// 0xa0000-0xa7fff offset 0
// 0xa8000-0xaffff offset 1 (if enabled with GRB bit 0)
// notice that "offset" in this context doesn't mean pitch like everything else in the
// (S)VGA realm but it's really intended as a window bank base here.
uint8_t cirrus_gd5428_vga_device::offset_select(offs_t offset)
{
	const uint8_t sa15 = BIT(offset, 15);
	return gc_bank[sa15 & BIT(gc_mode_ext, 0)];
}

uint8_t cirrus_gd5428_vga_device::mem_r(offs_t offset)
{
	uint32_t addr;
	uint8_t cur_mode = pc_vga_choosevideomode();

	const uint8_t bank = offset_select(offset);

	// TODO: incomplete, just enough for zorro2:picasso2p to not outright crash at display init
	// value should be a base to apply, also GRB[5] remaps banking granularity
	// NOTE: bebox also wants this
	if(vga.sequencer.data[0x07] & 0xf0)
	{
		return svga_device::mem_linear_r((offset & 0xffff) + bank * 0x10000);
	}

	if(gc_locked || offset >= 0x10000 || cur_mode == TEXT_MODE || cur_mode == SCREEN_OFF)
	{
		return vga_device::mem_r(offset & 0x1ffff);
	}

	if(gc_mode_ext & 0x20)  // 16kB bank granularity
		addr = bank * 0x4000;
	else  // 4kB bank granularity
		addr = bank * 0x1000;

	// Is the display address adjusted automatically when not using Chain-4 addressing?
	// The GD542x BIOS doesn't do it, but Virtual Pool expects it.
	if(!(vga.sequencer.data[4] & 0x8))
		addr <<= 2;

	if(svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en)
	{
		uint8_t data = 0;
		if(gc_mode_ext & 0x01)
		{
			if(offset & 0x10000)
				return 0;
			if(offset < 0x8000)
				offset &= 0x7fff;
			else
			{
				offset -= 0x8000;
				offset &= 0x7fff;
			}
		}
		else
			offset &= 0xffff;

		if(vga.sequencer.data[4] & 0x8)
			data = vga.memory[(offset+addr) % vga.svga_intf.vram_size];
		else
		{
			{
				int i;

				for(i=0;i<4;i++)
				{
					if(vga.sequencer.map_mask & 1 << i)
						data |= vga.memory[((offset*4+i)+addr) % vga.svga_intf.vram_size];
				}
			}
		return data;
		}
	}

	switch(vga.gc.memory_map_sel & 0x03)
	{
		case 0: break;
		case 1: if(gc_mode_ext & 0x01) offset &= 0x7fff; else offset &= 0x0ffff; break;
		case 2: offset -= 0x10000; offset &= 0x07fff; break;
		case 3: offset -= 0x18000; offset &= 0x07fff; break;
	}

	if(vga.sequencer.data[4] & 4)
	{
		int data;
		if (!machine().side_effects_disabled())
		{
			vga.gc.latch[0]=vga.memory[(offset+addr) % vga.svga_intf.vram_size];
			vga.gc.latch[1]=vga.memory[((offset+addr)+0x10000) % vga.svga_intf.vram_size];
			vga.gc.latch[2]=vga.memory[((offset+addr)+0x20000) % vga.svga_intf.vram_size];
			vga.gc.latch[3]=vga.memory[((offset+addr)+0x30000) % vga.svga_intf.vram_size];
		}

		if (vga.gc.read_mode)
		{
			uint8_t byte,layer;
			uint8_t fill_latch;
			data=0;

			for(byte=0;byte<8;byte++)
			{
				fill_latch = 0;
				for(layer=0;layer<4;layer++)
				{
					if(vga.gc.latch[layer] & 1 << byte)
						fill_latch |= 1 << layer;
				}
				fill_latch &= vga.gc.color_dont_care;
				if(fill_latch == vga.gc.color_compare)
					data |= 1 << byte;
			}
		}
		else
			data=vga.gc.latch[vga.gc.read_map_sel];

		return data;
	}
	else
	{
		// TODO: Lines up in 16-colour mode, likely different for 256-colour modes (docs say video addresses are shifted right 3 places)
		uint8_t i,data;
//      uint8_t bits = ((gc_mode_ext & 0x08) && (vga.gc.write_mode == 1)) ? 8 : 4;

		data = 0;
		//printf("%08x\n",offset);

		if(gc_mode_ext & 0x02)
		{
			for(i=0;i<8;i++)
			{
				if(vga.sequencer.map_mask & 1 << i)
					data |= vga.memory[(((offset+addr))+i*0x10000) % vga.svga_intf.vram_size];
			}
		}
		else
		{
			for(i=0;i<4;i++)
			{
				if(vga.sequencer.map_mask & 1 << i)
					data |= vga.memory[(((offset+addr))+i*0x10000) % vga.svga_intf.vram_size];
			}
		}

		return data;
	}
}

void cirrus_gd5428_vga_device::mem_w(offs_t offset, uint8_t data)
{
	uint32_t addr;
	uint8_t cur_mode = pc_vga_choosevideomode();

	if(m_blt_system_transfer)
	{
		if(m_blt_mode & 0x80)  // colour expand
		{
			m_blt_system_buffer &= ~(0x000000ff);
			m_blt_system_buffer |= data;
			blit_byte();
			m_blt_system_count = 0;
		}
		else
		{
			m_blt_system_buffer &= ~(0x000000ff << (m_blt_system_count * 8));
			m_blt_system_buffer |= (data << (m_blt_system_count * 8));
			m_blt_system_count++;
			if(m_blt_system_count >= 4)
			{
				blit_dword();
				m_blt_system_count = 0;
			}
		}
		return;
	}

	const uint8_t bank = offset_select(offset);

	// TODO: as above
	if(vga.sequencer.data[0x07] & 0xf0)
	{
		svga_device::mem_linear_w((offset + bank * 0x10000), data);
		return;
	}

	if(gc_locked || offset >= 0x10000 || cur_mode == TEXT_MODE || cur_mode == SCREEN_OFF)
	{
		vga_device::mem_w(offset & 0x1ffff,data);
		return;
	}

	if(gc_mode_ext & 0x20)  // 16kB bank granularity
		addr = bank * 0x4000;
	else  // 4kB bank granularity
		addr = bank * 0x1000;

	// Is the display address adjusted automatically when using Chain-4 addressing?  The GD542x BIOS doesn't do it, but Virtual Pool expects it.
	if(!(vga.sequencer.data[4] & 0x8))
		addr <<= 2;

	if(svga.rgb8_en || svga.rgb15_en || svga.rgb16_en || svga.rgb24_en)
	{
		if(offset & 0x10000)
			return;
		if(gc_mode_ext & 0x01)
		{
			if(offset < 0x8000)
				offset &= 0x7fff;
			else
			{
				offset -= 0x8000;
				offset &= 0x7fff;
			}
		}
		else
			offset &= 0xffff;

		// GR0 (and GR10 in 15/16bpp modes) = background colour in write mode 5
		// GR1 (and GR11 in 15/16bpp modes) = foreground colour in write modes 4 or 5
		if(vga.gc.write_mode == 4)
		{
			int i;

			for(i=0;i<8;i++)
			{
				if(svga.rgb8_en)
				{
					if(data & (0x01 << (7-i)))
						vga.memory[((addr+offset)*8+i) % vga.svga_intf.vram_size] = vga.gc.enable_set_reset;
				}
				else if(svga.rgb15_en || svga.rgb16_en)
				{
					if(data & (0x01 << (7-i)))
					{
						vga.memory[((addr+offset)*16+(i*2)) % vga.svga_intf.vram_size] = vga.gc.enable_set_reset;
						vga.memory[((addr+offset)*16+(i*2)+1) % vga.svga_intf.vram_size] = m_gr11;
					}
				}
			}
			return;
		}

		if(vga.gc.write_mode == 5)
		{
			int i;

			for(i=0;i<8;i++)
			{
				if(svga.rgb8_en)
				{
					if(data & (0x01 << (7-i)))
						vga.memory[((addr+offset)*8+i) % vga.svga_intf.vram_size] = vga.gc.enable_set_reset;
					else
						vga.memory[((addr+offset)*8+i) % vga.svga_intf.vram_size] = vga.gc.set_reset;
				}
				else if(svga.rgb15_en || svga.rgb16_en)
				{
					if(data & (0x01 << (7-i)))
					{
						vga.memory[((addr+offset)*16+(i*2)) % vga.svga_intf.vram_size] = vga.gc.enable_set_reset;
						vga.memory[((addr+offset)*16+(i*2)+1) % vga.svga_intf.vram_size] = m_gr11;
					}
					else
					{
						vga.memory[((addr+offset)*16+(i*2)) % vga.svga_intf.vram_size] = vga.gc.set_reset;
						vga.memory[((addr+offset)*16+(i*2)+1) % vga.svga_intf.vram_size] = m_gr10;
					}
				}
			}
			return;
		}

		if(vga.sequencer.data[4] & 0x8)
			vga.memory[(offset+addr) % vga.svga_intf.vram_size] = data;
		else
		{
			int i;
			for(i=0;i<4;i++)
			{
				if(vga.sequencer.map_mask & 1 << i)
					vga.memory[((offset*4+i)+addr) % vga.svga_intf.vram_size] = data;
			}
		}
	}
	else
	{
		//Inside each case must prevent writes to non-mapped VGA memory regions, not only mask the offset.
		switch(vga.gc.memory_map_sel & 0x03)
		{
			case 0: break;
			case 1:
				if(offset & 0x10000)
					return;

				if(gc_mode_ext & 0x01)
					offset &= 0x7fff;
				else
					offset &= 0xffff;
				break;
			case 2:
				if((offset & 0x18000) != 0x10000)
					return;

				offset &= 0x07fff;
				break;
			case 3:
				if((offset & 0x18000) != 0x18000)
					return;

				offset &= 0x07fff;
				break;
		}

		{
		// TODO: Lines up in 16-colour mode, likely different for 256-colour modes (docs say video addresses are shifted right 3 places)
			uint8_t i;
//          uint8_t bits = ((gc_mode_ext & 0x08) && (vga.gc.write_mode == 1)) ? 8 : 4;

			for(i=0;i<4;i++)
			{
				if(vga.sequencer.map_mask & 1 << i)
				{
					if(gc_mode_ext & 0x02)
					{
						vga.memory[(((offset+addr) << 1)+i*0x10000) % vga.svga_intf.vram_size] = (vga.sequencer.data[4] & 4) ? vga_latch_write(i,data) : data;
						vga.memory[(((offset+addr) << 1)+i*0x10000+1) % vga.svga_intf.vram_size] = (vga.sequencer.data[4] & 4) ? vga_latch_write(i,data) : data;
					}
					else
						vga.memory[(((offset+addr))+i*0x10000) % vga.svga_intf.vram_size] = (vga.sequencer.data[4] & 4) ? vga_latch_write(i,data) : data;
				}
			}
			return;
		}
	}
}

/*
 * CL-GD5430 overrides
 */

void cirrus_gd5430_vga_device::crtc_map(address_map &map)
{
	cirrus_gd5428_vga_device::crtc_map(map);
	map(0x1d, 0x1d).lrw8(
		NAME([this] (offs_t offset) {
			return m_cr1d;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOGMASKED(LOG_REGS, "CR1D: Overlay Extended Control %02x\n", data);
			m_cr1d = data;
			// TODO: '34/'36 onward
			vga.crtc.start_addr_latch = (vga.crtc.start_addr_latch & 0xf7ffff) | (BIT(data, 7) << 19);
		})
	);
}

void cirrus_gd5430_vga_device::gc_map(address_map &map)
{
	cirrus_gd5428_vga_device::gc_map(map);
}

void cirrus_gd5430_vga_device::sequencer_map(address_map &map)
{
	cirrus_gd5428_vga_device::sequencer_map(map);
}

/*
 * CL-GD5446 overrides
 */

void cirrus_gd5446_vga_device::crtc_map(address_map &map)
{
	cirrus_gd5430_vga_device::crtc_map(map);
}

void cirrus_gd5446_vga_device::gc_map(address_map &map)
{
	cirrus_gd5430_vga_device::gc_map(map);
}

void cirrus_gd5446_vga_device::sequencer_map(address_map &map)
{
	cirrus_gd5430_vga_device::sequencer_map(map);
}
