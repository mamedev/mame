// license:BSD-3-Clause
// copyright-holders:David Haywood
/*****************************************************************************

    AXC51-CORE (AppoTech Inc.)

    used in

    AX208 SoC



    Notes:

    AX208:
    The CPU has a bootloader that sets a few things up + copies data to RAM
    from the Flash meomry.  This will need to be simulated.

 *****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "axc51-core.h"
#include "axc51-core_dasm.h"

DEFINE_DEVICE_TYPE(AX208, ax208_cpu_device, "ax208", "AppoTech AX208 (AXC51-CORE)")

ax208_cpu_device::ax208_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mcs51_cpu_device(mconfig, AX208, tag, owner, clock, 0, 7)
{
}

std::unique_ptr<util::disasm_interface> ax208_cpu_device::create_disassembler()
{
	return std::make_unique<axc51core_disassembler>();
}
