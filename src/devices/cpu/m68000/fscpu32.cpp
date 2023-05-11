// license:BSD-3-Clause
// copyright-holders:Karl Stenerud

#include "emu.h"
#include "fscpu32.h"
#include "m68kdasm.h"

std::unique_ptr<util::disasm_interface> fscpu32_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_CPU32);
}

fscpu32_device::fscpu32_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock,
						const device_type type, address_map_constructor internal_map)
	: m68000_musashi_device(mconfig, tag, owner, clock, type, 16,32, internal_map)
{
}


void fscpu32_device::device_start()
{
	m68000_musashi_device::device_start();
	init_cpu_fscpu32();
}
