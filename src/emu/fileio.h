// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    fileio.h

    Core file I/O interface functions and definitions.

***************************************************************************/

#pragma once

#ifndef MAME_EMU_FILEIO_H
#define MAME_EMU_FILEIO_H

#include "corefile.h"
#include "hash.h"

// some systems use macros for getc/putc rather than functions
#ifdef getc
#undef getc
#endif

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declarations
namespace util { class archive_file; }

// ======================> path_iterator

// helper class for iterating over configured paths
class path_iterator
{
public:
	// construction/destruction
	path_iterator(const char *searchpath);

	// getters
	bool next(std::string &buffer, const char *name = nullptr);

	// reset
	void reset() { m_current = m_base; m_index = 0; }

private:
	// internal state
	const char *    m_base;
	const char *    m_current;
	int             m_index;
};



// ======================> file_enumerator

// iterate over all files in all paths specified in the searchpath
class file_enumerator
{
public:
	// construction/destruction
	file_enumerator(const char *searchpath);
	~file_enumerator();

	// iterator
	const osd_directory_entry *next();

private:
	// internal state
	path_iterator   m_iterator;
	osd_directory * m_curdir;
	std::string     m_pathbuffer;
	//int             m_buflen;
};



// ======================> emu_file

class emu_file
{
public:
	// file open/creation
	emu_file(UINT32 openflags);
	emu_file(const char *searchpath, UINT32 openflags);
	virtual ~emu_file();

	// getters
	operator util::core_file &();
	bool is_open() const { return bool(m_file); }
	const char *filename() const { return m_filename.c_str(); }
	const char *fullpath() const { return m_fullpath.c_str(); }
	UINT32 openflags() const { return m_openflags; }
	hash_collection &hashes(const char *types);
	bool restrict_to_mediapath() { return m_restrict_to_mediapath; }
	bool part_of_mediapath(std::string path);

	// setters
	void remove_on_close() { m_remove_on_close = true; }
	void set_openflags(UINT32 openflags) { assert(!m_file); m_openflags = openflags; }
	void set_restrict_to_mediapath(bool rtmp = true) { m_restrict_to_mediapath = rtmp; }

	// open/close
	osd_file::error open(const char *name);
	osd_file::error open(const char *name1, const char *name2);
	osd_file::error open(const char *name1, const char *name2, const char *name3);
	osd_file::error open(const char *name1, const char *name2, const char *name3, const char *name4);
	osd_file::error open(const char *name, UINT32 crc);
	osd_file::error open(const char *name1, const char *name2, UINT32 crc);
	osd_file::error open(const char *name1, const char *name2, const char *name3, UINT32 crc);
	osd_file::error open(const char *name1, const char *name2, const char *name3, const char *name4, UINT32 crc);
	osd_file::error open_next();
	osd_file::error open_ram(const void *data, UINT32 length);
	void close();

	// control
	osd_file::error compress(int compress);

	// position
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
	int vprintf(util::format_argument_pack<std::ostream> const &args);
	template <typename Format, typename... Params> int printf(Format &&fmt, Params &&...args)
	{
		return vprintf(util::make_format_argument_pack(std::forward<Format>(fmt), std::forward<Params>(args)...));
	}

	// buffers
	void flush();

private:
	bool compressed_file_ready(void);

	// internal helpers
	osd_file::error attempt_zipped();
	osd_file::error load_zipped_file();

	// internal state
	std::string     m_filename;                     // original filename provided
	std::string     m_fullpath;                     // full filename
	util::core_file::ptr m_file;                    // core file pointer
	path_iterator   m_iterator;                     // iterator for paths
	path_iterator   m_mediapaths;                   // media-path iterator
	UINT32          m_crc;                          // file's CRC
	UINT32          m_openflags;                    // flags we used for the open
	hash_collection m_hashes;                       // collection of hashes

	std::unique_ptr<util::archive_file> m_zipfile;  // ZIP file pointer
	dynamic_buffer  m_zipdata;                      // ZIP file data
	UINT64          m_ziplength;                    // ZIP file length

	bool            m_remove_on_close;              // flag: remove the file when closing
	bool            m_restrict_to_mediapath;        // flag: restrict to paths inside the media-path
};

#endif // MAME_EMU_FILEIO_H
