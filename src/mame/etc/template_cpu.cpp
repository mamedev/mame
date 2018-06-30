// license:BSD-3-Clause
// copyright-holders:<author_name>
/*****************************************************************************
 *
 * template for CPU cores
 *
 *****************************************************************************/

#include "emu.h"
#include "xxx.h"
#include "debugger.h"


DEFINE_DEVICE_TYPE(XXX, xxx_cpu_device, "xxx", "XXX")


/* FLAGS */
#if 0
#define S  0x80
#define Z  0x40
#define OV 0x20
#define C  0x10
#endif


#define xxx_readop(A) m_program->read_dword(A)
#define xxx_readmem16(A) m_data->read_dword(A)
#define xxx_writemem16(A,B) m_data->write_dword((A),B)


/***********************************
 *  illegal opcodes
 ***********************************/
void xxx_cpu_device::xxx_illegal()
{
	logerror("xxx illegal opcode at 0x%04x\n", m_pc);
	m_icount -= 1;
}

/* Execute cycles */
void cp1610_cpu_device::execute_run()
{
	uint16_t opcode;

	do
	{
		debugger_instruction_hook(this, m_pc);

		opcode = xxx_readop(m_pc);
		m_pc++;

		switch( opcode )
		{
			default:
				xxx_illegal();
				break;
		}

	} while( m_icount > 0 );
}


void xxx_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);

	save_item(NAME(m_pc));
	save_item(NAME(m_flags));

	// Register state for debugger
	state_add( CP1610_R0, "PC", m_pc ).formatstr("%02X");
	state_add( STATE_GENPC, "GENPC", m_r[7] ).noshow();
	state_add( STATE_GENPCBASE, "CURPC", m_r[7] ).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_flags ).noshow();

	m_icountptr = &m_icount;
}

#if 0
void xxx_cpu_device::execute_set_input(int irqline, int state)
{
	switch(irqline)
	{
		case XXX_INT_INTRM: // level-sensitive
			m_intrm_pending = ((ASSERT_LINE == state) || (HOLD_LINE == state));
			m_intrm_state = (ASSERT_LINE == state);
			break;
		case XXX_RESET: // edge-sensitive
			if (CLEAR_LINE != state)
				m_reset_pending = 1;
			m_reset_state = (ASSERT_LINE == state);
			break;
		case XXX_INT_INTR: // edge-sensitive
			if (CLEAR_LINE != state)
				m_intr_pending = 1;
			m_intr_state = (ASSERT_LINE == state);
			break;
	}
}
#endif

xxx_cpu_device::xxx_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, XXX, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 8, 32, -1)
	, m_data_config("data", ENDIANNESS_BIG, 8, 32, 0)
{
}


void xxx_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = util::string_format("%c%c%c%c",
					m_flags & 0x80 ? 'S':'.',
					m_flags & 0x40 ? 'Z':'.',
					m_flags & 0x20 ? 'V':'.',
					m_flags & 0x10 ? 'C':'.');
			break;
	}
}


offs_t xxx_cpu_device::disassemble(char *buffer, offs_t pc, const uint32_t *oprom, const uint32_t *opram, uint32_t options)
{
	return CPU_DISASSEMBLE_NAME(xxx)(this, buffer, pc, opcodes, params, options);
}
