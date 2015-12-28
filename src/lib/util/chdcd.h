// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    CDRDAO TOC parser for CHD compression frontend

***************************************************************************/

#pragma once

#ifndef __CHDCD_H__
#define __CHDCD_H__

#include "cdrom.h"

struct chdcd_track_input_entry
{
	chdcd_track_input_entry() { reset(); }
	void reset() { fname.clear(); offset = idx0offs = idx1offs = 0; swap = false; }

	std::string fname;      // filename for each track
	UINT32 offset;      // offset in the data file for each track
	bool swap;          // data needs to be byte swapped
	UINT32 idx0offs;
	UINT32 idx1offs;
};

struct chdcd_track_input_info
{
	void reset() { for (auto & elem : track) elem.reset(); }

	chdcd_track_input_entry track[CD_MAX_TRACKS];
};


chd_error chdcd_parse_toc(const char *tocfname, cdrom_toc &outtoc, chdcd_track_input_info &outinfo);

#endif  /* __CHDCD_H__ */
