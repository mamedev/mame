// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Motorola M88000 RISC microprocessors

    Currently this device is just a stub with no actual execution core.

***************************************************************************/

#include "emu.h"
#include "m88000.h"
#include "m88000d.h"

// device type definitions
DEFINE_DEVICE_TYPE(MC88100, mc88100_device, "mc88100", "Motorola MC88100")

mc88100_device::mc88100_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, MC88100, tag, owner, clock)
	, m_code_config("code", ENDIANNESS_BIG, 32, 32, 0)
	, m_data_config("data", ENDIANNESS_BIG, 32, 32, 0)
	, m_inst_cache(nullptr)
	, m_data_space(nullptr)
	, m_pc(0)
	, m_r{0}
	, m_cr{0}
	, m_icount(0)
{
	m_cr[0] = 0x00000001;
}

std::unique_ptr<util::disasm_interface> mc88100_device::create_disassembler()
{
	return std::make_unique<mc88100_disassembler>();
}

device_memory_interface::space_config_vector mc88100_device::memory_space_config() const
{
	// MC88100 has physically separate code and data buses
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_code_config),
		std::make_pair(AS_DATA, &m_data_config)
	};
}

void mc88100_device::device_start()
{
	m_inst_cache = space(AS_PROGRAM).cache<2, 0, ENDIANNESS_BIG>();
	m_data_space = &space(AS_DATA);

	set_icountptr(m_icount);

	state_add(M88000_PC, "PC", m_pc);
	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();
	state_add(STATE_GENFLAGS, "CURFLAGS", m_cr[1]).mask(0xf00003ff).noshow();
	for (int i = 1; i < 32; i++)
		state_add(M88000_R1 + i - 1, string_format("r%d", i).c_str(), m_r[i]);
	state_add(M88000_PSR, "PSR", m_cr[1]).mask(0xf00003ff);
	state_add(M88000_VBR, "VBR", m_cr[7]).mask(0xfffff000);
}

void mc88100_device::device_reset()
{
	m_pc = 0;
	m_cr[1] = 0x800003fb;
	m_cr[7] = 0;
}

void mc88100_device::execute_run()
{
	debugger_instruction_hook(m_pc);

	m_icount = 0;
}

void mc88100_device::execute_set_input(int inputnum, int state)
{
	// TODO
}
