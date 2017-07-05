// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "h8h.h"

h8h_device::h8h_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_delegate map_delegate) :
	h8_device(mconfig, type, tag, owner, clock, false, map_delegate)
{
	supports_advanced = true;
	mode_advanced = true;
}

offs_t h8h_device::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params, uint32_t options)
{
	return disassemble_generic(stream, pc, opcodes, params, options, disasm_entries);
}

#include "cpu/h8/h8h.hxx"
