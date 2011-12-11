#include "emu.h"
#include "includes/mexico86.h"

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
WRITE8_HANDLER( mexico86_f008_w )
{
	mexico86_state *state = space->machine().driver_data<mexico86_state>();

	device_set_input_line(state->m_audiocpu, INPUT_LINE_RESET, (data & 4) ? CLEAR_LINE : ASSERT_LINE);

	if (state->m_mcu != NULL)
	{
		// mexico 86, knight boy
		device_set_input_line(state->m_mcu, INPUT_LINE_RESET, (data & 2) ? CLEAR_LINE : ASSERT_LINE);
	}
	else
	{
		// simulation for KiKi KaiKai
		state->m_mcu_running = data & 2;

		if (!state->m_mcu_running)
			state->m_mcu_initialised = 0;
	}
}


/***************************************************************************

 KiKi KaiKai MCU simulation

 This is derived from examination of the bootleg 68705 MCU code, with an
 addition to fix collision detection which is missing from the bootleg.

***************************************************************************/

static void mcu_simulate( running_machine &machine )
{
	mexico86_state *state = machine.driver_data<mexico86_state>();

	if (!state->m_mcu_initialised)
	{
		if (state->m_protection_ram[0x01] == 0x00)
		{
			logerror("initialising MCU\n");
			state->m_protection_ram[0x04] = 0xfc;	// coin inputs
			state->m_protection_ram[0x02] = 0xff;	// player 1
			state->m_protection_ram[0x03] = 0xff;	// player 2
			state->m_protection_ram[0x1b] = 0xff;	// active player
			state->m_protection_ram[0x06] = 0xff;	// must be FF otherwise PS4 ERROR
			state->m_protection_ram[0x07] = 0x03;	// must be 03 otherwise PS4 ERROR
			state->m_protection_ram[0x00] = 0x00;
			state->m_mcu_initialised = 1;
		}
	}

	if (state->m_mcu_initialised)
	{
		int i;
		int coin_curr;

		coin_curr = ~input_port_read(machine, "IN0") & 1;
		if (coin_curr && !state->m_coin_last && state->m_protection_ram[0x01] < 9)
		{
			state->m_protection_ram[0x01]++;	// increase credits counter
			state->m_protection_ram[0x0a] = 0x01;	// set flag (coin inserted sound is not played otherwise)
		}
		state->m_coin_last = coin_curr;

		state->m_protection_ram[0x04] = 0x3c;	// coin inputs

		state->m_protection_ram[0x02] = BITSWAP8(input_port_read(machine, "IN1"), 7,6,5,4,2,3,1,0);	// player 1
		state->m_protection_ram[0x03] = BITSWAP8(input_port_read(machine, "IN2"), 7,6,5,4,2,3,1,0);	// player 2

		if (state->m_protection_ram[0x19] == 0xaa)	// player 2 active
			state->m_protection_ram[0x1b] = state->m_protection_ram[0x03];
		else
			state->m_protection_ram[0x1b] = state->m_protection_ram[0x02];


		for (i = 0; i < 0x10; i += 2)
			state->m_protection_ram[i + 0xb1] = state->m_protection_ram[i + 0xb0];

		for (i = 0; i < 0x0a; i++)
			state->m_protection_ram[i + 0xc0] = state->m_protection_ram[i + 0x90] + 1;

		if (state->m_protection_ram[0xd1] == 0xff)
		{
			if (state->m_protection_ram[0xd0] > 0 && state->m_protection_ram[0xd0] < 4)
			{
				state->m_protection_ram[0xd2] = 0x81;
				state->m_protection_ram[0xd0] = 0xff;
			}
		}

		if (state->m_protection_ram[0xe0] > 0 && state->m_protection_ram[0xe0] < 4)
		{
			static const UINT8 answers[3][16] =
			{
				{ 0x00,0x40,0x48,0x50,0x58,0x60,0x68,0x70,0x78,0x80,0x88,0x00,0x00,0x00,0x00,0x00 },
				{ 0x00,0x04,0x08,0x0C,0x10,0x14,0x18,0x1C,0x20,0x31,0x2B,0x35,0x00,0x00,0x00,0x00 },
				{ 0x00,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x03,0x0A,0x0B,0x14,0x00,0x00,0x00,0x00 },
			};
			int table = state->m_protection_ram[0xe0] - 1;

			for (i = 1; i < 0x10; i++)
				state->m_protection_ram[0xe0 + i] = answers[table][i];
			state->m_protection_ram[0xe0] = 0xff;
		}

		if (state->m_protection_ram[0xf0] > 0 && state->m_protection_ram[0xf0] < 4)
		{
			state->m_protection_ram[0xf1] = 0xb3;
			state->m_protection_ram[0xf0] = 0xff;
		}


		// The following is missing from Knight Boy
		// this should be equivalent to the obfuscated kiki_clogic() below
		{
			static const UINT8 db[16]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x10,0x18,0x00,0x00,0x00,0x00};
			UINT16 sy = state->m_protection_ram[0xa0] + ((0x18) >> 1);
			UINT16 sx = state->m_protection_ram[0xa1] + ((0x18) >> 1);

			for (i = 0; i < 0x38; i += 8)
			{
				UINT8 hw = db[state->m_protection_ram[0x20 + i] & 0xf];

				if (hw)
				{
					UINT16 xdiff = sx - ((UINT16)state->m_protection_ram[0x20 + i + 6] << 8 | state->m_protection_ram[0x20 + i + 7]);
					if (xdiff < hw)
					{
						UINT16 ydiff = sy - ((UINT16)state->m_protection_ram[0x20 + i + 4] << 8 | state->m_protection_ram[0x20 + i + 5]);
						if (ydiff < hw)
							state->m_protection_ram[0xa2] = 1; // we have a collision
					}
				}
			}
		}
	}
}


INTERRUPT_GEN( kikikai_interrupt )
{
	mexico86_state *state = device->machine().driver_data<mexico86_state>();

	if (state->m_mcu_running)
		mcu_simulate(device->machine());

	device_set_input_line_vector(device, 0, state->m_protection_ram[0]);
	device_set_input_line(device, 0, HOLD_LINE);
}



#if 0
/***************************************************************************

 Collision logic used by Kiki Kaikai (theoretical)

***************************************************************************/
#define KIKI_CL_OUT 0xa2
#define KIKI_CL_TRIGGER 0xa3
#define DCWIDTH 0
#define DCHEIGHT 0

static void kiki_clogic(running_machine &machine, int address, int latch)
{
	mexico86_state *state = machine.driver_data<mexico86_state>();
	static const UINT8 db[16]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x10,0x18,0x00,0x00,0x00,0x00};
	int sy, sx, hw, i, qptr, diff1, diff2;

	if (address != KIKI_CL_TRIGGER) // state->m_queue latched data
	{
		state->m_queue[state->m_qfront++] = latch;
		state->m_qfront &= 0x3f;
	}
	else if (state->m_qstate ^= 1) // scan state->m_queue
	{
		sy = state->m_queue[(state->m_qfront-0x3a)&0x3f] + ((0x18-DCHEIGHT)>>1);
		sx = state->m_queue[(state->m_qfront-0x39)&0x3f] + ((0x18-DCWIDTH)>>1);

		for (i=0x38; i; i-=8)
		{
			qptr = state->m_qfront - i;
			if (!(hw = db[state->m_queue[qptr&0x3f]&0xf])) continue;

			diff1 = sx - (short)(state->m_queue[(qptr+6)&0x3f]<<8|state->m_queue[(qptr+7)&0x3f]) + DCWIDTH;
			diff2 = diff1 - (hw + DCWIDTH);
			if ((diff1^diff2)<0)
			{
				diff1 = sy - (short)(state->m_queue[(qptr+4)&0x3f]<<8|state->m_queue[(qptr+5)&0x3f]) + DCHEIGHT;
				diff2 = diff1 - (hw + DCHEIGHT);
				if ((diff1^diff2)<0)
					state->m_protection_ram[KIKI_CL_OUT] = 1; // we have a collision
			}
		}
	}
}
#endif


/***************************************************************************

 Mexico 86 68705 protection interface

 The following is ENTIRELY GUESSWORK!!!

***************************************************************************/

INTERRUPT_GEN( mexico86_m68705_interrupt )
{
	device_set_input_line(device, 0, ASSERT_LINE);
}


READ8_HANDLER( mexico86_68705_port_a_r )
{
	mexico86_state *state = space->machine().driver_data<mexico86_state>();

	//logerror("%04x: 68705 port A read %02x\n", cpu_get_pc(&space->device()), state->m_port_a_in);
	return (state->m_port_a_out & state->m_ddr_a) | (state->m_port_a_in & ~state->m_ddr_a);
}

WRITE8_HANDLER( mexico86_68705_port_a_w )
{
	mexico86_state *state = space->machine().driver_data<mexico86_state>();

	//logerror("%04x: 68705 port A write %02x\n", cpu_get_pc(&space->device()), data);
	state->m_port_a_out = data;
}

WRITE8_HANDLER( mexico86_68705_ddr_a_w )
{
	mexico86_state *state = space->machine().driver_data<mexico86_state>();
	state->m_ddr_a = data;
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

READ8_HANDLER( mexico86_68705_port_b_r )
{
	mexico86_state *state = space->machine().driver_data<mexico86_state>();
	return (state->m_port_b_out & state->m_ddr_b) | (state->m_port_b_in & ~state->m_ddr_b);
}

WRITE8_HANDLER( mexico86_68705_port_b_w )
{
	mexico86_state *state = space->machine().driver_data<mexico86_state>();
	//logerror("%04x: 68705 port B write %02x\n", cpu_get_pc(&space->device()), data);

	if (BIT(state->m_ddr_b, 0) && BIT(~data, 0) && BIT(state->m_port_b_out, 0))
	{
		state->m_port_a_in = state->m_latch;
	}

	if (BIT(state->m_ddr_b, 1) && BIT(data, 1) && BIT(~state->m_port_b_out, 1)) /* positive edge trigger */
	{
		state->m_address = state->m_port_a_out;
		//if (state->m_address >= 0x80) logerror("%04x: 68705 address %02x\n", cpu_get_pc(&space->device()), state->m_port_a_out);
	}

	if (BIT(state->m_ddr_b, 3) && BIT(~data, 3) && BIT(state->m_port_b_out, 3))
	{
		if (data & 0x10)    /* read */
		{
			if (data & 0x04)
			{
				//logerror("%04x: 68705 read %02x from address %04x\n", cpu_get_pc(&space->device()), state->m_protection_ram[state->m_address], state->m_address);
				state->m_latch = state->m_protection_ram[state->m_address];
			}
			else
			{
				//logerror("%04x: 68705 read input port %04x\n", cpu_get_pc(&space->device()), state->m_address);
				state->m_latch = input_port_read(space->machine(), (state->m_address & 1) ? "IN2" : "IN1");
			}
		}
		else    /* write */
		{
				//logerror("%04x: 68705 write %02x to address %04x\n",cpu_get_pc(&space->device()), port_a_out, state->m_address);
				state->m_protection_ram[state->m_address] = state->m_port_a_out;
		}
	}

	if (BIT(state->m_ddr_b, 5) && BIT(data, 5) && BIT(~state->m_port_b_out, 5))
	{
		device_set_input_line_vector(state->m_maincpu, 0, state->m_protection_ram[0]);
		device_set_input_line(state->m_maincpu, 0, HOLD_LINE);        // HOLD_LINE works better in Z80 interrupt mode 1.
		device_set_input_line(state->m_mcu, 0, CLEAR_LINE);
	}

	if (BIT(state->m_ddr_b, 6) && BIT(~data, 6) && BIT(state->m_port_b_out, 6))
	{
		logerror("%04x: 68705 unknown port B bit %02x\n", cpu_get_pc(&space->device()), data);
	}

	if (BIT(state->m_ddr_b, 7) && BIT(~data, 7) && BIT(state->m_port_b_out, 7))
	{
		logerror("%04x: 68705 unknown port B bit %02x\n", cpu_get_pc(&space->device()), data);
	}

	state->m_port_b_out = data;
}

WRITE8_HANDLER( mexico86_68705_ddr_b_w )
{
	mexico86_state *state = space->machine().driver_data<mexico86_state>();
	state->m_ddr_b = data;
}
