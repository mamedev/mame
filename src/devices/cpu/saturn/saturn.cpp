// license:BSD-3-Clause
// copyright-holders:Peter Trauner,Antoine Mine
/*****************************************************************************
 *
 *   saturn.c
 *   portable saturn emulator interface
 *   (hp calculators)
 *
 *
 *****************************************************************************/

#include "emu.h"
#include "debugger.h"

#include "saturn.h"

#define R0 0
#define R1 1
#define R2 2
#define R3 3
#define R4 4
#define A 5
#define B 6
#define C 7
#define D 8
#define I 9 // invalid


#define VERBOSE 0

#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)


// Hardware status bits
#define XM 1 // external Modules missing
#define SB 2 // Sticky bit
#define SR 4 // Service Request
#define MP 8 // Module Pulled



const device_type SATURN = &device_creator<saturn_device>;


saturn_device::saturn_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, SATURN, "HP Saturn", tag, owner, clock, "saturn_cpu", __FILE__)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 20, 0)
	, m_out_func(*this)
	, m_in_func(*this)
	, m_reset_func(*this)
	, m_config_func(*this)
	, m_unconfig_func(*this)
	, m_id_func(*this)
	, m_crc_func(*this)
	, m_rsi_func(*this), m_pc(0), m_oldpc(0), m_p(0), m_out(0), m_carry(0), m_decimal(0), m_st(0), m_hst(0), m_nmi_state(0), m_irq_state(0), m_irq_enable(0), m_in_irq(0),
	m_pending_irq(0), m_sleeping(0), m_monitor_id(0), m_monitor_in(0), m_program(nullptr), m_direct(nullptr), m_icount(0), m_debugger_temp(0)
{
}


offs_t saturn_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( saturn );
	return CPU_DISASSEMBLE_NAME(saturn)(this, buffer, pc, oprom, opram, options);
}


/***************************************************************
 * include the opcode macros, functions and tables
 ***************************************************************/

#include "satops.inc"
#include "sattable.inc"

/*****************************************************************************
 *
 *      Saturn CPU interface functions
 *
 *****************************************************************************/

void saturn_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();

	m_out_func.resolve_safe();
	m_in_func.resolve_safe(0);
	m_reset_func.resolve_safe();
	m_config_func.resolve_safe();
	m_unconfig_func.resolve_safe();
	m_id_func.resolve_safe(0);
	m_crc_func.resolve_safe();
	m_rsi_func.resolve_safe();

	memset(m_reg, 0, sizeof(m_reg));
	memset(m_d, 0, sizeof(m_d));
	m_pc = 0;
	m_oldpc = 0;
	memset(m_rstk, 0, sizeof(m_rstk));
	m_out = 0;
	m_carry = 0;
	m_decimal = 0;
	m_st = 0;
	m_hst = 0;
	m_nmi_state = 0;
	m_irq_state = 0;
	m_irq_enable = 0;
	m_in_irq = 0;
	m_pending_irq = 0;
	m_sleeping = 0;
	m_p = 0;

	save_item(NAME(m_reg[R0]));
	save_item(NAME(m_reg[R1]));
	save_item(NAME(m_reg[R2]));
	save_item(NAME(m_reg[R3]));
	save_item(NAME(m_reg[R4]));
	save_item(NAME(m_reg[A]));
	save_item(NAME(m_reg[B]));
	save_item(NAME(m_reg[C]));
	save_item(NAME(m_reg[D]));
	save_item(NAME(m_d));
	save_item(NAME(m_pc));
	save_item(NAME(m_oldpc));
	save_item(NAME(m_rstk));
	save_item(NAME(m_out));
	save_item(NAME(m_carry));
	save_item(NAME(m_st));
	save_item(NAME(m_hst));
	save_item(NAME(m_nmi_state));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_in_irq));
	save_item(NAME(m_pending_irq));
	save_item(NAME(m_sleeping));

	// TODO: Register state
	state_add( SATURN_PC, "PC", m_pc ).formatstr("%5X");
	state_add( SATURN_D0, "D0", m_d[0] ).formatstr("%5X");
	state_add( SATURN_D1, "D1", m_d[1] ).formatstr("%5X");
	state_add( SATURN_A, "A", m_debugger_temp ).formatstr("%19s");
	state_add( SATURN_B, "B", m_debugger_temp ).formatstr("%19s");
	state_add( SATURN_C, "C", m_debugger_temp ).formatstr("%19s");
	state_add( SATURN_D, "D", m_debugger_temp ).formatstr("%19s");
	state_add( SATURN_R0, "R0", m_debugger_temp ).formatstr("%19s");
	state_add( SATURN_R1, "R1", m_debugger_temp ).formatstr("%19s");
	state_add( SATURN_R2, "R2", m_debugger_temp ).formatstr("%19s");
	state_add( SATURN_R3, "R3", m_debugger_temp ).formatstr("%19s");
	state_add( SATURN_R4, "R4", m_debugger_temp ).formatstr("%19s");
	state_add( SATURN_P, "P", m_p).formatstr("%1X");
	state_add( SATURN_OUT, "OUTP", m_out).formatstr("%3X");
	state_add( SATURN_CARRY, "Carry", m_carry).formatstr("%1X");
	state_add( SATURN_ST, "ST", m_st).formatstr("%4X");
	state_add( SATURN_HST, "HST", m_hst).formatstr("%1X");
	state_add( SATURN_RSTK0, "RSTK0", m_rstk[0]).formatstr("%5X");
	state_add( SATURN_RSTK1, "RSTK1", m_rstk[1]).formatstr("%5X");
	state_add( SATURN_RSTK2, "RSTK2", m_rstk[2]).formatstr("%5X");
	state_add( SATURN_RSTK3, "RSTK3", m_rstk[3]).formatstr("%5X");
	state_add( SATURN_RSTK4, "RSTK4", m_rstk[4]).formatstr("%5X");
	state_add( SATURN_RSTK5, "RSTK5", m_rstk[5]).formatstr("%5X");
	state_add( SATURN_RSTK6, "RSTK6", m_rstk[6]).formatstr("%5X");
	state_add( SATURN_RSTK7, "RSTK7", m_rstk[7]).formatstr("%5X");
	state_add( SATURN_IRQ_STATE, "IRQ", m_debugger_temp).formatstr("%4s");
	state_add( SATURN_SLEEPING, "sleep", m_sleeping).formatstr("%1X");

	state_add( STATE_GENPC, "GENPC", m_pc ).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_debugger_temp).formatstr("%2s").noshow();

	m_icountptr = &m_icount;
}

void saturn_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
#define Reg64Data(s) s[15],s[14],s[13],s[12],s[11],s[10],s[9],s[8],s[7],s[6],s[5],s[4],s[3],s[2],s[1],s[0]
#define Reg64Format "%x %x%x%x%x%x%x%x %x%x%x %x%x%x%x%x"

	switch (entry.index())
	{
		case SATURN_A:
			strprintf(str,  Reg64Format, Reg64Data(m_reg[A]) );
			break;

		case SATURN_B:
			strprintf(str,  Reg64Format, Reg64Data(m_reg[B]) );
			break;

		case SATURN_C:
			strprintf(str,  Reg64Format, Reg64Data(m_reg[C]) );
			break;

		case SATURN_D:
			strprintf(str,  Reg64Format, Reg64Data(m_reg[D]) );
			break;

		case SATURN_R0:
			strprintf(str,  Reg64Format, Reg64Data(m_reg[R0]) );
			break;

		case SATURN_R1:
			strprintf(str,  Reg64Format, Reg64Data(m_reg[R1]) );
			break;

		case SATURN_R2:
			strprintf(str,  Reg64Format, Reg64Data(m_reg[R2]) );
			break;

		case SATURN_R3:
			strprintf(str,  Reg64Format, Reg64Data(m_reg[R3]) );
			break;

		case SATURN_R4:
			strprintf(str,  Reg64Format, Reg64Data(m_reg[R4]) );
			break;

		case SATURN_IRQ_STATE:
			strprintf(str,  "%c%c%c%i", m_in_irq?'S':'.', m_irq_enable?'e':'.', m_pending_irq?'p':'.', m_irq_state );
			break;

		case STATE_GENFLAGS:
			strprintf(str,  "%c%c", m_decimal?'D':'.', m_carry ? 'C':'.' );
			break;
	}
}


void saturn_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case SATURN_A:
			IntReg64(m_reg[A], m_debugger_temp);
			break;

		case SATURN_B:
			IntReg64(m_reg[B], m_debugger_temp);
			break;

		case SATURN_C:
			IntReg64(m_reg[C], m_debugger_temp);
			break;

		case SATURN_D:
			IntReg64(m_reg[D], m_debugger_temp);
			break;

		case SATURN_R0:
			IntReg64(m_reg[R0], m_debugger_temp);
			break;

		case SATURN_R1:
			IntReg64(m_reg[R1], m_debugger_temp);
			break;

		case SATURN_R2:
			IntReg64(m_reg[R2], m_debugger_temp);
			break;

		case SATURN_R3:
			IntReg64(m_reg[R3], m_debugger_temp);
			break;

		case SATURN_R4:
			IntReg64(m_reg[R4], m_debugger_temp);
			break;
	}
}

void saturn_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case SATURN_A:
			m_debugger_temp = Reg64Int(m_reg[A]);
			break;

		case SATURN_B:
			m_debugger_temp = Reg64Int(m_reg[B]);
			break;

		case SATURN_C:
			m_debugger_temp = Reg64Int(m_reg[C]);
			break;

		case SATURN_D:
			m_debugger_temp = Reg64Int(m_reg[D]);
			break;

		case SATURN_R0:
			m_debugger_temp = Reg64Int(m_reg[R0]);
			break;

		case SATURN_R1:
			m_debugger_temp = Reg64Int(m_reg[R1]);
			break;

		case SATURN_R2:
			m_debugger_temp = Reg64Int(m_reg[R2]);
			break;

		case SATURN_R3:
			m_debugger_temp = Reg64Int(m_reg[R3]);
			break;

		case SATURN_R4:
			m_debugger_temp = Reg64Int(m_reg[R4]);
			break;
	}
}


void saturn_device::device_reset()
{
	m_pc=0;
	m_sleeping = 0;
	m_irq_enable = 0;
	m_in_irq = 0;
}


void saturn_device::saturn_take_irq()
{
	m_in_irq = 1;       /* reset by software, using RTI */
	m_pending_irq = 0;
	m_icount -= 7;
	saturn_push(m_pc);
	m_pc=IRQ_ADDRESS;

	LOG(("Saturn '%s' takes IRQ ($%04x)\n", tag(), m_pc));

	standard_irq_callback(SATURN_IRQ_LINE);
}

void saturn_device::execute_run()
{
	do
	{
		m_oldpc = m_pc;

		debugger_instruction_hook(this, m_pc);

		if ( m_sleeping )
		{
			/* advance time when sleeping */
			m_icount -= 100;
		}
		else
		{
			/* takes irq */
			if ( m_pending_irq && (!m_in_irq) )
				saturn_take_irq();

			/* execute one instruction */
			saturn_instruction();
		}

	} while (m_icount > 0);
}


void saturn_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
		case SATURN_NMI_LINE:
			if ( state == m_nmi_state ) return;
			m_nmi_state = state;
			if ( state != CLEAR_LINE )
			{
				LOG(( "SATURN '%s' set_nmi_line(ASSERT)\n", tag()));
				m_pending_irq = 1;
			}
			break;

		case SATURN_IRQ_LINE:
			if ( state == m_irq_state ) return;
			m_irq_state = state;
			if ( state != CLEAR_LINE && m_irq_enable )
			{
				LOG(( "SATURN '%s' set_irq_line(ASSERT)\n", tag()));
				m_pending_irq = 1;
			}
			break;

		case SATURN_WAKEUP_LINE:
			if (m_sleeping && state==1)
			{
				LOG(( "SATURN '%s' set_wakeup_line(ASSERT)\n", tag()));
				standard_irq_callback(SATURN_WAKEUP_LINE);
				m_sleeping = 0;
			}
			break;
	}
}


void saturn_device::IntReg64(Saturn64 r, INT64 d)
{
	int i;
	for (i=0; i<16; i++)
		r[i] = (d >> (4*i)) & 0xf;
}


INT64 saturn_device::Reg64Int(Saturn64 r)
{
	INT64 x = 0;
	int i;
	for (i=0; i<16; i++)
		x |= (INT64) r[i] << (4*i);
	return x;
}
