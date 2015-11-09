// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega G-80 raster hardware

    Across these games, there's a mixture of discrete sound circuitry,
    speech boards, ADPCM samples, and a TMS3617 music chip.

***************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "includes/segag80r.h"
#include "machine/i8255.h"
#include "machine/i8243.h"
#include "sound/samples.h"
#include "sound/tms36xx.h"
#include "sound/dac.h"


/*************************************
 *
 *  Constants
 *
 *************************************/

#define SEGA005_555_TIMER_FREQ      (1.44 / ((15000 + 2 * 4700) * 1.5e-6))
#define SEGA005_COUNTER_FREQ        (100000)    /* unknown, just a guess */

const device_type SEGA005 = &device_creator<sega005_sound_device>;

sega005_sound_device::sega005_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SEGA005, "Sega 005 Audio Custom", tag, owner, clock, "sega005_sound", __FILE__),
		device_sound_interface(mconfig, *this),
		m_sega005_sound_timer(NULL),
		m_sega005_stream(NULL)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void sega005_sound_device::device_config_complete()
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
	const UINT8 *sound_prom = state->memregion("proms")->base();
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
	0
};


MACHINE_CONFIG_FRAGMENT( astrob_sound_board )

	/* sound hardware */
	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(11)
	MCFG_SAMPLES_NAMES(astrob_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END


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

	UINT8 diff = data ^ m_sound_state[offset];
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
	0
};


MACHINE_CONFIG_FRAGMENT( 005_sound_board )

	MCFG_DEVICE_ADD("ppi8255", I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(segag80r_state, sega005_sound_a_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(segag80r_state, sega005_sound_b_w))

	/* sound hardware */

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(7)
	MCFG_SAMPLES_NAMES(sega005_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("005", SEGA005, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END


/*************************************
 *
 *  005 sound triggers
 *
 *************************************/

WRITE8_MEMBER(segag80r_state::sega005_sound_a_w)
{
	UINT8 diff = data ^ m_sound_state[0];
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
	UINT8 newval = memregion("005")->base()[m_sound_addr];
	UINT8 diff = newval ^ m_sound_data;

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
	UINT8 diff = data ^ m_sound_state[1];
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
	0
};


MACHINE_CONFIG_FRAGMENT( spaceod_sound_board )

	/* sound hardware */

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(11)
	MCFG_SAMPLES_NAMES(spaceod_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END


/*************************************
 *
 *  Space Odyssey sound triggers
 *
 *************************************/

WRITE8_MEMBER(segag80r_state::spaceod_sound_w)
{
	UINT8 diff = data ^ m_sound_state[offset];
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


static const char *const monsterb_sample_names[] =
{
	"*monsterb",
	"zap",
	"jumpdown",
	0
};


/*************************************
 *
 *  N7751 memory maps
 *
 *************************************/

static ADDRESS_MAP_START( monsterb_7751_portmap, AS_IO, 8, segag80r_state )
	AM_RANGE(MCS48_PORT_T1,   MCS48_PORT_T1) AM_READ(n7751_t1_r)
	AM_RANGE(MCS48_PORT_P2,   MCS48_PORT_P2) AM_READ(n7751_command_r)
	AM_RANGE(MCS48_PORT_BUS,  MCS48_PORT_BUS) AM_READ(n7751_rom_r)
	AM_RANGE(MCS48_PORT_P1,   MCS48_PORT_P1) AM_DEVWRITE("dac", dac_device, write_unsigned8)
	AM_RANGE(MCS48_PORT_P2,   MCS48_PORT_P2) AM_WRITE(n7751_p2_w)
	AM_RANGE(MCS48_PORT_PROG, MCS48_PORT_PROG) AM_DEVWRITE("audio_8243", i8243_device, i8243_prog_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

MACHINE_CONFIG_FRAGMENT( monsterb_sound_board )
	MCFG_DEVICE_ADD("ppi8255", I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(segag80r_state, monsterb_sound_a_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(segag80r_state, monsterb_sound_b_w))
	MCFG_I8255_IN_PORTC_CB(READ8(segag80r_state, n7751_status_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(segag80r_state, n7751_command_w))

	/* basic machine hardware */
	MCFG_CPU_ADD("audiocpu", N7751, 6000000)
	MCFG_CPU_IO_MAP(monsterb_7751_portmap)

	MCFG_I8243_ADD("audio_8243", NOOP, WRITE8(segag80r_state,n7751_rom_control_w))

	/* sound hardware */

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(2)
	MCFG_SAMPLES_NAMES(monsterb_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_TMS36XX_ADD("music", 247)
	MCFG_TMS36XX_TYPE(TMS3617)
	MCFG_TMS36XX_DECAY_TIMES(0.5, 0.5, 0.5, 0.5, 0.5, 0.5)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/*************************************
 *
 *  TMS3617 access
 *
 *************************************/

WRITE8_MEMBER(segag80r_state::monsterb_sound_a_w)
{
	tms36xx_device *tms = machine().device<tms36xx_device>("music");
	int enable_val;

	/* Lower four data lines get decoded into 13 control lines */
	tms->tms36xx_note_w(0, data & 15);

	/* Top four data lines address an 82S123 ROM that enables/disables voices */
	enable_val = machine().root_device().memregion("prom")->base()[(data & 0xF0) >> 4];
	tms->tms3617_enable_w(enable_val >> 2);
}



/*************************************
 *
 *  Discrete sound triggers
 *
 *************************************/

WRITE8_MEMBER(segag80r_state::monsterb_sound_b_w)
{
	UINT8 diff = data ^ m_sound_state[1];
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

READ8_MEMBER(segag80r_state::n7751_status_r)
{
	return m_n7751_busy << 4;
}


WRITE8_MEMBER(segag80r_state::n7751_command_w)
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


WRITE8_MEMBER(segag80r_state::n7751_rom_control_w)
{
	/* P4 - address lines 0-3 */
	/* P5 - address lines 4-7 */
	/* P6 - address lines 8-11 */
	/* P7 - ROM selects */
	switch (offset)
	{
		case 0:
			m_sound_addr = (m_sound_addr & ~0x00f) | ((data & 0x0f) << 0);
			break;

		case 1:
			m_sound_addr = (m_sound_addr & ~0x0f0) | ((data & 0x0f) << 4);
			break;

		case 2:
			m_sound_addr = (m_sound_addr & ~0xf00) | ((data & 0x0f) << 8);
			break;

		case 3:
			m_sound_addr &= 0xfff;
			{
				int numroms = memregion("n7751")->bytes() / 0x1000;
				if (!(data & 0x01) && numroms >= 1) m_sound_addr |= 0x0000;
				if (!(data & 0x02) && numroms >= 2) m_sound_addr |= 0x1000;
				if (!(data & 0x04) && numroms >= 3) m_sound_addr |= 0x2000;
				if (!(data & 0x08) && numroms >= 4) m_sound_addr |= 0x3000;
			}
			break;
	}
}


READ8_MEMBER(segag80r_state::n7751_rom_r)
{
	/* read from BUS */
	return memregion("n7751")->base()[m_sound_addr];
}


READ8_MEMBER(segag80r_state::n7751_command_r)
{
	/* read from P2 - 8255's PC0-2 connects to 7751's S0-2 (P24-P26 on an 8048) */
	/* bit 0x80 is an alternate way to control the sample on/off; doesn't appear to be used */
	return 0x80 | ((m_n7751_command & 0x07) << 4);
}


WRITE8_MEMBER(segag80r_state::n7751_p2_w)
{
	i8243_device *device = machine().device<i8243_device>("audio_8243");
	/* write to P2; low 4 bits go to 8243 */
	device->i8243_p2_w(space, offset, data & 0x0f);

	/* output of bit $80 indicates we are ready (1) or busy (0) */
	/* no other outputs are used */
	m_n7751_busy = data >> 7;
}


READ8_MEMBER(segag80r_state::n7751_t1_r)
{
	/* T1 - labelled as "TEST", connected to ground */
	return 0;
}
