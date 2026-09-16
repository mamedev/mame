// license:BSD-3-Clause
// copyright-holders:Karl Stenerud

#include "emu.h"
#include "m68030.h"
#include "m68kdasm.h"

DEFINE_DEVICE_TYPE(M68EC030,    m68ec030_device,    "m68ec030",     "Motorola MC68EC030")
DEFINE_DEVICE_TYPE(M68030,      m68030_device,      "m68030",       "Motorola MC68030")

std::unique_ptr<util::disasm_interface> m68ec030_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68030);
}

std::unique_ptr<util::disasm_interface> m68030_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68030);
}

m68030_device::m68030_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_musashi_device(mconfig, tag, owner, clock, M68030, 32, 32)
{
	m_has_fpu = true;
}

void m68030_device::device_start()
{
	m68000_musashi_device::device_start();
	init_cpu_m68030();
}

m68ec030_device::m68ec030_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_musashi_device(mconfig, tag, owner, clock, M68EC030, 32,32)
{
}

void m68ec030_device::device_start()
{
	m68000_musashi_device::device_start();
	init_cpu_m68ec030();
}
