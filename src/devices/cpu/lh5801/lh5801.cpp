// license:BSD-3-Clause
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 *
 *    02/2011 (Sandro Ronco)
 *   - Added IO_SPACE and updated all access in ME1 memory for use it.
 *   - Implemented interrupts.
 *   - Fixed the flags in the ROL/ROR/SHL/SHR opcodes.
 *   - Fixed decimal add/sub opcodes.
 *
 * based on info found on an artikel for the tandy trs80 pc2
 * and on "PC1500 Technical reference manual"
 *
 *****************************************************************************/

#include "emu.h"
#include "debugger.h"

#include "lh5801.h"

#define VERBOSE 0

#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)

enum
{
	LH5801_T=1,
	LH5801_P,
	LH5801_S,
	LH5801_U,
	LH5801_X,
	LH5801_Y,
	LH5801_A,

	LH5801_TM,
	LH5801_IN,
	LH5801_BF,
	LH5801_PU,
	LH5801_PV,
	LH5801_DP,
	LH5801_IRQ_STATE
};


#define P m_p.w.l
#define S m_s.w.l
#define U m_u.w.l
#define UL m_u.b.l
#define UH m_u.b.h
#define X m_x.w.l
#define XL m_x.b.l
#define XH m_x.b.h
#define Y m_y.w.l
#define YL m_y.b.l
#define YH m_y.b.h

#define C 0x01
#define IE 0x02
#define Z 0x04
#define V 0x08
#define H 0x10


const device_type LH5801 = &device_creator<lh5801_cpu_device>;


lh5801_cpu_device::lh5801_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, LH5801, "LH5801", tag, owner, clock, "lh5801", __FILE__)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_in_func(*this)
{
}


/***************************************************************
 * include the opcode macros, functions and tables
 ***************************************************************/
#include "5801tbl.inc"

void lh5801_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_io = &space(AS_IO);
	m_direct = &m_program->direct();

	m_in_func.resolve_safe(0);

	m_s.w.l = 0;
	m_p.w.l = 0;
	m_u.w.l = 0;
	m_x.w.l = 0;
	m_y.w.l = 0;
	m_tm = 0;
	m_t = 0;
	m_a = 0;
	m_bf = 0;
	m_dp = 0;
	m_pu = 0;
	m_pv = 0;
	m_oldpc = 0;
	m_irq_state = 0;
	memset(m_ir_flipflop, 0, sizeof(m_ir_flipflop));
	memset(m_lines_status, 0, sizeof(m_lines_status));
	m_idle = 0;

	save_item(NAME(m_s.w.l));
	save_item(NAME(m_p.w.l));
	save_item(NAME(m_u.w.l));
	save_item(NAME(m_x.w.l));
	save_item(NAME(m_y.w.l));
	save_item(NAME(m_tm));
	save_item(NAME(m_t));
	save_item(NAME(m_a));
	save_item(NAME(m_bf));
	save_item(NAME(m_dp));
	save_item(NAME(m_pu));
	save_item(NAME(m_pv));
	save_item(NAME(m_oldpc));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_ir_flipflop));
	save_item(NAME(m_lines_status));
	save_item(NAME(m_idle));

	state_add( LH5801_P,  "P",  m_p.w.l ).formatstr("%04X");
	state_add( LH5801_S,  "S",  m_s.w.l ).formatstr("%04X");
	state_add( LH5801_U,  "U",  m_u.w.l ).formatstr("%04X");
	state_add( LH5801_X,  "X",  m_x.w.l ).formatstr("%04X");
	state_add( LH5801_Y,  "Y",  m_y.w.l ).formatstr("%04X");
	state_add( LH5801_T,  "T",  m_t     ).formatstr("%02X");
	state_add( LH5801_A,  "A",  m_a     ).formatstr("%02X");
	state_add( LH5801_TM, "TM", m_tm    ).formatstr("%03X");
	state_add( LH5801_PV, "PV", m_pv    ).formatstr("%04X");
	state_add( LH5801_PU, "PU", m_pu    ).formatstr("%04X");
	state_add( LH5801_BF, "BF", m_bf    ).formatstr("%04X");
	state_add( LH5801_DP, "DP", m_dp    ).formatstr("%04X");

	state_add(STATE_GENPC,  "GENPC",  m_p.w.l).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_t).noshow().formatstr("%8s");

	m_icountptr = &m_icount;
}

void lh5801_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%c%c%c%c%c%c%c%c",
				m_t&0x80?'1':'0',
				m_t&0x40?'1':'0',
				m_t&0x20?'1':'0',
				m_t&0x10?'H':'.',
				m_t&0x08?'V':'.',
				m_t&0x04?'Z':'.',
				m_t&0x02?'I':'.',
				m_t&0x01?'C':'.');
			break;
	}
}

void lh5801_cpu_device::device_reset()
{
	P = (m_program->read_byte(0xfffe)<<8) | m_program->read_byte(0xffff);

	m_idle=0;

	memset(m_ir_flipflop, 0, sizeof(m_ir_flipflop));
	memset(m_lines_status, 0, sizeof(m_lines_status));
}


void lh5801_cpu_device::check_irq()
{
	if (m_ir_flipflop[0])
	{
		//NMI interrupt
		m_ir_flipflop[0] = 0;
		lh5801_push(m_t);
		m_t&=~IE;
		lh5801_push_word(P);
		P = (m_program->read_byte(0xfffc)<<8) | m_program->read_byte(0xfffd);
	}
	else if (m_ir_flipflop[1] && (m_t & IE))
	{
		//counter interrupt (counter not yet implemented)
		m_ir_flipflop[1] = 0;
		lh5801_push(m_t);
		m_t&=~IE;
		lh5801_push_word(P);
		P = (m_program->read_byte(0xfffa)<<8) | m_program->read_byte(0xfffb);
	}
	else if (m_ir_flipflop[2] && (m_t & IE))
	{
		//MI interrupt
		m_ir_flipflop[2] = 0;
		lh5801_push(m_t);
		m_t&=~IE;
		lh5801_push_word(P);
		P = (m_program->read_byte(0xfff8)<<8) | m_program->read_byte(0xfff9);
	}
}


void lh5801_cpu_device::execute_run()
{
	do
	{
		check_irq();

		if (m_idle)
			m_icount = 0;
		else
		{
			m_oldpc = P;

			debugger_instruction_hook(this, P);
			lh5801_instruction();
		}

	} while (m_icount > 0);
}

void lh5801_cpu_device::execute_set_input(int irqline, int state)
{
	switch( irqline)
	{
		case LH5801_LINE_MI:
			if (m_lines_status[0] == CLEAR_LINE && state == ASSERT_LINE)
			{
				m_idle = 0;
				m_ir_flipflop[2] = 1;
			}

			m_lines_status[0] = state;
			break;
		case INPUT_LINE_NMI:
			if (m_lines_status[1] == CLEAR_LINE && state == ASSERT_LINE)
			{
				m_idle = 0;
				m_ir_flipflop[0] = 1;
			}

			m_lines_status[1] = state;
			break;
	}
}

offs_t lh5801_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( lh5801 );
	return CPU_DISASSEMBLE_NAME(lh5801)(this, buffer, pc, oprom, opram, options);
}
