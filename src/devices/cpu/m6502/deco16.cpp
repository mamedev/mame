// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    deco16.c

    6502, reverse-engineered DECO variant

***************************************************************************/

#include "emu.h"
#include "deco16.h"

#define DECO16_VERBOSE 1

DEFINE_DEVICE_TYPE(DECO16, deco16_device, "deco16", "DECO16")

deco16_device::deco16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, DECO16, tag, owner, clock),
	io(nullptr),
	io_config("io", ENDIANNESS_LITTLE, 8, 16)
{
}

offs_t deco16_device::disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options)
{
	return disassemble_generic(stream, pc, oprom, opram, options, disasm_entries);
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

const address_space_config *deco16_device::memory_space_config(address_spacenum spacenum) const
{
	switch(spacenum)
	{
	case AS_PROGRAM:           return &program_config;
	case AS_IO:                return &io_config;
	case AS_DECRYPTED_OPCODES: return has_configured_map(AS_DECRYPTED_OPCODES) ? &sprogram_config : nullptr;
	default:                   return nullptr;
	}
}

#include "cpu/m6502/deco16.hxx"
