// license:BSD-3-Clause
// copyright-holders:hap
/*

  Mitsubishi MELPS 4 MCU family cores

  Known types and their features:
  (* means not emulated yet)

 *M58840: 42-pin DIL, 2Kx9 ROM, 128x4 RAM, A/D converter
 *M58841: almost same as M58840
 *M58842: 64-pin DIL, external ROM(11-bit PC), rest is same as M58840
 *M58843: 28-pin DIL, 1Kx9 ROM, 64x4 RAM, A/D converter
 *M58844: almost same as M58843
 *M58845: 42-pin DIL, 2Kx9 ROM, 128x4 RAM, A/D converter, 2 timers
  M58846: 42-pin DIL, 2Kx9 ROM, 128x4 RAM, 2 timers(not same as M58845), extra I/O ports
 *M58847: 40-pin DIL, 2Kx9 ROM, 128x4 RAM, extra I/O ports(not same as M58846)
 *M58848: ? (couldn't find info, just that it exists)

  MELPS 41/42 subfamily:

 *M58494: 72-pin QFP CMOS, 4Kx10 ROM, 32x4 internal + 4Kx4 external RAM, 2 timers
 *M58496: 72-pin QFP CMOS, 2Kx10 ROM, 128x4 internal + 256x4 external RAM, 1 timer, low-power
 *M58497: almost same as M58496

  MELPS 760 family has more differences, document them when needed.
  MELPS 720 family as well


  References:
  - 1980 and 1982 Mitsubishi LSI Data Books
  - M34550Mx-XXXFP datasheet (this one is MELPS 720 family)

  TODO:
  - need more drivers that use this, to be sure that emulation is accurate
  - add output PLA

*/

#include "melps4.h"
#include "debugger.h"


// disasm
void melps4_cpu_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	switch (entry.index())
	{
		// obviously not from a single flags register, letters are made up
		case STATE_GENFLAGS:
			strprintf(str, "%c%c%c%c%c %c%c%c",
				m_intp ? 'P':'p',
				m_inte ? 'I':'i',
				m_sm   ? 'S':'s',
				m_cps  ? 'D':'d',
				m_cy   ? 'C':'c',
				m_irqflag[0] ? 'X':'.', // exf
				m_irqflag[1] ? '1':'.', // 1f
				m_irqflag[2] ? '2':'.'  // 2f
			);
			break;

		default: break;
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

enum
{
	MELPS4_PC=1, MELPS4_A, MELPS4_B, MELPS4_E,
	MELPS4_Y, MELPS4_X, MELPS4_Z,
	MELPS4_H, MELPS4_L, MELPS4_C, MELPS4_V, MELPS4_W
};

void melps4_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);
	m_prgmask = (1 << m_prgwidth) - 1;
	m_datamask = (1 << m_datawidth) - 1;
	m_d_mask = (1 << m_d_pins) - 1;

	// resolve callbacks
	m_read_k.resolve_safe(0);
	m_read_d.resolve_safe(0);
	m_read_s.resolve_safe(0);
	m_read_f.resolve_safe(0);

	m_write_d.resolve_safe();
	m_write_s.resolve_safe();
	m_write_f.resolve_safe();
	m_write_g.resolve_safe();
	m_write_u.resolve_safe();
	m_write_t.resolve_safe();

	// zerofill
	m_pc = 0;
	m_prev_pc = 0;
	memset(m_stack, 0, sizeof(m_stack));
	m_op = 0;
	m_prev_op = 0;
	m_bitmask = 0;

	m_port_d = 0;
	m_port_s = 0;
	m_port_f = 0;
	m_port_t = 0;

	m_sm = m_sms = false;
	m_ba_flag = false;
	m_sp_param = 0;
	m_cps = 0;
	m_skip = false;
	m_inte = 0;
	m_intp = 1;
	m_irqflag[0] = m_irqflag[1] = m_irqflag[2] = false;
	m_int_state = 0;
	m_t_in_state = 0;
	m_prohibit_irq = false;
	m_possible_irq = false;

	memset(m_tmr_count, 0, sizeof(m_tmr_count));
	m_tmr_reload = 0;
	m_tmr_irq_enabled[0] = m_tmr_irq_enabled[1] = false;

	m_a = 0;
	m_b = 0;
	m_e = 0;
	m_y = m_y2 = 0;
	m_x = m_x2 = 0;
	m_z = m_z2 = 0;
	m_cy = m_cy2 = 0;

	m_h = 0;
	m_l = 0;
	m_c = 7;
	m_v = 0;
	m_w = 0;

	// register for savestates
	save_item(NAME(m_pc));
	save_item(NAME(m_prev_pc));
	save_item(NAME(m_stack));
	save_item(NAME(m_op));
	save_item(NAME(m_prev_op));
	save_item(NAME(m_bitmask));

	save_item(NAME(m_port_d));
	save_item(NAME(m_port_s));
	save_item(NAME(m_port_f));
	save_item(NAME(m_port_t));

	save_item(NAME(m_sm));
	save_item(NAME(m_sms));
	save_item(NAME(m_ba_flag));
	save_item(NAME(m_sp_param));
	save_item(NAME(m_cps));
	save_item(NAME(m_skip));
	save_item(NAME(m_inte));
	save_item(NAME(m_intp));
	save_item(NAME(m_irqflag));
	save_item(NAME(m_int_state));
	save_item(NAME(m_t_in_state));
	save_item(NAME(m_prohibit_irq));
	save_item(NAME(m_possible_irq));

	save_item(NAME(m_tmr_count));
	save_item(NAME(m_tmr_reload));
	save_item(NAME(m_tmr_irq_enabled));

	save_item(NAME(m_a));
	save_item(NAME(m_b));
	save_item(NAME(m_e));
	save_item(NAME(m_y)); save_item(NAME(m_y2));
	save_item(NAME(m_x)); save_item(NAME(m_x2));
	save_item(NAME(m_z)); save_item(NAME(m_z2));
	save_item(NAME(m_cy)); save_item(NAME(m_cy2));

	save_item(NAME(m_h));
	save_item(NAME(m_l));
	save_item(NAME(m_c));
	save_item(NAME(m_v));
	save_item(NAME(m_w));

	// register state for debugger
	state_add(STATE_GENPC, "curpc", m_pc).formatstr("%04X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_cy).formatstr("%9s").noshow();

	state_add(MELPS4_PC, "PC", m_pc).formatstr("%04X");
	state_add(MELPS4_A, "A", m_a).formatstr("%2d"); // show in decimal
	state_add(MELPS4_B, "B", m_b).formatstr("%2d"); // "
	state_add(MELPS4_E, "E", m_e).formatstr("%02X");
	state_add(MELPS4_Y, "Y", m_y).formatstr("%1X");
	state_add(MELPS4_X, "X", m_x).formatstr("%1d");
	state_add(MELPS4_Z, "Z", m_z).formatstr("%1d");

	state_add(MELPS4_H, "H", m_h).formatstr("%1X");
	state_add(MELPS4_L, "L", m_l).formatstr("%1X");
	state_add(MELPS4_C, "C", m_c).formatstr("%1X");
	state_add(MELPS4_V, "V", m_v).formatstr("%1X");
	state_add(MELPS4_W, "W", m_w).formatstr("%1X");

	m_icountptr = &m_icount;
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void melps4_cpu_device::device_reset()
{
	m_sm = m_sms = false;
	m_ba_flag = false;
	m_skip = false;
	m_op = m_prev_op = 0;
	m_pc = m_prev_pc = 0;
	op_lcps(); // CPS=0

	// clear interrupts
	m_inte = 0;
	m_intp = 1;
	write_v(0);
	write_w(0);
	m_irqflag[0] = m_irqflag[1] = m_irqflag[2] = false;
	m_prohibit_irq = false;
	m_possible_irq = false;

	// clear ports
	write_d_pin(MELPS4_PORTD_CLR, 0);
	write_gen_port(MELPS4_PORTS, 0);
	write_gen_port(MELPS4_PORTF, 0);
	write_gen_port(MELPS4_PORTG, 0);
	write_gen_port(MELPS4_PORTU, 0);
	m_write_t(0); m_port_t = 0;
}



//-------------------------------------------------
//  i/o handling
//-------------------------------------------------

UINT8 melps4_cpu_device::read_gen_port(int port)
{
	// input generic port
	switch (port)
	{
		case MELPS4_PORTS:
			return m_port_s | m_read_s(port, 0xff);
		case MELPS4_PORTF:
			return m_port_f | (m_read_f(port, 0xff) & 0xf);

		default:
			break;
	}

	return 0;
}

void melps4_cpu_device::write_gen_port(int port, UINT8 data)
{
	// output generic port
	switch (port)
	{
		case MELPS4_PORTS:
			m_port_s = data;
			m_write_s(port, data, 0xff);
			break;
		case MELPS4_PORTF:
			m_port_f = data & 0xf;
			m_write_f(port, data & 0xf, 0xff);
			break;
		case MELPS4_PORTG:
			m_write_g(port, data & 0xf, 0xff);
			break;
		case MELPS4_PORTU:
			m_write_u(port, data & 1, 0xff);
			break;

		default:
			break;
	}
}

int melps4_cpu_device::read_d_pin(int bit)
{
	// read port D, return state of selected pin
	bit &= 0xf;
	UINT16 d = (m_port_d | m_read_d(bit, 0xffff)) & m_d_mask;
	return d >> bit & 1;
}

void melps4_cpu_device::write_d_pin(int bit, int state)
{
	// clear all port D pins
	if (bit == MELPS4_PORTD_CLR)
	{
		m_port_d = 0;
		m_write_d(bit, 0, 0xffff);
	}

	// set/reset one port D pin
	else
	{
		bit &= 0xf;
		m_port_d = ((m_port_d & (~(1 << bit))) | (state << bit)) & m_d_mask;
		m_write_d(bit, m_port_d, 0xffff);
	}
}



//-------------------------------------------------
//  interrupts
//-------------------------------------------------

void melps4_cpu_device::execute_set_input(int line, int state)
{
	state = (state) ? 1 : 0;

	switch (line)
	{
		// external interrupt
		case MELPS4_INPUT_LINE_INT:
			// irq on rising/falling edge
			if (state != m_int_state && state == m_intp)
			{
				m_irqflag[0] = true;
				m_possible_irq = true;
			}
			m_int_state = state;
			break;

		// timer input pin
		case MELPS4_INPUT_LINE_T:
			write_t_in(state);
			break;

		default:
			break;
	}
}

void melps4_cpu_device::do_interrupt(int which)
{
	m_inte = 0;
	m_irqflag[which] = false;

	m_icount--;
	push_pc();
	m_sms = m_sm;
	m_sm = false;
	m_op = 0; // fake nop
	m_pc = m_int_page << 7 | (which * 2);

	standard_irq_callback(which);
}

void melps4_cpu_device::check_interrupt()
{
	if (!m_inte)
		return;

	int which = 0;

	// assume that lower irq vectors have higher priority
	if (m_irqflag[0])
		which = 0;
	else if (m_irqflag[1] && m_tmr_irq_enabled[0])
		which = 1;
	else if (m_irqflag[2] && m_tmr_irq_enabled[1])
		which = 2;
	else
		return;

	do_interrupt(which);
}



//-------------------------------------------------
//  execute
//-------------------------------------------------

void melps4_cpu_device::execute_one()
{
	// B is at $18x and BM is at $10x for all MCU types
	if (m_op >= 0x180)
		op_b();
	else if (m_op >= 0x100)
		op_bm();
	else
		op_illegal();
}

void melps4_cpu_device::execute_run()
{
	while (m_icount > 0)
	{
		// remember previous state
		m_prev_op = m_op;
		m_prev_pc = m_pc;

		// Interrupts are not accepted during skips or LXY, LA, EI, DI, RT/RTS/RTI or any branch.
		// Documentation is conflicting here: older docs say that it is allowed during skips,
		// newer docs specifically say when interrupts are prohibited.
		if (m_possible_irq && !m_prohibit_irq && !m_skip)
		{
			m_possible_irq = false;
			check_interrupt();
		}
		m_prohibit_irq = false;

		// fetch next opcode
		debugger_instruction_hook(this, m_pc);
		m_icount--;
		m_op = m_program->read_word(m_pc << 1) & 0x1ff;
		m_bitmask = 1 << (m_op & 3);
		m_pc = (m_pc & ~0x7f) | ((m_pc + 1) & 0x7f); // stays in the same page

		// handle opcode if it's not skipped
		if (m_skip)
		{
			// if it's a long jump, skip next one as well
			if (m_op != m_ba_op && (m_op & ~0xf) != m_sp_mask)
			{
				m_skip = false;
				m_op = 0; // fake nop
			}
		}
		else
			execute_one();
	}
}
