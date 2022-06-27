// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    segaspeech.cpp

    Sega Speech board for early G-80 games.

***************************************************************************/

#include "emu.h"
#include "segaspeech.h"

#include "sound/sp0250.h"

#include "netlist/nl_setup.h"

#include "nl_segaspeech.h"

#define VERBOSE 0
#include "logmacro.h"

#define ENABLE_NETLIST_FILTERING (0)


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define SPEECH_MASTER_CLOCK 3120000



/***************************************************************************
    SPEECH BOARD
***************************************************************************/

DEFINE_DEVICE_TYPE(SEGA_SPEECH_BOARD, sega_speech_device, "sega_speech_device", "Sega Speech Sound Board")

sega_speech_device::sega_speech_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SEGA_SPEECH_BOARD, tag, owner, clock),
	device_mixer_interface(mconfig, *this),
	m_speech(*this, "data"),
	m_cpu(*this, "cpu"),
	m_drq(0),
	m_latch(0),
	m_t0(0),
	m_p2(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sega_speech_device::device_start()
{
	save_item(NAME(m_latch));
	save_item(NAME(m_t0));
	save_item(NAME(m_p2));
	save_item(NAME(m_drq));
}


/*************************************
 *
 *  i8035 port accessors
 *
 *************************************/

READ_LINE_MEMBER( sega_speech_device::t0_r )
{
	return m_t0;
}

READ_LINE_MEMBER( sega_speech_device::t1_r )
{
//printf("%s: t1_r=%d\n", machine().scheduler().time().as_string(), m_drq);
	return m_drq;
}

uint8_t sega_speech_device::p1_r()
{
	return m_latch & 0x7f;
}

uint8_t sega_speech_device::rom_r(offs_t offset)
{
	return m_speech->base()[0x100 * (m_p2 & 0x3f) + offset];
}

void sega_speech_device::p1_w(uint8_t data)
{
	if (!(data & 0x80))
		m_t0 = 0;
}

void sega_speech_device::p2_w(uint8_t data)
{
	m_p2 = data;
}



/*************************************
 *
 *  i8035 port accessors
 *
 *************************************/

WRITE_LINE_MEMBER(sega_speech_device::drq_w)
{
//printf("%s: DRQ=%d\n", machine().scheduler().time().as_string(), state);
	m_drq = (state == ASSERT_LINE);
}



/*************************************
 *
 *  External access
 *
 *************************************/

TIMER_CALLBACK_MEMBER( sega_speech_device::delayed_speech_w )
{
	int data = param;
	u8 old = m_latch;

	/* all 8 bits are latched */
	m_latch = data;

	/* the high bit goes directly to the INT line */
	m_cpu->set_input_line(INPUT_LINE_IRQ0, (data & 0x80) ? CLEAR_LINE : ASSERT_LINE);

	/* a clock on the high bit clocks a 1 into T0 */
	if (!(old & 0x80) && (data & 0x80))
		m_t0 = 1;
}


void sega_speech_device::data_w(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(sega_speech_device::delayed_speech_w), this), data);
}


void sega_speech_device::control_w(uint8_t data)
{
	// bit 3 controls channel #1 of a CD4053 to enable/disable
	// speech output
	this->set_input_gain(0, BIT(data, 3) ? 1.0 : 0.0);

	// bit 4 controls channel #2, but the source is unconnected

	// bit 5 controls channel #3, which comes from off-board
	// and has a pot to control the incoming volume; at least on
	// Star Trek, this is the audio from the Universal Sound Board
	this->set_input_gain(1, BIT(data, 5) ? 1.0 : 0.0);

	LOG("Speech control = %X\n", data);
}


/*************************************
 *
 *  Speech board address maps
 *
 *************************************/

void sega_speech_device::speech_map(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x0800).rom();
}


void sega_speech_device::speech_portmap(address_map &map)
{
	map(0x00, 0xff).r(FUNC(sega_speech_device::rom_r));
	map(0x00, 0xff).w("sp0250", FUNC(sp0250_device::write));
}


/*************************************
 *
 *  Speech board machine drivers
 *
 *************************************/

void sega_speech_device::device_add_mconfig(machine_config &config)
{
	// CPU for the speech board
	i8035_device &audiocpu(I8035(config, "cpu", SPEECH_MASTER_CLOCK));    // divide by 15 in CPU
	audiocpu.set_addrmap(AS_PROGRAM, &sega_speech_device::speech_map);
	audiocpu.set_addrmap(AS_IO, &sega_speech_device::speech_portmap);
	audiocpu.p1_in_cb().set(FUNC(sega_speech_device::p1_r));
	audiocpu.p1_out_cb().set(FUNC(sega_speech_device::p1_w));
	audiocpu.p2_out_cb().set(FUNC(sega_speech_device::p2_w));
	audiocpu.t0_in_cb().set(FUNC(sega_speech_device::t0_r));
	audiocpu.t1_in_cb().set(FUNC(sega_speech_device::t1_r));

	// speech chip
	sp0250_device &speech(SP0250(config, "sp0250", SPEECH_MASTER_CLOCK));
	speech.drq().set(FUNC(sega_speech_device::drq_w));
#if (ENABLE_NETLIST_FILTERING)
	speech.set_pwm_mode();
	speech.add_route(ALL_OUTPUTS, "sound_nl", 1.0, 0);

	// netlist filtering
	NETLIST_SOUND(config, "sound_nl", SPEECH_MASTER_CLOCK/2)
		.set_source(NETLIST_NAME(segaspeech))
		.add_route(ALL_OUTPUTS, *this, 1.0);

	NETLIST_STREAM_INPUT(config, "sound_nl:cin0", 0, "I_SP0250.IN").set_mult_offset(1.0 / 32767.0, 0);

	NETLIST_STREAM_OUTPUT(config, "sound_nl:cout0", 0, "OUTPUT").set_mult_offset(75000.0, 0.0);
#else
	speech.add_route(ALL_OUTPUTS, *this, 1.0);
#endif
}
