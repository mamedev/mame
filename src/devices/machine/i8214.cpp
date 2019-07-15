// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Intel 8214/3214 Priority Interrupt Control Unit

**********************************************************************/

#include "emu.h"
#include "i8214.h"

//#define VERBOSE 1
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
	m_write_int(CLEAR_LINE);
}


//-------------------------------------------------
//  check_interrupt -
//-------------------------------------------------

void i8214_device::check_interrupt()
{
	if (m_int_dis || !m_etlg || !m_inte) return;

	for (int level = 7; level >= 0; level--)
	{
		if (!BIT(m_r, 7 - level))
		{
			if (m_sgs)
			{
				if (level > m_current_status)
				{
					trigger_interrupt(level);
				}
			}
			else
			{
				trigger_interrupt(level);
			}
		}
	}
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

	LOG("I8214 A: %01x\n", a);

	return a;
}


//-------------------------------------------------
//  vector_r - read A outputs to be latched as an
//  8080-compatible interrupt vector
//-------------------------------------------------

READ8_MEMBER(i8214_device::vector_r)
{
	return 0xc7 | (m_a << 3);
}


//-------------------------------------------------
//  b_w -
//-------------------------------------------------

void i8214_device::b_w(uint8_t data)
{
	m_current_status = data & 0x07;

	LOG("I8214 B: %01x\n", m_current_status);

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
	LOG("I8214 R%d: %d\n", line, state);

	m_r &= ~(1 << line);
	m_r |= (state << line);

	check_interrupt();
}


//-------------------------------------------------
//  sgs_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( i8214_device::sgs_w )
{
	LOG("I8214 SGS: %u\n", state);

	m_sgs = state;

	check_interrupt();
}


//-------------------------------------------------
//  etlg_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( i8214_device::etlg_w )
{
	LOG("I8214 ETLG: %u\n", state);

	m_etlg = state;

	check_interrupt();
}


//-------------------------------------------------
//  inte_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( i8214_device::inte_w )
{
	LOG("I8214 INTE: %u\n", state);

	m_inte = state;

	check_interrupt();
}
