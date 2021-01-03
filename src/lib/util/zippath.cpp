// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    zippath.c

    File/directory/path operations that work with ZIP files

***************************************************************************/

#include "zippath.h"
#include "unzip.h"
#include "corestr.h"

#include <cstdlib>

#include <cassert>
#include <cctype>
#include <forward_list>
#include <new>


namespace util {

namespace {

/**
 * @fn  int is_path_separator(char c)
 *
 * @brief   ============================================================
 *            is_path_separator
 *          ============================================================.
 *
 * @param   c   The character.
 *
 * @return  An int.
 */

int is_path_separator(char c)
{
	// FIXME: don't assume DOS-like behaviour - backslashes are valid filename characters on *NIX
	return (c == '/') || (c == '\\');
}

// -------------------------------------------------
//  is_root - tests to see if this path is the root
// -------------------------------------------------

bool is_root(std::string const &path)
{
	// FIXME: get rid of the assumption that paths are DOS-like

	std::string::size_type i = 0;

	// skip drive letter
	if (isalpha(path[i]) && (path[i + 1] == ':'))
		i += 2;

	// skip path separators
	while (is_path_separator(path[i]))
		i++;

	return path[i] == '\0';
}



// -------------------------------------------------
//  is_7z_file - tests to see if this file is a
//  7-zip file
// -------------------------------------------------

bool is_7z_file(std::string const &path)
{
	auto const s = path.rfind('.');
	return (std::string::npos != s) && !core_stricmp(path.c_str() + s, ".7z");
}


// -------------------------------------------------
//  is_zip_file - tests to see if this file is a
//  ZIP file
// -------------------------------------------------

bool is_zip_file(std::string const &path)
{
	auto const s = path.rfind('.');
	return (std::string::npos != s) && !core_stricmp(path.c_str() + s, ".zip");
}



// -------------------------------------------------
//  is_zip_file_separator - returns whether this
//  character is a path separator within a ZIP file
// -------------------------------------------------

bool is_zip_file_separator(char c)
{
	return (c == '/') || (c == '\\');
}



// -------------------------------------------------
//  is_zip_path_separator - returns whether this
//  character is a path separator within a ZIP path
// -------------------------------------------------

bool is_zip_path_separator(char c)
{
	return is_zip_file_separator(c) || is_path_separator(c);
}



// -------------------------------------------------
//  next_path_char - lexes out the next path
//  character, normalizing separators as '/'
// -------------------------------------------------

char next_path_char(std::string const &s, std::string::size_type &pos)
{
	// skip over any initial separators
	if (pos == 0)
	{
		while ((pos < s.length()) && is_zip_file_separator(s[pos]))
			pos++;
	}

	// are we at a path separator?
	if (pos == s.length())
	{
		// return NUL
		return '\0';
	}
	else if (is_zip_file_separator(s[pos]))
	{
		// skip over path separators
		while((pos < s.length()) && is_zip_file_separator(s[pos]))
			pos++;

		// normalize as '/'
		return '/';
	}
	else
	{
		// return character
		return std::tolower(s[pos++]);
	}
}




// -------------------------------------------------
//  zippath_find_sub_path - attempts to identify the
//  type of a sub path in a zip file
// -------------------------------------------------

int zippath_find_sub_path(archive_file &zipfile, std::string const &subpath, osd::directory::entry::entry_type &type)
{
	for (int header = zipfile.first_file(); header >= 0; header = zipfile.next_file())
	{
		std::string::size_type i = 0, j = 0;
		char c1, c2;
		do
		{
			c1 = next_path_char(zipfile.current_name(), i);
			c2 = next_path_char(subpath, j);
		}
		while ((c1 == c2) && c1 && c2);

		if (!c2 || ((c2 == '/') && !(c2 = next_path_char(subpath, j))))
		{
			if (!c1)
			{
				type = zipfile.current_is_directory() ? osd::directory::entry::entry_type::DIR : osd::directory::entry::entry_type::FILE;
				return header;
			}
			else if ((c1 == '/') || (i <= 1U))
			{
				type = osd::directory::entry::entry_type::DIR;
				return header;
			}
		}
	}

	type = osd::directory::entry::entry_type::NONE;
	return -1;
}



// -------------------------------------------------
//  parse_parent_path - parses out the parent path
// -------------------------------------------------

void parse_parent_path(const std::string &path, std::string::size_type *beginpos, std::string::size_type *endpos)
{
	std::string::size_type length = path.length();
	std::string::size_type pos;

	// skip over trailing path separators
	pos = length ? (length - 1) : std::string::npos;
	while ((pos > 0) && (pos != std::string::npos) && is_path_separator(path[pos]))
		pos--;

	// return endpos
	if (endpos != nullptr)
		*endpos = pos;

	// now skip until we find a path separator
	while ((pos != std::string::npos) && !is_path_separator(path[pos]))
		pos = (pos > 0) ? pos - 1 : std::string::npos;

	// return beginpos
	if (beginpos != nullptr)
		*beginpos = pos;
}


// -------------------------------------------------
//  zippath_resolve - separates a ZIP path out into
//  true path and ZIP entry components
// -------------------------------------------------

osd_file::error zippath_resolve(const char *path, osd::directory::entry::entry_type &entry_type, archive_file::ptr &zipfile, std::string &newpath)
{
	newpath.clear();

	// be conservative
	entry_type = osd::directory::entry::entry_type::NONE;
	zipfile.reset();

	std::string apath(path);
	std::string apath_trimmed;
	osd::directory::entry::entry_type current_entry_type;
	bool went_up = false;
	do
	{
		// trim the path of trailing path separators
		auto i = apath.length();
		while ((i > 1) && is_path_separator(apath[i - 1]))
			i--;
		apath.resize(i);
		apath_trimmed = apath;

		// stat the path
		auto current_entry = osd_stat(apath_trimmed);

		// did we find anything?
		if (current_entry)
		{
			// get the entry type and free the stat entry
			current_entry_type = current_entry->type;
		}
		else
		{
			// if we have not found the file or directory, go up
			current_entry_type = osd::directory::entry::entry_type::NONE;
			went_up = true;
			apath = zippath_parent(apath);
		}
	}
	while ((current_entry_type == osd::directory::entry::entry_type::NONE) && !is_root(apath));

	// if we did not find anything, then error out
	if (current_entry_type == osd::directory::entry::entry_type::NONE)
		return osd_file::error::NOT_FOUND;

	// is this file a ZIP file?
	if ((current_entry_type == osd::directory::entry::entry_type::FILE) &&
		((is_zip_file(apath_trimmed) && (archive_file::open_zip(apath_trimmed, zipfile) == archive_file::error::NONE)) ||
			(is_7z_file(apath_trimmed) && (archive_file::open_7z(apath_trimmed, zipfile) == archive_file::error::NONE))))
	{
		auto i = strlen(path + apath.length());
		while ((i > 0) && is_zip_path_separator(path[apath.length() + i - 1]))
			i--;
		newpath.assign(path + apath.length(), i);

		// this was a true ZIP path - attempt to identify the type of path
		zippath_find_sub_path(*zipfile, newpath, current_entry_type);
		if (current_entry_type == osd::directory::entry::entry_type::NONE)
			return osd_file::error::NOT_FOUND;
	}
	else
	{
		// this was a normal path
		if (went_up)
			return osd_file::error::NOT_FOUND;

		newpath = path;
	}

	// success!
	entry_type = current_entry_type;
	return osd_file::error::NONE;
}


// -------------------------------------------------
//  file_error_from_zip_error - translates a
//  osd_file::error to a zip_error
// -------------------------------------------------

osd_file::error file_error_from_zip_error(archive_file::error ziperr)
{
	osd_file::error filerr;
	switch(ziperr)
	{
	case archive_file::error::NONE:
		filerr = osd_file::error::NONE;
		break;
	case archive_file::error::OUT_OF_MEMORY:
		filerr = osd_file::error::OUT_OF_MEMORY;
		break;
	case archive_file::error::BAD_SIGNATURE:
	case archive_file::error::DECOMPRESS_ERROR:
	case archive_file::error::FILE_TRUNCATED:
	case archive_file::error::FILE_CORRUPT:
	case archive_file::error::UNSUPPORTED:
	case archive_file::error::FILE_ERROR:
		filerr = osd_file::error::INVALID_DATA;
		break;
	case archive_file::error::BUFFER_TOO_SMALL:
	default:
		filerr = osd_file::error::FAILURE;
		break;
	}
	return filerr;
}


// -------------------------------------------------
//  create_core_file_from_zip - creates a core_file
//  from a zip file entry
// -------------------------------------------------

osd_file::error create_core_file_from_zip(archive_file &zip, util::core_file::ptr &file)
{
	osd_file::error filerr;
	archive_file::error ziperr;

	void *ptr = malloc(zip.current_uncompressed_length());
	if (!ptr)
	{
		filerr = osd_file::error::OUT_OF_MEMORY;
		goto done;
	}

	ziperr = zip.decompress(ptr, zip.current_uncompressed_length());
	if (ziperr != archive_file::error::NONE)
	{
		filerr = file_error_from_zip_error(ziperr);
		goto done;
	}

	filerr = util::core_file::open_ram_copy(ptr, zip.current_uncompressed_length(), OPEN_FLAG_READ, file);
	if (filerr != osd_file::error::NONE)
		goto done;

done:
	if (ptr != nullptr)
		free(ptr);
	return filerr;
}



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class zippath_directory_base : public zippath_directory
{
protected:
	zippath_directory_base(bool fake_parent) : m_returned_parent(!fake_parent)
	{
	}

	virtual osd::directory::entry const *readdir() override
	{
		if (!m_returned_parent)
		{
			// return a fake entry representing the parent before anything else
			m_returned_parent = true;
			m_synthetic_entry.name = "..";
			m_synthetic_entry.type = osd::directory::entry::entry_type::DIR;
			m_synthetic_entry.size = 0; // FIXME: what would stat say?
			// FIXME: modified time?
			return &m_synthetic_entry;
		}
		else
		{
			return read_excluding_parent();
		}
	}

	osd::directory::entry m_synthetic_entry;

private:
	virtual osd::directory::entry const *read_excluding_parent() = 0;

	bool m_returned_parent;
};


class archive_directory_impl : public zippath_directory_base
{
public:
	archive_directory_impl(archive_file::ptr &&zipfile, std::string &&zipprefix) :
		zippath_directory_base(true),
		m_zipfile(std::move(zipfile)),
		m_zipprefix(std::move(zipprefix))
	{
	}

	virtual bool is_archive() const override { return true; }

private:
	virtual osd::directory::entry const *read_excluding_parent() override
	{
		osd::directory::entry const *result = nullptr;
		char const *relpath = nullptr;
		do
		{
			// a zip file read
			int header;
			do
			{
				header = m_called_zip_first ? m_zipfile->next_file() : m_zipfile->first_file();
				m_called_zip_first = true;
				relpath = nullptr;
			}
			while ((header >= 0) && ((relpath = get_relative_path()) == nullptr));

			if (relpath)
			{
				// we've found a ZIP entry; but this may be an entry deep within the target directory
				char const *separator = relpath;
				while (*separator && !is_zip_file_separator(*separator)) separator++;

				if (*separator || m_zipfile->current_is_directory())
				{
					// a nested entry; loop through returned_dirlist to see if we've returned the parent directory
					auto const len(separator - relpath);
					auto rdent = m_returned_dirlist.begin();
					while (m_returned_dirlist.end() != rdent)
					{
						if ((rdent->length() == len) && !core_strnicmp(rdent->c_str(), relpath, len))
							break;
						else
							++rdent;
					}

					if (m_returned_dirlist.end() == rdent)
					{
						// we've found a new directory; add this to returned_dirlist
						m_returned_dirlist.emplace_front(relpath, separator - relpath);

						// ...and return it
						m_synthetic_entry.name = m_returned_dirlist.front().c_str();
						m_synthetic_entry.type = osd::directory::entry::entry_type::DIR;
						m_synthetic_entry.size = 0; // FIXME: what would stat say?
						// FIXME: modified time?
						result = &m_synthetic_entry;
					}
				}
				else
				{
					// a real file
					m_synthetic_entry.name = relpath;
					m_synthetic_entry.type = osd::directory::entry::entry_type::FILE;
					m_synthetic_entry.size = m_zipfile->current_uncompressed_length();
					m_synthetic_entry.last_modified = m_zipfile->current_last_modified();
					result = &m_synthetic_entry;
				}
			}
		}
		while (relpath && !result);
		return result;
	}


	// -------------------------------------------------
	//  get_relative_path - checks to see if a specified
	//  header is in the zippath_directory, and if so
	//  returns the relative path
	// -------------------------------------------------

	char const *get_relative_path() const
	{
		std::string::size_type len = m_zipprefix.length();
		char const *prefix = m_zipprefix.c_str();
		while (is_zip_file_separator(*prefix))
		{
			len--;
			prefix++;
		}

		std::string const &current(m_zipfile->current_name());
		char const *result = current.c_str() + len;
		if ((current.length() >= len) &&
			!strncmp(prefix, current.c_str(), len) &&
			(!*prefix || is_zip_file_separator(*result) || is_zip_file_separator(m_zipprefix.back())))
		{
			while (is_zip_file_separator(*result))
				result++;

			return *result ? result : nullptr;
		}
		else
		{
			return nullptr;
		}
	}


	bool m_called_zip_first = false;
	archive_file::ptr const m_zipfile;
	std::string const m_zipprefix;
	std::forward_list<std::string> m_returned_dirlist;
};


class filesystem_diectory_impl : public zippath_directory_base
{
public:
	filesystem_diectory_impl(std::string const &path) : zippath_directory_base(!is_root(path)), m_directory(osd::directory::open(path))
	{
	}

	virtual bool is_archive() const override { return false; }

	explicit operator bool() const { return bool(m_directory); }

private:
	virtual osd::directory::entry const *read_excluding_parent() override
	{
		assert(m_directory);

		// a normal directory read
		osd::directory::entry const *result = nullptr;
		do
		{
			result = m_directory->read();
		}
		while (result && (!strcmp(result->name, ".") || !strcmp(result->name, "..")));

		// special case - is this entry a ZIP file?  if so we need to return it as a "directory"
		if (result && (is_zip_file(result->name) || is_7z_file(result->name)))
		{
			// copy; but change the entry type
			m_synthetic_entry = *result;
			m_synthetic_entry.type = osd::directory::entry::entry_type::DIR;
			m_synthetic_entry.size = 0; // FIXME: what would stat say?
			return &m_synthetic_entry;
		}
		else
		{
			return result;
		}
	}

	osd::directory::ptr const m_directory;
};

} // anonymous namespace


// -------------------------------------------------
//  zippath_directory::open - opens a directory
// -------------------------------------------------

osd_file::error zippath_directory::open(std::string const &path, ptr &directory)
{
	try
	{
		// resolve the path
		osd::directory::entry::entry_type entry_type;
		archive_file::ptr zipfile;
		std::string zipprefix;
		osd_file::error const err = zippath_resolve(path.c_str(), entry_type, zipfile, zipprefix);
		if (osd_file::error::NONE != err)
			return err;

		// we have to be a directory
		if (osd::directory::entry::entry_type::DIR != entry_type)
			return osd_file::error::NOT_FOUND;

		// was the result a ZIP?
		if (zipfile)
		{
			directory = std::make_unique<archive_directory_impl>(std::move(zipfile), std::move(zipprefix));
			return osd_file::error::NONE;
		}
		else
		{
			// a conventional directory
			std::unique_ptr<filesystem_diectory_impl> result(new filesystem_diectory_impl(path));
			if (!*result)
				return osd_file::error::FAILURE;

			directory = std::move(result);
			return osd_file::error::NONE;
		}
	}
	catch (std::bad_alloc const &)
	{
		return osd_file::error::OUT_OF_MEMORY;
	}
}


// -------------------------------------------------
//  zippath_directory::~zippath_directory - closes
//  a directory
// -------------------------------------------------

zippath_directory::~zippath_directory()
{
}


// -------------------------------------------------
//  zippath_parent - retrieves the parent directory
// -------------------------------------------------

std::string &zippath_parent(std::string &dst, const std::string &path)
{
	std::string::size_type pos;
	parse_parent_path(path, &pos, nullptr);

	if (pos != std::string::npos)
		dst = path.substr(0, pos + 1);
	else
		dst.clear();
	return dst;
}



// -------------------------------------------------
//  zippath_parent - retrieves the parent directory
// -------------------------------------------------

std::string zippath_parent(const std::string &path)
{
	std::string result;
	zippath_parent(result, path);
	return result;
}



// -------------------------------------------------
//  zippath_parent_basename - retrieves the parent
//  directory basename
// -------------------------------------------------

/**
 * @fn  std::string &zippath_parent_basename(std::string &dst, const std::string &path)
 *
 * @brief   Zippath parent basename.
 *
 * @param [in,out]  dst Destination for the.
 * @param   path        Full pathname of the file.
 *
 * @return  A std::string&amp;
 */

std::string &zippath_parent_basename(std::string &dst, const std::string &path)
{
	std::string::size_type beginpos, endpos;
	parse_parent_path(path, &beginpos, &endpos);
	dst.copy((char*)(path.c_str() + beginpos + 1), endpos - beginpos);
	return dst;
}



// -------------------------------------------------
//  zippath_parent_basename - retrieves the parent
//  directory basename
// -------------------------------------------------

std::string zippath_parent_basename(const std::string &path)
{
	std::string result;
	zippath_parent_basename(result, path);
	return result;
}



// -------------------------------------------------
//  zippath_combine - combines two paths
// -------------------------------------------------

/**
 * @fn  std::string &zippath_combine(std::string &dst, const char *path1, const char *path2)
 *
 * @brief   Zippath combine.
 *
 * @param [in,out]  dst Destination for the.
 * @param   path1       The first path.
 * @param   path2       The second path.
 *
 * @return  A std::string&amp;
 */

std::string &zippath_combine(std::string &dst, const std::string &path1, const std::string &path2)
{
	if (path2 == ".")
	{
		dst.assign(path1);
	}
	else if (path2 == "..")
	{
		dst = zippath_parent(path1);
	}
	else if (osd_is_absolute_path(path2))
	{
		dst.assign(path2);
	}
	else if (!path1.empty() && !is_path_separator(*path1.rbegin()))
	{
		dst.assign(path1).append(PATH_SEPARATOR).append(path2);
	}
	else
	{
		dst.assign(path1).append(path2);
	}
	return dst;
}



// -------------------------------------------------
//  zippath_combine - combines two paths
// -------------------------------------------------

std::string zippath_combine(const std::string &path1, const std::string &path2)
{
	std::string result;
	zippath_combine(result, path1, path2);
	return result;
}



/***************************************************************************
    FILE OPERATIONS
***************************************************************************/

// -------------------------------------------------
//  zippath_fopen - opens a zip path file
// -------------------------------------------------

/**
 * @fn  osd_file::error zippath_fopen(const std::string &filename, uint32_t openflags, util::core_file::ptr &file, std::string &revised_path)
 *
 * @brief   Zippath fopen.
 *
 * @param   filename                Filename of the file.
 * @param   openflags               The openflags.
 * @param [in,out]  file            [in,out] If non-null, the file.
 * @param [in,out]  revised_path    Full pathname of the revised file.
 *
 * @return  A osd_file::error.
 */

osd_file::error zippath_fopen(const std::string &filename, uint32_t openflags, util::core_file::ptr &file, std::string &revised_path)
{
	osd_file::error filerr = osd_file::error::NOT_FOUND;
	archive_file::error ziperr;
	archive_file::ptr zip;
	int header;
	osd::directory::entry::entry_type entry_type;
	int len;

	/* first, set up the two types of paths */
	std::string mainpath(filename);
	std::string subpath;
	file = nullptr;

	/* loop through */
	while((file == nullptr) && (mainpath.length() > 0)
		&& ((openflags == OPEN_FLAG_READ) || (subpath.length() == 0)))
	{
		/* is the mainpath a ZIP path? */
		if (is_zip_file(mainpath) || is_7z_file(mainpath))
		{
			/* this file might be a zip file - lets take a look */
			ziperr = is_zip_file(mainpath) ? archive_file::open_zip(mainpath, zip) : archive_file::open_7z(mainpath, zip);
			if (ziperr == archive_file::error::NONE)
			{
				/* it is a zip file - error if we're not opening for reading */
				if (openflags != OPEN_FLAG_READ)
				{
					filerr = osd_file::error::ACCESS_DENIED;
					goto done;
				}

				if (subpath.length() > 0)
					header = zippath_find_sub_path(*zip, subpath, entry_type);
				else
					header = zip->first_file();

				if (header < 0)
				{
					filerr = osd_file::error::NOT_FOUND;
					goto done;
				}

				/* attempt to read the file */
				filerr = create_core_file_from_zip(*zip, file);
				if (filerr != osd_file::error::NONE)
					goto done;

				/* update subpath, if appropriate */
				if (subpath.length() == 0)
					subpath.assign(zip->current_name());

				/* we're done */
				goto done;
			}
		}

		if (subpath.length() == 0)
			filerr = util::core_file::open(filename, openflags, file);
		else
			filerr = osd_file::error::NOT_FOUND;

		/* if we errored, then go up a directory */
		if (filerr != osd_file::error::NONE)
		{
			/* go up a directory */
			auto temp = zippath_parent(mainpath);

			/* append to the sub path */
			if (subpath.length() > 0)
			{
				std::string temp2;
				mainpath = mainpath.substr(temp.length());
				temp2.assign(mainpath).append(PATH_SEPARATOR).append(subpath);
				subpath.assign(temp2);
			}
			else
			{
				mainpath = mainpath.substr(temp.length());
				subpath.assign(mainpath);
			}
			/* get the new main path, truncating path separators */
			len = temp.length();
			while (len > 0 && is_zip_file_separator(temp[len - 1]))
				len--;
			temp = temp.substr(0, len);
			mainpath.assign(temp);
		}
	}

done:
	/* store the revised path */
	revised_path.clear();
	if (filerr == osd_file::error::NONE)
	{
		/* canonicalize mainpath */
		std::string alloc_fullpath;
		filerr = osd_get_full_path(alloc_fullpath, mainpath);
		if (filerr == osd_file::error::NONE)
		{
			revised_path = alloc_fullpath;
			if (subpath.length() > 0)
				revised_path.append(PATH_SEPARATOR).append(subpath);
		}
	}

	return filerr;
}

} // namespace util
