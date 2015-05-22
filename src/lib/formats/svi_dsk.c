// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    formats/svi_dsk.c

    SVI318 disk images

*********************************************************************/

#include <string.h>
#include <assert.h>

#include "svi_dsk.h"
#include "basicdsk.h"

static FLOPPY_IDENTIFY(svi_dsk_identify)
{
	*vote = ((floppy_image_size(floppy) == (172032)) || (floppy_image_size(floppy) == (346112))) ? 100 : 0;
	return FLOPPY_ERROR_SUCCESS;
}

static int svi_get_heads_per_disk(floppy_image_legacy *floppy)
{
	return (floppy_image_size(floppy) == (172032)) ? 1 : 2;
}

static int svi_get_tracks_per_disk(floppy_image_legacy *floppy)
{
	return 40;
}

static UINT64 svi_translate_offset(floppy_image_legacy *floppy,
		int track, int head, int sector)
{
	UINT64 o;
	if ((track==0) && (head==0))
		o = sector*128;
	else
		o = ((track*((floppy_image_size(floppy) == (172032)) ? 1 : 2)+head)*17+sector)*256-2048; /* (17*256)-(18*128)=2048 */

	return o;
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
	if ((head < 0) || (head >= ((floppy_image_size(floppy) == (172032)) ? 1 : 2)) || (track < 0) || (track >= 40)
			|| (sector < 0) || (sector >= ((head==0 && track==0) ? 18 : 17)))
		return FLOPPY_ERROR_SEEKERROR;

	offs = svi_translate_offset(floppy, track, head, sector);
	if (offset)
		*offset = offs;
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t internal_svi_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, int sector_is_index, void *buffer, size_t buflen)
{
	UINT64 offset;
	floperr_t err;
	err = get_offset(floppy, head, track, sector, sector_is_index, &offset);
	if (err)
		return err;

	floppy_image_read(floppy, buffer, offset, buflen);
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t internal_svi_write_sector(floppy_image_legacy *floppy, int head, int track, int sector, int sector_is_index, const void *buffer, size_t buflen, int ddam)
{
	UINT64 offset;
	floperr_t err;

	err = get_offset(floppy, head, track, sector, sector_is_index, &offset);
	if (err)
		return err;

	floppy_image_write(floppy, buffer, offset, buflen);
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t svi_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	return internal_svi_read_sector(floppy, head, track, sector, FALSE, buffer, buflen);
}

static floperr_t svi_write_sector(floppy_image_legacy *floppy, int head, int track, int sector, const void *buffer, size_t buflen, int ddam)
{
	return internal_svi_write_sector(floppy, head, track, sector, FALSE, buffer, buflen, ddam);
}

static floperr_t svi_read_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	return internal_svi_read_sector(floppy, head, track, sector, TRUE, buffer, buflen);
}

static floperr_t svi_write_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector, const void *buffer, size_t buflen, int ddam)
{
	return internal_svi_write_sector(floppy, head, track, sector, TRUE, buffer, buflen, ddam);
}

static floperr_t svi_get_sector_length(floppy_image_legacy *floppy, int head, int track, int sector, UINT32 *sector_length)
{
	floperr_t err;
	err = get_offset(floppy, head, track, sector, FALSE, NULL);
	if (err)
		return err;

	if (sector_length) {
		*sector_length = (head==0 && track==0) ? 128 : 256;
	}
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t svi_get_indexed_sector_info(floppy_image_legacy *floppy, int head, int track, int sector_index, int *cylinder, int *side, int *sector, UINT32 *sector_length, unsigned long *flags)
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
	return svi_get_sector_length(floppy, head, track, sector_index, sector_length);
}


static FLOPPY_CONSTRUCT(svi_dsk_construct)
{
	struct FloppyCallbacks *callbacks;
	callbacks = floppy_callbacks(floppy);
	callbacks->read_sector = svi_read_sector;
	callbacks->write_sector = svi_write_sector;
	callbacks->read_indexed_sector = svi_read_indexed_sector;
	callbacks->write_indexed_sector = svi_write_indexed_sector;
	callbacks->get_sector_length = svi_get_sector_length;
	callbacks->get_heads_per_disk = svi_get_heads_per_disk;
	callbacks->get_tracks_per_disk = svi_get_tracks_per_disk;
	callbacks->get_indexed_sector_info = svi_get_indexed_sector_info;

	return FLOPPY_ERROR_SUCCESS;
}



/* ----------------------------------------------------------------------- */

LEGACY_FLOPPY_OPTIONS_START( svi318 )
	LEGACY_FLOPPY_OPTION( svi_dsk, "dsk", "SVI-318 floppy disk image",  svi_dsk_identify, svi_dsk_construct, NULL, NULL)
	LEGACY_FLOPPY_OPTION( svi_cpm, "dsk", "SVI-728 DSDD CP/M disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([2])
		TRACKS([40])
		SECTORS([17])
		SECTOR_LENGTH([256])
		FIRST_SECTOR_ID([1]))
LEGACY_FLOPPY_OPTIONS_END
