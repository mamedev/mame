// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    fileio.c

    File access functions.

***************************************************************************/

#include "emu.h"
#include "unzip.h"
#include "fileio.h"


const UINT32 OPEN_FLAG_HAS_CRC  = 0x10000;



//**************************************************************************
//  PATH ITERATOR
//**************************************************************************

//-------------------------------------------------
//  path_iterator - constructor
//-------------------------------------------------

path_iterator::path_iterator(const char *rawsearchpath)
	: m_base(rawsearchpath),
		m_current(m_base),
		m_index(0)
{
}


//-------------------------------------------------
//  path_iterator_get_next - get the next entry
//  in a multipath sequence
//-------------------------------------------------

bool path_iterator::next(std::string &buffer, const char *name)
{
	// if none left, return FALSE to indicate we are done
	if (m_index != 0 && *m_current == 0)
		return false;

	// copy up to the next semicolon
	const char *semi = strchr(m_current, ';');
	if (semi == nullptr)
		semi = m_current + strlen(m_current);
	buffer.assign(m_current, semi - m_current);
	m_current = (*semi == 0) ? semi : semi + 1;

	// append the name if we have one
	if (name != nullptr)
	{
		// compute the full pathname
		if (buffer.length() > 0)
			buffer.append(PATH_SEPARATOR);
		buffer.append(name);
	}

	// bump the index and return TRUE
	m_index++;
	return true;
}



//**************************************************************************
//  FILE ENUMERATOR
//**************************************************************************

//-------------------------------------------------
//  file_enumerator - constructor
//-------------------------------------------------

file_enumerator::file_enumerator(const char *searchpath)
	: m_iterator(searchpath),
		m_curdir(nullptr)/*,
        m_buflen(0)*/
{
}


//-------------------------------------------------
//  ~file_enumerator - destructor
//-------------------------------------------------

file_enumerator::~file_enumerator()
{
	// close anything open
	if (m_curdir != nullptr)
		osd_closedir(m_curdir);
}


//-------------------------------------------------
//  next - return information about the next file
//  in the search path
//-------------------------------------------------

const osd_directory_entry *file_enumerator::next()
{
	// loop over potentially empty directories
	while (1)
	{
		// if no open directory, get the next path
		while (m_curdir == nullptr)
		{
			// if we fail to get anything more, we're done
			if (!m_iterator.next(m_pathbuffer))
				return nullptr;

			// open the path
			m_curdir = osd_opendir(m_pathbuffer.c_str());
		}

		// get the next entry from the current directory
		const osd_directory_entry *result = osd_readdir(m_curdir);
		if (result != nullptr)
			return result;

		// we're done; close this directory
		osd_closedir(m_curdir);
		m_curdir = nullptr;
	}
}



//**************************************************************************
//  EMU FILE
//**************************************************************************

//-------------------------------------------------
//  emu_file - constructor
//-------------------------------------------------

emu_file::emu_file(UINT32 openflags)
	: m_file(nullptr),
		m_iterator(""),
		m_mediapaths(""),
		m_crc(0),
		m_openflags(openflags),
		m_zipfile(nullptr),
		m_ziplength(0),
		m_remove_on_close(false),
		m_restrict_to_mediapath(false)
{
	// sanity check the open flags
	if ((m_openflags & OPEN_FLAG_HAS_CRC) && (m_openflags & OPEN_FLAG_WRITE))
		throw emu_fatalerror("Attempted to open a file for write with OPEN_FLAG_HAS_CRC");
}

emu_file::emu_file(const char *searchpath, UINT32 openflags)
	: m_file(nullptr),
		m_iterator(searchpath),
		m_mediapaths(searchpath),
		m_crc(0),
		m_openflags(openflags),
		m_zipfile(nullptr),
		m_ziplength(0),
		m_remove_on_close(false),
		m_restrict_to_mediapath(false)
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

hash_collection &emu_file::hashes(const char *types)
{
	// determine the hashes we already have
	std::string already_have = m_hashes.hash_types();

	// determine which hashes we need
	std::string needed;
	for (const char *scan = types; *scan != 0; scan++)
		if (already_have.find_first_of(*scan) == -1)
			needed.push_back(*scan);

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
	const UINT8 *filedata = (const UINT8 *)m_file->buffer();
	if (filedata == nullptr)
		return m_hashes;

	// compute the hash
	m_hashes.compute(filedata, m_file->size(), needed.c_str());
	return m_hashes;
}


//-------------------------------------------------
//  open - open a file by searching paths
//-------------------------------------------------

osd_file::error emu_file::open(const char *name)
{
	// remember the filename and CRC info
	m_filename = name;
	m_crc = 0;
	m_openflags &= ~OPEN_FLAG_HAS_CRC;

	// reset the iterator and open_next
	m_iterator.reset();
	return open_next();
}

osd_file::error emu_file::open(const char *name1, const char *name2)
{
	// concatenate the strings and do a standard open
	std::string name = std::string(name1).append(name2);
	return open(name.c_str());
}

osd_file::error emu_file::open(const char *name1, const char *name2, const char *name3)
{
	// concatenate the strings and do a standard open
	std::string name = std::string(name1).append(name2).append(name3);
	return open(name.c_str());
}

osd_file::error emu_file::open(const char *name1, const char *name2, const char *name3, const char *name4)
{
	// concatenate the strings and do a standard open
	std::string name = std::string(name1).append(name2).append(name3).append(name4);
	return open(name.c_str());
}

osd_file::error emu_file::open(const char *name, UINT32 crc)
{
	// remember the filename and CRC info
	m_filename = name;
	m_crc = crc;
	m_openflags |= OPEN_FLAG_HAS_CRC;

	// reset the iterator and open_next
	m_iterator.reset();
	return open_next();
}

osd_file::error emu_file::open(const char *name1, const char *name2, UINT32 crc)
{
	// concatenate the strings and do a standard open
	std::string name = std::string(name1).append(name2);
	return open(name.c_str(), crc);
}

osd_file::error emu_file::open(const char *name1, const char *name2, const char *name3, UINT32 crc)
{
	// concatenate the strings and do a standard open
	std::string name = std::string(name1).append(name2).append(name3);
	return open(name.c_str(), crc);
}

osd_file::error emu_file::open(const char *name1, const char *name2, const char *name3, const char *name4, UINT32 crc)
{
	// concatenate the strings and do a standard open
	std::string name = std::string(name1).append(name2).append(name3).append(name4);
	return open(name.c_str(), crc);
}


//-------------------------------------------------
//  open_next - open the next file that matches
//  the filename by iterating over paths
//-------------------------------------------------

osd_file::error emu_file::open_next()
{
	// if we're open from a previous attempt, close up now
	if (m_file != nullptr)
		close();

	// loop over paths
	osd_file::error filerr = osd_file::error::NOT_FOUND;
	while (m_iterator.next(m_fullpath, m_filename.c_str()))
	{
		// attempt to open the file directly
		filerr = util::core_file::open(m_fullpath, m_openflags, m_file);
		if (filerr == osd_file::error::NONE)
			break;

		// if we're opening for read-only we have other options
		if ((m_openflags & (OPEN_FLAG_READ | OPEN_FLAG_WRITE)) == OPEN_FLAG_READ)
		{
			filerr = attempt_zipped();
			if (filerr == osd_file::error::NONE)
				break;
		}
	}
	return filerr;
}


//-------------------------------------------------
//  open_ram - open a "file" which is actually
//  just an array of data in RAM
//-------------------------------------------------

osd_file::error emu_file::open_ram(const void *data, UINT32 length)
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
//  compress - enable/disable streaming file
//  compression via zlib; level is 0 to disable
//  compression, or up to 9 for max compression
//-------------------------------------------------

osd_file::error emu_file::compress(int level)
{
	return m_file->compress(level);
}


//-------------------------------------------------
//  compressed_file_ready - ensure our zip is ready
//   loading if needed
//-------------------------------------------------

bool emu_file::compressed_file_ready(void)
{
	// load the ZIP file now if we haven't yet
	if (m_zipfile && (load_zipped_file() != osd_file::error::NONE))
		return true;

	return false;
}

//-------------------------------------------------
//  seek - seek within a file
//-------------------------------------------------

int emu_file::seek(INT64 offset, int whence)
{
	// load the ZIP file now if we haven't yet
	if (compressed_file_ready())
		return 1;

	// seek if we can
	if (m_file)
		return m_file->seek(offset, whence);

	return 1;
}


//-------------------------------------------------
//  tell - return the current file position
//-------------------------------------------------

UINT64 emu_file::tell()
{
	// load the ZIP file now if we haven't yet
	if (compressed_file_ready())
		return 0;

	// tell if we can
	if (m_file)
		return m_file->tell();

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

UINT64 emu_file::size()
{
	// use the ZIP length if present
	if (m_zipfile != nullptr)
		return m_ziplength;

	// return length if we can
	if (m_file)
		return m_file->size();

	return 0;
}


//-------------------------------------------------
//  read - read from a file
//-------------------------------------------------

UINT32 emu_file::read(void *buffer, UINT32 length)
{
	// load the ZIP file now if we haven't yet
	if (compressed_file_ready())
		return 0;

	// read the data if we can
	if (m_file)
		return m_file->read(buffer, length);

	return 0;
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

UINT32 emu_file::write(const void *buffer, UINT32 length)
{
	// write the data if we can
	if (m_file)
		return m_file->write(buffer, length);

	return 0;
}


//-------------------------------------------------
//  puts - write a line to a text file
//-------------------------------------------------

int emu_file::puts(const char *s)
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

bool emu_file::part_of_mediapath(std::string path)
{
	bool result = false;
	std::string mediapath;
	m_mediapaths.reset();
	while (m_mediapaths.next(mediapath, nullptr) && !result) {
		if (path.compare(mediapath.substr(0, mediapath.length())))
			result = true;
	}
	return result;
}

//-------------------------------------------------
//  attempt_zipped - attempt to open a ZIPped file
//-------------------------------------------------

osd_file::error emu_file::attempt_zipped()
{
	typedef util::archive_file::error (*open_func)(const std::string &filename, util::archive_file::ptr &result);
	char const *const suffixes[] = { ".zip", ".7z" };
	open_func const open_funcs[ARRAY_LENGTH(suffixes)] = { &util::archive_file::open_zip, &util::archive_file::open_7z };

	// loop over archive types
	std::string const savepath(m_fullpath);
	std::string filename;
	for (unsigned i = 0; i < ARRAY_LENGTH(suffixes); i++, m_fullpath = savepath, filename.clear())
	{
		// loop over directory parts up to the start of filename
		while (1)
		{
			// find the final path separator
			auto const dirsep = m_fullpath.find_last_of(PATH_SEPARATOR[0]);
			if (dirsep == std::string::npos)
				break;

			if (restrict_to_mediapath() && !part_of_mediapath(m_fullpath))
				break;

			// insert the part from the right of the separator into the head of the filename
			if (!filename.empty()) filename.insert(0, 1, '/');
			filename.insert(0, m_fullpath.substr(dirsep + 1, std::string::npos));

			// remove this part of the filename and append an archive extension
			m_fullpath.resize(dirsep);
			m_fullpath.append(suffixes[i]);

			// attempt to open the archive file
			util::archive_file::ptr zip;
			util::archive_file::error ziperr = open_funcs[i](m_fullpath, zip);

			// chop the archive suffix back off the filename before continuing
			m_fullpath = m_fullpath.substr(0, dirsep);

			// if we failed to open this file, continue scanning
			if (ziperr != util::archive_file::error::NONE)
				continue;

			int header = -1;

			// see if we can find a file with the right name and (if available) CRC
			if (m_openflags & OPEN_FLAG_HAS_CRC) header = zip->search(m_crc, filename, false);
			if (header < 0 && (m_openflags & OPEN_FLAG_HAS_CRC)) header = zip->search(m_crc, filename, true);

			// if that failed, look for a file with the right CRC, but the wrong filename
			if (header < 0 && (m_openflags & OPEN_FLAG_HAS_CRC)) header = zip->search(m_crc);

			// if that failed, look for a file with the right name;
			// reporting a bad checksum is more helpful and less confusing than reporting "ROM not found"
			if (header < 0) header = zip->search(filename, false);
			if (header < 0) header = zip->search(filename, true);

			// if we got it, read the data
			if (header >= 0)
			{
				m_zipfile = std::move(zip);
				m_ziplength = m_zipfile->current_uncompressed_length();

				// build a hash with just the CRC
				m_hashes.reset();
				m_hashes.add_crc(m_zipfile->current_crc());
				return (m_openflags & OPEN_FLAG_NO_PRELOAD) ? osd_file::error::NONE : load_zipped_file();
			}

			// close up the archive file and try the next level
			zip.reset();
		}
	}
	return osd_file::error::NOT_FOUND;
}


//-------------------------------------------------
//  load_zipped_file - load a ZIPped file
//-------------------------------------------------

osd_file::error emu_file::load_zipped_file()
{
	assert(m_file == nullptr);
	assert(m_zipdata.empty());
	assert(m_zipfile);

	// allocate some memory
	m_zipdata.resize(m_ziplength);

	// read the data into our buffer and return
	auto const ziperr = m_zipfile->decompress(&m_zipdata[0], m_zipdata.size());
	if (ziperr != util::archive_file::error::NONE)
	{
		m_zipdata.clear();
		return osd_file::error::FAILURE;
	}

	// convert to RAM file
	osd_file::error filerr = util::core_file::open_ram(&m_zipdata[0], m_zipdata.size(), m_openflags, m_file);
	if (filerr != osd_file::error::NONE)
	{
		m_zipdata.clear();
		return osd_file::error::FAILURE;
	}

	// close out the ZIP file
	m_zipfile.reset();
	return osd_file::error::NONE;
}
