// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    deco16.c

    6502, reverse-engineered DECO variant

***************************************************************************/

#include "emu.h"
#include "deco16.h"
#include "deco16d.h"

#define DECO16_VERBOSE 1

DEFINE_DEVICE_TYPE(DECO16, deco16_device, "deco16", "DECO16")

deco16_device::deco16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, DECO16, tag, owner, clock),
	io(nullptr),
	io_config("io", ENDIANNESS_LITTLE, 8, 16)
{
}

util::disasm_interface *deco16_device::create_disassembler()
{
	return new deco16_disassembler;
}

void deco16_device::device_start()
{
	if(direct_disabled)
		mintf = new mi_default_nd;
	else
		mintf = new mi_default_normal;

	init();

	io = &space(AS_IO);
}

device_memory_interface::space_config_vector deco16_device::memory_space_config() const
{
	if(has_configured_map(AS_OPCODES))
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &program_config),
			std::make_pair(AS_OPCODES, &sprogram_config),
			std::make_pair(AS_IO,      &io_config)
		};
	else
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &program_config),
			std::make_pair(AS_IO,      &io_config)
		};
}

#include "cpu/m6502/deco16.hxx"
