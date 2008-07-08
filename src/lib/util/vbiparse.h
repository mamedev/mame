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
    FUNCTION PROTOTYPES
***************************************************************************/

/* parse a Philips code from a line of video data */
int vbi_parse_line(const UINT16 *source, int sourcewidth, int sourceshift, int expectedbits, UINT8 *result);

#endif /* __VBIPARSE_H__ */
