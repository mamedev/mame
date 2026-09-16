// license:BSD-3-Clause
// copyright-holders:Karl Stenerud

#include "emu.h"
#include "m68010.h"
#include "m68kdasm.h"

DEFINE_DEVICE_TYPE(M68010,      m68010_device,      "m68010",       "Motorola MC68010")

std::unique_ptr<util::disasm_interface> m68010_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68010);
}


m68010_device::m68010_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68010_device(mconfig, M68010, tag, owner, clock)
{
}

m68010_device::m68010_device(const machine_config &mconfig, const device_type type, const char *tag, device_t *owner, u32 clock)
	: m68000_musashi_device(mconfig, tag, owner, clock, type, 16,24)
{
}

m68010_device::m68010_device(const machine_config &mconfig, const device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor internal_map)
	: m68000_musashi_device(mconfig, tag, owner, clock, type, 16,24, internal_map)
{
}

void m68010_device::device_start()
{
	m68000_musashi_device::device_start();
	init_cpu_m68010();
}
