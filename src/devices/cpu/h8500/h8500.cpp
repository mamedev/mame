// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Hitachi H8/500 Series

    Currently this device is just a stub with no actual execution core.

***************************************************************************/

#include "emu.h"
#include "h8500.h"
#include "h8500dasm.h"

h8500_device::h8500_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int addrbits, int buswidth, int ramsize, int defmode, address_map_constructor map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, buswidth, addrbits, 0, map)
	, m_mode_control(defmode)
	, m_ram_size((1 << ramsize) - 1)
	, m_pc(0)
	, m_ppc(0)
	, m_sr(0)
	, m_cp(0)
	, m_dp(0)
	, m_ep(0)
	, m_tp(0)
	, m_br(0)
	, m_r{0, 0, 0, 0, 0, 0, 0, 0}
	, m_icount(0)
{
}

void h8500_device::device_config_complete()
{
	if (!h8_maximum_mode())
		m_program_config.m_addr_width = 16;
}

std::unique_ptr<util::disasm_interface> h8500_device::create_disassembler()
{
	return std::make_unique<h8500_disassembler>(h8_maximum_mode());
}

device_memory_interface::space_config_vector h8500_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

void h8500_device::debug_set_pc(offs_t pc) noexcept
{
	m_pc = m_ppc = pc & 0xffff;
	if (h8_maximum_mode())
		m_cp = u8(pc >> 16);
}

void h8500_device::device_start()
{
	m_program = &space(AS_PROGRAM);

	set_icountptr(m_icount);

	// Control registers
	state_add(H8500_PC, "PC", m_pc);
	if (h8_maximum_mode())
	{
		state_add<u32>(STATE_GENPC, "GENPC", [this]() { return u32(m_cp) << 16 | m_pc; }, [this](u32 data) { debug_set_pc(data); }).noshow().mask(0xffffff);
		state_add<u32>(STATE_GENPCBASE, "CURPC", [this]() { return u32(m_cp) << 16 | m_ppc; }, [this](u32 data) { debug_set_pc(data); }).noshow().mask(0xffffff);
	}
	else
	{
		state_add<u16>(STATE_GENPC, "GENPC", [this]() { return m_pc; }, [this](u16 data) { debug_set_pc(data); }).noshow();
		state_add<u16>(STATE_GENPCBASE, "CURPC", [this]() { return m_ppc; }, [this](u16 data) { debug_set_pc(data); }).noshow();
	}
	state_add(H8500_SR, "SR", m_sr).mask(0x870f);
	state_add<u8>(H8500_CCR, "CCR", [this]() { return m_sr & 0xff; }, [this](u8 data) { m_sr = (m_sr & 0xff00) | data; }).mask(0x0f).noshow();
	state_add<u8>(STATE_GENFLAGS, "FLAGS", [this]() { return m_sr & 0xff; }, [this](u8 data) { m_sr = (m_sr & 0xff00) | data; }).mask(0x0f).formatstr("%4s").noshow();
	if (h8_maximum_mode())
	{
		state_add(H8500_CP, "CP", m_cp);
		state_add(H8500_DP, "DP", m_dp);
		state_add(H8500_EP, "EP", m_ep);
		state_add(H8500_TP, "TP", m_tp);
	}
	state_add(H8500_BR, "BR", m_br);

	// General registers
	for (int n = 0; n < 6; n++)
		state_add(H8500_R0 + n, string_format("R%d", n).c_str(), m_r[n]);
	state_add(H8500_FP, "FP", m_r[6]);
	state_add(H8500_SP, "SP", m_r[7]);

	// Save state
	save_item(NAME(m_pc));
	save_item(NAME(m_ppc));
	save_item(NAME(m_sr));
	if (h8_maximum_mode())
		save_item(NAME(m_cp));
	save_item(NAME(m_dp));
	save_item(NAME(m_ep));
	save_item(NAME(m_tp));
	save_item(NAME(m_br));
	save_item(NAME(m_r));
}

void h8500_device::device_reset()
{
	m_sr = 0x0700 | (m_sr & 0x000f);
}

void h8500_device::execute_run()
{
	if (h8_maximum_mode())
	{
		m_cp = m_program->read_word(0) & 0x00ff;
		m_pc = m_program->read_word(2);
	}
	else
		m_pc = m_program->read_word(0);

	m_ppc = m_pc;
	debugger_instruction_hook(m_cp << 8 | m_pc);

	m_icount = 0;
}

void h8500_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = string_format("%c%c%c%c",
			BIT(m_sr, 3) ? 'N' : '.',
			BIT(m_sr, 2) ? 'Z' : '.',
			BIT(m_sr, 1) ? 'V' : '.',
			BIT(m_sr, 0) ? 'C' : '.');
		break;
	}
}
