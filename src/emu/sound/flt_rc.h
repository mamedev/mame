#ifndef FLT_RC_H
#define FLT_RC_H

#include "rescap.h"

#define FLT_RC_LOWPASS		0
#define FLT_RC_HIGHPASS		1
#define FLT_RC_AC			2

/*
 * FLT_RC_LOWPASS:
 *
 * signal >--R1--+--R2--+
 *               |      |
 *               C      R3---> amp
 *               |      |
 *              GND    GND
 *
 * Set C=0 to disable filter
 *
 * FLT_RC_HIGHPASS:
 *
 * signal >--C---+----> amp
 *               |
 *               R1
 *               |
 *              GND
 *
 * Set C = 0 to disable filter
 *
 * FLT_RC_AC:
 *
 * Same as FLT_RC_HIGHPASS, but with standard frequency of 16 HZ
 * This filter may be setup just with
 *
 * MDRV_SOUND_ADD(FILTER_RC, 0)
 * MDRV_SOUND_CONFIG(&flt_rc_ac_default)
 *
 * Default behaviour:
 *
 * Without MDRV_SOUND_CONFIG, a disabled FLT_RC_LOWPASS is created
 *
 */

typedef struct _flt_rc_config flt_rc_config;
struct _flt_rc_config
{
	int	type;
	double	R1;
	double	R2;
	double	R3;
	double	C;
};

extern flt_rc_config flt_rc_ac_default;

void filter_rc_set_RC(int num, int type, double R1, double R2, double R3, double C);

#endif
