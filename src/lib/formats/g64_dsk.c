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

    formats/g64_dsk.c

    Commodore 1541/1571 GCR disk image format

*********************************************************************/

#include "emu.h"
#include "formats/g64_dsk.h"

#define G64_FORMAT_HEADER 	"GCR-1541"

g64_format::g64_format()
{
}

const char *g64_format::name() const
{
	return "g64";
}

const char *g64_format::description() const
{
	return "Commodore 1541 GCR disk image";
}

const char *g64_format::extensions() const
{
	return "g64,g41,g71";
}

const UINT32 g64_format::c1541_cell_size[] =
{
	4000, // 16MHz/16/4
	3750, // 16MHz/15/4
	3500, // 16MHz/14/4
	3250  // 16MHz/13/4
};

int g64_format::identify(io_generic *io, UINT32 form_factor)
{
	UINT8 header[8];

	io_generic_read(io, &header, 0, sizeof(header));
	if ( memcmp( header, G64_FORMAT_HEADER, 8 ) == 0) {
		return 100;
	}
	return 0;
}

bool g64_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	int size = io_generic_size(io);
	UINT8 *img = global_alloc_array(UINT8, size);
	io_generic_read(io, img, 0, size);

	int version = img[0x08];
	if (version)
		throw emu_fatalerror("g64_format: Unsupported version %u", version);

	int track_count = img[0x09];

	int pos = 0x0c;
	int track_offset[track_count];
	for(int track = 0; track < track_count; track++) {
		track_offset[track] = pick_integer_le(img, pos, 4);
		pos += 4;
	}

	int speed_zone_offset[track_count];
	for(int track = 0; track < track_count; track++) {
		speed_zone_offset[track] = pick_integer_le(img, pos, 4);
		pos += 4;
	}

	for(int track = 0; track < track_count; track++) {
		int track_size = 0;
		pos = track_offset[track];
		if (pos > 0) {
			track_size = pick_integer_le(img, pos, 2);
			pos +=2;
		
			if (speed_zone_offset[track] > 3)
				throw emu_fatalerror("g64_format: Unsupported variable speed zones on track %d", track);

			UINT32 cell_size = c1541_cell_size[speed_zone_offset[track]];
			int total_size = 200000000/cell_size;
			UINT32 *buffer = global_alloc_array_clear(UINT32, total_size);
			int offset = 0;

			for (int i=0; i<track_size; i++, pos++) {
				for (int bit=7; bit>=0; bit--) {
					bit_w(buffer, offset++, BIT(img[pos], bit), cell_size);
					if (offset == total_size) break;
				}
			}

			if (offset < total_size) {
				// pad the remainder of the track with sync
				int count = (total_size-offset);
				for (int i=0; i<count;i++) {
					bit_w(buffer, offset++, 1, cell_size);
				}
			}

			int physical_track = track >= 84 ? track - 84 : track;
			int head = track >= 84;

			generate_track_from_levels(physical_track, head, buffer, total_size, 0, image);
			global_free(buffer);
		}
	}

	image->set_variant(floppy_image::SSSD);

	return true;
}

bool g64_format::save(io_generic *io, floppy_image *image)
{
	return false;
}

bool g64_format::supports_save() const
{
	return false;
}

const floppy_format_type FLOPPY_G64_FORMAT = &floppy_image_format_creator<g64_format>;


// ------ LEGACY -----


/*********************************************************************

    formats/g64_dsk.c

    Floppy format code for Commodore 1541 GCR disk images

    http://www.unusedino.de/ec64/technical/formats/g64.html

*********************************************************************/

/*

    TODO:

    - write to disk

*/

#define XTAL_16MHz      16000000
#define XTAL_12MHz      12000000

#include "g64_dsk.h"
#include "flopimg.h"
#include "imageutl.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define HEADER_LENGTH       0x2ac           // standard length for 84 half tracks
#define MAX_TRACKS          84

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct g64dsk_tag
{
	int version;
	int heads;                              // number of physical heads
	int tracks;                             // number of physical tracks
	UINT16 track_size[MAX_TRACKS];          // size of each track
	UINT32 track_offset[MAX_TRACKS];        // offset within data for each track
	UINT32 speed_zone_offset[MAX_TRACKS];   // offset within data for each track
};

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE struct g64dsk_tag *get_tag(floppy_image_legacy *floppy)
{
	return (g64dsk_tag *)floppy_tag(floppy);
}

INLINE float get_dos_track(int track)
{
	return ((float)track / 2) + 1;
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    g64_get_heads_per_disk - returns the number
    of heads in the disk image
-------------------------------------------------*/

static int g64_get_heads_per_disk(floppy_image_legacy *floppy)
{
	return get_tag(floppy)->heads;
}

/*-------------------------------------------------
    g64_get_tracks_per_disk - returns the number
    of DOS tracks in the disk image
-------------------------------------------------*/

static int g64_get_tracks_per_disk(floppy_image_legacy *floppy)
{
	return get_tag(floppy)->tracks;
}

/*-------------------------------------------------
    get_track_offset - returns the offset within
    the disk image for a given track
-------------------------------------------------*/

static floperr_t get_track_offset(floppy_image_legacy *floppy, int head, int track, UINT64 *offset)
{
	struct g64dsk_tag *tag = get_tag(floppy);
	UINT64 offs = 0;

	if ((track < 0) || (track >= tag->tracks))
		return FLOPPY_ERROR_SEEKERROR;

	offs = tag->track_offset[track];

	if (offset)
		*offset = offs;

	return FLOPPY_ERROR_SUCCESS;
}

/*-------------------------------------------------
    g64_get_track_size - returns the track size
-------------------------------------------------*/

static UINT32 g64_get_track_size(floppy_image_legacy *floppy, int head, int track)
{
	// get track offset
	UINT32 track_length = 0;
	UINT64 track_offset;
	floperr_t err = get_track_offset(floppy, head, track, &track_offset);

	if (err)
		return 0;

	// read track length
	if (track_offset > 0)
	{
		UINT8 size[2];
		floppy_image_read(floppy, &size, track_offset, 2);
		track_length = pick_integer_le(size, 0, 2);
	}

	return track_length;
}

/*-------------------------------------------------
    g64_read_track - reads a full track from the
    disk image
-------------------------------------------------*/

static floperr_t g64_read_track(floppy_image_legacy *floppy, int head, int track, UINT64 offset, void *buffer, size_t buflen)
{
	/*

	    The following is written into the buffer:

	    n bytes of track data
	    1982 bytes of speed zone data

	    get_track_size() returns n (size of track data)

	*/

	struct g64dsk_tag *tag = get_tag(floppy);
	floperr_t err;
	UINT64 track_offset;
	UINT16 track_length = tag->track_size[track];

	// get track offset
	err = get_track_offset(floppy, head, track, &track_offset);

	if (err)
		return err;

	if (!head && track_offset)
	{
		if (buflen < track_length) { printf("G64 track buffer too small: %u < %u!", (UINT32)buflen, track_length); exit(-1); }

		// read track data
		floppy_image_read(floppy, ((UINT8*)buffer), track_offset + 2, track_length); // skip the 2 track length bytes in the beginning of track data

		if (tag->speed_zone_offset[track] > 3)
		{
			// read speed block
			floppy_image_read(floppy, ((UINT8*)buffer) + track_length, tag->speed_zone_offset[track], G64_SPEED_BLOCK_SIZE);
		}
		else
		{
			// create a speed block with the same speed zone for the whole track
			UINT8 speed = tag->speed_zone_offset[track] & 0x03;
			UINT8 speed_byte = (speed << 6) | (speed << 4) | (speed << 2) | speed;

			memset(((UINT8*)buffer) + track_length, speed_byte, G64_SPEED_BLOCK_SIZE);
		}
	}
	else
	{
		// no data for given track, or tried to read side 1
		memset(((UINT8*)buffer), 0, buflen);
	}

	LOG_FORMATS("G64 track %.1f length %u\n", get_dos_track(track), track_length);

	return FLOPPY_ERROR_SUCCESS;
}

/*-------------------------------------------------
    g64_write_track - writes a full track to the
    disk image
-------------------------------------------------*/

static floperr_t g64_write_track(floppy_image_legacy *floppy, int head, int track, UINT64 offset, const void *buffer, size_t buflen)
{
	return FLOPPY_ERROR_UNSUPPORTED;
}

/*-------------------------------------------------
    FLOPPY_IDENTIFY( g64_dsk_identify )
-------------------------------------------------*/

FLOPPY_IDENTIFY( g64_dsk_identify )
{
	UINT8 header[8];

	floppy_image_read(floppy, header, 0, 8);

	if (!strncmp((const char *)header, "GCR-1541", 8))
	{
		*vote = 100;
	}
	else
	{
		*vote = 0;
	}

	return FLOPPY_ERROR_SUCCESS;
}

/*-------------------------------------------------
    FLOPPY_CONSTRUCT( g64_dsk_construct )
-------------------------------------------------*/

/*

    G64 File Header Format

  Bytes: $0000-0007: File signature "GCR-1541"
               0008: G64 version (presently only $00 defined)
               0009: Number of tracks in image (usually $54, decimal 84)
          000A-000B: Size of each stored track in bytes (usually  7928,  or
                     $1EF8 in LO/HI format.

  Bytes: $000C-000F: Offset  to  stored  track  1.0  ($000002AC,  in  LO/HI
                     format, see below for more)
          0010-0013: Offset to stored track 1.5 ($00000000)
          0014-0017: Offset to stored track 2.0 ($000021A6)
             ...
          0154-0157: Offset to stored track 42.0 ($0004F8B6)
          0158-015B: Offset to stored track 42.5 ($00000000)

  Bytes: $015C-015F: Speed zone entry for track 1 ($03,  in  LO/HI  format,
                     see below for more)
          0160-0163: Speed zone entry for track 1.5 ($03)
             ...
          02A4-02A7: Speed zone entry for track 42 ($00)
          02A8-02AB: Speed zone entry for track 42.5 ($00)

  Bytes: $02AC-02AD: Actual size of stored track (7692 or $1E0C,  in  LO/HI
                     format)
          02AE-02AE+$1E0C: Track data

*/

FLOPPY_CONSTRUCT( g64_dsk_construct )
{
	struct FloppyCallbacks *callbacks;
	struct g64dsk_tag *tag;
	UINT8 header[HEADER_LENGTH];
	UINT64 pos = 0xc;
	int i;

	if (params)
	{
		// create not supported
		return FLOPPY_ERROR_UNSUPPORTED;
	}

	tag = (struct g64dsk_tag *) floppy_create_tag(floppy, sizeof(struct g64dsk_tag));

	if (!tag) return FLOPPY_ERROR_OUTOFMEMORY;

	// read header
	floppy_image_read(floppy, header, 0, HEADER_LENGTH);

	// get version
	tag->version = header[8];
	LOG_FORMATS("G64 version: %u\n", tag->version);

	// get number of tracks
	tag->heads = 1;
	tag->tracks = header[9];
	LOG_FORMATS("G64 tracks: %u\n", tag->tracks);

	// get data offsets
	for (i = 0; i < tag->tracks; i++)
	{
		tag->track_offset[i] = pick_integer_le(header, pos, 4);
		pos += 4;
	}

	// get speed zone information
	for (i = 0; i < tag->tracks; i++)
	{
		tag->speed_zone_offset[i] = pick_integer_le(header, pos, 4);
		pos += 4;

		tag->track_size[i] = g64_get_track_size(floppy, 0, i);

		if (tag->track_offset[i] > 0)
		{
			LOG_FORMATS("G64 track %.1f offset %05x length %04x ", get_dos_track(i), tag->track_offset[i], tag->track_size[i]);

			if (tag->speed_zone_offset[i] < 4) {
				LOG_FORMATS("speed %u\n", tag->speed_zone_offset[i]);
			} else {
				LOG_FORMATS("speed offset %04x\n", tag->speed_zone_offset[i]);
			}
		}
	}

	// set callbacks
	callbacks = floppy_callbacks(floppy);

	callbacks->read_track = g64_read_track;
	callbacks->write_track = g64_write_track;
	callbacks->get_heads_per_disk = g64_get_heads_per_disk;
	callbacks->get_tracks_per_disk = g64_get_tracks_per_disk;
	callbacks->get_track_size = g64_get_track_size;

	return FLOPPY_ERROR_SUCCESS;
}
