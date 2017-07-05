// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "h8s2600.h"

h8s2600_device::h8s2600_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_delegate map_delegate) :
	h8s2000_device(mconfig, type, tag, owner, clock, map_delegate)
{
}

offs_t h8s2600_device::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params, uint32_t options)
{
	return disassemble_generic(stream, pc, opcodes, params, options, disasm_entries);
}

#include "cpu/h8/h8s2600.hxx"
