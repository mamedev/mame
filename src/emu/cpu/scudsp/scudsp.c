/*****************************************************************************
 *
 * SCUDSP CPU core
 *
 * skeleton for now ...
 *
 *****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "scudsp.h"


const device_type SCUDSP = &device_creator<scudsp_cpu_device>;


/* FLAGS */
#if 0
#define S  0x80
#define Z  0x40
#define OV 0x20
#define C  0x10
#endif


#define scudsp_readop(A) m_program->read_dword(A)
#define scudsp_readmem16(A) m_data->read_dword(A)
#define scudsp_writemem16(A,B) m_data->write_dword((A),B)


/***********************************
 *  illegal opcodes
 ***********************************/
void scudsp_cpu_device::scudsp_illegal()
{
	//logerror("scudsp illegal opcode at 0x%04x\n", m_pc);
	m_icount -= 1;
}

/* Execute cycles */
void scudsp_cpu_device::execute_run()
{
	UINT32 opcode;

	do
	{
		debugger_instruction_hook(this, m_pc);

		opcode = scudsp_readop(m_pc);
		m_pc++;

		switch( opcode )
		{
			default:
				scudsp_illegal();
				break;
		}

	} while( m_icount > 0 );
}


void scudsp_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);

	save_item(NAME(m_pc));
	save_item(NAME(m_flags));
	save_item(NAME(m_reset_state));

	// Register state for debugger
//	state_add( CP1610_R0, "PC", m_pc ).formatstr("%02X");
	state_add( STATE_GENPC, "curpc", m_pc ).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_flags ).noshow();

	m_icountptr = &m_icount;
}

void scudsp_cpu_device::device_reset()
{
	/* This is how we set the reset vector */
	//set_input_line(CP1610_RESET, PULSE_LINE);
}

void scudsp_cpu_device::execute_set_input(int irqline, int state)
{
	switch(irqline)
	{
		case SCUDSP_RESET:
			m_reset_state = state;
			break;
	}
}

scudsp_cpu_device::scudsp_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, SCUDSP, "SCUDSP", tag, owner, clock, "scudsp", __FILE__)
	, m_program_config("program", ENDIANNESS_BIG, 32, 8, -2)
	, m_data_config("data", ENDIANNESS_BIG, 32, 8, 0)
{
}


void scudsp_cpu_device::state_string_export(const device_state_entry &entry, astring &string)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			string.printf("%c%c%c%c",
				m_flags & 0x80 ? 'S':'.',
				m_flags & 0x40 ? 'Z':'.',
				m_flags & 0x20 ? 'V':'.',
				m_flags & 0x10 ? 'C':'.');
			break;
	}
}


offs_t scudsp_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( scudsp );
	return CPU_DISASSEMBLE_NAME(scudsp)(this, buffer, pc, oprom, opram, options);
}
