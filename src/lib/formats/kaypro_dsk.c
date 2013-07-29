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

    formats/kaypro_dsk.c

    Kaypro disk image format

    There is no inter-sector info on these disks. It is simply a
    dump of the 512 bytes from each sector and track in order.
    It is just like a headerless quickload.

*********************************************************************/

#include "emu.h"
#include "formats/kaypro_dsk.h"

kayproii_format::kayproii_format() : wd177x_format(formats)
{
}

const char *kayproii_format::name() const
{
	return "kaypro";
}

const char *kayproii_format::description() const
{
	return "Kaypro disk image";
}

const char *kayproii_format::extensions() const
{
	return "dsk";
}

// gap info is a total guess
const kayproii_format::format kayproii_format::formats[] = {
	{   /*  191K 13cm double density single sided */
		floppy_image::FF_525,  floppy_image::SSDD, floppy_image::MFM,
		2000, 10, 40, 1, 512, {}, 1, {}, 32, 22, 31
	},
	{}
};

kaypro2x_format::kaypro2x_format() : wd177x_format(formats)
{
}

const char *kaypro2x_format::name() const
{
	return "kaypro";
}

const char *kaypro2x_format::description() const
{
	return "Kaypro disk image";
}

const char *kaypro2x_format::extensions() const
{
	return "dsk";
}

// gap info is a total guess
const kaypro2x_format::format kaypro2x_format::formats[] = {
	{   /*  382K 13cm double density double sided */
		floppy_image::FF_525,  floppy_image::DSDD, floppy_image::MFM,
		2000, 10, 80, 2, 512, {}, 1, {}, 32, 22, 31
	},
	{}
};

const floppy_format_type FLOPPY_KAYPROII_FORMAT = &floppy_image_format_creator<kayproii_format>;
const floppy_format_type FLOPPY_KAYPRO2X_FORMAT = &floppy_image_format_creator<kaypro2x_format>;
