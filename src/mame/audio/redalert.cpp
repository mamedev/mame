// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

    Irem M27 hardware

    If you have any questions about how this driver works, don't hesitate to
    ask.  - Mike Balfour (mab22@po.cwru.edu)

****************************************************************************/

#include "emu.h"
#include "redalert.h"

#include "cpu/m6800/m6800.h"
#include "cpu/m6502/m6502.h"
#include "machine/rescap.h"
#include "sound/hc55516.h"

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


DEFINE_DEVICE_TYPE(IREM_M37B_AUDIO,        irem_m37b_audio_device,       "irem_m37b_audio",       "Irem M-37B audio")
DEFINE_DEVICE_TYPE(PANTHER_AUDIO,          panther_audio_device,         "panther_audio",         "Irem M-37B audio (Panther)")
DEFINE_DEVICE_TYPE(IREM_M37B_UE17B_AUDIO,  irem_m37b_ue17b_audio_device, "irem_m37b_ue17b_audio", "Irem M-37B/UE-17B audio/voice")
DEFINE_DEVICE_TYPE(DEMONEYE_AUDIO,         demoneye_audio_device,        "demoneye_audio",        "Irem Demoneye-X audio")


irem_m37b_audio_device::irem_m37b_audio_device(const machine_config &config, const char *tag, device_t *owner, uint32_t clock) :
	irem_m37b_audio_device(config, IREM_M37B_AUDIO, tag, owner, clock)
{
}

irem_m37b_audio_device::irem_m37b_audio_device(const machine_config &config, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(config, type, tag, owner, clock),
	m_audiocpu(*this, "audiocpu"),
	m_ay8910(*this, "aysnd"),
	m_soundlatch(*this, "soundlatch")
{
}


panther_audio_device::panther_audio_device(const machine_config &config, const char *tag, device_t *owner, uint32_t clock) :
	irem_m37b_audio_device(config, PANTHER_AUDIO, tag, owner, clock)
{
}


irem_m37b_ue17b_audio_device::irem_m37b_ue17b_audio_device(const machine_config &config, const char *tag, device_t *owner, uint32_t clock) :
	irem_m37b_audio_device(config, IREM_M37B_UE17B_AUDIO, tag, owner, clock),
	m_voicecpu(*this, "voice"),
	m_soundlatch2(*this, "soundlatch2")
{
}


demoneye_audio_device::demoneye_audio_device(const machine_config &config, const char *tag, device_t *owner, uint32_t clock) :
	device_t(config, DEMONEYE_AUDIO, tag, owner, clock),
	m_audiocpu(*this, "audiocpu"),
	m_ay(*this, "ay%u", 1U),
	m_sndpia(*this, "sndpia"),
	m_soundlatch(*this, "soundlatch")
{
}


TIMER_CALLBACK_MEMBER(irem_m37b_audio_device::audio_irq_on)
{
	m_audiocpu->set_input_line(m6502_device::IRQ_LINE, ASSERT_LINE);

	m_audio_irq_off_timer->adjust(REDALERT_AUDIO_CPU_IRQ_TIME);
}

TIMER_CALLBACK_MEMBER(irem_m37b_audio_device::audio_irq_off)
{
	m_audiocpu->set_input_line(m6502_device::IRQ_LINE, CLEAR_LINE);
}


TIMER_CALLBACK_MEMBER(demoneye_audio_device::audio_irq_on)
{
	m_sndpia->cb1_w(0); // guess

	m_audio_irq_off_timer->adjust(REDALERT_AUDIO_CPU_IRQ_TIME);
}

TIMER_CALLBACK_MEMBER(demoneye_audio_device::audio_irq_off)
{
	m_sndpia->cb1_w(1); // guess
}


/*************************************
 *
 *  Red Alert analog sounds
 *
 *************************************/

void irem_m37b_audio_device::analog_w(uint8_t data)
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

READ_LINE_MEMBER(irem_m37b_audio_device::sound_status_r)
{
	// communication handshake between host and sound CPU
	// at least Panther uses it, unconfirmed for Red Alert and Demoneye-X
	return m_sound_hs;
}

void irem_m37b_audio_device::audio_command_w(uint8_t data)
{
	/* the byte is connected to port A of the AY8910 */
	m_soundlatch->write(data);
	m_sound_hs = 1;

	/* D7 is also connected to the NMI input of the CPU -
	   the NMI is actually toggled by a 74121 (R1=27K, C10=330p) */
	if ((data & 0x80) == 0x00)
		m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


void irem_m37b_audio_device::ay8910_w(uint8_t data)
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


uint8_t irem_m37b_audio_device::ay8910_latch_1_r()
{
	return m_ay8910_latch_1;
}


void irem_m37b_audio_device::ay8910_latch_2_w(uint8_t data)
{
	m_ay8910_latch_2 = data;
}

void irem_m37b_audio_device::redalert_audio_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x03ff).mirror(0x0c00).ram();
	map(0x1000, 0x1000).mirror(0x0ffe).nopr().w(FUNC(irem_m37b_audio_device::ay8910_w));
	map(0x1001, 0x1001).mirror(0x0ffe).rw(FUNC(irem_m37b_audio_device::ay8910_latch_1_r), FUNC(irem_m37b_audio_device::ay8910_latch_2_w));
	map(0x2000, 0x6fff).noprw();
	map(0x7000, 0x77ff).mirror(0x0800).rom();
}

void panther_audio_device::panther_audio_map(address_map &map)
{
	redalert_audio_map(map);

	// Panther maps these two to $2000 while Red Alert to $1000, different PAL addressing?
	map(0x1000, 0x1fff).unmaprw();
	map(0x2000, 0x2000).mirror(0x0ffe).nopr().w(FUNC(panther_audio_device::ay8910_w));
	map(0x2001, 0x2001).mirror(0x0ffe).rw(FUNC(panther_audio_device::ay8910_latch_1_r), FUNC(panther_audio_device::ay8910_latch_2_w));
}

/*************************************
 *
 * Red Alert audio board
 *
 *************************************/

void irem_m37b_audio_device::device_start()
{
	m_audio_irq_on_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(irem_m37b_audio_device::audio_irq_on), this));
	m_audio_irq_off_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(irem_m37b_audio_device::audio_irq_off), this));

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

void irem_m37b_ue17b_audio_device::voice_command_w(uint8_t data)
{
	m_soundlatch2->write((data & 0x78) >> 3);
	m_voicecpu->set_input_line(I8085_RST75_LINE, (~data & 0x80) ? ASSERT_LINE : CLEAR_LINE);
}


void irem_m37b_ue17b_audio_device::redalert_voice_map(address_map &map)
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

void irem_m37b_audio_device::device_add_mconfig(machine_config &config)
{
	M6502(config, m_audiocpu, REDALERT_AUDIO_CPU_CLOCK);
	m_audiocpu->set_addrmap(AS_PROGRAM, &irem_m37b_audio_device::redalert_audio_map);

	GENERIC_LATCH_8(config, m_soundlatch);

	SPEAKER(config, "mono").front_center();

	AY8910(config, m_ay8910, REDALERT_AY8910_CLOCK);
	m_ay8910->port_a_read_callback().set(m_soundlatch, FUNC(generic_latch_8_device::read));
	m_ay8910->port_b_write_callback().set(FUNC(irem_m37b_audio_device::analog_w));
	m_ay8910->add_route(0, "mono", 0.50);
	m_ay8910->add_route(1, "mono", 0.50);
	/* channel C is used a noise source and is not connected to a speaker */
}


/*************************************
 *
 *  Red Alert voice board (ue17b)
 *
 *************************************/

void irem_m37b_ue17b_audio_device::device_add_mconfig(machine_config &config)
{
	irem_m37b_audio_device::device_add_mconfig(config);

	I8085A(config, m_voicecpu, REDALERT_VOICE_CPU_CLOCK);
	m_voicecpu->set_addrmap(AS_PROGRAM, &irem_m37b_ue17b_audio_device::redalert_voice_map);
	m_voicecpu->in_sid_func().set("cvsd", FUNC(hc55516_device::clock_state_r));
	m_voicecpu->out_sod_func().set("cvsd", FUNC(hc55516_device::digit_w));

	GENERIC_LATCH_8(config, m_soundlatch2);

	HC55516(config, "cvsd", REDALERT_HC55516_CLOCK).add_route(ALL_OUTPUTS, "mono", 0.50);
}

/*************************************
 *
 *  Red Alert
 *
 *************************************/

void panther_audio_device::device_add_mconfig(machine_config &config)
{
	irem_m37b_audio_device::device_add_mconfig(config);

	m_audiocpu->set_addrmap(AS_PROGRAM, &panther_audio_device::panther_audio_map);
}

/*************************************
 *
 *  Demoneye-X audio board
 *
 *************************************/

void demoneye_audio_device::device_start()
{
	m_audio_irq_on_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(demoneye_audio_device::audio_irq_on), this));
	m_audio_irq_off_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(demoneye_audio_device::audio_irq_off), this));

	m_audio_irq_on_timer->adjust(REDALERT_AUDIO_CPU_IRQ_FREQ, 0, REDALERT_AUDIO_CPU_IRQ_FREQ);

	save_item(NAME(m_ay8910_latch_1));
	save_item(NAME(m_ay8910_latch_2));
}


void demoneye_audio_device::audio_command_w(uint8_t data)
{
	/* the byte is connected to port A of the AY8910 */
	m_soundlatch->write(data);
	m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


void demoneye_audio_device::ay8910_latch_1_w(uint8_t data)
{
	m_ay8910_latch_1 = data;
}


uint8_t demoneye_audio_device::ay8910_latch_2_r()
{
	return m_ay8910_latch_2;
}


void demoneye_audio_device::ay8910_data_w(uint8_t data)
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
			logerror("demoneye_audio_device::ay8910_data_w called with latch %02X  data %02X\n", m_ay8910_latch_1, data);
			break;
	}
}


void demoneye_audio_device::demoneye_audio_map(address_map &map)
{
	map(0x0500, 0x0503).mirror(0xc000).rw(m_sndpia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x2000, 0x3fff).mirror(0xc000).rom();
}



/*************************************
 *
 *  Demoneye-X machine driver
 *
 *************************************/

void demoneye_audio_device::device_add_mconfig(machine_config &config)
{
	M6802(config, m_audiocpu, DEMONEYE_AUDIO_CPU_CLOCK);
	m_audiocpu->set_addrmap(AS_PROGRAM, &demoneye_audio_device::demoneye_audio_map);

	PIA6821(config, m_sndpia);
	m_sndpia->readpa_handler().set(FUNC(demoneye_audio_device::ay8910_latch_2_r));
	m_sndpia->writepa_handler().set(FUNC(demoneye_audio_device::ay8910_data_w));
	m_sndpia->writepb_handler().set(FUNC(demoneye_audio_device::ay8910_latch_1_w));
	m_sndpia->irqb_handler().set_inputline(m_audiocpu, M6802_IRQ_LINE);

	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	AY8910(config, m_ay[0], DEMONEYE_AY8910_CLOCK);
	m_ay[0]->add_route(ALL_OUTPUTS, "mono", 0.50);

	AY8910(config, m_ay[1], DEMONEYE_AY8910_CLOCK);
	m_ay[1]->port_a_read_callback().set(m_soundlatch, FUNC(generic_latch_8_device::read));
	m_ay[1]->add_route(ALL_OUTPUTS, "mono", 0.50);
}
