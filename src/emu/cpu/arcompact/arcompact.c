/*********************************\

 ARCompact Core

 The following procesors use the ARCompact instruction set

  - ARCtangent-A5
  - ARC 600
  - ARC 700

 (this is a skeleton core)

 ARCompact is a 32-bit CPU that freely mixes 32-bit and 16-bit instructions
 various user customizations could be made as with the ARC A4 based processors
 these include custom instructions and registers.

\*********************************/

#include "emu.h"
#include "debugger.h"
#include "arcompact.h"
#include "arcompact_common.h"


const device_type ARCA5 = &device_creator<arcompact_device>;

arcompact_device::arcompact_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, ARCA5, "ARCtangent-A5", tag, owner, clock, "arca5", __FILE__)
	, m_program_config("program", ENDIANNESS_LITTLE, 32, 32, 0) // some docs describe these as 'middle endian'?!
{
}


offs_t arcompact_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( arcompact );
	return CPU_DISASSEMBLE_NAME(arcompact)(this, buffer, pc, oprom, opram, options);
}


/*****************************************************************************/

/*****************************************************************************/

void arcompact_device::unimplemented_opcode(UINT16 op)
{
	fatalerror("ARCOMPACT: unknown opcode %04x at %04x\n", op, m_pc << 2);
}

/*****************************************************************************/


/*****************************************************************************/

void arcompact_device::device_start()
{
	m_pc = 0;

	m_debugger_temp = 0;

	m_program = &space(AS_PROGRAM);

	state_add( 0,  "PC", m_debugger_temp).callimport().callexport().formatstr("%08X");

	state_add( 0x10,  "STATUS32", m_debugger_temp).callimport().callexport().formatstr("%08X");

	state_add(STATE_GENPC, "GENPC", m_debugger_temp).callexport().noshow();

	for (int i = 0x100; i < 0x140; i++)
	{
		state_add(i, regnames[i-0x100], m_debugger_temp).callimport().callexport().formatstr("%08X");
	}


	m_icountptr = &m_icount;
}

void arcompact_device::state_export(const device_state_entry &entry)
{
	int index = entry.index();

	switch (index)
	{
		case 0:
			m_debugger_temp = m_pc;
			break;

		case 0x10:
			m_debugger_temp = m_status32;
			break;

		case STATE_GENPC:
			m_debugger_temp = m_pc;
			break;

		default:
			if ((index >= 0x100) && (index < 0x140))
			{
				m_debugger_temp = m_regs[index - 0x100];
			}
			break;

	}
}

void arcompact_device::state_import(const device_state_entry &entry)
{
	int index = entry.index();

	switch (index)
	{
		case 0:
			m_pc = (m_debugger_temp & 0xfffffffe);
			break;

		case 0x10:
			m_status32 = m_debugger_temp;
			break;

		default:
			if ((index >= 0x100) && (index < 0x140))
			{
				m_regs[index - 0x100] = m_debugger_temp;
			}
			break;
	}
}

void arcompact_device::device_reset()
{
	m_pc = 0x00000000;

	m_delayactive = 0;
	m_delayjump = 0x00000000;

	for (int i = 0; i < 0x40; i++)
		m_regs[i] = 0;

	m_status32 = 0;
}

/*****************************************************************************/

void arcompact_device::execute_set_input(int irqline, int state)
{

}
