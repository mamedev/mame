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

    formats/sf7000_dsk.c

    sf7000 format

*********************************************************************/

#include "emu.h"
#include "formats/sf7000_dsk.h"

sf7000_format::sf7000_format() : upd765_format(formats)
{
}

const char *sf7000_format::name() const
{
	return "sf7";
}

const char *sf7000_format::description() const
{
	return "SF7 disk image";
}

const char *sf7000_format::extensions() const
{
	return "sf7";
}

const sf7000_format::format sf7000_format::formats[] = {
	{
    // mfm h=00 n=01 sc=10 gpl=2a d=ff
		floppy_image::FF_3, floppy_image::SSDD, floppy_image::MFM,
		2000, // 2us, 300rpm
		16, 40, 1,
		256, {},
		1, {},
		80, 50, 22, 42
	},
	{}
};

const floppy_format_type FLOPPY_SF7000_FORMAT = &floppy_image_format_creator<sf7000_format>;
