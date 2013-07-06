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

    formats/mbee_dsk.c

    Microbee disk image format

*********************************************************************/

#include "emu.h"
#include "formats/mbee_dsk.h"

mbee_format::mbee_format() : wd177x_format(formats)
{
}

const char *mbee_format::name() const
{
	return "mbee";
}

const char *mbee_format::description() const
{
	return "Microbee disk image";
}

const char *mbee_format::extensions() const
{
	return "ss80,ds40,ds80,ds82,ds84";
}

// Unverified gap sizes
const mbee_format::format mbee_format::formats[] = {
	{   /*  ss80 3 1/2 inch double density */
		floppy_image::FF_35,  floppy_image::SSDD, floppy_image::MFM,
		2000,  10, 80, 1, 512, {}, 1, {}, 32, 22, 31
	},
	{   /*  ds40 5.25 inch double density */
		floppy_image::FF_525,  floppy_image::DSDD, floppy_image::MFM,
		2000,  10, 40, 2, 512, {}, 1, {}, 32, 22, 31
	},
	{   /*  ds80 3 1/2 inch double density */
		floppy_image::FF_35,  floppy_image::DSDD, floppy_image::MFM,
		2000,  10, 80, 2, 512, {}, 21, {}, 32, 22, 31
	},
	{   /*  ds82,ds84 3 1/2 inch double density */
		floppy_image::FF_35,  floppy_image::DSDD, floppy_image::MFM,
		2000,  10, 80, 2, 512, {}, 1, {}, 32, 22, 31
	},
	{}
};

const floppy_format_type FLOPPY_MBEE_FORMAT = &floppy_image_format_creator<mbee_format>;
