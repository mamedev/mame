// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer
/*************************************************************************

    Exidy Vertigo hardware

*************************************************************************/

#include "emu.h"
#include "includes/vertigo.h"



/*************************************
 *
 *  IRQ handling. The priority encoder
 *  has to be emulated. Otherwise
 *  interrupts are lost.
 *
 *************************************/

WRITE8_MEMBER(vertigo_state::update_irq)
{
	if (m_irq_state < 7)
		m_maincpu->set_input_line(m_irq_state ^ 7, CLEAR_LINE);

	m_irq_state = data;

	if (m_irq_state < 7)
		m_maincpu->set_input_line(m_irq_state ^ 7, ASSERT_LINE);
}


void vertigo_state::update_irq_encoder(int line, int state)
{
	m_ttl74148->input_line_w(line, !state);
	m_ttl74148->update();
}


WRITE_LINE_MEMBER(vertigo_state::v_irq4_w)
{
	update_irq_encoder(INPUT_LINE_IRQ4, state);
	vertigo_vproc(m_maincpu->attotime_to_cycles(machine().time() - m_irq4_time), state);
	m_irq4_time = machine().time();
}


WRITE_LINE_MEMBER(vertigo_state::v_irq3_w)
{
	m_custom->sound_interrupt_w(state);

	update_irq_encoder(INPUT_LINE_IRQ3, state);
}



/*************************************
 *
 *  ADC (ADC0808) and coin handlers
 *
 *************************************/

WRITE_LINE_MEMBER( vertigo_state::adc_eoc_w )
{
	update_irq_encoder(INPUT_LINE_IRQ2, state ? ASSERT_LINE : CLEAR_LINE);
}


READ16_MEMBER(vertigo_state::vertigo_io_convert)
{
	m_adc->address_offset_start_w(offset, 0);
	return 0;
}


READ16_MEMBER(vertigo_state::vertigo_coin_r)
{
	update_irq_encoder(INPUT_LINE_IRQ6, CLEAR_LINE);
	return (ioport("COIN")->read());
}


INTERRUPT_GEN_MEMBER(vertigo_state::vertigo_interrupt)
{
	/* Coin inputs cause IRQ6 */
	if ((ioport("COIN")->read() & 0x7) < 0x7)
		update_irq_encoder(INPUT_LINE_IRQ6, ASSERT_LINE);
}



/*************************************
 *
 *  Sound board interface
 *
 *************************************/

WRITE16_MEMBER(vertigo_state::vertigo_wsot_w)
{
	/* Reset sound cpu */
	if ((data & 2) == 0)
		m_custom->sound_reset_w(ASSERT_LINE);
	else
		m_custom->sound_reset_w(CLEAR_LINE);
}


TIMER_CALLBACK_MEMBER(vertigo_state::sound_command_w)
{
	m_custom->exidy440_sound_command(param);

	/* It is important that the sound cpu ACKs the sound command
	   quickly. Otherwise the main CPU gives up with sound. Boosting
	   the interleave for a while helps. */

	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(100));
}


WRITE16_MEMBER(vertigo_state::vertigo_audio_w)
{
	if (ACCESSING_BITS_0_7)
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(vertigo_state::sound_command_w),this), data & 0xff);
}


READ16_MEMBER(vertigo_state::vertigo_sio_r)
{
	return m_custom->exidy440_sound_command_ack() ? 0xfc : 0xfd;
}



/*************************************
 *
 *  Machine initialization
 *
 *************************************/

void vertigo_state::machine_start()
{
	save_item(NAME(m_irq_state));
	save_item(NAME(m_irq4_time));

	vertigo_vproc_init();
}

void vertigo_state::machine_reset()
{
	int i;

	m_ttl74148->enable_input_w(0);

	for (i = 0; i < 8; i++)
		m_ttl74148->input_line_w(i, 1);

	m_ttl74148->update();
	vertigo_vproc_reset();

	m_irq4_time = machine().time();
	m_irq_state = 7;
}



/*************************************
 *
 *  Motor controller interface
 *
 *************************************/

WRITE16_MEMBER(vertigo_state::vertigo_motor_w)
{
	/* Motor controller interface. Not emulated. */
}
