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

    formats/mm_dsk.c

    mm format

*********************************************************************/

#include "emu.h"
#include "formats/mm_dsk.h"

mm1_format::mm1_format() : upd765_format(formats)
{
}

const char *mm1_format::name() const
{
	return "mm1";
}

const char *mm1_format::description() const
{
	return "Nokia MikroMikko 1 disk image";
}

const char *mm1_format::extensions() const
{
	return "dsk";
}

mm2_format::mm2_format() : upd765_format(formats)
{
}

const char *mm2_format::name() const
{
	return "mm2";
}

const char *mm2_format::description() const
{
	return "Nokia MikroMikko 2 disk image";
}

const char *mm2_format::extensions() const
{
	return "dsk";
}

// Unverified gap sizes
const mm1_format::format mm1_format::formats[] = {
	{
		floppy_image::FF_525, floppy_image::DSQD,
		2000, // 2us, 300rpm
		8, 80, 2,
		512, {},
		-1, { 1,4,7,2,5,8,3,6 },
		80, 50, 22, 80
	},
	{}
};

// Unverified gap sizes
const mm2_format::format mm2_format::formats[] = {
	{
		floppy_image::FF_525, floppy_image::DSDD,
		2000, // 2us, 300rpm
		9, 40, 2,
		512, {},
		1, {},
		80, 50, 22, 80
	},
	// 40 tracks but 18 sectors implying HD density at 300rpm, i.e. on
	// 3.5" media?  That makes no sense
	{
		floppy_image::FF_525, floppy_image::DSHD,
		1000, // 1us, 300rpm, otherwise it just won't fit
		18, 40, 2, // That line is just nonsense
		512, {},
		1, {},
		80, 50, 22, 80
	}
};

const floppy_format_type FLOPPY_MM1_FORMAT = &floppy_image_format_creator<mm1_format>;
const floppy_format_type FLOPPY_MM2_FORMAT = &floppy_image_format_creator<mm2_format>;
