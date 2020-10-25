// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    DEC VT61 CPU

    Currently this device is just a stub with no actual execution core.

***************************************************************************/

#include "emu.h"
#include "vt61.h"
#include "vt61dasm.h"

// device type definition
DEFINE_DEVICE_TYPE(VT61_CPU, vt61_cpu_device, "vt61_cpu", "DEC VT61 CPU")

vt61_cpu_device::vt61_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, VT61_CPU, tag, owner, clock)
	, m_program_config("microprogram", ENDIANNESS_LITTLE, 16, 10, -1)
	, m_memory_config("memory", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_idr_config("IDR", ENDIANNESS_LITTLE, 8, 6, 0)
	, m_pc(0)
	, m_ac(0)
	, m_mar(0)
	, m_mdr(0)
	, m_ir(0)
	, m_sp{0}
	, m_icount(0)
	, m_misc_flags(0)
	, m_modem_flags(0)
	, m_intrpt_control(0)
{
	m_program_config.m_is_octal = true;
	m_memory_config.m_is_octal = true;
	m_idr_config.m_is_octal = true;
}

std::unique_ptr<util::disasm_interface> vt61_cpu_device::create_disassembler()
{
	return std::make_unique<vt61_disassembler>();
}

device_memory_interface::space_config_vector vt61_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA, &m_memory_config),
		std::make_pair(AS_IDR, &m_idr_config)
	};
}

void vt61_cpu_device::device_start()
{
	space(AS_PROGRAM).cache(m_program_cache);
	space(AS_DATA).cache(m_memory_cache);
	space(AS_IDR).cache(m_idr_cache);

	set_icountptr(m_icount);

	state_add(VT61_PC, "PC", m_pc).mask(01777).formatstr("%5s");
	state_add(STATE_GENPC, "GENPC", m_pc).mask(01777).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).mask(01777).noshow();
	state_add(VT61_AC, "AC", m_ac).formatstr("%03O");
	state_add(VT61_MAR, "MAR", m_mar).formatstr("%06O");
	state_add<u8>(VT61_MALO, "MALO",
		[this]() { return m_mar & 000377; },
		[this](u8 data) { m_mar = (m_mar & 0177400) | data; }
	).noshow();
	state_add<u8>(VT61_MAHI, "MAHI",
		[this]() { return (m_mar & 0177400) >> 8; },
		[this](u8 data) { m_mar = (m_mar & 000377) | (data << 8); }
	).noshow();
	state_add(VT61_MDR, "MDR", m_mdr).formatstr("%03O");
	state_add(VT61_IR, "IR", m_ir).mask(6);
	for (int i = 0; i < 16; i++)
		state_add(VT61_R0 + i, string_format("R%d", i).c_str(), m_sp[i]).formatstr("%03O");
	state_add(VT61_MISC, "MISC", m_misc_flags).formatstr("%03O");
	state_add(VT61_MOD, "MOD", m_modem_flags).formatstr("%02O").mask(017);
	state_add(VT61_INTRC, "INTRC", m_intrpt_control).formatstr("%02O").mask(017);

	save_item(NAME(m_pc));
	save_item(NAME(m_ac));
	save_item(NAME(m_mar));
	save_item(NAME(m_mdr));
	save_item(NAME(m_ir));
	save_item(NAME(m_sp));
	save_item(NAME(m_misc_flags));
	save_item(NAME(m_modem_flags));
	save_item(NAME(m_intrpt_control));
}

void vt61_cpu_device::device_reset()
{
	m_pc = 0;

	m_misc_flags = 0;
	m_modem_flags = 0;
	m_intrpt_control = 0;
}

void vt61_cpu_device::execute_run()
{
	debugger_instruction_hook(m_pc);

	m_icount = 0;
}

void vt61_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case VT61_PC:
		str = string_format("%o:%03o", m_pc >> 8, m_pc & 0377);
		break;
	}
}
