/***************************************************************************

    Atari Major Havoc hardware

***************************************************************************/

#include "driver.h"
#include "video/avgdvg.h"
#include "sound/tms5220.h"
#include "cpu/m6502/m6502.h"
#include "mhavoc.h"

UINT8 *mhavoc_zram0, *mhavoc_zram1;

static UINT8 alpha_data;
static UINT8 alpha_rcvd;
static UINT8 alpha_xmtd;

static UINT8 gamma_data;
static UINT8 gamma_rcvd;
static UINT8 gamma_xmtd;

static UINT8 player_1;

static UINT8 alpha_irq_clock;
static UINT8 alpha_irq_clock_enable;
static UINT8 gamma_irq_clock;

static UINT8 has_gamma_cpu;

static UINT8 speech_write_buffer;

/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

static TIMER_CALLBACK( cpu_irq_clock )
{
	/* clock the LS161 driving the alpha CPU IRQ */
	if (alpha_irq_clock_enable)
	{
		alpha_irq_clock++;
		if ((alpha_irq_clock & 0x0c) == 0x0c)
		{
			cputag_set_input_line(machine, "alpha", 0, ASSERT_LINE);
			alpha_irq_clock_enable = 0;
		}
	}

	/* clock the LS161 driving the gamma CPU IRQ */
	if (has_gamma_cpu)
	{
		gamma_irq_clock++;
		cputag_set_input_line(machine, "gamma", 0, (gamma_irq_clock & 0x08) ? ASSERT_LINE : CLEAR_LINE);
	}
}


WRITE8_HANDLER( mhavoc_alpha_irq_ack_w )
{
	/* clear the line and reset the clock */
	cputag_set_input_line(space->machine, "alpha", 0, CLEAR_LINE);
	alpha_irq_clock = 0;
	alpha_irq_clock_enable = 1;
}


WRITE8_HANDLER( mhavoc_gamma_irq_ack_w )
{
	/* clear the line and reset the clock */
	cputag_set_input_line(space->machine, "gamma", 0, CLEAR_LINE);
	gamma_irq_clock = 0;
}



/*************************************
 *
 *  Machine init
 *
 *************************************/

MACHINE_RESET( mhavoc )
{
	const address_space *space = cputag_get_address_space(machine, "alpha", ADDRESS_SPACE_PROGRAM);
	has_gamma_cpu = (cputag_get_cpu(machine, "gamma") != NULL);

	memory_configure_bank(machine, 1, 0, 1, mhavoc_zram0, 0);
	memory_configure_bank(machine, 1, 1, 1, mhavoc_zram1, 0);
	memory_configure_bank(machine, 2, 0, 4, memory_region(machine, "alpha") + 0x10000, 0x2000);

	/* reset RAM/ROM banks to 0 */
	mhavoc_ram_banksel_w(space, 0, 0);
	mhavoc_rom_banksel_w(space, 0, 0);

	/* reset alpha comm status */
	alpha_data = 0;
	alpha_rcvd = 0;
	alpha_xmtd = 0;

	/* reset gamma comm status */
	gamma_data = 0;
	gamma_rcvd = 0;
	gamma_xmtd = 0;

	/* reset player 1 flag */
	player_1 = 0;

	/* reset IRQ clock states */
	alpha_irq_clock = 0;
	alpha_irq_clock_enable = 1;
	gamma_irq_clock = 0;

	/* set a timer going for the CPU interrupt generators */
	timer_pulse(machine, ATTOTIME_IN_HZ(MHAVOC_CLOCK_5K), NULL, 0, cpu_irq_clock);

	state_save_register_item(machine, "misc", NULL, 0, alpha_data);
	state_save_register_item(machine, "misc", NULL, 0, alpha_rcvd);
	state_save_register_item(machine, "misc", NULL, 0, alpha_xmtd);
	state_save_register_item(machine, "misc", NULL, 0, gamma_data);
	state_save_register_item(machine, "misc", NULL, 0, gamma_rcvd);
	state_save_register_item(machine, "misc", NULL, 0, gamma_xmtd);
	state_save_register_item(machine, "misc", NULL, 0, player_1);
	state_save_register_item(machine, "misc", NULL, 0, alpha_irq_clock);
	state_save_register_item(machine, "misc", NULL, 0, alpha_irq_clock_enable);
	state_save_register_item(machine, "misc", NULL, 0, gamma_irq_clock);

	state_save_register_item(machine, "misc", NULL, 0, speech_write_buffer);
}



/*************************************
 *
 *  Alpha -> gamma communications
 *
 *************************************/

static TIMER_CALLBACK( delayed_gamma_w )
{
	/* mark the data received */
	gamma_rcvd = 0;
	alpha_xmtd = 1;
	alpha_data = param;

	/* signal with an NMI pulse */
	cputag_set_input_line(machine, "gamma", INPUT_LINE_NMI, PULSE_LINE);

	/* the sound CPU needs to reply in 250microseconds (according to Neil Bradley) */
	timer_set(machine, ATTOTIME_IN_USEC(250), NULL, 0, 0);
}


WRITE8_HANDLER( mhavoc_gamma_w )
{
	logerror("  writing to gamma processor: %02x (%d %d)\n", data, gamma_rcvd, alpha_xmtd);
	timer_call_after_resynch(space->machine, NULL, data, delayed_gamma_w);
}


READ8_HANDLER( mhavoc_alpha_r )
{
	logerror("\t\t\t\t\treading from alpha processor: %02x (%d %d)\n", alpha_data, gamma_rcvd, alpha_xmtd);
	gamma_rcvd = 1;
	alpha_xmtd = 0;
	return alpha_data;
}



/*************************************
 *
 *  Gamma -> alpha communications
 *
 *************************************/

WRITE8_HANDLER( mhavoc_alpha_w )
{
	logerror("\t\t\t\t\twriting to alpha processor: %02x %d %d\n", data, alpha_rcvd, gamma_xmtd);
	alpha_rcvd = 0;
	gamma_xmtd = 1;
	gamma_data = data;
}


READ8_HANDLER( mhavoc_gamma_r )
{
	logerror("  reading from gamma processor: %02x (%d %d)\n", gamma_data, alpha_rcvd, gamma_xmtd);
	alpha_rcvd = 1;
	gamma_xmtd = 0;
	return gamma_data;
}



/*************************************
 *
 *  RAM/ROM banking
 *
 *************************************/

WRITE8_HANDLER( mhavoc_ram_banksel_w )
{
	memory_set_bank(space->machine, 1, data & 1);
}


WRITE8_HANDLER( mhavoc_rom_banksel_w )
{
	memory_set_bank(space->machine, 2, data & 3);
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

CUSTOM_INPUT( tms5220_r )
{
	return tms5220_readyq_r(devtag_get_device(field->port->machine, "tms")) ? 1 : 0;
}

CUSTOM_INPUT( mhavoc_bit67_r )
{
	const char *tag1 = (const char *)param;
	const char *tag2 = tag1 + strlen(tag1) + 1;
	return input_port_read(field->port->machine, player_1 ? tag2 : tag1) & 0x03;
}

CUSTOM_INPUT( gamma_rcvd_r )
{
	/* Gamma rcvd flag */
	return gamma_rcvd;
}

CUSTOM_INPUT( gamma_xmtd_r )
{
	/* Gamma xmtd flag */
	return gamma_xmtd;
}

CUSTOM_INPUT( alpha_rcvd_r )
{
	/* Alpha rcvd flag */
	return (has_gamma_cpu && alpha_rcvd);
}

CUSTOM_INPUT( alpha_xmtd_r )
{
	/* Alpha xmtd flag */
	return (has_gamma_cpu && alpha_xmtd);
}

/*************************************
 *
 *  Output ports
 *
 *************************************/

WRITE8_HANDLER( mhavoc_out_0_w )
{
	/* Bit 7 = Invert Y -- unemulated */
	/* Bit 6 = Invert X -- unemulated */

	/* Bit 5 = Player 1 */
	player_1 = (data >> 5) & 1;

	/* Bit 3 = Gamma reset */
	cputag_set_input_line(space->machine, "gamma", INPUT_LINE_RESET, (data & 0x08) ? CLEAR_LINE : ASSERT_LINE);
	if (!(data & 0x08))
	{
		logerror("\t\t\t\t*** resetting gamma processor. ***\n");
		alpha_rcvd = 0;
		alpha_xmtd = 0;
		gamma_rcvd = 0;
		gamma_xmtd = 0;
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
	speech_write_buffer = data;
}


static WRITE8_HANDLER( mhavocrv_speech_strobe_w )
{
	const device_config *tms = devtag_get_device(space->machine, "tms");
	tms5220_data_w(tms, 0, speech_write_buffer);
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
