// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    CHD compression frontend

****************************************************************************/
#include <stdio.h> // must be stdio.h and here otherwise issues with I64FMT in MINGW

// osd
#include "osdcore.h"

// lib/util
#include "avhuff.h"
#include "aviio.h"
#include "bitmap.h"
#include "chdcd.h"
#include "corefile.h"
#include "hashing.h"
#include "md5.h"
#include "vbiparse.h"

#include <cassert>
#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <limits>
#include <memory>
#include <new>
#include <unordered_map>



//**************************************************************************
//  CONSTANTS & DEFINES
//**************************************************************************
// MINGW has adopted the MSVC formatting for 64-bit ints as of GCC 4.4 and deprecated it as of GCC 9.3
#if defined(WIN32) && defined(__GNUC__) && ((__GNUC__ < 9) || ((__GNUC__ == 9) && (__GNUC_MINOR__ < 3)))
#define I64FMT   "I64"
#elif !defined(__APPLE__) && defined(__LP64__)
#define I64FMT   "l"
#else
#define I64FMT   "ll"
#endif

// default hard disk sector size
const uint32_t IDE_SECTOR_SIZE = 512;

// temporary input buffer size
const uint32_t TEMP_BUFFER_SIZE = 32 * 1024 * 1024;

// modes
const int MODE_NORMAL = 0;
const int MODE_CUEBIN = 1;
const int MODE_GDI = 2;

// command modifier
#define REQUIRED "~"

// command strings
#define COMMAND_HELP "help"
#define COMMAND_INFO "info"
#define COMMAND_VERIFY "verify"
#define COMMAND_CREATE_RAW "createraw"
#define COMMAND_CREATE_HD "createhd"
#define COMMAND_CREATE_CD "createcd"
#define COMMAND_CREATE_LD "createld"
#define COMMAND_EXTRACT_RAW "extractraw"
#define COMMAND_EXTRACT_HD "extracthd"
#define COMMAND_EXTRACT_CD "extractcd"
#define COMMAND_EXTRACT_LD "extractld"
#define COMMAND_COPY "copy"
#define COMMAND_ADD_METADATA "addmeta"
#define COMMAND_DEL_METADATA "delmeta"
#define COMMAND_DUMP_METADATA "dumpmeta"
#define COMMAND_LIST_TEMPLATES "listtemplates"

// option strings
#define OPTION_INPUT "input"
#define OPTION_OUTPUT "output"
#define OPTION_OUTPUT_BIN "outputbin"
#define OPTION_OUTPUT_FORCE "force"
#define OPTION_INPUT_START_BYTE "inputstartbyte"
#define OPTION_INPUT_START_HUNK "inputstarthunk"
#define OPTION_INPUT_START_FRAME "inputstartframe"
#define OPTION_INPUT_LENGTH_BYTES "inputbytes"
#define OPTION_INPUT_LENGTH_HUNKS "inputhunks"
#define OPTION_INPUT_LENGTH_FRAMES "inputframes"
#define OPTION_HUNK_SIZE "hunksize"
#define OPTION_UNIT_SIZE "unitsize"
#define OPTION_COMPRESSION "compression"
#define OPTION_INPUT_PARENT "inputparent"
#define OPTION_OUTPUT_PARENT "outputparent"
#define OPTION_IDENT "ident"
#define OPTION_CHS "chs"
#define OPTION_SECTOR_SIZE "sectorsize"
#define OPTION_TAG "tag"
#define OPTION_INDEX "index"
#define OPTION_VALUE_TEXT "valuetext"
#define OPTION_VALUE_FILE "valuefile"
#define OPTION_NO_CHECKSUM "nochecksum"
#define OPTION_VERBOSE "verbose"
#define OPTION_FIX "fix"
#define OPTION_NUMPROCESSORS "numprocessors"
#define OPTION_SIZE "size"
#define OPTION_TEMPLATE "template"


//**************************************************************************
//  FUNCTION PROTOTYPES
//**************************************************************************

typedef std::unordered_map<std::string, std::string *> parameters_map;

template <typename Format, typename... Params> static void report_error(int error, Format &&fmt, Params &&...args);
static void do_info(parameters_map &params);
static void do_verify(parameters_map &params);
static void do_create_raw(parameters_map &params);
static void do_create_hd(parameters_map &params);
static void do_create_cd(parameters_map &params);
static void do_create_ld(parameters_map &params);
static void do_copy(parameters_map &params);
static void do_extract_raw(parameters_map &params);
static void do_extract_cd(parameters_map &params);
static void do_extract_ld(parameters_map &params);
static void do_add_metadata(parameters_map &params);
static void do_del_metadata(parameters_map &params);
static void do_dump_metadata(parameters_map &params);
static void do_list_templates(parameters_map &params);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> option_description

struct option_description
{
	const char *name;
	const char *shortname;
	bool parameter;
	const char *description;
};


// ======================> command_description

struct command_description
{
	const char *name;
	void (*handler)(parameters_map &);
	const char *description;
	const char *valid_options[16];
};


// ======================> avi_info

struct avi_info
{
	uint32_t fps_times_1million;
	uint32_t width;
	uint32_t height;
	bool interlaced;
	uint32_t channels;
	uint32_t rate;
	uint32_t max_samples_per_frame;
	uint32_t bytes_per_frame;
};

// ======================> hd_template

struct hd_template
{
	const char *manufacturer;
	const char *model;
	uint32_t cylinders;
	uint32_t heads;
	uint32_t sectors;
	uint32_t sector_size;
};

// ======================> metadata_index_info

struct metadata_index_info
{
	chd_metadata_tag    tag;
	uint32_t              index;
};


// ======================> fatal_error

class fatal_error : public std::exception
{
public:
	fatal_error(int error)
		: m_error(error) { }

	int error() const { return m_error; }

private:
	int m_error;
};


// ======================> chd_zero_compressor

class chd_zero_compressor : public chd_file_compressor
{
public:
	// construction/destruction
	chd_zero_compressor(std::uint64_t offset = 0, std::uint64_t maxoffset = 0)
		: m_offset(offset)
		, m_maxoffset(maxoffset)
	{
	}

	// read interface
	virtual std::uint32_t read_data(void *dest, std::uint64_t offset, std::uint32_t length) override
	{
		offset += m_offset;
		if (offset >= m_maxoffset)
			return 0;
		if (offset + length > m_maxoffset)
			length = m_maxoffset - offset;
		std::memset(dest, 0, length);
		return length;
	}

private:
	// internal state
	std::uint64_t   m_offset;
	std::uint64_t   m_maxoffset;
};


// ======================> chd_rawfile_compressor

class chd_rawfile_compressor : public chd_file_compressor
{
public:
	// construction/destruction
	chd_rawfile_compressor(util::core_file &file, std::uint64_t offset = 0, std::uint64_t maxoffset = std::numeric_limits<std::uint64_t>::max())
		: m_file(file)
		, m_offset(offset)
		, m_maxoffset((std::min)(maxoffset, file.size()))
	{
	}

	// read interface
	virtual std::uint32_t read_data(void *dest, std::uint64_t offset, std::uint32_t length) override
	{
		offset += m_offset;
		if (offset >= m_maxoffset)
			return 0;
		if (offset + length > m_maxoffset)
			length = m_maxoffset - offset;
		m_file.seek(offset, SEEK_SET);
		return m_file.read(dest, length);
	}

private:
	// internal state
	util::core_file &   m_file;
	std::uint64_t       m_offset;
	std::uint64_t       m_maxoffset;
};


// ======================> chd_chdfile_compressor

class chd_chdfile_compressor : public chd_file_compressor
{
public:
	// construction/destruction
	chd_chdfile_compressor(chd_file &file, uint64_t offset = 0, uint64_t maxoffset = ~0)
		: m_toc(nullptr),
			m_file(file),
			m_offset(offset),
			m_maxoffset(std::min(maxoffset, file.logical_bytes())) { }

	// read interface
	virtual uint32_t read_data(void *dest, uint64_t offset, uint32_t length)
	{
		offset += m_offset;
		if (offset >= m_maxoffset)
			return 0;
		if (offset + length > m_maxoffset)
			length = m_maxoffset - offset;
		chd_error err = m_file.read_bytes(offset, dest, length);
		if (err != CHDERR_NONE)
			throw err;

		// if we have TOC - detect audio sectors and swap data
		if (m_toc)
		{
			assert(offset % CD_FRAME_SIZE == 0);
			assert(length % CD_FRAME_SIZE == 0);

			int startlba = offset / CD_FRAME_SIZE;
			int lenlba = length / CD_FRAME_SIZE;
			uint8_t *_dest = reinterpret_cast<uint8_t *>(dest);

			for (int chdlba = 0; chdlba < lenlba; chdlba++)
			{
				// find current frame's track number
				int tracknum = m_toc->numtrks;
				for (int track = 0; track < m_toc->numtrks; track++)
					if ((chdlba + startlba) < m_toc->tracks[track + 1].chdframeofs)
					{
						tracknum = track;
						break;
					}
				// is it audio ?
				if (m_toc->tracks[tracknum].trktype != CD_TRACK_AUDIO)
					continue;
				// byteswap if yes
				int dataoffset = chdlba * CD_FRAME_SIZE;
				for (uint32_t swapindex = dataoffset; swapindex < (dataoffset + CD_MAX_SECTOR_DATA); swapindex += 2)
				{
					uint8_t temp = _dest[swapindex];
					_dest[swapindex] = _dest[swapindex + 1];
					_dest[swapindex + 1] = temp;
				}
			}
		}

		return length;
	}

const cdrom_toc *   m_toc;

private:
	// internal state
	chd_file &      m_file;
	uint64_t          m_offset;
	uint64_t          m_maxoffset;
};


// ======================> chd_cd_compressor

class chd_cd_compressor : public chd_file_compressor
{
public:
	// construction/destruction
	chd_cd_compressor(cdrom_toc &toc, chdcd_track_input_info &info)
		: m_file(),
			m_toc(toc),
			m_info(info) { }

	~chd_cd_compressor()
	{
	}

	// read interface
	virtual uint32_t read_data(void *_dest, uint64_t offset, uint32_t length)
	{
		// verify assumptions made below
		assert(offset % CD_FRAME_SIZE == 0);
		assert(length % CD_FRAME_SIZE == 0);

		// initialize destination to 0 so that unused areas are filled
		uint8_t *dest = reinterpret_cast<uint8_t *>(_dest);
		memset(dest, 0, length);

		// find out which track we're starting in
		uint64_t startoffs = 0;
		uint32_t length_remaining = length;
		for (int tracknum = 0; tracknum < m_toc.numtrks; tracknum++)
		{
			const cdrom_track_info &trackinfo = m_toc.tracks[tracknum];
			uint64_t endoffs = startoffs + (uint64_t)(trackinfo.frames + trackinfo.extraframes) * CD_FRAME_SIZE;
			if (offset >= startoffs && offset < endoffs)
			{
				// if we don't already have this file open, open it now
				if (!m_file || m_lastfile.compare(m_info.track[tracknum].fname)!=0)
				{
					m_file.reset();
					m_lastfile = m_info.track[tracknum].fname;
					osd_file::error filerr = util::core_file::open(m_lastfile, OPEN_FLAG_READ, m_file);
					if (filerr != osd_file::error::NONE)
						report_error(1, "Error opening input file (%s)'", m_lastfile.c_str());
				}

				// iterate over frames
				uint64_t bytesperframe = trackinfo.datasize + trackinfo.subsize;
				uint64_t src_track_start = m_info.track[tracknum].offset;
				uint64_t src_track_end = src_track_start + bytesperframe * (uint64_t)trackinfo.frames;
				uint64_t pad_track_start = src_track_end - ((uint64_t)m_toc.tracks[tracknum].padframes * bytesperframe);
				uint64_t split_track_start = pad_track_start - ((uint64_t)m_toc.tracks[tracknum].splitframes * bytesperframe);

				// dont split when split-bin read not required
				if ((uint64_t)m_toc.tracks[tracknum].splitframes == 0L)
					split_track_start = UINT64_MAX;

				while (length_remaining != 0 && offset < endoffs)
				{
					// determine start of current frame
					uint64_t src_frame_start = src_track_start + ((offset - startoffs) / CD_FRAME_SIZE) * bytesperframe;

					// auto-advance next track for split-bin read
					if (src_frame_start == split_track_start && m_lastfile.compare(m_info.track[tracknum+1].fname)!=0)
					{
						m_file.reset();
						m_lastfile = m_info.track[tracknum+1].fname;
						osd_file::error filerr = util::core_file::open(m_lastfile, OPEN_FLAG_READ, m_file);
						if (filerr != osd_file::error::NONE)
							report_error(1, "Error opening input file (%s)'", m_lastfile.c_str());
					}

					if (src_frame_start < src_track_end)
					{
						// read it in, or pad if we're into the padframes
						if (src_frame_start >= pad_track_start)
						{
							memset(dest, 0, bytesperframe);
						}
						else
						{
							m_file->seek((src_frame_start >= split_track_start)
								? src_frame_start - split_track_start
								: src_frame_start, SEEK_SET);
							uint32_t count = m_file->read(dest, bytesperframe);
							if (count != bytesperframe)
								report_error(1, "Error reading input file (%s)'", m_lastfile.c_str());
						}

						// swap if appropriate
						if (m_info.track[tracknum].swap)
							for (uint32_t swapindex = 0; swapindex < 2352; swapindex += 2)
							{
								uint8_t temp = dest[swapindex];
								dest[swapindex] = dest[swapindex + 1];
								dest[swapindex + 1] = temp;
							}
					}

					// advance
					offset += CD_FRAME_SIZE;
					dest += CD_FRAME_SIZE;
					length_remaining -= CD_FRAME_SIZE;
					if (length_remaining == 0)
						break;
				}
			}

			// next track starts after the previous one
			startoffs = endoffs;
		}
		return length - length_remaining;
	}

private:
	// internal state
	std::string                 m_lastfile;
	util::core_file::ptr        m_file;
	cdrom_toc &                 m_toc;
	chdcd_track_input_info &    m_info;
};


// ======================> chd_avi_compressor

class chd_avi_compressor : public chd_file_compressor
{
public:
	// construction/destruction
	chd_avi_compressor(avi_file &file, avi_info &info, uint32_t first_frame, uint32_t num_frames)
		: m_file(file),
			m_info(info),
			m_bitmap(info.width, info.height * (info.interlaced ? 2 : 1)),
			m_start_frame(first_frame),
			m_frame_count(num_frames),
			m_ldframedata(num_frames * VBI_PACKED_BYTES),
			m_rawdata(info.bytes_per_frame) { }

	// getters
	const std::vector<uint8_t> &ldframedata() const { return m_ldframedata; }

	// read interface
	virtual uint32_t read_data(void *_dest, uint64_t offset, uint32_t length)
	{
		uint8_t *dest = reinterpret_cast<uint8_t *>(_dest);
		uint8_t interlace_factor = m_info.interlaced ? 2 : 1;
		uint32_t length_remaining = length;

		// iterate over frames
		int32_t start_frame = offset / m_info.bytes_per_frame;
		int32_t end_frame = (offset + length - 1) / m_info.bytes_per_frame;
		for (int32_t framenum = start_frame; framenum <= end_frame; framenum++)
			if (framenum < m_frame_count)
			{
				// determine effective frame number and first/last samples
				int32_t effframe = m_start_frame + framenum;
				uint32_t first_sample = (uint64_t(m_info.rate) * uint64_t(effframe) * uint64_t(1000000) + m_info.fps_times_1million - 1) / uint64_t(m_info.fps_times_1million);
				uint32_t samples = (uint64_t(m_info.rate) * uint64_t(effframe + 1) * uint64_t(1000000) + m_info.fps_times_1million - 1) / uint64_t(m_info.fps_times_1million) - first_sample;

				// loop over channels and read the samples
				int channels = unsigned((std::min<std::size_t>)(m_info.channels, ARRAY_LENGTH(m_audio)));
				EQUIVALENT_ARRAY(m_audio, int16_t *) samplesptr;
				for (int chnum = 0; chnum < channels; chnum++)
				{
					// read the sound samples
					m_audio[chnum].resize(samples);
					samplesptr[chnum] = &m_audio[chnum][0];
					avi_file::error avierr = m_file.read_sound_samples(chnum, first_sample, samples, &m_audio[chnum][0]);
					if (avierr != avi_file::error::NONE)
						report_error(1, "Error reading audio samples %d-%d from channel %d: %s", first_sample, samples, chnum, avi_file::error_string(avierr));
				}

				// read the video data
				avi_file::error avierr = m_file.read_video_frame(effframe / interlace_factor, m_bitmap);
				if (avierr != avi_file::error::NONE)
					report_error(1, "Error reading AVI frame %d: %s", effframe / interlace_factor, avi_file::error_string(avierr));
				bitmap_yuy16 subbitmap(&m_bitmap.pix(effframe % interlace_factor), m_bitmap.width(), m_bitmap.height() / interlace_factor, m_bitmap.rowpixels() * interlace_factor);

				// update metadata for this frame
				if (m_info.height == 524/2 || m_info.height == 624/2)
				{
					vbi_metadata vbi;
					vbi_parse_all(&subbitmap.pix(0), subbitmap.rowpixels(), subbitmap.width(), 8, &vbi);
					vbi_metadata_pack(&m_ldframedata[framenum * VBI_PACKED_BYTES], framenum, &vbi);
				}

				// assemble the data into final form
				avhuff_error averr = avhuff_encoder::assemble_data(m_rawdata, subbitmap, channels, samples, samplesptr);
				if (averr != AVHERR_NONE)
					report_error(1, "Error assembling data for frame %d", framenum);
				if (m_rawdata.size() < m_info.bytes_per_frame)
				{
					int old_size = m_rawdata.size();
					m_rawdata.resize(m_info.bytes_per_frame);
					memset(&m_rawdata[old_size], 0, m_info.bytes_per_frame - old_size);
				}

				// copy to the destination
				uint64_t start_offset = uint64_t(framenum) * uint64_t(m_info.bytes_per_frame);
				uint64_t end_offset = start_offset + m_info.bytes_per_frame;
				uint32_t bytes_to_copy = (std::min<uint64_t>)(length_remaining, end_offset - offset);
				memcpy(dest, &m_rawdata[offset - start_offset], bytes_to_copy);

				// advance
				offset += bytes_to_copy;
				dest += bytes_to_copy;
				length_remaining -= bytes_to_copy;
			}

		return length;
	}

private:
	// internal state
	avi_file &                  m_file;
	avi_info &                  m_info;
	bitmap_yuy16                m_bitmap;
	uint32_t                      m_start_frame;
	uint32_t                      m_frame_count;
	std::vector<int16_t>        m_audio[8];
	std::vector<uint8_t>              m_ldframedata;
	std::vector<uint8_t>              m_rawdata;
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// timing
static clock_t lastprogress = 0;


// default compressors
static const chd_codec_type s_default_raw_compression[4] = { CHD_CODEC_LZMA, CHD_CODEC_ZLIB, CHD_CODEC_HUFFMAN, CHD_CODEC_FLAC };
static const chd_codec_type s_default_hd_compression[4] = { CHD_CODEC_LZMA, CHD_CODEC_ZLIB, CHD_CODEC_HUFFMAN, CHD_CODEC_FLAC };
static const chd_codec_type s_default_cd_compression[4] = { CHD_CODEC_CD_LZMA, CHD_CODEC_CD_ZLIB, CHD_CODEC_CD_FLAC };
static const chd_codec_type s_default_ld_compression[4] = { CHD_CODEC_AVHUFF };


// descriptions for each option
static const option_description s_options[] =
{
	{ OPTION_INPUT,                 "i",    true, " <filename>: input file name" },
	{ OPTION_INPUT_PARENT,          "ip",   true, " <filename>: parent file name for input CHD" },
	{ OPTION_OUTPUT,                "o",    true, " <filename>: output file name" },
	{ OPTION_OUTPUT_BIN,            "ob",   true, " <filename>: output file name for binary data" },
	{ OPTION_OUTPUT_FORCE,          "f",    false, ": force overwriting an existing file" },
	{ OPTION_OUTPUT_PARENT,         "op",   true, " <filename>: parent file name for output CHD" },
	{ OPTION_INPUT_START_BYTE,      "isb",  true, " <offset>: starting byte offset within the input" },
	{ OPTION_INPUT_START_HUNK,      "ish",  true, " <offset>: starting hunk offset within the input" },
	{ OPTION_INPUT_START_FRAME,     "isf",  true, " <offset>: starting frame within the input" },
	{ OPTION_INPUT_LENGTH_BYTES,    "ib",   true, " <length>: effective length of input in bytes" },
	{ OPTION_INPUT_LENGTH_HUNKS,    "ih",   true, " <length>: effective length of input in hunks" },
	{ OPTION_INPUT_LENGTH_FRAMES,   "if",   true, " <length>: effective length of input in frames" },
	{ OPTION_HUNK_SIZE,             "hs",   true, " <bytes>: size of each hunk, in bytes" },
	{ OPTION_UNIT_SIZE,             "us",   true, " <bytes>: size of each unit, in bytes" },
	{ OPTION_COMPRESSION,           "c",    true, " <none|type1[,type2[,...]]>: which compression codecs to use (up to 4)" },
	{ OPTION_IDENT,                 "id",   true, " <filename>: name of ident file to provide CHS information" },
	{ OPTION_CHS,                   "chs",  true, " <cylinders,heads,sectors>: specifies CHS values directly" },
	{ OPTION_SECTOR_SIZE,           "ss",   true, " <bytes>: size of each hard disk sector" },
	{ OPTION_TAG,                   "t",    true, " <tag>: 4-character tag for metadata" },
	{ OPTION_INDEX,                 "ix",   true, " <index>: indexed instance of this metadata tag" },
	{ OPTION_VALUE_TEXT,            "vt",   true, " <text>: text for the metadata" },
	{ OPTION_VALUE_FILE,            "vf",   true, " <file>: file containing data to add" },
	{ OPTION_NUMPROCESSORS,         "np",   true, " <processors>: limit the number of processors to use during compression" },
	{ OPTION_NO_CHECKSUM,           "nocs", false, ": do not include this metadata information in the overall SHA-1" },
	{ OPTION_FIX,                   "f",    false, ": fix the SHA-1 if it is incorrect" },
	{ OPTION_VERBOSE,               "v",    false, ": output additional information" },
	{ OPTION_SIZE,                  "s",    true, ": <bytes>: size of the output file" },
	{ OPTION_TEMPLATE,              "tp",   true, ": <id>: use hard disk template (see listtemplates)" },
};


// descriptions for each command
static const command_description s_commands[] =
{
	{ COMMAND_INFO, do_info, ": displays information about a CHD",
		{
			REQUIRED OPTION_INPUT,
			OPTION_VERBOSE
		}
	},

	{ COMMAND_VERIFY, do_verify, ": verifies a CHD's integrity",
		{
			REQUIRED OPTION_INPUT,
			OPTION_INPUT_PARENT
		}
	},

	{ COMMAND_CREATE_RAW, do_create_raw, ": create a raw CHD from the input file",
		{
			REQUIRED OPTION_OUTPUT,
			OPTION_OUTPUT_PARENT,
			OPTION_OUTPUT_FORCE,
			REQUIRED OPTION_INPUT,
			OPTION_INPUT_START_BYTE,
			OPTION_INPUT_START_HUNK,
			OPTION_INPUT_LENGTH_BYTES,
			OPTION_INPUT_LENGTH_HUNKS,
			REQUIRED OPTION_HUNK_SIZE,
			REQUIRED OPTION_UNIT_SIZE,
			OPTION_COMPRESSION,
			OPTION_NUMPROCESSORS
		}
	},

	{ COMMAND_CREATE_HD, do_create_hd, ": create a hard disk CHD from the input file",
		{
			REQUIRED OPTION_OUTPUT,
			OPTION_OUTPUT_PARENT,
			OPTION_OUTPUT_FORCE,
			OPTION_INPUT,
			OPTION_INPUT_START_BYTE,
			OPTION_INPUT_START_HUNK,
			OPTION_INPUT_LENGTH_BYTES,
			OPTION_INPUT_LENGTH_HUNKS,
			OPTION_HUNK_SIZE,
			OPTION_COMPRESSION,
			OPTION_TEMPLATE,
			OPTION_IDENT,
			OPTION_CHS,
			OPTION_SIZE,
			OPTION_SECTOR_SIZE,
			OPTION_NUMPROCESSORS
		}
	},

	{ COMMAND_CREATE_CD, do_create_cd, ": create a CD CHD from the input file",
		{
			REQUIRED OPTION_OUTPUT,
			OPTION_OUTPUT_PARENT,
			OPTION_OUTPUT_FORCE,
			REQUIRED OPTION_INPUT,
			OPTION_HUNK_SIZE,
			OPTION_COMPRESSION,
			OPTION_NUMPROCESSORS
		}
	},

	{ COMMAND_CREATE_LD, do_create_ld, ": create a laserdisc CHD from the input file",
		{
			REQUIRED OPTION_OUTPUT,
			OPTION_OUTPUT_PARENT,
			OPTION_OUTPUT_FORCE,
			REQUIRED OPTION_INPUT,
			OPTION_INPUT_START_FRAME,
			OPTION_INPUT_LENGTH_FRAMES,
			OPTION_HUNK_SIZE,
			OPTION_COMPRESSION,
			OPTION_NUMPROCESSORS
		}
	},

	{ COMMAND_EXTRACT_RAW, do_extract_raw, ": extract raw file from a CHD input file",
		{
			REQUIRED OPTION_OUTPUT,
			OPTION_OUTPUT_FORCE,
			REQUIRED OPTION_INPUT,
			OPTION_INPUT_PARENT,
			OPTION_INPUT_START_BYTE,
			OPTION_INPUT_START_HUNK,
			OPTION_INPUT_LENGTH_BYTES,
			OPTION_INPUT_LENGTH_HUNKS
		}
	},

	{ COMMAND_EXTRACT_HD, do_extract_raw, ": extract raw hard disk file from a CHD input file",
		{
			REQUIRED OPTION_OUTPUT,
			OPTION_OUTPUT_FORCE,
			REQUIRED OPTION_INPUT,
			OPTION_INPUT_PARENT,
			OPTION_INPUT_START_BYTE,
			OPTION_INPUT_START_HUNK,
			OPTION_INPUT_LENGTH_BYTES,
			OPTION_INPUT_LENGTH_HUNKS
		}
	},

	{ COMMAND_EXTRACT_CD, do_extract_cd, ": extract CD file from a CHD input file",
		{
			REQUIRED OPTION_OUTPUT,
			OPTION_OUTPUT_BIN,
			OPTION_OUTPUT_FORCE,
			REQUIRED OPTION_INPUT,
			OPTION_INPUT_PARENT,
		}
	},

	{ COMMAND_EXTRACT_LD, do_extract_ld, ": extract laserdisc AVI from a CHD input file",
		{
			REQUIRED OPTION_OUTPUT,
			OPTION_OUTPUT_FORCE,
			REQUIRED OPTION_INPUT,
			OPTION_INPUT_PARENT,
			OPTION_INPUT_START_FRAME,
			OPTION_INPUT_LENGTH_FRAMES
		}
	},

	{ COMMAND_COPY, do_copy, ": copy data from one CHD to another of the same type",
		{
			REQUIRED OPTION_OUTPUT,
			OPTION_OUTPUT_PARENT,
			OPTION_OUTPUT_FORCE,
			REQUIRED OPTION_INPUT,
			OPTION_INPUT_PARENT,
			OPTION_INPUT_START_BYTE,
			OPTION_INPUT_START_HUNK,
			OPTION_INPUT_LENGTH_BYTES,
			OPTION_INPUT_LENGTH_HUNKS,
			OPTION_HUNK_SIZE,
			OPTION_COMPRESSION,
			OPTION_NUMPROCESSORS
		}
	},

	{ COMMAND_ADD_METADATA, do_add_metadata, ": add metadata to the CHD",
		{
			REQUIRED OPTION_INPUT,
			REQUIRED OPTION_TAG,
			OPTION_INDEX,
			OPTION_VALUE_TEXT,
			OPTION_VALUE_FILE,
			OPTION_NO_CHECKSUM
		}
	},

	{ COMMAND_DEL_METADATA, do_del_metadata, ": remove metadata from the CHD",
		{
			REQUIRED OPTION_INPUT,
			REQUIRED OPTION_TAG,
			OPTION_INDEX
		}
	},

	{ COMMAND_DUMP_METADATA, do_dump_metadata, ": dump metadata from the CHD to stdout or to a file",
		{
			REQUIRED OPTION_INPUT,
			OPTION_OUTPUT,
			OPTION_OUTPUT_FORCE,
			REQUIRED OPTION_TAG,
			OPTION_INDEX
		}
	},

	{ COMMAND_LIST_TEMPLATES, do_list_templates, ": list hard disk templates",
		{
		}
	},
};


// hard disk templates
static const hd_template s_hd_templates[] =
{
	{ "Conner", "CFA170A", 332, 16, 63, 512 }, // 163 MB
	{ "Rodime", "R0201",   321,  2, 16, 512 }, //   5 MB
	{ "Rodime", "R0202",   321,  4, 16, 512 }, //  10 MB
	{ "Rodime", "R0203",   321,  6, 16, 512 }, //  15 MB
	{ "Rodime", "R0204",   321,  8, 16, 512 }, //  20 MB
};



//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  report_error - report an error
//-------------------------------------------------

template <typename Format, typename... Params> static void report_error(int error, Format &&fmt, Params &&...args)
{
	// output to stderr
	util::stream_format(std::cerr, std::forward<Format>(fmt), std::forward<Params>(args)...);
	std::cerr << std::endl;

	// reset time for progress and return the error
	lastprogress = 0;
	throw fatal_error(error);
}


//-------------------------------------------------
//  progress - generic progress callback
//-------------------------------------------------

template <typename Format, typename... Params> static void progress(bool forceit, Format &&fmt, Params &&...args)
{
	// skip if it hasn't been long enough
	clock_t curtime = clock();
	if (!forceit && lastprogress != 0 && curtime - lastprogress < CLOCKS_PER_SEC / 2)
		return;
	lastprogress = curtime;

	// standard vfprintf stuff here
	util::stream_format(std::cerr, std::forward<Format>(fmt), std::forward<Params>(args)...);
	std::cerr << std::flush;
}


//-------------------------------------------------
//  print_help - print help for all the commands
//-------------------------------------------------

static int print_help(const std::string &argv0, const char *error = nullptr)
{
	// print the error message first
	if (error != nullptr)
		fprintf(stderr, "Error: %s\n\n", error);

	// print a summary of each command
	printf("Usage:\n");
	for (auto & desc : s_commands)
	{
		printf("   %s %s%s\n", argv0.c_str(), desc.name, desc.description);
	}
	printf("\nFor help with any command, run:\n");
	printf("   %s %s <command>\n", argv0.c_str(), COMMAND_HELP);
	return 1;
}


//-------------------------------------------------
//  print_help - print help for all a specific
//  command
//-------------------------------------------------

static int print_help(const std::string &argv0, const command_description &desc, const char *error = nullptr)
{
	// print the error message first
	if (error != nullptr)
		fprintf(stderr, "Error: %s\n\n", error);

	// print usage for this command
	printf("Usage:\n");
	printf("   %s %s [options], where valid options are:\n", argv0.c_str(), desc.name);
	for (int valid = 0; valid < ARRAY_LENGTH(desc.valid_options); valid++)
	{
		// determine whether we are required
		const char *option = desc.valid_options[valid];
		if (option == nullptr)
			break;
		bool required = (option[0] == REQUIRED[0]);
		if (required)
			option++;

		// find the option
		for (auto & s_option : s_options)
			if (strcmp(option, s_option.name) == 0)
			{
				const option_description &odesc = s_option;
				printf("      --%s", odesc.name);
				if (odesc.shortname != nullptr)
					printf(", -%s", odesc.shortname);
				printf("%s%s\n", odesc.description, required ? " (required)" : "");
			}
	}
	return 1;
}


//-------------------------------------------------
//  big_int_string - create a 64-bit string
//-------------------------------------------------

std::string big_int_string(uint64_t intvalue)
{
	// 0 is a special case
	if (intvalue == 0)
		return "0";

	// loop until all chunks are done
	std::string str;
	bool first = true;
	while (intvalue != 0)
	{
		int chunk = intvalue % 1000;
		intvalue /= 1000;

		std::string insert = string_format((intvalue != 0) ? "%03d" : "%d", chunk);

		if (!first)
			str.insert(0, ",");
		first = false;
		str.insert(0, insert);
	}
	return str;
}


//-------------------------------------------------
//  msf_string_from_frames - output the given
//  number of frames in M:S:F format
//-------------------------------------------------

std::string msf_string_from_frames(uint32_t frames)
{
	return string_format("%02d:%02d:%02d", frames / (75 * 60), (frames / 75) % 60, frames % 75);
}


//-------------------------------------------------
//  parse_number - parse a number string with an
//  optional k/m/g suffix
//-------------------------------------------------

uint64_t parse_number(const char *string)
{
	// 0-length string is 0
	int length = strlen(string);
	if (length == 0)
		return 0;

	// scan forward over digits
	uint64_t result = 0;
	while (isdigit(*string))
	{
		result = (result * 10) + (*string - '0');
		string++;
	}

	// handle multipliers
	if (*string == 'k' || *string == 'K')
		result *= 1024;
	if (*string == 'm' || *string == 'M')
		result *= 1024 * 1024;
	if (*string == 'g' || *string == 'G')
		result *= 1024 * 1024 * 1024;

	return result;
}


//-------------------------------------------------
//  guess_chs - given a file and an offset,
//  compute a best guess CHS value set
//-------------------------------------------------

static void guess_chs(std::string *filename, uint64_t filesize, int sectorsize, uint32_t &cylinders, uint32_t &heads, uint32_t &sectors, uint32_t &bps)
{
	// if this is a direct physical drive read, handle it specially
	if (filename != nullptr && osd_get_physical_drive_geometry(filename->c_str(), &cylinders, &heads, &sectors, &bps))
		return;

	// if we have no length to work with, we can't guess
	if (filesize == 0)
		report_error(1, "Can't guess CHS values because there is no input file");

	// now find a valid value
	for (uint32_t totalsectors = filesize / sectorsize; ; totalsectors++)
		for (uint32_t cursectors = 63; cursectors > 1; cursectors--)
			if (totalsectors % cursectors == 0)
			{
				uint32_t totalheads = totalsectors / cursectors;
				for (uint32_t curheads = 16; curheads > 1; curheads--)
					if (totalheads % curheads == 0)
					{
						cylinders = totalheads / curheads;
						heads = curheads;
						sectors = cursectors;
						return;
					}
			}
}


//-------------------------------------------------
//  parse_input_chd_parameters - parse the
//  standard set of input CHD parameters
//-------------------------------------------------

static void parse_input_chd_parameters(const parameters_map &params, chd_file &input_chd, chd_file &input_parent_chd, bool writeable = false)
{
	// process input parent file
	auto input_chd_parent_str = params.find(OPTION_INPUT_PARENT);
	if (input_chd_parent_str != params.end())
	{
		chd_error err = input_parent_chd.open(input_chd_parent_str->second->c_str());
		if (err != CHDERR_NONE)
			report_error(1, "Error opening parent CHD file (%s): %s", input_chd_parent_str->second->c_str(), chd_file::error_string(err));
	}

	// process input file
	auto input_chd_str = params.find(OPTION_INPUT);
	if (input_chd_str != params.end())
	{
		chd_error err = input_chd.open(input_chd_str->second->c_str(), writeable, input_parent_chd.opened() ? &input_parent_chd : nullptr);
		if (err != CHDERR_NONE)
			report_error(1, "Error opening CHD file (%s): %s", input_chd_str->second->c_str(), chd_file::error_string(err));
	}
}


//-------------------------------------------------
//  parse_input_start_end - parse input start/end
//  parameters in a standard way
//-------------------------------------------------

static void parse_input_start_end(const parameters_map &params, uint64_t logical_size, uint32_t hunkbytes, uint32_t framebytes, uint64_t &input_start, uint64_t &input_end)
{
	// process start/end if we were provided an input CHD
	input_start = 0;
	input_end = logical_size;

	// process input start
	auto input_start_byte_str = params.find(OPTION_INPUT_START_BYTE);
	auto input_start_hunk_str = params.find(OPTION_INPUT_START_HUNK);
	auto input_start_frame_str = params.find(OPTION_INPUT_START_FRAME);
	if (input_start_byte_str != params.end())
		input_start = parse_number(input_start_byte_str->second->c_str());
	if (input_start_hunk_str != params.end())
		input_start = parse_number(input_start_hunk_str->second->c_str()) * hunkbytes;
	if (input_start_frame_str != params.end())
		input_start = parse_number(input_start_frame_str->second->c_str()) * framebytes;
	if (input_start >= input_end)
		report_error(1, "Input start offset greater than input file size");

	// process input length
	auto input_length_bytes_str = params.find(OPTION_INPUT_LENGTH_BYTES);
	auto input_length_hunks_str = params.find(OPTION_INPUT_LENGTH_HUNKS);
	auto input_length_frames_str = params.find(OPTION_INPUT_LENGTH_FRAMES);
	uint64_t input_length = input_end;
	if (input_length_bytes_str != params.end())
		input_length = parse_number(input_length_bytes_str->second->c_str());
	if (input_length_hunks_str != params.end())
		input_length = parse_number(input_length_hunks_str->second->c_str()) * hunkbytes;
	if (input_length_frames_str != params.end())
		input_length = parse_number(input_length_frames_str->second->c_str()) * framebytes;
	if (input_start + input_length < input_end)
		input_end = input_start + input_length;
}


//-------------------------------------------------
//  check_existing_output_file - see if an output
//  file already exists, and error if it does,
//  unless --force is specified
//-------------------------------------------------

static void check_existing_output_file(const parameters_map &params, const char *filename)
{
	if (params.find(OPTION_OUTPUT_FORCE) == params.end())
	{
		util::core_file::ptr file;
		osd_file::error filerr = util::core_file::open(filename, OPEN_FLAG_READ, file);
		if (filerr == osd_file::error::NONE)
		{
			file.reset();
			report_error(1, "Error: file already exists (%s)\nUse --force (or -f) to force overwriting", filename);
		}
	}
}


//-------------------------------------------------
//  parse_output_chd_parameters - parse the
//  standard set of output CHD parameters
//-------------------------------------------------

static std::string *parse_output_chd_parameters(const parameters_map &params, chd_file &output_parent_chd)
{
	// process output parent file
	auto output_chd_parent_str = params.find(OPTION_OUTPUT_PARENT);
	if (output_chd_parent_str != params.end())
	{
		chd_error err = output_parent_chd.open(output_chd_parent_str->second->c_str());
		if (err != CHDERR_NONE)
			report_error(1, "Error opening parent CHD file (%s): %s", output_chd_parent_str->second->c_str(), chd_file::error_string(err));
	}

	// process output file
	auto output_chd_str = params.find(OPTION_OUTPUT);
	if (output_chd_str != params.end())
		check_existing_output_file(params, output_chd_str->second->c_str());
	return (output_chd_str != params.end()) ? output_chd_str->second : nullptr;
}


//-------------------------------------------------
//  parse_hunk_size - parse the hunk_size
//  parameter in a standard way
//-------------------------------------------------

static void parse_hunk_size(const parameters_map &params, uint32_t required_granularity, uint32_t &hunk_size)
{
	auto hunk_size_str = params.find(OPTION_HUNK_SIZE);
	if (hunk_size_str != params.end())
	{
		hunk_size = parse_number(hunk_size_str->second->c_str());
		if (hunk_size < 16 || hunk_size > 1024 * 1024)
			report_error(1, "Invalid hunk size");
		if (hunk_size % required_granularity != 0)
			report_error(1, "Hunk size is not an even multiple of %d", required_granularity);
	}
}


//-------------------------------------------------
//  parse_compression - parse a standard
//  compression parameter string
//-------------------------------------------------

static void parse_compression(const parameters_map &params, chd_codec_type compression[4])
{
	// see if anything was specified
	auto compression_str = params.find(OPTION_COMPRESSION);
	if (compression_str == params.end())
		return;

	// special case: 'none'
	if (compression_str->second->compare("none")==0)
	{
		compression[0] = compression[1] = compression[2] = compression[3] = CHD_CODEC_NONE;
		return;
	}

	// iterate through compressors
	int index = 0;
	for (int start = 0, end = compression_str->second->find_first_of(','); index < 4; start = end + 1, end = compression_str->second->find_first_of(',', end + 1))
	{
		std::string name(*compression_str->second, start, (end == -1) ? -1 : end - start);
		if (name.length() != 4)
			report_error(1, "Invalid compressor '%s' specified", name.c_str());
		chd_codec_type type = CHD_MAKE_TAG(name[0], name[1], name[2], name[3]);
		if (!chd_codec_list::codec_exists(type))
			report_error(1, "Invalid compressor '%s' specified", name.c_str());
		compression[index++] = type;
		if (end == -1)
			break;
	}

	for(;index < 4; ++index)
	{
		compression[index] = CHD_CODEC_NONE;
	}
}


//-------------------------------------------------
//  parse_numprocessors - handle the numprocessors
//  command
//-------------------------------------------------

static void parse_numprocessors(const parameters_map &params)
{
	auto numprocessors_str = params.find(OPTION_NUMPROCESSORS);
	if (numprocessors_str == params.end())
		return;

	int count = atoi(numprocessors_str->second->c_str());
	if (count > 0)
	{
		extern int osd_num_processors;
		osd_num_processors = count;
	}
}


//-------------------------------------------------
//  compression_string - create a friendly string
//  describing a set of compressors
//-------------------------------------------------

static std::string compression_string(chd_codec_type compression[4])
{
	// output compression types
	if (compression[0] == CHD_CODEC_NONE)
		return "none";

	// iterate over types
	std::string str;
	for (int index = 0; index < 4; index++)
	{
		chd_codec_type type = compression[index];
		if (type == CHD_CODEC_NONE)
			break;
		if (index != 0)
			str.append(", ");
		str.push_back((type >> 24) & 0xff);
		str.push_back((type >> 16) & 0xff);
		str.push_back((type >> 8) & 0xff);
		str.push_back(type & 0xff);
		str.append(" (").append(chd_codec_list::codec_name(type)).append(")");
	}
	return str;
}


//-------------------------------------------------
//  compress_common - standard compression loop
//-------------------------------------------------

static void compress_common(chd_file_compressor &chd)
{
	// begin compressing
	chd.compress_begin();

	// loop until done
	double complete, ratio;
	chd_error err;
	while ((err = chd.compress_continue(complete, ratio)) == CHDERR_WALKING_PARENT || err == CHDERR_COMPRESSING)
		if (err == CHDERR_WALKING_PARENT)
			progress(false, "Examining parent, %.1f%% complete...  \r", 100.0 * complete);
		else
			progress(false, "Compressing, %.1f%% complete... (ratio=%.1f%%)  \r", 100.0 * complete, 100.0 * ratio);

	// handle errors
	if (err != CHDERR_NONE)
		report_error(1, "Error during compression: %-40s", chd_file::error_string(err));

	// final progress update
	progress(true, "Compression complete ... final ratio = %.1f%%            \n", 100.0 * ratio);
}


//-------------------------------------------------
//  output_track_metadata - output track metadata
//  to a CUE file
//-------------------------------------------------

void output_track_metadata(int mode, util::core_file &file, int tracknum, const cdrom_track_info &info, const char *filename, uint32_t frameoffs, uint64_t discoffs)
{
	if (mode == MODE_GDI)
	{
		int mode = 0, size = 2048;

		switch (info.trktype)
		{
			case CD_TRACK_MODE1:
				mode = 4;
				size = 2048;
				break;

			case CD_TRACK_MODE1_RAW:
				mode = 4;
				size = 2352;
				break;

			case CD_TRACK_MODE2:
				mode = 4;
				size = 2336;
				break;

			case CD_TRACK_MODE2_FORM1:
				mode = 4;
				size = 2048;
				break;

			case CD_TRACK_MODE2_FORM2:
				mode = 4;
				size = 2324;
				break;

			case CD_TRACK_MODE2_FORM_MIX:
				mode = 4;
				size = 2336;
				break;

			case CD_TRACK_MODE2_RAW:
				mode = 4;
				size = 2352;
				break;

			case CD_TRACK_AUDIO:
				mode = 0;
				size = 2352;
				break;
		}
		bool needquote = strchr(filename, ' ') != nullptr;
		file.printf("%d %d %d %d %s%s%s %d\n", tracknum+1, frameoffs, mode, size, needquote?"\"":"", filename, needquote?"\"":"", discoffs);
	}
	else if (mode == MODE_CUEBIN)
	{
		// first track specifies the file
		if (tracknum == 0)
			file.printf("FILE \"%s\" BINARY\n", filename);

		// determine submode
		std::string tempstr;
		switch (info.trktype)
		{
			case CD_TRACK_MODE1:
			case CD_TRACK_MODE1_RAW:
				tempstr = string_format("MODE1/%04d", info.datasize);
				break;

			case CD_TRACK_MODE2:
			case CD_TRACK_MODE2_FORM1:
			case CD_TRACK_MODE2_FORM2:
			case CD_TRACK_MODE2_FORM_MIX:
			case CD_TRACK_MODE2_RAW:
				tempstr = string_format("MODE2/%04d", info.datasize);
				break;

			case CD_TRACK_AUDIO:
				tempstr.assign("AUDIO");
				break;
		}

		// output TRACK entry
		file.printf("  TRACK %02d %s\n", tracknum + 1, tempstr);

		// output PREGAP tag if pregap sectors are not in the file
		if ((info.pregap > 0) && (info.pgdatasize == 0))
		{
			file.printf("    PREGAP %s\n", msf_string_from_frames(info.pregap));
			file.printf("    INDEX 01 %s\n", msf_string_from_frames(frameoffs));
		}
		else if ((info.pregap > 0) && (info.pgdatasize > 0))
		{
			file.printf("    INDEX 00 %s\n", msf_string_from_frames(frameoffs));
			file.printf("    INDEX 01 %s\n", msf_string_from_frames(frameoffs+info.pregap));
		}

		// if no pregap at all, output index 01 only
		if (info.pregap == 0)
		{
			file.printf("    INDEX 01 %s\n", msf_string_from_frames(frameoffs));
		}

		// output POSTGAP
		if (info.postgap > 0)
			file.printf("    POSTGAP %s\n", msf_string_from_frames(info.postgap));
	}
	// non-CUE mode
	else if (mode == MODE_NORMAL)
	{
		// header on the first track
		if (tracknum == 0)
			file.printf("CD_ROM\n\n\n");
		file.printf("// Track %d\n", tracknum + 1);

		// write out the track type
		std::string modesubmode;
		if (info.subtype != CD_SUB_NONE)
			modesubmode = string_format("%s %s", cdrom_get_type_string(info.trktype), cdrom_get_subtype_string(info.subtype));
		else
			modesubmode = string_format("%s", cdrom_get_type_string(info.trktype));
		file.printf("TRACK %s\n", modesubmode);

		// write out the attributes
		file.printf("NO COPY\n");
		if (info.trktype == CD_TRACK_AUDIO)
		{
			file.printf("NO PRE_EMPHASIS\n");
			file.printf("TWO_CHANNEL_AUDIO\n");
		}

		// output pregap
		if (info.pregap > 0)
			file.printf("ZERO %s %s\n", modesubmode, msf_string_from_frames(info.pregap));

		// all tracks but the first one have a file offset
		if (tracknum > 0)
			file.printf("DATAFILE \"%s\" #%d %s // length in bytes: %d\n", filename, uint32_t(discoffs), msf_string_from_frames(info.frames), info.frames * (info.datasize + info.subsize));
		else
			file.printf("DATAFILE \"%s\" %s // length in bytes: %d\n", filename, msf_string_from_frames(info.frames), info.frames * (info.datasize + info.subsize));

		// tracks with pregaps get a START marker too
		if (info.pregap > 0)
			file.printf("START %s\n", msf_string_from_frames(info.pregap));

		file.printf("\n\n");
	}
}


//-------------------------------------------------
//  do_info - dump the header information from
//  a drive image
//-------------------------------------------------

static void do_info(parameters_map &params)
{
	bool verbose = params.find(OPTION_VERBOSE) != params.end();
	// parse out input files
	chd_file input_parent_chd;
	chd_file input_chd;
	parse_input_chd_parameters(params, input_chd, input_parent_chd);

	// print filename and version
	printf("Input file:   %s\n", params.find(OPTION_INPUT)->second->c_str());
	printf("File Version: %d\n", input_chd.version());
	if (input_chd.version() < 3)
		report_error(1, "Unsupported version (%d); use an older chdman to upgrade to version 3 or later", input_chd.version());

	// output cmpression and size information
	chd_codec_type compression[4] = { input_chd.compression(0), input_chd.compression(1), input_chd.compression(2), input_chd.compression(3) };
	printf("Logical size: %s bytes\n", big_int_string(input_chd.logical_bytes()).c_str());
	printf("Hunk Size:    %s bytes\n", big_int_string(input_chd.hunk_bytes()).c_str());
	printf("Total Hunks:  %s\n", big_int_string(input_chd.hunk_count()).c_str());
	printf("Unit Size:    %s bytes\n", big_int_string(input_chd.unit_bytes()).c_str());
	printf("Total Units:  %s\n", big_int_string(input_chd.unit_count()).c_str());
	printf("Compression:  %s\n", compression_string(compression).c_str());
	printf("CHD size:     %s bytes\n", big_int_string(static_cast<util::core_file &>(input_chd).size()).c_str());
	if (compression[0] != CHD_CODEC_NONE)
		printf("Ratio:        %.1f%%\n", 100.0 * double(static_cast<util::core_file &>(input_chd).size()) / double(input_chd.logical_bytes()));

	// add SHA1 output
	util::sha1_t overall = input_chd.sha1();
	if (overall != util::sha1_t::null)
	{
		printf("SHA1:         %s\n", overall.as_string().c_str());
		if (input_chd.version() >= 4)
			printf("Data SHA1:    %s\n", input_chd.raw_sha1().as_string().c_str());
	}
	util::sha1_t parent = input_chd.parent_sha1();
	if (parent != util::sha1_t::null)
		printf("Parent SHA1:  %s\n", parent.as_string().c_str());

	// print out metadata
	std::vector<uint8_t> buffer;
	std::vector<metadata_index_info> info;
	for (int index = 0; ; index++)
	{
		// get the indexed metadata item; stop when we hit an error
		chd_metadata_tag metatag;
		uint8_t metaflags;
		chd_error err = input_chd.read_metadata(CHDMETATAG_WILDCARD, index, buffer, metatag, metaflags);
		if (err != CHDERR_NONE)
			break;

		// determine our index
		uint32_t metaindex = ~0;
		for (auto & elem : info)
			if (elem.tag == metatag)
			{
				metaindex = ++elem.index;
				break;
			}

		// if not found, add to our tracking
		if (metaindex == ~0)
		{
			metadata_index_info curinfo = { metatag, 0 };
			info.push_back(curinfo);
			metaindex = 0;
		}

		// print either a string representation or a hex representation of the tag
		if (isprint((metatag >> 24) & 0xff) && isprint((metatag >> 16) & 0xff) && isprint((metatag >> 8) & 0xff) && isprint(metatag & 0xff))
			printf("Metadata:     Tag='%c%c%c%c'  Index=%d  Length=%d bytes\n", (metatag >> 24) & 0xff, (metatag >> 16) & 0xff, (metatag >> 8) & 0xff, metatag & 0xff, metaindex, int(buffer.size()));
		else
			printf("Metadata:     Tag=%08x  Index=%d  Length=%d bytes\n", metatag, metaindex, int(buffer.size()));
		printf("              ");

		uint32_t count = buffer.size();
		// limit output to 60 characters of metadata if not verbose
		if (!verbose)
			count = std::min(60U, count);
		for (int chnum = 0; chnum < count; chnum++)
			printf("%c", isprint(uint8_t(buffer[chnum])) ? buffer[chnum] : '.');
		printf("\n");
	}

	// print compression stats if verbose
	if (verbose)
	{
		uint32_t compression_types[10] = { 0 };
		for (uint32_t hunknum = 0; hunknum < input_chd.hunk_count(); hunknum++)
		{
			// get info on this hunk
			chd_codec_type codec;
			uint32_t compbytes;
			chd_error err = input_chd.hunk_info(hunknum, codec, compbytes);
			if (err != CHDERR_NONE)
				report_error(1, "Error getting info on hunk %d: %s", hunknum, chd_file::error_string(err));

			// decode into our data
			if (codec > CHD_CODEC_MINI)
				for (int comptype = 0; comptype < 4; comptype++)
					if (codec == input_chd.compression(comptype))
					{
						codec = CHD_CODEC_MINI + 1 + comptype;
						break;
					}
			if (codec > ARRAY_LENGTH(compression_types))
				codec = ARRAY_LENGTH(compression_types) - 1;

			// count stats
			compression_types[codec]++;
		}

		// output the stats
		printf("\n");
		printf("     Hunks  Percent  Name\n");
		printf("----------  -------  ------------------------------------\n");
		for (int comptype = 0; comptype < ARRAY_LENGTH(compression_types); comptype++)
			if (compression_types[comptype] != 0)
			{
				// determine the name
				const char *name = "Unknown";
				switch (comptype)
				{
					case CHD_CODEC_NONE:        name = "Uncompressed";                  break;
					case CHD_CODEC_SELF:        name = "Copy from self";                break;
					case CHD_CODEC_PARENT:      name = "Copy from parent";              break;
					case CHD_CODEC_MINI:        name = "Legacy 8-byte mini";            break;
					default:
						int index = comptype - 1 - CHD_CODEC_MINI;
						if (index < 4)
							name = chd_codec_list::codec_name(input_chd.compression(index));
						break;
				}

				// output the stats
				printf("%10s   %5.1f%%  %-40s\n",
						big_int_string(compression_types[comptype]).c_str(),
						100.0 * double(compression_types[comptype]) / double(input_chd.hunk_count()),
						name);
			}
	}
}


//-------------------------------------------------
//  do_verify - validate the SHA1 on a CHD
//-------------------------------------------------

static void do_verify(parameters_map &params)
{
	// parse out input files
	chd_file input_parent_chd;
	chd_file input_chd;
	parse_input_chd_parameters(params, input_chd, input_parent_chd);

	// only makes sense for compressed CHDs with valid SHA1's
	if (!input_chd.compressed())
		report_error(0, "No verification to be done; CHD is uncompressed");
	util::sha1_t raw_sha1 = (input_chd.version() <= 3) ? input_chd.sha1() : input_chd.raw_sha1();
	if (raw_sha1 == util::sha1_t::null)
		report_error(0, "No verification to be done; CHD has no checksum");

	// create an array to read into
	std::vector<uint8_t> buffer((TEMP_BUFFER_SIZE / input_chd.hunk_bytes()) * input_chd.hunk_bytes());

	// read all the data and build up an SHA-1
	util::sha1_creator rawsha1;
	for (uint64_t offset = 0; offset < input_chd.logical_bytes(); )
	{
		progress(false, "Verifying, %.1f%% complete... \r", 100.0 * double(offset) / double(input_chd.logical_bytes()));

		// determine how much to read
		uint32_t bytes_to_read = (std::min<uint64_t>)(buffer.size(), input_chd.logical_bytes() - offset);
		chd_error err = input_chd.read_bytes(offset, &buffer[0], bytes_to_read);
		if (err != CHDERR_NONE)
			report_error(1, "Error reading CHD file (%s): %s", params.find(OPTION_INPUT)->second->c_str(), chd_file::error_string(err));

		// add to the checksum
		rawsha1.append(&buffer[0], bytes_to_read);
		offset += bytes_to_read;
	}
	util::sha1_t computed_sha1 = rawsha1.finish();

	// finish up
	if (raw_sha1 != computed_sha1)
	{
		fprintf(stderr, "Error: Raw SHA1 in header = %s\n", raw_sha1.as_string().c_str());
		fprintf(stderr, "              actual SHA1 = %s\n", computed_sha1.as_string().c_str());

		// fix it if requested; this also fixes the overall one so we don't need to do any more
		if (params.find(OPTION_FIX) != params.end())
		{
			input_chd.set_raw_sha1(computed_sha1);
			printf("SHA-1 updated to correct value in input CHD\n");
		}
	}
	else
	{
		printf("Raw SHA1 verification successful!\n");

		// now include the metadata for >= v4
		if (input_chd.version() >= 4)
		{
			util::sha1_t computed_overall_sha1 = input_chd.compute_overall_sha1(computed_sha1);
			if (input_chd.sha1() == computed_overall_sha1)
				printf("Overall SHA1 verification successful!\n");
			else
			{
				fprintf(stderr, "Error: Overall SHA1 in header = %s\n", input_chd.sha1().as_string().c_str());
				fprintf(stderr, "                  actual SHA1 = %s\n", computed_overall_sha1.as_string().c_str());

				// fix it if requested
				if (params.find(OPTION_FIX) != params.end())
				{
					input_chd.set_raw_sha1(computed_sha1);
					printf("SHA-1 updated to correct value in input CHD\n");
				}
			}
		}
	}
}


//-------------------------------------------------
//  do_create_raw - create a new compressed raw
//  image from a raw file
//-------------------------------------------------

static void do_create_raw(parameters_map &params)
{
	// process input file
	util::core_file::ptr input_file;
	auto input_file_str = params.find(OPTION_INPUT);
	if (input_file_str != params.end())
	{
		osd_file::error filerr = util::core_file::open(*input_file_str->second, OPEN_FLAG_READ, input_file);
		if (filerr != osd_file::error::NONE)
			report_error(1, "Unable to open file (%s)", input_file_str->second->c_str());
	}

	// process output CHD
	chd_file output_parent;
	std::string *output_chd_str = parse_output_chd_parameters(params, output_parent);

	// process hunk size
	uint32_t hunk_size = output_parent.opened() ? output_parent.hunk_bytes() : 0;
	parse_hunk_size(params, 1, hunk_size);

	// process unit size
	uint32_t unit_size = output_parent.opened() ? output_parent.unit_bytes() : 0;
	auto unit_size_str = params.find(OPTION_UNIT_SIZE);
	if (unit_size_str != params.end())
	{
		unit_size = parse_number(unit_size_str->second->c_str());
		if (hunk_size % unit_size != 0)
			report_error(1, "Unit size is not an even divisor of the hunk size");
	}

	// process input start/end (needs to know hunk_size)
	uint64_t input_start;
	uint64_t input_end;
	parse_input_start_end(params, input_file->size(), hunk_size, hunk_size, input_start, input_end);

	// process compression
	chd_codec_type compression[4];
	memcpy(compression, s_default_raw_compression, sizeof(compression));
	parse_compression(params, compression);

	// process numprocessors
	parse_numprocessors(params);

	// print some info
	printf("Output CHD:   %s\n", output_chd_str->c_str());
	if (output_parent.opened())
		printf("Parent CHD:   %s\n", params.find(OPTION_OUTPUT_PARENT)->second->c_str());
	printf("Input file:   %s\n", input_file_str->second->c_str());
	if (input_start != 0 || input_end != input_file->size())
	{
		printf("Input start:  %s\n", big_int_string(input_start).c_str());
		printf("Input length: %s\n", big_int_string(input_end - input_start).c_str());
	}
	printf("Compression:  %s\n", compression_string(compression).c_str());
	printf("Hunk size:    %s\n", big_int_string(hunk_size).c_str());
	printf("Logical size: %s\n", big_int_string(input_end - input_start).c_str());

	// catch errors so we can close & delete the output file
	try
	{
		// create the new CHD
		std::unique_ptr<chd_file_compressor> chd(new chd_rawfile_compressor(*input_file, input_start, input_end));
		chd_error err;
		if (output_parent.opened())
			err = chd->create(output_chd_str->c_str(), input_end - input_start, hunk_size, compression, output_parent);
		else
			err = chd->create(output_chd_str->c_str(), input_end - input_start, hunk_size, unit_size, compression);
		if (err != CHDERR_NONE)
			report_error(1, "Error creating CHD file (%s): %s", output_chd_str->c_str(), chd_file::error_string(err));

		// if we have a parent, copy forward all the metadata
		if (output_parent.opened())
			chd->clone_all_metadata(output_parent);

		// compress it generically
		compress_common(*chd);
	}
	catch (...)
	{
		// delete the output file
		auto output_chd_str = params.find(OPTION_OUTPUT);
		if (output_chd_str != params.end())
			osd_file::remove(*output_chd_str->second);
		throw;
	}
}


//-------------------------------------------------
//  do_create_hd - create a new compressed hard
//  disk image from a raw file
//-------------------------------------------------

static void do_create_hd(parameters_map &params)
{
	// process input file
	util::core_file::ptr input_file;
	auto input_file_str = params.find(OPTION_INPUT);
	if (input_file_str != params.end())
	{
		osd_file::error filerr = util::core_file::open(*input_file_str->second, OPEN_FLAG_READ, input_file);
		if (filerr != osd_file::error::NONE)
			report_error(1, "Unable to open file (%s)", input_file_str->second->c_str());
	}

	// process output CHD
	chd_file output_parent;
	std::string *output_chd_str = parse_output_chd_parameters(params, output_parent);

	// process sectorsize
	uint32_t sector_size = output_parent.opened() ? output_parent.unit_bytes() : IDE_SECTOR_SIZE;
	auto sectorsize_str = params.find(OPTION_SECTOR_SIZE);
	if (sectorsize_str != params.end())
	{
		if (output_parent.opened())
			report_error(1, "Sector size does not apply when creating a diff from the parent");
		sector_size = parse_number(sectorsize_str->second->c_str());
	}

	// process hunk size (needs to know sector_size)
	uint32_t hunk_size = output_parent.opened() ? output_parent.hunk_bytes() : std::max((4096 / sector_size) * sector_size, sector_size);
	parse_hunk_size(params, sector_size, hunk_size);

	// process input start/end (needs to know hunk_size)
	uint64_t filesize = 0;
	uint64_t input_start = 0;
	uint64_t input_end = 0;
	if (input_file)
	{
		parse_input_start_end(params, input_file->size(), hunk_size, hunk_size, input_start, input_end);
		filesize = input_end - input_start;
	}
	else
	{
		auto size_str = params.find(OPTION_SIZE);
		if (size_str != params.end())
		{
			if (sscanf(size_str->second->c_str(), "%" I64FMT"d", &filesize) != 1)
				report_error(1, "Invalid size string");
		}
	}

	// process compression
	chd_codec_type compression[4];
	memcpy(compression, s_default_hd_compression, sizeof(compression));
	if (!input_file)
		compression[0] = compression[1] = compression[2] = compression[3] = CHD_CODEC_NONE;
	parse_compression(params, compression);
	if (!input_file && compression[0] != CHD_CODEC_NONE)
		report_error(1, "Blank hard disks must be uncompressed");

	// process numprocessors
	parse_numprocessors(params);

	// process chs
	uint32_t cylinders = 0;
	uint32_t heads = 0;
	uint32_t sectors = 0;
	auto chs_str = params.find(OPTION_CHS);
	if (chs_str != params.end())
	{
		if (output_parent.opened())
			report_error(1, "CHS does not apply when creating a diff from the parent");
		if (sscanf(chs_str->second->c_str(), "%d,%d,%d", &cylinders, &heads, &sectors) != 3)
			report_error(1, "Invalid CHS string; must be of the form <cylinders>,<heads>,<sectors>");
	}

	// process ident
	std::vector<uint8_t> identdata;
	if (output_parent.opened())
		output_parent.read_metadata(HARD_DISK_IDENT_METADATA_TAG, 0, identdata);
	auto ident_str = params.find(OPTION_IDENT);
	if (ident_str != params.end())
	{
		// load the file
		osd_file::error filerr = util::core_file::load(ident_str->second->c_str(), identdata);
		if (filerr != osd_file::error::NONE)
			report_error(1, "Error reading ident file (%s)", ident_str->second->c_str());

		// must be at least 14 bytes; extract CHS data from there
		if (identdata.size() < 14)
			report_error(1, "Ident file '%s' is invalid (too short)", ident_str->second->c_str());
		cylinders = (identdata[3] << 8) | identdata[2];
		heads = (identdata[7] << 8) | identdata[6];
		sectors = (identdata[13] << 8) | identdata[12];

		// ignore CHS for > 8GB drives
		if (cylinders * heads * sectors >= 16514064)
			cylinders = 0;
	}

	// process template
	auto template_str = params.find(OPTION_TEMPLATE);
	if (template_str != params.end())
	{
		uint32_t id = parse_number(template_str->second->c_str());

		if (id >= ARRAY_LENGTH(s_hd_templates))
			report_error(1, "Template '%d' is invalid\n", id);

		cylinders = s_hd_templates[id].cylinders;
		heads = s_hd_templates[id].heads;
		sectors = s_hd_templates[id].sectors;
		sector_size = s_hd_templates[id].sector_size;

		printf("Template:     %s %s\n", s_hd_templates[id].manufacturer, s_hd_templates[id].model);
	}

	// extract geometry from the parent if we have one
	if (output_parent.opened() && cylinders == 0)
	{
		std::string metadata;
		if (output_parent.read_metadata(HARD_DISK_METADATA_TAG, 0, metadata) != CHDERR_NONE)
			report_error(1, "Unable to find hard disk metadata in parent CHD");
		if (sscanf(metadata.c_str(), HARD_DISK_METADATA_FORMAT, &cylinders, &heads, &sectors, &sector_size) != 4)
			report_error(1, "Error parsing hard disk metadata in parent CHD");
	}

	// validate the size
	if (filesize % sector_size != 0)
		report_error(1, "Data size is not divisible by sector size %d", sector_size);

	// if no CHS values, try to guess them
	if (cylinders == 0)
	{
		if (!input_file && filesize == 0)
			report_error(1, "Blank hard drives must specify either a length or a set of CHS values");
		guess_chs((input_file_str != params.end()) ? input_file_str->second : nullptr, filesize, sector_size, cylinders, heads, sectors, sector_size);
	}
	uint32_t totalsectors = cylinders * heads * sectors;

	// print some info
	printf("Output CHD:   %s\n", output_chd_str->c_str());
	if (output_parent.opened())
		printf("Parent CHD:   %s\n", params.find(OPTION_OUTPUT_PARENT)->second->c_str());
	if (input_file)
	{
		printf("Input file:   %s\n", input_file_str->second->c_str());
		if (input_start != 0 || input_end != input_file->size())
		{
			printf("Input start:  %s\n", big_int_string(input_start).c_str());
			printf("Input length: %s\n", big_int_string(filesize).c_str());
		}
	}
	printf("Compression:  %s\n", compression_string(compression).c_str());
	printf("Cylinders:    %d\n", cylinders);
	printf("Heads:        %d\n", heads);
	printf("Sectors:      %d\n", sectors);
	printf("Bytes/sector: %d\n", sector_size);
	printf("Sectors/hunk: %d\n", hunk_size / sector_size);
	printf("Logical size: %s\n", big_int_string(uint64_t(totalsectors) * uint64_t(sector_size)).c_str());

	// catch errors so we can close & delete the output file
	try
	{
		// create the new hard drive
		std::unique_ptr<chd_file_compressor> chd;
		if (input_file) chd.reset(new chd_rawfile_compressor(*input_file, input_start, input_end));
		else chd.reset(new chd_zero_compressor(input_start, input_end));
		chd_error err;
		if (output_parent.opened())
			err = chd->create(output_chd_str->c_str(), uint64_t(totalsectors) * uint64_t(sector_size), hunk_size, compression, output_parent);
		else
			err = chd->create(output_chd_str->c_str(), uint64_t(totalsectors) * uint64_t(sector_size), hunk_size, sector_size, compression);
		if (err != CHDERR_NONE)
			report_error(1, "Error creating CHD file (%s): %s", output_chd_str->c_str(), chd_file::error_string(err));

		// add the standard hard disk metadata
		std::string  metadata = string_format(HARD_DISK_METADATA_FORMAT, cylinders, heads, sectors, sector_size);
		err = chd->write_metadata(HARD_DISK_METADATA_TAG, 0, metadata);
		if (err != CHDERR_NONE)
			report_error(1, "Error adding hard disk metadata: %s", chd_file::error_string(err));

		// write the ident if present
		if (!identdata.empty())
		{
			err = chd->write_metadata(HARD_DISK_IDENT_METADATA_TAG, 0, identdata);
			if (err != CHDERR_NONE)
				report_error(1, "Error adding hard disk metadata: %s", chd_file::error_string(err));
		}

		// compress it generically
		if (input_file)
			compress_common(*chd);
	}
	catch (...)
	{
		// delete the output file
		auto output_chd_str = params.find(OPTION_OUTPUT);
		if (output_chd_str != params.end())
			osd_file::remove(*output_chd_str->second);
		throw;
	}
}


//-------------------------------------------------
//  do_create_cd - create a new compressed CD
//  image from a raw file
//-------------------------------------------------

static void do_create_cd(parameters_map &params)
{
	// process input file
	chdcd_track_input_info track_info;
	cdrom_toc toc = { 0 };
	auto input_file_str = params.find(OPTION_INPUT);
	if (input_file_str != params.end())
	{
		chd_error err = chdcd_parse_toc(input_file_str->second->c_str(), toc, track_info);
		if (err != CHDERR_NONE)
			report_error(1, "Error parsing input file (%s: %s)\n", input_file_str->second->c_str(), chd_file::error_string(err));
	}

	// process output CHD
	chd_file output_parent;
	std::string *output_chd_str = parse_output_chd_parameters(params, output_parent);

	// process hunk size
	uint32_t hunk_size = output_parent.opened() ? output_parent.hunk_bytes() : CD_FRAMES_PER_HUNK * CD_FRAME_SIZE;
	parse_hunk_size(params, CD_FRAME_SIZE, hunk_size);

	// process compression
	chd_codec_type compression[4];
	memcpy(compression, s_default_cd_compression, sizeof(compression));
	parse_compression(params, compression);

	// process numprocessors
	parse_numprocessors(params);

	// pad each track to a 4-frame boundary. cdrom.c will deal with this on the read side
	uint32_t origtotalsectors = 0;
	uint32_t totalsectors = 0;
	for (int tracknum = 0; tracknum < toc.numtrks; tracknum++)
	{
		cdrom_track_info &trackinfo = toc.tracks[tracknum];
		int padded = (trackinfo.frames + CD_TRACK_PADDING - 1) / CD_TRACK_PADDING;
		trackinfo.extraframes = padded * CD_TRACK_PADDING - trackinfo.frames;
		origtotalsectors += trackinfo.frames;
		totalsectors += trackinfo.frames + trackinfo.extraframes;
	}

	// print some info
	printf("Output CHD:   %s\n", output_chd_str->c_str());
	if (output_parent.opened())
		printf("Parent CHD:   %s\n", params.find(OPTION_OUTPUT_PARENT)->second->c_str());
	printf("Input file:   %s\n", input_file_str->second->c_str());
	printf("Input tracks: %d\n", toc.numtrks);
	printf("Input length: %s\n", msf_string_from_frames(origtotalsectors).c_str());
	printf("Compression:  %s\n", compression_string(compression).c_str());
	printf("Logical size: %s\n", big_int_string(uint64_t(totalsectors) * CD_FRAME_SIZE).c_str());

	// catch errors so we can close & delete the output file
	chd_cd_compressor *chd = nullptr;
	try
	{
		// create the new CD
		chd = new chd_cd_compressor(toc, track_info);
		chd_error err;
		if (output_parent.opened())
			err = chd->create(output_chd_str->c_str(), uint64_t(totalsectors) * uint64_t(CD_FRAME_SIZE), hunk_size, compression, output_parent);
		else
			err = chd->create(output_chd_str->c_str(), uint64_t(totalsectors) * uint64_t(CD_FRAME_SIZE), hunk_size, CD_FRAME_SIZE, compression);
		if (err != CHDERR_NONE)
			report_error(1, "Error creating CHD file (%s): %s", output_chd_str->c_str(), chd_file::error_string(err));

		// add the standard CD metadata; we do this even if we have a parent because it might be different
		err = cdrom_write_metadata(chd, &toc);
		if (err != CHDERR_NONE)
			report_error(1, "Error adding CD metadata: %s", chd_file::error_string(err));

		// compress it generically
		compress_common(*chd);
		delete chd;
	}
	catch (...)
	{
		delete chd;
		// delete the output file
		auto output_chd_str = params.find(OPTION_OUTPUT);
		if (output_chd_str != params.end())
			osd_file::remove(*output_chd_str->second);
		throw;
	}
}


//-------------------------------------------------
//  do_create_ld - create a new A/V file from an
//  input AVI file and metadata
//-------------------------------------------------

static void do_create_ld(parameters_map &params)
{
	// process input file
	avi_file::ptr input_file;
	auto input_file_str = params.find(OPTION_INPUT);
	if (input_file_str != params.end())
	{
		avi_file::error avierr = avi_file::open(*input_file_str->second, input_file);
		if (avierr != avi_file::error::NONE)
			report_error(1, "Error opening AVI file (%s): %s\n", input_file_str->second->c_str(), avi_file::error_string(avierr));
	}
	const avi_file::movie_info &aviinfo = input_file->get_movie_info();

	// process output CHD
	chd_file output_parent;
	std::string *output_chd_str = parse_output_chd_parameters(params, output_parent);

	// process input start/end
	uint64_t input_start;
	uint64_t input_end;
	parse_input_start_end(params, aviinfo.video_numsamples, 0, 1, input_start, input_end);

	// determine parameters of the incoming video stream
	avi_info info;
	info.fps_times_1million = uint64_t(aviinfo.video_timescale) * 1000000 / aviinfo.video_sampletime;
	info.width = aviinfo.video_width;
	info.height = aviinfo.video_height;
	info.interlaced = ((info.fps_times_1million / 1000000) <= 30) && (info.height % 2 == 0) && (info.height > 288);
	info.channels = aviinfo.audio_channels;
	info.rate = aviinfo.audio_samplerate;

	// adjust for interlacing
	if (info.interlaced)
	{
		info.fps_times_1million *= 2;
		info.height /= 2;
		input_start *= 2;
		input_end *= 2;
	}

	// determine the number of bytes per frame
	info.max_samples_per_frame = (uint64_t(info.rate) * 1000000 + info.fps_times_1million - 1) / info.fps_times_1million;
	info.bytes_per_frame = avhuff_encoder::raw_data_size(info.width, info.height, info.channels, info.max_samples_per_frame);

	// process hunk size
	uint32_t hunk_size = output_parent.opened() ? output_parent.hunk_bytes() : info.bytes_per_frame;
	parse_hunk_size(params, info.bytes_per_frame, hunk_size);

	// process compression
	chd_codec_type compression[4];
	memcpy(compression, s_default_ld_compression, sizeof(compression));
	parse_compression(params, compression);
	// disable support for uncompressed ones until the extraction code can handle it
	if (compression[0] == CHD_CODEC_NONE)
		report_error(1, "Uncompressed is not supported");

	// process numprocessors
	parse_numprocessors(params);

	// print some info
	printf("Output CHD:   %s\n", output_chd_str->c_str());
	if (output_parent.opened())
		printf("Parent CHD:   %s\n", params.find(OPTION_OUTPUT_PARENT)->second->c_str());
	printf("Input file:   %s\n", input_file_str->second->c_str());
	if (input_start != 0 && input_end != aviinfo.video_numsamples)
		printf("Input start:  %s\n", big_int_string(input_start).c_str());
	printf("Input length: %s (%02d:%02d:%02d)\n", big_int_string(input_end - input_start).c_str(),
			uint32_t((uint64_t(input_end - input_start) * 1000000 / info.fps_times_1million / 60 / 60)),
			uint32_t(((uint64_t(input_end - input_start) * 1000000 / info.fps_times_1million / 60) % 60)),
			uint32_t(((uint64_t(input_end - input_start) * 1000000 / info.fps_times_1million) % 60)));
	printf("Frame rate:   %d.%06d\n", info.fps_times_1million / 1000000, info.fps_times_1million % 1000000);
	printf("Frame size:   %d x %d %s\n", info.width, info.height * (info.interlaced ? 2 : 1), info.interlaced ? "interlaced" : "non-interlaced");
	printf("Audio:        %d channels at %d Hz\n", info.channels, info.rate);
	printf("Compression:  %s\n", compression_string(compression).c_str());
	printf("Hunk size:    %s\n", big_int_string(hunk_size).c_str());
	printf("Logical size: %s\n", big_int_string(uint64_t(input_end - input_start) * hunk_size).c_str());

	// catch errors so we can close & delete the output file
	chd_avi_compressor *chd = nullptr;
	try
	{
		// create the new CHD
		chd = new chd_avi_compressor(*input_file, info, input_start, input_end);
		chd_error err;
		if (output_parent.opened())
			err = chd->create(output_chd_str->c_str(), uint64_t(input_end - input_start) * hunk_size, hunk_size, compression, output_parent);
		else
			err = chd->create(output_chd_str->c_str(), uint64_t(input_end - input_start) * hunk_size, hunk_size, info.bytes_per_frame, compression);
		if (err != CHDERR_NONE)
			report_error(1, "Error creating CHD file (%s): %s", output_chd_str->c_str(), chd_file::error_string(err));

		// write the core A/V metadata
		std::string metadata = string_format(AV_METADATA_FORMAT, info.fps_times_1million / 1000000, info.fps_times_1million % 1000000, info.width, info.height, info.interlaced, info.channels, info.rate);
		err = chd->write_metadata(AV_METADATA_TAG, 0, metadata);
		if (err != CHDERR_NONE)
			report_error(1, "Error adding AV metadata: %s\n", chd_file::error_string(err));

		// create the compressor and then run it generically
		compress_common(*chd);

		// write the final LD metadata
		if (info.height == 524/2 || info.height == 624/2)
		{
			err = chd->write_metadata(AV_LD_METADATA_TAG, 0, chd->ldframedata(), 0);
			if (err != CHDERR_NONE)
				report_error(1, "Error adding AVLD metadata: %s\n", chd_file::error_string(err));
		}
		delete chd;
	}
	catch (...)
	{
		delete chd;
		// delete the output file
		auto output_chd_str = params.find(OPTION_OUTPUT);
		if (output_chd_str != params.end())
			osd_file::remove(*output_chd_str->second);
		throw;
	}
}


//-------------------------------------------------
//  do_copy - create a new CHD with data from
//  another CHD
//-------------------------------------------------

static void do_copy(parameters_map &params)
{
	// parse out input files
	chd_file input_parent_chd;
	chd_file input_chd;
	parse_input_chd_parameters(params, input_chd, input_parent_chd);

	// parse out input start/end
	uint64_t input_start;
	uint64_t input_end;
	parse_input_start_end(params, input_chd.logical_bytes(), input_chd.hunk_bytes(), input_chd.hunk_bytes(), input_start, input_end);

	// process output CHD
	chd_file output_parent;
	std::string *output_chd_str = parse_output_chd_parameters(params, output_parent);

	// process hunk size
	uint32_t hunk_size = input_chd.hunk_bytes();
	parse_hunk_size(params, 1, hunk_size);
	if (hunk_size % input_chd.hunk_bytes() != 0 && input_chd.hunk_bytes() % hunk_size != 0)
		report_error(1, "Hunk size is not an even multiple or divisor of input hunk size");

	// process compression; we default to our current preferences using metadata to pick the type
	chd_codec_type compression[4];
	{
		std::vector<uint8_t> metadata;
		if (input_chd.read_metadata(HARD_DISK_METADATA_TAG, 0, metadata) == CHDERR_NONE)
			memcpy(compression, s_default_hd_compression, sizeof(compression));
		else if (input_chd.read_metadata(AV_METADATA_TAG, 0, metadata) == CHDERR_NONE)
			memcpy(compression, s_default_ld_compression, sizeof(compression));
		else if (input_chd.read_metadata(CDROM_OLD_METADATA_TAG, 0, metadata) == CHDERR_NONE ||
					input_chd.read_metadata(CDROM_TRACK_METADATA_TAG, 0, metadata) == CHDERR_NONE ||
					input_chd.read_metadata(CDROM_TRACK_METADATA2_TAG, 0, metadata) == CHDERR_NONE ||
					input_chd.read_metadata(GDROM_OLD_METADATA_TAG, 0, metadata) == CHDERR_NONE ||
					input_chd.read_metadata(GDROM_TRACK_METADATA_TAG, 0, metadata) == CHDERR_NONE)
					memcpy(compression, s_default_cd_compression, sizeof(compression));
		else
			memcpy(compression, s_default_raw_compression, sizeof(compression));
	}
	parse_compression(params, compression);

	// process numprocessors
	parse_numprocessors(params);

	// print some info
	printf("Output CHD:   %s\n", output_chd_str->c_str());
	if (output_parent.opened())
		printf("Parent CHD:   %s\n", params.find(OPTION_OUTPUT_PARENT)->second->c_str());
	printf("Input CHD:    %s\n", params.find(OPTION_INPUT)->second->c_str());
	if (input_start != 0 || input_end != input_chd.logical_bytes())
	{
		printf("Input start:  %s\n", big_int_string(input_start).c_str());
		printf("Input length: %s\n", big_int_string(input_end - input_start).c_str());
	}
	printf("Compression:  %s\n", compression_string(compression).c_str());
	printf("Hunk size:    %s\n", big_int_string(hunk_size).c_str());
	printf("Logical size: %s\n", big_int_string(input_end - input_start).c_str());

	// catch errors so we can close & delete the output file
	chd_chdfile_compressor *chd = nullptr;
	try
	{
		// create the new CHD
		chd = new chd_chdfile_compressor(input_chd, input_start, input_end);
		chd_error err;
		if (output_parent.opened())
			err = chd->create(output_chd_str->c_str(), input_end - input_start, hunk_size, compression, output_parent);
		else
			err = chd->create(output_chd_str->c_str(), input_end - input_start, hunk_size, input_chd.unit_bytes(), compression);
		if (err != CHDERR_NONE)
			report_error(1, "Error creating CHD file (%s): %s", output_chd_str->c_str(), chd_file::error_string(err));

		// clone all the metadata, upgrading where appropriate
		std::vector<uint8_t> metadata;
		chd_metadata_tag metatag;
		uint8_t metaflags;
		uint32_t index = 0;
		bool redo_cd = false;
		bool cdda_swap = false;
		for (err = input_chd.read_metadata(CHDMETATAG_WILDCARD, index++, metadata, metatag, metaflags); err == CHDERR_NONE; err = input_chd.read_metadata(CHDMETATAG_WILDCARD, index++, metadata, metatag, metaflags))
		{
			// if this is an old CD-CHD tag, note that we want to re-do it
			if (metatag == CDROM_OLD_METADATA_TAG || metatag == CDROM_TRACK_METADATA_TAG)
			{
				redo_cd = true;
				continue;
			}
			// if this is old GD tag we want re-do it and swap CDDA
			if (metatag == GDROM_OLD_METADATA_TAG)
			{
				cdda_swap = redo_cd = true;
				continue;
			}

			// otherwise, clone it
			err = chd->write_metadata(metatag, CHDMETAINDEX_APPEND, metadata, metaflags);
			if (err != CHDERR_NONE)
				report_error(1, "Error writing cloned metadata: %s", chd_file::error_string(err));
		}

		// if we need to re-do the CD metadata, do it now
		if (redo_cd)
		{
			cdrom_file *cdrom = cdrom_open(&input_chd);
			if (cdrom == nullptr)
				report_error(1, "Error upgrading CD metadata");
			const cdrom_toc *toc = cdrom_get_toc(cdrom);
			err = cdrom_write_metadata(chd, toc);
			if (err != CHDERR_NONE)
				report_error(1, "Error writing upgraded CD metadata: %s", chd_file::error_string(err));
			if (cdda_swap)
				chd->m_toc = toc;
		}

		// compress it generically
		compress_common(*chd);
		delete chd;
	}
	catch (...)
	{
		delete chd;
		// delete the output file
		auto output_chd_str = params.find(OPTION_OUTPUT);
		if (output_chd_str != params.end())
			osd_file::remove(*output_chd_str->second);
		throw;
	}
}


//-------------------------------------------------
//  do_extract_raw - extract a raw file from a
//  CHD image
//-------------------------------------------------

static void do_extract_raw(parameters_map &params)
{
	// parse out input files
	chd_file input_parent_chd;
	chd_file input_chd;
	parse_input_chd_parameters(params, input_chd, input_parent_chd);

	// parse out input start/end
	uint64_t input_start;
	uint64_t input_end;
	parse_input_start_end(params, input_chd.logical_bytes(), input_chd.hunk_bytes(), input_chd.hunk_bytes(), input_start, input_end);

	// verify output file doesn't exist
	auto output_file_str = params.find(OPTION_OUTPUT);
	if (output_file_str != params.end())
		check_existing_output_file(params, output_file_str->second->c_str());

	// print some info
	printf("Output File:  %s\n", output_file_str->second->c_str());
	printf("Input CHD:    %s\n", params.find(OPTION_INPUT)->second->c_str());
	if (input_start != 0 || input_end != input_chd.logical_bytes())
	{
		printf("Input start:  %s\n", big_int_string(input_start).c_str());
		printf("Input length: %s\n", big_int_string(input_end - input_start).c_str());
	}

	// catch errors so we can close & delete the output file
	util::core_file::ptr output_file;
	try
	{
		// process output file
		osd_file::error filerr = util::core_file::open(*output_file_str->second, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, output_file);
		if (filerr != osd_file::error::NONE)
			report_error(1, "Unable to open file (%s)", output_file_str->second->c_str());

		// copy all data
		std::vector<uint8_t> buffer((TEMP_BUFFER_SIZE / input_chd.hunk_bytes()) * input_chd.hunk_bytes());
		for (uint64_t offset = input_start; offset < input_end; )
		{
			progress(false, "Extracting, %.1f%% complete... \r", 100.0 * double(offset - input_start) / double(input_end - input_start));

			// determine how much to read
			uint32_t bytes_to_read = (std::min<uint64_t>)(buffer.size(), input_end - offset);
			chd_error err = input_chd.read_bytes(offset, &buffer[0], bytes_to_read);
			if (err != CHDERR_NONE)
				report_error(1, "Error reading CHD file (%s): %s", params.find(OPTION_INPUT)->second->c_str(), chd_file::error_string(err));

			// write to the output
			uint32_t count = output_file->write(&buffer[0], bytes_to_read);
			if (count != bytes_to_read)
				report_error(1, "Error writing to file; check disk space (%s)", output_file_str->second->c_str());

			// advance
			offset += bytes_to_read;
		}

		// finish up
		output_file.reset();
		printf("Extraction complete                                    \n");
	}
	catch (...)
	{
		// delete the output file
		if (output_file != nullptr)
		{
			output_file.reset();
			osd_file::remove(*output_file_str->second);
		}
		throw;
	}
}


//-------------------------------------------------
//  do_extract_cd - extract a CD file from a
//  CHD image
//-------------------------------------------------

static void do_extract_cd(parameters_map &params)
{
	// parse out input files
	chd_file input_parent_chd;
	chd_file input_chd;
	parse_input_chd_parameters(params, input_chd, input_parent_chd);

	// further process input file
	cdrom_file *cdrom = cdrom_open(&input_chd);
	if (cdrom == nullptr)
		report_error(1, "Unable to recognize CHD file as a CD");
	const cdrom_toc *toc = cdrom_get_toc(cdrom);

	// verify output file doesn't exist
	auto output_file_str = params.find(OPTION_OUTPUT);
	if (output_file_str != params.end())
		check_existing_output_file(params, output_file_str->second->c_str());

	// verify output BIN file doesn't exist
	auto output_bin_file_fnd = params.find(OPTION_OUTPUT_BIN);
	std::string default_name(*output_file_str->second);
	int chop = default_name.find_last_of('.');
	if (chop != -1)
		default_name.erase(chop, default_name.size());
	char basename[128];
	strncpy(basename, default_name.c_str(), 127);
	default_name.append(".bin");
	std::string *output_bin_file_str;
	if (output_bin_file_fnd == params.end())
		output_bin_file_str = &default_name;
	else
		output_bin_file_str = output_bin_file_fnd->second;

	check_existing_output_file(params, output_bin_file_str->c_str());

	// print some info
	printf("Output TOC:   %s\n", output_file_str->second->c_str());
	printf("Output Data:  %s\n", output_bin_file_str->c_str());
	printf("Input CHD:    %s\n", params.find(OPTION_INPUT)->second->c_str());

	// catch errors so we can close & delete the output file
	util::core_file::ptr output_bin_file;
	util::core_file::ptr output_toc_file;
	try
	{
		int mode = MODE_NORMAL;

		if (output_file_str->second->find(".cue") != -1)
		{
			mode = MODE_CUEBIN;
		}
		else if (output_file_str->second->find(".gdi") != -1)
		{
			mode = MODE_GDI;
		}

		// process output file
		osd_file::error filerr = util::core_file::open(*output_file_str->second, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_NO_BOM, output_toc_file);
		if (filerr != osd_file::error::NONE)
			report_error(1, "Unable to open file (%s)", output_file_str->second->c_str());

		// process output BIN file
		if (mode != MODE_GDI)
		{
			filerr = util::core_file::open(*output_bin_file_str, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, output_bin_file);
			if (filerr != osd_file::error::NONE)
				report_error(1, "Unable to open file (%s)", output_bin_file_str->c_str());
		}

		// determine total frames
		uint64_t total_bytes = 0;
		for (int tracknum = 0; tracknum < toc->numtrks; tracknum++)
			total_bytes += toc->tracks[tracknum].frames * (toc->tracks[tracknum].datasize + toc->tracks[tracknum].subsize);

		// GDI must start with the # of tracks
		if (mode == MODE_GDI)
		{
			output_toc_file->printf("%d\n", toc->numtrks);
		}

		// iterate over tracks and copy all data
		uint64_t outputoffs = 0;
		uint32_t discoffs = 0;
		std::vector<uint8_t> buffer;
		for (int tracknum = 0; tracknum < toc->numtrks; tracknum++)
		{
			std::string trackbin_name(basename);

			if (mode == MODE_GDI)
			{
				char temp[11];
				sprintf(temp, "%02d", tracknum+1);
				trackbin_name.append(temp);
				if (toc->tracks[tracknum].trktype == CD_TRACK_AUDIO)
					trackbin_name.append(".raw");
				else
					trackbin_name.append(".bin");

				output_bin_file.reset();

				filerr = util::core_file::open(trackbin_name, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, output_bin_file);
				if (filerr != osd_file::error::NONE)
					report_error(1, "Unable to open file (%s)", trackbin_name.c_str());

				outputoffs = 0;
			}

			// output the metadata about the track to the TOC file
			const cdrom_track_info &trackinfo = toc->tracks[tracknum];
			if (mode == MODE_GDI)
			{
				output_track_metadata(mode, *output_toc_file, tracknum, trackinfo, core_filename_extract_base(trackbin_name).c_str(), discoffs, outputoffs);
			}
			else
			{
				output_track_metadata(mode, *output_toc_file, tracknum, trackinfo, core_filename_extract_base(*output_bin_file_str).c_str(), discoffs, outputoffs);
			}

			// If this is bin/cue output and the CHD contains subdata, warn the user and don't include
			// the subdata size in the buffer calculation.
			uint32_t output_frame_size = trackinfo.datasize + ((trackinfo.subtype != CD_SUB_NONE) ? trackinfo.subsize : 0);
			if (trackinfo.subtype != CD_SUB_NONE && ((mode == MODE_CUEBIN) || (mode == MODE_GDI)))
			{
				printf("Warning: Track %d has subcode data.  bin/cue and gdi formats cannot contain subcode data and it will be omitted.\n", tracknum+1);
				printf("       : This may affect usage of the output image.  Use bin/toc output to keep all data.\n");
				output_frame_size = trackinfo.datasize;
			}

			// resize the buffer for the track
			buffer.resize((TEMP_BUFFER_SIZE / output_frame_size) * output_frame_size);

			// now read and output the actual data
			uint32_t bufferoffs = 0;
			uint32_t actualframes = trackinfo.frames - trackinfo.padframes;
			for (uint32_t frame = 0; frame < actualframes; frame++)
			{
				progress(false, "Extracting, %.1f%% complete... \r", 100.0 * double(outputoffs) / double(total_bytes));

				// read the data
				cdrom_read_data(cdrom, cdrom_get_track_start_phys(cdrom, tracknum) + frame, &buffer[bufferoffs], trackinfo.trktype, true);

				// for CDRWin and GDI audio tracks must be reversed
				// in the case of GDI and CHD version < 5 we assuming source CHD image is GDROM so audio tracks is already reversed
				if (((mode == MODE_GDI && input_chd.version() > 4) || (mode == MODE_CUEBIN)) && (trackinfo.trktype == CD_TRACK_AUDIO))
					for (int swapindex = 0; swapindex < trackinfo.datasize; swapindex += 2)
					{
						uint8_t swaptemp = buffer[bufferoffs + swapindex];
						buffer[bufferoffs + swapindex] = buffer[bufferoffs + swapindex + 1];
						buffer[bufferoffs + swapindex + 1] = swaptemp;
					}
				bufferoffs += trackinfo.datasize;
				discoffs++;

				// read the subcode data
				if (trackinfo.subtype != CD_SUB_NONE && (mode == MODE_NORMAL))
				{
					cdrom_read_subcode(cdrom, cdrom_get_track_start_phys(cdrom, tracknum) + frame, &buffer[bufferoffs], true);
					bufferoffs += trackinfo.subsize;
				}

				// write it out if we need to
				if (bufferoffs == buffer.size() || frame == actualframes - 1)
				{
					output_bin_file->seek(outputoffs, SEEK_SET);
					uint32_t byteswritten = output_bin_file->write(&buffer[0], bufferoffs);
					if (byteswritten != bufferoffs)
						report_error(1, "Error writing frame %d to file (%s): %s\n", frame, output_file_str->second->c_str(), chd_file::error_string(CHDERR_WRITE_ERROR));
					outputoffs += bufferoffs;
					bufferoffs = 0;
				}
			}

			discoffs += trackinfo.padframes;
		}

		// finish up
		output_bin_file.reset();
		output_toc_file.reset();
		printf("Extraction complete                                    \n");
	}
	catch (...)
	{
		// delete the output files
		output_bin_file.reset();
		output_toc_file.reset();
		osd_file::remove(*output_bin_file_str);
		osd_file::remove(*output_file_str->second);
		throw;
	}
}


//-------------------------------------------------
//  do_extract_ld - extract an AVI file from a
//  CHD image
//-------------------------------------------------

static void do_extract_ld(parameters_map &params)
{
	// parse out input files
	chd_file input_parent_chd;
	chd_file input_chd;
	parse_input_chd_parameters(params, input_chd, input_parent_chd);

	// read core metadata
	std::string metadata;
	chd_error err = input_chd.read_metadata(AV_METADATA_TAG, 0, metadata);
	if (err != CHDERR_NONE)
		report_error(1, "Unable to find A/V metadata in the input CHD");

	// parse the metadata
	uint32_t fps_times_1million;
	uint32_t max_samples_per_frame;
	uint32_t frame_bytes;
	int width;
	int height;
	int interlaced;
	int channels;
	int rate;
	{
		int fps;
		int fpsfrac;
		if (sscanf(metadata.c_str(), AV_METADATA_FORMAT, &fps, &fpsfrac, &width, &height, &interlaced, &channels, &rate) != 7)
			report_error(1, "Improperly formatted A/V metadata found");
		fps_times_1million = fps * 1000000 + fpsfrac;
	}
	uint8_t interlace_factor = interlaced ? 2 : 1;

	// determine key parameters and validate
	max_samples_per_frame = (uint64_t(rate) * 1000000 + fps_times_1million - 1) / fps_times_1million;
	frame_bytes = avhuff_encoder::raw_data_size(width, height, channels, max_samples_per_frame);
	if (frame_bytes != input_chd.hunk_bytes())
		report_error(1, "Frame size does not match hunk size for this CHD");

	// parse out input start/end
	uint64_t input_start;
	uint64_t input_end;
	parse_input_start_end(params, input_chd.hunk_count() / interlace_factor, 0, 1, input_start, input_end);
	input_start *= interlace_factor;
	input_end *= interlace_factor;

	// build up the movie info
	avi_file::movie_info info;
	info.video_format = FORMAT_YUY2;
	info.video_timescale = fps_times_1million / interlace_factor;
	info.video_sampletime = 1000000;
	info.video_width = width;
	info.video_height = height * interlace_factor;
	info.video_depth = 16;
	info.audio_format = 0;
	info.audio_timescale = rate;
	info.audio_sampletime = 1;
	info.audio_channels = channels;
	info.audio_samplebits = 16;
	info.audio_samplerate = rate;

	// verify output file doesn't exist
	auto output_file_str = params.find(OPTION_OUTPUT);
	if (output_file_str != params.end())
		check_existing_output_file(params, output_file_str->second->c_str());

	// print some info
	printf("Output File:  %s\n", output_file_str->second->c_str());
	printf("Input CHD:    %s\n", params.find(OPTION_INPUT)->second->c_str());
	if (input_start != 0 || input_end != input_chd.hunk_count())
	{
		printf("Input start:  %s\n", big_int_string(input_start).c_str());
		printf("Input length: %s\n", big_int_string(input_end - input_start).c_str());
	}

	// catch errors so we can close & delete the output file
	avi_file::ptr output_file;
	try
	{
		// process output file
		avi_file::error avierr = avi_file::create(*output_file_str->second, info, output_file);
		if (avierr != avi_file::error::NONE)
			report_error(1, "Unable to open file (%s)", output_file_str->second->c_str());

		// create the codec configuration
		avhuff_decoder::config avconfig;
		bitmap_yuy16 avvideo;
		std::vector<int16_t> audio_data[16];
		uint32_t actsamples;
		avconfig.video = &avvideo;
		avconfig.maxsamples = max_samples_per_frame;
		avconfig.actsamples = &actsamples;
		for (int chnum = 0; chnum < ARRAY_LENGTH(audio_data); chnum++)
		{
			audio_data[chnum].resize(std::max(1U,max_samples_per_frame));
			avconfig.audio[chnum] = &audio_data[chnum][0];
		}

		// iterate over frames
		bitmap_yuy16 fullbitmap(width, height * interlace_factor);
		for (uint64_t framenum = input_start; framenum < input_end; framenum++)
		{
			progress(framenum == input_start, "Extracting, %.1f%% complete...  \r", 100.0 * double(framenum - input_start) / double(input_end - input_start));

			// set up the fake bitmap for this frame
			avvideo.wrap(&fullbitmap.pix(framenum % interlace_factor), fullbitmap.width(), fullbitmap.height() / interlace_factor, fullbitmap.rowpixels() * interlace_factor);
			input_chd.codec_configure(CHD_CODEC_AVHUFF, AVHUFF_CODEC_DECOMPRESS_CONFIG, &avconfig);

			// read the hunk into the buffers
			chd_error err = input_chd.read_hunk(framenum, nullptr);
			if (err != CHDERR_NONE)
			{
				uint64_t filepos = static_cast<util::core_file &>(input_chd).tell();
				report_error(1, "Error reading hunk %d at offset %d from CHD file (%s): %s\n", framenum, filepos, params.find(OPTION_INPUT)->second->c_str(), chd_file::error_string(err));
			}

			// write audio
			for (int chnum = 0; chnum < channels; chnum++)
			{
				avi_file::error avierr = output_file->append_sound_samples(chnum, avconfig.audio[chnum], actsamples, 0);
				if (avierr != avi_file::error::NONE)
					report_error(1, "Error writing samples for hunk %d to file (%s): %s\n", framenum, output_file_str->second->c_str(), avi_file::error_string(avierr));
			}

			// write video
			if ((framenum + 1) % interlace_factor == 0)
			{
				avi_file::error avierr = output_file->append_video_frame(fullbitmap);
				if (avierr != avi_file::error::NONE)
					report_error(1, "Error writing video for hunk %d to file (%s): %s\n", framenum, output_file_str->second->c_str(), avi_file::error_string(avierr));
			}
		}

		// close and return
		output_file.reset();
		printf("Extraction complete                                    \n");
	}
	catch (...)
	{
		// delete the output file
		output_file.reset();
		osd_file::remove(*output_file_str->second);
		throw;
	}
}


//-------------------------------------------------
//  do_add_metadata - add metadata to a CHD from a
//  file
//-------------------------------------------------

static void do_add_metadata(parameters_map &params)
{
	// parse out input files
	chd_file input_parent_chd;
	chd_file input_chd;
	parse_input_chd_parameters(params, input_chd, input_parent_chd, true);

	// process tag
	chd_metadata_tag tag = CHD_MAKE_TAG('?','?','?','?');
	auto tag_str = params.find(OPTION_TAG);
	if (tag_str != params.end())
	{
		tag_str->second->append("    ");
		tag = CHD_MAKE_TAG((*tag_str->second)[0], (*tag_str->second)[1], (*tag_str->second)[2], (*tag_str->second)[3]);
	}

	// process index
	uint32_t index = 0;
	auto index_str = params.find(OPTION_INDEX);
	if (index_str != params.end())
		index = atoi(index_str->second->c_str());

	// process text input
	auto text_str = params.find(OPTION_VALUE_TEXT);
	std::string text;
	if (text_str != params.end())
	{
		text = *text_str->second;
		if (text[0] == '"' && text[text.length() - 1] == '"')
			*text_str->second = text.substr(1, text.length() - 2);
	}

	// process file input
	auto file_str = params.find(OPTION_VALUE_FILE);
	std::vector<uint8_t> file;
	if (file_str != params.end())
	{
		osd_file::error filerr = util::core_file::load(file_str->second->c_str(), file);
		if (filerr != osd_file::error::NONE)
			report_error(1, "Error reading metadata file (%s)", file_str->second->c_str());
	}

	// make sure we have one or the other
	if (text_str == params.end() && file_str == params.end())
		report_error(1, "Error: missing either --valuetext/-vt or --valuefile/-vf parameters");
	if (text_str != params.end() && file_str != params.end())
		report_error(1, "Error: both --valuetext/-vt or --valuefile/-vf parameters specified; only one permitted");

	// process no checksum
	uint8_t flags = CHD_MDFLAGS_CHECKSUM;
	if (params.find(OPTION_NO_CHECKSUM) != params.end())
		flags &= ~CHD_MDFLAGS_CHECKSUM;

	// print some info
	printf("Input file:   %s\n", params.find(OPTION_INPUT)->second->c_str());
	printf("Tag:          %c%c%c%c\n", (tag >> 24) & 0xff, (tag >> 16) & 0xff, (tag >> 8) & 0xff, tag & 0xff);
	printf("Index:        %d\n", index);
	if (text_str != params.end())
		printf("Text:         %s\n", text.c_str());
	else
		printf("Data:         %s (%d bytes)\n", file_str->second->c_str(), int(file.size()));

	// write the metadata
	chd_error err;
	if (text_str != params.end())
		err = input_chd.write_metadata(tag, index, text, flags);
	else
		err = input_chd.write_metadata(tag, index, file, flags);
	if (err != CHDERR_NONE)
		report_error(1, "Error adding metadata: %s", chd_file::error_string(err));
	else
		printf("Metadata added\n");
}


//-------------------------------------------------
//  do_del_metadata - remove metadata from a CHD
//-------------------------------------------------

static void do_del_metadata(parameters_map &params)
{
	// parse out input files
	chd_file input_parent_chd;
	chd_file input_chd;
	parse_input_chd_parameters(params, input_chd, input_parent_chd, true);

	// process tag
	chd_metadata_tag tag = CHD_MAKE_TAG('?','?','?','?');
	auto tag_str = params.find(OPTION_TAG);
	if (tag_str != params.end())
	{
		tag_str->second->append("    ");
		tag = CHD_MAKE_TAG((*tag_str->second)[0], (*tag_str->second)[1], (*tag_str->second)[2], (*tag_str->second)[3]);
	}

	// process index
	uint32_t index = 0;
	auto index_str = params.find(OPTION_INDEX);
	if (index_str != params.end())
		index = atoi(index_str->second->c_str());

	// print some info
	printf("Input file:   %s\n", params.find(OPTION_INPUT)->second->c_str());
	printf("Tag:          %c%c%c%c\n", (tag >> 24) & 0xff, (tag >> 16) & 0xff, (tag >> 8) & 0xff, tag & 0xff);
	printf("Index:        %d\n", index);

	// write the metadata
	chd_error err = input_chd.delete_metadata(tag, index);
	if (err != CHDERR_NONE)
		report_error(1, "Error removing metadata: %s", chd_file::error_string(err));
	else
		printf("Metadata removed\n");
}


//-------------------------------------------------
//  do_dump_metadata - dump metadata from a CHD
//-------------------------------------------------

static void do_dump_metadata(parameters_map &params)
{
	// parse out input files
	chd_file input_parent_chd;
	chd_file input_chd;
	parse_input_chd_parameters(params, input_chd, input_parent_chd);

	// verify output file doesn't exist
	auto output_file_str = params.find(OPTION_OUTPUT);
	if (output_file_str != params.end())
		check_existing_output_file(params, output_file_str->second->c_str());

	// process tag
	chd_metadata_tag tag = CHD_MAKE_TAG('?','?','?','?');
	auto tag_str = params.find(OPTION_TAG);
	if (tag_str != params.end())
	{
		tag_str->second->append("    ");
		tag = CHD_MAKE_TAG((*tag_str->second)[0], (*tag_str->second)[1], (*tag_str->second)[2], (*tag_str->second)[3]);
	}

	// process index
	uint32_t index = 0;
	auto index_str = params.find(OPTION_INDEX);
	if (index_str != params.end())
		index = atoi(index_str->second->c_str());

	// write the metadata
	std::vector<uint8_t> buffer;
	chd_error err = input_chd.read_metadata(tag, index, buffer);
	if (err != CHDERR_NONE)
		report_error(1, "Error reading metadata: %s", chd_file::error_string(err));

	// catch errors so we can close & delete the output file
	util::core_file::ptr output_file;
	try
	{
		// create the file
		if (output_file_str != params.end())
		{
			osd_file::error filerr = util::core_file::open(*output_file_str->second, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, output_file);
			if (filerr != osd_file::error::NONE)
				report_error(1, "Unable to open file (%s)", output_file_str->second->c_str());

			// output the metadata
			uint32_t count = output_file->write(&buffer[0], buffer.size());
			if (count != buffer.size())
				report_error(1, "Error writing file (%s)", output_file_str->second->c_str());
			output_file.reset();

			// provide some feedback
			printf("File (%s) written, %s bytes\n", output_file_str->second->c_str(), big_int_string(buffer.size()).c_str());
		}

		// flush to stdout
		else
		{
			fwrite(&buffer[0], 1, buffer.size(), stdout);
			fflush(stdout);
		}
	}
	catch (...)
	{
		// delete the output file
		output_file.reset();
		osd_file::remove(*output_file_str->second);
		throw;
	}
}


//-------------------------------------------------
//  do_dump_metadata - dump metadata from a CHD
//-------------------------------------------------

static void do_list_templates(parameters_map &params)
{
	printf("\n");
	printf("ID  Manufacturer  Model           Cylinders  Heads  Sectors  Sector Size  Total Size\n");
	printf("------------------------------------------------------------------------------------\n");

	for (int id = 0; id < ARRAY_LENGTH(s_hd_templates); id++)
	{
		printf("%2d  %-13s %-15s %9d  %5d  %7d  %11d  %7d MB\n",
			id,
			s_hd_templates[id].manufacturer,
			s_hd_templates[id].model,
			s_hd_templates[id].cylinders,
			s_hd_templates[id].heads,
			s_hd_templates[id].sectors,
			s_hd_templates[id].sector_size,
			(s_hd_templates[id].cylinders * s_hd_templates[id].heads * s_hd_templates[id].sectors * s_hd_templates[id].sector_size) / 1024 / 1024
		);
	}
}


//-------------------------------------------------
//  main - entry point
//-------------------------------------------------

int CLIB_DECL main(int argc, char *argv[])
{
	const std::vector<std::string> args = osd_get_command_line(argc, argv);

	// print the header
	extern const char build_version[];
	printf("chdman - MAME Compressed Hunks of Data (CHD) manager %s\n", build_version);

	// handle help specially
	if (args.size() < 2)
		return print_help(args[0]);
	int argnum = 1;
	std::string command = args[argnum++];
	bool help(command == COMMAND_HELP);
	if (help)
	{
		if (args.size() <= 2)
			return print_help(args[0]);
		command = args[argnum++];
	}

	// iterate over commands to find our match
	for (auto & s_command : s_commands)
		if (command == s_command.name)
		{
			const command_description &desc = s_command;

			// print help if that was requested
			if (help)
				return print_help(args[0], desc);

			// otherwise, verify the parameters
			parameters_map parameters;
			while (argnum < args.size())
			{
				// should be an option name
				const std::string &arg = args[argnum++];
				if (arg.empty() || (arg[0] != '-'))
					return print_help(args[0], desc, "Expected option, not parameter");

				// iterate over valid options
				int valid;
				for (valid = 0; valid < ARRAY_LENGTH(desc.valid_options); valid++)
				{
					// reduce to the option name
					const char *validname = desc.valid_options[valid];
					if (validname == nullptr)
						break;
					if (*validname == REQUIRED[0])
						validname++;

					// find the matching option description
					int optnum;
					for (optnum = 0; optnum < ARRAY_LENGTH(s_options); optnum++)
						if (strcmp(s_options[optnum].name, validname) == 0)
							break;
					assert(optnum != ARRAY_LENGTH(s_options));

					// do we match?
					const option_description &odesc = s_options[optnum];
					if ((arg[1] == '-' && strcmp(odesc.name, &arg[2]) == 0) ||
						(arg[1] != '-' && odesc.shortname != nullptr && strcmp(odesc.shortname, &arg[1]) == 0))
					{
						// if we need a parameter, consume it
						const char *param = "";
						if (odesc.parameter)
						{
							if (argnum >= args.size() || (!args[argnum].empty() && args[argnum][0] == '-'))
								return print_help(args[0], desc, "Option is missing parameter");
							param = args[argnum++].c_str();
						}

						// add to the map
						if (!parameters.insert(std::make_pair(odesc.name, new std::string(param))).second)
							return print_help(args[0], desc, "Multiple parameters of the same type specified");
						break;
					}
				}

				// if not valid, error
				if (valid == ARRAY_LENGTH(desc.valid_options))
					return print_help(args[0], desc, "Option not valid for this command");
			}

			// make sure we got all our required parameters
			for (int valid = 0; valid < ARRAY_LENGTH(desc.valid_options); valid++)
			{
				const char *validname = desc.valid_options[valid];
				if (validname == nullptr)
					break;
				if (*validname == REQUIRED[0] && parameters.find(++validname) == parameters.end())
					return print_help(args[0], desc, "Required parameters missing");
			}

			// all clear, run the command
			try
			{
				(*s_command.handler)(parameters);
				return 0;
			}
			catch (chd_error &err)
			{
				fprintf(stderr, "CHD error occurred (main): %s\n", chd_file::error_string(err));
				return 1;
			}
			catch (fatal_error &err)
			{
				fprintf(stderr, "Fatal error occurred: %d\n", err.error());
				return err.error();
			}
			catch (std::exception& ex)
			{
				fprintf(stderr, "Unhandled exception: %s\n", ex.what());
				return 1;
			}
		}

	// print generic help if nothing found
	return print_help(args[0]);
}
