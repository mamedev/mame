// license:BSD-3-Clause
// copyright-holders:Karl Stenerud

#include "emu.h"
#include "m68020.h"
#include "m68kdasm.h"

DEFINE_DEVICE_TYPE(M68EC020,    m68ec020_device,    "m68ec020",     "Motorola MC68EC020")
DEFINE_DEVICE_TYPE(M68020,      m68020_device,      "m68020",       "Motorola MC68020")
DEFINE_DEVICE_TYPE(M68020FPU,   m68020fpu_device,   "m68020fpu",    "Motorola MC68020FPU")
DEFINE_DEVICE_TYPE(M68020PMMU,  m68020pmmu_device,  "m68020pmmu",   "Motorola MC68020PMMU")
DEFINE_DEVICE_TYPE(M68020HMMU,  m68020hmmu_device,  "m68020hmmu",   "Motorola MC68020HMMU")

std::unique_ptr<util::disasm_interface> m68ec020_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68020);
}

std::unique_ptr<util::disasm_interface> m68020_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68020);
}

std::unique_ptr<util::disasm_interface> m68020fpu_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68020);
}

std::unique_ptr<util::disasm_interface> m68020pmmu_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68020);
}

std::unique_ptr<util::disasm_interface> m68020hmmu_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68020);
}

m68020_device::m68020_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_musashi_device(mconfig, tag, owner, clock, M68020, 32,32)
{
}

void m68020_device::device_start()
{
	m68000_musashi_device::device_start();
	init_cpu_m68020();
}


m68020fpu_device::m68020fpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_musashi_device(mconfig, tag, owner, clock, M68020FPU, 32,32)
{
}

void m68020fpu_device::device_start()
{
	m68000_musashi_device::device_start();
	init_cpu_m68020fpu();
}

// 68020 with 68851 PMMU
m68020pmmu_device::m68020pmmu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_musashi_device(mconfig, tag, owner, clock, M68020PMMU, 32,32)
{
}

void m68020pmmu_device::device_start()
{
	m68000_musashi_device::device_start();
	init_cpu_m68020pmmu();
}

bool m68020hmmu_device::memory_translate(int space, int intention, offs_t &address)
{
	/* only applies to the program address space and only does something if the MMU's enabled */
	{
		if ((space == AS_PROGRAM) && (m_hmmu_enabled))
		{
			address = hmmu_translate_addr(address);
		}
	}
	return true;
}


// 68020 with Apple HMMU & 68881 FPU
//      case CPUINFO_FCT_TRANSLATE: info->translate = CPU_TRANSLATE_NAME(m68khmmu);     break;
m68020hmmu_device::m68020hmmu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_musashi_device(mconfig, tag, owner, clock, M68020HMMU, 32,32)
{
}

void m68020hmmu_device::device_start()
{
	m68000_musashi_device::device_start();
	init_cpu_m68020hmmu();
}


m68ec020_device::m68ec020_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_musashi_device(mconfig, tag, owner, clock, M68EC020, 32,24)
{
}

void m68ec020_device::device_start()
{
	m68000_musashi_device::device_start();
	init_cpu_m68ec020();
}

