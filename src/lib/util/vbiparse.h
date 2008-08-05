/***************************************************************************

    vbiparse.h

    Parse Philips codes and other data from VBI lines.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __VBIPARSE_H__
#define __VBIPARSE_H__


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _vbi_metadata vbi_metadata;
struct _vbi_metadata
{
	UINT8		white;				/* white flag: on or off */
	UINT32		line16;				/* line 16 code */
	UINT32		line17;				/* line 17 code */
	UINT32		line18;				/* line 18 code */
	UINT32		line1718;			/* most plausible value from lines 17/18 */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* parse a Manchester code from a line of video data */
int vbi_parse_manchester_code(const UINT16 *source, int sourcewidth, int sourceshift, int expectedbits, UINT32 *result);

/* compute the "white flag" from a line of video data */
int vbi_parse_white_flag(const UINT16 *source, int sourcewidth, int sourceshift);

/* parse everything from a video frame */
void vbi_parse_all(const UINT16 *source, int sourcerowpixels, int sourcewidth, int sourceshift, vbi_metadata *vbi);


#endif /* __VBIPARSE_H__ */
