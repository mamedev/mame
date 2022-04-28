// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Intel 8214/3214 Priority Interrupt Control Unit

**********************************************************************/

#include "emu.h"
#include "i8214.h"
#include <iostream>

//#define VERBOSE 1
//#define LOG_OUTPUT_STREAM std::cout
#include "logmacro.h"


// device type definition
DEFINE_DEVICE_TYPE(I8214, i8214_device, "i8214", "Intel 8214 PICU")



//**************************************************************************
//  HELPERS
//**************************************************************************

//-------------------------------------------------
//  trigger_interrupt -
//-------------------------------------------------

void i8214_device::trigger_interrupt(int level)
{
	LOG("I8214 Interrupt Level %u\n", level);

	m_a = level;

	// disable more interrupts from being latched
	m_int_dis = 1;

	// disable next level group
	m_write_enlg(0);

	// set interrupt line
	m_write_int(ASSERT_LINE);

	//m_write_int(CLEAR_LINE);
}


//-------------------------------------------------
//  check_interrupt -
//-------------------------------------------------

void i8214_device::check_interrupt()
{
	if (m_int_dis)
	{
		LOG("not checking interrupts because m_int_dis (%02x)\n", m_r);
		if (m_r == 0xff)
		{
			m_int_dis = 0;
			m_write_int(CLEAR_LINE);
		}
		return;
	}
	if (!m_etlg)
	{
		LOG("not checking interrupts because !m_etlg\n");
		return;
	}
	if (!m_inte)
	{
		LOG("not checking interrupts because !m_inte\n");
		return;
	}

	LOG("checking interrupts: R:%02x, sgs:%d, current:%d\n", m_r, m_sgs, m_current_status);

	for (int level = 7; level >= 0; level--)
	{
		if (!BIT(m_r, level))
		{
			if (m_sgs)
			{
				if (level > m_current_status)
				{
					LOG("bit %d unset, level %d > current %d, triggering interrupt\n", 7 - level, level, m_current_status);
					trigger_interrupt(level);
					return;
				}
			}
			else
			{
				LOG("bit %d unset, level %d, triggering interrupt\n", 7 - level, level);
				trigger_interrupt(level);
				return;
			}
		}
	}

	m_int_dis = 0;
	m_write_int(CLEAR_LINE);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  i8214_device - constructor
//-------------------------------------------------

i8214_device::i8214_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, I8214, tag, owner, clock)
	, m_write_int(*this)
	, m_write_enlg(*this)
	, m_inte(0)
	, m_int_dis(0)
	, m_a(0)
	, m_current_status(0)
	, m_r(0xff)
	, m_sgs(0)
	, m_etlg(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8214_device::device_start()
{
	// resolve callbacks
	m_write_int.resolve_safe();
	m_write_enlg.resolve_safe();

	m_int_dis = 0;
	m_etlg = 1;

	// register for state saving
	save_item(NAME(m_inte));
	save_item(NAME(m_int_dis));
	save_item(NAME(m_a));
	save_item(NAME(m_current_status));
	save_item(NAME(m_r));
	save_item(NAME(m_sgs));
	save_item(NAME(m_etlg));
}


//-------------------------------------------------
//  a_r -
//-------------------------------------------------

uint8_t i8214_device::a_r()
{
	uint8_t a = m_a & 0x07;

	LOG("%s: a_r: %01x\n", machine().describe_context(), a);

	return a;
}


//-------------------------------------------------
//  vector_r - read A outputs to be latched as an
//  8080-compatible interrupt vector
//-------------------------------------------------

uint8_t i8214_device::vector_r()
{
	return 0xc7 | (m_a << 3);
}


//-------------------------------------------------
//  b_sgs_w -
//-------------------------------------------------

void i8214_device::b_sgs_w(uint8_t data)
{
	m_current_status = data & 0x07;
	m_sgs = BIT(data, 3);

	LOG("%s: b_sgs_w: b:%d, sgs:%d\n", machine().describe_context(), m_current_status, m_sgs);

	// enable interrupts
	m_int_dis = 0;

	// enable next level group
	m_write_enlg(1);

	check_interrupt();
}

//-------------------------------------------------
//  b_w -
//-------------------------------------------------

void i8214_device::b_w(uint8_t data)
{
	m_current_status = data & 0x07;

	LOG("%s: b_w: %d\n", machine().describe_context(), m_current_status);

	// enable interrupts
	m_int_dis = 0;

	// enable next level group
	m_write_enlg(1);

	check_interrupt();
}


//-------------------------------------------------
//  r_w - update the interrupt request
//  state for a given line
//-------------------------------------------------

void i8214_device::r_w(int line, int state)
{
	LOG("%s: r_w: line %d, state %d\n", machine().describe_context(), line, state);

	m_r &= ~(1 << line);
	m_r |= (state << line);

	check_interrupt();
}


void i8214_device::r_all_w(uint8_t state)
{
	LOG("%s: r_all_w: state %02x\n", machine().describe_context(), state);

	m_r = state;

	check_interrupt();
}

//-------------------------------------------------
//  sgs_w -
//-------------------------------------------------

void i8214_device::sgs_w(int state)
{
	LOG("%s: sgs_w: %d\n", machine().describe_context(), state);

	m_sgs = state;

	check_interrupt();
}


//-------------------------------------------------
//  etlg_w -
//-------------------------------------------------

void i8214_device::etlg_w(int state)
{
	LOG("%s: etlg_w: %d\n", machine().describe_context(), state);

	m_etlg = state;

	check_interrupt();
}


//-------------------------------------------------
//  inte_w -
//-------------------------------------------------

void i8214_device::inte_w(int state)
{
	LOG("%s: inte_w: %d\n", machine().describe_context(), state);

	m_inte = state;

	check_interrupt();
}
