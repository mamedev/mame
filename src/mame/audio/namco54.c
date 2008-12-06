/***************************************************************************

Namco 54XX

This custom chip is a Fujitsu MB8844 MCU programmed to act as a noise
generator. It is used for explosions, the shoot sound in Bosconian, and the
tire screech sound in Pole Position.

CMD = command from main CPU
OUTn = sound outputs (3 channels)

The chip reads the command when the /IRQ is pulled down.

      +------+
 EXTAL|1   28|Vcc
  XTAL|2   27|CMD7
/RESET|3   26|CMD6
OUT0.0|4   25|CMD5
OUT0.1|5   24|CMD4
OUT0.2|6   23|/IRQ
OUT0.3|7   22|n.c. [1]
OUT1.0|8   21|n.c. [1]
OUT1.1|9   20|OUT2.3
OUT1.2|10  19|OUT2.2
OUT1.3|11  18|OUT2.1
  CMD0|12  17|OUT2.0
  CMD1|13  16|CMD3
   GND|14  15|CMD2
      +------+


[1] The RNG that drives the type A output is output on pin 21, and the one that
drives the type B output is output on pin 22, but those pins are not connected
on the board.


The command format is very simple:

0x: nop
1x: play sound type A
2x: play sound type B
3x: set parameters (type A) (followed by 4 bytes)
4x: set parameters (type B) (followed by 4 bytes)
5x: play sound type C
6x: set parameters (type C) (followed by 5 bytes)
7x: set volume for sound type C to x
8x-Fx: nop

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "namco54.h"
#include "cpu/mb88xx/mb88xx.h"


static UINT8 latched_cmd;

static TIMER_CALLBACK( namco_54xx_latch_callback )
{
	latched_cmd = param;
}

static READ8_HANDLER( namco_54xx_K_r )
{
	return latched_cmd >> 4;
}

static READ8_HANDLER( namco_54xx_R0_r )
{
	return latched_cmd & 0x0f;
}


static WRITE8_HANDLER( namco_54xx_O_w )
{
	UINT8 out = (data & 0x0f);
	if (data & 0x10)
		discrete_sound_w(space, NAMCO_54XX_1_DATA, out);
	else
		discrete_sound_w(space, NAMCO_54XX_0_DATA, out);
}

static WRITE8_HANDLER( namco_54xx_R1_w )
{
	UINT8 out = (data & 0x0f);

	discrete_sound_w(space, NAMCO_54XX_2_DATA, out);
}



ADDRESS_MAP_START( namco_54xx_map_program, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x000, 0x3ff) AM_ROM
ADDRESS_MAP_END

ADDRESS_MAP_START( namco_54xx_map_data, ADDRESS_SPACE_DATA, 8 )
	AM_RANGE(0x00, 0x3f) AM_RAM
ADDRESS_MAP_END

ADDRESS_MAP_START( namco_54xx_map_io, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(MB88_PORTK,  MB88_PORTK)  AM_READ(namco_54xx_K_r)
	AM_RANGE(MB88_PORTO,  MB88_PORTO)  AM_WRITE(namco_54xx_O_w)
	AM_RANGE(MB88_PORTR0, MB88_PORTR0) AM_READ(namco_54xx_R0_r)
	AM_RANGE(MB88_PORTR1, MB88_PORTR1) AM_WRITE(namco_54xx_R1_w)
	AM_RANGE(MB88_PORTR2, MB88_PORTR2) AM_NOP
ADDRESS_MAP_END



static TIMER_CALLBACK( namco_54xx_irq_clear )
{
	const device_config *device = ptr;
	cpu_set_input_line(device, 0, CLEAR_LINE);
}

void namco_54xx_write(UINT8 data)
{
	const device_config *device = cputag_get_cpu(Machine, CPUTAG_54XX);

	if (device == NULL)
		return;

	timer_call_after_resynch(Machine, NULL, data, namco_54xx_latch_callback);

	cpu_set_input_line(device, 0, ASSERT_LINE);

	// The execution time of one instruction is ~4us, so we must make sure to
	// give the cpu time to poll the /IRQ input before we clear it.
	// The input clock to the 06XX interface chip is 64H, that is
	// 18432000/6/64 = 48kHz, so it makes sense for the irq line to be
	// asserted for one clock cycle ~= 21us.
	timer_set(Machine, ATTOTIME_IN_USEC(21), (void *)device, 0, namco_54xx_irq_clear);
}
