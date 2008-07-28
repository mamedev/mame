/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "cpu/z80/z80.h"
#include "includes/bublbobl.h"


UINT8 *bublbobl_mcu_sharedram;


WRITE8_HANDLER( bublbobl_bankswitch_w )
{
	UINT8 *ROM = memory_region(machine, RGNCLASS_CPU, "main");

	/* bits 0-2 select ROM bank */
	memory_set_bankptr(1,&ROM[0x10000 + 0x4000 * ((data ^ 4) & 7)]);

	/* bit 3 n.c. */

	/* bit 4 resets second Z80 */
	cpunum_set_input_line(machine, 1, INPUT_LINE_RESET, (data & 0x10) ? CLEAR_LINE : ASSERT_LINE);

	/* bit 5 resets mcu */
	if (machine->config->cpu[3].type != CPU_DUMMY)	// only if we have a MCU
		cpunum_set_input_line(machine, 3, INPUT_LINE_RESET, (data & 0x20) ? CLEAR_LINE : ASSERT_LINE);

	/* bit 6 enables display */
	bublbobl_video_enable = data & 0x40;

	/* bit 7 flips screen */
	flip_screen_set(data & 0x80);
}

WRITE8_HANDLER( tokio_bankswitch_w )
{
	UINT8 *ROM = memory_region(machine, RGNCLASS_CPU, "main");

	/* bits 0-2 select ROM bank */
	memory_set_bankptr(1,&ROM[0x10000 + 0x4000 * (data & 7)]);

	/* bits 3-7 unknown */
}

WRITE8_HANDLER( tokio_videoctrl_w )
{
	/* bit 7 flips screen */
	flip_screen_set(data & 0x80);

	/* other bits unknown */
}

WRITE8_HANDLER( bublbobl_nmitrigger_w )
{
	cpunum_set_input_line(machine, 1,INPUT_LINE_NMI,PULSE_LINE);
}


static const UINT8 tokio_prot_data[] =
{
	0x6c,
	0x7f,0x5f,0x7f,0x6f,0x5f,0x77,0x5f,0x7f,0x5f,0x7f,0x5f,0x7f,0x5b,0x7f,0x5f,0x7f,
	0x5f,0x77,0x59,0x7f,0x5e,0x7e,0x5f,0x6d,0x57,0x7f,0x5d,0x7d,0x5f,0x7e,0x5f,0x7f,
	0x5d,0x7d,0x5f,0x7e,0x5e,0x79,0x5f,0x7f,0x5f,0x7f,0x5d,0x7f,0x5f,0x7b,0x5d,0x7e,
	0x5f,0x7f,0x5d,0x7d,0x5f,0x7e,0x5e,0x7e,0x5f,0x7d,0x5f,0x7f,0x5f,0x7e,0x7f,0x5f,
	0x01,0x00,0x02,0x01,0x01,0x01,0x03,0x00,0x05,0x02,0x04,0x01,0x03,0x00,0x05,0x01,
	0x02,0x03,0x00,0x04,0x04,0x01,0x02,0x00,0x05,0x03,0x02,0x01,0x04,0x05,0x00,0x03,
	0x00,0x05,0x02,0x01,0x03,0x04,0x05,0x00,0x01,0x04,0x04,0x02,0x01,0x04,0x01,0x00,
	0x03,0x01,0x02,0x05,0x00,0x03,0x00,0x01,0x02,0x00,0x03,0x04,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x00,0x00,0x00,0x00,0x01,0x02,0x00,0x00,0x00,
	0x01,0x02,0x01,0x00,0x00,0x00,0x02,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x01,0x01,
	0x00,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x00,0x01,0x01,0x00,0x00,0x02,0x01,0x00,
	0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x01
};
static int tokio_prot_count;

READ8_HANDLER( tokio_mcu_r )
{
	tokio_prot_count %= sizeof(tokio_prot_data);
	return tokio_prot_data[tokio_prot_count++];
}

READ8_HANDLER( tokiob_mcu_r )
{
	return 0xbf; /* ad-hoc value set to pass initial testing */
}



static int sound_nmi_enable,pending_nmi;

static TIMER_CALLBACK( nmi_callback )
{
	if (sound_nmi_enable) cpunum_set_input_line(machine, 2,INPUT_LINE_NMI,PULSE_LINE);
	else pending_nmi = 1;
}

WRITE8_HANDLER( bublbobl_sound_command_w )
{
	soundlatch_w(machine,offset,data);
	timer_call_after_resynch(NULL, data,nmi_callback);
}

WRITE8_HANDLER( bublbobl_sh_nmi_disable_w )
{
	sound_nmi_enable = 0;
}

WRITE8_HANDLER( bublbobl_sh_nmi_enable_w )
{
	sound_nmi_enable = 1;
	if (pending_nmi)
	{
		cpunum_set_input_line(machine, 2,INPUT_LINE_NMI,PULSE_LINE);
		pending_nmi = 0;
	}
}





/***************************************************************************

Bubble Bobble MCU

***************************************************************************/

static UINT8 ddr1, ddr2, ddr3, ddr4;
static UINT8 port1_in, port2_in, port3_in, port4_in;
static UINT8 port1_out, port2_out, port3_out, port4_out;

READ8_HANDLER( bublbobl_mcu_ddr1_r )
{
	return ddr1;
}

WRITE8_HANDLER( bublbobl_mcu_ddr1_w )
{
	ddr1 = data;
}

READ8_HANDLER( bublbobl_mcu_ddr2_r )
{
	return ddr2;
}

WRITE8_HANDLER( bublbobl_mcu_ddr2_w )
{
	ddr2 = data;
}

READ8_HANDLER( bublbobl_mcu_ddr3_r )
{
	return ddr3;
}

WRITE8_HANDLER( bublbobl_mcu_ddr3_w )
{
	ddr3 = data;
}

READ8_HANDLER( bublbobl_mcu_ddr4_r )
{
	return ddr4;
}

WRITE8_HANDLER( bublbobl_mcu_ddr4_w )
{
	ddr4 = data;
}

READ8_HANDLER( bublbobl_mcu_port1_r )
{
//logerror("%04x: 6801U4 port 1 read\n",activecpu_get_pc());
	port1_in = input_port_read(machine, "IN0");
	return (port1_out & ddr1) | (port1_in & ~ddr1);
}

WRITE8_HANDLER( bublbobl_mcu_port1_w )
{
//logerror("%04x: 6801U4 port 1 write %02x\n",activecpu_get_pc(),data);

	// bit 4: coin lockout
	coin_lockout_global_w(~data & 0x10);

	// bit 5: select 1-way or 2-way coin counter

	// bit 6: trigger IRQ on main CPU (jumper switchable to vblank)
	// trigger on high->low transition
	if ((port1_out & 0x40) && (~data & 0x40))
	{
//      logerror("triggering IRQ on main CPU\n");
		cpunum_set_input_line_vector(0,0,bublbobl_mcu_sharedram[0]);
		cpunum_set_input_line(machine, 0,0,HOLD_LINE);
	}

	// bit 7: select read or write shared RAM

	port1_out = data;
}

READ8_HANDLER( bublbobl_mcu_port2_r )
{
//logerror("%04x: 6801U4 port 2 read\n",activecpu_get_pc());
	return (port2_out & ddr2) | (port2_in & ~ddr2);
}

WRITE8_HANDLER( bublbobl_mcu_port2_w )
{
//logerror("%04x: 6801U4 port 2 write %02x\n",activecpu_get_pc(),data);
	static const char *portnames[] = { "DSW0", "DSW1", "IN1", "IN2" };

	// bits 0-3: bits 8-11 of shared RAM address

	// bit 4: clock (goes to PAL A78-04.12)
	// latch on low->high transition
	if ((~port2_out & 0x10) && (data & 0x10))
	{
		int address = port4_out | ((data & 0x0f) << 8);

		if (port1_out & 0x80)
		{
			// read
			if ((address & 0x0800) == 0x0000)
				port3_in = input_port_read(machine, portnames[address & 3]);
			else if ((address & 0x0c00) == 0x0c00)
				port3_in = bublbobl_mcu_sharedram[address & 0x03ff];
//          logerror("reading %02x from shared RAM %04x\n",port3_in,address);
		}
		else
		{
			// write
//          logerror("writing %02x to shared RAM %04x\n",port3_out,address);
			if ((address & 0x0c00) == 0x0c00)
				bublbobl_mcu_sharedram[address & 0x03ff] = port3_out;
		}
	}

	port2_out = data;
}

READ8_HANDLER( bublbobl_mcu_port3_r )
{
//logerror("%04x: 6801U4 port 3 read\n",activecpu_get_pc());
	return (port3_out & ddr3) | (port3_in & ~ddr3);
}

WRITE8_HANDLER( bublbobl_mcu_port3_w )
{
//logerror("%04x: 6801U4 port 3 write %02x\n",activecpu_get_pc(),data);

	port3_out = data;
}

READ8_HANDLER( bublbobl_mcu_port4_r )
{
//logerror("%04x: 6801U4 port 4 read\n",activecpu_get_pc());
	return (port4_out & ddr4) | (port4_in & ~ddr4);
}

WRITE8_HANDLER( bublbobl_mcu_port4_w )
{
//logerror("%04x: 6801U4 port 4 write %02x\n",activecpu_get_pc(),data);

	// bits 0-7 of shared RAM address

	port4_out = data;
}

/***************************************************************************

Bobble Bobble protection (IC43). This appears to be a PAL.

Note: the checks on the values returned by ic43_b_r are actually patched out
in boblbobl, so they don't matter. All checks are patched out in sboblbob.

***************************************************************************/

static int ic43_a,ic43_b;


READ8_HANDLER( boblbobl_ic43_a_r )
{
//  if (offset >= 2)
//      logerror("%04x: ic43_a_r (offs %d) res = %02x\n",activecpu_get_pc(),offset,res);

	if (offset == 0)
		return ic43_a << 4;
	else
		return mame_rand(machine) & 0xff;
}

WRITE8_HANDLER( boblbobl_ic43_a_w )
{
	int res = 0;

	switch (offset)
	{
		case 0:
			if (~ic43_a & 8) res ^= 1;
			if (~ic43_a & 1) res ^= 2;
			if (~ic43_a & 1) res ^= 4;
			if (~ic43_a & 2) res ^= 4;
			if (~ic43_a & 4) res ^= 8;
			break;
		case 1:
			if (~ic43_a & 8) res ^= 1;
			if (~ic43_a & 2) res ^= 1;
			if (~ic43_a & 8) res ^= 2;
			if (~ic43_a & 1) res ^= 4;
			if (~ic43_a & 4) res ^= 8;
			break;
		case 2:
			if (~ic43_a & 4) res ^= 1;
			if (~ic43_a & 8) res ^= 2;
			if (~ic43_a & 2) res ^= 4;
			if (~ic43_a & 1) res ^= 8;
			if (~ic43_a & 4) res ^= 8;
			break;
		case 3:
			if (~ic43_a & 2) res ^= 1;
			if (~ic43_a & 4) res ^= 2;
			if (~ic43_a & 8) res ^= 2;
			if (~ic43_a & 8) res ^= 4;
			if (~ic43_a & 1) res ^= 8;
			break;
	}
	ic43_a = res;
}

WRITE8_HANDLER( boblbobl_ic43_b_w )
{
	static const int xor[4] = { 4, 1, 8, 2 };

//  logerror("%04x: ic43_b_w (offs %d) %02x\n",activecpu_get_pc(),offset,data);
	ic43_b = (data >> 4) ^ xor[offset];
}

READ8_HANDLER( boblbobl_ic43_b_r )
{
//  logerror("%04x: ic43_b_r (offs %d)\n",activecpu_get_pc(),offset);
	if (offset == 0)
		return ic43_b << 4;
	else
		return 0xff;	// not used?
}



/***************************************************************************

 Bootleg Bubble Bobble 68705 protection interface

 This is used by the 68705 bootleg version. Note that this actually
 wasn't working 100%, for some unknown reason the enemy movement wasn't right.

 The following is ENTIRELY GUESSWORK!!!

***************************************************************************/


INTERRUPT_GEN( bublbobl_m68705_interrupt )
{
	/* I don't know how to handle the interrupt line so I just toggle it every time. */
	if (cpu_getiloops() & 1)
		cpunum_set_input_line(machine, 3,0,CLEAR_LINE);
	else
		cpunum_set_input_line(machine, 3,0,ASSERT_LINE);
}



static UINT8 portA_in,portA_out,ddrA;

READ8_HANDLER( bublbobl_68705_portA_r )
{
//logerror("%04x: 68705 port A read %02x\n",activecpu_get_pc(),portA_in);
	return (portA_out & ddrA) | (portA_in & ~ddrA);
}

WRITE8_HANDLER( bublbobl_68705_portA_w )
{
//logerror("%04x: 68705 port A write %02x\n",activecpu_get_pc(),data);
	portA_out = data;
}

WRITE8_HANDLER( bublbobl_68705_ddrA_w )
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
 *  2   W  loads the latch which holds the high 4 bits of the address of
 *               the main Z80 memory location to access
 *         00-07 = read input ports
 *         0c-0f = access z80 memory at 0xfc00
 *  3   W  selects Z80 memory access direction (0 = write 1 = read)
 *  4   W  clocks main Z80 memory access (goes to a PAL)
 *  5   W  clocks a flip-flop which causes IRQ on the main Z80
 *  6   W  not used?
 *  7   W  not used?
 */

static UINT8 portB_in,portB_out,ddrB;

READ8_HANDLER( bublbobl_68705_portB_r )
{
	return (portB_out & ddrB) | (portB_in & ~ddrB);
}

static int address,latch;

WRITE8_HANDLER( bublbobl_68705_portB_w )
{
//logerror("%04x: 68705 port B write %02x\n",activecpu_get_pc(),data);
	static const char *portnames[] = { "DSW0", "DSW1", "IN1", "IN2" };

	if ((ddrB & 0x01) && (~data & 0x01) && (portB_out & 0x01))
	{
		portA_in = latch;
	}
	if ((ddrB & 0x02) && (data & 0x02) && (~portB_out & 0x02)) /* positive edge trigger */
	{
		address = (address & 0xff00) | portA_out;
//logerror("%04x: 68705 address %02x\n",activecpu_get_pc(),portA_out);
	}
	if ((ddrB & 0x04) && (data & 0x04) && (~portB_out & 0x04)) /* positive edge trigger */
	{
		address = (address & 0x00ff) | ((portA_out & 0x0f) << 8);
	}
	if ((ddrB & 0x10) && (~data & 0x10) && (portB_out & 0x10))
	{
		if (data & 0x08)	/* read */
		{
			if ((address & 0x0800) == 0x0000)
			{
//logerror("%04x: 68705 read input port %02x\n",activecpu_get_pc(),address);
				latch = input_port_read(machine, portnames[address & 3]);
			}
			else if ((address & 0x0c00) == 0x0c00)
			{
//logerror("%04x: 68705 read %02x from address %04x\n",activecpu_get_pc(),bublbobl_mcu_sharedram[address],address);
				latch = bublbobl_mcu_sharedram[address & 0x03ff];
			}
			else
logerror("%04x: 68705 unknown read address %04x\n",activecpu_get_pc(),address);
		}
		else	/* write */
		{
			if ((address & 0x0c00) == 0x0c00)
			{
//logerror("%04x: 68705 write %02x to address %04x\n",activecpu_get_pc(),portA_out,address);
				bublbobl_mcu_sharedram[address & 0x03ff] = portA_out;
			}
			else
logerror("%04x: 68705 unknown write to address %04x\n",activecpu_get_pc(),address);
		}
	}
	if ((ddrB & 0x20) && (~data & 0x20) && (portB_out & 0x20))
	{
		/* hack to get random EXTEND letters (who is supposed to do this? 68705? PAL?) */
		bublbobl_mcu_sharedram[0x7c] = mame_rand(machine)%6;

		cpunum_set_input_line_vector(0,0,bublbobl_mcu_sharedram[0]);
		cpunum_set_input_line(machine, 0,0,HOLD_LINE);
	}
	if ((ddrB & 0x40) && (~data & 0x40) && (portB_out & 0x40))
	{
logerror("%04x: 68705 unknown port B bit %02x\n",activecpu_get_pc(),data);
	}
	if ((ddrB & 0x80) && (~data & 0x80) && (portB_out & 0x80))
	{
logerror("%04x: 68705 unknown port B bit %02x\n",activecpu_get_pc(),data);
	}

	portB_out = data;
}

WRITE8_HANDLER( bublbobl_68705_ddrB_w )
{
	ddrB = data;
}

