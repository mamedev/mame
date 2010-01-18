#pragma once

#ifndef __FLT_RC_H__
#define FLT_RC_H

#include "machine/rescap.h"

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
 * MDRV_SOUND_ADD("tag", FILTER_RC, 0)
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

extern const flt_rc_config flt_rc_ac_default;

void filter_rc_set_RC(running_device *device, int type, double R1, double R2, double R3, double C);

DEVICE_GET_INFO( filter_rc );
#define SOUND_FILTER_RC DEVICE_GET_INFO_NAME( filter_rc )

#endif /* __FLT_RC_H__ */
