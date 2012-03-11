/***************************************************************************

    cdrom.c

    Generic MAME CD-ROM utilties - build IDE and SCSI CD-ROMs on top of this

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

****************************************************************************

    IMPORTANT:
    "physical" block addresses are the actual addresses on the emulated CD.
    "chd" block addresses are the block addresses in the CHD file.
    Because we pad each track to a 4-frame boundry, these addressing
    schemes will differ after track 1!

***************************************************************************/

#include "cdrom.h"

#include <stdlib.h>
#include "chdcd.h"


/***************************************************************************
    DEBUGGING
***************************************************************************/

#define VERBOSE	(0)
#if VERBOSE
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)
void CLIB_DECL logerror(const char *text,...);
#else
#define LOG(x)
#endif




/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct _cdrom_file
{
	chd_file *			chd;				/* CHD file */
	cdrom_toc			cdtoc;				/* TOC for the CD */
	chdcd_track_input_info track_info;		/* track info */
	core_file *			fhandle[CD_MAX_TRACKS];/* file handle */
};



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    physical_to_chd_lba - find the CHD LBA
    and the track number
-------------------------------------------------*/

INLINE UINT32 physical_to_chd_lba(cdrom_file *file, UINT32 physlba, UINT32 &tracknum)
{
	UINT32 chdlba;
	int track;

	/* loop until our current LBA is less than the start LBA of the next track */
	for (track = 0; track < file->cdtoc.numtrks; track++)
		if (physlba < file->cdtoc.tracks[track + 1].physframeofs)
		{
			chdlba = physlba - file->cdtoc.tracks[track].physframeofs + file->cdtoc.tracks[track].chdframeofs;
			tracknum = track;
			return chdlba;
		}

	return physlba;
}



/***************************************************************************
    BASE FUNCTIONALITY
***************************************************************************/

cdrom_file *cdrom_open(const char *inputfile)
{
	int i;
	cdrom_file *file;
	UINT32 physofs;

	/* allocate memory for the CD-ROM file */
	file = new cdrom_file();
	if (file == NULL)
		return NULL;

	/* setup the CDROM module and get the disc info */
	chd_error err = chdcd_parse_toc(inputfile, file->cdtoc, file->track_info);
	if (err != CHDERR_NONE)
	{
		fprintf(stderr, "Error reading input file: %s\n", chd_file::error_string(err));
		return NULL;
	}

	/* fill in the data */
	file->chd = NULL;

	LOG(("CD has %d tracks\n", file->cdtoc.numtrks));

	for (i = 0; i < file->cdtoc.numtrks; i++)
	{
		file_error filerr = core_fopen(file->track_info.track[i].fname, OPEN_FLAG_READ, &file->fhandle[i]);
		if (filerr != FILERR_NONE)
		{
			fprintf(stderr, "Unable to open file: %s\n", file->track_info.track[i].fname.cstr());
			return NULL;
		}
	}
	/* calculate the starting frame for each track, keeping in mind that CHDMAN
       pads tracks out with extra frames to fit 4-frame size boundries
    */
	physofs = 0;
	for (i = 0; i < file->cdtoc.numtrks; i++)
	{
		file->cdtoc.tracks[i].physframeofs = physofs;
		file->cdtoc.tracks[i].chdframeofs = 0;

		physofs += file->cdtoc.tracks[i].frames;

		LOG(("Track %02d is format %d subtype %d datasize %d subsize %d frames %d extraframes %d physofs %d chdofs %d\n", i+1,
			file->cdtoc.tracks[i].trktype,
			file->cdtoc.tracks[i].subtype,
			file->cdtoc.tracks[i].datasize,
			file->cdtoc.tracks[i].subsize,
			file->cdtoc.tracks[i].frames,
			file->cdtoc.tracks[i].extraframes,
			file->cdtoc.tracks[i].physframeofs,
			file->cdtoc.tracks[i].chdframeofs));
	}

	/* fill out dummy entries for the last track to help our search */
	file->cdtoc.tracks[i].physframeofs = physofs;
	file->cdtoc.tracks[i].chdframeofs = 0;

	return file;
}

/*-------------------------------------------------
    cdrom_open - "open" a CD-ROM file from an
    already-opened CHD file
-------------------------------------------------*/

cdrom_file *cdrom_open(chd_file *chd)
{
	int i;
	cdrom_file *file;
	UINT32 physofs, chdofs;
	chd_error err;

	/* punt if no CHD */
	if (!chd)
		return NULL;

	/* validate the CHD information */
	if (chd->hunk_bytes() % CD_FRAME_SIZE != 0)
		return NULL;
	if (chd->unit_bytes() != CD_FRAME_SIZE)
		return NULL;

	/* allocate memory for the CD-ROM file */
	file = (cdrom_file *)malloc(sizeof(cdrom_file));
	if (file == NULL)
		return NULL;

	/* fill in the data */
	file->chd = chd;

	/* read the CD-ROM metadata */
	err = cdrom_parse_metadata(chd, &file->cdtoc);
	if (err != CHDERR_NONE)
	{
		delete(file);
		return NULL;
	}

	LOG(("CD has %d tracks\n", file->cdtoc.numtrks));

	/* calculate the starting frame for each track, keeping in mind that CHDMAN
       pads tracks out with extra frames to fit 4-frame size boundries
    */
	physofs = chdofs = 0;
	for (i = 0; i < file->cdtoc.numtrks; i++)
	{
		file->cdtoc.tracks[i].physframeofs = physofs;
		file->cdtoc.tracks[i].chdframeofs = chdofs;

		physofs += file->cdtoc.tracks[i].frames;
		chdofs  += file->cdtoc.tracks[i].frames;
		chdofs  += file->cdtoc.tracks[i].extraframes;

		LOG(("Track %02d is format %d subtype %d datasize %d subsize %d frames %d extraframes %d physofs %d chdofs %d\n", i+1,
			file->cdtoc.tracks[i].trktype,
			file->cdtoc.tracks[i].subtype,
			file->cdtoc.tracks[i].datasize,
			file->cdtoc.tracks[i].subsize,
			file->cdtoc.tracks[i].frames,
			file->cdtoc.tracks[i].extraframes,
			file->cdtoc.tracks[i].physframeofs,
			file->cdtoc.tracks[i].chdframeofs));
	}

	/* fill out dummy entries for the last track to help our search */
	file->cdtoc.tracks[i].physframeofs = physofs;
	file->cdtoc.tracks[i].chdframeofs = chdofs;

	return file;
}


/*-------------------------------------------------
    cdrom_close - "close" a CD-ROM file
-------------------------------------------------*/

void cdrom_close(cdrom_file *file)
{
	if (file == NULL)
		return;

	if (file->chd == NULL)
	{
		for (int i = 0; i < file->cdtoc.numtrks; i++)
		{
			core_fclose(file->fhandle[i]);
		}
	}

	delete(file);
}



/***************************************************************************
    CORE READ ACCESS
***************************************************************************/

chd_error read_partial_sector(cdrom_file *file, void *dest, UINT32 chdsector, UINT32 tracknum, UINT32 startoffs, UINT32 length)
{
	// if a CHD, just read
	if (file->chd != NULL)
		return file->chd->read_bytes(UINT64(chdsector) * UINT64(CD_FRAME_SIZE) + startoffs, dest, length);

	// else read from the appropriate file
	core_file *srcfile = file->fhandle[tracknum];

	UINT64 sourcefileoffset = file->track_info.track[tracknum].offset;
	int bytespersector = file->cdtoc.tracks[tracknum].datasize + file->cdtoc.tracks[tracknum].subsize;

	sourcefileoffset += chdsector * bytespersector + startoffs;

	core_fseek(srcfile, sourcefileoffset, SEEK_SET);
	core_fread(srcfile, dest, length);

	if (file->track_info.track[tracknum].swap)
	{
		UINT8 *buffer = (UINT8 *)dest - startoffs;
		for (int swapindex = startoffs; swapindex < 2352; swapindex += 2 )
		{
			int swaptemp = buffer[ swapindex ];
			buffer[ swapindex ] = buffer[ swapindex + 1 ];
			buffer[ swapindex + 1 ] = swaptemp;
		}
	}
	return CHDERR_NONE;
}


/*-------------------------------------------------
    cdrom_read_data - read one or more sectors
    from a CD-ROM
-------------------------------------------------*/

UINT32 cdrom_read_data(cdrom_file *file, UINT32 lbasector, void *buffer, UINT32 datatype)
{
	if (file == NULL)
		return 0;

	// compute CHD sector and tracknumber
	UINT32 tracknum = 0;
	UINT32 chdsector = physical_to_chd_lba(file, lbasector, tracknum);

	/* copy out the requested sector */
	UINT32 tracktype = file->cdtoc.tracks[tracknum].trktype;
	if ((datatype == tracktype) || (datatype == CD_TRACK_RAW_DONTCARE))
	{
		return (read_partial_sector(file, buffer, chdsector, tracknum, 0, file->cdtoc.tracks[tracknum].datasize) == CHDERR_NONE);
	}
	else
	{
		/* return 2048 bytes of mode 1 data from a 2352 byte mode 1 raw sector */
		if ((datatype == CD_TRACK_MODE1) && (tracktype == CD_TRACK_MODE1_RAW))
		{
			return (read_partial_sector(file, buffer, chdsector, tracknum, 16, 2048) == CHDERR_NONE);
		}

		/* return 2352 byte mode 1 raw sector from 2048 bytes of mode 1 data */
		if ((datatype == CD_TRACK_MODE1_RAW) && (tracktype == CD_TRACK_MODE1))
		{
			UINT8 *bufptr = (UINT8 *)buffer;
			UINT32 msf = lba_to_msf(lbasector);

			static const UINT8 syncbytes[12] = {0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00};
			memcpy(bufptr, syncbytes, 12);
			bufptr[12] = msf>>16;
			bufptr[13] = msf>>8;
			bufptr[14] = msf&0xff;
			bufptr[15] = 1;	// mode 1
			LOG(("CDROM: promotion of mode1/form1 sector to mode1 raw is not complete!\n"));
			return (read_partial_sector(file, bufptr+16, chdsector, tracknum, 0, 2048) == CHDERR_NONE);
		}

		/* return 2048 bytes of mode 1 data from a mode2 form1 or raw sector */
		if ((datatype == CD_TRACK_MODE1) && ((tracktype == CD_TRACK_MODE2_FORM1)||(tracktype == CD_TRACK_MODE2_RAW)))
		{
			return (read_partial_sector(file, buffer, chdsector, tracknum, 24, 2048) == CHDERR_NONE);
		}

		/* return mode 2 2336 byte data from a 2352 byte mode 1 or 2 raw sector (skip the header) */
		if ((datatype == CD_TRACK_MODE2) && ((tracktype == CD_TRACK_MODE1_RAW) || (tracktype == CD_TRACK_MODE2_RAW)))
		{
			return (read_partial_sector(file, buffer, chdsector, tracknum, 16, 2336) == CHDERR_NONE);
		}

		LOG(("CDROM: Conversion from type %d to type %d not supported!\n", tracktype, datatype));
		return 0;
	}
}


/*-------------------------------------------------
    cdrom_read_subcode - read subcode data for
    a sector
-------------------------------------------------*/

UINT32 cdrom_read_subcode(cdrom_file *file, UINT32 lbasector, void *buffer)
{
	if (file == NULL)
		return ~0;

	// compute CHD sector and tracknumber
	UINT32 tracknum = 0;
	UINT32 chdsector = physical_to_chd_lba(file, lbasector, tracknum);
	if (file->cdtoc.tracks[tracknum].subsize == 0)
		return 1;

	// read the data
	chd_error err = read_partial_sector(file, buffer, chdsector, tracknum, file->cdtoc.tracks[tracknum].datasize, file->cdtoc.tracks[tracknum].subsize);
	return (err == CHDERR_NONE);
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
	physical_to_chd_lba(file, frame, track);
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
    cdrom_get_info_from_type_string
    take a string and convert it into track type
    and track data size
-------------------------------------------------*/

static void cdrom_get_info_from_type_string(const char *typestring, UINT32 *trktype, UINT32 *datasize)
{
	if (!strcmp(typestring, "MODE1"))
	{
		*trktype = CD_TRACK_MODE1;
		*datasize = 2048;
	}
	else if (!strcmp(typestring, "MODE1/2048"))
	{
		*trktype = CD_TRACK_MODE1;
		*datasize = 2048;
	}
	else if (!strcmp(typestring, "MODE1_RAW"))
	{
		*trktype = CD_TRACK_MODE1_RAW;
		*datasize = 2352;
	}
	else if (!strcmp(typestring, "MODE1/2352"))
	{
		*trktype = CD_TRACK_MODE1_RAW;
		*datasize = 2352;
	}
	else if (!strcmp(typestring, "MODE2"))
	{
		*trktype = CD_TRACK_MODE2;
		*datasize = 2336;
	}
	else if (!strcmp(typestring, "MODE2/2336"))
	{
		*trktype = CD_TRACK_MODE2;
		*datasize = 2336;
	}
	else if (!strcmp(typestring, "MODE2_FORM1"))
	{
		*trktype = CD_TRACK_MODE2_FORM1;
		*datasize = 2048;
	}
	else if (!strcmp(typestring, "MODE2/2048"))
	{
		*trktype = CD_TRACK_MODE2_FORM1;
		*datasize = 2048;
	}
	else if (!strcmp(typestring, "MODE2_FORM2"))
	{
		*trktype = CD_TRACK_MODE2_FORM2;
		*datasize = 2324;
	}
	else if (!strcmp(typestring, "MODE2/2324"))
	{
		*trktype = CD_TRACK_MODE2_FORM2;
		*datasize = 2324;
	}
	else if (!strcmp(typestring, "MODE2_FORM_MIX"))
	{
		*trktype = CD_TRACK_MODE2_FORM_MIX;
		*datasize = 2336;
	}
	else if (!strcmp(typestring, "MODE2/2336"))
	{
		*trktype = CD_TRACK_MODE2_FORM_MIX;
		*datasize = 2336;
	}
	else if (!strcmp(typestring, "MODE2_RAW"))
	{
		*trktype = CD_TRACK_MODE2_RAW;
		*datasize = 2352;
	}
	else if (!strcmp(typestring, "MODE2/2352"))
	{
		*trktype = CD_TRACK_MODE2_RAW;
		*datasize = 2352;
	}
	else if (!strcmp(typestring, "AUDIO"))
	{
		*trktype = CD_TRACK_AUDIO;
		*datasize = 2352;
	}
}

/*-------------------------------------------------
    cdrom_convert_type_string_to_track_info -
    take a string and convert it into track type
    and track data size
-------------------------------------------------*/

void cdrom_convert_type_string_to_track_info(const char *typestring, cdrom_track_info *info)
{
	cdrom_get_info_from_type_string(typestring, &info->trktype, &info->datasize);
}

/*-------------------------------------------------
    cdrom_convert_type_string_to_pregap_info -
    take a string and convert it into pregap type
    and pregap data size
-------------------------------------------------*/

void cdrom_convert_type_string_to_pregap_info(const char *typestring, cdrom_track_info *info)
{
	cdrom_get_info_from_type_string(typestring, &info->pgtype, &info->pgdatasize);
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
    cdrom_convert_subtype_string_to_pregap_info -
    take a string and convert it into track subtype
    and track subcode data size
-------------------------------------------------*/

void cdrom_convert_subtype_string_to_pregap_info(const char *typestring, cdrom_track_info *info)
{
	if (!strcmp(typestring, "RW"))
	{
		info->pgsub = CD_SUB_NORMAL;
		info->pgsubsize = 96;
	}
	else if (!strcmp(typestring, "RW_RAW"))
	{
		info->pgsub = CD_SUB_RAW;
		info->pgsubsize = 96;
	}
}

/*-------------------------------------------------
    cdrom_get_type_string - get the string
    associated with the given type
-------------------------------------------------*/

const char *cdrom_get_type_string(UINT32 trktype)
{
	switch (trktype)
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

const char *cdrom_get_subtype_string(UINT32 subtype)
{
	switch (subtype)
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
    cdrom_parse_metadata - parse metadata into the
    TOC structure
-------------------------------------------------*/

chd_error cdrom_parse_metadata(chd_file *chd, cdrom_toc *toc)
{
	astring metadata;
	chd_error err;
	int i;

	/* start with no tracks */
	for (toc->numtrks = 0; toc->numtrks < CD_MAX_TRACKS; toc->numtrks++)
	{
		int tracknum = -1, frames = 0, pregap, postgap, padframes;
		char type[16], subtype[16], pgtype[16], pgsub[16];
		cdrom_track_info *track;

		pregap = postgap = padframes = 0;

		/* fetch the metadata for this track */
		err = chd->read_metadata(CDROM_TRACK_METADATA_TAG, toc->numtrks, metadata);
		if (err == CHDERR_NONE)
		{
			/* parse the metadata */
			type[0] = subtype[0] = 0;
			if (sscanf(metadata, CDROM_TRACK_METADATA_FORMAT, &tracknum, type, subtype, &frames) != 4)
				return CHDERR_INVALID_DATA;
			if (tracknum == 0 || tracknum > CD_MAX_TRACKS)
				return CHDERR_INVALID_DATA;
			track = &toc->tracks[tracknum - 1];
		}
		else
		{
			err = chd->read_metadata(CDROM_TRACK_METADATA2_TAG, toc->numtrks, metadata);
			if (err == CHDERR_NONE)
            {
                /* parse the metadata */
                type[0] = subtype[0] = 0;
                pregap = postgap = 0;
                if (sscanf(metadata, CDROM_TRACK_METADATA2_FORMAT, &tracknum, type, subtype, &frames, &pregap, pgtype, pgsub, &postgap) != 8)
                    return CHDERR_INVALID_DATA;
                if (tracknum == 0 || tracknum > CD_MAX_TRACKS)
                    return CHDERR_INVALID_DATA;
                track = &toc->tracks[tracknum - 1];
            }
            else
            {
                err = chd->read_metadata(GDROM_TRACK_METADATA_TAG, toc->numtrks, metadata);

                if (err == CHDERR_NONE)
                {
                    /* parse the metadata */
                    type[0] = subtype[0] = 0;
                    pregap = postgap = 0;
                    if (sscanf(metadata, GDROM_TRACK_METADATA_FORMAT, &tracknum, type, subtype, &frames, &padframes, &pregap, pgtype, pgsub, &postgap) != 9)
                        return CHDERR_INVALID_DATA;
                    if (tracknum == 0 || tracknum > CD_MAX_TRACKS)
                        return CHDERR_INVALID_DATA;
                    track = &toc->tracks[tracknum - 1];
                }
                else
                {
                    break;
                }
            }
		}

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
        track->padframes = padframes;
		int padded = (frames + CD_TRACK_PADDING - 1) / CD_TRACK_PADDING;
		track->extraframes = padded * CD_TRACK_PADDING - frames;

		/* set the pregap info */
		track->pregap = pregap;
		track->pgtype = CD_TRACK_MODE1;
		track->pgsub = CD_SUB_NONE;
		track->pgdatasize = 0;
		track->pgsubsize = 0;
		cdrom_convert_type_string_to_pregap_info(pgtype, track);
		cdrom_convert_subtype_string_to_pregap_info(pgsub, track);

        /* set the postgap info */
        track->postgap = postgap;
	}

	/* if we got any tracks this way, we're done */
	if (toc->numtrks > 0)
		return CHDERR_NONE;

    printf("toc->numtrks = %d?!\n", toc->numtrks);

	/* look for old-style metadata */
	dynamic_buffer oldmetadata;
	err = chd->read_metadata(CDROM_OLD_METADATA_TAG, 0, oldmetadata);
	if (err != CHDERR_NONE)
		return err;

	/* reconstruct the TOC from it */
	UINT32 *mrp = reinterpret_cast<UINT32 *>(&oldmetadata[0]);
	toc->numtrks = *mrp++;

	for (i = 0; i < CD_MAX_TRACKS; i++)
	{
		toc->tracks[i].trktype = *mrp++;
		toc->tracks[i].subtype = *mrp++;
		toc->tracks[i].datasize = *mrp++;
		toc->tracks[i].subsize = *mrp++;
		toc->tracks[i].frames = *mrp++;
		toc->tracks[i].extraframes = *mrp++;
		toc->tracks[i].pregap = 0;
		toc->tracks[i].postgap = 0;
		toc->tracks[i].pgtype = 0;
		toc->tracks[i].pgsub = 0;
		toc->tracks[i].pgdatasize = 0;
		toc->tracks[i].pgsubsize = 0;
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
			toc->tracks[i].padframes = FLIPENDIAN_INT32(toc->tracks[i].padframes);
			toc->tracks[i].extraframes = FLIPENDIAN_INT32(toc->tracks[i].extraframes);
		}
	}

	return CHDERR_NONE;
}


/*-------------------------------------------------
    cdrom_write_metadata - write metadata
-------------------------------------------------*/

chd_error cdrom_write_metadata(chd_file *chd, const cdrom_toc *toc)
{
	chd_error err;
	int i;

	/* write the metadata */
	for (i = 0; i < toc->numtrks; i++)
	{
        astring metadata;
        if (!(toc->flags & CD_FLAG_GDROM))
        {
            metadata.format(CDROM_TRACK_METADATA2_FORMAT, i + 1, cdrom_get_type_string(toc->tracks[i].trktype),
                    cdrom_get_subtype_string(toc->tracks[i].subtype), toc->tracks[i].frames, toc->tracks[i].pregap,
                    cdrom_get_type_string(toc->tracks[i].pgtype), cdrom_get_subtype_string(toc->tracks[i].pgsub),
                    toc->tracks[i].postgap);

            err = chd->write_metadata(CDROM_TRACK_METADATA2_TAG, i, metadata);
        }
        else
        {
            metadata.format(GDROM_TRACK_METADATA_FORMAT, i + 1, cdrom_get_type_string(toc->tracks[i].trktype),
                    cdrom_get_subtype_string(toc->tracks[i].subtype), toc->tracks[i].frames, toc->tracks[i].padframes,
                    toc->tracks[i].pregap, cdrom_get_type_string(toc->tracks[i].pgtype),
                    cdrom_get_subtype_string(toc->tracks[i].pgsub), toc->tracks[i].postgap);

            err = chd->write_metadata(GDROM_TRACK_METADATA_TAG, i, metadata);
        }
		if (err != CHDERR_NONE)
			return err;
	}
	return CHDERR_NONE;
}
