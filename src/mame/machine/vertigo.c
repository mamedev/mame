/*************************************************************************

    Exidy Vertigo hardware

*************************************************************************/

#include "emu.h"
#include "includes/vertigo.h"
#include "includes/exidy440.h"
#include "machine/74148.h"
#include "machine/pit8253.h"


static running_device *ttl74148;



/*************************************
 *
 *  Prototypes
 *
 *************************************/

static PIT8253_OUTPUT_CHANGED( v_irq4_w );
static PIT8253_OUTPUT_CHANGED( v_irq3_w );



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



/*************************************
 *
 *  IRQ handling. The priority encoder
 *  has to be emulated. Otherwise
 *  interrupts are lost.
 *
 *************************************/

void vertigo_update_irq(running_device *device)
{
	if (irq_state < 7)
		cputag_set_input_line(device->machine, "maincpu", irq_state ^ 7, CLEAR_LINE);

	irq_state = ttl74148_output_r(device);

	if (irq_state < 7)
		cputag_set_input_line(device->machine, "maincpu", irq_state ^ 7, ASSERT_LINE);
}


static void update_irq_encoder(running_machine *machine, int line, int state)
{
	ttl74148_input_line_w(ttl74148, line, !state);
	ttl74148_update(ttl74148);
}


static PIT8253_OUTPUT_CHANGED( v_irq4_w )
{
	update_irq_encoder(device->machine, INPUT_LINE_IRQ4, state);
	vertigo_vproc(cputag_attotime_to_clocks(device->machine, "maincpu", attotime_sub(timer_get_time(device->machine), irq4_time)), state);
	irq4_time = timer_get_time(device->machine);
}


static PIT8253_OUTPUT_CHANGED( v_irq3_w )
{
	if (state)
		cputag_set_input_line(device->machine, "audiocpu", INPUT_LINE_IRQ0, ASSERT_LINE);

	update_irq_encoder(device->machine, INPUT_LINE_IRQ3, state);
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

	update_irq_encoder(space->machine, INPUT_LINE_IRQ2, ASSERT_LINE);
	return 0;
}


READ16_HANDLER( vertigo_io_adc )
{
	update_irq_encoder(space->machine, INPUT_LINE_IRQ2, CLEAR_LINE);
	return adc_result;
}


READ16_HANDLER( vertigo_coin_r )
{
	update_irq_encoder(space->machine, INPUT_LINE_IRQ6, CLEAR_LINE);
	return (input_port_read(space->machine, "COIN"));
}


INTERRUPT_GEN( vertigo_interrupt )
{
	/* Coin inputs cause IRQ6 */
	if ((input_port_read(device->machine, "COIN") & 0x7) < 0x7)
		update_irq_encoder(device->machine, INPUT_LINE_IRQ6, ASSERT_LINE);
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
		cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_RESET, ASSERT_LINE);
	else
		cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_RESET, CLEAR_LINE);
}


static TIMER_CALLBACK( sound_command_w )
{
	exidy440_sound_command = param;
	exidy440_sound_command_ack = 0;
	cputag_set_input_line(machine, "audiocpu", INPUT_LINE_IRQ1, ASSERT_LINE);

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

MACHINE_START( vertigo )
{
	state_save_register_global(machine, irq_state);
	state_save_register_global(machine, adc_result);
	state_save_register_global(machine, irq4_time.seconds);
	state_save_register_global(machine, irq4_time.attoseconds);

	vertigo_vproc_init(machine);
}

MACHINE_RESET( vertigo )
{
	int i;

	ttl74148 = devtag_get_device(machine, "74148");
	ttl74148_enable_input_w(ttl74148, 0);

	for (i = 0; i < 8; i++)
		ttl74148_input_line_w(ttl74148, i, 1);

	ttl74148_update(ttl74148);
	vertigo_vproc_reset(machine);

	irq4_time = timer_get_time(machine);
	irq_state = 7;
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
