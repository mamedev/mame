// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    am2910.cpp
    AMD Am2910 Microprogram Controller emulation

    TODO:
    - Check /RLD behavior
    - Find and fix bugs that almost surely exist

***************************************************************************/

#include "emu.h"
#include "am2910.h"

#define LOG_INSN    (1 << 0)
#define LOG_STACK   (1 << 1)
#define LOG_ERROR   (1 << 2)
#define LOG_ALL     (LOG_INSN | LOG_STACK | LOG_ERROR)

#define VERBOSE     (0)
#include "logmacro.h"

/*****************************************************************************/

DEFINE_DEVICE_TYPE(AM2910, am2910_device, "am2910", "AMD Am2910 Microprogram Controller")

//-------------------------------------------------
//  am2910_device - constructor
//-------------------------------------------------

am2910_device::am2910_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, AM2910, tag, owner, clock)
	, m_pc(0)
	, m_r(0)
	, m_sp(0)
	, m_cc(0)
	, m_ccen(0)
	, m_ci(0)
	, m_rld(0)
	, m_cp(0)
	, m_y(*this)
	, m_full(*this)
	, m_pl(*this)
	, m_map(*this)
	, m_vect(*this)
{
}


void am2910_device::device_start()
{
	save_item(NAME(m_pc));
	save_item(NAME(m_r));
	save_item(NAME(m_sp));
	save_item(NAME(m_stack));
	save_item(NAME(m_cc));
	save_item(NAME(m_ccen));
	save_item(NAME(m_ci));
	save_item(NAME(m_rld));
	save_item(NAME(m_cp));
	save_item(NAME(m_d));
	save_item(NAME(m_i));

	m_y.resolve_safe();
	m_full.resolve_safe();
	m_pl.resolve_safe();
	m_map.resolve_safe();
	m_vect.resolve_safe();

	if (clock())
		m_execute_timer = timer_alloc(TIMER_CLOCK);
	else
		m_execute_timer = nullptr;
}

void am2910_device::device_reset()
{
	m_pc = 0;
	m_r = 0;
	m_sp = 0;
	memset(m_stack, 0, sizeof(uint16_t) * 5);

	if (m_execute_timer)
		m_execute_timer->adjust(attotime::from_hz(clock()), 0, attotime::from_hz(clock()));
}

void am2910_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_CLOCK)
	{
		cp_w(m_cp ? 0 : 1);
	}
}

WRITE_LINE_MEMBER(am2910_device::cc_w)
{
	m_cc = state;
}

WRITE_LINE_MEMBER(am2910_device::ccen_w)
{
	m_ccen = state;
}

WRITE_LINE_MEMBER(am2910_device::ci_w)
{
	m_ci = state;
}

WRITE_LINE_MEMBER(am2910_device::rld_w)
{
	m_rld = state;
}

WRITE_LINE_MEMBER(am2910_device::cp_w)
{
	int old_state = m_cp;
	m_cp = state;
	if (!old_state && state)
		execute();
}

void am2910_device::d_w(uint16_t data)
{
	m_d = data;
}

void am2910_device::i_w(uint8_t data)
{
	m_i = data;
}

void am2910_device::update_source()
{
	switch (m_i)
	{
	case 2:
		m_pl(0);
		m_map(1);
		m_vect(0);
		break;
	case 6:
		m_pl(0);
		m_map(0);
		m_vect(1);
		break;
	default:
		m_pl(1);
		m_map(0);
		m_vect(0);
		break;
	}
}

void am2910_device::execute()
{
	if (m_rld == 0)
		m_r = m_d;

	m_pc += (uint16_t)m_ci;
	switch (m_i)
	{
	case 0:     // JZ, Jump Zero
		LOGMASKED(LOG_INSN, "%04x: JZ\n", m_pc);
		m_pc = 0;
		m_sp = 0;
		break;
	case 1:     // CJS, Conditional Jump-To-Subroutine PL
		LOGMASKED(LOG_INSN, "%04x: CJS\n", m_pc);
		if (test_pass())
		{
			push(m_pc);
			m_pc = m_d;
		}
		break;
	case 2:     // JMAP, Jump Map
		LOGMASKED(LOG_INSN, "%04x: JMAP\n", m_pc);
		m_pc = m_d;
		break;
	case 3:     // CJP, Conditional Jump PL
		LOGMASKED(LOG_INSN, "%04x: CJP\n", m_pc);
		if (test_pass())
			m_pc = m_d;
		break;
	case 4:     // PUSH, Push / Conditional Load Counter
		LOGMASKED(LOG_INSN, "%04x: PUSH\n", m_pc);
		push(m_pc);
		if (test_pass())
			m_r = m_d;
		break;
	case 5:     // JSRP, Conditional JSB R/PL
		LOGMASKED(LOG_INSN, "%04x: JSRP\n", m_pc);
		push(m_pc);
		if (test_pass())
			m_pc = m_d;
		else
			m_pc = m_r;
		break;
	case 6:     // CJV, Conditional Jump Vector
		LOGMASKED(LOG_INSN, "%04x: CJV\n", m_pc);
		if (test_pass())
		{
			m_pc = m_d;
		}
		break;
	case 7:     // JRP, Conditional Jump R/PL
		LOGMASKED(LOG_INSN, "%04x: JRP\n", m_pc);
		if (test_pass())
			m_pc = m_d;
		else
			m_pc = m_r;
		break;
	case 8:     // RFCT, Repeat Loop, Counter != 0
		LOGMASKED(LOG_INSN, "%04x: RFCT\n", m_pc);
		if (m_r != 0)
		{
			m_r--;
			m_pc = m_stack[m_sp];
		}
		break;
	case 9:     // RPCT, Repeat PL, Counter != 0
		LOGMASKED(LOG_INSN, "%04x: RPCT\n", m_pc);
		if (m_r != 0)
		{
			m_r--;
			m_pc = m_d;
		}
		break;
	case 10:    // CRTN, Conditional Return
		LOGMASKED(LOG_INSN, "%04x: CRTN\n", m_pc);
		if (test_pass())
			m_pc = m_stack[m_sp];
		break;
	case 11:    // CJPP, Conditional Jump PL & Pop
		LOGMASKED(LOG_INSN, "%04x: CJPP\n", m_pc);
		if (test_pass())
		{
			m_pc = m_d;
			pop();
		}
		break;
	case 12:    // LDCT, Load Counter & Continue
		LOGMASKED(LOG_INSN, "%04x: LDCT\n", m_pc);
		m_r = m_d;
		break;
	case 13:    // LOOP, Test End Loop
		LOGMASKED(LOG_INSN, "%04x: LOOP\n", m_pc);
		if (!test_pass())
			m_pc = m_stack[m_sp];
		break;
	case 14:    // CONT, Continue
		LOGMASKED(LOG_INSN, "%04x: CONT\n", m_pc);
		break;
	case 15:    // TWB, Three-Way Branch
		LOGMASKED(LOG_INSN, "%04x: TWB\n", m_pc);
		if (!test_pass())
		{
			if (m_r != 0)
			{
				m_r--;
				m_pc = m_stack[m_sp];
			}
			else
			{
				m_pc = m_d;
			}
		}
		break;
	default:
		LOGMASKED(LOG_ERROR, "Unrecognized Instruction: %08x\n", m_i);
	}

	m_y(m_pc);
}

bool am2910_device::test_pass()
{
	return m_ccen != 0 || m_cc == 0;
}

void am2910_device::push(uint16_t value)
{
	if (m_sp < 4)
	{
		m_sp++;
		if (m_sp == 4)
		{
			m_full(1);
		}
	}
	m_stack[m_sp] = value;
}

void am2910_device::pop()
{
	if (m_sp > 0)
	{
		m_sp--;
		if (m_sp == 3)
		{
			m_full(0);
		}
	}
}
