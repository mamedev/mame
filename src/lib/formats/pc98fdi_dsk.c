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

    formats/pc98fdi_dsk.h

    PC98FDI disk images

*********************************************************************/

#include "emu.h"
#include "pc98fdi_dsk.h"

pc98fdi_format::pc98fdi_format()
{
}

const char *pc98fdi_format::name() const
{
	return "pc98-fdi";
}

const char *pc98fdi_format::description() const
{
	return "PC98 FDI disk image";
}

const char *pc98fdi_format::extensions() const
{
	return "fdi";
}

int pc98fdi_format::identify(io_generic *io, UINT32 form_factor)
{
	int size = io_generic_size(io);
	UINT8 h[32];

	io_generic_read(io, h, 0, 32);
	int hsize = LITTLE_ENDIANIZE_INT32(*(UINT32 *)(h+0x8));
	int psize = LITTLE_ENDIANIZE_INT32(*(UINT32 *)(h+0xc));
	int ssize = LITTLE_ENDIANIZE_INT32(*(UINT32 *)(h+0x10));
	int scnt  = LITTLE_ENDIANIZE_INT32(*(UINT32 *)(h+0x14));
	int sides = LITTLE_ENDIANIZE_INT32(*(UINT32 *)(h+0x18));
	int ntrk  = LITTLE_ENDIANIZE_INT32(*(UINT32 *)(h+0x1c));
	if(size == hsize + psize && psize == ssize*scnt*sides*ntrk)
		return 100;

	return 0;
}

bool pc98fdi_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	UINT8 h[32];

	io_generic_read(io, h, 0, 32);

	int hsize         = LITTLE_ENDIANIZE_INT32(*(UINT32 *)(h+0x8));
	int sector_size   = LITTLE_ENDIANIZE_INT32(*(UINT32 *)(h+0x10));
	int sector_count  = LITTLE_ENDIANIZE_INT32(*(UINT32 *)(h+0x14));
	int head_count    = LITTLE_ENDIANIZE_INT32(*(UINT32 *)(h+0x18));
	int track_count   = LITTLE_ENDIANIZE_INT32(*(UINT32 *)(h+0x1c));

	int cell_count = form_factor == floppy_image::FF_35 ? 200000 : 166666;

	int ssize;
	for(ssize=0; (128 << ssize) < sector_size; ssize++);

	desc_pc_sector sects[256];
	UINT8 sect_data[65536];

	for(int track=0; track < track_count; track++)
		for(int head=0; head < head_count; head++) {
			io_generic_read(io, sect_data, hsize + sector_size*sector_count*(track*head_count + head), sector_size*sector_count);

			for(int i=0; i<sector_count; i++) {
				sects[i].track       = track;
				sects[i].head        = head;
				sects[i].sector      = i+1;
				sects[i].size        = ssize;
				sects[i].actual_size = sector_size;
				sects[i].deleted     = false;
				sects[i].bad_crc     = false;
				sects[i].data        = sect_data + i*sector_size;
			}

			build_pc_track_mfm(track, head, image, cell_count, sector_count, sects, calc_default_pc_gap3_size(form_factor, sector_size));
		}

	return true;
}

bool pc98fdi_format::supports_save() const
{
	return false;
}

const floppy_format_type FLOPPY_PC98FDI_FORMAT = &floppy_image_format_creator<pc98fdi_format>;
