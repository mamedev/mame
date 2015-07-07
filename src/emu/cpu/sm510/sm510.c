// license:BSD-3-Clause
// copyright-holders:hap
/*

  Sharp SM510 MCU family - known chips:
  - SM510: x
  - SM511: x
  - SM512: x

  References:
  - 1990 Sharp Microcomputers Data Book
  
  TODO:
  - proper support for LFSR program counter in debugger

*/

#include "sm510.h"
#include "debugger.h"


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

enum
{
	SM510_PC=1, SM510_ACC, SM510_BL, SM510_BM,
	SM510_C, SM510_W
};

void sm510_base_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);
	m_prgmask = (1 << m_prgwidth) - 1;
	m_datamask = (1 << m_datawidth) - 1;

	m_div_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(sm510_base_device::div_timer_cb), this));
	reset_divider();

	// resolve callbacks
	m_read_k.resolve_safe(0);
	m_read_ba.resolve_safe(1);
	m_read_b.resolve_safe(1);
	m_write_s.resolve_safe();
	m_write_r.resolve_safe();

	// zerofill
	memset(m_stack, 0, sizeof(m_stack));
	m_pc = 0;
	m_prev_pc = 0;
	m_op = 0;
	m_prev_op = 0;
	m_param = 0;
	m_acc = 0;
	m_bl = 0;
	m_bm = 0;
	m_c = 0;
	m_skip = false;
	m_w = 0;
//	m_div = 0;
	m_1s = false;
	m_k_active = false;
	m_bp = false;
	m_bc = false;
	m_halt = false;

	// register for savestates
	save_item(NAME(m_stack));
	save_item(NAME(m_pc));
	save_item(NAME(m_prev_pc));
	save_item(NAME(m_op));
	save_item(NAME(m_prev_op));
	save_item(NAME(m_param));
	save_item(NAME(m_acc));
	save_item(NAME(m_bl));
	save_item(NAME(m_bm));
	save_item(NAME(m_c));
	save_item(NAME(m_skip));
	save_item(NAME(m_w));
	save_item(NAME(m_div));
	save_item(NAME(m_1s));
	save_item(NAME(m_k_active));
	save_item(NAME(m_bp));
	save_item(NAME(m_bc));
	save_item(NAME(m_halt));

	// register state for debugger
	state_add(SM510_PC,  "PC",  m_pc).formatstr("%04X");
	state_add(SM510_ACC, "ACC", m_acc).formatstr("%01X");
	state_add(SM510_BL,  "BL",  m_bl).formatstr("%01X");
	state_add(SM510_BM,  "BM",  m_bm).formatstr("%01X");
	state_add(SM510_C,   "C",   m_c).formatstr("%01X");
	state_add(SM510_W,   "W",   m_w).formatstr("%02X");

	state_add(STATE_GENPC, "curpc", m_pc).formatstr("%04X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_c).formatstr("%1s").noshow();

	m_icountptr = &m_icount;
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sm510_base_device::device_reset()
{
	m_skip = false;
	m_halt = false;
	m_op = m_prev_op = 0;
	do_branch(3, 7, 0);
	m_prev_pc = m_pc;
	
	// lcd is on (Bp on, BC off)
	m_bp = true;
	m_bc = false;
	
	m_write_r(0, 0, 0xff);
	// y=0(bs), r=0
}



//-------------------------------------------------
//  interrupt/timer
//-------------------------------------------------

void sm510_base_device::wake_me_up()
{
	// in halt mode, wake up after 1S signal or K input
	if (m_halt)
	{
		m_halt = false;
		do_branch(1, 0, 0);
		
		standard_irq_callback(0);

		// note: official doc warns that Bl/Bm and the stack are undefined
		// after waking up, but we leave it unchanged
	}
}

void sm510_base_device::execute_set_input(int line, int state)
{
	if (line != 0)
		return;
	
	// set K input lines active state
	m_k_active = (state != 0);
}

TIMER_CALLBACK_MEMBER(sm510_base_device::div_timer_cb)
{
	// no need to increment it by 1 everytime, since only the
	// highest bits are accessible
	m_div = (m_div + 0x800) & 0x7fff;
	
	// 1S signal on overflow(falling edge of f1)
	if (m_div == 0)
	{
		m_1s = true;
		wake_me_up();
	}

	// schedule next timeout
	m_div_timer->adjust(attotime::from_ticks(0x800, unscaled_clock()));
}

void sm510_base_device::reset_divider()
{
	m_div = 0;
	m_div_timer->adjust(attotime::from_ticks(0x800, unscaled_clock()));
}



//-------------------------------------------------
//  execute
//-------------------------------------------------

void sm510_base_device::increment_pc()
{
	// PL(program counter low 6 bits) is a simple LFSR: newbit = (bit0==bit1)
	// PU,PM(high bits) specify page, PL specifies steps within page
	int feed = ((m_pc >> 1 ^ m_pc) & 1) ? 0 : 0x20;
	m_pc = feed | (m_pc >> 1 & 0x1f) | (m_pc & ~0x3f);
}

void sm510_base_device::execute_run()
{
	while (m_icount > 0)
	{
		if (m_halt)
		{
			// wake up from K input (note: 1S signal is handled above)
			if (m_k_active)
				wake_me_up();
			else
			{
				// got nothing to do
				m_icount = 0;
				return;
			}
		}

		// remember previous state
		m_prev_op = m_op;
		m_prev_pc = m_pc;

		// fetch next opcode
		debugger_instruction_hook(this, m_pc);
		m_icount--;
		m_op = m_program->read_byte(m_pc);
		increment_pc();
		get_opcode_param();

		// handle opcode if it's not skipped
		if (m_skip)
		{
			m_skip = false;
			m_op = 0; // fake nop
		}
		else
			execute_one();
	}
}
