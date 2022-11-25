// license:BSD-3-Clause
// copyright-holders:Karl Stenerud

#include "emu.h"
#include "m68008.h"
#include "m68kdasm.h"

DEFINE_DEVICE_TYPE(M68008,      m68008_device,      "m68008",       "Motorola MC68008") // 48-pin plastic or ceramic DIP
DEFINE_DEVICE_TYPE(M68008FN,    m68008fn_device,    "m68008fn",     "Motorola MC68008FN") // 52-pin PLCC

std::unique_ptr<util::disasm_interface> m68008_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68008);
}

std::unique_ptr<util::disasm_interface> m68008fn_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68008);
}

m68008_device::m68008_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_musashi_device(mconfig, tag, owner, clock, M68008, 8,20)
{
}

void m68008_device::device_start()
{
	m68000_musashi_device::device_start();
	init_cpu_m68008();
}


m68008fn_device::m68008fn_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_musashi_device(mconfig, tag, owner, clock, M68008FN, 8,22)
{
}

void m68008fn_device::device_start()
{
	m68000_musashi_device::device_start();
	init_cpu_m68008();
}
