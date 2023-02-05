// license:BSD-3-Clause
// copyright-holders:Karl Stenerud

#include "emu.h"
#include "m68000.h"
#include "m68kdasm.h"

DEFINE_DEVICE_TYPE(M68000,      m68000_device,      "m68000",       "Motorola MC68000")

std::unique_ptr<util::disasm_interface> m68000_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68000);
}

m68000_device::m68000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_device(mconfig, M68000, tag, owner, clock)
{
}

m68000_device::m68000_device(const machine_config &mconfig, const device_type type, const char *tag, device_t *owner, u32 clock)
	: m68000_musashi_device(mconfig, tag, owner, clock, type, 16,24)
{
}

void m68000_device::device_start()
{
	m68000_musashi_device::device_start();
	init_cpu_m68000();
}

m68000_device::m68000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock,
										const device_type type, u32 prg_data_width, u32 prg_address_bits, address_map_constructor internal_map)
	: m68000_musashi_device(mconfig, tag, owner, clock, type, prg_data_width, prg_address_bits, internal_map)
{
}
