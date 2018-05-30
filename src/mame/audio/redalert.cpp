// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

    Irem Red Alert hardware

    If you have any questions about how this driver works, don't hesitate to
    ask.  - Mike Balfour (mab22@po.cwru.edu)

****************************************************************************/

#include "emu.h"
#include "includes/redalert.h"

#include "cpu/i8085/i8085.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6502/m6502.h"
#include "machine/rescap.h"
#include "machine/6821pia.h"
#include "sound/ay8910.h"
#include "speaker.h"



#define REDALERT_AUDIO_PCB_CLOCK    (XTAL(12'500'000))
#define REDALERT_AUDIO_CPU_CLOCK    (REDALERT_AUDIO_PCB_CLOCK / 12)
#define REDALERT_AY8910_CLOCK       (REDALERT_AUDIO_PCB_CLOCK / 6)
#define REDALERT_AUDIO_CPU_IRQ_FREQ (1000000000.0 / PERIOD_OF_555_ASTABLE_NSEC(RES_K(120), RES_K(2.7), CAP_U(0.01)))

#define REDALERT_VOICE_PCB_CLOCK    (XTAL(6'000'000))
#define REDALERT_VOICE_CPU_CLOCK    (REDALERT_VOICE_PCB_CLOCK)
#define REDALERT_HC55516_CLOCK      (REDALERT_VOICE_PCB_CLOCK / 256)

#define DEMONEYE_AUDIO_PCB_CLOCK    (XTAL(3'579'545))
#define DEMONEYE_AUDIO_CPU_CLOCK    (DEMONEYE_AUDIO_PCB_CLOCK / 4)  /* what's the real divisor? */
#define DEMONEYE_AY8910_CLOCK       (DEMONEYE_AUDIO_PCB_CLOCK / 2)  /* what's the real divisor? */



/*************************************
 *
 *  Read Alert analog sounds
 *
 *************************************/

WRITE8_MEMBER(redalert_state::redalert_analog_w)
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

WRITE8_MEMBER(redalert_state::redalert_audio_command_w)
{
	/* the byte is connected to port A of the AY8910 */
	m_soundlatch->write(space, 0, data);

	/* D7 is also connected to the NMI input of the CPU -
	   the NMI is actually toggled by a 74121 (R1=27K, C10=330p) */
	if ((data & 0x80) == 0x00)
		m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


WRITE8_MEMBER(redalert_state::redalert_AY8910_w)
{
	ay8910_device *ay8910 = machine().device<ay8910_device>("aysnd");
	/* BC2 is connected to a pull-up resistor, so BC2=1 always */
	switch (data & 0x03)
	{
		/* BC1=0, BDIR=0 : inactive */
		case 0x00:
			break;

		/* BC1=1, BDIR=0 : read from PSG */
		case 0x01:
			m_ay8910_latch_1 = ay8910->data_r(space, 0);
			break;

		/* BC1=0, BDIR=1 : write to PSG */
		/* BC1=1, BDIR=1 : latch address */
		case 0x02:
		case 0x03:
		default:
			ay8910->data_address_w(space, data, m_ay8910_latch_2);
			break;
	}
}


READ8_MEMBER(redalert_state::redalert_ay8910_latch_1_r)
{
	return m_ay8910_latch_1;
}


WRITE8_MEMBER(redalert_state::redalert_ay8910_latch_2_w)
{
	m_ay8910_latch_2 = data;
}

void redalert_state::redalert_audio_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x03ff).mirror(0x0c00).ram();
	map(0x1000, 0x1000).mirror(0x0ffe).nopr().w(this, FUNC(redalert_state::redalert_AY8910_w));
	map(0x1001, 0x1001).mirror(0x0ffe).rw(this, FUNC(redalert_state::redalert_ay8910_latch_1_r), FUNC(redalert_state::redalert_ay8910_latch_2_w));
	map(0x2000, 0x6fff).noprw();
	map(0x7000, 0x77ff).mirror(0x0800).rom();
}

/*************************************
 *
 * Red Alert audio board
 *
 *************************************/

void redalert_state::sound_start()
{
	save_item(NAME(m_ay8910_latch_1));
	save_item(NAME(m_ay8910_latch_2));
}


/*************************************
 *
 * Red Alert voice board
 *
 *************************************/

WRITE8_MEMBER(redalert_state::redalert_voice_command_w)
{
	m_soundlatch2->write(space, 0, (data & 0x78) >> 3);
	machine().device("voice")->execute().set_input_line(I8085_RST75_LINE, (~data & 0x80) ? ASSERT_LINE : CLEAR_LINE);
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

MACHINE_CONFIG_START(redalert_state::redalert_audio_m37b)

	MCFG_DEVICE_ADD("audiocpu", M6502, REDALERT_AUDIO_CPU_CLOCK)
	MCFG_DEVICE_PROGRAM_MAP(redalert_audio_map)
	MCFG_DEVICE_PERIODIC_INT_DRIVER(redalert_state, irq0_line_hold,  REDALERT_AUDIO_CPU_IRQ_FREQ)

	MCFG_GENERIC_LATCH_8_ADD("soundlatch")

	MCFG_DEVICE_ADD("aysnd", AY8910, REDALERT_AY8910_CLOCK)
	MCFG_AY8910_PORT_A_READ_CB(READ8("soundlatch", generic_latch_8_device, read))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(*this, redalert_state, redalert_analog_w))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.50)
	/* channel C is used a noise source and is not connected to a speaker */

MACHINE_CONFIG_END

/*************************************
 *
 *  Red Alert voice board (ue17b)
 *
 *************************************/

MACHINE_CONFIG_START(redalert_state::redalert_audio_voice)

	MCFG_DEVICE_ADD("voice", I8085A, REDALERT_VOICE_CPU_CLOCK)
	MCFG_DEVICE_PROGRAM_MAP(redalert_voice_map)
	MCFG_I8085A_SID(READLINE(*this, redalert_state,sid_callback))
	MCFG_I8085A_SOD(WRITELINE(*this, redalert_state,sod_callback))

	MCFG_GENERIC_LATCH_8_ADD("soundlatch2")

	MCFG_DEVICE_ADD("cvsd", HC55516, REDALERT_HC55516_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

/*************************************
 *
 *  Red Alert
 *
 *************************************/

MACHINE_CONFIG_START(redalert_state::redalert_audio)

	SPEAKER(config, "mono").front_center();

	redalert_audio_m37b(config);
	redalert_audio_voice(config);

MACHINE_CONFIG_END

/*************************************
 *
 *  Red Alert
 *
 *************************************/

MACHINE_CONFIG_START(redalert_state::ww3_audio)

	SPEAKER(config, "mono").front_center();

	redalert_audio_m37b(config);

MACHINE_CONFIG_END

/*************************************
 *
 *  Demoneye-X audio board
 *
 *************************************/


WRITE8_MEMBER(redalert_state::demoneye_audio_command_w)
{
	/* the byte is connected to port A of the AY8910 */
	m_soundlatch->write(space, 0, data);
	m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


WRITE8_MEMBER(redalert_state::demoneye_ay8910_latch_1_w)
{
	m_ay8910_latch_1 = data;
}


READ8_MEMBER(redalert_state::demoneye_ay8910_latch_2_r)
{
	return m_ay8910_latch_2;
}


WRITE8_MEMBER(redalert_state::demoneye_ay8910_data_w)
{
	ay8910_device *ay1 = machine().device<ay8910_device>("ay1");
	ay8910_device *ay2 = machine().device<ay8910_device>("ay2");

	switch (m_ay8910_latch_1 & 0x03)
	{
		case 0x00:
			if (m_ay8910_latch_1 & 0x10)
				ay1->data_w(space, 0, data);

			if (m_ay8910_latch_1 & 0x20)
				ay2->data_w(space, 0, data);

			break;

		case 0x01:
			if (m_ay8910_latch_1 & 0x10)
				m_ay8910_latch_2 = ay1->data_r(space, 0);

			if (m_ay8910_latch_1 & 0x20)
				m_ay8910_latch_2 = ay2->data_r(space, 0);

			break;

		case 0x03:
			if (m_ay8910_latch_1 & 0x10)
				ay1->address_w(space, 0, data);

			if (m_ay8910_latch_1 & 0x20)
				ay2->address_w(space, 0, data);

			break;

		default:
			logerror("demoneye_ay8910_data_w called with latch %02X  data %02X\n", m_ay8910_latch_1, data);
			break;
	}
}


void redalert_state::demoneye_audio_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x007f).ram();
	map(0x0500, 0x0503).rw("sndpia", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x2000, 0x3fff).rom();
}



/*************************************
 *
 *  Demoneye-X machine driver
 *
 *************************************/

MACHINE_CONFIG_START(redalert_state::demoneye_audio)

	MCFG_DEVICE_ADD("audiocpu", M6802, DEMONEYE_AUDIO_CPU_CLOCK)
	MCFG_DEVICE_PROGRAM_MAP(demoneye_audio_map)
	MCFG_DEVICE_PERIODIC_INT_DRIVER(redalert_state, irq0_line_hold,  REDALERT_AUDIO_CPU_IRQ_FREQ)  /* guess */

	MCFG_DEVICE_ADD("sndpia", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(*this, redalert_state, demoneye_ay8910_latch_2_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(*this, redalert_state, demoneye_ay8910_data_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(*this, redalert_state, demoneye_ay8910_latch_1_w))

	SPEAKER(config, "mono").front_center();

	MCFG_GENERIC_LATCH_8_ADD("soundlatch")

	MCFG_DEVICE_ADD("ay1", AY8910, DEMONEYE_AY8910_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DEVICE_ADD("ay2", AY8910, DEMONEYE_AY8910_CLOCK)
	MCFG_AY8910_PORT_A_READ_CB(READ8("soundlatch", generic_latch_8_device, read))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END
