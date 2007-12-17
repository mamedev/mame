/*****************************************************************************

    Texas Instruments SN76477 emulator

    authors: Derrick Renaud - info
             Zsolt Vasvari  - software

    (see sn76477.h for details)

    Notes:
        * All formulas were derived by taking measurements of a real device,
          then running the data sets through the numerical analysis
          application at http://zunzun.com to come up with the functions.

    Known issues/to-do's:
        * VCO
            * confirm value of VCO_MAX_EXT_VOLTAGE, VCO_TO_SLF_VOLTAGE_DIFF
              VCO_CAP_VOLTAGE_MIN and VCO_CAP_VOLTAGE_MAX
            * confirm value of VCO_MIN_DUTY_CYCLE
            * get real formulas for VCO cap charging and discharging
            * get real formula for VCO duty cycle
            * what happens if no vco_res
            * what happens if no vco_cap

        * Attack/Decay
            * get real formulas for a/d cap charging and discharging

 *****************************************************************************/

#include <math.h>		/* for pow() */
#include "sndintrf.h"
#include "streams.h"
#include "sn76477.h"



/*****************************************************************************
 *
 *  Debugging
 *
 *****************************************************************************/

#define VERBOSE					1

/* if 1, create a .wav file of the sound produced */
#define LOG_WAV					0

/* if 1 and LOG_WAV is 1, only logs to .wav file in chip is enabled */
#define LOG_WAV_ENABLED_ONLY	0

/* determines what value will be logged in the left channel of the .wav file */
#define LOG_WAV_VALUE_L			0	/* 0 = OUT voltage */
									/* 1 = enable line */
									/* 2 = one-shot cap voltage */
									/* 3 = a/d cap voltage */
									/* 4 = slf cap voltage */
									/* 5 = vco cap voltage */
									/* 6 = noise filter cap voltage */

/* determines what value will be logged in the right channel of the .wav file
   same values as for the left channel above */
#define LOG_WAV_VALUE_R			3

#define LOG_WAV_GAIN_FACTOR		1000

#define LOG_WAV_FILE_NAME		"sn76477_%d.wav"


#if VERBOSE
#define LOG(n,x) if (VERBOSE >= (n)) logerror x
#else
#define LOG(n,x)
#endif

#define CHECK_CHIP_NUM					assert(sn != NULL)
#define CHECK_CHIP_NUM_AND_BOOLEAN		CHECK_CHIP_NUM; assert((data & 0x01) == data)
#define CHECK_CHIP_NUM_AND_POSITIVE		CHECK_CHIP_NUM; assert(data >= 0.0)
#define CHECK_CHIP_NUM_AND_VOLTAGE		CHECK_CHIP_NUM; assert((data >= 0.0) && (data <= 5.0))
#define CHECK_CHIP_NUM_AND_CAP_VOLTAGE	CHECK_CHIP_NUM; assert(((data >= 0.0) && (data <= 5.0)) || (data == SN76477_EXTERNAL_VOLTAGE_DISCONNECT))



/*****************************************************************************
 *
 *  Test Mode
 *
 *  in test mode, the interface structure
 *  passed in by the driver is not used.
 *  Instead, the values for all the inputs
 *  can be specified by modifing the structure
 *  below.  Calls by the driver to the input
 *  setter functions are ignored. Use the
 *  space bar to enable/disable the chip.
 *
 *****************************************************************************/

#define TEST_MODE	0


#if TEST_MODE
#include "input.h"

static struct SN76477interface empty_interface =
{
	0,			/*  4 noise_clock_res  */
	0,			/*  5 filter_res       */
	0,			/*  6 filter_cap       */
	0,			/*  7 decay_res        */
	0,			/*  8 attack_decay_cap */
	0, 			/* 10 attack_res       */
	0,			/* 11 amplitude_res    */
	0,			/* 12 feedback_res     */
	0,			/* 16 vco_voltage      */
	0,			/* 17 vco_cap          */
	0,			/* 18 vco_res          */
	0,			/* 19 pitch_voltage    */
	0,			/* 20 slf_res          */
	0,			/* 21 slf_cap          */
	0,			/* 23 oneshot_cap      */
	0,			/* 24 oneshot_res      */
	0,			/* 22 vco              */
	0,			/* 26 mixer A          */
	0,			/* 25 mixer B          */
	0,			/* 27 mixer C          */
	0,			/* 1  envelope 1       */
	0,			/* 28 envelope 2       */
	0			/* 9  enable           */
};

#define test_interface empty_empty_interface

#endif



/*****************************************************************************
 *
 *  Constants
 *
 *****************************************************************************/

#define ONE_SHOT_CAP_VOLTAGE_MIN 	(0)			/* the voltage at which the one-shot starts from (measured) */
#define ONE_SHOT_CAP_VOLTAGE_MAX 	(2.5)		/* the voltage at which the one-shot finishes (measured) */
#define ONE_SHOT_CAP_VOLTAGE_RANGE	(ONE_SHOT_CAP_VOLTAGE_MAX - ONE_SHOT_CAP_VOLTAGE_MIN)

#define SLF_CAP_VOLTAGE_MIN			(0.33)		/* the voltage at the bottom peak of the SLF triangle wave (measured) */
#define SLF_CAP_VOLTAGE_MAX			(2.37)		/* the voltage at the top peak of the SLF triangle wave (measured) */
#define SLF_CAP_VOLTAGE_RANGE		(SLF_CAP_VOLTAGE_MAX - SLF_CAP_VOLTAGE_MIN)

#define VCO_MAX_EXT_VOLTAGE			(2.35)		/* the external voltage at which the VCO saturates and produces no output,
                                                   also used as the voltage threshold for the SLF */
#define VCO_TO_SLF_VOLTAGE_DIFF		(0.35)
#define VCO_CAP_VOLTAGE_MIN			(SLF_CAP_VOLTAGE_MIN)	/* the voltage at the bottom peak of the VCO triangle wave */
#define VCO_CAP_VOLTAGE_MAX			(SLF_CAP_VOLTAGE_MAX + VCO_TO_SLF_VOLTAGE_DIFF)	/* the voltage at the bottom peak of the VCO triangle wave */
#define VCO_CAP_VOLTAGE_RANGE		(VCO_CAP_VOLTAGE_MAX - VCO_CAP_VOLTAGE_MIN)
#define VCO_DUTY_CYCLE_50			(5.0)		/* the high voltage that produces a 50% duty cycle */
#define VCO_MIN_DUTY_CYCLE			(18)		/* the smallest possible duty cycle, in % */

#define NOISE_MIN_CLOCK_RES			RES_K(10)	/* the maximum resistor value that still produces a noise (measured) */
#define NOISE_MAX_CLOCK_RES			RES_M(3.3)	/* the minimum resistor value that still produces a noise (measured) */
#define NOISE_CAP_VOLTAGE_MIN		(0)			/* the minimum voltage that the noise filter cap can hold (measured) */
#define NOISE_CAP_VOLTAGE_MAX		(5.0)		/* the maximum voltage that the noise filter cap can hold (measured) */
#define NOISE_CAP_VOLTAGE_RANGE		(NOISE_CAP_VOLTAGE_MAX - NOISE_CAP_VOLTAGE_MIN)
#define NOISE_CAP_HIGH_THRESHOLD	(3.35)		/* the voltage at which the filtered noise bit goes to 0 (measured) */
#define NOISE_CAP_LOW_THRESHOLD		(0.74)		/* the voltage at which the filtered noise bit goes to 1 (measured) */

#define AD_CAP_VOLTAGE_MIN			(0)			/* the minimum voltage the attack/decay cap can hold (measured) */
#define AD_CAP_VOLTAGE_MAX			(4.44)		/* the minimum voltage the attack/decay cap can hold (measured) */
#define AD_CAP_VOLTAGE_RANGE		(AD_CAP_VOLTAGE_MAX - AD_CAP_VOLTAGE_MIN)

#define OUT_CENTER_LEVEL_VOLTAGE	(2.57)		/* the voltage that gets outputted when the volumne is 0 (measured) */
#define OUT_HIGH_CLIP_THRESHOLD		(3.51)		/* the maximum voltage that can be put out (measured) */
#define OUT_LOW_CLIP_THRESHOLD		(0.715)		/* the minimum voltage that can be put out (measured) */

/* gain factors for OUT voltage in 0.1V increments (measured) */
static const double out_pos_gain[] =
{
	0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.01,	 /* 0.0 - 0.9V */
	0.03, 0.11, 0.15, 0.19, 0.21, 0.23, 0.26, 0.29, 0.31, 0.33,  /* 1.0 - 1.9V */
	0.36, 0.38, 0.41, 0.43, 0.46, 0.49, 0.52, 0.54, 0.57, 0.60,  /* 2.0 - 2.9V */
	0.62, 0.65, 0.68, 0.70, 0.73, 0.76, 0.80, 0.82, 0.84, 0.87,  /* 3.0 - 3.9V */
	0.90, 0.93, 0.96, 0.98, 1.00							 	 /* 4.0 - 4.4V */
};

static const double out_neg_gain[] =
{
	 0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00,  0.00, -0.01,  /* 0.0 - 0.9V */
	-0.02, -0.09, -0.13, -0.15, -0.17, -0.19, -0.22, -0.24, -0.26, -0.28,  /* 1.0 - 1.9V */
	-0.30, -0.32, -0.34, -0.37, -0.39, -0.41, -0.44, -0.46, -0.48, -0.51,  /* 2.0 - 2.9V */
	-0.53, -0.56, -0.58, -0.60, -0.62, -0.65, -0.67, -0.69, -0.72, -0.74,  /* 3.0 - 3.9V */
	-0.76, -0.78, -0.81, -0.84, -0.85									   /* 4.0 - 4.4V */
};



/*****************************************************************************
 *
 *  State structure
 *
 *****************************************************************************/

struct SN76477
{
	/* chip's external interface */
	UINT32 enable;
	UINT32 envelope_mode;
	UINT32 vco_mode;
	UINT32 mixer_mode;

	double one_shot_res;
	double one_shot_cap;
	UINT32 one_shot_cap_voltage_ext;

	double slf_res;
	double slf_cap;
	UINT32 slf_cap_voltage_ext;

	double vco_voltage;
	double vco_res;
	double vco_cap;
	UINT32 vco_cap_voltage_ext;

	double noise_clock_res;
	UINT32 noise_clock_ext;
	UINT32 noise_clock;
	double noise_filter_res;
	double noise_filter_cap;
	UINT32 noise_filter_cap_voltage_ext;

	double attack_res;
	double decay_res;
	double attack_decay_cap;
	UINT32 attack_decay_cap_voltage_ext;

	double amplitude_res;
	double feedback_res;
	double pitch_voltage;

	/* chip's internal state */
	double one_shot_cap_voltage;		/* voltage on the one-shot cap */
	UINT32 one_shot_running_ff;			/* 1 = one-shot running, 0 = stopped */

	double slf_cap_voltage;				/* voltage on the SLF cap */
	UINT32 slf_out_ff;					/* output of the SLF */

	double vco_cap_voltage;				/* voltage on the VCO cap */
	UINT32 vco_out_ff;					/* output of the VCO */
	UINT32 vco_alt_pos_edge_ff;			/* keeps track of the # of positive edges for VCO Alt envelope */

	double noise_filter_cap_voltage;	/* voltage on the noise filter cap */
	UINT32 real_noise_bit_ff;			/* the current noise bit before filtering */
	UINT32 filtered_noise_bit_ff;		/* the noise bit after filtering */
	UINT32 noise_gen_count;				/* noise freq emulation */

	double attack_decay_cap_voltage;	/* voltage on the attack/decay cap */

	UINT32 rng;							/* current value of the random number generator */

	/* others */
	sound_stream *channel;				/* returned by stream_create() */
	UINT32 index;
	int sample_rate; 					/* from Machine->sample_rate */

#if LOG_WAV
	wav_file *file;						/* handle of the wave file to produce */
#endif
};



/*****************************************************************************
 *
 *  Max/min
 *
 *****************************************************************************/

#undef max
#undef min

INLINE double max(double a, double b)
{
	return (a > b) ? a : b;
}


INLINE double min(double a, double b)
{
	return (a < b) ? a : b;
}



/*****************************************************************************
 *
 *  Functions for computing frequencies, voltages and similar values based
 *  on the hardware itself.  Do NOT put anything emulation specific here,
 *  such as calculations based on sample_rate.
 *
 *****************************************************************************/

static double compute_one_shot_cap_charging_rate(struct SN76477 *sn) /* in V/sec */
{
	/* this formula was derived using the data points below

     Res (kohms)  Cap (uF)   Time (millisec)
         47         0.33         11.84
         47         1.0          36.2
         47         1.5          52.1
         47         2.0          76.4
        100         0.33         24.4
        100         1.0          75.2
        100         1.5         108.5
        100         2.0         158.4
    */

	double ret = 0;

	if ((sn->one_shot_res > 0) && (sn->one_shot_cap > 0))
	{
		ret = ONE_SHOT_CAP_VOLTAGE_RANGE / (0.8024 * sn->one_shot_res * sn->one_shot_cap + 0.002079);
	}
	else if (sn->one_shot_cap > 0)
	{
		/* if no resistor, there is no current to charge the cap,
           effectively making the one-shot time effectively infinite */
		ret = +1e-30;
	}
	else if (sn->one_shot_res > 0)
	{
		/* if no cap, the voltage changes extremely fast,
           effectively making the one-shot time 0 */
		ret = +1e+30;
	}

	return ret;
}


static double compute_one_shot_cap_discharging_rate(struct SN76477 *sn) /* in V/sec */
{
	/* this formula was derived using the data points below

    Cap (uF)   Time (microsec)
      0.33           300
      1.0            850
      1.5           1300
      2.0           1900
    */

	double ret = 0;

	if ((sn->one_shot_res > 0) && (sn->one_shot_cap > 0))
	{
		ret = ONE_SHOT_CAP_VOLTAGE_RANGE / (854.7 * sn->one_shot_cap + 0.00001795);
	}
	else if (sn->one_shot_res > 0)
	{
		/* if no cap, the voltage changes extremely fast,
           effectively making the one-shot time 0 */
		ret = +1e+30;
	}

	return ret;
}


static double compute_slf_cap_charging_rate(struct SN76477 *sn) /* in V/sec */
{
	/* this formula was derived using the data points below

    Res (kohms)  Cap (uF)   Time (millisec)
         47        0.47          14.3
        120        0.47          35.6
        200        0.47          59.2
         47        1.00          28.6
        120        1.00          71.6
        200        1.00         119.0
    */
	double ret = 0;

	if ((sn->slf_res > 0) && (sn->slf_cap > 0))
	{
		ret = SLF_CAP_VOLTAGE_RANGE / (0.5885 * sn->slf_res * sn->slf_cap + 0.001300);
	}

	return ret;
}


static double compute_slf_cap_discharging_rate(struct SN76477 *sn) /* in V/sec */
{
	/* this formula was derived using the data points below

    Res (kohms)  Cap (uF)   Time (millisec)
         47        0.47          13.32
        120        0.47          32.92
        200        0.47          54.4
         47        1.00          26.68
        120        1.00          66.2
        200        1.00         109.6
    */
	double ret = 0;

	if ((sn->slf_res > 0) && (sn->slf_cap > 0))
	{
		ret = SLF_CAP_VOLTAGE_RANGE / (0.5413 * sn->slf_res * sn->slf_cap + 0.001343);
	}

	return ret;
}


static double compute_vco_cap_charging_discharging_rate(struct SN76477 *sn) /* in V/sec */
{
	double ret = 0;

	if ((sn->vco_res > 0) && (sn->vco_cap > 0))
	{
		ret = 0.64 * 2 * VCO_CAP_VOLTAGE_RANGE / (sn->vco_res * sn->vco_cap);
	}

	return ret;
}


static double compute_vco_duty_cycle(struct SN76477 *sn) /* no measure, just a number */
{
	double ret = 0.5;	/* 50% */

	if ((sn->vco_voltage > 0) && (sn->pitch_voltage != VCO_DUTY_CYCLE_50))
	{
		ret = max(0.5 * (sn->pitch_voltage / sn->vco_voltage), (VCO_MIN_DUTY_CYCLE / 100.0));

		ret = min(ret, 1);
	}

	return ret;
}


static UINT32 compute_noise_gen_freq(struct SN76477 *sn) /* in Hz */
{
	/* this formula was derived using the data points below

     Res (ohms)   Freq (Hz)
        10k         97493
        12k         83333
        15k         68493
        22k         49164
        27k         41166
        33k         34449
        36k         31969
        47k         25126
        56k         21322
        68k         17721.5
        82k         15089.2
        100k        12712.0
        150k         8746.4
        220k         6122.4
        270k         5101.5
        330k         4217.2
        390k         3614.5
        470k         3081.7
        680k         2132.7
        820k         1801.8
          1M         1459.9
        2.2M          705.13
        3.3M          487.59
    */

	UINT32 ret = 0;

	if ((sn->noise_clock_res >= NOISE_MIN_CLOCK_RES) &&
	    (sn->noise_clock_res <= NOISE_MAX_CLOCK_RES))
	{
		ret = 339100000 * pow(sn->noise_clock_res, -0.8849);
	}

	return ret;
}


static double compute_noise_filter_cap_charging_rate(struct SN76477 *sn) /* in V/sec */
{
	/* this formula was derived using the data points below

     R*C        Time (sec)
    .000068     .0000184
    .0001496    .0000378
    .0002244    .0000548
    .0003196    .000077
    .0015       .000248
    .0033       .000540
    .00495      .000792
    .00705      .001096
    */

	double ret = 0;

	if ((sn->noise_filter_res > 0) && (sn->noise_filter_cap > 0))
	{
		ret = NOISE_CAP_VOLTAGE_RANGE / (0.1571 * sn->noise_filter_res * sn->noise_filter_cap + 0.00001430);
	}
	else if (sn->noise_filter_cap > 0)
	{
		/* if no resistor, there is no current to charge the cap,
           effectively making the filter's output constants */
		ret = +1e-30;
	}
	else if (sn->noise_filter_res > 0)
	{
		/* if no cap, the voltage changes extremely fast,
           effectively disabling the filter */
		ret = +1e+30;
	}

	return ret;
}


static double compute_noise_filter_cap_discharging_rate(struct SN76477 *sn) /* in V/sec */
{
	/* this formula was derived using the data points below

     R*C        Time (sec)
    .000068     .000016
    .0001496    .0000322
    .0002244    .0000472
    .0003196    .0000654
    .0015       .000219
    .0033       .000468
    .00495      .000676
    .00705      .000948
    */

	double ret = 0;

	if ((sn->noise_filter_res > 0) && (sn->noise_filter_cap > 0))
	{
		ret = NOISE_CAP_VOLTAGE_RANGE / (0.1331 * sn->noise_filter_res * sn->noise_filter_cap + 0.00001734);
	}
	else if (sn->noise_filter_cap > 0)
	{
		/* if no resistor, there is no current to charge the cap,
           effectively making the filter's output constants */
		ret = +1e-30;
	}
	else if (sn->noise_filter_res > 0)
	{
		/* if no cap, the voltage changes extremely fast,
           effectively disabling the filter */
		ret = +1e+30;
	}

	return ret;
}


static double compute_attack_decay_cap_charging_rate(struct SN76477 *sn)  /* in V/sec */
{
	double ret = 0;

	if ((sn->attack_res > 0) && (sn->attack_decay_cap > 0))
	{
		ret = AD_CAP_VOLTAGE_RANGE / (sn->attack_res * sn->attack_decay_cap);
	}
	else if (sn->attack_decay_cap > 0)
	{
		/* if no resistor, there is no current to charge the cap,
           effectively making the attack time infinite */
		ret = +1e-30;
	}
	else if (sn->attack_res > 0)
	{
		/* if no cap, the voltage changes extremely fast,
           effectively making the attack time 0 */
		ret = +1e+30;
	}

	return ret;
}


static double compute_attack_decay_cap_discharging_rate(struct SN76477 *sn)  /* in V/sec */
{
	double ret = 0;

	if ((sn->decay_res > 0) && (sn->attack_decay_cap > 0))
	{
		ret = AD_CAP_VOLTAGE_RANGE / (sn->decay_res * sn->attack_decay_cap);
	}
	else if (sn->attack_decay_cap > 0)
	{
		/* if no resistor, there is no current to charge the cap,
           effectively making the decay time infinite */
		ret = +1e-30;
	}
	else if (sn->attack_res > 0)
	{
		/* if no cap, the voltage changes extremely fast,
           effectively making the decay time 0 */
		ret = +1e+30;
	}

	return ret;
}


static double compute_center_to_peak_voltage_out(struct SN76477 *sn)
{
	/* this formula was derived using the data points below

     Ra (kohms)  Rf (kohms)   Voltage
        150         47          1.28
        200         47          0.96
         47         22          1.8
        100         22          0.87
        150         22          0.6
        200         22          0.45
         47         10          0.81
        100         10          0.4
        150         10          0.27
    */

	double ret = 0;

	if (sn->amplitude_res > 0)
	{
		ret = 3.818 * (sn->feedback_res / sn->amplitude_res) + 0.03;
	}

	return ret;
}



/*****************************************************************************
 *
 *  Logging functions
 *
 *****************************************************************************/

static void log_enable_line(struct SN76477 *sn)
{
#if VERBOSE
	static const char *desc[] =
	{
		"Enabled", "Inhibited"
	};
#endif

	LOG(1, ("SN76477 #%d:              Enable line (9): %d [%s]\n", sn->index, sn->enable, desc[sn->enable]));
}


static void log_mixer_mode(struct SN76477 *sn)
{
#if VERBOSE
	const char *desc[] =
	{
		"VCO", "SLF", "Noise", "VCO/Noise",
		"SLF/Noise", "SLF/VCO/Noise", "SLF/VCO", "Inhibit"
	};
#endif

	LOG(1, ("SN76477 #%d:           Mixer mode (25-27): %d [%s]\n", sn->index, sn->mixer_mode, desc[sn->mixer_mode]));
}


static void log_envelope_mode(struct SN76477 *sn)
{
#if VERBOSE
	const char *desc[] =
	{
		"VCO", "One-Shot", "Mixer Only", "VCO with Alternating Polarity"
	};
#endif

	LOG(1, ("SN76477 #%d:         Envelope mode (1,28): %d [%s]\n", sn->index, sn->envelope_mode, desc[sn->envelope_mode]));
}


static void log_vco_mode(struct SN76477 *sn)
{
#if VERBOSE
	const char *desc[] =
	{
		"External (Pin 16)", "Internal (SLF)"
	};
#endif

	LOG(1, ("SN76477 #%d:                VCO mode (22): %d [%s]\n", sn->index, sn->vco_mode, desc[sn->vco_mode]));
}


static void log_one_shot_time(struct SN76477 *sn)
{
	if (!sn->one_shot_cap_voltage_ext)
	{
		if (compute_one_shot_cap_charging_rate(sn) > 0)
		{
			LOG(1, ("SN76477 #%d:        One-shot time (23,24): %.4f sec\n", sn->index, ONE_SHOT_CAP_VOLTAGE_RANGE * (1 / compute_one_shot_cap_charging_rate(sn))));
		}
		else
		{
			LOG(1, ("SN76477 #%d:        One-shot time (23,24): N/A\n", sn->index));
		}
	}
	else
	{
		LOG(1, ("SN76477 #%d:        One-shot time (23,24): External (cap = %.2fV)\n", sn->index, sn->one_shot_cap_voltage));
	}
}


static void log_slf_freq(struct SN76477 *sn)
{
	if (!sn->slf_cap_voltage_ext)
	{
		if (compute_slf_cap_charging_rate(sn) > 0)
		{
			double charging_time = (1 / compute_slf_cap_charging_rate(sn)) * SLF_CAP_VOLTAGE_RANGE;
			double discharging_time = (1 / compute_slf_cap_discharging_rate(sn)) * SLF_CAP_VOLTAGE_RANGE;

			LOG(1, ("SN76477 #%d:        SLF frequency (20,21): %.2f Hz\n", sn->index, 1 / (charging_time + discharging_time)));
		}
		else
		{
			LOG(1, ("SN76477 #%d:        SLF frequency (20,21): N/A\n", sn->index));
		}
	}
	else
	{
		LOG(1, ("SN76477 #%d:        SLF frequency (20,21): External (cap = %.2fV)\n", sn->index, sn->slf_cap_voltage));
	}
}


static void log_vco_pitch_voltage(struct SN76477 *sn)
{
	LOG(1, ("SN76477 #%d:       VCO pitch voltage (19): %.2fV\n", sn->index, sn->pitch_voltage));
}


static void log_vco_duty_cycle(struct SN76477 *sn)
{
	LOG(1, ("SN76477 #%d:       VCO duty cycle (16,19): %.0f%%\n", sn->index, compute_vco_duty_cycle(sn) * 100.0));
}


static void log_vco_freq(struct SN76477 *sn)
{
	if (!sn->vco_cap_voltage_ext)
	{
		if (compute_vco_cap_charging_discharging_rate(sn) > 0)
		{
			double min_freq = compute_vco_cap_charging_discharging_rate(sn) / (2 * VCO_CAP_VOLTAGE_RANGE);
			double max_freq = compute_vco_cap_charging_discharging_rate(sn) / (2 * VCO_TO_SLF_VOLTAGE_DIFF);

			LOG(1, ("SN76477 #%d:        VCO frequency (17,18): %.2f Hz - %.1f Hz\n", sn->index, min_freq, max_freq));
		}
		else
		{
			LOG(1, ("SN76477 #%d:        VCO frequency (17,18): N/A\n", sn->index));
		}
	}
	else
	{
		LOG(1, ("SN76477 #%d:        VCO frequency (17,18): External (cap = %.2fV)\n", sn->index, sn->vco_cap_voltage));
	}
}


static void log_vco_ext_voltage(struct SN76477 *sn)
{
	if (sn->vco_voltage <= VCO_MAX_EXT_VOLTAGE)
	{
		double min_freq = compute_vco_cap_charging_discharging_rate(sn) / (2 * VCO_CAP_VOLTAGE_RANGE);
		double max_freq = compute_vco_cap_charging_discharging_rate(sn) / (2 * VCO_TO_SLF_VOLTAGE_DIFF);

		LOG(1, ("SN76477 #%d:        VCO ext. voltage (16): %.2fV (%.2f Hz)\n", sn->index,
				sn->vco_voltage,
				min_freq + ((max_freq - min_freq) * sn->vco_voltage / VCO_MAX_EXT_VOLTAGE)));
	}
	else
	{
		LOG(1, ("SN76477 #%d:        VCO ext. voltage (16): %.2fV (saturated, no output)\n", sn->index, sn->vco_voltage));
	}
}


static void log_noise_gen_freq(struct SN76477 *sn)
{
	if (sn->noise_clock_ext)
	{
		LOG(1, ("SN76477 #%d:      Noise gen frequency (4): External\n", sn->index));
	}
	else
	{
		if (compute_noise_gen_freq(sn) > 0)
		{
			LOG(1, ("SN76477 #%d:      Noise gen frequency (4): %d Hz\n", sn->index, compute_noise_gen_freq(sn)));
		}
		else
		{
			LOG(1, ("SN76477 #%d:      Noise gen frequency (4): N/A\n", sn->index));
		}
	}
}


static void log_noise_filter_freq(struct SN76477 *sn)
{
	if (!sn->noise_filter_cap_voltage_ext)
	{
		double charging_rate = compute_noise_filter_cap_charging_rate(sn);

		if (charging_rate > 0)
		{
			if (charging_rate < 1000000.0)
			{
				double charging_time = (1 / charging_rate) * NOISE_CAP_VOLTAGE_RANGE;
				double discharging_time = (1 / charging_rate) * NOISE_CAP_VOLTAGE_RANGE;

				LOG(1, ("SN76477 #%d: Noise filter frequency (5,6): %.0f Hz\n", sn->index, 1 / (charging_time + discharging_time)));
			}
			else
			{
				LOG(1, ("SN76477 #%d: Noise filter frequency (5,6): Very Large (Filtering Disabled)\n", sn->index));
			}
		}
		else
		{
			LOG(1, ("SN76477 #%d: Noise filter frequency (5,6): N/A\n", sn->index));
		}
	}
	else
	{
		LOG(1, ("SN76477 #%d: Noise filter frequency (5,6): External (cap = %.2fV)\n", sn->index, sn->noise_filter_cap));
	}
}


static void log_attack_time(struct SN76477 *sn)
{
	if (!sn->attack_decay_cap_voltage_ext)
	{
		if (compute_attack_decay_cap_charging_rate(sn) > 0)
		{
			LOG(1, ("SN76477 #%d:           Attack time (8,10): %.4f sec\n", sn->index, AD_CAP_VOLTAGE_RANGE * (1 / compute_attack_decay_cap_charging_rate(sn))));
		}
		else
		{
			LOG(1, ("SN76477 #%d:           Attack time (8,10): N/A\n", sn->index));
		}
	}
	else
	{
		LOG(1, ("SN76477 #%d:           Attack time (8,10): External (cap = %.2fV)\n", sn->index, sn->attack_decay_cap_voltage));
	}
}


static void log_decay_time(struct SN76477 *sn)
{
	if (!sn->attack_decay_cap_voltage_ext)
	{
		if (compute_attack_decay_cap_discharging_rate(sn) > 0)
		{
			LOG(1, ("SN76477 #%d:             Decay time (7,8): %.4f sec\n", sn->index, AD_CAP_VOLTAGE_RANGE * (1 / compute_attack_decay_cap_discharging_rate(sn))));
		}
		else
		{
			LOG(1, ("SN76477 #%d:            Decay time (8,10): N/A\n", sn->index));
		}
	}
	else
	{
		LOG(1, ("SN76477 #%d:             Decay time (7, 8): External (cap = %.2fV)\n", sn->index, sn->attack_decay_cap_voltage));
	}
}


static void log_voltage_out(struct SN76477 *sn)
{
	LOG(1, ("SN76477 #%d:    Voltage OUT range (11,12): %.2fV - %.2fV (clips above %.2fV)\n",
			sn->index,
			OUT_CENTER_LEVEL_VOLTAGE + compute_center_to_peak_voltage_out(sn) * out_neg_gain[(int)(AD_CAP_VOLTAGE_MAX * 10)],
			OUT_CENTER_LEVEL_VOLTAGE + compute_center_to_peak_voltage_out(sn) * out_pos_gain[(int)(AD_CAP_VOLTAGE_MAX * 10)],
			OUT_HIGH_CLIP_THRESHOLD));
}


static void log_complete_state(struct SN76477 *sn)
{
	log_enable_line(sn);
	log_mixer_mode(sn);
	log_envelope_mode(sn);
	log_vco_mode(sn);
	log_one_shot_time(sn);
	log_slf_freq(sn);
	log_vco_freq(sn);
	log_vco_ext_voltage(sn);
	log_vco_pitch_voltage(sn);
	log_vco_duty_cycle(sn);
	log_noise_filter_freq(sn);
	log_noise_gen_freq(sn);
	log_attack_time(sn);
	log_decay_time(sn);
	log_voltage_out(sn);
}



/*****************************************************************************
 *
 *  .WAV file functions
 *
 *****************************************************************************/

#if LOG_WAV

#include "wavwrite.h"


static void open_wav_file(struct SN76477 *sn)
{
	char wav_file_name[30];

	sprintf(wav_file_name, LOG_WAV_FILE_NAME, sn->index);
	sn->file = wav_open(wav_file_name, sn->sample_rate, 2);

	LOG(1, ("SN76477 #%d:         Logging output: %s\n", sn->index, wav_file_name));
}


static void close_wav_file(struct SN76477 *sn)
{
	wav_close(sn->file);
}


static void add_wav_data(struct SN76477 *sn, INT16 data_l, INT16 data_r)
{
	wav_add_data_16lr(sn->file, &data_l, &data_r, 1);
}

#endif



/*****************************************************************************
 *
 *  Noise generator
 *
 *****************************************************************************/

static void intialize_noise(struct SN76477 *sn)
{
	sn->rng = 0;
}


INLINE UINT32 generate_next_real_noise_bit(struct SN76477 *sn)
{
	UINT32 out = ((sn->rng >> 28) & 1) ^ ((sn->rng >> 0) & 1);

	 /* if bits 0-4 and 28 are all zero then force the output to 1 */
	if ((sn->rng & 0x1000001f) == 0)
	{
		out = 1;
	}

	sn->rng = (sn->rng >> 1) | (out << 30);

	return out;
}



/*****************************************************************************
 *
 *  Set enable input
 *
 *****************************************************************************/

static void _SN76477_enable_w(struct SN76477 *sn, UINT32 data)
{
	sn->enable = data;

	 /* if falling edge */
	if (!sn->enable)
	{
		/* start the attack phase */
		sn->attack_decay_cap_voltage = AD_CAP_VOLTAGE_MIN;

		/* one-shot runs regardless of envelope mode */
		sn->one_shot_running_ff = 1;
	}
}


static void SN76477_test_enable_w(struct SN76477 *sn, UINT32 data)
{
	if (data != sn->enable)
	{
		stream_update(sn->channel);

		_SN76477_enable_w(sn, data);

		log_enable_line(sn);
	}
}


void SN76477_enable_w(int chip, UINT32 data)
{
#if TEST_MODE == 0
	struct SN76477 *sn = sndti_token(SOUND_SN76477, chip);

	CHECK_CHIP_NUM_AND_BOOLEAN;

	SN76477_test_enable_w(sn, data);
#endif
}



/*****************************************************************************
 *
 *  Set mixer select inputs
 *
 *****************************************************************************/

static void _SN76477_mixer_a_w(struct SN76477 *sn, UINT32 data)
{
	sn->mixer_mode = (sn->mixer_mode & ~0x01) | (data << 0);
}


void SN76477_mixer_a_w(int chip, UINT32 data)
{
#if TEST_MODE == 0
	struct SN76477 *sn = sndti_token(SOUND_SN76477, chip);

	CHECK_CHIP_NUM_AND_BOOLEAN;

	if (data != ((sn->mixer_mode >> 0) & 0x01))
	{
		stream_update(sn->channel);

		_SN76477_mixer_a_w(sn, data);

		log_mixer_mode(sn);
	}
#endif
}


static void _SN76477_mixer_b_w(struct SN76477 *sn, UINT32 data)
{
	sn->mixer_mode = (sn->mixer_mode & ~0x02) | (data << 1);
}


void SN76477_mixer_b_w(int chip, UINT32 data)
{
#if TEST_MODE == 0
	struct SN76477 *sn = sndti_token(SOUND_SN76477, chip);

	CHECK_CHIP_NUM_AND_BOOLEAN;

	if (data != ((sn->mixer_mode >> 1) & 0x01))
	{
		stream_update(sn->channel);

		_SN76477_mixer_b_w(sn, data);

		log_mixer_mode(sn);
	}
#endif
}


static void _SN76477_mixer_c_w(struct SN76477 *sn, UINT32 data)
{
	sn->mixer_mode = (sn->mixer_mode & ~0x04) | (data << 2);
}


void SN76477_mixer_c_w(int chip, UINT32 data)
{
#if TEST_MODE == 0
	struct SN76477 *sn = sndti_token(SOUND_SN76477, chip);

	CHECK_CHIP_NUM_AND_BOOLEAN;

	if (data != ((sn->mixer_mode >> 2) & 0x01))
	{
		stream_update(sn->channel);

		_SN76477_mixer_c_w(sn, data);

		log_mixer_mode(sn);
	}
#endif
}



/*****************************************************************************
 *
 *  Set envelope select inputs
 *
 *****************************************************************************/

static void _SN76477_envelope_1_w(struct SN76477 *sn, UINT32 data)
{
	sn->envelope_mode = (sn->envelope_mode & ~0x01) | (data << 0);
}


void SN76477_envelope_1_w(int chip, UINT32 data)
{
#if TEST_MODE == 0
	struct SN76477 *sn = sndti_token(SOUND_SN76477, chip);

	CHECK_CHIP_NUM_AND_BOOLEAN;

	if (data != ((sn->envelope_mode >> 0) & 0x01))
	{
		stream_update(sn->channel);

		_SN76477_envelope_1_w(sn, data);

		log_envelope_mode(sn);
	}
#endif
}


static void _SN76477_envelope_2_w(struct SN76477 *sn, UINT32 data)
{
	sn->envelope_mode = (sn->envelope_mode & ~0x02) | (data << 1);
}


void SN76477_envelope_2_w(int chip, UINT32 data)
{
#if TEST_MODE == 0
	struct SN76477 *sn = sndti_token(SOUND_SN76477, chip);

	CHECK_CHIP_NUM_AND_BOOLEAN;

	if (data != ((sn->envelope_mode >> 1) & 0x01))
	{
		stream_update(sn->channel);

		_SN76477_envelope_2_w(sn, data);

		log_envelope_mode(sn);
	}
#endif
}



/*****************************************************************************
 *
 *  Set VCO select input
 *
 *****************************************************************************/

static void _SN76477_vco_w(struct SN76477 *sn, UINT32 data)
{
	sn->vco_mode = data;
}


void SN76477_vco_w(int chip, UINT32 data)
{
#if TEST_MODE == 0
	struct SN76477 *sn = sndti_token(SOUND_SN76477, chip);

	CHECK_CHIP_NUM_AND_BOOLEAN;

	if (data != sn->vco_mode)
	{
		stream_update(sn->channel);

		_SN76477_vco_w(sn, data);

		log_vco_mode(sn);
	}
#endif
}



/*****************************************************************************
 *
 *  Set one-shot resistor
 *
 *****************************************************************************/

static void _SN76477_one_shot_res_w(struct SN76477 *sn, double data)
{
	sn->one_shot_res = data;
}


void SN76477_one_shot_res_w(int chip, double data)
{
#if TEST_MODE == 0
	struct SN76477 *sn = sndti_token(SOUND_SN76477, chip);

	CHECK_CHIP_NUM_AND_POSITIVE;

	if (data != sn->one_shot_res)
	{
		stream_update(sn->channel);

		_SN76477_one_shot_res_w(sn, data);

		log_one_shot_time(sn);
	}
#endif
}



/*****************************************************************************
 *
 *  Set one-shot capacitor
 *
 *****************************************************************************/

static void _SN76477_one_shot_cap_w(struct SN76477 *sn, double data)
{
	sn->one_shot_cap = data;
}


void SN76477_one_shot_cap_w(int chip, double data)
{
#if TEST_MODE == 0
	struct SN76477 *sn = sndti_token(SOUND_SN76477, chip);

	CHECK_CHIP_NUM_AND_POSITIVE;

	if (data != sn->one_shot_cap)
	{
		stream_update(sn->channel);

		_SN76477_one_shot_cap_w(sn, data);

		log_one_shot_time(sn);
	}
#endif
}



/*****************************************************************************
 *
 *  Set the voltage on the one-shot capacitor
 *
 *****************************************************************************/

void SN76477_one_shot_cap_voltage_w(int chip, double data)
{
#if TEST_MODE == 0
	struct SN76477 *sn = sndti_token(SOUND_SN76477, chip);

	CHECK_CHIP_NUM_AND_CAP_VOLTAGE;

	if (data == SN76477_EXTERNAL_VOLTAGE_DISCONNECT)
	{
		/* switch to internal, if not already */
		if (sn->one_shot_cap_voltage_ext)
		{
			stream_update(sn->channel);

			sn->one_shot_cap_voltage_ext = 0;

			log_one_shot_time(sn);
		}
	}
	else
	{
		/* set the voltage on the cap */
		if (!sn->one_shot_cap_voltage_ext || (data != sn->one_shot_cap_voltage))
		{
			stream_update(sn->channel);

			sn->one_shot_cap_voltage_ext = 1;
			sn->one_shot_cap_voltage = data;

			log_one_shot_time(sn);
		}
	}
#endif
}



/*****************************************************************************
 *
 *  Set SLF resistor
 *
 *****************************************************************************/

static void _SN76477_slf_res_w(struct SN76477 *sn, double data)
{
	sn->slf_res = data;
}


void SN76477_slf_res_w(int chip, double data)
{
#if TEST_MODE == 0
	struct SN76477 *sn = sndti_token(SOUND_SN76477, chip);

	CHECK_CHIP_NUM_AND_POSITIVE;

	if (data != sn->slf_res)
	{
		stream_update(sn->channel);

		_SN76477_slf_res_w(sn, data);

		log_slf_freq(sn);
	}
#endif
}



/*****************************************************************************
 *
 *  Set SLF capacitor
 *
 *****************************************************************************/

static void _SN76477_slf_cap_w(struct SN76477 *sn, double data)
{
	sn->slf_cap = data;
}


void SN76477_slf_cap_w(int chip, double data)
{
#if TEST_MODE == 0
	struct SN76477 *sn = sndti_token(SOUND_SN76477, chip);

	CHECK_CHIP_NUM_AND_POSITIVE;

	if (data != sn->slf_cap)
	{
		stream_update(sn->channel);

		_SN76477_slf_cap_w(sn, data);

		log_slf_freq(sn);
	}
#endif
}



/*****************************************************************************
 *
 *  Set the voltage on the SLF capacitor
 *
 *  This is an alternate way of controlling the VCO as described in the book
 *
 *****************************************************************************/

void SN76477_slf_cap_voltage_w(int chip, double data)
{
#if TEST_MODE == 0
	struct SN76477 *sn = sndti_token(SOUND_SN76477, chip);

	CHECK_CHIP_NUM_AND_CAP_VOLTAGE;

	if (data == SN76477_EXTERNAL_VOLTAGE_DISCONNECT)
	{
		/* switch to internal, if not already */
		if (sn->slf_cap_voltage_ext)
		{
			stream_update(sn->channel);

			sn->slf_cap_voltage_ext = 0;

			log_slf_freq(sn);
		}
	}
	else
	{
		/* set the voltage on the cap */
		if (!sn->slf_cap_voltage_ext || (data != sn->slf_cap_voltage))
		{
			stream_update(sn->channel);

			sn->slf_cap_voltage_ext = 1;
			sn->slf_cap_voltage = data;

			log_slf_freq(sn);
		}
	}
#endif
}



/*****************************************************************************
 *
 *  Set VCO resistor
 *
 *****************************************************************************/

static void _SN76477_vco_res_w(struct SN76477 *sn, double data)
{
	sn->vco_res = data;
}


void SN76477_vco_res_w(int chip, double data)
{
#if TEST_MODE == 0
	struct SN76477 *sn = sndti_token(SOUND_SN76477, chip);

	CHECK_CHIP_NUM_AND_POSITIVE;

	if (data != sn->vco_res)
	{
		stream_update(sn->channel);

		_SN76477_vco_res_w(sn, data);

		log_vco_freq(sn);
	}
#endif
}



/*****************************************************************************
 *
 *  Set VCO capacitor
 *
 *****************************************************************************/

static void _SN76477_vco_cap_w(struct SN76477 *sn, double data)
{
	sn->vco_cap = data;
}


void SN76477_vco_cap_w(int chip, double data)
{
#if TEST_MODE == 0
	struct SN76477 *sn = sndti_token(SOUND_SN76477, chip);

	CHECK_CHIP_NUM_AND_POSITIVE;

	if (data != sn->vco_cap)
	{
		stream_update(sn->channel);

		_SN76477_vco_cap_w(sn, data);

		log_vco_freq(sn);
	}
#endif
}



/*****************************************************************************
 *
 *  Set the voltage on the VCO capacitor
 *
 *****************************************************************************/

void SN76477_vco_cap_voltage_w(int chip, double data)
{
#if TEST_MODE == 0
	struct SN76477 *sn = sndti_token(SOUND_SN76477, chip);

	CHECK_CHIP_NUM_AND_CAP_VOLTAGE;

	if (data == SN76477_EXTERNAL_VOLTAGE_DISCONNECT)
	{
		/* switch to internal, if not already */
		if (sn->vco_cap_voltage_ext)
		{
			stream_update(sn->channel);

			sn->vco_cap_voltage_ext = 0;

			log_vco_freq(sn);
		}
	}
	else
	{
		/* set the voltage on the cap */
		if (!sn->vco_cap_voltage_ext || (data != sn->vco_cap_voltage))
		{
			stream_update(sn->channel);

			sn->vco_cap_voltage_ext = 1;
			sn->vco_cap_voltage = data;

			log_vco_freq(sn);
		}
	}
#endif
}



/*****************************************************************************
 *
 *  Set VCO voltage
 *
 *****************************************************************************/

static void _SN76477_vco_voltage_w(struct SN76477 *sn, double data)
{
	sn->vco_voltage = data;
}


void SN76477_vco_voltage_w(int chip, double data)
{
#if TEST_MODE == 0
	struct SN76477 *sn = sndti_token(SOUND_SN76477, chip);

	CHECK_CHIP_NUM_AND_VOLTAGE;

	if (data != sn->vco_voltage)
	{
		stream_update(sn->channel);

		_SN76477_vco_voltage_w(sn, data);

		log_vco_ext_voltage(sn);
		log_vco_duty_cycle(sn);
	}
#endif
}



/*****************************************************************************
 *
 *  Set pitch voltage
 *
 *****************************************************************************/

static void _SN76477_pitch_voltage_w(struct SN76477 *sn, double data)
{
	sn->pitch_voltage = data;
}


void SN76477_pitch_voltage_w(int chip, double data)
{
#if TEST_MODE == 0
	struct SN76477 *sn = sndti_token(SOUND_SN76477, chip);

	CHECK_CHIP_NUM_AND_VOLTAGE;

	if (data != sn->pitch_voltage)
	{
		stream_update(sn->channel);

		_SN76477_pitch_voltage_w(sn, data);

		log_vco_pitch_voltage(sn);
		log_vco_duty_cycle(sn);
	}
#endif
}



/*****************************************************************************
 *
 *  Set noise external clock
 *
 *****************************************************************************/

void SN76477_noise_clock_w(int chip, UINT32 data)
{
#if TEST_MODE == 0
	struct SN76477 *sn = sndti_token(SOUND_SN76477, chip);

	CHECK_CHIP_NUM_AND_BOOLEAN;

	if (data != sn->noise_clock)
	{
		sn->noise_clock = data;

		/* on the rising edge shift generate next value,
           if external control is enabled */
		if (sn->noise_clock && sn->noise_clock_ext)
		{
			stream_update(sn->channel);

			sn->real_noise_bit_ff = generate_next_real_noise_bit(sn);
		}
	}
#endif
}



/*****************************************************************************
 *
 *  Set noise clock resistor
 *
 *****************************************************************************/

static void _SN76477_noise_clock_res_w(struct SN76477 *sn, double data)
{
	if (data == 0)
	{
		sn->noise_clock_ext = 1;
	}
	else
	{
		sn->noise_clock_ext = 0;

		sn->noise_clock_res = data;
	}
}


void SN76477_noise_clock_res_w(int chip, double data)
{
#if TEST_MODE == 0
	struct SN76477 *sn = sndti_token(SOUND_SN76477, chip);

	CHECK_CHIP_NUM_AND_POSITIVE;

	if (((data == 0) && !sn->noise_clock_ext) ||
		((data != 0) && (data != sn->noise_clock_res)))
	{
		stream_update(sn->channel);

		_SN76477_noise_clock_res_w(sn, data);

		log_noise_gen_freq(sn);
	}
#endif
}



/*****************************************************************************
 *
 *  Set noise filter resistor
 *
 *****************************************************************************/

static void _SN76477_noise_filter_res_w(struct SN76477 *sn, double data)
{
	sn->noise_filter_res = data;
}


void SN76477_noise_filter_res_w(int chip, double data)
{
#if TEST_MODE == 0
	struct SN76477 *sn = sndti_token(SOUND_SN76477, chip);

	CHECK_CHIP_NUM_AND_POSITIVE;

	if (data != sn->noise_filter_res)
	{
		stream_update(sn->channel);

		_SN76477_noise_filter_res_w(sn, data);

		log_noise_filter_freq(sn);
	}
#endif
}



/*****************************************************************************
 *
 *  Set noise filter capacitor
 *
 *****************************************************************************/

static void _SN76477_noise_filter_cap_w(struct SN76477 *sn, double data)
{
	sn->noise_filter_cap = data;
}


void SN76477_noise_filter_cap_w(int chip, double data)
{
#if TEST_MODE == 0
	struct SN76477 *sn = sndti_token(SOUND_SN76477, chip);

	CHECK_CHIP_NUM_AND_POSITIVE;

	if (data != sn->noise_filter_cap)
	{
		stream_update(sn->channel);

		_SN76477_noise_filter_cap_w(sn, data);

		log_noise_filter_freq(sn);
	}
#endif
}



/*****************************************************************************
 *
 *  Set the voltage on the noise filter capacitor
 *
 *****************************************************************************/

void SN76477_noise_filter_cap_voltage_w(int chip, double data)
{
#if TEST_MODE == 0
	struct SN76477 *sn = sndti_token(SOUND_SN76477, chip);

	CHECK_CHIP_NUM_AND_CAP_VOLTAGE;

	if (data == SN76477_EXTERNAL_VOLTAGE_DISCONNECT)
	{
		/* switch to internal, if not already */
		if (sn->noise_filter_cap_voltage_ext)
		{
			stream_update(sn->channel);

			sn->noise_filter_cap_voltage_ext = 0;

			log_noise_filter_freq(sn);
		}
	}
	else
	{
		/* set the voltage on the cap */
		if (!sn->noise_filter_cap_voltage_ext || (data != sn->noise_filter_cap_voltage))
		{
			stream_update(sn->channel);

			sn->noise_filter_cap_voltage_ext = 1;
			sn->noise_filter_cap_voltage = data;

			log_noise_filter_freq(sn);
		}
	}
#endif
}



/*****************************************************************************
 *
 *  Set attack resistor
 *
 *****************************************************************************/

static void _SN76477_attack_res_w(struct SN76477 *sn, double data)
{
	sn->attack_res = data;
}


void SN76477_attack_res_w(int chip, double data)
{
#if TEST_MODE == 0
	struct SN76477 *sn = sndti_token(SOUND_SN76477, chip);

	CHECK_CHIP_NUM_AND_POSITIVE;

	if (data != sn->attack_res)
	{
		stream_update(sn->channel);

		_SN76477_attack_res_w(sn, data);

		log_attack_time(sn);
	}
#endif
}



/*****************************************************************************
 *
 *  Set decay resistor
 *
 *****************************************************************************/

static void _SN76477_decay_res_w(struct SN76477 *sn, double data)
{
	sn->decay_res = data;
}


void SN76477_decay_res_w(int chip, double data)
{
#if TEST_MODE == 0
	struct SN76477 *sn = sndti_token(SOUND_SN76477, chip);

	CHECK_CHIP_NUM_AND_POSITIVE;

	if (data != sn->decay_res)
	{
		stream_update(sn->channel);

		_SN76477_decay_res_w(sn, data);

		log_decay_time(sn);
	}
#endif
}



/*****************************************************************************
 *
 *  Set attack/decay capacitor
 *
 *****************************************************************************/

static void _SN76477_attack_decay_cap_w(struct SN76477 *sn, double data)
{
	sn->attack_decay_cap = data;
}


void SN76477_attack_decay_cap_w(int chip, double data)
{
#if TEST_MODE == 0
	struct SN76477 *sn = sndti_token(SOUND_SN76477, chip);

	CHECK_CHIP_NUM_AND_POSITIVE;

	if (data != sn->attack_decay_cap)
	{
		stream_update(sn->channel);

		_SN76477_attack_decay_cap_w(sn, data);

		log_attack_time(sn);
		log_decay_time(sn);
	}
#endif
}



/*****************************************************************************
 *
 *  Set the voltage on the attack/decay capacitor
 *
 *****************************************************************************/

void SN76477_attack_decay_cap_voltage_w(int chip, double data)
{
#if TEST_MODE == 0
	struct SN76477 *sn = sndti_token(SOUND_SN76477, chip);

	CHECK_CHIP_NUM_AND_CAP_VOLTAGE;

	if (data == SN76477_EXTERNAL_VOLTAGE_DISCONNECT)
	{
		/* switch to internal, if not already */
		if (sn->attack_decay_cap_voltage_ext)
		{
			stream_update(sn->channel);

			sn->attack_decay_cap_voltage_ext = 0;

			log_attack_time(sn);
			log_decay_time(sn);
		}
	}
	else
	{
		/* set the voltage on the cap */
		if (!sn->attack_decay_cap_voltage_ext || (data != sn->attack_decay_cap_voltage))
		{
			stream_update(sn->channel);

			sn->attack_decay_cap_voltage_ext = 1;
			sn->attack_decay_cap_voltage = data;

			log_attack_time(sn);
			log_decay_time(sn);
		}
	}
#endif
}



/*****************************************************************************
 *
 *  Set amplitude resistor
 *
 *****************************************************************************/

static void _SN76477_amplitude_res_w(struct SN76477 *sn, double data)
{
	sn->amplitude_res = data;
}


void SN76477_amplitude_res_w(int chip, double data)
{
#if TEST_MODE == 0
	struct SN76477 *sn = sndti_token(SOUND_SN76477, chip);

	CHECK_CHIP_NUM_AND_POSITIVE;

	if (data != sn->amplitude_res)
	{
		stream_update(sn->channel);

		_SN76477_amplitude_res_w(sn, data);

		log_voltage_out(sn);
	}
#endif
}



/*****************************************************************************
 *
 *  Set feedback resistor
 *
 *****************************************************************************/

static void _SN76477_feedback_res_w(struct SN76477 *sn, double data)
{
	sn->feedback_res = data;
}


void SN76477_feedback_res_w(int chip, double data)
{
#if TEST_MODE == 0
	struct SN76477 *sn = sndti_token(SOUND_SN76477, chip);

	CHECK_CHIP_NUM_AND_POSITIVE;

	if (data != sn->feedback_res)
	{
		stream_update(sn->channel);

		_SN76477_feedback_res_w(sn, data);

		log_voltage_out(sn);
	}
#endif
}



/*****************************************************************************
 *
 *  Sample generation
 *
 *****************************************************************************/

static void SN76477_update(void *param, stream_sample_t **inputs, stream_sample_t **_buffer, int length)
{
	double one_shot_cap_charging_step;
	double one_shot_cap_discharging_step;
	double slf_cap_charging_step;
	double slf_cap_discharging_step;
	double vco_duty_cycle_multiplier;
	double vco_cap_charging_step;
	double vco_cap_discharging_step;
	double vco_cap_voltage_max;
	UINT32 noise_gen_freq;
	double noise_filter_cap_charging_step;
	double noise_filter_cap_discharging_step;
	double attack_decay_cap_charging_step;
	double attack_decay_cap_discharging_step;
	int    attack_decay_cap_charging;
	double voltage_out;
	double center_to_peak_voltage_out;

	struct SN76477 *sn = param;
	stream_sample_t *buffer = _buffer[0];


#if TEST_MODE
	static int recursing = 0;	/* we need to prevent recursion since enable_w calls stream_update */

	if (input_code_pressed_once(KEYCODE_SPACE) && !recursing)
	{
		recursing = 1;

		sound_global_enable(1);
		SN76477_test_enable_w(sn, !sn->enable);
	}

	recursing = 0;
#endif

	/* compute charging values, doing it here ensures that we always use the latest values */
	one_shot_cap_charging_step = compute_one_shot_cap_charging_rate(sn) / sn->sample_rate;
	one_shot_cap_discharging_step = compute_one_shot_cap_discharging_rate(sn) / sn->sample_rate;

	slf_cap_charging_step = compute_slf_cap_charging_rate(sn) / sn->sample_rate;
	slf_cap_discharging_step = compute_slf_cap_discharging_rate(sn) / sn->sample_rate;

	vco_duty_cycle_multiplier = (1 - compute_vco_duty_cycle(sn)) * 2;
	vco_cap_charging_step = compute_vco_cap_charging_discharging_rate(sn) / vco_duty_cycle_multiplier / sn->sample_rate;
	vco_cap_discharging_step = compute_vco_cap_charging_discharging_rate(sn) * vco_duty_cycle_multiplier / sn->sample_rate;

	noise_filter_cap_charging_step = compute_noise_filter_cap_charging_rate(sn) / sn->sample_rate;
	noise_filter_cap_discharging_step = compute_noise_filter_cap_discharging_rate(sn) / sn->sample_rate;
	noise_gen_freq = compute_noise_gen_freq(sn);

	attack_decay_cap_charging_step = compute_attack_decay_cap_charging_rate(sn) / sn->sample_rate;
	attack_decay_cap_discharging_step = compute_attack_decay_cap_discharging_rate(sn) / sn->sample_rate;

	center_to_peak_voltage_out = compute_center_to_peak_voltage_out(sn);


	/* process 'length' number of samples */
	while (length--)
	{
		/* update the one-shot cap voltage */
		if (!sn->one_shot_cap_voltage_ext)
		{
			if (sn->one_shot_running_ff)
			{
				/* charging */
				sn->one_shot_cap_voltage = min(sn->one_shot_cap_voltage + one_shot_cap_charging_step, ONE_SHOT_CAP_VOLTAGE_MAX);
			}
			else
			{
				/* discharging */
				sn->one_shot_cap_voltage = max(sn->one_shot_cap_voltage - one_shot_cap_discharging_step, ONE_SHOT_CAP_VOLTAGE_MIN);
			}
		}

		if (sn->one_shot_cap_voltage >= ONE_SHOT_CAP_VOLTAGE_MAX)
		{
			sn->one_shot_running_ff = 0;
		}


		/* update the SLF (super low frequency oscillator) */
		if (!sn->slf_cap_voltage_ext)
		{
			/* internal */
			if (!sn->slf_out_ff)
			{
				/* charging */
				sn->slf_cap_voltage = min(sn->slf_cap_voltage + slf_cap_charging_step, SLF_CAP_VOLTAGE_MAX);
			}
			else
			{
				/* discharging */
				sn->slf_cap_voltage = max(sn->slf_cap_voltage - slf_cap_discharging_step, SLF_CAP_VOLTAGE_MIN);
			}
		}

		if (sn->slf_cap_voltage >= SLF_CAP_VOLTAGE_MAX)
		{
			sn->slf_out_ff = 1;
		}
		else if (sn->slf_cap_voltage <= SLF_CAP_VOLTAGE_MIN)
		{
			sn->slf_out_ff = 0;
		}


		/* update the VCO (voltage controlled oscillator) */
		if (sn->vco_mode)
		{
			/* VCO is controlled by SLF */
			vco_cap_voltage_max =  sn->slf_cap_voltage + VCO_TO_SLF_VOLTAGE_DIFF;
		}
		else
		{
			/* VCO is controlled by external voltage */
			vco_cap_voltage_max = sn->vco_voltage + VCO_TO_SLF_VOLTAGE_DIFF;
		}

		if (!sn->vco_cap_voltage_ext)
		{
			if (!sn->vco_out_ff)
			{
				/* charging */
				sn->vco_cap_voltage = min(sn->vco_cap_voltage + vco_cap_charging_step, vco_cap_voltage_max);
			}
			else
			{
				/* discharging */
				sn->vco_cap_voltage = max(sn->vco_cap_voltage - vco_cap_discharging_step, VCO_CAP_VOLTAGE_MIN);
			}
		}

		if (sn->vco_cap_voltage >= vco_cap_voltage_max)
		{
			if (!sn->vco_out_ff)
			{
				/* positive edge */
				sn->vco_alt_pos_edge_ff = !sn->vco_alt_pos_edge_ff;
			}

			sn->vco_out_ff = 1;
		}
		else if (sn->vco_cap_voltage <= VCO_CAP_VOLTAGE_MIN)
		{
			sn->vco_out_ff = 0;
		}


		/* update the noise generator */
		while (!sn->noise_clock_ext && (sn->noise_gen_count <= noise_gen_freq))
		{
			sn->noise_gen_count = sn->noise_gen_count + sn->sample_rate;

			sn->real_noise_bit_ff = generate_next_real_noise_bit(sn);
		}

		sn->noise_gen_count = sn->noise_gen_count - noise_gen_freq;


		/* update the noise filter */
		if (!sn->noise_filter_cap_voltage_ext)
		{
			/* internal */
			if (sn->real_noise_bit_ff)
			{
				/* charging */
				sn->noise_filter_cap_voltage = min(sn->noise_filter_cap_voltage + noise_filter_cap_charging_step, NOISE_CAP_VOLTAGE_MAX);
			}
			else
			{
				/* discharging */
				sn->noise_filter_cap_voltage = max(sn->noise_filter_cap_voltage - noise_filter_cap_discharging_step, NOISE_CAP_VOLTAGE_MIN);
			}
		}

		/* check the thresholds */
		if (sn->noise_filter_cap_voltage >= NOISE_CAP_HIGH_THRESHOLD)
		{
			sn->filtered_noise_bit_ff = 0;
		}
		else if (sn->noise_filter_cap_voltage <= NOISE_CAP_LOW_THRESHOLD)
		{
			sn->filtered_noise_bit_ff = 1;
		}


		/* based on the envelope mode figure out the attack/decay phase we are in */
		switch (sn->envelope_mode)
		{
		case 0:		/* VCO */
			attack_decay_cap_charging = sn->vco_out_ff;
			break;

		case 1:		/* one-shot */
			attack_decay_cap_charging = sn->one_shot_running_ff;
			break;

		case 2:
		default:	/* mixer only */
			attack_decay_cap_charging = 1;	/* never a decay phase */
			break;

		case 3:		/* VCO with alternating polarity */
			attack_decay_cap_charging = sn->vco_out_ff && sn->vco_alt_pos_edge_ff;
			break;
		}


		/* update a/d cap voltage */
		if (!sn->attack_decay_cap_voltage_ext)
		{
			if (attack_decay_cap_charging)
			{
				if (attack_decay_cap_charging_step > 0)
				{
					sn->attack_decay_cap_voltage = min(sn->attack_decay_cap_voltage + attack_decay_cap_charging_step, AD_CAP_VOLTAGE_MAX);
				}
				else
				{
					/* no attack, voltage to max instantly */
					sn->attack_decay_cap_voltage = AD_CAP_VOLTAGE_MAX;
				}
			}
			else
			{
				/* discharging */
				if (attack_decay_cap_discharging_step > 0)
				{
					sn->attack_decay_cap_voltage = max(sn->attack_decay_cap_voltage - attack_decay_cap_discharging_step, AD_CAP_VOLTAGE_MIN);
				}
				else
				{
					/* no decay, voltage to min instantly */
					sn->attack_decay_cap_voltage = AD_CAP_VOLTAGE_MIN;
				}
			}
		}


		/* mix the output, if enabled, or not saturated by the VCO */
		if (!sn->enable && (sn->vco_cap_voltage <= VCO_CAP_VOLTAGE_MAX))
		{
			UINT32 out;

			/* enabled */
			switch (sn->mixer_mode)
			{
			case 0:		/* VCO */
				out = sn->vco_out_ff;
				break;

			case 1:		/* SLF */
				out = sn->slf_out_ff;
				break;

			case 2:		/* noise */
				out = sn->filtered_noise_bit_ff;
				break;

			case 3:		/* VCO and noise */
				out = sn->vco_out_ff & sn->filtered_noise_bit_ff;
				break;

			case 4:		/* SLF and noise */
				out = sn->slf_out_ff & sn->filtered_noise_bit_ff;
				break;

			case 5:		/* VCO, SLF and noise */
				out = sn->vco_out_ff & sn->slf_out_ff & sn->filtered_noise_bit_ff;
				break;

			case 6:		/* VCO and SLF */
				out = sn->vco_out_ff & sn->slf_out_ff;
				break;

			case 7:		/* inhibit */
			default:
				out = 0;
				break;
			}

			/* determine the OUT voltage from the attack/delay cap voltage and clip it */
			if (out)
			{
				voltage_out = OUT_CENTER_LEVEL_VOLTAGE + center_to_peak_voltage_out * out_pos_gain[(int)(sn->attack_decay_cap_voltage * 10)],
				voltage_out = min(voltage_out, OUT_HIGH_CLIP_THRESHOLD);
			}
			else
			{
				voltage_out = OUT_CENTER_LEVEL_VOLTAGE + center_to_peak_voltage_out * out_neg_gain[(int)(sn->attack_decay_cap_voltage * 10)],
				voltage_out = max(voltage_out, OUT_LOW_CLIP_THRESHOLD);
			}
		}
		else
		{
			/* disabled */
			voltage_out = OUT_CENTER_LEVEL_VOLTAGE;
		}


		/* convert it to a signed 16-bit sample,
           -32767 = OUT_LOW_CLIP_THRESHOLD
                0 = OUT_CENTER_LEVEL_VOLTAGE
            32767 = 2 * OUT_CENTER_LEVEL_VOLTAGE + OUT_LOW_CLIP_THRESHOLD

                      / Vout - Vmin    \
            sample = |  ----------- - 1 | * 32767
                      \ Vcen - Vmin    /
         */
		*buffer++ = (((voltage_out - OUT_LOW_CLIP_THRESHOLD) / (OUT_CENTER_LEVEL_VOLTAGE - OUT_LOW_CLIP_THRESHOLD)) - 1) * 32767;

#if LOG_WAV
#if LOG_WAV_ENABLED_ONLY
		if (!sn->enable)
#endif
		{
			INT16 log_data_l;
			INT16 log_data_r;

#if LOG_WAV_VALUE_L == 0
			log_data_l = LOG_WAV_GAIN_FACTOR * voltage_out;
#elif LOG_WAV_VALUE_L == 1
			log_data_l = LOG_WAV_GAIN_FACTOR * sn->enable;
#elif LOG_WAV_VALUE_L == 2
			log_data_l = LOG_WAV_GAIN_FACTOR * sn->one_shot_cap_voltage;
#elif LOG_WAV_VALUE_L == 3
			log_data_l = LOG_WAV_GAIN_FACTOR * sn->attack_decay_cap_voltage;
#elif LOG_WAV_VALUE_L == 4
			log_data_l = LOG_WAV_GAIN_FACTOR * sn->slf_cap_voltage;
#elif LOG_WAV_VALUE_L == 5
			log_data_l = LOG_WAV_GAIN_FACTOR * sn->vco_cap_voltage;
#elif LOG_WAV_VALUE_L == 6
			log_data_l = LOG_WAV_GAIN_FACTOR * sn->noise_filter_cap_voltage;
#endif

#if LOG_WAV_VALUE_R == 0
			log_data_r = LOG_WAV_GAIN_FACTOR * voltage_out;
#elif LOG_WAV_VALUE_R == 1
			log_data_r = LOG_WAV_GAIN_FACTOR * sn->enable;
#elif LOG_WAV_VALUE_R == 2
			log_data_r = LOG_WAV_GAIN_FACTOR * sn->one_shot_cap_voltage;
#elif LOG_WAV_VALUE_R == 3
			log_data_r = LOG_WAV_GAIN_FACTOR * sn->attack_decay_cap_voltage;
#elif LOG_WAV_VALUE_R == 4
			log_data_r = LOG_WAV_GAIN_FACTOR * sn->slf_cap_voltage;
#elif LOG_WAV_VALUE_R == 5
			log_data_r = LOG_WAV_GAIN_FACTOR * sn->vco_cap_voltage;
#elif LOG_WAV_VALUE_R == 6
			log_data_r = LOG_WAV_GAIN_FACTOR * sn->noise_filter_cap_voltage;
#endif
			add_wav_data(sn, log_data_l, log_data_r);
		}
#endif
	}
}



/*****************************************************************************
 *
 *  State saving
 *
 *****************************************************************************/

static void state_save_register(struct SN76477 *sn)
{
	state_save_register_item("sn76744", sn->index, sn->enable);
	state_save_register_item("sn76744", sn->index, sn->envelope_mode);
	state_save_register_item("sn76744", sn->index, sn->vco_mode);
	state_save_register_item("sn76744", sn->index, sn->mixer_mode);

	state_save_register_item("sn76744", sn->index, sn->one_shot_res);
	state_save_register_item("sn76744", sn->index, sn->one_shot_cap);
	state_save_register_item("sn76744", sn->index, sn->one_shot_cap_voltage_ext);

	state_save_register_item("sn76744", sn->index, sn->slf_res);
	state_save_register_item("sn76744", sn->index, sn->slf_cap);
	state_save_register_item("sn76744", sn->index, sn->slf_cap_voltage_ext);

	state_save_register_item("sn76744", sn->index, sn->vco_voltage);
	state_save_register_item("sn76744", sn->index, sn->vco_res);
	state_save_register_item("sn76744", sn->index, sn->vco_cap);
	state_save_register_item("sn76744", sn->index, sn->vco_cap_voltage_ext);

	state_save_register_item("sn76744", sn->index, sn->noise_clock_res);
	state_save_register_item("sn76744", sn->index, sn->noise_clock_ext);
	state_save_register_item("sn76744", sn->index, sn->noise_clock);
	state_save_register_item("sn76744", sn->index, sn->noise_filter_res);
	state_save_register_item("sn76744", sn->index, sn->noise_filter_cap);
	state_save_register_item("sn76744", sn->index, sn->noise_filter_cap_voltage_ext);

	state_save_register_item("sn76744", sn->index, sn->attack_res);
	state_save_register_item("sn76744", sn->index, sn->decay_res);
	state_save_register_item("sn76744", sn->index, sn->attack_decay_cap);
	state_save_register_item("sn76744", sn->index, sn->attack_decay_cap_voltage_ext);

	state_save_register_item("sn76744", sn->index, sn->amplitude_res);
	state_save_register_item("sn76744", sn->index, sn->feedback_res);
	state_save_register_item("sn76744", sn->index, sn->pitch_voltage);

	state_save_register_item("sn76744", sn->index, sn->one_shot_cap_voltage);
	state_save_register_item("sn76744", sn->index, sn->one_shot_running_ff);

	state_save_register_item("sn76744", sn->index, sn->slf_cap_voltage);
	state_save_register_item("sn76744", sn->index, sn->slf_out_ff);

	state_save_register_item("sn76744", sn->index, sn->vco_cap_voltage);
	state_save_register_item("sn76744", sn->index, sn->vco_out_ff);
	state_save_register_item("sn76744", sn->index, sn->vco_alt_pos_edge_ff);

	state_save_register_item("sn76744", sn->index, sn->noise_filter_cap_voltage);
	state_save_register_item("sn76744", sn->index, sn->real_noise_bit_ff);
	state_save_register_item("sn76744", sn->index, sn->filtered_noise_bit_ff);
	state_save_register_item("sn76744", sn->index, sn->noise_gen_count);

	state_save_register_item("sn76744", sn->index, sn->attack_decay_cap_voltage);

	state_save_register_item("sn76744", sn->index, sn->rng);
}



/*****************************************************************************
 *
 *  Sound interface glue functions
 *
 *****************************************************************************/

static void *sn76477_start(int sndindex, int clock, const void *config)
{
	struct SN76477 *sn;
	struct SN76477interface *intf;


#if TEST_MODE == 0
	intf = (struct SN76477interface *)config;
#else
	intf = &test_interface;
#endif


	sn = auto_malloc(sizeof(*sn));
	memset(sn, 0, sizeof(*sn));

	sn->index = sndindex;

	sn->channel = stream_create(0, 1, Machine->sample_rate, sn, SN76477_update);

	if (clock > 0)
	{
		sn->sample_rate = clock;
	}
	else
	{
		sn->sample_rate = Machine->sample_rate;
	}

	intialize_noise(sn);

	sndintrf_register_token(sn);

	/* set up interface values */
	_SN76477_enable_w(sn, intf->enable);
	_SN76477_vco_w(sn, intf->vco);
	_SN76477_mixer_a_w(sn, intf->mixer_a);
	_SN76477_mixer_b_w(sn, intf->mixer_b);
	_SN76477_mixer_c_w(sn, intf->mixer_c);
	_SN76477_envelope_1_w(sn, intf->envelope_1);
	_SN76477_envelope_2_w(sn, intf->envelope_2);
	_SN76477_one_shot_res_w(sn, intf->one_shot_res);
	_SN76477_one_shot_cap_w(sn, intf->one_shot_cap);
	_SN76477_slf_res_w(sn, intf->slf_res);
	_SN76477_slf_cap_w(sn, intf->slf_cap);
	_SN76477_vco_res_w(sn, intf->vco_res);
	_SN76477_vco_cap_w(sn, intf->vco_cap);
	_SN76477_vco_voltage_w(sn, intf->vco_voltage);
	_SN76477_noise_clock_res_w(sn, intf->noise_clock_res);
	_SN76477_noise_filter_res_w(sn, intf->noise_filter_res);
	_SN76477_noise_filter_cap_w(sn, intf->noise_filter_cap);
	_SN76477_decay_res_w(sn, intf->decay_res);
	_SN76477_attack_res_w(sn, intf->attack_res);
	_SN76477_attack_decay_cap_w(sn, intf->attack_decay_cap);
	_SN76477_amplitude_res_w(sn, intf->amplitude_res);
	_SN76477_feedback_res_w(sn, intf->feedback_res);
	_SN76477_pitch_voltage_w(sn, intf->pitch_voltage);

	sn->one_shot_cap_voltage = ONE_SHOT_CAP_VOLTAGE_MIN;
	sn->slf_cap_voltage = SLF_CAP_VOLTAGE_MIN;
	sn->vco_cap_voltage = VCO_CAP_VOLTAGE_MIN;
	sn->noise_filter_cap_voltage = NOISE_CAP_VOLTAGE_MIN;
	sn->attack_decay_cap_voltage = AD_CAP_VOLTAGE_MIN;

	state_save_register(sn);

	log_complete_state(sn);

#if LOG_WAV
	open_wav_file(sn);
#endif

	return sn;
}


#if LOG_WAV
static void sn76477_stop(void *token)
{
	struct SN76477 *sn = (struct SN76477 *)token;

	close_wav_file(sn);
}
#endif


void sn76477_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
	case SNDINFO_PTR_START:			info->start = sn76477_start; break;
#if LOG_WAV
	case SNDINFO_PTR_STOP:			info->stop = sn76477_stop; break;
#endif
	case SNDINFO_STR_NAME:			info->s = "SN76477"; break;
	case SNDINFO_STR_CORE_FAMILY:	info->s = "Analog"; break;
	case SNDINFO_STR_CORE_VERSION:	info->s = "2.1"; break;
	case SNDINFO_STR_CORE_FILE:		info->s = __FILE__; break;
	case SNDINFO_STR_CORE_CREDITS:	info->s = "Copyright (c) 2007, The MAME Team"; break;
	}
}

