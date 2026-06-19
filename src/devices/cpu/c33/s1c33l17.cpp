// license:BSD-3-Clause
// copyright-holders:AJR

#include "emu.h"
#include "s1c33l17.h"

// device type definitions
DEFINE_DEVICE_TYPE(S1C33L17, s1c33l17_device, "s1c33l17", "Epson S1C33L17")
DEFINE_DEVICE_TYPE(S1C33E07, s1c33e07_device, "s1c33e07", "Epson S1C33E07")

// FIXME: based on C33 PE Core, not STD Core
s1c33l17_device::s1c33l17_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor map)
	: c33std_cpu_device_base(mconfig, type, tag, owner, clock, map)
{
}

s1c33l17_device::s1c33l17_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: s1c33l17_device(mconfig, S1C33L17, tag, owner, clock, address_map_constructor(FUNC(s1c33l17_device::internal_map), this))
{
}

s1c33e07_device::s1c33e07_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: s1c33l17_device(mconfig, S1C33E07, tag, owner, clock, address_map_constructor(FUNC(s1c33e07_device::internal_map), this))
{
}

void s1c33l17_device::device_reset()
{
	c33std_cpu_device_base::device_reset();

	// TODO: gate ROM depending on boot mode
	m_pc = m_data.read_dword(0x00c00000);
}

void s1c33l17_device::base_internal_map(address_map &map)
{
	map(0x00000000, 0x00004fff).ram();
	map(0x00084000, 0x000847ff).ram();
	// 0x00300010-0x00300020 Misc Register (1)
	// 0x00300380-0x003003d5 I/O Ports
	// 0x00300520-0x0030055e A/D Converter
	// 0x00300660-0x0030033c Watchdog Timer (WDT)
	// 0x00300900-0x0030099f USB Function Controller
	// 0x00300a00-0x00300aff USB DMA Area
	// 0x00300b00-0x00300b4f Serial Interface (EFSIO)
	// 0x00300c00-0x00300c25 Extended Ports
	// 0x00300c40-0x00300c4d Misc Register (2)
	// 0x00301100-0x00301105 Intelligent DMA
	// 0x00301120-0x0030119e High-Speed DMA
	// 0x00301500-0x00301510 SRAM Controller
	// 0x00301600-0x00301610 SDRAM Controller
	// 0x00301700-0x0030171c SPI
	// 0x00301900-0x00301928 Real Time Clock
	// 0x00301b00-0x00301b24 Clock Management Unit
}

void s1c33l17_device::internal_map(address_map &map)
{
	base_internal_map(map);
	// 0x00300260-0x003002af Interrupt Controller
	// 0x00300300-0x0030031b Card Interface and Reed Solomon CODEC
	// 0x00300780-0x003007ea 16-bit Timer (4 channels)
	// 0x00301a00-0x00301aac LCD Controller (LCDC)
	// 0x00301c00-0x00301c30 I²S Interface
}

void s1c33e07_device::internal_map(address_map &map)
{
	base_internal_map(map);
	// 0x00300260-0x003002af Interrupt Controller (more sources than S1C33L17)
	// 0x00300300-0x0030031b Card Interface with ECC
	// 0x00300780-0x003007ea 16-bit Timer (6 channels)
	// 0x00301800-0x0030181c Direction Control Serial Interface (DCSIO)
	// 0x00301a00-0x00301aac LCD Controller (LCDC) (no 16bpp mode)
	// 0x00301c00-0x00301c30 I²S Interface (different from S1C33L17)
}
