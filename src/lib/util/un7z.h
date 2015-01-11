// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    un7z.h

    7z file management.

***************************************************************************/

// this is based on unzip.h, with modifications needed to use the 7zip library

#pragma once

#ifndef __UN_7Z_H__
#define __UN_7Z_H__

#include "osdcore.h"

#include "lzma/C/7z.h"
#include "lzma/C/7zCrc.h"
#include "lzma/C/7zVersion.h"


void *SZipAlloc(void *p, size_t size);
void SZipFree(void *p, void *address);
void *SZipAllocTemp(void *p, size_t size);
void SZipFreeTemp(void *p, void *address);

struct CSzFile
{
	long _7z_currfpos;
	UINT64          _7z_length;
	osd_file *      _7z_osdfile;                    /* OSD file handle */

};


struct CFileSeqInStream
{
	ISeqInStream s;
	CSzFile file;
};

void FileSeqInStream_CreateVTable(CFileSeqInStream *p);


struct CFileInStream
{
	ISeekInStream s;
	CSzFile file;
};

void FileInStream_CreateVTable(CFileInStream *p);


struct CFileOutStream
{
	ISeqOutStream s;
	CSzFile file;
} ;

void FileOutStream_CreateVTable(CFileOutStream *p);



/***************************************************************************
    CONSTANTS
***************************************************************************/


/* Error types */
enum _7z_error
{
	_7ZERR_NONE = 0,
	_7ZERR_OUT_OF_MEMORY,
	_7ZERR_FILE_ERROR,
	_7ZERR_BAD_SIGNATURE,
	_7ZERR_DECOMPRESS_ERROR,
	_7ZERR_FILE_TRUNCATED,
	_7ZERR_FILE_CORRUPT,
	_7ZERR_UNSUPPORTED,
	_7ZERR_BUFFER_TOO_SMALL
};



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* describes an open _7Z file */
struct  _7z_file
{
	const char *    filename;               /* copy of _7Z filename (for caching) */

	int curr_file_idx;                      /* current file index */
	UINT64 uncompressed_length;             /* current file uncompressed length */
	UINT64 crc;                             /* current file crc */

	CFileInStream archiveStream;
	CLookToRead lookStream;
	CSzArEx db;
	SRes res;
	ISzAlloc allocImp;
	ISzAlloc allocTempImp;
	bool inited;

	// cached stuff for solid blocks
	UInt32 blockIndex;// = 0xFFFFFFFF; /* it can have any value before first call (if outBuffer = 0) */
	Byte *outBuffer;// = 0; /* it must be 0 before first call for each new archive. */
	size_t outBufferSize;// = 0;  /* it can have any value before first call (if outBuffer = 0) */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- _7Z file access ----- */

/* open a _7Z file and parse its central directory */
_7z_error _7z_file_open(const char *filename, _7z_file **_7z);

/* close a _7Z file (may actually be left open due to caching) */
void _7z_file_close(_7z_file *_7z);

/* clear out all open _7Z files from the cache */
void _7z_file_cache_clear(void);


/* ----- contained file access ----- */

/* find a file index by crc, filename or both */
int _7z_search_crc_match(_7z_file *new_7z, UINT32 crc, const char *search_filename, int search_filename_length, bool matchcrc, bool matchname);

/* decompress the most recently found file in the _7Z */
_7z_error _7z_file_decompress(_7z_file *_7z, void *buffer, UINT32 length);


#endif  /* __UN_7Z_H__ */
