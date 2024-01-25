// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
    Epson C33 STD Core (S1C33000)
*/

#include "emu.h"
#include "c33std.h"

#include "c33dasm.h"

#include <algorithm>
#include <iterator>
#include <utility>

#include "c33helpers.ipp"


c33std_cpu_device_base::c33std_cpu_device_base(
		machine_config const &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		u32 clock,
		address_map_constructor internal_map) :
	cpu_device(mconfig, type, tag, owner, clock),
	m_memory_config("memory", ENDIANNESS_LITTLE, 16, 28, 0, internal_map),
	m_icount(0)
{
}


void c33std_cpu_device_base::device_start()
{
	set_icountptr(m_icount);

	space(AS_PROGRAM).cache(m_opcodes);
	space(AS_PROGRAM).specific(m_data);

	std::fill(std::begin(m_gprs), std::end(m_gprs), 0);
	m_pc = 0;
	m_psr = 0;
	m_sp = 0;
	m_alr = 0;
	m_ahr = 0;

	save_item(NAME(m_gprs));
	save_item(NAME(m_pc));
	save_item(NAME(m_psr));
	save_item(NAME(m_sp));
	save_item(NAME(m_alr));
	save_item(NAME(m_ahr));

	state_add(STATE_GENPC,     "PC",       m_pc      ).mask(0xffff'fffe);
	state_add(STATE_GENPCBASE, "PCBASE",   m_pc      ).mask(0xffff'fffe).noshow();
	state_add(STATE_GENFLAGS,  "GENGLAGS", m_psr     ).mask(0x0000'0fdf).formatstr("%4s").noshow();
	state_add(C33_R0,          "R0",       m_gprs[0] );
	state_add(C33_R1,          "R1",       m_gprs[1] );
	state_add(C33_R2,          "R2",       m_gprs[2] );
	state_add(C33_R3,          "R3",       m_gprs[3] );
	state_add(C33_R4,          "R4",       m_gprs[4] );
	state_add(C33_R5,          "R5",       m_gprs[5] );
	state_add(C33_R6,          "R6",       m_gprs[6] );
	state_add(C33_R7,          "R7",       m_gprs[7] );
	state_add(C33_R8,          "R8",       m_gprs[8] );
	state_add(C33_R9,          "R9",       m_gprs[9] );
	state_add(C33_R10,         "R10",      m_gprs[10]);
	state_add(C33_R11,         "R11",      m_gprs[11]);
	state_add(C33_R12,         "R12",      m_gprs[12]);
	state_add(C33_R13,         "R13",      m_gprs[13]);
	state_add(C33_R14,         "R14",      m_gprs[14]);
	state_add(C33_R15,         "R15",      m_gprs[15]);
	state_add(C33_PSR,         "PSR",      m_psr     ).mask(0x0000'0fdf);
	state_add(C33_SP,          "SP",       m_sp      );
	state_add(C33_ALR,         "ALR",      m_alr     );
	state_add(C33_AHR,         "AHR",      m_ahr     );
}


void c33std_cpu_device_base::device_reset()
{
	m_psr = 0;
}


void c33std_cpu_device_base::execute_run()
{
	debugger_instruction_hook(m_pc);
	m_icount = 0;
}


device_memory_interface::space_config_vector c33std_cpu_device_base::memory_space_config() const
{
	return space_config_vector{ std::make_pair(AS_PROGRAM, &m_memory_config) };
}


std::unique_ptr<util::disasm_interface> c33std_cpu_device_base::create_disassembler()
{
	return std::make_unique<c33_disassembler>();
}


void c33std_cpu_device_base::state_string_export(device_state_entry const &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = util::string_format("%c%c%c%c",
				(m_psr & PSR_MASK_C) ? 'C' : 'c',
				(m_psr & PSR_MASK_V) ? 'V' : 'v',
				(m_psr & PSR_MASK_Z) ? 'Z' : 'z',
				(m_psr & PSR_MASK_N) ? 'N' : 'n');
		break;
	}
}
