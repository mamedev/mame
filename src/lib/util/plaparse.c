// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    plaparse.h

    Parser for Berkeley standard PLA files into raw fusemaps.

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
	UINT32  inputs;
	UINT32  outputs;
	UINT32  terms;
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

static UINT32 suck_number(const UINT8 **psrc)
{
	const UINT8 *src = *psrc;
	UINT32 value = 0;

	// loop over and accumulate digits
	while (isdigit(*src))
	{
		value = value * 10 + *src - '0';
		src++;
	}

	// return a pointer to the string afterwards
	*psrc = src;
	return value;
}



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    process_field - process a single field
-------------------------------------------------*/

static void process_field(jed_data *data, const UINT8 *cursrc, const UINT8 *srcend, parse_info *pinfo)
{
	cursrc++;

	// switch off of the field type
	switch (*cursrc)
	{
		// number of inputs
		case 'i':
			cursrc += 2;
			pinfo->inputs = suck_number(&cursrc);
			if (LOG_PARSE) printf("Inputs: %u\n", pinfo->inputs);
			break;

		// number of outputs
		case 'o':
			cursrc += 2;
			pinfo->outputs = suck_number(&cursrc);
			if (LOG_PARSE) printf("Outputs: %u\n", pinfo->outputs);
			break;

		// number of product terms
		case 'p':
		{
			cursrc += 2;
			pinfo->terms = suck_number(&cursrc);
			if (LOG_PARSE) printf("Terms: %u\n", pinfo->terms);

			UINT32 curfuse = 0;
			bool outputs = false;

			cursrc++;
			while (cursrc < srcend && *cursrc != '.')
			{
				switch (*cursrc)
				{
				case '-':
					if (!outputs)
					{
						jed_set_fuse(data, curfuse++, 1);
						jed_set_fuse(data, curfuse++, 1);

						if (LOG_PARSE) printf("11");
					}
					break;

				case '1':
					if (outputs)
					{
						jed_set_fuse(data, curfuse++, 0);

						if (LOG_PARSE) printf("0");
					}
					else
					{
						jed_set_fuse(data, curfuse++, 1);
						jed_set_fuse(data, curfuse++, 0);

						if (LOG_PARSE) printf("10");
					}
					break;

				case '0':
					if (outputs)
					{
						jed_set_fuse(data, curfuse++, 1);

						if (LOG_PARSE) printf("1");
					}
					else
					{
						jed_set_fuse(data, curfuse++, 0);
						jed_set_fuse(data, curfuse++, 1);

						if (LOG_PARSE) printf("01");
					}
					break;

				case ' ':
					outputs = true;
					if (LOG_PARSE) printf(" ");
					break;
				}

				if (iscrlf(*cursrc) && outputs)
				{
					outputs = false;
					if (LOG_PARSE) printf("\n");
				}

				cursrc++;
			}

			data->numfuses = curfuse;
			break;
		}

		// end of file
		case 'e':
			printf("End of file\n");
			break;
	}

	cursrc++;
}



/*-------------------------------------------------
    pla_parse - parse a Berkeley standard PLA file
    that has been loaded raw into memory
-------------------------------------------------*/

int pla_parse(const void *data, size_t length, jed_data *result)
{
	const UINT8 *cursrc = (const UINT8 *)data;
	const UINT8 *srcend = cursrc + length;
	const UINT8 *scan;
	parse_info pinfo;

	result->numfuses = 0;
	memset(result->fusemap, 0x00, sizeof(result->fusemap));

	while (cursrc < srcend)
	{
		if (*cursrc == '#')
		{
			cursrc++;
			while (cursrc < srcend && !iscrlf(*cursrc))
				cursrc++;
		}
		else if (*cursrc == '.')
		{
			scan = cursrc;
			while (scan < srcend && !iscrlf(*scan))
				scan++;
			if (scan >= srcend)
				return JEDERR_INVALID_DATA;

			process_field(result, cursrc, srcend, &pinfo);

			cursrc = scan + 1;
		}

		cursrc++;
	}

	return JEDERR_NONE;
}
