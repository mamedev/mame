// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    segasnd.cpp

    Sound boards for early Sega G-80 based games.

***************************************************************************/

#include "emu.h"
#include "segasnd.h"

#include "sound/sp0250.h"
#include "includes/segag80r.h"
#include "includes/segag80v.h"


#define VERBOSE 0
#include "logmacro.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define SPEECH_MASTER_CLOCK 3120000



/***************************************************************************
    SPEECH BOARD
***************************************************************************/

DEFINE_DEVICE_TYPE(SEGASPEECH, speech_sound_device, "sega_speech_sound", "Sega Speech Sound Board")

#define SEGASPEECH_REGION "speech"

speech_sound_device::speech_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SEGASPEECH, tag, owner, clock),
		device_sound_interface(mconfig, *this),
		m_int_cb(*this),
		m_speech(*this, SEGASPEECH_REGION),
		m_drq(0),
		m_latch(0),
		m_t0(0),
		m_p2(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void speech_sound_device::device_start()
{
	m_int_cb.resolve();

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



READ_LINE_MEMBER( speech_sound_device::t0_r )
{
	return m_t0;
}

READ_LINE_MEMBER( speech_sound_device::t1_r )
{
	return m_drq;
}

uint8_t speech_sound_device::p1_r()
{
	return m_latch & 0x7f;
}

uint8_t speech_sound_device::rom_r(offs_t offset)
{
	return m_speech->base()[0x100 * (m_p2 & 0x3f) + offset];
}

void speech_sound_device::p1_w(uint8_t data)
{
	if (!(data & 0x80))
		m_t0 = 0;
}

void speech_sound_device::p2_w(uint8_t data)
{
	m_p2 = data;
}



/*************************************
 *
 *  i8035 port accessors
 *
 *************************************/

WRITE_LINE_MEMBER(speech_sound_device::drq_w)
{
	m_drq = (state == ASSERT_LINE);
}



/*************************************
 *
 *  External access
 *
 *************************************/

TIMER_CALLBACK_MEMBER( speech_sound_device::delayed_speech_w )
{
	int data = param;
	u8 old = m_latch;

	/* all 8 bits are latched */
	m_latch = data;

	/* the high bit goes directly to the INT line */
	m_int_cb((data & 0x80) ? CLEAR_LINE : ASSERT_LINE);

	/* a clock on the high bit clocks a 1 into T0 */
	if (!(old & 0x80) && (data & 0x80))
		m_t0 = 1;
}


void speech_sound_device::data_w(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(speech_sound_device::delayed_speech_w), this), data);
}


void speech_sound_device::control_w(uint8_t data)
{
	LOG("Speech control = %X\n", data);
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void speech_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
}

/*************************************
 *
 *  Speech board functions
 *
 *************************************/

WRITE_LINE_MEMBER(segag80snd_common::segaspeech_int_w)
{
	m_audiocpu->set_input_line(0, state);
}

/*************************************
 *
 *  Speech board address maps
 *
 *************************************/

void segag80snd_common::speech_map(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x0800).rom();
}


void segag80snd_common::speech_portmap(address_map &map)
{
	map(0x00, 0xff).r("segaspeech", FUNC(speech_sound_device::rom_r));
	map(0x00, 0xff).w("speech", FUNC(sp0250_device::write));
}


/*************************************
 *
 *  Speech board machine drivers
 *
 *************************************/

void segag80snd_common::sega_speech_board(machine_config &config)
{
	/* CPU for the speech board */
	i8035_device &audiocpu(I8035(config, m_audiocpu, SPEECH_MASTER_CLOCK));        /* divide by 15 in CPU */
	audiocpu.set_addrmap(AS_PROGRAM, &segag80snd_common::speech_map);
	audiocpu.set_addrmap(AS_IO, &segag80snd_common::speech_portmap);
	audiocpu.p1_in_cb().set("segaspeech", FUNC(speech_sound_device::p1_r));
	audiocpu.p1_out_cb().set("segaspeech", FUNC(speech_sound_device::p1_w));
	audiocpu.p2_out_cb().set("segaspeech", FUNC(speech_sound_device::p2_w));
	audiocpu.t0_in_cb().set("segaspeech", FUNC(speech_sound_device::t0_r));
	audiocpu.t1_in_cb().set("segaspeech", FUNC(speech_sound_device::t1_r));

	/* sound hardware */
	SEGASPEECH(config, "segaspeech", 0).int_cb().set(FUNC(segag80snd_common::segaspeech_int_w));
	sp0250_device &speech(SP0250(config, "speech", SPEECH_MASTER_CLOCK));
	speech.drq().set("segaspeech", FUNC(speech_sound_device::drq_w));
	speech.add_route(ALL_OUTPUTS, "speaker", 1.0);
}
