// license:BSD-3-Clause
// copyright-holders:Sterophonick
/*****************************************************************************
 *
 * Skeleton device for Gigatron CPU Core
 *
 *****************************************************************************/

#include "emu.h"
#include "gigatron.h"
#include "debugger.h"


DEFINE_DEVICE_TYPE(GTRON, gigatron_cpu_device, "gigatron", "Gigatron")


#define gigatron_readop(A) m_program->read_dword(A)
#define gigatron_readmem16(A) m_data->read_dword(A)
#define gigatron_writemem16(A,B) m_data->write_dword((A),B)


/***********************************
 *  illegal opcodes
 ***********************************/
void gigatron_cpu_device::gigatron_illegal()
{
	logerror("gigatron illegal opcode at 0x%04x\n", m_pc);
	m_icount -= 1;
}

/* Execute cycles */
void gigatron_cpu_device::execute_run()
{
	uint16_t opcode;
	do
	{
		debugger_instruction_hook(this, m_pc);

		opcode = gigatron_readop(m_pc);
		m_pc++;

		switch (opcode)
		{
			default:
				gigatron_illegal();
				break;
		}

	} while (m_icount > 0);
}


void gigatron_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);

	save_item(NAME(m_pc));
	save_item(NAME(m_flags));

	// Register state for debugger
	state_add( GTRON_AC, "PC", m_pc ).formatstr("%02X");
	state_add( STATE_GENPC, "GENPC", m_r[7] ).noshow();
	state_add( STATE_GENPCBASE, "CURPC", m_r[7] ).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_flags ).noshow();

	m_icountptr = &m_icount;
}

#if 0
void gigatron_cpu_device::execute_set_input(int irqline, int state)
{
	switch(irqline)
	{
		case gigatron_INT_INTRM: // level-sensitive
			m_intrm_pending = ((ASSERT_LINE == state) || (HOLD_LINE == state));
			m_intrm_state = (ASSERT_LINE == state);
			break;
		case gigatron_RESET: // edge-sensitive
			if (CLEAR_LINE != state)
				m_reset_pending = 1;
			m_reset_state = (ASSERT_LINE == state);
			break;
		case gigatron_INT_INTR: // edge-sensitive
			if (CLEAR_LINE != state)
				m_intr_pending = 1;
			m_intr_state = (ASSERT_LINE == state);
			break;
	}
}
#endif

gigatron_cpu_device::gigatron_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, GTRON, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 8, 32, -1)
	, m_data_config("data", ENDIANNESS_BIG, 8, 32, 0)
{
}
