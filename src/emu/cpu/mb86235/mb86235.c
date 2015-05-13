// license:BSD-3-Clause
// copyright-holders:Angelo Salese, ElSemi
/*****************************************************************************
 *
 * MB86235 "TGPx4" (c) Fujitsu
 *
 * Written by Angelo Salese & ElSemi
 *
 * TODO:
 * - Everything!
 *
 *****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "mb86235.h"


const device_type MB86235 = &device_creator<mb86235_cpu_device>;





#define mb86235_readop(A) m_program->read_dword(A)
#define mb86235_readmem(A) m_program->read_dword(A)
#define mb86235_writemem(A,B) m_program->write_dword((A),B)


/***********************************
 *  illegal opcodes
 ***********************************/
void mb86235_cpu_device::mb86235_illegal()
{
	//logerror("mb86235 illegal opcode at 0x%04x\n", m_pc);
	m_icount -= 1;
}

/* Execute cycles */
void mb86235_cpu_device::execute_run()
{
	UINT32 opcode;

	do
	{
		debugger_instruction_hook(this, m_pc);

		opcode = mb86235_readop(m_pc);
		//m_pc++;

		switch( opcode )
		{
			default:
				mb86235_illegal();
				break;
		}

	} while( m_icount > 0 );
}


void mb86235_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);

	save_item(NAME(m_pc));
	save_item(NAME(m_flags));

	// Register state for debugger
	//state_add( CP1610_R0, "PC", m_pc ).formatstr("%02X");
	state_add( STATE_GENPC, "curpc", m_pc ).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_flags ).noshow();

	m_icountptr = &m_icount;
}

void mb86235_cpu_device::device_reset()
{
}

#if 0
void mb86235_cpu_device::execute_set_input(int irqline, int state)
{
	switch(irqline)
	{
		case MB86235_INT_INTRM:
			m_intrm_pending = (state == ASSERT_LINE);
			m_intrm_state = state;
			break;
		case MB86235_RESET:
			if (state == ASSERT_LINE)
				m_reset_pending = 1;
			m_reset_state = state;
			break;
		case MB86235_INT_INTR:
			if (state == ASSERT_LINE)
				m_intr_pending = 1;
			m_intr_state = state;
			break;
	}
}
#endif

mb86235_cpu_device::mb86235_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, MB86235, "MB86235", tag, owner, clock, "mb86235", __FILE__)
	, m_program_config("program", ENDIANNESS_LITTLE, 32, 32, -2)
{
}


void mb86235_cpu_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%c%c%c%c",
				m_flags & 0x80 ? 'S':'.',
				m_flags & 0x40 ? 'Z':'.',
				m_flags & 0x20 ? 'V':'.',
				m_flags & 0x10 ? 'C':'.');
			break;
	}
}

offs_t mb86235_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( mb86235 );
	return CPU_DISASSEMBLE_NAME(mb86235)(this, buffer, pc, oprom, opram, options);
}
