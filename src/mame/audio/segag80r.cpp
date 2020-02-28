// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega G-80 raster hardware

    Across these games, there's a mixture of discrete sound circuitry,
    speech boards, ADPCM samples, and a TMS3617 music chip.

***************************************************************************/

#include "emu.h"
#include "includes/segag80r.h"

#include "audio/segag80r.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"

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
	m_sega005_stream = machine().sound().stream_alloc(*this, 0, 1, SEGA005_COUNTER_FREQ);

	/* create a timer for the 555 */
	m_sega005_sound_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(sega005_sound_device::sega005_auto_timer), this));

	/* set the initial sound data */
	state->m_sound_data = 0x00;
	state->sega005_update_sound_data();
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void sega005_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	segag80r_state *state = machine().driver_data<segag80r_state>();
	const uint8_t *sound_prom = state->memregion("proms")->base();
	int i;

	/* no implementation yet */
	for (i = 0; i < samples; i++)
	{
		if (!(state->m_sound_state[1] & 0x10) && (++state->m_square_count & 0xff) == 0)
		{
			state->m_square_count = sound_prom[state->m_sound_data & 0x1f];

			/* hack - the RC should filter this out */
			if (state->m_square_count != 0xff)
				state->m_square_state += 2;
		}

		outputs[0][i] = (state->m_square_state & 2) ? 0x7fff : 0x0000;
	}
}





/*************************************
 *
 *  Astro Blaster sound hardware
 *
 *************************************/

/*
    Description of Astro Blaster sounds (in the hope of future discrete goodness):

    CD4017 = decade counter with one output per decoded stage (10 outputs altogether)
    CD4024 = 7-bit counter with 7 outputs


    "V" signal
    ----------
        CD4017 @ U15:
            reset by RATE RESET signal = 1
            clocked by falling edge of ATTACK signal
            +12V output from here goes through a diode and one of 10 resistors:
                0 = 120k
                1 = 82k
                2 = 62k
                3 = 56k
                4 = 47k
                5 = 39k
                6 = 35k
                7 = 27k
                8 = 24k
                9 = 22k
            and then in series through a 22k resistor

        Op-amp @ U6 takes the WARP signal and the output of CD4017 @ U15
            and forms the signal "V" which is used to control the invader
            sounds


        How to calculate the output voltage at U16 labeled (V).
        (Derrick Renaud)

        First you have an inverting amp.  To get the gain you
        use G=-Rf/Ri, where Rf=R178=22k.  Ri is the selected
        resistor on the output of U15.

        The input voltage to the amp (pin 6) will always be
        about 12V - 0.5V (diode drop in low current circuit) =
        11.5V.

        Now you need to calculate the reference voltage on the
        + input (pin 5).  Depending on the state of WARP...

        If the warp data is 0, then U31 inverts it to an Open
        Collector high, meaning WARP is out of circuit. So:
        Vref = 12V * (R163)/(R162+R163)
             = 12V * 10k/(10K+4.7k)
             = 8.163V

        When warp data is 1, then U31 inverts it to low,
        grounding R164 putting it in parallel with R163,
        giving:
        Vref = 12V * (R163||R164)/(R163||R164 +R162)
             = 12V * 5k/(5k+4.7k)
             = 6.186V

        Now to get the control voltage V:
        V = (Vi - Vref) * G + Vref
          = (11.5V - Vref) * G + Vref

        That gives you the control voltage at V.  From there I
        would have to millman the voltage with the internal
        voltage/resistors of the 555 to get the actual used
        control voltage.

        But it seems you just want a range, so just use the
        above info to get the highest and lowest voltages
        generated, and create the frequency shift you desire.
        Remember as the control voltage (V) lowers, the
        frequency increases.



    INVADER-1 output
    ----------------




    INVADER-2 output
    ----------------
        555 timer @ U13 in astable mode with the following parameters:
            R1 = 10k
            R2 = 100k
            C = 0.0022u
            CV = "V" signal
            Reset = (PORT076 & 0x02)
        Output goes to CD4024 @ U12

        CD4024 @ U12:
            reset through some unknown means
            clocked by 555 timer @ U13
            +12 output from here goes through a resistor ladder:
                Q1 -> 82k
                Q2 -> 39k
                Q3 -> 22k
                Q4 -> 10k
        Summed output from here is INVADER-2


    INVADER-3 output
    ----------------
        555 timer at U17 in astable mode with the following parameters:
            R1 = 10k
            R2 = 68k
            C = 0.1u
            CV = some combination of "V" and "W" signals
            Reset = (PORT076 & 0x04)
        Output from here is INVADER-3

*/

static const char *const astrob_sample_names[] =
{
	"*astrob",
	"invadr1",      /* 0 */
	"winvadr1",     /* 1 */
	"invadr2",      /* 2 */
	"winvadr2",     /* 3 */
	"invadr3",      /* 4 */
	"winvadr3",     /* 5 */
	"invadr4",      /* 6 */
	"winvadr4",     /* 7 */
	"asteroid",     /* 8 */
	"refuel",       /* 9 */
	"pbullet",      /* 10 */
	"ebullet",      /* 11 */
	"eexplode",     /* 12 */
	"pexplode",     /* 13 */
	"deedle",       /* 14 */
	"sonar",        /* 15 */
	nullptr
};


void segag80r_state::astrob_sound_board(machine_config &config)
{
	/* sound hardware */
	SAMPLES(config, m_samples);
	m_samples->set_channels(11);
	m_samples->set_samples_names(astrob_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "speaker", 0.25);
}


/*************************************
 *
 *  Astro Blaster sound triggers
 *
 *************************************/

WRITE8_MEMBER(segag80r_state::astrob_sound_w)
{
	static const float attack_resistor[10] =
	{
		120.0f, 82.0f, 62.0f, 56.0f, 47.0f, 39.0f, 33.0f, 27.0f, 24.0f, 22.0f
	};
	float freq_factor;

	uint8_t diff = data ^ m_sound_state[offset];
	m_sound_state[offset] = data;

	switch (offset)
	{
		case 0:
			/* INVADER-1: channel 0 */
			if ((diff & 0x01) && !(data & 0x01)) m_samples->start(0, (data & 0x80) ? 0 : 1, true);
			if ((data & 0x01) && m_samples->playing(0)) m_samples->stop(0);

			/* INVADER-2: channel 1 */
			if ((diff & 0x02) && !(data & 0x02)) m_samples->start(1, (data & 0x80) ? 2 : 3, true);
			if ((data & 0x02) && m_samples->playing(1)) m_samples->stop(1);

			/* INVADER-3: channel 2 */
			if ((diff & 0x04) && !(data & 0x04)) m_samples->start(2, (data & 0x80) ? 4 : 5, true);
			if ((data & 0x04) && m_samples->playing(2)) m_samples->stop(2);

			/* INVADER-4: channel 3 */
			if ((diff & 0x08) && !(data & 0x08)) m_samples->start(3, (data & 0x80) ? 6 : 7, true);
			if ((data & 0x08) && m_samples->playing(3)) m_samples->stop(3);

			/* ASTROIDS: channel 4 */
			if ((diff & 0x10) && !(data & 0x10)) m_samples->start(4, 8, true);
			if ((data & 0x10) && m_samples->playing(4)) m_samples->stop(4);

			/* MUTE */
			machine().sound().system_mute(data & 0x20);

			/* REFILL: channel 5 */
			if (!(data & 0x40) && !m_samples->playing(5)) m_samples->start(5, 9);
			if ( (data & 0x40) && m_samples->playing(5))  m_samples->stop(5);

			/* WARP: changes which sample is played for the INVADER samples above */
			if (diff & 0x80)
			{
				if (m_samples->playing(0)) m_samples->start(0, (data & 0x80) ? 0 : 1, true);
				if (m_samples->playing(1)) m_samples->start(1, (data & 0x80) ? 2 : 3, true);
				if (m_samples->playing(2)) m_samples->start(2, (data & 0x80) ? 4 : 5, true);
				if (m_samples->playing(3)) m_samples->start(3, (data & 0x80) ? 6 : 7, true);
			}
			break;

		case 1:
			/* LASER #1: channel 6 */
			if ((diff & 0x01) && !(data & 0x01)) m_samples->start(6, 10);

			/* LASER #2: channel 7 */
			if ((diff & 0x02) && !(data & 0x02)) m_samples->start(7, 11);

			/* SHORT EXPL: channel 8 */
			if ((diff & 0x04) && !(data & 0x04)) m_samples->start(8, 12);

			/* LONG EXPL: channel 8 */
			if ((diff & 0x08) && !(data & 0x08)) m_samples->start(8, 13);

			/* ATTACK RATE */
			if ((diff & 0x10) && !(data & 0x10)) m_sound_rate = (m_sound_rate + 1) % 10;

			/* RATE RESET */
			if (!(data & 0x20)) m_sound_rate = 0;

			/* BONUS: channel 9 */
			if ((diff & 0x40) && !(data & 0x40)) m_samples->start(9, 14);

			/* SONAR: channel 10 */
			if ((diff & 0x80) && !(data & 0x80)) m_samples->start(10, 15);
			break;
	}

	/* the samples were recorded with sound_rate = 0, so we need to scale */
	/* the frequency as a fraction of that; these equations come from */
	/* Derrick's analysis above; we compute the inverted scale factor to */
	/* account for the fact that frequency goes up as CV goes down */
	/* WARP is already taken into account by the differing samples above */
	freq_factor  = (11.5f - 8.163f) * (-22.0f / attack_resistor[0]) + 8.163f;
	freq_factor /= (11.5f - 8.163f) * (-22.0f / attack_resistor[m_sound_rate]) + 8.163f;

	/* adjust the sample rate of invader sounds based the sound_rate */
	/* this is an approximation */
	if (m_samples->playing(0)) m_samples->set_frequency(0, m_samples->base_frequency(0) * freq_factor);
	if (m_samples->playing(1)) m_samples->set_frequency(1, m_samples->base_frequency(1) * freq_factor);
	if (m_samples->playing(2)) m_samples->set_frequency(2, m_samples->base_frequency(2) * freq_factor);
	if (m_samples->playing(3)) m_samples->set_frequency(3, m_samples->base_frequency(3) * freq_factor);
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

WRITE8_MEMBER(segag80r_state::sega005_sound_a_w)
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


WRITE8_MEMBER(segag80r_state::sega005_sound_b_w)
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

WRITE8_MEMBER(segag80r_state::spaceod_sound_w)
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
	, m_audiocpu_region(*this, "n7751")
	, m_music(*this, "music")
	, m_samples(*this, "samples")
	, m_i8243(*this, "i8243")
{
}

void monsterb_sound_device::device_start()
{
	save_item(NAME(m_n7751_command));
	save_item(NAME(m_n7751_busy));
	save_item(NAME(m_sound_state));
	save_item(NAME(m_sound_addr));
}


/*************************************
 *
 *  TMS3617 access
 *
 *************************************/

WRITE8_MEMBER(monsterb_sound_device::sound_a_w)
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

WRITE8_MEMBER(monsterb_sound_device::sound_b_w)
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
 *  N7751 connections
 *
 *************************************/

READ8_MEMBER(monsterb_sound_device::n7751_status_r)
{
	return m_n7751_busy << 4;
}


WRITE8_MEMBER(monsterb_sound_device::n7751_command_w)
{
	/*
	    Z80 7751 control port

	    D0-D2 = connected to 7751 port C
	    D3    = /INT line
	*/
	m_n7751_command = data & 0x07;
	m_audiocpu->set_input_line(0, ((data & 0x08) == 0) ? ASSERT_LINE : CLEAR_LINE);
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(100));
}


template<int Shift>
void monsterb_sound_device::n7751_rom_addr_w(uint8_t data)
{
	// P4 - address lines 0-3
	// P5 - address lines 4-7
	// P5 - address lines 8-11
	m_sound_addr = (m_sound_addr & ~(0x00f << Shift)) | ((data & 0x0f) << Shift);
}


void monsterb_sound_device::n7751_rom_select_w(uint8_t data)
{
	// P7 - ROM selects
	m_sound_addr &= 0xfff;

	int numroms = m_audiocpu_region->bytes() / 0x1000;
	if (!(data & 0x01) && numroms >= 1) m_sound_addr |= 0x0000;
	if (!(data & 0x02) && numroms >= 2) m_sound_addr |= 0x1000;
	if (!(data & 0x04) && numroms >= 3) m_sound_addr |= 0x2000;
	if (!(data & 0x08) && numroms >= 4) m_sound_addr |= 0x3000;
}


READ8_MEMBER(monsterb_sound_device::n7751_rom_r)
{
	/* read from BUS */
	return m_audiocpu_region->base()[m_sound_addr];
}


READ8_MEMBER(monsterb_sound_device::n7751_command_r)
{
	/* read from P2 - 8255's PC0-2 connects to 7751's S0-2 (P24-P26 on an 8048) */
	/* bit 0x80 is an alternate way to control the sample on/off; doesn't appear to be used */
	return 0x80 | ((m_n7751_command & 0x07) << 4);
}


WRITE8_MEMBER(monsterb_sound_device::n7751_p2_w)
{
	/* write to P2; low 4 bits go to 8243 */
	m_i8243->p2_w(data & 0x0f);

	/* output of bit $80 indicates we are ready (1) or busy (0) */
	/* no other outputs are used */
	m_n7751_busy = data >> 7;
}



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void monsterb_sound_device::device_add_mconfig(machine_config &config)
{
	/* basic machine hardware */
	N7751(config, m_audiocpu, 6000000);
	m_audiocpu->t1_in_cb().set_constant(0); // labelled as "TEST", connected to ground
	m_audiocpu->p2_in_cb().set(FUNC(monsterb_sound_device::n7751_command_r));
	m_audiocpu->bus_in_cb().set(FUNC(monsterb_sound_device::n7751_rom_r));
	m_audiocpu->p1_out_cb().set("dac", FUNC(dac_byte_interface::data_w));
	m_audiocpu->p2_out_cb().set(FUNC(monsterb_sound_device::n7751_p2_w));
	m_audiocpu->prog_out_cb().set(m_i8243, FUNC(i8243_device::prog_w));

	I8243(config, m_i8243);
	m_i8243->p4_out_cb().set(FUNC(monsterb_sound_device::n7751_rom_addr_w<0>));
	m_i8243->p5_out_cb().set(FUNC(monsterb_sound_device::n7751_rom_addr_w<4>));
	m_i8243->p6_out_cb().set(FUNC(monsterb_sound_device::n7751_rom_addr_w<8>));
	m_i8243->p7_out_cb().set(FUNC(monsterb_sound_device::n7751_rom_select_w));

	SAMPLES(config, m_samples);
	m_samples->set_channels(2);
	m_samples->set_samples_names(monsterb_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "speaker", 0.25);

	TMS36XX(config, m_music, 247);
	m_music->set_subtype(tms36xx_device::subtype::TMS3617);
	m_music->set_decays(0.5, 0.5, 0.5, 0.5, 0.5, 0.5);
	m_music->add_route(ALL_OUTPUTS, "speaker", 0.5);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // 50K (R91-97)/100K (R98-106) ladder network
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref", 0));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);

	SPEAKER(config, "speaker").front_center();
}
