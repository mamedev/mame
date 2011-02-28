/***************************************************************************

    fileio.h

    Core file I/O interface functions and definitions.

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

#pragma once

#ifndef __FILEIO_H__
#define __FILEIO_H__

#include "corefile.h"
#include "hash.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// search paths
#define SEARCHPATH_RAW             NULL
#define SEARCHPATH_LANGUAGE        NULL
#define SEARCHPATH_DEBUGLOG        NULL

#define SEARCHPATH_ROM             OPTION_ROMPATH
#define SEARCHPATH_IMAGE           OPTION_ROMPATH
#define SEARCHPATH_HASH            OPTION_HASHPATH
#define SEARCHPATH_SAMPLE          OPTION_SAMPLEPATH
#define SEARCHPATH_ARTWORK         OPTION_ARTPATH
#define SEARCHPATH_CTRLR           OPTION_CTRLRPATH
#define SEARCHPATH_INI             OPTION_INIPATH
#define SEARCHPATH_FONT            OPTION_FONTPATH
#define SEARCHPATH_CHEAT           OPTION_CHEATPATH
#define SEARCHPATH_CROSSHAIRPATH   OPTION_CROSSHAIRPATH

#define SEARCHPATH_IMAGE_DIFF      OPTION_DIFF_DIRECTORY
#define SEARCHPATH_NVRAM           OPTION_NVRAM_DIRECTORY
#define SEARCHPATH_CONFIG          OPTION_CFG_DIRECTORY
#define SEARCHPATH_INPUTLOG        OPTION_INPUT_DIRECTORY
#define SEARCHPATH_STATE           OPTION_STATE_DIRECTORY
#define SEARCHPATH_MEMCARD         OPTION_MEMCARD_DIRECTORY
#define SEARCHPATH_SCREENSHOT      OPTION_SNAPSHOT_DIRECTORY
#define SEARCHPATH_MOVIE           OPTION_SNAPSHOT_DIRECTORY
#define SEARCHPATH_COMMENT         OPTION_COMMENT_DIRECTORY



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declarations
typedef struct _core_options core_options;
typedef struct _zip_file_header zip_file_header;
typedef struct _zip_file zip_file;


// ======================> path_iterator

// helper class for iterating over configured paths
class path_iterator
{
public:
	// construction/destruction
	path_iterator(core_options &options, const char *searchpath = "");

	// getters
	bool next(astring &buffer);

	// reset
	void reset() { m_current = m_base; m_index = 0; }

private:
	// internal state
	const char *	m_base;
	const char *	m_current;
	int				m_index;
};



// ======================> file_enumerator

// iterate over all files in all paths specified in the searchpath
class file_enumerator
{
public:
	// construction/destruction
	file_enumerator(core_options &opts, const char *searchpath);
	~file_enumerator();

	// iterator
	const osd_directory_entry *next();

private:
	// internal state
	path_iterator	m_iterator;
	osd_directory *	m_curdir;
	astring			m_pathbuffer;
	int				m_buflen;
};



// ======================> emu_file

class emu_file
{
public:
	// file open/creation
	emu_file(core_options &options, const char *searchpath, UINT32 openflags);
	virtual ~emu_file();

	// getters
	operator core_file *();
	bool open() const { return (m_file != NULL); }
	const char *filename() const { return m_filename; }
	const char *fullpath() const { return m_fullpath; }
	UINT32 openflags() const { return m_openflags; }
	hash_collection &hashes(const char *types);

	// setters
	void remove_on_close() { m_remove_on_close = true; }
	void set_openflags(UINT32 openflags) { assert(m_file == NULL); m_openflags = openflags; }

	// open/close
	file_error open(const char *name);
	file_error open(const char *name1, const char *name2);
	file_error open(const char *name1, const char *name2, const char *name3);
	file_error open(const char *name1, const char *name2, const char *name3, const char *name4);
	file_error open(const char *name, UINT32 crc);
	file_error open(const char *name1, const char *name2, UINT32 crc);
	file_error open(const char *name1, const char *name2, const char *name3, UINT32 crc);
	file_error open(const char *name1, const char *name2, const char *name3, const char *name4, UINT32 crc);
	file_error open_next();
	file_error open_ram(const void *data, UINT32 length);
	void close();

	// control
	file_error compress(int compress);
	int seek(INT64 offset, int whence);
	UINT64 tell();
	bool eof();
	UINT64 size();

	// reading
	UINT32 read(void *buffer, UINT32 length);
	int getc();
	int ungetc(int c);
	char *gets(char *s, int n);

	// writing
	UINT32 write(const void *buffer, UINT32 length);
	int puts(const char *s);
	int vprintf(const char *fmt, va_list va);
	int printf(const char *fmt, ...);

private:
	// internal helpers
	file_error attempt_zipped();
	file_error load_zipped_file();
	bool zip_filename_match(const zip_file_header &header, const astring &filename);
	bool zip_header_is_path(const zip_file_header &header);

	// internal state
	astring			m_filename;						// original filename provided
	astring			m_fullpath;						// full filename
	core_file *		m_file;							// core file pointer
	path_iterator	m_iterator;						// iterator for paths
	UINT32			m_crc;							// iterator for paths
	UINT32			m_openflags;					// flags we used for the open
	hash_collection m_hashes;						// collection of hashes
	zip_file *		m_zipfile;						// ZIP file pointer
	UINT8 *			m_zipdata;						// ZIP file data
	UINT64			m_ziplength;					// ZIP file length
	bool			m_remove_on_close;				// flag: remove the file when closing
};


#endif	/* __FILEIO_H__ */
