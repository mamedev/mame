// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "h8s2000.h"
#include "h8s2000d.h"

h8s2000_device::h8s2000_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor map_delegate) :
	h8h_device(mconfig, type, tag, owner, clock, map_delegate)
{
	m_has_exr = true;
}

std::unique_ptr<util::disasm_interface> h8s2000_device::create_disassembler()
{
	return std::make_unique<h8s2000_disassembler>();
}

#include "cpu/h8/h8s2000.hxx"
