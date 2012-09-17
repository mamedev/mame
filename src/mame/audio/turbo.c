/*************************************************************************

    Sega Z80-3D system

*************************************************************************/

#include "emu.h"
#include "includes/turbo.h"
#include "sound/samples.h"


#define DISCRETE_TEST (0)



/*************************************
 *
 *  Turbo shared state updates
 *
 *************************************/

static void turbo_update_samples(turbo_state *state, samples_device *samples)
{
	/* accelerator sounds */
	/* BSEL == 3 --> off */
	/* BSEL == 2 --> standard */
	/* BSEL == 1 --> tunnel */
	/* BSEL == 0 --> ??? */
	if (state->m_turbo_bsel == 3 && samples->playing(5))
		samples->stop(5);
	else if (state->m_turbo_bsel != 3 && !samples->playing(5))
		samples->start(5, 7, true);
	if (samples->playing(5))
		samples->set_frequency(5, samples->base_frequency(5) * ((state->m_turbo_accel & 0x3f) / 5.25 + 1));
}


#if (DISCRETE_TEST)

static TIMER_CALLBACK( update_sound_a )
{
	device_t *discrete = machine.device("discrete");
	int data = param;

	/* missing short crash sample, but I've never seen it triggered */
	discrete_sound_w(discrete, 0, !(data & 0x01));
	discrete_sound_w(discrete, 1, (data >> 1) & 1);
	discrete_sound_w(discrete, 2, (data >> 2) & 1);
	discrete_sound_w(discrete, 3, (data >> 3) & 1);
	discrete_sound_w(discrete, 4, (data >> 4) & 1);
	discrete_sound_w(discrete, 5, !(data & 0x20));
	discrete_sound_w(discrete, 6, !(data & 0x40));

if (!((data >> 1) & 1)) mame_printf_debug("/TRIG1\n");
if (!((data >> 2) & 1)) mame_printf_debug("/TRIG2\n");
if (!((data >> 3) & 1)) mame_printf_debug("/TRIG3\n");
if (!((data >> 4) & 1)) mame_printf_debug("/TRIG4\n");

//  osel = (osel & 6) | ((data >> 5) & 1);
//  turbo_update_samples(samples);
}
#endif



/*************************************
 *
 *  Turbo PPI write handlers
 *
 *************************************/

WRITE8_DEVICE_HANDLER( turbo_sound_a_w )
{
#if (!DISCRETE_TEST)
	samples_device *samples = space.machine().device<samples_device>("samples");
#endif
	turbo_state *state = space.machine().driver_data<turbo_state>();
#if (!DISCRETE_TEST)
	UINT8 diff = data ^ state->m_sound_state[0];
#endif
	state->m_sound_state[0] = data;

#if (!DISCRETE_TEST)

	/* /CRASH.S: channel 0 */
	if ((diff & 0x01) && !(data & 0x01)) samples->start(0, 5);

	/* /TRIG1: channel 1 */
	if ((diff & 0x02) && !(data & 0x02)) samples->start(1, 0);

	/* /TRIG2: channel 1 */
	if ((diff & 0x04) && !(data & 0x04)) samples->start(1, 1);

	/* /TRIG3: channel 1 */
	if ((diff & 0x08) && !(data & 0x08)) samples->start(1, 2);

	/* /TRIG4: channel 1 */
	if ((diff & 0x10) && !(data & 0x10)) samples->start(1, 3);

	/* OSEL0 */
	state->m_turbo_osel = (state->m_turbo_osel & 6) | ((data >> 5) & 1);

	/* /SLIP: channel 2 */
	if ((diff & 0x40) && !(data & 0x40)) samples->start(2, 4);

	/* /CRASH.L: channel 3 */
	if ((diff & 0x80) && !(data & 0x80)) samples->start(3, 5);

	/* update any samples */
	turbo_update_samples(state, samples);

#else

	if (((data ^ state->m_last_sound_a) & 0x1e) && (state->m_last_sound_a & 0x1e) != 0x1e)
		space.machine().scheduler().timer_set(attotime::from_hz(20000), FUNC(update_sound_a), data);
	else
		update_sound_a(data);

	state->m_last_sound_a = data;

#endif
}


WRITE8_DEVICE_HANDLER( turbo_sound_b_w )
{
	samples_device *samples = space.machine().device<samples_device>("samples");
	turbo_state *state = space.machine().driver_data<turbo_state>();
	UINT8 diff = data ^ state->m_sound_state[1];
	state->m_sound_state[1] = data;

	/* ACC0-ACC5 */
	state->m_turbo_accel = data & 0x3f;
	output_set_value("tachometer", state->m_turbo_accel);

	/* /AMBU: channel 4 */
	if ((diff & 0x40) && !(data & 0x40) && !samples->playing(4)) samples->start(4, 8, true);
	if ((diff & 0x40) &&  (data & 0x40)) samples->stop(4);

	/* /SPIN: channel 2 */
	if ((diff & 0x80) && !(data & 0x80)) samples->start(2, 6);

	/* update any samples */
	turbo_update_samples(state, samples);
}


WRITE8_DEVICE_HANDLER( turbo_sound_c_w )
{
	samples_device *samples = space.machine().device<samples_device>("samples");
	turbo_state *state = space.machine().driver_data<turbo_state>();

	/* OSEL1-2 */
	state->m_turbo_osel = (state->m_turbo_osel & 1) | ((data & 3) << 1);

	/* BSEL0-1 */
	state->m_turbo_bsel = (data >> 2) & 3;

	/* SPEED0-3 */
	output_set_value("speed", (data >> 4) & 0x0f);

	/* update any samples */
	turbo_update_samples(state, samples);
}



/*************************************
 *
 *  Turbo sound hardware description
 *
 *************************************/

static const char *const turbo_sample_names[] =
{
	"*turbo",
	"01",		/* 0: Trig1 */
	"02",		/* 1: Trig2 */
	"03",		/* 2: Trig3 */
	"04",		/* 3: Trig4 */
	"05",		/* 4: Screech */
	"06",		/* 5: Crash */
	"skidding",	/* 6: Spin */
	"idle",		/* 7: Idle */
	"ambulanc",	/* 8: Ambulance */
	0
};


static const samples_interface turbo_samples_interface =
{
	10,
	turbo_sample_names
};


MACHINE_CONFIG_FRAGMENT( turbo_samples )

	/* this is the cockpit speaker configuration */
	MCFG_SPEAKER_ADD("fspeaker", 0.0, 0.0, 1.0)		/* front */
	MCFG_SPEAKER_ADD("bspeaker",  0.0, 0.0, -0.5)	/* back */
	MCFG_SPEAKER_ADD("lspeaker", -0.2, 0.0, 1.0)	/* left */
	MCFG_SPEAKER_ADD("rspeaker", 0.2, 0.0, 1.0)		/* right */

	MCFG_SAMPLES_ADD("samples", turbo_samples_interface)

	/* channel 0 = CRASH.S -> CRASH.S/SM */
	MCFG_SOUND_ROUTE(0, "fspeaker", 0.25)

	/* channel 1 = TRIG1-4 -> ALARM.M/F/R/L */
	MCFG_SOUND_ROUTE(1, "fspeaker", 0.25)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.25)
	MCFG_SOUND_ROUTE(1, "lspeaker",  0.25)

	/* channel 2 = SLIP/SPIN -> SKID.F/R/L/M */
	MCFG_SOUND_ROUTE(2, "fspeaker", 0.25)
	MCFG_SOUND_ROUTE(2, "rspeaker", 0.25)
	MCFG_SOUND_ROUTE(2, "lspeaker",  0.25)

	/* channel 3 = CRASH.L -> CRASH.L/LM */
	MCFG_SOUND_ROUTE(3, "bspeaker",  0.25)

	/* channel 4 = AMBU -> AMBULANCE/AMBULANCE.M */
	MCFG_SOUND_ROUTE(4, "fspeaker", 0.25)

	/* channel 5 = ACCEL+BSEL -> MYCAR.F/W/M + MYCAR0.F/M + MYCAR1.F/M */
	MCFG_SOUND_ROUTE(5, "fspeaker", 0.25)
	MCFG_SOUND_ROUTE(5, "bspeaker",  0.25)

	/* channel 6 = OSEL -> OCAR.F/FM */
	MCFG_SOUND_ROUTE(6, "fspeaker", 0.25)

	/* channel 7 = OSEL -> OCAR.L/LM */
	MCFG_SOUND_ROUTE(7, "lspeaker",  0.25)

	/* channel 8 = OSEL -> OCAR.R/RM */
	MCFG_SOUND_ROUTE(8, "rspeaker", 0.25)

	/* channel 9 = OSEL -> OCAR.W/WM */
	MCFG_SOUND_ROUTE(9, "bspeaker",  0.25)
MACHINE_CONFIG_END

/*
    Cockpit: CN2 1+2 -> FRONT
             CN2 3+4 -> REAR
             CN2 5+6 -> RIGHT
             CN2 7+8 -> LEFT

    Upright: CN2 1+2 -> UPPER
             CN2 3+4 -> LOWER

    F.OUT = CRASH.S +
            ALARM.F +
            SKID.F +
            OCAR.F +
            MYCAR.F +
            MYCAR0.F +
            MYCAR1.F +
            AMBULACE

    W.OUT = CRASH.L +
            OCAR.W +
            MYCAR.W +
            MYCAR0.W +
            MYCAR1.W +
            SLF

    R.OUT = ALARM.R +
            SKID.R +
            OCAR.R

    L.OUT = ALARM.L +
            SKID.L +
            OCAR.L

    M.OUT = CRASH.SM +
            CRASH.LM +
            SKID.M +
            ALARM.M +
            AMBULACE.M +
            MYCAR.M +
            MYCAR0.M +
            MYCAR1.M +
            OCAR.FM +
            OCAR.LM +
            OCAR.RM +
            OCAR.WM
*/



/*************************************
 *
 *  Subroc 3D PPI write handlers
 *
 *************************************/

WRITE8_DEVICE_HANDLER( subroc3d_sound_a_w )
{
	turbo_state *state = space.machine().driver_data<turbo_state>();
	state->m_sound_state[0] = data;

	/* DIS0-3 contained in bits 0-3 */
	/* DIR0-2 contained in bits 4-6 */
}


INLINE void subroc3d_update_volume(samples_device *samples, int leftchan, UINT8 dis, UINT8 dir)
{
	float volume = (float)(15 - dis) / 16.0f;
	float lvol, rvol;

	/* compute the left/right volume from the data */
	if (dir != 7)
	{
		lvol = volume * (float)(6 - dir) / 6.0f;
		rvol = volume * (float)dir / 6.0f;
	}
	else
		lvol = rvol = 0;

	/* if the sample is playing, adjust it */
	samples->set_volume(leftchan + 0, lvol);
	samples->set_volume(leftchan + 1, rvol);
}


WRITE8_DEVICE_HANDLER( subroc3d_sound_b_w )
{
	samples_device *samples = space.machine().device<samples_device>("samples");
	turbo_state *state = space.machine().driver_data<turbo_state>();
	UINT8 diff = data ^ state->m_sound_state[1];
	state->m_sound_state[1] = data;

	/* bit 0 latches direction/volume for missile */
	if ((diff & 0x01) && (data & 0x01))
	{
		state->m_subroc3d_mdis = state->m_sound_state[0] & 0x0f;
		state->m_subroc3d_mdir = (state->m_sound_state[0] >> 4) & 0x07;
		if (!samples->playing(0))
		{
			samples->start(0, 0, true);
			samples->start(1, 0, true);
		}
		subroc3d_update_volume(samples, 0, state->m_subroc3d_mdis, state->m_subroc3d_mdir);
	}

	/* bit 1 latches direction/volume for torpedo */
	if ((diff & 0x02) && (data & 0x02))
	{
		state->m_subroc3d_tdis = state->m_sound_state[0] & 0x0f;
		state->m_subroc3d_tdir = (state->m_sound_state[0] >> 4) & 0x07;
		if (!samples->playing(2))
		{
			samples->start(2, 1, true);
			samples->start(3, 1, true);
		}
		subroc3d_update_volume(samples, 2, state->m_subroc3d_tdis, state->m_subroc3d_tdir);
	}

	/* bit 2 latches direction/volume for fighter */
	if ((diff & 0x04) && (data & 0x04))
	{
		state->m_subroc3d_fdis = state->m_sound_state[0] & 0x0f;
		state->m_subroc3d_fdir = (state->m_sound_state[0] >> 4) & 0x07;
		if (!samples->playing(4))
		{
			samples->start(4, 2, true);
			samples->start(5, 2, true);
		}
		subroc3d_update_volume(samples, 4, state->m_subroc3d_fdis, state->m_subroc3d_fdir);
	}

	/* bit 3 latches direction/volume for hit */
	if ((diff & 0x08) && (data & 0x08))
	{
		state->m_subroc3d_hdis = state->m_sound_state[0] & 0x0f;
		state->m_subroc3d_hdir = (state->m_sound_state[0] >> 4) & 0x07;
		subroc3d_update_volume(samples, 6, state->m_subroc3d_hdis, state->m_subroc3d_hdir);
	}
}


WRITE8_DEVICE_HANDLER( subroc3d_sound_c_w )
{
	samples_device *samples = space.machine().device<samples_device>("samples");
	turbo_state *state = space.machine().driver_data<turbo_state>();
	UINT8 diff = data ^ state->m_sound_state[2];
	state->m_sound_state[2] = data;

	/* /FIRE TRIG */
	/* FIRE SELECT */
	if ((diff & 0x01) && (data & 0x01))
		samples->start(8, (data & 0x02) ? 6 : 5);

	/* /SHIP EXP TRIG -> MY SHIP EXP: channel 9 */
	if ((diff & 0x04) && (data & 0x04))
		samples->start(9, 7);

	/* /HIT TRIG -> HIT.L/R: channels 6+7 */
	if ((diff & 0x08) && (data & 0x08))
	{
		samples->start(6, (state->m_sound_state[0] & 0x80) ? 4 : 3);
		samples->start(7, (state->m_sound_state[0] & 0x80) ? 4 : 3);
	}

	/* /ALARM TRIG -> ALARM.M: channel 10 */
	/* ALARM SELECT */
	if ((diff & 0x10) && (data & 0x10))
		samples->start(10, (data & 0x20) ? 10 : 9);

	/* /PROLOGUE */
	if (!samples->playing(11))
		samples->start(11, 8, true);
	samples->set_volume(11, (data & 0x40) ? 0 : 1.0);

	/* /GAME START */
	space.machine().sound().system_mute(data & 0x80);
}



/*************************************
 *
 *  Subroc 3D sound hardware description
 *
 *************************************/

static const char *const subroc3d_sample_names[] =
{
	"*subroc3d",
	"01",   /*  0: enemy missile */
	"02",   /*  1: enemy torpedo */
	"03",   /*  2: enemy fighter */
	"04",   /*  3: explosion in sky */
	"05",   /*  4: explosion on sea */
	"06",   /*  5: missile shoot */
	"07",   /*  6: torpedo shoot */
	"08",   /*  7: my ship expl */
	"09",   /*  8: prolog sound */
	"11",   /*  9: alarm 0 */
	"12",   /* 10: alarm 1 */
	0
};


static const samples_interface subroc3d_samples_interface =
{
	12,
	subroc3d_sample_names
};


MACHINE_CONFIG_FRAGMENT( subroc3d_samples )
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SAMPLES_ADD("samples", subroc3d_samples_interface)

	/* MISSILE in channels 0 and 1 */
	MCFG_SOUND_ROUTE(0, "lspeaker",  0.25)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.25)

	/* TORPEDO in channels 2 and 3 */
	MCFG_SOUND_ROUTE(2, "lspeaker",  0.25)
	MCFG_SOUND_ROUTE(3, "rspeaker", 0.25)

	/* FIGHTER in channels 4 and 5 */
	MCFG_SOUND_ROUTE(4, "lspeaker",  0.25)
	MCFG_SOUND_ROUTE(5, "rspeaker", 0.25)

	/* HIT in channels 6 and 7 */
	MCFG_SOUND_ROUTE(6, "lspeaker",  0.25)
	MCFG_SOUND_ROUTE(7, "rspeaker", 0.25)

	/* FIRE sound in channel 8 */
	MCFG_SOUND_ROUTE(8, "lspeaker",  0.25)
	MCFG_SOUND_ROUTE(8, "rspeaker", 0.25)

	/* SHIP EXP sound in channel 9 */
	MCFG_SOUND_ROUTE(9, "lspeaker",  0.25)
	MCFG_SOUND_ROUTE(9, "rspeaker", 0.25)

	/* ALARM TRIG sound in channel 10 */
	MCFG_SOUND_ROUTE(10, "lspeaker",  0.25)
	MCFG_SOUND_ROUTE(10, "rspeaker", 0.25)

	/* PROLOGUE sound in channel 11 */
	MCFG_SOUND_ROUTE(11, "lspeaker",  0.25)
	MCFG_SOUND_ROUTE(11, "rspeaker", 0.25)
MACHINE_CONFIG_END



/*************************************
 *
 *  Buck Rogers PPI write handlers
 *
 *************************************/

static void buckrog_update_samples(turbo_state *state, samples_device *samples)
{
	/* accelerator sounds */
	if (samples->playing(5))
		samples->set_frequency(5, samples->base_frequency(5) * (state->m_buckrog_myship / 100.25 + 1));
}


WRITE8_DEVICE_HANDLER( buckrog_sound_a_w )
{
	samples_device *samples = space.machine().device<samples_device>("samples");
	turbo_state *state = space.machine().driver_data<turbo_state>();
	UINT8 diff = data ^ state->m_sound_state[0];
	state->m_sound_state[0] = data;

	/* clock HIT DIS from bits 0-2 */
	if ((diff & 0x10) && (data & 0x10))
		samples->set_volume(3, (float)(/*7 - */(data & 7)) / 7.0f);

	/* clock ACC from bits 0-3 */
	if ((diff & 0x20) && (data & 0x20))
	{
		state->m_buckrog_myship = data & 0x0f;
		buckrog_update_samples(state, samples);
	}

	/* /ALARM0: channel 0 */
	if ((diff & 0x40) && !(data & 0x40)) samples->start(0, 0);

	/* /ALARM1: channel 0 */
	if ((diff & 0x80) && !(data & 0x80)) samples->start(0, 1);
}


WRITE8_DEVICE_HANDLER( buckrog_sound_b_w )
{
	samples_device *samples = space.machine().device<samples_device>("samples");
	turbo_state *state = space.machine().driver_data<turbo_state>();
	UINT8 diff = data ^ state->m_sound_state[1];
	state->m_sound_state[1] = data;

	/* /ALARM3: channel 0 */
	if ((diff & 0x01) && !(data & 0x01)) samples->start(0, 2);

	/* /ALARM4: channel 0 */
	if ((diff & 0x02) && !(data & 0x02)) samples->start(0, 3);

	/* /FIRE: channel 1 */
	if ((diff & 0x04) && !(data & 0x04)) samples->start(1, 5);

	/* /EXP: channel 2 */
	if ((diff & 0x08) && !(data & 0x08)) samples->start(2, 4);

	/* /HIT: channel 3 */
	if ((diff & 0x10) && !(data & 0x10))
	{
		samples->start(3, 7);
		buckrog_update_samples(state, samples);
	}

	/* /REBOUND: channel 4 */
	if ((diff & 0x20) && !(data & 0x20)) samples->start(4, 6);

	/* SHIP: channel 5 */
	if ((diff & 0x40) &&  (data & 0x40) && !samples->playing(5))
	{
		samples->start(5, 8, true);
		buckrog_update_samples(state, samples);
	}
	if ((diff & 0x40) && !(data & 0x40) &&  samples->playing(5)) samples->stop(5);

	/* GAME ON */
	space.machine().sound().system_enable(data & 0x80);
}



/*************************************
 *
 *  Sound interfaces
 *
 *************************************/

static const char *const buckrog_sample_names[]=
{
	"*buckrog",
	"alarm0",	/* 0 */
	"alarm1",	/* 1 */
	"alarm2",	/* 2 */
	"alarm3",	/* 3 */
	"exp",		/* 4 */
	"fire",		/* 5 */
	"rebound",	/* 6 */
	"hit",		/* 7 */
	"shipsnd1",	/* 8 */
	"shipsnd2",	/* 9 */
	"shipsnd3",	/* 10 */
	0
};


static const samples_interface buckrog_samples_interface =
{
	6,          /* 6 channels */
	buckrog_sample_names
};


MACHINE_CONFIG_FRAGMENT( buckrog_samples )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SAMPLES_ADD("samples", buckrog_samples_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/*************************************
 *
 *  Discrete test code
 *
 *************************************/

#if (DISCRETE_TEST)

/* Nodes - Inputs */
#define TURBO_CRASH_EN			NODE_01
#define TURBO_TRIG1_INV			NODE_02
#define TURBO_TRIG2_INV			NODE_03
#define TURBO_TRIG3_INV			NODE_04
#define TURBO_TRIG4_INV			NODE_05
#define TURBO_SLIP_EN			NODE_06
#define TURBO_CRASHL_EN			NODE_07
#define TURBO_ACC_VAL			NODE_08
#define TURBO_AMBU_EN			NODE_09
#define TURBO_SPIN_EN			NODE_10
#define TURBO_OSEL_VAL			NODE_11
#define TURBO_BSEL_VAL			NODE_12

/* Nodes - Sounds */
#define FIRETRUCK_NOISE			NODE_20

static const discrete_555_desc turbo_alarm_555 =
{
	DISC_555_OUT_SQW | DISC_555_OUT_DC,
	5,				// B+ voltage of 555
	DEFAULT_555_VALUES,
};

DISCRETE_SOUND_START(turbo)
	/************************************************/
	/* Input register mapping for turbo             */
	/************************************************/
	/*                  NODE             ADDR  MASK    GAIN    OFFSET  INIT */
	DISCRETE_INPUT(TURBO_CRASH_EN		,0x00,0x001f,                  0.0)
	DISCRETE_INPUT(TURBO_TRIG1_INV  	,0x01,0x001f,                  1.0)
	DISCRETE_INPUT(TURBO_TRIG2_INV		,0x02,0x001f,                  1.0)
	DISCRETE_INPUT(TURBO_TRIG3_INV		,0x03,0x001f,                  1.0)
	DISCRETE_INPUT(TURBO_TRIG4_INV  	,0x04,0x001f,                  1.0)
	DISCRETE_INPUT(TURBO_SLIP_EN    	,0x05,0x001f,                  0.0)
	DISCRETE_INPUT(TURBO_CRASHL_EN		,0x06,0x001f,                  0.0)
	DISCRETE_INPUT(TURBO_ACC_VAL		,0x07,0x001f,                  0.0)
	DISCRETE_INPUT(TURBO_AMBU_EN		,0x08,0x001f,                  0.0)
	DISCRETE_INPUT(TURBO_SPIN_EN		,0x09,0x001f,                  0.0)
	DISCRETE_INPUT(TURBO_OSEL_VAL		,0x0a,0x001f,                  0.0)
	DISCRETE_INPUT(TURBO_BSEL_VAL		,0x0b,0x001f,                  0.0)

	/************************************************/
	/* Alarm sounds                                 */
	/************************************************/

	// 5-5-5 counter provides the input clock
	DISCRETE_555_ASTABLE(NODE_50,1,470,120,0.1e-6,&turbo_alarm_555)
	// which clocks a 74393 dual 4-bit counter, clocked on the falling edge
	DISCRETE_COUNTER(NODE_51,1,0,NODE_50,0,15,1,0,DISC_CLK_ON_F_EDGE)
	// the high bit of this counter
	DISCRETE_TRANSFORM2(NODE_52,NODE_51,8,"01/")
	// clocks the other half of the 74393
	DISCRETE_COUNTER(NODE_53,1,0,NODE_52,0,15,1,0,DISC_CLK_ON_F_EDGE)

	// trig1 triggers a LS123 retriggerable multivibrator
	DISCRETE_ONESHOT(NODE_60,TURBO_TRIG1_INV,5.0,(0.33e-9)*47*1e6, DISC_ONESHOT_FEDGE|DISC_ONESHOT_RETRIG|DISC_OUT_ACTIVE_HIGH)
	// which interacts with bit 0 of the second counter
	DISCRETE_TRANSFORM2(NODE_61,NODE_53,1,"01&")
	// via a NAND
	DISCRETE_LOGIC_NAND(NODE_62,1,NODE_60,NODE_61)

	// trig2 triggers a LS123 retriggerable multivibrator
	DISCRETE_ONESHOT(NODE_65,TURBO_TRIG2_INV,5.0,(0.33e-9)*47*10e6,DISC_ONESHOT_FEDGE|DISC_ONESHOT_RETRIG|DISC_OUT_ACTIVE_HIGH)
	// which interacts with bit 3 of the first counter via a NAND
	DISCRETE_LOGIC_NAND(NODE_66,1,NODE_65,NODE_52)

	// trig3 triggers a LS123 retriggerable multivibrator
	DISCRETE_ONESHOT(NODE_70,TURBO_TRIG3_INV,5.0,(0.33e-9)*47*33e6,DISC_ONESHOT_FEDGE|DISC_ONESHOT_RETRIG|DISC_OUT_ACTIVE_HIGH)
	// which interacts with bit 2 of the first counter
	DISCRETE_TRANSFORM3(NODE_71,NODE_51,4,1,"01/2&")
	// via a NAND
	DISCRETE_LOGIC_NAND(NODE_72,1,NODE_70,NODE_71)

	// trig4 triggers a LS123 retriggerable multivibrator
	DISCRETE_ONESHOT(NODE_75,TURBO_TRIG4_INV,5.0,(0.33e-9)*47*10e6,DISC_ONESHOT_FEDGE|DISC_ONESHOT_RETRIG|DISC_OUT_ACTIVE_HIGH)
	// which interacts with bit 1 of the first counter
	DISCRETE_TRANSFORM3(NODE_76,NODE_51,2,1,"01/2&")
	// via a NAND
	DISCRETE_LOGIC_NAND(NODE_77,1,NODE_75,NODE_76)

	// everything is effectively NANDed together
	DISCRETE_LOGIC_NAND4(NODE_80,1,NODE_62,NODE_66,NODE_72,NODE_77)

/*

    the rest of the circuit looks like this:

                      +5V            +12V                                +---+
                       ^              ^   +--------+               1K    v   |
                       |              |   | |\     |           +---NNN--NNN--+
                       Z 1K       10K Z   | | \    |           | |\     20K  |   +--|(----> ALARM_M
                       Z              Z   +-|- \   |           | | \         |   |  4.7u
                       |              |     |   >--+---NNNN----+-|- \        |   +--|(----> ALARM_F
                       +--NNNN--|(----+-----|+ /        22K      |   >-------+---+  4.7u
    +-\                |  5.1K  4.7u  |     | /             +6V--|+ /            +--|(----> ALARM_R
    |  >o---(NODE_62)--+              Z     |/                   | /             |  4.7u
    +-/                |          10K Z                          |/              +--|(----> ALARM_L
                       |              |                                             4.7u
    +-\                |              v
    |  >o---(NODE_66)--+             GND
    +-/                |
                       |
    +-\                |
    |  >o---(NODE_72)--+
    +-/                |
                       |
    +-\                |
    |  >o---(NODE_77)--+
    +-/


*/

	/************************************************/
	/* Combine all 7 sound sources.                 */
	/************************************************/

	DISCRETE_OUTPUT(NODE_80, 16000)
DISCRETE_SOUND_END

#endif
