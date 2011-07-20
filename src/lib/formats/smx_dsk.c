/*********************************************************************

    formats/smx_dsk.c

    Specialist MX disk images

*********************************************************************/

#include <string.h>

#include "smx_dsk.h"
#include "basicdsk.h"

static FLOPPY_IDENTIFY(smx_dsk_identify)
{
	*vote = (floppy_image_size(floppy) == 819200) ? 100 : 0;
	return FLOPPY_ERROR_SUCCESS;
}

static int smx_get_heads_per_disk(floppy_image_legacy *floppy)
{
	return 2;
}

static int smx_get_tracks_per_disk(floppy_image_legacy *floppy)
{
	return 80;
}

static UINT64 smx_translate_offset(floppy_image_legacy *floppy,
	 int track, int head, int sector)
{
	return (track * 1024 * 5 * 2) + (head * 1024 * 5) + 1024 * sector;
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
	if ((head < 0) || (head >= 2) || (track < 0) || (track >= 80)
			|| (sector < 0) || (sector >= 6))
		return FLOPPY_ERROR_SEEKERROR;

	offs = smx_translate_offset(floppy, track, head, sector);
	if (offset)
		*offset = offs;
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t internal_smx_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, int sector_is_index, void *buffer, size_t buflen)
{
	UINT64 offset;
	floperr_t err;
	err = get_offset(floppy, head, track, sector, sector_is_index, &offset);
	if (err)
		return err;

	floppy_image_read(floppy, buffer, offset, buflen);
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t internal_smx_write_sector(floppy_image_legacy *floppy, int head, int track, int sector, int sector_is_index, const void *buffer, size_t buflen, int ddam)
{
	UINT64 offset;
	floperr_t err;

	err = get_offset(floppy, head, track, sector, sector_is_index, &offset);
	if (err)
		return err;

	floppy_image_write(floppy, buffer, offset, buflen);
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t smx_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	return internal_smx_read_sector(floppy, head, track, sector, FALSE, buffer, buflen);
}

static floperr_t smx_write_sector(floppy_image_legacy *floppy, int head, int track, int sector, const void *buffer, size_t buflen, int ddam)
{
	return internal_smx_write_sector(floppy, head, track, sector, FALSE, buffer, buflen, ddam);
}

static floperr_t smx_read_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	return internal_smx_read_sector(floppy, head, track, sector, TRUE, buffer, buflen);
}

static floperr_t smx_write_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector, const void *buffer, size_t buflen, int ddam)
{
	return internal_smx_write_sector(floppy, head, track, sector, TRUE, buffer, buflen, ddam);
}

static floperr_t smx_get_sector_length(floppy_image_legacy *floppy, int head, int track, int sector, UINT32 *sector_length)
{
	floperr_t err;
	err = get_offset(floppy, head, track, sector, FALSE, NULL);
	if (err)
		return err;

	if (sector_length) {
		*sector_length = 1024;
	}
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t smx_get_indexed_sector_info(floppy_image_legacy *floppy, int head, int track, int sector_index, int *cylinder, int *side, int *sector, UINT32 *sector_length, unsigned long *flags)
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
	return smx_get_sector_length(floppy, head, track, sector_index, sector_length);
}


static FLOPPY_CONSTRUCT(smx_dsk_construct)
{
	struct FloppyCallbacks *callbacks;
	callbacks = floppy_callbacks(floppy);
	callbacks->read_sector = smx_read_sector;
	callbacks->write_sector = smx_write_sector;
	callbacks->read_indexed_sector = smx_read_indexed_sector;
	callbacks->write_indexed_sector = smx_write_indexed_sector;
	callbacks->get_sector_length = smx_get_sector_length;
	callbacks->get_heads_per_disk = smx_get_heads_per_disk;
	callbacks->get_tracks_per_disk = smx_get_tracks_per_disk;
	callbacks->get_indexed_sector_info = smx_get_indexed_sector_info;

	return FLOPPY_ERROR_SUCCESS;
}



/* ----------------------------------------------------------------------- */

LEGACY_FLOPPY_OPTIONS_START( specimx )
	LEGACY_FLOPPY_OPTION( smx_dsk, "odi", "Specialist MX floppy disk image",	smx_dsk_identify, smx_dsk_construct, NULL, NULL)
LEGACY_FLOPPY_OPTIONS_END
