/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "driver.h"


static UINT8 from_main,from_mcu;
static int mcu_sent = 0,main_sent = 0;


/***************************************************************************

 Fairy Land Story 68705 protection interface

 The following is ENTIRELY GUESSWORK!!!

 It seems, however, to be identical to Buggy Challenge.

***************************************************************************/

static UINT8 portA_in,portA_out,ddrA;

READ8_HANDLER( flstory_68705_portA_r )
{
//logerror("%04x: 68705 port A read %02x\n",activecpu_get_pc(),portA_in);
	return (portA_out & ddrA) | (portA_in & ~ddrA);
}

WRITE8_HANDLER( flstory_68705_portA_w )
{
//logerror("%04x: 68705 port A write %02x\n",activecpu_get_pc(),data);
	portA_out = data;
}

WRITE8_HANDLER( flstory_68705_ddrA_w )
{
	ddrA = data;
}



/*
 *  Port B connections:
 *
 *  all bits are logical 1 when read (+5V pullup)
 *
 *  1   W  when 1->0, enables latch which brings the command from main CPU (read from port A)
 *  2   W  when 0->1, copies port A to the latch for the main CPU
 */

static UINT8 portB_in,portB_out,ddrB;

READ8_HANDLER( flstory_68705_portB_r )
{
	return (portB_out & ddrB) | (portB_in & ~ddrB);
}

WRITE8_HANDLER( flstory_68705_portB_w )
{
//logerror("%04x: 68705 port B write %02x\n",activecpu_get_pc(),data);

	if ((ddrB & 0x02) && (~data & 0x02) && (portB_out & 0x02))
	{
		portA_in = from_main;
		if (main_sent) cpunum_set_input_line(2,0,CLEAR_LINE);
		main_sent = 0;
logerror("read command %02x from main cpu\n",portA_in);
	}
	if ((ddrB & 0x04) && (data & 0x04) && (~portB_out & 0x04))
	{
logerror("send command %02x to main cpu\n",portA_out);
		from_mcu = portA_out;
		mcu_sent = 1;
	}

	portB_out = data;
}

WRITE8_HANDLER( flstory_68705_ddrB_w )
{
	ddrB = data;
}


static UINT8 portC_in,portC_out,ddrC;

READ8_HANDLER( flstory_68705_portC_r )
{
	portC_in = 0;
	if (main_sent) portC_in |= 0x01;
	if (!mcu_sent) portC_in |= 0x02;
//logerror("%04x: 68705 port C read %02x\n",activecpu_get_pc(),portC_in);
	return (portC_out & ddrC) | (portC_in & ~ddrC);
}

WRITE8_HANDLER( flstory_68705_portC_w )
{
logerror("%04x: 68705 port C write %02x\n",activecpu_get_pc(),data);
	portC_out = data;
}

WRITE8_HANDLER( flstory_68705_ddrC_w )
{
	ddrC = data;
}

WRITE8_HANDLER( flstory_mcu_w )
{
logerror("%04x: mcu_w %02x\n",activecpu_get_pc(),data);
	from_main = data;
	main_sent = 1;
	cpunum_set_input_line(2,0,ASSERT_LINE);
}

READ8_HANDLER( flstory_mcu_r )
{
logerror("%04x: mcu_r %02x\n",activecpu_get_pc(),from_mcu);
	mcu_sent = 0;
	return from_mcu;
}

READ8_HANDLER( flstory_mcu_status_r )
{
	int res = 0;

	/* bit 0 = when 1, mcu is ready to receive data from main cpu */
	/* bit 1 = when 1, mcu has sent data to the main cpu */
//logerror("%04x: mcu_status_r\n",activecpu_get_pc());
	if (!main_sent) res |= 0x01;
	if (mcu_sent) res |= 0x02;

	return res;
}

WRITE8_HANDLER( onna34ro_mcu_w )
{
	UINT8 *RAM = memory_region(REGION_CPU1);
	UINT16 score_adr = RAM[0xe29e]*0x100 + RAM[0xe29d];

	switch (data)
	{
		case 0x0e:
			from_mcu = 0xff;
			break;
		case 0x01:
			from_mcu = 0x6a;
			break;
		case 0x40:
			from_mcu = RAM[score_adr];			/* score l*/
			break;
		case 0x41:
			from_mcu = RAM[score_adr+1];		/* score m*/
			break;
		case 0x42:
			from_mcu = RAM[score_adr+2] & 0x0f;	/* score h*/
			break;
		default:
			from_mcu = 0x80;
	}
}

READ8_HANDLER( onna34ro_mcu_r )
{
	return from_mcu;
}

READ8_HANDLER( onna34ro_mcu_status_r )
{
	int res = 3;

	return res;
}


#define VICTNINE_MCU_SEED	(memory_region(REGION_CPU1)[0xE685])

static UINT8 victnine_mcu_data[0x100] =
{
	0x3e, 0x08, 0xdd, 0x29, 0xcb, 0x14, 0xfd, 0x29,
	0xcb, 0x15, 0xd9, 0x29, 0xd9, 0x30, 0x0d, 0xd9,
	0x19, 0xd9, 0xdd, 0x09, 0x30, 0x01, 0x24, 0xfd,
	0x19, 0x30, 0x01, 0x2c, 0x3d, 0x20, 0xe3, 0xc9,
	0x11, 0x14, 0x00, 0x19, 0x7e, 0x32, 0xed, 0xe4,
	0x2a, 0x52, 0xe5, 0x22, 0xe9, 0xe4, 0x2a, 0x54,
	0xe5, 0x22, 0xeb, 0xe4, 0x21, 0x2a, 0xe6, 0xfe,
	0x06, 0x38, 0x02, 0xcb, 0xc6, 0xcb, 0xce, 0xc9,
	0x06, 0x00, 0x3a, 0xaa, 0xe4, 0x07, 0x07, 0x07,
	0xb0, 0x47, 0x3a, 0xab, 0xe4, 0x07, 0x07, 0xb0,
	0x47, 0x3a, 0xac, 0xe4, 0xe6, 0x03, 0xb0, 0x21,
	0xe3, 0xe6, 0xc9, 0x38, 0xe1, 0x29, 0x07, 0xc9,
	0x23, 0x7e, 0x47, 0xe6, 0x1f, 0x32, 0x0c, 0xe6,
	0x78, 0xe6, 0xe0, 0x07, 0x07, 0x47, 0xe6, 0x03,
	0x28, 0x06, 0xcb, 0x7a, 0x28, 0x02, 0xc6, 0x02,
	0x32, 0x0a, 0xe6, 0x78, 0xe6, 0x80, 0xc9, 0x3a,
	0x21, 0x29, 0xe5, 0x7e, 0xe6, 0xf8, 0xf6, 0x01,
	0x77, 0x23, 0x3a, 0x0a, 0xe6, 0x77, 0x21, 0x08,
	0xe6, 0xcb, 0xc6, 0xcb, 0x8e, 0x3a, 0x2b, 0xe5,
	0x21, 0xff, 0xe5, 0xfe, 0x02, 0xc9, 0x1f, 0xc6,
	0x47, 0x3a, 0xaa, 0xe4, 0xa7, 0x21, 0x00, 0xe5,
	0x28, 0x03, 0x21, 0x1b, 0xe5, 0x70, 0x3a, 0xaa,
	0xe4, 0xee, 0x01, 0x32, 0xaa, 0xe4, 0x21, 0xb0,
	0xe4, 0x34, 0x23, 0x36, 0x00, 0xc9, 0x2b, 0xb2,
	0xaf, 0x77, 0x12, 0x23, 0x13, 0x3c, 0xfe, 0x09,
	0x20, 0xf7, 0x3e, 0x01, 0x32, 0xad, 0xe4, 0x21,
	0x48, 0xe5, 0xcb, 0xfe, 0xc9, 0x32, 0xe5, 0xaa,
	0x21, 0x00, 0x13, 0xe4, 0x47, 0x1b, 0xa1, 0xc9,
	0x00, 0x08, 0x04, 0x0c, 0x05, 0x0d, 0x06, 0x0e,
	0x22, 0x66, 0xaa, 0x22, 0x33, 0x01, 0x11, 0x88,
	0x06, 0x05, 0x03, 0x04, 0x08, 0x01, 0x03, 0x02,
	0x06, 0x07, 0x02, 0x03, 0x15, 0x17, 0x11, 0x13
};

static int mcu_select = 0;

WRITE8_HANDLER( victnine_mcu_w )
{
	UINT8 seed = VICTNINE_MCU_SEED;

	if (!seed && (data & 0x37) == 0x37)
	{
		from_mcu = 0xa6;
		logerror("mcu initialize (%02x)\n", data);
	}
	else
	{
		data += seed;

		if ((data & ~0x1f) == 0xa0)
		{
			mcu_select = data & 0x1f;
			//logerror("mcu select: 0x%02x\n", mcu_select);
		}
		else if (data < 0x20)
		{
			int offset = mcu_select * 8 + data;

			//logerror("mcu fetch: 0x%02x\n", offset);
			from_mcu = victnine_mcu_data[offset];
		}
		else if (data >= 0x38 && data <= 0x3a)
		{
			UINT8 *RAM = memory_region(REGION_CPU1);

			from_mcu = RAM[0xe691 - 0x38 + data];
		}
		else
		{
			//logerror("mcu: 0x%02x: unknown command\n", data);
		}
	}
}

READ8_HANDLER( victnine_mcu_r )
{
	//logerror("%04x: mcu read (0x%02x)\n", activecpu_get_previouspc(), from_mcu);

	return from_mcu - VICTNINE_MCU_SEED;
}

READ8_HANDLER( victnine_mcu_status_r )
{
	int res = 3;

	return res;
}
