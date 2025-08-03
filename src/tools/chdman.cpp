// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    CHD compression frontend

****************************************************************************/
#include <stdio.h> // must be stdio.h and here otherwise issues with I64FMT in MINGW

// lib/util
#include "avhuff.h"
#include "aviio.h"
#include "bitmap.h"
#include "cdrom.h"
#include "corefile.h"
#include "coretmpl.h"
#include "hashing.h"
#include "md5.h"
#include "multibyte.h"
#include "osdcore.h"
#include "path.h"
#include "strformat.h"
#include "vbiparse.h"

#include <array>
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <limits>
#include <memory>
#include <new>
#include <optional>
#include <regex>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

using util::string_format;



//**************************************************************************
//  CONSTANTS & DEFINES
//**************************************************************************
// MINGW has adopted the MSVC formatting for 64-bit ints as of GCC 4.4 and deprecated it as of GCC 9.3
#if defined(_WIN32) && defined(__GNUC__) && ((__GNUC__ < 9) || ((__GNUC__ == 9) && (__GNUC_MINOR__ < 3)))
#define I64FMT   "I64"
#elif !defined(__APPLE__) && defined(__LP64__)
#define I64FMT   "l"
#else
#define I64FMT   "ll"
#endif

// hunk size limits
constexpr uint32_t HUNK_SIZE_MIN = 16;
constexpr uint32_t HUNK_SIZE_MAX = 1024 * 1024;

// default hard disk sector size
constexpr uint32_t IDE_SECTOR_SIZE = 512;

// temporary input buffer size
constexpr uint32_t TEMP_BUFFER_SIZE = 32 * 1024 * 1024;

// modes
constexpr int MODE_NORMAL = 0;
constexpr int MODE_CUEBIN = 1;
constexpr int MODE_GDI = 2;

// osd printf verbosity
constexpr bool OSD_PRINTF_VERBOSE = false;

// command modifier
#define REQUIRED "~"

// command strings
#define COMMAND_HELP "help"
#define COMMAND_INFO "info"
#define COMMAND_VERIFY "verify"
#define COMMAND_CREATE_RAW "createraw"
#define COMMAND_CREATE_HD "createhd"
#define COMMAND_CREATE_CD "createcd"
#define COMMAND_CREATE_DVD "createdvd"
#define COMMAND_CREATE_LD "createld"
#define COMMAND_EXTRACT_RAW "extractraw"
#define COMMAND_EXTRACT_HD "extracthd"
#define COMMAND_EXTRACT_CD "extractcd"
#define COMMAND_EXTRACT_DVD "extractdvd"
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
#define OPTION_OUTPUT_SPLITBIN "splitbin"
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

template <typename Format, typename... Params>
[[noreturn]] static void report_error(int error, Format &&fmt, Params &&...args);

static void do_info(parameters_map &params);
static void do_verify(parameters_map &params);
static void do_create_raw(parameters_map &params);
static void do_create_hd(parameters_map &params);
static void do_create_cd(parameters_map &params);
static void do_create_dvd(parameters_map &params);
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

// Allow chdman to show osd_printf_X messages
class chdman_osd_output : public osd_output
{
public:
	chdman_osd_output() {
		osd_output::push(this);
	}

	~chdman_osd_output() {
		osd_output::pop(this);
	}

	void output_callback(osd_output_channel channel, const util::format_argument_pack<char> &args);
};

void chdman_osd_output::output_callback(osd_output_channel channel, const util::format_argument_pack<char> &args)
{
	switch (channel)
	{
	case OSD_OUTPUT_CHANNEL_ERROR:
	case OSD_OUTPUT_CHANNEL_WARNING:
		util::stream_format(std::cerr, args);
		break;
	case OSD_OUTPUT_CHANNEL_INFO:
	case OSD_OUTPUT_CHANNEL_LOG:
		util::stream_format(std::cout, args);
		break;
	case OSD_OUTPUT_CHANNEL_VERBOSE:
		if (OSD_PRINTF_VERBOSE) util::stream_format(std::cout, args);
		break;
	case OSD_OUTPUT_CHANNEL_DEBUG:
#ifdef MAME_DEBUG
		util::stream_format(std::cout, args);
#endif
		break;
	default:
		break;
	}
}

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
	fatal_error(int error) : m_error(error) { }

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
	chd_rawfile_compressor(util::random_read &file, std::uint64_t offset = 0, std::uint64_t maxoffset = std::numeric_limits<std::uint64_t>::max())
		: m_file(file)
		, m_offset(offset)
	{
		// TODO: what to do about error getting file size?
		std::uint64_t filelen;
		if (!file.length(filelen))
			m_maxoffset = (std::min)(maxoffset, filelen);
		else
			m_maxoffset = maxoffset;
	}

	// read interface
	virtual std::uint32_t read_data(void *dest, std::uint64_t offset, std::uint32_t length) override
	{
		offset += m_offset;
		if (offset >= m_maxoffset)
			return 0;
		if (offset + length > m_maxoffset)
			length = m_maxoffset - offset;
		if (m_file.seek(offset, SEEK_SET)) // FIXME: better error reporting?
			return 0;
		auto const [err, actual] = read(m_file, dest, length); // FIXME: check for error return
		return actual;
	}

private:
	// internal state
	util::random_read & m_file;
	std::uint64_t       m_offset;
	std::uint64_t       m_maxoffset;
};


// ======================> chd_chdfile_compressor

class chd_chdfile_compressor : public chd_file_compressor
{
public:
	// construction/destruction
	chd_chdfile_compressor(chd_file &file, uint64_t offset = 0, uint64_t maxoffset = ~0)
		: m_toc(nullptr)
		, m_file(file)
		, m_offset(offset)
		, m_maxoffset(std::min(maxoffset, file.logical_bytes()))
	{
	}

	// read interface
	virtual uint32_t read_data(void *dest, uint64_t offset, uint32_t length)
	{
		offset += m_offset;
		if (offset >= m_maxoffset)
			return 0;
		if (offset + length > m_maxoffset)
			length = m_maxoffset - offset;
		std::error_condition err = m_file.read_bytes(offset, dest, length);
		if (err)
			throw err;

		// if we have TOC - detect audio sectors and swap data
		if (m_toc)
		{
			assert(offset % cdrom_file::FRAME_SIZE == 0);
			assert(length % cdrom_file::FRAME_SIZE == 0);

			int startlba = offset / cdrom_file::FRAME_SIZE;
			int lenlba = length / cdrom_file::FRAME_SIZE;
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
				if (m_toc->tracks[tracknum].trktype != cdrom_file::CD_TRACK_AUDIO)
					continue;
				// byteswap if yes
				int dataoffset = chdlba * cdrom_file::FRAME_SIZE;
				for (uint32_t swapindex = dataoffset; swapindex < (dataoffset + cdrom_file::MAX_SECTOR_DATA); swapindex += 2)
				{
					uint8_t temp = _dest[swapindex];
					_dest[swapindex] = _dest[swapindex + 1];
					_dest[swapindex + 1] = temp;
				}
			}
		}

		return length;
	}

const cdrom_file::toc *   m_toc;

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
	chd_cd_compressor(cdrom_file::toc &toc, cdrom_file::track_input_info &info)
		: m_file()
		, m_toc(toc)
		, m_info(info)
	{
	}

	~chd_cd_compressor()
	{
	}

	// read interface
	virtual uint32_t read_data(void *_dest, uint64_t offset, uint32_t length)
	{
		// verify assumptions made below
		assert(offset % cdrom_file::FRAME_SIZE == 0);
		assert(length % cdrom_file::FRAME_SIZE == 0);

		// initialize destination to 0 so that unused areas are filled
		uint8_t *dest = reinterpret_cast<uint8_t *>(_dest);
		memset(dest, 0, length);

		// find out which track we're starting in
		uint64_t startoffs = 0;
		uint32_t length_remaining = length;
		for (int tracknum = 0; tracknum < m_toc.numtrks; tracknum++)
		{
			const cdrom_file::track_info &trackinfo = m_toc.tracks[tracknum];
			uint64_t endoffs = startoffs + (uint64_t)(trackinfo.frames + trackinfo.extraframes) * cdrom_file::FRAME_SIZE;

			if (offset >= startoffs && offset < endoffs)
			{
				// if we don't already have this file open, open it now
				if (!m_file || m_lastfile.compare(m_info.track[tracknum].fname)!=0)
				{
					m_file.reset();
					m_lastfile = m_info.track[tracknum].fname;
					std::error_condition const filerr = util::core_file::open(m_lastfile, OPEN_FLAG_READ, m_file);
					if (filerr)
						report_error(1, "Error opening input file (%s): %s", m_lastfile, filerr.message());
				}

				// iterate over frames
				uint64_t bytesperframe = trackinfo.datasize + trackinfo.subsize;
				uint64_t src_track_start = m_info.track[tracknum].offset;
				uint64_t src_track_end = src_track_start + bytesperframe * (uint64_t)trackinfo.frames;
				uint64_t split_track_start = src_track_end - ((uint64_t)trackinfo.splitframes * bytesperframe);
				uint64_t pad_track_start = split_track_start - ((uint64_t)trackinfo.padframes * bytesperframe);

				// dont split when split-bin read not required
				if ((uint64_t)trackinfo.splitframes == 0L)
					split_track_start = UINT64_MAX;

				while (length_remaining != 0 && offset < endoffs)
				{
					// determine start of current frame
					uint64_t src_frame_start = src_track_start + ((offset - startoffs) / cdrom_file::FRAME_SIZE) * bytesperframe;

					// auto-advance next track for split-bin read
					if (src_frame_start >= split_track_start && src_frame_start < src_track_end && m_lastfile.compare(m_info.track[tracknum+1].fname)!=0)
					{
						m_file.reset();
						m_lastfile = m_info.track[tracknum+1].fname;
						std::error_condition const filerr = util::core_file::open(m_lastfile, OPEN_FLAG_READ, m_file);
						if (filerr)
							report_error(1, "Error opening input file (%s): %s", m_lastfile, filerr.message());
					}

					if (src_frame_start < src_track_end)
					{
						// read it in, or pad if we're into the padframes
						if (src_frame_start >= pad_track_start && src_frame_start < split_track_start)
						{
							memset(dest, 0, bytesperframe);
						}
						else
						{
							std::error_condition err = m_file->seek(
									(src_frame_start >= split_track_start)
										? src_frame_start - split_track_start
										: src_frame_start,
									SEEK_SET);
							std::size_t count = 0;
							if (!err)
								std::tie(err, count) = read(*m_file, dest, bytesperframe);
							if (err || (count != bytesperframe))
								report_error(1, "Error reading input file (%s)'", m_lastfile);
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
					offset += cdrom_file::FRAME_SIZE;
					dest += cdrom_file::FRAME_SIZE;
					length_remaining -= cdrom_file::FRAME_SIZE;
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
	std::string                   m_lastfile;
	util::core_file::ptr          m_file;
	cdrom_file::toc &             m_toc;
	cdrom_file::track_input_info &m_info;
};


// ======================> chd_avi_compressor

class chd_avi_compressor : public chd_file_compressor
{
public:
	// construction/destruction
	chd_avi_compressor(avi_file &file, avi_info &info, uint32_t first_frame, uint32_t num_frames)
		: m_file(file)
		, m_info(info)
		, m_bitmap(info.width, info.height * (info.interlaced ? 2 : 1))
		, m_start_frame(first_frame)
		, m_frame_count(num_frames)
		, m_ldframedata(num_frames * VBI_PACKED_BYTES)
		, m_rawdata(info.bytes_per_frame)
	{
	}

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
		{
			if (framenum < m_frame_count)
			{
				// determine effective frame number and first/last samples
				int32_t effframe = m_start_frame + framenum;
				uint32_t first_sample = (uint64_t(m_info.rate) * uint64_t(effframe) * uint64_t(1000000) + m_info.fps_times_1million - 1) / uint64_t(m_info.fps_times_1million);
				uint32_t samples = (uint64_t(m_info.rate) * uint64_t(effframe + 1) * uint64_t(1000000) + m_info.fps_times_1million - 1) / uint64_t(m_info.fps_times_1million) - first_sample;

				// loop over channels and read the samples
				int channels = unsigned(std::min<std::size_t>(m_info.channels, std::size(m_audio)));
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
		}

		return length;
	}

private:
	// internal state
	avi_file &                  m_file;
	avi_info &                  m_info;
	bitmap_yuy16                m_bitmap;
	uint32_t                    m_start_frame;
	uint32_t                    m_frame_count;
	std::vector<int16_t>        m_audio[8];
	std::vector<uint8_t>        m_ldframedata;
	std::vector<uint8_t>        m_rawdata;
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// timing
static clock_t lastprogress = 0;


// default compressors
static const std::array<chd_codec_type, 4> s_no_compression = { CHD_CODEC_NONE, CHD_CODEC_NONE, CHD_CODEC_NONE, CHD_CODEC_NONE };
static const std::array<chd_codec_type, 4> s_default_raw_compression = { CHD_CODEC_LZMA, CHD_CODEC_ZLIB, CHD_CODEC_HUFFMAN, CHD_CODEC_FLAC };
static const std::array<chd_codec_type, 4> s_default_hd_compression = { CHD_CODEC_LZMA, CHD_CODEC_ZLIB, CHD_CODEC_HUFFMAN, CHD_CODEC_FLAC };
static const std::array<chd_codec_type, 4> s_default_cd_compression = { CHD_CODEC_CD_LZMA, CHD_CODEC_CD_ZLIB, CHD_CODEC_CD_FLAC };
static const std::array<chd_codec_type, 4> s_default_ld_compression = { CHD_CODEC_AVHUFF };


// descriptions for each option
static const option_description s_options[] =
{
	{ OPTION_INPUT,                 "i",    true, " <filename>: input file name" },
	{ OPTION_INPUT_PARENT,          "ip",   true, " <filename>: parent file name for input CHD" },
	{ OPTION_OUTPUT,                "o",    true, " <filename>: output file name" },
	{ OPTION_OUTPUT_BIN,            "ob",   true, " <filename>: output file name for binary data" },
	{ OPTION_OUTPUT_SPLITBIN,       "sb",   false, ": output one binary file per track" },
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
	{ OPTION_CHS,                   "chs",  true, " <cylinders,heads,sectors>: specifies CHS geometry directly" },
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
			OPTION_INPUT_PARENT,
			OPTION_FIX
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
			OPTION_HUNK_SIZE,
			OPTION_UNIT_SIZE,
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
	{ COMMAND_CREATE_DVD, do_create_dvd, ": create a DVD CHD from the input file",
		{
			REQUIRED OPTION_OUTPUT,
			OPTION_OUTPUT_PARENT,
			OPTION_OUTPUT_FORCE,
			REQUIRED OPTION_INPUT,
			OPTION_INPUT_START_BYTE,
			OPTION_INPUT_START_HUNK,
			OPTION_INPUT_LENGTH_BYTES,
			OPTION_INPUT_LENGTH_HUNKS,
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
			OPTION_OUTPUT_SPLITBIN,
			OPTION_OUTPUT_FORCE,
			REQUIRED OPTION_INPUT,
			OPTION_INPUT_PARENT,
		}
	},

	{ COMMAND_EXTRACT_DVD, do_extract_raw, ": extract DVD file from a CHD input file",
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
	{ "Conner",     "CFA170A",    332, 16, 63, 512 }, //  163 MB
	{ "Rodime",     "R0201",      321,  2, 16, 512 }, //    5 MB
	{ "Rodime",     "R0202",      321,  4, 16, 512 }, //   10 MB
	{ "Rodime",     "R0203",      321,  6, 16, 512 }, //   15 MB
	{ "Rodime",     "R0204",      321,  8, 16, 512 }, //   20 MB
	{ "Seagate",    "ST-213",     615,  2, 17, 512 }, //   10 MB
	{ "Seagate",    "ST-225",     615,  4, 17, 512 }, //   20 MB
	{ "Seagate",    "ST-251",     820,  6, 17, 512 }, //   40 MB
	{ "Seagate",    "ST-3600N",  1877,  7, 76, 512 }, //  525 MB
	{ "Maxtor",     "LXT-213S",  1314,  7, 53, 512 }, //  200 MB
	{ "Maxtor",     "LXT-340S",  1574,  7, 70, 512 }, //  340 MB
	{ "Maxtor",     "MXT-540SL", 2466,  7, 87, 512 }, //  540 MB
	{ "Micropolis", "1528",      2094, 15, 83, 512 }, // 1342 MB
};



//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  report_error - report an error
//-------------------------------------------------

template <typename Format, typename... Params>
static void report_error(int error, Format &&fmt, Params &&...args)
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

template <typename Format, typename... Params>
static void progress(bool forceit, Format &&fmt, Params &&...args)
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
	if (error)
		util::stream_format(std::cerr, "Error: %s\n\n", error);

	// print a summary of each command
	util::stream_format(std::cout, "Usage:\n");
	for (auto & desc : s_commands)
	{
		util::stream_format(std::cout, "   %s %s%s\n", argv0, desc.name, desc.description);
	}
	util::stream_format(std::cout, "\nFor help with any command, run:\n");
	util::stream_format(std::cout, "   %s %s <command>\n", argv0, COMMAND_HELP);
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
		util::stream_format(std::cerr, "Error: %s\n\n", error);

	// print usage for this command
	util::stream_format(std::cout, "Usage:\n");
	util::stream_format(std::cout, "   %s %s [options], where valid options are:\n", argv0, desc.name);
	for (int valid = 0; valid < std::size(desc.valid_options); valid++)
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
				util::stream_format(std::cout, "      --%s", odesc.name);
				if (odesc.shortname != nullptr)
					util::stream_format(std::cout, ", -%s", odesc.shortname);
				util::stream_format(std::cout, "%s%s\n", odesc.description, required ? " (required)" : "");
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

static void guess_chs(
		const std::string *filename,
		uint64_t filesize,
		int sectorsize,
		uint32_t &cylinders,
		uint32_t &heads,
		uint32_t &sectors,
		uint32_t &bps)
{
	// if this is a direct physical drive read, handle it specially
	if (filename && osd_get_physical_drive_geometry(filename->c_str(), &cylinders, &heads, &sectors, &bps))
		return;

	// if we have no length to work with, we can't guess
	if (filesize == 0)
		report_error(1, "Can't guess CHS values because there is no input file");

	// now find a valid value
	for (uint32_t totalsectors = filesize / sectorsize; ; totalsectors++)
	{
		for (uint32_t cursectors = 63; cursectors > 1; cursectors--)
		{
			if (totalsectors % cursectors == 0)
			{
				uint32_t totalheads = totalsectors / cursectors;
				for (uint32_t curheads = 16; curheads > 1; curheads--)
				{
					if (totalheads % curheads == 0)
					{
						cylinders = totalheads / curheads;
						heads = curheads;
						sectors = cursectors;
						return;
					}
				}
			}
		}
	}
}


//-------------------------------------------------
//  parse_input_parent_chd - parse the parent CHD
//-------------------------------------------------

static void parse_input_parent_chd(const parameters_map &params, chd_file &input_parent_chd)
{
	// process input parent file
	auto input_chd_parent_str = params.find(OPTION_INPUT_PARENT);
	if (input_chd_parent_str != params.end())
	{
		std::error_condition err = input_parent_chd.open(*input_chd_parent_str->second);
		if (err)
			report_error(1, "Error opening parent CHD file (%s): %s", *input_chd_parent_str->second, err.message());
	}
}


//-------------------------------------------------
//  parse_input_chd_parameters - parse the
//  standard set of input CHD parameters
//-------------------------------------------------

static void parse_input_chd_parameters(const parameters_map &params, chd_file &input_chd, chd_file &input_parent_chd, bool writeable = false)
{
	// process input parent file
	parse_input_parent_chd(params, input_parent_chd);

	// process input file
	auto input_chd_str = params.find(OPTION_INPUT);
	if (input_chd_str != params.end())
	{
		std::error_condition err = input_chd.open(*input_chd_str->second, writeable, input_parent_chd.opened() ? &input_parent_chd : nullptr);
		if (err)
			report_error(1, "Error opening CHD file (%s): %s", *input_chd_str->second, err.message());
	}
}


//-------------------------------------------------
//  parse_input_start_end - parse input start/end
//  parameters in a standard way
//-------------------------------------------------

static std::pair<uint64_t, uint64_t> parse_input_start_end(
		const parameters_map &params,
		uint64_t logical_size,
		uint32_t hunkbytes,
		uint32_t framebytes)
{
	// process input start
	const auto input_start_byte_str = params.find(OPTION_INPUT_START_BYTE);
	const auto input_start_hunk_str = params.find(OPTION_INPUT_START_HUNK);
	const auto input_start_frame_str = params.find(OPTION_INPUT_START_FRAME);
	uint64_t input_start = 0;
	if (input_start_byte_str != params.end())
	{
		if (input_start_hunk_str != params.end())
			report_error(1, "Start offset cannot be specified in both bytes and hunks");
		if (input_start_frame_str != params.end())
			report_error(1, "Start offset cannot be specified in both bytes and frames");
		input_start = parse_number(input_start_byte_str->second->c_str());
	}
	else if (input_start_hunk_str != params.end())
	{
		if (input_start_frame_str != params.end())
			report_error(1, "Start offset cannot be specified in both hunks and frames");
		input_start = parse_number(input_start_hunk_str->second->c_str()) * hunkbytes;
	}
	else if (input_start_frame_str != params.end())
	{
		input_start = parse_number(input_start_frame_str->second->c_str()) * framebytes;
	}

	// process input length
	const auto input_length_bytes_str = params.find(OPTION_INPUT_LENGTH_BYTES);
	auto input_length_hunks_str = params.find(OPTION_INPUT_LENGTH_HUNKS);
	auto input_length_frames_str = params.find(OPTION_INPUT_LENGTH_FRAMES);
	std::optional<uint64_t> input_length;
	if (input_length_bytes_str != params.end())
	{
		if (input_length_hunks_str != params.end())
			report_error(1, "Length cannot be specified in both bytes and hunks");
		if (input_length_frames_str != params.end())
			report_error(1, "Length cannot be specified in both bytes and frames");
		input_length = parse_number(input_length_bytes_str->second->c_str());
	}
	else if (input_length_hunks_str != params.end())
	{
		if (input_length_frames_str != params.end())
			report_error(1, "Length cannot be specified in both hunks and frames");
		input_length = parse_number(input_length_hunks_str->second->c_str()) * hunkbytes;
	}
	else if (input_length_frames_str != params.end())
	{
		input_length = parse_number(input_length_frames_str->second->c_str()) * framebytes;
	}

	// check that offsets are within input
	if (input_start >= logical_size)
		report_error(1, "Input start offset is beyond end of input");
	if (input_length && ((input_start + *input_length) > logical_size))
		report_error(1, "Input length is larger than available input from start offset");
	return std::make_pair(input_start, input_length ? (input_start + *input_length) : logical_size);
}

static std::tuple<uint64_t, uint64_t, uint64_t> parse_input_start_end(
		const parameters_map &params,
		util::random_read &input_file,
		uint32_t hunkbytes,
		uint32_t framebytes)
{
	uint64_t input_size = 0;
	const std::error_condition err = input_file.length(input_size);
	if (err)
		report_error(1, "Error getting size of input file: %s", err.message());
	const auto [input_start, input_end] = parse_input_start_end(params, input_size, hunkbytes, framebytes);
	return std::make_tuple(input_size, input_start, input_end);
}


//-------------------------------------------------
//  check_existing_output_file - see if an output
//  file already exists, and error if it does,
//  unless --force is specified
//-------------------------------------------------

static void check_existing_output_file(const parameters_map &params, std::string_view filename)
{
	if (params.find(OPTION_OUTPUT_FORCE) == params.end())
	{
		util::core_file::ptr file;
		std::error_condition const filerr = util::core_file::open(filename, OPEN_FLAG_READ, file);
		if (!filerr)
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

static const std::string *parse_output_chd_parameters(const parameters_map &params, chd_file &output_parent_chd)
{
	// process output parent file
	const auto output_chd_parent_str = params.find(OPTION_OUTPUT_PARENT);
	if (output_chd_parent_str != params.end())
	{
		std::error_condition err = output_parent_chd.open(*output_chd_parent_str->second);
		if (err)
			report_error(1, "Error opening parent CHD file (%s): %s", *output_chd_parent_str->second, err.message());
	}

	// process output file
	const auto output_chd_str = params.find(OPTION_OUTPUT);
	if (output_chd_str == params.end())
		return nullptr;
	check_existing_output_file(params, *output_chd_str->second);
	return output_chd_str->second;
}


//-------------------------------------------------
//  parse_hunk_size - parse the hunk_size
//  parameter in a standard way
//-------------------------------------------------

static uint32_t parse_hunk_size(
		const parameters_map &params,
		const chd_file &output_parent,
		uint32_t required_granularity,
		uint32_t default_size)
{
	const auto hunk_size_str = params.find(OPTION_HUNK_SIZE);
	uint32_t hunk_size = default_size;
	if (hunk_size_str != params.end())
	{
		hunk_size = parse_number(hunk_size_str->second->c_str());
		if (output_parent.opened() && (output_parent.hunk_bytes() != hunk_size))
			report_error(1, "Specified hunk size %u bytes does not match output parent CHD hunk size %u bytes", hunk_size, output_parent.hunk_bytes());
		if (hunk_size < HUNK_SIZE_MIN)
			report_error(1, "Invalid hunk size (minimum %u)", HUNK_SIZE_MIN);
		if (hunk_size > HUNK_SIZE_MAX)
			report_error(1, "Invalid hunk size (maximum %u)", HUNK_SIZE_MAX);
	}
	else if (output_parent.opened())
	{
		hunk_size = output_parent.hunk_bytes();
	}

	if (hunk_size % required_granularity)
		report_error(1, "Hunk size %u bytes is not a whole multiple of %u", hunk_size, required_granularity);
	return hunk_size;
}


//-------------------------------------------------
//  parse_compression - parse a standard
//  compression parameter string
//-------------------------------------------------

static void parse_compression(const parameters_map &params, const std::array<chd_codec_type, 4> &defaults, const chd_file &output_parent, chd_codec_type compression[4])
{
	// TODO: should we default to the same compression as the output parent?
	std::copy(std::begin(defaults), std::end(defaults), compression);

	// see if anything was specified
	const auto compression_str = params.find(OPTION_COMPRESSION);
	if (compression_str == params.end())
		return;

	// special case: 'none'
	if (*compression_str->second == "none")
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
			report_error(1, "Invalid compressor '%s' specified", name);
		chd_codec_type type = CHD_MAKE_TAG(name[0], name[1], name[2], name[3]);
		if (!chd_codec_list::codec_exists(type))
			report_error(1, "Invalid compressor '%s' specified", name);
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
//  open_input_file - open input file if specified
//-------------------------------------------------

static std::pair<util::core_file::ptr, const std::string *> open_input_file(const parameters_map &params)
{
	const auto path = params.find(OPTION_INPUT);
	if (path == params.end())
		return std::make_pair(nullptr, nullptr);

	util::core_file::ptr file;
	const std::error_condition err = util::core_file::open(*path->second, OPEN_FLAG_READ, file);
	if (err)
		report_error(1, "Unable to open input file (%s): %s", *path->second, err.message());

	return std::make_pair(std::move(file), path->second);
}


//-------------------------------------------------
//  create_output_chd - open output CHD with or
//  without parent CHD file
//-------------------------------------------------

static void create_output_chd(
		chd_file_compressor &compressor,
		std::string_view path,
		uint64_t logical_size,
		uint32_t hunk_size,
		uint32_t unit_size,
		const chd_codec_type (&compression)[4],
		chd_file &parent)
{
	std::error_condition err;
	if (parent.opened())
		err = compressor.create(path, logical_size, hunk_size, compression, parent);
	else
		err = compressor.create(path, logical_size, hunk_size, unit_size, compression);
	if (err)
		report_error(1, "Error creating CHD file (%s): %s", path, err.message());
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
	std::error_condition err;
	while ((err = chd.compress_continue(complete, ratio)) == chd_file::error::WALKING_PARENT || err == chd_file::error::COMPRESSING)
		if (err == chd_file::error::WALKING_PARENT)
			progress(false, "Examining parent, %.1f%% complete...  \r", 100.0 * complete);
		else
			progress(false, "Compressing, %.1f%% complete... (ratio=%.1f%%)  \r", 100.0 * complete, 100.0 * ratio);

	// handle errors
	if (err)
		report_error(1, "Error during compression: %-40s", err.message());

	// final progress update
	progress(true, "Compression complete ... final ratio = %.1f%%            \n", 100.0 * ratio);
}


//-------------------------------------------------
//  output_track_metadata - output track metadata
//  to a CUE file
//-------------------------------------------------

void output_track_metadata(int mode, util::core_file &file, int tracknum, const cdrom_file::track_info &info, const std::string &filename, uint32_t frameoffs, uint64_t outputoffs)
{
	if (mode == MODE_GDI)
	{
		const int tracktype = info.trktype == cdrom_file::CD_TRACK_AUDIO ? 0 : 4;
		const bool needquote = filename.find(' ') != std::string::npos;
		const char *const quotestr = needquote ? "\"" : "";
		file.printf("%d %d %d %d %s%s%s %d\n", tracknum+1, frameoffs, tracktype, info.datasize, quotestr, filename, quotestr, outputoffs);
	}
	else if (mode == MODE_CUEBIN)
	{
		// specify a new file when writing to the beginning of a file
		if (outputoffs == 0)
			file.printf("FILE \"%s\" BINARY\n", filename);

		// determine submode
		std::string tempstr;
		switch (info.trktype)
		{
			case cdrom_file::CD_TRACK_MODE1:
			case cdrom_file::CD_TRACK_MODE1_RAW:
				tempstr = string_format("MODE1/%04d", info.datasize);
				break;

			case cdrom_file::CD_TRACK_MODE2:
			case cdrom_file::CD_TRACK_MODE2_FORM1:
			case cdrom_file::CD_TRACK_MODE2_FORM2:
			case cdrom_file::CD_TRACK_MODE2_FORM_MIX:
			case cdrom_file::CD_TRACK_MODE2_RAW:
				tempstr = string_format("MODE2/%04d", info.datasize);
				break;

			case cdrom_file::CD_TRACK_AUDIO:
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
		file.printf("// Track %d\n", tracknum + 1);

		// write out the track type
		std::string modesubmode;
		if (info.subtype != cdrom_file::CD_SUB_NONE)
			modesubmode = string_format("%s %s", cdrom_file::get_type_string(info.trktype), cdrom_file::get_subtype_string(info.subtype));
		else
			modesubmode = string_format("%s", cdrom_file::get_type_string(info.trktype));
		file.printf("TRACK %s\n", modesubmode);

		// write out the attributes
		file.printf("NO COPY\n");
		if (info.trktype == cdrom_file::CD_TRACK_AUDIO)
		{
			file.printf("NO PRE_EMPHASIS\n");
			file.printf("TWO_CHANNEL_AUDIO\n");
		}

		// output pregap
		if (info.pregap > 0)
			file.printf("ZERO %s %s\n", modesubmode, msf_string_from_frames(info.pregap));

		if (outputoffs == 0)
			file.printf("DATAFILE \"%s\" %s // length in bytes: %d\n", filename, msf_string_from_frames(info.frames), info.frames * (info.datasize + info.subsize));
		else
			file.printf("DATAFILE \"%s\" #%d %s // length in bytes: %d\n", filename, uint32_t(outputoffs), msf_string_from_frames(info.frames), info.frames * (info.datasize + info.subsize));

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
	util::stream_format(std::cout, "Input file:   %s\n", *params.find(OPTION_INPUT)->second);
	util::stream_format(std::cout, "File Version: %d\n", input_chd.version());
	if (input_chd.version() < 3)
		report_error(1, "Unsupported version (%d); use an older chdman to upgrade to version 3 or later", input_chd.version());

	// output cmpression and size information
	chd_codec_type compression[4] = { input_chd.compression(0), input_chd.compression(1), input_chd.compression(2), input_chd.compression(3) };
	uint64_t filesize = 0;
	input_chd.file().length(filesize);
	util::stream_format(std::cout, "Logical size: %s bytes\n", big_int_string(input_chd.logical_bytes()));
	util::stream_format(std::cout, "Hunk Size:    %s bytes\n", big_int_string(input_chd.hunk_bytes()));
	util::stream_format(std::cout, "Total Hunks:  %s\n", big_int_string(input_chd.hunk_count()));
	util::stream_format(std::cout, "Unit Size:    %s bytes\n", big_int_string(input_chd.unit_bytes()));
	util::stream_format(std::cout, "Total Units:  %s\n", big_int_string(input_chd.unit_count()));
	util::stream_format(std::cout, "Compression:  %s\n", compression_string(compression));
	util::stream_format(std::cout, "CHD size:     %s bytes\n", big_int_string(filesize));
	if (compression[0] != CHD_CODEC_NONE)
		util::stream_format(std::cout, "Ratio:        %.1f%%\n", 100.0 * double(filesize) / double(input_chd.logical_bytes()));

	// add SHA-1 output
	util::sha1_t overall = input_chd.sha1();
	if (overall != util::sha1_t::null)
	{
		util::stream_format(std::cout, "SHA1:         %s\n", overall.as_string());
		if (input_chd.version() >= 4)
			util::stream_format(std::cout, "Data SHA1:    %s\n", input_chd.raw_sha1().as_string());
	}
	util::sha1_t parent = input_chd.parent_sha1();
	if (parent != util::sha1_t::null)
		util::stream_format(std::cout, "Parent SHA1:  %s\n", parent.as_string());

	// print out metadata
	std::vector<uint8_t> buffer;
	std::vector<metadata_index_info> info;
	for (int index = 0; ; index++)
	{
		// get the indexed metadata item; stop when we hit an error
		chd_metadata_tag metatag;
		uint8_t metaflags;
		std::error_condition err = input_chd.read_metadata(CHDMETATAG_WILDCARD, index, buffer, metatag, metaflags);
		if (err)
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
			util::stream_format(std::cout, "Metadata:     Tag='%c%c%c%c'  Index=%d  Length=%d bytes\n", (metatag >> 24) & 0xff, (metatag >> 16) & 0xff, (metatag >> 8) & 0xff, metatag & 0xff, metaindex, buffer.size());
		else
			util::stream_format(std::cout, "Metadata:     Tag=%08x  Index=%d  Length=%d bytes\n", metatag, metaindex, buffer.size());
		util::stream_format(std::cout, "              ");

		uint32_t count = buffer.size();
		// limit output to 60 characters of metadata if not verbose
		if (!verbose)
			count = std::min(60U, count);
		for (int chnum = 0; chnum < count; chnum++)
			util::stream_format(std::cout, "%c", isprint(uint8_t(buffer[chnum])) ? buffer[chnum] : '.');
		util::stream_format(std::cout, "\n");
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
			std::error_condition err = input_chd.hunk_info(hunknum, codec, compbytes);
			if (err)
				report_error(1, "Error getting info on hunk %d: %s", hunknum, err.message());

			// decode into our data
			if (codec > CHD_CODEC_MINI)
				for (int comptype = 0; comptype < 4; comptype++)
					if (codec == input_chd.compression(comptype))
					{
						codec = CHD_CODEC_MINI + 1 + comptype;
						break;
					}
			if (codec >= std::size(compression_types))
				codec = std::size(compression_types) - 1;

			// count stats
			compression_types[codec]++;
		}

		// output the stats
		util::stream_format(std::cout, "\n");
		util::stream_format(std::cout, "     Hunks  Percent  Name\n");
		util::stream_format(std::cout, "----------  -------  ------------------------------------\n");
		for (int comptype = 0; comptype < std::size(compression_types); comptype++)
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
				util::stream_format(std::cout, "%10s   %5.1f%%  %-40s\n",
						big_int_string(compression_types[comptype]),
						100.0 * double(compression_types[comptype]) / double(input_chd.hunk_count()),
						name);
			}
	}
}


//-------------------------------------------------
//  do_verify - validate the SHA-1 on a CHD
//-------------------------------------------------

static void do_verify(parameters_map &params)
{
	bool fix_sha1 = params.find(OPTION_FIX) != params.end();
	// parse out input files
	chd_file input_parent_chd;
	chd_file input_chd;
	parse_input_parent_chd(params, input_parent_chd);

	// process input file
	auto input_chd_str = params.find(OPTION_INPUT);
	if (input_chd_str != params.end())
	{
		const uint32_t openflags = fix_sha1 ? (OPEN_FLAG_READ | OPEN_FLAG_WRITE) : OPEN_FLAG_READ;
		util::core_file::ptr file;
		std::error_condition err = util::core_file::open(*input_chd_str->second, openflags, file);
		if (!err)
			err = input_chd.open(std::move(file), false, input_parent_chd.opened() ? &input_parent_chd : nullptr);
		if (err)
			report_error(1, "Error opening CHD file (%s): %s", *input_chd_str->second, err.message());
	}

	// only makes sense for compressed CHDs with valid SHA-1's
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
		std::error_condition err = input_chd.read_bytes(offset, &buffer[0], bytes_to_read);
		if (err)
			report_error(1, "Error reading CHD file (%s): %s", *input_chd_str->second, err.message());

		// add to the checksum
		rawsha1.append(&buffer[0], bytes_to_read);
		offset += bytes_to_read;
	}
	util::sha1_t computed_sha1 = rawsha1.finish();

	// finish up
	if (raw_sha1 != computed_sha1)
	{
		util::stream_format(std::cerr, "Error: Raw SHA1 in header = %s\n", raw_sha1.as_string());
		util::stream_format(std::cerr, "              actual SHA1 = %s\n", computed_sha1.as_string());

		// fix it if requested; this also fixes the overall one so we don't need to do any more
		if (fix_sha1)
		{
			std::error_condition err = input_chd.set_raw_sha1(computed_sha1);
			if (err)
				report_error(1, "Error updating SHA1: %s", err.message());
			util::stream_format(std::cout, "SHA1 updated to correct value in input CHD\n");
		}
	}
	else
	{
		util::stream_format(std::cout, "Raw SHA1 verification successful!\n");

		// now include the metadata for >= v4
		if (input_chd.version() >= 4)
		{
			util::sha1_t computed_overall_sha1 = input_chd.compute_overall_sha1(computed_sha1);
			if (input_chd.sha1() == computed_overall_sha1)
				util::stream_format(std::cout, "Overall SHA1 verification successful!\n");
			else
			{
				util::stream_format(std::cerr, "Error: Overall SHA1 in header = %s\n", input_chd.sha1().as_string());
				util::stream_format(std::cerr, "                  actual SHA1 = %s\n", computed_overall_sha1.as_string());

				// fix it if requested
				if (fix_sha1)
				{
					std::error_condition err = input_chd.set_raw_sha1(computed_sha1);
					if (err)
						report_error(1, "Error updating SHA1: %s", err.message());
					util::stream_format(std::cout, "SHA1 updated to correct value in input CHD\n");
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
	auto [input_file, input_file_str] = open_input_file(params);

	// process output CHD
	chd_file output_parent;
	const auto output_chd_str = parse_output_chd_parameters(params, output_parent);

	// process unit size
	uint32_t unit_size = output_parent.opened() ? output_parent.unit_bytes() : 0;
	auto unit_size_str = params.find(OPTION_UNIT_SIZE);
	if (unit_size_str != params.end())
	{
		unit_size = parse_number(unit_size_str->second->c_str());
		if (output_parent.opened() && (output_parent.unit_bytes() != unit_size))
			report_error(1, "Specified unit size %u bytes does not match output parent CHD unit size %u bytes", unit_size, output_parent.unit_bytes());
	}
	else if (!output_parent.opened())
	{
		report_error(1, "Unit size must be specified if no output parent CHD is supplied");
	}

	// process hunk size
	const uint32_t hunk_size = parse_hunk_size(params, output_parent, unit_size, std::max((4096 / unit_size) * unit_size, unit_size));

	// process input start/end (needs to know hunk_size)
	const auto [input_size, input_start, input_end] = parse_input_start_end(params, *input_file, hunk_size, hunk_size);
	if ((input_end - input_start) % unit_size)
		report_error(1, "Data size %s is not divisible by unit size %d", big_int_string(input_end - input_start), unit_size);

	// process compression
	chd_codec_type compression[4];
	parse_compression(params, s_default_raw_compression, output_parent, compression);

	// process numprocessors
	parse_numprocessors(params);

	// print some info
	util::stream_format(std::cout, "Output CHD:   %s\n", *output_chd_str);
	if (output_parent.opened())
		util::stream_format(std::cout, "Parent CHD:   %s\n", *params.find(OPTION_OUTPUT_PARENT)->second);
	util::stream_format(std::cout, "Input file:   %s\n", *input_file_str);
	if (input_start != 0 || input_end != input_size)
	{
		util::stream_format(std::cout, "Input start:  %s\n", big_int_string(input_start));
		util::stream_format(std::cout, "Input length: %s\n", big_int_string(input_end - input_start));
	}
	util::stream_format(std::cout, "Compression:  %s\n", compression_string(compression));
	util::stream_format(std::cout, "Hunk size:    %s\n", big_int_string(hunk_size));
	util::stream_format(std::cout, "Logical size: %s\n", big_int_string(input_end - input_start));

	// catch errors so we can close & delete the output file
	try
	{
		// create the new CHD
		auto chd = std::make_unique<chd_rawfile_compressor>(*input_file, input_start, input_end);
		create_output_chd(*chd, *output_chd_str, input_end - input_start, hunk_size, unit_size, compression, output_parent);

		// if we have a parent, copy forward all the metadata
		if (output_parent.opened())
			chd->clone_all_metadata(output_parent);

		// compress it generically
		compress_common(*chd);
	}
	catch (...)
	{
		// delete the output file
		osd_file::remove(*output_chd_str);
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
	auto [input_file, input_file_str] = open_input_file(params);

	// process output CHD
	chd_file output_parent;
	const auto output_chd_str = parse_output_chd_parameters(params, output_parent);

	// process template
	uint32_t cylinders = 0;
	uint32_t heads = 0;
	uint32_t sectors = 0;
	uint32_t sector_size = output_parent.opened() ? output_parent.unit_bytes() : IDE_SECTOR_SIZE;
	const auto template_str = params.find(OPTION_TEMPLATE);
	uint32_t template_id = 0;
	if (template_str != params.end())
	{
		if (output_parent.opened())
			report_error(1, "Template cannot be used when a parent CHD is supplied");
		template_id = parse_number(template_str->second->c_str());
		if (template_id >= std::size(s_hd_templates))
			report_error(1, "Template '%s' is invalid\n", *template_str->second);

		cylinders = s_hd_templates[template_id].cylinders;
		heads = s_hd_templates[template_id].heads;
		sectors = s_hd_templates[template_id].sectors;
		sector_size = s_hd_templates[template_id].sector_size;
	}

	// process CHS
	const auto chs_str = params.find(OPTION_CHS);
	if (chs_str != params.end())
	{
		if (template_str != params.end())
			report_error(1, "CHS geometry cannot be specified separately when a template is specified");
		if (output_parent.opened())
			report_error(1, "CHS geometry cannot be specified when a parent CHD is supplied");
		if (sscanf(chs_str->second->c_str(), "%u,%u,%u", &cylinders, &heads, &sectors) != 3)
			report_error(1, "Invalid CHS string; must be of the form <cylinders>,<heads>,<sectors>");
	}

	// process sector size
	auto sectorsize_str = params.find(OPTION_SECTOR_SIZE);
	if (sectorsize_str != params.end())
	{
		if (template_str != params.end())
			report_error(1, "Sector size cannot be specified separately when a template is specified");
		sector_size = parse_number(sectorsize_str->second->c_str());
	}
	if (output_parent.opened() && (output_parent.unit_bytes() != sector_size))
		report_error(1, "Sector size %u bytes does not match output parent CHD sector size %u bytes", sector_size, output_parent.unit_bytes());

	// process hunk size (needs to know sector_size)
	const uint32_t hunk_size = parse_hunk_size(params, output_parent, sector_size, std::max((4096 / sector_size) * sector_size, sector_size));

	// process input start/end (needs to know hunk_size)
	uint64_t filesize = 0;
	uint64_t input_size = 0;
	uint64_t input_start = 0;
	uint64_t input_end = 0;
	if (input_file)
	{
		std::tie(input_size, input_start, input_end) = parse_input_start_end(params, *input_file, hunk_size, hunk_size);
		filesize = input_end - input_start;
		if (params.find(OPTION_SIZE) != params.end())
			report_error(1, "Size cannot be specified when an input file is supplied");
	}
	else
	{
		const auto size_str = params.find(OPTION_SIZE);
		if (size_str != params.end())
		{
			if (sscanf(size_str->second->c_str(), "%" I64FMT"u", &filesize) != 1)
				report_error(1, "Invalid size specified");
		}
		if ((params.find(OPTION_INPUT_START_BYTE) != params.end()) || (params.find(OPTION_INPUT_START_HUNK) != params.end()) ||
				(params.find(OPTION_INPUT_LENGTH_BYTES) != params.end()) || (params.find(OPTION_INPUT_LENGTH_HUNKS) != params.end()))
			report_error(1, "Input start/length cannot be specified when no input file is supplied");
	}

	// process compression
	chd_codec_type compression[4];
	parse_compression(params, input_file ? s_default_hd_compression : s_no_compression, output_parent, compression);
	if (!input_file && (compression[0] != CHD_CODEC_NONE))
		report_error(1, "Blank hard disk images must be uncompressed");

	// process numprocessors
	parse_numprocessors(params);

	// process ident
	std::vector<uint8_t> identdata;
	if (output_parent.opened())
		output_parent.read_metadata(HARD_DISK_IDENT_METADATA_TAG, 0, identdata);
	auto ident_str = params.find(OPTION_IDENT);
	if (ident_str != params.end())
	{
		// load the file
		std::error_condition const filerr = util::core_file::load(*ident_str->second, identdata);
		if (filerr)
			report_error(1, "Error reading ident file (%s): %s", *ident_str->second, filerr.message());

		// must be at least 14 bytes; extract CHS data from there
		if (identdata.size() < 14)
			report_error(1, "Ident file '%s' is invalid (too short)", *ident_str->second);
		cylinders = get_u16le(&identdata[2]);
		heads = get_u16le(&identdata[6]);
		sectors = get_u16le(&identdata[12]);

		// ignore CHS for > 8GB drives
		if (cylinders * heads * sectors >= 16'514'064)
			cylinders = 0;
	}

	// extract geometry from the parent if we have one
	if (output_parent.opened() && cylinders == 0)
	{
		std::string metadata;
		if (output_parent.read_metadata(HARD_DISK_METADATA_TAG, 0, metadata))
			report_error(1, "Unable to find hard disk metadata in parent CHD");
		if (sscanf(metadata.c_str(), HARD_DISK_METADATA_FORMAT, &cylinders, &heads, &sectors, &sector_size) != 4)
			report_error(1, "Error parsing hard disk metadata in parent CHD");
	}

	// validate the size
	if (filesize % sector_size)
		report_error(1, "Data size %s is not divisible by sector size %u", big_int_string(input_end - input_start), sector_size);

	// if no CHS values, try to guess them
	if (cylinders == 0)
	{
		if (!input_file && filesize == 0)
			report_error(1, "Length or CHS geometry must be specified when creating a blank hard disk image");
		guess_chs(input_file_str, filesize, sector_size, cylinders, heads, sectors, sector_size);
	}
	uint32_t totalsectors = cylinders * heads * sectors;

	// print some info
	util::stream_format(std::cout, "Output CHD:   %s\n", *output_chd_str);
	if (output_parent.opened())
		util::stream_format(std::cout, "Parent CHD:   %s\n", *params.find(OPTION_OUTPUT_PARENT)->second);
	if (input_file)
	{
		util::stream_format(std::cout, "Input file:   %s\n", *input_file_str);
		if (input_start != 0 || input_end != input_size)
		{
			util::stream_format(std::cout, "Input start:  %s\n", big_int_string(input_start));
			util::stream_format(std::cout, "Input length: %s\n", big_int_string(filesize));
		}
	}
	util::stream_format(std::cout, "Compression:  %s\n", compression_string(compression));
	if (template_str != params.end())
		util::stream_format(std::cout, "Template:     %s %s\n", s_hd_templates[template_id].manufacturer, s_hd_templates[template_id].model);
	util::stream_format(std::cout, "Cylinders:    %u\n", cylinders);
	util::stream_format(std::cout, "Heads:        %u\n", heads);
	util::stream_format(std::cout, "Sectors:      %u\n", sectors);
	util::stream_format(std::cout, "Bytes/sector: %u\n", sector_size);
	util::stream_format(std::cout, "Sectors/hunk: %u\n", hunk_size / sector_size);
	util::stream_format(std::cout, "Logical size: %s\n", big_int_string(uint64_t(totalsectors) * uint64_t(sector_size)));

	// catch errors so we can close & delete the output file
	try
	{
		// create the new hard drive
		std::unique_ptr<chd_file_compressor> chd;
		if (input_file)
			chd.reset(new chd_rawfile_compressor(*input_file, input_start, input_end));
		else
			chd.reset(new chd_zero_compressor(input_start, input_end));
		create_output_chd(*chd, *output_chd_str, uint64_t(totalsectors) * sector_size, hunk_size, sector_size, compression, output_parent);

		// add the standard hard disk metadata
		std::string metadata = string_format(HARD_DISK_METADATA_FORMAT, cylinders, heads, sectors, sector_size);
		std::error_condition err;
		err = chd->write_metadata(HARD_DISK_METADATA_TAG, 0, metadata);
		if (err)
			report_error(1, "Error adding hard disk metadata: %s", err.message());

		// write the ident if present
		if (!identdata.empty())
		{
			err = chd->write_metadata(HARD_DISK_IDENT_METADATA_TAG, 0, identdata);
			if (err)
				report_error(1, "Error adding hard disk metadata: %s", err.message());
		}

		// compress it generically
		if (input_file)
			compress_common(*chd);
	}
	catch (...)
	{
		// delete the output file
		osd_file::remove(*output_chd_str);
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
	cdrom_file::track_input_info track_info;
	cdrom_file::toc toc = { 0 };
	auto input_file_str = params.find(OPTION_INPUT);
	if (input_file_str != params.end())
	{
		std::error_condition err = cdrom_file::parse_toc(*input_file_str->second, toc, track_info);
		if (err)
			report_error(1, "Error parsing input file (%s: %s)\n", *input_file_str->second, err.message());
	}

	// process output CHD
	chd_file output_parent;
	const auto output_chd_str = parse_output_chd_parameters(params, output_parent);

	// process hunk size
	const uint32_t hunk_size = parse_hunk_size(params, output_parent, cdrom_file::FRAME_SIZE, cdrom_file::FRAMES_PER_HUNK * cdrom_file::FRAME_SIZE);
	if (output_parent.opened() && (output_parent.unit_bytes() != cdrom_file::FRAME_SIZE))
		report_error(1, "Output parent CHD sector size %u bytes does not match CD-ROM frame size %u bytes", output_parent.unit_bytes(), cdrom_file::FRAME_SIZE);

	// process compression
	chd_codec_type compression[4];
	parse_compression(params, s_default_cd_compression, output_parent, compression);

	// process numprocessors
	parse_numprocessors(params);

	// pad each track to a 4-frame boundary. cdrom.cpp will deal with this on the read side
	uint32_t origtotalsectors = 0;
	uint32_t totalsectors = 0;
	for (int tracknum = 0; tracknum < toc.numtrks; tracknum++)
	{
		cdrom_file::track_info &trackinfo = toc.tracks[tracknum];
		int padded = (trackinfo.frames + cdrom_file::TRACK_PADDING - 1) / cdrom_file::TRACK_PADDING;
		trackinfo.extraframes = padded * cdrom_file::TRACK_PADDING - trackinfo.frames;
		origtotalsectors += trackinfo.frames;
		totalsectors += trackinfo.frames + trackinfo.extraframes;
	}

	// print some info
	util::stream_format(std::cout, "Output CHD:   %s\n", *output_chd_str);
	if (output_parent.opened())
		util::stream_format(std::cout, "Parent CHD:   %s\n", *params.find(OPTION_OUTPUT_PARENT)->second);
	util::stream_format(std::cout, "Input file:   %s\n", *input_file_str->second);
	util::stream_format(std::cout, "Input tracks: %d\n", toc.numtrks);
	util::stream_format(std::cout, "Input length: %s\n", msf_string_from_frames(origtotalsectors));
	util::stream_format(std::cout, "Compression:  %s\n", compression_string(compression));
	util::stream_format(std::cout, "Logical size: %s\n", big_int_string(uint64_t(totalsectors) * cdrom_file::FRAME_SIZE));

	// catch errors so we can close & delete the output file
	try
	{
		// create the new CD
		auto chd = std::make_unique<chd_cd_compressor>(toc, track_info);
		create_output_chd(*chd, *output_chd_str, uint64_t(totalsectors) * cdrom_file::FRAME_SIZE, hunk_size, cdrom_file::FRAME_SIZE, compression, output_parent);

		// add the standard CD metadata; we do this even if we have a parent because it might be different
		const std::error_condition err = cdrom_file::write_metadata(chd.get(), toc);
		if (err)
			report_error(1, "Error adding CD metadata: %s", err.message());

		// compress it generically
		compress_common(*chd);
	}
	catch (...)
	{
		// delete the output file
		osd_file::remove(*output_chd_str);
		throw;
	}
}


//-------------------------------------------------
//  do_create_dvd - create a new compressed dvd
//  image from a raw file
//-------------------------------------------------

static void do_create_dvd(parameters_map &params)
{
	// process input file
	auto [input_file, input_file_str] = open_input_file(params);

	// process output CHD
	chd_file output_parent;
	const auto output_chd_str = parse_output_chd_parameters(params, output_parent);

	// process hunk size
	const uint32_t hunk_size = parse_hunk_size(params, output_parent, 2048, 2 * 2048);
	if (output_parent.opened() && (output_parent.unit_bytes() != 2048))
		report_error(1, "Output parent CHD sector size %u bytes does not match DVD-ROM sector size 2048 bytes", output_parent.unit_bytes());

	// process input start/end
	const auto [input_size, input_start, input_end] = parse_input_start_end(params, *input_file, hunk_size, hunk_size);
	if ((input_end - input_start) % 2048)
		report_error(1, "Data size %s is not divisible by sector size 2048", big_int_string(input_end - input_start));

	// process compression
	chd_codec_type compression[4];
	parse_compression(params, s_default_hd_compression, output_parent, compression); // No reason to be different than HD for compression

	// process numprocessors
	parse_numprocessors(params);

	// print some info
	util::stream_format(std::cout, "Output CHD:   %s\n", *output_chd_str);
	if (output_parent.opened())
		util::stream_format(std::cout, "Parent CHD:   %s\n", *params.find(OPTION_OUTPUT_PARENT)->second);
	util::stream_format(std::cout, "Input file:   %s\n", *input_file_str);
	if (input_start != 0 || input_end != input_size)
	{
		util::stream_format(std::cout, "Input start:  %s\n", big_int_string(input_start));
		util::stream_format(std::cout, "Input length: %s\n", big_int_string(input_end - input_start));
	}
	util::stream_format(std::cout, "Compression:  %s\n", compression_string(compression));
	util::stream_format(std::cout, "Logical size: %s\n", big_int_string(input_end - input_start));

	// catch errors so we can close & delete the output file
	try
	{
		// create the new DVD
		auto chd = std::make_unique<chd_rawfile_compressor>(*input_file, input_start, input_end);
		create_output_chd(*chd, *output_chd_str, input_end - input_start, hunk_size, 2048, compression, output_parent);

		// add the standard DVD type tag
		const std::error_condition err = chd->write_metadata(DVD_METADATA_TAG, 0, "");
		if (err)
			report_error(1, "Error adding DVD metadata: %s", err.message());

		// compress it generically
		compress_common(*chd);
	}
	catch (...)
	{
		// delete the output file
		osd_file::remove(*output_chd_str);
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
			report_error(1, "Error opening AVI file (%s): %s\n", *input_file_str->second, avi_file::error_string(avierr));
	}
	const avi_file::movie_info &aviinfo = input_file->get_movie_info();

	// process output CHD
	chd_file output_parent;
	const auto output_chd_str = parse_output_chd_parameters(params, output_parent);

	// process input start/end
	auto [input_start, input_end] = parse_input_start_end(params, aviinfo.video_numsamples, 0, 1);

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
	const uint32_t hunk_size = parse_hunk_size(params, output_parent, info.bytes_per_frame, info.bytes_per_frame);

	// process compression
	chd_codec_type compression[4];
	parse_compression(params, s_default_ld_compression, output_parent, compression);
	// disable support for uncompressed ones until the extraction code can handle it
	if (compression[0] == CHD_CODEC_NONE)
		report_error(1, "Uncompressed is not supported");

	// process numprocessors
	parse_numprocessors(params);

	// print some info
	util::stream_format(std::cout, "Output CHD:   %s\n", *output_chd_str);
	if (output_parent.opened())
		util::stream_format(std::cout, "Parent CHD:   %s\n", *params.find(OPTION_OUTPUT_PARENT)->second);
	util::stream_format(std::cout, "Input file:   %s\n", *input_file_str->second);
	if (input_start != 0 && input_end != aviinfo.video_numsamples)
		util::stream_format(std::cout, "Input start:  %s\n", big_int_string(input_start));
	util::stream_format(std::cout, "Input length: %s (%02d:%02d:%02d)\n", big_int_string(input_end - input_start),
			uint32_t((uint64_t(input_end - input_start) * 1000000 / info.fps_times_1million / 60 / 60)),
			uint32_t(((uint64_t(input_end - input_start) * 1000000 / info.fps_times_1million / 60) % 60)),
			uint32_t(((uint64_t(input_end - input_start) * 1000000 / info.fps_times_1million) % 60)));
	util::stream_format(std::cout, "Frame rate:   %d.%06d\n", info.fps_times_1million / 1000000, info.fps_times_1million % 1000000);
	util::stream_format(std::cout, "Frame size:   %d x %d %s\n", info.width, info.height * (info.interlaced ? 2 : 1), info.interlaced ? "interlaced" : "non-interlaced");
	util::stream_format(std::cout, "Audio:        %d channels at %d Hz\n", info.channels, info.rate);
	util::stream_format(std::cout, "Compression:  %s\n", compression_string(compression));
	util::stream_format(std::cout, "Hunk size:    %s\n", big_int_string(hunk_size));
	util::stream_format(std::cout, "Logical size: %s\n", big_int_string(uint64_t(input_end - input_start) * hunk_size));

	// catch errors so we can close & delete the output file
	try
	{
		// create the new CHD
		auto chd = std::make_unique<chd_avi_compressor>(*input_file, info, input_start, input_end);
		create_output_chd(*chd, *output_chd_str, uint64_t(input_end - input_start) * hunk_size, hunk_size, info.bytes_per_frame, compression, output_parent);

		// write the core A/V metadata
		std::string metadata = string_format(AV_METADATA_FORMAT, info.fps_times_1million / 1000000, info.fps_times_1million % 1000000, info.width, info.height, info.interlaced, info.channels, info.rate);
		std::error_condition err;
		err = chd->write_metadata(AV_METADATA_TAG, 0, metadata);
		if (err)
			report_error(1, "Error adding AV metadata: %s\n", err.message());

		// create the compressor and then run it generically
		compress_common(*chd);

		// write the final LD metadata
		if (info.height == 524/2 || info.height == 624/2)
		{
			err = chd->write_metadata(AV_LD_METADATA_TAG, 0, chd->ldframedata(), 0);
			if (err)
				report_error(1, "Error adding AVLD metadata: %s\n", err.message());
		}
	}
	catch (...)
	{
		// delete the output file
		osd_file::remove(*output_chd_str);
		throw;
	}
}


//-------------------------------------------------
//  get_compression_defaults - use CHD metadata to
//  pick the preferred type
//-------------------------------------------------

static const std::array<chd_codec_type, 4> &get_compression_defaults(chd_file &input_chd)
{
	std::error_condition err = input_chd.check_is_hd();
	if (err == chd_file::error::METADATA_NOT_FOUND)
		err = input_chd.check_is_dvd();
	if (!err)
		return s_default_hd_compression;
	if (err != chd_file::error::METADATA_NOT_FOUND)
		throw err;

	err = input_chd.check_is_av();
	if (!err)
		return s_default_ld_compression;
	if (err != chd_file::error::METADATA_NOT_FOUND)
		throw err;

	err = input_chd.check_is_cd();
	if (err == chd_file::error::METADATA_NOT_FOUND)
		err = input_chd.check_is_gd();
	if (!err)
		return s_default_cd_compression;
	if (err != chd_file::error::METADATA_NOT_FOUND)
		throw err;

	return s_default_raw_compression;
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
	const auto [input_start, input_end] = parse_input_start_end(params, input_chd.logical_bytes(), input_chd.hunk_bytes(), input_chd.hunk_bytes());
	// TODO: should we check that the input range is aligned to the unit size?

	// process output CHD
	chd_file output_parent;
	const auto output_chd_str = parse_output_chd_parameters(params, output_parent);

	// process hunk size
	const uint32_t hunk_size = parse_hunk_size(params, output_parent, input_chd.unit_bytes(), input_chd.hunk_bytes());
	if ((hunk_size % input_chd.hunk_bytes()) && (input_chd.hunk_bytes() % hunk_size)) // TODO: is this check still necessary, or is the unit size check enough?
		report_error(1, "Hunk size is not a whole multiple or factor of input hunk size");

	// process compression; we default to our current preferences using metadata to pick the type
	chd_codec_type compression[4];
	parse_compression(params, get_compression_defaults(input_chd), output_parent, compression);

	// process numprocessors
	parse_numprocessors(params);

	// print some info
	util::stream_format(std::cout, "Output CHD:   %s\n", *output_chd_str);
	if (output_parent.opened())
		util::stream_format(std::cout, "Parent CHD:   %s\n", *params.find(OPTION_OUTPUT_PARENT)->second);
	util::stream_format(std::cout, "Input CHD:    %s\n", *params.find(OPTION_INPUT)->second);
	if (input_start != 0 || input_end != input_chd.logical_bytes())
	{
		util::stream_format(std::cout, "Input start:  %s\n", big_int_string(input_start));
		util::stream_format(std::cout, "Input length: %s\n", big_int_string(input_end - input_start));
	}
	util::stream_format(std::cout, "Compression:  %s\n", compression_string(compression));
	util::stream_format(std::cout, "Hunk size:    %s\n", big_int_string(hunk_size));
	util::stream_format(std::cout, "Logical size: %s\n", big_int_string(input_end - input_start));

	// catch errors so we can close & delete the output file
	try
	{
		// create the new CHD
		std::unique_ptr<cdrom_file> cdrom; // want this to be unwound after chd
		auto chd = std::make_unique<chd_chdfile_compressor>(input_chd, input_start, input_end);
		create_output_chd(*chd, *output_chd_str, input_end - input_start, hunk_size, input_chd.unit_bytes(), compression, output_parent);

		// clone all the metadata, upgrading where appropriate
		std::error_condition err;
		std::vector<uint8_t> metadata;
		chd_metadata_tag metatag;
		uint8_t metaflags;
		uint32_t index = 0;
		bool redo_cd = false;
		bool cdda_swap = false;
		for (err = input_chd.read_metadata(CHDMETATAG_WILDCARD, index++, metadata, metatag, metaflags); !err; err = input_chd.read_metadata(CHDMETATAG_WILDCARD, index++, metadata, metatag, metaflags))
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
			if (err)
				report_error(1, "Error writing cloned metadata: %s", err.message());
		}

		// if we need to re-do the CD metadata, do it now
		if (redo_cd)
		{
			cdrom = std::make_unique<cdrom_file>(&input_chd);
			const cdrom_file::toc &toc = cdrom->get_toc();
			err = cdrom_file::write_metadata(chd.get(), toc);
			if (err)
				report_error(1, "Error writing upgraded CD metadata: %s", err.message());
			if (cdda_swap)
				chd->m_toc = &toc;
		}

		// compress it generically
		compress_common(*chd);
	}
	catch (...)
	{
		// delete the output file
		osd_file::remove(*output_chd_str);
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
	const auto [input_start, input_end] = parse_input_start_end(params, input_chd.logical_bytes(), input_chd.hunk_bytes(), input_chd.hunk_bytes());

	// verify output file doesn't exist
	auto output_file_str = params.find(OPTION_OUTPUT);
	if (output_file_str != params.end())
		check_existing_output_file(params, *output_file_str->second);

	// print some info
	util::stream_format(std::cout, "Output File:  %s\n", *output_file_str->second);
	util::stream_format(std::cout, "Input CHD:    %s\n", *params.find(OPTION_INPUT)->second);
	if (input_start != 0 || input_end != input_chd.logical_bytes())
	{
		util::stream_format(std::cout, "Input start:  %s\n", big_int_string(input_start));
		util::stream_format(std::cout, "Input length: %s\n", big_int_string(input_end - input_start));
	}

	// catch errors so we can close & delete the output file
	util::core_file::ptr output_file;
	try
	{
		// process output file
		std::error_condition const filerr = util::core_file::open(*output_file_str->second, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, output_file);
		if (filerr)
			report_error(1, "Unable to open file (%s): %s", *output_file_str->second, filerr.message());

		// copy all data
		std::vector<uint8_t> buffer((TEMP_BUFFER_SIZE / input_chd.hunk_bytes()) * input_chd.hunk_bytes());
		for (uint64_t offset = input_start; offset < input_end; )
		{
			progress(false, "Extracting, %.1f%% complete... \r", 100.0 * double(offset - input_start) / double(input_end - input_start));

			// determine how much to read
			uint32_t bytes_to_read = (std::min<uint64_t>)(buffer.size(), input_end - offset);
			std::error_condition err = input_chd.read_bytes(offset, &buffer[0], bytes_to_read);
			if (err)
				report_error(1, "Error reading CHD file (%s): %s", *params.find(OPTION_INPUT)->second, err.message());

			// write to the output
			auto const [writerr, count] = write(*output_file, &buffer[0], bytes_to_read);
			if (writerr)
				report_error(1, "Error writing to file; check disk space (%s)", *output_file_str->second);

			// advance
			offset += bytes_to_read;
		}

		// finish up
		output_file.reset();
		util::stream_format(std::cout, "Extraction complete                                    \n");
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
	cdrom_file *cdrom = new cdrom_file(&input_chd);
	const cdrom_file::toc &toc = cdrom->get_toc();

	// verify output file doesn't exist
	auto output_file_str = params.find(OPTION_OUTPUT);
	if (output_file_str != params.end())
		check_existing_output_file(params, *output_file_str->second);

	// determine output type based on the specified file extension
	int mode = MODE_NORMAL;
	if (core_filename_ends_with(*output_file_str->second, ".cue"))
		mode = MODE_CUEBIN;
	else if (core_filename_ends_with(*output_file_str->second, ".gdi"))
		mode = MODE_GDI;

	// determine the output bin filename based on provided input parameters
	auto output_bin_file_fnd = params.find(OPTION_OUTPUT_BIN);
	std::string default_name(*output_file_str->second);

	// split path and extension
	int chop = default_name.find_last_of('.');
	if (chop != std::string::npos)
		default_name.erase(chop, default_name.size());

	// GDIs will always output as split bin
	bool is_splitbin = mode == MODE_GDI || params.find(OPTION_OUTPUT_SPLITBIN) != params.end();
	if (!is_splitbin && cdrom->is_gdrom() && mode == MODE_CUEBIN)
	{
		// GD-ROM cue/bin is in Redump format which should always be split by tracks
		util::stream_format(std::cout, "Warning: --%s is required for this specific combination of input disc type and output format, enabling automatically\n", OPTION_OUTPUT_SPLITBIN);
		is_splitbin = true;
	}

	if (is_splitbin)
	{
		if (mode == MODE_GDI)
		{
			default_name += "%02t";
		}
		else
		{
			const std::string format = toc.numtrks >= 10 ? "%02t" : "%t";
			default_name += " (Track " + format + ")";
		}
	}

	std::string output_bin_file_ext = ".bin";
	std::string *output_bin_file_str;
	if (output_bin_file_fnd == params.end())
	{
		output_bin_file_str = &default_name;
	}
	else
	{
		output_bin_file_str = output_bin_file_fnd->second;

		chop = output_bin_file_str->find_last_of('.');
		if (chop != std::string::npos)
		{
			output_bin_file_ext = output_bin_file_str->substr(chop, output_bin_file_str->size() - chop);
			output_bin_file_str->erase(chop, output_bin_file_str->size());
		}
	}

	if (output_bin_file_str->find('"') != std::string::npos || output_bin_file_ext.find('"') != std::string::npos)
		report_error(1, "Output bin filename (%s%s) must not contain quotation marks", *output_bin_file_str, output_bin_file_ext);

	// print some info
	util::stream_format(std::cout, "Input CHD:    %s\n", *params.find(OPTION_INPUT)->second);
	util::stream_format(std::cout, "Output TOC:   %s\n", *output_file_str->second);

	// catch errors so we can close & delete the output file
	std::vector<std::string> track_filenames;
	util::core_file::ptr output_bin_file;
	util::core_file::ptr output_toc_file;
	std::vector<std::string> output_bin_filenames;
	std::string trackbin_name;
	try
	{
		// process output file
		std::error_condition filerr = util::core_file::open(*output_file_str->second, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_NO_BOM, output_toc_file);
		if (filerr)
			report_error(1, "Unable to open file (%s): %s", *output_file_str->second, filerr.message());

		uint64_t total_bytes = 0;
		for (int tracknum = 0; tracknum < toc.numtrks; tracknum++)
		{
			// determine total frames
			total_bytes += toc.tracks[tracknum].frames * (toc.tracks[tracknum].datasize + toc.tracks[tracknum].subsize);

			// generate output filename for each track
			std::string new_trackbin_name = *output_bin_file_str;

			if (mode == MODE_GDI && toc.tracks[tracknum].trktype == cdrom_file::CD_TRACK_AUDIO)
				new_trackbin_name += ".raw";
			else
				new_trackbin_name += output_bin_file_ext;

			// variable replacement in output filename
			const std::regex variables_regex("(%*)(%([+-]?\\d+)?([a-zA-Z]))");
			std::string::const_iterator new_trackbin_name_itr = new_trackbin_name.begin();
			std::string::const_iterator new_trackbin_name_end = new_trackbin_name.end();
			std::string filename_formatted = new_trackbin_name;
			std::smatch variable_matches;
			bool found_track_variable = false;

			while (std::regex_search(new_trackbin_name_itr, new_trackbin_name_end, variable_matches, variables_regex))
			{
				// full_match will always have one leading %, so if leading_escape has an even number of %s then
				// we can know that we're working on an unescaped %
				const std::string leading_escape = variable_matches[1].str();
				const std::string full_match = variable_matches[2].str();
				const std::string format_part = variable_matches[3].str();
				const std::string format_type = variable_matches[4].str();

				if ((leading_escape.size() % 2) == 0)
				{
					std::string replacement;

					if (format_type == "t")
					{
						// track number
						if (is_splitbin)
						{
							replacement = util::string_format("%" + format_part + "d", tracknum+1);
							found_track_variable = true;
						}
					}
					else
					{
						util::stream_format(std::cout, "Warning: encountered unknown format value '%s', ignoring\n", format_type);
					}

					if (!replacement.empty())
					{
						// replace all instances of encountered full format token
						size_t index = std::string::npos;
						while ((index = filename_formatted.find(full_match)) != std::string::npos)
							filename_formatted.replace(index, full_match.size(), replacement);
					}
				}

				new_trackbin_name_itr = variable_matches.suffix().first; // move past match for next loop
			}

			if (is_splitbin && !found_track_variable)
			{
				report_error(1, "A track number variable (%%t) must be specified in the output bin filename when --%s is enabled\n", OPTION_OUTPUT_SPLITBIN);
			}

			// verify output BIN file doesn't exist
			check_existing_output_file(params, filename_formatted);

			// display all new filenames to the user
			if (track_filenames.empty() || filename_formatted != track_filenames.back())
				util::stream_format(std::cout, "Output Data:  %s\n", filename_formatted);

			track_filenames.push_back(filename_formatted);
		}

		// GDI must start with the # of tracks
		if (mode == MODE_GDI)
		{
			output_toc_file->printf("%d\n", toc.numtrks);
		}
		else if (mode == MODE_NORMAL)
		{
			bool mode1 = false;
			bool mode2 = false;
			bool cdda = false;

			for (int tracknum = 0; tracknum < toc.numtrks; tracknum++)
			{
				switch (toc.tracks[tracknum].trktype)
				{
					case cdrom_file::CD_TRACK_MODE1:
					case cdrom_file::CD_TRACK_MODE1_RAW:
						mode1 = true;
						break;

					case cdrom_file::CD_TRACK_MODE2:
					case cdrom_file::CD_TRACK_MODE2_FORM1:
					case cdrom_file::CD_TRACK_MODE2_FORM2:
					case cdrom_file::CD_TRACK_MODE2_FORM_MIX:
					case cdrom_file::CD_TRACK_MODE2_RAW:
						mode2 = true;
						break;

					case cdrom_file::CD_TRACK_AUDIO:
						cdda = true;
						break;
				}
			}

			if (mode2)
				output_toc_file->printf("CD_ROM_XA\n\n\n");
			else if (cdda && !mode1)
				output_toc_file->printf("CD_DA\n\n\n");
			else
				output_toc_file->printf("CD_ROM\n\n\n");
		}

		if (cdrom->is_gdrom() && mode == MODE_CUEBIN)
		{
			// modify TOC to match Redump cue/bin format as best as possible
			cdrom_file::toc *trackinfo = (cdrom_file::toc*)&toc;

			// TOSEC GDI-based CHDs have the padframes field set to non-0 where the pregaps for the next track would be
			const bool has_physical_pregap = trackinfo->tracks[0].padframes == 0;

			for (int tracknum = 1; tracknum < toc.numtrks; tracknum++)
			{
				// pgdatasize should never be set in GD-ROMs currently, so if it is set then assume the TOC has proper pregap values
				if (trackinfo->tracks[tracknum].pgdatasize != 0)
					break;

				// don't adjust the first track of the single-density and high-density areas
				if (toc.tracks[tracknum].physframeofs == 45000)
					continue;

				if (!has_physical_pregap)
				{
					// NOTE: This will generate a cue with PREGAP commands instead of INDEX 00 because the pregap data isn't baked into the bins
					trackinfo->tracks[tracknum].pregap += trackinfo->tracks[tracknum-1].padframes;

					// "type 1" (only one data track in high-density area) and "type 2" (1 data and then the rest of the tracks being audio tracks in high-density area) don't require any adjustments
					if (tracknum + 1 >= toc.numtrks && toc.tracks[tracknum].trktype != cdrom_file::CD_TRACK_AUDIO)
					{
						if (toc.tracks[tracknum-1].trktype != cdrom_file::CD_TRACK_AUDIO)
						{
							// "type 3" where the high-density area is just two data tracks
							// there shouldn't be any pregap in the padframes from the previous track in this case, and the full 3s pregap is baked into the previous track
							// Only known to be used by Shenmue II JP's discs 2, 3, 4 and Virtua Fighter History & VF4
							trackinfo->tracks[tracknum-1].padframes += 225;

							trackinfo->tracks[tracknum].pregap += 225;
							trackinfo->tracks[tracknum].splitframes = 225;
							trackinfo->tracks[tracknum].pgdatasize = trackinfo->tracks[tracknum].datasize;
							trackinfo->tracks[tracknum].pgtype = trackinfo->tracks[tracknum].trktype;
						}
						else
						{
							// "type 3 split" where the first track and last of the high-density area are data tracks and in between is audio tracks
							// TODO: These 75 frames are actually included at the end of the previous track so should be written
							// It's currently not possible to format it as expected without hacky code because the 150 pregap for the last track
							// is sandwiched between these 75 frames and the actual track data.
							// The 75 frames seems to normally be 0s so this should be ok for now until a use case is found.
							trackinfo->tracks[tracknum-1].frames -= 75;
							trackinfo->tracks[tracknum].pregap += 75;
						}
					}
				}
				else
				{
					int curextra = 150; // 00:02:00
					if (tracknum + 1 >= toc.numtrks && toc.tracks[tracknum].trktype != cdrom_file::CD_TRACK_AUDIO)
						curextra += 75; // 00:01:00, special case when last track is data

					trackinfo->tracks[tracknum-1].padframes = curextra;

					trackinfo->tracks[tracknum].pregap += curextra;
					trackinfo->tracks[tracknum].splitframes = curextra;
					trackinfo->tracks[tracknum].pgdatasize = trackinfo->tracks[tracknum].datasize;
					trackinfo->tracks[tracknum].pgtype = trackinfo->tracks[tracknum].trktype;
				}
			}
		}

		// iterate over tracks and copy all data
		uint64_t totaloutputoffs = 0;
		uint64_t outputoffs = 0;
		uint32_t discoffs = 0;
		std::vector<uint8_t> buffer;

		for (int tracknum = 0; tracknum < toc.numtrks; tracknum++)
		{
			if (track_filenames[tracknum] != trackbin_name)
			{
				totaloutputoffs += outputoffs;
				outputoffs = 0;

				if (mode != MODE_GDI)
					discoffs = 0;

				output_bin_file.reset();

				trackbin_name = track_filenames[tracknum];

				filerr = util::core_file::open(trackbin_name, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, output_bin_file);
				if (filerr)
					report_error(1, "Unable to open file (%s): %s", trackbin_name, filerr.message());

				output_bin_filenames.push_back(trackbin_name);
			}

			if (cdrom->is_gdrom() && mode == MODE_CUEBIN)
			{
				if (tracknum == 0)
					output_toc_file->printf("REM SINGLE-DENSITY AREA\n");
				else if (toc.tracks[tracknum].physframeofs == 45000)
					output_toc_file->printf("REM HIGH-DENSITY AREA\n");
			}

			// output the metadata about the track to the TOC file
			const cdrom_file::track_info &trackinfo = toc.tracks[tracknum];
			output_track_metadata(mode, *output_toc_file, tracknum, trackinfo, std::string(core_filename_extract_base(trackbin_name)), discoffs, outputoffs);

			// If this is bin/cue output and the CHD contains subdata, warn the user and don't include
			// the subdata size in the buffer calculation.
			uint32_t output_frame_size = trackinfo.datasize + ((trackinfo.subtype != cdrom_file::CD_SUB_NONE) ? trackinfo.subsize : 0);
			if (trackinfo.subtype != cdrom_file::CD_SUB_NONE && ((mode == MODE_CUEBIN) || (mode == MODE_GDI)))
			{
				util::stream_format(std::cout, "Warning: Track %d has subcode data.  bin/cue and gdi formats cannot contain subcode data and it will be omitted.\n", tracknum+1);
				util::stream_format(std::cout, "       : This may affect usage of the output image.  Use bin/toc output to keep all data.\n");
				output_frame_size = trackinfo.datasize;
			}

			// resize the buffer for the track
			buffer.resize((TEMP_BUFFER_SIZE / output_frame_size) * output_frame_size);

			// now read and output the actual data
			uint32_t bufferoffs = 0;
			uint32_t actualframes = trackinfo.frames - trackinfo.padframes + trackinfo.splitframes;
			for (uint32_t frame = 0; frame < actualframes; frame++)
			{
				progress(false, "Extracting, %.1f%% complete... \r", 100.0 * double(totaloutputoffs + outputoffs) / double(total_bytes));

				int trk, frameofs;
				if (tracknum > 0 && frame < trackinfo.splitframes)
				{
					// pull data from previous track, the reverse of how splitframes is used when making the GD-ROM CHDs
					trk = tracknum - 1;
					frameofs = toc.tracks[trk].frames - trackinfo.splitframes + frame;
				}
				else
				{
					trk = tracknum;
					frameofs = frame - trackinfo.splitframes;
				}

				// read the data
				cdrom->read_data(cdrom->get_track_start_phys(trk) + frameofs, &buffer[bufferoffs], toc.tracks[trk].trktype, true);

				// for CDRWin and GDI audio tracks must be reversed
				// in the case of GDI and CHD version < 5 we assuming source CHD image is GDROM so audio tracks is already reversed
				if (((mode == MODE_GDI && input_chd.version() > 4) || (mode == MODE_CUEBIN)) && (toc.tracks[trk].trktype == cdrom_file::CD_TRACK_AUDIO))
					for (int swapindex = 0; swapindex < toc.tracks[trk].datasize; swapindex += 2)
					{
						uint8_t swaptemp = buffer[bufferoffs + swapindex];
						buffer[bufferoffs + swapindex] = buffer[bufferoffs + swapindex + 1];
						buffer[bufferoffs + swapindex + 1] = swaptemp;
					}
				bufferoffs += toc.tracks[trk].datasize;
				discoffs++;

				// read the subcode data
				if (toc.tracks[trk].subtype != cdrom_file::CD_SUB_NONE && (mode == MODE_NORMAL))
				{
					cdrom->read_subcode(cdrom->get_track_start_phys(trk) + frameofs, &buffer[bufferoffs], true);
					bufferoffs += toc.tracks[trk].subsize;
				}

				// write it out if we need to
				if (bufferoffs == buffer.size() || frame == actualframes - 1)
				{
					output_bin_file->seek(outputoffs, SEEK_SET);
					auto const [writerr, byteswritten] = write(*output_bin_file, &buffer[0], bufferoffs);
					if (writerr)
						report_error(1, "Error writing frame %d to file (%s): %s\n", frame, *output_file_str->second, "Write error");
					outputoffs += bufferoffs;
					bufferoffs = 0;
				}
			}

			discoffs += trackinfo.padframes;
		}

		// finish up
		output_bin_file.reset();
		output_toc_file.reset();
		util::stream_format(std::cout, "Extraction complete                                    \n");
	}
	catch (...)
	{
		// delete the output files
		output_bin_file.reset();
		output_toc_file.reset();
		for (auto const &output_bin_filename : output_bin_filenames)
			osd_file::remove(output_bin_filename);
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
	std::error_condition err = input_chd.read_metadata(AV_METADATA_TAG, 0, metadata);
	if (err)
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
	auto [input_start, input_end] = parse_input_start_end(params, input_chd.hunk_count() / interlace_factor, 0, 1);
	input_start *= interlace_factor;
	input_end *= interlace_factor;

	// build up the movie info
	avi_file::movie_info info;
	info.video_format = avi_file::FORMAT_YUY2;
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
		check_existing_output_file(params, *output_file_str->second);

	// print some info
	util::stream_format(std::cout, "Output File:  %s\n", *output_file_str->second);
	util::stream_format(std::cout, "Input CHD:    %s\n", *params.find(OPTION_INPUT)->second);
	if (input_start != 0 || input_end != input_chd.hunk_count())
	{
		util::stream_format(std::cout, "Input start:  %s\n", big_int_string(input_start));
		util::stream_format(std::cout, "Input length: %s\n", big_int_string(input_end - input_start));
	}

	// catch errors so we can close & delete the output file
	avi_file::ptr output_file;
	try
	{
		// process output file
		avi_file::error avierr = avi_file::create(*output_file_str->second, info, output_file);
		if (avierr != avi_file::error::NONE)
			report_error(1, "Unable to open file (%s)", *output_file_str->second);

		// create the codec configuration
		avhuff_decoder::config avconfig;
		bitmap_yuy16 avvideo;
		std::vector<int16_t> audio_data[16];
		uint32_t actsamples;
		avconfig.video = &avvideo;
		avconfig.maxsamples = max_samples_per_frame;
		avconfig.actsamples = &actsamples;
		for (int chnum = 0; chnum < std::size(audio_data); chnum++)
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
			std::error_condition err = input_chd.codec_process_hunk(framenum);
			if (err)
			{
				uint64_t filepos = ~uint64_t(0);
				input_chd.file().tell(filepos);
				report_error(1, "Error reading hunk %d at offset %d from CHD file (%s): %s\n", framenum, filepos, *params.find(OPTION_INPUT)->second, err.message());
			}

			// write audio
			for (int chnum = 0; chnum < channels; chnum++)
			{
				avi_file::error avierr = output_file->append_sound_samples(chnum, avconfig.audio[chnum], actsamples, 0);
				if (avierr != avi_file::error::NONE)
					report_error(1, "Error writing samples for hunk %d to file (%s): %s\n", framenum, *output_file_str->second, avi_file::error_string(avierr));
			}

			// write video
			if ((framenum + 1) % interlace_factor == 0)
			{
				avi_file::error avierr = output_file->append_video_frame(fullbitmap);
				if (avierr != avi_file::error::NONE)
					report_error(1, "Error writing video for hunk %d to file (%s): %s\n", framenum, *output_file_str->second, avi_file::error_string(avierr));
			}
		}

		// close and return
		output_file.reset();
		util::stream_format(std::cout, "Extraction complete                                    \n");
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
		std::error_condition const filerr = util::core_file::load(*file_str->second, file);
		if (filerr)
			report_error(1, "Error reading metadata file (%s): %s", *file_str->second, filerr.message());
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
	util::stream_format(std::cout, "Input file:   %s\n", *params.find(OPTION_INPUT)->second);
	util::stream_format(std::cout, "Tag:          %c%c%c%c\n", (tag >> 24) & 0xff, (tag >> 16) & 0xff, (tag >> 8) & 0xff, tag & 0xff);
	util::stream_format(std::cout, "Index:        %d\n", index);
	if (text_str != params.end())
		util::stream_format(std::cout, "Text:         %s\n", text);
	else
		util::stream_format(std::cout, "Data:         %s (%d bytes)\n", *file_str->second, file.size());

	// write the metadata
	std::error_condition err;
	if (text_str != params.end())
		err = input_chd.write_metadata(tag, index, text, flags);
	else
		err = input_chd.write_metadata(tag, index, file, flags);
	if (err)
		report_error(1, "Error adding metadata: %s", err.message());
	else
		util::stream_format(std::cout, "Metadata added\n");
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
	util::stream_format(std::cout, "Input file:   %s\n", *params.find(OPTION_INPUT)->second);
	util::stream_format(std::cout, "Tag:          %c%c%c%c\n", (tag >> 24) & 0xff, (tag >> 16) & 0xff, (tag >> 8) & 0xff, tag & 0xff);
	util::stream_format(std::cout, "Index:        %d\n", index);

	// write the metadata
	std::error_condition err = input_chd.delete_metadata(tag, index);
	if (err)
		report_error(1, "Error removing metadata: %s", err.message());
	else
		util::stream_format(std::cout, "Metadata removed\n");
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
		check_existing_output_file(params, *output_file_str->second);

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
	std::error_condition err = input_chd.read_metadata(tag, index, buffer);
	if (err)
		report_error(1, "Error reading metadata: %s", err.message());

	// catch errors so we can close & delete the output file
	util::core_file::ptr output_file;
	try
	{
		// create the file
		if (output_file_str != params.end())
		{
			std::error_condition filerr;

			filerr = util::core_file::open(*output_file_str->second, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, output_file);
			if (filerr)
				report_error(1, "Unable to open file (%s): %s", *output_file_str->second, filerr.message());

			// output the metadata
			size_t count;
			std::tie(filerr, count) = write(*output_file, &buffer[0], buffer.size());
			if (!filerr)
				filerr = output_file->flush();
			if (filerr)
				report_error(1, "Error writing file (%s)", *output_file_str->second);
			output_file.reset();

			// provide some feedback
			util::stream_format(std::cout, "File (%s) written, %s bytes\n", *output_file_str->second, big_int_string(buffer.size()));
		}
		else
		{
			// flush to stdout
			// FIXME: check for errors
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
//  do_list_templates - list hard drive templates
//-------------------------------------------------

static void do_list_templates(parameters_map &params)
{
	util::stream_format(std::cout, "\n");
	util::stream_format(std::cout, "ID  Manufacturer  Model           Cylinders  Heads  Sectors  Sector Size  Total Size\n");
	util::stream_format(std::cout, "------------------------------------------------------------------------------------\n");

	for (int id = 0; id < std::size(s_hd_templates); id++)
	{
		util::stream_format(std::cout, "%2d  %-13s %-15s %9d  %5d  %7d  %11d  %7d MB\n",
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
	chdman_osd_output osdoutput;

	// print the header
	extern const char build_version[];
	util::stream_format(std::cout, "chdman - MAME Compressed Hunks of Data (CHD) manager %s\n", build_version);

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
	{
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
				for (valid = 0; (valid < std::size(desc.valid_options)) && desc.valid_options[valid]; valid++)
				{
					// reduce to the option name
					const char *validname = desc.valid_options[valid];
					if (*validname == REQUIRED[0])
						validname++;

					// find the matching option description
					int optnum;
					for (optnum = 0; optnum < std::size(s_options); optnum++)
						if (strcmp(s_options[optnum].name, validname) == 0)
							break;
					assert(optnum != std::size(s_options));

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
				if ((valid == std::size(desc.valid_options)) || !desc.valid_options[valid])
					return print_help(args[0], desc, string_format("Option '%s' not valid for this command", arg).c_str());
			}

			// make sure we got all our required parameters
			for (int valid = 0; valid < std::size(desc.valid_options); valid++)
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
			catch (std::error_condition const &err)
			{
				util::stream_format(std::cerr, "CHD error occurred (main): %s\n", err.message());
				return 1;
			}
			catch (fatal_error &err)
			{
				util::stream_format(std::cerr, "Fatal error occurred: %d\n", err.error());
				return err.error();
			}
			catch (std::exception& ex)
			{
				util::stream_format(std::cerr, "Unhandled exception: %s\n", ex.what());
				return 1;
			}
		}
	}

	// print generic help if nothing found
	return print_help(args[0]);
}
