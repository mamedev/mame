/*********************************************************************

    formats/dsk_dsk.c

    DSK disk images

*********************************************************************/

#include <string.h>

#include "emu.h"
#include "imageutl.h"
#include "imagedev/flopimg.h"
#include "imagedev/flopdrv.h"

#define MV_CPC  	"MV - CPC"
#define EXTENDED	"EXTENDED"

struct dskdsk_tag
{
	int disk_image_type;  /* image type: standard or extended */
	int heads;
	int tracks;
	int sector_size;
	UINT64 track_offsets[84*2]; /* offset within data for each track */

};


static struct dskdsk_tag *get_tag(floppy_image *floppy)
{
	struct dskdsk_tag *tag;
	tag = (dskdsk_tag *)floppy_tag(floppy);
	return tag;
}



FLOPPY_IDENTIFY( dsk_dsk_identify )
{
	UINT8 header[8];

	floppy_image_read(floppy, header, 0, 8);
	if ( memcmp( header, MV_CPC, 8 ) ==0) {
		*vote = 100;
	} else
	if ( memcmp( header, EXTENDED, 8 ) ==0) {
		*vote = 100;
	} else {
		*vote = 0;
	}
	return FLOPPY_ERROR_SUCCESS;
}

static int dsk_get_heads_per_disk(floppy_image *floppy)
{
	return get_tag(floppy)->heads;
}

static int dsk_get_tracks_per_disk(floppy_image *floppy)
{
	return get_tag(floppy)->tracks;
}

static UINT64 dsk_get_track_offset(floppy_image *floppy, int head, int track)
{
	return get_tag(floppy)->track_offsets[(track<<1) + head];
}

static floperr_t get_offset(floppy_image *floppy, int head, int track, int sector, int sector_is_index, UINT64 *offset)
{
	UINT64 offs;
	UINT64 track_offset;
	UINT8 track_info[0x100];
	UINT8 sectors_per_track;
	int i;
	/* translate the sector to a raw sector */

	/* check to see if we are out of range */
	if ((head < 0) || (head >= get_tag(floppy)->heads) || (track < 0) || (track >= get_tag(floppy)->tracks)
			|| (sector < 0) )
		return FLOPPY_ERROR_SEEKERROR;

	track_offset = dsk_get_track_offset(floppy, head, track);

	floppy_image_read(floppy, track_info, track_offset, 0x100);

	sectors_per_track = track_info[0x015];
	if (!sector_is_index) {
		if (sector >= sectors_per_track) {
			return FLOPPY_ERROR_SEEKERROR;
		}
	}

	if (get_tag(floppy)->disk_image_type==0) {
		get_tag(floppy)->sector_size = (1<<(track_info[0x014]+7));
		offs = track_offset + 0x100 +sector * get_tag(floppy)->sector_size;
	} else {
		get_tag(floppy)->sector_size = track_info[0x18 + (sector<<3) + 6] + (track_info[0x18+(sector<<3) + 7]<<8);
		offs = track_offset + 0x100;
		for (i=0;i<sector;i++) {
			offs += track_info[0x18 + (i<<3) + 6] + (track_info[0x18+(i<<3) + 7]<<8);
		}
	}

	if (offset)
		*offset = offs;
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t internal_dsk_read_sector(floppy_image *floppy, int head, int track, int sector, int sector_is_index, void *buffer, size_t buflen)
{
	UINT64 offset;
	floperr_t err;
	err = get_offset(floppy, head, track, sector, sector_is_index, &offset);
	if (err)
		return err;
	floppy_image_read(floppy, buffer, offset, buflen);
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t internal_dsk_write_sector(floppy_image *floppy, int head, int track, int sector, int sector_is_index, const void *buffer, size_t buflen, int ddam)
{
	UINT64 offset;
	floperr_t err;

	err = get_offset(floppy, head, track, sector, sector_is_index, &offset);
	if (err)
		return err;

	floppy_image_write(floppy, buffer, offset, buflen);
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t dsk_read_sector(floppy_image *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	return internal_dsk_read_sector(floppy, head, track, sector, FALSE, buffer, buflen);
}

static floperr_t dsk_write_sector(floppy_image *floppy, int head, int track, int sector, const void *buffer, size_t buflen, int ddam)
{
	return internal_dsk_write_sector(floppy, head, track, sector, FALSE, buffer, buflen, ddam);
}

static floperr_t dsk_read_indexed_sector(floppy_image *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	return internal_dsk_read_sector(floppy, head, track, sector, TRUE, buffer, buflen);
}

static floperr_t dsk_write_indexed_sector(floppy_image *floppy, int head, int track, int sector, const void *buffer, size_t buflen, int ddam)
{
	return internal_dsk_write_sector(floppy, head, track, sector, TRUE, buffer, buflen, ddam);
}

static floperr_t dsk_get_sector_length(floppy_image *floppy, int head, int track, int sector, UINT32 *sector_length)
{
	floperr_t err;
	err = get_offset(floppy, head, track, sector, FALSE, NULL);
	if (err)
		return err;

	if (sector_length) {
		*sector_length = get_tag(floppy)->sector_size;
	}
	return FLOPPY_ERROR_SUCCESS;
}

static floperr_t dsk_get_indexed_sector_info(floppy_image *floppy, int head, int track, int sector_index, int *cylinder, int *side, int *sector, UINT32 *sector_length, unsigned long *flags)
{
	floperr_t retVal;
	UINT64 offset;
	UINT8 sector_info[0x100];
	int pos;

	retVal = get_offset(floppy, head, track, sector_index, FALSE, NULL);
	offset = dsk_get_track_offset(floppy,head,track);
	pos = 0x18 + (sector_index << 3);
	floppy_image_read(floppy, sector_info, offset, 0x100);
	if (cylinder)
		*cylinder = sector_info[pos + 0];
	if (side)
		*side = sector_info[pos + 1];
	if (sector)
		*sector = sector_info[pos + 2];
	if (sector_length) {
		*sector_length = 1 << (sector_info[pos + 3] + 7);
	}
	if (flags)
		*flags = (sector_info[pos + 5] & 0x40) ? ID_FLAG_DELETED_DATA : 0;
	return retVal;
}


FLOPPY_CONSTRUCT( dsk_dsk_construct )
{
	struct FloppyCallbacks *callbacks;
	struct dskdsk_tag *tag;
	UINT8 header[0x100];
	UINT64 tmp = 0;
	int i;
	int skip,cnt;

	if(params)
	{
		// create
		return FLOPPY_ERROR_UNSUPPORTED;
	}

	floppy_image_read(floppy, header, 0, 0x100);

	tag = (struct dskdsk_tag *) floppy_create_tag(floppy, sizeof(struct dskdsk_tag));
	if (!tag)
		return FLOPPY_ERROR_OUTOFMEMORY;

	tag->heads   = header[0x31];
	if (tag->heads==1) {
		skip = 2;
	} else {
		skip = 1;
	}
	tag->tracks  = header[0x30];
	cnt =0;
	if ( memcmp( header, MV_CPC, 8 ) ==0) {
		tag->disk_image_type = 0;
		tmp = 0x100;
		for (i=0; i<tag->tracks * tag->heads; i++)
		{
			tag->track_offsets[cnt] = tmp;
			tmp += pick_integer_le(header, 0x32, 2);
			cnt += skip;
		}
	} else  {
		tag->disk_image_type = 1;
		tmp = 0x100;
		for (i=0; i<tag->tracks * tag->heads; i++)
		{
			tag->track_offsets[cnt] = tmp;
			tmp += header[0x34 + i] << 8;
			cnt += skip;
		}
	}

	callbacks = floppy_callbacks(floppy);
	callbacks->read_sector = dsk_read_sector;
	callbacks->write_sector = dsk_write_sector;
	callbacks->read_indexed_sector = dsk_read_indexed_sector;
	callbacks->write_indexed_sector = dsk_write_indexed_sector;
	callbacks->get_sector_length = dsk_get_sector_length;
	callbacks->get_heads_per_disk = dsk_get_heads_per_disk;
	callbacks->get_tracks_per_disk = dsk_get_tracks_per_disk;
	callbacks->get_indexed_sector_info = dsk_get_indexed_sector_info;
	return FLOPPY_ERROR_SUCCESS;
}
