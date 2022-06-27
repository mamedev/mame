// license:BSD-3-Clause
// copyright-holders:Dan Boris
/***************************************************************************

    Atari Return of the Jedi hardware

    driver by Dan Boris

***************************************************************************/

#include "emu.h"
#include "jedi.h"
#include "cpu/m6502/m6502.h"
#include "machine/rescap.h"
#include "sound/pokey.h"
#include "speaker.h"



/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

void jedi_state::irq_ack_w(u8 data)
{
	m_audiocpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
}



/*************************************
 *
 *  Main CPU -> Sound CPU communications
 *
 *************************************/

WRITE_LINE_MEMBER(jedi_state::audio_reset_w)
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, state ? CLEAR_LINE : ASSERT_LINE);
	if (!state)
		m_tms->set_output_gain(ALL_OUTPUTS, 0.0);
}


u8 jedi_state::audio_comm_stat_r()
{
	return (m_soundlatch->pending_r() << 7) | (m_sacklatch->pending_r() << 6);
}


CUSTOM_INPUT_MEMBER(jedi_state::jedi_audio_comm_stat_r)
{
	return (m_soundlatch->pending_r() << 1) | m_sacklatch->pending_r();
}



/*************************************
 *
 *  Speech access
 *
 *************************************/

void jedi_state::speech_strobe_w(offs_t offset, u8 data)
{
	m_tms->wsq_w(BIT(offset, 8));
}


u8 jedi_state::speech_ready_r()
{
	return m_tms->readyq_r() << 7;
}


void jedi_state::speech_reset_w(u8 data)
{
	// Flip-flop at 8C controls the power supply to the TMS5220 (through transistors Q6 and Q7)
	m_tms->set_output_gain(ALL_OUTPUTS, BIT(data, 0) ? 1.0 : 0.0);
}



/*************************************
 *
 *  Audio CPU memory handlers
 *
 *************************************/

void jedi_state::audio_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x0800, 0x080f).mirror(0x07c0).rw("pokey1", FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x0810, 0x081f).mirror(0x07c0).rw("pokey2", FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x0820, 0x082f).mirror(0x07c0).rw("pokey3", FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x0830, 0x083f).mirror(0x07c0).rw("pokey4", FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x1000, 0x1000).mirror(0x00ff).nopr().w(FUNC(jedi_state::irq_ack_w));
	map(0x1100, 0x1100).mirror(0x00ff).nopr().w("tms", FUNC(tms5220_device::data_w));
	map(0x1200, 0x13ff).nopr().w(FUNC(jedi_state::speech_strobe_w));
	map(0x1400, 0x1400).mirror(0x00ff).nopr().w("sacklatch", FUNC(generic_latch_8_device::write));
	map(0x1500, 0x1500).mirror(0x00ff).nopr().w(FUNC(jedi_state::speech_reset_w));
	map(0x1600, 0x17ff).noprw();
	map(0x1800, 0x1800).mirror(0x03ff).r("soundlatch", FUNC(generic_latch_8_device::read)).nopw();
	map(0x1c00, 0x1c00).mirror(0x03fe).r(FUNC(jedi_state::speech_ready_r)).nopw();
	map(0x1c01, 0x1c01).mirror(0x03fe).r(FUNC(jedi_state::audio_comm_stat_r)).nopw();
	map(0x2000, 0x7fff).noprw();
	map(0x8000, 0xffff).rom();
}



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void jedi_state::jedi_audio(machine_config &config)
{
	M6502(config, m_audiocpu, JEDI_AUDIO_CPU_CLOCK);
	m_audiocpu->set_addrmap(AS_PROGRAM, &jedi_state::audio_map);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	pokey_device &pokey1(POKEY(config, "pokey1", JEDI_POKEY_CLOCK));
	pokey1.set_output_opamp(RES_K(1), 0.0, 5.0);
	pokey1.add_route(ALL_OUTPUTS, "lspeaker", 0.30);
	pokey1.add_route(ALL_OUTPUTS, "rspeaker", 0.30);

	pokey_device &pokey2(POKEY(config, "pokey2", JEDI_POKEY_CLOCK));
	pokey2.set_output_opamp(RES_K(1), 0.0, 5.0);
	pokey2.add_route(ALL_OUTPUTS, "lspeaker", 0.30);
	pokey2.add_route(ALL_OUTPUTS, "rspeaker", 0.30);

	pokey_device &pokey3(POKEY(config, "pokey3", JEDI_POKEY_CLOCK));
	pokey3.set_output_opamp(RES_K(1), 0.0, 5.0);
	pokey3.add_route(ALL_OUTPUTS, "lspeaker", 0.30);

	pokey_device &pokey4(POKEY(config, "pokey4", JEDI_POKEY_CLOCK));
	pokey4.set_output_opamp(RES_K(1), 0.0, 5.0);
	pokey4.add_route(ALL_OUTPUTS, "rspeaker", 0.30);

	TMS5220(config, m_tms, JEDI_TMS5220_CLOCK);
	m_tms->add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	m_tms->add_route(ALL_OUTPUTS, "rspeaker", 1.0);

	GENERIC_LATCH_8(config, m_soundlatch); // 5E (LS374) + 3E (LS279) pins 13-15
	GENERIC_LATCH_8(config, m_sacklatch); // 4E (LS374) + 3E (LS279) pins 1-4
}
