/***************************************************************************

    jedparse.c

    Parser for .JED files into raw fusemaps.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    Binary file format:

    Offset
        0 = Total number of fuses (32 bits)
        4 = Raw fuse data, packed 8 bits at a time, LSB to MSB

***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "jedparse.h"



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define LOG_PARSE		0



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _parse_info parse_info;
struct _parse_info
{
	UINT16		checksum;				/* checksum value */
	UINT32		explicit_numfuses;		/* explicitly specified number of fuses */
};



/***************************************************************************
    UTILITIES
***************************************************************************/

/*-------------------------------------------------
    ishex - is a character a valid hex digit?
-------------------------------------------------*/

static int ishex(char c)
{
	return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F');
}



/*-------------------------------------------------
    hexval - the hex value of a given character
-------------------------------------------------*/

static int hexval(char c)
{
	return (c >= '0' && c <= '9') ? (c - '0') : (10 + c - 'A');
}



/*-------------------------------------------------
    isdelim - is a character a JEDEC delimiter?
-------------------------------------------------*/

static int isdelim(char c)
{
	return (c == ' ' || c == 13 || c == 10);
}



/*-------------------------------------------------
    suck_number - read a decimal value from the
    character stream
-------------------------------------------------*/

static UINT32 suck_number(const UINT8 **psrc)
{
	const UINT8 *src = *psrc;
	UINT32 value = 0;

	/* skip delimiters */
	while (isdelim(*src))
		src++;

	/* loop over and accumulate digits */
	while (isdigit(*src))
	{
		value = value * 10 + *src - '0';
		src++;
	}

	/* return a pointer to the string afterwards */
	*psrc = src;
	return value;
}



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    process_field - process a single JEDEC field
-------------------------------------------------*/

static void process_field(jed_data *data, const UINT8 *cursrc, const UINT8 *srcend, parse_info *pinfo)
{
	/* switch off of the field type */
	switch (*cursrc)
	{
		case 'Q':
			cursrc++;
			switch (*cursrc)
			{
				/* number of fuses */
				case 'F':
					cursrc++;
					pinfo->explicit_numfuses = data->numfuses = suck_number(&cursrc);
					break;
			}
			break;

		/* default fuse state (0 or 1) */
		case 'F':
			cursrc++;
			if (LOG_PARSE) printf("F%c\n", *cursrc);
			if (*cursrc == '0')
				memset(data->fusemap, 0x00, sizeof(data->fusemap));
			else
				memset(data->fusemap, 0xff, sizeof(data->fusemap));
			break;

		/* fuse states */
		case 'L':
		{
			UINT32 curfuse;

			/* read the fuse number */
			cursrc++;
			curfuse = suck_number(&cursrc);
			if (LOG_PARSE) printf("L%d\n", curfuse);

			/* read digits, skipping delimiters */
			for ( ; cursrc < srcend; cursrc++)
				if (*cursrc == '0' || *cursrc == '1')
				{
					jed_set_fuse(data, curfuse, *cursrc - '0');
					if (LOG_PARSE) printf("  fuse %d = %d\n", curfuse, 0);
					if (curfuse >= data->numfuses)
						data->numfuses = curfuse + 1;
					curfuse++;
				}
			break;
		}

		/* fuse checksum */
		case 'C':
			cursrc++;
			if (cursrc < srcend + 4 && ishex(cursrc[0]) && ishex(cursrc[1]) && ishex(cursrc[2]) && ishex(cursrc[3]))
			{
				pinfo->checksum = 0;
				while (ishex(*cursrc) && cursrc < srcend)
					pinfo->checksum = (pinfo->checksum << 4) | hexval(*cursrc++);
			}
			break;
	}
}



/*-------------------------------------------------
    jed_parse - parse a .JED file that has been
    loaded raw into memory
-------------------------------------------------*/

int jed_parse(const void *data, size_t length, jed_data *result)
{
	const UINT8 *cursrc = data;
	const UINT8 *srcend = cursrc + length;
	const UINT8 *scan;
	parse_info pinfo;
	UINT16 checksum;
	int i;

	/* initialize the output and the intermediate info struct */
	memset(result, 0, sizeof(*result));
	memset(&pinfo, 0, sizeof(pinfo));

	/* first scan for the STX character; ignore anything prior */
	while (cursrc < srcend && *cursrc != 0x02)
		cursrc++;
	if (cursrc >= srcend)
		return JEDERR_INVALID_DATA;

	/* then scan to see if we have an ETX */
	scan = cursrc;
	checksum = 0;
	while (scan < srcend && *scan != 0x03)
		checksum += *scan++ & 0x7f;
	if (scan >= srcend)
		return JEDERR_INVALID_DATA;

	/* see if there is a transmission checksum at the end */
	checksum += *scan;
	if (scan + 4 < srcend && ishex(scan[1]) && ishex(scan[2]) && ishex(scan[3]) && ishex(scan[4]))
	{
		UINT16 dessum = (hexval(scan[1]) << 12) | (hexval(scan[2]) << 8) | (hexval(scan[3]) << 4) | hexval(scan[4] << 0);
		if (dessum != 0 && dessum != checksum)
			return JEDERR_BAD_XMIT_SUM;
	}

	/* the ETX becomes the real srcend */
	srcend = scan;

	/* blast through the comment field */
	cursrc++;
	while (cursrc < srcend && *cursrc != '*')
		cursrc++;

	/* now loop over fields and decide which ones go in the file output */
	cursrc++;
	while (cursrc < srcend)
	{
		/* skip over delimiters */
		while (cursrc < srcend && isdelim(*cursrc))
			cursrc++;
		if (cursrc >= srcend)
			break;

		/* end of field is an asterisk -- find it */
		scan = cursrc;
		while (scan < srcend && *scan != '*')
			scan++;
		if (scan >= srcend)
			return JEDERR_INVALID_DATA;

		/* process the field */
		process_field(result, cursrc, scan, &pinfo);

		/* advance past it */
		cursrc = scan + 1;
	}

	/* if we got an explicit fuse count, override our computed count */
	if (pinfo.explicit_numfuses != 0)
		result->numfuses = pinfo.explicit_numfuses;

	/* clear out leftover bits */
	if (result->numfuses % 8 != 0)
		result->fusemap[result->numfuses / 8] &= (1 << (result->numfuses % 8)) - 1;
	memset(&result->fusemap[(result->numfuses + 7) / 8], 0, sizeof(result->fusemap) - (result->numfuses + 7) / 8);

	/* validate the checksum */
	checksum = 0;
	for (i = 0; i < (result->numfuses + 7) / 8; i++)
		checksum += result->fusemap[i];
	if (pinfo.checksum != 0 && checksum != pinfo.checksum)
		return JEDERR_BAD_FUSE_SUM;

	return JEDERR_NONE;
}



/*-------------------------------------------------
    jed_output - generate a new .JED file based
    on the jed_data provided
-------------------------------------------------*/

size_t jed_output(const jed_data *data, void *result, size_t length)
{
	UINT8 *curdst = result;
	UINT8 *dstend = curdst + length;
	int i, zeros, ones;
	char tempbuf[256];
	UINT16 checksum;
	UINT8 defbyte;
	UINT8 *temp;

	/* always start the DST with a standard header and an STX */
	tempbuf[0] = 0x02;
	sprintf(&tempbuf[1], "JEDEC file generated by jedutil*\n");
	if (curdst + strlen(tempbuf) <= dstend)
		memcpy(curdst, tempbuf, strlen(tempbuf));
	curdst += strlen(tempbuf);

	/* append the package information */
	sprintf(tempbuf, "QF%d*\n", data->numfuses);
	if (curdst + strlen(tempbuf) <= dstend)
		memcpy(curdst, tempbuf, strlen(tempbuf));
	curdst += strlen(tempbuf);

	/* compute the checksum */
	checksum = 0;
	for (i = 0; i < data->numfuses / 8; i++)
		checksum += data->fusemap[i];
	if (data->numfuses % 8 != 0)
		checksum += data->fusemap[data->numfuses / 8] & ((1 << (data->numfuses % 8)) - 1);

	/* determine if we are mostly 0's or mostly 1's */
	for (i = zeros = ones = 0; i < data->numfuses / 8; i++)
		if (data->fusemap[i] == 0x00)
			zeros++;
		else if (data->fusemap[i] == 0xff)
			ones++;
	defbyte = (ones > zeros) ? 0xff : 0x00;

	/* output the default fuse state */
	sprintf(tempbuf, "F%d*\n", defbyte & 1);
	if (curdst + strlen(tempbuf) <= dstend)
		memcpy(curdst, tempbuf, strlen(tempbuf));
	curdst += strlen(tempbuf);

	/* now loop over groups of 32 fuses and output non-default groups */
	for (i = 0; i < data->numfuses; i += 32)
		if (data->fusemap[i / 8 + 0] != defbyte ||
			data->fusemap[i / 8 + 1] != defbyte ||
			data->fusemap[i / 8 + 2] != defbyte ||
			data->fusemap[i / 8 + 3] != defbyte)
		{
			int stroffs;
			int j;

			/* build up a string of 32 fuses */
			stroffs = sprintf(tempbuf, "L%05d ", i);
			for (j = 0; j < 32 && i+j < data->numfuses; j++)
				tempbuf[stroffs++] = '0' + jed_get_fuse(data, i + j);
			stroffs += sprintf(&tempbuf[stroffs], "*\n");

			/* append to the buffer */
			if (curdst + strlen(tempbuf) <= dstend)
				memcpy(curdst, tempbuf, strlen(tempbuf));
			curdst += strlen(tempbuf);
		}

	/* write the checksum */
	sprintf(tempbuf, "C%04X*\n", checksum);
	if (curdst + strlen(tempbuf) <= dstend)
		memcpy(curdst, tempbuf, strlen(tempbuf));
	curdst += strlen(tempbuf);

	/* now compute the transmission checksum */
	checksum = 0;
	for (temp = result; temp < curdst && temp < dstend; temp++)
		checksum += *temp & 0x7f;
	checksum += 0x03;

	/* append the ETX and the transmission checksum */
	tempbuf[0] = 0x03;
	sprintf(&tempbuf[1], "%04X", checksum);
	if (curdst + strlen(tempbuf) <= dstend)
		memcpy(curdst, tempbuf, strlen(tempbuf));
	curdst += strlen(tempbuf);

	/* return the final size */
	return curdst - (UINT8 *)result;
}



/*-------------------------------------------------
    jedbin_parse - parse a binary JED file that
    has been loaded raw into memory
-------------------------------------------------*/

int jedbin_parse(const void *data, size_t length, jed_data *result)
{
	const UINT8 *cursrc = data;

	/* initialize the output */
	memset(result, 0, sizeof(*result));

	/* need at least 4 bytes */
	if (length < 4)
		return JEDERR_INVALID_DATA;

	/* first unpack the number of fuses */
	result->numfuses = (cursrc[0] << 24) | (cursrc[1] << 16) | (cursrc[2] << 8) | cursrc[3];
	cursrc += 4;
	if (result->numfuses == 0 || result->numfuses > JED_MAX_FUSES)
		return JEDERR_INVALID_DATA;

	/* now make sure we have enough data in the source */
	if (length < 4 + (result->numfuses + 7) / 8)
		return JEDERR_INVALID_DATA;

	/* copy in the data */
	memcpy(result->fusemap, cursrc, (result->numfuses + 7) / 8);
	return JEDERR_NONE;
}



/*-------------------------------------------------
    jedbin_output - generate a new binary JED file
    based on the jed_data provided
-------------------------------------------------*/

size_t jedbin_output(const jed_data *data, void *result, size_t length)
{
	UINT8 *curdst = result;

	/* ensure we have enough room */
	if (length >= 4 + (data->numfuses + 7) / 8)
	{
		/* store the number of fuses */
		*curdst++ = data->numfuses >> 24;
		*curdst++ = data->numfuses >> 16;
		*curdst++ = data->numfuses >> 8;
		*curdst++ = data->numfuses >> 0;

		/* copy in the rest of the data */
		memcpy(curdst, data->fusemap, (data->numfuses + 7) / 8);
	}

	/* return the final size */
	return 4 + (data->numfuses + 7) / 8;
}

