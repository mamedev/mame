// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    CHD compression frontend

****************************************************************************/

#include <assert.h>

#include "osdcore.h"
#include "corefile.h"
#include "chdcd.h"
#include "aviio.h"
#include "avhuff.h"
#include "bitmap.h"
#include "md5.h"
#include "sha1.h"
#include "vbiparse.h"
#include "tagmap.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <new>



//**************************************************************************
//  CONSTANTS & DEFINES
//**************************************************************************

// default hard disk sector size
const UINT32 IDE_SECTOR_SIZE = 512;

// temporary input buffer size
const UINT32 TEMP_BUFFER_SIZE = 32 * 1024 * 1024;

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


//**************************************************************************
//  FUNCTION PROTOTYPES
//**************************************************************************

typedef tagmap_t<std::string *> parameters_t;

static void report_error(int error, const char *format, ...) ATTR_PRINTF(2,3);
static void do_info(parameters_t &params);
static void do_verify(parameters_t &params);
static void do_create_raw(parameters_t &params);
static void do_create_hd(parameters_t &params);
static void do_create_cd(parameters_t &params);
static void do_create_ld(parameters_t &params);
static void do_copy(parameters_t &params);
static void do_extract_raw(parameters_t &params);
static void do_extract_cd(parameters_t &params);
static void do_extract_ld(parameters_t &params);
static void do_add_metadata(parameters_t &params);
static void do_del_metadata(parameters_t &params);
static void do_dump_metadata(parameters_t &params);



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
	void (*handler)(parameters_t &);
	const char *description;
	const char *valid_options[16];
};


// ======================> avi_info

struct avi_info
{
	UINT32 fps_times_1million;
	UINT32 width;
	UINT32 height;
	bool interlaced;
	UINT32 channels;
	UINT32 rate;
	UINT32 max_samples_per_frame;
	UINT32 bytes_per_frame;
};


// ======================> metadata_index_info

struct metadata_index_info
{
	chd_metadata_tag    tag;
	UINT32              index;
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


// ======================> chd_rawfile_compressor

class chd_rawfile_compressor : public chd_file_compressor
{
public:
	// construction/destruction
	chd_rawfile_compressor(core_file *file, UINT64 offset = 0, UINT64 maxoffset = ~0)
		: m_file(file),
			m_offset(offset),
			m_maxoffset(MIN(maxoffset, (file != NULL) ? core_fsize(file) : 0)) { }

	// read interface
	virtual UINT32 read_data(void *dest, UINT64 offset, UINT32 length)
	{
		offset += m_offset;
		if (offset >= m_maxoffset)
			return 0;
		if (offset + length > m_maxoffset)
			length = m_maxoffset - offset;
		core_fseek(m_file, offset, SEEK_SET);
		return core_fread(m_file, dest, length);
	}

private:
	// internal state
	core_file *     m_file;
	UINT64          m_offset;
	UINT64          m_maxoffset;
};


// ======================> chd_chdfile_compressor

class chd_chdfile_compressor : public chd_file_compressor
{
public:
	// construction/destruction
	chd_chdfile_compressor(chd_file &file, UINT64 offset = 0, UINT64 maxoffset = ~0)
		: m_toc(NULL),
			m_file(file),
			m_offset(offset),
			m_maxoffset(MIN(maxoffset, file.logical_bytes())) { }

	// read interface
	virtual UINT32 read_data(void *dest, UINT64 offset, UINT32 length)
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
			UINT8 *_dest = reinterpret_cast<UINT8 *>(dest);

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
				for (UINT32 swapindex = dataoffset; swapindex < (dataoffset + CD_MAX_SECTOR_DATA); swapindex += 2)
				{
					UINT8 temp = _dest[swapindex];
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
	UINT64          m_offset;
	UINT64          m_maxoffset;
};


// ======================> chd_cd_compressor

class chd_cd_compressor : public chd_file_compressor
{
public:
	// construction/destruction
	chd_cd_compressor(cdrom_toc &toc, chdcd_track_input_info &info)
		: m_file(NULL),
			m_toc(toc),
			m_info(info) { }

	~chd_cd_compressor()
	{
		if (m_file != NULL)
			core_fclose(m_file);
	}

	// read interface
	virtual UINT32 read_data(void *_dest, UINT64 offset, UINT32 length)
	{
		// verify assumptions made below
		assert(offset % CD_FRAME_SIZE == 0);
		assert(length % CD_FRAME_SIZE == 0);

		// initialize destination to 0 so that unused areas are filled
		UINT8 *dest = reinterpret_cast<UINT8 *>(_dest);
		memset(dest, 0, length);

		// find out which track we're starting in
		UINT64 startoffs = 0;
		UINT32 length_remaining = length;
		for (int tracknum = 0; tracknum < m_toc.numtrks; tracknum++)
		{
			const cdrom_track_info &trackinfo = m_toc.tracks[tracknum];
			UINT64 endoffs = startoffs + (trackinfo.frames + trackinfo.extraframes) * CD_FRAME_SIZE;
			if (offset >= startoffs && offset < endoffs)
			{
				// if we don't already have this file open, open it now
				if (m_file == NULL || m_lastfile.compare(m_info.track[tracknum].fname)!=0)
				{
					if (m_file != NULL)
						core_fclose(m_file);
					m_lastfile = m_info.track[tracknum].fname;
					file_error filerr = core_fopen(m_lastfile.c_str(), OPEN_FLAG_READ, &m_file);
					if (filerr != FILERR_NONE)
						report_error(1, "Error opening input file (%s)'", m_lastfile.c_str());
				}

				// iterate over frames
				UINT32 bytesperframe = trackinfo.datasize + trackinfo.subsize;
				UINT64 src_track_start = m_info.track[tracknum].offset;
				UINT64 src_track_end = src_track_start + bytesperframe * trackinfo.frames;
				UINT64 pad_track_start = src_track_end - (m_toc.tracks[tracknum].padframes * bytesperframe);
				while (length_remaining != 0 && offset < endoffs)
				{
					// determine start of current frame
					UINT64 src_frame_start = src_track_start + ((offset - startoffs) / CD_FRAME_SIZE) * bytesperframe;
					if (src_frame_start < src_track_end)
					{
						// read it in, or pad if we're into the padframes
						if (src_frame_start >= pad_track_start)
						{
							memset(dest, 0, bytesperframe);
						}
						else
						{
							core_fseek(m_file, src_frame_start, SEEK_SET);
							UINT32 count = core_fread(m_file, dest, bytesperframe);
							if (count != bytesperframe)
								report_error(1, "Error reading input file (%s)'", m_lastfile.c_str());
						}

						// swap if appropriate
						if (m_info.track[tracknum].swap)
							for (UINT32 swapindex = 0; swapindex < 2352; swapindex += 2)
							{
								UINT8 temp = dest[swapindex];
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
	core_file *                 m_file;
	cdrom_toc &                 m_toc;
	chdcd_track_input_info &    m_info;
};


// ======================> chd_avi_compressor

class chd_avi_compressor : public chd_file_compressor
{
public:
	// construction/destruction
	chd_avi_compressor(avi_file &file, avi_info &info, UINT32 first_frame, UINT32 num_frames)
		: m_file(file),
			m_info(info),
			m_bitmap(info.width, info.height * (info.interlaced ? 2 : 1)),
			m_start_frame(first_frame),
			m_frame_count(num_frames),
			m_ldframedata(num_frames * VBI_PACKED_BYTES),
			m_rawdata(info.bytes_per_frame) { }

	// getters
	const dynamic_buffer &ldframedata() const { return m_ldframedata; }

	// read interface
	virtual UINT32 read_data(void *_dest, UINT64 offset, UINT32 length)
	{
		UINT8 *dest = reinterpret_cast<UINT8 *>(_dest);
		UINT8 interlace_factor = m_info.interlaced ? 2 : 1;
		UINT32 length_remaining = length;

		// iterate over frames
		INT32 start_frame = offset / m_info.bytes_per_frame;
		INT32 end_frame = (offset + length - 1) / m_info.bytes_per_frame;
		for (INT32 framenum = start_frame; framenum <= end_frame; framenum++)
			if (framenum < m_frame_count)
			{
				// determine effective frame number and first/last samples
				INT32 effframe = m_start_frame + framenum;
				UINT32 first_sample = (UINT64(m_info.rate) * UINT64(effframe) * UINT64(1000000) + m_info.fps_times_1million - 1) / UINT64(m_info.fps_times_1million);
				UINT32 samples = (UINT64(m_info.rate) * UINT64(effframe + 1) * UINT64(1000000) + m_info.fps_times_1million - 1) / UINT64(m_info.fps_times_1million) - first_sample;

				// loop over channels and read the samples
				int channels = MIN(m_info.channels, ARRAY_LENGTH(m_audio));
				INT16 *samplesptr[ARRAY_LENGTH(m_audio)];
				for (int chnum = 0; chnum < channels; chnum++)
				{
					// read the sound samples
					m_audio[chnum].resize(samples);
					samplesptr[chnum] = &m_audio[chnum][0];
					avi_error avierr = avi_read_sound_samples(&m_file, chnum, first_sample, samples, &m_audio[chnum][0]);
					if (avierr != AVIERR_NONE)
						report_error(1, "Error reading audio samples %d-%d from channel %d: %s", first_sample, samples, chnum, avi_error_string(avierr));
				}

				// read the video data
				avi_error avierr = avi_read_video_frame(&m_file, effframe / interlace_factor, m_bitmap);
				if (avierr != AVIERR_NONE)
					report_error(1, "Error reading AVI frame %d: %s", effframe / interlace_factor, avi_error_string(avierr));
				bitmap_yuy16 subbitmap(&m_bitmap.pix(effframe % interlace_factor), m_bitmap.width(), m_bitmap.height() / interlace_factor, m_bitmap.rowpixels() * interlace_factor);

				// update metadata for this frame
				if (m_info.height == 524/2 || m_info.height == 624/2)
				{
					vbi_metadata vbi;
					vbi_parse_all(&subbitmap.pix16(0), subbitmap.rowpixels(), subbitmap.width(), 8, &vbi);
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
				UINT64 start_offset = UINT64(framenum) * UINT64(m_info.bytes_per_frame);
				UINT64 end_offset = start_offset + m_info.bytes_per_frame;
				UINT32 bytes_to_copy = MIN(length_remaining, end_offset - offset);
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
	UINT32                      m_start_frame;
	UINT32                      m_frame_count;
	std::vector<INT16>        m_audio[8];
	dynamic_buffer              m_ldframedata;
	dynamic_buffer              m_rawdata;
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
	}
};



//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  report_error - report an error
//-------------------------------------------------

static void report_error(int error, const char *format, ...)
{
	// output to stderr
	va_list arg;
	va_start(arg, format);
	vfprintf(stderr, format, arg);
	fflush(stderr);
	va_end(arg);
	fprintf(stderr, "\n");

	// reset time for progress and return the error
	lastprogress = 0;
	throw fatal_error(error);
}


//-------------------------------------------------
//  progress - generic progress callback
//-------------------------------------------------

static void ATTR_PRINTF(2,3) progress(bool forceit, const char *format, ...)
{
	// skip if it hasn't been long enough
	clock_t curtime = clock();
	if (!forceit && lastprogress != 0 && curtime - lastprogress < CLOCKS_PER_SEC / 2)
		return;
	lastprogress = curtime;

	// standard vfprintf stuff here
	va_list arg;
	va_start(arg, format);
	vfprintf(stderr, format, arg);
	fflush(stderr);
	va_end(arg);
}


//-------------------------------------------------
//  print_help - print help for all the commands
//-------------------------------------------------

static int print_help(const char *argv0, const char *error = NULL)
{
	// print the error message first
	if (error != NULL)
		fprintf(stderr, "Error: %s\n\n", error);

	// print a summary of each command
	printf("Usage:\n");
	for (int cmdnum = 0; cmdnum < ARRAY_LENGTH(s_commands); cmdnum++)
	{
		const command_description &desc = s_commands[cmdnum];
		printf("   %s %s%s\n", argv0, desc.name, desc.description);
	}
	printf("\nFor help with any command, run:\n");
	printf("   %s %s <command>\n", argv0, COMMAND_HELP);
	return 1;
}


//-------------------------------------------------
//  print_help - print help for all a specific
//  command
//-------------------------------------------------

static int print_help(const char *argv0, const command_description &desc, const char *error = NULL)
{
	// print the error message first
	if (error != NULL)
		fprintf(stderr, "Error: %s\n\n", error);

	// print usage for this command
	printf("Usage:\n");
	printf("   %s %s [options], where valid options are:\n", argv0, desc.name);
	for (int valid = 0; valid < ARRAY_LENGTH(desc.valid_options); valid++)
	{
		// determine whether we are required
		const char *option = desc.valid_options[valid];
		if (option == NULL)
			break;
		bool required = (option[0] == REQUIRED[0]);
		if (required)
			option++;

		// find the option
		for (int optnum = 0; optnum < ARRAY_LENGTH(s_options); optnum++)
			if (strcmp(option, s_options[optnum].name) == 0)
			{
				const option_description &odesc = s_options[optnum];
				printf("      --%s", odesc.name);
				if (odesc.shortname != NULL)
					printf(", -%s", odesc.shortname);
				printf("%s%s\n", odesc.description, required ? " (required)" : "");
			}
	}
	return 1;
}


//-------------------------------------------------
//  big_int_string - create a 64-bit string
//-------------------------------------------------

const char *big_int_string(std::string &str, UINT64 intvalue)
{
	// 0 is a special case
	if (intvalue == 0)
		return str.assign("0").c_str();

	// loop until all chunks are done
	str.clear();
	bool first = true;
	while (intvalue != 0)
	{
		int chunk = intvalue % 1000;
		intvalue /= 1000;

		std::string insert;
		strprintf(insert, (intvalue != 0) ? "%03d" : "%d", chunk);

		if (!first)
			str.insert(0, ",").c_str();
		first = false;
		str.insert(0, insert);
	}
	return str.c_str();
}


//-------------------------------------------------
//  msf_string_from_frames - output the given
//  number of frames in M:S:F format
//-------------------------------------------------

const char *msf_string_from_frames(std::string &str, UINT32 frames)
{
	strprintf(str, "%02d:%02d:%02d", frames / (75 * 60), (frames / 75) % 60, frames % 75);
	return str.c_str();
}


//-------------------------------------------------
//  parse_number - parse a number string with an
//  optional k/m/g suffix
//-------------------------------------------------

UINT64 parse_number(const char *string)
{
	// 0-length string is 0
	int length = strlen(string);
	if (length == 0)
		return 0;

	// scan forward over digits
	UINT64 result = 0;
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

static void guess_chs(std::string *filename, UINT64 filesize, int sectorsize, UINT32 &cylinders, UINT32 &heads, UINT32 &sectors, UINT32 &bps)
{
	// if this is a direct physical drive read, handle it specially
	if (filename != NULL && osd_get_physical_drive_geometry(filename->c_str(), &cylinders, &heads, &sectors, &bps))
		return;

	// if we have no length to work with, we can't guess
	if (filesize == 0)
		report_error(1, "Can't guess CHS values because there is no input file");

	// now find a valid value
	for (UINT32 totalsectors = filesize / sectorsize; ; totalsectors++)
		for (UINT32 cursectors = 63; cursectors > 1; cursectors--)
			if (totalsectors % cursectors == 0)
			{
				UINT32 totalheads = totalsectors / cursectors;
				for (UINT32 curheads = 16; curheads > 1; curheads--)
					if (totalheads % curheads == 0)
					{
						cylinders = totalheads / curheads;
						heads = curheads;
						sectors = cursectors;
						return;
					}
			}

	// ack, it didn't work!
	report_error(1, "Can't guess CHS values because no logical combination works!");
}


//-------------------------------------------------
//  parse_input_chd_parameters - parse the
//  standard set of input CHD parameters
//-------------------------------------------------

static void parse_input_chd_parameters(const parameters_t &params, chd_file &input_chd, chd_file &input_parent_chd, bool writeable = false)
{
	// process input parent file
	std::string *input_chd_parent_str = params.find(OPTION_INPUT_PARENT);
	if (input_chd_parent_str != NULL)
	{
		chd_error err = input_parent_chd.open(input_chd_parent_str->c_str());
		if (err != CHDERR_NONE)
			report_error(1, "Error opening parent CHD file (%s): %s", input_chd_parent_str->c_str(), chd_file::error_string(err));
	}

	// process input file
	std::string *input_chd_str = params.find(OPTION_INPUT);
	if (input_chd_str != NULL)
	{
		chd_error err = input_chd.open(input_chd_str->c_str(), writeable, input_parent_chd.opened() ? &input_parent_chd : NULL);
		if (err != CHDERR_NONE)
			report_error(1, "Error opening CHD file (%s): %s", input_chd_str->c_str(), chd_file::error_string(err));
	}
}


//-------------------------------------------------
//  parse_input_start_end - parse input start/end
//  parameters in a standard way
//-------------------------------------------------

static void parse_input_start_end(const parameters_t &params, UINT64 logical_size, UINT32 hunkbytes, UINT32 framebytes, UINT64 &input_start, UINT64 &input_end)
{
	// process start/end if we were provided an input CHD
	input_start = 0;
	input_end = logical_size;

	// process input start
	std::string *input_start_byte_str = params.find(OPTION_INPUT_START_BYTE);
	std::string *input_start_hunk_str = params.find(OPTION_INPUT_START_HUNK);
	std::string *input_start_frame_str = params.find(OPTION_INPUT_START_FRAME);
	if (input_start_byte_str != NULL)
		input_start = parse_number(input_start_byte_str->c_str());
	if (input_start_hunk_str != NULL)
		input_start = parse_number(input_start_hunk_str->c_str()) * hunkbytes;
	if (input_start_frame_str != NULL)
		input_start = parse_number(input_start_frame_str->c_str()) * framebytes;
	if (input_start >= input_end)
		report_error(1, "Input start offset greater than input file size");

	// process input length
	std::string *input_length_bytes_str = params.find(OPTION_INPUT_LENGTH_BYTES);
	std::string *input_length_hunks_str = params.find(OPTION_INPUT_LENGTH_HUNKS);
	std::string *input_length_frames_str = params.find(OPTION_INPUT_LENGTH_FRAMES);
	UINT64 input_length = input_end;
	if (input_length_bytes_str != NULL)
		input_length = parse_number(input_length_bytes_str->c_str());
	if (input_length_hunks_str != NULL)
		input_length = parse_number(input_length_hunks_str->c_str()) * hunkbytes;
	if (input_length_frames_str != NULL)
		input_length = parse_number(input_length_frames_str->c_str()) * framebytes;
	if (input_start + input_length < input_end)
		input_end = input_start + input_length;
}


//-------------------------------------------------
//  check_existing_output_file - see if an output
//  file already exists, and error if it does,
//  unless --force is specified
//-------------------------------------------------

static void check_existing_output_file(const parameters_t &params, const char *filename)
{
	if (params.find(OPTION_OUTPUT_FORCE) == NULL)
	{
		core_file *file;
		file_error filerr = core_fopen(filename, OPEN_FLAG_READ, &file);
		if (filerr == FILERR_NONE)
		{
			core_fclose(file);
			report_error(1, "Error: file already exists (%s)\nUse --force (or -f) to force overwriting", filename);
		}
	}
}


//-------------------------------------------------
//  parse_output_chd_parameters - parse the
//  standard set of output CHD parameters
//-------------------------------------------------

static std::string *parse_output_chd_parameters(const parameters_t &params, chd_file &output_parent_chd)
{
	// process output parent file
	std::string *output_chd_parent_str = params.find(OPTION_OUTPUT_PARENT);
	if (output_chd_parent_str != NULL)
	{
		chd_error err = output_parent_chd.open(output_chd_parent_str->c_str());
		if (err != CHDERR_NONE)
			report_error(1, "Error opening parent CHD file (%s): %s", output_chd_parent_str->c_str(), chd_file::error_string(err));
	}

	// process output file
	std::string *output_chd_str = params.find(OPTION_OUTPUT);
	if (output_chd_str != NULL)
		check_existing_output_file(params, output_chd_str->c_str());
	return output_chd_str;
}


//-------------------------------------------------
//  parse_hunk_size - parse the hunk_size
//  parameter in a standard way
//-------------------------------------------------

static void parse_hunk_size(const parameters_t &params, UINT32 required_granularity, UINT32 &hunk_size)
{
	std::string *hunk_size_str = params.find(OPTION_HUNK_SIZE);
	if (hunk_size_str != NULL)
	{
		hunk_size = parse_number(hunk_size_str->c_str());
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

static void parse_compression(const parameters_t &params, chd_codec_type compression[4])
{
	// see if anything was specified
	std::string *compression_str = params.find(OPTION_COMPRESSION);
	if (compression_str == NULL)
		return;

	// special case: 'none'
	if (compression_str->compare("none")==0)
	{
		compression[0] = compression[1] = compression[2] = compression[3] = CHD_CODEC_NONE;
		return;
	}

	// iterate through compressors
	int index = 0;
	for (int start = 0, end = compression_str->find_first_of(','); index < 4; start = end + 1, end = compression_str->find_first_of(',', end + 1))
	{
		std::string name(*compression_str, start, (end == -1) ? -1 : end - start);
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

static void parse_numprocessors(const parameters_t &params)
{
	std::string *numprocessors_str = params.find(OPTION_NUMPROCESSORS);
	if (numprocessors_str == NULL)
		return;

	int count = atoi(numprocessors_str->c_str());
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

static const char *compression_string(std::string &str, chd_codec_type compression[4])
{
	// output compression types
	str.clear();
	if (compression[0] == CHD_CODEC_NONE)
		return str.assign("none").c_str();

	// iterate over types
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
	return str.c_str();
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

void output_track_metadata(int mode, core_file *file, int tracknum, const cdrom_track_info &info, const char *filename, UINT32 frameoffs, UINT64 discoffs)
{
	if (mode == MODE_GDI)
	{
		int mode = 0, size = 2048;

		switch (info.trktype)
		{
			case CD_TRACK_MODE1:
				mode = 0;
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
		bool needquote = strchr(filename, ' ') != NULL;
		core_fprintf(file, "%d %d %d %d %s%s%s %" I64FMT "d\n", tracknum+1, frameoffs, mode, size, needquote?"\"":"", filename, needquote?"\"":"", discoffs);
	}
	else if (mode == MODE_CUEBIN)
	{
		// first track specifies the file
		if (tracknum == 0)
			core_fprintf(file, "FILE \"%s\" BINARY\n", filename);

		// determine submode
		std::string tempstr;
		switch (info.trktype)
		{
			case CD_TRACK_MODE1:
			case CD_TRACK_MODE1_RAW:
				strprintf(tempstr,"MODE1/%04d", info.datasize);
				break;

			case CD_TRACK_MODE2:
			case CD_TRACK_MODE2_FORM1:
			case CD_TRACK_MODE2_FORM2:
			case CD_TRACK_MODE2_FORM_MIX:
			case CD_TRACK_MODE2_RAW:
				strprintf(tempstr,"MODE2/%04d", info.datasize);
				break;

			case CD_TRACK_AUDIO:
				tempstr.assign("AUDIO");
				break;
		}

		// output TRACK entry
		core_fprintf(file, "  TRACK %02d %s\n", tracknum + 1, tempstr.c_str());

		// output PREGAP tag if pregap sectors are not in the file
		if ((info.pregap > 0) && (info.pgdatasize == 0))
		{
			core_fprintf(file, "    PREGAP %s\n", msf_string_from_frames(tempstr, info.pregap));
			core_fprintf(file, "    INDEX 01 %s\n", msf_string_from_frames(tempstr, frameoffs));
		}
		else if ((info.pregap > 0) && (info.pgdatasize > 0))
		{
			core_fprintf(file, "    INDEX 00 %s\n", msf_string_from_frames(tempstr, frameoffs));
			core_fprintf(file, "    INDEX 01 %s\n", msf_string_from_frames(tempstr, frameoffs+info.pregap));
		}

		// if no pregap at all, output index 01 only
		if (info.pregap == 0)
		{
			core_fprintf(file, "    INDEX 01 %s\n", msf_string_from_frames(tempstr, frameoffs));
		}

		// output POSTGAP
		if (info.postgap > 0)
			core_fprintf(file, "    POSTGAP %s\n", msf_string_from_frames(tempstr, info.postgap));
	}
	// non-CUE mode
	else if (mode == MODE_NORMAL)
	{
		// header on the first track
		if (tracknum == 0)
			core_fprintf(file, "CD_ROM\n\n\n");
		core_fprintf(file, "// Track %d\n", tracknum + 1);

		// write out the track type
		std::string modesubmode;
		if (info.subtype != CD_SUB_NONE)
			strprintf(modesubmode,"%s %s", cdrom_get_type_string(info.trktype), cdrom_get_subtype_string(info.subtype));
		else
			strprintf(modesubmode,"%s", cdrom_get_type_string(info.trktype));
		core_fprintf(file, "TRACK %s\n", modesubmode.c_str());

		// write out the attributes
		core_fprintf(file, "NO COPY\n");
		if (info.trktype == CD_TRACK_AUDIO)
		{
			core_fprintf(file, "NO PRE_EMPHASIS\n");
			core_fprintf(file, "TWO_CHANNEL_AUDIO\n");
		}

		// output pregap
		std::string tempstr;
		if (info.pregap > 0)
			core_fprintf(file, "ZERO %s %s\n", modesubmode.c_str(), msf_string_from_frames(tempstr, info.pregap));

		// all tracks but the first one have a file offset
		if (tracknum > 0)
			core_fprintf(file, "DATAFILE \"%s\" #%d %s // length in bytes: %d\n", filename, UINT32(discoffs), msf_string_from_frames(tempstr, info.frames), info.frames * (info.datasize + info.subsize));
		else
			core_fprintf(file, "DATAFILE \"%s\" %s // length in bytes: %d\n", filename, msf_string_from_frames(tempstr, info.frames), info.frames * (info.datasize + info.subsize));

		// tracks with pregaps get a START marker too
		if (info.pregap > 0)
			core_fprintf(file, "START %s\n", msf_string_from_frames(tempstr, info.pregap));

		core_fprintf(file, "\n\n");
	}
}


//-------------------------------------------------
//  do_info - dump the header information from
//  a drive image
//-------------------------------------------------

static void do_info(parameters_t &params)
{
	// parse out input files
	chd_file input_parent_chd;
	chd_file input_chd;
	parse_input_chd_parameters(params, input_chd, input_parent_chd);

	// print filename and version
	std::string tempstr;
	printf("Input file:   %s\n", params.find(OPTION_INPUT)->c_str());
	printf("File Version: %d\n", input_chd.version());
	if (input_chd.version() < 3)
		report_error(1, "Unsupported version (%d); use an older chdman to upgrade to version 3 or later", input_chd.version());

	// output cmpression and size information
	chd_codec_type compression[4] = { input_chd.compression(0), input_chd.compression(1), input_chd.compression(2), input_chd.compression(3) };
	printf("Logical size: %s bytes\n", big_int_string(tempstr, input_chd.logical_bytes()));
	printf("Hunk Size:    %s bytes\n", big_int_string(tempstr, input_chd.hunk_bytes()));
	printf("Total Hunks:  %s\n", big_int_string(tempstr, input_chd.hunk_count()));
	printf("Unit Size:    %s bytes\n", big_int_string(tempstr, input_chd.unit_bytes()));
	printf("Total Units:  %s\n", big_int_string(tempstr, input_chd.unit_count()));
	printf("Compression:  %s\n", compression_string(tempstr, compression));
	printf("CHD size:     %s bytes\n", big_int_string(tempstr, core_fsize(input_chd)));
	if (compression[0] != CHD_CODEC_NONE)
		printf("Ratio:        %.1f%%\n", 100.0 * double(core_fsize(input_chd)) / double(input_chd.logical_bytes()));

	// add SHA1 output
	sha1_t overall = input_chd.sha1();
	if (overall != sha1_t::null)
	{
		printf("SHA1:         %s\n", overall.as_string(tempstr));
		if (input_chd.version() >= 4)
			printf("Data SHA1:    %s\n", input_chd.raw_sha1().as_string(tempstr));
	}
	sha1_t parent = input_chd.parent_sha1();
	if (parent != sha1_t::null)
		printf("Parent SHA1:  %s\n", parent.as_string(tempstr));

	// print out metadata
	dynamic_buffer buffer;
	std::vector<metadata_index_info> info;
	for (int index = 0; ; index++)
	{
		// get the indexed metadata item; stop when we hit an error
		chd_metadata_tag metatag;
		UINT8 metaflags;
		chd_error err = input_chd.read_metadata(CHDMETATAG_WILDCARD, index, buffer, metatag, metaflags);
		if (err != CHDERR_NONE)
			break;

		// determine our index
		UINT32 metaindex = ~0;
		for (unsigned int cur = 0; cur < info.size(); cur++)
			if (info[cur].tag == metatag)
			{
				metaindex = ++info[cur].index;
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

		// print up to 60 characters of metadata
		UINT32 count = MIN(60, buffer.size());
		for (int chnum = 0; chnum < count; chnum++)
			printf("%c", isprint(UINT8(buffer[chnum])) ? buffer[chnum] : '.');
		printf("\n");
	}

	// print compression stats if verbose
	if (params.find(OPTION_VERBOSE) != NULL)
	{
		UINT32 compression_types[10] = { 0 };
		for (UINT32 hunknum = 0; hunknum < input_chd.hunk_count(); hunknum++)
		{
			// get info on this hunk
			chd_codec_type codec;
			UINT32 compbytes;
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
				std::string tempstr;
				printf("%10s   %5.1f%%  %-40s\n",
						big_int_string(tempstr, compression_types[comptype]),
						100.0 * double(compression_types[comptype]) / double(input_chd.hunk_count()),
						name);
			}
	}
}


//-------------------------------------------------
//  do_verify - validate the SHA1 on a CHD
//-------------------------------------------------

static void do_verify(parameters_t &params)
{
	// parse out input files
	chd_file input_parent_chd;
	chd_file input_chd;
	parse_input_chd_parameters(params, input_chd, input_parent_chd);

	// only makes sense for compressed CHDs with valid SHA1's
	if (!input_chd.compressed())
		report_error(0, "No verification to be done; CHD is uncompressed");
	sha1_t raw_sha1 = (input_chd.version() <= 3) ? input_chd.sha1() : input_chd.raw_sha1();
	if (raw_sha1 == sha1_t::null)
		report_error(0, "No verification to be done; CHD has no checksum");

	// create an array to read into
	dynamic_buffer buffer((TEMP_BUFFER_SIZE / input_chd.hunk_bytes()) * input_chd.hunk_bytes());

	// read all the data and build up an SHA-1
	sha1_creator rawsha1;
	for (UINT64 offset = 0; offset < input_chd.logical_bytes(); )
	{
		progress(false, "Verifying, %.1f%% complete... \r", 100.0 * double(offset) / double(input_chd.logical_bytes()));

		// determine how much to read
		UINT32 bytes_to_read = MIN((UINT32)buffer.size(), input_chd.logical_bytes() - offset);
		chd_error err = input_chd.read_bytes(offset, &buffer[0], bytes_to_read);
		if (err != CHDERR_NONE)
			report_error(1, "Error reading CHD file (%s): %s", params.find(OPTION_INPUT)->c_str(), chd_file::error_string(err));

		// add to the checksum
		rawsha1.append(&buffer[0], bytes_to_read);
		offset += bytes_to_read;
	}
	sha1_t computed_sha1 = rawsha1.finish();

	// finish up
	if (raw_sha1 != computed_sha1)
	{
		std::string tempstr;
		fprintf(stderr, "Error: Raw SHA1 in header = %s\n", raw_sha1.as_string(tempstr));
		fprintf(stderr, "              actual SHA1 = %s\n", computed_sha1.as_string(tempstr));

		// fix it if requested; this also fixes the overall one so we don't need to do any more
		if (params.find(OPTION_FIX) != NULL)
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
			sha1_t computed_overall_sha1 = input_chd.compute_overall_sha1(computed_sha1);
			if (input_chd.sha1() == computed_overall_sha1)
				printf("Overall SHA1 verification successful!\n");
			else
			{
				std::string tempstr;
				fprintf(stderr, "Error: Overall SHA1 in header = %s\n", input_chd.sha1().as_string(tempstr));
				fprintf(stderr, "                  actual SHA1 = %s\n", computed_overall_sha1.as_string(tempstr));

				// fix it if requested
				if (params.find(OPTION_FIX) != NULL)
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

static void do_create_raw(parameters_t &params)
{
	// process input file
	core_file *input_file = NULL;
	std::string *input_file_str = params.find(OPTION_INPUT);
	if (input_file_str != NULL)
	{
		file_error filerr = core_fopen(input_file_str->c_str(), OPEN_FLAG_READ, &input_file);
		if (filerr != FILERR_NONE)
			report_error(1, "Unable to open file (%s)", input_file_str->c_str());
	}

	// process output CHD
	chd_file output_parent;
	std::string *output_chd_str = parse_output_chd_parameters(params, output_parent);

	// process hunk size
	UINT32 hunk_size = output_parent.opened() ? output_parent.hunk_bytes() : 0;
	parse_hunk_size(params, 1, hunk_size);

	// process unit size
	UINT32 unit_size = output_parent.opened() ? output_parent.unit_bytes() : 0;
	std::string *unit_size_str = params.find(OPTION_UNIT_SIZE);
	if (unit_size_str != NULL)
	{
		unit_size = parse_number(unit_size_str->c_str());
		if (hunk_size % unit_size != 0)
			report_error(1, "Unit size is not an even divisor of the hunk size");
	}

	// process input start/end (needs to know hunk_size)
	UINT64 input_start;
	UINT64 input_end;
	parse_input_start_end(params, core_fsize(input_file), hunk_size, hunk_size, input_start, input_end);

	// process compression
	chd_codec_type compression[4];
	memcpy(compression, s_default_raw_compression, sizeof(compression));
	parse_compression(params, compression);

	// process numprocessors
	parse_numprocessors(params);

	// print some info
	std::string tempstr;
	printf("Output CHD:   %s\n", output_chd_str->c_str());
	if (output_parent.opened())
		printf("Parent CHD:   %s\n", params.find(OPTION_OUTPUT_PARENT)->c_str());
	printf("Input file:   %s\n", input_file_str->c_str());
	if (input_start != 0 || input_end != core_fsize(input_file))
	{
		printf("Input start:  %s\n", big_int_string(tempstr, input_start));
		printf("Input length: %s\n", big_int_string(tempstr, input_end - input_start));
	}
	printf("Compression:  %s\n", compression_string(tempstr, compression));
	printf("Hunk size:    %s\n", big_int_string(tempstr, hunk_size));
	printf("Logical size: %s\n", big_int_string(tempstr, input_end - input_start));

	// catch errors so we can close & delete the output file
	chd_rawfile_compressor *chd = NULL;
	try
	{
		// create the new CHD
		chd = new chd_rawfile_compressor(input_file, input_start, input_end);
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
		delete chd;
	}
	catch (...)
	{
		delete chd;
		// delete the output file
		std::string *output_chd_str = params.find(OPTION_OUTPUT);
		if (output_chd_str != NULL)
			osd_rmfile(output_chd_str->c_str());
		throw;
	}
}


//-------------------------------------------------
//  do_create_hd - create a new compressed hard
//  disk image from a raw file
//-------------------------------------------------

static void do_create_hd(parameters_t &params)
{
	// process input file
	core_file *input_file = NULL;
	std::string *input_file_str = params.find(OPTION_INPUT);
	if (input_file_str != NULL)
	{
		file_error filerr = core_fopen(input_file_str->c_str(), OPEN_FLAG_READ, &input_file);
		if (filerr != FILERR_NONE)
			report_error(1, "Unable to open file (%s)", input_file_str->c_str());
	}

	// process output CHD
	chd_file output_parent;
	std::string *output_chd_str = parse_output_chd_parameters(params, output_parent);

	// process sectorsize
	UINT32 sector_size = output_parent.opened() ? output_parent.unit_bytes() : IDE_SECTOR_SIZE;
	std::string *sectorsize_str = params.find(OPTION_SECTOR_SIZE);
	if (sectorsize_str != NULL)
	{
		if (output_parent.opened())
			report_error(1, "Sector size does not apply when creating a diff from the parent");
		sector_size = parse_number(sectorsize_str->c_str());
	}

	// process hunk size (needs to know sector_size)
	UINT32 hunk_size = output_parent.opened() ? output_parent.hunk_bytes() : MAX((4096 / sector_size) * sector_size, sector_size);
	parse_hunk_size(params, sector_size, hunk_size);

	// process input start/end (needs to know hunk_size)
	UINT64 filesize = 0;
	UINT64 input_start = 0;
	UINT64 input_end = 0;
	if (input_file != NULL)
	{
		parse_input_start_end(params, core_fsize(input_file), hunk_size, hunk_size, input_start, input_end);
		filesize = input_end - input_start;
	}
	else
	{
		std::string *size_str = params.find(OPTION_SIZE);
		if (size_str != NULL)
		{
			if (sscanf(size_str->c_str(), "%" I64FMT"d", &filesize) != 1)
				report_error(1, "Invalid size string");
		}
	}

	// process compression
	chd_codec_type compression[4];
	memcpy(compression, s_default_hd_compression, sizeof(compression));
	if (input_file == NULL)
		compression[0] = compression[1] = compression[2] = compression[3] = CHD_CODEC_NONE;
	parse_compression(params, compression);
	if (input_file == NULL && compression[0] != CHD_CODEC_NONE)
		report_error(1, "Blank hard disks must be uncompressed");

	// process numprocessors
	parse_numprocessors(params);

	// process chs
	UINT32 cylinders = 0;
	UINT32 heads = 0;
	UINT32 sectors = 0;
	std::string *chs_str = params.find(OPTION_CHS);
	if (chs_str != NULL)
	{
		if (output_parent.opened())
			report_error(1, "CHS does not apply when creating a diff from the parent");
		if (sscanf(chs_str->c_str(), "%d,%d,%d", &cylinders, &heads, &sectors) != 3)
			report_error(1, "Invalid CHS string; must be of the form <cylinders>,<heads>,<sectors>");
	}

	// process ident
	dynamic_buffer identdata;
	if (output_parent.opened())
		output_parent.read_metadata(HARD_DISK_IDENT_METADATA_TAG, 0, identdata);
	std::string *ident_str = params.find(OPTION_IDENT);
	if (ident_str != NULL)
	{
		// load the file
		file_error filerr = core_fload(ident_str->c_str(), identdata);
		if (filerr != FILERR_NONE)
			report_error(1, "Error reading ident file (%s)", ident_str->c_str());

		// must be at least 14 bytes; extract CHS data from there
		if (identdata.size() < 14)
			report_error(1, "Ident file '%s' is invalid (too short)", ident_str->c_str());
		cylinders = (identdata[3] << 8) | identdata[2];
		heads = (identdata[7] << 8) | identdata[6];
		sectors = (identdata[13] << 8) | identdata[12];
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
		if (input_file == NULL && filesize == 0)
			report_error(1, "Blank hard drives must specify either a length or a set of CHS values");
		guess_chs(input_file_str, filesize, sector_size, cylinders, heads, sectors, sector_size);
	}
	UINT32 totalsectors = cylinders * heads * sectors;

	// print some info
	std::string tempstr;
	printf("Output CHD:   %s\n", output_chd_str->c_str());
	if (output_parent.opened())
		printf("Parent CHD:   %s\n", params.find(OPTION_OUTPUT_PARENT)->c_str());
	if (input_file != NULL)
	{
		printf("Input file:   %s\n", input_file_str->c_str());
		if (input_start != 0 || input_end != core_fsize(input_file))
		{
			printf("Input start:  %s\n", big_int_string(tempstr, input_start));
			printf("Input length: %s\n", big_int_string(tempstr, filesize));
		}
	}
	printf("Compression:  %s\n", compression_string(tempstr, compression));
	printf("Cylinders:    %d\n", cylinders);
	printf("Heads:        %d\n", heads);
	printf("Sectors:      %d\n", sectors);
	printf("Bytes/sector: %d\n", sector_size);
	printf("Sectors/hunk: %d\n", hunk_size / sector_size);
	printf("Logical size: %s\n", big_int_string(tempstr, UINT64(totalsectors) * UINT64(sector_size)));

	// catch errors so we can close & delete the output file
	chd_rawfile_compressor *chd = NULL;
	try
	{
		// create the new hard drive
		chd = new chd_rawfile_compressor(input_file, input_start, input_end);
		chd_error err;
		if (output_parent.opened())
			err = chd->create(output_chd_str->c_str(), UINT64(totalsectors) * UINT64(sector_size), hunk_size, compression, output_parent);
		else
			err = chd->create(output_chd_str->c_str(), UINT64(totalsectors) * UINT64(sector_size), hunk_size, sector_size, compression);
		if (err != CHDERR_NONE)
			report_error(1, "Error creating CHD file (%s): %s", output_chd_str->c_str(), chd_file::error_string(err));

		// add the standard hard disk metadata
		std::string metadata;
		strprintf(metadata, HARD_DISK_METADATA_FORMAT, cylinders, heads, sectors, sector_size);
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
		if (input_file != NULL)
			compress_common(*chd);
		delete chd;
	}
	catch (...)
	{
		delete chd;
		// delete the output file
		std::string *output_chd_str = params.find(OPTION_OUTPUT);
		if (output_chd_str != NULL)
			osd_rmfile(output_chd_str->c_str());
		throw;
	}
}


//-------------------------------------------------
//  do_create_cd - create a new compressed CD
//  image from a raw file
//-------------------------------------------------

static void do_create_cd(parameters_t &params)
{
	// process input file
	chdcd_track_input_info track_info;
	cdrom_toc toc = { 0 };
	std::string *input_file_str = params.find(OPTION_INPUT);
	if (input_file_str != NULL)
	{
		chd_error err = chdcd_parse_toc(input_file_str->c_str(), toc, track_info);
		if (err != CHDERR_NONE)
			report_error(1, "Error parsing input file (%s: %s)\n", input_file_str->c_str(), chd_file::error_string(err));
	}

	// process output CHD
	chd_file output_parent;
	std::string *output_chd_str = parse_output_chd_parameters(params, output_parent);

	// process hunk size
	UINT32 hunk_size = output_parent.opened() ? output_parent.hunk_bytes() : CD_FRAMES_PER_HUNK * CD_FRAME_SIZE;
	parse_hunk_size(params, CD_FRAME_SIZE, hunk_size);

	// process compression
	chd_codec_type compression[4];
	memcpy(compression, s_default_cd_compression, sizeof(compression));
	parse_compression(params, compression);

	// process numprocessors
	parse_numprocessors(params);

	// pad each track to a 4-frame boundry. cdrom.c will deal with this on the read side
	UINT32 origtotalsectors = 0;
	UINT32 totalsectors = 0;
	for (int tracknum = 0; tracknum < toc.numtrks; tracknum++)
	{
		cdrom_track_info &trackinfo = toc.tracks[tracknum];
		int padded = (trackinfo.frames + CD_TRACK_PADDING - 1) / CD_TRACK_PADDING;
		trackinfo.extraframes = padded * CD_TRACK_PADDING - trackinfo.frames;
		origtotalsectors += trackinfo.frames;
		totalsectors += trackinfo.frames + trackinfo.extraframes;
	}

	// print some info
	std::string tempstr;
	printf("Output CHD:   %s\n", output_chd_str->c_str());
	if (output_parent.opened())
		printf("Parent CHD:   %s\n", params.find(OPTION_OUTPUT_PARENT)->c_str());
	printf("Input file:   %s\n", input_file_str->c_str());
	printf("Input tracks: %d\n", toc.numtrks);
	printf("Input length: %s\n", msf_string_from_frames(tempstr, origtotalsectors));
	printf("Compression:  %s\n", compression_string(tempstr, compression));
	printf("Logical size: %s\n", big_int_string(tempstr, UINT64(totalsectors) * CD_FRAME_SIZE));

	// catch errors so we can close & delete the output file
	chd_cd_compressor *chd = NULL;
	try
	{
		// create the new CD
		chd = new chd_cd_compressor(toc, track_info);
		chd_error err;
		if (output_parent.opened())
			err = chd->create(output_chd_str->c_str(), UINT64(totalsectors) * UINT64(CD_FRAME_SIZE), hunk_size, compression, output_parent);
		else
			err = chd->create(output_chd_str->c_str(), UINT64(totalsectors) * UINT64(CD_FRAME_SIZE), hunk_size, CD_FRAME_SIZE, compression);
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
		std::string *output_chd_str = params.find(OPTION_OUTPUT);
		if (output_chd_str != NULL)
			osd_rmfile(output_chd_str->c_str());
		throw;
	}
}


//-------------------------------------------------
//  do_create_ld - create a new A/V file from an
//  input AVI file and metadata
//-------------------------------------------------

static void do_create_ld(parameters_t &params)
{
	// process input file
	avi_file *input_file = NULL;
	std::string *input_file_str = params.find(OPTION_INPUT);
	if (input_file_str != NULL)
	{
		avi_error avierr = avi_open(input_file_str->c_str(), &input_file);
		if (avierr != AVIERR_NONE)
			report_error(1, "Error opening AVI file (%s): %s\n", input_file_str->c_str(), avi_error_string(avierr));
	}
	const avi_movie_info *aviinfo = avi_get_movie_info(input_file);

	// process output CHD
	chd_file output_parent;
	std::string *output_chd_str = parse_output_chd_parameters(params, output_parent);

	// process input start/end
	UINT64 input_start;
	UINT64 input_end;
	parse_input_start_end(params, aviinfo->video_numsamples, 0, 1, input_start, input_end);

	// determine parameters of the incoming video stream
	avi_info info;
	info.fps_times_1million = UINT64(aviinfo->video_timescale) * 1000000 / aviinfo->video_sampletime;
	info.width = aviinfo->video_width;
	info.height = aviinfo->video_height;
	info.interlaced = ((info.fps_times_1million / 1000000) <= 30) && (info.height % 2 == 0) && (info.height > 288);
	info.channels = aviinfo->audio_channels;
	info.rate = aviinfo->audio_samplerate;

	// adjust for interlacing
	if (info.interlaced)
	{
		info.fps_times_1million *= 2;
		info.height /= 2;
		input_start *= 2;
		input_end *= 2;
	}

	// determine the number of bytes per frame
	info.max_samples_per_frame = (UINT64(info.rate) * 1000000 + info.fps_times_1million - 1) / info.fps_times_1million;
	info.bytes_per_frame = avhuff_encoder::raw_data_size(info.width, info.height, info.channels, info.max_samples_per_frame);

	// process hunk size
	UINT32 hunk_size = output_parent.opened() ? output_parent.hunk_bytes() : info.bytes_per_frame;
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
	std::string tempstr;
	printf("Output CHD:   %s\n", output_chd_str->c_str());
	if (output_parent.opened())
		printf("Parent CHD:   %s\n", params.find(OPTION_OUTPUT_PARENT)->c_str());
	printf("Input file:   %s\n", input_file_str->c_str());
	if (input_start != 0 && input_end != aviinfo->video_numsamples)
		printf("Input start:  %s\n", big_int_string(tempstr, input_start));
	printf("Input length: %s (%02d:%02d:%02d)\n", big_int_string(tempstr, input_end - input_start),
			UINT32((UINT64(input_end - input_start) * 1000000 / info.fps_times_1million / 60 / 60)),
			UINT32(((UINT64(input_end - input_start) * 1000000 / info.fps_times_1million / 60) % 60)),
			UINT32(((UINT64(input_end - input_start) * 1000000 / info.fps_times_1million) % 60)));
	printf("Frame rate:   %d.%06d\n", info.fps_times_1million / 1000000, info.fps_times_1million % 1000000);
	printf("Frame size:   %d x %d %s\n", info.width, info.height * (info.interlaced ? 2 : 1), info.interlaced ? "interlaced" : "non-interlaced");
	printf("Audio:        %d channels at %d Hz\n", info.channels, info.rate);
	printf("Compression:  %s\n", compression_string(tempstr, compression));
	printf("Hunk size:    %s\n", big_int_string(tempstr, hunk_size));
	printf("Logical size: %s\n", big_int_string(tempstr, UINT64(input_end - input_start) * hunk_size));

	// catch errors so we can close & delete the output file
	chd_avi_compressor *chd = NULL;
	try
	{
		// create the new CHD
		chd = new chd_avi_compressor(*input_file, info, input_start, input_end);
		chd_error err;
		if (output_parent.opened())
			err = chd->create(output_chd_str->c_str(), UINT64(input_end - input_start) * hunk_size, hunk_size, compression, output_parent);
		else
			err = chd->create(output_chd_str->c_str(), UINT64(input_end - input_start) * hunk_size, hunk_size, info.bytes_per_frame, compression);
		if (err != CHDERR_NONE)
			report_error(1, "Error creating CHD file (%s): %s", output_chd_str->c_str(), chd_file::error_string(err));

		// write the core A/V metadata
		std::string metadata;
		strprintf(metadata, AV_METADATA_FORMAT, info.fps_times_1million / 1000000, info.fps_times_1million % 1000000, info.width, info.height, info.interlaced, info.channels, info.rate);
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
		std::string *output_chd_str = params.find(OPTION_OUTPUT);
		if (output_chd_str != NULL)
			osd_rmfile(output_chd_str->c_str());
		throw;
	}
}


//-------------------------------------------------
//  do_copy - create a new CHD with data from
//  another CHD
//-------------------------------------------------

static void do_copy(parameters_t &params)
{
	// parse out input files
	chd_file input_parent_chd;
	chd_file input_chd;
	parse_input_chd_parameters(params, input_chd, input_parent_chd);

	// parse out input start/end
	UINT64 input_start;
	UINT64 input_end;
	parse_input_start_end(params, input_chd.logical_bytes(), input_chd.hunk_bytes(), input_chd.hunk_bytes(), input_start, input_end);

	// process output CHD
	chd_file output_parent;
	std::string *output_chd_str = parse_output_chd_parameters(params, output_parent);

	// process hunk size
	UINT32 hunk_size = input_chd.hunk_bytes();
	parse_hunk_size(params, 1, hunk_size);
	if (hunk_size % input_chd.hunk_bytes() != 0 && input_chd.hunk_bytes() % hunk_size != 0)
		report_error(1, "Hunk size is not an even multiple or divisor of input hunk size");

	// process compression; we default to our current preferences using metadata to pick the type
	chd_codec_type compression[4];
	{
		dynamic_buffer metadata;
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
	std::string tempstr;
	printf("Output CHD:   %s\n", output_chd_str->c_str());
	if (output_parent.opened())
		printf("Parent CHD:   %s\n", params.find(OPTION_OUTPUT_PARENT)->c_str());
	printf("Input CHD:    %s\n", params.find(OPTION_INPUT)->c_str());
	if (input_start != 0 || input_end != input_chd.logical_bytes())
	{
		printf("Input start:  %s\n", big_int_string(tempstr, input_start));
		printf("Input length: %s\n", big_int_string(tempstr, input_end - input_start));
	}
	printf("Compression:  %s\n", compression_string(tempstr, compression));
	printf("Hunk size:    %s\n", big_int_string(tempstr, hunk_size));
	printf("Logical size: %s\n", big_int_string(tempstr, input_end - input_start));

	// catch errors so we can close & delete the output file
	chd_chdfile_compressor *chd = NULL;
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
		dynamic_buffer metadata;
		chd_metadata_tag metatag;
		UINT8 metaflags;
		UINT32 index = 0;
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
			if (cdrom == NULL)
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
		std::string *output_chd_str = params.find(OPTION_OUTPUT);
		if (output_chd_str != NULL)
			osd_rmfile(output_chd_str->c_str());
		throw;
	}
}


//-------------------------------------------------
//  do_extract_raw - extract a raw file from a
//  CHD image
//-------------------------------------------------

static void do_extract_raw(parameters_t &params)
{
	// parse out input files
	chd_file input_parent_chd;
	chd_file input_chd;
	parse_input_chd_parameters(params, input_chd, input_parent_chd);

	// parse out input start/end
	UINT64 input_start;
	UINT64 input_end;
	parse_input_start_end(params, input_chd.logical_bytes(), input_chd.hunk_bytes(), input_chd.hunk_bytes(), input_start, input_end);

	// verify output file doesn't exist
	std::string *output_file_str = params.find(OPTION_OUTPUT);
	if (output_file_str != NULL)
		check_existing_output_file(params, output_file_str->c_str());

	// print some info
	std::string tempstr;
	printf("Output File:  %s\n", output_file_str->c_str());
	printf("Input CHD:    %s\n", params.find(OPTION_INPUT)->c_str());
	if (input_start != 0 || input_end != input_chd.logical_bytes())
	{
		printf("Input start:  %s\n", big_int_string(tempstr, input_start));
		printf("Input length: %s\n", big_int_string(tempstr, input_end - input_start));
	}

	// catch errors so we can close & delete the output file
	core_file *output_file = NULL;
	try
	{
		// process output file
		file_error filerr = core_fopen(output_file_str->c_str(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, &output_file);
		if (filerr != FILERR_NONE)
			report_error(1, "Unable to open file (%s)", output_file_str->c_str());

		// copy all data
		dynamic_buffer buffer((TEMP_BUFFER_SIZE / input_chd.hunk_bytes()) * input_chd.hunk_bytes());
		for (UINT64 offset = input_start; offset < input_end; )
		{
			progress(false, "Extracting, %.1f%% complete... \r", 100.0 * double(offset - input_start) / double(input_end - input_start));

			// determine how much to read
			UINT32 bytes_to_read = MIN((UINT32)buffer.size(), input_end - offset);
			chd_error err = input_chd.read_bytes(offset, &buffer[0], bytes_to_read);
			if (err != CHDERR_NONE)
				report_error(1, "Error reading CHD file (%s): %s", params.find(OPTION_INPUT)->c_str(), chd_file::error_string(err));

			// write to the output
			UINT32 count = core_fwrite(output_file, &buffer[0], bytes_to_read);
			if (count != bytes_to_read)
				report_error(1, "Error writing to file; check disk space (%s)", output_file_str->c_str());

			// advance
			offset += bytes_to_read;
		}

		// finish up
		core_fclose(output_file);
		printf("Extraction complete                                    \n");
	}
	catch (...)
	{
		// delete the output file
		if (output_file != NULL)
		{
			core_fclose(output_file);
			osd_rmfile(output_file_str->c_str());
		}
		throw;
	}
}


//-------------------------------------------------
//  do_extract_cd - extract a CD file from a
//  CHD image
//-------------------------------------------------

static void do_extract_cd(parameters_t &params)
{
	// parse out input files
	chd_file input_parent_chd;
	chd_file input_chd;
	parse_input_chd_parameters(params, input_chd, input_parent_chd);

	// further process input file
	cdrom_file *cdrom = cdrom_open(&input_chd);
	if (cdrom == NULL)
		report_error(1, "Unable to recognize CHD file as a CD");
	const cdrom_toc *toc = cdrom_get_toc(cdrom);

	// verify output file doesn't exist
	std::string *output_file_str = params.find(OPTION_OUTPUT);
	if (output_file_str != NULL)
		check_existing_output_file(params, output_file_str->c_str());

	// verify output BIN file doesn't exist
	std::string *output_bin_file_str = params.find(OPTION_OUTPUT_BIN);
	std::string default_name(*output_file_str);
	int chop = default_name.find_last_of('.');
	if (chop != -1)
		default_name.substr(0, chop);
	char basename[128];
	strncpy(basename, default_name.c_str(), 127);
	default_name.append(".bin");
	if (output_bin_file_str == NULL)
		output_bin_file_str = &default_name;
	check_existing_output_file(params, output_bin_file_str->c_str());

	// print some info
	std::string tempstr;
	printf("Output TOC:   %s\n", output_file_str->c_str());
	printf("Output Data:  %s\n", output_bin_file_str->c_str());
	printf("Input CHD:    %s\n", params.find(OPTION_INPUT)->c_str());

	// catch errors so we can close & delete the output file
	core_file *output_bin_file = NULL;
	core_file *output_toc_file = NULL;
	try
	{
		int mode = MODE_NORMAL;

		if (output_file_str->find(".cue") != -1)
		{
			mode = MODE_CUEBIN;
		}
		else if (output_file_str->find(".gdi") != -1)
		{
			mode = MODE_GDI;
		}

		// process output file
		file_error filerr = core_fopen(output_file_str->c_str(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_NO_BOM, &output_toc_file);
		if (filerr != FILERR_NONE)
			report_error(1, "Unable to open file (%s)", output_file_str->c_str());

		// process output BIN file
		if (mode != MODE_GDI)
		{
			filerr = core_fopen(output_bin_file_str->c_str(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, &output_bin_file);
			if (filerr != FILERR_NONE)
				report_error(1, "Unable to open file (%s)", output_bin_file_str->c_str());
		}

		// determine total frames
		UINT64 total_bytes = 0;
		for (int tracknum = 0; tracknum < toc->numtrks; tracknum++)
			total_bytes += toc->tracks[tracknum].frames * (toc->tracks[tracknum].datasize + toc->tracks[tracknum].subsize);

		// GDI must start with the # of tracks
		if (mode == MODE_GDI)
		{
			core_fprintf(output_toc_file, "%d\n", toc->numtrks);
		}

		// iterate over tracks and copy all data
		UINT64 outputoffs = 0;
		UINT32 discoffs = 0;
		dynamic_buffer buffer;
		for (int tracknum = 0; tracknum < toc->numtrks; tracknum++)
		{
			std::string trackbin_name(basename);

			if (mode == MODE_GDI)
			{
				char temp[8];
				sprintf(temp, "%02d", tracknum+1);
				trackbin_name.append(temp);
				if (toc->tracks[tracknum].trktype == CD_TRACK_AUDIO)
					trackbin_name.append(".raw");
				else
					trackbin_name.append(".bin");

				if (output_bin_file)
				{
					core_fclose(output_bin_file);
					output_bin_file = NULL;
				}

				filerr = core_fopen(trackbin_name.c_str(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, &output_bin_file);
				if (filerr != FILERR_NONE)
					report_error(1, "Unable to open file (%s)", trackbin_name.c_str());

				outputoffs = 0;
			}

			// output the metadata about the track to the TOC file
			const cdrom_track_info &trackinfo = toc->tracks[tracknum];
			std::string temp;
			if (mode == MODE_GDI)
			{
				output_track_metadata(mode, output_toc_file, tracknum, trackinfo, core_filename_extract_base(temp, trackbin_name.c_str()).c_str(), discoffs, outputoffs);
			}
			else
			{
				output_track_metadata(mode, output_toc_file, tracknum, trackinfo, core_filename_extract_base(temp, output_bin_file_str->c_str()).c_str(), discoffs, outputoffs);
			}

			// If this is bin/cue output and the CHD contains subdata, warn the user and don't include
			// the subdata size in the buffer calculation.
			UINT32 output_frame_size = trackinfo.datasize + ((trackinfo.subtype != CD_SUB_NONE) ? trackinfo.subsize : 0);
			if (trackinfo.subtype != CD_SUB_NONE && ((mode == MODE_CUEBIN) || (mode == MODE_GDI)))
			{
				printf("Warning: Track %d has subcode data.  bin/cue and gdi formats cannot contain subcode data and it will be omitted.\n", tracknum+1);
				printf("       : This may affect usage of the output image.  Use bin/toc output to keep all data.\n");
				output_frame_size = trackinfo.datasize;
			}

			// resize the buffer for the track
			buffer.resize((TEMP_BUFFER_SIZE / output_frame_size) * output_frame_size);

			// now read and output the actual data
			UINT32 bufferoffs = 0;
			UINT32 actualframes = trackinfo.frames - trackinfo.padframes;
			for (UINT32 frame = 0; frame < actualframes; frame++)
			{
				progress(false, "Extracting, %.1f%% complete... \r", 100.0 * double(outputoffs) / double(total_bytes));

				// read the data
				cdrom_read_data(cdrom, cdrom_get_track_start_phys(cdrom, tracknum) + frame, &buffer[bufferoffs], trackinfo.trktype, true);

				// for CDRWin and GDI audio tracks must be reversed
				// in the case of GDI and CHD version < 5 we assuming source CHD image is GDROM so audio tracks is already reversed
				if (((mode == MODE_GDI && input_chd.version() > 4) || (mode == MODE_CUEBIN)) && (trackinfo.trktype == CD_TRACK_AUDIO))
					for (int swapindex = 0; swapindex < trackinfo.datasize; swapindex += 2)
					{
						UINT8 swaptemp = buffer[bufferoffs + swapindex];
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
					core_fseek(output_bin_file, outputoffs, SEEK_SET);
					UINT32 byteswritten = core_fwrite(output_bin_file, &buffer[0], bufferoffs);
					if (byteswritten != bufferoffs)
						report_error(1, "Error writing frame %d to file (%s): %s\n", frame, output_file_str->c_str(), chd_file::error_string(CHDERR_WRITE_ERROR));
					outputoffs += bufferoffs;
					bufferoffs = 0;
				}
			}

			discoffs += trackinfo.padframes;
		}

		// finish up
		core_fclose(output_bin_file);
		core_fclose(output_toc_file);
		printf("Extraction complete                                    \n");
	}
	catch (...)
	{
		// delete the output files
		if (output_bin_file != NULL)
			core_fclose(output_bin_file);
		if (output_toc_file != NULL)
			core_fclose(output_toc_file);
		osd_rmfile(output_bin_file_str->c_str());
		osd_rmfile(output_file_str->c_str());
		throw;
	}
}


//-------------------------------------------------
//  do_extract_ld - extract an AVI file from a
//  CHD image
//-------------------------------------------------

static void do_extract_ld(parameters_t &params)
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
	UINT32 fps_times_1million;
	UINT32 max_samples_per_frame;
	UINT32 frame_bytes;
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
	UINT8 interlace_factor = interlaced ? 2 : 1;

	// determine key parameters and validate
	max_samples_per_frame = (UINT64(rate) * 1000000 + fps_times_1million - 1) / fps_times_1million;
	frame_bytes = avhuff_encoder::raw_data_size(width, height, channels, max_samples_per_frame);
	if (frame_bytes != input_chd.hunk_bytes())
		report_error(1, "Frame size does not match hunk size for this CHD");

	// parse out input start/end
	UINT64 input_start;
	UINT64 input_end;
	parse_input_start_end(params, input_chd.hunk_count() / interlace_factor, 0, 1, input_start, input_end);
	input_start *= interlace_factor;
	input_end *= interlace_factor;

	// build up the movie info
	avi_movie_info info;
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
	std::string *output_file_str = params.find(OPTION_OUTPUT);
	if (output_file_str != NULL)
		check_existing_output_file(params, output_file_str->c_str());

	// print some info
	std::string tempstr;
	printf("Output File:  %s\n", output_file_str->c_str());
	printf("Input CHD:    %s\n", params.find(OPTION_INPUT)->c_str());
	if (input_start != 0 || input_end != input_chd.hunk_count())
	{
		printf("Input start:  %s\n", big_int_string(tempstr, input_start));
		printf("Input length: %s\n", big_int_string(tempstr, input_end - input_start));
	}

	// catch errors so we can close & delete the output file
	avi_file *output_file = NULL;
	try
	{
		// process output file
		avi_error avierr = avi_create(output_file_str->c_str(), &info, &output_file);
		if (avierr != AVIERR_NONE)
			report_error(1, "Unable to open file (%s)", output_file_str->c_str());

		// create the codec configuration
		avhuff_decompress_config avconfig;
		std::vector<INT16> audio_data[16];
		UINT32 actsamples;
		avconfig.maxsamples = max_samples_per_frame;
		avconfig.actsamples = &actsamples;
		for (int chnum = 0; chnum < ARRAY_LENGTH(audio_data); chnum++)
		{
			audio_data[chnum].resize(max_samples_per_frame);
			avconfig.audio[chnum] = &audio_data[chnum][0];
		}

		// iterate over frames
		bitmap_yuy16 fullbitmap(width, height * interlace_factor);
		for (UINT64 framenum = input_start; framenum < input_end; framenum++)
		{
			progress(framenum == input_start, "Extracting, %.1f%% complete...  \r", 100.0 * double(framenum - input_start) / double(input_end - input_start));

			// set up the fake bitmap for this frame
			avconfig.video.wrap(&fullbitmap.pix(framenum % interlace_factor), fullbitmap.width(), fullbitmap.height() / interlace_factor, fullbitmap.rowpixels() * interlace_factor);
			input_chd.codec_configure(CHD_CODEC_AVHUFF, AVHUFF_CODEC_DECOMPRESS_CONFIG, &avconfig);

			// read the hunk into the buffers
			chd_error err = input_chd.read_hunk(framenum, NULL);
			if (err != CHDERR_NONE)
			{
				UINT64 filepos = core_ftell(input_chd);
				report_error(1, "Error reading hunk %" I64FMT "d at offset %" I64FMT "d from CHD file (%s): %s\n", framenum, filepos, params.find(OPTION_INPUT)->c_str(), chd_file::error_string(err));
			}

			// write audio
			for (int chnum = 0; chnum < channels; chnum++)
			{
				avi_error avierr = avi_append_sound_samples(output_file, chnum, avconfig.audio[chnum], actsamples, 0);
				if (avierr != AVIERR_NONE)
					report_error(1, "Error writing samples for hunk %" I64FMT "d to file (%s): %s\n", framenum, output_file_str->c_str(), avi_error_string(avierr));
			}

			// write video
			if ((framenum + 1) % interlace_factor == 0)
			{
				avi_error avierr = avi_append_video_frame(output_file, fullbitmap);
				if (avierr != AVIERR_NONE)
					report_error(1, "Error writing video for hunk %" I64FMT "d to file (%s): %s\n", framenum, output_file_str->c_str(), avi_error_string(avierr));
			}
		}

		// close and return
		avi_close(output_file);
		printf("Extraction complete                                    \n");
	}
	catch (...)
	{
		// delete the output file
		if (output_file != NULL)
			avi_close(output_file);
		osd_rmfile(output_file_str->c_str());
		throw;
	}
}


//-------------------------------------------------
//  do_add_metadata - add metadata to a CHD from a
//  file
//-------------------------------------------------

static void do_add_metadata(parameters_t &params)
{
	// parse out input files
	chd_file input_parent_chd;
	chd_file input_chd;
	parse_input_chd_parameters(params, input_chd, input_parent_chd, true);

	// process tag
	chd_metadata_tag tag = CHD_MAKE_TAG('?','?','?','?');
	std::string *tag_str = params.find(OPTION_TAG);
	if (tag_str != NULL)
	{
		tag_str->append("    ");
		tag = CHD_MAKE_TAG((*tag_str)[0], (*tag_str)[1], (*tag_str)[2], (*tag_str)[3]);
	}

	// process index
	UINT32 index = 0;
	std::string *index_str = params.find(OPTION_INDEX);
	if (index_str != NULL)
		index = atoi(index_str->c_str());

	// process text input
	std::string *text_str = params.find(OPTION_VALUE_TEXT);
	std::string text;
	if (text_str != NULL)
	{
		text = *text_str;
		if (text[0] == '"' && text[text.length() - 1] == '"')
			text.substr(1, text.length() - 2);
	}

	// process file input
	std::string *file_str = params.find(OPTION_VALUE_FILE);
	dynamic_buffer file;
	if (file_str != NULL)
	{
		file_error filerr = core_fload(file_str->c_str(), file);
		if (filerr != FILERR_NONE)
			report_error(1, "Error reading metadata file (%s)", file_str->c_str());
	}

	// make sure we have one or the other
	if (text_str == NULL && file_str == NULL)
		report_error(1, "Error: missing either --valuetext/-vt or --valuefile/-vf parameters");
	if (text_str != NULL && file_str != NULL)
		report_error(1, "Error: both --valuetext/-vt or --valuefile/-vf parameters specified; only one permitted");

	// process no checksum
	UINT8 flags = CHD_MDFLAGS_CHECKSUM;
	if (params.find(OPTION_NO_CHECKSUM) != NULL)
		flags &= ~CHD_MDFLAGS_CHECKSUM;

	// print some info
	std::string tempstr;
	printf("Input file:   %s\n", params.find(OPTION_INPUT)->c_str());
	printf("Tag:          %c%c%c%c\n", (tag >> 24) & 0xff, (tag >> 16) & 0xff, (tag >> 8) & 0xff, tag & 0xff);
	printf("Index:        %d\n", index);
	if (text_str != NULL)
		printf("Text:         %s\n", text.c_str());
	else
		printf("Data:         %s (%d bytes)\n", file_str->c_str(), int(file.size()));

	// write the metadata
	chd_error err;
	if (text_str != NULL)
		err = input_chd.write_metadata(tag, index, text, flags);
	else
		err = input_chd.write_metadata(tag, index, &file[0], flags);
	if (err != CHDERR_NONE)
		report_error(1, "Error adding metadata: %s", chd_file::error_string(err));
	else
		printf("Metadata added\n");
}


//-------------------------------------------------
//  do_del_metadata - remove metadata from a CHD
//-------------------------------------------------

static void do_del_metadata(parameters_t &params)
{
	// parse out input files
	chd_file input_parent_chd;
	chd_file input_chd;
	parse_input_chd_parameters(params, input_chd, input_parent_chd, true);

	// process tag
	chd_metadata_tag tag = CHD_MAKE_TAG('?','?','?','?');
	std::string *tag_str = params.find(OPTION_TAG);
	if (tag_str != NULL)
	{
		tag_str->append("    ");
		tag = CHD_MAKE_TAG((*tag_str)[0], (*tag_str)[1], (*tag_str)[2], (*tag_str)[3]);
	}

	// process index
	UINT32 index = 0;
	std::string *index_str = params.find(OPTION_INDEX);
	if (index_str != NULL)
		index = atoi(index_str->c_str());

	// print some info
	std::string tempstr;
	printf("Input file:   %s\n", params.find(OPTION_INPUT)->c_str());
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

static void do_dump_metadata(parameters_t &params)
{
	// parse out input files
	chd_file input_parent_chd;
	chd_file input_chd;
	parse_input_chd_parameters(params, input_chd, input_parent_chd);

	// verify output file doesn't exist
	std::string *output_file_str = params.find(OPTION_OUTPUT);
	if (output_file_str != NULL)
		check_existing_output_file(params, output_file_str->c_str());

	// process tag
	chd_metadata_tag tag = CHD_MAKE_TAG('?','?','?','?');
	std::string *tag_str = params.find(OPTION_TAG);
	if (tag_str != NULL)
	{
		tag_str->append("    ");
		tag = CHD_MAKE_TAG((*tag_str)[0], (*tag_str)[1], (*tag_str)[2], (*tag_str)[3]);
	}

	// process index
	UINT32 index = 0;
	std::string *index_str = params.find(OPTION_INDEX);
	if (index_str != NULL)
		index = atoi(index_str->c_str());

	// write the metadata
	dynamic_buffer buffer;
	chd_error err = input_chd.read_metadata(tag, index, buffer);
	if (err != CHDERR_NONE)
		report_error(1, "Error reading metadata: %s", chd_file::error_string(err));

	// catch errors so we can close & delete the output file
	core_file *output_file = NULL;
	try
	{
		// create the file
		if (output_file_str != NULL)
		{
			file_error filerr = core_fopen(output_file_str->c_str(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, &output_file);
			if (filerr != FILERR_NONE)
				report_error(1, "Unable to open file (%s)", output_file_str->c_str());

			// output the metadata
			UINT32 count = core_fwrite(output_file, &buffer[0], buffer.size());
			if (count != buffer.size())
				report_error(1, "Error writing file (%s)", output_file_str->c_str());
			core_fclose(output_file);

			// provide some feedback
			std::string tempstr;
			printf("File (%s) written, %s bytes\n", output_file_str->c_str(), big_int_string(tempstr, buffer.size()));
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
		if (output_file != NULL)
			core_fclose(output_file);
		osd_rmfile(output_file_str->c_str());
		throw;
	}
}


//-------------------------------------------------
//  main - entry point
//-------------------------------------------------

int CLIB_DECL main(int argc, char *argv[])
{
	// print the header
	extern const char build_version[];
	printf("chdman - MAME Compressed Hunks of Data (CHD) manager %s\n", build_version);

	// handle help specially
	if (argc < 2)
		return print_help(argv[0]);
	int argnum = 1;
	const char *command = argv[argnum++];
	bool help = (strcmp(command, COMMAND_HELP) == 0);
	if (help)
	{
		if (argc <= 2)
			return print_help(argv[0]);
		command = argv[argnum++];
	}

	// iterate over commands to find our match
	for (int cmdnum = 0; cmdnum < ARRAY_LENGTH(s_commands); cmdnum++)
		if (strcmp(command, s_commands[cmdnum].name) == 0)
		{
			const command_description &desc = s_commands[cmdnum];

			// print help if that was requested
			if (help)
				return print_help(argv[0], desc);

			// otherwise, verify the parameters
			tagmap_t<std::string *> parameters;
			while (argnum < argc)
			{
				// should be an option name
				const char *arg = argv[argnum++];
				if (arg[0] != '-')
					return print_help(argv[0], desc, "Expected option, not parameter");

				// iterate over valid options
				int valid;
				for (valid = 0; valid < ARRAY_LENGTH(desc.valid_options); valid++)
				{
					// reduce to the option name
					const char *validname = desc.valid_options[valid];
					if (validname == NULL)
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
						(arg[1] != '-' && odesc.shortname != NULL && strcmp(odesc.shortname, &arg[1]) == 0))
					{
						// if we need a parameter, consume it
						const char *param = "";
						if (odesc.parameter)
						{
							if (argnum >= argc || argv[argnum][0] == '-')
								return print_help(argv[0], desc, "Option is missing parameter");
							param = argv[argnum++];
						}

						// add to the map
						if (parameters.add(odesc.name, new std::string(param)) == TMERR_DUPLICATE)
							return print_help(argv[0], desc, "Multiple parameters of the same type specified");
						break;
					}
				}

				// if not valid, error
				if (valid == ARRAY_LENGTH(desc.valid_options))
					return print_help(argv[0], desc, "Option not valid for this command");
			}

			// make sure we got all our required parameters
			for (int valid = 0; valid < ARRAY_LENGTH(desc.valid_options); valid++)
			{
				const char *validname = desc.valid_options[valid];
				if (validname == NULL)
					break;
				if (*validname == REQUIRED[0] && parameters.find(++validname) == NULL)
					return print_help(argv[0], desc, "Required parameters missing");
			}

			// all clear, run the command
			try
			{
				(*s_commands[cmdnum].handler)(parameters);
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
	return print_help(argv[0]);
}
