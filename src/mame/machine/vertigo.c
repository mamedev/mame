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

static void v_irq4_w(int level);
static void v_irq3_w(int level);
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
static const struct pit8253_config pit8254_config =
{
	TYPE8254,
	{
		{
			240000,
			v_irq4_w,
			NULL
		}, {
			240000,
			v_irq3_w,
			NULL
		}, {
			240000,
			NULL,
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
		cpunum_set_input_line(Machine, 0, irq_state ^ 7, CLEAR_LINE);

	irq_state = TTL74148_output_r(0);

	if (irq_state < 7)
		cpunum_set_input_line(Machine, 0, irq_state ^ 7, ASSERT_LINE);
}


static void update_irq_encoder(int line, int state)
{
	TTL74148_input_line_w(0, line, !state);
	TTL74148_update(0);
}


static void v_irq4_w(int level)
{
	update_irq_encoder(INPUT_LINE_IRQ4, level);
	vertigo_vproc(ATTOTIME_TO_CYCLES(0, attotime_sub(timer_get_time(), irq4_time)), level);
	irq4_time = timer_get_time();
}


static void v_irq3_w(int level)
{
	if (level)
		cpunum_set_input_line(Machine, 1, INPUT_LINE_IRQ0, ASSERT_LINE);

	update_irq_encoder(INPUT_LINE_IRQ3, level);
}



/*************************************
 *
 *  ADC and coin handlers
 *
 *************************************/

READ16_HANDLER( vertigo_io_convert )
{
	if (offset > 2)
		adc_result = 0;
	else
		adc_result = readinputport(offset);

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
	return (readinputportbytag("COIN"));
}


INTERRUPT_GEN( vertigo_interrupt )
{
	/* Coin inputs cause IRQ6 */
	if ((readinputportbytag("COIN") & 0x7) < 0x7)
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
		cpunum_set_input_line(Machine, 1, INPUT_LINE_RESET, ASSERT_LINE);
	else
		cpunum_set_input_line(Machine, 1, INPUT_LINE_RESET, CLEAR_LINE);
}


static TIMER_CALLBACK( sound_command_w )
{
	exidy440_sound_command = param;
	exidy440_sound_command_ack = 0;
	cpunum_set_input_line(machine, 1, INPUT_LINE_IRQ1, ASSERT_LINE);

	/* It is important that the sound cpu ACKs the sound command
       quickly. Otherwise the main CPU gives up with sound. Boosting
       the interleave for a while helps. */

	cpu_boost_interleave(attotime_zero, ATTOTIME_IN_USEC(100));
}


WRITE16_HANDLER( vertigo_audio_w )
{
	if (ACCESSING_BYTE_0)
		timer_call_after_resynch(NULL, data & 0xff, sound_command_w);
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
	pit8253_init(1, &pit8254_config);
	vertigo_vproc_init();

	irq4_time = timer_get_time();
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
