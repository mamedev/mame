// license:BSD-3-Clause
// copyright-holders:Jarek Parchanski, Andrea Mazzoleni
#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/retofinv.h"


/***************************************************************************

 Return of Invaders 68705 protection interface

***************************************************************************/


uint8_t retofinv_state::mcu_portA_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
//logerror("%04x: 68705 port A read %02x\n",space.device().safe_pc(),m_portA_in);
	return (m_portA_out & m_ddrA) | (m_portA_in & ~m_ddrA);
}

void retofinv_state::mcu_portA_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
//logerror("%04x: 68705 port A write %02x\n",space.device().safe_pc(),data);
	m_portA_out = data;
}

void retofinv_state::mcu_ddrA_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_ddrA = data;
}



/*
 *  Port B connections:
 *
 *  all bits are logical 1 when read (+5V pullup)
 *
 *  1   W  IRQ ack and enable latch which holds data from main Z80 memory
 *  2   W  loads latch to Z80
 */


uint8_t retofinv_state::mcu_portB_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return (m_portB_out & m_ddrB) | (m_portB_in & ~m_ddrB);
}

void retofinv_state::mcu_portB_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
//logerror("%04x: 68705 port B write %02x\n",space.device().safe_pc(),data);

	if ((m_ddrB & 0x02) && (~data & 0x02) && (m_portB_out & 0x02))
	{
		m_portA_in = m_from_main;
		if (m_main_sent) m_68705->set_input_line(0, CLEAR_LINE);
		m_main_sent = 0;
//logerror("read command %02x from main cpu\n",m_portA_in);
	}
	if ((m_ddrB & 0x04) && (data & 0x04) && (~m_portB_out & 0x04))
	{
//logerror("send command %02x to main cpu\n",m_portA_out);
		m_from_mcu = m_portA_out;
		m_mcu_sent = 1;
	}

	m_portB_out = data;
}

void retofinv_state::mcu_ddrB_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_ddrB = data;
}


/*
 *  Port C connections:
 *
 *  all bits are logical 1 when read (+5V pullup)
 *
 *  0   R  1 when pending command Z80->68705
 *  1   R  0 when pending command 68705->Z80
 */


uint8_t retofinv_state::mcu_portC_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	m_portC_in = 0;
	if (m_main_sent) m_portC_in |= 0x01;
	if (!m_mcu_sent) m_portC_in |= 0x02;
//logerror("%04x: 68705 port C read %02x\n",space.device().safe_pc(),m_portC_in);
	return (m_portC_out & m_ddrC) | (m_portC_in & ~m_ddrC);
}

void retofinv_state::mcu_portC_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
logerror("%04x: 68705 port C write %02x\n",space.device().safe_pc(),data);
	m_portC_out = data;
}

void retofinv_state::mcu_ddrC_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_ddrC = data;
}


void retofinv_state::mcu_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
logerror("%04x: mcu_w %02x\n",space.device().safe_pc(),data);
	m_from_main = data;
	m_main_sent = 1;
	m_68705->set_input_line(0, ASSERT_LINE);
}

uint8_t retofinv_state::mcu_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
logerror("%04x: mcu_r %02x\n",space.device().safe_pc(),m_from_mcu);
	m_mcu_sent = 0;
	return m_from_mcu;
}

uint8_t retofinv_state::mcu_status_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	int res = 0;

	/* bit 4 = when 1, mcu is ready to receive data from main cpu */
	/* bit 5 = when 1, mcu has sent data to the main cpu */
//logerror("%04x: mcu_status_r\n",space.device().safe_pc());
	if (!m_main_sent) res |= 0x10;
	if (m_mcu_sent) res |= 0x20;

	return res;
}
