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

#include "hxcmfm_dsk.h"

#define MFM_FORMAT_HEADER   "HXCMFM"

#pragma pack(1)

struct MFMIMG
{
	UINT8 headername[7];

	UINT16 number_of_track;
	UINT8 number_of_side;

	UINT16 floppyRPM;
	UINT16 floppyBitRate;
	UINT8 floppyiftype;

	UINT32 mfmtracklistoffset;
};

struct MFMTRACKIMG
{
	UINT16 track_number;
	UINT8 side_number;
	UINT32 mfmtracksize;
	UINT32 mfmtrackoffset;
};

#pragma pack()

mfm_format::mfm_format() : floppy_image_format_t()
{
}

const char *mfm_format::name() const
{
	return "mfm";
}

const char *mfm_format::description() const
{
	return "HxCFloppyEmulator floppy disk image";
}

const char *mfm_format::extensions() const
{
	return "mfm";
}

bool mfm_format::supports_save() const
{
	return true;
}

int mfm_format::identify(io_generic *io, UINT32 form_factor)
{
	UINT8 header[7];

	io_generic_read(io, &header, 0, sizeof(header));
	if ( memcmp( header, MFM_FORMAT_HEADER, 6 ) ==0) {
		return 100;
	}
	return 0;
}

bool mfm_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	MFMIMG header;
	MFMTRACKIMG trackdesc;
	UINT8 *trackbuf = 0;
	int trackbuf_size = 0;

	// read header
	io_generic_read(io, &header, 0, sizeof(header));
	int counter = 0;
	for(int track=0; track < header.number_of_track; track++) {
		for(int side=0; side < header.number_of_side; side++) {
			// read location of
			io_generic_read(io, &trackdesc,(header.mfmtracklistoffset)+( counter *sizeof(trackdesc)),sizeof(trackdesc));

			if(trackdesc.mfmtracksize > trackbuf_size) {
				if(trackbuf)
					global_free(trackbuf);
				trackbuf_size = trackdesc.mfmtracksize;
				trackbuf = global_alloc_array(UINT8, trackbuf_size);
			}

			// actual data read
			io_generic_read(io, trackbuf, trackdesc.mfmtrackoffset, trackdesc.mfmtracksize);

			generate_track_from_bitstream(track, side, trackbuf, trackdesc.mfmtracksize*8, image);

			counter++;
		}
	}
	if(trackbuf)
		global_free(trackbuf);

	image->set_variant(floppy_image::DSDD);
	return true;
}

bool mfm_format::save(io_generic *io, floppy_image *image)
{
	// TODO: HD support
	MFMIMG header;
	int track_count, head_count;
	image->get_actual_geometry(track_count, head_count);

	memcpy(&header.headername, MFM_FORMAT_HEADER, 7);
	header.number_of_track = track_count;
	header.number_of_side = head_count;
	header.floppyRPM = 0;
	header.floppyBitRate = 250;
	header.floppyiftype = 4;
	header.mfmtracklistoffset = sizeof(MFMIMG);

	io_generic_write(io, &header, 0, sizeof(MFMIMG));

	int tpos = sizeof(MFMIMG);
	int dpos = tpos + track_count*head_count*sizeof(MFMTRACKIMG);

	UINT8 trackbuf[150000/8];

	for(int track=0; track < track_count; track++) {
		for(int side=0; side < head_count; side++) {
			int track_size;
			generate_bitstream_from_track(track, side, 2000, trackbuf, track_size, image);
			track_size = (track_size+7)/8;

			MFMTRACKIMG trackdesc;
			trackdesc.track_number = track;
			trackdesc.side_number = side;
			trackdesc.mfmtracksize = track_size;
			trackdesc.mfmtrackoffset = dpos;

			io_generic_write(io, &trackdesc, tpos, sizeof(MFMTRACKIMG));
			io_generic_write(io, trackbuf, dpos, track_size);

			tpos += sizeof(MFMTRACKIMG);
			dpos += track_size;
		}
	}

	return true;
}

const floppy_format_type FLOPPY_MFM_FORMAT = &floppy_image_format_creator<mfm_format>;
