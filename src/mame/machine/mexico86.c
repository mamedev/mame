#include "emu.h"
#include "deprecat.h"
#include "includes/mexico86.h"


UINT8 *mexico86_protection_ram;


static int kikikai_mcu_running, kikikai_mcu_initialised;


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
	cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_RESET, (data & 4) ? CLEAR_LINE : ASSERT_LINE);
	if (devtag_get_device(space->machine, "mcu") != NULL)
	{
		// mexico 86, knight boy
		cputag_set_input_line(space->machine, "mcu", INPUT_LINE_RESET, (data & 2) ? CLEAR_LINE : ASSERT_LINE);
	}
	else
	{
		// simulation for KiKi KaiKai
		kikikai_mcu_running = data & 2;
		if (!kikikai_mcu_running)
			kikikai_mcu_initialised = 0;
	}
}


/***************************************************************************

 KiKi KaiKai MCU simulation

 This is derived from examination of the bootleg 68705 MCU code, with an
 addition to fix collision detection which is missing from the bootleg.

***************************************************************************/

static void mcu_simulate(running_machine *machine)
{
	if (!kikikai_mcu_initialised)
	{
		if (mexico86_protection_ram[0x01] == 0x00)
		{
logerror("initialising MCU\n");
			mexico86_protection_ram[0x04] = 0xfc;	// coin inputs
			mexico86_protection_ram[0x02] = 0xff;	// player 1
			mexico86_protection_ram[0x03] = 0xff;	// player 2
			mexico86_protection_ram[0x1b] = 0xff;	// active player
			mexico86_protection_ram[0x06] = 0xff;	// must be FF otherwise PS4 ERROR
			mexico86_protection_ram[0x07] = 0x03;	// must be 03 otherwise PS4 ERROR
			mexico86_protection_ram[0x00] = 0x00;
			kikikai_mcu_initialised = 1;
		}
	}

	if (kikikai_mcu_initialised)
	{
		int i;
		static int coin_last;
		int coin_curr;


		coin_curr = ~input_port_read(machine, "IN0") & 1;
		if (coin_curr && !coin_last && mexico86_protection_ram[0x01] < 9)
		{
			mexico86_protection_ram[0x01]++;	// increase credits counter
			mexico86_protection_ram[0x0a] = 0x01;	// set flag (coin inserted sound is not played otherwise)
		}
		coin_last = coin_curr;

		mexico86_protection_ram[0x04] = 0x3c;	// coin inputs

		mexico86_protection_ram[0x02] = BITSWAP8(input_port_read(machine, "IN1"), 7,6,5,4,2,3,1,0);	// player 1
		mexico86_protection_ram[0x03] = BITSWAP8(input_port_read(machine, "IN2"), 7,6,5,4,2,3,1,0);	// player 2

		if (mexico86_protection_ram[0x19] == 0xaa)	// player 2 active
			mexico86_protection_ram[0x1b] = mexico86_protection_ram[0x03];
		else
			mexico86_protection_ram[0x1b] = mexico86_protection_ram[0x02];


		for (i = 0; i < 0x10; i += 2)
			mexico86_protection_ram[i + 0xb1] = mexico86_protection_ram[i + 0xb0];

		for (i = 0; i < 0x0a; i++)
			mexico86_protection_ram[i + 0xc0] = mexico86_protection_ram[i + 0x90] + 1;

		if (mexico86_protection_ram[0xd1] == 0xff)
		{
			if (mexico86_protection_ram[0xd0] > 0 && mexico86_protection_ram[0xd0] < 4)
			{
				mexico86_protection_ram[0xd2] = 0x81;
				mexico86_protection_ram[0xd0] = 0xff;
			}
		}


		if (mexico86_protection_ram[0xe0] > 0 && mexico86_protection_ram[0xe0] < 4)
		{
			static const UINT8 answers[3][16] =
			{
				{ 0x00,0x40,0x48,0x50,0x58,0x60,0x68,0x70,0x78,0x80,0x88,0x00,0x00,0x00,0x00,0x00 },
				{ 0x00,0x04,0x08,0x0C,0x10,0x14,0x18,0x1C,0x20,0x31,0x2B,0x35,0x00,0x00,0x00,0x00 },
				{ 0x00,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x03,0x0A,0x0B,0x14,0x00,0x00,0x00,0x00 },
			};
			int table = mexico86_protection_ram[0xe0] - 1;

			for (i = 1; i < 0x10; i++)
				mexico86_protection_ram[0xe0 + i] = answers[table][i];
			mexico86_protection_ram[0xe0] = 0xff;
		}

		if (mexico86_protection_ram[0xf0] > 0 && mexico86_protection_ram[0xf0] < 4)
		{
			mexico86_protection_ram[0xf1] = 0xb3;
			mexico86_protection_ram[0xf0] = 0xff;
		}


		// The following is missing from Knight Boy
		// this should be equivalent to the obfuscated kiki_clogic() below
		{
			static const UINT8 db[16]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x10,0x18,0x00,0x00,0x00,0x00};
			UINT16 sy = mexico86_protection_ram[0xa0] + ((0x18)>>1);
			UINT16 sx = mexico86_protection_ram[0xa1] + ((0x18)>>1);

			for (i = 0; i < 0x38; i += 8)
			{
				UINT8 hw = db[mexico86_protection_ram[0x20 + i] & 0xf];

				if (hw)
				{
					UINT16 xdiff = sx - ((UINT16)mexico86_protection_ram[0x20 + i+6] << 8 | mexico86_protection_ram[0x20 + i+7]);
					if (xdiff < hw)
					{
						UINT16 ydiff = sy - ((UINT16)mexico86_protection_ram[0x20 + i+4] << 8 | mexico86_protection_ram[0x20 + i+5]);
						if (ydiff < hw)
							mexico86_protection_ram[0xa2] = 1; // we have a collision
					}
				}
			}
		}
	}
}


INTERRUPT_GEN( kikikai_interrupt )
{
	if (kikikai_mcu_running)
		mcu_simulate(device->machine);

	cpu_set_input_line_vector(device,0,mexico86_protection_ram[0]);
	cpu_set_input_line(device,0,HOLD_LINE);
}



#if 0
//AT
/***************************************************************************

 Collision logic used by Kiki Kaikai (theoretical)

***************************************************************************/
#define KIKI_CL_OUT 0xa2
#define KIKI_CL_TRIGGER 0xa3
#define DCWIDTH 0
#define DCHEIGHT 0

static void kiki_clogic(int address, int latch)
{
	static const UINT8 db[16]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x10,0x18,0x00,0x00,0x00,0x00};
	static UINT8 queue[64];
	static int qfront = 0, state = 0;
	int sy, sx, hw, i, qptr, diff1, diff2;

	if (address != KIKI_CL_TRIGGER) // queue latched data
	{
		queue[qfront++] = latch;
		qfront &= 0x3f;
	}
	else if (state ^= 1) // scan queue
	{
		sy = queue[(qfront-0x3a)&0x3f] + ((0x18-DCHEIGHT)>>1);
		sx = queue[(qfront-0x39)&0x3f] + ((0x18-DCWIDTH)>>1);

		for (i=0x38; i; i-=8)
		{
			qptr = qfront - i;
			if (!(hw = db[queue[qptr&0x3f]&0xf])) continue;

			diff1 = sx - (short)(queue[(qptr+6)&0x3f]<<8|queue[(qptr+7)&0x3f]) + DCWIDTH;
			diff2 = diff1 - (hw + DCWIDTH);
			if ((diff1^diff2)<0)
			{
				diff1 = sy - (short)(queue[(qptr+4)&0x3f]<<8|queue[(qptr+5)&0x3f]) + DCHEIGHT;
				diff2 = diff1 - (hw + DCHEIGHT);
				if ((diff1^diff2)<0)
					mexico86_protection_ram[KIKI_CL_OUT] = 1; // we have a collision
			}
		}
	}
}
//ZT
#endif


/***************************************************************************

 Mexico 86 68705 protection interface

 The following is ENTIRELY GUESSWORK!!!

***************************************************************************/

INTERRUPT_GEN( mexico86_m68705_interrupt )
{
	/* I don't know how to handle the interrupt line so I just toggle it every time. */
	if (cpu_getiloops(device) & 1)
		cpu_set_input_line(device,0,CLEAR_LINE);
	else
		cpu_set_input_line(device,0,ASSERT_LINE);
}



static UINT8 portA_in,portA_out,ddrA;

READ8_HANDLER( mexico86_68705_portA_r )
{
//logerror("%04x: 68705 port A read %02x\n",cpu_get_pc(space->cpu),portA_in);
	return (portA_out & ddrA) | (portA_in & ~ddrA);
}

WRITE8_HANDLER( mexico86_68705_portA_w )
{
//logerror("%04x: 68705 port A write %02x\n",cpu_get_pc(space->cpu),data);
	portA_out = data;
}

WRITE8_HANDLER( mexico86_68705_ddrA_w )
{
	ddrA = data;
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

static UINT8 portB_in,portB_out,ddrB;

READ8_HANDLER( mexico86_68705_portB_r )
{
	return (portB_out & ddrB) | (portB_in & ~ddrB);
}

static int address,latch;

WRITE8_HANDLER( mexico86_68705_portB_w )
{
//logerror("%04x: 68705 port B write %02x\n",cpu_get_pc(space->cpu),data);

	if ((ddrB & 0x01) && (~data & 0x01) && (portB_out & 0x01))
	{
		portA_in = latch;
	}
	if ((ddrB & 0x02) && (data & 0x02) && (~portB_out & 0x02)) /* positive edge trigger */
	{
		address = portA_out;
//if (address >= 0x80) logerror("%04x: 68705 address %02x\n",cpu_get_pc(space->cpu),portA_out);
	}
	if ((ddrB & 0x08) && (~data & 0x08) && (portB_out & 0x08))
	{
		if (data & 0x10)    /* read */
		{
			if (data & 0x04)
			{
//logerror("%04x: 68705 read %02x from address %04x\n",cpu_get_pc(space->cpu),shared[0x800+address],address);
				latch = mexico86_protection_ram[address];
			}
			else
			{
//logerror("%04x: 68705 read input port %04x\n",cpu_get_pc(space->cpu),address);
				latch = input_port_read(space->machine, (address & 1) ? "IN2" : "IN1");
			}
		}
		else    /* write */
		{
//logerror("%04x: 68705 write %02x to address %04x\n",cpu_get_pc(space->cpu),portA_out,address);
				mexico86_protection_ram[address] = portA_out;
		}
	}
	if ((ddrB & 0x20) && (data & 0x20) && (~portB_out & 0x20))
	{
		cpu_set_input_line_vector(devtag_get_device(space->machine, "maincpu"), 0, mexico86_protection_ram[0]);
		cputag_set_input_line(space->machine, "maincpu", 0, HOLD_LINE); //AT: HOLD_LINE works better in Z80 interrupt mode 1.
	}
	if ((ddrB & 0x40) && (~data & 0x40) && (portB_out & 0x40))
	{
logerror("%04x: 68705 unknown port B bit %02x\n",cpu_get_pc(space->cpu),data);
	}
	if ((ddrB & 0x80) && (~data & 0x80) && (portB_out & 0x80))
	{
logerror("%04x: 68705 unknown port B bit %02x\n",cpu_get_pc(space->cpu),data);
	}

	portB_out = data;
}

WRITE8_HANDLER( mexico86_68705_ddrB_w )
{
	ddrB = data;
}
