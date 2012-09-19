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
	mhavoc_state *state = timer.machine().driver_data<mhavoc_state>();
	/* clock the LS161 driving the alpha CPU IRQ */
	if (state->m_alpha_irq_clock_enable)
	{
		state->m_alpha_irq_clock++;
		if ((state->m_alpha_irq_clock & 0x0c) == 0x0c)
		{
			timer.machine().device("alpha")->execute().set_input_line(0, ASSERT_LINE);
			state->m_alpha_irq_clock_enable = 0;
		}
	}

	/* clock the LS161 driving the gamma CPU IRQ */
	if (state->m_has_gamma_cpu)
	{
		state->m_gamma_irq_clock++;
		timer.machine().device("gamma")->execute().set_input_line(0, (state->m_gamma_irq_clock & 0x08) ? ASSERT_LINE : CLEAR_LINE);
	}
}


WRITE8_MEMBER(mhavoc_state::mhavoc_alpha_irq_ack_w)
{
	/* clear the line and reset the clock */
	machine().device("alpha")->execute().set_input_line(0, CLEAR_LINE);
	m_alpha_irq_clock = 0;
	m_alpha_irq_clock_enable = 1;
}


WRITE8_MEMBER(mhavoc_state::mhavoc_gamma_irq_ack_w)
{
	/* clear the line and reset the clock */
	machine().device("gamma")->execute().set_input_line(0, CLEAR_LINE);
	m_gamma_irq_clock = 0;
}



/*************************************
 *
 *  Machine init
 *
 *************************************/

void mhavoc_state::machine_start()
{
	state_save_register_item(machine(), "misc", NULL, 0, m_alpha_data);
	state_save_register_item(machine(), "misc", NULL, 0, m_alpha_rcvd);
	state_save_register_item(machine(), "misc", NULL, 0, m_alpha_xmtd);
	state_save_register_item(machine(), "misc", NULL, 0, m_gamma_data);
	state_save_register_item(machine(), "misc", NULL, 0, m_gamma_rcvd);
	state_save_register_item(machine(), "misc", NULL, 0, m_gamma_xmtd);
	state_save_register_item(machine(), "misc", NULL, 0, m_player_1);
	state_save_register_item(machine(), "misc", NULL, 0, m_alpha_irq_clock);
	state_save_register_item(machine(), "misc", NULL, 0, m_alpha_irq_clock_enable);
	state_save_register_item(machine(), "misc", NULL, 0, m_gamma_irq_clock);

	state_save_register_item(machine(), "misc", NULL, 0, m_speech_write_buffer);
}


void mhavoc_state::machine_reset()
{
	address_space &space = machine().device("alpha")->memory().space(AS_PROGRAM);
	m_has_gamma_cpu = (machine().device("gamma") != NULL);

	membank("bank1")->configure_entry(0, m_zram0);
	membank("bank1")->configure_entry(1, m_zram1);
	membank("bank2")->configure_entries(0, 4, memregion("alpha")->base() + 0x10000, 0x2000);

	/* reset RAM/ROM banks to 0 */
	mhavoc_ram_banksel_w(space, 0, 0);
	mhavoc_rom_banksel_w(space, 0, 0);

	/* reset alpha comm status */
	m_alpha_data = 0;
	m_alpha_rcvd = 0;
	m_alpha_xmtd = 0;

	/* reset gamma comm status */
	m_gamma_data = 0;
	m_gamma_rcvd = 0;
	m_gamma_xmtd = 0;

	/* reset player 1 flag */
	m_player_1 = 0;

	/* reset IRQ clock states */
	m_alpha_irq_clock = 0;
	m_alpha_irq_clock_enable = 1;
	m_gamma_irq_clock = 0;
}



/*************************************
 *
 *  Alpha -> gamma communications
 *
 *************************************/

static TIMER_CALLBACK( delayed_gamma_w )
{
	mhavoc_state *state = machine.driver_data<mhavoc_state>();
	/* mark the data received */
	state->m_gamma_rcvd = 0;
	state->m_alpha_xmtd = 1;
	state->m_alpha_data = param;

	/* signal with an NMI pulse */
	machine.device("gamma")->execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);

	/* the sound CPU needs to reply in 250microseconds (according to Neil Bradley) */
	machine.scheduler().timer_set(attotime::from_usec(250), FUNC_NULL);
}


WRITE8_MEMBER(mhavoc_state::mhavoc_gamma_w)
{
	logerror("  writing to gamma processor: %02x (%d %d)\n", data, m_gamma_rcvd, m_alpha_xmtd);
	machine().scheduler().synchronize(FUNC(delayed_gamma_w), data);
}


READ8_MEMBER(mhavoc_state::mhavoc_alpha_r)
{
	logerror("\t\t\t\t\treading from alpha processor: %02x (%d %d)\n", m_alpha_data, m_gamma_rcvd, m_alpha_xmtd);
	m_gamma_rcvd = 1;
	m_alpha_xmtd = 0;
	return m_alpha_data;
}



/*************************************
 *
 *  Gamma -> alpha communications
 *
 *************************************/

WRITE8_MEMBER(mhavoc_state::mhavoc_alpha_w)
{
	logerror("\t\t\t\t\twriting to alpha processor: %02x %d %d\n", data, m_alpha_rcvd, m_gamma_xmtd);
	m_alpha_rcvd = 0;
	m_gamma_xmtd = 1;
	m_gamma_data = data;
}


READ8_MEMBER(mhavoc_state::mhavoc_gamma_r)
{
	logerror("  reading from gamma processor: %02x (%d %d)\n", m_gamma_data, m_alpha_rcvd, m_gamma_xmtd);
	m_alpha_rcvd = 1;
	m_gamma_xmtd = 0;
	return m_gamma_data;
}



/*************************************
 *
 *  RAM/ROM banking
 *
 *************************************/

WRITE8_MEMBER(mhavoc_state::mhavoc_ram_banksel_w)
{
	membank("bank1")->set_entry(data & 1);
}


WRITE8_MEMBER(mhavoc_state::mhavoc_rom_banksel_w)
{
	membank("bank2")->set_entry(data & 3);
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

CUSTOM_INPUT_MEMBER(mhavoc_state::tms5220_r)
{
	return tms5220_readyq_r(machine().device("tms")) ? 1 : 0;
}

CUSTOM_INPUT_MEMBER(mhavoc_state::mhavoc_bit67_r)
{
	const char *tag1 = (const char *)param;
	const char *tag2 = tag1 + strlen(tag1) + 1;
	return ioport(m_player_1 ? tag2 : tag1)->read() & 0x03;
}

CUSTOM_INPUT_MEMBER(mhavoc_state::gamma_rcvd_r)
{
	/* Gamma rcvd flag */
	return m_gamma_rcvd;
}

CUSTOM_INPUT_MEMBER(mhavoc_state::gamma_xmtd_r)
{
	/* Gamma xmtd flag */
	return m_gamma_xmtd;
}

CUSTOM_INPUT_MEMBER(mhavoc_state::alpha_rcvd_r)
{
	/* Alpha rcvd flag */
	return (m_has_gamma_cpu && m_alpha_rcvd);
}

CUSTOM_INPUT_MEMBER(mhavoc_state::alpha_xmtd_r)
{
	/* Alpha xmtd flag */
	return (m_has_gamma_cpu && m_alpha_xmtd);
}

/*************************************
 *
 *  Output ports
 *
 *************************************/

WRITE8_MEMBER(mhavoc_state::mhavoc_out_0_w)
{
	/* Bit 7 = Invert Y -- unemulated */
	/* Bit 6 = Invert X -- unemulated */

	/* Bit 5 = Player 1 */
	m_player_1 = (data >> 5) & 1;

	/* Bit 3 = Gamma reset */
	machine().device("gamma")->execute().set_input_line(INPUT_LINE_RESET, (data & 0x08) ? CLEAR_LINE : ASSERT_LINE);
	if (!(data & 0x08))
	{
		logerror("\t\t\t\t*** resetting gamma processor. ***\n");
		m_alpha_rcvd = 0;
		m_alpha_xmtd = 0;
		m_gamma_rcvd = 0;
		m_gamma_xmtd = 0;
	}

	/* Bit 0 = Roller light (Blinks on fatal errors) */
	set_led_status(machine(), 0, data & 0x01);
}


WRITE8_MEMBER(mhavoc_state::alphaone_out_0_w)
{
	/* Bit 5 = P2 lamp */
	set_led_status(machine(), 0, ~data & 0x20);

	/* Bit 4 = P1 lamp */
	set_led_status(machine(), 1, ~data & 0x10);

	/* Bit 1 = right coin counter */
	coin_counter_w(machine(), 1, data & 0x02);

	/* Bit 0 = left coin counter */
	coin_counter_w(machine(), 0, data & 0x01);

logerror("alphaone_out_0_w(%02X)\n", data);
}


WRITE8_MEMBER(mhavoc_state::mhavoc_out_1_w)
{
	/* Bit 1 = left coin counter */
	coin_counter_w(machine(), 0, data & 0x02);

	/* Bit 0 = right coin counter */
	coin_counter_w(machine(), 1, data & 0x01);
}

/*************************************
 *
 *  Speech access
 *
 *************************************/

WRITE8_MEMBER(mhavoc_state::mhavocrv_speech_data_w)
{
	m_speech_write_buffer = data;
}


WRITE8_MEMBER(mhavoc_state::mhavocrv_speech_strobe_w)
{
	device_t *tms = machine().device("tms");
	tms5220_data_w(tms, space, 0, m_speech_write_buffer);
}

/*************************************
 *
 *  Driver-specific init
 *
 *************************************/

DRIVER_INIT_MEMBER(mhavoc_state,mhavocrv)
{
	/* install the speech support that was only optionally stuffed for use */
	/* in the Return to Vax hack */
	machine().device("gamma")->memory().space(AS_PROGRAM).install_write_handler(0x5800, 0x5800, write8_delegate(FUNC(mhavoc_state::mhavocrv_speech_data_w),this));
	machine().device("gamma")->memory().space(AS_PROGRAM).install_write_handler(0x5900, 0x5900, write8_delegate(FUNC(mhavoc_state::mhavocrv_speech_strobe_w),this));
}
