// license:BSD-3-Clause
// copyright-holders:Aaron Giles, hap
/***************************************************************************

    plaparse.cpp

    Simple parser for Berkeley standard PLA files into raw fusemaps.
    It supports no more than one output matrix, and is limited to
    keywords: i, o, p, phase, e

***************************************************************************/

#include "plaparse.h"

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

struct parse_info
{
	uint32_t  inputs;     /* number of input columns */
	uint32_t  outputs;    /* number of output columns */
	uint32_t  terms;      /* number of terms */
	uint32_t  xorval[JED_MAX_FUSES/64];   /* output polarity */
	uint32_t  xorptr;
};



/***************************************************************************
    UTILITIES
***************************************************************************/

/*-------------------------------------------------
    iscrlf - is a line feed character
-------------------------------------------------*/

static bool iscrlf(char c)
{
	return (c == 13 || c == 10);
}


/*-------------------------------------------------
    suck_number - read a decimal value from the
    character stream
-------------------------------------------------*/

static uint32_t suck_number(util::random_read &src)
{
	uint32_t value = 0;

	// find first digit
	uint8_t ch;
	std::size_t actual;
	bool found = false;
	while (!src.read(&ch, 1, actual) && actual == 1)
	{
		// loop over and accumulate digits
		if (isdigit(ch))
		{
			found = true;
			value = value * 10 + ch - '0';
		}
		else if (found || iscrlf(ch))
		{
			src.seek(-1, SEEK_CUR);
			break;
		}
	}

	return value;
}



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    process_terms - process input/output matrix
-------------------------------------------------*/

static bool process_terms(jed_data *data, util::random_read &src, uint8_t ch, parse_info *pinfo)
{
	uint32_t curinput = 0;
	uint32_t curoutput = 0;
	bool outputs = false;

	// symbols for 0, 1, dont_care, no_meaning
	// PLA format documentation also describes them as simply 0, 1, 2, 3
	static const char symbols[] = { "01-~" };

	while (ch != '.' && ch != '#')
	{
		if (!outputs)
		{
			// and-matrix
			if (strrchr(symbols, ch))
				curinput++;

			switch (ch)
			{
			case '0':
				jed_set_fuse(data, data->numfuses++, 0);
				jed_set_fuse(data, data->numfuses++, 1);

				if (LOG_PARSE) printf("01");
				break;

			case '1':
				jed_set_fuse(data, data->numfuses++, 1);
				jed_set_fuse(data, data->numfuses++, 0);

				if (LOG_PARSE) printf("10");
				break;

			// anything goes
			case '-':
				jed_set_fuse(data, data->numfuses++, 1);
				jed_set_fuse(data, data->numfuses++, 1);

				if (LOG_PARSE) printf("11");
				break;

			// this product term is inhibited
			case '~':
				jed_set_fuse(data, data->numfuses++, 0);
				jed_set_fuse(data, data->numfuses++, 0);

				if (LOG_PARSE) printf("00");
				break;

			case ' ': case '\t':
				if (curinput > 0)
				{
					outputs = true;
					if (LOG_PARSE) printf(" ");
				}
				break;

			default:
				break;
			}
		}
		else
		{
			// or-matrix
			if (strrchr(symbols, ch))
			{
				curoutput++;
				if (ch == '1')
				{
					jed_set_fuse(data, data->numfuses++, 0);
					if (LOG_PARSE) printf("0");
				}
				else
				{
					// write 1 for anything else
					jed_set_fuse(data, data->numfuses++, 1);
					if (LOG_PARSE) printf("1");
				}
			}
		}

		if (iscrlf(ch) && outputs)
		{
			outputs = false;
			if (LOG_PARSE) printf("\n");

			if (curinput != pinfo->inputs || curoutput != pinfo->outputs)
				return false;

			curinput = 0;
			curoutput = 0;
		}

		std::size_t actual;
		if (src.read(&ch, 1, actual))
			return false;
		if (actual != 1)
			return true;
	}

	src.seek(-1, SEEK_CUR);
	return true;
}



/*-------------------------------------------------
    process_field - process a single field
-------------------------------------------------*/

static bool process_field(jed_data *data, util::random_read &src, parse_info *pinfo)
{
	// valid keywords
	static const char *const keywords[] = { "i", "o", "p", "phase", "e", "\0" };
	enum
	{
		KW_INPUTS = 0,
		KW_OUTPUTS,
		KW_TERMS,
		KW_PHASE,
		KW_END,

		KW_INVALID
	};

	// find keyword
	char dest[0x10];
	memset(dest, 0, sizeof(dest));
	int destptr = 0;

	uint8_t seek;
	std::size_t actual;
	while (!src.read(&seek, 1, actual) && actual == 1 && isalpha(seek))
	{
		dest[destptr++] = tolower(seek);
		if (destptr == sizeof(dest))
			break;
	}

	uint8_t find = 0;
	while (strlen(keywords[find]) && strcmp(dest, keywords[find]))
		find++;

	if (find == KW_INVALID)
		return false;

	// handle it
	switch (find)
	{
	// number of inputs
	case KW_INPUTS:
		pinfo->inputs = suck_number(src);
		if (pinfo->inputs == 0 || pinfo->inputs >= (JED_MAX_FUSES/2))
			return false;

		if (LOG_PARSE) printf("Inputs: %u\n", pinfo->inputs);
		break;

	// number of outputs
	case KW_OUTPUTS:
		pinfo->outputs = suck_number(src);
		if (pinfo->outputs == 0 || pinfo->outputs >= (JED_MAX_FUSES/2))
			return false;

		if (LOG_PARSE) printf("Outputs: %u\n", pinfo->outputs);
		break;

	// number of product terms (optional)
	case KW_TERMS:
		pinfo->terms = suck_number(src);
		if (pinfo->terms == 0 || pinfo->terms >= (JED_MAX_FUSES/2))
			return false;

		if (LOG_PARSE) printf("Terms: %u\n", pinfo->terms);
		break;

	// output polarity (optional)
	case KW_PHASE:
		if (LOG_PARSE) printf("Phase...\n");
		while (!src.read(&seek, 1, actual) && actual == 1 && !iscrlf(seek) && pinfo->xorptr < (JED_MAX_FUSES/2))
		{
			if (seek == '0' || seek == '1')
			{
				// 0 is negative
				if (seek == '0')
					pinfo->xorval[pinfo->xorptr/32] |= 1 << (pinfo->xorptr & 31);
				pinfo->xorptr++;
			}
		}
		break;

	// end of file (optional)
	case KW_END:
		if (LOG_PARSE) printf("End of file\n");
		break;
	}

	return true;
}



/*-------------------------------------------------
    pla_parse - parse a Berkeley standard PLA file
    that has been loaded raw into memory
-------------------------------------------------*/

/**
 * @fn  int pla_parse(util::random_read &src, jed_data *result)
 *
 * @brief   Pla parse.
 *
 * @param   src         The source file.
 * @param [out] result  If non-null, the result.
 *
 * @return  An int.
 */

int pla_parse(util::random_read &src, jed_data *result)
{
	parse_info pinfo;
	memset(&pinfo, 0, sizeof(pinfo));

	result->numfuses = 0;
	memset(result->fusemap, 0, sizeof(result->fusemap));

	uint8_t ch;
	std::size_t actual;
	while (!src.read(&ch, 1, actual) && actual == 1)
	{
		switch (ch)
		{
		// comment line
		case '#':
			while (!src.read(&ch, 1, actual) && actual == 1)
			{
				if (iscrlf(ch))
					break;
			}
			break;

		// keyword
		case '.':
			if (!process_field(result, src, &pinfo))
				return JEDERR_INVALID_DATA;
			break;

		// terms
		case '0': case '1': case '-': case '~':
			if (!process_terms(result, src, ch, &pinfo))
				return JEDERR_INVALID_DATA;
			break;

		default:
			break;
		}
	}

	// write output polarity
	if (pinfo.xorptr > 0)
	{
		if (LOG_PARSE) printf("Polarity: ");

		for (int i = 0; i < pinfo.outputs; i++)
		{
			int bit = pinfo.xorval[i/32] >> (i & 31) & 1;
			jed_set_fuse(result, result->numfuses++, bit);
			if (LOG_PARSE) printf("%d", bit);
		}
		if (LOG_PARSE) printf("\n");
	}

	return JEDERR_NONE;
}
