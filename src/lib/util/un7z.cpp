// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    un7z.c

    Functions to manipulate data within 7z files.

***************************************************************************/

// this is based on unzip.c, with modifications needed to use the 7zip library

#include "osdcore.h"
#include "un7z.h"

#include <ctype.h>
#include <stdlib.h>
#include <zlib.h>

/***************************************************************************
    7Zip Memory / File handling (adapted from 7zfile.c/.h and 7zalloc.c/.h)
***************************************************************************/

void *SZipAlloc(void *p, size_t size)
{
	if (size == 0)
		return nullptr;

	return malloc(size);
}

void SZipFree(void *p, void *address)
{
	free(address);
}



void File_Construct(CSzFile *p)
{
	p->_7z_osdfile = nullptr;
}

static WRes File_Open(CSzFile *p, const char *name, int writeMode)
{
	/* we handle this ourselves ... */
	if (!p->_7z_osdfile) return 1;
	else return 0;
}

WRes InFile_Open(CSzFile *p, const char *name) { return File_Open(p, name, 0); }
WRes OutFile_Open(CSzFile *p, const char *name) { return File_Open(p, name, 1); }


WRes File_Close(CSzFile *p)
{
	/* we handle this ourselves ... */
	return 0;
}

WRes File_Read(CSzFile *p, void *data, size_t *size)
{
//  file_error err;
	UINT32 read_length;

	if (!p->_7z_osdfile)
	{
		printf("un7z.c: called File_Read without file\n");
		return 1;
	}

	size_t originalSize = *size;
	if (originalSize == 0)
		return 0;

//  err =
	osd_read( p->_7z_osdfile, data, p->_7z_currfpos, originalSize, &read_length );
	*size = read_length;
	p->_7z_currfpos += read_length;

	if (*size == originalSize)
		return 0;

	return 0;
}

WRes File_Write(CSzFile *p, const void *data, size_t *size)
{
	return 0;
}

WRes File_Seek(CSzFile *p, Int64 *pos, ESzSeek origin)
{
	if (origin==0) p->_7z_currfpos = *pos;
	if (origin==1) p->_7z_currfpos = p->_7z_currfpos + *pos;
	if (origin==2) p->_7z_currfpos = p->_7z_length - *pos;

	*pos = p->_7z_currfpos;

	return 0;
}

WRes File_GetLength(CSzFile *p, UInt64 *length)
{
	*length = p->_7z_length;
	return 0;
}

/* ---------- FileSeqInStream ---------- */

static SRes FileSeqInStream_Read(void *pp, void *buf, size_t *size)
{
	CFileSeqInStream *p = (CFileSeqInStream *)pp;
	return File_Read(&p->file, buf, size) == 0 ? SZ_OK : SZ_ERROR_READ;
}

void FileSeqInStream_CreateVTable(CFileSeqInStream *p)
{
	p->s.Read = FileSeqInStream_Read;
}


/* ---------- FileInStream ---------- */

static SRes FileInStream_Read(void *pp, void *buf, size_t *size)
{
	CFileInStream *p = (CFileInStream *)pp;
	return (File_Read(&p->file, buf, size) == 0) ? SZ_OK : SZ_ERROR_READ;
}

static SRes FileInStream_Seek(void *pp, Int64 *pos, ESzSeek origin)
{
	CFileInStream *p = (CFileInStream *)pp;
	return File_Seek(&p->file, pos, origin);
}

void FileInStream_CreateVTable(CFileInStream *p)
{
	p->s.Read = FileInStream_Read;
	p->s.Seek = FileInStream_Seek;
}

/* ---------- FileOutStream ---------- */

static size_t FileOutStream_Write(void *pp, const void *data, size_t size)
{
	CFileOutStream *p = (CFileOutStream *)pp;
	File_Write(&p->file, data, &size);
	return size;
}

void FileOutStream_CreateVTable(CFileOutStream *p)
{
	p->s.Write = FileOutStream_Write;
}

/***************************************************************************
    CONSTANTS
***************************************************************************/

/* number of open files to cache */
#define _7Z_CACHE_SIZE  8


/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static _7z_file *_7z_cache[_7Z_CACHE_SIZE];

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* cache management */
static void free__7z_file(_7z_file *_7z);


/***************************************************************************
    _7Z FILE ACCESS
***************************************************************************/

/*-------------------------------------------------
    _7z_file_open - opens a _7Z file for reading
-------------------------------------------------*/

int _7z_search_crc_match(_7z_file *new_7z, UINT32 search_crc, const char* search_filename, int search_filename_length, bool matchcrc, bool matchname)
{
	UInt16 *temp = nullptr;
	size_t tempSize = 0;

	for (int i = 0; i < new_7z->db.db.NumFiles; i++)
	{
		const CSzFileItem *f = new_7z->db.db.Files + i;
		size_t len;

		len = SzArEx_GetFileNameUtf16(&new_7z->db, i, nullptr);

		// if it's a directory entry we don't care about it..
		if (f->IsDir)
			continue;

		if (len > tempSize)
		{
			SZipFree(nullptr, temp);
			tempSize = len;
			temp = (UInt16 *)SZipAlloc(nullptr, tempSize * sizeof(temp[0]));
			if (temp == nullptr)
			{
				return -1; // memory error
			}
		}

		bool crcmatch = false;
		bool namematch = false;

		UINT64 size = f->Size;
		UINT32 crc = f->Crc;

		/* Check for a name match */
		SzArEx_GetFileNameUtf16(&new_7z->db, i, temp);

		if (len == search_filename_length+1)
		{
			int j;
			for (j=0;j<search_filename_length;j++)
			{
				UINT8 sn = search_filename[j];
				UINT16 zn = temp[j]; // these are utf16

				// MAME filenames are always lowercase so be case insensitive
				if ((zn>=0x41) && (zn<=0x5a)) zn+=0x20;

				if (sn != zn) break;
			}
			if (j==search_filename_length) namematch = true;
		}


		/* Check for a CRC match */
		if (crc==search_crc) crcmatch = true;

		bool found = false;

		if (matchcrc && matchname)
		{
			if (crcmatch && namematch)
				found = true;
		}
		else if (matchcrc)
		{
			if (crcmatch)
				found = true;
		}
		else if (matchname)
		{
			if (namematch)
				found = true;
		}

		if (found)
		{
		//  printf("found %S %d %08x %08x %08x %s %d\n", temp, len, crc, search_crc, size, search_filename, search_filename_length);
			new_7z->curr_file_idx = i;
			new_7z->uncompressed_length = size;
			new_7z->crc = crc;

			SZipFree(nullptr, temp);
			return i;
		}
	}

	SZipFree(nullptr, temp);
	return -1;
}


_7z_error _7z_file_open(const char *filename, _7z_file **_7z)
{
	file_error err;
	_7z_error _7zerr = _7ZERR_NONE;


	_7z_file *new_7z;
	char *string;
	int cachenum;

	SRes res;

	/* ensure we start with a NULL result */
	*_7z = nullptr;

	/* see if we are in the cache, and reopen if so */
	for (cachenum = 0; cachenum < ARRAY_LENGTH(_7z_cache); cachenum++)
	{
		_7z_file *cached = _7z_cache[cachenum];

		/* if we have a valid entry and it matches our filename, use it and remove from the cache */
		if (cached != nullptr && cached->filename != nullptr && strcmp(filename, cached->filename) == 0)
		{
			*_7z = cached;
			_7z_cache[cachenum] = nullptr;
			return _7ZERR_NONE;
		}
	}

	/* allocate memory for the _7z_file structure */
	new_7z = (_7z_file *)malloc(sizeof(*new_7z));
	if (new_7z == nullptr)
		return _7ZERR_OUT_OF_MEMORY;
	memset(new_7z, 0, sizeof(*new_7z));

	new_7z->inited = false;
	new_7z->archiveStream.file._7z_currfpos = 0;
	err = osd_open(filename, OPEN_FLAG_READ, &new_7z->archiveStream.file._7z_osdfile, &new_7z->archiveStream.file._7z_length);
	if (err != FILERR_NONE)
	{
		_7zerr = _7ZERR_FILE_ERROR;
		goto error;
	}

	new_7z->allocImp.Alloc = SZipAlloc;
	new_7z->allocImp.Free = SZipFree;

	new_7z->allocTempImp.Alloc = SZipAlloc;
	new_7z->allocTempImp.Free = SZipFree;

	if (InFile_Open(&new_7z->archiveStream.file, filename))
	{
		_7zerr = _7ZERR_FILE_ERROR;
		goto error;
	}

	FileInStream_CreateVTable(&new_7z->archiveStream);
	LookToRead_CreateVTable(&new_7z->lookStream, False);

	new_7z->lookStream.realStream = &new_7z->archiveStream.s;
	LookToRead_Init(&new_7z->lookStream);

	CrcGenerateTable();

	SzArEx_Init(&new_7z->db);
	new_7z->inited = true;

	res = SzArEx_Open(&new_7z->db, &new_7z->lookStream.s, &new_7z->allocImp, &new_7z->allocTempImp);
	if (res != SZ_OK)
	{
		_7zerr = _7ZERR_FILE_ERROR;
		goto error;
	}

	new_7z->blockIndex = 0xFFFFFFFF; /* it can have any value before first call (if outBuffer = 0) */
	new_7z->outBuffer = nullptr; /* it must be 0 before first call for each new archive. */
	new_7z->outBufferSize = 0;  /* it can have any value before first call (if outBuffer = 0) */

	/* make a copy of the filename for caching purposes */
	string = (char *)malloc(strlen(filename) + 1);
	if (string == nullptr)
	{
		_7zerr = _7ZERR_OUT_OF_MEMORY;
		goto error;
	}
	strcpy(string, filename);
	new_7z->filename = string;
	*_7z = new_7z;
	return _7ZERR_NONE;

error:
	free__7z_file(new_7z);
	return _7zerr;
}


/*-------------------------------------------------
    _7z_file_close - close a _7Z file and add it
    to the cache
-------------------------------------------------*/

void _7z_file_close(_7z_file *_7z)
{
	int cachenum;

	/* close the open files */
	if (_7z->archiveStream.file._7z_osdfile != nullptr)
		osd_close(_7z->archiveStream.file._7z_osdfile);
	_7z->archiveStream.file._7z_osdfile = nullptr;

	/* find the first NULL entry in the cache */
	for (cachenum = 0; cachenum < ARRAY_LENGTH(_7z_cache); cachenum++)
		if (_7z_cache[cachenum] == nullptr)
			break;

	/* if no room left in the cache, free the bottommost entry */
	if (cachenum == ARRAY_LENGTH(_7z_cache))
		free__7z_file(_7z_cache[--cachenum]);

	/* move everyone else down and place us at the top */
	if (cachenum != 0)
		memmove(&_7z_cache[1], &_7z_cache[0], cachenum * sizeof(_7z_cache[0]));
	_7z_cache[0] = _7z;
}


/*-------------------------------------------------
    _7z_file_cache_clear - clear the _7Z file
    cache and free all memory
-------------------------------------------------*/

void _7z_file_cache_clear(void)
{
	int cachenum;

	/* clear call cache entries */
	for (cachenum = 0; cachenum < ARRAY_LENGTH(_7z_cache); cachenum++)
		if (_7z_cache[cachenum] != nullptr)
		{
			free__7z_file(_7z_cache[cachenum]);
			_7z_cache[cachenum] = nullptr;
		}
}


/*-------------------------------------------------
    _7z_file_decompress - decompress a file
    from a _7Z into the target buffer
-------------------------------------------------*/

_7z_error _7z_file_decompress(_7z_file *new_7z, void *buffer, UINT32 length)
{
	file_error err;
	SRes res;
	int index = new_7z->curr_file_idx;

	/* make sure the file is open.. */
	if (new_7z->archiveStream.file._7z_osdfile==nullptr)
	{
		new_7z->archiveStream.file._7z_currfpos = 0;
		err = osd_open(new_7z->filename, OPEN_FLAG_READ, &new_7z->archiveStream.file._7z_osdfile, &new_7z->archiveStream.file._7z_length);
		if (err != FILERR_NONE)
			return _7ZERR_FILE_ERROR;
	}

	size_t offset = 0;
	size_t outSizeProcessed = 0;

	res = SzArEx_Extract(&new_7z->db, &new_7z->lookStream.s, index,
		&new_7z->blockIndex, &new_7z->outBuffer, &new_7z->outBufferSize,
		&offset, &outSizeProcessed,
		&new_7z->allocImp, &new_7z->allocTempImp);

	if (res != SZ_OK)
		return _7ZERR_FILE_ERROR;

	memcpy(buffer, new_7z->outBuffer + offset, length);

	return _7ZERR_NONE;
}



/***************************************************************************
    CACHE MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    free__7z_file - free all the data for a
    _7z_file
-------------------------------------------------*/

/**
 * @fn  static void free__7z_file(_7z_file *_7z)
 *
 * @brief   Free 7z file.
 *
 * @param [in,out]  _7z If non-null, the 7z.
 */

static void free__7z_file(_7z_file *_7z)
{
	if (_7z != nullptr)
	{
		if (_7z->archiveStream.file._7z_osdfile != nullptr)
			osd_close(_7z->archiveStream.file._7z_osdfile);
		if (_7z->filename != nullptr)
			free((void *)_7z->filename);


		if (_7z->outBuffer) IAlloc_Free(&_7z->allocImp, _7z->outBuffer);
		if (_7z->inited) SzArEx_Free(&_7z->db, &_7z->allocImp);


		free(_7z);
	}
}
