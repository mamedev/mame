/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "includes/matmania.h"


/***************************************************************************

 Mania Challenge 68705 protection interface

 The following is ENTIRELY GUESSWORK!!!

***************************************************************************/

READ8_MEMBER(matmania_state::maniach_68705_port_a_r )
{
	//logerror("%04x: 68705 port A read %02x\n", space.device().safe_pc(), m_port_a_in);
	return (m_port_a_out & m_ddr_a) | (m_port_a_in & ~m_ddr_a);
}

WRITE8_MEMBER(matmania_state::maniach_68705_port_a_w )
{
	//logerror("%04x: 68705 port A write %02x\n", space.device().safe_pc(), data);
	m_port_a_out = data;
}

WRITE8_MEMBER(matmania_state::maniach_68705_ddr_a_w )
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

READ8_MEMBER(matmania_state::maniach_68705_port_b_r )
{
	return (m_port_b_out & m_ddr_b) | (m_port_b_in & ~m_ddr_b);
}

WRITE8_MEMBER(matmania_state::maniach_68705_port_b_w )
{
	//logerror("%04x: 68705 port B write %02x\n", space.device().safe_pc(), data);

	if (BIT(m_ddr_b, 1) && BIT(~data, 1) && BIT(m_port_b_out, 1))
	{
		m_port_a_in = m_from_main;
		m_main_sent = 0;
		//logerror("read command %02x from main cpu\n", m_port_a_in);
	}
	if (BIT(m_ddr_b, 2) && BIT(data, 2) && BIT(~m_port_b_out, 2))
	{
		//logerror("send command %02x to main cpu\n", m_port_a_out);
		m_from_mcu = m_port_a_out;
		m_mcu_sent = 1;
	}

	m_port_b_out = data;
}

WRITE8_MEMBER(matmania_state::maniach_68705_ddr_b_w )
{
	m_ddr_b = data;
}


READ8_MEMBER(matmania_state::maniach_68705_port_c_r )
{
	m_port_c_in = 0;

	if (m_main_sent)
		m_port_c_in |= 0x01;

	if (!m_mcu_sent)
		m_port_c_in |= 0x02;

	//logerror("%04x: 68705 port C read %02x\n",m_space->device().safe_pc(), m_port_c_in);

	return (m_port_c_out & m_ddr_c) | (m_port_c_in & ~m_ddr_c);
}

WRITE8_MEMBER(matmania_state::maniach_68705_port_c_w )
{
	//logerror("%04x: 68705 port C write %02x\n", space.device().safe_pc(), data);
	m_port_c_out = data;
}

WRITE8_MEMBER(matmania_state::maniach_68705_ddr_c_w )
{
	m_ddr_c = data;
}


WRITE8_MEMBER(matmania_state::maniach_mcu_w )
{
	//logerror("%04x: 3040_w %02x\n", space.device().safe_pc(), data);
	m_from_main = data;
	m_main_sent = 1;
}

READ8_MEMBER(matmania_state::maniach_mcu_r )
{
	//logerror("%04x: 3040_r %02x\n", space.device().safe_pc(), m_from_mcu);
	m_mcu_sent = 0;
	return m_from_mcu;
}

READ8_MEMBER(matmania_state::maniach_mcu_status_r )
{
	int res = 0;

	/* bit 0 = when 0, mcu has sent data to the main cpu */
	/* bit 1 = when 1, mcu is ready to receive data from main cpu */
	//logerror("%04x: 3041_r\n", space.device().safe_pc());
	if (!m_mcu_sent)
		res |= 0x01;
	if (!m_main_sent)
		res |= 0x02;

	return res;
}
