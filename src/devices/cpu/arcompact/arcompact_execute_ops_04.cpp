// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"
#include "arcompactdasm.h"


uint32_t arcompact_device::handleop32_FLAG(uint32_t op)
{
	// leapster bios uses formats for FLAG that are not defined, bug I guess work anyway (P modes 0 / 1)
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x29], /*"FLAG"*/ 1,1);
}
