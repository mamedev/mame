/***************************************************************************

    Atari Major Havoc hardware

***************************************************************************/

#include "emu.h"
#include "sound/tms5220.h"
#include "cpu/m6502/m6502.h"
#include "includes/mhavoc.h"


/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

TIMER_DEVICE_CALLBACK( mhavoc_cpu_irq_clock )
{
	mhavoc_state *state = timer.machine->driver_data<mhavoc_state>();
	/* clock the LS161 driving the alpha CPU IRQ */
	if (state->alpha_irq_clock_enable)
	{
		state->alpha_irq_clock++;
		if ((state->alpha_irq_clock & 0x0c) == 0x0c)
		{
			cputag_set_input_line(timer.machine, "alpha", 0, ASSERT_LINE);
			state->alpha_irq_clock_enable = 0;
		}
	}

	/* clock the LS161 driving the gamma CPU IRQ */
	if (state->has_gamma_cpu)
	{
		state->gamma_irq_clock++;
		cputag_set_input_line(timer.machine, "gamma", 0, (state->gamma_irq_clock & 0x08) ? ASSERT_LINE : CLEAR_LINE);
	}
}


WRITE8_HANDLER( mhavoc_alpha_irq_ack_w )
{
	mhavoc_state *state = space->machine->driver_data<mhavoc_state>();
	/* clear the line and reset the clock */
	cputag_set_input_line(space->machine, "alpha", 0, CLEAR_LINE);
	state->alpha_irq_clock = 0;
	state->alpha_irq_clock_enable = 1;
}


WRITE8_HANDLER( mhavoc_gamma_irq_ack_w )
{
	mhavoc_state *state = space->machine->driver_data<mhavoc_state>();
	/* clear the line and reset the clock */
	cputag_set_input_line(space->machine, "gamma", 0, CLEAR_LINE);
	state->gamma_irq_clock = 0;
}



/*************************************
 *
 *  Machine init
 *
 *************************************/

MACHINE_START( mhavoc )
{
	mhavoc_state *state = machine->driver_data<mhavoc_state>();
	state_save_register_item(machine, "misc", NULL, 0, state->alpha_data);
	state_save_register_item(machine, "misc", NULL, 0, state->alpha_rcvd);
	state_save_register_item(machine, "misc", NULL, 0, state->alpha_xmtd);
	state_save_register_item(machine, "misc", NULL, 0, state->gamma_data);
	state_save_register_item(machine, "misc", NULL, 0, state->gamma_rcvd);
	state_save_register_item(machine, "misc", NULL, 0, state->gamma_xmtd);
	state_save_register_item(machine, "misc", NULL, 0, state->player_1);
	state_save_register_item(machine, "misc", NULL, 0, state->alpha_irq_clock);
	state_save_register_item(machine, "misc", NULL, 0, state->alpha_irq_clock_enable);
	state_save_register_item(machine, "misc", NULL, 0, state->gamma_irq_clock);

	state_save_register_item(machine, "misc", NULL, 0, state->speech_write_buffer);
}


MACHINE_RESET( mhavoc )
{
	mhavoc_state *state = machine->driver_data<mhavoc_state>();
	address_space *space = cputag_get_address_space(machine, "alpha", ADDRESS_SPACE_PROGRAM);
	state->has_gamma_cpu = (machine->device("gamma") != NULL);

	memory_configure_bank(machine, "bank1", 0, 1, state->zram0, 0);
	memory_configure_bank(machine, "bank1", 1, 1, state->zram1, 0);
	memory_configure_bank(machine, "bank2", 0, 4, machine->region("alpha")->base() + 0x10000, 0x2000);

	/* reset RAM/ROM banks to 0 */
	mhavoc_ram_banksel_w(space, 0, 0);
	mhavoc_rom_banksel_w(space, 0, 0);

	/* reset alpha comm status */
	state->alpha_data = 0;
	state->alpha_rcvd = 0;
	state->alpha_xmtd = 0;

	/* reset gamma comm status */
	state->gamma_data = 0;
	state->gamma_rcvd = 0;
	state->gamma_xmtd = 0;

	/* reset player 1 flag */
	state->player_1 = 0;

	/* reset IRQ clock states */
	state->alpha_irq_clock = 0;
	state->alpha_irq_clock_enable = 1;
	state->gamma_irq_clock = 0;
}



/*************************************
 *
 *  Alpha -> gamma communications
 *
 *************************************/

static TIMER_CALLBACK( delayed_gamma_w )
{
	mhavoc_state *state = machine->driver_data<mhavoc_state>();
	/* mark the data received */
	state->gamma_rcvd = 0;
	state->alpha_xmtd = 1;
	state->alpha_data = param;

	/* signal with an NMI pulse */
	cputag_set_input_line(machine, "gamma", INPUT_LINE_NMI, PULSE_LINE);

	/* the sound CPU needs to reply in 250microseconds (according to Neil Bradley) */
	timer_set(machine, attotime::from_usec(250), NULL, 0, 0);
}


WRITE8_HANDLER( mhavoc_gamma_w )
{
	mhavoc_state *state = space->machine->driver_data<mhavoc_state>();
	logerror("  writing to gamma processor: %02x (%d %d)\n", data, state->gamma_rcvd, state->alpha_xmtd);
	timer_call_after_resynch(space->machine, NULL, data, delayed_gamma_w);
}


READ8_HANDLER( mhavoc_alpha_r )
{
	mhavoc_state *state = space->machine->driver_data<mhavoc_state>();
	logerror("\t\t\t\t\treading from alpha processor: %02x (%d %d)\n", state->alpha_data, state->gamma_rcvd, state->alpha_xmtd);
	state->gamma_rcvd = 1;
	state->alpha_xmtd = 0;
	return state->alpha_data;
}



/*************************************
 *
 *  Gamma -> alpha communications
 *
 *************************************/

WRITE8_HANDLER( mhavoc_alpha_w )
{
	mhavoc_state *state = space->machine->driver_data<mhavoc_state>();
	logerror("\t\t\t\t\twriting to alpha processor: %02x %d %d\n", data, state->alpha_rcvd, state->gamma_xmtd);
	state->alpha_rcvd = 0;
	state->gamma_xmtd = 1;
	state->gamma_data = data;
}


READ8_HANDLER( mhavoc_gamma_r )
{
	mhavoc_state *state = space->machine->driver_data<mhavoc_state>();
	logerror("  reading from gamma processor: %02x (%d %d)\n", state->gamma_data, state->alpha_rcvd, state->gamma_xmtd);
	state->alpha_rcvd = 1;
	state->gamma_xmtd = 0;
	return state->gamma_data;
}



/*************************************
 *
 *  RAM/ROM banking
 *
 *************************************/

WRITE8_HANDLER( mhavoc_ram_banksel_w )
{
	memory_set_bank(space->machine, "bank1", data & 1);
}


WRITE8_HANDLER( mhavoc_rom_banksel_w )
{
	memory_set_bank(space->machine, "bank2", data & 3);
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

CUSTOM_INPUT( tms5220_r )
{
	return tms5220_readyq_r(field->port->machine->device("tms")) ? 1 : 0;
}

CUSTOM_INPUT( mhavoc_bit67_r )
{
	mhavoc_state *state = field->port->machine->driver_data<mhavoc_state>();
	const char *tag1 = (const char *)param;
	const char *tag2 = tag1 + strlen(tag1) + 1;
	return input_port_read(field->port->machine, state->player_1 ? tag2 : tag1) & 0x03;
}

CUSTOM_INPUT( gamma_rcvd_r )
{
	mhavoc_state *state = field->port->machine->driver_data<mhavoc_state>();
	/* Gamma rcvd flag */
	return state->gamma_rcvd;
}

CUSTOM_INPUT( gamma_xmtd_r )
{
	mhavoc_state *state = field->port->machine->driver_data<mhavoc_state>();
	/* Gamma xmtd flag */
	return state->gamma_xmtd;
}

CUSTOM_INPUT( alpha_rcvd_r )
{
	mhavoc_state *state = field->port->machine->driver_data<mhavoc_state>();
	/* Alpha rcvd flag */
	return (state->has_gamma_cpu && state->alpha_rcvd);
}

CUSTOM_INPUT( alpha_xmtd_r )
{
	mhavoc_state *state = field->port->machine->driver_data<mhavoc_state>();
	/* Alpha xmtd flag */
	return (state->has_gamma_cpu && state->alpha_xmtd);
}

/*************************************
 *
 *  Output ports
 *
 *************************************/

WRITE8_HANDLER( mhavoc_out_0_w )
{
	mhavoc_state *state = space->machine->driver_data<mhavoc_state>();
	/* Bit 7 = Invert Y -- unemulated */
	/* Bit 6 = Invert X -- unemulated */

	/* Bit 5 = Player 1 */
	state->player_1 = (data >> 5) & 1;

	/* Bit 3 = Gamma reset */
	cputag_set_input_line(space->machine, "gamma", INPUT_LINE_RESET, (data & 0x08) ? CLEAR_LINE : ASSERT_LINE);
	if (!(data & 0x08))
	{
		logerror("\t\t\t\t*** resetting gamma processor. ***\n");
		state->alpha_rcvd = 0;
		state->alpha_xmtd = 0;
		state->gamma_rcvd = 0;
		state->gamma_xmtd = 0;
	}

	/* Bit 0 = Roller light (Blinks on fatal errors) */
	set_led_status(space->machine, 0, data & 0x01);
}


WRITE8_HANDLER( alphaone_out_0_w )
{
	/* Bit 5 = P2 lamp */
	set_led_status(space->machine, 0, ~data & 0x20);

	/* Bit 4 = P1 lamp */
	set_led_status(space->machine, 1, ~data & 0x10);

	/* Bit 1 = right coin counter */
	coin_counter_w(space->machine, 1, data & 0x02);

	/* Bit 0 = left coin counter */
	coin_counter_w(space->machine, 0, data & 0x01);

logerror("alphaone_out_0_w(%02X)\n", data);
}


WRITE8_HANDLER( mhavoc_out_1_w )
{
	/* Bit 1 = left coin counter */
	coin_counter_w(space->machine, 0, data & 0x02);

	/* Bit 0 = right coin counter */
	coin_counter_w(space->machine, 1, data & 0x01);
}

/*************************************
 *
 *  Speech access
 *
 *************************************/

static WRITE8_HANDLER( mhavocrv_speech_data_w )
{
	mhavoc_state *state = space->machine->driver_data<mhavoc_state>();
	state->speech_write_buffer = data;
}


static WRITE8_HANDLER( mhavocrv_speech_strobe_w )
{
	mhavoc_state *state = space->machine->driver_data<mhavoc_state>();
	device_t *tms = space->machine->device("tms");
	tms5220_data_w(tms, 0, state->speech_write_buffer);
}

/*************************************
 *
 *  Driver-specific init
 *
 *************************************/

DRIVER_INIT( mhavocrv )
{
	/* install the speech support that was only optionally stuffed for use */
	/* in the Return to Vax hack */
	memory_install_write8_handler(cputag_get_address_space(machine, "gamma", ADDRESS_SPACE_PROGRAM), 0x5800, 0x5800, 0, 0, mhavocrv_speech_data_w);
	memory_install_write8_handler(cputag_get_address_space(machine, "gamma", ADDRESS_SPACE_PROGRAM), 0x5900, 0x5900, 0, 0, mhavocrv_speech_strobe_w);
}
