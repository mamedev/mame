/*************************************************************************

    formats/coupedsk.c

    SAM Coupe disk image formats

**************************************************************************/

#include "coupedsk.h"
#include "formats/basicdsk.h"


#define SAD_HEADER_LEN  22
#define SAD_SIGNATURE   "Aley's disk backup"

#define SDF_TRACKSIZE   (512 * 12)


struct sdf_tag
{
	UINT8 heads;
	UINT8 tracks;
};



/*************************************
 *
 *  MGT disk image format
 *
 *************************************/

/*
    Straight rip without any header

    Data:

    Side 0 Track 0
    Side 1 Track 0
    Side 0 Track 1
    Side 1 Track 1
    ...
*/


FLOPPY_CONSTRUCT( coupe_mgt_construct )
{
	struct basicdsk_geometry geometry;

	memset(&geometry, 0, sizeof(geometry));
	geometry.heads = 2;
	geometry.tracks = 80;
	geometry.sector_length = 512;
	geometry.first_sector_id = 1;

	if (params)
	{
		/* create */
		geometry.sectors = option_resolution_lookup_int(params, PARAM_SECTORS);
	}
	else
	{
		/* load */
		UINT64 size = floppy_image_size(floppy);

		/* verify size */
		if (size != 737280 && size != 819200)
			return FLOPPY_ERROR_INVALIDIMAGE;

		geometry.sectors = size / 81920;

	}

	return basicdsk_construct(floppy, &geometry);
}


FLOPPY_IDENTIFY( coupe_mgt_identify )
{
	UINT64 size;

	size = floppy_image_size(floppy);

	*vote = (size == 737280 || size == 819200) ? 100 : 0;
	return FLOPPY_ERROR_SUCCESS;
}



/*************************************
 *
 *  SAD disk image format
 *
 *************************************/

/*
    Header (22 bytes):

    0-17 "Aley's disk backup"
    18   number of sides
    19   number of tracks
    20   number of sectors per track
    21   sector size divided by 64

    Data:

    Side 0 Track 0
    Side 0 Track 1
    Side 0 Track 2
    ...
    Side 1 Track 0
    Side 1 Track 1
    ...
*/


static UINT64 coupe_sad_translate_offset(floppy_image *floppy,
	const struct basicdsk_geometry *geom, int track, int head, int sector)
{
	return head * geom->tracks * geom->sectors + geom->sectors * track + sector;
}


static void coupe_sad_interpret_header(floppy_image *floppy,
	int *heads, int *tracks, int *sectors, int *sector_size)
{
	UINT8 header[SAD_HEADER_LEN];

	floppy_image_read(floppy, header, 0, SAD_HEADER_LEN);

	*heads = header[18];
	*tracks = header[19];
	*sectors = header[20];
	*sector_size = header[21] << 6;
}


FLOPPY_CONSTRUCT( coupe_sad_construct )
{
	struct basicdsk_geometry geometry;
	int heads, tracks, sectors, sector_length;

	memset(&geometry, 0, sizeof(geometry));

	if (params)
	{
		/* create */
		UINT8 header[SAD_HEADER_LEN];

		/* get format options */
		heads = option_resolution_lookup_int(params, PARAM_HEADS);
		tracks = option_resolution_lookup_int(params, PARAM_TRACKS);
		sectors = option_resolution_lookup_int(params, PARAM_SECTORS);
		sector_length = option_resolution_lookup_int(params, PARAM_SECTOR_LENGTH);

		/* build the header */
		memset(header, 0, SAD_HEADER_LEN);
		memcpy(header, SAD_SIGNATURE, 18);
		header[18] = heads;
		header[19] = tracks;
		header[20] = sectors;
		header[21] = sector_length >> 6;

		/* and write it to disk */
		floppy_image_write(floppy, header, 0, sizeof(header));
	}
	else
	{
		/* load */
		coupe_sad_interpret_header(floppy, &heads, &tracks, &sectors, &sector_length);
	}

	/* fill in the data */
	geometry.offset = SAD_HEADER_LEN;
	geometry.heads = heads;
	geometry.tracks = tracks;
	geometry.sectors = sectors;
	geometry.first_sector_id = 1;
	geometry.sector_length = sector_length;
	geometry.translate_offset = coupe_sad_translate_offset;

	return basicdsk_construct(floppy, &geometry);
}


FLOPPY_IDENTIFY( coupe_sad_identify )
{
	int heads, tracks, sectors, sector_size;
	UINT64 size, calculated_size;

	size = floppy_image_size(floppy);

	/* read values from SAD header */
	coupe_sad_interpret_header(floppy, &heads, &tracks, &sectors, &sector_size);

	/* calculate expected disk image size */
	calculated_size = SAD_HEADER_LEN + heads * tracks * sectors * sector_size;

	*vote = (size == calculated_size) ? 100 : 0;
	return FLOPPY_ERROR_SUCCESS;
}



/*************************************
 *
 *  SDF disk image format
 *
 *  TODO: wd17xx status codes are
 *        currently ignored
 *
 *************************************/

static int coupe_sdf_get_heads_per_disk(floppy_image *floppy)
{
	struct sdf_tag *tag = (sdf_tag *)floppy_tag(floppy);
	return tag->heads;
}


static int coupe_sdf_get_tracks_per_disk(floppy_image *floppy)
{
	struct sdf_tag *tag = (sdf_tag *)floppy_tag(floppy);
	return tag->tracks;
}


static UINT32 coupe_sdf_get_track_size(floppy_image *floppy, int head, int track)
{
	return SDF_TRACKSIZE;
}


static floperr_t coupe_sdf_get_offset(floppy_image *floppy,
	int head, int track, UINT64 *offset)
{
	struct sdf_tag *tag = (sdf_tag *)floppy_tag(floppy);

	if (head > tag->heads || track > tag->tracks)
		return FLOPPY_ERROR_SEEKERROR;

	*offset = (head * tag->tracks + track) * SDF_TRACKSIZE;

	return FLOPPY_ERROR_SUCCESS;
}


static floperr_t coupe_sdf_read_track(floppy_image *floppy,
	int head, int track, UINT64 offset, void *buffer, size_t buflen)
{
	floperr_t err;
	UINT64 track_offset;

	/* get the offset to this track */
	err = coupe_sdf_get_offset(floppy, head, track, &track_offset);
	if (err) return err;

	/* read track data into buffer */
	floppy_image_read(floppy, buffer, offset + track_offset, buflen);

	return FLOPPY_ERROR_SUCCESS;
}


static floperr_t coupe_sdf_write_track(floppy_image *floppy,
	int head, int track, UINT64 offset, const void *buffer, size_t buflen)
{
	floperr_t err;
	UINT64 track_offset;

	/* get the offset to this track */
	err = coupe_sdf_get_offset(floppy, head, track, &track_offset);
	if (err) return err;

	/* write buffer to image */
	floppy_image_write(floppy, buffer, offset + track_offset, buflen);

	return FLOPPY_ERROR_SUCCESS;
}


static floperr_t coupe_sdf_get_sector_offset(floppy_image *floppy,
	int head, int track, int sector, UINT64 *offset)
{
	floperr_t err;
	UINT8 buffer[SDF_TRACKSIZE];
	int i, buffer_pos = 1;

	/* get track data */
	err = coupe_sdf_read_track(floppy, head, track, 0, &buffer, SDF_TRACKSIZE);
	if (err) return err;

	/* check if the sector is available in this track */
	if (sector >= buffer[0])
		return FLOPPY_ERROR_SEEKERROR;

	/* find the right sector in this track */
	for (i = 0; i < buffer[0]; i++)
	{
		int sector_number = buffer[buffer_pos + 4];

		if (sector_number - 1 == sector)
		{
			*offset = buffer_pos;
			return FLOPPY_ERROR_SUCCESS;
		}

		buffer_pos += (128 << buffer[buffer_pos + 5]) + 8;
	}

	return FLOPPY_ERROR_INVALIDIMAGE;
}


static floperr_t coupe_sdf_get_total_sector_offset(floppy_image *floppy,
	int head, int track, int sector, UINT64 *offset)
{
	floperr_t err;
	UINT64 track_offset, sector_offset;

	/* get offset to the track start */
	err = coupe_sdf_get_offset(floppy, head, track, &track_offset);
	if (err) return err;

	/* get offset to the start of the sector */
	err = coupe_sdf_get_sector_offset(floppy, head, track, sector, &sector_offset);
	if (err) return err;

	*offset = track_offset + sector_offset;

	return FLOPPY_ERROR_SUCCESS;
}


static floperr_t coupe_sdf_get_sector_length(floppy_image *floppy,
	int head, int track, int sector, UINT32 *sector_length)
{
	floperr_t err;
	UINT8 buffer[SDF_TRACKSIZE];
	UINT64 offset;

	/* get track data */
	err = coupe_sdf_read_track(floppy, head, track, 0, &buffer, SDF_TRACKSIZE);
	if (err) return err;

	/* get offset to the start of the sector */
	err = coupe_sdf_get_sector_offset(floppy, head, track, sector, &offset);
	if (err) return err;

	/* get size */
	*sector_length = 128 << buffer[offset + 5];

	return FLOPPY_ERROR_SUCCESS;
}


static floperr_t coupe_sdf_get_indexed_sector_info(floppy_image *floppy,
	int head, int track, int sector_index,
	int *cylinder, int *side, int *sector, UINT32 *sector_length,
	unsigned long *flags)
{
	floperr_t err;
	UINT8 buffer[SDF_TRACKSIZE];
	UINT64 offset;

	/* get track data */
	err = coupe_sdf_read_track(floppy, head, track, 0, &buffer, SDF_TRACKSIZE);
	if (err) return err;

	/* get offset to the start of the sector */
	err = coupe_sdf_get_sector_offset(floppy, head, track, sector_index, &offset);
	if (err) return err;

	/* extract data */
	if (cylinder)      *cylinder      = buffer[offset + 2];
	if (side)          *side          = buffer[offset + 3];
	if (sector)        *sector        = buffer[offset + 4];
	if (sector_length) *sector_length = 128 << buffer[offset + 5];
	if (flags)			*flags        = 0;	/* TODO: read DAM or DDAM and determine flags */

	return FLOPPY_ERROR_SUCCESS;
}


static floperr_t coupe_sdf_read_indexed_sector(floppy_image *floppy,
	int head, int track, int sector, void *buffer, size_t buflen)
{
	floperr_t err;
	UINT64 offset;

	err = coupe_sdf_get_total_sector_offset(floppy, head, track, sector, &offset);
	if (err) return err;

	/* read sector data into buffer */
	floppy_image_read(floppy, buffer, offset + 8, buflen);

	return FLOPPY_ERROR_SUCCESS;
}


static floperr_t coupe_sdf_write_indexed_sector(floppy_image *floppy,
	int head, int track, int sector, const void *buffer, size_t buflen, int ddam)
{
	floperr_t err;
	UINT64 offset;

	err = coupe_sdf_get_total_sector_offset(floppy, head, track, sector, &offset);
	if (err) return err;

	/* write buffer into image */
	floppy_image_write(floppy, buffer, offset + 8, buflen);

	return FLOPPY_ERROR_SUCCESS;
}


static void coupe_sdf_interpret_header(floppy_image *floppy, int *heads, int *tracks)
{
	UINT64 size = floppy_image_size(floppy);

	if (size % SDF_TRACKSIZE == 0)
	{
		*heads = 2;
		*tracks = size / (SDF_TRACKSIZE * 2);
	}
	else
	{
		*heads = 0;
		*tracks = 0;
	}
}


FLOPPY_CONSTRUCT( coupe_sdf_construct )
{
	struct FloppyCallbacks *callbacks;
	struct sdf_tag *tag;
	int heads, tracks;

	if (params)
	{
		/* create */
		return FLOPPY_ERROR_UNSUPPORTED;
	}
	else
	{
		/* load */
		coupe_sdf_interpret_header(floppy, &heads, &tracks);
	}

	tag = (sdf_tag *)floppy_create_tag(floppy, sizeof(struct sdf_tag));

	if (!tag)
		return FLOPPY_ERROR_OUTOFMEMORY;

	tag->heads = heads;
	tag->tracks = tracks;

	callbacks = floppy_callbacks(floppy);
	callbacks->read_track = coupe_sdf_read_track;
	callbacks->write_track = coupe_sdf_write_track;
	callbacks->get_heads_per_disk = coupe_sdf_get_heads_per_disk;
	callbacks->get_tracks_per_disk = coupe_sdf_get_tracks_per_disk;
	callbacks->get_track_size = coupe_sdf_get_track_size;
	callbacks->get_sector_length = coupe_sdf_get_sector_length;
	callbacks->read_indexed_sector = coupe_sdf_read_indexed_sector;
	callbacks->write_indexed_sector = coupe_sdf_write_indexed_sector;
	callbacks->get_indexed_sector_info = coupe_sdf_get_indexed_sector_info;

	return FLOPPY_ERROR_SUCCESS;
}


FLOPPY_IDENTIFY( coupe_sdf_identify )
{
	int heads, tracks;

	/* read header values */
	coupe_sdf_interpret_header(floppy, &heads, &tracks);

	/* check for sensible values */
	if (heads > 0 && heads < 3 && tracks > 0 && tracks < 84)
		*vote = 100;
	else
		*vote = 0;

	return FLOPPY_ERROR_SUCCESS;
}
