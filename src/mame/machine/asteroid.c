/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "driver.h"
#include "machine/atari_vg.h"
#include "video/avgdvg.h"
#include "asteroid.h"

UINT8 *asteroid_ram1, *asteroid_ram2;

INTERRUPT_GEN( asteroid_interrupt )
{
	/* Turn off interrupts if self-test is enabled */
	if (!(readinputport(0) & 0x80))
		cpunum_set_input_line(0, INPUT_LINE_NMI, PULSE_LINE);
}

INTERRUPT_GEN( asterock_interrupt )
{
	/* Turn off interrupts if self-test is enabled */
	if ((readinputport(0) & 0x80))
		cpunum_set_input_line(0, INPUT_LINE_NMI, PULSE_LINE);
}

INTERRUPT_GEN( llander_interrupt )
{
	/* Turn off interrupts if self-test is enabled */
	if (readinputport(0) & 0x02)
		cpunum_set_input_line(0, INPUT_LINE_NMI, PULSE_LINE);
}


READ8_HANDLER( asteroid_IN0_r )
{

	int res;
	int bitmask;

	res=readinputport(0);

	bitmask = (1 << offset);

	if (activecpu_gettotalcycles() & 0x100)
		res |= 0x02;
	if (!avgdvg_done())
		res |= 0x04;

	if (res & bitmask)
		res = 0x80;
	else
		res = ~0x80;

	return res;
}


READ8_HANDLER( asteroib_IN0_r )
{
	int res;

	res=readinputport(0);

//  if (activecpu_gettotalcycles() & 0x100)
//      res |= 0x02;
	if (!avgdvg_done())
		res |= 0x80;

	return res;
}

READ8_HANDLER( asterock_IN0_r )
{
	int res;
	int bitmask;

	res=readinputport(0);

	bitmask = (1 << offset);

	if (activecpu_gettotalcycles() & 0x100)
		res |= 0x04;
	if (!avgdvg_done())
		res |= 0x01;

	if (res & bitmask)
		res = ~0x80;
	else
		res = 0x80;

	return res;
}

/*
 * These 7 memory locations are used to read the player's controls.
 * Typically, only the high bit is used. This is handled by one input port.
 */

READ8_HANDLER( asteroid_IN1_r )
{
	int res;
	int bitmask;

	res=readinputport(1);
	bitmask = (1 << offset);

	if (res & bitmask)
		res = 0x80;
	else
	 	res = ~0x80;
	return (res);
}


READ8_HANDLER( asteroid_DSW1_r )
{
	int res;
	int res1;

	res1 = readinputport(2);

	res = 0xfc | ((res1 >> (2 * (3 - (offset & 0x3)))) & 0x3);
	return res;
}


WRITE8_HANDLER( asteroid_bank_switch_w )
{
	if (data & 4)
	{
		memory_set_bankptr(1, asteroid_ram2);
		memory_set_bankptr(2, asteroid_ram1);
	}
	else
	{
		memory_set_bankptr(1, asteroid_ram1);
		memory_set_bankptr(2, asteroid_ram2);
	}

	set_led_status (0, ~data & 0x02);
	set_led_status (1, ~data & 0x01);
}


WRITE8_HANDLER( astdelux_bank_switch_w )
{
	if (data & 0x80)
	{
		memory_set_bankptr(1, asteroid_ram2);
		memory_set_bankptr(2, asteroid_ram1);
	}
	else
	{
		memory_set_bankptr(1, asteroid_ram1);
		memory_set_bankptr(2, asteroid_ram2);
	}
}


WRITE8_HANDLER( astdelux_led_w )
{
	set_led_status(offset,(data&0x80)?0:1);
}


MACHINE_RESET( asteroid )
{
	asteroid_bank_switch_w (0,0);
	avgdvg_reset_w (0,0);
}


/*
 * This is Lunar Lander's Inputport 0.
 */
READ8_HANDLER( llander_IN0_r )
{
	int res;

	res = readinputport(0);

	if (avgdvg_done())
		res |= 0x01;
	if (activecpu_gettotalcycles() & 0x100)
		res |= 0x40;

	return res;
}
