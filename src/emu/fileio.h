// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    fileio.h

    Core file I/O interface functions and definitions.

***************************************************************************/

#ifndef MAME_EMU_FILEIO_H
#define MAME_EMU_FILEIO_H

#pragma once

#include "corefile.h"
#include "hash.h"

// some systems use macros for getc/putc rather than functions
#ifdef getc
#undef getc
#endif

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> path_iterator

// helper class for iterating over configured paths
class path_iterator
{
public:
	// constructors
	path_iterator(std::string &&searchpath);
	path_iterator(std::string const &searchpath);
	path_iterator(path_iterator &&that);
	path_iterator(path_iterator const &that);

	// assignment operators
	path_iterator &operator=(path_iterator &&that);
	path_iterator &operator=(path_iterator const &that);

	// main interface
	bool next(std::string &buffer, const char *name = nullptr);
	void reset();

private:
	// internal state
	std::string                 m_searchpath;
	std::string::const_iterator m_current;
	bool                        m_is_first;
};



// ======================> file_enumerator

// iterate over all files in all paths specified in the searchpath
class file_enumerator
{
public:
	// construction/destruction
	template <typename... T> file_enumerator(T &&... args) : m_iterator(std::forward<T>(args)...) { }
	file_enumerator(file_enumerator &&) = default;
	file_enumerator(file_enumerator const &) = delete;
	file_enumerator &operator=(file_enumerator &&) = default;
	file_enumerator &operator=(file_enumerator const &) = delete;

	// iterator
	const osd::directory::entry *next();

private:
	// internal state
	path_iterator       m_iterator;
	osd::directory::ptr m_curdir;
	std::string         m_pathbuffer;
};



// ======================> emu_file

class emu_file
{
public:
	// file open/creation
	emu_file(u32 openflags);
	emu_file(std::string &&searchpath, u32 openflags);
	virtual ~emu_file();

	// getters
	operator util::core_file &();
	bool is_open() const { return bool(m_file); }
	const char *filename() const { return m_filename.c_str(); }
	const char *fullpath() const { return m_fullpath.c_str(); }
	u32 openflags() const { return m_openflags; }
	util::hash_collection &hashes(const char *types);
	bool restrict_to_mediapath() const { return m_restrict_to_mediapath; }
	bool part_of_mediapath(std::string path);

	// setters
	void remove_on_close() { m_remove_on_close = true; }
	void set_openflags(u32 openflags) { assert(!m_file); m_openflags = openflags; }
	void set_restrict_to_mediapath(bool rtmp = true) { m_restrict_to_mediapath = rtmp; }

	// open/close
	osd_file::error open(const std::string &name);
	osd_file::error open(const std::string &name1, const std::string &name2);
	osd_file::error open(const std::string &name1, const std::string &name2, const std::string &name3);
	osd_file::error open(const std::string &name1, const std::string &name2, const std::string &name3, const std::string &name4);
	osd_file::error open(const std::string &name, u32 crc);
	osd_file::error open(const std::string &name1, const std::string &name2, u32 crc);
	osd_file::error open(const std::string &name1, const std::string &name2, const std::string &name3, u32 crc);
	osd_file::error open(const std::string &name1, const std::string &name2, const std::string &name3, const std::string &name4, u32 crc);
	osd_file::error open_next();
	osd_file::error open_ram(const void *data, u32 length);
	void close();

	// control
	osd_file::error compress(int compress);

	// position
	int seek(s64 offset, int whence);
	u64 tell();
	bool eof();
	u64 size();

	// reading
	u32 read(void *buffer, u32 length);
	int getc();
	int ungetc(int c);
	char *gets(char *s, int n);

	// writing
	u32 write(const void *buffer, u32 length);
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
	std::string             m_filename;             // original filename provided
	std::string             m_fullpath;             // full filename
	util::core_file::ptr    m_file;                 // core file pointer
	path_iterator           m_iterator;             // iterator for paths
	path_iterator           m_mediapaths;           // media-path iterator
	u32                     m_crc;                  // file's CRC
	u32                     m_openflags;            // flags we used for the open
	util::hash_collection   m_hashes;               // collection of hashes

	std::unique_ptr<util::archive_file> m_zipfile;  // ZIP file pointer
	std::vector<u8>         m_zipdata;               // ZIP file data
	u64                     m_ziplength;             // ZIP file length

	bool                    m_remove_on_close;       // flag: remove the file when closing
	bool                    m_restrict_to_mediapath; // flag: restrict to paths inside the media-path
};

#endif // MAME_EMU_FILEIO_H
