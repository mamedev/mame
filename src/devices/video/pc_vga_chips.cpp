// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

1st Generation Chips & Technologies VGA cores, up to F65548

(F65550+ "HiQVision" looks very different and will go in a separate file / structure,
where they ditched the exclusive $3d6/$3d7 address spacing for inheriting them in regular
VGA spaces)

**************************************************************************************************/

#include "emu.h"
#include "pc_vga_chips.h"

#include "screen.h"

#define LOG_XR (1U << 1) // log extended info about registers writes

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGXR(...)     LOGMASKED(LOG_XR,  __VA_ARGS__)

DEFINE_DEVICE_TYPE(F65535_VGA,  f65535_vga_device,  "f65535_vga",  "Chips and Technologies F65535 VGA i/f")

f65535_vga_device::f65535_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: svga_device(mconfig, F65535_VGA, tag, owner, clock)
{
	m_main_if_space_config = address_space_config("io_regs", ENDIANNESS_LITTLE, 8, 4, 0, address_map_constructor(FUNC(f65535_vga_device::io_3bx_3dx_map), this));
	m_ext_space_config = address_space_config("ext_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(f65535_vga_device::ext_map), this));
}

device_memory_interface::space_config_vector f65535_vga_device::memory_space_config() const
{
	auto r = svga_device::memory_space_config();
	r.emplace_back(std::make_pair(EXT_REG,     &m_ext_space_config));
	return r;
}


void f65535_vga_device::device_start()
{
	svga_device::device_start();

	save_item(NAME(m_ext_index));
	save_item(NAME(m_cpu_interface_1));
	save_item(NAME(m_memory_control_1));
	save_item(NAME(m_memory_control_2));
	save_item(NAME(m_palette_control));
	save_item(NAME(m_cpu_paging));
	save_item(NAME(m_start_address_top));
	save_item(NAME(m_software_flags_0));
	save_item(NAME(m_horizontal_overflow));
	save_item(NAME(m_alt_h_panel_size));

	save_item(NAME(m_video_interface));
	save_item(NAME(m_software_flags_1));
	save_item(NAME(m_clock_control));
	save_item(NAME(m_software_flags_2));

	save_item(NAME(m_display_type));
	save_item(NAME(m_h_compensation));
	save_item(NAME(m_v_compensation));

	save_item(NAME(m_output_drive));

	// TODO: 0x04 part guessed, not provided in doc
	m_chip_version = 0xc0 | 0x04;
}

void f65535_vga_device::device_reset()
{
	svga_device::device_reset();

	m_ext_index = 0;
	m_cpu_interface_1 = 0;
	m_memory_control_1 = m_memory_control_2 = 0;
	m_palette_control = 0;
	m_cpu_paging = 0;
	m_start_address_top = 0;
	m_software_flags_0 = m_software_flags_1 = m_software_flags_2 = 0;
	m_horizontal_overflow = 0;
	m_alt_h_panel_size = 0;
	m_video_interface = 0;
	m_display_type = 0;
	m_h_compensation = 0;
	m_v_compensation = 0;
	m_v_line_replication = 0;
	m_output_drive = 0;
}

void f65535_vga_device::io_3bx_3dx_map(address_map &map)
{
	svga_device::io_3bx_3dx_map(map);
	map(0x06, 0x06).lrw8(
		NAME([this] (offs_t offset) { return m_ext_index; }),
		NAME([this] (offs_t offset, u8 data) { m_ext_index = data; })
	);
	map(0x07, 0x07).lrw8(
		NAME([this] (offs_t offset) { return space(EXT_REG).read_byte(m_ext_index); }),
		NAME([this] (offs_t offset, u8 data) { space(EXT_REG).write_byte(m_ext_index, data); })
	);
}

// XRnn
void f65535_vga_device::ext_map(address_map &map)
{
	map(0x00, 0x00).lr8(NAME([this] () { LOG("XR00: read version\n"); return m_chip_version; }));
	// Configuration
	// TODO: configurable via external pull-down
	map(0x01, 0x01).lr8(NAME([this] () { LOG("XR01: read configuration\n"); return 0x01; }));
	// CPU Interface Control 1
	map(0x02, 0x02).lrw8(
		NAME([this] () { return m_cpu_interface_1 | (vga.attribute.state << 7); }),
		NAME([this] (offs_t offset, u8 data) {
			LOG("XR02: CPU Interface Control 1 %02x\n", data);
			LOGXR("\tCPU i/f %d-bit | Clock Mode %s | Attribute %d | I/O Address Decoding %s | $83c6 enable %d\n"
				, BIT(data, 0) ? 16 : 8
				, BIT(data, 1) ? "Digital Monitor" : "Normal"
				, (data >> 3) & 3
				, BIT(data, 5) ? "10-bit only" : "16-bit"
				// Palette decoding, extra decoding at $83c6~$83c9 if enabled,
				// $3c6~$3c9 only otherwise
				, BIT(data, 6)
				// bit 7 read only, reads the Attribute f/f
			);
			m_cpu_interface_1 = data & 0x7f;
		})
	);
//	map(0x03, 0x03) CPU Interface Control 2 / ROM Interface (F64300, F65540+)
	// Memory Control 1
	map(0x04, 0x04).lrw8(
		NAME([this] () { return m_memory_control_1; }),
		NAME([this] (offs_t offset, u8 data) {
			LOG("XR04: Memory Control 1 %02x\n", data);
			LOGXR("\tMemory Configuration %d | Memory Wraparound %d | Write Buffer Enable %d\n"
				, data & 3
				, BIT(data, 2)
				, BIT(data, 5)
			);
			m_memory_control_1 = data;
		})
	);
	// Memory Control 2
	map(0x05, 0x05).lrw8(
		NAME([this] () { return m_memory_control_2; }),
		NAME([this] (offs_t offset, u8 data) {
			LOG("XR05: Memory Control 2 %02x\n", data);
			// RAS/CAS stuff + PC Video Interface Enable on bit 6
			m_memory_control_2 = data;
		})
	);
	// Palette Control
	map(0x06, 0x06).lrw8(
		NAME([this] () { return m_palette_control; }),
		NAME([this] (offs_t offset, u8 data) {
			LOG("XR06: Palette Control %02x\n", data);
			LOGXR("\tDisable Pixel Data Pins %d | Disable Internal DAC %d | Display Mode Color %d | PC Video Color Key Enable %d | Bypass Internal VGA Palette %d | Color Reduction Select %d\n"
				, BIT(data, 0)
				, BIT(data, 1)
				, (data >> 2) & 3
				, BIT(data, 4)
				, BIT(data, 5)
				, (data >> 6) & 3
			);

			m_palette_control = data;
			flush_true_color_mode();
		})
	);
//	map(0x07, 0x07) <reserved> (I/O Base)
//	map(0x08, 0x08) Linear Addressing Base L (F64300 only)
//	map(0x09, 0x09) <reserved> Linear Base H
//	map(0x0a, 0x0a) <reserved> XRAM Mode
	// CPU Paging
	map(0x0b, 0x0b).lrw8(
		NAME([this] () { return m_cpu_paging; }),
		NAME([this] (offs_t offset, u8 data) {
			LOG("XR0B: CPU Paging %02x\n", data);
			LOGXR("\tMemory Mapping Mode %d | %s Mapping | CPU Address /4 %d | Extended Text Mode %d | Linear Addressing Enable %d | Linear Address Size %d\n"
				, BIT(data, 0)
				, BIT(data, 1) ? "Dual" : "Single"
				, BIT(data, 2)
				, BIT(data, 3)
				, BIT(data, 4)
				, BIT(data, 5) ? 1024 : 512
			);
			m_cpu_paging = data;
		})
	);
	// Start Address Top
	map(0x0c, 0x0c).lrw8(
		NAME([this] () { return m_start_address_top; }),
		NAME([this] (offs_t offset, u8 data) {
			LOG("XR0C: Start Address Top %02x\n", data);
			m_start_address_top = data & 3;
		})
	);
//	map(0x0d, 0x0d) Auxiliary Offset
//	map(0x0e, 0x0e) Text Mode Control
	// Software Flags 0
	// (defined by BIOS or driver, has no actual function)
	map(0x0f, 0x0f).lrw8(
		NAME([this] () { return m_software_flags_0; }),
		NAME([this] (offs_t offset, u8 data) {
			LOG("XR0F: Software Flags %02x\n", data);
			LOGXR("\tMemory Size %d | True Color %d | Packed-Pixel Mode Dot Clock %d | Interlace Select %d | Text Compensation %d\n"
				// 00 256KB
				// 01 512KB
				// 1x 1MB
				, data & 3
				, BIT(data, 4)
				, BIT(data, 5)
				, BIT(data, 6)
				, BIT(data, 7)
			);
			m_software_flags_0 = data;
		})
	);
//	map(0x10, 0x10) Single/Low Map
//	map(0x11, 0x11) High Map
//	map(0x14, 0x14) Emulation Mode
//	map(0x15, 0x15) Write Protect
//	map(0x16, 0x16) Vertical Overflow (F64300 only)
	// Horizontal Overflow (F64300 only)
	map(0x17, 0x17).lrw8(
		NAME([this] () { return m_horizontal_overflow; }),
		NAME([this] (offs_t offset, u8 data) {
			// TODO: as the comment claims this shouldn't exist in F65535
			// ptpc110 BIOS still writes with zeroes, left-over?
			if (data)
				popmessage("pc_vga_chips.cpp: XR17 actually written to %02x", data);
			m_horizontal_overflow = data;
		})
	);
//	map(0x18, 0x18) Alt H Disp End
//	map(0x19, 0x19) Alt H Sync Start
//	map(0x1a, 0x1a) Alt H Sync End
//	map(0x1b, 0x1b) Alt H Total
	// Alt H Blank Start / H Panel Size
	map(0x1c, 0x1c).lrw8(
		NAME([this] () { return m_alt_h_panel_size; }),
		NAME([this] (offs_t offset, u8 data) {
			// FP: Panel Size + 1 * 8 or 9
			// CRT: Alt Horizontal Blank Start in CGA/Hercules modes
			LOG("XR1C: H Panel Size %02x\n", data);
			m_alt_h_panel_size = data;
		})
	);
//	map(0x1d, 0x1d) Alt H Blank End
//	map(0x1e, 0x1e) Alt Offset
//	map(0x1f, 0x1f) Virtual EGA Switch

//	map(0x21, 0x21) Alt H Sync Start Ext Modes
//	map(0x22, 0x22) Alt H Sync End Ext Modes
//	map(0x23, 0x23) Alt H Total Ext
//	map(0x24, 0x24) FP Alt Max Scanline
//	map(0x25, 0x25) FP Alt Text Mode H Virtual Panel Size
//	map(0x26, 0x26) Alt H Sync Start

	// Video Interface
	map(0x28, 0x28).lrw8(
		NAME([this] () { return m_video_interface; }),
		NAME([this] (offs_t offset, u8 data) {
			LOG("XR28: Video Interface %02x\n", data);
			LOGXR("\tBlank / Display Enable Select %d | 256-Color Video Path %d | Interlace %d | 8-bit Video Pixel Panning %d | Tall Font Replication %d\n"
				// CRT mode only
				, BIT(data, 0)
				, BIT(data, 4)
				, BIT(data, 5)
				, BIT(data, 6)
				, BIT(data, 7)
			);
			// TODO: easy to implement but applies to CRT only
			if (BIT(data, 5))
				popmessage("pc_vga_chips.cpp: Interlace Enable");
			m_video_interface = data;
		})
	);
//	map(0x29, 0x29) Half Line Compare

	// Software Flags 1
	map(0x2b, 0x2b).lrw8(
		NAME([this] () { return m_software_flags_1; }),
		NAME([this] (offs_t offset, u8 data) {
			LOG("XR2B: Software Flags 1 (Current Display Mode) %02x\n", data);
			m_software_flags_1 = data;
		})
	);
//	map(0x2c, 0x2c) FLM Delay
//	map(0x2d, 0x2d) LP Delay (Comp Disabled)
//	map(0x2e, 0x2e) LP Delay (Comp Enabled)
//	map(0x2f, 0x2f) LP Width

	// Clock Control (F64300, F65540+)
	// Programmed in F65535 anyway (not on '30 really?)
	map(0x30, 0x33).lrw8(
		NAME([this] (offs_t offset) { return m_clock_control[offset]; }),
		NAME([this] (offs_t offset, u8 data) {
			LOG("XR%02x: Clock Control %02x\n", offset + 0x30, data);
			m_clock_control[offset] = data;
		})
	);

//	map(0x3a, 0x3c) Color Key (F64300, F65540+)
//	map(0x3d, 0x3f) Color Key Mask (F64300, F65540+)
//	map(0x40, 0x40) <reserved> BitBlt Config (F64300, F65540+)

	// Software Flag 2
	map(0x44, 0x44).lrw8(
		NAME([this] () { return m_software_flags_2; }),
		NAME([this] (offs_t offset, u8 data) {
			LOG("XR44: Software Flags 2 %02x\n", data);
			LOGXR("\tSet Panel Type #%d%s | Optimal Compensation Enable %d\n"
				, data & 0xf
				, BIT(data, 3) ? " <reserved>!" : ""
				, BIT(data, 4)
			);
			m_software_flags_2 = data;
		})
	);
//	map(0x45, 0x45) Software Flag 3 (F65540+)

//	map(0x4f, 0x4f) Panel Format 2 (F65540+)
//	map(0x50, 0x50) Panel Format 1
	// Display Type
	map(0x51, 0x51).lrw8(
		NAME([this] (offs_t offset) { return m_display_type; }),
		NAME([this] (offs_t offset, u8 data) {
			LOG("XR51: Display Type %02x\n", data);
			m_display_type = data;
		})
	);
//	map(0x52, 0x52) Power Down Control / Refresh Control
//	map(0x53, 0x53) Panel Format 3
//	map(0x54, 0x54) Panel Interface
	// H Compensation
	map(0x55, 0x55).lrw8(
		NAME([this] () { return m_h_compensation; }),
		NAME([this] (offs_t offset, u8 data) {
			LOG("XR55: H Compensation %02x\n", data);
			m_h_compensation = data;
		})
	);
//	map(0x56, 0x56) H Centering
	// V Compensation
	map(0x57, 0x57).lrw8(
		NAME([this] () { return m_v_compensation; }),
		NAME([this] (offs_t offset, u8 data) {
			LOG("XR57: V Compensation %02x\n", data);
			m_v_compensation = data;
		})
	);
//	map(0x58, 0x58) V Centering
//	map(0x59, 0x59) V Line Insertion
	// V Line Replication
	map(0x5a, 0x5a).lrw8(
		NAME([this] () { return m_v_line_replication; }),
		NAME([this] (offs_t offset, u8 data) {
			LOG("XR5A: V Line Replication %02x\n", data);
			m_v_line_replication = data;
		})
	);
//	map(0x5b, 0x5b) Power Sequencing Delay
//	map(0x5c, 0x5c) Activity Indicator Control (F65540+)
//	map(0x5d, 0x5d) FP Diag (F65540+)
//	map(0x5e, 0x5e) M(ACDCLK) Control
//	map(0x5f, 0x5f) Power Down Mode Refresh
//	map(0x60, 0x60) Blink Rate Control
//	map(0x61, 0x61) SmartMap Control
//	map(0x62, 0x62) SmartMap Shift Parameter
//	map(0x63, 0x63) SmartMap Color Mapping Control
//	map(0x64, 0x64) FP Alt Vertical Total
//	map(0x65, 0x65) FP Alt Overflow
//	map(0x66, 0x66) FP Alt Vertical Sync Start
//	map(0x67, 0x67) FP Alt Vertical Sync End
//	map(0x68, 0x68) FP V Panel Size

	// Programmable Output Drive
	map(0x6c, 0x6c).lrw8(
		NAME([this] () { return m_output_drive; }),
		NAME([this] (offs_t offset, u8 data) {
			LOG("XR6C: Programmable Output Drive %02x\n", data);
			// Double rate various interfaces
			m_output_drive = data;
		})
	);

//	map(0x6e, 0x6e) Polynomial FRC Control
//	map(0x6f, 0x6f) Frame Buffer Control
//	map(0x70, 0x70) Setup / Disable Control
//	map(0x71, 0x71) <reserved> GPIO Control (F64300 only)
//	map(0x72, 0x72) GPIO Data (F64300 only)
//	map(0x73, 0x73) DPMS Control / Misc Control (F64300, F65540+)
//	map(0x74, 0x74) <reserved> Configuration 2 (F64300 only)
//	map(0x75, 0x75) <reserved> Software Flags 3 (F64300 only)

//	map(0x7d, 0x7d) FP Compensation Diagnostic (r/o)
//	map(0x7e, 0x7e) CGA / Hercules Color Select
//	map(0x7f, 0x7f) Diagnostic
}

void f65535_vga_device::flush_true_color_mode()
{
	svga.rgb8_en = svga.rgb15_en = svga.rgb16_en = svga.rgb24_en = svga.rgb32_en = 0;
	const u8 display_mode = (m_palette_control >> 2) & 3;
	if (!display_mode)
	{
		LOG("\tVGA mode\n");
		return;
	}

	switch(display_mode)
	{
		case 1: LOG("\tSierra mode 5-5-5\n"); svga.rgb15_en = 1; break;
		case 2: LOG("\tTrue Color\n"); svga.rgb24_en = 1; break;
		case 3: LOG("\tXGA mode 5-6-5\n"); svga.rgb16_en = 1; break;
	}

	// TODO: may require that or segmented mode support in mem_r/_w
	popmessage("pc_vga_chips.cpp: needs mem_linear_r/_w");

	recompute_params();
}
