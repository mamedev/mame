// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    zippath.c

    File/directory/path operations that work with ZIP files

***************************************************************************/

#include <ctype.h>
#include <stdlib.h>
#include <new>
#include <assert.h>
#include "zippath.h"
#include "unzip.h"
#include "corestr.h"
#include "osdcore.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/**
 * @struct  zippath_returned_directory
 *
 * @brief   A zippath returned directory.
 */

struct zippath_returned_directory
{
	/** @brief  The next. */
	zippath_returned_directory *next;
	/** @brief  The name. */
	std::string name;
};

/**
 * @class   zippath_directory
 *
 * @brief   A zippath directory.
 */

class zippath_directory
{
public:
	zippath_directory()
		: returned_parent(false),
			directory(NULL),
			called_zip_first(false),
			zipfile(NULL),
			returned_dirlist(NULL) { }

	/* common */
	/** @brief  true to returned parent. */
	bool returned_parent;
	/** @brief  The returned entry. */
	osd_directory_entry returned_entry;

	/* specific to normal directories */
	/** @brief  Pathname of the directory. */
	osd_directory *directory;

	/* specific to ZIP directories */
	/** @brief  true to called zip first. */
	bool called_zip_first;
	/** @brief  The zipfile. */
	zip_file *zipfile;
	/** @brief  The zipprefix. */
	std::string zipprefix;
	/** @brief  The returned dirlist. */
	zippath_returned_directory *returned_dirlist;
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static const zip_file_header *zippath_find_sub_path(zip_file *zipfile, const char *subpath, osd_dir_entry_type *type);
static int is_zip_file(const char *path);
static int is_zip_file_separator(char c);
static int is_7z_file(const char *path);


/***************************************************************************
    PATH OPERATIONS
***************************************************************************/

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
	return (c == '/') || (c == '\\');
}

/*-------------------------------------------------
    parse_parent_path - parses out the parent path
-------------------------------------------------*/

/**
 * @fn  static void parse_parent_path(const char *path, int *beginpos, int *endpos)
 *
 * @brief   Parse parent path.
 *
 * @param   path                Full pathname of the file.
 * @param [in,out]  beginpos    If non-null, the beginpos.
 * @param [in,out]  endpos      If non-null, the endpos.
 */

static void parse_parent_path(const char *path, int *beginpos, int *endpos)
{
	int length = strlen(path);
	int pos;

	/* skip over trailing path separators */
	pos = length - 1;
	while((pos > 0) && is_path_separator(path[pos]))
		pos--;

	/* return endpos */
	if (endpos != NULL)
		*endpos = pos;

	/* now skip until we find a path separator */
	while((pos >= 0) && !is_path_separator(path[pos]))
		pos--;

	/* return beginpos */
	if (beginpos != NULL)
		*beginpos = pos;
}



/*-------------------------------------------------
    zippath_parent - retrieves the parent directory
-------------------------------------------------*/

/**
 * @fn  std::string &zippath_parent(std::string &dst, const char *path)
 *
 * @brief   Zippath parent.
 *
 * @param [in,out]  dst Destination for the.
 * @param   path        Full pathname of the file.
 *
 * @return  A std::string&amp;
 */

std::string &zippath_parent(std::string &dst, const char *path)
{
	int pos;
	parse_parent_path(path, &pos, NULL);

	/* return the result */
	if (pos >= 0) {
		dst.assign(path, pos + 1);
	}
	else {
		dst.clear();
	}
	return dst;
}



/*-------------------------------------------------
    zippath_parent_basename - retrieves the parent
    directory basename
-------------------------------------------------*/

/**
 * @fn  std::string &zippath_parent_basename(std::string &dst, const char *path)
 *
 * @brief   Zippath parent basename.
 *
 * @param [in,out]  dst Destination for the.
 * @param   path        Full pathname of the file.
 *
 * @return  A std::string&amp;
 */

std::string &zippath_parent_basename(std::string &dst, const char *path)
{
	int beginpos, endpos;
	parse_parent_path(path, &beginpos, &endpos);
	dst.copy((char*)(path + beginpos + 1), endpos - beginpos);
	return dst;
}



/*-------------------------------------------------
    zippath_combine - combines two paths
-------------------------------------------------*/

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

std::string &zippath_combine(std::string &dst, const char *path1, const char *path2)
{
	if (!strcmp(path2, "."))
	{
		dst.assign(path1);
	}
	else if (!strcmp(path2, ".."))
	{
		zippath_parent(dst, path1);
	}
	else if (osd_is_absolute_path(path2))
	{
		dst.assign(path2);
	}
	else if ((path1[0] != '\0') && !is_path_separator(path1[strlen(path1) - 1]))
	{
		dst.assign(path1).append(PATH_SEPARATOR).append(path2);
	}
	else
	{
		dst.assign(path1).append(path2);
	}
	return dst;
}



/***************************************************************************
    FILE OPERATIONS
***************************************************************************/

/*-------------------------------------------------
    file_error_from_zip_error - translates a
    file_error to a zip_error
-------------------------------------------------*/

/**
 * @fn  static file_error file_error_from_zip_error(zip_error ziperr)
 *
 * @brief   File error from zip error.
 *
 * @param   ziperr  The ziperr.
 *
 * @return  A file_error.
 */

static file_error file_error_from_zip_error(zip_error ziperr)
{
	file_error filerr;
	switch(ziperr)
	{
		case ZIPERR_NONE:
			filerr = FILERR_NONE;
			break;
		case ZIPERR_OUT_OF_MEMORY:
			filerr = FILERR_OUT_OF_MEMORY;
			break;
		case ZIPERR_BAD_SIGNATURE:
		case ZIPERR_DECOMPRESS_ERROR:
		case ZIPERR_FILE_TRUNCATED:
		case ZIPERR_FILE_CORRUPT:
		case ZIPERR_UNSUPPORTED:
		case ZIPERR_FILE_ERROR:
			filerr = FILERR_INVALID_DATA;
			break;
		case ZIPERR_BUFFER_TOO_SMALL:
		default:
			filerr = FILERR_FAILURE;
			break;
	}
	return filerr;
}


/*-------------------------------------------------
    create_core_file_from_zip - creates a core_file
    from a zip file entry
-------------------------------------------------*/

/**
 * @fn  static file_error create_core_file_from_zip(zip_file *zip, const zip_file_header *header, core_file *&file)
 *
 * @brief   Creates core file from zip.
 *
 * @param [in,out]  zip     If non-null, the zip.
 * @param   header          The header.
 * @param [in,out]  file    [in,out] If non-null, the file.
 *
 * @return  The new core file from zip.
 */

static file_error create_core_file_from_zip(zip_file *zip, const zip_file_header *header, core_file *&file)
{
	file_error filerr;
	zip_error ziperr;
	void *ptr;

	ptr = malloc(header->uncompressed_length);
	if (ptr == NULL)
	{
		filerr = FILERR_OUT_OF_MEMORY;
		goto done;
	}

	ziperr = zip_file_decompress(zip, ptr, header->uncompressed_length);
	if (ziperr != ZIPERR_NONE)
	{
		filerr = file_error_from_zip_error(ziperr);
		goto done;
	}

	filerr = core_fopen_ram_copy(ptr, header->uncompressed_length, OPEN_FLAG_READ, &file);
	if (filerr != FILERR_NONE)
		goto done;

done:
	if (ptr != NULL)
		free(ptr);
	return filerr;
}


/*-------------------------------------------------
    zippath_fopen - opens a zip path file
-------------------------------------------------*/

/**
 * @fn  file_error zippath_fopen(const char *filename, UINT32 openflags, core_file *&file, std::string &revised_path)
 *
 * @brief   Zippath fopen.
 *
 * @param   filename                Filename of the file.
 * @param   openflags               The openflags.
 * @param [in,out]  file            [in,out] If non-null, the file.
 * @param [in,out]  revised_path    Full pathname of the revised file.
 *
 * @return  A file_error.
 */

file_error zippath_fopen(const char *filename, UINT32 openflags, core_file *&file, std::string &revised_path)
{
	file_error filerr = FILERR_NOT_FOUND;
	zip_error ziperr;
	zip_file *zip = NULL;
	const zip_file_header *header;
	osd_dir_entry_type entry_type;
	char *alloc_fullpath = NULL;
	int len;

	/* first, set up the two types of paths */
	std::string mainpath(filename);
	std::string subpath;
	file = NULL;

	/* loop through */
	while((file == NULL) && (mainpath.length() > 0)
		&& ((openflags == OPEN_FLAG_READ) || (subpath.length() == 0)))
	{
		/* is the mainpath a ZIP path? */
		if (is_zip_file(mainpath.c_str()))
		{
			/* this file might be a zip file - lets take a look */
			ziperr = zip_file_open(mainpath.c_str(), &zip);
			if (ziperr == ZIPERR_NONE)
			{
				/* it is a zip file - error if we're not opening for reading */
				if (openflags != OPEN_FLAG_READ)
				{
					filerr = FILERR_ACCESS_DENIED;
					goto done;
				}

				if (subpath.length() > 0)
					header = zippath_find_sub_path(zip, subpath.c_str(), &entry_type);
				else
					header = zip_file_first_file(zip);

				if (header == NULL)
				{
					filerr = FILERR_NOT_FOUND;
					goto done;
				}

				/* attempt to read the file */
				filerr = create_core_file_from_zip(zip, header, file);
				if (filerr != FILERR_NONE)
					goto done;

				/* update subpath, if appropriate */
				if (subpath.length() == 0)
					subpath.assign(header->filename);

				/* we're done */
				goto done;
			}
		}
		else if (is_7z_file(mainpath.c_str()))
		{
			filerr = FILERR_INVALID_DATA;
			goto done;
		}

		if (subpath.length() == 0)
			filerr = core_fopen(filename, openflags, &file);
		else
			filerr = FILERR_NOT_FOUND;

		/* if we errored, then go up a directory */
		if (filerr != FILERR_NONE)
		{
			/* go up a directory */
			std::string temp;
			zippath_parent(temp, mainpath.c_str());

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
	if (filerr == FILERR_NONE)
	{
		/* cannonicalize mainpath */
		filerr = osd_get_full_path(&alloc_fullpath, mainpath.c_str());
		if (filerr == FILERR_NONE)
		{
			if (subpath.length() > 0)
				revised_path.assign(alloc_fullpath).append(PATH_SEPARATOR).append(subpath);
			else
				revised_path.assign(alloc_fullpath);
		}
	}

	if (zip != NULL)
		zip_file_close(zip);
	if (alloc_fullpath != NULL)
		osd_free(alloc_fullpath);
	return filerr;
}


/***************************************************************************
    DIRECTORY OPERATIONS
***************************************************************************/

/*-------------------------------------------------
    is_root - tests to see if this path is the root
-------------------------------------------------*/

/**
 * @fn  static int is_root(const char *path)
 *
 * @brief   Is root.
 *
 * @param   path    Full pathname of the file.
 *
 * @return  An int.
 */

static int is_root(const char *path)
{
	int i = 0;

	/* skip drive letter */
	if (isalpha(path[i]) && (path[i + 1] == ':'))
		i += 2;

	/* skip path separators */
	while (is_path_separator(path[i]))
		i++;

	return path[i] == '\0';
}



/*-------------------------------------------------
    is_7z_file - tests to see if this file is a
    7-zip file
-------------------------------------------------*/

/**
 * @fn  static int is_7z_file(const char *path)
 *
 * @brief   Is 7z file.
 *
 * @param   path    Full pathname of the file.
 *
 * @return  An int.
 */

static int is_7z_file(const char *path)
{
	const char *s = strrchr(path, '.');
	return (s != NULL) && !core_stricmp(s, ".7z");
}


/*-------------------------------------------------
    is_zip_file - tests to see if this file is a
    ZIP file
-------------------------------------------------*/

/**
 * @fn  static int is_zip_file(const char *path)
 *
 * @brief   Is zip file.
 *
 * @param   path    Full pathname of the file.
 *
 * @return  An int.
 */

static int is_zip_file(const char *path)
{
	const char *s = strrchr(path, '.');
	return (s != NULL) && !core_stricmp(s, ".zip");
}



/*-------------------------------------------------
    is_zip_file_separator - returns whether this
    character is a path separator within a ZIP file
-------------------------------------------------*/

/**
 * @fn  static int is_zip_file_separator(char c)
 *
 * @brief   Is zip file separator.
 *
 * @param   c   The character.
 *
 * @return  An int.
 */

static int is_zip_file_separator(char c)
{
	return (c == '/') || (c == '\\');
}



/*-------------------------------------------------
    is_zip_path_separator - returns whether this
    character is a path separator within a ZIP path
-------------------------------------------------*/

/**
 * @fn  static int is_zip_path_separator(char c)
 *
 * @brief   Is zip path separator.
 *
 * @param   c   The character.
 *
 * @return  An int.
 */

static int is_zip_path_separator(char c)
{
	return is_zip_file_separator(c) || is_path_separator(c);
}



/*-------------------------------------------------
    next_path_char - lexes out the next path
    character, normalizing separators as '/'
-------------------------------------------------*/

/**
 * @fn  static char next_path_char(const char *s, int *pos)
 *
 * @brief   Next path character.
 *
 * @param   s           The const char * to process.
 * @param [in,out]  pos If non-null, the position.
 *
 * @return  A char.
 */

static char next_path_char(const char *s, int *pos)
{
	char result;

	/* skip over any initial separators */
	if (*pos == 0)
	{
		while(is_zip_file_separator(s[*pos]))
			(*pos)++;
	}

	/* are we at a path separator? */
	if (is_zip_file_separator(s[*pos]))
	{
		/* skip over path separators */
		while(is_zip_file_separator(s[*pos]))
			(*pos)++;

		/* normalize as '/' */
		result = '/';
	}
	else if (s[*pos] != '\0')
	{
		/* return character */
		result = tolower(s[(*pos)++]);
	}
	else
	{
		/* return NUL */
		result = '\0';
	}
	return result;
}




/*-------------------------------------------------
    zippath_find_sub_path - attempts to identify the
    type of a sub path in a zip file
-------------------------------------------------*/

/**
 * @fn  static const zip_file_header *zippath_find_sub_path(zip_file *zipfile, const char *subpath, osd_dir_entry_type *type)
 *
 * @brief   Zippath find sub path.
 *
 * @param [in,out]  zipfile If non-null, the zipfile.
 * @param   subpath         The subpath.
 * @param [in,out]  type    If non-null, the type.
 *
 * @return  null if it fails, else a zip_file_header*.
 */

static const zip_file_header *zippath_find_sub_path(zip_file *zipfile, const char *subpath, osd_dir_entry_type *type)
{
	int i, j;
	char c1, c2, last_char;
	const zip_file_header *header;

	for (header = zip_file_first_file(zipfile); header != NULL; header = zip_file_next_file(zipfile))
	{
		/* special case */
		if (subpath == NULL)
		{
			if (type != NULL)
				*type = ENTTYPE_FILE;
			return header;
		}

		i = 0;
		j = 0;
		last_char = '/';
				while(((c1 = next_path_char(header->filename, &i)) == (c2 = next_path_char(subpath, &j))) &&
						( c1 != '\0' && c2 != '\0' ))
						last_char = c2;


		if (c2 == '\0')
		{
			if (c1 == '\0')
			{
				if (type != NULL)
					*type = ENTTYPE_FILE;
				return header;
			}
			else if ((last_char == '/') || (c1 == '/'))
			{
				if (type != NULL)
					*type = ENTTYPE_DIR;
				return header;
			}
		}
	}

	if (type != NULL)
		*type = ENTTYPE_NONE;
	return NULL;
}



/*-------------------------------------------------
    zippath_resolve - separates a ZIP path out into
    true path and ZIP entry components
-------------------------------------------------*/

/**
 * @fn  static file_error zippath_resolve(const char *path, osd_dir_entry_type &entry_type, zip_file *&zipfile, std::string &newpath)
 *
 * @brief   Zippath resolve.
 *
 * @param   path                Full pathname of the file.
 * @param [in,out]  entry_type  Type of the entry.
 * @param [in,out]  zipfile     [in,out] If non-null, the zipfile.
 * @param [in,out]  newpath     The newpath.
 *
 * @return  A file_error.
 */

static file_error zippath_resolve(const char *path, osd_dir_entry_type &entry_type, zip_file *&zipfile, std::string &newpath)
{
	file_error err;
	osd_directory_entry *current_entry = NULL;
	osd_dir_entry_type current_entry_type;
	int went_up = FALSE;
	int i;

	newpath.clear();

	/* be conservative */
	entry_type = ENTTYPE_NONE;
	zipfile = NULL;

	std::string apath(path);
	std::string apath_trimmed;
	do
	{
		/* trim the path of trailing path separators */
		i = apath.length();
		while (i > 1 && is_path_separator(apath[i - 1]))
			i--;
		apath = apath.substr(0, i);
		apath_trimmed.assign(apath);

		/* stat the path */
		current_entry = osd_stat(apath_trimmed.c_str());

		/* did we find anything? */
		if (current_entry != NULL)
		{
			/* get the entry type and free the stat entry */
			current_entry_type = current_entry->type;
			osd_free(current_entry);
			current_entry = NULL;
		}
		else
		{
			/* if we have not found the file or directory, go up */
			current_entry_type = ENTTYPE_NONE;
			went_up = TRUE;
			std::string parent;
			apath.assign(zippath_parent(parent, apath.c_str()));
		}
	}
	while (current_entry_type == ENTTYPE_NONE && !is_root(apath.c_str()));

	/* if we did not find anything, then error out */
	if (current_entry_type == ENTTYPE_NONE)
	{
		err = FILERR_NOT_FOUND;
		goto done;
	}

	/* is this file a ZIP file? */
	if ((current_entry_type == ENTTYPE_FILE) && is_zip_file(apath_trimmed.c_str())
		&& (zip_file_open(apath_trimmed.c_str(), &zipfile) == ZIPERR_NONE))
	{
		i = strlen(path + apath.length());
		while (i > 0 && is_zip_path_separator(path[apath.length() + i - 1]))
			i--;
		newpath.assign(path + apath.length(), i);

		/* this was a true ZIP path - attempt to identify the type of path */
		zippath_find_sub_path(zipfile, newpath.c_str(), &current_entry_type);
		if (current_entry_type == ENTTYPE_NONE)
		{
			err = FILERR_NOT_FOUND;
			goto done;
		}
	}
	else
	{
		/* this was a normal path */
		if (went_up)
		{
			err = FILERR_NOT_FOUND;
			goto done;
		}
		newpath.assign(path);
	}

	/* success! */
	entry_type = current_entry_type;
	err = FILERR_NONE;

done:
	return err;
}


/*-------------------------------------------------
    zippath_opendir - opens a directory
-------------------------------------------------*/

/**
 * @fn  file_error zippath_opendir(const char *path, zippath_directory **directory)
 *
 * @brief   Zippath opendir.
 *
 * @param   path                Full pathname of the file.
 * @param [in,out]  directory   If non-null, pathname of the directory.
 *
 * @return  A file_error.
 */

file_error zippath_opendir(const char *path, zippath_directory **directory)
{
	file_error err;

	/* allocate a directory */
	zippath_directory *result = NULL;
	try
	{
		result = new zippath_directory;
	}
	catch (std::bad_alloc &)
	{
		err = FILERR_OUT_OF_MEMORY;
		goto done;
	}
	/* resolve the path */
	osd_dir_entry_type entry_type;
	err = zippath_resolve(path, entry_type, result->zipfile, result->zipprefix);
	if (err != FILERR_NONE)
		goto done;

	/* we have to be a directory */
	if (entry_type != ENTTYPE_DIR)
	{
		err = FILERR_NOT_FOUND;
		goto done;
	}

	/* was the result a ZIP? */
	if (result->zipfile == NULL)
	{
		/* a conventional directory */
		result->directory = osd_opendir(path);
		if (result->directory == NULL)
		{
			err = FILERR_FAILURE;
			goto done;
		}

		/* is this path the root? if so, pretend we've already returned the parent */
		if (is_root(path))
			result->returned_parent = true;
	}

done:
	if ((directory == NULL || err != FILERR_NONE) && result != NULL)
	{
		zippath_closedir(result);
		result = NULL;
	}
	if (directory != NULL)
		*directory = result;
	return err;
}


/*-------------------------------------------------
    zippath_closedir - closes a directory
-------------------------------------------------*/

/**
 * @fn  void zippath_closedir(zippath_directory *directory)
 *
 * @brief   Zippath closedir.
 *
 * @param [in,out]  directory   If non-null, pathname of the directory.
 */

void zippath_closedir(zippath_directory *directory)
{
	if (directory->directory != NULL)
		osd_closedir(directory->directory);

	if (directory->zipfile != NULL)
		zip_file_close(directory->zipfile);

	while (directory->returned_dirlist != NULL)
	{
		zippath_returned_directory *dirlist = directory->returned_dirlist;
		directory->returned_dirlist = directory->returned_dirlist->next;
		delete dirlist;
	}

	delete directory;
}


/*-------------------------------------------------
    get_relative_path - checks to see if a specified
    header is in the zippath_directory, and if so
    returns the relative path
-------------------------------------------------*/

/**
 * @fn  static const char *get_relative_path(zippath_directory *directory, const zip_file_header *header)
 *
 * @brief   Gets relative path.
 *
 * @param [in,out]  directory   If non-null, pathname of the directory.
 * @param   header              The header.
 *
 * @return  null if it fails, else the relative path.
 */

static const char *get_relative_path(zippath_directory *directory, const zip_file_header *header)
{
	const char *result = NULL;
	int len = directory->zipprefix.length();

	if ((len <= strlen(header->filename))
		&& !strncmp(directory->zipprefix.c_str(), header->filename, len))
	{
		result = &header->filename[len];
		while(is_zip_file_separator(*result))
			result++;
	}

	return result;
}


/*-------------------------------------------------
    zippath_readdir - reads a directory
-------------------------------------------------*/

/**
 * @fn  const osd_directory_entry *zippath_readdir(zippath_directory *directory)
 *
 * @brief   Zippath readdir.
 *
 * @param [in,out]  directory   If non-null, pathname of the directory.
 *
 * @return  null if it fails, else an osd_directory_entry*.
 */

const osd_directory_entry *zippath_readdir(zippath_directory *directory)
{
	const osd_directory_entry *result = NULL;
	const zip_file_header *header;
	const char *relpath;
	const char *separator;
	const char *s;
	zippath_returned_directory *rdent;

	if (!directory->returned_parent)
	{
		/* first thing's first - return parent directory */
		directory->returned_parent = true;
		memset(&directory->returned_entry, 0, sizeof(directory->returned_entry));
		directory->returned_entry.name = "..";
		directory->returned_entry.type = ENTTYPE_DIR;
		result = &directory->returned_entry;
	}
	else if (directory->directory != NULL)
	{
		/* a normal directory read */
		do
		{
			result = osd_readdir(directory->directory);
		}
		while((result != NULL) && (!strcmp(result->name, ".") || !strcmp(result->name, "..")));

		/* special case - is this entry a ZIP file?  if so we need to return it as a "directory" */
		if ((result != NULL) && is_zip_file(result->name))
		{
			/* copy; but change the entry type */
			directory->returned_entry = *result;
			directory->returned_entry.type = ENTTYPE_DIR;
			result = &directory->returned_entry;
		}
	}
	else if (directory->zipfile != NULL)
	{
		do
		{
			/* a zip file read */
			do
			{
				if (!directory->called_zip_first)
					header = zip_file_first_file(directory->zipfile);
				else
					header = zip_file_next_file(directory->zipfile);
				directory->called_zip_first = true;
				relpath = NULL;
			}
			while((header != NULL) && ((relpath = get_relative_path(directory, header)) == NULL));

			if (relpath != NULL)
			{
				/* we've found a ZIP entry; but this may be an entry deep within the target directory */
				for (s = relpath; *s && !is_zip_file_separator(*s); s++)
					;
				separator = *s ? s : NULL;

				if (separator != NULL)
				{
					/* a nested entry; loop through returned_dirlist to see if we've returned the parent directory */
					for (rdent = directory->returned_dirlist; rdent != NULL; rdent = rdent->next)
					{
						if (!core_strnicmp(rdent->name.c_str(), relpath, separator - relpath))
							break;
					}

					if (rdent == NULL)
					{
						/* we've found a new directory; add this to returned_dirlist */
						rdent = new zippath_returned_directory;
						rdent->next = directory->returned_dirlist;
						rdent->name.assign(relpath, separator - relpath);
						directory->returned_dirlist = rdent;

						/* ...and return it */
						memset(&directory->returned_entry, 0, sizeof(directory->returned_entry));
						directory->returned_entry.name = rdent->name.c_str();
						directory->returned_entry.type = ENTTYPE_DIR;
						result = &directory->returned_entry;
					}
				}
				else
				{
					/* a real file */
					memset(&directory->returned_entry, 0, sizeof(directory->returned_entry));
					directory->returned_entry.name = relpath;
					directory->returned_entry.type = ENTTYPE_FILE;
					directory->returned_entry.size = header->uncompressed_length;
					result = &directory->returned_entry;
				}
			}
		}
		while((relpath != NULL) && (result == NULL));
	}
	return result;
}



/*-------------------------------------------------
    zippath_is_zip - returns TRUE if this path is
    a ZIP path or FALSE if not
-------------------------------------------------*/

/**
 * @fn  int zippath_is_zip(zippath_directory *directory)
 *
 * @brief   Zippath is zip.
 *
 * @param [in,out]  directory   If non-null, pathname of the directory.
 *
 * @return  An int.
 */

int zippath_is_zip(zippath_directory *directory)
{
	return directory->zipfile != NULL;
}
