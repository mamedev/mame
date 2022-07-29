// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "h8s2600.h"
#include "h8s2600d.h"

h8s2600_device::h8s2600_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor map_delegate) :
	h8s2000_device(mconfig, type, tag, owner, clock, map_delegate)
{
	has_mac = true;
}

std::unique_ptr<util::disasm_interface> h8s2600_device::create_disassembler()
{
	return std::make_unique<h8s2600_disassembler>();
}

#include "cpu/h8/h8s2600.hxx"
