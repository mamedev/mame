// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

1st Generation Chips & Technologies VGA cores, up to F65548

(F65550+ looks very different, where they ditched the exclusive $3d6/$3d7 address spacing for
inheriting them in regular VGA spaces)

**************************************************************************************************/

#include "emu.h"
#include "pc_vga_chips.h"

#include "screen.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

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
	// TODO: 0x04 part guessed, not provided in doc
	m_chip_version = 0xc0 | 0x04;
}

void f65535_vga_device::device_reset()
{
	svga_device::device_reset();

	m_ext_index = 0;
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
	map(0x00, 0x00).lr8(NAME([this] () { return m_chip_version; }));
	// Configuration
	// TODO: configurable via external pull-down
	map(0x01, 0x01).lr8(NAME([] () { return 0x01; }));
//	map(0x02, 0x02) CPU Interface Control 1
//	map(0x03, 0x03) CPU Interface Control 2 / ROM Interface (F64300, F65540+)
//	map(0x04, 0x04) Memory Control 1
//	map(0x05, 0x05) Memory Control 2
//	map(0x06, 0x06) Palette Control
//	map(0x07, 0x07) <reserved> (I/O Base)
//	map(0x08, 0x08) Linear Addressing Base L (F64300 only)
//	map(0x09, 0x09) <reserved> Linear Base H
//	map(0x0a, 0x0a) <reserved> XRAM Mode
//	map(0x0b, 0x0b) CPU Paging
//	map(0x0c, 0x0c) Start Address Top
//	map(0x0d, 0x0d) Auxiliary Offset
//	map(0x0e, 0x0e) Text Mode Control
//	map(0x0f, 0x0f) Software Flags 0
//	map(0x10, 0x10) Single/Low Map
//	map(0x11, 0x11) High Map
//	map(0x14, 0x14) Emulation Mode
//	map(0x15, 0x15) Write Protect
//	map(0x16, 0x16) Vertical Overflow (F64300 only)
//	map(0x17, 0x17) Horizontal Overflow (F64300 only)
//	map(0x18, 0x18) Alt H Disp End
//	map(0x19, 0x19) Alt H Sync Start
//	map(0x1a, 0x1a) Alt H Sync End
//	map(0x1b, 0x1b) Alt H Total
//	map(0x1c, 0x1c) Alt H Blank Start / H Panel Size
//	map(0x1d, 0x1d) Alt H Blank End
//	map(0x1e, 0x1e) Alt Offset
//	map(0x1f, 0x1f) Virtual EGA Switch

//	map(0x21, 0x21) Alt H Sync Start Ext Modes
//	map(0x22, 0x22) Alt H Sync End Ext Modes
//	map(0x23, 0x23) Alt H Total Ext
//	map(0x24, 0x24) FP Alt Max Scanline
//	map(0x25, 0x25) FP Alt Text Mode H Virtual Panel Size
//	map(0x26, 0x26) Alt H Sync Start

//	map(0x28, 0x28) Video Interface
//	map(0x29, 0x29) Half Line Compare

//	map(0x2b, 0x2b) Software Flags
//	map(0x2c, 0x2c) FLM Delay
//	map(0x2d, 0x2d) LP Delay (Comp Disabled)
//	map(0x2e, 0x2e) LP Delay (Comp Enabled)
//	map(0x2f, 0x2f) LP Width

//	map(0x30, 0x33) Clock Control (F64300, F65540+)

//	map(0x3a, 0x3c) Color Key (F64300, F65540+)
//	map(0x3d, 0x3f) Color Key Mask (F64300, F65540+)
//	map(0x40, 0x40) <reserved> BitBlt Config (F64300, F65540+)

//	map(0x44, 0x44) Software Flag 2
//	map(0x45, 0x45) Software Flag 3 (F65540+)

//	map(0x4f, 0x4f) Panel Format 2 (F65540+)
//	map(0x50, 0x50) Panel Format 1
	// Display Type
	// TODO: read/write really, mostly matters for Flat Panel scenarios
	map(0x51, 0x51).lr8(NAME([] () { return 0; }));
//	map(0x52, 0x52) Power Down Control / Refresh Control
//	map(0x53, 0x53) Panel Format 3
//	map(0x54, 0x54) Panel Interface
//	map(0x55, 0x55) H Compensation
//	map(0x56, 0x56) H Centering
//	map(0x57, 0x57) V Compensation
//	map(0x58, 0x58) V Centering
//	map(0x59, 0x59) V Line Insertion
//	map(0x5a, 0x5a) V Line Replication
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

//	map(0x6c, 0x6c) Programmable Output Drive

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
