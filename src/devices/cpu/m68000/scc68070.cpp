// license:BSD-3-Clause
// copyright-holders:Karl Stenerud

#include "emu.h"
#include "scc68070.h"
#include "m68kdasm.h"

std::unique_ptr<util::disasm_interface> scc68070_base_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68000);
}


scc68070_base_device::scc68070_base_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock,
						const device_type type, address_map_constructor internal_map)
	: m68000_musashi_device(mconfig, tag, owner, clock, type, 16,32, internal_map)
{
}

void scc68070_base_device::device_start()
{
	m68000_musashi_device::device_start();
	init_cpu_scc68070();
}
