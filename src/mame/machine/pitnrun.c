/***************************************************************************

    Pit&Run

    Based on TaitsoSJ driver
    68705 has access to Z80 memory space

***************************************************************************/

#include "emu.h"
#include "cpu/m6805/m6805.h"
#include "includes/pitnrun.h"


static UINT8 fromz80,toz80;
static int zaccept,zready;

MACHINE_RESET( pitnrun )
{
	zaccept = 1;
	zready = 0;
	cputag_set_input_line(machine, "mcu", 0, CLEAR_LINE);
}

static TIMER_CALLBACK( pitnrun_mcu_real_data_r )
{
	zaccept = 1;
}

READ8_HANDLER( pitnrun_mcu_data_r )
{
	timer_call_after_resynch(space->machine, NULL, 0,pitnrun_mcu_real_data_r);
	return toz80;
}

static TIMER_CALLBACK( pitnrun_mcu_real_data_w )
{
	zready = 1;
	cputag_set_input_line(machine, "mcu", 0, ASSERT_LINE);
	fromz80 = param;
}

WRITE8_HANDLER( pitnrun_mcu_data_w )
{
	timer_call_after_resynch(space->machine, NULL, data,pitnrun_mcu_real_data_w);
	cpuexec_boost_interleave(space->machine, attotime_zero, ATTOTIME_IN_USEC(5));
}

READ8_HANDLER( pitnrun_mcu_status_r )
{
	/* mcu synchronization */
	/* bit 0 = the 68705 has read data from the Z80 */
	/* bit 1 = the 68705 has written data for the Z80 */
	return ~((zready << 1) | (zaccept << 0));
}

static UINT8 portA_in,portA_out;

READ8_HANDLER( pitnrun_68705_portA_r )
{
	return portA_in;
}

WRITE8_HANDLER( pitnrun_68705_portA_w )
{
	portA_out = data;
}



/*
 *  Port B connections:
 *
 *  all bits are logical 1 when read (+5V pullup)
 *
 *  0   W  !68INTRQ
 *  1   W  !68LRD (enables latch which holds command from the Z80)
 *  2   W  !68LWR (loads the latch which holds data for the Z80, and sets a
 *                 status bit so the Z80 knows there's data waiting)
 *  3   W  to Z80 !BUSRQ (aka !WAIT) pin
 *  4   W  !68WRITE (triggers write to main Z80 memory area )
 *  5   W  !68READ (triggers read from main Z80 memory area )
 *  6   W  !LAL (loads the latch which holds the low 8 bits of the address of
 *               the main Z80 memory location to access)
 *  7   W  !UAL (loads the latch which holds the high 8 bits of the address of
 *               the main Z80 memory location to access)
 */

READ8_HANDLER( pitnrun_68705_portB_r )
{
	return 0xff;
}

static int address;

static TIMER_CALLBACK( pitnrun_mcu_data_real_r )
{
	zready = 0;
}

static TIMER_CALLBACK( pitnrun_mcu_status_real_w )
{
	toz80 = param;
	zaccept = 0;
}

WRITE8_HANDLER( pitnrun_68705_portB_w )
{
	const address_space *cpu0space = cputag_get_address_space(space->machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	if (~data & 0x02)
	{
		/* 68705 is going to read data from the Z80 */
		timer_call_after_resynch(space->machine, NULL, 0,pitnrun_mcu_data_real_r);
		cputag_set_input_line(space->machine, "mcu",0,CLEAR_LINE);
		portA_in = fromz80;
	}
	if (~data & 0x04)
	{
		/* 68705 is writing data for the Z80 */
		timer_call_after_resynch(space->machine, NULL, portA_out,pitnrun_mcu_status_real_w);
	}
	if (~data & 0x10)
	{
		memory_write_byte(cpu0space, address, portA_out);
	}
	if (~data & 0x20)
	{
		portA_in = memory_read_byte(cpu0space, address);
	}
	if (~data & 0x40)
	{
		address = (address & 0xff00) | portA_out;
	}
	if (~data & 0x80)
	{
		address = (address & 0x00ff) | (portA_out << 8);
	}
}

/*
 *  Port C connections:
 *
 *  0   R  ZREADY (1 when the Z80 has written a command in the latch)
 *  1   R  ZACCEPT (1 when the Z80 has read data from the latch)
 *  2   R  from Z80 !BUSAK pin
 *  3   R  68INTAK (goes 0 when the interrupt request done with 68INTRQ
 *                  passes through)
 */

READ8_HANDLER( pitnrun_68705_portC_r )
{
	return (zready << 0) | (zaccept << 1);
}
