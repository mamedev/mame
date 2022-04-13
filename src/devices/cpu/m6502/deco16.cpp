// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    deco16.c

    6502, reverse-engineered DECO variant

    Note that the "DECO CPU16" is not in fact a CPU in itself, but a custom
    bus controller with protection features used with a standard 6502.

***************************************************************************/

#include "emu.h"
#include "deco16.h"
#include "deco16d.h"

#define DECO16_VERBOSE 1

DEFINE_DEVICE_TYPE(DECO16, deco16_device, "deco16", "Data East DECO16")

deco16_device::deco16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, DECO16, tag, owner, clock),
	io(nullptr),
	io_config("io", ENDIANNESS_LITTLE, 8, 16)
{
}

std::unique_ptr<util::disasm_interface> deco16_device::create_disassembler()
{
	return std::make_unique<deco16_disassembler>();
}

void deco16_device::device_start()
{
	mintf = std::make_unique<mi_default>();

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
