// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
/***************************************************************************

    aviio.c

    AVI movie format parsing helpers.

***************************************************************************/

#include <array>
#include <cassert>
#include <cstdlib>
#include <cstring>

#include "aviio.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/**
 * @def FILETYPE_READ
 *
 * @brief   A macro that defines filetype read.
 */

#define FILETYPE_READ           1

/**
 * @def FILETYPE_CREATE
 *
 * @brief   A macro that defines filetype create.
 */

#define FILETYPE_CREATE         2

/** @brief  Size of the maximum riff. */
#define MAX_RIFF_SIZE           (2UL * 1024 * 1024 * 1024 - 1024)   /* just under 2GB */
/** @brief  The maximum avi size in gigabytes. */
#define MAX_AVI_SIZE_IN_GB      (256)

/**
 * @def FOUR_GB
 *
 * @brief   A macro that defines four gigabytes.
 */

#define FOUR_GB                 (std::uint64_t(1) << 32)

/**
 * @def MAX_SOUND_CHANNELS
 *
 * @brief   A macro that defines maximum sound channels.
 */

#define MAX_SOUND_CHANNELS      2

/**
 * @def SOUND_BUFFER_MSEC
 *
 * @brief   A macro that defines sound buffer msec.
 */

#define SOUND_BUFFER_MSEC       2000        /* milliseconds of sound buffering */

/** @brief  The chunktype riff. */
#define CHUNKTYPE_RIFF          AVI_FOURCC('R','I','F','F')
/** @brief  List of chunktypes. */
#define CHUNKTYPE_LIST          AVI_FOURCC('L','I','S','T')
/** @brief  The chunktype junk. */
#define CHUNKTYPE_JUNK          AVI_FOURCC('J','U','N','K')
/** @brief  The chunktype avih. */
#define CHUNKTYPE_AVIH          AVI_FOURCC('a','v','i','h')
/** @brief  The chunktype strh. */
#define CHUNKTYPE_STRH          AVI_FOURCC('s','t','r','h')
/** @brief  The chunktype strf. */
#define CHUNKTYPE_STRF          AVI_FOURCC('s','t','r','f')
/** @brief  The first chunktype index. */
#define CHUNKTYPE_IDX1          AVI_FOURCC('i','d','x','1')
/** @brief  The chunktype indx. */
#define CHUNKTYPE_INDX          AVI_FOURCC('i','n','d','x')
/** @brief  The chunktype xxdb. */
#define CHUNKTYPE_XXDB          AVI_FOURCC(0x00,0x00,'d','b')
/** @brief  The chunktype xxdc. */
#define CHUNKTYPE_XXDC          AVI_FOURCC(0x00,0x00,'d','c')
/** @brief  The chunktype xxwb. */
#define CHUNKTYPE_XXWB          AVI_FOURCC(0x00,0x00,'w','b')
/** @brief  The chunktype ixxx. */
#define CHUNKTYPE_IXXX          AVI_FOURCC('i','x',0x00,0x00)
/** @brief  The chunktype xx mask. */
#define CHUNKTYPE_XX_MASK       AVI_FOURCC(0x00,0x00,0xff,0xff)

/** @brief  The listtype avi. */
#define LISTTYPE_AVI            AVI_FOURCC('A','V','I',' ')
/** @brief  The listtype avix. */
#define LISTTYPE_AVIX           AVI_FOURCC('A','V','I','X')
/** @brief  The listtype hdrl. */
#define LISTTYPE_HDRL           AVI_FOURCC('h','d','r','l')
/** @brief  The listtype strl. */
#define LISTTYPE_STRL           AVI_FOURCC('s','t','r','l')
/** @brief  The listtype movi. */
#define LISTTYPE_MOVI           AVI_FOURCC('m','o','v','i')

/** @brief  The streamtype vids. */
#define STREAMTYPE_VIDS         AVI_FOURCC('v','i','d','s')
/** @brief  The streamtype auds. */
#define STREAMTYPE_AUDS         AVI_FOURCC('a','u','d','s')

/** @brief  The handler bitmap. */
#define HANDLER_DIB             AVI_FOURCC('D','I','B',' ')
/** @brief  The handler hfyu. */
#define HANDLER_HFYU            AVI_FOURCC('h','f','y','u')

/* main AVI header files */

/**
 * @def AVIF_HASINDEX
 *
 * @brief   A macro that defines avif hasindex.
 */

#define AVIF_HASINDEX           0x00000010

/**
 * @def AVIF_MUSTUSEINDEX
 *
 * @brief   A macro that defines avif mustuseindex.
 */

#define AVIF_MUSTUSEINDEX       0x00000020

/**
 * @def AVIF_ISINTERLEAVED
 *
 * @brief   A macro that defines avif isinterleaved.
 */

#define AVIF_ISINTERLEAVED      0x00000100

/**
 * @def AVIF_COPYRIGHTED
 *
 * @brief   A macro that defines avif copyrighted.
 */

#define AVIF_COPYRIGHTED        0x00010000

/**
 * @def AVIF_WASCAPTUREFILE
 *
 * @brief   A macro that defines avif wascapturefile.
 */

#define AVIF_WASCAPTUREFILE     0x00020000

/* index definitions */

/**
 * @def AVI_INDEX_OF_INDEXES
 *
 * @brief   A macro that defines avi index of indexes.
 */

#define AVI_INDEX_OF_INDEXES    0x00

/**
 * @def AVI_INDEX_OF_CHUNKS
 *
 * @brief   A macro that defines avi index of chunks.
 */

#define AVI_INDEX_OF_CHUNKS     0x01

/**
 * @def AVI_INDEX_IS_DATA
 *
 * @brief   A macro that defines avi index is data.
 */

#define AVI_INDEX_IS_DATA       0x80

/**
 * @def AVI_INDEX_2FIELD
 *
 * @brief   A macro that defines avi index 2 field.
 */

#define AVI_INDEX_2FIELD        0x01

/* HuffYUV definitions */

/**
 * @def HUFFYUV_PREDICT_LEFT
 *
 * @brief   A macro that defines huffyuv predict left.
 */

#define HUFFYUV_PREDICT_LEFT     0

/**
 * @def HUFFYUV_PREDICT_GRADIENT
 *
 * @brief   A macro that defines huffyuv predict gradient.
 */

#define HUFFYUV_PREDICT_GRADIENT 1

/**
 * @def HUFFYUV_PREDICT_MEDIAN
 *
 * @brief   A macro that defines huffyuv predict median.
 */

#define HUFFYUV_PREDICT_MEDIAN   2

/**
 * @def HUFFYUV_PREDICT_DECORR
 *
 * @brief   A macro that defines huffyuv predict decorr.
 */

#define HUFFYUV_PREDICT_DECORR   0x40


namespace {
/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/**
 * @struct  avi_chunk
 *
 * @brief   An avi chunk.
 */

struct avi_chunk
{
	avi_chunk() : offset(0), size(0), type(0), listtype(0) { }

	std::uint64_t       offset;                 /* file offset of chunk header */
	std::uint64_t       size;                   /* size of this chunk */
	std::uint32_t       type;                   /* type of this chunk */
	std::uint32_t       listtype;               /* type of this list (if we are a list) */
};

/**
 * @struct  avi_chunk_list
 *
 * @brief   List of avi chunks.
 */

struct avi_chunk_list
{
	std::uint64_t       offset;                 /* offset in the file of header */
	std::uint32_t       length;                 /* length of the chunk including header */
};

/**
 * @struct  avi_stream
 *
 * @brief   An avi stream.
 */

class avi_stream
{
public:
	avi_stream()
		: m_type(0)
		, m_format(0)
		, m_rate(0)
		, m_scale(0)
		, m_samples(0)
		, m_chunks()
		, m_width(0)
		, m_height(0)
		, m_depth(0)
		, m_interlace(0)
		, m_huffyuv()
		, m_channels(0)
		, m_samplebits(0)
		, m_samplerate(0)
		, m_saved_strh_offset(0)
		, m_saved_indx_offset(0)
	{
	}

	void initialize_video(avi_file::movie_info const &info)
	{
		m_type = STREAMTYPE_VIDS;
		m_format = info.video_format;
		m_rate = info.video_timescale;
		m_scale = info.video_sampletime;
		m_width = info.video_width;
		m_height = info.video_height;
		m_depth = info.video_depth;
	}

	void initialize_audio(avi_file::movie_info const &info)
	{
		m_type = STREAMTYPE_AUDS;
		m_format = info.audio_format;
		m_rate = info.audio_timescale;
		m_scale = info.audio_sampletime;
		m_channels = info.audio_channels;
		m_samplebits = info.audio_samplebits;
		m_samplerate = info.audio_samplerate;
	}

	std::uint32_t type() const { return m_type; }
	std::uint32_t format() const { return m_format; }

	std::uint32_t rate() const { return m_rate; }
	std::uint32_t scale() const { return m_scale; }
	std::uint32_t samples() const { return m_samples; }
	void set_samples(std::uint32_t value) { m_samples = value; }
	std::uint32_t add_samples(std::uint32_t value) { return m_samples += value; }

	std::uint32_t chunks() const { return m_chunks.size(); }
	avi_chunk_list const &chunk(std::uint32_t index) const { assert(index < m_chunks.size()); return m_chunks[index]; }
	avi_chunk_list &chunk(std::uint32_t index) { assert(index < m_chunks.size()); return m_chunks[index]; }
	avi_file::error set_chunk_info(std::uint32_t index, std::uint64_t offset, std::uint32_t length);
	void set_chunks(std::uint32_t count) { assert(count <= m_chunks.size()); m_chunks.resize(count); }

	std::uint32_t width() const { return m_width; }
	std::uint32_t height() const { return m_height; }
	std::uint32_t depth() const { return m_depth; }

	std::uint16_t channels() const { return m_channels; }
	std::uint16_t samplebits() const { return m_samplebits; }
	std::uint32_t samplerate() const { return m_samplerate; }
	std::uint32_t bytes_per_sample() const { return (m_samplebits / 8) * m_channels; }

	avi_file::error set_strh_data(std::uint8_t const *data, std::uint32_t size);
	avi_file::error set_strf_data(std::uint8_t const *data, std::uint32_t size);

	std::uint64_t &saved_strh_offset() { return m_saved_strh_offset; }
	std::uint64_t &saved_indx_offset() { return m_saved_indx_offset; }

	// RGB helpers
	avi_file::error rgb32_compress_to_rgb(const bitmap_rgb32 &bitmap, std::uint8_t *data, std::uint32_t numbytes) const;

	// YUY helpers
	avi_file::error yuv_decompress_to_yuy16(const std::uint8_t *data, std::uint32_t numbytes, bitmap_yuy16 &bitmap) const;
	avi_file::error yuy16_compress_to_yuy(const bitmap_yuy16 &bitmap, std::uint8_t *data, std::uint32_t numbytes) const;

	// HuffYUV helpers
	avi_file::error huffyuv_decompress_to_yuy16(const std::uint8_t *data, std::uint32_t numbytes, bitmap_yuy16 &bitmap) const;

private:
	struct huffyuv_table
	{
		std::uint8_t        shift[256];             /* bit shift amounts */
		std::uint32_t       bits[256];              /* bit match values */
		std::uint32_t       mask[256];              /* bit mask values */
		std::uint16_t       baselookup[65536];      /* base lookup table */
		std::vector<std::uint16_t> extralookup;     /* extra lookup tables */
	};

	struct huffyuv_data
	{
		std::uint8_t        predictor;              /* predictor */
		huffyuv_table       table[3];               /* array of tables */
	};

	avi_file::error huffyuv_extract_tables(const std::uint8_t *chunkdata, std::uint32_t size);

	std::uint32_t       m_type;                 /* subtype of stream */
	std::uint32_t       m_format;               /* format of stream data */

	std::uint32_t       m_rate;                 /* timescale for stream */
	std::uint32_t       m_scale;                /* duration of one sample in the stream */
	std::uint32_t       m_samples;              /* number of samples */

	std::vector<avi_chunk_list> m_chunks;       /* list of chunks */

	std::uint32_t       m_width;                /* width of video */
	std::uint32_t       m_height;               /* height of video */
	std::uint32_t       m_depth;                /* depth of video */
	std::uint8_t        m_interlace;            /* interlace parameters */
	std::unique_ptr<huffyuv_data const> m_huffyuv; /* huffyuv decompression data */

	std::uint16_t       m_channels;             /* audio channels */
	std::uint16_t       m_samplebits;           /* audio bits per sample */
	std::uint32_t       m_samplerate;           /* audio sample rate */

	/* only used when creating */
	std::uint64_t       m_saved_strh_offset;    /* writeoffset of strh chunk */
	std::uint64_t       m_saved_indx_offset;    /* writeoffset of indx chunk */
};

/**
 * @class   avi_file_impl
 *
 * @brief   An avi file.
 */

class avi_file_impl : public avi_file
{
public:
	avi_file_impl(osd_file::ptr &&file, std::uint64_t length) : avi_file_impl()
	{
		m_file = std::move(file);
		m_type = FILETYPE_READ;

		// make a root atom
		m_rootchunk.offset = 0;
		m_rootchunk.size = length;
		m_rootchunk.type = 0;
		m_rootchunk.listtype = 0;
	}

	avi_file_impl(osd_file::ptr &&file, movie_info const &info) : avi_file_impl()
	{
		m_file = std::move(file);
		m_type = FILETYPE_CREATE;

		// copy the movie info
		m_info = info;
		m_info.video_numsamples = 0;
		m_info.audio_numsamples = 0;

		// initialize the video track
		m_streams.emplace_back();
		m_streams.back().initialize_video(m_info);

		// initialize the audio track
		if (m_info.audio_channels > 0)
		{
			m_streams.emplace_back();
			m_streams.back().initialize_audio(m_info);
		}
	}

	virtual ~avi_file_impl() override;

	virtual void printf_chunks() override;

	virtual movie_info const &get_movie_info() const override;
	virtual std::uint32_t first_sample_in_frame(std::uint32_t framenum) const override;

	virtual error read_video_frame(std::uint32_t framenum, bitmap_yuy16 &bitmap) override;
	virtual error read_sound_samples(int channel, std::uint32_t firstsample, std::uint32_t numsamples, std::int16_t *output) override;

	virtual error append_video_frame(bitmap_yuy16 &bitmap) override;
	virtual error append_video_frame(bitmap_rgb32 &bitmap) override;
	virtual error append_sound_samples(int channel, std::int16_t const *samples, std::uint32_t numsamples, std::uint32_t sampleskip) override;

	error read_movie_data();
	error write_initial_headers();
	error soundbuf_initialize();

private:
	avi_file_impl()
		: m_file()
		, m_type(0)
		, m_info({ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 })
		, m_tempbuffer()
		, m_streams()
		, m_rootchunk()
		, m_writeoffs(0)
		, m_riffbase(0)
		, m_chunkstack()
		, m_chunksp(0)
		, m_saved_movi_offset(0)
		, m_saved_avih_offset(0)
		, m_soundbuf()
		, m_soundbuf_samples(0)
		, m_soundbuf_chunks(0)
		, m_soundbuf_frames(0)
	{
		std::fill(std::begin(m_soundbuf_chansamples), std::end(m_soundbuf_chansamples), 0);
	}

	avi_stream *get_video_stream();
	avi_stream *get_audio_stream(int channel, int &offset);
	std::uint32_t compute_idx1_size() const;
	std::uint32_t get_chunkid_for_stream(const avi_stream *stream) const;
	std::uint32_t framenum_to_samplenum(std::uint32_t framenum) const;
	error expand_tempbuffer(std::uint32_t length);

	// core chunk read routines
	error get_first_chunk(avi_chunk const *parent, avi_chunk &newchunk);
	error get_next_chunk(avi_chunk const *parent, avi_chunk &newchunk);
	error find_first_chunk(std::uint32_t findme, avi_chunk const *container, avi_chunk &result);
	error find_next_chunk(std::uint32_t findme, avi_chunk const *container, avi_chunk &result);
	error find_first_list(std::uint32_t findme, avi_chunk const *container, avi_chunk &result);
	error find_next_list(std::uint32_t findme, avi_chunk const *container, avi_chunk &result);
	error get_next_chunk_internal(avi_chunk const *parent, avi_chunk &newchunk, std::uint64_t offset);
	error read_chunk_data(avi_chunk const &chunk, std::unique_ptr<std::uint8_t []> &buffer);

	// chunk read helpers
	error extract_movie_info();
	error parse_avih_chunk(avi_chunk const &avih);
	error parse_strh_chunk(avi_stream &stream, avi_chunk const &strh);
	error parse_strf_chunk(avi_stream &stream, avi_chunk const &strf);
	error parse_indx_chunk(avi_stream &stream, avi_chunk const &strf);
	error parse_idx1_chunk(std::uint64_t baseoffset, avi_chunk const &idx1);

	// core chunk write routines
	error chunk_open(std::uint32_t type, std::uint32_t listtype, std::uint32_t estlength);
	error chunk_close();
	error chunk_write(std::uint32_t type, const void *data, std::uint32_t length);
	error chunk_overwrite(std::uint32_t type, const void *data, std::uint32_t length, std::uint64_t &offset, bool initial_write);

	// chunk write helpers
	error write_avih_chunk(bool initial_write);
	error write_strh_chunk(avi_stream &stream, bool initial_write);
	error write_strf_chunk(avi_stream const &stream);
	error write_indx_chunk(avi_stream &stream, bool initial_write);
	error write_idx1_chunk();

	// sound buffering helpers
	error soundbuf_write_chunk(std::uint32_t framenum);
	error soundbuf_flush(bool only_flush_full);

	// debugging
	void printf_chunk_recursive(avi_chunk const *chunk, int indent);

	/* shared data */
	osd_file::ptr       m_file;                 /* pointer to open file */
	int                 m_type;                 /* type of access (read/create) */
	movie_info          m_info;                 /* movie info structure */
	std::vector<std::uint8_t> m_tempbuffer;     /* temporary buffer */

	/* only used when reading */
	std::vector<avi_stream> m_streams;          /* allocated array of stream information */
	avi_chunk           m_rootchunk;            /* dummy root chunk that wraps the whole file */

	/* only used when creating */
	std::uint64_t       m_writeoffs;            /* current file write offset */
	std::uint64_t       m_riffbase;             /* base of the current RIFF */

	std::array<avi_chunk, 8> m_chunkstack;      /* stack of chunks we are writing */
	int                 m_chunksp;              /* stack pointer for the current chunk */

	std::uint64_t       m_saved_movi_offset;    /* writeoffset of movi list */
	std::uint64_t       m_saved_avih_offset;    /* writeoffset of avih chunk */

	std::vector<std::int16_t> m_soundbuf;       /* buffer for sound data */
	std::uint32_t       m_soundbuf_samples;     /* length of sound buffer in samples */
	std::uint32_t       m_soundbuf_chansamples[MAX_SOUND_CHANNELS]; /* samples in buffer for each channel */
	std::uint32_t       m_soundbuf_chunks;      /* number of chunks completed so far */
	std::uint32_t       m_soundbuf_frames;      /* number of frames ahead of the video */
};



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    fetch_16bits - read 16 bits in LSB order
    from the given buffer
-------------------------------------------------*/

/**
 * @fn  static inline std::uint16_t fetch_16bits(const std::uint8_t *data)
 *
 * @brief   Fetches the 16bits.
 *
 * @param   data    The data.
 *
 * @return  The 16bits.
 */

inline std::uint16_t fetch_16bits(const std::uint8_t *data)
{
	return data[0] | (data[1] << 8);
}


/*-------------------------------------------------
    fetch_32bits - read 32 bits in LSB order
    from the given buffer
-------------------------------------------------*/

/**
 * @fn  static inline std::uint32_t fetch_32bits(const std::uint8_t *data)
 *
 * @brief   Fetches the 32bits.
 *
 * @param   data    The data.
 *
 * @return  The 32bits.
 */

inline std::uint32_t fetch_32bits(const std::uint8_t *data)
{
	return data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
}


/*-------------------------------------------------
    fetch_64bits - read 64 bits in LSB order
    from the given buffer
-------------------------------------------------*/

/**
 * @fn  static inline std::uint64_t fetch_64bits(const std::uint8_t *data)
 *
 * @brief   Fetches the 64bits.
 *
 * @param   data    The data.
 *
 * @return  The 64bits.
 */

inline std::uint64_t fetch_64bits(const std::uint8_t *data)
{
	return std::uint64_t(data[0]) | (std::uint64_t(data[1]) << 8) |
			(std::uint64_t(data[2]) << 16) | (std::uint64_t(data[3]) << 24) |
			(std::uint64_t(data[4]) << 32) | (std::uint64_t(data[5]) << 40) |
			(std::uint64_t(data[6]) << 48) | (std::uint64_t(data[7]) << 56);
}


/*-------------------------------------------------
    put_16bits - write 16 bits in LSB order
    to the given buffer
-------------------------------------------------*/

/**
 * @fn  static inline void put_16bits(std::uint8_t *data, std::uint16_t value)
 *
 * @brief   Puts the 16bits.
 *
 * @param [in,out]  data    If non-null, the data.
 * @param   value           The value.
 */

inline void put_16bits(std::uint8_t *data, std::uint16_t value)
{
	data[0] = value >> 0;
	data[1] = value >> 8;
}


/*-------------------------------------------------
    put_32bits - write 32 bits in LSB order
    to the given buffer
-------------------------------------------------*/

/**
 * @fn  static inline void put_32bits(std::uint8_t *data, std::uint32_t value)
 *
 * @brief   Puts the 32bits.
 *
 * @param [in,out]  data    If non-null, the data.
 * @param   value           The value.
 */

inline void put_32bits(std::uint8_t *data, std::uint32_t value)
{
	data[0] = value >> 0;
	data[1] = value >> 8;
	data[2] = value >> 16;
	data[3] = value >> 24;
}


/*-------------------------------------------------
    put_64bits - write 64 bits in LSB order
    to the given buffer
-------------------------------------------------*/

/**
 * @fn  static inline void put_64bits(std::uint8_t *data, std::uint64_t value)
 *
 * @brief   Puts the 64bits.
 *
 * @param [in,out]  data    If non-null, the data.
 * @param   value           The value.
 */

inline void put_64bits(std::uint8_t *data, std::uint64_t value)
{
	data[0] = value >> 0;
	data[1] = value >> 8;
	data[2] = value >> 16;
	data[3] = value >> 24;
	data[4] = value >> 32;
	data[5] = value >> 40;
	data[6] = value >> 48;
	data[7] = value >> 56;
}


/**
 * @fn  static void u64toa(std::uint64_t val, char *output)
 *
 * @brief   64toas.
 *
 * @param   val             The value.
 * @param [in,out]  output  If non-null, the output.
 */

inline void u64toa(std::uint64_t val, char *output)
{
	std::uint32_t lo = std::uint32_t(val & 0xffffffff);
	std::uint32_t hi = std::uint32_t(val >> 32);
	if (hi != 0)
		sprintf(output, "%X%08X", hi, lo);
	else
		sprintf(output, "%X", lo);
}


/*-------------------------------------------------
    set_stream_chunk_info - set the chunk info
    for a given chunk within a stream
-------------------------------------------------*/

/**
 * @fn  static inline avi_error set_stream_chunk_info(avi_stream *stream, std::uint32_t index, std::uint64_t offset, std::uint32_t length)
 *
 * @brief   Sets stream chunk information.
 *
 * @param [in,out]  stream  If non-null, the stream.
 * @param   index           Zero-based index of the.
 * @param   offset          The offset.
 * @param   length          The length.
 *
 * @return  An avi_error.
 */

inline avi_file::error avi_stream::set_chunk_info(std::uint32_t index, std::uint64_t offset, std::uint32_t length)
{
	/* if we need to allocate more, allocate more */
	if (index >= m_chunks.capacity())
	{
		try { m_chunks.reserve((std::max<std::size_t>)(index, m_chunks.capacity() + 1000)); }
		catch (...) { return avi_file::error::NO_MEMORY; }
	}

	/* update the number of chunks */
	m_chunks.resize((std::max<std::size_t>)(m_chunks.size(), index + 1));

	/* set the data */
	m_chunks[index].offset = offset;
	m_chunks[index].length = length;

	return avi_file::error::NONE;
}


inline avi_file::error avi_stream::set_strh_data(std::uint8_t const *data, std::uint32_t size)
{
	m_type = fetch_32bits(&data[0]);
	m_scale = fetch_32bits(&data[20]);
	m_rate = fetch_32bits(&data[24]);
	m_samples = fetch_32bits(&data[32]);
	return avi_file::error::NONE;
}


inline avi_file::error avi_stream::set_strf_data(std::uint8_t const *data, std::uint32_t size)
{
	/* audio and video streams have differing headers */
	if (m_type == STREAMTYPE_VIDS)
	{
		m_width = fetch_32bits(&data[4]);
		m_height = fetch_32bits(&data[8]);
		m_depth = fetch_16bits(&data[14]);
		m_format = fetch_32bits(&data[16]);

		/* extra extraction for HuffYUV data */
		if ((m_format == FORMAT_HFYU) && (size >= 56))
		{
			avi_file::error const avierr = huffyuv_extract_tables(data, size);
			if (avierr != avi_file::error::NONE)
				return avierr;
		}
	}
	else if (m_type == STREAMTYPE_AUDS)
	{
		m_channels = fetch_16bits(&data[2]);
		m_samplebits = fetch_16bits(&data[14]);
		m_samplerate = fetch_32bits(&data[4]);
	}
	return avi_file::error::NONE;
}


/*-------------------------------------------------
    get_video_stream - return a pointer to the
    video stream
-------------------------------------------------*/

/**
 * @fn  static inline avi_stream *get_video_stream()
 *
 * @brief   Gets video stream.
 *
 * @return  null if it fails, else the video stream.
 */

inline avi_stream *avi_file_impl::get_video_stream()
{
	for (int streamnum = 0; streamnum < m_streams.size(); streamnum++)
		if (m_streams[streamnum].type() == STREAMTYPE_VIDS)
			return &m_streams[streamnum];

	return nullptr;
}


/*-------------------------------------------------
    get_audio_stream - return a pointer to the
    audio stream for the 'n'th channel
-------------------------------------------------*/

/**
 * @fn  static inline avi_stream *get_audio_stream(avi_file *file, int channel, int *offset)
 *
 * @brief   Gets audio stream.
 *
 * @param [in,out]  file    If non-null, the file.
 * @param   channel         The channel.
 * @param [in,out]  offset  If non-null, the offset.
 *
 * @return  null if it fails, else the audio stream.
 */

inline avi_stream *avi_file_impl::get_audio_stream(int channel, int &offset)
{
	/* find the audios stream */
	for (int streamnum = 0; streamnum < m_streams.size(); streamnum++)
		if (m_streams[streamnum].type() == STREAMTYPE_AUDS)
		{
			if (channel < m_streams[streamnum].channels())
			{
				offset = channel;
				return &m_streams[streamnum];
			}
			channel -= m_streams[streamnum].channels();
		}

	return nullptr;
}


/*-------------------------------------------------
    compute_idx1_size - compute the size of the
    idx1 chunk
-------------------------------------------------*/

/**
 * @fn  static inline std::uint32_t compute_idx1_size(avi_file *file)
 *
 * @brief   Calculates the index 1 size.
 *
 * @param [in,out]  file    If non-null, the file.
 *
 * @return  The calculated index 1 size.
 */

inline std::uint32_t avi_file_impl::compute_idx1_size() const
{
	int chunks = 0;

	/* count chunks in streams */
	for (int strnum = 0; strnum < m_streams.size(); strnum++)
		chunks += m_streams[strnum].chunks();

	return chunks * 16 + 8;
}


/*-------------------------------------------------
    get_chunkid_for_stream - make a chunk id for
    a given stream
-------------------------------------------------*/

/**
 * @fn  static inline std::uint32_t get_chunkid_for_stream(avi_file *file, avi_stream *stream)
 *
 * @brief   Gets chunkid for stream.
 *
 * @param [in,out]  file    If non-null, the file.
 * @param [in,out]  stream  If non-null, the stream.
 *
 * @return  The chunkid for stream.
 */

inline std::uint32_t avi_file_impl::get_chunkid_for_stream(const avi_stream *stream) const
{
	std::uint32_t chunkid;

	chunkid = AVI_FOURCC('0' + (stream - &m_streams[0]) / 10, '0' +  (stream - &m_streams[0]) % 10, 0, 0);
	if (stream->type() == STREAMTYPE_VIDS)
		chunkid |= (stream->format() == 0) ? CHUNKTYPE_XXDB : CHUNKTYPE_XXDC;
	else if (stream->type() == STREAMTYPE_AUDS)
		chunkid |= CHUNKTYPE_XXWB;

	return chunkid;
}


/*-------------------------------------------------
    framenum_to_samplenum - given a video frame
    number, get the first sample number
-------------------------------------------------*/

/**
 * @fn  static inline std::uint32_t framenum_to_samplenum(avi_file *file, std::uint32_t framenum)
 *
 * @brief   Framenum to samplenum.
 *
 * @param [in,out]  file    If non-null, the file.
 * @param   framenum        The framenum.
 *
 * @return  An std::uint32_t.
 */

inline std::uint32_t avi_file_impl::framenum_to_samplenum(std::uint32_t framenum) const
{
	return (std::uint64_t(m_info.audio_samplerate) * std::uint64_t(framenum) * std::uint64_t(m_info.video_sampletime) + m_info.video_timescale - 1) / std::uint64_t(m_info.video_timescale);
}


/*-------------------------------------------------
    expand_tempbuffer - expand the file's
    tempbuffer if necessary to contain the
    requested amount of data
-------------------------------------------------*/

/**
 * @fn  static inline avi_error expand_tempbuffer(avi_file *file, std::uint32_t length)
 *
 * @brief   Expand tempbuffer.
 *
 * @param [in,out]  file    If non-null, the file.
 * @param   length          The length.
 *
 * @return  An avi_error.
 */

inline avi_file::error avi_file_impl::expand_tempbuffer(std::uint32_t length)
{
	/* expand the tempbuffer to hold the data if necessary */
	if (length > m_tempbuffer.size())
	{
		try { m_tempbuffer.resize(2 * length); }
		catch (...) { return error::NO_MEMORY; }
	}
	return error::NONE;
}


/*-------------------------------------------------
    rgb32_compress_to_rgb - "compress" an RGB32
    bitmap to an RGB encoded frame
-------------------------------------------------*/

/**
 * @fn  static avi_error rgb32_compress_to_rgb(avi_stream *stream, const bitmap_rgb32 &bitmap, std::uint8_t *data, std::uint32_t numbytes)
 *
 * @brief   RGB 32 compress to RGB.
 *
 * @param [in,out]  stream  If non-null, the stream.
 * @param   bitmap          The bitmap.
 * @param [in,out]  data    If non-null, the data.
 * @param   numbytes        The numbytes.
 *
 * @return  An avi_error.
 */

avi_file::error avi_stream::rgb32_compress_to_rgb(const bitmap_rgb32 &bitmap, std::uint8_t *data, std::uint32_t numbytes) const
{
	int const height = (std::min<int>)(m_height, bitmap.height());
	int const width = (std::min<int>)(m_width, bitmap.width());
	std::uint8_t *const dataend = data + numbytes;
	int x, y;

	/* compressed video */
	for (y = 0; y < height; y++)
	{
		const std::uint32_t *source = &bitmap.pix32(y);
		std::uint8_t *dest = data + (m_height - 1 - y) * m_width * 3;

		for (x = 0; x < width && dest < dataend; x++)
		{
			rgb_t pix = *source++;
			*dest++ = pix.b();
			*dest++ = pix.g();
			*dest++ = pix.r();
		}

		/* fill in any blank space on the right */
		for ( ; x < m_width && dest < dataend; x++)
		{
			*dest++ = 0;
			*dest++ = 0;
			*dest++ = 0;
		}
	}

	/* fill in any blank space on the bottom */
	for ( ; y < m_height; y++)
	{
		std::uint8_t *dest = data + (m_height - 1 - y) * m_width * 3;
		for (x = 0; x < m_width && dest < dataend; x++)
		{
			*dest++ = 0;
			*dest++ = 0;
			*dest++ = 0;
		}
	}

	return avi_file::error::NONE;
}


/*-------------------------------------------------
    yuv_decompress_to_yuy16 - decompress a YUV
    encoded frame to a YUY16 bitmap
-------------------------------------------------*/

/**
 * @fn  static avi_error yuv_decompress_to_yuy16(avi_stream *stream, const std::uint8_t *data, std::uint32_t numbytes, bitmap_yuy16 &bitmap)
 *
 * @brief   Yuv decompress to yuy 16.
 *
 * @param [in,out]  stream  If non-null, the stream.
 * @param   data            The data.
 * @param   numbytes        The numbytes.
 * @param [in,out]  bitmap  The bitmap.
 *
 * @return  An avi_error.
 */

avi_file::error avi_stream::yuv_decompress_to_yuy16(const std::uint8_t *data, std::uint32_t numbytes, bitmap_yuy16 &bitmap) const
{
	std::uint16_t const *const dataend = reinterpret_cast<const std::uint16_t *>(data + numbytes);
	int x, y;

	/* compressed video */
	for (y = 0; y < m_height; y++)
	{
		const std::uint16_t *source = reinterpret_cast<const std::uint16_t *>(data) + y * m_width;
		std::uint16_t *dest = &bitmap.pix16(y);

		/* switch off the compression */
		switch (m_format)
		{
		case FORMAT_UYVY:
			for (x = 0; x < m_width && source < dataend; x++)
				*dest++ = *source++;
			break;

		case FORMAT_VYUY:
		case FORMAT_YUY2:
			for (x = 0; x < m_width && source < dataend; x++)
			{
				std::uint16_t pix = *source++;
				*dest++ = (pix >> 8) | (pix << 8);
			}
			break;
		}
	}

	return avi_file::error::NONE;
}


/*-------------------------------------------------
    yuy16_compress_to_yuy - "compress" a YUY16
    bitmap to a YUV encoded frame
-------------------------------------------------*/

/**
 * @fn  static avi_error yuy16_compress_to_yuy(avi_stream *stream, const bitmap_yuy16 &bitmap, std::uint8_t *data, std::uint32_t numbytes)
 *
 * @brief   Yuy 16 compress to yuy.
 *
 * @param [in,out]  stream  If non-null, the stream.
 * @param   bitmap          The bitmap.
 * @param [in,out]  data    If non-null, the data.
 * @param   numbytes        The numbytes.
 *
 * @return  An avi_error.
 */

avi_file::error avi_stream::yuy16_compress_to_yuy(const bitmap_yuy16 &bitmap, std::uint8_t *data, std::uint32_t numbytes) const
{
	std::uint16_t *const dataend = reinterpret_cast<std::uint16_t *>(data + numbytes);
	int x, y;

	/* compressed video */
	for (y = 0; y < m_height; y++)
	{
		const std::uint16_t *source = &bitmap.pix16(y);
		std::uint16_t *dest = reinterpret_cast<std::uint16_t *>(data) + y * m_width;

		/* switch off the compression */
		switch (m_format)
		{
		case FORMAT_UYVY:
			for (x = 0; x < m_width && dest < dataend; x++)
				*dest++ = *source++;
			break;

		case FORMAT_VYUY:
		case FORMAT_YUY2:
			for (x = 0; x < m_width && dest < dataend; x++)
			{
				std::uint16_t pix = *source++;
				*dest++ = (pix >> 8) | (pix << 8);
			}
			break;
		}
	}

	return avi_file::error::NONE;
}


/*-------------------------------------------------
    huffyuv_extract_tables - extract HuffYUV
    tables
-------------------------------------------------*/

/**
 * @fn  static avi_error huffyuv_extract_tables(avi_stream *stream, const std::uint8_t *chunkdata, std::uint32_t size)
 *
 * @brief   Huffyuv extract tables.
 *
 * @param [in,out]  stream  If non-null, the stream.
 * @param   chunkdata       The chunkdata.
 * @param   size            The size.
 *
 * @return  An avi_error.
 */

avi_file::error avi_stream::huffyuv_extract_tables(const std::uint8_t *chunkdata, std::uint32_t size)
{
	const std::uint8_t *const chunkend = chunkdata + size;

	/* allocate memory for the data */
	std::unique_ptr<huffyuv_data> huffyuv;
	try { huffyuv = std::make_unique<huffyuv_data>(); }
	catch (...) { return avi_file::error::NO_MEMORY; }

	/* extract predictor information */
	if (&chunkdata[40] >= chunkend)
		return avi_file::error::INVALID_DATA;
	huffyuv->predictor = chunkdata[40];

	/* make sure it's the left predictor */
	if ((huffyuv->predictor & ~HUFFYUV_PREDICT_DECORR) != HUFFYUV_PREDICT_LEFT)
		return avi_file::error::UNSUPPORTED_VIDEO_FORMAT;

	/* make sure it's 16bpp YUV data */
	if (chunkdata[41] != 16)
		return avi_file::error::UNSUPPORTED_VIDEO_FORMAT;
	chunkdata += 44;

	/* loop over tables */
	for (int tabnum = 0; tabnum < 3; tabnum++)
	{
		huffyuv_table &table = huffyuv->table[tabnum];
		std::uint32_t curbits, bitadd;
		std::uint16_t bitsat16 = 0;
		int offset = 0, bits;

		/* loop until we populate the whole table */
		while (offset < 256)
		{
			int data, shift, count, i;

			/* extract the next run */
			if (chunkdata >= chunkend)
				return avi_file::error::INVALID_DATA;
			data = *chunkdata++;
			shift = data & 0x1f;
			count = data >> 5;

			/* zero count means next whole byte is a count */
			if (count == 0)
			{
				if (chunkdata >= chunkend)
					return avi_file::error::INVALID_DATA;
				count = *chunkdata++;
			}
			for (i = 0; i < count; i++)
				table.shift[offset++] = shift;
		}

		/* now determine bit patterns and masks */
		curbits = 0;
		for (bits = 31; bits >= 0; bits--)
		{
			bitadd = 1 << (32 - bits);

			/* make sure we've cleared out all the bits below */
			if ((curbits & (bitadd - 1)) != 0)
				return avi_file::error::INVALID_DATA;

			/* find all entries with this shift count and assign them */
			for (offset = 0; offset < 256; offset++)
				if (table.shift[offset] == bits)
				{
					table.bits[offset] = curbits;
					table.mask[offset] = ~(bitadd - 1);
					curbits += bitadd;
				}

			/* remember the bit pattern when we complete all the 17-bit codes */
			if (bits == 17)
				bitsat16 = curbits >> 16;
		}

		/* allocate the number of extra lookup tables we need */
		if (bitsat16 > 0)
		{
			try { table.extralookup.resize(bitsat16 * 65536); }
			catch (...) { return avi_file::error::NO_MEMORY; }
			for (offset = 0; offset < bitsat16; offset++)
				table.baselookup[offset] = (offset << 8) | 0;
		}

		/* then create lookup tables */
		for (offset = 0; offset < 256; offset++)
			if (table.shift[offset] > 16)
			{
				std::uint16_t *tablebase = &table.extralookup[(table.bits[offset] >> 16) * 65536];
				std::uint32_t start = table.bits[offset] & 0xffff;
				std::uint32_t end = start + ((1 << (32 - table.shift[offset])) - 1);
				while (start <= end)
					tablebase[start++] = (offset << 8) | (table.shift[offset] - 16);
			}
			else if (table.shift[offset] > 0)
			{
				std::uint32_t start = table.bits[offset] >> 16;
				std::uint32_t end = start + ((1 << (16 - table.shift[offset])) - 1);
				while (start <= end)
					table.baselookup[start++] = (offset << 8) | table.shift[offset];
			}
	}

	m_huffyuv = std::move(huffyuv);
	return avi_file::error::NONE;
}


/*-------------------------------------------------
    huffyuv_decompress_to_yuy16 - decompress a
    HuffYUV-encoded frame to a YUY16 bitmap
-------------------------------------------------*/

/**
 * @fn  static avi_error huffyuv_decompress_to_yuy16(avi_stream *stream, const std::uint8_t *data, std::uint32_t numbytes, bitmap_yuy16 &bitmap)
 *
 * @brief   Huffyuv decompress to yuy 16.
 *
 * @param [in,out]  stream  If non-null, the stream.
 * @param   data            The data.
 * @param   numbytes        The numbytes.
 * @param [in,out]  bitmap  The bitmap.
 *
 * @return  An avi_error.
 */

avi_file::error avi_stream::huffyuv_decompress_to_yuy16(const std::uint8_t *data, std::uint32_t numbytes, bitmap_yuy16 &bitmap) const
{
	int prevlines = (m_height > 288) ? 2 : 1;
	std::uint8_t lastprevy = 0, lastprevcb = 0, lastprevcr = 0;
	std::uint8_t lasty = 0, lastcb = 0, lastcr = 0;
	std::uint8_t bitsinbuffer = 0;
	std::uint32_t bitbuffer = 0;
	std::uint32_t dataoffs = 0;
	int x, y;

	/* compressed video */
	for (y = 0; y < m_height; y++)
	{
		std::uint16_t *dest = &bitmap.pix16(y);

		/* handle the first four bytes independently */
		x = 0;
		if (y == 0)
		{
			/* first DWORD is stored as YUY2 */
			lasty = data[dataoffs++];
			lastcb = data[dataoffs++];
			*dest++ = (lasty << 8) | lastcb;
			lasty = data[dataoffs++];
			lastcr = data[dataoffs++];
			*dest++ = (lasty << 8) | lastcr;
			x = 2;
		}

		/* loop over pixels */
		for ( ; x < m_width; x++)
		{
			huffyuv_table const &ytable = m_huffyuv->table[0];
			huffyuv_table const &ctable = m_huffyuv->table[1 + (x & 1)];
			std::uint16_t huffdata;
			int shift;

			/* fill up the buffer; they store little-endian DWORDs, so we XOR with 3 */
			while (bitsinbuffer <= 24 && dataoffs < numbytes)
			{
				bitbuffer |= data[dataoffs++ ^ 3] << (24 - bitsinbuffer);
				bitsinbuffer += 8;
			}

			/* look up the Y component */
			huffdata = ytable.baselookup[bitbuffer >> 16];
			shift = huffdata & 0xff;
			if (shift == 0)
			{
				bitsinbuffer -= 16;
				bitbuffer <<= 16;

				/* fill up the buffer; they store little-endian DWORDs, so we XOR with 3 */
				while (bitsinbuffer <= 24 && dataoffs < numbytes)
				{
					bitbuffer |= data[dataoffs++ ^ 3] << (24 - bitsinbuffer);
					bitsinbuffer += 8;
				}

				huffdata = ytable.extralookup[(huffdata >> 8) * 65536 + (bitbuffer >> 16)];
				shift = huffdata & 0xff;
			}
			bitsinbuffer -= shift;
			bitbuffer <<= shift;
			std::uint16_t const pixel = huffdata & 0xff00;

			/* fill up the buffer; they store little-endian DWORDs, so we XOR with 3 */
			while (bitsinbuffer <= 24 && dataoffs < numbytes)
			{
				bitbuffer |= data[dataoffs++ ^ 3] << (24 - bitsinbuffer);
				bitsinbuffer += 8;
			}

			/* look up the Cb/Cr component */
			huffdata = ctable.baselookup[bitbuffer >> 16];
			shift = huffdata & 0xff;
			if (shift == 0)
			{
				bitsinbuffer -= 16;
				bitbuffer <<= 16;

				/* fill up the buffer; they store little-endian DWORDs, so we XOR with 3 */
				while (bitsinbuffer <= 24 && dataoffs < numbytes)
				{
					bitbuffer |= data[dataoffs++ ^ 3] << (24 - bitsinbuffer);
					bitsinbuffer += 8;
				}

				huffdata = ctable.extralookup[(huffdata >> 8) * 65536 + (bitbuffer >> 16)];
				shift = huffdata & 0xff;
			}
			bitsinbuffer -= shift;
			bitbuffer <<= shift;
			*dest++ = pixel | (huffdata >> 8);
		}
	}

	/* apply deltas */
	lastprevy = lastprevcb = lastprevcr = 0;
	for (y = 0; y < m_height; y++)
	{
		std::uint16_t *prevrow = &bitmap.pix16(y - prevlines);
		std::uint16_t *dest = &bitmap.pix16(y);

		/* handle the first four bytes independently */
		x = 0;
		if (y == 0)
		{
			/* lasty, lastcr, lastcb are set up previously */
			x = 2;
		}

		/* left predict or gradient predict */
		if ((m_huffyuv->predictor & ~HUFFYUV_PREDICT_DECORR) == HUFFYUV_PREDICT_LEFT ||
			((m_huffyuv->predictor & ~HUFFYUV_PREDICT_DECORR) == HUFFYUV_PREDICT_GRADIENT))
		{
			/* first do left deltas */
			for ( ; x < m_width; x += 2)
			{
				std::uint16_t pixel0 = dest[x + 0];
				std::uint16_t pixel1 = dest[x + 1];

				lasty += pixel0 >> 8;
				lastcb += pixel0;
				dest[x + 0] = (lasty << 8) | lastcb;

				lasty += pixel1 >> 8;
				lastcr += pixel1;
				dest[x + 1] = (lasty << 8) | lastcr;
			}

			/* for gradient, we then add in the previous row */
			if ((m_huffyuv->predictor & ~HUFFYUV_PREDICT_DECORR) == HUFFYUV_PREDICT_GRADIENT && y >= prevlines)
				for (x = 0; x < m_width; x++)
				{
					std::uint16_t curpix = dest[x];
					std::uint16_t prevpix = prevrow[x];
					std::uint8_t ysum = (curpix >> 8) + (prevpix >> 8);
					std::uint8_t csum = curpix + prevpix;
					dest[x] = (ysum << 8) | csum;
				}
		}

		/* median predict on rows > 0 */
		else if ((m_huffyuv->predictor & ~HUFFYUV_PREDICT_DECORR) == HUFFYUV_PREDICT_MEDIAN && y >= prevlines)
		{
			for ( ; x < m_width; x += 2)
			{
				std::uint16_t prevpixel0 = prevrow[x + 0];
				std::uint16_t prevpixel1 = prevrow[x + 1];
				std::uint16_t pixel0 = dest[x + 0];
				std::uint16_t pixel1 = dest[x + 1];
				std::uint8_t a, b, c;

				/* compute previous, above, and (prev + above - above-left) */
				a = lasty;
				b = prevpixel0 >> 8;
				c = lastprevy;
				lastprevy = b;
				if (a > b) { std::uint8_t tmp = a; a = b; b = tmp; }
				if (a > c) { std::uint8_t tmp = a; a = c; c = tmp; }
				if (b > c) { std::uint8_t tmp = b; b = c; c = tmp; }
				lasty = (pixel0 >> 8) + b;

				/* compute previous, above, and (prev + above - above-left) */
				a = lastcb;
				b = prevpixel0 & 0xff;
				c = lastprevcb;
				lastprevcb = b;
				if (a > b) { std::uint8_t tmp = a; a = b; b = tmp; }
				if (a > c) { std::uint8_t tmp = a; a = c; c = tmp; }
				if (b > c) { std::uint8_t tmp = b; b = c; c = tmp; }
				lastcb = (pixel0 & 0xff) + b;
				dest[x + 0] = (lasty << 8) | lastcb;

				/* compute previous, above, and (prev + above - above-left) */
				a = lasty;
				b = prevpixel1 >> 8;
				c = lastprevy;
				lastprevy = b;
				if (a > b) { std::uint8_t tmp = a; a = b; b = tmp; }
				if (a > c) { std::uint8_t tmp = a; a = c; c = tmp; }
				if (b > c) { std::uint8_t tmp = b; b = c; c = tmp; }
				lasty = (pixel1 >> 8) + b;

				/* compute previous, above, and (prev + above - above-left) */
				a = lastcr;
				b = prevpixel1 & 0xff;
				c = lastprevcr;
				lastprevcr = b;
				if (a > b) { std::uint8_t tmp = a; a = b; b = tmp; }
				if (a > c) { std::uint8_t tmp = a; a = c; c = tmp; }
				if (b > c) { std::uint8_t tmp = b; b = c; c = tmp; }
				lastcr = (pixel1 & 0xff) + b;
				dest[x + 1] = (lasty << 8) | lastcr;
			}
		}
	}

	return avi_file::error::NONE;
}


/*-------------------------------------------------
    avi_close - close an AVI movie file
-------------------------------------------------*/

/**
 * @fn  avi_error avi_close(avi_file *file)
 *
 * @brief   Avi close.
 *
 * @param [in,out]  file    If non-null, the file.
 *
 * @return  An avi_error.
 */

avi_file_impl::~avi_file_impl()
{
	error avierr = error::NONE;

	/* if we're creating a new file, finalize it by writing out the non-media chunks */
	if (m_type == FILETYPE_CREATE)
	{
		/* flush any pending sound data */
		avierr = soundbuf_flush(false);

		/* close the movi chunk */
		if (avierr == error::NONE)
			avierr = chunk_close();

		/* if this is the first RIFF chunk, write an idx1 */
		if (avierr == error::NONE && m_riffbase == 0)
			avierr = write_idx1_chunk();

		/* update the strh and indx chunks for each stream */
		for (int strnum = 0; strnum < m_streams.size(); strnum++)
		{
			if (avierr == error::NONE)
				avierr = write_strh_chunk(m_streams[strnum], false);
			if (avierr == error::NONE)
				avierr = write_indx_chunk(m_streams[strnum], false);
		}

		/* update the avih chunk */
		if (avierr == error::NONE)
			avierr = write_avih_chunk(false);

		/* close the RIFF chunk */
		if (avierr == error::NONE)
			avierr = chunk_close();
	}

	/* close the file */
	m_file.reset();

	//return avierr;
}


/*-------------------------------------------------
    avi_printf_chunks - print the chunks in a file
-------------------------------------------------*/

/**
 * @fn  void avi_printf_chunks(avi_file *file)
 *
 * @brief   Avi printf chunks.
 *
 * @param [in,out]  file    If non-null, the file.
 */

void avi_file_impl::printf_chunks()
{
	printf_chunk_recursive(&m_rootchunk, 0);
}


/*-------------------------------------------------
    get_movie_info - return a pointer to the
    movie info
-------------------------------------------------*/

/**
 * @fn  const avi_movie_info *avi_get_movie_info(avi_file *file)
 *
 * @brief   Avi get movie information.
 *
 * @param [in,out]  file    If non-null, the file.
 *
 * @return  null if it fails, else an avi_movie_info*.
 */

avi_file::movie_info const &avi_file_impl::get_movie_info() const
{
	return m_info;
}


/*-------------------------------------------------
    avi_frame_to_sample - convert a frame index
    to a sample index
-------------------------------------------------*/

/**
 * @fn  std::uint32_t avi_first_sample_in_frame(avi_file *file, std::uint32_t framenum)
 *
 * @brief   Avi first sample in frame.
 *
 * @param [in,out]  file    If non-null, the file.
 * @param   framenum        The framenum.
 *
 * @return  An std::uint32_t.
 */

std::uint32_t avi_file_impl::first_sample_in_frame(std::uint32_t framenum) const
{
	return framenum_to_samplenum(framenum);
}


/*-------------------------------------------------
    avi_read_video_frame - read video data
    for a particular frame from the AVI file,
    converting to YUY16 format
-------------------------------------------------*/

/**
 * @fn  avi_error avi_read_video_frame(avi_file *file, std::uint32_t framenum, bitmap_yuy16 &bitmap)
 *
 * @brief   Avi read video frame.
 *
 * @param [in,out]  file    If non-null, the file.
 * @param   framenum        The framenum.
 * @param [in,out]  bitmap  The bitmap.
 *
 * @return  An avi_error.
 */

avi_file::error avi_file_impl::read_video_frame(std::uint32_t framenum, bitmap_yuy16 &bitmap)
{
	/* get the video stream */
	avi_stream *const stream = get_video_stream();
	if (!stream)
		return error::INVALID_STREAM;

	/* validate our ability to handle the data */
	if (stream->format() != FORMAT_UYVY && stream->format() != FORMAT_VYUY && stream->format() != FORMAT_YUY2 && stream->format() != FORMAT_HFYU)
		return error::UNSUPPORTED_VIDEO_FORMAT;

	/* assume one chunk == one frame */
	if (framenum >= stream->chunks())
		return error::INVALID_FRAME;

	/* we only support YUY-style bitmaps (16bpp) */
	if (bitmap.width() < stream->width() || bitmap.height() < stream->height())
		return error::INVALID_BITMAP;

	/* expand the tempbuffer to hold the data if necessary */
	error avierr = error::NONE;
	avierr = expand_tempbuffer(stream->chunk(framenum).length);
	if (avierr != error::NONE)
		return avierr;

	/* read in the data */
	std::uint32_t bytes_read;
	osd_file::error const filerr = m_file->read(&m_tempbuffer[0], stream->chunk(framenum).offset, stream->chunk(framenum).length, bytes_read);
	if (filerr != osd_file::error::NONE || bytes_read != stream->chunk(framenum).length)
		return error::READ_ERROR;

	/* validate this is good data */
	std::uint32_t const chunkid = fetch_32bits(&m_tempbuffer[0]);
	if (chunkid == get_chunkid_for_stream(stream))
	{
		/* HuffYUV-compressed */
		if (stream->format() == FORMAT_HFYU)
			avierr = stream->huffyuv_decompress_to_yuy16(&m_tempbuffer[8], stream->chunk(framenum).length - 8, bitmap);

		/* other YUV-compressed */
		else
			avierr = stream->yuv_decompress_to_yuy16(&m_tempbuffer[8], stream->chunk(framenum).length - 8, bitmap);
	}
	else
		avierr = error::INVALID_DATA;

	return avierr;
}


/*-------------------------------------------------
    avi_read_sound_samples - read sound sample
    data from an AVI file
-------------------------------------------------*/

/**
 * @fn  avi_error avi_read_sound_samples(avi_file *file, int channel, std::uint32_t firstsample, std::uint32_t numsamples, std::int16_t *output)
 *
 * @brief   Avi read sound samples.
 *
 * @param [in,out]  file    If non-null, the file.
 * @param   channel         The channel.
 * @param   firstsample     The firstsample.
 * @param   numsamples      The numsamples.
 * @param [in,out]  output  If non-null, the output.
 *
 * @return  An avi_error.
 */

avi_file::error avi_file_impl::read_sound_samples(int channel, std::uint32_t firstsample, std::uint32_t numsamples, std::int16_t *output)
{
	/* get the audio stream */
	int offset = 0;
	avi_stream *const stream = get_audio_stream(channel, offset);
	if (!stream)
		return error::INVALID_STREAM;

	/* validate our ability to handle the data */
	if (stream->format() != 0 || (stream->samplebits() != 8 && stream->samplebits() != 16))
		return error::UNSUPPORTED_AUDIO_FORMAT;

	/* verify we are in range */
	if (firstsample >= stream->samples())
		return error::INVALID_FRAME;
	if (firstsample + numsamples > stream->samples())
		numsamples = stream->samples() - firstsample;

	/* determine bytes per sample */
	std::uint32_t const bytes_per_sample = stream->bytes_per_sample();

	/* loop until all samples have been extracted */
	while (numsamples > 0)
	{
		std::uint32_t chunkbase = 0, chunkend = 0, chunkid;
		std::uint32_t bytes_read, samples_this_chunk;
		int chunknum, sampnum;

		/* locate the chunk with the first sample */
		for (chunknum = 0; chunknum < stream->chunks(); chunknum++)
		{
			chunkend = chunkbase + (stream->chunk(chunknum).length - 8) / bytes_per_sample;
			if (firstsample < chunkend)
				break;
			chunkbase = chunkend;
		}

		/* if we hit the end, fill the rest with silence */
		if (chunknum == stream->chunks())
		{
			std::memset(output, 0, numsamples * 2);
			break;
		}

		/* expand the tempbuffer to hold the data if necessary */
		error avierr = expand_tempbuffer(stream->chunk(chunknum).length);
		if (avierr != error::NONE)
			return avierr;

		/* read in the data */
		auto const filerr = m_file->read(&m_tempbuffer[0], stream->chunk(chunknum).offset, stream->chunk(chunknum).length, bytes_read);
		if (filerr != osd_file::error::NONE || bytes_read != stream->chunk(chunknum).length)
			return error::READ_ERROR;

		/* validate this is good data */
		chunkid = fetch_32bits(&m_tempbuffer[0]);
		if (chunkid != get_chunkid_for_stream(stream))
			return error::INVALID_DATA;

		/* determine how many samples to copy */
		samples_this_chunk = chunkend - firstsample;
		samples_this_chunk = (std::min)(samples_this_chunk, numsamples);

		/* extract 16-bit samples from the chunk */
		if (stream->samplebits() == 16)
		{
			const std::int16_t *base = reinterpret_cast<const std::int16_t *>(&m_tempbuffer[8]);
			base += stream->channels() * (firstsample - chunkbase) + offset;
			for (sampnum = 0; sampnum < samples_this_chunk; sampnum++)
			{
				*output++ = LITTLE_ENDIANIZE_INT16(*base);
				base += stream->channels();
			}
		}

		/* extract 8-bit samples from the chunk */
		else if (stream->samplebits() == 8)
		{
			const std::uint8_t *base = &m_tempbuffer[8];
			base += stream->channels() * (firstsample - chunkbase) + offset;
			for (sampnum = 0; sampnum < samples_this_chunk; sampnum++)
			{
				*output++ = (*base << 8) - 0x8000;
				base += stream->channels();
			}
		}

		/* update our counters */
		firstsample += samples_this_chunk;
		numsamples -= samples_this_chunk;
	}

	return error::NONE;
}


/*-------------------------------------------------
    avi_append_video_frame_yuy16 - append a frame
    of video in YUY16 format
-------------------------------------------------*/

/**
 * @fn  avi_error avi_append_video_frame(avi_file *file, bitmap_yuy16 &bitmap)
 *
 * @brief   Avi append video frame.
 *
 * @param [in,out]  file    If non-null, the file.
 * @param [in,out]  bitmap  The bitmap.
 *
 * @return  An avi_error.
 */

avi_file::error avi_file_impl::append_video_frame(bitmap_yuy16 &bitmap)
{
	avi_stream *const stream = get_video_stream();
	error avierr;
	std::uint32_t maxlength;

	/* validate our ability to handle the data */
	if (stream->format() != FORMAT_UYVY && stream->format() != FORMAT_VYUY && stream->format() != FORMAT_YUY2 && stream->format() != FORMAT_HFYU)
		return error::UNSUPPORTED_VIDEO_FORMAT;

	/* write out any sound data first */
	avierr = soundbuf_write_chunk(stream->chunks());
	if (avierr != error::NONE)
		return avierr;

	/* make sure we have enough room */
	maxlength = 2 * stream->width() * stream->height();
	avierr = expand_tempbuffer(maxlength);
	if (avierr != error::NONE)
		return avierr;

	/* now compress the data */
	avierr = stream->yuy16_compress_to_yuy(bitmap, &m_tempbuffer[0], maxlength);
	if (avierr != error::NONE)
		return avierr;

	/* write the data */
	avierr = chunk_write(get_chunkid_for_stream(stream), &m_tempbuffer[0], maxlength);
	if (avierr != error::NONE)
		return avierr;

	/* set the info for this new chunk */
	avierr = stream->set_chunk_info(stream->chunks(), m_writeoffs - maxlength - 8, maxlength + 8);
	if (avierr != error::NONE)
		return avierr;

	stream->set_samples(m_info.video_numsamples = stream->chunks());

	return error::NONE;
}


/*-------------------------------------------------
    avi_append_video_frame_rgb32 - append a frame
    of video in RGB32 format
-------------------------------------------------*/

/**
 * @fn  avi_error avi_append_video_frame(avi_file *file, bitmap_rgb32 &bitmap)
 *
 * @brief   Avi append video frame.
 *
 * @param [in,out]  file    If non-null, the file.
 * @param [in,out]  bitmap  The bitmap.
 *
 * @return  An avi_error.
 */

avi_file::error avi_file_impl::append_video_frame(bitmap_rgb32 &bitmap)
{
	avi_stream *const stream = get_video_stream();
	error avierr;
	std::uint32_t maxlength;

	/* validate our ability to handle the data */
	if (stream->format() != 0)
		return error::UNSUPPORTED_VIDEO_FORMAT;

	/* depth must be 24 */
	if (stream->depth() != 24)
		return error::UNSUPPORTED_VIDEO_FORMAT;

	/* write out any sound data first */
	avierr = soundbuf_write_chunk(stream->chunks());
	if (avierr != error::NONE)
		return avierr;

	/* make sure we have enough room */
	maxlength = 3 * stream->width() * stream->height();
	avierr = expand_tempbuffer(maxlength);
	if (avierr != error::NONE)
		return avierr;

	/* copy the RGB data to the destination */
	avierr = stream->rgb32_compress_to_rgb(bitmap, &m_tempbuffer[0], maxlength);
	if (avierr != error::NONE)
		return avierr;

	/* write the data */
	avierr = chunk_write(get_chunkid_for_stream(stream), &m_tempbuffer[0], maxlength);
	if (avierr != error::NONE)
		return avierr;

	/* set the info for this new chunk */
	avierr = stream->set_chunk_info(stream->chunks(), m_writeoffs - maxlength - 8, maxlength + 8);
	if (avierr != error::NONE)
		return avierr;

	stream->set_samples(m_info.video_numsamples = stream->chunks());

	return error::NONE;
}


/*-------------------------------------------------
    avi_append_sound_samples - append sound
    samples
-------------------------------------------------*/

/**
 * @fn  avi_error avi_append_sound_samples(avi_file *file, int channel, const std::int16_t *samples, std::uint32_t numsamples, std::uint32_t sampleskip)
 *
 * @brief   Avi append sound samples.
 *
 * @param [in,out]  file    If non-null, the file.
 * @param   channel         The channel.
 * @param   samples         The samples.
 * @param   numsamples      The numsamples.
 * @param   sampleskip      The sampleskip.
 *
 * @return  An avi_error.
 */

avi_file::error avi_file_impl::append_sound_samples(int channel, const std::int16_t *samples, std::uint32_t numsamples, std::uint32_t sampleskip)
{
	std::uint32_t sampoffset = m_soundbuf_chansamples[channel];
	std::uint32_t sampnum;

	/* see if we have enough room in the buffer */
	if (sampoffset + numsamples > m_soundbuf_samples)
		return error::EXCEEDED_SOUND_BUFFER;

	/* append samples to the buffer in little-endian format */
	for (sampnum = 0; sampnum < numsamples; sampnum++)
	{
		std::int16_t data = *samples++;
		samples += sampleskip;
		data = LITTLE_ENDIANIZE_INT16(data);
		m_soundbuf[sampoffset++ * m_info.audio_channels + channel] = data;
	}
	m_soundbuf_chansamples[channel] = sampoffset;

	/* flush any full sound chunks to disk */
	return soundbuf_flush(TRUE);
}


/*-------------------------------------------------
    read_chunk_data - read a chunk's data into
    memory
-------------------------------------------------*/

/**
 * @fn  static avi_error read_chunk_data(avi_file *file, const avi_chunk *chunk, std::uint8_t **buffer)
 *
 * @brief   Reads chunk data.
 *
 * @param [in,out]  file    If non-null, the file.
 * @param   chunk           The chunk.
 * @param [in,out]  buffer  If non-null, the buffer.
 *
 * @return  The chunk data.
 */

avi_file::error avi_file_impl::read_chunk_data(avi_chunk const &chunk, std::unique_ptr<std::uint8_t []> &buffer)
{
	/* allocate memory for the data */
	try { buffer.reset(new std::uint8_t[chunk.size]); }
	catch (...) { return error::NO_MEMORY; }

	/* read from the file */
	std::uint32_t bytes_read;
	osd_file::error const filerr = m_file->read(&buffer[0], chunk.offset + 8, chunk.size, bytes_read);
	if (filerr != osd_file::error::NONE || bytes_read != chunk.size)
	{
		buffer.reset();
		return error::READ_ERROR;
	}

	return error::NONE;
}


/*-------------------------------------------------
    get_first_chunk - get information about the
    first chunk in a container
-------------------------------------------------*/

/**
 * @fn  static avi_error get_first_chunk(avi_file *file, const avi_chunk *parent, avi_chunk *newchunk)
 *
 * @brief   Gets the first chunk.
 *
 * @param [in,out]  file        If non-null, the file.
 * @param   parent              The parent.
 * @param [in,out]  newchunk    If non-null, the newchunk.
 *
 * @return  The first chunk.
 */

avi_file::error avi_file_impl::get_first_chunk(avi_chunk const *parent, avi_chunk &newchunk)
{
	std::uint64_t startoffset = (parent != nullptr && parent->type != 0) ? parent->offset + 12 : 0;
	if (parent != nullptr && parent->type != CHUNKTYPE_LIST && parent->type != CHUNKTYPE_RIFF && parent->type != 0)
		return error::INVALID_DATA;
	return get_next_chunk_internal(parent, newchunk, startoffset);
}


/*-------------------------------------------------
    get_next_chunk - get information about the
    next chunk in a container
-------------------------------------------------*/

/**
 * @fn  static avi_error get_next_chunk(avi_file *file, const avi_chunk *parent, avi_chunk *newchunk)
 *
 * @brief   Gets the next chunk.
 *
 * @param [in,out]  file        If non-null, the file.
 * @param   parent              The parent.
 * @param [in,out]  newchunk    If non-null, the newchunk.
 *
 * @return  The next chunk.
 */

avi_file::error avi_file_impl::get_next_chunk(avi_chunk const *parent, avi_chunk &newchunk)
{
	std::uint64_t nextoffset = newchunk.offset + 8 + newchunk.size + (newchunk.size & 1);
	return get_next_chunk_internal(parent, newchunk, nextoffset);
}


/*-------------------------------------------------
    find_first_chunk - get information about the
    first chunk of a particular type in a container
-------------------------------------------------*/

/**
 * @fn  static avi_error find_first_chunk(avi_file *file, std::uint32_t findme, const avi_chunk *container, avi_chunk *result)
 *
 * @brief   Searches for the first chunk.
 *
 * @param [in,out]  file    If non-null, the file.
 * @param   findme          The findme.
 * @param   container       The container.
 * @param [out] result      If non-null, the result.
 *
 * @return  The found chunk.
 */

avi_file::error avi_file_impl::find_first_chunk(std::uint32_t findme, const avi_chunk *container, avi_chunk &result)
{
	error avierr;

	for (avierr = get_first_chunk(container, result); avierr == error::NONE; avierr = get_next_chunk(container, result))
		if (result.type == findme)
			return error::NONE;

	return avierr;
}


/*-------------------------------------------------
    find_next_chunk - get information about the
    next chunk of a particular type in a container
-------------------------------------------------*/

/**
 * @fn  static avi_error find_next_chunk(avi_file *file, std::uint32_t findme, const avi_chunk *container, avi_chunk *result)
 *
 * @brief   Searches for the next chunk.
 *
 * @param [in,out]  file    If non-null, the file.
 * @param   findme          The findme.
 * @param   container       The container.
 * @param [out] result      If non-null, the result.
 *
 * @return  The found chunk.
 */

avi_file::error avi_file_impl::find_next_chunk(std::uint32_t findme, const avi_chunk *container, avi_chunk &result)
{
	error avierr;

	for (avierr = get_next_chunk(container, result); avierr == error::NONE; avierr = get_next_chunk(container, result))
		if (result.type == findme)
			return error::NONE;

	return avierr;
}


/*-------------------------------------------------
    find_first_list - get information about the
    first list of a particular type in a container
-------------------------------------------------*/

/**
 * @fn  static avi_error find_first_list(avi_file *file, std::uint32_t findme, const avi_chunk *container, avi_chunk *result)
 *
 * @brief   Searches for the first list.
 *
 * @param [in,out]  file    If non-null, the file.
 * @param   findme          The findme.
 * @param   container       The container.
 * @param [out] result      If non-null, the result.
 *
 * @return  The found list.
 */

avi_file::error avi_file_impl::find_first_list(std::uint32_t findme, const avi_chunk *container, avi_chunk &result)
{
	error avierr;

	for (avierr = find_first_chunk(CHUNKTYPE_LIST, container, result); avierr == error::NONE; avierr = find_next_chunk(CHUNKTYPE_LIST, container, result))
		if (result.listtype == findme)
			return error::NONE;

	return avierr;
}


/*-------------------------------------------------
    find_next_list - get information about the
    next list of a particular type in a container
-------------------------------------------------*/

/**
 * @fn  static avi_error find_next_list(avi_file *file, std::uint32_t findme, const avi_chunk *container, avi_chunk *result)
 *
 * @brief   Searches for the next list.
 *
 * @param [in,out]  file    If non-null, the file.
 * @param   findme          The findme.
 * @param   container       The container.
 * @param [out] result      If non-null, the result.
 *
 * @return  The found list.
 */

avi_file::error avi_file_impl::find_next_list(std::uint32_t findme, const avi_chunk *container, avi_chunk &result)
{
	error avierr;

	for (avierr = find_next_chunk(CHUNKTYPE_LIST, container, result); avierr == error::NONE; avierr = find_next_chunk(CHUNKTYPE_LIST, container, result))
		if (result.listtype == findme)
			return error::NONE;

	return avierr;
}


/*-------------------------------------------------
    get_next_chunk_internal - fetch the next
    chunk relative to the current one
-------------------------------------------------*/

/**
 * @fn  static avi_error get_next_chunk_internal(avi_file *file, const avi_chunk *parent, avi_chunk *newchunk, std::uint64_t offset)
 *
 * @brief   Gets the next chunk internal.
 *
 * @param [in,out]  file        If non-null, the file.
 * @param   parent              The parent.
 * @param [in,out]  newchunk    If non-null, the newchunk.
 * @param   offset              The offset.
 *
 * @return  The next chunk internal.
 */

avi_file::error avi_file_impl::get_next_chunk_internal(const avi_chunk *parent, avi_chunk &newchunk, std::uint64_t offset)
{
	osd_file::error filerr;
	std::uint8_t buffer[12];
	std::uint32_t bytesread;

	/* NULL parent implies the root */
	if (parent == nullptr)
		parent = &m_rootchunk;

	/* start at the current offset */
	newchunk.offset = offset;

	/* if we're past the bounds of the parent, bail */
	if (newchunk.offset + 8 >= parent->offset + 8 + parent->size)
		return error::END;

	/* read the header */
	filerr = m_file->read(buffer, newchunk.offset, 8, bytesread);
	if (filerr != osd_file::error::NONE || bytesread != 8)
		return error::INVALID_DATA;

	/* fill in the new chunk */
	newchunk.type = fetch_32bits(&buffer[0]);
	newchunk.size = fetch_32bits(&buffer[4]);

	/* if we are a list, fetch the list type */
	if (newchunk.type == CHUNKTYPE_LIST || newchunk.type == CHUNKTYPE_RIFF)
	{
		filerr = m_file->read(&buffer[8], newchunk.offset + 8, 4, bytesread);
		if (filerr != osd_file::error::NONE || bytesread != 4)
			return error::INVALID_DATA;
		newchunk.listtype = fetch_32bits(&buffer[8]);
	}

	return error::NONE;
}


/*-------------------------------------------------
    read_movie_data - get data about a movie
-------------------------------------------------*/

/**
 * @fn  static avi_error read_movie_data(avi_file *file)
 *
 * @brief   Reads movie data.
 *
 * @param [in,out]  file    If non-null, the file.
 *
 * @return  The movie data.
 */

avi_file::error avi_file_impl::read_movie_data()
{
	avi_chunk riff, hdrl, avih, strl, strh, strf, indx, movi, idx1;
	error avierr;

	/* find the RIFF chunk */
	avierr = find_first_chunk(CHUNKTYPE_RIFF, nullptr, riff);
	if (avierr != error::NONE)
		return avierr;

	/* verify that the RIFF type is AVI */
	if (riff.listtype != LISTTYPE_AVI)
	{
		avierr = error::INVALID_DATA;
		return avierr;
	}

	/* find the hdrl LIST chunk within the RIFF */
	avierr = find_first_list(LISTTYPE_HDRL, &riff, hdrl);
	if (avierr != error::NONE)
		return avierr;

	/* find the avih chunk */
	avierr = find_first_chunk(CHUNKTYPE_AVIH, &hdrl, avih);
	if (avierr != error::NONE)
		return avierr;

	/* parse the avih chunk */
	avierr = parse_avih_chunk(avih);
	if (avierr != error::NONE)
		return avierr;

	/* loop over strl LIST chunks */
	int strindex = 0;
	for (avierr = find_first_list(LISTTYPE_STRL, &hdrl, strl); avierr == error::NONE; avierr = find_next_list(LISTTYPE_STRL, &hdrl, strl))
	{
		/* if we have too many, it's a bad file */
		if (strindex >= m_streams.size())
			return avierr;

		/* find the strh chunk */
		avierr = find_first_chunk(CHUNKTYPE_STRH, &strl, strh);
		if (avierr != error::NONE)
			return avierr;

		/* parse the data */
		avierr = parse_strh_chunk(m_streams[strindex], strh);
		if (avierr != error::NONE)
			return avierr;

		/* find the strf chunk */
		avierr = find_first_chunk(CHUNKTYPE_STRF, &strl, strf);
		if (avierr != error::NONE)
			return avierr;

		/* parse the data */
		avierr = parse_strf_chunk(m_streams[strindex], strf);
		if (avierr != error::NONE)
			return avierr;

		/* find the indx chunk, if present */
		avierr = find_first_chunk(CHUNKTYPE_INDX, &strl, indx);
		if (avierr == error::NONE)
			avierr = parse_indx_chunk(m_streams[strindex], indx);

		/* next stream */
		strindex++;
	}

	/* normalize the error after parsing the stream headers */
	if (avierr == error::END)
		avierr = error::NONE;
	if (avierr != error::NONE)
		return avierr;

	/* find the base of the movi data */
	avierr = find_first_list(LISTTYPE_MOVI, &riff, movi);
	if (avierr != error::NONE)
		return avierr;

	/* find and parse the idx1 chunk within the RIFF (if present) */
	avierr = find_first_chunk(CHUNKTYPE_IDX1, &riff, idx1);
	if (avierr == error::NONE)
		avierr = parse_idx1_chunk(movi.offset + 8, idx1);
	if (avierr != error::NONE)
		return avierr;

	/* now extract the movie info */
	avierr = extract_movie_info();
	return avierr;
}


/*-------------------------------------------------
    extract_movie_info - extract the movie info
    from the streams we've read
-------------------------------------------------*/

/**
 * @fn  static avi_error extract_movie_info(avi_file *file)
 *
 * @brief   Extracts the movie information described by file.
 *
 * @param [in,out]  file    If non-null, the file.
 *
 * @return  The extracted movie information.
 */

avi_file::error avi_file_impl::extract_movie_info()
{
	avi_stream *stream;
	int offset;

	/* get the video stream */
	stream = get_video_stream();
	if (stream != nullptr)
	{
		/* fill in the info */
		m_info.video_format = stream->format();
		m_info.video_timescale = stream->rate();
		m_info.video_sampletime = stream->scale();
		m_info.video_numsamples = stream->samples();
		m_info.video_width = stream->width();
		m_info.video_height = stream->height();
	}

	/* get the first audio stream */
	stream = get_audio_stream(0, offset);
	if (stream != nullptr)
	{
		/* fill in the info */
		m_info.audio_format = stream->format();
		m_info.audio_timescale = stream->rate();
		m_info.audio_sampletime = stream->scale();
		m_info.audio_numsamples = stream->samples();
		m_info.audio_channels = 1;
		m_info.audio_samplebits = stream->samplebits();
		m_info.audio_samplerate = stream->samplerate();
	}

	/* now make sure all other audio streams are valid */
	while (nullptr != (stream = get_audio_stream(m_info.audio_channels, offset)))
	{
		/* get the stream info */
		m_info.audio_channels++;

		/* verify compatibility */
		if (m_info.audio_format != stream->format() ||
			m_info.audio_timescale != stream->rate() ||
			m_info.audio_sampletime != stream->scale() ||
			m_info.audio_numsamples != stream->samples() ||
			m_info.audio_samplebits != stream->samplebits() ||
			m_info.audio_samplerate != stream->samplerate())
			return error::INCOMPATIBLE_AUDIO_STREAMS;
	}

	return error::NONE;
}


/*-------------------------------------------------
    parse_avih_chunk - parse an avih header
    chunk
-------------------------------------------------*/

/**
 * @fn  static avi_error parse_avih_chunk(avi_file *file, avi_chunk *avih)
 *
 * @brief   Parse avih chunk.
 *
 * @param [in,out]  file    If non-null, the file.
 * @param [in,out]  avih    If non-null, the avih.
 *
 * @return  An avi_error.
 */

avi_file::error avi_file_impl::parse_avih_chunk(avi_chunk const &avih)
{
	/* read the data */
	std::unique_ptr<std::uint8_t []> chunkdata;
	error avierr = read_chunk_data(avih, chunkdata);
	if (avierr != error::NONE)
		return avierr;

	/* extract the data */
	std::uint32_t const streams = fetch_32bits(&chunkdata[24]);

	/* allocate memory for the streams */
	m_streams.clear();
	m_streams.resize(streams);

	return avierr;
}


/*-------------------------------------------------
    parse_strh_chunk - parse a strh header
    chunk
-------------------------------------------------*/

/**
 * @fn  static avi_error parse_strh_chunk(avi_file *file, avi_stream *stream, avi_chunk *strh)
 *
 * @brief   Parse strh chunk.
 *
 * @param [in,out]  file    If non-null, the file.
 * @param [in,out]  stream  If non-null, the stream.
 * @param [in,out]  strh    If non-null, the strh.
 *
 * @return  An avi_error.
 */

avi_file::error avi_file_impl::parse_strh_chunk(avi_stream &stream, avi_chunk const &strh)
{
	/* read the data */
	std::unique_ptr<std::uint8_t []> chunkdata;
	error const avierr = read_chunk_data(strh, chunkdata);
	if (avierr != error::NONE)
		return avierr;

	/* extract the data */
	return stream.set_strh_data(&chunkdata[0], strh.size);
}


/*-------------------------------------------------
    parse_strf_chunk - parse a strf header
    chunk
-------------------------------------------------*/

/**
 * @fn  static avi_error parse_strf_chunk(avi_file *file, avi_stream *stream, avi_chunk *strf)
 *
 * @brief   Parse strf chunk.
 *
 * @param [in,out]  file    If non-null, the file.
 * @param [in,out]  stream  If non-null, the stream.
 * @param [in,out]  strf    If non-null, the strf.
 *
 * @return  An avi_error.
 */

avi_file::error avi_file_impl::parse_strf_chunk(avi_stream &stream, avi_chunk const &strf)
{
	error avierr;

	/* read the data */
	std::unique_ptr<std::uint8_t []> chunkdata;
	avierr = read_chunk_data(strf, chunkdata);
	if (avierr != error::NONE)
		return avierr;

	return stream.set_strf_data(&chunkdata[0], strf.size);
}


/*-------------------------------------------------
    parse_indx_chunk - parse an indx chunk
-------------------------------------------------*/

/**
 * @fn  static avi_error parse_indx_chunk(avi_file *file, avi_stream *stream, avi_chunk *strf)
 *
 * @brief   Parse indx chunk.
 *
 * @param [in,out]  file    If non-null, the file.
 * @param [in,out]  stream  If non-null, the stream.
 * @param [in,out]  strf    If non-null, the strf.
 *
 * @return  An avi_error.
 */

avi_file::error avi_file_impl::parse_indx_chunk(avi_stream &stream, avi_chunk const &strf)
{
	error avierr;

	/* read the data */
	std::unique_ptr<std::uint8_t []> chunkdata;
	avierr = read_chunk_data(strf, chunkdata);
	if (avierr != error::NONE)
		return avierr;

	/* extract the data */
	std::uint16_t const longs_per_entry = fetch_16bits(&chunkdata[0]);
	//subtype = chunkdata[2];
	std::uint8_t const type = chunkdata[3];
	std::uint32_t const entries = fetch_32bits(&chunkdata[4]);
	//id = fetch_32bits(&chunkdata[8]);
	std::uint64_t const baseoffset = fetch_64bits(&chunkdata[12]);

	/* if this is a superindex, loop over entries and call ourselves recursively */
	if (type == AVI_INDEX_OF_INDEXES)
	{
		/* validate the size of each entry */
		if (longs_per_entry != 4)
			return error::INVALID_DATA;

		/* loop over entries and create subchunks for each */
		for (std::uint32_t entry = 0; entry < entries; entry++)
		{
			const std::uint8_t *base = &chunkdata[24 + entry * 16];
			osd_file::error filerr;
			avi_chunk subchunk;
			std::uint32_t bytes_read;
			std::uint8_t buffer[8];

			/* go read the subchunk */
			subchunk.offset = fetch_64bits(&base[0]);
			filerr = m_file->read(buffer, subchunk.offset, sizeof(buffer), bytes_read);
			if (filerr != osd_file::error::NONE || bytes_read != sizeof(buffer))
			{
				avierr = error::READ_ERROR;
				break;
			}

			/* parse the data */
			subchunk.type = fetch_32bits(&buffer[0]);
			subchunk.size = fetch_32bits(&buffer[4]);

			/* recursively parse each referenced chunk; stop if we hit an error */
			avierr = parse_indx_chunk(stream, subchunk);
			if (avierr != error::NONE)
				break;
		}
	}

	/* otherwise, this is a standard index */
	else if (type == AVI_INDEX_OF_CHUNKS)
	{
		/* validate the size of each entry */
		if (longs_per_entry != 2 && longs_per_entry != 3)
			return error::INVALID_DATA;

		/* loop over entries and parse out the data */
		for (std::uint32_t entry = 0; entry < entries; entry++)
		{
			const std::uint8_t *base = &chunkdata[24 + entry * 4 * longs_per_entry];
			std::uint32_t offset = fetch_32bits(&base[0]);
			std::uint32_t size = fetch_32bits(&base[4]) & 0x7fffffff;  // bit 31 == NOT a keyframe

			/* set the info for this chunk and advance */
			avierr = stream.set_chunk_info(stream.chunks(), baseoffset + offset - 8, size + 8);
			if (avierr != error::NONE)
				break;
		}
	}

	return avierr;
}


/*-------------------------------------------------
    parse_idx1_chunk - parse an idx1 chunk
-------------------------------------------------*/

/**
 * @fn  static avi_error parse_idx1_chunk(avi_file *file, std::uint64_t baseoffset, avi_chunk *idx1)
 *
 * @brief   Parse index 1 chunk.
 *
 * @param [in,out]  file    If non-null, the file.
 * @param   baseoffset      The baseoffset.
 * @param [in,out]  idx1    If non-null, the first index.
 *
 * @return  An avi_error.
 */

avi_file::error avi_file_impl::parse_idx1_chunk(std::uint64_t baseoffset, avi_chunk const &idx1)
{
	error avierr;

	/* read the data */
	std::unique_ptr<std::uint8_t []> chunkdata;
	avierr = read_chunk_data(idx1, chunkdata);
	if (avierr != error::NONE)
		return avierr;

	/* loop over entries */
	std::uint32_t const entries = idx1.size / 16;
	for (std::uint32_t entry = 0; entry < entries; entry++)
	{
		std::uint8_t const *const base = &chunkdata[entry * 16];
		std::uint32_t const chunkid = fetch_32bits(&base[0]);
		std::uint32_t const offset = fetch_32bits(&base[8]);
		std::uint32_t const size = fetch_32bits(&base[12]);
		int streamnum;

		/* determine the stream index */
		streamnum = ((chunkid >> 8) & 0xff) - '0';
		streamnum += 10 * ((chunkid & 0xff) - '0');
		if (streamnum >= m_streams.size())
			return error::INVALID_DATA;
		avi_stream &stream = m_streams[streamnum];

		/* set the appropriate entry */
		avierr = stream.set_chunk_info(stream.chunks(), baseoffset + offset, size + 8);
		if (avierr != error::NONE)
			return avierr;
	}

	return avierr;
}


/*-------------------------------------------------
    chunk_open - open a new chunk for writing
-------------------------------------------------*/

/**
 * @fn  static avi_error chunk_open(avi_file *file, std::uint32_t type, std::uint32_t listtype, std::uint32_t estlength)
 *
 * @brief   Queries if a given chunk open.
 *
 * @param [in,out]  file    If non-null, the file.
 * @param   type            The type.
 * @param   listtype        The listtype.
 * @param   estlength       The estlength.
 *
 * @return  An avi_error.
 */

avi_file::error avi_file_impl::chunk_open(std::uint32_t type, std::uint32_t listtype, std::uint32_t estlength)
{
	/* if we're out of stack entries, bail */
	if (m_chunksp >= m_chunkstack.size())
		return error::STACK_TOO_DEEP;
	avi_chunk &chunk = m_chunkstack[m_chunksp++];

	/* set up the chunk information */
	chunk.offset = m_writeoffs;
	chunk.size = estlength;
	chunk.type = type;
	chunk.listtype = listtype;

	/* non-list types */
	if (type != CHUNKTYPE_RIFF && type != CHUNKTYPE_LIST)
	{
		std::uint8_t buffer[8];

		/* populate the header */
		put_32bits(&buffer[0], chunk.type);
		put_32bits(&buffer[4], chunk.size);

		/* write the header */
		std::uint32_t written;
		osd_file::error const filerr = m_file->write(buffer, m_writeoffs, sizeof(buffer), written);
		if (filerr != osd_file::error::NONE || written != sizeof(buffer))
			return error::WRITE_ERROR;
		m_writeoffs += written;
	}

	/* list types */
	else
	{
		std::uint8_t buffer[12];

		/* populate the header */
		put_32bits(&buffer[0], chunk.type);
		put_32bits(&buffer[4], chunk.size);
		put_32bits(&buffer[8], chunk.listtype);

		/* write the header */
		std::uint32_t written;
		osd_file::error const filerr = m_file->write(buffer, m_writeoffs, sizeof(buffer), written);
		if (filerr != osd_file::error::NONE || written != sizeof(buffer))
			return error::WRITE_ERROR;
		m_writeoffs += written;
	}

	return error::NONE;
}


/*-------------------------------------------------
    chunk_close - finish writing an chunk
-------------------------------------------------*/

/**
 * @fn  static avi_error chunk_close(avi_file *file)
 *
 * @brief   Chunk close.
 *
 * @param [in,out]  file    If non-null, the file.
 *
 * @return  An avi_error.
 */

avi_file::error avi_file_impl::chunk_close()
{
	avi_chunk const &chunk = m_chunkstack[--m_chunksp];
	std::uint64_t const chunksize = m_writeoffs - (chunk.offset + 8);

	/* error if we don't fit into 32 bits */
	if (chunksize != std::uint32_t(chunksize))
		return error::INVALID_DATA;

	/* write the final size if it is different from the guess */
	if (chunk.size != chunksize)
	{
		std::uint8_t buffer[4];

		put_32bits(&buffer[0], std::uint32_t(chunksize));
		std::uint32_t written;
		osd_file::error const filerr = m_file->write(buffer, chunk.offset + 4, sizeof(buffer), written);
		if (filerr != osd_file::error::NONE || written != sizeof(buffer))
			return error::WRITE_ERROR;
	}

	/* round up to the next word */
	m_writeoffs += chunksize & 1;

	return error::NONE;
}


/*-------------------------------------------------
    chunk_write - write an chunk and its data
-------------------------------------------------*/

/**
 * @fn  static avi_error chunk_write(avi_file *file, std::uint32_t type, const void *data, std::uint32_t length)
 *
 * @brief   Chunk write.
 *
 * @param [in,out]  file    If non-null, the file.
 * @param   type            The type.
 * @param   data            The data.
 * @param   length          The length.
 *
 * @return  An avi_error.
 */

avi_file::error avi_file_impl::chunk_write(std::uint32_t type, const void *data, std::uint32_t length)
{
	error avierr;

	/* if we are the first RIFF, we must reserve enough space for the IDX chunk */
	std::uint32_t const idxreserve = (m_riffbase == 0 && type != CHUNKTYPE_IDX1) ? compute_idx1_size() : 0;

	/* if we are getting too big, split the RIFF */
	/* note that we ignore writes before the current RIFF base, as those are assumed to be
	   overwrites of a chunk from the previous RIFF */
	if ((m_writeoffs >= m_riffbase) && ((m_writeoffs + length + idxreserve - m_riffbase) >= MAX_RIFF_SIZE))
	{
		/* close the movi list */
		avierr = chunk_close();
		if (avierr != error::NONE)
			return avierr;

		/* write the idx1 chunk if this is the first */
		if (m_riffbase == 0)
		{
			avierr = write_idx1_chunk();
			if (avierr != error::NONE)
				return avierr;
		}

		/* close the RIFF */
		avierr = chunk_close();
		if (avierr != error::NONE)
			return avierr;

		/* open a new RIFF */
		m_riffbase = m_writeoffs;
		avierr = chunk_open(CHUNKTYPE_RIFF, LISTTYPE_AVIX, 0);
		if (avierr != error::NONE)
			return avierr;

		/* open a nested movi list */
		m_saved_movi_offset = m_writeoffs;
		avierr = chunk_open(CHUNKTYPE_LIST, LISTTYPE_MOVI, 0);
		if (avierr != error::NONE)
			return avierr;
	}

	/* open the chunk */
	avierr = chunk_open(type, 0, length);
	if (avierr != error::NONE)
		return avierr;

	/* write the data */
	std::uint32_t written;
	osd_file::error const filerr = m_file->write(data, m_writeoffs, length, written);
	if (filerr != osd_file::error::NONE || written != length)
		return error::WRITE_ERROR;
	m_writeoffs += written;

	/* close the chunk */
	return chunk_close();
}


/*-------------------------------------------------
    chunk_overwrite - write a chunk in two passes;
    first pass writes to the end of file and
    saves the offset; second pass overwrites the
    original
-------------------------------------------------*/

/**
 * @fn  static avi_error chunk_overwrite(avi_file *file, std::uint32_t type, const void *data, std::uint32_t length, std::uint64_t *offset, int initial_write)
 *
 * @brief   Chunk overwrite.
 *
 * @param [in,out]  file    If non-null, the file.
 * @param   type            The type.
 * @param   data            The data.
 * @param   length          The length.
 * @param [in,out]  offset  If non-null, the offset.
 * @param   initial_write   The initial write.
 *
 * @return  An avi_error.
 */

avi_file::error avi_file_impl::chunk_overwrite(std::uint32_t type, const void *data, std::uint32_t length, std::uint64_t &offset, bool initial_write)
{
	std::uint64_t savedoffset = 0;

	/* if this is our initial write, save the offset */
	if (initial_write)
		offset = m_writeoffs;

	/* otherwise, remember the current write offset and replace it with the original */
	else
	{
		savedoffset = m_writeoffs;
		m_writeoffs = offset;
	}

	/* write the chunk */
	error const avierr = chunk_write(type, data, length);

	/* if this isn't the initial write, restore the previous offset */
	if (!initial_write)
		m_writeoffs = savedoffset;

	return avierr;
}


/*-------------------------------------------------
    write_initial_headers - write out the inital
    set of AVI and stream headers
-------------------------------------------------*/

/**
 * @fn  static avi_error write_initial_headers(avi_file *file)
 *
 * @brief   Writes an initial headers.
 *
 * @param [in,out]  file    If non-null, the file.
 *
 * @return  An avi_error.
 */

avi_file::error avi_file_impl::write_initial_headers()
{
	error avierr;

	/* reset the write pointer */
	m_writeoffs = 0;

	/* open a RIFF chunk */
	avierr = chunk_open(CHUNKTYPE_RIFF, LISTTYPE_AVI, 0);
	if (avierr != error::NONE)
		return avierr;

	/* open a hdlr LIST */
	avierr = chunk_open(CHUNKTYPE_LIST, LISTTYPE_HDRL, 0);
	if (avierr != error::NONE)
		return avierr;

	/* write an avih chunk */
	avierr = write_avih_chunk(true);
	if (avierr != error::NONE)
		return avierr;

	/* for each stream, write a strl LIST */
	for (int strnum = 0; strnum < m_streams.size(); strnum++)
	{
		/* open a strl LIST */
		avierr = chunk_open(CHUNKTYPE_LIST, LISTTYPE_STRL, 0);
		if (avierr != error::NONE)
			return avierr;

		/* write the strh chunk */
		avierr = write_strh_chunk(m_streams[strnum], true);
		if (avierr != error::NONE)
			return avierr;

		/* write the strf chunk */
		avierr = write_strf_chunk(m_streams[strnum]);
		if (avierr != error::NONE)
			return avierr;

		/* write the indx chunk */
		avierr = write_indx_chunk(m_streams[strnum], true);
		if (avierr != error::NONE)
			return avierr;

		/* close the strl LIST */
		avierr = chunk_close();
		if (avierr != error::NONE)
			return avierr;
	}

	/* close the hdlr LIST */
	avierr = chunk_close();
	if (avierr != error::NONE)
		return avierr;

	/* open a movi LIST */
	m_saved_movi_offset = m_writeoffs;
	avierr = chunk_open(CHUNKTYPE_LIST, LISTTYPE_MOVI, 0);
	if (avierr != error::NONE)
		return avierr;

	return avierr;
}


/*-------------------------------------------------
    write_avih_chunk - write the avih header
    chunk
-------------------------------------------------*/

/**
 * @fn  static avi_error write_avih_chunk(avi_file *file, int initial_write)
 *
 * @brief   Writes an avih chunk.
 *
 * @param [in,out]  file    If non-null, the file.
 * @param   initial_write   The initial write.
 *
 * @return  An avi_error.
 */

avi_file::error avi_file_impl::write_avih_chunk(bool initial_write)
{
	avi_stream *video = get_video_stream();
	std::uint8_t buffer[56];

	/* reset the buffer */
	std::memset(buffer, 0, sizeof(buffer));

	put_32bits(&buffer[0], 1000000 * std::int64_t(video->scale()) / video->rate()); /* dwMicroSecPerFrame */
	put_32bits(&buffer[12], AVIF_HASINDEX | AVIF_ISINTERLEAVED); /* dwFlags */
	put_32bits(&buffer[16], video->samples());          /* dwTotalFrames */
	put_32bits(&buffer[24], m_streams.size());          /* dwStreams */
	put_32bits(&buffer[32], video->width());            /* dwWidth */
	put_32bits(&buffer[36], video->height());           /* dwHeight */

	/* (over)write the chunk */
	return chunk_overwrite(CHUNKTYPE_AVIH, buffer, sizeof(buffer), m_saved_avih_offset, initial_write);
}


/*-------------------------------------------------
    write_strh_chunk - write the strh header
    chunk
-------------------------------------------------*/

/**
 * @fn  static avi_error write_strh_chunk(avi_file *file, avi_stream *stream, int initial_write)
 *
 * @brief   Writes a strh chunk.
 *
 * @param [in,out]  file    If non-null, the file.
 * @param [in,out]  stream  If non-null, the stream.
 * @param   initial_write   The initial write.
 *
 * @return  An avi_error.
 */

avi_file::error avi_file_impl::write_strh_chunk(avi_stream &stream, bool initial_write)
{
	std::uint8_t buffer[56];

	/* reset the buffer */
	std::memset(buffer, 0, sizeof(buffer));

	put_32bits(&buffer[0], stream.type());              /* fccType */
	put_32bits(&buffer[20], stream.scale());            /* dwScale */
	put_32bits(&buffer[24], stream.rate());             /* dwRate */
	put_32bits(&buffer[32], stream.samples());          /* dwLength */
	put_32bits(&buffer[40], 10000);                     /* dwQuality */

	/* video-stream specific data */
	if (stream.type() == STREAMTYPE_VIDS)
	{
		put_32bits(&buffer[4],                          /* fccHandler */
					(stream.format() == FORMAT_HFYU) ? HANDLER_HFYU : HANDLER_DIB);
		put_32bits(&buffer[36],                         /* dwSuggestedBufferSize */
					stream.width() * stream.height() * 4);
		put_16bits(&buffer[52], stream.width());        /* rcFrame.right */
		put_16bits(&buffer[54], stream.height());       /* rcFrame.bottom */
	}

	/* audio-stream specific data */
	if (stream.type() == STREAMTYPE_AUDS)
	{
		put_32bits(&buffer[36],                         /* dwSuggestedBufferSize */
					stream.samplerate() * stream.bytes_per_sample());
		put_32bits(&buffer[44],                         /* dwSampleSize */
					stream.bytes_per_sample());
	}

	/* write the chunk */
	return chunk_overwrite(CHUNKTYPE_STRH, buffer, sizeof(buffer), stream.saved_strh_offset(), initial_write);
}


/*-------------------------------------------------
    write_strf_chunk - write the strf header
    chunk
-------------------------------------------------*/

/**
 * @fn  static avi_error write_strf_chunk(avi_file *file, avi_stream *stream)
 *
 * @brief   Writes a strf chunk.
 *
 * @param [in,out]  file    If non-null, the file.
 * @param [in,out]  stream  If non-null, the stream.
 *
 * @return  An avi_error.
 */

avi_file::error avi_file_impl::write_strf_chunk(avi_stream const &stream)
{
	/* video stream */
	if (stream.type() == STREAMTYPE_VIDS)
	{
		std::uint8_t buffer[40];

		/* reset the buffer */
		std::memset(buffer, 0, sizeof(buffer));

		put_32bits(&buffer[0], sizeof(buffer));         /* biSize */
		put_32bits(&buffer[4], stream.width());         /* biWidth */
		put_32bits(&buffer[8], stream.height());        /* biHeight */
		put_16bits(&buffer[12], 1);                     /* biPlanes */
		put_16bits(&buffer[14], stream.depth());        /* biBitCount */
		put_32bits(&buffer[16], stream.format());       /* biCompression */
		put_32bits(&buffer[20],                         /* biSizeImage */
					stream.width() * stream.height() * (stream.depth() + 7) / 8);

		/* write the chunk */
		return chunk_write(CHUNKTYPE_STRF, buffer, sizeof(buffer));
	}

	/* audio stream */
	if (stream.type() == STREAMTYPE_AUDS)
	{
		std::uint8_t buffer[16];

		/* reset the buffer */
		std::memset(buffer, 0, sizeof(buffer));

		put_16bits(&buffer[0], 1);                      /* wFormatTag */
		put_16bits(&buffer[2], stream.channels());      /* nChannels */
		put_32bits(&buffer[4], stream.samplerate());    /* nSamplesPerSec */
		put_32bits(&buffer[8],                          /* nAvgBytesPerSec */
					stream.samplerate() * stream.bytes_per_sample());
		put_16bits(&buffer[12],                         /* nBlockAlign */
					stream.bytes_per_sample());
		put_16bits(&buffer[14], stream.samplebits());   /* wBitsPerSample */

		/* write the chunk */
		return chunk_write(CHUNKTYPE_STRF, buffer, sizeof(buffer));
	}

	return error::INVALID_DATA;
}


/*-------------------------------------------------
    write_indx_chunk - write the indx header
    chunk
-------------------------------------------------*/

/**
 * @fn  static avi_error write_indx_chunk(avi_file *file, avi_stream *stream, int initial_write)
 *
 * @brief   Writes an indx chunk.
 *
 * @param [in,out]  file    If non-null, the file.
 * @param [in,out]  stream  If non-null, the stream.
 * @param   initial_write   The initial write.
 *
 * @return  An avi_error.
 */

avi_file::error avi_file_impl::write_indx_chunk(avi_stream &stream, bool initial_write)
{
	std::uint8_t buffer[24 + 16 * MAX_AVI_SIZE_IN_GB / 4];
	std::uint32_t master_entries = 0;

	/* reset the buffer */
	std::memset(buffer, 0, sizeof(buffer));

	/* construct the chunk ID and index chunk ID */
	std::uint32_t const chunkid = get_chunkid_for_stream(&stream);
	std::uint32_t const indexchunkid = AVI_FOURCC('i', 'x', '0' + (&stream - &m_streams[0]) / 10, '0' + (&stream - &m_streams[0]) % 10);

	/* loop over chunks of 4GB and write out indexes for them first */
	if (!initial_write && m_riffbase != 0)
	{
		for (std::uint64_t currentbase = 0; currentbase < m_writeoffs; currentbase += FOUR_GB)
		{
			std::uint64_t const currentend = currentbase + FOUR_GB;
			std::uint32_t chunks_this_index = 0;
			std::uint32_t bytes_this_index = 0;

			/* count chunks that are in this region */
			for (std::uint32_t chunknum = 0; chunknum < stream.chunks(); chunknum++)
				if (stream.chunk(chunknum).offset >= currentbase && stream.chunk(chunknum).offset < currentend)
					chunks_this_index++;

			/* if no chunks, skip */
			if (chunks_this_index == 0)
				continue;

			/* allocate memory */
			std::unique_ptr<std::uint8_t []> tempbuf;
			try { tempbuf.reset(new std::uint8_t[24 + 8 * chunks_this_index]); }
			catch (...) { return error::NO_MEMORY; }
			std::memset(&tempbuf[0], 0, 24 + 8 * chunks_this_index);

			/* make a regular index */
			put_16bits(&tempbuf[0], 2);                     /* wLongsPerEntry */
			tempbuf[2] = 0;                                 /* bIndexSubType */
			tempbuf[3] = AVI_INDEX_OF_CHUNKS;               /* bIndexType */
			put_32bits(&tempbuf[4], chunks_this_index);     /* nEntriesInUse */
			put_32bits(&tempbuf[8], chunkid);               /* dwChunkId */
			put_64bits(&tempbuf[12], currentbase);          /* qwBaseOffset */

			/* now fill in the indexes */
			chunks_this_index = 0;
			for (std::uint32_t chunknum = 0; chunknum < stream.chunks(); chunknum++)
				if (stream.chunk(chunknum).offset >= currentbase && stream.chunk(chunknum).offset < currentend)
				{
					put_32bits(&tempbuf[24 + 8 * chunks_this_index + 0], stream.chunk(chunknum).offset + 8 - currentbase);
					put_32bits(&tempbuf[24 + 8 * chunks_this_index + 4], stream.chunk(chunknum).length - 8);
					bytes_this_index += stream.chunk(chunknum).length;
					chunks_this_index++;
				}

			/* write the offset of this index to the master table */
			put_64bits(&buffer[24 + 16 * master_entries + 0], m_writeoffs);
			put_32bits(&buffer[24 + 16 * master_entries + 8], 24 + 8 * chunks_this_index + 8);
			if (stream.type() == STREAMTYPE_VIDS)
				put_32bits(&buffer[24 + 16 * master_entries + 12], chunks_this_index);
			else if (stream.type() == STREAMTYPE_AUDS)
				put_32bits(&buffer[24 + 16 * master_entries + 12], bytes_this_index / stream.bytes_per_sample());
			master_entries++;

			/* write the index */
			error const avierr = chunk_write(indexchunkid, &tempbuf[0], 24 + 8 * chunks_this_index);
			if (avierr != error::NONE)
				return avierr;
		}
	}

	/* build up the master index */
	if (master_entries != 0)
	{
		put_16bits(&buffer[0], 4);                      /* wLongsPerEntry */
		buffer[2] = 0;                                  /* bIndexSubType */
		buffer[3] = AVI_INDEX_OF_INDEXES;               /* bIndexType */
		put_32bits(&buffer[4], master_entries);         /* nEntriesInUse */
		put_32bits(&buffer[8], chunkid);                /* dwChunkId */
	}

	/* (over)write the chunk */
	return chunk_overwrite((master_entries == 0) ? CHUNKTYPE_JUNK : CHUNKTYPE_INDX, buffer, sizeof(buffer), stream.saved_indx_offset(), initial_write);
}


/*-------------------------------------------------
    write_idx1_chunk - write the idx1 chunk
-------------------------------------------------*/

/**
 * @fn  static avi_error write_idx1_chunk(avi_file *file)
 *
 * @brief   Writes an index 1 chunk.
 *
 * @param [in,out]  file    If non-null, the file.
 *
 * @return  An avi_error.
 */

avi_file::error avi_file_impl::write_idx1_chunk()
{
	std::uint32_t const tempbuflength = compute_idx1_size() - 8;
	std::uint32_t curchunk[2] = { 0 };

	/* allocate a temporary buffer */
	std::unique_ptr<std::uint8_t []> tempbuf;
	try { tempbuf.reset(new std::uint8_t[tempbuflength]); }
	catch (...) { return error::NO_MEMORY; }

	/* fill it in */
	for (std::uint32_t curoffset = 0; curoffset < tempbuflength; curoffset += 16)
	{
		std::uint64_t minoffset = ~std::uint64_t(0);
		int minstr = 0;

		/* determine which stream has the next chunk */
		for (int strnum = 0; strnum < m_streams.size(); strnum++)
			if (curchunk[strnum] < m_streams[strnum].chunks() && m_streams[strnum].chunk(curchunk[strnum]).offset < minoffset)
			{
				minoffset = m_streams[strnum].chunk(curchunk[strnum]).offset;
				minstr = strnum;
			}

		/* make an entry for this index */
		put_32bits(&tempbuf[curoffset + 0], get_chunkid_for_stream(&m_streams[minstr]));
		put_32bits(&tempbuf[curoffset + 4], 0x0010 /* AVIIF_KEYFRAME */);
		put_32bits(&tempbuf[curoffset + 8], minoffset - (m_saved_movi_offset + 8));
		put_32bits(&tempbuf[curoffset + 12], m_streams[minstr].chunk(curchunk[minstr]).length - 8);

		/* advance the chunk counter for this stream */
		curchunk[minstr]++;
	}

	/* write the chunk */
	return chunk_write(CHUNKTYPE_IDX1, &tempbuf[0], tempbuflength);
}


/*-------------------------------------------------
    soundbuf_initialize - initialize the sound
    buffering system
-------------------------------------------------*/

/**
 * @fn  static avi_error soundbuf_initialize(avi_file *file)
 *
 * @brief   Soundbuf initialize.
 *
 * @param [in,out]  file    If non-null, the file.
 *
 * @return  An avi_error.
 */

avi_file::error avi_file_impl::soundbuf_initialize()
{
	int offset;
	avi_stream *const audio = get_audio_stream(0, offset);
	avi_stream *const video = get_video_stream();

	/* we require a video stream */
	if (video == nullptr)
		return error::UNSUPPORTED_VIDEO_FORMAT;

	/* skip if no audio stream */
	if (audio == nullptr)
		return error::NONE;

	/* determine the number of samples we want in our buffer; 2 seconds should be enough */
	m_soundbuf_samples = m_info.audio_samplerate * SOUND_BUFFER_MSEC / 1000;

	/* allocate a buffer */
	m_soundbuf.clear();
	try { m_soundbuf.resize(m_soundbuf_samples * m_info.audio_channels, 0); }
	catch (...) { return error::NO_MEMORY; }

	/* determine the number of frames to be ahead (0.75secs) */
	m_soundbuf_frames = (std::uint64_t(video->rate()) * 75) / (std::uint64_t(video->scale()) * 100) + 1;
	return error::NONE;
}


/*-------------------------------------------------
    soundbuf_write_chunk - write the initial
    chunk data
-------------------------------------------------*/

/**
 * @fn  static avi_error soundbuf_write_chunk(avi_file *file, std::uint32_t framenum)
 *
 * @brief   Soundbuf write chunk.
 *
 * @param [in,out]  file    If non-null, the file.
 * @param   framenum        The framenum.
 *
 * @return  An avi_error.
 */

avi_file::error avi_file_impl::soundbuf_write_chunk(std::uint32_t framenum)
{
	int offset;
	avi_stream *const stream = get_audio_stream(0, offset);
	std::uint32_t length;

	/* skip if no audio stream */
	if (stream == nullptr)
		return error::NONE;

	/* determine the length of this chunk */
	if (framenum == 0)
		length = framenum_to_samplenum(m_soundbuf_frames);
	else
		length = framenum_to_samplenum(framenum + 1 + m_soundbuf_frames) - framenum_to_samplenum(framenum + m_soundbuf_frames);
	length *= stream->channels() * sizeof(std::int16_t);

	/* then do the initial write */
	error const avierr = chunk_write(get_chunkid_for_stream(stream), &m_soundbuf[0], length);
	if (avierr != error::NONE)
		return avierr;

	/* set the info for this new chunk */
	return stream->set_chunk_info(stream->chunks(), m_writeoffs - length - 8, length + 8);
}


/*-------------------------------------------------
    soundbuf_flush - flush data from the sound
    buffers
-------------------------------------------------*/

/**
 * @fn  static avi_error soundbuf_flush(avi_file *file, int only_flush_full)
 *
 * @brief   Soundbuf flush.
 *
 * @param [in,out]  file    If non-null, the file.
 * @param   only_flush_full The only flush full.
 *
 * @return  An avi_error.
 */

avi_file::error avi_file_impl::soundbuf_flush(bool only_flush_full)
{
	int offset;
	avi_stream *const stream = get_audio_stream(0, offset);
	std::int32_t channelsamples = m_soundbuf_samples;
	std::int32_t processedsamples = 0;
	std::uint32_t finalchunks;
	error avierr;
	std::uint32_t chunknum;
	std::uint32_t chunkid;

	/* skip if no stream */
	if (stream == nullptr)
		return error::NONE;

	/* get the chunk ID for this stream */
	chunkid = get_chunkid_for_stream(stream);
	std::uint32_t const bytes_per_sample = stream->channels() * sizeof(std::int16_t);
	finalchunks = stream->chunks();

	/* find out how many samples we've accumulated */
	for (int channel = 0; channel < stream->channels(); channel++)
		channelsamples = (std::min<std::int32_t>)(channelsamples, m_soundbuf_chansamples[channel]);

	/* loop over pending sound chunks */
	for (chunknum = m_soundbuf_chunks; chunknum < stream->chunks(); chunknum++)
	{
		avi_chunk_list &chunk = stream->chunk(chunknum);
		std::uint32_t const chunksamples = (chunk.length - 8) / bytes_per_sample;

		/* stop if we don't have enough to satisfy this chunk */
		if (only_flush_full && channelsamples < chunksamples)
			break;

		/* if we don't have all the samples we need, pad with 0's */
		if (channelsamples > 0 && channelsamples < chunksamples)
		{
			if (processedsamples + chunksamples > m_soundbuf_samples)
				return error::EXCEEDED_SOUND_BUFFER;
			std::memset(&m_soundbuf[(processedsamples + channelsamples) * stream->channels()], 0, (chunksamples - channelsamples) * bytes_per_sample);
		}

		/* if we're completely out of samples, clear the buffer entirely and use the end */
		else if (channelsamples <= 0)
		{
			processedsamples = m_soundbuf_samples - chunksamples;
			std::memset(&m_soundbuf[processedsamples * stream->channels()], 0, chunksamples * bytes_per_sample);
			chunkid = CHUNKTYPE_JUNK;
			finalchunks--;
		}

		/* copy the sample data in */
		avierr = chunk_overwrite(chunkid, &m_soundbuf[processedsamples * stream->channels()], chunk.length - 8, chunk.offset, false);
		if (avierr != error::NONE)
			return avierr;

		/* add up the samples */
		if (channelsamples > chunksamples)
			m_info.audio_numsamples = stream->add_samples(chunksamples);
		else if (channelsamples > 0)
			m_info.audio_numsamples = stream->add_samples(channelsamples);

		/* advance past those */
		processedsamples += chunksamples;
		channelsamples -= chunksamples;
		channelsamples = (std::max)(0, channelsamples);
	}

	/* if we have a non-zero offset, shift everything down */
	if (processedsamples > 0)
	{
		/* first account for the samples we processed */
		std::memmove(&m_soundbuf[0], &m_soundbuf[processedsamples * stream->channels()], (m_soundbuf_samples - processedsamples) * bytes_per_sample);
		for (int channel = 0; channel < stream->channels(); channel++)
			m_soundbuf_chansamples[channel] -= processedsamples;
	}

	/* update the final chunk count */
	if (!only_flush_full)
		stream->set_chunks(finalchunks);

	/* account for flushed chunks */
	m_soundbuf_chunks = chunknum;
	return error::NONE;
}


/*-------------------------------------------------
    printf_chunk_recursive - print information
    about a chunk recursively
-------------------------------------------------*/

/**
 * @fn  static void printf_chunk_recursive(avi_file *file, avi_chunk *container, int indent)
 *
 * @brief   Printf chunk recursive.
 *
 * @param [in,out]  file        If non-null, the file.
 * @param [in,out]  container   If non-null, the container.
 * @param   indent              The indent.
 */

void avi_file_impl::printf_chunk_recursive(avi_chunk const *container, int indent)
{
	char size[20], offset[20];
	avi_chunk curchunk;
	error avierr;

	/* iterate over chunks in this container */
	for (avierr = get_first_chunk(container, curchunk); avierr == error::NONE; avierr = get_next_chunk(container, curchunk))
	{
		std::uint32_t chunksize = curchunk.size;
		bool recurse = false;

		u64toa(curchunk.size, size);
		u64toa(curchunk.offset, offset);
		printf("%*schunk = %c%c%c%c, size=%s (%s)\n", indent, "",
				std::uint8_t(curchunk.type >> 0),
				std::uint8_t(curchunk.type >> 8),
				std::uint8_t(curchunk.type >> 16),
				std::uint8_t(curchunk.type >> 24),
				size, offset);

		/* certain chunks are just containers; recurse into them */
		switch (curchunk.type)
		{
			/* basic containers */
			case CHUNKTYPE_RIFF:
			case CHUNKTYPE_LIST:
				printf("%*stype = %c%c%c%c\n", indent, "",
						std::uint8_t(curchunk.listtype >> 0),
						std::uint8_t(curchunk.listtype >> 8),
						std::uint8_t(curchunk.listtype >> 16),
						std::uint8_t(curchunk.listtype >> 24));
				recurse = true;
				chunksize = 0;
				break;
		}

		/* print data within the chunk */
		if (chunksize > 0 && curchunk.size < 1024 * 1024)
		{
			std::unique_ptr<std::uint8_t []> data;

			/* read the data for a chunk */
			avierr = read_chunk_data(curchunk, data);
			if (avierr == error::NONE)
			{
				std::uint32_t const bytes = (std::min)(std::uint32_t(512), chunksize);
				for (std::uint32_t i = 0; i < bytes; i++)
				{
					if (i % 16 == 0) printf("%*s   ", indent, "");
					printf("%02X ", data[i]);
					if (i % 16 == 15) printf("\n");
				}
				if (chunksize % 16 != 0) printf("\n");
			}
		}

		/* if we're recursing, dive down */
		if (recurse)
			printf_chunk_recursive(&curchunk, indent + 4);
	}

	/* if we didn't get a legitimate error, indicate that */
	if (avierr != error::END)
		printf("[chunk error %d]\n", int(avierr));
}

} // anonymous namespace



/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    avi_open - open an AVI movie file for read
-------------------------------------------------*/

/**
 * @fn  avi_error avi_open(const char *filename, avi_file **file)
 *
 * @brief   Queries if a given avi open.
 *
 * @param   filename        Filename of the file.
 * @param [in,out]  file    If non-null, the file.
 *
 * @return  An avi_error.
 */

avi_file::error avi_file::open(std::string const &filename, ptr &file)
{
	/* open the file */
	osd_file::ptr f;
	std::uint64_t length;
	if (osd_file::open(filename, OPEN_FLAG_READ, f, length) != osd_file::error::NONE)
		return error::CANT_OPEN_FILE;

	/* allocate the file */
	std::unique_ptr<avi_file_impl> newfile;
	try { newfile = std::make_unique<avi_file_impl>(std::move(f), length); }
	catch (...) { return error::NO_MEMORY; }

	/* parse the data */
	error const avierr = newfile->read_movie_data();
	if (avierr != error::NONE)
		return avierr;

	file = std::move(newfile);
	return error::NONE;
}


/*-------------------------------------------------
    avi_create - create a new AVI movie file
-------------------------------------------------*/

/**
 * @fn  avi_error avi_create(const char *filename, const avi_movie_info *info, avi_file **file)
 *
 * @brief   Avi create.
 *
 * @param   filename        Filename of the file.
 * @param   info            The information.
 * @param [in,out]  file    If non-null, the file.
 *
 * @return  An avi_error.
 */

avi_file::error avi_file::create(std::string const &filename, movie_info const &info, ptr &file)
{
	/* validate video info */
	if ((info.video_format != 0 && info.video_format != FORMAT_UYVY && info.video_format != FORMAT_VYUY && info.video_format != FORMAT_YUY2) ||
		(info.video_width == 0) ||
		(info.video_height == 0) ||
		(info.video_depth == 0) ||
		(info.video_depth % 8 != 0))
		return error::UNSUPPORTED_VIDEO_FORMAT;

	/* validate audio info */
	if (info.audio_format != 0 ||
		info.audio_channels > MAX_SOUND_CHANNELS ||
		info.audio_samplebits != 16)
		return error::UNSUPPORTED_AUDIO_FORMAT;

	/* open the file */
	osd_file::ptr f;
	std::uint64_t length;
	osd_file::error const filerr = osd_file::open(filename, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, f, length);
	if (filerr != osd_file::error::NONE)
		return error::CANT_OPEN_FILE;

	/* allocate the file */
	error avierr;
	std::unique_ptr<avi_file_impl> newfile;
	try { newfile = std::make_unique<avi_file_impl>(std::move(f), info); }
	catch (...)
	{
		avierr = error::NO_MEMORY;
		goto error;
	}

	/* initialize the sound buffering */
	avierr = newfile->soundbuf_initialize();
	if (avierr != error::NONE)
		goto error;

	/* write the initial headers */
	avierr = newfile->write_initial_headers();
	if (avierr != error::NONE)
		goto error;

	file = std::move(newfile);
	return error::NONE;

error:
	f.reset();
	newfile.reset();
	osd_file::remove(filename);
	return avierr;
}


/*-------------------------------------------------
    avi_error_string - get the error string for
    an avi_error
-------------------------------------------------*/

/**
 * @fn  const char *avi_error_string(avi_error err)
 *
 * @brief   Avi error string.
 *
 * @param   err The error.
 *
 * @return  null if it fails, else a char*.
 */

const char *avi_file::error_string(error err)
{
	switch (err)
	{
		case error::NONE:                       return "success";
		case error::END:                        return "hit end of file";
		case error::INVALID_DATA:               return "invalid data";
		case error::NO_MEMORY:                  return "out of memory";
		case error::READ_ERROR:                 return "read error";
		case error::WRITE_ERROR:                return "write error";
		case error::STACK_TOO_DEEP:             return "stack overflow";
		case error::UNSUPPORTED_FEATURE:        return "unsupported feature";
		case error::CANT_OPEN_FILE:             return "unable to open file";
		case error::INCOMPATIBLE_AUDIO_STREAMS: return "found incompatible audio streams";
		case error::INVALID_SAMPLERATE:         return "found invalid sample rate";
		case error::INVALID_STREAM:             return "invalid stream";
		case error::INVALID_FRAME:              return "invalid frame index";
		case error::INVALID_BITMAP:             return "invalid bitmap";
		case error::UNSUPPORTED_VIDEO_FORMAT:   return "unsupported video format";
		case error::UNSUPPORTED_AUDIO_FORMAT:   return "unsupported audio format";
		case error::EXCEEDED_SOUND_BUFFER:      return "sound buffer overflow";
		default:                                return "undocumented error";
	}
}


avi_file::avi_file()
{
}


avi_file::~avi_file()
{
}
