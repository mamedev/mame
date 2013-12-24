// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    MAME source code dependency generator

***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <zlib.h>
#include "osdcore.h"
#include "astring.h"
#include "corefile.h"
#include "tagmap.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define HASH_SIZE       193


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct include_path
{
	include_path *  next;
	astring         path;
};


struct exclude_path
{
	exclude_path *  next;
	astring         path;
	int             pathlen;
	UINT8           recursive;
};


struct list_entry
{
	list_entry *    next;
	astring         name;
};


struct file_entry;

struct dependency
{
	dependency *    next;
	file_entry *    file;
};


struct file_entry
{
	astring         name;
	dependency *    deplist;
};

typedef tagmap_t<UINT8> dependency_map;



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static include_path *incpaths;
static exclude_path *excpaths;
static tagmap_t<file_entry *> file_map;



/***************************************************************************
    PROTOTYPES
***************************************************************************/

// core output functions
static int recurse_dir(int srcrootlen, astring &srcdir);
static file_entry &compute_dependencies(int srcrootlen, astring &srcfile);

// path helpers
static bool find_include_file(astring &srcincpath, int srcrootlen, const astring &srcfile, const astring &filename);



/***************************************************************************
    MAIN
***************************************************************************/

/*-------------------------------------------------
    main - main entry point
-------------------------------------------------*/

void usage(const char *argv0)
{
	fprintf(stderr, "Usage:\n%s <srcroot> [-Iincpath [-Iincpath [...]]]\n", argv0);
	exit(1);
}

int main(int argc, char *argv[])
{
	include_path **incpathhead = &incpaths;
	exclude_path **excpathhead = &excpaths;
	astring srcdir;
	int unadorned = 0;

	// loop over arguments
	for (int argnum = 1; argnum < argc; argnum++)
	{
		char *arg = argv[argnum];

		// include path?
		if (arg[0] == '-' && arg[1] == 'I')
		{
			*incpathhead = new include_path;
			(*incpathhead)->next = NULL;
			(*incpathhead)->path.cpy(&arg[2]).replacechr('/', PATH_SEPARATOR[0]);
			incpathhead = &(*incpathhead)->next;
		}

		// exclude path?
		else if (arg[0] == '-' && arg[1] == 'X')
		{
			*excpathhead = new exclude_path;
			(*excpathhead)->next = NULL;
			(*excpathhead)->path.cpy(&arg[2]).replacechr(PATH_SEPARATOR[0], '/');
			(*excpathhead)->recursive = ((*excpathhead)->path.replace((*excpathhead)->path.len() - 4, "/...", "") != 0);
			(*excpathhead)->pathlen = (*excpathhead)->path.len();
			excpathhead = &(*excpathhead)->next;
		}

		else if (arg[0] == '-' && arg[1] == 'F')
		{
			argnum++;
		}

		else if (arg[0] == '-' && arg[1] == 'D')
		{
			// some pkgconfigs return defines (e.g. pkg-config QtGui --cflags) ==> ignore
			argnum++;
		}

		// ignore -include which is used by sdlmame to include sdlprefix.h before all other includes
		else if (strcmp(arg,"-include") == 0)
		{
			argnum++;
		}

		// other parameter
		else if (arg[0] != '-' && unadorned == 0)
		{
			srcdir.cpy(arg).replacechr('/', PATH_SEPARATOR[0]);
			unadorned++;
		}
		else
			usage(argv[0]);
	}

	// make sure we got 1 parameter
	if (srcdir.len() == 0)
		usage(argv[0]);

	// recurse over subdirectories
	return recurse_dir(srcdir.len(), srcdir);
}



/***************************************************************************
    CORE OUTPUT FUNCTIONS
***************************************************************************/

static int compare_list_entries(const void *p1, const void *p2)
{
	const list_entry *entry1 = *(const list_entry **)p1;
	const list_entry *entry2 = *(const list_entry **)p2;
	return entry1->name.cmp(entry2->name);
}


/*-------------------------------------------------
    recurse_dependencies - recurse through the
    dependencies found, adding the mto the tagmap
    unless we already exist in the map
-------------------------------------------------*/

static void recurse_dependencies(file_entry &file, dependency_map &map)
{
	// skip if we're in an exclude path
	int filelen = file.name.len();
	for (exclude_path *exclude = excpaths; exclude != NULL; exclude = exclude->next)
		if (exclude->pathlen < filelen && strncmp(file.name, exclude->path, exclude->pathlen) == 0)
			if (exclude->recursive || file.name.chr(exclude->pathlen + 1, '/') == -1)
				return;

	// attempt to add; if we get an error, we're already present
	if (map.add(file.name, 0) != TMERR_NONE)
		return;

	// recurse the list from there
	for (dependency *dep = file.deplist; dep != NULL; dep = dep->next)
		recurse_dependencies(*dep->file, map);
}


/*-------------------------------------------------
    recurse_dir - recurse through a directory
-------------------------------------------------*/

static int recurse_dir(int srcrootlen, astring &srcdir)
{
	static const osd_dir_entry_type typelist[] = { ENTTYPE_DIR, ENTTYPE_FILE };
	int result = 0;

	// iterate first over directories, then over files
	for (int entindex = 0; entindex < ARRAY_LENGTH(typelist) && result == 0; entindex++)
	{
		osd_dir_entry_type entry_type = typelist[entindex];

		// open the directory and iterate through it
		osd_directory *dir = osd_opendir(srcdir);
		if (dir == NULL)
		{
			result = 1;
			goto error;
		}

		// build up the list of files
		const osd_directory_entry *entry;
		list_entry *list = NULL;
		int found = 0;
		while ((entry = osd_readdir(dir)) != NULL)
			if (entry->type == entry_type && entry->name[0] != '.')
			{
				list_entry *lentry = new list_entry;
				lentry->name.cpy(entry->name);
				lentry->next = list;
				list = lentry;
				found++;
			}

		// close the directory
		osd_closedir(dir);

		// skip if nothing found
		if (found == 0)
			continue;

		// allocate memory for sorting
		list_entry **listarray = new list_entry *[found];
		found = 0;
		for (list_entry *curlist = list; curlist != NULL; curlist = curlist->next)
			listarray[found++] = curlist;

		// sort the list
		qsort(listarray, found, sizeof(listarray[0]), compare_list_entries);

		// rebuild the list
		list = NULL;
		while (--found >= 0)
		{
			listarray[found]->next = list;
			list = listarray[found];
		}
		delete[] listarray;

		// iterate through each file
		for (list_entry *curlist = list; curlist != NULL && result == 0; curlist = curlist->next)
		{
			astring srcfile;

			// build the source filename
			srcfile.printf("%s%c%s", srcdir.cstr(), PATH_SEPARATOR[0], curlist->name.cstr());

			// if we have a file, output it
			if (entry_type == ENTTYPE_FILE)
			{
				// make sure we care, first
				if (core_filename_ends_with(curlist->name, ".c"))
				{
					dependency_map depend_map;

					// find dependencies
					file_entry &file = compute_dependencies(srcrootlen, srcfile);
					recurse_dependencies(file, depend_map);

					// convert the target from source to object (makes assumptions about rules)
					astring target(file.name);
					target.replace(0, "src/", "$(OBJ)/");
					target.replace(0, ".c", ".o");
					printf("\n%s : \\\n", target.cstr());

					// iterate over the hashed dependencies and output them as well
					for (dependency_map::entry_t *entry = depend_map.first(); entry != NULL; entry = depend_map.next(entry))
						printf("\t%s \\\n", entry->tag().cstr());
				}
			}

			// if we have a directory, recurse
			else
				result = recurse_dir(srcrootlen, srcfile);
		}

		// free all the allocated entries
		while (list != NULL)
		{
			list_entry *next = list->next;
			delete list;
			list = next;
		}
	}

error:
	return result;
}


/*-------------------------------------------------
    output_file - output a file, converting to
    HTML
-------------------------------------------------*/

static file_entry &compute_dependencies(int srcrootlen, astring &srcfile)
{
	// see if we already have an entry
	astring normalfile(srcfile);
	normalfile.replacechr(PATH_SEPARATOR[0], '/');
	file_entry *foundfile = file_map.find(normalfile);
	if (foundfile != NULL)
		return *foundfile;

	// create a new header entry
	file_entry &file = *new file_entry;
	file.deplist = NULL;
	file.name = normalfile;
	file_map.add(file.name, &file);

	// read the source file
	UINT32 filelength;
	char *filedata;
	if (core_fload(srcfile, (void **)&filedata, &filelength) != FILERR_NONE)
	{
		fprintf(stderr, "Unable to read file '%s'\n", srcfile.cstr());
		return file;
	}

	// find the #include directives in this file
	for (int index = 0; index < filelength; index++)
		if (filedata[index] == '#' && strncmp(&filedata[index + 1], "include", 7) == 0)
		{
			// first make sure we're not commented or quoted
			bool just_continue = false;
			for (int scan = index; scan > 2 && filedata[scan] != 13 && filedata[scan] != 10; scan--)
				if ((filedata[scan] == '/' && filedata[scan - 1] == '/') || filedata[scan] == '"')
				{
					just_continue = true;
					break;
				}
			if (just_continue)
				continue;

			// scan forward to find the quotes or bracket
			index += 7;
			int scan;
			for (scan = index; scan < filelength && filedata[scan] != '<' && filedata[scan] != '"' && filedata[scan] != 13 && filedata[scan] != 10; scan++) ;

			// ignore if not found or if it's bracketed
			if (scan >= filelength || filedata[scan] != '"')
				continue;
			int start = ++scan;

			// find the closing quote
			while (scan < filelength && filedata[scan] != '"')
				scan++;
			if (scan >= filelength)
				continue;

			// find the include file
			astring filename(&filedata[start], scan - start);
			astring target;

			// create a new dependency
			if (find_include_file(target, srcrootlen, srcfile, filename))
			{
				dependency *dep = new dependency;
				dep->next = file.deplist;
				file.deplist = dep;
				dep->file = &compute_dependencies(srcrootlen, target);
			}
		}

	osd_free(filedata);
	return file;
}



/***************************************************************************
    HELPERS
***************************************************************************/

/*-------------------------------------------------
    find_include_file - find an include file
-------------------------------------------------*/

static bool find_include_file(astring &srcincpath, int srcrootlen, const astring &srcfile, const astring &filename)
{
	// iterate over include paths and find the file
	for (include_path *curpath = incpaths; curpath != NULL; curpath = curpath->next)
	{
		// a '.' include path is specially treated
		if (curpath->path == ".")
			srcincpath.cpysubstr(srcfile, 0, srcfile.rchr(0, PATH_SEPARATOR[0]));
		else
			srcincpath.cpy(curpath->path);

		// append the filename piecemeal to account for directories
		int lastsepindex = 0;
		int sepindex;
		while ((sepindex = filename.chr(lastsepindex, '/')) != -1)
		{
			astring pathpart(filename, lastsepindex, sepindex - lastsepindex);

			// handle .. by removing a chunk from the incpath
			if (pathpart == "..")
			{
				int sepindex_part = srcincpath.rchr(0, PATH_SEPARATOR[0]);
				if (sepindex_part != -1)
					srcincpath.substr(0, sepindex_part);
			}

			// otherwise, append a path separator and the pathpart
			else
				srcincpath.cat(PATH_SEPARATOR).cat(pathpart);

			// advance past the previous index
			lastsepindex = sepindex + 1;
		}

		// now append the filename
		srcincpath.cat(PATH_SEPARATOR).catsubstr(filename, lastsepindex, -1);

		// see if we can open it
		core_file *testfile;
		if (core_fopen(srcincpath, OPEN_FLAG_READ, &testfile) == FILERR_NONE)
		{
			// close the file
			core_fclose(testfile);
			return true;
		}
	}
	return false;
}
