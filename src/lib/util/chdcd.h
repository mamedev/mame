// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    CDRDAO TOC parser for CHD compression frontend

***************************************************************************/

#ifndef MAME_UTIL_CHDCD_H
#define MAME_UTIL_CHDCD_H

#pragma once

#include "cdrom.h"

struct chdcd_track_input_entry
{
	chdcd_track_input_entry() { reset(); }
	void reset() { fname.clear(); offset = idx0offs = idx1offs = 0; swap = false; }

	std::string fname;      // filename for each track
	uint32_t offset;      // offset in the data file for each track
	bool swap;          // data needs to be byte swapped
	uint32_t idx0offs;
	uint32_t idx1offs;
};

struct chdcd_track_input_info
{
	void reset() { for (auto & elem : track) elem.reset(); }

	chdcd_track_input_entry track[CD_MAX_TRACKS];
};


chd_error chdcd_parse_toc(const char *tocfname, cdrom_toc &outtoc, chdcd_track_input_info &outinfo);


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


#endif // MAME_UTIL_CHDCD_H
