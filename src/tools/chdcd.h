/***************************************************************************

    CDRDAO TOC parser for CHD compression frontend

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __CHDCD_H__
#define __CHDCD_H__

#include "cdrom.h"

typedef struct _chdcd_track_input_info chdcd_track_input_info;
struct _chdcd_track_input_info	/* used only at compression time */
{
	char fname[CD_MAX_TRACKS][256];	/* filename for each track */
	UINT32 offset[CD_MAX_TRACKS];	/* offset in the data file for each track */
	int swap[CD_MAX_TRACKS];	/* data needs to be byte swapped */
};


chd_error chdcd_parse_toc(const char *tocfname, cdrom_toc *outtoc, chdcd_track_input_info *outinfo);

#endif	/* __CHDCD_H__ */
