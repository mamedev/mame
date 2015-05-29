// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    APRIDISK disk image format

***************************************************************************/

#include "emu.h" // fatalerror
#include "apridisk.h"
#include "imageutl.h"
#include "coretmpl.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define APR_HEADER_SIZE     128

/* sector types */
#define APR_DELETED     0xe31d0000
#define APR_MAGIC       0xe31d0001
#define APR_COMMENT     0xe31d0002
#define APR_CREATOR     0xe31d0003

/* compression type */
#define APR_UNCOMPRESSED    0x9e90
#define APR_COMPRESSED      0x3e5a

static const char *apr_magic = "ACT Apricot disk image\x1a\x04";


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct apr_sector
{
	int head;
	int track;
	int sector;
	UINT8 data[512];
};

struct apr_tag
{
	int heads;
	int tracks;
	int sectors_per_track;
	struct apr_sector sectors[2880];
	char *comment;
};


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/* sector format:
 *
 * 00-03: sector type
 * 04-05: compression type
 * 06-07: header size
 * 08-11: data size
 * 12-13: track
 *    14: head
 *    15: sector
 */

static floperr_t apr_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	struct apr_tag *tag = (apr_tag *)floppy_tag(floppy);
//  printf("apr_read_sector %d %d %d\n", head, track, sector);
	memcpy(buffer, tag->sectors[head * track * sector].data, buflen);
	return FLOPPY_ERROR_SUCCESS;
}

static floperr_t apr_read_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	struct apr_tag *tag = (apr_tag *)floppy_tag(floppy);
//  printf("apr_read_indexed_sector %d %d %d\n", head, track, sector);
	memcpy(buffer, tag->sectors[head * track * sector].data, buflen);
	return FLOPPY_ERROR_SUCCESS;
}

/* sector length is always 512 byte */
static floperr_t apr_get_sector_length(floppy_image_legacy *floppy, int head, int track, int sector, UINT32 *sector_length)
{
	*sector_length = 512;
	return FLOPPY_ERROR_SUCCESS;
}

static int apr_get_heads_per_disk(floppy_image_legacy *floppy)
{
	struct apr_tag *tag = (apr_tag *)floppy_tag(floppy);
	return tag->heads;
}

static int apr_get_tracks_per_disk(floppy_image_legacy *floppy)
{
	struct apr_tag *tag = (apr_tag *)floppy_tag(floppy);
	return tag->tracks;
}

static floperr_t apr_get_indexed_sector_info(floppy_image_legacy *floppy, int head, int track, int sector_index, int *cylinder, int *side, int *sector, UINT32 *sector_length, unsigned long *flags)
{
	struct apr_tag *tag = (apr_tag *)floppy_tag(floppy);

//  printf("apr_get_indexed_sector_info %d %d %d\n", head, track, sector_index);

	/* sanity checks */
	if (head         < 0 || head         > (tag->heads - 1))             return FLOPPY_ERROR_SEEKERROR;
	if (track        < 0 || track        > (tag->tracks - 1))            return FLOPPY_ERROR_SEEKERROR;
	if (sector_index < 0 || sector_index > (tag->sectors_per_track - 1)) return FLOPPY_ERROR_SEEKERROR;

	if (cylinder)      *cylinder = tag->sectors[head * track * sector_index].track;
	if (side)          *side = tag->sectors[head * track * sector_index].head;
	if (sector)        *sector = tag->sectors[head * track * sector_index].sector;
	if (sector_length) *sector_length = 512;

	return FLOPPY_ERROR_SUCCESS;
}

FLOPPY_IDENTIFY( apridisk_identify )
{
	UINT8 header[APR_HEADER_SIZE];

	/* get header */
	floppy_image_read(floppy, &header, 0, sizeof(header));

	/* look for the magic string */
	if (memcmp(header, apr_magic, sizeof(*apr_magic)) == 0)
		*vote = 100;
	else
		*vote = 0;

	return FLOPPY_ERROR_SUCCESS;
}

FLOPPY_CONSTRUCT( apridisk_construct )
{
	struct FloppyCallbacks *callbacks;
	struct apr_tag *tag;

	int cur_sector = 0;
	UINT8 sector_header[16];
	UINT64 pos = 128;
	UINT32 type;

	tag = (apr_tag *)floppy_create_tag(floppy, sizeof(struct apr_tag));

	if (!tag)
		return FLOPPY_ERROR_OUTOFMEMORY;

	tag->heads = 0;
	tag->tracks = 0;
	tag->sectors_per_track = 0;

	floppy_image_read(floppy, &sector_header, pos, 16);
	type = pick_integer_le(&sector_header, 0, 4);

	while (type == APR_DELETED || type == APR_MAGIC || type == APR_COMMENT || type == APR_CREATOR)
	{
		UINT16 compression = pick_integer_le(&sector_header, 4, 2);
		UINT16 header_size = pick_integer_le(&sector_header, 6, 2);
		UINT32 data_size = pick_integer_le(&sector_header, 8, 4);

		tag->sectors[cur_sector].head = pick_integer_le(&sector_header, 12, 1);
		tag->sectors[cur_sector].sector = pick_integer_le(&sector_header, 13, 1);
		tag->sectors[cur_sector].track = pick_integer_le(&sector_header, 14, 1);

		pos += header_size;

		if (type == APR_MAGIC)
		{
			if (compression == APR_UNCOMPRESSED)
			{
				floppy_image_read(floppy, &tag->sectors[cur_sector].data, pos, data_size);

			}
			else if (compression == APR_COMPRESSED)
			{
				dynamic_buffer buffer(data_size);
				UINT16 length;
				UINT8 value;

				floppy_image_read(floppy, &buffer[0], pos, data_size);

				length = pick_integer_le(&buffer[0], 0, 2);
				value = pick_integer_le(&buffer[0], 2, 1);

				/* not sure if this is possible */
				if (length != 512) {
					fatalerror("Compression unsupported\n");
				}

				memset(&tag->sectors[cur_sector].data, value, length);
			}
			else
				return FLOPPY_ERROR_INVALIDIMAGE;

			tag->heads = MAX(tag->sectors[cur_sector].head, tag->heads);
			tag->tracks = MAX(tag->sectors[cur_sector].track, tag->tracks);
			tag->sectors_per_track = MAX(tag->sectors[cur_sector].sector, tag->sectors_per_track);
		}

		/* seek to next sector */
		pos += data_size;
		cur_sector++;

		floppy_image_read(floppy, &sector_header, pos, 16);
		type = pick_integer_le(&sector_header, 0, 4);
	}

	tag->heads++;
	tag->tracks++;

	callbacks = floppy_callbacks(floppy);
	callbacks->read_sector = apr_read_sector;
	callbacks->read_indexed_sector = apr_read_indexed_sector;
	callbacks->get_sector_length = apr_get_sector_length;
	callbacks->get_heads_per_disk = apr_get_heads_per_disk;
	callbacks->get_tracks_per_disk = apr_get_tracks_per_disk;
	callbacks->get_indexed_sector_info = apr_get_indexed_sector_info;

	return FLOPPY_ERROR_SUCCESS;
}
