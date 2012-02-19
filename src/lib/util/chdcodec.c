/***************************************************************************

    chdcodec.c

    Codecs used by the CHD format

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

***************************************************************************/

#include "chd.h"
#include "hashing.h"
#include "avhuff.h"
#include "flac.h"
#include "cdrom.h"
#include <zlib.h>
#include "lib7z/LzmaEnc.h"
#include "lib7z/LzmaDec.h"
#include <new>

// function that should exist but doesn't in the official release
extern "C" SRes LzmaDec_Allocate_MAME(CLzmaDec *p, const CLzmaProps *propNew, ISzAlloc *alloc);


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
	UINT32 *				m_allocptr[MAX_ZLIB_ALLOCS];
};


// ======================> chd_zlib_compressor

// ZLIB compressor
class chd_zlib_compressor : public chd_compressor
{
public:
	// construction/destruction
	chd_zlib_compressor(chd_file &chd, bool lossy);
	~chd_zlib_compressor();

	// core functionality
	virtual UINT32 compress(const UINT8 *src, UINT32 srclen, UINT8 *dest);

private:
	// internal state
	z_stream				m_deflater;
	chd_zlib_allocator		m_allocator;
};


// ======================> chd_zlib_decompressor

// ZLIB decompressor
class chd_zlib_decompressor : public chd_decompressor
{
public:
	// construction/destruction
	chd_zlib_decompressor(chd_file &chd, bool lossy);
	~chd_zlib_decompressor();

	// core functionality
	virtual void decompress(const UINT8 *src, UINT32 complen, UINT8 *dest, UINT32 destlen);

private:
	// internal state
	z_stream				m_inflater;
	chd_zlib_allocator		m_allocator;
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
	UINT32 *				m_allocptr[MAX_LZMA_ALLOCS];
};


// ======================> chd_lzma_compressor

// LZMA compressor
class chd_lzma_compressor : public chd_compressor
{
public:
	// construction/destruction
	chd_lzma_compressor(chd_file &chd, bool lossy);
	~chd_lzma_compressor();

	// core functionality
	virtual UINT32 compress(const UINT8 *src, UINT32 srclen, UINT8 *dest);

	// helpers
	static void configure_properties(CLzmaEncProps &props, chd_file &chd);

private:
	// internal state
	CLzmaEncProps			m_props;
	chd_lzma_allocator		m_allocator;
};


// ======================> chd_lzma_decompressor

// LZMA decompressor
class chd_lzma_decompressor : public chd_decompressor
{
public:
	// construction/destruction
	chd_lzma_decompressor(chd_file &chd, bool lossy);
	~chd_lzma_decompressor();

	// core functionality
	virtual void decompress(const UINT8 *src, UINT32 complen, UINT8 *dest, UINT32 destlen);

private:
	// internal state
	CLzmaProps				m_props;
	CLzmaDec				m_decoder;
	chd_lzma_allocator		m_allocator;
};


// ======================> chd_huffman_compressor

// Huffman compressor
class chd_huffman_compressor : public chd_compressor
{
public:
	// construction/destruction
	chd_huffman_compressor(chd_file &chd, bool lossy);

	// core functionality
	virtual UINT32 compress(const UINT8 *src, UINT32 srclen, UINT8 *dest);

private:
	// internal state
	huffman_8bit_encoder	m_encoder;
};


// ======================> chd_huffman_decompressor

// Huffman decompressor
class chd_huffman_decompressor : public chd_decompressor
{
public:
	// construction/destruction
	chd_huffman_decompressor(chd_file &chd, bool lossy);

	// core functionality
	virtual void decompress(const UINT8 *src, UINT32 complen, UINT8 *dest, UINT32 destlen);

private:
	// internal state
	huffman_8bit_decoder	m_decoder;
};


// ======================> chd_flac_compressor

// FLAC compressor
class chd_flac_compressor : public chd_compressor
{
public:
	// construction/destruction
	chd_flac_compressor(chd_file &chd, bool lossy, bool bigendian);

	// core functionality
	virtual UINT32 compress(const UINT8 *src, UINT32 srclen, UINT8 *dest);

private:
	// internal state
	bool			m_swap_endian;
	flac_encoder	m_encoder;
};

// big-endian variant
class chd_flac_compressor_be : public chd_flac_compressor
{
public:
	// construction/destruction
	chd_flac_compressor_be(chd_file &chd, bool lossy)
		: chd_flac_compressor(chd, lossy, true) { }
};

// little-endian variant
class chd_flac_compressor_le : public chd_flac_compressor
{
public:
	// construction/destruction
	chd_flac_compressor_le(chd_file &chd, bool lossy)
		: chd_flac_compressor(chd, lossy, false) { }
};


// ======================> chd_flac_decompressor

// FLAC decompressor
class chd_flac_decompressor : public chd_decompressor
{
public:
	// construction/destruction
	chd_flac_decompressor(chd_file &chd, bool lossy, bool bigendian);

	// core functionality
	virtual void decompress(const UINT8 *src, UINT32 complen, UINT8 *dest, UINT32 destlen);

private:
	// internal state
	bool			m_swap_endian;
	flac_decoder	m_decoder;
};

// big-endian variant
class chd_flac_decompressor_be : public chd_flac_decompressor
{
public:
	// construction/destruction
	chd_flac_decompressor_be(chd_file &chd, bool lossy)
		: chd_flac_decompressor(chd, lossy, true) { }
};

// little-endian variant
class chd_flac_decompressor_le : public chd_flac_decompressor
{
public:
	// construction/destruction
	chd_flac_decompressor_le(chd_file &chd, bool lossy)
		: chd_flac_decompressor(chd, lossy, false) { }
};


// ======================> chd_cd_flac_compressor

// CD/FLAC compressor
class chd_cd_flac_compressor : public chd_compressor
{
public:
	// construction/destruction
	chd_cd_flac_compressor(chd_file &chd, bool lossy);
	~chd_cd_flac_compressor();

	// core functionality
	virtual UINT32 compress(const UINT8 *src, UINT32 srclen, UINT8 *dest);

private:
	// internal state
	bool				m_swap_endian;
	flac_encoder		m_encoder;
	z_stream			m_deflater;
	chd_zlib_allocator	m_allocator;
	dynamic_buffer		m_buffer;
};


// ======================> chd_cd_flac_decompressor

// FLAC decompressor
class chd_cd_flac_decompressor : public chd_decompressor
{
public:
	// construction/destruction
	chd_cd_flac_decompressor(chd_file &chd, bool lossy);
	~chd_cd_flac_decompressor();

	// core functionality
	virtual void decompress(const UINT8 *src, UINT32 complen, UINT8 *dest, UINT32 destlen);

private:
	// internal state
	bool				m_swap_endian;
	flac_decoder		m_decoder;
	z_stream			m_inflater;
	chd_zlib_allocator	m_allocator;
	dynamic_buffer		m_buffer;
};


// ======================> chd_avhuff_compressor

// A/V compressor
class chd_avhuff_compressor : public chd_compressor
{
public:
	// construction/destruction
	chd_avhuff_compressor(chd_file &chd, bool lossy);

	// core functionality
	virtual UINT32 compress(const UINT8 *src, UINT32 srclen, UINT8 *dest);

private:
	// internal helpers
	void postinit();

	// internal state
	avhuff_encoder				m_encoder;
	bool						m_postinit;
};


// ======================> chd_avhuff_decompressor

// A/V decompressor
class chd_avhuff_decompressor : public chd_decompressor
{
public:
	// construction/destruction
	chd_avhuff_decompressor(chd_file &chd, bool lossy);

	// core functionality
	virtual void decompress(const UINT8 *src, UINT32 complen, UINT8 *dest, UINT32 destlen);
	virtual void configure(int param, void *config);

private:
	// internal state
	avhuff_decoder				m_decoder;
};



//**************************************************************************
//  CODEC LIST
//**************************************************************************

// static list of available known codecs
const chd_codec_list::codec_entry chd_codec_list::s_codec_list[] =
{
	{ CHD_CODEC_ZLIB,		false,	"Deflate",				&chd_codec_list::construct_compressor<chd_zlib_compressor>,		&chd_codec_list::construct_decompressor<chd_zlib_decompressor> },
	{ CHD_CODEC_LZMA,		false,	"LZMA",					&chd_codec_list::construct_compressor<chd_lzma_compressor>,		&chd_codec_list::construct_decompressor<chd_lzma_decompressor> },
	{ CHD_CODEC_HUFFMAN,	false,	"Huffman",				&chd_codec_list::construct_compressor<chd_huffman_compressor>,	&chd_codec_list::construct_decompressor<chd_huffman_decompressor> },
	{ CHD_CODEC_FLAC_BE,	false,	"FLAC, big-endian",		&chd_codec_list::construct_compressor<chd_flac_compressor_be>,	&chd_codec_list::construct_decompressor<chd_flac_decompressor_be> },
	{ CHD_CODEC_FLAC_LE,	false,	"FLAC, little-endian",	&chd_codec_list::construct_compressor<chd_flac_compressor_le>,	&chd_codec_list::construct_decompressor<chd_flac_decompressor_le> },
	{ CHD_CODEC_CD_FLAC,	false,	"CD FLAC",				&chd_codec_list::construct_compressor<chd_cd_flac_compressor>,	&chd_codec_list::construct_decompressor<chd_cd_flac_decompressor> },
	{ CHD_CODEC_AVHUFF,		false,	"A/V Huffman",			&chd_codec_list::construct_compressor<chd_avhuff_compressor>,	&chd_codec_list::construct_decompressor<chd_avhuff_decompressor> },
};



//**************************************************************************
//  CHD CODEC
//**************************************************************************

//-------------------------------------------------
//  chd_codec - constructor
//-------------------------------------------------

chd_codec::chd_codec(chd_file &file, bool lossy)
	: m_chd(file),
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

chd_compressor::chd_compressor(chd_file &file, bool lossy)
	: chd_codec(file, lossy)
{
}



//**************************************************************************
//  CHD DECOMPRESSOR
//**************************************************************************

//-------------------------------------------------
//  chd_decompressor - constructor
//-------------------------------------------------

chd_decompressor::chd_decompressor(chd_file &file, bool lossy)
	: chd_codec(file, lossy)
{
}



//**************************************************************************
//  CHD CODEC LIST
//**************************************************************************

//-------------------------------------------------
//  new_compressor - create a new compressor
//  instance of the given type
//-------------------------------------------------

chd_compressor *chd_codec_list::new_compressor(chd_codec_type type, chd_file &file)
{
	// find in the list and construct the class
	const codec_entry *entry = find_in_list(type);
	return (entry == NULL) ? NULL : (*entry->m_construct_compressor)(file, entry->m_lossy);
}


//-------------------------------------------------
//  new_compressor - create a new decompressor
//  instance of the given type
//-------------------------------------------------

chd_decompressor *chd_codec_list::new_decompressor(chd_codec_type type, chd_file &file)
{
	// find in the list and construct the class
	const codec_entry *entry = find_in_list(type);
	return (entry == NULL) ? NULL : (*entry->m_construct_decompressor)(file, entry->m_lossy);
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

chd_compressor_group::chd_compressor_group(chd_file &file, UINT32 compressor_list[4])
	: m_hunkbytes(file.hunk_bytes()),
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
			m_compressor[codecnum] = chd_codec_list::new_compressor(compressor_list[codecnum], file);
			if (m_compressor[codecnum] == NULL)
				throw CHDERR_UNKNOWN_COMPRESSION;
#if CHDCODEC_VERIFY_COMPRESSION
			m_decompressor[codecnum] = chd_codec_list::new_decompressor(compressor_list[codecnum], file);
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
				UINT32 compbytes = m_compressor[codecnum]->compress(src, m_hunkbytes, m_compress_test);
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
					memcpy(compressed, m_compress_test, compbytes);
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

chd_zlib_compressor::chd_zlib_compressor(chd_file &chd, bool lossy)
	: chd_compressor(chd, lossy)
{
	// initialize the deflater
	m_deflater.next_in = (Bytef *)this;	// bogus, but that's ok
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

chd_zlib_decompressor::chd_zlib_decompressor(chd_file &chd, bool lossy)
	: chd_decompressor(chd, lossy)
{
	// init the inflater
	m_inflater.next_in = (Bytef *)this;	// bogus, but that's ok
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

chd_lzma_compressor::chd_lzma_compressor(chd_file &chd, bool lossy)
	: chd_compressor(chd, lossy)
{
	// initialize the properties
	configure_properties(m_props, chd);
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

void chd_lzma_compressor::configure_properties(CLzmaEncProps &props, chd_file &chd)
{
	LzmaEncProps_Init(&props);
	props.level = 9;
	props.reduceSize = chd.hunk_bytes();
	LzmaEncProps_Normalize(&props);
}



//**************************************************************************
//  LZMA DECOMPRESSOR
//**************************************************************************

//-------------------------------------------------
//  chd_lzma_decompressor - constructor
//-------------------------------------------------

chd_lzma_decompressor::chd_lzma_decompressor(chd_file &chd, bool lossy)
	: chd_decompressor(chd, lossy)
{
	// construct the decoder
	LzmaDec_Construct(&m_decoder);

	// configure the properties like the compressor did
	CLzmaEncProps encoder_props;
	chd_lzma_compressor::configure_properties(encoder_props, chd);

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

chd_huffman_compressor::chd_huffman_compressor(chd_file &chd, bool lossy)
	: chd_compressor(chd, lossy)
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

chd_huffman_decompressor::chd_huffman_decompressor(chd_file &chd, bool lossy)
	: chd_decompressor(chd, lossy)
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

chd_flac_compressor::chd_flac_compressor(chd_file &chd, bool lossy, bool bigendian)
	: chd_compressor(chd, lossy)
{
	// determine whether we want native or swapped samples
	UINT16 native_endian = 0;
	*reinterpret_cast<UINT8 *>(&native_endian) = 1;
	if (native_endian == 1)
		m_swap_endian = bigendian;
	else
		m_swap_endian = !bigendian;

	// configure the encoder
	m_encoder.set_sample_rate(44100);
	m_encoder.set_num_channels(2);
	m_encoder.set_block_size(chd.hunk_bytes() / 4);
	m_encoder.set_strip_metadata(true);
}


//-------------------------------------------------
//  compress - compress data using the FLAC codec
//-------------------------------------------------

UINT32 chd_flac_compressor::compress(const UINT8 *src, UINT32 srclen, UINT8 *dest)
{
	// reset and encode
	m_encoder.reset(dest, chd().hunk_bytes());
	if (!m_encoder.encode_interleaved(reinterpret_cast<const INT16 *>(src), srclen / 4, m_swap_endian))
		throw CHDERR_COMPRESSION_ERROR;

	// finish up
	UINT32 complen = m_encoder.finish();
	if (complen >= chd().hunk_bytes())
		throw CHDERR_COMPRESSION_ERROR;
	return complen;
}



//**************************************************************************
//  FLAC DECOMPRESSOR
//**************************************************************************

//-------------------------------------------------
//  chd_flac_decompressor - constructor
//-------------------------------------------------

chd_flac_decompressor::chd_flac_decompressor(chd_file &chd, bool lossy, bool bigendian)
	: chd_decompressor(chd, lossy)
{
	// determine whether we want native or swapped samples
	UINT16 native_endian = 0;
	*reinterpret_cast<UINT8 *>(&native_endian) = 1;
	if (native_endian == 1)
		m_swap_endian = bigendian;
	else
		m_swap_endian = !bigendian;
}


//-------------------------------------------------
//  decompress - decompress data using the FLAC
//  codec
//-------------------------------------------------

void chd_flac_decompressor::decompress(const UINT8 *src, UINT32 complen, UINT8 *dest, UINT32 destlen)
{
	// reset and decode
	if (!m_decoder.reset(44100, 2, chd().hunk_bytes() / 4, src, complen))
		throw CHDERR_DECOMPRESSION_ERROR;
	if (!m_decoder.decode_interleaved(reinterpret_cast<INT16 *>(dest), destlen / 4, m_swap_endian))
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

chd_cd_flac_compressor::chd_cd_flac_compressor(chd_file &chd, bool lossy)
	: chd_compressor(chd, lossy),
	  m_buffer(chd.hunk_bytes())
{
	// make sure the CHD's hunk size is an even multiple of the frame size
	if (chd.hunk_bytes() % CD_FRAME_SIZE != 0)
		throw CHDERR_CODEC_ERROR;

	// determine whether we want native or swapped samples
	UINT16 native_endian = 0;
	*reinterpret_cast<UINT8 *>(&native_endian) = 1;
	m_swap_endian = (native_endian == 1);

	// configure the encoder
	m_encoder.set_sample_rate(44100);
	m_encoder.set_num_channels(2);
	m_encoder.set_block_size((chd.hunk_bytes() / CD_FRAME_SIZE) * (CD_MAX_SECTOR_DATA/4));
	m_encoder.set_strip_metadata(true);

	// initialize the deflater
	m_deflater.next_in = (Bytef *)this;	// bogus, but that's ok
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
	UINT32 frames = chd().hunk_bytes() / CD_FRAME_SIZE;
	for (UINT32 framenum = 0; framenum < frames; framenum++)
	{
		memcpy(&m_buffer[framenum * CD_MAX_SECTOR_DATA], &src[framenum * CD_FRAME_SIZE], CD_MAX_SECTOR_DATA);
		memcpy(&m_buffer[frames * CD_MAX_SECTOR_DATA + framenum * CD_MAX_SUBCODE_DATA], &src[framenum * CD_FRAME_SIZE + CD_MAX_SECTOR_DATA], CD_MAX_SUBCODE_DATA);
	}

	// reset and encode the audio portion
	m_encoder.reset(dest, chd().hunk_bytes());
	UINT8 *buffer = m_buffer;
	if (!m_encoder.encode_interleaved(reinterpret_cast<INT16 *>(buffer), frames * CD_MAX_SECTOR_DATA/4, m_swap_endian))
		throw CHDERR_COMPRESSION_ERROR;

	// finish up
	UINT32 complen = m_encoder.finish();

	// deflate the subcode data
	m_deflater.next_in = const_cast<Bytef *>(&m_buffer[frames * CD_MAX_SECTOR_DATA]);
	m_deflater.avail_in = frames * CD_MAX_SUBCODE_DATA;
	m_deflater.total_in = 0;
	m_deflater.next_out = &dest[complen];
	m_deflater.avail_out = chd().hunk_bytes() - complen;
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



//**************************************************************************
//  CD FLAC DECOMPRESSOR
//**************************************************************************

//-------------------------------------------------
//  chd_cd_flac_decompressor - constructor
//-------------------------------------------------

chd_cd_flac_decompressor::chd_cd_flac_decompressor(chd_file &chd, bool lossy)
	: chd_decompressor(chd, lossy),
	  m_buffer(chd.hunk_bytes())
{
	// make sure the CHD's hunk size is an even multiple of the frame size
	if (chd.hunk_bytes() % CD_FRAME_SIZE != 0)
		throw CHDERR_CODEC_ERROR;

	// determine whether we want native or swapped samples
	UINT16 native_endian = 0;
	*reinterpret_cast<UINT8 *>(&native_endian) = 1;
	m_swap_endian = (native_endian == 1);

	// init the inflater
	m_inflater.next_in = (Bytef *)this;	// bogus, but that's ok
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
//  ~chd_cd_flac_decompressor - destructor
//-------------------------------------------------

chd_cd_flac_decompressor::~chd_cd_flac_decompressor()
{
	inflateEnd(&m_inflater);
}


//-------------------------------------------------
//  decompress - decompress data using the FLAC
//  codec
//-------------------------------------------------

void chd_cd_flac_decompressor::decompress(const UINT8 *src, UINT32 complen, UINT8 *dest, UINT32 destlen)
{
	// reset and decode
	UINT32 frames = chd().hunk_bytes() / CD_FRAME_SIZE;
	if (!m_decoder.reset(44100, 2, frames * CD_MAX_SECTOR_DATA/4, src, complen))
		throw CHDERR_DECOMPRESSION_ERROR;
	UINT8 *buffer = m_buffer;
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

//-------------------------------------------------
//  chd_avhuff_compressor - constructor
//-------------------------------------------------

chd_avhuff_compressor::chd_avhuff_compressor(chd_file &chd, bool lossy)
	: chd_compressor(chd, lossy),
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


//-------------------------------------------------
//  compress - compress data using the A/V codec
//-------------------------------------------------

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


//-------------------------------------------------
//  postinit - actual initialization of avhuff
//  happens here, on the first attempt to compress
//  or decompress data
//-------------------------------------------------

void chd_avhuff_compressor::postinit()
{
	// get the metadata
	astring metadata;
	chd_error err = chd().read_metadata(AV_METADATA_TAG, 0, metadata);
	if (err != CHDERR_NONE)
		throw err;

	// extract the info
	int fps, fpsfrac, width, height, interlaced, channels, rate;
	if (sscanf(metadata, AV_METADATA_FORMAT, &fps, &fpsfrac, &width, &height, &interlaced, &channels, &rate) != 7)
		throw CHDERR_INVALID_METADATA;

	// compute the bytes per frame
	UINT32 fps_times_1million = fps * 1000000 + fpsfrac;
	UINT32 max_samples_per_frame = (UINT64(rate) * 1000000 + fps_times_1million - 1) / fps_times_1million;
	UINT32 bytes_per_frame = 12 + channels * max_samples_per_frame * 2 + width * height * 2;
	if (bytes_per_frame > chd().hunk_bytes())
		throw CHDERR_INVALID_METADATA;

	// done with post-init
	m_postinit = true;
}



//**************************************************************************
//  AVHUFF DECOMPRESSOR
//**************************************************************************

//-------------------------------------------------
//  chd_avhuff_decompressor - constructor
//-------------------------------------------------

chd_avhuff_decompressor::chd_avhuff_decompressor(chd_file &chd, bool lossy)
	: chd_decompressor(chd, lossy)
{
}


//-------------------------------------------------
//  decompress - decompress data using the A/V
//  codec
//-------------------------------------------------

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


//-------------------------------------------------
//  config - codec-specific configuration for the
//  A/V codec
//-------------------------------------------------

void chd_avhuff_decompressor::configure(int param, void *config)
{
	// if we're getting the decompression configuration, apply it now
	if (param == AVHUFF_CODEC_DECOMPRESS_CONFIG)
		m_decoder.configure(*reinterpret_cast<avhuff_decompress_config *>(config));

	// anything else is invalid
	else
		throw CHDERR_INVALID_PARAMETER;
}
