// license:BSD-3-Clause
/***************************************************************************

    gdrom.h

    Definitions used in GDROM routines

***************************************************************************/

#ifndef MAME_UTIL_GDROM_H
#define MAME_UTIL_GDROM_H

#pragma once

/***************************************************************************
    CONSTANTS
***************************************************************************/

enum gdrom_area
{
	GDROM_SINGLE_DENSITY = 0,
	GDROM_HIGH_DENSITY
};

enum gdrom_pattern
{
	GDROM_TYPE_UNKNOWN = 0,
	GDROM_TYPE_I,
	GDROM_TYPE_II,
	GDROM_TYPE_III,
	GDROM_TYPE_III_SPLIT
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* GDROM utilities */
enum gdrom_pattern gdrom_identify_pattern(const cdrom_toc *toc);

#endif // MAME_UTIL_GDROM_H
