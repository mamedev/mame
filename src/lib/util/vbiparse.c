/***************************************************************************

    vbiparse.c

    Parse Philips codes and other data from VBI lines.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "osdcore.h"



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

int vbi_parse_manchester_code(const UINT16 *source, int sourcewidth, int sourceshift, int expectedbits, UINT8 *result)
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
		int leftbit = firstedge + ((double)x - 0.25) * bestclock;
		int rightbit = firstedge + ((double)x + 0.25) * bestclock;
		int left = srcabs[leftbit];
		int right = srcabs[rightbit];

		/* all bits should be marked by transitions; fail if we don't get one */
		if (left == right)
			return 0;
		result[x] = (left < right);
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
