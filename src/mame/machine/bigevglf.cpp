// license:GPL-2.0+
// copyright-holders:Jarek Burczynski, Tomasz Slanina
/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "includes/bigevglf.h"


uint8_t bigevglf_state::bigevglf_68705_port_a_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return (m_port_a_out & m_ddr_a) | (m_port_a_in & ~m_ddr_a);
}

void bigevglf_state::bigevglf_68705_port_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_port_a_out = data;
}

void bigevglf_state::bigevglf_68705_ddr_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_ddr_a = data;
}

uint8_t bigevglf_state::bigevglf_68705_port_b_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return (m_port_b_out & m_ddr_b) | (m_port_b_in & ~m_ddr_b);
}

void bigevglf_state::bigevglf_68705_port_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
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

void bigevglf_state::bigevglf_68705_ddr_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_ddr_b = data;
}

uint8_t bigevglf_state::bigevglf_68705_port_c_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	m_port_c_in = 0;
	if (m_main_sent)
		m_port_c_in |= 0x01;
	if (m_mcu_sent)
		m_port_c_in |= 0x02;

	return (m_port_c_out & m_ddr_c) | (m_port_c_in & ~m_ddr_c);
}

void bigevglf_state::bigevglf_68705_port_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_port_c_out = data;
}

void bigevglf_state::bigevglf_68705_ddr_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_ddr_c = data;
}

void bigevglf_state::bigevglf_mcu_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_port_a_in = data;
	m_main_sent = 1;
	m_mcu->set_input_line(0, ASSERT_LINE);
}


uint8_t bigevglf_state::bigevglf_mcu_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	m_mcu_sent = 1;
	return m_from_mcu;
}

uint8_t bigevglf_state::bigevglf_mcu_status_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	int res = 0;

	if (!m_main_sent)
		res |= 0x08;
	if (!m_mcu_sent)
		res |= 0x10;

	return res;
}
