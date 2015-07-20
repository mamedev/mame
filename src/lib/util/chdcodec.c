// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    chdcodec.c

    Codecs used by the CHD format

***************************************************************************/

#include <assert.h>

#include "chd.h"
#include "hashing.h"
#include "avhuff.h"
#include "flac.h"
#include "cdrom.h"
#include <zlib.h>
#include "lzma/C/LzmaEnc.h"
#include "lzma/C/LzmaDec.h"
#include <new>


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

static const UINT8 s_cd_sync_header[12] = { 0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00 };



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> chd_zlib_allocator

// allocation helper clas for zlib
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

	static const int MAX_ZLIB_ALLOCS = 64;
	UINT32 *                m_allocptr[MAX_ZLIB_ALLOCS];
};


// ======================> chd_zlib_compressor

// ZLIB compressor
class chd_zlib_compressor : public chd_compressor
{
public:
	// construction/destruction
	chd_zlib_compressor(chd_file &chd, UINT32 hunkbytes, bool lossy);
	~chd_zlib_compressor();

	// core functionality
	virtual UINT32 compress(const UINT8 *src, UINT32 srclen, UINT8 *dest);

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
	chd_zlib_decompressor(chd_file &chd, UINT32 hunkbytes, bool lossy);
	~chd_zlib_decompressor();

	// core functionality
	virtual void decompress(const UINT8 *src, UINT32 complen, UINT8 *dest, UINT32 destlen);

private:
	// internal state
	z_stream                m_inflater;
	chd_zlib_allocator      m_allocator;
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
	static void *fast_alloc(void *p, size_t size);
	static void fast_free(void *p, void *address);

	static const int MAX_LZMA_ALLOCS = 64;
	UINT32 *                m_allocptr[MAX_LZMA_ALLOCS];
};


// ======================> chd_lzma_compressor

// LZMA compressor
class chd_lzma_compressor : public chd_compressor
{
public:
	// construction/destruction
	chd_lzma_compressor(chd_file &chd, UINT32 hunkbytes, bool lossy);
	~chd_lzma_compressor();

	// core functionality
	virtual UINT32 compress(const UINT8 *src, UINT32 srclen, UINT8 *dest);

	// helpers
	static void configure_properties(CLzmaEncProps &props, UINT32 hunkbytes);

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
	chd_lzma_decompressor(chd_file &chd, UINT32 hunkbytes, bool lossy);
	~chd_lzma_decompressor();

	// core functionality
	virtual void decompress(const UINT8 *src, UINT32 complen, UINT8 *dest, UINT32 destlen);

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
	chd_huffman_compressor(chd_file &chd, UINT32 hunkbytes, bool lossy);

	// core functionality
	virtual UINT32 compress(const UINT8 *src, UINT32 srclen, UINT8 *dest);

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
	chd_huffman_decompressor(chd_file &chd, UINT32 hunkbytes, bool lossy);

	// core functionality
	virtual void decompress(const UINT8 *src, UINT32 complen, UINT8 *dest, UINT32 destlen);

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
	chd_flac_compressor(chd_file &chd, UINT32 hunkbytes, bool lossy);

	// core functionality
	virtual UINT32 compress(const UINT8 *src, UINT32 srclen, UINT8 *dest);

	// static helpers
	static UINT32 blocksize(UINT32 bytes);

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
	chd_flac_decompressor(chd_file &chd, UINT32 hunkbytes, bool lossy);

	// core functionality
	virtual void decompress(const UINT8 *src, UINT32 complen, UINT8 *dest, UINT32 destlen);

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
	chd_cd_flac_compressor(chd_file &chd, UINT32 hunkbytes, bool lossy);
	~chd_cd_flac_compressor();

	// core functionality
	virtual UINT32 compress(const UINT8 *src, UINT32 srclen, UINT8 *dest);

	// static helpers
	static UINT32 blocksize(UINT32 bytes);

private:
	// internal state
	bool                m_swap_endian;
	flac_encoder        m_encoder;
	z_stream            m_deflater;
	chd_zlib_allocator  m_allocator;
	dynamic_buffer      m_buffer;
};


// ======================> chd_cd_flac_decompressor

// FLAC decompressor
class chd_cd_flac_decompressor : public chd_decompressor
{
public:
	// construction/destruction
	chd_cd_flac_decompressor(chd_file &chd, UINT32 hunkbytes, bool lossy);
	~chd_cd_flac_decompressor();

	// core functionality
	virtual void decompress(const UINT8 *src, UINT32 complen, UINT8 *dest, UINT32 destlen);

private:
	// internal state
	bool                m_swap_endian;
	flac_decoder        m_decoder;
	z_stream            m_inflater;
	chd_zlib_allocator  m_allocator;
	dynamic_buffer      m_buffer;
};


// ======================> chd_cd_compressor

template<class _BaseCompressor, class _SubcodeCompressor>
class chd_cd_compressor : public chd_compressor
{
public:
	// construction/destruction
	chd_cd_compressor(chd_file &chd, UINT32 hunkbytes, bool lossy)
		: chd_compressor(chd, hunkbytes, lossy),
			m_base_compressor(chd, (hunkbytes / CD_FRAME_SIZE) * CD_MAX_SECTOR_DATA, lossy),
			m_subcode_compressor(chd, (hunkbytes / CD_FRAME_SIZE) * CD_MAX_SUBCODE_DATA, lossy),
			m_buffer(hunkbytes + (hunkbytes / CD_FRAME_SIZE) * CD_MAX_SUBCODE_DATA)
	{
		// make sure the CHD's hunk size is an even multiple of the frame size
		if (hunkbytes % CD_FRAME_SIZE != 0)
			throw CHDERR_CODEC_ERROR;
	}

	// core functionality
	virtual UINT32 compress(const UINT8 *src, UINT32 srclen, UINT8 *dest)
	{
		// determine header bytes
		UINT32 frames = srclen / CD_FRAME_SIZE;
		UINT32 complen_bytes = (srclen < 65536) ? 2 : 3;
		UINT32 ecc_bytes = (frames + 7) / 8;
		UINT32 header_bytes = ecc_bytes + complen_bytes;

		// clear out destination header
		memset(dest, 0, header_bytes);

		// copy audio data followed by subcode data
		for (UINT32 framenum = 0; framenum < frames; framenum++)
		{
			memcpy(&m_buffer[framenum * CD_MAX_SECTOR_DATA], &src[framenum * CD_FRAME_SIZE], CD_MAX_SECTOR_DATA);
			memcpy(&m_buffer[frames * CD_MAX_SECTOR_DATA + framenum * CD_MAX_SUBCODE_DATA], &src[framenum * CD_FRAME_SIZE + CD_MAX_SECTOR_DATA], CD_MAX_SUBCODE_DATA);

			// clear out ECC data if we can
			UINT8 *sector = &m_buffer[framenum * CD_MAX_SECTOR_DATA];
			if (memcmp(sector, s_cd_sync_header, sizeof(s_cd_sync_header)) == 0 && ecc_verify(sector))
			{
				dest[framenum / 8] |= 1 << (framenum % 8);
				memset(sector, 0, sizeof(s_cd_sync_header));
				ecc_clear(sector);
			}
		}

		// encode the base portion
		UINT32 complen = m_base_compressor.compress(&m_buffer[0], frames * CD_MAX_SECTOR_DATA, &dest[header_bytes]);
		if (complen >= srclen)
			throw CHDERR_COMPRESSION_ERROR;

		// write compressed length
		dest[ecc_bytes + 0] = complen >> ((complen_bytes - 1) * 8);
		dest[ecc_bytes + 1] = complen >> ((complen_bytes - 2) * 8);
		if (complen_bytes > 2)
			dest[ecc_bytes + 2] = complen >> ((complen_bytes - 3) * 8);

		// encode the subcode
		return header_bytes + complen + m_subcode_compressor.compress(&m_buffer[frames * CD_MAX_SECTOR_DATA], frames * CD_MAX_SUBCODE_DATA, &dest[header_bytes + complen]);
	}

private:
	// internal state
	_BaseCompressor     m_base_compressor;
	_SubcodeCompressor  m_subcode_compressor;
	dynamic_buffer      m_buffer;
};


// ======================> chd_cd_decompressor

template<class _BaseDecompressor, class _SubcodeDecompressor>
class chd_cd_decompressor : public chd_decompressor
{
public:
	// construction/destruction
	chd_cd_decompressor(chd_file &chd, UINT32 hunkbytes, bool lossy)
		: chd_decompressor(chd, hunkbytes, lossy),
			m_base_decompressor(chd, (hunkbytes / CD_FRAME_SIZE) * CD_MAX_SECTOR_DATA, lossy),
			m_subcode_decompressor(chd, (hunkbytes / CD_FRAME_SIZE) * CD_MAX_SUBCODE_DATA, lossy),
			m_buffer(hunkbytes)
	{
		// make sure the CHD's hunk size is an even multiple of the frame size
		if (hunkbytes % CD_FRAME_SIZE != 0)
			throw CHDERR_CODEC_ERROR;
	}

	// core functionality
	virtual void decompress(const UINT8 *src, UINT32 complen, UINT8 *dest, UINT32 destlen)
	{
		// determine header bytes
		UINT32 frames = destlen / CD_FRAME_SIZE;
		UINT32 complen_bytes = (destlen < 65536) ? 2 : 3;
		UINT32 ecc_bytes = (frames + 7) / 8;
		UINT32 header_bytes = ecc_bytes + complen_bytes;

		// extract compressed length of base
		UINT32 complen_base = (src[ecc_bytes + 0] << 8) | src[ecc_bytes + 1];
		if (complen_bytes > 2)
			complen_base = (complen_base << 8) | src[ecc_bytes + 2];

		// reset and decode
		m_base_decompressor.decompress(&src[header_bytes], complen_base, &m_buffer[0], frames * CD_MAX_SECTOR_DATA);
		m_subcode_decompressor.decompress(&src[header_bytes + complen_base], complen - complen_base - header_bytes, &m_buffer[frames * CD_MAX_SECTOR_DATA], frames * CD_MAX_SUBCODE_DATA);

		// reassemble the data
		for (UINT32 framenum = 0; framenum < frames; framenum++)
		{
			memcpy(&dest[framenum * CD_FRAME_SIZE], &m_buffer[framenum * CD_MAX_SECTOR_DATA], CD_MAX_SECTOR_DATA);
			memcpy(&dest[framenum * CD_FRAME_SIZE + CD_MAX_SECTOR_DATA], &m_buffer[frames * CD_MAX_SECTOR_DATA + framenum * CD_MAX_SUBCODE_DATA], CD_MAX_SUBCODE_DATA);

			// reconstitute the ECC data and sync header
			UINT8 *sector = &dest[framenum * CD_FRAME_SIZE];
			if ((src[framenum / 8] & (1 << (framenum % 8))) != 0)
			{
				memcpy(sector, s_cd_sync_header, sizeof(s_cd_sync_header));
				ecc_generate(sector);
			}
		}
	}

private:
	// internal state
	_BaseDecompressor   m_base_decompressor;
	_SubcodeDecompressor m_subcode_decompressor;
	dynamic_buffer      m_buffer;
};


// ======================> chd_avhuff_compressor

// A/V compressor
class chd_avhuff_compressor : public chd_compressor
{
public:
	// construction/destruction
	chd_avhuff_compressor(chd_file &chd, UINT32 hunkbytes, bool lossy);

	// core functionality
	virtual UINT32 compress(const UINT8 *src, UINT32 srclen, UINT8 *dest);

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
	chd_avhuff_decompressor(chd_file &chd, UINT32 hunkbytes, bool lossy);

	// core functionality
	virtual void decompress(const UINT8 *src, UINT32 complen, UINT8 *dest, UINT32 destlen);
	virtual void configure(int param, void *config);

private:
	// internal state
	avhuff_decoder              m_decoder;
};



//**************************************************************************
//  CODEC LIST
//**************************************************************************

// static list of available known codecs
const chd_codec_list::codec_entry chd_codec_list::s_codec_list[] =
{
	// general codecs
	{ CHD_CODEC_ZLIB,       false,  "Deflate",              &chd_codec_list::construct_compressor<chd_zlib_compressor>,     &chd_codec_list::construct_decompressor<chd_zlib_decompressor> },
	{ CHD_CODEC_LZMA,       false,  "LZMA",                 &chd_codec_list::construct_compressor<chd_lzma_compressor>,     &chd_codec_list::construct_decompressor<chd_lzma_decompressor> },
	{ CHD_CODEC_HUFFMAN,    false,  "Huffman",              &chd_codec_list::construct_compressor<chd_huffman_compressor>,  &chd_codec_list::construct_decompressor<chd_huffman_decompressor> },
	{ CHD_CODEC_FLAC,       false,  "FLAC",                 &chd_codec_list::construct_compressor<chd_flac_compressor>,     &chd_codec_list::construct_decompressor<chd_flac_decompressor> },

	// general codecs with CD frontend
	{ CHD_CODEC_CD_ZLIB,    false,  "CD Deflate",           &chd_codec_list::construct_compressor<chd_cd_compressor<chd_zlib_compressor, chd_zlib_compressor> >,        &chd_codec_list::construct_decompressor<chd_cd_decompressor<chd_zlib_decompressor, chd_zlib_decompressor> > },
	{ CHD_CODEC_CD_LZMA,    false,  "CD LZMA",              &chd_codec_list::construct_compressor<chd_cd_compressor<chd_lzma_compressor, chd_zlib_compressor> >,        &chd_codec_list::construct_decompressor<chd_cd_decompressor<chd_lzma_decompressor, chd_zlib_decompressor> > },
	{ CHD_CODEC_CD_FLAC,    false,  "CD FLAC",              &chd_codec_list::construct_compressor<chd_cd_flac_compressor>,  &chd_codec_list::construct_decompressor<chd_cd_flac_decompressor> },

	// A/V codecs
	{ CHD_CODEC_AVHUFF,     false,  "A/V Huffman",          &chd_codec_list::construct_compressor<chd_avhuff_compressor>,   &chd_codec_list::construct_decompressor<chd_avhuff_decompressor> },
};



//**************************************************************************
//  CHD CODEC
//**************************************************************************

//-------------------------------------------------
//  chd_codec - constructor
//-------------------------------------------------

chd_codec::chd_codec(chd_file &chd, UINT32 hunkbytes, bool lossy)
	: m_chd(chd),
		m_hunkbytes(hunkbytes),
		m_lossy(lossy)
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
	throw CHDERR_INVALID_PARAMETER;
}



//**************************************************************************
//  CHD COMPRESSOR
//**************************************************************************

//-------------------------------------------------
//  chd_compressor - constructor
//-------------------------------------------------

chd_compressor::chd_compressor(chd_file &chd, UINT32 hunkbytes, bool lossy)
	: chd_codec(chd, hunkbytes, lossy)
{
}



//**************************************************************************
//  CHD DECOMPRESSOR
//**************************************************************************

//-------------------------------------------------
//  chd_decompressor - constructor
//-------------------------------------------------

chd_decompressor::chd_decompressor(chd_file &chd, UINT32 hunkbytes, bool lossy)
	: chd_codec(chd, hunkbytes, lossy)
{
}



//**************************************************************************
//  CHD CODEC LIST
//**************************************************************************

//-------------------------------------------------
//  new_compressor - create a new compressor
//  instance of the given type
//-------------------------------------------------

chd_compressor *chd_codec_list::new_compressor(chd_codec_type type, chd_file &chd)
{
	// find in the list and construct the class
	const codec_entry *entry = find_in_list(type);
	return (entry == NULL) ? NULL : (*entry->m_construct_compressor)(chd, chd.hunk_bytes(), entry->m_lossy);
}


//-------------------------------------------------
//  new_compressor - create a new decompressor
//  instance of the given type
//-------------------------------------------------

chd_decompressor *chd_codec_list::new_decompressor(chd_codec_type type, chd_file &chd)
{
	// find in the list and construct the class
	const codec_entry *entry = find_in_list(type);
	return (entry == NULL) ? NULL : (*entry->m_construct_decompressor)(chd, chd.hunk_bytes(), entry->m_lossy);
}


//-------------------------------------------------
//  codec_name - return the name of the given
//  codec
//-------------------------------------------------

const char *chd_codec_list::codec_name(chd_codec_type type)
{
	// find in the list and construct the class
	const codec_entry *entry = find_in_list(type);
	return (entry == NULL) ? NULL : entry->m_name;
}


//-------------------------------------------------
//  find_in_list - create a new compressor
//  instance of the given type
//-------------------------------------------------

const chd_codec_list::codec_entry *chd_codec_list::find_in_list(chd_codec_type type)
{
	// find in the list and construct the class
	for (int listnum = 0; listnum < ARRAY_LENGTH(s_codec_list); listnum++)
		if (s_codec_list[listnum].m_type == type)
			return &s_codec_list[listnum];
	return NULL;
}



//**************************************************************************
//  CODEC INSTANCE
//**************************************************************************

//-------------------------------------------------
//  chd_compressor_group - constructor
//-------------------------------------------------

chd_compressor_group::chd_compressor_group(chd_file &chd, UINT32 compressor_list[4])
	: m_hunkbytes(chd.hunk_bytes()),
		m_compress_test(m_hunkbytes)
#if CHDCODEC_VERIFY_COMPRESSION
		,m_decompressed(m_hunkbytes)
#endif
{
	// verify the compression types and initialize the codecs
	for (int codecnum = 0; codecnum < ARRAY_LENGTH(m_compressor); codecnum++)
	{
		m_compressor[codecnum] = NULL;
		if (compressor_list[codecnum] != CHD_CODEC_NONE)
		{
			m_compressor[codecnum] = chd_codec_list::new_compressor(compressor_list[codecnum], chd);
			if (m_compressor[codecnum] == NULL)
				throw CHDERR_UNKNOWN_COMPRESSION;
#if CHDCODEC_VERIFY_COMPRESSION
			m_decompressor[codecnum] = chd_codec_list::new_decompressor(compressor_list[codecnum], chd);
			if (m_decompressor[codecnum] == NULL)
				throw CHDERR_UNKNOWN_COMPRESSION;
#endif
		}
	}
}


//-------------------------------------------------
//  ~chd_compressor_group - destructor
//-------------------------------------------------

chd_compressor_group::~chd_compressor_group()
{
	// delete the codecs and the test buffer
	for (int codecnum = 0; codecnum < ARRAY_LENGTH(m_compressor); codecnum++)
		delete m_compressor[codecnum];
}


//-------------------------------------------------
//  find_best_compressor - iterate over all codecs
//  to determine which one produces the best
//  compression for this hunk
//-------------------------------------------------

INT8 chd_compressor_group::find_best_compressor(const UINT8 *src, UINT8 *compressed, UINT32 &complen)
{
	// determine best compression technique
	complen = m_hunkbytes;
	INT8 compression = -1;
	for (int codecnum = 0; codecnum < ARRAY_LENGTH(m_compressor); codecnum++)
		if (m_compressor[codecnum] != NULL)
		{
			// attempt to compress, swallowing errors
			try
			{
				// if this is the best one, copy the data into the permanent buffer
				UINT32 compbytes = m_compressor[codecnum]->compress(src, m_hunkbytes, &m_compress_test[0]);
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
			catch (...) { }
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
//  ~chd_zlib_allocator - constructor
//-------------------------------------------------

chd_zlib_allocator::~chd_zlib_allocator()
{
	// free our memory
	for (int memindex = 0; memindex < ARRAY_LENGTH(m_allocptr); memindex++)
		delete[] m_allocptr[memindex];
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
	chd_zlib_allocator *codec = reinterpret_cast<chd_zlib_allocator *>(opaque);

	// compute the size, rounding to the nearest 1k
	size = (size * items + 0x3ff) & ~0x3ff;

	// reuse a hunk if we can
	for (int scan = 0; scan < MAX_ZLIB_ALLOCS; scan++)
	{
		UINT32 *ptr = codec->m_allocptr[scan];
		if (ptr != NULL && size == *ptr)
		{
			// set the low bit of the size so we don't match next time
			*ptr |= 1;
			return ptr + 1;
		}
	}

	// alloc a new one and put it into the list
	UINT32 *ptr = reinterpret_cast<UINT32 *>(new UINT8[size + sizeof(UINT32)]);
	for (int scan = 0; scan < MAX_ZLIB_ALLOCS; scan++)
		if (codec->m_allocptr[scan] == NULL)
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
	chd_zlib_allocator *codec = reinterpret_cast<chd_zlib_allocator *>(opaque);

	// find the hunk
	UINT32 *ptr = reinterpret_cast<UINT32 *>(address) - 1;
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

chd_zlib_compressor::chd_zlib_compressor(chd_file &chd, UINT32 hunkbytes, bool lossy)
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
		throw CHDERR_CODEC_ERROR;
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

UINT32 chd_zlib_compressor::compress(const UINT8 *src, UINT32 srclen, UINT8 *dest)
{
	// reset the decompressor
	m_deflater.next_in = const_cast<Bytef *>(src);
	m_deflater.avail_in = srclen;
	m_deflater.total_in = 0;
	m_deflater.next_out = dest;
	m_deflater.avail_out = srclen;
	m_deflater.total_out = 0;
	int zerr = deflateReset(&m_deflater);
	if (zerr != Z_OK)
		throw CHDERR_COMPRESSION_ERROR;

	// do it
	zerr = deflate(&m_deflater, Z_FINISH);

	// if we ended up with more data than we started with, return an error
	if (zerr != Z_STREAM_END || m_deflater.total_out >= srclen)
		throw CHDERR_COMPRESSION_ERROR;

	// otherwise, return the length
	return m_deflater.total_out;
}



//**************************************************************************
//  ZLIB DECOMPRESSOR
//**************************************************************************

//-------------------------------------------------
//  chd_zlib_decompressor - constructor
//-------------------------------------------------

chd_zlib_decompressor::chd_zlib_decompressor(chd_file &chd, UINT32 hunkbytes, bool lossy)
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
		throw CHDERR_CODEC_ERROR;
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

void chd_zlib_decompressor::decompress(const UINT8 *src, UINT32 complen, UINT8 *dest, UINT32 destlen)
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
		throw CHDERR_DECOMPRESSION_ERROR;

	// do it
	zerr = inflate(&m_inflater, Z_FINISH);
	if (zerr != Z_STREAM_END)
		throw CHDERR_DECOMPRESSION_ERROR;
	if (m_inflater.total_out != destlen)
		throw CHDERR_DECOMPRESSION_ERROR;
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
	for (int memindex = 0; memindex < ARRAY_LENGTH(m_allocptr); memindex++)
		delete[] m_allocptr[memindex];
}


//-------------------------------------------------
//  lzma_fast_alloc - fast malloc for lzma, which
//  allocates and frees memory frequently
//-------------------------------------------------

void *chd_lzma_allocator::fast_alloc(void *p, size_t size)
{
	chd_lzma_allocator *codec = reinterpret_cast<chd_lzma_allocator *>(p);

	// compute the size, rounding to the nearest 1k
	size = (size + 0x3ff) & ~0x3ff;

	// reuse a hunk if we can
	for (int scan = 0; scan < MAX_LZMA_ALLOCS; scan++)
	{
		UINT32 *ptr = codec->m_allocptr[scan];
		if (ptr != NULL && size == *ptr)
		{
			// set the low bit of the size so we don't match next time
			*ptr |= 1;
			return ptr + 1;
		}
	}

	// alloc a new one and put it into the list
	UINT32 *ptr = reinterpret_cast<UINT32 *>(new UINT8[size + sizeof(UINT32)]);
	for (int scan = 0; scan < MAX_LZMA_ALLOCS; scan++)
		if (codec->m_allocptr[scan] == NULL)
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

void chd_lzma_allocator::fast_free(void *p, void *address)
{
	if (address == NULL)
		return;

	chd_lzma_allocator *codec = reinterpret_cast<chd_lzma_allocator *>(p);

	// find the hunk
	UINT32 *ptr = reinterpret_cast<UINT32 *>(address) - 1;
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

chd_lzma_compressor::chd_lzma_compressor(chd_file &chd, UINT32 hunkbytes, bool lossy)
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

UINT32 chd_lzma_compressor::compress(const UINT8 *src, UINT32 srclen, UINT8 *dest)
{
	// allocate the encoder
	CLzmaEncHandle encoder = LzmaEnc_Create(&m_allocator);
	if (encoder == NULL)
		throw CHDERR_COMPRESSION_ERROR;

	try
	{
		// configure the encoder
		SRes res = LzmaEnc_SetProps(encoder, &m_props);
		if (res != SZ_OK)
			throw CHDERR_COMPRESSION_ERROR;

		// run it
		SizeT complen = srclen;
		res = LzmaEnc_MemEncode(encoder, dest, &complen, src, srclen, 0, NULL, &m_allocator, &m_allocator);
		if (res != SZ_OK)
			throw CHDERR_COMPRESSION_ERROR;

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

void chd_lzma_compressor::configure_properties(CLzmaEncProps &props, UINT32 hunkbytes)
{
	LzmaEncProps_Init(&props);
	props.level = 9;
	props.reduceSize = hunkbytes;
	LzmaEncProps_Normalize(&props);
}



//**************************************************************************
//  LZMA DECOMPRESSOR
//**************************************************************************

//-------------------------------------------------
//  chd_lzma_decompressor - constructor
//-------------------------------------------------

chd_lzma_decompressor::chd_lzma_decompressor(chd_file &chd, UINT32 hunkbytes, bool lossy)
	: chd_decompressor(chd, hunkbytes, lossy)
{
	// construct the decoder
	LzmaDec_Construct(&m_decoder);

	// configure the properties like the compressor did
	CLzmaEncProps encoder_props;
	chd_lzma_compressor::configure_properties(encoder_props, hunkbytes);

	// convert to decoder properties
	CLzmaProps decoder_props;
	decoder_props.lc = encoder_props.lc;
	decoder_props.lp = encoder_props.lp;
	decoder_props.pb = encoder_props.pb;
	decoder_props.dicSize = encoder_props.dictSize;

	// do memory allocations
	SRes res = LzmaDec_Allocate_MAME(&m_decoder, &decoder_props, &m_allocator);
	if (res != SZ_OK)
		throw CHDERR_DECOMPRESSION_ERROR;
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

void chd_lzma_decompressor::decompress(const UINT8 *src, UINT32 complen, UINT8 *dest, UINT32 destlen)
{
	// initialize
	LzmaDec_Init(&m_decoder);

	// decode
	SizeT consumedlen = complen;
	SizeT decodedlen = destlen;
	ELzmaStatus status;
	SRes res = LzmaDec_DecodeToBuf(&m_decoder, dest, &decodedlen, src, &consumedlen, LZMA_FINISH_END, &status);
	if ((res != SZ_OK && res != LZMA_STATUS_MAYBE_FINISHED_WITHOUT_MARK) || consumedlen != complen || decodedlen != destlen)
		throw CHDERR_DECOMPRESSION_ERROR;
}



//**************************************************************************
//  HUFFMAN COMPRESSOR
//**************************************************************************

//-------------------------------------------------
//  chd_huffman_compressor - constructor
//-------------------------------------------------

chd_huffman_compressor::chd_huffman_compressor(chd_file &chd, UINT32 hunkbytes, bool lossy)
	: chd_compressor(chd, hunkbytes, lossy)
{
}


//-------------------------------------------------
//  compress - compress data using the Huffman
//  codec
//-------------------------------------------------

UINT32 chd_huffman_compressor::compress(const UINT8 *src, UINT32 srclen, UINT8 *dest)
{
	UINT32 complen;
	if (m_encoder.encode(src, srclen, dest, srclen, complen) != HUFFERR_NONE)
		throw CHDERR_COMPRESSION_ERROR;
	return complen;
}



//**************************************************************************
//  HUFFMAN DECOMPRESSOR
//**************************************************************************

//-------------------------------------------------
//  chd_huffman_decompressor - constructor
//-------------------------------------------------

chd_huffman_decompressor::chd_huffman_decompressor(chd_file &chd, UINT32 hunkbytes, bool lossy)
	: chd_decompressor(chd, hunkbytes, lossy)
{
}


//-------------------------------------------------
//  decompress - decompress data using the Huffman
//  codec
//-------------------------------------------------

void chd_huffman_decompressor::decompress(const UINT8 *src, UINT32 complen, UINT8 *dest, UINT32 destlen)
{
	if (m_decoder.decode(src, complen, dest, destlen) != HUFFERR_NONE)
		throw CHDERR_COMPRESSION_ERROR;
}



//**************************************************************************
//  FLAC COMPRESSOR
//**************************************************************************

//-------------------------------------------------
//  chd_flac_compressor - constructor
//-------------------------------------------------

chd_flac_compressor::chd_flac_compressor(chd_file &chd, UINT32 hunkbytes, bool lossy)
	: chd_compressor(chd, hunkbytes, lossy)
{
	// determine whether we want native or swapped samples
	UINT16 native_endian = 0;
	*reinterpret_cast<UINT8 *>(&native_endian) = 1;
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

UINT32 chd_flac_compressor::compress(const UINT8 *src, UINT32 srclen, UINT8 *dest)
{
	// reset and encode big-endian
	m_encoder.reset(dest + 1, hunkbytes() - 1);
	if (!m_encoder.encode_interleaved(reinterpret_cast<const INT16 *>(src), srclen / 4, !m_big_endian))
		throw CHDERR_COMPRESSION_ERROR;
	UINT32 complen_be = m_encoder.finish();

	// reset and encode little-endian
	m_encoder.reset(dest + 1, hunkbytes() - 1);
	if (!m_encoder.encode_interleaved(reinterpret_cast<const INT16 *>(src), srclen / 4, m_big_endian))
		throw CHDERR_COMPRESSION_ERROR;
	UINT32 complen_le = m_encoder.finish();

	// pick the best one and add a byte
	UINT32 complen = MIN(complen_le, complen_be);
	if (complen + 1 >= hunkbytes())
		throw CHDERR_COMPRESSION_ERROR;

	// if big-endian was better, re-do it
	dest[0] = 'L';
	if (complen != complen_le)
	{
		dest[0] = 'B';
		m_encoder.reset(dest + 1, hunkbytes() - 1);
		if (!m_encoder.encode_interleaved(reinterpret_cast<const INT16 *>(src), srclen / 4, !m_big_endian))
			throw CHDERR_COMPRESSION_ERROR;
		m_encoder.finish();
	}
	return complen + 1;
}


//-------------------------------------------------
//  blocksize - return the optimal block size
//-------------------------------------------------

UINT32 chd_flac_compressor::blocksize(UINT32 bytes)
{
	// determine FLAC block size, which must be 16-65535
	// clamp to 2k since that's supposed to be the sweet spot
	UINT32 hunkbytes = bytes / 4;
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

chd_flac_decompressor::chd_flac_decompressor(chd_file &chd, UINT32 hunkbytes, bool lossy)
	: chd_decompressor(chd, hunkbytes, lossy)
{
	// determine whether we want native or swapped samples
	UINT16 native_endian = 0;
	*reinterpret_cast<UINT8 *>(&native_endian) = 1;
	m_big_endian = (native_endian == 0x100);
}


//-------------------------------------------------
//  decompress - decompress data using the FLAC
//  codec
//-------------------------------------------------

void chd_flac_decompressor::decompress(const UINT8 *src, UINT32 complen, UINT8 *dest, UINT32 destlen)
{
	// determine the endianness
	bool swap_endian;
	if (src[0] == 'L')
		swap_endian = m_big_endian;
	else if (src[0] == 'B')
		swap_endian = !m_big_endian;
	else
		throw CHDERR_DECOMPRESSION_ERROR;

	// reset and decode
	if (!m_decoder.reset(44100, 2, chd_flac_compressor::blocksize(destlen), src + 1, complen - 1))
		throw CHDERR_DECOMPRESSION_ERROR;
	if (!m_decoder.decode_interleaved(reinterpret_cast<INT16 *>(dest), destlen / 4, swap_endian))
		throw CHDERR_DECOMPRESSION_ERROR;

	// finish up
	m_decoder.finish();
}



//**************************************************************************
//  CD FLAC COMPRESSOR
//**************************************************************************

//-------------------------------------------------
//  chd_cd_flac_compressor - constructor
//-------------------------------------------------

chd_cd_flac_compressor::chd_cd_flac_compressor(chd_file &chd, UINT32 hunkbytes, bool lossy)
	: chd_compressor(chd, hunkbytes, lossy),
		m_buffer(hunkbytes)
{
	// make sure the CHD's hunk size is an even multiple of the frame size
	if (hunkbytes % CD_FRAME_SIZE != 0)
		throw CHDERR_CODEC_ERROR;

	// determine whether we want native or swapped samples
	UINT16 native_endian = 0;
	*reinterpret_cast<UINT8 *>(&native_endian) = 1;
	m_swap_endian = (native_endian == 1);

	// configure the encoder
	m_encoder.set_sample_rate(44100);
	m_encoder.set_num_channels(2);
	m_encoder.set_block_size(blocksize((hunkbytes / CD_FRAME_SIZE) * CD_MAX_SECTOR_DATA));
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
		throw CHDERR_CODEC_ERROR;
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

UINT32 chd_cd_flac_compressor::compress(const UINT8 *src, UINT32 srclen, UINT8 *dest)
{
	// copy audio data followed by subcode data
	UINT32 frames = hunkbytes() / CD_FRAME_SIZE;
	for (UINT32 framenum = 0; framenum < frames; framenum++)
	{
		memcpy(&m_buffer[framenum * CD_MAX_SECTOR_DATA], &src[framenum * CD_FRAME_SIZE], CD_MAX_SECTOR_DATA);
		memcpy(&m_buffer[frames * CD_MAX_SECTOR_DATA + framenum * CD_MAX_SUBCODE_DATA], &src[framenum * CD_FRAME_SIZE + CD_MAX_SECTOR_DATA], CD_MAX_SUBCODE_DATA);
	}

	// reset and encode the audio portion
	m_encoder.reset(dest, hunkbytes());
	UINT8 *buffer = &m_buffer[0];
	if (!m_encoder.encode_interleaved(reinterpret_cast<INT16 *>(buffer), frames * CD_MAX_SECTOR_DATA/4, m_swap_endian))
		throw CHDERR_COMPRESSION_ERROR;

	// finish up
	UINT32 complen = m_encoder.finish();

	// deflate the subcode data
	m_deflater.next_in = const_cast<Bytef *>(&m_buffer[frames * CD_MAX_SECTOR_DATA]);
	m_deflater.avail_in = frames * CD_MAX_SUBCODE_DATA;
	m_deflater.total_in = 0;
	m_deflater.next_out = &dest[complen];
	m_deflater.avail_out = hunkbytes() - complen;
	m_deflater.total_out = 0;
	int zerr = deflateReset(&m_deflater);
	if (zerr != Z_OK)
		throw CHDERR_COMPRESSION_ERROR;

	// do it
	zerr = deflate(&m_deflater, Z_FINISH);

	// if we ended up with more data than we started with, return an error
	complen += m_deflater.total_out;
	if (zerr != Z_STREAM_END || complen >= srclen)
		throw CHDERR_COMPRESSION_ERROR;
	return complen;
}

/**
 * @fn  UINT32 chd_cd_flac_compressor::blocksize(UINT32 bytes)
 *
 * @brief   -------------------------------------------------
 *            blocksize - return the optimal block size
 *          -------------------------------------------------.
 *
 * @param   bytes   The bytes.
 *
 * @return  An UINT32.
 */

UINT32 chd_cd_flac_compressor::blocksize(UINT32 bytes)
{
	// for CDs it seems that CD_MAX_SECTOR_DATA is the right target
	UINT32 blocksize = bytes / 4;
	while (blocksize > CD_MAX_SECTOR_DATA)
		blocksize /= 2;
	return blocksize;
}



//**************************************************************************
//  CD FLAC DECOMPRESSOR
//**************************************************************************

/**
 * @fn  chd_cd_flac_decompressor::chd_cd_flac_decompressor(chd_file &chd, UINT32 hunkbytes, bool lossy)
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

chd_cd_flac_decompressor::chd_cd_flac_decompressor(chd_file &chd, UINT32 hunkbytes, bool lossy)
	: chd_decompressor(chd, hunkbytes, lossy),
		m_buffer(hunkbytes)
{
	// make sure the CHD's hunk size is an even multiple of the frame size
	if (hunkbytes % CD_FRAME_SIZE != 0)
		throw CHDERR_CODEC_ERROR;

	// determine whether we want native or swapped samples
	UINT16 native_endian = 0;
	*reinterpret_cast<UINT8 *>(&native_endian) = 1;
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
		throw CHDERR_CODEC_ERROR;
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
 * @fn  void chd_cd_flac_decompressor::decompress(const UINT8 *src, UINT32 complen, UINT8 *dest, UINT32 destlen)
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

void chd_cd_flac_decompressor::decompress(const UINT8 *src, UINT32 complen, UINT8 *dest, UINT32 destlen)
{
	// reset and decode
	UINT32 frames = destlen / CD_FRAME_SIZE;
	if (!m_decoder.reset(44100, 2, chd_cd_flac_compressor::blocksize(frames * CD_MAX_SECTOR_DATA), src, complen))
		throw CHDERR_DECOMPRESSION_ERROR;
	UINT8 *buffer = &m_buffer[0];
	if (!m_decoder.decode_interleaved(reinterpret_cast<INT16 *>(buffer), frames * CD_MAX_SECTOR_DATA/4, m_swap_endian))
		throw CHDERR_DECOMPRESSION_ERROR;

	// inflate the subcode data
	UINT32 offset = m_decoder.finish();
	m_inflater.next_in = const_cast<Bytef *>(src + offset);
	m_inflater.avail_in = complen - offset;
	m_inflater.total_in = 0;
	m_inflater.next_out = &m_buffer[frames * CD_MAX_SECTOR_DATA];
	m_inflater.avail_out = frames * CD_MAX_SUBCODE_DATA;
	m_inflater.total_out = 0;
	int zerr = inflateReset(&m_inflater);
	if (zerr != Z_OK)
		throw CHDERR_DECOMPRESSION_ERROR;

	// do it
	zerr = inflate(&m_inflater, Z_FINISH);
	if (zerr != Z_STREAM_END)
		throw CHDERR_DECOMPRESSION_ERROR;
	if (m_inflater.total_out != frames * CD_MAX_SUBCODE_DATA)
		throw CHDERR_DECOMPRESSION_ERROR;

	// reassemble the data
	for (UINT32 framenum = 0; framenum < frames; framenum++)
	{
		memcpy(&dest[framenum * CD_FRAME_SIZE], &m_buffer[framenum * CD_MAX_SECTOR_DATA], CD_MAX_SECTOR_DATA);
		memcpy(&dest[framenum * CD_FRAME_SIZE + CD_MAX_SECTOR_DATA], &m_buffer[frames * CD_MAX_SECTOR_DATA + framenum * CD_MAX_SUBCODE_DATA], CD_MAX_SUBCODE_DATA);
	}
}



//**************************************************************************
//  AVHUFF COMPRESSOR
//**************************************************************************

/**
 * @fn  chd_avhuff_compressor::chd_avhuff_compressor(chd_file &chd, UINT32 hunkbytes, bool lossy)
 *
 * @brief   -------------------------------------------------
 *            chd_avhuff_compressor - constructor
 *          -------------------------------------------------.
 *
 * @param [in,out]  chd The chd.
 * @param   hunkbytes   The hunkbytes.
 * @param   lossy       true to lossy.
 */

chd_avhuff_compressor::chd_avhuff_compressor(chd_file &chd, UINT32 hunkbytes, bool lossy)
	: chd_compressor(chd, hunkbytes, lossy),
		m_postinit(false)
{
	try
	{
		// attempt to do a post-init now
		postinit();
	}
	catch (chd_error &)
	{
		// if we're creating a new CHD, it won't work but that's ok
	}
}

/**
 * @fn  UINT32 chd_avhuff_compressor::compress(const UINT8 *src, UINT32 srclen, UINT8 *dest)
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
 * @return  An UINT32.
 */

UINT32 chd_avhuff_compressor::compress(const UINT8 *src, UINT32 srclen, UINT8 *dest)
{
	// if we haven't yet set up the avhuff code, do it now
	if (!m_postinit)
		postinit();

	// make sure short frames are padded with 0
	if (src != NULL)
	{
		int size = avhuff_encoder::raw_data_size(src);
		while (size < srclen)
			if (src[size++] != 0)
				throw CHDERR_INVALID_DATA;
	}

	// encode the audio and video
	UINT32 complen;
	avhuff_error averr = m_encoder.encode_data(src, dest, complen);
	if (averr != AVHERR_NONE || complen > srclen)
		throw CHDERR_COMPRESSION_ERROR;
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
	chd_error err = chd().read_metadata(AV_METADATA_TAG, 0, metadata);
	if (err != CHDERR_NONE)
		throw err;

	// extract the info
	int fps, fpsfrac, width, height, interlaced, channels, rate;
	if (sscanf(metadata.c_str(), AV_METADATA_FORMAT, &fps, &fpsfrac, &width, &height, &interlaced, &channels, &rate) != 7)
		throw CHDERR_INVALID_METADATA;

	// compute the bytes per frame
	UINT32 fps_times_1million = fps * 1000000 + fpsfrac;
	UINT32 max_samples_per_frame = (UINT64(rate) * 1000000 + fps_times_1million - 1) / fps_times_1million;
	UINT32 bytes_per_frame = 12 + channels * max_samples_per_frame * 2 + width * height * 2;
	if (bytes_per_frame > hunkbytes())
		throw CHDERR_INVALID_METADATA;

	// done with post-init
	m_postinit = true;
}



//**************************************************************************
//  AVHUFF DECOMPRESSOR
//**************************************************************************

/**
 * @fn  chd_avhuff_decompressor::chd_avhuff_decompressor(chd_file &chd, UINT32 hunkbytes, bool lossy)
 *
 * @brief   -------------------------------------------------
 *            chd_avhuff_decompressor - constructor
 *          -------------------------------------------------.
 *
 * @param [in,out]  chd The chd.
 * @param   hunkbytes   The hunkbytes.
 * @param   lossy       true to lossy.
 */

chd_avhuff_decompressor::chd_avhuff_decompressor(chd_file &chd, UINT32 hunkbytes, bool lossy)
	: chd_decompressor(chd, hunkbytes, lossy)
{
}

/**
 * @fn  void chd_avhuff_decompressor::decompress(const UINT8 *src, UINT32 complen, UINT8 *dest, UINT32 destlen)
 *
 * @brief   -------------------------------------------------
 *            decompress - decompress data using the A/V codec
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

void chd_avhuff_decompressor::decompress(const UINT8 *src, UINT32 complen, UINT8 *dest, UINT32 destlen)
{
	// decode the audio and video
	avhuff_error averr = m_decoder.decode_data(src, complen, dest);
	if (averr != AVHERR_NONE)
		throw CHDERR_DECOMPRESSION_ERROR;

	// pad short frames with 0
	if (dest != NULL)
	{
		int size = avhuff_encoder::raw_data_size(dest);
		if (size < destlen)
			memset(dest + size, 0, destlen - size);
	}
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
		m_decoder.configure(*reinterpret_cast<avhuff_decompress_config *>(config));

	// anything else is invalid
	else
		throw CHDERR_INVALID_PARAMETER;
}
