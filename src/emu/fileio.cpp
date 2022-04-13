// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    fileio.cpp

    File access functions.

***************************************************************************/

#include "emu.h"
#include "fileio.h"

#include "util/path.h"
#include "util/unzip.h"

//#define VERBOSE 1
#define LOG_OUTPUT_FUNC osd_printf_verbose
#include "logmacro.h"


template path_iterator::path_iterator(char *&, int);
template path_iterator::path_iterator(char * const &, int);
template path_iterator::path_iterator(char const *&, int);
template path_iterator::path_iterator(char const * const &, int);
template path_iterator::path_iterator(std::vector<std::string> &, int);
template path_iterator::path_iterator(const std::vector<std::string> &, int);

template emu_file::emu_file(std::string &, u32);
template emu_file::emu_file(const std::string &, u32);
template emu_file::emu_file(char *&, u32);
template emu_file::emu_file(char * const &, u32);
template emu_file::emu_file(char const *&, u32);
template emu_file::emu_file(char const * const &, u32);
template emu_file::emu_file(std::vector<std::string> &, u32);
template emu_file::emu_file(const std::vector<std::string> &, u32);


const u32 OPEN_FLAG_HAS_CRC  = 0x10000;



//**************************************************************************
//  PATH ITERATOR
//**************************************************************************

//-------------------------------------------------
//  path_iterator - constructors
//-------------------------------------------------

path_iterator::path_iterator(std::string &&searchpath)
	: m_searchpath(std::move(searchpath))
	, m_current(m_searchpath.cbegin())
	, m_separator(';') // FIXME this should be a macro - UNIX prefers :
	, m_is_first(true)
{
}

path_iterator::path_iterator(std::string const &searchpath)
	: m_searchpath(searchpath)
	, m_current(m_searchpath.cbegin())
	, m_separator(';') // FIXME this should be a macro - UNIX prefers :
	, m_is_first(true)
{
}

path_iterator::path_iterator(path_iterator &&that)
{
	operator=(std::move(that));
}

path_iterator::path_iterator(path_iterator const &that)
	: m_searchpath(that.m_searchpath)
	, m_current(std::next(m_searchpath.cbegin(), std::distance(that.m_searchpath.cbegin(), that.m_current)))
	, m_separator(that.m_separator)
	, m_is_first(that.m_is_first)
{
}


//-------------------------------------------------
//  path_iterator - assignement operators
//-------------------------------------------------

path_iterator &path_iterator::operator=(path_iterator &&that)
{
	auto const current(std::distance(that.m_searchpath.cbegin(), that.m_current));
	m_searchpath = std::move(that.m_searchpath);
	m_current = std::next(m_searchpath.cbegin(), current);
	m_separator = that.m_separator;
	m_is_first = that.m_is_first;
	return *this;
}

path_iterator &path_iterator::operator=(path_iterator const &that)
{
	m_searchpath = that.m_searchpath;
	m_current = std::next(m_searchpath.cbegin(), std::distance(that.m_searchpath.cbegin(), that.m_current));
	m_separator = that.m_separator;
	m_is_first = that.m_is_first;
	return *this;
}


//-------------------------------------------------
//  path_iterator::next - get the next entry in a
//  multipath sequence
//-------------------------------------------------

bool path_iterator::next(std::string &buffer)
{
	// if none left, return false to indicate we are done
	if (!m_is_first && (m_searchpath.cend() == m_current))
		return false;

	// copy up to the next separator
	auto const sep(std::find(m_current, m_searchpath.cend(), m_separator));
	buffer.assign(m_current, sep);
	m_current = sep;
	if (m_searchpath.cend() != m_current)
		++m_current;

	// bump the index and return true
	m_is_first = false;
	return true;
}


//-------------------------------------------------
//  path_iterator::reset - let's go again
//-------------------------------------------------

void path_iterator::reset()
{
	m_current = m_searchpath.cbegin();
	m_is_first = true;
}



//**************************************************************************
//  FILE ENUMERATOR
//**************************************************************************

//-------------------------------------------------
//  next - return information about the next file
//  in the search path
//-------------------------------------------------

const osd::directory::entry *file_enumerator::next(const char *subdir)
{
	// loop over potentially empty directories
	while (true)
	{
		// if no open directory, get the next path
		while (!m_curdir)
		{
			// if we fail to get anything more, we're done
			if (!m_iterator.next(m_pathbuffer))
				return nullptr;

			// append the subdir if we have one
			if (subdir)
				util::path_append(m_pathbuffer, subdir);

			// open the path
			m_curdir = osd::directory::open(m_pathbuffer);
		}

		// get the next entry from the current directory
		const osd::directory::entry *const result = m_curdir->read();
		if (result)
			return result;

		// we're done; close this directory
		m_curdir.reset();
	}
}



//**************************************************************************
//  EMU FILE
//**************************************************************************

//-------------------------------------------------
//  emu_file - constructor
//-------------------------------------------------

emu_file::emu_file(u32 openflags)
	: emu_file(path_iterator(std::string()), openflags)
{

}

emu_file::emu_file(path_iterator &&searchpath, u32 openflags)
	: emu_file(openflags, EMPTY)
{
	m_iterator.emplace_back(searchpath, std::string());
	m_mediapaths.emplace_back(std::move(searchpath), std::string());
}

emu_file::emu_file(u32 openflags, empty_t)
	: m_filename()
	, m_fullpath()
	, m_file()
	, m_iterator()
	, m_mediapaths()
	, m_first(true)
	, m_crc(0)
	, m_openflags(openflags)
	, m_zipfile(nullptr)
	, m_ziplength(0)
	, m_remove_on_close(false)
	, m_restrict_to_mediapath(0)
{
	// sanity check the open flags
	if ((m_openflags & OPEN_FLAG_HAS_CRC) && (m_openflags & OPEN_FLAG_WRITE))
		throw emu_fatalerror("Attempted to open a file for write with OPEN_FLAG_HAS_CRC");
}


//-------------------------------------------------
//  ~emu_file - destructor
//-------------------------------------------------

emu_file::~emu_file()
{
	// close in the standard way
	close();
}


//-------------------------------------------------
//  operator util::core_file - automatically
//  convert ourselves to a core_file reference
//-------------------------------------------------

emu_file::operator util::core_file &()
{
	// load the ZIP file now if we haven't yet
	if (compressed_file_ready())
		throw emu_fatalerror("operator core_file & used on invalid file");

	// return the core file
	return *m_file;
}


//-------------------------------------------------
//  hash - returns the hash for a file
//-------------------------------------------------

util::hash_collection &emu_file::hashes(std::string_view types)
{
	// determine the hashes we already have
	std::string already_have = m_hashes.hash_types();

	// determine which hashes we need
	std::string needed;
	for (char scan : types)
		if (already_have.find_first_of(scan) == -1)
			needed.push_back(scan);

	// if we need nothing, skip it
	if (needed.empty())
		return m_hashes;

	// load the ZIP file if needed
	if (compressed_file_ready())
		return m_hashes;
	if (m_file == nullptr)
		return m_hashes;

	// if we have ZIP data, just hash that directly
	if (!m_zipdata.empty())
	{
		m_hashes.compute(&m_zipdata[0], m_zipdata.size(), needed.c_str());
		return m_hashes;
	}

	// read the data if we can
	const u8 *filedata = (const u8 *)m_file->buffer();
	if (filedata == nullptr)
		return m_hashes;

	// compute the hash
	std::uint64_t length;
	if (!m_file->length(length))
		m_hashes.compute(filedata, length, needed.c_str());
	return m_hashes;
}


//-------------------------------------------------
//  open - open a file by searching paths
//-------------------------------------------------

std::error_condition emu_file::open(std::string &&name)
{
	// remember the filename and CRC info
	m_filename = std::move(name);
	m_crc = 0;
	m_openflags &= ~OPEN_FLAG_HAS_CRC;

	// reset the iterator and open_next
	m_first = true;
	return open_next();
}

std::error_condition emu_file::open(std::string &&name, u32 crc)
{
	// remember the filename and CRC info
	m_filename = std::move(name);
	m_crc = crc;
	m_openflags |= OPEN_FLAG_HAS_CRC;

	// reset the iterator and open_next
	m_first = true;
	return open_next();
}


//-------------------------------------------------
//  open_next - open the next file that matches
//  the filename by iterating over paths
//-------------------------------------------------

std::error_condition emu_file::open_next()
{
	// if we're open from a previous attempt, close up now
	if (m_file)
		close();

	// loop over paths
	LOG("emu_file: open next '%s'\n", m_filename);
	std::error_condition filerr = std::errc::no_such_file_or_directory;
	while (filerr)
	{
		if (m_first)
		{
			m_first = false;
			for (searchpath_vector::value_type &i : m_iterator)
			{
				i.first.reset();
				if (!i.first.next(i.second))
					return filerr;
			}
		}
		else
		{
			searchpath_vector::iterator i(m_iterator.begin());
			while (i != m_iterator.end())
			{
				if (i->first.next(i->second))
				{
					LOG("emu_file: next path %d '%s'\n", std::distance(m_iterator.begin(), i), i->second);
					for (searchpath_vector::iterator j = m_iterator.begin(); i != j; ++j)
					{
						j->first.reset();
						j->first.next(j->second);
					}
					break;
				}
				++i;
			}
			if (m_iterator.end() == i)
				return filerr;
		}

		// build full path
		m_fullpath.clear();
		for (searchpath_vector::value_type const &path : m_iterator)
		{
			m_fullpath.append(path.second);
			if (!m_fullpath.empty() && !util::is_directory_separator(m_fullpath.back()))
				m_fullpath.append(PATH_SEPARATOR);
		}
		m_fullpath.append(m_filename);

		// attempt to open the file directly
		LOG("emu_file: attempting to open '%s' directly\n", m_fullpath);
		filerr = util::core_file::open(m_fullpath, m_openflags, m_file);

		// if we're opening for read-only we have other options
		if (filerr && ((m_openflags & (OPEN_FLAG_READ | OPEN_FLAG_WRITE)) == OPEN_FLAG_READ))
		{
			LOG("emu_file: attempting to open '%s' from archives\n", m_fullpath);
			filerr = attempt_zipped();
		}
	}
	return filerr;
}


//-------------------------------------------------
//  open_ram - open a "file" which is actually
//  just an array of data in RAM
//-------------------------------------------------

std::error_condition emu_file::open_ram(const void *data, u32 length)
{
	// set a fake filename and CRC
	m_filename = "RAM";
	m_crc = 0;

	// use the core_file's built-in RAM support
	return util::core_file::open_ram(data, length, m_openflags, m_file);
}


//-------------------------------------------------
//  close - close a file and free all data; also
//  remove the file if requested
//-------------------------------------------------

void emu_file::close()
{
	// close files and free memory
	m_zipfile.reset();
	m_file.reset();

	m_zipdata.clear();

	if (m_remove_on_close)
		osd_file::remove(m_fullpath);
	m_remove_on_close = false;

	// reset our hashes and path as well
	m_hashes.reset();
	m_fullpath.clear();
}


//-------------------------------------------------
//  compressed_file_ready - ensure our zip is ready
//   loading if needed
//-------------------------------------------------

std::error_condition emu_file::compressed_file_ready()
{
	// load the ZIP file now if we haven't yet
	return m_zipfile ? load_zipped_file() : std::error_condition();
}

//-------------------------------------------------
//  seek - seek within a file
//-------------------------------------------------

std::error_condition emu_file::seek(s64 offset, int whence)
{
	// load the ZIP file now if we haven't yet
	std::error_condition err = compressed_file_ready();
	if (err)
		return err;

	// seek if we can
	if (m_file)
		return m_file->seek(offset, whence);

	return std::errc::bad_file_descriptor; // TODO: revisit this error condition
}


//-------------------------------------------------
//  tell - return the current file position
//-------------------------------------------------

u64 emu_file::tell()
{
	// FIXME: need better interface to report errors
	// load the ZIP file now if we haven't yet
	if (compressed_file_ready())
		return 0;

	// tell if we can
	u64 result;
	if (m_file && !m_file->tell(result))
		return result;

	return 0;
}


//-------------------------------------------------
//  eof - return true if we're at the end of file
//-------------------------------------------------

bool emu_file::eof()
{
	// load the ZIP file now if we haven't yet
	if (compressed_file_ready())
		return 0;

	// return EOF if we can
	if (m_file)
		return m_file->eof();

	return 0;
}


//-------------------------------------------------
//  size - returns the size of a file
//-------------------------------------------------

u64 emu_file::size()
{
	// FIXME: need better interface to report errors
	// use the ZIP length if present
	if (m_zipfile)
		return m_ziplength;

	// return length if we can
	u64 result;
	if (m_file && !m_file->length(result))
		return result;

	return 0;
}


//-------------------------------------------------
//  read - read from a file
//-------------------------------------------------

u32 emu_file::read(void *buffer, u32 length)
{
	// FIXME: need better interface to report errors
	// load the ZIP file now if we haven't yet
	if (compressed_file_ready())
		return 0;

	// read the data if we can
	size_t actual = 0;
	if (m_file)
		m_file->read(buffer, length, actual);

	return actual;
}


//-------------------------------------------------
//  getc - read a character from a file
//-------------------------------------------------

int emu_file::getc()
{
	// load the ZIP file now if we haven't yet
	if (compressed_file_ready())
		return EOF;

	// read the data if we can
	if (m_file)
		return m_file->getc();

	return EOF;
}


//-------------------------------------------------
//  ungetc - put back a character read from a file
//-------------------------------------------------

int emu_file::ungetc(int c)
{
	// load the ZIP file now if we haven't yet
	if (compressed_file_ready())
		return 1;

	// read the data if we can
	if (m_file)
		return m_file->ungetc(c);

	return 1;
}


//-------------------------------------------------
//  gets - read a line from a text file
//-------------------------------------------------

char *emu_file::gets(char *s, int n)
{
	// load the ZIP file now if we haven't yet
	if (compressed_file_ready())
		return nullptr;

	// read the data if we can
	if (m_file)
		return m_file->gets(s, n);

	return nullptr;
}


//-------------------------------------------------
//  write - write to a file
//-------------------------------------------------

u32 emu_file::write(const void *buffer, u32 length)
{
	// FIXME: need better interface to report errors
	// write the data if we can
	size_t actual = 0;
	if (m_file)
		m_file->write(buffer, length, actual);

	return actual;
}


//-------------------------------------------------
//  puts - write a line to a text file
//-------------------------------------------------

int emu_file::puts(std::string_view s)
{
	// write the data if we can
	if (m_file)
		return m_file->puts(s);

	return 0;
}


//-------------------------------------------------
//  vfprintf - vfprintf to a text file
//-------------------------------------------------

int emu_file::vprintf(util::format_argument_pack<std::ostream> const &args)
{
	// write the data if we can
	return m_file ? m_file->vprintf(args) : 0;
}


//-------------------------------------------------
//  flush - flush file buffers
//-------------------------------------------------

void emu_file::flush()
{
	// flush the buffers if we can
	if (m_file)
		m_file->flush();
}


//-------------------------------------------------
//  part_of_mediapath - checks if 'path' is part of
//  any media path
//-------------------------------------------------

bool emu_file::part_of_mediapath(const std::string &path)
{
	if (!m_restrict_to_mediapath)
		return true;

	for (size_t i = 0U; (m_mediapaths.size() > i) && ((0 > m_restrict_to_mediapath) || (i < m_restrict_to_mediapath)); i++)
	{
		m_mediapaths[i].first.reset();
		if (!m_mediapaths[i].first.next(m_mediapaths[i].second))
			return false;
	}

	std::string mediapath;
	while (true)
	{
		mediapath.clear();
		for (size_t i = 0U; (m_mediapaths.size() > i) && ((0 > m_restrict_to_mediapath) || (i < m_restrict_to_mediapath)); i++)
		{
			mediapath.append(m_mediapaths[i].second);
			if (!mediapath.empty() && !util::is_directory_separator(mediapath.back()))
				mediapath.append(PATH_SEPARATOR);
		}

		if (!path.compare(0, mediapath.size(), mediapath))
		{
			LOG("emu_file: path '%s' matches media path '%s'\n", path, mediapath);
			return true;
		}

		size_t i = 0U;
		while ((m_mediapaths.size() > i) && ((0 > m_restrict_to_mediapath) || (i < m_restrict_to_mediapath)))
		{
			if (m_mediapaths[i].first.next(m_mediapaths[i].second))
			{
				for (size_t j = 0U; i != j; j++)
				{
					m_mediapaths[j].first.reset();
					m_mediapaths[j].first.next(m_mediapaths[j].second);
				}
				break;
			}
			i++;
		}
		if ((m_mediapaths.size() == i) || ((0 <= m_restrict_to_mediapath) && (i == m_restrict_to_mediapath)))
		{
			LOG("emu_file: path '%s' not in media path\n", path);
			return false;
		}
	}
}

//-------------------------------------------------
//  attempt_zipped - attempt to open a ZIPped file
//-------------------------------------------------

std::error_condition emu_file::attempt_zipped()
{
	typedef std::error_condition (*open_func)(std::string_view filename, util::archive_file::ptr &result);
	char const *const suffixes[] = { ".zip", ".7z" };
	open_func const open_funcs[std::size(suffixes)] = { &util::archive_file::open_zip, &util::archive_file::open_7z };

	// loop over archive types
	std::string const savepath(m_fullpath);
	std::string filename;
	for (unsigned i = 0; i < std::size(suffixes); i++, m_fullpath = savepath, filename.clear())
	{
		// loop over directory parts up to the start of filename
		while (1)
		{
			if (!part_of_mediapath(m_fullpath))
				break;

			// find the final path separator
			auto const dirsepiter(std::find_if(m_fullpath.rbegin(), m_fullpath.rend(), util::is_directory_separator));
			if (dirsepiter == m_fullpath.rend())
				break;
			std::string::size_type const dirsep(std::distance(m_fullpath.begin(), dirsepiter.base()) - 1);

			// insert the part from the right of the separator into the head of the filename
			if (!filename.empty())
				filename.insert(0, 1, '/');
			filename.insert(0, m_fullpath.substr(dirsep + 1, std::string::npos));

			// remove this part of the filename and append an archive extension
			m_fullpath.resize(dirsep);
			m_fullpath.append(suffixes[i]);
			LOG("emu_file: looking for '%s' in archive '%s'\n", filename, m_fullpath);

			// attempt to open the archive file
			util::archive_file::ptr zip;
			std::error_condition ziperr = open_funcs[i](m_fullpath, zip);

			// chop the archive suffix back off the filename before continuing
			m_fullpath = m_fullpath.substr(0, dirsep);

			// if we failed to open this file, continue scanning
			if (ziperr)
				continue;

			int header = -1;

			// see if we can find a file with the right name and (if available) CRC
			if (m_openflags & OPEN_FLAG_HAS_CRC)
				header = zip->search(m_crc, filename, false);
			if (header < 0 && (m_openflags & OPEN_FLAG_HAS_CRC))
				header = zip->search(m_crc, filename, true);

			// if that failed, look for a file with the right CRC, but the wrong filename
			if (header < 0 && (m_openflags & OPEN_FLAG_HAS_CRC))
				header = zip->search(m_crc);

			// if that failed, look for a file with the right name;
			// reporting a bad checksum is more helpful and less confusing than reporting "ROM not found"
			if (header < 0)
				header = zip->search(filename, false);
			if (header < 0)
				header = zip->search(filename, true);

			// if we got it, read the data
			if (header >= 0)
			{
				m_zipfile = std::move(zip);
				m_ziplength = m_zipfile->current_uncompressed_length();

				// build a hash with just the CRC
				m_hashes.reset();
				m_hashes.add_crc(m_zipfile->current_crc());
				m_fullpath = savepath;
				return (m_openflags & OPEN_FLAG_NO_PRELOAD) ? std::error_condition() : load_zipped_file();
			}

			// close up the archive file and try the next level
			zip.reset();
		}
	}
	return std::errc::no_such_file_or_directory;
}


//-------------------------------------------------
//  load_zipped_file - load a ZIPped file
//-------------------------------------------------

std::error_condition emu_file::load_zipped_file()
{
	assert(m_file == nullptr);
	assert(m_zipdata.empty());
	assert(m_zipfile);

	// allocate some memory
	m_zipdata.resize(m_ziplength);

	// read the data into our buffer and return
	auto const ziperr = m_zipfile->decompress(&m_zipdata[0], m_zipdata.size());
	if (ziperr)
	{
		m_zipdata.clear();
		return ziperr;
	}

	// convert to RAM file
	std::error_condition const filerr = util::core_file::open_ram(&m_zipdata[0], m_zipdata.size(), m_openflags, m_file);
	if (filerr)
	{
		m_zipdata.clear();
		return filerr;
	}

	// close out the ZIP file
	m_zipfile.reset();
	return std::error_condition();
}
