// license:BSD-3-Clause
// copyright-holders:Aaron Giles,R. Belmont
/***************************************************************************

    cdrom.c

    Generic MAME CD-ROM utilties - build IDE and SCSI CD-ROMs on top of this

****************************************************************************

    IMPORTANT:
    "physical" block addresses are the actual addresses on the emulated CD.
    "chd" block addresses are the block addresses in the CHD file.
    Because we pad each track to a 4-frame boundary, these addressing
    schemes will differ after track 1!

***************************************************************************/

#include "cdrom.h"

#include "corestr.h"
#include "multibyte.h"
#include "osdfile.h"
#include "path.h"
#include "strformat.h"

#include <cassert>
#include <cstdlib>
#include <tuple>


/***************************************************************************
    DEBUGGING
***************************************************************************/

/** @brief  The verbose. */
#define VERBOSE (0)
#define EXTRA_VERBOSE (0)

/**
 * @def LOG(x) do
 *
 * @brief   A macro that defines log.
 *
 * @param   x   The void to process.
 */

#define LOG(x) do { if (VERBOSE) { osd_printf_info x; } } while (0)



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    physical_to_chd_lba - find the CHD LBA
    and the track number
-------------------------------------------------*/

/**
 * @fn  static inline uint32_t physical_to_chd_lba(uint32_t physlba, uint32_t &tracknum)
 *
 * @brief   Physical to chd lba.
 *
 * @param   physlba             The physlba.
 * @param [in,out]  tracknum    The tracknum.
 *
 * @return  An uint32_t.
 */

uint32_t cdrom_file::physical_to_chd_lba(uint32_t physlba, uint32_t &tracknum) const
{
	// loop until our current LBA is less than the start LBA of the next track
	for (int track = 0; track < cdtoc.numtrks; track++)
		if (physlba < cdtoc.tracks[track + 1].physframeofs)
		{
			uint32_t chdlba = physlba - cdtoc.tracks[track].physframeofs + cdtoc.tracks[track].chdframeofs;
			tracknum = track;
			return chdlba;
		}

	return physlba;
}

/*-------------------------------------------------
    logical_to_chd_lba - find the CHD LBA
    and the track number
-------------------------------------------------*/

/**
 * @fn  uint32_t logical_to_chd_lba(uint32_t loglba, uint32_t &tracknum)
 *
 * @brief   Logical to chd lba.
 *
 * @param   loglba              The loglba.
 * @param [in,out]  tracknum    The tracknum.
 *
 * @return  An uint32_t.
 */

uint32_t cdrom_file::logical_to_chd_lba(uint32_t loglba, uint32_t &tracknum) const
{
	// loop until our current LBA is less than the start LBA of the next track
	for (int track = 0; track < cdtoc.numtrks; track++)
	{
		if (loglba < cdtoc.tracks[track + 1].logframeofs)
		{
			// convert to physical and proceed
			uint32_t physlba = cdtoc.tracks[track].physframeofs + (loglba - cdtoc.tracks[track].logframeofs);
			uint32_t chdlba = physlba - cdtoc.tracks[track].physframeofs + cdtoc.tracks[track].chdframeofs;
			tracknum = track;
			return chdlba;
		}
	}

	return loglba;
}


/***************************************************************************
    BASE FUNCTIONALITY
***************************************************************************/

/**
 * @fn  constructor
 *
 * @brief   Open a cdrom for a file.
 *
 * @param   inputfile   The inputfile.
 */

cdrom_file::cdrom_file(std::string_view inputfile)
{
	// set up the CD-ROM module and get the disc info
	std::error_condition err = parse_toc(inputfile, cdtoc, cdtrack_info);
	if (err)
	{
		fprintf(stderr, "Error reading input file: %s\n", err.message().c_str());
		throw nullptr;
	}

	// fill in the data
	chd = nullptr;

	LOG(("CD has %d tracks\n", cdtoc.numtrks));

	for (int i = 0; i < cdtoc.numtrks; i++)
	{
		osd_file::ptr file;
		std::uint64_t length;
		std::error_condition const filerr = osd_file::open(cdtrack_info.track[i].fname, OPEN_FLAG_READ, file, length);
		if (filerr)
		{
			fprintf(stderr, "Unable to open file: %s\n", cdtrack_info.track[i].fname.c_str());
			throw nullptr;
		}
		fhandle[i] = util::osd_file_read(std::move(file));
		if (!fhandle[i])
		{
			fprintf(stderr, "Unable to open file: %s\n", cdtrack_info.track[i].fname.c_str());
			throw nullptr;
		}
	}

	/* calculate the starting frame for each track, keeping in mind that CHDMAN
	   pads tracks out with extra frames to fit 4-frame size boundries
	*/
	uint32_t physofs = 0, logofs = 0;
	for (int i = 0; i < cdtoc.numtrks; i++)
	{
		track_info &track = cdtoc.tracks[i];
		track.logframeofs = 0;

		if (track.pgdatasize == 0)
		{
			logofs += track.pregap;
		}
		else
		{
			track.logframeofs = track.pregap;
		}

		if ((cdtoc.flags & CD_FLAG_MULTISESSION) && (cdtrack_info.track[i].leadin != -1))
			logofs += cdtrack_info.track[i].leadin;

		track.physframeofs = physofs;
		track.chdframeofs = 0;
		track.logframeofs += logofs;
		track.logframes = track.frames - track.pregap;

		// postgap adds to the track length
		logofs += track.postgap;

		physofs += track.frames;
		logofs  += track.frames;

		if ((cdtoc.flags & CD_FLAG_MULTISESSION) && cdtrack_info.track[i].leadout != -1)
			logofs += cdtrack_info.track[i].leadout;

		if (EXTRA_VERBOSE)
		{
			printf("session %d track %02d is format %d subtype %d datasize %d subsize %d frames %d extraframes %d pregap %d pgmode %d presize %d postgap %d logofs %d physofs %d chdofs %d logframes %d pad %d\n",
					track.session + 1,
					i + 1,
					track.trktype,
					track.subtype,
					track.datasize,
					track.subsize,
					track.frames,
					track.extraframes,
					track.pregap,
					track.pgtype,
					track.pgdatasize,
					track.postgap,
					track.logframeofs,
					track.physframeofs,
					track.chdframeofs,
					track.logframes,
					track.padframes);
		}
	}

	// fill out dummy entries for the last track to help our search
	track_info &track = cdtoc.tracks[cdtoc.numtrks];
	track.physframeofs = physofs;
	track.logframeofs = logofs;
	track.chdframeofs = 0;
	track.logframes = 0;
}

/*-------------------------------------------------
    constructor - "open" a CD-ROM file from an
    already-opened CHD file
-------------------------------------------------*/

/**
 * @fn  cdrom_file *cdrom_open(chd_file *chd)
 *
 * @brief   Queries if a given cdrom open.
 *
 * @param [in,out]  chd If non-null, the chd.
 *
 * @return  null if it fails, else a cdrom_file*.
 */

cdrom_file::cdrom_file(chd_file *_chd)
{
	chd = _chd;

	/* validate the CHD information */
	if (chd->hunk_bytes() % FRAME_SIZE != 0)
		throw nullptr;
	if (chd->unit_bytes() != FRAME_SIZE)
		throw nullptr;

	/* read the CD-ROM metadata */
	std::error_condition err = parse_metadata(chd, cdtoc);
	if (err)
		throw nullptr;

	LOG(("CD has %d tracks\n", cdtoc.numtrks));

	/* calculate the starting frame for each track, keeping in mind that CHDMAN
	   pads tracks out with extra frames to fit 4-frame size boundries
	*/
	uint32_t physofs = 0, chdofs = 0, logofs = 0;
	for (int i = 0; i < cdtoc.numtrks; i++)
	{
		track_info &track = cdtoc.tracks[i];
		track.logframeofs = 0;

		if (track.pgdatasize == 0)
		{
			// Anything that isn't cue.
			// toc (cdrdao): Pregap data seems to be included at the end of previous track.
			// START/PREGAP is only issued in special cases, for instance alongside ZERO commands.
			// ZERO and SILENCE commands are supposed to generate additional data that's not included
			// in the image directly, so the total logofs value must be offset to point to index 1.
			logofs += track.pregap;
		}
		else
		{
			// cues: Pregap is the difference between index 0 and index 1 unless PREGAP is specified.
			// The data is assumed to be in the bin and not generated separately, so the pregap should
			// only be added to the current track's lba to offset it to index 1.
			track.logframeofs = track.pregap;
		}

		track.physframeofs = physofs;
		track.chdframeofs = chdofs;
		track.logframeofs += logofs;
		track.logframes = track.frames - track.pregap;

		// postgap counts against the next track
		logofs += track.postgap;

		physofs += track.frames;
		chdofs  += track.frames;
		chdofs  += track.extraframes;
		logofs  += track.frames;

		if (EXTRA_VERBOSE)
		{
			printf("session %d track %02d is format %d subtype %d datasize %d subsize %d frames %d extraframes %d pregap %d pgmode %d presize %d postgap %d logofs %d physofs %d chdofs %d logframes %d pad %d\n",
					track.session + 1,
					i + 1,
					track.trktype,
					track.subtype,
					track.datasize,
					track.subsize,
					track.frames,
					track.extraframes,
					track.pregap,
					track.pgtype,
					track.pgdatasize,
					track.postgap,
					track.logframeofs,
					track.physframeofs,
					track.chdframeofs,
					track.logframes,
					track.padframes);
		}
	}

	// fill out dummy entries for the last track to help our search
	track_info &track = cdtoc.tracks[cdtoc.numtrks];
	track.physframeofs = physofs;
	track.logframeofs = logofs;
	track.chdframeofs = chdofs;
	track.logframes = 0;
}


/*-------------------------------------------------
    destructor - "close" a CD-ROM file
-------------------------------------------------*/

cdrom_file::~cdrom_file()
{
	if (chd == nullptr)
	{
		for (int i = 0; i < cdtoc.numtrks; i++)
		{
			fhandle[i].reset();
		}
	}
}



/***************************************************************************
    CORE READ ACCESS
***************************************************************************/

/**
 * @fn  std::error_condition read_partial_sector(void *dest, uint32_t lbasector, uint32_t chdsector, uint32_t tracknum, uint32_t startoffs, uint32_t length, bool phys)
 *
 * @brief   Reads partial sector.
 *
 * @param [in,out]  dest    If non-null, destination for the.
 * @param   lbasector       The lbasector.
 * @param   chdsector       The chdsector.
 * @param   tracknum        The tracknum.
 * @param   startoffs       The startoffs.
 * @param   length          The length.
 * @param   phys            true to physical.
 *
 * @return  The partial sector.
 */

std::error_condition cdrom_file::read_partial_sector(void *dest, uint32_t lbasector, uint32_t chdsector, uint32_t tracknum, uint32_t startoffs, uint32_t length, bool phys)
{
	std::error_condition result;
	bool needswap = false;

	// if this is pregap info that isn't actually in the file, just return blank data
	if (!phys)
	{
		if ((cdtoc.tracks[tracknum].pgdatasize == 0) && (lbasector < cdtoc.tracks[tracknum].logframeofs))
		{
			if (EXTRA_VERBOSE)
				printf("PG missing sector: LBA %d, trklog %d\n", lbasector, cdtoc.tracks[tracknum].logframeofs);
			memset(dest, 0, length);
			return result;
		}
	}

	// if a CHD, just read
	if (chd != nullptr)
	{
		if (!phys && cdtoc.tracks[tracknum].pgdatasize != 0)
		{
			// chdman (phys=true) relies on chdframeofs to point to index 0 instead of index 1 for extractcd.
			// Actually playing CDs requires it to point to index 1 instead of index 0, so adjust the offset when phys=false.
			chdsector += cdtoc.tracks[tracknum].pregap;
		}

		result = chd->read_bytes(uint64_t(chdsector) * uint64_t(FRAME_SIZE) + startoffs, dest, length);

		// swap CDDA in the case of LE GDROMs
		if ((cdtoc.flags & CD_FLAG_GDROMLE) && (cdtoc.tracks[tracknum].trktype == CD_TRACK_AUDIO))
			needswap = true;
	}
	else
	{
		// else read from the appropriate file
		util::random_read &srcfile = *fhandle[tracknum];

		int bytespersector = cdtoc.tracks[tracknum].datasize + cdtoc.tracks[tracknum].subsize;
		uint64_t sourcefileoffset = cdtrack_info.track[tracknum].offset;

		if (cdtoc.tracks[tracknum].pgdatasize != 0)
			chdsector += cdtoc.tracks[tracknum].pregap;

		sourcefileoffset += chdsector * bytespersector + startoffs;

		if (EXTRA_VERBOSE)
			printf("Reading %u bytes from sector %d from track %d at offset %lu\n", (unsigned)length, chdsector, tracknum + 1, (unsigned long)sourcefileoffset);

		result = srcfile.seek(sourcefileoffset, SEEK_SET);
		size_t actual;
		if (!result)
			std::tie(result, actual) = read(srcfile, dest, length);
		// FIXME: if (!result && (actual < length)) report error

		needswap = cdtrack_info.track[tracknum].swap;
	}

	if (needswap)
	{
		uint8_t *buffer = (uint8_t *)dest - startoffs;
		for (int swapindex = startoffs; swapindex < 2352; swapindex += 2 )
		{
			using std::swap;
			swap(buffer[ swapindex ], buffer[ swapindex + 1 ]);
		}
	}
	return result;
}


/*-------------------------------------------------
    cdrom_read_data - read one or more sectors
    from a CD-ROM
-------------------------------------------------*/

/**
 * @fn  bool read_data(uint32_t lbasector, void *buffer, uint32_t datatype, bool phys)
 *
 * @brief   Cdrom read data.
 *
 * @param   lbasector       The lbasector.
 * @param [in,out]  buffer  If non-null, the buffer.
 * @param   datatype        The datatype.
 * @param   phys            true to physical.
 *
 * @return  An uint32_t.
 */

bool cdrom_file::read_data(uint32_t lbasector, void *buffer, uint32_t datatype, bool phys)
{
	// compute CHD sector and tracknumber
	uint32_t tracknum = 0;
	uint32_t chdsector;

	if (phys)
	{
		chdsector = physical_to_chd_lba(lbasector, tracknum);
	}
	else
	{
		chdsector = logical_to_chd_lba(lbasector, tracknum);
	}

	// copy out the requested sector
	uint32_t tracktype = cdtoc.tracks[tracknum].trktype;

	if ((datatype == tracktype) || (datatype == CD_TRACK_RAW_DONTCARE))
	{
		assert(cdtoc.tracks[tracknum].datasize != 0);
		return !read_partial_sector(buffer, lbasector, chdsector, tracknum, 0, cdtoc.tracks[tracknum].datasize, phys);
	}
	else
	{
		// return 2048 bytes of mode 1 data from a 2352 byte mode 1 raw sector
		if ((datatype == CD_TRACK_MODE1) && (tracktype == CD_TRACK_MODE1_RAW))
		{
			return !read_partial_sector(buffer, lbasector, chdsector, tracknum, 16, 2048, phys);
		}

		// return 2352 byte mode 1 raw sector from 2048 bytes of mode 1 data
		if ((datatype == CD_TRACK_MODE1_RAW) && (tracktype == CD_TRACK_MODE1))
		{
			auto *bufptr = (uint8_t *)buffer;
			uint32_t msf = lba_to_msf(lbasector);

			static const uint8_t syncbytes[12] = {0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00};
			memcpy(bufptr, syncbytes, 12);
			put_u24be(&bufptr[12], msf);
			bufptr[15] = 1; // mode 1
			LOG(("CDROM: promotion of mode1/form1 sector to mode1 raw is not complete!\n"));
			return !read_partial_sector(bufptr+16, lbasector, chdsector, tracknum, 0, 2048, phys);
		}

		// return 2048 bytes of mode 1 data from a mode2 form1 or raw sector
		if ((datatype == CD_TRACK_MODE1) && ((tracktype == CD_TRACK_MODE2_FORM1)||(tracktype == CD_TRACK_MODE2_RAW)))
		{
			return !read_partial_sector(buffer, lbasector, chdsector, tracknum, 24, 2048, phys);
		}

		// return 2048 bytes of mode 1 data from a mode2 form2 or XA sector
		if ((datatype == CD_TRACK_MODE1) && (tracktype == CD_TRACK_MODE2_FORM_MIX))
		{
			return !read_partial_sector(buffer, lbasector, chdsector, tracknum, 8, 2048, phys);
		}

		// return mode 2 2336 byte data from a 2352 byte mode 1 or 2 raw sector (skip the header)
		if ((datatype == CD_TRACK_MODE2) && ((tracktype == CD_TRACK_MODE1_RAW) || (tracktype == CD_TRACK_MODE2_RAW)))
		{
			return !read_partial_sector(buffer, lbasector, chdsector, tracknum, 16, 2336, phys);
		}

		LOG(("CDROM: Conversion from type %d to type %d not supported!\n", tracktype, datatype));
		return 0;
	}
}


/*-------------------------------------------------
    read_subcode - read subcode data for
    a sector
-------------------------------------------------*/

/**
 * @fn  bool read_subcode(uint32_t lbasector, void *buffer, bool phys)
 *
 * @brief   Cdrom read subcode.
 *
 * @param   lbasector       The lbasector.
 * @param [in,out]  buffer  If non-null, the buffer.
 * @param   phys            true to physical.
 *
 * @return  false on failure.
 */

bool cdrom_file::read_subcode(uint32_t lbasector, void *buffer, bool phys)
{
	// compute CHD sector and tracknumber
	uint32_t tracknum = 0;
	uint32_t chdsector;

	if (phys)
	{
		chdsector = physical_to_chd_lba(lbasector, tracknum);
	}
	else
	{
		chdsector = logical_to_chd_lba(lbasector, tracknum);
	}

	if (cdtoc.tracks[tracknum].subsize == 0)
		return false;

	// read the data
	std::error_condition err = read_partial_sector(buffer, lbasector, chdsector, tracknum, cdtoc.tracks[tracknum].datasize, cdtoc.tracks[tracknum].subsize, phys);
	return !err;
}



/***************************************************************************
    HANDY UTILITIES
***************************************************************************/

/*-------------------------------------------------
    get_track - get the track number
    for a physical frame number
-------------------------------------------------*/

/**
 * @fn  uint32_t get_track(uint32_t frame)
 *
 * @brief   Cdrom get track.
 *
 * @param   frame           The frame.
 *
 * @return  An uint32_t.
 */

uint32_t cdrom_file::get_track(uint32_t frame) const
{
	uint32_t track = 0;

	/* convert to a CHD sector offset and get track information */
	logical_to_chd_lba(frame, track);

	return track;
}

uint32_t cdrom_file::get_track_index(uint32_t frame) const
{
	const uint32_t track = get_track(frame);
	const uint32_t track_start = get_track_start(track);
	const uint32_t index_offset = frame - track_start;
	int index = 0;

	for (int i = 0; i < std::size(cdtrack_info.track[track].idx); i++)
	{
		if (index_offset >= cdtrack_info.track[track].idx[i])
			index = i;
		else
			break;
	}

	if (cdtrack_info.track[track].idx[index] == -1)
		index = 1; // valid index not found, default to index 1

	return index;
}


/***************************************************************************
    EXTRA UTILITIES
***************************************************************************/

/*-------------------------------------------------
    get_info_from_type_string
    take a string and convert it into track type
    and track data size
-------------------------------------------------*/

/**
 * @fn  static void get_info_from_type_string(const char *typestring, uint32_t *trktype, uint32_t *datasize)
 *
 * @brief   Cdrom get information from type string.
 *
 * @param   typestring          The typestring.
 * @param [in,out]  trktype     If non-null, the trktype.
 * @param [in,out]  datasize    If non-null, the datasize.
 */

void cdrom_file::get_info_from_type_string(const char *typestring, uint32_t *trktype, uint32_t *datasize)
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
	else if (!strcmp(typestring, "CDI/2352"))
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
    convert_type_string_to_track_info -
    take a string and convert it into track type
    and track data size
-------------------------------------------------*/

/**
 * @fn  void convert_type_string_to_track_info(const char *typestring, track_info *info)
 *
 * @brief   Convert type string to track information.
 *
 * @param   typestring      The typestring.
 * @param [in,out]  info    If non-null, the information.
 */

void cdrom_file::convert_type_string_to_track_info(const char *typestring, track_info *info)
{
	get_info_from_type_string(typestring, &info->trktype, &info->datasize);
}

/*-------------------------------------------------
    convert_type_string_to_pregap_info -
    take a string and convert it into pregap type
    and pregap data size
-------------------------------------------------*/

/**
 * @fn  void convert_type_string_to_pregap_info(const char *typestring, track_info *info)
 *
 * @brief   Convert type string to pregap information.
 *
 * @param   typestring      The typestring.
 * @param [in,out]  info    If non-null, the information.
 */

void cdrom_file::convert_type_string_to_pregap_info(const char *typestring, track_info *info)
{
	get_info_from_type_string(typestring, &info->pgtype, &info->pgdatasize);
}

/*-------------------------------------------------
    convert_subtype_string_to_track_info -
    take a string and convert it into track subtype
    and track subcode data size
-------------------------------------------------*/

/**
 * @fn  void convert_subtype_string_to_track_info(const char *typestring, track_info *info)
 *
 * @brief   Convert subtype string to track information.
 *
 * @param   typestring      The typestring.
 * @param [in,out]  info    If non-null, the information.
 */

void cdrom_file::convert_subtype_string_to_track_info(const char *typestring, track_info *info)
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
    convert_subtype_string_to_pregap_info -
    take a string and convert it into track subtype
    and track subcode data size
-------------------------------------------------*/

/**
 * @fn  void convert_subtype_string_to_pregap_info(const char *typestring, track_info *info)
 *
 * @brief   Convert subtype string to pregap information.
 *
 * @param   typestring      The typestring.
 * @param [in,out]  info    If non-null, the information.
 */

void cdrom_file::convert_subtype_string_to_pregap_info(const char *typestring, track_info *info)
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
    get_type_string - get the string
    associated with the given type
-------------------------------------------------*/

/**
 * @fn  const char *get_type_string(uint32_t trktype)
 *
 * @brief   Get type string.
 *
 * @param   trktype The trktype.
 *
 * @return  null if it fails, else a char*.
 */

const char *cdrom_file::get_type_string(uint32_t trktype)
{
	switch (trktype)
	{
		case CD_TRACK_MODE1:            return "MODE1";
		case CD_TRACK_MODE1_RAW:        return "MODE1_RAW";
		case CD_TRACK_MODE2:            return "MODE2";
		case CD_TRACK_MODE2_FORM1:      return "MODE2_FORM1";
		case CD_TRACK_MODE2_FORM2:      return "MODE2_FORM2";
		case CD_TRACK_MODE2_FORM_MIX:   return "MODE2_FORM_MIX";
		case CD_TRACK_MODE2_RAW:        return "MODE2_RAW";
		case CD_TRACK_AUDIO:            return "AUDIO";
		default:                        return "UNKNOWN";
	}
}


/*-------------------------------------------------
    get_subtype_string - get the string
    associated with the given subcode type
-------------------------------------------------*/

/**
 * @fn  const char *get_subtype_string(uint32_t subtype)
 *
 * @brief   Get subtype string.
 *
 * @param   subtype The subtype.
 *
 * @return  null if it fails, else a char*.
 */

const char *cdrom_file::get_subtype_string(uint32_t subtype)
{
	switch (subtype)
	{
		case CD_SUB_NORMAL:             return "RW";
		case CD_SUB_RAW:                return "RW_RAW";
		default:                        return "NONE";
	}
}



/***************************************************************************
    INTERNAL UTILITIES
***************************************************************************/

/*-------------------------------------------------
    parse_metadata - parse metadata into the
    TOC structure
-------------------------------------------------*/

/**
 * @fn  std::error_condition cdrom_parse_metadata(chd_file *chd, toc *toc)
 *
 * @brief   Parse metadata.
 *
 * @param [in,out]  chd If non-null, the chd.
 * @param [in,out]  toc If non-null, the TOC.
 *
 * @return  A std::error_condition.
 */

std::error_condition cdrom_file::parse_metadata(chd_file *chd, toc &toc)
{
	std::string metadata;
	std::error_condition err;

	/* clear structures */
	memset(&toc, 0, sizeof(toc));

	toc.numsessions = 1;

	/* start with no tracks */
	for (toc.numtrks = 0; toc.numtrks < MAX_TRACKS; toc.numtrks++)
	{
		int tracknum, frames, pregap, postgap, padframes;
		char type[16], subtype[16], pgtype[16], pgsub[16];
		track_info *track;

		tracknum = -1;
		frames = pregap = postgap = padframes = 0;
		std::fill(std::begin(type), std::end(type), 0);
		std::fill(std::begin(subtype), std::end(subtype), 0);
		std::fill(std::begin(pgtype), std::end(pgtype), 0);
		std::fill(std::begin(pgsub), std::end(pgsub), 0);

		// fetch the metadata for this track
		if (!chd->read_metadata(CDROM_TRACK_METADATA_TAG, toc.numtrks, metadata))
		{
			if (sscanf(metadata.c_str(), CDROM_TRACK_METADATA_FORMAT, &tracknum, type, subtype, &frames) != 4)
				return chd_file::error::INVALID_DATA;
		}
		else if (!chd->read_metadata(CDROM_TRACK_METADATA2_TAG, toc.numtrks, metadata))
		{
			if (sscanf(metadata.c_str(), CDROM_TRACK_METADATA2_FORMAT, &tracknum, type, subtype, &frames, &pregap, pgtype, pgsub, &postgap) != 8)
				return chd_file::error::INVALID_DATA;
		}
		else
		{
			// fall through to GD-ROM detection
			err = chd->read_metadata(GDROM_OLD_METADATA_TAG, toc.numtrks, metadata);
			if (!err)
				toc.flags |= CD_FLAG_GDROMLE; // legacy GDROM track was detected
			else
				err = chd->read_metadata(GDROM_TRACK_METADATA_TAG, toc.numtrks, metadata);

			if (err)
				break;

			if (sscanf(metadata.c_str(), GDROM_TRACK_METADATA_FORMAT, &tracknum, type, subtype, &frames, &padframes, &pregap, pgtype, pgsub, &postgap) != 9)
				return chd_file::error::INVALID_DATA;

			toc.flags |= CD_FLAG_GDROM;
		}

		if (tracknum == 0 || tracknum > MAX_TRACKS)
			return chd_file::error::INVALID_DATA;

		track = &toc.tracks[tracknum - 1];

		// extract the track type and determine the data size
		track->trktype = CD_TRACK_MODE1;
		track->datasize = 0;
		convert_type_string_to_track_info(type, track);
		if (track->datasize == 0)
			return chd_file::error::INVALID_DATA;

		// extract the subtype and determine the subcode data size
		track->subtype = CD_SUB_NONE;
		track->subsize = 0;
		convert_subtype_string_to_track_info(subtype, track);

		// set the frames and extra frames data
		track->frames = frames;
		track->padframes = padframes;
		int padded = (frames + TRACK_PADDING - 1) / TRACK_PADDING;
		track->extraframes = padded * TRACK_PADDING - frames;

		// set the pregap info
		track->pregap = pregap;
		track->pgtype = CD_TRACK_MODE1;
		track->pgsub = CD_SUB_NONE;
		track->pgdatasize = 0;
		track->pgsubsize = 0;
		if (track->pregap > 0)
		{
			if (pgtype[0] == 'V')
			{
				convert_type_string_to_pregap_info(&pgtype[1], track);
			}

			convert_subtype_string_to_pregap_info(pgsub, track);
		}

		/* set the postgap info */
		track->postgap = postgap;
	}

	/* if we got any tracks this way, we're done */
	if (toc.numtrks > 0)
		return std::error_condition();

	printf("toc.numtrks = %u?!\n", toc.numtrks);

	/* look for old-style metadata */
	std::vector<uint8_t> oldmetadata;
	err = chd->read_metadata(CDROM_OLD_METADATA_TAG, 0, oldmetadata);
	if (err)
		return err;

	/* reconstruct the TOC from it */
	auto *mrp = reinterpret_cast<uint32_t *>(&oldmetadata[0]);
	toc.numtrks = *mrp++;

	toc.numsessions = 1;

	for (int i = 0; i < MAX_TRACKS; i++)
	{
		toc.tracks[i].session = 0;
		toc.tracks[i].trktype = *mrp++;
		toc.tracks[i].subtype = *mrp++;
		toc.tracks[i].datasize = *mrp++;
		toc.tracks[i].subsize = *mrp++;
		toc.tracks[i].frames = *mrp++;
		toc.tracks[i].extraframes = *mrp++;
		toc.tracks[i].pregap = 0;
		toc.tracks[i].postgap = 0;
		toc.tracks[i].pgtype = 0;
		toc.tracks[i].pgsub = 0;
		toc.tracks[i].pgdatasize = 0;
		toc.tracks[i].pgsubsize = 0;
	}

	/* TODO: I don't know why sometimes the data is one endian and sometimes another */
	if (toc.numtrks > MAX_TRACKS)
	{
		toc.numtrks = swapendian_int32(toc.numtrks);
		for (int i = 0; i < MAX_TRACKS; i++)
		{
			toc.tracks[i].trktype = swapendian_int32(toc.tracks[i].trktype);
			toc.tracks[i].subtype = swapendian_int32(toc.tracks[i].subtype);
			toc.tracks[i].datasize = swapendian_int32(toc.tracks[i].datasize);
			toc.tracks[i].subsize = swapendian_int32(toc.tracks[i].subsize);
			toc.tracks[i].frames = swapendian_int32(toc.tracks[i].frames);
			toc.tracks[i].padframes = swapendian_int32(toc.tracks[i].padframes);
			toc.tracks[i].extraframes = swapendian_int32(toc.tracks[i].extraframes);
		}
	}

	return std::error_condition();
}


/*-------------------------------------------------
    write_metadata - write metadata
-------------------------------------------------*/

/**
 * @fn  std::error_condition write_metadata(chd_file *chd, const toc *toc)
 *
 * @brief   Write metadata.
 *
 * @param [in,out]  chd If non-null, the chd.
 * @param   toc         The TOC.
 *
 * @return  A std::error_condition.
 */

std::error_condition cdrom_file::write_metadata(chd_file *chd, const toc &toc)
{
	std::error_condition err;

	/* write the metadata */
	for (int i = 0; i < toc.numtrks; i++)
	{
		std::string metadata;
		if (toc.flags & CD_FLAG_GDROM)
		{
			metadata = util::string_format(GDROM_TRACK_METADATA_FORMAT, i + 1, get_type_string(toc.tracks[i].trktype),
					get_subtype_string(toc.tracks[i].subtype), toc.tracks[i].frames, toc.tracks[i].padframes,
					toc.tracks[i].pregap, get_type_string(toc.tracks[i].pgtype),
					get_subtype_string(toc.tracks[i].pgsub), toc.tracks[i].postgap);

			err = chd->write_metadata(GDROM_TRACK_METADATA_TAG, i, metadata);
		}
		else
		{
			char submode[32];

			if (toc.tracks[i].pgdatasize > 0)
			{
				strcpy(&submode[1], get_type_string(toc.tracks[i].pgtype));
				submode[0] = 'V';   // indicate valid submode
			}
			else
			{
				strcpy(submode, get_type_string(toc.tracks[i].pgtype));
			}

			metadata = util::string_format(CDROM_TRACK_METADATA2_FORMAT, i + 1, get_type_string(toc.tracks[i].trktype),
					get_subtype_string(toc.tracks[i].subtype), toc.tracks[i].frames, toc.tracks[i].pregap,
					submode, get_subtype_string(toc.tracks[i].pgsub),
					toc.tracks[i].postgap);
			err = chd->write_metadata(CDROM_TRACK_METADATA2_TAG, i, metadata);
		}
		if (err)
			return err;
	}
	return std::error_condition();
}

/**
 * @brief   -------------------------------------------------
 *            ECC lookup tables pre-calculated tables for ECC data calcs
 *          -------------------------------------------------.
 */

const uint8_t cdrom_file::ecclow[256] =
{
	0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1a, 0x1c, 0x1e,
	0x20, 0x22, 0x24, 0x26, 0x28, 0x2a, 0x2c, 0x2e, 0x30, 0x32, 0x34, 0x36, 0x38, 0x3a, 0x3c, 0x3e,
	0x40, 0x42, 0x44, 0x46, 0x48, 0x4a, 0x4c, 0x4e, 0x50, 0x52, 0x54, 0x56, 0x58, 0x5a, 0x5c, 0x5e,
	0x60, 0x62, 0x64, 0x66, 0x68, 0x6a, 0x6c, 0x6e, 0x70, 0x72, 0x74, 0x76, 0x78, 0x7a, 0x7c, 0x7e,
	0x80, 0x82, 0x84, 0x86, 0x88, 0x8a, 0x8c, 0x8e, 0x90, 0x92, 0x94, 0x96, 0x98, 0x9a, 0x9c, 0x9e,
	0xa0, 0xa2, 0xa4, 0xa6, 0xa8, 0xaa, 0xac, 0xae, 0xb0, 0xb2, 0xb4, 0xb6, 0xb8, 0xba, 0xbc, 0xbe,
	0xc0, 0xc2, 0xc4, 0xc6, 0xc8, 0xca, 0xcc, 0xce, 0xd0, 0xd2, 0xd4, 0xd6, 0xd8, 0xda, 0xdc, 0xde,
	0xe0, 0xe2, 0xe4, 0xe6, 0xe8, 0xea, 0xec, 0xee, 0xf0, 0xf2, 0xf4, 0xf6, 0xf8, 0xfa, 0xfc, 0xfe,
	0x1d, 0x1f, 0x19, 0x1b, 0x15, 0x17, 0x11, 0x13, 0x0d, 0x0f, 0x09, 0x0b, 0x05, 0x07, 0x01, 0x03,
	0x3d, 0x3f, 0x39, 0x3b, 0x35, 0x37, 0x31, 0x33, 0x2d, 0x2f, 0x29, 0x2b, 0x25, 0x27, 0x21, 0x23,
	0x5d, 0x5f, 0x59, 0x5b, 0x55, 0x57, 0x51, 0x53, 0x4d, 0x4f, 0x49, 0x4b, 0x45, 0x47, 0x41, 0x43,
	0x7d, 0x7f, 0x79, 0x7b, 0x75, 0x77, 0x71, 0x73, 0x6d, 0x6f, 0x69, 0x6b, 0x65, 0x67, 0x61, 0x63,
	0x9d, 0x9f, 0x99, 0x9b, 0x95, 0x97, 0x91, 0x93, 0x8d, 0x8f, 0x89, 0x8b, 0x85, 0x87, 0x81, 0x83,
	0xbd, 0xbf, 0xb9, 0xbb, 0xb5, 0xb7, 0xb1, 0xb3, 0xad, 0xaf, 0xa9, 0xab, 0xa5, 0xa7, 0xa1, 0xa3,
	0xdd, 0xdf, 0xd9, 0xdb, 0xd5, 0xd7, 0xd1, 0xd3, 0xcd, 0xcf, 0xc9, 0xcb, 0xc5, 0xc7, 0xc1, 0xc3,
	0xfd, 0xff, 0xf9, 0xfb, 0xf5, 0xf7, 0xf1, 0xf3, 0xed, 0xef, 0xe9, 0xeb, 0xe5, 0xe7, 0xe1, 0xe3
};

/** @brief  The ecchigh[ 256]. */
const uint8_t cdrom_file::ecchigh[256] =
{
	0x00, 0xf4, 0xf5, 0x01, 0xf7, 0x03, 0x02, 0xf6, 0xf3, 0x07, 0x06, 0xf2, 0x04, 0xf0, 0xf1, 0x05,
	0xfb, 0x0f, 0x0e, 0xfa, 0x0c, 0xf8, 0xf9, 0x0d, 0x08, 0xfc, 0xfd, 0x09, 0xff, 0x0b, 0x0a, 0xfe,
	0xeb, 0x1f, 0x1e, 0xea, 0x1c, 0xe8, 0xe9, 0x1d, 0x18, 0xec, 0xed, 0x19, 0xef, 0x1b, 0x1a, 0xee,
	0x10, 0xe4, 0xe5, 0x11, 0xe7, 0x13, 0x12, 0xe6, 0xe3, 0x17, 0x16, 0xe2, 0x14, 0xe0, 0xe1, 0x15,
	0xcb, 0x3f, 0x3e, 0xca, 0x3c, 0xc8, 0xc9, 0x3d, 0x38, 0xcc, 0xcd, 0x39, 0xcf, 0x3b, 0x3a, 0xce,
	0x30, 0xc4, 0xc5, 0x31, 0xc7, 0x33, 0x32, 0xc6, 0xc3, 0x37, 0x36, 0xc2, 0x34, 0xc0, 0xc1, 0x35,
	0x20, 0xd4, 0xd5, 0x21, 0xd7, 0x23, 0x22, 0xd6, 0xd3, 0x27, 0x26, 0xd2, 0x24, 0xd0, 0xd1, 0x25,
	0xdb, 0x2f, 0x2e, 0xda, 0x2c, 0xd8, 0xd9, 0x2d, 0x28, 0xdc, 0xdd, 0x29, 0xdf, 0x2b, 0x2a, 0xde,
	0x8b, 0x7f, 0x7e, 0x8a, 0x7c, 0x88, 0x89, 0x7d, 0x78, 0x8c, 0x8d, 0x79, 0x8f, 0x7b, 0x7a, 0x8e,
	0x70, 0x84, 0x85, 0x71, 0x87, 0x73, 0x72, 0x86, 0x83, 0x77, 0x76, 0x82, 0x74, 0x80, 0x81, 0x75,
	0x60, 0x94, 0x95, 0x61, 0x97, 0x63, 0x62, 0x96, 0x93, 0x67, 0x66, 0x92, 0x64, 0x90, 0x91, 0x65,
	0x9b, 0x6f, 0x6e, 0x9a, 0x6c, 0x98, 0x99, 0x6d, 0x68, 0x9c, 0x9d, 0x69, 0x9f, 0x6b, 0x6a, 0x9e,
	0x40, 0xb4, 0xb5, 0x41, 0xb7, 0x43, 0x42, 0xb6, 0xb3, 0x47, 0x46, 0xb2, 0x44, 0xb0, 0xb1, 0x45,
	0xbb, 0x4f, 0x4e, 0xba, 0x4c, 0xb8, 0xb9, 0x4d, 0x48, 0xbc, 0xbd, 0x49, 0xbf, 0x4b, 0x4a, 0xbe,
	0xab, 0x5f, 0x5e, 0xaa, 0x5c, 0xa8, 0xa9, 0x5d, 0x58, 0xac, 0xad, 0x59, 0xaf, 0x5b, 0x5a, 0xae,
	0x50, 0xa4, 0xa5, 0x51, 0xa7, 0x53, 0x52, 0xa6, 0xa3, 0x57, 0x56, 0xa2, 0x54, 0xa0, 0xa1, 0x55
};

/**
 * @brief   -------------------------------------------------
 *            poffsets - each row represents the addresses used to calculate a byte of the ECC P
 *            data 86 (*2) ECC P bytes, 24 values represented by each
 *          -------------------------------------------------.
 */

const uint16_t cdrom_file::poffsets[cdrom_file::ECC_P_NUM_BYTES][cdrom_file::ECC_P_COMP] =
{
	{ 0x000,0x056,0x0ac,0x102,0x158,0x1ae,0x204,0x25a,0x2b0,0x306,0x35c,0x3b2,0x408,0x45e,0x4b4,0x50a,0x560,0x5b6,0x60c,0x662,0x6b8,0x70e,0x764,0x7ba },
	{ 0x001,0x057,0x0ad,0x103,0x159,0x1af,0x205,0x25b,0x2b1,0x307,0x35d,0x3b3,0x409,0x45f,0x4b5,0x50b,0x561,0x5b7,0x60d,0x663,0x6b9,0x70f,0x765,0x7bb },
	{ 0x002,0x058,0x0ae,0x104,0x15a,0x1b0,0x206,0x25c,0x2b2,0x308,0x35e,0x3b4,0x40a,0x460,0x4b6,0x50c,0x562,0x5b8,0x60e,0x664,0x6ba,0x710,0x766,0x7bc },
	{ 0x003,0x059,0x0af,0x105,0x15b,0x1b1,0x207,0x25d,0x2b3,0x309,0x35f,0x3b5,0x40b,0x461,0x4b7,0x50d,0x563,0x5b9,0x60f,0x665,0x6bb,0x711,0x767,0x7bd },
	{ 0x004,0x05a,0x0b0,0x106,0x15c,0x1b2,0x208,0x25e,0x2b4,0x30a,0x360,0x3b6,0x40c,0x462,0x4b8,0x50e,0x564,0x5ba,0x610,0x666,0x6bc,0x712,0x768,0x7be },
	{ 0x005,0x05b,0x0b1,0x107,0x15d,0x1b3,0x209,0x25f,0x2b5,0x30b,0x361,0x3b7,0x40d,0x463,0x4b9,0x50f,0x565,0x5bb,0x611,0x667,0x6bd,0x713,0x769,0x7bf },
	{ 0x006,0x05c,0x0b2,0x108,0x15e,0x1b4,0x20a,0x260,0x2b6,0x30c,0x362,0x3b8,0x40e,0x464,0x4ba,0x510,0x566,0x5bc,0x612,0x668,0x6be,0x714,0x76a,0x7c0 },
	{ 0x007,0x05d,0x0b3,0x109,0x15f,0x1b5,0x20b,0x261,0x2b7,0x30d,0x363,0x3b9,0x40f,0x465,0x4bb,0x511,0x567,0x5bd,0x613,0x669,0x6bf,0x715,0x76b,0x7c1 },
	{ 0x008,0x05e,0x0b4,0x10a,0x160,0x1b6,0x20c,0x262,0x2b8,0x30e,0x364,0x3ba,0x410,0x466,0x4bc,0x512,0x568,0x5be,0x614,0x66a,0x6c0,0x716,0x76c,0x7c2 },
	{ 0x009,0x05f,0x0b5,0x10b,0x161,0x1b7,0x20d,0x263,0x2b9,0x30f,0x365,0x3bb,0x411,0x467,0x4bd,0x513,0x569,0x5bf,0x615,0x66b,0x6c1,0x717,0x76d,0x7c3 },
	{ 0x00a,0x060,0x0b6,0x10c,0x162,0x1b8,0x20e,0x264,0x2ba,0x310,0x366,0x3bc,0x412,0x468,0x4be,0x514,0x56a,0x5c0,0x616,0x66c,0x6c2,0x718,0x76e,0x7c4 },
	{ 0x00b,0x061,0x0b7,0x10d,0x163,0x1b9,0x20f,0x265,0x2bb,0x311,0x367,0x3bd,0x413,0x469,0x4bf,0x515,0x56b,0x5c1,0x617,0x66d,0x6c3,0x719,0x76f,0x7c5 },
	{ 0x00c,0x062,0x0b8,0x10e,0x164,0x1ba,0x210,0x266,0x2bc,0x312,0x368,0x3be,0x414,0x46a,0x4c0,0x516,0x56c,0x5c2,0x618,0x66e,0x6c4,0x71a,0x770,0x7c6 },
	{ 0x00d,0x063,0x0b9,0x10f,0x165,0x1bb,0x211,0x267,0x2bd,0x313,0x369,0x3bf,0x415,0x46b,0x4c1,0x517,0x56d,0x5c3,0x619,0x66f,0x6c5,0x71b,0x771,0x7c7 },
	{ 0x00e,0x064,0x0ba,0x110,0x166,0x1bc,0x212,0x268,0x2be,0x314,0x36a,0x3c0,0x416,0x46c,0x4c2,0x518,0x56e,0x5c4,0x61a,0x670,0x6c6,0x71c,0x772,0x7c8 },
	{ 0x00f,0x065,0x0bb,0x111,0x167,0x1bd,0x213,0x269,0x2bf,0x315,0x36b,0x3c1,0x417,0x46d,0x4c3,0x519,0x56f,0x5c5,0x61b,0x671,0x6c7,0x71d,0x773,0x7c9 },
	{ 0x010,0x066,0x0bc,0x112,0x168,0x1be,0x214,0x26a,0x2c0,0x316,0x36c,0x3c2,0x418,0x46e,0x4c4,0x51a,0x570,0x5c6,0x61c,0x672,0x6c8,0x71e,0x774,0x7ca },
	{ 0x011,0x067,0x0bd,0x113,0x169,0x1bf,0x215,0x26b,0x2c1,0x317,0x36d,0x3c3,0x419,0x46f,0x4c5,0x51b,0x571,0x5c7,0x61d,0x673,0x6c9,0x71f,0x775,0x7cb },
	{ 0x012,0x068,0x0be,0x114,0x16a,0x1c0,0x216,0x26c,0x2c2,0x318,0x36e,0x3c4,0x41a,0x470,0x4c6,0x51c,0x572,0x5c8,0x61e,0x674,0x6ca,0x720,0x776,0x7cc },
	{ 0x013,0x069,0x0bf,0x115,0x16b,0x1c1,0x217,0x26d,0x2c3,0x319,0x36f,0x3c5,0x41b,0x471,0x4c7,0x51d,0x573,0x5c9,0x61f,0x675,0x6cb,0x721,0x777,0x7cd },
	{ 0x014,0x06a,0x0c0,0x116,0x16c,0x1c2,0x218,0x26e,0x2c4,0x31a,0x370,0x3c6,0x41c,0x472,0x4c8,0x51e,0x574,0x5ca,0x620,0x676,0x6cc,0x722,0x778,0x7ce },
	{ 0x015,0x06b,0x0c1,0x117,0x16d,0x1c3,0x219,0x26f,0x2c5,0x31b,0x371,0x3c7,0x41d,0x473,0x4c9,0x51f,0x575,0x5cb,0x621,0x677,0x6cd,0x723,0x779,0x7cf },
	{ 0x016,0x06c,0x0c2,0x118,0x16e,0x1c4,0x21a,0x270,0x2c6,0x31c,0x372,0x3c8,0x41e,0x474,0x4ca,0x520,0x576,0x5cc,0x622,0x678,0x6ce,0x724,0x77a,0x7d0 },
	{ 0x017,0x06d,0x0c3,0x119,0x16f,0x1c5,0x21b,0x271,0x2c7,0x31d,0x373,0x3c9,0x41f,0x475,0x4cb,0x521,0x577,0x5cd,0x623,0x679,0x6cf,0x725,0x77b,0x7d1 },
	{ 0x018,0x06e,0x0c4,0x11a,0x170,0x1c6,0x21c,0x272,0x2c8,0x31e,0x374,0x3ca,0x420,0x476,0x4cc,0x522,0x578,0x5ce,0x624,0x67a,0x6d0,0x726,0x77c,0x7d2 },
	{ 0x019,0x06f,0x0c5,0x11b,0x171,0x1c7,0x21d,0x273,0x2c9,0x31f,0x375,0x3cb,0x421,0x477,0x4cd,0x523,0x579,0x5cf,0x625,0x67b,0x6d1,0x727,0x77d,0x7d3 },
	{ 0x01a,0x070,0x0c6,0x11c,0x172,0x1c8,0x21e,0x274,0x2ca,0x320,0x376,0x3cc,0x422,0x478,0x4ce,0x524,0x57a,0x5d0,0x626,0x67c,0x6d2,0x728,0x77e,0x7d4 },
	{ 0x01b,0x071,0x0c7,0x11d,0x173,0x1c9,0x21f,0x275,0x2cb,0x321,0x377,0x3cd,0x423,0x479,0x4cf,0x525,0x57b,0x5d1,0x627,0x67d,0x6d3,0x729,0x77f,0x7d5 },
	{ 0x01c,0x072,0x0c8,0x11e,0x174,0x1ca,0x220,0x276,0x2cc,0x322,0x378,0x3ce,0x424,0x47a,0x4d0,0x526,0x57c,0x5d2,0x628,0x67e,0x6d4,0x72a,0x780,0x7d6 },
	{ 0x01d,0x073,0x0c9,0x11f,0x175,0x1cb,0x221,0x277,0x2cd,0x323,0x379,0x3cf,0x425,0x47b,0x4d1,0x527,0x57d,0x5d3,0x629,0x67f,0x6d5,0x72b,0x781,0x7d7 },
	{ 0x01e,0x074,0x0ca,0x120,0x176,0x1cc,0x222,0x278,0x2ce,0x324,0x37a,0x3d0,0x426,0x47c,0x4d2,0x528,0x57e,0x5d4,0x62a,0x680,0x6d6,0x72c,0x782,0x7d8 },
	{ 0x01f,0x075,0x0cb,0x121,0x177,0x1cd,0x223,0x279,0x2cf,0x325,0x37b,0x3d1,0x427,0x47d,0x4d3,0x529,0x57f,0x5d5,0x62b,0x681,0x6d7,0x72d,0x783,0x7d9 },
	{ 0x020,0x076,0x0cc,0x122,0x178,0x1ce,0x224,0x27a,0x2d0,0x326,0x37c,0x3d2,0x428,0x47e,0x4d4,0x52a,0x580,0x5d6,0x62c,0x682,0x6d8,0x72e,0x784,0x7da },
	{ 0x021,0x077,0x0cd,0x123,0x179,0x1cf,0x225,0x27b,0x2d1,0x327,0x37d,0x3d3,0x429,0x47f,0x4d5,0x52b,0x581,0x5d7,0x62d,0x683,0x6d9,0x72f,0x785,0x7db },
	{ 0x022,0x078,0x0ce,0x124,0x17a,0x1d0,0x226,0x27c,0x2d2,0x328,0x37e,0x3d4,0x42a,0x480,0x4d6,0x52c,0x582,0x5d8,0x62e,0x684,0x6da,0x730,0x786,0x7dc },
	{ 0x023,0x079,0x0cf,0x125,0x17b,0x1d1,0x227,0x27d,0x2d3,0x329,0x37f,0x3d5,0x42b,0x481,0x4d7,0x52d,0x583,0x5d9,0x62f,0x685,0x6db,0x731,0x787,0x7dd },
	{ 0x024,0x07a,0x0d0,0x126,0x17c,0x1d2,0x228,0x27e,0x2d4,0x32a,0x380,0x3d6,0x42c,0x482,0x4d8,0x52e,0x584,0x5da,0x630,0x686,0x6dc,0x732,0x788,0x7de },
	{ 0x025,0x07b,0x0d1,0x127,0x17d,0x1d3,0x229,0x27f,0x2d5,0x32b,0x381,0x3d7,0x42d,0x483,0x4d9,0x52f,0x585,0x5db,0x631,0x687,0x6dd,0x733,0x789,0x7df },
	{ 0x026,0x07c,0x0d2,0x128,0x17e,0x1d4,0x22a,0x280,0x2d6,0x32c,0x382,0x3d8,0x42e,0x484,0x4da,0x530,0x586,0x5dc,0x632,0x688,0x6de,0x734,0x78a,0x7e0 },
	{ 0x027,0x07d,0x0d3,0x129,0x17f,0x1d5,0x22b,0x281,0x2d7,0x32d,0x383,0x3d9,0x42f,0x485,0x4db,0x531,0x587,0x5dd,0x633,0x689,0x6df,0x735,0x78b,0x7e1 },
	{ 0x028,0x07e,0x0d4,0x12a,0x180,0x1d6,0x22c,0x282,0x2d8,0x32e,0x384,0x3da,0x430,0x486,0x4dc,0x532,0x588,0x5de,0x634,0x68a,0x6e0,0x736,0x78c,0x7e2 },
	{ 0x029,0x07f,0x0d5,0x12b,0x181,0x1d7,0x22d,0x283,0x2d9,0x32f,0x385,0x3db,0x431,0x487,0x4dd,0x533,0x589,0x5df,0x635,0x68b,0x6e1,0x737,0x78d,0x7e3 },
	{ 0x02a,0x080,0x0d6,0x12c,0x182,0x1d8,0x22e,0x284,0x2da,0x330,0x386,0x3dc,0x432,0x488,0x4de,0x534,0x58a,0x5e0,0x636,0x68c,0x6e2,0x738,0x78e,0x7e4 },
	{ 0x02b,0x081,0x0d7,0x12d,0x183,0x1d9,0x22f,0x285,0x2db,0x331,0x387,0x3dd,0x433,0x489,0x4df,0x535,0x58b,0x5e1,0x637,0x68d,0x6e3,0x739,0x78f,0x7e5 },
	{ 0x02c,0x082,0x0d8,0x12e,0x184,0x1da,0x230,0x286,0x2dc,0x332,0x388,0x3de,0x434,0x48a,0x4e0,0x536,0x58c,0x5e2,0x638,0x68e,0x6e4,0x73a,0x790,0x7e6 },
	{ 0x02d,0x083,0x0d9,0x12f,0x185,0x1db,0x231,0x287,0x2dd,0x333,0x389,0x3df,0x435,0x48b,0x4e1,0x537,0x58d,0x5e3,0x639,0x68f,0x6e5,0x73b,0x791,0x7e7 },
	{ 0x02e,0x084,0x0da,0x130,0x186,0x1dc,0x232,0x288,0x2de,0x334,0x38a,0x3e0,0x436,0x48c,0x4e2,0x538,0x58e,0x5e4,0x63a,0x690,0x6e6,0x73c,0x792,0x7e8 },
	{ 0x02f,0x085,0x0db,0x131,0x187,0x1dd,0x233,0x289,0x2df,0x335,0x38b,0x3e1,0x437,0x48d,0x4e3,0x539,0x58f,0x5e5,0x63b,0x691,0x6e7,0x73d,0x793,0x7e9 },
	{ 0x030,0x086,0x0dc,0x132,0x188,0x1de,0x234,0x28a,0x2e0,0x336,0x38c,0x3e2,0x438,0x48e,0x4e4,0x53a,0x590,0x5e6,0x63c,0x692,0x6e8,0x73e,0x794,0x7ea },
	{ 0x031,0x087,0x0dd,0x133,0x189,0x1df,0x235,0x28b,0x2e1,0x337,0x38d,0x3e3,0x439,0x48f,0x4e5,0x53b,0x591,0x5e7,0x63d,0x693,0x6e9,0x73f,0x795,0x7eb },
	{ 0x032,0x088,0x0de,0x134,0x18a,0x1e0,0x236,0x28c,0x2e2,0x338,0x38e,0x3e4,0x43a,0x490,0x4e6,0x53c,0x592,0x5e8,0x63e,0x694,0x6ea,0x740,0x796,0x7ec },
	{ 0x033,0x089,0x0df,0x135,0x18b,0x1e1,0x237,0x28d,0x2e3,0x339,0x38f,0x3e5,0x43b,0x491,0x4e7,0x53d,0x593,0x5e9,0x63f,0x695,0x6eb,0x741,0x797,0x7ed },
	{ 0x034,0x08a,0x0e0,0x136,0x18c,0x1e2,0x238,0x28e,0x2e4,0x33a,0x390,0x3e6,0x43c,0x492,0x4e8,0x53e,0x594,0x5ea,0x640,0x696,0x6ec,0x742,0x798,0x7ee },
	{ 0x035,0x08b,0x0e1,0x137,0x18d,0x1e3,0x239,0x28f,0x2e5,0x33b,0x391,0x3e7,0x43d,0x493,0x4e9,0x53f,0x595,0x5eb,0x641,0x697,0x6ed,0x743,0x799,0x7ef },
	{ 0x036,0x08c,0x0e2,0x138,0x18e,0x1e4,0x23a,0x290,0x2e6,0x33c,0x392,0x3e8,0x43e,0x494,0x4ea,0x540,0x596,0x5ec,0x642,0x698,0x6ee,0x744,0x79a,0x7f0 },
	{ 0x037,0x08d,0x0e3,0x139,0x18f,0x1e5,0x23b,0x291,0x2e7,0x33d,0x393,0x3e9,0x43f,0x495,0x4eb,0x541,0x597,0x5ed,0x643,0x699,0x6ef,0x745,0x79b,0x7f1 },
	{ 0x038,0x08e,0x0e4,0x13a,0x190,0x1e6,0x23c,0x292,0x2e8,0x33e,0x394,0x3ea,0x440,0x496,0x4ec,0x542,0x598,0x5ee,0x644,0x69a,0x6f0,0x746,0x79c,0x7f2 },
	{ 0x039,0x08f,0x0e5,0x13b,0x191,0x1e7,0x23d,0x293,0x2e9,0x33f,0x395,0x3eb,0x441,0x497,0x4ed,0x543,0x599,0x5ef,0x645,0x69b,0x6f1,0x747,0x79d,0x7f3 },
	{ 0x03a,0x090,0x0e6,0x13c,0x192,0x1e8,0x23e,0x294,0x2ea,0x340,0x396,0x3ec,0x442,0x498,0x4ee,0x544,0x59a,0x5f0,0x646,0x69c,0x6f2,0x748,0x79e,0x7f4 },
	{ 0x03b,0x091,0x0e7,0x13d,0x193,0x1e9,0x23f,0x295,0x2eb,0x341,0x397,0x3ed,0x443,0x499,0x4ef,0x545,0x59b,0x5f1,0x647,0x69d,0x6f3,0x749,0x79f,0x7f5 },
	{ 0x03c,0x092,0x0e8,0x13e,0x194,0x1ea,0x240,0x296,0x2ec,0x342,0x398,0x3ee,0x444,0x49a,0x4f0,0x546,0x59c,0x5f2,0x648,0x69e,0x6f4,0x74a,0x7a0,0x7f6 },
	{ 0x03d,0x093,0x0e9,0x13f,0x195,0x1eb,0x241,0x297,0x2ed,0x343,0x399,0x3ef,0x445,0x49b,0x4f1,0x547,0x59d,0x5f3,0x649,0x69f,0x6f5,0x74b,0x7a1,0x7f7 },
	{ 0x03e,0x094,0x0ea,0x140,0x196,0x1ec,0x242,0x298,0x2ee,0x344,0x39a,0x3f0,0x446,0x49c,0x4f2,0x548,0x59e,0x5f4,0x64a,0x6a0,0x6f6,0x74c,0x7a2,0x7f8 },
	{ 0x03f,0x095,0x0eb,0x141,0x197,0x1ed,0x243,0x299,0x2ef,0x345,0x39b,0x3f1,0x447,0x49d,0x4f3,0x549,0x59f,0x5f5,0x64b,0x6a1,0x6f7,0x74d,0x7a3,0x7f9 },
	{ 0x040,0x096,0x0ec,0x142,0x198,0x1ee,0x244,0x29a,0x2f0,0x346,0x39c,0x3f2,0x448,0x49e,0x4f4,0x54a,0x5a0,0x5f6,0x64c,0x6a2,0x6f8,0x74e,0x7a4,0x7fa },
	{ 0x041,0x097,0x0ed,0x143,0x199,0x1ef,0x245,0x29b,0x2f1,0x347,0x39d,0x3f3,0x449,0x49f,0x4f5,0x54b,0x5a1,0x5f7,0x64d,0x6a3,0x6f9,0x74f,0x7a5,0x7fb },
	{ 0x042,0x098,0x0ee,0x144,0x19a,0x1f0,0x246,0x29c,0x2f2,0x348,0x39e,0x3f4,0x44a,0x4a0,0x4f6,0x54c,0x5a2,0x5f8,0x64e,0x6a4,0x6fa,0x750,0x7a6,0x7fc },
	{ 0x043,0x099,0x0ef,0x145,0x19b,0x1f1,0x247,0x29d,0x2f3,0x349,0x39f,0x3f5,0x44b,0x4a1,0x4f7,0x54d,0x5a3,0x5f9,0x64f,0x6a5,0x6fb,0x751,0x7a7,0x7fd },
	{ 0x044,0x09a,0x0f0,0x146,0x19c,0x1f2,0x248,0x29e,0x2f4,0x34a,0x3a0,0x3f6,0x44c,0x4a2,0x4f8,0x54e,0x5a4,0x5fa,0x650,0x6a6,0x6fc,0x752,0x7a8,0x7fe },
	{ 0x045,0x09b,0x0f1,0x147,0x19d,0x1f3,0x249,0x29f,0x2f5,0x34b,0x3a1,0x3f7,0x44d,0x4a3,0x4f9,0x54f,0x5a5,0x5fb,0x651,0x6a7,0x6fd,0x753,0x7a9,0x7ff },
	{ 0x046,0x09c,0x0f2,0x148,0x19e,0x1f4,0x24a,0x2a0,0x2f6,0x34c,0x3a2,0x3f8,0x44e,0x4a4,0x4fa,0x550,0x5a6,0x5fc,0x652,0x6a8,0x6fe,0x754,0x7aa,0x800 },
	{ 0x047,0x09d,0x0f3,0x149,0x19f,0x1f5,0x24b,0x2a1,0x2f7,0x34d,0x3a3,0x3f9,0x44f,0x4a5,0x4fb,0x551,0x5a7,0x5fd,0x653,0x6a9,0x6ff,0x755,0x7ab,0x801 },
	{ 0x048,0x09e,0x0f4,0x14a,0x1a0,0x1f6,0x24c,0x2a2,0x2f8,0x34e,0x3a4,0x3fa,0x450,0x4a6,0x4fc,0x552,0x5a8,0x5fe,0x654,0x6aa,0x700,0x756,0x7ac,0x802 },
	{ 0x049,0x09f,0x0f5,0x14b,0x1a1,0x1f7,0x24d,0x2a3,0x2f9,0x34f,0x3a5,0x3fb,0x451,0x4a7,0x4fd,0x553,0x5a9,0x5ff,0x655,0x6ab,0x701,0x757,0x7ad,0x803 },
	{ 0x04a,0x0a0,0x0f6,0x14c,0x1a2,0x1f8,0x24e,0x2a4,0x2fa,0x350,0x3a6,0x3fc,0x452,0x4a8,0x4fe,0x554,0x5aa,0x600,0x656,0x6ac,0x702,0x758,0x7ae,0x804 },
	{ 0x04b,0x0a1,0x0f7,0x14d,0x1a3,0x1f9,0x24f,0x2a5,0x2fb,0x351,0x3a7,0x3fd,0x453,0x4a9,0x4ff,0x555,0x5ab,0x601,0x657,0x6ad,0x703,0x759,0x7af,0x805 },
	{ 0x04c,0x0a2,0x0f8,0x14e,0x1a4,0x1fa,0x250,0x2a6,0x2fc,0x352,0x3a8,0x3fe,0x454,0x4aa,0x500,0x556,0x5ac,0x602,0x658,0x6ae,0x704,0x75a,0x7b0,0x806 },
	{ 0x04d,0x0a3,0x0f9,0x14f,0x1a5,0x1fb,0x251,0x2a7,0x2fd,0x353,0x3a9,0x3ff,0x455,0x4ab,0x501,0x557,0x5ad,0x603,0x659,0x6af,0x705,0x75b,0x7b1,0x807 },
	{ 0x04e,0x0a4,0x0fa,0x150,0x1a6,0x1fc,0x252,0x2a8,0x2fe,0x354,0x3aa,0x400,0x456,0x4ac,0x502,0x558,0x5ae,0x604,0x65a,0x6b0,0x706,0x75c,0x7b2,0x808 },
	{ 0x04f,0x0a5,0x0fb,0x151,0x1a7,0x1fd,0x253,0x2a9,0x2ff,0x355,0x3ab,0x401,0x457,0x4ad,0x503,0x559,0x5af,0x605,0x65b,0x6b1,0x707,0x75d,0x7b3,0x809 },
	{ 0x050,0x0a6,0x0fc,0x152,0x1a8,0x1fe,0x254,0x2aa,0x300,0x356,0x3ac,0x402,0x458,0x4ae,0x504,0x55a,0x5b0,0x606,0x65c,0x6b2,0x708,0x75e,0x7b4,0x80a },
	{ 0x051,0x0a7,0x0fd,0x153,0x1a9,0x1ff,0x255,0x2ab,0x301,0x357,0x3ad,0x403,0x459,0x4af,0x505,0x55b,0x5b1,0x607,0x65d,0x6b3,0x709,0x75f,0x7b5,0x80b },
	{ 0x052,0x0a8,0x0fe,0x154,0x1aa,0x200,0x256,0x2ac,0x302,0x358,0x3ae,0x404,0x45a,0x4b0,0x506,0x55c,0x5b2,0x608,0x65e,0x6b4,0x70a,0x760,0x7b6,0x80c },
	{ 0x053,0x0a9,0x0ff,0x155,0x1ab,0x201,0x257,0x2ad,0x303,0x359,0x3af,0x405,0x45b,0x4b1,0x507,0x55d,0x5b3,0x609,0x65f,0x6b5,0x70b,0x761,0x7b7,0x80d },
	{ 0x054,0x0aa,0x100,0x156,0x1ac,0x202,0x258,0x2ae,0x304,0x35a,0x3b0,0x406,0x45c,0x4b2,0x508,0x55e,0x5b4,0x60a,0x660,0x6b6,0x70c,0x762,0x7b8,0x80e },
	{ 0x055,0x0ab,0x101,0x157,0x1ad,0x203,0x259,0x2af,0x305,0x35b,0x3b1,0x407,0x45d,0x4b3,0x509,0x55f,0x5b5,0x60b,0x661,0x6b7,0x70d,0x763,0x7b9,0x80f }
};

/**
 * @brief   -------------------------------------------------
 *            qoffsets - each row represents the addresses used to calculate a byte of the ECC Q
 *            data 52 (*2) ECC Q bytes, 43 values represented by each
 *          -------------------------------------------------.
 */

const uint16_t cdrom_file::qoffsets[ECC_Q_NUM_BYTES][ECC_Q_COMP] =
{
	{ 0x000,0x058,0x0b0,0x108,0x160,0x1b8,0x210,0x268,0x2c0,0x318,0x370,0x3c8,0x420,0x478,0x4d0,0x528,0x580,0x5d8,0x630,0x688,0x6e0,0x738,0x790,0x7e8,0x840,0x898,0x034,0x08c,0x0e4,0x13c,0x194,0x1ec,0x244,0x29c,0x2f4,0x34c,0x3a4,0x3fc,0x454,0x4ac,0x504,0x55c,0x5b4 },
	{ 0x001,0x059,0x0b1,0x109,0x161,0x1b9,0x211,0x269,0x2c1,0x319,0x371,0x3c9,0x421,0x479,0x4d1,0x529,0x581,0x5d9,0x631,0x689,0x6e1,0x739,0x791,0x7e9,0x841,0x899,0x035,0x08d,0x0e5,0x13d,0x195,0x1ed,0x245,0x29d,0x2f5,0x34d,0x3a5,0x3fd,0x455,0x4ad,0x505,0x55d,0x5b5 },
	{ 0x056,0x0ae,0x106,0x15e,0x1b6,0x20e,0x266,0x2be,0x316,0x36e,0x3c6,0x41e,0x476,0x4ce,0x526,0x57e,0x5d6,0x62e,0x686,0x6de,0x736,0x78e,0x7e6,0x83e,0x896,0x032,0x08a,0x0e2,0x13a,0x192,0x1ea,0x242,0x29a,0x2f2,0x34a,0x3a2,0x3fa,0x452,0x4aa,0x502,0x55a,0x5b2,0x60a },
	{ 0x057,0x0af,0x107,0x15f,0x1b7,0x20f,0x267,0x2bf,0x317,0x36f,0x3c7,0x41f,0x477,0x4cf,0x527,0x57f,0x5d7,0x62f,0x687,0x6df,0x737,0x78f,0x7e7,0x83f,0x897,0x033,0x08b,0x0e3,0x13b,0x193,0x1eb,0x243,0x29b,0x2f3,0x34b,0x3a3,0x3fb,0x453,0x4ab,0x503,0x55b,0x5b3,0x60b },
	{ 0x0ac,0x104,0x15c,0x1b4,0x20c,0x264,0x2bc,0x314,0x36c,0x3c4,0x41c,0x474,0x4cc,0x524,0x57c,0x5d4,0x62c,0x684,0x6dc,0x734,0x78c,0x7e4,0x83c,0x894,0x030,0x088,0x0e0,0x138,0x190,0x1e8,0x240,0x298,0x2f0,0x348,0x3a0,0x3f8,0x450,0x4a8,0x500,0x558,0x5b0,0x608,0x660 },
	{ 0x0ad,0x105,0x15d,0x1b5,0x20d,0x265,0x2bd,0x315,0x36d,0x3c5,0x41d,0x475,0x4cd,0x525,0x57d,0x5d5,0x62d,0x685,0x6dd,0x735,0x78d,0x7e5,0x83d,0x895,0x031,0x089,0x0e1,0x139,0x191,0x1e9,0x241,0x299,0x2f1,0x349,0x3a1,0x3f9,0x451,0x4a9,0x501,0x559,0x5b1,0x609,0x661 },
	{ 0x102,0x15a,0x1b2,0x20a,0x262,0x2ba,0x312,0x36a,0x3c2,0x41a,0x472,0x4ca,0x522,0x57a,0x5d2,0x62a,0x682,0x6da,0x732,0x78a,0x7e2,0x83a,0x892,0x02e,0x086,0x0de,0x136,0x18e,0x1e6,0x23e,0x296,0x2ee,0x346,0x39e,0x3f6,0x44e,0x4a6,0x4fe,0x556,0x5ae,0x606,0x65e,0x6b6 },
	{ 0x103,0x15b,0x1b3,0x20b,0x263,0x2bb,0x313,0x36b,0x3c3,0x41b,0x473,0x4cb,0x523,0x57b,0x5d3,0x62b,0x683,0x6db,0x733,0x78b,0x7e3,0x83b,0x893,0x02f,0x087,0x0df,0x137,0x18f,0x1e7,0x23f,0x297,0x2ef,0x347,0x39f,0x3f7,0x44f,0x4a7,0x4ff,0x557,0x5af,0x607,0x65f,0x6b7 },
	{ 0x158,0x1b0,0x208,0x260,0x2b8,0x310,0x368,0x3c0,0x418,0x470,0x4c8,0x520,0x578,0x5d0,0x628,0x680,0x6d8,0x730,0x788,0x7e0,0x838,0x890,0x02c,0x084,0x0dc,0x134,0x18c,0x1e4,0x23c,0x294,0x2ec,0x344,0x39c,0x3f4,0x44c,0x4a4,0x4fc,0x554,0x5ac,0x604,0x65c,0x6b4,0x70c },
	{ 0x159,0x1b1,0x209,0x261,0x2b9,0x311,0x369,0x3c1,0x419,0x471,0x4c9,0x521,0x579,0x5d1,0x629,0x681,0x6d9,0x731,0x789,0x7e1,0x839,0x891,0x02d,0x085,0x0dd,0x135,0x18d,0x1e5,0x23d,0x295,0x2ed,0x345,0x39d,0x3f5,0x44d,0x4a5,0x4fd,0x555,0x5ad,0x605,0x65d,0x6b5,0x70d },
	{ 0x1ae,0x206,0x25e,0x2b6,0x30e,0x366,0x3be,0x416,0x46e,0x4c6,0x51e,0x576,0x5ce,0x626,0x67e,0x6d6,0x72e,0x786,0x7de,0x836,0x88e,0x02a,0x082,0x0da,0x132,0x18a,0x1e2,0x23a,0x292,0x2ea,0x342,0x39a,0x3f2,0x44a,0x4a2,0x4fa,0x552,0x5aa,0x602,0x65a,0x6b2,0x70a,0x762 },
	{ 0x1af,0x207,0x25f,0x2b7,0x30f,0x367,0x3bf,0x417,0x46f,0x4c7,0x51f,0x577,0x5cf,0x627,0x67f,0x6d7,0x72f,0x787,0x7df,0x837,0x88f,0x02b,0x083,0x0db,0x133,0x18b,0x1e3,0x23b,0x293,0x2eb,0x343,0x39b,0x3f3,0x44b,0x4a3,0x4fb,0x553,0x5ab,0x603,0x65b,0x6b3,0x70b,0x763 },
	{ 0x204,0x25c,0x2b4,0x30c,0x364,0x3bc,0x414,0x46c,0x4c4,0x51c,0x574,0x5cc,0x624,0x67c,0x6d4,0x72c,0x784,0x7dc,0x834,0x88c,0x028,0x080,0x0d8,0x130,0x188,0x1e0,0x238,0x290,0x2e8,0x340,0x398,0x3f0,0x448,0x4a0,0x4f8,0x550,0x5a8,0x600,0x658,0x6b0,0x708,0x760,0x7b8 },
	{ 0x205,0x25d,0x2b5,0x30d,0x365,0x3bd,0x415,0x46d,0x4c5,0x51d,0x575,0x5cd,0x625,0x67d,0x6d5,0x72d,0x785,0x7dd,0x835,0x88d,0x029,0x081,0x0d9,0x131,0x189,0x1e1,0x239,0x291,0x2e9,0x341,0x399,0x3f1,0x449,0x4a1,0x4f9,0x551,0x5a9,0x601,0x659,0x6b1,0x709,0x761,0x7b9 },
	{ 0x25a,0x2b2,0x30a,0x362,0x3ba,0x412,0x46a,0x4c2,0x51a,0x572,0x5ca,0x622,0x67a,0x6d2,0x72a,0x782,0x7da,0x832,0x88a,0x026,0x07e,0x0d6,0x12e,0x186,0x1de,0x236,0x28e,0x2e6,0x33e,0x396,0x3ee,0x446,0x49e,0x4f6,0x54e,0x5a6,0x5fe,0x656,0x6ae,0x706,0x75e,0x7b6,0x80e },
	{ 0x25b,0x2b3,0x30b,0x363,0x3bb,0x413,0x46b,0x4c3,0x51b,0x573,0x5cb,0x623,0x67b,0x6d3,0x72b,0x783,0x7db,0x833,0x88b,0x027,0x07f,0x0d7,0x12f,0x187,0x1df,0x237,0x28f,0x2e7,0x33f,0x397,0x3ef,0x447,0x49f,0x4f7,0x54f,0x5a7,0x5ff,0x657,0x6af,0x707,0x75f,0x7b7,0x80f },
	{ 0x2b0,0x308,0x360,0x3b8,0x410,0x468,0x4c0,0x518,0x570,0x5c8,0x620,0x678,0x6d0,0x728,0x780,0x7d8,0x830,0x888,0x024,0x07c,0x0d4,0x12c,0x184,0x1dc,0x234,0x28c,0x2e4,0x33c,0x394,0x3ec,0x444,0x49c,0x4f4,0x54c,0x5a4,0x5fc,0x654,0x6ac,0x704,0x75c,0x7b4,0x80c,0x864 },
	{ 0x2b1,0x309,0x361,0x3b9,0x411,0x469,0x4c1,0x519,0x571,0x5c9,0x621,0x679,0x6d1,0x729,0x781,0x7d9,0x831,0x889,0x025,0x07d,0x0d5,0x12d,0x185,0x1dd,0x235,0x28d,0x2e5,0x33d,0x395,0x3ed,0x445,0x49d,0x4f5,0x54d,0x5a5,0x5fd,0x655,0x6ad,0x705,0x75d,0x7b5,0x80d,0x865 },
	{ 0x306,0x35e,0x3b6,0x40e,0x466,0x4be,0x516,0x56e,0x5c6,0x61e,0x676,0x6ce,0x726,0x77e,0x7d6,0x82e,0x886,0x022,0x07a,0x0d2,0x12a,0x182,0x1da,0x232,0x28a,0x2e2,0x33a,0x392,0x3ea,0x442,0x49a,0x4f2,0x54a,0x5a2,0x5fa,0x652,0x6aa,0x702,0x75a,0x7b2,0x80a,0x862,0x8ba },
	{ 0x307,0x35f,0x3b7,0x40f,0x467,0x4bf,0x517,0x56f,0x5c7,0x61f,0x677,0x6cf,0x727,0x77f,0x7d7,0x82f,0x887,0x023,0x07b,0x0d3,0x12b,0x183,0x1db,0x233,0x28b,0x2e3,0x33b,0x393,0x3eb,0x443,0x49b,0x4f3,0x54b,0x5a3,0x5fb,0x653,0x6ab,0x703,0x75b,0x7b3,0x80b,0x863,0x8bb },
	{ 0x35c,0x3b4,0x40c,0x464,0x4bc,0x514,0x56c,0x5c4,0x61c,0x674,0x6cc,0x724,0x77c,0x7d4,0x82c,0x884,0x020,0x078,0x0d0,0x128,0x180,0x1d8,0x230,0x288,0x2e0,0x338,0x390,0x3e8,0x440,0x498,0x4f0,0x548,0x5a0,0x5f8,0x650,0x6a8,0x700,0x758,0x7b0,0x808,0x860,0x8b8,0x054 },
	{ 0x35d,0x3b5,0x40d,0x465,0x4bd,0x515,0x56d,0x5c5,0x61d,0x675,0x6cd,0x725,0x77d,0x7d5,0x82d,0x885,0x021,0x079,0x0d1,0x129,0x181,0x1d9,0x231,0x289,0x2e1,0x339,0x391,0x3e9,0x441,0x499,0x4f1,0x549,0x5a1,0x5f9,0x651,0x6a9,0x701,0x759,0x7b1,0x809,0x861,0x8b9,0x055 },
	{ 0x3b2,0x40a,0x462,0x4ba,0x512,0x56a,0x5c2,0x61a,0x672,0x6ca,0x722,0x77a,0x7d2,0x82a,0x882,0x01e,0x076,0x0ce,0x126,0x17e,0x1d6,0x22e,0x286,0x2de,0x336,0x38e,0x3e6,0x43e,0x496,0x4ee,0x546,0x59e,0x5f6,0x64e,0x6a6,0x6fe,0x756,0x7ae,0x806,0x85e,0x8b6,0x052,0x0aa },
	{ 0x3b3,0x40b,0x463,0x4bb,0x513,0x56b,0x5c3,0x61b,0x673,0x6cb,0x723,0x77b,0x7d3,0x82b,0x883,0x01f,0x077,0x0cf,0x127,0x17f,0x1d7,0x22f,0x287,0x2df,0x337,0x38f,0x3e7,0x43f,0x497,0x4ef,0x547,0x59f,0x5f7,0x64f,0x6a7,0x6ff,0x757,0x7af,0x807,0x85f,0x8b7,0x053,0x0ab },
	{ 0x408,0x460,0x4b8,0x510,0x568,0x5c0,0x618,0x670,0x6c8,0x720,0x778,0x7d0,0x828,0x880,0x01c,0x074,0x0cc,0x124,0x17c,0x1d4,0x22c,0x284,0x2dc,0x334,0x38c,0x3e4,0x43c,0x494,0x4ec,0x544,0x59c,0x5f4,0x64c,0x6a4,0x6fc,0x754,0x7ac,0x804,0x85c,0x8b4,0x050,0x0a8,0x100 },
	{ 0x409,0x461,0x4b9,0x511,0x569,0x5c1,0x619,0x671,0x6c9,0x721,0x779,0x7d1,0x829,0x881,0x01d,0x075,0x0cd,0x125,0x17d,0x1d5,0x22d,0x285,0x2dd,0x335,0x38d,0x3e5,0x43d,0x495,0x4ed,0x545,0x59d,0x5f5,0x64d,0x6a5,0x6fd,0x755,0x7ad,0x805,0x85d,0x8b5,0x051,0x0a9,0x101 },
	{ 0x45e,0x4b6,0x50e,0x566,0x5be,0x616,0x66e,0x6c6,0x71e,0x776,0x7ce,0x826,0x87e,0x01a,0x072,0x0ca,0x122,0x17a,0x1d2,0x22a,0x282,0x2da,0x332,0x38a,0x3e2,0x43a,0x492,0x4ea,0x542,0x59a,0x5f2,0x64a,0x6a2,0x6fa,0x752,0x7aa,0x802,0x85a,0x8b2,0x04e,0x0a6,0x0fe,0x156 },
	{ 0x45f,0x4b7,0x50f,0x567,0x5bf,0x617,0x66f,0x6c7,0x71f,0x777,0x7cf,0x827,0x87f,0x01b,0x073,0x0cb,0x123,0x17b,0x1d3,0x22b,0x283,0x2db,0x333,0x38b,0x3e3,0x43b,0x493,0x4eb,0x543,0x59b,0x5f3,0x64b,0x6a3,0x6fb,0x753,0x7ab,0x803,0x85b,0x8b3,0x04f,0x0a7,0x0ff,0x157 },
	{ 0x4b4,0x50c,0x564,0x5bc,0x614,0x66c,0x6c4,0x71c,0x774,0x7cc,0x824,0x87c,0x018,0x070,0x0c8,0x120,0x178,0x1d0,0x228,0x280,0x2d8,0x330,0x388,0x3e0,0x438,0x490,0x4e8,0x540,0x598,0x5f0,0x648,0x6a0,0x6f8,0x750,0x7a8,0x800,0x858,0x8b0,0x04c,0x0a4,0x0fc,0x154,0x1ac },
	{ 0x4b5,0x50d,0x565,0x5bd,0x615,0x66d,0x6c5,0x71d,0x775,0x7cd,0x825,0x87d,0x019,0x071,0x0c9,0x121,0x179,0x1d1,0x229,0x281,0x2d9,0x331,0x389,0x3e1,0x439,0x491,0x4e9,0x541,0x599,0x5f1,0x649,0x6a1,0x6f9,0x751,0x7a9,0x801,0x859,0x8b1,0x04d,0x0a5,0x0fd,0x155,0x1ad },
	{ 0x50a,0x562,0x5ba,0x612,0x66a,0x6c2,0x71a,0x772,0x7ca,0x822,0x87a,0x016,0x06e,0x0c6,0x11e,0x176,0x1ce,0x226,0x27e,0x2d6,0x32e,0x386,0x3de,0x436,0x48e,0x4e6,0x53e,0x596,0x5ee,0x646,0x69e,0x6f6,0x74e,0x7a6,0x7fe,0x856,0x8ae,0x04a,0x0a2,0x0fa,0x152,0x1aa,0x202 },
	{ 0x50b,0x563,0x5bb,0x613,0x66b,0x6c3,0x71b,0x773,0x7cb,0x823,0x87b,0x017,0x06f,0x0c7,0x11f,0x177,0x1cf,0x227,0x27f,0x2d7,0x32f,0x387,0x3df,0x437,0x48f,0x4e7,0x53f,0x597,0x5ef,0x647,0x69f,0x6f7,0x74f,0x7a7,0x7ff,0x857,0x8af,0x04b,0x0a3,0x0fb,0x153,0x1ab,0x203 },
	{ 0x560,0x5b8,0x610,0x668,0x6c0,0x718,0x770,0x7c8,0x820,0x878,0x014,0x06c,0x0c4,0x11c,0x174,0x1cc,0x224,0x27c,0x2d4,0x32c,0x384,0x3dc,0x434,0x48c,0x4e4,0x53c,0x594,0x5ec,0x644,0x69c,0x6f4,0x74c,0x7a4,0x7fc,0x854,0x8ac,0x048,0x0a0,0x0f8,0x150,0x1a8,0x200,0x258 },
	{ 0x561,0x5b9,0x611,0x669,0x6c1,0x719,0x771,0x7c9,0x821,0x879,0x015,0x06d,0x0c5,0x11d,0x175,0x1cd,0x225,0x27d,0x2d5,0x32d,0x385,0x3dd,0x435,0x48d,0x4e5,0x53d,0x595,0x5ed,0x645,0x69d,0x6f5,0x74d,0x7a5,0x7fd,0x855,0x8ad,0x049,0x0a1,0x0f9,0x151,0x1a9,0x201,0x259 },
	{ 0x5b6,0x60e,0x666,0x6be,0x716,0x76e,0x7c6,0x81e,0x876,0x012,0x06a,0x0c2,0x11a,0x172,0x1ca,0x222,0x27a,0x2d2,0x32a,0x382,0x3da,0x432,0x48a,0x4e2,0x53a,0x592,0x5ea,0x642,0x69a,0x6f2,0x74a,0x7a2,0x7fa,0x852,0x8aa,0x046,0x09e,0x0f6,0x14e,0x1a6,0x1fe,0x256,0x2ae },
	{ 0x5b7,0x60f,0x667,0x6bf,0x717,0x76f,0x7c7,0x81f,0x877,0x013,0x06b,0x0c3,0x11b,0x173,0x1cb,0x223,0x27b,0x2d3,0x32b,0x383,0x3db,0x433,0x48b,0x4e3,0x53b,0x593,0x5eb,0x643,0x69b,0x6f3,0x74b,0x7a3,0x7fb,0x853,0x8ab,0x047,0x09f,0x0f7,0x14f,0x1a7,0x1ff,0x257,0x2af },
	{ 0x60c,0x664,0x6bc,0x714,0x76c,0x7c4,0x81c,0x874,0x010,0x068,0x0c0,0x118,0x170,0x1c8,0x220,0x278,0x2d0,0x328,0x380,0x3d8,0x430,0x488,0x4e0,0x538,0x590,0x5e8,0x640,0x698,0x6f0,0x748,0x7a0,0x7f8,0x850,0x8a8,0x044,0x09c,0x0f4,0x14c,0x1a4,0x1fc,0x254,0x2ac,0x304 },
	{ 0x60d,0x665,0x6bd,0x715,0x76d,0x7c5,0x81d,0x875,0x011,0x069,0x0c1,0x119,0x171,0x1c9,0x221,0x279,0x2d1,0x329,0x381,0x3d9,0x431,0x489,0x4e1,0x539,0x591,0x5e9,0x641,0x699,0x6f1,0x749,0x7a1,0x7f9,0x851,0x8a9,0x045,0x09d,0x0f5,0x14d,0x1a5,0x1fd,0x255,0x2ad,0x305 },
	{ 0x662,0x6ba,0x712,0x76a,0x7c2,0x81a,0x872,0x00e,0x066,0x0be,0x116,0x16e,0x1c6,0x21e,0x276,0x2ce,0x326,0x37e,0x3d6,0x42e,0x486,0x4de,0x536,0x58e,0x5e6,0x63e,0x696,0x6ee,0x746,0x79e,0x7f6,0x84e,0x8a6,0x042,0x09a,0x0f2,0x14a,0x1a2,0x1fa,0x252,0x2aa,0x302,0x35a },
	{ 0x663,0x6bb,0x713,0x76b,0x7c3,0x81b,0x873,0x00f,0x067,0x0bf,0x117,0x16f,0x1c7,0x21f,0x277,0x2cf,0x327,0x37f,0x3d7,0x42f,0x487,0x4df,0x537,0x58f,0x5e7,0x63f,0x697,0x6ef,0x747,0x79f,0x7f7,0x84f,0x8a7,0x043,0x09b,0x0f3,0x14b,0x1a3,0x1fb,0x253,0x2ab,0x303,0x35b },
	{ 0x6b8,0x710,0x768,0x7c0,0x818,0x870,0x00c,0x064,0x0bc,0x114,0x16c,0x1c4,0x21c,0x274,0x2cc,0x324,0x37c,0x3d4,0x42c,0x484,0x4dc,0x534,0x58c,0x5e4,0x63c,0x694,0x6ec,0x744,0x79c,0x7f4,0x84c,0x8a4,0x040,0x098,0x0f0,0x148,0x1a0,0x1f8,0x250,0x2a8,0x300,0x358,0x3b0 },
	{ 0x6b9,0x711,0x769,0x7c1,0x819,0x871,0x00d,0x065,0x0bd,0x115,0x16d,0x1c5,0x21d,0x275,0x2cd,0x325,0x37d,0x3d5,0x42d,0x485,0x4dd,0x535,0x58d,0x5e5,0x63d,0x695,0x6ed,0x745,0x79d,0x7f5,0x84d,0x8a5,0x041,0x099,0x0f1,0x149,0x1a1,0x1f9,0x251,0x2a9,0x301,0x359,0x3b1 },
	{ 0x70e,0x766,0x7be,0x816,0x86e,0x00a,0x062,0x0ba,0x112,0x16a,0x1c2,0x21a,0x272,0x2ca,0x322,0x37a,0x3d2,0x42a,0x482,0x4da,0x532,0x58a,0x5e2,0x63a,0x692,0x6ea,0x742,0x79a,0x7f2,0x84a,0x8a2,0x03e,0x096,0x0ee,0x146,0x19e,0x1f6,0x24e,0x2a6,0x2fe,0x356,0x3ae,0x406 },
	{ 0x70f,0x767,0x7bf,0x817,0x86f,0x00b,0x063,0x0bb,0x113,0x16b,0x1c3,0x21b,0x273,0x2cb,0x323,0x37b,0x3d3,0x42b,0x483,0x4db,0x533,0x58b,0x5e3,0x63b,0x693,0x6eb,0x743,0x79b,0x7f3,0x84b,0x8a3,0x03f,0x097,0x0ef,0x147,0x19f,0x1f7,0x24f,0x2a7,0x2ff,0x357,0x3af,0x407 },
	{ 0x764,0x7bc,0x814,0x86c,0x008,0x060,0x0b8,0x110,0x168,0x1c0,0x218,0x270,0x2c8,0x320,0x378,0x3d0,0x428,0x480,0x4d8,0x530,0x588,0x5e0,0x638,0x690,0x6e8,0x740,0x798,0x7f0,0x848,0x8a0,0x03c,0x094,0x0ec,0x144,0x19c,0x1f4,0x24c,0x2a4,0x2fc,0x354,0x3ac,0x404,0x45c },
	{ 0x765,0x7bd,0x815,0x86d,0x009,0x061,0x0b9,0x111,0x169,0x1c1,0x219,0x271,0x2c9,0x321,0x379,0x3d1,0x429,0x481,0x4d9,0x531,0x589,0x5e1,0x639,0x691,0x6e9,0x741,0x799,0x7f1,0x849,0x8a1,0x03d,0x095,0x0ed,0x145,0x19d,0x1f5,0x24d,0x2a5,0x2fd,0x355,0x3ad,0x405,0x45d },
	{ 0x7ba,0x812,0x86a,0x006,0x05e,0x0b6,0x10e,0x166,0x1be,0x216,0x26e,0x2c6,0x31e,0x376,0x3ce,0x426,0x47e,0x4d6,0x52e,0x586,0x5de,0x636,0x68e,0x6e6,0x73e,0x796,0x7ee,0x846,0x89e,0x03a,0x092,0x0ea,0x142,0x19a,0x1f2,0x24a,0x2a2,0x2fa,0x352,0x3aa,0x402,0x45a,0x4b2 },
	{ 0x7bb,0x813,0x86b,0x007,0x05f,0x0b7,0x10f,0x167,0x1bf,0x217,0x26f,0x2c7,0x31f,0x377,0x3cf,0x427,0x47f,0x4d7,0x52f,0x587,0x5df,0x637,0x68f,0x6e7,0x73f,0x797,0x7ef,0x847,0x89f,0x03b,0x093,0x0eb,0x143,0x19b,0x1f3,0x24b,0x2a3,0x2fb,0x353,0x3ab,0x403,0x45b,0x4b3 },
	{ 0x810,0x868,0x004,0x05c,0x0b4,0x10c,0x164,0x1bc,0x214,0x26c,0x2c4,0x31c,0x374,0x3cc,0x424,0x47c,0x4d4,0x52c,0x584,0x5dc,0x634,0x68c,0x6e4,0x73c,0x794,0x7ec,0x844,0x89c,0x038,0x090,0x0e8,0x140,0x198,0x1f0,0x248,0x2a0,0x2f8,0x350,0x3a8,0x400,0x458,0x4b0,0x508 },
	{ 0x811,0x869,0x005,0x05d,0x0b5,0x10d,0x165,0x1bd,0x215,0x26d,0x2c5,0x31d,0x375,0x3cd,0x425,0x47d,0x4d5,0x52d,0x585,0x5dd,0x635,0x68d,0x6e5,0x73d,0x795,0x7ed,0x845,0x89d,0x039,0x091,0x0e9,0x141,0x199,0x1f1,0x249,0x2a1,0x2f9,0x351,0x3a9,0x401,0x459,0x4b1,0x509 },
	{ 0x866,0x002,0x05a,0x0b2,0x10a,0x162,0x1ba,0x212,0x26a,0x2c2,0x31a,0x372,0x3ca,0x422,0x47a,0x4d2,0x52a,0x582,0x5da,0x632,0x68a,0x6e2,0x73a,0x792,0x7ea,0x842,0x89a,0x036,0x08e,0x0e6,0x13e,0x196,0x1ee,0x246,0x29e,0x2f6,0x34e,0x3a6,0x3fe,0x456,0x4ae,0x506,0x55e },
	{ 0x867,0x003,0x05b,0x0b3,0x10b,0x163,0x1bb,0x213,0x26b,0x2c3,0x31b,0x373,0x3cb,0x423,0x47b,0x4d3,0x52b,0x583,0x5db,0x633,0x68b,0x6e3,0x73b,0x793,0x7eb,0x843,0x89b,0x037,0x08f,0x0e7,0x13f,0x197,0x1ef,0x247,0x29f,0x2f7,0x34f,0x3a7,0x3ff,0x457,0x4af,0x507,0x55f }
};


//-------------------------------------------------
//  ecc_source_byte - return data from the sector
//  at the given offset, masking anything
//  particular to a mode
//-------------------------------------------------

uint8_t cdrom_file::ecc_source_byte(const uint8_t *sector, uint32_t offset)
{
	// in mode 2 always treat these as 0 bytes
	return (sector[MODE_OFFSET] == 2 && offset < 4) ? 0x00 : sector[SYNC_OFFSET + SYNC_NUM_BYTES + offset];
}

/**
 * @fn  void ecc_compute_bytes(const uint8_t *sector, const uint16_t *row, int rowlen, uint8_t &val1, uint8_t &val2)
 *
 * @brief   -------------------------------------------------
 *            ecc_compute_bytes - calculate an ECC value (P or Q)
 *          -------------------------------------------------.
 *
 * @param   sector          The sector.
 * @param   row             The row.
 * @param   rowlen          The rowlen.
 * @param [in,out]  val1    The first value.
 * @param [in,out]  val2    The second value.
 */

void cdrom_file::ecc_compute_bytes(const uint8_t *sector, const uint16_t *row, int rowlen, uint8_t &val1, uint8_t &val2)
{
	val1 = val2 = 0;
	for (int component = 0; component < rowlen; component++)
	{
		val1 ^= ecc_source_byte(sector, row[component]);
		val2 ^= ecc_source_byte(sector, row[component]);
		val1 = ecclow[val1];
	}
	val1 = ecchigh[ecclow[val1] ^ val2];
	val2 ^= val1;
}

/**
 * @fn  bool ecc_verify(const uint8_t *sector)
 *
 * @brief   -------------------------------------------------
 *            ecc_verify - verify the P and Q ECC codes in a sector
 *          -------------------------------------------------.
 *
 * @param   sector  The sector.
 *
 * @return  true if it succeeds, false if it fails.
 */

bool cdrom_file::ecc_verify(const uint8_t *sector)
{
	// first verify P bytes
	for (int byte = 0; byte < ECC_P_NUM_BYTES; byte++)
	{
		uint8_t val1, val2;
		ecc_compute_bytes(sector, poffsets[byte], ECC_P_COMP, val1, val2);
		if (sector[ECC_P_OFFSET + byte] != val1 || sector[ECC_P_OFFSET + ECC_P_NUM_BYTES + byte] != val2)
			return false;
	}

	// then verify Q bytes
	for (int byte = 0; byte < ECC_Q_NUM_BYTES; byte++)
	{
		uint8_t val1, val2;
		ecc_compute_bytes(sector, qoffsets[byte], ECC_Q_COMP, val1, val2);
		if (sector[ECC_Q_OFFSET + byte] != val1 || sector[ECC_Q_OFFSET + ECC_Q_NUM_BYTES + byte] != val2)
			return false;
	}
	return true;
}

/**
 * @fn  void ecc_generate(uint8_t *sector)
 *
 * @brief   -------------------------------------------------
 *            ecc_generate - generate the P and Q ECC codes for a sector, overwriting any
 *            existing codes
 *          -------------------------------------------------.
 *
 * @param [in,out]  sector  If non-null, the sector.
 */

void cdrom_file::ecc_generate(uint8_t *sector)
{
	// first verify P bytes
	for (int byte = 0; byte < ECC_P_NUM_BYTES; byte++)
		ecc_compute_bytes(sector, poffsets[byte], ECC_P_COMP, sector[ECC_P_OFFSET + byte], sector[ECC_P_OFFSET + ECC_P_NUM_BYTES + byte]);

	// then verify Q bytes
	for (int byte = 0; byte < ECC_Q_NUM_BYTES; byte++)
		ecc_compute_bytes(sector, qoffsets[byte], ECC_Q_COMP, sector[ECC_Q_OFFSET + byte], sector[ECC_Q_OFFSET + ECC_Q_NUM_BYTES + byte]);
}

/**
 * @fn  void ecc_clear(uint8_t *sector)
 *
 * @brief   -------------------------------------------------
 *            ecc_clear - erase the ECC P and Q cods to 0 within a sector
 *          -------------------------------------------------.
 *
 * @param [in,out]  sector  If non-null, the sector.
 */

void cdrom_file::ecc_clear(uint8_t *sector)
{
	memset(&sector[ECC_P_OFFSET], 0, 2 * ECC_P_NUM_BYTES);
	memset(&sector[ECC_Q_OFFSET], 0, 2 * ECC_Q_NUM_BYTES);
}

/***************************************************************************

    TOC parser
    Handles CDRDAO .toc, CDRWIN .cue, Nero .nrg, and Sega GDROM .gdi

***************************************************************************/

/***************************************************************************
    CONSTANTS & DEFINES
***************************************************************************/

/**
 * @def TOKENIZE();
 *
 * @brief   A macro that defines tokenize.
 *
 * @param   linebuffer             The linebuffer.
 * @param   i                      Zero-based index of the.
 * @param   std::size(linebuffer)  The std::size(linebuffer)
 * @param   token                  The token.
 * @param   std::size(token)       The std::size(token)
 */

#define TOKENIZE i = tokenize( linebuffer, i, std::size(linebuffer), token, std::size(token) );


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/**
 * @fn  std::string get_file_path(std::string &path)
 *
 * @brief   Gets file path.
 *
 * @param [in,out]  path    Full pathname of the file.
 *
 * @return  The file path.
 */

std::string cdrom_file::get_file_path(std::string &path)
{
	int pos = path.find_last_of('\\');
	if (pos!=-1) {
		path = path.substr(0,pos+1);
	} else {
		pos = path.find_last_of('/');
		path = path.substr(0,pos+1);
	}
	return path;
}
/*-------------------------------------------------
    get_file_size - get the size of a file
-------------------------------------------------*/

/**
 * @fn  static uint64_t get_file_size(std::string_view filename)
 *
 * @brief   Gets file size.
 *
 * @param   filename    Filename of the file.
 *
 * @return  The file size.
 */

uint64_t cdrom_file::get_file_size(std::string_view filename)
{
	osd_file::ptr file;
	std::uint64_t filesize = 0;

	osd_file::open(std::string(filename), OPEN_FLAG_READ, file, filesize); // FIXME: allow osd_file to accept std::string_view

	return filesize;
}


/*-------------------------------------------------
    tokenize - get a token from the line buffer
-------------------------------------------------*/

/**
 * @fn  static int tokenize( const char *linebuffer, int i, int linebuffersize, char *token, int tokensize )
 *
 * @brief   Tokenizes.
 *
 * @param   linebuffer      The linebuffer.
 * @param   i               Zero-based index of the.
 * @param   linebuffersize  The linebuffersize.
 * @param [in,out]  token   If non-null, the token.
 * @param   tokensize       The tokensize.
 *
 * @return  An int.
 */

int cdrom_file::tokenize(const char *linebuffer, int i, int linebuffersize, char *token, int tokensize)
{
	int j = 0;
	bool singlequote = false;
	bool doublequote = false;

	while ((i < linebuffersize) && isspace((uint8_t)linebuffer[i]))
	{
		i++;
	}

	while ((i < linebuffersize) && (j < tokensize) && (linebuffer[i] != '\0'))
	{
		if (!singlequote && linebuffer[i] == '"')
		{
			doublequote = !doublequote;
		}
		else if (!doublequote && linebuffer[i] == '\'')
		{
			singlequote = !singlequote;
		}
		else if (!singlequote && !doublequote && isspace((uint8_t)linebuffer[i]))
		{
			break;
		}
		else
		{
			token[j] = linebuffer[i];
			j++;
		}

		i++;
	}

	token[j] = '\0';

	return i;
}


/*-------------------------------------------------
    msf_to_frames - convert m:s:f into a number of frames
-------------------------------------------------*/

/**
 * @fn  static int msf_to_frames( char *token )
 *
 * @brief   Msf to frames.
 *
 * @param [in,out]  token   If non-null, the token.
 *
 * @return  An int.
 */

int cdrom_file::msf_to_frames(const char *token)
{
	int m = 0;
	int s = 0;
	int f = 0;

	if (sscanf(token, "%d:%d:%d", &m, &s, &f ) == 1)
	{
		f = m;
	}
	else
	{
		// convert to just frames
		s += (m * 60);
		f += (s * 75);
	}

	return f;
}

/*-------------------------------------------------
    parse_wav_sample - takes a .WAV file, verifies
    that the file is 16 bits, and returns the
    length in bytes of the data and the offset in
    bytes to where the data starts in the file.
-------------------------------------------------*/

/**
 * @fn  static uint32_t parse_wav_sample(std::string_view filename, uint32_t *dataoffs)
 *
 * @brief   Parse WAV sample.
 *
 * @param   filename            Filename of the file.
 * @param [in,out]  dataoffs    If non-null, the dataoffs.
 *
 * @return  An uint32_t.
 */

uint32_t cdrom_file::parse_wav_sample(std::string_view filename, uint32_t *dataoffs)
{
	unsigned long offset = 0;
	uint32_t length, rate, filesize;
	uint16_t bits, temp16;
	char buf[32];
	osd_file::ptr file;
	uint64_t fsize = 0;
	std::uint32_t actual;

	std::string fname = std::string(filename);
	std::error_condition const filerr = osd_file::open(fname, OPEN_FLAG_READ, file, fsize);
	if (filerr)
	{
		printf("ERROR: could not open (%s)\n", fname.c_str());
		return 0;
	}

	/* read the core header and make sure it's a WAVE file */
	file->read(buf, 0, 4, actual);
	offset += actual;
	if (offset < 4)
	{
		printf("ERROR: unexpected RIFF offset %lu (%s)\n", offset, fname.c_str());
		return 0;
	}
	if (memcmp(&buf[0], "RIFF", 4) != 0)
	{
		printf("ERROR: could not find RIFF header (%s)\n", fname.c_str());
		return 0;
	}

	/* get the total size */
	file->read(&filesize, offset, 4, actual);
	offset += actual;
	if (offset < 8)
	{
		printf("ERROR: unexpected size offset %lu (%s)\n", offset, fname.c_str());
		return 0;
	}
	filesize = little_endianize_int32(filesize);

	/* read the RIFF file type and make sure it's a WAVE file */
	file->read(buf, offset, 4, actual);
	offset += actual;
	if (offset < 12)
	{
		printf("ERROR: unexpected WAVE offset %lu (%s)\n", offset, fname.c_str());
		return 0;
	}
	if (memcmp(&buf[0], "WAVE", 4) != 0)
	{
		printf("ERROR: could not find WAVE header (%s)\n", fname.c_str());
		return 0;
	}

	/* seek until we find a format tag */
	while (true)
	{
		file->read(buf, offset, 4, actual);
		offset += actual;
		file->read(&length, offset, 4, actual);
		offset += actual;
		length = little_endianize_int32(length);
		if (memcmp(&buf[0], "fmt ", 4) == 0)
			break;

		/* seek to the next block */
		offset += length;
		if (offset >= filesize)
		{
			printf("ERROR: could not find fmt tag (%s)\n", fname.c_str());
			return 0;
		}
	}

	/* read the format -- make sure it is PCM */
	file->read(&temp16, offset, 2, actual);
	offset += actual;
	temp16 = little_endianize_int16(temp16);
	if (temp16 != 1)
	{
		printf("ERROR: unsupported format %u - only PCM is supported (%s)\n", temp16, fname.c_str());
		return 0;
	}

	/* number of channels -- only stereo is supported */
	file->read(&temp16, offset, 2, actual);
	offset += actual;
	temp16 = little_endianize_int16(temp16);
	if (temp16 != 2)
	{
		printf("ERROR: unsupported number of channels %u - only stereo is supported (%s)\n", temp16, fname.c_str());
		return 0;
	}

	/* sample rate */
	file->read(&rate, offset, 4, actual);
	offset += actual;
	rate = little_endianize_int32(rate);
	if (rate != 44100)
	{
		printf("ERROR: unsupported samplerate %u - only 44100 is supported (%s)\n", rate, fname.c_str());
		return 0;
	}

	/* bytes/second and block alignment are ignored */
	file->read(buf, offset, 6, actual);
	offset += actual;

	/* bits/sample */
	file->read(&bits, offset, 2, actual);
	offset += actual;
	bits = little_endianize_int16(bits);
	if (bits != 16)
	{
		printf("ERROR: unsupported bits/sample %u - only 16 is supported (%s)\n", bits, fname.c_str());
		return 0;
	}

	/* seek past any extra data */
	offset += length - 16;

	/* seek until we find a data tag */
	while (true)
	{
		file->read(buf, offset, 4, actual);
		offset += actual;
		file->read(&length, offset, 4, actual);
		offset += actual;
		length = little_endianize_int32(length);
		if (memcmp(&buf[0], "data", 4) == 0)
			break;

		/* seek to the next block */
		offset += length;
		if (offset >= filesize)
		{
			printf("ERROR: could not find data tag (%s)\n", fname.c_str());
			return 0;
		}
	}

	/* if there was a 0 length data block, we're done */
	if (length == 0)
	{
		printf("ERROR: empty data block (%s)\n", fname.c_str());
		return 0;
	}

	*dataoffs = offset;

	return length;
}

/**
 * @fn  uint16_t read_uint16(FILE *infile)
 *
 * @brief   Reads uint 16.
 *
 * @param [in,out]  infile  If non-null, the infile.
 *
 * @return  The uint 16.
 */

uint16_t cdrom_file::read_uint16(FILE *infile)
{
	unsigned char buffer[2];

	fread(buffer, 2, 1, infile);

	return get_u16be(buffer);
}

/**
 * @fn  uint32_t read_uint32(FILE *infile)
 *
 * @brief   Reads uint 32.
 *
 * @param [in,out]  infile  If non-null, the infile.
 *
 * @return  The uint 32.
 */

uint32_t cdrom_file::read_uint32(FILE *infile)
{
	unsigned char buffer[4];

	fread(buffer, 4, 1, infile);

	return get_u32be(buffer);
}

/**
 * @fn  uint64_t read_uint64(FILE *infile)
 *
 * @brief   Reads uint 64.
 *
 * @param [in,out]  infile  If non-null, the infile.
 *
 * @return  The uint 64.
 */

uint64_t cdrom_file::read_uint64(FILE *infile)
{
	unsigned char buffer[8];

	fread(buffer, 8, 1, infile);

	return get_u64be(buffer);
}

/*-------------------------------------------------
    parse_nero - parse a Nero .NRG file
-------------------------------------------------*/

/**
 * @fn  std::error_condition parse_nero(std::string_view tocfname, toc &outtoc, track_input_info &outinfo)
 *
 * @brief   Chdcd parse nero.
 *
 * @param   tocfname        The tocfname.
 * @param [in,out]  outtoc  The outtoc.
 * @param [in,out]  outinfo The outinfo.
 *
 * @return  A std::error_condition.
 */

std::error_condition cdrom_file::parse_nero(std::string_view tocfname, toc &outtoc, track_input_info &outinfo)
{
	unsigned char buffer[12];
	uint32_t chain_offs, chunk_size;
	int done = 0;

	std::string path = std::string(tocfname);

	FILE *infile = fopen(path.c_str(), "rb");
	if (!infile)
	{
		return std::error_condition(errno, std::generic_category());
	}

	path = get_file_path(path);

	/* clear structures */
	memset(&outtoc, 0, sizeof(outtoc));
	outinfo.reset();

	outtoc.numsessions = 1;

	// seek to 12 bytes before the end
	fseek(infile, -12, SEEK_END);
	fread(buffer, 12, 1, infile);

	if (memcmp(buffer, "NER5", 4))
	{
		printf("ERROR: Not a Nero 5.5 or later image!\n");
		fclose(infile);
		return chd_file::error::UNSUPPORTED_FORMAT;
	}

	chain_offs = get_u32be(&buffer[8]);

	if ((buffer[7] != 0) || (buffer[6] != 0) || (buffer[5] != 0) || (buffer[4] != 0))
	{
		printf("ERROR: File size is > 4GB, this version of CHDMAN cannot handle it.");
		fclose(infile);
		return chd_file::error::UNSUPPORTED_FORMAT;
	}

//  printf("NER5 detected, chain offset: %x\n", chain_offs);

	while (!done)
	{
		fseek(infile, chain_offs, SEEK_SET);
		fread(buffer, 8, 1, infile);

		chunk_size = get_u32be(&buffer[4]);

//      printf("Chunk type: %c%c%c%c, size %x\n", buffer[0], buffer[1], buffer[2], buffer[3], chunk_size);

		// we want the DAOX chunk, which has the TOC information
		if (!memcmp(buffer, "DAOX", 4))
		{
			// skip second chunk size and UPC code
			fseek(infile, 20, SEEK_CUR);

			uint8_t start, end;
			fread(&start, 1, 1, infile);
			fread(&end, 1, 1, infile);

//          printf("Start track %d  End track: %d\n", start, end);

			outtoc.numtrks = (end-start) + 1;

			uint32_t offset = 0;
			for (int track = start; track <= end; track++)
			{
				uint32_t size, mode;
				uint64_t index0, index1, track_end;

				fseek(infile, 12, SEEK_CUR);    // skip ISRC code
				size = read_uint16(infile);
				mode = read_uint16(infile);
				fseek(infile, 2, SEEK_CUR);
				index0 = read_uint64(infile);
				index1 = read_uint64(infile);
				track_end = read_uint64(infile);

//              printf("Track %d: sector size %d mode %x index0 %llx index1 %llx track_end %llx (pregap %d sectors, length %d sectors)\n", track, size, mode, index0, index1, track_end, (uint32_t)(index1-index0)/size, (uint32_t)(track_end-index1)/size);
				outinfo.track[track-1].fname.assign(tocfname);
				outinfo.track[track-1].offset = offset + (uint32_t)(index1-index0);
				outinfo.track[track-1].idx[0] = outinfo.track[track-1].idx[1] = 0;

				switch (mode)
				{
					case 0x0000:    // 2048 byte data
						outtoc.tracks[track-1].trktype = CD_TRACK_MODE1;
						outinfo.track[track-1].swap = false;
						break;

					case 0x0300:    // Mode 2 Form 1
						printf("ERROR: Mode 2 Form 1 tracks not supported\n");
						fclose(infile);
						return chd_file::error::UNSUPPORTED_FORMAT;

					case 0x0500:    // raw data
						printf("ERROR: Raw data tracks not supported\n");
						fclose(infile);
						return chd_file::error::UNSUPPORTED_FORMAT;

					case 0x0600:    // 2352 byte mode 2 raw
						outtoc.tracks[track-1].trktype = CD_TRACK_MODE2_RAW;
						outinfo.track[track-1].swap = false;
						break;

					case 0x0700:    // 2352 byte audio
						outtoc.tracks[track-1].trktype = CD_TRACK_AUDIO;
						outinfo.track[track-1].swap = true;
						break;

					case 0x0f00:    // raw data with sub-channel
						printf("ERROR: Raw data tracks with sub-channel not supported\n");
						fclose(infile);
						return chd_file::error::UNSUPPORTED_FORMAT;

					case 0x1000:    // audio with sub-channel
						printf("ERROR: Audio tracks with sub-channel not supported\n");
						fclose(infile);
						return chd_file::error::UNSUPPORTED_FORMAT;

					case 0x1100:    // raw Mode 2 Form 1 with sub-channel
						printf("ERROR: Raw Mode 2 Form 1 tracks with sub-channel not supported\n");
						fclose(infile);
						return chd_file::error::UNSUPPORTED_FORMAT;

					default:
						printf("ERROR: Unknown track type %x, contact MAMEDEV!\n", mode);
						fclose(infile);
						return chd_file::error::UNSUPPORTED_FORMAT;
				}

				outtoc.tracks[track-1].datasize = size;

				outtoc.tracks[track-1].subtype = CD_SUB_NONE;
				outtoc.tracks[track-1].subsize = 0;

				outtoc.tracks[track-1].pregap = (uint32_t)(index1-index0)/size;
				outtoc.tracks[track-1].frames = (uint32_t)(track_end-index1)/size;
				outtoc.tracks[track-1].postgap = 0;
				outtoc.tracks[track-1].pgtype = 0;
				outtoc.tracks[track-1].pgsub = CD_SUB_NONE;
				outtoc.tracks[track-1].pgdatasize = 0;
				outtoc.tracks[track-1].pgsubsize = 0;
				outtoc.tracks[track-1].padframes = 0;

				offset += (uint32_t)track_end-index1;
			}
		}

		if (!memcmp(buffer, "END!", 4))
		{
			done = 1;
		}
		else
		{
			chain_offs += chunk_size + 8;
		}
	}

	fclose(infile);

	return std::error_condition();
}

/*-------------------------------------------------
    parse_iso - parse a .ISO file
-------------------------------------------------*/

/**
 * @fn  std::error_condition parse_iso(std::string_view tocfname, toc &outtoc, track_input_info &outinfo)
 *
 * @brief   Parse ISO.
 *
 * @param   tocfname        The tocfname.
 * @param [in,out]  outtoc  The outtoc.
 * @param [in,out]  outinfo The outinfo.
 *
 * @return  A std::error_condition.
 */

std::error_condition cdrom_file::parse_iso(std::string_view tocfname, toc &outtoc, track_input_info &outinfo)
{
	std::string path = std::string(tocfname);

	FILE *infile = fopen(path.c_str(), "rb");
	if (!infile)
	{
		return std::error_condition(errno, std::generic_category());
	}

	path = get_file_path(path);

	/* clear structures */
	memset(&outtoc, 0, sizeof(outtoc));
	outinfo.reset();

	uint64_t size = get_file_size(tocfname);
	fclose(infile);


	outtoc.numtrks = 1;
	outtoc.numsessions = 1;

	outinfo.track[0].fname = tocfname;
	outinfo.track[0].offset = 0;
	outinfo.track[0].idx[0] = outinfo.track[0].idx[1] = 0;

	if ((size % 2048)==0 ) {
		outtoc.tracks[0].trktype = CD_TRACK_MODE1;
		outtoc.tracks[0].frames = size / 2048;
		outtoc.tracks[0].datasize = 2048;
		outinfo.track[0].swap = false;
	} else if ((size % 2336)==0 ) {
		// 2352 byte mode 2
		outtoc.tracks[0].trktype = CD_TRACK_MODE2;
		outtoc.tracks[0].frames = size / 2336;
		outtoc.tracks[0].datasize = 2336;
		outinfo.track[0].swap = false;
	} else if ((size % 2352)==0 ) {
		// 2352 byte mode 2 raw
		outtoc.tracks[0].trktype = CD_TRACK_MODE2_RAW;
		outtoc.tracks[0].frames = size / 2352;
		outtoc.tracks[0].datasize = 2352;
		outinfo.track[0].swap = false;
	} else {
		printf("ERROR: Unrecognized track type\n");
		return chd_file::error::UNSUPPORTED_FORMAT;
	}

	outtoc.tracks[0].subtype = CD_SUB_NONE;
	outtoc.tracks[0].subsize = 0;

	outtoc.tracks[0].pregap = 0;

	outtoc.tracks[0].postgap = 0;
	outtoc.tracks[0].pgtype = 0;
	outtoc.tracks[0].pgsub = CD_SUB_NONE;
	outtoc.tracks[0].pgdatasize = 0;
	outtoc.tracks[0].pgsubsize = 0;
	outtoc.tracks[0].padframes = 0;


	return std::error_condition();
}

/*-------------------------------------------------
    parse_gdi - parse a Sega GD-ROM rip
-------------------------------------------------*/

/**
 * @fn  static std::error_condition parse_gdi(std::string_view tocfname, toc &outtoc, track_input_info &outinfo)
 *
 * @brief   Chdcd parse GDI.
 *
 * @param   tocfname        The tocfname.
 * @param [in,out]  outtoc  The outtoc.
 * @param [in,out]  outinfo The outinfo.
 *
 * @return  A std::error_condition.
 */

std::error_condition cdrom_file::parse_gdi(std::string_view tocfname, toc &outtoc, track_input_info &outinfo)
{
	int i, numtracks;

	std::string path = std::string(tocfname);

	FILE *infile = fopen(path.c_str(), "rt");
	if (!infile)
	{
		return std::error_condition(errno, std::generic_category());
	}

	path = get_file_path(path);

	/* clear structures */
	memset(&outtoc, 0, sizeof(outtoc));
	outinfo.reset();

	outtoc.flags = CD_FLAG_GDROM;

	char linebuffer[512];
	memset(linebuffer, 0, sizeof(linebuffer));

	fgets(linebuffer,511,infile);
	numtracks=atoi(linebuffer);

	for(i=0;i<numtracks;++i)
	{
		char *tok;
		int trknum;
		int trksize,trktype;
		int sz;

		fgets(linebuffer,511,infile);

		tok=strtok(linebuffer," ");

		trknum=atoi(tok)-1;

		outinfo.track[trknum].swap=false;
		outinfo.track[trknum].offset=0;

		outtoc.tracks[trknum].datasize = 0;
		outtoc.tracks[trknum].subtype = CD_SUB_NONE;
		outtoc.tracks[trknum].subsize = 0;
		outtoc.tracks[trknum].pgsub = CD_SUB_NONE;

		tok=strtok(nullptr," ");
		outtoc.tracks[trknum].physframeofs=atoi(tok);

		tok=strtok(nullptr," ");
		trktype=atoi(tok);

		tok=strtok(nullptr," ");
		trksize=atoi(tok);

		if(trktype==4 && trksize==2352)
		{
			outtoc.tracks[trknum].trktype=CD_TRACK_MODE1_RAW;
			outtoc.tracks[trknum].datasize=2352;
		}
		if(trktype==4 && trksize==2048)
		{
			outtoc.tracks[trknum].trktype=CD_TRACK_MODE1;
			outtoc.tracks[trknum].datasize=2048;
		}
		if(trktype==0)
		{
			outtoc.tracks[trknum].trktype=CD_TRACK_AUDIO;
			outtoc.tracks[trknum].datasize=2352;
			outinfo.track[trknum].swap = true;
		}

		std::string name;

		tok=strtok(nullptr," ");
		name = tok;
		if (tok[0]=='"') {
			do {
				tok=strtok(nullptr," ");
				if (tok!=nullptr) {
					name += " ";
					name += tok;
				}
			} while(tok!=nullptr && (strrchr(tok,'"')-tok !=(strlen(tok)-1)));
			strdelchr(name,'"');
		}
		outinfo.track[trknum].fname.assign(path).append(name);

		sz = get_file_size(outinfo.track[trknum].fname);

		outtoc.tracks[trknum].frames = sz/trksize;
		outtoc.tracks[trknum].padframes = 0;

		if (trknum != 0)
		{
			int dif=outtoc.tracks[trknum].physframeofs-(outtoc.tracks[trknum-1].frames+outtoc.tracks[trknum-1].physframeofs);
			outtoc.tracks[trknum-1].frames += dif;
			outtoc.tracks[trknum-1].padframes = dif;
		}
	}

	if (EXTRA_VERBOSE)
		for(i=0; i < numtracks; i++)
		{
			printf("%s %d %d %d (true %d)\n", outinfo.track[i].fname.c_str(), outtoc.tracks[i].frames, outtoc.tracks[i].padframes, outtoc.tracks[i].physframeofs, outtoc.tracks[i].frames - outtoc.tracks[i].padframes);
		}

	/* close the input TOC */
	fclose(infile);

	/* store the number of tracks found */
	outtoc.numtrks = numtracks;
	outtoc.numsessions = 1;

	return std::error_condition();
}

/*-------------------------------------------------
    parse_cue - parse a .CUE file
-------------------------------------------------*/

/**
 * @fn  std::error_condition parse_cue(std::string_view tocfname, toc &outtoc, track_input_info &outinfo)
 *
 * @brief   Chdcd parse cue.
 *
 * @param   tocfname        The tocfname.
 * @param [in,out]  outtoc  The outtoc.
 * @param [in,out]  outinfo The outinfo.
 *
 * @return  A std::error_condition.
 *
 * Redump multi-CUE for Dreamcast GDI:
 * Dreamcast discs have two images on a single disc. The first image is SINGLE-DENSITY and the second image
 * is HIGH-DENSITY. The SINGLE-DENSITY area starts 0 LBA and HIGH-DENSITY area starts 45000 LBA.
 */

std::error_condition cdrom_file::parse_cue(std::string_view tocfname, toc &outtoc, track_input_info &outinfo)
{
	int i, trknum, sessionnum, session_pregap;
	char token[512];
	std::string lastfname;
	uint32_t wavlen, wavoffs;
	std::string path = std::string(tocfname);
	const bool is_gdrom = is_gdicue(tocfname);
	enum gdi_area current_area = SINGLE_DENSITY;
	bool is_multibin = false;
	int leadin = -1;

	FILE *infile = fopen(path.c_str(), "rt");
	if (!infile)
	{
		return std::error_condition(errno, std::generic_category());
	}

	path = get_file_path(path);

	/* clear structures */
	memset(&outtoc, 0, sizeof(outtoc));
	outinfo.reset();

	trknum = -1;
	wavoffs = wavlen = 0;
	sessionnum = 0;
	session_pregap = 0;

	if (is_gdrom)
	{
		outtoc.flags = CD_FLAG_GDROM;
	}

	char linebuffer[512];
	memset(linebuffer, 0, sizeof(linebuffer));

	while (!feof(infile))
	{
		/* get the next line */
		if (!fgets(linebuffer, 511, infile))
			break;

		/* if EOF didn't hit, keep going */
		if (!feof(infile))
		{
			i = 0;

			TOKENIZE

			if (!strcmp(token, "REM"))
			{
				/* skip to actual data of REM command */
				while (i < std::size(linebuffer) && isspace((uint8_t)linebuffer[i]))
					i++;

				if (!strncmp(linebuffer+i, "SESSION", 7))
				{
					/* IsoBuster extension */
					TOKENIZE

					/* get the session number */
					TOKENIZE

					sessionnum = strtoul(token, nullptr, 10) - 1;

					if (sessionnum >= 1) /* don't consider it a multisession CD unless there's actually more than 1 session */
						outtoc.flags |= CD_FLAG_MULTISESSION;
				}
				else if ((outtoc.flags & CD_FLAG_MULTISESSION) && !strncmp(linebuffer+i, "PREGAP", 6))
				{
					/*
					Redump extension? PREGAP associated with the session instead of the track

					DiscImageCreator - Older versions would write a bogus value here (and maybe session lead-in and lead-out).
					These should be considered bad dumps and will not be supported.
					*/
					TOKENIZE

					/* get pregap time */
					TOKENIZE
					session_pregap = msf_to_frames(token);
				}
				else if (!strncmp(linebuffer+i, "LEAD-OUT", 8))
				{
					/*
					IsoBuster and ImgBurn (single bin file) - Lead-out time is the start of the lead-out
					lead-out time - MSF of last track of session = size of last track

					Redump and DiscImageCreator (multiple bins) - Lead-out time is the duration of just the lead-out
					*/
					TOKENIZE

					/* get lead-out time */
					TOKENIZE
					int leadout_offset = msf_to_frames(token);
					outinfo.track[trknum].leadout = leadout_offset;
				}
				else if (!strncmp(linebuffer+i, "LEAD-IN", 7))
				{
					/*
					IsoBuster and ImgBurn (single bin file) - Not used?
					Redump and DiscImageCreator (multiple bins) - Lead-in time is the duration of just the lead-in
					*/
					TOKENIZE

					/* get lead-in time */
					TOKENIZE
					leadin = msf_to_frames(token);
				}
				else if (is_gdrom && !strncmp(linebuffer+i, "SINGLE-DENSITY AREA", 19))
				{
					/* single-density area starts LBA = 0 */
					current_area = SINGLE_DENSITY;
				}
				else if (is_gdrom && !strncmp(linebuffer+i, "HIGH-DENSITY AREA", 17))
				{
					/* high-density area starts LBA = 45000 */
					current_area = HIGH_DENSITY;
				}
			}
			else if (!strcmp(token, "FILE"))
			{
				/* found the data file for a track */
				TOKENIZE

				/* keep the filename */
				std::string prevfname = lastfname;
				lastfname.assign(path).append(token);
				if (prevfname.length() > 0 && lastfname.compare(prevfname) != 0)
					is_multibin = true;

				/* get the file type */
				TOKENIZE

				if (!strcmp(token, "BINARY"))
				{
					outinfo.track[trknum+1].swap = false;
				}
				else if (!strcmp(token, "MOTOROLA"))
				{
					outinfo.track[trknum+1].swap = true;
				}
				else if (!strcmp(token, "WAVE"))
				{
					wavlen = parse_wav_sample(lastfname, &wavoffs);
					if (!wavlen)
					{
						fclose(infile);
						printf("ERROR: couldn't read [%s] or not a valid .WAV\n", lastfname.c_str());
						return chd_file::error::INVALID_DATA;
					}
				}
				else
				{
					fclose(infile);
					printf("ERROR: Unhandled track type %s\n", token);
					return chd_file::error::UNSUPPORTED_FORMAT;
				}
			}
			else if (!strcmp(token, "TRACK"))
			{
				/* get the track number */
				TOKENIZE
				trknum = strtoul(token, nullptr, 10) - 1;

				/* next token on the line is the track type */
				TOKENIZE

				outtoc.tracks[trknum].session = sessionnum;
				outtoc.tracks[trknum].subtype = CD_SUB_NONE;
				outtoc.tracks[trknum].subsize = 0;
				outtoc.tracks[trknum].pgsub = CD_SUB_NONE;
				outtoc.tracks[trknum].pregap = 0;
				outtoc.tracks[trknum].padframes = 0;
				outtoc.tracks[trknum].datasize = 0;
				outtoc.tracks[trknum].multicuearea = is_gdrom ? current_area : 0;
				outinfo.track[trknum].offset = 0;
				std::fill(std::begin(outinfo.track[trknum].idx), std::end(outinfo.track[trknum].idx), -1);

				outinfo.track[trknum].leadout = -1;
				outinfo.track[trknum].leadin = leadin; /* use previously saved lead-in value */
				leadin = -1;

				if (session_pregap != 0)
				{
					/*
					associated the pregap from the session transition with the lead-in to simplify things for now.
					setting it as the proper pregap for the track causes logframeofs of the last dummy entry in the TOC
					to become 2s later than it should. this might be an issue with how pgdatasize = 0 pregaps are handled.
					*/
					if (outinfo.track[trknum].leadin == -1)
						outinfo.track[trknum].leadin = session_pregap;
					else
						outinfo.track[trknum].leadin += session_pregap;
					session_pregap = 0;
				}

				if (wavlen != 0)
				{
					outtoc.tracks[trknum].frames = wavlen/2352;
					outinfo.track[trknum].offset = wavoffs;
					wavoffs = wavlen = 0;
				}

				outinfo.track[trknum].fname.assign(lastfname); /* default filename to the last one */

				if (EXTRA_VERBOSE)
				{
					if (is_gdrom)
					{
						printf("trk %d: fname %s offset %d area %d\n", trknum, outinfo.track[trknum].fname.c_str(), outinfo.track[trknum].offset, outtoc.tracks[trknum].multicuearea);
					}
					else
					{
						printf("trk %d: fname %s offset %d\n", trknum, outinfo.track[trknum].fname.c_str(), outinfo.track[trknum].offset);
					}
				}

				convert_type_string_to_track_info(token, &outtoc.tracks[trknum]);
				if (outtoc.tracks[trknum].datasize == 0)
				{
					fclose(infile);
					printf("ERROR: Unknown track type [%s].  Contact MAMEDEV.\n", token);
					return chd_file::error::UNSUPPORTED_FORMAT;
				}

				/* next (optional) token on the line is the subcode type */
				TOKENIZE

				convert_subtype_string_to_track_info(token, &outtoc.tracks[trknum]);
			}
			else if (!strcmp(token, "INDEX"))
			{
				int idx, frames;

				/* get index number */
				TOKENIZE
				idx = strtoul(token, nullptr, 10);

				/* get index */
				TOKENIZE
				frames = msf_to_frames(token);

				if (idx < 0 || idx > MAX_INDEX)
				{
					printf("ERROR: encountered invalid index %d\n", idx);
					return chd_file::error::INVALID_DATA;
				}

				outinfo.track[trknum].idx[idx] = frames;

				if (idx == 1)
				{
					if (outtoc.tracks[trknum].pregap == 0 && outinfo.track[trknum].idx[0] != -1)
					{
						outtoc.tracks[trknum].pregap = frames - outinfo.track[trknum].idx[0];
						outtoc.tracks[trknum].pgtype = outtoc.tracks[trknum].trktype;
						outtoc.tracks[trknum].pgdatasize = outtoc.tracks[trknum].datasize;
					}
					else if (outinfo.track[trknum].idx[0] == -1) /* pregap sectors not in file, but we're always using idx 0 for track length calc now */
					{
						outinfo.track[trknum].idx[0] = frames;
					}
				}
			}
			else if (!strcmp(token, "PREGAP"))
			{
				int frames;

				/* get index */
				TOKENIZE
				frames = msf_to_frames(token);

				outtoc.tracks[trknum].pregap = frames;
			}
			else if (!strcmp(token, "POSTGAP"))
			{
				int frames;

				/* get index */
				TOKENIZE
				frames = msf_to_frames(token);

				outtoc.tracks[trknum].postgap = frames;
			}
			else if (!strcmp(token, "FLAGS"))
			{
				outtoc.tracks[trknum].control_flags = 0;

				/* keep looping over remaining tokens in FLAGS line until there's no more to read */
				while (i < std::size(linebuffer))
				{
					int last_idx = i;

					TOKENIZE

					if (i == last_idx)
						break;

					if (!strcmp(token, "DCP"))
						outtoc.tracks[trknum].control_flags |= CD_FLAG_CONTROL_DIGITAL_COPY_PERMITTED;
					else if (!strcmp(token, "4CH"))
						outtoc.tracks[trknum].control_flags |= CD_FLAG_CONTROL_4CH;
					else if (!strcmp(token, "PRE"))
						outtoc.tracks[trknum].control_flags |= CD_FLAG_CONTROL_PREEMPHASIS;
				}
			}
		}
	}

	/* close the input CUE */
	fclose(infile);

	/* store the number of tracks found */
	outtoc.numtrks = trknum + 1;
	outtoc.numsessions = sessionnum + 1;

	/* now go over the files again and set the lengths */
	for (trknum = 0; trknum < outtoc.numtrks; trknum++)
	{
		uint64_t tlen = 0;

		if (outinfo.track[trknum].idx[1] == -1)
		{
			/* index 1 should always be set */
			printf("ERROR: track %d is missing INDEX 01 marker\n", trknum+1);
			return chd_file::error::INVALID_DATA;
		}

		/* this is true for cue/bin and cue/iso, and we need it for cue/wav since .WAV is little-endian */
		if (outtoc.tracks[trknum].trktype == CD_TRACK_AUDIO)
		{
			outinfo.track[trknum].swap = true;
		}

		/* don't do this for .WAV tracks, we already have their length and offset filled out */
		if (outinfo.track[trknum].offset != 0)
			continue;

		if (trknum+1 >= outtoc.numtrks && trknum > 0 && (outinfo.track[trknum].fname.compare(outinfo.track[trknum-1].fname)==0))
		{
			/* if the last track's filename is the same as the previous track */
			tlen = get_file_size(outinfo.track[trknum].fname);
			if (tlen == 0)
			{
				printf("ERROR: couldn't find bin file [%s]\n", outinfo.track[trknum-1].fname.c_str());
				return std::errc::no_such_file_or_directory;
			}

			outinfo.track[trknum].offset = outinfo.track[trknum-1].offset + outtoc.tracks[trknum-1].frames * (outtoc.tracks[trknum-1].datasize + outtoc.tracks[trknum-1].subsize);
			outtoc.tracks[trknum].frames = (tlen - outinfo.track[trknum].offset) / (outtoc.tracks[trknum].datasize + outtoc.tracks[trknum].subsize);
		}
		else if (trknum+1 < outtoc.numtrks && outinfo.track[trknum].fname.compare(outinfo.track[trknum+1].fname)==0)
		{
			/* if the current filename is the same as the next track */
			outtoc.tracks[trknum].frames = outinfo.track[trknum+1].idx[0] - outinfo.track[trknum].idx[0];

			if (outtoc.tracks[trknum].frames == 0)
			{
				printf("ERROR: unable to determine size of track %d, missing INDEX 01 markers?\n", trknum+1);
				return chd_file::error::INVALID_DATA;
			}

			if (trknum > 0)
			{
				const uint32_t previous_track_raw_size = outtoc.tracks[trknum-1].frames * (outtoc.tracks[trknum-1].datasize + outtoc.tracks[trknum-1].subsize);
				outinfo.track[trknum].offset = outinfo.track[trknum-1].offset + previous_track_raw_size;
			}
		}
		else if (outtoc.tracks[trknum].frames == 0)
		{
			/* if the filenames between tracks are different */
			tlen = get_file_size(outinfo.track[trknum].fname);
			if (tlen == 0)
			{
				printf("ERROR: couldn't find bin file [%s]\n", outinfo.track[trknum-1].fname.c_str());
				return std::errc::no_such_file_or_directory;
			}

			outtoc.tracks[trknum].frames = tlen / (outtoc.tracks[trknum].datasize + outtoc.tracks[trknum].subsize);
			outinfo.track[trknum].offset = 0;
		}

		if (outtoc.flags & CD_FLAG_MULTISESSION)
		{
			if (is_multibin)
			{
				if (outinfo.track[trknum].leadout == -1 && trknum + 1 < outtoc.numtrks && outtoc.tracks[trknum].session != outtoc.tracks[trknum+1].session)
				{
					/* add a standard lead-out to the last track before changing sessions */
					outinfo.track[trknum].leadout = outtoc.tracks[trknum].session == 0 ? 6750 : 2250; /* first session lead-out (1m30s0f) is longer than the rest (0m30s0f) */
				}

				if (outinfo.track[trknum].leadin == -1 && trknum > 0 && outtoc.tracks[trknum].session != outtoc.tracks[trknum-1].session)
				{
					/* add a standard lead-in to the first track of a new session */
					outinfo.track[trknum].leadin = 4500; /* lead-in (1m0s0f) */
				}
			}
			else
			{
				if (outinfo.track[trknum].leadout != -1)
				{
					/*
					if a lead-out time is specified in a multisession CD then the size of the previous track needs to be trimmed
					to use the lead-out time instead of the idx 0 of the next track
					*/
					const int endframes = outinfo.track[trknum].leadout - outinfo.track[trknum].idx[0];
					if (outtoc.tracks[trknum].frames >= endframes)
					{
						outtoc.tracks[trknum].frames = endframes; /* trim track length */

						if (trknum + 1 < outtoc.numtrks)
						{
							/* lead-out value becomes just the duration between the lead-out to the pre-gap of the next track */
							outinfo.track[trknum].leadout = outinfo.track[trknum+1].idx[0] - outinfo.track[trknum].leadout;
						}
					}
				}

				if (trknum > 0 && outinfo.track[trknum-1].leadout != -1)
				{
					/*
					ImgBurn bin/cue have dummy data to pad between the lead-out and the start of the next track.
					DiscImageCreator img/cue does not have any data between the lead-out and the start of the next track.

					Detecting by extension is an awful way to handle this but there's no other way to determine what format
					the data will be in since we don't know the exact length of the last track just from the cue.
					*/
					if (!core_filename_ends_with(outinfo.track[trknum-1].fname, ".img"))
					{
						outtoc.tracks[trknum-1].padframes += outinfo.track[trknum-1].leadout;
						outtoc.tracks[trknum].frames -= outinfo.track[trknum-1].leadout;
						outinfo.track[trknum].offset += outinfo.track[trknum-1].leadout * (outtoc.tracks[trknum].datasize + outtoc.tracks[trknum].subsize);
					}
				}
			}
		}
	}

	if (is_gdrom)
	{
		/*
		* Strip pregaps from Redump tracks and adjust the LBA offset to match TOSEC layout
		*/
		for (trknum = 1; trknum < outtoc.numtrks; trknum++)
		{
			uint32_t this_pregap = outtoc.tracks[trknum].pregap;
			uint32_t this_offset = this_pregap * (outtoc.tracks[trknum].datasize + outtoc.tracks[trknum].subsize);

			outtoc.tracks[trknum-1].frames += this_pregap;
			outtoc.tracks[trknum-1].splitframes += this_pregap;

			outinfo.track[trknum].offset += this_offset;
			outtoc.tracks[trknum].frames -= this_pregap;
			outinfo.track[trknum].idx[1] -= this_pregap;

			outtoc.tracks[trknum].pregap = 0;
			outtoc.tracks[trknum].pgtype = 0;
		}

		/*
		* TOC now matches TOSEC layout, set LBA for every track with HIGH-DENSITY area @ LBA 45000
		*/
		for (trknum = 1; trknum < outtoc.numtrks; trknum++)
		{
			if (outtoc.tracks[trknum].multicuearea == HIGH_DENSITY && outtoc.tracks[trknum-1].multicuearea == SINGLE_DENSITY)
			{
				outtoc.tracks[trknum].physframeofs = 45000;
				int dif=outtoc.tracks[trknum].physframeofs-(outtoc.tracks[trknum-1].frames+outtoc.tracks[trknum-1].physframeofs);
				outtoc.tracks[trknum-1].frames += dif;
				outtoc.tracks[trknum-1].padframes = dif;
			}
			else
			{
				outtoc.tracks[trknum].physframeofs = outtoc.tracks[trknum-1].physframeofs + outtoc.tracks[trknum-1].frames;
			}
		}
	}

	if (EXTRA_VERBOSE)
	{
		for (trknum = 0; trknum < outtoc.numtrks; trknum++)
		{
			printf("session %d trk %d: %d frames @ offset %d, pad=%d, split=%d, area=%d, phys=%d, pregap=%d, pgtype=%d, pgdatasize=%d, idx0=%d, idx1=%d, dataframes=%d\n",
					outtoc.tracks[trknum].session + 1,
					trknum + 1,
					outtoc.tracks[trknum].frames,
					outinfo.track[trknum].offset,
					outtoc.tracks[trknum].padframes,
					outtoc.tracks[trknum].splitframes,
					outtoc.tracks[trknum].multicuearea,
					outtoc.tracks[trknum].physframeofs,
					outtoc.tracks[trknum].pregap,
					outtoc.tracks[trknum].pgtype,
					outtoc.tracks[trknum].pgdatasize,
					outinfo.track[trknum].idx[0],
					outinfo.track[trknum].idx[1],
					outtoc.tracks[trknum].frames - outtoc.tracks[trknum].padframes);
		}
	}

	return std::error_condition();
}

/*---------------------------------------------------------------------------------------
    is_gdicue - determine if CUE contains Redump multi-CUE format for Dreamcast GDI
----------------------------------------------------------------------------------------*/

/**
 * Dreamcast GDI has two images on one disc, SINGLE-DENSITY and HIGH-DENSITY.
 *
 * Redump stores both images in a single .cue with a REM comment separating the images.
 * This multi-cue format replaces the old flawed .gdi format.
 *
 *    http://forum.redump.org/topic/19969/done-sega-dreamcast-multicue-gdi/
 *
 * This function looks for strings "REM SINGLE-DENSITY AREA" & "REM HIGH-DENSITY AREA"
 * indicating the Redump multi-cue format and therefore a Dreamcast GDI disc.
 */

bool cdrom_file::is_gdicue(std::string_view tocfname)
{
	char token[512];
	bool has_rem_singledensity = false;
	bool has_rem_highdensity = false;
	std::string path = std::string(tocfname);

	FILE *infile = fopen(path.c_str(), "rt");
	if (!infile)
	{
		return false;
	}

	path = get_file_path(path);

	char linebuffer[512];
	memset(linebuffer, 0, sizeof(linebuffer));

	while (!feof(infile))
	{
		if (!fgets(linebuffer, 511, infile))
			break;

		/* if EOF didn't hit, keep going */
		if (!feof(infile))
		{
			int i = 0;

			TOKENIZE

			if (!strcmp(token, "REM"))
			{
				/* skip to actual data of REM command */
				while (i < std::size(linebuffer) && isspace((uint8_t)linebuffer[i]))
					i++;

				if (!strncmp(linebuffer+i, "SINGLE-DENSITY AREA", 19))
					has_rem_singledensity = true;
				else if (!strncmp(linebuffer+i, "HIGH-DENSITY AREA", 17))
					has_rem_highdensity = true;
			}
		}
	}

	fclose(infile);

	return has_rem_singledensity && has_rem_highdensity;
}

/*-------------------------------------------------
    parse_toc - parse a CDRDAO format TOC file
-------------------------------------------------*/

/**
 * @fn  std::error_condition parse_toc(std::string_view tocfname, toc &outtoc, track_input_info &outinfo)
 *
 * @brief   Chdcd parse TOC.
 *
 * @param   tocfname        The tocfname.
 * @param [in,out]  outtoc  The outtoc.
 * @param [in,out]  outinfo The outinfo.
 *
 * @return  A std::error_condition.
 */

std::error_condition cdrom_file::parse_toc(std::string_view tocfname, toc &outtoc, track_input_info &outinfo)
{
	char token[512];

	auto pos = tocfname.rfind('.');
	std::string tocfext = pos == std::string_view::npos ? std::string() : strmakelower(tocfname.substr(pos + 1));

	if (tocfext == "gdi")
	{
		return parse_gdi(tocfname, outtoc, outinfo);
	}

	if (tocfext == "cue")
	{
		return parse_cue(tocfname, outtoc, outinfo);
	}

	if (tocfext == "nrg")
	{
		return parse_nero(tocfname, outtoc, outinfo);
	}

	if (tocfext == "iso" || tocfext == "cdr" || tocfext == "toast")
	{
		return parse_iso(tocfname, outtoc, outinfo);
	}

	std::string path = std::string(tocfname);

	FILE *infile = fopen(path.c_str(), "rt");
	if (!infile)
	{
		return std::error_condition(errno, std::generic_category());
	}

	path = get_file_path(path);

	/* clear structures */
	memset(&outtoc, 0, sizeof(outtoc));
	outinfo.reset();

	int trknum = -1;

	char linebuffer[512];
	memset(linebuffer, 0, sizeof(linebuffer));

	while (!feof(infile))
	{
		/* get the next line */
		if (!fgets(linebuffer, 511, infile))
			break;

		/* if EOF didn't hit, keep going */
		if (!feof(infile))
		{
			int i = 0;

			TOKENIZE

			if ((!strcmp(token, "DATAFILE")) || (!strcmp(token, "AUDIOFILE")) || (!strcmp(token, "FILE")))
			{
				int f;

				/* found the data file for a track */
				TOKENIZE

				/* keep the filename */
				outinfo.track[trknum].fname.assign(path).append(token);

				/* get either the offset or the length */
				TOKENIZE

				if (!strcmp(token, "SWAP"))
				{
					TOKENIZE

					outinfo.track[trknum].swap = true;
				}
				else
				{
					outinfo.track[trknum].swap = false;
				}

				if (token[0] == '#')
				{
					/* it's a decimal offset, use it */
					f = strtoul(&token[1], nullptr, 10);
				}
				else if (isdigit((uint8_t)token[0]))
				{
					/* convert the time to an offset */
					f = msf_to_frames(token);

					f *= (outtoc.tracks[trknum].datasize + outtoc.tracks[trknum].subsize);
				}
				else
				{
					f = 0;
				}

				outinfo.track[trknum].offset = f;

				TOKENIZE

				if (isdigit((uint8_t)token[0]))
				{
					// this could be the length or an offset from the previous field.
					f = msf_to_frames(token);

					TOKENIZE

					if (isdigit((uint8_t)token[0]))
					{
						// it was an offset.
						f *= (outtoc.tracks[trknum].datasize + outtoc.tracks[trknum].subsize);

						outinfo.track[trknum].offset += f;

						// this is the length.
						f = msf_to_frames(token);
					}
				}
				else if( trknum == 0 && outinfo.track[trknum].offset != 0 )
				{
					/* the 1st track might have a length with no offset */
					f = outinfo.track[trknum].offset / (outtoc.tracks[trknum].datasize + outtoc.tracks[trknum].subsize);
					outinfo.track[trknum].offset = 0;
				}
				else
				{
					/* guesstimate the track length? */
					f = 0;
				}

				outtoc.tracks[trknum].frames = f;
			}
			else if (!strcmp(token, "TRACK"))
			{
				trknum++;

				/* next token on the line is the track type */
				TOKENIZE

				outtoc.tracks[trknum].trktype = CD_TRACK_MODE1;
				outtoc.tracks[trknum].datasize = 0;
				outtoc.tracks[trknum].subtype = CD_SUB_NONE;
				outtoc.tracks[trknum].subsize = 0;
				outtoc.tracks[trknum].pgsub = CD_SUB_NONE;
				outtoc.tracks[trknum].padframes = 0;

				convert_type_string_to_track_info(token, &outtoc.tracks[trknum]);
				if (outtoc.tracks[trknum].datasize == 0)
				{
					fclose(infile);
					printf("ERROR: Unknown track type [%s].  Contact MAMEDEV.\n", token);
					return chd_file::error::UNSUPPORTED_FORMAT;
				}

				/* next (optional) token on the line is the subcode type */
				TOKENIZE

				convert_subtype_string_to_track_info(token, &outtoc.tracks[trknum]);
			}
			else if (!strcmp(token, "START"))
			{
				int frames;

				/* get index */
				TOKENIZE
				frames = msf_to_frames(token);

				outtoc.tracks[trknum].pregap = frames;
			}
		}
	}

	/* close the input TOC */
	fclose(infile);

	/* store the number of tracks found */
	outtoc.numtrks = trknum + 1;
	outtoc.numsessions = 1;

	return std::error_condition();
}
