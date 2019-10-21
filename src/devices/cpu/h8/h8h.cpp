// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "h8h.h"
#include "h8hd.h"

h8h_device::h8h_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor map_delegate) :
	h8_device(mconfig, type, tag, owner, clock, map_delegate)
{
	supports_advanced = true;
	mode_advanced = true;
}

std::unique_ptr<util::disasm_interface> h8h_device::create_disassembler()
{
	return std::make_unique<h8h_disassembler>();
}

#include "cpu/h8/h8h.hxx"
