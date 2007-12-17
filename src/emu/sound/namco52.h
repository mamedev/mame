#ifndef namco52_h
#define namco52_h

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

struct namco_52xx_interface
{
	int		region;			/* memory region */
	double	play_rate;		/* Playback frequency */
	double	hp_filt_fc;
	double	hp_filt_q;
	double	lp_filt_fc;
	double	lp_filt_q;
	double	filt_gain;
};

void namcoio_52XX_write(int data);

#endif

