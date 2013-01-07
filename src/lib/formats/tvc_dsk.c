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

    formats/tvc_dsk.c

    tvc format

*********************************************************************/

#include "emu.h"
#include "formats/tvc_dsk.h"

tvc_format::tvc_format() : wd177x_format(formats)
{
}

const char *tvc_format::name() const
{
	return "tvc";
}

const char *tvc_format::description() const
{
	return "Videoton TVC HBF disk image";
}

const char *tvc_format::extensions() const
{
	return "img,dsk";
}

// Unverified gap sizes
const tvc_format::format tvc_format::formats[] =
{
	{   //  720K 5.25 inch
		floppy_image::FF_525,  floppy_image::DSQD, floppy_image::MFM,
		2000, 9, 80, 2, 512, {}, 1, {}, 100, 22, 20
	},
	{   //  360K 5.25 inch
		floppy_image::FF_525,  floppy_image::SSQD, floppy_image::MFM,
		2000, 9, 80, 1, 512, {}, 1, {}, 100, 22, 20
	},
	{}
};

const floppy_format_type FLOPPY_TVC_FORMAT = &floppy_image_format_creator<tvc_format>;
