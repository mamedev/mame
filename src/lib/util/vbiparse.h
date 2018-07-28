// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    vbiparse.h

    Parse Philips codes and other data from VBI lines.

***************************************************************************/

#ifndef MAME_UTIL_VBIPARSE_H
#define MAME_UTIL_VBIPARSE_H

#pragma once

#include "osdcomm.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

/* size of packed VBI data */
#define VBI_PACKED_BYTES        16

/* these codes are full 24-bit codes with no parameter data */
#define VBI_CODE_LEADIN         0x88ffff
#define VBI_CODE_LEADOUT        0x80eeee
#define VBI_CODE_STOP           0x82cfff
#define VBI_CODE_CLV            0x87ffff

/* these codes require a mask because some bits are parameters */
#define VBI_MASK_CAV_PICTURE    0xf00000
#define VBI_CODE_CAV_PICTURE    0xf00000
#define VBI_MASK_CHAPTER        0xf00fff
#define VBI_CODE_CHAPTER        0x800ddd
#define VBI_MASK_CLV_TIME       0xf0ff00
#define VBI_CODE_CLV_TIME       0xf0dd00
#define VBI_MASK_STATUS_CX_ON   0xfff000
#define VBI_CODE_STATUS_CX_ON   0x8dc000
#define VBI_MASK_STATUS_CX_OFF  0xfff000
#define VBI_CODE_STATUS_CX_OFF  0x8bc000
#define VBI_MASK_USER           0xf0f000
#define VBI_CODE_USER           0x80d000
#define VBI_MASK_CLV_PICTURE    0xf0f000
#define VBI_CODE_CLV_PICTURE    0x80e000



/***************************************************************************
    MACROS
***************************************************************************/

#define VBI_CAV_PICTURE(x)      (((((x) >> 16) & 0x07) * 10000) + ((((x) >> 12) & 0x0f) * 1000) + ((((x) >> 8) & 0x0f) * 100) + ((((x) >> 4) & 0x0f) * 10) + ((((x) >> 0) & 0x0f) * 1))
#define VBI_CHAPTER(x)          (((((x) >> 16) & 0x07) * 10) + ((((x) >> 12) & 0x0f) * 1))



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct vbi_metadata
{
	uint8_t       white;              /* white flag: on or off */
	uint32_t      line16;             /* line 16 code */
	uint32_t      line17;             /* line 17 code */
	uint32_t      line18;             /* line 18 code */
	uint32_t      line1718;           /* most plausible value from lines 17/18 */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* parse a Manchester code from a line of video data */
int vbi_parse_manchester_code(const uint16_t *source, int sourcewidth, int sourceshift, int expectedbits, uint32_t *result);

/* compute the "white flag" from a line of video data */
bool vbi_parse_white_flag(const uint16_t *source, int sourcewidth, int sourceshift);

/* parse everything from a video frame */
void vbi_parse_all(const uint16_t *source, int sourcerowpixels, int sourcewidth, int sourceshift, vbi_metadata *vbi);

/* pack the VBI data down into a smaller form for storage */
void vbi_metadata_pack(uint8_t *dest, uint32_t framenum, const vbi_metadata *vbi);

/* unpack the VBI data from a smaller form into the full structure */
void vbi_metadata_unpack(vbi_metadata *vbi, uint32_t *framenum, const uint8_t *source);


#endif // MAME_UTIL_VBIPARSE_H
