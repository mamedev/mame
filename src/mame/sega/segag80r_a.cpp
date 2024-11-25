// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega G-80 raster hardware

    Across these games, there's a mixture of discrete sound circuitry,
    speech boards, ADPCM samples, and a TMS3617 music chip.

***************************************************************************/

#include "emu.h"
#include "segag80r.h"

#include "segag80r_a.h"
#include "sound/dac.h"

#include "speaker.h"

/*************************************
 *
 *  Constants
 *
 *************************************/

#define SEGA005_555_TIMER_FREQ      (1.44 / ((15000 + 2 * 4700) * 1.5e-6))
#define SEGA005_COUNTER_FREQ        (100000)    /* unknown, just a guess */

DEFINE_DEVICE_TYPE(SEGA005, sega005_sound_device, "sega005_sound", "Sega 005 Custom Sound")

sega005_sound_device::sega005_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SEGA005, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_sega005_sound_timer(nullptr)
	, m_sega005_stream(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sega005_sound_device::device_start()
{
	segag80r_state *state = machine().driver_data<segag80r_state>();

	/* create the stream */
	m_sega005_stream = stream_alloc(0, 1, SEGA005_COUNTER_FREQ);

	/* create a timer for the 555 */
	m_sega005_sound_timer = timer_alloc(FUNC(sega005_sound_device::sega005_auto_timer), this);

	/* set the initial sound data */
	state->m_sound_data = 0x00;
	state->sega005_update_sound_data();
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void sega005_sound_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	segag80r_state *state = machine().driver_data<segag80r_state>();
	const uint8_t *sound_prom = state->memregion("proms")->base();
	int i;

	/* no implementation yet */
	for (i = 0; i < outputs[0].samples(); i++)
	{
		if (!(state->m_sound_state[1] & 0x10) && (++state->m_square_count & 0xff) == 0)
		{
			state->m_square_count = sound_prom[state->m_sound_data & 0x1f];

			/* hack - the RC should filter this out */
			if (state->m_square_count != 0xff)
				state->m_square_state += 2;
		}

		outputs[0].put(i, (state->m_square_state & 2) ? 1.0 : 0.0);
	}
}





/*************************************
 *
 *  005 sound hardware
 *
 *************************************/

/*
    005

    The Sound Board consists of the following:

    An 8255:
        Port A controls the sounds that use discrete circuitry
            A0 - Large Expl. Sound Trig
            A1 - Small Expl. Sound Trig
            A2 - Drop Sound Bomb Trig
            A3 - Shoot Sound Pistol Trig
            A4 - Missile Sound Trig
            A5 - Helicopter Sound Trig
            A6 - Whistle Sound Trig
            A7 - <unused>

      Port B controls the melody generator (described below)

      Port C is apparently unused


    Melody Generator:

        555 timer frequency = 1.44/((R1 + 2R2)*C)
        R1 = 15e3
        R2 = 4.7e3
        C=1.5e-6
        Frequency = 39.344 Hz

        Auto timer is enabled if port B & 0x20 == 1
        Auto timer is reset if 2716 value & 0x20 == 0

        Manual timer is enabled if port B & 0x20 == 0
        Manual timer is clocked if port B & 0x40 goes from 0 to 1

        Both auto and manual timers clock LS393 counter
        Counter is held to 0 if port B & 0x10 == 1

        Output of LS393 >> 1 selects low 7 bits of lookup in 2716.
        High 4 bits come from port B bits 0-3.

        Low 5 bits of output from 2716 look up value in 6331 PROM at U8 (32x8)

        8-bit output of 6331 at U8 is loaded into pair of LS161 counters whenever they overflow.
        LS161 counters are clocked somehow (not clear how)

        Carry output from LS161 counters (overflowing 8 bits) goes to the B
            input on the LS293 counter at U14.
        Rising edge of B input clocks bit 1 of counter (effectively adding 2).
        Output B (bit 1) is mixed with output D (bit 3) with different weights
            through a small RC circuit and fed into the 4391 input at U32.

        The 4391 output is the final output.
*/

static const char *const sega005_sample_names[] =
{
	"*005",
	"lexplode",     /* 0 */
	"sexplode",     /* 1 */
	"dropbomb",     /* 2 */
	"shoot",        /* 3 */
	"missile",      /* 4 */
	"helicopt",     /* 5 */
	"whistle",      /* 6 */
	nullptr
};


void segag80r_state::sega005_sound_board(machine_config &config)
{
	i8255_device &ppi(I8255A(config, "ppi8255"));
	ppi.out_pa_callback().set(FUNC(segag80r_state::sega005_sound_a_w));
	ppi.out_pb_callback().set(FUNC(segag80r_state::sega005_sound_b_w));

	/* sound hardware */

	SAMPLES(config, m_samples);
	m_samples->set_channels(7);
	m_samples->set_samples_names(sega005_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "speaker", 0.25);

	SEGA005(config, "005", 0).add_route(ALL_OUTPUTS, "speaker", 0.25);
}


/*************************************
 *
 *  005 sound triggers
 *
 *************************************/

void segag80r_state::sega005_sound_a_w(uint8_t data)
{
	uint8_t diff = data ^ m_sound_state[0];
	m_sound_state[0] = data;

	/* LARGE EXPL: channel 0 */
	if ((diff & 0x01) && !(data & 0x01)) m_samples->start(0, 0);

	/* SMALL EXPL: channel 1 */
	if ((diff & 0x02) && !(data & 0x02)) m_samples->start(1, 1);

	/* DROP BOMB: channel 2 */
	if ((diff & 0x04) && !(data & 0x04)) m_samples->start(2, 2);

	/* SHOOT PISTOL: channel 3 */
	if ((diff & 0x08) && !(data & 0x08)) m_samples->start(3, 3);

	/* MISSILE: channel 4 */
	if ((diff & 0x10) && !(data & 0x10)) m_samples->start(4, 4);

	/* HELICOPTER: channel 5 */
	if ((diff & 0x20) && !(data & 0x20) && !m_samples->playing(5)) m_samples->start(5, 5, true);
	if ((diff & 0x20) &&  (data & 0x20)) m_samples->stop(5);

	/* WHISTLE: channel 6 */
	if ((diff & 0x40) && !(data & 0x40) && !m_samples->playing(6)) m_samples->start(6, 6, true);
	if ((diff & 0x40) &&  (data & 0x40)) m_samples->stop(6);
}


inline void segag80r_state::sega005_update_sound_data()
{
	uint8_t newval = memregion("005")->base()[m_sound_addr];
	uint8_t diff = newval ^ m_sound_data;

	//osd_printf_debug("  [%03X] = %02X\n", m_sound_addr, newval);

	/* latch the new value */
	m_sound_data = newval;

	/* if bit 5 goes high, we reset the timer */
	if ((diff & 0x20) && !(newval & 0x20))
	{
		//osd_printf_debug("Stopping timer\n");
		m_005snd->m_sega005_sound_timer->adjust(attotime::never);
	}

	/* if bit 5 goes low, we start the timer again */
	if ((diff & 0x20) && (newval & 0x20))
	{
		//osd_printf_debug("Starting timer\n");
		m_005snd->m_sega005_sound_timer->adjust(attotime::from_hz(SEGA005_555_TIMER_FREQ), 0, attotime::from_hz(SEGA005_555_TIMER_FREQ));
	}
}


void segag80r_state::sega005_sound_b_w(uint8_t data)
{
	/*
	       D6: manual timer clock (0->1)
	       D5: 0 = manual timer, 1 = auto timer
	       D4: 1 = hold/reset address counter to 0
	    D3-D0: upper 4 bits of ROM address
	*/
	uint8_t diff = data ^ m_sound_state[1];
	m_sound_state[1] = data;

	//osd_printf_debug("sound[%d] = %02X\n", 1, data);

	/* force a stream update */
	m_005snd->m_sega005_stream->update();

	/* ROM address */
	m_sound_addr = ((data & 0x0f) << 7) | (m_sound_addr & 0x7f);

	/* reset both sound address and square wave counters */
	if (data & 0x10)
	{
		m_sound_addr &= 0x780;
		m_square_state = 0;
	}

	/* manual clock */
	if ((diff & 0x40) && (data & 0x40) && !(data & 0x20) && !(data & 0x10))
		m_sound_addr = (m_sound_addr & 0x780) | ((m_sound_addr + 1) & 0x07f);

	/* update the sound data */
	sega005_update_sound_data();
}



/*************************************
 *
 *  005 custom sound generation
 *
 *************************************/



TIMER_CALLBACK_MEMBER( sega005_sound_device::sega005_auto_timer )
{
	segag80r_state *state = machine().driver_data<segag80r_state>();
	/* force an update then clock the sound address if not held in reset */
	m_sega005_stream->update();
	if ((state->m_sound_state[1] & 0x20) && !(state->m_sound_state[1] & 0x10))
	{
		state->m_sound_addr = (state->m_sound_addr & 0x780) | ((state->m_sound_addr + 1) & 0x07f);
		state->sega005_update_sound_data();
	}
}



/*************************************
 *
 *  Space Odyssey sound hardware
 *
 *************************************/

static const char *const spaceod_sample_names[] =
{
	"*spaceod",
	"fire",         /* 0 */
	"bomb",         /* 1 */
	"eexplode",     /* 2 */
	"pexplode",     /* 3 */
	"warp",         /* 4 */
	"birth",        /* 5 */
	"scoreup",      /* 6 */
	"ssound",       /* 7 */
	"accel",        /* 8 */
	"damaged",      /* 9 */
	"erocket",      /* 10 */
	nullptr
};


void segag80r_state::spaceod_sound_board(machine_config &config)
{
	/* sound hardware */

	SAMPLES(config, m_samples);
	m_samples->set_channels(11);
	m_samples->set_samples_names(spaceod_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "speaker", 0.25);
}


/*************************************
 *
 *  Space Odyssey sound triggers
 *
 *************************************/

void segag80r_state::spaceod_sound_w(offs_t offset, uint8_t data)
{
	uint8_t diff = data ^ m_sound_state[offset];
	m_sound_state[offset] = data;

	switch (offset)
	{
		case 0:
			/* BACK G: channel 0 */
			if ((diff & 0x01) && !(data & 0x01) && !m_samples->playing(0)) m_samples->start(0, 7, true);
			if ((diff & 0x01) &&  (data & 0x01)) m_samples->stop(0);

			/* SHORT EXP: channel 1 */
			if ((diff & 0x04) && !(data & 0x04)) m_samples->start(1, 2);

			/* ACCELERATE: channel 2 */
			if ((diff & 0x10) && !(data & 0x10)) m_samples->start(2, 8);

			/* BATTLE STAR: channel 3 */
			if ((diff & 0x20) && !(data & 0x20)) m_samples->start(3, 10);

			/* D BOMB: channel 4 */
			if ((diff & 0x40) && !(data & 0x40)) m_samples->start(4, 1);

			/* LONG EXP: channel 5 */
			if ((diff & 0x80) && !(data & 0x80)) m_samples->start(5, 3);
			break;

		case 1:
			/* SHOT: channel 6 */
			if ((diff & 0x01) && !(data & 0x01)) m_samples->start(6, 0);

			/* BONUS UP: channel 7 */
			if ((diff & 0x02) && !(data & 0x02)) m_samples->start(7, 6);

			/* WARP: channel 8 */
			if ((diff & 0x08) && !(data & 0x08)) m_samples->start(8, 4);

			/* APPEARANCE UFO: channel 9 */
			if ((diff & 0x40) && !(data & 0x40)) m_samples->start(9, 5);

			/* BLACK HOLE: channel 10 */
			if ((diff & 0x80) && !(data & 0x80)) m_samples->start(10, 9);
			break;
	}
}



/*************************************
 *
 *  Monster Bash sound hardware
 *
 *************************************/

/*
    Monster Bash

    The Sound Board is a fairly complex mixture of different components.
    An 8255A-5 controls the interface to/from the sound board.
    Port A connects to a TMS3617 (basic music synthesizer) circuit.
    Port B connects to two sounds generated by discrete circuitry.
    Port C connects to a NEC7751 (8048 CPU derivative) to control four "samples".
*/

DEFINE_DEVICE_TYPE(MONSTERB_SOUND, monsterb_sound_device, "monsterb_sound", "Monster Bash Sound Board")

static const char *const monsterb_sample_names[] =
{
	"*monsterb",
	"zap",
	"jumpdown",
	nullptr
};

monsterb_sound_device::monsterb_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MONSTERB_SOUND, tag, owner, clock)
	, m_audiocpu(*this, "audiocpu")
	, m_audiocpu_region(*this, "upd7751")
	, m_music(*this, "music")
	, m_samples(*this, "samples")
	, m_i8243(*this, "i8243")
{
}

void monsterb_sound_device::device_start()
{
	save_item(NAME(m_upd7751_command));
	save_item(NAME(m_upd7751_busy));
	save_item(NAME(m_sound_state));
	save_item(NAME(m_sound_addr));
}


/*************************************
 *
 *  TMS3617 access
 *
 *************************************/

void monsterb_sound_device::sound_a_w(uint8_t data)
{
	/* Lower four data lines get decoded into 13 control lines */
	m_music->tms36xx_note_w(0, data & 15);

	/* Top four data lines address an 82S123 ROM that enables/disables voices */
	int enable_val = machine().root_device().memregion("prom")->base()[(data & 0xF0) >> 4];
	m_music->tms3617_enable_w(enable_val >> 2);
}



/*************************************
 *
 *  Discrete sound triggers
 *
 *************************************/

void monsterb_sound_device::sound_b_w(uint8_t data)
{
	uint8_t diff = data ^ m_sound_state[1];
	m_sound_state[1] = data;

	/* SHOT: channel 0 */
	if ((diff & 0x01) && !(data & 0x01)) m_samples->start(0, 0);

	/* DIVE: channel 1 */
	if ((diff & 0x02) && !(data & 0x02)) m_samples->start(1, 1);

	/* TODO: D7 on Port B might affect TMS3617 output (mute?) */
}



/*************************************
 *
 *  D7751 connections
 *
 *************************************/

uint8_t monsterb_sound_device::upd7751_status_r()
{
	return m_upd7751_busy << 4;
}


void monsterb_sound_device::upd7751_command_w(uint8_t data)
{
	/*
	    Z80 7751 control port

	    D0-D2 = connected to 7751 port C
	    D3    = /INT line
	*/
	m_upd7751_command = data & 0x07;
	m_audiocpu->set_input_line(0, ((data & 0x08) == 0) ? ASSERT_LINE : CLEAR_LINE);
	machine().scheduler().perfect_quantum(attotime::from_usec(100));
}


template<int Shift>
void monsterb_sound_device::upd7751_rom_addr_w(uint8_t data)
{
	// P4 - address lines 0-3
	// P5 - address lines 4-7
	// P5 - address lines 8-11
	m_sound_addr = (m_sound_addr & ~(0x00f << Shift)) | ((data & 0x0f) << Shift);
}


void monsterb_sound_device::upd7751_rom_select_w(uint8_t data)
{
	// P7 - ROM selects
	m_sound_addr &= 0xfff;

	int numroms = m_audiocpu_region->bytes() / 0x1000;
	if (!(data & 0x01) && numroms >= 1) m_sound_addr |= 0x0000;
	if (!(data & 0x02) && numroms >= 2) m_sound_addr |= 0x1000;
	if (!(data & 0x04) && numroms >= 3) m_sound_addr |= 0x2000;
	if (!(data & 0x08) && numroms >= 4) m_sound_addr |= 0x3000;
}


uint8_t monsterb_sound_device::upd7751_rom_r()
{
	/* read from BUS */
	return m_audiocpu_region->base()[m_sound_addr];
}


uint8_t monsterb_sound_device::upd7751_command_r()
{
	/* read from P2 - 8255's PC0-2 connects to 7751's S0-2 (P24-P26 on an 8048) */
	/* bit 0x80 is an alternate way to control the sample on/off; doesn't appear to be used */
	return 0x80 | ((m_upd7751_command & 0x07) << 4);
}


void monsterb_sound_device::upd7751_p2_w(uint8_t data)
{
	/* write to P2; low 4 bits go to 8243 */
	m_i8243->p2_w(data & 0x0f);

	/* output of bit $80 indicates we are ready (1) or busy (0) */
	/* no other outputs are used */
	m_upd7751_busy = data >> 7;
}



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void monsterb_sound_device::device_add_mconfig(machine_config &config)
{
	/* basic machine hardware */
	UPD7751(config, m_audiocpu, 6000000);
	m_audiocpu->t1_in_cb().set_constant(0); // labelled as "TEST", connected to ground
	m_audiocpu->p2_in_cb().set(FUNC(monsterb_sound_device::upd7751_command_r));
	m_audiocpu->bus_in_cb().set(FUNC(monsterb_sound_device::upd7751_rom_r));
	m_audiocpu->p1_out_cb().set("dac", FUNC(dac_byte_interface::data_w));
	m_audiocpu->p2_out_cb().set(FUNC(monsterb_sound_device::upd7751_p2_w));
	m_audiocpu->prog_out_cb().set(m_i8243, FUNC(i8243_device::prog_w));

	I8243(config, m_i8243);
	m_i8243->p4_out_cb().set(FUNC(monsterb_sound_device::upd7751_rom_addr_w<0>));
	m_i8243->p5_out_cb().set(FUNC(monsterb_sound_device::upd7751_rom_addr_w<4>));
	m_i8243->p6_out_cb().set(FUNC(monsterb_sound_device::upd7751_rom_addr_w<8>));
	m_i8243->p7_out_cb().set(FUNC(monsterb_sound_device::upd7751_rom_select_w));

	SAMPLES(config, m_samples);
	m_samples->set_channels(2);
	m_samples->set_samples_names(monsterb_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "speaker", 0.25);

	TMS36XX(config, m_music, 247);
	m_music->set_subtype(tms36xx_device::subtype::TMS3617);
	m_music->set_decays(0.5, 0.5, 0.5, 0.5, 0.5, 0.5);
	m_music->add_route(ALL_OUTPUTS, "speaker", 0.5);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // 50K (R91-97)/100K (R98-106) ladder network

	SPEAKER(config, "speaker").front_center();
}
