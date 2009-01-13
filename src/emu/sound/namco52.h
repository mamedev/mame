#pragma once

#ifndef __NAMCO52_H__
#define __NAMCO52_H__

/* While a little confusing, this interface uses 2 gains.
 *
 * "mixing_level" is the relative level of the signal
 * compared to other effects before entering the filter.
 *
 * "filt_gain" is the combined gain of the filters.
 *
 * If I did not do it this way, then the filters could
 * cause the signal to go beyond the 16bit range.
 *
 * If "play_rate" is 0 (ground) then the sample clock rate
 * defaults to the 52xx internal sample clock. (baseclock/384)
 */

typedef struct _namco_52xx_interface namco_52xx_interface;
struct _namco_52xx_interface
{
	double	play_rate;		/* Playback frequency */
	double	hp_filt_fc;
	double	hp_filt_q;
	double	lp_filt_fc;
	double	lp_filt_q;
	double	filt_gain;
};

void namcoio_52xx_write(int data);

SND_GET_INFO( namco_52xx );
#define SOUND_NAMCO_52XX SND_GET_INFO_NAME( namco_52xx )

#endif /* __NAMCO52_H__ */

