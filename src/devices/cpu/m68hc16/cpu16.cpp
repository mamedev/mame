// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Motorola MC68HC16/CPU16 emulation

    Currently this device is just a stub with no actual execution core.

***************************************************************************/

#include "emu.h"
#include "cpu16.h"
#include "cpu16dasm.h"

// device type definition
DEFINE_DEVICE_TYPE(MC68HC16Z1, mc68hc16z1_device, "mc68hc16z1", "Motorola MC68HC16Z1")

cpu16_device::cpu16_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 16, 20, 0, map)
	, m_data_config("data", ENDIANNESS_BIG, 16, 20, 0, map)
	, m_pc(0)
	, m_fwa(0)
	, m_fetch_pipe{0, 0, 0}
	, m_ccr(0)
	, m_d(0)
	, m_e(0)
	, m_ek(0)
	, m_index_regs{0, 0, 0}
	, m_sp(0)
	, m_hr(0)
	, m_ir(0)
	, m_am(0)
	, m_sl(false)
	, m_index_mask{0, 0}
	, m_icount(0)
{
}

mc68hc16z1_device::mc68hc16z1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cpu16_device(mconfig, MC68HC16Z1, tag, owner, clock, address_map_constructor(/* TODO */))
{
}

std::unique_ptr<util::disasm_interface> cpu16_device::create_disassembler()
{
	return std::make_unique<cpu16_disassembler>();
}

device_memory_interface::space_config_vector cpu16_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA, &m_data_config),
	};
}

void cpu16_device::debug_set_pcbase(u32 value)
{
	m_fwa = value;
	debug_set_pc((value + 6) & 0xffffe);
	// TODO: prefetch
}

void cpu16_device::debug_set_pc(u32 value)
{
	m_pc = value;
	m_ccr = (m_ccr & 0xfff0) | (value & 0xf0000) >> 16;
}

void cpu16_device::debug_set_ccr(u16 value)
{
	m_ccr = value;
	m_pc = (value & 0x000f) << 16 | (m_pc & 0x0ffff);
}

u16 cpu16_device::get_k() noexcept
{
	return u16(m_ek) << 12
			| (m_index_regs[0] & 0xf0000) >> 8
			| (m_index_regs[1] & 0xf0000) >> 12
			| (m_index_regs[2] & 0xf0000) >> 16;
}

void cpu16_device::set_k(u16 value) noexcept
{
	m_ek = (value & 0xf000) >> 12;
	m_index_regs[0] = u32(value & 0x0f00) << 8 | (m_index_regs[0] & 0x0ffff);
	m_index_regs[1] = u32(value & 0x00f0) << 12 | (m_index_regs[1] & 0x0ffff);
	m_index_regs[2] = u32(value & 0x000f) << 16 | (m_index_regs[2] & 0x0ffff);
}

void cpu16_device::device_start()
{
	space(AS_PROGRAM).cache(m_cache);
	space(has_space(AS_DATA) ? AS_DATA : AS_PROGRAM).specific(m_data);

	set_icountptr(m_icount);

	using namespace std::placeholders;
	state_add(CPU16_FWA, "FWA", m_fwa, std::bind(&cpu16_device::debug_set_pcbase, this, _1)).mask(0xffffe);
	state_add(CPU16_IPIPEC, "IPIPEC", m_fetch_pipe[2]);
	state_add(CPU16_IPIPEB, "IPIPEB", m_fetch_pipe[1]);
	state_add(CPU16_IPIPEA, "IPIPEA", m_fetch_pipe[0]);
	state_add(CPU16_D, "D", m_d);
	state_add<u8>(CPU16_A, "A",
		[this]() { return (m_d & 0xff00) >> 8; },
		[this](u8 value) { m_d = (m_d & 0x00ff) | u16(value) << 8; }).noshow();
	state_add<u8>(CPU16_B, "B",
		[this]() { return m_d & 0x00ff; },
		[this](u8 value) { m_d = (m_d & 0xff00) | value; }).noshow();
	state_add(CPU16_E, "E", m_e);
	for (int i = 0; i < 3; i++)
	{
		state_add(CPU16_X + i, util::string_format("%c", 'X' + i).c_str(), m_index_regs[i]).mask(0xfffff);
		state_add<u16>(CPU16_IX + i, util::string_format("I%c", 'X' + i).c_str(),
			[this, i]() { return m_index_regs[i] & 0x0ffff; },
			[this, i](u16 value) { m_index_regs[i] = (m_index_regs[i] & 0xf0000) | value; }).noshow();
		state_add<u8>(CPU16_XK + i, util::string_format("%cK", 'X' + i).c_str(),
			[this, i]() { return (m_index_regs[i] & 0xf0000) >> 16; },
			[this, i](u8 value) { m_index_regs[i] = (m_index_regs[i] & 0x0ffff) | u32(value) << 16; }).mask(0xf).noshow();
	}
	state_add(CPU16_SP, "SP", m_sp).mask(0xfffff);
	state_add<u8>(CPU16_SK, "SK",
		[this]() { return (m_sp & 0xf0000) >> 16; },
		[this](u8 value) { m_sp = (m_sp & 0x0ffff) | u32(value) << 16; }).mask(0xf).noshow();
	state_add(CPU16_PC, "PC", m_pc, std::bind(&cpu16_device::debug_set_pc, this, _1)).mask(0xffffe);
	state_add(STATE_GENPC, "GENPC", m_pc, std::bind(&cpu16_device::debug_set_pc, this, _1)).mask(0xffffe).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_fwa, std::bind(&cpu16_device::debug_set_pcbase, this, _1)).mask(0xffffe).noshow();
	state_add(CPU16_CCR, "CCR", m_ccr, std::bind(&cpu16_device::debug_set_ccr, this, _1));
	state_add(STATE_GENFLAGS, "CURFLAGS", m_ccr, std::bind(&cpu16_device::debug_set_ccr, this, _1)).noshow().formatstr("%13s");
	state_add(CPU16_EK, "EK", m_ek).mask(0xf);
	state_add<u16>(CPU16_K, "K", std::bind(&cpu16_device::get_k, this), std::bind(&cpu16_device::set_k, this, _1)).noshow();
	state_add(CPU16_HR, "HR", m_hr);
	state_add(CPU16_IR, "IR", m_ir);
	state_add(CPU16_AM, "AM", m_am).mask(0xfffffffff);
	state_add(CPU16_SL, "SL", m_sl);
	state_add(CPU16_XMSK, "XMSK", m_index_mask[0]);
	state_add(CPU16_YMSK, "YMSK", m_index_mask[1]);

	save_item(NAME(m_fwa));
	save_item(NAME(m_fetch_pipe));
	save_item(NAME(m_ccr));
	save_item(NAME(m_d));
	save_item(NAME(m_e));
	save_item(NAME(m_ek));
	save_item(NAME(m_index_regs));
	save_item(NAME(m_sp));
	save_item(NAME(m_pc));
	save_item(NAME(m_hr));
	save_item(NAME(m_ir));
	save_item(NAME(m_am));
	save_item(NAME(m_sl));
	save_item(NAME(m_index_mask));
}

void cpu16_device::device_reset()
{
	m_ccr = 0x80e0 | (m_ccr & 0x7f00); // IP, S, SM initialized
	for (u32 &i : m_index_regs)
		i &= 0x0ffff;
	m_ek = 0;
}

void cpu16_device::execute_run()
{
	u16 word0 = m_cache.read_word(0x00000); // reserved:ZK:SK:PK
	m_pc = m_cache.read_word(0x00002) | u32(word0 & 0x000f) << 16;
	m_sp = m_cache.read_word(0x00004) | u32(word0 & 0x00f0) << 12;
	m_index_regs[2] = m_cache.read_word(0x00006) | u32(word0 & 0x0f00) << 8;
	m_ccr = (m_ccr & 0xfff0) | (word0 & 0x000f);

	m_fwa = m_pc;
	debugger_instruction_hook(m_fwa);

	m_icount = 0;
}

void cpu16_device::execute_set_input(int inputnum, int state)
{
	// TODO
}

void cpu16_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = string_format("%c%c%c%c%c%c%c%c$%d %s",
			BIT(m_ccr, 15) ? '.' : 'S',
			BIT(m_ccr, 14) ? 'M' : '.',
			BIT(m_ccr, 13) ? 'H' : '.',
			BIT(m_ccr, 12) ? 'E' : '.',
			BIT(m_ccr, 11) ? 'N' : '.',
			BIT(m_ccr, 10) ? 'Z' : '.',
			BIT(m_ccr, 9) ? 'V' : '.',
			BIT(m_ccr, 8) ? 'C' : '.',
			BIT(m_ccr, 5, 3),
			BIT(m_ccr, 4) ? "SM" : "  ");
		break;
	}
}
