/*********************************************************************

    formats/m20_dsk.c

    Olivetti M20 floppy-disk images

    Track 0/head 0 is FM, 128 byte sectors. The rest is MFM,
    256 byte sectors.
    In image files the sectors of track 0/sector 0 are 256 bytes
    long to simplify access. Only the first half of these sectors
    contain image data.

*********************************************************************/

#include <string.h>

#include "m20_dsk.h"
#include "basicdsk.h"

static FLOPPY_IDENTIFY(m20_dsk_identify)
{
	*vote = (floppy_image_size(floppy) == 286720) ? 100 : 0;
	return FLOPPY_ERROR_SUCCESS;
}

static int m20_get_heads_per_disk(floppy_image_legacy *floppy)
{
	return 2;
}

static int m20_get_tracks_per_disk(floppy_image_legacy *floppy)
{
	return 35;
}

static UINT64 m20_translate_offset(floppy_image_legacy *floppy, int track, int head, int sector)
{
	return 256*(32*track+16*head+sector);
}

static floperr_t get_offset(floppy_image_legacy *floppy, int head, int track, int sector, int sector_is_index, UINT64 *offset)
{
	UINT64 offs;
	/* translate the sector to a raw sector */
	if (!sector_is_index)
	{
		sector -= 1;
	}

	/* check to see if we are out of range */
	if ((head < 0) || (head >= 2) || (track < 0) || (track >= 35) || (sector < 0) || (sector >= 16))
		return FLOPPY_ERROR_SEEKERROR;

	offs = m20_translate_offset(floppy, track, head, sector);
	if (offset)
		*offset = offs;
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t internal_m20_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, int sector_is_index, void *buffer, size_t buflen)
{
	UINT64 offset;
	floperr_t err;

	//printf("internal_m20_read_sector: track = %d, head = %d, sector = %d, secisix = %d, buflen = %ld\n", track, head, sector, sector_is_index, (long)buflen);
	err = get_offset(floppy, head, track, sector, sector_is_index, &offset);
	if (err)
		return err;

	floppy_image_read(floppy, buffer, offset, buflen);
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t internal_m20_write_sector(floppy_image_legacy *floppy, int head, int track, int sector, int sector_is_index, const void *buffer, size_t buflen, int ddam)
{
	UINT64 offset;
	floperr_t err;

	err = get_offset(floppy, head, track, sector, sector_is_index, &offset);
	if (err)
		return err;

	floppy_image_write(floppy, buffer, offset, buflen);
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t m20_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	return internal_m20_read_sector(floppy, head, track, sector, FALSE, buffer, buflen);
}

static floperr_t m20_write_sector(floppy_image_legacy *floppy, int head, int track, int sector, const void *buffer, size_t buflen, int ddam)
{
	return internal_m20_write_sector(floppy, head, track, sector, FALSE, buffer, buflen, ddam);
}

static floperr_t m20_read_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	return internal_m20_read_sector(floppy, head, track, sector, TRUE, buffer, buflen);
}

static floperr_t m20_write_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector, const void *buffer, size_t buflen, int ddam)
{
	return internal_m20_write_sector(floppy, head, track, sector, TRUE, buffer, buflen, ddam);
}

static floperr_t m20_get_sector_length(floppy_image_legacy *floppy, int head, int track, int sector, UINT32 *sector_length)
{
	floperr_t err;
	err = get_offset(floppy, head, track, sector, FALSE, NULL);
	if (err)
		return err;

	if (sector_length) {
		if (track == 0 && head == 0)
			*sector_length = 128;
		else
			*sector_length = 256;
	}
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t m20_get_indexed_sector_info(floppy_image_legacy *floppy, int head, int track, int sector_index, int *cylinder, int *side, int *sector, UINT32 *sector_length, unsigned long *flags)
{
	sector_index += 1;
	if (cylinder)
		*cylinder = track;
	if (side)
		*side = head;
	if (sector)
		*sector = sector_index;
	if (flags)
		/* TODO: read DAM or DDAM and determine flags */
		*flags = 0;
	return m20_get_sector_length(floppy, head, track, sector_index, sector_length);
}


static FLOPPY_CONSTRUCT(m20_dsk_construct)
{
	struct FloppyCallbacks *callbacks;
	callbacks = floppy_callbacks(floppy);
	callbacks->read_sector = m20_read_sector;
	callbacks->write_sector = m20_write_sector;
	callbacks->read_indexed_sector = m20_read_indexed_sector;
	callbacks->write_indexed_sector = m20_write_indexed_sector;
	callbacks->get_sector_length = m20_get_sector_length;
	callbacks->get_heads_per_disk = m20_get_heads_per_disk;
	callbacks->get_tracks_per_disk = m20_get_tracks_per_disk;
	callbacks->get_indexed_sector_info = m20_get_indexed_sector_info;

	return FLOPPY_ERROR_SUCCESS;
}



/* ----------------------------------------------------------------------- */

LEGACY_FLOPPY_OPTIONS_START( m20 )
	LEGACY_FLOPPY_OPTION(m20_dsk, "img", "M20 disk image", m20_dsk_identify, m20_dsk_construct, NULL, NULL)
LEGACY_FLOPPY_OPTIONS_END
