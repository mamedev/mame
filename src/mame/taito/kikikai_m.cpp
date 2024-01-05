// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#include "emu.h"
#include "kikikai.h"

/*
$f008 - write
bit 7 = ? (unused?)
bit 6 = ? (unused?)
bit 5 = ? (unused?)
bit 4 = ? (usually set in game)
bit 3 = ? (usually set in game)
bit 2 = sound cpu reset line
bit 1 = microcontroller reset line
bit 0 = ? (unused?)
*/
void kikikai_state::main_f008_w(uint8_t data)
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, (data & 4) ? CLEAR_LINE : ASSERT_LINE);

	m_mcu->set_input_line(INPUT_LINE_RESET, (data & 2) ? CLEAR_LINE : ASSERT_LINE);
}

void mexico86_state::main_f008_w(uint8_t data)
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, (data & 4) ? CLEAR_LINE : ASSERT_LINE);

	// mexico 86, knight boy
	m_68705mcu->set_input_line(INPUT_LINE_RESET, (data & 2) ? CLEAR_LINE : ASSERT_LINE);
}

INTERRUPT_GEN_MEMBER(kikikai_state::kikikai_interrupt)
{
	device.execute().set_input_line(0, ASSERT_LINE);
}

IRQ_CALLBACK_MEMBER(kikikai_state::mcram_vect_r)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
	return m_mcu_sharedram[0];
}

/***************************************************************************

 Mexico 86 68705 protection interface

 The following is ENTIRELY GUESSWORK!!!

***************************************************************************/

INTERRUPT_GEN_MEMBER(mexico86_state::mexico86_m68705_interrupt)
{
	device.execute().set_input_line(M68705_IRQ_LINE, ASSERT_LINE);
}


void mexico86_state::mexico86_68705_port_a_w(u8 data)
{
	//logerror("%s: 68705 port A write %02x\n", machine().describe_context(), data);
	m_port_a_out = data;
}



/*
 *  Port B connections:
 *
 *  all bits are logical 1 when read (+5V pullup)
 *
 *  0   W  enables latch which holds data from main Z80 memory
 *  1   W  loads the latch which holds the low 8 bits of the address of
 *               the main Z80 memory location to access
 *  2   W  0 = read input ports, 1 = access Z80 memory
 *  3   W  clocks main Z80 memory access
 *  4   W  selects Z80 memory access direction (0 = write 1 = read)
 *  5   W  clocks a flip-flop which causes IRQ on the main Z80
 *  6   W  not used?
 *  7   W  not used?
 */

void mexico86_state::mexico86_68705_port_b_w(offs_t offset, u8 data, u8 mem_mask)
{
	//logerror("%s: 68705 port B write %02x\n", machine().describe_context(), data);

	u8 const port_a_value(m_port_a_out & (BIT(m_port_b_out, 0) ? 0xff : m_latch));

	if (BIT(mem_mask, 3) && !BIT(data, 3) && BIT(m_port_b_out, 3))
	{
		if (BIT(m_port_b_out, 4)) // read
		{
			if (BIT(m_port_b_out, 2))
			{
				//logerror("%s: 68705 read %02x from address %04x\n", machine().describe_context(), m_mcu_sharedram[m_address], m_address);
				m_latch = m_mcu_sharedram[m_address];
			}
			else
			{
				//logerror("%s: 68705 read input port %04x\n", machine().describe_context(), m_address);
				m_latch = ioport(BIT(m_address, 0) ? "IN2" : "IN1")->read();
			}
		}
		else // write
		{
				//logerror("%s: 68705 write %02x to address %04x\n",machine().describe_context(), port_a_value, m_address);
				m_mcu_sharedram[m_address] = port_a_value;
		}
	}

	m_68705mcu->pa_w((BIT(mem_mask, 0) && !BIT(data, 0)) ? m_latch : 0xff);

	if (BIT(mem_mask, 1) && !BIT(data, 1) && BIT(m_port_b_out, 1))
	{
		m_address = port_a_value;
		//if (m_address >= 0x80) logerror("%s: 68705 address %02x\n", machine().describe_context(), port_a_value);
	}

	if (BIT(mem_mask, 5) && BIT(data, 5) && !BIT(m_port_b_out, 5))
	{
		m_maincpu->set_input_line(0, ASSERT_LINE);
		m_68705mcu->set_input_line(M68705_IRQ_LINE, CLEAR_LINE);
	}

	if (BIT(mem_mask, 6) && !BIT(data, 6) && BIT(m_port_b_out, 6))
		logerror("%s: 68705 unknown port B bit %02x\n", machine().describe_context(), data);

	if (BIT(mem_mask, 7) && !BIT(data, 7) && BIT(m_port_b_out, 7))
		logerror("%s: 68705 unknown port B bit %02x\n", machine().describe_context(), data);

	m_port_b_out = data;
}


/***************************************************************************

Kiki KaiKai / Kick 'n Run MCU

***************************************************************************/

void kikikai_state::kikikai_mcu_port1_w(uint8_t data)
{
	//logerror("%04x: 6801U4 port 1 write %02x\n", m_mcu->pc(), data);

	// bit 0, 1: coin counters (?)
	if (data & 0x01 && ~m_port1_out & 0x01)
	{
		machine().bookkeeping().coin_counter_w(0, data & 0x01);
	}

	if (data & 0x02 && ~m_port1_out & 0x02)
	{
		machine().bookkeeping().coin_counter_w(1, data & 0x02);
	}

	// bit 4, 5: coin lockouts
	machine().bookkeeping().coin_lockout_w(0, ~data & 0x10);
	machine().bookkeeping().coin_lockout_w(0, ~data & 0x20);

	// bit 7: ? (set briefly while MCU boots)
	m_port1_out = data;
}

void kikikai_state::kikikai_mcu_port2_w(uint8_t data)
{
	//logerror("%04x: 6801U4 port 2 write %02x\n", m_mcu->pc(), data);
	static const char *const portnames[] = { "IN1", "IN2" };

	// bit 2: clock
	// latch on high->low transition
	if ((m_port2_out & 0x04) && (~data & 0x04))
	{
		int address = m_port4_out;

		if (data & 0x10)
		{
			// read
			if (data & 0x01)
			{
				m_port3_in = m_mcu_sharedram[address];
			}
			else
			{
				m_port3_in = ioport(portnames[address & 1])->read();
			}
			m_mcu->pulse_input_line(M6801_IS3_LINE, attotime::from_usec(1));
		}
		else
		{
			// write
			m_mcu_sharedram[address] = m_port3_out;
		}
	}

	m_port2_out = data;
}

uint8_t kikikai_state::kikikai_mcu_port3_r()
{
	//logerror("%04x: 6801U4 port 3 read\n", m_mcu->pc());
	return m_port3_in;
}

void kikikai_state::kikikai_mcu_port3_w(uint8_t data)
{
	//logerror("%04x: 6801U4 port 3 write %02x\n", m_mcu->pc(), data);
	m_port3_out = data;
}

void kikikai_state::kikikai_mcu_port4_w(uint8_t data)
{
	//logerror("%04x: 6801U4 port 4 write %02x\n", m_mcu->pc(), data);
	// bits 0-7 of shared RAM address
	m_port4_out = data;
}
