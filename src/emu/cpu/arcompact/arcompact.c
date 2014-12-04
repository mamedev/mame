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

UINT32 arcompact_device::READ32(UINT32 address)
{
	return m_program->read_dword(address << 2);
}

void arcompact_device::WRITE32(UINT32 address, UINT32 data)
{
	m_program->write_dword(address << 2, data);
}

UINT16 arcompact_device::READ16(UINT32 address)
{
	return m_program->read_word(address << 1);
}

void arcompact_device::WRITE16(UINT32 address, UINT16 data)
{
	m_program->write_word(address << 1, data);
}


/*****************************************************************************/

void arcompact_device::device_start()
{
	m_pc = 0;

	m_debugger_temp = 0;

	m_program = &space(AS_PROGRAM);

	state_add( 0,  "PC", m_debugger_temp).callimport().callexport().formatstr("%08X");
	state_add(STATE_GENPC, "GENPC", m_debugger_temp).callexport().noshow();

	m_icountptr = &m_icount;
}

void arcompact_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case 0:
			m_debugger_temp = m_pc << 1;
			break;

		case STATE_GENPC:
			m_debugger_temp = m_pc << 1;
			break;
	}
}

void arcompact_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case 0:
			m_pc = (m_debugger_temp & 0xfffffffe) >> 1;
			break;
	}
}

void arcompact_device::device_reset()
{
	m_pc = 0x00000000;
}

/*****************************************************************************/

void arcompact_device::execute_set_input(int irqline, int state)
{

}


void arcompact_device::execute_run()
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
