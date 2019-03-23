// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*****************************************************************************
*
*   ns32000.cpp
*
*   NS32000 CPU family
*
*****************************************************************************/

#include "emu.h"
#include "ns32000.h"
#include "ns32000dasm.h"
#include "debugger.h"

//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

DEFINE_DEVICE_TYPE(NS32008, ns32008_cpu_device, "ns32008", "National Semiconductor NS32008")
DEFINE_DEVICE_TYPE(NS32016, ns32016_cpu_device, "ns32016", "National Semiconductor NS32016")
DEFINE_DEVICE_TYPE(NS32032, ns32032_cpu_device, "ns32032", "National Semiconductor NS32032")


//-------------------------------------------------
//  ns32000_cpu_device - constructor
//-------------------------------------------------

ns32000_cpu_device::ns32000_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int databits, int addrbits)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, databits, addrbits, 0)
	, m_icount(0)
{
}

ns32008_cpu_device::ns32008_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ns32000_cpu_device(mconfig, NS32008, tag, owner, clock, 8, 24)
{
}

ns32016_cpu_device::ns32016_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ns32000_cpu_device(mconfig, NS32016, tag, owner, clock, 16, 24)
{
}

ns32032_cpu_device::ns32032_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ns32000_cpu_device(mconfig, NS32032, tag, owner, clock, 32, 24)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ns32000_cpu_device::device_start()
{
	set_icountptr(m_icount);

	save_item(NAME(m_pc));
	save_item(NAME(m_r));
	save_item(NAME(m_f));

	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();

	// dedicated registers
	state_add(NS32000_PC, "PC", m_pc);
	state_add(NS32000_SB, "SB", m_sb);
	state_add(NS32000_FP, "FP", m_fp);
	state_add(NS32000_SP1, "SP1", m_sp1);
	state_add(NS32000_SP0, "SP0", m_sp0);
	state_add(NS32000_INTBASE, "INTBASE", m_intbase);
	state_add(NS32000_PSR, "PSR", m_psr);
	state_add(NS32000_MOD, "MOD", m_mod);

	// general registers
	for (unsigned i = 0; i < 8; i++)
		state_add(NS32000_R0 + i, util::string_format("R%d", i).c_str(), m_r[i]);

	// floating point registers
	//for (unsigned i = 0; i < 8; i++)
	//  state_add(NS32000_R7 + i, util::string_format("F%d", i).c_str(), m_f[i]);

	// set our instruction counter
	//set_icountptr(m_icount);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ns32000_cpu_device::device_reset()
{
	m_nmi_line = false;
	m_irq_line = false;

	m_pc = 0;
}

//-------------------------------------------------
//  execute_run - execute a timeslice's worth of
//  opcodes
//-------------------------------------------------

void ns32000_cpu_device::execute_run()
{
	while (m_icount > 0)
	{
		debugger_instruction_hook(m_pc);

		m_icount--;
	}
}

//-------------------------------------------------
//  execute_set_input - act on a changed input/
//  interrupt line
//-------------------------------------------------

void ns32000_cpu_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
	case INPUT_LINE_NMI:
		// NMI is edge triggered
		m_nmi_line = m_nmi_line || (state == ASSERT_LINE);
		break;

	case INPUT_LINE_IRQ0:
		// IRQ is line triggered
		m_irq_line = state == ASSERT_LINE;
		break;
	}
}

//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the specified address space, or nullptr if
//  the space doesn't exist
//-------------------------------------------------

device_memory_interface::space_config_vector ns32000_cpu_device::memory_space_config() const
{
	return space_config_vector{ std::make_pair(AS_PROGRAM, &m_program_config) };
}

bool ns32000_cpu_device::memory_translate(int spacenum, int intention, offs_t &address)
{
	return true;
}

//-------------------------------------------------
//  disassemble - call the disassembly
//  helper function
//-------------------------------------------------

std::unique_ptr<util::disasm_interface> ns32000_cpu_device::create_disassembler()
{
	return std::make_unique<ns32000_disassembler>();
}
