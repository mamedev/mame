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

    formats/d67_dsk.c

    Commodore 2040 sector disk image format

*********************************************************************/

#include "emu.h"
#include "formats/d67_dsk.h"

d67_format::d67_format() : d64_format(formats)
{
}

const char *d67_format::name() const
{
	return "d67";
}

const char *d67_format::description() const
{
	return "Commodore 2040 disk image";
}

const char *d67_format::extensions() const
{
	return "d67";
}

const d67_format::format d67_format::formats[] = {
	{ // d67, dos 1, 35 tracks, head 48 tpi, stepper 96 tpi
		floppy_image::FF_525, floppy_image::SSSD, 690, 35, 1, 256, 9, 8
	},
	{}
};

const int d67_format::sectors_per_track[] =
{
	21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, //  1-17
	20, 20, 20, 20, 20, 20, 20,                                         // 18-24
	18, 18, 18, 18, 18, 18,                                             // 25-30
	17, 17, 17, 17, 17,                                                 // 31-35
	17, 17, 17, 17, 17,                                                 // 36-40
	17, 17                                                              // 41-42
};

int d67_format::get_sectors_per_track(const format &f, int track)
{
	return sectors_per_track[track];
}

const floppy_format_type FLOPPY_D67_FORMAT = &floppy_image_format_creator<d67_format>;
