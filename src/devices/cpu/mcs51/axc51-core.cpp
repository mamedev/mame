// license:BSD-3-Clause
// copyright-holders:David Haywood
/*****************************************************************************

    AXC51-CORE (AppoTech Inc.)

    used in

    AX208 SoC

    Notes:

    AXC51CORE:
    somes sources indicate that the extended opcode encoding may change
    on some CPU models despite all being 'AXC51CORE' however we lack solid
    information on this at present.

    AX208:
    The CPU has 0x2000 bytes of internal ROM mapped at 0x8000-0x9fff providing
    bootcode, operating kernel and many standard library functions

 *****************************************************************************/

#include "emu.h"
#include "axc51-core.h"
#include "axc51-core_dasm.h"

DEFINE_DEVICE_TYPE(AXC51CORE, axc51core_cpu_device, "axc51core", "AppoTech AXC51-CORE")
DEFINE_DEVICE_TYPE(AX208, ax208_cpu_device, "ax208", "AppoTech AX208 (AXC51-CORE)")
DEFINE_DEVICE_TYPE(AX208P, ax208p_cpu_device, "ax208p", "AppoTech AX208 (AXC51-CORE) (prototype?)")

// AXC51CORE (base device)

axc51core_cpu_device::axc51core_cpu_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock,  address_map_constructor program_map, address_map_constructor data_map, int program_width, int data_width, uint8_t features)
	: mcs51_cpu_device(mconfig, type, tag, owner, clock, program_map, data_map, program_width, data_width, features)
{
}

axc51core_cpu_device::axc51core_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: axc51core_cpu_device(mconfig, AXC51CORE, tag, owner, clock, address_map_constructor(FUNC(axc51core_cpu_device::program_internal), this), address_map_constructor(FUNC(axc51core_cpu_device::data_internal), this), 0, 7)
{
}

std::unique_ptr<util::disasm_interface> axc51core_cpu_device::create_disassembler()
{
	return std::make_unique<axc51core_disassembler>();
}

// AX208 (specific CPU)

ax208_cpu_device::ax208_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: axc51core_cpu_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(ax208_cpu_device::ax208_internal_program_mem), this), address_map_constructor(FUNC(ax208_cpu_device::ax208_internal_data_mem), this), 0, 7)
{
}

ax208_cpu_device::ax208_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ax208_cpu_device(mconfig, AX208, tag, owner, clock)
{
}


std::unique_ptr<util::disasm_interface> ax208_cpu_device::create_disassembler()
{
	return std::make_unique<ax208_disassembler>();
}

void ax208_cpu_device::ax208_internal_program_mem(address_map &map)
{
	map(0x8000, 0x9fff).rom().region("rom", 0); // this can only be read from code running within the same region
}

void ax208_cpu_device::ax208_internal_data_mem(address_map &map)
{
	map(0x0000, 0x00ff).ram().share("scratchpad");
	map(0x0100, 0x01ff).ram().share("sfr_ram"); /* SFR */
}


ROM_START( ax208 ) // assume all production ax208 chips use this internal ROM
	ROM_REGION( 0x2000, "rom", 0 )
	ROM_LOAD("ax208.bin", 0x0000, 0x2000, CRC(b85f954a) SHA1(0dc7ab9bdaf73231d4d6627fe6308fe8103e1bbc) )
ROM_END

const tiny_rom_entry *ax208_cpu_device::device_rom_region() const
{
	return ROM_NAME( ax208 );
}

void ax208_cpu_device::device_reset()
{
	axc51core_cpu_device::device_reset();
	set_state_int(MCS51_PC, 0x8000);
}


ax208p_cpu_device::ax208p_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ax208_cpu_device(mconfig, AX208P, tag, owner, clock)
{
}

ROM_START( ax208p ) // this is an early revision of the internal AX208 code, some functions are moved around so it isn't entirely compatible
	ROM_REGION( 0x2000, "rom", 0 )
	ROM_LOAD("mask208.bin", 0x0000, 0x2000, CRC(52396183) SHA1(b119000f93251894a352ecf675ee42f2e5c347bd) )
ROM_END

const tiny_rom_entry *ax208p_cpu_device::device_rom_region() const
{
	return ROM_NAME( ax208p );
}

