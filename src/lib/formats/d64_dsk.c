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

    formats/d64_dsk.c

    Commodore 2040/8050/1541 sector disk image format

*********************************************************************/

#include "emu.h"
#include "formats/d64_dsk.h"

d64_format::d64_format()
{
}

const char *d64_format::name() const
{
	return "d64";
}

const char *d64_format::description() const
{
	return "Commodore 2040/8050/1541 disk image";
}

const char *d64_format::extensions() const
{
	return "d64,d67,d71,d80,d82";
}

const d64_format::format d64_format::formats[] = {
	{ // d67, dos 1, 35 tracks
		floppy_image::FF_525, floppy_image::SSSD, DOS_1, 690, 35, 1, 9, 8
	},
	{ // d64, dos 2, 35 tracks
		floppy_image::FF_525, floppy_image::SSSD, DOS_2, 683, 35, 1, 9, 8
	},
	{ // d64, dos 2, 40 tracks
		floppy_image::FF_525, floppy_image::SSSD, DOS_2, 768, 35, 1, 9, 8
	},
	{ // d64, dos 2, 42 tracks
		floppy_image::FF_525, floppy_image::SSSD, DOS_2, 802, 35, 1, 9, 8
	},
	{ // d71, dos 2, 35 tracks, 2 heads
		floppy_image::FF_525, floppy_image::DSSD, DOS_2, 683, 35, 2, 9, 8
	},
	{ // d80, dos 2.5, 77 tracks
		floppy_image::FF_525, floppy_image::SSQD, DOS_25, 2083, 77, 1, 9, 8
	},
	{ // d82, dos 2.5, 77 tracks, 2 heads
		floppy_image::FF_525, floppy_image::DSQD, DOS_25, 2083, 77, 2, 9, 8
	},
	{}
};

const UINT32 d64_format::dos1_cell_size[] =
{
	4000, // 16MHz/16/4
	3750, // 16MHz/15/4
	3500, // 16MHz/14/4
	3250  // 16MHz/13/4
};

const UINT32 d64_format::dos25_cell_size[] =
{
	2667, // 12MHz/16/2
	2500, // 12MHz/15/2
	2333, // 12MHz/14/2
	2167  // 12MHz/13/2
};

const int d64_format::dos1_sectors_per_track[] =
{
	21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, //  1-17
	20, 20, 20, 20, 20, 20, 20,                                         // 18-24
	18, 18, 18, 18, 18, 18,                                             // 25-30
	17, 17, 17, 17, 17,                                                 // 31-35
	17, 17, 17, 17, 17,                                                 // 36-40
	17, 17                                                              // 41-42
};

const int d64_format::dos2_sectors_per_track[] =
{
	21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, //  1-17
	19, 19, 19, 19, 19, 19, 19,                                         // 18-24
	18, 18, 18, 18, 18, 18,                                             // 25-30
	17, 17, 17, 17, 17,                                                 // 31-35
	17, 17, 17, 17, 17,                                                 // 36-40
	17, 17                                                              // 41-42
};

const int d64_format::dos25_sectors_per_track[] =
{
	29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, //  1-39
	29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
	27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,                         // 40-53
	25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,                                     // 54-64
	23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,                             // 65-77
	23, 23, 23, 23, 23, 23, 23                                                      // 78-84
};

const int d64_format::dos1_speed_zone[] =
{
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, //  1-17
	2, 2, 2, 2, 2, 2, 2,                               // 18-24
	1, 1, 1, 1, 1, 1,                                  // 25-30
	0, 0, 0, 0, 0,                                     // 31-35
	0, 0, 0, 0, 0,                                     // 36-40
	0, 0                                               // 41-42
};

const int d64_format::dos25_speed_zone[] =
{
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, //  1-39
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,                   // 40-53
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                            // 54-64
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                      // 65-77
	0, 0, 0, 0, 0, 0, 0                                         // 78-84
};

int d64_format::find_size(io_generic *io, UINT32 form_factor)
{
	int size = io_generic_size(io);
	for(int i=0; formats[i].sector_count; i++) {
		const format &f = formats[i];
		if(size == f.sector_count*SECTOR_SIZE)
			return i;
		if(size == (f.sector_count*SECTOR_SIZE) + f.sector_count)
			return i;
	}
	return -1;
}

int d64_format::identify(io_generic *io, UINT32 form_factor)
{
	int type = find_size(io, form_factor);

	if(type != -1)
		return 50;
	return 0;
}

int d64_format::get_physical_track(const format &f, int track)
{
	int physical_track = 0;

	switch (f.dos) {
	case DOS_1: physical_track = track*2; break; // skip halftracks
	case DOS_2: physical_track = track*2; break; // skip halftracks
	case DOS_25: physical_track = track; break;
	}

	return physical_track;
}

void d64_format::get_disk_id(const format &f, io_generic *io, UINT8 &id1, UINT8 &id2)
{
	int offset = 0;

	switch (f.dos) {
	case DOS_1: offset = DOS1_DISK_ID_OFFSET; break;
	case DOS_2: offset = DOS1_DISK_ID_OFFSET; break;
	case DOS_25: offset = DOS25_DISK_ID_OFFSET; break;
	}

	UINT8 id[2];
	io_generic_read(io, id, offset, 2);
	id1 = id[0];
	id2 = id[1];
}

UINT32 d64_format::get_cell_size(const format &f, int track)
{
	int cell_size = 0;

	switch (f.dos) {
	case DOS_1: cell_size = dos1_cell_size[dos1_speed_zone[track]]; break;
	case DOS_2: cell_size = dos1_cell_size[dos1_speed_zone[track]]; break;
	case DOS_25: cell_size = dos25_cell_size[dos25_speed_zone[track]]; break;
	}

	return cell_size;
}

int d64_format::get_sectors_per_track(const format &f, int track)
{
	int sector_count = 0;

	switch (f.dos) {
	case DOS_1: sector_count = dos1_sectors_per_track[track]; break;
	case DOS_2: sector_count = dos2_sectors_per_track[track]; break;
	case DOS_25: sector_count = dos25_sectors_per_track[track]; break;
	}

	return sector_count;
}

floppy_image_format_t::desc_e* d64_format::get_sector_desc(const format &f, int &current_size, int track, int sector_count, UINT8 id1, UINT8 id2, int gap_2)
{
	static floppy_image_format_t::desc_e desc[] = {
		/* 00 */ { SECTOR_LOOP_START, 0, sector_count-1 },
		/* 01 */ {   RAWBYTE, 0xff, 5 },
		/* 02 */ {   GCR5, 0x08, 1 },
		/* 03 */ {   CRC, 1 },
		/* 04 */ {   CRC_CBM_START, 1 },
		/* 05 */ {     SECTOR_ID_GCR5 },
		/* 06 */ {     GCR5, track, 1 },
		/* 07 */ {     GCR5, id2, 1 },
		/* 08 */ {     GCR5, id1, 1 },
		/* 09 */ {   CRC_END, 1 },
		/* 10 */ {   GCR5, 0x0f, 2 },
		/* 11 */ {   RAWBYTE, 0x55, f.gap_1 },
		/* 12 */ {   RAWBYTE, 0xff, 5 },
		/* 13 */ {   GCR5, 0x07, 1 },
		/* 14 */ {   CRC_CBM_START, 2 },
		/* 15 */ {     SECTOR_DATA_GCR5, -1 },
		/* 16 */ {   CRC_END, 2 },
		/* 17 */ {   CRC, 2 },
		/* 18 */ {   GCR5, 0x00, 2 },
		/* 19 */ {   RAWBYTE, 0x55, gap_2 },
		/* 20 */ { SECTOR_LOOP_END },
		/* 21 */ { RAWBYTE, 0x55, 0 },
		/* 22 */ { RAWBITS, 0xffff, 0 },
		/* 23 */ { END }
	};

	current_size = 40 + (1+1+4+2)*10 + (f.gap_1)*8 + 40 + (1+SECTOR_SIZE+1+2)*10 + gap_2*8;
	current_size *= sector_count;

	return desc;
}

void d64_format::build_sector_description(const format &f, UINT8 *sectdata, desc_s *sectors, int sector_count, UINT8 *errordata) const
{
	int cur_offset = 0;

	for(int i=0; i<f.sector_count; i++) {
		sectors[i].data = sectdata + cur_offset;
		sectors[i].size = SECTOR_SIZE;
		sectors[i].sector_id = i;
		sectors[i].sector_info = errordata[i];

		cur_offset += sectors[i].size;
	}
}

bool d64_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	int type = find_size(io, form_factor);
	if(type == -1)
		return false;

	const format &f = formats[type];
	int size = io_generic_size(io);
	UINT8 *img;

	if(size == f.sector_count*SECTOR_SIZE) {
		img = global_alloc_array(UINT8, size + f.sector_count);
		memset(&img[size + f.sector_count], ERROR_00, f.sector_count);
	}
	else {
		img = global_alloc_array(UINT8, size);
	}

	io_generic_read(io, img, 0, size);

	floppy_image_format_t::desc_e *desc;
	desc_s sectors[40];
	int track_offset = 0, error_offset = 0;
	
	UINT8 id1 = 0, id2 = 0;
	get_disk_id(f, io, id1, id2);

	for(int head=0; head < f.head_count; head++) {
		for(int track=0; track < f.track_count; track++) {
			int current_size = 0;
			int total_size = 200000000/get_cell_size(f, track);

			int physical_track = get_physical_track(f, track);
			int sector_count = get_sectors_per_track(f, track);
			int track_size = sector_count*SECTOR_SIZE;

			desc = get_sector_desc(f, current_size, track+1, sector_count, id1, id2, f.gap_2);

			int remaining_size = total_size - current_size;
			if(remaining_size < 0)
				throw emu_fatalerror("d64_format: Incorrect track layout, max_size=%d, current_size=%d", total_size, current_size);
			
			// Fixup the end gap
			desc[21].p2 = remaining_size / 8;
			desc[22].p2 = remaining_size & 7;
			desc[22].p1 >>= 8-(remaining_size & 7);

			printf("track %u cursize %u totsize %u phystrack %u secnt %u trksize %u trkofs %u\n", track,current_size,total_size,physical_track,sector_count,track_size,track_offset);

			build_sector_description(f, &img[track_offset], sectors, sector_count, &img[f.sector_count*SECTOR_SIZE + error_offset]);
			generate_track(desc, physical_track, head, sectors, sector_count, total_size, image);
			
			track_offset += track_size;
			error_offset += sector_count;
		}
	}

	image->set_variant(f.variant);

	return true;
}

bool d64_format::save(io_generic *io, floppy_image *image)
{
	return false;
}

bool d64_format::supports_save() const
{
	return false;
}

const floppy_format_type FLOPPY_D64_FORMAT = &floppy_image_format_creator<d64_format>;


// ------ LEGACY -----


/*********************************************************************

    formats/d64_dsk.c

    Floppy format code for Commodore 1541/2040/8050 disk images

*********************************************************************/

/*

    TODO:

    - write to disk
    - disk errors 24, 25, 26, 28, 74
    - variable gaps

*/

#define XTAL_16MHz      16000000
#define XTAL_12MHz      12000000
#include "g64_dsk.h"
#include "flopimg.h"
#include "d64_dsk.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define MAX_HEADS           2
#define MAX_TRACKS          84
#define MAX_ERROR_SECTORS   4166
#define SECTOR_SIZE         256
#define SECTOR_SIZE_GCR     368

#define INVALID_OFFSET      0xbadbad

#define D64_SIZE_35_TRACKS               174848
#define D64_SIZE_35_TRACKS_WITH_ERRORS   175531
#define D64_SIZE_40_TRACKS               196608
#define D64_SIZE_40_TRACKS_WITH_ERRORS   197376
#define D64_SIZE_42_TRACKS               205312
#define D64_SIZE_42_TRACKS_WITH_ERRORS   206114
#define D67_SIZE_35_TRACKS               176640
#define D71_SIZE_70_TRACKS               349696
#define D71_SIZE_70_TRACKS_WITH_ERRORS   351062
#define D80_SIZE_77_TRACKS               533248
#define D80_SIZE_77_TRACKS_WITH_ERRORS   535331
#define D82_SIZE_154_TRACKS             1066496
#define D82_SIZE_154_TRACKS_WITH_ERRORS 1070662

enum
{
	DOS1,
	DOS2,
	DOS25
};

static const char *const DOS_VERSION[] = { "1.0", "2.0", "2.5" };

enum
{
	ERROR_00 = 1,
	ERROR_20,       /* header block not found */
	ERROR_21,       /* no sync character */
	ERROR_22,       /* data block not present */
	ERROR_23,       /* checksum error in data block */
	ERROR_24,       /* write verify (on format) UNIMPLEMENTED */
	ERROR_25,       /* write verify error UNIMPLEMENTED */
	ERROR_26,       /* write protect on UNIMPLEMENTED */
	ERROR_27,       /* checksum error in header block */
	ERROR_28,       /* write error UNIMPLEMENTED */
	ERROR_29,       /* disk ID mismatch */
	ERROR_74,       /* disk not ready (no device 1) UNIMPLEMENTED */
};

static const char *const ERROR_CODE[] = { "00", "00", "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "74" };

static const UINT8 bin_2_gcr[] =
{
	0x0a, 0x0b, 0x12, 0x13, 0x0e, 0x0f, 0x16, 0x17,
	0x09, 0x19, 0x1a, 0x1b, 0x0d, 0x1d, 0x1e, 0x15
};

/* This could be of use if we ever implement saving in .d64 format, to convert back GCR -> d64 */
/*
static const int gcr_2_bin[] =
{
    -1, -1,   -1,   -1,
    -1, -1,   -1,   -1,
    -1, 0x08, 0x00, 0x01,
    -1, 0x0c, 0x04, 0x05,
    -1, -1,   0x02, 0x03,
    -1, 0x0f, 0x06, 0x07,
    -1, 0x09, 0x0a, 0x0b,
    -1, 0x0d, 0x0e, -1
};
*/

static const int DOS1_SECTORS_PER_TRACK[] =
{
	21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
	20, 20, 20, 20, 20, 20, 20,
	18, 18, 18, 18, 18, 18,
	17, 17, 17, 17, 17,
	17, 17, 17, 17, 17,
	17, 17
};

static const int DOS1_SPEED_ZONE[] =
{
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	2, 2, 2, 2, 2, 2, 2,
	1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0
};

static const int DOS2_SECTORS_PER_TRACK[] =
{
	21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
	19, 19, 19, 19, 19, 19, 19,
	18, 18, 18, 18, 18, 18,
	17, 17, 17, 17, 17,
	17, 17, 17, 17, 17,
	17, 17
};

static const int DOS25_SECTORS_PER_TRACK[] =
{
	29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
	29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,     /* 1-39 */
	27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,                         /* 40-53 */
	25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,                                     /* 54-64 */
	23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,                             /* 65-77 */
	23, 23, 23, 23, 23, 23, 23                                                      /* 78-84 */
};

static const int DOS25_SPEED_ZONE[] =
{
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,    /* 1-39 */
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,                   /* 40-53 */
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                            /* 54-64 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                      /* 65-77 */
	0, 0, 0, 0, 0, 0, 0                                         /* 78-84 */
};

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct d64dsk_tag
{
	int dos;                                    /* CBM DOS version */
	int heads;                                  /* number of physical heads */
	int tracks;                                 /* number of physical tracks */
	int dos_tracks;                             /* number of logical tracks */
	int track_offset[MAX_HEADS][MAX_TRACKS];    /* offset within image for each physical track */
	UINT32 speed_zone[MAX_TRACKS];              /* speed zone for each physical track */
	bool has_errors;                            /* flag to check for available error codes */
	UINT8 error[MAX_ERROR_SECTORS];             /* error code for each logical sector */
	int error_offset[MAX_HEADS][MAX_TRACKS];    /* offset within error array for sector 0 of each logical track */

	UINT8 id1, id2;                             /* DOS disk format ID */
};

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE struct d64dsk_tag *get_tag(floppy_image_legacy *floppy)
{
	return (d64dsk_tag *)floppy_tag(floppy);
}

INLINE float get_dos_track(int track)
{
	return ((float)track / 2) + 1;
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    d64_get_heads_per_disk - returns the number
    of heads in the disk image
-------------------------------------------------*/

static int d64_get_heads_per_disk(floppy_image_legacy *floppy)
{
	return get_tag(floppy)->heads;
}

/*-------------------------------------------------
    d64_get_tracks_per_disk - returns the number
    of DOS tracks in the disk image
-------------------------------------------------*/

static int d64_get_tracks_per_disk(floppy_image_legacy *floppy)
{
	return get_tag(floppy)->tracks;
}

/*-------------------------------------------------
    d64_get_sectors_per_track - returns the number
    of sectors per given track
-------------------------------------------------*/

static int d64_get_sectors_per_track(floppy_image_legacy *floppy, int head, int track)
{
	int sectors_per_track = 0;

	switch (get_tag(floppy)->dos)
	{
	case DOS1:  sectors_per_track = DOS1_SECTORS_PER_TRACK[track / 2]; break;
	case DOS2:  sectors_per_track = DOS2_SECTORS_PER_TRACK[track / 2]; break;
	case DOS25: sectors_per_track = DOS25_SECTORS_PER_TRACK[track];    break;
	}

	return sectors_per_track;
}

/*-------------------------------------------------
    get_track_offset - returns the offset within
    the disk image for a given track
-------------------------------------------------*/

static floperr_t get_track_offset(floppy_image_legacy *floppy, int head, int track, UINT64 *offset)
{
	struct d64dsk_tag *tag = get_tag(floppy);
	UINT64 offs = 0;

	if ((track < 0) || (track >= tag->tracks))
		return FLOPPY_ERROR_SEEKERROR;

	offs = tag->track_offset[head][track];

	if (offset)
		*offset = offs;

	return FLOPPY_ERROR_SUCCESS;
}

/*-------------------------------------------------
    d64_get_track_size - returns the track size
-------------------------------------------------*/

static UINT32 d64_get_track_size(floppy_image_legacy *floppy, int head, int track)
{
	struct d64dsk_tag *tag = get_tag(floppy);

	if (tag->track_offset[head][track] == INVALID_OFFSET)
		return 0;

	/* determine number of sectors per track */
	int sectors_per_track = d64_get_sectors_per_track(floppy, head, track);

	/* allocate temporary GCR track data buffer */
	UINT32 track_length = sectors_per_track * SECTOR_SIZE_GCR;

	return track_length;
}

/*-------------------------------------------------
    get_sector_error_code - returns the error
    code for the given sector
-------------------------------------------------*/

static int get_sector_error_code(floppy_image_legacy *floppy, int head, int dos_track, int sector)
{
	struct d64dsk_tag *tag = get_tag(floppy);

	if (!tag->has_errors)
		return ERROR_00;

	int sector_error = tag->error[tag->error_offset[head][dos_track] + sector]; // TODO index out of bounds!!!

	if (sector_error != ERROR_00)
	{
		LOG_FORMATS("D64 error %s head %d track %d sector %d\n", ERROR_CODE[sector_error], head, dos_track, sector);
	}

	return sector_error;
}

/*-------------------------------------------------
    gcr_double_2_gcr - GCR decodes given data
-------------------------------------------------*/

/* gcr_double_2_gcr takes 4 bytes (a, b, c, d) and shuffles their nibbles to obtain 5 bytes in dest */
/* The result is basically res = (enc(a) << 15) | (enc(b) << 10) | (enc(c) << 5) | enc(d)
 * with res being 5 bytes long and enc(x) being the GCR encode of x.
 * In fact, we store the result as five separate bytes in the dest argument
 */

/*

 Commodore GCR format

Original    Encoded
4 bits      5 bits

0000    ->  01010 = 0x0a
0001    ->  01011 = 0x0b
0010    ->  10010 = 0x12
0011    ->  10011 = 0x13
0100    ->  01110 = 0x0e
0101    ->  01111 = 0x0f
0110    ->  10110 = 0x16
0111    ->  10111 = 0x17
1000    ->  01001 = 0x09
1001    ->  11001 = 0x19
1010    ->  11010 = 0x1a
1011    ->  11011 = 0x1b
1100    ->  01101 = 0x0d
1101    ->  11101 = 0x1d
1110    ->  11110 = 0x1e
1111    ->  10101 = 0x15

We use the encoded values in bytes because we use them to encode
groups of 4 bytes into groups of 5 bytes, below.

*/

static void gcr_double_2_gcr(UINT8 a, UINT8 b, UINT8 c, UINT8 d, UINT8 *dest)
{
	UINT8 gcr[8];

	/* Encode each nibble to 5 bits */
	gcr[0] = bin_2_gcr[a >> 4];
	gcr[1] = bin_2_gcr[a & 0x0f];
	gcr[2] = bin_2_gcr[b >> 4];
	gcr[3] = bin_2_gcr[b & 0x0f];
	gcr[4] = bin_2_gcr[c >> 4];
	gcr[5] = bin_2_gcr[c & 0x0f];
	gcr[6] = bin_2_gcr[d >> 4];
	gcr[7] = bin_2_gcr[d & 0x0f];

	/* Re-order the encoded data to only keep the 5 lower bits of each byte */
	dest[0] = (gcr[0] << 3) | (gcr[1] >> 2);
	dest[1] = (gcr[1] << 6) | (gcr[2] << 1) | (gcr[3] >> 4);
	dest[2] = (gcr[3] << 4) | (gcr[4] >> 1);
	dest[3] = (gcr[4] << 7) | (gcr[5] << 2) | (gcr[6] >> 3);
	dest[4] = (gcr[6] << 5) | gcr[7];
}

/*-------------------------------------------------
    d64_read_track - reads a full track from the
    disk image
-------------------------------------------------*/

/*

    Commodore sector format

    SYNC                        FF * 5
    08
    CHECKSUM                    sector ^ track ^ id2 ^ id1
    SECTOR                      0..20 (2040), 0..28 (8050)
    TRACK                       1..35 (2040), 1..77 (8050), 1..70 (1571)
    ID2
    ID1
    GAP 1                       55 * 9 (2040), 55 * 8 (1541)

    SYNC                        FF * 5
    07
    NEXT TRACK
    NEXT SECTOR
    254 BYTES OF DATA
    CHECKSUM
    GAP 2                       55 * 8..19

*/

static floperr_t d64_read_track(floppy_image_legacy *floppy, int head, int track, UINT64 offset, void *buffer, size_t buflen)
{
	struct d64dsk_tag *tag = get_tag(floppy);
	floperr_t err;
	UINT64 track_offset;

	/* get track offset */
	err = get_track_offset(floppy, head, track, &track_offset);

	if (err)
		return err;

	if (track_offset != INVALID_OFFSET)
	{
		UINT8 id1 = tag->id1;
		UINT8 id2 = tag->id2;
		int sectors_per_track;
		UINT16 d64_track_size;
		UINT8 *d64_track_data;
		UINT16 gcr_track_size;
		UINT8 *gcr_track_data;
		UINT64 gcr_pos = 0;

		/* determine logical track number */
		int dos_track = get_dos_track(track);

		if (tag->dos == DOS25)
		{
			dos_track = track + 1;
		}

		/* logical track numbers continue on the flip side */
		if (head == 1) dos_track += tag->dos_tracks;

		/* determine number of sectors per track */
		sectors_per_track = d64_get_sectors_per_track(floppy, head, track);

		/* allocate D64 track data buffer */
		d64_track_size = sectors_per_track * SECTOR_SIZE;
		d64_track_data = (UINT8 *)alloca(d64_track_size);

		/* allocate temporary GCR track data buffer */
		gcr_track_size = sectors_per_track * SECTOR_SIZE_GCR;
		gcr_track_data = (UINT8 *)alloca(gcr_track_size);

		if (buflen < gcr_track_size) { printf("D64 track buffer too small: %u!", (UINT32)buflen); exit(-1); }

		/* read D64 track data */
		floppy_image_read(floppy, d64_track_data, track_offset, d64_track_size);

		/* GCR encode D64 sector data */
		for (int sector = 0; sector < sectors_per_track; sector++)
		{
			// here we convert the sector data to gcr directly!
			// IMPORTANT: errors in reading sectors can modify e.g. header info $01 & $05
			int sector_error = get_sector_error_code(floppy, head, track, sector);

			/* first we set the position at which sector data starts in the image */
			UINT64 d64_pos = sector * SECTOR_SIZE;
			int i;

			/*
			    1. Header sync       FF FF FF FF FF (40 'on' bits, not GCR encoded)
			    2. Header info       52 54 B5 29 4B 7A 5E 95 55 55 (10 GCR bytes)
			    3. Header gap        55 55 55 55 55 55 55 55 55 (9 bytes, never read)
			    4. Data sync         FF FF FF FF FF (40 'on' bits, not GCR encoded)
			    5. Data block        55...4A (325 GCR bytes)
			    6. Inter-sector gap  55 55 55 55...55 55 (4 to 19 bytes, never read)
			*/

			if (sector_error == ERROR_29)
				id1 ^= 0xff;

			/* Header sync */
			if (sector_error != ERROR_21)
			{
				for (i = 0; i < 5; i++)
					gcr_track_data[gcr_pos + i] = 0xff;
				gcr_pos += 5;
			}

			/* Header info */
			/* These are 8 bytes unencoded, which become 10 bytes encoded */
			// $00 - header block ID ($08)                      // this byte can be modified by error code 20 -> 0xff
			// $01 - header block checksum (EOR of $02-$05)     // this byte can be modified by error code 27 -> ^ 0xff
			// $02 - Sector# of data block
			// $03 - Track# of data block
			UINT8 header_block_id = (sector_error == ERROR_20) ? 0xff : 0x08;
			UINT8 header_block_checksum = sector ^ dos_track ^ id2 ^ id1;

			if (sector_error == ERROR_27)
				header_block_checksum ^= 0xff;

			gcr_double_2_gcr(header_block_id, header_block_checksum, sector, dos_track, gcr_track_data + gcr_pos);
			gcr_pos += 5;

			// $04 - Format ID byte #2
			// $05 - Format ID byte #1
			// $06 - $0F ("off" byte)
			// $07 - $0F ("off" byte)
			gcr_double_2_gcr(id2, id1, 0x0f, 0x0f, gcr_track_data + gcr_pos);
			gcr_pos += 5;

			/* Header gap */
			for (i = 0; i < 9; i++)
				gcr_track_data[gcr_pos + i] = 0x55;
			gcr_pos += 9;

			/* Data sync */
			for (i = 0; i < 5; i++)
				gcr_track_data[gcr_pos + i] = 0xff;
			gcr_pos += 5;

			/* Data block */
			UINT8 data_block_id = (sector_error == ERROR_22) ? 0xff : 0x07;

			// we first need to calculate the checksum of the 256 bytes of the sector
			UINT8 sector_checksum = d64_track_data[d64_pos];
			for (i = 1; i < 256; i++)
				sector_checksum ^= d64_track_data[d64_pos + i];

			if (sector_error == ERROR_23)
				sector_checksum ^= 0xff;

			/*
			    $00      - data block ID ($07)
			    $01-100  - 256 bytes sector data
			    $101     - data block checksum (EOR of $01-100)
			    $102-103 - $00 ("off" bytes, to make the sector size a multiple of 5)
			*/
			gcr_double_2_gcr(data_block_id, d64_track_data[d64_pos], d64_track_data[d64_pos + 1], d64_track_data[d64_pos + 2], gcr_track_data + gcr_pos);
			gcr_pos += 5;

			for (i = 1; i < 64; i++)
			{
				gcr_double_2_gcr(d64_track_data[d64_pos + 4 * i - 1], d64_track_data[d64_pos + 4 * i],
									d64_track_data[d64_pos + 4 * i + 1], d64_track_data[d64_pos + 4 * i + 2], gcr_track_data + gcr_pos);
				gcr_pos += 5;
			}

			gcr_double_2_gcr(d64_track_data[d64_pos + 255], sector_checksum, 0x00, 0x00, gcr_track_data + gcr_pos);
			gcr_pos += 5;

			/* Inter-sector gap */
			// "In tests that the author conducted on a real 1541 disk, gap sizes of 8 to 19 bytes were seen."
			// Here we put 14 as an average...
			for (i = 0; i < 14; i++)
				gcr_track_data[gcr_pos + i] = 0x55;
			gcr_pos += 14;
		}

		/* copy GCR track data to buffer */
		memcpy((UINT8*)buffer, gcr_track_data, gcr_track_size);

		// create a speed block with the same speed zone for the whole track
		UINT8 speed = tag->speed_zone[track] & 0x03;
		UINT8 speed_byte = (speed << 6) | (speed << 4) | (speed << 2) | speed;

		memset(((UINT8*)buffer) + gcr_track_size, speed_byte, G64_SPEED_BLOCK_SIZE);

		LOG_FORMATS("D64 track %.1f length %u\n", get_dos_track(track), gcr_track_size);
	}
	else    /* half tracks */
	{
		/* set track length to 0 */
		memset(buffer, 0, buflen);

		LOG_FORMATS("D64 track %.1f length %u\n", get_dos_track(track), 0);
	}

	return FLOPPY_ERROR_SUCCESS;
}

/*-------------------------------------------------
    d64_write_track - writes a full track to the
    disk image
-------------------------------------------------*/

static floperr_t d64_write_track(floppy_image_legacy *floppy, int head, int track, UINT64 offset, const void *buffer, size_t buflen)
{
	return FLOPPY_ERROR_UNSUPPORTED;
}

/*-------------------------------------------------
    d64_identify - identifies the disk image
-------------------------------------------------*/

static void d64_identify(floppy_image_legacy *floppy, int *dos, int *heads, int *tracks, bool *has_errors)
{
	switch (floppy_image_size(floppy))
	{
	/* 2040/3040 */
	case D67_SIZE_35_TRACKS:                *dos = DOS1;  *heads = 1; *tracks = 35; *has_errors = false; break;

	/* 4040/2031/1541/1551 */
	case D64_SIZE_35_TRACKS:                *dos = DOS2;  *heads = 1; *tracks = 35; *has_errors = false; break;
	case D64_SIZE_35_TRACKS_WITH_ERRORS:    *dos = DOS2;  *heads = 1; *tracks = 35; *has_errors = true;  break;
	case D64_SIZE_40_TRACKS:                *dos = DOS2;  *heads = 1; *tracks = 40; *has_errors = false; break;
	case D64_SIZE_40_TRACKS_WITH_ERRORS:    *dos = DOS2;  *heads = 1; *tracks = 40; *has_errors = true;  break;
	case D64_SIZE_42_TRACKS:                *dos = DOS2;  *heads = 1; *tracks = 42; *has_errors = false; break;
	case D64_SIZE_42_TRACKS_WITH_ERRORS:    *dos = DOS2;  *heads = 1; *tracks = 42; *has_errors = true;  break;

	/* 1571 */
	case D71_SIZE_70_TRACKS:                *dos = DOS2;  *heads = 2; *tracks = 35; *has_errors = false; break;
	case D71_SIZE_70_TRACKS_WITH_ERRORS:    *dos = DOS2;  *heads = 2; *tracks = 35; *has_errors = true;  break;

	/* 8050 */
	case D80_SIZE_77_TRACKS:                *dos = DOS25; *heads = 1; *tracks = 77; *has_errors = false; break;
	case D80_SIZE_77_TRACKS_WITH_ERRORS:    *dos = DOS25; *heads = 1; *tracks = 77; *has_errors = true;  break;

	/* 8250/SFD1001 */
	case D82_SIZE_154_TRACKS:               *dos = DOS25; *heads = 2; *tracks = 77; *has_errors = false; break;
	case D82_SIZE_154_TRACKS_WITH_ERRORS:   *dos = DOS25; *heads = 2; *tracks = 77; *has_errors = true;  break;
	}
}

/*-------------------------------------------------
    FLOPPY_IDENTIFY( d64_dsk_identify )
-------------------------------------------------*/

FLOPPY_IDENTIFY( d64_dsk_identify )
{
	int dos = 0, heads, tracks;
	bool has_errors = false;

	*vote = 0;

	d64_identify(floppy, &dos, &heads, &tracks, &has_errors);

	if (dos == DOS2 && heads == 1)
	{
		*vote = 100;
	}

	return FLOPPY_ERROR_SUCCESS;
}

/*-------------------------------------------------
    FLOPPY_IDENTIFY( d67_dsk_identify )
-------------------------------------------------*/

FLOPPY_IDENTIFY( d67_dsk_identify )
{
	*vote = 0;

	if (floppy_image_size(floppy) == D67_SIZE_35_TRACKS)
	{
		*vote = 100;
	}

	return FLOPPY_ERROR_SUCCESS;
}

/*-------------------------------------------------
    FLOPPY_IDENTIFY( d71_dsk_identify )
-------------------------------------------------*/

FLOPPY_IDENTIFY( d71_dsk_identify )
{
	int heads = 0, tracks = 0, dos = -1;
	bool has_errors = false;

	*vote = 0;

	d64_identify(floppy, &dos, &heads, &tracks, &has_errors);

	if (dos == DOS2 && heads == 2)
	{
		*vote = 100;
	}

	return FLOPPY_ERROR_SUCCESS;
}

/*-------------------------------------------------
    FLOPPY_IDENTIFY( d80_dsk_identify )
-------------------------------------------------*/

FLOPPY_IDENTIFY( d80_dsk_identify )
{
	int heads = 0, tracks = 0, dos = -1;
	bool has_errors = false;
	*vote = 0;

	d64_identify(floppy, &dos, &heads, &tracks, &has_errors);

	if (dos == DOS25 && heads == 1)
	{
		*vote = 100;
	}

	return FLOPPY_ERROR_SUCCESS;
}

/*-------------------------------------------------
    FLOPPY_IDENTIFY( d82_dsk_identify )
-------------------------------------------------*/

FLOPPY_IDENTIFY( d82_dsk_identify )
{
	int heads = 0, tracks = 0, dos = -1;
	bool has_errors = false;
	*vote = 0;

	d64_identify(floppy, &dos, &heads, &tracks, &has_errors);

	if (dos == DOS25 && heads == 2)
	{
		*vote = 100;
	}

	return FLOPPY_ERROR_SUCCESS;
}

/*-------------------------------------------------
    FLOPPY_CONSTRUCT( d64_dsk_construct )
-------------------------------------------------*/

FLOPPY_CONSTRUCT( d64_dsk_construct )
{
	struct FloppyCallbacks *callbacks;
	struct d64dsk_tag *tag;
	UINT8 id[2];

	int track_offset = 0;
	int head, track;

	int heads = 0, dos_tracks = 0, dos = 0;
	bool has_errors = 0;
	int errors_size = 0;

	if (params)
	{
		/* create not supported */
		return FLOPPY_ERROR_UNSUPPORTED;
	}

	tag = (struct d64dsk_tag *) floppy_create_tag(floppy, sizeof(struct d64dsk_tag));

	if (!tag) return FLOPPY_ERROR_OUTOFMEMORY;

	/* identify image type */
	d64_identify(floppy, &dos, &heads, &dos_tracks, &has_errors);

	tag->dos = dos;
	tag->heads = heads;
	tag->tracks = MAX_TRACKS;
	tag->dos_tracks = dos_tracks;
	tag->has_errors = has_errors;

	LOG_FORMATS("D64 size: %04x\n", (UINT32)floppy_image_size(floppy));
	LOG_FORMATS("D64 heads: %d\n", heads);
	LOG_FORMATS("D64 tracks: %d\n", dos_tracks);
	LOG_FORMATS("D64 DOS version: %s\n", DOS_VERSION[dos]);
	LOG_FORMATS("D64 error codes: %s\n", has_errors ? "yes" : "no");

	/* clear track data offsets */
	for (head = 0; head < MAX_HEADS; head++)
	{
		for (track = 0; track < MAX_TRACKS; track++)
		{
			tag->track_offset[head][track] = INVALID_OFFSET;
		}
	}

	/* determine track data offsets */
	for (head = 0; head < heads; head++)
	{
		for (track = 0; track < tag->tracks; track++)
		{
			if (dos == DOS25)
			{
				if (track >= dos_tracks)
				{
					/* track out of range */
					tag->track_offset[head][track] = INVALID_OFFSET;
				}
				else
				{
					tag->track_offset[head][track] = track_offset;
					tag->error_offset[head][track] = errors_size;
					tag->speed_zone[track] = DOS25_SPEED_ZONE[track];

					track_offset += DOS25_SECTORS_PER_TRACK[track] * SECTOR_SIZE;
					/* also store an error entry for each sector */
					errors_size += DOS25_SECTORS_PER_TRACK[track];

					LOG_FORMATS("D64 head %d track %d offset %05x", head, track + 1, tag->track_offset[head][track]);
					if (has_errors) LOG_FORMATS(" errors %05x", tag->error_offset[head][track]);
					LOG_FORMATS(" speed %u\n", tag->speed_zone[track]);
				}
			}
			else
			{
				if ((track % 2) || ((track / 2) >= dos_tracks))
				{
					/* half track or out of range */
					tag->track_offset[head][track] = INVALID_OFFSET;
				}
				else
				{
					/* full track */
					tag->track_offset[head][track] = track_offset;
					tag->error_offset[head][track] = errors_size;
					tag->speed_zone[track] = DOS1_SPEED_ZONE[track / 2];

					if (dos == DOS1)
					{
						track_offset += DOS1_SECTORS_PER_TRACK[track / 2] * SECTOR_SIZE;
						/* also store an error entry for each sector */
						errors_size += DOS1_SECTORS_PER_TRACK[track / 2];
					}
					else
					{
						track_offset += DOS2_SECTORS_PER_TRACK[track / 2] * SECTOR_SIZE;
						/* also store an error entry for each sector */
						errors_size += DOS2_SECTORS_PER_TRACK[track / 2];
					}

					LOG_FORMATS("D64 head %d track %.1f offset %05x", head, get_dos_track(track), tag->track_offset[head][track]);
					if (has_errors) LOG_FORMATS(" errors %05x", tag->error_offset[head][track]);
					LOG_FORMATS(" speed %u\n", tag->speed_zone[track]);
				}
			}
		}
	}

	/* read format ID from directory */
	/*
	id1, id2 are the same for extended d64 (i.e. with error tables), for d67 and for d71

	for d81 they are at track 40 bytes 0x17 & 0x18
	for d80 & d82 they are at track 39 bytes 0x18 & 0x19
	*/
	if (dos == DOS25)
		floppy_image_read(floppy, id, tag->track_offset[0][38] + 0x18, 2);
	else
		floppy_image_read(floppy, id, tag->track_offset[0][34] + 0xa2, 2);

	tag->id1 = id[0];
	tag->id2 = id[1];

	LOG_FORMATS("D64 format ID: %02x%02x\n", id[0], id[1]);

	/* read errors */
	if (tag->has_errors)
	{
		LOG_FORMATS("D64 error blocks: %d %d\n", errors_size, track_offset);
		floppy_image_read(floppy, tag->error, track_offset, errors_size);
	}
	else
	{
		memset(tag->error, ERROR_00, MAX_ERROR_SECTORS);
	}

	/* set callbacks */
	callbacks = floppy_callbacks(floppy);

	callbacks->read_track = d64_read_track;
	callbacks->write_track = d64_write_track;
	callbacks->get_heads_per_disk = d64_get_heads_per_disk;
	callbacks->get_tracks_per_disk = d64_get_tracks_per_disk;
	callbacks->get_track_size = d64_get_track_size;

	return FLOPPY_ERROR_SUCCESS;
}
