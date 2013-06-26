/***************************************************************************

    vbiparse.h

    Parse Philips codes and other data from VBI lines.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#pragma once

#ifndef __VBIPARSE_H__
#define __VBIPARSE_H__


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
	UINT8       white;              /* white flag: on or off */
	UINT32      line16;             /* line 16 code */
	UINT32      line17;             /* line 17 code */
	UINT32      line18;             /* line 18 code */
	UINT32      line1718;           /* most plausible value from lines 17/18 */
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

/* pack the VBI data down into a smaller form for storage */
void vbi_metadata_pack(UINT8 *dest, UINT32 framenum, const vbi_metadata *vbi);

/* unpack the VBI data from a smaller form into the full structure */
void vbi_metadata_unpack(vbi_metadata *vbi, UINT32 *framenum, const UINT8 *source);


#endif /* __VBIPARSE_H__ */
