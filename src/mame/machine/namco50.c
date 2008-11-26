/***************************************************************************

Namco 50XX

This custom chip is a Fujitsu MB8842 MCU programmed to act as a protection
device. It keeps track of the players scores, and checks if a high score has
been obtained or bonus lives should be awarded. The main CPU has a range of
commands to increment/decrement the score by various fixed amounts.

The device is used to its full potential only by Bosconian; Xevious uses it
too, but only to do a protection check on startup.

CMD = command from main CPU
ANS = answer to main CPU

The chip reads/writes the I/O ports when the /IRQ is pulled down. Pin 21
determines whether a read or write should happen (1=R, 0=W).

      +------+
 EXTAL|1   28|Vcc
  XTAL|2   27|CMD7
/RESET|3   26|CMD6
  ANS0|4   25|CMD5
  ANS1|5   24|CMD4
  ANS2|6   23|/IRQ
  ANS3|7   22|n.c.
  ANS4|8   21|R/W
  ANS5|9   20|n.c.
  ANS6|10  19|n.c.
  ANS7|11  18|n.c.
  CMD0|12  17|n.c.
  CMD1|13  16|CMD3
   GND|14  15|CMD2
      +------+



Commands:

0x = nop

1x = reset scores

2x = set first bonus score (followed by 3 bytes)

3x = set interval bonus score (followed by 3 bytes)

4x = ?

5x = set high score (followed by 3 bytes)

60 = switch to player 1
68 = switch to player 2

70 = switch to increment score
7x = switch to decrement score

score increments/decrements:

80 =    5
81 =   10
82 =   15
83 =   20
84 =   25
85 =   30
86 =   40
87 =   50
88 =   60
89 =   70
8A =   80
8B =   90
8C =  100
8D =  200
8E =  300
8F =  500

9x same as 8x but *10
Ax same as 8x but *100

B0h =   10
B1h =   20
B2h =   30
B3h =   40
B4h =   50
B5h =   60
B6h =   80
B7h =  100
B8h =  120
B9h =  140
BAh =  160
BBh =  180
BCh =  200
BDh =  400
BEh =  600
BFh = 1000

Cx same as Bx but *10
Dx same as Bx but *100

E0 =   15
E1 =   30
E2 =   45
E3 =   60
E4 =   75
E5 =   90
E6 =  120
E7 =  150
E8 =  180
E9 =  210
EA =  240
EB =  270
EC =  300
ED =  600
EE =  900
EF = 1500

Fx same as Ex but *10


When reading, the score for the currently selected player is returned. The first
byte also contains flags.

Byte 0: BCD Score (fs------) and flags
Byte 1: BCD Score (--ss----)
Byte 2: BCD Score (----ss--)
Byte 3: BCD Score (------ss)

Flags: 80=high score, 40=first bonus, 20=interval bonus, 10=?

***************************************************************************/

#include "driver.h"
#include "namco50.h"
#include "cpu/mb88xx/mb88xx.h"


static UINT8 latched_cmd[2];
static UINT8 latched_rw[2];
static UINT8 portO[2];


static TIMER_CALLBACK( namco_50xx_latch_callback )
{
	latched_cmd[0] = param;
	latched_rw[0] = 1;
}

static TIMER_CALLBACK( namco_50xx_2_latch_callback )
{
	latched_cmd[1] = param;
	latched_rw[1] = 1;
}


static TIMER_CALLBACK( namco_50xx_readrequest_callback )
{
	latched_rw[0] = 0;
}

static TIMER_CALLBACK( namco_50xx_2_readrequest_callback )
{
	latched_rw[1] = 0;
}


static READ8_HANDLER( namco_50xx_K_r )
{
	return latched_cmd[0] >> 4;
}

static READ8_HANDLER( namco_50xx_R0_r )
{
	return latched_cmd[0] & 0x0f;
}

static READ8_HANDLER( namco_50xx_R2_r )
{
	return latched_rw[0] & 1;
}



static READ8_HANDLER( namco_50xx_2_K_r )
{
	return latched_cmd[1] >> 4;
}

static READ8_HANDLER( namco_50xx_2_R0_r )
{
	return latched_cmd[1] & 0x0f;
}

static READ8_HANDLER( namco_50xx_2_R2_r )
{
	return latched_rw[1] & 1;
}


static WRITE8_HANDLER( namco_50xx_O_w )
{
	UINT8 out = (data & 0x0f);
	if (data & 0x10)
		portO[0] = (portO[0] & 0x0f) | (out << 4);
	else
		portO[0] = (portO[0] & 0xf0) | (out);
}



static WRITE8_HANDLER( namco_50xx_2_O_w )
{
	UINT8 out = (data & 0x0f);
	if (data & 0x10)
		portO[1] = (portO[1] & 0x0f) | (out << 4);
	else
		portO[1] = (portO[1] & 0xf0) | (out);
}



ADDRESS_MAP_START( namco_50xx_map_program, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x000, 0x7ff) AM_ROM
ADDRESS_MAP_END

ADDRESS_MAP_START( namco_50xx_map_data, ADDRESS_SPACE_DATA, 8 )
	AM_RANGE(0x00, 0x7f) AM_RAM
ADDRESS_MAP_END

ADDRESS_MAP_START( namco_50xx_map_io, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(MB88_PORTK,  MB88_PORTK)  AM_READ(namco_50xx_K_r)
	AM_RANGE(MB88_PORTO,  MB88_PORTO)  AM_WRITE(namco_50xx_O_w)
	AM_RANGE(MB88_PORTR0, MB88_PORTR0) AM_READ(namco_50xx_R0_r)
	AM_RANGE(MB88_PORTR2, MB88_PORTR2) AM_READ(namco_50xx_R2_r)
ADDRESS_MAP_END



ADDRESS_MAP_START( namco_50xx_2_map_program, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x000, 0x7ff) AM_ROM
ADDRESS_MAP_END

ADDRESS_MAP_START( namco_50xx_2_map_data, ADDRESS_SPACE_DATA, 8 )
	AM_RANGE(0x00, 0x7f) AM_RAM
ADDRESS_MAP_END

ADDRESS_MAP_START( namco_50xx_2_map_io, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(MB88_PORTK,  MB88_PORTK)  AM_READ(namco_50xx_2_K_r)
	AM_RANGE(MB88_PORTO,  MB88_PORTO)  AM_WRITE(namco_50xx_2_O_w)
	AM_RANGE(MB88_PORTR0, MB88_PORTR0) AM_READ(namco_50xx_2_R0_r)
	AM_RANGE(MB88_PORTR2, MB88_PORTR2) AM_READ(namco_50xx_2_R2_r)
ADDRESS_MAP_END



static TIMER_CALLBACK( namco_50xx_irq_clear )
{
	cpu_set_input_line(machine->cpu[param], 0, CLEAR_LINE);
}

static void namco_50xx_irq_set(running_machine *machine, int cpunum)
{
	cpu_set_input_line(machine->cpu[cpunum], 0, ASSERT_LINE);

	// The execution time of one instruction is ~4us, so we must make sure to
	// give the cpu time to poll the /IRQ input before we clear it.
	// The input clock to the 06XX interface chip is 64H, that is
	// 18432000/6/64 = 48kHz, so it makes sense for the irq line to be
	// asserted for one clock cycle ~= 21us.
	timer_set(machine, ATTOTIME_IN_USEC(21), NULL, cpunum, namco_50xx_irq_clear);
}

void namco_50xx_write(running_machine *machine, UINT8 data)
{
	int cpunum = mame_find_cpu_index(machine, CPUTAG_50XX);

	if (cpunum == -1)
		return;

	timer_call_after_resynch(machine, NULL, data, namco_50xx_latch_callback);

	namco_50xx_irq_set(machine, cpunum);
}

void namco_50xx_2_write(running_machine *machine, UINT8 data)
{
	int cpunum = mame_find_cpu_index(machine, CPUTAG_50XX_2);

	if (cpunum == -1)
		return;

	timer_call_after_resynch(machine, NULL, data, namco_50xx_2_latch_callback);

	namco_50xx_irq_set(machine, cpunum);
}


void namco_50xx_read_request(running_machine *machine)
{
	int cpunum = mame_find_cpu_index(machine, CPUTAG_50XX);

	if (cpunum == -1)
		return;

	timer_call_after_resynch(machine, NULL, 0, namco_50xx_readrequest_callback);

	namco_50xx_irq_set(machine, cpunum);
}

void namco_50xx_2_read_request(running_machine *machine)
{
	int cpunum = mame_find_cpu_index(machine, CPUTAG_50XX_2);

	if (cpunum == -1)
		return;

	timer_call_after_resynch(machine, NULL, 0, namco_50xx_2_readrequest_callback);

	namco_50xx_irq_set(machine, cpunum);
}


UINT8 namco_50xx_read(running_machine *machine)
{
	UINT8 res = portO[0];

	namco_50xx_read_request(machine);

	return res;
}

UINT8 namco_50xx_2_read(running_machine *machine)
{
	UINT8 res = portO[1];

	namco_50xx_2_read_request(machine);

	return res;
}
