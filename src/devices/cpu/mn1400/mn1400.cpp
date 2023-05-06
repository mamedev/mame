// license:BSD-3-Clause
// copyright-holders:hap
/*

  Matsushita MN1400, MN1405

TODO:
- stuff

*/

#include "emu.h"
#include "mn1400.h"

#include "mn1400d.h"


// device definitions
DEFINE_DEVICE_TYPE(MN1400, mn1400_cpu_device, "mn1400", "Matsushita MN1400")
DEFINE_DEVICE_TYPE(MN1405, mn1405_cpu_device, "mn1405", "Matsushita MN1405")


// constructor
mn1400_cpu_device::mn1400_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int stack_levels, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data) :
	mn1400_base_device(mconfig, type, tag, owner, clock, stack_levels, prgwidth, program, datawidth, data)
{ }

mn1400_cpu_device::mn1400_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	mn1400_cpu_device(mconfig, MN1400, tag, owner, clock, 2 /* stack levels */, 10 /* rom bits */, address_map_constructor(FUNC(mn1400_cpu_device::program_1kx8), this), 6 /* ram bits */, address_map_constructor(FUNC(mn1400_cpu_device::data_64x4), this))
{ }

mn1405_cpu_device::mn1405_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	mn1400_cpu_device(mconfig, MN1405, tag, owner, clock, 2, 11, address_map_constructor(FUNC(mn1405_cpu_device::program_2kx8), this), 7, address_map_constructor(FUNC(mn1405_cpu_device::data_128x4), this))
{ }


// disasm
std::unique_ptr<util::disasm_interface> mn1400_cpu_device::create_disassembler()
{
	return std::make_unique<mn1400_disassembler>();
}


//-------------------------------------------------
//  execute
//-------------------------------------------------

bool mn1400_cpu_device::op_has_param(u8 op)
{
	return false;
}

void mn1400_cpu_device::execute_one()
{
	op_illegal();
}
