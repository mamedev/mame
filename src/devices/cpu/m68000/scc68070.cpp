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

	m_readimm16 = [this](offs_t address) -> u16  { return m_oprogram16.read_word(translate_addr(address)); };
	m_read8   = [this](offs_t address) -> u8     { return m_program16.read_byte(translate_addr(address)); };
	m_read16  = [this](offs_t address) -> u16    { return m_program16.read_word(translate_addr(address)); };
	m_read32  = [this](offs_t address) -> u32    { return m_program16.read_dword(translate_addr(address)); };
	m_write8  = [this](offs_t address, u8 data)
		{
			address = translate_addr(address);
			m_program16.write_word(address & ~1, data | (data << 8), address & 1 ? 0x00ff : 0xff00);
		};
	m_write16 = [this](offs_t address, u16 data) { m_program16.write_word(translate_addr(address), data); };
	m_write32 = [this](offs_t address, u32 data) { m_program16.write_dword(translate_addr(address), data); };
}

offs_t scc68070_base_device::translate_addr(offs_t address) const
{
	// internal addresses (0x80000000-bfffffff) are only accessible in supervisor mode;
	// all other accesses use the external 24-bit address bus.
	if (!(supervisor_mode() && (address >> 30) == 0x2))
		address &= 0xffffff;

	return address;
}

bool scc68070_base_device::memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space)
{
	target_space = &space(spacenum);
	if (spacenum == AS_PROGRAM)
		address = translate_addr(address);
	return true;
}
