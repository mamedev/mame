// license:???
// copyright-holders:Jarek Burczynski, Tomasz Slanina
/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "includes/bigevglf.h"


READ8_MEMBER(bigevglf_state::bigevglf_68705_port_a_r)
{
	return (m_port_a_out & m_ddr_a) | (m_port_a_in & ~m_ddr_a);
}

WRITE8_MEMBER(bigevglf_state::bigevglf_68705_port_a_w)
{
	m_port_a_out = data;
}

WRITE8_MEMBER(bigevglf_state::bigevglf_68705_ddr_a_w)
{
	m_ddr_a = data;
}

READ8_MEMBER(bigevglf_state::bigevglf_68705_port_b_r)
{
	return (m_port_b_out & m_ddr_b) | (m_port_b_in & ~m_ddr_b);
}

WRITE8_MEMBER(bigevglf_state::bigevglf_68705_port_b_w)
{
	if ((m_ddr_b & 0x02) && (~m_port_b_out & 0x02) && (data & 0x02)) /* positive going transition of the clock */
	{
		m_mcu->set_input_line(0, CLEAR_LINE);
		m_main_sent = 0;

	}
	if ((m_ddr_b & 0x04) && (~m_port_b_out & 0x04) && (data & 0x04) ) /* positive going transition of the clock */
	{
		m_from_mcu = m_port_a_out;
		m_mcu_sent = 0;
	}

	m_port_b_out = data;
}

WRITE8_MEMBER(bigevglf_state::bigevglf_68705_ddr_b_w)
{
	m_ddr_b = data;
}

READ8_MEMBER(bigevglf_state::bigevglf_68705_port_c_r)
{
	m_port_c_in = 0;
	if (m_main_sent)
		m_port_c_in |= 0x01;
	if (m_mcu_sent)
		m_port_c_in |= 0x02;

	return (m_port_c_out & m_ddr_c) | (m_port_c_in & ~m_ddr_c);
}

WRITE8_MEMBER(bigevglf_state::bigevglf_68705_port_c_w)
{
	m_port_c_out = data;
}

WRITE8_MEMBER(bigevglf_state::bigevglf_68705_ddr_c_w)
{
	m_ddr_c = data;
}

WRITE8_MEMBER(bigevglf_state::bigevglf_mcu_w)
{
	m_port_a_in = data;
	m_main_sent = 1;
	m_mcu->set_input_line(0, ASSERT_LINE);
}


READ8_MEMBER(bigevglf_state::bigevglf_mcu_r)
{
	m_mcu_sent = 1;
	return m_from_mcu;
}

READ8_MEMBER(bigevglf_state::bigevglf_mcu_status_r)
{
	int res = 0;

	if (!m_main_sent)
		res |= 0x08;
	if (!m_mcu_sent)
		res |= 0x10;

	return res;
}
