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
#include "debugger.h"
#include "axc51-core.h"
#include "axc51-core_dasm.h"

DEFINE_DEVICE_TYPE(AXC51CORE, axc51core_cpu_device, "axc51core", "AppoTech AXC51-CORE")
DEFINE_DEVICE_TYPE(AX208, ax208_cpu_device, "ax208", "AppoTech AX208 (AXC51-CORE)")

// AXC51CORE (base device)

axc51core_cpu_device::axc51core_cpu_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock, int program_width, int data_width, uint8_t features)
	: mcs51_cpu_device(mconfig, type, tag, owner, clock, program_width, data_width, features)
{
}

axc51core_cpu_device::axc51core_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: axc51core_cpu_device(mconfig, AXC51CORE, tag, owner, clock, 0, 7)
{
}

std::unique_ptr<util::disasm_interface> axc51core_cpu_device::create_disassembler()
{
	return std::make_unique<axc51core_disassembler>();
}

// AX208 (specific CPU)

ax208_cpu_device::ax208_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: axc51core_cpu_device(mconfig, AX208, tag, owner, clock, 0, 7)
{
}

std::unique_ptr<util::disasm_interface> ax208_cpu_device::create_disassembler()
{
	return std::make_unique<ax208_disassembler>();
}

ROM_START( ax208 )
	ROM_REGION( 0x2000, "rom", 0 )
	ROM_LOAD("ax208.rom", 0x0000, 0x2000, NO_DUMP )
ROM_END

const tiny_rom_entry *ax208_cpu_device::device_rom_region() const
{
	return ROM_NAME( ax208 );
}
