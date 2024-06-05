// license:BSD-3-Clause
// copyright-holders:Karl Stenerud

#include "emu.h"
#include "m68000musashi.h"
#include "m68kdasm.h"

DEFINE_DEVICE_TYPE(M68000MUSASHI,      m68000msh_device,      "m68000msh",       "Motorola MC68000 (Musashi)")

std::unique_ptr<util::disasm_interface> m68000msh_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68010);
}


m68000msh_device::m68000msh_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_musashi_device(mconfig, tag, owner, clock, M68000MUSASHI, 16, 24)
{
}

void m68000msh_device::device_start()
{
	m68000_musashi_device::device_start();
	init_cpu_m68000();
}
