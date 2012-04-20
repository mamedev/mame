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

#include "emu.h"
#include "dfi_dsk.h"
#include <zlib.h>
#define NUMBER_OF_MULTIREADS 1

dfi_format::dfi_format() : floppy_image_format_t()
{
}

const char *dfi_format::name() const
{
	return "dfi";
}

const char *dfi_format::description() const
{
	return "DiscFerret flux dump format";
}

const char *dfi_format::extensions() const
{
	return "dfi";
}

bool dfi_format::supports_save() const
{
	return false;
}

int dfi_format::identify(io_generic *io, UINT32 form_factor)
{
	char sign[4];
	io_generic_read(io, sign, 0, 4);
	return memcmp(sign, "DFE2", 4) ? 0 : 100;
}

bool dfi_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	int size = io_generic_size(io);
	int pos = 4;
	UINT8 *data = 0;
	int data_size = 0;

	while(pos < size) {
		UINT8 h[10];
		io_generic_read(io, h, pos, 10);
		UINT16 track = (h[0] << 8) | h[1];
		UINT16 head  = (h[2] << 8) | h[3];
		// Ignore sector
		UINT32 tsize = (h[6] << 24) | (h[7] << 16) | (h[8] << 8) | h[9];

		if(pos+tsize+10 > size) {
			if(data)
				global_free(data);
			return false;
		}

		if(tsize > data_size) {
			if(data)
				global_free(data);
			data_size = tsize;
			data = global_alloc_array(UINT8, data_size);
		}

		io_generic_read(io, data, pos+16, tsize);
		pos += tsize+10;
		tsize--; // Drop the extra 0x00 at the end

		int index_time = 0;
		int index_count = 0;
		int total_time = 0;
		for(int i=0; i<tsize; i++) {
			UINT8 v = data[i];
			if((v & 0x7f) == 0x7f)
				total_time += 0x7f;
			else {
				total_time += v & 0x7f;
				if((v & 0x80) && (index_count<NUMBER_OF_MULTIREADS)) {
					index_time = total_time;///NUMBER_OF_MULTIREADS;
					index_count++;
				}
			}
		}

		if(!track && !head)
			fprintf(stderr, "%02d:%d tt=%10d it=%10d\n", track, head, total_time, index_time);

		if(!index_time)
			index_time = total_time;

		image->set_track_size(track, head, tsize);

		int cur_time = 0;
		index_count = 0;
		int index_polarity = 0;
		UINT32 mg = floppy_image::MG_A;
		UINT32 *buf = image->get_buffer(track, head);
		int tpos = 0;
		buf[tpos++] = mg;
		for(int i=0; i<tsize; i++) {
			UINT8 v = data[i];
			if((v & 0x7f) == 0x7f) // 0x7F or 0xFF: no transition, but a carry
				cur_time += 0x7f;
			else if((v & 0x80) == 0) { // 0x00-0x7E: not an index or carry, add the number and store transition
				cur_time += v & 0x7f;
				mg = mg == floppy_image::MG_A ? floppy_image::MG_B : floppy_image::MG_A;
				buf[tpos++] = mg | UINT32(200000000ULL*cur_time/index_time);
			} else if(v & 0x80) { // 0x80-0xFF an index, note the index (TODO: actually do this!) and do not add number
				index_polarity ^= 1;
				index_count += index_polarity;
				if (index_count == NUMBER_OF_MULTIREADS) break;
			}
		}
		image->set_track_size(track, head, tpos);
	}

	if(data)
		global_free(data);

	return true;
}

const floppy_format_type FLOPPY_DFI_FORMAT = &floppy_image_format_creator<dfi_format>;
