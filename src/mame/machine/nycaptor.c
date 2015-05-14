// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina
/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "includes/nycaptor.h"

READ8_MEMBER(nycaptor_state::nycaptor_68705_port_a_r)
{
	return (m_port_a_out & m_ddr_a) | (m_port_a_in & ~m_ddr_a);
}

WRITE8_MEMBER(nycaptor_state::nycaptor_68705_port_a_w)
{
	m_port_a_out = data;
}

WRITE8_MEMBER(nycaptor_state::nycaptor_68705_ddr_a_w)
{
	m_ddr_a = data;
}

/*
 *  Port B connections:
 *
 *  all bits are logical 1 when read (+5V pullup)
 *
 *  1   W  when 1->0, enables latch which brings the command from main CPU (read from port A)
 *  2   W  when 0->1, copies port A to the latch for the main CPU
 */

READ8_MEMBER(nycaptor_state::nycaptor_68705_port_b_r)
{
	return (m_port_b_out & m_ddr_b) | (m_port_b_in & ~m_ddr_b);
}

WRITE8_MEMBER(nycaptor_state::nycaptor_68705_port_b_w)
{
	if (BIT(m_ddr_b, 1) && BIT(~data, 1) && BIT(m_port_b_out, 1))
	{
		m_port_a_in = m_from_main;

		if (m_main_sent)
			m_mcu->set_input_line(0, CLEAR_LINE);
		m_main_sent = 0;

	}

	if (BIT(m_ddr_b, 2) && BIT(data, 2) && BIT(~m_port_b_out, 2))
	{
		m_from_mcu = m_port_a_out;
		m_mcu_sent = 1;
	}

	m_port_b_out = data;
}

WRITE8_MEMBER(nycaptor_state::nycaptor_68705_ddr_b_w)
{
	m_ddr_b = data;
}


READ8_MEMBER(nycaptor_state::nycaptor_68705_port_c_r)
{
	m_port_c_in = 0;

	if (m_main_sent)
		m_port_c_in |= 0x01;
	if (!m_mcu_sent)
		m_port_c_in |= 0x02;

	return (m_port_c_out & m_ddr_c) | (m_port_c_in & ~m_ddr_c);
}

WRITE8_MEMBER(nycaptor_state::nycaptor_68705_port_c_w)
{
	m_port_c_out = data;
}

WRITE8_MEMBER(nycaptor_state::nycaptor_68705_ddr_c_w)
{
	m_ddr_c = data;
}

WRITE8_MEMBER(nycaptor_state::nycaptor_mcu_w)
{
	m_from_main = data;
	m_main_sent = 1;
	m_mcu->set_input_line(0, ASSERT_LINE);
}

READ8_MEMBER(nycaptor_state::nycaptor_mcu_r)
{
	m_mcu_sent = 0;
	return m_from_mcu;
}

READ8_MEMBER(nycaptor_state::nycaptor_mcu_status_r1)
{
	/* bit 1 = when 1, mcu has sent data to the main cpu */
	return m_mcu_sent ? 2 : 0;
}

READ8_MEMBER(nycaptor_state::nycaptor_mcu_status_r2)
{
	/* bit 0 = when 1, mcu is ready to receive data from main cpu */
	return m_main_sent ? 0 : 1;
}
