// license:BSD-3-Clause
// copyright-holders:Aaron Giles,R. Belmont
/***************************************************************************

    cdrom.h

    Generic MAME cd-rom implementation

***************************************************************************/
#ifndef MAME_LIB_UTIL_CDROM_H
#define MAME_LIB_UTIL_CDROM_H

#pragma once

#include "chd.h"
#include "ioprocs.h"
#include "osdcore.h"

#include <algorithm>
#include <string>
#include <string_view>
#include <system_error>


class cdrom_file {
public:
	// tracks are padded to a multiple of this many frames
	static constexpr uint32_t TRACK_PADDING    = 4;

	static constexpr uint32_t MAX_TRACKS       = 99;        /* AFAIK the theoretical limit */
	static constexpr uint32_t MAX_SECTOR_DATA  = 2352;
	static constexpr uint32_t MAX_SUBCODE_DATA = 96;
	static constexpr uint32_t MAX_INDEX        = 99;

	static constexpr uint32_t FRAME_SIZE       = MAX_SECTOR_DATA + MAX_SUBCODE_DATA;
	static constexpr uint32_t FRAMES_PER_HUNK  = 8;

	static constexpr uint32_t METADATA_WORDS   = 1 + MAX_TRACKS * 6;

	enum
	{
		CD_TRACK_MODE1 = 0,         /* mode 1 2048 bytes/sector */
		CD_TRACK_MODE1_RAW,         /* mode 1 2352 bytes/sector */
		CD_TRACK_MODE2,             /* mode 2 2336 bytes/sector */
		CD_TRACK_MODE2_FORM1,       /* mode 2 2048 bytes/sector */
		CD_TRACK_MODE2_FORM2,       /* mode 2 2324 bytes/sector */
		CD_TRACK_MODE2_FORM_MIX,    /* mode 2 2336 bytes/sector */
		CD_TRACK_MODE2_RAW,         /* mode 2 2352 bytes/sector */
		CD_TRACK_AUDIO,             /* redbook audio track 2352 bytes/sector (588 samples) */

		CD_TRACK_RAW_DONTCARE       /* special flag for cdrom_read_data: just return me whatever is there */
	};

	enum
	{
		CD_SUB_NORMAL = 0,          /* "cooked" 96 bytes per sector */
		CD_SUB_RAW,                 /* raw uninterleaved 96 bytes per sector */
		CD_SUB_NONE                 /* no subcode data stored */
	};

	enum
	{
		CD_FLAG_GDROM        = 0x00000001,  // disc is a GD-ROM, all tracks should be stored with GD-ROM metadata
		CD_FLAG_GDROMLE      = 0x00000002,  // legacy GD-ROM, with little-endian CDDA data
		CD_FLAG_MULTISESSION = 0x00000004,  // multisession CD-ROM
	};

	enum
	{
		CD_FLAG_CONTROL_PREEMPHASIS = 1,
		CD_FLAG_CONTROL_DIGITAL_COPY_PERMITTED = 2,
		CD_FLAG_CONTROL_DATA_TRACK = 4,
		CD_FLAG_CONTROL_4CH = 8,
	};

	enum
	{
		CD_FLAG_ADR_START_TIME = 1,
		CD_FLAG_ADR_CATALOG_CODE,
		CD_FLAG_ADR_ISRC_CODE,
	};

	struct track_info
	{
		/* fields used by CHDMAN and in MAME */
		uint32_t trktype;       /* track type */
		uint32_t subtype;       /* subcode data type */
		uint32_t datasize;      /* size of data in each sector of this track */
		uint32_t subsize;       /* size of subchannel data in each sector of this track */
		uint32_t frames;        /* number of frames in this track */
		uint32_t extraframes;   /* number of "spillage" frames in this track */
		uint32_t pregap;        /* number of pregap frames */
		uint32_t postgap;       /* number of postgap frames */
		uint32_t pgtype;        /* type of sectors in pregap */
		uint32_t pgsub;         /* type of subchannel data in pregap */
		uint32_t pgdatasize;    /* size of data in each sector of the pregap */
		uint32_t pgsubsize;     /* size of subchannel data in each sector of the pregap */
		uint32_t control_flags; /* metadata flags associated with each track */
		uint32_t session;       /* session number */

		/* fields used in CHDMAN only */
		uint32_t padframes;   /* number of frames of padding to add to the end of the track; needed for GDI */
		uint32_t splitframes; /* number of frames from the next file to add to the end of the current track after padding; needed for Redump split-bin GDI */

		/* fields used in MAME/MESS only */
		uint32_t logframeofs; /* logical frame of actual track data - offset by pregap size if pregap not physically present */
		uint32_t physframeofs; /* physical frame of actual track data in CHD data */
		uint32_t chdframeofs; /* frame number this track starts at on the CHD */
		uint32_t logframes; /* number of frames from logframeofs until end of track data */

		/* fields used in multi-cue GDI */
		uint32_t multicuearea;
	};


	struct toc
	{
		uint32_t numtrks;     /* number of tracks */
		uint32_t numsessions; /* number of sessions */
		uint32_t flags;       /* see FLAG_ above */
		track_info tracks[MAX_TRACKS + 1];
	};

	struct track_input_entry
	{
		track_input_entry() { reset(); }
		void reset() { fname.clear(); offset = 0; leadin = leadout = -1; swap = false; std::fill(std::begin(idx), std::end(idx), -1); }

		std::string fname;      // filename for each track
		uint32_t offset;      // offset in the data file for each track
		bool swap;          // data needs to be byte swapped
		int32_t idx[MAX_INDEX + 1];
		int32_t leadin, leadout; // TODO: these should probably be their own tracks entirely
	};

	struct track_input_info
	{
		void reset() { for (auto & elem : track) elem.reset(); }

		track_input_entry track[MAX_TRACKS];
	};


	cdrom_file(chd_file *chd);
	cdrom_file(std::string_view inputfile);
	~cdrom_file();


	/* core read access */
	bool read_data(uint32_t lbasector, void *buffer, uint32_t datatype, bool phys=false);
	bool read_subcode(uint32_t lbasector, void *buffer, bool phys=false);

	/* handy utilities */
	uint32_t get_track(uint32_t frame) const;
	uint32_t get_track_start(uint32_t track) const {return cdtoc.tracks[track == 0xaa ? cdtoc.numtrks : track].logframeofs; }
	uint32_t get_track_start_phys(uint32_t track) const { return cdtoc.tracks[track == 0xaa ? cdtoc.numtrks : track].physframeofs; }
	uint32_t get_track_index(uint32_t frame) const;

	/* TOC utilities */
	static std::error_condition parse_nero(std::string_view tocfname, toc &outtoc, track_input_info &outinfo);
	static std::error_condition parse_iso(std::string_view tocfname, toc &outtoc, track_input_info &outinfo);
	static std::error_condition parse_gdi(std::string_view tocfname, toc &outtoc, track_input_info &outinfo);
	static std::error_condition parse_cue(std::string_view tocfname, toc &outtoc, track_input_info &outinfo);
	static bool is_gdicue(std::string_view tocfname);
	static std::error_condition parse_toc(std::string_view tocfname, toc &outtoc, track_input_info &outinfo);
	int get_last_session() const { return cdtoc.numsessions ? cdtoc.numsessions : 1; }
	int get_last_track() const { return cdtoc.numtrks; }
	int get_adr_control(int track) const
	{
		if (track == 0xaa)
			track = get_last_track() - 1; // use last track's flags
		int adrctl = (CD_FLAG_ADR_START_TIME << 4) | (cdtoc.tracks[track].control_flags & 0x0f);
		if (cdtoc.tracks[track].trktype != CD_TRACK_AUDIO)
			adrctl |= CD_FLAG_CONTROL_DATA_TRACK;
		return adrctl;
	}
	int get_track_type(int track) const { return cdtoc.tracks[track].trktype; }
	const toc &get_toc() const { return cdtoc; }

	/* extra utilities */
	static void convert_type_string_to_track_info(const char *typestring, track_info *info);
	static void convert_type_string_to_pregap_info(const char *typestring, track_info *info);
	static void convert_subtype_string_to_track_info(const char *typestring, track_info *info);
	static void convert_subtype_string_to_pregap_info(const char *typestring, track_info *info);
	static const char *get_type_string(uint32_t trktype);
	static const char *get_subtype_string(uint32_t subtype);
	static std::error_condition parse_metadata(chd_file *chd, toc &toc);
	static std::error_condition write_metadata(chd_file *chd, const toc &toc);
	bool is_gdrom() const { return cdtoc.flags & (CD_FLAG_GDROM|CD_FLAG_GDROMLE); }

	// ECC utilities
	static bool ecc_verify(const uint8_t *sector);
	static void ecc_generate(uint8_t *sector);
	static void ecc_clear(uint8_t *sector);



	static inline uint32_t msf_to_lba(uint32_t msf)
	{
		return ( ((msf&0x00ff0000)>>16) * 60 * 75) + (((msf&0x0000ff00)>>8) * 75) + ((msf&0x000000ff)>>0);
	}

	static inline uint32_t lba_to_msf(uint32_t lba)
	{
		uint8_t m, s, f;

		m = lba / (60 * 75);
		lba -= m * (60 * 75);
		s = lba / 75;
		f = lba % 75;

		return ((m / 10) << 20) | ((m % 10) << 16) |
			((s / 10) << 12) | ((s % 10) <<  8) |
			((f / 10) <<  4) | ((f % 10) <<  0);
	}

	// segacd needs it like this.. investigate
	// Angelo also says PCE tracks often start playing at the
	// wrong address.. related?
	static inline uint32_t lba_to_msf_alt(int lba)
	{
		uint32_t ret = 0;

		ret |= ((lba / (60 * 75))&0xff)<<16;
		ret |= (((lba / 75) % 60)&0xff)<<8;
		ret |= ((lba % 75)&0xff)<<0;

		return ret;
	}

private:
	enum gdi_area {
		SINGLE_DENSITY,
		HIGH_DENSITY
	};

	enum gdi_pattern {
		TYPE_UNKNOWN = 0,
		TYPE_I,
		TYPE_II,
		TYPE_III,
		TYPE_III_SPLIT
	};

	/** @brief  offset within sector. */
	static constexpr int SYNC_OFFSET = 0x000;
	/** @brief  12 bytes. */
	static constexpr int SYNC_NUM_BYTES = 12;

	/** @brief  offset within sector. */
	static constexpr int MODE_OFFSET = 0x00f;

	/** @brief  offset within sector. */
	static constexpr int ECC_P_OFFSET = 0x81c;
	/** @brief  2 lots of 86. */
	static constexpr int ECC_P_NUM_BYTES = 86;
	/** @brief  24 bytes each. */
	static constexpr int ECC_P_COMP = 24;

	/** @brief  The ECC q offset. */
	static constexpr int ECC_Q_OFFSET = ECC_P_OFFSET + 2 * ECC_P_NUM_BYTES;
	/** @brief  2 lots of 52. */
	static constexpr int ECC_Q_NUM_BYTES = 52;
	/** @brief  43 bytes each. */
	static constexpr int ECC_Q_COMP = 43;

	// ECC tables
	static const uint8_t ecclow[256];
	static const uint8_t ecchigh[256];
	static const uint16_t poffsets[ECC_P_NUM_BYTES][ECC_P_COMP];
	static const uint16_t qoffsets[ECC_Q_NUM_BYTES][ECC_Q_COMP];

	/** @brief  The chd. */
	chd_file *           chd;                /* CHD file */
	/** @brief  The cdtoc. */
	toc                  cdtoc;              /* TOC for the CD */
	/** @brief  Information describing the track. */
	track_input_info     cdtrack_info;       /* track info */
	/** @brief  The fhandle[ CD maximum tracks]. */
	util::random_read::ptr fhandle[MAX_TRACKS];/* file handle */

	inline uint32_t physical_to_chd_lba(uint32_t physlba, uint32_t &tracknum) const;
	inline uint32_t logical_to_chd_lba(uint32_t physlba, uint32_t &tracknum) const;

	static void get_info_from_type_string(const char *typestring, uint32_t *trktype, uint32_t *datasize);
	static uint8_t ecc_source_byte(const uint8_t *sector, uint32_t offset);
	static void ecc_compute_bytes(const uint8_t *sector, const uint16_t *row, int rowlen, uint8_t &val1, uint8_t &val2);
	std::error_condition read_partial_sector(void *dest, uint32_t lbasector, uint32_t chdsector, uint32_t tracknum, uint32_t startoffs, uint32_t length, bool phys);

	static std::string get_file_path(std::string &path);
	static uint64_t get_file_size(std::string_view filename);
	static int tokenize(const char *linebuffer, int i, int linebuffersize, char *token, int tokensize);
	static int msf_to_frames(const char *token);
	static uint32_t parse_wav_sample(std::string_view filename, uint32_t *dataoffs);
	static uint16_t read_uint16(util::read_stream &infile);
	static uint32_t read_uint32(util::read_stream &infile);
	static uint64_t read_uint64(util::read_stream &infile);
};

#endif // MAME_LIB_UTIL_CDROM_H
