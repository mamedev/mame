// license:BSD-3-Clause
/***************************************************************************

    gdrom.cpp

    GDROM routines

***************************************************************************/


#include "cdrom.h"
#include "gdrom.h"


/*---------------------------------------------------------------------------------------
    gdrom_identify_pattern - Dreamcast has well-known standardised GDROM patterns
----------------------------------------------------------------------------------------*/

/**
 * Dreamcast GDROM patterns are identified by track types and number of tracks
 *
 *   Pattern I - (SD) DATA + AUDIO, (HD) DATA
 *   Pattern II - (SD) DATA + AUDIO, (HD) DATA + ... + AUDIO
 *   Pattern III - (SD) DATA + AUDIO, (HD) DATA + ... + DATA
 *
 * And a III variant when two HD DATA tracks are split by one or more AUDIO tracks.
 */

enum gdrom_pattern gdrom_identify_pattern(const cdrom_toc *toc)
{
	if (toc->numtrks > 4 && toc->tracks[toc->numtrks-1].trktype == CD_TRACK_MODE1_RAW)
	{
		if (toc->tracks[toc->numtrks-2].trktype == CD_TRACK_AUDIO)
			return GDROM_TYPE_III_SPLIT;
		else
			return GDROM_TYPE_III;
	}
	else if (toc->numtrks > 3)
	{
		if (toc->tracks[toc->numtrks-1].trktype == CD_TRACK_AUDIO)
			return GDROM_TYPE_II;
		else
			return GDROM_TYPE_III;
	}
	else if (toc->numtrks == 3)
	{
		return GDROM_TYPE_I;
	}

	return GDROM_TYPE_UNKNOWN;
}
