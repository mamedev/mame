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

    formats/bw12_dsk.c

    bw12 format

*********************************************************************/

#include "emu.h"
#include "formats/bw12_dsk.h"

bw12_format::bw12_format() : upd765_format(formats)
{
}

const char *bw12_format::name() const
{
	return "bw12";
}

const char *bw12_format::description() const
{
	return "Bronwell 12/14 disk image";
}

const char *bw12_format::extensions() const
{
	return "dsk";
}

// Unverified gap sizes
const bw12_format::format bw12_format::formats[] = {
	{ // 180KB BW 12
		floppy_image::FF_525, floppy_image::SSDD,
		2000, // 2us, 300rpm
		18, 40, 1,
		256, {},
		0, {},
		80, 50, 22, 80
	},
	{ // 360KB BW 12
		floppy_image::FF_525, floppy_image::DSDD,
		2000, // 2us, 300rpm
		18, 40, 2,
		256, {},
		0, {},
		80, 50, 22, 80
	},
	{ // SVI-328
		floppy_image::FF_525, floppy_image::SSDD,
		2000, // 2us, 300rpm
		17, 40, 1,
		256, {},
		0, {},
		80, 50, 22, 80
	},
	{ // SVI-328
		floppy_image::FF_525, floppy_image::DSDD,
		2000, // 2us, 300rpm
		17, 40, 2,
		256, {},
		0, {},
		80, 50, 22, 80
	},
	{ // Kaypro II
		floppy_image::FF_525, floppy_image::SSDD,
		2000, // 2us, 300rpm
		10, 40, 1,
		512, {},
		0, {},
		80, 50, 22, 80
	},
	{}
};

const floppy_format_type FLOPPY_BW12_FORMAT = &floppy_image_format_creator<bw12_format>;
