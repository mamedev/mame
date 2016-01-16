// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\

 ARCtangent (A4) core
 ARC == Argonaut RISC Core

 (this is a skeleton core)

\*********************************/

#include "emu.h"
#include "debugger.h"
#include "arc.h"


const device_type ARC = &device_creator<arc_device>;


arc_device::arc_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, ARC, "ARCtangent A4", tag, owner, clock, "arc", __FILE__)
	, m_program_config("program", ENDIANNESS_BIG, 32, 24, 0), m_pc(0), m_program(nullptr), m_icount(0), m_debugger_temp(0)
// some docs describe these as 'middle endian'?!
{
}


offs_t arc_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( arc );
	return CPU_DISASSEMBLE_NAME(arc)(this, buffer, pc, oprom, opram, options);
}


/*****************************************************************************/

/*****************************************************************************/

void arc_device::unimplemented_opcode(UINT16 op)
{
	fatalerror("arc: unknown opcode %04x at %04x\n", op, m_pc << 2);
}

/*****************************************************************************/

UINT32 arc_device::READ32(UINT32 address)
{
	return m_program->read_dword(address << 2);
}

void arc_device::WRITE32(UINT32 address, UINT32 data)
{
	m_program->write_dword(address << 2, data);
}

/*****************************************************************************/

void arc_device::device_start()
{
	m_pc = 0;

	m_debugger_temp = 0;

	m_program = &space(AS_PROGRAM);

	state_add( 0,  "PC", m_debugger_temp).callimport().callexport().formatstr("%08X");
	state_add(STATE_GENPC, "GENPC", m_debugger_temp).callexport().noshow();

	m_icountptr = &m_icount;
}

void arc_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case 0:
			m_debugger_temp = m_pc << 2;
			break;

		case STATE_GENPC:
			m_debugger_temp = m_pc << 2;
			break;
	}
}

void arc_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case 0:
			m_pc = (m_debugger_temp & 0xfffffffc) >> 2;
			break;
	}
}

void arc_device::device_reset()
{
	m_pc = 0x00000000;
}

/*****************************************************************************/

void arc_device::execute_set_input(int irqline, int state)
{
}


void arc_device::execute_run()
{
	//UINT32 lres;
	//lres = 0;

	while (m_icount > 0)
	{
		debugger_instruction_hook(this, m_pc<<2);

		//UINT32 op = READ32(m_pc);

		m_pc++;

		m_icount--;
	}

}
