// license:BSD-3-Clause
// copyright-holders:hap, Igor
/*

  KB1013VK1-2 MCU core implementation

*/

#include "sm500.h"
#include "debugger.h"

// MCU types
const device_type KB1013VK12 = &device_creator<kb1013vk12_device>;


// internal memory maps
static ADDRESS_MAP_START(program_2_7k, AS_PROGRAM, 8, sm510_base_device)
	AM_RANGE(0x0000, 0x00ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(data_96_32x4, AS_DATA, 8, sm510_base_device)
	AM_RANGE(0x00, 0x1f) AM_RAM
ADDRESS_MAP_END

// device definitions
kb1013vk12_device::kb1013vk12_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sm500_device(mconfig, KB1013VK12, "KB1013VK1-2", tag, owner, clock, 2 /* stack levels */, 12 /* prg width */, ADDRESS_MAP_NAME(program_2_7k), 7 /* data width */, ADDRESS_MAP_NAME(data_96_32x4), "kb1013vk1-2", __FILE__)
{ }
