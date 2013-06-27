/***************************************************************************

    Copyright Olivier Galibert
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

****************************************************************************/

/*********************************************************************

    formats/comx35_dsk.c

    COMX-35 disk image format

*********************************************************************/

/*

    TODO:

    - implement 70 track image detection (first byte of file = 0x01 -> single sided)
    - format double sided, single track density

        :exp:fd:wd1770: track description
        40xff 6x00
        fe 4x00 f7 17x00 fb 128x00 f7 17x00
        fe 2x00 07 00 f7 17x00 fb 128x00 f7 17x00
        fe 2x00 0e 00 f7 17x00 fb 128x00 f7 16x00
        fe 2x00 05 00 f7 17x00 fb 128x00 f7 17x00
        fe 2x00 0c 00 f7 17x00 fb 128x00 f7 16x00
        fe 2x00 03 00 f7 17x00 fb 128x00 f7 17x00
        fe 2x00 0a 00 f7 17x00 fb 128x00 f7 16x00
        fe 2x00 01 00 f7 17x00 fb 128x00 f7 17x00
        fe 2x00 08 00 f7 17x00 fb 128x00 f7 17x00
        fe 2x00 0f 00 f7 17x00 fb 128x00 f7 16x00
        fe 2x00 06 00 f7 17x00 fb 128x00 f7 17x00
        fe 2x00 0d 00 f7 17x00 fb 128x00 f7 16x00
        fe 2x00 04 00 f7 17x00 fb 128x00 f7 17x00
        fe 2x00 0b 00 f7 17x00 fb 128x00 f7 16x00
        fe 2x00 02 00 f7 17x00 fb 128x00 f7 17x00
        fe 2x00 09 00 f7 17x00 fb 128x00 f7 3476x00

*/

#include "emu.h"
#include "formats/comx35_dsk.h"

comx35_format::comx35_format() : wd177x_format(formats)
{
}

const char *comx35_format::name() const
{
	return "comx35";
}

const char *comx35_format::description() const
{
	return "COMX-35 disk image";
}

const char *comx35_format::extensions() const
{
	return "img";
}

// Unverified gap sizes
const comx35_format::format comx35_format::formats[] = {
	{   //  70K 5 1/4 inch single density, single sided, 35 tracks
		floppy_image::FF_525, floppy_image::SSSD, floppy_image::FM,
		4000, 16, 35, 1, 128, {}, -1, { 0,7,14,5,12,3,10,1,8,15,6,13,4,11,2,9 }, 40, 11, 10
	},
	{   //  140K 5 1/4 inch single density, double sided, 35 tracks
		floppy_image::FF_525, floppy_image::DSSD, floppy_image::FM,
		4000, 16, 35, 2, 128, {}, -1, { 0,7,14,5,12,3,10,1,8,15,6,13,4,11,2,9 }, 40, 11, 10
	},
	/*{   //  140K 5 1/4 inch single density, double track density, single sided, 70 tracks
	    floppy_image::FF_525, floppy_image::SSQD, floppy_image::FM,
	    4000, 16, 70, 1, 128, {}, -1, { 0,7,14,5,12,3,10,1,8,15,6,13,4,11,2,9 }, 40, 11, 10
	},*/
	{}
};

const floppy_format_type FLOPPY_COMX35_FORMAT = &floppy_image_format_creator<comx35_format>;
