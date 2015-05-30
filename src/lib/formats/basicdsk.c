// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    formats/basicdsk.c

    Floppy format code for basic disks

*********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "basicdsk.h"

static floperr_t basicdsk_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen);
static floperr_t basicdsk_write_sector(floppy_image_legacy *floppy, int head, int track, int sector, const void *buffer, size_t buflen, int ddam);
static floperr_t basicdsk_read_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen);
static floperr_t basicdsk_write_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector, const void *buffer, size_t buflen, int ddam);
static floperr_t basicdsk_get_sector_length(floppy_image_legacy *floppy, int head, int track, int sector, UINT32 *sector_length);
static floperr_t basicdsk_get_indexed_sector_info(floppy_image_legacy *floppy, int head, int track, int sector_index, int *cylinder, int *side, int *sector, UINT32 *sector_length, unsigned long *flags);
static int basicdsk_get_heads_per_disk(floppy_image_legacy *floppy);
static int basicdsk_get_tracks_per_disk(floppy_image_legacy *floppy);
static floperr_t basicdsk_format_track(floppy_image_legacy *floppy, int head, int track, option_resolution *params);



#define BASICDSK_TAG    "basicdsktag"

struct basicdsk_tag
{
	struct basicdsk_geometry geometry;
};



/********************************************************************/

static const struct basicdsk_geometry *get_geometry(floppy_image_legacy *floppy)
{
	const struct basicdsk_tag *tag;
	tag = (const basicdsk_tag *)floppy_tag(floppy);
	return &tag->geometry;
}



floperr_t basicdsk_construct(floppy_image_legacy *floppy, const struct basicdsk_geometry *geometry)
{
	struct basicdsk_tag *tag;
	struct FloppyCallbacks *format;

	assert(geometry->heads);
	assert(geometry->tracks);
	assert(geometry->sectors);

	tag = (struct basicdsk_tag *) floppy_create_tag(floppy, sizeof(struct basicdsk_tag));
	if (!tag)
		return FLOPPY_ERROR_OUTOFMEMORY;
	tag->geometry = *geometry;

	/* set up format callbacks */
	format = floppy_callbacks(floppy);
	format->read_sector = basicdsk_read_sector;
	format->write_sector = basicdsk_write_sector;
	format->read_indexed_sector = basicdsk_read_indexed_sector;
	format->write_indexed_sector = basicdsk_write_indexed_sector;
	format->get_sector_length = basicdsk_get_sector_length;
	format->get_heads_per_disk = basicdsk_get_heads_per_disk;
	format->get_tracks_per_disk = basicdsk_get_tracks_per_disk;
	format->get_indexed_sector_info = basicdsk_get_indexed_sector_info;
	format->format_track = basicdsk_format_track;

	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t get_offset(floppy_image_legacy *floppy, int head, int track, int sector, int sector_is_index, UINT64 *offset)
{
	const struct basicdsk_geometry *geom;
	UINT64 offs;

	geom = get_geometry(floppy);

	/* translate the sector to a raw sector */
	if (!sector_is_index)
	{
		sector -= geom->first_sector_id;
	}

	if (geom->translate_sector)
		sector = geom->translate_sector(floppy, sector);

	/* check to see if we are out of range */
	if ((head < 0) || (head >= geom->heads) || (track < 0) || (track >= geom->tracks)
			|| (sector < 0) || (sector >= geom->sectors))
		return FLOPPY_ERROR_SEEKERROR;

	if (geom->translate_offset)
		offs = geom->translate_offset(floppy, geom, track, head, sector);
	else
	{
		offs = 0;
		offs += track;
		offs *= geom->heads;
		offs += head;
		offs *= geom->sectors;
		offs += sector;
	}
	offs *= geom->sector_length;
	offs += geom->offset;

	if (offset)
		*offset = offs;
	return FLOPPY_ERROR_SUCCESS;
}



static int internal_basicdsk_translate_sector_interleave(floppy_image_legacy *floppy, int sector)
{
	const struct basicdsk_geometry *geom = get_geometry(floppy);
	if (sector >= geom->sectors)
		return sector;
	return geom->sector_map[sector];
}



static floperr_t internal_basicdsk_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, int sector_is_index, void *buffer, size_t buflen)
{
	UINT64 offset;
	floperr_t err;

	err = get_offset(floppy, head, track, sector, sector_is_index, &offset);
	if (err)
		return err;
	floppy_image_read(floppy, buffer, offset, buflen);
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t internal_basicdsk_write_sector(floppy_image_legacy *floppy, int head, int track, int sector, int sector_is_index, const void *buffer, size_t buflen, int ddam)
{
	UINT64 offset;
	floperr_t err;

	err = get_offset(floppy, head, track, sector, sector_is_index, &offset);
	if (err)
		return err;

	floppy_image_write(floppy, buffer, offset, buflen);
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t basicdsk_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	return internal_basicdsk_read_sector(floppy, head, track, sector, FALSE, buffer, buflen);
}

static floperr_t basicdsk_write_sector(floppy_image_legacy *floppy, int head, int track, int sector, const void *buffer, size_t buflen, int ddam)
{
	return internal_basicdsk_write_sector(floppy, head, track, sector, FALSE, buffer, buflen, ddam);
}

static floperr_t basicdsk_read_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	return internal_basicdsk_read_sector(floppy, head, track, sector, TRUE, buffer, buflen);
}

static floperr_t basicdsk_write_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector, const void *buffer, size_t buflen, int ddam)
{
	return internal_basicdsk_write_sector(floppy, head, track, sector, TRUE, buffer, buflen, ddam);
}



static floperr_t basicdsk_format_track(floppy_image_legacy *floppy, int head, int track, option_resolution *params)
{
	floperr_t err = FLOPPY_ERROR_SUCCESS;
	UINT8 local_buffer[512];
	void *alloc_buffer = NULL;
	void *buffer;
	UINT32 sector_length;
	int sector;
	const struct basicdsk_geometry *geometry;

	geometry = get_geometry(floppy);

	sector_length = geometry->sector_length;

	if (sector_length > sizeof(local_buffer))
	{
		alloc_buffer = malloc(sector_length);
		if (!alloc_buffer)
		{
			err = FLOPPY_ERROR_OUTOFMEMORY;
			goto done;
		}
		buffer = alloc_buffer;
	}
	else
	{
		alloc_buffer = NULL;
		buffer = local_buffer;
	}

	memset(buffer, floppy_get_filler(floppy), sector_length);

	for (sector = 0; sector < geometry->sectors; sector++)
	{
		err = basicdsk_write_sector(floppy, head, track, sector + geometry->first_sector_id, buffer, sector_length, 0);
		if (err)
			goto done;
	}

done:
	if (alloc_buffer)
		free(alloc_buffer);
	return err;
}



static int basicdsk_get_heads_per_disk(floppy_image_legacy *floppy)
{
	return get_geometry(floppy)->heads;
}



static int basicdsk_get_tracks_per_disk(floppy_image_legacy *floppy)
{
	return get_geometry(floppy)->tracks;
}



static floperr_t basicdsk_get_sector_length(floppy_image_legacy *floppy, int head, int track, int sector, UINT32 *sector_length)
{
	floperr_t err;

	err = get_offset(floppy, head, track, sector, FALSE, NULL);
	if (err)
		return err;

	if (sector_length)
		*sector_length = get_geometry(floppy)->sector_length;
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t basicdsk_get_indexed_sector_info(floppy_image_legacy *floppy, int head, int track, int sector_index, int *cylinder, int *side, int *sector, UINT32 *sector_length, unsigned long *flags)
{
	const struct basicdsk_geometry *geom = get_geometry(floppy);

	if (geom->translate_sector)
		sector_index = geom->translate_sector(floppy, sector_index);

	sector_index += geom->first_sector_id;

	if (cylinder)
		*cylinder = track;
	if (side)
		*side = head;
	if (sector)
		*sector = sector_index;
	if (flags) {
		/* TODO: read DAM or DDAM and determine flags */
		*flags = 0;
		if (geom->get_ddam)
			*flags = geom->get_ddam(floppy, geom, track, head, sector_index);
	}
	return basicdsk_get_sector_length(floppy, head, track, sector_index, sector_length);
}



/********************************************************************
 * Generic Basicdsk Constructors
 ********************************************************************/

static void basicdsk_default_geometry(const struct FloppyFormat *format, struct basicdsk_geometry *geometry)
{
	optreserr_t err;
	int sector_length;
	memset(geometry, 0, sizeof(*geometry));

	err = option_resolution_getdefault(format->param_guidelines, PARAM_HEADS,           &geometry->heads);
	assert(!err);
	err = option_resolution_getdefault(format->param_guidelines, PARAM_TRACKS,          &geometry->tracks);
	assert(!err);
	err = option_resolution_getdefault(format->param_guidelines, PARAM_SECTORS,         &geometry->sectors);
	assert(!err);
	err = option_resolution_getdefault(format->param_guidelines, PARAM_FIRST_SECTOR_ID, &geometry->first_sector_id);
	assert(!err);
	err = option_resolution_getdefault(format->param_guidelines, PARAM_INTERLEAVE,      &geometry->interleave);
	if (err!=0) {
		geometry->interleave = 1;
	}
	err = option_resolution_getdefault(format->param_guidelines, PARAM_SECTOR_LENGTH,   &sector_length);
	assert(!err);
	geometry->sector_length = sector_length;

	if (geometry->interleave > 1)
	{
		int sector = 0;

		for (int i = 0; i < geometry->sectors; i++)
		{
			geometry->sector_map[i] = sector;

			sector += geometry->interleave;

			if (sector >= geometry->sectors)
				sector -= geometry->sectors;
		}

		geometry->translate_sector = internal_basicdsk_translate_sector_interleave;
	}
}



FLOPPY_CONSTRUCT(basicdsk_construct_default)
{
	struct basicdsk_geometry geometry;
	basicdsk_default_geometry(format, &geometry);
	return basicdsk_construct(floppy, &geometry);
}



FLOPPY_IDENTIFY(basicdsk_identify_default)
{
	UINT64 expected_size;
	struct basicdsk_geometry geometry;

	basicdsk_default_geometry(format, &geometry);

	expected_size = geometry.sector_length;
	expected_size *= geometry.heads;
	expected_size *= geometry.tracks;
	expected_size *= geometry.sectors;
	*vote = (floppy_image_size(floppy) == expected_size) ? 100 : 50;
	return FLOPPY_ERROR_SUCCESS;
}
