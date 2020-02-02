// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    vbiparse.c

    Parse Philips codes and other data from VBI lines.

***************************************************************************/

#include "osdcore.h"
#include "vbiparse.h"
#include <cmath>
#include <algorithm>


/***************************************************************************
    DEBUGGING
***************************************************************************/

#define PRINTF_WHITE_FLAG   0



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_SOURCE_WIDTH    1024
#define MAX_CLOCK_DIFF      3



/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    vbi_parse_manchester_code - parse a Manchester
    code from a line of video data
-------------------------------------------------*/

int vbi_parse_manchester_code(const uint16_t *source, int sourcewidth, int sourceshift, int expectedbits, uint32_t *result)
{
	uint8_t srcabs[MAX_SOURCE_WIDTH];
	uint8_t min, max, mid, srcabsval;
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
		uint8_t rawsrc = source[x] >> sourceshift;
		min = std::min(min, rawsrc);
		max = std::max(max, rawsrc);
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
		uint8_t rawsrc = source[x] >> sourceshift;
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
//      int leftmid = firstedge + ((double)x - 0.25) * bestclock;
		int leftend = firstedge + floor(((double)x - 0.0) * bestclock);
		int rightstart = firstedge + ceil(((double)x + 0.0) * bestclock);
//      int rightmid = firstedge + ((double)x + 0.25) * bestclock;
		int rightend = firstedge + floor(((double)x + 0.5) * bestclock);
		int leftavg, rightavg, leftabs, rightabs;
		int confidence = 0;
		int tx;

		/* compute left and right average values */
		leftavg = 0;
		for (tx = leftstart; tx <= leftend; tx++)
			leftavg += (uint8_t)(source[tx] >> sourceshift) - mid;
		leftabs = (leftavg >= 0);
		leftavg = (leftavg < 0) ? -leftavg : leftavg;

		rightavg = 0;
		for (tx = rightstart; tx <= rightend; tx++)
			rightavg += (uint8_t)(source[tx] >> sourceshift) - mid;
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

bool vbi_parse_white_flag(const uint16_t *source, int sourcewidth, int sourceshift)
{
	int histo[256] = { 0 };
	int minval = 0xff;
	int maxval = 0x00;
	int subtract;
	int peakval;
	int result;
	int x;

	/* compute a histogram of values */
	for (x = 0; x < sourcewidth; x++)
	{
		uint8_t yval = source[x] >> sourceshift;
		histo[yval]++;
	}

	/* remove the lowest 1% of the values to account for noise and determine the minimum */
	subtract = sourcewidth / 100;
	for (minval = 0; minval < 255; minval++)
		if ((subtract -= histo[minval]) < 0)
			break;

	/* remove the highest 1% of the values to account for noise and determine the maximum */
	subtract = sourcewidth / 100;
	for (maxval = 255; maxval > 0; maxval--)
		if ((subtract -= histo[maxval]) < 0)
			break;

	/* this is useful for debugging issues with white flag detection */
	if (PRINTF_WHITE_FLAG)
	{
		printf("Histo: min=%02X max=%02X\n", minval, maxval);
		for (x = 0; x < 256; x++)
			if (histo[x] != 0) printf("%dx%02X\n", histo[x], x);
	}

	/* ignore if we have no dynamic range */
	if (maxval - minval < 10)
	{
		if (PRINTF_WHITE_FLAG)
			printf("White flag NOT detected; threshold too low\n");
		return false;
	}

	/*
	    At this point, there are two approaches that have been tried:

	    1. Find the peak value and call it white if the peak is above
	       the 90% line

	    2. Ignore the first and last 20% of the line and count how
	       many pixels are above some threshold (75% line was used).
	       Call it white if at least 80% of the pixels are above
	       the threshold.

	    Both approaches agree 99% of the time, but the first tends to
	    be more correct when there is a discrepancy.
	*/

	/* determine where the peak is */
	peakval = 0;
	for (x = 1; x < 256; x++)
		if (histo[x] > histo[peakval])
			peakval = x;

	/* return true if it is above the 90% mark */
	result = (peakval > minval + 9 * (maxval - minval) / 10);
	if (PRINTF_WHITE_FLAG)
		printf("White flag %s: peak=%02X thresh=%02X\n", result ? "detected" : "NOT detected", peakval, minval + 9 * (maxval - minval) / 10);
	return result;

#ifdef UNUSED_CODE
{
	int above = 0;
	int thresh;

	/* alternate approach: */

	/* ignore the first 1/5 and last 1/5 of the line for the remaining computations */
	source += sourcewidth / 5;
	sourcewidth -= 2 * (sourcewidth / 5);

	/* count how many values were above the 75% mark of the range */
	thresh = minval + 3 * (maxval - minval) / 4;
	for (x = 0; x < sourcewidth; x++)
	{
		uint8_t yval = source[x] >> sourceshift;
		above += (yval >= thresh);
	}
	/* if at least 80% of the pixels are above the threshold, we'll call it white */
	return ((above * 100) / sourcewidth >= 80);
}
#endif
}


/*-------------------------------------------------
    vbi_parse_all - parse everything from a video
    frame
-------------------------------------------------*/

/**
 * @fn  void vbi_parse_all(const uint16_t *source, int sourcerowpixels, int sourcewidth, int sourceshift, vbi_metadata *vbi)
 *
 * @brief   Vbi parse all.
 *
 * @param   source          Source for the.
 * @param   sourcerowpixels The sourcerowpixels.
 * @param   sourcewidth     The sourcewidth.
 * @param   sourceshift     The sourceshift.
 * @param [in,out]  vbi     If non-null, the vbi.
 */

void vbi_parse_all(const uint16_t *source, int sourcerowpixels, int sourcewidth, int sourceshift, vbi_metadata *vbi)
{
	uint32_t bits[2][24];
	uint8_t bitnum;

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
		if ((vbi->line17 & VBI_MASK_CAV_PICTURE) == VBI_CODE_CAV_PICTURE && (vbi->line18 & VBI_MASK_CAV_PICTURE) == VBI_CODE_CAV_PICTURE)
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


/*-------------------------------------------------
    vbi_metadata_pack - pack the VBI data down
    into a smaller form for storage
-------------------------------------------------*/

/**
 * @fn  void vbi_metadata_pack(uint8_t *dest, uint32_t framenum, const vbi_metadata *vbi)
 *
 * @brief   Vbi metadata pack.
 *
 * @param [in,out]  dest    If non-null, destination for the.
 * @param   framenum        The framenum.
 * @param   vbi             The vbi.
 */

void vbi_metadata_pack(uint8_t *dest, uint32_t framenum, const vbi_metadata *vbi)
{
	dest[0] = framenum >> 16;
	dest[1] = framenum >> 8;
	dest[2] = framenum >> 0;
	dest[3] = vbi->white;
	dest[4] = vbi->line16 >> 16;
	dest[5] = vbi->line16 >> 8;
	dest[6] = vbi->line16 >> 0;
	dest[7] = vbi->line17 >> 16;
	dest[8] = vbi->line17 >> 8;
	dest[9] = vbi->line17 >> 0;
	dest[10] = vbi->line18 >> 16;
	dest[11] = vbi->line18 >> 8;
	dest[12] = vbi->line18 >> 0;
	dest[13] = vbi->line1718 >> 16;
	dest[14] = vbi->line1718 >> 8;
	dest[15] = vbi->line1718 >> 0;
}


/*-------------------------------------------------
    vbi_metadata_unpack - unpack the VBI data
    from a smaller form into the full structure
-------------------------------------------------*/

/**
 * @fn  void vbi_metadata_unpack(vbi_metadata *vbi, uint32_t *framenum, const uint8_t *source)
 *
 * @brief   Vbi metadata unpack.
 *
 * @param [in,out]  vbi         If non-null, the vbi.
 * @param [in,out]  framenum    If non-null, the framenum.
 * @param   source              Source for the.
 */

void vbi_metadata_unpack(vbi_metadata *vbi, uint32_t *framenum, const uint8_t *source)
{
	if (framenum != nullptr)
		*framenum = (source[0] << 16) | (source[1] << 8) | source[2];
	vbi->white = source[3];
	vbi->line16 = (source[4] << 16) | (source[5] << 8) | source[6];
	vbi->line17 = (source[7] << 16) | (source[8] << 8) | source[9];
	vbi->line18 = (source[10] << 16) | (source[11] << 8) | source[12];
	vbi->line1718 = (source[13] << 16) | (source[14] << 8) | source[15];
}
