// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    jedparse.h

    Parser for .JED files into raw fusemaps.

***************************************************************************/

#ifndef MAME_UTIL_JEDPARSE_H
#define MAME_UTIL_JEDPARSE_H

#pragma once

#include <cstddef>
#include <cstdint>



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define JED_MAX_FUSES           (65536)

/* error codes */
#define JEDERR_NONE             0
#define JEDERR_INVALID_DATA     1
#define JEDERR_BAD_XMIT_SUM     2
#define JEDERR_BAD_FUSE_SUM     3



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct jed_data
{
	uint32_t      numfuses;           /* number of defined fuses */
	uint8_t       fusemap[JED_MAX_FUSES / 8];/* array of bit-packed data */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* parse a file (read into memory) into a jed_data structure */
int jed_parse(const void *data, size_t length, jed_data *result);

/* output a jed_data structure into a well-formatted JED file */
size_t jed_output(const jed_data *data, void *result, size_t length);

/* parse a binary JED file (read into memory) into a jed_data structure */
int jedbin_parse(const void *data, size_t length, jed_data *result);

/* output a jed_data structure into a binary JED file */
size_t jedbin_output(const jed_data *data, void *result, size_t length);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

static inline int jed_get_fuse(const jed_data *data, uint32_t fusenum)
{
	if (fusenum < JED_MAX_FUSES)
		return (data->fusemap[fusenum / 8] >> (fusenum % 8)) & 1;
	else
		return 0;
}


static inline void jed_set_fuse(jed_data *data, uint32_t fusenum, uint8_t value)
{
	if (fusenum < JED_MAX_FUSES)
	{
		/* set or clear the bit as appropriate */
		if (value)
			data->fusemap[fusenum / 8] |= (1 << (fusenum % 8));
		else
			data->fusemap[fusenum / 8] &= ~(1 << (fusenum % 8));
	}
}

#endif // MAME_UTIL_JEDPARSE_H
