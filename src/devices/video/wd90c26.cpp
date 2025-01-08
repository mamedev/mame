// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Paradise / Western Digital SVGA LCD chipsets variants

TODO:
- WD90C24;

- Locking for extra GC addresses;

**************************************************************************************************/

#include "emu.h"
#include "wd90c26.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


DEFINE_DEVICE_TYPE(WD90C26,  wd90c26_vga_device,   "wd90c26_vga",  "Western Digital WD90C26 VGA Controller")

wd90c26_vga_device::wd90c26_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: wd90c11a_vga_device(mconfig, WD90C26, tag, owner, clock)
{
	m_crtc_space_config = address_space_config("crtc_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(wd90c26_vga_device::crtc_map), this));
	m_gc_space_config = address_space_config("gc_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(wd90c26_vga_device::gc_map), this));
	m_seq_space_config = address_space_config("sequencer_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(wd90c26_vga_device::sequencer_map), this));
}

// PR0:PR5, PR10:PR17, PR20:PR21, PR31:PR32 assumed same as derived classes
// TODO: implement the extra unlocks
// when PR30 is locked but PR10 ext CRTC read is enabled then $31-$3f
// reads with ASCII device signature "WD90C26REVx199y"
// (x = rev number, y = manufacturing year)
// cfr. section 6-17 at page 152
void wd90c26_vga_device::crtc_map(address_map &map)
{
	wd90c11a_vga_device::crtc_map(map);
	// PR18 Flat Panel Status
	map(0x31, 0x31).lrw8(
		NAME([this] (offs_t offset) {
			LOG("PR18 Flat Panel Status R\n");
			return m_pr18_fp_status;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_pr18_fp_status = data;
			LOG("PR18 Flat Panel Status W %02x\n", data);
		})
	);
//  PR19 Flat Panel Control I / PR1A Flat Panel Control II
	map(0x32, 0x33).lrw8(
		NAME([this] (offs_t offset) {
			LOG("PR%02X Flat Panel Control %s R\n", offset + 0x19, offset ? "II" : "I");
			return m_fp_control[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_fp_control[offset] = data;
			LOG("PR%02X Flat Panel Control %s W %02x\n", offset + 0x19, offset ? "II" : "I", data);
		})
	);
//  PR1B Flat Panel Unlock
	map(0x34, 0x34).lrw8(
		NAME([this] (offs_t offset) {
			LOG("PR1B Flat Panel Unlock R\n");
			return m_pr1b_fp_unlock;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_pr1b_fp_unlock = data;
			LOG("PR1B Flat Panel Unlock W %02x\n", data);
		})
	);
//  map(0x35, 0x35) PR30 Mapping RAM Unlock
//  map(0x37, 0x37) PR41 Vertical Expansion Initial Value
//  map(0x38, 0x38) PR33 Mapping RAM Address Counter
//  map(0x39, 0x39) PR34 Mapping RAM Data
	// PR35 Mapping RAM Control and Power-Down
	map(0x3a, 0x3a).lrw8(
		NAME([this] (offs_t offset) {
			LOG("PR35 Mapping RAM Control and Power-Down R\n");
			return m_pr35_powerdown;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_pr35_powerdown = data;
			LOG("PR35 Mapping RAM Control and Power-Down W %02x\n", data);
		})
	);
	// PR36 LCD Panel Height Select
	map(0x3b, 0x3b).lrw8(
		NAME([this] (offs_t offset) {
			return m_pr36_lcd_height;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_pr36_lcd_height = data;
			LOG("PR36 LCD Panel Height W %02x\n", data);
		})
	);
//  map(0x3c, 0x3c) PR37 Flat Panel Blinking Control
//  map(0x3e, 0x3e) PR39 Color LCD Control
//  map(0x3f, 0x3f) PR44 Power-Down Memory Refresh Control
}

void wd90c26_vga_device::gc_map(address_map &map)
{
	wd90c11a_vga_device::gc_map(map);
//  map(0x10, 0x11) PR57/PR58 WD90C26 Feature Register I/II
//  map(0x12, 0x12) PR59 Memory Arbitration Cycle Setup
//  map(0x15, 0x15) PR62 FR Timing
}

void wd90c26_vga_device::sequencer_map(address_map &map)
{
	wd90c11a_vga_device::sequencer_map(map);
	m_ext_seq_view[1](0x08, 0x08).unmaprw(); // undefined, assume unmapped
	m_ext_seq_view[1](0x09, 0x09).unmaprw(); // ^
//  m_ext_seq_view[1](0x10, 0x10) PR30A Memory Interface Write Buffer & FIFO Control
//  m_ext_seq_view[1](0x14, 0x14) PR34A Video Memory Virtual Page
//  m_ext_seq_view[1](0x16, 0x16) PR45 Video Signature Control
//  m_ext_seq_view[1](0x17, 0x18) PR45A/PR45B Signature Analyzer
}
