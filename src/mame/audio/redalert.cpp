// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

    Irem M27 hardware

    If you have any questions about how this driver works, don't hesitate to
    ask.  - Mike Balfour (mab22@po.cwru.edu)

****************************************************************************/

#include "emu.h"
#include "includes/redalert.h"

#include "cpu/m6800/m6800.h"
#include "cpu/m6502/m6502.h"
#include "machine/rescap.h"
#include "speaker.h"



#define REDALERT_AUDIO_PCB_CLOCK    (XTAL(12'500'000))
#define REDALERT_AUDIO_CPU_CLOCK    (REDALERT_AUDIO_PCB_CLOCK / 12)
#define REDALERT_AY8910_CLOCK       (REDALERT_AUDIO_PCB_CLOCK / 6)
#define REDALERT_AUDIO_CPU_IRQ_FREQ (PERIOD_OF_555_ASTABLE(RES_K(120), RES_K(2.7), CAP_U(0.01)))
#define REDALERT_AUDIO_CPU_IRQ_TIME (PERIOD_OF_555_ASTABLE(RES_K(2.7), 0, CAP_U(0.01))) /* 555 discharge time */

#define REDALERT_VOICE_PCB_CLOCK    (XTAL(6'000'000))
#define REDALERT_VOICE_CPU_CLOCK    (REDALERT_VOICE_PCB_CLOCK)
#define REDALERT_HC55516_CLOCK      (REDALERT_VOICE_PCB_CLOCK / 256)

#define DEMONEYE_AUDIO_PCB_CLOCK    (XTAL(3'579'545))
#define DEMONEYE_AUDIO_CPU_CLOCK    (DEMONEYE_AUDIO_PCB_CLOCK)      /* what's the real divisor? */
#define DEMONEYE_AY8910_CLOCK       (DEMONEYE_AUDIO_PCB_CLOCK / 2)  /* what's the real divisor? */


TIMER_CALLBACK_MEMBER(redalert_state::audio_irq_on)
{
	if (m_sndpia.found())
		m_sndpia->cb1_w(0); // guess
	else
		m_audiocpu->set_input_line(m6502_device::IRQ_LINE, ASSERT_LINE);

	m_audio_irq_off_timer->adjust(REDALERT_AUDIO_CPU_IRQ_TIME);
}


TIMER_CALLBACK_MEMBER(redalert_state::audio_irq_off)
{
	if (m_sndpia.found())
		m_sndpia->cb1_w(1); // guess
	else
		m_audiocpu->set_input_line(m6502_device::IRQ_LINE, CLEAR_LINE);
}


/*************************************
 *
 *  Red Alert analog sounds
 *
 *************************************/

void redalert_state::redalert_analog_w(uint8_t data)
{
	/* this port triggers analog sounds
	   D0 = Formation Aircraft?
	   D1 = Dive bombers?
	   D2 = Helicopters?
	   D3 = Launcher firing?
	   D4 = Explosion #1?
	   D5 = Explosion #2?
	   D6 = Explosion #3? */

	logerror("Analog: %02X\n",data);
}



/*************************************
 *
 *  Red Alert audio board
 *
 *************************************/

void redalert_state::redalert_audio_command_w(uint8_t data)
{
	/* the byte is connected to port A of the AY8910 */
	m_soundlatch->write(data);
	m_sound_hs = 1;

	/* D7 is also connected to the NMI input of the CPU -
	   the NMI is actually toggled by a 74121 (R1=27K, C10=330p) */
	if ((data & 0x80) == 0x00)
		m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


void redalert_state::redalert_AY8910_w(uint8_t data)
{
	/* BC2 is connected to a pull-up resistor, so BC2=1 always */
	switch (data & 0x03)
	{
		/* BC1=0, BDIR=0 : inactive */
		case 0x00:
			break;

		/* BC1=1, BDIR=0 : read from PSG */
		case 0x01:
			m_sound_hs = 0;
			m_ay8910_latch_1 = m_ay8910->data_r();
			break;

		/* BC1=0, BDIR=1 : write to PSG */
		/* BC1=1, BDIR=1 : latch address */
		case 0x02:
		case 0x03:
		default:
			m_ay8910->data_address_w(data, m_ay8910_latch_2);
			break;
	}
}


uint8_t redalert_state::redalert_ay8910_latch_1_r()
{
	return m_ay8910_latch_1;
}


void redalert_state::redalert_ay8910_latch_2_w(uint8_t data)
{
	m_ay8910_latch_2 = data;
}

void redalert_state::redalert_audio_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x03ff).mirror(0x0c00).ram();
	map(0x1000, 0x1000).mirror(0x0ffe).nopr().w(FUNC(redalert_state::redalert_AY8910_w));
	map(0x1001, 0x1001).mirror(0x0ffe).rw(FUNC(redalert_state::redalert_ay8910_latch_1_r), FUNC(redalert_state::redalert_ay8910_latch_2_w));
	map(0x2000, 0x6fff).noprw();
	map(0x7000, 0x77ff).mirror(0x0800).rom();
}

void redalert_state::panther_audio_map(address_map &map)
{
	redalert_audio_map(map);
	// Panther maps these two to $2000 while Red Alert to $1000, different PAL addressing?
	map(0x1000, 0x1fff).unmaprw();
	map(0x2000, 0x2000).mirror(0x0ffe).nopr().w(FUNC(redalert_state::redalert_AY8910_w));
	map(0x2001, 0x2001).mirror(0x0ffe).rw(FUNC(redalert_state::redalert_ay8910_latch_1_r), FUNC(redalert_state::redalert_ay8910_latch_2_w));
}

/*************************************
 *
 * Red Alert audio board
 *
 *************************************/

void redalert_state::sound_start()
{
	m_audio_irq_on_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(redalert_state::audio_irq_on), this));
	m_audio_irq_off_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(redalert_state::audio_irq_off), this));

	m_audio_irq_on_timer->adjust(REDALERT_AUDIO_CPU_IRQ_FREQ, 0, REDALERT_AUDIO_CPU_IRQ_FREQ);

	save_item(NAME(m_sound_hs));
	save_item(NAME(m_ay8910_latch_1));
	save_item(NAME(m_ay8910_latch_2));
}


/*************************************
 *
 * Red Alert voice board
 *
 *************************************/

void redalert_state::redalert_voice_command_w(uint8_t data)
{
	m_soundlatch2->write((data & 0x78) >> 3);
	m_voicecpu->set_input_line(I8085_RST75_LINE, (~data & 0x80) ? ASSERT_LINE : CLEAR_LINE);
}


WRITE_LINE_MEMBER(redalert_state::sod_callback)
{
	m_cvsd->digit_w(state);
}


READ_LINE_MEMBER(redalert_state::sid_callback)
{
	return m_cvsd->clock_state_r();
}


void redalert_state::redalert_voice_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).noprw();
	map(0x8000, 0x83ff).mirror(0x3c00).ram();
	map(0xc000, 0xc000).mirror(0x3fff).r(m_soundlatch2, FUNC(generic_latch_8_device::read)).nopw();
}



/*************************************
 *
 *  Red Alert audio board (m37b)
 *
 *************************************/

void redalert_state::redalert_audio_m37b(machine_config &config)
{
	M6502(config, m_audiocpu, REDALERT_AUDIO_CPU_CLOCK);
	m_audiocpu->set_addrmap(AS_PROGRAM, &redalert_state::redalert_audio_map);

	GENERIC_LATCH_8(config, m_soundlatch);

	AY8910(config, m_ay8910, REDALERT_AY8910_CLOCK);
	m_ay8910->port_a_read_callback().set(m_soundlatch, FUNC(generic_latch_8_device::read));
	m_ay8910->port_b_write_callback().set(FUNC(redalert_state::redalert_analog_w));
	m_ay8910->add_route(0, "mono", 0.50);
	m_ay8910->add_route(1, "mono", 0.50);
	/* channel C is used a noise source and is not connected to a speaker */
}

/*************************************
 *
 *  Red Alert voice board (ue17b)
 *
 *************************************/

void redalert_state::redalert_audio_voice(machine_config &config)
{
	I8085A(config, m_voicecpu, REDALERT_VOICE_CPU_CLOCK);
	m_voicecpu->set_addrmap(AS_PROGRAM, &redalert_state::redalert_voice_map);
	m_voicecpu->in_sid_func().set(FUNC(redalert_state::sid_callback));
	m_voicecpu->out_sod_func().set(FUNC(redalert_state::sod_callback));

	GENERIC_LATCH_8(config, m_soundlatch2);

	HC55516(config, m_cvsd, REDALERT_HC55516_CLOCK).add_route(ALL_OUTPUTS, "mono", 0.50);
}

/*************************************
 *
 *  Red Alert
 *
 *************************************/

void redalert_state::redalert_audio(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	redalert_audio_m37b(config);
	redalert_audio_voice(config);
}

/*************************************
 *
 *  Red Alert
 *
 *************************************/

void redalert_state::ww3_audio(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	redalert_audio_m37b(config);
}

void redalert_state::panther_audio(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	redalert_audio_m37b(config);
	m_audiocpu->set_addrmap(AS_PROGRAM, &redalert_state::panther_audio_map);
}

/*************************************
 *
 *  Demoneye-X audio board
 *
 *************************************/


void redalert_state::demoneye_audio_command_w(uint8_t data)
{
	/* the byte is connected to port A of the AY8910 */
	m_soundlatch->write(data);
	m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


void redalert_state::demoneye_ay8910_latch_1_w(uint8_t data)
{
	m_ay8910_latch_1 = data;
}


uint8_t redalert_state::demoneye_ay8910_latch_2_r()
{
	return m_ay8910_latch_2;
}


void redalert_state::demoneye_ay8910_data_w(uint8_t data)
{
	switch (m_ay8910_latch_1 & 0x03)
	{
		case 0x00:
			if (m_ay8910_latch_1 & 0x10)
				m_ay[0]->data_w(data);

			if (m_ay8910_latch_1 & 0x20)
				m_ay[1]->data_w(data);

			break;

		case 0x01:
			if (m_ay8910_latch_1 & 0x10)
				m_ay8910_latch_2 = m_ay[0]->data_r();

			if (m_ay8910_latch_1 & 0x20)
				m_ay8910_latch_2 = m_ay[1]->data_r();

			break;

		case 0x03:
			if (m_ay8910_latch_1 & 0x10)
				m_ay[0]->address_w(data);

			if (m_ay8910_latch_1 & 0x20)
				m_ay[1]->address_w(data);

			break;

		default:
			logerror("demoneye_ay8910_data_w called with latch %02X  data %02X\n", m_ay8910_latch_1, data);
			break;
	}
}


void redalert_state::demoneye_audio_map(address_map &map)
{
	map(0x0500, 0x0503).mirror(0xc000).rw(m_sndpia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x2000, 0x3fff).mirror(0xc000).rom();
}



/*************************************
 *
 *  Demoneye-X machine driver
 *
 *************************************/

void redalert_state::demoneye_audio(machine_config &config)
{
	M6802(config, m_audiocpu, DEMONEYE_AUDIO_CPU_CLOCK);
	m_audiocpu->set_addrmap(AS_PROGRAM, &redalert_state::demoneye_audio_map);

	PIA6821(config, m_sndpia);
	m_sndpia->readpa_handler().set(FUNC(redalert_state::demoneye_ay8910_latch_2_r));
	m_sndpia->writepa_handler().set(FUNC(redalert_state::demoneye_ay8910_data_w));
	m_sndpia->writepb_handler().set(FUNC(redalert_state::demoneye_ay8910_latch_1_w));
	m_sndpia->irqb_handler().set_inputline(m_audiocpu, M6802_IRQ_LINE);

	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	AY8910(config, m_ay[0], DEMONEYE_AY8910_CLOCK);
	m_ay[0]->add_route(ALL_OUTPUTS, "mono", 0.50);

	AY8910(config, m_ay[1], DEMONEYE_AY8910_CLOCK);
	m_ay[1]->port_a_read_callback().set(m_soundlatch, FUNC(generic_latch_8_device::read));
	m_ay[1]->add_route(ALL_OUTPUTS, "mono", 0.50);
}
