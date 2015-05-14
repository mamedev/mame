// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "h8h.h"

h8h_device::h8h_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, address_map_delegate map_delegate) :
	h8_device(mconfig, type, name, tag, owner, clock, shortname, source, false, map_delegate)
{
	supports_advanced = true;
	mode_advanced = true;
}

offs_t h8h_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	return disassemble_generic(buffer, pc, oprom, opram, options, disasm_entries);
}

#include "cpu/h8/h8h.inc"
