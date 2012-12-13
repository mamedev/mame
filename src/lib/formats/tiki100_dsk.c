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

    formats/tiki100_dsk.c

    TIKI 100 disk image format

*********************************************************************/

#include "emu.h"
#include "formats/tiki100_dsk.h"

tiki100_format::tiki100_format() : wd177x_format(formats)
{
}

const char *tiki100_format::name() const
{
	return "tiki100";
}

const char *tiki100_format::description() const
{
	return "TIKI 100 disk image";
}

const char *tiki100_format::extensions() const
{
	return "dsk";
}

// Unverified gap sizes
// double sided disks have t0s0,t0s1,t1s0,t1s1... format
const tiki100_format::format tiki100_format::formats[] = {
	{   //  90K 5 1/4 inch single density single sided
		floppy_image::FF_525, floppy_image::SSSD,
		2000, 18, 40, 1, 128, {}, 1, {}, 80, 22, 20
	},
	{   //  200K 5 1/4 inch double density single sided
		floppy_image::FF_525, floppy_image::SSDD,
        2000, 10, 40, 1, 512, {}, -1, { 1,6,2,7,3,8,4,9,5,10 }, 80, 22, 20
	},
	{   //  400K 5 1/4 inch double density
		floppy_image::FF_525, floppy_image::DSDD,
        2000, 10, 40, 2, 512, {}, -1, { 1,6,2,7,3,8,4,9,5,10 }, 80, 22, 20
	},
	{   //  800K 5 1/4 inch quad density
		floppy_image::FF_525, floppy_image::DSQD,
        2000, 10, 80, 2, 512, {}, -1, { 1,6,2,7,3,8,4,9,5,10 }, 80, 22, 20
	},
	{}
};

const floppy_format_type FLOPPY_TIKI100_FORMAT = &floppy_image_format_creator<tiki100_format>;
