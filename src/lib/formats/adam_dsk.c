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

    formats/adam_dsk.c

    Coleco Adam disk image format

*********************************************************************/

#include "emu.h"
#include "formats/adam_dsk.h"

adam_format::adam_format() : wd177x_format(formats)
{
}

const char *adam_format::name() const
{
	return "adam";
}

const char *adam_format::description() const
{
	return "Coleco Adam disk image";
}

const char *adam_format::extensions() const
{
	return "dsk";
}

// Unverified gap sizes
const adam_format::format adam_format::formats[] = {
	{   /*  160K 5 1/4 inch double density single sided */
		floppy_image::FF_525, floppy_image::SSDD,
		2000,  8, 40, 1, 512, {}, 1, {}, 100, 22, 84
	},
	{   /*  320K 5 1/4 inch double density */
		floppy_image::FF_525, floppy_image::DSDD,
		2000,  8, 40, 2, 512, {}, 1, {}, 100, 22, 84
	},
	{   /*  640K 5 1/4 inch quad density */
		floppy_image::FF_525, floppy_image::DSQD,
		2000,  8, 80, 2, 512, {}, 1, {}, 100, 22, 84
	},
	{   /*  720K 3 1/2 inch double density */
		floppy_image::FF_35,  floppy_image::DSDD,
		2000,  9, 80, 2, 512, {}, 1, {}, 100, 22, 84
	},
	{   /* 1440K 3 1/2 inch high density */
		floppy_image::FF_35,  floppy_image::DSHD,
		1000, 18, 80, 2, 512, {}, 1, {}, 100, 22, 84
	},
	{}
};

const floppy_format_type FLOPPY_ADAM_FORMAT = &floppy_image_format_creator<adam_format>;
