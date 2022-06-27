// license:BSD-3-Clause
// copyright-holders:Derrick Renaud,Couriersud
/*************************************************************************

    audio/qix.cpp

*************************************************************************/

#include "emu.h"
#include "includes/qix.h"

#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "sound/discrete.h"
#include "sound/sn76496.h"
#include "speaker.h"



/* Discrete Sound Input Nodes */
#define QIX_DAC_DATA        NODE_01
#define QIX_VOL_DATA        NODE_02
#define QIX_VOL_DATA_L      NODE_03
#define QIX_VOL_DATA_R      NODE_04



/***************************************************************************
Audio handlers
***************************************************************************/

void qix_state::qix_dac_w(uint8_t data)
{
	m_discrete->write(QIX_DAC_DATA, data);
}

void qix_state::qix_vol_w(uint8_t data)
{
	m_discrete->write(QIX_VOL_DATA, data);
}


/************************************************************************/
/* qix Sound System Analog emulation                                    */
/************************************************************************/
/*
 * This hardware is capable of independent L/R volume control,
 * but only sdungeon uses it for a stereo effect.
 * Other games just use it for fixed L/R volume control.
 *
 * This is such a basic sound system that there is only one effect.
 * So I won't bother keeping proper voltages levels, and will just
 * start with the final gain.
 */

static const discrete_comp_adder_table qix_attn_table =
{
	DISC_COMP_P_RESISTOR, 0, 4,
	{RES_K(22)+250, RES_K(10)+250, RES_K(5.6)+250, RES_K(3.3)+250}
};

static DISCRETE_SOUND_START(qix_discrete)
	/*                    NODE                      */
	DISCRETE_INPUTX_DATA(QIX_DAC_DATA, 128, -128*128, 128)
	DISCRETE_INPUT_DATA (QIX_VOL_DATA)

	/* Separate the two 4-bit channels. */
	DISCRETE_TRANSFORM3(QIX_VOL_DATA_L, QIX_VOL_DATA, 16, 0x0f, "01/2&")
	DISCRETE_TRANSFORM2(QIX_VOL_DATA_R, QIX_VOL_DATA, 0x0f, "01&")

	/* Work out the parallel resistance of the selected resistors. */
	DISCRETE_COMP_ADDER(NODE_10, QIX_VOL_DATA_L, &qix_attn_table)
	DISCRETE_COMP_ADDER(NODE_20, QIX_VOL_DATA_R, &qix_attn_table)

	/* Then use it for the resistor divider network. */
	DISCRETE_TRANSFORM3(NODE_11, NODE_10, RES_K(10), QIX_DAC_DATA, "001+/2*")
	DISCRETE_TRANSFORM3(NODE_21, NODE_20, RES_K(10), QIX_DAC_DATA, "001+/2*")

	/* If no resistors are selected (0), then the volume is full. */
	DISCRETE_SWITCH(NODE_12, 1, QIX_VOL_DATA_L, QIX_DAC_DATA, NODE_11)
	DISCRETE_SWITCH(NODE_22, 1, QIX_VOL_DATA_R, QIX_DAC_DATA, NODE_21)

	/* Filter the DC using the lowest case filter. */
	DISCRETE_CRFILTER(NODE_13, NODE_12, RES_K(1.5), CAP_U(1))
	DISCRETE_CRFILTER(NODE_23, NODE_22, RES_K(1.5), CAP_U(1))

	DISCRETE_OUTPUT(NODE_13, 1)
	DISCRETE_OUTPUT(NODE_23, 1)
DISCRETE_SOUND_END



/*************************************
 *
 *  PIA handlers
 *
 *************************************/

void qix_state::sndpia_2_warning_w(uint8_t data)
{
	// this PIA is supposedly unused
	if (machine().phase() > machine_phase::RESET)
		logerror("PIA 5 write!!\n");
}


TIMER_CALLBACK_MEMBER(qix_state::deferred_sndpia1_porta_w)
{
	m_sndpia1->porta_w(param);
}


void qix_state::sync_sndpia1_porta_w(uint8_t data)
{
	/* we need to synchronize this so the sound CPU doesn't drop anything important */
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(qix_state::deferred_sndpia1_porta_w), this), data);
}


void qix_state::slither_coinctl_w(uint8_t data)
{
	machine().bookkeeping().coin_lockout_w(0, (~data >> 6) & 1);
	machine().bookkeeping().coin_counter_w(0, (data >> 5) & 1);
}



/*************************************
 *
 *  IRQ generation
 *
 *************************************/

WRITE_LINE_MEMBER(qix_state::qix_pia_dint)
{
	int combined_state = m_sndpia0->irq_a_state() | m_sndpia0->irq_b_state();

	/* DINT is connected to the data CPU's IRQ line */
	m_maincpu->set_input_line(M6809_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
}


WRITE_LINE_MEMBER(qix_state::qix_pia_sint)
{
	int combined_state = m_sndpia1->irq_a_state() | m_sndpia1->irq_b_state();

	/* SINT is connected to the sound CPU's IRQ line */
	m_audiocpu->set_input_line(M6802_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  Audio CPU memory handlers
 *
 *************************************/

void qix_state::audio_map(address_map &map)
{
	map(0x2000, 0x2003).mirror(0x5ffc).rw(m_sndpia2, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x4000, 0x4003).mirror(0x3ffc).rw(m_sndpia1, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xd000, 0xffff).rom();
}




/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void qix_state::qix_audio(machine_config &config)
{
	M6802(config, m_audiocpu, SOUND_CLOCK_OSC/2); /* 0.92 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &qix_state::audio_map);

	PIA6821(config, m_sndpia0, 0);
	m_sndpia0->writepa_handler().set(FUNC(qix_state::sync_sndpia1_porta_w));
	m_sndpia0->writepb_handler().set(FUNC(qix_state::qix_vol_w));
	m_sndpia0->ca2_handler().set("sndpia1", FUNC(pia6821_device::ca1_w));
	m_sndpia0->cb2_handler().set(FUNC(qix_state::qix_flip_screen_w));
	m_sndpia0->irqa_handler().set(FUNC(qix_state::qix_pia_dint));
	m_sndpia0->irqb_handler().set(FUNC(qix_state::qix_pia_dint));

	PIA6821(config, m_sndpia1, 0);
	m_sndpia1->writepa_handler().set("sndpia0", FUNC(pia6821_device::porta_w));
	m_sndpia1->writepb_handler().set(FUNC(qix_state::qix_dac_w));
	m_sndpia1->ca2_handler().set("sndpia0", FUNC(pia6821_device::ca1_w));
	m_sndpia1->irqa_handler().set(FUNC(qix_state::qix_pia_sint));
	m_sndpia1->irqb_handler().set(FUNC(qix_state::qix_pia_sint));

	PIA6821(config, m_sndpia2, 0);
	m_sndpia2->writepa_handler().set(FUNC(qix_state::sndpia_2_warning_w));
	m_sndpia2->writepb_handler().set(FUNC(qix_state::sndpia_2_warning_w));
	m_sndpia2->ca2_handler().set(FUNC(qix_state::sndpia_2_warning_w));
	m_sndpia2->cb2_handler().set(FUNC(qix_state::sndpia_2_warning_w));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DISCRETE(config, m_discrete, qix_discrete);
	m_discrete->add_route(0, "lspeaker", 1.0);
	m_discrete->add_route(1, "rspeaker", 1.0);
}

void qix_state::slither_audio(machine_config &config)
{
	PIA6821(config, m_sndpia0, 0);
	m_sndpia0->readpa_handler().set_ioport("P2");
	m_sndpia0->writepb_handler().set(FUNC(qix_state::slither_coinctl_w));
	m_sndpia0->cb2_handler().set(FUNC(qix_state::qix_flip_screen_w));
	m_sndpia0->irqa_handler().set(FUNC(qix_state::qix_pia_dint));
	m_sndpia0->irqb_handler().set(FUNC(qix_state::qix_pia_dint));

	SPEAKER(config, "mono").front_center();

	SN76489(config, m_sn1, SLITHER_CLOCK_OSC/4/4);
	m_sn1->add_route(ALL_OUTPUTS, "mono", 0.50);

	SN76489(config, m_sn2, SLITHER_CLOCK_OSC/4/4);
	m_sn2->add_route(ALL_OUTPUTS, "mono", 0.50);
}
