// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Fujitsu FR series

    Currently this device is just a stub with no actual execution core.

***************************************************************************/

#include "emu.h"
#include "fr.h"
#include "frdasm.h"

// device type definitions
DEFINE_DEVICE_TYPE(MB91101A, mb91101a_device, "mb91101a", "Fujitsu MB91101A")

fr_cpu_device::fr_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int addrbits, address_map_constructor map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_space_config("program", ENDIANNESS_BIG, 32, addrbits, 0, map)
	, m_space(nullptr)
	, m_cache(nullptr)
	, m_regs{0}
	, m_pc(0)
	, m_ps(0)
	, m_tbr(0)
	, m_rp(0)
	, m_md(0)
	, m_icount(0)
{
}

mb91101a_device::mb91101a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: fr_cpu_device(mconfig, MB91101A, tag, owner, clock, 32, address_map_constructor())
{
}

std::unique_ptr<util::disasm_interface> fr_cpu_device::create_disassembler()
{
	return std::make_unique<fr_disassembler>();
}

device_memory_interface::space_config_vector fr_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_space_config),
	};
}

void fr_cpu_device::device_start()
{
	m_space = &space(AS_PROGRAM);
	m_cache = m_space->cache<2, 0, ENDIANNESS_BIG>();

	set_icountptr(m_icount);

	state_add(FR_PC, "PC", m_pc).mask(0xfffffffe);
	state_add(STATE_GENPC, "GENPC", m_pc).mask(0xfffffffe).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).mask(0xfffffffe).noshow();
	state_add(FR_PS, "PS", m_ps).mask(0x001f073f).formatstr("%08X");
	state_add(STATE_GENFLAGS, "CURFLAGS", m_ps).mask(0x001f073f).noshow().formatstr("%19s");
	state_add<u8>(FR_CCR, "CCR",
		[this]() { return u8(m_ps & 0x0000003f); },
		[this](u8 value) { m_ps = (m_ps & 0x001f0700) | value; }).mask(0x3f).noshow();
	state_add<u8>(FR_ILM, "ILM",
		[this]() { return u8((m_ps & 0x001f0000) >> 16); },
		[this](u8 value) { m_ps = (m_ps & 0x0000073f) | u32(value) << 16; }).mask(0x1f).noshow();
	state_add(FR_TBR, "TBR", m_tbr);
	state_add(FR_RP, "RP", m_rp);
	state_add(FR_SSP, "SSP", m_regs[15]);
	state_add(FR_USP, "USP", m_regs[16]);
	state_add(FR_MD, "MD", m_md);
	state_add<u32>(FR_MDH, "MDH",
		[this]() { return u32(m_md >> 32); },
		[this](u32 value) { m_md = (m_md & 0x00000000ffffffffULL) | u64(value) <<  32; }).noshow();
	state_add<u32>(FR_MDL, "MDL",
		[this]() { return u32(m_md & 0x00000000ffffffffULL); },
		[this](u32 value) { m_md = (m_md & 0xffffffff00000000ULL) | value; }).noshow();
	for (int i = 0; i < 15; i++)
		state_add(FR_R0 + i, string_format("R%d", i).c_str(), m_regs[i]);
	state_add<u32>(FR_R15, "R15",
		[this]() { return m_regs[BIT(m_ps, 5) ? 16 : 15]; },
		[this](u32 value) { m_regs[BIT(m_ps, 5) ? 16 : 15] = value; });

	save_item(NAME(m_regs));
	save_item(NAME(m_pc));
	save_item(NAME(m_ps));
	save_item(NAME(m_tbr));
	save_item(NAME(m_rp));
	save_item(NAME(m_md));
}

void fr_cpu_device::device_reset()
{
	// Only TBR, SSP, ILM and certain CCR and SCR bits are reset here; PC will be reloaded subsequently
	m_ps = (m_ps & 0x0000060f) | 0x000f0000;
	m_tbr = 0x000ffc00;
	m_regs[15] = 0x00000000;
}

void fr_cpu_device::execute_run()
{
	m_pc = m_space->read_dword(m_tbr + 0x3fc);

	debugger_instruction_hook(m_pc);

	m_icount = 0;
}

void fr_cpu_device::execute_set_input(int inputnum, int state)
{
	// TODO
}

void fr_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = string_format("<%d%d%d%d%d> %c%c%c--%c%c%c%c%c%c",
				BIT(m_ps, 20),
				BIT(m_ps, 19),
				BIT(m_ps, 18),
				BIT(m_ps, 17),
				BIT(m_ps, 16),
				BIT(m_ps, 10) ? 'D' : '.',
				BIT(m_ps, 9) ? 'd' : '.',
				BIT(m_ps, 8) ? 'T' : '.',
				BIT(m_ps, 5) ? 'S' : '.',
				BIT(m_ps, 4) ? 'I' : '.',
				BIT(m_ps, 3) ? 'N' : '.',
				BIT(m_ps, 2) ? 'Z' : '.',
				BIT(m_ps, 1) ? 'V' : '.',
				BIT(m_ps, 0) ? 'C' : '.');
		break;
	}
}
