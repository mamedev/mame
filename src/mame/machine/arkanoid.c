/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "driver.h"
#include "arkanoid.h"


/* To log specific reads and writes of the bootlegs */
#define ARKANOID_BOOTLEG_VERBOSE 1


READ8_HANDLER( arkanoid_Z80_mcu_r )
{
	arkanoid_state *state = (arkanoid_state *)space->machine->driver_data;

	/* return the last value the 68705 wrote, and mark that we've read it */
	state->m68705write = 0;
	return state->toz80;
}

static TIMER_CALLBACK( test )
{
	arkanoid_state *state = (arkanoid_state *)machine->driver_data;

	state->z80write = 1;
	state->fromz80 = param;
}

WRITE8_HANDLER( arkanoid_Z80_mcu_w )
{
	timer_call_after_resynch(space->machine, NULL, data, test);
	/* boost the interleave for a few usecs to make sure it is read successfully */
	cpuexec_boost_interleave(space->machine, attotime_zero, ATTOTIME_IN_USEC(10));
}

READ8_HANDLER( arkanoid_68705_port_a_r )
{
	arkanoid_state *state = (arkanoid_state *)space->machine->driver_data;

	return (state->port_a_out & state->ddr_a) | (state->port_a_in & ~state->ddr_a);
}

WRITE8_HANDLER( arkanoid_68705_port_a_w )
{
	arkanoid_state *state = (arkanoid_state *)space->machine->driver_data;

	state->port_a_out = data;
}

WRITE8_HANDLER( arkanoid_68705_ddr_a_w )
{
	arkanoid_state *state = (arkanoid_state *)space->machine->driver_data;

	state->ddr_a = data;
}


READ8_HANDLER( arkanoid_68705_port_c_r )
{
	arkanoid_state *state = (arkanoid_state *)space->machine->driver_data;
	int res = 0;

	/* bit 0 is high on a write strobe; clear it once we've detected it */
	if (state->z80write)
		res |= 0x01;

	/* bit 1 is high if the previous write has been read */
	if (!state->m68705write)
		res |= 0x02;

	return (state->port_c_out & state->ddr_c) | (res & ~state->ddr_c);
}

WRITE8_HANDLER( arkanoid_68705_port_c_w )
{
	arkanoid_state *state = (arkanoid_state *)space->machine->driver_data;

	if ((state->ddr_c & 0x04) && (~data & 0x04) && (state->port_c_out & 0x04))
	{
		/* return the last value the Z80 wrote */
		state->z80write = 0;
		state->port_a_in = state->fromz80;
	}
	if ((state->ddr_c & 0x08) && (~data & 0x08) && (state->port_c_out & 0x08))
	{
		/* a write from the 68705 to the Z80; remember its value */
		state->m68705write = 1;
		state->toz80 = state->port_a_out;
	}

	state->port_c_out = data;
}

WRITE8_HANDLER( arkanoid_68705_ddr_c_w )
{
	arkanoid_state *state = (arkanoid_state *)space->machine->driver_data;

	state->ddr_c = data;
}

CUSTOM_INPUT( arkanoid_68705_input_r )
{
	arkanoid_state *state = (arkanoid_state *)field->port->machine->driver_data;
	int res = 0;

	/* bit 0x40 of comes from the sticky bit */
	if (!state->z80write)
		res |= 0x01;

	/* bit 0x80 comes from a write latch */
	if (!state->m68705write)
		res |= 0x02;

	return res;
}

CUSTOM_INPUT( arkanoid_input_mux )
{
	arkanoid_state *state = (arkanoid_state *)field->port->machine->driver_data;
	const char *tag1 = (const char *)param;
	const char *tag2 = tag1 + strlen(tag1) + 1;
	return input_port_read(field->port->machine, (state->paddle_select == 0) ? tag1 : tag2);
}

/*

Bootlegs stuff

The bootlegs simulate the missing MCU behaviour with writes to 0xd018 and reads value back from 0xf002.
Fortunately, 'arkangc', 'arkangc2', 'block2', 'arkbloc2' and 'arkblock' has patched code not to bother with that.
So I've fixed 'arkbl3' and 'paddle2' to return the expected values (code is strongly similar).
However, 'block2' is the only bootleg that writes some values to 0xd018 and reads them back from 0xf000.

Some bootlegs also test some bits from 0xd008 after reading the paddle value at 0xd018.
Their effect is completely unknown but I need to set some bits to 1 so the games are playable :

  - 'arkangc'  : NO read from 0xd008 !
  - 'arkangc2' :
       * bit 1 must be set to 1 or you enter sort of endless "demo mode" when you start :
           . you can't select your starting level (it always starts at level 1)
           . you can't control the paddle (it automoves by following the ball)
           . you can use the "fire" button (the game never shoots)
           . you are awarded points as in a normal game
           . sounds are played
  - 'block2' :
       * bit 1 must be set to 1 or you enter sort of endless "demo mode" when you start :
           . you can't control the paddle (it automoves by following the ball)
           . you can use the "fire" button (the game never shoots)
           . you are awarded points as in a normal game
           . sounds are played
  - 'arkblock' : NO read from 0xd008 !
  - 'arkbloc2' :
       * bit 5 must sometimes be set to 1 or you can't reach right side of the screen
         nor select all levels at the begining of the game
  - 'arkgcbl' :
       * bit 1 must be set to 1 or you enter sort of endless "demo mode" when you start :
           . you can't select your starting level (it always starts at level 1)
           . you can't control the paddle (it automoves by following the ball)
           . you can use the "fire" button (the game never shoots)
           . you are awarded points as in a normal game
           . sounds are played
       * bit 5 must sometimes be set to 1 or you can't reach right side of the screen
         nor select all levels at the begining of the game
  - 'paddle2' :
       * bits 0 and 1 must be set to 1 or the paddle goes up   (joystick issue ?)
       * bits 2 and 3 must be set to 1 or the paddle goes down (joystick issue ?)
       * bit 5 must sometimes be set to 1 or you can't reach right side of the screen
         nor select all levels at the begining of the game


TO DO (2006.09.12) :

  - understand reads from 0xd008 (even if the games are playable)
  - try to document writes to 0xd018 with unknown effect

*/


#define LOG_F000_R if (ARKANOID_BOOTLEG_VERBOSE) logerror("%04x: arkanoid_bootleg_f000_r - cmd = %02x - val = %02x\n", cpu_get_pc(space->cpu), state->bootleg_cmd, arkanoid_bootleg_val);
#define LOG_F002_R if (ARKANOID_BOOTLEG_VERBOSE) logerror("%04x: arkanoid_bootleg_f002_r - cmd = %02x - val = %02x\n", cpu_get_pc(space->cpu), state->bootleg_cmd, arkanoid_bootleg_val);
#define LOG_D018_W if (ARKANOID_BOOTLEG_VERBOSE) logerror("%04x: arkanoid_bootleg_d018_w - data = %02x - cmd = %02x\n", cpu_get_pc(space->cpu), data, state->bootleg_cmd);
#define LOG_D008_R if (ARKANOID_BOOTLEG_VERBOSE) logerror("%04x: arkanoid_bootleg_d008_r - val = %02x\n", cpu_get_pc(space->cpu), arkanoid_bootleg_d008_val);


/* Kludge for some bootlegs that read this address */
READ8_HANDLER( arkanoid_bootleg_f000_r )
{
	arkanoid_state *state = (arkanoid_state *)space->machine->driver_data;
	UINT8 arkanoid_bootleg_val = 0x00;

	switch (state->bootleg_id)
	{
		case ARKANGC:	/* There are no reads from 0xf000 in these bootlegs */
		case ARKBLOCK:
		case ARKANGC2:
		case ARKBLOC2:
		case ARKGCBL:
		case PADDLE2:
			switch (state->bootleg_cmd)
			{
				default:
					break;
			}
			LOG_F000_R
			break;
		case BLOCK2:
			switch (state->bootleg_cmd)
			{
				case 0x05:  /* Check 1 */
					arkanoid_bootleg_val = 0x05;
					break;
				case 0x0a:  /* Check 2 */
					arkanoid_bootleg_val = 0x0a;
					break;
				default:
					break;
			}
			LOG_F000_R
			break;
		default:
			logerror("%04x: arkanoid_bootleg_f000_r - cmd = %02x - unknown bootleg !\n", cpu_get_pc(space->cpu), state->bootleg_cmd);
			break;
	}

	return arkanoid_bootleg_val;
}

/* Kludge for some bootlegs that read this address */
READ8_HANDLER( arkanoid_bootleg_f002_r )
{
	arkanoid_state *state = (arkanoid_state *)space->machine->driver_data;
	UINT8 arkanoid_bootleg_val = 0x00;

	switch (state->bootleg_id)
	{
		case ARKANGC:
		case ARKBLOCK:
			switch (state->bootleg_cmd)
			{
				default:
					break;
			}
			LOG_F002_R
			break;
		case ARKANGC2:  /* There are no reads from 0xf002 in these bootlegs */
		case BLOCK2:
			switch (state->bootleg_cmd)
			{
				default:
					break;
			}
			LOG_F002_R
			break;
		case ARKBLOC2:
			switch (state->bootleg_cmd)
			{
				default:
					break;
			}
			LOG_F002_R
			break;
		case ARKGCBL:
			switch (state->bootleg_cmd)
			{
				case 0x8a:  /* Current level (fixed routine) */
					arkanoid_bootleg_val = 0xa5;
					break;
				case 0xff:  /* Avoid "BAD HARDWARE    " message (fixed routine) */
					arkanoid_bootleg_val = 0xe2;
					break;
				default:
					break;
			}
			LOG_F002_R
			break;
		case PADDLE2:
			switch (state->bootleg_cmd)
			{
				case 0x24:  /* Avoid bad jump to 0x0066 */
					arkanoid_bootleg_val = 0x9b;
					break;
				case 0x36:  /* Avoid "BAD HARDWARE    " message */
					arkanoid_bootleg_val = 0x2d;
					break;
				case 0x38:  /* Start of levels table (fixed offset) */
					arkanoid_bootleg_val = 0xf3;
					break;
				case 0x8a:  /* Current level (fixed routine) */
					arkanoid_bootleg_val = 0xa5;
					break;
				case 0xc3:  /* Avoid bad jump to 0xf000 */
					arkanoid_bootleg_val = 0x1d;
					break;
				case 0xe3:  /* Number of bricks left (fixed offset) */
					arkanoid_bootleg_val = 0x61;
					break;
				case 0xf7:  /* Avoid "U69" message */
					arkanoid_bootleg_val = 0x00;
					break;
				case 0xff:  /* Avoid "BAD HARDWARE    " message (fixed routine) */
					arkanoid_bootleg_val = 0xe2;
					break;
				default:
					break;
			}
			LOG_F002_R
			break;
		default:
			logerror("%04x: arkanoid_bootleg_f002_r - cmd = %02x - unknown bootleg !\n", cpu_get_pc(space->cpu), state->bootleg_cmd);
			break;
	}

	return arkanoid_bootleg_val;
}

/* Kludge for some bootlegs that write this address */
WRITE8_HANDLER( arkanoid_bootleg_d018_w )
{
	arkanoid_state *state = (arkanoid_state *)space->machine->driver_data;
	state->bootleg_cmd = 0x00;

	switch (state->bootleg_id)
	{
		case ARKANGC:
		case ARKBLOCK:
			switch (data)
			{
				case 0x36:  /* unneeded value : no call 0x2050, unused A and overwritten HL (0x0313 -> 0x0340) */
					if (cpu_get_pc(space->cpu) == 0x7c47)
						state->bootleg_cmd = 0x00;
					break;
				case 0x38:  /* unneeded value : no call 0x2050, unused A and fixed HL (0x7bd5) */
					if (cpu_get_pc(space->cpu) == 0x7b87)
						state->bootleg_cmd = 0x00;
					break;
				case 0x8a:  /* unneeded value : no call 0x2050, unused A and overwritten HL (0x7b77 -> 0x7c1c) */
					if (cpu_get_pc(space->cpu) == 0x9661)
						state->bootleg_cmd = 0x00;
					break;
				case 0xe3:  /* unneeded value : call 0x2050 but fixed A (0x00) and fixed HL (0xed83) */
					if (cpu_get_pc(space->cpu) == 0x67e3)
						state->bootleg_cmd = 0x00;
					break;
				case 0xf7:  /* unneeded value : 3 * 'NOP' at 0x034f + 2 * 'NOP' at 0x35b */
					if (cpu_get_pc(space->cpu) == 0x0349)
						state->bootleg_cmd = 0x00;
					break;
				case 0xff:  /* unneeded value : no call 0x2050, unused A and overwritten HL (0x7c4f -> 0x7d31) */
					if (cpu_get_pc(space->cpu) == 0x9670)
						state->bootleg_cmd = 0x00;
					break;
				default:
					state->bootleg_cmd = 0x00;
					break;
			}
			LOG_D018_W
			break;
		case ARKANGC2:
			switch (data)
			{
				case 0x36:  /* unneeded value : call 0x2050 but fixed A (0x2d) */
					if (cpu_get_pc(space->cpu) == 0x7c4c)
						state->bootleg_cmd = 0x00;
					break;
				case 0x38:  /* unneeded value : call 0x2050 but fixed A (0xf3) */
					if (cpu_get_pc(space->cpu) == 0x7b87)
						state->bootleg_cmd = 0x00;
					break;
				case 0x88:  /* unneeded value : no read back */
					if (cpu_get_pc(space->cpu) == 0x67e3)
						state->bootleg_cmd = 0x00;
					if (cpu_get_pc(space->cpu) == 0x7c47)
						state->bootleg_cmd = 0x00;
					break;
				case 0x89:  /* unneeded value : no read back */
					if (cpu_get_pc(space->cpu) == 0x67e5)
						state->bootleg_cmd = 0x00;
					break;
				case 0x8a:  /* unneeded value : call 0x2050 but fixed A (0xa5) */
					if (cpu_get_pc(space->cpu) == 0x9661)
						state->bootleg_cmd = 0x00;
					break;
				case 0xc0:  /* unneeded value : no read back */
					if (cpu_get_pc(space->cpu) == 0x67e7)
						state->bootleg_cmd = 0x00;
					break;
				case 0xe3:  /* unneeded value : call 0x2050 but fixed A (0x61) */
					if (cpu_get_pc(space->cpu) == 0x67e9)
						state->bootleg_cmd = 0x00;
					break;
				case 0xff:  /* unneeded value : call 0x2050 but fixed A (0xe2) */
					if (cpu_get_pc(space->cpu) == 0x9670)
						state->bootleg_cmd = 0x00;
					break;
				default:
					state->bootleg_cmd = 0x00;
					break;
			}
			LOG_D018_W
			break;
		case BLOCK2:
			switch (data)
			{
				case 0x05:  /* Check 1 */
					if (cpu_get_pc(space->cpu) == 0x0363)
						state->bootleg_cmd = 0x05;
					break;
				case 0x0a:  /* Check 2 */
					if (cpu_get_pc(space->cpu) == 0x0372)
						state->bootleg_cmd = 0x0a;
					break;
				default:
					state->bootleg_cmd = 0x00;
					break;
			}
			LOG_D018_W
			break;
		case ARKBLOC2:
			switch (data)
			{
				case 0x36:  /* unneeded value : call 0x2050 but fixed A (0x2d) */
					if (cpu_get_pc(space->cpu) == 0x7c4c)
						state->bootleg_cmd = 0x00;
					break;
				case 0x38:  /* unneeded value : call 0x2050 but fixed A (0xf3) */
					if (cpu_get_pc(space->cpu) == 0x7b87)
						state->bootleg_cmd = 0x00;
					break;
				case 0x88:  /* unneeded value : no read back */
					if (cpu_get_pc(space->cpu) == 0x67e3)
						state->bootleg_cmd = 0x00;
					if (cpu_get_pc(space->cpu) == 0x7c47)
						state->bootleg_cmd = 0x00;
					break;
				case 0x89:  /* unneeded value : no read back */
					if (cpu_get_pc(space->cpu) == 0x67e5)
						state->bootleg_cmd = 0x00;
					break;
				case 0x8a:  /* unneeded value : call 0x2050 but unused HL and fixed DE (0x7c1c) */
					if (cpu_get_pc(space->cpu) == 0x9661)
						state->bootleg_cmd = 0x00;
					break;
				case 0xc0:  /* unneeded value : no read back */
					if (cpu_get_pc(space->cpu) == 0x67e7)
						state->bootleg_cmd = 0x00;
					break;
				case 0xe3:  /* unneeded value : call 0x2050 but fixed A (0x61) */
					if (cpu_get_pc(space->cpu) == 0x67e9)
						state->bootleg_cmd = 0x00;
					break;
				case 0xf7:  /* unneeded value : call 0x2050 but never called (check code at 0x0340) */
					if (cpu_get_pc(space->cpu) == 0x0349)
						state->bootleg_cmd = 0x00;
					break;
				case 0xff:  /* unneeded value : no call 0x2050, unused A and fixed HL (0x7d31) */
					if (cpu_get_pc(space->cpu) == 0x9670)
						state->bootleg_cmd = 0x00;
					break;
				default:
					state->bootleg_cmd = 0x00;
					break;
			}
			LOG_D018_W
			break;
		case ARKGCBL:
			switch (data)
			{
				case 0x36:  /* unneeded value : call 0x2050 but fixed A (0x2d) */
					if (cpu_get_pc(space->cpu) == 0x7c4c)
						state->bootleg_cmd = 0x00;
					break;
				case 0x38:  /* unneeded value : call 0x2050 but fixed A (0xf3) */
					if (cpu_get_pc(space->cpu) == 0x7b87)
						state->bootleg_cmd = 0x00;
					break;
				case 0x88:  /* unneeded value : no read back */
					if (cpu_get_pc(space->cpu) == 0x67e3)
						state->bootleg_cmd = 0x00;
					if (cpu_get_pc(space->cpu) == 0x7c47)
						state->bootleg_cmd = 0x00;
				case 0x89:  /* unneeded value : no read back */
					if (cpu_get_pc(space->cpu) == 0x67e5)
						state->bootleg_cmd = 0x00;
					break;
				case 0x8a:  /* call 0x2050 with A read from 0xf002 and wrong HL */
					if (cpu_get_pc(space->cpu) == 0x9661)
						state->bootleg_cmd = data;
					break;
				case 0xc0:  /* unneeded value : no read back */
					if (cpu_get_pc(space->cpu) == 0x67e7)
						state->bootleg_cmd = 0x00;
					break;
				case 0xe3:  /* unneeded value : call 0x2050 but fixed A (0x61) */
					if (cpu_get_pc(space->cpu) == 0x67e9)
						state->bootleg_cmd = 0x00;
					break;
				case 0xf7:  /* unneeded value : 3 * 'NOP' at 0x034f + 'JR NZ,$035D' at 0x35b */
					if (cpu_get_pc(space->cpu) == 0x0349)
						state->bootleg_cmd = 0x00;
					break;
				case 0xff:  /* call 0x2050 with A read from 0xf002 and wrong HL */
					if (cpu_get_pc(space->cpu) == 0x9670)
						state->bootleg_cmd = data;
					break;
				default:
					state->bootleg_cmd = 0x00;
					break;
			}
			LOG_D018_W
			break;
		case PADDLE2:
			switch (data)
			{
				case 0x24:  /* A read from 0xf002 (expected to be 0x9b) */
					if (cpu_get_pc(space->cpu) == 0xbd7a)
						state->bootleg_cmd = data;
					break;
				case 0x36:  /* call 0x2050 with A read from 0xf002 and wrong HL */
					if (cpu_get_pc(space->cpu) == 0x7c4c)
						state->bootleg_cmd = data;
					break;
				case 0x38:  /* call 0x2050 with A read from 0xf002 and wrong HL */
					if (cpu_get_pc(space->cpu) == 0x7b87)
						state->bootleg_cmd = data;
					break;
				case 0x88:  /* unneeded value : no read back */
					if (cpu_get_pc(space->cpu) == 0x67e3)
						state->bootleg_cmd = 0x00;
					if (cpu_get_pc(space->cpu) == 0x7c47)
						state->bootleg_cmd = 0x00;
				case 0x89:  /* unneeded value : no read back */
					if (cpu_get_pc(space->cpu) == 0x67e5)
						state->bootleg_cmd = 0x00;
					break;
				case 0x8a:  /* call 0x2050 with A read from 0xf002 and wrong HL */
					if (cpu_get_pc(space->cpu) == 0x9661)
						state->bootleg_cmd = data;
					break;
				case 0xc0:  /* unneeded value : no read back */
					if (cpu_get_pc(space->cpu) == 0x67e7)
						state->bootleg_cmd = 0x00;
					break;
				case 0xc3:  /* A read from 0xf002 (expected to be 0x1d) */
					if (cpu_get_pc(space->cpu) == 0xbd8a)
						state->bootleg_cmd = data;
					break;
				case 0xe3:  /* call 0x2050 with A read from 0xf002 and wrong HL */
					if (cpu_get_pc(space->cpu) == 0x67e9)
						state->bootleg_cmd = data;
					break;
				case 0xf7:  /* call 0x2050 with A read from 0xf002 and wrong HL */
					if (cpu_get_pc(space->cpu) == 0x0349)
						state->bootleg_cmd = data;
					break;
				case 0xff:  /* call 0x2050 with A read from 0xf002 and wrong HL */
					if (cpu_get_pc(space->cpu) == 0x9670)
						state->bootleg_cmd = data;
					break;
				default:
					state->bootleg_cmd = 0x00;
					break;
			}
			LOG_D018_W
			break;

		default:
			logerror("%04x: arkanoid_bootleg_d018_w - data = %02x - unknown bootleg !\n", cpu_get_pc(space->cpu), data);
			break;
	}
}

#ifdef UNUSED_CODE
READ8_HANDLER( block2_bootleg_f000_r )
{
	arkanoid_state *state = (arkanoid_state *)space->machine->driver_data;
	return state->bootleg_cmd;
}
#endif

/* Kludge for some bootlegs that read this address */
READ8_HANDLER( arkanoid_bootleg_d008_r )
{
	arkanoid_state *state = (arkanoid_state *)space->machine->driver_data;
	UINT8 arkanoid_bootleg_d008_bit[8];
	UINT8 arkanoid_bootleg_d008_val;
	UINT8 arkanoid_paddle_value = input_port_read(space->machine, "MUX");
	int b;

	arkanoid_bootleg_d008_bit[4] = arkanoid_bootleg_d008_bit[6] = arkanoid_bootleg_d008_bit[7] = 0;  /* untested bits */

	switch (state->bootleg_id)
	{
		case ARKANGC:
		case ARKBLOCK:
			arkanoid_bootleg_d008_bit[0] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[1] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[2] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[3] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[5] = 0;  /* untested bit */
			break;
		case ARKANGC2:
		case BLOCK2:
			arkanoid_bootleg_d008_bit[0] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[1] = 1;  /* check code at 0x0cad */
			arkanoid_bootleg_d008_bit[2] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[3] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[5] = 0;  /* untested bit */
			break;
		case ARKBLOC2:
			arkanoid_bootleg_d008_bit[0] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[1] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[2] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[3] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[5] = (arkanoid_paddle_value < 0x40);  /* check code at 0x96b0 */
			break;
		case ARKGCBL:
			arkanoid_bootleg_d008_bit[0] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[1] = 1;  /* check code at 0x0cad */
			arkanoid_bootleg_d008_bit[2] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[3] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[5] = (arkanoid_paddle_value < 0x40);  /* check code at 0x96b0 */
			break;
		case PADDLE2:
			arkanoid_bootleg_d008_bit[0] = 1;  /* check code at 0x7d65 */
			arkanoid_bootleg_d008_bit[1] = 1;  /* check code at 0x7d65 */
			arkanoid_bootleg_d008_bit[2] = 1;  /* check code at 0x7d65 */
			arkanoid_bootleg_d008_bit[3] = 1;  /* check code at 0x7d65 */
			arkanoid_bootleg_d008_bit[5] = (arkanoid_paddle_value < 0x40);  /* check code at 0x96b0 */
			break;
		default:
			arkanoid_bootleg_d008_bit[0] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[1] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[2] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[3] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[5] = 0;  /* untested bit */
			logerror("%04x: arkanoid_bootleg_d008_r - unknown bootleg !\n",cpu_get_pc(space->cpu));
			break;
	}

	arkanoid_bootleg_d008_val = 0;
	for (b = 0; b < 8; b++)
		arkanoid_bootleg_d008_val |= (arkanoid_bootleg_d008_bit[b] << b);

	LOG_D008_R

	return arkanoid_bootleg_d008_val;
}
