/*********************************************************************

    formats/oric_dsk.c

    Oric disk images

*********************************************************************/

#include <string.h>

#include "imageutl.h"
#include "flopimg.h"
#include "oric_dsk.h"
#include "basicdsk.h"


#define mfm_disk_header_size    0x0100
#define MFM_ID  "MFM_DISK"

#define TRACK_SIZE_MFM 0x1900

struct mfm_disk_sector_info
{
	int id_ptr;
	int data_ptr;
	int sector_size;
	UINT8 ddam;
};

struct oricdsk_tag
{
	int tracks;
	int heads;
	int geometry;
	int tracksize;
	int num_sectors;
	struct mfm_disk_sector_info sector_data[32];
};


static struct oricdsk_tag *get_tag(floppy_image_legacy *floppy)
{
	struct oricdsk_tag *tag;
	tag = (oricdsk_tag *)floppy_tag(floppy);
	return tag;
}


static FLOPPY_IDENTIFY(oric_dsk_identify)
{
	UINT8 header[mfm_disk_header_size];

	floppy_image_read(floppy, header, 0, mfm_disk_header_size);
	if ( memcmp( header, MFM_ID, 8 ) ==0) {
		UINT32 heads  = pick_integer_le(header, 8, 4);
		UINT32 tracks = pick_integer_le(header, 12, 4);

		if (floppy_image_size(floppy)==((tracks*heads*TRACK_SIZE_MFM)+mfm_disk_header_size)) {
			*vote = 100;
		} else {
			*vote = 0;
		}
	} else {
		*vote = 0;
	}
	return FLOPPY_ERROR_SUCCESS;
}
static int oric_get_track_offset(floppy_image_legacy *floppy,int track, int head)
{
	if (get_tag(floppy)->geometry==1) {
		return mfm_disk_header_size + (get_tag(floppy)->tracksize * track) + (head * get_tag(floppy)->tracksize * get_tag(floppy)->tracks);
	} else {
		return mfm_disk_header_size + (get_tag(floppy)->tracksize*((track * get_tag(floppy)->heads)+head));
	}
}

static int oric_get_heads_per_disk(floppy_image_legacy *floppy)
{
	return get_tag(floppy)->heads;
}

static int oric_get_tracks_per_disk(floppy_image_legacy *floppy)
{
	return get_tag(floppy)->tracks;
}

static void mfm_info_cache_sector_info(floppy_image_legacy *floppy,int track,int head)
{
	UINT8 track_data[TRACK_SIZE_MFM];

	/* initialise these with single density values if single density */
	UINT8 IdMark = 0x0fe;
	UINT8 DataMark = 0x0fb;
	UINT8 DeletedDataMark = 0x0f8;

	UINT8 SectorCount;
	UINT8 SearchCode = 0;
	UINT8 sector_number = 0;
	int ptr = 0;
	int track_offset = oric_get_track_offset(floppy,track,head);
	floppy_image_read(floppy, track_data, track_offset, TRACK_SIZE_MFM);
	SectorCount = 0;

	do
	{
		switch (SearchCode)
		{
			/* searching for id's */
			case 0:
			{
				/* found id mark? */
				if (track_data[ptr] == IdMark)
				{
					sector_number  = track_data[ptr+3]-1;
					/* store pointer to id mark */
					get_tag(floppy)->sector_data[sector_number].id_ptr = ptr + track_offset;
					SectorCount++;

					/* grab N value - used to skip data in data field */
					get_tag(floppy)->sector_data[sector_number].sector_size = (1<< (track_data[ptr+4]+7));

					/* skip past id field and crc */
					ptr+=7;

					/* now looking for data field */
					SearchCode = 1;
				}
				else
				{
					/* update position */
					ptr++;
				}
			}
			break;

			/* searching for data id's */
			case 1:
			{
				/* found data or deleted data? */
				if ((track_data[ptr] == DataMark) || (track_data[ptr] == DeletedDataMark))
				{
					/* yes */
					get_tag(floppy)->sector_data[sector_number].data_ptr = ptr + track_offset + 1;
					get_tag(floppy)->sector_data[sector_number].ddam = (track_data[ptr] == DeletedDataMark) ? ID_FLAG_DELETED_DATA : 0;

					/* skip data field and id */
					ptr += get_tag(floppy)->sector_data[sector_number].sector_size + 3;

					/* now looking for id field */
					SearchCode = 0;
				}
				else
				{
					ptr++;
				}
			}
			break;

			default:
				break;
		}
	}
	while (ptr < TRACK_SIZE_MFM);
	get_tag(floppy)->num_sectors = SectorCount;

}

static floperr_t get_offset(floppy_image_legacy *floppy, int head, int track, int sector, int sector_is_index, UINT64 *offset)
{
	UINT64 offs;

	/* translate the sector to a raw sector */
	if (!sector_is_index)
	{
		sector -= 1;
	}
	mfm_info_cache_sector_info(floppy,track,head);

	/* check to see if we are out of range */
	if ((head < 0) || (head >= get_tag(floppy)->heads) || (track < 0) || (track >= get_tag(floppy)->tracks)
			|| (sector < 0) || (sector >=get_tag(floppy)->num_sectors))
		return FLOPPY_ERROR_SEEKERROR;

	offs = get_tag(floppy)->sector_data[sector].data_ptr;
	if (offset)
		*offset = offs;
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t internal_oric_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, int sector_is_index, void *buffer, size_t buflen)
{
	UINT64 offset;
	floperr_t err;
	err = get_offset(floppy, head, track, sector, sector_is_index, &offset);
	if (err)
		return err;
	floppy_image_read(floppy, buffer, offset, buflen);
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t internal_oric_write_sector(floppy_image_legacy *floppy, int head, int track, int sector, int sector_is_index, const void *buffer, size_t buflen, int ddam)
{
	UINT64 offset;
	floperr_t err;

	err = get_offset(floppy, head, track, sector, sector_is_index, &offset);
	if (err)
		return err;

	floppy_image_write(floppy, buffer, offset, buflen);
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t oric_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	return internal_oric_read_sector(floppy, head, track, sector, FALSE, buffer, buflen);
}

static floperr_t oric_write_sector(floppy_image_legacy *floppy, int head, int track, int sector, const void *buffer, size_t buflen, int ddam)
{
	return internal_oric_write_sector(floppy, head, track, sector, FALSE, buffer, buflen, ddam);
}

static floperr_t oric_read_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	return internal_oric_read_sector(floppy, head, track, sector, TRUE, buffer, buflen);
}

static floperr_t oric_write_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector, const void *buffer, size_t buflen, int ddam)
{
	return internal_oric_write_sector(floppy, head, track, sector, TRUE, buffer, buflen, ddam);
}

static floperr_t oric_get_sector_length(floppy_image_legacy *floppy, int head, int track, int sector, UINT32 *sector_length)
{
	floperr_t err;
	err = get_offset(floppy, head, track, sector, FALSE, NULL);
	if (err)
		return err;

	if (sector_length) {
		*sector_length = get_tag(floppy)->sector_data[sector].sector_size;
	}
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t oric_get_indexed_sector_info(floppy_image_legacy *floppy, int head, int track, int sector_index, int *cylinder, int *side, int *sector, UINT32 *sector_length, unsigned long *flags)
{
	floperr_t retVal;

	sector_index += 1;

	retVal = oric_get_sector_length(floppy, head, track, sector_index, sector_length);
	if (sector_length!=NULL) {
		*sector_length =  get_tag(floppy)->sector_data[sector_index-1].sector_size;
	}
	if (cylinder)
		*cylinder = track;
	if (side)
		*side = head;
	if (sector)
		*sector = sector_index;
	if (flags)
		*flags = get_tag(floppy)->sector_data[sector_index].ddam;
	return retVal;
}


static FLOPPY_CONSTRUCT(oric_dsk_construct)
{
	struct FloppyCallbacks *callbacks;
	struct oricdsk_tag *tag;
	UINT8 header[mfm_disk_header_size];

	floppy_image_read(floppy, header, 0, mfm_disk_header_size);

	tag = (struct oricdsk_tag *) floppy_create_tag(floppy, sizeof(struct oricdsk_tag));
	if (!tag)
		return FLOPPY_ERROR_OUTOFMEMORY;

	tag->heads   = pick_integer_le(header, 8, 4);
	tag->tracks  = pick_integer_le(header, 12, 4);
	tag->geometry = pick_integer_le(header, 16, 4);
	tag->tracksize = TRACK_SIZE_MFM;
	memset(tag->sector_data,0,sizeof(tag->sector_data));

	callbacks = floppy_callbacks(floppy);
	callbacks->read_sector = oric_read_sector;
	callbacks->write_sector = oric_write_sector;
	callbacks->read_indexed_sector = oric_read_indexed_sector;
	callbacks->write_indexed_sector = oric_write_indexed_sector;
	callbacks->get_sector_length = oric_get_sector_length;
	callbacks->get_heads_per_disk = oric_get_heads_per_disk;
	callbacks->get_tracks_per_disk = oric_get_tracks_per_disk;
	callbacks->get_indexed_sector_info = oric_get_indexed_sector_info;
	return FLOPPY_ERROR_SUCCESS;
}
/* ----------------------------------------------------------------------- */

LEGACY_FLOPPY_OPTIONS_START( oric )
	LEGACY_FLOPPY_OPTION( oricmfm, "dsk", "Oric MFM floppy disk image", oric_dsk_identify, oric_dsk_construct, NULL, NULL)
	LEGACY_FLOPPY_OPTION( oric, "dsk", "Oric disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([2])
		TRACKS([80])
		SECTORS([9])
		SECTOR_LENGTH([512])
		FIRST_SECTOR_ID([1]))
LEGACY_FLOPPY_OPTIONS_END
