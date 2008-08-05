/***************************************************************************

    vbiparse.c

    Parse Philips codes and other data from VBI lines.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "osdcore.h"
#include "vbiparse.h"
#include <math.h>



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_SOURCE_WIDTH	1024
#define MAX_CLOCK_DIFF		3



/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    vbi_parse_manchester_code - parse a Manchester
    code from a line of video data
-------------------------------------------------*/

int vbi_parse_manchester_code(const UINT16 *source, int sourcewidth, int sourceshift, int expectedbits, UINT32 *result)
{
	UINT8 srcabs[MAX_SOURCE_WIDTH];
	UINT8 min, max, mid, srcabsval;
	double clock, bestclock;
	int x, firstedge;
	int besterr;

	/* fail if the width is too large */
	if (sourcewidth > MAX_SOURCE_WIDTH)
		return 0;

	/* find highs and lows in the line */
	min = 0xff;
	max = 0x00;
	for (x = 0; x < sourcewidth; x++)
	{
		UINT8 rawsrc = source[x] >> sourceshift;
		min = MIN(min, rawsrc);
		max = MAX(max, rawsrc);
	}

	/* bail if the line is all black or all white */
	if (max < 0x80 || min > 0x80)
		return 0;

	/* determine the midpoint and then set the thresholds to be halfway */
	mid = (min + max) / 2;
	min = mid - (mid - min) / 2;
	max = mid + (max - mid) / 2;

	/* convert the source into absolute high/low  */
	srcabsval = (source[0] > mid);
	for (x = 0; x < sourcewidth; x++)
	{
		UINT8 rawsrc = source[x] >> sourceshift;
		if (rawsrc >= max)
			srcabsval = 1;
		else if (rawsrc <= min)
			srcabsval = 0;
		srcabs[x] = srcabsval;
	}

	/* find the first transition; this is assumed to be the middle of the first bit */
	for (x = 0; x < sourcewidth - 1; x++)
		if (srcabs[x] != srcabs[x + 1])
			break;
	if (x == sourcewidth - 1)
		return 0;
	firstedge = x;

	/* now scan to find a clock that has a nearby transition on each beat */
	bestclock = 0;
	besterr = 1000;
	for (clock = (double)sourcewidth / (double)expectedbits; clock >= 2.0; clock -= 1.0 / (double)expectedbits)
	{
		int error = 0;

		/* scan for all the expected bits */
		for (x = 1; x < expectedbits; x++)
		{
			int curbit = firstedge + (double)x * clock;
			int offby;

			/* look for a match that is off by an amount up to the maximum */
			for (offby = 0; offby <= MAX_CLOCK_DIFF; offby++)
				if (srcabs[curbit + offby + 0] != srcabs[curbit + offby + 1] || srcabs[curbit - offby + 0] != srcabs[curbit - offby + 1])
					break;
			
			/* if we never found the edge, fail immediately */
			if (offby > MAX_CLOCK_DIFF)
				break;

			/* only continue if we're still in the running */
			error += offby;
			if (error >= besterr)
				break;
		}

		/* if we got to the end, this is the best candidate so far */
		if (x == expectedbits)
		{
			besterr = error;
			bestclock = clock;
		}
	}

	/* if nobody matched, fail */
	if (bestclock == 0)
		return 0;

	/* now extract the bits */
	for (x = 0; x < expectedbits; x++)
	{
		int leftstart = firstedge + ceil(((double)x - 0.5) * bestclock);
//		int leftmid = firstedge + ((double)x - 0.25) * bestclock;
		int leftend = firstedge + floor(((double)x - 0.0) * bestclock);
		int rightstart = firstedge + ceil(((double)x + 0.0) * bestclock);
//		int rightmid = firstedge + ((double)x + 0.25) * bestclock;
		int rightend = firstedge + floor(((double)x + 0.5) * bestclock);
		int leftavg, rightavg, leftabs, rightabs;
		int confidence = 0;
		int tx;

		/* compute left and right average values */
		leftavg = 0;
		for (tx = leftstart; tx <= leftend; tx++)
			leftavg += (UINT8)(source[tx] >> sourceshift) - mid;
		leftabs = (leftavg >= 0);
		leftavg = (leftavg < 0) ? -leftavg : leftavg;

		rightavg = 0;
		for (tx = rightstart; tx <= rightend; tx++)
			rightavg += (UINT8)(source[tx] >> sourceshift) - mid;
		rightabs = (rightavg >= 0);
		rightavg = (rightavg < 0) ? -rightavg : rightavg;
		
		/* all bits should be marked by transitions; fail if we don't get one */
		if (leftabs == rightabs)
			return 0;
		
		/* store the bit and its confidence level */
		confidence = leftavg + rightavg;
		result[x] = (leftabs < rightabs) | (confidence << 1);
	}
	return expectedbits;
}


/*-------------------------------------------------
    vbi_parse_white_flag - compute the "white
    flag" from a line of video data
-------------------------------------------------*/

int vbi_parse_white_flag(const UINT16 *source, int sourcewidth, int sourceshift)
{
	int minval = 0xff;
	int maxval = 0x00;
	int avgval = 0x00;
	int diff;
	int x;

	/* compute minimum, maximum, and average values across the line */
	for (x = 0; x < sourcewidth; x++)
	{
		UINT8 yval = source[x] >> sourceshift;
		minval = MIN(yval, minval);
		maxval = MAX(yval, maxval);
		avgval += yval;
	}
	avgval /= sourcewidth;
	diff = maxval - minval;

	/* if there's a spread of at least 0x20, and the average is above 3/4 of the center, call it good */
	return (diff >= 0x20) && (avgval >= minval + 3 * diff / 4);
}


/*-------------------------------------------------
    vbi_parse_all - parse everything from a video 
    frame
-------------------------------------------------*/

void vbi_parse_all(const UINT16 *source, int sourcerowpixels, int sourcewidth, int sourceshift, vbi_metadata *vbi)
{
	UINT32 bits[2][24];
	UINT8 bitnum;

	/* first reset it all */
	memset(vbi, 0, sizeof(*vbi));
	
	/* get the white flag */
	vbi->white = vbi_parse_white_flag(source + 11 * sourcerowpixels, sourcewidth, sourceshift);
	
	/* parse line 16 */
	if (vbi_parse_manchester_code(source + 16 * sourcerowpixels, sourcewidth, sourceshift, 24, bits[0]) == 24)
		for (bitnum = 0; bitnum < 24; bitnum++)
			vbi->line16 = (vbi->line16 << 1) | (bits[0][bitnum] & 1);

	/* parse line 17 */
	if (vbi_parse_manchester_code(source + 17 * sourcerowpixels, sourcewidth, sourceshift, 24, bits[0]) == 24)
		for (bitnum = 0; bitnum < 24; bitnum++)
			vbi->line17 = (vbi->line17 << 1) | (bits[0][bitnum] & 1);

	/* parse line 18 */
	if (vbi_parse_manchester_code(source + 18 * sourcerowpixels, sourcewidth, sourceshift, 24, bits[1]) == 24)
		for (bitnum = 0; bitnum < 24; bitnum++)
			vbi->line18 = (vbi->line18 << 1) | (bits[1][bitnum] & 1);

	/* pick the best out of lines 17/18 */
	/* if we only got one or the other, that's all we have */
	if (vbi->line17 == 0)
		vbi->line1718 = vbi->line18;
	else if (vbi->line18 == 0)
		vbi->line1718 = vbi->line17;
	
	/* if they agree, we're golden */
	else if (vbi->line17 == vbi->line18)
		vbi->line1718 = vbi->line17;
	
	/* if they don't agree, we have to pick one */
	else
	{
		/* if both are frame numbers, and one is not valid BCD, pick the other */
		if ((vbi->line17 & 0xf80000) == 0xf80000 && (vbi->line18 & 0xf80000) == 0xf80000)
		{
			if ((vbi->line17 & 0xf000) > 0x9000 || (vbi->line17 & 0xf00) > 0x900 || (vbi->line17 & 0xf0) > 0x90 || (vbi->line17 & 0xf) > 0x9)
				vbi->line1718 = vbi->line18;
			else if ((vbi->line18 & 0xf000) > 0x9000 || (vbi->line18 & 0xf00) > 0x900 || (vbi->line18 & 0xf0) > 0x90 || (vbi->line18 & 0xf) > 0x9)
				vbi->line1718 = vbi->line17;
		}

		/* if still nothing, then scan through the bits and pick the ones with the most confidence */
		if (vbi->line1718 == 0)
			for (bitnum = 0; bitnum < 24; bitnum++)
				vbi->line1718 = (vbi->line1718 << 1) | ((bits[0][bitnum] > bits[1][bitnum]) ? (bits[0][bitnum] & 1) : (bits[1][bitnum] & 1));
	}
}
