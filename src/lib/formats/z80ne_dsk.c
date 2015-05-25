// license:???
// copyright-holders:Roberto Lavarone
/*********************************************************************

    formats/z80ne_dsk.c

    Nuova Elettronica Z80NE disk images

*********************************************************************/

#include <assert.h>

#include "z80ne_dsk.h"
#include "basicdsk.h"
#include "imageutl.h"
#include "coretmpl.h"

/* -----------------------------------------------------------------------
 * DMK file format
 *
 * David M. Keil's disk image format is aptly called an 'on disk' image
 * format. This means that whatever written to the disk is encoded into
 * the image file. IDAMS, sector headers, trailing CRCs, and intra-sector
 * spacing.                                                         *
 *
 *  HEADER DESCRIPTION:
 *
 *  Offset  Bytes   Field           Description
 *  ------  -----   ------------    -----------
 *       0      1   write_prot      0xff = Write Protected, 0x00 = R/W
 *       1      1   tracks          Number of tracks
 *       2      2   track_length    Bytes per track on DMK file (little endian)
 *       4      1   disk_options    Miscellaneous flags
 *                                      bit 0-3:  Unused
 *                                      bit   4:  Sides on DMK file
 *                                                1=single, 0=double
 *                                      bit   5:  Unused
 *                                      bit   6:  Single Density DMK file
 *                                                1=single (bytes written once
 *                                                  on IDAMs)
 *                                                0=double (bytes written
 *                                                  two times for SD IDAMs)
 *       5      7   reserved        Reserved for future use
 *      12      4   real_disk_code  If this is 0x12345678 (little endian)
 *                                  then access a real disk drive
 *                                  (unsupported)
 *
 * Each track begins with a track TOC, consisting of 64 little endian 16-bit
 * integers.  Each integer has the following format:
 *      bit 0-13:   Offset from beginning of track to 'FE' byte of IDAM
 *                  Note these are always sorted from first to last. All empty
 *                  entries are 0x00
 *      bit 14:     Undefined (reserved)
 *      bit 15:     Sector double density (0=SD 1=DD)
 *                  If not set and SD DMK flag is set then bytes are written
 *                  once.
 * ----------------------------------------------------------------------- */

struct dmk_tag
{
	int heads;
	int tracks;
	UINT32 track_size;
};

/* DMK file structure
 * See WD1711 documentation
 */
/* DMK file header */
#define DMK_HEADER_LEN          16          /* DMK header */
	/*  For every track (40) */
	#define DMK_TOC_LEN             128     /* 64 16-bit relative offsets to IDAMs */
	#define DMK_GAP1_LEN            32      /* GAP1 (26 bytes 0xFF and 6 bytes 0x00) */

		/* For every sector (10) 301 bytes */
		#define DMK_IDAM_LEN            7   /* IDAM (0xFE, Track, 0x00, Sector, 0x00, CRChi, CRClo) */
		#define DMK_ID_GAP_LEN          17  /* GAP2 (11 bytes 0xFF and 6 bytes 0x00) */
		#define DMK_DAM_LEN             1   /* Data Address Mark (0xFB) */
		#define DMK_DATA_LEN            256 /* Data */
		#define DMK_DATA_CRC_LEN        2   /* Data CRC (CRChi, CRClo) */
		#define DMK_DATA_GAP_LEN        18  /* GAP3 (12 bytes 0xFF and 6 bytes 0x00) (not for last sector) */

	#define DMK_EXTRA_TRACK_LENGTH  (176)   /* GAP4 (192 bytes 0xFF) */
/* End of DMK file structure */

#define dmk_idam_type(x)            (x)[0]
#define dmk_idam_track(x)           (x)[1]
#define dmk_idam_side(x)            (x)[2]
#define dmk_idam_sector(x)          (x)[3]
#define dmk_idam_sectorlength(x)    (x)[4]
#define dmk_idam_crc(x)             (((x)[5] << 8) + (x)[6])
#define dmk_idam_set_crc(x, crc)    (x)[5] = ((crc) >> 8); (x)[6] = ((crc) >> 0);

static const char needle_data[] = "\x00\x00\x00\x00\x00\x00\x00\x00\xFB\xFB";
static const char needle_deleted_data_fa[] = "\x00\x00\x00\x00\x00\x00\x00\x00\xFA\xFA";
static const char needle_deleted_data_f8[] = "\x00\x00\x00\x00\x00\x00\x00\x00\xF8\xF8";


static struct dmk_tag *get_dmk_tag(floppy_image_legacy *floppy)
{
	return (dmk_tag *)floppy_tag(floppy);
}


static floperr_t z80ne_dmk_get_track_offset(floppy_image_legacy *floppy, int head, int track, UINT64 *offset)
{
	struct dmk_tag *tag = get_dmk_tag(floppy);

	if ((head < 0) || (head >= tag->heads) || (track < 0) || (track >= tag->tracks))
		return FLOPPY_ERROR_SEEKERROR;

	*offset = track;
	*offset *= tag->heads;
	*offset += head;
	*offset *= tag->track_size;
	*offset += DMK_HEADER_LEN;
	return FLOPPY_ERROR_SUCCESS;
}


static UINT32 z80ne_dmk_min_track_size(int sectors, int sector_length)
{
	int sector_physical_length;
	sector_physical_length = (DMK_IDAM_LEN + DMK_ID_GAP_LEN + DMK_DAM_LEN + sector_length + DMK_DATA_CRC_LEN + DMK_DATA_GAP_LEN);
	return DMK_TOC_LEN + 2 * (DMK_GAP1_LEN + (sectors * sector_physical_length) - DMK_DATA_GAP_LEN + DMK_EXTRA_TRACK_LENGTH);
}



static floperr_t z80ne_dmk_read_track(floppy_image_legacy *floppy, int head, int track, UINT64 offset, void *buffer, size_t buflen)
{
	floperr_t err;
	UINT64 track_offset;

	err = z80ne_dmk_get_track_offset(floppy, head, track, &track_offset);
	if (err)
		return err;

	floppy_image_read(floppy, buffer, offset + track_offset, buflen);
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t z80ne_dmk_write_track(floppy_image_legacy *floppy, int head, int track, UINT64 offset, const void *buffer, size_t buflen)
{
	floperr_t err;
	UINT64 track_offset;

	err = z80ne_dmk_get_track_offset(floppy, head, track, &track_offset);
	if (err)
		return err;

	floppy_image_write(floppy, buffer, offset + track_offset, buflen);
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t z80ne_dmk_get_track_data_offset(floppy_image_legacy *floppy, int head, int track, UINT64 *offset)
{
	*offset = DMK_TOC_LEN + 1;
	return FLOPPY_ERROR_SUCCESS;
}


static floperr_t z80ne_dmk_format_track(floppy_image_legacy *floppy, int head, int track, option_resolution *params)
{
	int sectors;
	int sector_length;
	int interleave;
	int first_sector_id;
	floperr_t err;
	int physical_sector;
	int logical_sector;
	int track_position;
	UINT16 idam_offset;
	UINT16 crc;
	UINT8 *track_data;
	void *track_data_v;
	UINT32 max_track_size;
	std::vector<int> sector_map;
	dynamic_buffer local_sector;
	int local_sector_size;
	int sector_position;
	int i;

	sectors         = option_resolution_lookup_int(params, PARAM_SECTORS);
	sector_length   = option_resolution_lookup_int(params, PARAM_SECTOR_LENGTH);
	interleave      = option_resolution_lookup_int(params, PARAM_INTERLEAVE);
	first_sector_id = option_resolution_lookup_int(params, PARAM_FIRST_SECTOR_ID);

	max_track_size = get_dmk_tag(floppy)->track_size;

	if (sectors > DMK_TOC_LEN/2)
	{
		err = FLOPPY_ERROR_INTERNAL;
		goto done;
	}

	if (max_track_size < z80ne_dmk_min_track_size(sectors, sector_length))
	{
		err = FLOPPY_ERROR_NOSPACE;
		goto done;
	}

	err = floppy_load_track(floppy, head, track, TRUE, &track_data_v, NULL);
	if (err)
		goto done;
	track_data = (UINT8 *) track_data_v;

	/* set up sector map */
	sector_map.resize(sectors);
	memset(&sector_map, 0xff, sectors);

	physical_sector = 0;
	for (logical_sector = 0; logical_sector < sectors; logical_sector++)
	{
		while(sector_map[physical_sector] >= 0)
		{
			physical_sector++;
			physical_sector %= sectors;
		}

		sector_map[physical_sector] = logical_sector + first_sector_id;
		physical_sector += interleave + 1;
		physical_sector %= sectors;
	}

	/* set up a local physical sector template */
	local_sector_size = (DMK_IDAM_LEN +
							DMK_ID_GAP_LEN +
							DMK_DAM_LEN +
							sector_length +
							DMK_DATA_CRC_LEN +
							DMK_DATA_GAP_LEN);
	local_sector.resize(local_sector_size);
	memset(&local_sector, 0, local_sector_size);

	/* set up track table of contents */
	physical_sector = 0;

	/* write GAP1 (track lead in) */
	memset(&track_data[DMK_TOC_LEN],    0xFF, 52);
	memset(&track_data[DMK_TOC_LEN+52], 0x00, 12);
	track_position = DMK_TOC_LEN + DMK_GAP1_LEN * 2;

	/*
	 *  prepare template in local sector
	 */

	/* IDAM */
	sector_position = 0;
	dmk_idam_type(              &local_sector[sector_position]) = 0xFE;
	dmk_idam_track(             &local_sector[sector_position]) = track;
	dmk_idam_side(              &local_sector[sector_position]) = head;
	dmk_idam_sector(            &local_sector[sector_position]) = 0;        /* replace in sector instances */
	dmk_idam_sectorlength(      &local_sector[sector_position]) = compute_log2(sector_length / 128);
	dmk_idam_set_crc(           &local_sector[sector_position], 0);         /* replace in sector instances */
	sector_position += 7;

	/* GAP2 (ID GAP) */
	memset(&local_sector[sector_position], 0xFF, 11);
	sector_position += 11;
	memset(&local_sector[sector_position], 0x00, 6);
	sector_position += 6;

	/* sector body */
	local_sector[sector_position] = 0xFB;
	memset(&local_sector[sector_position + 1], floppy_get_filler(floppy), sector_length);
	crc = ccitt_crc16(0xffff, &local_sector[sector_position], sector_length + 1);
	local_sector[sector_position + sector_length + 1] = (UINT8) (crc >> 8);
	local_sector[sector_position + sector_length + 2] = (UINT8) (crc >> 0);
	sector_position += sector_length + 3;

	/* GAP3 (Data GAP) */
	memset(&local_sector[sector_position], 0xFF, 12);
	sector_position += 12;
	memset(&local_sector[sector_position], 0x00, 6);
	sector_position += 6;

	/*
	 *  start of track data
	 */
	while(physical_sector < DMK_TOC_LEN/2)
	{
		if (physical_sector >= sectors)
		{
			/* no more sectors */
			idam_offset = 0;
		}
		else
		{
			/* this is a sector */
			idam_offset = track_position;
			logical_sector = sector_map[physical_sector];

			/* update IDAM in local sector template */
			dmk_idam_sector(            &local_sector[0]) = logical_sector;     /* update sector number in sector instance  */
			crc = ccitt_crc16(0xffff,   &local_sector[0], DMK_IDAM_LEN - 2);    /* and recalculate crc */
			dmk_idam_set_crc(           &local_sector[0], crc);

			/* write local sector in track doubling every byte (DMK double density mode) */
			for(i=0; i<local_sector_size; i++)
				track_data[track_position + i*2] = track_data[track_position + (i*2) + 1] = local_sector[i];
			track_position += (2 * local_sector_size);
			if (physical_sector == (sectors-1)) /* on last sector remove GAP3 */
				track_position -= (2 * DMK_DATA_GAP_LEN);
		}

		/* write the TOC entry */
		track_data[physical_sector * 2 + 0] = (UINT8) (idam_offset >> 0);
		track_data[physical_sector * 2 + 1] = (UINT8) (idam_offset >> 8);

		physical_sector++;
	}

	/* write track footer GAP4 */
	assert(max_track_size >= (UINT32)track_position);
	memset(&track_data[track_position], 0xFF, max_track_size - track_position);

done:
	return err;
}



static int z80ne_dmk_get_heads_per_disk(floppy_image_legacy *floppy)
{
	return get_dmk_tag(floppy)->heads;
}



static int z80ne_dmk_get_tracks_per_disk(floppy_image_legacy *floppy)
{
	return get_dmk_tag(floppy)->tracks;
}



static UINT32 z80ne_dmk_get_track_size(floppy_image_legacy *floppy, int head, int track)
{
	return get_dmk_tag(floppy)->track_size;
}

static UINT8 * my_memmem(UINT8 *haystack, size_t haystacklen,
		UINT8 *needle, size_t needlelen)
{
		register UINT8 *h = haystack;
		register UINT8 *n = needle;
		register size_t hl = haystacklen;
		register size_t nl = needlelen;
		register size_t i;

		if (nl == 0) return haystack;    /* The first occurrence of the empty string is deemed to occur at
                                                     the beginning of the string.  */
		if (hl < nl)
			return NULL;

		for( i = 0; (i < hl) && (i + nl <= hl ); i++ )
			if (h[i] == *n) /* first match */
				if ( !memcmp(&h[i]+1, n + 1, nl - 1) )
					return (haystack+i);    /* returns a pointer to the substring */

		return (UINT8 *)NULL;    /* not found */
}



static floperr_t z80ne_dmk_seek_sector_in_track(floppy_image_legacy *floppy, int head, int track, int sector, int sector_is_index, int dirtify, UINT8 **sector_data, UINT32 *sector_length)
{
	struct dmk_tag *tag = get_dmk_tag(floppy);
	floperr_t err;
	size_t idam_offset = 0;
	UINT16 calculated_crc;
	size_t i, j;
	size_t offs;
	/*int state;*/
	UINT8 *track_data;
	void *track_data_v;
	size_t track_length;
	size_t sec_len;
	dynamic_buffer local_idam;
	int local_idam_size;
	UINT8 *sec_data;

	err = floppy_load_track(floppy, head, track, dirtify, &track_data_v, &track_length);
	if (err)
		goto done;
	track_data = (UINT8 *) track_data_v;

	/* set up a local IDAM space */
	local_idam_size = (DMK_IDAM_LEN +
							DMK_ID_GAP_LEN +
							DMK_DAM_LEN +
							DMK_DATA_GAP_LEN);
	local_idam.resize(local_idam_size);
	memset(&local_idam[0], 0, local_idam_size);

	/* search for matching IDAM */
	for (i = 0; i < DMK_TOC_LEN / 2; i++)
	{
		idam_offset = track_data[i * 2 + 1];
		idam_offset <<= 8;
		idam_offset |= track_data[i * 2 + 0];
		idam_offset &= 0x3FFF;

		if (idam_offset == 0)
		{
			/* we've reached the end of the road */
			i = DMK_TOC_LEN;
			break;
		}

		if ((idam_offset + DMK_IDAM_LEN * 2) >= tag->track_size)
			continue;

		/* overflowing the track? */
		if ((idam_offset + local_idam_size * 2) >= tag->track_size)
		{
			err = FLOPPY_ERROR_SEEKERROR;
			goto done;
		}

		/* create a local copy of sector of IDAM */
		for(j = 0; j < (DMK_IDAM_LEN + DMK_ID_GAP_LEN + DMK_DAM_LEN); j++)
			local_idam[j] = track_data[idam_offset + j*2];

		/* from now on all operations are done in local copy of IDAM */
		calculated_crc = ccitt_crc16(0xffff, &local_idam[0], DMK_IDAM_LEN - 2);

		/* check IDAM integrity and check for matching sector */
		if (calculated_crc == dmk_idam_crc(&local_idam[0]))
		{
			if (sector_is_index)
			{
				/* the sector is indexed; decrement the index and go */
				if (sector-- == 0)
					break;
			}
			else
			{
				/* check for matching sector ID */
				if (sector == dmk_idam_sector(&local_idam[0]))
					break;
			}
		}
	}

	if (i >= DMK_TOC_LEN / 2)
	{
		err = FLOPPY_ERROR_SEEKERROR;
		goto done;
	}

	/* we found a matching sector ID */
	/*state = 0;*/
	offs = idam_offset + 2 * DMK_IDAM_LEN;

	/* find pattern 0x0000FB; this represents the start of a data sector */
	sec_data = my_memmem(&track_data[offs], 4 * (DMK_ID_GAP_LEN + DMK_DAM_LEN), (UINT8 *)needle_data, 10);
	if(!sec_data)
	{
		/* find pattern 0x0000FA; this represents the start of a deleted data sector (used for system sectors) */
		sec_data = my_memmem(&track_data[offs], 4 * (DMK_ID_GAP_LEN + DMK_DAM_LEN), (UINT8 *)needle_deleted_data_fa, 10);
	}
	if(!sec_data)
	{
		/* find pattern 0x0000F8; this represents the start of a deleted data sector (used for system sectors) */
		sec_data = my_memmem(&track_data[offs], 4 * (DMK_ID_GAP_LEN + DMK_DAM_LEN), (UINT8 *)needle_deleted_data_f8, 10);
	}

	if(sec_data)
		sec_data += 10;
	else
	{
		err = FLOPPY_ERROR_SEEKERROR;
		goto done;
	}
	offs = sec_data - &track_data[offs];

	sec_len = 128 << dmk_idam_sectorlength(&local_idam[0]);

	if ((offs + 2 * sec_len) > track_length)
	{
		err = FLOPPY_ERROR_INVALIDIMAGE;
		goto done;
	}

	if (sector_data)
		*sector_data = sec_data;
	if (sector_length)
		*sector_length = sec_len;
done:
	return err;
}



static floperr_t z80ne_dmk_get_sector_length(floppy_image_legacy *floppy, int head, int track, int sector, UINT32 *sector_length)
{
	return z80ne_dmk_seek_sector_in_track(floppy, head, track, sector, FALSE, FALSE, NULL, sector_length);
}



static floperr_t z80ne_dmk_get_indexed_sector_info(floppy_image_legacy *floppy, int head, int track, int sector_index, int *cylinder, int *side, int *sector, UINT32 *sector_length, unsigned long *flags)
{
	floperr_t err;
	size_t idam_offset;
	UINT8 *track_data;
	void *track_data_v;
	unsigned long dam;

	if (sector_index * 2 >= DMK_TOC_LEN)
		return FLOPPY_ERROR_SEEKERROR;

	err = floppy_load_track(floppy, head, track, FALSE, &track_data_v, NULL);
	if (err)
		return err;
	track_data = (UINT8 *) track_data_v;

	idam_offset = track_data[sector_index * 2 + 1];
	idam_offset <<= 8;
	idam_offset |= track_data[sector_index * 2 + 0];
	idam_offset &= 0x3FFF;

	if (idam_offset == 0)
		return FLOPPY_ERROR_SEEKERROR;

	/* determine type of data sector */
	/* find pattern 0x0000FB; this represents the start of a data sector */
	if ( my_memmem(&track_data[idam_offset+14], 4 * (DMK_ID_GAP_LEN + DMK_DAM_LEN), (UINT8 *)needle_data, 10) )
		dam = 0;
	/* find pattern 0x0000FA; this represents the start of a deleted data sector (used for system sectors) */
	else if ( my_memmem(&track_data[idam_offset+14], 4 * (DMK_ID_GAP_LEN + DMK_DAM_LEN), (UINT8 *)needle_deleted_data_fa, 10) )
		dam = ID_FLAG_DELETED_DATA;
	/* find pattern 0x0000F8; this represents the start of a deleted data sector (used for system sectors) */
	else if ( my_memmem(&track_data[idam_offset+14], 4 * (DMK_ID_GAP_LEN + DMK_DAM_LEN), (UINT8 *)needle_deleted_data_f8, 10) )
		dam = ID_FLAG_DELETED_DATA;
	else
		return FLOPPY_ERROR_SEEKERROR;

	if (cylinder)
		*cylinder = (&track_data[idam_offset])[2];
	if (side)
		*side = (&track_data[idam_offset])[4];
	if (sector)
		*sector = (&track_data[idam_offset])[6];
	if (sector_length)
		*sector_length = 128 << (&track_data[idam_offset])[8];
	if (flags)
		*flags = dam;
	return FLOPPY_ERROR_SUCCESS;
}



static floperr_t internal_z80ne_dmk_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, int sector_is_index, void *buffer, size_t buflen)
{
	floperr_t err;
	UINT32 sector_length;
	UINT16 crc_on_disk;
	UINT16 calculated_crc;
	UINT8 *sector_data = NULL;
	dynamic_buffer local_sector;
	int local_sector_size;
	int i;

	/* get sector length and data pointer */
	err = z80ne_dmk_seek_sector_in_track(floppy, head, track, sector, sector_is_index, FALSE, &sector_data, &sector_length);
	if (err)
		goto done;

	/* set up a local physical sector space (DAM + data + crc + GAP) */
	local_sector_size = (DMK_DAM_LEN + sector_length + DMK_DATA_CRC_LEN + DMK_DATA_GAP_LEN);
	local_sector.resize(local_sector_size);
	memset(&local_sector, 0, local_sector_size);

	/* get sector data */
	/* create a local copy of sector data including DAM (for crc calculation) */
	for(i = 0; i < (DMK_DAM_LEN + sector_length + DMK_DATA_CRC_LEN); i++)
		local_sector[i] = (sector_data - 2)[i*2];

	crc_on_disk = local_sector[sector_length + 0 + 1];
	crc_on_disk <<= 8;
	crc_on_disk += local_sector[sector_length + 1 + 1];

	/* crc calculated including DAM */
	calculated_crc = ccitt_crc16(0xffff, &local_sector[0], sector_length+1);
	if (calculated_crc != crc_on_disk)
	{
		err = FLOPPY_ERROR_INVALIDIMAGE;
		goto done;
	}

	/* copy local sector data excluding DAM */
	memcpy(buffer, &local_sector[1], MIN(sector_length, buflen));

	done:
	return err;
}



static floperr_t internal_z80ne_dmk_write_sector(floppy_image_legacy *floppy, int head, int track, int sector, int sector_is_index, const void *buffer, size_t buflen, int ddam)
{
	floperr_t err;
	UINT32 sector_length;
	UINT8 *sector_data;
	UINT16 crc;
	dynamic_buffer local_sector;
	int local_sector_size;
	int i;

	/* get sector length */
	err = z80ne_dmk_seek_sector_in_track(floppy, head, track, sector, sector_is_index, TRUE, &sector_data, &sector_length);
	if (err)
		goto done;

	/* set up a local physical sector space */
	local_sector_size = (DMK_DAM_LEN + sector_length + DMK_DATA_CRC_LEN + DMK_DATA_GAP_LEN);
	local_sector.resize(local_sector_size);
	memset(&local_sector[0], 0, local_sector_size);
	if(!ddam)
		local_sector[0] = 0xFB;
	else
		local_sector[0] = 0xFA;

	if (buflen > sector_length)
	{
		err = FLOPPY_ERROR_INTERNAL;
		goto done;
	}

	memcpy(&local_sector[1], buffer, buflen);

	crc = ccitt_crc16(0xffff, &local_sector[0], DMK_DAM_LEN + sector_length);
	local_sector[DMK_DAM_LEN + sector_length + 0] = crc >> 8;
	local_sector[DMK_DAM_LEN + sector_length + 1] = crc >> 0;

	/* write local sector in track doubling every byte (DMK double density mode) */
	/* write DAM or DDAM too */
	sector_data -= 2;
	for(i=0; i < (DMK_DAM_LEN + sector_length + DMK_DATA_CRC_LEN); i++)
		sector_data[i*2] = sector_data[(i*2)+1] = local_sector[i];

	done:
	return err;
}



static floperr_t z80ne_dmk_read_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	return internal_z80ne_dmk_read_sector(floppy, head, track, sector, FALSE, buffer, buflen);
}

static floperr_t z80ne_dmk_write_sector(floppy_image_legacy *floppy, int head, int track, int sector, const void *buffer, size_t buflen, int ddam)
{
	return internal_z80ne_dmk_write_sector(floppy, head, track, sector, FALSE, buffer, buflen, ddam);
}

static floperr_t z80ne_dmk_read_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector, void *buffer, size_t buflen)
{
	return internal_z80ne_dmk_read_sector(floppy, head, track, sector, TRUE, buffer, buflen);
}

static floperr_t z80ne_dmk_write_indexed_sector(floppy_image_legacy *floppy, int head, int track, int sector, const void *buffer, size_t buflen, int ddam)
{
	return internal_z80ne_dmk_write_sector(floppy, head, track, sector, TRUE, buffer, buflen, ddam);
}



static void z80ne_dmk_interpret_header(floppy_image_legacy *floppy, int *heads, int *tracks, int * sectors, int *track_size)
{
	UINT8 header[DMK_HEADER_LEN + DMK_TOC_LEN];
	int i;
	int idam_offset;
	int sects;

	floppy_image_read(floppy, header, 0, DMK_HEADER_LEN + DMK_TOC_LEN);

	if (tracks)
		*tracks = header[1];
	if (heads)
		*heads = (header[4] & 0x10) ? 1 : 2;
	if (track_size)
		*track_size = ((int) header[3]) * 0x100 + header[2];
	if(sectors)
	{
		/* count sectors on first track */
		sects = 0;
		for (i = 0; i < DMK_TOC_LEN / 2; i++)
		{
			idam_offset = header[DMK_HEADER_LEN + i * 2 + 1];
			idam_offset <<= 8;
			idam_offset |= header[DMK_HEADER_LEN + i * 2 + 0];
			idam_offset &= 0x3FFF;

			if (idam_offset == 0)
				break;
			else
				sects++;
		}
		*sectors = sects;
	}

}



FLOPPY_CONSTRUCT(z80ne_dmk_construct)
{
	struct FloppyCallbacks *callbacks;
	struct dmk_tag *tag;
	UINT8 header[DMK_HEADER_LEN];
	int heads, tracks, track_size, sectors, sector_length;

	if (params)
	{
		heads           = option_resolution_lookup_int(params, PARAM_HEADS);
		tracks          = option_resolution_lookup_int(params, PARAM_TRACKS);
		sectors         = option_resolution_lookup_int(params, PARAM_SECTORS);
		sector_length   = option_resolution_lookup_int(params, PARAM_SECTOR_LENGTH);

		track_size = z80ne_dmk_min_track_size(sectors, sector_length);

		memset(header, 0, sizeof(header));
		header[1] = tracks;
		header[2] = track_size >> 0;
		header[3] = track_size >> 8;
		header[4] = (heads == 2) ? 0x00 : 0x10;

		floppy_image_write(floppy, header, 0, sizeof(header));
	}
	else
	{
		z80ne_dmk_interpret_header(floppy, &heads, &tracks, &sectors, &track_size);
	}

	tag = (dmk_tag*)floppy_create_tag(floppy, sizeof(struct dmk_tag));
	if (!tag)
		return FLOPPY_ERROR_OUTOFMEMORY;
	tag->heads = heads;
	tag->track_size = track_size;
	tag->tracks = tracks;

	callbacks = floppy_callbacks(floppy);
	callbacks->read_track = z80ne_dmk_read_track;
	callbacks->write_track = z80ne_dmk_write_track;
	callbacks->get_track_data_offset = z80ne_dmk_get_track_data_offset;
	callbacks->format_track = z80ne_dmk_format_track;
	callbacks->get_heads_per_disk = z80ne_dmk_get_heads_per_disk;
	callbacks->get_tracks_per_disk = z80ne_dmk_get_tracks_per_disk;
	callbacks->get_track_size = z80ne_dmk_get_track_size;
	callbacks->get_sector_length = z80ne_dmk_get_sector_length;
	callbacks->get_indexed_sector_info = z80ne_dmk_get_indexed_sector_info;
	callbacks->read_sector = z80ne_dmk_read_sector;
	callbacks->write_sector = z80ne_dmk_write_sector;
	callbacks->read_indexed_sector = z80ne_dmk_read_indexed_sector;
	callbacks->write_indexed_sector = z80ne_dmk_write_indexed_sector;

	return FLOPPY_ERROR_SUCCESS;
}



FLOPPY_IDENTIFY(z80ne_dmk_identify)
{
	int heads, tracks, sectors, track_size;
	UINT64 size, expected_size;

	size = floppy_image_size(floppy);
	z80ne_dmk_interpret_header(floppy, &heads, &tracks, &sectors, &track_size);
	expected_size = DMK_HEADER_LEN + (heads * tracks * track_size);
	*vote = (size == expected_size) ? 90 : 0;
	*vote += (sectors == 10) ? 10 : 0;
	return FLOPPY_ERROR_SUCCESS;
}



/* ----------------------------------------------------------------------- */

LEGACY_FLOPPY_OPTIONS_START( z80ne )
	/*
	 * Single side
	 * Single density
	 * FM, 125 kbit/s,
	 * 40 tracks, track ID: 0..39
	 * 10 sectors/track, sector ID: 0..9
	 * 256 bytes/sector
	 * Interleave 1 (sector ID sequence on track: 0 5 1 6 2 7 3 8 4 9)
	 * 3125 byte/track unformatted
	 * Rotation speed 300 rpm (5 rps)
	 */
	LEGACY_FLOPPY_OPTION( z80ne_dmk, "zmk",     "Z80NE DMK disk image", z80ne_dmk_identify, z80ne_dmk_construct, NULL,
		HEADS([1])
		TRACKS([40])
		SECTORS([10])
		SECTOR_LENGTH([256])
		INTERLEAVE([1])
		FIRST_SECTOR_ID([0]))
LEGACY_FLOPPY_OPTIONS_END
