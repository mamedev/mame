// license:BSD-3-Clause
// copyright-holders:Aaron Giles, hap
/***************************************************************************

    plaparse.h

    Parser for Berkeley standard PLA files into raw fusemaps.

***************************************************************************/

#ifndef MAME_UTIL_PLAPARSE_H
#define MAME_UTIL_PLAPARSE_H

#pragma once

#include "jedparse.h"


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* parse a file (read into memory) into a jed_data structure */
int pla_parse(const void *data, size_t length, jed_data *result);

#endif // MAME_UTIL_PLAPARSE_H
