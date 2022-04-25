// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    jedparse.c

    Parser for .JED files into raw fusemaps.

****************************************************************************

    Binary file format:

    Offset
        0 = Total number of fuses (32 bits)
        4 = Raw fuse data, packed 8 bits at a time, LSB to MSB

***************************************************************************/

#include "jedparse.h"

#include "ioprocs.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define LOG_PARSE       0



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct jed_parse_info
{
	uint16_t      checksum;               /* checksum value */
	uint32_t      explicit_numfuses;      /* explicitly specified number of fuses */
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

static uint32_t suck_number(const uint8_t **psrc)
{
	const uint8_t *src = *psrc;
	uint32_t value = 0;

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

static void process_field(jed_data *data, const uint8_t *cursrc, const uint8_t *srcend, jed_parse_info *pinfo)
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
			uint32_t curfuse;

			/* read the fuse number */
			cursrc++;
			curfuse = suck_number(&cursrc);
			if (LOG_PARSE) printf("L%u\n", curfuse);

			/* read digits, skipping delimiters */
			for ( ; cursrc < srcend; cursrc++)
				if (*cursrc == '0' || *cursrc == '1')
				{
					jed_set_fuse(data, curfuse, *cursrc - '0');
					if (LOG_PARSE) printf("  fuse %u = %d\n", curfuse, 0);
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

int jed_parse(util::random_read &src, jed_data *result)
{
	jed_parse_info pinfo;
	int i;
	std::size_t actual;

	/* initialize the output and the intermediate info struct */
	memset(result, 0, sizeof(*result));
	memset(&pinfo, 0, sizeof(pinfo));

	/* first scan for the STX character; ignore anything prior */
	uint8_t ch;
	do
	{
		if (src.read(&ch, 1, actual) || actual != 1)
			return JEDERR_INVALID_DATA;
	}
	while (ch != 0x02);

	/* then scan to see if we have an ETX */
	uint64_t startpos = 0;
	uint16_t checksum = ch;
	do
	{
		if (src.read(&ch, 1, actual) || actual != 1)
			return JEDERR_INVALID_DATA;
		checksum += ch & 0x7f;

		/* mark end of comment field */
		if (ch == '*' && startpos == 0)
		{
			if (src.tell(startpos))
				return JEDERR_INVALID_DATA;
		}
	}
	while (ch != 0x03);

	/* the ETX becomes the real srcend */
	uint64_t endpos;
	if (src.tell(endpos))
		return JEDERR_INVALID_DATA;
	endpos--;

	/* see if there is a transmission checksum at the end */
	uint8_t sumbuf[4];
	if (!src.read(&sumbuf[0], 4, actual) && actual == 4 && ishex(sumbuf[0]) && ishex(sumbuf[1]) && ishex(sumbuf[2]) && ishex(sumbuf[3]))
	{
		uint16_t dessum = (hexval(sumbuf[0]) << 12) | (hexval(sumbuf[1]) << 8) | (hexval(sumbuf[2]) << 4) | hexval(sumbuf[3] << 0);
		if (dessum != 0 && dessum != checksum)
			return JEDERR_BAD_XMIT_SUM;
	}

	/* blast through the comment field */
	if (startpos == 0 || src.seek(startpos, SEEK_SET))
		return JEDERR_INVALID_DATA;
	auto srcdata = std::make_unique<uint8_t[]>(endpos - startpos);
	if (src.read(&srcdata[0], endpos - startpos, actual) || actual != endpos - startpos)
		return JEDERR_INVALID_DATA;
	const uint8_t *cursrc = &srcdata[0];
	const uint8_t *const srcend = &srcdata[endpos - startpos];

	/* now loop over fields and decide which ones go in the file output */
	while (cursrc < srcend)
	{
		/* skip over delimiters */
		while (cursrc < srcend && isdelim(*cursrc))
			cursrc++;
		if (cursrc >= srcend)
			break;

		/* end of field is an asterisk -- find it */
		const uint8_t *scan = cursrc;
		while (scan < srcend && *scan != '*')
			scan++;
		if (scan >= srcend)
			return JEDERR_INVALID_DATA;

		/* process the field */
		process_field(result, cursrc, scan, &pinfo);

		/* advance past it */
		cursrc = scan + 1;
	}

	srcdata.reset();

	/* if we got an explicit fuse count, override our computed count */
	if (pinfo.explicit_numfuses != 0)
		result->numfuses = pinfo.explicit_numfuses;

	/* clear out leftover bits g*/
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
	auto *curdst = (uint8_t *)result;
	uint8_t *dstend = curdst + length;
	int i, zeros, ones;
	char tempbuf[256];
	uint16_t checksum;
	uint8_t defbyte;
	uint8_t *temp;

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
	for (temp = (uint8_t *)result; temp < curdst && temp < dstend; temp++)
		checksum += *temp & 0x7f;
	checksum += 0x03;

	/* append the ETX and the transmission checksum */
	tempbuf[0] = 0x03;
	sprintf(&tempbuf[1], "%04X", checksum);
	if (curdst + strlen(tempbuf) <= dstend)
		memcpy(curdst, tempbuf, strlen(tempbuf));
	curdst += strlen(tempbuf);

	/* return the final size */
	return curdst - (uint8_t *)result;
}



/*-------------------------------------------------
    jedbin_parse - parse a binary JED file that
    has been loaded raw into memory
-------------------------------------------------*/

int jedbin_parse(util::read_stream &src, jed_data *result)
{
	/* initialize the output */
	memset(result, 0, sizeof(*result));

	/* need at least 4 bytes */
	uint8_t buf[4];
	std::size_t actual;
	if (src.read(&buf[0], 4, actual) || actual != 4)
		return JEDERR_INVALID_DATA;

	/* first unpack the number of fuses */
	result->numfuses = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
	if (result->numfuses == 0 || result->numfuses > JED_MAX_FUSES)
		return JEDERR_INVALID_DATA;

	/* now make sure we have enough data in the source */
	/* copy in the data */
	if (src.read(result->fusemap, (result->numfuses + 7) / 8, actual) || actual != (result->numfuses + 7) / 8)
		return JEDERR_INVALID_DATA;
	return JEDERR_NONE;
}



/*-------------------------------------------------
    jedbin_output - generate a new binary JED file
    based on the jed_data provided
-------------------------------------------------*/

/**
 * @fn  size_t jedbin_output(const jed_data *data, void *result, size_t length)
 *
 * @brief   Jedbin output.
 *
 * @param   data        The data.
 * @param [out] result  If non-null, the result.
 * @param   length      The length.
 *
 * @return  A size_t.
 */

size_t jedbin_output(const jed_data *data, void *result, size_t length)
{
	auto *curdst = (uint8_t *)result;

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
