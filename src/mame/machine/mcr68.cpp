// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Bryan McPhail
/***************************************************************************

    Midway MCR-68k system

***************************************************************************/

#include "emu.h"
#include "audio/midway.h"
#include "includes/mcr68.h"

#define VERBOSE 0


/*************************************
 *
 *  6821 PIA declarations
 *
 *************************************/

READ8_MEMBER(mcr68_state::zwackery_port_1_r)
{
	uint8_t ret = ioport("IN1")->read();

	downcast<pia6821_device *>(machine().device("pia1"))->set_port_a_z_mask(ret);

	return ret;
}


READ8_MEMBER(mcr68_state::zwackery_port_3_r)
{
	uint8_t ret = ioport("IN3")->read();

	downcast<pia6821_device *>(machine().device("pia2"))->set_port_a_z_mask(ret);

	return ret;
}



/*************************************
 *
 *  Generic MCR/68k machine initialization
 *
 *************************************/

MACHINE_START_MEMBER(mcr68_state,mcr68)
{
	save_item(NAME(m_v493_irq_state));
	save_item(NAME(m_zwackery_sound_data));
}


MACHINE_RESET_MEMBER(mcr68_state,mcr68)
{
	/* for the most part all MCR/68k games are the same */
	m_v493_callback = timer_expired_delegate(FUNC(mcr68_state::mcr68_493_callback),this);

	/* vectors are 1 and 2 */
	m_v493_irq_vector = 1;
	m_m6840_irq_vector = 2;
}


MACHINE_START_MEMBER(mcr68_state,zwackery)
{
	MACHINE_START_CALL_MEMBER(mcr68);
}


MACHINE_RESET_MEMBER(mcr68_state,zwackery)
{
	/* for the most part all MCR/68k games are the same */
	m_v493_callback = timer_expired_delegate(FUNC(mcr68_state::zwackery_493_callback),this);

	/* vectors are 5 and 6 */
	m_v493_irq_vector = 5;
	m_m6840_irq_vector = 6;
}



/*************************************
 *
 *  Generic MCR interrupt handler
 *
 *************************************/

INTERRUPT_GEN_MEMBER(mcr68_state::mcr68_interrupt)
{
	logerror("--- VBLANK ---\n");

	/* also set a timer to generate the 493 signal at a specific time before the next VBLANK */
	/* the timing of this is crucial for Blasted and Tri-Sports, which check the timing of */
	/* VBLANK and 493 using counter 2 */
	machine().scheduler().timer_set(attotime::from_hz(30) - m_timing_factor, m_v493_callback);
}



/*************************************
 *
 *  MCR/68k interrupt central
 *
 *************************************/

void mcr68_state::update_mcr68_interrupts()
{
	m_maincpu->set_input_line(m_v493_irq_vector, m_v493_irq_state ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(m_m6840_irq_vector, m_m6840_irq_state ? ASSERT_LINE : CLEAR_LINE);
}


TIMER_CALLBACK_MEMBER(mcr68_state::mcr68_493_off_callback)
{
	m_v493_irq_state = 0;
	update_mcr68_interrupts();
}


TIMER_CALLBACK_MEMBER(mcr68_state::mcr68_493_callback)
{
	m_v493_irq_state = 1;
	update_mcr68_interrupts();
	machine().scheduler().timer_set(m_screen->scan_period(), timer_expired_delegate(FUNC(mcr68_state::mcr68_493_off_callback),this));
	logerror("--- (INT1) ---\n");
}


WRITE_LINE_MEMBER( mcr68_state::ptm_irq_w )
{
	m_m6840_irq_state = state;
	update_mcr68_interrupts();
}


/*************************************
 *
 *  Zwackery-specific interfaces
 *
 *************************************/

WRITE8_MEMBER(mcr68_state::zwackery_pia0_w)
{
	/* bit 7 is the watchdog */
	if (!(data & 0x80)) m_watchdog->watchdog_reset();

	/* bits 5 and 6 control hflip/vflip */
	/* bits 3 and 4 control coin counters? */
	/* bits 0, 1 and 2 control meters? */
}


WRITE8_MEMBER(mcr68_state::zwackery_pia1_w)
{
	m_zwackery_sound_data = (data >> 4) & 0x0f;
}


WRITE_LINE_MEMBER(mcr68_state::zwackery_ca2_w)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	m_chip_squeak_deluxe->write(space, 0, (state << 4) | m_zwackery_sound_data);
}


WRITE_LINE_MEMBER(mcr68_state::zwackery_pia_irq)
{
	pia6821_device *pia = machine().device<pia6821_device>("pia0");
	m_v493_irq_state = pia->irq_a_state() | pia->irq_b_state();
	update_mcr68_interrupts();
}


TIMER_CALLBACK_MEMBER(mcr68_state::zwackery_493_off_callback)
{
	pia6821_device *pia = machine().device<pia6821_device>("pia0");
	pia->ca1_w(0);
}


TIMER_CALLBACK_MEMBER(mcr68_state::zwackery_493_callback)
{
	pia6821_device *pia = machine().device<pia6821_device>("pia0");

	pia->ca1_w(1);
	machine().scheduler().timer_set(m_screen->scan_period(), timer_expired_delegate(FUNC(mcr68_state::zwackery_493_off_callback),this));
}
