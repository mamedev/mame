// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Paradise / Western Digital (S)VGA chipsets

TODO:
- stub, (S?)VGA features not yet implemented;
- PVGA1A-JK / WD90C90-JK (same as PVGA1A with extra connectors?)
- WD90C00-JK
- WD90C11-LR / WD90C11A-LR
- WD90C30-LR / WD90C31-LR / WD90C31-ZS / WD90C31A-LR / WD90C31A-ZS
- WD90C33-ZZ
- WD90C24A-ZZ / WD90C24A2-ZZ (mobile chips, no ISA option)
- WD90C26A (apple/macpwrbk030.cpp macpb180c, no ISA)
- WD9710-MZ (PCI + MPEG-1, a.k.a. Pipeline 9710 / 9712)

**************************************************************************************************/

#include "emu.h"
#include "pc_vga_paradise.h"

#include "screen.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

DEFINE_DEVICE_TYPE(PVGA1A, pvga1a_vga_device, "pvga1a_vga", "Paradise Systems PVGA1A")

pvga1a_vga_device::pvga1a_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: svga_device(mconfig, PVGA1A, tag, owner, clock)
{
	m_gc_space_config = address_space_config("gc_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(pvga1a_vga_device::gc_map), this));
}

void pvga1a_vga_device::device_start()
{
	svga_device::device_start();
	zero();

	// Avoid an infinite loop when displaying.  0 is not possible anyway.
	vga.crtc.maximum_scan_line = 1;

	// copy over interfaces
	vga.svga_intf.vram_size = 1*1024*1024;
	//vga.memory = std::make_unique<uint8_t []>(vga.svga_intf.vram_size);
}

void pvga1a_vga_device::device_reset()
{
	svga_device::device_reset();

	// ...
}

void pvga1a_vga_device::gc_map(address_map &map)
{
	svga_device::gc_map(map);
	// TODO: temp hack so the card boots
	map(0x09, 0x0f).ram();
}
