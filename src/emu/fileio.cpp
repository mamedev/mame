// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    fileio.c

    File access functions.

***************************************************************************/

#include "emu.h"
#include "unzip.h"
#include "un7z.h"
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
		m__7zfile(nullptr),
		m__7zlength(0),
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
		m__7zfile(nullptr),
		m__7zlength(0),
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
//  operator core_file - automatically convert
//  ourselves to a core_file pointer
//-------------------------------------------------

emu_file::operator core_file *()
{
	// load the ZIP file now if we haven't yet
	if (compressed_file_ready())
		return nullptr;

	// return the core file
	return m_file;
}

emu_file::operator core_file &()
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
	if (!m__7zdata.empty())
	{
		m_hashes.compute(&m__7zdata[0], m__7zdata.size(), needed.c_str());
		return m_hashes;
	}

	if (!m_zipdata.empty())
	{
		m_hashes.compute(&m_zipdata[0], m_zipdata.size(), needed.c_str());
		return m_hashes;
	}

	// read the data if we can
	const UINT8 *filedata = (const UINT8 *)core_fbuffer(m_file);
	if (filedata == nullptr)
		return m_hashes;

	// compute the hash
	m_hashes.compute(filedata, core_fsize(m_file), needed.c_str());
	return m_hashes;
}


//-------------------------------------------------
//  open - open a file by searching paths
//-------------------------------------------------

file_error emu_file::open(const char *name)
{
	// remember the filename and CRC info
	m_filename = name;
	m_crc = 0;
	m_openflags &= ~OPEN_FLAG_HAS_CRC;

	// reset the iterator and open_next
	m_iterator.reset();
	return open_next();
}

file_error emu_file::open(const char *name1, const char *name2)
{
	// concatenate the strings and do a standard open
	std::string name = std::string(name1).append(name2);
	return open(name.c_str());
}

file_error emu_file::open(const char *name1, const char *name2, const char *name3)
{
	// concatenate the strings and do a standard open
	std::string name = std::string(name1).append(name2).append(name3);
	return open(name.c_str());
}

file_error emu_file::open(const char *name1, const char *name2, const char *name3, const char *name4)
{
	// concatenate the strings and do a standard open
	std::string name = std::string(name1).append(name2).append(name3).append(name4);
	return open(name.c_str());
}

file_error emu_file::open(const char *name, UINT32 crc)
{
	// remember the filename and CRC info
	m_filename = name;
	m_crc = crc;
	m_openflags |= OPEN_FLAG_HAS_CRC;

	// reset the iterator and open_next
	m_iterator.reset();
	return open_next();
}

file_error emu_file::open(const char *name1, const char *name2, UINT32 crc)
{
	// concatenate the strings and do a standard open
	std::string name = std::string(name1).append(name2);
	return open(name.c_str(), crc);
}

file_error emu_file::open(const char *name1, const char *name2, const char *name3, UINT32 crc)
{
	// concatenate the strings and do a standard open
	std::string name = std::string(name1).append(name2).append(name3);
	return open(name.c_str(), crc);
}

file_error emu_file::open(const char *name1, const char *name2, const char *name3, const char *name4, UINT32 crc)
{
	// concatenate the strings and do a standard open
	std::string name = std::string(name1).append(name2).append(name3).append(name4);
	return open(name.c_str(), crc);
}


//-------------------------------------------------
//  open_next - open the next file that matches
//  the filename by iterating over paths
//-------------------------------------------------

file_error emu_file::open_next()
{
	// if we're open from a previous attempt, close up now
	if (m_file != nullptr)
		close();

	// loop over paths
	file_error filerr = FILERR_NOT_FOUND;
	while (m_iterator.next(m_fullpath, m_filename.c_str()))
	{
		// attempt to open the file directly
		filerr = core_fopen(m_fullpath.c_str(), m_openflags, &m_file);
		if (filerr == FILERR_NONE)
			break;

		// if we're opening for read-only we have other options
		if ((m_openflags & (OPEN_FLAG_READ | OPEN_FLAG_WRITE)) == OPEN_FLAG_READ)
		{
			std::string tempfullpath = m_fullpath;

			filerr = attempt_zipped();
			if (filerr == FILERR_NONE)
				break;

			m_fullpath = tempfullpath;

			filerr = attempt__7zped();
			if (filerr == FILERR_NONE)
				break;
		}
	}
	return filerr;
}


//-------------------------------------------------
//  open_ram - open a "file" which is actually
//  just an array of data in RAM
//-------------------------------------------------

file_error emu_file::open_ram(const void *data, UINT32 length)
{
	// set a fake filename and CRC
	m_filename = "RAM";
	m_crc = 0;

	// use the core_file's built-in RAM support
	return core_fopen_ram(data, length, m_openflags, &m_file);
}


//-------------------------------------------------
//  close - close a file and free all data; also
//  remove the file if requested
//-------------------------------------------------

void emu_file::close()
{
	// close files and free memory
	if (m__7zfile != nullptr)
		_7z_file_close(m__7zfile);
	m__7zfile = nullptr;

	if (m_zipfile != nullptr)
		zip_file_close(m_zipfile);
	m_zipfile = nullptr;

	if (m_file != nullptr)
		core_fclose(m_file);
	m_file = nullptr;

	m__7zdata.clear();
	m_zipdata.clear();

	if (m_remove_on_close)
		osd_rmfile(m_fullpath.c_str());
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

file_error emu_file::compress(int level)
{
	return core_fcompress(m_file, level);
}


//-------------------------------------------------
//  compressed_file_ready - ensure our zip is ready
//   loading if needed
//-------------------------------------------------

bool emu_file::compressed_file_ready(void)
{
	// load the ZIP file now if we haven't yet
	if (m__7zfile != nullptr && load__7zped_file() != FILERR_NONE)
		return true;

	if (m_zipfile != nullptr && load_zipped_file() != FILERR_NONE)
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
	if (m_file != nullptr)
		return core_fseek(m_file, offset, whence);

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
	if (m_file != nullptr)
		return core_ftell(m_file);

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
	if (m_file != nullptr)
		return core_feof(m_file);

	return 0;
}


//-------------------------------------------------
//  size - returns the size of a file
//-------------------------------------------------

UINT64 emu_file::size()
{
	// use the ZIP length if present
	if (m__7zfile != nullptr)
		return m__7zlength;

	if (m_zipfile != nullptr)
		return m_ziplength;

	// return length if we can
	if (m_file != nullptr)
		return core_fsize(m_file);

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
	if (m_file != nullptr)
		return core_fread(m_file, buffer, length);

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
	if (m_file != nullptr)
		return core_fgetc(m_file);

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
	if (m_file != nullptr)
		return core_ungetc(c, m_file);

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
	if (m_file != nullptr)
		return core_fgets(s, n, m_file);

	return nullptr;
}


//-------------------------------------------------
//  write - write to a file
//-------------------------------------------------

UINT32 emu_file::write(const void *buffer, UINT32 length)
{
	// write the data if we can
	if (m_file != nullptr)
		return core_fwrite(m_file, buffer, length);

	return 0;
}


//-------------------------------------------------
//  puts - write a line to a text file
//-------------------------------------------------

int emu_file::puts(const char *s)
{
	// write the data if we can
	if (m_file != nullptr)
		return core_fputs(m_file, s);

	return 0;
}


//-------------------------------------------------
//  printf - vfprintf to a text file
//-------------------------------------------------

int CLIB_DECL emu_file::printf(const char *fmt, ...)
{
	int rc;
	va_list va;
	va_start(va, fmt);
	rc = vprintf(fmt, va);
	va_end(va);
	return rc;
}


//-------------------------------------------------
//  mame_vfprintf - vfprintf to a text file
//-------------------------------------------------

int emu_file::vprintf(const char *fmt, va_list va)
{
	// write the data if we can
	return (m_file != nullptr) ? core_vfprintf(m_file, fmt, va) : 0;
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

file_error emu_file::attempt_zipped()
{
	std::string filename;

	// loop over directory parts up to the start of filename
	while (1)
	{
		// find the final path separator
		int dirsep = m_fullpath.find_last_of(PATH_SEPARATOR[0]);
		if (dirsep == -1)
			return FILERR_NOT_FOUND;

		if (restrict_to_mediapath())
			if ( !part_of_mediapath(m_fullpath) )
				return FILERR_NOT_FOUND;

		// insert the part from the right of the separator into the head of the filename
		if (filename.length() > 0)
			filename.insert(0, "/");
		filename.insert(0, m_fullpath.substr(dirsep + 1, -1));

		// remove this part of the filename and append a .zip extension
		m_fullpath =  m_fullpath.substr(0, dirsep).append(".zip");

		// attempt to open the ZIP file
		zip_file *zip;
		zip_error ziperr = zip_file_open(m_fullpath.c_str(), &zip);

		// chop the .zip back off the filename before continuing
		m_fullpath = m_fullpath.substr(0, dirsep);

		// if we failed to open this file, continue scanning
		if (ziperr != ZIPERR_NONE)
			continue;

		// see if we can find a file with the right name and (if available) crc
		const zip_file_header *header;
		for (header = zip_file_first_file(zip); header != nullptr; header = zip_file_next_file(zip))
			if (zip_filename_match(*header, filename) && (!(m_openflags & OPEN_FLAG_HAS_CRC) || header->crc == m_crc))
				break;

		// if that failed, look for a file with the right crc, but the wrong filename
		if (header == nullptr && (m_openflags & OPEN_FLAG_HAS_CRC))
			for (header = zip_file_first_file(zip); header != nullptr; header = zip_file_next_file(zip))
				if (header->crc == m_crc && !zip_header_is_path(*header))
					break;

		// if that failed, look for a file with the right name; reporting a bad checksum
		// is more helpful and less confusing than reporting "rom not found"
		if (header == nullptr)
			for (header = zip_file_first_file(zip); header != nullptr; header = zip_file_next_file(zip))
				if (zip_filename_match(*header, filename))
					break;

		// if we got it, read the data
		if (header != nullptr)
		{
			m_zipfile = zip;
			m_ziplength = header->uncompressed_length;

			// build a hash with just the CRC
			m_hashes.reset();
			m_hashes.add_crc(header->crc);
			return (m_openflags & OPEN_FLAG_NO_PRELOAD) ? FILERR_NONE : load_zipped_file();
		}

		// close up the ZIP file and try the next level
		zip_file_close(zip);
	}
}


//-------------------------------------------------
//  load_zipped_file - load a ZIPped file
//-------------------------------------------------

file_error emu_file::load_zipped_file()
{
	assert(m_file == nullptr);
	assert(m_zipdata.empty());
	assert(m_zipfile != nullptr);

	// allocate some memory
	m_zipdata.resize(m_ziplength);

	// read the data into our buffer and return
	zip_error ziperr = zip_file_decompress(m_zipfile, &m_zipdata[0], m_zipdata.size());
	if (ziperr != ZIPERR_NONE)
	{
		m_zipdata.clear();
		return FILERR_FAILURE;
	}

	// convert to RAM file
	file_error filerr = core_fopen_ram(&m_zipdata[0], m_zipdata.size(), m_openflags, &m_file);
	if (filerr != FILERR_NONE)
	{
		m_zipdata.clear();
		return FILERR_FAILURE;
	}

	// close out the ZIP file
	zip_file_close(m_zipfile);
	m_zipfile = nullptr;
	return FILERR_NONE;
}


//-------------------------------------------------
//  zip_filename_match - compare zip filename
//  to expected filename, ignoring any directory
//-------------------------------------------------

bool emu_file::zip_filename_match(const zip_file_header &header, const std::string &filename)
{
	const char *zipfile = header.filename + header.filename_length - filename.length();
	return (zipfile >= header.filename && core_stricmp(filename.c_str(),zipfile) == 0 && (zipfile == header.filename || zipfile[-1] == '/'));
}


//-------------------------------------------------
//  zip_header_is_path - check whether filename
//  in header is a path
//-------------------------------------------------

bool emu_file::zip_header_is_path(const zip_file_header &header)
{
	const char *zipfile = header.filename + header.filename_length - 1;
	return (zipfile >= header.filename && zipfile[0] == '/');
}

//-------------------------------------------------
//  attempt__7zped - attempt to open a .7z file
//-------------------------------------------------

file_error emu_file::attempt__7zped()
{
	std::string filename;

	// loop over directory parts up to the start of filename
	while (1)
	{
		// find the final path separator
		int dirsep = m_fullpath.find_last_of(PATH_SEPARATOR[0]);
		if (dirsep == -1)
			return FILERR_NOT_FOUND;

		if (restrict_to_mediapath())
			if ( !part_of_mediapath(m_fullpath) )
				return FILERR_NOT_FOUND;

		// insert the part from the right of the separator into the head of the filename
		if (filename.length() > 0)
			filename.insert(0, "/");
		filename.insert(0, m_fullpath.substr(dirsep + 1, -1));

		// remove this part of the filename and append a .7z extension
		m_fullpath = m_fullpath.substr(0, dirsep).append(".7z");

		// attempt to open the _7Z file
		_7z_file *_7z;
		_7z_error _7zerr = _7z_file_open(m_fullpath.c_str(), &_7z);

		// chop the ._7z back off the filename before continuing
		m_fullpath = m_fullpath.substr(0, dirsep);

		// if we failed to open this file, continue scanning
		if (_7zerr != _7ZERR_NONE)
			continue;

		int fileno = -1;

		// see if we can find a file with the right name and (if available) crc
		if (m_openflags & OPEN_FLAG_HAS_CRC) fileno = _7z_search_crc_match(_7z, m_crc, filename.c_str(), filename.length(), true, true);

		// if that failed, look for a file with the right crc, but the wrong filename
		if (fileno==-1)
			if (m_openflags & OPEN_FLAG_HAS_CRC) fileno = _7z_search_crc_match(_7z, m_crc, filename.c_str(), filename.length(), true, false);

		// if that failed, look for a file with the right name; reporting a bad checksum
		// is more helpful and less confusing than reporting "rom not found"
		if (fileno==-1)
			fileno = _7z_search_crc_match(_7z, m_crc, filename.c_str(), filename.length(), false, true);

		if (fileno != -1)
		{
			m__7zfile = _7z;
			m__7zlength = _7z->uncompressed_length;

			// build a hash with just the CRC
			m_hashes.reset();
			m_hashes.add_crc(_7z->crc);
			return (m_openflags & OPEN_FLAG_NO_PRELOAD) ? FILERR_NONE : load__7zped_file();
		}

		// close up the _7Z file and try the next level
		_7z_file_close(_7z);
	}
}


//-------------------------------------------------
//  load__7zped_file - load a _7Zped file
//-------------------------------------------------

file_error emu_file::load__7zped_file()
{
	assert(m_file == nullptr);
	assert(m__7zdata.empty());
	assert(m__7zfile != nullptr);

	// allocate some memory
	m__7zdata.resize(m__7zlength);

	// read the data into our buffer and return
	_7z_error _7zerr = _7z_file_decompress(m__7zfile, &m__7zdata[0], m__7zdata.size());
	if (_7zerr != _7ZERR_NONE)
	{
		m__7zdata.clear();
		return FILERR_FAILURE;
	}

	// convert to RAM file
	file_error filerr = core_fopen_ram(&m__7zdata[0], m__7zdata.size(), m_openflags, &m_file);
	if (filerr != FILERR_NONE)
	{
		m__7zdata.clear();
		return FILERR_FAILURE;
	}

	// close out the _7Z file
	_7z_file_close(m__7zfile);
	m__7zfile = nullptr;
	return FILERR_NONE;
}
