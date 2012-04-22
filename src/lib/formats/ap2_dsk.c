/*********************************************************************

    ap2_dsk.c

    Apple II disk images

*********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ap2_dsk.h"
#include "basicdsk.h"


#define APPLE2_IMAGE_DO		0
#define APPLE2_IMAGE_PO		1
#define APPLE2_IMAGE_NIB	2


/* used in for all Apple II images */
static UINT32 apple2_get_track_size(floppy_image_legacy *floppy, int head, int track);
static int disk_decode_nib(UINT8 *data, const UINT8 *nibble, int *volume, int *track, int *sector);
static void disk_encode_nib(UINT8 *nibble, const UINT8 *data, int volume, int track, int sector);

/* used in DOS/ProDOS order images */
static int apple2_do_translate_sector(floppy_image_legacy *floppy, int sector);
static int apple2_po_translate_sector(floppy_image_legacy *floppy, int sector);
static floperr_t apple2_dsk_read_track(floppy_image_legacy *floppy, int head, int track, UINT64 offset, void *buffer, size_t buflen);
static floperr_t apple2_dsk_write_track(floppy_image_legacy *floppy, int head, int track, UINT64 offset, const void *buffer, size_t buflen);

/* used in nibble order images */
static floperr_t apple2_nib_read_track(floppy_image_legacy *floppy, int head, int track, UINT64 offset, void *buffer, size_t buflen);
static floperr_t apple2_nib_write_track(floppy_image_legacy *floppy, int head, int track, UINT64 offset, const void *buffer, size_t buflen);
static floperr_t apple2_nib_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen);
static floperr_t apple2_nib_write_sector(floppy_image_legacy *floppy, int head, int track, int sector, const void *buffer, size_t buflen, int ddam);
static floperr_t apple2_nib_get_sector_length(floppy_image_legacy *floppy, int head, int track, int sector, UINT32 *sector_length);


static const UINT8 translate6[0x40] =
{
	0x96, 0x97, 0x9a, 0x9b, 0x9d, 0x9e, 0x9f, 0xa6,
	0xa7, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb2, 0xb3,
	0xb4, 0xb5, 0xb6, 0xb7, 0xb9, 0xba, 0xbb, 0xbc,
	0xbd, 0xbe, 0xbf, 0xcb, 0xcd, 0xce, 0xcf, 0xd3,
	0xd6, 0xd7, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde,
	0xdf, 0xe5, 0xe6, 0xe7, 0xe9, 0xea, 0xeb, 0xec,
	0xed, 0xee, 0xef, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6,
	0xf7, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};



/* -----------------------------------------------------------------------
 * Utility code
 * ----------------------------------------------------------------------- */

static const UINT8 *get_untranslate6_map(void)
{
	static UINT8 map[256];
	static int map_inited = 0;
	UINT8 i;

	if (!map_inited)
	{
		memset(map, 0xff, sizeof(map));
		for (i = 0; i < ARRAY_LENGTH(translate6); i++)
			map[translate6[i]] = i;
		map_inited = 1;
	}
	return map;
}



/* -----------------------------------------------------------------------
 * Core constructor
 * ----------------------------------------------------------------------- */


static floperr_t apple2_general_construct(floppy_image_legacy *floppy, int floppy_type)
{
	floperr_t err;
	struct basicdsk_geometry geometry;
	struct FloppyCallbacks *format;

	format = floppy_callbacks(floppy);

	switch(floppy_type) {
	case APPLE2_IMAGE_DO:
	case APPLE2_IMAGE_PO:
		memset(&geometry, 0, sizeof(geometry));
		geometry.heads = 1;
		geometry.tracks = APPLE2_TRACK_COUNT;
		geometry.sectors = APPLE2_SECTOR_COUNT;
		geometry.sector_length = APPLE2_SECTOR_SIZE;
		geometry.translate_sector = (floppy_type == APPLE2_IMAGE_DO) ? apple2_do_translate_sector : apple2_po_translate_sector;

		err = basicdsk_construct(floppy, &geometry);
		if (err)
			return err;

		format->read_track = apple2_dsk_read_track;
		format->write_track = apple2_dsk_write_track;
		break;

	case APPLE2_IMAGE_NIB:
		format->read_track = apple2_nib_read_track;
		format->write_track = apple2_nib_write_track;
		format->read_sector = apple2_nib_read_sector;
		format->write_sector = apple2_nib_write_sector;
		format->get_sector_length = apple2_nib_get_sector_length;
		break;

	default:
		assert(0);
		return FLOPPY_ERROR_INTERNAL;
	}

	format->get_track_size = apple2_get_track_size;

	return FLOPPY_ERROR_SUCCESS;
}



/* -----------------------------------------------------------------------
 * DOS order and ProDOS order code
 * ----------------------------------------------------------------------- */

static FLOPPY_IDENTIFY(apple2_dsk_identify)
{
	UINT64 size;
	UINT64 expected_size;

	size = floppy_image_size(floppy);
	expected_size = APPLE2_TRACK_COUNT * APPLE2_SECTOR_COUNT * APPLE2_SECTOR_SIZE;

	if (size == expected_size)
		*vote = 100;
	else if ((size > expected_size) && ((size - expected_size) < 8))
		*vote = 90;		/* tolerate images with up to eight fewer/extra bytes (bug #638) */
	else if ((size < expected_size) && ((expected_size - size) < 8))
		*vote = 90;		/* tolerate images with up to eight fewer/extra bytes (bug #638) */
	else
		*vote = 0;
	return FLOPPY_ERROR_SUCCESS;
}



static int apple2_do_translate_sector(floppy_image_legacy *floppy, int sector)
{
	static const UINT8 skewing[] =
	{
		/* DOS order (*.do) */
		0x00, 0x07, 0x0E, 0x06, 0x0D, 0x05, 0x0C, 0x04,
		0x0B, 0x03, 0x0A, 0x02, 0x09, 0x01, 0x08, 0x0F
	};
	return skewing[sector];
}



static int apple2_po_translate_sector(floppy_image_legacy *floppy, int sector)
{
	static const UINT8 skewing[] =
	{
		/* ProDOS order (*.po) */
		0x00, 0x08, 0x01, 0x09, 0x02, 0x0A, 0x03, 0x0B,
		0x04, 0x0C, 0x05, 0x0D, 0x06, 0x0E, 0x07, 0x0F
	};
	return skewing[sector];
}



static FLOPPY_CONSTRUCT(apple2_do_construct)
{
	return apple2_general_construct(floppy, APPLE2_IMAGE_DO);
}



static FLOPPY_CONSTRUCT(apple2_po_construct)
{
	return apple2_general_construct(floppy, APPLE2_IMAGE_PO);
}



static floperr_t apple2_dsk_read_track(floppy_image_legacy *floppy, int head, int track, UINT64 offset, void *buffer, size_t buflen)
{
	UINT8 sector_buffer[APPLE2_SECTOR_SIZE];
	int sector;
	UINT8 *nibble;

	if (buflen < APPLE2_NIBBLE_SIZE*APPLE2_SECTOR_COUNT)
		return FLOPPY_ERROR_INTERNAL;
	if (offset != 0)
		return FLOPPY_ERROR_UNSUPPORTED;

	memset(buffer, 0, buflen);

	for (sector = 0; sector < APPLE2_SECTOR_COUNT; sector++)
	{
		nibble = (UINT8 *)buffer;
		nibble += sector * APPLE2_SMALL_NIBBLE_SIZE;

		floppy_read_sector(floppy, head, track, sector, 0, sector_buffer, sizeof(sector_buffer));
		disk_encode_nib(nibble, sector_buffer, 254, track, sector);
	}
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t apple2_dsk_write_track(floppy_image_legacy *floppy, int head, int track, UINT64 offset, const void *buffer, size_t buflen)
{
	int sector;
	UINT8 sector_buffer[APPLE2_SECTOR_SIZE];
	const UINT8 *nibble;

	if (offset != 0)
		return FLOPPY_ERROR_UNSUPPORTED;

	for (sector = 0; sector < APPLE2_SECTOR_COUNT; sector++)
	{
		nibble = (UINT8 *)buffer;
		nibble += sector * APPLE2_SMALL_NIBBLE_SIZE;

		disk_decode_nib(sector_buffer, nibble, NULL, NULL, NULL);
		floppy_write_sector(floppy, head, track, sector, 0, sector_buffer, sizeof(sector_buffer), 0);
	}

	return FLOPPY_ERROR_SUCCESS;
}



/* -----------------------------------------------------------------------
 * Nibble order code
 * ----------------------------------------------------------------------- */

static FLOPPY_IDENTIFY(apple2_nib_identify)
{
	UINT64 size;
	size = floppy_image_size(floppy);
	*vote = (size == APPLE2_TRACK_COUNT * APPLE2_SECTOR_COUNT * APPLE2_NIBBLE_SIZE) ? 100 : 0;
	return FLOPPY_ERROR_SUCCESS;
}



static FLOPPY_CONSTRUCT(apple2_nib_construct)
{
	return apple2_general_construct(floppy, APPLE2_IMAGE_NIB);
}



static floperr_t apple2_nib_read_track(floppy_image_legacy *floppy, int head, int track, UINT64 offset, void *buffer, size_t buflen)
{
	if ((head != 0) || (track < 0) || (track >= APPLE2_TRACK_COUNT))
		return FLOPPY_ERROR_SEEKERROR;
	if (offset != 0)
		return FLOPPY_ERROR_UNSUPPORTED;
	floppy_image_read(floppy, buffer, track * APPLE2_NIBBLE_SIZE * APPLE2_SECTOR_COUNT, buflen);
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t apple2_nib_write_track(floppy_image_legacy *floppy, int head, int track, UINT64 offset, const void *buffer, size_t buflen)
{
	if ((head != 0) || (track < 0) || (track >= APPLE2_TRACK_COUNT))
		return FLOPPY_ERROR_SEEKERROR;
	if (offset != 0)
		return FLOPPY_ERROR_UNSUPPORTED;
	floppy_image_write(floppy, buffer, track * APPLE2_NIBBLE_SIZE * APPLE2_SECTOR_COUNT, buflen);
	return FLOPPY_ERROR_SUCCESS;
}



/* -----------------------------------------------------------------------
 * Track conversion
 * ----------------------------------------------------------------------- */

static int decode_nibbyte(UINT8 *nibint, const UINT8 *nibdata)
{
	if ((nibdata[0] & 0xAA) != 0xAA)
		return 1;
	if ((nibdata[1] & 0xAA) != 0xAA)
		return 1;

	*nibint =  (nibdata[0] & ~0xAA) << 1;
	*nibint |= (nibdata[1] & ~0xAA) << 0;
	return 0;
}



static int disk_decode_nib(UINT8 *data, const UINT8 *nibble, int *volume, int *track, int *sector)
{
	UINT8 read_volume;
	UINT8 read_track;
	UINT8 read_sector;
	UINT8 read_checksum;
	int i;
	UINT8 b, xorvalue, newvalue;

	const UINT8 *untranslate6 = get_untranslate6_map();

	/* pick apart the volume/track/sector info and checksum */
	if (decode_nibbyte(&read_volume, &nibble[10]))
		return 1;
	if (decode_nibbyte(&read_track, &nibble[12]))
		return 1;
	if (decode_nibbyte(&read_sector, &nibble[14]))
		return 1;
	if (decode_nibbyte(&read_checksum, &nibble[16]))
		return 1;
	if (read_checksum != (read_volume ^ read_track ^ read_sector))
		return 1;

	/* decode the nibble core */
	xorvalue = 0;
	for (i = 0; i < 342; i++)
	{
		b = untranslate6[nibble[i+28]];
		if (b == 0xff)
			return 1;
		newvalue = b ^ xorvalue;

		if (i >= 0x56)
		{
			/* 6 bit */
			data[i - 0x56] |= (newvalue << 2);
		}
		else
		{
			/* 3 * 2 bit */
			data[i + 0x00] = ((newvalue >> 1) & 0x01) | ((newvalue << 1) & 0x02);
			data[i + 0x56] = ((newvalue >> 3) & 0x01) | ((newvalue >> 1) & 0x02);
			if (i + 0xAC < APPLE2_SECTOR_SIZE)
				data[i + 0xAC] = ((newvalue >> 5) & 0x01) | ((newvalue >> 3) & 0x02);
		}
		xorvalue = newvalue;
	}

	/* success; write out values if pointers not null */
	if (volume)
		*volume = read_volume;
	if (track)
		*track = read_track;
	if (sector)
		*sector = read_sector;
	return 0;
}



static floperr_t apple2_nib_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	floperr_t err;
	const UINT8 *nibble;
	UINT8 *track_data;
	void *track_data_v;

	if ((sector < 0) || (sector >= APPLE2_SECTOR_COUNT))
		return FLOPPY_ERROR_SEEKERROR;
	if (buflen != APPLE2_SECTOR_SIZE)
		return FLOPPY_ERROR_INTERNAL;

	err = floppy_load_track(floppy, head, track, FALSE, &track_data_v, NULL);
	if (err)
		return err;
	track_data = (UINT8 *) track_data_v;

	nibble = track_data + (sector * APPLE2_NIBBLE_SIZE);

	if (disk_decode_nib((UINT8 *)buffer, nibble, NULL, NULL, NULL))
		return FLOPPY_ERROR_INVALIDIMAGE;

	return FLOPPY_ERROR_SUCCESS;
}



static void disk_encode_nib(UINT8 *nibble, const UINT8 *data, int volume, int track, int sector)
{
	int checksum, oldvalue, xorvalue, i;

	/* setup header values */
	checksum = volume ^ track ^ sector;

	memset(nibble, 0xFF, APPLE2_NIBBLE_SIZE);
	nibble[ 7]     = 0xD5;
	nibble[ 8]     = 0xAA;
	nibble[ 9]     = 0x96;
	nibble[10]     = (volume >> 1) | 0xAA;
	nibble[11]     = volume | 0xAA;
	nibble[12]     = (track >> 1) | 0xAA;
	nibble[13]     = track | 0xAA;
	nibble[14]     = (sector >> 1) | 0xAA;
	nibble[15]     = sector | 0xAA;
	nibble[16]     = (checksum >> 1) | 0xAA;
	nibble[17]     = (checksum) | 0xAA;
	nibble[18]     = 0xDE;
	nibble[19]     = 0xAA;
	nibble[20]     = 0xEB;
	nibble[25]     = 0xD5;
	nibble[26]     = 0xAA;
	nibble[27]     = 0xAD;
	nibble[27+344] = 0xDE;
	nibble[27+345] = 0xAA;
	nibble[27+346] = 0xEB;
	xorvalue = 0;

	for (i = 0; i < 342; i++)
	{
		if (i >= 0x56)
		{
			/* 6 bit */
			oldvalue = data[i - 0x56];
			oldvalue = oldvalue >> 2;
		}
		else
		{
			/* 3 * 2 bit */
			oldvalue = 0;
			oldvalue |= (data[i + 0x00] & 0x01) << 1;
			oldvalue |= (data[i + 0x00] & 0x02) >> 1;
			oldvalue |= (data[i + 0x56] & 0x01) << 3;
			oldvalue |= (data[i + 0x56] & 0x02) << 1;
			if (i + 0xAC < APPLE2_SECTOR_SIZE)
			{
				oldvalue |= (data[i + 0xAC] & 0x01) << 5;
				oldvalue |= (data[i + 0xAC] & 0x02) << 3;
			}

		}
		xorvalue ^= oldvalue;
		nibble[28+i] = translate6[xorvalue & 0x3F];
		xorvalue = oldvalue;
	}

	nibble[27+343] = translate6[xorvalue & 0x3F];
}



static floperr_t apple2_nib_write_sector(floppy_image_legacy *floppy, int head, int track, int sector, const void *buffer, size_t buflen, int ddam)
{
	floperr_t err;
	UINT8 *track_data;
	void *track_data_v;

	if ((sector < 0) || (sector >= APPLE2_SECTOR_COUNT))
		return FLOPPY_ERROR_SEEKERROR;
	if (buflen != APPLE2_SECTOR_SIZE)
		return FLOPPY_ERROR_INTERNAL;

	err = floppy_load_track(floppy, head, track, TRUE, &track_data_v, NULL);
	if (err)
		return err;
	track_data = (UINT8 *) track_data_v;

	disk_encode_nib(track_data + sector * APPLE2_NIBBLE_SIZE, (const UINT8 *)buffer, 254, track, sector);
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t apple2_nib_get_sector_length(floppy_image_legacy *floppy, int head, int track, int sector, UINT32 *sector_length)
{
	*sector_length = APPLE2_SECTOR_SIZE;
	return FLOPPY_ERROR_SUCCESS;
}



static UINT32 apple2_get_track_size(floppy_image_legacy *floppy, int head, int track)
{
	return APPLE2_NIBBLE_SIZE * APPLE2_SECTOR_COUNT;
}



/* ----------------------------------------------------------------------- */

LEGACY_FLOPPY_OPTIONS_START( apple2 )
	LEGACY_FLOPPY_OPTION( apple2_do, "do,dsk,bin",	"Apple ][ DOS order disk image",	apple2_dsk_identify,	apple2_do_construct, NULL,
		HEADS([1])
		TRACKS([35])
		SECTORS([16])
		SECTOR_LENGTH([256])
		FIRST_SECTOR_ID([0]))
	LEGACY_FLOPPY_OPTION( apple2_po, "po,dsk,bin",	"Apple ][ ProDOS order disk image",	apple2_dsk_identify,	apple2_po_construct, NULL,
		HEADS([1])
		TRACKS([35])
		SECTORS([16])
		SECTOR_LENGTH([256])
		FIRST_SECTOR_ID([0]))
	LEGACY_FLOPPY_OPTION( apple2_nib, "dsk,nib",	"Apple ][ Nibble order disk image",	apple2_nib_identify,	apple2_nib_construct, NULL,
		HEADS([1])
		TRACKS([35])
		SECTORS([16])
		SECTOR_LENGTH([256])
		FIRST_SECTOR_ID([0]))
LEGACY_FLOPPY_OPTIONS_END

/***************************************************************************

    New implementation

****************************************************************************

    Copyright Olivier Galibert, Lord Nightmare and Balrog
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


a2_16sect_format::a2_16sect_format() : floppy_image_format_t()
{
}

const char *a2_16sect_format::name() const
{
		return "a2_16sect";
}

const char *a2_16sect_format::description() const
{
		return "Apple II 16-sector dsk image";
}

const char *a2_16sect_format::extensions() const
{
		return "dsk";
}

bool a2_16sect_format::supports_save() const
{
		return true;
}

int a2_16sect_format::identify(io_generic *io, UINT32 form_factor)
{
		int size = io_generic_size(io);
		int expected_size = 35 * 16 * 256;
		return size == expected_size;
}

const floppy_image_format_t::desc_e a2_16sect_format::mac_gcr[] = {
		{ SECTOR_LOOP_START, 0, -1 },
		{   RAWBITS, 0xff3fcf, 24 }, { RAWBITS, 0xf3fcff, 24 },
		{   RAWBITS, 0xff3fcf, 24 }, { RAWBITS, 0xf3fcff, 24 },
		{   RAWBITS, 0xff3fcf, 24 }, { RAWBITS, 0xf3fcff, 24 },
		{   RAWBITS, 0xff3fcf, 24 }, { RAWBITS, 0xf3fcff, 24 },
		{   RAWBITS, 0xff3fcf, 24 }, { RAWBITS, 0xf3fcff, 24 },
		{   RAWBITS, 0xff3fcf, 24 }, { RAWBITS, 0xf3fcff, 24 },
		{   RAWBITS, 0xff3fcf, 24 }, { RAWBITS, 0xf3fcff, 24 },
		{   RAWBITS, 0xff3fcf, 24 }, { RAWBITS, 0xf3fcff, 24 },
		{   RAWBITS, 0xd5aa96, 24 },
		{   CRC_MACHEAD_START, 0 },
		{     TRACK_ID_GCR6 },
		{     SECTOR_ID_GCR6 },
		{     TRACK_HEAD_ID_GCR6 },
		{     SECTOR_INFO_GCR6 },
		{   CRC_END, 0 },
		{   CRC, 0 },
		{   RAWBITS, 0xdeaaff, 24 },
		{   RAWBITS, 0xff3fcf, 24 }, { RAWBITS, 0xf3fcff, 24 },
		{   RAWBITS, 0xd5aaad, 24 },
		{   SECTOR_ID_GCR6 },
		{   SECTOR_DATA_MAC, -1 },
		{   RAWBITS, 0xdeaaff, 24 },
		{   RAWBITS, 0xff, 8 },
		{ SECTOR_LOOP_END },
		{ END },
};


bool a2_16sect_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{

		UINT8 sector_data[(256)*16];
		memset(sector_data, 0, sizeof(sector_data));

		desc_s sectors[16];
		int format = 0;
		int pos_data = 0;

		int head_count = 1;

		for(int track=0; track < 35; track++) {
				for(int head=0; head < head_count; head++) {
						for(int si=0; si<16; si++) {
								UINT8 *data = sector_data + (256)*si;
								sectors[si].data = data;
								sectors[si].size = 256;
								sectors[si].sector_id = si;
								sectors[si].sector_info = format;
								io_generic_read(io, data, pos_data, 256);
								pos_data += 256;
						}
						generate_track(mac_gcr, track, head, sectors, 16, 3104*16, image);
				}
		}
		return true;
}

UINT8 a2_16sect_format::gb(const UINT8 *buf, int ts, int &pos, int &wrap)
{
		UINT8 v = 0;
		int w1 = wrap;
		while(wrap != w1+2 && !(v & 0x80)) {
				v = v << 1 | ((buf[pos >> 3] >> (7-(pos & 7))) & 1);
				pos++;
				if(pos == ts) {
						pos = 0;
						wrap++;
				}
		}
		return v;
}

void a2_16sect_format::update_chk(const UINT8 *data, int size, UINT32 &chk)
{
}

bool a2_16sect_format::save(io_generic *io, floppy_image *image)
{
		int g_tracks, g_heads;
		int visualgrid[16][35]; // visualizer grid, cleared/initialized below
#define NOTFOUND 0
#define ADDRFOUND 1
#define ADDRGOOD 2
#define DATAFOUND 4
#define DATAGOOD 8
#define DATAPOST 16
		for (int i = 0; i < 16; i++) {
			for (int j = 0; j < 35; j++) {
				visualgrid[i][j] = 0;
			}
		}
		image->get_actual_geometry(g_tracks, g_heads);

		int head = 0;

		int pos_data = 0;

		for(int track=0; track < 70; track+=2) {
				UINT8 sectdata[(256)*16];
				memset(sectdata, 0, sizeof(sectdata));
				int nsect = 16;
				UINT8 buf[130000]; // originally 13000, multiread dfi disks need larger
				int ts;
fprintf(stderr,"DEBUG: a2_16sect_format::save() about to generate bitstream from physical track %d (logical %d)...", track, track/2);
				generate_bitstream_from_track(track, head, 200000000/(3104*nsect*1), buf, ts, image); // 3104 needs tweaking, *3 is 3x multiread from a dfi disk
fprintf(stderr,"done.\n");
				int pos = 0;
				int wrap = 0;
				int hb = 0;
				for(;;) {
						UINT8 v = gb(buf, ts, pos, wrap);
						if(v == 0xff)
								hb = 1;
						else if(hb == 1 && v == 0xd5)
								hb = 2;
						else if(hb == 2 && v == 0xaa)
								hb = 3;
						else if(hb == 3 && v == 0x96)
								hb = 4;
						else
								hb = 0;

						if(hb == 4) {
								UINT8 h[11];
								for(int i=0; i<11; i++)
										h[i] = gb(buf, ts, pos, wrap);
								//UINT8 v2 = gcr6bw_tb[h[2]];
								UINT8 vl = gcr4_decode(h[0],h[1]);
								UINT8 tr = gcr4_decode(h[2],h[3]);
								UINT8 se = gcr4_decode(h[4],h[5]);
								UINT8 chk = gcr4_decode(h[6],h[7]);
								UINT32 post = (h[8]<<16)|(h[9]<<8)|h[10];
								printf("Address Mark:\tVolume %d, Track %d, Sector %2d, Checksum %02X: %s, Postamble %03X: %s\n", vl, tr, se, chk, (chk ^ vl ^ tr ^ se)==0?"OK":"BAD", post, (post&0xFFFF00)==0xDEAA00?"OK":"BAD");
								if (tr == track/2 && se < nsect) {
										visualgrid[se][track/2] |= ADDRFOUND;
										visualgrid[se][track/2] |= (chk ^ vl ^ tr ^ se)==0?ADDRGOOD:0;
										int opos = pos;
										int owrap = wrap;
										hb = 0;
										for(int i=0; i<20 && hb != 4; i++) {
												v = gb(buf, ts, pos, wrap);
												if(v == 0xff)
														hb = 1;
												else if(hb == 1 && v == 0xd5)
														hb = 2;
												else if(hb == 2 && v == 0xaa)
														hb = 3;
												else if(hb == 3 && v == 0xad)
														hb = 4;
												else
														hb = 0;
										}
										if(hb == 4) {
												visualgrid[se][track/2] |= DATAFOUND;
												UINT8 *dest = sectdata+(256)*se;
												UINT8 data[0x157];
												UINT32 dpost = 0;
												UINT8 c = 0;
												// first read in sector and decode to 6bit form
												for(int i=0; i<0x156; i++) {
														data[i] = gcr6bw_tb[gb(buf, ts, pos, wrap)] ^ c;
														c = data[i];
												//  printf("%02x ", c);
												//  if (((i&0xf)+1)==0x10) printf("\n");
												}
												// read the checksum byte
												data[0x156] = gcr6bw_tb[gb(buf,ts,pos,wrap)];
												// now read the postamble bytes
												for(int i=0; i<3; i++) {
														dpost <<= 8;
														dpost |= gb(buf, ts, pos, wrap);
												}
												// next combine in the upper 2 bits of each byte
												UINT8 bit_swap[4] = { 0, 2, 1, 3 };
												for(int i=0; i<0x56; i++)
														data[i+0x056] = data[i+0x056]<<2 |  bit_swap[data[i]&3];
												for(int i=0; i<0x56; i++)
														data[i+0x0ac] = data[i+0x0ac]<<2 |  bit_swap[(data[i]>>2)&3];
												for(int i=0; i<0x54; i++)
														data[i+0x102] = data[i+0x102]<<2 |  bit_swap[(data[i]>>4)&3];
												// now decode it into 256 bytes
												// but only write it if the bitfield of the track shows datagood is NOT set.
												// if it is set we don't want to overwrite a guaranteed good read with a bad one
												if ((visualgrid[se][track/2]&DATAGOOD)==0) {
													// TODO:
													// if the current read is good but the postamble isn't, write it in.
													// if the current read isn't good but the postamble is, its better than nothing.
													// if the current read isn't good and neither is the postamble but nothing better
													// has been written before, write it anyway.
													for(int i=0x56; i<0x156; i++) {
														UINT8 dv = data[i];
														*dest++ = dv;
													}
												}
												// do some checking
												if ((data[0x156] != c) || (dpost&0xFFFF00)!=0xDEAA00) {
												printf("Data Mark:\tChecksum xpctd %d found %d: %s, Postamble %03X: %s\n", data[0x156], c, (data[0x156]==c)?"OK":"BAD", dpost, (dpost&0xFFFF00)==0xDEAA00?"OK":"BAD");
												}
												else visualgrid[se][track/2] |= DATAGOOD;
										} else {
												pos = opos;
												wrap = owrap;
										}
								}
								hb = 0;
						}
						if(wrap)
								break;
				}
				for(int i=0; i<nsect; i++) {
						//if(nsect>0) printf("t%d,", track);
						UINT8 *data = sectdata + (256)*i;
						io_generic_write(io, data, pos_data, 256);
						pos_data += 256;
				}
				//printf("\n");
		}
		// display a little table of which sectors decoded ok
		for (int j = 0; j < 35; j++) {
			printf("T%2d: ",j);
			for (int i = 0; i < 16; i++) {
				if (visualgrid[i][j] == NOTFOUND) printf("-NF-");
				else {
				if (visualgrid[i][j] & ADDRFOUND) printf("a"); else printf(" ");
				if (visualgrid[i][j] & ADDRGOOD) printf("A"); else printf(" ");
				if (visualgrid[i][j] & DATAFOUND) printf("d"); else printf(" ");
				if (visualgrid[i][j] & DATAGOOD) printf("D"); else printf(" ");
				}
				printf(" ");
			}
			printf("\n");
		}

		return true;
}

const floppy_format_type FLOPPY_A216S_FORMAT = &floppy_image_format_creator<a2_16sect_format>;
