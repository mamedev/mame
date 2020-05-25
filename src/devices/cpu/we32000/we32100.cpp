// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    AT&T WE32100 32-Bit Microprocessor

    Currently this device is just a stub with no actual execution core.

***************************************************************************/

#include "emu.h"
#include "we32100.h"
#include "we32100d.h"

// device type definitions
DEFINE_DEVICE_TYPE(WE32100, we32100_device, "we32100", "AT&T WE32100")

we32100_device::we32100_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, WE32100, tag, owner, clock)
	, m_space_config("program", ENDIANNESS_BIG, 32, 32, 0)
	, m_r{0}
	, m_icount(0)
{
}

std::unique_ptr<util::disasm_interface> we32100_device::create_disassembler()
{
	return std::make_unique<we32100_disassembler>();
}

device_memory_interface::space_config_vector we32100_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_space_config),
	};
}

void we32100_device::device_start()
{
	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_space);

	set_icountptr(m_icount);

	state_add(STATE_GENPC, "GENPC", m_r[15]).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_r[15]).noshow();
	state_add(STATE_GENFLAGS, "CURFLAGS", m_r[11]).noshow().formatstr("%23s");
	for (int i = 0; i < 9; i++)
		state_add(WE_R0 + i, string_format("r%d", i).c_str(), m_r[i]);
	for (int i = 9; i < 15; i++)
		state_add(WE_R0 + i, string_format("r%d", i).c_str(), m_r[i]).noshow();
	state_add(WE_FP, "FP", m_r[9]);
	state_add(WE_AP, "AP", m_r[10]);
	state_add(WE_PSW, "PSW", m_r[11]);
	state_add(WE_SP, "SP", m_r[12]);
	state_add(WE_PCBP, "PCBP", m_r[13]);
	state_add(WE_ISP, "ISP", m_r[14]);
	state_add(WE_PC, "PC", m_r[15]);

	save_item(NAME(m_r));
}

void we32100_device::device_reset()
{
	// TODO
}

void we32100_device::execute_run()
{
	// On-reset sequence: load PCBP, then load PSW, PC and SP from there
	m_r[13] = m_space.read_dword(0x00000080);
	m_r[11] = m_space.read_dword(m_r[13]);
	m_r[15] = m_space.read_dword(m_r[13] + 4);
	m_r[12] = m_space.read_dword(m_r[13] + 8);

	debugger_instruction_hook(m_r[15]);

	m_icount = 0;
}

void we32100_device::execute_set_input(int inputnum, int state)
{
	// TODO
}

void we32100_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = string_format("%c%c%c%c%c%c%c%c%c<%X>%c(%c)%c%c<%X>%c%c",
				BIT(m_r[11], 25) ? 'f' : '.',
				BIT(m_r[11], 24) ? 'Q' : '.',
				BIT(m_r[11], 23) ? 'c' : '.',
				BIT(m_r[11], 22) ? 'O' : '.',
				BIT(m_r[11], 21) ? 'N' : '.',
				BIT(m_r[11], 20) ? 'Z' : '.',
				BIT(m_r[11], 19) ? 'V' : '.',
				BIT(m_r[11], 18) ? 'C' : '.',
				BIT(m_r[11], 17) ? 'T' : '.',
				(m_r[11] & 0x0001e000) >> 13,
				"KESU"[(m_r[11] & 0x00001800) >> 11],
				"KESU"[(m_r[11] & 0x00000600) >> 9],
				BIT(m_r[11], 8) ? 'R' : '.',
				BIT(m_r[11], 7) ? 'I' : '.',
				(m_r[11] & 0x00000078) >> 3,
				BIT(m_r[11], 2) ? 'T' : '.',
				"RPSN"[m_r[11] & 0x00000003]);
		break;
	}
}
