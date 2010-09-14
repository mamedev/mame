/***************************************************************************

    MAME source code dependency generator

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

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

#define HASH_SIZE		193


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _include_path include_path;
struct _include_path
{
	include_path *	next;
	const astring *	path;
};


typedef struct _exclude_path exclude_path;
struct _exclude_path
{
	exclude_path *	next;
	const astring *	path;
	int				pathlen;
	UINT8			recursive;
};


typedef struct _list_entry list_entry;
struct _list_entry
{
	list_entry *	next;
	const astring *	name;
};


typedef struct _file_entry file_entry;

typedef struct _dependency dependency;
struct _dependency
{
	dependency *	next;
	file_entry *	file;
};


struct _file_entry
{
	astring *		name;
	dependency *	deplist;
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static include_path *incpaths;
static exclude_path *excpaths;
static tagmap *file_map;



/***************************************************************************
    PROTOTYPES
***************************************************************************/

/* core output functions */
static int recurse_dir(int srcrootlen, const astring *srcdir);
static file_entry *compute_dependencies(int srcrootlen, const astring *srcfile);

/* path helpers */
static astring *find_include_file(int srcrootlen, const astring *srcfile, const astring *filename);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/* core output functions */
static int recurse_dir(int srcrootlen, const astring *srcdir);
static file_entry *compute_dependencies(int srcrootlen, const astring *srcfile);

/* path helpers */
static astring *find_include_file(int srcrootlen, const astring *srcfile, const astring *filename);



/***************************************************************************
    MAIN
***************************************************************************/

/*-------------------------------------------------
    main - main entry point
-------------------------------------------------*/

int main(int argc, char *argv[])
{
	include_path **incpathhead = &incpaths;
	exclude_path **excpathhead = &excpaths;
	astring *srcdir = NULL;
	int unadorned = 0;
	int result;
	int argnum;

	/* loop over arguments */
	for (argnum = 1; argnum < argc; argnum++)
	{
		char *arg = argv[argnum];

		/* include path? */
		if (arg[0] == '-' && arg[1] == 'I')
		{
			*incpathhead = (include_path *)malloc(sizeof(**incpathhead));
			if (*incpathhead != NULL)
			{
				(*incpathhead)->next = NULL;
				(*incpathhead)->path = astring_replacechr(astring_dupc(&arg[2]), '/', PATH_SEPARATOR[0]);
				incpathhead = &(*incpathhead)->next;
			}
		}

		/* exclude path? */
		else if (arg[0] == '-' && arg[1] == 'X')
		{
			*excpathhead = (exclude_path *)malloc(sizeof(**excpathhead));
			if (*excpathhead != NULL)
			{
				astring *path;
				(*excpathhead)->next = NULL;
				path = astring_replacechr(astring_dupc(&arg[2]), PATH_SEPARATOR[0], '/');
				(*excpathhead)->recursive = (astring_replacec(path, astring_len(path) - 4, "/...", "") != 0);
				(*excpathhead)->path = path;
				(*excpathhead)->pathlen = astring_len(path);
				excpathhead = &(*excpathhead)->next;
			}
		}

		/* other parameter */
		else if (arg[0] != '-' && unadorned == 0)
		{
			srcdir = astring_replacechr(astring_dupc(arg), '/', PATH_SEPARATOR[0]);
			unadorned++;
		}
		else
			goto usage;
	}

	/* make sure we got 1 parameter */
	if (srcdir == NULL)
		goto usage;
	
	/* create a tagmap for tracking files we've visited */
	file_map = tagmap_alloc();

	/* recurse over subdirectories */
	result = recurse_dir(astring_len(srcdir), srcdir);

	/* free source and destination directories */
	tagmap_free(file_map);
	astring_free(srcdir);
	return result;

usage:
	fprintf(stderr, "Usage:\n%s <srcroot> [-Iincpath [-Iincpath [...]]]\n", argv[0]);
	return 1;
}



/***************************************************************************
    CORE OUTPUT FUNCTIONS
***************************************************************************/

static int compare_list_entries(const void *p1, const void *p2)
{
	const list_entry *entry1 = *(const list_entry **)p1;
	const list_entry *entry2 = *(const list_entry **)p2;
	return strcmp(astring_c(entry1->name), astring_c(entry2->name));
}


/*-------------------------------------------------
    recurse_dependencies - recurse through the
    dependencies found, adding the mto the tagmap
    unless we already exist in the map
-------------------------------------------------*/

static void recurse_dependencies(file_entry *file, tagmap *map)
{
	int filelen = astring_len(file->name);
	exclude_path *exclude;
	dependency *dep;
	
	/* skip if we're in an exclude path */
	for (exclude = excpaths; exclude != NULL; exclude = exclude->next)
		if (exclude->pathlen < filelen && strncmp(astring_c(file->name), astring_c(exclude->path), exclude->pathlen) == 0)
			if (exclude->recursive || astring_chr(file->name, exclude->pathlen + 1, '/') == -1)
				return;
	
	/* attempt to add; if we get an error, we're already present */
	if (tagmap_add(map, astring_c(file->name), file->name, FALSE) != TMERR_NONE)
		return;
	
	/* recurse the list from there */
	for (dep = file->deplist; dep != NULL; dep = dep->next)
		recurse_dependencies(dep->file, map);
}


/*-------------------------------------------------
    recurse_dir - recurse through a directory
-------------------------------------------------*/

static int recurse_dir(int srcrootlen, const astring *srcdir)
{
	static const osd_dir_entry_type typelist[] = { ENTTYPE_DIR, ENTTYPE_FILE };
	int result = 0;
	int entindex;

	/* iterate first over directories, then over files */
	for (entindex = 0; entindex < ARRAY_LENGTH(typelist) && result == 0; entindex++)
	{
		osd_dir_entry_type entry_type = typelist[entindex];
		const osd_directory_entry *entry;
		list_entry **listarray = NULL;
		list_entry *list = NULL;
		list_entry *curlist;
		osd_directory *dir;
		int found = 0;

		/* open the directory and iterate through it */
		dir = osd_opendir(astring_c(srcdir));
		if (dir == NULL)
		{
			result = 1;
			goto error;
		}

		/* build up the list of files */
		while ((entry = osd_readdir(dir)) != NULL)
			if (entry->type == entry_type && entry->name[0] != '.')
			{
				list_entry *lentry = (list_entry *)malloc(sizeof(*lentry));
				lentry->name = astring_dupc(entry->name);
				lentry->next = list;
				list = lentry;
				found++;
			}

		/* close the directory */
		osd_closedir(dir);

		/* skip if nothing found */
		if (found == 0)
			continue;

		/* allocate memory for sorting */
		listarray = (list_entry **)malloc(sizeof(list_entry *) * found);
		found = 0;
		for (curlist = list; curlist != NULL; curlist = curlist->next)
			listarray[found++] = curlist;

		/* sort the list */
		qsort(listarray, found, sizeof(listarray[0]), compare_list_entries);

		/* rebuild the list */
		list = NULL;
		while (--found >= 0)
		{
			listarray[found]->next = list;
			list = listarray[found];
		}
		free(listarray);

		/* iterate through each file */
		for (curlist = list; curlist != NULL && result == 0; curlist = curlist->next)
		{
			astring *srcfile;

			/* build the source filename */
			srcfile = astring_alloc();
			astring_printf(srcfile, "%s%c%s", astring_c(srcdir), PATH_SEPARATOR[0], astring_c(curlist->name));

			/* if we have a file, output it */
			if (entry_type == ENTTYPE_FILE)
			{
				/* make sure we care, first */
				if (core_filename_ends_with(astring_c(curlist->name), ".c"))
				{
					tagmap *depend_map = tagmap_alloc();
					tagmap_entry *entry;
					file_entry *file;
					astring *target;
					int taghash;
					
					/* find dependencies */
					file = compute_dependencies(srcrootlen, srcfile);
					recurse_dependencies(file, depend_map);

					/* convert the target from source to object (makes assumptions about rules) */
					target = astring_dup(file->name);
					astring_replacec(target, 0, "src/", "$(OBJ)/");
					astring_replacec(target, 0, ".c", ".o");
					printf("\n%s : \\\n", astring_c(target));
					
					/* iterate over the hashed dependencies and output them as well */
					for (taghash = 0; taghash < TAGMAP_HASH_SIZE; taghash++)
						for (entry = depend_map->table[taghash]; entry != NULL; entry = entry->next)
							printf("\t%s \\\n", astring_c((astring *)entry->object));
					
					astring_free(target);
					tagmap_free(depend_map);
				}
			}

			/* if we have a directory, recurse */
			else
				result = recurse_dir(srcrootlen, srcfile);

			/* free memory for the names */
			astring_free(srcfile);
		}

		/* free all the allocated entries */
		while (list != NULL)
		{
			list_entry *next = list->next;
			astring_free((astring *)list->name);
			free(list);
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

static file_entry *compute_dependencies(int srcrootlen, const astring *srcfile)
{
	astring *normalfile;
	UINT32 filelength;
	file_entry *file;
	char *filedata;
	int index;
	
	/* see if we already have an entry */
	normalfile = astring_dup(srcfile);
	astring_replacechr(normalfile, PATH_SEPARATOR[0], '/');
	file = (file_entry *)tagmap_find(file_map, astring_c(normalfile));
	if (file != NULL)
		return file;

	/* create a new header entry */
	file = (file_entry *)malloc(sizeof(*file));
	file->deplist = NULL;
	file->name = normalfile;
	tagmap_add(file_map, astring_c(file->name), file, FALSE);

	/* read the source file */
	if (core_fload(astring_c(srcfile), (void **)&filedata, &filelength) != FILERR_NONE)
	{
		fprintf(stderr, "Unable to read file '%s'\n", astring_c(srcfile));
		return file;
	}

	/* find the #include directives in this file */
	for (index = 0; index < filelength; index++)
		if (filedata[index] == '#' && strncmp(&filedata[index + 1], "include", 7) == 0)
		{
			astring *filename, *target;
			int scan = index;
			dependency *dep;
			int start;
			
			/* first make sure we're not commented or quoted */
			for (scan = index; scan > 2 && filedata[scan] != 13 && filedata[scan] != 10; scan--)
				if ((filedata[scan] == '/' && filedata[scan - 1] == '/') || filedata[scan] == '"')
					break;
			if (filedata[scan] != 13 && filedata[scan] != 10)
				continue;
			
			/* scan forward to find the quotes or bracket */
			index += 7;
			for (scan = index; scan < filelength && filedata[scan] != '<' && filedata[scan] != '"' && filedata[scan] != 13 && filedata[scan] != 10; scan++) ;
				
			/* ignore if not found or if it's bracketed */
			if (scan >= filelength || filedata[scan] != '"')
				continue;
			start = ++scan;
			
			/* find the closing quote */
			while (scan < filelength && filedata[scan] != '"')
				scan++;
			if (scan >= filelength)
				continue;

			/* find the include file */
			filename = astring_dupch(&filedata[start], scan - start);
			target = find_include_file(srcrootlen, srcfile, filename);
			
			/* create a new dependency */
			if (target != NULL)
			{
				dep = (dependency *)malloc(sizeof(*dep));
				dep->next = file->deplist;
				file->deplist = dep;
				dep->file = compute_dependencies(srcrootlen, target);
				astring_free(target);
			}

			astring_free(filename);
		}

	free(filedata);
	return file;
}



/***************************************************************************
    HELPERS
***************************************************************************/

/*-------------------------------------------------
    find_include_file - find an include file
-------------------------------------------------*/

static astring *find_include_file(int srcrootlen, const astring *srcfile, const astring *filename)
{
	include_path *curpath;

	/* iterate over include paths and find the file */
	for (curpath = incpaths; curpath != NULL; curpath = curpath->next)
	{
		astring *srcincpath = astring_dup(curpath->path);
		core_file *testfile;
		int lastsepindex = 0;
		int sepindex;

		/* a '.' include path is specially treated */
		if (astring_cmpc(curpath->path, ".") == 0)
			astring_cpysubstr(srcincpath, srcfile, 0, astring_rchr(srcfile, 0, PATH_SEPARATOR[0]));

		/* append the filename piecemeal to account for directories */
		while ((sepindex = astring_chr(filename, lastsepindex, '/')) != -1)
		{
			astring *pathpart = astring_dupsubstr(filename, lastsepindex, sepindex - lastsepindex);

			/* handle .. by removing a chunk from the incpath */
			if (astring_cmpc(pathpart, "..") == 0)
			{
				int sepindex = astring_rchr(srcincpath, 0, PATH_SEPARATOR[0]);
				if (sepindex != -1)
					astring_substr(srcincpath, 0, sepindex);
			}

			/* otherwise, append a path separator and the pathpart */
			else
				astring_cat(astring_catc(srcincpath, PATH_SEPARATOR), pathpart);

			/* advance past the previous index */
			lastsepindex = sepindex + 1;

			/* free the path part we extracted */
			astring_free(pathpart);
		}

		/* now append the filename */
		astring_catsubstr(astring_catc(srcincpath, PATH_SEPARATOR), filename, lastsepindex, -1);

		/* see if we can open it */
		if (core_fopen(astring_c(srcincpath), OPEN_FLAG_READ, &testfile) == FILERR_NONE)
		{
			/* close the file */
			core_fclose(testfile);
			return srcincpath;
		}

		/* free our include path */
		astring_free(srcincpath);
	}
	return NULL;
}
