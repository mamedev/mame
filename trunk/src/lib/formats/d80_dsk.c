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

    formats/d80_dsk.c

    Commodore 8050/8250/SFD-1001 sector disk image format

*********************************************************************/

#include "emu.h"
#include "formats/d80_dsk.h"

d80_format::d80_format() : d64_format(formats)
{
}

const char *d80_format::name() const
{
	return "d80";
}

const char *d80_format::description() const
{
	return "Commodore 8050/8250/SFD-1001 disk image";
}

const char *d80_format::extensions() const
{
	return "d80,d82";
}

const d80_format::format d80_format::formats[] = {
	{ // d80, dos 2.5, 77 tracks, head/stepper 100 tpi
		floppy_image::FF_525, floppy_image::SSQD, 2083, 77, 1, 256, 9, 8
	},
	{ // d82, dos 2.5, 77 tracks, 2 heads, head/stepper 100 tpi
		floppy_image::FF_525, floppy_image::DSQD, 2083, 77, 2, 256, 9, 8
	},
	{}
};

const UINT32 d80_format::cell_size[] =
{
	2667, // 12MHz/16/2
	2500, // 12MHz/15/2
	2333, // 12MHz/14/2
	2167  // 12MHz/13/2
};

const int d80_format::sectors_per_track[] =
{
	29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, //  1-39
	29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
	27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,                         // 40-53
	25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,                                     // 54-64
	23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,                             // 65-77
	23, 23, 23, 23, 23, 23, 23                                                      // 78-84
};

const int d80_format::speed_zone[] =
{
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, //  1-39
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,                   // 40-53
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                            // 54-64
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                      // 65-77
	0, 0, 0, 0, 0, 0, 0                                         // 78-84
};

int d80_format::get_physical_track(const format &f, int track)
{
	return track;
}

UINT32 d80_format::get_cell_size(const format &f, int track)
{
	return cell_size[speed_zone[track]];
}

int d80_format::get_sectors_per_track(const format &f, int track)
{
	return sectors_per_track[track];
}

int d80_format::get_disk_id_offset(const format &f)
{
	return 282136;
}

const floppy_format_type FLOPPY_D80_FORMAT = &floppy_image_format_creator<d80_format>;
