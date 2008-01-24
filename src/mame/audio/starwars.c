/***************************************************************************

    Atari Star Wars hardware

    This file is Copyright Steve Baines.
    Modified by Frank Palazzolo for sound support

***************************************************************************/

#include "driver.h"
#include "cpu/m6809/m6809.h"
#include "sound/5220intf.h"
#include "includes/starwars.h"

/* Sound commands from the main CPU are stored in a single byte */
/* register.  The main CPU then interrupts the Sound CPU.       */

static int port_A = 0;   /* 6532 port A data register */

                         /* Configured as follows:           */
                         /* d7 (in)  Main Ready Flag         */
                         /* d6 (in)  Sound Ready Flag        */
                         /* d5 (out) Mute Speech             */
                         /* d4 (in)  Not Sound Self Test     */
                         /* d3 (out) Hold Main CPU in Reset? */
                         /*          + enable delay circuit? */
                         /* d2 (in)  TMS5220 Not Ready       */
                         /* d1 (out) TMS5220 Not Read        */
                         /* d0 (out) TMS5220 Not Write       */

static int port_B = 0;     /* 6532 port B data register        */
                           /* (interfaces to TMS5220 data bus) */

static int irq_flag = 0;   /* 6532 interrupt flag register */

static int port_A_ddr = 0; /* 6532 Data Direction Register A */
static int port_B_ddr = 0; /* 6532 Data Direction Register B */
                           /* for each bit, 0 = input, 1 = output */

static int PA7_irq = 0;  /* IRQ-on-write flag (sound CPU) */

static int sound_data; /* data for the sound cpu */
static int main_data;   /* data for the main  cpu */



/*************************************
 *
 *  Sound interrupt generation
 *
 *************************************/

static TIMER_CALLBACK( snd_interrupt )
{
	irq_flag |= 0x80; /* set timer interrupt flag */
	cpunum_set_input_line(machine, 1, M6809_IRQ_LINE, ASSERT_LINE);
}



/*************************************
 *
 *  M6532 I/O read
 *
 *************************************/

READ8_HANDLER( starwars_m6532_r )
{
	static int temp;

	switch (offset)
	{
		case 0: /* 0x80 - Read Port A */

			/* Note: bit 4 is always set to avoid sound self test */

			return port_A|0x10|(!tms5220_ready_r()<<2);

		case 1: /* 0x81 - Read Port A DDR */
			return port_A_ddr;

		case 2: /* 0x82 - Read Port B */
			return port_B;  /* speech data read? */

		case 3: /* 0x83 - Read Port B DDR */
			return port_B_ddr;

		case 5: /* 0x85 - Read Interrupt Flag Register */
			if (irq_flag)
				cpunum_set_input_line(Machine, 1, M6809_IRQ_LINE, CLEAR_LINE);
			temp = irq_flag;
			irq_flag = 0;   /* Clear int flags */
			return temp;

		default:
			return 0;
	}

	return 0; /* will never execute this */
}



/*************************************
 *
 *  M6532 I/O write
 *
 *************************************/

WRITE8_HANDLER( starwars_m6532_w )
{
	switch (offset)
	{
		case 0: /* 0x80 - Port A Write */

			/* Write to speech chip on PA0 falling edge */

			if((port_A&0x01)==1)
			{
				port_A = (port_A&(~port_A_ddr))|(data&port_A_ddr);
				if ((port_A&0x01)==0)
					tms5220_data_w(0,port_B);
			}
			else
				port_A = (port_A&(~port_A_ddr))|(data&port_A_ddr);

			return;

		case 1: /* 0x81 - Port A DDR Write */
			port_A_ddr = data;
			return;

		case 2: /* 0x82 - Port B Write */
			/* TMS5220 Speech Data on port B */

			/* ignore DDR for now */
			port_B = data;

			return;

		case 3: /* 0x83 - Port B DDR Write */
			port_B_ddr = data;
			return;

		case 7: /* 0x87 - Enable Interrupt on PA7 Transitions */

			/* This feature is emulated now.  When the Main CPU  */
			/* writes to mainwrite, it may send an IRQ to the    */
			/* sound CPU, depending on the state of this flag.   */

			PA7_irq = data;
			return;


		case 0x1f: /* 0x9f - Set Timer to decrement every n*1024 clocks, */
			/*        With IRQ enabled on countdown               */

			/* Should be decrementing every data*1024 6532 clock cycles */
			/* 6532 runs at 1.5 MHz */

			timer_set(attotime_mul(ATTOTIME_IN_HZ(1500000), data * 1024), NULL, 0, snd_interrupt);
			return;

		default:
			return;
	}

	return; /* will never execute this */

}



/*************************************
 *
 *  Sound CPU to/from main CPU
 *
 *************************************/

static TIMER_CALLBACK( sound_callback )
{
	port_A |= 0x40; /* result from sound cpu pending */
	main_data = param;
	cpu_boost_interleave(attotime_zero, ATTOTIME_IN_USEC(100));
}

READ8_HANDLER( starwars_sin_r )
{
	port_A &= 0x7f; /* ready to receive new commands from main */
	if (PA7_irq)
		cpunum_set_input_line(Machine, 1, M6809_IRQ_LINE, CLEAR_LINE);
	return sound_data;
}


WRITE8_HANDLER( starwars_sout_w )
{
	timer_call_after_resynch(NULL, data, sound_callback);
}



/*************************************
 *
 *  Main CPU to/from source CPU
 *
 *************************************/

READ8_HANDLER( starwars_main_read_r )
{
	port_A &= 0xbf;  /* ready to receive new commands from sound cpu */
	return main_data;
}


READ8_HANDLER( starwars_main_ready_flag_r )
{
	return (port_A & 0xc0); /* only upper two flag bits mapped */
}

static TIMER_CALLBACK( main_callback )
{
	if (port_A & 0x80)
		logerror ("Sound data not read %x\n",sound_data);

	port_A |= 0x80;  /* command from main cpu pending */
	sound_data = param;
	cpu_boost_interleave(attotime_zero, ATTOTIME_IN_USEC(100));

	if (PA7_irq)
		cpunum_set_input_line(machine, 1, M6809_IRQ_LINE, ASSERT_LINE);
}

WRITE8_HANDLER( starwars_main_wr_w )
{
	timer_call_after_resynch(NULL, data, main_callback);
}


WRITE8_HANDLER( starwars_soundrst_w )
{
	port_A &= 0x3f;

	/* reset sound CPU here  */
	cpunum_set_input_line(Machine, 1, INPUT_LINE_RESET, PULSE_LINE);
}

