// license:BSD-3-Clause
// copyright-holders:Dan Boris
/***************************************************************************

    Atari Return of the Jedi hardware

    driver by Dan Boris

***************************************************************************/

#include "emu.h"
#include "includes/jedi.h"
#include "cpu/m6502/m6502.h"
#include "sound/pokey.h"
#include "speaker.h"



/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

WRITE8_MEMBER(jedi_state::irq_ack_w)
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


READ8_MEMBER(jedi_state::audio_comm_stat_r)
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

WRITE8_MEMBER(jedi_state::speech_strobe_w)
{
	m_tms->wsq_w(BIT(offset, 8));
}


READ8_MEMBER(jedi_state::speech_ready_r)
{
	return m_tms->readyq_r() << 7;
}


WRITE8_MEMBER(jedi_state::speech_reset_w)
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

MACHINE_CONFIG_START(jedi_state::jedi_audio)

	MCFG_DEVICE_ADD("audiocpu", M6502, JEDI_AUDIO_CPU_CLOCK)
	MCFG_DEVICE_PROGRAM_MAP(audio_map)

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	MCFG_DEVICE_ADD("pokey1", POKEY, JEDI_POKEY_CLOCK)
	MCFG_POKEY_OUTPUT_OPAMP(RES_K(1), 0.0, 5.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.30)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.30)

	MCFG_DEVICE_ADD("pokey2", POKEY, JEDI_POKEY_CLOCK)
	MCFG_POKEY_OUTPUT_OPAMP(RES_K(1), 0.0, 5.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.30)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.30)

	MCFG_DEVICE_ADD("pokey3", POKEY, JEDI_POKEY_CLOCK)
	MCFG_POKEY_OUTPUT_OPAMP(RES_K(1), 0.0, 5.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.30)

	MCFG_DEVICE_ADD("pokey4", POKEY, JEDI_POKEY_CLOCK)
	MCFG_POKEY_OUTPUT_OPAMP(RES_K(1), 0.0, 5.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.30)

	MCFG_DEVICE_ADD("tms", TMS5220, JEDI_TMS5220_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MCFG_GENERIC_LATCH_8_ADD("soundlatch") // 5E (LS374) + 3E (LS279) pins 13-15
	MCFG_GENERIC_LATCH_8_ADD("sacklatch") // 4E (LS374) + 3E (LS279) pins 1-4
MACHINE_CONFIG_END
