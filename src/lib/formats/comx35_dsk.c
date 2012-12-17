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

    - implement 70 track image detection

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
	{   //  70K 5 1/4 inch single density single sided 35 tracks
		floppy_image::FF_525, floppy_image::SSSD,
		2000, 16, 35, 1, 128, {}, 0, {}, 100, 22, 84
	},
	{   //  140K 5 1/4 inch single density double sided 35 tracks
		floppy_image::FF_525, floppy_image::DSSD,
		2000, 16, 35, 2, 128, {}, 0, {}, 100, 22, 84
	},
	/*{   //  140K 5 1/4 inch quad density single sided 70 tracks
        floppy_image::FF_525, floppy_image::SSQD,
        2000, 16, 70, 1, 128, {}, 0, {}, 100, 22, 84
    },*/
	{}
};

const floppy_format_type FLOPPY_COMX35_FORMAT = &floppy_image_format_creator<comx35_format>;


#ifdef UNUSED_CODE

/*********************************************************************

    formats/comx35_dsk.c

    COMX35 disk images

*********************************************************************/

#include "formats/comx35_dsk.h"
#include "formats/basicdsk.h"

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    FLOPPY_IDENTIFY( comx35_dsk_identify )
-------------------------------------------------*/

static FLOPPY_IDENTIFY( comx35_dsk_identify )
{
	*vote = ((floppy_image_size(floppy) == (35*1*16*128)) || (floppy_image_size(floppy) == (35*2*16*128))) ? 100 : 0;

	return FLOPPY_ERROR_SUCCESS;
}

/*-------------------------------------------------
    FLOPPY_CONSTRUCT( comx35_dsk_construct )
-------------------------------------------------*/

static FLOPPY_CONSTRUCT( comx35_dsk_construct )
{
	UINT8 header[1];
	int heads = 1;
	int cylinders = 35;

	switch (floppy_image_size(floppy))
	{
	case 35*1*16*128:
		heads = 1;
		cylinders = 35;
		break;

	case 35*2*16*128:
		floppy_image_read(floppy, header, 0x12, 1);

		if (header[0] == 0x01)
		{
			heads = 1;
			cylinders = 70;
		}
		else
		{
			heads = 2;
			cylinders = 35;
		}
		break;
	}

	struct basicdsk_geometry geometry;
	memset(&geometry, 0, sizeof(geometry));

	geometry.heads = heads;
	geometry.first_sector_id = 0;
	geometry.sector_length = 128;
	geometry.tracks = cylinders;
	geometry.sectors = 16;

	return basicdsk_construct(floppy, &geometry);
}

/*-------------------------------------------------
    FLOPPY_OPTIONS( comx35 )
-------------------------------------------------*/

LEGACY_FLOPPY_OPTIONS_START( comx35 )
	LEGACY_FLOPPY_OPTION( comx35, "img", "COMX35 floppy disk image", comx35_dsk_identify, comx35_dsk_construct, NULL, NULL )
LEGACY_FLOPPY_OPTIONS_END

#endif
