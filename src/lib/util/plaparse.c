// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    plaparse.h

    Simple parser for Berkeley standard PLA files into raw fusemaps.
    It supports no more than one output matrix, and is limited to
    keywords: i, o, p, phase, e

***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "jedparse.h"
#include "plaparse.h"



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define LOG_PARSE       0



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct parse_info
{
	UINT32  inputs;     /* number of input columns */
	UINT32  outputs;    /* number of output columns */
	UINT32  terms;      /* number of terms */
	UINT32  xorval[JED_MAX_FUSES/64];   /* output polarity */
	UINT32  xorptr;
};



/***************************************************************************
    UTILITIES
***************************************************************************/

/*-------------------------------------------------
    iscrlf - is a line feed character
-------------------------------------------------*/

static int iscrlf(char c)
{
	return (c == 13 || c == 10);
}


/*-------------------------------------------------
    suck_number - read a decimal value from the
    character stream
-------------------------------------------------*/

static UINT32 suck_number(const UINT8 **cursrc, const UINT8 *srcend)
{
	UINT32 value = 0;
	(*cursrc)++;
	
	// find first digit
	while (*cursrc < srcend && !iscrlf(**cursrc) && !isdigit(**cursrc))
		(*cursrc)++;

	// loop over and accumulate digits
	while (isdigit(**cursrc))
	{
		value = value * 10 + (**cursrc) - '0';
		(*cursrc)++;
	}

	return value;
}



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    process_terms - process input/output matrix
-------------------------------------------------*/

static bool process_terms(jed_data *data, const UINT8 **cursrc, const UINT8 *srcend, parse_info *pinfo)
{
	UINT32 curinput = 0;
	UINT32 curoutput = 0;
	bool outputs = false;

	while (*cursrc < srcend && **cursrc != '.' && **cursrc != '#')
	{
		switch (**cursrc)
		{
			case '-':
				if (!outputs)
				{
					curinput++;
					jed_set_fuse(data, data->numfuses++, 1);
					jed_set_fuse(data, data->numfuses++, 1);

					if (LOG_PARSE) printf("11");
				}
				break;

			case '~':
				if (!outputs)
				{
					curinput++;
					// this product term is inhibited
					jed_set_fuse(data, data->numfuses++, 0);
					jed_set_fuse(data, data->numfuses++, 0);

					if (LOG_PARSE) printf("00");
				}
				break;

			case '1':
				if (outputs)
				{
					curoutput++;
					jed_set_fuse(data, data->numfuses++, 0);

					if (LOG_PARSE) printf("0");
				}
				else
				{
					curinput++;
					jed_set_fuse(data, data->numfuses++, 1);
					jed_set_fuse(data, data->numfuses++, 0);

					if (LOG_PARSE) printf("10");
				}
				break;

			case '0':
				if (outputs)
				{
					curoutput++;
					jed_set_fuse(data, data->numfuses++, 1);

					if (LOG_PARSE) printf("1");
				}
				else
				{
					curinput++;
					jed_set_fuse(data, data->numfuses++, 0);
					jed_set_fuse(data, data->numfuses++, 1);

					if (LOG_PARSE) printf("01");
				}
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

		if (iscrlf(**cursrc) && outputs)
		{
			outputs = false;
			if (LOG_PARSE) printf("\n");
			
			if (curinput != pinfo->inputs || curoutput != pinfo->outputs)
				return false;

			curinput = 0;
			curoutput = 0;
		}

		(*cursrc)++;
	}
	
	return true;
}



/*-------------------------------------------------
    process_field - process a single field
-------------------------------------------------*/

static bool process_field(jed_data *data, const UINT8 **cursrc, const UINT8 *srcend, parse_info *pinfo)
{
	(*cursrc)++;

	switch (**cursrc)
	{
		// number of inputs
		case 'i':
			pinfo->inputs = suck_number(cursrc, srcend);
			if (pinfo->inputs == 0 || pinfo->inputs >= (JED_MAX_FUSES/2))
				return false;

			if (LOG_PARSE) printf("Inputs: %u\n", pinfo->inputs);
			break;

		// number of outputs
		case 'o':
			pinfo->outputs = suck_number(cursrc, srcend);
			if (pinfo->outputs == 0 || pinfo->outputs >= (JED_MAX_FUSES/2))
				return false;

			if (LOG_PARSE) printf("Outputs: %u\n", pinfo->outputs);
			break;

		case 'p':
			// output polarity (optional)
			if ((*cursrc)[1] == 'h' && (*cursrc)[2] == 'a' && (*cursrc)[3] == 's' && (*cursrc)[4] == 'e')
			{
				if (LOG_PARSE) printf("Phase...\n");
				while (*cursrc < srcend && !iscrlf(**cursrc) && pinfo->xorptr < (JED_MAX_FUSES/2))
				{
					if (**cursrc == '0' || **cursrc == '1')
					{
						// 0 is negative
						if (**cursrc == '0')
							pinfo->xorval[pinfo->xorptr/32] |= 1 << (pinfo->xorptr & 31);
						pinfo->xorptr++;
					}
					
					(*cursrc)++;
				}
			}
			
			// or number of product terms (optional)
			else
			{
				pinfo->terms = suck_number(cursrc, srcend);
				if (pinfo->terms == 0 || pinfo->terms >= (JED_MAX_FUSES/2))
					return false;

				if (LOG_PARSE) printf("Terms: %u\n", pinfo->terms);
			}
			break;

		// end of file (optional)
		case 'e':
			if (LOG_PARSE) printf("End of file\n");
			break;
		
		default:
			return false;
	}
	
	return true;
}



/*-------------------------------------------------
    pla_parse - parse a Berkeley standard PLA file
    that has been loaded raw into memory
-------------------------------------------------*/

int pla_parse(const void *data, size_t length, jed_data *result)
{
	const UINT8 *cursrc = (const UINT8 *)data;
	const UINT8 *srcend = cursrc + length;
	
	parse_info pinfo;
	memset(&pinfo, 0, sizeof(pinfo));

	result->numfuses = 0;
	memset(result->fusemap, 0, sizeof(result->fusemap));

	while (cursrc < srcend)
	{
		switch (*cursrc)
		{
			// comment line
			case '#':
				while (cursrc < srcend && !iscrlf(*cursrc))
					cursrc++;
				break;

			// keyword
			case '.':
				if (!process_field(result, &cursrc, srcend, &pinfo))
					return JEDERR_INVALID_DATA;
				break;
			
			// terms
			case '0': case '1': case '-': case '~':
				if (!process_terms(result, &cursrc, srcend, &pinfo))
					return JEDERR_INVALID_DATA;
				break;
			
			default:
				cursrc++;
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
