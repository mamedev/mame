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

void kikikai_simulation_state::main_f008_w(uint8_t data)
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, (data & 4) ? CLEAR_LINE : ASSERT_LINE);

	// simulation for KiKi KaiKai
	m_kikikai_simulated_mcu_running = data & 2;

	if (!m_kikikai_simulated_mcu_running)
		m_kikikai_simulated_mcu_initialised = 0;
}

/***************************************************************************

 KiKi KaiKai MCU simulation

 This is derived from examination of the bootleg 68705 MCU code, with an
 addition to fix collision detection which is missing from the bootleg.

***************************************************************************/

void kikikai_simulation_state::mcu_simulate(  )
{
	if (!m_kikikai_simulated_mcu_initialised)
	{
		if (m_mcu_sharedram[0x01] == 0x00)
		{
			logerror("initialising MCU\n");
			m_mcu_sharedram[0x04] = 0xfc;   // coin inputs
			m_mcu_sharedram[0x02] = 0xff;   // player 1
			m_mcu_sharedram[0x03] = 0xff;   // player 2
			m_mcu_sharedram[0x1b] = 0xff;   // active player
			m_mcu_sharedram[0x06] = 0xff;   // must be FF otherwise PS4 ERROR
			m_mcu_sharedram[0x07] = 0x03;   // must be 03 otherwise PS4 ERROR
			m_mcu_sharedram[0x00] = 0x00;
			m_kikikai_simulated_mcu_initialised = 1;
		}
	}

	if (m_kikikai_simulated_mcu_initialised)
	{
		int i;
		bool coin_curr;
		u8 coin_in_read = ioport("IN0")->read() & 3;

		// TODO: still needs Coinage B into account
		for(int coin_idx = 0; coin_idx < 2; coin_idx++)
		{
			coin_curr = (coin_in_read & (1 << coin_idx)) == 0;
			if (coin_curr && m_coin_last[coin_idx] == false)
			{
				u8 coinage_setting = (ioport("DSW0")->read() >> (coin_idx*2 + 4)) & 3;

				// increase credits counter
				switch(coinage_setting)
				{
					case 0: // 2c / 3c
					case 1: // 2c / 1c
						if(m_coin_fract == 1)
						{
							m_mcu_sharedram[0x01]+= (coinage_setting == 0) ? 3 : 1;
							m_coin_fract = 0;
						}
						else
							m_coin_fract ++;

						break;
					case 2: // 1c / 2c
					case 3: // 1c / 1c
						m_mcu_sharedram[0x01]+= (coinage_setting == 2) ? 2 : 1;
						break;

				}

				m_mcu_sharedram[0x0a] = 0x01;   // set flag (coin inserted sound is not played otherwise)
			}
			m_coin_last[coin_idx] = coin_curr;
		}
		// Purge any coin counter higher than 9 TODO: is this limit correct?
		if(m_mcu_sharedram[0x01] > 9)
			m_mcu_sharedram[0x01] = 9;

		m_mcu_sharedram[0x04] = 0x3c | (coin_in_read ^ 3);   // coin inputs

		m_mcu_sharedram[0x02] = bitswap<8>(ioport("IN1")->read(), 7,6,5,4,2,3,1,0); // player 1
		m_mcu_sharedram[0x03] = bitswap<8>(ioport("IN2")->read(), 7,6,5,4,2,3,1,0); // player 2

		if (m_mcu_sharedram[0x19] == 0xaa)  // player 2 active
			m_mcu_sharedram[0x1b] = m_mcu_sharedram[0x03];
		else
			m_mcu_sharedram[0x1b] = m_mcu_sharedram[0x02];


		for (i = 0; i < 0x10; i += 2)
			m_mcu_sharedram[i + 0xb1] = m_mcu_sharedram[i + 0xb0];

		for (i = 0; i < 0x0a; i++)
			m_mcu_sharedram[i + 0xc0] = m_mcu_sharedram[i + 0x90] + 1;

		if (m_mcu_sharedram[0xd1] == 0xff)
		{
			if (m_mcu_sharedram[0xd0] > 0 && m_mcu_sharedram[0xd0] < 4)
			{
				m_mcu_sharedram[0xd2] = 0x81;
				m_mcu_sharedram[0xd0] = 0xff;
			}
		}

		if (m_mcu_sharedram[0xe0] > 0 && m_mcu_sharedram[0xe0] < 4)
		{
			static const u8 answers[3][16] =
			{
				{ 0x00,0x40,0x48,0x50,0x58,0x60,0x68,0x70,0x78,0x80,0x88,0x00,0x00,0x00,0x00,0x00 },
				{ 0x00,0x04,0x08,0x0C,0x10,0x14,0x18,0x1C,0x20,0x31,0x2B,0x35,0x00,0x00,0x00,0x00 },
				{ 0x00,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x03,0x0A,0x0B,0x14,0x00,0x00,0x00,0x00 },
			};
			int table = m_mcu_sharedram[0xe0] - 1;

			for (i = 1; i < 0x10; i++)
				m_mcu_sharedram[0xe0 + i] = answers[table][i];
			m_mcu_sharedram[0xe0] = 0xff;
		}

		if (m_mcu_sharedram[0xf0] > 0 && m_mcu_sharedram[0xf0] < 4)
		{
			m_mcu_sharedram[0xf1] = 0xb3;
			m_mcu_sharedram[0xf0] = 0xff;
		}


		// The following is missing from Knight Boy
		// this should be equivalent to the obfuscated kiki_clogic() below
		{
			static const u8 db[16]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x10,0x18,0x00,0x00,0x00,0x00};
			u16 sy = m_mcu_sharedram[0xa0] + ((0x18) >> 1);
			u16 sx = m_mcu_sharedram[0xa1] + ((0x18) >> 1);

			for (i = 0; i < 0x38; i += 8)
			{
				u8 hw = db[m_mcu_sharedram[0x20 + i] & 0xf];

				if (hw)
				{
					u16 xdiff = sx - (u16(m_mcu_sharedram[0x20 + i + 6]) << 8 | m_mcu_sharedram[0x20 + i + 7]);
					if (xdiff < hw)
					{
						u16 ydiff = sy - (u16(m_mcu_sharedram[0x20 + i + 4]) << 8 | m_mcu_sharedram[0x20 + i + 5]);
						if (ydiff < hw)
							m_mcu_sharedram[0xa2] = 1; // we have a collision
					}
				}
			}
		}
	}
}


INTERRUPT_GEN_MEMBER(kikikai_state::kikikai_interrupt)
{
	device.execute().set_input_line(0, ASSERT_LINE);
}


INTERRUPT_GEN_MEMBER(kikikai_simulation_state::kikikai_interrupt)
{
	if (m_kikikai_simulated_mcu_running)
		mcu_simulate();

	device.execute().set_input_line(0, ASSERT_LINE);
}


IRQ_CALLBACK_MEMBER(kikikai_state::mcram_vect_r)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
	return m_mcu_sharedram[0];
}

#if 0
/***************************************************************************

 Collision logic used by Kiki Kaikai (theoretical)

***************************************************************************/
#define KIKI_CL_OUT 0xa2
#define KIKI_CL_TRIGGER 0xa3
#define DCWIDTH 0
#define DCHEIGHT 0

void kikikai_state::kiki_clogic(int address, int latch)
{
	static const u8 db[16]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x10,0x18,0x00,0x00,0x00,0x00};
	int sy, sx, hw, i, qptr, diff1, diff2;

	if (address != KIKI_CL_TRIGGER) // m_queue latched data
	{
		m_queue[m_qfront++] = latch;
		m_qfront &= 0x3f;
	}
	else if (m_qstate ^= 1) // scan m_queue
	{
		sy = m_queue[(m_qfront-0x3a)&0x3f] + ((0x18-DCHEIGHT)>>1);
		sx = m_queue[(m_qfront-0x39)&0x3f] + ((0x18-DCWIDTH)>>1);

		for (i=0x38; i; i-=8)
		{
			qptr = m_qfront - i;
			if (!(hw = db[m_queue[qptr&0x3f]&0xf])) continue;

			diff1 = sx - (short)(m_queue[(qptr+6)&0x3f]<<8|m_queue[(qptr+7)&0x3f]) + DCWIDTH;
			diff2 = diff1 - (hw + DCWIDTH);
			if ((diff1^diff2)<0)
			{
				diff1 = sy - (short)(m_queue[(qptr+4)&0x3f]<<8|m_queue[(qptr+5)&0x3f]) + DCHEIGHT;
				diff2 = diff1 - (hw + DCHEIGHT);
				if ((diff1^diff2)<0)
					m_mcu_sharedram[KIKI_CL_OUT] = 1; // we have a collision
			}
		}
	}
}
#endif


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
	if (data & 0x01 && ~m_port1_out & 0x01) {
		machine().bookkeeping().coin_counter_w(0, data & 0x01);
	}

	if (data & 0x02 && ~m_port1_out & 0x02) {
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
