// license:BSD-3-Clause
// copyright-holders:Aaron Giles, hap
/***************************************************************************

    plaparse.h

    Parser for Berkeley standard PLA files into raw fusemaps.

***************************************************************************/

#ifndef __PLAPARSE_H__
#define __PLAPARSE_H__

#include "osdcore.h"



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* parse a file (read into memory) into a jed_data structure */
int pla_parse(const void *data, size_t length, jed_data *result);



#endif  /* __PLAPARSE_H__ */
