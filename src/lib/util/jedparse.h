// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    jedparse.h

    Parser for .JED files into raw fusemaps.

***************************************************************************/

#ifndef __JEDPARSE_H__
#define __JEDPARSE_H__

#include "osdcore.h"



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
	UINT32      numfuses;           /* number of defined fuses */
	UINT8       fusemap[JED_MAX_FUSES / 8];/* array of bit-packed data */
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

static inline int jed_get_fuse(const jed_data *data, UINT32 fusenum)
{
	if (fusenum < JED_MAX_FUSES)
		return (data->fusemap[fusenum / 8] >> (fusenum % 8)) & 1;
	else
		return 0;
}


static inline void jed_set_fuse(jed_data *data, UINT32 fusenum, UINT8 value)
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

#endif  /* __JEDPARSE_H__ */
