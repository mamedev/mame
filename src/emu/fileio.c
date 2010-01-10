/***************************************************************************

    fileio.c

    File access functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "hash.h"
#include "unzip.h"
#include "options.h"
#include "fileio.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define PLAIN_FILE				0
#define ZIPPED_FILE				1

#define OPEN_FLAG_HAS_CRC		0x10000

#ifdef MAME_DEBUG
#define DEBUG_COOKIE			0xbaadf00d
#endif



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _path_iterator path_iterator;
struct _path_iterator
{
	const char *	base;
	const char *	cur;
	int				index;
};


/* typedef struct _mame_file mame_file -- declared in fileio.h */
struct _mame_file
{
#ifdef DEBUG_COOKIE
	UINT32			debug_cookie;					/* sanity checking for debugging */
#endif
	astring			filename;						/* full filename */
	core_file *		file;							/* core file pointer */
	path_iterator	iterator;						/* iterator for paths */
	UINT32			openflags;						/* flags we used for the open */
	char			hash[HASH_BUF_SIZE];			/* hash data for the file */
	zip_file *		zipfile;						/* ZIP file pointer */
	UINT8 *			zipdata;						/* ZIP file data */
	UINT64			ziplength;						/* ZIP file length */
};


/* typedef struct _mame_path mame_path -- declared in fileio.h */
struct _mame_path
{
	path_iterator	iterator;
	osd_directory *	curdir;
	astring			pathbuffer;
	int				buflen;
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* core functions */
static void fileio_exit(running_machine *machine);

/* file open/close */
static file_error fopen_internal(core_options *opts, path_iterator *iterator, const char *filename, UINT32 crc, UINT32 flags, mame_file **file);
static file_error fopen_attempt_zipped(astring &fullname, UINT32 crc, UINT32 openflags, mame_file *file);

/* path iteration */
static void path_iterator_init(path_iterator *iterator, core_options *opts, const char *searchpath);
static int path_iterator_get_next(path_iterator *iterator, astring &buffer);

/* misc helpers */
static file_error load_zipped_file(mame_file *file);
static int zip_filename_match(const zip_file_header *header, const astring &afilename);
static int zip_header_is_path(const zip_file_header *header);



/***************************************************************************
    CORE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    fileio_init - initialize the internal file
    I/O system; note that the OS layer is free to
    call mame_fopen before fileio_init
-------------------------------------------------*/

void fileio_init(running_machine *machine)
{
	add_exit_callback(machine, fileio_exit);
}


/*-------------------------------------------------
    fileio_exit - clean up behind ourselves
-------------------------------------------------*/

static void fileio_exit(running_machine *machine)
{
	zip_file_cache_clear();
}



/***************************************************************************
    FILE OPEN/CLOSE
***************************************************************************/

/*-------------------------------------------------
    mame_fopen - open a file for access and
    return an error code
-------------------------------------------------*/

file_error mame_fopen(const char *searchpath, const char *filename, UINT32 openflags, mame_file **file)
{
	path_iterator iterator;
	path_iterator_init(&iterator, mame_options(), searchpath);
	return fopen_internal(mame_options(), &iterator, filename, 0, openflags, file);
}


/*-------------------------------------------------
    mame_fopen_crc - open a file by name or CRC
    and return an error code
-------------------------------------------------*/

file_error mame_fopen_crc(const char *searchpath, const char *filename, UINT32 crc, UINT32 openflags, mame_file **file)
{
	path_iterator iterator;
	path_iterator_init(&iterator, mame_options(), searchpath);
	return mame_fopen_crc_options(mame_options(), searchpath, filename, crc, openflags, file);
}


/*-------------------------------------------------
    mame_fopen_options - open a file for access and
    return an error code
-------------------------------------------------*/

file_error mame_fopen_options(core_options *opts, const char *searchpath, const char *filename, UINT32 openflags, mame_file **file)
{
	path_iterator iterator;
	path_iterator_init(&iterator, opts, searchpath);
	return fopen_internal(opts, &iterator, filename, 0, openflags, file);
}


/*-------------------------------------------------
    mame_fopen_crc_options - open a file by name
    or CRC and return an error code
-------------------------------------------------*/

file_error mame_fopen_crc_options(core_options *opts, const char *searchpath, const char *filename, UINT32 crc, UINT32 openflags, mame_file **file)
{
	path_iterator iterator;
	path_iterator_init(&iterator, opts, searchpath);
	return fopen_internal(opts, &iterator, filename, crc, openflags | OPEN_FLAG_HAS_CRC, file);
}


/*-------------------------------------------------
    mame_fopen_ram - open a "file" which is
    actually just an array of data in RAM
-------------------------------------------------*/

file_error mame_fopen_ram(const void *data, UINT32 length, UINT32 openflags, mame_file **file)
{
	file_error filerr;

	/* allocate the file itself */
	*file = global_alloc_clear(mame_file);

	/* reset the file handle */
	(*file)->openflags = openflags;
#ifdef DEBUG_COOKIE
	(*file)->debug_cookie = DEBUG_COOKIE;
#endif

	/* attempt to open the file directly */
	filerr = core_fopen_ram(data, length, openflags, &(*file)->file);
	if (filerr == FILERR_NONE)
		goto error;

	/* handle errors and return */
error:
	if (filerr != FILERR_NONE)
	{
		mame_fclose(*file);
		*file = NULL;
	}
	return filerr;
}


/*-------------------------------------------------
    fopen_internal - open a file
-------------------------------------------------*/

static file_error fopen_internal(core_options *opts, path_iterator *iterator, const char *filename, UINT32 crc, UINT32 openflags, mame_file **file)
{
	file_error filerr = FILERR_NOT_FOUND;

	/* can either have a hash or open for write, but not both */
	if ((openflags & OPEN_FLAG_HAS_CRC) && (openflags & OPEN_FLAG_WRITE))
		return FILERR_INVALID_ACCESS;

	/* allocate the file itself */
	*file = global_alloc_clear(mame_file);

	/* reset the file handle */
	(*file)->openflags = openflags;
#ifdef DEBUG_COOKIE
	(*file)->debug_cookie = DEBUG_COOKIE;
#endif

	/* loop over paths */
	while (path_iterator_get_next(iterator, (*file)->filename))
	{
		/* compute the full pathname */
		if ((*file)->filename.len() > 0)
			(*file)->filename.cat(PATH_SEPARATOR);
		(*file)->filename.cat(filename);

		/* attempt to open the file directly */
		filerr = core_fopen((*file)->filename, openflags, &(*file)->file);
		if (filerr == FILERR_NONE)
			break;

		/* if we're opening for read-only we have other options */
		if ((openflags & (OPEN_FLAG_READ | OPEN_FLAG_WRITE)) == OPEN_FLAG_READ)
		{
			filerr = fopen_attempt_zipped((*file)->filename, crc, openflags, *file);
			if (filerr == FILERR_NONE)
				break;
		}
	}

	/* if we succeeded, save the iterator */
	if (filerr == FILERR_NONE)
		(*file)->iterator = *iterator;

	/* handle errors and return */
	else
	{
		mame_fclose(*file);
		*file = NULL;
	}
	return filerr;
}


/*-------------------------------------------------
    fopen_attempt_zipped - attempt to open a
    ZIPped file
-------------------------------------------------*/

static file_error fopen_attempt_zipped(astring &fullname, UINT32 crc, UINT32 openflags, mame_file *file)
{
	astring filename;
	zip_error ziperr;
	zip_file *zip;

	/* loop over directory parts up to the start of filename */
	while (1)
	{
		const zip_file_header *header;
		int dirsep;

		/* find the final path separator */
		dirsep = fullname.rchr(0, PATH_SEPARATOR[0]);
		if (dirsep == -1)
			return FILERR_NOT_FOUND;

		/* insert the part from the right of the separator into the head of the filename */
		if (filename.len() > 0)
			filename.ins(0, "/");
		filename.inssubstr(0, fullname, dirsep + 1, -1);

		/* remove this part of the filename and append a .zip extension */
		fullname.substr(0, dirsep).cat(".zip");

		/* attempt to open the ZIP file */
		ziperr = zip_file_open(fullname, &zip);

		/* chop the .zip back off the filename before continuing */
		fullname.substr(0, dirsep);

		/* if we failed to open this file, continue scanning */
		if (ziperr != ZIPERR_NONE)
			continue;

		/* see if we can find a file with the right name and (if available) crc */
		for (header = zip_file_first_file(zip); header != NULL; header = zip_file_next_file(zip))
			if (zip_filename_match(header, filename) && (!(openflags & OPEN_FLAG_HAS_CRC) || header->crc == crc))
				break;

		/* if that failed, look for a file with the right crc, but the wrong filename */
		if (header == NULL && (openflags & OPEN_FLAG_HAS_CRC))
			for (header = zip_file_first_file(zip); header != NULL; header = zip_file_next_file(zip))
				if (header->crc == crc && (!zip_header_is_path(header)))
					break;

		/* if that failed, look for a file with the right name; reporting a bad checksum */
		/* is more helpful and less confusing than reporting "rom not found" */
		if (header == NULL)
			for (header = zip_file_first_file(zip); header != NULL; header = zip_file_next_file(zip))
				if (zip_filename_match(header, filename))
					break;

		/* if we got it, read the data */
		if (header != NULL)
		{
			UINT8 crcs[4];

			file->zipfile = zip;
			file->ziplength = header->uncompressed_length;

			/* build a hash with just the CRC */
			hash_data_clear(file->hash);
			crcs[0] = header->crc >> 24;
			crcs[1] = header->crc >> 16;
			crcs[2] = header->crc >> 8;
			crcs[3] = header->crc >> 0;
			hash_data_insert_binary_checksum(file->hash, HASH_CRC, crcs);

			if (openflags & OPEN_FLAG_NO_PRELOAD)
				return FILERR_NONE;
			else
				return load_zipped_file(file);
		}

		/* close up the ZIP file and try the next level */
		zip_file_close(zip);
	}
}


/*-------------------------------------------------
    mame_fclose - closes a file
-------------------------------------------------*/

void mame_fclose(mame_file *file)
{
#ifdef DEBUG_COOKIE
	assert(file->debug_cookie == DEBUG_COOKIE);
	file->debug_cookie = 0;
#endif

	/* close files and free memory */
	if (file->zipfile != NULL)
		zip_file_close(file->zipfile);
	if (file->file != NULL)
		core_fclose(file->file);
	if (file->zipdata != NULL)
		global_free(file->zipdata);
	global_free(file);
}


/*-------------------------------------------------
    mame_fclose_and_open_next - close an open
    file, and open the next entry in the original
    searchpath
-------------------------------------------------*/

file_error mame_fclose_and_open_next(mame_file **file, const char *filename, UINT32 openflags)
{
	path_iterator iterator = (*file)->iterator;
	mame_fclose(*file);
	*file = NULL;
	return fopen_internal(mame_options(), &iterator, filename, 0, openflags, file);
}


/*-------------------------------------------------
    mame_fcompress - enable/disable streaming file
    compression via zlib; level is 0 to disable
    compression, or up to 9 for max compression
-------------------------------------------------*/

file_error mame_fcompress(mame_file *file, int level)
{
	return core_fcompress(file->file, level);
}



/***************************************************************************
    FILE POSITIONING
***************************************************************************/

/*-------------------------------------------------
    mame_fseek - seek within a file
-------------------------------------------------*/

int mame_fseek(mame_file *file, INT64 offset, int whence)
{
	/* load the ZIP file now if we haven't yet */
	if (file->zipfile != NULL)
	{
		if (load_zipped_file(file) != FILERR_NONE)
			return 1;
	}

	/* seek if we can */
	if (file->file != NULL)
		return core_fseek(file->file, offset, whence);

	return 1;
}


/*-------------------------------------------------
    mame_ftell - return the current file position
-------------------------------------------------*/

UINT64 mame_ftell(mame_file *file)
{
	/* load the ZIP file now if we haven't yet */
	if (file->zipfile != NULL)
	{
		if (load_zipped_file(file) != FILERR_NONE)
			return 0;
	}

	/* tell if we can */
	if (file->file != NULL)
		return core_ftell(file->file);

	return 0;
}


/*-------------------------------------------------
    mame_feof - return 1 if we're at the end
    of file
-------------------------------------------------*/

int mame_feof(mame_file *file)
{
	/* load the ZIP file now if we haven't yet */
	if (file->zipfile != NULL)
	{
		if (load_zipped_file(file) != FILERR_NONE)
			return 0;
	}

	/* return EOF if we can */
	if (file->file != NULL)
		return core_feof(file->file);

	return 0;
}


/*-------------------------------------------------
    mame_fsize - returns the size of a file
-------------------------------------------------*/

UINT64 mame_fsize(mame_file *file)
{
	/* use the ZIP length if present */
	if (file->zipfile != NULL)
		return file->ziplength;

	/* return length if we can */
	if (file->file != NULL)
		return core_fsize(file->file);

	return 0;
}



/***************************************************************************
    FILE READ
***************************************************************************/

/*-------------------------------------------------
    mame_fread - read from a file
-------------------------------------------------*/

UINT32 mame_fread(mame_file *file, void *buffer, UINT32 length)
{
	/* load the ZIP file now if we haven't yet */
	if (file->zipfile != NULL)
	{
		if (load_zipped_file(file) != FILERR_NONE)
			return 0;
	}

	/* read the data if we can */
	if (file->file != NULL)
		return core_fread(file->file, buffer, length);

	return 0;
}


/*-------------------------------------------------
    mame_fgetc - read a character from a file
-------------------------------------------------*/

int mame_fgetc(mame_file *file)
{
	/* load the ZIP file now if we haven't yet */
	if (file->zipfile != NULL)
	{
		if (load_zipped_file(file) != FILERR_NONE)
			return EOF;
	}

	/* read the data if we can */
	if (file->file != NULL)
		return core_fgetc(file->file);

	return EOF;
}


/*-------------------------------------------------
    mame_ungetc - put back a character read from
    a file
-------------------------------------------------*/

int mame_ungetc(int c, mame_file *file)
{
	/* load the ZIP file now if we haven't yet */
	if (file->zipfile != NULL)
	{
		if (load_zipped_file(file) != FILERR_NONE)
			return 1;
	}

	/* read the data if we can */
	if (file->file != NULL)
		return core_ungetc(c, file->file);

	return 1;
}


/*-------------------------------------------------
    mame_fgets - read a line from a text file
-------------------------------------------------*/

char *mame_fgets(char *s, int n, mame_file *file)
{
	/* load the ZIP file now if we haven't yet */
	if (file->zipfile != NULL)
	{
		if (load_zipped_file(file) != FILERR_NONE)
			return NULL;
	}

	/* read the data if we can */
	if (file->file != NULL)
		return core_fgets(s, n, file->file);

	return NULL;
}



/***************************************************************************
    FILE WRITE
***************************************************************************/

/*-------------------------------------------------
    mame_fwrite - write to a file
-------------------------------------------------*/

UINT32 mame_fwrite(mame_file *file, const void *buffer, UINT32 length)
{
	/* write the data if we can */
	if (file->file != NULL)
		return core_fwrite(file->file, buffer, length);

	return 0;
}


/*-------------------------------------------------
    mame_fputs - write a line to a text file
-------------------------------------------------*/

int mame_fputs(mame_file *file, const char *s)
{
	/* write the data if we can */
	if (file->file != NULL)
		return core_fputs(file->file, s);

	return 0;
}


/*-------------------------------------------------
    mame_vfprintf - vfprintf to a text file
-------------------------------------------------*/

static int mame_vfprintf(mame_file *file, const char *fmt, va_list va)
{
	/* write the data if we can */
	return (file->file != NULL) ? core_vfprintf(file->file, fmt, va) : 0;
}


/*-------------------------------------------------
    mame_fprintf - vfprintf to a text file
-------------------------------------------------*/

int CLIB_DECL mame_fprintf(mame_file *file, const char *fmt, ...)
{
	int rc;
	va_list va;
	va_start(va, fmt);
	rc = mame_vfprintf(file, fmt, va);
	va_end(va);
	return rc;
}



/***************************************************************************
    FILE ENUMERATION
***************************************************************************/

/*-------------------------------------------------
    mame_openpath - open a search path (multiple
    directories) for iteration
-------------------------------------------------*/

mame_path *mame_openpath(core_options *opts, const char *searchpath)
{
	mame_path *path;

	/* allocate a new mame_path */
	path = global_alloc_clear(mame_path);

	/* initialize the iterator */
	path_iterator_init(&path->iterator, opts, searchpath);

	return path;
}


/*-------------------------------------------------
    mame_readpath - return information about the
    next file in the search path
-------------------------------------------------*/

const osd_directory_entry *mame_readpath(mame_path *path)
{
	const osd_directory_entry *result;

	/* loop over potentially empty directories */
	while (1)
	{
		/* if no open directory, get the next path */
		while (path->curdir == NULL)
		{
			/* if we fail to get anything more, we're done */
			if (!path_iterator_get_next(&path->iterator, path->pathbuffer))
				return NULL;

			/* open the path */
			path->curdir = osd_opendir(path->pathbuffer);
		}

		/* get the next entry from the current directory */
		result = osd_readdir(path->curdir);
		if (result != NULL)
			return result;

		/* we're done; close this directory */
		osd_closedir(path->curdir);
		path->curdir = NULL;
	}
}


/*-------------------------------------------------
    mame_closepath - close an open seach path
-------------------------------------------------*/

void mame_closepath(mame_path *path)
{
	/* close anything open */
	if (path->curdir != NULL)
		osd_closedir(path->curdir);

	/* free memory */
	global_free(path);
}



/***************************************************************************
    MISCELLANEOUS
***************************************************************************/

/*-------------------------------------------------
    mame_core_file - return the core_file
    underneath the mame_file
-------------------------------------------------*/

core_file *mame_core_file(mame_file *file)
{
	/* load the ZIP file now if we haven't yet */
	if (file->zipfile != NULL)
	{
		if (load_zipped_file(file) != FILERR_NONE)
			return NULL;
	}

	/* return the core file */
	return file->file;
}


/*-------------------------------------------------
    mame_file_full_name - return the full filename
    for a given mame_file
-------------------------------------------------*/

const astring &mame_file_full_name(mame_file *file)
{
	return file->filename;
}


/*-------------------------------------------------
    mame_fhash - returns the hash for a file
-------------------------------------------------*/

const char *mame_fhash(mame_file *file, UINT32 functions)
{
	const UINT8 *filedata;
	UINT32 wehave;

	/* if we already have the functions we need, just return */
	wehave = hash_data_used_functions(file->hash);
	if ((wehave & functions) == functions)
		return file->hash;

	/* load the ZIP file now if we haven't yet */
	if (file->zipfile != NULL)
	{
		if (load_zipped_file(file) != FILERR_NONE)
			return file->hash;
	}
	if (file->file == NULL)
		return file->hash;

	/* read the data if we can */
	filedata = (const UINT8 *)core_fbuffer(file->file);
	if (filedata == NULL)
		return file->hash;

	/* compute the hash */
	hash_compute(file->hash, filedata, core_fsize(file->file), wehave | functions);
	return file->hash;
}



/***************************************************************************
    PATH ITERATION
***************************************************************************/

/*-------------------------------------------------
    path_iterator_init - prepare to iterate over
    a given path type
-------------------------------------------------*/

static void path_iterator_init(path_iterator *iterator, core_options *opts, const char *searchpath)
{
	/* reset the structure */
	iterator->base = (searchpath != NULL && !osd_is_absolute_path(searchpath)) ? options_get_string(opts, searchpath) : "";
	iterator->cur = iterator->base;
	iterator->index = 0;
}


/*-------------------------------------------------
    path_iterator_get_next - get the next entry
    in a multipath sequence
-------------------------------------------------*/

static int path_iterator_get_next(path_iterator *iterator, astring &buffer)
{
	const char *semi;

	/* if none left, return FALSE to indicate we are done */
	if (iterator->index != 0 && *iterator->cur == 0)
		return FALSE;

	/* copy up to the next semicolon */
	semi = strchr(iterator->cur, ';');
	if (semi == NULL)
		semi = iterator->cur + strlen(iterator->cur);
	buffer.cpy(iterator->cur, semi - iterator->cur);
	iterator->cur = (*semi == 0) ? semi : semi + 1;

	/* bump the index and return TRUE */
	iterator->index++;
	return TRUE;
}



/***************************************************************************
    MISC HELPERS
***************************************************************************/

/*-------------------------------------------------
    load_zipped_file - load a ZIPped file
-------------------------------------------------*/

static file_error load_zipped_file(mame_file *file)
{
	file_error filerr;
	zip_error ziperr;

	assert(file->file == NULL);
	assert(file->zipdata == NULL);
	assert(file->zipfile != NULL);

	/* allocate some memory */
	file->zipdata = global_alloc_array(UINT8, file->ziplength);

	/* read the data into our buffer and return */
	ziperr = zip_file_decompress(file->zipfile, file->zipdata, file->ziplength);
	if (ziperr != ZIPERR_NONE)
	{
		global_free(file->zipdata);
		file->zipdata = NULL;
		return FILERR_FAILURE;
	}

	/* convert to RAM file */
	filerr = core_fopen_ram(file->zipdata, file->ziplength, file->openflags, &file->file);
	if (filerr != FILERR_NONE)
	{
		global_free(file->zipdata);
		file->zipdata = NULL;
		return FILERR_FAILURE;
	}

	/* close out the ZIP file */
	zip_file_close(file->zipfile);
	file->zipfile = NULL;
	return FILERR_NONE;
}


/*-------------------------------------------------
    zip_filename_match - compare zip filename
    to expected filename, ignoring any directory
-------------------------------------------------*/

static int zip_filename_match(const zip_file_header *header, const astring &filename)
{
	const char *zipfile = header->filename + header->filename_length - filename.len();

	return (zipfile >= header->filename && filename.icmp(zipfile) == 0 &&
		(zipfile == header->filename || zipfile[-1] == '/'));
}

/*-------------------------------------------------
    zip_header_is_path - check whether filename
                         in header is a path
  -------------------------------------------------*/

static int zip_header_is_path(const zip_file_header *header)
{
	const char *zipfile = header->filename + header->filename_length - 1;

	return (zipfile >= header->filename && zipfile[0] == '/');
}
