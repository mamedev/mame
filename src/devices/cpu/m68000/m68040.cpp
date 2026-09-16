// license:BSD-3-Clause
// copyright-holders:Karl Stenerud

#include "emu.h"
#include "m68040.h"
#include "m68kdasm.h"

DEFINE_DEVICE_TYPE(M68EC040,    m68ec040_device,    "m68ec040",     "Motorola MC68EC040")
DEFINE_DEVICE_TYPE(M68LC040,    m68lc040_device,    "m68lc040",     "Motorola MC68LC040")
DEFINE_DEVICE_TYPE(M68040,      m68040_device,      "m68040",       "Motorola MC68040")

std::unique_ptr<util::disasm_interface> m68ec040_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68040);
}

std::unique_ptr<util::disasm_interface> m68lc040_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68040);
}

std::unique_ptr<util::disasm_interface> m68040_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68040);
}

m68040_device::m68040_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_musashi_device(mconfig, tag, owner, clock, M68040, 32,32)
{
	m_has_fpu = true;
}


void m68040_device::device_start()
{
	m68000_musashi_device::device_start();
	init_cpu_m68040();
}



m68ec040_device::m68ec040_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_musashi_device(mconfig, tag, owner, clock, M68EC040, 32,32)
{
}

void m68ec040_device::device_start()
{
	m68000_musashi_device::device_start();
	init_cpu_m68ec040();
}



m68lc040_device::m68lc040_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_musashi_device(mconfig, tag, owner, clock, M68LC040, 32,32)
{
}

void m68lc040_device::device_start()
{
	m68000_musashi_device::device_start();
	init_cpu_m68lc040();
}
