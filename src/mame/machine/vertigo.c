/*************************************************************************

    Exidy Vertigo hardware

*************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "vertigo.h"
#include "exidy440.h"
#include "machine/74148.h"
#include "machine/pit8253.h"



/*************************************
 *
 *  Prototypes
 *
 *************************************/

static PIT8253_OUTPUT_CHANGED( v_irq4_w );
static PIT8253_OUTPUT_CHANGED( v_irq3_w );
static void update_irq(void);



/*************************************
 *
 *  Statics
 *
 *************************************/

/* Timestamp of last INTL4 change. The vector CPU runs for
   the delta between this and now.
*/
static attotime irq4_time;

/* State of the priority encoder output */
static UINT8 irq_state;

/* Result of the last ADC channel sampled */
static UINT8 adc_result;

/* 8254 timer config */
const struct pit8253_config vertigo_pit8254_config =
{
	{
		{
			240000,
			v_irq4_w
		}, {
			240000,
			v_irq3_w
		}, {
			240000,
			NULL
		}
	}
};


static const struct TTL74148_interface irq_encoder =
{
	update_irq
};



/*************************************
 *
 *  IRQ handling. The priority encoder
 *  has to be emulated. Otherwise
 *  interrupts are lost.
 *
 *************************************/

static void update_irq(void)
{
	if (irq_state < 7)
		cpu_set_input_line(Machine->cpu[0], irq_state ^ 7, CLEAR_LINE);

	irq_state = TTL74148_output_r(0);

	if (irq_state < 7)
		cpu_set_input_line(Machine->cpu[0], irq_state ^ 7, ASSERT_LINE);
}


static void update_irq_encoder(int line, int state)
{
	TTL74148_input_line_w(0, line, !state);
	TTL74148_update(0);
}


static PIT8253_OUTPUT_CHANGED( v_irq4_w )
{
	update_irq_encoder(INPUT_LINE_IRQ4, state);
	vertigo_vproc(cpu_attotime_to_clocks(device->machine->cpu[0], attotime_sub(timer_get_time(device->machine), irq4_time)), state);
	irq4_time = timer_get_time(device->machine);
}


static PIT8253_OUTPUT_CHANGED( v_irq3_w )
{
	if (state)
		cpu_set_input_line(device->machine->cpu[1], INPUT_LINE_IRQ0, ASSERT_LINE);

	update_irq_encoder(INPUT_LINE_IRQ3, state);
}



/*************************************
 *
 *  ADC and coin handlers
 *
 *************************************/

READ16_HANDLER( vertigo_io_convert )
{
	static const char *const adcnames[] = { "P1X", "P1Y", "PADDLE" };

	if (offset > 2)
		adc_result = 0;
	else
		adc_result = input_port_read(space->machine, adcnames[offset]);

	update_irq_encoder(INPUT_LINE_IRQ2, ASSERT_LINE);
	return 0;
}


READ16_HANDLER( vertigo_io_adc )
{
	update_irq_encoder(INPUT_LINE_IRQ2, CLEAR_LINE);
	return adc_result;
}


READ16_HANDLER( vertigo_coin_r )
{
	update_irq_encoder(INPUT_LINE_IRQ6, CLEAR_LINE);
	return (input_port_read(space->machine, "COIN"));
}


INTERRUPT_GEN( vertigo_interrupt )
{
	/* Coin inputs cause IRQ6 */
	if ((input_port_read(device->machine, "COIN") & 0x7) < 0x7)
		update_irq_encoder(INPUT_LINE_IRQ6, ASSERT_LINE);
}



/*************************************
 *
 *  Sound board interface
 *
 *************************************/

WRITE16_HANDLER( vertigo_wsot_w )
{
	/* Reset sound cpu */
	if ((data & 2) == 0)
		cpu_set_input_line(space->machine->cpu[1], INPUT_LINE_RESET, ASSERT_LINE);
	else
		cpu_set_input_line(space->machine->cpu[1], INPUT_LINE_RESET, CLEAR_LINE);
}


static TIMER_CALLBACK( sound_command_w )
{
	exidy440_sound_command = param;
	exidy440_sound_command_ack = 0;
	cpu_set_input_line(machine->cpu[1], INPUT_LINE_IRQ1, ASSERT_LINE);

	/* It is important that the sound cpu ACKs the sound command
       quickly. Otherwise the main CPU gives up with sound. Boosting
       the interleave for a while helps. */

	cpuexec_boost_interleave(machine, attotime_zero, ATTOTIME_IN_USEC(100));
}


WRITE16_HANDLER( vertigo_audio_w )
{
	if (ACCESSING_BITS_0_7)
		timer_call_after_resynch(space->machine, NULL, data & 0xff, sound_command_w);
}


READ16_HANDLER( vertigo_sio_r )
{
	if (exidy440_sound_command_ack)
		return 0xfc;
	else
		return 0xfd;
}



/*************************************
 *
 *  Machine initialization
 *
 *************************************/

MACHINE_RESET( vertigo )
{
	int i;

	TTL74148_config(0, &irq_encoder);
	TTL74148_enable_input_w(0, 0);

	for (i = 0; i < 8; i++)
		TTL74148_input_line_w(0, i, 1);

	TTL74148_update(0);
	vertigo_vproc_init(machine);

	irq4_time = timer_get_time(machine);
	irq_state = 7;

	state_save_register_global(irq_state);
	state_save_register_global(adc_result);
	state_save_register_global(irq4_time.seconds);
	state_save_register_global(irq4_time.attoseconds);
}



/*************************************
 *
 *  Motor controller interface
 *
 *************************************/

WRITE16_HANDLER( vertigo_motor_w )
{
	/* Motor controller interface. Not emulated. */
}
