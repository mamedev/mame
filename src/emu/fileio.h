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

#include <iterator>
#include <string>
#include <system_error>
#include <type_traits>
#include <utility>
#include <vector>


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

	template <typename T>
	path_iterator(T &&searchpath, std::enable_if_t<std::is_constructible<std::string, T>::value, int> = 0)
		: path_iterator(std::string(std::forward<T>(searchpath)))
	{ }

	// TODO: this doesn't work with C arrays (only vector, std::array, etc.)
	template <typename T>
	path_iterator(T &&paths, std::enable_if_t<std::is_constructible<std::string, typename std::remove_reference_t<T>::value_type>::value, int> = 0)
		: path_iterator(concatenate_paths(std::forward<T>(paths)))
	{ m_separator = '\0'; }

	// assignment operators
	path_iterator &operator=(path_iterator &&that);
	path_iterator &operator=(path_iterator const &that);

	// main interface
	bool next(std::string &buffer);
	void reset();

private:
	// helpers
	template <typename T>
	static std::string concatenate_paths(T &&paths)
	{
		std::string result;
		auto it(std::begin(paths));
		if (std::end(paths) != it)
		{
			result.append(*it);
			++it;
		}
		while (std::end(paths) != it)
		{
			result.append(1, '\0');
			result.append(*it);
			++it;
		}
		return result;
	}

	// internal state
	std::string                 m_searchpath;
	std::string::const_iterator m_current;
	char                        m_separator;
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
	const osd::directory::entry *next(const char *subdir = nullptr);

private:
	// internal state
	path_iterator       m_iterator;
	osd::directory::ptr m_curdir;
	std::string         m_pathbuffer;
};



// ======================> emu_file

class emu_file
{
	enum empty_t { EMPTY };
	using searchpath_vector = std::vector<std::pair<path_iterator, std::string> >;

public:
	// file open/creation
	emu_file(u32 openflags);
	template <typename T>
	emu_file(T &&searchpath, std::enable_if_t<std::is_constructible<path_iterator, T>::value, u32> openflags)
		: emu_file(path_iterator(std::forward<T>(searchpath)), openflags)
	{ }
	template <typename T, typename U, typename V, typename... W>
	emu_file(T &&searchpath, U &&x, V &&y, W &&... z)
		: emu_file(0U, EMPTY)
	{
		m_iterator.reserve(sizeof...(W) + 1);
		m_mediapaths.reserve(sizeof...(W) + 1);
		set_searchpaths(std::forward<T>(searchpath), std::forward<U>(x), std::forward<V>(y), std::forward<W>(z)...);
	}
	virtual ~emu_file();

	// getters
	operator util::core_file &();
	bool is_open() const { return bool(m_file); }
	const char *filename() const { return m_filename.c_str(); }
	const char *fullpath() const { return m_fullpath.c_str(); }
	u32 openflags() const { return m_openflags; }
	util::hash_collection &hashes(std::string_view types);

	// setters
	void remove_on_close() { m_remove_on_close = true; }
	void set_openflags(u32 openflags) { assert(!m_file); m_openflags = openflags; }
	void set_restrict_to_mediapath(int rtmp) { m_restrict_to_mediapath = rtmp; }

	// open/close
	std::error_condition open(std::string &&name);
	std::error_condition open(std::string &&name, u32 crc);
	std::error_condition open(std::string_view name) { return open(std::string(name)); }
	std::error_condition open(std::string_view name, u32 crc) { return open(std::string(name), crc); }
	std::error_condition open(const char *name) { return open(std::string(name)); }
	std::error_condition open(const char *name, u32 crc) { return open(std::string(name), crc); }
	std::error_condition open_next();
	std::error_condition open_ram(const void *data, u32 length);
	void close();

	// position
	std::error_condition seek(s64 offset, int whence);
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
	int puts(std::string_view s);
	int vprintf(util::format_argument_pack<std::ostream> const &args);
	template <typename Format, typename... Params> int printf(Format &&fmt, Params &&...args)
	{
		return vprintf(util::make_format_argument_pack(std::forward<Format>(fmt), std::forward<Params>(args)...));
	}

	// buffers
	void flush();

private:
	emu_file(u32 openflags, empty_t);
	emu_file(path_iterator &&searchpath, u32 openflags);

	template <typename T>
	void set_searchpaths(T &&searchpath, u32 openflags)
	{
		m_iterator.emplace_back(searchpath, "");
		m_mediapaths.emplace_back(std::forward<T>(searchpath), "");
		m_openflags = openflags;
	}
	template <typename T, typename U, typename V, typename... W>
	void set_searchpaths(T &&searchpath, U &&x, V &&y, W &&... z)
	{
		m_iterator.emplace_back(searchpath, "");
		m_mediapaths.emplace_back(std::forward<T>(searchpath), "");
		set_searchpaths(std::forward<U>(x), std::forward<V>(y), std::forward<W>(z)...);
	}

	bool part_of_mediapath(const std::string &path);
	std::error_condition compressed_file_ready();

	// internal helpers
	std::error_condition attempt_zipped();
	std::error_condition load_zipped_file();

	// internal state
	std::string             m_filename;             // original filename provided
	std::string             m_fullpath;             // full filename
	util::core_file::ptr    m_file;                 // core file pointer
	searchpath_vector       m_iterator;             // iterator for paths
	searchpath_vector       m_mediapaths;           // media-path iterator
	bool                    m_first;                // true if this is the start of iteration
	u32                     m_crc;                  // file's CRC
	u32                     m_openflags;            // flags we used for the open
	util::hash_collection   m_hashes;               // collection of hashes

	std::unique_ptr<util::archive_file> m_zipfile;  // ZIP file pointer
	std::vector<u8>         m_zipdata;              // ZIP file data
	u64                     m_ziplength;            // ZIP file length

	bool                    m_remove_on_close;      // flag: remove the file when closing
	int                     m_restrict_to_mediapath; // flag: restrict to paths inside the media-path
};


extern template path_iterator::path_iterator(char *&, int);
extern template path_iterator::path_iterator(char * const &, int);
extern template path_iterator::path_iterator(char const *&, int);
extern template path_iterator::path_iterator(char const * const &, int);
extern template path_iterator::path_iterator(std::vector<std::string> &, int);
extern template path_iterator::path_iterator(const std::vector<std::string> &, int);

extern template emu_file::emu_file(std::string &, u32);
extern template emu_file::emu_file(const std::string &, u32);
extern template emu_file::emu_file(char *&, u32);
extern template emu_file::emu_file(char * const &, u32);
extern template emu_file::emu_file(char const *&, u32);
extern template emu_file::emu_file(char const * const &, u32);
extern template emu_file::emu_file(std::vector<std::string> &, u32);
extern template emu_file::emu_file(const std::vector<std::string> &, u32);

#endif // MAME_EMU_FILEIO_H
