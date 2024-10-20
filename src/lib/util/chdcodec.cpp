// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Codecs used by the CHD format

***************************************************************************/

#include "chdcodec.h"

#include "avhuff.h"
#include "cdrom.h"
#include "chd.h"
#include "flac.h"
#include "hashing.h"
#include "multibyte.h"

#include "lzma/C/LzmaDec.h"
#include "lzma/C/LzmaEnc.h"

#include <zlib.h>
#include <zstd.h>

#include <new>


namespace {

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

constexpr uint8_t f_cd_sync_header[12] = { 0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00 };



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> chd_zlib_allocator

// allocation helper class for zlib
class chd_zlib_allocator
{
public:
	// construction/destruction
	chd_zlib_allocator();
	~chd_zlib_allocator();

	// installation
	void install(z_stream &stream);

private:
	// internal helpers
	static voidpf fast_alloc(voidpf opaque, uInt items, uInt size);
	static void fast_free(voidpf opaque, voidpf address);

	static constexpr int MAX_ZLIB_ALLOCS = 64;

	uint32_t *                m_allocptr[MAX_ZLIB_ALLOCS];
};


// ======================> chd_zlib_compressor

// ZLIB compressor
class chd_zlib_compressor : public chd_compressor
{
public:
	// construction/destruction
	chd_zlib_compressor(chd_file &chd, uint32_t hunkbytes, bool lossy);
	~chd_zlib_compressor();

	// core functionality
	virtual uint32_t compress(const uint8_t *src, uint32_t srclen, uint8_t *dest) override;

private:
	// internal state
	z_stream                m_deflater;
	chd_zlib_allocator      m_allocator;
};


// ======================> chd_zlib_decompressor

// ZLIB decompressor
class chd_zlib_decompressor : public chd_decompressor
{
public:
	// construction/destruction
	chd_zlib_decompressor(chd_file &chd, uint32_t hunkbytes, bool lossy);
	~chd_zlib_decompressor();

	// core functionality
	virtual void decompress(const uint8_t *src, uint32_t complen, uint8_t *dest, uint32_t destlen) override;

private:
	// internal state
	z_stream                m_inflater;
	chd_zlib_allocator      m_allocator;
};


// ======================> chd_zstd_compressor

// Zstandard compressor
class chd_zstd_compressor : public chd_compressor
{
public:
	// construction/destruction
	chd_zstd_compressor(chd_file &chd, uint32_t hunkbytes, bool lossy);
	~chd_zstd_compressor();

	// core functionality
	virtual uint32_t compress(const uint8_t *src, uint32_t srclen, uint8_t *dest) override;

private:
	// internal state
	ZSTD_CStream *          m_stream;
};


// ======================> chd_zstd_decompressor

// Zstandard decompressor
class chd_zstd_decompressor : public chd_decompressor
{
public:
	// construction/destruction
	chd_zstd_decompressor(chd_file &chd, uint32_t hunkbytes, bool lossy);
	~chd_zstd_decompressor();

	// core functionality
	virtual void decompress(const uint8_t *src, uint32_t complen, uint8_t *dest, uint32_t destlen) override;

private:
	// internal state
	ZSTD_DStream *          m_stream;
};


// ======================> chd_lzma_allocator

// allocation helper clas for zlib
class chd_lzma_allocator : public ISzAlloc
{
public:
	// construction/destruction
	chd_lzma_allocator();
	~chd_lzma_allocator();

private:
	// internal helpers
	static void *fast_alloc(ISzAllocPtr p, size_t size);
	static void fast_free(ISzAllocPtr p, void *address);

	static constexpr int MAX_LZMA_ALLOCS = 64;
	uint32_t *m_allocptr[MAX_LZMA_ALLOCS];
};


// ======================> chd_lzma_compressor

// LZMA compressor
class chd_lzma_compressor : public chd_compressor
{
public:
	// construction/destruction
	chd_lzma_compressor(chd_file &chd, uint32_t hunkbytes, bool lossy);
	~chd_lzma_compressor();

	// core functionality
	virtual uint32_t compress(const uint8_t *src, uint32_t srclen, uint8_t *dest) override;

	// helpers
	static void configure_properties(CLzmaEncProps &props, uint32_t hunkbytes);

private:
	// internal state
	CLzmaEncProps           m_props;
	chd_lzma_allocator      m_allocator;
};


// ======================> chd_lzma_decompressor

// LZMA decompressor
class chd_lzma_decompressor : public chd_decompressor
{
public:
	// construction/destruction
	chd_lzma_decompressor(chd_file &chd, uint32_t hunkbytes, bool lossy);
	~chd_lzma_decompressor();

	// core functionality
	virtual void decompress(const uint8_t *src, uint32_t complen, uint8_t *dest, uint32_t destlen) override;

private:
	// internal state
	CLzmaDec                m_decoder;
	chd_lzma_allocator      m_allocator;
};


// ======================> chd_huffman_compressor

// Huffman compressor
class chd_huffman_compressor : public chd_compressor
{
public:
	// construction/destruction
	chd_huffman_compressor(chd_file &chd, uint32_t hunkbytes, bool lossy);

	// core functionality
	virtual uint32_t compress(const uint8_t *src, uint32_t srclen, uint8_t *dest) override;

private:
	// internal state
	huffman_8bit_encoder    m_encoder;
};


// ======================> chd_huffman_decompressor

// Huffman decompressor
class chd_huffman_decompressor : public chd_decompressor
{
public:
	// construction/destruction
	chd_huffman_decompressor(chd_file &chd, uint32_t hunkbytes, bool lossy);

	// core functionality
	virtual void decompress(const uint8_t *src, uint32_t complen, uint8_t *dest, uint32_t destlen) override;

private:
	// internal state
	huffman_8bit_decoder    m_decoder;
};


// ======================> chd_flac_compressor

// FLAC compressor
class chd_flac_compressor : public chd_compressor
{
public:
	// construction/destruction
	chd_flac_compressor(chd_file &chd, uint32_t hunkbytes, bool lossy);

	// core functionality
	virtual uint32_t compress(const uint8_t *src, uint32_t srclen, uint8_t *dest) override;

	// static helpers
	static uint32_t blocksize(uint32_t bytes);

private:
	// internal state
	bool            m_big_endian;
	flac_encoder    m_encoder;
};


// ======================> chd_flac_decompressor

// FLAC decompressor
class chd_flac_decompressor : public chd_decompressor
{
public:
	// construction/destruction
	chd_flac_decompressor(chd_file &chd, uint32_t hunkbytes, bool lossy);

	// core functionality
	virtual void decompress(const uint8_t *src, uint32_t complen, uint8_t *dest, uint32_t destlen) override;

private:
	// internal state
	bool            m_big_endian;
	flac_decoder    m_decoder;
};


// ======================> chd_cd_flac_compressor

// CD/FLAC compressor
class chd_cd_flac_compressor : public chd_compressor
{
public:
	// construction/destruction
	chd_cd_flac_compressor(chd_file &chd, uint32_t hunkbytes, bool lossy);
	~chd_cd_flac_compressor();

	// core functionality
	virtual uint32_t compress(const uint8_t *src, uint32_t srclen, uint8_t *dest) override;

	// static helpers
	static uint32_t blocksize(uint32_t bytes);

private:
	// internal state
	bool                m_swap_endian;
	flac_encoder        m_encoder;
	z_stream            m_deflater;
	chd_zlib_allocator  m_allocator;
	std::vector<uint8_t>      m_buffer;
};


// ======================> chd_cd_flac_decompressor

// FLAC decompressor
class chd_cd_flac_decompressor : public chd_decompressor
{
public:
	// construction/destruction
	chd_cd_flac_decompressor(chd_file &chd, uint32_t hunkbytes, bool lossy);
	~chd_cd_flac_decompressor();

	// core functionality
	virtual void decompress(const uint8_t *src, uint32_t complen, uint8_t *dest, uint32_t destlen) override;

private:
	// internal state
	bool                m_swap_endian;
	flac_decoder        m_decoder;
	z_stream            m_inflater;
	chd_zlib_allocator  m_allocator;
	std::vector<uint8_t>      m_buffer;
};


// ======================> chd_cd_compressor

template <class BaseCompressor, class SubcodeCompressor>
class chd_cd_compressor : public chd_compressor
{
public:
	// construction/destruction
	chd_cd_compressor(chd_file &chd, uint32_t hunkbytes, bool lossy)
		: chd_compressor(chd, hunkbytes, lossy)
		, m_base_compressor(chd, (hunkbytes / cdrom_file::FRAME_SIZE) * cdrom_file::MAX_SECTOR_DATA, lossy)
		, m_subcode_compressor(chd, (hunkbytes / cdrom_file::FRAME_SIZE) * cdrom_file::MAX_SUBCODE_DATA, lossy)
		, m_buffer(hunkbytes + (hunkbytes / cdrom_file::FRAME_SIZE) * cdrom_file::MAX_SUBCODE_DATA)
	{
		// make sure the CHD's hunk size is an even multiple of the frame size
		if (hunkbytes % cdrom_file::FRAME_SIZE != 0)
			throw std::error_condition(chd_file::error::CODEC_ERROR);
	}

	// core functionality
	virtual uint32_t compress(const uint8_t *src, uint32_t srclen, uint8_t *dest) override
	{
		// determine header bytes
		uint32_t frames = srclen / cdrom_file::FRAME_SIZE;
		uint32_t complen_bytes = (srclen < 65536) ? 2 : 3;
		uint32_t ecc_bytes = (frames + 7) / 8;
		uint32_t header_bytes = ecc_bytes + complen_bytes;

		// clear out destination header
		memset(dest, 0, header_bytes);

		// copy audio data followed by subcode data
		for (uint32_t framenum = 0; framenum < frames; framenum++)
		{
			memcpy(&m_buffer[framenum * cdrom_file::MAX_SECTOR_DATA], &src[framenum * cdrom_file::FRAME_SIZE], cdrom_file::MAX_SECTOR_DATA);
			memcpy(&m_buffer[frames * cdrom_file::MAX_SECTOR_DATA + framenum * cdrom_file::MAX_SUBCODE_DATA], &src[framenum * cdrom_file::FRAME_SIZE + cdrom_file::MAX_SECTOR_DATA], cdrom_file::MAX_SUBCODE_DATA);

			// clear out ECC data if we can
			uint8_t *sector = &m_buffer[framenum * cdrom_file::MAX_SECTOR_DATA];
			if (memcmp(sector, f_cd_sync_header, sizeof(f_cd_sync_header)) == 0 && cdrom_file::ecc_verify(sector))
			{
				dest[framenum / 8] |= 1 << (framenum % 8);
				memset(sector, 0, sizeof(f_cd_sync_header));
				cdrom_file::ecc_clear(sector);
			}
		}

		// encode the base portion
		uint32_t complen = m_base_compressor.compress(&m_buffer[0], frames * cdrom_file::MAX_SECTOR_DATA, &dest[header_bytes]);
		if (complen >= srclen)
			throw std::error_condition(chd_file::error::COMPRESSION_ERROR);

		// write compressed length
		if (complen_bytes > 2)
			put_u24be(&dest[ecc_bytes], complen);
		else
			put_u16be(&dest[ecc_bytes], complen);

		// encode the subcode
		return header_bytes + complen + m_subcode_compressor.compress(&m_buffer[frames * cdrom_file::MAX_SECTOR_DATA], frames * cdrom_file::MAX_SUBCODE_DATA, &dest[header_bytes + complen]);
	}

private:
	// internal state
	BaseCompressor     m_base_compressor;
	SubcodeCompressor  m_subcode_compressor;
	std::vector<uint8_t>      m_buffer;
};


// ======================> chd_cd_decompressor

template <class BaseDecompressor, class SubcodeDecompressor>
class chd_cd_decompressor : public chd_decompressor
{
public:
	// construction/destruction
	chd_cd_decompressor(chd_file &chd, uint32_t hunkbytes, bool lossy)
		: chd_decompressor(chd, hunkbytes, lossy)
		, m_base_decompressor(chd, (hunkbytes / cdrom_file::FRAME_SIZE) * cdrom_file::MAX_SECTOR_DATA, lossy)
		, m_subcode_decompressor(chd, (hunkbytes / cdrom_file::FRAME_SIZE) * cdrom_file::MAX_SUBCODE_DATA, lossy)
		, m_buffer(hunkbytes)
	{
		// make sure the CHD's hunk size is an even multiple of the frame size
		if (hunkbytes % cdrom_file::FRAME_SIZE != 0)
			throw std::error_condition(chd_file::error::CODEC_ERROR);
	}

	// core functionality
	virtual void decompress(const uint8_t *src, uint32_t complen, uint8_t *dest, uint32_t destlen) override
	{
		// determine header bytes
		uint32_t frames = destlen / cdrom_file::FRAME_SIZE;
		uint32_t complen_bytes = (destlen < 65536) ? 2 : 3;
		uint32_t ecc_bytes = (frames + 7) / 8;
		uint32_t header_bytes = ecc_bytes + complen_bytes;

		// extract compressed length of base
		uint32_t complen_base = (complen_bytes > 2) ? get_u24be(&src[ecc_bytes]) : get_u16be(&src[ecc_bytes]);

		// reset and decode
		m_base_decompressor.decompress(&src[header_bytes], complen_base, &m_buffer[0], frames * cdrom_file::MAX_SECTOR_DATA);
		m_subcode_decompressor.decompress(&src[header_bytes + complen_base], complen - complen_base - header_bytes, &m_buffer[frames * cdrom_file::MAX_SECTOR_DATA], frames * cdrom_file::MAX_SUBCODE_DATA);

		// reassemble the data
		for (uint32_t framenum = 0; framenum < frames; framenum++)
		{
			memcpy(&dest[framenum * cdrom_file::FRAME_SIZE], &m_buffer[framenum * cdrom_file::MAX_SECTOR_DATA], cdrom_file::MAX_SECTOR_DATA);
			memcpy(&dest[framenum * cdrom_file::FRAME_SIZE + cdrom_file::MAX_SECTOR_DATA], &m_buffer[frames * cdrom_file::MAX_SECTOR_DATA + framenum * cdrom_file::MAX_SUBCODE_DATA], cdrom_file::MAX_SUBCODE_DATA);

			// reconstitute the ECC data and sync header
			uint8_t *sector = &dest[framenum * cdrom_file::FRAME_SIZE];
			if ((src[framenum / 8] & (1 << (framenum % 8))) != 0)
			{
				memcpy(sector, f_cd_sync_header, sizeof(f_cd_sync_header));
				cdrom_file::ecc_generate(sector);
			}
		}
	}

private:
	// internal state
	BaseDecompressor   m_base_decompressor;
	SubcodeDecompressor m_subcode_decompressor;
	std::vector<uint8_t>      m_buffer;
};


// ======================> chd_avhuff_compressor

// A/V compressor
class chd_avhuff_compressor : public chd_compressor
{
public:
	// construction/destruction
	chd_avhuff_compressor(chd_file &chd, uint32_t hunkbytes, bool lossy);

	// core functionality
	virtual uint32_t compress(const uint8_t *src, uint32_t srclen, uint8_t *dest) override;

private:
	// internal helpers
	void postinit();

	// internal state
	avhuff_encoder              m_encoder;
	bool                        m_postinit;
};


// ======================> chd_avhuff_decompressor

// A/V decompressor
class chd_avhuff_decompressor : public chd_decompressor
{
public:
	// construction/destruction
	chd_avhuff_decompressor(chd_file &chd, uint32_t hunkbytes, bool lossy);

	// core functionality
	virtual void process(const uint8_t *src, uint32_t complen) override;
	virtual void decompress(const uint8_t *src, uint32_t complen, uint8_t *dest, uint32_t destlen) override;
	virtual void configure(int param, void *config) override;

private:
	// internal state
	avhuff_decoder              m_decoder;
};



//**************************************************************************
//  CODEC LIST
//**************************************************************************

// an entry in the list
struct codec_entry
{
	chd_codec_type         m_type;
	bool                   m_lossy;
	const char *           m_name;
	chd_compressor::ptr    (*m_construct_compressor)(chd_file &, uint32_t, bool);
	chd_decompressor::ptr  (*m_construct_decompressor)(chd_file &, uint32_t, bool);

	template <class CompressorClass>
	static chd_compressor::ptr construct_compressor(chd_file &chd, uint32_t hunkbytes, bool lossy)
	{
		return std::make_unique<CompressorClass>(chd, hunkbytes, lossy);
	}

	template <class DecompressorClass>
	static chd_decompressor::ptr construct_decompressor(chd_file &chd, uint32_t hunkbytes, bool lossy)
	{
		return std::make_unique<DecompressorClass>(chd, hunkbytes, lossy);
	}
};


// static list of available known codecs
const codec_entry f_codec_list[] =
{
	// general codecs
	{ CHD_CODEC_ZLIB,       false,  "Deflate",              &codec_entry::construct_compressor<chd_zlib_compressor>,     &codec_entry::construct_decompressor<chd_zlib_decompressor> },
	{ CHD_CODEC_ZSTD,       false,  "Zstandard",            &codec_entry::construct_compressor<chd_zstd_compressor>,     &codec_entry::construct_decompressor<chd_zstd_decompressor> },
	{ CHD_CODEC_LZMA,       false,  "LZMA",                 &codec_entry::construct_compressor<chd_lzma_compressor>,     &codec_entry::construct_decompressor<chd_lzma_decompressor> },
	{ CHD_CODEC_HUFFMAN,    false,  "Huffman",              &codec_entry::construct_compressor<chd_huffman_compressor>,  &codec_entry::construct_decompressor<chd_huffman_decompressor> },
	{ CHD_CODEC_FLAC,       false,  "FLAC",                 &codec_entry::construct_compressor<chd_flac_compressor>,     &codec_entry::construct_decompressor<chd_flac_decompressor> },

	// general codecs with CD frontend
	{ CHD_CODEC_CD_ZLIB,    false,  "CD Deflate",           &codec_entry::construct_compressor<chd_cd_compressor<chd_zlib_compressor, chd_zlib_compressor> >,        &codec_entry::construct_decompressor<chd_cd_decompressor<chd_zlib_decompressor, chd_zlib_decompressor> > },
	{ CHD_CODEC_CD_ZSTD,    false,  "CD Zstandard",         &codec_entry::construct_compressor<chd_cd_compressor<chd_zstd_compressor, chd_zstd_compressor> >,        &codec_entry::construct_decompressor<chd_cd_decompressor<chd_zstd_decompressor, chd_zstd_decompressor> > },
	{ CHD_CODEC_CD_LZMA,    false,  "CD LZMA",              &codec_entry::construct_compressor<chd_cd_compressor<chd_lzma_compressor, chd_zlib_compressor> >,        &codec_entry::construct_decompressor<chd_cd_decompressor<chd_lzma_decompressor, chd_zlib_decompressor> > },
	{ CHD_CODEC_CD_FLAC,    false,  "CD FLAC",              &codec_entry::construct_compressor<chd_cd_flac_compressor>,                                              &codec_entry::construct_decompressor<chd_cd_flac_decompressor> },

	// A/V codecs
	{ CHD_CODEC_AVHUFF,     false,  "A/V Huffman",          &codec_entry::construct_compressor<chd_avhuff_compressor>,   &codec_entry::construct_decompressor<chd_avhuff_decompressor> },
};


//-------------------------------------------------
//  find_in_list - create a new compressor
//  instance of the given type
//-------------------------------------------------

const codec_entry *find_in_list(chd_codec_type type) noexcept
{
	// find in the list and construct the class
	for (auto & elem : f_codec_list)
		if (elem.m_type == type)
			return &elem;
	return nullptr;
}

} // anonymous namespace



//**************************************************************************
//  CHD CODEC
//**************************************************************************

//-------------------------------------------------
//  chd_codec - constructor
//-------------------------------------------------

chd_codec::chd_codec(chd_file &chd, uint32_t hunkbytes, bool lossy)
	: m_chd(chd)
	, m_hunkbytes(hunkbytes)
	, m_lossy(lossy)
{
}


//-------------------------------------------------
//  ~chd_codec - destructor
//-------------------------------------------------

chd_codec::~chd_codec()
{
}


//-------------------------------------------------
//  configure - configuration
//-------------------------------------------------

void chd_codec::configure(int param, void *config)
{
	// if not overridden, it is always a failure
	throw std::error_condition(std::errc::invalid_argument);
}



//**************************************************************************
//  CHD COMPRESSOR
//**************************************************************************

chd_compressor::chd_compressor(chd_file &chd, uint32_t hunkbytes, bool lossy)
	: chd_codec(chd, hunkbytes, lossy)
{
}



//**************************************************************************
//  CHD DECOMPRESSOR
//**************************************************************************

chd_decompressor::chd_decompressor(chd_file &chd, uint32_t hunkbytes, bool lossy)
	: chd_codec(chd, hunkbytes, lossy)
{
}

void chd_decompressor::process(const uint8_t *src, uint32_t complen)
{
	throw std::error_condition(chd_file::error::UNSUPPORTED_FORMAT);
}



//**************************************************************************
//  CHD CODEC LIST
//**************************************************************************

//-------------------------------------------------
//  new_compressor - create a new compressor
//  instance of the given type
//-------------------------------------------------

chd_compressor::ptr chd_codec_list::new_compressor(chd_codec_type type, chd_file &chd)
{
	// find in the list and construct the class
	codec_entry const *const entry = find_in_list(type);
	return entry ? (*entry->m_construct_compressor)(chd, chd.hunk_bytes(), entry->m_lossy) : nullptr;
}


//-------------------------------------------------
//  new_compressor - create a new decompressor
//  instance of the given type
//-------------------------------------------------

chd_decompressor::ptr chd_codec_list::new_decompressor(chd_codec_type type, chd_file &chd)
{
	// find in the list and construct the class
	const codec_entry *entry = find_in_list(type);
	return entry ? (*entry->m_construct_decompressor)(chd, chd.hunk_bytes(), entry->m_lossy) : nullptr;
}


//-------------------------------------------------
//  codec_exists - determine whether a codec type
//  corresponds to a supported codec
//-------------------------------------------------

bool chd_codec_list::codec_exists(chd_codec_type type) noexcept
{
	// find in the list and construct the class
	return bool(find_in_list(type));
}


//-------------------------------------------------
//  codec_name - return the name of the given
//  codec
//-------------------------------------------------

const char *chd_codec_list::codec_name(chd_codec_type type) noexcept
{
	// find in the list and construct the class
	const codec_entry *entry = find_in_list(type);
	return entry ? entry->m_name : nullptr;
}



//**************************************************************************
//  CODEC INSTANCE
//**************************************************************************

//-------------------------------------------------
//  chd_compressor_group - constructor
//-------------------------------------------------

chd_compressor_group::chd_compressor_group(chd_file &chd, uint32_t compressor_list[4])
	: m_hunkbytes(chd.hunk_bytes())
	, m_compress_test(m_hunkbytes)
#if CHDCODEC_VERIFY_COMPRESSION
	, m_decompressed(m_hunkbytes)
#endif
{
	// verify the compression types and initialize the codecs
	for (int codecnum = 0; codecnum < std::size(m_compressor); codecnum++)
	{
		if (compressor_list[codecnum] != CHD_CODEC_NONE)
		{
			m_compressor[codecnum] = chd_codec_list::new_compressor(compressor_list[codecnum], chd);
			if (!m_compressor[codecnum])
				throw std::error_condition(chd_file::error::UNKNOWN_COMPRESSION);
#if CHDCODEC_VERIFY_COMPRESSION
			m_decompressor[codecnum] = chd_codec_list::new_decompressor(compressor_list[codecnum], chd);
			if (!m_decompressor[codecnum])
				throw std::error_condition(chd_file::error::UNKNOWN_COMPRESSION);
#endif
		}
	}
}


//-------------------------------------------------
//  ~chd_compressor_group - destructor
//-------------------------------------------------

chd_compressor_group::~chd_compressor_group()
{
	// codecs and test buffer deleted automatically
}


//-------------------------------------------------
//  find_best_compressor - iterate over all codecs
//  to determine which one produces the best
//  compression for this hunk
//-------------------------------------------------

int8_t chd_compressor_group::find_best_compressor(const uint8_t *src, uint8_t *compressed, uint32_t &complen)
{
	// determine best compression technique
	complen = m_hunkbytes;
	int8_t compression = -1;
	for (int codecnum = 0; codecnum < std::size(m_compressor); codecnum++)
		if (m_compressor[codecnum])
		{
			// attempt to compress, swallowing errors
			try
			{
				// if this is the best one, copy the data into the permanent buffer
				uint32_t compbytes = m_compressor[codecnum]->compress(src, m_hunkbytes, &m_compress_test[0]);
#if CHDCODEC_VERIFY_COMPRESSION
				try
				{
					memset(m_decompressed, 0, m_hunkbytes);
					m_decompressor[codecnum]->decompress(m_compress_test, compbytes, m_decompressed, m_hunkbytes);
				}
				catch (...)
				{
				}

				if (memcmp(src, m_decompressed, m_hunkbytes) != 0)
				{
					compbytes = m_compressor[codecnum]->compress(src, m_hunkbytes, m_compress_test);
					try
					{
						m_decompressor[codecnum]->decompress(m_compress_test, compbytes, m_decompressed, m_hunkbytes);
					}
					catch (...)
					{
						memset(m_decompressed, 0, m_hunkbytes);
					}
				}
printf("   codec%d=%d bytes            \n", codecnum, compbytes);
#endif
				if (compbytes < complen)
				{
					compression = codecnum;
					complen = compbytes;
					memcpy(compressed, &m_compress_test[0], compbytes);
				}
			}
			catch (...)
			{
			}
		}

	// if the best is none, copy it over
	if (compression == -1)
		memcpy(compressed, src, m_hunkbytes);
	return compression;
}



//**************************************************************************
//  ZLIB ALLOCATOR HELPER
//**************************************************************************

//-------------------------------------------------
//  chd_zlib_allocator - constructor
//-------------------------------------------------

chd_zlib_allocator::chd_zlib_allocator()
{
	// reset pointer list
	memset(m_allocptr, 0, sizeof(m_allocptr));
}


//-------------------------------------------------
//  ~chd_zlib_allocator - destructor
//-------------------------------------------------

chd_zlib_allocator::~chd_zlib_allocator()
{
	// free our memory
	for (auto & elem : m_allocptr)
		delete[] elem;
}


//-------------------------------------------------
//  install - configure the allocators for a
//  stream
//-------------------------------------------------

void chd_zlib_allocator::install(z_stream &stream)
{
	stream.zalloc = &chd_zlib_allocator::fast_alloc;
	stream.zfree = &chd_zlib_allocator::fast_free;
	stream.opaque = this;
}


//-------------------------------------------------
//  zlib_fast_alloc - fast malloc for ZLIB, which
//  allocates and frees memory frequently
//-------------------------------------------------

voidpf chd_zlib_allocator::fast_alloc(voidpf opaque, uInt items, uInt size)
{
	auto *codec = reinterpret_cast<chd_zlib_allocator *>(opaque);

	// compute the size, rounding to the nearest 1k
	size = (size * items + 0x3ff) & ~0x3ff;

	// reuse a hunk if we can
	for (int scan = 0; scan < MAX_ZLIB_ALLOCS; scan++)
	{
		uint32_t *ptr = codec->m_allocptr[scan];
		if (ptr != nullptr && size == *ptr)
		{
			// set the low bit of the size so we don't match next time
			*ptr |= 1;
			return ptr + 1;
		}
	}

	// alloc a new one and put it into the list
	auto *ptr = reinterpret_cast<uint32_t *>(new uint8_t[size + sizeof(uint32_t)]);
	for (int scan = 0; scan < MAX_ZLIB_ALLOCS; scan++)
		if (codec->m_allocptr[scan] == nullptr)
		{
			codec->m_allocptr[scan] = ptr;
			break;
		}

	// set the low bit of the size so we don't match next time
	*ptr = size | 1;
	return ptr + 1;
}


//-------------------------------------------------
//  zlib_fast_free - fast free for ZLIB, which
//  allocates and frees memory frequently
//-------------------------------------------------

void chd_zlib_allocator::fast_free(voidpf opaque, voidpf address)
{
	auto *codec = reinterpret_cast<chd_zlib_allocator *>(opaque);

	// find the hunk
	uint32_t *ptr = reinterpret_cast<uint32_t *>(address) - 1;
	for (int scan = 0; scan < MAX_ZLIB_ALLOCS; scan++)
		if (ptr == codec->m_allocptr[scan])
		{
			// clear the low bit of the size to allow matches
			*ptr &= ~1;
			return;
		}
}



//**************************************************************************
//  ZLIB COMPRESSOR
//**************************************************************************

//-------------------------------------------------
//  chd_zlib_compressor - constructor
//-------------------------------------------------

chd_zlib_compressor::chd_zlib_compressor(chd_file &chd, uint32_t hunkbytes, bool lossy)
	: chd_compressor(chd, hunkbytes, lossy)
{
	// initialize the deflater
	m_deflater.next_in = (Bytef *)this; // bogus, but that's ok
	m_deflater.avail_in = 0;
	m_allocator.install(m_deflater);
	int zerr = deflateInit2(&m_deflater, Z_BEST_COMPRESSION, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY);

	// convert errors
	if (zerr == Z_MEM_ERROR)
		throw std::bad_alloc();
	else if (zerr != Z_OK)
		throw std::error_condition(chd_file::error::CODEC_ERROR);
}


//-------------------------------------------------
//  ~chd_zlib_compressor - destructor
//-------------------------------------------------

chd_zlib_compressor::~chd_zlib_compressor()
{
	deflateEnd(&m_deflater);
}


//-------------------------------------------------
//  compress - compress data using the ZLIB codec
//-------------------------------------------------

uint32_t chd_zlib_compressor::compress(const uint8_t *src, uint32_t srclen, uint8_t *dest)
{
	// reset the compressor
	m_deflater.next_in = const_cast<Bytef *>(src);
	m_deflater.avail_in = srclen;
	m_deflater.total_in = 0;
	m_deflater.next_out = dest;
	m_deflater.avail_out = srclen;
	m_deflater.total_out = 0;
	int zerr = deflateReset(&m_deflater);
	if (zerr != Z_OK)
		throw std::error_condition(chd_file::error::COMPRESSION_ERROR);

	// do it
	zerr = deflate(&m_deflater, Z_FINISH);

	// if we ended up with more data than we started with, return an error
	if (zerr != Z_STREAM_END || m_deflater.total_out >= srclen)
		throw std::error_condition(chd_file::error::COMPRESSION_ERROR);

	// otherwise, return the length
	return m_deflater.total_out;
}



//**************************************************************************
//  ZLIB DECOMPRESSOR
//**************************************************************************

//-------------------------------------------------
//  chd_zlib_decompressor - constructor
//-------------------------------------------------

chd_zlib_decompressor::chd_zlib_decompressor(chd_file &chd, uint32_t hunkbytes, bool lossy)
	: chd_decompressor(chd, hunkbytes, lossy)
{
	// init the inflater
	m_inflater.next_in = (Bytef *)this; // bogus, but that's ok
	m_inflater.avail_in = 0;
	m_allocator.install(m_inflater);
	int zerr = inflateInit2(&m_inflater, -MAX_WBITS);

	// convert errors
	if (zerr == Z_MEM_ERROR)
		throw std::bad_alloc();
	else if (zerr != Z_OK)
		throw std::error_condition(chd_file::error::CODEC_ERROR);
}


//-------------------------------------------------
//  ~chd_zlib_decompressor - destructor
//-------------------------------------------------

chd_zlib_decompressor::~chd_zlib_decompressor()
{
	inflateEnd(&m_inflater);
}


//-------------------------------------------------
//  decompress - decompress data using the ZLIB
//  codec
//-------------------------------------------------

void chd_zlib_decompressor::decompress(const uint8_t *src, uint32_t complen, uint8_t *dest, uint32_t destlen)
{
	// reset the decompressor
	m_inflater.next_in = const_cast<Bytef *>(src);
	m_inflater.avail_in = complen;
	m_inflater.total_in = 0;
	m_inflater.next_out = dest;
	m_inflater.avail_out = destlen;
	m_inflater.total_out = 0;
	int zerr = inflateReset(&m_inflater);
	if (zerr != Z_OK)
		throw std::error_condition(chd_file::error::DECOMPRESSION_ERROR);

	// do it
	zerr = inflate(&m_inflater, Z_FINISH);
	if (zerr != Z_STREAM_END)
		throw std::error_condition(chd_file::error::DECOMPRESSION_ERROR);
	if (m_inflater.total_out != destlen)
		throw std::error_condition(chd_file::error::DECOMPRESSION_ERROR);
}



//**************************************************************************
//  ZSTANDARD COMPRESSOR
//**************************************************************************

//-------------------------------------------------
//  chd_zstd_compressor - constructor
//-------------------------------------------------

chd_zstd_compressor::chd_zstd_compressor(chd_file &chd, uint32_t hunkbytes, bool lossy)
	: chd_compressor(chd, hunkbytes, lossy)
	, m_stream(nullptr)
{
	// initialize the stream
	m_stream = ZSTD_createCStream();

	// convert errors
	if (!m_stream)
		throw std::bad_alloc();
}


//-------------------------------------------------
//  ~chd_zstd_compressor - destructor
//-------------------------------------------------

chd_zstd_compressor::~chd_zstd_compressor()
{
	ZSTD_freeCStream(m_stream);
}


//-------------------------------------------------
//  compress - compress data using the Zstandard
//  codec
//-------------------------------------------------

uint32_t chd_zstd_compressor::compress(const uint8_t *src, uint32_t srclen, uint8_t *dest)
{
	// reset the compressor
	auto result = ZSTD_initCStream(m_stream, ZSTD_maxCLevel());
	if (ZSTD_isError(result))
		throw std::error_condition(chd_file::error::COMPRESSION_ERROR);

	// do it
	ZSTD_inBuffer input{ src, srclen, 0 };
	ZSTD_outBuffer output = { dest, srclen, 0 };
	while (output.pos < output.size)
	{
		result = ZSTD_compressStream2(m_stream, &output, &input, ZSTD_e_end);
		if (ZSTD_isError(result))
			throw std::error_condition(chd_file::error::COMPRESSION_ERROR);
		else if (!result)
			break;
	}

	// if we ended up with more data than we started with, return an error
	if (output.pos == output.size)
		throw std::error_condition(chd_file::error::COMPRESSION_ERROR);

	// otherwise, return the length
	return output.pos;
}



//**************************************************************************
//  ZSTANDARD DECOMPRESSOR
//**************************************************************************

//-------------------------------------------------
//  chd_zstd_compressor - constructor
//-------------------------------------------------

chd_zstd_decompressor::chd_zstd_decompressor(chd_file &chd, uint32_t hunkbytes, bool lossy)
	: chd_decompressor(chd, hunkbytes, lossy)
	, m_stream(nullptr)
{
	// initialize the stream
	m_stream = ZSTD_createDStream();

	// convert errors
	if (!m_stream)
		throw std::bad_alloc();
}


//-------------------------------------------------
//  ~chd_zstd_decompressor - destructor
//-------------------------------------------------

chd_zstd_decompressor::~chd_zstd_decompressor()
{
	ZSTD_freeDStream(m_stream);
}


//-------------------------------------------------
//  decompress - decompress data using the
//  Zstandard codec
//-------------------------------------------------

void chd_zstd_decompressor::decompress(const uint8_t *src, uint32_t complen, uint8_t *dest, uint32_t destlen)
{
	// reset the decompressor
	auto result = ZSTD_initDStream(m_stream);
	if (ZSTD_isError(result))
		throw std::error_condition(chd_file::error::DECOMPRESSION_ERROR);

	// do it
	ZSTD_inBuffer input{ src, complen, 0 };
	ZSTD_outBuffer output = { dest, destlen, 0 };
	while ((input.pos < input.size) && (output.pos < output.size))
	{
		result = ZSTD_decompressStream(m_stream, &output, &input);
		if (ZSTD_isError(result))
			throw std::error_condition(chd_file::error::DECOMPRESSION_ERROR);
	}

	// ensure the expected amount of output was generated
	if (output.pos != output.size)
		throw std::error_condition(chd_file::error::DECOMPRESSION_ERROR);
}



//**************************************************************************
//  LZMA ALLOCATOR HELPER
//**************************************************************************

//-------------------------------------------------
//  chd_lzma_allocator - constructor
//-------------------------------------------------

chd_lzma_allocator::chd_lzma_allocator()
{
	// reset pointer list
	memset(m_allocptr, 0, sizeof(m_allocptr));

	// set our pointers
	Alloc = &chd_lzma_allocator::fast_alloc;
	Free = &chd_lzma_allocator::fast_free;
}


//-------------------------------------------------
//  ~chd_lzma_allocator - constructor
//-------------------------------------------------

chd_lzma_allocator::~chd_lzma_allocator()
{
	// free our memory
	for (auto & elem : m_allocptr)
		delete[] elem;
}


//-------------------------------------------------
//  lzma_fast_alloc - fast malloc for lzma, which
//  allocates and frees memory frequently
//-------------------------------------------------

void *chd_lzma_allocator::fast_alloc(ISzAllocPtr p, size_t size)
{
	auto *const codec = static_cast<chd_lzma_allocator *>(const_cast<ISzAlloc *>(p));

	// compute the size, rounding to the nearest 1k
	size = (size + 0x3ff) & ~0x3ff;

	// reuse a hunk if we can
	for (int scan = 0; scan < MAX_LZMA_ALLOCS; scan++)
	{
		uint32_t *ptr = codec->m_allocptr[scan];
		if (ptr != nullptr && size == *ptr)
		{
			// set the low bit of the size so we don't match next time
			*ptr |= 1;
			return ptr + 1;
		}
	}

	// alloc a new one and put it into the list
	auto *ptr = reinterpret_cast<uint32_t *>(new uint8_t[size + sizeof(uint32_t)]);
	for (int scan = 0; scan < MAX_LZMA_ALLOCS; scan++)
		if (codec->m_allocptr[scan] == nullptr)
		{
			codec->m_allocptr[scan] = ptr;
			break;
		}

	// set the low bit of the size so we don't match next time
	*ptr = size | 1;
	return ptr + 1;
}


//-------------------------------------------------
//  lzma_fast_free - fast free for lzma, which
//  allocates and frees memory frequently
//-------------------------------------------------

void chd_lzma_allocator::fast_free(ISzAllocPtr p, void *address)
{
	if (address == nullptr)
		return;

	auto *const codec = static_cast<chd_lzma_allocator *>(const_cast<ISzAlloc *>(p));

	// find the hunk
	uint32_t *ptr = reinterpret_cast<uint32_t *>(address) - 1;
	for (int scan = 0; scan < MAX_LZMA_ALLOCS; scan++)
		if (ptr == codec->m_allocptr[scan])
		{
			// clear the low bit of the size to allow matches
			*ptr &= ~1;
			return;
		}
}



//**************************************************************************
//  LZMA COMPRESSOR
//**************************************************************************

//-------------------------------------------------
//  chd_lzma_compressor - constructor
//-------------------------------------------------

chd_lzma_compressor::chd_lzma_compressor(chd_file &chd, uint32_t hunkbytes, bool lossy)
	: chd_compressor(chd, hunkbytes, lossy)
{
	// initialize the properties
	configure_properties(m_props, hunkbytes);
}


//-------------------------------------------------
//  ~chd_lzma_compressor - destructor
//-------------------------------------------------

chd_lzma_compressor::~chd_lzma_compressor()
{
}


//-------------------------------------------------
//  compress - compress data using the LZMA codec
//-------------------------------------------------

uint32_t chd_lzma_compressor::compress(const uint8_t *src, uint32_t srclen, uint8_t *dest)
{
	// allocate the encoder
	CLzmaEncHandle encoder = LzmaEnc_Create(&m_allocator);
	if (encoder == nullptr)
		throw std::error_condition(chd_file::error::COMPRESSION_ERROR);

	try
	{
		// configure the encoder
		SRes res = LzmaEnc_SetProps(encoder, &m_props);
		if (res != SZ_OK)
			throw std::error_condition(chd_file::error::COMPRESSION_ERROR);

		// run it
		SizeT complen = srclen;
		res = LzmaEnc_MemEncode(encoder, dest, &complen, src, srclen, 0, nullptr, &m_allocator, &m_allocator);
		if (res != SZ_OK)
			throw std::error_condition(chd_file::error::COMPRESSION_ERROR);

		// clean up
		LzmaEnc_Destroy(encoder, &m_allocator, &m_allocator);
		return complen;
	}
	catch (...)
	{
		// destroy before re-throwing
		LzmaEnc_Destroy(encoder, &m_allocator, &m_allocator);
		throw;
	}
}


//-------------------------------------------------
//  configure_properties - configure the LZMA
//  codec
//-------------------------------------------------

void chd_lzma_compressor::configure_properties(CLzmaEncProps &props, uint32_t hunkbytes)
{
	LzmaEncProps_Init(&props);
	props.level = 8;
	props.reduceSize = hunkbytes;
	LzmaEncProps_Normalize(&props);
}



//**************************************************************************
//  LZMA DECOMPRESSOR
//**************************************************************************

//-------------------------------------------------
//  chd_lzma_decompressor - constructor
//-------------------------------------------------

chd_lzma_decompressor::chd_lzma_decompressor(chd_file &chd, uint32_t hunkbytes, bool lossy)
	: chd_decompressor(chd, hunkbytes, lossy)
{
	// construct the decoder
	LzmaDec_Construct(&m_decoder);

	// FIXME: this code is written in a way that makes it impossible to safely upgrade the LZMA SDK
	// This code assumes that the current version of the encoder imposes the same requirements on the
	// decoder as the encoder used to produce the file.  This is not necessarily true.  The format
	// needs to be changed so the encoder properties are written to the file.

	// configure the properties like the compressor did
	CLzmaEncProps encoder_props;
	chd_lzma_compressor::configure_properties(encoder_props, hunkbytes);

	// convert to decoder properties
	CLzmaEncHandle enc = LzmaEnc_Create(&m_allocator);
	if (!enc)
		throw std::error_condition(chd_file::error::DECOMPRESSION_ERROR);
	if (LzmaEnc_SetProps(enc, &encoder_props) != SZ_OK)
	{
		LzmaEnc_Destroy(enc, &m_allocator, &m_allocator);
		throw std::error_condition(chd_file::error::DECOMPRESSION_ERROR);
	}
	Byte decoder_props[LZMA_PROPS_SIZE];
	SizeT props_size = sizeof(decoder_props);
	if (LzmaEnc_WriteProperties(enc, decoder_props, &props_size) != SZ_OK)
	{
		LzmaEnc_Destroy(enc, &m_allocator, &m_allocator);
		throw std::error_condition(chd_file::error::DECOMPRESSION_ERROR);
	}
	LzmaEnc_Destroy(enc, &m_allocator, &m_allocator);

	// do memory allocations
	if (LzmaDec_Allocate(&m_decoder, decoder_props, LZMA_PROPS_SIZE, &m_allocator) != SZ_OK)
		throw std::error_condition(chd_file::error::DECOMPRESSION_ERROR);
}


//-------------------------------------------------
//  ~chd_lzma_decompressor - destructor
//-------------------------------------------------

chd_lzma_decompressor::~chd_lzma_decompressor()
{
	// free memory
	LzmaDec_Free(&m_decoder, &m_allocator);
}


//-------------------------------------------------
//  decompress - decompress data using the LZMA
//  codec
//-------------------------------------------------

void chd_lzma_decompressor::decompress(const uint8_t *src, uint32_t complen, uint8_t *dest, uint32_t destlen)
{
	// initialize
	LzmaDec_Init(&m_decoder);

	// decode
	SizeT consumedlen = complen;
	SizeT decodedlen = destlen;
	ELzmaStatus status;
	SRes res = LzmaDec_DecodeToBuf(&m_decoder, dest, &decodedlen, src, &consumedlen, LZMA_FINISH_END, &status);
	if ((res != SZ_OK && res != LZMA_STATUS_MAYBE_FINISHED_WITHOUT_MARK) || consumedlen != complen || decodedlen != destlen)
		throw std::error_condition(chd_file::error::DECOMPRESSION_ERROR);
}



//**************************************************************************
//  HUFFMAN COMPRESSOR
//**************************************************************************

//-------------------------------------------------
//  chd_huffman_compressor - constructor
//-------------------------------------------------

chd_huffman_compressor::chd_huffman_compressor(chd_file &chd, uint32_t hunkbytes, bool lossy)
	: chd_compressor(chd, hunkbytes, lossy)
{
}


//-------------------------------------------------
//  compress - compress data using the Huffman
//  codec
//-------------------------------------------------

uint32_t chd_huffman_compressor::compress(const uint8_t *src, uint32_t srclen, uint8_t *dest)
{
	uint32_t complen;
	if (m_encoder.encode(src, srclen, dest, srclen, complen) != HUFFERR_NONE)
		throw std::error_condition(chd_file::error::COMPRESSION_ERROR);
	return complen;
}



//**************************************************************************
//  HUFFMAN DECOMPRESSOR
//**************************************************************************

//-------------------------------------------------
//  chd_huffman_decompressor - constructor
//-------------------------------------------------

chd_huffman_decompressor::chd_huffman_decompressor(chd_file &chd, uint32_t hunkbytes, bool lossy)
	: chd_decompressor(chd, hunkbytes, lossy)
{
}


//-------------------------------------------------
//  decompress - decompress data using the Huffman
//  codec
//-------------------------------------------------

void chd_huffman_decompressor::decompress(const uint8_t *src, uint32_t complen, uint8_t *dest, uint32_t destlen)
{
	if (m_decoder.decode(src, complen, dest, destlen) != HUFFERR_NONE)
		throw std::error_condition(chd_file::error::COMPRESSION_ERROR);
}



//**************************************************************************
//  FLAC COMPRESSOR
//**************************************************************************

//-------------------------------------------------
//  chd_flac_compressor - constructor
//-------------------------------------------------

chd_flac_compressor::chd_flac_compressor(chd_file &chd, uint32_t hunkbytes, bool lossy)
	: chd_compressor(chd, hunkbytes, lossy)
{
	// determine whether we want native or swapped samples
	uint16_t native_endian = 0;
	*reinterpret_cast<uint8_t *>(&native_endian) = 1;
	m_big_endian = (native_endian == 0x100);

	// configure the encoder
	m_encoder.set_sample_rate(44100);
	m_encoder.set_num_channels(2);
	m_encoder.set_block_size(blocksize(hunkbytes));
	m_encoder.set_strip_metadata(true);
}


//-------------------------------------------------
//  compress - compress data using the FLAC codec
//-------------------------------------------------

uint32_t chd_flac_compressor::compress(const uint8_t *src, uint32_t srclen, uint8_t *dest)
{
	// reset and encode big-endian
	m_encoder.reset(dest + 1, hunkbytes() - 1);
	if (!m_encoder.encode_interleaved(reinterpret_cast<const int16_t *>(src), srclen / 4, !m_big_endian))
		throw std::error_condition(chd_file::error::COMPRESSION_ERROR);
	uint32_t complen_be = m_encoder.finish();

	// reset and encode little-endian
	m_encoder.reset(dest + 1, hunkbytes() - 1);
	if (!m_encoder.encode_interleaved(reinterpret_cast<const int16_t *>(src), srclen / 4, m_big_endian))
		throw std::error_condition(chd_file::error::COMPRESSION_ERROR);
	uint32_t complen_le = m_encoder.finish();

	// pick the best one and add a byte
	uint32_t complen = std::min(complen_le, complen_be);
	if (complen + 1 >= hunkbytes())
		throw std::error_condition(chd_file::error::COMPRESSION_ERROR);

	// if big-endian was better, re-do it
	dest[0] = 'L';
	if (complen != complen_le)
	{
		dest[0] = 'B';
		m_encoder.reset(dest + 1, hunkbytes() - 1);
		if (!m_encoder.encode_interleaved(reinterpret_cast<const int16_t *>(src), srclen / 4, !m_big_endian))
			throw std::error_condition(chd_file::error::COMPRESSION_ERROR);
		m_encoder.finish();
	}
	return complen + 1;
}


//-------------------------------------------------
//  blocksize - return the optimal block size
//-------------------------------------------------

uint32_t chd_flac_compressor::blocksize(uint32_t bytes)
{
	// determine FLAC block size, which must be 16-65535
	// clamp to 2k since that's supposed to be the sweet spot
	uint32_t hunkbytes = bytes / 4;
	while (hunkbytes > 2048)
		hunkbytes /= 2;
	return hunkbytes;
}



//**************************************************************************
//  FLAC DECOMPRESSOR
//**************************************************************************

//-------------------------------------------------
//  chd_flac_decompressor - constructor
//-------------------------------------------------

chd_flac_decompressor::chd_flac_decompressor(chd_file &chd, uint32_t hunkbytes, bool lossy)
	: chd_decompressor(chd, hunkbytes, lossy)
{
	// determine whether we want native or swapped samples
	uint16_t native_endian = 0;
	*reinterpret_cast<uint8_t *>(&native_endian) = 1;
	m_big_endian = (native_endian == 0x100);
}


//-------------------------------------------------
//  decompress - decompress data using the FLAC
//  codec
//-------------------------------------------------

void chd_flac_decompressor::decompress(const uint8_t *src, uint32_t complen, uint8_t *dest, uint32_t destlen)
{
	// determine the endianness
	bool swap_endian;
	if (src[0] == 'L')
		swap_endian = m_big_endian;
	else if (src[0] == 'B')
		swap_endian = !m_big_endian;
	else
		throw std::error_condition(chd_file::error::DECOMPRESSION_ERROR);

	// reset and decode
	if (!m_decoder.reset(44100, 2, chd_flac_compressor::blocksize(destlen), src + 1, complen - 1))
		throw std::error_condition(chd_file::error::DECOMPRESSION_ERROR);
	if (!m_decoder.decode_interleaved(reinterpret_cast<int16_t *>(dest), destlen / 4, swap_endian))
		throw std::error_condition(chd_file::error::DECOMPRESSION_ERROR);

	// finish up
	m_decoder.finish();
}



//**************************************************************************
//  CD FLAC COMPRESSOR
//**************************************************************************

//-------------------------------------------------
//  chd_cd_flac_compressor - constructor
//-------------------------------------------------

chd_cd_flac_compressor::chd_cd_flac_compressor(chd_file &chd, uint32_t hunkbytes, bool lossy)
	: chd_compressor(chd, hunkbytes, lossy)
	, m_buffer(hunkbytes)
{
	// make sure the CHD's hunk size is an even multiple of the frame size
	if (hunkbytes % cdrom_file::FRAME_SIZE != 0)
		throw std::error_condition(chd_file::error::CODEC_ERROR);

	// determine whether we want native or swapped samples
	uint16_t native_endian = 0;
	*reinterpret_cast<uint8_t *>(&native_endian) = 1;
	m_swap_endian = (native_endian == 1);

	// configure the encoder
	m_encoder.set_sample_rate(44100);
	m_encoder.set_num_channels(2);
	m_encoder.set_block_size(blocksize((hunkbytes / cdrom_file::FRAME_SIZE) * cdrom_file::MAX_SECTOR_DATA));
	m_encoder.set_strip_metadata(true);

	// initialize the deflater
	m_deflater.next_in = (Bytef *)this; // bogus, but that's ok
	m_deflater.avail_in = 0;
	m_allocator.install(m_deflater);
	int zerr = deflateInit2(&m_deflater, Z_BEST_COMPRESSION, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY);

	// convert errors
	if (zerr == Z_MEM_ERROR)
		throw std::bad_alloc();
	else if (zerr != Z_OK)
		throw std::error_condition(chd_file::error::CODEC_ERROR);
}


//-------------------------------------------------
//  ~chd_cd_flac_compressor - destructor
//-------------------------------------------------

chd_cd_flac_compressor::~chd_cd_flac_compressor()
{
	deflateEnd(&m_deflater);
}


//-------------------------------------------------
//  compress - compress data using the FLAC codec,
//  and use zlib on the subcode data
//-------------------------------------------------

uint32_t chd_cd_flac_compressor::compress(const uint8_t *src, uint32_t srclen, uint8_t *dest)
{
	// copy audio data followed by subcode data
	uint32_t frames = hunkbytes() / cdrom_file::FRAME_SIZE;
	for (uint32_t framenum = 0; framenum < frames; framenum++)
	{
		memcpy(&m_buffer[framenum * cdrom_file::MAX_SECTOR_DATA], &src[framenum * cdrom_file::FRAME_SIZE], cdrom_file::MAX_SECTOR_DATA);
		memcpy(&m_buffer[frames * cdrom_file::MAX_SECTOR_DATA + framenum * cdrom_file::MAX_SUBCODE_DATA], &src[framenum * cdrom_file::FRAME_SIZE + cdrom_file::MAX_SECTOR_DATA], cdrom_file::MAX_SUBCODE_DATA);
	}

	// reset and encode the audio portion
	m_encoder.reset(dest, hunkbytes());
	uint8_t *buffer = &m_buffer[0];
	if (!m_encoder.encode_interleaved(reinterpret_cast<int16_t *>(buffer), frames * cdrom_file::MAX_SECTOR_DATA/4, m_swap_endian))
		throw std::error_condition(chd_file::error::COMPRESSION_ERROR);

	// finish up
	uint32_t complen = m_encoder.finish();

	// deflate the subcode data
	m_deflater.next_in = const_cast<Bytef *>(&m_buffer[frames * cdrom_file::MAX_SECTOR_DATA]);
	m_deflater.avail_in = frames * cdrom_file::MAX_SUBCODE_DATA;
	m_deflater.total_in = 0;
	m_deflater.next_out = &dest[complen];
	m_deflater.avail_out = hunkbytes() - complen;
	m_deflater.total_out = 0;
	int zerr = deflateReset(&m_deflater);
	if (zerr != Z_OK)
		throw std::error_condition(chd_file::error::COMPRESSION_ERROR);

	// do it
	zerr = deflate(&m_deflater, Z_FINISH);

	// if we ended up with more data than we started with, return an error
	complen += m_deflater.total_out;
	if (zerr != Z_STREAM_END || complen >= srclen)
		throw std::error_condition(chd_file::error::COMPRESSION_ERROR);
	return complen;
}

/**
 * @fn  uint32_t chd_cd_flac_compressor::blocksize(uint32_t bytes)
 *
 * @brief   -------------------------------------------------
 *            blocksize - return the optimal block size
 *          -------------------------------------------------.
 *
 * @param   bytes   The bytes.
 *
 * @return  An uint32_t.
 */

uint32_t chd_cd_flac_compressor::blocksize(uint32_t bytes)
{
	// for CDs it seems that CD_MAX_SECTOR_DATA is the right target
	uint32_t blocksize = bytes / 4;
	while (blocksize > cdrom_file::MAX_SECTOR_DATA)
		blocksize /= 2;
	return blocksize;
}



//**************************************************************************
//  CD FLAC DECOMPRESSOR
//**************************************************************************

/**
 * @fn  chd_cd_flac_decompressor::chd_cd_flac_decompressor(chd_file &chd, uint32_t hunkbytes, bool lossy)
 *
 * @brief   -------------------------------------------------
 *            chd_cd_flac_decompressor - constructor
 *          -------------------------------------------------.
 *
 * @exception   CHDERR_CODEC_ERROR  Thrown when a chderr codec error error condition occurs.
 *
 * @param [in,out]  chd The chd.
 * @param   hunkbytes   The hunkbytes.
 * @param   lossy       true to lossy.
 */

chd_cd_flac_decompressor::chd_cd_flac_decompressor(chd_file &chd, uint32_t hunkbytes, bool lossy)
	: chd_decompressor(chd, hunkbytes, lossy)
	, m_buffer(hunkbytes)
{
	// make sure the CHD's hunk size is an even multiple of the frame size
	if (hunkbytes % cdrom_file::FRAME_SIZE != 0)
		throw std::error_condition(chd_file::error::CODEC_ERROR);

	// determine whether we want native or swapped samples
	uint16_t native_endian = 0;
	*reinterpret_cast<uint8_t *>(&native_endian) = 1;
	m_swap_endian = (native_endian == 1);

	// init the inflater
	m_inflater.next_in = (Bytef *)this; // bogus, but that's ok
	m_inflater.avail_in = 0;
	m_allocator.install(m_inflater);
	int zerr = inflateInit2(&m_inflater, -MAX_WBITS);

	// convert errors
	if (zerr == Z_MEM_ERROR)
		throw std::bad_alloc();
	else if (zerr != Z_OK)
		throw std::error_condition(chd_file::error::CODEC_ERROR);
}

/**
 * @fn  chd_cd_flac_decompressor::~chd_cd_flac_decompressor()
 *
 * @brief   -------------------------------------------------
 *            ~chd_cd_flac_decompressor - destructor
 *          -------------------------------------------------.
 */

chd_cd_flac_decompressor::~chd_cd_flac_decompressor()
{
	inflateEnd(&m_inflater);
}

/**
 * @fn  void chd_cd_flac_decompressor::decompress(const uint8_t *src, uint32_t complen, uint8_t *dest, uint32_t destlen)
 *
 * @brief   -------------------------------------------------
 *            decompress - decompress data using the FLAC codec
 *          -------------------------------------------------.
 *
 * @exception   CHDERR_DECOMPRESSION_ERROR  Thrown when a chderr decompression error error
 *                                          condition occurs.
 *
 * @param   src             Source for the.
 * @param   complen         The complen.
 * @param [in,out]  dest    If non-null, destination for the.
 * @param   destlen         The destlen.
 */

void chd_cd_flac_decompressor::decompress(const uint8_t *src, uint32_t complen, uint8_t *dest, uint32_t destlen)
{
	// reset and decode
	uint32_t frames = destlen / cdrom_file::FRAME_SIZE;
	if (!m_decoder.reset(44100, 2, chd_cd_flac_compressor::blocksize(frames * cdrom_file::MAX_SECTOR_DATA), src, complen))
		throw std::error_condition(chd_file::error::DECOMPRESSION_ERROR);
	uint8_t *buffer = &m_buffer[0];
	if (!m_decoder.decode_interleaved(reinterpret_cast<int16_t *>(buffer), frames * cdrom_file::MAX_SECTOR_DATA/4, m_swap_endian))
		throw std::error_condition(chd_file::error::DECOMPRESSION_ERROR);

	// inflate the subcode data
	uint32_t offset = m_decoder.finish();
	m_inflater.next_in = const_cast<Bytef *>(src + offset);
	m_inflater.avail_in = complen - offset;
	m_inflater.total_in = 0;
	m_inflater.next_out = &m_buffer[frames * cdrom_file::MAX_SECTOR_DATA];
	m_inflater.avail_out = frames * cdrom_file::MAX_SUBCODE_DATA;
	m_inflater.total_out = 0;
	int zerr = inflateReset(&m_inflater);
	if (zerr != Z_OK)
		throw std::error_condition(chd_file::error::DECOMPRESSION_ERROR);

	// do it
	zerr = inflate(&m_inflater, Z_FINISH);
	if (zerr != Z_STREAM_END)
		throw std::error_condition(chd_file::error::DECOMPRESSION_ERROR);
	if (m_inflater.total_out != frames * cdrom_file::MAX_SUBCODE_DATA)
		throw std::error_condition(chd_file::error::DECOMPRESSION_ERROR);

	// reassemble the data
	for (uint32_t framenum = 0; framenum < frames; framenum++)
	{
		memcpy(&dest[framenum * cdrom_file::FRAME_SIZE], &m_buffer[framenum * cdrom_file::MAX_SECTOR_DATA], cdrom_file::MAX_SECTOR_DATA);
		memcpy(&dest[framenum * cdrom_file::FRAME_SIZE + cdrom_file::MAX_SECTOR_DATA], &m_buffer[frames * cdrom_file::MAX_SECTOR_DATA + framenum * cdrom_file::MAX_SUBCODE_DATA], cdrom_file::MAX_SUBCODE_DATA);
	}
}



//**************************************************************************
//  AVHUFF COMPRESSOR
//**************************************************************************

/**
 * @fn  chd_avhuff_compressor::chd_avhuff_compressor(chd_file &chd, uint32_t hunkbytes, bool lossy)
 *
 * @brief   -------------------------------------------------
 *            chd_avhuff_compressor - constructor
 *          -------------------------------------------------.
 *
 * @param [in,out]  chd The chd.
 * @param   hunkbytes   The hunkbytes.
 * @param   lossy       true to lossy.
 */

chd_avhuff_compressor::chd_avhuff_compressor(chd_file &chd, uint32_t hunkbytes, bool lossy)
	: chd_compressor(chd, hunkbytes, lossy)
	, m_postinit(false)
{
	try
	{
		// attempt to do a post-init now
		postinit();
	}
	catch (std::error_condition const &)
	{
		// if we're creating a new CHD, it won't work but that's ok
	}
}

/**
 * @fn  uint32_t chd_avhuff_compressor::compress(const uint8_t *src, uint32_t srclen, uint8_t *dest)
 *
 * @brief   -------------------------------------------------
 *            compress - compress data using the A/V codec
 *          -------------------------------------------------.
 *
 * @exception   CHDERR_INVALID_DATA         Thrown when a chderr invalid data error condition
 *                                          occurs.
 * @exception   CHDERR_COMPRESSION_ERROR    Thrown when a chderr compression error error
 *                                          condition occurs.
 *
 * @param   src             Source for the.
 * @param   srclen          The srclen.
 * @param [in,out]  dest    If non-null, destination for the.
 *
 * @return  An uint32_t.
 */

uint32_t chd_avhuff_compressor::compress(const uint8_t *src, uint32_t srclen, uint8_t *dest)
{
	// if we haven't yet set up the avhuff code, do it now
	if (!m_postinit)
		postinit();

	// make sure short frames are padded with 0
	if (src != nullptr)
	{
		int size = avhuff_encoder::raw_data_size(src);
		while (size < srclen)
			if (src[size++] != 0)
				throw std::error_condition(chd_file::error::INVALID_DATA);
	}

	// encode the audio and video
	uint32_t complen;
	avhuff_error averr = m_encoder.encode_data(src, dest, complen);
	if (averr != AVHERR_NONE || complen > srclen)
		throw std::error_condition(chd_file::error::COMPRESSION_ERROR);
	return complen;
}

/**
 * @fn  void chd_avhuff_compressor::postinit()
 *
 * @brief   -------------------------------------------------
 *            postinit - actual initialization of avhuff happens here, on the first attempt to
 *            compress or decompress data
 *          -------------------------------------------------.
 *
 * @exception   err                     Thrown when an error error condition occurs.
 * @exception   CHDERR_INVALID_METADATA Thrown when a chderr invalid metadata error condition
 *                                      occurs.
 */

void chd_avhuff_compressor::postinit()
{
	// get the metadata
	std::string metadata;
	std::error_condition err = chd().read_metadata(AV_METADATA_TAG, 0, metadata);
	if (err)
		throw err;

	// extract the info
	int fps, fpsfrac, width, height, interlaced, channels, rate;
	if (sscanf(metadata.c_str(), AV_METADATA_FORMAT, &fps, &fpsfrac, &width, &height, &interlaced, &channels, &rate) != 7)
		throw std::error_condition(chd_file::error::INVALID_METADATA);

	// compute the bytes per frame
	uint32_t fps_times_1million = fps * 1000000 + fpsfrac;
	uint32_t max_samples_per_frame = (uint64_t(rate) * 1000000 + fps_times_1million - 1) / fps_times_1million;
	uint32_t bytes_per_frame = 12 + channels * max_samples_per_frame * 2 + width * height * 2;
	if (bytes_per_frame > hunkbytes())
		throw std::error_condition(chd_file::error::INVALID_METADATA);

	// done with post-init
	m_postinit = true;
}



//**************************************************************************
//  AVHUFF DECOMPRESSOR
//**************************************************************************

/**
 * @fn  chd_avhuff_decompressor::chd_avhuff_decompressor(chd_file &chd, uint32_t hunkbytes, bool lossy)
 *
 * @brief   -------------------------------------------------
 *            chd_avhuff_decompressor - constructor
 *          -------------------------------------------------.
 *
 * @param [in,out]  chd The chd.
 * @param   hunkbytes   The hunkbytes.
 * @param   lossy       true to lossy.
 */

chd_avhuff_decompressor::chd_avhuff_decompressor(chd_file &chd, uint32_t hunkbytes, bool lossy)
	: chd_decompressor(chd, hunkbytes, lossy)
{
}

void chd_avhuff_decompressor::process(const uint8_t *src, uint32_t complen)
{
	// decode the audio and video
	avhuff_error averr = m_decoder.decode_data(src, complen, nullptr);
	if (averr != AVHERR_NONE)
		throw std::error_condition(chd_file::error::DECOMPRESSION_ERROR);
}

void chd_avhuff_decompressor::decompress(const uint8_t *src, uint32_t complen, uint8_t *dest, uint32_t destlen)
{
	// decode the audio and video
	avhuff_error averr = m_decoder.decode_data(src, complen, dest);
	if (averr != AVHERR_NONE)
		throw std::error_condition(chd_file::error::DECOMPRESSION_ERROR);

	// pad short frames with 0
	auto const size = avhuff_encoder::raw_data_size(dest);
	if (size < destlen)
		memset(dest + size, 0, destlen - size);
}

/**
 * @fn  void chd_avhuff_decompressor::configure(int param, void *config)
 *
 * @brief   -------------------------------------------------
 *            config - codec-specific configuration for the A/V codec
 *          -------------------------------------------------.
 *
 * @exception   CHDERR_INVALID_PARAMETER    Thrown when a chderr invalid parameter error
 *                                          condition occurs.
 *
 * @param   param           The parameter.
 * @param [in,out]  config  If non-null, the configuration.
 */

void chd_avhuff_decompressor::configure(int param, void *config)
{
	// if we're getting the decompression configuration, apply it now
	if (param == AVHUFF_CODEC_DECOMPRESS_CONFIG)
		m_decoder.configure(*reinterpret_cast<avhuff_decoder::config const *>(config));

	// anything else is invalid
	else
		throw std::error_condition(std::errc::invalid_argument);
}
