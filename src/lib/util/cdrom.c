/***************************************************************************

    Generic MAME CD-ROM utilties - build IDE and SCSI CD-ROMs on top of this

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    IMPORTANT:
    "physical" block addresses are the actual addresses on the emulated CD.
    "chd" block addresses are the block addresses in the CHD file.
    Because we pad each track to a hunk boundry, these addressing
    schemes will differ after track 1!

***************************************************************************/

#include "cdrom.h"



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define VERBOSE	(0)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct _cdrom_file
{
	chd_file *			chd;				/* CHD file */
	cdrom_toc			cdtoc;				/* TOC for the CD */
	UINT32				hunksectors;		/* sectors per hunk */
	UINT32				cachehunk;			/* which hunk is cached */
	UINT8 *				cache;				/* cache of the current hunk */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static chd_error read_sector_into_cache(cdrom_file *file, UINT32 lbasector, UINT32 *sectoroffs, UINT32 *tracknum);
static chd_error parse_metadata(chd_file *chd, cdrom_toc *toc);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    physical_to_chd_lba - find the CHD LBA
    and the track number
-------------------------------------------------*/

INLINE UINT32 physical_to_chd_lba(cdrom_file *file, UINT32 physlba, UINT32 *tracknum)
{
	UINT32 chdlba;
	int track;

	/* loop until our current LBA is less than the start LBA of the next track */
	for (track = 0; track < file->cdtoc.numtrks; track++)
		if (physlba < file->cdtoc.tracks[track + 1].physframeofs)
		{
			chdlba = physlba - file->cdtoc.tracks[track].physframeofs + file->cdtoc.tracks[track].chdframeofs;
			if (tracknum != NULL)
				*tracknum = track;
			return chdlba;
		}

	return physlba;
}



/***************************************************************************
    BASE FUNCTIONALITY
***************************************************************************/

/*-------------------------------------------------
    cdrom_open - "open" a CD-ROM file from an
    already-opened CHD file
-------------------------------------------------*/

cdrom_file *cdrom_open(chd_file *chd)
{
	const chd_header *header = chd_get_header(chd);
	int i;
	cdrom_file *file;
	UINT32 physofs, chdofs;
	chd_error err;

	/* punt if no CHD */
	if (!chd)
		return NULL;

	/* validate the CHD information */
	if (header->hunkbytes % CD_FRAME_SIZE != 0)
		return NULL;

	/* allocate memory for the CD-ROM file */
	file = malloc(sizeof(cdrom_file));
	if (file == NULL)
		return NULL;

	/* fill in the data */
	file->chd = chd;
	file->hunksectors = header->hunkbytes / CD_FRAME_SIZE;
	file->cachehunk = -1;

	/* read the CD-ROM metadata */
	err = parse_metadata(chd, &file->cdtoc);
	if (err != CHDERR_NONE)
	{
		free(file);
		return NULL;
	}

	#if VERBOSE
	logerror("CD has %d tracks\n", file->cdtoc.numtrks);
	#endif

	/* calculate the starting frame for each track, keeping in mind that CHDMAN
       pads tracks out with extra frames to fit hunk size boundries
    */
	physofs = chdofs = 0;
	for (i = 0; i < file->cdtoc.numtrks; i++)
	{
		file->cdtoc.tracks[i].physframeofs = physofs;
		file->cdtoc.tracks[i].chdframeofs = chdofs;

		physofs += file->cdtoc.tracks[i].frames;
		chdofs  += file->cdtoc.tracks[i].frames;
		chdofs  += file->cdtoc.tracks[i].extraframes;

		#if VERBOSE
		logerror("Track %02d is format %d subtype %d datasize %d subsize %d frames %d extraframes %d physofs %d chdofs %d\n", i+1,
			file->cdtoc.tracks[i].trktype,
			file->cdtoc.tracks[i].subtype,
			file->cdtoc.tracks[i].datasize,
			file->cdtoc.tracks[i].subsize,
			file->cdtoc.tracks[i].frames,
			file->cdtoc.tracks[i].extraframes,
			file->cdtoc.tracks[i].physframeofs,
			file->cdtoc.tracks[i].chdframeofs);
		#endif
	}

	/* fill out dummy entries for the last track to help our search */
	file->cdtoc.tracks[i].physframeofs = physofs;
	file->cdtoc.tracks[i].chdframeofs = chdofs;

	/* allocate a cache */
	file->cache = malloc(chd_get_header(chd)->hunkbytes);
	if (file->cache == NULL)
	{
		free(file);
		return NULL;
	}

	return file;
}


/*-------------------------------------------------
    cdrom_close - "close" a CD-ROM file
-------------------------------------------------*/

void cdrom_close(cdrom_file *file)
{
	if (file == NULL)
		return;

	/* free the cache */
	if (file->cache)
		free(file->cache);
	free(file);
}



/***************************************************************************
    CORE READ ACCESS
***************************************************************************/

/*-------------------------------------------------
    cdrom_read_data - read one or more sectors
    from a CD-ROM
-------------------------------------------------*/

UINT32 cdrom_read_data(cdrom_file *file, UINT32 lbasector, void *buffer, UINT32 datatype)
{
	UINT32 tracktype, tracknum, sectoroffs;
	chd_error err;

	if (file == NULL)
		return 0;

	/* cache in the sector */
	err = read_sector_into_cache(file, lbasector, &sectoroffs, &tracknum);
	if (err != CHDERR_NONE)
		return 0;

	/* copy out the requested sector */
	tracktype = file->cdtoc.tracks[tracknum].trktype;
	if ((datatype == tracktype) || (datatype == CD_TRACK_RAW_DONTCARE))
	{
		memcpy(buffer, &file->cache[sectoroffs * CD_FRAME_SIZE], file->cdtoc.tracks[tracknum].datasize);
	}
	else
	{
		/* return 2048 bytes of mode1 data from a 2336 byte mode 1 or 2 raw sector */
		if ((datatype == CD_TRACK_MODE1) && ((tracktype == CD_TRACK_MODE1_RAW) || (tracktype == CD_TRACK_MODE2_RAW)))
		{
			memcpy(buffer, &file->cache[(sectoroffs * CD_FRAME_SIZE) + 16], 2048);
			return 1;
		}

		/* return 2048 bytes of mode1 data from a mode2 form1 sector */
		if ((datatype == CD_TRACK_MODE1) && (tracktype == CD_TRACK_MODE2_FORM1))
		{
			memcpy(buffer, &file->cache[(sectoroffs * CD_FRAME_SIZE) + 24], 2048);
			return 1;
		}

		/* return mode 2 2336 byte data from a RAW sector */
		if ((datatype == CD_TRACK_MODE2) && ((tracktype == CD_TRACK_MODE1_RAW) || (tracktype == CD_TRACK_MODE2_RAW)))
		{
			memcpy(buffer, &file->cache[(sectoroffs * CD_FRAME_SIZE) + 16], 2048);
			return 1;
		}

		#if VERBOSE
		logerror("CDROM: Conversion from type %d to type %d not supported!\n", tracktype, datatype);
		#endif
		return 0;
	}
	return 1;
}


/*-------------------------------------------------
    cdrom_read_subcode - read subcode data for
    a sector
-------------------------------------------------*/

UINT32 cdrom_read_subcode(cdrom_file *file, UINT32 lbasector, void *buffer)
{
	UINT32 sectoroffs, tracknum;
	chd_error err;

	if (file == NULL)
		return ~0;

	/* cache in the sector */
	err = read_sector_into_cache(file, lbasector, &sectoroffs, &tracknum);
	if (err != CHDERR_NONE)
		return 0;

	/* copy out the requested data */
	memcpy(buffer, &file->cache[(sectoroffs * CD_FRAME_SIZE) + file->cdtoc.tracks[tracknum].datasize], file->cdtoc.tracks[tracknum].subsize);
	return 1;
}



/***************************************************************************
    HANDY UTILITIES
***************************************************************************/

/*-------------------------------------------------
    cdrom_get_track - get the track number
    for a physical frame number
-------------------------------------------------*/

UINT32 cdrom_get_track(cdrom_file *file, UINT32 frame)
{
	UINT32 track = 0;

	if (file == NULL)
		return ~0;

	/* convert to a CHD sector offset and get track information */
	physical_to_chd_lba(file, frame, &track);
	return track;
}


/*-------------------------------------------------
    cdrom_get_track_start - get the frame number
    that a track starts at
-------------------------------------------------*/

UINT32 cdrom_get_track_start(cdrom_file *file, UINT32 track)
{
	if (file == NULL)
		return ~0;

	/* handle lead-out specially */
	if (track == 0xaa)
		track = file->cdtoc.numtrks;

	return file->cdtoc.tracks[track].physframeofs;
}



/***************************************************************************
    TOC UTILITIES
***************************************************************************/

/*-------------------------------------------------
    cdrom_get_last_track - returns the last track
    number
-------------------------------------------------*/

int cdrom_get_last_track(cdrom_file *file)
{
	if (file == NULL)
		return -1;

	return file->cdtoc.numtrks;
}


/*-------------------------------------------------
    cdrom_get_adr_control - get the ADR | CONTROL
    for a track
-------------------------------------------------*/

int cdrom_get_adr_control(cdrom_file *file, int track)
{
	if (file == NULL)
		return -1;

	if (track == 0xaa || file->cdtoc.tracks[track].trktype == CD_TRACK_AUDIO)
	{
		return 0x10;	// audio track, subchannel is position
	}

	return 0x14;	// data track, subchannel is position
}


/*-------------------------------------------------
    cdrom_get_track_type - return the track type
-------------------------------------------------*/

int cdrom_get_track_type(cdrom_file *file, int track)
{
	if (file == NULL)
		return -1;

	return file->cdtoc.tracks[track].trktype;
}


/*-------------------------------------------------
    cdrom_get_toc - return the TOC data for a
    CD-ROM
-------------------------------------------------*/

const cdrom_toc *cdrom_get_toc(cdrom_file *file)
{
	if (file == NULL)
		return NULL;

	return &file->cdtoc;
}



/***************************************************************************
    EXTRA UTILITIES
***************************************************************************/

/*-------------------------------------------------
    cdrom_convert_type_string_to_track_info -
    take a string and convert it into track type
    and track data size
-------------------------------------------------*/

void cdrom_convert_type_string_to_track_info(const char *typestring, cdrom_track_info *info)
{
	if (!strcmp(typestring, "MODE1"))
	{
		info->trktype = CD_TRACK_MODE1;
		info->datasize = 2048;
	}
	else if (!strcmp(typestring, "MODE1_RAW"))
	{
		info->trktype = CD_TRACK_MODE1_RAW;
		info->datasize = 2352;
	}
	else if (!strcmp(typestring, "MODE2"))
	{
		info->trktype = CD_TRACK_MODE2;
		info->datasize = 2336;
	}
	else if (!strcmp(typestring, "MODE2_FORM1"))
	{
		info->trktype = CD_TRACK_MODE2_FORM1;
		info->datasize = 2048;
	}
	else if (!strcmp(typestring, "MODE2_FORM2"))
	{
		info->trktype = CD_TRACK_MODE2_FORM2;
		info->datasize = 2324;
	}
	else if (!strcmp(typestring, "MODE2_FORM_MIX"))
	{
		info->trktype = CD_TRACK_MODE2_FORM_MIX;
		info->datasize = 2336;
	}
	else if (!strcmp(typestring, "MODE2_RAW"))
	{
		info->trktype = CD_TRACK_MODE2_RAW;
		info->datasize = 2352;
	}
	else if (!strcmp(typestring, "AUDIO"))
	{
		info->trktype = CD_TRACK_AUDIO;
		info->datasize = 2352;
	}
}


/*-------------------------------------------------
    cdrom_convert_subtype_string_to_track_info -
    take a string and convert it into track subtype
    and track subcode data size
-------------------------------------------------*/

void cdrom_convert_subtype_string_to_track_info(const char *typestring, cdrom_track_info *info)
{
	if (!strcmp(typestring, "RW"))
	{
		info->subtype = CD_SUB_NORMAL;
		info->subsize = 96;
	}
	else if (!strcmp(typestring, "RW_RAW"))
	{
		info->subtype = CD_SUB_RAW;
		info->subsize = 96;
	}
}


/*-------------------------------------------------
    cdrom_get_type_string - get the string
    associated with the given type
-------------------------------------------------*/

const char *cdrom_get_type_string(const cdrom_track_info *info)
{
	switch (info->trktype)
	{
		case CD_TRACK_MODE1:			return "MODE1";
		case CD_TRACK_MODE1_RAW:		return "MODE1_RAW";
		case CD_TRACK_MODE2:			return "MODE2";
		case CD_TRACK_MODE2_FORM1:		return "MODE2_FORM1";
		case CD_TRACK_MODE2_FORM2:		return "MODE2_FORM2";
		case CD_TRACK_MODE2_FORM_MIX:	return "MODE2_FORM_MIX";
		case CD_TRACK_MODE2_RAW:		return "MODE2_RAW";
		case CD_TRACK_AUDIO:			return "AUDIO";
		default:						return "UNKNOWN";
	}
}


/*-------------------------------------------------
    cdrom_get_subtype_string - get the string
    associated with the given subcode type
-------------------------------------------------*/

const char *cdrom_get_subtype_string(const cdrom_track_info *info)
{
	switch (info->subtype)
	{
		case CD_SUB_NORMAL:				return "RW";
		case CD_SUB_RAW:				return "RW_RAW";
		default:						return "NONE";
	}
}



/***************************************************************************
    INTERNAL UTILITIES
***************************************************************************/

/*-------------------------------------------------
    read_sector_into_cache - cache a sector at
    the given physical LBA
-------------------------------------------------*/

static chd_error read_sector_into_cache(cdrom_file *file, UINT32 lbasector, UINT32 *sectoroffs, UINT32 *tracknum)
{
	UINT32 chdsector, hunknum;
	chd_error err;

	/* convert to a CHD sector offset and get track information */
	*tracknum = 0;
	chdsector = physical_to_chd_lba(file, lbasector, tracknum);
	hunknum = chdsector / file->hunksectors;
	*sectoroffs = chdsector % file->hunksectors;

	/* if we haven't cached this hunk, read it now */
	if (file->cachehunk != hunknum)
	{
		err = chd_read(file->chd, hunknum, file->cache);
		if (err != CHDERR_NONE)
			return err;
		file->cachehunk = hunknum;
	}
	return CHDERR_NONE;
}


/*-------------------------------------------------
    parse_metadata - parse metadata into the TOC
    structure
-------------------------------------------------*/

static chd_error parse_metadata(chd_file *chd, cdrom_toc *toc)
{
	static UINT32 oldmetadata[CD_METADATA_WORDS], *mrp;
	const chd_header *header = chd_get_header(chd);
	UINT32 hunksectors = header->hunkbytes / CD_FRAME_SIZE;
	char metadata[256];
	chd_error err;
	int i;

	/* start with no tracks */
	for (toc->numtrks = 0; toc->numtrks < CD_MAX_TRACKS; toc->numtrks++)
	{
		cdrom_track_info *track = &toc->tracks[toc->numtrks];
		int tracknum = -1, frames = 0, hunks;
		char type[11], subtype[11];

		/* fetch the metadata for this track */
		err = chd_get_metadata(chd, CDROM_TRACK_METADATA_TAG, toc->numtrks, metadata, sizeof(metadata), NULL, NULL);
		if (err != CHDERR_NONE)
			break;

		/* parse the metadata */
		type[0] = subtype[0] = 0;
		if (sscanf(metadata, CDROM_TRACK_METADATA_FORMAT, &tracknum, type, subtype, &frames) != 4)
			return CHDERR_INVALID_DATA;
		if (tracknum != toc->numtrks + 1)
			return CHDERR_INVALID_DATA;

		/* extract the track type and determine the data size */
		track->trktype = CD_TRACK_MODE1;
		track->datasize = 0;
		cdrom_convert_type_string_to_track_info(type, track);
		if (track->datasize == 0)
			return CHDERR_INVALID_DATA;

		/* extract the subtype and determine the subcode data size */
		track->subtype = CD_SUB_NONE;
		track->subsize = 0;
		cdrom_convert_subtype_string_to_track_info(subtype, track);

		/* set the frames and extra frames data */
		track->frames = frames;
		hunks = (frames + hunksectors - 1) / hunksectors;
		track->extraframes = hunks * hunksectors - frames;
	}

	/* if we got any tracks this way, we're done */
	if (toc->numtrks > 0)
		return CHDERR_NONE;

	/* look for old-style metadata */
	err = chd_get_metadata(chd, CDROM_OLD_METADATA_TAG, 0, oldmetadata, sizeof(oldmetadata), NULL, NULL);
	if (err != CHDERR_NONE)
		return err;

	/* reconstruct the TOC from it */
	mrp = &oldmetadata[0];
	toc->numtrks = *mrp++;

	for (i = 0; i < CD_MAX_TRACKS; i++)
	{
		toc->tracks[i].trktype = *mrp++;
		toc->tracks[i].subtype = *mrp++;
		toc->tracks[i].datasize = *mrp++;
		toc->tracks[i].subsize = *mrp++;
		toc->tracks[i].frames = *mrp++;
		toc->tracks[i].extraframes = *mrp++;
	}

	/* TODO: I don't know why sometimes the data is one endian and sometimes another */
	if ((toc->numtrks < 0) || (toc->numtrks > CD_MAX_TRACKS))
	{
		toc->numtrks = FLIPENDIAN_INT32(toc->numtrks);
		for (i = 0; i < CD_MAX_TRACKS; i++)
		{
			toc->tracks[i].trktype = FLIPENDIAN_INT32(toc->tracks[i].trktype);
			toc->tracks[i].subtype = FLIPENDIAN_INT32(toc->tracks[i].subtype);
			toc->tracks[i].datasize = FLIPENDIAN_INT32(toc->tracks[i].datasize);
			toc->tracks[i].subsize = FLIPENDIAN_INT32(toc->tracks[i].subsize);
			toc->tracks[i].frames = FLIPENDIAN_INT32(toc->tracks[i].frames);
			toc->tracks[i].extraframes = FLIPENDIAN_INT32(toc->tracks[i].extraframes);
		}
	}

	return CHDERR_NONE;
}
